#pragma once
#include <array>
#include <cstddef>
#include <vector>

// Computes momentary LUFS (ITU-R BS.1770) and dynamic range from raw audio.
// All buffers are pre-allocated at construction — zero allocation in steady state.
//
// Algorithm:
//   1. K-weighting filter: two cascaded biquad stages (high-shelf + high-pass)
//   2. Accumulate squared K-weighted samples in a 400ms sliding window
//   3. LUFS = -0.691 + 10 * log10(meanSquare)
//   4. Dynamic range = crest factor (peak / RMS) from raw signal
class LoudnessAnalyzer
{
public:
    // Construct with sample rate (e.g. 48000).
    // Pre-allocates the 400ms circular buffer.
    explicit LoudnessAnalyzer(int sampleRate);

    // Feed raw time-domain audio samples. Call once per analysis hop.
    void process(const float* samples, int numSamples);

    // --- Accessors (valid after process()) ---
    float lufs()         const { return lufs_; }
    float rmsDB()        const { return rmsDB_; }
    float dynamicRange() const { return dynamicRange_; }

private:
    // Biquad filter state (direct form II transposed)
    struct BiquadState
    {
        float b0, b1, b2;
        float a1, a2;
        float s1 = 0.0f;
        float s2 = 0.0f;
    };

    float processBiquad(BiquadState& state, float x);

    int sampleRate_;
    int windowSize_;  // 400ms in samples

    // K-weighting biquad stages
    BiquadState stage1_{};  // high-shelf boost
    BiquadState stage2_{};  // high-pass rolloff

    // Circular buffer for 400ms sliding window of squared K-weighted samples
    std::vector<float> windowBuffer_;
    int writePos_ = 0;
    int fillCount_ = 0;
    double runningSum_ = 0.0;

    // Results
    float lufs_         = -100.0f;
    float rmsDB_        = -100.0f;
    float dynamicRange_ = 0.0f;
};
