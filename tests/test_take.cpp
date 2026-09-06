// test_take -- s168 step 1: the headless recorder core (s167 spec section
// 5, Build order row 1). Covers verify items (a)-(g) of that row: Take's
// JSON round-trip, the v1 loader, the "future fixture" byte-preservation
// contract, RecorderClock's beat integration, PerformanceRecorder's
// coalescing + AutomationCurve::eval, Program::compile's resolution
// tri-state, and Player's discrete/continuous dispatch.
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "model/Composition.h"
#include "recording/Take.h"
#include "recording/Program.h"
#include "recording/Player.h"
#include "recording/PerformanceRecorder.h"
#include "recording/RecorderClock.h"
#include "analysis/FeatureSnapshot.h"
#include <algorithm>
#include <random>
#include <vector>

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

    // A recording Sink (D5) for exercising Player in isolation: records
    // every dispatch and can be told to refuse the next touch()/set() call
    // (D8's TOUCH override -- "a more deliberate hand already owns it").
    struct FakeSink : Sink
    {
        std::vector<Fired> fired;
        std::vector<std::pair<ControlPath, std::string>> touches;
        std::vector<std::pair<ControlPath, float>> sets;
        std::vector<ControlPath> releases;
        bool refuseNextTouch = false;
        bool refuseNextSet = false;

        bool fire(const Fired& f) override { fired.push_back(f); return true; }

        bool touch(const ControlPath& k, const std::string& grip) override
        {
            touches.emplace_back(k, grip);
            if (refuseNextTouch) { refuseNextTouch = false; return false; }
            return true;
        }

        bool set(const ControlPath& k, float v) override
        {
            sets.emplace_back(k, v);
            if (refuseNextSet) { refuseNextSet = false; return false; }
            return true;
        }

        void release(const ControlPath& k) override { releases.push_back(k); }
    };
}

// === (a) JSON round-trip lane-by-lane with stable seqs ===

TEST_CASE("Take JSON round-trip is lane-by-lane stable, seqs unchanged", "[take]")
{
    Take take;
    take.nextSeq = 1;

    const ControlPath discreteKey = layerKey(0, "activeClip");
    {
        Lane lane;
        lane.key = discreteKey;
        lane.kind = Lane::Kind::Discrete;
        DiscretePoint p1; p1.s = { take.nextSeq++, 1.0, 100 }; p1.beat = 2.0; p1.bpm = 120.0f; p1.v = 3;
        DiscretePoint p2; p2.s = { take.nextSeq++, 2.0, 200 }; p2.beat = 4.0; p2.bpm = 120.0f; p2.v = 5; p2.retrigger = true;
        lane.points = { p1, p2 };
        take.lanes[discreteKey] = lane;
    }

    const ControlPath contKey = layerKey(1, "scalar");
    ControlPath contKeyFull = contKey;
    contKeyFull.scalar = "opacity";
    {
        Lane lane;
        lane.key = contKeyFull;
        lane.kind = Lane::Kind::Continuous;
        Gesture g;
        g.grip = "held";
        g.curve.pts = { { 0.0, 0.2f, Breakpoint::Interp::Linear }, { 1.0, 0.8f, Breakpoint::Interp::Linear } };
        g.stamps = { { take.nextSeq++, 5.0, 500 }, { take.nextSeq++, 6.0, 600 } };
        lane.gestures = { g };
        take.lanes[contKeyFull] = lane;
    }

    const auto originalSeqs = take.nextSeq;

    LoadStats stats;
    auto roundTripped = Take::fromVar(take.toVar(), stats);
    REQUIRE(roundTripped.has_value());
    REQUIRE_FALSE(stats.refused);

    // Idempotent re-serialization proves the round trip is lossless.
    REQUIRE(juce::JSON::toString(take.toVar()) == juce::JSON::toString(roundTripped->toVar()));

    REQUIRE(roundTripped->lanes.count(discreteKey) == 1);
    const auto& rtDiscrete = roundTripped->lanes.at(discreteKey);
    REQUIRE(rtDiscrete.points.size() == 2);
    REQUIRE(rtDiscrete.points[0].s.seq == 1);
    REQUIRE(rtDiscrete.points[1].s.seq == 2);
    REQUIRE(rtDiscrete.points[1].retrigger);

    REQUIRE(roundTripped->lanes.count(contKeyFull) == 1);
    const auto& rtCont = roundTripped->lanes.at(contKeyFull);
    REQUIRE(rtCont.gestures.size() == 1);
    REQUIRE(rtCont.gestures[0].stamps[0].seq == 3);
    REQUIRE(rtCont.gestures[0].stamps[1].seq == 4);
    REQUIRE(rtCont.gestures[0].curve.pts[0].y == Approx(0.2f));
    REQUIRE(rtCont.gestures[0].curve.pts[1].y == Approx(0.8f));

    REQUIRE(roundTripped->nextSeq == originalSeqs);   // stable -- never renumbered on load
}

// === (b) v1 fixture loads to lanes with wallOnly and correct counts ===

TEST_CASE("Take::load bridges a v1 (SessionRecorder) fixture to lanes, wallOnly", "[take][v1]")
{
    juce::File fixture = juce::File(TEST_FIXTURES_DIR).getChildFile("take_v1.json");
    REQUIRE(fixture.existsAsFile());

    LoadStats stats;
    auto take = Take::load(fixture, stats);
    REQUIRE(take.has_value());
    REQUIRE_FALSE(stats.refused);
    REQUIRE(stats.wasV1);

    REQUIRE(std::find(take->unknownFeatures.begin(), take->unknownFeatures.end(), "wallOnly")
            != take->unknownFeatures.end());

    const auto layer0 = layerKey(0, "activeClip");
    const auto layer1 = layerKey(1, "activeClip");
    REQUIRE(take->lanes.count(layer0) == 1);
    REQUIRE(take->lanes.at(layer0).points.size() == 2);
    REQUIRE(take->lanes.at(layer0).points[0].v == 2);
    REQUIRE(take->lanes.at(layer0).points[1].v == 5);

    REQUIRE(take->lanes.count(layer1) == 1);
    REQUIRE(take->lanes.at(layer1).points.size() == 1);
    REQUIRE(take->lanes.at(layer1).points[0].v == 1);

    ControlPath columnKey; columnKey.scope = ControlPath::Scope::Comp; columnKey.control = "columnTrigger";
    REQUIRE(take->lanes.count(columnKey) == 1);
    REQUIRE(take->lanes.at(columnKey).points.size() == 1);
    REQUIRE(take->lanes.at(columnKey).points[0].v == 7);
}

// === (c) a "future" fixture round-trips unknown control/section/feature
//         BYTE-PRESERVED and reports them ===

TEST_CASE("Take::load reports and byte-preserves unknown feature/section/kind", "[take][future]")
{
    juce::File fixture = juce::File(TEST_FIXTURES_DIR).getChildFile("take_v2_future.json");
    REQUIRE(fixture.existsAsFile());

    LoadStats stats;
    auto take = Take::load(fixture, stats);
    REQUIRE(take.has_value());
    REQUIRE_FALSE(stats.refused);
    REQUIRE_FALSE(stats.wasV1);

    REQUIRE(std::find(stats.unknownFeatures.begin(), stats.unknownFeatures.end(), "quantumFlux")
            != stats.unknownFeatures.end());
    REQUIRE(std::find(stats.unknownTopLevelSections.begin(), stats.unknownTopLevelSections.end(),
                       "experimentalSectionFromTheFuture") != stats.unknownTopLevelSections.end());
    REQUIRE(stats.unknownKindLanes == 1);
    REQUIRE(std::find(stats.unknownKindNames.begin(), stats.unknownKindNames.end(), "temporal-blend")
            != stats.unknownKindNames.end());

    // The unknown top-level section's VALUE survives, unexamined.
    REQUIRE(take->unknownTopLevel.count("experimentalSectionFromTheFuture") == 1);
    auto* sectionObj = take->unknownTopLevel.at("experimentalSectionFromTheFuture").getDynamicObject();
    REQUIRE(sectionObj != nullptr);
    REQUIRE(sectionObj->getProperty("note").toString() == "a top-level section this reader has never heard of");

    // The unknown-kind lane's WHOLE content survives, byte-preserved.
    Lane* opaqueLane = nullptr;
    for (auto& [key, lane] : take->lanes)
        if (lane.kind == Lane::Kind::Opaque) opaqueLane = &lane;
    REQUIRE(opaqueLane != nullptr);
    auto* rawObj = opaqueLane->raw.getDynamicObject();
    REQUIRE(rawObj != nullptr);
    REQUIRE(rawObj->getProperty("kind").toString() == "temporal-blend");
    auto* weird = rawObj->getProperty("somethingWeird").getArray();
    REQUIRE(weird != nullptr);
    REQUIRE(weird->size() == 3);
    REQUIRE(static_cast<int>((*weird)[1]) == 2);

    // Re-saving must not disturb any of the above (value-preservation, not
    // just "readable once") -- reload the RE-SAVED var and re-check.
    LoadStats stats2;
    auto reloaded = Take::fromVar(take->toVar(), stats2);
    REQUIRE(reloaded.has_value());
    REQUIRE(reloaded->unknownTopLevel.count("experimentalSectionFromTheFuture") == 1);
    Lane* opaqueLane2 = nullptr;
    for (auto& [key, lane] : reloaded->lanes)
        if (lane.kind == Lane::Kind::Opaque) opaqueLane2 = &lane;
    REQUIRE(opaqueLane2 != nullptr);
    REQUIRE(opaqueLane2->raw.getDynamicObject()->getProperty("kind").toString() == "temporal-blend");
}

// === (d) RecorderClock: scripted beatPhase incl. a resync drop yields
//         monotonic beat + anchors + an unmetered segment at bpm=0 ===

TEST_CASE("RecorderClock keeps beat monotonic across a resync and an unmetered gap", "[recorderclock]")
{
    RecorderClock clock;
    double wall = 0.0;
    uint64_t samples = 0;
    double lastBeat = -1.0;
    bool sawReset = false, sawUnmetered = false, sawLock = false;

    auto step = [&](float bpm, float phase)
    {
        wall += 0.1;
        samples += 4800;
        clock.tick(makeSnap(bpm, phase), wall, samples);
        const auto now = clock.now();
        REQUIRE(now.beat >= lastBeat - 1e-9);   // never decreases
        lastBeat = now.beat;
    };

    clock.tick(makeSnap(120.0f, 0.0f), wall, samples);   // seeds t=0 ("start" anchor)
    lastBeat = clock.now().beat;

    step(120.0f, 0.2f);
    step(120.0f, 0.4f);
    step(120.0f, 0.3f);   // smaller backward jump -- a resync, not a wrap
    step(120.0f, 0.5f);
    step(120.0f, 0.9f);
    step(120.0f, 0.05f);  // ordinary sawtooth wrap (0.05 < 0.9 - 0.5)

    step(0.0f, 0.0f);     // lock lost -- unmetered
    const double frozen = clock.now().beat;
    step(0.0f, 0.0f);
    REQUIRE(clock.now().beat == Approx(frozen));   // frozen while unmetered

    step(125.0f, 0.1f);   // re-locked
    step(125.0f, 0.3f);
    REQUIRE(clock.now().beat > frozen);            // resumes advancing after relock

    for (const auto& anc : clock.tempo().a)
    {
        if (anc.why == "reset") sawReset = true;
        if (anc.why == "unmetered") sawUnmetered = true;
        if (anc.why == "lock") sawLock = true;
    }
    REQUIRE(sawReset);
    REQUIRE(sawUnmetered);
    REQUIRE(sawLock);
}

// === (e) coalescing + AutomationCurve::eval linear/hold ===

TEST_CASE("AutomationCurve::eval -- linear ramps, Hold steps", "[automationcurve]")
{
    AutomationCurve curve;
    curve.pts = {
        { 0.0, 0.0f, Breakpoint::Interp::Linear },
        { 1.0, 1.0f, Breakpoint::Interp::Hold },
        { 2.0, 0.5f, Breakpoint::Interp::Linear }
    };
    REQUIRE(curve.eval(0.5) == Approx(0.5f));    // linear midpoint
    REQUIRE(curve.eval(1.5) == Approx(1.0f));    // Hold: steps, does not ramp toward 0.5
    REQUIRE(curve.eval(2.0) == Approx(0.5f));
}

TEST_CASE("PerformanceRecorder coalesces a burst of set() calls to a bounded gesture, begin/end exact", "[performancerecorder]")
{
    Composition comp; comp.initDefault();
    RecorderClock clock;
    PerformanceRecorder rec;

    const FeatureSnapshot snap = makeSnap(120.0f, 0.0f);
    double wall = 0.0;
    uint64_t samples = 0;
    clock.tick(snap, wall, samples);   // seed t=0

    juce::File scratchFolder = juce::File::getSpecialLocation(juce::File::tempDirectory)
        .getChildFile("s168_test_take_scratch");
    REQUIRE(rec.start(comp, clock, scratchFolder));

    const ControlPath key = [] { auto k = layerKey(1, "scalar"); k.scalar = "opacity"; return k; }();
    rec.touch(key, "held");

    for (int i = 0; i < 100; ++i)
    {
        wall += 0.0003;   // 100 * 0.3ms ~= 30ms total -- all inside one 50ms coalescing window
        ++samples;
        clock.tick(snap, wall, samples);
        rec.set(key, static_cast<float>(i) / 100.0f);
    }

    wall += 0.1;   // past the coalescing window
    clock.tick(snap, wall, samples);
    rec.release(key);

    Take take = rec.stop(comp);
    REQUIRE(take.lanes.count(key) == 1);
    const auto& lane = take.lanes.at(key);
    REQUIRE(lane.kind == Lane::Kind::Continuous);
    REQUIRE(lane.gestures.size() == 1);

    const auto& g = lane.gestures[0];
    REQUIRE(g.curve.pts.size() >= 2);
    REQUIRE(g.curve.pts.size() <= 10);         // bounded -- nowhere near 100 raw set() calls
    REQUIRE(g.curve.pts.front().y == Approx(0.0f));    // begin: first set() value, exact
    REQUIRE(g.curve.pts.back().y == Approx(0.99f));    // end: last set() value before release, exact
}

// === (f) compile: unresolved / rebound-by-position / rebound-by-name ===

TEST_CASE("Program::compile resolves, rebinds-by-position, rebinds-by-name, and reports unresolved", "[program][compile]")
{
    Composition comp;
    comp.initDefault();   // "Deck 1" (id 0), 3 layers: "Layer 1","Layer 2","Layer 3"

    Take take;
    take.nextSeq = 1;

    // Layer-scope activeClip lanes: compile() only needs deck+layer to
    // resolve one of these, so each case below tests DECK/LAYER
    // resolution in isolation.
    auto makeLane = [&](int layerIdx, const std::string& layerName) {
        ControlPath key;
        key.scope = ControlPath::Scope::Layer;
        key.deck = 0; key.deckName = "Deck 1";
        key.layer = layerIdx; key.layerName = layerName;
        key.control = "activeClip";
        Lane lane; lane.key = key; lane.kind = Lane::Kind::Discrete;
        DiscretePoint p; p.s = { take.nextSeq++, 0.0, 0 }; p.v = 1;
        lane.points = { p };
        return lane;
    };

    // `layer` (the index) is part of ControlPath's identity (D2: "positional
    // fields first, names never compared") -- each case below MUST use a
    // distinct index, or two of these would collide onto the same map key
    // and only one would survive in `take.lanes`.
    const std::vector<Lane> lanes = {
        makeLane(0, "Layer 1"),        // deck + layer both index- and name-match -> resolved
        makeLane(1, "WRONG NAME"),     // layer index 1 valid, actual name "Layer 2" differs -> rebound-by-position
        makeLane(9, "Layer 3"),        // layer index 9 invalid, found by name at index 2 -> rebound-by-name
        makeLane(10, "Nonexistent"),   // neither -> unresolved
    };
    for (const auto& lane : lanes)
        take.lanes[lane.key] = lane;

    auto program = compile(take, comp, DriveClock::Wall);
    REQUIRE(program->report.resolvedCount == 1);
    REQUIRE(program->report.reboundByPosition.size() == 1);
    REQUIRE(program->report.reboundByName.size() == 1);
    REQUIRE(program->report.unresolved.size() == 1);

    // The resolved + rebound lanes are compiled IN; unresolved is compiled OUT.
    REQUIRE(program->discrete.size() == 3);
}

// === (g) Player: scripted dt fires discrete events in order; a continuous
//         gesture emits touch/set/release; TOUCH refusal is scoped to one
//         gesture; swap re-seats + releases orphans; stop releases all ===

TEST_CASE("Player fires discrete events in order under jittery dt with a stall", "[player]")
{
    auto prog = std::make_shared<Program>();
    prog->clock = DriveClock::Wall;

    const ControlPath markerKey = [] { ControlPath k; k.scope = ControlPath::Scope::Comp; k.control = "marker"; return k; }();
    const std::vector<double> ats = { 0.10, 0.20, 0.30, 0.40, 0.50 };
    for (size_t i = 0; i < ats.size(); ++i)
    {
        DiscretePoint p; p.s = { static_cast<uint64_t>(i + 1), ats[i], 0 }; p.v = static_cast<int>(i);
        prog->discrete.push_back(Fired{ ats[i], p.s.seq, markerKey, {}, p });
    }

    ControlPath contKey = layerKey(0, "scalar"); contKey.scalar = "opacity";
    ContLane cl; cl.key = contKey;
    ContLane::G g; g.grip = "held";
    g.curve.pts = { { 0.15, 0.0f, Breakpoint::Interp::Linear }, { 0.45, 1.0f, Breakpoint::Interp::Linear } };
    g.x0 = 0.15; g.x1 = 0.45;
    cl.gestures = { g };
    prog->continuous.push_back(cl);

    FakeSink sink;
    Player player(prog);
    player.start(0.0);

    std::mt19937 rng(12345);
    std::uniform_real_distribution<double> jitter(-0.002, 0.002);
    std::vector<double> firedAtPos;
    double pos = 0.0;
    int step = 0;
    while (pos < 0.6)
    {
        double dt = 0.0083 + jitter(rng);
        if (step == 20) dt = 0.4;   // one deliberate 400ms stall
        pos += dt;
        const size_t before = sink.fired.size();
        player.advanceTo(pos, sink);
        for (size_t i = before; i < sink.fired.size(); ++i) firedAtPos.push_back(pos);
        ++step;
    }

    REQUIRE(sink.fired.size() == 5);
    for (size_t i = 0; i < ats.size(); ++i)
    {
        REQUIRE(sink.fired[i].at == Approx(ats[i]));
        REQUIRE(firedAtPos[i] >= ats[i]);   // fires only once pos has reached its `at`
    }

    REQUIRE(sink.touches.size() == 1);
    REQUIRE(sink.touches[0].second == "held");
    REQUIRE(sink.releases.size() == 1);
    REQUIRE_FALSE(sink.sets.empty());
    for (const auto& [k, v] : sink.sets)
    {
        REQUIRE(v >= -0.001f);
        REQUIRE(v <= 1.001f);
    }
    for (size_t i = 1; i < sink.sets.size(); ++i)
        REQUIRE(sink.sets[i].second >= sink.sets[i - 1].second - 1e-4f);   // monotonic ramp
}

TEST_CASE("Player TOUCH refusal displaces a lane for one gesture only", "[player]")
{
    ControlPath key = layerKey(0, "scalar"); key.scalar = "opacity";

    ContLane::G g1; g1.grip = "held"; g1.x0 = 0.0; g1.x1 = 1.0;
    g1.curve.pts = { { 0.0, 0.0f }, { 1.0, 1.0f } };
    ContLane::G g2; g2.grip = "held"; g2.x0 = 2.0; g2.x1 = 3.0;
    g2.curve.pts = { { 2.0, 0.0f }, { 3.0, 1.0f } };

    ContLane cl; cl.key = key; cl.gestures = { g1, g2 };

    auto prog = std::make_shared<Program>();
    prog->clock = DriveClock::Beat;
    prog->continuous.push_back(cl);

    FakeSink sink;
    Player player(prog);
    player.start(0.0);

    player.advanceTo(0.2, sink);
    REQUIRE(sink.touches.size() == 1);
    REQUIRE_FALSE(sink.sets.empty());

    sink.refuseNextSet = true;
    player.advanceTo(0.4, sink);
    const size_t setsAfterRefusal = sink.sets.size();

    player.advanceTo(0.6, sink);   // still inside g1, but displaced -- no further set()
    REQUIRE(sink.sets.size() == setsAfterRefusal);

    player.advanceTo(1.0, sink);   // leaving g1 -- displaced, so no release() either (never held it)
    REQUIRE(sink.releases.empty());

    player.advanceTo(2.2, sink);   // g2: a fresh touch(), not permanently displaced
    REQUIRE(sink.touches.size() == 2);
    REQUIRE(sink.sets.size() > setsAfterRefusal);

    player.advanceTo(3.0, sink);   // leaving g2, held -> release fires
    REQUIRE(sink.releases.size() == 1);
}

TEST_CASE("Player::swap re-seats cursors and releases orphaned gestures", "[player]")
{
    ControlPath keyA = layerKey(0, "scalar"); keyA.scalar = "opacity";
    ControlPath keyB = layerKey(1, "scalar"); keyB.scalar = "opacity";

    auto makeProgWith = [](std::vector<ContLane> lanes) {
        auto p = std::make_shared<Program>();
        p->clock = DriveClock::Beat;
        p->continuous = std::move(lanes);
        return p;
    };

    ContLane::G gA; gA.grip = "held"; gA.x0 = 0.0; gA.x1 = 1.0;
    gA.curve.pts = { { 0.0, 0.0f }, { 1.0, 1.0f } };
    ContLane clA; clA.key = keyA; clA.gestures = { gA };

    ContLane::G gB; gB.grip = "held"; gB.x0 = 0.0; gB.x1 = 1.0;
    gB.curve.pts = { { 0.0, 0.0f }, { 1.0, 1.0f } };
    ContLane clB; clB.key = keyB; clB.gestures = { gB };

    auto progOld = makeProgWith({ clA, clB });

    FakeSink sink;
    Player player(progOld);
    player.start(0.0);
    player.advanceTo(0.5, sink);   // both A and B touched, mid-gesture
    REQUIRE(sink.touches.size() == 2);
    REQUIRE(sink.releases.empty());

    auto progNew = makeProgWith({ clB });   // A dropped entirely (orphan); B still covers pos 0.5
    player.swap(progNew, sink);

    REQUIRE(sink.releases.size() == 1);
    REQUIRE(sink.releases[0] == keyA);

    const size_t touchesBefore = sink.touches.size();
    const size_t setsBefore = sink.sets.size();
    player.advanceTo(0.7, sink);   // B continues WITHOUT a fresh touch() -- re-seated, still held
    REQUIRE(sink.touches.size() == touchesBefore);
    REQUIRE(sink.sets.size() > setsBefore);

    player.advanceTo(1.0, sink);   // B's gesture ends
    REQUIRE(sink.releases.size() == 2);
    REQUIRE(sink.releases[1] == keyB);
}

TEST_CASE("Player::stop releases every held gesture", "[player]")
{
    ControlPath key; key.scope = ControlPath::Scope::Comp; key.control = "scalar"; key.scalar = "masterOpacity";
    ContLane::G g; g.grip = "held"; g.x0 = 0.0; g.x1 = 1.0;
    g.curve.pts = { { 0.0, 0.0f }, { 1.0, 1.0f } };
    ContLane cl; cl.key = key; cl.gestures = { g };

    auto prog = std::make_shared<Program>();
    prog->clock = DriveClock::Beat;
    prog->continuous.push_back(cl);

    FakeSink sink;
    Player player(prog);
    player.start(0.0);
    player.advanceTo(0.5, sink);
    REQUIRE(sink.touches.size() == 1);
    REQUIRE(sink.releases.empty());

    player.stop(sink);
    REQUIRE(sink.releases.size() == 1);
    REQUIRE_FALSE(player.running());

    const size_t setsBefore = sink.sets.size();
    player.advanceTo(0.7, sink);   // stopped -- no further dispatch at all
    REQUIRE(sink.sets.size() == setsBefore);
}
