#pragma once

// s-rta-0926b render lane (R1): when does a layer's clip-chain history change
// hands during a clip-to-clip crossfade?
//
// While a layer crossfades, TWO clip chains render on it every frame: the
// incoming (active) clip's and the outgoing (previous) clip's. Their Echo /
// Freeze / Posterize Time buffer (u_prev_frame) and Screen Split / Frame
// Stutter ring must not be shared, or each chain reads the other's output as
// its own history. At the first frame of every crossfade the compositor hands
// the layer's clip-chain history to the layer's OUTGOING slot
// (LayerStateKey::outgoingChain): the temporal buffer is COPIED (the incoming
// clip starts from the picture the layer was just showing, exactly as after a
// cut) and the frame ring is SWAPPED (the incoming clip's ring starts empty).
// The outgoing chain then keys by the slot for the whole fade.
//
// This detector decides "first frame of a crossfade" from the Layer fields the
// compositor already reads every frame. Pure (no GL, no JUCE), so
// tests/test_crossfade_history.cpp drives it directly; one instance per clip
// chain lives in CompositorEngine (compositor state, not a Layer field).
//
// Threads: Layer fields are written on the message thread and read on the GL
// thread without atomics, like every other Layer field the compositor reads.
// Every observable write ordering of Layer::triggerClipImmediate
// (previousClipColumn = activeClipColumn; activeClipColumn = column;
// crossfadeProgress = 0) is handled: the intermediate states (prev=X, active=X,
// p=1) and (prev=X, active=Y, p=1) are "not running"; the first frame that shows
// p < 1 with the new pair fires. CompositorEngine::advanceCrossfade runs before
// the stages and only increases progress, so one fade never fires twice; a
// zero dt keeps progress equal (not lower), so it never fires a false start.
struct CrossfadeStartDetector
{
    bool wasRunning = false;
    int lastPrev = -1;
    int lastActive = -1;
    float lastProgress = 1.0f;

    // true exactly on the first frame of a crossfade: the layer is crossfading
    // now AND either it was not last frame, or the (previous, active) pair
    // changed (re-trigger mid-fade), or progress went backwards (a PerfState /
    // routine restore). Never true for a cut (progress >= 1) or with no
    // previous clip.
    bool observe(int prev, int active, float progress) noexcept
    {
        const bool running = progress < 1.0f && prev >= 0;
        const bool start = running && (!wasRunning || prev != lastPrev || active != lastActive
                                       || progress < lastProgress);
        wasRunning = running;
        lastPrev = prev;
        lastActive = active;
        lastProgress = progress;
        return start;
    }
};
