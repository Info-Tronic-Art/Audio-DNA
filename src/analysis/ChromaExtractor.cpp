#include "ChromaExtractor.h"
#include <cmath>
#include <algorithm>

ChromaExtractor::ChromaExtractor(int numBins, float sampleRate, int fftSize)
    : numBins_(numBins),
      sampleRate_(sampleRate),
      fftSize_(fftSize)
{
    chromagram_.fill(0.0f);
    prevChromagram_.fill(0.0f);
    binToChroma_.fill(-1);
    computeBinToChromaMap();
}

void ChromaExtractor::computeBinToChromaMap()
{
    // Pre-compute which chroma bin each FFT bin maps to.
    // Pitch class = round(12 * log2(f / 440)) mod 12, adjusted so C=0.
    // A4 = 440 Hz = pitch class 9 (A), so we offset by +3 to make C=0:
    //   pitchClass = (round(12 * log2(f / 440)) + 9) mod 12
    // But more directly: semitone from C0 = 12 * log2(f / C0_freq)
    // where C0 ~ 16.3516 Hz. Then pitchClass = round(semitone) mod 12.

    static constexpr float kC0Freq = 16.3515978312874f; // C0 in Hz
    static constexpr float kMinFreq = 65.0f;  // C2, skip bins below this

    for (int k = 1; k < numBins_; ++k)
    {
        float freq = static_cast<float>(k) * sampleRate_ / static_cast<float>(fftSize_);

        if (freq < kMinFreq)
        {
            binToChroma_[static_cast<size_t>(k)] = -1;
            continue;
        }

        // Semitones above C0
        float semitones = 12.0f * std::log2(freq / kC0Freq);
        int pitchClass = static_cast<int>(std::round(semitones)) % 12;
        if (pitchClass < 0)
            pitchClass += 12;

        binToChroma_[static_cast<size_t>(k)] = pitchClass;
    }
}

void ChromaExtractor::process(const float* mag)
{
    // --- Chroma accumulation ---
    // Accumulate magnitude squared into pitch class bins
    chromagram_.fill(0.0f);

    for (int k = 1; k < numBins_; ++k)
    {
        int pc = binToChroma_[static_cast<size_t>(k)];
        if (pc < 0)
            continue;

        float m = mag[k];
        chromagram_[static_cast<size_t>(pc)] += m * m;
    }

    // --- Normalize so sum = 1.0 ---
    float total = 0.0f;
    for (int i = 0; i < kNumChroma; ++i)
        total += chromagram_[static_cast<size_t>(i)];

    if (total > 1e-10f)
    {
        float invTotal = 1.0f / total;
        for (int i = 0; i < kNumChroma; ++i)
            chromagram_[static_cast<size_t>(i)] *= invTotal;
    }

    // --- HCDF: Euclidean distance between current and previous chroma ---
    if (hasPrevFrame_)
    {
        float sumSq = 0.0f;
        for (int i = 0; i < kNumChroma; ++i)
        {
            float diff = chromagram_[static_cast<size_t>(i)]
                       - prevChromagram_[static_cast<size_t>(i)];
            sumSq += diff * diff;
        }
        hcdf_ = std::sqrt(sumSq);
    }
    else
    {
        hcdf_ = 0.0f;
        hasPrevFrame_ = true;
    }

    // Store current chroma for next frame's HCDF
    prevChromagram_ = chromagram_;
}
