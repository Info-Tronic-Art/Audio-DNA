#pragma once
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <cstddef>

// Wraps juce::dsp::FFT (order 11 = 2048-point) with a Hann window.
// Produces a magnitude spectrum of 1025 bins (N/2 + 1).
// All buffers are pre-allocated at construction — zero allocation in steady state.
class FFTProcessor
{
public:
    static constexpr int kFFTOrder = 11;
    static constexpr int kFFTSize = 1 << kFFTOrder;          // 2048
    static constexpr int kNumBins = (kFFTSize / 2) + 1;      // 1025

    FFTProcessor();

    // Compute magnitude spectrum from a time-domain block.
    // input must have at least kFFTSize samples.
    // After calling, magnitudeSpectrum() returns the result.
    void process(const float* input, int numSamples);

    // Access the last computed magnitude spectrum (kNumBins floats).
    const float* magnitudeSpectrum() const { return magnitudeSpectrum_.data(); }

    // Access the raw FFT output (complex interleaved, kFFTSize * 2 floats).
    // Useful for phase-dependent algorithms.
    const float* rawFFTData() const { return fftData_.data(); }

    int fftSize() const { return kFFTSize; }
    int numBins() const { return kNumBins; }

private:
    void buildHannWindow();

    juce::dsp::FFT fft_;

    // Pre-allocated work buffers
    alignas(64) std::array<float, kFFTSize>     windowedInput_{};
    alignas(64) std::array<float, kFFTSize * 2> fftData_{};       // complex interleaved output
    alignas(64) std::array<float, kNumBins>     magnitudeSpectrum_{};
    alignas(64) std::array<float, kFFTSize>     hannWindow_{};
};
