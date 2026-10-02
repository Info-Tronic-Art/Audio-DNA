#include "AudioEngine.h"

AudioEngine::AudioEngine(RingBuffer<float>& ringBuffer)
    : audioCallback_(ringBuffer),
      combinedCallback_(sourcePlayer_, audioCallback_)
{
    formatManager_.registerBasicFormats();
    transportSource_.addChangeListener(this);
    sourcePlayer_.setSource(&transportSource_);
    readAheadThread_.startThread(juce::Thread::Priority::normal);

    // s-rta-0929b btguard: every device change (open / close / device list) reaches changeListenerCallback.
    deviceManager_.addChangeListener(this);
    deviceReconciler_.onReapplied = [this](const juce::String& error) {
        std::cerr << devguard::describeDevices(deviceManager_, deviceManager_.guardedType()) << std::endl;
        if (error.isNotEmpty() && onError)
            onError("Audio device: " + error);
        if (onDevicesReapplied)
            onDevicesReapplied(error);
    };

    // Initialize with both input and output channels available
    // Input channels are needed for mic mode
    // s-rta-0929b btguard: the manager's device type hides every Bluetooth / wireless device, so the "default devices"
    // here are the macOS defaults when allowed, else the built-in ones (DeviceGuard.h).
    // s-rta-0930 bt2: the reconciler's openDefaultDevices() is the app's ONE device choice (launch + every re-apply);
    // it records the launch's device scan, so only a device change AFTER launch is ever acted on.
    auto result = deviceReconciler_.openDefaultDevices();
    if (result.isNotEmpty())
    {
        std::cerr << "[AudioEngine] Device init error: " << result << std::endl;
        // Will report via onError callback once set up
    }

    deviceManager_.addAudioCallback(&combinedCallback_);
    std::cerr << devguard::describeDevices(deviceManager_, deviceManager_.guardedType()) << std::endl;
#if AUDIODNA_TEST_SERVER
    publishDeviceStatus();
#endif
}

AudioEngine::~AudioEngine()
{
    deviceManager_.removeChangeListener(this);
    deviceManager_.removeAudioCallback(&combinedCallback_);
    transportSource_.setSource(nullptr);
    sourcePlayer_.setSource(nullptr);
    readAheadThread_.stopThread(1000);
}

bool AudioEngine::loadFile(const juce::File& file)
{
    stop();

    // Disconnect the transport from the old source BEFORE destroying it
    transportSource_.setSource(nullptr);
    readerSource_.reset();

    auto* reader = formatManager_.createReaderFor(file);
    if (reader == nullptr)
    {
        if (onError)
            onError("Failed to load: " + file.getFileName());
        return false;
    }

    readerSource_ = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
    transportSource_.setSource(readerSource_.get(), 32768,
                               &readAheadThread_, reader->sampleRate);
    return true;
}

void AudioEngine::play()
{
    transportSource_.start();
}

void AudioEngine::pause()
{
    transportSource_.stop();
}

void AudioEngine::stop()
{
    transportSource_.stop();
    transportSource_.setPosition(0.0);
}

bool AudioEngine::isPlaying() const
{
    return transportSource_.isPlaying();
}


bool AudioEngine::hasAudioDevice() const
{
    return deviceManager_.getCurrentAudioDevice() != nullptr;
}

bool AudioEngine::hasInputDevice() const
{
    return hasAudioDevice() && deviceManager_.getAudioDeviceSetup().inputDeviceName.isNotEmpty();
}

AudioEngine::DeviceState AudioEngine::deviceStateFor(bool haveDevice, const juce::String& openInput,
                                                     const juce::String& lostInput)
{
    if (!haveDevice)
        return DeviceState::NoDevice;
    if (openInput.isEmpty())
        return DeviceState::NoInput;
    if (lostInput.isNotEmpty() && lostInput != openInput)
        return DeviceState::MicReplaced;
    return DeviceState::Ok;
}

AudioEngine::DeviceState AudioEngine::getDeviceState() const
{
    return deviceStateFor(hasAudioDevice(), openInput(), deviceReconciler_.lostInput());
}

juce::String AudioEngine::openInput() const
{
    return hasAudioDevice() ? deviceManager_.getAudioDeviceSetup().inputDeviceName : juce::String();
}

juce::String AudioEngine::lostInput() const
{
    return deviceReconciler_.lostInput();
}

juce::String AudioEngine::getDeviceStatus() const
{
    // s-rta-0929b btguard (Q4): names the INPUT device -- the device's own name is the combiner's, i.e. the OUTPUT's.
    auto* device = deviceManager_.getCurrentAudioDevice();
    if (device == nullptr)
        return "no audio device (Bluetooth is never used)";
    const auto input = deviceManager_.getAudioDeviceSetup().inputDeviceName;
    if (input.isEmpty())
        return "no wired mic (Bluetooth is never used)";
    return input + " @ " + juce::String(static_cast<int>(device->getCurrentSampleRate())) + "Hz";
}

double AudioEngine::getCurrentSampleRate() const
{
    auto* device = deviceManager_.getCurrentAudioDevice();
    return device != nullptr ? device->getCurrentSampleRate() : 0.0;
}

// bt2 C3 (Pitfall 61): the source mode is an atomic flag CombinedCallback reads every block -- never a device call here.
// The old setAudioDeviceSetup re-open changed nothing (JUCE re-enables the default input channels, updateSetupChannels)
// and sent the device through JUCE 8.0.4's CoreAudio open again at launch and on every switch.
void AudioEngine::setSourceMode(SourceMode mode)
{
    sourceMode_ = mode;

    if (mode == SourceMode::MicInput)
    {
        // Stop file playback
        stop();

        // Enable input channels
        combinedCallback_.useInputForAnalysis.store(true, std::memory_order_relaxed);

        std::cerr << "[AudioEngine] Switched to mic input mode" << std::endl;
    }
    else
    {
        // Disable input analysis mode
        combinedCallback_.useInputForAnalysis.store(false, std::memory_order_relaxed);

        std::cerr << "[AudioEngine] Switched to file playback mode" << std::endl;
    }
}

void AudioEngine::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &deviceManager_)
    {
#if AUDIODNA_TEST_SERVER
        publishDeviceStatus();
#endif
        if (onDeviceStateChanged)
            onDeviceStateChanged();
        return;
    }
    if (onTransportStateChanged)
        onTransportStateChanged(transportSource_.isPlaying());
}

#if AUDIODNA_TEST_SERVER
void AudioEngine::publishDeviceStatus()
{
    namespace dp = audiodna::devpolicy;
    const auto setup = deviceManager_.getAudioDeviceSetup();
    auto* device = deviceManager_.getCurrentAudioDevice();
    const auto* guarded = deviceManager_.guardedType();

    auto* obj = new juce::DynamicObject();
    obj->setProperty("ok", true);
    const auto state = getDeviceState();
    obj->setProperty("state", state == DeviceState::Ok ? "ok"
                              : state == DeviceState::NoInput ? "no-input"
                              : state == DeviceState::MicReplaced ? "mic-replaced" : "no-device");

    auto* opened = new juce::DynamicObject();
    opened->setProperty("input", device != nullptr ? setup.inputDeviceName : juce::String());
    opened->setProperty("output", device != nullptr ? setup.outputDeviceName : juce::String());
    opened->setProperty("sample_rate", device != nullptr ? device->getCurrentSampleRate() : 0.0);
    opened->setProperty("buffer_size", device != nullptr ? device->getCurrentBufferSizeSamples() : 0);
    opened->setProperty("input_channels", device != nullptr ? device->getActiveInputChannels().countNumberOfSetBits() : 0);
    opened->setProperty("output_channels", device != nullptr ? device->getActiveOutputChannels().countNumberOfSetBits() : 0);
    obj->setProperty("opened", juce::var(opened));
    obj->setProperty("reapplies", deviceReconciler_.reapplies());
    obj->setProperty("last_reapply", dp::toString(deviceReconciler_.lastAction()));   // s-rta-0930 bt2
    obj->setProperty("lost_input", deviceReconciler_.lostInput());

    const auto strings = [](const juce::StringArray& a) {
        juce::Array<juce::var> v;
        for (const auto& s : a)
            v.add(s);
        return juce::var(v);
    };
    if (guarded != nullptr)
    {
        const auto& scan = guarded->lastScan();
        obj->setProperty("scan_seq", static_cast<juce::int64>(scan.seq));
        obj->setProperty("enumerate_ms", scan.enumerateMs);
        auto* lists = new juce::DynamicObject();
        lists->setProperty("inputs", strings(scan.lists.inputs));
        lists->setProperty("outputs", strings(scan.lists.outputs));
        lists->setProperty("default_input", scan.lists.defaultInput);
        lists->setProperty("default_output", scan.lists.defaultOutput);
        obj->setProperty("lists", juce::var(lists));

        juce::Array<juce::var> devices;
        for (const auto& d : scan.devices)
        {
            auto* row = new juce::DynamicObject();
            row->setProperty("name_in", d.inputName);
            row->setProperty("name_out", d.outputName);
            row->setProperty("uid", d.uid);
            row->setProperty("transport", dp::transport::toString(d.transport));
            row->setProperty("transport_read_ok", d.transportReadOk);
            juce::Array<juce::var> members;
            for (const auto& m : d.members)
            {
                auto* mo = new juce::DynamicObject();
                mo->setProperty("uid", m.uid);
                mo->setProperty("transport", dp::transport::toString(m.transport));
                mo->setProperty("resolved", m.resolved);
                members.add(juce::var(mo));
            }
            row->setProperty("members", members);
            row->setProperty("default_input", d.isDefaultInput);
            row->setProperty("default_output", d.isDefaultOutput);
            // allowed / reason as the guard decided (the listed names are the allowed ones)
            juce::String reason;
            bool allowed = (d.inputName.isNotEmpty() && scan.lists.inputs.contains(d.inputName))
                        || (d.outputName.isNotEmpty() && scan.lists.outputs.contains(d.outputName));
            for (const auto& sk : scan.lists.skipped)
                if (sk.name == d.inputName || sk.name == d.outputName)
                {
                    reason = sk.reason;
                    allowed = false;
                }
            row->setProperty("allowed", allowed);
            row->setProperty("reason", reason);
            devices.add(juce::var(row));
        }
        obj->setProperty("devices", devices);

        juce::Array<juce::var> skipped;
        for (const auto& sk : scan.lists.skipped)
        {
            auto* so = new juce::DynamicObject();
            so->setProperty("name", sk.name);
            so->setProperty("reason", sk.reason);
            so->setProperty("input", sk.input);
            so->setProperty("output", sk.output);
            skipped.add(juce::var(so));
        }
        obj->setProperty("skipped", skipped);
        obj->setProperty("unmapped", strings(scan.lists.unmapped));
    }
    obj->setProperty("test_denied", strings(deviceManager_.policyConfig().testDeniedNames));

    std::lock_guard<std::mutex> lock(deviceStatusMutex_);
    deviceStatus_ = juce::var(obj);
}

juce::var AudioEngine::deviceStatusVar() const
{
    juce::var published;
    {
        std::lock_guard<std::mutex> lock(deviceStatusMutex_);
        published = deviceStatus_;
    }
    // A fresh top-level object (the published one is never mutated) + the live device-start count.
    auto* obj = new juce::DynamicObject();
    if (auto* src = published.getDynamicObject())
        for (const auto& p : src->getProperties())
            obj->setProperty(p.name, p.value);
    obj->setProperty("opens", combinedCallback_.opens());
    return juce::var(obj);
}

void AudioEngine::debugSetDeniedDevices(const juce::StringArray& names)
{
    deviceManager_.setTestDeniedNames(names);
}

void AudioEngine::debugStopDevice()
{
    if (auto* device = deviceManager_.getCurrentAudioDevice())
    {
        device->stop();
        std::cerr << "[AudioEngine] TEST-ONLY audio_stop: stopped \"" << device->getName() << "\" (the manager keeps it)"
                  << std::endl;
    }
}
#endif
