// s-rta-0925 mastersignal Step 1 (S1-T1): SignalDepth.h::applyDepth, the ONE
// function every Master Signal consumer (ConnectionEngine::evaluate,
// MacroBank::updateValues, v1 MappingEngine::processFrame) blends through.
// Pure function, no JUCE -- same shape as test_per_type_autopilot_layout.
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "features/SignalDepth.h"

using Catch::Approx;

TEST_CASE("applyDepth: depth 1.0 returns driven, bit-for-bit", "[signaldepth]")
{
    const float m = 0.1f;
    const float y = 0.7f;
    REQUIRE(applyDepth(m, y, 1.0f) == y);
    // Documents WHY the >=1 guard exists rather than always running the
    // lerp: at m=0.1, y=0.7, d=1.0, the naive lerp m + 1.0f*(y-m) is
    // mathematically equal to y, but the guard makes this exact by
    // construction rather than by floating-point luck.
}

TEST_CASE("applyDepth: depth 0.0 returns manual, bit-for-bit", "[signaldepth]")
{
    REQUIRE(applyDepth(0.1f, 0.7f, 0.0f) == 0.1f);
}

TEST_CASE("applyDepth: depth 0.5 is the midpoint", "[signaldepth]")
{
    REQUIRE(applyDepth(0.1f, 0.7f, 0.5f) == Approx(0.4f).margin(0.0001f));
}

TEST_CASE("applyDepth: out-of-range depth degrades to the nearest guard", "[signaldepth]")
{
    REQUIRE(applyDepth(0.1f, 0.7f, 1.5f) == 0.7f);   // > 1 -> driven
    REQUIRE(applyDepth(0.1f, 0.7f, -0.5f) == 0.1f);  // < 0 -> manual
}
