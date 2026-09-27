// test_routine -- s-rta-0926 routines slice 1, lane 1a (plan-routines-s1-final.md
// sections 3.1-3.6, test table 6.1 cases 1-6): the Routine model and its
// composition serialization, sliceRoutine (cut a take's lanes into a
// beat-native, relative lane set + an explicit restore list), takeBeatOfBar,
// and compileRoutine (a Beat-clock Program for a routine; stampless gestures
// are exact on the Beat clock, never reported as invalid).
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "model/Composition.h"
#include "model/Routine.h"
#include "recording/Take.h"
#include "recording/Program.h"
#include "recording/PerfStateCapture.h"
#include "recording/RoutineSlice.h"
#include "effects/EffectLibrary.h"
#include <algorithm>
#include <string>

using Catch::Approx;

namespace
{
    EffectLibrary& library()
    {
        static EffectLibrary lib = [] {
            EffectLibrary l;
            l.registerDefaults();
            return l;
        }();
        return lib;
    }

    // One deck ("Deck 1"), three layers ("Layer 1/2/3"), 12 columns each.
    Composition makeComposition()
    {
        Composition comp;
        comp.initDefault();
        return comp;
    }

    // A recorded (absolute-deck) Layer-scope key, names matching initDefault().
    ControlPath layerKey(int layer, const std::string& control, const std::string& scalar = "")
    {
        ControlPath k;
        k.scope = ControlPath::Scope::Layer;
        k.deck = 0;
        k.deckName = "Deck 1";
        k.layer = layer;
        k.layerName = "Layer " + std::to_string(layer + 1);
        k.control = control;
        k.scalar = scalar;
        return k;
    }

    ControlPath opacityKey(int layer) { return layerKey(layer, "scalar", "opacity"); }

    ControlPath compKey(const std::string& control, const std::string& scalar = "")
    {
        ControlPath k;
        k.scope = ControlPath::Scope::Comp;
        k.control = control;
        k.scalar = scalar;
        return k;
    }

    void setSteadyTempo(Take& take)
    {
        take.tempo.a = { { 0.0, 0.0, 0, 120.0f, "start" } };
    }

    // A discrete point at `beat` with real-looking (non-zero) wall/sample stamps.
    DiscretePoint point(uint64_t seq, double beat, int v)
    {
        DiscretePoint p;
        p.s = { seq, beat * 0.5, static_cast<uint64_t>(beat * 24000.0) };
        p.beat = beat;
        p.bpm = 120.0f;
        p.v = v;
        return p;
    }

    // A two-breakpoint linear gesture from (x0, y0) to (x1, y1), with parallel stamps.
    Gesture gesture(double x0, float y0, double x1, float y1, bool withStamps = true)
    {
        Gesture g;
        g.grip = "held";
        g.curve.pts = { { x0, y0 }, { x1, y1 } };
        if (withStamps)
            g.stamps = { { 1, x0 * 0.5, static_cast<uint64_t>(x0 * 24000.0) },
                         { 2, x1 * 0.5, static_cast<uint64_t>(x1 * 24000.0) } };
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

    // Finds the routine lane whose key matches `recorded` once made deck-relative
    // (the slicer's step 3).
    const Lane* findLane(const Routine& r, ControlPath recorded)
    {
        recorded.deckRelative = true;
        auto it = r.lanes.find(recorded);
        return it == r.lanes.end() ? nullptr : &it->second;
    }

    int indexOfControl(const Routine& r, const std::string& control)
    {
        for (size_t i = 0; i < r.preamble.size(); ++i)
            if (r.preamble[i].key.control == control)
                return static_cast<int>(i);
        return -1;
    }

    std::string routinesJson(const Composition& comp)
    {
        auto v = comp.toVar();
        auto* obj = v.getDynamicObject();
        return juce::JSON::toString(obj->getProperty("routines")).toStdString()
             + juce::JSON::toString(obj->getProperty("routineBank")).toStdString();
    }
}

// === 1: the model round-trips inside the composition JSON; old files load with none ===

TEST_CASE("Routine: composition round-trip, legacy file, unknown quantize, orphan bank entry", "[routine][model]")
{
    Composition comp = makeComposition();

    Routine r;
    r.uuid = "11111111-2222-3333-4444-555555555555";
    r.name = "Drop 1";
    r.lengthBeats = 16.0;
    r.quantize = Clip::BeatSnapMode::TwoBar;
    r.loop = true;
    r.restoreState = true;
    r.deckRelative = true;
    r.source = { "/tmp/Friday.adna-take", 128.0, 144.0 };

    ControlPath activeClip = layerKey(0, "activeClip"); activeClip.deckRelative = true;
    ControlPath playing = layerKey(0, "playing"); playing.scope = ControlPath::Scope::Clip;
    playing.col = 2; playing.clipName = "Clip C"; playing.deckRelative = true;
    ControlPath opacity = opacityKey(1); opacity.deckRelative = true;

    Routine::PreambleEntry e1; e1.key = activeClip; e1.v = 2;
    Routine::PreambleEntry e2; e2.key = playing; e2.action = "pause";
    Routine::PreambleEntry e3; e3.key = opacity; e3.continuous = true; e3.norm = 0.5f;
    r.preamble = { e1, e2, e3 };

    DiscretePoint p0 = point(7, 1.0, 3); p0.s.t = 0.0; p0.s.sample = 0;
    r.lanes[activeClip] = discreteLane(activeClip, { p0 });
    r.lanes[opacity] = continuousLane(opacity, { gesture(0.0, 0.2f, 2.0, 0.9f, /*withStamps*/ false) });

    comp.routines.push_back(r);
    REQUIRE(comp.assignRoutineSlot(3, r.uuid));
    CHECK(comp.firstFreeRoutineSlot() == 0);
    REQUIRE(comp.routineInSlot(3) != nullptr);
    CHECK(comp.routineInSlot(0) == nullptr);

    const juce::var saved = comp.toVar();
    REQUIRE(saved.getDynamicObject()->hasProperty("routines"));
    REQUIRE(saved.getDynamicObject()->hasProperty("routineBank"));

    Composition back;
    back.fromVar(saved);
    CHECK(back.routineLoadNote.empty());
    REQUIRE(back.routines.size() == 1);
    REQUIRE(back.routineBank.size() == 1);
    CHECK(back.routineBank[0].slot == 3);
    CHECK(back.routineBank[0].uuid == r.uuid);

    const Routine* rr = back.routineInSlot(3);
    REQUIRE(rr != nullptr);
    CHECK(rr->uuid == r.uuid);
    CHECK(rr->name == "Drop 1");
    CHECK(rr->lengthBeats == Approx(16.0));
    CHECK(rr->quantize == Clip::BeatSnapMode::TwoBar);
    CHECK(rr->loop);
    CHECK(rr->restoreState);
    CHECK(rr->deckRelative);
    CHECK(rr->source.takeFolder == "/tmp/Friday.adna-take");
    CHECK(rr->source.fromBeat == Approx(128.0));
    CHECK(rr->source.toBeat == Approx(144.0));

    REQUIRE(rr->preamble.size() == 3);
    CHECK(rr->preamble[0].key == activeClip);
    CHECK_FALSE(rr->preamble[0].continuous);
    CHECK(rr->preamble[0].v == 2);
    CHECK(rr->preamble[1].key == playing);
    CHECK(rr->preamble[1].action == "pause");
    CHECK(rr->preamble[1].key.clipName == "Clip C");
    CHECK(rr->preamble[2].key == opacity);
    CHECK(rr->preamble[2].continuous);
    CHECK(rr->preamble[2].norm == Approx(0.5f));

    REQUIRE(rr->lanes.size() == 2);
    REQUIRE(rr->lanes.count(activeClip) == 1);
    REQUIRE(rr->lanes.count(opacity) == 1);
    CHECK(rr->lanes.at(activeClip).points.size() == 1);
    CHECK(rr->lanes.at(activeClip).points[0].beat == Approx(1.0));
    CHECK(rr->lanes.at(opacity).gestures.size() == 1);
    CHECK(rr->lanes.at(opacity).gestures[0].stamps.empty());

    // Idempotent: the two routine keys re-serialize byte-identically.
    CHECK(routinesJson(comp) == routinesJson(back));

    // A var WITHOUT the keys (every composition saved before routines existed) loads with
    // both vectors empty -- even into a Composition that held routines before (fromVar clears).
    {
        juce::var legacy = comp.toVar();
        legacy.getDynamicObject()->removeProperty("routines");
        legacy.getDynamicObject()->removeProperty("routineBank");
        Composition reused = back;
        reused.fromVar(legacy);
        CHECK(reused.routines.empty());
        CHECK(reused.routineBank.empty());
        CHECK(reused.routineLoadNote.empty());
    }

    // Enums are strings; an unknown quantize string reads as Bar, never silently Off.
    CHECK(Routine::quantizeFromString("sideways") == Clip::BeatSnapMode::Bar);
    for (auto m : { Clip::BeatSnapMode::Off, Clip::BeatSnapMode::Beat, Clip::BeatSnapMode::Bar,
                    Clip::BeatSnapMode::TwoBar, Clip::BeatSnapMode::FourBar })
        CHECK(Routine::quantizeFromString(Routine::quantizeToString(m)) == m);
    CHECK(std::string(Routine::quantizeToString(Clip::BeatSnapMode::TwoBar)) == "2bar");

    // A bank entry whose uuid matches no routine is dropped at load and said once.
    {
        juce::var withOrphan = comp.toVar();
        juce::Array<juce::var> bank;
        const std::vector<std::pair<int, std::string>> entries{ { 3, r.uuid }, { 4, "no-such-routine" } };
        for (const auto& [slot, uuid] : entries)
        {
            auto* o = new juce::DynamicObject();
            o->setProperty("slot", slot);
            o->setProperty("uuid", juce::String(uuid));
            bank.add(juce::var(o));
        }
        withOrphan.getDynamicObject()->setProperty("routineBank", bank);
        Composition loaded;
        loaded.fromVar(withOrphan);
        REQUIRE(loaded.routineBank.size() == 1);
        CHECK(loaded.routineBank[0].slot == 3);
        CHECK_FALSE(loaded.routineLoadNote.empty());
    }

    // Saving onto an occupied pad replaces it; the old routine is erased because no other
    // pad references it. Removing the last pad erases its routine too.
    {
        Composition c2 = back;
        Routine other = r;
        other.uuid = "other-uuid";
        other.name = "Drop 2";
        c2.routines.push_back(other);
        REQUIRE(c2.assignRoutineSlot(3, other.uuid));
        REQUIRE(c2.routines.size() == 1);
        CHECK(c2.routines[0].uuid == "other-uuid");
        CHECK(c2.routineInSlot(3)->name == "Drop 2");
        CHECK_FALSE(c2.assignRoutineSlot(Composition::kRoutineBankSize, other.uuid));
        REQUIRE(c2.removeRoutineSlot(3));
        CHECK(c2.routineBank.empty());
        CHECK(c2.routines.empty());
        CHECK_FALSE(c2.removeRoutineSlot(3));
    }
}

// === s-rta-0926b routines-followup ITEM 2: the per-routine restore style (Ease / Jump) is saved with the show ===

TEST_CASE("Routine: the restore style round-trips; a file without it, or with an unknown one, loads as Ease",
          "[routine][model][restorestyle]")
{
    Composition comp = makeComposition();
    Routine r;
    r.uuid = "style-uuid";
    r.name = "Jumper";
    CHECK(r.restoreStyle == Routine::RestoreStyle::Ease);   // the default
    r.restoreStyle = Routine::RestoreStyle::Jump;
    comp.routines.push_back(r);
    REQUIRE(comp.assignRoutineSlot(0, r.uuid));

    const juce::var saved = comp.toVar();
    auto* savedRoutines = saved.getDynamicObject()->getProperty("routines").getArray();
    REQUIRE(savedRoutines != nullptr);
    REQUIRE(savedRoutines->size() == 1);
    CHECK(savedRoutines->getReference(0).getProperty("restoreStyle", "").toString() == "jump");   // a string (D12)

    Composition back;
    back.fromVar(saved);
    REQUIRE(back.routineInSlot(0) != nullptr);
    CHECK(back.routineInSlot(0)->restoreStyle == Routine::RestoreStyle::Jump);
    CHECK(routinesJson(comp) == routinesJson(back));

    // A routine saved before the setting existed (no key) loads as Ease; so does an unknown value.
    for (const juce::var& style : { juce::var(), juce::var("sideways") })
    {
        const juce::var v = comp.toVar();
        auto* obj = v.getDynamicObject()->getProperty("routines").getArray()->getReference(0).getDynamicObject();
        REQUIRE(obj != nullptr);
        if (style.isVoid())
            obj->removeProperty("restoreStyle");
        else
            obj->setProperty("restoreStyle", style);
        Composition loaded;
        loaded.fromVar(v);
        REQUIRE(loaded.routineInSlot(0) != nullptr);
        CHECK(loaded.routineInSlot(0)->restoreStyle == Routine::RestoreStyle::Ease);
    }
}

// === 2: slice cuts, rebases, synthesizes straddling breakpoints, and builds the restore list ===

TEST_CASE("sliceRoutine: rebases to routine beats, synthesizes straddling breakpoints, restore list in order", "[routine][slice]")
{
    Composition comp = makeComposition();
    Clip clipB; clipB.name = "Clip B";
    comp.decks[0].layers[0].clips[1] = clipB;

    Take take;
    setSteadyTempo(take);
    take.checkpoint0 = capturePerfState(comp, 120.0f, "");

    const ControlPath activeClip = layerKey(0, "activeClip");
    take.lanes[activeClip] = discreteLane(activeClip, { point(1, 1.0, 1), point(2, 5.0, 2), point(3, 9.0, 3) });

    const ControlPath op0 = opacityKey(0);
    take.lanes[op0] = continuousLane(op0, {
        gesture(0.0, 0.3f, 1.0, 0.6f),      // entirely before the cut
        gesture(3.0, 0.2f, 7.0, 0.8f),      // straddles the start (covers beat 4)
        gesture(10.0, 0.0f, 14.0, 1.0f),    // straddles the end (beat 12)
    });

    const ControlPath op1 = opacityKey(1);
    take.lanes[op1] = continuousLane(op1, { gesture(0.0, 0.1f, 1.0, 0.6f) });   // before the cut only

    SliceRequest req;
    req.fromBeat = 4.0;
    req.toBeat = 12.0;
    req.name = "Drop";
    req.takeFolder = "/tmp/probe.adna-take";

    const SliceResult res = sliceRoutine(take, req, library());
    REQUIRE(res.error.empty());
    REQUIRE(res.routine.has_value());
    const Routine& r = *res.routine;

    CHECK_FALSE(r.uuid.empty());
    CHECK(r.name == "Drop");
    CHECK(r.lengthBeats == Approx(8.0));
    CHECK(r.source.takeFolder == "/tmp/probe.adna-take");
    CHECK(r.source.fromBeat == Approx(4.0));
    CHECK(r.source.toBeat == Approx(12.0));
    CHECK(res.droppedLanes.empty());

    // Discrete: points at beats 5 and 9 survive, rebased to x = 1 and 5; wall/sample stamps dropped,
    // seq kept (it is the (at, seq) tie-break).
    const Lane* ac = findLane(r, activeClip);
    REQUIRE(ac != nullptr);
    CHECK(ac->key.deckRelative);
    REQUIRE(ac->points.size() == 2);
    CHECK(ac->points[0].beat == Approx(1.0));
    CHECK(ac->points[0].v == 2);
    CHECK(ac->points[0].s.seq == 2);
    CHECK(ac->points[0].s.t == 0.0);
    CHECK(ac->points[0].s.sample == 0);
    CHECK(ac->points[1].beat == Approx(5.0));
    CHECK(ac->points[1].v == 3);

    // Continuous, layer 0: the straddling gesture begins at x = 0 with y = eval(4) = 0.35 and ends
    // at x = 3 (beat 7) with 0.8; the end-straddling one keeps x = 6 (beat 10) and gets a
    // synthesized last breakpoint at x = 8 with eval(12) = 0.5. No stamps survive.
    const Lane* l0 = findLane(r, op0);
    REQUIRE(l0 != nullptr);
    REQUIRE(l0->gestures.size() == 2);
    const auto& g0 = l0->gestures[0];
    REQUIRE(g0.curve.pts.size() == 2);
    CHECK(g0.curve.pts[0].x == Approx(0.0));
    CHECK(g0.curve.pts[0].y == Approx(0.35f));
    CHECK(g0.curve.pts[1].x == Approx(3.0));
    CHECK(g0.curve.pts[1].y == Approx(0.8f));
    CHECK(g0.stamps.empty());
    CHECK(g0.grip == "held");
    const auto& g1 = l0->gestures[1];
    REQUIRE(g1.curve.pts.size() == 2);
    CHECK(g1.curve.pts[0].x == Approx(6.0));
    CHECK(g1.curve.pts[0].y == Approx(0.0f));
    CHECK(g1.curve.pts[1].x == Approx(8.0));
    CHECK(g1.curve.pts[1].y == Approx(0.5f));
    CHECK(g1.stamps.empty());

    // Layer 1 has no movement inside the cut: no timeline, but its control IS restored.
    CHECK(findLane(r, op1) == nullptr);
    CHECK(r.lanes.size() == 2);

    // Restore list: NO layer-0 opacity entry (a gesture covers the start -- its synthesized
    // begin IS the state); layer-1 opacity from its last gesture (0.6); activeClip from the last
    // point before the cut (v = 1), FOLLOWED by a play/pause entry for that clip (checkpoint 0:
    // the clip was not playing -> "pause").
    for (const auto& e : r.preamble)
        CHECK(e.key.deckRelative);

    int layer0Opacity = 0;
    int layer1Opacity = 0;
    for (const auto& e : r.preamble)
    {
        if (e.continuous && e.key.scalar == "opacity" && e.key.layer == 0) ++layer0Opacity;
        if (e.continuous && e.key.scalar == "opacity" && e.key.layer == 1)
        {
            ++layer1Opacity;
            CHECK(e.norm == Approx(0.6f));
        }
    }
    CHECK(layer0Opacity == 0);
    CHECK(layer1Opacity == 1);

    const int iActive = indexOfControl(r, "activeClip");
    const int iPlaying = indexOfControl(r, "playing");
    REQUIRE(iActive >= 0);
    REQUIRE(iPlaying >= 0);
    CHECK(iActive < iPlaying);
    CHECK(r.preamble[static_cast<size_t>(iActive)].v == 1);
    const auto& play = r.preamble[static_cast<size_t>(iPlaying)];
    CHECK(play.key.scope == ControlPath::Scope::Clip);
    CHECK(play.key.layer == 0);
    CHECK(play.key.col == 1);
    CHECK(play.action == "pause");
    CHECK_FALSE(play.continuous);
    CHECK(r.preamble.size() == 3);

    CHECK(res.preambleFromLanes == 2);
    CHECK(res.preambleFromCheckpoint == 1);
    CHECK(res.preambleFromDefaults == 0);
    CHECK(res.preambleUnknown == 0);
}

// === 3: restore fallbacks -- checkpoint 0, then library/scalar defaults, else counted unknown ===

TEST_CASE("sliceRoutine: restore falls back to checkpoint 0, then defaults; comp scalars are unknown", "[routine][slice]")
{
    Composition comp = makeComposition();
    comp.decks[0].layers[1].opacity = 0.4f;

    const auto* ripple = library().getEffectDef("Ripple");
    REQUIRE(ripple != nullptr);
    REQUIRE(ripple->params.size() >= 2);
    Clip clipA; clipA.name = "Clip A";
    Clip::EffectSlot slot;
    slot.effectName = "Ripple";
    for (const auto& p : ripple->params)
        slot.addParam(p.defaultValue);
    clipA.effects.push_back(slot);
    comp.decks[0].layers[0].clips[0] = clipA;

    Take take;
    setSteadyTempo(take);
    take.checkpoint0 = capturePerfState(comp, 120.0f, "");

    // Layer 1 opacity: only moved AFTER the cut start -> checkpoint 0 (0.4).
    const ControlPath op1 = opacityKey(1);
    take.lanes[op1] = continuousLane(op1, { gesture(6.0, 0.9f, 8.0, 0.1f) });

    // Clip 0's Ripple param 1: at its default at Record (absent from checkpoint 0) -> library default.
    ControlPath fxParam = layerKey(0, "param");
    fxParam.scope = ControlPath::Scope::Clip;
    fxParam.col = 0; fxParam.clipName = "Clip A";
    fxParam.fx = 0; fxParam.fxName = "Ripple";
    fxParam.param = 1;
    take.lanes[fxParam] = continuousLane(fxParam, { gesture(5.0, 0.1f, 6.0, 0.9f) });

    // Composition master opacity: not in PerfState -> unknown, no entry (the lane itself is kept).
    const ControlPath compOpacity = compKey("scalar", "opacity");
    take.lanes[compOpacity] = continuousLane(compOpacity, { gesture(5.0, 0.5f, 6.0, 1.0f) });

    SliceRequest req;
    req.fromBeat = 4.0;
    req.toBeat = 12.0;
    const SliceResult res = sliceRoutine(take, req, library());
    REQUIRE(res.error.empty());
    REQUIRE(res.routine.has_value());
    const Routine& r = *res.routine;

    CHECK(res.preambleFromLanes == 0);
    CHECK(res.preambleFromCheckpoint == 1);
    CHECK(res.preambleFromDefaults == 1);
    CHECK(res.preambleUnknown == 1);
    REQUIRE(r.preamble.size() == 2);

    bool sawLayer1 = false, sawParam = false;
    for (const auto& e : r.preamble)
    {
        REQUIRE(e.continuous);
        CHECK(e.key.scope != ControlPath::Scope::Comp);
        if (e.key.scope == ControlPath::Scope::Layer && e.key.layer == 1)
        {
            sawLayer1 = true;
            CHECK(e.norm == Approx(0.4f));
        }
        if (e.key.scope == ControlPath::Scope::Clip && e.key.control == "param")
        {
            sawParam = true;
            CHECK(e.norm == Approx(ripple->params[1].defaultValue));
            CHECK(e.key.fx == 0);
            CHECK(e.key.param == 1);
        }
    }
    CHECK(sawLayer1);
    CHECK(sawParam);

    CHECK(r.lanes.size() == 3);
    CHECK(r.lanes.count(compOpacity) == 1);   // Comp keys carry no deck -> never made deck-relative
}

// === 4: transport-class lanes dropped by name; unmetered stretch refused; whole-bar rounding ===

TEST_CASE("sliceRoutine: drops tempo/audio/activeDeck lanes, refuses an unmetered stretch, rounds to whole bars", "[routine][slice]")
{
    Take take;
    setSteadyTempo(take);

    const ControlPath tempo = compKey("tempo");
    const ControlPath audio = compKey("audio");
    const ControlPath deck = compKey("activeDeck");
    take.lanes[tempo] = discreteLane(tempo, { point(1, 1.0, 0) });
    take.lanes[audio] = discreteLane(audio, { point(2, 2.0, 0) });
    take.lanes[deck] = discreteLane(deck, { point(3, 3.0, 0) });
    const ControlPath op0 = opacityKey(0);
    take.lanes[op0] = continuousLane(op0, { gesture(1.0, 0.2f, 2.0, 0.8f) });

    SliceRequest req;
    req.fromBeat = 0.0;
    req.toBeat = 6.0;
    {
        const SliceResult res = sliceRoutine(take, req, library());
        REQUIRE(res.error.empty());
        REQUIRE(res.routine.has_value());
        REQUIRE(res.droppedLanes.size() == 3);
        auto has = [&](const std::string& name) {
            return std::any_of(res.droppedLanes.begin(), res.droppedLanes.end(),
                               [&](const std::string& d) { return d.find(name) != std::string::npos; });
        };
        CHECK(has("tempo"));
        CHECK(has("audio"));
        CHECK(has("activeDeck"));
        CHECK(res.routine->lanes.size() == 1);
        CHECK(res.routine->lengthBeats == Approx(8.0));   // 6 beats -> 2 whole bars
    }
    {
        SliceRequest exact = req;
        exact.wholeBars = false;
        const SliceResult res = sliceRoutine(take, exact, library());
        REQUIRE(res.routine.has_value());
        CHECK(res.routine->lengthBeats == Approx(6.0));
    }

    // An anchor with bpm 0 inside the range: "this stretch has no beat" -> refused, no routine.
    {
        Take unmetered = take;
        unmetered.tempo.a = { { 0.0, 0.0, 0, 120.0f, "start" },
                              { 2.0, 4.0, 0, 0.0f, "unlock" },
                              { 5.0, 4.0, 0, 120.0f, "lock" } };
        SliceRequest r2;
        r2.fromBeat = 2.0;
        r2.toBeat = 10.0;
        const SliceResult res = sliceRoutine(unmetered, r2, library());
        CHECK_FALSE(res.error.empty());
        CHECK_FALSE(res.routine.has_value());
    }

    // No tempo map at all, or an empty/backwards range: refused.
    {
        Take noTempo = take;
        noTempo.tempo.a.clear();
        const SliceResult res = sliceRoutine(noTempo, req, library());
        CHECK_FALSE(res.error.empty());
        CHECK_FALSE(res.routine.has_value());
    }
    {
        SliceRequest backwards;
        backwards.fromBeat = 8.0;
        backwards.toBeat = 8.0;
        const SliceResult res = sliceRoutine(take, backwards, library());
        CHECK_FALSE(res.error.empty());
        CHECK_FALSE(res.routine.has_value());
    }
}

// === 5: the take's bar grid ===

TEST_CASE("takeBeatOfBar: bar 1 is the first full bar after Record; unknown grid counts from the start", "[routine][slice]")
{
    Take take;
    take.meta.startBeatInBar = 2.5;
    CHECK(takeBeatOfBar(take, 1) == Approx(1.5));
    CHECK(takeBeatOfBar(take, 2) == Approx(5.5));

    take.meta.startBeatInBar = -1.0;
    CHECK(takeBeatOfBar(take, 1) == Approx(0.0));
    CHECK(takeBeatOfBar(take, 2) == Approx(4.0));

    take.meta.startBeatInBar = 0.0;
    CHECK(takeBeatOfBar(take, 1) == Approx(0.0));
}

TEST_CASE("takeBeatOfBar refuses bar < 1: treated as bar 1, never a negative beat", "[routine][slice]")
{
    Take take;
    take.meta.startBeatInBar = 2.5;
    const double bar1 = takeBeatOfBar(take, 1);
    CHECK(takeBeatOfBar(take, 0) == Approx(bar1));
    CHECK(takeBeatOfBar(take, -3) == Approx(bar1));
}

// === 6: compileRoutine -- Beat clock, routine length, unresolved counted, stampless is exact ===

TEST_CASE("compileRoutine: Beat clock, routine length, missing targets counted, stampless gestures exact", "[routine][compile]")
{
    Composition comp = makeComposition();

    ControlPath missing = opacityKey(5); missing.deckRelative = true;   // no layer 6 in initDefault()
    ControlPath present = opacityKey(0); present.deckRelative = true;

    Routine r;
    r.uuid = "u";
    r.name = "R";
    r.lengthBeats = 16.0;
    r.loop = true;
    r.lanes[missing] = continuousLane(missing, { gesture(0.0, 0.1f, 2.0, 0.9f, /*withStamps*/ false) });
    r.lanes[present] = continuousLane(present, { gesture(1.0, 0.2f, 3.0, 0.7f, /*withStamps*/ false) });

    Routine::PreambleEntry lost; lost.key = missing; lost.continuous = true; lost.norm = 0.5f;
    ControlPath activeClip = layerKey(0, "activeClip"); activeClip.deckRelative = true;
    Routine::PreambleEntry clipEntry; clipEntry.key = activeClip; clipEntry.v = 0;
    r.preamble = { clipEntry, lost };

    auto p = compileRoutine(r, comp);
    REQUIRE(p != nullptr);
    CHECK(p->clock == DriveClock::Beat);
    CHECK(p->loop);
    CHECK(p->length == Approx(16.0));
    CHECK(p->report.unresolved.size() == 1);
    CHECK(p->report.preambleUnresolved.size() == 1);
    CHECK(p->report.resolvedCount == 1);
    CHECK(p->report.invalid.empty());
    CHECK(p->report.preambleCount == 1);

    REQUIRE(p->preamble.size() == 1);
    CHECK(p->preamble[0].key.control == "activeClip");
    CHECK(p->preamble[0].p.v == 0);
    CHECK(p->preamble[0].p.origin == Origin::Preamble);
    CHECK(p->preamble[0].target.deck == 0);
    CHECK(p->preamble[0].target.layer == 0);
    CHECK(p->preambleContinuous.empty());

    REQUIRE(p->continuous.size() == 1);
    REQUIRE(p->continuous[0].gestures.size() == 1);
    CHECK(p->continuous[0].gestures[0].x0 == Approx(1.0));
    CHECK(p->continuous[0].gestures[0].x1 == Approx(3.0));

    // compile() on a take: a stampless gesture is exact (not invalid) on the Beat clock, and STILL
    // reported invalid on the Wall clock (regression guard for the stamp-rule change).
    Take take;
    setSteadyTempo(take);
    const ControlPath op0 = opacityKey(0);
    take.lanes[op0] = continuousLane(op0, { gesture(0.0, 0.0f, 4.0, 1.0f, /*withStamps*/ false) });

    auto wall = compile(take, comp, DriveClock::Wall);
    REQUIRE(wall->report.invalid.size() == 1);
    CHECK(wall->report.invalid.front().key == op0);

    auto beat = compile(take, comp, DriveClock::Beat);
    CHECK(beat->report.invalid.empty());
    REQUIRE(beat->continuous.size() == 1);
    CHECK(beat->continuous[0].gestures[0].x1 == Approx(4.0));
}
