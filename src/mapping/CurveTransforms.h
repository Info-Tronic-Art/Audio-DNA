#pragma once
#include <cmath>
#include <algorithm>

// Pure functions for curve transforms used by the mapping engine.
// All input values are expected to be in [0, 1] (clamped internally).

namespace CurveTransforms
{

// === Original curves ===

inline float linear(float x)
{
    return std::clamp(x, 0.0f, 1.0f);
}

// x^2.0 — emphasizes peaks
inline float exponential(float x)
{
    x = std::clamp(x, 0.0f, 1.0f);
    return x * x;
}

// log(1 + 9x) / log(10) — compresses peaks, lifts lows
inline float logarithmic(float x)
{
    x = std::clamp(x, 0.0f, 1.0f);
    return std::log(1.0f + 9.0f * x) / std::log(10.0f);
}

// smoothstep: x^2 * (3 - 2x) — de-emphasizes extremes
inline float sCurve(float x)
{
    x = std::clamp(x, 0.0f, 1.0f);
    return x * x * (3.0f - 2.0f * x);
}

// floor(x * N) / N — quantized to N steps
inline float stepped(float x, int n = 4)
{
    x = std::clamp(x, 0.0f, 1.0f);
    float nf = static_cast<float>(std::max(n, 1));
    return std::floor(x * nf) / nf;
}

// === Easing functions (P24) ===
// Standard animation easing curves: Elastic, Back, Bounce, Circular × In/Out/InOut + Hold

static constexpr float kPi = 3.14159265358979323846f;

// --- Circular ---
inline float circularIn(float x)
{
    x = std::clamp(x, 0.0f, 1.0f);
    return 1.0f - std::sqrt(1.0f - x * x);
}

inline float circularOut(float x)
{
    x = std::clamp(x, 0.0f, 1.0f);
    float t = x - 1.0f;
    return std::sqrt(1.0f - t * t);
}

inline float circularInOut(float x)
{
    x = std::clamp(x, 0.0f, 1.0f);
    if (x < 0.5f)
        return (1.0f - std::sqrt(1.0f - 4.0f * x * x)) * 0.5f;
    float t = 2.0f * x - 2.0f;
    return (std::sqrt(1.0f - t * t) + 1.0f) * 0.5f;
}

// --- Back (overshoots then settles) ---
static constexpr float kBackC1 = 1.70158f;
static constexpr float kBackC2 = kBackC1 * 1.525f;
static constexpr float kBackC3 = kBackC1 + 1.0f;

inline float backIn(float x)
{
    x = std::clamp(x, 0.0f, 1.0f);
    return kBackC3 * x * x * x - kBackC1 * x * x;
}

inline float backOut(float x)
{
    x = std::clamp(x, 0.0f, 1.0f);
    float t = x - 1.0f;
    return 1.0f + kBackC3 * t * t * t + kBackC1 * t * t;
}

inline float backInOut(float x)
{
    x = std::clamp(x, 0.0f, 1.0f);
    if (x < 0.5f)
    {
        float t = 2.0f * x;
        return (t * t * ((kBackC2 + 1.0f) * t - kBackC2)) * 0.5f;
    }
    float t = 2.0f * x - 2.0f;
    return (t * t * ((kBackC2 + 1.0f) * t + kBackC2) + 2.0f) * 0.5f;
}

// --- Elastic (spring oscillation) ---
static constexpr float kElasticC4 = (2.0f * kPi) / 3.0f;
static constexpr float kElasticC5 = (2.0f * kPi) / 4.5f;

inline float elasticIn(float x)
{
    x = std::clamp(x, 0.0f, 1.0f);
    if (x <= 0.0f) return 0.0f;
    if (x >= 1.0f) return 1.0f;
    return -std::pow(2.0f, 10.0f * x - 10.0f) * std::sin((x * 10.0f - 10.75f) * kElasticC4);
}

inline float elasticOut(float x)
{
    x = std::clamp(x, 0.0f, 1.0f);
    if (x <= 0.0f) return 0.0f;
    if (x >= 1.0f) return 1.0f;
    return std::pow(2.0f, -10.0f * x) * std::sin((x * 10.0f - 0.75f) * kElasticC4) + 1.0f;
}

inline float elasticInOut(float x)
{
    x = std::clamp(x, 0.0f, 1.0f);
    if (x <= 0.0f) return 0.0f;
    if (x >= 1.0f) return 1.0f;
    if (x < 0.5f)
        return -(std::pow(2.0f, 20.0f * x - 10.0f) * std::sin((20.0f * x - 11.125f) * kElasticC5)) * 0.5f;
    return (std::pow(2.0f, -20.0f * x + 10.0f) * std::sin((20.0f * x - 11.125f) * kElasticC5)) * 0.5f + 1.0f;
}

// --- Bounce ---
inline float bounceOut(float x)
{
    x = std::clamp(x, 0.0f, 1.0f);
    constexpr float n1 = 7.5625f;
    constexpr float d1 = 2.75f;
    if (x < 1.0f / d1)
        return n1 * x * x;
    if (x < 2.0f / d1)
    {
        float t = x - 1.5f / d1;
        return n1 * t * t + 0.75f;
    }
    if (x < 2.5f / d1)
    {
        float t = x - 2.25f / d1;
        return n1 * t * t + 0.9375f;
    }
    float t = x - 2.625f / d1;
    return n1 * t * t + 0.984375f;
}

inline float bounceIn(float x)
{
    x = std::clamp(x, 0.0f, 1.0f);
    return 1.0f - bounceOut(1.0f - x);
}

inline float bounceInOut(float x)
{
    x = std::clamp(x, 0.0f, 1.0f);
    if (x < 0.5f)
        return (1.0f - bounceOut(1.0f - 2.0f * x)) * 0.5f;
    return (1.0f + bounceOut(2.0f * x - 1.0f)) * 0.5f;
}

// --- Cubic (standard easing) ---
inline float cubicIn(float x)
{
    x = std::clamp(x, 0.0f, 1.0f);
    return x * x * x;
}

inline float cubicOut(float x)
{
    x = std::clamp(x, 0.0f, 1.0f);
    float t = x - 1.0f;
    return t * t * t + 1.0f;
}

inline float cubicInOut(float x)
{
    x = std::clamp(x, 0.0f, 1.0f);
    if (x < 0.5f)
        return 4.0f * x * x * x;
    float t = -2.0f * x + 2.0f;
    return 1.0f - t * t * t * 0.5f;
}

// --- Sine ---
inline float sineIn(float x)
{
    x = std::clamp(x, 0.0f, 1.0f);
    return 1.0f - std::cos(x * kPi * 0.5f);
}

inline float sineOut(float x)
{
    x = std::clamp(x, 0.0f, 1.0f);
    return std::sin(x * kPi * 0.5f);
}

inline float sineInOut(float x)
{
    x = std::clamp(x, 0.0f, 1.0f);
    return -(std::cos(kPi * x) - 1.0f) * 0.5f;
}

// --- Hold (step function: 0 until threshold, then 1) ---
inline float hold(float x)
{
    x = std::clamp(x, 0.0f, 1.0f);
    return x >= 1.0f ? 1.0f : 0.0f;
}

// --- Dispatcher: apply a curve by enum index ---
// Matches MappingCurve enum order.
inline float applyCurve(int curveIndex, float x, int steppedN = 4)
{
    switch (curveIndex)
    {
        case 0:  return linear(x);
        case 1:  return exponential(x);
        case 2:  return logarithmic(x);
        case 3:  return sCurve(x);
        case 4:  return stepped(x, steppedN);
        case 5:  return circularIn(x);
        case 6:  return circularOut(x);
        case 7:  return circularInOut(x);
        case 8:  return backIn(x);
        case 9:  return backOut(x);
        case 10: return backInOut(x);
        case 11: return elasticIn(x);
        case 12: return elasticOut(x);
        case 13: return elasticInOut(x);
        case 14: return bounceIn(x);
        case 15: return bounceOut(x);
        case 16: return bounceInOut(x);
        case 17: return cubicIn(x);
        case 18: return cubicOut(x);
        case 19: return cubicInOut(x);
        case 20: return sineIn(x);
        case 21: return sineOut(x);
        case 22: return sineInOut(x);
        case 23: return hold(x);
        default: return linear(x);
    }
}

} // namespace CurveTransforms
