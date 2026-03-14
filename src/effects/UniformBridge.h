#pragma once
#include "effects/EffectChain.h"
#include "analysis/FeatureSnapshot.h"

// UniformBridge: applies audio feature values to effect parameters.
//
// For M3, this uses hardcoded demo mappings:
//   - RMS → ripple intensity
//   - Spectral centroid → hue shift amount
//   - Onset strength → RGB split amount
//   - Bass energy → vignette intensity
//
// In M4, this will be replaced by the full MappingEngine.
class UniformBridge
{
public:
    UniformBridge() = default;

    // Apply audio features to effect parameters via hardcoded mappings.
    // Call this each frame before rendering the effect chain.
    void applyDemoMappings(EffectChain& chain, const FeatureSnapshot& snap);

private:
    // Smoothed values (simple EMA)
    float smoothedRMS_ = 0.0f;
    float smoothedCentroid_ = 0.0f;
    float smoothedOnset_ = 0.0f;
    float smoothedBass_ = 0.0f;

    static constexpr float kSmoothAlpha = 0.15f;       // Responsive
    static constexpr float kOnsetSmoothAlpha = 0.3f;    // Faster for transients
    static constexpr float kOnsetDecayAlpha = 0.05f;    // Slow decay for onset
};
