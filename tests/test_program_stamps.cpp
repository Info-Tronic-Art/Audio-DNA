// test_program_stamps -- s-rta-0923 lane L2 (review fix b): Program::compile
// must take each continuous breakpoint's x from its own gesture's parallel
// Stamp (Lane.h Gesture::stamps, D1/D7), not reconstruct it from the take's
// sparse TempoMap. See .harmony/specs/s-rta-0923-review-fixes-plan.md lane
// L2 for the full design (the tempo map becomes a reported fallback only,
// via CompileReport::invalid).
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "model/Composition.h"
#include "recording/Take.h"
#include "recording/Program.h"

using Catch::Approx;

namespace
{
    // Layer scope, deck 0 "Deck 1", layer 0 "Layer 1" -- ExactMatch against
    // Composition::initDefault()'s single deck / three layers (test_take.cpp
    // pins the same names at its Program::compile test).
    ControlPath makeOpacityKey()
    {
        ControlPath key;
        key.scope = ControlPath::Scope::Layer;
        key.deck = 0; key.deckName = "Deck 1";
        key.layer = 0; key.layerName = "Layer 1";
        key.control = "scalar";
        key.scalar = "opacity";
        return key;
    }

    // Tempo map = exactly what RecorderClock writes for a steady take: one
    // anchor at 120 BPM. tAt(4) == 2.0, tAt(8) == 4.0 (TempoMap.cpp linear
    // math); sampleAt(t) == 0 for every t with only one anchor (single-anchor
    // rate is 0 -- TempoMap.cpp:90-107).
    void setSteadyTempo(Take& take)
    {
        take.tempo.a = { { 0.0, 0.0, 0, 120.0f, "start" } };
    }

    // Gesture with x(beat) = {0, 4, 8}. The live-clock stamps deliberately
    // drift from the 120-BPM map's tAt(4)=2.0/tAt(8)=4.0 (2.03/4.07) so a
    // test that reads x from the map instead of the stamp is distinguishable.
    Gesture makeGesture(bool withStamps)
    {
        Gesture g;
        g.grip = "held";
        g.curve.pts = {
            { 0.0, 0.0f },
            { 4.0, 0.5f },
            { 8.0, 1.0f },
        };
        if (withStamps)
            g.stamps = {
                { 1, 0.0,  0 },
                { 2, 2.03, 97440 },
                { 3, 4.07, 195360 },
            };
        return g;
    }
}

TEST_CASE("Program::compile takes each breakpoint's x from its own stamp, not the tempo map", "[program][compile][stamps]")
{
    Composition comp;
    comp.initDefault();

    const ControlPath key = makeOpacityKey();

    Take take;
    take.nextSeq = 1;
    setSteadyTempo(take);

    Lane lane;
    lane.key = key;
    lane.kind = Lane::Kind::Continuous;
    lane.gestures = { makeGesture(/*withStamps*/ true) };
    take.lanes[key] = lane;

    {
        auto program = compile(take, comp, DriveClock::Wall);
        REQUIRE(program->report.invalid.empty());
        REQUIRE(program->continuous.size() == 1);
        const auto& cg = program->continuous.front().gestures.front();
        REQUIRE(cg.curve.pts.size() == 3);
        REQUIRE(cg.curve.pts[0].x == Approx(0.0).margin(1e-9));
        REQUIRE(cg.curve.pts[1].x == Approx(2.03).margin(1e-9));
        REQUIRE(cg.curve.pts[2].x == Approx(4.07).margin(1e-9));
        REQUIRE(cg.x0 == Approx(0.0).margin(1e-9));
        REQUIRE(cg.x1 == Approx(4.07).margin(1e-9));
        REQUIRE(program->length == Approx(4.07).margin(1e-9));
    }
    {
        auto program = compile(take, comp, DriveClock::Sample);
        REQUIRE(program->report.invalid.empty());
        const auto& cg = program->continuous.front().gestures.front();
        REQUIRE(cg.curve.pts[0].x == Approx(0.0).margin(1e-9));
        REQUIRE(cg.curve.pts[1].x == Approx(97440.0).margin(1e-9));
        REQUIRE(cg.curve.pts[2].x == Approx(195360.0).margin(1e-9));
    }
    {
        auto program = compile(take, comp, DriveClock::Beat);
        REQUIRE(program->report.invalid.empty());
        const auto& cg = program->continuous.front().gestures.front();
        REQUIRE(cg.curve.pts[0].x == Approx(0.0).margin(1e-9));
        REQUIRE(cg.curve.pts[1].x == Approx(4.0).margin(1e-9));
        REQUIRE(cg.curve.pts[2].x == Approx(8.0).margin(1e-9));
    }
}

TEST_CASE("Program::compile falls back to the tempo map and reports it when a gesture has no parallel stamps", "[program][compile][stamps]")
{
    Composition comp;
    comp.initDefault();

    const ControlPath key = makeOpacityKey();

    Take take;
    take.nextSeq = 1;
    setSteadyTempo(take);

    Lane lane;
    lane.key = key;
    lane.kind = Lane::Kind::Continuous;
    lane.gestures = { makeGesture(/*withStamps*/ false) };
    take.lanes[key] = lane;

    auto program = compile(take, comp, DriveClock::Wall);
    REQUIRE(program->continuous.size() == 1);
    const auto& cg = program->continuous.front().gestures.front();
    REQUIRE(cg.curve.pts[0].x == Approx(0.0).margin(1e-9));
    REQUIRE(cg.curve.pts[1].x == Approx(2.0).margin(1e-9));
    REQUIRE(cg.curve.pts[2].x == Approx(4.0).margin(1e-9));

    REQUIRE(program->report.invalid.size() == 1);
    REQUIRE(program->report.invalid.front().key == key);
    REQUIRE(program->report.invalid.front().reason.find("stamps") != std::string::npos);
}
