# Frequency Band Decomposition for Real-Time Audio Analysis

> **Scope**: Strategies for decomposing audio spectra into perceptually and musically meaningful frequency bands for music visualization and VJ engine applications.
>
> **Cross-references**: [FEATURES_spectral.md](FEATURES_spectral.md) | [FEATURES_mfcc_mel.md](FEATURES_mfcc_mel.md) | [FEATURES_psychoacoustic.md](FEATURES_psychoacoustic.md) | [LIB_fft_comparison.md](LIB_fft_comparison.md) | [REF_math_reference.md](REF_math_reference.md)

---

## 1. Standard Frequency Bands for Music Visualization

### 1.1 The Seven-Band Model

The following band definitions are widely used in audio visualization, graphic EQ displays, and VJ engines. They reflect a compromise between musical content boundaries and perceptual grouping.

| Band         | Range (Hz)   | Musical Content                                                                 |
|--------------|--------------|---------------------------------------------------------------------------------|
| **Sub-bass** | 20 -- 60     | Kick drum fundamental, sub-bass synths (808s), organ pedal tones, rumble        |
| **Bass**     | 60 -- 250    | Bass guitar fundamental (41--250 Hz), kick drum body, cello low register, tuba  |
| **Low-mid**  | 250 -- 500   | Vocal warmth/body, guitar body resonance, snare fundamental, piano low-mid      |
| **Mid**      | 500 -- 2000  | Vocal intelligibility, guitar mid-range, horn section, piano middle register    |
| **High-mid** | 2000 -- 4000 | Vocal presence/consonants, snare crack, guitar bite, trumpet brightness         |
| **Presence** | 4000 -- 6000 | Vocal sibilance onset, cymbal body, string articulation, definition region      |
| **Brilliance/Air** | 6000 -- 20000 | Cymbal shimmer, hi-hat sizzle, air/breathiness, harmonic overtones, reverb tail |

### 1.2 Mapping FFT Bins to Bands

Given an FFT of size `N` and sample rate `fs`, bin `k` corresponds to frequency:

```
f(k) = k * fs / N
```

For a given band `[f_low, f_high]`, the contributing bins are:

```
k_low  = ceil(f_low * N / fs)
k_high = floor(f_high * N / fs)
```

The band energy (power) is the sum of squared magnitudes across those bins:

```
E_band = sum_{k=k_low}^{k_high} |X[k]|^2
```

For amplitude (RMS-like), take `sqrt(E_band / num_bins)` or simply `sqrt(E_band)` depending on whether you want density or total energy.

**Critical detail**: At low frequencies with typical FFT sizes, bin resolution is coarse. With `N = 2048` at `fs = 44100`:

| Band       | f_low | f_high | k_low | k_high | Bin count |
|------------|-------|--------|-------|--------|-----------|
| Sub-bass   | 20    | 60     | 1     | 2      | 2         |
| Bass       | 60    | 250    | 3     | 11     | 9         |
| Low-mid    | 250   | 500    | 12    | 23     | 12        |
| Mid        | 500   | 2000   | 24    | 92     | 69        |
| High-mid   | 2000  | 4000   | 93    | 185    | 93        |
| Presence   | 4000  | 6000   | 186   | 278    | 93        |
| Brilliance | 6000  | 20000  | 279   | 928    | 650       |

The sub-bass band has only 2 bins -- extremely poor frequency resolution. This is a fundamental limitation of the STFT at standard window sizes. Mitigation strategies:

1. **Increase FFT size**: `N = 8192` gives ~5.4 Hz resolution, yielding ~7 bins in sub-bass. Cost: 4x latency (186ms at 44.1kHz).
2. **Use separate long-window analysis** for low bands only.
3. **Use IIR filterbank** for low-frequency bands (see Section 5).
4. **Zero-pad** to interpolate spectral peaks (does not improve true resolution, but helps peak detection).

### 1.3 Energy vs. Amplitude vs. dB

For visualization, the choice of scale matters enormously:

- **Linear amplitude** `|X[k]|`: dominated by low frequencies, high-frequency content nearly invisible.
- **Power** `|X[k]|^2`: even more dominated by lows.
- **dB scale** `20 * log10(|X[k]| / ref)`: perceptually closer to loudness, reveals quiet high-frequency content.
- **Sqrt of power** (RMS): good balance for visualization.

For VJ engines, dB or log-scale display with a floor (e.g., -60dB) is almost always the right choice.

---

## 2. Perceptual Frequency Scales

Human frequency perception is approximately logarithmic. Several psychoacoustically motivated scales model this nonlinearity.

### 2.1 Bark Scale

The Bark scale divides the audible range into 24 **critical bands**, each approximately one critical bandwidth wide. Named after Heinrich Barkhausen.

**Frequency-to-Bark conversion** (Traunmuller, 1990):

```
z(f) = 26.81 * f / (1960 + f) - 0.53
```

With corrections:
- If `z < 2`: `z = z + 0.15 * (2 - z)`
- If `z > 20.1`: `z = z + 0.22 * (z - 20.1)`

**Inverse (Bark-to-frequency)**:

```
f(z) = 1960 * (z + 0.53) / (26.28 - z)
```

**Critical Band Table (Zwicker)**:

| Bark | Center (Hz) | Lower (Hz) | Upper (Hz) | Bandwidth (Hz) |
|------|-------------|-------------|-------------|-----------------|
| 1    | 50          | 20          | 100         | 80              |
| 2    | 150         | 100         | 200         | 100             |
| 3    | 250         | 200         | 300         | 100             |
| 4    | 350         | 300         | 400         | 100             |
| 5    | 450         | 400         | 510         | 110             |
| 6    | 570         | 510         | 630         | 120             |
| 7    | 700         | 630         | 770         | 140             |
| 8    | 840         | 770         | 920         | 150             |
| 9    | 1000        | 920         | 1080        | 160             |
| 10   | 1170        | 1080        | 1270        | 190             |
| 11   | 1370        | 1270        | 1480        | 210             |
| 12   | 1600        | 1480        | 1720        | 240             |
| 13   | 1850        | 1720        | 2000        | 280             |
| 14   | 2150        | 2000        | 2320        | 320             |
| 15   | 2500        | 2320        | 2700        | 380             |
| 16   | 2900        | 2700        | 3150        | 450             |
| 17   | 3400        | 3150        | 3700        | 550             |
| 18   | 4000        | 3700        | 4400        | 700             |
| 19   | 4800        | 4400        | 5300        | 900             |
| 20   | 5800        | 5300        | 6400        | 1100            |
| 21   | 7000        | 6400        | 7700        | 1300            |
| 22   | 8500        | 7700        | 9500        | 1800            |
| 23   | 10500       | 9500        | 12000       | 2500            |
| 24   | 13500       | 12000       | 15500       | 3500            |

**Implementation** -- mapping FFT bins to Bark bands:

```cpp
// Compute Bark band index for a given frequency
int barkBand(float freqHz) {
    float z = 26.81f * freqHz / (1960.0f + freqHz) - 0.53f;
    if (z < 0.0f) z = 0.0f;
    return static_cast<int>(std::floor(z));  // 0-indexed band
}

// Build lookup table: FFT bin -> Bark band index
void buildBarkMap(int fftSize, float sampleRate, std::vector<int>& binToBark) {
    int numBins = fftSize / 2 + 1;
    binToBark.resize(numBins);
    for (int k = 0; k < numBins; ++k) {
        float freq = static_cast<float>(k) * sampleRate / static_cast<float>(fftSize);
        binToBark[k] = std::clamp(barkBand(freq), 0, 23);
    }
}
```

### 2.2 Mel Scale

The Mel scale is the most widely used perceptual scale in audio ML (MFCCs, mel-spectrograms). It approximates human pitch perception.

**Frequency-to-Mel** (O'Shaughnessy, 1987):

```
m = 2595 * log10(1 + f / 700)
```

**Mel-to-Frequency (inverse)**:

```
f = 700 * (10^(m / 2595) - 1)
```

**Triangular Filterbank Construction**:

Given `numFilters` mel filters spanning `[f_min, f_max]`:

1. Convert `f_min` and `f_max` to mel scale.
2. Create `numFilters + 2` linearly spaced points in mel domain.
3. Convert back to Hz -- these are the filter center/edge frequencies.
4. Map each center frequency to the nearest FFT bin.
5. Construct triangular filters: each filter `m` has edges at points `m`, `m+1`, `m+2`.

```cpp
std::vector<std::vector<float>> buildMelFilterbank(
    int numFilters, int fftSize, float sampleRate,
    float fMin = 20.0f, float fMax = 0.0f)
{
    if (fMax <= 0.0f) fMax = sampleRate / 2.0f;

    int numBins = fftSize / 2 + 1;

    // Hz to Mel
    auto hzToMel = [](float f) { return 2595.0f * std::log10(1.0f + f / 700.0f); };
    auto melToHz = [](float m) { return 700.0f * (std::pow(10.0f, m / 2595.0f) - 1.0f); };

    float melMin = hzToMel(fMin);
    float melMax = hzToMel(fMax);

    // numFilters + 2 points (edges + centers)
    std::vector<float> melPoints(numFilters + 2);
    for (int i = 0; i < numFilters + 2; ++i) {
        melPoints[i] = melMin + i * (melMax - melMin) / (numFilters + 1);
    }

    // Convert to FFT bin indices
    std::vector<int> binIndices(numFilters + 2);
    for (int i = 0; i < numFilters + 2; ++i) {
        float freq = melToHz(melPoints[i]);
        binIndices[i] = static_cast<int>(std::floor(freq * fftSize / sampleRate + 0.5f));
        binIndices[i] = std::clamp(binIndices[i], 0, numBins - 1);
    }

    // Build triangular filters
    std::vector<std::vector<float>> filterbank(numFilters, std::vector<float>(numBins, 0.0f));
    for (int m = 0; m < numFilters; ++m) {
        int left   = binIndices[m];
        int center = binIndices[m + 1];
        int right  = binIndices[m + 2];

        // Rising slope
        for (int k = left; k <= center; ++k) {
            if (center != left)
                filterbank[m][k] = static_cast<float>(k - left) / (center - left);
        }
        // Falling slope
        for (int k = center; k <= right; ++k) {
            if (right != center)
                filterbank[m][k] = static_cast<float>(right - k) / (right - center);
        }
    }
    return filterbank;
}
```

**Area normalization**: For energy-preserving filterbanks, normalize each filter by the sum of its coefficients (or by `2.0 / (f_right - f_left)` for unit-area triangles). For visualization, unnormalized or peak-normalized filters often look better since they prevent low-frequency bands from dominating.

### 2.3 ERB Scale (Equivalent Rectangular Bandwidth)

The ERB scale (Glasberg & Moore, 1990) models the bandwidth of auditory filters more accurately than Bark at low frequencies.

**ERB of a filter centered at frequency f**:

```
ERB(f) = 24.7 * (4.37 * f / 1000 + 1)
```

This simplifies to: `ERB(f) = 24.7 + 0.10798 * f`

**ERB-rate scale** (number of ERBs below frequency f):

```
ERB_rate(f) = 21.4 * log10(1 + 0.00437 * f)
```

**Inverse**:

```
f = (10^(ERB_rate / 21.4) - 1) / 0.00437
```

The ERB-rate scale produces approximately 38--40 bands across 20--20000 Hz (compared to Bark's 24). This finer resolution at low frequencies is advantageous for bass-heavy music visualization.

### 2.4 Scale Comparison

| Property              | Bark                  | Mel                   | ERB                      |
|-----------------------|-----------------------|-----------------------|--------------------------|
| Bands (20-20kHz)      | 24                    | Configurable          | ~39                      |
| Low-freq resolution   | Moderate              | Low                   | High                     |
| Origin                | Psychoacoustics       | Pitch perception      | Auditory filter modeling |
| Common use            | Psychoacoustic models | ML / speech / MFCCs   | Auditory scene analysis  |
| Formula complexity    | Moderate              | Simple                | Simple                   |
| Best for viz          | Moderate band count   | ML feature extraction | Fine low-freq detail     |
| Musical alignment     | Poor (not octave-based) | Poor                | Poor                     |

**Recommendation for VJ engines**: For a standard 24-band analyzer, Bark bands are natural. For ML-driven features, use Mel. For perceptually accurate loudness analysis, use ERB. For musically meaningful analysis, use octave bands (Section 7) or CQT (Section 3).

---

## 3. Constant-Q Transform (CQT)

### 3.1 Theory

The CQT produces a frequency representation where bins are logarithmically spaced, with a constant ratio of center frequency to bandwidth:

```
Q = f_k / delta_f_k = constant
```

For musical analysis with `b` bins per octave:

```
f_k = f_min * 2^(k/b)
Q = 1 / (2^(1/b) - 1)
```

With `b = 12` (semitone resolution): `Q = 1 / (2^(1/12) - 1) ≈ 16.82`

With `b = 24` (quarter-tone resolution): `Q ≈ 33.76`

This means each bin's window length is proportional to the period of its center frequency:

```
N_k = ceil(Q * fs / f_k)
```

For `f_min = 32.7 Hz` (C1), `fs = 44100`, `Q = 16.82`:
- `N_k` at 32.7 Hz = 22,676 samples (~514ms) -- extremely long window
- `N_k` at 4186 Hz (C8) = 177 samples (~4ms)

### 3.2 Direct CQT (Brown-Puckette Algorithm)

The original Brown (1991) / Brown-Puckette (1992) formulation computes each CQT bin directly:

```
X_CQ[k] = (1/N_k) * sum_{n=0}^{N_k-1} x[n] * w[n, N_k] * exp(-j * 2 * pi * Q * n / N_k)
```

where `w[n, N_k]` is a window function of length `N_k`.

**Computational cost**: For `K` total CQT bins, direct computation is `O(K * N_max)` per frame, where `N_max` is the window length at the lowest frequency. This is dramatically more expensive than an FFT-based approach for real-time use.

### 3.3 Efficient CQT via STFT (Real-Time Approach)

For real-time applications, compute CQT by aggregating STFT bins with a precomputed sparse kernel:

1. Compute STFT with sufficient resolution for the lowest CQT bin.
2. Multiply the STFT output by a sparse CQT kernel matrix `K`:

```
X_CQ = K * X_STFT
```

The kernel matrix `K` has shape `[num_cq_bins, fft_size/2+1]` and is sparse because each CQT bin only uses a subset of FFT bins.

**Schorkhuber & Klapuri (2010)** provide an efficient implementation using multiple STFT resolutions:
- High resolution (large N) for low CQT bins.
- Lower resolution (small N) for high CQT bins.
- Combine results from 2--3 STFT passes at different window sizes.

### 3.4 CQT vs STFT Comparison

| Aspect                | STFT                          | CQT                            |
|-----------------------|-------------------------------|---------------------------------|
| Freq spacing          | Linear (uniform Hz)           | Logarithmic (uniform semitones) |
| Low-freq resolution   | Poor (fixed by N)             | Excellent (long window)         |
| High-freq resolution  | Excessive (wasted bins)       | Matched to musical intervals    |
| Time resolution       | Uniform                       | Variable (fast at high freq)    |
| Pitch tracking        | Requires interpolation        | Direct bin-to-note mapping      |
| Computational cost    | O(N log N)                    | O(K * N) direct; O(N log N) via kernel |
| Real-time suitability | Excellent                     | Good (with kernel method)       |
| Onset detection       | Good                          | Poor at low frequencies (long window) |

**When CQT beats STFT for music visualization**:
- Chromagram / pitch-class visualization
- Note onset visualization aligned to musical scale
- Harmonic structure display (harmonics appear at fixed intervals)
- Any display that should look "musical" rather than "scientific"

**When STFT is better**:
- Broadband energy analysis (percussion, noise)
- Onset/transient detection (needs uniform time resolution)
- General-purpose spectrum display
- Lower latency requirements (sub-10ms)

---

## 4. Filterbank Approaches

### 4.1 IIR Filterbank

IIR (Infinite Impulse Response) filters are ideal for real-time band decomposition due to their low per-sample cost. A typical design uses cascaded second-order sections (biquads).

**Butterworth bandpass biquad** (most common for visualization due to maximally flat passband):

For a bandpass filter with center frequency `f0` and bandwidth `BW`:

```
omega_0 = 2 * pi * f0 / fs
alpha   = sin(omega_0) * sinh(ln(2) / 2 * BW * omega_0 / sin(omega_0))

b0 =  alpha
b1 =  0
b2 = -alpha
a0 =  1 + alpha
a1 = -2 * cos(omega_0)
a2 =  1 - alpha
```

Normalize all coefficients by `a0`.

**C++ Biquad Implementation**:

```cpp
struct Biquad {
    float b0, b1, b2, a1, a2;
    float z1 = 0.0f, z2 = 0.0f;  // state (Direct Form II Transposed)

    float process(float x) {
        float y = b0 * x + z1;
        z1 = b1 * x - a1 * y + z2;
        z2 = b2 * x - a2 * y;
        return y;
    }

    void reset() { z1 = z2 = 0.0f; }
};

// Design Butterworth bandpass (2nd order = 1 biquad section)
Biquad designBandpass(float f0, float bandwidth, float sampleRate) {
    float omega0 = 2.0f * M_PI * f0 / sampleRate;
    float sinW   = std::sin(omega0);
    float cosW   = std::cos(omega0);
    float alpha  = sinW * std::sinh(std::log(2.0f) / 2.0f * bandwidth * omega0 / sinW);

    Biquad bq;
    float a0_inv = 1.0f / (1.0f + alpha);
    bq.b0 =  alpha * a0_inv;
    bq.b1 =  0.0f;
    bq.b2 = -alpha * a0_inv;
    bq.a1 = -2.0f * cosW * a0_inv;
    bq.a2 =  (1.0f - alpha) * a0_inv;
    return bq;
}
```

**Chebyshev Type I** adds passband ripple for steeper rolloff:
- Allows specified passband ripple (e.g., 0.5 dB) for sharper transition bands.
- Better band separation than Butterworth at the same order.
- Introduces passband ringing -- visible as amplitude oscillation in visualization.

**Elliptic (Cauer)** filters achieve the steepest possible rolloff for a given order:
- Both passband and stopband ripple.
- Excellent for strict band separation.
- Phase response is highly nonlinear -- problematic if phase coherence matters (e.g., for reconstruction or inter-band correlation analysis).

**Cascading for steeper slopes**: For 4th-order Butterworth (24 dB/octave), cascade two 2nd-order sections. Design the prototype lowpass, transform to bandpass, then realize as cascaded biquads. Cookbook coefficients for higher orders require pole-zero placement from the Butterworth polynomial.

### 4.2 FIR Filterbank

FIR (Finite Impulse Response) filters offer linear phase at the cost of higher tap counts.

**Windowed sinc design** for a bandpass FIR:

```cpp
// Design FIR bandpass via spectral inversion of two lowpass filters
std::vector<float> designFIRBandpass(
    float fLow, float fHigh, float sampleRate, int numTaps)
{
    // numTaps must be odd for Type I FIR
    if (numTaps % 2 == 0) numTaps++;

    float fcLow  = fLow / sampleRate;
    float fcHigh = fHigh / sampleRate;
    int M = numTaps - 1;
    int halfM = M / 2;

    std::vector<float> h(numTaps);
    for (int n = 0; n < numTaps; ++n) {
        int nm = n - halfM;
        if (nm == 0) {
            h[n] = 2.0f * (fcHigh - fcLow);
        } else {
            float x = static_cast<float>(nm);
            h[n] = (std::sin(2.0f * M_PI * fcHigh * x) -
                     std::sin(2.0f * M_PI * fcLow * x)) / (M_PI * x);
        }
        // Blackman window
        float w = 0.42f - 0.5f * std::cos(2.0f * M_PI * n / M)
                        + 0.08f * std::cos(4.0f * M_PI * n / M);
        h[n] *= w;
    }
    return h;
}
```

**Tap count guidelines**: For a transition bandwidth of `delta_f`, the required tap count is approximately:

```
N ≈ 4 * fs / delta_f    (Blackman window)
N ≈ 3.3 * fs / delta_f  (Hamming window)
```

For a sub-bass filter (20--60 Hz) at 44100 Hz with 20 Hz transition width:
`N ≈ 4 * 44100 / 20 = 8820 taps` -- impractical for real-time per-sample processing, but feasible with overlap-save FFT convolution.

### 4.3 IIR Filterbank vs FFT Bin Aggregation

| Aspect                    | IIR Filterbank                  | FFT Bin Aggregation            |
|---------------------------|----------------------------------|--------------------------------|
| Latency                   | 1 sample (instantaneous)        | 1 FFT window (23--186ms)      |
| Low-freq resolution       | Excellent (analog-like)         | Poor (limited by N)           |
| CPU cost (7 bands)        | ~14 multiplies/sample (2nd ord) | 1 FFT + bin sum per hop       |
| CPU cost (24 bands)       | ~48 multiplies/sample           | Same FFT + more bin sums      |
| Phase coherence           | Preserved (Butterworth)         | Not applicable (magnitude)    |
| Implementation complexity | Moderate (filter design)        | Low (just bin indexing)       |
| Band edge sharpness       | Configurable (filter order)     | Rectangular (abrupt)          |
| Aliasing artifacts        | None (time-domain)              | Spectral leakage (windowing)  |
| Flexibility               | Fixed at design time             | Can redefine bands per frame  |

**Recommendation**: For a VJ engine, a hybrid approach works best:
- **FFT bin aggregation** for mid and high bands (adequate resolution, already computed for spectral features).
- **IIR filters** for sub-bass and bass bands (sample-accurate response, no latency penalty).

---

## 5. Perceptual Weighting

### 5.1 A-Weighting

A-weighting approximates the inverse of the human equal-loudness contour at low SPL (~40 phons). It de-emphasizes low and very high frequencies.

**Continuous-time transfer function** (IEC 61672:2003):

```
             k_A * s^4
H_A(s) = --------------------------------------------------
          (s + 2pi*20.6)^2 * (s + 2pi*107.7) * (s + 2pi*737.9) * (s + 2pi*12194)^2
```

where `k_A` is chosen for 0 dB at 1 kHz.

**Tabulated A-weighting values** (for reference):

| Freq (Hz) | A-weight (dB) |
|-----------|---------------|
| 31.5      | -39.4         |
| 63        | -26.2         |
| 125       | -16.1         |
| 250       | -8.6          |
| 500       | -3.2          |
| 1000      | 0.0           |
| 2000      | +1.2          |
| 4000      | +1.0          |
| 8000      | -1.1          |
| 16000     | -6.6          |

**Digital IIR implementation at 48 kHz** (cascaded second-order sections):

These coefficients implement the A-weighting curve as a cascade of three biquad sections, derived from bilinear transform of the analog prototype with prewarping at 1 kHz:

```cpp
// A-weighting filter coefficients for fs = 48000 Hz
// Source: derived from IEC 61672 analog prototype via bilinear transform
// Implemented as 3 cascaded biquad sections

// Section 1 (high-pass pair from 20.6 Hz poles)
// b = [1.0, -2.0, 1.0], a = [1.0, -1.99733, 0.99734]

// Section 2 (high-pass from 107.7 Hz and 737.9 Hz poles)
// b = [1.0, -2.0, 1.0], a = [1.0, -1.96847, 0.96906]

// Section 3 (low-pass pair from 12194 Hz poles)
// b = [1.0, 2.0, 1.0], a = [1.0, -0.24645, 0.00633]

// Overall gain: adjusted for 0 dB at 1 kHz
```

For `fs = 44100 Hz`, the same analog prototype is bilinear-transformed with different prewarping. The coefficients change -- **never reuse coefficients designed for a different sample rate**.

A practical approach: compute A-weighting in the frequency domain by multiplying FFT magnitudes by the A-weighting curve evaluated at each bin's center frequency:

```cpp
float aWeightDB(float freqHz) {
    if (freqHz < 1.0f) return -200.0f;  // effectively silence
    float f2 = freqHz * freqHz;
    float f4 = f2 * f2;

    float num = 12194.0f * 12194.0f * f4;
    float den = (f2 + 20.6f * 20.6f)
              * std::sqrt((f2 + 107.7f * 107.7f) * (f2 + 737.9f * 737.9f))
              * (f2 + 12194.0f * 12194.0f);

    float ra = num / den;
    return 20.0f * std::log10(ra) + 2.0f;  // +2.0 dB normalization
}
```

### 5.2 C-Weighting

C-weighting is flatter than A-weighting, approximating equal-loudness at ~100 phons. It retains more low-frequency energy.

```
          k_C * s^2
H_C(s) = ----------------------------------
          (s + 2pi*20.6)^2 * (s + 2pi*12194)^2
```

**Use case**: C-weighting is used for peak SPL measurement and as the basis for C-A difference (which quantifies low-frequency content dominance -- useful for bass-heavy music detection).

```cpp
float cWeightDB(float freqHz) {
    if (freqHz < 1.0f) return -200.0f;
    float f2 = freqHz * freqHz;

    float num = 12194.0f * 12194.0f * f2;
    float den = (f2 + 20.6f * 20.6f) * (f2 + 12194.0f * 12194.0f);

    float rc = num / den;
    return 20.0f * std::log10(rc) + 0.06f;  // +0.06 dB normalization
}
```

### 5.3 K-Weighting (ITU-R BS.1770 / LUFS)

K-weighting is used for integrated loudness measurement (LUFS/LKFS). It consists of two cascaded filters:

**Stage 1: Pre-filter (shelving)** -- models the acoustic effect of the head. Boosts high frequencies by ~4 dB.

At `fs = 48000`:
```
b = [1.53512485958697, -2.69169618940638, 1.19839281085285]
a = [1.0,              -1.69065929318241, 0.73248077421585]
```

**Stage 2: Revised Low-frequency B-curve (RLB)** -- high-pass removing content below ~60 Hz.

At `fs = 48000`:
```
b = [1.0, -2.0, 1.0]
a = [1.0, -1.99004745483398, 0.99007225036621]
```

**Use case in VJ engines**: K-weighted loudness (LUFS) is the standard for consistent loudness normalization across tracks. Apply K-weighting before computing short-term (3s) or momentary (400ms) loudness. This prevents visual intensity from varying wildly between quiet acoustic tracks and heavily compressed EDM.

### 5.4 When to Apply Each Weighting

| Weighting | Use Case in Visualization                                                |
|-----------|-------------------------------------------------------------------------|
| **None**  | Raw spectral analysis, waveform display, technical spectrum analyzer    |
| **A**     | Perceived loudness bands, "how loud does this sound" per band          |
| **C**     | Peak level metering, bass content detection (compare C-A)              |
| **K**     | Cross-track loudness normalization, LUFS metering display              |
| **Custom**| VJ-specific curves (e.g., boost bass perception for dance music viz)   |

---

## 6. Implementation Patterns

### 6.1 Seven-Band FFT Bin Analyzer

```cpp
#include <cmath>
#include <vector>
#include <algorithm>
#include <array>

class SevenBandAnalyzer {
public:
    static constexpr int NUM_BANDS = 7;

    struct BandDef {
        float fLow, fHigh;
        const char* name;
    };

    static constexpr std::array<BandDef, NUM_BANDS> BANDS = {{
        {   20.0f,    60.0f, "Sub-bass"  },
        {   60.0f,   250.0f, "Bass"      },
        {  250.0f,   500.0f, "Low-mid"   },
        {  500.0f,  2000.0f, "Mid"       },
        { 2000.0f,  4000.0f, "High-mid"  },
        { 4000.0f,  6000.0f, "Presence"  },
        { 6000.0f, 20000.0f, "Brilliance"}
    }};

    SevenBandAnalyzer(int fftSize, float sampleRate,
                      float smoothAttack = 0.3f, float smoothRelease = 0.05f)
        : fftSize_(fftSize)
        , sampleRate_(sampleRate)
        , smoothAttack_(smoothAttack)
        , smoothRelease_(smoothRelease)
    {
        smoothed_.fill(0.0f);

        // Precompute bin ranges for each band
        for (int b = 0; b < NUM_BANDS; ++b) {
            binLow_[b]  = static_cast<int>(std::ceil(BANDS[b].fLow * fftSize / sampleRate));
            binHigh_[b] = static_cast<int>(std::floor(BANDS[b].fHigh * fftSize / sampleRate));
            binLow_[b]  = std::clamp(binLow_[b], 0, fftSize / 2);
            binHigh_[b] = std::clamp(binHigh_[b], 0, fftSize / 2);
        }
    }

    // magnitudes: FFT magnitude array of length fftSize/2 + 1
    void process(const float* magnitudes) {
        for (int b = 0; b < NUM_BANDS; ++b) {
            // Compute RMS energy in band
            float sum = 0.0f;
            int count = 0;
            for (int k = binLow_[b]; k <= binHigh_[b]; ++k) {
                sum += magnitudes[k] * magnitudes[k];
                ++count;
            }

            float rms = (count > 0) ? std::sqrt(sum / count) : 0.0f;

            // Convert to dB with floor
            float db = (rms > 1e-10f) ? 20.0f * std::log10(rms) : -100.0f;
            float normalized = std::clamp((db + 60.0f) / 60.0f, 0.0f, 1.0f);  // -60dB floor

            // Asymmetric smoothing (fast attack, slow release)
            float coeff = (normalized > smoothed_[b]) ? smoothAttack_ : smoothRelease_;
            smoothed_[b] += coeff * (normalized - smoothed_[b]);
        }
    }

    float getBand(int band) const { return smoothed_[band]; }
    const std::array<float, NUM_BANDS>& getBands() const { return smoothed_; }

    // Get dB value for display (maps 0..1 back to -60..0 dB)
    float getBandDB(int band) const {
        return smoothed_[band] * 60.0f - 60.0f;
    }

private:
    int fftSize_;
    float sampleRate_;
    float smoothAttack_, smoothRelease_;
    std::array<int, NUM_BANDS> binLow_, binHigh_;
    std::array<float, NUM_BANDS> smoothed_;
};
```

### 6.2 Twenty-Four-Band Bark Analyzer

```cpp
class BarkBandAnalyzer {
public:
    static constexpr int NUM_BANDS = 24;

    // Zwicker critical band upper edges (Hz)
    static constexpr std::array<float, NUM_BANDS> UPPER_EDGES = {{
        100, 200, 300, 400, 510, 630, 770, 920,
        1080, 1270, 1480, 1720, 2000, 2320, 2700, 3150,
        3700, 4400, 5300, 6400, 7700, 9500, 12000, 15500
    }};

    BarkBandAnalyzer(int fftSize, float sampleRate,
                     float smoothCoeff = 0.15f)
        : fftSize_(fftSize)
        , sampleRate_(sampleRate)
        , smoothCoeff_(smoothCoeff)
    {
        smoothed_.fill(0.0f);
        peak_.fill(0.0f);

        // Compute bin ranges for each Bark band
        float prevEdge = 20.0f;
        for (int b = 0; b < NUM_BANDS; ++b) {
            binLow_[b]  = static_cast<int>(std::ceil(prevEdge * fftSize / sampleRate));
            binHigh_[b] = static_cast<int>(std::floor(UPPER_EDGES[b] * fftSize / sampleRate));
            binLow_[b]  = std::clamp(binLow_[b], 0, fftSize / 2);
            binHigh_[b] = std::clamp(binHigh_[b], binLow_[b], fftSize / 2);
            prevEdge = UPPER_EDGES[b];
        }
    }

    void process(const float* magnitudes) {
        for (int b = 0; b < NUM_BANDS; ++b) {
            // Sum power in band
            float power = 0.0f;
            int count = 0;
            for (int k = binLow_[b]; k <= binHigh_[b]; ++k) {
                power += magnitudes[k] * magnitudes[k];
                ++count;
            }

            // RMS amplitude
            float rms = (count > 0) ? std::sqrt(power / count) : 0.0f;

            // dB with -80 dB floor, normalized to 0..1
            float db = (rms > 1e-12f) ? 20.0f * std::log10(rms) : -80.0f;
            float normalized = std::clamp((db + 80.0f) / 80.0f, 0.0f, 1.0f);

            // Exponential smoothing
            smoothed_[b] += smoothCoeff_ * (normalized - smoothed_[b]);

            // Peak hold with decay
            if (normalized > peak_[b]) {
                peak_[b] = normalized;
            } else {
                peak_[b] *= 0.995f;  // slow decay for peak indicator
            }
        }
    }

    float getBand(int b) const { return smoothed_[b]; }
    float getPeak(int b) const { return peak_[b]; }

    // Log-scale display: convert band index to x-position [0..1]
    // Maps 24 bands uniformly since Bark is already quasi-logarithmic
    static float bandToDisplayX(int band) {
        return static_cast<float>(band) / (NUM_BANDS - 1);
    }

    // Get center frequency of each band for axis labeling
    static float bandCenterHz(int band) {
        static constexpr std::array<float, NUM_BANDS> CENTERS = {{
            50, 150, 250, 350, 450, 570, 700, 840,
            1000, 1170, 1370, 1600, 1850, 2150, 2500, 2900,
            3400, 4000, 4800, 5800, 7000, 8500, 10500, 13500
        }};
        return CENTERS[band];
    }

private:
    int fftSize_;
    float sampleRate_;
    float smoothCoeff_;
    std::array<int, NUM_BANDS> binLow_, binHigh_;
    std::array<float, NUM_BANDS> smoothed_;
    std::array<float, NUM_BANDS> peak_;
};
```

### 6.3 Smoothing Strategies

**Exponential Moving Average (EMA)**: `y[n] = alpha * x[n] + (1 - alpha) * y[n-1]`

- Simple, cheap, single parameter.
- Time constant: `tau = -1 / ln(1 - alpha)` (in frames).
- For 30fps display with ~100ms time constant: `alpha ≈ 0.28`.

**Asymmetric attack/release**: Use different `alpha` for rising vs. falling values:
- Fast attack (`alpha_attack ≈ 0.3--0.5`): responds quickly to beats.
- Slow release (`alpha_release ≈ 0.02--0.1`): smooth visual decay.

**Per-band time constants**: Low-frequency content (sub-bass, bass) has longer natural periods. Using the same time constant across all bands makes bass appear sluggish relative to high-frequency transients. Consider scaling alpha by band:

```cpp
float alphaForBand(int band, float baseAlpha) {
    // Lower bands get faster response to compensate for their
    // inherently slower oscillation
    // This is a design choice -- some VJs prefer uniform smoothing
    float scale = 1.0f + 0.5f * (1.0f - static_cast<float>(band) / (NUM_BANDS - 1));
    return std::clamp(baseAlpha * scale, 0.01f, 0.99f);
}
```

### 6.4 Log-Scale Display Mapping

For a frequency axis display, linear spacing wastes visual space on high frequencies and compresses musically important low frequencies. Map to log scale:

```cpp
// Map frequency to display x-position [0..1]
float freqToDisplayX(float freqHz, float fMin = 20.0f, float fMax = 20000.0f) {
    if (freqHz <= fMin) return 0.0f;
    if (freqHz >= fMax) return 1.0f;
    return std::log2(freqHz / fMin) / std::log2(fMax / fMin);
}

// Inverse: display position to frequency
float displayXToFreq(float x, float fMin = 20.0f, float fMax = 20000.0f) {
    return fMin * std::pow(fMax / fMin, x);
}
```

For the 7-band analyzer, position each bar at the geometric mean of its band edges:

```cpp
float bandDisplayCenter(float fLow, float fHigh) {
    return freqToDisplayX(std::sqrt(fLow * fHigh));
}
```

---

## 7. Octave Band Analysis

### 7.1 1/1 Octave Bands

Octave bands are defined by ISO 266 with preferred center frequencies based on the reference frequency of 1000 Hz:

```
f_center(n) = 1000 * 2^n   (for integer n)
```

| Band (n) | Center (Hz) | Lower Edge (Hz) | Upper Edge (Hz) |
|----------|-------------|------------------|------------------|
| -5       | 31.25       | 22.1             | 44.2             |
| -4       | 62.5        | 44.2             | 88.4             |
| -3       | 125         | 88.4             | 176.8            |
| -2       | 250         | 176.8            | 353.6            |
| -1       | 500         | 353.6            | 707.1            |
| 0        | 1000        | 707.1            | 1414.2           |
| +1       | 2000        | 1414.2           | 2828.4           |
| +2       | 4000        | 2828.4           | 5656.9           |
| +3       | 8000        | 5656.9           | 11313.7          |
| +4       | 16000       | 11313.7          | 22627.4          |

Band edges: `f_lower = f_center / sqrt(2)`, `f_upper = f_center * sqrt(2)`.

This gives 10 bands across the audible range -- a natural fit for many visualization layouts.

### 7.2 1/3 Octave Bands

Each octave is split into 3 sub-bands. Center frequencies follow:

```
f_center(n) = 1000 * 2^(n/3)
```

Band edges: `f_lower = f_center / 2^(1/6)`, `f_upper = f_center * 2^(1/6)`.

**ISO 266 Preferred 1/3-Octave Center Frequencies (Hz)**:

```
20, 25, 31.5, 40, 50, 63, 80, 100, 125, 160,
200, 250, 315, 400, 500, 630, 800, 1000, 1250, 1600,
2000, 2500, 3150, 4000, 5000, 6300, 8000, 10000, 12500, 16000, 20000
```

This gives 31 bands across 20--20000 Hz. The 1/3-octave resolution is close to the critical bandwidth at mid-to-high frequencies, making it a reasonable perceptual approximation.

### 7.3 1/6 Octave Bands

Six sub-bands per octave, yielding ~62 bands across 20--20kHz:

```
f_center(n) = 1000 * 2^(n/6)
```

Band edges: `f_lower = f_center / 2^(1/12)`, `f_upper = f_center * 2^(1/12)`.

At 1/6 octave, each band spans exactly 2 semitones (a whole tone). This resolution bridges musical interval analysis and broadband spectral display.

### 7.4 Fractional-Octave Band Implementation

Generalized for any fraction `1/N` octave:

```cpp
struct OctaveBand {
    float centerHz;
    float lowerHz;
    float upperHz;
    int binLow;
    int binHigh;
};

std::vector<OctaveBand> buildFractionalOctaveBands(
    int fractionsPerOctave,  // 1, 3, 6, 12, 24...
    float fMin, float fMax,
    int fftSize, float sampleRate)
{
    std::vector<OctaveBand> bands;
    float refFreq = 1000.0f;
    float ratio = 1.0f / static_cast<float>(fractionsPerOctave);

    // Find starting band index
    int nStart = static_cast<int>(
        std::floor(fractionsPerOctave * std::log2(fMin / refFreq)));
    int nEnd = static_cast<int>(
        std::ceil(fractionsPerOctave * std::log2(fMax / refFreq)));

    float halfBandRatio = std::pow(2.0f, 0.5f * ratio);

    for (int n = nStart; n <= nEnd; ++n) {
        OctaveBand band;
        band.centerHz = refFreq * std::pow(2.0f, n * ratio);
        band.lowerHz  = band.centerHz / halfBandRatio;
        band.upperHz  = band.centerHz * halfBandRatio;

        if (band.lowerHz > fMax || band.upperHz < fMin) continue;

        band.binLow  = static_cast<int>(std::ceil(band.lowerHz * fftSize / sampleRate));
        band.binHigh = static_cast<int>(std::floor(band.upperHz * fftSize / sampleRate));
        band.binLow  = std::clamp(band.binLow, 0, fftSize / 2);
        band.binHigh = std::clamp(band.binHigh, band.binLow, fftSize / 2);

        // Skip bands with no FFT bins
        if (band.binHigh >= band.binLow) {
            bands.push_back(band);
        }
    }
    return bands;
}
```

### 7.5 FFT Size Requirements for Octave Analysis

The minimum FFT size to guarantee at least 1 bin per band at the lowest frequency:

| Resolution    | Lowest Center (Hz) | Min Bandwidth (Hz) | Min FFT Size (44.1kHz) | Min FFT Size (48kHz) |
|---------------|--------------------|--------------------|------------------------|----------------------|
| 1/1 octave    | 31.25              | 22.1               | 2048                   | 2048                 |
| 1/3 octave    | 20                 | 4.6                | 16384                  | 16384                |
| 1/6 octave    | 20                 | 2.3                | 32768                  | 32768                |
| 1/12 octave   | 20                 | 1.15               | 65536                  | 65536                |

At 1/3-octave and finer, the FFT sizes needed for low-frequency resolution introduce unacceptable latency for real-time visualization (>370ms at 1/3-octave). Mitigations:

1. **Multi-resolution STFT**: Use large FFT only for low bands, short FFT for highs.
2. **IIR filterbank for low bands**: No FFT latency, sample-accurate tracking.
3. **Accept sparse low bins**: Use 4096-point FFT and interpolate for the lowest 1/3-octave bands.
4. **CQT**: Inherently multi-resolution (see Section 3).

---

## 8. Design Recommendations for VJ Engines

### 8.1 Choosing Band Count and Scale

| Visualization Type              | Recommended Bands                  | Scale      |
|--------------------------------|-------------------------------------|------------|
| Simple beat-reactive visuals   | 3--4 (bass, mid, treble)           | Custom     |
| Graphic EQ style display       | 7--10 (octave bands)              | Octave     |
| Detailed spectrum bar graph    | 24--31 (Bark or 1/3 octave)       | Bark / ISO |
| Full spectrogram / waterfall   | 128--512 (mel or CQT bins)        | Mel / CQT  |
| Chromagram / pitch display     | 12--24 per octave (CQT)           | CQT        |
| Perceptual loudness analysis   | 24--40 (Bark or ERB)              | ERB        |

### 8.2 Complete Processing Pipeline

A production VJ audio engine typically follows this chain:

```
Audio Input
  -> Window (Hann/Blackman-Harris)
  -> FFT
  -> Magnitude Spectrum
  -> [Optional: A-weighting in frequency domain]
  -> Band Aggregation (7-band, 24-band, 1/3-octave, or mel filterbank)
  -> dB Conversion (20 * log10)
  -> Normalization (map dB range to 0..1)
  -> Smoothing (per-band EMA, asymmetric attack/release)
  -> Peak Hold (optional, for peak indicators)
  -> Output to Visualization Parameters
```

### 8.3 Latency Budget

For real-time VJ work at 60fps, the audio analysis must complete within the display frame period of ~16.6ms. Typical latency contributions:

| Stage                    | Latency (samples @ 44.1kHz)       | Time          |
|--------------------------|-----------------------------------|---------------|
| Buffer fill (hop size)   | 512--1024                         | 11.6--23.2ms  |
| FFT compute (N=4096)     | Negligible (<0.1ms on modern CPU) | <0.1ms        |
| Filterbank multiply      | Negligible                        | <0.01ms       |
| Smoothing                | None (causal)                     | 0ms           |
| **Total pipeline**       |                                   | **12--24ms**  |

The dominant factor is the hop size, which determines how often the analysis updates. A hop of 512 samples at 44.1 kHz gives ~86 updates/second, comfortably above 60fps display rate.

### 8.4 Thread Safety Note

In a VJ engine, audio analysis runs on the audio thread while visualization reads values on the render thread. The band analyzer output should be double-buffered or use atomic operations:

```cpp
// Lock-free band output using atomic swap
struct BandOutput {
    alignas(64) std::array<float, 24> bands;  // cache-line aligned
};

std::atomic<BandOutput*> currentOutput;
BandOutput bufferA, bufferB;
BandOutput* writeBuffer = &bufferA;

// Audio thread: write to writeBuffer, then swap
void audioThreadUpdate(const float* magnitudes) {
    analyzer.process(magnitudes);
    for (int b = 0; b < 24; ++b)
        writeBuffer->bands[b] = analyzer.getBand(b);

    BandOutput* old = currentOutput.exchange(writeBuffer, std::memory_order_release);
    writeBuffer = old;  // reclaim old read buffer as next write target
}

// Render thread: read latest
const BandOutput* renderThreadRead() {
    return currentOutput.load(std::memory_order_acquire);
}
```

---

## 9. Summary of Formulas

| Quantity                  | Formula                                                     |
|---------------------------|-------------------------------------------------------------|
| FFT bin frequency         | `f(k) = k * fs / N`                                        |
| Bin range for band        | `k_low = ceil(f_low * N / fs)`, `k_high = floor(f_high * N / fs)` |
| Band energy               | `E = sum |X[k]|^2` over band bins                           |
| Hz to Mel                 | `m = 2595 * log10(1 + f/700)`                               |
| Mel to Hz                 | `f = 700 * (10^(m/2595) - 1)`                               |
| Hz to Bark                | `z = 26.81*f/(1960+f) - 0.53`                               |
| Hz to ERB-rate            | `E = 21.4 * log10(1 + 0.00437*f)`                           |
| ERB bandwidth             | `ERB(f) = 24.7 * (4.37*f/1000 + 1)`                         |
| CQT Q factor              | `Q = 1 / (2^(1/b) - 1)`                                     |
| Octave band edges         | `f_lower = f_c / 2^(1/2N)`, `f_upper = f_c * 2^(1/2N)`    |
| 1/N octave center         | `f_c(n) = f_ref * 2^(n/N)`                                  |
| A-weighting (approx)      | See `aWeightDB()` function in Section 5.1                   |
| EMA smoothing              | `y[n] = alpha * x[n] + (1-alpha) * y[n-1]`                  |

---

*Document version: 2026-03-13. For corrections or additions, update this file and cross-referenced documents.*
