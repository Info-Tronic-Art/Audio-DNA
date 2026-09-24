// s-rta-0923 lane 3, Lane C0 scaffold: manualWrite's headless core, fail-
// first. .harmony/specs/s-rta-0923-lane3-plan.md sections 3.1-3.2 and 5
// (Lane C1), amended by the critic pass folded into this dispatch (findings
// #1, #3, #5). Every case below is written NOW against Lane C0's stub
// (src/connect/ManualWrite.cpp: every function returns
// std::nullopt/false/no-op) and is expected to FAIL until Lane C1
// implements it for real -- that failure is this lane's fail-first
// evidence. Case (i) is the one PIN: it round-trips Composition::
// scalarConns through toVar/fromVar and does not touch ManualWrite at all,
// so it passes on HEAD already (critic finding #5: "failing-case count ==
// total minus the one pin").
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "connect/AutomationCurve.h"
#include "connect/ParamConnection.h"
#include "connect/LiveValue.h"
#include "connect/ScalarParams.h"
#include "connect/ManualWrite.h"
#include "connect/ConnectionEngine.h"
#include "model/Composition.h"
#include "model/ControlPath.h"
#include "routing/MacroBank.h"
#include "signal/SignalRegistry.h"
#include "analysis/FeatureSnapshot.h"
#include <cmath>
#include <limits>

using Catch::Approx;

namespace
{
    // Local ControlPath builders -- deliberately NOT the MainComponent
    // helpers named in the plan (compScalarPath/layerScalarPath/
    // clipScalarPath/macroPath/clipParamPath, section 3.4): those are
    // Lane C3's, live in MainComponent.cpp, and are not reachable from a
    // headless test target. These reproduce the same positional shape.
    ControlPath compScalar(const std::string& key)
    {
        ControlPath p; p.scope = ControlPath::Scope::Comp; p.control = "scalar"; p.scalar = key; return p;
    }
    ControlPath layerScalar(int deck, int layer, const std::string& key)
    {
        ControlPath p; p.scope = ControlPath::Scope::Layer; p.deck = deck; p.layer = layer;
        p.control = "scalar"; p.scalar = key; return p;
    }
    ControlPath clipScalar(int deck, int layer, int col, const std::string& key)
    {
        ControlPath p; p.scope = ControlPath::Scope::Clip; p.deck = deck; p.layer = layer; p.col = col;
        p.control = "scalar"; p.scalar = key; return p;
    }
    ControlPath clipParam(int deck, int layer, int col, int fx, int param)
    {
        ControlPath p; p.scope = ControlPath::Scope::Clip; p.deck = deck; p.layer = layer; p.col = col;
        p.fx = fx; p.control = "param"; p.param = param; return p;
    }
    ControlPath clipDryWet(int deck, int layer, int col, int fx)
    {
        ControlPath p; p.scope = ControlPath::Scope::Clip; p.deck = deck; p.layer = layer; p.col = col;
        p.fx = fx; p.control = "dryWet"; return p;
    }
    ControlPath macroPath(int i)
    {
        ControlPath p; p.scope = ControlPath::Scope::Macro; p.control = "macro"; p.macroScope = 0; p.macro = i; return p;
    }
    ControlPath clipSpeed(int deck, int layer, int col)
    {
        ControlPath p; p.scope = ControlPath::Scope::Clip; p.deck = deck; p.layer = layer; p.col = col;
        p.control = "speed"; return p;
    }

    // A bare Clip's ClipScalar::PosX ControlRef, built by hand (not via
    // resolveControl -- cases (b)-(i) test manualWriteCore/manualReleaseCore
    // in isolation, decoupled from resolution, matching how the plan
    // describes them: only case (a) is about resolveControl itself).
    ControlRef posXRef(Clip& clip)
    {
        const auto& def = clipScalarDefs()[static_cast<size_t>(ClipScalar::PosX)];
        return ControlRef{ &clip.scalarConns[static_cast<size_t>(ClipScalar::PosX)], &clip.positionX,
                            &clip.scalarLive[static_cast<size_t>(ClipScalar::PosX)], def.toModel, def.toNorm };
    }
}

// ============================================================================
// (a) resolveControl -- every ControlPath shape the funnel must resolve, and
// three that must not.
// ============================================================================

TEST_CASE("resolveControl: every addressable control shape", "[manualwrite][resolve]")
{
    Composition comp;
    comp.initDefault();   // one deck (id 0), 3 layers (ids 0,1,2), 12 columns
    MacroBank bank;

    SECTION("comp scalar 'opacity' -> &comp.masterOpacity + &comp.scalarConns[Opacity]")
    {
        auto ref = resolveControl(comp, bank, compScalar("opacity"));
        REQUIRE(ref.has_value());
        REQUIRE(ref->manual == &comp.masterOpacity);
        REQUIRE(ref->conn == &comp.scalarConns[static_cast<size_t>(CompScalar::Opacity)]);
    }

    SECTION("layer (deck 0, layer 1) positionX -> &layer.positionX")
    {
        auto* layer = comp.decks[0].getLayer(1);
        REQUIRE(layer != nullptr);
        auto ref = resolveControl(comp, bank, layerScalar(0, 1, "positionX"));
        REQUIRE(ref.has_value());
        REQUIRE(ref->manual == &layer->positionX);
        REQUIRE(ref->conn == &layer->scalarConns[static_cast<size_t>(LayerScalar::PosX)]);
    }

    SECTION("clip (0,0,col 2) scale -> &clip.scale, toModel(0.5) == 1.0")
    {
        comp.decks[0].setClip(0, 2, Clip{});
        auto* clip = comp.decks[0].getClip(0, 2);
        REQUIRE(clip != nullptr);
        auto ref = resolveControl(comp, bank, clipScalar(0, 0, 2, "scale"));
        REQUIRE(ref.has_value());
        REQUIRE(ref->manual == &clip->scale);
        REQUIRE(ref->conn == &clip->scalarConns[static_cast<size_t>(ClipScalar::Scale)]);
        REQUIRE(ref->toModel(0.5f) == Approx(1.0f).margin(0.001f));
    }

    SECTION("clip effect param (fx 0, param 1) -> &fx.paramValues[1] / &fx.paramConns[1] after resizeParams")
    {
        Clip clip;
        Clip::EffectSlot fx;
        fx.effectName = "ripple";
        fx.paramValues = { 0.5f, 0.5f };
        fx.resizeParams(2);
        clip.effects.push_back(fx);
        comp.decks[0].setClip(0, 2, clip);
        auto* placed = comp.decks[0].getClip(0, 2);
        REQUIRE(placed != nullptr);

        auto ref = resolveControl(comp, bank, clipParam(0, 0, 2, 0, 1));
        REQUIRE(ref.has_value());
        REQUIRE(ref->manual == &placed->effects[0].paramValues[1]);
        REQUIRE(ref->conn == &placed->effects[0].paramConns[1]);
    }

    SECTION("clip dryWet (fx 0) -> &fx.dryWet / &fx.dryWetConn")
    {
        Clip clip;
        Clip::EffectSlot fx;
        fx.effectName = "ripple";
        fx.paramValues = { 0.5f };
        fx.resizeParams(1);
        clip.effects.push_back(fx);
        comp.decks[0].setClip(0, 2, clip);
        auto* placed = comp.decks[0].getClip(0, 2);
        REQUIRE(placed != nullptr);

        auto ref = resolveControl(comp, bank, clipDryWet(0, 0, 2, 0));
        REQUIRE(ref.has_value());
        REQUIRE(ref->manual == &placed->effects[0].dryWet);
        REQUIRE(ref->conn == &placed->effects[0].dryWetConn);
    }

    SECTION("source param (fx -1, param 0) -> &clip.sourceParams[0].value / .conn")
    {
        Clip clip;
        clip.sourceParams.push_back(Clip::SourceParam{});
        comp.decks[0].setClip(0, 2, clip);
        auto* placed = comp.decks[0].getClip(0, 2);
        REQUIRE(placed != nullptr);

        auto ref = resolveControl(comp, bank, clipParam(0, 0, 2, -1, 0));
        REQUIRE(ref.has_value());
        REQUIRE(ref->manual == &placed->sourceParams[0].value);
        REQUIRE(ref->conn == &placed->sourceParams[0].conn);
    }

    SECTION("macro 3 -> &bank.getMacro(3).manualValue")
    {
        auto ref = resolveControl(comp, bank, macroPath(3));
        REQUIRE(ref.has_value());
        REQUIRE(ref->manual == &bank.getMacro(3).manualValue);
        REQUIRE(ref->conn == &bank.getMacro(3).conn);
    }

    SECTION("unknown scalar key -> nullopt")
    {
        auto ref = resolveControl(comp, bank, compScalar("not_a_real_key"));
        REQUIRE_FALSE(ref.has_value());
    }

    SECTION("out-of-range col -> nullopt")
    {
        auto ref = resolveControl(comp, bank, clipScalar(0, 0, 99, "opacity"));
        REQUIRE_FALSE(ref.has_value());
    }

    SECTION("control:\"speed\" -> nullopt (Clip::speed has no ParamConnection in this lane)")
    {
        comp.decks[0].setClip(0, 2, Clip{});
        auto ref = resolveControl(comp, bank, clipSpeed(0, 0, 2));
        REQUIRE_FALSE(ref.has_value());
    }
}

// ============================================================================
// (b) an unconnected scalar writes model units through toModel.
// ============================================================================

TEST_CASE("manualWriteCore: an unconnected scalar writes through toModel", "[manualwrite]")
{
    Clip clip;
    ControlRef r = posXRef(clip);
    bool ok = manualWriteCore(r, 0.25f, Hand::HumanDecaying, ParamConnection::Grip::Kind::Decaying, 1.0, 250.0f);
    REQUIRE(ok == true);
    REQUIRE(clip.positionX == Approx((0.25f - 0.5f) * 3840.0f).margin(0.01f));
}

// ============================================================================
// (c) HumanHeld displaces a Lane grip; rank == 3.
// ============================================================================

TEST_CASE("manualWriteCore: HumanHeld displaces a Lane grip", "[manualwrite][grip]")
{
    Clip clip;
    ControlRef r = posXRef(clip);

    REQUIRE(manualWriteCore(r, 0.1f, Hand::Lane, ParamConnection::Grip::Kind::Held, 1.0, 250.0f) == true);
    REQUIRE(r.conn->grip.rank == static_cast<uint8_t>(Hand::Lane));

    REQUIRE(manualWriteCore(r, 0.2f, Hand::HumanHeld, ParamConnection::Grip::Kind::Held, 1.0, 250.0f) == true);
    REQUIRE(r.conn->grip.rank == static_cast<uint8_t>(Hand::HumanHeld));
}

// ============================================================================
// (d) a Lane write is refused while HumanHeld holds the grip (critic finding
// #5: the HumanHeld precondition is set up THROUGH the core, not by hand).
// ============================================================================

TEST_CASE("manualWriteCore: a Lane write is refused while HumanHeld holds the grip", "[manualwrite][grip]")
{
    Clip clip;
    ControlRef r = posXRef(clip);

    REQUIRE(manualWriteCore(r, 0.7f, Hand::HumanHeld, ParamConnection::Grip::Kind::Held, 1.0, 250.0f) == true);
    REQUIRE(r.conn->grip.rank == static_cast<uint8_t>(Hand::HumanHeld));
    float afterSetup = clip.positionX;

    bool ok = manualWriteCore(r, 0.1f, Hand::Lane, ParamConnection::Grip::Kind::Held, 1.0, 250.0f);
    REQUIRE(ok == false);
    REQUIRE(clip.positionX == Approx(afterSetup).margin(0.001f));
    REQUIRE(r.conn->grip.rank == static_cast<uint8_t>(Hand::HumanHeld));
}

// ============================================================================
// (e) a Lane write succeeds once a HumanDecaying grip has expired.
// ============================================================================

TEST_CASE("manualWriteCore: a Lane write succeeds once a HumanDecaying grip has expired", "[manualwrite][grip]")
{
    Clip clip;
    ControlRef r = posXRef(clip);

    REQUIRE(manualWriteCore(r, 0.6f, Hand::HumanDecaying, ParamConnection::Grip::Kind::Decaying, 1.0, 250.0f) == true);
    REQUIRE(r.conn->grip.rank == static_cast<uint8_t>(Hand::HumanDecaying));

    // 300ms later -- past the 250ms hold.
    bool ok = manualWriteCore(r, 0.2f, Hand::Lane, ParamConnection::Grip::Kind::Held, 1.3, 250.0f);
    REQUIRE(ok == true);
    REQUIRE(clip.positionX == Approx((0.2f - 0.5f) * 3840.0f).margin(0.01f));
}

// ============================================================================
// (f) a HumanDecaying write (MIDI) is refused while a slider holds
// HumanHeld (critic finding #5: setup THROUGH the core).
// ============================================================================

TEST_CASE("manualWriteCore: a HumanDecaying write is refused while HumanHeld holds the grip", "[manualwrite][grip]")
{
    Clip clip;
    ControlRef r = posXRef(clip);

    REQUIRE(manualWriteCore(r, 0.4f, Hand::HumanHeld, ParamConnection::Grip::Kind::Held, 1.0, 250.0f) == true);
    float afterSetup = clip.positionX;

    bool ok = manualWriteCore(r, 0.9f, Hand::HumanDecaying, ParamConnection::Grip::Kind::Decaying, 1.0, 250.0f);
    REQUIRE(ok == false);
    REQUIRE(clip.positionX == Approx(afterSetup).margin(0.001f));
    REQUIRE(r.conn->grip.rank == static_cast<uint8_t>(Hand::HumanHeld));
}

// ============================================================================
// (g) manualReleaseCore: a lower hand cannot release a higher hand's grip
// (critic finding #5: setup THROUGH the core).
// ============================================================================

TEST_CASE("manualReleaseCore: a lower hand cannot release a higher hand's grip; the human releases anything", "[manualwrite][grip]")
{
    Clip clip;
    ControlRef r = posXRef(clip);

    REQUIRE(manualWriteCore(r, 0.4f, Hand::HumanHeld, ParamConnection::Grip::Kind::Held, 1.0, 250.0f) == true);

    manualReleaseCore(r, Hand::Lane);
    REQUIRE(r.conn->grip.kind == ParamConnection::Grip::Kind::Held);   // survives a lower hand's release

    manualReleaseCore(r, Hand::HumanHeld);
    REQUIRE(r.conn->grip.kind == ParamConnection::Grip::Kind::None);   // the human releases anything
}

// ============================================================================
// (h) engine interplay: a HumanHeld grip suspends the connection AND leaves
// the twin at NaN (critic finding #1: reworded from "the twin is untouched"
// -- ConnectionEngine::tick's per-scalar store must always run, publishing
// NaN through, not skip the store while gripped); release starts the
// hand-back glide from the hand's last value.
// ============================================================================

TEST_CASE("manualWriteCore + ConnectionEngine::tick: a HumanHeld grip leaves the twin at NaN; release glides from the hand's value", "[manualwrite][engine]")
{
    SignalRegistry sig;
    MacroBank bank;
    FeatureSnapshot snap;
    snap.clear();

    Composition comp;
    comp.initDefault();
    comp.gripHoldMs = 250.0f;
    comp.handBackGlideMs = 100.0f;

    auto& conn = comp.scalarConns[static_cast<size_t>(CompScalar::Opacity)];
    conn.source.kind = ConnSource::Kind::Lfo;
    conn.source.lfo.cycleBeats = 4.0f;

    const auto& def = compScalarDefs()[static_cast<size_t>(CompScalar::Opacity)];
    ControlRef r{ &conn, &comp.masterOpacity, &comp.scalarLive[static_cast<size_t>(CompScalar::Opacity)],
                  def.toModel, def.toNorm };

    REQUIRE(manualWriteCore(r, 0.5f, Hand::HumanHeld, ParamConnection::Grip::Kind::Held, 1.0, comp.gripHoldMs) == true);
    REQUIRE(r.conn->grip.rank == static_cast<uint8_t>(Hand::HumanHeld));

    ConnectionEngine engine;
    ConnectionEngine::Context ctx{ sig, bank, snap, 0.016f, 1.0, comp.gripHoldMs, comp.handBackGlideMs };
    engine.tick(comp, ctx);
    REQUIRE(std::isnan(comp.scalarLive[static_cast<size_t>(CompScalar::Opacity)].v.load()));

    manualReleaseCore(r, Hand::HumanHeld);
    REQUIRE(r.conn->grip.kind == ParamConnection::Grip::Kind::None);

    ConnectionEngine::Context ctxAfter{ sig, bank, snap, 0.016f, 1.0, comp.gripHoldMs, comp.handBackGlideMs };
    engine.tick(comp, ctxAfter);
    float first = comp.scalarLive[static_cast<size_t>(CompScalar::Opacity)].v.load();
    REQUIRE_FALSE(std::isnan(first));
    REQUIRE(first == Approx(def.toNorm(comp.masterOpacity)).margin(0.05f));   // glide starts at the hand's value
}

// ============================================================================
// (i) [PIN] Composition::scalarConns round-trips through toVar/fromVar with
// the grip cleared. Does not touch ManualWrite; passes on HEAD already.
// ============================================================================

TEST_CASE("[pin] Composition scalarConns round-trip through toVar/fromVar with grip cleared", "[manualwrite][serialization][pin]")
{
    Composition comp;
    comp.initDefault();
    auto& conn = comp.scalarConns[static_cast<size_t>(CompScalar::Opacity)];
    conn.source.kind = ConnSource::Kind::Lfo;
    conn.source.lfo.shape = ConnSource::Lfo::Shape::Square;
    conn.source.lfo.cycleBeats = 2.0f;
    conn.shape.outMin = 0.1f;
    conn.shape.outMax = 0.9f;
    conn.gripHeld();   // a live human grip at save time -- must never survive the round trip

    juce::var v = comp.toVar();
    Composition loaded;
    loaded.fromVar(v);

    auto& loadedConn = loaded.scalarConns[static_cast<size_t>(CompScalar::Opacity)];
    REQUIRE(loadedConn.isConnected());
    REQUIRE(loadedConn.source.kind == ConnSource::Kind::Lfo);
    REQUIRE(loadedConn.source.lfo.shape == ConnSource::Lfo::Shape::Square);
    REQUIRE(loadedConn.source.lfo.cycleBeats == Approx(2.0f).margin(0.001f));
    REQUIRE(loadedConn.shape.outMin == Approx(0.1f).margin(0.001f));
    REQUIRE(loadedConn.shape.outMax == Approx(0.9f).margin(0.001f));
    REQUIRE(loadedConn.grip.kind == ParamConnection::Grip::Kind::None);
}
