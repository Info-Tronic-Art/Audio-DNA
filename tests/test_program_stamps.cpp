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
    // math). A single-anchor map has no segment to derive a slope from, so
    // sampleAt(t, nominalRate) extrapolates at nominalRate for every t
    // (TempoMap.cpp:81-108, s-rta-0928b plan-sampleat A1) -- not exercised
    // by the two cases below, since both gestures carry parallel stamps and
    // compile() reads x straight off the stamp (see the stampless-fallback
    // TEST_CASEs below for the Sample-clock nominalRate path).
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

// === s-rta-0928b plan-sampleat: TempoMap::sampleAt's nominalRate fallback (FILED by tempo.md
// section 8, executed here) -- Program.cpp:451 (convertBeatX) is the only caller, reached only by
// a stampless gesture on the Sample clock. A2 exercises it through compile(); A3/A4 exercise
// TempoMap::sampleAt directly. ===

TEST_CASE("Program::compile's Sample-clock fallback uses nominalRate for a single-anchor tempo map", "[program][compile][stamps][tempomap]")
{
    // plan-sampleat A2: a single-anchor map has no segment to derive its own slope from, so the
    // fallback must extrapolate at nominalRate (48 kHz here) rather than freezing at the anchor's
    // sample. Before the fix, all three points below compiled to 150016 (the anchor's own sample).
    Composition comp;
    comp.initDefault();

    const ControlPath key = makeOpacityKey();

    Take take;
    take.nextSeq = 1;
    take.tempo.a = { { 0.0, 0.0, 150016, 120.0f, "start" } };

    Lane lane;
    lane.key = key;
    lane.kind = Lane::Kind::Continuous;
    lane.gestures = { makeGesture(/*withStamps*/ false) };   // beats {0, 4, 8}, no stamps
    take.lanes[key] = lane;

    auto program = compile(take, comp, DriveClock::Sample, {}, /*nominalRate*/ 48000.0);
    REQUIRE(program->continuous.size() == 1);
    const auto& cg = program->continuous.front().gestures.front();
    REQUIRE(cg.curve.pts[0].x == Approx(150016.0).margin(1e-9));
    REQUIRE(cg.curve.pts[1].x == Approx(246016.0).margin(1e-9));
    REQUIRE(cg.curve.pts[2].x == Approx(342016.0).margin(1e-9));
}

TEST_CASE("TempoMap::sampleAt uses nominalRate under 1 s, its own slope at 1 s or more", "[tempomap]")
{
    // plan-sampleat A3: a short segment's own delivered-sample slope is noisy -- the diagnosis's
    // 12 ms start/lock pair extrapolated at ~85,000 samples/s (tempo0-diag.md line 17). Below,
    // the unconditional old slope would be (151037-150016)/0.012 = ~85,083 samples/s; the >= 1 s
    // rule instead falls back to nominalRate.
    {
        TempoMap tempo;
        tempo.a = {
            { 0.0,   0.0, 150016, 0.0f,   "start" },
            { 0.012, 0.0, 151037, 120.0f, "lock"  },
        };
        REQUIRE(static_cast<double>(tempo.sampleAt(1.0, /*nominalRate*/ 48000.0))
                == Approx(151037.0 + 0.988 * 48000.0).margin(1.0));
    }

    // plan-sampleat A4: a segment spanning >= 1 s still uses its OWN slope (44,100 samples/s
    // here), never nominalRate (48,000) -- the rule is not "always nominal".
    {
        TempoMap tempo;
        tempo.a = {
            { 0.0, 0.0, 0,     120.0f, "start"    },
            { 2.0, 0.0, 88200, 120.0f, "periodic" },
        };
        REQUIRE(tempo.sampleAt(1.0, /*nominalRate*/ 48000.0) == 44100);
    }
}
