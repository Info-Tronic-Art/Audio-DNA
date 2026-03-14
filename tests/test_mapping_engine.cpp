#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "mapping/MappingEngine.h"
#include "mapping/MappingTypes.h"
#include "mapping/CurveTransforms.h"
#include "analysis/FeatureSnapshot.h"
#include "effects/Effect.h"
#include "effects/EffectChain.h"
#include <cmath>

using Catch::Approx;

// ============================================================
// Helper: create a zeroed snapshot with one field set
// ============================================================

static FeatureSnapshot makeSnapshot()
{
    FeatureSnapshot s;
    s.clear();
    return s;
}

// Helper: create an EffectChain with one effect that has N params
static EffectChain makeChainWithEffect(int numParams)
{
    EffectChain chain;
    auto effect = std::make_unique<Effect>("TestEffect", "test", "test_shader");
    for (int i = 0; i < numParams; ++i)
        effect->addParam("p" + std::to_string(i), "u_test_p" + std::to_string(i), 0.0f);
    chain.addEffect(std::move(effect));
    return chain;
}

// ============================================================
// CurveTransforms
// ============================================================

TEST_CASE("CurveTransforms::linear", "[curves]")
{
    REQUIRE(CurveTransforms::linear(0.0f) == Approx(0.0f));
    REQUIRE(CurveTransforms::linear(0.5f) == Approx(0.5f));
    REQUIRE(CurveTransforms::linear(1.0f) == Approx(1.0f));
    // Clamps out-of-range
    REQUIRE(CurveTransforms::linear(-0.5f) == Approx(0.0f));
    REQUIRE(CurveTransforms::linear(1.5f) == Approx(1.0f));
}

TEST_CASE("CurveTransforms::exponential", "[curves]")
{
    REQUIRE(CurveTransforms::exponential(0.0f) == Approx(0.0f));
    REQUIRE(CurveTransforms::exponential(0.5f) == Approx(0.25f));  // 0.5^2
    REQUIRE(CurveTransforms::exponential(1.0f) == Approx(1.0f));
    // Monotonically increasing
    REQUIRE(CurveTransforms::exponential(0.3f) < CurveTransforms::exponential(0.7f));
    // Below linear for x in (0, 1)
    REQUIRE(CurveTransforms::exponential(0.5f) < 0.5f);
}

TEST_CASE("CurveTransforms::logarithmic", "[curves]")
{
    REQUIRE(CurveTransforms::logarithmic(0.0f) == Approx(0.0f));
    REQUIRE(CurveTransforms::logarithmic(1.0f) == Approx(1.0f));
    // Above linear for x in (0, 1) — lifts lows
    REQUIRE(CurveTransforms::logarithmic(0.5f) > 0.5f);
    // Monotonically increasing
    REQUIRE(CurveTransforms::logarithmic(0.3f) < CurveTransforms::logarithmic(0.7f));
}

TEST_CASE("CurveTransforms::sCurve", "[curves]")
{
    REQUIRE(CurveTransforms::sCurve(0.0f) == Approx(0.0f));
    REQUIRE(CurveTransforms::sCurve(0.5f) == Approx(0.5f));  // smoothstep midpoint
    REQUIRE(CurveTransforms::sCurve(1.0f) == Approx(1.0f));
    // Below linear in first half, above in second half
    REQUIRE(CurveTransforms::sCurve(0.25f) < 0.25f);
    REQUIRE(CurveTransforms::sCurve(0.75f) > 0.75f);
}

TEST_CASE("CurveTransforms::stepped", "[curves]")
{
    // 4 steps: output is 0, 0.25, 0.5, 0.75
    REQUIRE(CurveTransforms::stepped(0.0f, 4) == Approx(0.0f));
    REQUIRE(CurveTransforms::stepped(0.24f, 4) == Approx(0.0f));
    REQUIRE(CurveTransforms::stepped(0.25f, 4) == Approx(0.25f));
    REQUIRE(CurveTransforms::stepped(0.49f, 4) == Approx(0.25f));
    REQUIRE(CurveTransforms::stepped(0.5f, 4) == Approx(0.5f));
    REQUIRE(CurveTransforms::stepped(0.99f, 4) == Approx(0.75f));
    REQUIRE(CurveTransforms::stepped(1.0f, 4) == Approx(1.0f));

    // 2 steps
    REQUIRE(CurveTransforms::stepped(0.3f, 2) == Approx(0.0f));
    REQUIRE(CurveTransforms::stepped(0.7f, 2) == Approx(0.5f));

    // Edge: 1 step
    REQUIRE(CurveTransforms::stepped(0.5f, 1) == Approx(0.0f));
    REQUIRE(CurveTransforms::stepped(1.0f, 1) == Approx(1.0f));
}

// ============================================================
// MappingEngine::applyCurve (delegates to CurveTransforms)
// ============================================================

TEST_CASE("MappingEngine::applyCurve dispatches correctly", "[mapping]")
{
    REQUIRE(MappingEngine::applyCurve(MappingCurve::Linear, 0.5f) == Approx(0.5f));
    REQUIRE(MappingEngine::applyCurve(MappingCurve::Exponential, 0.5f) == Approx(0.25f));
    REQUIRE(MappingEngine::applyCurve(MappingCurve::Logarithmic, 0.5f) ==
            Approx(CurveTransforms::logarithmic(0.5f)));
    REQUIRE(MappingEngine::applyCurve(MappingCurve::SCurve, 0.5f) == Approx(0.5f));
    REQUIRE(MappingEngine::applyCurve(MappingCurve::Stepped, 0.5f, 4) == Approx(0.5f));
}

// ============================================================
// MappingEngine::extractSource
// ============================================================

TEST_CASE("extractSource reads correct FeatureSnapshot fields", "[mapping]")
{
    auto snap = makeSnapshot();
    snap.rms = 0.42f;
    snap.peak = 0.88f;
    snap.spectralCentroid = 1500.0f;
    snap.beatPhase = 0.75f;
    snap.bandEnergies[1] = 0.6f;  // Bass
    snap.mfccs[3] = -0.5f;
    snap.chromagram[9] = 0.33f;   // A
    snap.structuralState = 2;
    snap.onsetStrength = 0.9f;

    REQUIRE(MappingEngine::extractSource(MappingSource::RMS, snap) == Approx(0.42f));
    REQUIRE(MappingEngine::extractSource(MappingSource::Peak, snap) == Approx(0.88f));
    REQUIRE(MappingEngine::extractSource(MappingSource::SpectralCentroid, snap) == Approx(1500.0f));
    REQUIRE(MappingEngine::extractSource(MappingSource::BeatPhase, snap) == Approx(0.75f));
    REQUIRE(MappingEngine::extractSource(MappingSource::BandBass, snap) == Approx(0.6f));
    REQUIRE(MappingEngine::extractSource(MappingSource::MFCC3, snap) == Approx(-0.5f));
    REQUIRE(MappingEngine::extractSource(MappingSource::ChromaA, snap) == Approx(0.33f));
    REQUIRE(MappingEngine::extractSource(MappingSource::StructuralState, snap) == Approx(2.0f));
    REQUIRE(MappingEngine::extractSource(MappingSource::OnsetStrength, snap) == Approx(0.9f));
}

// ============================================================
// Normalization (tested through processFrame)
// ============================================================

TEST_CASE("Mapping normalization scales source to [0,1]", "[mapping]")
{
    MappingEngine engine;
    auto chain = makeChainWithEffect(1);

    Mapping m;
    m.source = MappingSource::RMS;
    m.targetEffectId = 0;
    m.targetParamIndex = 0;
    m.curve = MappingCurve::Linear;
    m.inputMin = 0.2f;
    m.inputMax = 0.8f;
    m.outputMin = 0.0f;
    m.outputMax = 1.0f;
    m.smoothing = 1.0f;  // No smoothing (alpha=1 = passthrough)
    engine.addMapping(m);

    SECTION("Value at inputMin maps to outputMin")
    {
        auto snap = makeSnapshot();
        snap.rms = 0.2f;
        engine.processFrame(snap, chain);
        REQUIRE(chain.getEffect(0)->getParam(0).value == Approx(0.0f).margin(0.001f));
    }

    SECTION("Value at inputMax maps to outputMax")
    {
        auto snap = makeSnapshot();
        snap.rms = 0.8f;
        engine.processFrame(snap, chain);
        REQUIRE(chain.getEffect(0)->getParam(0).value == Approx(1.0f).margin(0.001f));
    }

    SECTION("Value at midpoint maps to 0.5")
    {
        auto snap = makeSnapshot();
        snap.rms = 0.5f;
        engine.processFrame(snap, chain);
        REQUIRE(chain.getEffect(0)->getParam(0).value == Approx(0.5f).margin(0.001f));
    }

    SECTION("Value below inputMin clamps to outputMin")
    {
        auto snap = makeSnapshot();
        snap.rms = 0.0f;
        engine.processFrame(snap, chain);
        REQUIRE(chain.getEffect(0)->getParam(0).value == Approx(0.0f).margin(0.001f));
    }

    SECTION("Value above inputMax clamps to outputMax")
    {
        auto snap = makeSnapshot();
        snap.rms = 1.0f;
        engine.processFrame(snap, chain);
        REQUIRE(chain.getEffect(0)->getParam(0).value == Approx(1.0f).margin(0.001f));
    }
}

TEST_CASE("Mapping with custom output range", "[mapping]")
{
    MappingEngine engine;
    auto chain = makeChainWithEffect(1);

    Mapping m;
    m.source = MappingSource::RMS;
    m.targetEffectId = 0;
    m.targetParamIndex = 0;
    m.curve = MappingCurve::Linear;
    m.inputMin = 0.0f;
    m.inputMax = 1.0f;
    m.outputMin = 0.3f;
    m.outputMax = 0.7f;
    m.smoothing = 1.0f;
    engine.addMapping(m);

    auto snap = makeSnapshot();
    snap.rms = 0.5f;
    engine.processFrame(snap, chain);
    // 0.5 linear → output = 0.3 + 0.5 * (0.7 - 0.3) = 0.5
    REQUIRE(chain.getEffect(0)->getParam(0).value == Approx(0.5f).margin(0.001f));

    snap.rms = 0.0f;
    engine.processFrame(snap, chain);
    REQUIRE(chain.getEffect(0)->getParam(0).value == Approx(0.3f).margin(0.001f));

    snap.rms = 1.0f;
    engine.processFrame(snap, chain);
    REQUIRE(chain.getEffect(0)->getParam(0).value == Approx(0.7f).margin(0.001f));
}

// ============================================================
// Smoothing convergence
// ============================================================

TEST_CASE("Mapping smoothing converges to target", "[mapping]")
{
    MappingEngine engine;
    auto chain = makeChainWithEffect(1);

    Mapping m;
    m.source = MappingSource::RMS;
    m.targetEffectId = 0;
    m.targetParamIndex = 0;
    m.curve = MappingCurve::Linear;
    m.inputMin = 0.0f;
    m.inputMax = 1.0f;
    m.outputMin = 0.0f;
    m.outputMax = 1.0f;
    m.smoothing = 0.3f;  // Moderate smoothing
    engine.addMapping(m);

    auto snap = makeSnapshot();
    snap.rms = 0.8f;

    // Run many frames with constant input — should converge
    for (int i = 0; i < 100; ++i)
        engine.processFrame(snap, chain);

    REQUIRE(chain.getEffect(0)->getParam(0).value == Approx(0.8f).margin(0.01f));
}

TEST_CASE("Smoothing alpha=1 gives immediate response", "[mapping]")
{
    MappingEngine engine;
    auto chain = makeChainWithEffect(1);

    Mapping m;
    m.source = MappingSource::RMS;
    m.targetEffectId = 0;
    m.targetParamIndex = 0;
    m.curve = MappingCurve::Linear;
    m.smoothing = 1.0f;  // No smoothing
    engine.addMapping(m);

    auto snap = makeSnapshot();
    snap.rms = 0.6f;
    engine.processFrame(snap, chain);
    REQUIRE(chain.getEffect(0)->getParam(0).value == Approx(0.6f).margin(0.001f));
}

TEST_CASE("Low smoothing alpha gives slow response", "[mapping]")
{
    MappingEngine engine;
    auto chain = makeChainWithEffect(1);

    Mapping m;
    m.source = MappingSource::RMS;
    m.targetEffectId = 0;
    m.targetParamIndex = 0;
    m.curve = MappingCurve::Linear;
    m.smoothing = 0.05f;  // Very heavy smoothing
    engine.addMapping(m);

    auto snap = makeSnapshot();

    // First frame with 0 to initialize the smoother
    snap.rms = 0.0f;
    engine.processFrame(snap, chain);

    // Now step to 1.0 — smoother should track slowly
    snap.rms = 1.0f;
    engine.processFrame(snap, chain);
    float afterOne = chain.getEffect(0)->getParam(0).value;
    REQUIRE(afterOne < 0.5f);

    // After 10 more frames, closer but not there yet
    for (int i = 0; i < 10; ++i)
        engine.processFrame(snap, chain);
    float afterEleven = chain.getEffect(0)->getParam(0).value;
    REQUIRE(afterEleven > afterOne);
    REQUIRE(afterEleven < 0.95f);
}

// ============================================================
// Multiple mappings to same target (summed + clamped)
// ============================================================

TEST_CASE("Multiple mappings to same parameter are summed", "[mapping]")
{
    MappingEngine engine;
    auto chain = makeChainWithEffect(1);

    // Mapping 1: RMS → param 0
    Mapping m1;
    m1.source = MappingSource::RMS;
    m1.targetEffectId = 0;
    m1.targetParamIndex = 0;
    m1.curve = MappingCurve::Linear;
    m1.smoothing = 1.0f;
    m1.outputMin = 0.0f;
    m1.outputMax = 0.3f;
    engine.addMapping(m1);

    // Mapping 2: Peak → param 0
    Mapping m2;
    m2.source = MappingSource::Peak;
    m2.targetEffectId = 0;
    m2.targetParamIndex = 0;
    m2.curve = MappingCurve::Linear;
    m2.smoothing = 1.0f;
    m2.outputMin = 0.0f;
    m2.outputMax = 0.2f;
    engine.addMapping(m2);

    auto snap = makeSnapshot();
    snap.rms = 1.0f;
    snap.peak = 1.0f;

    engine.processFrame(snap, chain);
    // 0.3 + 0.2 = 0.5
    REQUIRE(chain.getEffect(0)->getParam(0).value == Approx(0.5f).margin(0.001f));
}

TEST_CASE("Summed mappings clamp to [0,1]", "[mapping]")
{
    MappingEngine engine;
    auto chain = makeChainWithEffect(1);

    // Two mappings each outputting up to 0.7 → sum = 1.4, clamped to 1.0
    Mapping m1;
    m1.source = MappingSource::RMS;
    m1.targetEffectId = 0;
    m1.targetParamIndex = 0;
    m1.curve = MappingCurve::Linear;
    m1.smoothing = 1.0f;
    m1.outputMax = 0.7f;
    engine.addMapping(m1);

    Mapping m2;
    m2.source = MappingSource::Peak;
    m2.targetEffectId = 0;
    m2.targetParamIndex = 0;
    m2.curve = MappingCurve::Linear;
    m2.smoothing = 1.0f;
    m2.outputMax = 0.7f;
    engine.addMapping(m2);

    auto snap = makeSnapshot();
    snap.rms = 1.0f;
    snap.peak = 1.0f;

    engine.processFrame(snap, chain);
    REQUIRE(chain.getEffect(0)->getParam(0).value == Approx(1.0f).margin(0.001f));
}

// ============================================================
// Disabled mappings
// ============================================================

TEST_CASE("Disabled mapping has no effect", "[mapping]")
{
    MappingEngine engine;
    auto chain = makeChainWithEffect(1);

    Mapping m;
    m.source = MappingSource::RMS;
    m.targetEffectId = 0;
    m.targetParamIndex = 0;
    m.curve = MappingCurve::Linear;
    m.smoothing = 1.0f;
    m.enabled = false;
    engine.addMapping(m);

    auto snap = makeSnapshot();
    snap.rms = 0.8f;
    engine.processFrame(snap, chain);
    // Param stays at default (0.0)
    REQUIRE(chain.getEffect(0)->getParam(0).value == Approx(0.0f).margin(0.001f));
}

// ============================================================
// Add / remove mappings
// ============================================================

TEST_CASE("Add and remove mappings", "[mapping]")
{
    MappingEngine engine;

    Mapping m;
    int idx0 = engine.addMapping(m);
    int idx1 = engine.addMapping(m);
    REQUIRE(engine.getNumMappings() == 2);
    REQUIRE(idx0 == 0);
    REQUIRE(idx1 == 1);

    REQUIRE(engine.removeMapping(0) == true);
    REQUIRE(engine.getNumMappings() == 1);

    REQUIRE(engine.removeMapping(5) == false);  // out of range
    REQUIRE(engine.removeMapping(-1) == false);

    engine.clearAll();
    REQUIRE(engine.getNumMappings() == 0);
}

TEST_CASE("getMapping returns nullptr for invalid index", "[mapping]")
{
    MappingEngine engine;
    REQUIRE(engine.getMapping(0) == nullptr);
    REQUIRE(engine.getMapping(-1) == nullptr);
}

// ============================================================
// Curve types through full pipeline
// ============================================================

TEST_CASE("Full pipeline with exponential curve", "[mapping]")
{
    MappingEngine engine;
    auto chain = makeChainWithEffect(1);

    Mapping m;
    m.source = MappingSource::RMS;
    m.targetEffectId = 0;
    m.targetParamIndex = 0;
    m.curve = MappingCurve::Exponential;
    m.smoothing = 1.0f;
    engine.addMapping(m);

    auto snap = makeSnapshot();
    snap.rms = 0.5f;
    engine.processFrame(snap, chain);
    // 0.5^2 = 0.25
    REQUIRE(chain.getEffect(0)->getParam(0).value == Approx(0.25f).margin(0.001f));
}

TEST_CASE("Full pipeline with logarithmic curve", "[mapping]")
{
    MappingEngine engine;
    auto chain = makeChainWithEffect(1);

    Mapping m;
    m.source = MappingSource::RMS;
    m.targetEffectId = 0;
    m.targetParamIndex = 0;
    m.curve = MappingCurve::Logarithmic;
    m.smoothing = 1.0f;
    engine.addMapping(m);

    auto snap = makeSnapshot();
    snap.rms = 0.5f;
    engine.processFrame(snap, chain);
    float expected = std::log(1.0f + 9.0f * 0.5f) / std::log(10.0f);
    REQUIRE(chain.getEffect(0)->getParam(0).value == Approx(expected).margin(0.001f));
}

TEST_CASE("Full pipeline with stepped curve", "[mapping]")
{
    MappingEngine engine;
    auto chain = makeChainWithEffect(1);

    Mapping m;
    m.source = MappingSource::RMS;
    m.targetEffectId = 0;
    m.targetParamIndex = 0;
    m.curve = MappingCurve::Stepped;
    m.smoothing = 1.0f;
    engine.addMapping(m);

    auto snap = makeSnapshot();
    snap.rms = 0.6f;
    engine.processFrame(snap, chain);
    // stepped(0.6, 4) = floor(0.6*4)/4 = floor(2.4)/4 = 2/4 = 0.5
    REQUIRE(chain.getEffect(0)->getParam(0).value == Approx(0.5f).margin(0.001f));
}

// ============================================================
// Invalid target (effect or param out of range)
// ============================================================

TEST_CASE("Mapping to nonexistent effect is safely ignored", "[mapping]")
{
    MappingEngine engine;
    auto chain = makeChainWithEffect(1);

    Mapping m;
    m.source = MappingSource::RMS;
    m.targetEffectId = 99;  // No such effect
    m.targetParamIndex = 0;
    m.smoothing = 1.0f;
    engine.addMapping(m);

    auto snap = makeSnapshot();
    snap.rms = 0.5f;
    // Should not crash
    engine.processFrame(snap, chain);
    // Original param unchanged
    REQUIRE(chain.getEffect(0)->getParam(0).value == Approx(0.0f).margin(0.001f));
}

TEST_CASE("Mapping to nonexistent param is safely ignored", "[mapping]")
{
    MappingEngine engine;
    auto chain = makeChainWithEffect(1);

    Mapping m;
    m.source = MappingSource::RMS;
    m.targetEffectId = 0;
    m.targetParamIndex = 99;  // No such param
    m.smoothing = 1.0f;
    engine.addMapping(m);

    auto snap = makeSnapshot();
    snap.rms = 0.5f;
    // Should not crash
    engine.processFrame(snap, chain);
}
