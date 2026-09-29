#pragma once
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "AudioCallback.h"
#include "recording/AudioTap.h"

// CombinedCallback -- lifted out of AudioEngine.h (s167 D10.1/Build order
// row 2) purely for testability: T1 (tests/test_audio_tap_sync.cpp)
// constructs one directly, without a real AudioIODevice/AudioEngine, and
// drives it block-by-block. This is a MOVE -- behaviour for the pre-existing
// mic/file analysis fan-out is unchanged from the private nested class it
// replaces (AudioEngine.h:57-148 prior to this packet), except that the two
// branches now compute `listened`/`listenedChans` ONCE instead of calling
// analysisCallback_ inline from each branch, so the SAME buffer/channel
// count can also be handed to the new second fan-out, AudioTap (D10.1: "fed
// the same buffers the analysis is fed, at the same point").
//
// New in this packet: the delivered-sample counter (the take's timebase
// origin, D1) and the AudioTap fan-out itself. Both are additive; nothing
// about the analysis ring buffer's single-consumer ownership changes (G26,
// R12 -- untouched, not this class's problem).
class CombinedCallback : public juce::AudioIODeviceCallback
{
public:
    CombinedCallback(juce::AudioSourcePlayer& player, AudioCallback& analysisCallback)
        : player_(player), analysisCallback_(analysisCallback) {}

    // When true, analysis reads from input channels (mic) instead of output
    std::atomic<bool> useInputForAnalysis{false};
    std::atomic<float> inputGain{1.0f};
    std::atomic<float> inputLevel{0.0f};  // Peak level for metering

    // D10.1: the take's timebase origin. Relaxed/monotonic per the spec's
    // own threading-contract table (section 4) -- independent of the
    // analysis ring buffer, so G26's silent overflow there can never
    // desynchronise a take.
    uint64_t getDeliveredSamples() const { return deliveredSamples_.load(std::memory_order_relaxed); }

    AudioTap& tap() { return audioTap_; }

#if AUDIODNA_TEST_SERVER
    // s-rta-0929 asyncload (plan-asyncload.md 5.7 / R9, TEST-ONLY build path): the audio-callback witnesses of
    // /api/state "load" -- the longest gap between two callback arrivals since the last read (reset on read), the
    // callback count and the last period in samples. The callback's cost: one mach_absolute_time read (a commpage
    // read, no syscall) and three relaxed atomics; no allocation, lock, syscall or exception (Sacred Rule 1). Absent
    // from a build without AUDIODNA_BUILD_TEST_SERVER.
    double takeGapMaxMs()
    {
        const int64_t t = gapMaxTicks_.exchange(0, std::memory_order_relaxed);
        return static_cast<double>(t) * 1000.0 / static_cast<double>(juce::Time::getHighResolutionTicksPerSecond());
    }
    uint64_t callbacks() const { return callbacks_.load(std::memory_order_relaxed); }
    int periodSamples() const { return periodSamples_.load(std::memory_order_relaxed); }
#endif

    void audioDeviceIOCallbackWithContext(
        const float* const* inputChannelData, int numInputChannels,
        float* const* outputChannelData, int numOutputChannels,
        int numSamples,
        const juce::AudioIODeviceCallbackContext& context) override
    {
#if AUDIODNA_TEST_SERVER
        // s-rta-0929 asyncload (TEST-ONLY): the inter-arrival gap witness (see takeGapMaxMs).
        {
            const int64_t now = juce::Time::getHighResolutionTicks();
            if (lastTicks_ != 0)
            {
                const int64_t gap = now - lastTicks_;
                if (gap > gapMaxTicks_.load(std::memory_order_relaxed))
                    gapMaxTicks_.store(gap, std::memory_order_relaxed);
            }
            lastTicks_ = now;
            callbacks_.fetch_add(1, std::memory_order_relaxed);
            periodSamples_.store(numSamples, std::memory_order_relaxed);
        }
#endif
        // "What the app listens to" -- computed once (D10.1's restructure),
        // then fed to BOTH the analysis callback and the tap. Preserves the
        // pre-existing mic/file behaviour exactly; only the shape changed
        // (inline calls -> one shared local).
        const float* const* listened = nullptr;
        int listenedChans = 0;

        if (useInputForAnalysis.load(std::memory_order_relaxed))
        {
            // Mic/loopback mode: analyze input channels.
            if (numInputChannels > 0 && inputChannelData != nullptr)
            {
                // Peak level for metering (unchanged).
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

                if (gain != 1.0f)
                {
                    // Gain-applied input, written into outputChannelData as
                    // scratch (unchanged from the pre-lift version).
                    int chans = std::min(numInputChannels, numOutputChannels);
                    for (int ch = 0; ch < chans; ++ch)
                        for (int i = 0; i < numSamples; ++i)
                            outputChannelData[ch][i] = inputChannelData[ch][i] * gain;

                    listened = outputChannelData;
                    listenedChans = chans;
                }
                else
                {
                    listened = inputChannelData;
                    listenedChans = numInputChannels;
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

            listened = outputChannelData;
            listenedChans = numOutputChannels;
        }

        // D10.1: read BEFORE advancing -- this is the "before this block"
        // reading AudioTap uses to capture firstSample on arm.
        const uint64_t deliveredBefore = deliveredSamples_.load(std::memory_order_relaxed);

        if (listened != nullptr && listenedChans > 0)
        {
            analysisCallback_.audioDeviceIOCallbackWithContext(
                nullptr, 0, const_cast<float* const*>(listened), listenedChans, numSamples, context);
        }

        // D10.1's second fan-out -- one tap call per block, UNCONDITIONALLY:
        // even when there is nothing to analyze this callback (listened ==
        // nullptr, e.g. mic mode with no input channels open), AudioTap
        // must still be told about this block. Skipping the call here would
        // advance deliveredSamples_ (below) without ever advancing the
        // tap's own framesWritten_ for the same span -- a silent desync
        // between the take's origin and its own audio file (s168 review's
        // "listened == nullptr" gap). AudioTap::push()'s existing
        // pad-with-silence path (writeFrames, AudioTap.cpp) already turns a
        // chans == 0 call into a full block of silence, so pushing silence
        // rather than skipping the advance is the fix: D10.1 says the
        // counter "increments ... at the top of every callback in both
        // modes" -- unconditionally -- so the counter's identity as THE
        // origin must not bend around this case; the tap's own bookkeeping
        // bends to match it instead, exactly as it already does for a gap
        // or a FIFO overrun.
        const uint32_t gapFrames = audioTap_.push(listened, listenedChans, numSamples, deliveredBefore, context);

        // D10.1: "incremented by numSamples at the top of every callback in
        // both modes (plus any inserted gap)". Done once, after computing
        // the gap so both parts of the advance land together.
        deliveredSamples_.fetch_add(static_cast<uint64_t>(numSamples) + gapFrames, std::memory_order_relaxed);

        // Silence output in all modes -- this app is analysis-only, no
        // audible output.
        for (int ch = 0; ch < numOutputChannels; ++ch)
            juce::FloatVectorOperations::clear(outputChannelData[ch], numSamples);
    }

    void audioDeviceAboutToStart(juce::AudioIODevice* device) override
    {
#if AUDIODNA_TEST_SERVER
        lastTicks_ = 0;   // s-rta-0929 asyncload: a restart's first callback measures no gap (callback quiesced here)
#endif
        player_.audioDeviceAboutToStart(device);
        analysisCallback_.audioDeviceAboutToStart(device);

        // D10.1: take the rate from the device (R13's pre-existing 48 kHz
        // hard-code elsewhere is NOT something to repeat here). Channel
        // count ASSUMPTION: the active output channel count -- AudioEngine
        // opens the device symmetric in/out (initialiseWithDefaultDevices
        // (2, 2)), so this matches what `listened`/`listenedChans` will
        // actually be in both modes in practice; push() defensively clamps
        // if a block's real chans ever disagrees (see AudioTap.cpp).
        const int channels = device != nullptr
            ? device->getActiveOutputChannels().countNumberOfSetBits()
            : 0;
        const int maxBlock = device != nullptr ? device->getCurrentBufferSizeSamples() : 0;
        const double rate = device != nullptr ? device->getCurrentSampleRate() : 0.0;
        audioTap_.prepare(rate, channels, maxBlock);
    }

    void audioDeviceStopped() override
    {
        player_.audioDeviceStopped();
        analysisCallback_.audioDeviceStopped();
    }

private:
    juce::AudioSourcePlayer& player_;
    AudioCallback& analysisCallback_;

    std::atomic<uint64_t> deliveredSamples_{0};
    AudioTap audioTap_;
#if AUDIODNA_TEST_SERVER
    int64_t lastTicks_ = 0;                        // the callback's own (and audioDeviceAboutToStart's, quiesced)
    std::atomic<int64_t> gapMaxTicks_{ 0 };
    std::atomic<uint64_t> callbacks_{ 0 };
    std::atomic<int> periodSamples_{ 0 };
    // s-rta-0929 asyncload (adoption AL8 b): every counter the audio callback touches is lock-free.
    static_assert(std::atomic<int64_t>::is_always_lock_free, "audio-callback witness must be lock-free");
    static_assert(std::atomic<uint64_t>::is_always_lock_free, "audio-callback witness must be lock-free");
    static_assert(std::atomic<int>::is_always_lock_free, "audio-callback witness must be lock-free");
#endif
};
