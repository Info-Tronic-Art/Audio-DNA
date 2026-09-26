// test_routine_engine -- s-rta-0926 routines slice 1, lane 1b (plan-routines-s1-final.md
// sections 4.1-4.3 and 5.3, test table 6.1 cases 7-12 and 15): RoutineEngine's scheduler
// (pending -> next bar -> restore -> lanes on the routine's own beat grid), loop / once / stop /
// restart, the global Quantize override and "no beat yet" start, D9 stacking (the gesture that
// BEGAN later wins for the rest of the earlier one), barCount parity for 2 Bar / 4 Bar, the
// post-loop compaction of finished routines, Binding::Action::TriggerRoutine's round trip, and
// the save funnel's whole-words refusal text for an unmetered stretch (carried concern (b)).
//
// The engine is driven with synthetic FeatureSnapshots whose beat advances in exact binary
// steps (1/16 beat), so RecorderClock's integrated beat equals the driver's beat bit for bit and
// every "fires exactly when pos >= x" assertion is exact.
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "model/Composition.h"
#include "model/Routine.h"
#include "recording/RoutineEngine.h"
#include "recording/RoutineSlice.h"
#include "recording/Take.h"
#include "effects/EffectLibrary.h"
#include "binding/Binding.h"
#include "binding/BindingManager.h"
#include "analysis/FeatureSnapshot.h"
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

using Catch::Approx;

namespace
{
    ControlPath layerKey(int layer, const std::string& control, const std::string& scalar = "")
    {
        ControlPath k;
        k.scope = ControlPath::Scope::Layer;
        k.deckRelative = true;
        k.deck = 0;
        k.deckName = "Deck 1";
        k.layer = layer;
        k.layerName = "Layer " + std::to_string(layer + 1);
        k.control = control;
        k.scalar = scalar;
        return k;
    }

    ControlPath opacityKey(int layer) { return layerKey(layer, "scalar", "opacity"); }

    DiscretePoint point(uint64_t seq, double beat, int v)
    {
        DiscretePoint p;
        p.s = { seq, 0.0, 0 };
        p.beat = beat;
        p.bpm = 120.0f;
        p.v = v;
        return p;
    }

    // A two-breakpoint gesture (stampless -- a routine's gestures never carry stamps).
    Gesture gesture(double x0, float y0, double x1, float y1)
    {
        Gesture g;
        g.grip = "held";
        g.curve.pts = { { x0, y0 }, { x1, y1 } };
        return g;
    }

    Lane discreteLane(const ControlPath& key, std::vector<DiscretePoint> pts)
    {
        Lane lane;
        lane.key = key;
        lane.kind = Lane::Kind::Discrete;
        lane.points = std::move(pts);
        return lane;
    }

    Lane continuousLane(const ControlPath& key, std::vector<Gesture> gs)
    {
        Lane lane;
        lane.key = key;
        lane.kind = Lane::Kind::Continuous;
        lane.gestures = std::move(gs);
        return lane;
    }

    Routine makeRoutine(const std::string& uuid, double lengthBeats, Clip::BeatSnapMode quantize, bool loop)
    {
        Routine r;
        r.uuid = uuid;
        r.name = "Routine " + uuid;
        r.lengthBeats = lengthBeats;
        r.quantize = quantize;
        r.loop = loop;
        return r;
    }

    void addToBank(Composition& comp, const Routine& r, int slot)
    {
        comp.routines.push_back(r);
        REQUIRE(comp.assignRoutineSlot(slot, r.uuid));
    }

    // One event the engine dispatched, in order.
    struct Ev
    {
        enum Type { Fire, Touch, Set, Release } type;
        ControlPath key;
        float v = 0.0f;        // Set: the value; Fire: p.v
        Origin origin = Origin::Human;   // Fire only
    };

    struct FakeDispatch
    {
        std::vector<Ev> log;
        std::vector<std::string> notices;
        bool refuseTouch = false;

        void wire(RoutineEngine& eng)
        {
            eng.dispatch.fire = [this](const Fired& f) {
                log.push_back({ Ev::Fire, f.key, static_cast<float>(f.p.v), f.p.origin });
                return true;
            };
            eng.dispatch.touch = [this](const ControlPath& k, const std::string&) {
                log.push_back({ Ev::Touch, k });
                return !refuseTouch;
            };
            eng.dispatch.set = [this](const ControlPath& k, float v) {
                log.push_back({ Ev::Set, k, v });
                return true;
            };
            eng.dispatch.release = [this](const ControlPath& k) { log.push_back({ Ev::Release, k }); };
            eng.dispatch.notify = [this](const std::string& s) { notices.push_back(s); };
        }

        int count(Ev::Type t, const ControlPath& key) const
        {
            return static_cast<int>(std::count_if(log.begin(), log.end(),
                [&](const Ev& e) { return e.type == t && e.key == key; }));
        }
        int firedLanePoints(const ControlPath& key, int v) const   // recorded (non-restore) fires
        {
            return static_cast<int>(std::count_if(log.begin(), log.end(), [&](const Ev& e) {
                return e.type == Ev::Fire && e.key == key && e.origin != Origin::Preamble
                       && static_cast<int>(e.v) == v; }));
        }
        int firedRestores() const
        {
            return static_cast<int>(std::count_if(log.begin(), log.end(),
                [](const Ev& e) { return e.type == Ev::Fire && e.origin == Origin::Preamble; }));
        }
    };

    // Drives the engine with a steady 120 BPM beat. `beat` advances in exact 1/16 steps; the bar
    // counters follow it: totalBarCount = totalBase + floor(beat / 4), barCount = barBase +
    // floor(beat / 4) (the two bases differ where a test needs their parities to differ).
    struct Rig
    {
        static constexpr double kStep = 1.0 / 16.0;
        RoutineEngine eng;
        FakeDispatch fd;
        Composition comp;
        double beat = 0.0;
        uint32_t totalBase = 10;
        int barBase = 0;
        RoutineSnap forced = RoutineSnap::Off;
        bool beatAvailable = true;

        Rig()
        {
            comp.initDefault();
            fd.wire(eng);
        }

        FeatureSnapshot snap() const
        {
            FeatureSnapshot s;
            s.clear();
            s.bpm = 120.0f;
            s.trackerState = 2;
            s.beatPhase = static_cast<float>(beat - std::floor(beat));
            const auto bars = static_cast<int>(std::floor(beat / 4.0));
            s.totalBarCount = totalBase + static_cast<uint32_t>(bars);
            s.barCount = static_cast<uint16_t>(barBase + bars);
            return s;
        }

        void tick() { eng.tick(snap(), beat * 0.5, comp, forced, beatAvailable); }

        // Ticks up to and INCLUDING `target` (exact: target must be a multiple of kStep).
        void runTo(double target, double step = kStep)
        {
            while (beat + step <= target + 1e-12)
            {
                beat += step;
                tick();
            }
        }

        std::string fire(int slot) { return eng.fire(comp, slot, forced, beatAvailable); }
        RoutineEngine::Status::Slot slot(int i) const { return eng.status().slots[i]; }
    };
}

// === 7: pending until the next bar, then restore (discrete before continuous), then the lanes ===

TEST_CASE("RoutineEngine: waits for the next bar, restores, then plays its lanes on its own beat grid", "[routine][engine]")
{
    Rig rig;
    const ControlPath clipKey = layerKey(0, "activeClip");
    const ControlPath op0 = opacityKey(0);
    const ControlPath op1 = opacityKey(1);

    Routine r = makeRoutine("a", 8.0, Clip::BeatSnapMode::Bar, false);
    Routine::PreambleEntry restoreClip; restoreClip.key = clipKey; restoreClip.v = 2;
    Routine::PreambleEntry restoreOp; restoreOp.key = op1; restoreOp.continuous = true; restoreOp.norm = 0.3f;
    r.preamble = { restoreClip, restoreOp };
    r.lanes[clipKey] = discreteLane(clipKey, { point(1, 1.0, 1) });
    r.lanes[op0] = continuousLane(op0, { gesture(2.0, 0.2f, 3.0, 0.8f) });
    addToBank(rig.comp, r, 0);

    rig.tick();                  // beat 0, totalBarCount 10
    rig.runTo(1.0);
    CHECK(rig.slot(0).state == "idle");
    CHECK(rig.fire(0).empty());
    CHECK(rig.slot(0).state == "pending");

    rig.runTo(3.9375);           // still bar 10
    CHECK(rig.slot(0).state == "pending");
    CHECK(rig.fd.log.empty());

    rig.runTo(4.0);              // totalBarCount 10 -> 11: the start boundary
    {
        const auto s = rig.slot(0);
        CHECK(s.state == "running");
        CHECK(s.startedTotalBar == 11);
        CHECK(s.preambleCount == 2);
        CHECK(s.preambleFired == 2);
        CHECK(s.preambleRefused == 0);
    }
    // Restore order: every discrete entry before every continuous one (touch -> set -> release).
    REQUIRE(rig.fd.log.size() >= 4);
    CHECK(rig.fd.log[0].type == Ev::Fire);
    CHECK(rig.fd.log[0].key == clipKey);
    CHECK(rig.fd.log[0].origin == Origin::Preamble);
    CHECK(static_cast<int>(rig.fd.log[0].v) == 2);
    CHECK(rig.fd.log[1].type == Ev::Touch);
    CHECK(rig.fd.log[1].key == op1);
    CHECK(rig.fd.log[2].type == Ev::Set);
    CHECK(rig.fd.log[2].v == Approx(0.3f));
    CHECK(rig.fd.log[3].type == Ev::Release);

    // The point at routine beat 1.0 fires exactly once, on the tick where clockBeat - startBeat >= 1.
    rig.runTo(4.9375);
    CHECK(rig.fd.firedLanePoints(clipKey, 1) == 0);
    rig.runTo(5.0);
    CHECK(rig.fd.firedLanePoints(clipKey, 1) == 1);
    CHECK(rig.slot(0).position == Approx(1.0));
    rig.runTo(5.9375);
    CHECK(rig.fd.firedLanePoints(clipKey, 1) == 1);
    CHECK(rig.fd.count(Ev::Touch, op0) == 0);

    // The gesture [2, 3]: one touch at its begin, sets while inside, one release at its end.
    rig.runTo(6.0);
    CHECK(rig.fd.count(Ev::Touch, op0) == 1);
    rig.runTo(6.9375);
    CHECK(rig.fd.count(Ev::Set, op0) == 16);
    CHECK(rig.fd.count(Ev::Release, op0) == 0);
    rig.runTo(7.0);
    CHECK(rig.fd.count(Ev::Release, op0) == 1);
    CHECK(rig.fd.count(Ev::Touch, op0) == 1);
    CHECK(rig.slot(0).skipped == 0);
    CHECK(rig.eng.status().fires == 1);
}

// === 8: loop re-fires the restore every cycle; stop and once both let go of their hands ===

TEST_CASE("RoutineEngine: loop re-fires the restore each cycle; stop and the end of a once routine release", "[routine][engine]")
{
    const ControlPath clipKey = layerKey(0, "activeClip");
    const ControlPath op0 = opacityKey(0);
    const ControlPath op1 = opacityKey(1);

    auto build = [&](bool loop) {
        Routine r = makeRoutine(loop ? "loop" : "once", 4.0, Clip::BeatSnapMode::Bar, loop);
        Routine::PreambleEntry restoreOp; restoreOp.key = op1; restoreOp.continuous = true; restoreOp.norm = 0.4f;
        r.preamble = { restoreOp };
        r.lanes[clipKey] = discreteLane(clipKey, { point(1, 1.0, 3) });
        r.lanes[op0] = continuousLane(op0, { gesture(0.5, 0.1f, 3.5, 0.9f) });
        return r;
    };

    SECTION("loop")
    {
        Rig rig;
        addToBank(rig.comp, build(true), 0);
        rig.tick();
        CHECK(rig.fire(0).empty());
        rig.runTo(4.0);                                   // start at bar 11
        CHECK(rig.slot(0).state == "running");
        CHECK(rig.fd.count(Ev::Touch, op1) == 1);
        CHECK(rig.slot(0).cycle == 1);
        rig.runTo(7.9375);
        CHECK(rig.fd.firedLanePoints(clipKey, 3) == 1);
        CHECK(rig.fd.count(Ev::Release, op0) == 1);       // gesture [0.5, 3.5] ended inside cycle 1
        rig.runTo(8.0);                                   // pos 4 -> cycle 2: the restore fires again
        CHECK(rig.fd.count(Ev::Touch, op1) == 2);
        CHECK(rig.slot(0).cycle == 2);
        CHECK(rig.slot(0).preambleFired == 2);
        CHECK(rig.slot(0).state == "running");
        rig.runTo(9.0);
        CHECK(rig.fd.firedLanePoints(clipKey, 3) == 2);   // the x = 1 point again, in cycle 2
        CHECK(rig.fd.count(Ev::Touch, op0) == 2);         // cycle 2's gesture is under way

        const int releasesBefore = rig.fd.count(Ev::Release, op0);
        rig.eng.stop(0);
        CHECK(rig.fd.count(Ev::Release, op0) == releasesBefore + 1);   // the held gesture let go
        CHECK(rig.slot(0).state == "idle");
        const size_t logSize = rig.fd.log.size();
        rig.runTo(14.0);
        CHECK(rig.fd.log.size() == logSize);              // nothing after stop
    }

    SECTION("once")
    {
        Rig rig;
        addToBank(rig.comp, build(false), 0);
        rig.tick();
        CHECK(rig.fire(0).empty());
        rig.runTo(7.9375);
        CHECK(rig.slot(0).state == "running");
        CHECK(rig.fd.count(Ev::Release, op0) == 1);
        rig.runTo(8.0);                                   // pos 4 == length: done, the look holds
        CHECK(rig.slot(0).state == "idle");
        CHECK(rig.fd.count(Ev::Touch, op1) == 1);         // no second restore
        const size_t logSize = rig.fd.log.size();
        rig.runTo(16.0);
        CHECK(rig.fd.log.size() == logSize);
        CHECK(rig.fd.firedLanePoints(clipKey, 3) == 1);
        CHECK(rig.fd.count(Ev::Touch, op0) == rig.fd.count(Ev::Release, op0));
    }
}

// === 9: re-fire = restart at the next boundary; global Quantize overrides; no beat -> start now ===

TEST_CASE("RoutineEngine: re-fire restarts on the next bar; global Quantize overrides; no beat starts at once", "[routine][engine]")
{
    const ControlPath op1 = opacityKey(1);
    Routine r = makeRoutine("r", 16.0, Clip::BeatSnapMode::Bar, false);
    Routine::PreambleEntry restoreOp; restoreOp.key = op1; restoreOp.continuous = true; restoreOp.norm = 0.5f;
    r.preamble = { restoreOp };

    SECTION("re-fire while running restarts at the next bar edge and restores again")
    {
        Rig rig;
        addToBank(rig.comp, r, 0);
        rig.tick();
        CHECK(rig.fire(0).empty());
        rig.runTo(4.0);
        CHECK(rig.slot(0).state == "running");
        CHECK(rig.fd.count(Ev::Touch, op1) == 1);
        rig.runTo(5.0);
        CHECK(rig.fire(0).empty());                        // restart requested
        CHECK(rig.fire(0).empty());                        // asking twice is still one restart
        rig.runTo(7.9375);
        CHECK(rig.slot(0).restarts == 0);
        CHECK(rig.fd.count(Ev::Touch, op1) == 1);
        rig.runTo(8.0);                                    // the next bar edge
        CHECK(rig.slot(0).restarts == 1);
        CHECK(rig.slot(0).startedTotalBar == 12);
        CHECK(rig.slot(0).position == Approx(0.0));
        CHECK(rig.fd.count(Ev::Touch, op1) == 2);
        CHECK(rig.eng.status().fires == 1);                // a restart is not a new fire
    }

    SECTION("fire while pending is a no-op")
    {
        Rig rig;
        addToBank(rig.comp, r, 0);
        rig.tick();
        CHECK(rig.fire(0).empty());
        CHECK(rig.fire(0).empty());
        rig.runTo(4.0);
        CHECK(rig.slot(0).state == "running");
        CHECK(rig.slot(0).restarts == 0);
        CHECK(rig.fd.count(Ev::Touch, op1) == 1);
    }

    SECTION("a forced Beat overrides the routine's own Bar")
    {
        Rig rig;
        addToBank(rig.comp, r, 0);
        rig.forced = RoutineSnap::Beat;
        rig.tick();
        rig.runTo(1.25);
        CHECK(rig.fire(0).empty());
        CHECK(rig.slot(0).state == "pending");
        rig.runTo(1.9375);
        CHECK(rig.slot(0).state == "pending");
        rig.runTo(2.0);                                    // next whole beat, mid-bar
        CHECK(rig.slot(0).state == "running");
        CHECK(rig.slot(0).startedTotalBar == 10);
    }

    SECTION("no beat yet: starts inside the fire call, with a notice")
    {
        Rig rig;
        addToBank(rig.comp, r, 0);
        rig.tick();
        rig.beatAvailable = false;
        CHECK(rig.fire(0).empty());
        CHECK(rig.slot(0).state == "running");
        CHECK(rig.fd.count(Ev::Touch, op1) == 1);
        const bool said = std::any_of(rig.fd.notices.begin(), rig.fd.notices.end(),
            [](const std::string& n) { return n.find("no beat yet") != std::string::npos; });
        CHECK(said);
    }

    SECTION("an empty pad or an unknown pad is refused in words")
    {
        Rig rig;
        addToBank(rig.comp, r, 0);
        rig.tick();
        CHECK_FALSE(rig.fire(3).empty());
        CHECK_FALSE(rig.fire(9).empty());
        CHECK_FALSE(rig.eng.status().lastError.empty());
        CHECK(rig.eng.status().fires == 0);
    }
}

// === 10: D9 stacking -- the gesture that BEGAN later wins for the rest of the earlier gesture ===

namespace
{
    // Two routines on layer 0's opacity, both quantize Bar, fired in `order` before the same bar:
    // A holds 0.2 over [0, 10] then [12, 14]; B holds 0.8 over [2, 5].
    struct Stack
    {
        Rig rig;
        ControlPath key = opacityKey(0);
        int slotA = 0, slotB = 1;
        size_t startIndex = 0;

        explicit Stack(bool aFirst, double bBegin = 2.0, double bEnd = 5.0, double aEnd = 10.0)
        {
            Routine a = makeRoutine("A", 16.0, Clip::BeatSnapMode::Bar, false);
            a.lanes[key] = continuousLane(key, { gesture(0.0, 0.2f, aEnd, 0.2f), gesture(12.0, 0.2f, 14.0, 0.2f) });
            Routine b = makeRoutine("B", 16.0, Clip::BeatSnapMode::Bar, false);
            b.lanes[key] = continuousLane(key, { gesture(bBegin, 0.8f, bEnd, 0.8f) });
            addToBank(rig.comp, a, slotA);
            addToBank(rig.comp, b, slotB);
            rig.tick();
            if (aFirst) { CHECK(rig.fire(slotA).empty()); CHECK(rig.fire(slotB).empty()); }
            else        { CHECK(rig.fire(slotB).empty()); CHECK(rig.fire(slotA).empty()); }
            rig.runTo(4.0);                                   // both start on bar 11, same tick
            CHECK(rig.slot(slotA).state == "running");
            CHECK(rig.slot(slotB).state == "running");
        }

        std::vector<float> setsBetween(size_t from, size_t to) const
        {
            std::vector<float> out;
            for (size_t i = from; i < to && i < rig.fd.log.size(); ++i)
                if (rig.fd.log[i].type == Ev::Set && rig.fd.log[i].key == key)
                    out.push_back(rig.fd.log[i].v);
            return out;
        }
    };

    void checkLaterBeginWins(Stack& st)
    {
        // Routine beats [0, 2): only A's value.
        st.rig.runTo(5.9375);
        for (float v : st.setsBetween(0, st.rig.fd.log.size()))
            CHECK(v == Approx(0.2f));

        // From B's begin on: every set on the key is B's 0.8, and no 0.2 appears after the first 0.8.
        st.rig.runTo(8.9375);                                 // routine beat 4.9375: B still inside
        const auto sets = st.setsBetween(0, st.rig.fd.log.size());
        const auto first08 = std::find_if(sets.begin(), sets.end(), [](float v) { return v > 0.5f; });
        REQUIRE(first08 != sets.end());
        CHECK(std::none_of(first08, sets.end(), [](float v) { return v < 0.5f; }));
        CHECK(st.rig.slot(st.slotA).yielded == 1);
        CHECK(st.rig.slot(st.slotB).yielded == 0);

        // B ends at routine beat 5: released once. A stays displaced for ITS gesture ([0, 10]):
        // no set at all on the key until A's next gesture begins at 12.
        st.rig.runTo(9.0);
        CHECK(st.rig.fd.count(Ev::Release, st.key) == 1);
        const size_t afterB = st.rig.fd.log.size();
        st.rig.runTo(15.9375);                                // routine beat 11.9375
        CHECK(st.setsBetween(afterB, st.rig.fd.log.size()).empty());
        CHECK(st.rig.fd.count(Ev::Release, st.key) == 1);     // the displaced A never releases B's grip

        // A's next gesture [12, 14]: A is back.
        st.rig.runTo(16.0);
        const size_t atA2 = st.rig.fd.log.size();
        st.rig.runTo(17.9375);
        const auto late = st.setsBetween(atA2 - 2, st.rig.fd.log.size());
        REQUIRE_FALSE(late.empty());
        for (float v : late)
            CHECK(v == Approx(0.2f));
        st.rig.runTo(18.0);
        CHECK(st.rig.fd.count(Ev::Release, st.key) == 2);
    }
}

TEST_CASE("RoutineEngine: two routines on one control -- the gesture that began later wins", "[routine][engine][stacking]")
{
    SECTION("fire A, then B") { Stack st(true);  checkLaterBeginWins(st); }
    SECTION("fire B, then A (fire order does not decide)") { Stack st(false); checkLaterBeginWins(st); }

    SECTION("both gestures begin in the same tick: the later-fired routine's value stands from the next tick on")
    {
        Stack st(true, /*bBegin*/ 0.0, /*bEnd*/ 4.0, /*aEnd*/ 4.0);
        // The start tick itself carries A's 0.2 then B's 0.8; from the next tick on, only 0.8.
        const size_t afterStart = st.rig.fd.log.size();
        st.rig.runTo(6.0);
        const auto later = st.setsBetween(afterStart, st.rig.fd.log.size());
        REQUIRE_FALSE(later.empty());
        for (float v : later)
            CHECK(v == Approx(0.8f));
        CHECK(st.rig.slot(st.slotA).yielded == 1);
    }

    SECTION("a restore that begins later also displaces a running gesture")
    {
        Rig rig;
        const ControlPath key = opacityKey(0);
        Routine a = makeRoutine("A", 16.0, Clip::BeatSnapMode::Bar, false);
        a.lanes[key] = continuousLane(key, { gesture(0.0, 0.2f, 10.0, 0.2f) });
        Routine b = makeRoutine("B", 16.0, Clip::BeatSnapMode::Bar, false);
        Routine::PreambleEntry restore; restore.key = key; restore.continuous = true; restore.norm = 0.6f;
        b.preamble = { restore };
        addToBank(rig.comp, a, 0);
        addToBank(rig.comp, b, 1);
        rig.tick();
        CHECK(rig.fire(0).empty());
        rig.runTo(5.0);                                   // A running since bar 11
        CHECK(rig.fire(1).empty());
        rig.runTo(8.0);                                   // B starts on bar 12: its restore touches the key
        const size_t afterRestore = rig.fd.log.size();
        rig.runTo(12.0);
        std::vector<float> sets;
        for (size_t i = afterRestore; i < rig.fd.log.size(); ++i)
            if (rig.fd.log[i].type == Ev::Set && rig.fd.log[i].key == key)
                sets.push_back(rig.fd.log[i].v);
        CHECK(sets.empty());                              // A stays quiet for the rest of its gesture
        CHECK(rig.slot(0).yielded == 1);
    }
}

// === 11: quantize -- Off / Beat, and 2 Bar / 4 Bar take their PARITY from barCount ===

TEST_CASE("RoutineEngine: quantize Off and Beat; 2 Bar and 4 Bar parity comes from barCount, not totalBarCount", "[routine][engine][quantize]")
{
    auto routine = [](Clip::BeatSnapMode q) { return makeRoutine("q", 16.0, q, false); };

    SECTION("Off starts in the fire call itself")
    {
        Rig rig;
        addToBank(rig.comp, routine(Clip::BeatSnapMode::Off), 0);
        rig.tick();
        rig.runTo(1.25);
        CHECK(rig.fire(0).empty());
        CHECK(rig.slot(0).state == "running");
        CHECK(rig.slot(0).startedTotalBar == 10);
    }

    SECTION("Beat starts on the next whole beat")
    {
        Rig rig;
        addToBank(rig.comp, routine(Clip::BeatSnapMode::Beat), 0);
        rig.tick();
        rig.runTo(2.5);
        CHECK(rig.fire(0).empty());
        rig.runTo(2.9375);
        CHECK(rig.slot(0).state == "pending");
        rig.runTo(3.0);
        CHECK(rig.slot(0).state == "running");
    }

    // s-rta-0926 routine-grid: the Beat edge is the tracker's beat (its beatPhase wrap -- the same
    // rule a Beat-quantized clip uses, Autopilot.cpp), never a whole beat of the engine's own
    // integrated clock, whose zero sits wherever the tracker happened to be when it first locked.
    SECTION("Beat starts on the tracker's beat when the tracker locked mid-beat")
    {
        Rig rig;
        addToBank(rig.comp, routine(Clip::BeatSnapMode::Beat), 0);
        FeatureSnapshot unmetered = rig.snap();   // the app starts with no tempo
        unmetered.bpm = 0.0f;
        unmetered.beatPhase = 0.0f;
        rig.eng.tick(unmetered, 0.0, rig.comp, rig.forced, false);
        rig.beat = 0.8125;                        // the tracker locks 0.8125 into a beat
        rig.tick();
        rig.runTo(2.5);
        CHECK(rig.fire(0).empty());
        rig.runTo(2.9375);
        CHECK(rig.slot(0).state == "pending");
        rig.runTo(3.0);
        CHECK(rig.slot(0).state == "running");
    }

    SECTION("2 Bar: an edge where totalBarCount is even but barCount is odd does NOT start")
    {
        Rig rig;
        rig.totalBase = 11;    // beat 4: totalBarCount 12 (even), barCount 1 (odd)
        rig.barBase = 0;       // beat 8: totalBarCount 13 (odd),  barCount 2 (even)
        addToBank(rig.comp, routine(Clip::BeatSnapMode::TwoBar), 0);
        rig.tick();
        CHECK(rig.fire(0).empty());
        rig.runTo(4.0);
        CHECK(rig.slot(0).state == "pending");
        rig.runTo(7.9375);
        CHECK(rig.slot(0).state == "pending");
        rig.runTo(8.0);
        CHECK(rig.slot(0).state == "running");
        CHECK(rig.slot(0).startedTotalBar == 13);
    }

    SECTION("4 Bar: starts on the edge where barCount reaches a multiple of 4")
    {
        Rig rig;
        rig.totalBase = 11;    // beat 4: total 12 (a multiple of 4!) but barCount 1
        rig.barBase = 0;
        addToBank(rig.comp, routine(Clip::BeatSnapMode::FourBar), 0);
        rig.tick();
        CHECK(rig.fire(0).empty());
        rig.runTo(15.9375);
        CHECK(rig.slot(0).state == "pending");
        rig.runTo(16.0);                                  // barCount 4
        CHECK(rig.slot(0).state == "running");
        CHECK(rig.slot(0).startedTotalBar == 15);
    }
}

// === 12: Binding::Action::TriggerRoutine is a pure append and round-trips its slot ===

TEST_CASE("Binding: TriggerRoutine appends after MasterSignal and round-trips targetRoutineSlot", "[routine][binding]")
{
    CHECK(static_cast<int>(Binding::Action::TriggerRoutine) == static_cast<int>(Binding::Action::MasterSignal) + 1);

    BindingManager m;
    Binding b;
    b.inputType = Binding::InputType::MidiNote;
    b.midiNote = 36;
    b.action = Binding::Action::TriggerRoutine;
    b.targetRoutineSlot = 5;
    b.triggerMode = Binding::TriggerMode::Momentary;
    m.addBinding(b);

    BindingManager back;
    back.fromVar(m.toVar());
    REQUIRE(back.getNumBindings() == 1);
    const Binding* got = back.getBindingAt(0);
    REQUIRE(got != nullptr);
    CHECK(got->action == Binding::Action::TriggerRoutine);
    CHECK(got->targetRoutineSlot == 5);
    CHECK(got->triggerMode == Binding::TriggerMode::Momentary);

    // A bindings file written before routines existed (no targetRoutineSlot key) loads slot 0.
    juce::var legacy = m.toVar();
    auto* arr = legacy.getDynamicObject()->getProperty("bindings").getArray();
    REQUIRE(arr != nullptr);
    arr->getReference(0).getDynamicObject()->removeProperty("targetRoutineSlot");
    BindingManager old;
    old.fromVar(legacy);
    REQUIRE(old.getNumBindings() == 1);
    CHECK(old.getBindingAt(0)->targetRoutineSlot == 0);
}

// === 15: two once-mode routines finishing in the SAME tick are both compacted, once each ===

TEST_CASE("RoutineEngine: two routines ending in one tick are both released and removed exactly once", "[routine][engine][compaction]")
{
    Rig rig;
    const ControlPath op0 = opacityKey(0);
    const ControlPath op1 = opacityKey(1);
    const ControlPath clipKey = layerKey(0, "activeClip");
    Routine a = makeRoutine("A", 4.0, Clip::BeatSnapMode::Bar, false);
    a.lanes[op0] = continuousLane(op0, { gesture(1.0, 0.3f, 8.0, 0.3f) });   // still held at the end
    a.lanes[clipKey] = discreteLane(clipKey, { point(1, 3.875, 4) });        // inside the skipped span
    Routine b = makeRoutine("B", 4.0, Clip::BeatSnapMode::Bar, false);
    b.lanes[op1] = continuousLane(op1, { gesture(1.0, 0.7f, 8.0, 0.7f) });
    addToBank(rig.comp, a, 0);
    addToBank(rig.comp, b, 1);
    rig.tick();
    CHECK(rig.fire(0).empty());
    CHECK(rig.fire(1).empty());
    rig.runTo(7.75);                                      // routine beat 3.75: both held
    CHECK(rig.slot(0).state == "running");
    CHECK(rig.slot(1).state == "running");
    CHECK(rig.fd.count(Ev::Release, op0) == 0);
    CHECK(rig.fd.count(Ev::Release, op1) == 0);

    CHECK(rig.fd.firedLanePoints(clipKey, 4) == 0);
    rig.runTo(8.125, 0.375);                              // ONE tick: pos 3.75 -> 4.125 for both
    CHECK(rig.slot(0).state == "idle");
    CHECK(rig.slot(1).state == "idle");
    CHECK(rig.fd.count(Ev::Release, op0) == 1);
    CHECK(rig.fd.count(Ev::Release, op1) == 1);
    CHECK(rig.fd.firedLanePoints(clipKey, 4) == 1);      // the point just before the end still fired

    const size_t logSize = rig.fd.log.size();
    rig.runTo(12.0);
    CHECK(rig.fd.log.size() == logSize);
    CHECK(rig.slot(0).skipped == 0);
    CHECK(rig.slot(1).skipped == 0);

    // Both were removed from the engine, not just hidden: pad 1 fires as a NEW run (pending, then
    // running on the next bar), never mistaken for a restart of the finished one.
    CHECK(rig.fire(0).empty());
    CHECK(rig.slot(0).state == "pending");
    rig.runTo(16.0);
    CHECK(rig.slot(0).state == "running");
    CHECK(rig.slot(0).restarts == 0);
    CHECK(rig.eng.status().fires == 3);
}

// === carried concern (b): the save refusal for an unmetered stretch says why, in whole words ===

TEST_CASE("RoutineEngine::saveRefusalText: an unmetered slice is refused in whole words", "[routine][save]")
{
    EffectLibrary lib;
    lib.registerDefaults();

    Take take;
    take.tempo.a = { { 0.0, 0.0, 0, 0.0f, "start" },      // recorded before the tracker locked
                     { 6.0, 4.0, 0, 120.0f, "lock" } };
    SliceRequest req;
    req.fromBeat = 0.0;
    req.toBeat = 8.0;
    const SliceResult res = sliceRoutine(take, req, lib);
    REQUIRE_FALSE(res.error.empty());
    CHECK(RoutineEngine::saveRefusalText(res.error)
          == "This part of the take was recorded before the tempo was known, so it has no bars to cut.");

    const std::string other = RoutineEngine::saveRefusalText("the end of the range must come after its start");
    CHECK(other.find("the end of the range must come after its start") != std::string::npos);
    CHECK(other.rfind("Could not save the routine", 0) == 0);
}

// === s-rta-0926 routine-grid: a move keeps its place -- in the take, in the cut, on the grid ===
//
// The gate take: Record pressed 0.84 of the way through beat 4 of a bar (meta.startBeatInBar 3.84),
// one move 0.6 s later (120 BPM: 1.2 beats after Record = 1.04 beats into bar 1). Driven through the
// REAL RecorderClock (seeded mid-beat, which every earlier test avoided) so the take's beat stamps
// are what the app writes. Cut from Record the move is 1.2 routine beats in (0.6 s after the bar the
// routine starts on -- the probe's grid); cut on bar lines it keeps its place in the bar (1.04).
TEST_CASE("Routine: a move keeps its place in the bar when Record fell late in a bar", "[routine][engine][bargrid]")
{
    EffectLibrary lib;
    lib.registerDefaults();

    auto tracker = [](double phase) {
        FeatureSnapshot s;
        s.clear();
        s.bpm = 120.0f;
        s.trackerState = 2;
        s.beatPhase = static_cast<float>(phase);
        return s;
    };
    RecorderClock clock;
    double phase = 0.84, wall = 0.0;
    clock.tick(tracker(phase), wall, 0);
    for (int i = 0; i < 72; ++i)   // 0.6 s at 120 Hz
    {
        wall += 1.0 / 120.0;
        phase = std::fmod(phase + 1.0 / 60.0, 1.0);
        clock.tick(tracker(phase), wall, 0);
    }
    const ClockStamp moveAt = clock.now();

    const ControlPath playKey = layerKey(0, "activeClip");   // deck-relative, as a routine lane is keyed
    ControlPath recorded = playKey;
    recorded.deckRelative = false;                            // as the take records it

    Take take;
    take.meta.startBeatInBar = 3.84;
    take.tempo = clock.tempo();
    DiscretePoint p;
    p.s = { 1, moveAt.t, 0 };
    p.beat = moveAt.beat;
    p.bpm = 120.0f;
    p.v = 1;
    take.lanes[recorded] = discreteLane(recorded, { p });

    auto cut = [&](double fromBeat, double toBeat) {
        SliceRequest req;
        req.fromBeat = fromBeat;
        req.toBeat = toBeat;
        req.name = "Cut";
        SliceResult res = sliceRoutine(take, req, lib);
        REQUIRE(res.error.empty());
        REQUIRE(res.routine.has_value());
        return *res.routine;
    };
    auto moveBeat = [&](const Routine& r) {
        auto it = r.lanes.find(playKey);
        REQUIRE(it != r.lanes.end());
        REQUIRE(it->second.points.size() == 1);
        return it->second.points[0].beat;
    };

    // Cut from Record (the probe's "fromBeat 0"): 1.2 routine beats = +0.6 s at 120 BPM.
    CHECK(moveBeat(cut(0.0, 16.0)) == Approx(1.2).margin(1e-3));

    // Cut on bar lines (bars 1 to 4): the move is 1.04 beats into its bar, as it was recorded.
    CHECK(takeBeatOfBar(take, 1) == Approx(0.16).margin(1e-9));
    Routine onBars = cut(takeBeatOfBar(take, 1), takeBeatOfBar(take, 5));
    CHECK(moveBeat(onBars) == Approx(1.04).margin(1e-3));

    // ... and the engine plays it 1.04 beats after the downbeat it starts on (Rig: 1/16-beat ticks,
    // the routine starts on the bar at beat 4, so the move fires on the tick at 5.0625, not before).
    Rig rig;
    onBars.uuid = "bars";
    onBars.quantize = Clip::BeatSnapMode::Bar;
    addToBank(rig.comp, onBars, 0);
    rig.tick();
    rig.runTo(1.0);
    CHECK(rig.fire(0).empty());
    rig.runTo(4.0);
    CHECK(rig.slot(0).state == "running");
    rig.runTo(5.0);
    CHECK(rig.fd.firedLanePoints(playKey, 1) == 0);
    rig.runTo(5.0625);
    CHECK(rig.fd.firedLanePoints(playKey, 1) == 1);
}
