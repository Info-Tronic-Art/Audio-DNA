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

    // Load an audio file for analysis (no playback output)
    bool loadFile(const juce::File& file);
    void play();   // Start reading file into analysis pipeline
    void pause();
    void stop();
    bool isPlaying() const;

    juce::AudioDeviceManager& getDeviceManager() { return deviceManager_; }
    juce::AudioTransportSource& getTransportSource() { return transportSource_; }

    // ChangeListener — transport state changes
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    std::function<void(bool isPlaying)> onTransportStateChanged;
    std::function<void(const juce::String& message)> onError;

    bool hasAudioDevice() const;
    juce::String getDeviceStatus() const;

    // Audio source mode
    enum class SourceMode { File, MicInput };
    void setSourceMode(SourceMode mode);
    SourceMode getSourceMode() const { return sourceMode_; }

    void setInputGain(float gain) { combinedCallback_.inputGain.store(gain, std::memory_order_relaxed); }
    float getInputLevel() const { return combinedCallback_.inputLevel.load(std::memory_order_relaxed); }

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

        // When true, analysis reads from input channels (mic) instead of output
        std::atomic<bool> useInputForAnalysis{false};
        std::atomic<float> inputGain{1.0f};
        std::atomic<float> inputLevel{0.0f};  // Peak level for metering

        void audioDeviceIOCallbackWithContext(
            const float* const* inputChannelData, int numInputChannels,
            float* const* outputChannelData, int numOutputChannels,
            int numSamples,
            const juce::AudioIODeviceCallbackContext& context) override
        {
            // Always silence output — this app is analysis-only, no audio playback
            if (useInputForAnalysis.load(std::memory_order_relaxed))
            {
                // Mic/loopback mode: analyze input channels
                // Compute input peak level for metering
                if (numInputChannels > 0 && inputChannelData != nullptr)
                {
                    float peak = 0.0f;
                    for (int i = 0; i < numSamples; ++i)
                    {
                        float s = std::fabs(inputChannelData[0][i]);
                        if (s > peak) peak = s;
                    }
                    float gain = inputGain.load(std::memory_order_relaxed);
                    peak *= gain;
                    float prev = inputLevel.load(std::memory_order_relaxed);
                    float smoothed = (peak > prev) ? peak : prev * 0.92f;
                    inputLevel.store(smoothed, std::memory_order_relaxed);

                    // Feed input to analysis with gain applied
                    if (gain != 1.0f)
                    {
                        int chans = std::min(numInputChannels, numOutputChannels);
                        for (int ch = 0; ch < chans; ++ch)
                            for (int i = 0; i < numSamples; ++i)
                                outputChannelData[ch][i] = inputChannelData[ch][i] * gain;

                        analysisCallback_.audioDeviceIOCallbackWithContext(
                            nullptr, 0, outputChannelData, chans, numSamples, context);
                    }
                    else
                    {
                        analysisCallback_.audioDeviceIOCallbackWithContext(
                            nullptr, 0,
                            const_cast<float* const*>(inputChannelData), numInputChannels,
                            numSamples, context);
                    }
                }
            }
            else
            {
                // File mode: read file into internal buffers for analysis (no audible output)
                player_.audioDeviceIOCallbackWithContext(
                    inputChannelData, numInputChannels,
                    outputChannelData, numOutputChannels,
                    numSamples, context);

                // Feed to analysis before silencing
                analysisCallback_.audioDeviceIOCallbackWithContext(
                    inputChannelData, numInputChannels,
                    outputChannelData, numOutputChannels,
                    numSamples, context);
            }

            // Silence output in all modes
            for (int ch = 0; ch < numOutputChannels; ++ch)
                juce::FloatVectorOperations::clear(outputChannelData[ch], numSamples);
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
    SourceMode sourceMode_ = SourceMode::File;
};
