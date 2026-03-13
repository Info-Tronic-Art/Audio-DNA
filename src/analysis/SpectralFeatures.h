#pragma once
#include <array>
#include <cstddef>

// Computes spectral features from the magnitude spectrum produced by FFTProcessor.
// All buffers are pre-allocated at construction — zero allocation in steady state.
//
// Features computed:
//   - Spectral centroid (Hz): center of mass of the spectrum
//   - Spectral flux (normalized): half-wave rectified frame-to-frame magnitude change
//   - Spectral flatness [0, 1]: Wiener entropy (tonal vs noisy)
//   - Spectral rolloff (Hz): frequency below which 85% of energy resides
//   - 7-band energies (normalized): Sub/Bass/LowMid/Mid/HighMid/Presence/Brilliance
class SpectralFeatures
{
public:
    static constexpr int kNumBands = 7;

    // Construct with the FFT size and sample rate used by FFTProcessor.
    // numBins = FFTSize / 2 + 1 (e.g., 1025 for 2048-pt FFT).
    SpectralFeatures(int numBins, float sampleRate, int fftSize);

    // Process a new magnitude spectrum. Must have numBins elements.
    // Call once per analysis hop.
    void process(const float* magnitudeSpectrum);

    // --- Accessors (valid after process()) ---
    float centroid()  const { return centroid_; }
    float flux()      const { return flux_; }
    float flatness()  const { return flatness_; }
    float rolloff()   const { return rolloff_; }

    // 7-band energies, normalized to [0, 1] range.
    const float* bandEnergies() const { return bandEnergies_.data(); }

private:
    void computeBandBinRanges();

    int numBins_;
    float sampleRate_;
    int fftSize_;

    // Results
    float centroid_  = 0.0f;
    float flux_      = 0.0f;
    float flatness_  = 0.0f;
    float rolloff_   = 0.0f;
    std::array<float, kNumBands> bandEnergies_{};

    // Previous magnitude spectrum for spectral flux (half-wave rectified)
    std::array<float, 4096> prevMagnitude_{};  // sized for up to 4096 bins
    bool hasPrevFrame_ = false;

    // Band bin ranges [low, high) for each of the 7 bands
    struct BandRange { int low; int high; };
    std::array<BandRange, kNumBands> bandRanges_{};

    // Running max for band energy normalization (EMA-based)
    std::array<float, kNumBands> bandMaxEnergy_{};

    // Running max for flux normalization
    float fluxMax_ = 0.0f;
};
