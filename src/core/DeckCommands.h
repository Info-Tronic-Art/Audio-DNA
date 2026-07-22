#pragma once
#include "core/Command.h"
#include "core/ClipCommands.h"   // ClipLayerResolver / ClipDeckResolver / ClipMediaHook
#include "model/Deck.h"          // Deck / Layer / Clip
#include <juce_events/juce_events.h>   // MessageManager (guarded jassert, like UndoManager)
#include <algorithm>
#include <functional>
#include <optional>
#include <string>
#include <utility>
#include <vector>

// Deck-structure undo commands (Undo v1, build step 4 — composites):
//   - SetColumnCountCmd   — column-count changes (#25 menu add + drop-growth gap)
//   - RemoveColumnCmd     — remove the last column, restoring its cells (#26)
//   - ClearLayerClipsCmd  — clear one layer's clips row (#19; composited for #23)
//
// All commands store COORDINATES (deckIndex / layerIndex) and re-resolve through
// the live model on every apply — they never hold raw Deck*/Layer*/Clip*, which
// dangle across vector reallocation. They are inline + renderer-free so the same
// headless unit tests that cover the clip commands cover these too.

// SetColumnCountCmd: set a deck's visible column count (deck->numColumns),
// re-resolved by deckIndex. Covers the menu "Add Column" (#25, before=N
// after=N+1) AND the column-growth undo of multi-cell drops (#7 multi-video,
// #9 multi-source, and the #8 multi-FX-onto-empty-cells path all place clips
// beyond the current grid, growing numColumns; undo must restore the prior
// count). Conforms to SwapClipsCmd's treatment: numColumns is the authoritative
// visible count (the grid draws numColumns, not clips.size()), so apply only
// sets numColumns and grows each layer's clips vector when the count increases
// (so grown columns render / can hold restored cells); it never erases on
// shrink — trailing cells past numColumns are invisible, exactly as
// SwapClipsCmd leaves them.
class SetColumnCountCmd : public Command
{
public:
    SetColumnCountCmd(ClipDeckResolver deckResolver, int deckIndex,
                      int before, int after, std::string description)
        : deckResolver_(std::move(deckResolver)), deckIndex_(deckIndex),
          before_(before), after_(after), description_(std::move(description)) {}

    void execute() override { apply(after_); }
    void undo() override    { apply(before_); }
    std::string description() const override { return description_; }

private:
    void apply(int count)
    {
        Deck* deck = deckResolver_ ? deckResolver_(deckIndex_) : nullptr;
        if (deck == nullptr)
            return;
        deck->numColumns = count;
        for (auto& layer : deck->layers)
            layer.ensureColumns(count);   // grow-only; ensureColumns never shrinks
    }

    ClipDeckResolver deckResolver_;
    int deckIndex_, before_, after_;
    std::string description_;
};

// RemoveColumnCmd: remove a deck column (in practice the last one, per the
// kColumnRemove handler) as one undo unit — spec §2 row #26. Deck::removeColumn
// ERASES the column's cell from every layer, so the removed cells must be
// snapshotted per layer; undo re-inserts them at the same index and restores the
// prior column count. Ids travel with each restored clip and renderer players
// are keyed by clip id, so the media hook re-resolves each restored cell for
// free (spec risk #4 guard).
class RemoveColumnCmd : public Command
{
public:
    RemoveColumnCmd(ClipDeckResolver deckResolver, ClipMediaHook mediaHook,
                    int deckIndex, int column, int columnsBefore,
                    std::vector<std::optional<Clip>> removedCells,
                    std::string description)
        : deckResolver_(std::move(deckResolver)), mediaHook_(std::move(mediaHook)),
          deckIndex_(deckIndex), column_(column), columnsBefore_(columnsBefore),
          removedCells_(std::move(removedCells)),
          description_(std::move(description))
    {
        // Invariant: this command removes only the LAST column. execute() re-runs
        // Deck::removeColumn(column_), which is idempotent with the live removal
        // ONLY when column_ == columnsBefore_ - 1 (removeColumn's col>=numColumns
        // guard then no-ops the re-run). A middle-column call site would silently
        // double-erase — assert so a future variant (column-insert, REST endpoint)
        // trips immediately. Guarded like UndoManager's message-thread assert:
        // enforced in the app (MessageManager present), skipped headless so
        // Catch2 does not abort.
        jassert(juce::MessageManager::getInstanceWithoutCreating() == nullptr
                || column_ == columnsBefore_ - 1);
    }

    void execute() override
    {
        if (Deck* deck = resolve())
            deck->removeColumn(column_);   // re-remove (matches live mutation)
    }

    void undo() override
    {
        Deck* deck = resolve();
        if (deck == nullptr)
            return;
        // Re-insert the removed cell into each layer at the same column index.
        for (size_t l = 0; l < deck->layers.size() && l < removedCells_.size(); ++l)
        {
            auto& clips = deck->layers[l].clips;
            const size_t insertAt = std::min(static_cast<size_t>(column_), clips.size());
            clips.insert(clips.begin() + static_cast<std::ptrdiff_t>(insertAt),
                         removedCells_[l]);
            if (removedCells_[l].has_value() && mediaHook_)
                mediaHook_(*removedCells_[l]);
        }
        deck->numColumns = columnsBefore_;
    }

    std::string description() const override { return description_; }

private:
    Deck* resolve() { return deckResolver_ ? deckResolver_(deckIndex_) : nullptr; }

    ClipDeckResolver deckResolver_;
    ClipMediaHook mediaHook_;
    int deckIndex_, column_, columnsBefore_;
    std::vector<std::optional<Clip>> removedCells_;
    std::string description_;
};

// Value snapshot of one layer's RUNTIME fields — the per-layer trigger state
// that clearActiveClip / clear-clips reset (activeClipColumn / previousClipColumn
// / crossfadeProgress; pendingTriggerColumn rounds out the trigger runtime).
// Factored out so both the clips-row snapshot (below) and ClearActiveClipCmd
// (the X-button clear, which touches ONLY these fields) share one definition.
struct LayerRuntimeSnapshot
{
    int activeClipColumn = -1;
    int previousClipColumn = -1;
    float crossfadeProgress = 1.0f;
    int pendingTriggerColumn = -1;
};

inline bool operator==(const LayerRuntimeSnapshot& a, const LayerRuntimeSnapshot& b)
{
    return a.activeClipColumn == b.activeClipColumn
        && a.previousClipColumn == b.previousClipColumn
        && a.crossfadeProgress == b.crossfadeProgress
        && a.pendingTriggerColumn == b.pendingTriggerColumn;
}

inline LayerRuntimeSnapshot captureLayerRuntime(const Layer& layer)
{
    return { layer.activeClipColumn, layer.previousClipColumn,
             layer.crossfadeProgress, layer.pendingTriggerColumn };
}

inline void applyLayerRuntime(Layer& layer, const LayerRuntimeSnapshot& r)
{
    layer.activeClipColumn = r.activeClipColumn;
    layer.previousClipColumn = r.previousClipColumn;
    layer.crossfadeProgress = r.crossfadeProgress;
    layer.pendingTriggerColumn = r.pendingTriggerColumn;
}

// Value snapshot of one layer's clips row plus its runtime (above). Captured
// before AND after so undo/redo is a plain value restore, matching SetClipCmd's
// before/after style.
struct LayerClipsSnapshot
{
    std::vector<std::optional<Clip>> clips;
    LayerRuntimeSnapshot runtime;
};

inline LayerClipsSnapshot captureLayerClips(const Layer& layer)
{
    LayerClipsSnapshot s;
    s.clips = layer.clips;
    s.runtime = captureLayerRuntime(layer);
    return s;
}

// True if the snapshot holds anything a clear would actually remove — any
// occupied cell or an active clip. Lets the clear handlers skip no-op layers so
// clearing an already-empty layer/deck pushes nothing (empty-composite spirit).
inline bool layerClipsSnapshotHasContent(const LayerClipsSnapshot& s)
{
    if (s.runtime.activeClipColumn >= 0)
        return true;
    for (const auto& c : s.clips)
        if (c.has_value())
            return true;
    return false;
}

// ClearLayerClipsCmd: clear one layer's clips row as one undo unit — spec §2
// row #19 (Wave 1-D fixed kLayerClearClips to clear only the SELECTED layer).
// Also the building block for whole-deck clear (#23): kDeckClearClips composites
// one ClearLayerClipsCmd per layer. Addressed by (deckIndex, layerIndex) and
// re-resolved on every apply. before/after are full value snapshots of the
// layer's clips row + runtime, so undo/redo is a plain restore. The media hook
// re-resolves each restored clip (spec risk #4 guard); on the cleared (after)
// state there are no clips so it never fires.
class ClearLayerClipsCmd : public Command
{
public:
    ClearLayerClipsCmd(ClipLayerResolver resolver, ClipMediaHook mediaHook,
                       int deckIndex, int layerIndex,
                       LayerClipsSnapshot before, LayerClipsSnapshot after,
                       std::string description)
        : resolver_(std::move(resolver)), mediaHook_(std::move(mediaHook)),
          deckIndex_(deckIndex), layerIndex_(layerIndex),
          before_(std::move(before)), after_(std::move(after)),
          description_(std::move(description)) {}

    void execute() override { apply(after_); }
    void undo() override    { apply(before_); }
    std::string description() const override { return description_; }

private:
    void apply(const LayerClipsSnapshot& state)
    {
        Layer* layer = resolver_ ? resolver_(deckIndex_, layerIndex_) : nullptr;
        if (layer == nullptr)
            return;
        layer->clips = state.clips;                       // value copy
        applyLayerRuntime(*layer, state.runtime);
        if (mediaHook_)
            for (const auto& c : layer->clips)
                if (c.has_value())
                    mediaHook_(*c);                       // reconnect restored media
    }

    ClipLayerResolver resolver_;
    ClipMediaHook mediaHook_;
    int deckIndex_, layerIndex_;
    LayerClipsSnapshot before_, after_;
    std::string description_;
};

// ===========================================================================
// Undo v1 step 5 — layer ops (#13-18, #20).
// ===========================================================================

// GL fence: runs a model mutation with the renderer's active deck detached and
// the GL thread fenced, so a mutation that reallocates/erases/moves deck->layers
// cannot be torn-read mid-frame (spec risk #1). Injected as a hook — the
// commands stay renderer-free / headless-testable; MainComponent binds it to
// UndoService::withDeckDetached, headless tests pass a pass-through. A null hook
// runs the mutation directly (no fence), which is exactly the headless case.
using DeckFenceHook = std::function<void(const std::function<void()>&)>;

// ClearActiveClipCmd: the layer X-button clear (#13) — Layer::clearActiveClip
// resets ONLY the per-layer runtime (activeClipColumn/previousClipColumn/
// crossfadeProgress; pendingTriggerColumn is snapshotted for completeness).
// The clips ROW is untouched, so this is a runtime-only before/after restore and
// needs NO GL fence (field-level write, status-quo risk). NOTE: clearActiveClip
// also sets the (previously) active clip's `playing` flag false; per spec risk
// #5 (runtime playback state in undo is accepted/imperfect) this command does
// not restore that flag — it restores the 4 layer runtime fields only.
class ClearActiveClipCmd : public Command
{
public:
    ClearActiveClipCmd(ClipLayerResolver resolver, int deckIndex, int layerIndex,
                       LayerRuntimeSnapshot before, LayerRuntimeSnapshot after,
                       std::string description)
        : resolver_(std::move(resolver)), deckIndex_(deckIndex),
          layerIndex_(layerIndex), before_(before), after_(after),
          description_(std::move(description)) {}

    void execute() override { apply(after_); }
    void undo() override    { apply(before_); }
    std::string description() const override { return description_; }

private:
    void apply(const LayerRuntimeSnapshot& r)
    {
        Layer* layer = resolver_ ? resolver_(deckIndex_, layerIndex_) : nullptr;
        if (layer == nullptr)
            return;
        applyLayerRuntime(*layer, r);
    }

    ClipLayerResolver resolver_;
    int deckIndex_, layerIndex_;
    LayerRuntimeSnapshot before_, after_;
    std::string description_;
};

// ToggleLayerFlagCmd: flip one boolean layer flag (#14 bypass, #15 solo, #20
// fold) as one undo unit. Field-level bool write → NO fence, NO merge (spec §3:
// each toggle is its own tiny command). before/after bools make undo/redo a
// plain restore. Addressed by (deckIndex, layerIndex) — never captures the raw
// Layer* LayerStrip holds.
class ToggleLayerFlagCmd : public Command
{
public:
    enum class Flag { Bypassed, Solo, Folded };

    ToggleLayerFlagCmd(ClipLayerResolver resolver, int deckIndex, int layerIndex,
                       Flag flag, bool before, bool after, std::string description)
        : resolver_(std::move(resolver)), deckIndex_(deckIndex),
          layerIndex_(layerIndex), flag_(flag), before_(before), after_(after),
          description_(std::move(description)) {}

    void execute() override { apply(after_); }
    void undo() override    { apply(before_); }
    std::string description() const override { return description_; }

private:
    void apply(bool value)
    {
        Layer* layer = resolver_ ? resolver_(deckIndex_, layerIndex_) : nullptr;
        if (layer == nullptr)
            return;
        switch (flag_)
        {
            case Flag::Bypassed: layer->bypassed = value; break;
            case Flag::Solo:     layer->solo = value;     break;
            case Flag::Folded:   layer->folded = value;   break;
        }
    }

    ClipLayerResolver resolver_;
    int deckIndex_, layerIndex_;
    Flag flag_;
    bool before_, after_;
    std::string description_;
};

// AddLayerCmd: layer new/insert (#16) → Deck::addLayer appends a layer. Uses the
// classic command-owns-the-mutation pattern (the handler does NOT pre-mutate;
// perform() runs execute()), because addLayer is non-idempotent — re-applying a
// live add in execute() would double-add. This also avoids the step-4
// "execute re-runs a live mutation" pattern entirely: the single fenced mutation
// happens exactly once per execute/undo/redo. The added layer is captured on the
// first execute so redo re-inserts the EXACT same layer (deterministic id), not
// a fresh addLayer with a new auto-id. GL fence: erase/insert reallocate
// deck->layers.
class AddLayerCmd : public Command
{
public:
    AddLayerCmd(ClipDeckResolver deckResolver, DeckFenceHook fence,
                int deckIndex, std::string description)
        : deckResolver_(std::move(deckResolver)), fence_(std::move(fence)),
          deckIndex_(deckIndex), description_(std::move(description)) {}

    void execute() override
    {
        runFenced([this]
        {
            Deck* deck = resolve();
            if (deck == nullptr)
                return;
            if (added_.has_value())
            {
                // redo: re-insert the exact layer captured on first execute.
                const size_t at = std::min(static_cast<size_t>(addedIndex_),
                                           deck->layers.size());
                deck->layers.insert(deck->layers.begin() + static_cast<std::ptrdiff_t>(at),
                                    *added_);
            }
            else
            {
                deck->addLayer();                                 // first do: append
                addedIndex_ = static_cast<int>(deck->layers.size()) - 1;
                added_ = deck->layers.back();                     // capture for redo
            }
        });
    }

    void undo() override
    {
        runFenced([this]
        {
            Deck* deck = resolve();
            if (deck == nullptr)
                return;
            if (addedIndex_ >= 0 && addedIndex_ < static_cast<int>(deck->layers.size()))
                deck->layers.erase(deck->layers.begin() + addedIndex_);
        });
    }

    std::string description() const override { return description_; }

private:
    Deck* resolve() { return deckResolver_ ? deckResolver_(deckIndex_) : nullptr; }
    void runFenced(const std::function<void()>& m) { if (fence_) fence_(m); else if (m) m(); }

    ClipDeckResolver deckResolver_;
    DeckFenceHook fence_;
    int deckIndex_;
    std::optional<Layer> added_;
    int addedIndex_ = -1;
    std::string description_;
};

// RemoveLayerCmd: layer remove (#17). Command-owns-the-mutation (handler
// snapshots the full Layer value, does NOT pre-remove; execute() erases). undo
// re-inserts the exact layer at its index; the media hook reconnects any clips
// it carried (spec risk #4). GL fence: erase/insert reallocate deck->layers.
class RemoveLayerCmd : public Command
{
public:
    RemoveLayerCmd(ClipDeckResolver deckResolver, DeckFenceHook fence,
                   ClipMediaHook mediaHook, int deckIndex, int layerIndex,
                   Layer removed, std::string description)
        : deckResolver_(std::move(deckResolver)), fence_(std::move(fence)),
          mediaHook_(std::move(mediaHook)), deckIndex_(deckIndex),
          layerIndex_(layerIndex), removed_(std::move(removed)),
          description_(std::move(description)) {}

    void execute() override
    {
        runFenced([this]
        {
            Deck* deck = resolve();
            if (deck == nullptr)
                return;
            // Deck must keep >=1 layer; the handler only builds this command when
            // more than one exists, and linear undo preserves that.
            if (deck->layers.size() > 1
                && layerIndex_ >= 0 && layerIndex_ < static_cast<int>(deck->layers.size()))
                deck->layers.erase(deck->layers.begin() + layerIndex_);
        });
    }

    void undo() override
    {
        runFenced([this]
        {
            Deck* deck = resolve();
            if (deck == nullptr)
                return;
            const size_t at = std::min(static_cast<size_t>(layerIndex_),
                                       deck->layers.size());
            deck->layers.insert(deck->layers.begin() + static_cast<std::ptrdiff_t>(at),
                                removed_);
            if (mediaHook_)
                for (const auto& c : deck->layers[at].clips)
                    if (c.has_value())
                        mediaHook_(*c);
        });
    }

    std::string description() const override { return description_; }

private:
    Deck* resolve() { return deckResolver_ ? deckResolver_(deckIndex_) : nullptr; }
    void runFenced(const std::function<void()>& m) { if (fence_) fence_(m); else if (m) m(); }

    ClipDeckResolver deckResolver_;
    DeckFenceHook fence_;
    ClipMediaHook mediaHook_;
    int deckIndex_, layerIndex_;
    Layer removed_;
    std::string description_;
};

// MoveLayerCmd: layer move up/down (#18) → Deck::moveLayer. Command-owns-the-
// mutation with clean index inverses: execute moves from->to, undo moves to->from
// (moveLayer(to,from) is the exact inverse of moveLayer(from,to) — the layer that
// landed at `to` is put back at `from`). GL fence: moveLayer erases + inserts,
// reallocating deck->layers.
class MoveLayerCmd : public Command
{
public:
    MoveLayerCmd(ClipDeckResolver deckResolver, DeckFenceHook fence,
                 int deckIndex, int fromIndex, int toIndex, std::string description)
        : deckResolver_(std::move(deckResolver)), fence_(std::move(fence)),
          deckIndex_(deckIndex), fromIndex_(fromIndex), toIndex_(toIndex),
          description_(std::move(description)) {}

    void execute() override { runFenced([this] { move(fromIndex_, toIndex_); }); }
    void undo() override    { runFenced([this] { move(toIndex_, fromIndex_); }); }
    std::string description() const override { return description_; }

private:
    void move(int from, int to)
    {
        if (Deck* deck = resolve())
            deck->moveLayer(from, to);
    }
    Deck* resolve() { return deckResolver_ ? deckResolver_(deckIndex_) : nullptr; }
    void runFenced(const std::function<void()>& m) { if (fence_) fence_(m); else if (m) m(); }

    ClipDeckResolver deckResolver_;
    DeckFenceHook fence_;
    int deckIndex_, fromIndex_, toIndex_;
    std::string description_;
};
