# Spectral Feature Extraction for Real-Time Music Visualization

> **Scope**: Exhaustive reference for every spectral feature extractable in real-time (sub-16ms latency budget) targeting 30--60 fps visual update rates in a VJ / music-visualization engine.
>
> **Cross-references**: [FEATURES_frequency_bands.md](FEATURES_frequency_bands.md) | [FEATURES_mfcc_mel.md](FEATURES_mfcc_mel.md) | [LIB_fft_comparison.md](LIB_fft_comparison.md) | [REF_math_reference.md](REF_math_reference.md) | [VIDEO_feature_to_visual_mapping.md](VIDEO_feature_to_visual_mapping.md)

---

## 1. FFT Setup for Real-Time Audio

### 1.1 Window Functions

Windowing eliminates discontinuities at frame boundaries that would otherwise smear energy across all bins (spectral leakage). Every window is a tradeoff between **main-lobe width** (frequency resolution) and **side-lobe level** (leakage suppression).

The general form applied element-wise before the FFT:

```
x_w[n] = x[n] * w[n],   0 <= n < N
```

#### 1.1.1 Hann (Hanning)

```
w[n] = 0.5 * (1 - cos(2*pi*n / (N-1)))
```

| Property | Value |
|---|---|
| Main-lobe width (-3 dB) | 1.44 bins |
| First side-lobe | -31.5 dB |
| Side-lobe rolloff | -18 dB/octave |
| Coherent gain | 0.50 |
| ENBW | 1.50 bins |

**Use case**: Default choice for spectral feature extraction. Good balance between resolution and leakage. The 50% overlap produces constant-power (COLA) reconstruction, which matters if you ever need to resynthesize. Most spectral centroid / flux / rolloff literature assumes Hann windowing.

```cpp
void hann_window(float* w, int N) {
    const double k = 2.0 * M_PI / (N - 1);
    for (int n = 0; n < N; ++n)
        w[n] = 0.5f * (1.0f - std::cos(k * n));
}
```

#### 1.1.2 Hamming

```
w[n] = 0.54 - 0.46 * cos(2*pi*n / (N-1))
```

| Property | Value |
|---|---|
| Main-lobe width (-3 dB) | 1.30 bins |
| First side-lobe | -42.7 dB |
| Side-lobe rolloff | -6 dB/octave (flat far-out) |
| Coherent gain | 0.54 |
| ENBW | 1.36 bins |

**Use case**: Better peak side-lobe than Hann, but the side-lobes roll off more slowly. Historically popular in speech processing (MFCC pipelines). Use when you need slightly sharper main lobe and can tolerate far-out leakage. Not COLA at 50% overlap (use 75% or accept ~0.08 dB ripple).

```cpp
void hamming_window(float* w, int N) {
    const double k = 2.0 * M_PI / (N - 1);
    for (int n = 0; n < N; ++n)
        w[n] = 0.54f - 0.46f * std::cos(k * n);
}
```

#### 1.1.3 Blackman

```
w[n] = 0.42 - 0.5*cos(2*pi*n/(N-1)) + 0.08*cos(4*pi*n/(N-1))
```

| Property | Value |
|---|---|
| Main-lobe width (-3 dB) | 1.68 bins |
| First side-lobe | -58.1 dB |
| Side-lobe rolloff | -18 dB/octave |
| Coherent gain | 0.42 |
| ENBW | 1.73 bins |

**Use case**: When you need cleaner spectral floor (e.g., spectral flatness measurement where leakage from strong tones corrupts the noise-floor estimate). Wider main lobe costs frequency resolution -- acceptable at N >= 4096.

```cpp
void blackman_window(float* w, int N) {
    const double k = 2.0 * M_PI / (N - 1);
    for (int n = 0; n < N; ++n)
        w[n] = 0.42f - 0.5f * std::cos(k * n) + 0.08f * std::cos(2.0 * k * n);
}
```

#### 1.1.4 Blackman-Harris (4-term)

```
w[n] = a0 - a1*cos(2*pi*n/(N-1)) + a2*cos(4*pi*n/(N-1)) - a3*cos(6*pi*n/(N-1))
a0=0.35875, a1=0.48829, a2=0.14128, a3=0.01168
```

| Property | Value |
|---|---|
| Main-lobe width (-3 dB) | 1.90 bins |
| First side-lobe | -92.0 dB |
| Side-lobe rolloff | -6 dB/octave |
| Coherent gain | 0.36 |
| ENBW | 2.00 bins |

**Use case**: Excellent side-lobe suppression. Use for spectral contrast and spectral irregularity where you need the cleanest possible separation between peaks and valleys. The wide main lobe is acceptable because these features operate on coarse band structure, not individual bin resolution.

```cpp
void blackman_harris_window(float* w, int N) {
    constexpr double a0 = 0.35875, a1 = 0.48829, a2 = 0.14128, a3 = 0.01168;
    const double k = 2.0 * M_PI / (N - 1);
    for (int n = 0; n < N; ++n)
        w[n] = float(a0 - a1*std::cos(k*n) + a2*std::cos(2*k*n) - a3*std::cos(3*k*n));
}
```

#### 1.1.5 Kaiser

```
w[n] = I0(beta * sqrt(1 - ((2n/(N-1)) - 1)^2)) / I0(beta)
```

where `I0` is the zeroth-order modified Bessel function of the first kind.

| beta | Approx. side-lobe (dB) | Main-lobe width (bins) |
|---|---|---|
| 4.0 | -30 | 1.4 |
| 6.0 | -50 | 1.8 |
| 8.6 | -70 | 2.3 |
| 12.0 | -90 | 2.9 |
| 14.0 | -100 | 3.3 |

**Use case**: The only continuously-tunable window. Set `beta` to dial the exact resolution/leakage tradeoff for your application. Compute `I0` via the polynomial series (converges in ~20 terms for beta <= 15). Expensive to compute -- precompute and cache.

```cpp
static double bessel_i0(double x) {
    double sum = 1.0, term = 1.0;
    for (int k = 1; k <= 25; ++k) {
        term *= (x * x) / (4.0 * k * k);
        sum += term;
        if (term < 1e-12 * sum) break;
    }
    return sum;
}

void kaiser_window(float* w, int N, double beta) {
    const double denom = bessel_i0(beta);
    const double Nm1 = N - 1;
    for (int n = 0; n < N; ++n) {
        double ratio = (2.0 * n / Nm1) - 1.0;
        w[n] = float(bessel_i0(beta * std::sqrt(1.0 - ratio * ratio)) / denom);
    }
}
```

#### 1.1.6 Flat-Top

```
w[n] = a0 - a1*cos(2*pi*n/(N-1)) + a2*cos(4*pi*n/(N-1)) - a3*cos(6*pi*n/(N-1)) + a4*cos(8*pi*n/(N-1))
a0=0.21557895, a1=0.41663158, a2=0.277263158, a3=0.083578947, a4=0.006947368
```

| Property | Value |
|---|---|
| Main-lobe width (-3 dB) | 3.72 bins |
| First side-lobe | -93.0 dB |
| Amplitude error | < 0.01 dB |
| Coherent gain | 0.22 |
| ENBW | 3.77 bins |

**Use case**: Amplitude-accurate measurement -- the flat main lobe means a tone's measured amplitude is correct regardless of where it falls between bins. Poor frequency resolution. Use for calibration or when you need exact dB values per-bin (e.g., driving a precise spectrum analyzer visualization). Not typical for feature extraction.

```cpp
void flat_top_window(float* w, int N) {
    constexpr double a0=0.21557895, a1=0.41663158, a2=0.277263158,
                     a3=0.083578947, a4=0.006947368;
    const double k = 2.0 * M_PI / (N - 1);
    for (int n = 0; n < N; ++n)
        w[n] = float(a0 - a1*std::cos(k*n) + a2*std::cos(2*k*n)
                     - a3*std::cos(3*k*n) + a4*std::cos(4*k*n));
}
```

#### Window Comparison Summary

| Window | Main Lobe (bins) | Side Lobe (dB) | Best For |
|---|---|---|---|
| Hann | 1.44 | -31.5 | General spectral features (default) |
| Hamming | 1.30 | -42.7 | MFCCs, speech |
| Blackman | 1.68 | -58.1 | Spectral flatness, low-SNR |
| Blackman-Harris | 1.90 | -92.0 | Spectral contrast, clean valleys |
| Kaiser (beta=8.6) | 2.3 | -70 | Tunable tradeoff |
| Flat-Top | 3.72 | -93.0 | Amplitude calibration |

### 1.2 Overlap and Hop Size

**Overlap** determines how many samples consecutive frames share. **Hop size** `H = N - overlap_samples = N * (1 - overlap_fraction)`.

| Overlap | Hop (N=4096, fs=48000) | Hops/sec | Use Case |
|---|---|---|---|
| 50% | 2048 (42.7 ms) | 23.4 | Standard COLA for Hann. Adequate for 30 fps viz. |
| 75% | 1024 (21.3 ms) | 46.9 | Smoother features, 60 fps viz. Required for COLA with Hamming. |
| 87.5% | 512 (10.7 ms) | 93.8 | Onset detection, high-rate spectral flux. |

**Why overlap?** Windowing attenuates samples near frame edges. Without overlap, transients falling at frame boundaries are suppressed. 50% overlap with Hann ensures every sample contributes at full weight in at least one frame (COLA property: sum of overlapping windows = constant). 75% overlap gives smoother temporal evolution of features -- critical for visual smoothness at 60 fps.

**Hop size and visual frame rate**: At 48 kHz with N=4096, 75% overlap yields ~46.9 spectral frames per second. This aligns well with 30 or 60 fps rendering (decimate or interpolate as needed -- see Section 5).

### 1.3 Zero-Padding

Append zeros to the windowed frame before the FFT to increase the DFT length without adding new information. This interpolates the magnitude spectrum (smoother peaks) but does **not** improve true frequency resolution.

```cpp
// N_fft > N_window: zero-padded
std::vector<float> padded(N_fft, 0.0f);
std::memcpy(padded.data(), windowed.data(), N_window * sizeof(float));
// Feed padded[] to FFT
```

**When to use**: Visualizations that display raw spectrum benefit from 2x zero-padding (N_fft = 2*N_window) to fill in between bins for smoother spectrum plots. For feature extraction (centroid, flux, etc.) zero-padding has negligible effect on computed values -- skip it to save cycles.

### 1.4 Bin Frequency Calculation

For a real-valued FFT of length `N_fft` at sample rate `fs`:
- Output bins: `k = 0, 1, ..., N_fft/2` (i.e., `N_fft/2 + 1` complex values)
- Frequency of bin k: `f_k = k * fs / N_fft`
- Frequency resolution (bin width): `delta_f = fs / N_fft`

```cpp
float bin_to_freq(int k, float fs, int N_fft) {
    return k * fs / N_fft;
}

int freq_to_bin(float f, float fs, int N_fft) {
    return std::lround(f * N_fft / fs);
}
```

| N_fft | fs=44100 | fs=48000 | Bins (real) |
|---|---|---|---|
| 1024 | 43.1 Hz | 46.9 Hz | 513 |
| 2048 | 21.5 Hz | 23.4 Hz | 1025 |
| 4096 | 10.8 Hz | 11.7 Hz | 2049 |
| 8192 | 5.4 Hz | 5.9 Hz | 4097 |

### 1.5 Frequency Resolution vs Time Resolution Tradeoff

This is the Gabor limit -- the DSP analog of the Heisenberg uncertainty principle:

```
delta_t * delta_f >= 1/(4*pi)
```

In practical FFT terms:
- **Time resolution** = hop size in seconds = H / fs
- **Frequency resolution** = fs / N_fft

Doubling N_fft halves frequency resolution (better) but doubles the frame duration (worse time resolution). For music visualization:

| Priority | Recommended N | Hop | delta_f | delta_t |
|---|---|---|---|---|
| Bass accuracy (< 100 Hz) | 8192 | 2048 | 5.9 Hz | 42.7 ms |
| General music | 4096 | 1024 | 11.7 Hz | 21.3 ms |
| Percussive / transient-heavy | 2048 | 512 | 23.4 Hz | 10.7 ms |

A common strategy: run two FFTs in parallel -- a long one (8192) for bass features and a short one (2048) for onset/transient features. See [FEATURES_frequency_bands.md](FEATURES_frequency_bands.md) for multi-resolution band extraction.

### 1.6 Hop Size Considerations for Visual Engines

The audio callback delivers buffers of fixed size (commonly 128, 256, or 512 samples). The FFT frame size N is typically larger (2048--8192). You must accumulate audio samples in a ring buffer and trigger FFT computation when enough new samples arrive (i.e., one hop's worth).

```cpp
class FrameAccumulator {
    std::vector<float> ring_;
    int write_pos_ = 0;
    int accumulated_ = 0;
    int frame_size_;
    int hop_size_;

public:
    FrameAccumulator(int frame_size, int hop_size)
        : ring_(frame_size, 0.0f), frame_size_(frame_size), hop_size_(hop_size) {}

    // Returns true when a new frame is ready
    bool push(const float* samples, int count) {
        for (int i = 0; i < count; ++i) {
            ring_[write_pos_] = samples[i];
            write_pos_ = (write_pos_ + 1) % frame_size_;
            ++accumulated_;
        }
        if (accumulated_ >= hop_size_) {
            accumulated_ -= hop_size_;
            return true;
        }
        return false;
    }

    void get_frame(float* out) const {
        // Read frame_size_ samples ending at write_pos_
        for (int i = 0; i < frame_size_; ++i)
            out[i] = ring_[(write_pos_ - frame_size_ + i + ring_.size()) % ring_.size()];
    }
};
```

---

## 2. Spectral Features

All features below operate on the magnitude spectrum `|X[k]|` for `k = 0 .. N/2`, where `X[k]` is the complex DFT output. We use the following notation throughout:

- `M[k] = |X[k]|` -- magnitude spectrum
- `P[k] = |X[k]|^2` -- power spectrum
- `f_k = k * fs / N` -- frequency of bin k
- `K = N/2` -- number of usable bins (excluding DC when noted)
- `S = sum(M[k], k=1..K)` -- total magnitude (excluding DC)

### 2.1 Spectral Centroid

**What it measures**: The "center of mass" of the spectrum. Correlates strongly with perceived brightness. A sine wave at 1 kHz has centroid = 1000 Hz. White noise has centroid near fs/4.

**Formula**:

```
centroid = sum(f_k * M[k], k=1..K) / sum(M[k], k=1..K)
```

**Typical range**: 200--8000 Hz for music (higher for cymbals/hi-hats, lower for bass-heavy material).

**Visual mapping**: Map to color temperature (warm-to-cool), particle size (larger = brighter spectrum), vertical position, or blur radius. Normalize to [0,1] via `(centroid - f_min) / (f_max - f_min)` where f_min/f_max are chosen empirically (e.g., 100 Hz / 10 kHz).

```cpp
float spectral_centroid(const float* mag, int num_bins, float fs, int N_fft) {
    float weighted_sum = 0.0f, total_mag = 0.0f;
    for (int k = 1; k < num_bins; ++k) {
        float freq = k * fs / N_fft;
        weighted_sum += freq * mag[k];
        total_mag += mag[k];
    }
    return (total_mag > 1e-10f) ? weighted_sum / total_mag : 0.0f;
}
```

### 2.2 Spectral Flux

**What it measures**: Frame-to-frame change in the magnitude spectrum. Strongly correlated with perceived onsets -- a sudden increase in spectral energy across many bins indicates a new note or percussion hit.

**Formula (L2 norm, half-wave rectified)**:

```
flux = sum(max(0, M[k] - M_prev[k])^2, k=1..K)
```

Half-wave rectification (the `max(0, ...)`) ensures only *increases* in energy contribute. This prevents offsets (energy decay) from triggering false onsets. Some implementations use the full (unrectified) L2 norm; half-rectified is better for onset detection.

**Typical range**: 0 to ~10^6 (unnormalized, depends on magnitude scale). Normalize by dividing by K, or use log scale.

**Visual mapping**: Triggers (flash, pulse, particle burst) when flux exceeds a running adaptive threshold. Smooth for continuous brightness modulation.

```cpp
float spectral_flux(const float* mag, const float* mag_prev, int num_bins) {
    float flux = 0.0f;
    for (int k = 1; k < num_bins; ++k) {
        float diff = mag[k] - mag_prev[k];
        if (diff > 0.0f)
            flux += diff * diff;
    }
    return flux;
}

// Adaptive threshold for onset detection
struct FluxOnsetDetector {
    float median_buf[16] = {};
    int buf_idx = 0;
    float threshold_multiplier = 1.5f;

    bool is_onset(float flux) {
        median_buf[buf_idx++ % 16] = flux;
        // Compute running median
        float sorted[16];
        std::copy(std::begin(median_buf), std::end(median_buf), sorted);
        std::nth_element(sorted, sorted + 8, sorted + 16);
        float median = sorted[8];
        return flux > median * threshold_multiplier;
    }
};
```

### 2.3 Spectral Rolloff

**What it measures**: The frequency below which a given percentage (typically 85% or 95%) of the total spectral energy is contained. A bandwidth measure -- tonal content (e.g., flute) has low rolloff; noisy content (e.g., distorted guitar, hi-hat) has high rolloff.

**Formula**:

```
rolloff = f_k  such that  sum(M[j], j=1..k) >= threshold * sum(M[j], j=1..K)
```

**Typical range**: 500--15000 Hz for music. Threshold of 0.85 is standard in MIR literature; 0.95 captures more of the spectral tail.

**Visual mapping**: Aperture/zoom of a camera effect, trail length, fog density.

```cpp
float spectral_rolloff(const float* mag, int num_bins, float fs, int N_fft,
                       float threshold = 0.85f) {
    float total = 0.0f;
    for (int k = 1; k < num_bins; ++k) total += mag[k];
    float cumsum = 0.0f;
    float target = threshold * total;
    for (int k = 1; k < num_bins; ++k) {
        cumsum += mag[k];
        if (cumsum >= target)
            return k * fs / N_fft;
    }
    return fs / 2.0f; // Nyquist
}
```

### 2.4 Spectral Flatness (Wiener Entropy)

**What it measures**: How "noise-like" vs "tonal" the spectrum is. Ratio of geometric mean to arithmetic mean of the power spectrum. A pure tone yields ~0; white noise yields ~1.

**Formula**:

```
flatness = exp(mean(ln(P[k]))) / mean(P[k])
         = (prod(P[k])^(1/K)) / (sum(P[k]) / K)
```

Computed in log domain to avoid numerical overflow/underflow:

```
log_flatness = mean(ln(P[k])) - ln(mean(P[k]))
flatness = exp(log_flatness)
```

**Typical range**: 0.0 (pure tone) to 1.0 (white noise). Music typically 0.01--0.4.

**Visual mapping**: Texture detail (smooth vs noisy surfaces), particle jitter amplitude, distortion/glitch intensity. Low flatness = clean geometry; high flatness = fragmented/chaotic visuals.

```cpp
float spectral_flatness(const float* mag, int num_bins) {
    double log_sum = 0.0, arith_sum = 0.0;
    int count = 0;
    for (int k = 1; k < num_bins; ++k) {
        float p = mag[k] * mag[k]; // power
        if (p > 1e-20f) {
            log_sum += std::log(p);
            arith_sum += p;
            ++count;
        }
    }
    if (count == 0 || arith_sum < 1e-20) return 0.0f;
    double log_geo_mean = log_sum / count;
    double arith_mean = arith_sum / count;
    return float(std::exp(log_geo_mean - std::log(arith_mean)));
}
```

### 2.5 Spectral Contrast

**What it measures**: The difference (in dB) between spectral peaks and valleys in each sub-band. High contrast = clear harmonic structure; low contrast = noise-like. Defined per-band, typically using 6--7 octave-spaced sub-bands.

**Formula** (per sub-band `b`):

1. Sort magnitudes within band b in descending order.
2. `peak_b = mean of top alpha fraction` (e.g., alpha = 0.2)
3. `valley_b = mean of bottom alpha fraction`
4. `contrast_b = log(peak_b) - log(valley_b)` (or dB difference)

**Typical range**: 0--60 dB per band. Highly variable with content.

**Visual mapping**: Per-band ring brightness, bar graph intensity, layer opacity.

```cpp
struct SpectralContrast {
    static constexpr int NUM_BANDS = 7;
    float peaks[NUM_BANDS];
    float valleys[NUM_BANDS];
    float contrast[NUM_BANDS]; // in dB

    void compute(const float* mag, int num_bins, float fs, int N_fft,
                 float alpha = 0.2f) {
        // Octave-spaced bands: 0-200, 200-400, 400-800, 800-1600,
        //                      1600-3200, 3200-6400, 6400-fs/2
        float edges[] = {0, 200, 400, 800, 1600, 3200, 6400, fs/2.0f};

        for (int b = 0; b < NUM_BANDS; ++b) {
            int k_lo = std::max(1, int(edges[b] * N_fft / fs));
            int k_hi = std::min(num_bins - 1, int(edges[b+1] * N_fft / fs));
            int band_size = k_hi - k_lo + 1;
            if (band_size <= 0) { peaks[b] = valleys[b] = contrast[b] = 0; continue; }

            // Collect and sort magnitudes in this band
            std::vector<float> band_mags(band_size);
            for (int k = k_lo; k <= k_hi; ++k)
                band_mags[k - k_lo] = mag[k];
            std::sort(band_mags.begin(), band_mags.end(), std::greater<float>());

            int n_alpha = std::max(1, int(alpha * band_size));
            float peak_sum = 0, valley_sum = 0;
            for (int i = 0; i < n_alpha; ++i) {
                peak_sum += band_mags[i];
                valley_sum += band_mags[band_size - 1 - i];
            }
            peaks[b] = peak_sum / n_alpha;
            valleys[b] = std::max(valley_sum / n_alpha, 1e-10f);
            contrast[b] = 20.0f * std::log10(peaks[b] / valleys[b]);
        }
    }
};
```

### 2.6 Spectral Bandwidth / Spread

**What it measures**: The "width" of the spectrum around the centroid -- the standard deviation of the frequency distribution weighted by magnitude. Narrow bandwidth = tonal; wide = broadband/noisy.

**Formula**:

```
spread = sqrt( sum(M[k] * (f_k - centroid)^2, k=1..K) / sum(M[k], k=1..K) )
```

**Typical range**: 500--5000 Hz for music. Drum hits have wide spread; solo flute has narrow spread.

**Visual mapping**: Object scale/size, blur kernel width, ring diameter, spread of particle emitter.

```cpp
float spectral_spread(const float* mag, int num_bins, float fs, int N_fft,
                      float centroid) {
    float weighted_var = 0.0f, total_mag = 0.0f;
    for (int k = 1; k < num_bins; ++k) {
        float freq = k * fs / N_fft;
        float diff = freq - centroid;
        weighted_var += mag[k] * diff * diff;
        total_mag += mag[k];
    }
    return (total_mag > 1e-10f) ? std::sqrt(weighted_var / total_mag) : 0.0f;
}
```

### 2.7 Spectral Skewness

**What it measures**: Asymmetry of the spectral distribution around the centroid. Positive skewness = more energy below the centroid (tail extends to high frequencies); negative = more energy above (tail extends low). Indicates timbral asymmetry.

**Formula** (third standardized moment):

```
skewness = sum(M[k] * (f_k - centroid)^3, k=1..K) / (S * spread^3)
```

**Typical range**: -3.0 to +3.0 for typical musical signals.

**Visual mapping**: Directional distortion (lean/tilt of geometry), asymmetric motion bias, wind direction in particle systems.

```cpp
float spectral_skewness(const float* mag, int num_bins, float fs, int N_fft,
                        float centroid, float spread) {
    if (spread < 1e-6f) return 0.0f;
    float m3 = 0.0f, total = 0.0f;
    for (int k = 1; k < num_bins; ++k) {
        float diff = (k * fs / N_fft) - centroid;
        m3 += mag[k] * diff * diff * diff;
        total += mag[k];
    }
    return (total > 1e-10f) ? (m3 / total) / (spread * spread * spread) : 0.0f;
}
```

### 2.8 Spectral Kurtosis

**What it measures**: "Peakedness" of the spectral distribution. High kurtosis = energy concentrated in a few bins (sharp peaks); low kurtosis = flat spectrum (diffuse). A Gaussian distribution has kurtosis = 3 (excess kurtosis = 0).

**Formula** (fourth standardized moment):

```
kurtosis = sum(M[k] * (f_k - centroid)^4, k=1..K) / (S * spread^4)
```

**Typical range**: 1.5 to 20+ for music. Pure tones > 10; noise ~ 3.

**Visual mapping**: Sharpness of visual edges, point vs area light source, focus/defocus amount.

```cpp
float spectral_kurtosis(const float* mag, int num_bins, float fs, int N_fft,
                        float centroid, float spread) {
    if (spread < 1e-6f) return 0.0f;
    float m4 = 0.0f, total = 0.0f;
    float spread4 = spread * spread * spread * spread;
    for (int k = 1; k < num_bins; ++k) {
        float diff = (k * fs / N_fft) - centroid;
        float diff2 = diff * diff;
        m4 += mag[k] * diff2 * diff2;
        total += mag[k];
    }
    return (total > 1e-10f) ? (m4 / total) / spread4 : 0.0f;
}
```

### 2.9 Spectral Entropy

**What it measures**: Information content of the spectrum (Shannon entropy). A single pure tone has low entropy (energy in one bin = predictable); white noise has maximum entropy (uniform distribution = unpredictable).

**Formula**:

```
p[k] = M[k] / sum(M[k])    (normalize to probability distribution)
entropy = -sum(p[k] * log2(p[k]), k=1..K)
```

Normalize to [0,1] by dividing by `log2(K)` (maximum possible entropy for K bins).

**Typical range**: 0.0 (pure tone) to 1.0 (white noise) when normalized.

**Visual mapping**: Chaos/randomness parameter, number of active elements, complexity of generated geometry, color palette width (monochrome vs rainbow).

```cpp
float spectral_entropy(const float* mag, int num_bins) {
    float total = 0.0f;
    for (int k = 1; k < num_bins; ++k) total += mag[k];
    if (total < 1e-10f) return 0.0f;

    float entropy = 0.0f;
    for (int k = 1; k < num_bins; ++k) {
        float p = mag[k] / total;
        if (p > 1e-10f)
            entropy -= p * std::log2(p);
    }
    // Normalize to [0, 1]
    float max_entropy = std::log2(float(num_bins - 1));
    return entropy / max_entropy;
}
```

### 2.10 Spectral Irregularity

**What it measures**: How much the magnitude of each bin deviates from its neighbors. High irregularity = jagged spectrum (many independent partials); low = smooth spectral envelope (noise, or closely-spaced harmonics).

Two common definitions:

**Jensen (sum of squared differences between adjacent bins)**:

```
irregularity_jensen = sum((M[k] - M[k+1])^2, k=1..K-1) / sum(M[k]^2, k=1..K)
```

**Krimphoff (deviation from local 3-bin average)**:

```
irregularity_krimphoff = sum(|M[k] - (M[k-1] + M[k] + M[k+1])/3|, k=2..K-1) / sum(M[k], k=1..K)
```

**Typical range**: 0 to 1 (normalized). Tonal instruments ~0.1--0.3; percussive/noisy ~0.5--0.9.

**Visual mapping**: Surface roughness/displacement, wireframe vs solid rendering mix, grid distortion amount.

```cpp
float spectral_irregularity_jensen(const float* mag, int num_bins) {
    float diff_sum = 0.0f, energy = 0.0f;
    for (int k = 1; k < num_bins - 1; ++k) {
        float d = mag[k] - mag[k + 1];
        diff_sum += d * d;
        energy += mag[k] * mag[k];
    }
    energy += mag[num_bins - 1] * mag[num_bins - 1];
    return (energy > 1e-10f) ? diff_sum / energy : 0.0f;
}

float spectral_irregularity_krimphoff(const float* mag, int num_bins) {
    float dev_sum = 0.0f, total = 0.0f;
    for (int k = 2; k < num_bins - 1; ++k) {
        float local_avg = (mag[k - 1] + mag[k] + mag[k + 1]) / 3.0f;
        dev_sum += std::abs(mag[k] - local_avg);
    }
    for (int k = 1; k < num_bins; ++k) total += mag[k];
    return (total > 1e-10f) ? dev_sum / total : 0.0f;
}
```

### 2.11 Spectral Decrease

**What it measures**: Rate of decrease of the spectral amplitude. Emphasizes low-frequency content's dominance over high-frequency. Related to spectral tilt but weighted to prioritize lower bins.

**Formula**:

```
decrease = sum((M[k] - M[1]) / (k - 1), k=2..K) / sum(M[k], k=2..K)
```

**Typical range**: -0.5 to 0.5 (normalized). Negative values indicate increasing spectrum (unusual in natural audio).

**Visual mapping**: Gravitational pull direction, decay rate of particle trails, opacity falloff gradient.

```cpp
float spectral_decrease(const float* mag, int num_bins) {
    float num = 0.0f, denom = 0.0f;
    float m1 = mag[1];
    for (int k = 2; k < num_bins; ++k) {
        num += (mag[k] - m1) / float(k - 1);
        denom += mag[k];
    }
    return (denom > 1e-10f) ? num / denom : 0.0f;
}
```

### 2.12 Spectral Slope

**What it measures**: Linear regression slope of the magnitude spectrum (magnitude vs frequency). Indicates overall spectral tilt. Most natural sounds have negative slope (energy decreases with frequency). Brighter timbres have less steep (closer to zero) slope.

**Formula** (linear least squares):

```
slope = (K * sum(f_k * M[k]) - sum(f_k) * sum(M[k])) /
        (K * sum(f_k^2) - (sum(f_k))^2)
```

**Typical range**: -0.01 to 0 for music (in magnitude per Hz). Scale by fs for a dimensionless value.

**Visual mapping**: Tilt/rotation of visual elements, gradient angle, perspective distortion strength.

```cpp
float spectral_slope(const float* mag, int num_bins, float fs, int N_fft) {
    float sum_f = 0, sum_m = 0, sum_fm = 0, sum_f2 = 0;
    int K = num_bins - 1; // bins 1..num_bins-1
    for (int k = 1; k < num_bins; ++k) {
        float f = k * fs / N_fft;
        sum_f += f;
        sum_m += mag[k];
        sum_fm += f * mag[k];
        sum_f2 += f * f;
    }
    float denom = K * sum_f2 - sum_f * sum_f;
    return (std::abs(denom) > 1e-10f) ? (K * sum_fm - sum_f * sum_m) / denom : 0.0f;
}
```

### 2.13 Odd/Even Harmonic Ratio (OER)

**What it measures**: The ratio of energy in odd harmonics (f0, 3*f0, 5*f0, ...) to even harmonics (2*f0, 4*f0, 6*f0, ...). Clarinet-like timbres have strong odd harmonics (OER >> 1); sawtooth waves have equal odd and even (OER ~ 1). Square waves are pure-odd (OER -> infinity).

**Prerequisite**: Requires fundamental frequency (f0) estimation. Use autocorrelation, YIN, or a pitch tracker. See [FEATURES_frequency_bands.md](FEATURES_frequency_bands.md).

**Formula**:

```
odd_energy  = sum(M[bin(n*f0)]^2, n=1,3,5,7,...)
even_energy = sum(M[bin(n*f0)]^2, n=2,4,6,8,...)
OER = odd_energy / even_energy
```

**Typical range**: 0.5 to 20. Values near 1 = balanced harmonics; >> 1 = hollow/clarinet-like.

**Visual mapping**: Shape morphing (circle/square vs triangle/pentagon), symmetry parameter, hollow vs filled rendering.

```cpp
float odd_even_ratio(const float* mag, int num_bins, float f0, float fs, int N_fft,
                     int max_harmonics = 16) {
    float odd_energy = 0.0f, even_energy = 0.0f;
    for (int n = 1; n <= max_harmonics; ++n) {
        int k = std::lround(n * f0 * N_fft / fs);
        if (k >= num_bins) break;
        float e = mag[k] * mag[k];
        if (n % 2 == 1) odd_energy += e;
        else            even_energy += e;
    }
    return (even_energy > 1e-10f) ? odd_energy / even_energy : 100.0f;
}
```

### 2.14 Tristimulus

**What it measures**: Three-band decomposition of harmonic energy, analogous to the color tristimulus (XYZ) in colorimetry. Proposed by Pollard & Jansson (1982). Gives a compact 3D timbre coordinate.

**Formula** (requires f0 and harmonic magnitudes `a_n = M[bin(n*f0)]`):

```
T1 = a_1 / sum(a_n)                           (fundamental ratio)
T2 = (a_2 + a_3 + a_4) / sum(a_n)             (mid harmonics)
T3 = sum(a_n, n=5..N_h) / sum(a_n)            (upper harmonics)
T1 + T2 + T3 = 1.0
```

**Typical range**: Each component 0.0--1.0, summing to 1.0. Flute: T1~0.8, T2~0.15, T3~0.05. Oboe: T1~0.3, T2~0.35, T3~0.35.

**Visual mapping**: Direct RGB color mapping (T1=R, T2=G, T3=B), barycentric coordinate on a triangle, three-layer blending weights.

```cpp
struct Tristimulus {
    float t1, t2, t3;

    void compute(const float* mag, int num_bins, float f0, float fs, int N_fft,
                 int max_harmonics = 12) {
        float total = 0.0f, mid = 0.0f, upper = 0.0f;
        float a1 = 0.0f;

        for (int n = 1; n <= max_harmonics; ++n) {
            int k = std::lround(n * f0 * N_fft / fs);
            if (k >= num_bins) break;
            float a = mag[k];
            total += a;
            if (n == 1) a1 = a;
            else if (n >= 2 && n <= 4) mid += a;
            else upper += a;
        }
        if (total < 1e-10f) { t1 = t2 = t3 = 0; return; }
        t1 = a1 / total;
        t2 = mid / total;
        t3 = upper / total;
    }
};
```

### Feature Summary Table

| Feature | Formula Complexity | CPU Cost | Requires f0? | Typical Visual Use |
|---|---|---|---|---|
| Centroid | O(K) | Very low | No | Color temperature, brightness |
| Flux | O(K) | Very low | No | Onset flash, pulse intensity |
| Rolloff | O(K) | Low | No | Zoom, aperture, trail length |
| Flatness | O(K) | Low | No | Noise/glitch amount, texture |
| Contrast | O(K log K) per band | Medium | No | Per-band intensity, layers |
| Spread | O(K) | Very low | No | Object size, blur radius |
| Skewness | O(K) | Very low | No | Tilt, directional bias |
| Kurtosis | O(K) | Very low | No | Edge sharpness, focus |
| Entropy | O(K) | Low | No | Chaos, complexity |
| Irregularity | O(K) | Very low | No | Surface roughness |
| Decrease | O(K) | Very low | No | Decay rate, gravity |
| Slope | O(K) | Very low | No | Rotation, perspective |
| OER | O(harmonics) | Very low | **Yes** | Shape symmetry, hollowness |
| Tristimulus | O(harmonics) | Very low | **Yes** | RGB color, layer weights |

---

## 3. FFTW Usage Patterns

FFTW ("Fastest Fourier Transform in the West") is the gold-standard FFT library. Key concepts for real-time use:

### 3.1 Planning

FFTW separates plan creation (expensive, one-time) from execution (fast, per-frame). Plan creation measures the machine's hardware to find the optimal algorithm.

**Planning flags**:

| Flag | Speed | Planning Time | Use |
|---|---|---|---|
| `FFTW_ESTIMATE` | Good | Instant | Development/prototyping |
| `FFTW_MEASURE` | Better | ~1 second | Production startup |
| `FFTW_PATIENT` | Best | ~10 seconds | Offline optimization |
| `FFTW_EXHAUSTIVE` | Marginal gain | Minutes | Benchmarking only |

### 3.2 Wisdom

FFTW can export its planning results ("wisdom") to a file and reimport them later, avoiding re-measurement at startup.

```cpp
// Save after planning
fftwf_export_wisdom_to_filename("fftw_wisdom.dat");

// Load at startup
fftwf_import_wisdom_from_filename("fftw_wisdom.dat");
```

Generate wisdom once per target machine. Ship with your application or generate on first run.

### 3.3 Real-to-Complex Transform

For real-valued audio signals, use `fftwf_plan_dft_r2c_1d`. It computes only the non-redundant half of the spectrum (`N/2 + 1` complex bins), halving computation and memory.

### 3.4 In-Place vs Out-of-Place

- **In-place**: Input array is overwritten with output. Saves memory but requires careful alignment (real array must be padded to `2*(N/2+1)` floats for in-place r2c).
- **Out-of-place**: Separate input/output arrays. Simpler, slightly faster on some architectures. Preferred for clarity.

### 3.5 Thread Safety

- Plan creation/destruction is **NOT** thread-safe. Protect with a mutex or do all planning on a single thread at startup.
- Plan execution **IS** thread-safe for different plans or different I/O arrays.
- The global wisdom state is shared. Serialize all `import/export_wisdom` calls.

### 3.6 Complete C++ Wrapper

```cpp
#include <fftw3.h>
#include <vector>
#include <cmath>
#include <mutex>

class FFTProcessor {
public:
    FFTProcessor(int fft_size, int hop_size)
        : N_(fft_size)
        , hop_(hop_size)
        , num_bins_(fft_size / 2 + 1)
        , accumulator_(fft_size, hop_size)
    {
        // Allocate FFTW-aligned memory
        in_  = fftwf_alloc_real(N_);
        out_ = fftwf_alloc_complex(num_bins_);
        window_.resize(N_);
        magnitude_.resize(num_bins_, 0.0f);
        prev_magnitude_.resize(num_bins_, 0.0f);

        // Precompute Hann window
        hann_window(window_.data(), N_);

        // Create plan (thread-safe guard)
        {
            static std::mutex plan_mutex;
            std::lock_guard<std::mutex> lock(plan_mutex);
            plan_ = fftwf_plan_dft_r2c_1d(N_, in_, out_, FFTW_MEASURE);
        }
    }

    ~FFTProcessor() {
        static std::mutex plan_mutex;
        std::lock_guard<std::mutex> lock(plan_mutex);
        fftwf_destroy_plan(plan_);
        fftwf_free(in_);
        fftwf_free(out_);
    }

    // Non-copyable
    FFTProcessor(const FFTProcessor&) = delete;
    FFTProcessor& operator=(const FFTProcessor&) = delete;

    // Push audio samples. Returns true when a new frame is analyzed.
    bool process(const float* samples, int count) {
        if (!accumulator_.push(samples, count))
            return false;

        // Get frame and apply window
        accumulator_.get_frame(in_);
        for (int n = 0; n < N_; ++n)
            in_[n] *= window_[n];

        // Execute FFT
        fftwf_execute(plan_);

        // Store previous magnitudes for flux
        std::swap(magnitude_, prev_magnitude_);

        // Compute magnitude spectrum
        for (int k = 0; k < num_bins_; ++k) {
            float re = out_[k][0];
            float im = out_[k][1];
            magnitude_[k] = std::sqrt(re * re + im * im);
        }
        return true;
    }

    const float* magnitude() const { return magnitude_.data(); }
    const float* prev_magnitude() const { return prev_magnitude_.data(); }
    int num_bins() const { return num_bins_; }
    int fft_size() const { return N_; }

private:
    int N_, hop_, num_bins_;
    fftwf_plan plan_;
    float* in_;
    fftwf_complex* out_;
    std::vector<float> window_;
    std::vector<float> magnitude_;
    std::vector<float> prev_magnitude_;
    FrameAccumulator accumulator_;
};
```

---

## 4. Magnitude Spectrum Processing

### 4.1 Power Spectrum

```
P[k] = |X[k]|^2 = Re(X[k])^2 + Im(X[k])^2
```

Use power spectrum (not magnitude) for: spectral flatness, energy-based features, MFCCs. Avoids a square root per bin.

```cpp
void power_spectrum(const fftwf_complex* fft_out, float* power, int num_bins) {
    for (int k = 0; k < num_bins; ++k)
        power[k] = fft_out[k][0] * fft_out[k][0] + fft_out[k][1] * fft_out[k][1];
}
```

### 4.2 dB Conversion

```
dB[k] = 20 * log10(M[k] / M_ref)
```

For visualization, `M_ref` is typically 1.0 (0 dBFS) or the peak magnitude. Clamp the lower bound to avoid -infinity.

```cpp
void magnitude_to_dB(const float* mag, float* dB, int num_bins,
                     float ref = 1.0f, float floor_dB = -120.0f) {
    float inv_ref = 1.0f / std::max(ref, 1e-20f);
    for (int k = 0; k < num_bins; ++k) {
        float val = 20.0f * std::log10(std::max(mag[k] * inv_ref, 1e-20f));
        dB[k] = std::max(val, floor_dB);
    }
}
```

### 4.3 Log-Magnitude

For feature extraction, `log(M[k])` is often more perceptually meaningful than linear magnitude, compressing dynamic range similarly to human hearing.

```cpp
void log_magnitude(const float* mag, float* log_mag, int num_bins, float floor = 1e-6f) {
    for (int k = 0; k < num_bins; ++k)
        log_mag[k] = std::log(std::max(mag[k], floor));
}
```

### 4.4 Spectral Smoothing

Raw magnitude spectra are noisy frame-to-frame. Two smoothing strategies:

**Frequency-domain smoothing** (moving average across bins -- smooths spectral shape):

```cpp
void smooth_spectrum(const float* in, float* out, int num_bins, int half_width = 3) {
    for (int k = 0; k < num_bins; ++k) {
        float sum = 0.0f;
        int count = 0;
        for (int j = std::max(0, k - half_width);
             j <= std::min(num_bins - 1, k + half_width); ++j) {
            sum += in[j];
            ++count;
        }
        out[k] = sum / count;
    }
}
```

**Temporal smoothing** (exponential moving average across frames -- smooths feature evolution):

```cpp
void ema_smooth(float* state, float new_value, float alpha) {
    // alpha in (0, 1]: higher = less smoothing, more responsive
    *state = alpha * new_value + (1.0f - alpha) * (*state);
}
```

Typical alpha values for visualization:

| alpha | Behavior | Use |
|---|---|---|
| 0.05--0.1 | Very smooth, slow | Ambient parameters (color drift, fog) |
| 0.2--0.4 | Moderate | Geometry deformation, camera movement |
| 0.5--0.7 | Responsive | Brightness, scale, opacity |
| 0.8--1.0 | Nearly raw | Beat triggers, flash effects |

---

## 5. Update Rates: Bridging Audio Rate and Video Frame Rate

The FFT runs at audio rate (hop-driven, typically 23--93 frames/sec at 48 kHz). The rendering loop runs at 30--60 fps. These rates rarely align. Three strategies:

### 5.1 Decimation (FFT Rate > Video Rate)

When spectral frames arrive faster than video frames consume them, take the most recent frame or average the last N frames.

```cpp
class FeatureToVideoBridge {
    std::atomic<bool> new_data_{false};

    // Double-buffer: audio thread writes to back, video thread reads from front
    struct FeatureSet {
        float centroid, flux, rolloff, flatness, spread, entropy;
        float skewness, kurtosis, slope, decrease;
        float irregularity_j, irregularity_k;
        float contrast[7];
    };

    FeatureSet buffers_[2];
    std::atomic<int> write_idx_{0};

public:
    // Called from audio thread after FFT
    void publish(const FeatureSet& features) {
        int idx = write_idx_.load(std::memory_order_relaxed);
        buffers_[idx] = features;
        write_idx_.store(1 - idx, std::memory_order_release);
        new_data_.store(true, std::memory_order_release);
    }

    // Called from render thread each frame
    FeatureSet consume() {
        new_data_.store(false, std::memory_order_relaxed);
        int idx = 1 - write_idx_.load(std::memory_order_acquire);
        return buffers_[idx];
    }

    bool has_new_data() const {
        return new_data_.load(std::memory_order_acquire);
    }
};
```

### 5.2 Interpolation (FFT Rate < Video Rate)

When rendering faster than FFT frames arrive (e.g., FFT at 23 fps, video at 60 fps), interpolate features between the two most recent FFT frames.

```cpp
struct InterpolatedFeatures {
    FeatureSet prev, current;
    double fft_timestamp_prev, fft_timestamp_curr;

    FeatureSet interpolate(double render_time) const {
        if (fft_timestamp_curr <= fft_timestamp_prev) return current;
        double t = (render_time - fft_timestamp_prev)
                 / (fft_timestamp_curr - fft_timestamp_prev);
        t = std::clamp(t, 0.0, 1.0);
        FeatureSet result;
        // Linear interpolation per-field
        result.centroid = prev.centroid + t * (current.centroid - prev.centroid);
        result.flux     = prev.flux     + t * (current.flux     - prev.flux);
        // ... repeat for all scalar features
        return result;
    }
};
```

### 5.3 Hybrid: Audio-Thread Feature Extraction + Lock-Free Queue

The most robust architecture:

1. **Audio callback** pushes raw samples into a lock-free ring buffer.
2. **Analysis thread** (dedicated, pinned to a core) pulls samples, runs FFT, computes all features, publishes to a lock-free SPSC (single-producer single-consumer) queue.
3. **Render thread** drains the queue each frame, applies temporal smoothing, drives visuals.

This decouples all three timing domains. The analysis thread runs at its natural rate (hop-driven). The render thread never blocks on the audio thread. Lock-free SPSC queues (e.g., `boost::lockfree::spsc_queue` or a simple power-of-two ring buffer with atomic indices) guarantee zero contention.

```cpp
// Minimal lock-free SPSC ring buffer for feature snapshots
template <typename T, size_t Capacity>
class SPSCQueue {
    static_assert((Capacity & (Capacity - 1)) == 0, "Capacity must be power of 2");
    std::array<T, Capacity> buf_;
    alignas(64) std::atomic<size_t> head_{0};  // written by producer
    alignas(64) std::atomic<size_t> tail_{0};  // written by consumer

public:
    bool try_push(const T& item) {
        size_t h = head_.load(std::memory_order_relaxed);
        size_t next = (h + 1) & (Capacity - 1);
        if (next == tail_.load(std::memory_order_acquire)) return false; // full
        buf_[h] = item;
        head_.store(next, std::memory_order_release);
        return true;
    }

    bool try_pop(T& item) {
        size_t t = tail_.load(std::memory_order_relaxed);
        if (t == head_.load(std::memory_order_acquire)) return false; // empty
        item = buf_[t];
        tail_.store((t + 1) & (Capacity - 1), std::memory_order_release);
        return true;
    }
};
```

### 5.4 Timing Budget

At 48 kHz, fs=48000, N=4096, hop=1024 (75% overlap): each FFT frame spans 21.3 ms of audio, and frames arrive every 21.3 ms. The feature extraction budget per frame:

| Operation | Typical Time (N=4096, x86-64) |
|---|---|
| Hann window application | ~2 us |
| FFTW r2c execute | ~15--30 us |
| Magnitude computation | ~3 us |
| All 14 spectral features | ~10--20 us |
| **Total** | **~30--55 us** |

This is well under 1 ms -- roughly 0.25% of the 21.3 ms budget. There is no performance concern with computing all features every frame on any modern CPU. The bottleneck in a VJ engine is always GPU rendering, not audio analysis.

---

## Appendix A: Complete Feature Extraction Pipeline

Bringing everything together into a single callable class:

```cpp
struct SpectralFeatures {
    float centroid;
    float flux;
    float rolloff_85;
    float rolloff_95;
    float flatness;
    float spread;
    float skewness;
    float kurtosis;
    float entropy;
    float irregularity_jensen;
    float irregularity_krimphoff;
    float decrease;
    float slope;
    float odd_even_ratio;
    Tristimulus tristimulus;
    SpectralContrast contrast;

    void compute(const float* mag, const float* mag_prev, int num_bins,
                 float fs, int N_fft, float f0 = 0.0f) {
        centroid    = spectral_centroid(mag, num_bins, fs, N_fft);
        flux        = spectral_flux(mag, mag_prev, num_bins);
        rolloff_85  = spectral_rolloff(mag, num_bins, fs, N_fft, 0.85f);
        rolloff_95  = spectral_rolloff(mag, num_bins, fs, N_fft, 0.95f);
        flatness    = spectral_flatness(mag, num_bins);
        spread      = spectral_spread(mag, num_bins, fs, N_fft, centroid);
        skewness    = spectral_skewness(mag, num_bins, fs, N_fft, centroid, spread);
        kurtosis    = spectral_kurtosis(mag, num_bins, fs, N_fft, centroid, spread);
        entropy     = spectral_entropy(mag, num_bins);
        irregularity_jensen    = spectral_irregularity_jensen(mag, num_bins);
        irregularity_krimphoff = spectral_irregularity_krimphoff(mag, num_bins);
        decrease    = spectral_decrease(mag, num_bins);
        slope       = spectral_slope(mag, num_bins, fs, N_fft);
        contrast.compute(mag, num_bins, fs, N_fft);

        if (f0 > 20.0f) { // Only if pitch is detected
            odd_even_ratio = odd_even_ratio_fn(mag, num_bins, f0, fs, N_fft);
            tristimulus.compute(mag, num_bins, f0, fs, N_fft);
        } else {
            odd_even_ratio = 1.0f;
            tristimulus = {0.33f, 0.33f, 0.33f};
        }
    }
};
```

---

## Appendix B: Visual Mapping Cheat Sheet

| Feature | Low Value Visual | High Value Visual |
|---|---|---|
| Centroid | Warm colors, large shapes | Cool colors, small/sharp shapes |
| Flux | Static, calm | Flash, burst, particles |
| Rolloff | Narrow beam, focused | Wide spread, diffuse |
| Flatness | Clean geometry, smooth | Noise, glitch, fragmentation |
| Contrast | Uniform layers | Distinct peaks, defined bands |
| Spread | Tight, focused | Expanded, wide |
| Skewness | Left-leaning motion | Right-leaning motion |
| Kurtosis | Soft edges, blurred | Hard edges, crystalline |
| Entropy | Ordered, repetitive | Chaotic, random, complex |
| Irregularity | Smooth surfaces | Rough, displaced surfaces |
| Decrease | Sustained, floating | Rapid decay, falling |
| Slope | Tilted perspective | Flat/neutral perspective |
| OER | Filled shapes, symmetric | Hollow shapes, odd geometry |
| Tristimulus | (direct RGB) | (direct RGB) |

---

## References

- Peeters, G. (2004). "A large set of audio features for sound description." IRCAM Technical Report.
- McAdams, S. et al. (1995). "Perceptual scaling of synthesized musical timbres."
- Pollard, H.F. & Jansson, E.V. (1982). "A tristimulus method for the specification of musical timbre."
- FFTW manual: https://www.fftw.org/fftw3_doc/
- Lerch, A. (2012). "An Introduction to Audio Content Analysis." Wiley/IEEE.
- Jensen, K. (1999). "Timbre models of musical sounds." PhD Thesis, University of Copenhagen.
