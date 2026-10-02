#include "DeviceGuard.h"
#include <cstdlib>
#include <iostream>

namespace dp = audiodna::devpolicy;

GuardedDeviceType::GuardedDeviceType(std::unique_ptr<juce::AudioIODeviceType> inner, Enumerate enumerate, dp::Config config)
    : juce::AudioIODeviceType(inner->getTypeName()),
      inner_(std::move(inner)),
      enumerate_(std::move(enumerate)),
      config_(std::move(config))
{
    inner_->addListener(this);
}

GuardedDeviceType::~GuardedDeviceType()
{
    inner_->removeListener(this);   // before inner_ is destroyed
}

void GuardedDeviceType::scanForDevices()
{
    inner_->scanForDevices();
    rebuild();
}

juce::StringArray GuardedDeviceType::getDeviceNames(bool wantInputNames) const
{
    return wantInputNames ? scan_.lists.inputs : scan_.lists.outputs;
}

int GuardedDeviceType::getDefaultDeviceIndex(bool forInput) const
{
    return forInput ? scan_.lists.defaultInput : scan_.lists.defaultOutput;
}

int GuardedDeviceType::getIndexOfDevice(juce::AudioIODevice* device, bool asInput) const
{
    const int i = inner_->getIndexOfDevice(device, asInput);
    if (i < 0)
        return -1;
    const auto name = inner_->getDeviceNames(asInput)[i];
    return (asInput ? scan_.lists.inputs : scan_.lists.outputs).indexOf(name);
}

bool GuardedDeviceType::hasSeparateInputsAndOutputs() const
{
    return inner_->hasSeparateInputsAndOutputs();
}

juce::AudioIODevice* GuardedDeviceType::createDevice(const juce::String& outputDeviceName, const juce::String& inputDeviceName)
{
    // "" is always fine (that direction stays closed); any other name must be in the filtered list.
    const auto refuse = [](const juce::String& name) {
        std::cerr << "[AudioEngine] refused device \"" << name << "\" (not allowed by the device policy)" << std::endl;
        return nullptr;
    };
    if (outputDeviceName.isNotEmpty() && !scan_.lists.outputs.contains(outputDeviceName))
        return refuse(outputDeviceName);
    if (inputDeviceName.isNotEmpty() && !scan_.lists.inputs.contains(inputDeviceName))
        return refuse(inputDeviceName);
    return inner_->createDevice(outputDeviceName, inputDeviceName);
}

void GuardedDeviceType::audioDeviceListChanged()
{
    rebuild();
    callDeviceChangeListeners();
}

void GuardedDeviceType::rebuild()
{
    const auto enumerateTimed = [this](double& ms) {
        const auto t0 = juce::Time::getMillisecondCounterHiRes();
        auto devices = enumerate_ ? enumerate_() : std::vector<dp::DeviceInfo>{};
        ms += juce::Time::getMillisecondCounterHiRes() - t0;
        return devices;
    };

    double ms = 0.0;
    auto devices = enumerateTimed(ms);
    auto lists = dp::filter(inner_->getDeviceNames(true), inner_->getDeviceNames(false), devices, config_);
    if (!lists.unmapped.isEmpty())
    {
        // A device appeared / vanished between the two HAL queries: re-scan once, then hide what still does not map.
        inner_->scanForDevices();
        devices = enumerateTimed(ms);
        lists = dp::filter(inner_->getDeviceNames(true), inner_->getDeviceNames(false), devices, config_);
    }

    scan_.devices = std::move(devices);
    scan_.lists = std::move(lists);
    scan_.enumerateMs = ms;
    ++scan_.seq;

    // One stderr line per change of what is hidden (never per scan), plus a slow-scan line (attack S3/S7).
    juce::String hidden;
    for (const auto& s : scan_.lists.skipped)
        hidden << "[AudioEngine] skipped \"" << s.name << "\" (" << s.reason << "; "
               << (s.input && s.output ? "input, output" : (s.input ? "input" : "output")) << ")\n";
    for (const auto& n : scan_.lists.unmapped)
        hidden << "[AudioEngine] skipped \"" << n << "\" (not found in the device scan; hidden)\n";
    if (hidden != lastLogged_)
    {
        lastLogged_ = hidden;
        std::cerr << hidden;
    }
    if (ms > 50.0)
        std::cerr << "[AudioEngine] device scan took " << juce::String(ms, 1) << " ms" << std::endl;
}

GuardedAudioDeviceManager::GuardedAudioDeviceManager(dp::Config config, GuardedDeviceType::Enumerate enumerate)
    : config_(std::move(config)), enumerate_(std::move(enumerate))
{
}

void GuardedAudioDeviceManager::createAudioDeviceTypes(juce::OwnedArray<juce::AudioIODeviceType>& types)
{
    const bool testTypes = static_cast<bool>(typeFactoryForTests);
    if (testTypes)
        typeFactoryForTests(types);
    else
        juce::AudioDeviceManager::createAudioDeviceTypes(types);

#if ! JUCE_MAC
    if (!testTypes)
        return;   // no enumerator off macOS: the policy would hide every device
#endif
    for (int i = 0; i < types.size(); ++i)
    {
        if (types.getUnchecked(i)->getTypeName() != "CoreAudio")
            continue;
        std::unique_ptr<juce::AudioIODeviceType> inner(types.getUnchecked(i));
        guarded_ = new GuardedDeviceType(std::move(inner), enumerate_, config_);
        types.set(i, guarded_, false);   // the old pointer is owned by the decorator now
    }
}

DeviceReconciler::DeviceReconciler(GuardedAudioDeviceManager& manager, int numInputChannels, int numOutputChannels,
                                   int settleMs, int minIntervalMs)
    : manager_(manager), numIns_(numInputChannels), numOuts_(numOutputChannels), settleMs_(settleMs),
      minIntervalMs_(minIntervalMs)
{
    manager_.addChangeListener(this);
}

DeviceReconciler::~DeviceReconciler()
{
    stopTimer();
    manager_.removeChangeListener(this);
}

void DeviceReconciler::changeListenerCallback(juce::ChangeBroadcaster*)
{
    startTimer(1);   // MUTANT restarts: re-evaluated after the LAST change message
}

juce::String DeviceReconciler::openDefaultDevices()
{
    if (manager_.getCurrentAudioDevice() != nullptr)
        manager_.closeAudioDevice();
    const auto error = manager_.initialiseWithDefaultDevices(numIns_, numOuts_);
    if (const auto* guarded = manager_.guardedType())
        lastAttemptSeq_ = guarded->lastScan().seq;   // AFTER the open: a list change the open itself caused is not new
    return error;
}

void DeviceReconciler::timerCallback()
{
    stopTimer();
    const auto* guarded = manager_.guardedType();
    if (guarded == nullptr)
        return;
    const auto& scan = guarded->lastScan();
    if (!noInputOn_.isEmpty() && noInputOn_ != scan.lists.inputs)
        noInputOn_.clear();   // the allowed inputs changed: an adoption may try again
    auto* device = manager_.getCurrentAudioDevice();
    const bool haveDevice = device != nullptr;
    const auto previous = manager_.getAudioDeviceSetup();
    const auto action = dp::reconcile(haveDevice, haveDevice && device->isPlaying(), previous.inputDeviceName, scan.lists);
    if (action == dp::Reapply::None)
        return;
    if (action == dp::Reapply::AdoptInput && noInputOn_ == scan.lists.inputs)
        return;   // no listed input could be opened: wait for a DIFFERENT list of allowed inputs
    if (scan.seq == lastAttemptSeq_)
        return;   // this scan was already acted on (or opened at launch): wait for a NEW device-list change

    const double now = juce::Time::getMillisecondCounterHiRes();
    if (reapplies_ > 0 && now - lastReapplyMs_ < minIntervalMs_)
    {
        startTimer(juce::jmax(1, static_cast<int>(minIntervalMs_ - (now - lastReapplyMs_))));
        return;
    }
    lastReapplyMs_ = now;
    ++reapplies_;
    lastAction_ = action;
    std::cerr << "[AudioEngine] " << dp::toString(action) << " -- re-applying the device policy" << std::endl;
    const auto lists = scan.lists;   // copy: the open below may rebuild the scan
    auto error = openDefaultDevices();
    if (haveDevice && manager_.getCurrentAudioDevice() == nullptr)
    {
        // A re-apply never leaves the app worse off: put back what still works of the device it had (output-only).
        auto keep = previous;
        if (!lists.inputs.contains(keep.inputDeviceName))
            keep.inputDeviceName = {};
        if (!lists.outputs.contains(keep.outputDeviceName))
            keep.outputDeviceName = {};
        if (keep.inputDeviceName.isNotEmpty() || keep.outputDeviceName.isNotEmpty())
        {
            const auto restoreError = manager_.setAudioDeviceSetup(keep, false);
            std::cerr << "[AudioEngine] the re-apply failed (" << error << "); kept the previous device"
                      << (restoreError.isEmpty() ? juce::String() : " -- that failed too: " + restoreError) << std::endl;
        }
    }
    const bool nowDevice = manager_.getCurrentAudioDevice() != nullptr;
    const auto nowInput = nowDevice ? manager_.getAudioDeviceSetup().inputDeviceName : juce::String();
    noInputOn_ = (nowDevice && nowInput.isEmpty() && !lists.inputs.isEmpty()) ? lists.inputs : juce::StringArray();
    if (nowInput != previous.inputDeviceName)
        lostInput_ = ((action == dp::Reapply::InputLost || action == dp::Reapply::DeviceStopped)
                      && previous.inputDeviceName.isNotEmpty() && nowInput.isNotEmpty())
                         ? previous.inputDeviceName : juce::String();
    if (onReapplied)
        onReapplied(error);
}

namespace devguard
{
dp::Config productionConfig()
{
    dp::Config config;
#if AUDIODNA_TEST_SERVER
    if (const char* env = std::getenv("ADNA_AUDIO_DENY_DEVICES"))
    {
        config.testDeniedNames.addTokens(juce::String::fromUTF8(env), ";", "");
        config.testDeniedNames.trim();
        config.testDeniedNames.removeEmptyStrings();
        std::cerr << "[AudioEngine] TEST-ONLY ADNA_AUDIO_DENY_DEVICES: "
                  << config.testDeniedNames.joinIntoString("; ") << std::endl;
    }
#endif
    return config;
}

juce::String describeDevices(const juce::AudioDeviceManager& manager, const GuardedDeviceType* guarded)
{
    const auto setup = manager.getAudioDeviceSetup();
    const auto transportOf = [guarded](const juce::String& name, bool input) -> juce::String {
        if (guarded != nullptr)
            for (const auto& d : guarded->lastScan().devices)
                if ((input ? d.inputName : d.outputName) == name)
                    return d.transportReadOk ? dp::transport::toString(d.transport) : juce::String("unreadable");
        return "?";
    };
    const bool open = manager.getCurrentAudioDevice() != nullptr;
    const auto side = [&](const juce::String& name, bool input) -> juce::String {
        if (!open || name.isEmpty())
            return "none";
        return "\"" + name + "\" (" + transportOf(name, input) + ")";
    };

    juce::String line = "[AudioEngine] audio devices: input " + side(setup.inputDeviceName, true)
                      + ", output " + side(setup.outputDeviceName, false) + "; skipped: ";
    juce::StringArray skipped;
    if (guarded != nullptr)
        for (const auto& s : guarded->lastScan().lists.skipped)
            skipped.add("\"" + s.name + "\" (" + s.reason + "; "
                        + (s.input && s.output ? "input, output" : (s.input ? "input" : "output")) + ")");
    line += skipped.isEmpty() ? juce::String("none") : skipped.joinIntoString(", ");
    return line;
}
}
