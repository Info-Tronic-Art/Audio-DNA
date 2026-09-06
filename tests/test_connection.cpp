// s167-l2: the universal connection model + engine core, headless.
// .harmony/specs/s166-universal-connection-architecture.md section 5, Lane 2,
// tests (a)-(g). No renderer/UI: Composition/Clip/Layer + connect/* only.
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "connect/AutomationCurve.h"
#include "connect/ParamConnection.h"
#include "connect/LiveValue.h"
#include "connect/ScalarParams.h"
#include "connect/ConnectionShaper.h"
#include "connect/ConnectionEngine.h"
#include "connect/ConnSerialization.h"
#include "model/Composition.h"
#include "routing/MacroBank.h"
#include "signal/SignalRegistry.h"
#include "analysis/FeatureSnapshot.h"
#include <cmath>
#include <limits>

using Catch::Approx;

namespace
{
    FeatureSnapshot bareSnapshot()
    {
        FeatureSnapshot snap;
        snap.clear();
        return snap;
    }
}

// ============================================================================
// (a) Shaper parity -- v1 curve cases (tests/test_mapping_engine.cpp:463-525)
// reproduced bit-for-bit against ConnectionShaper::shapeValue at
// inverted==false, plus the RANGE (inMin/inMax/outMin/outMax) normalization
// tests (test_mapping_engine.cpp:149-207).
// ============================================================================

TEST_CASE("ConnectionShaper::shapeValue matches v1 MappingEngine's curve pipeline bit-for-bit", "[connection][shaper]")
{
    SECTION("Linear, default range -- identity")
    {
        ConnShape s;
        REQUIRE(ConnectionShaper::shapeValue(s, 0.3f) == Approx(0.3f).margin(0.001f));
    }
    SECTION("Exponential: 0.5^2 = 0.25")
    {
        ConnShape s; s.curve = 1;   // MappingCurve::Exponential
        REQUIRE(ConnectionShaper::shapeValue(s, 0.5f) == Approx(0.25f).margin(0.001f));
    }
    SECTION("Logarithmic: log(1+9x)/log(10) at x=0.5")
    {
        ConnShape s; s.curve = 2;   // MappingCurve::Logarithmic
        float expected = std::log(1.0f + 9.0f * 0.5f) / std::log(10.0f);
        REQUIRE(ConnectionShaper::shapeValue(s, 0.5f) == Approx(expected).margin(0.001f));
    }
    SECTION("Stepped: floor(0.6*4)/4 = 0.5")
    {
        ConnShape s; s.curve = 4;   // MappingCurve::Stepped
        REQUIRE(ConnectionShaper::shapeValue(s, 0.6f) == Approx(0.5f).margin(0.001f));
    }
    SECTION("inMin/inMax normalization: value at inMin -> outMin, at inMax -> outMax")
    {
        ConnShape s; s.inMin = 0.2f; s.inMax = 0.8f; s.outMin = 0.0f; s.outMax = 1.0f;
        REQUIRE(ConnectionShaper::shapeValue(s, 0.2f) == Approx(0.0f).margin(0.001f));
        REQUIRE(ConnectionShaper::shapeValue(s, 0.8f) == Approx(1.0f).margin(0.001f));
    }
    SECTION("Values outside inMin/inMax clamp")
    {
        ConnShape s; s.inMin = 0.2f; s.inMax = 0.8f;
        REQUIRE(ConnectionShaper::shapeValue(s, 0.0f) == Approx(0.0f).margin(0.001f));
        REQUIRE(ConnectionShaper::shapeValue(s, 1.0f) == Approx(1.0f).margin(0.001f));
    }
}

// ============================================================================
// (c) INVERT and sub-RANGE
// ============================================================================

TEST_CASE("ConnectionShaper::shapeValue applies INVERT then the RANGE sub-range", "[connection][shaper]")
{
    ConnShape s; s.outMin = 0.2f; s.outMax = 0.8f; s.inverted = true;
    // raw 0 -> curve 0 -> inverted 1 -> outMax; raw 1 -> curve 1 -> inverted 0 -> outMin
    REQUIRE(ConnectionShaper::shapeValue(s, 0.0f) == Approx(0.8f).margin(0.001f));
    REQUIRE(ConnectionShaper::shapeValue(s, 1.0f) == Approx(0.2f).margin(0.001f));
}

TEST_CASE("ConnectionShaper::shapeValue: RANGE sweeps only the chosen sub-range", "[connection][shaper]")
{
    ConnShape s; s.outMin = 0.4f; s.outMax = 0.6f;
    REQUIRE(ConnectionShaper::shapeValue(s, 0.0f) == Approx(0.4f).margin(0.001f));
    REQUIRE(ConnectionShaper::shapeValue(s, 0.5f) == Approx(0.5f).margin(0.001f));
    REQUIRE(ConnectionShaper::shapeValue(s, 1.0f) == Approx(0.6f).margin(0.001f));
}

// ============================================================================
// (b) LFO phase at bar 0/1/2 for cycle lengths 1/4/8/16 beats, including the
// 4*barCount term -- the shipped bug class this test exists to guard
// (S166-L1: an 8-beat LFO silently retracing a fraction of its waveform
// forever without the bar fold).
// ============================================================================

TEST_CASE("ConnectionShaper::beatsNow folds across bars (4*barCount term)", "[connection][shaper]")
{
    // S168: beatsNow now takes BOTH counters plus the switch that picks
    // between them (ConnShape::resetPhaseOnStructural, default false ==
    // totalBarCount). This test predates the switch and is about the raw
    // fold math, not which counter feeds it -- barCount and totalBarCount
    // are passed equal so the default (totalBarCount) reproduces the exact
    // pre-S168 numbers unchanged.
    REQUIRE(ConnectionShaper::beatsNow(0.5f, 2, 3, 3) == Approx(0.5f + 2.0f + 4.0f * 3.0f).margin(0.0001f));
    REQUIRE(ConnectionShaper::beatsNow(0.0f, 0, 0, 0) == Approx(0.0f).margin(0.0001f));
}

TEST_CASE("Lfo phase at bar 0/1/2 for cycle lengths 1, 4, 8, 16 beats", "[connection][lfo]")
{
    SignalRegistry sig;
    MacroBank bank;
    FeatureSnapshot snap = bareSnapshot();

    for (float cycleBeats : { 1.0f, 4.0f, 8.0f, 16.0f })
    {
        for (uint16_t bar : { static_cast<uint16_t>(0), static_cast<uint16_t>(1), static_cast<uint16_t>(2) })
        {
            snap.beatPhase = 0.5f;
            snap.beatInBar = 1;
            snap.barCount = bar;
            snap.totalBarCount = bar;   // no structural reset in this scenario -- the two agree

            ParamConnection conn;
            conn.source.kind = ConnSource::Kind::Lfo;
            conn.source.lfo.shape = ConnSource::Lfo::Shape::SawUp;   // raw == phase, easiest to check exactly
            conn.source.lfo.cycleBeats = cycleBeats;

            ConnectionEngine::Context ctx{ sig, bank, snap, 0.016f, 1.0 + bar, 250.0f, 120.0f };
            float bn = ConnectionShaper::beatsNow(snap.beatPhase, snap.beatInBar, snap.barCount,
                                                  snap.totalBarCount, conn.shape.resetPhaseOnStructural);
            float expectedPhase = bn / cycleBeats;
            expectedPhase -= std::floor(expectedPhase);

            float y = ConnectionEngine::evaluate(conn, 0.0f, ctx, nullptr);
            REQUIRE(y == Approx(expectedPhase).margin(0.001f));
        }
    }
}

TEST_CASE("An 8-beat LFO progresses across two full bars instead of retracing forever", "[connection][lfo]")
{
    // Without the 4*barCount fold, bn would wrap at 4 (one bar) every time,
    // so an 8-beat cycle would only ever see bn in [0,4) -- retracing the
    // first half of its waveform forever. With the fold, bar 1 (bn=4) reaches
    // the cycle's exact midpoint.
    SignalRegistry sig;
    MacroBank bank;
    FeatureSnapshot snap = bareSnapshot();
    snap.beatPhase = 0.0f;
    snap.beatInBar = 0;
    snap.barCount = 1;   // bn = 4 with the fold, 0 without it
    snap.totalBarCount = 1;   // default (resetPhaseOnStructural=false) reads this instead

    ParamConnection conn;
    conn.source.kind = ConnSource::Kind::Lfo;
    conn.source.lfo.shape = ConnSource::Lfo::Shape::SawUp;
    conn.source.lfo.cycleBeats = 8.0f;

    ConnectionEngine::Context ctx{ sig, bank, snap, 0.016f, 1.0, 250.0f, 120.0f };
    float y = ConnectionEngine::evaluate(conn, 0.0f, ctx, nullptr);
    REQUIRE(y == Approx(0.5f).margin(0.001f));
}

// ============================================================================
// review-b2 (s168), blocking issue: no test at the ConnectionShaper/
// ConnectionEngine level proved the two bar counters actually DIVERGE
// across a structural reset, and none exercised the legacy opt-in
// (ConnShape::resetPhaseOnStructural = true) at this level either.
// ConnectionEngine::evaluate (ConnectionEngine.cpp) is the call site hit
// for EVERY enabled Lfo/Envelope(Beats) connection at runtime -- higher
// blast radius than the oscillator-level guard alone (tests/
// test_oscillator_bar_fold.cpp's "S168: default oscillator phase is
// monotonic..." case, which only proves OscillatorSignal's own copy of the
// same switch). These two cases mirror that oscillator-level case's exact
// snapshot sequence (start / beforeReset / atReset / afterReset), first at
// the pure beatsNow() level, then through a real Lfo connection via
// evaluate().
// ============================================================================

TEST_CASE("ConnectionShaper::beatsNow diverges across a structural reset -- default (totalBarCount) keeps climbing, legacy (barCount) jumps back", "[connection][shaper][s168]")
{
    // Pre-reset, both switch settings must agree exactly (barCount ==
    // totalBarCount so far -- the switch has not diverged them yet).
    REQUIRE(ConnectionShaper::beatsNow(0.0f, 0, 0, 0, false)
            == Approx(ConnectionShaper::beatsNow(0.0f, 0, 0, 0, true)).margin(0.0001f));
    REQUIRE(ConnectionShaper::beatsNow(0.0f, 0, 5, 5, false)
            == Approx(ConnectionShaper::beatsNow(0.0f, 0, 5, 5, true)).margin(0.0001f));

    const float bBefore = ConnectionShaper::beatsNow(0.0f, 0, 5, 5, false);

    // A real structural-transition reset lands mid-cycle: barCount snaps to
    // 0, beatPhase/beatInBar continue undisturbed, totalBarCount is
    // untouched (the S168 guarantee itself).
    const float bAtResetDefault = ConnectionShaper::beatsNow(0.5f, 2, 0, 5, false);   // reads totalBarCount (5) -- untouched
    const float bAtResetLegacy = ConnectionShaper::beatsNow(0.5f, 2, 0, 5, true);     // reads barCount (0) -- snapped

    // Default: still climbing through the reset. Legacy: a real, visible
    // backward jump at the reset (matches the oscillator-level case's
    // l1 - l2 > 0.3f threshold).
    REQUIRE(bAtResetDefault > bBefore);
    REQUIRE(bBefore - bAtResetLegacy > 0.3f);

    // The two switch settings must therefore DISAGREE at the reset point --
    // this is what "diverge" means; a test that passed with these equal
    // would prove nothing about the switch at all.
    REQUIRE(bAtResetDefault - bAtResetLegacy > 0.3f);
}

TEST_CASE("ConnectionEngine::evaluate: default Lfo phase is monotonic across a structural reset; resetPhaseOnStructural=true still jumps", "[connection][lfo][s168]")
{
    // ConnectionEngine::evaluate is the call site actually hit by every
    // enabled Lfo/Envelope(Beats) connection at runtime (ConnectionEngine
    // .cpp: "ConnectionShaper::beatsNow(... c.shape.resetPhaseOnStructural)")
    // -- the coverage gap review-b2 named. cycleBeats=32 (8-bar cycle) keeps
    // bn well under one full cycle across every snapshot below, so a SawUp
    // connection's value tracks bn/cycleBeats with no wraparound to confuse
    // a genuine backward jump with the waveform's own 1->0 wrap (same
    // reasoning as the oscillator-level S168 case this mirrors).
    SignalRegistry sig;
    MacroBank bank;

    ParamConnection defaultConn;
    defaultConn.source.kind = ConnSource::Kind::Lfo;
    defaultConn.source.lfo.shape = ConnSource::Lfo::Shape::SawUp;
    defaultConn.source.lfo.cycleBeats = 32.0f;
    REQUIRE_FALSE(defaultConn.shape.resetPhaseOnStructural);   // default is false

    ParamConnection legacyConn;
    legacyConn.source.kind = ConnSource::Kind::Lfo;
    legacyConn.source.lfo.shape = ConnSource::Lfo::Shape::SawUp;
    legacyConn.source.lfo.cycleBeats = 32.0f;
    legacyConn.shape.resetPhaseOnStructural = true;

    FeatureSnapshot start = bareSnapshot();
    start.beatPhase = 0.0f; start.beatInBar = 0; start.barCount = 0; start.totalBarCount = 0;

    FeatureSnapshot beforeReset = bareSnapshot();
    beforeReset.beatPhase = 0.0f; beforeReset.beatInBar = 0; beforeReset.barCount = 5; beforeReset.totalBarCount = 5;

    // A real structural-transition reset (BPMTracker::updatePhrase's
    // drop-entry branch): barCount snaps to 0; totalBarCount is untouched
    // (the S168 guarantee this test exists to prove at THIS level).
    FeatureSnapshot atReset = bareSnapshot();
    atReset.beatPhase = 0.5f; atReset.beatInBar = 2; atReset.barCount = 0; atReset.totalBarCount = 5;

    FeatureSnapshot afterReset = bareSnapshot();
    afterReset.beatPhase = 0.0f; afterReset.beatInBar = 0; afterReset.barCount = 1; afterReset.totalBarCount = 6;

    auto evalAt = [&](ParamConnection& conn, const FeatureSnapshot& snap, double now)
    {
        ConnectionEngine::Context ctx{ sig, bank, snap, 0.016f, now, 250.0f, 120.0f };
        return ConnectionEngine::evaluate(conn, 0.0f, ctx, nullptr);
    };

    const float d0 = evalAt(defaultConn, start, 1.0);
    const float d1 = evalAt(defaultConn, beforeReset, 2.0);
    const float d2 = evalAt(defaultConn, atReset, 3.0);
    const float d3 = evalAt(defaultConn, afterReset, 4.0);
    INFO("default sequence: " << d0 << ", " << d1 << ", " << d2 << ", " << d3);
    // Monotonically non-decreasing across the whole sequence, INCLUDING the
    // structural reset -- the defining assertion. A reverted evaluate()
    // (always feeding beatsNow() the resettable barCount, i.e. as if
    // resetPhaseOnStructural were hardcoded true) would drop at d2 exactly
    // the way the legacy connection does below -- confirmed by mutation
    // test (see the report).
    REQUIRE(d1 >= d0 - 0.0001f);
    REQUIRE(d2 >= d1 - 0.0001f);
    REQUIRE(d3 >= d2 - 0.0001f);

    const float l0 = evalAt(legacyConn, start, 1.0);
    const float l1 = evalAt(legacyConn, beforeReset, 2.0);
    const float l2 = evalAt(legacyConn, atReset, 3.0);
    const float l3 = evalAt(legacyConn, afterReset, 4.0);
    INFO("legacy sequence: " << l0 << ", " << l1 << ", " << l2 << ", " << l3);
    // The legacy opt-in (resetPhaseOnStructural=true) must still be
    // reachable THROUGH evaluate(), not just through beatsNow() directly: a
    // real, visible backward jump at the reset, then climbing again.
    REQUIRE(l1 - l2 > 0.3f);
    REQUIRE(l3 > l2);
}

// ============================================================================
// Each Playback transform.
// ============================================================================

TEST_CASE("ConnectionShaper::playbackXform for each direction", "[connection][shaper]")
{
    using PB = ConnShape::Playback;
    REQUIRE(ConnectionShaper::playbackXform(0.25f, PB::Forward) == Approx(0.25f).margin(0.001f));
    REQUIRE(ConnectionShaper::playbackXform(0.25f, PB::Backward) == Approx(0.75f).margin(0.001f));
    // PingPong: one period spans TWO raw cycles -- a forward leg over the
    // first, a backward leg over the second (s167-l2 report FINDINGS: an
    // inferred resolution of the spec's underspecified "p_cycles" term).
    REQUIRE(ConnectionShaper::playbackXform(0.25f, PB::PingPong) == Approx(0.25f).margin(0.001f));  // early in the forward leg
    REQUIRE(ConnectionShaper::playbackXform(1.25f, PB::PingPong) == Approx(0.75f).margin(0.001f));  // into the backward leg
    REQUIRE(ConnectionShaper::playbackXform(2.0f, PB::PingPong) == Approx(0.0f).margin(0.001f));    // back at the start
}

// ============================================================================
// loop=false holds its end value.
// ============================================================================

TEST_CASE("loop=false holds the Lfo's end value instead of wrapping", "[connection][lfo][once]")
{
    SignalRegistry sig;
    MacroBank bank;
    FeatureSnapshot snap = bareSnapshot();

    ParamConnection conn;
    conn.source.kind = ConnSource::Kind::Lfo;
    conn.source.lfo.shape = ConnSource::Lfo::Shape::SawUp;
    conn.source.lfo.cycleBeats = 1.0f;
    conn.shape.loop = false;

    snap.beatPhase = 0.0f; snap.beatInBar = 0; snap.barCount = 0;
    ConnectionEngine::Context ctx0{ sig, bank, snap, 0.016f, 1.0, 250.0f, 120.0f };
    REQUIRE(ConnectionEngine::evaluate(conn, 0.0f, ctx0, nullptr) == Approx(0.0f).margin(0.01f));

    snap.beatPhase = 0.5f;
    ConnectionEngine::Context ctx1{ sig, bank, snap, 0.016f, 1.5, 250.0f, 120.0f };
    REQUIRE(ConnectionEngine::evaluate(conn, 0.0f, ctx1, nullptr) == Approx(0.5f).margin(0.01f));

    // Well past one full cycle -- must HOLD near 1.0, not wrap back to 0.
    snap.beatPhase = 0.0f; snap.barCount = 5; snap.totalBarCount = 5;
    ConnectionEngine::Context ctx2{ sig, bank, snap, 0.016f, 5.0, 250.0f, 120.0f };
    REQUIRE(ConnectionEngine::evaluate(conn, 0.0f, ctx2, nullptr) == Approx(1.0f).margin(0.01f));

    snap.barCount = 20; snap.totalBarCount = 20;
    ConnectionEngine::Context ctx3{ sig, bank, snap, 0.016f, 20.0, 250.0f, 120.0f };
    REQUIRE(ConnectionEngine::evaluate(conn, 0.0f, ctx3, nullptr) == Approx(1.0f).margin(0.01f));
}

// ============================================================================
// (d) GRIP: held grip suspends publishing (twin reads NAN), decaying grip
// expires at gripHoldMs, held displaces decaying and not vice-versa, release
// glides over handBackGlideMs and snaps at 0.
// ============================================================================

TEST_CASE("Grip: held suspends publishing", "[connection][grip]")
{
    SignalRegistry sig;
    MacroBank bank;
    FeatureSnapshot snap = bareSnapshot();

    ParamConnection conn;
    conn.source.kind = ConnSource::Kind::Lfo;
    conn.source.lfo.cycleBeats = 4.0f;
    conn.gripHeld();

    ConnectionEngine::Context ctx{ sig, bank, snap, 0.016f, 1.0, 250.0f, 0.0f };
    float y = ConnectionEngine::evaluate(conn, 0.3f, ctx, nullptr);
    REQUIRE(std::isnan(y));
}

TEST_CASE("Grip: release snaps immediately when handBackGlideMs == 0", "[connection][grip]")
{
    SignalRegistry sig;
    MacroBank bank;
    FeatureSnapshot snap = bareSnapshot();
    snap.beatPhase = 0.0f; snap.beatInBar = 0; snap.barCount = 0;   // signal sits at raw == 0

    ParamConnection conn;
    conn.source.kind = ConnSource::Kind::Lfo;
    conn.source.lfo.shape = ConnSource::Lfo::Shape::SawUp;
    conn.source.lfo.cycleBeats = 4.0f;
    conn.gripHeld();

    ConnectionEngine::Context ctxGrip{ sig, bank, snap, 0.016f, 1.0, 250.0f, 0.0f };
    REQUIRE(std::isnan(ConnectionEngine::evaluate(conn, 0.3f, ctxGrip, nullptr)));

    conn.release(1.0);
    ConnectionEngine::Context ctxAfter{ sig, bank, snap, 0.016f, 1.016, 250.0f, 0.0f };
    float y = ConnectionEngine::evaluate(conn, 0.3f, ctxAfter, nullptr);
    REQUIRE(!std::isnan(y));
    REQUIRE(y == Approx(0.0f).margin(0.01f));   // 0 glide -> snaps straight to the signal
}

TEST_CASE("Grip: release glides over handBackGlideMs, starting from the gripped value", "[connection][grip]")
{
    SignalRegistry sig;
    MacroBank bank;
    FeatureSnapshot snap = bareSnapshot();
    snap.beatPhase = 0.0f; snap.beatInBar = 0; snap.barCount = 0;   // signal sits at raw == 0

    ParamConnection conn;
    conn.source.kind = ConnSource::Kind::Lfo;
    conn.source.lfo.shape = ConnSource::Lfo::Shape::SawUp;
    conn.source.lfo.cycleBeats = 4.0f;
    conn.gripHeld();

    ConnectionEngine::Context ctxGrip{ sig, bank, snap, 0.016f, 1.0, 250.0f, 100.0f };
    ConnectionEngine::evaluate(conn, 0.3f, ctxGrip, nullptr);   // tracks handBackFrom = 0.3
    conn.release(1.0);

    ConnectionEngine::Context ctxStart{ sig, bank, snap, 0.016f, 1.0, 250.0f, 100.0f };
    float yStart = ConnectionEngine::evaluate(conn, 0.3f, ctxStart, nullptr);   // glide STARTS here
    REQUIRE(yStart == Approx(0.3f).margin(0.01f));

    ConnectionEngine::Context ctxHalf{ sig, bank, snap, 0.016f, 1.05, 250.0f, 100.0f };
    float yHalf = ConnectionEngine::evaluate(conn, 0.3f, ctxHalf, nullptr);
    REQUIRE(yHalf == Approx(0.15f).margin(0.02f));   // 50ms of a 100ms glide from 0.3 to 0.0

    ConnectionEngine::Context ctxDone{ sig, bank, snap, 0.016f, 1.2, 250.0f, 100.0f };
    float yDone = ConnectionEngine::evaluate(conn, 0.3f, ctxDone, nullptr);
    REQUIRE(yDone == Approx(0.0f).margin(0.01f));
}

TEST_CASE("Grip: decaying expires after gripHoldMs; Held does not", "[connection][grip]")
{
    SignalRegistry sig;
    MacroBank bank;
    FeatureSnapshot snap = bareSnapshot();

    ParamConnection conn;
    conn.source.kind = ConnSource::Kind::Lfo;
    conn.source.lfo.cycleBeats = 4.0f;
    conn.gripTouch(1.0);   // Decaying

    ConnectionEngine::Context ctxStill{ sig, bank, snap, 0.016f, 1.1, 250.0f, 0.0f };   // 100ms later, < 250ms
    REQUIRE(std::isnan(ConnectionEngine::evaluate(conn, 0.3f, ctxStill, nullptr)));

    ConnectionEngine::Context ctxExpired{ sig, bank, snap, 0.016f, 1.3, 250.0f, 0.0f };  // 300ms later, > 250ms
    REQUIRE(!std::isnan(ConnectionEngine::evaluate(conn, 0.3f, ctxExpired, nullptr)));

    ParamConnection heldConn;
    heldConn.source.kind = ConnSource::Kind::Lfo;
    heldConn.gripHeld();
    ConnectionEngine::Context ctxHeldWait{ sig, bank, snap, 0.016f, 1000.0, 250.0f, 0.0f };  // way past any hold time
    REQUIRE(std::isnan(ConnectionEngine::evaluate(heldConn, 0.3f, ctxHeldWait, nullptr)));   // Held never expires on its own
}

TEST_CASE("Grip: Held displaces Decaying; Decaying does not displace Held", "[connection][grip]")
{
    ParamConnection conn;
    conn.gripTouch(1.0);
    REQUIRE(conn.grip.kind == ParamConnection::Grip::Kind::Decaying);

    conn.gripHeld();
    REQUIRE(conn.grip.kind == ParamConnection::Grip::Kind::Held);

    conn.gripTouch(1.01);   // must NOT displace the Held grip
    REQUIRE(conn.grip.kind == ParamConnection::Grip::Kind::Held);
}

// ============================================================================
// (f) JSON: old files (no connection data) still load; unknown kind -> None
// and reported.
// ============================================================================

TEST_CASE("ConnSerialization::fromVar: an absent/empty object loads as disconnected", "[connection][serialization]")
{
    ParamConnection conn;
    conn.source.kind = ConnSource::Kind::Lfo;   // pre-existing value fromVar must wipe
    auto* obj = new juce::DynamicObject();
    ConnSerialization::fromVar(conn, juce::var(obj));
    REQUIRE_FALSE(conn.isConnected());
}

TEST_CASE("ConnSerialization::fromVar: an unrecognized source kind loads as None and is reported", "[connection][serialization]")
{
    ParamConnection conn;
    auto* srcObj = new juce::DynamicObject();
    srcObj->setProperty("kind", "timeline");   // not a kind this session implements
    auto* obj = new juce::DynamicObject();
    obj->setProperty("src", juce::var(srcObj));

    int unknownCount = 0;
    ConnSerialization::fromVar(conn, juce::var(obj), &unknownCount);
    REQUIRE_FALSE(conn.isConnected());
    REQUIRE(unknownCount == 1);
}

TEST_CASE("ConnSerialization: round-trips a real connection through toVar/fromVar", "[connection][serialization]")
{
    ParamConnection conn;
    conn.source.kind = ConnSource::Kind::Lfo;
    conn.source.lfo.shape = ConnSource::Lfo::Shape::Triangle;
    conn.source.lfo.cycleBeats = 2.0f;
    conn.source.lfo.phaseOffset = 0.25f;
    conn.shape.outMin = 0.2f; conn.shape.outMax = 0.9f; conn.shape.inverted = true;
    conn.shape.playback = ConnShape::Playback::PingPong;
    conn.shape.loop = false;
    conn.shape.curve = 3;   // SCurve
    conn.enabled = true;

    auto v = ConnSerialization::toVar(conn);
    ParamConnection loaded;
    int unknownCount = 0;
    ConnSerialization::fromVar(loaded, v, &unknownCount);

    REQUIRE(unknownCount == 0);
    REQUIRE(loaded.source.kind == ConnSource::Kind::Lfo);
    REQUIRE(loaded.source.lfo.shape == ConnSource::Lfo::Shape::Triangle);
    REQUIRE(loaded.source.lfo.cycleBeats == Approx(2.0f));
    REQUIRE(loaded.source.lfo.phaseOffset == Approx(0.25f));
    REQUIRE(loaded.shape.outMin == Approx(0.2f));
    REQUIRE(loaded.shape.outMax == Approx(0.9f));
    REQUIRE(loaded.shape.inverted == true);
    REQUIRE(loaded.shape.playback == ConnShape::Playback::PingPong);
    REQUIRE(loaded.shape.loop == false);
    REQUIRE(loaded.shape.curve == 3);
    REQUIRE(loaded.enabled == true);
}

// ============================================================================
// AutomationCurve -- the shared curve type behind Envelope (s167-l2, added
// after a team-lead correction: this is the SAME type a future recorded
// performance take will use, so its shape is fixed now rather than
// migrated later).
// ============================================================================

TEST_CASE("AutomationCurve::eval interpolates Linear/Hold/Smooth segments and clamps outside range", "[connection][automation]")
{
    AutomationCurve curve;
    curve.pts = {
        { 0.0, 0.0f, Breakpoint::Interp::Linear },
        { 0.5, 1.0f, Breakpoint::Interp::Hold },
        { 1.0, 0.0f, Breakpoint::Interp::Linear },
    };

    REQUIRE(curve.eval(0.0) == Approx(0.0f).margin(0.001f));
    REQUIRE(curve.eval(0.25) == Approx(0.5f).margin(0.001f));   // linear ramp, first segment
    REQUIRE(curve.eval(0.5) == Approx(1.0f).margin(0.001f));
    REQUIRE(curve.eval(0.75) == Approx(1.0f).margin(0.001f));   // held through the Hold segment
    REQUIRE(curve.eval(1.0) == Approx(0.0f).margin(0.001f));
    REQUIRE(curve.eval(-1.0) == Approx(0.0f).margin(0.001f));   // clamps below xMin
    REQUIRE(curve.eval(2.0) == Approx(0.0f).margin(0.001f));    // clamps above xMax
}

TEST_CASE("AutomationCurve::eval Smooth eases via smoothstep, not linear", "[connection][automation]")
{
    AutomationCurve curve;
    curve.pts = {
        { 0.0, 0.0f, Breakpoint::Interp::Smooth },
        { 1.0, 1.0f, Breakpoint::Interp::Linear },
    };
    // smoothstep(0.25) = 0.25^2 * (3 - 2*0.25) = 0.15625, well below the
    // linear 0.25 a plain lerp would give -- proves Smooth is really eased.
    REQUIRE(curve.eval(0.25) == Approx(0.15625f).margin(0.001f));
    REQUIRE(curve.eval(0.5) == Approx(0.5f).margin(0.001f));   // smoothstep is symmetric at the midpoint
}

TEST_CASE("ConnSerialization: Envelope round-trips AutomationCurve breakpoints with interp", "[connection][serialization][automation]")
{
    ParamConnection conn;
    conn.source.kind = ConnSource::Kind::Envelope;
    conn.source.env.cycleBeats = 8.0f;
    conn.source.env.clock = ConnSource::Envelope::Clock::ClipPosition;
    conn.source.env.curve.pts = {
        { 0.0, 0.0f, Breakpoint::Interp::Linear },
        { 0.5, 1.0f, Breakpoint::Interp::Hold },
        { 1.0, 0.2f, Breakpoint::Interp::Smooth },
    };

    auto v = ConnSerialization::toVar(conn);
    ParamConnection loaded;
    ConnSerialization::fromVar(loaded, v);

    REQUIRE(loaded.source.kind == ConnSource::Kind::Envelope);
    REQUIRE(loaded.source.env.clock == ConnSource::Envelope::Clock::ClipPosition);
    REQUIRE(loaded.source.env.cycleBeats == Approx(8.0f));
    REQUIRE(loaded.source.env.curve.pts.size() == 3);
    REQUIRE(loaded.source.env.curve.pts[0].interp == Breakpoint::Interp::Linear);
    REQUIRE(loaded.source.env.curve.pts[1].interp == Breakpoint::Interp::Hold);
    REQUIRE(loaded.source.env.curve.pts[1].x == Approx(0.5));
    REQUIRE(loaded.source.env.curve.pts[1].y == Approx(1.0f));
    REQUIRE(loaded.source.env.curve.pts[2].interp == Breakpoint::Interp::Smooth);
}

TEST_CASE("ConnSerialization: old flat [x,y] pair points load as Interp::Linear", "[connection][serialization][automation]")
{
    // Simulates a file written before the Breakpoint/interp change -- a
    // "points" array of bare [x,y] pairs with no interp field at all.
    auto* srcObj = new juce::DynamicObject();
    srcObj->setProperty("kind", "envelope");
    juce::Array<juce::var> pts;
    { juce::Array<juce::var> p; p.add(0.0); p.add(0.0); pts.add(p); }
    { juce::Array<juce::var> p; p.add(1.0); p.add(1.0); pts.add(p); }
    srcObj->setProperty("points", pts);
    auto* obj = new juce::DynamicObject();
    obj->setProperty("src", juce::var(srcObj));

    ParamConnection conn;
    ConnSerialization::fromVar(conn, juce::var(obj));

    REQUIRE(conn.source.kind == ConnSource::Kind::Envelope);
    REQUIRE(conn.source.env.curve.pts.size() == 2);
    REQUIRE(conn.source.env.curve.pts[0].interp == Breakpoint::Interp::Linear);
    REQUIRE(conn.source.env.curve.pts[1].interp == Breakpoint::Interp::Linear);
    REQUIRE(conn.source.env.curve.pts[1].x == Approx(1.0));
    REQUIRE(conn.source.env.curve.pts[1].y == Approx(1.0f));
}

TEST_CASE("ConnectionEngine::evaluate drives an Envelope connection through AutomationCurve", "[connection][engine][automation]")
{
    SignalRegistry sig;
    MacroBank bank;
    FeatureSnapshot snap = bareSnapshot();
    snap.beatPhase = 0.0f; snap.beatInBar = 2; snap.barCount = 0;   // bn = 2

    ParamConnection conn;
    conn.source.kind = ConnSource::Kind::Envelope;
    conn.source.env.clock = ConnSource::Envelope::Clock::Beats;
    conn.source.env.cycleBeats = 4.0f;   // pos = bn/4 = 0.5
    conn.source.env.curve.pts = {
        { 0.0, 0.0f, Breakpoint::Interp::Linear },
        { 0.5, 1.0f, Breakpoint::Interp::Linear },
        { 1.0, 0.0f, Breakpoint::Interp::Linear },
    };

    ConnectionEngine::Context ctx{ sig, bank, snap, 0.016f, 1.0, 250.0f, 120.0f };
    float y = ConnectionEngine::evaluate(conn, 0.0f, ctx, nullptr);
    REQUIRE(y == Approx(1.0f).margin(0.001f));
}

TEST_CASE("Clip::fromVar loads a clip with no connections at all (old-file shape)", "[connection][serialization]")
{
    Clip original;
    original.name = "legacy";
    original.mediaType = Clip::MediaType::Video;
    auto v = original.toVar();
    REQUIRE_FALSE(v.getDynamicObject()->hasProperty("conns"));   // nothing connected -> sparse, absent

    Clip loaded;
    loaded.fromVar(v);
    REQUIRE_FALSE(loaded.scalarConns[static_cast<size_t>(ClipScalar::Opacity)].isConnected());
}

TEST_CASE("Clip::toVar/fromVar round-trips a scalar connection and an effect param connection", "[connection][serialization]")
{
    Clip original;
    original.name = "connected";
    original.effects.push_back(Clip::EffectSlot{});
    original.effects[0].effectName = "ripple";
    original.effects[0].paramValues = { 0.5f, 0.3f };
    original.effects[0].resizeParams(2);
    original.effects[0].paramConns[1].source.kind = ConnSource::Kind::Lfo;
    original.effects[0].paramConns[1].source.lfo.cycleBeats = 1.0f;
    original.effects[0].dryWetConn.source.kind = ConnSource::Kind::Signal;
    original.effects[0].dryWetConn.source.signalName = "Bass";
    original.scalarConns[static_cast<size_t>(ClipScalar::PosX)].source.kind = ConnSource::Kind::Macro;
    original.scalarConns[static_cast<size_t>(ClipScalar::PosX)].source.macroIndex = 3;

    auto v = original.toVar();
    Clip loaded;
    loaded.fromVar(v);

    REQUIRE(loaded.effects.size() == 1);
    REQUIRE(loaded.effects[0].paramConns.size() == 2);
    REQUIRE_FALSE(loaded.effects[0].paramConns[0].isConnected());
    REQUIRE(loaded.effects[0].paramConns[1].source.kind == ConnSource::Kind::Lfo);
    REQUIRE(loaded.effects[0].paramConns[1].source.lfo.cycleBeats == Approx(1.0f));
    REQUIRE(loaded.effects[0].dryWetConn.source.kind == ConnSource::Kind::Signal);
    REQUIRE(loaded.effects[0].dryWetConn.source.signalName == "Bass");
    REQUIRE(loaded.scalarConns[static_cast<size_t>(ClipScalar::PosX)].source.kind == ConnSource::Kind::Macro);
    REQUIRE(loaded.scalarConns[static_cast<size_t>(ClipScalar::PosX)].source.macroIndex == 3);
}

// ============================================================================
// (g) Engine tick publishes into the live twins, macros evaluated first, and
// a disconnect publishes NAN.
// ============================================================================

TEST_CASE("ConnectionEngine::tick publishes into twins with macros ticked first; disconnect leaves the twin at NaN",
         "[connection][engine]")
{
    SignalRegistry sig;
    MacroBank bank(MacroBank::Scope::Global);
    FeatureSnapshot snap = bareSnapshot();
    snap.beatPhase = 0.5f; snap.beatInBar = 0; snap.barCount = 0;   // bn = 0.5

    Composition comp;
    comp.initDefault();

    // Macro 0 is driven by an Lfo; the Composition's Opacity scalar is
    // driven by macro 0. If the engine did NOT tick macros before scalars,
    // the scalar would read a stale (default 0.5) macro currentValue.
    bank.getMacro(0).conn.source.kind = ConnSource::Kind::Lfo;
    bank.getMacro(0).conn.source.lfo.shape = ConnSource::Lfo::Shape::SawUp;
    bank.getMacro(0).conn.source.lfo.cycleBeats = 4.0f;   // phase = bn/4 = 0.125

    auto& opacityConn = comp.scalarConns[static_cast<size_t>(CompScalar::Opacity)];
    opacityConn.source.kind = ConnSource::Kind::Macro;
    opacityConn.source.macroIndex = 0;

    ConnectionEngine engine;
    ConnectionEngine::Context ctx{ sig, bank, snap, 0.016f, 1.0, 250.0f, 120.0f };
    engine.tick(comp, ctx);

    REQUIRE(bank.getMacro(0).currentValue == Approx(0.125f).margin(0.001f));
    float twin = comp.scalarLive[static_cast<size_t>(CompScalar::Opacity)].v.load(std::memory_order_relaxed);
    REQUIRE(twin == Approx(0.125f).margin(0.001f));   // CompScalar::Opacity's toModel is identity

    // Disconnect is the model mutator's job, not the engine's (s166 spec
    // section 4.3) -- simulate it directly, then confirm the NEXT tick
    // leaves the twin exactly as the mutator set it, because the engine
    // skips a None/disabled connection outright.
    opacityConn.source.kind = ConnSource::Kind::None;
    comp.scalarLive[static_cast<size_t>(CompScalar::Opacity)].v.store(
        std::numeric_limits<float>::quiet_NaN(), std::memory_order_relaxed);
    engine.tick(comp, ctx);
    REQUIRE(std::isnan(comp.scalarLive[static_cast<size_t>(CompScalar::Opacity)].v.load(std::memory_order_relaxed)));
}

TEST_CASE("ConnectionEngine::tick publishes an effect param connection into paramLive", "[connection][engine]")
{
    SignalRegistry sig;
    MacroBank bank;
    FeatureSnapshot snap = bareSnapshot();
    snap.beatPhase = 0.0f; snap.beatInBar = 0; snap.barCount = 0;

    Composition comp;
    comp.initDefault();
    Clip::EffectSlot fx;
    fx.effectName = "ripple";
    fx.paramValues = { 0.5f };
    fx.resizeParams(1);
    fx.paramConns[0].source.kind = ConnSource::Kind::Lfo;
    fx.paramConns[0].source.lfo.shape = ConnSource::Lfo::Shape::SawUp;
    fx.paramConns[0].source.lfo.cycleBeats = 1.0f;
    comp.globalEffects.push_back(fx);

    ConnectionEngine engine;
    ConnectionEngine::Context ctx{ sig, bank, snap, 0.016f, 1.0, 250.0f, 120.0f };
    engine.tick(comp, ctx);

    REQUIRE(comp.globalEffects[0].effParam(0) == Approx(0.0f).margin(0.01f));   // SawUp(phase 0) == 0
}
