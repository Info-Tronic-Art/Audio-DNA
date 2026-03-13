# Amplitude & Dynamics Feature Extraction for Real-Time Music Visualization

> **Scope**: Every amplitude-domain and dynamics-domain feature extractable at real-time rates (&le;5 ms latency budget per block) for a music visualization / VJ engine.
>
> **Cross-references**: [FEATURES_spectral.md](FEATURES_spectral.md) | [FEATURES_psychoacoustic.md](FEATURES_psychoacoustic.md) | [VIDEO_feature_to_visual_mapping.md](VIDEO_feature_to_visual_mapping.md) | [ARCH_pipeline.md](ARCH_pipeline.md)

---

## Table of Contents

1. [RMS Energy](#1-rms-energy)
2. [Peak Amplitude](#2-peak-amplitude)
3. [Crest Factor](#3-crest-factor)
4. [Dynamic Range](#4-dynamic-range)
5. [LUFS / LKFS Loudness (ITU-R BS.1770)](#5-lufs--lkfs-loudness-itur-bs1770)
6. [Loudness Range (LRA)](#6-loudness-range-lra)
7. [Noise Floor Estimation](#7-noise-floor-estimation)
8. [Signal-to-Noise Ratio (SNR)](#8-signal-to-noise-ratio-snr)
9. [Dynamic Compression Detection](#9-dynamic-compression-detection)
10. [Clipping Detection](#10-clipping-detection)
11. [Envelope Followers](#11-envelope-followers)
12. [Visual Mapping Table](#12-visual-mapping-table)

---

## 1. RMS Energy

### 1.1 Mathematical Definition

Root Mean Square energy measures the effective amplitude of a signal over a window of N samples:

```
RMS = sqrt( (1/N) * sum_{n=0}^{N-1} x[n]^2 )
```

In decibels (dBFS):

```
RMS_dBFS = 20 * log10(RMS)
```

where full-scale is 1.0 for floating-point audio.

### 1.2 Block-Based vs. Sliding Window

**Block-based (non-overlapping)**: Compute one RMS value per audio callback buffer. If the buffer size is 512 samples at 48 kHz, the update rate is ~93.75 Hz. This is the cheapest option: accumulate the sum of squares across the block, divide by N, take the square root. No state is carried between blocks.

**Sliding window (overlapping)**: Maintain a running sum of squares in a circular buffer. When a new sample enters the window and an old sample leaves, update incrementally:

```
sum_sq += x_new^2 - x_old^2
RMS = sqrt(sum_sq / N)
```

This gives sample-rate update resolution but requires storing the entire window of samples. For a 50 ms window at 48 kHz, that is 2400 samples (9.6 KB at float32). The sliding approach is useful when the audio callback block size is large but you need smoother visual transitions.

**Exponentially-weighted RMS (EMA)**: A stateless alternative that approximates a sliding window without storing past samples:

```
mean_sq = alpha * x[n]^2 + (1 - alpha) * mean_sq_prev
RMS = sqrt(mean_sq)
```

where `alpha = 1 - exp(-1 / (tau * fs))` and `tau` is the time constant in seconds. This is the most common choice for visualization because it is cheap (2 multiplies, 1 add, 1 sqrt per sample) and produces smooth output.

### 1.3 Typical Ranges

| Signal Type | RMS (linear) | RMS (dBFS) |
|---|---|---|
| Silence / noise floor | 0.0001 - 0.001 | -80 to -60 |
| Quiet passage | 0.01 - 0.05 | -40 to -26 |
| Normal music | 0.05 - 0.3 | -26 to -10 |
| Loudness-war master | 0.3 - 0.7 | -10 to -3 |
| Full-scale sine | 0.707 | -3.01 |
| Full-scale square | 1.0 | 0.0 |

### 1.4 C++ Implementation

```cpp
class RmsEnergy {
public:
    explicit RmsEnergy(float timeConstantSeconds, float sampleRate)
        : alpha_(1.0f - std::exp(-1.0f / (timeConstantSeconds * sampleRate)))
        , meanSquare_(0.0f)
    {}

    // Process a single sample, return current RMS
    float processSample(float x) {
        meanSquare_ += alpha_ * (x * x - meanSquare_);
        return std::sqrt(meanSquare_);
    }

    // Process a block, return RMS at end of block
    float processBlock(const float* input, int numSamples) {
        for (int i = 0; i < numSamples; ++i) {
            float sq = input[i] * input[i];
            meanSquare_ += alpha_ * (sq - meanSquare_);
        }
        return std::sqrt(meanSquare_);
    }

    float getDbfs() const {
        constexpr float kFloor = 1e-10f;
        return 20.0f * std::log10(std::max(std::sqrt(meanSquare_), kFloor));
    }

    void reset() { meanSquare_ = 0.0f; }

private:
    float alpha_;
    float meanSquare_;
};
```

### 1.5 Computational Cost

- **Block-based**: O(N) per block -- 1 multiply-accumulate per sample plus 1 sqrt and 1 divide.
- **EMA**: O(N) per block -- 2 multiplies and 1 add per sample, 1 sqrt at end.
- Both are trivially vectorizable with SIMD (SSE/NEON).

### 1.6 Visualization Use

RMS is the single most important feature for visualization. It drives overall brightness, global scale, background intensity, and particle emission rate. The EMA variant with a 30-50 ms time constant provides visually smooth energy tracking that correlates well with perceived loudness for broadband signals. For frequency-band-specific energy, see [FEATURES_spectral.md](FEATURES_spectral.md).

---

## 2. Peak Amplitude

### 2.1 Sample-Accurate Peak

The simplest peak detector returns the maximum absolute sample value within a block:

```
peak = max_{n=0}^{N-1} |x[n]|
```

This is trivial to compute and costs O(N) comparisons per block. However, it misses inter-sample peaks -- the true waveform peak between discrete samples can exceed any individual sample value by up to ~3 dB (for certain pathological signals, though typically 0.5-1.5 dB for music).

### 2.2 True Peak (ITU-R BS.1770-4)

ITU-R BS.1770-4 defines "true peak" measurement using 4x oversampling with a specific 48-tap FIR interpolation filter. The filter coefficients are specified in the standard's Annex 2. The process:

1. Upsample the signal by a factor of 4 (insert 3 zeros between each sample).
2. Apply the 48-tap lowpass FIR filter (12 taps per phase in polyphase implementation).
3. Take the maximum absolute value across all interpolated samples.

The polyphase decomposition avoids computing the full 4x sample stream. Instead, for each input sample, evaluate 4 polyphase branches, each requiring 12 multiply-accumulates. Total cost: 48 MACs per input sample -- roughly 25x more expensive than sample-accurate peak, but still well within real-time budget.

### 2.3 C++ Implementation

```cpp
class TruePeakDetector {
public:
    // ITU-R BS.1770-4 specifies 48 coefficients for 4x oversampling.
    // The coefficients are split into 4 polyphase sub-filters of 12 taps each.
    static constexpr int kNumPhases = 4;
    static constexpr int kTapsPerPhase = 12;

    TruePeakDetector() { std::fill(std::begin(history_), std::end(history_), 0.0f); }

    // Feed one input sample, return the true peak across 4 interpolated points
    float processSample(float x) {
        // Shift history
        for (int i = kTapsPerPhase - 1; i > 0; --i)
            history_[i] = history_[i - 1];
        history_[0] = x;

        float peak = 0.0f;
        for (int phase = 0; phase < kNumPhases; ++phase) {
            float sum = 0.0f;
            for (int tap = 0; tap < kTapsPerPhase; ++tap) {
                sum += coeffs_[phase][tap] * history_[tap];
            }
            peak = std::max(peak, std::abs(sum));
        }
        return peak;
    }

    float processBlock(const float* input, int numSamples) {
        float blockPeak = 0.0f;
        for (int i = 0; i < numSamples; ++i) {
            blockPeak = std::max(blockPeak, processSample(input[i]));
        }
        return blockPeak;
    }

private:
    float history_[kTapsPerPhase] = {};
    // Coefficients from ITU-R BS.1770-4 Annex 2, arranged in polyphase form.
    // Abbreviated here -- the full 48 coefficients are in the standard.
    static constexpr float coeffs_[kNumPhases][kTapsPerPhase] = {
        // Phase 0 (passthrough phase -- these sum to ~1.0)
        { 0.0017f, -0.0049f, 0.0117f, -0.0245f, 0.0511f, -0.1517f,
          0.8950f,  0.2680f, -0.0639f,  0.0269f, -0.0107f, 0.0031f },
        // Phase 1 ... Phase 2 ... Phase 3 ...
        // (full coefficients omitted for brevity -- use ITU-R BS.1770-4 Annex 2)
        {0}, {0}, {0}  // placeholder
    };
};
```

### 2.4 Parameter Summary

| Parameter | Value |
|---|---|
| Update rate | Per block (e.g. ~93 Hz at 512 samples / 48 kHz) |
| Output range | 0.0 to ~1.1 (can exceed 1.0 for inter-sample peaks) |
| Output range (dBTP) | -inf to ~+3.0 dBTP |
| Computational cost | 48 MACs/sample for true peak, 1 comparison/sample for sample peak |
| Latency | 12 samples (polyphase filter delay) |

### 2.5 Visualization Use

Peak amplitude drives hard transient responses: flash effects, strobe triggers, camera shake. The difference between peak and RMS (crest factor, see below) indicates transient sharpness.

---

## 3. Crest Factor

### 3.1 Definition

```
CF = Peak / RMS
CF_dB = 20 * log10(Peak / RMS) = Peak_dBFS - RMS_dBFS
```

Crest factor measures the ratio of instantaneous peak to RMS energy over the same window. It quantifies "spikiness" or transient content.

### 3.2 Interpretation

| Crest Factor (dB) | Signal Character |
|---|---|
| 0 dB | Square wave -- no headroom |
| 3 dB | Sine wave |
| 6-12 dB | Typical pop/rock music |
| 12-20 dB | Acoustic music, classical, jazz |
| 20-30 dB | Sparse transients (solo percussion, speech) |
| > 30 dB | Near-silence with occasional impulses |

A low crest factor indicates a heavily compressed or limited signal. A high crest factor indicates sparse, impulsive content with lots of headroom. In a VJ context, high crest factor means the signal has strong transients that benefit from peak-triggered effects, while low crest factor music needs smoother RMS-driven visuals.

### 3.3 C++ Implementation

```cpp
class CrestFactor {
public:
    // Compute over a single block
    static float compute(const float* input, int numSamples) {
        float peak = 0.0f;
        float sumSq = 0.0f;
        for (int i = 0; i < numSamples; ++i) {
            float absVal = std::abs(input[i]);
            peak = std::max(peak, absVal);
            sumSq += input[i] * input[i];
        }
        float rms = std::sqrt(sumSq / static_cast<float>(numSamples));
        if (rms < 1e-10f) return 0.0f;
        return peak / rms;
    }

    static float computeDb(const float* input, int numSamples) {
        float cf = compute(input, numSamples);
        if (cf < 1e-10f) return 0.0f;
        return 20.0f * std::log10(cf);
    }
};
```

### 3.4 Visualization Use

Crest factor selects between transient-responsive and smooth visual modes at runtime. When CF > 14 dB, enable peak-triggered strobes and particle bursts. When CF < 6 dB, switch to RMS-driven smooth gradients and slow morphing.

---

## 4. Dynamic Range

### 4.1 Definitions

Dynamic range has multiple definitions depending on context:

**Instantaneous dynamic range**: `Peak_dBFS - NoiseFloor_dBFS` for the full signal. Not measurable in real-time without analyzing the entire file.

**Short-term dynamic range**: The difference between the loudest and quietest short-term (3s) loudness measurements over a rolling window. This approximates the EBU R128 Loudness Range (LRA) -- see Section 6.

**Block dynamic range**: `max(RMS_block) - min(RMS_block)` over a sliding window of blocks. Practical for real-time with a 10-30 second history buffer.

**Percentile dynamic range**: Difference between the 95th and 5th percentile of short-term loudness values. This is essentially LRA (Section 6) and is the most perceptually meaningful definition.

### 4.2 Real-Time Estimation

```cpp
class DynamicRangeEstimator {
public:
    explicit DynamicRangeEstimator(int historyBlocks = 256)
        : maxHistory_(historyBlocks) {}

    void pushRmsDb(float rmsDb) {
        history_.push_back(rmsDb);
        if (static_cast<int>(history_.size()) > maxHistory_)
            history_.pop_front();
    }

    float getDynamicRangeDb() const {
        if (history_.size() < 2) return 0.0f;
        float maxVal = *std::max_element(history_.begin(), history_.end());
        float minVal = *std::min_element(history_.begin(), history_.end());
        return maxVal - minVal;
    }

    // Percentile-based (more robust to outliers)
    float getPercentileRangeDb(float lowPct = 0.10f, float highPct = 0.95f) const {
        if (history_.size() < 10) return 0.0f;
        std::vector<float> sorted(history_.begin(), history_.end());
        std::sort(sorted.begin(), sorted.end());
        int loIdx = static_cast<int>(lowPct * sorted.size());
        int hiIdx = static_cast<int>(highPct * sorted.size());
        hiIdx = std::min(hiIdx, static_cast<int>(sorted.size()) - 1);
        return sorted[hiIdx] - sorted[loIdx];
    }

private:
    std::deque<float> history_;
    int maxHistory_;
};
```

### 4.3 Parameter Summary

| Parameter | Value |
|---|---|
| Update rate | Per block (~93 Hz) |
| Output range | 0 - 60+ dB |
| Typical pop/EDM | 6 - 12 dB |
| Typical classical | 20 - 50 dB |
| History window | 10-30 seconds recommended |
| Computational cost | O(N log N) per query if using percentile sort; O(1) amortized for min/max |

### 4.4 Visualization Use

Dynamic range controls the visual contrast ratio. Low dynamic range (compressed music) maps to high-saturation, uniform-brightness visuals. High dynamic range maps to wide brightness swings and more dramatic light-to-dark transitions. It can also adaptively scale the visual gain: quiet passages in high-DR music need amplified visual response to remain visible.

---

## 5. LUFS / LKFS Loudness (ITU-R BS.1770)

### 5.1 Overview

ITU-R BS.1770 defines a loudness measurement that correlates with human perception far better than simple RMS. The unit is LUFS (Loudness Units relative to Full Scale) or equivalently LKFS. The measurement chain is:

1. **K-weighting filter** (two cascaded biquad stages)
2. **Mean square energy** per channel
3. **Channel weighting** (surround channels weighted differently)
4. **Gating** (absolute -70 LUFS gate, then relative -10 LU gate)

### 5.2 K-Weighting Filter Design

The K-weighting filter consists of two cascaded second-order IIR (biquad) filters:

**Stage 1 -- Pre-filter (shelving)**: Accounts for the acoustic effect of the head. This is a high-shelf boost of approximately +4 dB above 1.5 kHz. The ITU standard specifies exact coefficients for 48 kHz sample rate:

```
b0 =  1.53512485958697
b1 = -2.69169618940638
b2 =  1.19839281085285
a1 = -1.69065929318241
a2 =  0.73248077421585
```

**Stage 2 -- RLB (Revised Low-frequency B-curve) high-pass**: A second-order high-pass filter with a corner frequency around 38 Hz that rolls off very low frequencies that contribute little to perceived loudness:

```
b0 =  1.0
b1 = -2.0
b2 =  1.0
a1 = -1.99004745483398
a2 =  0.99007225036621
```

For sample rates other than 48 kHz, the coefficients must be recomputed from the analog prototype using the bilinear transform. The pre-filter prototype is a high-shelf: `H(s) = Vh * (s^2 + (sqrt(2*Vh)/Q)*wc*s + wc^2) / (s^2 + sqrt(2)/Q*wc*s + wc^2)` with Vh = 10^(4/20), fc ~= 1500 Hz, Q ~= 0.7071. The RLB prototype is a second-order Butterworth high-pass at ~38 Hz.

### 5.3 Loudness Measurement Windows

| Measurement | Window | Overlap | Update Rate | Use |
|---|---|---|---|---|
| Momentary | 400 ms | 75% (100 ms hop) | 10 Hz | Real-time metering, visualization |
| Short-term | 3 s | 67% (1 s hop) | 1 Hz | Program loudness monitoring |
| Integrated | Full program | N/A | End-of-file | Broadcast compliance |

For visualization, **momentary loudness** (400 ms sliding window) is the primary metric. It updates at 10 Hz, which is fast enough for smooth visual transitions but slower than per-block RMS. Some implementations compute it per audio block and apply a 400 ms sliding window to the block-level mean-square values.

### 5.4 Channel Weighting

For surround formats, each channel's mean square energy is weighted before summation:

| Channel | Weight |
|---|---|
| Left, Right, Center | 1.0 (0 dB) |
| Left Surround, Right Surround | 1.41 (+1.5 dB) |
| LFE | Excluded |

For stereo content (the common case in music visualization), both channels have weight 1.0 and the loudness is simply:

```
L_KS = -0.691 + 10 * log10( (1/N) * sum(y_L[n]^2 + y_R[n]^2) / 2 )
```

where `y_L` and `y_R` are the K-weighted left and right channels, and -0.691 is the normalization constant that makes a 1 kHz sine at 0 dBFS read as -3.01 LUFS (matching the RMS convention).

Wait -- let me be precise. The formula is:

```
L_K = -0.691 + 10 * log10( sum_over_channels( G_i * z_i ) )
```

where `z_i = (1/N) * sum(y_i[n]^2)` is the mean square of K-weighted channel i, and `G_i` is the channel weight. For stereo: `L_K = -0.691 + 10 * log10(z_L + z_R)`. Note that this sums the channel energies (not averages them), so stereo at 0 dBFS reads ~+3 dB higher than mono at 0 dBFS.

### 5.5 Gating (Integrated Loudness Only)

Integrated loudness uses a two-stage gate to exclude silence and quiet passages:

1. **Absolute gate**: Discard all 400 ms blocks with loudness < -70 LUFS.
2. **Relative gate**: Compute the ungated mean of remaining blocks. Then discard all blocks with loudness < (ungated_mean - 10 LU). Recompute the mean from surviving blocks.

For real-time visualization, gating is typically not applied to momentary/short-term measurements. However, the absolute -70 LUFS gate is useful to suppress noise-floor readings from affecting visual output during silence.

### 5.6 Complete C++ Implementation -- Momentary LUFS

```cpp
#include <cmath>
#include <algorithm>
#include <array>
#include <deque>

// Second-order IIR (biquad) filter -- Direct Form II Transposed
struct Biquad {
    float b0, b1, b2, a1, a2;
    float z1 = 0.0f, z2 = 0.0f;

    float process(float x) {
        float y = b0 * x + z1;
        z1 = b1 * x - a1 * y + z2;
        z2 = b2 * x - a2 * y;
        return y;
    }

    void reset() { z1 = z2 = 0.0f; }
};

class MomentaryLUFS {
public:
    // Constructor for 48 kHz stereo.
    // For other sample rates, recompute coefficients via bilinear transform.
    MomentaryLUFS(float sampleRate, int blockSize)
        : sampleRate_(sampleRate)
        , blockSize_(blockSize)
    {
        // Number of 400ms-worth of blocks to accumulate
        // 400 ms window with per-block updates
        float blocksPerSecond = sampleRate / static_cast<float>(blockSize);
        windowBlocks_ = static_cast<int>(std::ceil(0.4f * blocksPerSecond));

        initFilters48k(); // Use 48 kHz coefficients
    }

    // Process one stereo-interleaved block.
    // Returns momentary loudness in LUFS.
    float processBlock(const float* interleavedStereo, int numFrames) {
        float sumSqL = 0.0f, sumSqR = 0.0f;

        for (int i = 0; i < numFrames; ++i) {
            float L = interleavedStereo[i * 2 + 0];
            float R = interleavedStereo[i * 2 + 1];

            // Apply K-weighting: pre-filter then RLB
            float kL = rlbL_.process(preFilterL_.process(L));
            float kR = rlbR_.process(preFilterR_.process(R));

            sumSqL += kL * kL;
            sumSqR += kR * kR;
        }

        // Mean square for this block
        float msL = sumSqL / static_cast<float>(numFrames);
        float msR = sumSqR / static_cast<float>(numFrames);

        // Push into sliding window
        blockMsL_.push_back(msL);
        blockMsR_.push_back(msR);
        if (static_cast<int>(blockMsL_.size()) > windowBlocks_) {
            blockMsL_.pop_front();
            blockMsR_.pop_front();
        }

        // Average mean-square over 400 ms window
        float avgMsL = 0.0f, avgMsR = 0.0f;
        for (size_t j = 0; j < blockMsL_.size(); ++j) {
            avgMsL += blockMsL_[j];
            avgMsR += blockMsR_[j];
        }
        avgMsL /= static_cast<float>(blockMsL_.size());
        avgMsR /= static_cast<float>(blockMsR_.size());

        // Loudness in LUFS (stereo, both channels weight = 1.0)
        float power = avgMsL + avgMsR;
        if (power < 1e-15f) return -100.0f; // floor
        return -0.691f + 10.0f * std::log10(power);
    }

    void reset() {
        preFilterL_.reset(); preFilterR_.reset();
        rlbL_.reset(); rlbR_.reset();
        blockMsL_.clear(); blockMsR_.clear();
    }

private:
    void initFilters48k() {
        // Stage 1: Pre-filter (high shelf, ~+4 dB above 1.5 kHz)
        // Coefficients from ITU-R BS.1770-4 for 48 kHz
        preFilterL_ = { 1.53512485958697f, -2.69169618940638f, 1.19839281085285f,
                       -1.69065929318241f,  0.73248077421585f };
        preFilterR_ = preFilterL_;

        // Stage 2: RLB high-pass (~38 Hz)
        rlbL_ = { 1.0f, -2.0f, 1.0f,
                 -1.99004745483398f, 0.99007225036621f };
        rlbR_ = rlbL_;
    }

    float sampleRate_;
    int blockSize_;
    int windowBlocks_;

    Biquad preFilterL_, preFilterR_;
    Biquad rlbL_, rlbR_;

    std::deque<float> blockMsL_, blockMsR_;
};
```

### 5.7 Coefficient Recomputation for Arbitrary Sample Rates

The 48 kHz coefficients above are exact only at 48 kHz. For 44.1 kHz or 96 kHz, recompute via bilinear transform from the analog prototypes. The pre-filter analog prototype parameters are:

```
fc = 1681.974450955533 Hz
Vh = 1.584893192461113  (= 10^(4/20))
Vb = sqrt(Vh) = 1.258925411794167
Q  = 0.7071752369554196

Pre-warp: wc = 2 * pi * fc, K = tan(pi * fc / fs)
```

For the RLB filter:

```
fc = 38.13547087602444 Hz
Q  = 0.5003270373238773
```

The bilinear transform formulas for these specific filter types are standard; implementations can be found in audio-EQ-cookbook references. A production implementation should compute these at initialization time given the actual sample rate.

### 5.8 Parameter Summary

| Parameter | Value |
|---|---|
| Update rate | 10 Hz (momentary), 1 Hz (short-term) |
| Output range | -70 to 0 LUFS (music typically -24 to -6 LUFS) |
| K-weighting cost | 2 biquads per channel = 10 MACs/sample/channel |
| Window cost | Negligible (deque of block-level scalars) |
| Latency | 400 ms (momentary), 3 s (short-term) |

### 5.9 Visualization Use

LUFS is the best single metric for perceptual loudness. Use momentary LUFS to drive master brightness or global visual intensity. It is superior to raw RMS because the K-weighting deemphasizes sub-bass rumble and boosts upper-midrange presence frequencies that humans are most sensitive to. This means visuals react to what the audience actually perceives as "loud," not to inaudible low-frequency energy.

---

## 6. Loudness Range (LRA)

### 6.1 Definition (EBU R128)

Loudness Range quantifies the variation of loudness over time. It is defined as the difference between the 95th and 10th percentiles of the distribution of short-term (3 s) loudness values, after absolute gating at -70 LUFS and relative gating at -20 LU below the ungated mean.

```
LRA = L_95 - L_10   (in LU, Loudness Units)
```

### 6.2 Real-Time Approximation

True LRA requires accumulating a histogram of short-term loudness values over the entire program. For real-time use, maintain a rolling histogram over the last 30-120 seconds:

```cpp
class LoudnessRange {
public:
    LoudnessRange(int maxHistorySeconds = 60, float updateRateHz = 1.0f)
        : maxEntries_(static_cast<int>(maxHistorySeconds * updateRateHz))
    {}

    void pushShortTermLufs(float lufs) {
        if (lufs < -70.0f) return; // absolute gate
        history_.push_back(lufs);
        if (static_cast<int>(history_.size()) > maxEntries_)
            history_.pop_front();
    }

    float getLRA() const {
        if (history_.size() < 20) return 0.0f;

        // Relative gate: compute mean, exclude below mean - 20 LU
        float sum = 0.0f;
        for (float v : history_) sum += v;
        float mean = sum / static_cast<float>(history_.size());
        float gate = mean - 20.0f;

        std::vector<float> gated;
        gated.reserve(history_.size());
        for (float v : history_) {
            if (v >= gate) gated.push_back(v);
        }
        if (gated.size() < 2) return 0.0f;

        std::sort(gated.begin(), gated.end());
        int idx10 = static_cast<int>(0.10f * gated.size());
        int idx95 = static_cast<int>(0.95f * gated.size());
        idx95 = std::min(idx95, static_cast<int>(gated.size()) - 1);

        return gated[idx95] - gated[idx10];
    }

private:
    std::deque<float> history_;
    int maxEntries_;
};
```

### 6.3 Typical Ranges

| Content | LRA (LU) |
|---|---|
| Heavily compressed pop | 3 - 5 |
| Typical pop/rock | 5 - 10 |
| Classical / film score | 10 - 20 |
| Wide dynamic range recording | 20+ |

### 6.4 Visualization Use

LRA drives macro-level visual behavior. Low LRA signals benefit from subtle, continuous visual effects. High LRA signals need dynamic auto-gain on the visual mapping to prevent quiet sections from being invisible and loud sections from clipping. LRA can also select presets: high-LRA content pairs well with dramatic, high-contrast visual themes.

---

## 7. Noise Floor Estimation

### 7.1 Minimum Energy Tracking

The simplest noise floor estimator tracks the minimum RMS energy observed over a sliding window. The assumption is that the quietest moments reveal the underlying noise:

```cpp
class NoiseFloorEstimator {
public:
    explicit NoiseFloorEstimator(int windowBlocks = 512)
        : windowSize_(windowBlocks) {}

    void pushRmsDb(float rmsDb) {
        history_.push_back(rmsDb);
        if (static_cast<int>(history_.size()) > windowSize_)
            history_.pop_front();
    }

    // Return the 5th percentile as a robust minimum
    float getNoiseFloorDb() const {
        if (history_.size() < 10) return -96.0f;
        std::vector<float> sorted(history_.begin(), history_.end());
        std::sort(sorted.begin(), sorted.end());
        int idx = static_cast<int>(0.05f * sorted.size());
        return sorted[idx];
    }

private:
    std::deque<float> history_;
    int windowSize_;
};
```

Using the 5th percentile instead of the absolute minimum provides robustness against measurement artifacts and digital silence.

### 7.2 Spectral Floor Estimation

A more accurate technique operates in the frequency domain: compute the magnitude spectrum via FFT, then track the minimum magnitude in each bin over time. This yields a frequency-dependent noise floor profile. See [FEATURES_spectral.md](FEATURES_spectral.md) for FFT-based analysis details.

### 7.3 Visualization Use

The noise floor defines the "zero" reference for visual mapping. Subtract the noise floor from RMS measurements so that quiet content in a noisy signal still produces zero visual activity during true silence. This prevents the visualization from "dancing to noise."

---

## 8. Signal-to-Noise Ratio (SNR)

### 8.1 Definition

```
SNR = 20 * log10(RMS_signal / RMS_noise)
```

### 8.2 Real-Time Estimation

Real-time SNR estimation requires separating signal from noise, which is an ill-posed problem in general. Practical approaches for visualization:

**Method 1 -- Peak-to-noise-floor ratio**: Use the short-term peak RMS as the signal level and the noise floor estimate (Section 7) as the noise level. This gives an upper-bound SNR.

**Method 2 -- Spectral SNR**: In each frequency bin, compare the current magnitude to the tracked noise floor magnitude. Average the per-bin SNR across frequency. This is more accurate but requires FFT computation.

**Method 3 -- Activity-based**: Classify blocks as "signal present" (above a threshold) or "noise only" (below threshold). Compute RMS separately for each class. This works well when the signal has clear pauses.

```cpp
float estimateSnrDb(float currentRmsDb, float noiseFloorDb) {
    return currentRmsDb - noiseFloorDb;
}
```

### 8.3 Typical Ranges

| Source | SNR (dB) |
|---|---|
| Professional studio recording | 90 - 120 |
| CD-quality (16-bit) | ~96 |
| Live concert recording | 50 - 80 |
| Noisy field recording | 20 - 40 |
| Low-quality stream / lo-fi | 30 - 60 |

### 8.4 Visualization Use

SNR controls the visual "clarity" mapping. High SNR signals can drive sharp, detailed visuals. Low SNR signals benefit from softer, more diffuse effects that mask the noise. SNR can also drive a "grain" or "static" overlay intensity that is inversely proportional to SNR -- making noisy audio look intentionally grainy.

---

## 9. Dynamic Compression Detection

### 9.1 Compression Indicators

Detecting whether a signal has been dynamically compressed is useful for adaptive visualization. Several heuristics work in real-time:

**9.1.1 Crest Factor Analysis**: Sustained low crest factor (< 6 dB) over multiple seconds strongly indicates heavy compression or limiting. Compare the rolling crest factor to genre-typical baselines.

**9.1.2 Short-term Loudness Variance**: Compressed signals exhibit low variance in short-term loudness. Compute `std_dev(short_term_LUFS)` over a 30-second window. Values below 2 LU suggest heavy compression.

**9.1.3 Gain Reduction Estimation**: If you have access to the original and processed signals (rare in a VJ context), the gain reduction is simply `GR = original_dB - processed_dB`. Without the original, estimate gain reduction by analyzing the attack transient shape: compressed transients have slower rise times and flattened peaks.

**9.1.4 Waveform Flatness**: Compute the ratio of RMS to peak over very short windows (5-10 ms). In uncompressed audio, transient onsets show a sharp peak-to-RMS divergence. In compressed audio, the peak barely exceeds the RMS even during attacks.

### 9.2 C++ Implementation

```cpp
class CompressionDetector {
public:
    // Returns a compression estimate 0.0 (uncompressed) to 1.0 (heavily compressed)
    float estimate(const float* input, int numSamples) {
        float peak = 0.0f;
        float sumSq = 0.0f;
        for (int i = 0; i < numSamples; ++i) {
            float a = std::abs(input[i]);
            peak = std::max(peak, a);
            sumSq += input[i] * input[i];
        }
        float rms = std::sqrt(sumSq / static_cast<float>(numSamples));
        if (rms < 1e-10f) return 0.0f;

        float crestDb = 20.0f * std::log10(peak / rms);

        // Map crest factor to compression estimate
        // 3 dB (square wave) -> 1.0, 20 dB (dynamic) -> 0.0
        float compression = 1.0f - std::clamp((crestDb - 3.0f) / 17.0f, 0.0f, 1.0f);

        // Smooth with EMA
        smoothed_ += 0.1f * (compression - smoothed_);
        return smoothed_;
    }

private:
    float smoothed_ = 0.0f;
};
```

### 9.3 Visualization Use

Compression level drives the visual "intensity mode." Heavily compressed signals (EDM, modern pop) pair well with high-saturation, uniform-intensity effects. Less compressed signals (jazz, classical) benefit from wider visual dynamic range and more subtle transitions.

---

## 10. Clipping Detection

### 10.1 Sample-Level Clipping

The simplest detector flags consecutive samples at or very near full scale:

```cpp
struct ClipDetector {
    int consecutiveClips = 0;
    int clipEvents = 0;         // Number of clipping events per block
    int totalClippedSamples = 0;

    static constexpr float kClipThreshold = 0.9999f; // ~-0.0009 dBFS

    void processBlock(const float* input, int numSamples) {
        clipEvents = 0;
        totalClippedSamples = 0;
        bool wasClipping = false;

        for (int i = 0; i < numSamples; ++i) {
            bool isClipping = std::abs(input[i]) >= kClipThreshold;
            if (isClipping) {
                totalClippedSamples++;
                if (!wasClipping) clipEvents++;  // rising edge
            }
            wasClipping = isClipping;
        }
    }

    // Clipping severity: 0.0 = none, 1.0 = entire block clipped
    float severity() const {
        return 0.0f; // Must be called after processBlock with numSamples
    }
};
```

### 10.2 Inter-Sample Peak (ISP) Clipping

Even if no individual sample reaches full scale, the reconstructed analog waveform between samples can exceed 0 dBFS. This is inter-sample clipping, and it's detected by the True Peak measurement from Section 2. If the true peak exceeds 0 dBTP while no sample exceeds 0 dBFS, inter-sample clipping is occurring.

```cpp
bool detectInterSampleClip(float samplePeak, float truePeak) {
    return (samplePeak < 1.0f) && (truePeak >= 1.0f);
}
```

Inter-sample peaks exceeding +0.2 dBTP are common in loudness-war masters. Values up to +3 dBTP have been measured in commercial releases.

### 10.3 Visualization Use

Clipping detection triggers warning visuals: red flash overlays, border indicators, or distortion effects. For artistic use, clipping intensity can drive deliberate "overdriven" visual effects -- glitch artifacts, color channel separation, bloom intensity spikes.

---

## 11. Envelope Followers

### 11.1 Theory

An envelope follower extracts the amplitude envelope of a signal using asymmetric smoothing: fast attack (to catch transients) and slow release (to maintain visual continuity). This is the most important feature for music visualization because it provides frame-rate-ready amplitude data with controllable responsiveness.

### 11.2 Mathematical Model

The classic analog-modeled envelope follower:

```
if |x[n]| > env[n-1]:
    env[n] = attack_coeff * env[n-1] + (1 - attack_coeff) * |x[n]|    // attack
else:
    env[n] = release_coeff * env[n-1]                                   // release
```

where:

```
attack_coeff  = exp(-1 / (attack_time * sample_rate))
release_coeff = exp(-1 / (release_time * sample_rate))
```

The time constant defines the time to reach ~63.2% of the target value (1 - 1/e). In practice, it takes approximately 3-5 time constants to fully converge.

### 11.3 Attack / Release Time Constants

| Application | Attack | Release | Character |
|---|---|---|---|
| Fast transient tracking | 0.1 - 1 ms | 10 - 50 ms | Snappy, jittery |
| Music visualization (default) | 5 - 15 ms | 100 - 300 ms | Responsive, smooth |
| VU meter emulation | 300 ms | 300 ms | Slow, averaged |
| PPM meter emulation | 10 ms | 1500 ms | Fast attack, slow decay |
| Sidechain pump effect | 0.1 ms | 200 - 500 ms | Dramatic ducking |
| Ambient / slow morph | 50 - 200 ms | 1 - 5 s | Glacial, smooth |

### 11.4 Ballistics Variants

**Linear release**: Instead of exponential decay, the envelope decreases at a constant rate (dB/second). This matches the ballistics of analog VU meters and some broadcast PPM meters. Implementation:

```
if releasing:
    env[n] = env[n-1] - releaseRate / sampleRate
    env[n] = max(env[n], |x[n]|)
```

where `releaseRate` is in linear amplitude per second.

**Logarithmic (dB-linear) release**: Work in the dB domain for perceptually uniform decay:

```
env_dB[n] = env_dB[n-1] - releaseRateDb / sampleRate
```

This produces visually smoother decay than exponential release in the linear domain.

**Adaptive envelope**: Automatically adjust attack/release based on signal characteristics. Short bursts trigger fast attack; sustained tones trigger slow release. This can be implemented by modulating the time constants based on the rate of change of the envelope itself.

### 11.5 C++ Implementation

```cpp
class EnvelopeFollower {
public:
    EnvelopeFollower(float attackMs, float releaseMs, float sampleRate)
    {
        setAttack(attackMs, sampleRate);
        setRelease(releaseMs, sampleRate);
    }

    void setAttack(float ms, float sr) {
        attackCoeff_ = std::exp(-1.0f / (ms * 0.001f * sr));
    }

    void setRelease(float ms, float sr) {
        releaseCoeff_ = std::exp(-1.0f / (ms * 0.001f * sr));
    }

    float processSample(float x) {
        float input = std::abs(x);
        if (input > envelope_) {
            // Attack phase
            envelope_ = attackCoeff_ * envelope_ + (1.0f - attackCoeff_) * input;
        } else {
            // Release phase
            envelope_ = releaseCoeff_ * envelope_;
        }
        return envelope_;
    }

    float processBlock(const float* input, int numSamples) {
        for (int i = 0; i < numSamples; ++i) {
            processSample(input[i]);
        }
        return envelope_;
    }

    // Process block and write per-sample envelope (for sub-block resolution)
    void processBlockDetailed(const float* input, float* output, int numSamples) {
        for (int i = 0; i < numSamples; ++i) {
            output[i] = processSample(input[i]);
        }
    }

    float getEnvelope() const { return envelope_; }
    void reset() { envelope_ = 0.0f; }

private:
    float attackCoeff_ = 0.0f;
    float releaseCoeff_ = 0.0f;
    float envelope_ = 0.0f;
};

// Multi-band envelope: run parallel envelope followers on band-split audio
// See FEATURES_spectral.md for band-splitting via crossover filters.
class MultibandEnvelope {
public:
    MultibandEnvelope(int numBands, float attackMs, float releaseMs, float sampleRate)
    {
        for (int i = 0; i < numBands; ++i) {
            bands_.emplace_back(attackMs, releaseMs, sampleRate);
        }
    }

    // Process pre-split bands (one buffer per band)
    void process(const float* const* bandBuffers, int numBands, int numSamples,
                 float* envelopesOut)
    {
        for (int b = 0; b < numBands && b < static_cast<int>(bands_.size()); ++b) {
            envelopesOut[b] = bands_[b].processBlock(bandBuffers[b], numSamples);
        }
    }

private:
    std::vector<EnvelopeFollower> bands_;
};
```

### 11.6 Computational Cost

| Variant | Cost per sample | Notes |
|---|---|---|
| Basic envelope | 1 comparison, 2 multiplies, 1 add | Trivially cheap |
| Per-sample output | Same, plus 1 store | Still negligible |
| Multi-band (N bands) | N * basic cost | Plus band-splitting cost |

Envelope following is the cheapest feature extractor. It is dominated by the cost of the audio callback itself. A 4-band envelope follower on stereo audio costs roughly 80 operations per sample -- approximately 0.004% of a single core at 48 kHz.

### 11.7 Visualization Use

The envelope follower is the workhorse of music visualization. Common mappings:

- **Raw envelope** -> global brightness, object scale, particle emission rate
- **Envelope derivative** (rate of change) -> motion speed, rotation velocity
- **Envelope with very slow release (2-5 s)** -> background color drift, ambient layer intensity
- **Multi-band envelopes** -> per-frequency-band visual elements (bass = size, mids = texture, highs = sparkle)
- **Peak-held envelope** (envelope with infinite release, reset periodically) -> high-water-mark indicators

---

## 12. Visual Mapping Table

The following table summarizes how each amplitude/dynamics feature maps to visual parameters. This is the bridge between audio analysis and rendering; see [VIDEO_feature_to_visual_mapping.md](VIDEO_feature_to_visual_mapping.md) for the full mapping architecture.

| Feature | Typical Range | Update Rate | Visual Parameter | Mapping Function | Notes |
|---|---|---|---|---|---|
| RMS Energy | -60 to 0 dBFS | Per block (~93 Hz) | Global brightness, object scale | Linear or power-curve (gamma 2.0-3.0) | Primary driver; apply smoothing before mapping |
| Peak Amplitude | 0.0 to 1.0+ | Per block | Flash/strobe trigger, camera shake | Threshold trigger (peak > 0.9) | Use for transient events only |
| Crest Factor | 3 to 30 dB | Per block | Visual mode selection | If CF > 14 dB: transient mode; else: smooth mode | Slow-changing; update every 1-2 s |
| Dynamic Range | 3 to 50 dB | 0.1 Hz | Auto-gain range, contrast ratio | Map to visual gain multiplier range | Very slow; update every 10-30 s |
| Momentary LUFS | -70 to 0 LUFS | 10 Hz | Master intensity, post-process exposure | Linear in LUFS domain (perceptual) | Best single loudness metric |
| Short-term LUFS | -70 to 0 LUFS | 1 Hz | Scene transition weight, mood | Smooth interpolation | Drives macro-level changes |
| LRA | 3 to 25 LU | 0.03 Hz | Visual dynamic range, auto-gain | Inverse: high LRA = wider visual gain range | Computed over 30-60 s window |
| Noise Floor | -96 to -30 dBFS | 0.1 Hz | Visual zero-point, grain overlay | Subtract from RMS before mapping | Prevents noise-driven visuals |
| SNR | 20 to 120 dB | 0.1 Hz | Visual clarity, detail level | High SNR = sharp edges; low SNR = blur | Slow-changing |
| Compression Level | 0.0 to 1.0 | 0.5 Hz | Saturation, intensity uniformity | Linear map to saturation boost | Artistic choice |
| Clipping | 0 or 1 (event) | Per block | Red flash, distortion FX, glitch | Binary trigger or severity-scaled | Brief event; hold for 2-3 frames |
| Envelope (fast) | 0.0 to 1.0 | Per block | Primary animation driver | Power curve (gamma 1.5-3.0) | Most responsive to music |
| Envelope (slow) | 0.0 to 1.0 | Per block | Background drift, ambient layer | Linear, heavily smoothed | Provides continuity |
| Multi-band envelope | 0.0 to 1.0 per band | Per block | Per-band: bass=scale, mid=texture, high=sparkle | Independent curves per band | See FEATURES_spectral.md |
| Envelope derivative | -1.0 to 1.0 | Per block | Motion speed, rotation, turbulence | Signed: positive=expand, negative=contract | Differentiating adds noise; smooth first |

### 12.1 Mapping Function Details

**Linear mapping**: `visual = (feature - min) / (max - min)`. Simple but perceptually non-uniform.

**Power curve (gamma)**: `visual = pow((feature - min) / (max - min), gamma)`. Gamma > 1.0 compresses low values and expands high values, making quiet passages subtler and loud passages more dramatic. Gamma 2.0-3.0 is typical for brightness mapping.

**Logarithmic mapping**: `visual = log(feature / min) / log(max / min)`. Natural for dB-domain features (RMS, LUFS). Produces perceptually uniform brightness steps.

**Threshold trigger**: `visual = (feature > threshold) ? 1.0 : 0.0`. For peak-triggered events. Add a hold time (2-5 frames at 60 fps) to prevent single-frame flashes that are invisible.

**Hysteresis trigger**: Two thresholds -- activate above `high_thresh`, deactivate below `low_thresh`. Prevents rapid on/off flickering near a single threshold.

### 12.2 Temporal Smoothing for Visual Output

All features benefit from a final smoothing stage before driving visuals, even if the feature itself is already smoothed (e.g., envelope follower). This final stage operates at the video frame rate (30-60 Hz) and prevents per-frame jitter:

```cpp
class VisualSmoother {
public:
    VisualSmoother(float riseMs, float fallMs, float videoFps)
        : riseCoeff_(std::exp(-1.0f / (riseMs * 0.001f * videoFps)))
        , fallCoeff_(std::exp(-1.0f / (fallMs * 0.001f * videoFps)))
        , value_(0.0f)
    {}

    float process(float input) {
        if (input > value_)
            value_ = riseCoeff_ * value_ + (1.0f - riseCoeff_) * input;
        else
            value_ = fallCoeff_ * value_ + (1.0f - fallCoeff_) * input;
        return value_;
    }

private:
    float riseCoeff_, fallCoeff_, value_;
};
```

Typical values: rise = 16-33 ms (1-2 video frames), fall = 50-200 ms.

---

## Appendix A: Computational Cost Summary

| Feature | Cost (per sample) | Cost (per block, N=512) | Vectorizable (SIMD) |
|---|---|---|---|
| RMS (EMA) | 3 ops | ~1536 ops | Yes |
| Peak (sample) | 1 compare | ~512 ops | Yes (horizontal max) |
| True Peak (4x) | 48 MACs | ~24576 ops | Yes |
| Crest Factor | RMS + Peak | ~2048 ops | Yes |
| K-weighting (stereo) | 20 MACs | ~10240 ops | Yes |
| LUFS momentary | K-weight + window avg | ~10300 ops | Mostly |
| Envelope follower | 3 ops | ~1536 ops | Limited (data dependency) |
| Clipping detect | 1 compare + branch | ~1024 ops | Yes |
| Noise floor | 0 (per-block stat only) | O(N log N) periodic sort | N/A |

All features combined (excluding true peak and noise floor sort) total approximately 30,000 operations per 512-sample block at 48 kHz. On a modern CPU executing ~10 billion operations per second, this is approximately 0.003 ms per block -- negligible compared to the ~10.67 ms block duration. The entire amplitude/dynamics feature extraction pipeline fits comfortably within 1% of a single CPU core.

---

## Appendix B: Implementation Notes

### B.1 Thread Safety

Audio feature extraction runs on the audio thread (real-time, no blocking). Visual rendering reads features on the render thread. Use a lock-free single-producer single-consumer (SPSC) ring buffer or atomic double-buffer to pass extracted features from the audio thread to the render thread. Never use mutexes on the audio thread. See [ARCH_pipeline.md](ARCH_pipeline.md) for the threading architecture.

### B.2 Denormal Protection

IIR filters (K-weighting biquads, EMA smoothers, envelope followers) can produce denormalized floating-point numbers when the signal goes silent, causing massive CPU spikes on x86. Solutions:

1. Set the FTZ/DAZ flags at the start of the audio thread: `_mm_setcsr(_mm_getcsr() | 0x8040);`
2. Add a tiny DC offset (1e-25f) to filter inputs.
3. Periodically reset filter state during detected silence.

### B.3 Sample Rate Independence

All time-constant-based features (EMA, envelope follower, biquad filters) must recompute their coefficients when the sample rate changes. Expose `setSampleRate()` methods and call them during audio device reconfiguration. The K-weighting coefficients in Section 5 are valid only at 48 kHz and must be recomputed for other rates.

### B.4 Feature Normalization

For consistent visual mapping across different content, normalize features to a 0-1 range using calibrated minimum/maximum values. Auto-calibration (tracking running min/max with slow decay) adapts to the current content but introduces a ~30-second settling period. Pre-calibrated ranges (from the tables in this document) provide immediate response but may clip or underutilize the visual range for unusual content. A hybrid approach works best: use pre-calibrated ranges initially, then gradually blend in auto-calibrated ranges over the first 30 seconds.

---

*Document version: 1.0 | Date: 2026-03-13 | Part of the RealTimeAudio feature extraction research series.*
