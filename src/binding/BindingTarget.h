#pragma once
#include "binding/Binding.h"
#include "model/Composition.h"

// BindingTarget (lane bf9b, s-rta-1002b; plan-bf9b S2.8, ruling-bf9b amendment 20): WHERE a TriggerClip binding fires.
// Pure (model only), so MainComponent::handleBindingAction and tests/test_show_model.cpp T16 run the same resolution.
//   ByPosition: (targetLayerIndex, targetColumn) on the SHOWN deck (deck -1).
//   Selected:   the first SHARED layer with an active clip -- that clip's own ref (its deck may be another one than
//               the shown deck); a layer playing a removed (retired) deck's clip: retired = true, nothing fires.
//   ThisItem:   the clip with targetClipId in EVERY live deck ("firing a clip from ANY deck"); not found -> ByPosition.
struct BindingTarget
{
    int layer = -1;
    int column = -1;
    int deck = -1;          // the live deck index the cell is in; -1 = the shown deck
    bool retired = false;   // Selected on a layer playing a removed deck's clip

    // The deck a velocity write lands on: the cell's own deck, else the shown deck.
    int velocityDeck(const Composition& comp) const { return deck >= 0 ? deck : comp.activeDeckIndex.load(); }
};

inline BindingTarget resolveBindingTarget(const Composition& comp, const Binding& binding)
{
    BindingTarget t;
    t.layer = binding.targetLayerIndex;
    t.column = binding.targetColumn;

    if (binding.targetMode == Binding::TargetMode::Selected)
    {
        for (int li = 0; li < comp.getNumLayers(); ++li)
        {
            if (const auto ref = comp.layers[static_cast<size_t>(li)].runtime().activeRef(); ref.column >= 0)
            {
                t.layer = li;
                t.column = ref.column;
                t.deck = comp.findDeckIndexById(ref.deckId);
                t.retired = t.deck < 0;
                break;
            }
        }
    }
    else if (binding.targetMode == Binding::TargetMode::ThisItem && binding.targetClipId > 0)
    {
        comp.forEachClip([&](const Clip& clip, const ClipSite& site) {
            if (t.deck < 0 && !site.retired && clip.id == binding.targetClipId)
            {
                t.layer = site.row;
                t.column = site.column;
                t.deck = site.deckIndex;
            }
        });
    }
    return t;
}
