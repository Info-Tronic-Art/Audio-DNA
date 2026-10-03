#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "model/Layer.h"
#include "render/LayerClock.h"

using Catch::Approx;

// LayerClock::advanceCrossfade is the ONE clip-to-clip crossfade clock (CompositorEngine::compositeShow ticks it once per
// shared layer per frame). Case (e) of the retired test_deck_clock.cpp, moved here unchanged (plan-bf9b 4.B: DeckClock and
// its cases (a)-(d), (f) were deleted with the off-screen deck clock, lane bf9b). Drives the real header on a real Layer.

TEST_CASE("(e) LayerClock::advanceCrossfade: step = dt / duration, 0.5 s default, clamps, clears previousClipColumn", "[layer_clock]")
{
    Layer layer;
    {
        LayerRuntimeSnapshot rt = layer.runtime();
        rt.previousClipColumn = 0;
        rt.activeClipColumn = 1;
        rt.crossfadeProgress = 0.0f;
        layer.setRuntime(rt);
    }

    SECTION("step = dt / duration")
    {
        layer.transitionSpeed = 4.0f;
        LayerClock::advanceCrossfade(layer, 1.0f);
        CHECK(layer.runtime().crossfadeProgress == Approx(0.25f));
        CHECK(layer.runtime().previousClipColumn == 0);
    }
    SECTION("duration <= 0 uses 0.5 s")
    {
        layer.transitionSpeed = 0.0f;
        LayerClock::advanceCrossfade(layer, 0.125f);
        CHECK(layer.runtime().crossfadeProgress == Approx(0.25f));
    }
    SECTION("clamps at 1.0 and clears previousClipColumn")
    {
        layer.transitionSpeed = 1.0f;
        LayerClock::advanceCrossfade(layer, 5.0f);
        CHECK(layer.runtime().crossfadeProgress == Approx(1.0f));
        CHECK(layer.runtime().previousClipColumn == -1);
    }
    SECTION("no fade in progress: untouched")
    {
        {
            LayerRuntimeSnapshot rt = layer.runtime();
            rt.previousClipColumn = -1;
            layer.setRuntime(rt);
        }
        layer.transitionSpeed = 1.0f;
        LayerClock::advanceCrossfade(layer, 0.5f);
        CHECK(layer.runtime().crossfadeProgress == Approx(0.0f));
    }
}
