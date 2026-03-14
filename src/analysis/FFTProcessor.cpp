#include "FFTProcessor.h"
#define _USE_MATH_DEFINES
#include <cmath>
#include <cstring>
#include <algorithm>

FFTProcessor::FFTProcessor()
    : fft_(kFFTOrder)
{
    buildHannWindow();

    windowedInput_.fill(0.0f);
    fftData_.fill(0.0f);
    magnitudeSpectrum_.fill(0.0f);
}

void FFTProcessor::buildHannWindow()
{
    const auto N = static_cast<float>(kFFTSize);
    for (int i = 0; i < kFFTSize; ++i)
    {
        // Hann window: 0.5 * (1 - cos(2π * i / N))
        hannWindow_[static_cast<size_t>(i)] =
            0.5f * (1.0f - std::cos(2.0f * static_cast<float>(M_PI) * static_cast<float>(i) / N));
    }
}

void FFTProcessor::process(const float* input, int numSamples)
{
    const int count = std::min(numSamples, kFFTSize);

    // Apply Hann window
    for (int i = 0; i < count; ++i)
    {
        windowedInput_[static_cast<size_t>(i)] =
            input[i] * hannWindow_[static_cast<size_t>(i)];
    }

    // Zero-pad if input is shorter than kFFTSize
    if (count < kFFTSize)
    {
        std::memset(windowedInput_.data() + count, 0,
                    static_cast<size_t>(kFFTSize - count) * sizeof(float));
    }

    // Copy windowed input into fftData (real parts; JUCE expects interleaved real/imag
    // for performRealOnlyForwardTransform, but actually it expects N floats followed by
    // N floats of workspace — the input is N real values in the first N slots).
    std::memcpy(fftData_.data(), windowedInput_.data(), kFFTSize * sizeof(float));
    std::memset(fftData_.data() + kFFTSize, 0, kFFTSize * sizeof(float));

    // Perform forward FFT (real-only). Output is N complex values in interleaved format.
    fft_.performRealOnlyForwardTransform(fftData_.data());

    // Compute magnitude spectrum: |X[k]| = sqrt(re^2 + im^2)
    // JUCE's real-only FFT outputs interleaved [re0, im0, re1, im1, ...] for N/2+1 bins
    for (int i = 0; i < kNumBins; ++i)
    {
        const auto idx = static_cast<size_t>(i) * 2;
        float re = fftData_[idx];
        float im = fftData_[idx + 1];
        magnitudeSpectrum_[static_cast<size_t>(i)] = std::sqrt(re * re + im * im);
    }
}
