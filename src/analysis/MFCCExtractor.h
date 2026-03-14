#pragma once
#include <array>
#include <cstddef>

// Computes 13 MFCCs from an FFT magnitude spectrum.
// All buffers are pre-allocated at construction — zero allocation in steady state.
//
// Algorithm:
//   1. Apply Mel filterbank (40 triangular filters, 20–8000 Hz) to magnitude spectrum
//   2. Take log of each filter output (floored to avoid log(0))
//   3. Apply DCT-II to produce 13 MFCC coefficients
class MFCCExtractor
{
public:
    static constexpr int kNumMelBands = 40;
    static constexpr int kNumMFCCs = 13;

    // Construct with FFT parameters.
    // numBins = FFTSize / 2 + 1 (e.g., 1025 for 2048-pt FFT).
    MFCCExtractor(int numBins, float sampleRate, int fftSize);

    // Process a new magnitude spectrum. Must have numBins elements.
    // Call once per analysis hop.
    void process(const float* magnitudeSpectrum);

    // Access the last computed MFCCs (kNumMFCCs floats).
    const float* mfccs() const { return mfccs_.data(); }

private:
    static float hzToMel(float hz);
    static float melToHz(float mel);

    void buildFilterbank();
    void buildDCTMatrix();

    int numBins_;
    float sampleRate_;
    int fftSize_;

    // Results
    std::array<float, kNumMFCCs> mfccs_{};

    // Mel filterbank: for each band, store the start bin, center bin, end bin,
    // and the triangular weights for every bin in [start, end].
    // We store the weights in a flat array with index ranges per band.
    struct MelFilter
    {
        int startBin;
        int endBin;   // exclusive
    };
    std::array<MelFilter, kNumMelBands> melFilters_{};

    // Flat storage for filter weights — maximum possible is numBins per filter,
    // but typically much smaller. We store offsets into this array.
    // Max 4096 bins is generous for up to 8192-pt FFT.
    alignas(64) std::array<float, 4096 * kNumMelBands> filterWeights_{};
    std::array<int, kNumMelBands> filterWeightOffsets_{};  // offset into filterWeights_ for each band

    // Mel band energies (intermediate)
    alignas(64) std::array<float, kNumMelBands> melEnergies_{};

    // Log mel energies (intermediate)
    alignas(64) std::array<float, kNumMelBands> logMelEnergies_{};

    // Pre-computed DCT-II matrix: kNumMFCCs × kNumMelBands
    alignas(64) std::array<float, kNumMFCCs * kNumMelBands> dctMatrix_{};
};
