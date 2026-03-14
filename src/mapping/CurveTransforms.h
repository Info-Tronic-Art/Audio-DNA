#pragma once
#include <cmath>
#include <algorithm>

// Pure functions for curve transforms used by the mapping engine.
// All input values are expected to be in [0, 1] (clamped internally).

namespace CurveTransforms
{

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

} // namespace CurveTransforms
