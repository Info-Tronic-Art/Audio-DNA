#include "core/UndoService.h"
#include "render/Renderer.h"
#include "ui/DeckView.h"
#include <juce_events/juce_events.h>   // MessageManager (guarded jassert, like UndoManager)

// setCollaborators + resolveDeck/Layer/Clip are inline in UndoService.h (they
// touch only the Composition, so they stay renderer-free and headless-testable).
// The methods below reach into the renderer / UI, so they live here where the
// heavy headers are available and only the app links against them.

void UndoService::syncAfterModelChange(SyncScope scope)
{
    // Deck grid: runtime-only changes just repaint; anything else rebuilds.
    if (deckView_ != nullptr)
    {
        if (scope == SyncScope::RuntimeOnly)
            deckView_->refresh();
        else
            deckView_->rebuildGrid();
    }

    // (Deck ADD/REMOVE/SWITCH re-point the renderer's active deck through their
    // OWN command hooks — withDeckDetached re-resolves getActiveDeck() after the
    // fenced mutation, SwitchDeckCmd via DeckActivateHook — never through this
    // helper, so there is no active-deck re-point here.)
    //
    // Inspector re-pointing after a mutation is handled by refreshAfterUndoRedo
    // (EffectScope-aware setClip/setLayer) at the call sites that need it, not
    // here.
}

void UndoService::withDeckDetached(const std::function<void()>& mutation)
{
    // Reentrancy guard (family-fence fix, 2026-07-28): withDeckDetached is NOT
    // safe to nest — a nested call would restore the renderer's active-deck
    // pointer before the OUTER mutation finishes, briefly re-exposing the model
    // to the GL thread mid-mutation. All fenced call sites are structured to
    // run sequentially (never nested); this catches an accidental future
    // nesting. Guarded like the lane's other invariant asserts: enforced
    // whenever a MessageManager exists (the app), skipped headless so Catch2
    // never aborts on it.
    jassert(juce::MessageManager::getInstanceWithoutCreating() == nullptr || !fenceActive_);
    fenceActive_ = true;

    // RAII (reviewer fix-round 1, 2026-07-28): `mutation` reallocates model
    // vectors (that is the whole point of this fence), so a throwing mutation
    // (e.g. bad_alloc) is real, not theoretical. A plain post-mutation reset/
    // restore would skip on an exceptional exit, leaving the reentrancy guard
    // armed forever AND the renderer permanently deck-less. This guard resets
    // fenceActive_ on EVERY exit path (normal, exceptional, and the GL-thread-
    // absent fallback below) unconditionally.
    struct FenceResetGuard
    {
        bool& flag;
        ~FenceResetGuard() { flag = false; }
    } fenceReset{ fenceActive_ };

    // Lane bf9b (amendment 4(a)): the decks this edit reaps, handed to onDecksReaped once the fence has ended.
    std::vector<Deck> reaped;
    auto handOver = [this, &reaped] {
        if (!reaped.empty() && onDecksReaped)
            onDecksReaped(std::move(reaped));
    };

    if (renderer_ == nullptr)
    {
        if (mutation)
            mutation();
        if (composition_ != nullptr)
            reaped = composition_->reapRetiredDecks();
        handOver();
        return;   // fenceReset resets fenceActive_ on scope exit
    }

    Deck* saved = renderer_->getFenceToken();
    // s-rta-0928b mediaopen: no deck AND fenced, in ONE store (Renderer's FencedPtrSlot, adoption P3). The GL thread
    // counts a fenced deck-less frame (fence_hold_frames / fence_black_frames); restoreDeck's setActiveDeck below ends
    // the fence in the same single store that restores the deck.
    renderer_->detachActiveDeckFenced();

    // RAII: guarantees the renderer's active deck is re-pointed on EVERY exit
    // path (normal or exceptional) — same reasoning as fenceReset above.
    struct ActiveDeckRestoreGuard
    {
        Renderer* renderer;
        Composition* composition;
        Deck* saved;
        ~ActiveDeckRestoreGuard()
        {
            // Restore by re-resolving through the composition so deck add/
            // remove (which may reallocate the decks vector) leaves the
            // renderer pointing at the current active deck, not a stale
            // address.
            renderer->setActiveDeck(composition != nullptr ? composition->getActiveDeck() : saved);
        }
    };

    {
        // The restore runs at the end of THIS scope (before the hand-over below): the reap happens inside the
        // fence, its media disposal after it.
        ActiveDeckRestoreGuard restoreDeck{ renderer_, composition_, saved };

        // Fence: block until the GL thread finishes any in-flight frame reading the
        // deck. With setComponentPaintingEnabled(false) the GL thread never takes
        // the message-manager lock, so blocking here from the message thread cannot
        // deadlock (validated empirically, Undo v1 build step 1).
        renderer_->getContext().executeOnGLThread([](juce::OpenGLContext&) {}, true);

        if (mutation)
            mutation();
        if (composition_ != nullptr)
            reaped = composition_->reapRetiredDecks();
    }
    handOver();

    // fenceReset runs on scope exit.
}
