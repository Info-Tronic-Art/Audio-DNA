#include "core/UndoService.h"
#include "model/Composition.h"
#include "render/Renderer.h"
#include "ui/DeckView.h"
#include "ui/InspectorPanel.h"

void UndoService::setCollaborators(Composition* composition, Renderer* renderer,
                                   DeckView* deckView, InspectorPanel* inspector)
{
    composition_ = composition;
    renderer_ = renderer;
    deckView_ = deckView;
    inspector_ = inspector;
}

Deck* UndoService::resolveDeck(int deckIndex) const
{
    if (composition_ == nullptr)
        return nullptr;
    if (deckIndex < 0 || deckIndex >= static_cast<int>(composition_->decks.size()))
        return nullptr;
    return &composition_->decks[static_cast<size_t>(deckIndex)];
}

Layer* UndoService::resolveLayer(int deckIndex, int layerIndex) const
{
    if (Deck* deck = resolveDeck(deckIndex))
        return deck->getLayer(layerIndex);
    return nullptr;
}

Clip* UndoService::resolveClip(int deckIndex, int layerIndex, int column) const
{
    if (Deck* deck = resolveDeck(deckIndex))
        return deck->getClip(layerIndex, column);
    return nullptr;
}

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

    // Renderer active-deck pointer follows deck structure / active-index change.
    if (scope == SyncScope::DeckStructure && renderer_ != nullptr && composition_ != nullptr)
        renderer_->setActiveDeck(composition_->getActiveDeck());

    // Inspector: re-point BY COORDINATES so a reallocated model never leaves it
    // holding a dangling Clip*/Layer*. If the coordinate no longer resolves,
    // fall back to a plain refresh of the current tab.
    if (reinspect.active && inspector_ != nullptr)
    {
        if (reinspect.column >= 0)
        {
            if (Clip* clip = resolveClip(reinspect.deckIndex, reinspect.layerIndex, reinspect.column))
                inspector_->inspectClip(clip);
            else
                inspector_->refresh();
        }
        else
        {
            if (Layer* layer = resolveLayer(reinspect.deckIndex, reinspect.layerIndex))
                inspector_->inspectLayer(layer);
            else
                inspector_->refresh();
        }
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
