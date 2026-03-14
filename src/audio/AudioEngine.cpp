#include "AudioEngine.h"

AudioEngine::AudioEngine(RingBuffer<float>& ringBuffer)
    : audioCallback_(ringBuffer),
      combinedCallback_(sourcePlayer_, audioCallback_)
{
    formatManager_.registerBasicFormats();
    transportSource_.addChangeListener(this);
    sourcePlayer_.setSource(&transportSource_);
    readAheadThread_.startThread(juce::Thread::Priority::normal);

    // Initialize with both input and output channels available
    // Input channels are needed for mic mode
    auto result = deviceManager_.initialiseWithDefaultDevices(2, 2);
    if (result.isNotEmpty())
    {
        std::cerr << "[AudioEngine] Device init error: " << result << std::endl;
        // Will report via onError callback once set up
    }

    deviceManager_.addAudioCallback(&combinedCallback_);
}

AudioEngine::~AudioEngine()
{
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

juce::String AudioEngine::getDeviceStatus() const
{
    auto* device = deviceManager_.getCurrentAudioDevice();
    if (device == nullptr)
        return "No audio device";
    return device->getName() + " @ " + juce::String(static_cast<int>(device->getCurrentSampleRate())) + "Hz";
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

void AudioEngine::changeListenerCallback(juce::ChangeBroadcaster*)
{
    if (onTransportStateChanged)
        onTransportStateChanged(transportSource_.isPlaying());
}
