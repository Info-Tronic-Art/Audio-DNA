#pragma once
#include "model/Composition.h"   // Composition / Deck / Layer / Clip (for inline resolvers)
#include <functional>
#include <vector>

class Renderer;
class DeckView;

// UndoService: the shared plumbing that undo commands lean on.
//
// Three responsibilities (Undo v1, build step 1):
//   1. Coordinate resolution — commands address their target by
//      (deckIndex, layerIndex, column) and re-resolve a fresh pointer through
//      the Composition at execute/undo time. Commands NEVER store raw
//      Clip*/Layer*/Deck* pointers, which dangle across vector reallocation.
//      These resolvers are inline + renderer-free so they can be unit-tested
//      headless against a bare Composition.
//   2. syncAfterModelChange — one shared refresh after a mutation: rebuild or
//      refresh the deck grid. It does NOT re-point the renderer's active deck
//      — deck add/remove do that through their own command hook
//      (withDeckDetached); a deck switch is no command (bf9b S2c).
//   3. withDeckDetached — GL fence for structure-changing mutations: store
//      nullptr into the renderer's active-deck atomic, block on an empty
//      GL-thread job to fence out any in-flight frame, run the mutation, then
//      restore the pointer.
//
// All methods must be called on the message thread (same invariant as
// UndoManager). The collaborators are owned by MainComponent; UndoService only
// holds non-owning pointers.
class UndoService
{
public:
    // What kind of refresh a mutation needs.
    enum class SyncScope
    {
        RuntimeOnly,    // per-cell/per-layer runtime fields — grid refresh only
        Grid            // clip content / structure changed — rebuild the grid
    };

    void setCollaborators(Composition* composition, Renderer* renderer,
                          DeckView* deckView)
    {
        composition_ = composition;
        renderer_ = renderer;
        deckView_ = deckView;
    }

    // --- Coordinate resolution (re-resolved every call; never cached) ---
    // Inline + renderer-free: only touch the Composition, so they link into
    // headless unit tests without pulling in the renderer/UI.
    Deck* resolveDeck(int deckIndex) const
    {
        if (composition_ == nullptr)
            return nullptr;
        if (deckIndex < 0 || deckIndex >= static_cast<int>(composition_->decks.size()))
            return nullptr;
        return &composition_->decks[static_cast<size_t>(deckIndex)];
    }

    // Lane bf9b: a layer is a SHARED layer (Composition::layers) -- no deck in its address.
    Layer* resolveLayer(int layerIndex) const
    {
        return composition_ != nullptr ? composition_->getLayer(layerIndex) : nullptr;
    }

    // A deck box's row (the clips row `row` of deck `deckIndex`).
    ClipRow* resolveRow(int deckIndex, int row) const
    {
        if (Deck* deck = resolveDeck(deckIndex))
            return deck->getRow(row);
        return nullptr;
    }

    Clip* resolveClip(int deckIndex, int row, int column) const
    {
        if (Deck* deck = resolveDeck(deckIndex))
            return deck->getClip(row, column);
        return nullptr;
    }

    // --- Shared post-mutation refresh (defined in UndoService.cpp) ---
    void syncAfterModelChange(SyncScope scope);

    // --- GL fence for structure-changing mutations (defined in UndoService.cpp) ---
    // Runs `mutation` with the renderer's active deck detached and the GL
    // thread fenced. If no renderer is wired, runs `mutation` directly.
    // s-rta-0928b mediaopen: the detach also MARKS the renderer fenced (one atomic word with the deck,
    // Renderer::detachActiveDeckFenced); the restore unmarks it in the same store.
    // NOT reentrant-safe: a nested call would restore the active-deck pointer
    // before the OUTER mutation finishes, briefly re-exposing the model to the
    // GL thread mid-mutation. Callers must never nest a withDeckDetached call
    // inside another's `mutation` — fenced call sites are structured to run
    // sequentially (guarded by a jassert in the .cpp, see 2026-07-28 family-
    // fence fix).
    // Lane bf9b (ruling-bf9b amendment 4(a)): after `mutation` returns and before
    // the restore, every retired deck no layer's active or previous ref names is
    // reaped (Composition::reapRetiredDecks); after the restore the reaped decks go
    // to onDecksReaped (MainComponent disposes their media). So ANY fenced edit reaps
    // -- headless too (no renderer: the same reap, the same hook).
    void withDeckDetached(const std::function<void()>& mutation);

    // Receives the decks a fenced edit reaped, after the fence ends (message thread).
    std::function<void(std::vector<Deck>&&)> onDecksReaped;

    // Lane bf9b fix round: called after a fenced edit that MOVED or RESIZED Composition::layers (message thread,
    // after the fence and onDecksReaped). A Layer* held into the shared stack may dangle then -- Load / Duplicate Deck
    // of a deck wider than the show and Add Layer grow the vector, Remove Layer and their undos shrink it.
    // MainComponent re-points the inspectors here (repointInspectorsAfterStackMove, ui/InspectorRepoint.h; the
    // LayerStrips are re-pointed by the grid rebuild).
    std::function<void()> onLayerStackMoved;

    // Lane bf9b fix stage (s-rta-1003, adoption item 2): called after EVERY OTHER fenced edit -- one that left
    // Composition::layers where it was (message thread, after the fence and onDecksReaped). Such an edit can still
    // destroy or move clips (Layer > Clear Clips, Deck > Clear Clips, a column add / remove), so a Clip* held into a
    // deck row may dangle. MainComponent clears a Clip inspector whose clip the model no longer owns
    // (clearClipInspectorIfUnowned -- the same check onLayerStackMoved's function runs first). So after any fenced
    // edit exactly one of the two hooks runs.
    std::function<void()> onFencedEdit;

private:
    Composition* composition_ = nullptr;
    Renderer* renderer_ = nullptr;
    DeckView* deckView_ = nullptr;
    bool fenceActive_ = false;   // reentrancy guard for withDeckDetached
};
