#pragma once
#include <cstdint>
#include <cstring>

struct alignas(64) FeatureSnapshot
{
    // Timing
    uint64_t timestamp = 0;
    double   wallClockSeconds = 0.0;

    // Amplitude
    float rms  = 0.0f;
    float peak = 0.0f;

    void clear()
    {
        std::memset(this, 0, sizeof(FeatureSnapshot));
    }
};
