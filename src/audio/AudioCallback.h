#pragma once
#include <juce_audio_devices/juce_audio_devices.h>
#include "RingBuffer.h"

// Sits in the real-time audio callback. Mono-downmixes and pushes samples
// into the SPSC ring buffer. Zero allocation, no locks.
class AudioCallback : public juce::AudioIODeviceCallback
{
public:
    explicit AudioCallback(RingBuffer<float>& ringBuffer);

    void audioDeviceIOCallbackWithContext(
        const float* const* inputChannelData,
        int numInputChannels,
        float* const* outputChannelData,
        int numOutputChannels,
        int numSamples,
        const juce::AudioIODeviceCallbackContext& context) override;

    void audioDeviceAboutToStart(juce::AudioIODevice* device) override;
    void audioDeviceStopped() override;

private:
    RingBuffer<float>& ringBuffer_;
    std::vector<float> monoBuffer_;  // pre-allocated in audioDeviceAboutToStart
};
