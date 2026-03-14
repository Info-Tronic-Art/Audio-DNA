#pragma once
#include <array>
#include <cmath>
#include <cstdint>

// Krumhansl-Schmuckler key detection from a 12-element chroma profile.
// Pre-allocates all storage at construction — zero allocation in steady state.
//
// Feed a chroma vector (C=0 .. B=11) per hop via process().
// Uses EMA-style hysteresis: the reported key only changes after a new key
// has been dominant for several consecutive frames (default 10).
//
// Outputs:
//   - detectedKey:  0=C, 1=C#, ..., 11=B, -1=unknown
//   - isMajor:      true if major, false if minor
//   - confidence:   correlation strength mapped to [0, 1]
class KeyDetector
{
public:
    KeyDetector();

    // Feed a 12-element chroma array (C=0..B=11, values >= 0).
    // After this call, query detectedKey(), isMajor(), confidence().
    void process(const float* chroma);

    // --- Accessors (valid after process()) ---
    int   detectedKey() const { return reportedKey_; }
    bool  isMajor()     const { return reportedMajor_; }
    float confidence()  const { return confidence_; }

private:
    // Pearson correlation between two 12-element arrays
    static float pearsonCorrelation(const float* x, const float* y);

    // Krumhansl-Kessler key profiles
    static constexpr std::array<float, 12> majorProfile_ = {
        6.35f, 2.23f, 3.48f, 2.33f, 4.38f, 4.09f,
        2.52f, 5.19f, 2.39f, 3.66f, 2.29f, 2.88f
    };
    static constexpr std::array<float, 12> minorProfile_ = {
        6.33f, 2.68f, 3.52f, 5.38f, 2.60f, 3.53f,
        2.54f, 4.75f, 3.98f, 2.69f, 3.34f, 3.17f
    };

    // Pre-allocated buffer for rotated template
    std::array<float, 12> rotated_{};

    // Current raw detection (before hysteresis)
    int  rawKey_   = -1;
    bool rawMajor_ = true;

    // Reported (smoothed) detection
    int   reportedKey_   = -1;
    bool  reportedMajor_ = true;
    float confidence_    = 0.0f;

    // Hysteresis: require N consecutive frames of same key to switch
    static constexpr int kHysteresisFrames = 10;
    int  candidateKey_   = -1;
    bool candidateMajor_ = true;
    int  candidateCount_ = 0;
};
