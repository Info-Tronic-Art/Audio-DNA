# Transient Detection, Texture Analysis, and Source Separation Features

## Overview

This document covers features that decompose audio into structural components -- harmonic vs. percussive content, transient events vs. sustained texture, tonal vs. noise content -- and extract metrics describing temporal microstructure. These features are critical for a real-time VJ engine because they drive the distinction between smooth/flowing visuals (harmonic sustain) and sharp/explosive visuals (percussive attacks).

All implementations target single-threaded worst-case latency under 1 ms per frame at 44.1 kHz / 2048-sample hop. Where that budget is tight, tiered approximations are given.

Cross-references: [FEATURES_rhythm_tempo.md](FEATURES_rhythm_tempo.md), [FEATURES_spectral.md](FEATURES_spectral.md), [FEATURES_psychoacoustic.md](FEATURES_psychoacoustic.md), [VIDEO_feature_to_visual_mapping.md](VIDEO_feature_to_visual_mapping.md), [ARCH_pipeline.md](ARCH_pipeline.md).

---

## 1. Harmonic-Percussive Source Separation (HPSS)

### 1.1 Theory: Median Filtering (Fitzgerald 2010)

HPSS exploits a fundamental structural difference in the spectrogram:

- **Harmonic content** produces horizontal ridges (sustained energy at fixed frequencies across time).
- **Percussive content** produces vertical ridges (broadband energy concentrated at a single time instant).

Given a magnitude spectrogram `|S(t, f)|`:

1. Apply a **horizontal median filter** of length `L_h` across the time axis at each frequency bin:

```
H(t, f) = median(|S(t - L_h/2, f)|, ..., |S(t + L_h/2, f)|)
```

2. Apply a **vertical median filter** of length `L_p` across the frequency axis at each time frame:

```
P(t, f) = median(|S(t, f - L_p/2)|, ..., |S(t, f + L_p/2)|)
```

The horizontal median preserves energy that persists across time (harmonic), suppressing transient spikes. The vertical median preserves energy that spans many frequency bins simultaneously (percussive), suppressing narrowband harmonics.

### 1.2 Wiener Mask Construction

Soft masks are constructed via Wiener filtering to avoid binary artifacts:

```
M_h(t, f) = H(t, f)^p / (H(t, f)^p + P(t, f)^p + epsilon)
M_p(t, f) = P(t, f)^p / (H(t, f)^p + P(t, f)^p + epsilon)
```

where `p` controls mask sharpness (typically `p = 2` for Wiener, `p = 1` for linear). `epsilon` prevents division by zero (typically `1e-10`).

The separated spectrograms are:

```
S_h(t, f) = M_h(t, f) * S(t, f)     (complex spectrogram, preserving phase)
S_p(t, f) = M_p(t, f) * S(t, f)
```

A residual component can be extracted:

```
M_r(t, f) = 1 - M_h(t, f) - M_p(t, f)
```

This residual captures content that is neither clearly harmonic nor percussive (e.g., noise, transient tails).

### 1.3 Parameter Selection

| Parameter | Typical Value | Effect |
|-----------|--------------|--------|
| FFT size | 2048-4096 | Larger favors harmonic resolution |
| Hop size | 512-1024 | Smaller gives finer temporal resolution for percussive |
| L_h (harmonic median) | 17-31 frames | Longer suppresses more transients; too long smears onsets |
| L_p (percussive median) | 17-31 bins | Longer suppresses more harmonics; too long removes pitched drums |
| p (mask exponent) | 2 | Higher gives harder separation but more artifacts |

### 1.4 Real-Time Approximation with Sliding Window

Full HPSS requires a lookahead of `L_h / 2` frames for the horizontal median, introducing latency of `(L_h / 2) * hop_size / sample_rate` seconds. With `L_h = 17` and hop = 1024 at 44.1 kHz, that is ~186 ms -- too much for real-time visuals.

**Causal approximation strategy:**

1. Use a **one-sided horizontal median** -- only past frames. This biases the harmonic estimate but eliminates lookahead latency entirely.
2. Reduce `L_h` to 9-11 frames for the causal window (the effective context is halved compared to centered).
3. The vertical median (percussive) is already non-causal in frequency only, which requires no temporal lookahead.
4. Maintain a **ring buffer** of the last `L_h` magnitude spectrogram frames. Each new frame triggers a column-wise median for harmonic and a row-wise median for percussive.

**Alternative: Running median via a skip-list or order-statistic tree** -- maintains a sorted window with O(log L) insert and delete per element, yielding O(N_bins * log L_h) per frame for the harmonic median and O(N_frames_in_buffer * log L_p) for the percussive median.

### 1.5 Computational Cost

For FFT size `N = 2048` (1025 bins), hop = 1024:

| Operation | Naive Cost | Optimized Cost |
|-----------|-----------|---------------|
| Horizontal median (per frame) | O(1025 * L_h * log L_h) | O(1025 * log L_h) with running median |
| Vertical median (per frame) | O(1025 * L_p * log L_p) | O(1025 * log L_p) with running median |
| Mask + multiply | O(1025) | O(1025) |
| **Total per frame** | ~500 us naive | ~50-80 us optimized |

The running median using two heaps (max-heap for lower half, min-heap for upper half) gives O(log L) amortized insert/remove:

### 1.6 C++ Implementation Sketch

```cpp
#include <vector>
#include <queue>
#include <algorithm>
#include <cmath>

class RunningMedian {
    // Max-heap for lower half, min-heap for upper half
    std::priority_queue<float> lower_;
    std::priority_queue<float, std::vector<float>, std::greater<float>> upper_;
    std::queue<float> window_;
    size_t maxSize_;

    // For O(log n) removal we need a lazy-deletion augmented heap.
    // Simplified here; production code should use an order-statistic tree
    // or Fenwick tree over quantized magnitudes.
public:
    explicit RunningMedian(size_t maxSize) : maxSize_(maxSize) {}

    void push(float val) {
        if (window_.size() >= maxSize_) {
            // In production: lazy-delete window_.front() from heaps
            window_.pop();
        }
        window_.push(val);

        if (lower_.empty() || val <= lower_.top()) {
            lower_.push(val);
        } else {
            upper_.push(val);
        }
        rebalance();
    }

    float median() const {
        if (lower_.size() > upper_.size()) return lower_.top();
        return (lower_.top() + upper_.top()) * 0.5f;
    }

private:
    void rebalance() {
        while (lower_.size() > upper_.size() + 1) {
            upper_.push(lower_.top()); lower_.pop();
        }
        while (upper_.size() > lower_.size()) {
            lower_.push(upper_.top()); upper_.pop();
        }
    }
};

class HPSS {
    size_t fftSize_;
    size_t numBins_;
    size_t harmonicLen_;   // L_h: number of past frames for horizontal median
    size_t percussiveLen_; // L_p: number of frequency bins for vertical median

    // Ring buffer of past magnitude frames: [frame_index][bin]
    std::vector<std::vector<float>> magHistory_;
    size_t historyHead_ = 0;
    size_t historyCount_ = 0;

    // Per-bin running median for horizontal (harmonic) estimate
    std::vector<RunningMedian> harmonicMedians_;

public:
    HPSS(size_t fftSize, size_t harmonicLen, size_t percussiveLen)
        : fftSize_(fftSize),
          numBins_(fftSize / 2 + 1),
          harmonicLen_(harmonicLen),
          percussiveLen_(percussiveLen),
          magHistory_(harmonicLen, std::vector<float>(fftSize / 2 + 1, 0.0f)),
          harmonicMedians_(fftSize / 2 + 1, RunningMedian(harmonicLen))
    {}

    struct Result {
        std::vector<float> harmonicMask;
        std::vector<float> percussiveMask;
        float harmonicEnergy;
        float percussiveEnergy;
        float hpRatio; // harmonic / (harmonic + percussive)
    };

    Result process(const float* magnitudes) {
        Result result;
        result.harmonicMask.resize(numBins_);
        result.percussiveMask.resize(numBins_);

        // Store current frame in ring buffer
        size_t idx = historyHead_ % harmonicLen_;
        for (size_t b = 0; b < numBins_; ++b) {
            magHistory_[idx][b] = magnitudes[b];
        }
        historyHead_++;
        historyCount_ = std::min(historyCount_ + 1, harmonicLen_);

        // Harmonic estimate: horizontal median (across time) per bin
        std::vector<float> H(numBins_);
        for (size_t b = 0; b < numBins_; ++b) {
            harmonicMedians_[b].push(magnitudes[b]);
            H[b] = harmonicMedians_[b].median();
        }

        // Percussive estimate: vertical median (across frequency) per frame
        // Use current frame magnitudes, apply median filter across bins
        std::vector<float> P(numBins_);
        std::vector<float> sortBuf(percussiveLen_);
        int halfP = static_cast<int>(percussiveLen_) / 2;

        for (size_t b = 0; b < numBins_; ++b) {
            size_t count = 0;
            for (int k = -halfP; k <= halfP; ++k) {
                int bin = static_cast<int>(b) + k;
                if (bin >= 0 && bin < static_cast<int>(numBins_)) {
                    sortBuf[count++] = magnitudes[bin];
                }
            }
            std::nth_element(sortBuf.begin(), sortBuf.begin() + count / 2,
                           sortBuf.begin() + count);
            P[b] = sortBuf[count / 2];
        }

        // Wiener masks (p=2)
        constexpr float kEpsilon = 1e-10f;
        float hEnergy = 0.0f, pEnergy = 0.0f;

        for (size_t b = 0; b < numBins_; ++b) {
            float h2 = H[b] * H[b];
            float p2 = P[b] * P[b];
            float denom = h2 + p2 + kEpsilon;
            result.harmonicMask[b] = h2 / denom;
            result.percussiveMask[b] = p2 / denom;

            float mag2 = magnitudes[b] * magnitudes[b];
            hEnergy += result.harmonicMask[b] * mag2;
            pEnergy += result.percussiveMask[b] * mag2;
        }

        result.harmonicEnergy = hEnergy;
        result.percussiveEnergy = pEnergy;
        result.hpRatio = hEnergy / (hEnergy + pEnergy + kEpsilon);

        return result;
    }
};
```

### 1.7 HPSS Output Features for Visualization

| Feature | Formula | Visual Use |
|---------|---------|-----------|
| HP Ratio | `H_energy / (H_energy + P_energy)` | 0 = pure percussion, 1 = pure harmonic |
| Percussive flux | Frame-to-frame diff of percussive energy | Spike = drum hit |
| Harmonic stability | Variance of HP ratio over 0.5 s | Low = stable drone, high = dynamic |

---

## 2. Transient Analysis

### 2.1 Attack Time Detection

Attack time is the duration from the onset of a transient to its peak amplitude. It is one of the most informative features for distinguishing instrument types and driving visual sharpness.

**Envelope extraction:**

```
e(n) = alpha * |x(n)| + (1 - alpha) * e(n-1)
```

With `alpha_attack` (fast, ~0.01-0.1) and `alpha_release` (slow, ~0.9999) selected based on whether the signal is rising or falling:

```
if |x(n)| > e(n-1):
    alpha = alpha_attack
else:
    alpha = alpha_release
```

**Threshold crossing method:**

1. Detect onset when envelope exceeds `threshold_low` (e.g., 10% of recent peak).
2. Mark attack end when envelope exceeds `threshold_high` (e.g., 90% of the peak reached in this event).
3. Attack time `t_attack = (n_high - n_low) / sample_rate`.

**Logarithmic attack time (LAT):**

```
LAT = log10(t_attack)
```

LAT is perceptually more meaningful because human perception of attack sharpness is roughly logarithmic. Typical values:

| Source | t_attack (ms) | LAT |
|--------|--------------|-----|
| Click / impulse | 0.1-1 | -4.0 to -3.0 |
| Snare drum | 2-10 | -2.7 to -2.0 |
| Kick drum | 5-20 | -2.3 to -1.7 |
| Piano | 10-50 | -2.0 to -1.3 |
| Bowed string | 50-300 | -1.3 to -0.5 |
| Pad / drone | 300-3000 | -0.5 to 0.5 |

### 2.2 Decay Estimation

After the peak, the envelope decays. Model as exponential:

```
e(t) = A * exp(-t / tau) + floor
```

where `tau` is the decay time constant (time to fall to 1/e of peak) and `floor` is the noise floor.

**Real-time estimation:** Fit a linear regression to `log(e(t) - floor)` over a short window after the peak. The slope gives `-1/tau`.

For visualization, `tau` maps to trail length or fade time of visual elements.

### 2.3 Transient Density

Transient density `D_t` counts onsets per second in a sliding window:

```
D_t(t) = count(onsets in [t - W, t]) / W
```

where `W` is typically 1-3 seconds. This captures the "busyness" of the music.

| Content | Typical D_t (onsets/s) |
|---------|----------------------|
| Ambient drone | 0-1 |
| Slow ballad | 1-3 |
| Pop/rock | 4-8 |
| Fast punk/metal | 8-16 |
| Blast beat / drum & bass | 16-32 |
| Noise / glitch | 32+ |

```cpp
class TransientDensity {
    std::deque<double> onsetTimes_;  // timestamps of recent onsets
    double windowSec_;

public:
    explicit TransientDensity(double windowSec = 2.0) : windowSec_(windowSec) {}

    void addOnset(double timeSec) {
        onsetTimes_.push_back(timeSec);
    }

    float getDensity(double currentTimeSec) {
        // Evict old onsets
        while (!onsetTimes_.empty() &&
               onsetTimes_.front() < currentTimeSec - windowSec_) {
            onsetTimes_.pop_front();
        }
        return static_cast<float>(onsetTimes_.size()) / static_cast<float>(windowSec_);
    }
};
```

### 2.4 Transient Sharpness

Sharpness of a transient is characterized by:

1. **Rise time** (10%-90% of peak): shorter = sharper. See attack time above.
2. **Spectral centroid at onset**: higher centroid during the attack portion indicates a brighter, sharper transient (cymbal vs. kick).
3. **High-frequency energy ratio during attack**: `E_above_4kHz / E_total` in the first 5-10 ms after onset.

Combining these into a single sharpness metric:

```
sharpness = w1 * (1 / log10(t_attack + 0.001)) + w2 * (centroid / nyquist) + w3 * hf_ratio
```

Normalize to [0, 1] by empirical calibration or sigmoid mapping.

---

## 3. Zero Crossing Rate (ZCR)

### 3.1 Definition and Formula

ZCR counts the number of times the signal changes sign per unit time:

```
ZCR = (1 / (2N)) * sum_{n=1}^{N-1} |sign(x[n]) - sign(x[n-1])|
```

where `sign(x) = +1 if x >= 0, -1 if x < 0`. The factor `1/(2N)` normalizes to [0, 1] (each sign change contributes 2 to the absolute difference).

### 3.2 C++ Implementation

```cpp
float computeZCR(const float* samples, size_t numSamples) {
    if (numSamples < 2) return 0.0f;

    uint32_t crossings = 0;
    for (size_t i = 1; i < numSamples; ++i) {
        // Branchless: XOR of sign bits
        uint32_t a = *reinterpret_cast<const uint32_t*>(&samples[i]);
        uint32_t b = *reinterpret_cast<const uint32_t*>(&samples[i - 1]);
        crossings += (a ^ b) >> 31;
    }
    return static_cast<float>(crossings) / static_cast<float>(numSamples - 1);
}
```

The branchless XOR-of-sign-bits approach avoids branch mispredictions entirely. For a 2048-sample buffer this takes roughly 1-2 us on modern x86.

### 3.3 Interpretation

| Content Type | Typical ZCR (normalized) |
|-------------|-------------------------|
| Silence / DC | ~0 |
| Bass-heavy (sub, kick) | 0.01-0.05 |
| Voiced speech / singing | 0.05-0.15 |
| Mixed music | 0.10-0.30 |
| Unvoiced fricatives (s, sh) | 0.30-0.50 |
| Hi-hat / cymbal | 0.25-0.45 |
| White noise | ~0.50 |

ZCR is cheap to compute and provides a rough proxy for spectral centroid. High ZCR + low energy = noise/silence. High ZCR + high energy = bright percussive content. Low ZCR + high energy = bass/drone.

For visualization: ZCR can modulate texture detail level -- high ZCR drives fine-grained particles, low ZCR drives smooth geometry.

---

## 4. Roughness and Dissonance

### 4.1 Sethares Roughness Model

Roughness arises from the beating between closely spaced partials. For two sinusoids at frequencies `f1, f2` with amplitudes `a1, a2`:

```
r(f1, f2, a1, a2) = a1 * a2 * [exp(-b1 * s * |f2 - f1|) - exp(-b2 * s * |f2 - f1|)]
```

where:
- `s = d_max / (s1 * f_min + s2)` -- scaling factor for critical bandwidth
- `d_max = 0.24`, `s1 = 0.021`, `s2 = 19.0` (Sethares 1993)
- `b1 = 3.5`, `b2 = 5.75`
- `f_min = min(f1, f2)`

Total roughness for a set of N partials:

```
R = sum_{i=1}^{N} sum_{j=i+1}^{N} r(f_i, f_j, a_i, a_j)
```

### 4.2 Vassilakis Model (Improved Amplitude Weighting)

Vassilakis (2001) introduced a more perceptually accurate amplitude term:

```
r_v(f1, f2, a1, a2) = (a1 * a2)^0.1 * 0.5 * (2 * a_min / (a1 + a2))^3.11
                      * [exp(-b1 * s * |f2 - f1|) - exp(-b2 * s * |f2 - f1|)]
```

where `a_min = min(a1, a2)`. The `(a1*a2)^0.1` term captures loudness dependence, while the `(2*a_min/(a1+a2))^3.11` term models the masking effect where unequal amplitudes reduce perceived roughness.

### 4.3 Sensory Dissonance Curve

For two pure tones starting in unison and separating:
- **0 Hz separation**: no roughness (unison)
- **Peak roughness**: at approximately 25% of the critical bandwidth (`~0.25 * CB`)
- **Minimum roughness**: beyond ~1 critical bandwidth

The critical bandwidth (ERB approximation):

```
CB(f) = 24.7 * (4.37 * f / 1000 + 1)
```

At 500 Hz, CB ~ 79 Hz, so peak roughness occurs around 500 +/- 20 Hz. At 2000 Hz, CB ~ 240 Hz, peak roughness at ~60 Hz separation.

### 4.4 Real-Time Computation from Spectral Peaks

Full pairwise computation over all spectral bins is O(N^2) and infeasible in real-time. Instead:

1. **Peak-pick** the magnitude spectrum (find local maxima above a threshold). Typically yields 10-50 peaks.
2. Compute pairwise roughness only between peaks. With 30 peaks, that is 435 pairs -- very fast.
3. Optionally limit to peaks within 1 critical bandwidth of each other (pairs further apart contribute negligible roughness).

### 4.5 C++ Implementation

```cpp
struct SpectralPeak {
    float frequency;
    float amplitude;
};

float computeRoughness(const std::vector<SpectralPeak>& peaks) {
    constexpr float b1 = 3.5f, b2 = 5.75f;
    constexpr float d_max = 0.24f, s1 = 0.021f, s2 = 19.0f;

    float totalRoughness = 0.0f;

    for (size_t i = 0; i < peaks.size(); ++i) {
        for (size_t j = i + 1; j < peaks.size(); ++j) {
            float f1 = peaks[i].frequency;
            float f2 = peaks[j].frequency;
            float a1 = peaks[i].amplitude;
            float a2 = peaks[j].amplitude;

            float fMin = std::min(f1, f2);
            float fDiff = std::abs(f2 - f1);

            // Critical bandwidth scaling
            float s = d_max / (s1 * fMin + s2);

            // Vassilakis amplitude weighting
            float aMin = std::min(a1, a2);
            float aSum = a1 + a2 + 1e-10f;
            float ampTerm = std::pow(a1 * a2, 0.1f)
                          * 0.5f
                          * std::pow(2.0f * aMin / aSum, 3.11f);

            // Roughness kernel
            float r = ampTerm * (std::exp(-b1 * s * fDiff)
                               - std::exp(-b2 * s * fDiff));

            totalRoughness += std::max(0.0f, r);
        }
    }

    return totalRoughness;
}

// Peak picking from magnitude spectrum
std::vector<SpectralPeak> pickPeaks(const float* magnitudes, size_t numBins,
                                     float sampleRate, float threshold) {
    std::vector<SpectralPeak> peaks;
    float binWidth = sampleRate / (2.0f * (numBins - 1));

    for (size_t i = 1; i < numBins - 1; ++i) {
        if (magnitudes[i] > magnitudes[i - 1] &&
            magnitudes[i] > magnitudes[i + 1] &&
            magnitudes[i] > threshold) {
            // Parabolic interpolation for sub-bin frequency accuracy
            float alpha = magnitudes[i - 1];
            float beta  = magnitudes[i];
            float gamma = magnitudes[i + 1];
            float p = 0.5f * (alpha - gamma) / (alpha - 2.0f * beta + gamma);

            peaks.push_back({
                (static_cast<float>(i) + p) * binWidth,
                beta - 0.25f * (alpha - gamma) * p
            });
        }
    }
    return peaks;
}
```

Computational cost: peak picking is O(N_bins), roughness is O(P^2) where P is typically 20-50 peaks. Total: well under 100 us.

---

## 5. Fluctuation Strength

### 5.1 Temporal Modulation Spectrum

Fluctuation strength captures the perception of amplitude modulation (AM) at rates between approximately 1-20 Hz, peaking around 4 Hz (the rate of syllabic speech and typical rhythmic pulse).

The Fastl/Zwicker model defines fluctuation strength `F` in vacil:

```
F = 0.008 * integral[ (delta_L / (f_mod / 4 + 4 / f_mod)) ] df_bark
```

where:
- `delta_L` is the modulation depth in dB within a critical band
- `f_mod` is the modulation frequency in Hz
- The denominator peaks at `f_mod = 4 Hz`

### 5.2 Real-Time AM Detection

For each critical band (or ERB band):

1. Compute the band energy envelope `e_k(t)` for band `k` using a lowpass filter on the squared subband signal.
2. Compute the FFT of `e_k(t)` over a 1-2 second window to get the **modulation spectrum** `M_k(f_mod)`.
3. Weight by the fluctuation strength transfer function `W(f_mod) = 1 / (f_mod/4 + 4/f_mod)`.
4. Sum weighted modulation energy across bands.

The modulation spectrum reveals rhythmic structure: a strong peak at 2 Hz indicates a 120 BPM pulse; peaks at 4 Hz indicate 16th-note patterns at 120 BPM or 8th-notes at 240 BPM.

### 5.3 Relationship to Rhythm

| Modulation Rate | Musical Interpretation | Fluctuation Weight |
|----------------|----------------------|-------------------|
| 0.5 Hz | Very slow pulsation, ambient swell | 0.12 |
| 1 Hz | Slow tempo pulse (~60 BPM) | 0.20 |
| 2 Hz | Moderate tempo (~120 BPM) | 0.36 |
| 4 Hz | Fast pulse, peak perception | 1.00 |
| 8 Hz | Tremolo, fast rhythmic subdivision | 0.36 |
| 16 Hz | Transition to roughness perception | 0.12 |
| 32+ Hz | Roughness domain, not fluctuation | ~0 |

For the VJ engine, fluctuation strength at the dominant modulation rate can modulate the visual "breathing" rate -- matching visual oscillations to the rhythmic modulation of the audio.

### 5.4 C++ Sketch

```cpp
class FluctuationStrength {
    static constexpr size_t kModFFTSize = 256; // ~2 sec at 128 Hz envelope rate
    size_t numBands_;
    std::vector<std::vector<float>> envelopeBuffers_; // per-band ring buffers

    float fluctuationWeight(float fMod) const {
        return 1.0f / (fMod / 4.0f + 4.0f / (fMod + 1e-6f));
    }

public:
    explicit FluctuationStrength(size_t numBands)
        : numBands_(numBands),
          envelopeBuffers_(numBands, std::vector<float>(kModFFTSize, 0.0f)) {}

    // Call per frame with band energies
    float compute(const float* bandEnergies, float envelopeRate) {
        // Update ring buffers, compute modulation FFT per band,
        // weight by fluctuation curve, sum.
        // (Full FFT-based implementation follows standard pattern.)

        float totalFluctuation = 0.0f;
        float binWidth = envelopeRate / kModFFTSize;

        for (size_t band = 0; band < numBands_; ++band) {
            // After computing modulation spectrum modSpec[] for this band:
            for (size_t m = 1; m < kModFFTSize / 2; ++m) {
                float fMod = m * binWidth;
                if (fMod >= 0.5f && fMod <= 32.0f) {
                    float modSpec_m = 0.0f; // placeholder: magnitude of modulation FFT bin
                    totalFluctuation += modSpec_m * fluctuationWeight(fMod);
                }
            }
        }

        return 0.008f * totalFluctuation;
    }
};
```

---

## 6. Textural Complexity Metrics

### 6.1 Spectral Irregularity Over Time

Spectral irregularity measures how much the spectral envelope deviates from a smooth shape. The Jensen (1999) formulation:

```
IRR = sum_{k=2}^{N-1} |mag[k] - (mag[k-1] + mag[k] + mag[k+1]) / 3|^2
      / sum_{k=1}^{N} mag[k]^2
```

Tracking `IRR` over time: a solo flute produces low irregularity (smooth harmonic series); a distorted guitar or noise produces high irregularity. The temporal variance of IRR itself is a second-order complexity metric.

### 6.2 Feature Variance as Complexity Proxy

For any feature `F(t)`, the variance over a rolling window is a measure of how much the music is "changing":

```
complexity_F = var(F(t), ..., F(t-W)) / (mean(F(t), ..., F(t-W))^2 + epsilon)
```

This is the coefficient of variation squared. Good candidates for `F`:

- Spectral centroid (timbral change)
- Spectral flux (onset activity)
- MFCC vectors (timbral distance)
- Chroma vectors (harmonic change)

A composite complexity metric averages the normalized variances of multiple features:

```
C(t) = mean(norm_var(centroid), norm_var(flux), norm_var(mfcc_dist), norm_var(zcr))
```

### 6.3 Entropy-Based Complexity

**Spectral entropy** (see also FEATURES_spectral.md):

```
H_spectral = -sum_{k} p(k) * log2(p(k))
```

where `p(k) = |S(k)|^2 / sum |S(k)|^2`.

**Temporal entropy**: Quantize the amplitude envelope into `M` levels, build a histogram over a sliding window, and compute entropy of the histogram. High temporal entropy = unpredictable dynamics = complex.

**Joint spectro-temporal entropy**: Compute the 2D entropy of a time-frequency patch. This captures both spectral and temporal unpredictability simultaneously but is more expensive.

```cpp
float spectralEntropy(const float* magnitudes, size_t numBins) {
    float totalEnergy = 0.0f;
    for (size_t i = 0; i < numBins; ++i)
        totalEnergy += magnitudes[i] * magnitudes[i];

    if (totalEnergy < 1e-20f) return 0.0f;

    float entropy = 0.0f;
    float invTotal = 1.0f / totalEnergy;

    for (size_t i = 0; i < numBins; ++i) {
        float p = magnitudes[i] * magnitudes[i] * invTotal;
        if (p > 1e-10f) {
            entropy -= p * std::log2(p);
        }
    }

    // Normalize to [0, 1] by dividing by log2(numBins)
    return entropy / std::log2(static_cast<float>(numBins));
}
```

### 6.4 Polyphony Estimation

Polyphony -- the number of simultaneous pitched voices -- can be estimated by:

1. **Peak counting in the spectrum**: After harmonic grouping (assigning overtones to fundamentals), count distinct fundamental groups. This is hard and error-prone.
2. **Chroma energy spread**: Count chroma bins above a threshold. More active bins = more polyphony.
3. **NMF rank estimation**: Non-negative matrix factorization of the spectrogram with automatic rank selection (e.g., via BIC or residual error threshold). The rank approximates the number of sources. Too expensive for real-time.

A practical real-time proxy:

```
polyphony_est = count(chroma[k] > threshold * max(chroma), k = 0..11)
```

With `threshold = 0.3`, this yields 1-2 for monophonic melody, 3-4 for simple chords, 6-8 for dense voicings.

---

## 7. Noise vs. Tonal Content

### 7.1 Spectral Flatness for Noise Detection

Spectral flatness (Wiener entropy) is the ratio of the geometric mean to the arithmetic mean of the power spectrum:

```
SF = (prod_{k} P(k))^{1/N} / (1/N * sum_{k} P(k))
```

In log domain (more numerically stable):

```
SF = exp((1/N) * sum log(P(k))) / ((1/N) * sum P(k))
```

- SF = 1.0: white noise (perfectly flat spectrum)
- SF = 0.0: pure tone (all energy in one bin)
- Typical music: 0.01-0.3

```cpp
float spectralFlatness(const float* powerSpectrum, size_t numBins) {
    float logSum = 0.0f;
    float linearSum = 0.0f;
    size_t validBins = 0;

    for (size_t i = 0; i < numBins; ++i) {
        float p = powerSpectrum[i] + 1e-20f; // avoid log(0)
        logSum += std::log(p);
        linearSum += p;
        ++validBins;
    }

    float geometricMean = std::exp(logSum / validBins);
    float arithmeticMean = linearSum / validBins;

    return geometricMean / (arithmeticMean + 1e-20f);
}
```

### 7.2 Harmonic-to-Noise Ratio (HNR)

HNR measures how much of the signal energy is in harmonic partials vs. noise:

```
HNR_dB = 10 * log10(E_harmonic / E_noise)
```

Estimation approaches:

1. **Autocorrelation method**: The peak of the normalized autocorrelation at the fundamental period gives a ratio related to HNR. `HNR = r_max / (1 - r_max)` where `r_max` is the autocorrelation at the pitch lag.
2. **Cepstral method**: Lifter the cepstrum to separate harmonic (quefrency peaks) from noise (smooth cepstral floor). The energy ratio gives HNR.
3. **Spectral method**: After pitch detection, sum energy at harmonic multiples of f0 (within a tolerance) as `E_harmonic`, and everything else as `E_noise`.

Typical values:

| Source | HNR (dB) |
|--------|----------|
| Breathy whisper | 0-5 |
| Normal speech | 10-20 |
| Clean singing | 20-30 |
| Synthesizer sawtooth | 30-40 |
| Pure sine | 40+ |
| White noise | -infinity |

### 7.3 Noise Floor Estimation

Estimate the noise floor using the **minimum statistics** approach (Martin 1994):

1. Divide the spectrum into overlapping blocks of D frames.
2. Track the minimum power in each bin across the block. The minimum is biased toward the noise floor because speech/music energy fluctuates above it.
3. Apply a bias correction factor (the minimum of D samples from an exponential distribution has expectation `P_noise * correction(D)`).

Alternatively, use the **MMSE noise estimator** (Ephraim-Malah) which tracks noise during speech pauses. For a VJ engine, the simpler minimum-statistics approach is sufficient.

### 7.4 Tonal-to-Noise Ratio Per Band

For each ERB or Bark band:

```
TNR_k = 10 * log10(E_tonal_k / E_noise_k)
```

where `E_tonal_k` is estimated from spectral peaks within the band and `E_noise_k` is the remaining energy. This gives a per-band map of tonal vs. noisy content, useful for driving per-band visual effects (e.g., the bass band might be tonal while the treble is noisy).

---

## 8. Percussive Element Isolation

### 8.1 Using HPSS Output

The percussive component from HPSS (Section 1) provides an isolated percussive signal. From this, specific drum elements can be detected via band-limited energy analysis.

### 8.2 Kick Drum Detection

Kick drums concentrate energy in the 40-120 Hz range with a sharp attack and relatively fast decay.

```cpp
struct DrumDetector {
    // Band energy thresholds (empirically tuned)
    float kickLowHz  = 40.0f;
    float kickHighHz = 120.0f;

    float snareLowHz  = 150.0f;
    float snareHighHz = 1000.0f;

    float hihatLowHz  = 6000.0f;
    float hihatHighHz = 16000.0f;

    struct DrumHits {
        float kickEnergy;
        float snareEnergy;
        float hihatEnergy;
        bool kickDetected;
        bool snareDetected;
        bool hihatDetected;
    };

    DrumHits detect(const float* percussiveMagnitudes, size_t numBins,
                    float sampleRate, float kickThreshold,
                    float snareThreshold, float hihatThreshold) {
        DrumHits hits = {};
        float binWidth = sampleRate / (2.0f * (numBins - 1));

        for (size_t i = 0; i < numBins; ++i) {
            float freq = i * binWidth;
            float mag2 = percussiveMagnitudes[i] * percussiveMagnitudes[i];

            if (freq >= kickLowHz && freq <= kickHighHz)
                hits.kickEnergy += mag2;
            if (freq >= snareLowHz && freq <= snareHighHz)
                hits.snareEnergy += mag2;
            if (freq >= hihatLowHz && freq <= hihatHighHz)
                hits.hihatEnergy += mag2;
        }

        // Onset detection on each band (compare to previous frame)
        // In production: use spectral flux per band with adaptive threshold
        hits.kickDetected   = hits.kickEnergy > kickThreshold;
        hits.snareDetected  = hits.snareEnergy > snareThreshold;
        hits.hihatDetected  = hits.hihatEnergy > hihatThreshold;

        return hits;
    }
};
```

### 8.3 Snare Detection

Snare drums occupy a wider band (~150-1000 Hz for the body, with a noise burst extending to 5-10 kHz from the snare wires). Detection uses:

1. Mid-band energy spike (from HPSS percussive component).
2. Simultaneously high broadband noise (check spectral flatness of the percussive component in the 1-10 kHz range). A spectral flatness > 0.3 in this range during a percussive onset strongly indicates snare.

### 8.4 Hi-Hat Detection

Hi-hats produce energy primarily above 5-6 kHz with high spectral flatness (metallic noise). Open hi-hats have longer decay; closed hi-hats have very short decay (< 50 ms).

Detection criteria:
- Percussive onset detected in the HPSS percussive stream
- High-frequency energy ratio > 0.5 (most energy above 6 kHz)
- Very short attack (< 5 ms)
- Decay time discriminates open vs. closed

### 8.5 Drum Element Feature Summary

| Element | Frequency Range | Attack Time | Decay Time | Spectral Flatness |
|---------|----------------|-------------|------------|-------------------|
| Kick | 40-120 Hz | 5-20 ms | 50-200 ms | Low (0.05-0.15) |
| Snare | 150-1000 Hz (body) + 1-10 kHz (wires) | 2-10 ms | 50-150 ms | Medium (0.2-0.4) |
| Tom | 80-400 Hz | 5-15 ms | 100-500 ms | Low (0.05-0.15) |
| Hi-hat (closed) | 6-16 kHz | 1-5 ms | 10-50 ms | High (0.4-0.7) |
| Hi-hat (open) | 6-16 kHz | 1-5 ms | 200-1000 ms | High (0.4-0.7) |
| Crash cymbal | 3-16 kHz | 1-5 ms | 500-3000 ms | High (0.5-0.8) |

---

## 9. Visual Mapping

This section summarizes how transient/texture features map to visual parameters. See [VIDEO_feature_to_visual_mapping.md](VIDEO_feature_to_visual_mapping.md) for the full mapping table.

### 9.1 Transient Density --> Particle Emission Rate

```
emission_rate = base_rate * (1 + gain * density)
```

| Density Range | Visual Behavior |
|--------------|----------------|
| 0-2/s | Sparse, slow-moving particles. Meditative feel. |
| 2-8/s | Moderate bursts synchronized to beats. |
| 8-16/s | Dense particle streams, constant visual activity. |
| 16+/s | Overwhelming flood; consider clamping or switching to noise texture. |

Apply a smoothing filter (`alpha = 0.1`) to prevent visual flicker from onset detection jitter.

### 9.2 Roughness --> Distortion / Glitch Effects

```
glitch_intensity = sigmoid(roughness * gain - bias)
```

Low roughness (consonant harmony) maps to clean, smooth visuals. High roughness (dissonant clusters, distortion) maps to:
- Fragment / glitch shader intensity
- Chromatic aberration amount
- Displacement map amplitude
- Edge detection / posterization strength

### 9.3 HPSS Ratio --> Texture vs. Geometry

The harmonic-percussive ratio provides a continuous axis:

| HP Ratio | Visual Mode |
|----------|------------|
| 0.0 (pure percussive) | Hard geometric shapes, sharp edges, flash/strobe |
| 0.3 (percussive-dominant) | Angular geometry with some organic motion |
| 0.5 (balanced) | Mixed mode: flowing geometry with rhythmic punctuation |
| 0.7 (harmonic-dominant) | Smooth organic shapes, gradual color transitions |
| 1.0 (pure harmonic) | Fluid simulation, plasma, aurora-like continuous textures |

### 9.4 Additional Mappings

| Audio Feature | Visual Parameter | Mapping |
|--------------|-----------------|---------|
| Attack time (LAT) | Flash/strobe duration | Inverse: fast attack = short bright flash |
| Decay time (tau) | Trail / fade length | Linear: long decay = long visual trails |
| ZCR | Texture detail / grain size | Linear: high ZCR = fine grain |
| Spectral flatness | Visual noise overlay amount | Linear: flat spectrum = noisy visuals |
| Fluctuation strength | Visual breathing / pulsation rate | Match modulation rate to visual oscillation |
| Textural complexity | Number of simultaneous visual layers | Stepped: low = 1-2 layers, high = 4-6 layers |
| Drum element | Specific visual event | Kick = camera shake, snare = flash, hi-hat = sparkle |

### 9.5 Combined Feature Routing (C++ Sketch)

```cpp
struct VisualParams {
    float particleEmissionRate;
    float glitchIntensity;
    float geometrySmoothness;  // 0 = hard edges, 1 = fluid
    float flashIntensity;
    float trailLength;
    float textureGrain;
    float noiseOverlay;
    float breathingRate;
    float cameraShake;
};

VisualParams mapFeaturesToVisuals(
    float transientDensity,
    float roughness,
    float hpRatio,
    float attackTimeSec,
    float decayTimeSec,
    float zcr,
    float spectralFlatness,
    float fluctuationStrength,
    float kickEnergy,
    bool kickDetected)
{
    VisualParams vp;

    // Particle emission: 10 base + density scaling
    vp.particleEmissionRate = 10.0f + 50.0f * std::min(transientDensity / 16.0f, 1.0f);

    // Glitch: sigmoid of roughness
    vp.glitchIntensity = 1.0f / (1.0f + std::exp(-10.0f * (roughness - 0.3f)));

    // Geometry smoothness from HP ratio
    vp.geometrySmoothness = hpRatio;

    // Flash from attack time (inverse log)
    float lat = std::log10(attackTimeSec + 0.0001f);
    vp.flashIntensity = std::clamp((-lat - 1.5f) / 2.0f, 0.0f, 1.0f);

    // Trail length from decay
    vp.trailLength = std::clamp(decayTimeSec / 2.0f, 0.0f, 1.0f);

    // Texture grain from ZCR
    vp.textureGrain = zcr;

    // Noise overlay from spectral flatness
    vp.noiseOverlay = spectralFlatness;

    // Breathing rate from fluctuation
    vp.breathingRate = fluctuationStrength * 4.0f; // Scale to visual Hz

    // Camera shake from kick
    vp.cameraShake = kickDetected ? std::min(kickEnergy * 0.01f, 1.0f) : 0.0f;

    return vp;
}
```

---

## Computational Budget Summary

| Feature | Cost per Frame (2048 samples) | Latency Added |
|---------|------------------------------|---------------|
| HPSS (causal, running median) | 50-80 us | 0 ms (causal) |
| Attack/decay detection | 5-10 us | 0 ms |
| Transient density | < 1 us | 0 ms |
| ZCR | 1-2 us | 0 ms |
| Roughness (30 peaks) | 10-20 us | 0 ms |
| Fluctuation strength | 30-50 us | ~1 s (needs modulation window) |
| Spectral flatness | 5 us | 0 ms |
| Spectral entropy | 5 us | 0 ms |
| Drum detection (3 bands) | 5-10 us | 0 ms |
| **Total** | **~110-180 us** | **0-1 s** |

All features combined consume well under 200 us per frame, leaving ample headroom within the ~23 ms frame budget at 44.1 kHz / 1024 hop.

---

## Cross-References

- **[FEATURES_rhythm_tempo.md](FEATURES_rhythm_tempo.md)**: Onset detection functions feed both rhythm tracking and the transient density metric here. Fluctuation strength's dominant modulation rate relates to tempo estimation.
- **[FEATURES_spectral.md](FEATURES_spectral.md)**: Spectral centroid, flux, and rolloff are inputs to textural complexity. Spectral flatness is defined there in full; this document covers its noise-detection interpretation.
- **[FEATURES_psychoacoustic.md](FEATURES_psychoacoustic.md)**: Roughness and fluctuation strength are psychoacoustic descriptors. Critical band definitions and loudness models from that document apply here.
- **[VIDEO_feature_to_visual_mapping.md](VIDEO_feature_to_visual_mapping.md)**: Section 9 summarizes mappings; the full mapping document provides interpolation curves, priority/arbitration when features conflict, and GPU shader parameter binding.
- **[ARCH_pipeline.md](ARCH_pipeline.md)**: HPSS and drum detection sit in the "source separation" pipeline stage, after FFT and before feature extraction. The pipeline document defines threading and buffer management.
