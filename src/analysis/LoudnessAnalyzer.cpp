#include "LoudnessAnalyzer.h"
#include <cmath>
#include <algorithm>

LoudnessAnalyzer::LoudnessAnalyzer(int sampleRate)
    : windowSize_(static_cast<int>(sampleRate * 0.4))  // 400ms window
{
    // Pre-allocate circular buffer for sliding window
    windowBuffer_.resize(static_cast<size_t>(windowSize_), 0.0f);

    // K-weighting Stage 1: high-shelf boost (pre-filter)
    // Coefficients for 48kHz from ITU-R BS.1770
    stage1_.b0 = 1.53512485958697f;
    stage1_.b1 = -2.69169618940638f;
    stage1_.b2 = 1.19839281085285f;
    stage1_.a1 = -1.69065929318241f;
    stage1_.a2 = 0.73248077421585f;
    stage1_.s1 = 0.0f;
    stage1_.s2 = 0.0f;

    // K-weighting Stage 2: high-pass (revised low-frequency rolloff)
    // Coefficients for 48kHz from ITU-R BS.1770
    stage2_.b0 = 1.0f;
    stage2_.b1 = -2.0f;
    stage2_.b2 = 1.0f;
    stage2_.a1 = -1.99004745483398f;
    stage2_.a2 = 0.99007225036621f;
    stage2_.s1 = 0.0f;
    stage2_.s2 = 0.0f;
}

float LoudnessAnalyzer::processBiquad(BiquadState& state, float x)
{
    // Direct form II transposed
    float y = state.b0 * x + state.s1;
    state.s1 = state.b1 * x - state.a1 * y + state.s2;
    state.s2 = state.b2 * x - state.a2 * y;
    return y;
}

void LoudnessAnalyzer::process(const float* samples, int numSamples)
{
    float rawSumSquared = 0.0f;
    float rawPeak = 0.0f;

    for (int i = 0; i < numSamples; ++i)
    {
        float raw = samples[i];

        // Track raw signal statistics for RMS dB and dynamic range
        rawSumSquared += raw * raw;
        float absSample = std::fabs(raw);
        if (absSample > rawPeak)
            rawPeak = absSample;

        // Apply K-weighting: stage 1 (high-shelf) then stage 2 (high-pass)
        float kWeighted = processBiquad(stage1_, raw);
        kWeighted = processBiquad(stage2_, kWeighted);

        float squared = kWeighted * kWeighted;

        // Update circular buffer: subtract old value, add new
        auto pos = static_cast<size_t>(writePos_);
        runningSum_ -= static_cast<double>(windowBuffer_[pos]);
        windowBuffer_[pos] = squared;
        runningSum_ += static_cast<double>(squared);

        writePos_ = (writePos_ + 1) % windowSize_;
        if (fillCount_ < windowSize_)
            ++fillCount_;
    }

    // Clamp running sum to avoid negative due to floating-point drift
    if (runningSum_ < 0.0)
        runningSum_ = 0.0;

    // --- LUFS ---
    if (fillCount_ > 0)
    {
        double meanSquare = runningSum_ / static_cast<double>(fillCount_);
        if (meanSquare > 1e-20)
            lufs_ = static_cast<float>(-0.691 + 10.0 * std::log10(meanSquare));
        else
            lufs_ = -100.0f;
    }

    // --- RMS dB (from raw signal) ---
    if (numSamples > 0)
    {
        float rms = std::sqrt(rawSumSquared / static_cast<float>(numSamples));
        if (rms > 1e-10f)
            rmsDB_ = 20.0f * std::log10(rms);
        else
            rmsDB_ = -100.0f;

        // --- Dynamic Range (crest factor: peak / RMS) ---
        if (rms > 1e-10f)
            dynamicRange_ = rawPeak / rms;
        else
            dynamicRange_ = 0.0f;
    }
}
