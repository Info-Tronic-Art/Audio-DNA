#pragma once
#include "model/Composition.h"   // Composition / Deck / Layer / Clip (for inline resolvers)
#include <functional>

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
//      — deck add/remove/switch do that through their own command hooks
//      (withDeckDetached / DeckActivateHook).
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

    Layer* resolveLayer(int deckIndex, int layerIndex) const
    {
        if (Deck* deck = resolveDeck(deckIndex))
            return deck->getLayer(layerIndex);
        return nullptr;
    }

    Clip* resolveClip(int deckIndex, int layerIndex, int column) const
    {
        if (Deck* deck = resolveDeck(deckIndex))
            return deck->getClip(layerIndex, column);
        return nullptr;
    }

    // --- Shared post-mutation refresh (defined in UndoService.cpp) ---
    void syncAfterModelChange(SyncScope scope);

    // --- GL fence for structure-changing mutations (defined in UndoService.cpp) ---
    // Runs `mutation` with the renderer's active deck detached and the GL
    // thread fenced. If no renderer is wired, runs `mutation` directly.
    // NOT reentrant-safe: a nested call would restore the active-deck pointer
    // before the OUTER mutation finishes, briefly re-exposing the model to the
    // GL thread mid-mutation. Callers must never nest a withDeckDetached call
    // inside another's `mutation` — fenced call sites are structured to run
    // sequentially (guarded by a jassert in the .cpp, see 2026-07-28 family-
    // fence fix).
    void withDeckDetached(const std::function<void()>& mutation);

private:
    Composition* composition_ = nullptr;
    Renderer* renderer_ = nullptr;
    DeckView* deckView_ = nullptr;
    bool fenceActive_ = false;   // reentrancy guard for withDeckDetached
};
