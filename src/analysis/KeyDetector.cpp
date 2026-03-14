#include "KeyDetector.h"

KeyDetector::KeyDetector()
{
    rotated_.fill(0.0f);
}

float KeyDetector::pearsonCorrelation(const float* x, const float* y)
{
    float mx = 0.0f, my = 0.0f;
    for (int i = 0; i < 12; ++i)
    {
        mx += x[i];
        my += y[i];
    }
    mx /= 12.0f;
    my /= 12.0f;

    float num = 0.0f;
    float dx2 = 0.0f;
    float dy2 = 0.0f;
    for (int i = 0; i < 12; ++i)
    {
        float dx = x[i] - mx;
        float dy = y[i] - my;
        num += dx * dy;
        dx2 += dx * dx;
        dy2 += dy * dy;
    }

    float denom = std::sqrt(dx2 * dy2);
    if (denom < 1e-12f)
        return 0.0f;

    return num / denom;
}

void KeyDetector::process(const float* chroma)
{
    float bestCorr = -2.0f;
    int   bestKey  = -1;
    bool  bestMajor = true;

    // Try all 24 keys (12 major + 12 minor)
    for (int key = 0; key < 12; ++key)
    {
        // Rotate the major template by key semitones
        for (int i = 0; i < 12; ++i)
            rotated_[i] = majorProfile_[(i + key) % 12];

        float corr = pearsonCorrelation(chroma, rotated_.data());
        if (corr > bestCorr)
        {
            bestCorr  = corr;
            bestKey   = key;
            bestMajor = true;
        }

        // Rotate the minor template by key semitones
        for (int i = 0; i < 12; ++i)
            rotated_[i] = minorProfile_[(i + key) % 12];

        corr = pearsonCorrelation(chroma, rotated_.data());
        if (corr > bestCorr)
        {
            bestCorr  = corr;
            bestKey   = key;
            bestMajor = false;
        }
    }

    // Map correlation from [-1, 1] to [0, 1]
    confidence_ = (bestCorr + 1.0f) * 0.5f;

    // If correlation is very weak, report unknown
    if (bestCorr < 0.1f)
    {
        rawKey_   = -1;
        rawMajor_ = true;
    }
    else
    {
        rawKey_   = bestKey;
        rawMajor_ = bestMajor;
    }

    // Hysteresis: only change reported key after N consecutive frames
    // of the same raw detection
    if (rawKey_ == candidateKey_ && rawMajor_ == candidateMajor_)
    {
        ++candidateCount_;
    }
    else
    {
        candidateKey_   = rawKey_;
        candidateMajor_ = rawMajor_;
        candidateCount_ = 1;
    }

    if (candidateCount_ >= kHysteresisFrames)
    {
        reportedKey_   = candidateKey_;
        reportedMajor_ = candidateMajor_;
    }
    // If raw is unknown and sustained, also report unknown
    if (rawKey_ == -1 && candidateCount_ >= kHysteresisFrames)
    {
        reportedKey_ = -1;
    }
}
