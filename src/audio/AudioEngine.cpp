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
    };

    // Initialize with both input and output channels available
    // Input channels are needed for mic mode
    // s-rta-0929b btguard: the manager's device type hides every Bluetooth / wireless device, so the "default devices"
    // here are the macOS defaults when allowed, else the built-in ones (DeviceGuard.h).
    auto result = deviceManager_.initialiseWithDefaultDevices(2, 2);
    if (result.isNotEmpty())
    {
        std::cerr << "[AudioEngine] Device init error: " << result << std::endl;
        // Will report via onError callback once set up
    }

    deviceManager_.addAudioCallback(&combinedCallback_);
    std::cerr << devguard::describeDevices(deviceManager_, deviceManager_.guardedType()) << std::endl;
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

AudioEngine::DeviceState AudioEngine::getDeviceState() const
{
    if (!hasAudioDevice())
        return DeviceState::NoDevice;
    return hasInputDevice() ? DeviceState::Ok : DeviceState::NoInput;
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

void AudioEngine::setSourceMode(SourceMode mode)
{
    sourceMode_ = mode;

    if (mode == SourceMode::MicInput)
    {
        // Stop file playback
        stop();

        // Enable input channels
        combinedCallback_.useInputForAnalysis.store(true, std::memory_order_relaxed);

        // Re-open device with input enabled
        auto setup = deviceManager_.getAudioDeviceSetup();
        setup.inputChannels.setRange(0, 2, true);  // Enable stereo input
        deviceManager_.setAudioDeviceSetup(setup, true);

        std::cerr << "[AudioEngine] Switched to mic input mode" << std::endl;
    }
    else
    {
        // Disable input analysis mode
        combinedCallback_.useInputForAnalysis.store(false, std::memory_order_relaxed);

        // Can disable input channels to reduce latency
        auto setup = deviceManager_.getAudioDeviceSetup();
        setup.inputChannels.clear();
        deviceManager_.setAudioDeviceSetup(setup, true);

        std::cerr << "[AudioEngine] Switched to file playback mode" << std::endl;
    }
}

void AudioEngine::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &deviceManager_)
    {
        if (onDeviceStateChanged)
            onDeviceStateChanged();
        return;
    }
    if (onTransportStateChanged)
        onTransportStateChanged(transportSource_.isPlaying());
}
