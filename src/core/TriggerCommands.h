#pragma once
#include "core/Command.h"
#include "core/ClipCommands.h"   // ClipLayerResolver
#include "core/DeckCommands.h"   // LayerRuntimeSnapshot / capture / applyLayerRuntime
#include "model/Layer.h"         // Layer / Clip (getClipAt, runtime fields)
#include <optional>
#include <string>
#include <utility>

// Undo v1 step 8 — clip / column triggers (spec §2 rows #1-#2, §3 merge rules).
//
// TriggerClipCmd wraps a single user-initiated clip trigger
// (MainComponent::handleClipTrigger — the deck-cell click, plus the REST / OSC /
// MIDI paths that all marshal to the message thread and route through the SAME
// handler). A column trigger (spec row #2) is a CompositeCommand of one
// TriggerClipCmd per non-ignoring layer, built through the shared pushCommands
// helper — there is no separate TriggerColumnCmd class (the composite machinery
// IS the "TriggerColumnCmd" the spec names).
//
// PATTERN (two-pattern rule): a trigger is an IDEMPOTENT field write — undo/redo
// is a plain value-assignment of before/after snapshots, not a re-run of
// Layer::triggerClip — so it is MUTATE-THEN-PUSH (the handler performs the live
// trigger, then wraps the captured before/after; execute() re-applies `after`, a
// harmless no-op because the model is already there). This is the same shape as
// ClearActiveClipCmd (#13) and SwitchDeckCmd (#24), NOT the command-owns-the-
// mutation shape used for the non-idempotent structural layer/deck ops.
//
// GL-PATH LAW (spec §1 consequence 3, risk #2): command creation lives ONLY in
// the handlers, never inside Layer::triggerClip — autopilot calls
// Layer::triggerClip directly from the GL render thread
// (Renderer::renderOpenGL → Autopilot::processFrame → Layer::triggerClip), so
// autopilot-driven triggers create NO commands.
//
// NO GL FENCE: a trigger only writes per-layer runtime fields (and one clip's
// `playing` flag) in place — status-quo field-level race, exactly like today's
// direct writes. The fence is reserved for vector-structure ops.

// Snapshot payload for one clip trigger (spec §2 row 1): the four per-layer
// runtime fields (via LayerRuntimeSnapshot — activeClipColumn / previousClipColumn
// / crossfadeProgress / pendingTriggerColumn) PLUS the target column clip's
// `playing` flag. The `playing` flag is std::optional<bool>: nullopt when the
// target cell is empty (an empty-cell trigger clears the active clip — a
// runtime-field change only; the previously-active clip's `playing` is NOT
// restored, the same accepted imperfection as ClearActiveClipCmd, spec risk #5).
//
// Also accepted (spec risk #5 family): `hasBeenTriggered` is set unconditionally
// by handleClipTrigger (MainComponent.cpp ~2852) OUTSIDE this snapshot window and
// is never rolled back by undo, so first-ever-trigger → undo → re-trigger skips
// the auto-play branch (Layer.h ~247) — the clip goes active but does not
// auto-play. Documented here + on the step-9 manual checklist.
//
// TriggerClipCmd re-resolves the Layer by (deckIndex, layerIndex) on every apply
// and never stores a raw Layer*/Clip* (which dangle across vector reallocation).
class TriggerClipCmd : public Command
{
public:
    TriggerClipCmd(ClipLayerResolver resolver, int deckIndex, int layerIndex,
                   int column, LayerRuntimeSnapshot before, LayerRuntimeSnapshot after,
                   std::optional<bool> targetPlayingBefore,
                   std::optional<bool> targetPlayingAfter, std::string description)
        : resolver_(std::move(resolver)), deckIndex_(deckIndex),
          layerIndex_(layerIndex), column_(column),
          before_(before), after_(after),
          targetPlayingBefore_(targetPlayingBefore),
          targetPlayingAfter_(targetPlayingAfter),
          description_(std::move(description)) {}

    void execute() override { apply(after_, targetPlayingAfter_); }
    void undo() override    { apply(before_, targetPlayingBefore_); }
    std::string description() const override { return description_; }

    // Merge rule (spec §3): consecutive triggers on the SAME layer coalesce into
    // one history slot — keep the ORIGINAL before-state, update the after-state —
    // so mashing cells mid-set costs one slot per layer run, not one per click.
    // Only same-(deck,layer) TriggerClipCmds merge; different-layer triggers do
    // NOT (each layer run is its own slot). A column trigger is a CompositeCommand,
    // whose base canMergeWith is false, so column triggers never merge (chosen:
    // simplest conforming behavior — spec does not require column-to-column merge).
    bool canMergeWith(const Command& other) const override
    {
        const auto* o = dynamic_cast<const TriggerClipCmd*>(&other);
        return o != nullptr && o->deckIndex_ == deckIndex_
            && o->layerIndex_ == layerIndex_;
    }

    void mergeWith(const Command& other) override
    {
        const auto* o = dynamic_cast<const TriggerClipCmd*>(&other);
        if (o == nullptr)
            return;
        // Keep the original runtime before-state (before_ untouched); adopt the
        // latest trigger's runtime after-state. The target column follows the
        // latest trigger, so its `playing` before/after retarget with it — the
        // best-effort playing restore then tracks the run's FINAL target clip
        // (an earlier mashed clip's `playing` may linger, accepted per spec
        // risk #5). The runtime restore — which clip is active — stays exact.
        after_ = o->after_;
        column_ = o->column_;
        targetPlayingBefore_ = o->targetPlayingBefore_;
        targetPlayingAfter_ = o->targetPlayingAfter_;
    }

private:
    void apply(const LayerRuntimeSnapshot& runtime, const std::optional<bool>& playing)
    {
        Layer* layer = resolver_ ? resolver_(deckIndex_, layerIndex_) : nullptr;
        if (layer == nullptr)
            return;                             // stale coordinate → safe no-op
        applyLayerRuntime(*layer, runtime);
        if (playing.has_value())
            if (Clip* clip = layer->getClipAt(column_))
                clip->playing = *playing;
    }

    ClipLayerResolver resolver_;
    int deckIndex_, layerIndex_, column_;
    LayerRuntimeSnapshot before_, after_;
    std::optional<bool> targetPlayingBefore_, targetPlayingAfter_;
    std::string description_;
};
