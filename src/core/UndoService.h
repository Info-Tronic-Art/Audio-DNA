#pragma once
#include "model/Composition.h"   // Composition / Deck / Layer / Clip (for inline resolvers)
#include <functional>

class Renderer;
class DeckView;
class InspectorPanel;

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
//      refresh the deck grid, re-point the renderer's active deck when deck
//      structure/active index changed, and re-inspect the inspector BY
//      COORDINATES (which fixes the pre-existing dangling-inspector-pointer
//      class).
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
        Grid,           // clip content changed — rebuild the grid
        DeckStructure   // deck/layer/column count or active index changed
    };

    // Which coordinate the inspector should re-point to after the change.
    // column < 0 means inspect the layer rather than a clip.
    struct ReinspectTarget
    {
        bool active = false;
        int deckIndex = -1;
        int layerIndex = -1;
        int column = -1;
    };

    void setCollaborators(Composition* composition, Renderer* renderer,
                          DeckView* deckView, InspectorPanel* inspector)
    {
        composition_ = composition;
        renderer_ = renderer;
        deckView_ = deckView;
        inspector_ = inspector;
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
    void syncAfterModelChange(SyncScope scope, ReinspectTarget reinspect);

    // --- GL fence for structure-changing mutations (defined in UndoService.cpp) ---
    // Runs `mutation` with the renderer's active deck detached and the GL
    // thread fenced. If no renderer is wired, runs `mutation` directly.
    void withDeckDetached(const std::function<void()>& mutation);

private:
    Composition* composition_ = nullptr;
    Renderer* renderer_ = nullptr;
    DeckView* deckView_ = nullptr;
    InspectorPanel* inspector_ = nullptr;
};
