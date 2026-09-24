#include "SpectralFeatures.h"
#include <cmath>
#include <algorithm>

// 7-band definitions from CLAUDE.md:
// Sub 20-60, Bass 60-250, LowMid 250-500, Mid 500-2k,
// HighMid 2k-4k, Presence 4k-6k, Brilliance 6k-20k Hz
// Shared by computeBandBinRanges() (bin ranges) and setInputBandwidthHz()
// (per-band coverage / validity).
static constexpr float kBandEdges[SpectralFeatures::kNumBands + 1] = {
    20.0f, 60.0f, 250.0f, 500.0f, 2000.0f, 4000.0f, 6000.0f, 20000.0f
};

SpectralFeatures::SpectralFeatures(int numBins, float sampleRate, int fftSize)
    : numBins_(numBins),
      sampleRate_(sampleRate),
      fftSize_(fftSize)
{
    prevMagnitude_.fill(0.0f);
    bandEnergies_.fill(0.0f);
    bandMaxEnergy_.fill(1e-10f);
    computeBandBinRanges();

    // R13: default bandwidth is the full Nyquist — bit-identical to pre-R13
    // behaviour until setInputBandwidthHz() is called with something narrower.
    inputBandwidthHz_ = sampleRate_ / 2.0f;
    binLimit_ = numBins_;
    bandRangesEff_ = bandRanges_;
    bandValidMask_ = 0x7F;
}

void SpectralFeatures::computeBandBinRanges()
{
    for (int b = 0; b < kNumBands; ++b)
    {
        // k = ceil(f * N / fs) for low edge, floor(f * N / fs) for high edge
        int kLow  = static_cast<int>(std::ceil(kBandEdges[b] * static_cast<float>(fftSize_) / sampleRate_));
        int kHigh = static_cast<int>(std::floor(kBandEdges[b + 1] * static_cast<float>(fftSize_) / sampleRate_));

        // Clamp to valid bin range [1, numBins-1] (skip DC bin 0)
        kLow  = std::max(1, std::min(kLow, numBins_ - 1));
        kHigh = std::max(kLow, std::min(kHigh, numBins_ - 1));

        bandRanges_[static_cast<size_t>(b)] = { kLow, kHigh + 1 }; // half-open [low, high)
    }
}

void SpectralFeatures::setInputBandwidthHz(float hz)
{
    hz = std::clamp(hz, 1.0f, sampleRate_ / 2.0f);
    inputBandwidthHz_ = hz;

    binLimit_ = std::min(numBins_,
                          static_cast<int>(std::floor(hz * static_cast<float>(fftSize_) / sampleRate_)) + 1);

    bandValidMask_ = 0;
    for (int b = 0; b < kNumBands; ++b)
    {
        auto idx = static_cast<size_t>(b);

        bandRangesEff_[idx] = bandRanges_[idx];
        bandRangesEff_[idx].high = std::min(bandRangesEff_[idx].high, binLimit_);

        float edgeLo = kBandEdges[b];
        float edgeHi = kBandEdges[b + 1];
        float coverage = std::clamp((hz - edgeLo) / (edgeHi - edgeLo), 0.0f, 1.0f);
        if (coverage >= 0.5f)
            bandValidMask_ = static_cast<uint8_t>(bandValidMask_ | static_cast<uint8_t>(1u << b));
    }
}

void SpectralFeatures::process(const float* mag)
{
    // --- Spectral Centroid ---
    // centroid = sum(f_k * M[k]) / sum(M[k]), k=1..K (K = binLimit_, gated to the input bandwidth)
    float weightedSum = 0.0f;
    float totalMag = 0.0f;
    for (int k = 1; k < binLimit_; ++k)
    {
        float freq = static_cast<float>(k) * sampleRate_ / static_cast<float>(fftSize_);
        weightedSum += freq * mag[k];
        totalMag += mag[k];
    }
    centroid_ = (totalMag > 1e-10f) ? weightedSum / totalMag : 0.0f;

    // --- Spectral Flux (half-wave rectified L2) ---
    // flux = sum(max(0, M[k] - M_prev[k])^2, k=1..K)
    if (hasPrevFrame_)
    {
        float rawFlux = 0.0f;
        for (int k = 1; k < binLimit_; ++k)
        {
            float diff = mag[k] - prevMagnitude_[static_cast<size_t>(k)];
            if (diff > 0.0f)
                rawFlux += diff * diff;
        }

        // Adaptive normalization via running max with slow decay
        if (rawFlux > fluxMax_)
            fluxMax_ = rawFlux;
        else
            fluxMax_ *= 0.9995f; // slow decay (~5 second half-life at 93.8 hops/sec)

        flux_ = (fluxMax_ > 1e-10f) ? rawFlux / fluxMax_ : 0.0f;
    }
    else
    {
        flux_ = 0.0f;
        hasPrevFrame_ = true;
    }

    // Store current magnitude for next frame's flux computation. R13 B-1:
    // stores ALL bins (not just k < binLimit_) so a bandwidth increase
    // (hot-swap to a wider device) never sees stale prevMagnitude_ above the
    // old limit — that would otherwise produce one spurious flux spike.
    for (int k = 0; k < numBins_; ++k)
        prevMagnitude_[static_cast<size_t>(k)] = mag[k];

    // --- Spectral Flatness (Wiener entropy) ---
    // flatness = exp(mean(ln(P[k]))) / mean(P[k])
    // Computed in log domain to avoid overflow
    {
        double logSum = 0.0;
        double arithSum = 0.0;
        int count = 0;
        for (int k = 1; k < binLimit_; ++k)
        {
            float p = mag[k] * mag[k]; // power spectrum
            if (p > 1e-20f)
            {
                logSum += std::log(static_cast<double>(p));
                arithSum += static_cast<double>(p);
                ++count;
            }
        }
        if (count > 0 && arithSum > 1e-20)
        {
            double logGeoMean = logSum / count;
            double arithMean = arithSum / count;
            flatness_ = static_cast<float>(std::exp(logGeoMean - std::log(arithMean)));
            flatness_ = std::clamp(flatness_, 0.0f, 1.0f);
        }
        else
        {
            flatness_ = 0.0f;
        }
    }

    // --- Spectral Rolloff (85% threshold) ---
    // Find frequency below which 85% of total magnitude resides
    {
        float target = 0.85f * totalMag;
        float cumsum = 0.0f;
        rolloff_ = std::min(sampleRate_ / 2.0f, inputBandwidthHz_); // default to Nyquist (or the input bandwidth, if narrower)
        for (int k = 1; k < binLimit_; ++k)
        {
            cumsum += mag[k];
            if (cumsum >= target)
            {
                rolloff_ = static_cast<float>(k) * sampleRate_ / static_cast<float>(fftSize_);
                break;
            }
        }
    }

    // --- 7-Band Energies ---
    // Sum magnitudes per band, normalize via running max. A band whose
    // validity bit is clear (mostly above the input bandwidth) reports 0 and
    // its running max is left untouched, so it recovers cleanly if the
    // bandwidth widens again.
    for (int b = 0; b < kNumBands; ++b)
    {
        auto idx = static_cast<size_t>(b);

        if ((bandValidMask_ & static_cast<uint8_t>(1u << b)) == 0)
        {
            bandEnergies_[idx] = 0.0f;
            continue;
        }

        float energy = 0.0f;
        for (int k = bandRangesEff_[idx].low; k < bandRangesEff_[idx].high; ++k)
            energy += mag[k] * mag[k]; // sum of squared magnitudes (power)

        energy = std::sqrt(energy); // RMS-like amplitude

        // Adaptive normalization via running max with slow decay
        if (energy > bandMaxEnergy_[idx])
            bandMaxEnergy_[idx] = energy;
        else
            bandMaxEnergy_[idx] *= 0.9995f; // slow decay

        bandEnergies_[idx] = (bandMaxEnergy_[idx] > 1e-10f)
                                 ? energy / bandMaxEnergy_[idx]
                                 : 0.0f;
    }
}
