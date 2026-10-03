#pragma once
// InspectorRepoint (lane bf9b fix, s-rta-1003; ruling-bf9b-merge.md AM-2 + Harmony's adoption item 2): what the Clip
// and Layer inspectors do after a fenced edit, as free functions so the tests drive the very code the app runs
// (MainComponent's two hook statements are one-line adapters, pinned by lint B4h in test_render_thread_lint).
// Both inspectors keep a raw pointer into the show model (ClipInspector::clip_, LayerInspector::layer_) that their
// timers read. Safe to call with a pointer that already dangles: setClip / setLayer forget their scalar bindings
// first (AM-1), their effect stacks and source-param rows forget theirs, and nothing here dereferences the old pointer.
// Message thread only, after the fence has ended.
#include "core/EffectScope.h"
#include "model/Composition.h"
#include "ui/ClipInspector.h"
#include "ui/LayerInspector.h"

// Does a live or retired deck of `comp` hold a clip at this address? An address walk: `clip` is never dereferenced.
inline bool compositionOwnsClip(const Composition& comp, const Clip* clip)
{
    if (clip == nullptr)
        return false;
    bool owned = false;
    comp.forEachClip([&owned, clip](const Clip& c, const ClipSite&) { owned = owned || &c == clip; });
    return owned;
}

// "Owned, or clear": a Clip inspector whose clip the composition no longer owns is cleared; one it still owns is
// left alone (never re-pointed by coordinate: a Load Deck must not change what the Clip tab shows).
// UndoService::onFencedEdit -- every fenced edit that kept the layer stack (Layer > Clear Clips, Deck > Clear Clips,
// a column add / remove that moved a row's clips).
inline void clearClipInspectorIfUnowned(ClipInspector& clipInspector, const Composition& comp)
{
    if (clipInspector.getClip() != nullptr && !compositionOwnsClip(comp, clipInspector.getClip()))
        clipInspector.setClip(nullptr);
}

// UndoService::onLayerStackMoved -- a fenced edit that moved or resized Composition::layers (Load / Duplicate Deck of
// a deck wider than the show, Add / Remove Layer, their undos, a model swap):
// (1) the Clip inspector: owned, or clear (Remove Layer frees the last row's clips);
// (2) the Layer inspector is re-pointed by the selected layer row; a stale row clears it.
inline void repointInspectorsAfterStackMove(ClipInspector& clipInspector, LayerInspector& layerInspector,
                                            Composition& comp, int selectedLayerRow)
{
    clearClipInspectorIfUnowned(clipInspector, comp);
    Layer* fresh = selectedLayerRow >= 0 ? comp.getLayer(selectedLayerRow) : nullptr;
    layerInspector.setLayer(fresh, fresh != nullptr ? EffectScope::layer(-1, selectedLayerRow) : EffectScope::none());
}
