#pragma once
#include "model/Layer.h"
#include <algorithm>

// LayerClock: a layer's clip-to-clip crossfade clock, pure on Layer (no GL) -- s-rta-0926b plan4 item 2 T1.
// ONE body for every caller: CompositorEngine (the active deck) and DeckClock::tick (decks
// that are not on screen) call tick() with the tuple they loaded once for the layer this frame.
namespace LayerClock
{
// P14: the tuple one frame later -- pure. S167-L4b DT-FIX: `transitionSpeed` is actually a DURATION in seconds
// (transitionSpeed is a misleading name inherited from the model -- see its slider wiring in LayerInspector.cpp/
// LayerStrip.cpp, both duration-in-seconds UI), so step = dt / duration is the frame-rate-independent progress
// increment: cumulative progress after real elapsed time T is T / duration, completing exactly at T == duration
// regardless of callback rate. Not gated on the render_frame time override: crossfadeProgress is persistent
// per-layer state (like previousClipColumn) that advances every real GL callback, so there is no byte-identical-
// repeat contract covering it -- only the rate matters.
inline LayerRuntimeSnapshot advanced(LayerRuntimeSnapshot rt, float transitionSpeed, float dt)
{
    if (rt.crossfadeProgress < 1.0f && rt.previousClipColumn >= 0)
    {
        float speed = transitionSpeed;
        if (speed <= 0.0f) speed = 0.5f; // default transition duration in seconds
        const float step = dt / speed;
        rt.crossfadeProgress = std::min(rt.crossfadeProgress + step, 1.0f);
        if (rt.crossfadeProgress >= 1.0f)
            rt.previousClipColumn = -1; // transition complete
    }
    return rt;
}

// Lane tsan (s-rta-1002; ruling amendment 7): publish one fade tick of the tuple `rt` the caller loaded for this
// layer this frame. ONE compare-exchange, never a loop: the GL thread never waits. On success rt becomes the
// advanced tuple; if a trigger landed since the load, the CAS fails and rt becomes the trigger's tuple, un-advanced
// (adopt-on-fail: the GL thread never overwrites a trigger; the new fade starts one frame later). Returns false
// exactly on an adopt. Nothing to advance (no fade) = no CAS, true.
inline bool tick(Layer& layer, LayerRuntimeSnapshot& rt, float dt)
{
    const LayerRuntimeSnapshot next = advanced(rt, layer.transitionSpeed, dt);
    if (next == rt)
        return true;
    LayerRuntimeSnapshot seen = rt;
    if (layer.casRuntime(seen, next))
    {
        rt = next;
        return true;
    }
    rt = seen;
    return false;
}

// Load + tick, for a caller that has no tuple of its own this frame (tests). Returns tick()'s result.
inline bool advanceCrossfade(Layer& layer, float dt)
{
    LayerRuntimeSnapshot rt = layer.runtime();
    return tick(layer, rt, dt);
}
} // namespace LayerClock
