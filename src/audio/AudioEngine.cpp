#include "AudioEngine.h"

AudioEngine::AudioEngine(RingBuffer<float>& ringBuffer)
    : audioCallback_(ringBuffer),
      combinedCallback_(sourcePlayer_, audioCallback_)
{
    formatManager_.registerBasicFormats();
    transportSource_.addChangeListener(this);
    sourcePlayer_.setSource(&transportSource_);
    readAheadThread_.startThread(juce::Thread::Priority::normal);

    auto result = deviceManager_.initialiseWithDefaultDevices(0, 2);
    if (result.isNotEmpty())
        DBG("Audio device init error: " + result);

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
        return false;

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

void AudioEngine::setLooping(bool shouldLoop)
{
    if (readerSource_)
        readerSource_->setLooping(shouldLoop);
}

bool AudioEngine::isLooping() const
{
    if (readerSource_)
        return readerSource_->isLooping();
    return false;
}

void AudioEngine::changeListenerCallback(juce::ChangeBroadcaster*)
{
    if (onTransportStateChanged)
        onTransportStateChanged(transportSource_.isPlaying());
}
