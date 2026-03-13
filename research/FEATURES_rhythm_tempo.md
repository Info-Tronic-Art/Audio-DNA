# Rhythm & Tempo Analysis for Real-Time Music Visualization

> **Scope**: Onset detection, BPM estimation, beat tracking, downbeat/meter analysis, and advanced rhythm features — all constrained to causal (real-time) operation for a VJ engine.
>
> **Cross-references**: [FEATURES_transients_texture.md](FEATURES_transients_texture.md) | [FEATURES_amplitude_dynamics.md](FEATURES_amplitude_dynamics.md) | [LIB_aubio.md](LIB_aubio.md) | [LIB_essentia.md](LIB_essentia.md) | [VIDEO_feature_to_visual_mapping.md](VIDEO_feature_to_visual_mapping.md) | [REF_latency_numbers.md](REF_latency_numbers.md)

---

## 1. Onset Detection

An **onset** is the beginning of a musical event — the transient attack of a note, drum hit, or percussive sound. Onset detection is the foundation of all rhythm analysis: BPM estimation, beat tracking, and meter detection all depend on a reliable onset stream.

### 1.1 The Onset Detection Pipeline

```
Audio Buffer (N samples)
    │
    ▼
STFT (hop H, window W)
    │
    ▼
Onset Detection Function (ODF)   ← spectral flux, HFC, complex domain, etc.
    │
    ▼
Onset Strength Envelope (OSE)    ← continuous non-negative signal
    │
    ▼
Peak Picking (adaptive threshold)
    │
    ▼
Onset Times (sample-accurate)
```

The STFT is computed with typical parameters: window size W = 1024 or 2048 samples, hop size H = 256 or 512 samples, at 44100 Hz sample rate. The hop size determines the temporal resolution of the ODF: at H=512 and sr=44100, each ODF sample spans ~11.6 ms.

### 1.2 Onset Detection Functions (ODFs)

#### 1.2.1 Spectral Flux

The most widely used ODF. Measures the positive increase in magnitude across frequency bins between consecutive frames.

```
SF(n) = sum_k( H( |X(n,k)| - |X(n-1,k)| ) )
```

Where `H(x) = (x + |x|) / 2` is the half-wave rectifier (only positive differences count — energy appearing, not disappearing). `|X(n,k)|` is the magnitude of bin k at frame n.

**Why half-wave rectification matters**: Without it, a note ending (energy disappearing) would register as an onset. The rectifier ensures only energy *arrivals* trigger onsets.

**Normalized variant**: Divide by the number of bins K, or by the total spectral energy of frame n, to make the function invariant to overall level.

```cpp
// C++ implementation: spectral flux onset detection
#include <vector>
#include <cmath>
#include <algorithm>

class SpectralFluxODF {
public:
    explicit SpectralFluxODF(int fft_size)
        : num_bins_(fft_size / 2 + 1)
        , prev_magnitude_(num_bins_, 0.0f)
    {}

    /// Feed one STFT frame (magnitude spectrum), returns onset strength.
    /// @param magnitude  pointer to num_bins_ floats (magnitude of each bin)
    /// @return           non-negative onset detection function value
    float process(const float* magnitude) {
        float flux = 0.0f;

        for (int k = 0; k < num_bins_; ++k) {
            float diff = magnitude[k] - prev_magnitude_[k];
            // Half-wave rectification: only positive differences
            if (diff > 0.0f) {
                flux += diff;
            }
            prev_magnitude_[k] = magnitude[k];
        }

        return flux;
    }

    void reset() {
        std::fill(prev_magnitude_.begin(), prev_magnitude_.end(), 0.0f);
    }

private:
    int num_bins_;
    std::vector<float> prev_magnitude_;
};
```

#### 1.2.2 High Frequency Content (HFC)

Weights each bin by its frequency index before summing. High frequencies carry more transient energy (attacks have broadband spectra with strong HF content), so HFC is naturally more sensitive to percussive onsets.

```
HFC(n) = sum_k( k * |X(n,k)|^2 )
```

Or the difference variant:

```
HFC_diff(n) = sum_k( k * H( |X(n,k)|^2 - |X(n-1,k)|^2 ) )
```

The weighting by bin index `k` acts as an approximate linear frequency weighting (since bin k corresponds to frequency `k * sr / N`). This makes HFC extremely responsive to hi-hats, snare rims, and other high-frequency transients, but less sensitive to bass drum onsets or tonal note beginnings.

#### 1.2.3 Complex Domain

Uses both magnitude and phase information from the STFT (Duxbury et al., 2003). The idea: predict what each bin's complex value *should* be based on the previous two frames, then measure deviation from that prediction.

```
Predicted:  X_hat(n,k) = |X(n-1,k)| * exp(j * (2*phase(n-1,k) - phase(n-2,k)))
CD(n) = sum_k( | X(n,k) - X_hat(n,k) | )
```

The prediction assumes constant magnitude and constant phase advance (i.e., a stationary sinusoid). Any deviation — whether magnitude change (a new note) or phase discontinuity (a transient) — contributes to the ODF.

**Advantage**: Detects soft tonal onsets that pure magnitude-based methods miss, because even a pitch change with no amplitude change causes a phase deviation.

**Disadvantage**: Phase is noisy in low-energy bins, so the function can be noisy. Common mitigation: weight each bin's contribution by its magnitude, or threshold out low-energy bins.

#### 1.2.4 Phase Deviation

Measures only the phase component, ignoring magnitude entirely.

```
PD(n) = sum_k( | phase(n,k) - 2*phase(n-1,k) + phase(n-2,k) | )
```

This is the second difference of the instantaneous phase — the "phase acceleration." A stationary sinusoid has zero phase acceleration; an onset disrupts phase coherence. Phase deviation is highly sensitive to pitched onsets but susceptible to noise in low-energy regions.

#### 1.2.5 Weighted Phase Deviation

Combines phase deviation with magnitude weighting to suppress noise from low-energy bins:

```
WPD(n) = sum_k( |X(n,k)| * | phase(n,k) - 2*phase(n-1,k) + phase(n-2,k) | )
```

This is arguably the best single ODF for mixed content — it captures both percussive (magnitude) and tonal (phase) onsets, while the magnitude weighting suppresses the phase noise that plagues pure PD.

#### 1.2.6 Modified Kullback-Leibler Divergence

Treats consecutive magnitude spectra as probability distributions and measures divergence:

```
MKL(n) = sum_k( log(1 + |X(n,k)| / (|X(n-1,k)| + epsilon)) )
```

The logarithm compresses large differences and amplifies small ones, making MKL sensitive to subtle spectral changes. The epsilon prevents division by zero. MKL is effective for detecting onsets in polyphonic music where multiple instruments overlap.

### 1.3 ODF Comparison Table

| ODF | Accuracy (F1) | Latency | CPU Cost | Best For | Weaknesses |
|-----|---------------|---------|----------|----------|------------|
| **Spectral Flux** | 0.82-0.87 | 1 hop | Very Low | General purpose, electronic, rock | Misses soft tonal onsets |
| **HFC** | 0.80-0.85 | 1 hop | Very Low | Percussion, hi-hats, fast transients | Insensitive to bass/tonal onsets |
| **Complex Domain** | 0.85-0.90 | 2 hops | Medium | Mixed/polyphonic, classical, jazz | Phase noise in quiet sections |
| **Phase Deviation** | 0.75-0.82 | 2 hops | Low | Pitched onsets, monophonic | Very noisy, poor for drums |
| **Weighted Phase Dev** | 0.84-0.89 | 2 hops | Medium | Mixed content, best all-rounder | Slightly higher latency |
| **Modified KL** | 0.83-0.88 | 1 hop | Low | Polyphonic, dense textures | Can miss isolated transients |

**Notes on latency**: "1 hop" means the ODF requires the current frame plus the previous frame. "2 hops" means it needs 3 frames (current + 2 previous) for the phase second-difference. At H=512, sr=44100: 1 hop = 11.6 ms additional latency, 2 hops = 23.2 ms. The STFT window itself adds W/2 latency (~11.6 ms at W=1024).

**F1 scores** are approximate ranges from published evaluations on MIREX onset detection datasets. Exact values depend on genre, peak-picking parameters, and evaluation tolerance window (typically 50 ms).

### 1.4 Peak Picking from the Onset Function

The raw ODF is a continuous signal. To extract discrete onset times, we need **peak picking** — identifying local maxima that exceed a threshold.

#### 1.4.1 Adaptive Threshold (Median-Based)

A fixed threshold fails because the ODF magnitude varies with musical dynamics. The standard approach uses a local median plus a constant offset:

```
threshold(n) = delta + lambda * median( ODF[n-M : n+M] )
```

Where:
- `delta` = absolute minimum threshold (prevents false positives in silence)
- `lambda` = multiplier on the local median (typically 0.5-1.5)
- `M` = half-width of the median window (typically 3-10 frames)

For causal (real-time) operation, the future portion of the window is unavailable, so we use a one-sided window:

```
threshold_causal(n) = delta + lambda * median( ODF[n-2M : n] )
```

#### 1.4.2 Peak Picking Algorithm

```cpp
struct OnsetPeakPicker {
    float delta    = 0.01f;   // absolute floor
    float lambda   = 0.8f;    // median multiplier
    int   median_w = 7;       // median window frames (causal)
    int   wait     = 5;       // minimum frames between onsets (~58 ms at H=512)

    std::deque<float> history;
    int frames_since_last = 0;

    bool is_onset(float odf_value) {
        history.push_back(odf_value);
        if ((int)history.size() > median_w)
            history.pop_front();

        frames_since_last++;

        // Compute median of history
        std::vector<float> sorted(history.begin(), history.end());
        std::sort(sorted.begin(), sorted.end());
        float med = sorted[sorted.size() / 2];

        float thresh = delta + lambda * med;

        // Must exceed threshold AND be a local max (greater than previous)
        bool above = odf_value > thresh;
        bool is_peak = history.size() >= 2
                    && odf_value >= history[history.size() - 2];
        bool past_wait = frames_since_last >= wait;

        if (above && is_peak && past_wait) {
            frames_since_last = 0;
            return true;
        }
        return false;
    }
};
```

The `wait` parameter enforces a minimum inter-onset interval (IOI), preventing double-triggers on a single event. At 5 frames with H=512, sr=44100, the minimum IOI is ~58 ms, corresponding to a maximum onset rate of ~17 onsets/second — fast enough for 32nd notes at 130 BPM.

### 1.5 Onset Strength Envelope

The **onset strength envelope** (OSE) is the smoothed, non-negative ODF signal before peak picking. It serves as input to tempo estimation and beat tracking algorithms. Typical processing:

1. Compute raw ODF (spectral flux or similar)
2. Half-wave rectify (take `max(0, odf)`)
3. Low-pass filter or smooth with a short Gaussian kernel (sigma ~ 3-5 frames)
4. Normalize to [0, 1] using a running max with decay

The OSE represents "onset-ness" as a continuous function of time. Its periodicity reflects the tempo, and its peaks correspond to individual onsets.

---

## 2. BPM Detection Algorithms

BPM (beats per minute) estimation finds the dominant periodicity in the onset strength envelope. The key challenge: multiple periodicities coexist (subdivisions, beat, bar), and we need to identify the one humans would tap along to.

### 2.1 Autocorrelation of Onset Envelope

The simplest tempo estimation method. The autocorrelation of the OSE reveals periodicities as peaks:

```
R(lag) = sum_n( OSE(n) * OSE(n + lag) )
```

The lag of the highest peak (within the valid BPM range) gives the tempo estimate.

```
BPM = 60.0 * sr / (hop_size * peak_lag)
```

**Implementation detail**: Restrict the lag search to the range corresponding to 60-200 BPM:

```
lag_min = 60 * sr / (hop_size * 200)  // ~20 frames at sr=44100, H=512
lag_max = 60 * sr / (hop_size * 60)   // ~86 frames
```

**Strengths**: Simple, low CPU, works well for music with clear periodic beats.
**Weaknesses**: Ambiguous between half-tempo and double-tempo (the autocorrelation has peaks at multiples of the fundamental lag). No phase information.

### 2.2 Comb Filter Bank (Scheirer 1998)

Eric Scheirer's 1998 method uses a bank of resonant comb filters, each tuned to a candidate tempo. The filter with the highest output energy wins.

Each comb filter for tempo T has impulse response peaks at intervals of T, 2T, 3T, ... A periodic signal at exactly tempo T will resonate maximally in that filter.

```
y_T(n) = alpha * y_T(n - period_T) + (1 - alpha) * OSE(n)
```

Where `period_T = round(60 * sr / (hop_size * T))` is the period in ODF frames for tempo T BPM, and `alpha` controls the resonance sharpness (typically 0.8-0.95).

The output energy `E_T = sum(y_T(n)^2)` over a window is computed for each candidate tempo T. The tempo with maximum E_T is selected.

**Strengths**: Naturally causal, low latency (accumulates evidence over time), handles tempo changes reasonably.
**Weaknesses**: Quantized tempo resolution (each filter is tuned to one BPM), moderate CPU with many candidate tempos, still ambiguous at octave errors.

### 2.3 Fourier Transform of Onset Envelope (Tempo Spectrum)

Take the magnitude spectrum of a windowed segment of the OSE. Peaks in this "tempo spectrum" correspond to periodicities.

```
Tempo_Spectrum(f) = |FFT( OSE[n-L : n] * window )|
```

Convert frequency bins to BPM: `BPM = f * 60` where f is in Hz. A 6-second window of OSE sampled at sr/hop ≈ 86 Hz gives frequency resolution of ~0.17 Hz = ~10 BPM, which is too coarse. Solutions:
- Use longer windows (10-15 seconds) for ~1-2 BPM resolution
- Zero-pad the FFT for interpolated resolution
- Use parabolic interpolation around the peak

**Strengths**: Clean frequency-domain picture, easy to visualize, good resolution with sufficient window length.
**Weaknesses**: Long windows add latency and reduce responsiveness to tempo changes. Non-causal without modification.

### 2.4 BPM Estimation Comparison

| Method | Accuracy (±2 BPM) | Convergence Time | Latency | CPU | Notes |
|--------|-------------------|------------------|---------|-----|-------|
| **Autocorrelation** | ~80% | 3-5 sec | Low | Very Low | Octave errors common |
| **Comb Filter Bank** | ~83% | 2-4 sec | Very Low | Medium | Best for real-time; inherently causal |
| **FFT of OSE** | ~85% | 6-15 sec | High | Low | Best accuracy but slow convergence |
| **Combined** | ~90% | 4-8 sec | Medium | Medium | Use comb filter + autocorrelation verification |

### 2.5 Half-Tempo / Double-Tempo Problem

The most persistent problem in tempo estimation. A song at 140 BPM has strong periodicity at 70 BPM (every other beat) and 280 BPM (every subdivision). Strategies:

1. **Perceptual weighting**: Apply a Gaussian or Rayleigh weighting centered on ~120 BPM to bias toward the most common "tapped" tempo range. Multiply the autocorrelation or comb filter output by this weighting.

2. **Resonance ratio check**: If the strongest peak is at lag L, check if the peak at 2L is nearly as strong. If so, the true tempo is likely at L (the faster one), since the slower periodicity is a harmonic.

3. **Genre-specific priors**: Electronic/dance music strongly clusters at 120-130 BPM. Hip-hop at 80-100. Metal at 160-200. If the genre is known, narrow the prior.

### 2.6 C++ BPM Estimation (Autocorrelation Method)

```cpp
#include <vector>
#include <cmath>
#include <algorithm>
#include <numeric>

class TempoEstimator {
public:
    TempoEstimator(float sample_rate, int hop_size,
                   float min_bpm = 60.0f, float max_bpm = 200.0f,
                   int history_frames = 512)
        : sr_(sample_rate)
        , hop_(hop_size)
        , history_frames_(history_frames)
    {
        // Convert BPM range to lag range (in ODF frames)
        float frames_per_sec = sr_ / hop_;
        lag_min_ = (int)(60.0f * frames_per_sec / max_bpm);
        lag_max_ = (int)(60.0f * frames_per_sec / min_bpm);

        // Perceptual tempo weighting: Rayleigh centered at ~120 BPM
        float center_lag = 60.0f * frames_per_sec / 120.0f;
        tempo_weight_.resize(lag_max_ + 1);
        for (int l = lag_min_; l <= lag_max_; ++l) {
            float x = (float)l / center_lag;
            tempo_weight_[l] = x * std::exp(-0.5f * x * x);
        }

        ose_buffer_.reserve(history_frames_);
    }

    /// Feed one onset strength value per STFT hop.
    /// Returns estimated BPM (0 if insufficient data).
    float process(float onset_strength) {
        ose_buffer_.push_back(onset_strength);
        if ((int)ose_buffer_.size() > history_frames_)
            ose_buffer_.erase(ose_buffer_.begin());

        if ((int)ose_buffer_.size() < lag_max_ + 1)
            return 0.0f;

        // Autocorrelation over valid lag range
        int N = (int)ose_buffer_.size();
        float best_score = -1.0f;
        int   best_lag   = lag_min_;

        for (int lag = lag_min_; lag <= lag_max_; ++lag) {
            float sum = 0.0f;
            int count = N - lag;
            for (int i = 0; i < count; ++i) {
                sum += ose_buffer_[i] * ose_buffer_[i + lag];
            }
            float acf = sum / count;  // normalized

            // Apply perceptual weighting
            float score = acf * tempo_weight_[lag];

            if (score > best_score) {
                best_score = score;
                best_lag = lag;
            }
        }

        // Convert lag to BPM
        float frames_per_sec = sr_ / hop_;
        float bpm = 60.0f * frames_per_sec / best_lag;

        return bpm;
    }

private:
    float sr_;
    int   hop_;
    int   lag_min_, lag_max_;
    int   history_frames_;
    std::vector<float> ose_buffer_;
    std::vector<float> tempo_weight_;
};
```

---

## 3. Beat Tracking

BPM estimation tells us *how fast*; beat tracking tells us *where* each beat falls. A beat tracker outputs a sequence of beat times and, critically for visualization, a continuous **beat phase** signal.

### 3.1 Causal vs Non-Causal

Non-causal beat trackers (e.g., Ellis 2007 dynamic programming) process the entire audio file and find the globally optimal beat sequence. They achieve the highest accuracy but are useless for real-time.

**For a VJ engine, only causal (online) beat trackers apply.** These process audio frame-by-frame and emit beat predictions with minimal latency. The trade-off is reduced accuracy (no future lookahead) and potential instability during ambiguous sections.

### 3.2 Dynamic Programming Beat Tracker (Ellis 2007) — Adapted for Real-Time

Ellis's original algorithm finds the beat sequence that maximizes:

```
Score = sum_i( OSE(beat_i) ) + alpha * sum_i( -( (beat_i - beat_{i-1}) - period )^2 )
```

The first term rewards placing beats on onset peaks. The second term penalizes deviations from the expected inter-beat interval (IBI = 60/BPM seconds). `alpha` controls the balance between following onsets and maintaining tempo stability.

For real-time adaptation:
- Run DP over a sliding window (last 4-8 seconds)
- Use the last few beats from the previous window as constraints
- Re-run every N beats to correct drift

**Latency**: Window length + computation time. Typically 2-4 seconds of latency, which is problematic for tight visual sync. Useful as a "background" tracker that corrects a faster causal tracker.

### 3.3 Particle Filter Beat Tracker

A probabilistic approach that maintains a population of "particles," each representing a hypothesis about the current beat phase and tempo.

```
Each particle: (phase, period, weight)
- phase:  position within the current beat [0.0, 1.0)
- period: inter-beat interval in frames
- weight: probability of this hypothesis
```

**Update cycle** (every ODF frame):

1. **Predict**: Advance each particle's phase by `1.0 / period`. If phase wraps past 1.0, a beat is predicted.
2. **Weight**: Multiply each particle's weight by the likelihood of the current OSE value given its phase. Particles near phase=0 (beat position) should see high OSE; particles mid-beat should see lower OSE.
3. **Resample**: If effective particle count drops below threshold, resample from weighted distribution (systematic resampling).
4. **Report**: The weighted average of all particles gives the current phase and tempo estimate. A beat occurs when the mode phase crosses 0.

**Typical particle count**: 500-2000. At 2000 particles and ~86 frames/sec (H=512), this is ~172,000 particle updates/sec — trivially cheap on modern CPUs.

**Strengths**: Naturally handles tempo changes, maintains multiple hypotheses (e.g., half-tempo vs full-tempo), provides uncertainty estimates.
**Weaknesses**: Can be "jittery" — the beat phase fluctuates frame-to-frame as particle weights shift. Needs careful tuning of the transition model noise.

### 3.4 Dynamic Bayesian Network (DBN) Beat Tracker (Bock et al.)

The state-of-the-art causal beat tracker, used in `madmom`. Models beat tracking as inference in a DBN with hidden states for beat position and tempo:

- **State space**: `(position, tempo)` where position is quantized (e.g., 0 to period-1) and tempo is a discrete set of candidate tempos.
- **Observation model**: `P(OSE | position)` — high OSE expected at position=0, low elsewhere.
- **Transition model**: `P(state_t | state_{t-1})` — position increments by 1 each frame; tempo can change slowly.
- **Inference**: Forward algorithm (Viterbi or forward-filtering) computes the most likely state sequence causally.

The DBN tracker in `madmom` uses a neural network (RNN) to compute a beat activation function instead of a traditional ODF, achieving F-measure scores of ~0.90+ on standard datasets.

**For C++ real-time use**: The BeatNet or `madmom`-style RNN can be exported to ONNX and run with ONNX Runtime. The DBN inference itself is straightforward to implement in C++; the neural network component is the heavy part.

### 3.5 Multi-Agent Beat Tracker

Multiple independent beat-tracking agents run in parallel, each with different tempo/phase hypotheses. Agents accumulate score by "hitting" onsets. Periodically, the highest-scoring agent is selected as the current estimate.

**Implementation sketch**:
- Spawn 20-50 agents with tempos uniformly spanning 60-200 BPM
- Each agent maintains a phase counter and accumulates OSE values at its predicted beat times
- Agents that consistently miss onsets lose score and are replaced (respawned with new hypotheses derived from recent onset patterns)
- The top-scoring agent's phase and tempo become the output

**Strengths**: Robust to tempo changes (some agent will eventually match). Conceptually simple.
**Weaknesses**: Convergence can be slow. Transitions between agents cause phase discontinuities.

### 3.6 Beat Phase (0.0-1.0)

For visualization, the most useful output is a continuous **beat phase** signal, not just discrete beat times.

```
beat_phase(t) = (t - last_beat_time) / inter_beat_interval
```

Where `beat_phase ∈ [0.0, 1.0)`: 0.0 at the beat, 0.5 at the midpoint, approaching 1.0 just before the next beat.

**Usage in visuals**:
- Map phase 0.0 to maximum visual impact (flash, scale pulse, color change)
- Use `sin(beat_phase * 2 * PI)` for smooth pulsing
- Use `1.0 - beat_phase` for decay envelope (bright at beat, fades to next beat)
- Use `beat_phase < 0.1` as a boolean "on the beat" flag

### 3.7 Beat Confidence Measure

Not all beats are equally reliable. A beat confidence metric helps visuals gracefully degrade when tracking is uncertain:

- **Onset coincidence**: Is there a strong onset near the predicted beat? Confidence += if OSE at beat time exceeds threshold.
- **Tempo stability**: Has the estimated tempo been consistent over the last N beats? Confidence += if variance of recent IBIs is low.
- **Phase consistency**: Are beats landing at regular intervals? Large phase corrections indicate low confidence.

A practical confidence score:

```cpp
float beat_confidence(float ose_at_beat, float ose_median,
                      float ibi_variance, float expected_ibi) {
    // Onset strength relative to local median
    float onset_score = std::min(1.0f, ose_at_beat / (ose_median + 1e-6f));

    // Tempo stability: low variance = high confidence
    float tempo_score = std::exp(-ibi_variance / (0.05f * expected_ibi * expected_ibi));

    return 0.6f * onset_score + 0.4f * tempo_score;
}
```

---

## 4. Downbeat & Meter

### 4.1 Downbeat Detection

The **downbeat** (beat 1 of a bar) is distinguished from other beats by musical accent patterns:

- **Bass accent**: Bass notes (low-frequency energy) are typically strongest on beat 1.
- **Harmonic change**: Chord changes most frequently align with the downbeat.
- **Spectral centroid drop**: The downbeat often has more low-frequency content (bass + kick together), lowering the spectral centroid.
- **Loudness accent**: Beat 1 is often louder than beats 2-4.

**Algorithm**: Split the spectrum into sub-bands (e.g., 3 bands: low < 250 Hz, mid 250-2000 Hz, high > 2000 Hz). Compute onset strength in each band. Look for the cyclic accent pattern:

```
For 4/4 meter:  strong - weak - medium - weak
                 beat1    beat2   beat3    beat4
```

Use autocorrelation of the low-band OSE at lags of 2x, 3x, 4x the beat period to determine bar length.

### 4.2 Meter Estimation

Once beats are tracked, meter estimation determines how many beats per bar:

| Meter | Bar Length | Accent Pattern | Common Genres |
|-------|-----------|----------------|---------------|
| 4/4 | 4 beats | S-w-M-w | Pop, rock, electronic, hip-hop |
| 3/4 | 3 beats | S-w-w | Waltz, some folk |
| 6/8 | 6 eighth-notes (2 dotted quarters) | S-w-w-M-w-w | Irish jig, some metal |
| 5/4 | 5 beats | S-w-M-w-w | Progressive rock, jazz |
| 7/8 | 7 eighth-notes | Variable | Balkan, prog |

**Practical approach for a VJ engine**: Default to 4/4 (covers >85% of popular music). Provide a manual override. Attempt automatic detection by testing bar-length hypotheses (3, 4, 5, 6, 7 beats) and scoring each by the periodicity of the low-band accent pattern at that bar length.

### 4.3 Bar Tracking and Bar Phase

Analogous to beat phase, **bar phase** gives position within the current bar:

```
bar_phase(t) = beat_within_bar / beats_per_bar + beat_phase / beats_per_bar
```

For 4/4: bar_phase ranges [0.0, 1.0), advancing by 0.25 per beat. This is useful for visuals that operate on multi-beat cycles — color palette rotations, scene transitions, particle system resets.

---

## 5. Advanced Rhythm Features

### 5.1 Onset Rate (Onsets Per Second)

A simple but powerful feature: count onsets within a sliding window.

```
onset_rate(t) = count(onsets in [t - window, t]) / window
```

With a 1-second window, onset_rate directly reflects rhythmic density. Values:
- ~2-4: sparse, half-time feel
- ~4-8: typical pop/rock
- ~8-16: fast drumming, breakbeats, fills
- ~16+: drum rolls, glitch

**Use in visuals**: Map onset_rate to particle density, complexity of visual patterns, or visual "business" parameter.

### 5.2 Tempo Change Detection

Monitor the estimated BPM over time. A tempo change is detected when:

```
|BPM(t) - BPM(t - delta)| > threshold
```

Where delta is a comparison window (e.g., 4-8 seconds). Threshold is typically 3-5 BPM to avoid false positives from estimation noise.

**Types of tempo change**:
- **Abrupt**: DJ transition, song section change. BPM jumps instantly.
- **Gradual**: Live performance accelerando/ritardando. BPM drifts continuously.
- **Tempo modulation**: Intentional half-time / double-time sections.

For the comb filter bank, tempo change manifests as the dominant filter shifting. Track the centroid of the comb filter output distribution over time for smooth tempo change detection.

### 5.3 Groove / Swing Quantification

**Swing** is the systematic deviation of even-numbered subdivisions from a strict grid. In a "swung" rhythm, the second eighth note of each pair is delayed:

```
swing_ratio = duration(first_eighth) / duration(second_eighth)
```

- Straight: ratio = 1.0 (50/50 split)
- Light swing: ratio ≈ 1.2 (55/45)
- Medium swing: ratio ≈ 1.5 (60/40)
- Heavy swing: ratio ≈ 2.0 (67/33, triplet feel)

**Measurement**: After beat tracking, detect eighth-note onsets between beats. Measure the timing offset of each even-numbered onset from the exact midpoint between beats.

```
swing_offset = actual_midpoint_onset_time - expected_midpoint
groove_swing = mean(swing_offset) / beat_period
```

Positive values = laid-back / swung. Negative values = pushed / rushed.

**Microtiming / groove template**: Beyond swing, individual instruments may systematically deviate from the grid in characteristic ways (e.g., bass slightly behind the beat, hi-hats slightly ahead). This creates the "feel" or "pocket" of a performance.

### 5.4 Pulse Clarity

A measure of how clearly the periodic pulse (beat) can be perceived. Computed as the peak-to-mean ratio of the autocorrelation at the beat lag:

```
pulse_clarity = ACF(beat_lag) / mean(ACF)
```

High pulse clarity: steady 4-on-the-floor kick drum pattern. Electronic dance music typically scores highest.
Low pulse clarity: free jazz, ambient, rubato passages, spoken word.

**Use in visuals**: When pulse clarity is low, switch from beat-synced visuals to amplitude/spectral-driven visuals. This prevents visuals from looking "wrong" when the beat tracker is struggling.

### 5.5 Rhythmic Pattern Matching

For advanced VJ applications, classify the rhythmic pattern of the current section:

- **Four-on-the-floor**: Kick on every beat (EDM, disco, house)
- **Backbeat**: Snare on beats 2 & 4 (rock, pop, funk)
- **Breakbeat**: Syncopated drum pattern (jungle, hip-hop, breakbeat)
- **Halftime**: Snare on beat 3 only (trap, dubstep drops)

**Detection approach**: Compute a beat-synchronous spectrogram — warp the spectrogram so each beat occupies the same number of frames. Then look at the average energy pattern within a beat cycle:

- Four-on-the-floor: low-band energy peaks on every beat
- Backbeat: mid-band (snare ~200-1000 Hz) energy peaks on beats 2 & 4
- Halftime: mid-band peak on beat 3 (or every other beat 2&4 pattern)

This can be implemented as a set of template correlations or a small classifier.

---

## 6. Aubio Implementation

[aubio](https://aubio.org/) is the go-to C library for real-time onset, tempo, and beat detection. It is lightweight, BSD-licensed, and designed for low-latency operation.

### 6.1 `aubio_onset` — Onset Detection

```c
#include <aubio/aubio.h>

// Setup
uint_t win_s = 1024;        // window size
uint_t hop_s = 512;         // hop size
uint_t samplerate = 44100;

aubio_onset_t *onset = new_aubio_onset("default", win_s, hop_s, samplerate);

// Configure
aubio_onset_set_threshold(onset, 0.3f);   // peak picking threshold
aubio_onset_set_silence(onset, -70.0f);   // silence threshold in dB
aubio_onset_set_minioi_ms(onset, 50.0f);  // minimum inter-onset interval (ms)

// Per-hop processing
fvec_t *input  = new_fvec(hop_s);
fvec_t *output = new_fvec(1);

// ... fill input->data with hop_s samples ...

aubio_onset_do(onset, input, output);

if (output->data[0] != 0) {
    uint_t onset_sample = aubio_onset_get_last(onset);
    float onset_seconds = aubio_onset_get_last_s(onset);
    // An onset was detected!
}

// Access the onset detection function value (before peak picking)
float odf_value = aubio_onset_get_descriptor(onset);

// Access the current adaptive threshold
float threshold_value = aubio_onset_get_thresholded_descriptor(onset);

// Cleanup
del_aubio_onset(onset);
del_fvec(input);
del_fvec(output);
```

**Onset method strings**: `"default"` (HFC), `"energy"`, `"hfc"`, `"complex"`, `"phase"`, `"wphase"`, `"specdiff"`, `"kl"`, `"mkl"`, `"specflux"`.

### 6.2 `aubio_tempo` — Tempo and Beat Tracking

```c
aubio_tempo_t *tempo = new_aubio_tempo("default", win_s, hop_s, samplerate);

// Configure
aubio_tempo_set_threshold(tempo, 0.3f);
aubio_tempo_set_silence(tempo, -70.0f);

// Per-hop processing
aubio_tempo_do(tempo, input, output);

if (output->data[0] != 0) {
    // A beat was detected at this hop
    float beat_time_s  = aubio_tempo_get_last_s(tempo);
    uint_t beat_sample = aubio_tempo_get_last(tempo);
}

// Current BPM estimate
float bpm = aubio_tempo_get_bpm(tempo);

// Confidence (0.0 - 1.0)
float confidence = aubio_tempo_get_confidence(tempo);

// Beat period in seconds
float period = aubio_tempo_get_period_s(tempo);

// Cleanup
del_aubio_tempo(tempo);
```

### 6.3 Integration Pattern: Real-Time Audio Callback

```cpp
// Integrate aubio into a real-time audio callback (e.g., PortAudio, JACK, CoreAudio)

class AubioRhythmAnalyzer {
public:
    AubioRhythmAnalyzer(int sr = 44100, int hop = 512, int win = 1024)
        : sr_(sr), hop_(hop)
    {
        onset_ = new_aubio_onset("specflux", win, hop, sr);
        aubio_onset_set_threshold(onset_, 0.3f);
        aubio_onset_set_minioi_ms(onset_, 30.0f);

        tempo_ = new_aubio_tempo("default", win, hop, sr);
        aubio_tempo_set_threshold(tempo_, 0.3f);

        in_  = new_fvec(hop);
        out_onset_ = new_fvec(1);
        out_tempo_ = new_fvec(1);
    }

    ~AubioRhythmAnalyzer() {
        del_aubio_onset(onset_);
        del_aubio_tempo(tempo_);
        del_fvec(in_);
        del_fvec(out_onset_);
        del_fvec(out_tempo_);
    }

    struct RhythmState {
        bool  onset_detected = false;
        float onset_strength = 0.0f;
        bool  beat_detected  = false;
        float bpm            = 0.0f;
        float beat_confidence = 0.0f;
        float beat_phase     = 0.0f;     // [0.0, 1.0)
        double last_beat_time = 0.0;     // seconds
        double beat_period    = 0.5;     // seconds (120 BPM default)
    };

    /// Call once per hop_size samples. Thread-safe if called from one thread.
    RhythmState process(const float* samples, double current_time) {
        // Copy samples into aubio buffer
        for (int i = 0; i < hop_; ++i)
            in_->data[i] = samples[i];

        RhythmState state;

        // Onset detection
        aubio_onset_do(onset_, in_, out_onset_);
        state.onset_detected = (out_onset_->data[0] != 0);
        state.onset_strength = aubio_onset_get_descriptor(onset_);

        // Beat tracking
        aubio_tempo_do(tempo_, in_, out_tempo_);
        state.beat_detected = (out_tempo_->data[0] != 0);
        state.bpm = aubio_tempo_get_bpm(tempo_);
        state.beat_confidence = aubio_tempo_get_confidence(tempo_);

        if (state.beat_detected) {
            last_beat_time_ = current_time;
            if (state.bpm > 0.0f)
                beat_period_ = 60.0 / state.bpm;
        }

        state.last_beat_time = last_beat_time_;
        state.beat_period = beat_period_;

        // Compute beat phase
        if (beat_period_ > 0.0) {
            double elapsed = current_time - last_beat_time_;
            state.beat_phase = (float)std::fmod(elapsed / beat_period_, 1.0);
            if (state.beat_phase < 0.0f) state.beat_phase += 1.0f;
        }

        return state;
    }

private:
    int sr_, hop_;
    aubio_onset_t *onset_;
    aubio_tempo_t *tempo_;
    fvec_t *in_, *out_onset_, *out_tempo_;
    double last_beat_time_ = 0.0;
    double beat_period_    = 0.5;
};
```

### 6.4 Aubio Parameter Tuning Guide

| Parameter | Default | Range | Effect |
|-----------|---------|-------|--------|
| `threshold` (onset) | 0.3 | 0.05-0.9 | Lower = more sensitive, more false positives |
| `threshold` (tempo) | 0.3 | 0.1-0.8 | Lower = more responsive, less stable |
| `silence` | -70 dB | -90 to -20 | Below this level, no onsets/beats reported |
| `minioi_ms` | 50 ms | 10-200 | Min gap between onsets; prevents double-triggers |
| `onset method` | "default" (HFC) | see list | `"specflux"` recommended for general use |

---

## 7. Syncing Video to Beats

The critical challenge for a VJ engine: visuals must appear synchronized to the music. Human perception tolerates visual events being **up to ~20 ms early** or **up to ~40 ms late** relative to the audio beat. Beyond that, the sync feels "off."

### 7.1 The Latency Budget

```
Total visual latency = Audio buffer latency
                     + STFT computation latency
                     + ODF/beat tracker processing
                     + Render pipeline latency
                     + Display latency

Typical breakdown:
  Audio buffer:          5-20 ms  (256-1024 samples at 44.1kHz)
  STFT (half window):   ~11 ms   (512 samples of a 1024 window)
  Beat tracker:          ~1 ms   (negligible compute)
  Render pipeline:       ~5-16 ms (depending on frame rate)
  Display:               ~8-16 ms (1-2 frames at 60fps)
  ─────────────────────────────
  Total:                 ~30-64 ms
```

This exceeds the ~40 ms late tolerance. The solution: **beat prediction**.

### 7.2 Beat Prediction for Zero-Latency Visual Sync

Once a stable tempo is established, the next beat time is predictable:

```
next_beat_time = last_beat_time + beat_period
```

Schedule the visual event to fire at `next_beat_time - total_system_latency`. This way, the visual reaches the display at exactly the beat time.

```cpp
class BeatPredictor {
public:
    void on_beat_detected(double beat_time, double period) {
        last_beat_ = beat_time;
        period_ = period;
        confidence_ += 0.1f;
        confidence_ = std::min(confidence_, 1.0f);
    }

    void on_beat_missed() {
        confidence_ -= 0.2f;
        confidence_ = std::max(confidence_, 0.0f);
    }

    /// Returns the predicted next beat time, compensating for system latency.
    /// visual_latency_sec: total time from "decide to render" to "photons on screen"
    double next_visual_trigger(double current_time, double visual_latency_sec) {
        if (confidence_ < 0.3f || period_ <= 0.0)
            return -1.0;  // not confident enough to predict

        // How many beats since last detected beat?
        double elapsed = current_time - last_beat_;
        int beats_ahead = (int)(elapsed / period_) + 1;
        double next_beat = last_beat_ + beats_ahead * period_;

        // Schedule visual event early to compensate for rendering latency
        double trigger_time = next_beat - visual_latency_sec;

        // If trigger time is in the past, advance to next beat
        while (trigger_time < current_time)
            trigger_time += period_;

        return trigger_time;
    }

    /// Current beat phase [0.0, 1.0), suitable for continuous animation.
    float phase(double current_time) const {
        if (period_ <= 0.0) return 0.0f;
        double elapsed = current_time - last_beat_;
        float p = (float)std::fmod(elapsed / period_, 1.0);
        return p < 0.0f ? p + 1.0f : p;
    }

    float confidence() const { return confidence_; }

private:
    double last_beat_ = 0.0;
    double period_    = 0.5;  // 120 BPM default
    float  confidence_ = 0.0f;
};
```

### 7.3 Anticipation vs Reaction

Two paradigms for visual-beat sync:

**Reactive** (simpler): Wait for the beat tracker to confirm a beat, then trigger the visual. Inherent latency of 30-60 ms. Acceptable for smooth animations (e.g., pulsing glow that peaks slightly after the beat) but noticeable for sharp transients (e.g., a flash).

**Predictive** (recommended): Use the beat predictor to trigger visuals ahead of time, compensating for system latency. Achieves near-zero perceptual latency. Risk: if the prediction is wrong (tempo change, missed beat), the visual fires at the wrong time. Mitigate by gating prediction on high confidence and using the reactive path as fallback.

**Hybrid approach**:
1. Use predictive sync when beat confidence > 0.7 (steady section)
2. Fall back to reactive sync when confidence < 0.5 (transitions, breaks)
3. In the 0.5-0.7 range, blend: trigger at prediction time but with reduced intensity, boosted by actual onset confirmation

### 7.4 Frame-Level Timing

At 60 fps, each frame is ~16.67 ms. The visual system operates in discrete frames, so beat alignment is quantized to frame boundaries.

```
Frame timing diagram (60fps):

Audio beat:    |                    B                    |
               0ms                 200ms                 400ms

Frames:     |  F0  |  F1  |  F2  |  F3  |  F4  |  ...
            0    16.7   33.3   50    66.7

If beat falls at 200ms, it aligns with F12 start.
But due to 40ms system latency, the visual for beat B
must be committed at frame F9 or F10 (~150-167ms).

With prediction:
  Predicted beat = 200ms
  Visual latency = 40ms
  Trigger render at = 160ms → frame F9 (begins rendering at 150ms)
  Frame F9 appears on screen at ~167ms + display latency ≈ 183ms
  Visual-audio offset ≈ -17ms (visual 17ms early) → well within tolerance
```

### 7.5 Beat Phase as Animation Driver

Rather than triggering discrete events, use the continuous beat phase to drive animations. This guarantees smoothness regardless of prediction accuracy.

```cpp
// In the render loop (called every frame):
float phase = beat_predictor.phase(current_time);

// Example visual mappings:
float pulse     = 0.5f + 0.5f * std::cos(phase * 2.0f * M_PI);     // smooth pulse
float sawtooth  = 1.0f - phase;                                      // linear decay
float attack    = std::exp(-phase * 8.0f);                           // sharp attack, slow decay
float square    = phase < 0.1f ? 1.0f : 0.0f;                       // brief flash

// For bar-level animation (4/4 meter):
float bar_phase = (beat_in_bar + phase) / 4.0f;  // [0.0, 1.0) over 4 beats
float color_rotation = bar_phase;                 // rotate hue over one bar
```

### 7.6 Handling Tempo Transitions

When BPM changes (e.g., DJ mix transition), the beat phase can jump or stutter. Strategies:

1. **Phase smoothing**: Low-pass filter the raw phase to prevent visible jumps. Cost: adds ~50 ms latency to phase updates.
2. **Cross-fade**: When a tempo change is detected (>5 BPM shift over 2 seconds), cross-fade between old-tempo and new-tempo visuals over 2-4 beats.
3. **Confidence gating**: During tempo transitions, beat confidence drops. Reduce beat-reactive visual intensity and rely more on amplitude/spectral features until confidence recovers.

---

## 8. Summary: Choosing the Right Approach for a VJ Engine

### Recommended Real-Time Pipeline

```
┌─────────────────────────────────────────────────────────────────────┐
│  AUDIO INPUT (callback, 256-512 sample blocks)                      │
│                                                                     │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────────────┐       │
│  │ aubio_onset   │  │ aubio_tempo   │  │ Custom beat predictor│       │
│  │ (specflux)    │  │ (beat track)  │  │ (phase + prediction) │       │
│  └──────┬───────┘  └──────┬───────┘  └──────────┬───────────┘       │
│         │                 │                      │                   │
│         ▼                 ▼                      ▼                   │
│  ┌──────────────────────────────────────────────────────────────┐    │
│  │                    RhythmState struct                         │    │
│  │  onset_detected, onset_strength, onset_rate                  │    │
│  │  beat_detected, bpm, beat_phase, beat_confidence             │    │
│  │  bar_phase, downbeat_detected                                │    │
│  │  predicted_next_beat, pulse_clarity                           │    │
│  └──────────────────────────────────────────────────────────────┘    │
│                              │                                       │
│                    ┌─────────▼──────────┐                            │
│                    │  Lock-free queue    │                            │
│                    │  (audio → render)   │                            │
│                    └─────────┬──────────┘                            │
│                              │                                       │
│                    ┌─────────▼──────────┐                            │
│                    │  Render Thread      │                            │
│                    │  (reads phase,      │                            │
│                    │   drives visuals)   │                            │
│                    └────────────────────┘                            │
└─────────────────────────────────────────────────────────────────────┘
```

### Algorithm Selection Guide

| Need | Best Approach | Library | Latency |
|------|--------------|---------|---------|
| Onset detection (general) | Spectral flux + adaptive threshold | aubio (`specflux`) | ~23 ms |
| Onset detection (percussive) | HFC | aubio (`hfc`) | ~12 ms |
| Onset detection (polyphonic) | Complex domain or MKL | aubio (`complex`, `mkl`) | ~35 ms |
| BPM estimation | Comb filter bank | aubio (internal) | 2-4 sec convergence |
| Beat tracking (real-time) | aubio_tempo or DBN | aubio / madmom | ~23 ms + prediction |
| Beat phase | Custom predictor on top of beat tracker | Custom C++ | <1 ms |
| Downbeat / meter | Sub-band accent analysis | Custom / Essentia | 4-16 beats convergence |
| Tempo change detection | BPM derivative monitoring | Custom | 2-4 sec |
| Visual sync | Beat prediction with latency compensation | Custom C++ | ~0 ms perceived |

### Key Latency Numbers (see REF_latency_numbers.md)

| Component | Latency |
|-----------|---------|
| Audio buffer (512 samples @ 44.1kHz) | 11.6 ms |
| STFT window center (1024 samples) | 11.6 ms |
| Spectral flux ODF (1 previous frame) | 11.6 ms |
| Complex domain ODF (2 previous frames) | 23.2 ms |
| Peak picking (causal median window) | 0 ms (no lookahead) |
| Beat prediction phase advance | -30 to -50 ms (compensates latency) |
| GPU render (single frame @ 60fps) | 8-16 ms |
| Display output (1 frame @ 60fps) | 8-16 ms |

---

## References

- Bello, J.P., et al. (2005). "A Tutorial on Onset Detection in Music Signals." IEEE Trans. Speech Audio Process.
- Dixon, S. (2006). "Onset Detection Revisited." Proc. DAFx.
- Scheirer, E. (1998). "Tempo and Beat Analysis of Acoustic Musical Signals." JASA.
- Ellis, D. (2007). "Beat Tracking by Dynamic Programming." JNMR.
- Bock, S., et al. (2011). "Enhanced Beat Tracking with Context-Aware Neural Networks." Proc. DAFx.
- Bock, S., & Schedl, M. (2014). "Accurate Tempo Estimation based on Recurrent Neural Networks and Resonating Comb Filters." Proc. ISMIR.
- aubio documentation: https://aubio.org/manual/latest/
- Essentia documentation: https://essentia.upf.edu/reference/
