#include "recording/PerfStateCapture.h"
#include "model/Composition.h"
#include "connect/ScalarParams.h"
#include "effects/EffectLibrary.h"
#include <cmath>

namespace
{
    // Function-local static (thread-safe init, C++11) -- capturePerfState is
    // message-thread only (same as everything else in src/recording/), so no
    // synchronization beyond the static's own guarantee is needed. Mirrors
    // test_take's own link set (EffectLibrary.cpp/Effect.cpp), see the
    // header's T17 comment.
    const EffectLibrary& library()
    {
        static const EffectLibrary lib = [] {
            EffectLibrary l;
            l.registerDefaults();
            return l;
        }();
        return lib;
    }

    // "Manual values that differ from the library default" (plan section
    // 3.2). Key encoding: `slotIndex * 100 + paramIndex` -- PerfState.h's
    // own field comment ("fx slot index -> manual value") undersells the
    // multi-param reality (an effect slot can carry several params, up to
    // EffectLibrary's largest param list, well under 100); *100 leaves
    // ample headroom with no collisions across any registered effect. This
    // encoding is PerfStateCapture's own, private to how it fills the map --
    // PerfState.h is not touched by this lane and this map's `int` key type
    // does not change.
    std::map<int, float> nonDefaultEffectParams(const std::vector<Clip::EffectSlot>& effects)
    {
        std::map<int, float> out;
        for (size_t slotIdx = 0; slotIdx < effects.size(); ++slotIdx)
        {
            const auto& slot = effects[slotIdx];
            const auto* def = library().getEffectDef(juce::String(slot.effectName));
            if (def == nullptr)
                continue;
            const size_t paramCount = std::min(slot.paramValues.size(), def->params.size());
            for (size_t p = 0; p < paramCount; ++p)
            {
                if (std::abs(slot.paramValues[p] - def->params[p].defaultValue) > 1e-4f)
                    out[static_cast<int>(slotIdx * 100 + p)] = slot.paramValues[p];
            }
        }
        return out;
    }
}

PerfState capturePerfState(const Composition& comp, float bpm, const std::string& audioAction)
{
    PerfState state;
    state.activeDeckIndex = comp.activeDeckIndex;
    state.quantizeMode = static_cast<int>(comp.quantizeMode);
    state.bpm = bpm;
    state.audioAction = audioAction;

    const auto& scalarDefs = clipScalarDefs();

    for (size_t deckIdx = 0; deckIdx < comp.decks.size(); ++deckIdx)
    {
        const Deck& deck = comp.decks[deckIdx];
        PerfState::DeckRuntime deckRT;
        deckRT.deck = deck.name;

        for (size_t layerIdx = 0; layerIdx < deck.layers.size(); ++layerIdx)
        {
            const Layer& layer = deck.layers[layerIdx];
            PerfState::LayerRuntime layerRT;
            layerRT.layer = layer.name;
            layerRT.activeClipColumn = layer.activeClipColumn;
            layerRT.previousClipColumn = layer.previousClipColumn;
            layerRT.crossfadeProgress = layer.crossfadeProgress;
            layerRT.pendingTriggerColumn = layer.pendingTriggerColumn;
            layerRT.pendingTriggerSnapOverride = static_cast<int>(layer.pendingTriggerSnapOverride);
            layerRT.opacity = layer.opacity;
            layerRT.visible = layer.visible;
            layerRT.bypassed = layer.bypassed;
            layerRT.solo = layer.solo;
            layerRT.muted = layer.muted;
            layerRT.autopilotEnabled = layer.autopilotEnabled;
            layerRT.effectParams = nonDefaultEffectParams(layer.layerEffects);

            for (size_t col = 0; col < layer.clips.size(); ++col)
            {
                if (!layer.clips[col].has_value())
                    continue;
                const Clip& clip = *layer.clips[col];

                PerfState::ClipRuntime clipRT;
                clipRT.clip = clip.name;
                clipRT.playing = clip.playing;
                clipRT.playheadPosition = clip.playheadPosition;
                clipRT.effectParams = nonDefaultEffectParams(clip.effects);

                for (size_t s = 0; s < scalarDefs.size(); ++s)
                {
                    const float modelVal = clip.eff(static_cast<ClipScalar>(s));
                    const float norm = scalarDefs[s].toNorm(modelVal);
                    if (std::abs(norm - scalarDefs[s].defaultNorm) > 1e-4f)
                        clipRT.scalars[scalarDefs[s].key] = norm;
                }

                // D4: "Only NON-DEFAULT clip state is worth keeping" -- skip
                // an entirely-default clip entirely rather than storing an
                // empty entry.
                const bool nonDefault = clipRT.playing || clipRT.playheadPosition != 0.0
                    || !clipRT.effectParams.empty() || !clipRT.scalars.empty();
                if (nonDefault)
                    layerRT.clips[static_cast<int>(col)] = std::move(clipRT);
            }

            deckRT.layers[static_cast<int>(layerIdx)] = std::move(layerRT);
        }

        state.decks[static_cast<int>(deckIdx)] = std::move(deckRT);
    }

    return state;
}
