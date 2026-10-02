#pragma once
#include "model/Deck.h"
#include "render/LayerClock.h"

// DeckClock: the clocks of a deck that is NOT on screen (s-rta-0926b plan4 item 2, Boris 2026-09-26: "finish the
// fade. when we load a new deck that does not touch the clips playing in the layer"). Pure (no GL): the caller
// passes the per-clip media clock (plan4 B2: Renderer::tickMediaClock -- transport math, no decode, no upload).
namespace DeckClock
{
// Advance the clocks of a deck that is not on screen, for every layer the active-deck path would composite: the
// same layer gate as CompositorEngine::compositeDeck (visible, not bypassed, solo rule).
// Ownership: CompositorEngine::compositePersistentLayers already advances EVERY persistent layer's crossfade
// (before its type check) and the media of the types it renders (Layer::canBePersistent) -- never advance those
// twice.
// ClockFn: void(const Clip*, float dt) -- ticks one playable clip's transport, no decode.
template <class ClockFn>
void tick(Deck& deck, float dt, ClockFn&& clock)
{
    bool anySolo = false;
    for (const auto& l : deck.layers)
        if (l.solo) { anySolo = true; break; }

    for (auto& layer : deck.layers)
    {
        if (!layer.visible || layer.bypassed || (anySolo && !layer.solo))
            continue;
        const bool fadeOwnedElsewhere  = layer.persistent;
        const bool mediaOwnedElsewhere = layer.persistent && Layer::canBePersistent(layer.type);
        if (!fadeOwnedElsewhere)
            LayerClock::advanceCrossfade(layer, dt);
        if (mediaOwnedElsewhere)
            continue;
        if (const Clip* c = layer.getActiveClip(); c != nullptr && c->isPlayable())
            clock(c, dt);
        // The outgoing clip runs during a fade, as on screen (CompositorEngine::applyTransition fetches it only
        // while crossfadeProgress < 1 -- so on the frame a fade completes it is not ticked, the same parity).
        if (const auto rt = layer.runtime(); rt.previousClipColumn >= 0 && rt.crossfadeProgress < 1.0f)
            if (const Clip* p = layer.getClipAt(rt.previousClipColumn); p != nullptr && p->isPlayable())
                clock(p, dt);
    }
}
} // namespace DeckClock
