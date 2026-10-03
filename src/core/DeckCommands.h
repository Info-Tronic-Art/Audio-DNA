#pragma once
#include "core/Command.h"
#include "core/ClipCommands.h"   // ClipLayerResolver / ClipDeckResolver / CompositionResolver / media hooks
#include "model/Deck.h"          // Deck / ClipRow / Clip
#include "model/Composition.h"   // Composition (deck-vector ops: decks + activeDeckIndex)
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

// GL fence for structure-changing mutations (family-fence fix, 2026-07-28 — see
// .harmony/notebook.md LAW entry): a message-thread mutation that resizes,
// clears, or erases a layer's clips vector can reallocate it while the GL
// thread (unlocked — setComponentPaintingEnabled(false)) holds an interior
// Clip* via getActiveClip()/applyClipEffects — a proven UAF (SIGSEGV on
// Column -> New). Injected as a hook so commands stay renderer-free /
// headless-testable; MainComponent binds it to UndoService::withDeckDetached
// (GL fence, validated in build step 1), headless tests pass a pass-through. A
// null hook runs the mutation directly (no fence), which is exactly the
// headless case. SetColumnCountCmd / RemoveColumnCmd / ClearLayerClipsCmd
// below, the step-5/6 layer/deck commands further down, and (round 3,
// 2026-07-28) ClipCommands.h's SetClipCmd/SwapClipsCmd all fence through this
// same hook type. The `DeckFenceHook` alias itself is now DEFINED IN
// ClipCommands.h (included above) since those two commands need it too and
// ClipCommands.h sits below this file in the include graph — this file no
// longer redeclares it, it just uses the one definition.

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
// SwapClipsCmd leaves them. GL fence (2026-07-28): apply() grows EVERY layer's
// clips vector (ensureColumns), the same reallocation-prone mutation as
// addColumn's live handler — fenced identically on execute/undo/redo.
class SetColumnCountCmd : public Command
{
public:
    SetColumnCountCmd(ClipDeckResolver deckResolver, DeckFenceHook fence,
                      int deckIndex, int before, int after, std::string description)
        : deckResolver_(std::move(deckResolver)), fence_(std::move(fence)),
          deckIndex_(deckIndex),
          before_(before), after_(after), description_(std::move(description)) {}

    void execute() override { runFenced([this] { apply(after_); }); }
    void undo() override    { runFenced([this] { apply(before_); }); }
    std::string description() const override { return description_; }

private:
    void apply(int count)
    {
        Deck* deck = deckResolver_ ? deckResolver_(deckIndex_) : nullptr;
        if (deck == nullptr)
            return;
        deck->numColumns = count;
        for (auto& row : deck->rows)
            row.ensureColumns(count);     // grow-only; ensureColumns never shrinks
    }
    void runFenced(const std::function<void()>& m) { if (fence_) fence_(m); else if (m) m(); }

    ClipDeckResolver deckResolver_;
    DeckFenceHook fence_;
    int deckIndex_, before_, after_;
    std::string description_;
};

// RemoveColumnCmd: remove a deck column (in practice the last one, per the
// kColumnRemove handler) as one undo unit — spec §2 row #26. Deck::removeColumn
// ERASES the column's cell from every layer, so the removed cells must be
// snapshotted per layer; undo re-inserts them at the same index and restores the
// prior column count. Ids travel with each restored clip and renderer players
// are keyed by clip id, so the media hook re-resolves each restored cell for
// free (spec risk #4 guard). GL fence (2026-07-28): execute() re-runs
// Deck::removeColumn (clips.erase per layer) and undo() re-inserts across every
// layer, the same crash-proven reallocation/erase class as the live handler
// (kColumnRemove) — fenced identically on execute/undo/redo.
class RemoveColumnCmd : public Command
{
public:
    RemoveColumnCmd(ClipDeckResolver deckResolver, DeckFenceHook fence,
                    ClipMediaHook mediaHook, ClipMediaDisposeHook disposeHook,
                    int deckIndex, int column, int columnsBefore,
                    std::vector<std::optional<Clip>> removedCells,
                    std::string description)
        : deckResolver_(std::move(deckResolver)), fence_(std::move(fence)),
          mediaHook_(std::move(mediaHook)), disposeHook_(std::move(disposeHook)),
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
        runFenced([this]
        {
            if (Deck* deck = resolve())
                deck->removeColumn(column_);   // re-remove (matches live mutation)
            // Family coverage (media-leak fix, L1): removedCells_ is the
            // BEFORE snapshot, captured by the live handler ahead of its own
            // removeColumn() call — same double-apply shape as SetClipCmd.
            // The removed column is gone from the model for good (no move
            // target, unlike SwapClipsCmd), so every occupied cell in it
            // disposes unconditionally; undo's mediaHook_ call below
            // reconnects them if the column comes back.
            if (disposeHook_)
                for (const auto& cell : removedCells_)
                    if (cell.has_value())
                        disposeHook_(*cell);
        });
    }

    void undo() override
    {
        runFenced([this]
        {
            Deck* deck = resolve();
            if (deck == nullptr)
                return;
            // Re-insert the removed cell into each row at the same column index.
            for (size_t l = 0; l < deck->rows.size() && l < removedCells_.size(); ++l)
            {
                auto& clips = deck->rows[l].clips;
                const size_t insertAt = std::min(static_cast<size_t>(column_), clips.size());
                clips.insert(clips.begin() + static_cast<std::ptrdiff_t>(insertAt),
                             removedCells_[l]);
                if (removedCells_[l].has_value() && mediaHook_)
                    mediaHook_(*removedCells_[l]);
            }
            deck->numColumns = columnsBefore_;
        });
    }

    std::string description() const override { return description_; }

private:
    Deck* resolve() { return deckResolver_ ? deckResolver_(deckIndex_) : nullptr; }
    void runFenced(const std::function<void()>& m) { if (fence_) fence_(m); else if (m) m(); }

    ClipDeckResolver deckResolver_;
    DeckFenceHook fence_;
    ClipMediaHook mediaHook_;
    ClipMediaDisposeHook disposeHook_;
    int deckIndex_, column_, columnsBefore_;
    std::vector<std::optional<Clip>> removedCells_;
    std::string description_;
};

// The per-layer trigger tuple (LayerRuntimeSnapshot, src/model/Layer.h -- moved there by lane tsan, s-rta-1002: the
// tuple is ONE atomic word inside Layer). These two keep their signatures as the compat surface: capture is ONE
// acquire load of the whole tuple, apply is ONE release store (a value restore).
inline LayerRuntimeSnapshot captureLayerRuntime(const Layer& layer)
{
    return layer.runtime();
}

inline void applyLayerRuntime(Layer& layer, const LayerRuntimeSnapshot& r)
{
    layer.setRuntime(r);
}

// Cancel every QUEUED trigger whose pending ref names deck `deckId` (lane bf9b, ruling-bf9b amendment 4(c)): a clip
// from a box you can no longer see must not start later. Only RemoveDeckCmd calls it, inside its fence, for the deck
// it retires or erases. A deck switch cancels nothing any more (plan-bf9b F11: a queued trigger lands in the shared,
// visible stack). Each layer's cancel is ONE compare-exchange of its tuple word. Returns the number cancelled.
inline int cancelPendingInto(Composition& comp, uint32_t deckId)
{
    int cancelled = 0;
    for (auto& layer : comp.layers)
    {
        const auto t = layer.updateRuntime([deckId](LayerRuntimeSnapshot r) {
            if (r.pendingTriggerColumn < 0 || r.pendingDeckId != deckId)
                return r;
            r.pendingTriggerColumn = -1;
            r.pendingDeckId = ClipRef::kNoDeck;
            r.pendingTriggerSnapOverride = Clip::BeatSnapMode::Off;
            return r;
        });
        if (t.changed())
            ++cancelled;
    }
    return cancelled;
}

// Value snapshot of one deck's clip row plus -- only when the shared layer plays or queues from THAT row of THAT
// deck -- the layer's tuple (lane bf9b). Captured before AND after so undo/redo is a plain value restore, matching
// SetClipCmd's before/after style. runtime nullopt = the tuple is not this row's to touch (another deck's clip keeps
// playing).
struct LayerClipsSnapshot
{
    std::vector<std::optional<Clip>> clips;
    std::optional<LayerRuntimeSnapshot> runtime;
};

// Is the tuple a clear of deck `deckId`'s row has to touch: it PLAYS (active) or QUEUES a clip of that row? (An
// outgoing clip of that row just ends its fade early: the compositor finds no outgoing clip and cuts.)
inline bool tupleNamesDeck(const LayerRuntimeSnapshot& r, uint32_t deckId)
{
    return (r.activeClipColumn >= 0 && r.activeDeckId == deckId)
        || (r.pendingTriggerColumn >= 0 && r.pendingDeckId == deckId);
}

inline LayerClipsSnapshot captureLayerClips(const Composition& comp, int deckIndex, int layerIndex)
{
    LayerClipsSnapshot s;
    if (deckIndex < 0 || deckIndex >= static_cast<int>(comp.decks.size()))
        return s;
    const Deck& deck = comp.decks[static_cast<size_t>(deckIndex)];
    if (const ClipRow* row = deck.getRow(layerIndex))
        s.clips = row->clips;
    if (const Layer* layer = comp.getLayer(layerIndex))
        if (const auto rt = layer->runtime(); tupleNamesDeck(rt, deck.id))
            s.runtime = rt;
    return s;
}

// The state a clear of deck `deckId`'s row leaves: the same number of EMPTY cells; the tuple cleared
// (Layer::clearedNext, incl. its queue) only when the layer plays from that row of that deck, else only a queued
// trigger into it cancelled; untouched otherwise.
inline LayerClipsSnapshot clearedLayerClips(const LayerClipsSnapshot& before, uint32_t deckId)
{
    LayerClipsSnapshot s;
    s.clips.assign(before.clips.size(), std::nullopt);
    if (before.runtime.has_value())
    {
        LayerRuntimeSnapshot r = *before.runtime;
        if (r.activeClipColumn >= 0 && r.activeDeckId == deckId)
            r = Layer::clearedNext(r);
        else if (r.pendingTriggerColumn >= 0 && r.pendingDeckId == deckId)
        {
            r.pendingTriggerColumn = -1;
            r.pendingDeckId = ClipRef::kNoDeck;
            r.pendingTriggerSnapOverride = Clip::BeatSnapMode::Off;
        }
        s.runtime = r;
    }
    return s;
}

// The live half of a row clear (kDeckClearClips / kLayerClearClips; lane bf9b): ONE compare-exchange of the shared
// layer's tuple -- cleared (Layer::clearedNext, incl. its queue) when it plays from deck `deckId`'s row, else a queued
// trigger into that row cancelled; untouched otherwise (another deck's clip keeps playing).
inline LayerRuntimeTransition clearTupleForRow(Layer& layer, uint32_t deckId)
{
    return layer.updateRuntime([deckId](LayerRuntimeSnapshot r) {
        if (r.activeClipColumn >= 0 && r.activeDeckId == deckId)
            return Layer::clearedNext(r);
        if (r.pendingTriggerColumn >= 0 && r.pendingDeckId == deckId)
        {
            r.pendingTriggerColumn = -1;
            r.pendingDeckId = ClipRef::kNoDeck;
            r.pendingTriggerSnapOverride = Clip::BeatSnapMode::Off;
        }
        return r;
    });
}

// True if the snapshot holds anything a clear would actually remove — any
// occupied cell or a tuple to clear. Lets the clear handlers skip no-op rows so
// clearing an already-empty row/deck pushes nothing (empty-composite spirit).
inline bool layerClipsSnapshotHasContent(const LayerClipsSnapshot& s)
{
    if (s.runtime.has_value())   // captured only when the layer plays or queues from this row of this deck
        return true;
    for (const auto& c : s.clips)
        if (c.has_value())
            return true;
    return false;
}

// ClearLayerClipsCmd: clear one deck's row as one undo unit — spec §2 row #19 (Wave 1-D fixed kLayerClearClips to
// clear only the SELECTED layer; lane bf9b: the row of the SHOWN deck). Also the building block for whole-deck clear
// (#23): kDeckClearClips composites one ClearLayerClipsCmd per row. Addressed by (deckIndex, layerIndex) and
// re-resolved on every apply. before/after are value snapshots of the row + (only when it plays from that row of
// that deck) the shared layer's tuple, so undo/redo is a plain restore and a layer playing ANOTHER deck's clip keeps
// playing (plan-bf9b 4.B). The media hook re-resolves each restored clip (spec risk #4 guard); on the cleared
// (after) state there are no clips so it never fires. GL fence (2026-07-28): apply() replaces the WHOLE clips vector
// (row.clips = state.clips), the same crash-proven reallocation class as the live handlers — fenced on
// execute/undo/redo.
class ClearLayerClipsCmd : public Command
{
public:
    ClearLayerClipsCmd(CompositionResolver resolver, DeckFenceHook fence,
                       ClipMediaHook mediaHook, ClipMediaDisposeHook disposeHook,
                       int deckIndex, int layerIndex,
                       LayerClipsSnapshot before, LayerClipsSnapshot after,
                       std::string description)
        : resolver_(std::move(resolver)), fence_(std::move(fence)),
          mediaHook_(std::move(mediaHook)), disposeHook_(std::move(disposeHook)),
          deckIndex_(deckIndex), layerIndex_(layerIndex),
          before_(std::move(before)), after_(std::move(after)),
          description_(std::move(description)) {}

    void execute() override { runFenced([this] { apply(after_, before_); }); }
    void undo() override    { runFenced([this] { apply(before_, after_); }); }
    std::string description() const override { return description_; }

private:
    void apply(const LayerClipsSnapshot& state, const LayerClipsSnapshot& leaving)
    {
        Composition* comp = resolver_ ? resolver_() : nullptr;
        if (comp == nullptr || deckIndex_ < 0 || deckIndex_ >= static_cast<int>(comp->decks.size()))
            return;
        ClipRow* row = comp->decks[static_cast<size_t>(deckIndex_)].getRow(layerIndex_);
        if (row == nullptr)
            return;
        row->clips = state.clips;                          // value copy
        if (state.runtime.has_value())
            if (Layer* layer = comp->getLayer(layerIndex_))
                layer->setRuntime(*state.runtime);
        if (mediaHook_)
            for (const auto& c : row->clips)
                if (c.has_value())
                    mediaHook_(*c);                       // reconnect restored media
        // Family coverage (media-leak fix, L1): dispose every clip this
        // apply() is leaving behind whose id isn't retained anywhere in the
        // LANDED row (clip ids are unique per the whole composition, never
        // reshuffled within one row's own clear/restore, so a plain
        // per-id membership check is sufficient here — no cross-cell "moved,
        // not removed" case like SwapClipsCmd's).
        if (disposeHook_)
            for (const auto& lc : leaving.clips)
            {
                if (!lc.has_value()) continue;
                bool stillPresent = false;
                for (const auto& sc : state.clips)
                    if (sc.has_value() && sc->id == lc->id) { stillPresent = true; break; }
                if (!stillPresent)
                    disposeHook_(*lc);
            }
    }
    void runFenced(const std::function<void()>& m) { if (fence_) fence_(m); else if (m) m(); }

    CompositionResolver resolver_;
    DeckFenceHook fence_;
    ClipMediaHook mediaHook_;
    ClipMediaDisposeHook disposeHook_;
    int deckIndex_, layerIndex_;
    LayerClipsSnapshot before_, after_;
    std::string description_;
};

// ===========================================================================
// Undo v1 step 5 — layer ops (#13-18, #20).
// ===========================================================================

// DeckFenceHook is defined in ClipCommands.h (included above), needed by
// SetColumnCountCmd / RemoveColumnCmd / ClearLayerClipsCmd too, since
// 2026-07-28's family-fence fix. The layer ops below (#13-18, #20) reuse it
// unchanged: reallocating/erasing/moving deck->layers cannot be torn-read
// mid-frame (spec risk #1).

// ClearActiveClipCmd: the layer X-button clear (#13) — Layer::clearActiveClip
// resets ONLY the shared layer's tuple (lane bf9b: Composition::layers[i]). The
// clips ROWS are untouched, so this is a runtime-only before/after restore and
// needs NO GL fence (the tuple is one atomic word: undo / redo is one release
// store, lane tsan). MUTATE-THEN-PUSH like TriggerClipCmd: the FIRST execute()
// (UndoManager::perform) is a no-op, the handler already cleared live; redo
// applies `after`, undo `before` (value restores). NOTE: clearActiveClip
// also sets the (previously) active clip's `playing` flag false; per spec risk
// #5 (runtime playback state in undo is accepted/imperfect) this command does
// not restore that flag — it restores the layer tuple only.
class ClearActiveClipCmd : public Command
{
public:
    ClearActiveClipCmd(ClipLayerResolver resolver, int layerIndex,
                       LayerRuntimeSnapshot before, LayerRuntimeSnapshot after,
                       std::string description)
        : resolver_(std::move(resolver)),
          layerIndex_(layerIndex), before_(before), after_(after),
          description_(std::move(description)) {}

    void execute() override
    {
        if (firstExecute_)
        {
            firstExecute_ = false;   // perform(): the live clear already happened
            return;
        }
        apply(after_);
    }
    void undo() override    { apply(before_); }
    std::string description() const override { return description_; }

private:
    void apply(const LayerRuntimeSnapshot& r)
    {
        Layer* layer = resolver_ ? resolver_(layerIndex_) : nullptr;
        if (layer == nullptr)
            return;
        layer->setRuntime(r);
    }

    ClipLayerResolver resolver_;
    int layerIndex_;
    LayerRuntimeSnapshot before_, after_;
    bool firstExecute_ = true;
    std::string description_;
};

// ToggleLayerFlagCmd: flip one boolean flag of a SHARED layer (#14 bypass, #15 solo, #20
// fold) as one undo unit. Field-level bool write → NO fence, NO merge (spec §3:
// each toggle is its own tiny command). before/after bools make undo/redo a
// plain restore. Addressed by the shared layer index — never captures the raw
// Layer* LayerStrip holds.
class ToggleLayerFlagCmd : public Command
{
public:
    enum class Flag { Bypassed, Solo, Folded };

    ToggleLayerFlagCmd(ClipLayerResolver resolver, int layerIndex,
                       Flag flag, bool before, bool after, std::string description)
        : resolver_(std::move(resolver)),
          layerIndex_(layerIndex), flag_(flag), before_(before), after_(after),
          description_(std::move(description)) {}

    void execute() override { apply(after_); }
    void undo() override    { apply(before_); }
    std::string description() const override { return description_; }

private:
    void apply(bool value)
    {
        Layer* layer = resolver_ ? resolver_(layerIndex_) : nullptr;
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
    int layerIndex_;
    Flag flag_;
    bool before_, after_;
    std::string description_;
};

// AddLayerCmd: layer new/insert (#16) → a shared layer appended to the show
// (Composition::makeLayer + insertLayer: an empty row in EVERY deck, live and
// retired -- lane bf9b). Uses the classic command-owns-the-mutation pattern (the
// handler does NOT pre-mutate; perform() runs execute()), because the add is
// non-idempotent — re-applying a live add in execute() would double-add. The
// added layer is captured on the first execute so redo re-inserts the EXACT same
// layer (deterministic id), not a fresh one with a new auto-id. GL fence:
// insert / erase reallocate Composition::layers and every deck's rows.
class AddLayerCmd : public Command
{
public:
    AddLayerCmd(CompositionResolver compResolver, DeckFenceHook fence, std::string description)
        : compResolver_(std::move(compResolver)), fence_(std::move(fence)),
          description_(std::move(description)) {}

    void execute() override
    {
        runFenced([this]
        {
            Composition* comp = resolve();
            if (comp == nullptr)
                return;
            if (added_.has_value())
            {
                addedIndex_ = comp->insertLayer(addedIndex_, *added_);   // redo: the exact layer captured on first do
            }
            else
            {
                Layer layer = comp->makeLayer();                          // first do: append
                added_ = layer;                                           // capture for redo
                addedIndex_ = comp->insertLayer(comp->getNumLayers(), std::move(layer));
            }
        });
    }

    void undo() override
    {
        runFenced([this]
        {
            if (Composition* comp = resolve())
                comp->eraseLayer(addedIndex_);
        });
    }

    std::string description() const override { return description_; }

private:
    Composition* resolve() { return compResolver_ ? compResolver_() : nullptr; }
    void runFenced(const std::function<void()>& m) { if (fence_) fence_(m); else if (m) m(); }

    CompositionResolver compResolver_;
    DeckFenceHook fence_;
    std::optional<Layer> added_;
    int addedIndex_ = -1;
    std::string description_;
};

// RemoveLayerCmd: layer remove (#17) -- the shared layer AND row `layerIndex` of
// every deck (lane bf9b). Command-owns-the-mutation (the handler snapshots the
// Layer value, does NOT pre-remove; execute() erases). execute() snapshots every
// deck's row by deck id (live and retired) right before erasing; undo re-inserts
// the layer at its index with each snapshot row into every deck that still exists,
// an EMPTY row into any deck the snapshot lacks, and drops snapshot rows of reaped
// decks -- rows == layers everywhere (ruling-bf9b amendment 22). The media hook
// reconnects every restored clip (spec risk #4). GL fence: erase/insert reallocate
// Composition::layers and every deck's rows. Media-leak fix (L1 round 2): execute()
// disposes every occupied cell the erased rows carried.
class RemoveLayerCmd : public Command
{
public:
    RemoveLayerCmd(CompositionResolver compResolver, DeckFenceHook fence,
                   ClipMediaHook mediaHook, ClipMediaDisposeHook disposeHook,
                   int layerIndex, Layer removed, std::string description)
        : compResolver_(std::move(compResolver)), fence_(std::move(fence)),
          mediaHook_(std::move(mediaHook)), disposeHook_(std::move(disposeHook)),
          layerIndex_(layerIndex), removed_(std::move(removed)),
          description_(std::move(description)) {}

    void execute() override
    {
        runFenced([this]
        {
            Composition* comp = resolve();
            if (comp == nullptr)
                return;
            // The show must keep >=1 layer; the handler only builds this command when
            // more than one exists, and linear undo preserves that.
            if (comp->getNumLayers() <= 1 || layerIndex_ < 0 || layerIndex_ >= comp->getNumLayers())
                return;
            rows_.clear();
            auto snap = [this](const Deck& d) {
                if (const ClipRow* row = d.getRow(layerIndex_))
                    rows_.emplace_back(d.id, *row);
            };
            for (const auto& d : comp->decks)
                snap(d);
            for (const auto& d : comp->retiredDecks())
                snap(d);
            comp->eraseLayer(layerIndex_);
            // Family coverage (media-leak fix, L1 round 2): this guard is the ONLY place the removal happens, so the
            // dispose is gated on it too; undo's mediaHook_ call below reconnects them if the layer comes back.
            if (disposeHook_)
                for (const auto& [id, row] : rows_)
                    for (const auto& c : row.clips)
                        if (c.has_value())
                            disposeHook_(*c);
        });
    }

    void undo() override
    {
        runFenced([this]
        {
            Composition* comp = resolve();
            if (comp == nullptr)
                return;
            const int at = comp->insertLayerWithRows(layerIndex_, removed_, rows_);
            if (mediaHook_)
                comp->forEachClip([&](const Clip& c, const ClipSite& site) {
                    if (site.row == at)
                        mediaHook_(c);
                });
        });
    }

    std::string description() const override { return description_; }

private:
    Composition* resolve() { return compResolver_ ? compResolver_() : nullptr; }
    void runFenced(const std::function<void()>& m) { if (fence_) fence_(m); else if (m) m(); }

    CompositionResolver compResolver_;
    DeckFenceHook fence_;
    ClipMediaHook mediaHook_;
    ClipMediaDisposeHook disposeHook_;
    int layerIndex_;
    Layer removed_;
    std::vector<std::pair<uint32_t, ClipRow>> rows_;   // row layerIndex_ of every deck, by deck id
    std::string description_;
};

// MoveLayerCmd: layer move up/down (#18) → Composition::moveLayer (the shared
// layer and row `from` of every deck in step, lane bf9b). Command-owns-the-
// mutation with clean index inverses: execute moves from->to, undo moves to->from
// (moveLayer(to,from) is the exact inverse of moveLayer(from,to)). GL fence:
// moveLayer erases + inserts, reallocating Composition::layers and the rows.
class MoveLayerCmd : public Command
{
public:
    MoveLayerCmd(CompositionResolver compResolver, DeckFenceHook fence,
                 int fromIndex, int toIndex, std::string description)
        : compResolver_(std::move(compResolver)), fence_(std::move(fence)),
          fromIndex_(fromIndex), toIndex_(toIndex),
          description_(std::move(description)) {}

    void execute() override { runFenced([this] { move(fromIndex_, toIndex_); }); }
    void undo() override    { runFenced([this] { move(toIndex_, fromIndex_); }); }
    std::string description() const override { return description_; }

    // P24.13: this command IS a layer reorder — see Command::affectsLayerOrder.
    bool affectsLayerOrder() const override { return true; }

private:
    void move(int from, int to)
    {
        if (Composition* comp = compResolver_ ? compResolver_() : nullptr)
            comp->moveLayer(from, to);
    }
    void runFenced(const std::function<void()>& m) { if (fence_) fence_(m); else if (m) m(); }

    CompositionResolver compResolver_;
    DeckFenceHook fence_;
    int fromIndex_, toIndex_;
    std::string description_;
};

// ===========================================================================
// Undo v1 step 6 — deck ops (#21 new, #22 remove, #24 switch).
// (#23 deck-clear-clips landed in step 4 as a ClearLayerClipsCmd composite.)
// Lane bf9b: a deck is a box of clips; activating one changes only the grid, so no
// deck op cancels a queued trigger any more (plan-bf9b F11) -- except Remove Deck,
// for the triggers queued INTO the deck it removes (cancelPendingInto).
// ===========================================================================

// CompositionResolver is defined in ClipCommands.h (included above).

// Re-point the renderer's active-deck atomic at the CURRENT active deck
// (renderer.setActiveDeck(composition.getActiveDeck())). Injected as a hook so
// SwitchDeckCmd stays renderer-free / headless-testable; no-op in headless.
// Deck ADD/REMOVE do NOT need this: their DeckFenceHook (withDeckDetached) already
// re-points by re-resolving getActiveDeck() after the fenced mutation. SwitchDeckCmd
// does not fence (no vector mutation — just an atomic pointer handoff), so it
// re-points through this lightweight hook instead.
using DeckActivateHook = std::function<void()>;

// AddDeckCmd: deck new (#21) — a deck named "Deck N" with one empty row per
// shared layer (x 12 columns), appended via Composition::appendDeck, which mints
// a fresh deck id (never reused in a session; it REFUSES once the show has used
// every deck number -- ruling-bf9b amendment 7(a): then nothing is added and
// refused() is true), then made the shown deck. Command-owns-the-mutation (like
// AddLayerCmd): the handler does NOT pre-mutate; perform() runs the single fenced
// mutation, because push_back is non-idempotent. The appended deck (minted id
// included) is captured on first execute so redo re-inserts the EXACT same deck.
// GL fence: push_back can reallocate composition->decks -- withDeckDetached fences
// the GL thread AND re-points activeDeck_ after the mutation.
class AddDeckCmd : public Command
{
public:
    AddDeckCmd(CompositionResolver compResolver, DeckFenceHook fence,
               std::string description)
        : compResolver_(std::move(compResolver)), fence_(std::move(fence)),
          description_(std::move(description)) {}

    void execute() override
    {
        runFenced([this]
        {
            Composition* comp = resolve();
            if (comp == nullptr)
                return;
            if (added_.has_value())
            {
                // redo: re-insert the exact deck captured on first execute.
                comp->activeDeckIndex = comp->insertDeckKeepingId(addedIndex_, *added_);
            }
            else if (!refused_)
            {
                // first do: "Deck N", one empty row per shared layer, a freshly minted id; becomes the shown deck.
                priorActiveIndex_ = comp->activeDeckIndex;
                Deck newDeck;
                newDeck.name = "Deck " + std::to_string(comp->decks.size() + 1);
                newDeck.initDefault(comp->getNumLayers());
                addedIndex_ = comp->appendDeck(std::move(newDeck));   // mints deck.id (never 0), push_back
                if (addedIndex_ < 0)
                {
                    refused_ = true;                                  // every deck number used (amendment 7(a))
                    return;
                }
                added_ = comp->decks.back();               // redo re-inserts THIS deck, minted id included
                comp->activeDeckIndex = addedIndex_;
            }
        });
    }

    void undo() override
    {
        runFenced([this]
        {
            Composition* comp = resolve();
            if (comp == nullptr || !added_.has_value())
                return;
            if (addedIndex_ >= 0 && addedIndex_ < static_cast<int>(comp->decks.size())
                && comp->decks[static_cast<size_t>(addedIndex_)].id == added_->id)
                comp->decks.erase(comp->decks.begin() + addedIndex_);
            comp->activeDeckIndex = priorActiveIndex_;      // restore prior shown deck
        });
    }

    std::string description() const override { return description_; }
    bool refused() const { return refused_; }

private:
    Composition* resolve() { return compResolver_ ? compResolver_() : nullptr; }
    void runFenced(const std::function<void()>& m) { if (fence_) fence_(m); else if (m) m(); }

    CompositionResolver compResolver_;
    DeckFenceHook fence_;
    std::optional<Deck> added_;
    int addedIndex_ = -1;
    int priorActiveIndex_ = 0;
    bool refused_ = false;
    std::string description_;
};

// InsertDeckCmd: append a FULLY-FORMED deck — Load Deck (a library file) or
// Duplicate Deck — under a fresh id and make it the shown deck (plan6 §5 A1-d).
// Undoable like New Deck, NOT a whole-model swap. Command-owns-the-mutation: the
// caller stages the deck (validate, re-mint clip ids, open media on the staged
// copy) and does not touch the model; execute() appends it through the fence via
// Composition::appendDeck (fresh deck id) and captures it so redo re-inserts the
// EXACT same deck. Lane bf9b (plan-bf9b F6 + ruling-bf9b amendment 8): a deck with
// MORE rows than the show has layers adds the missing shared layers first -- a row
// whose file carried legacy layer settings (rowSettings[r]) gives its layer those
// settings, any other row a layer as Add Layer makes it -- and every other deck
// gets an empty row for each; undo removes the deck AND those layers. Media: open
// already on first do (the caller ran openMediaForDeck on the staged deck); undo
// disposes every occupied cell of the inserted deck (the app's dispose hook
// re-scans the live model, so a clip id still live elsewhere is never closed),
// redo reconnects them. Refused (nothing added, refused() true) once the show has
// used every deck number (amendment 7(a)). All bodies are fenced.
class InsertDeckCmd : public Command
{
public:
    InsertDeckCmd(CompositionResolver compResolver, DeckFenceHook fence,
                  ClipMediaHook mediaHook, ClipMediaDisposeHook disposeHook,
                  Deck prebuilt, std::string description,
                  std::vector<std::optional<Layer>> rowSettings = {})
        : compResolver_(std::move(compResolver)), fence_(std::move(fence)),
          mediaHook_(std::move(mediaHook)), disposeHook_(std::move(disposeHook)),
          prebuilt_(std::move(prebuilt)), description_(std::move(description)),
          rowSettings_(std::move(rowSettings)) {}

    void execute() override
    {
        runFenced([this]
        {
            Composition* comp = resolve();
            if (comp == nullptr)
                return;
            if (added_.has_value())
            {
                // redo: the exact layers, then the exact deck captured on first execute.
                for (const auto& l : addedLayers_)
                    comp->insertLayer(comp->getNumLayers(), l);
                comp->activeDeckIndex = comp->insertDeckKeepingId(addedIndex_, *added_);
                // Reconnect every occupied cell of the restored deck (undo disposed them).
                if (mediaHook_)
                    for (const auto& row : comp->decks[static_cast<size_t>(comp->activeDeckIndex.load())].rows)
                        for (const auto& c : row.clips)
                            if (c.has_value())
                                mediaHook_(*c);
            }
            else if (!refused_)
            {
                // first do: grow the shared stack to the deck's rows (F6), append under a fresh id, capture, show.
                if (!comp->canMintDeckId())
                {
                    refused_ = true;
                    return;
                }
                priorActiveIndex_ = comp->activeDeckIndex;
                for (int r = comp->getNumLayers(); r < prebuilt_.getNumRows(); ++r)
                {
                    Layer layer = comp->makeLayer();
                    if (r < static_cast<int>(rowSettings_.size()) && rowSettings_[static_cast<size_t>(r)].has_value())
                    {
                        const uint32_t freshId = layer.id;
                        layer = *rowSettings_[static_cast<size_t>(r)];   // the file's legacy row settings
                        layer.id = freshId;                              // never a file id (ids unique per show)
                        layer.setRuntime(LayerRuntimeSnapshot{});
                    }
                    addedLayers_.push_back(layer);
                    comp->insertLayer(comp->getNumLayers(), std::move(layer));
                }
                addedIndex_ = comp->appendDeck(std::move(prebuilt_));   // fresh id; media ALREADY open (the caller)
                added_ = comp->decks.back();                            // capture for redo
                comp->activeDeckIndex = addedIndex_;
            }
        });
    }

    void undo() override
    {
        runFenced([this]
        {
            Composition* comp = resolve();
            if (comp == nullptr || !added_.has_value())
                return;
            if (addedIndex_ >= 0 && addedIndex_ < static_cast<int>(comp->decks.size())
                && comp->decks[static_cast<size_t>(addedIndex_)].id == added_->id)
                comp->decks.erase(comp->decks.begin() + addedIndex_);
            for (size_t k = 0; k < addedLayers_.size(); ++k)
                comp->eraseLayer(comp->getNumLayers() - 1);
            comp->activeDeckIndex = priorActiveIndex_;      // restore prior shown deck
            // The inserted deck is gone from the model: dispose every occupied cell it held.
            if (disposeHook_)
                for (const auto& row : added_->rows)
                    for (const auto& c : row.clips)
                        if (c.has_value())
                            disposeHook_(*c);
        });
    }

    std::string description() const override { return description_; }
    bool refused() const { return refused_; }
    int addedLayerCount() const { return static_cast<int>(addedLayers_.size()); }

private:
    Composition* resolve() { return compResolver_ ? compResolver_() : nullptr; }
    void runFenced(const std::function<void()>& m) { if (fence_) fence_(m); else if (m) m(); }

    CompositionResolver compResolver_;
    DeckFenceHook fence_;
    ClipMediaHook mediaHook_;
    ClipMediaDisposeHook disposeHook_;
    Deck prebuilt_;                     // moved into the model on first execute
    std::optional<Deck> added_;         // the appended deck (minted id included), for redo/undo
    std::vector<Layer> addedLayers_;    // the shared layers the deck's extra rows added (F6)
    int addedIndex_ = -1;
    int priorActiveIndex_ = 0;
    bool refused_ = false;
    std::string description_;
    std::vector<std::optional<Layer>> rowSettings_;
};

// RemoveDeckCmd: deck remove (#22) — removes the deck at deckIndex_ (only when
// more than one deck exists; any deck since plan6 §5 A1-b) and keeps the on-screen
// deck OBJECT shown. Lane bf9b (plan-bf9b F3, ruling-bf9b amendment 4): while a
// shared layer plays from the deck (its active or previous ref names it) the deck is
// RETIRED -- kept, not shown, not saved, its clips at their addresses -- so that clip
// keeps playing until it is replaced; otherwise it is erased. Either way every
// trigger QUEUED into it is cancelled inside the fence (cancelPendingInto). A
// retired deck is reaped -- its media disposed -- by the next fenced edit once no
// ref names it (UndoService::withDeckDetached). Command-owns-the-mutation (like
// RemoveLayerCmd): the handler snapshots the full Deck VALUE + the prior shown index
// and does NOT pre-erase; execute() removes through the fence. undo moves a deck that
// is still retired back (live playheads kept), else re-inserts the snapshot (same
// id) and reconnects its media; the prior shown index is restored. GL fence:
// erase/insert reallocate composition->decks; withDeckDetached re-points the
// renderer's activeDeck_ after the mutation. Media-leak fix (L1 round 2): an ERASED
// deck's occupied cells are disposed on execute (a retired deck's stay open: one of
// them is playing; the reap disposes them).
class RemoveDeckCmd : public Command
{
public:
    RemoveDeckCmd(CompositionResolver compResolver, DeckFenceHook fence,
                  ClipMediaHook mediaHook, ClipMediaDisposeHook disposeHook,
                  int deckIndex, Deck removed, int priorActiveIndex,
                  std::string description)
        : compResolver_(std::move(compResolver)), fence_(std::move(fence)),
          mediaHook_(std::move(mediaHook)), disposeHook_(std::move(disposeHook)),
          deckIndex_(deckIndex), removed_(std::move(removed)),
          priorActiveIndex_(priorActiveIndex), description_(std::move(description))
    {
        // deckIndex_ may be any deck; priorActiveIndex_ is the shown index at push
        // time and is restored verbatim on undo (plan6 §5 A1-b).
    }

    void execute() override
    {
        runFenced([this]
        {
            Composition* comp = resolve();
            if (comp == nullptr)
                return;
            // Composition must keep >=1 deck; the handler only builds this command
            // when more than one exists, and linear undo preserves that invariant.
            if (comp->decks.size() > 1
                && deckIndex_ >= 0 && deckIndex_ < static_cast<int>(comp->decks.size())
                && comp->decks[static_cast<size_t>(deckIndex_)].id == removed_.id)
            {
                cancelPendingInto(*comp, removed_.id);
                std::optional<Deck> erased;
                retired_ = comp->retireOrEraseDeck(deckIndex_, &erased);
                if (deckIndex_ < comp->activeDeckIndex)
                    comp->activeDeckIndex = comp->activeDeckIndex - 1;   // the deck on screen slid down one slot: the same OBJECT stays shown
                else if (comp->activeDeckIndex >= static_cast<int>(comp->decks.size()))
                    comp->activeDeckIndex = static_cast<int>(comp->decks.size()) - 1;   // the shown deck was last: its previous neighbour
                // (deckIndex_ == shown, not last: the next deck slides into the slot — unchanged from today)
                // Family coverage (media-leak fix, L1 round 2): an ERASED deck's clips leave the model for good here
                // (this guard is the ONLY place the removal happens); undo's mediaHook_ loop reconnects them.
                if (disposeHook_ && erased.has_value())
                    for (const auto& row : erased->rows)
                        for (const auto& c : row.clips)
                            if (c.has_value())
                                disposeHook_(*c);
            }
        });
    }

    void undo() override
    {
        runFenced([this]
        {
            Composition* comp = resolve();
            if (comp == nullptr)
                return;
            if (comp->findDeckIndexById(removed_.id) >= 0)
                return;                                     // never removed (a refused execute): nothing to undo
            // Still retired: move it back with its live playheads. Else (erased, or reaped since) re-insert the
            // snapshot under the same id and reconnect its media.
            if (!comp->restoreRetiredDeck(removed_.id, deckIndex_))
            {
                const int at = comp->insertDeckKeepingId(deckIndex_, removed_);
                if (mediaHook_)
                    for (const auto& row : comp->decks[static_cast<size_t>(at)].rows)
                        for (const auto& c : row.clips)
                            if (c.has_value())
                                mediaHook_(*c);
            }
            comp->activeDeckIndex = priorActiveIndex_;      // restore prior shown deck
        });
    }

    std::string description() const override { return description_; }
    bool retired() const { return retired_; }

private:
    Composition* resolve() { return compResolver_ ? compResolver_() : nullptr; }
    void runFenced(const std::function<void()>& m) { if (fence_) fence_(m); else if (m) m(); }

    CompositionResolver compResolver_;
    DeckFenceHook fence_;
    ClipMediaHook mediaHook_;
    ClipMediaDisposeHook disposeHook_;
    int deckIndex_;
    Deck removed_;
    int priorActiveIndex_;
    bool retired_ = false;
    std::string description_;
};

// SwitchDeckCmd: deck switch (#24) — the deck-tab click. Lane bf9b: a switch changes
// only which box the grid shows (index + the activate hook; nothing plays or stops,
// no queue is touched -- plan-bf9b F11, R7). Setting activeDeckIndex is an IDEMPOTENT
// field write, so this is mutate-then-push (the handler's handleDeckSwitch performs
// the live switch; perform() re-applies `after`, a harmless no-op). NO GL fence: a
// switch does not mutate the decks vector. Stale index (deck removed) → safe no-op.
class SwitchDeckCmd : public Command
{
public:
    SwitchDeckCmd(CompositionResolver compResolver, DeckActivateHook activate,
                  int before, int after, std::string description)
        : compResolver_(std::move(compResolver)), activate_(std::move(activate)),
          before_(before), after_(after), description_(std::move(description)) {}

    void execute() override { apply(after_); }
    void undo() override    { apply(before_); }
    std::string description() const override { return description_; }

private:
    void apply(int index)
    {
        Composition* comp = compResolver_ ? compResolver_() : nullptr;
        if (comp == nullptr)
            return;
        if (index < 0 || index >= static_cast<int>(comp->decks.size()))
            return;                             // stale coordinate → safe no-op
        comp->activeDeckIndex = index;
        if (activate_)
            activate_();                        // renderer.setActiveDeck(getActiveDeck())
    }

    CompositionResolver compResolver_;
    DeckActivateHook activate_;
    int before_, after_;
    std::string description_;
};

// RenameDeckCmd: the deck tab menu's Rename Deck... (plan6 §5 A1-c).
// Command-owns-the-mutation: execute() writes `after`, undo() writes `before`,
// re-resolved by deckIndex on every apply; a stale index is a safe no-op. No
// fence and no hooks: a name write never reallocates `decks`, and the GL thread
// never reads deck.name.
class RenameDeckCmd : public Command
{
public:
    RenameDeckCmd(CompositionResolver compResolver, int deckIndex,
                  std::string before, std::string after, std::string description)
        : compResolver_(std::move(compResolver)), deckIndex_(deckIndex),
          before_(std::move(before)), after_(std::move(after)),
          description_(std::move(description)) {}

    void execute() override { apply(after_); }
    void undo() override    { apply(before_); }
    std::string description() const override { return description_; }

private:
    void apply(const std::string& name)
    {
        Composition* comp = compResolver_ ? compResolver_() : nullptr;
        if (comp == nullptr || deckIndex_ < 0 || deckIndex_ >= static_cast<int>(comp->decks.size()))
            return;                             // stale coordinate → safe no-op
        comp->decks[static_cast<size_t>(deckIndex_)].name = name;
    }

    CompositionResolver compResolver_;
    int deckIndex_;
    std::string before_, after_;
    std::string description_;
};
