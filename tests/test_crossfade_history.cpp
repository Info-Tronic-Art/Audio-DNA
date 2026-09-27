// test_crossfade_history.cpp -- s-rta-0926b render lane (R1).
//
// Covers the REAL detector CompositorEngine uses to decide when a layer's
// clip-chain history is handed over to its outgoing slot:
// render/CrossfadeHistory.h (pure, no GL). It is not a mirror of compositor
// logic -- renderLayerStages calls this exact observe() with the layer's
// (previousClipColumn, activeClipColumn, crossfadeProgress).
//
// What it does NOT cover: the GL hand-over itself (copy temporal buffer, swap
// frame ring) and that the outgoing chain uses the slot key -- covered live by
// .harmony/probe-render-state.sh rows r1_temporal, r1_ring, r1_retrigger,
// r1_counts (pixel-decoded frames + /api/state counters).

#include <catch2/catch_test_macros.hpp>
#include "render/CrossfadeHistory.h"

TEST_CASE("CrossfadeStartDetector: a layer that is not crossfading never fires", "[crossfade_history]")
{
    CrossfadeStartDetector det;
    // no previous clip (fresh layer, or a finished fade: advanceCrossfade sets prev = -1)
    REQUIRE_FALSE(det.observe(-1, 0, 1.0f));
    REQUIRE_FALSE(det.observe(-1, 0, 1.0f));
    REQUIRE_FALSE(det.observe(-1, 2, 0.0f));   // progress < 1 but no previous clip
    // previous clip set but progress complete
    REQUIRE_FALSE(det.observe(0, 1, 1.0f));
}

TEST_CASE("CrossfadeStartDetector: the first running frame fires once, rising progress does not",
          "[crossfade_history]")
{
    CrossfadeStartDetector det;
    REQUIRE_FALSE(det.observe(-1, 0, 1.0f));
    REQUIRE(det.observe(0, 1, 0.0f));
    REQUIRE_FALSE(det.observe(0, 1, 0.1f));
    REQUIRE_FALSE(det.observe(0, 1, 0.5f));
    REQUIRE_FALSE(det.observe(0, 1, 0.99f));
    // fade complete: advanceCrossfade clamps to 1 and clears previousClipColumn
    REQUIRE_FALSE(det.observe(-1, 1, 1.0f));
    // a later fade on the same layer fires again (the spare slot is reused)
    REQUIRE(det.observe(1, 0, 0.02f));
}

TEST_CASE("CrossfadeStartDetector: a re-trigger mid-fade fires again", "[crossfade_history]")
{
    CrossfadeStartDetector det;
    REQUIRE(det.observe(0, 1, 0.0f));          // X -> Y starts
    REQUIRE_FALSE(det.observe(0, 1, 0.3f));
    REQUIRE(det.observe(1, 2, 0.0f));          // re-trigger: Y -> Z (pair changed)
    REQUIRE_FALSE(det.observe(1, 2, 0.1f));
    REQUIRE(det.observe(2, 1, 0.05f));         // and back: Z -> Y
    // the first observed progress of the new fade can be HIGHER than the old one
    // (advanceCrossfade has already stepped it by dt / a shorter duration), so the
    // pair change alone must fire
    REQUIRE_FALSE(det.observe(2, 1, 0.10f));
    REQUIRE(det.observe(1, 0, 0.16f));         // Y -> X with a faster transition
}

TEST_CASE("CrossfadeStartDetector: same pair with progress going backwards fires (restore)",
          "[crossfade_history]")
{
    CrossfadeStartDetector det;
    REQUIRE(det.observe(0, 1, 0.0f));
    REQUIRE_FALSE(det.observe(0, 1, 0.6f));
    REQUIRE(det.observe(0, 1, 0.2f));          // PerfState / routine restore rewound the fade
    REQUIRE_FALSE(det.observe(0, 1, 0.3f));
}

TEST_CASE("CrossfadeStartDetector: a cut never fires and ends a running fade", "[crossfade_history]")
{
    CrossfadeStartDetector det;
    REQUIRE(det.observe(0, 1, 0.0f));
    REQUIRE_FALSE(det.observe(0, 1, 0.4f));
    // cut (transitionSpeed <= 0): triggerClipImmediate sets progress 1.0 with a previous clip
    REQUIRE_FALSE(det.observe(1, 2, 1.0f));
    REQUIRE_FALSE(det.wasRunning);
    // the next fade, even with the pair the cut left, is a new start
    REQUIRE(det.observe(1, 2, 0.0f));
}

TEST_CASE("CrossfadeStartDetector: triggerClipImmediate's write ordering fires exactly once, on the last state",
          "[crossfade_history]")
{
    CrossfadeStartDetector det;
    REQUIRE_FALSE(det.observe(-1, 0, 1.0f));   // X alone, settled
    REQUIRE_FALSE(det.observe(0, 0, 1.0f));    // previousClipColumn = activeClipColumn written first
    REQUIRE_FALSE(det.observe(0, 1, 1.0f));    // activeClipColumn = column written next
    REQUIRE(det.observe(0, 1, 0.0f));          // crossfadeProgress = 0 written last: the start
    REQUIRE_FALSE(det.observe(0, 1, 0.0f));    // a second frame with dt = 0 is NOT a new start
}

TEST_CASE("CrossfadeStartDetector: equal progress (dt = 0) never fires", "[crossfade_history]")
{
    CrossfadeStartDetector det;
    REQUIRE(det.observe(3, 4, 0.25f));
    for (int i = 0; i < 5; ++i)
        REQUIRE_FALSE(det.observe(3, 4, 0.25f));
    REQUIRE_FALSE(det.observe(3, 4, 0.26f));
}
