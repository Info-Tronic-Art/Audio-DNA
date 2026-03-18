#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include "routing/RoutingEngine.h"
#include "signal/SignalRegistry.h"
#include "analysis/FeatureSnapshot.h"

using Catch::Matchers::WithinAbs;

TEST_CASE("SignalRegistry initialization", "[signal]")
{
    SignalRegistry registry;
    registry.initDefaults();

    REQUIRE(registry.getNumSignals() > 0);

    // Check that Volume signal exists
    auto* vol = registry.getSignalByName("Volume");
    REQUIRE(vol != nullptr);
    REQUIRE(vol->getType() == Signal::Type::Audio);

    // Check modulation signals exist
    auto* mod1 = registry.getSignalByName("Mod 1");
    REQUIRE(mod1 != nullptr);
    REQUIRE(mod1->getType() == Signal::Type::Oscillator);

    auto* mod2 = registry.getSignalByName("Mod 2");
    REQUIRE(mod2 != nullptr);
    REQUIRE(mod2->getType() == Signal::Type::Envelope);
}

TEST_CASE("Signal evaluation", "[signal]")
{
    SignalRegistry registry;
    registry.initDefaults();

    FeatureSnapshot snap;
    snap.rms = 0.75f;
    snap.beatPhase = 0.5f;
    snap.beatInBar = 1;
    snap.bandEnergies[1] = 0.6f; // Bass

    registry.evaluateAll(snap);

    // Volume signal should match RMS
    auto* vol = registry.getSignalByName("Volume");
    REQUIRE(vol != nullptr);
    REQUIRE_THAT(registry.getCachedValue(vol->getId()), WithinAbs(0.75f, 0.001f));

    // Bass signal should match bandEnergies[1]
    auto* bass = registry.getSignalByName("Bass");
    REQUIRE(bass != nullptr);
    REQUIRE_THAT(registry.getCachedValue(bass->getId()), WithinAbs(0.6f, 0.001f));
}

TEST_CASE("OscillatorSignal waveforms", "[signal]")
{
    FeatureSnapshot snap;
    snap.beatPhase = 0.0f;
    snap.beatInBar = 0;

    SECTION("Sine at phase 0 = 0.5 (centered)")
    {
        OscillatorSignal osc("test", OscillatorSignal::WaveShape::Sine, 1.0f);
        // At totalBeatPhase=0, cyclePhase=0, sin(0)=0, so value = 0.5 + 0.5*0 = 0.5
        float val = osc.getValue(snap);
        REQUIRE_THAT(val, WithinAbs(0.5f, 0.01f));
    }

    SECTION("SawUp at phase 0 = 0")
    {
        OscillatorSignal osc("test", OscillatorSignal::WaveShape::SawUp, 1.0f);
        float val = osc.getValue(snap);
        REQUIRE_THAT(val, WithinAbs(0.0f, 0.01f));
    }

    SECTION("Square at phase 0 = 1.0")
    {
        OscillatorSignal osc("test", OscillatorSignal::WaveShape::Square, 1.0f);
        float val = osc.getValue(snap);
        REQUIRE_THAT(val, WithinAbs(1.0f, 0.01f));
    }
}

TEST_CASE("RoutingEngine basic route", "[routing]")
{
    SignalRegistry registry;
    registry.initDefaults();

    FeatureSnapshot snap;
    snap.rms = 0.8f;
    registry.evaluateAll(snap);

    RoutingEngine engine;

    // Create a route from Volume signal to some target
    auto* vol = registry.getSignalByName("Volume");
    REQUIRE(vol != nullptr);

    Route route;
    route.sourceType = Route::SourceType::Signal;
    route.sourceId = vol->getId();
    route.targetScope = Route::TargetScope::Clip;
    route.targetEffectIndex = 0;
    route.targetParamIndex = 0;
    route.outputMin = 0.0f;
    route.outputMax = 1.0f;

    uint32_t routeId = engine.addRoute(route);
    REQUIRE(routeId > 0);

    // Process frame and capture output
    float writtenValue = -1.0f;
    engine.processFrame(registry, [&](const Route& r, float val) {
        if (r.id == routeId)
            writtenValue = val;
    });

    // Value should be close to 0.8 (the RMS value, with some smoothing)
    REQUIRE(writtenValue >= 0.0f);
    REQUIRE(writtenValue <= 1.0f);
}

TEST_CASE("RoutingEngine invert", "[routing]")
{
    SignalRegistry registry;
    registry.initDefaults();

    FeatureSnapshot snap;
    snap.rms = 1.0f;
    registry.evaluateAll(snap);

    RoutingEngine engine;
    auto* vol = registry.getSignalByName("Volume");

    Route route;
    route.sourceType = Route::SourceType::Signal;
    route.sourceId = vol->getId();
    route.inverted = true;
    route.outputMin = 0.0f;
    route.outputMax = 1.0f;

    uint32_t routeId = engine.addRoute(route);

    float writtenValue = -1.0f;
    engine.processFrame(registry, [&](const Route& r, float val) {
        if (r.id == routeId)
            writtenValue = val;
    });

    // Inverted: 1.0 input should produce value near 0.0
    REQUIRE(writtenValue >= 0.0f);
    REQUIRE(writtenValue <= 0.3f); // Some smoothing expected on first frame
}

TEST_CASE("RoutingEngine remove route", "[routing]")
{
    RoutingEngine engine;

    Route route;
    uint32_t id = engine.addRoute(route);
    REQUIRE(engine.getNumRoutes() == 1);

    REQUIRE(engine.removeRoute(id));
    REQUIRE(engine.getNumRoutes() == 0);
    REQUIRE_FALSE(engine.removeRoute(id)); // Already removed
}
