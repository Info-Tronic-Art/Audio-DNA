// test_recorder_double_touch -- s-rta-0923 lane L3 (addendum 4a):
// PerformanceRecorder::touch() must not silently overwrite an
// already-open gesture on the same key. A second touch() closes the
// open gesture EXACTLY at the current stamp (as release() would) before
// opening the new one -- see .harmony/specs/s-rta-0923-review-fixes-plan.md.
//
// Standalone target (headless, same dependency set as test_take) --
// self-contained per the lane's file-ownership rule; does not touch the
// shared test_take.cpp/test_take target.
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "model/Composition.h"
#include "recording/Take.h"
#include "recording/PerformanceRecorder.h"
#include "recording/RecorderClock.h"
#include "analysis/FeatureSnapshot.h"

using Catch::Approx;

namespace
{
    ControlPath layerKey(int layer, const std::string& control)
    {
        ControlPath k;
        k.scope = ControlPath::Scope::Layer;
        k.deckRelative = true;
        k.layer = layer;
        k.control = control;
        return k;
    }

    FeatureSnapshot makeSnap(float bpm, float phase)
    {
        FeatureSnapshot s;
        s.clear();
        s.bpm = bpm;
        s.beatPhase = phase;
        return s;
    }
}

TEST_CASE("PerformanceRecorder::touch() closes an already-open gesture exactly instead of discarding it", "[performancerecorder][doubletouch]")
{
    Composition comp; comp.initDefault();
    RecorderClock clock;
    PerformanceRecorder rec;

    const FeatureSnapshot snap = makeSnap(120.0f, 0.0f);
    double wall = 0.0;
    uint64_t samples = 0;
    clock.tick(snap, wall, samples);   // seed t=0

    juce::File scratchFolder = juce::File::getSpecialLocation(juce::File::tempDirectory)
        .getChildFile("s_rta_0923_l3_test_scratch");
    REQUIRE(rec.start(comp, clock, scratchFolder));

    const ControlPath key = [] { auto k = layerKey(1, "scalar"); k.scalar = "opacity"; return k; }();

    // touch(k,"held") @0.0
    rec.touch(key, "held");

    // set 0.1 @0.1 -- past the 50ms coalescing window from touch()
    wall = 0.1; ++samples; clock.tick(snap, wall, samples);
    rec.set(key, 0.1f);

    // set 0.5 @0.3 -- past the 50ms window from the 0.1 set()
    wall = 0.3; ++samples; clock.tick(snap, wall, samples);
    rec.set(key, 0.5f);

    // touch(k,"decaying") @0.4 with NO preceding release() -- the case
    // under test: this must close the "held" gesture exactly, not
    // discard it.
    wall = 0.4; ++samples; clock.tick(snap, wall, samples);
    rec.touch(key, "decaying");

    // set 0.9 @0.5
    wall = 0.5; ++samples; clock.tick(snap, wall, samples);
    rec.set(key, 0.9f);

    // release @0.7
    wall = 0.7; ++samples; clock.tick(snap, wall, samples);
    rec.release(key);

    Take take = rec.stop(comp);
    REQUIRE(take.lanes.count(key) == 1);
    const auto& lane = take.lanes.at(key);
    REQUIRE(lane.kind == Lane::Kind::Continuous);
    REQUIRE(lane.gestures.size() == 2);

    const auto& g0 = lane.gestures[0];
    REQUIRE(g0.grip == "held");
    REQUIRE(g0.curve.pts.size() == 3);
    REQUIRE(g0.curve.pts.back().y == Approx(0.5f));
    REQUIRE(g0.stamps.size() == 3);
    REQUIRE(g0.stamps.back().t == Approx(0.4));

    const auto& g1 = lane.gestures[1];
    REQUIRE(g1.grip == "decaying");
    REQUIRE(g1.curve.pts.size() == 2);   // 0.9 + the synthesized exact end
    REQUIRE(g1.stamps.front().t == Approx(0.5));

    // seqs strictly increasing across g0 then g1 -- one shared capture
    // sequence, never reset or reordered by the close-and-reopen.
    for (size_t i = 1; i < g0.stamps.size(); ++i)
        REQUIRE(g0.stamps[i].seq > g0.stamps[i - 1].seq);
    REQUIRE(g1.stamps.front().seq > g0.stamps.back().seq);
    for (size_t i = 1; i < g1.stamps.size(); ++i)
        REQUIRE(g1.stamps[i].seq > g1.stamps[i - 1].seq);
}
