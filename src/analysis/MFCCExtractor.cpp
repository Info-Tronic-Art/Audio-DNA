#include "MFCCExtractor.h"
#include <cmath>
#include <algorithm>

MFCCExtractor::MFCCExtractor(int numBins, float sampleRate, int fftSize)
    : numBins_(numBins),
      sampleRate_(sampleRate),
      fftSize_(fftSize)
{
    mfccs_.fill(0.0f);
    melEnergies_.fill(0.0f);
    logMelEnergies_.fill(0.0f);
    filterWeights_.fill(0.0f);
    filterWeightOffsets_.fill(0);
    dctMatrix_.fill(0.0f);

    buildFilterbank();
    buildDCTMatrix();
}

float MFCCExtractor::hzToMel(float hz)
{
    return 2595.0f * std::log10(1.0f + hz / 700.0f);
}

float MFCCExtractor::melToHz(float mel)
{
    return 700.0f * (std::pow(10.0f, mel / 2595.0f) - 1.0f);
}

void MFCCExtractor::buildFilterbank()
{
    // Create 42 mel-spaced points (40 bands + 2 edges) from 20 Hz to 8000 Hz
    static constexpr float kMinFreq = 20.0f;
    static constexpr float kMaxFreq = 8000.0f;
    static constexpr int kNumPoints = kNumMelBands + 2;  // 42

    float melMin = hzToMel(kMinFreq);
    float melMax = hzToMel(kMaxFreq);

    // Mel-spaced center frequencies
    std::array<float, kNumPoints> melPoints{};
    std::array<int, kNumPoints> binIndices{};

    for (int i = 0; i < kNumPoints; ++i)
    {
        float mel = melMin + static_cast<float>(i) * (melMax - melMin) / static_cast<float>(kNumPoints - 1);
        melPoints[static_cast<size_t>(i)] = mel;
        float hz = melToHz(mel);

        // Convert Hz to FFT bin index
        int bin = static_cast<int>(std::round(hz * static_cast<float>(fftSize_) / sampleRate_));
        bin = std::clamp(bin, 0, numBins_ - 1);
        binIndices[static_cast<size_t>(i)] = bin;
    }

    // Build triangular filters
    int weightOffset = 0;

    for (int m = 0; m < kNumMelBands; ++m)
    {
        auto mi = static_cast<size_t>(m);
        int startBin = binIndices[mi];          // left edge
        int centerBin = binIndices[mi + 1];     // peak
        int endBin = binIndices[mi + 2];        // right edge

        // Ensure at least one bin width
        if (startBin == centerBin) centerBin = startBin + 1;
        if (centerBin == endBin) endBin = centerBin + 1;

        // Clamp to valid range
        startBin = std::clamp(startBin, 0, numBins_ - 1);
        centerBin = std::clamp(centerBin, 0, numBins_ - 1);
        endBin = std::clamp(endBin, 0, numBins_);  // exclusive, can equal numBins_

        melFilters_[mi].startBin = startBin;
        melFilters_[mi].endBin = endBin;
        filterWeightOffsets_[mi] = weightOffset;

        // Rising slope: startBin to centerBin
        for (int k = startBin; k < endBin; ++k)
        {
            float weight = 0.0f;
            if (k < centerBin && centerBin > startBin)
            {
                weight = static_cast<float>(k - startBin) /
                         static_cast<float>(centerBin - startBin);
            }
            else if (k >= centerBin && endBin > centerBin)
            {
                weight = static_cast<float>(endBin - 1 - k) /
                         static_cast<float>(endBin - 1 - centerBin);
            }
            weight = std::max(0.0f, weight);

            auto idx = static_cast<size_t>(weightOffset + (k - startBin));
            if (idx < filterWeights_.size())
                filterWeights_[idx] = weight;
        }

        weightOffset += (endBin - startBin);
    }
}

void MFCCExtractor::buildDCTMatrix()
{
    // DCT-II: dct[i][j] = cos(pi * i * (j + 0.5) / N)
    // where i = 0..kNumMFCCs-1, j = 0..kNumMelBands-1, N = kNumMelBands
    static constexpr float kPi = 3.14159265358979323846f;

    for (int i = 0; i < kNumMFCCs; ++i)
    {
        for (int j = 0; j < kNumMelBands; ++j)
        {
            auto idx = static_cast<size_t>(i * kNumMelBands + j);
            dctMatrix_[idx] = std::cos(kPi * static_cast<float>(i)
                                       * (static_cast<float>(j) + 0.5f)
                                       / static_cast<float>(kNumMelBands));
        }
    }
}

void MFCCExtractor::process(const float* mag)
{
    // Step 1: Apply Mel filterbank
    for (int m = 0; m < kNumMelBands; ++m)
    {
        auto mi = static_cast<size_t>(m);
        float energy = 0.0f;
        int start = melFilters_[mi].startBin;
        int end = melFilters_[mi].endBin;
        int offset = filterWeightOffsets_[mi];

        for (int k = start; k < end; ++k)
        {
            auto wIdx = static_cast<size_t>(offset + (k - start));
            energy += mag[k] * filterWeights_[wIdx];
        }

        melEnergies_[mi] = energy;
    }

    // Step 2: Take log (with floor to avoid log(0))
    static constexpr float kLogFloor = 1e-10f;

    for (int m = 0; m < kNumMelBands; ++m)
    {
        auto mi = static_cast<size_t>(m);
        logMelEnergies_[mi] = std::log(std::max(melEnergies_[mi], kLogFloor));
    }

    // Step 3: Apply DCT-II using pre-computed matrix
    for (int i = 0; i < kNumMFCCs; ++i)
    {
        float sum = 0.0f;
        auto rowOffset = static_cast<size_t>(i * kNumMelBands);

        for (int j = 0; j < kNumMelBands; ++j)
        {
            sum += logMelEnergies_[static_cast<size_t>(j)]
                   * dctMatrix_[rowOffset + static_cast<size_t>(j)];
        }

        mfccs_[static_cast<size_t>(i)] = sum;
    }
}
