# Pitch Detection & Harmonic Analysis

> Research document for real-time music visualization / VJ engine.
> Cross-references: [FEATURES_mfcc_mel.md](FEATURES_mfcc_mel.md), [FEATURES_spectral.md](FEATURES_spectral.md), [FEATURES_transients_texture.md](FEATURES_transients_texture.md), [LIB_aubio.md](LIB_aubio.md), [LIB_essentia.md](LIB_essentia.md), [REF_math_reference.md](REF_math_reference.md)

---

## 1. Monophonic Pitch Detection Algorithms

### 1.1 YIN Algorithm (de Cheveigne & Kawahara, 2002)

YIN is the de facto standard for monophonic pitch detection in real-time systems. It operates entirely in the time domain and avoids the resolution limits of FFT-based methods at low frequencies. The algorithm proceeds through six steps.

**Step 1 -- Difference Function**

For a signal x[t] of window size W, the difference function at lag tau is:

```
d(tau) = sum_{j=0}^{W-1} (x[j] - x[j + tau])^2
```

This measures the squared difference between the signal and a lagged copy of itself. A perfect periodic signal produces d(tau) = 0 at the fundamental period.

**Step 2 -- Cumulative Mean Normalized Difference Function (CMND)**

Raw d(tau) is biased: d(0) = 0 always, and d(tau) grows with tau due to energy accumulation. The CMND normalizes this:

```
d'(tau) = 1,                                          if tau = 0
d'(tau) = d(tau) / [(1/tau) * sum_{j=1}^{tau} d(j)],  if tau > 0
```

This division by the running mean eliminates the energy bias. Dips below 1.0 indicate periodicity, with the deepest dip corresponding to the fundamental period.

**Step 3 -- Absolute Threshold**

Select the smallest tau where d'(tau) < threshold (typically 0.10--0.15 for clean signals, 0.20 for noisy signals). This prevents octave errors by picking the first sub-threshold dip rather than the deepest overall minimum. When no value falls below the threshold, the algorithm reports "unvoiced" or takes the global minimum as a fallback.

**Step 4 -- Parabolic Interpolation**

The lag from step 3 has integer resolution. Fit a parabola through d'(tau-1), d'(tau), d'(tau+1) and find the vertex:

```
tau_refined = tau + (d'(tau-1) - d'(tau+1)) / (2 * (d'(tau-1) - 2*d'(tau) + d'(tau+1)))
```

This yields sub-sample accuracy, reducing quantization error from ~50 cents at 100 Hz / 44100 Hz to under 1 cent.

**Step 5 -- Best Local Estimate**

Search for a deeper minimum near the selected tau. If a neighboring lag has a lower d'(tau), switch to it. This corrects cases where noise pushes the first sub-threshold dip slightly off the true period.

**Step 6 -- Pitch Result**

```
f0 = sample_rate / tau_refined
```

**C++ Implementation (production-quality)**

```cpp
#include <vector>
#include <cmath>
#include <algorithm>

struct YinResult {
    float frequency;    // Hz, 0 if unvoiced
    float confidence;   // 1.0 - d'(tau_best), range [0, 1]
};

class YinDetector {
public:
    YinDetector(int sampleRate, int bufferSize, float threshold = 0.15f)
        : sr_(sampleRate)
        , W_(bufferSize / 2)  // integration window = half buffer
        , threshold_(threshold)
        , diff_(W_)
        , cmnd_(W_)
    {}

    YinResult detect(const float* audio, int numSamples) {
        if (numSamples < 2 * W_) return {0.f, 0.f};

        // Step 1: difference function
        for (int tau = 0; tau < W_; ++tau) {
            double sum = 0.0;
            for (int j = 0; j < W_; ++j) {
                double delta = audio[j] - audio[j + tau];
                sum += delta * delta;
            }
            diff_[tau] = static_cast<float>(sum);
        }

        // Step 2: CMND
        cmnd_[0] = 1.0f;
        double runningSum = 0.0;
        for (int tau = 1; tau < W_; ++tau) {
            runningSum += diff_[tau];
            cmnd_[tau] = diff_[tau] * tau / static_cast<float>(runningSum);
        }

        // Step 3: absolute threshold -- find first dip below threshold
        int tauBest = -1;
        // Skip tau 0 and 1 (meaningless), start from tau corresponding to max freq
        int tauMin = sr_ / 2000;  // ~2000 Hz upper bound
        int tauMax = std::min(W_ - 1, sr_ / 50);  // ~50 Hz lower bound

        for (int tau = tauMin; tau < tauMax; ++tau) {
            if (cmnd_[tau] < threshold_) {
                // Step 5: walk to the local minimum
                while (tau + 1 < tauMax && cmnd_[tau + 1] < cmnd_[tau]) {
                    ++tau;
                }
                tauBest = tau;
                break;
            }
        }

        if (tauBest < 0) {
            // Fallback: global minimum
            tauBest = tauMin;
            for (int tau = tauMin + 1; tau < tauMax; ++tau) {
                if (cmnd_[tau] < cmnd_[tauBest]) tauBest = tau;
            }
            if (cmnd_[tauBest] >= 1.0f) return {0.f, 0.f}; // truly unvoiced
        }

        // Step 4: parabolic interpolation
        float tauRefined = static_cast<float>(tauBest);
        if (tauBest > 0 && tauBest < W_ - 1) {
            float a = cmnd_[tauBest - 1];
            float b = cmnd_[tauBest];
            float c = cmnd_[tauBest + 1];
            float denom = 2.0f * (a - 2.0f * b + c);
            if (std::abs(denom) > 1e-12f) {
                tauRefined += (a - c) / denom;
            }
        }

        float freq = sr_ / tauRefined;
        float conf = 1.0f - cmnd_[tauBest];
        return {freq, std::clamp(conf, 0.f, 1.f)};
    }

private:
    int sr_;
    int W_;
    float threshold_;
    std::vector<float> diff_;
    std::vector<float> cmnd_;
};
```

Key implementation notes:
- Buffer size of 2048 at 44100 Hz gives a minimum detectable frequency of ~43 Hz (tau_max = 1024). For bass content, use 4096 samples.
- The O(W^2) inner loop dominates cost. For W = 1024, that is ~1M multiply-adds per frame. On modern CPUs with SIMD, this is well under 1 ms.

### 1.2 pYIN (Probabilistic YIN)

pYIN (Mauch & Dixon, 2014) extends YIN by treating multiple candidate periods as a probability distribution rather than committing to a single estimate.

**Procedure:**

1. Run the standard YIN difference function and CMND.
2. Instead of thresholding once, extract all local minima in d'(tau) and assign each a probability proportional to exp(-d'(tau) / beta), where beta is a smoothing parameter.
3. Feed these per-frame candidate distributions into a Hidden Markov Model (HMM):
   - States: quantized pitch values (e.g., 480 states covering 55--1760 Hz in 10-cent steps), plus one "unvoiced" state.
   - Transition probabilities: penalize large pitch jumps (Gaussian with sigma ~ 40 cents for typical vocal material).
   - Observation probabilities: from step 2.
4. Viterbi decoding yields the most likely pitch trajectory.

**Advantages over YIN:**
- Octave errors are dramatically reduced (the HMM's transition model penalizes octave jumps unless evidence is strong).
- Confidence is the posterior probability of the voiced state, which is better calibrated than YIN's raw 1 - d'(tau).
- Latency cost: pYIN is typically run offline or with a short lookahead (2--5 frames). For real-time use, a fixed-lag Viterbi decoder with ~50 ms lookahead gives most of the benefit.

**Real-time feasibility:** Marginal. The HMM adds ~0.1 ms per frame, which is negligible, but the lookahead latency (~50 ms) is the real cost. For VJ engines where pitch-to-visual mapping tolerates 50 ms delay, pYIN is preferable to YIN for its superior stability.

### 1.3 Autocorrelation Method

The autocorrelation function is the foundational pitch detection approach:

```
R(tau) = sum_{j=0}^{W-1} x[j] * x[j + tau]
```

**Normalized autocorrelation** divides by the geometric mean of the signal energy at lag 0 and lag tau:

```
R_norm(tau) = R(tau) / sqrt(sum x[j]^2 * sum x[j+tau]^2)
```

**Peak picking:** The first peak in R_norm(tau) above a threshold (typically 0.9) after the initial decay from R(0) gives the fundamental period. Parabolic interpolation refines the lag.

**Weaknesses:** Raw autocorrelation is biased toward higher lags (the integration window shrinks), producing octave errors. YIN's difference function is mathematically equivalent to autocorrelation but formulated to avoid this bias. For this reason, raw autocorrelation is rarely used in modern systems.

### 1.4 Harmonic Product Spectrum (HPS)

HPS operates in the frequency domain. The core insight: harmonics of f0 (at f0, 2*f0, 3*f0, ...) will align when the spectrum is downsampled by integer factors.

**Procedure:**

1. Compute magnitude spectrum |X(f)| from an FFT.
2. Downsample |X(f)| by factors 2, 3, ..., H (typically H = 5).
3. Multiply all versions element-wise:

```
HPS(f) = |X(f)| * |X(f/2)| * |X(f/3)| * ... * |X(f/H)|
```

4. The peak of HPS(f) indicates f0.

**Strengths:** Extremely robust against single missing harmonics. Simple to implement.

**Weaknesses:**
- Frequency resolution limited by FFT bin spacing: delta_f = sr / N. At 44100/2048 = 21.5 Hz, this is ~360 cents at 100 Hz. Zero-padding or interpolation helps.
- Downsampling by factor H reduces the usable frequency range by H.
- Not suitable for low-latency applications because it requires long FFT windows for adequate frequency resolution.

Best use case: quick-and-dirty pitch estimation for mid/high-range instruments, or as a secondary confirmation for time-domain methods.

### 1.5 McLeod Pitch Method (MPM)

MPM (McLeod & Wyvill, 2005) uses the Normalized Square Difference Function (NSDF):

```
NSDF(tau) = 2 * R(tau) / (sum_{j=0}^{W-1-tau} x[j]^2 + sum_{j=tau}^{W-1} x[j+tau-tau]^2)
```

The NSDF is bounded in [-1, 1] and peaks exactly at 1.0 for a perfectly periodic signal at lag tau = period.

**Key innovation:** Instead of looking for the first peak above a threshold, MPM finds all "key maxima" -- peaks that are the highest point between two zero crossings of the NSDF. It then selects the first key maximum whose height exceeds a fraction (typically 0.93) of the global maximum. This naturally avoids octave errors without YIN's absolute threshold heuristic.

**Implementation outline:**
1. Compute NSDF via autocorrelation (can be accelerated with FFT: O(N log N) instead of O(N^2)).
2. Find all positive zero crossings.
3. Between consecutive zero crossings, find the maximum (key maximum).
4. Select the first key maximum exceeding 0.93 * global_max.
5. Parabolic interpolation around the selected peak.

**Performance:** MPM has similar accuracy to YIN but fewer octave errors on certain instrument types (particularly plucked strings). It is the algorithm used in the Tartini tuner software.

### 1.6 CREPE (Convolutional Representation for Pitch Estimation)

CREPE (Kim et al., 2018) is a data-driven approach using a 6-layer CNN trained on synthesized audio with known ground truth f0.

**Architecture:**
- Input: 1024-sample frames at 16 kHz (64 ms windows).
- 6 convolutional layers with batch norm and dropout, operating on the raw waveform.
- Output: 360-dimensional vector representing pitch activation across 360 bins spanning 32.7--1975.5 Hz in 20-cent steps.
- The argmax gives the pitch; the max value gives confidence.

**Accuracy:** CREPE achieves raw pitch accuracy (RPA) of ~91% at 50-cent tolerance on the MIR-1K dataset, significantly outperforming YIN (~78%) and pYIN (~86%).

**Real-time feasibility:** The full CREPE model requires ~2 ms per frame on a GPU. The "tiny" variant (~4x fewer parameters) runs at ~0.5 ms on GPU. On CPU, full CREPE is 10--50 ms per frame depending on hardware -- not viable for real-time without GPU acceleration. For a VJ engine, CREPE serves as an offline accuracy benchmark rather than a production choice.

### 1.7 Comparison Table

| Algorithm | Accuracy (RPA 50c) | Typical Error (cents) | Latency (ms) | CPU Cost (per frame) | Best Use Case |
|-----------|-------------------|-----------------------|---------------|---------------------|---------------|
| YIN | ~78% | 5--15 | 23--46 (1024--2048 samples @ 44.1k) | 0.1--0.5 ms | Real-time monophonic, general purpose |
| pYIN | ~86% | 3--8 | 50--100 (+ HMM lookahead) | 0.2--0.6 ms | Vocal tracking, studio analysis |
| Autocorrelation | ~65% | 20--50 | 23--46 | 0.1--0.3 ms | Educational, legacy systems |
| HPS | ~70% | 20--100 (depends on N) | 46--93 (needs long FFT) | 0.05--0.2 ms | Mid-range instrument ID |
| MPM | ~80% | 3--10 | 23--46 | 0.1--0.5 ms | Instrument tuning, plucked strings |
| CREPE | ~91% | 1--5 | 64 (input) + 2 (GPU) / 30 (CPU) | 2 ms GPU / 30 ms CPU | Offline benchmark, GPU-equipped systems |

Notes on the table:
- Latency is dominated by the input buffer size, not computation time. A 2048-sample buffer at 44100 Hz imposes 46 ms of inherent algorithmic latency.
- CPU cost assumes single-threaded x86-64 with AVX2. SIMD vectorization of YIN/MPM inner loops gives 3--4x speedup.
- For the VJ engine, **YIN** (with parabolic interpolation, threshold 0.15) is the recommended default. Switch to pYIN if pitch stability matters more than latency.

---

## 2. Pitch Confidence and Voicing Detection

Reliable pitch confidence is critical for a VJ engine: visualizing pitch during unpitched segments (drums, noise, silence) produces chaotic, meaningless output.

### 2.1 Per-Algorithm Confidence Measures

**YIN:** Confidence = 1 - d'(tau_best). Range [0, 1]. A clean sine gives ~0.98; speech gives 0.7--0.95; noise gives < 0.3. Threshold of 0.75--0.85 works well as a voicing decision.

**pYIN:** The HMM posterior probability of the voiced state. Better calibrated than YIN because it integrates temporal context. Values above 0.5 reliably indicate pitched content.

**Autocorrelation:** The normalized autocorrelation peak value. Values above 0.9 indicate periodicity. Poorly calibrated for noisy signals.

**HPS:** Ratio of the HPS peak to the mean HPS value ("peak prominence"). No standard threshold; requires per-application tuning.

**MPM:** The NSDF key maximum height. Values above 0.93 indicate strong periodicity; this is the same threshold used for octave-error avoidance. Dual use limits flexibility.

**CREPE:** The maximum activation in the output layer. Well calibrated by training.

### 2.2 Voicing Detection Pipeline

For real-time use, a multi-stage voicing gate is recommended:

```cpp
struct VoicingDecision {
    bool is_voiced;
    float confidence;
};

VoicingDecision voicingGate(float rmsDb, float yinConf, float zeroCrossingRate) {
    // Stage 1: silence gate (see FEATURES_transients_texture.md)
    if (rmsDb < -60.0f) return {false, 0.0f};

    // Stage 2: noise gate via zero-crossing rate
    // High ZCR (> 0.3 for 44.1k) indicates noise/unvoiced fricatives
    if (zeroCrossingRate > 0.3f) return {false, 0.1f};

    // Stage 3: YIN confidence
    if (yinConf < 0.75f) return {false, yinConf};

    return {true, yinConf};
}
```

### 2.3 Handling Noise and Silence

- **Silence:** Apply a -55 to -65 dB RMS gate before pitch detection. Running YIN on silence produces random frequencies with low confidence, but even low-confidence values can leak into visualizations via smoothing.
- **Noise:** Broadband noise produces d'(tau) ~ 1.0 everywhere, so YIN confidence is near zero. Pink noise can occasionally produce false sub-threshold dips at low tau (high frequencies); the minimum tau bound (sr/2000) prevents this.
- **Transients:** Drum hits produce brief windows with quasi-periodic content (the resonant modes of the drum). To avoid spurious pitch during transients, apply onset detection (see FEATURES_transients_texture.md) and suppress pitch output for 20--50 ms after each onset.

---

## 3. Polyphonic Pitch Detection

### 3.1 NMF (Non-negative Matrix Factorization)

NMF decomposes a magnitude spectrogram V (F frequency bins x T time frames) into:

```
V ~ W * H
```

Where W (F x K) contains K spectral templates (basis vectors) and H (K x T) contains their activations over time.

**For pitch detection:**
- Pre-define W as harmonic combs: for each MIDI note n, W[:,n] has peaks at f0(n), 2*f0(n), 3*f0(n), ..., with amplitudes decaying as 1/k or learned from training data.
- Solve for H using multiplicative update rules:

```
H <- H * (W^T * V) / (W^T * W * H + epsilon)
```

- Active pitches at time t are the columns of H with activation above a threshold.

**Real-time considerations:**
- Per-frame NMF update with fixed W is O(F * K) per iteration, ~20 iterations needed. For F = 1025, K = 88 (piano range): ~1.8M multiply-adds per frame per iteration = ~36M total. At 44100/1024 = 43 fps, this is ~1.5 GFLOPS -- feasible on modern CPUs but leaves little headroom.
- Practical compromise: reduce K to the active pitch range, use fewer iterations (5--10 with warm-starting from the previous frame), and downsample the spectrogram.

### 3.2 Harmonic Summation Methods

Compute a "salience function" S(f0) by summing spectral energy at expected harmonic locations:

```
S(f0) = sum_{h=1}^{H} w(h) * |X(h * f0)|^2
```

Where w(h) are harmonic weights (typically w(h) = 1/h or learned). Peaks in S(f0) indicate active pitches.

This is the approach used in Melodia (Salamon & Gomez, 2012) and is the basis for Essentia's PitchSalience and PredominantPitchMelodia algorithms (see LIB_essentia.md).

**Real-time cost:** O(P * H) where P is the number of f0 candidates and H the number of harmonics. For P = 500 candidates (logarithmically spaced), H = 10: 5000 lookups per frame, negligible.

### 3.3 Neural Approaches

**Basic Pitch (Spotify, 2022):** A lightweight CNN that outputs a piano-roll-like activation matrix (128 MIDI notes x T frames). Trained on synthesized polyphonic mixtures. Runs near real-time on GPU (~5 ms per 256-sample hop). CPU performance is ~20 ms per frame -- borderline for real-time.

**CREPE Multi-f0:** Research extensions of CREPE to polyphonic scenarios exist but are computationally expensive (multiple forward passes or multi-output architectures). Not recommended for real-time.

### 3.4 Feasibility Summary for Real-Time VJ Engine

For the VJ engine, polyphonic pitch detection is a "nice to have" rather than a requirement. Recommended strategy:

1. **Primary:** Run monophonic YIN on the full mix. This tracks the most prominent pitched element (lead vocal, lead synth) with high reliability.
2. **Secondary (if CPU budget allows):** Run harmonic summation on the chroma representation to identify active pitch classes (not octave-specific). This is computationally cheap and feeds directly into chord/key detection.
3. **Avoid:** Full NMF or neural multi-f0 in real-time unless GPU acceleration is available.

---

## 4. Key Detection

### 4.1 Krumhansl-Schmuckler Key-Finding Algorithm

The Krumhansl-Schmuckler algorithm (1990) correlates a chroma vector against 24 key profiles (12 major + 12 minor). The key profile with the highest correlation is the detected key.

**Key Profiles (Krumhansl & Kessler, 1982):**

Probe-tone ratings representing the perceived stability of each pitch class within a key context:

```
Major profile: [6.35, 2.23, 3.48, 2.33, 4.38, 4.09, 2.52, 5.19, 2.39, 3.66, 2.29, 2.88]
Minor profile: [6.33, 2.68, 3.52, 5.38, 2.60, 3.53, 2.54, 4.75, 3.98, 2.69, 3.34, 3.17]
```

These are indexed chromatically starting from the tonic (C=0 for C major, etc.).

**Algorithm:**

1. Compute a 12-dimensional chroma vector from the audio (sum energy across all octaves per pitch class, see FEATURES_spectral.md for chroma computation).
2. For each of 24 keys (12 roots x {major, minor}), circularly shift the appropriate profile to align with the candidate tonic.
3. Compute the Pearson correlation between the observed chroma and each shifted profile:

```
r = (sum (x_i - x_mean)(y_i - y_mean)) / sqrt(sum (x_i - x_mean)^2 * sum (y_i - y_mean)^2)
```

4. The key with the highest r is the detected key. The value of r serves as a confidence measure.

**C++ Implementation:**

```cpp
#include <array>
#include <cmath>
#include <string>

struct KeyResult {
    int root;           // 0=C, 1=C#, ..., 11=B
    bool isMajor;       // true = major, false = minor
    float correlation;  // Pearson r, range [-1, 1]
    std::string name;   // e.g., "A minor"
};

class KrumhanslSchmuckler {
public:
    static constexpr std::array<float, 12> majorProfile = {
        6.35f, 2.23f, 3.48f, 2.33f, 4.38f, 4.09f,
        2.52f, 5.19f, 2.39f, 3.66f, 2.29f, 2.88f
    };
    static constexpr std::array<float, 12> minorProfile = {
        6.33f, 2.68f, 3.52f, 5.38f, 2.60f, 3.53f,
        2.54f, 4.75f, 3.98f, 2.69f, 3.34f, 3.17f
    };

    static const char* noteNames[12];

    KeyResult detect(const std::array<float, 12>& chroma) {
        KeyResult best = {0, true, -2.0f, ""};

        // Mean of observed chroma
        float chromaMean = 0.f;
        for (float c : chroma) chromaMean += c;
        chromaMean /= 12.f;

        for (int root = 0; root < 12; ++root) {
            for (int mode = 0; mode < 2; ++mode) {
                const auto& profile = (mode == 0) ? majorProfile : minorProfile;

                // Circularly shifted profile mean
                float profMean = 0.f;
                for (float p : profile) profMean += p;
                profMean /= 12.f;

                // Pearson correlation
                float num = 0.f, denomA = 0.f, denomB = 0.f;
                for (int i = 0; i < 12; ++i) {
                    int chromaIdx = (i + root) % 12;
                    float a = chroma[chromaIdx] - chromaMean;
                    float b = profile[i] - profMean;
                    num += a * b;
                    denomA += a * a;
                    denomB += b * b;
                }
                float denom = std::sqrt(denomA * denomB);
                float r = (denom > 1e-12f) ? num / denom : 0.f;

                if (r > best.correlation) {
                    best.root = root;
                    best.isMajor = (mode == 0);
                    best.correlation = r;
                }
            }
        }

        best.name = std::string(noteNames[best.root]) +
                     (best.isMajor ? " major" : " minor");
        return best;
    }
};

const char* KrumhanslSchmuckler::noteNames[12] = {
    "C", "C#", "D", "D#", "E", "F",
    "F#", "G", "G#", "A", "A#", "B"
};
```

### 4.2 Temperley Key Profiles

Temperley (1999, 2007) proposed alternative key profiles derived from corpus analysis of common-practice music:

```
Major: [5.0, 2.0, 3.5, 2.0, 4.5, 4.0, 2.0, 4.5, 2.0, 3.5, 1.5, 4.0]
Minor: [5.0, 2.0, 3.5, 4.5, 2.0, 3.5, 2.0, 4.5, 3.5, 2.0, 1.5, 4.0]
```

These profiles tend to perform better on popular and folk music, while Krumhansl profiles perform better on classical. For a VJ engine handling diverse genres, testing both and selecting based on the maximum correlation across both profile sets is a pragmatic approach.

### 4.3 Chroma-Based Key Detection with Template Matching

The pipeline is:

1. **Compute chroma:** From the STFT, map each frequency bin to a pitch class and sum magnitudes. Use a log-frequency filterbank for better frequency resolution at low frequencies (see FEATURES_mfcc_mel.md for filterbank construction).
2. **Accumulate chroma:** Key detection needs long temporal context. Accumulate chroma vectors over 5--30 seconds using exponential smoothing:
   ```
   chroma_acc[i] = alpha * chroma[i] + (1 - alpha) * chroma_acc[i]
   ```
   With alpha = 0.01--0.05 per hop (10 ms hops).
3. **Normalize chroma:** L1 or L2 normalize the accumulated vector.
4. **Correlate against templates:** As in Krumhansl-Schmuckler.

### 4.4 Major/Minor Classification

The relative correlation strength between the best major and best minor key indicates the clarity of the classification. When the top major and top minor key are relative (e.g., C major vs. A minor), their chroma profiles overlap significantly, producing similar correlation values.

**Disambiguation strategies:**
- **Leading tone presence:** If the 7th scale degree has high energy, prefer minor (harmonic minor uses raised 7th).
- **3rd scale degree weighting:** Emphasize the 3rd: a major 3rd strongly indicates major, a minor 3rd indicates minor.
- **Temporal context:** If the piece starts and ends on the same chord, use that chord's root as a strong prior for the tonic.

In practice, for a VJ engine, the distinction between relative major/minor is often unimportant -- they map to similar visual palettes. Report both the primary key and the relative key with their respective correlations.

---

## 5. Chord Recognition

### 5.1 Template-Based Chord Recognition from Chroma

The most practical real-time approach: compare the observed chroma vector against a dictionary of chord templates.

**Common Chord Templates** (binary, 1 = pitch class present):

| Chord Type | Intervals | Template (root = C) |
|------------|-----------|---------------------|
| Major | R, M3, P5 | [1,0,0,0,1,0,0,1,0,0,0,0] |
| Minor | R, m3, P5 | [1,0,0,1,0,0,0,1,0,0,0,0] |
| Diminished | R, m3, d5 | [1,0,0,1,0,0,1,0,0,0,0,0] |
| Augmented | R, M3, A5 | [1,0,0,0,1,0,0,0,1,0,0,0] |
| Dominant 7th | R, M3, P5, m7 | [1,0,0,0,1,0,0,1,0,0,1,0] |
| Major 7th | R, M3, P5, M7 | [1,0,0,0,1,0,0,1,0,0,0,1] |
| Minor 7th | R, m3, P5, m7 | [1,0,0,1,0,0,0,1,0,0,1,0] |
| Sus2 | R, M2, P5 | [1,0,1,0,0,0,0,1,0,0,0,0] |
| Sus4 | R, P4, P5 | [1,0,0,0,0,1,0,1,0,0,0,0] |

**Matching procedure:**

For each of 12 roots x N chord types = 12N candidates:

1. Circularly shift the template by the root offset.
2. Compute cosine similarity with the observed chroma:

```
sim(c, t) = (c . t) / (|c| * |t|)
```

3. The chord with the highest similarity is the recognized chord.

**Weighted templates:** Replace binary 1s with relative weights (root = 1.0, fifth = 0.8, third = 0.7, seventh = 0.5) to reflect typical harmonic energy distribution. This reduces false positives from passing tones.

### 5.2 Real-Time Chord Tracking with Smoothing

Raw frame-by-frame chord recognition produces noisy output. Smoothing strategies:

**Median filtering:** Buffer the last N chord labels (N = 5--15 frames) and output the mode (most frequent). Adds N * hop_size latency but eliminates transient mis-classifications.

**Exponential smoothing on similarities:** Rather than hard-selecting each frame, maintain a running similarity score for each chord:

```cpp
// For each chord candidate c:
smoothedSim[c] = alpha * frameSim[c] + (1 - alpha) * smoothedSim[c];
// Output chord = argmax(smoothedSim)
```

With alpha = 0.1--0.2, this produces stable output that still responds to genuine chord changes within 100--200 ms.

**Hysteresis:** Only change the reported chord if the new candidate's similarity exceeds the current chord's similarity by a margin (e.g., 0.05). This prevents flickering between similar chords (e.g., C major and A minor).

### 5.3 HMM-Based Chord Sequence Modeling

For higher accuracy, model chord sequences as a first-order HMM:

- **States:** 24 chord types (12 major + 12 minor), plus "no chord."
- **Transition probabilities:** Learned from annotated chord datasets (e.g., Beatles, RWC). Common transitions (I--IV, I--V, IV--V) get higher probability. The transition matrix encodes music-theoretic knowledge: chords related by fifths, relative major/minor, and parallel major/minor have higher transition probabilities.
- **Observation probabilities:** Gaussian or von Mises distribution over the cosine similarity between observed chroma and each chord template.
- **Decoding:** Viterbi algorithm (offline) or fixed-lag Viterbi (real-time, ~500 ms lookahead).

The Chordino VAMP plugin (Mauch & Dixon, 2010) implements this approach and is available via the VAMP plugin SDK. For the VJ engine, the simpler template-matching + smoothing approach is recommended unless chord accuracy is paramount.

---

## 6. Harmonic Features

### 6.1 Harmonic Change Detection Function (HCDF)

HCDF measures the rate of harmonic change over time. It is computed as the Euclidean distance (or cosine distance) between successive chroma vectors in Tonnetz space:

```
HCDF(t) = || T(chroma(t)) - T(chroma(t-1)) ||
```

Where T maps a chroma vector to a 6-dimensional Tonnetz representation (see section 6.3).

Peaks in the HCDF indicate chord boundaries. For visualization, HCDF drives transitions: high HCDF triggers visual changes (scene cuts, color shifts, geometry morphs). Low HCDF indicates harmonic stasis, suitable for slow evolution.

### 6.2 Inharmonicity Measure

Inharmonicity quantifies how far the partials of a sound deviate from a perfect harmonic series. For a fundamental f0 and partial frequencies f_k:

```
inharmonicity = (1/K) * sum_{k=1}^{K} |f_k - k * f0| / f0
```

For a perfectly harmonic sound (flute, sine), inharmonicity = 0. For piano strings, inharmonicity increases with pitch due to string stiffness (partials are stretched sharp). For bells and metallic percussion, inharmonicity can exceed 0.1.

**Real-time computation:** After detecting f0 via YIN, scan the spectrum for peaks near k*f0 (k = 2..10). Measure the deviation of each detected peak from the expected harmonic location. Average the deviations.

**Visual mapping:** Inharmonicity maps naturally to visual "roughness" or complexity. Zero inharmonicity produces clean, symmetric visuals; high inharmonicity drives chaotic, asymmetric geometry.

### 6.3 Tonal Centroid (Tonnetz Space)

The Tonnetz is a 6-dimensional space encoding the pitch class relationships of perfect fifths, minor thirds, and major thirds. Following Harte, Sandler & Gasser (2006), the mapping from a 12-dimensional chroma vector c to 6D Tonnetz coordinates is:

```
phi_1 = (1/||c||) * sum_{l=0}^{11} c[l] * sin(l * 7 * pi / 6)    // fifths (sin)
phi_2 = (1/||c||) * sum_{l=0}^{11} c[l] * cos(l * 7 * pi / 6)    // fifths (cos)
phi_3 = (1/||c||) * sum_{l=0}^{11} c[l] * sin(l * 3 * pi / 2)    // minor thirds (sin)
phi_4 = (1/||c||) * sum_{l=0}^{11} c[l] * cos(l * 3 * pi / 2)    // minor thirds (cos)
phi_5 = (1/||c||) * sum_{l=0}^{11} c[l] * sin(l * 4 * pi / 3)    // major thirds (sin)
phi_6 = (1/||c||) * sum_{l=0}^{11} c[l] * cos(l * 4 * pi / 3)    // major thirds (cos)
```

The Tonnetz encodes musical distance: chords that share notes are close in Tonnetz space. The trajectory of the tonal centroid over time traces a path that reflects the harmonic structure of the music.

**Visual mapping:** The 6D Tonnetz maps directly to visual parameters -- e.g., phi_1/phi_2 to x/y position, phi_3/phi_4 to color hue/saturation, phi_5/phi_6 to geometry parameters. Movement in Tonnetz space drives visual animation.

### 6.4 Harmonic-to-Noise Ratio (HNR)

HNR measures the proportion of energy in harmonic partials versus noise:

```
HNR_dB = 10 * log10(E_harmonic / E_noise)
```

**Computation via autocorrelation:**

1. Compute normalized autocorrelation R_norm(tau).
2. Find the peak at the fundamental period: R_peak = R_norm(tau_0).
3. HNR = 10 * log10(R_peak / (1 - R_peak)).

Clean sung vowels: HNR ~ 20--40 dB. Breathy voice: HNR ~ 5--15 dB. Whispered speech: HNR ~ 0--5 dB. Noise: HNR < 0 dB.

**Alternative via spectral method:**
1. Identify harmonic peak locations from f0.
2. Sum energy in narrow bands around each harmonic (harmonic energy).
3. Sum remaining energy (noise energy).
4. HNR = 10 * log10(harmonic / noise).

The spectral method is more accurate but requires prior f0 detection.

**Visual mapping:** HNR controls visual "clarity" -- high HNR produces sharp, defined shapes; low HNR produces blurred, noisy textures. This creates an intuitive correspondence: clean sounds look clean, noisy sounds look noisy.

### 6.5 Fundamental Frequency Stability

Track the variance of f0 over a short window (50--200 ms):

```
f0_stability = 1.0 / (1.0 + var(f0_history))
```

Where var() is the variance of the last N f0 estimates, measured in cents:

```
cents(f1, f2) = 1200 * log2(f1 / f2)
```

High stability (sustained notes, drones) produces f0_stability near 1.0. Vibrato produces moderate instability (~5--7 Hz oscillation, 50--200 cent range). Pitch slides and rapid melodies produce low stability.

**Visual mapping:** Stability controls visual smoothness. Stable pitches hold visual parameters steady; unstable pitches (vibrato, portamento) produce oscillation or drift in visual parameters. The vibrato rate itself (detected via the periodicity of f0 modulation) can modulate visual parameters at the same rate.

---

## 7. Visual Mapping Strategies

### 7.1 Pitch to Color: The Hue Wheel

The most natural mapping places pitch on a color wheel, with octave equivalence:

```
hue = (midi_note % 12) * 30.0  // 0--360 degrees, 30 degrees per semitone
```

This maps C to red (0 deg), E to green (120 deg), G# to blue (240 deg). The mapping is musically satisfying because:
- Fifths (7 semitones apart) map to hue differences of 210 degrees -- nearly complementary colors.
- Major thirds (4 semitones) map to 120-degree separation -- triadic colors.
- Semitones (1 apart) produce 30-degree hue shifts -- perceptible but not jarring.

**Refinements:**
- Map the octave to brightness/value: higher octaves are brighter.
- Map pitch confidence to saturation: low confidence (noise) desaturates to gray.
- Use the continuous f0 (not quantized MIDI) for smooth hue transitions during pitch bends.

```cpp
struct HSV {
    float h, s, v;  // h in [0, 360), s and v in [0, 1]
};

HSV pitchToColor(float freqHz, float confidence, float referenceA4 = 440.f) {
    if (freqHz <= 0.f || confidence < 0.3f)
        return {0.f, 0.f, 0.1f};  // dark gray for unvoiced

    // Continuous MIDI note number
    float midi = 69.f + 12.f * std::log2(freqHz / referenceA4);

    // Hue from pitch class (octave-invariant)
    float pitchClass = std::fmod(midi, 12.f);
    if (pitchClass < 0.f) pitchClass += 12.f;
    float hue = pitchClass * 30.f;  // 0--360

    // Saturation from confidence
    float sat = std::clamp(confidence, 0.f, 1.f);

    // Value from octave (MIDI 36=low, 96=high -> value 0.3--1.0)
    float value = std::clamp((midi - 36.f) / 60.f * 0.7f + 0.3f, 0.3f, 1.0f);

    return {hue, sat, value};
}
```

### 7.2 Key and Chord to Color Palette

Rather than mapping individual pitches, map the detected key to a color palette:

- **Major keys:** Warm, bright palettes (high saturation, high value).
- **Minor keys:** Cool, muted palettes (lower saturation, shifted toward blue/purple).
- **Key root determines base hue:** Same hue wheel as pitch, but applied to the tonic.

For chords, the chord quality modifies the palette:
- **Major chord:** Full saturation, base hue of root.
- **Minor chord:** Desaturated, hue shifted 180 degrees from major.
- **Diminished:** Dark, low value.
- **Augmented:** High saturation, complementary color scheme.
- **Dominant 7th:** Base hue with a secondary accent color at the tritone (6 semitones away = 180 degrees).

### 7.3 Spatial Position

- **Pitch height:** Map f0 linearly or logarithmically to vertical position. Bass at bottom, treble at top. This mirrors the universal metaphor of pitch as spatial height.
- **Tonnetz position:** Map the first two Tonnetz dimensions (fifths circle) to x/y position. Harmonically related chords cluster together; distant modulations produce large spatial jumps.
- **Harmonic change:** HCDF spikes trigger spatial transitions (camera moves, particle bursts, scene changes).

### 7.4 Harmonic Tension to Visual Complexity

Define a composite "tension" metric:

```cpp
float harmonicTension(float hcdf, float inharmonicity, float dissonance,
                      float chordComplexity) {
    // Normalize each to [0, 1] range (application-specific scaling)
    return 0.3f * hcdf + 0.2f * inharmonicity +
           0.3f * dissonance + 0.2f * chordComplexity;
}
```

Where:
- **hcdf:** Harmonic change rate (high = rapid key/chord changes).
- **inharmonicity:** Partial misalignment (high = metallic/bell-like timbres).
- **dissonance:** Computed from the chroma via Plomp-Levelt roughness model (see FEATURES_spectral.md).
- **chordComplexity:** Number of pitch classes active (triads = 3, 7ths = 4, clusters = 5+).

Map tension to:
- **Geometric complexity:** Low tension produces simple shapes (circles, slow waves). High tension produces fractals, jagged edges, particle explosions.
- **Animation speed:** Tension modulates the rate of visual change.
- **Color variance:** Low tension holds a stable palette; high tension introduces rapid hue cycling or multi-hue schemes.

### 7.5 Complete Feature-to-Visual Routing Table

| Feature | Visual Parameter | Mapping | Update Rate |
|---------|-----------------|---------|-------------|
| f0 (pitch) | Hue | Pitch class * 30 deg | Per frame (~23 ms) |
| f0 octave | Brightness | Linear 0.3--1.0 | Per frame |
| Pitch confidence | Saturation | Linear | Per frame |
| Key root | Base palette hue | Root * 30 deg | Slow (~5 sec smoothing) |
| Key mode (maj/min) | Palette warmth/coolness | Categorical | Slow |
| Chord root | Accent color | Root * 30 deg | ~200 ms |
| Chord quality | Saturation/geometry | Categorical table | ~200 ms |
| HCDF | Transition trigger | Threshold detection | Per frame |
| Inharmonicity | Geometric roughness | Linear | Per frame |
| HNR | Shape clarity/blur | Logarithmic | Per frame |
| Tonnetz (6D) | Position, shape params | Direct mapping | Per frame |
| f0 stability | Animation smoothness | Inverse variance | 50--200 ms window |
| Harmonic tension | Overall complexity | Weighted composite | Per frame |

---

## References

- de Cheveigne, A. & Kawahara, H. (2002). YIN, a fundamental frequency estimator for speech and music. JASA 111(4).
- Mauch, M. & Dixon, S. (2014). pYIN: A fundamental frequency estimator using probabilistic threshold distributions. ICASSP.
- McLeod, P. & Wyvill, G. (2005). A smarter way to find pitch. ICMC.
- Kim, J.W. et al. (2018). CREPE: A convolutional representation for pitch estimation. ICASSP.
- Krumhansl, C. & Kessler, E. (1982). Tracing the dynamic changes in perceived tonal organization in a spatial representation of musical keys. Psychological Review 89(4).
- Temperley, D. (1999). What's key for key? The Krumhansl-Schmuckler key-finding algorithm reconsidered. Music Perception 17(1).
- Harte, C., Sandler, M. & Gasser, M. (2006). Detecting harmonic change in musical audio. ACM Multimedia Workshop on Audio and Music Computing.
- Mauch, M. & Dixon, S. (2010). Simultaneous estimation of chords and musical context from audio. IEEE TASLP 18(6).
- Salamon, J. & Gomez, E. (2012). Melody extraction from polyphonic music signals using pitch contour characteristics. IEEE TASLP 20(6).
- Bittner, R. et al. (2022). A lightweight instrument-agnostic model for polyphonic note transcription and multipitch estimation. ICASSP (Basic Pitch).
