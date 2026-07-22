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

// Value snapshot of one layer's clips row plus the runtime fields that
// Deck/Layer clear-clips resets (clearActiveClip touches activeClipColumn /
// previousClipColumn / crossfadeProgress; pendingTriggerColumn rounds out the
// per-layer trigger runtime). Captured before AND after so undo/redo is a plain
// value restore, matching SetClipCmd's before/after style.
struct LayerClipsSnapshot
{
    std::vector<std::optional<Clip>> clips;
    int activeClipColumn = -1;
    int previousClipColumn = -1;
    float crossfadeProgress = 1.0f;
    int pendingTriggerColumn = -1;
};

inline LayerClipsSnapshot captureLayerClips(const Layer& layer)
{
    LayerClipsSnapshot s;
    s.clips = layer.clips;
    s.activeClipColumn = layer.activeClipColumn;
    s.previousClipColumn = layer.previousClipColumn;
    s.crossfadeProgress = layer.crossfadeProgress;
    s.pendingTriggerColumn = layer.pendingTriggerColumn;
    return s;
}

// True if the snapshot holds anything a clear would actually remove — any
// occupied cell or an active clip. Lets the clear handlers skip no-op layers so
// clearing an already-empty layer/deck pushes nothing (empty-composite spirit).
inline bool layerClipsSnapshotHasContent(const LayerClipsSnapshot& s)
{
    if (s.activeClipColumn >= 0)
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
        layer->activeClipColumn = state.activeClipColumn;
        layer->previousClipColumn = state.previousClipColumn;
        layer->crossfadeProgress = state.crossfadeProgress;
        layer->pendingTriggerColumn = state.pendingTriggerColumn;
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
