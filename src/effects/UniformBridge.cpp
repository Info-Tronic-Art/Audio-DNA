#include "UniformBridge.h"
#include <algorithm>
#include <cmath>

void UniformBridge::applyDemoMappings(EffectChain& chain, const FeatureSnapshot& snap)
{
    // --- Smooth the raw audio features ---

    // RMS: smooth for steady response
    smoothedRMS_ += kSmoothAlpha * (snap.rms - smoothedRMS_);

    // Spectral centroid: normalize from Hz to [0,1]
    // Typical range 200-8000 Hz → map to [0,1]
    float normCentroid = std::clamp((snap.spectralCentroid - 200.0f) / 7800.0f, 0.0f, 1.0f);
    smoothedCentroid_ += kSmoothAlpha * (normCentroid - smoothedCentroid_);

    // Onset strength: fast attack, slow decay for punch
    float targetOnset = snap.onsetStrength;
    if (targetOnset > smoothedOnset_)
        smoothedOnset_ += kOnsetSmoothAlpha * (targetOnset - smoothedOnset_);
    else
        smoothedOnset_ += kOnsetDecayAlpha * (targetOnset - smoothedOnset_);
    smoothedOnset_ = std::clamp(smoothedOnset_, 0.0f, 1.0f);

    // Bass energy (bandEnergies[1] = 60-250 Hz)
    smoothedBass_ += kSmoothAlpha * (snap.bandEnergies[1] - smoothedBass_);

    // --- Apply to effects by index (order matches how we add them) ---

    // Effect 0: Ripple — RMS drives intensity, bass drives frequency
    if (auto* ripple = chain.getEffect(0))
    {
        ripple->setParamValue(0, smoothedRMS_);            // u_ripple_intensity
        ripple->setParamValue(1, smoothedBass_);           // u_ripple_freq
        ripple->setParamValue(2, 0.5f);                    // u_ripple_speed (constant)
    }

    // Effect 1: Hue Shift — spectral centroid drives hue rotation
    if (auto* hueShift = chain.getEffect(1))
    {
        hueShift->setParamValue(0, smoothedCentroid_);     // u_hue_shift
    }

    // Effect 2: RGB Split — onset strength drives split amount
    if (auto* rgbSplit = chain.getEffect(2))
    {
        rgbSplit->setParamValue(0, smoothedOnset_);        // u_rgb_split
        rgbSplit->setParamValue(1, 0.0f);                  // u_rgb_angle (horizontal)
    }

    // Effect 3: Vignette — bass drives vignette intensity
    if (auto* vignette = chain.getEffect(3))
    {
        vignette->setParamValue(0, smoothedBass_);         // u_vignette_int
        vignette->setParamValue(1, 0.6f);                  // u_vignette_soft (moderate)
    }
}
