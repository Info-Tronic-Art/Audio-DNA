#pragma once
#include <array>
#include <cstddef>
#include <cstdint>

// Computes spectral features from the magnitude spectrum produced by FFTProcessor.
// All buffers are pre-allocated at construction — zero allocation in steady state.
//
// Features computed:
//   - Spectral centroid (Hz): center of mass of the spectrum
//   - Spectral flux (normalized): half-wave rectified frame-to-frame magnitude change
//   - Spectral flatness [0, 1]: Wiener entropy (tonal vs noisy)
//   - Spectral rolloff (Hz): frequency below which 85% of energy resides
//   - 7-band energies (normalized): Sub/Bass/LowMid/Mid/HighMid/Presence/Brilliance
//
// R13: input-bandwidth gating (non-48 kHz devices). Call setInputBandwidthHz()
// with the device Nyquist (or the resampler's inputBandwidthHz()) whenever it
// changes. Spectral statistics (centroid/flux/flatness/rolloff) are computed
// only over bins below the bandwidth; a band whose coverage of its own
// frequency range falls below the bandwidth is reported as absent (energy 0,
// its validity bit clear in bandValidMask()) instead of normalising leftover
// residue. Default bandwidth is the full Nyquist (sampleRate/2) — bit-identical
// to pre-R13 behaviour until setInputBandwidthHz() is called with something
// narrower.
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

    // R13: analysis-thread-only. Reconfigures the bin limit and per-band
    // validity mask for the given input bandwidth (Hz). Pure arithmetic, no
    // allocation. `hz` is clamped to [1, sampleRate/2].
    void setInputBandwidthHz(float hz);
    float inputBandwidthHz() const { return inputBandwidthHz_; }

    // Bit b set (b = band index, 0=Sub .. 6=Brilliance) when bandEnergies()[b]
    // is meaningful at the current input bandwidth. 0x7F = all 7 bands valid
    // (the default / 48 kHz case).
    uint8_t bandValidMask() const { return bandValidMask_; }

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

    // R13: input-bandwidth gating state.
    int binLimit_;                                        // exclusive upper bin for stats/bands (numBins_ = full Nyquist)
    std::array<BandRange, kNumBands> bandRangesEff_{};     // bandRanges_ truncated to binLimit_
    uint8_t bandValidMask_ = 0x7F;                         // bit b = band b valid at current bandwidth
    float inputBandwidthHz_;                               // current input bandwidth (Hz), default sampleRate_/2
};
