#pragma once
#include <array>
#include <cstddef>

// Maps FFT magnitude bins to 12 pitch classes (C through B), normalized so sum=1.
// Also computes HCDF (Harmonic Change Detection Function) as the Euclidean distance
// between consecutive chroma frames.
//
// All buffers are pre-allocated at construction — zero allocation in steady state.
class ChromaExtractor
{
public:
    static constexpr int kNumChroma = 12;

    // Construct with FFT parameters.
    // numBins = FFTSize / 2 + 1 (e.g., 1025 for 2048-pt FFT).
    ChromaExtractor(int numBins, float sampleRate, int fftSize);

    // Process a new magnitude spectrum. Must have numBins elements.
    // Call once per analysis hop.
    void process(const float* magnitudeSpectrum);

    // --- Accessors (valid after process()) ---

    // Returns pointer to 12 floats: C=0, C#=1, D=2, ..., B=11.
    // Normalized so sum = 1.0 (or all zeros if silent).
    const float* chromagram() const { return chromagram_.data(); }

    // Harmonic Change Detection Function: Euclidean distance between
    // current and previous chroma vectors.
    float hcdf() const { return hcdf_; }

private:
    void computeBinToChromaMap();

    int numBins_;
    float sampleRate_;
    int fftSize_;

    // Results
    std::array<float, kNumChroma> chromagram_{};
    float hcdf_ = 0.0f;

    // Previous chroma for HCDF computation
    std::array<float, kNumChroma> prevChromagram_{};
    bool hasPrevFrame_ = false;

    // Pre-computed bin-to-chroma mapping: for each FFT bin, which chroma index (0-11).
    // -1 means skip this bin (e.g., DC bin or below minimum frequency).
    alignas(64) std::array<int, 4096> binToChroma_{};
};
