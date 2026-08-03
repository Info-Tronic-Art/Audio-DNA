#include "MappingEngine.h"
#include "mapping/CurveTransforms.h"
#include <algorithm>
#include <cmath>

int MappingEngine::addMapping(const Mapping& mapping)
{
    // A6 (outputwindow-arc-design.md): mappings_/smoothers_ are confined to
    // the message thread (see processFrame). Debug-only; would have caught
    // the GL-thread clearAll this arc deletes.
    jassert(juce::MessageManager::getInstance()->isThisTheMessageThread());

    mappings_.push_back(mapping);
    smoothers_.emplace_back(mapping.smoothing);
    return static_cast<int>(mappings_.size()) - 1;
}

bool MappingEngine::removeMapping(int index)
{
    jassert(juce::MessageManager::getInstance()->isThisTheMessageThread()); // A6

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
    jassert(juce::MessageManager::getInstance()->isThisTheMessageThread()); // A6

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
        case MappingSource::BarPhase:         return snap.barPhase;
        case MappingSource::PhrasePhase:      return snap.phrasePhase;
        case MappingSource::BarCount:         return static_cast<float>(snap.barCount);

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

        // Advanced audio (P25)
        case MappingSource::SidechainPump:    return snap.sidechainPump;
        case MappingSource::SwingRatio:       return snap.swingRatio;
        case MappingSource::FormantPresence:  return snap.formantPresence;
        case MappingSource::ResonancePeak:    return snap.resonancePeak;
        case MappingSource::ReeseBass:        return snap.reeseBass;

        case MappingSource::Count:    return 0.0f;
    }
    return 0.0f;
}

float MappingEngine::applyCurve(MappingCurve curve, float x, int steppedN)
{
    return CurveTransforms::applyCurve(static_cast<int>(curve), x, steppedN);
}

void MappingEngine::processFrame(const FeatureSnapshot& snapshot, EffectChain& chain)
{
    // A6 (outputwindow-arc-design.md): confined to the message thread since
    // W5 (MainComponent's MappingTickTimer, kMappingTickHz) is the sole
    // caller — no other thread may run processFrame concurrently with
    // addMapping/removeMapping/clearAll.
    jassert(juce::MessageManager::getInstance()->isThisTheMessageThread());

    // Early exit if no active mappings
    if (mappings_.empty()) return;

    // Single-store discipline (outputwindow-arc-design.md W4/A3): the old
    // shape here was three stores per param per tick — reset-to-0, then
    // += accumulate, then clamp. Both GL renderers upload
    // EffectParam::value live (plain float, the A2 known-deferred
    // crossing), so the intermediate zero and half-sums were uploadable
    // mid-tick — a real, pre-existing zero-flash. Contributions are now
    // summed LOCALLY and written back in exactly ONE clamped store per
    // targeted param per tick.
    //
    // Grouping is an owner scan: the first enabled mapping targeting a
    // param owns it and folds in every later mapping with the same target.
    // O(M^2) over the mapping count (small), allocation-free, and it
    // preserves the old code's smoother tick set/inputs and per-target
    // float-add order — end-of-tick values are bit-identical (see
    // test_mapping_engine "single-store" equivalence cases).
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

        // Owner check: an earlier enabled mapping with the same target
        // already folded this mapping's contribution into its store.
        // Review minor 1: each mapping's `enabled` flag is read twice per
        // tick (here, and again in the sum loop below) with no lock between
        // the two reads; a concurrent flip of `enabled` mid-processFrame
        // could disagree between the two reads and double-tick one
        // smoother. Unreachable once A6 message-thread confinement holds —
        // processFrame and every `enabled` writer then run on the same
        // (message) thread, so no flip can land between the two reads.
        bool ownedEarlier = false;
        for (size_t j = 0; j < i; ++j)
        {
            const auto& prev = mappings_[j];
            if (prev.enabled
                && prev.targetEffectId == m.targetEffectId
                && prev.targetParamIndex == m.targetParamIndex)
            {
                ownedEarlier = true;
                break;
            }
        }
        if (ownedEarlier)
            continue;

        // Sum contributions from mapping i and every later enabled mapping
        // with the same target. Same-target mappings resolve to the same
        // effect/param, so validity was already established above.
        float sum = 0.0f;
        for (size_t k = i; k < mappings_.size(); ++k)
        {
            const auto& mk = mappings_[k];
            if (!mk.enabled
                || mk.targetEffectId != m.targetEffectId
                || mk.targetParamIndex != m.targetParamIndex)
                continue;

            // 1. Extract raw source value
            float raw = extractSource(mk.source, snapshot);

            // 2. Normalize to [0, 1]
            float range = mk.inputMax - mk.inputMin;
            float normalized = (range > 1e-8f)
                ? std::clamp((raw - mk.inputMin) / range, 0.0f, 1.0f)
                : 0.0f;

            // 3. Apply curve
            float curved = applyCurve(mk.curve, normalized);

            // 4. Scale to output range
            float scaled = mk.outputMin + curved * (mk.outputMax - mk.outputMin);

            // 5. Smooth
            // Update smoother alpha if it changed
            if (smoothers_[k].alpha() != mk.smoothing)
                smoothers_[k].setAlpha(mk.smoothing);
            sum += smoothers_[k].process(scaled);
        }

        // 6. The single clamped store — the only write this param sees
        // this tick.
        effect->getParam(static_cast<int>(m.targetParamIndex)).value =
            std::clamp(sum, 0.0f, 1.0f);
    }
}
