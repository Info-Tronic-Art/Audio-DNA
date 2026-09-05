#pragma once
#include "connect/ParamConnection.h"
#include <vector>
#include <utility>
#include <cstdint>

// Pure, stateless shaping math shared by ConnectionEngine and its unit
// tests -- the connection world's equivalent of mapping/CurveTransforms.h
// (s166 spec section 1: "the proven math from v1 kept as pure functions").
// No ParamConnection mutation and no grip/timing decisions here: callers
// (ConnectionEngine::evaluate, or a test) own all mutable state and pass it
// in/out explicitly.
namespace ConnectionShaper
{
    // beatPhase + beatInBar + 4*barCount, folded across bars so a cycle
    // longer than one bar actually completes instead of retracing a
    // fraction of itself forever (s166 spec section 2.4). Matches the fix
    // already shipped in OscillatorSignal.h / EnvelopeSignal.h (S166-L1).
    float beatsNow(float beatPhase, uint8_t beatInBar, uint16_t barCount);

    // Maps a continuous, ever-increasing "cycles elapsed" counter (NOT
    // wrapped to [0,1) by the caller) to a position in [0,1) per Playback
    // direction. Forward/Backward each read one cycleBeats-long pass;
    // PingPong reads TWO (a forward leg then a backward leg), so one full
    // ping-pong period is 2x cycleBeats -- an inferred resolution of the
    // s166 spec's underspecified "p_cycles" term (section 2.4); see the
    // s167-l2 report FINDINGS for the reasoning.
    float playbackXform(float continuousCycles, ConnShape::Playback pb);

    // Lfo::Shape wave functions given a [0,1) phase already run through
    // playbackXform. shValue/shCycle (Sample & Hold memory) are caller-owned
    // (ParamConnection::State), so this stays a pure function of its inputs.
    float lfoShapeValue(ConnSource::Lfo::Shape shape, float phase01, float pulseWidth,
                        long cycleIndex, float& shValue, int& shCycle);

    // Piecewise-linear interpolation over Envelope::points (sorted by
    // position); empty -> 0.0f, clamps to the first/last point outside range.
    float envelopeValue(const std::vector<std::pair<float, float>>& points, float pos01);

    // normalize(inMin/inMax) -> curve -> invert -> range (RANGE). No
    // smoothing (needs persistent EMA memory; see applySmoothing). Bit-
    // identical to v1's MappingEngine::processFrame pipeline at
    // inverted==false (s166 spec section 2.5's D9 parity claim;
    // tests/test_mapping_engine.cpp:463-525).
    float shapeValue(const ConnShape& s, float raw01);

    // EMA smoothing parameterized in MILLISECONDS (not v1's alpha-per-tick):
    // alpha = 1 - exp(-dtSeconds / (smoothingMs/1000)). smoothingMs <= 0 is
    // a passthrough that leaves smoothState untouched. smoothState == NAN
    // means "uninitialized" (matches ParamConnection::State::smooth's
    // default) -- the first sample passes through and seeds it, mirroring
    // features/Smoother.h's own first-sample behavior.
    float applySmoothing(float y, float smoothingMs, float dtSeconds, float& smoothState);
}
