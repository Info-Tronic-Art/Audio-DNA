#include "GenreDetector.h"
#include <cmath>
#include <algorithm>
#include <numeric>

GenreDetector::GenreDetector(float sampleRate, int hopSize)
{
    float hopsPerSecond = sampleRate / static_cast<float>(hopSize);

    // EMA alpha for ~2 second smoothing window
    // alpha = 1 - exp(-1 / (tau * hopsPerSecond)), tau = 2.0s
    smoothAlpha_ = 1.0f - std::exp(-1.0f / (2.0f * hopsPerSecond));

    // Hysteresis: ~3 seconds of consistency before switching genre
    holdThreshold_ = static_cast<int>(3.0f * hopsPerSecond);

    // Initialize scores
    rawScores_.fill(0.0f);
    smoothedScores_.fill(0.0f);
    // Start with slight pop/electronic bias
    smoothedScores_[kPopElectronic] = 0.3f;
}

void GenreDetector::process(const Features& features)
{
    computeScores(features);
    smoothScores();
    classify();
    computeEnergyState(features);
}

void GenreDetector::computeScores(const Features& features)
{
    rawScores_.fill(0.0f);

    float bpm = features.bpm;
    float bass = features.bandEnergies[0] + features.bandEnergies[1]; // Sub + Bass
    float mids = features.bandEnergies[2] + features.bandEnergies[3]; // LowMid + Mid
    float highs = features.bandEnergies[4] + features.bandEnergies[5] + features.bandEnergies[6];
    float chromComplexity = chromaticComplexity(features.chromagram);
    bool bpmLocked = (features.trackerState == 2);

    // Only score BPM-dependent features when BPM is locked
    float bpmWeight = bpmLocked ? 1.0f : 0.0f;

    // === House: 120-130 BPM, strong steady bass, moderate transient density ===
    {
        float& s = rawScores_[kHouse];
        // BPM range scoring (bell curve centered at 125)
        if (bpmLocked && bpm >= 115.0f && bpm <= 135.0f)
        {
            float bpmDist = std::fabs(bpm - 125.0f) / 10.0f;
            s += (1.0f - bpmDist * bpmDist) * 0.35f * bpmWeight;
        }
        // Strong bass
        if (bass > 0.4f) s += std::min(bass, 1.0f) * 0.25f;
        // Moderate transient density (4-on-floor = predictable, not chaotic)
        if (features.transientDensity > 2.0f && features.transientDensity < 8.0f)
            s += 0.15f;
        // Moderate spectral centroid (not too bright)
        if (features.spectralCentroid > 1500.0f && features.spectralCentroid < 4500.0f)
            s += 0.1f;
        // Low chromatic complexity (simpler harmony)
        if (chromComplexity < 5.0f) s += 0.1f;
    }

    // === Techno: 125-145 BPM, high transient density, high spectral flux ===
    {
        float& s = rawScores_[kTechno];
        if (bpmLocked && bpm >= 120.0f && bpm <= 150.0f)
        {
            float bpmDist = std::fabs(bpm - 135.0f) / 15.0f;
            s += (1.0f - bpmDist * bpmDist) * 0.3f * bpmWeight;
        }
        if (features.transientDensity > 5.0f)
            s += std::min(features.transientDensity / 15.0f, 0.3f);
        if (features.spectralFlux > 0.3f)
            s += std::min(features.spectralFlux * 0.5f, 0.2f);
        // Strong bass
        if (bass > 0.3f) s += 0.1f;
        // Low chromatic complexity (repetitive)
        if (chromComplexity < 4.0f) s += 0.1f;
    }

    // === DnB: 160-180 BPM, heavy bass, complex rhythm ===
    {
        float& s = rawScores_[kDnB];
        if (bpmLocked && bpm >= 155.0f && bpm <= 185.0f)
        {
            float bpmDist = std::fabs(bpm - 172.0f) / 15.0f;
            s += (1.0f - bpmDist * bpmDist) * 0.4f * bpmWeight;
        }
        // Heavy sub-bass
        if (features.bandEnergies[0] > 0.3f) s += 0.2f;
        // High transient density (breakbeats)
        if (features.transientDensity > 6.0f) s += 0.15f;
        // High spectral flux (energetic)
        if (features.spectralFlux > 0.4f) s += 0.1f;
    }

    // === Hip-Hop: 80-100 BPM, strong bass + mids, groovy ===
    {
        float& s = rawScores_[kHipHop];
        if (bpmLocked && bpm >= 75.0f && bpm <= 105.0f)
        {
            float bpmDist = std::fabs(bpm - 90.0f) / 15.0f;
            s += (1.0f - bpmDist * bpmDist) * 0.35f * bpmWeight;
        }
        // Strong bass and mids
        if (bass > 0.3f) s += std::min(bass * 0.3f, 0.2f);
        if (mids > 0.3f) s += std::min(mids * 0.2f, 0.15f);
        // Moderate transient density
        if (features.transientDensity > 2.0f && features.transientDensity < 7.0f)
            s += 0.1f;
        // Dynamic range (punchy beats)
        if (features.dynamicRange > 3.0f) s += 0.1f;
    }

    // === Ambient: low transient density, high spectral flatness, sustained ===
    {
        float& s = rawScores_[kAmbient];
        // Very low transient density
        if (features.transientDensity < 2.0f) s += 0.3f;
        else if (features.transientDensity < 4.0f) s += 0.1f;
        // High spectral flatness (noise-like or pad-like)
        if (features.spectralFlatness > 0.3f)
            s += std::min(features.spectralFlatness * 0.4f, 0.25f);
        // Low spectral centroid (dark/warm)
        if (features.spectralCentroid < 2000.0f) s += 0.15f;
        // Low RMS (quiet)
        if (features.rms < 0.15f) s += 0.15f;
        // Low spectral flux (sustained tones, not transient)
        if (features.spectralFlux < 0.2f) s += 0.1f;
    }

    // === Rock: broad spectrum, high peak levels, mid-heavy ===
    {
        float& s = rawScores_[kRock];
        if (bpmLocked && bpm >= 95.0f && bpm <= 150.0f)
        {
            s += 0.15f * bpmWeight;
        }
        // High dynamic range (loud drums vs. quieter parts)
        if (features.dynamicRange > 4.0f) s += 0.2f;
        // High RMS
        if (features.rms > 0.3f) s += 0.15f;
        // Mid-heavy (guitar/vocal frequencies)
        if (mids > 0.4f) s += 0.2f;
        // Moderate high end (cymbals/hi-hats)
        if (highs > 0.2f && highs < 0.6f) s += 0.1f;
        // Higher harmonic change (chord progressions)
        if (features.harmonicChangeDetection > 0.3f) s += 0.1f;
    }

    // === Pop/Electronic: mid-range BPM, bright, high centroid ===
    {
        float& s = rawScores_[kPopElectronic];
        if (bpmLocked && bpm >= 100.0f && bpm <= 140.0f)
        {
            s += 0.15f * bpmWeight;
        }
        // High spectral centroid (bright production)
        if (features.spectralCentroid > 3000.0f)
            s += std::min((features.spectralCentroid - 3000.0f) / 10000.0f, 0.2f);
        // Moderate transient density
        if (features.transientDensity > 3.0f && features.transientDensity < 10.0f)
            s += 0.1f;
        // Full spectrum presence
        if (bass > 0.2f && mids > 0.2f && highs > 0.15f) s += 0.15f;
        // Moderate spectral flux
        if (features.spectralFlux > 0.15f && features.spectralFlux < 0.5f) s += 0.1f;
    }

    // === Jazz/Other: complex harmony, variable rhythm, chromatic ===
    {
        float& s = rawScores_[kJazzOther];
        // High chromatic complexity (many pitch classes active)
        if (chromComplexity > 6.0f) s += 0.3f;
        else if (chromComplexity > 4.0f) s += 0.15f;
        // High harmonic change rate
        if (features.harmonicChangeDetection > 0.4f) s += 0.2f;
        // Moderate spectral flatness (tonal variety)
        if (features.spectralFlatness > 0.2f && features.spectralFlatness < 0.6f)
            s += 0.1f;
        // Variable dynamics
        if (features.dynamicRange > 3.0f) s += 0.1f;
        // Mid-frequency presence (acoustic instruments)
        if (mids > 0.3f) s += 0.1f;
    }

    // Normalize scores so they sum to 1
    float totalScore = 0.0f;
    for (float score : rawScores_)
        totalScore += score;

    if (totalScore > 0.0f)
    {
        for (auto& score : rawScores_)
            score /= totalScore;
    }
    else
    {
        // No clear signal — default to uniform
        for (auto& score : rawScores_)
            score = 1.0f / kNumGenres;
    }
}

void GenreDetector::smoothScores()
{
    for (int i = 0; i < kNumGenres; ++i)
    {
        smoothedScores_[static_cast<size_t>(i)] +=
            smoothAlpha_ * (rawScores_[static_cast<size_t>(i)] - smoothedScores_[static_cast<size_t>(i)]);
    }
}

void GenreDetector::classify()
{
    // Find the genre with the highest smoothed score
    int bestGenre = 0;
    float bestScore = smoothedScores_[0];
    for (int i = 1; i < kNumGenres; ++i)
    {
        if (smoothedScores_[static_cast<size_t>(i)] > bestScore)
        {
            bestScore = smoothedScores_[static_cast<size_t>(i)];
            bestGenre = i;
        }
    }

    // Confidence: ratio of best score to second-best
    float secondBest = 0.0f;
    for (int i = 0; i < kNumGenres; ++i)
    {
        if (i != bestGenre && smoothedScores_[static_cast<size_t>(i)] > secondBest)
            secondBest = smoothedScores_[static_cast<size_t>(i)];
    }

    // Confidence is how much the top genre dominates
    if (bestScore > 0.0f && secondBest > 0.0f)
        confidence_ = (bestScore - secondBest) / bestScore;
    else if (bestScore > 0.0f)
        confidence_ = 1.0f;
    else
        confidence_ = 0.0f;

    confidence_ = std::clamp(confidence_, 0.0f, 1.0f);

    // Hysteresis: must sustain for holdThreshold_ hops before switching
    auto candidate = static_cast<uint8_t>(bestGenre);
    if (candidate == confirmedGenre_)
    {
        // Already confirmed — reset candidate tracking
        candidateGenre_ = candidate;
        holdCounter_ = 0;
    }
    else if (candidate == candidateGenre_)
    {
        // Same candidate — keep counting
        ++holdCounter_;
        if (holdCounter_ >= holdThreshold_ && confidence_ > 0.15f)
        {
            confirmedGenre_ = candidate;
            holdCounter_ = 0;
        }
    }
    else
    {
        // New candidate — restart counter
        candidateGenre_ = candidate;
        holdCounter_ = 1;
    }
}

void GenreDetector::computeEnergyState(const Features& features)
{
    // Compute an energy value from RMS + transient density + spectral flux
    float energy = features.rms * 0.4f
                 + std::min(features.transientDensity / 10.0f, 1.0f) * 0.3f
                 + std::min(features.spectralFlux, 1.0f) * 0.3f;

    // EMA smoothing
    energyEMA_ += smoothAlpha_ * (energy - energyEMA_);

    // Classify with hysteresis bands
    if (energyEMA_ < 0.2f)
        energyState_ = 0; // Low
    else if (energyEMA_ > 0.5f)
        energyState_ = 2; // High
    else
        energyState_ = 1; // Medium
}

float GenreDetector::chromaticComplexity(const float chromagram[12]) const
{
    // Count how many pitch classes have significant energy
    float count = 0.0f;
    float threshold = 1.0f / 24.0f; // Half of uniform distribution (1/12)
    for (int i = 0; i < 12; ++i)
    {
        if (chromagram[i] > threshold)
            count += 1.0f;
    }
    return count;
}

const char* GenreDetector::genreName(uint8_t genre)
{
    static const char* names[] = {
        "House", "Techno", "Drum & Bass", "Hip-Hop",
        "Ambient", "Rock", "Pop/Electronic", "Jazz/Other"
    };
    if (genre < kNumGenres)
        return names[genre];
    return "Unknown";
}
