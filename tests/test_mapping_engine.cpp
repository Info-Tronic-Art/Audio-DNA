#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "mapping/MappingEngine.h"
#include "mapping/MappingTypes.h"
#include "mapping/CurveTransforms.h"
#include "analysis/FeatureSnapshot.h"
#include "effects/Effect.h"
#include "effects/EffectChain.h"
#include <algorithm>
#include <cmath>
#include <vector>

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

// Helper: populate an EffectChain with one effect that has N params.
// Takes the chain by reference (rather than returning by value) because
// EffectChain is non-movable (it owns a mutex — see EffectChain.h) as well
// as non-copyable (effects_ holds unique_ptr<Effect>).
static void makeChainWithEffect(EffectChain& chain, int numParams)
{
    auto effect = std::make_unique<Effect>("TestEffect", "test", "test_shader");
    for (int i = 0; i < numParams; ++i)
        effect->addParam("p" + std::to_string(i), "u_test_p" + std::to_string(i), 0.0f);
    chain.addEffect(std::move(effect));
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
    EffectChain chain;
    makeChainWithEffect(chain, 1);

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
    EffectChain chain;
    makeChainWithEffect(chain, 1);

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
    EffectChain chain;
    makeChainWithEffect(chain, 1);

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
    EffectChain chain;
    makeChainWithEffect(chain, 1);

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
    EffectChain chain;
    makeChainWithEffect(chain, 1);

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
    EffectChain chain;
    makeChainWithEffect(chain, 1);

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
    EffectChain chain;
    makeChainWithEffect(chain, 1);

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
    EffectChain chain;
    makeChainWithEffect(chain, 1);

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
    EffectChain chain;
    makeChainWithEffect(chain, 1);

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
    EffectChain chain;
    makeChainWithEffect(chain, 1);

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
    EffectChain chain;
    makeChainWithEffect(chain, 1);

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
    EffectChain chain;
    makeChainWithEffect(chain, 1);

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
    EffectChain chain;
    makeChainWithEffect(chain, 1);

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

// ============================================================
// Integration test: RMS → Ripple intensity with exponential curve
// Simulates playing audio with varying RMS over time and verifies
// the mapping output follows x^2 behavior.
// ============================================================

TEST_CASE("Integration: RMS → Ripple intensity with exponential curve", "[integration][mapping]")
{
    // Create a chain mimicking the real Ripple effect (3 params: intensity, freq, speed)
    EffectChain chain;
    auto ripple = std::make_unique<Effect>("Ripple", "warp", "ripple");
    ripple->addParam("intensity", "u_ripple_intensity", 0.0f);
    ripple->addParam("freq",      "u_ripple_freq",      0.5f);
    ripple->addParam("speed",     "u_ripple_speed",     0.5f);
    chain.addEffect(std::move(ripple));

    MappingEngine engine;

    // RMS → Ripple intensity, exponential curve, no smoothing
    Mapping m;
    m.source = MappingSource::RMS;
    m.targetEffectId = 0;
    m.targetParamIndex = 0;  // intensity
    m.curve = MappingCurve::Exponential;
    m.inputMin = 0.0f;
    m.inputMax = 1.0f;
    m.outputMin = 0.0f;
    m.outputMax = 1.0f;
    m.smoothing = 1.0f;  // No smoothing for deterministic verification
    engine.addMapping(m);

    SECTION("Exponential output matches x^2 for a sweep of RMS values")
    {
        // Simulate 11 frames with RMS linearly ramping from 0.0 to 1.0
        for (int i = 0; i <= 10; ++i)
        {
            float rmsInput = static_cast<float>(i) / 10.0f;
            float expectedOutput = rmsInput * rmsInput;  // x^2

            auto snap = makeSnapshot();
            snap.rms = rmsInput;
            engine.processFrame(snap, chain);

            INFO("RMS = " << rmsInput << " → expected " << expectedOutput);
            REQUIRE(chain.getEffect(0)->getParam(0).value ==
                    Approx(expectedOutput).margin(0.001f));
        }
    }

    SECTION("Exponential curve suppresses low RMS, amplifies high RMS")
    {
        // Low RMS → very low intensity (quieter parts are suppressed)
        auto snap = makeSnapshot();
        snap.rms = 0.2f;
        engine.processFrame(snap, chain);
        float lowOutput = chain.getEffect(0)->getParam(0).value;
        REQUIRE(lowOutput == Approx(0.04f).margin(0.001f));  // 0.2^2
        REQUIRE(lowOutput < 0.2f);  // Less than linear

        // High RMS → close to full intensity (loud parts dominate)
        snap.rms = 0.9f;
        engine.processFrame(snap, chain);
        float highOutput = chain.getEffect(0)->getParam(0).value;
        REQUIRE(highOutput == Approx(0.81f).margin(0.001f));  // 0.9^2
        REQUIRE(highOutput > 0.9f * 0.9f - 0.01f);
    }

    SECTION("Unmapped params retain their defaults")
    {
        auto snap = makeSnapshot();
        snap.rms = 0.7f;
        engine.processFrame(snap, chain);

        // intensity is driven by mapping
        REQUIRE(chain.getEffect(0)->getParam(0).value == Approx(0.49f).margin(0.001f));

        // freq and speed are NOT mapped — they should keep their defaults
        // Note: processFrame only resets targeted params, so untargeted params
        // retain whatever value they had (initially set by addParam defaults)
        REQUIRE(chain.getEffect(0)->getParam(1).value == Approx(0.5f).margin(0.001f));
        REQUIRE(chain.getEffect(0)->getParam(2).value == Approx(0.5f).margin(0.001f));
    }
}

TEST_CASE("Integration: RMS → Ripple with exponential + smoothing", "[integration][mapping]")
{
    EffectChain chain;
    auto ripple = std::make_unique<Effect>("Ripple", "warp", "ripple");
    ripple->addParam("intensity", "u_ripple_intensity", 0.0f);
    chain.addEffect(std::move(ripple));

    MappingEngine engine;

    Mapping m;
    m.source = MappingSource::RMS;
    m.targetEffectId = 0;
    m.targetParamIndex = 0;
    m.curve = MappingCurve::Exponential;
    m.inputMin = 0.0f;
    m.inputMax = 1.0f;
    m.outputMin = 0.0f;
    m.outputMax = 1.0f;
    m.smoothing = 0.2f;  // Moderate smoothing
    engine.addMapping(m);

    SECTION("Smoothed output converges to exponential target")
    {
        auto snap = makeSnapshot();
        snap.rms = 0.8f;
        float target = 0.64f;  // 0.8^2

        // Run many frames — smoothed value should converge
        for (int i = 0; i < 200; ++i)
            engine.processFrame(snap, chain);

        REQUIRE(chain.getEffect(0)->getParam(0).value == Approx(target).margin(0.01f));
    }

    SECTION("Smoothed output lags behind instantaneous changes")
    {
        auto snap = makeSnapshot();

        // Initialize at 0
        snap.rms = 0.0f;
        engine.processFrame(snap, chain);

        // Jump to 1.0 — exponential output = 1.0, but smoothed should lag
        snap.rms = 1.0f;
        engine.processFrame(snap, chain);
        float afterOneFrame = chain.getEffect(0)->getParam(0).value;
        REQUIRE(afterOneFrame < 0.5f);  // Should lag significantly

        // After several frames, should be closer
        for (int i = 0; i < 20; ++i)
            engine.processFrame(snap, chain);
        float afterTwentyFrames = chain.getEffect(0)->getParam(0).value;
        REQUIRE(afterTwentyFrames > afterOneFrame);
        REQUIRE(afterTwentyFrames < 1.0f);
    }
}

TEST_CASE("Integration: simulated audio playback with varying features", "[integration][mapping]")
{
    // Set up a chain with Ripple + Hue Shift, mimicking real renderer setup
    EffectChain chain;
    {
        auto ripple = std::make_unique<Effect>("Ripple", "warp", "ripple");
        ripple->addParam("intensity", "u_ripple_intensity", 0.0f);
        ripple->addParam("freq",      "u_ripple_freq",      0.5f);
        chain.addEffect(std::move(ripple));

        auto hueShift = std::make_unique<Effect>("Hue Shift", "color", "hue_shift");
        hueShift->addParam("amount", "u_hue_shift", 0.0f);
        chain.addEffect(std::move(hueShift));
    }

    MappingEngine engine;

    // RMS → Ripple intensity (exponential)
    {
        Mapping m;
        m.source = MappingSource::RMS;
        m.targetEffectId = 0;
        m.targetParamIndex = 0;
        m.curve = MappingCurve::Exponential;
        m.smoothing = 1.0f;
        engine.addMapping(m);
    }

    // SpectralCentroid → Hue Shift (linear, normalized from Hz range)
    {
        Mapping m;
        m.source = MappingSource::SpectralCentroid;
        m.targetEffectId = 1;
        m.targetParamIndex = 0;
        m.curve = MappingCurve::Linear;
        m.inputMin = 200.0f;
        m.inputMax = 8000.0f;
        m.smoothing = 1.0f;
        engine.addMapping(m);
    }

    // Simulate 60 frames (1 second at 60fps) of "audio" with sinusoidal RMS
    // and linearly rising spectral centroid
    for (int frame = 0; frame < 60; ++frame)
    {
        auto snap = makeSnapshot();

        // Simulate RMS with a sine envelope (simulates a rhythmic pulse)
        float t = static_cast<float>(frame) / 60.0f;
        snap.rms = 0.5f + 0.5f * std::sin(2.0f * 3.14159f * t);  // [0, 1]

        // Simulate spectral centroid rising from 200Hz to 8000Hz
        snap.spectralCentroid = 200.0f + (8000.0f - 200.0f) * t;

        engine.processFrame(snap, chain);

        // Verify ripple intensity = RMS^2
        float expectedRipple = snap.rms * snap.rms;
        INFO("Frame " << frame << ": RMS=" << snap.rms);
        REQUIRE(chain.getEffect(0)->getParam(0).value ==
                Approx(expectedRipple).margin(0.001f));

        // Verify hue shift = normalized centroid
        float expectedHue = (snap.spectralCentroid - 200.0f) / (8000.0f - 200.0f);
        REQUIRE(chain.getEffect(1)->getParam(0).value ==
                Approx(expectedHue).margin(0.001f));
    }
}

// ============================================================
// W4 single-store equivalence (outputwindow-arc-design.md A3)
// ============================================================
// Reference implementation of the PRE-W4 processFrame: the exact old
// 3-pass reset→accumulate→clamp pipeline, operating on its own parallel
// Smoother set. The W4 single-store rewrite removed only the observable
// INTERMEDIATE stores (the reset-to-zero and partial sums a concurrent
// GL-thread uniform upload could catch mid-tick); end-of-tick values must
// be bit-identical to this reference for any mapping history.

namespace
{
struct ReferenceOldMappingEngine
{
    std::vector<Mapping>  mappings;
    std::vector<Smoother> smoothers;

    void add(const Mapping& m)
    {
        mappings.push_back(m);
        smoothers.emplace_back(m.smoothing);
    }

    // Verbatim shape of the pre-W4 MappingEngine::processFrame.
    void processFrameOld(const FeatureSnapshot& snapshot, EffectChain& chain)
    {
        if (mappings.empty()) return;

        // Pass 1: reset targeted params to 0 before accumulation
        for (const auto& m : mappings)
        {
            if (!m.enabled)
                continue;
            auto* effect = chain.getEffect(static_cast<int>(m.targetEffectId));
            if (effect == nullptr)
                continue;
            if (static_cast<int>(m.targetParamIndex) >= effect->getNumParams())
                continue;
            effect->getParam(static_cast<int>(m.targetParamIndex)).value = 0.0f;
        }

        // Pass 2: accumulate mapping contributions
        for (size_t i = 0; i < mappings.size(); ++i)
        {
            const auto& m = mappings[i];
            if (!m.enabled)
                continue;
            auto* effect = chain.getEffect(static_cast<int>(m.targetEffectId));
            if (effect == nullptr)
                continue;
            if (static_cast<int>(m.targetParamIndex) >= effect->getNumParams())
                continue;

            float raw = MappingEngine::extractSource(m.source, snapshot);
            float range = m.inputMax - m.inputMin;
            float normalized = (range > 1e-8f)
                ? std::clamp((raw - m.inputMin) / range, 0.0f, 1.0f)
                : 0.0f;
            float curved = MappingEngine::applyCurve(m.curve, normalized);
            float scaled = m.outputMin + curved * (m.outputMax - m.outputMin);

            if (smoothers[i].alpha() != m.smoothing)
                smoothers[i].setAlpha(m.smoothing);
            float smoothed = smoothers[i].process(scaled);

            effect->getParam(static_cast<int>(m.targetParamIndex)).value += smoothed;
        }

        // Pass 3: clamp all targeted params to [0, 1]
        for (const auto& m : mappings)
        {
            if (!m.enabled)
                continue;
            auto* effect = chain.getEffect(static_cast<int>(m.targetEffectId));
            if (effect == nullptr)
                continue;
            if (static_cast<int>(m.targetParamIndex) >= effect->getNumParams())
                continue;
            auto& param = effect->getParam(static_cast<int>(m.targetParamIndex));
            param.value = std::clamp(param.value, 0.0f, 1.0f);
        }
    }
};

// Deterministic pseudo-audio for the equivalence drive.
FeatureSnapshot equivalenceSnapshotAt(int tick)
{
    FeatureSnapshot s;
    s.clear();
    float t = static_cast<float>(tick);
    s.rms  = 0.5f + 0.5f * std::sin(t * 0.13f);
    s.peak = 0.5f + 0.5f * std::sin(t * 0.31f + 1.0f);
    s.bandEnergies[1] = 0.5f + 0.5f * std::sin(t * 0.07f + 2.0f);
    return s;
}
} // namespace

TEST_CASE("Single-store processFrame is bit-identical to the old 3-pass pipeline",
          "[mapping][singlestore]")
{
    MappingEngine engine;
    EffectChain chain;
    makeChainWithEffect(chain, 3);

    ReferenceOldMappingEngine ref;
    EffectChain refChain;
    makeChainWithEffect(refChain, 3);

    auto addBoth = [&](const Mapping& m) {
        engine.addMapping(m);
        ref.add(m);
    };

    // m0: RMS → (0,0), default smoothing — the summing group's owner
    Mapping m0;
    m0.source = MappingSource::RMS;
    m0.targetEffectId = 0;
    m0.targetParamIndex = 0;
    addBoth(m0);

    // m1: Peak → (0,0), same target (sums with m0), different alpha
    Mapping m1;
    m1.source = MappingSource::Peak;
    m1.targetEffectId = 0;
    m1.targetParamIndex = 0;
    m1.smoothing = 0.3f;
    m1.outputMax = 0.6f;
    addBoth(m1);

    // m2: BandBass → (0,1), exponential curve, own target
    Mapping m2;
    m2.source = MappingSource::BandBass;
    m2.targetEffectId = 0;
    m2.targetParamIndex = 1;
    m2.curve = MappingCurve::Exponential;
    m2.smoothing = 1.0f;
    addBoth(m2);

    // m3: invalid effect id — ignored by both, smoother must not tick
    Mapping m3;
    m3.source = MappingSource::RMS;
    m3.targetEffectId = 7;
    m3.targetParamIndex = 0;
    addBoth(m3);

    // m4: invalid param index — ignored by both
    Mapping m4;
    m4.source = MappingSource::RMS;
    m4.targetEffectId = 0;
    m4.targetParamIndex = 9;
    addBoth(m4);

    // m5: disabled — target (0,2) must never be touched
    Mapping m5;
    m5.source = MappingSource::RMS;
    m5.targetEffectId = 0;
    m5.targetParamIndex = 2;
    m5.enabled = false;
    addBoth(m5);

    // Sentinel: (0,2) starts nonzero in both chains; only a spurious store
    // (the old reset bug reappearing, or grouping gone wrong) can move it.
    chain.getEffect(0)->getParam(2).value = 0.42f;
    refChain.getEffect(0)->getParam(2).value = 0.42f;

    for (int tick = 0; tick < 300; ++tick)
    {
        // Mid-run structural mutations, applied identically to both
        if (tick == 100)
        {
            engine.getMapping(1)->enabled = false;
            ref.mappings[1].enabled = false;
        }
        if (tick == 150)
        {
            engine.getMapping(1)->enabled = true;
            ref.mappings[1].enabled = true;
        }
        if (tick == 200)
        {
            engine.getMapping(0)->smoothing = 0.8f;
            ref.mappings[0].smoothing = 0.8f;
        }

        auto snap = equivalenceSnapshotAt(tick);
        engine.processFrame(snap, chain);
        ref.processFrameOld(snap, refChain);

        INFO("tick " << tick);
        // Exact equality, not Approx: identical float operation sequences
        // must produce identical bits.
        REQUIRE(chain.getEffect(0)->getParam(0).value
                == refChain.getEffect(0)->getParam(0).value);
        REQUIRE(chain.getEffect(0)->getParam(1).value
                == refChain.getEffect(0)->getParam(1).value);
        REQUIRE(chain.getEffect(0)->getParam(2).value == 0.42f);
        REQUIRE(refChain.getEffect(0)->getParam(2).value == 0.42f);
    }
}

TEST_CASE("Single-store step response matches the old pipeline tick-for-tick (EMA parity)",
          "[mapping][singlestore]")
{
    // W7(iii) tick-domain half of the EMA parity gate: a 0→1 source step
    // must cross 63.2% of final value on the SAME tick in old and new code.
    // (C2 changes no cadence, so the tick axis IS the time axis; the
    // wall-clock half is asserted at the app gate.)
    MappingEngine engine;
    EffectChain chain;
    makeChainWithEffect(chain, 1);

    ReferenceOldMappingEngine ref;
    EffectChain refChain;
    makeChainWithEffect(refChain, 1);

    Mapping m; // default smoothing 0.15 — the Mapping struct default
    m.source = MappingSource::RMS;
    m.targetEffectId = 0;
    m.targetParamIndex = 0;
    engine.addMapping(m);
    ref.add(m);

    // Settle at 0 first so the smoother is initialized below the step
    auto zero = makeSnapshot();
    engine.processFrame(zero, chain);
    ref.processFrameOld(zero, refChain);

    auto stepped = makeSnapshot();
    stepped.rms = 1.0f;

    int newCrossTick = -1;
    int oldCrossTick = -1;
    for (int tick = 1; tick <= 60; ++tick)
    {
        engine.processFrame(stepped, chain);
        ref.processFrameOld(stepped, refChain);

        float v = chain.getEffect(0)->getParam(0).value;
        float rv = refChain.getEffect(0)->getParam(0).value;
        REQUIRE(v == rv);

        if (newCrossTick < 0 && v >= 0.632f)  newCrossTick = tick;
        if (oldCrossTick < 0 && rv >= 0.632f) oldCrossTick = tick;
    }

    // alpha 0.15: value after n ticks = 1 - 0.85^n; 63.2% falls on tick 7
    // (1 - 0.85^6 = 0.623, 1 - 0.85^7 = 0.679).
    REQUIRE(newCrossTick == 7);
    REQUIRE(oldCrossTick == 7);
}
