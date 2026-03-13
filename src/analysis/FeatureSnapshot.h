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

    // Spectral
    float spectralCentroid  = 0.0f;  // Hz — center of mass of spectrum
    float spectralFlux      = 0.0f;  // Normalized — frame-to-frame spectral change
    float spectralFlatness  = 0.0f;  // [0, 1] — tonal vs noisy (Wiener entropy)
    float spectralRolloff   = 0.0f;  // Hz — frequency below 85% energy

    // 7-Band Energies (normalized)
    // Sub(20-60), Bass(60-250), LowMid(250-500), Mid(500-2k),
    // HighMid(2k-4k), Presence(4k-6k), Brilliance(6k-20k)
    float bandEnergies[7] = {};

    void clear()
    {
        std::memset(this, 0, sizeof(FeatureSnapshot));
    }
};
