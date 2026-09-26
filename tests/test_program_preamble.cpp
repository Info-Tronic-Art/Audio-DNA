// test_program_preamble -- s-rta-0925: "replay restore" (checkpoint 0 becomes
// the Program preamble, fired at Play). Boris ruled 2026-09-25
// (binding-decisions.md): whole-take replay first restores the look at
// Record time, then plays the moves. This file pins Program::compile's
// buildPreamble() synthesis (plan section 3.2's emission-order table):
// resolution tri-state (D2) applies to the preamble exactly as to a
// recorded lane, emission order is load-bearing (a trigger must precede its
// clip's play/pause -- R4), and the preamble never enters the discrete/
// continuous schedule a recorded lane point does.
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "model/Composition.h"
#include "recording/Take.h"
#include "recording/Program.h"
#include "recording/PerfStateCapture.h"
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

    // One deck ("Deck 1"), three layers ("Layer 1/2/3"), 12 columns each --
    // Composition::initDefault()'s own shape (matches test_program_stamps.cpp).
    Composition makeComposition()
    {
        Composition comp;
        comp.initDefault();
        return comp;
    }
}

// === 1: compile builds the preamble from checkpoint0 in restore order ===

TEST_CASE("Program::compile builds the preamble from checkpoint0 in restore order", "[program][preamble]")
{
    Composition comp = makeComposition();
    comp.activeDeckIndex = 0;
    comp.quantizeMode = Composition::QuantizeMode::NextDownbeat;   // v = 2

    Layer& layer0 = comp.decks[0].layers[0];
    layer0.activeClipColumn = 2;
    layer0.opacity = 0.5f;
    layer0.visible = false;
    layer0.solo = true;
    // bypass, mute, autopilot stay at their live-model defaults (false).

    const auto* def = library().getEffectDef("Ripple");
    REQUIRE(def != nullptr);
    REQUIRE(def->params.size() >= 2);

    Clip clip;
    clip.name = "C";
    clip.playing = true;
    Clip::EffectSlot slot;
    slot.effectName = "Ripple";
    for (const auto& p : def->params)
        slot.paramValues.push_back(p.defaultValue);
    slot.paramValues[1] = std::clamp(def->params[1].defaultValue + 0.4f, 0.0f, 1.0f);
    clip.effects.push_back(slot);
    clip.clipOpacity = 0.25f;   // ClipScalar::Opacity, identity toNorm -> 0.25
    layer0.clips[2] = clip;

    Take take;
    take.checkpoint0 = capturePerfState(comp, 120.0f, "");
    auto p = compile(take, comp, DriveClock::Wall);

    REQUIRE(p->report.preambleUnresolved.empty());
    REQUIRE(p->report.reboundByPosition.empty());
    REQUIRE(p->report.reboundByName.empty());

    // Every preamble entry is at=0/seq=0/origin=Preamble -- never part of the recorded schedule.
    for (const auto& f : p->preamble)
    {
        CHECK(f.at == 0.0);
        CHECK(f.seq == 0);
        CHECK(f.p.origin == Origin::Preamble);
    }

    // Discrete order (plan section 3.2's emission table): comp/activeDeck, comp/quantize, then
    // EVERY layer's 5 flags + activeClip (layers 1-2 have nothing captured, so their flags read the
    // live-model defaults) -- ALL of that before layer 0's captured clip's play/pause, which is a
    // separate, later pass (R4: the trigger must precede the play/pause it could otherwise auto-play
    // over).
    REQUIRE(p->preamble.size() == 21);
    size_t i = 0;
    CHECK(p->preamble[i].key.control == "activeDeck");   CHECK(p->preamble[i++].p.v == 0);
    CHECK(p->preamble[i].key.control == "quantize");     CHECK(p->preamble[i++].p.v == 2);

    auto checkLayerFlags = [&](int layerIdx, bool visible, bool bypass, bool solo, bool mute,
                                bool autopilot, int activeClip)
    {
        CHECK(p->preamble[i].key.layer == layerIdx);
        CHECK(p->preamble[i].key.control == "visible");   CHECK(p->preamble[i++].p.v == (visible ? 1 : 0));
        CHECK(p->preamble[i].key.control == "bypass");    CHECK(p->preamble[i++].p.v == (bypass ? 1 : 0));
        CHECK(p->preamble[i].key.control == "solo");      CHECK(p->preamble[i++].p.v == (solo ? 1 : 0));
        CHECK(p->preamble[i].key.control == "mute");      CHECK(p->preamble[i++].p.v == (mute ? 1 : 0));
        CHECK(p->preamble[i].key.control == "autopilot"); CHECK(p->preamble[i++].p.v == (autopilot ? 1 : 0));
        CHECK(p->preamble[i].key.control == "activeClip"); CHECK(p->preamble[i++].p.v == activeClip);
    };
    checkLayerFlags(0, false, false, true, false, false, 2);
    checkLayerFlags(1, true, false, false, false, false, -1);
    checkLayerFlags(2, true, false, false, false, false, -1);

    REQUIRE(i + 1 == p->preamble.size());
    CHECK(p->preamble[i].key.control == "playing");
    CHECK(p->preamble[i].key.col == 2);
    CHECK(p->preamble[i].key.layer == 0);
    CHECK(p->preamble[i].p.action == "resume");

    // Continuous order: EVERY layer's opacity (0, 1, 2 -- s-rta-0925 rr-fix: a checkpoint opacity
    // equal to the scalar default is NOT skipped; the live value at Play time may have since
    // diverged from default), then the captured clip's fx param, then its scalar.
    REQUIRE(p->preambleContinuous.size() == 5);
    CHECK(p->preambleContinuous[0].key.scope == ControlPath::Scope::Layer);
    CHECK(p->preambleContinuous[0].key.control == "scalar");
    CHECK(p->preambleContinuous[0].key.scalar == "opacity");
    CHECK(p->preambleContinuous[0].key.layer == 0);
    CHECK(p->preambleContinuous[0].v == Approx(0.5f));

    CHECK(p->preambleContinuous[1].key.scope == ControlPath::Scope::Layer);
    CHECK(p->preambleContinuous[1].key.scalar == "opacity");
    CHECK(p->preambleContinuous[1].key.layer == 1);
    CHECK(p->preambleContinuous[1].v == Approx(1.0f));

    CHECK(p->preambleContinuous[2].key.scope == ControlPath::Scope::Layer);
    CHECK(p->preambleContinuous[2].key.scalar == "opacity");
    CHECK(p->preambleContinuous[2].key.layer == 2);
    CHECK(p->preambleContinuous[2].v == Approx(1.0f));

    CHECK(p->preambleContinuous[3].key.scope == ControlPath::Scope::Clip);
    CHECK(p->preambleContinuous[3].key.control == "param");
    CHECK(p->preambleContinuous[3].key.fx == 0);
    CHECK(p->preambleContinuous[3].key.param == 1);
    CHECK(p->preambleContinuous[3].v == Approx(slot.paramValues[1]));

    CHECK(p->preambleContinuous[4].key.scope == ControlPath::Scope::Clip);
    CHECK(p->preambleContinuous[4].key.control == "scalar");
    CHECK(p->preambleContinuous[4].key.scalar == "opacity");
    CHECK(p->preambleContinuous[4].v == Approx(0.25f));

    CHECK(p->report.preambleCount == static_cast<int>(p->preamble.size() + p->preambleContinuous.size()));
}

// === 2: an active clip with no ClipRuntime restores paused ===

TEST_CASE("Program::compile: an active clip with no captured ClipRuntime restores paused", "[program][preamble]")
{
    Composition comp = makeComposition();
    comp.activeDeckIndex = 0;

    Layer& layer1 = comp.decks[0].layers[1];
    layer1.activeClipColumn = 0;
    layer1.clips[0] = Clip{};   // present, but every field at its default -- PerfStateCapture will
                                // NOT capture a ClipRuntime for it (nonDefault check fails).

    Take take;
    take.checkpoint0 = capturePerfState(comp, 120.0f, "");
    auto p = compile(take, comp, DriveClock::Wall);

    REQUIRE(p->report.preambleUnresolved.empty());

    bool found = false;
    for (const auto& f : p->preamble)
    {
        if (f.key.control == "playing" && f.key.layer == 1 && f.key.col == 0)
        {
            found = true;
            CHECK(f.p.action == "pause");
        }
    }
    CHECK(found);
}

// === 3: a deck or layer that no longer exists is counted, never silently dropped ===

TEST_CASE("Program::compile: a missing deck or layer is counted in preambleUnresolved, never silently dropped",
          "[program][preamble]")
{
    Composition comp = makeComposition();   // one deck ("Deck 1"), 3 layers (indices 0-2)

    PerfState cp0;
    cp0.activeDeckIndex = 0;

    PerfState::DeckRuntime deck0RT;
    deck0RT.deck = "Deck 1";
    PerfState::LayerRuntime layer0RT;
    layer0RT.layer = "Layer 1";
    deck0RT.layers[0] = layer0RT;
    PerfState::LayerRuntime layer7RT;   // index 7 -- deck 0 only has layers 0-2
    layer7RT.layer = "Layer 8";
    deck0RT.layers[7] = layer7RT;
    cp0.decks[0] = deck0RT;

    PerfState::DeckRuntime deck9RT;   // index 3 -- comp only has one deck (index 0)
    deck9RT.deck = "Deck 9";
    PerfState::LayerRuntime someLayerRT;
    someLayerRT.layer = "Layer 1";
    deck9RT.layers[0] = someLayerRT;
    cp0.decks[3] = deck9RT;

    Take take;
    take.checkpoint0 = cp0;
    auto p = compile(take, comp, DriveClock::Wall);

    REQUIRE(p->report.preambleUnresolved.size() == 2);
    bool namesDeck = false, namesLayer = false;
    for (const auto& issue : p->report.preambleUnresolved)
    {
        if (issue.reason.find("deck") != std::string::npos) namesDeck = true;
        if (issue.reason.find("layer") != std::string::npos) namesLayer = true;
    }
    CHECK(namesDeck);
    CHECK(namesLayer);

    // deck 0's entries (the valid layer 0) are still present -- not swallowed by the two failures.
    bool foundDeck0Entry = false;
    for (const auto& f : p->preamble)
        if (f.target.deck == 0 && f.target.layer == 0)
            foundDeck0Entry = true;
    CHECK(foundDeck0Entry);
}

// === 4: rebind by name ===

TEST_CASE("Program::compile: a layer moved position but kept its name rebinds by name", "[program][preamble]")
{
    Composition comp = makeComposition();   // "Layer 2" lives at index 1 today

    PerfState cp0;
    cp0.activeDeckIndex = 0;
    PerfState::DeckRuntime deckRT;
    deckRT.deck = "Deck 1";
    PerfState::LayerRuntime layerRT;
    layerRT.layer = "Layer 2";
    deckRT.layers[5] = layerRT;   // index 5 doesn't exist; the NAME does, at index 1
    cp0.decks[0] = deckRT;

    Take take;
    take.checkpoint0 = cp0;
    auto p = compile(take, comp, DriveClock::Wall);

    REQUIRE(p->report.reboundByName.size() == 1);
    CHECK(p->report.reboundByName[0].reason.rfind("preamble: ", 0) == 0);

    bool found = false;
    for (const auto& f : p->preamble)
        if (f.key.control == "visible")
        {
            found = true;
            CHECK(f.target.layer == 1);
        }
    CHECK(found);
}

// === 5: the preamble never enters the discrete schedule ===

TEST_CASE("Program::compile: the preamble never enters the discrete lane schedule", "[program][preamble]")
{
    Composition comp = makeComposition();

    ControlPath laneKey;
    laneKey.scope = ControlPath::Scope::Layer;
    laneKey.deck = 0; laneKey.deckName = "Deck 1";
    laneKey.layer = 0; laneKey.layerName = "Layer 1";
    laneKey.control = "activeClip";

    Take take;
    take.nextSeq = 1;
    Lane lane;
    lane.key = laneKey;
    lane.kind = Lane::Kind::Discrete;
    DiscretePoint pt; pt.s = { 1, 0.5, 0 }; pt.v = 3;
    lane.points = { pt };
    take.lanes[laneKey] = lane;

    SECTION("a real checkpoint0 adds preamble entries alongside the lane -- discrete/length untouched")
    {
        take.checkpoint0.activeDeckIndex = 0;
        auto p = compile(take, comp, DriveClock::Wall);
        REQUIRE(p->discrete.size() == 1);
        CHECK(p->length == Approx(0.5));
        CHECK_FALSE(p->preamble.empty());
        for (const auto& f : p->preamble)
            CHECK(f.at == 0.0);
    }

    SECTION("R9: a v1/checkpoint-less take (default PerfState) synthesizes no preamble at all")
    {
        // take.checkpoint0 left at its struct default: activeDeckIndex == -1, decks empty.
        auto p = compile(take, comp, DriveClock::Wall);
        REQUIRE(p->discrete.size() == 1);
        CHECK(p->length == Approx(0.5));
        CHECK(p->preamble.empty());
        CHECK(p->preambleContinuous.empty());
        CHECK(p->report.preambleUnresolved.empty());
        CHECK(p->report.preambleCount == 0);
    }
}

// === 6: a checkpoint opacity equal to the scalar default still restores (rr-fix) ===
//
// s-rta-0925 rr-fix: live-gate fixture. `.harmony/probe-step3.sh`'s snap-back section loads a take
// (step3gate1.adna-take) whose checkpoint0 layer 0 is CLEAN -- activeClipColumn=-1, opacity=1.0 (the
// LayerScalar::Opacity default) -- then perturbs the LIVE composition to column 3 / opacity 0.9
// before pressing Play. Pre-fix, buildPreamble() skipped emitting a continuous restore for opacity
// whenever the CHECKPOINT value equalled the scalar default, reasoning "restoring a default is a
// no-op write" -- true only if the live value has not since diverged from default, which the
// perturbation here does on purpose. The result: activeClipColumn correctly cleared to -1 (discrete,
// never skipped), but opacity stayed stuck at the perturbed 0.9 forever, because no
// touch/set/release ever ran for it. This test pins the checkpoint-equals-default case directly:
// it does not need to simulate a live perturbation (Program::compile has no visibility into runtime
// state anyway) -- the fix is simply "always emit the continuous entry captured opacity carries",
// exactly like the five discrete flags immediately above it in emission order, so the CALLER
// (Player::firePreamble -> Sink::touch/set/release) is always given the chance to correct whatever
// the live value actually is at Play time.
TEST_CASE("Program::compile: a layer opacity captured AT the scalar default still emits a continuous restore",
          "[program][preamble][rr-fix]")
{
    Composition comp = makeComposition();
    comp.activeDeckIndex = 0;

    Layer& layer0 = comp.decks[0].layers[0];
    layer0.activeClipColumn = -1;   // clean, matches the live-gate fixture exactly
    layer0.opacity = 1.0f;          // == LayerScalar::Opacity's defaultNorm (ScalarParams.h)

    Take take;
    take.checkpoint0 = capturePerfState(comp, 120.0f, "");
    REQUIRE(take.checkpoint0.decks.at(0).layers.at(0).opacity == Approx(1.0f));   // capture is unconditional

    auto p = compile(take, comp, DriveClock::Wall);
    REQUIRE(p->report.preambleUnresolved.empty());

    bool foundOpacityRestore = false;
    for (const auto& ps : p->preambleContinuous)
    {
        if (ps.key.scope == ControlPath::Scope::Layer && ps.key.layer == 0 && ps.key.scalar == "opacity")
        {
            foundOpacityRestore = true;
            CHECK(ps.v == Approx(1.0f));
        }
    }
    // RED pre-fix: buildPreamble's "skip if norm == defaultNorm" check silently drops this entry,
    // so a perturbed-away-from-default live opacity is never corrected by Play.
    CHECK(foundOpacityRestore);

    // activeClip's -1 (clear) IS a discrete entry and was never gated on default -- confirms the
    // bug is isolated to the continuous opacity path, not a general "checkpoint == default" skip.
    bool foundActiveClipClear = false;
    for (const auto& f : p->preamble)
        if (f.key.control == "activeClip" && f.key.layer == 0)
        {
            foundActiveClipClear = true;
            CHECK(f.p.v == -1);
        }
    CHECK(foundActiveClipClear);
}
