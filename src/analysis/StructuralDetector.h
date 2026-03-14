#pragma once
#include <cstdint>
#include <cmath>

// Detects structural states in music using multi-scale EMA envelopes.
// Pre-allocates all state at construction — zero allocation in steady state.
//
// Feed RMS, spectral flux, and onset rate each hop via process().
// Query structuralState() for the current state:
//   0 = normal, 1 = buildup, 2 = drop, 3 = breakdown
//
// Algorithm:
//   - Four EMA envelopes at ~100ms, ~1s, ~4s, ~16s time scales
//   - Separate envelopes for RMS and spectral flux
//   - State machine with ~200ms hysteresis to prevent flickering
class StructuralDetector
{
public:
    // sampleRate: audio sample rate in Hz
    // hopSize:    samples per call to process() (must match analysis hop, e.g. 512)
    StructuralDetector(float sampleRate, int hopSize);

    // Feed one hop's worth of computed features.
    // After this call, query structuralState().
    void process(float rms, float spectralFlux, float onsetRate);

    // --- Accessor (valid after process()) ---
    uint8_t structuralState() const { return confirmedState_; }

private:
    // EMA envelope indices
    static constexpr int kFast     = 0;  // ~100ms
    static constexpr int kMedium   = 1;  // ~1s
    static constexpr int kSlow     = 2;  // ~4s
    static constexpr int kVerySlow = 3;  // ~16s
    static constexpr int kNumScales = 4;

    // State codes
    static constexpr uint8_t kNormal    = 0;
    static constexpr uint8_t kBuildup   = 1;
    static constexpr uint8_t kDrop      = 2;
    static constexpr uint8_t kBreakdown = 3;

    // EMA smoothing alphas for each time scale
    float rmsAlpha_[kNumScales]  = {};
    float fluxAlpha_[kNumScales] = {};

    // EMA envelope values
    float rmsEnv_[kNumScales]  = {};
    float fluxEnv_[kNumScales] = {};

    // Hysteresis
    uint8_t candidateState_ = kNormal;
    uint8_t confirmedState_ = kNormal;
    int     holdCounter_    = 0;
    int     holdThreshold_  = 0;  // number of hops for ~200ms

    // Onset rate threshold for drop detection
    float onsetRateThreshold_ = 3.0f;  // onsets/sec

    // Determine raw state from current envelopes and onset rate
    uint8_t classifyState(float onsetRate) const;
};
