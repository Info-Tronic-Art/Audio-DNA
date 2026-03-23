#pragma once
#include <array>
#include <cstddef>

// Computes advanced audio analysis features from the magnitude spectrum.
// All buffers pre-allocated at construction — zero allocation in steady state.
//
// Features computed (P25):
//   - Sidechain pump: cross-correlation of bass envelope inversion vs mid amplitude
//   - Swing ratio: inter-onset interval histogram → deviation from straight timing
//   - Formant presence: spectral energy concentration in 300-3000 Hz vocal range
//   - Resonance peak: spectral kurtosis in 200-8000 Hz — sharp peaks vs flat
//   - Reese bass: spectral spread in 30-200 Hz for DnB wobble/reese detection
class AdvancedAudioAnalyzer
{
public:
    // Construct with FFT size and sample rate.
    // numBins = FFTSize / 2 + 1 (e.g., 1025 for 2048-pt FFT).
    AdvancedAudioAnalyzer(int numBins, float sampleRate, int fftSize, int hopSize);

    // Process a new magnitude spectrum + onset/amplitude data.
    // Call once per analysis hop.
    void process(const float* magnitudeSpectrum, float bassEnergy, float midEnergy,
                 bool onsetDetected, float rms);

    // --- Accessors (valid after process()) ---
    float sidechainPump()    const { return sidechainPump_; }
    float swingRatio()       const { return swingRatio_; }
    float formantPresence()  const { return formantPresence_; }
    float resonancePeak()    const { return resonancePeak_; }
    float reeseBass()        const { return reeseBass_; }

private:
    int numBins_;
    float sampleRate_;
    int fftSize_;
    int hopSize_;
    float hopsPerSecond_;

    // === Sidechain pump detection ===
    // Tracks the inverse relationship between bass envelope and mid energy.
    // In sidechain-compressed music (techno, house), the mid ducks when bass hits.
    static constexpr int kEnvelopeLength = 64;  // ~680ms at 93.75 hops/sec
    std::array<float, kEnvelopeLength> bassHistory_{};
    std::array<float, kEnvelopeLength> midHistory_{};
    int envelopePos_ = 0;
    bool envelopeFull_ = false;
    float sidechainPump_ = 0.0f;
    float sidechainSmoothed_ = 0.0f;

    // === Swing detection ===
    // Tracks inter-onset intervals and measures deviation from grid.
    static constexpr int kIOIHistorySize = 32;  // Last 32 onset intervals
    std::array<int, kIOIHistorySize> ioiHistory_{};  // In hops
    int ioiPos_ = 0;
    int ioiCount_ = 0;
    int hopsSinceLastOnset_ = 0;
    float swingRatio_ = 0.5f;  // 0.5 = straight, >0.5 = swung (first beat longer)
    float swingSmoothed_ = 0.5f;

    // === Formant presence ===
    // Spectral energy concentration in vocal range (300-3000 Hz)
    int formantBinLow_ = 0;
    int formantBinHigh_ = 0;
    float formantPresence_ = 0.0f;
    float formantSmoothed_ = 0.0f;

    // === Resonance peak tracking ===
    // Spectral kurtosis in 200-8000 Hz — measures peakedness
    int resonanceBinLow_ = 0;
    int resonanceBinHigh_ = 0;
    float resonancePeak_ = 0.0f;
    float resonanceSmoothed_ = 0.0f;

    // === Reese bass detection ===
    // Spectral spread (standard deviation of energy) in 30-200 Hz
    int reeseBinLow_ = 0;
    int reeseBinHigh_ = 0;
    float reeseBass_ = 0.0f;
    float reeseSmoothed_ = 0.0f;

    // Running max for adaptive normalization
    float formantMax_ = 1e-10f;
    float reeseMax_ = 1e-10f;
};
