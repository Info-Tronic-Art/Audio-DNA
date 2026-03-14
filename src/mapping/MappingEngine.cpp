#include "MappingEngine.h"
#include "mapping/CurveTransforms.h"
#include <algorithm>
#include <cmath>

int MappingEngine::addMapping(const Mapping& mapping)
{
    mappings_.push_back(mapping);
    smoothers_.emplace_back(mapping.smoothing);
    return static_cast<int>(mappings_.size()) - 1;
}

bool MappingEngine::removeMapping(int index)
{
    if (index < 0 || index >= static_cast<int>(mappings_.size()))
        return false;

    mappings_.erase(mappings_.begin() + index);
    smoothers_.erase(smoothers_.begin() + index);
    return true;
}

Mapping* MappingEngine::getMapping(int index)
{
    if (index < 0 || index >= static_cast<int>(mappings_.size()))
        return nullptr;
    return &mappings_[static_cast<size_t>(index)];
}

const Mapping* MappingEngine::getMapping(int index) const
{
    if (index < 0 || index >= static_cast<int>(mappings_.size()))
        return nullptr;
    return &mappings_[static_cast<size_t>(index)];
}

void MappingEngine::clearAll()
{
    mappings_.clear();
    smoothers_.clear();
}

float MappingEngine::extractSource(MappingSource source, const FeatureSnapshot& snap)
{
    switch (source)
    {
        // Amplitude
        case MappingSource::RMS:              return snap.rms;
        case MappingSource::Peak:             return snap.peak;
        case MappingSource::RmsDB:            return snap.rmsDB;

        // Loudness
        case MappingSource::LUFS:             return snap.lufs;
        case MappingSource::DynamicRange:     return snap.dynamicRange;
        case MappingSource::TransientDensity: return snap.transientDensity;

        // Spectral
        case MappingSource::SpectralCentroid:  return snap.spectralCentroid;
        case MappingSource::SpectralFlux:      return snap.spectralFlux;
        case MappingSource::SpectralFlatness:  return snap.spectralFlatness;
        case MappingSource::SpectralRolloff:   return snap.spectralRolloff;

        // 7-Band energies
        case MappingSource::BandSub:          return snap.bandEnergies[0];
        case MappingSource::BandBass:         return snap.bandEnergies[1];
        case MappingSource::BandLowMid:       return snap.bandEnergies[2];
        case MappingSource::BandMid:          return snap.bandEnergies[3];
        case MappingSource::BandHighMid:      return snap.bandEnergies[4];
        case MappingSource::BandPresence:     return snap.bandEnergies[5];
        case MappingSource::BandBrilliance:   return snap.bandEnergies[6];

        // Onset / rhythm
        case MappingSource::OnsetStrength:    return snap.onsetStrength;
        case MappingSource::BeatPhase:        return snap.beatPhase;
        case MappingSource::BPM:              return snap.bpm;

        // Structural
        case MappingSource::StructuralState:  return static_cast<float>(snap.structuralState);

        // Pitch / harmony
        case MappingSource::DominantPitch:    return snap.dominantPitch;
        case MappingSource::PitchConfidence:  return snap.pitchConfidence;
        case MappingSource::DetectedKey:      return static_cast<float>(snap.detectedKey);
        case MappingSource::HarmonicChange:   return snap.harmonicChangeDetection;

        // MFCCs
        case MappingSource::MFCC0:  return snap.mfccs[0];
        case MappingSource::MFCC1:  return snap.mfccs[1];
        case MappingSource::MFCC2:  return snap.mfccs[2];
        case MappingSource::MFCC3:  return snap.mfccs[3];
        case MappingSource::MFCC4:  return snap.mfccs[4];
        case MappingSource::MFCC5:  return snap.mfccs[5];
        case MappingSource::MFCC6:  return snap.mfccs[6];
        case MappingSource::MFCC7:  return snap.mfccs[7];
        case MappingSource::MFCC8:  return snap.mfccs[8];
        case MappingSource::MFCC9:  return snap.mfccs[9];
        case MappingSource::MFCC10: return snap.mfccs[10];
        case MappingSource::MFCC11: return snap.mfccs[11];
        case MappingSource::MFCC12: return snap.mfccs[12];

        // Chroma
        case MappingSource::ChromaC:  return snap.chromagram[0];
        case MappingSource::ChromaCs: return snap.chromagram[1];
        case MappingSource::ChromaD:  return snap.chromagram[2];
        case MappingSource::ChromaDs: return snap.chromagram[3];
        case MappingSource::ChromaE:  return snap.chromagram[4];
        case MappingSource::ChromaF:  return snap.chromagram[5];
        case MappingSource::ChromaFs: return snap.chromagram[6];
        case MappingSource::ChromaG:  return snap.chromagram[7];
        case MappingSource::ChromaGs: return snap.chromagram[8];
        case MappingSource::ChromaA:  return snap.chromagram[9];
        case MappingSource::ChromaAs: return snap.chromagram[10];
        case MappingSource::ChromaB:  return snap.chromagram[11];

        case MappingSource::Count:    return 0.0f;
    }
    return 0.0f;
}

float MappingEngine::applyCurve(MappingCurve curve, float x, int steppedN)
{
    switch (curve)
    {
        case MappingCurve::Linear:      return CurveTransforms::linear(x);
        case MappingCurve::Exponential: return CurveTransforms::exponential(x);
        case MappingCurve::Logarithmic: return CurveTransforms::logarithmic(x);
        case MappingCurve::SCurve:      return CurveTransforms::sCurve(x);
        case MappingCurve::Stepped:     return CurveTransforms::stepped(x, steppedN);
        case MappingCurve::Count:       return CurveTransforms::linear(x);
    }
    return CurveTransforms::linear(x);
}

void MappingEngine::processFrame(const FeatureSnapshot& snapshot, EffectChain& chain)
{
    // First pass: reset all targeted parameters to their defaults,
    // so that summing works correctly when multiple mappings target
    // the same parameter.
    // We track which (effect, param) pairs have been written to avoid
    // resetting after the first mapping writes.
    // For simplicity, we accumulate into the effect params directly.

    // Reset targeted params to 0 before accumulation
    // Use a small local bitset — max 64 effects × 16 params = 1024 slots.
    // We'll use a simpler approach: just reset on first encounter.
    struct Target { uint32_t effect; uint32_t param; };
    // Pre-scan: collect unique targets and reset them
    for (const auto& m : mappings_)
    {
        if (!m.enabled)
            continue;
        auto* effect = chain.getEffect(static_cast<int>(m.targetEffectId));
        if (effect == nullptr)
            continue;
        if (static_cast<int>(m.targetParamIndex) >= effect->getNumParams())
            continue;
        // Reset to 0 for accumulation — we'll clamp after all mappings
        effect->getParam(static_cast<int>(m.targetParamIndex)).value = 0.0f;
    }

    // Second pass: accumulate mapping contributions
    for (size_t i = 0; i < mappings_.size(); ++i)
    {
        const auto& m = mappings_[i];
        if (!m.enabled)
            continue;

        auto* effect = chain.getEffect(static_cast<int>(m.targetEffectId));
        if (effect == nullptr)
            continue;
        if (static_cast<int>(m.targetParamIndex) >= effect->getNumParams())
            continue;

        // 1. Extract raw source value
        float raw = extractSource(m.source, snapshot);

        // 2. Normalize to [0, 1]
        float range = m.inputMax - m.inputMin;
        float normalized = (range > 1e-8f)
            ? std::clamp((raw - m.inputMin) / range, 0.0f, 1.0f)
            : 0.0f;

        // 3. Apply curve
        float curved = applyCurve(m.curve, normalized);

        // 4. Scale to output range
        float scaled = m.outputMin + curved * (m.outputMax - m.outputMin);

        // 5. Smooth
        // Update smoother alpha if it changed
        if (smoothers_[i].alpha() != m.smoothing)
            smoothers_[i].setAlpha(m.smoothing);
        float smoothed = smoothers_[i].process(scaled);

        // 6. Accumulate into target parameter
        effect->getParam(static_cast<int>(m.targetParamIndex)).value += smoothed;
    }

    // Final pass: clamp all targeted params to [0, 1]
    for (const auto& m : mappings_)
    {
        if (!m.enabled)
            continue;
        auto* effect = chain.getEffect(static_cast<int>(m.targetEffectId));
        if (effect == nullptr)
            continue;
        if (static_cast<int>(m.targetParamIndex) >= effect->getNumParams())
            continue;
        auto& param = effect->getParam(static_cast<int>(m.targetParamIndex));
        param.value = std::clamp(param.value, 0.0f, 1.0f);
    }
}
