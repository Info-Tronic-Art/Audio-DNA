#pragma once
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "AudioCallback.h"
#include "RingBuffer.h"

// Owns AudioDeviceManager, AudioTransportSource, format readers.
// Provides transport controls and feeds AudioCallback for analysis.
class AudioEngine : public juce::ChangeListener
{
public:
    AudioEngine(RingBuffer<float>& ringBuffer);
    ~AudioEngine() override;

    bool loadFile(const juce::File& file);
    void play();
    void pause();
    void stop();
    bool isPlaying() const;

    void setLooping(bool shouldLoop);
    bool isLooping() const;

    juce::AudioDeviceManager& getDeviceManager() { return deviceManager_; }
    juce::AudioTransportSource& getTransportSource() { return transportSource_; }

    // ChangeListener — transport state changes
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    std::function<void(bool isPlaying)> onTransportStateChanged;
    std::function<void(const juce::String& message)> onError;

    bool hasAudioDevice() const;
    juce::String getDeviceStatus() const;

private:
    juce::AudioDeviceManager deviceManager_;
    juce::AudioFormatManager formatManager_;
    juce::AudioSourcePlayer sourcePlayer_;
    juce::AudioTransportSource transportSource_;

    std::unique_ptr<juce::AudioFormatReaderSource> readerSource_;
    juce::TimeSliceThread readAheadThread_{"AudioReadAhead"};
    AudioCallback audioCallback_;

    // Custom callback that mixes transport output and feeds the ring buffer
    class CombinedCallback : public juce::AudioIODeviceCallback
    {
    public:
        CombinedCallback(juce::AudioSourcePlayer& player, AudioCallback& analysisCallback)
            : player_(player), analysisCallback_(analysisCallback) {}

        void audioDeviceIOCallbackWithContext(
            const float* const* inputChannelData, int numInputChannels,
            float* const* outputChannelData, int numOutputChannels,
            int numSamples,
            const juce::AudioIODeviceCallbackContext& context) override
        {
            // Let the source player fill output buffers
            player_.audioDeviceIOCallbackWithContext(
                inputChannelData, numInputChannels,
                outputChannelData, numOutputChannels,
                numSamples, context);

            // Feed the output to analysis ring buffer
            analysisCallback_.audioDeviceIOCallbackWithContext(
                inputChannelData, numInputChannels,
                outputChannelData, numOutputChannels,
                numSamples, context);
        }

        void audioDeviceAboutToStart(juce::AudioIODevice* device) override
        {
            player_.audioDeviceAboutToStart(device);
            analysisCallback_.audioDeviceAboutToStart(device);
        }

        void audioDeviceStopped() override
        {
            player_.audioDeviceStopped();
            analysisCallback_.audioDeviceStopped();
        }

    private:
        juce::AudioSourcePlayer& player_;
        AudioCallback& analysisCallback_;
    };

    CombinedCallback combinedCallback_;
};
