#include "StructuralDetector.h"

StructuralDetector::StructuralDetector(float sampleRate, int hopSize)
{
    // Time constants in seconds for each scale
    constexpr float timeScales[kNumScales] = { 0.1f, 1.0f, 4.0f, 16.0f };

    for (int i = 0; i < kNumScales; ++i)
    {
        float alpha = static_cast<float>(hopSize) / (sampleRate * timeScales[i]);
        // Clamp alpha to [0, 1]
        if (alpha > 1.0f) alpha = 1.0f;

        rmsAlpha_[i]  = alpha;
        fluxAlpha_[i] = alpha;

        rmsEnv_[i]  = 0.0f;
        fluxEnv_[i] = 0.0f;
    }

    // Hysteresis: ~200ms worth of hops
    holdThreshold_ = static_cast<int>((sampleRate * 0.2f) / static_cast<float>(hopSize));
    if (holdThreshold_ < 1)
        holdThreshold_ = 1;
}

void StructuralDetector::process(float rms, float spectralFlux, float onsetRate)
{
    // Update all EMA envelopes
    for (int i = 0; i < kNumScales; ++i)
    {
        rmsEnv_[i]  = rmsAlpha_[i]  * rms          + (1.0f - rmsAlpha_[i])  * rmsEnv_[i];
        fluxEnv_[i] = fluxAlpha_[i] * spectralFlux + (1.0f - fluxAlpha_[i]) * fluxEnv_[i];
    }

    // Classify raw state
    uint8_t rawState = classifyState(onsetRate);

    // Hysteresis: require consistent state for holdThreshold_ hops
    if (rawState == candidateState_)
    {
        ++holdCounter_;
        if (holdCounter_ >= holdThreshold_)
            confirmedState_ = candidateState_;
    }
    else
    {
        candidateState_ = rawState;
        holdCounter_ = 1;
    }
}

uint8_t StructuralDetector::classifyState(float onsetRate) const
{
    float fast = rmsEnv_[kFast];
    float slow = rmsEnv_[kSlow];

    float fluxFast = fluxEnv_[kFast];
    float fluxSlow = fluxEnv_[kSlow];

    // Guard against a near-silent slow envelope. Was 1e-8f (~-160 dBFS, five
    // orders of magnitude below any real microphone noise floor -- a real
    // room measured ~0.0053 RMS, i.e. always past that guard), so it never
    // actually caught anything but true digital silence (e.g. before input
    // starts flowing). Raised to nearSilentRmsThreshold_ so it's in the same
    // range as BPMTracker's own silence threshold. Strict `<`, matching
    // BPMTracker::feedSilenceDetection()'s `rms < silenceRmsThreshold_`: at
    // exactly the threshold value, both components agree it's NOT silent, so
    // this guard and BPMTracker's inSilence_ decision never disagree right at
    // the boundary. This is still just a numerical-stability guard, not the
    // fix for ambient noise flipping structuralState during quiet playback --
    // that's handled at the BPMTracker::updatePhrase() call site by gating
    // the phrase reset on predictedBeatRegime_, since a slow-envelope
    // threshold here can't distinguish "genuinely quiet music" from "room
    // noise with no music at all".
    if (slow < nearSilentRmsThreshold_)
        return kNormal;

    // DROP: energy well above average AND high onset rate
    if (fast > slow * 1.8f && onsetRate > onsetRateThreshold_)
        return kDrop;

    // BUILDUP: energy rising AND spectral flux rising
    if (fast > slow * 1.3f && fluxFast > fluxSlow * 1.2f)
        return kBuildup;

    // BREAKDOWN: energy well below average
    if (fast < slow * 0.5f)
        return kBreakdown;

    return kNormal;
}
