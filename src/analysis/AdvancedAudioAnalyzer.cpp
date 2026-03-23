#include "AdvancedAudioAnalyzer.h"
#include <cmath>
#include <algorithm>
#include <numeric>

AdvancedAudioAnalyzer::AdvancedAudioAnalyzer(int numBins, float sampleRate, int fftSize, int hopSize)
    : numBins_(numBins),
      sampleRate_(sampleRate),
      fftSize_(fftSize),
      hopSize_(hopSize)
{
    hopsPerSecond_ = sampleRate_ / static_cast<float>(hopSize_);

    // Pre-compute bin ranges for each feature's frequency band
    auto freqToBin = [&](float freq) {
        return std::clamp(
            static_cast<int>(std::round(freq * static_cast<float>(fftSize_) / sampleRate_)),
            1, numBins_ - 1);
    };

    // Formant: 300-3000 Hz (vocal range)
    formantBinLow_  = freqToBin(300.0f);
    formantBinHigh_ = freqToBin(3000.0f);

    // Resonance: 200-8000 Hz
    resonanceBinLow_  = freqToBin(200.0f);
    resonanceBinHigh_ = freqToBin(8000.0f);

    // Reese bass: 30-200 Hz
    reeseBinLow_  = freqToBin(30.0f);
    reeseBinHigh_ = freqToBin(200.0f);

    // Zero init histories
    bassHistory_.fill(0.0f);
    midHistory_.fill(0.0f);
    ioiHistory_.fill(0);
}

void AdvancedAudioAnalyzer::process(const float* mag, float bassEnergy, float midEnergy,
                                     bool onsetDetected, float /*rms*/)
{
    // ==========================================
    // 1. SIDECHAIN PUMP DETECTION
    // ==========================================
    // Concept: In sidechain-compressed music, bass hits cause mid/high to duck.
    // We measure the negative correlation between bass envelope and mid envelope.
    // High pump = bass and mid are anti-correlated (negative Pearson r).
    {
        bassHistory_[static_cast<size_t>(envelopePos_)] = bassEnergy;
        midHistory_[static_cast<size_t>(envelopePos_)]  = midEnergy;
        envelopePos_ = (envelopePos_ + 1) % kEnvelopeLength;
        if (!envelopeFull_ && envelopePos_ == 0)
            envelopeFull_ = true;

        if (envelopeFull_)
        {
            // Compute Pearson correlation coefficient between bass and mid envelopes
            float sumB = 0.0f, sumM = 0.0f;
            for (int i = 0; i < kEnvelopeLength; ++i)
            {
                sumB += bassHistory_[static_cast<size_t>(i)];
                sumM += midHistory_[static_cast<size_t>(i)];
            }
            float meanB = sumB / kEnvelopeLength;
            float meanM = sumM / kEnvelopeLength;

            float covBM = 0.0f, varB = 0.0f, varM = 0.0f;
            for (int i = 0; i < kEnvelopeLength; ++i)
            {
                float db = bassHistory_[static_cast<size_t>(i)] - meanB;
                float dm = midHistory_[static_cast<size_t>(i)] - meanM;
                covBM += db * dm;
                varB  += db * db;
                varM  += dm * dm;
            }

            float denom = std::sqrt(varB * varM);
            if (denom > 1e-10f)
            {
                float r = covBM / denom;
                // Negate: negative correlation → high pump value
                // Clamp to [0, 1]: r = -1 → pump = 1, r = 0 → pump = 0, r > 0 → pump = 0
                float rawPump = std::clamp(-r, 0.0f, 1.0f);
                // EMA smoothing (attack ~100ms, release ~300ms)
                float alpha = (rawPump > sidechainSmoothed_) ? 0.15f : 0.05f;
                sidechainSmoothed_ += alpha * (rawPump - sidechainSmoothed_);
            }

            sidechainPump_ = sidechainSmoothed_;
        }
    }

    // ==========================================
    // 2. SWING DETECTION
    // ==========================================
    // Concept: In straight timing, consecutive onset intervals are equal.
    // In swung timing (shuffle), alternating intervals differ (long-short pattern).
    // Swing ratio = long / (long + short), where 0.5 = straight, 0.67 = triplet swing.
    {
        ++hopsSinceLastOnset_;

        if (onsetDetected && hopsSinceLastOnset_ > 2)  // Minimum 2 hops between onsets
        {
            ioiHistory_[static_cast<size_t>(ioiPos_)] = hopsSinceLastOnset_;
            ioiPos_ = (ioiPos_ + 1) % kIOIHistorySize;
            if (ioiCount_ < kIOIHistorySize) ++ioiCount_;
            hopsSinceLastOnset_ = 0;

            // Need at least 8 intervals to detect swing pattern
            if (ioiCount_ >= 8)
            {
                // Look for alternating long-short pattern in recent intervals.
                // Group consecutive pairs and compute ratio.
                float sumRatio = 0.0f;
                int pairCount = 0;
                for (int i = 0; i < ioiCount_ - 1; i += 2)
                {
                    int idx0 = (ioiPos_ - ioiCount_ + i + kIOIHistorySize) % kIOIHistorySize;
                    int idx1 = (idx0 + 1) % kIOIHistorySize;
                    float a = static_cast<float>(ioiHistory_[static_cast<size_t>(idx0)]);
                    float b = static_cast<float>(ioiHistory_[static_cast<size_t>(idx1)]);

                    if (a > 0.0f && b > 0.0f)
                    {
                        // Ensure 'long' is always the larger interval
                        float longer  = std::max(a, b);
                        float shorter = std::min(a, b);
                        float total = longer + shorter;
                        if (total > 0.0f)
                        {
                            sumRatio += longer / total;
                            ++pairCount;
                        }
                    }
                }

                if (pairCount > 0)
                {
                    float rawSwing = sumRatio / static_cast<float>(pairCount);
                    // EMA smoothing (slow — swing is a persistent style feature)
                    float alpha = 0.03f;
                    swingSmoothed_ += alpha * (rawSwing - swingSmoothed_);
                }
            }

            swingRatio_ = swingSmoothed_;
        }
    }

    // ==========================================
    // 3. FORMANT PRESENCE
    // ==========================================
    // Concept: Vocal content concentrates energy in 300-3000 Hz (formant region).
    // Measure the ratio of formant-band energy to total energy.
    // High ratio = likely vocal content.
    {
        float formantEnergy = 0.0f;
        float totalEnergy = 0.0f;

        for (int k = 1; k < numBins_; ++k)
        {
            float power = mag[k] * mag[k];
            totalEnergy += power;
            if (k >= formantBinLow_ && k < formantBinHigh_)
                formantEnergy += power;
        }

        float rawFormant = (totalEnergy > 1e-10f) ? formantEnergy / totalEnergy : 0.0f;

        // Adaptive normalization
        if (rawFormant > formantMax_)
            formantMax_ = rawFormant;
        else
            formantMax_ *= 0.9995f;

        float normalized = (formantMax_ > 1e-10f) ? rawFormant / formantMax_ : 0.0f;

        // EMA smoothing (medium speed — vocal presence changes over phrases)
        float alpha = 0.08f;
        formantSmoothed_ += alpha * (normalized - formantSmoothed_);
        formantPresence_ = std::clamp(formantSmoothed_, 0.0f, 1.0f);
    }

    // ==========================================
    // 4. RESONANCE PEAK TRACKING
    // ==========================================
    // Concept: Kurtosis measures how "peaky" a distribution is.
    // High kurtosis in 200-8000 Hz = sharp resonance peaks (filter sweeps, synth resonance).
    // Low kurtosis = flat/noisy spectrum.
    {
        int count = resonanceBinHigh_ - resonanceBinLow_;
        if (count > 2)
        {
            // Compute mean and variance of magnitude in the resonance band
            float sum = 0.0f;
            for (int k = resonanceBinLow_; k < resonanceBinHigh_; ++k)
                sum += mag[k];
            float mean = sum / static_cast<float>(count);

            float m2 = 0.0f, m4 = 0.0f;
            for (int k = resonanceBinLow_; k < resonanceBinHigh_; ++k)
            {
                float d = mag[k] - mean;
                float d2 = d * d;
                m2 += d2;
                m4 += d2 * d2;
            }
            m2 /= static_cast<float>(count);
            m4 /= static_cast<float>(count);

            // Kurtosis = m4 / m2^2 - 3 (excess kurtosis; 0 for Gaussian)
            // We use non-excess: m4 / m2^2 (3 for Gaussian)
            float rawKurtosis = (m2 > 1e-10f) ? m4 / (m2 * m2) : 0.0f;

            // Normalize: kurtosis of 3 = Gaussian (flat), >3 = peaky.
            // Map [3, 30] → [0, 1] with diminishing returns above ~15.
            float normalized = std::clamp((rawKurtosis - 3.0f) / 27.0f, 0.0f, 1.0f);

            // EMA smoothing (fast attack for filter sweeps, slow release)
            float alpha = (normalized > resonanceSmoothed_) ? 0.2f : 0.05f;
            resonanceSmoothed_ += alpha * (normalized - resonanceSmoothed_);
        }

        resonancePeak_ = std::clamp(resonanceSmoothed_, 0.0f, 1.0f);
    }

    // ==========================================
    // 5. REESE BASS DETECTION
    // ==========================================
    // Concept: A "reese bass" (common in DnB) has energy spread across 30-200 Hz
    // due to detuned oscillators creating beating/wobble patterns.
    // A pure sub bass concentrates energy at one frequency.
    // We measure spectral spread (std dev of energy distribution) in the bass region.
    {
        int count = reeseBinHigh_ - reeseBinLow_;
        if (count > 2)
        {
            // Compute weighted mean frequency and std dev in bass region
            float totalPower = 0.0f;
            float weightedFreq = 0.0f;

            for (int k = reeseBinLow_; k < reeseBinHigh_; ++k)
            {
                float power = mag[k] * mag[k];
                float freq = static_cast<float>(k) * sampleRate_ / static_cast<float>(fftSize_);
                totalPower += power;
                weightedFreq += freq * power;
            }

            if (totalPower > 1e-10f)
            {
                float meanFreq = weightedFreq / totalPower;

                float variance = 0.0f;
                for (int k = reeseBinLow_; k < reeseBinHigh_; ++k)
                {
                    float freq = static_cast<float>(k) * sampleRate_ / static_cast<float>(fftSize_);
                    float power = mag[k] * mag[k];
                    float d = freq - meanFreq;
                    variance += d * d * power;
                }
                variance /= totalPower;
                float spread = std::sqrt(variance); // Hz

                // Adaptive normalization
                if (spread > reeseMax_)
                    reeseMax_ = spread;
                else
                    reeseMax_ *= 0.9995f;

                float normalized = (reeseMax_ > 1e-10f) ? spread / reeseMax_ : 0.0f;

                // EMA smoothing (medium — reese bass is a sustained feature)
                float alpha = 0.06f;
                reeseSmoothed_ += alpha * (normalized - reeseSmoothed_);
            }
        }

        reeseBass_ = std::clamp(reeseSmoothed_, 0.0f, 1.0f);
    }
}
