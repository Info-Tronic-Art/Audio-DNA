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
// ClockFn: void(const Clip*, float dt) -- ticks one playable clip's transport, no decode.
// Lane tsan (s-rta-1002): ONE tuple load per layer; the fade tick publishes from it (LayerClock::tick, adopt-on-fail)
// and the active / outgoing clips come from the same tuple. Returns the number of adopts (render_tuple_adopts).
template <class ClockFn>
int tick(Deck& deck, float dt, ClockFn&& clock)
{
    int adopts = 0;
    bool anySolo = false;
    for (const auto& l : deck.layers)
        if (l.solo) { anySolo = true; break; }

    for (auto& layer : deck.layers)
    {
        if (!layer.visible || layer.bypassed || (anySolo && !layer.solo))
            continue;
        LayerRuntimeSnapshot rt = layer.runtime();
        if (!LayerClock::tick(layer, rt, dt)) ++adopts;
        if (const Clip* c = layer.getClipAt(rt.activeClipColumn); c != nullptr && c->isPlayable())
            clock(c, dt);
        // The outgoing clip runs during a fade, as on screen (CompositorEngine::applyTransition fetches it only
        // while crossfadeProgress < 1 -- so on the frame a fade completes it is not ticked, the same parity).
        if (rt.previousClipColumn >= 0 && rt.crossfadeProgress < 1.0f)
            if (const Clip* p = layer.getClipAt(rt.previousClipColumn); p != nullptr && p->isPlayable())
                clock(p, dt);
    }
    return adopts;
}
} // namespace DeckClock
