#pragma once
#include "model/Layer.h"
#include <algorithm>

// LayerClock: a layer's clip-to-clip crossfade clock, pure on Layer (no GL) -- s-rta-0926b plan4 item 2 T1.
// ONE body for every caller: CompositorEngine::advanceCrossfade (the active deck and persistent layers) forwards
// here, and DeckClock::tick (decks that are not on screen) calls it directly.
namespace LayerClock
{
// P14: Advance crossfade progress by the real frame delta. S167-L4b DT-FIX: `speed` here is actually a
// DURATION in seconds (transitionSpeed is a misleading name inherited from the model -- see its slider wiring
// in LayerInspector.cpp/LayerStrip.cpp, both duration-in-seconds UI), so step = dt / duration is the
// frame-rate-independent progress increment: cumulative progress after real elapsed time T is T / duration,
// completing exactly at T == duration regardless of callback rate. Not gated on the render_frame time override:
// crossfadeProgress is persistent per-layer state (like previousClipColumn) that advances every real GL
// callback, so there is no byte-identical-repeat contract covering it -- only the rate matters.
inline void advanceCrossfade(Layer& layer, float dt)
{
    if (layer.crossfadeProgress < 1.0f && layer.previousClipColumn >= 0)
    {
        float speed = layer.transitionSpeed;
        if (speed <= 0.0f) speed = 0.5f; // default transition duration in seconds
        float step = dt / speed;
        layer.crossfadeProgress = std::min(layer.crossfadeProgress + step, 1.0f);
        if (layer.crossfadeProgress >= 1.0f)
            layer.previousClipColumn = -1; // transition complete
    }
}
} // namespace LayerClock
