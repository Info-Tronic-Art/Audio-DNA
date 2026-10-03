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
#include <map>
#include <optional>
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
        std::map<ControlPath, float> values;   // s-rta-0926b plan3 C: what each control shows now (`read`)
        bool refuseTouch = false;
        bool refuseSet = false;
        int reads = 0;                         // s-rta-0926b routines-followup: `read` calls (a Jump routine makes none)
        int preambleFireSleepMs = 0;           // s-rta-0928: a restore's discrete fire takes this long (H1)

        void wire(RoutineEngine& eng)
        {
            eng.dispatch.fire = [this](const Fired& f) {
                log.push_back({ Ev::Fire, f.key, static_cast<float>(f.p.v), f.p.origin });
                if (preambleFireSleepMs > 0 && f.p.origin == Origin::Preamble)
                    juce::Thread::sleep(preambleFireSleepMs);
                return true;
            };
            eng.dispatch.touch = [this](const ControlPath& k, const std::string&) {
                log.push_back({ Ev::Touch, k });
                return !refuseTouch;
            };
            eng.dispatch.set = [this](const ControlPath& k, float v) {
                log.push_back({ Ev::Set, k, v });
                if (refuseSet)
                    return false;
                values[k] = v;   // an accepted write moves the control
                return true;
            };
            eng.dispatch.release = [this](const ControlPath& k) { log.push_back({ Ev::Release, k }); };
            eng.dispatch.notify = [this](const std::string& s) { notices.push_back(s); };
            eng.dispatch.read = [this](const ControlPath& k) -> std::optional<float> {
                ++reads;
                const auto it = values.find(k);
                if (it == values.end())
                    return std::nullopt;
                return it->second;
            };
        }

        int count(Ev::Type t, const ControlPath& key) const
        {
            return static_cast<int>(std::count_if(log.begin(), log.end(),
                [&](const Ev& e) { return e.type == t && e.key == key; }));
        }
        // The value of the last Set on `key` (NaN when there is none).
        float lastSet(const ControlPath& key) const
        {
            for (auto it = log.rbegin(); it != log.rend(); ++it)
                if (it->type == Ev::Set && it->key == key)
                    return it->v;
            return std::nanf("");
        }
        // Every event on `key` from log index `from` on, in order.
        std::vector<Ev> on(const ControlPath& key, size_t from = 0) const
        {
            std::vector<Ev> out;
            for (size_t i = from; i < log.size(); ++i)
                if (log[i].key == key)
                    out.push_back(log[i]);
            return out;
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
            s.totalBeatCount = static_cast<uint32_t>(std::floor(beat));   // s-rta-0927: what BPMTracker publishes
            s.beatInBar = static_cast<uint8_t>(static_cast<int>(std::floor(beat)) % 4);   // bar edges at multiples of 4
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

    // Test 7's routine (s-rta-0926b plan3 C.5): a continuous restore of layer 1's opacity to 0.3 (no lane on
    // it), a discrete restore of layer 0's clip, a point at 1.0 and a gesture on layer 0's opacity over [2, 3].
    // 8 beats, quantize Bar, once.
    Routine test7Routine(Clip::BeatSnapMode quantize = Clip::BeatSnapMode::Bar)
    {
        const ControlPath clipKey = layerKey(0, "activeClip");
        const ControlPath op0 = opacityKey(0);
        const ControlPath op1 = opacityKey(1);
        Routine r = makeRoutine("a", 8.0, quantize, false);
        Routine::PreambleEntry restoreClip; restoreClip.key = clipKey; restoreClip.v = 2;
        Routine::PreambleEntry restoreOp; restoreOp.key = op1; restoreOp.continuous = true; restoreOp.norm = 0.3f;
        r.preamble = { restoreClip, restoreOp };
        r.lanes[clipKey] = discreteLane(clipKey, { point(1, 1.0, 1) });
        r.lanes[op0] = continuousLane(op0, { gesture(2.0, 0.2f, 3.0, 0.8f) });
        return r;
    }

    void checkEvent(const Ev& e, Ev::Type type, const ControlPath& key)
    {
        CHECK(e.type == type);
        CHECK(e.key == key);
    }
}

// === 7: pending until the next bar, then restore (discrete before continuous), then the lanes ===
// s-rta-0926b plan3 C G1: the continuous restore GLIDES over the last beat before the bar and lands ON it,
// after the discrete restore fires on the bar.

TEST_CASE("RoutineEngine: waits for the next bar, restores, then plays its lanes on its own beat grid", "[routine][engine][glide]")
{
    Rig rig;
    const ControlPath clipKey = layerKey(0, "activeClip");
    const ControlPath op0 = opacityKey(0);
    const ControlPath op1 = opacityKey(1);

    addToBank(rig.comp, test7Routine(), 0);
    rig.fd.values[op1] = 0.9f;   // what layer 1's opacity shows when the routine is fired

    rig.tick();                  // beat 0, totalBarCount 10
    rig.runTo(1.0);
    CHECK(rig.slot(0).state == "idle");
    CHECK(rig.fire(0).empty());  // Bar: the boundary is beat 4.0, the glide window [3.0, 4.0]
    CHECK(rig.slot(0).state == "pending");

    rig.runTo(2.9375);
    CHECK(rig.fd.count(Ev::Touch, op1) == 0);
    CHECK(rig.fd.log.empty());

    const size_t at3 = rig.fd.log.size();
    rig.runTo(3.0);              // the glide begins: touch, then a set from where the knob is (0.9)
    {
        const auto ev = rig.fd.on(op1, at3);
        REQUIRE(ev.size() == 2);
        checkEvent(ev[0], Ev::Touch, op1);
        checkEvent(ev[1], Ev::Set, op1);
        CHECK(ev[1].v == Approx(0.9f));
    }
    rig.runTo(3.5);
    CHECK(rig.fd.lastSet(op1) == Approx(0.6f));
    rig.runTo(3.9375);           // still bar 10
    CHECK(rig.fd.count(Ev::Touch, op1) == 1);
    CHECK(rig.fd.count(Ev::Release, op1) == 0);
    {
        const auto s = rig.slot(0);
        CHECK(s.state == "pending");
        CHECK(s.glides == 1);
        CHECK(s.preambleFired == 1);
    }

    const size_t at4 = rig.fd.log.size();
    rig.runTo(4.0);              // totalBarCount 10 -> 11: the start boundary
    {
        const auto s = rig.slot(0);
        CHECK(s.state == "running");
        CHECK(s.startedTotalBar == 11);
        CHECK(s.preambleCount == 2);
        CHECK(s.preambleFired == 2);
        CHECK(s.preambleRefused == 0);
        CHECK(s.glides == 0);
    }
    // This tick: the discrete restore fires on the bar, then the glide lands ON it and lets go.
    REQUIRE(rig.fd.log.size() == at4 + 3);
    CHECK(rig.fd.log[at4].type == Ev::Fire);
    CHECK(rig.fd.log[at4].key == clipKey);
    CHECK(rig.fd.log[at4].origin == Origin::Preamble);
    CHECK(static_cast<int>(rig.fd.log[at4].v) == 2);
    checkEvent(rig.fd.log[at4 + 1], Ev::Set, op1);
    CHECK(rig.fd.log[at4 + 1].v == Approx(0.3f));
    checkEvent(rig.fd.log[at4 + 2], Ev::Release, op1);
    REQUIRE_FALSE(rig.fd.notices.empty());
    CHECK(rig.fd.notices.back().rfind("Routine Routine a started.", 0) == 0);   // no growing bar number

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
        // s-rta-0926b plan3 C G12: the restart's restore glides over the last beat before ITS bar, from
        // where the knob is when the glide begins.
        Rig rig;
        addToBank(rig.comp, r, 0);
        rig.tick();
        rig.runTo(1.0);
        CHECK(rig.fire(0).empty());
        rig.runTo(4.0);
        CHECK(rig.slot(0).state == "running");
        CHECK(rig.fd.count(Ev::Touch, op1) == 1);          // the start's glide, [3.0, 4.0]
        rig.runTo(5.0);
        CHECK(rig.fire(0).empty());                        // restart requested
        CHECK(rig.fire(0).empty());                        // asking twice is still one restart
        rig.runTo(5.5);
        rig.fd.values[op1] = 0.9f;                         // a hand moved the knob while nobody held it
        rig.runTo(6.9375);
        CHECK(rig.slot(0).restarts == 0);
        CHECK(rig.fd.count(Ev::Touch, op1) == 1);
        rig.runTo(7.0);                                    // the restart's glide begins: [7.0, 8.0]
        CHECK(rig.fd.count(Ev::Touch, op1) == 2);
        rig.runTo(7.5);
        CHECK(rig.fd.lastSet(op1) == Approx(0.7f));        // 0.9 -> 0.5, half way
        rig.runTo(7.9375);
        CHECK(rig.slot(0).restarts == 0);
        CHECK(rig.fd.count(Ev::Release, op1) == 1);        // only the start's glide has let go
        const size_t at8 = rig.fd.log.size();
        rig.runTo(8.0);                                    // the next bar edge
        CHECK(rig.slot(0).restarts == 1);
        CHECK(rig.slot(0).startedTotalBar == 12);
        CHECK(rig.slot(0).position == Approx(0.0));
        {
            const auto ev = rig.fd.on(op1, at8);
            REQUIRE(ev.size() == 2);
            checkEvent(ev[0], Ev::Set, op1);
            CHECK(ev[0].v == Approx(0.5f));
            checkEvent(ev[1], Ev::Release, op1);
        }
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
        // s-rta-0926b plan3 C: B's restore now begins one beat BEFORE its bar -- its glide's touch at 7.0 is
        // the later begin; from there every write on the key is B's, rising from A's 0.2 to 0.6.
        rig.runTo(6.9375);
        const size_t atB = rig.fd.log.size();
        rig.runTo(7.0);
        const auto touched = std::find_if(rig.fd.log.begin() + static_cast<std::ptrdiff_t>(atB), rig.fd.log.end(),
                                          [&](const Ev& e) { return e.type == Ev::Touch && e.key == key; });
        REQUIRE(touched != rig.fd.log.end());
        const size_t fromB = static_cast<size_t>(touched - rig.fd.log.begin());
        rig.runTo(7.0625);
        CHECK(rig.slot(0).yielded == 1);                  // A's next write was refused
        rig.runTo(8.0);                                   // B starts on bar 12: its glide lands and lets go
        {
            std::vector<float> bSets;
            for (size_t i = fromB; i < rig.fd.log.size(); ++i)
                if (rig.fd.log[i].type == Ev::Set && rig.fd.log[i].key == key)
                    bSets.push_back(rig.fd.log[i].v);
            REQUIRE(bSets.size() == 17);                  // 7.0 .. 8.0 in 1/16 steps: B's glide only
            CHECK(bSets.front() == Approx(0.2f));
            CHECK(bSets.back() == Approx(0.6f));
            CHECK(std::is_sorted(bSets.begin(), bSets.end()));
        }
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

    auto tracker = [](double beats) {   // s-rta-0927: the continuous beat -> count + phase, as BPMTracker publishes
        FeatureSnapshot s;
        s.clear();
        s.bpm = 120.0f;
        s.trackerState = 2;
        s.beatPhase = static_cast<float>(beats - std::floor(beats));
        s.totalBeatCount = static_cast<uint32_t>(std::floor(beats));
        return s;
    };
    RecorderClock clock;
    double beats = 0.84, wall = 0.0;
    clock.tick(tracker(beats), wall, 0);
    for (int i = 0; i < 72; ++i)   // 0.6 s at 120 Hz
    {
        wall += 1.0 / 120.0;
        beats += 1.0 / 60.0;
        clock.tick(tracker(beats), wall, 0);
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

// === s-rta-0926b plan3 C: the routine's restore GLIDES (start, loop return, restart) ===
//
// Rig: 1/16-beat ticks, bar edges at multiples of 4. "Fire at X" = runTo(X) then fire(), so the next tick
// is X + 1/16. The continuous restore of op1 travels from what the control shows (FakeDispatch::values,
// the `read` seam) to the recorded start value on a straight line; the discrete half fires on the bar.

TEST_CASE("RoutineEngine glide G2: fired inside the last beat, the restore glides from the fire to the bar", "[routine][engine][glide]")
{
    Rig rig;
    const ControlPath clipKey = layerKey(0, "activeClip");
    const ControlPath op1 = opacityKey(1);
    addToBank(rig.comp, test7Routine(), 0);
    rig.fd.values[op1] = 0.9f;
    rig.tick();
    rig.runTo(3.5);
    CHECK(rig.fire(0).empty());                       // window [3.5, 4.0]

    const size_t mark = rig.fd.log.size();
    rig.runTo(3.5625);
    {
        const auto ev = rig.fd.on(op1, mark);
        REQUIRE(ev.size() == 2);
        checkEvent(ev[0], Ev::Touch, op1);
        checkEvent(ev[1], Ev::Set, op1);
        CHECK(ev[1].v == Approx(0.825f));             // 0.9 + 0.125 * (0.3 - 0.9)
    }
    rig.runTo(3.75);
    CHECK(rig.fd.lastSet(op1) == Approx(0.6f));
    rig.runTo(3.9375);
    const size_t at4 = rig.fd.log.size();
    rig.runTo(4.0);
    REQUIRE(rig.fd.log.size() == at4 + 3);
    checkEvent(rig.fd.log[at4], Ev::Fire, clipKey);
    checkEvent(rig.fd.log[at4 + 1], Ev::Set, op1);
    CHECK(rig.fd.log[at4 + 1].v == Approx(0.3f));
    checkEvent(rig.fd.log[at4 + 2], Ev::Release, op1);
    CHECK(rig.slot(0).state == "running");
}

TEST_CASE("RoutineEngine glide G3: under a quarter beat to go, or Quantize Off, the glide spills a quarter beat past the start", "[routine][engine][glide]")
{
    const ControlPath clipKey = layerKey(0, "activeClip");
    const ControlPath op1 = opacityKey(1);

    SECTION("fired 1/16 beat before the bar: the window is [3.9375, 4.1875]")
    {
        Rig rig;
        addToBank(rig.comp, test7Routine(), 0);
        rig.fd.values[op1] = 0.9f;
        rig.tick();
        rig.runTo(3.9375);
        CHECK(rig.fire(0).empty());
        const size_t mark = rig.fd.log.size();
        rig.runTo(4.0);
        CHECK(rig.slot(0).state == "running");
        REQUIRE(rig.fd.log.size() == mark + 3);
        checkEvent(rig.fd.log[mark], Ev::Fire, clipKey);
        checkEvent(rig.fd.log[mark + 1], Ev::Touch, op1);
        checkEvent(rig.fd.log[mark + 2], Ev::Set, op1);
        CHECK(rig.fd.log[mark + 2].v == Approx(0.75f));
        CHECK(rig.fd.count(Ev::Release, op1) == 0);
        CHECK(rig.slot(0).glides == 1);
        rig.runTo(4.0625);
        CHECK(rig.fd.lastSet(op1) == Approx(0.6f));
        rig.runTo(4.125);
        CHECK(rig.fd.lastSet(op1) == Approx(0.45f));
        CHECK(rig.fd.count(Ev::Release, op1) == 0);
        const size_t mark2 = rig.fd.log.size();
        rig.runTo(4.1875);
        const auto ev = rig.fd.on(op1, mark2);
        REQUIRE(ev.size() == 2);
        checkEvent(ev[0], Ev::Set, op1);
        CHECK(ev[0].v == Approx(0.3f));
        checkEvent(ev[1], Ev::Release, op1);
        CHECK(rig.slot(0).glides == 0);
    }

    SECTION("Quantize Off: the glide begins inside the fire call and lands a quarter beat later")
    {
        Rig rig;
        addToBank(rig.comp, test7Routine(Clip::BeatSnapMode::Off), 0);
        rig.fd.values[op1] = 0.9f;
        rig.tick();
        rig.runTo(1.0);
        const size_t mark = rig.fd.log.size();
        CHECK(rig.fire(0).empty());
        CHECK(rig.slot(0).state == "running");
        REQUIRE(rig.fd.log.size() == mark + 3);
        checkEvent(rig.fd.log[mark], Ev::Fire, clipKey);
        checkEvent(rig.fd.log[mark + 1], Ev::Touch, op1);
        checkEvent(rig.fd.log[mark + 2], Ev::Set, op1);
        CHECK(rig.fd.log[mark + 2].v == Approx(0.9f));
        rig.runTo(1.1875);
        CHECK(rig.fd.count(Ev::Release, op1) == 0);
        CHECK(rig.fd.lastSet(op1) == Approx(0.45f));
        const size_t mark2 = rig.fd.log.size();
        rig.runTo(1.25);
        const auto ev = rig.fd.on(op1, mark2);
        REQUIRE(ev.size() == 2);
        checkEvent(ev[0], Ev::Set, op1);
        CHECK(ev[0].v == Approx(0.3f));
        checkEvent(ev[1], Ev::Release, op1);
    }
}

TEST_CASE("RoutineEngine glide G4: the recording's own move on the knob cancels the glide -- no release, no second hand", "[routine][engine][glide]")
{
    Rig rig;
    const ControlPath op1 = opacityKey(1);
    Routine r = test7Routine();
    r.lanes[op1] = continuousLane(op1, { gesture(0.125, 0.2f, 1.0, 0.2f) });
    addToBank(rig.comp, r, 0);
    rig.fd.values[op1] = 0.9f;
    rig.tick();
    rig.runTo(3.9375);
    CHECK(rig.fire(0).empty());                       // spill [3.9375, 4.1875]
    rig.runTo(4.0625);
    CHECK(rig.fd.lastSet(op1) == Approx(0.6f));

    const size_t mark = rig.fd.log.size();
    rig.runTo(4.125);                                 // routine beat 0.125: the recording takes the knob
    CHECK(rig.fd.count(Ev::Touch, op1) == 2);
    {
        const auto ev = rig.fd.on(op1, mark);
        REQUIRE(ev.size() == 2);
        checkEvent(ev[0], Ev::Touch, op1);
        checkEvent(ev[1], Ev::Set, op1);
        CHECK(ev[1].v == Approx(0.2f));               // the gesture's value -- the glide wrote nothing
    }
    rig.runTo(4.9375);
    for (const Ev& e : rig.fd.on(op1, mark))
        if (e.type == Ev::Set)
            CHECK(e.v == Approx(0.2f));
    CHECK(rig.fd.count(Ev::Release, op1) == 0);       // the glide never closed the recording's grip
    rig.runTo(5.0);                                   // the gesture's end (routine beat 1.0)
    CHECK(rig.fd.count(Ev::Release, op1) == 1);
    CHECK(rig.slot(0).preambleRefused == 0);
    CHECK(rig.slot(0).glides == 0);
    rig.runTo(6.0);
    CHECK(rig.fd.count(Ev::Release, op1) == 1);
    CHECK(rig.fd.count(Ev::Touch, op1) == 2);
}

TEST_CASE("RoutineEngine glide G5: a loop eases back to its start look over the last beat and lands on the loop point", "[routine][engine][glide]")
{
    Rig rig;
    const ControlPath op0 = opacityKey(0);
    const ControlPath op1 = opacityKey(1);
    Routine r = makeRoutine("L", 4.0, Clip::BeatSnapMode::Bar, true);
    Routine::PreambleEntry p1; p1.key = op1; p1.continuous = true; p1.norm = 0.4f;
    Routine::PreambleEntry p0; p0.key = op0; p0.continuous = true; p0.norm = 0.1f;
    r.preamble = { p1, p0 };
    r.lanes[op0] = continuousLane(op0, { gesture(0.5, 0.1f, 4.0, 0.9f) });   // held to the very end
    addToBank(rig.comp, r, 0);
    rig.fd.values[op1] = 0.9f;
    rig.fd.values[op0] = 0.0f;
    rig.tick();
    rig.runTo(1.0);
    CHECK(rig.fire(0).empty());
    rig.runTo(4.0);
    CHECK(rig.slot(0).state == "running");
    CHECK(rig.slot(0).cycle == 1);
    rig.runTo(6.0);
    rig.fd.values[op1] = 0.9f;                        // a hand moved op1 while nobody held it

    rig.runTo(6.9375);
    CHECK(rig.fd.count(Ev::Touch, op1) == 1);
    const size_t at7 = rig.fd.log.size();
    rig.runTo(7.0);                                   // the cycle's last beat: op1's return glide [7.0, 8.0]
    {
        const auto ev = rig.fd.on(op1, at7);
        REQUIRE(ev.size() == 2);
        checkEvent(ev[0], Ev::Touch, op1);
        checkEvent(ev[1], Ev::Set, op1);
        CHECK(ev[1].v == Approx(0.9f));
    }
    rig.runTo(7.5);
    CHECK(rig.fd.lastSet(op1) == Approx(0.65f));
    rig.runTo(7.9375);
    const float gestureLast = rig.fd.lastSet(op0);    // where the recording's own hand left op0
    CHECK(gestureLast < 0.9f);   // the last tick inside the gesture caught it short of its recorded end
    CHECK(rig.fd.count(Ev::Touch, op0) == 2);         // the start's glide + cycle 1's gesture
    const size_t at8 = rig.fd.log.size();
    rig.runTo(8.0);                                   // the loop point
    {
        const auto ev1 = rig.fd.on(op1, at8);         // op1 lands ON the loop point
        REQUIRE(ev1.size() == 2);
        checkEvent(ev1[0], Ev::Set, op1);
        CHECK(ev1[0].v == Approx(0.4f));
        checkEvent(ev1[1], Ev::Release, op1);
        const auto ev0 = rig.fd.on(op0, at8);         // op0 held to the end: its end lands, then a quarter-beat spill
        REQUIRE(ev0.size() == 4);
        checkEvent(ev0[0], Ev::Set, op0);             // s-rta-0928: the gesture's recorded end (0.9) first ...
        CHECK(ev0[0].v == Approx(0.9f));
        checkEvent(ev0[1], Ev::Release, op0);         // ... then it lets go
        checkEvent(ev0[2], Ev::Touch, op0);
        checkEvent(ev0[3], Ev::Set, op0);
        CHECK(ev0[3].v == Approx(0.9f));              // the glide starts where the recording left op0: its end
        const auto s = rig.slot(0);
        CHECK(s.cycle == 2);
        CHECK(s.preambleFired == 4);
        CHECK(s.glides == 1);
    }
    rig.runTo(8.1875);
    const size_t at825 = rig.fd.log.size();
    rig.runTo(8.25);
    {
        const auto ev = rig.fd.on(op0, at825);
        REQUIRE(ev.size() == 2);
        checkEvent(ev[0], Ev::Set, op0);
        CHECK(ev[0].v == Approx(0.1f));
        checkEvent(ev[1], Ev::Release, op0);
        CHECK(rig.slot(0).glides == 0);
    }
    rig.runTo(8.4375);
    CHECK(rig.fd.count(Ev::Touch, op0) == 3);
    rig.runTo(8.5);                                   // cycle 2's gesture takes op0
    CHECK(rig.fd.count(Ev::Touch, op0) == 4);
}

TEST_CASE("RoutineEngine glide G6: a routine that takes the knob later keeps it -- the gliding one yields and never releases", "[routine][engine][glide][stacking]")
{
    Rig rig;
    const ControlPath op1 = opacityKey(1);
    Routine a = makeRoutine("A", 8.0, Clip::BeatSnapMode::Bar, false);
    Routine::PreambleEntry restore; restore.key = op1; restore.continuous = true; restore.norm = 0.3f;
    a.preamble = { restore };
    Routine b = makeRoutine("B", 8.0, Clip::BeatSnapMode::Off, false);
    b.lanes[op1] = continuousLane(op1, { gesture(0.0, 0.8f, 2.0, 0.8f) });
    addToBank(rig.comp, a, 0);
    addToBank(rig.comp, b, 1);
    rig.fd.values[op1] = 0.9f;
    rig.tick();
    rig.runTo(1.0);
    CHECK(rig.fire(0).empty());                       // A's glide [3.0, 4.0]
    rig.runTo(3.25);
    CHECK(rig.fd.count(Ev::Touch, op1) == 1);
    const size_t mark = rig.fd.log.size();
    CHECK(rig.fire(1).empty());                       // B starts at once and takes op1
    CHECK(rig.slot(1).state == "running");
    rig.runTo(3.3125);
    CHECK(rig.slot(0).yielded == 1);
    CHECK(rig.slot(0).preambleRefused == 0);
    CHECK(rig.slot(0).glides == 0);
    rig.runTo(5.1875);
    CHECK(rig.fd.count(Ev::Release, op1) == 0);
    rig.runTo(5.25);                                  // B's gesture ends (routine beat 2.0)
    CHECK(rig.fd.count(Ev::Release, op1) == 1);
    rig.runTo(12.0);                                  // A started at 4.0 and ended at 12.0
    for (const Ev& e : rig.fd.on(op1, mark))
        if (e.type == Ev::Set)
            CHECK(e.v == Approx(0.8f));
    CHECK(rig.fd.count(Ev::Release, op1) == 1);       // A never released B's knob
    CHECK(rig.slot(0).state == "idle");
}

TEST_CASE("RoutineEngine glide G7: a human hand refuses the glide's touch, or cuts in mid-glide", "[routine][engine][glide]")
{
    const ControlPath op1 = opacityKey(1);

    SECTION("the touch is refused: left alone, counted once, said in the start notice")
    {
        Rig rig;
        addToBank(rig.comp, test7Routine(), 0);
        rig.fd.values[op1] = 0.9f;
        rig.fd.refuseTouch = true;
        rig.tick();
        rig.runTo(1.0);
        CHECK(rig.fire(0).empty());
        rig.runTo(4.0);
        CHECK(rig.slot(0).state == "running");
        CHECK(rig.fd.count(Ev::Set, op1) == 0);
        CHECK(rig.fd.count(Ev::Touch, op1) == 1);     // one attempt, not a second one at the start
        CHECK(rig.slot(0).preambleRefused == 1);
        CHECK(rig.slot(0).glides == 0);
        REQUIRE_FALSE(rig.fd.notices.empty());
        CHECK(rig.fd.notices.back().find("1 control you are holding was left alone") != std::string::npos);
    }

    SECTION("a set is refused mid-glide: dropped where it is, no release")
    {
        Rig rig;
        addToBank(rig.comp, test7Routine(), 0);
        rig.fd.values[op1] = 0.9f;
        rig.tick();
        rig.runTo(1.0);
        CHECK(rig.fire(0).empty());
        rig.runTo(3.5);
        CHECK(rig.fd.count(Ev::Set, op1) == 9);       // 3.0 .. 3.5, all accepted
        rig.fd.refuseSet = true;                      // a human grabs the knob
        const size_t mark = rig.fd.log.size();
        rig.runTo(3.5625);
        CHECK(rig.slot(0).glides == 0);
        CHECK(rig.slot(0).preambleRefused == 1);
        rig.runTo(4.0);
        CHECK(rig.slot(0).state == "running");
        CHECK(rig.fd.count(Ev::Release, op1) == 0);
        CHECK(rig.fd.on(op1, mark).size() == 1);      // only the one refused set -- no retry, no release
        CHECK(rig.slot(0).preambleRefused == 1);
    }
}

TEST_CASE("RoutineEngine glide G8: a stop during the wait lets go where the glide left the knob", "[routine][engine][glide]")
{
    Rig rig;
    const ControlPath op1 = opacityKey(1);
    addToBank(rig.comp, test7Routine(), 0);
    Routine b = makeRoutine("B", 4.0, Clip::BeatSnapMode::Off, false);
    b.lanes[op1] = continuousLane(op1, { gesture(0.0, 0.5f, 1.0, 0.5f) });
    addToBank(rig.comp, b, 1);
    rig.fd.values[op1] = 0.9f;
    rig.tick();
    rig.runTo(1.0);
    CHECK(rig.fire(0).empty());
    rig.runTo(3.5);
    CHECK(rig.fd.lastSet(op1) == Approx(0.6f));
    const size_t mark = rig.fd.log.size();
    rig.eng.stop(0);
    {
        const auto ev = rig.fd.on(op1, mark);
        REQUIRE(ev.size() == 1);
        checkEvent(ev[0], Ev::Release, op1);
    }
    CHECK(rig.slot(0).state == "idle");
    CHECK(rig.slot(0).glides == 0);
    CHECK(rig.fd.values[op1] == Approx(0.6f));        // the knob stays where the glide left it
    rig.runTo(8.0);
    CHECK(rig.fd.on(op1, mark).size() == 1);          // nothing more from the stopped routine

    // The stopped routine owns nothing any more: a later routine's gesture on op1 writes and lets go freely.
    CHECK(rig.fire(1).empty());
    rig.runTo(9.0);
    CHECK(rig.fd.count(Ev::Touch, op1) == 2);
    CHECK(rig.fd.lastSet(op1) == Approx(0.5f));
    CHECK(rig.fd.count(Ev::Release, op1) == 2);
    CHECK(rig.slot(1).yielded == 0);
}

TEST_CASE("RoutineEngine glide G9: no beat, or no read seam -- the continuous restore lands in one call, as before", "[routine][engine][glide]")
{
    const ControlPath clipKey = layerKey(0, "activeClip");
    const ControlPath op1 = opacityKey(1);

    SECTION("no beat yet: the whole restore inside the fire call")
    {
        Rig rig;
        addToBank(rig.comp, test7Routine(), 0);
        rig.fd.values[op1] = 0.9f;
        rig.tick();
        rig.runTo(1.0);
        rig.beatAvailable = false;
        const size_t mark = rig.fd.log.size();
        CHECK(rig.fire(0).empty());
        CHECK(rig.slot(0).state == "running");
        CHECK(rig.slot(0).glides == 0);
        REQUIRE(rig.fd.log.size() == mark + 4);
        checkEvent(rig.fd.log[mark], Ev::Fire, clipKey);
        checkEvent(rig.fd.log[mark + 1], Ev::Touch, op1);
        checkEvent(rig.fd.log[mark + 2], Ev::Set, op1);
        CHECK(rig.fd.log[mark + 2].v == Approx(0.3f));
        checkEvent(rig.fd.log[mark + 3], Ev::Release, op1);
    }

    SECTION("no read seam wired: touch, set 0.3 and release on the bar")
    {
        Rig rig;
        rig.eng.dispatch.read = nullptr;
        addToBank(rig.comp, test7Routine(), 0);
        rig.fd.values[op1] = 0.9f;
        rig.tick();
        rig.runTo(1.0);
        CHECK(rig.fire(0).empty());
        rig.runTo(3.9375);
        CHECK(rig.fd.count(Ev::Touch, op1) == 0);
        const size_t mark = rig.fd.log.size();
        rig.runTo(4.0);
        REQUIRE(rig.fd.log.size() == mark + 4);
        checkEvent(rig.fd.log[mark], Ev::Fire, clipKey);
        checkEvent(rig.fd.log[mark + 1], Ev::Touch, op1);
        checkEvent(rig.fd.log[mark + 2], Ev::Set, op1);
        CHECK(rig.fd.log[mark + 2].v == Approx(0.3f));
        checkEvent(rig.fd.log[mark + 3], Ev::Release, op1);
        CHECK(rig.slot(0).preambleFired == 2);
    }
}

TEST_CASE("RoutineEngine glide G11: loop switched off inside the return glide -- the glide is let go at the end, never leaked", "[routine][engine][glide]")
{
    Rig rig;
    const ControlPath clipKey = layerKey(0, "activeClip");
    const ControlPath op0 = opacityKey(0);
    const ControlPath op1 = opacityKey(1);
    Routine r = makeRoutine("loop", 4.0, Clip::BeatSnapMode::Bar, true);   // test 8's loop routine
    Routine::PreambleEntry restoreOp; restoreOp.key = op1; restoreOp.continuous = true; restoreOp.norm = 0.4f;
    r.preamble = { restoreOp };
    r.lanes[clipKey] = discreteLane(clipKey, { point(1, 1.0, 3) });
    r.lanes[op0] = continuousLane(op0, { gesture(0.5, 0.1f, 3.5, 0.9f) });
    addToBank(rig.comp, r, 0);
    rig.fd.values[op1] = 0.9f;
    rig.tick();
    CHECK(rig.fire(0).empty());
    rig.runTo(4.0);
    CHECK(rig.slot(0).state == "running");
    CHECK(rig.fd.count(Ev::Release, op1) == 1);       // the start's glide landed and let go
    rig.runTo(7.5);                                   // the return glide has been in flight since 7.0
    CHECK(rig.fd.count(Ev::Touch, op1) == 2);
    CHECK(rig.slot(0).glides == 1);

    for (auto& routine : rig.comp.routines)           // the performer switches loop off mid-glide
        if (routine.uuid == "loop")
            routine.loop = false;
    rig.runTo(7.9375);
    CHECK(rig.fd.count(Ev::Release, op1) == 1);
    const size_t mark = rig.fd.log.size();
    rig.runTo(8.0);                                   // the end: once now -- the glide is let go where it is
    {
        const auto ev = rig.fd.on(op1, mark);
        REQUIRE(ev.size() == 1);
        checkEvent(ev[0], Ev::Release, op1);
    }
    CHECK(rig.fd.count(Ev::Release, op1) == 2);
    CHECK(rig.slot(0).state == "idle");
    CHECK(rig.slot(0).glides == 0);
    const size_t logSize = rig.fd.log.size();
    rig.runTo(12.0);
    CHECK(rig.fd.log.size() == logSize);
}

// === s-rta-0926b routines-followup ITEM 2: a per-routine RESTORE STYLE -- Ease (default: the plan3 C glide above)
// or Jump (Boris: "we should have controls for jump or ease in each"). Jump is exactly the pre-glide restore: the
// discrete and continuous halves together, in one call, ON the boundary -- at the start, every loop return and a
// re-fire restart -- and Dispatch::read is never called. ===

TEST_CASE("RoutineEngine restore style J1: Jump restores in one call ON the start boundary -- no glide, no read", "[routine][engine][restorestyle]")
{
    const ControlPath clipKey = layerKey(0, "activeClip");
    const ControlPath op1 = opacityKey(1);
    auto styled = [](Clip::BeatSnapMode q, Routine::RestoreStyle style) {
        Routine r = test7Routine(q);
        r.restoreStyle = style;
        return r;
    };

    SECTION("fired more than a beat before the bar: nothing moves until the bar, then touch / set 0.3 / release")
    {
        Rig rig;
        addToBank(rig.comp, styled(Clip::BeatSnapMode::Bar, Routine::RestoreStyle::Jump), 0);
        rig.fd.values[op1] = 0.9f;
        rig.tick();
        rig.runTo(1.0);
        CHECK(rig.fire(0).empty());
        CHECK(rig.slot(0).restoreStyle == "jump");
        rig.runTo(3.9375);
        CHECK(rig.slot(0).state == "pending");
        CHECK(rig.fd.log.empty());                    // Ease touches op1 at 3.0 (G1)
        CHECK(rig.slot(0).glides == 0);
        const size_t at4 = rig.fd.log.size();
        rig.runTo(4.0);                               // the bar
        CHECK(rig.slot(0).state == "running");
        REQUIRE(rig.fd.log.size() == at4 + 4);
        checkEvent(rig.fd.log[at4], Ev::Fire, clipKey);
        CHECK(rig.fd.log[at4].origin == Origin::Preamble);
        checkEvent(rig.fd.log[at4 + 1], Ev::Touch, op1);
        checkEvent(rig.fd.log[at4 + 2], Ev::Set, op1);
        CHECK(rig.fd.log[at4 + 2].v == Approx(0.3f)); // the recorded value at once, not a step from 0.9
        checkEvent(rig.fd.log[at4 + 3], Ev::Release, op1);
        CHECK(rig.slot(0).preambleFired == 2);
        CHECK(rig.slot(0).preambleRefused == 0);
        CHECK(rig.slot(0).glides == 0);
        CHECK(rig.fd.reads == 0);                     // Dispatch::read is never used
    }

    SECTION("fired 1/16 beat before the bar: a hard set ON the bar, no quarter-beat spill")
    {
        Rig rig;
        addToBank(rig.comp, styled(Clip::BeatSnapMode::Bar, Routine::RestoreStyle::Jump), 0);
        rig.fd.values[op1] = 0.9f;
        rig.tick();
        rig.runTo(3.9375);
        CHECK(rig.fire(0).empty());
        const size_t at4 = rig.fd.log.size();
        rig.runTo(4.0);
        REQUIRE(rig.fd.log.size() == at4 + 4);        // Ease (G3): Fire, Touch, Set 0.75 -- a spill to 4.1875
        checkEvent(rig.fd.log[at4 + 1], Ev::Touch, op1);
        checkEvent(rig.fd.log[at4 + 2], Ev::Set, op1);
        CHECK(rig.fd.log[at4 + 2].v == Approx(0.3f));
        checkEvent(rig.fd.log[at4 + 3], Ev::Release, op1);
        const size_t after = rig.fd.log.size();
        rig.runTo(4.1875);
        CHECK(rig.fd.on(op1, after).empty());
        CHECK(rig.fd.reads == 0);
    }

    SECTION("Quantize Off: the whole restore inside the fire call")
    {
        Rig rig;
        addToBank(rig.comp, styled(Clip::BeatSnapMode::Off, Routine::RestoreStyle::Jump), 0);
        rig.fd.values[op1] = 0.9f;
        rig.tick();
        rig.runTo(1.0);
        const size_t mark = rig.fd.log.size();
        CHECK(rig.fire(0).empty());
        CHECK(rig.slot(0).state == "running");
        REQUIRE(rig.fd.log.size() == mark + 4);       // Ease (G3): Touch + Set 0.9 now, landing at 1.25
        checkEvent(rig.fd.log[mark], Ev::Fire, clipKey);
        checkEvent(rig.fd.log[mark + 1], Ev::Touch, op1);
        checkEvent(rig.fd.log[mark + 2], Ev::Set, op1);
        CHECK(rig.fd.log[mark + 2].v == Approx(0.3f));
        checkEvent(rig.fd.log[mark + 3], Ev::Release, op1);
        const size_t after = rig.fd.log.size();
        rig.runTo(1.25);
        CHECK(rig.fd.on(op1, after).empty());
        CHECK(rig.fd.reads == 0);
    }

    SECTION("guard: the same routine on Ease (the default) still glides from where the knob is (G1)")
    {
        Rig rig;
        addToBank(rig.comp, styled(Clip::BeatSnapMode::Bar, Routine::RestoreStyle::Ease), 0);
        rig.fd.values[op1] = 0.9f;
        rig.tick();
        rig.runTo(1.0);
        CHECK(rig.fire(0).empty());
        CHECK(rig.slot(0).restoreStyle == "ease");
        rig.runTo(2.9375);
        CHECK(rig.fd.count(Ev::Touch, op1) == 0);
        rig.runTo(3.0);
        CHECK(rig.fd.count(Ev::Touch, op1) == 1);
        CHECK(rig.fd.lastSet(op1) == Approx(0.9f));
        CHECK(rig.fd.reads >= 1);
    }
}

TEST_CASE("RoutineEngine restore style J2: a Jump loop snaps back ON the loop point -- no return glide, no spill", "[routine][engine][restorestyle]")
{
    Rig rig;
    const ControlPath op0 = opacityKey(0);
    const ControlPath op1 = opacityKey(1);
    Routine r = makeRoutine("L", 4.0, Clip::BeatSnapMode::Bar, true);     // G5's loop, on Jump
    r.restoreStyle = Routine::RestoreStyle::Jump;
    Routine::PreambleEntry p1; p1.key = op1; p1.continuous = true; p1.norm = 0.4f;
    Routine::PreambleEntry p0; p0.key = op0; p0.continuous = true; p0.norm = 0.1f;
    r.preamble = { p1, p0 };
    r.lanes[op0] = continuousLane(op0, { gesture(0.5, 0.1f, 4.0, 0.9f) });   // held to the very end
    addToBank(rig.comp, r, 0);
    rig.fd.values[op1] = 0.9f;
    rig.fd.values[op0] = 0.0f;
    rig.tick();
    rig.runTo(1.0);
    CHECK(rig.fire(0).empty());
    rig.runTo(3.9375);
    CHECK(rig.fd.log.empty());
    rig.runTo(4.0);                                   // the start: both hard-set on the bar
    CHECK(rig.slot(0).state == "running");
    CHECK(rig.fd.count(Ev::Touch, op1) == 1);
    CHECK(rig.fd.lastSet(op1) == Approx(0.4f));
    CHECK(rig.fd.count(Ev::Release, op1) == 1);
    rig.runTo(6.0);
    rig.fd.values[op1] = 0.9f;                        // a hand moved op1 while nobody held it

    rig.runTo(7.9375);
    CHECK(rig.fd.count(Ev::Touch, op1) == 1);         // Ease begins op1's return glide at 7.0 (G5)
    CHECK(rig.slot(0).glides == 0);
    const float gestureLast = rig.fd.lastSet(op0);    // where the recording's own hand left op0
    CHECK(gestureLast > 0.5f);
    const size_t at8 = rig.fd.log.size();
    rig.runTo(8.0);                                   // the loop point: the restore in one call, as before the glide
    {
        const auto ev1 = rig.fd.on(op1, at8);
        REQUIRE(ev1.size() == 3);
        checkEvent(ev1[0], Ev::Touch, op1);
        checkEvent(ev1[1], Ev::Set, op1);
        CHECK(ev1[1].v == Approx(0.4f));
        checkEvent(ev1[2], Ev::Release, op1);
        const auto ev0 = rig.fd.on(op0, at8);         // held to the end: its end lands and lets go, then set straight to 0.1
        REQUIRE(ev0.size() == 5);                     // s-rta-0928: Set(0.9) (the gesture's end), Release, then the Jump restore
        checkEvent(ev0[0], Ev::Set, op0);
        CHECK(ev0[0].v == Approx(0.9f));
        checkEvent(ev0[1], Ev::Release, op0);
        checkEvent(ev0[2], Ev::Touch, op0);
        checkEvent(ev0[3], Ev::Set, op0);
        CHECK(ev0[3].v == Approx(0.1f));
        checkEvent(ev0[4], Ev::Release, op0);
        const auto s = rig.slot(0);
        CHECK(s.cycle == 2);
        CHECK(s.preambleFired == 4);
        CHECK(s.glides == 0);
    }
    const size_t after = rig.fd.log.size();
    rig.runTo(8.4375);
    CHECK(rig.fd.on(op0, after).empty());             // nothing until cycle 2's gesture
    CHECK(rig.fd.on(op1, after).empty());
    rig.runTo(8.5);
    CHECK(rig.fd.count(Ev::Touch, op0) == 4);         // start restore, cycle 1 gesture, loop restore, cycle 2 gesture
    CHECK(rig.fd.reads == 0);
}

TEST_CASE("RoutineEngine restore style J3: a Jump re-fire restarts with a one-call restore ON its bar", "[routine][engine][restorestyle]")
{
    const ControlPath op1 = opacityKey(1);
    Routine r = makeRoutine("r", 16.0, Clip::BeatSnapMode::Bar, false);   // test 9's routine (G12), on Jump
    r.restoreStyle = Routine::RestoreStyle::Jump;
    Routine::PreambleEntry restoreOp; restoreOp.key = op1; restoreOp.continuous = true; restoreOp.norm = 0.5f;
    r.preamble = { restoreOp };

    Rig rig;
    addToBank(rig.comp, r, 0);
    rig.fd.values[op1] = 0.9f;
    rig.tick();
    rig.runTo(1.0);
    CHECK(rig.fire(0).empty());
    rig.runTo(3.9375);
    CHECK(rig.fd.count(Ev::Touch, op1) == 0);
    rig.runTo(4.0);
    CHECK(rig.slot(0).state == "running");
    CHECK(rig.fd.count(Ev::Touch, op1) == 1);
    CHECK(rig.fd.count(Ev::Release, op1) == 1);        // the start: touch / set 0.5 / release on the bar
    rig.runTo(5.0);
    CHECK(rig.fire(0).empty());                        // restart requested
    rig.runTo(5.5);
    rig.fd.values[op1] = 0.9f;                         // a hand moved the knob while nobody held it
    rig.runTo(7.9375);
    CHECK(rig.slot(0).restarts == 0);
    CHECK(rig.fd.count(Ev::Touch, op1) == 1);          // Ease begins the restart's glide at 7.0 (G12)
    CHECK(rig.slot(0).glides == 0);
    const size_t at8 = rig.fd.log.size();
    rig.runTo(8.0);                                    // the next bar: restart, restore in one call
    CHECK(rig.slot(0).restarts == 1);
    CHECK(rig.slot(0).position == Approx(0.0));
    {
        const auto ev = rig.fd.on(op1, at8);
        REQUIRE(ev.size() == 3);
        checkEvent(ev[0], Ev::Touch, op1);
        checkEvent(ev[1], Ev::Set, op1);
        CHECK(ev[1].v == Approx(0.5f));
        checkEvent(ev[2], Ev::Release, op1);
    }
    CHECK(rig.slot(0).glides == 0);
    CHECK(rig.fd.reads == 0);
}

TEST_CASE("RoutineEngine restore style J4: a running loop picks up a style change at its next loop return", "[routine][engine][restorestyle]")
{
    Rig rig;
    const ControlPath clipKey = layerKey(0, "activeClip");
    const ControlPath op0 = opacityKey(0);
    const ControlPath op1 = opacityKey(1);
    Routine r = makeRoutine("loop", 4.0, Clip::BeatSnapMode::Bar, true);   // test 8's loop routine, Ease
    Routine::PreambleEntry restoreOp; restoreOp.key = op1; restoreOp.continuous = true; restoreOp.norm = 0.4f;
    r.preamble = { restoreOp };
    r.lanes[clipKey] = discreteLane(clipKey, { point(1, 1.0, 3) });
    r.lanes[op0] = continuousLane(op0, { gesture(0.5, 0.1f, 3.5, 0.9f) });
    addToBank(rig.comp, r, 0);
    auto setStyle = [&rig](Routine::RestoreStyle style) {
        for (auto& routine : rig.comp.routines)       // what POST /api/routine/set writes
            if (routine.uuid == "loop")
                routine.restoreStyle = style;
    };
    rig.fd.values[op1] = 0.9f;
    rig.tick();
    rig.runTo(1.0);
    CHECK(rig.fire(0).empty());
    rig.runTo(4.0);                                    // the start glided in over [3.0, 4.0]
    CHECK(rig.slot(0).state == "running");
    CHECK(rig.fd.count(Ev::Touch, op1) == 1);
    CHECK(rig.fd.count(Ev::Release, op1) == 1);

    rig.runTo(5.0);
    setStyle(Routine::RestoreStyle::Jump);             // switched to Jump mid-cycle
    rig.runTo(5.0625);
    CHECK(rig.slot(0).restoreStyle == "jump");
    rig.fd.values[op1] = 0.9f;
    rig.runTo(7.9375);
    CHECK(rig.fd.count(Ev::Touch, op1) == 1);          // no return glide over [7.0, 8.0]
    const size_t at8 = rig.fd.log.size();
    rig.runTo(8.0);                                    // cycle 2: the return jumps
    {
        const auto ev = rig.fd.on(op1, at8);
        REQUIRE(ev.size() == 3);
        checkEvent(ev[0], Ev::Touch, op1);
        checkEvent(ev[1], Ev::Set, op1);
        CHECK(ev[1].v == Approx(0.4f));
        checkEvent(ev[2], Ev::Release, op1);
    }
    CHECK(rig.slot(0).cycle == 2);

    rig.runTo(9.0);
    setStyle(Routine::RestoreStyle::Ease);             // back to Ease: the next return glides again
    rig.fd.values[op1] = 0.9f;
    rig.runTo(10.9375);
    CHECK(rig.fd.count(Ev::Touch, op1) == 2);
    rig.runTo(11.0);                                   // cycle 2's last beat: the return glide [11.0, 12.0]
    CHECK(rig.fd.count(Ev::Touch, op1) == 3);
    CHECK(rig.fd.lastSet(op1) == Approx(0.9f));
    rig.runTo(11.5);
    CHECK(rig.fd.lastSet(op1) == Approx(0.65f));
    rig.runTo(12.0);
    CHECK(rig.fd.lastSet(op1) == Approx(0.4f));
    CHECK(rig.fd.count(Ev::Release, op1) == 3);
    CHECK(rig.slot(0).cycle == 3);
}

// === s-rta-0926b routines-followup ITEM 3a: the 2 Bar / 4 Bar boundary prediction ===
// RoutineEngine::beatsUntilBoundary (private) predicts, from the last snapshot, how many beats remain until
// the boundary a routine starts on; the restore glide is scheduled from it ([boundary - 1, boundary]). It is
// pinned here through what it decides -- WHEN the glide begins: a wrong 2 Bar / 4 Bar prediction starts the
// glide a bar (or more) early and holds it, or turns it into a quarter-beat spill after the start. The PARITY
// is barCount's (the counter dueNow consults), never totalBarCount's: totalBase 11 makes the two disagree.

TEST_CASE("RoutineEngine glide: the 2 Bar and 4 Bar boundary prediction puts the glide in the last beat before the start", "[routine][engine][glide][quantize]")
{
    const ControlPath op1 = opacityKey(1);
    auto check = [&op1](Clip::BeatSnapMode quantize, uint32_t totalBase, int barBase, double fireAt, double start) {
        Rig rig;
        rig.totalBase = totalBase;
        rig.barBase = barBase;
        addToBank(rig.comp, test7Routine(quantize), 0);
        rig.fd.values[op1] = 0.9f;
        rig.tick();
        rig.runTo(fireAt);
        CHECK(rig.fire(0).empty());
        rig.runTo(start - 1.0 - Rig::kStep);
        CHECK(rig.fd.count(Ev::Touch, op1) == 0);     // nothing moves before the last beat
        rig.runTo(start - 1.0);
        CHECK(rig.fd.count(Ev::Touch, op1) == 1);     // the glide begins exactly one beat before the start
        CHECK(rig.fd.lastSet(op1) == Approx(0.9f));
        rig.runTo(start - 0.5);
        CHECK(rig.fd.lastSet(op1) == Approx(0.6f));   // half way at half the beat
        rig.runTo(start - Rig::kStep);
        CHECK(rig.slot(0).state == "pending");
        rig.runTo(start);
        CHECK(rig.slot(0).state == "running");        // dueNow agrees with the prediction
        CHECK(rig.fd.lastSet(op1) == Approx(0.3f));   // landed ON the start
        CHECK(rig.fd.count(Ev::Release, op1) == 1);
    };

    SECTION("2 Bar, fired in barCount 0 (totalBarCount 11): the start is barCount 2 at beat 8, not the next bar")
    {
        check(Clip::BeatSnapMode::TwoBar, 11, 0, 1.0, 8.0);
    }
    SECTION("2 Bar, fired mid-beat in barCount 1: only the rest of this bar")
    {
        check(Clip::BeatSnapMode::TwoBar, 11, 0, 5.5, 8.0);
    }
    SECTION("4 Bar, fired in barCount 0 (totalBarCount 12 at beat 4 is a multiple of 4): three more bars, beat 16")
    {
        check(Clip::BeatSnapMode::FourBar, 11, 0, 1.0, 16.0);
    }
    SECTION("4 Bar, fired in barCount 3: only the rest of this bar")
    {
        check(Clip::BeatSnapMode::FourBar, 11, 0, 13.0, 16.0);
    }
    SECTION("4 Bar, barCount already 2 at the fire (a phrase reset left it mid-count): beat 8")
    {
        check(Clip::BeatSnapMode::FourBar, 10, 2, 1.0, 8.0);
    }
}

// === s-rta-0927 routine display (plan-routine-display-A.md 3.1-3.2, 5.2): the status says WHERE a pending /
// running routine plays (deck, resolved layers, fire order, the grid it starts on, a pending restart), the
// layer X stops every routine on that layer, and a finished run's warning stays on its idle pad. ===

TEST_CASE("RoutineEngine display D1: a fired routine reports its deck, resolved layers, fire order and start grid", "[routine][engine][display]")
{
    Rig rig;
    addToBank(rig.comp, test7Routine(), 0);   // restores layer 0's clip and layer 1's opacity; a lane on layer 0
    rig.tick();
    rig.runTo(1.0);
    {
        const auto s = rig.slot(0);
        CHECK(s.state == "idle");
        CHECK(s.deck == -1);
        CHECK(s.layers.empty());
        CHECK(s.fireSeq == 0);
        CHECK(s.startsOn.empty());
    }
    CHECK(rig.fire(0).empty());
    {
        const auto s = rig.slot(0);
        CHECK(s.state == "pending");
        CHECK(s.deck == 0);
        CHECK(s.layers == std::vector<int>{ 0, 1 });
        CHECK_FALSE(s.touchesComp);
        CHECK(s.fireSeq == 1);
        CHECK(s.startsOn == "bar");
        CHECK_FALSE(s.restartPending);
    }
    rig.runTo(4.0);
    {
        const auto s = rig.slot(0);
        CHECK(s.state == "running");
        CHECK(s.deck == 0);
        CHECK(s.layers == std::vector<int>{ 0, 1 });
        CHECK(s.startsOn.empty());   // only a waiting routine has a grid still to start on
    }

    SECTION("the global Quantize override is the grid it starts on")
    {
        Rig q;
        addToBank(q.comp, test7Routine(), 0);
        q.forced = RoutineSnap::FourBar;
        q.tick();
        q.runTo(1.0);
        CHECK(q.fire(0).empty());
        CHECK(q.slot(0).startsOn == "4bar");
    }
}

TEST_CASE("RoutineEngine display D2: a deck-relative routine resolves on the deck that is active at the fire", "[routine][engine][display]")
{
    Rig rig;
    Deck second;
    second.name = "Deck 2";
    second.initDefault();
    const int idx = rig.comp.appendDeck(std::move(second));
    REQUIRE(idx == 1);
    rig.comp.activeDeckIndex = 1;
    addToBank(rig.comp, test7Routine(), 0);
    rig.tick();
    rig.runTo(1.0);
    CHECK(rig.fire(0).empty());
    CHECK(rig.slot(0).deck == 1);
    CHECK(rig.slot(0).layers == std::vector<int>{ 0, 1 });
    rig.comp.activeDeckIndex = 0;   // a deck switch after the fire does not move it
    rig.runTo(5.0);
    CHECK(rig.slot(0).state == "running");
    CHECK(rig.slot(0).deck == 1);
}

// Lane bf9b (plan-bf9b S2.9 / F9): stopOnLayer(layer) names a SHARED layer -- it stops every routine touching it,
// whatever deck the routine fired from (the old "another deck: nothing" step has no deck to name any more).
TEST_CASE("RoutineEngine display D3: stopOnLayer stops every routine on that shared layer, whole, grips released", "[routine][engine][display]")
{
    Rig rig;
    const ControlPath op0 = opacityKey(0);
    const ControlPath speed0 = layerKey(0, "scalar", "speed");
    addToBank(rig.comp, test7Routine(), 0);                                  // layers {0, 1}
    Routine b = makeRoutine("b", 8.0, Clip::BeatSnapMode::Bar, false);       // layer {0} only
    b.lanes[speed0] = continuousLane(speed0, { gesture(0.0, 0.1f, 4.0, 0.9f) });
    addToBank(rig.comp, b, 1);

    rig.tick();
    rig.runTo(1.0);
    CHECK(rig.fire(0).empty());
    CHECK(rig.fire(1).empty());
    CHECK(rig.slot(1).layers == std::vector<int>{ 0 });
    CHECK(rig.slot(1).fireSeq > rig.slot(0).fireSeq);
    rig.runTo(6.5);   // both running; A's gesture on layer 0 opacity (clock 6..7) and B's on speed (4..8) held
    REQUIRE(rig.slot(0).state == "running");
    REQUIRE(rig.slot(1).state == "running");
    CHECK(rig.fd.count(Ev::Release, op0) == 0);
    CHECK(rig.fd.count(Ev::Release, speed0) == 0);

    rig.eng.stopOnLayer(1);   // only A plays on layer 1
    CHECK(rig.slot(0).state == "idle");
    CHECK(rig.slot(0).layers.empty());
    CHECK(rig.slot(1).state == "running");
    CHECK(rig.fd.count(Ev::Release, op0) == 1);   // A's gesture on layer 0 let go too: the WHOLE routine stops

    CHECK(rig.slot(1).state == "running");
    CHECK(rig.fd.count(Ev::Release, speed0) == 0);

    rig.eng.stopOnLayer(0);
    CHECK(rig.slot(1).state == "idle");
    CHECK(rig.fd.count(Ev::Release, speed0) == 1);

    SECTION("both routines on layer 0 stop together")
    {
        Rig r2;
        addToBank(r2.comp, test7Routine(), 0);
        addToBank(r2.comp, b, 1);
        r2.tick();
        r2.runTo(1.0);
        CHECK(r2.fire(0).empty());
        CHECK(r2.fire(1).empty());
        r2.eng.stopOnLayer(0);   // while still pending
        CHECK(r2.slot(0).state == "idle");
        CHECK(r2.slot(1).state == "idle");
    }
}

// Fix stage (ruling-bf9b-merge AM-10, 4.B row 1): the positive half of the removed "another deck: nothing" step --
// plan-bf9b :332-333 "`RoutineEngine::stopOnLayer(int layer)` (was (deck, layer)): stops every running routine
// touching that shared layer, whatever deck it fired from."
TEST_CASE("RoutineEngine display D3b: stopOnLayer stops a routine that was fired with another deck shown", "[routine][engine][display]")
{
    Rig rig;
    Deck second;
    second.name = "Deck 2";
    second.initDefault();
    REQUIRE(rig.comp.appendDeck(std::move(second)) == 1);
    rig.comp.activeDeckIndex = 1;                                            // deck 1 is shown at the fire
    addToBank(rig.comp, test7Routine(), 0);                                  // layers {0, 1}
    rig.tick();
    rig.runTo(1.0);
    CHECK(rig.fire(0).empty());
    REQUIRE(rig.slot(0).deck == 1);
    rig.comp.activeDeckIndex = 0;                                            // Boris looks at deck 0 now
    rig.runTo(6.5);
    REQUIRE(rig.slot(0).state == "running");
    REQUIRE(rig.slot(0).deck == 1);
    CHECK(rig.fd.count(Ev::Release, opacityKey(0)) == 0);

    rig.eng.stopOnLayer(1);                                                  // the layer X, pressed with deck 0 shown
    CHECK(rig.slot(0).state == "idle");
    CHECK(rig.slot(0).layers.empty());
    CHECK(rig.fd.count(Ev::Release, opacityKey(0)) == 1);                    // whole, its grip on layer 0 released
}

TEST_CASE("RoutineEngine display D4: a finished run's warning stays on its idle pad until stopAll", "[routine][engine][display]")
{
    Rig rig;
    Routine w = makeRoutine("w", 4.0, Clip::BeatSnapMode::Bar, false);
    Routine::PreambleEntry gone; gone.key = opacityKey(5); gone.continuous = true; gone.norm = 0.5f;   // no layer 6
    w.preamble = { gone };
    addToBank(rig.comp, w, 2);
    rig.tick();
    rig.runTo(1.0);
    CHECK(rig.fire(2).empty());
    rig.runTo(5.0);
    CHECK(rig.slot(2).state == "running");
    CHECK(rig.slot(2).preambleUnresolved == 1);
    rig.runTo(9.0);   // 4 beats from beat 4: once, ended
    CHECK(rig.slot(2).state == "idle");
    CHECK(rig.slot(2).preambleUnresolved == 1);
    CHECK(rig.slot(2).layers.empty());

    SECTION("a stop keeps it too; stopAll clears it")
    {
        CHECK(rig.fire(2).empty());
        rig.eng.stop(2);
        CHECK(rig.slot(2).state == "idle");
        CHECK(rig.slot(2).preambleUnresolved == 1);
        rig.eng.stopAll();
        CHECK(rig.slot(2).preambleUnresolved == 0);
    }
}

TEST_CASE("RoutineEngine display D5: restartPending is set by a re-fire while running and cleared when the restart lands", "[routine][engine][display]")
{
    Rig rig;
    addToBank(rig.comp, test7Routine(), 0);
    rig.tick();
    rig.runTo(1.0);
    CHECK(rig.fire(0).empty());
    rig.runTo(4.5);
    REQUIRE(rig.slot(0).state == "running");
    CHECK_FALSE(rig.slot(0).restartPending);
    CHECK(rig.slot(0).startsOn.empty());
    CHECK(rig.fire(0).empty());
    CHECK(rig.slot(0).restartPending);
    CHECK(rig.slot(0).startsOn == "bar");   // s-rta-0927 fix round: the grid the restart lands on (the pad's cue)
    CHECK(rig.slot(0).fireSeq == 1);   // a restart is not a new fire
    rig.runTo(8.0);
    CHECK(rig.slot(0).restarts == 1);
    CHECK_FALSE(rig.slot(0).restartPending);
    CHECK(rig.slot(0).startsOn.empty());
}

// === s-rta-0927 fix round: a pad-menu (or REST) settings edit made while a routine WAITS reaches that very
// start -- Quantize, Start: Ease / Jump and Restore first / Start from now are re-read from the live routine on
// every tick of the wait, not frozen at the press (critic MUST: the menu re-ticked, the start did not). ===

TEST_CASE("RoutineEngine display D6: a settings edit made while waiting reaches the pending start", "[routine][engine][display][pending]")
{
    const ControlPath clipKey = layerKey(0, "activeClip");
    const ControlPath op1 = opacityKey(1);

    SECTION("Quantize Bar -> 4 Bar while waiting: startsOn follows at once and the bar edge no longer starts it")
    {
        Rig rig;
        addToBank(rig.comp, test7Routine(), 0);   // Bar
        rig.tick();
        rig.runTo(1.0);
        CHECK(rig.fire(0).empty());
        CHECK(rig.slot(0).startsOn == "bar");
        rig.comp.routines[0].quantize = Clip::BeatSnapMode::FourBar;   // what perfRoutineSet writes
        rig.runTo(1.0625);
        CHECK(rig.slot(0).startsOn == "4bar");                          // the tooltip agrees with the menu tick
        CHECK(rig.slot(0).quantize == "4bar");
        rig.runTo(4.0);                                                 // barCount 1: not a four-bar line
        CHECK(rig.slot(0).state == "pending");
        rig.runTo(16.0);                                                // barCount 4
        CHECK(rig.slot(0).state == "running");
    }

    SECTION("Ease -> Jump before the glide began: nothing moves in the wait, one call ON the bar, no read")
    {
        Rig rig;
        addToBank(rig.comp, test7Routine(), 0);   // Ease (the default)
        rig.fd.values[op1] = 0.9f;
        rig.tick();
        rig.runTo(1.0);
        CHECK(rig.fire(0).empty());
        rig.runTo(2.0);
        rig.comp.routines[0].restoreStyle = Routine::RestoreStyle::Jump;
        rig.runTo(3.9375);
        CHECK(rig.slot(0).state == "pending");
        CHECK(rig.slot(0).restoreStyle == "jump");
        CHECK(rig.fd.count(Ev::Touch, op1) == 0);                       // Ease would have touched op1 at 3.0
        CHECK(rig.slot(0).glides == 0);
        const size_t at4 = rig.fd.log.size();
        rig.runTo(4.0);
        CHECK(rig.slot(0).state == "running");
        REQUIRE(rig.fd.log.size() == at4 + 4);
        checkEvent(rig.fd.log[at4], Ev::Fire, clipKey);
        checkEvent(rig.fd.log[at4 + 1], Ev::Touch, op1);
        checkEvent(rig.fd.log[at4 + 2], Ev::Set, op1);
        CHECK(rig.fd.log[at4 + 2].v == Approx(0.3f));                   // at once, not a step of a glide from 0.9
        checkEvent(rig.fd.log[at4 + 3], Ev::Release, op1);
        CHECK(rig.fd.reads == 0);
    }

    SECTION("Ease -> Jump inside the glide: the glide lets go where it is, the restore lands in one call ON the bar")
    {
        Rig rig;
        addToBank(rig.comp, test7Routine(), 0);
        rig.fd.values[op1] = 0.9f;
        rig.tick();
        rig.runTo(1.0);
        CHECK(rig.fire(0).empty());
        rig.runTo(3.5);                                                 // gliding 0.9 -> 0.3 over [3, 4]
        REQUIRE(rig.fd.count(Ev::Touch, op1) == 1);
        CHECK(rig.fd.count(Ev::Release, op1) == 0);
        rig.comp.routines[0].restoreStyle = Routine::RestoreStyle::Jump;
        rig.runTo(3.5625);
        CHECK(rig.fd.count(Ev::Release, op1) == 1);                     // let go, never a leaked grip
        CHECK(rig.slot(0).glides == 0);
        const size_t mark = rig.fd.log.size();
        rig.runTo(3.9375);
        CHECK(rig.fd.on(op1, mark).empty());                            // no more glide steps
        rig.runTo(4.0);
        CHECK(rig.fd.count(Ev::Touch, op1) == 2);
        CHECK(rig.fd.lastSet(op1) == Approx(0.3f));
        CHECK(rig.fd.count(Ev::Release, op1) == 2);
    }

    SECTION("Jump -> Ease while waiting: the restore now glides over the last beat before the bar")
    {
        Rig rig;
        Routine r = test7Routine();
        r.restoreStyle = Routine::RestoreStyle::Jump;
        addToBank(rig.comp, r, 0);
        rig.fd.values[op1] = 0.9f;
        rig.tick();
        rig.runTo(1.0);
        CHECK(rig.fire(0).empty());
        rig.runTo(2.0);
        rig.comp.routines[0].restoreStyle = Routine::RestoreStyle::Ease;
        rig.runTo(3.0);
        CHECK(rig.fd.count(Ev::Touch, op1) == 1);                       // Jump would touch nothing before the bar
        CHECK(rig.fd.lastSet(op1) == Approx(0.9f));                     // from where the knob is
        rig.runTo(3.5);
        CHECK(rig.fd.lastSet(op1) == Approx(0.6f));
        rig.runTo(4.0);
        CHECK(rig.slot(0).state == "running");
        CHECK(rig.fd.lastSet(op1) == Approx(0.3f));
        CHECK(rig.fd.count(Ev::Release, op1) == 1);
    }

    SECTION("Restore first -> Start from now while waiting: nothing is restored at the start")
    {
        Rig rig;
        addToBank(rig.comp, test7Routine(), 0);
        rig.fd.values[op1] = 0.9f;
        rig.tick();
        rig.runTo(1.0);
        CHECK(rig.fire(0).empty());
        rig.runTo(2.0);
        rig.comp.routines[0].restoreState = false;
        rig.runTo(4.0);
        CHECK(rig.slot(0).state == "running");
        CHECK(rig.fd.firedRestores() == 0);
        CHECK(rig.fd.count(Ev::Touch, op1) == 0);
        CHECK_FALSE(rig.slot(0).restoreState);
    }
}

// === s-rta-0927 fix round 2: a RESTART waiting for its boundary (a pad pressed again while it plays) follows the
// same edits a waiting start does -- Quantize, Start: Ease / Jump and Restore first / Start from now are re-read from
// the live routine on every tick of the restart wait, not frozen at the second press (critic MUST: the menu, the
// notice and the restart tooltip said one thing; the restart landed on the settings of the press). ===

TEST_CASE("RoutineEngine display D7: a settings edit made while a restart waits reaches that restart", "[routine][engine][display][pending]")
{
    const ControlPath op1 = opacityKey(1);

    // test7Routine started on the bar at 4.0 (its Ease restore glided op1 0.9 -> 0.3 over [3, 4]), then pressed
    // again at 4.5: a restart waits for the bar at 8.0, its restore glide scheduled over [7, 8].
    auto startThenRefire = [](Rig& rig) {
        rig.tick();
        rig.runTo(1.0);
        CHECK(rig.fire(0).empty());
        rig.runTo(4.5);
        REQUIRE(rig.slot(0).state == "running");
        CHECK(rig.fire(0).empty());
        REQUIRE(rig.slot(0).restartPending);
        CHECK(rig.slot(0).startsOn == "bar");
    };

    SECTION("Quantize Bar -> Beat while the restart waits: startsOn follows at once and the restart lands on the next beat")
    {
        Rig rig;
        addToBank(rig.comp, test7Routine(), 0);
        startThenRefire(rig);
        rig.comp.routines[0].quantize = Clip::BeatSnapMode::Beat;      // what perfRoutineSet writes
        rig.runTo(4.5625);
        CHECK(rig.slot(0).startsOn == "beat");                          // the restart tooltip agrees with the menu tick
        CHECK(rig.slot(0).restarts == 0);
        rig.runTo(5.0);                                                 // the next beat, not the bar at 8.0
        CHECK(rig.slot(0).restarts == 1);
        CHECK_FALSE(rig.slot(0).restartPending);
        CHECK(rig.slot(0).position == Approx(0.0));
        CHECK(rig.fd.lastSet(op1) == Approx(0.3f));                     // its restore landed with it
    }

    SECTION("Quantize Bar -> 4 Bar while a loop's restart waits: the next bar no longer restarts it, the four-bar line does")
    {
        Rig rig;
        Routine r = test7Routine();
        r.loop = true;
        addToBank(rig.comp, r, 0);
        startThenRefire(rig);
        rig.runTo(5.0);
        rig.comp.routines[0].quantize = Clip::BeatSnapMode::FourBar;
        rig.runTo(5.0625);
        CHECK(rig.slot(0).startsOn == "4bar");
        rig.runTo(8.0);                                                 // barCount 2: not a four-bar line
        CHECK(rig.slot(0).restarts == 0);
        CHECK(rig.slot(0).restartPending);
        rig.runTo(15.9375);
        CHECK(rig.slot(0).restarts == 0);
        rig.runTo(16.0);                                                // barCount 4
        CHECK(rig.slot(0).restarts == 1);
        CHECK_FALSE(rig.slot(0).restartPending);
        CHECK(rig.slot(0).position == Approx(0.0));
    }

    SECTION("Ease -> Jump before the restart's glide began: nothing moves before the bar, one call ON the bar, no read")
    {
        Rig rig;
        addToBank(rig.comp, test7Routine(), 0);   // Ease (the default)
        rig.fd.values[op1] = 0.9f;
        startThenRefire(rig);
        const int reads = rig.fd.reads;
        rig.comp.routines[0].restoreStyle = Routine::RestoreStyle::Jump;
        rig.fd.values[op1] = 0.9f;                                      // a hand moved the knob since the start
        rig.runTo(7.9375);
        CHECK(rig.slot(0).restarts == 0);
        CHECK(rig.fd.count(Ev::Touch, op1) == 1);                       // the start's glide only; Ease would touch at 7.0
        CHECK(rig.slot(0).glides == 0);
        const size_t at8 = rig.fd.log.size();
        rig.runTo(8.0);
        CHECK(rig.slot(0).restarts == 1);
        {
            const auto ev = rig.fd.on(op1, at8);
            REQUIRE(ev.size() == 3);
            checkEvent(ev[0], Ev::Touch, op1);
            checkEvent(ev[1], Ev::Set, op1);
            CHECK(ev[1].v == Approx(0.3f));                             // at once, not a step of a glide from 0.9
            checkEvent(ev[2], Ev::Release, op1);
        }
        CHECK(rig.fd.reads == reads);
    }

    SECTION("Ease -> Jump inside the restart's glide: the glide lets go where it is, the restore lands in one call ON the bar")
    {
        Rig rig;
        addToBank(rig.comp, test7Routine(), 0);
        rig.fd.values[op1] = 0.9f;
        startThenRefire(rig);
        rig.runTo(5.0);
        rig.fd.values[op1] = 0.9f;
        rig.runTo(7.5);                                                 // the restart's glide 0.9 -> 0.3 over [7, 8]
        REQUIRE(rig.fd.count(Ev::Touch, op1) == 2);
        CHECK(rig.fd.count(Ev::Release, op1) == 1);
        rig.comp.routines[0].restoreStyle = Routine::RestoreStyle::Jump;
        rig.runTo(7.5625);
        CHECK(rig.fd.count(Ev::Release, op1) == 2);                     // let go, never a leaked grip
        CHECK(rig.slot(0).glides == 0);
        const size_t mark = rig.fd.log.size();
        rig.runTo(7.9375);
        CHECK(rig.fd.on(op1, mark).empty());                            // no more glide steps
        rig.runTo(8.0);
        CHECK(rig.slot(0).restarts == 1);
        CHECK(rig.fd.count(Ev::Touch, op1) == 3);
        CHECK(rig.fd.lastSet(op1) == Approx(0.3f));
        CHECK(rig.fd.count(Ev::Release, op1) == 3);
    }

    SECTION("Jump -> Ease while the restart waits: the restart's restore now glides over the last beat before the bar")
    {
        Rig rig;
        Routine r = test7Routine();
        r.restoreStyle = Routine::RestoreStyle::Jump;
        addToBank(rig.comp, r, 0);
        rig.fd.values[op1] = 0.9f;
        startThenRefire(rig);                                           // the start: one call ON the bar at 4.0
        REQUIRE(rig.fd.count(Ev::Touch, op1) == 1);
        rig.runTo(5.0);
        rig.comp.routines[0].restoreStyle = Routine::RestoreStyle::Ease;
        rig.fd.values[op1] = 0.9f;
        rig.runTo(7.0);
        CHECK(rig.fd.count(Ev::Touch, op1) == 2);                       // Jump would touch nothing before the bar
        CHECK(rig.fd.lastSet(op1) == Approx(0.9f));                     // from where the knob is
        rig.runTo(7.5);
        CHECK(rig.fd.lastSet(op1) == Approx(0.6f));
        rig.runTo(8.0);
        CHECK(rig.slot(0).restarts == 1);
        CHECK(rig.fd.lastSet(op1) == Approx(0.3f));
        CHECK(rig.fd.count(Ev::Release, op1) == 2);
    }

    SECTION("Restore first -> Start from now while the restart waits: nothing is restored at the restart")
    {
        Rig rig;
        addToBank(rig.comp, test7Routine(), 0);
        rig.fd.values[op1] = 0.9f;
        startThenRefire(rig);
        const int restores = rig.fd.firedRestores();
        rig.runTo(5.0);
        rig.comp.routines[0].restoreState = false;
        rig.runTo(8.0);
        CHECK(rig.slot(0).restarts == 1);
        CHECK(rig.fd.firedRestores() == restores);
        CHECK(rig.fd.count(Ev::Touch, op1) == 1);                       // the start's glide only
        CHECK(rig.slot(0).glides == 0);
    }
}

// === s-rta-0927 beat clock: a tick gap swallows no beat and no Beat edge (Pitfall 42) ===
//
// The 120 Hz message-thread tick can stall (a deck load held it 0.53 s in s-rta-0927's loadpost1 sample). The routine
// clock (RecorderClock) and the Beat edge used to read the beatPhase sawtooth's wraps, so a gap carried only its
// fractional part: every later event of a running routine landed a beat late for good, and a Beat-quantized start
// missed any edge inside the gap. Both now read the tracker's counter (totalBeatCount).

TEST_CASE("RoutineEngine: position is exact across a 1.1-beat tick gap and the point inside it fires once",
          "[routine][engine][stall]")
{
    Rig rig;
    const ControlPath clipKey = layerKey(0, "activeClip");
    addToBank(rig.comp, test7Routine(Clip::BeatSnapMode::Off), 0);
    rig.tick();
    rig.runTo(4.0);
    CHECK(rig.fire(0).empty());                 // Off: starts now, startBeat 4.0
    CHECK(rig.slot(0).state == "running");
    rig.runTo(4.5);
    CHECK(rig.fd.firedLanePoints(clipKey, 1) == 0);
    rig.beat = 5.6;                             // the message thread slept 0.55 s
    rig.tick();
    CHECK(rig.slot(0).position == Approx(1.6));    // the old clock: 0.6
    CHECK(rig.fd.firedLanePoints(clipKey, 1) == 1); // the point at routine beat 1.0
}

TEST_CASE("RoutineEngine: a Beat-quantized start survives a tick gap that swallows the beat edge", "[routine][engine][stall]")
{
    Rig rig;
    addToBank(rig.comp, test7Routine(Clip::BeatSnapMode::Beat), 0);
    rig.tick();
    rig.runTo(4.25);
    CHECK(rig.fire(0).empty());
    CHECK(rig.slot(0).state == "pending");
    rig.beat = 4.5;
    rig.tick();
    CHECK(rig.slot(0).state == "pending");      // same beat (count 4 == 4)
    rig.beat = 5.2;                             // a gap across the beat line at 5.0
    rig.tick();
    CHECK(rig.slot(0).state == "running");      // the old wrap test: 0.2 < 0.5 - 0.5 is false, no edge
}

TEST_CASE("RoutineEngine: several points inside one tick gap all fire, once each, in order, in the resume tick",
          "[routine][engine][stall]")
{
    Rig rig;
    const ControlPath clipKey = layerKey(0, "activeClip");
    Routine r = test7Routine(Clip::BeatSnapMode::Off);
    r.lanes[clipKey] = discreteLane(clipKey, { point(1, 1.0, 1), point(2, 1.25, 2), point(3, 1.5, 3) });
    addToBank(rig.comp, r, 0);
    rig.tick();
    rig.runTo(4.0);
    CHECK(rig.fire(0).empty());
    rig.runTo(4.5);
    const size_t n = rig.fd.log.size();
    rig.beat = 5.6;
    rig.tick();
    const auto ev = rig.fd.on(clipKey, n);
    REQUIRE(ev.size() == 3);                    // the old clock: none (position 0.6)
    for (size_t i = 0; i < ev.size(); ++i)
    {
        CHECK(ev[i].type == Ev::Fire);
        CHECK(static_cast<int>(ev[i].v) == static_cast<int>(i) + 1);
    }
    CHECK(rig.slot(0).position == Approx(1.6));
}

// The clock now keeps every beat across a stall, so a looping routine's position can pass more than one whole cycle in
// ONE tick (the old wrap reader never advanced a whole beat per tick -- this path was unreachable). It must fold every
// whole cycle at once with ONE restore, landing in the right cycle; the skipped cycles' events never fire.
TEST_CASE("RoutineEngine: a looping routine folds every whole cycle a tick gap covers at once, with one restore",
          "[routine][engine][stall][loop]")
{
    const ControlPath clipKey = layerKey(0, "activeClip");
    const ControlPath op0 = opacityKey(0);
    const ControlPath op1 = opacityKey(1);
    Routine r = makeRoutine("loop", 4.0, Clip::BeatSnapMode::Bar, true);   // test 8's loop routine
    Routine::PreambleEntry restoreOp; restoreOp.key = op1; restoreOp.continuous = true; restoreOp.norm = 0.4f;
    r.preamble = { restoreOp };
    r.lanes[clipKey] = discreteLane(clipKey, { point(1, 1.0, 3) });
    r.lanes[op0] = continuousLane(op0, { gesture(0.5, 0.1f, 3.5, 0.9f) });

    Rig rig;
    addToBank(rig.comp, r, 0);
    rig.tick();
    CHECK(rig.fire(0).empty());
    rig.runTo(4.0);                                   // start at bar 11
    CHECK(rig.slot(0).state == "running");
    CHECK(rig.slot(0).cycle == 1);
    rig.runTo(6.0);                                   // pos 2.0
    CHECK(rig.fd.firedLanePoints(clipKey, 3) == 1);
    CHECK(rig.fd.count(Ev::Touch, op1) == 1);

    rig.beat = 16.0;                                  // a 10-beat gap: 2.5 cycles
    rig.tick();
    CHECK(rig.slot(0).cycle == 4);                    // one fold per tick: cycle 2
    CHECK(rig.slot(0).position == Approx(0.0).margin(1e-9));
    CHECK(rig.fd.count(Ev::Touch, op1) == 2);         // ONE more restore
    CHECK(rig.fd.firedLanePoints(clipKey, 3) == 1);   // nothing due at pos 0; the skipped cycles' points never fired

    rig.runTo(17.0);
    CHECK(rig.fd.firedLanePoints(clipKey, 3) == 2);
    CHECK(rig.slot(0).cycle == 4);
    CHECK(rig.fd.count(Ev::Touch, op1) == 2);
}

// s-rta-0928 (restore-diag.md cause 2): a stall that steps over a whole recorded move (touched and closed in ONE tick)
// used to touch and release it unwritten -- the knob stayed where it was. The move's end value now lands before the
// release. The diag's 450 ms arm: a 0.26-beat one-write gesture, a 0.456 s (0.91-beat) tick gap across it.
TEST_CASE("RoutineEngine: a tick gap that steps over a whole recorded move still lands it", "[routine][engine][stall]")
{
    const ControlPath op0 = opacityKey(0);
    Routine r = makeRoutine("m", 8.0, Clip::BeatSnapMode::Off, false);
    r.lanes[op0] = continuousLane(op0, { gesture(1.173, 0.5f, 1.707, 0.5f) });

    Rig rig;
    addToBank(rig.comp, r, 0);
    rig.tick();
    rig.runTo(4.0);
    CHECK(rig.fire(0).empty());                       // Off: starts now, startBeat 4.0
    CHECK(rig.slot(0).state == "running");
    rig.runTo(4.9375);
    const size_t mark = rig.fd.log.size();

    SECTION("the gap spans the whole move")
    {
        rig.beat = 5.85;                              // routine position 0.94 -> 1.85
        rig.tick();
        const auto ev = rig.fd.on(op0, mark);
        REQUIRE(ev.size() == 3);
        checkEvent(ev[0], Ev::Touch, op0);
        checkEvent(ev[1], Ev::Set, op0);
        CHECK(ev[1].v == Approx(0.5f));
        checkEvent(ev[2], Ev::Release, op0);
        CHECK(rig.fd.lastSet(op0) == Approx(0.5f));
        CHECK(rig.slot(0).yielded == 0);
    }
    SECTION("control: the gap lands inside the move")
    {
        rig.beat = 5.25;                              // routine position 1.25: inside [1.173, 1.707]
        rig.tick();
        rig.runTo(5.75);
        CHECK(rig.fd.lastSet(op0) == Approx(0.5f));
        CHECK(rig.fd.count(Ev::Release, op0) == 1);
    }
}

// s-rta-0928 (Harmony adoption D5): the end value a gesture close now writes passes the owner check like any in-gesture
// write. Routine A holds layer 0's opacity over [0, 4]; routine B (fired after A, so ticked after it) begins a gesture on
// the same knob one tick before A's end. A's end-value write is REFUSED -- no write, no release of B's grip -- and counts
// as A's one yield, exactly as a refused in-gesture write does in the D9 stacking tests above.
TEST_CASE("RoutineEngine: a gesture's end value is refused when a later routine took the knob just before it",
          "[routine][engine][stacking][stall]")
{
    Rig rig;
    const ControlPath key = opacityKey(0);
    Routine a = makeRoutine("A", 16.0, Clip::BeatSnapMode::Bar, false);
    a.lanes[key] = continuousLane(key, { gesture(0.0, 0.2f, 4.0, 0.2f) });
    Routine b = makeRoutine("B", 16.0, Clip::BeatSnapMode::Bar, false);
    b.lanes[key] = continuousLane(key, { gesture(3.9375, 0.8f, 6.0, 0.8f) });
    addToBank(rig.comp, a, 0);
    addToBank(rig.comp, b, 1);
    rig.tick();
    CHECK(rig.fire(0).empty());
    CHECK(rig.fire(1).empty());
    rig.runTo(4.0);                                   // both start on bar 11, same tick
    rig.runTo(7.875);
    CHECK(rig.fd.lastSet(key) == Approx(0.2f));       // A's hand until B begins
    const size_t atB = rig.fd.log.size();
    rig.runTo(7.9375);                                // routine beat 3.9375: A writes 0.2, then B touches and writes 0.8
    {
        const auto ev = rig.fd.on(key, atB);
        REQUIRE(ev.size() == 3);
        checkEvent(ev[0], Ev::Set, key);
        CHECK(ev[0].v == Approx(0.2f));
        checkEvent(ev[1], Ev::Touch, key);
        checkEvent(ev[2], Ev::Set, key);
        CHECK(ev[2].v == Approx(0.8f));
    }
    CHECK(rig.slot(0).yielded == 0);
    const size_t atEnd = rig.fd.log.size();
    rig.runTo(8.0);                                   // A's end: its end value 0.2 is refused -- B's write only
    {
        const auto ev = rig.fd.on(key, atEnd);
        REQUIRE(ev.size() == 1);
        checkEvent(ev[0], Ev::Set, key);
        CHECK(ev[0].v == Approx(0.8f));
    }
    CHECK(rig.slot(0).yielded == 1);                  // A yielded once (the stacking tests' count)
    CHECK(rig.slot(1).yielded == 0);
    CHECK(rig.fd.count(Ev::Release, key) == 0);       // A never lets go of B's grip
    rig.runTo(10.0);                                  // B's end: its own end value, then its release
    CHECK(rig.fd.lastSet(key) == Approx(0.8f));
    CHECK(rig.fd.count(Ev::Release, key) == 1);
}

// s-rta-0928 (restore-diag.md): /api/routine/status bank[].holdMs / holdMsMax -- how long a start and every loop
// return held the engine (the message thread), measured on the engine's steady clock around the restore and that
// tick's replay. A restore whose discrete fire takes 20 ms must publish >= 20 ms.
TEST_CASE("RoutineEngine: a start and every loop return publish how long they held the engine", "[routine][engine][hold]")
{
    const ControlPath clipKey = layerKey(0, "activeClip");
    Routine r = makeRoutine("h", 4.0, Clip::BeatSnapMode::Bar, true);
    r.restoreStyle = Routine::RestoreStyle::Jump;     // no glides: the whole restore lands in the boundary tick
    Routine::PreambleEntry restoreClip; restoreClip.key = clipKey; restoreClip.v = 2;
    r.preamble = { restoreClip };

    SECTION("restore on: the start and the loop return each hold >= the restore's 20 ms")
    {
        Rig rig;
        addToBank(rig.comp, r, 0);
        rig.fd.preambleFireSleepMs = 20;
        rig.tick();
        CHECK(rig.fire(0).empty());
        CHECK(rig.slot(0).state == "pending");
        CHECK(rig.slot(0).holdMs == -1.0);
        CHECK(rig.slot(0).holdMsMax == -1.0);
        rig.runTo(4.0);                               // the start, on bar 11
        {
            const auto s = rig.slot(0);
            CHECK(s.state == "running");
            CHECK(s.holdMs >= 20.0);
            CHECK(s.holdMs < 2000.0);
            CHECK(s.holdMsMax == s.holdMs);
        }
        rig.runTo(8.0);                               // the loop return
        {
            const auto s = rig.slot(0);
            CHECK(s.cycle == 2);
            CHECK(s.holdMs >= 20.0);
            CHECK(s.holdMsMax >= s.holdMs);
        }
    }
    SECTION("restore off: the start holds under the restore's 20 ms")
    {
        r.restoreState = false;
        Rig rig;
        addToBank(rig.comp, r, 0);
        rig.fd.preambleFireSleepMs = 20;
        rig.tick();
        CHECK(rig.fire(0).empty());
        rig.runTo(4.0);
        const auto s = rig.slot(0);
        CHECK(s.state == "running");
        CHECK(s.holdMs >= 0.0);
        CHECK(s.holdMs < 20.0);
    }
}
