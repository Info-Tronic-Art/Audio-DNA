#pragma once
#include <juce_audio_devices/juce_audio_devices.h>
#include <atomic>
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

    // R13: the device sample rate, release-stored in audioDeviceAboutToStart
    // (message thread, callback quiesced) and acquire-loaded by the analysis
    // thread's resampler. 0.0 = no device / not started yet.
    const std::atomic<double>& sampleRateCell() const noexcept { return sampleRate_; }

#if AUDIODNA_TEST_SERVER
    // s-rta-0929 asyncload (plan-asyncload.md 5.7 / R9, TEST-ONLY build path): pushes the ring buffer could not take
    // whole (the analysis thread, its consumer, was starved). One compare + a relaxed add on the overrun path only.
    uint64_t ringOverruns() const noexcept { return overruns_.load(std::memory_order_relaxed); }
#endif

private:
    RingBuffer<float>& ringBuffer_;
    std::vector<float> monoBuffer_;  // pre-allocated in audioDeviceAboutToStart
    std::atomic<double> sampleRate_{0.0};
#if AUDIODNA_TEST_SERVER
    std::atomic<uint64_t> overruns_{0};
    static_assert(std::atomic<uint64_t>::is_always_lock_free, "audio-callback witness must be lock-free");   // AL8 b
#endif
};

static_assert(std::atomic<double>::is_always_lock_free,
              "AudioCallback's rate cell must be lock-free — read from the analysis thread");
