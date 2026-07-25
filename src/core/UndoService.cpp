#include "core/UndoService.h"
#include "render/Renderer.h"
#include "ui/DeckView.h"
#include "ui/InspectorPanel.h"

// setCollaborators + resolveDeck/Layer/Clip are inline in UndoService.h (they
// touch only the Composition, so they stay renderer-free and headless-testable).
// The methods below reach into the renderer / UI, so they live here where the
// heavy headers are available and only the app links against them.

void UndoService::syncAfterModelChange(SyncScope scope)
{
    syncAfterModelChange(scope, ReinspectTarget{});
}

void UndoService::syncAfterModelChange(SyncScope scope, ReinspectTarget reinspect)
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

    // Inspector: re-point BY COORDINATES so a reallocated model never leaves it
    // holding a dangling Clip*/Layer*. When the coordinate no longer resolves,
    // inspect nullptr (a null-safe clear) rather than refreshing a stale pointer.
    //
    // DEAD PATH (Undo v1 — kept, never exercised): no caller constructs an active
    // ReinspectTarget; every syncAfterModelChange call passes the default
    // (active == false), and refreshAfterUndoRedo re-points the inspectors itself
    // with EffectScope-aware setClip/setLayer. REVIVAL HAZARD: the inspectClip
    // below passes NO EffectScope, so it defaults to EffectScope::none() → the
    // reinspected clip's effect stack would be scope-None → an effect
    // add/remove/bypass on it becomes SILENTLY un-undoable (resolveEffectVector
    // returns nullptr). Before wiring an active ReinspectTarget, pass the matching
    // EffectScope::clip(...)/layer(...) into inspectClip/inspectLayer here.
    if (reinspect.active && inspector_ != nullptr)
    {
        if (reinspect.column >= 0)
            inspector_->inspectClip(resolveClip(reinspect.deckIndex, reinspect.layerIndex, reinspect.column));
        else
            inspector_->inspectLayer(resolveLayer(reinspect.deckIndex, reinspect.layerIndex));
    }
}

void UndoService::withDeckDetached(const std::function<void()>& mutation)
{
    if (renderer_ == nullptr)
    {
        if (mutation)
            mutation();
        return;
    }

    Deck* saved = renderer_->getActiveDeck();
    renderer_->setActiveDeck(nullptr);

    // Fence: block until the GL thread finishes any in-flight frame reading the
    // deck. With setComponentPaintingEnabled(false) the GL thread never takes
    // the message-manager lock, so blocking here from the message thread cannot
    // deadlock (validated empirically, Undo v1 build step 1).
    renderer_->getContext().executeOnGLThread([](juce::OpenGLContext&) {}, true);

    if (mutation)
        mutation();

    // Restore by re-resolving through the composition so deck add/remove (which
    // may reallocate the decks vector) leaves the renderer pointing at the
    // current active deck, not a stale address.
    renderer_->setActiveDeck(composition_ != nullptr ? composition_->getActiveDeck() : saved);
}
