# MFCCs, Mel Spectrograms, Chroma Features, and Tonnetz

## Purpose and Scope

This document covers the four primary perceptual/tonal feature families used in real-time music analysis and visualization: Mel-Frequency Cepstral Coefficients (MFCCs), Mel spectrograms, Chroma features (chromagrams), and Tonnetz (tonal centroid) representations. Each section provides the mathematical foundation, implementation details, and real-time streaming considerations.

**Cross-references**: [FEATURES_spectral.md](FEATURES_spectral.md) (spectral centroid, rolloff, flux), [FEATURES_frequency_bands.md](FEATURES_frequency_bands.md) (octave/bark bands), [FEATURES_pitch_harmonic.md](FEATURES_pitch_harmonic.md) (f0 estimation, harmonic tracking), [LIB_essentia.md](LIB_essentia.md) (Essentia library integration), [REF_math_reference.md](REF_math_reference.md) (DCT, FFT, filterbank math).

---

## 1. MFCC Pipeline: Step-by-Step

MFCCs are the dominant feature representation for timbre characterization. The pipeline transforms raw audio into a compact cepstral representation that approximates human auditory perception. Each step has a specific psychoacoustic or mathematical motivation.

### 1.1 Pre-Emphasis Filter

**Purpose**: Boost high-frequency energy to compensate for the spectral tilt introduced by the glottal source (in speech) and the general -6 dB/octave roll-off of natural sounds. Without pre-emphasis, the Mel filterbank's upper filters see disproportionately low energy, reducing the signal-to-noise ratio of high-frequency cepstral information.

**Formula (first-order FIR high-pass)**:

```
y[n] = x[n] - α * x[n-1]
```

where `α` is typically 0.95--0.97. The transfer function is:

```
H(z) = 1 - α * z^(-1)
```

This produces approximately +20 dB/decade boost above ~1 kHz. The exact corner frequency is:

```
f_corner = (α * fs) / (2π)    (for α close to 1)
```

At `fs = 44100` and `α = 0.97`, the corner is ~6.8 kHz, meaning the boost is gradual across the spectrum rather than a sharp transition. For music (as opposed to speech), some implementations reduce `α` to 0.93 or skip pre-emphasis entirely when the downstream model is robust to spectral tilt.

**Implementation note**: Pre-emphasis requires exactly one sample of state (`x[n-1]`) across frame boundaries in streaming mode.

### 1.2 Framing and Windowing

The signal is segmented into overlapping frames to approximate short-time stationarity.

**Typical parameters for real-time music analysis**:

| Parameter | Value | Rationale |
|-----------|-------|-----------|
| Frame length | 1024 samples (~23.2 ms at 44.1 kHz) | Long enough for bass resolution (~43 Hz bin spacing), short enough for temporal detail |
| Hop size | 512 samples (~11.6 ms) | 50% overlap balances temporal resolution and compute cost |
| Window | Hann (Hanning) | Side-lobe suppression without excessive main-lobe widening |

The Hann window is defined as:

```
w[n] = 0.5 * (1 - cos(2π * n / (N - 1)))    for n = 0, 1, ..., N-1
```

The windowed frame is:

```
x_w[n] = x[n + m*H] * w[n]
```

where `m` is the frame index and `H` is the hop size.

**Why Hann over Hamming**: Hann has zeros at the window edges, producing cleaner spectral leakage characteristics. Hamming has slightly better side-lobe roll-off but non-zero endpoints. For MFCC extraction where the spectrum feeds into broad Mel filters, the difference is negligible -- either works. Avoid rectangular windows (severe leakage) and Blackman-Harris (main-lobe too wide, smears adjacent Mel bins).

### 1.3 FFT and Power Spectrum

Apply an N-point FFT (typically N = frame length, zero-padded to next power of 2 if necessary):

```
X[k] = Σ_{n=0}^{N-1} x_w[n] * e^{-j2πkn/N}    for k = 0, 1, ..., N/2
```

We keep only the first `N/2 + 1` bins (the positive-frequency half) since the input is real-valued.

The **power spectrum** (periodogram estimate):

```
P[k] = |X[k]|^2 / N
```

Some implementations use the **magnitude spectrum** `|X[k]|` instead. The choice matters less than it seems because the subsequent log compression reduces the dynamic range regardless. Using `|X[k]|^2` is more standard in the MFCC literature (Davis & Mermelstein, 1980).

### 1.4 Mel Filterbank Application

The Mel scale approximates the human ear's frequency resolution -- logarithmic above ~1 kHz, approximately linear below.

**Mel-frequency conversion (O'Shaughnessy variant)**:

```
mel(f) = 2595 * log10(1 + f / 700)
f(mel) = 700 * (10^(mel / 2595) - 1)
```

An alternative (HTK-style):

```
mel(f) = 1127 * ln(1 + f / 700)
f(mel) = 700 * (e^(mel / 1127) - 1)
```

These are equivalent up to a constant factor (2595 * log10 = 1127 * ln).

**Triangular filter construction** for `M` filters over the range `[f_low, f_high]`:

1. Convert `f_low` and `f_high` to Mel scale
2. Create `M + 2` equally spaced points on the Mel scale
3. Convert these points back to Hz
4. Map Hz center frequencies to FFT bin indices
5. Construct triangular filters where filter `m` has:
   - Rising slope from point `m` to point `m+1`
   - Falling slope from point `m+1` to point `m+2`

```
         /\
        /  \
       /    \
      /      \
-----/--------\-----
   f[m]  f[m+1]  f[m+2]

H_m[k] = 0                                        if f[k] < f[m]
        (f[k] - f[m]) / (f[m+1] - f[m])           if f[m] <= f[k] < f[m+1]
        (f[m+2] - f[k]) / (f[m+2] - f[m+1])       if f[m+1] <= f[k] < f[m+2]
        0                                          if f[k] >= f[m+2]
```

**Number of filters**: 26 is the classic minimum (speech recognition). 40 is common for music. 128 is used for full Mel spectrograms destined for CNNs. For MFCC extraction, 26--40 is the useful range; beyond 40, the additional filters add negligible cepstral information because the DCT discards the fine detail anyway.

**Filter normalization**: Filters should be area-normalized (divide each filter by its area) to ensure equal energy contribution. Without this, wider low-frequency filters dominate the representation.

The Mel-filtered energy for filter `m`:

```
S[m] = Σ_{k=0}^{N/2} P[k] * H_m[k]    for m = 0, 1, ..., M-1
```

### 1.5 Log Compression

```
log_S[m] = ln(S[m] + ε)
```

where `ε` is a small constant (e.g., `1e-10`) to avoid `ln(0)`.

**Why logarithmic**: Human loudness perception is approximately logarithmic (Weber-Fechner law). Log compression also makes the representation more robust to gain variations and converts the multiplicative effects of the vocal tract transfer function (in speech) or instrument body resonance (in music) into additive components, which the subsequent DCT can separate.

Some implementations use `log10` or `dB = 10 * log10(S[m])`. The choice affects only the scaling of the resulting MFCCs, not their discriminative power. Natural log is most common in the literature.

### 1.6 DCT (Type-II) to MFCCs

The Discrete Cosine Transform decorrelates the log Mel energies and compacts the information into the first few coefficients:

```
c[n] = Σ_{m=0}^{M-1} log_S[m] * cos(π * n * (m + 0.5) / M)    for n = 0, 1, ..., C-1
```

where `C` is the number of cepstral coefficients to retain (typically 13, 20, or 40).

**Why DCT and not inverse FFT**: The log Mel energies are real, non-negative, and roughly symmetric in their correlation structure. The DCT (a real-valued transform) is the optimal decorrelating transform for such data -- it approximates the Karhunen-Loeve Transform (KLT) for signals with near-Markov correlation. Using a full complex FFT would double the output dimensionality for no benefit.

**Why we keep only the first C coefficients**: The DCT concentrates energy into low-quefrency coefficients. High-quefrency coefficients encode rapid spectral fluctuations (fine harmonic structure), which are less relevant for timbre and more sensitive to pitch and noise.

### 1.7 Complete C++ Implementation

```cpp
#include <cmath>
#include <vector>
#include <algorithm>
#include <numeric>

// Forward declaration -- assumes an FFT implementation exists.
// See REF_math_reference.md for pffft / KissFFT integration.
void fft_real(const float* input, float* output_re, float* output_im, int N);

class MFCCExtractor {
public:
    struct Config {
        int sample_rate    = 44100;
        int frame_size     = 1024;
        int hop_size       = 512;
        int num_mel_filters = 40;
        int num_coeffs     = 13;
        float preemphasis  = 0.97f;
        float mel_low_hz   = 20.0f;
        float mel_high_hz  = 0.0f;  // 0 = Nyquist
        bool apply_lifter  = true;
        int lifter_param   = 22;    // L in sinusoidal liftering
    };

    explicit MFCCExtractor(const Config& cfg) : cfg_(cfg) {
        if (cfg_.mel_high_hz <= 0.0f)
            cfg_.mel_high_hz = cfg_.sample_rate / 2.0f;

        fft_size_ = cfg_.frame_size;  // Assume power of 2
        num_fft_bins_ = fft_size_ / 2 + 1;

        build_window();
        build_mel_filterbank();
        build_dct_matrix();
        if (cfg_.apply_lifter)
            build_lifter_coeffs();

        // Working buffers
        frame_buf_.resize(fft_size_, 0.0f);
        fft_re_.resize(fft_size_, 0.0f);
        fft_im_.resize(fft_size_, 0.0f);
        power_spectrum_.resize(num_fft_bins_, 0.0f);
        mel_energies_.resize(cfg_.num_mel_filters, 0.0f);
        log_mel_.resize(cfg_.num_mel_filters, 0.0f);
    }

    // Extract MFCCs from a single frame of audio (frame_size samples).
    // Caller is responsible for framing and overlap.
    void extract_frame(const float* audio_frame, float* mfcc_out) {
        // --- Pre-emphasis ---
        frame_buf_[0] = audio_frame[0] - cfg_.preemphasis * prev_sample_;
        for (int n = 1; n < cfg_.frame_size; ++n)
            frame_buf_[n] = audio_frame[n] - cfg_.preemphasis * audio_frame[n - 1];
        prev_sample_ = audio_frame[cfg_.frame_size - 1];

        // --- Windowing ---
        for (int n = 0; n < cfg_.frame_size; ++n)
            frame_buf_[n] *= window_[n];

        // --- FFT ---
        fft_real(frame_buf_.data(), fft_re_.data(), fft_im_.data(), fft_size_);

        // --- Power spectrum ---
        float inv_n = 1.0f / static_cast<float>(fft_size_);
        for (int k = 0; k < num_fft_bins_; ++k)
            power_spectrum_[k] = (fft_re_[k] * fft_re_[k]
                                + fft_im_[k] * fft_im_[k]) * inv_n;

        // --- Mel filterbank ---
        for (int m = 0; m < cfg_.num_mel_filters; ++m) {
            float sum = 0.0f;
            for (int k = filter_start_[m]; k < filter_end_[m]; ++k)
                sum += power_spectrum_[k] * mel_weights_[m][k - filter_start_[m]];
            mel_energies_[m] = sum;
        }

        // --- Log compression ---
        constexpr float epsilon = 1e-10f;
        for (int m = 0; m < cfg_.num_mel_filters; ++m)
            log_mel_[m] = std::log(mel_energies_[m] + epsilon);

        // --- DCT ---
        for (int n = 0; n < cfg_.num_coeffs; ++n) {
            float sum = 0.0f;
            for (int m = 0; m < cfg_.num_mel_filters; ++m)
                sum += log_mel_[m] * dct_matrix_[n * cfg_.num_mel_filters + m];
            mfcc_out[n] = sum;
        }

        // --- Liftering (optional) ---
        if (cfg_.apply_lifter) {
            for (int n = 0; n < cfg_.num_coeffs; ++n)
                mfcc_out[n] *= lifter_coeffs_[n];
        }
    }

    // Reset streaming state (call when audio source changes)
    void reset() { prev_sample_ = 0.0f; }

    int num_coeffs() const { return cfg_.num_coeffs; }
    int num_mel_filters() const { return cfg_.num_mel_filters; }
    const std::vector<float>& get_log_mel() const { return log_mel_; }

private:
    static float hz_to_mel(float hz) {
        return 2595.0f * std::log10(1.0f + hz / 700.0f);
    }
    static float mel_to_hz(float mel) {
        return 700.0f * (std::pow(10.0f, mel / 2595.0f) - 1.0f);
    }

    void build_window() {
        window_.resize(cfg_.frame_size);
        for (int n = 0; n < cfg_.frame_size; ++n)
            window_[n] = 0.5f * (1.0f - std::cos(2.0f * M_PI * n
                         / (cfg_.frame_size - 1)));
    }

    void build_mel_filterbank() {
        float mel_low  = hz_to_mel(cfg_.mel_low_hz);
        float mel_high = hz_to_mel(cfg_.mel_high_hz);

        // M+2 equally spaced points on Mel scale
        int num_points = cfg_.num_mel_filters + 2;
        std::vector<float> mel_points(num_points);
        for (int i = 0; i < num_points; ++i)
            mel_points[i] = mel_low + i * (mel_high - mel_low)
                          / (num_points - 1);

        // Convert to Hz then to FFT bin indices
        std::vector<int> bin_indices(num_points);
        for (int i = 0; i < num_points; ++i) {
            float hz = mel_to_hz(mel_points[i]);
            bin_indices[i] = static_cast<int>(
                std::floor((fft_size_ + 1) * hz / cfg_.sample_rate));
        }

        // Build triangular filters (sparse storage)
        mel_weights_.resize(cfg_.num_mel_filters);
        filter_start_.resize(cfg_.num_mel_filters);
        filter_end_.resize(cfg_.num_mel_filters);

        for (int m = 0; m < cfg_.num_mel_filters; ++m) {
            int start = bin_indices[m];
            int center = bin_indices[m + 1];
            int end = bin_indices[m + 2];

            filter_start_[m] = start;
            filter_end_[m] = end + 1;

            int width = end - start + 1;
            mel_weights_[m].resize(width, 0.0f);

            // Rising slope
            for (int k = start; k <= center; ++k) {
                if (center != start)
                    mel_weights_[m][k - start] =
                        static_cast<float>(k - start)
                        / (center - start);
            }
            // Falling slope
            for (int k = center; k <= end; ++k) {
                if (end != center)
                    mel_weights_[m][k - start] =
                        static_cast<float>(end - k)
                        / (end - center);
            }

            // Area normalization
            float area = 0.0f;
            for (float w : mel_weights_[m]) area += w;
            if (area > 0.0f)
                for (float& w : mel_weights_[m]) w /= area;
        }
    }

    void build_dct_matrix() {
        // Type-II DCT
        dct_matrix_.resize(cfg_.num_coeffs * cfg_.num_mel_filters);
        for (int n = 0; n < cfg_.num_coeffs; ++n)
            for (int m = 0; m < cfg_.num_mel_filters; ++m)
                dct_matrix_[n * cfg_.num_mel_filters + m] =
                    std::cos(M_PI * n * (m + 0.5f) / cfg_.num_mel_filters);
    }

    void build_lifter_coeffs() {
        lifter_coeffs_.resize(cfg_.num_coeffs);
        int L = cfg_.lifter_param;
        for (int n = 0; n < cfg_.num_coeffs; ++n)
            lifter_coeffs_[n] = 1.0f + (L / 2.0f)
                              * std::sin(M_PI * n / L);
    }

    Config cfg_;
    int fft_size_;
    int num_fft_bins_;
    float prev_sample_ = 0.0f;

    std::vector<float> window_;
    std::vector<std::vector<float>> mel_weights_;
    std::vector<int> filter_start_;
    std::vector<int> filter_end_;
    std::vector<float> dct_matrix_;
    std::vector<float> lifter_coeffs_;

    // Working buffers (pre-allocated, no per-frame allocation)
    std::vector<float> frame_buf_;
    std::vector<float> fft_re_;
    std::vector<float> fft_im_;
    std::vector<float> power_spectrum_;
    std::vector<float> mel_energies_;
    std::vector<float> log_mel_;
};
```

---

## 2. MFCC Configuration

### 2.1 Number of Coefficients: 13 vs 20 vs 40

The number of retained DCT coefficients `C` controls the tradeoff between compactness and spectral resolution:

| Coefficients | Dimensionality | Captures | Use Case |
|---|---|---|---|
| **13** | 13 (39 with deltas) | Broad spectral envelope. Sufficient for speech recognition and basic timbre classification. | Real-time visualization, lightweight genre classification |
| **20** | 20 (60 with deltas) | Finer spectral detail. Better separation between similar instruments (e.g., trumpet vs. French horn). | Instrument recognition, music similarity |
| **40** | 40 (120 with deltas) | Near-complete reconstruction of the log-Mel spectrum. Diminishing returns beyond ~30 for most tasks. | Research, autoencoder input, tasks requiring spectral reconstruction |

**For real-time music visualization, 13 coefficients are sufficient.** The additional coefficients (14+) encode spectral micro-detail that is below the perceptual threshold for visualization mapping and adds compute cost without visual benefit.

### 2.2 Delta and Delta-Delta MFCCs

Static MFCCs represent a single snapshot. Temporal dynamics are captured by appending first-order (delta / velocity) and second-order (delta-delta / acceleration) differences:

**Delta computation** (regression over a window of `±N` frames, typically `N = 2`):

```
d[t] = (Σ_{n=1}^{N} n * (c[t+n] - c[t-n])) / (2 * Σ_{n=1}^{N} n^2)
```

For `N = 2`:

```
d[t] = (1*(c[t+1] - c[t-1]) + 2*(c[t+2] - c[t-2])) / (2*(1+4))
     = (c[t+1] - c[t-1] + 2*c[t+2] - 2*c[t-2]) / 10
```

Delta-deltas are computed identically but on the delta sequence rather than the static MFCCs.

**Real-time implication**: Delta computation introduces latency of `N` frames (e.g., 2 frames = ~23 ms at 512 hop). For visualization, this latency is acceptable. For instrument-onset detection, the delta-deltas provide valuable attack transient information.

**Feature vector construction**: `[c0..c12, d0..d12, dd0..dd12]` = 39-dimensional vector per frame.

### 2.3 Liftering (Cepstral Weighting)

Liftering applies a windowing function in the cepstral domain to reduce the variance of higher-order coefficients, which tend to have smaller magnitudes and higher relative variance:

**Sinusoidal liftering**:

```
w_lift[n] = 1 + (L/2) * sin(π * n / L)
```

where `L = 22` is the standard value. This boosts coefficients around index `L/2 ≈ 11` relative to `c0`, producing more uniform dynamic range across all coefficients. This is primarily useful when MFCCs are fed to distance-based classifiers (k-NN, GMM) where unequal variances distort the distance metric.

**For neural network inputs, liftering is unnecessary** -- the network learns its own normalization. For visualization mapping, liftering can help produce more visually balanced parameter ranges.

### 2.4 What Each Coefficient Captures

```
Quefrency →

c0   ████████████████████████████  Overall frame energy (log-energy)
c1   ██████████████████             Spectral balance: low vs. high frequency
c2   ██████████████                 Broad spectral shape (2 lobes)
c3   ████████████                   3-lobe spectral structure
c4   ██████████                     4-lobe structure
c5   ████████                       Formant-like detail
c6   ██████                         Fine spectral peaks
...
c12  ██                             Very fine spectral ripple
```

**Detailed breakdown**:

- **c0**: Log total energy of the frame. Directly proportional to loudness. Often excluded from distance calculations (replaced by a separate energy feature) because it dominates the variance.
- **c1--c3**: Broadband spectral tilt and gross shape. `c1` is strongly correlated with spectral centroid (see [FEATURES_spectral.md](FEATURES_spectral.md)). A positive `c1` indicates energy concentrated in low frequencies (bass-heavy); negative indicates treble-heavy. These three coefficients alone can often distinguish major instrument families (strings vs. brass vs. percussion).
- **c4--c5**: Mid-level spectral structure. Captures formant-like resonances in vocals and body resonances in acoustic instruments. Strong discriminators for voice vs. non-voice.
- **c6--c12**: Fine spectral detail. Sensitive to specific harmonic patterns and instrument-specific resonance characteristics. The most variable across time, most affected by noise, and least perceptually salient. Useful for distinguishing similar instruments within a family.

---

## 3. What MFCCs Capture vs. Don't

### 3.1 Strengths

MFCCs are excellent representations for:

- **Timbre / tonal color**: The Mel filterbank + log + DCT pipeline mimics the ear's spectral resolution, making MFCCs natural timbre descriptors. Two sounds with the same MFCCs will sound similar in tonal quality.
- **Instrument identification**: c1--c12 encode enough spectral envelope detail to distinguish instrument families and, with enough training data, individual instruments. A random forest on 13 MFCCs achieves ~70% accuracy on IRMAS instrument recognition; adding deltas pushes to ~80%.
- **Vocal presence detection**: The human vocal tract produces characteristic formant patterns that are well-captured by c3--c8. A simple threshold on the variance of c4--c6 can serve as a rough vocal activity detector.
- **Genre fingerprinting**: Mean and variance of MFCCs over 3--5 second windows produce genre-discriminative features. Electronic music shows low c1 variance (consistent spectral tilt); jazz shows high c6--c12 variance (diverse timbres).
- **Audio texture classification**: Noise-like vs. tonal, dense vs. sparse, are encoded in the distribution of MFCC values over time.

### 3.2 Weaknesses

MFCCs explicitly discard or poorly represent:

- **Pitch (fundamental frequency)**: The DCT truncation removes fine harmonic structure. A C4 piano note and a C5 piano note have similar MFCCs despite being an octave apart. Use pitch estimation instead (see [FEATURES_pitch_harmonic.md](FEATURES_pitch_harmonic.md)).
- **Rhythm and temporal structure**: MFCCs are per-frame features with no inherent temporal modeling. A 120 BPM and 140 BPM drum loop have identical MFCC distributions. Temporal structure requires onset detection + beat tracking.
- **Dynamics / loudness contour**: The log compression and optional c0 removal flatten dynamic range. A crescendo and a decrescendo with the same average loudness produce near-identical MFCC distributions. Use RMS energy or loudness models for dynamics.
- **Phase information**: The power spectrum discards phase. Sounds with identical magnitude spectra but different phase are indistinguishable by MFCCs.
- **Spatial information**: MFCCs operate on mono signals. Stereo width, panning, and spatial effects are lost.

### 3.3 Real-Time Timbre Classification

For visualization, a practical approach is to compute the MFCC mean + variance over a sliding window (e.g., 1 second = ~86 frames at 512 hop), then classify using a pre-trained model:

```
Feature vector: [mean(c1..c12), var(c1..c12)] = 24 dimensions
Classifier:     k-NN (k=5) or small random forest
Classes:        {vocal, drums, bass, pad, lead, noise, silence}
Latency:        ~1 second (window length)
Update rate:    Every hop (~11.6 ms)
```

This classification can drive visualization parameters: color palette selection, particle behavior, geometric complexity, etc.

---

## 4. Mel Spectrogram

### 4.1 Construction from STFT

A Mel spectrogram is the 2D time-frequency representation obtained by applying the Mel filterbank to successive STFT frames, without the DCT step. It preserves the full spectral resolution of the Mel filterbank.

**Pipeline**: Pre-emphasis (optional) -> Frame -> Window -> FFT -> Power Spectrum -> Mel Filterbank -> Log -> **Stop here** (no DCT).

The result is a matrix `S[m, t]` where `m` is the Mel filter index (0 to M-1) and `t` is the frame index:

```
S[m, t] = log(Σ_k P_t[k] * H_m[k] + ε)
```

### 4.2 Log-Mel Spectrogram as 2D Feature

The log-Mel spectrogram is the standard input representation for convolutional neural networks (CNNs) in audio:

- **Dimensions**: `(M, T)` where M = number of Mel bins (64, 128, or 256 are common for neural networks) and T = number of frames.
- **Interpretation**: Treat as a single-channel grayscale image. The vertical axis is perceptual frequency, the horizontal axis is time.
- **Normalization for neural networks**: Per-channel (per-Mel-bin) mean/variance normalization, or global min-max scaling to [0, 1].

**Why log-Mel over raw STFT for CNNs**: The Mel compression reduces the frequency dimension (e.g., from 513 FFT bins to 128 Mel bins) while preserving perceptually relevant resolution. The log scaling matches the approximately logarithmic nature of human loudness perception, making learned features more interpretable and training more stable.

### 4.3 Real-Time Streaming Implementation

For real-time visualization, the Mel spectrogram is computed frame-by-frame and stored in a circular buffer:

```cpp
class MelSpectrogramStream {
public:
    MelSpectrogramStream(int num_mel_bins, int history_frames)
        : num_bins_(num_mel_bins)
        , history_(history_frames)
        , buffer_(num_mel_bins * history_frames, 0.0f)
        , write_pos_(0) {}

    // Call once per hop with the log-mel vector from MFCCExtractor::get_log_mel()
    void push_frame(const float* log_mel) {
        int offset = write_pos_ * num_bins_;
        std::copy(log_mel, log_mel + num_bins_, buffer_.data() + offset);
        write_pos_ = (write_pos_ + 1) % history_;
    }

    // Get pointer to the circular buffer for GPU upload (texture).
    // The oldest frame starts at (write_pos_ * num_bins_).
    const float* data() const { return buffer_.data(); }
    int write_position() const { return write_pos_; }
    int width() const { return history_; }
    int height() const { return num_bins_; }

private:
    int num_bins_;
    int history_;
    std::vector<float> buffer_;
    int write_pos_;
};
```

**GPU visualization**: Upload the circular buffer as a 2D texture (`GL_R32F` or `GL_R16F`). In the fragment shader, offset the x-coordinate by `write_pos_` (modulo `history_`) to unwrap the circular buffer into a scrolling spectrogram. Apply a colormap (viridis, magma, inferno) in the shader.

### 4.4 Use as Neural Network Input

For real-time neural network inference (e.g., instrument classification, mood detection):

1. Accumulate a context window of Mel frames (e.g., 128 frames = ~1.5 seconds at 512 hop / 44.1 kHz)
2. Normalize: subtract dataset mean, divide by dataset std (pre-computed offline)
3. Feed to a lightweight CNN (e.g., MobileNet-v2 with 128x128 input)
4. Inference every N hops (e.g., every 32 hops = ~370 ms) to amortize cost

Inference latency budget for 60 fps visualization: ~16 ms per video frame. A quantized MobileNet on Apple Neural Engine runs in ~2 ms for 128x128 input, well within budget.

---

## 5. Chroma Features (Chromagram)

### 5.1 Twelve Pitch Class Bins

Chroma features collapse the frequency spectrum into 12 bins corresponding to the pitch classes of the Western chromatic scale:

```
Index:  0    1    2    3    4    5    6    7    8    9    10   11
Note:   C    C#   D    D#   E    F    F#   G    G#   A    A#   B
```

All octaves of a given pitch class contribute to the same bin. A C2 at 65.4 Hz and a C5 at 523.3 Hz both contribute to chroma bin 0. This octave-invariance makes chroma features ideal for harmonic analysis regardless of register.

### 5.2 Implementation from STFT

**Mapping FFT bins to pitch classes**:

Given FFT bin `k` with center frequency `f_k = k * fs / N`, its pitch class is determined by:

```
pitch_class(f_k) = round(12 * log2(f_k / f_ref)) mod 12
```

where `f_ref` is the reference frequency for A4 (440 Hz). Since `12 * log2(440 / 440) = 0` and A maps to chroma index 9, we adjust:

```
chroma_bin(f_k) = (round(12 * log2(f_k / f_ref)) + 9) mod 12
```

But using `f_ref = 261.63` (C4) simplifies: `chroma_bin = round(12 * log2(f_k / 261.63)) mod 12`, and C maps directly to bin 0.

**Full implementation**: Rather than hard-assigning each FFT bin to a single chroma bin, a better approach uses a chroma filterbank where each FFT bin contributes to its nearest chroma bins with Gaussian weighting (to handle tuning deviations):

```
Chroma filterbank:

      C    C#   D    D#   ...   B
k=0  [0    0    0    0    ...   0  ]   (DC, below musical range)
k=1  [0    0    0    0    ...   0  ]
...
k=j  [0.2  0.8  0    0    ...   0  ]   (between C and C#)
k=j+1[0    1.0  0    0    ...   0  ]   (exactly C#)
...
```

### 5.3 C++ Implementation

```cpp
#include <cmath>
#include <vector>
#include <algorithm>

class ChromaExtractor {
public:
    struct Config {
        int sample_rate = 44100;
        int fft_size    = 4096;     // Larger FFT for better low-freq resolution
        int hop_size    = 2048;
        float ref_freq  = 261.63f;  // C4
        float min_freq  = 65.0f;    // C2 -- ignore below this
        float max_freq  = 2100.0f;  // C7 -- ignore above this
    };

    explicit ChromaExtractor(const Config& cfg) : cfg_(cfg) {
        num_fft_bins_ = cfg_.fft_size / 2 + 1;
        build_chroma_filterbank();
    }

    // Input: power spectrum (num_fft_bins_ values).
    // Output: 12-element chroma vector (normalized).
    void extract(const float* power_spectrum, float* chroma_out) {
        // Apply chroma filterbank
        std::fill(chroma_out, chroma_out + 12, 0.0f);

        for (int k = min_bin_; k <= max_bin_; ++k) {
            int c = bin_to_chroma_[k];
            if (c >= 0)
                chroma_out[c] += power_spectrum[k] * bin_weight_[k];
        }

        // Normalize (L2 norm)
        float norm = 0.0f;
        for (int c = 0; c < 12; ++c)
            norm += chroma_out[c] * chroma_out[c];
        norm = std::sqrt(norm) + 1e-10f;
        for (int c = 0; c < 12; ++c)
            chroma_out[c] /= norm;
    }

private:
    void build_chroma_filterbank() {
        bin_to_chroma_.resize(num_fft_bins_, -1);
        bin_weight_.resize(num_fft_bins_, 0.0f);

        float bin_hz = static_cast<float>(cfg_.sample_rate) / cfg_.fft_size;
        min_bin_ = static_cast<int>(std::ceil(cfg_.min_freq / bin_hz));
        max_bin_ = static_cast<int>(std::floor(cfg_.max_freq / bin_hz));
        max_bin_ = std::min(max_bin_, num_fft_bins_ - 1);

        for (int k = min_bin_; k <= max_bin_; ++k) {
            float freq = k * bin_hz;
            if (freq < 1.0f) continue;

            // Continuous pitch in semitones relative to ref
            float semitones = 12.0f * std::log2(freq / cfg_.ref_freq);
            // Fractional chroma position
            float chroma_pos = std::fmod(semitones, 12.0f);
            if (chroma_pos < 0.0f) chroma_pos += 12.0f;

            int nearest_chroma = static_cast<int>(std::round(chroma_pos)) % 12;
            float deviation = chroma_pos - nearest_chroma;
            if (deviation > 6.0f) deviation -= 12.0f;
            if (deviation < -6.0f) deviation += 12.0f;

            // Gaussian weighting (sigma = 0.5 semitones)
            float weight = std::exp(-0.5f * deviation * deviation / (0.5f * 0.5f));

            if (weight > 0.01f) {  // Threshold to avoid noise contributions
                bin_to_chroma_[k] = nearest_chroma;
                bin_weight_[k] = weight;
            }
        }
    }

    Config cfg_;
    int num_fft_bins_;
    int min_bin_;
    int max_bin_;
    std::vector<int> bin_to_chroma_;
    std::vector<float> bin_weight_;
};
```

### 5.4 Chroma Normalization Strategies

Different normalization choices affect downstream tasks:

| Strategy | Formula | Best For |
|----------|---------|----------|
| **L1 norm** | `c[i] /= Σ|c[j]|` | When relative pitch distribution matters; robust to silence |
| **L2 norm** | `c[i] /= sqrt(Σc[j]^2)` | Distance-based matching, template correlation |
| **Max norm** | `c[i] /= max(c[j])` | When the dominant pitch class should always be 1.0 |
| **Threshold + L2** | Clamp to [0, quantile_95], then L2 | Reduces impact of harmonic bleed; best for chord recognition |
| **None** | Raw energy per bin | When absolute energy matters (e.g., energy-weighted visualizations) |

For visualization, **max-norm** is often best: the strongest pitch class is always 1.0, providing consistent visual range regardless of volume.

### 5.5 Key Detection and Chord Recognition

**Key detection** from chroma: Correlate the averaged chroma vector against key profiles (Krumhansl-Kessler profiles):

```
C major profile: [6.35, 2.23, 3.48, 2.33, 4.38, 4.09, 2.52, 5.19, 2.39, 3.66, 2.29, 2.88]
C minor profile: [6.33, 2.68, 3.52, 5.38, 2.60, 3.53, 2.54, 4.75, 3.98, 2.69, 3.34, 3.17]
```

Rotate these profiles through all 12 keys (circular shift) and pick the maximum Pearson correlation. This gives 24 possible keys (12 major + 12 minor).

**Chord recognition**: Use chroma templates for common chord types:

```
C major:  [1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0]  (C, E, G)
C minor:  [1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0]  (C, Eb, G)
C7:       [1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0]  (C, E, G, Bb)
```

Rotate each template through 12 positions, correlate with the observed chroma, and pick the best match. For real-time visualization, this provides a chord label that can drive harmonic color mapping.

---

## 6. Tonnetz (Tonal Centroid Features)

### 6.1 Six-Dimensional Tonal Centroid

The Tonnetz (German: "tone network") is a geometric representation of tonal relationships. It maps the 12 chroma values into a 6-dimensional space that encodes three fundamental harmonic intervals:

1. **Fifths** (7 semitones, frequency ratio 3:2)
2. **Minor thirds** (3 semitones, ratio 6:5)
3. **Major thirds** (4 semitones, ratio 5:4)

Each interval is represented by a pair of coordinates (cos, sin) on a unit circle, giving 6 dimensions total.

### 6.2 Mathematical Definition

Given a normalized chroma vector `c[0..11]` (L1-normalized so `Σc[i] = 1`), the 6 Tonnetz coordinates are:

```
φ_1 = Σ_{i=0}^{11} c[i] * sin(i * 7π/6)      (fifths, sine)
φ_2 = Σ_{i=0}^{11} c[i] * cos(i * 7π/6)      (fifths, cosine)
φ_3 = Σ_{i=0}^{11} c[i] * sin(i * 3π/2)      (minor thirds, sine -- period 4 = 12/3)
φ_4 = Σ_{i=0}^{11} c[i] * cos(i * 3π/2)      (minor thirds, cosine)
φ_5 = Σ_{i=0}^{11} c[i] * sin(i * 2π/3)      (major thirds, sine -- period 3 = 12/4... wait)
φ_6 = Σ_{i=0}^{11} c[i] * cos(i * 2π/3)      (major thirds, cosine)
```

More precisely, using the standard formulation from Harte et al. (2006):

```
φ_1 = Σ c[i] * sin(i * 7 * 2π / 12)    = Σ c[i] * sin(i * 7π / 6)
φ_2 = Σ c[i] * cos(i * 7 * 2π / 12)    = Σ c[i] * cos(i * 7π / 6)
φ_3 = Σ c[i] * sin(i * 3 * 2π / 12)    = Σ c[i] * sin(i * π / 2)
φ_4 = Σ c[i] * cos(i * 3 * 2π / 12)    = Σ c[i] * cos(i * π / 2)
φ_5 = Σ c[i] * sin(i * 4 * 2π / 12)    = Σ c[i] * sin(i * 2π / 3)
φ_6 = Σ c[i] * cos(i * 4 * 2π / 12)    = Σ c[i] * cos(i * 2π / 3)
```

The multipliers (7, 3, 4) correspond to the semitone intervals for fifths, minor thirds, and major thirds. Each pair `(φ_{2k-1}, φ_{2k})` lives on a unit disk; its angle encodes which pitch class dominates that interval axis, and its radius encodes how strongly concentrated the chroma energy is along that axis.

### 6.3 Implementation

```cpp
class TonnetzExtractor {
public:
    TonnetzExtractor() {
        // Precompute sin/cos tables for the three interval circles
        // Intervals: fifths (7 semitones), minor 3rds (3), major 3rds (4)
        const int intervals[3] = {7, 3, 4};

        for (int axis = 0; axis < 3; ++axis) {
            for (int i = 0; i < 12; ++i) {
                float angle = i * intervals[axis] * 2.0f * M_PI / 12.0f;
                sin_table_[axis][i] = std::sin(angle);
                cos_table_[axis][i] = std::cos(angle);
            }
        }
    }

    // Input: L1-normalized chroma vector (12 values summing to ~1).
    // Output: 6-dimensional Tonnetz vector.
    void extract(const float* chroma, float* tonnetz_out) {
        for (int axis = 0; axis < 3; ++axis) {
            float s = 0.0f, c = 0.0f;
            for (int i = 0; i < 12; ++i) {
                s += chroma[i] * sin_table_[axis][i];
                c += chroma[i] * cos_table_[axis][i];
            }
            tonnetz_out[axis * 2]     = s;
            tonnetz_out[axis * 2 + 1] = c;
        }
    }

private:
    float sin_table_[3][12];
    float cos_table_[3][12];
};
```

### 6.4 Harmonic Analysis with Tonnetz

Key properties of the Tonnetz representation:

- **Major chords** cluster tightly in Tonnetz space because they span one major third + one minor third (C-E-G = 4+3 semitones). Their centroid has high radius on all three axes.
- **Minor chords** cluster differently: C-Eb-G = 3+4 semitones. The fifths component is identical to the relative major, but the thirds axes rotate.
- **Key proximity**: Related keys (e.g., C major and G major, sharing 6 of 7 diatonic notes) produce nearby Tonnetz centroids. Modulations trace smooth paths through the space.
- **Diminished/augmented chords** have distinctive signatures: diminished chords collapse the minor-thirds axis (they evenly divide the octave into minor thirds), augmented chords collapse the major-thirds axis.

**Chord transition detection**: The Euclidean distance between successive Tonnetz frames serves as a harmonic change detector. Peaks in `||tonnetz[t] - tonnetz[t-1]||` indicate chord changes, analogous to spectral flux for onset detection (see [FEATURES_spectral.md](FEATURES_spectral.md)).

### 6.5 Visual Mapping Possibilities

The 6 Tonnetz dimensions map naturally to visual parameters:

```
Tonnetz Axis          Visual Mapping Ideas
─────────────         ─────────────────────
φ_1,2 (fifths)        Color hue rotation (harmonic "warmth")
φ_3,4 (minor 3rds)    Saturation / intensity (tension)
φ_5,6 (major 3rds)    Brightness / scale (resolution)

Radius (√(φ_1²+φ_2²)) → Confidence / opacity (strong tonal center = high radius)
Angular velocity       → Animation speed (fast modulation = fast visual change)
```

The 3 axis pairs can also be visualized directly as 3 points on 3 unit circles, forming a "tonal compass" display:

```
     Fifths          Minor 3rds       Major 3rds
    ╭──────╮         ╭──────╮         ╭──────╮
   ╱   •    ╲       ╱        ╲       ╱  •     ╲
  │  (angle  │     │    •     │     │  (angle  │
  │ = dom.   │     │          │     │ = dom.   │
   ╲  fifth)╱       ╲        ╱       ╲  3rd)  ╱
    ╰──────╯         ╰──────╯         ╰──────╯
```

---

## 7. Real-Time Streaming Implementation

### 7.1 Architecture Overview

All four feature types (MFCC, Mel spectrogram, chroma, Tonnetz) share the same front-end: framed, windowed, FFTed audio. The streaming architecture computes the shared FFT once and fans out to each feature extractor:

```
Audio Input (callback buffer, e.g. 256 samples)
     │
     ▼
┌──────────────────────────┐
│  Ring Buffer (accumulate  │
│  until frame_size reached)│
└─────────┬────────────────┘
          │ frame ready (1024 samples)
          ▼
┌──────────────────────────┐
│  Pre-emphasis + Window   │
│  + FFT → Power Spectrum  │
└─────┬──────┬─────────────┘
      │      │
      ▼      ▼
  ┌───────┐ ┌────────────────────┐
  │ Mel   │ │ Chroma Filterbank  │
  │ Bank  │ │ (from same power   │
  │       │ │  spectrum, but     │
  │       │ │  larger FFT ideal) │
  └───┬───┘ └────────┬───────────┘
      │              │
      ▼              ▼
  ┌───────┐     ┌──────────┐
  │ Log   │     │ Normalize│
  │ Mel   │     │ Chroma   │
  └─┬───┬─┘    └────┬─────┘
    │   │            │
    ▼   ▼            ▼
 ┌────┐┌─────┐  ┌────────┐
 │DCT ││ Mel │  │Tonnetz │
 │→   ││Spec.│  │(from   │
 │MFCC││Buf. │  │chroma) │
 └────┘└─────┘  └────────┘
```

### 7.2 Ring Buffer and Overlap Management

Audio callbacks deliver variable-sized buffers (commonly 256 or 512 samples). The analysis frame size (1024 for MFCCs, 4096 for chroma) is typically larger. A ring buffer accumulates samples and triggers feature extraction at each hop boundary:

```cpp
class StreamingFeatureExtractor {
public:
    struct Config {
        int callback_size      = 256;    // Audio callback buffer size
        int mfcc_frame_size    = 1024;   // MFCC analysis frame
        int mfcc_hop_size      = 512;    // MFCC hop
        int chroma_frame_size  = 4096;   // Chroma needs longer window
        int chroma_hop_size    = 2048;   // Chroma hop
        int sample_rate        = 44100;
    };

    explicit StreamingFeatureExtractor(const Config& cfg)
        : cfg_(cfg)
        , ring_buffer_(cfg.chroma_frame_size * 4, 0.0f) // 4x largest frame
        , write_pos_(0)
        , mfcc_read_pos_(0)
        , chroma_read_pos_(0)
        , samples_since_mfcc_hop_(0)
        , samples_since_chroma_hop_(0)
    {
        MFCCExtractor::Config mfcc_cfg;
        mfcc_cfg.sample_rate = cfg.sample_rate;
        mfcc_cfg.frame_size  = cfg.mfcc_frame_size;
        mfcc_extractor_ = std::make_unique<MFCCExtractor>(mfcc_cfg);

        ChromaExtractor::Config chroma_cfg;
        chroma_cfg.sample_rate = cfg.sample_rate;
        chroma_cfg.fft_size    = cfg.chroma_frame_size;
        chroma_extractor_ = std::make_unique<ChromaExtractor>(chroma_cfg);

        mfcc_output_.resize(mfcc_extractor_->num_coeffs());
        chroma_output_.resize(12);
        tonnetz_output_.resize(6);
    }

    // Called from audio thread. Must be lock-free.
    void process_audio_block(const float* input, int num_samples) {
        int buf_size = static_cast<int>(ring_buffer_.size());

        // Write to ring buffer
        for (int i = 0; i < num_samples; ++i) {
            ring_buffer_[(write_pos_ + i) % buf_size] = input[i];
        }
        write_pos_ = (write_pos_ + num_samples) % buf_size;

        samples_since_mfcc_hop_ += num_samples;
        samples_since_chroma_hop_ += num_samples;

        // Check if enough samples for MFCC hop(s)
        while (samples_since_mfcc_hop_ >= cfg_.mfcc_hop_size) {
            extract_mfcc_frame();
            samples_since_mfcc_hop_ -= cfg_.mfcc_hop_size;
        }

        // Check if enough samples for chroma hop(s)
        while (samples_since_chroma_hop_ >= cfg_.chroma_hop_size) {
            extract_chroma_frame();
            samples_since_chroma_hop_ -= cfg_.chroma_hop_size;
        }
    }

    // Accessors for visualization thread (read atomically or with lock)
    const std::vector<float>& mfcc() const { return mfcc_output_; }
    const std::vector<float>& chroma() const { return chroma_output_; }
    const std::vector<float>& tonnetz() const { return tonnetz_output_; }
    const std::vector<float>& log_mel() const {
        return mfcc_extractor_->get_log_mel();
    }

private:
    void extract_mfcc_frame() {
        // Copy frame from ring buffer (handles wraparound)
        std::vector<float> frame(cfg_.mfcc_frame_size);
        int buf_size = static_cast<int>(ring_buffer_.size());
        for (int i = 0; i < cfg_.mfcc_frame_size; ++i)
            frame[i] = ring_buffer_[(mfcc_read_pos_ + i) % buf_size];
        mfcc_read_pos_ = (mfcc_read_pos_ + cfg_.mfcc_hop_size) % buf_size;

        mfcc_extractor_->extract_frame(frame.data(), mfcc_output_.data());
        // log_mel is available via mfcc_extractor_->get_log_mel()
    }

    void extract_chroma_frame() {
        // Copy frame, compute FFT + power spectrum, extract chroma
        std::vector<float> frame(cfg_.chroma_frame_size);
        int buf_size = static_cast<int>(ring_buffer_.size());
        for (int i = 0; i < cfg_.chroma_frame_size; ++i)
            frame[i] = ring_buffer_[(chroma_read_pos_ + i) % buf_size];
        chroma_read_pos_ = (chroma_read_pos_ + cfg_.chroma_hop_size) % buf_size;

        // Window + FFT (reuse same pattern as MFCC but with larger frame)
        int N = cfg_.chroma_frame_size;
        int num_bins = N / 2 + 1;
        std::vector<float> windowed(N);
        for (int i = 0; i < N; ++i)
            windowed[i] = frame[i] * 0.5f * (1.0f - std::cos(2.0f * M_PI * i / (N - 1)));

        std::vector<float> fft_re(N), fft_im(N);
        fft_real(windowed.data(), fft_re.data(), fft_im.data(), N);

        std::vector<float> power(num_bins);
        float inv_n = 1.0f / N;
        for (int k = 0; k < num_bins; ++k)
            power[k] = (fft_re[k] * fft_re[k] + fft_im[k] * fft_im[k]) * inv_n;

        chroma_extractor_->extract(power.data(), chroma_output_.data());

        // Tonnetz from chroma (L1-normalize first)
        std::vector<float> chroma_l1(12);
        float sum = 0.0f;
        for (int i = 0; i < 12; ++i) sum += chroma_output_[i];
        if (sum > 1e-10f)
            for (int i = 0; i < 12; ++i) chroma_l1[i] = chroma_output_[i] / sum;
        else
            std::fill(chroma_l1.begin(), chroma_l1.end(), 1.0f / 12.0f);

        tonnetz_.extract(chroma_l1.data(), tonnetz_output_.data());
    }

    Config cfg_;
    std::vector<float> ring_buffer_;
    int write_pos_;
    int mfcc_read_pos_;
    int chroma_read_pos_;
    int samples_since_mfcc_hop_;
    int samples_since_chroma_hop_;

    std::unique_ptr<MFCCExtractor> mfcc_extractor_;
    std::unique_ptr<ChromaExtractor> chroma_extractor_;
    TonnetzExtractor tonnetz_;

    std::vector<float> mfcc_output_;
    std::vector<float> chroma_output_;
    std::vector<float> tonnetz_output_;
};
```

### 7.3 Thread Safety Considerations

The audio callback runs on a real-time thread. Feature extraction (FFT, filterbank, DCT) must not allocate memory, lock mutexes, or make system calls. The implementation above pre-allocates all buffers in the constructor.

For communicating features to the visualization thread, use one of:

1. **Lock-free SPSC (single-producer, single-consumer) queue**: Audio thread pushes feature frames; render thread pops. Best for logging all frames.
2. **Triple buffer**: Audio thread writes to back buffer, atomically swaps. Render thread reads front buffer. Minimal latency, but drops intermediate frames if render is slower than audio. Best for visualization where only the latest value matters.
3. **Atomic snapshot**: For small feature vectors (13 MFCCs, 12 chroma, 6 Tonnetz), use `std::atomic<FeatureSnapshot>` where `FeatureSnapshot` fits in a cache line (128 bytes). Not standard-guaranteed to be lock-free for large types, but works on x86-64 with 128-byte structs.

### 7.4 Latency Budget

| Feature | Frame Size | Hop Size | Latency (ms) | Update Rate (Hz) |
|---------|-----------|----------|---------------|-------------------|
| MFCC    | 1024      | 512      | 23.2          | 86.1             |
| Mel Spec| 1024      | 512      | 23.2          | 86.1             |
| Chroma  | 4096      | 2048     | 92.9          | 21.5             |
| Tonnetz | (from chroma) | (from chroma) | 92.9 | 21.5            |

Chroma requires larger windows for adequate low-frequency resolution (a C2 at 65.4 Hz has a period of ~15.3 ms; at 4096/44100 = 92.9 ms we get ~6 full cycles, which is marginal). For lower latencies, use CQT (Constant-Q Transform) instead of STFT for chroma -- it provides better low-frequency resolution with smaller windows, at the cost of higher computational complexity (see [FEATURES_pitch_harmonic.md](FEATURES_pitch_harmonic.md)).

### 7.5 Optimization Notes

- **SIMD for Mel filterbank**: The filterbank multiply-accumulate is embarrassingly vectorizable. Use NEON (ARM) or SSE/AVX (x86) intrinsics for 4--8x speedup on the inner loop.
- **Pre-computed FFT plan**: Reuse the FFT plan across frames (FFTW `fftw_plan`, or pffft setup). Plan creation is expensive; execution is cheap.
- **Shared power spectrum**: When MFCC and chroma use the same FFT size, compute the power spectrum once. When they differ (1024 vs 4096), maintain separate pipelines.
- **Downsampling for chroma**: Chroma features only need content below ~2 kHz. Downsample to 8 kHz before the 4096-point FFT to reduce the FFT to 735 points (round to 1024), saving ~75% compute.
- **Batch DCT**: If computing MFCCs for multiple frames (e.g., catching up after a large callback), batch the DCT as a matrix multiply and leverage BLAS.

---

## Appendix A: Comparison Summary

| Feature | Dimensions | Captures | Invariant To | Update Rate | Compute Cost |
|---------|-----------|----------|-------------|-------------|-------------|
| MFCC (13) | 13 | Timbre, spectral envelope | Pitch, phase | ~86 Hz | Low |
| MFCC + Δ + ΔΔ | 39 | Timbre + dynamics | Pitch, phase | ~86 Hz | Low |
| Mel Spectrogram | M x T (e.g. 128 x T) | Full spectral content (perceptual scale) | Phase | ~86 Hz | Low |
| Chroma | 12 | Harmonic content, pitch classes | Octave, timbre | ~21 Hz | Medium |
| Tonnetz | 6 | Tonal relationships (fifths, thirds) | Octave, timbre, inversion | ~21 Hz | Negligible (from chroma) |

## Appendix B: References

- Davis, S.B. & Mermelstein, P. (1980). "Comparison of Parametric Representations for Monosyllabic Word Recognition in Continuously Spoken Sentences." IEEE TASSP.
- Harte, C., Sandler, M., & Gasser, M. (2006). "Detecting Harmonic Change in Musical Audio." ACM Multimedia Workshop on Audio and Music Computing.
- Müller, M. (2015). *Fundamentals of Music Processing*. Springer. Chapters 3 (chroma), 7 (Tonnetz).
- Stevens, S.S., Volkmann, J., & Newman, E.B. (1937). "A Scale for the Measurement of the Psychological Magnitude of Pitch." JASA.

## Appendix C: Cross-Reference Index

| Topic | Primary Document | Related Sections Here |
|-------|-----------------|----------------------|
| Spectral centroid, rolloff, flux | FEATURES_spectral.md | Section 2.4 (c1 ≈ centroid) |
| Octave and bark bands | FEATURES_frequency_bands.md | Section 1.4 (Mel vs. bark) |
| f0, harmonics, HPS | FEATURES_pitch_harmonic.md | Section 3 (MFCCs don't capture pitch), Section 5 (CQT for chroma) |
| Essentia MFCC/chroma API | LIB_essentia.md | All sections (alternative to custom impl) |
| FFT, DCT, filterbank math | REF_math_reference.md | Sections 1.3, 1.6, 1.4 |
