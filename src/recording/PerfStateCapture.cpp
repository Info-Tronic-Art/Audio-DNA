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
    // 3.2). Key encoding is PerfState::fxParamKey (s-rta-0925: moved out of
    // this file into PerfState.h so Program::compile's preamble synthesis
    // shares the SAME encoding when reading this map back, rather than
    // re-deriving it independently).
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
                    out[PerfState::fxParamKey(static_cast<int>(slotIdx), static_cast<int>(p))] = slot.paramValues[p];
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

    // PerfState v2 (lane bf9b): the SHARED layers once -- tuple, flags, opacity, layer effects and the deck each
    // active clip came from (index + name; -1 when it came from a removed deck).
    for (int layerIdx = 0; layerIdx < comp.getNumLayers(); ++layerIdx)
    {
        const Layer& layer = comp.layers[static_cast<size_t>(layerIdx)];
        PerfState::LayerRuntime layerRT;
        layerRT.layer = layer.name;
        const LayerRuntimeSnapshot rt = layer.runtime();   // one consistent tuple
        layerRT.activeClipColumn = rt.activeClipColumn;
        layerRT.previousClipColumn = rt.previousClipColumn;
        layerRT.crossfadeProgress = rt.crossfadeProgress;
        layerRT.pendingTriggerColumn = rt.pendingTriggerColumn;
        layerRT.pendingTriggerSnapOverride = static_cast<int>(rt.pendingTriggerSnapOverride);
        layerRT.opacity = layer.opacity;
        layerRT.visible = layer.visible;
        layerRT.bypassed = layer.bypassed;
        layerRT.solo = layer.solo;
        layerRT.muted = layer.muted;
        layerRT.autopilotEnabled = layer.autopilotEnabled;
        layerRT.effectParams = nonDefaultEffectParams(layer.layerEffects);
        if (rt.activeClipColumn >= 0)
        {
            layerRT.activeDeck = comp.findDeckIndexById(rt.activeDeckId);
            if (layerRT.activeDeck >= 0)
                layerRT.activeDeckName = comp.decks[static_cast<size_t>(layerRT.activeDeck)].name;
            else
                layerRT.activeClipColumn = -1;   // from a removed (retired) deck: nothing a take can restore
        }
        state.layers[layerIdx] = std::move(layerRT);
    }

    // Per deck, only the CLIP runtime of each row.
    for (size_t deckIdx = 0; deckIdx < comp.decks.size(); ++deckIdx)
    {
        const Deck& deck = comp.decks[deckIdx];
        PerfState::DeckRuntime deckRT;
        deckRT.deck = deck.name;

        for (size_t rowIdx = 0; rowIdx < deck.rows.size(); ++rowIdx)
        {
            const ClipRow& row = deck.rows[rowIdx];
            PerfState::LayerRuntime rowRT;
            if (const Layer* layer = comp.getLayer(static_cast<int>(rowIdx)))
                rowRT.layer = layer->name;

            for (size_t col = 0; col < row.clips.size(); ++col)
            {
                if (!row.clips[col].has_value())
                    continue;
                const Clip& clip = *row.clips[col];

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
                    rowRT.clips[static_cast<int>(col)] = std::move(clipRT);
            }

            deckRT.layers[static_cast<int>(rowIdx)] = std::move(rowRT);
        }

        state.decks[static_cast<int>(deckIdx)] = std::move(deckRT);
    }

    return state;
}
