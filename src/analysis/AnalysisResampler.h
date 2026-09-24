#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <array>
#include <cstdint>
#include "audio/RingBuffer.h"

// R13: converts the device-rate mono stream in the analysis ring buffer to the
// fixed internal analysis rate (48 kHz). Lives on the analysis thread only.
// Zero heap after construction; a rate change (device hot-swap) resets state
// in O(1) with no allocation. When the source rate IS 48 kHz (or unknown, 0)
// pullHop() is a plain ring pop — bit-identical to the pre-R13 behaviour.
class AnalysisResampler
{
public:
    static constexpr double kTargetRate      = 48000.0;
    static constexpr int    kMaxOutputHop    = 512;              // == AnalysisThread::kHopSize
    static constexpr int    kMaxRatio        = 8;                // source rates up to 384 kHz
    static constexpr int    kStagingCapacity = kMaxOutputHop * kMaxRatio + 64;   // 4160 floats

    using Interpolator = juce::LagrangeInterpolator;         // ONE-LINE swap point (WindowedSincInterpolator if ever needed)

    // Analysis thread only. `hz <= 0` means unknown -> bypass (treated as 48 kHz).
    // Reconfigures ratio, anti-alias filter, and resets interpolator + staging when the rate changes.
    void setSourceRate(double hz);
    double sourceRate() const        { return sourceRate_; }
    double ratio() const             { return ratio_; }        // source samples per output sample
    bool   isBypass() const          { return bypass_; }
    float  inputBandwidthHz() const;                            // min(sourceRate/2, kTargetRate/2); 24000 in bypass
    int    inputNeededFor(int numOut) const;                    // (int)ceil(ratio_*numOut) + 2 (>= JUCE's floor(1+ratio*numOut) bound, G17)

    // Produce exactly numOut samples at 48 kHz from `ring`. Returns false (and
    // produces nothing) when the ring does not yet hold enough source samples;
    // the caller sleeps and retries. NOTE: may have staged (popped) some source
    // samples before returning false — those are kept for the next call.
    bool pullHop(RingBuffer<float>& ring, float* out, int numOut);

private:
    struct Biquad
    {
        float b0 = 0, b1 = 0, b2 = 0, a1 = 0, a2 = 0, s1 = 0, s2 = 0;
        float tick(float x);
        void reset();
    };
    void configureLowpass();   // RBJ LPF, fc = 21600 Hz (relative to the source rate); Q = {0.54120, 1.30656}; only used when ratio_ > 1

    double sourceRate_ = 0.0, ratio_ = 1.0;
    bool   bypass_ = true, lowpassActive_ = false;
    Interpolator interp_;
    Biquad lp1_, lp2_;
    std::array<float, kStagingCapacity> staging_{};
    int staged_ = 0;
};
