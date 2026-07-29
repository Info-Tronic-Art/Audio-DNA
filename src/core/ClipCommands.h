#pragma once
#include "core/Command.h"
#include "model/Layer.h"   // Layer (and Clip)
#include "model/Deck.h"    // Deck (for SwapClipsCmd's numColumns + two-layer reach)
#include <functional>
#include <optional>
#include <string>
#include <utility>

// Injected hooks that keep clip commands decoupled from the renderer/UI so they
// can be unit-tested headless against a bare Composition (spec §7).

// Re-resolve a Layer through the live model by coordinate. Returns nullptr if
// the coordinate no longer resolves (deck/layer removed). Commands NEVER store
// raw Layer*/Clip* — they hold coordinates and re-resolve on every apply.
using ClipLayerResolver = std::function<Layer*(int deckIndex, int layerIndex)>;

// Reconnect renderer-side media (video / image sequence, keyed by clip id) for
// a clip if it is missing. No-op in headless contexts. This is the spec risk #4
// guard: players are never closed today so redo reconnects for free, but the
// guard keeps redo correct if a future wave adds player disposal.
using ClipMediaHook = std::function<void(const Clip& clip)>;

// Re-resolve a Deck through the live model by index. Returns nullptr if the
// index no longer resolves (deck removed). Like ClipLayerResolver, this keeps
// commands pointer-free — SwapClipsCmd holds indices and re-resolves the Deck
// (for numColumns and both affected layers) on every apply.
using ClipDeckResolver = std::function<Deck*(int deckIndex)>;

// GL fence for structure-changing clip mutations (family-fence fix,
// 2026-07-28, FINALIZED round 3 — see .harmony/notebook.md LAW entry): a
// message-thread mutation that grows or reassigns a layer's clips vector can
// reallocate it while the GL thread (unlocked —
// setComponentPaintingEnabled(false)) holds an interior Clip* via
// getActiveClip()/applyClipEffects — a proven UAF (SIGSEGV on Column -> New).
// Injected as a hook so commands stay renderer-free / headless-testable;
// MainComponent binds it to UndoService::withDeckDetached (GL fence,
// validated in build step 1), headless tests pass a pass-through. A null hook
// runs the mutation directly (no fence), which is exactly the headless case.
// Defined HERE (not DeckCommands.h, which #includes this header) because
// SetClipCmd/SwapClipsCmd below now fence too, as of round 3 — DeckCommands.h's
// SetColumnCountCmd/RemoveColumnCmd/ClearLayerClipsCmd and the step-5/6 layer/
// deck commands share this exact same alias via the include (single
// definition, no redeclaration).
using DeckFenceHook = std::function<void(const std::function<void()>&)>;

// SetClipCmd: set or clear a single deck cell, addressed by (deckIndex,
// layerIndex, column). `before`/`after` are value-copied std::optional<Clip>
// snapshots (nullopt = empty cell). execute()/redo apply `after`; undo applies
// `before`. GL fence (round 3, 2026-07-28): apply()'s ensureColumns() call can
// grow the layer's clips vector, and `cell = *state` reassigns an OCCUPIED
// cell's inner vectors (effects, etc.) during undo/redo replay — both are the
// same crash-proven reallocation class the GL thread can observe unlocked.
// Fenced unconditionally on execute/undo/redo, closing the round-1/round-2
// GL-FENCE EXEMPTION this command previously carried (reviewer-ruled blocker
// on the occupied-cell reassignment case; the prior exemption's (a)/(b)/(c)
// preconditions covered only outer vector growth, not whole-Clip reassignment).
class SetClipCmd : public Command
{
public:
    SetClipCmd(ClipLayerResolver resolver, DeckFenceHook fence, ClipMediaHook mediaHook,
               int deckIndex, int layerIndex, int column,
               std::optional<Clip> before, std::optional<Clip> after,
               std::string description)
        : resolver_(std::move(resolver)), fence_(std::move(fence)),
          mediaHook_(std::move(mediaHook)),
          deckIndex_(deckIndex), layerIndex_(layerIndex), column_(column),
          before_(std::move(before)), after_(std::move(after)),
          description_(std::move(description)) {}

    void execute() override { runFenced([this] { apply(after_); }); }
    void undo() override    { runFenced([this] { apply(before_); }); }
    std::string description() const override { return description_; }

private:
    void apply(const std::optional<Clip>& state)
    {
        Layer* layer = resolver_ ? resolver_(deckIndex_, layerIndex_) : nullptr;
        if (layer == nullptr)
            return;
        layer->ensureColumns(column_ + 1);
        auto& cell = layer->clips[static_cast<size_t>(column_)];
        if (state.has_value())
        {
            cell = *state;                      // value copy
            if (mediaHook_) mediaHook_(*state); // reconnect media if missing
        }
        else
        {
            cell.reset();                       // empty cell
        }
    }
    void runFenced(const std::function<void()>& m) { if (fence_) fence_(m); else if (m) m(); }

    ClipLayerResolver resolver_;
    DeckFenceHook fence_;
    ClipMediaHook mediaHook_;
    int deckIndex_, layerIndex_, column_;
    std::optional<Clip> before_, after_;
    std::string description_;
};

// ToggleClipLockCmd: flip a clip's contentLocked flag. Uses bool before/after
// (not a full-clip snapshot) so undo of a lock toggle does not rewind the
// clip's playback / runtime state (spec risk #5).
class ToggleClipLockCmd : public Command
{
public:
    ToggleClipLockCmd(ClipLayerResolver resolver, int deckIndex, int layerIndex,
                      int column, bool before, bool after, std::string description)
        : resolver_(std::move(resolver)), deckIndex_(deckIndex),
          layerIndex_(layerIndex), column_(column),
          before_(before), after_(after), description_(std::move(description)) {}

    void execute() override { apply(after_); }
    void undo() override    { apply(before_); }
    std::string description() const override { return description_; }

private:
    void apply(bool locked)
    {
        Layer* layer = resolver_ ? resolver_(deckIndex_, layerIndex_) : nullptr;
        if (layer == nullptr)
            return;
        if (Clip* clip = layer->getClipAt(column_))
            clip->contentLocked = locked;
    }

    ClipLayerResolver resolver_;
    int deckIndex_, layerIndex_, column_;
    bool before_, after_;
    std::string description_;
};

// SwapClipsCmd: move or swap a clip between two deck cells as one undo unit —
// the drag-name-bar gesture (spec §2 row #3). Snapshots BOTH cells' value-copied
// std::optional<Clip> before/after AND the deck's numColumns before/after,
// because dropping a clip on a far column grows the column count (ensureColumns)
// and undo must restore the prior count. Addressed by coordinates only: the Deck
// and both layers are re-resolved on every apply so nothing dangles across a
// vector reallocation. Ids travel with the clip and renderer players are keyed
// by clip id, so the media hook re-resolves each moved cell for free. GL
// fence (round 3, 2026-07-28): applyCell()'s ensureColumns() calls AND its
// occupied-cell `cell = *state` reassignment are now fenced unconditionally —
// same rationale as SetClipCmd above (a two-occupied-cell swap reassigns
// BOTH cells' inner vectors during replay). One fence covers both applyCell()
// calls plus the numColumns write, per execute/undo/redo.
class SwapClipsCmd : public Command
{
public:
    SwapClipsCmd(ClipDeckResolver deckResolver, DeckFenceHook fence, ClipMediaHook mediaHook,
                 int deckIndex,
                 int srcLayer, int srcColumn, int dstLayer, int dstColumn,
                 std::optional<Clip> srcBefore, std::optional<Clip> srcAfter,
                 std::optional<Clip> dstBefore, std::optional<Clip> dstAfter,
                 int numColumnsBefore, int numColumnsAfter,
                 std::string description)
        : deckResolver_(std::move(deckResolver)), fence_(std::move(fence)),
          mediaHook_(std::move(mediaHook)),
          deckIndex_(deckIndex),
          srcLayer_(srcLayer), srcColumn_(srcColumn),
          dstLayer_(dstLayer), dstColumn_(dstColumn),
          srcBefore_(std::move(srcBefore)), srcAfter_(std::move(srcAfter)),
          dstBefore_(std::move(dstBefore)), dstAfter_(std::move(dstAfter)),
          numColumnsBefore_(numColumnsBefore), numColumnsAfter_(numColumnsAfter),
          description_(std::move(description)) {}

    void execute() override { runFenced([this] { apply(srcAfter_,  dstAfter_,  numColumnsAfter_); }); }
    void undo() override    { runFenced([this] { apply(srcBefore_, dstBefore_, numColumnsBefore_); }); }
    std::string description() const override { return description_; }

private:
    void apply(const std::optional<Clip>& srcState,
               const std::optional<Clip>& dstState, int numColumns)
    {
        Deck* deck = deckResolver_ ? deckResolver_(deckIndex_) : nullptr;
        if (deck == nullptr)
            return;
        applyCell(*deck, srcLayer_, srcColumn_, srcState);
        applyCell(*deck, dstLayer_, dstColumn_, dstState);
        // Restore the column count LAST: applyCell's ensureColumns may grow a
        // layer's clips vector, but numColumns is the authoritative visible
        // count (the grid draws numColumns, not raw clips.size()).
        deck->numColumns = numColumns;
    }

    void applyCell(Deck& deck, int layerIndex, int column,
                   const std::optional<Clip>& state)
    {
        Layer* layer = deck.getLayer(layerIndex);
        if (layer == nullptr)
            return;
        layer->ensureColumns(column + 1);
        auto& cell = layer->clips[static_cast<size_t>(column)];
        if (state.has_value())
        {
            cell = *state;                      // value copy
            if (mediaHook_) mediaHook_(*state); // reconnect media if missing
        }
        else
        {
            cell.reset();                       // empty cell
        }
    }
    void runFenced(const std::function<void()>& m) { if (fence_) fence_(m); else if (m) m(); }

    ClipDeckResolver deckResolver_;
    DeckFenceHook fence_;
    ClipMediaHook mediaHook_;
    int deckIndex_;
    int srcLayer_, srcColumn_, dstLayer_, dstColumn_;
    std::optional<Clip> srcBefore_, srcAfter_, dstBefore_, dstAfter_;
    int numColumnsBefore_, numColumnsAfter_;
    std::string description_;
};
