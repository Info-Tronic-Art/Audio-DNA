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

#if AUDIODNA_TEST_SERVER
void GuardedDeviceType::setTestDeniedNames(const juce::StringArray& names)
{
    config_.testDeniedNames = names;
    std::cerr << "[AudioEngine] TEST-ONLY audio_deny: "
              << (names.isEmpty() ? juce::String("none") : names.joinIntoString("; ")) << std::endl;
    inner_->scanForDevices();
    audioDeviceListChanged();   // what CoreAudioIODeviceType::audioDeviceListChanged does, minus the HAL trigger
}
#endif

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

#if AUDIODNA_TEST_SERVER
void GuardedAudioDeviceManager::setTestDeniedNames(const juce::StringArray& names)
{
    config_.testDeniedNames = names;
    if (guarded_ != nullptr)
        guarded_->setTestDeniedNames(names);
}
#endif

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
    if (manager_.getCurrentAudioDevice() != nullptr)
    {
        hadDevice_ = true;
        stopTimer();
        return;
    }
    if (hadDevice_)
        startTimer(settleMs_);   // restarts: re-evaluated after the LAST change message
}

void DeviceReconciler::timerCallback()
{
    stopTimer();
    if (manager_.getCurrentAudioDevice() != nullptr)
    {
        hadDevice_ = true;
        return;
    }
    if (!hadDevice_)
        return;
    const auto* guarded = manager_.guardedType();
    if (guarded == nullptr
        || (guarded->lastScan().lists.inputs.isEmpty() && guarded->lastScan().lists.outputs.isEmpty()))
        return;   // nothing allowed now; the next device-list change re-evaluates

    if (reapplies_ > 0 && guarded->lastScan().seq == lastReappliedSeq_)
        return;   // a failed re-apply is retried only after a NEW device-list change

    const double now = juce::Time::getMillisecondCounterHiRes();
    if (reapplies_ > 0 && now - lastReapplyMs_ < minIntervalMs_)
    {
        startTimer(juce::jmax(1, static_cast<int>(minIntervalMs_ - (now - lastReapplyMs_))));
        return;
    }
    lastReapplyMs_ = now;
    lastReappliedSeq_ = guarded->lastScan().seq;
    ++reapplies_;
    std::cerr << "[AudioEngine] the open audio device is gone -- re-applying the device policy" << std::endl;
    manager_.closeAudioDevice();
    const auto error = manager_.initialiseWithDefaultDevices(numIns_, numOuts_);
    if (manager_.getCurrentAudioDevice() != nullptr)
        hadDevice_ = true;
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
