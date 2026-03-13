# Mathematical Foundations Reference for Real-Time Audio Analysis

> Every formula a developer needs, with derivation context, implementation notes, and cross-references to feature documentation.

**Scope**: This document is a self-contained mathematical reference. It covers the transforms, window functions, perceptual scales, feature extraction formulas, filter design equations, and algebraic identities required to implement a complete real-time audio analysis pipeline. All formulas assume discrete-time signals sampled at rate `f_s` with `N`-sample analysis frames unless stated otherwise.

---

## 1. Discrete Fourier Transform (DFT)

### 1.1 Forward DFT

The DFT maps an N-point discrete-time signal `x[n]` to its frequency-domain representation `X[k]`:

```
X[k] = Σ_{n=0}^{N-1} x[n] * e^{-j2πkn/N},    k = 0, 1, ..., N-1
```

where `j = sqrt(-1)`. Each output bin `X[k]` is a complex number encoding the amplitude and phase of the sinusoidal component at frequency index `k`.

Expanding via Euler's formula:

```
X[k] = Σ_{n=0}^{N-1} x[n] * [cos(2πkn/N) - j*sin(2πkn/N)]
```

The real part captures the cosine projection, the imaginary part captures the (negated) sine projection. Computational complexity of the naive DFT is O(N^2).

### 1.2 Inverse DFT (IDFT)

The inverse transform recovers the time-domain signal:

```
x[n] = (1/N) * Σ_{k=0}^{N-1} X[k] * e^{j2πkn/N},    n = 0, 1, ..., N-1
```

The `1/N` normalization factor ensures perfect reconstruction: IDFT(DFT(x)) = x. Some implementations split the normalization as `1/sqrt(N)` on both transforms (unitary convention).

### 1.3 Fast Fourier Transform (FFT) -- Cooley-Tukey Algorithm

The radix-2 Cooley-Tukey algorithm exploits the periodicity and symmetry of the complex exponential `W_N = e^{-j2π/N}` (the N-th root of unity) to reduce the DFT from O(N^2) to O(N log_2 N).

**Decimation-in-time decomposition** (N must be a power of 2):

```
X[k] = Σ_{m=0}^{N/2-1} x[2m] * W_{N/2}^{mk}  +  W_N^k * Σ_{m=0}^{N/2-1} x[2m+1] * W_{N/2}^{mk}
```

Equivalently:

```
X[k] = E[k] + W_N^k * O[k]
X[k + N/2] = E[k] - W_N^k * O[k]
```

where `E[k]` is the N/2-point DFT of the even-indexed samples and `O[k]` is the N/2-point DFT of the odd-indexed samples. This "butterfly" operation is applied recursively log_2(N) times.

**Twiddle factors**: `W_N^k = e^{-j2πk/N}`. Precompute and store these for performance.

**Operation counts** for N-point radix-2 FFT:
- Complex multiplications: (N/2) * log_2(N)
- Complex additions: N * log_2(N)

| N    | Naive DFT ops (N^2) | FFT ops (N/2 * log_2 N) | Speedup |
|------|----------------------|--------------------------|---------|
| 256  | 65,536               | 1,024                    | 64x     |
| 512  | 262,144              | 2,304                    | 114x    |
| 1024 | 1,048,576            | 5,120                    | 205x    |
| 2048 | 4,194,304            | 11,264                   | 372x    |
| 4096 | 16,777,216           | 24,576                   | 683x    |

### 1.4 Frequency-Domain Relationships

**Bin frequency** -- the center frequency of bin `k`:

```
f_k = k * f_s / N    [Hz]
```

**Frequency resolution** -- the spacing between adjacent bins:

```
Δf = f_s / N    [Hz]
```

For `f_s = 44100 Hz`, `N = 2048`: `Δf = 21.53 Hz`. For `N = 4096`: `Δf = 10.77 Hz`.

**Nyquist frequency** -- the maximum representable frequency:

```
f_N = f_s / 2    [Hz]
```

Only bins `k = 0` to `k = N/2` contain unique information for real-valued signals (the upper half mirrors the lower half via conjugate symmetry: `X[N-k] = X[k]*`).

**Time-frequency uncertainty principle** (Gabor limit):

```
Δt * Δf >= 1 / (4π)
```

where `Δt` is the temporal resolution (RMS width of the time window) and `Δf` is the spectral resolution (RMS bandwidth). This is a fundamental limit: you cannot simultaneously achieve arbitrarily fine resolution in both time and frequency. Longer windows improve frequency resolution at the cost of temporal resolution, and vice versa.

Practical consequence for frame size selection:

| Frame size N | Δt at 44.1kHz (ms) | Δf (Hz) | Δt * Δf product |
|-------------|---------------------|---------|-----------------|
| 256         | 5.8                 | 172.3   | 999             |
| 512         | 11.6                | 86.1    | 999             |
| 1024        | 23.2                | 43.1    | 999             |
| 2048        | 46.4                | 21.5    | 999             |
| 4096        | 92.9                | 10.8    | 999             |

The product is constant (within the discrete approximation) -- this is the uncertainty tradeoff in action.

### 1.5 Reference Implementation (C++)

```cpp
#include <complex>
#include <vector>
#include <cmath>

using Complex = std::complex<float>;

// In-place radix-2 Cooley-Tukey FFT
void fft(std::vector<Complex>& x) {
    const size_t N = x.size();
    if (N <= 1) return;

    // Bit-reversal permutation
    for (size_t i = 1, j = 0; i < N; ++i) {
        size_t bit = N >> 1;
        for (; j & bit; bit >>= 1) {
            j ^= bit;
        }
        j ^= bit;
        if (i < j) std::swap(x[i], x[j]);
    }

    // Butterfly stages
    for (size_t len = 2; len <= N; len <<= 1) {
        float angle = -2.0f * M_PI / static_cast<float>(len);
        Complex wlen(std::cos(angle), std::sin(angle));
        for (size_t i = 0; i < N; i += len) {
            Complex w(1.0f, 0.0f);
            for (size_t j = 0; j < len / 2; ++j) {
                Complex u = x[i + j];
                Complex v = x[i + j + len / 2] * w;
                x[i + j] = u + v;
                x[i + j + len / 2] = u - v;
                w *= wlen;
            }
        }
    }
}
```

> Cross-reference: [LIB_fft_comparison.md](LIB_fft_comparison.md) for library-specific FFT APIs (FFTW, KissFFT, pffft, Accelerate vDSP).

---

## 2. Window Functions

Windowing multiplies the time-domain frame `x[n]` by a window `w[n]` before the FFT to reduce spectral leakage. The choice of window trades off between main lobe width (frequency resolution) and side lobe attenuation (leakage suppression).

All windows below are defined for `n = 0, 1, ..., N-1`.

### 2.1 Rectangular (Dirichlet)

```
w[n] = 1
```

- **Main lobe width**: 2 bins (4π/N rad)
- **First side lobe**: -13.3 dB
- **Side lobe rolloff**: -6 dB/octave
- **Use case**: Maximum frequency resolution when spectral leakage is acceptable; analysis of transients where windowing would smear the onset.
- **Coherent gain**: 1.0
- **ENBW**: 1.0 bin

### 2.2 Hann (Hanning)

```
w[n] = 0.5 * (1 - cos(2πn / (N-1)))
```

- **Main lobe width**: 4 bins (8π/N rad)
- **First side lobe**: -31.5 dB
- **Side lobe rolloff**: -18 dB/octave
- **Use case**: General-purpose spectral analysis. Good compromise between resolution and leakage. Standard choice for STFT with 50% overlap.
- **Coherent gain**: 0.5
- **ENBW**: 1.5 bins

### 2.3 Hamming

```
w[n] = 0.54 - 0.46 * cos(2πn / (N-1))
```

- **Main lobe width**: 4 bins (8π/N rad)
- **First side lobe**: -42.7 dB
- **Side lobe rolloff**: -6 dB/octave (discontinuity at edges prevents fast rolloff)
- **Use case**: Speech processing (MFCC computation). The non-zero endpoints cause slower side lobe decay than Hann but lower first side lobe.
- **Coherent gain**: 0.54
- **ENBW**: 1.36 bins

### 2.4 Blackman

```
w[n] = 0.42 - 0.5 * cos(2πn / (N-1)) + 0.08 * cos(4πn / (N-1))
```

- **Main lobe width**: 6 bins (12π/N rad)
- **First side lobe**: -58.1 dB
- **Side lobe rolloff**: -18 dB/octave
- **Use case**: When strong leakage suppression is needed and wider main lobe is acceptable. Good for spectral feature extraction on mixed signals.
- **Coherent gain**: 0.42
- **ENBW**: 1.73 bins

### 2.5 Blackman-Harris (4-term)

```
w[n] = a_0 - a_1*cos(2πn/(N-1)) + a_2*cos(4πn/(N-1)) - a_3*cos(6πn/(N-1))
```

Coefficients:

```
a_0 = 0.35875
a_1 = 0.48829
a_2 = 0.14128
a_3 = 0.01168
```

- **Main lobe width**: 8 bins
- **First side lobe**: -92.0 dB
- **Side lobe rolloff**: -6 dB/octave
- **Use case**: High dynamic range spectral analysis. When you need to resolve weak components near strong ones (> 90 dB separation).
- **Coherent gain**: 0.36
- **ENBW**: 2.0 bins

### 2.6 Kaiser

```
w[n] = I_0(β * sqrt(1 - ((2n/(N-1)) - 1)^2)) / I_0(β)
```

where `I_0` is the zero-order modified Bessel function of the first kind. The parameter `β` controls the tradeoff:

| β     | Side lobe (dB) | Main lobe width | Approximate equivalent |
|-------|----------------|-----------------|----------------------|
| 0     | -13.3          | 2 bins          | Rectangular          |
| 5.0   | -36.7          | ~4 bins         | ~Hann                |
| 6.0   | -44.2          | ~5 bins         | ~Hamming             |
| 8.6   | -68.6          | ~6 bins         | ~Blackman            |
| 14.0  | -114.8         | ~10 bins        | Extreme suppression  |

**Design formula** (Kaiser's empirical relation): Given desired side lobe attenuation `A` (in dB):

```
β = 0.1102 * (A - 8.7)             if A > 50
β = 0.5842 * (A - 21)^0.4 + 0.07886 * (A - 21)   if 21 <= A <= 50
β = 0                               if A < 21
```

- **Use case**: Tunable window -- single parameter adjusts the resolution/leakage tradeoff continuously. Optimal in the sense of maximizing energy concentration in the main lobe for a given side lobe level.

### 2.7 Flat-Top

```
w[n] = a_0 - a_1*cos(2πn/(N-1)) + a_2*cos(4πn/(N-1)) - a_3*cos(6πn/(N-1)) + a_4*cos(8πn/(N-1))
```

Coefficients (HFT90D):

```
a_0 = 1.0
a_1 = 1.942604
a_2 = 1.340318
a_3 = 0.440811
a_4 = 0.043097
```

- **Main lobe width**: 10 bins
- **First side lobe**: -90.2 dB
- **Amplitude accuracy**: < 0.01 dB (the main lobe is flat at the top)
- **Use case**: Amplitude-accurate measurements (calibration, tuner displays). The flat main lobe ensures that a tone falling between bins still reads the correct amplitude.
- **Coherent gain**: ~0.22
- **ENBW**: 3.77 bins

### 2.8 Comparison Table

| Window         | Main Lobe (bins) | 1st Side Lobe (dB) | Rolloff (dB/oct) | ENBW (bins) | Coherent Gain | 50% Overlap COLA |
|----------------|:----------------:|:-------------------:|:-----------------:|:-----------:|:-------------:|:----------------:|
| Rectangular    | 2                | -13.3               | -6                | 1.00        | 1.00          | No               |
| Hann           | 4                | -31.5               | -18               | 1.50        | 0.50          | Yes              |
| Hamming        | 4                | -42.7               | -6                | 1.36        | 0.54          | No (use 75%)     |
| Blackman       | 6                | -58.1               | -18               | 1.73        | 0.42          | Yes (67%)        |
| Blackman-Harris| 8                | -92.0               | -6                | 2.00        | 0.36          | Yes (67%)        |
| Kaiser (β=6)   | 5                | -44.2               | varies            | ~1.5        | ~0.49         | Depends on β     |
| Flat-Top       | 10               | -90.2               | -6                | 3.77        | ~0.22         | Yes (75%)        |

**COLA** = Constant Overlap-Add. Required for perfect reconstruction in STFT synthesis.

### 2.9 Overlap-Add Constraint

For STFT resynthesis, the window must satisfy the Constant Overlap-Add (COLA) property:

```
Σ_m w[n - m*H] = C    for all n
```

where `H` is the hop size and `C` is a constant. The Hann window at 50% overlap (`H = N/2`) satisfies COLA with `C = 1`.

### 2.10 Windowed DFT

The windowed DFT is simply:

```
X_w[k] = Σ_{n=0}^{N-1} w[n] * x[n] * e^{-j2πkn/N}
```

To recover correct amplitudes from a windowed FFT, divide by the coherent gain (sum of window values / N).

---

## 3. Perceptual Frequency Scales

Human hearing does not perceive frequency linearly. These scales approximate the ear's frequency resolution, which is roughly logarithmic above ~500 Hz.

### 3.1 Mel Scale (O'Shaughnessy, 1987)

**Frequency to Mel**:

```
m = 2595 * log10(1 + f/700)
```

**Mel to Frequency**:

```
f = 700 * (10^(m/2595) - 1)
```

The Mel scale is approximately linear below 1 kHz and logarithmic above. It was derived from perceptual experiments on pitch perception.

| Frequency (Hz) | Mel   |
|-----------------|-------|
| 0               | 0     |
| 100             | 150   |
| 200             | 284   |
| 500             | 607   |
| 1000            | 1000  |
| 2000            | 1500  |
| 4000            | 2146  |
| 8000            | 2840  |
| 16000           | 3564  |

### 3.2 Bark Scale (Zwicker & Terhardt, 1980)

**Frequency to Bark**:

```
z = 13 * arctan(0.00076 * f) + 3.5 * arctan((f/7500)^2)
```

**Bark to Frequency** (approximation by Traunmuller):

```
f = 1960 * (z + 0.53) / (26.28 - z)    for z > 2
```

One Bark corresponds approximately to one critical band of the auditory system.

| Frequency (Hz) | Bark  |
|-----------------|-------|
| 0               | 0.00  |
| 100             | 1.00  |
| 200             | 1.95  |
| 500             | 4.53  |
| 1000            | 8.01  |
| 2000            | 12.02 |
| 4000            | 16.71 |
| 8000            | 21.01 |
| 16000           | 24.42 |

### 3.3 ERB Scale (Glasberg & Moore, 1990)

The Equivalent Rectangular Bandwidth at frequency `f`:

```
ERB(f) = 24.7 * (4.37 * f/1000 + 1)    [Hz]
```

This gives the bandwidth of the auditory filter centered at `f`. The ERB-rate scale (number of ERBs from 0 Hz):

```
ERB_rate(f) = 21.4 * log10(1 + 0.00437 * f)
```

**Inverse**:

```
f = (10^(ERB_rate / 21.4) - 1) / 0.00437
```

| Frequency (Hz) | ERB (Hz) | ERB-rate |
|-----------------|----------|----------|
| 100             | 35.5     | 3.36     |
| 500             | 78.7     | 9.26     |
| 1000            | 132.6    | 13.64    |
| 2000            | 240.0    | 18.48    |
| 4000            | 456.5    | 23.12    |
| 8000            | 888.2    | 27.28    |
| 16000           | 1749.1   | 30.66    |

### 3.4 Scale Comparison

At low frequencies (< 500 Hz), all three scales are roughly linear and close to each other. Above 1 kHz, they diverge:

- **Mel**: Widely used in speech (MFCC). Simple formula. Slightly less physiologically accurate than ERB.
- **Bark**: 24 critical bands spanning 0-15.5 kHz. Used in psychoacoustic models (MPEG audio coding, perceptual entropy).
- **ERB**: Most physiologically accurate for modern auditory filter models. Preferred in computational auditory scene analysis (CASA).

> Cross-reference: [FEATURES_mfcc_mel.md](FEATURES_mfcc_mel.md), [FEATURES_frequency_bands.md](FEATURES_frequency_bands.md).

---

## 4. MFCC Derivation

Mel-Frequency Cepstral Coefficients capture the spectral envelope shape on a perceptually motivated scale. The full processing chain:

### 4.1 Pre-emphasis

High-pass filter to compensate for the spectral tilt of voiced speech:

```
y[n] = x[n] - α * x[n-1],    α ∈ [0.95, 0.97]
```

Transfer function: `H(z) = 1 - α * z^{-1}`

### 4.2 Framing

Segment the signal into overlapping frames of length N (typically 20-40 ms, e.g., N=1024 at 44.1 kHz) with hop size H (typically 50% overlap).

### 4.3 Windowing

Apply a Hamming window to each frame (see Section 2.3).

### 4.4 FFT

Compute the magnitude spectrum: `|X[k]|` for `k = 0, ..., N/2`.

### 4.5 Mel Filterbank

Apply M triangular filters (typically M=26 or M=40) spaced linearly on the Mel scale. Each filter `H_m[k]` is triangular, centered at Mel frequency `c_m`:

```
H_m[k] = 0                                          if f[k] < f[m-1]
        = (f[k] - f[m-1]) / (f[m] - f[m-1])         if f[m-1] <= f[k] < f[m]
        = (f[m+1] - f[k]) / (f[m+1] - f[m])         if f[m] <= f[k] <= f[m+1]
        = 0                                          if f[k] > f[m+1]
```

where `f[m]` are the center frequencies of the M filters, equally spaced in Mel.

The filterbank energy for filter `m`:

```
S[m] = Σ_{k=0}^{N/2} |X[k]|^2 * H_m[k]
```

### 4.6 Log Compression

```
L[m] = log(S[m])
```

The log compression models the ear's roughly logarithmic loudness perception and turns multiplicative channel effects into additive ones (beneficial for robust speech recognition).

### 4.7 DCT-II (Discrete Cosine Transform, Type II)

```
c[i] = Σ_{m=0}^{M-1} L[m] * cos(πi * (m + 0.5) / M),    i = 0, 1, ..., C-1
```

Typically `C = 13` coefficients are retained (including c[0], which represents overall energy). The DCT decorrelates the log-mel spectrum, concentrating information in the first few coefficients. Higher-order coefficients capture fine spectral detail.

**Delta (velocity) coefficients**:

```
Δc[i, t] = (Σ_{τ=1}^{T} τ * (c[i, t+τ] - c[i, t-τ])) / (2 * Σ_{τ=1}^{T} τ^2)
```

typically with T=2. Delta-delta (acceleration) coefficients are computed by applying the same formula to the delta coefficients.

> Cross-reference: [FEATURES_mfcc_mel.md](FEATURES_mfcc_mel.md).

---

## 5. Autocorrelation

### 5.1 Definition

The (unnormalized) autocorrelation of a discrete signal `x[n]` at lag `τ`:

```
r[τ] = Σ_{n=0}^{N-1-τ} x[n] * x[n + τ]
```

### 5.2 Normalized Autocorrelation

```
r_norm[τ] = r[τ] / r[0]
```

where `r[0] = Σ x[n]^2` is the signal energy. `r_norm[τ] ∈ [-1, 1]`, with `r_norm[0] = 1`.

### 5.3 Wiener-Khinchin Theorem

The autocorrelation function and the power spectral density (PSD) form a Fourier transform pair:

```
R[k] = |X[k]|^2    (power spectrum)
r[τ] = IDFT(R[k])
```

This means autocorrelation can be computed efficiently via FFT:

```
r[τ] = IFFT(|FFT(x)|^2)
```

Complexity: O(N log N) instead of O(N^2) for direct computation.

### 5.4 Applications

**Pitch detection**: For a periodic signal with period T_0 samples, `r[τ]` peaks at `τ = T_0, 2*T_0, 3*T_0, ...`. The fundamental frequency is:

```
f_0 = f_s / T_0    [Hz]
```

Search range for pitch: `τ_min = f_s / f_max` to `τ_max = f_s / f_min`. For speech (80-500 Hz at 44.1 kHz): τ ∈ [88, 551].

**BPM estimation**: Compute the autocorrelation of the onset detection function (see Section 7). Peaks in the autocorrelation correspond to the beat period. Convert lag to BPM: `BPM = 60 * f_s / (τ * H)` where H is the hop size.

> Cross-reference: [FEATURES_pitch_harmonic.md](FEATURES_pitch_harmonic.md), [FEATURES_rhythm_tempo.md](FEATURES_rhythm_tempo.md).

---

## 6. YIN Pitch Detection Algorithm

YIN (de Cheveigne & Kawahara, 2002) is a time-domain fundamental frequency estimator. It improves on autocorrelation-based methods by addressing octave errors and providing sub-sample accuracy.

### 6.1 Difference Function

```
d[τ] = Σ_{n=0}^{W-1} (x[n] - x[n + τ])^2
```

where W is the integration window (typically W = N/2). Expanding the square:

```
d[τ] = r_x[0] + r_{x_τ}[0] - 2 * r_{x,x_τ}[τ]
```

where `r_x[0]` is the energy of the first segment, `r_{x_τ}[0]` is the energy of the shifted segment, and `r_{x,x_τ}[τ]` is the cross-correlation. The difference function equals zero at the true period (for perfectly periodic signals) -- the opposite behavior of autocorrelation.

### 6.2 Cumulative Mean Normalized Difference Function

To eliminate the bias toward `d[0] = 0` and normalize for varying signal energy:

```
d'[τ] = 1                                           if τ = 0
       = d[τ] / ((1/τ) * Σ_{j=1}^{τ} d[j])         if τ > 0
```

This divides `d[τ]` by its running mean. A perfect period produces `d'[τ] = 0` regardless of signal amplitude.

### 6.3 Absolute Threshold Selection

Instead of taking the global minimum of `d'[τ]`, YIN sets an absolute threshold (typically 0.10 to 0.15) and selects the **smallest** `τ` where `d'[τ]` dips below the threshold:

```
τ_est = min{τ : d'[τ] < threshold AND d'[τ] is a local minimum}
```

This preference for the smallest qualifying lag prevents octave errors (selecting τ = 2*T_0 instead of T_0).

If no lag falls below the threshold, the frame is classified as unvoiced (aperiodic).

### 6.4 Parabolic Interpolation

For sub-sample accuracy, fit a parabola through the three points around the selected minimum `(τ-1, d'[τ-1])`, `(τ, d'[τ])`, `(τ+1, d'[τ+1])`:

```
τ_refined = τ + (d'[τ-1] - d'[τ+1]) / (2 * (d'[τ-1] - 2*d'[τ] + d'[τ+1]))
```

The estimated fundamental frequency:

```
f_0 = f_s / τ_refined
```

### 6.5 Reference Implementation (C++)

```cpp
// Compute d'[tau] -- cumulative mean normalized difference
void yin_cmnd(const float* x, int N, float* d_prime, int tau_max) {
    d_prime[0] = 1.0f;
    float running_sum = 0.0f;

    for (int tau = 1; tau < tau_max; ++tau) {
        float sum = 0.0f;
        for (int n = 0; n < N / 2; ++n) {
            float diff = x[n] - x[n + tau];
            sum += diff * diff;
        }
        running_sum += sum;
        d_prime[tau] = sum * tau / running_sum;
    }
}

// Find pitch period with absolute threshold
float yin_pitch(const float* d_prime, int tau_max, float threshold, float fs) {
    for (int tau = 2; tau < tau_max; ++tau) {
        if (d_prime[tau] < threshold) {
            // Find local minimum
            while (tau + 1 < tau_max && d_prime[tau + 1] < d_prime[tau]) {
                ++tau;
            }
            // Parabolic interpolation
            float s0 = d_prime[tau - 1];
            float s1 = d_prime[tau];
            float s2 = d_prime[tau + 1];
            float refined = tau + (s0 - s2) / (2.0f * (s0 - 2.0f * s1 + s2));
            return fs / refined;
        }
    }
    return 0.0f;  // Unvoiced
}
```

> Cross-reference: [FEATURES_pitch_harmonic.md](FEATURES_pitch_harmonic.md).

---

## 7. Onset Detection Functions

Onset detection identifies the start of musical events (note attacks, drum hits, transients). All functions operate on successive STFT frames.

### 7.1 Spectral Flux

Measures the increase in spectral energy from frame to frame:

```
SF(t) = Σ_{k=0}^{N/2} max(0, |X_t[k]|^2 - |X_{t-1}[k]|^2)
```

The half-wave rectification (`max(0, ...)`) ensures only energy *increases* are detected, suppressing offsets and decays. This is also called **rectified spectral flux**.

Without rectification (unrectified):

```
SF_unrect(t) = Σ_{k=0}^{N/2} (|X_t[k]|^2 - |X_{t-1}[k]|^2)^2
```

### 7.2 High Frequency Content (HFC)

Weights spectral bins by their frequency index, emphasizing transient energy (which tends to be broadband, including high frequencies):

```
HFC(t) = Σ_{k=0}^{N/2} k * |X_t[k]|^2
```

Variants use `k^2` weighting for even stronger high-frequency emphasis.

### 7.3 Complex Domain Onset Detection

Uses both magnitude and phase information. Predict the next frame's complex spectrum from the previous two:

```
X_predicted[k] = |X_{t-1}[k]| * e^{j * (2 * φ_{t-1}[k] - φ_{t-2}[k])}
```

where `φ_t[k] = ∠X_t[k]` is the phase. The onset function measures the deviation from prediction:

```
CD(t) = Σ_{k=0}^{N/2} |X_t[k] - X_predicted[k]|
```

This detects onsets even when they occur without an energy increase (e.g., pitch changes).

### 7.4 Post-Processing

Raw onset functions are typically:
1. **Normalized**: subtract mean, divide by standard deviation over a sliding window.
2. **Peak-picked**: apply a threshold (e.g., mean + λ * std, λ ~ 0.5-1.5) and find local maxima above it.
3. **Backtracked**: the peak in the ODF may lag the true onset; search backward to find the first rise above the noise floor.

> Cross-reference: [FEATURES_transients_texture.md](FEATURES_transients_texture.md), [FEATURES_rhythm_tempo.md](FEATURES_rhythm_tempo.md).

---

## 8. Spectral Feature Formulas

All formulas below use the magnitude spectrum `|X[k]|` (or power spectrum `|X[k]|^2`) and the center frequency `f_k = k * f_s / N` for bin `k`. The summation range is `k = 0` to `K = N/2`.

### 8.1 Spectral Centroid (First Spectral Moment)

The "center of mass" of the spectrum:

```
μ_1 = Σ_{k=0}^{K} (f_k * |X[k]|) / Σ_{k=0}^{K} |X[k]|
```

Correlates with perceived brightness. Units: Hz.

### 8.2 Spectral Spread (Second Central Moment)

The bandwidth around the centroid:

```
μ_2 = sqrt( Σ_{k=0}^{K} (f_k - μ_1)^2 * |X[k]| / Σ_{k=0}^{K} |X[k]| )
```

A narrow spread indicates a tonal signal; a wide spread indicates noise or polyphonic content. Units: Hz.

### 8.3 Spectral Skewness (Third Central Moment, Normalized)

Asymmetry of the spectral distribution:

```
μ_3 = (Σ_{k=0}^{K} (f_k - μ_1)^3 * |X[k]|) / (μ_2^3 * Σ_{k=0}^{K} |X[k]|)
```

Positive skewness: energy concentrated below the centroid. Negative: above.

### 8.4 Spectral Kurtosis (Fourth Central Moment, Normalized)

Peakedness of the spectral distribution:

```
μ_4 = (Σ_{k=0}^{K} (f_k - μ_1)^4 * |X[k]|) / (μ_2^4 * Σ_{k=0}^{K} |X[k]|) - 3
```

The `-3` makes it excess kurtosis (zero for a Gaussian distribution). High kurtosis indicates sharp spectral peaks; low kurtosis indicates a flat spectrum.

### 8.5 Spectral Rolloff

The frequency below which a specified percentage (typically 85% or 95%) of the total spectral energy is contained:

```
Σ_{k=0}^{k_rolloff} |X[k]|^2 = α * Σ_{k=0}^{K} |X[k]|^2
```

Solve for `k_rolloff`; then `f_rolloff = k_rolloff * f_s / N`. Common thresholds: α = 0.85 or α = 0.95.

### 8.6 Spectral Flatness (Wiener Entropy)

Ratio of the geometric mean to the arithmetic mean of the power spectrum:

```
Flatness = (∏_{k=0}^{K} |X[k]|^2)^{1/(K+1)} / ((1/(K+1)) * Σ_{k=0}^{K} |X[k]|^2)
```

In decibels:

```
Flatness_dB = 10 * log10(Flatness)
```

In practice, use log-domain computation to avoid numerical overflow/underflow:

```
Flatness_dB = (10/(K+1)) * Σ log10(|X[k]|^2) - 10 * log10((1/(K+1)) * Σ |X[k]|^2)
```

Values near 1 (0 dB) indicate noise (flat spectrum). Values near 0 (-inf dB) indicate tonal content.

### 8.7 Spectral Entropy

Treating the normalized power spectrum as a probability distribution:

```
p[k] = |X[k]|^2 / Σ_{k=0}^{K} |X[k]|^2

H = -Σ_{k=0}^{K} p[k] * log_2(p[k])
```

Maximum entropy = `log_2(K+1)` (flat/noise spectrum). Minimum = 0 (single tone). Often normalized:

```
H_norm = H / log_2(K+1)    ∈ [0, 1]
```

### 8.8 Spectral Flux

(Repeated from Section 7.1 for completeness in the spectral features context.)

```
Flux(t) = Σ_{k=0}^{K} (|X_t[k]| - |X_{t-1}[k]|)^2
```

or the L1 variant:

```
Flux_L1(t) = Σ_{k=0}^{K} | |X_t[k]| - |X_{t-1}[k]| |
```

### 8.9 Spectral Slope

Linear regression of the magnitude spectrum as a function of frequency:

```
Slope = (K * Σ f_k * |X[k]| - Σ f_k * Σ |X[k]|) / (K * Σ f_k^2 - (Σ f_k)^2)
```

Negative slope indicates energy concentration at low frequencies (typical of speech, bass-heavy music).

### 8.10 Spectral Decrease

Emphasizes changes in the lower part of the spectrum (perceptually relevant):

```
Decrease = Σ_{k=1}^{K} (|X[k]| - |X[0]|) / k  /  Σ_{k=1}^{K} |X[k]|
```

> Cross-reference: [FEATURES_spectral.md](FEATURES_spectral.md).

---

## 9. Filter Design

### 9.1 IIR Biquad Transfer Function

The second-order IIR (biquad) filter:

```
H(z) = (b_0 + b_1*z^{-1} + b_2*z^{-2}) / (1 + a_1*z^{-1} + a_2*z^{-2})
```

Difference equation (Direct Form I):

```
y[n] = b_0*x[n] + b_1*x[n-1] + b_2*x[n-2] - a_1*y[n-1] - a_2*y[n-2]
```

Note: many references include `a_0` in the denominator and normalize all coefficients by `a_0`. The convention above assumes `a_0 = 1`.

**Transposed Direct Form II** (preferred for floating-point stability):

```
y[n] = b_0*x[n] + s_1
s_1   = b_1*x[n] - a_1*y[n] + s_2
s_2   = b_2*x[n] - a_2*y[n]
```

Only 2 state variables instead of 4.

### 9.2 Biquad Coefficient Formulas (Audio EQ Cookbook)

Common intermediate variables:

```
ω_0 = 2π * f_c / f_s        (center/cutoff angular frequency)
α = sin(ω_0) / (2 * Q)      (bandwidth parameter)
A = 10^(dB_gain / 40)       (for peaking/shelving filters)
```

**Lowpass (LPF)**:

```
b_0 = (1 - cos(ω_0)) / 2
b_1 = 1 - cos(ω_0)
b_2 = (1 - cos(ω_0)) / 2
a_0 = 1 + α
a_1 = -2 * cos(ω_0)
a_2 = 1 - α
```

Normalize: divide all by `a_0`.

**Highpass (HPF)**:

```
b_0 = (1 + cos(ω_0)) / 2
b_1 = -(1 + cos(ω_0))
b_2 = (1 + cos(ω_0)) / 2
a_0 = 1 + α
a_1 = -2 * cos(ω_0)
a_2 = 1 - α
```

**Bandpass (BPF, constant skirt gain)**:

```
b_0 = α
b_1 = 0
b_2 = -α
a_0 = 1 + α
a_1 = -2 * cos(ω_0)
a_2 = 1 - α
```

**Notch (Band-reject)**:

```
b_0 = 1
b_1 = -2 * cos(ω_0)
b_2 = 1
a_0 = 1 + α
a_1 = -2 * cos(ω_0)
a_2 = 1 - α
```

**Peaking EQ**:

```
b_0 = 1 + α * A
b_1 = -2 * cos(ω_0)
b_2 = 1 - α * A
a_0 = 1 + α / A
a_1 = -2 * cos(ω_0)
a_2 = 1 - α / A
```

**Low Shelf**:

```
b_0 = A * ((A+1) - (A-1)*cos(ω_0) + 2*sqrt(A)*α)
b_1 = 2*A * ((A-1) - (A+1)*cos(ω_0))
b_2 = A * ((A+1) - (A-1)*cos(ω_0) - 2*sqrt(A)*α)
a_0 = (A+1) + (A-1)*cos(ω_0) + 2*sqrt(A)*α
a_1 = -2 * ((A-1) + (A+1)*cos(ω_0))
a_2 = (A+1) + (A-1)*cos(ω_0) - 2*sqrt(A)*α
```

**High Shelf**:

```
b_0 = A * ((A+1) + (A-1)*cos(ω_0) + 2*sqrt(A)*α)
b_1 = -2*A * ((A-1) + (A+1)*cos(ω_0))
b_2 = A * ((A+1) + (A-1)*cos(ω_0) - 2*sqrt(A)*α)
a_0 = (A+1) - (A-1)*cos(ω_0) + 2*sqrt(A)*α
a_1 = 2 * ((A-1) - (A+1)*cos(ω_0))
a_2 = (A+1) - (A-1)*cos(ω_0) - 2*sqrt(A)*α
```

### 9.3 Bilinear Transform

Maps the s-plane (continuous) to the z-plane (discrete):

```
s = (2 * f_s) * (z - 1) / (z + 1)
```

or equivalently, substituting `z = e^{jω}`:

```
s = (2 * f_s) * (1 - z^{-1}) / (1 + z^{-1})
```

**Frequency warping**: The bilinear transform introduces a nonlinear mapping between analog frequency `Ω` and digital frequency `ω`:

```
Ω = (2 * f_s) * tan(ω / 2)
```

To design a filter with cutoff at digital frequency `ω_c`, pre-warp to get the analog prototype cutoff:

```
Ω_c = (2 * f_s) * tan(ω_c / 2) = (2 * f_s) * tan(π * f_c / f_s)
```

### 9.4 Butterworth Filter Design

An Nth-order Butterworth lowpass filter has maximally flat magnitude response. The analog prototype has poles at:

```
s_k = Ω_c * e^{j * π * (2k + N + 1) / (2N)},    k = 0, 1, ..., N-1
```

(select only the left-half-plane poles). The transfer function:

```
H(s) = Ω_c^N / ∏_{k} (s - s_k)
```

For digital implementation, apply the bilinear transform with pre-warping, then cascade into second-order sections (biquads) for numerical stability.

The magnitude response:

```
|H(jΩ)|^2 = 1 / (1 + (Ω/Ω_c)^{2N})
```

At the cutoff: `|H(jΩ_c)| = 1/sqrt(2)` (-3 dB). The rolloff beyond the cutoff is `20*N` dB/decade.

### 9.5 A-Weighting Filter

A-weighting approximates the frequency response of human hearing at moderate levels (per IEC 61672).

The analog transfer function:

```
H_A(s) = K_A * s^4 / ((s + 2π*20.6)^2 * (s + 2π*107.7) * (s + 2π*737.9) * (s + 2π*12194)^2)
```

where `K_A` is chosen so that `|H_A(j*2π*1000)| = 1` (0 dB at 1 kHz).

**Digital IIR coefficients at 48 kHz** (cascade of 3 biquad sections):

Section 1 (highpass, 20.6 Hz pole pair):
```
b = [1.0, -2.0, 1.0]
a = [1.0, -1.99004745, 0.99007225]
```

Section 2 (highpass + mid-frequency shaping):
```
b = [1.0, -2.0, 1.0]
a = [1.0, -1.96977856, 0.97022206]
```

Section 3 (lowpass, 12194 Hz pole pair):
```
b = [1.0, 2.0, 1.0]
a = [1.0, -0.28209479, 0.15034686]
```

Overall gain adjusted so response = 0 dB at 1 kHz. Exact coefficients vary with sample rate; recompute via bilinear transform with pre-warping for other rates.

### 9.6 K-Weighting (ITU-R BS.1770, for Loudness Measurement)

K-weighting consists of two cascaded filters:

**Stage 1: Pre-filter (shelving boost, +4 dB above ~1.5 kHz)**

At 48 kHz:
```
b = [1.53512485958697, -2.69169618940638, 1.19839281085285]
a = [1.0, -1.69065929318241, 0.73248077421585]
```

**Stage 2: Revised Low-frequency B-curve (RLB, highpass ~38 Hz)**

At 48 kHz:
```
b = [1.0, -2.0, 1.0]
a = [1.0, -1.99004745483398, 0.99007225036621]
```

After K-weighting, loudness is computed as mean square over a gating window (see ITU-R BS.1770-4 for gating thresholds at -70 LKFS and -10 LKFS relative).

> Cross-reference: [FEATURES_frequency_bands.md](FEATURES_frequency_bands.md), [FEATURES_amplitude_dynamics.md](FEATURES_amplitude_dynamics.md).

---

## 10. Complex Number Operations in Audio DSP

### 10.1 Representation

An FFT bin `X[k]` is a complex number:

```
X[k] = re + j*im
```

### 10.2 Magnitude (Amplitude Spectrum)

```
|X[k]| = sqrt(re^2 + im^2)
```

For power spectrum: `|X[k]|^2 = re^2 + im^2` (avoids the sqrt).

To convert to dB:

```
L[k] = 20 * log10(|X[k]| / X_ref)       (amplitude)
L[k] = 10 * log10(|X[k]|^2 / X_ref^2)   (power, equivalent)
```

### 10.3 Phase

```
φ[k] = atan2(im, re)    ∈ (-π, π]
```

Note: use `atan2`, not `atan(im/re)`, to get the correct quadrant.

### 10.4 Phase Unwrapping

Raw phase values jump by 2π at the wrapping boundary. Unwrapped phase:

```
φ_unwrapped[k] = φ_unwrapped[k-1] + principal_value(φ[k] - φ[k-1])
```

where `principal_value(Δ) = Δ - 2π * round(Δ / (2π))`.

### 10.5 Instantaneous Frequency

From the time derivative of unwrapped phase across successive STFT frames:

```
f_inst[k, t] = f_k + (φ_unwrapped[k, t] - φ_unwrapped[k, t-1]) / (2π * H / f_s)
```

where `H` is the hop size. This gives a more accurate frequency estimate than the bin center frequency alone -- the basis for phase vocoder pitch shifting and time stretching.

Simplified computation using phase deviation from expected phase advance:

```
Δφ_expected = 2π * k * H / N
Δφ_actual = φ[k, t] - φ[k, t-1]
Δφ_deviation = principal_value(Δφ_actual - Δφ_expected)
f_inst[k] = (k + Δφ_deviation * N / (2π * H)) * f_s / N
```

---

## 11. Convolution Theorem

### 11.1 Statement

If `x[n]` and `h[n]` are two sequences with DFTs `X[k]` and `H[k]`:

**Time-domain convolution corresponds to frequency-domain multiplication**:

```
DFT{x[n] * h[n]} = X[k] · H[k]     (element-wise product)
```

where `*` denotes circular convolution:

```
(x * h)[n] = Σ_{m=0}^{N-1} x[m] * h[(n-m) mod N]
```

**Conversely, time-domain multiplication corresponds to frequency-domain convolution**:

```
DFT{x[n] · h[n]} = (1/N) * (X * H)[k]
```

### 11.2 Fast Convolution (Overlap-Add / Overlap-Save)

For convolving a long signal `x` (length L) with a short filter `h` (length M):

1. Zero-pad `h` to length `N >= L_block + M - 1` (choose N as a power of 2 for FFT efficiency).
2. Pre-compute `H = FFT(h_padded)`.
3. Partition `x` into blocks of length `L_block = N - M + 1`.
4. For each block: compute `Y = FFT(x_block) · H`, then `y = IFFT(Y)`.
5. Overlap-add the output blocks.

Complexity: O((L/L_block) * N * log N) vs O(L * M) for direct convolution. Efficient when `M > ~64`.

### 11.3 Application: Efficient FIR Filtering

Any FIR filter of length M can be applied via:

```
y = IFFT(FFT(x) · FFT(h))
```

This is the basis for convolution reverb (impulse responses of length 10^5 to 10^6 samples).

---

## 12. Useful Identities and Properties

### 12.1 Parseval's Theorem (Energy Preservation)

The total energy computed in time domain equals the total energy computed in frequency domain:

```
Σ_{n=0}^{N-1} |x[n]|^2 = (1/N) * Σ_{k=0}^{N-1} |X[k]|^2
```

This means the FFT preserves energy (up to the 1/N scaling factor). For the unitary DFT (1/sqrt(N) normalization), the factor disappears.

### 12.2 Linearity

```
DFT{a * x[n] + b * y[n]} = a * X[k] + b * Y[k]
```

### 12.3 Time-Shift Property

A delay of `d` samples in time corresponds to a linear phase shift in frequency:

```
DFT{x[n - d]} = X[k] * e^{-j2πkd/N}
```

The magnitude spectrum is unchanged; only the phase changes.

### 12.4 Modulation Property (Frequency-Shift)

Multiplying by a complex exponential in time shifts the spectrum:

```
DFT{x[n] * e^{j2πn*m/N}} = X[k - m]    (circular shift)
```

This is the basis for frequency shifting and single-sideband modulation.

### 12.5 Conjugate Symmetry (Real Signals)

For real-valued `x[n]`:

```
X[N - k] = X[k]*    (complex conjugate)
```

Consequences:
- `|X[N-k]| = |X[k]|` -- magnitude spectrum is symmetric
- `∠X[N-k] = -∠X[k]` -- phase spectrum is anti-symmetric
- Only bins `k = 0` to `k = N/2` need to be computed/stored (N/2 + 1 unique complex values)

### 12.6 Circular vs. Linear Convolution

Circular convolution of length-N sequences produces a length-N result. To get linear (aperiodic) convolution of sequences of length L and M, zero-pad both to length `N >= L + M - 1` before applying the DFT.

### 12.7 dB Conversion Constants

```
Power to dB:     L = 10 * log10(P / P_ref)
Amplitude to dB: L = 20 * log10(A / A_ref)

dB to power:     P = P_ref * 10^(L/10)
dB to amplitude: A = A_ref * 10^(L/20)
```

Common references:
- `A_ref = 1.0` for normalized signals (dBFS -- decibels relative to full scale)
- `P_ref = 20 μPa` for sound pressure level (dB SPL)
- `P_ref = 1 mW` for electrical power (dBm)

### 12.8 RMS (Root Mean Square)

```
x_rms = sqrt((1/N) * Σ_{n=0}^{N-1} x[n]^2)
```

RMS in dBFS:

```
L_rms = 20 * log10(x_rms)
```

For a full-scale sine wave (amplitude = 1.0): `x_rms = 1/sqrt(2) ≈ 0.707`, so `L_rms ≈ -3.01 dBFS`.

### 12.9 Zero-Padding and Interpolation

Zero-padding a time-domain signal from N to M > N points before the FFT does NOT increase frequency resolution (still Δf = f_s / N), but it interpolates the magnitude spectrum, producing a smoother-looking result with M/N times as many bins. Useful for peak frequency estimation (combined with parabolic interpolation).

---

## Appendix A: Common Constants

| Constant | Value | Context |
|----------|-------|---------|
| ln(2) | 0.693147 | Octave calculations |
| ln(10) | 2.302585 | dB conversions |
| log10(2) | 0.301030 | Octave-to-decade conversion |
| 20/ln(10) | 8.685890 | Amplitude ratio to dB (natural log shortcut) |
| 2^{1/12} | 1.059463 | Semitone frequency ratio (equal temperament) |
| 2^{1/12} - 1 | 0.059463 | ~5.95% frequency increase per semitone |
| A4 | 440 Hz | Standard tuning reference |
| MIDI note to Hz | f = 440 * 2^{(n-69)/12} | n = MIDI note number |
| Hz to MIDI | n = 69 + 12 * log2(f/440) | |

## Appendix B: Sample Rate / Frame Size Quick Reference

| Sample Rate | N=256 (ms/Δf) | N=512 | N=1024 | N=2048 | N=4096 |
|-------------|---------------|-------|--------|--------|--------|
| 22050 Hz    | 11.6/86.1     | 23.2/43.1 | 46.4/21.5 | 92.9/10.8 | 185.8/5.4 |
| 44100 Hz    | 5.8/172.3     | 11.6/86.1 | 23.2/43.1 | 46.4/21.5 | 92.9/10.8 |
| 48000 Hz    | 5.3/187.5     | 10.7/93.8 | 21.3/46.9 | 42.7/23.4 | 85.3/11.7 |
| 96000 Hz    | 2.7/375.0     | 5.3/187.5 | 10.7/93.8 | 21.3/46.9 | 42.7/23.4 |

Format: frame duration (ms) / frequency resolution (Hz).

## Appendix C: Biquad Filter Implementation (C++)

```cpp
struct Biquad {
    float b0, b1, b2, a1, a2;  // Coefficients (a0 normalized to 1)
    float s1 = 0, s2 = 0;       // State (Transposed Direct Form II)

    float process(float x) {
        float y = b0 * x + s1;
        s1 = b1 * x - a1 * y + s2;
        s2 = b2 * x - a2 * y;
        return y;
    }

    void reset() { s1 = s2 = 0; }

    // Design: lowpass
    static Biquad lowpass(float fc, float Q, float fs) {
        float w0 = 2.0f * M_PI * fc / fs;
        float alpha = sinf(w0) / (2.0f * Q);
        float cos_w0 = cosf(w0);
        float a0 = 1.0f + alpha;
        return {
            (1.0f - cos_w0) / 2.0f / a0,
            (1.0f - cos_w0) / a0,
            (1.0f - cos_w0) / 2.0f / a0,
            -2.0f * cos_w0 / a0,
            (1.0f - alpha) / a0
        };
    }

    // Design: highpass
    static Biquad highpass(float fc, float Q, float fs) {
        float w0 = 2.0f * M_PI * fc / fs;
        float alpha = sinf(w0) / (2.0f * Q);
        float cos_w0 = cosf(w0);
        float a0 = 1.0f + alpha;
        return {
            (1.0f + cos_w0) / 2.0f / a0,
            -(1.0f + cos_w0) / a0,
            (1.0f + cos_w0) / 2.0f / a0,
            -2.0f * cos_w0 / a0,
            (1.0f - alpha) / a0
        };
    }

    // Design: bandpass (constant skirt gain)
    static Biquad bandpass(float fc, float Q, float fs) {
        float w0 = 2.0f * M_PI * fc / fs;
        float alpha = sinf(w0) / (2.0f * Q);
        float cos_w0 = cosf(w0);
        float a0 = 1.0f + alpha;
        return {
            alpha / a0, 0.0f, -alpha / a0,
            -2.0f * cos_w0 / a0, (1.0f - alpha) / a0
        };
    }

    // Design: peaking EQ
    static Biquad peaking(float fc, float Q, float dBgain, float fs) {
        float A = powf(10.0f, dBgain / 40.0f);
        float w0 = 2.0f * M_PI * fc / fs;
        float alpha = sinf(w0) / (2.0f * Q);
        float cos_w0 = cosf(w0);
        float a0 = 1.0f + alpha / A;
        return {
            (1.0f + alpha * A) / a0,
            -2.0f * cos_w0 / a0,
            (1.0f - alpha * A) / a0,
            -2.0f * cos_w0 / a0,
            (1.0f - alpha / A) / a0
        };
    }
};
```

---

## Cross-References

| Document | Relevant sections here |
|----------|----------------------|
| [FEATURES_spectral.md](FEATURES_spectral.md) | Sections 2, 8 (window functions, spectral features) |
| [FEATURES_frequency_bands.md](FEATURES_frequency_bands.md) | Sections 1, 3, 9 (DFT, perceptual scales, filters) |
| [FEATURES_mfcc_mel.md](FEATURES_mfcc_mel.md) | Sections 3, 4 (Mel scale, MFCC derivation) |
| [FEATURES_rhythm_tempo.md](FEATURES_rhythm_tempo.md) | Sections 5, 7 (autocorrelation for BPM, onset detection) |
| [FEATURES_pitch_harmonic.md](FEATURES_pitch_harmonic.md) | Sections 5, 6 (autocorrelation, YIN algorithm) |
| [LIB_fft_comparison.md](LIB_fft_comparison.md) | Section 1 (FFT algorithms and libraries) |

---

*Document version: 1.0. All formulas verified against standard references: Oppenheim & Schafer (Discrete-Time Signal Processing, 3rd ed.), Smith (Mathematics of the DFT), Muller (Fundamentals of Music Processing), Audio EQ Cookbook (Robert Bristow-Johnson).*
