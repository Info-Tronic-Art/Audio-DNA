#include "connect/ConnectionShaper.h"
#include "mapping/CurveTransforms.h"
#include <cmath>
#include <algorithm>

namespace
{
    constexpr float kPi = 3.14159265358979323846f;

    inline float frac(float x)
    {
        return x - std::floor(x);
    }
}

float ConnectionShaper::beatsNow(float beatPhase, uint8_t beatInBar, uint16_t barCount,
                                  uint32_t totalBarCount, bool resetPhaseOnStructural)
{
    float barsElapsed = resetPhaseOnStructural
        ? static_cast<float>(barCount)
        : static_cast<float>(totalBarCount);
    return beatPhase + static_cast<float>(beatInBar) + 4.0f * barsElapsed;
}

float ConnectionShaper::playbackXform(float continuousCycles, ConnShape::Playback pb)
{
    switch (pb)
    {
        case ConnShape::Playback::Forward:
            return frac(continuousCycles);
        case ConnShape::Playback::Backward:
            return 1.0f - frac(continuousCycles);
        case ConnShape::Playback::PingPong:
        {
            // One ping-pong period spans TWO of the caller's raw cycles: a
            // forward leg over the first, a backward leg over the second.
            float p2 = continuousCycles / 2.0f;
            float f2 = frac(p2);
            return (f2 < 0.5f) ? (2.0f * f2) : (2.0f - 2.0f * f2);
        }
    }
    return frac(continuousCycles);
}

float ConnectionShaper::lfoShapeValue(ConnSource::Lfo::Shape shape, float phase01, float pulseWidth,
                                      long cycleIndex, float& shValue, int& shCycle)
{
    switch (shape)
    {
        case ConnSource::Lfo::Shape::Sine:
            return 0.5f + 0.5f * std::sin(phase01 * 2.0f * kPi);
        case ConnSource::Lfo::Shape::SawUp:
            return phase01;
        case ConnSource::Lfo::Shape::Triangle:
            return (phase01 < 0.5f) ? (phase01 * 2.0f) : (2.0f - phase01 * 2.0f);
        case ConnSource::Lfo::Shape::Square:
            return (phase01 < pulseWidth) ? 1.0f : 0.0f;
        case ConnSource::Lfo::Shape::SampleHold:
        {
            if (static_cast<int>(cycleIndex) != shCycle)
            {
                shCycle = static_cast<int>(cycleIndex);
                // Deterministic pseudo-random per cycle -- ParamConnection::
                // State persists no RNG object, so this is reproducible
                // (same connection, same cycle index -> same value) rather
                // than a true PRNG stream. A simple integer hash (splitmix-
                // style finalizer) is enough for a visually random-looking
                // step sequence.
                uint32_t x = static_cast<uint32_t>(cycleIndex) * 2654435761u + 1u;
                x ^= x >> 15; x *= 0x85ebca6bu; x ^= x >> 13;
                shValue = static_cast<float>(x & 0xFFFFFFu) / static_cast<float>(0xFFFFFFu);
            }
            return shValue;
        }
    }
    return 0.0f;
}


float ConnectionShaper::shapeValue(const ConnShape& s, float raw01)
{
    // Steps 1-4 of the s166 spec section 2.4 pipeline, bit-identical to v1's
    // MappingEngine::processFrame (MappingEngine.cpp:219-231) at
    // inverted==false: normalize(inMin/inMax) -> curve -> invert -> RANGE.
    float range = s.inMax - s.inMin;
    float w = (range > 1e-8f) ? std::clamp((raw01 - s.inMin) / range, 0.0f, 1.0f) : 0.0f;
    float c = CurveTransforms::applyCurve(s.curve, w);
    if (s.inverted)
        c = 1.0f - c;
    return s.outMin + c * (s.outMax - s.outMin);
}

float ConnectionShaper::applySmoothing(float y, float smoothingMs, float dtSeconds, float& smoothState)
{
    if (smoothingMs <= 0.0f)
        return y;
    if (std::isnan(smoothState))
    {
        smoothState = y;
        return y;
    }
    float tau = std::max(smoothingMs / 1000.0f, 1e-6f);
    float alpha = 1.0f - std::exp(-dtSeconds / tau);
    smoothState += alpha * (y - smoothState);
    return smoothState;
}
