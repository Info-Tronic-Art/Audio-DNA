# Structural Analysis Features for Real-Time Music Visualization

**Document**: FEATURES_structural.md
**Scope**: High-level structural analysis features operating in real-time for a VJ / music visualization engine.
**Cross-references**: FEATURES_rhythm_tempo.md, FEATURES_spectral.md, FEATURES_mfcc_mel.md, FEATURES_amplitude_dynamics.md, VIDEO_feature_to_visual_mapping.md, IMPL_calibration_adaptation.md

---

## 0. Preamble: Why Structural Analysis in Real-Time Is Hard

Traditional MIR structural analysis (Foote's novelty, McFee & Ellis's spectral clustering, Serra et al.'s community detection) operates on complete recordings. The algorithms assume access to the entire self-similarity matrix (SSM), the full time series of chroma or MFCCs, and unlimited compute budgets. A real-time VJ engine has none of these luxuries. We must detect section boundaries, buildups, drops, and breakdowns using only causal information -- features from the past and present, never the future.

The core strategy is a **ring buffer of feature snapshots** (30-60 seconds of history at beat-synchronous or fixed-rate intervals) combined with streaming novelty functions, exponential moving averages at multiple time scales, and heuristic state machines that classify the current structural region. Latency budget for the entire structural analysis pass is **< 2 ms per frame** on a single core, computed once per audio analysis hop (typically every 512 or 1024 samples at 44.1 kHz, i.e., every 11.6-23.2 ms).

---

## 1. Novelty Function Construction

A **novelty function** is a one-dimensional time series that peaks at moments of significant musical change. In offline analysis, Foote (2000) proposed convolving a checkerboard kernel along the diagonal of an SSM. In real-time, we approximate this by computing frame-to-frame distances on multiple feature streams and fusing the results.

### 1.1 Spectral Flux Novelty

Spectral flux measures the frame-to-frame change in the magnitude spectrum. Given the magnitude spectrum $|X_t(k)|$ at frame $t$:

$$\text{SF}(t) = \sum_{k=0}^{N/2} H\!\bigl(|X_t(k)| - |X_{t-1}(k)|\bigr)$$

where $H(x) = \max(0, x)$ is the half-wave rectifier that retains only positive (increasing) spectral energy, suppressing offsets and decays. This is the same quantity computed by the onset detector (see FEATURES_rhythm_tempo.md), but here we use it as a structural novelty signal rather than for onset triggering.

For structural purposes, we often want a **broadband** spectral flux (the full spectrum sum) plus **sub-band** spectral fluxes (low: 0-300 Hz, mid: 300-2000 Hz, high: 2000-8000 Hz, brilliance: 8000+ Hz). Sub-band novelty helps differentiate a kick pattern change (low-band flux) from a hi-hat pattern change (high-band flux).

### 1.2 Chroma Novelty (Harmonic Change Detection Function)

The Harmonic Change Detection Function (HCDF) detects chord changes by measuring the rate of change in chroma space. Given a 12-dimensional chroma vector $\mathbf{c}_t$:

$$\text{HCDF}(t) = \|\mathbf{c}_t - \mathbf{c}_{t-1}\|_2$$

Before differencing, the chroma vectors should be L2-normalized to unit magnitude. This removes the influence of loudness variation and focuses purely on pitch-class distribution changes. An alternative is cosine distance:

$$\text{HCDF}_{\cos}(t) = 1 - \frac{\mathbf{c}_t \cdot \mathbf{c}_{t-1}}{\|\mathbf{c}_t\| \, \|\mathbf{c}_{t-1}\|}$$

Chord boundaries produce sharp peaks in HCDF. Key changes produce sustained elevated values. Both are musically meaningful structural cues.

### 1.3 MFCC Novelty (Timbral Change Detection)

MFCCs capture the spectral envelope shape (see FEATURES_mfcc_mel.md). Timbral novelty is the Euclidean distance between successive MFCC vectors, typically using coefficients 1-12 (excluding the 0th, which encodes energy and is tracked separately):

$$\text{TN}(t) = \|\mathbf{m}_t - \mathbf{m}_{t-1}\|_2$$

where $\mathbf{m}_t = [\text{MFCC}_1(t), \dots, \text{MFCC}_{12}(t)]$. A singer entering, a synthesizer timbre change, or an instrument drop-out will produce sharp MFCC novelty peaks. Because MFCCs decorrelate spectral information, this metric captures changes that spectral flux might miss (e.g., a formant shift at constant energy).

### 1.4 Multi-Feature Novelty Fusion

Each novelty stream captures different aspects of musical change. Fusion combines them into a single novelty curve. The simplest approach is a weighted sum:

$$N(t) = w_{\text{sf}} \cdot \hat{N}_{\text{sf}}(t) + w_{\text{hc}} \cdot \hat{N}_{\text{hc}}(t) + w_{\text{tn}} \cdot \hat{N}_{\text{tn}}(t)$$

where $\hat{N}_x(t)$ is the z-score normalized version of each novelty function (subtract running mean, divide by running standard deviation, computed over the ring buffer window). Normalization is critical because the raw magnitudes of spectral flux, chroma distance, and MFCC distance inhabit completely different scales.

Default weights: $w_{\text{sf}} = 0.4$, $w_{\text{hc}} = 0.35$, $w_{\text{tn}} = 0.25$. These can be adapted by genre (see IMPL_calibration_adaptation.md).

An alternative is **product fusion**: $N(t) = \hat{N}_{\text{sf}}(t) \cdot \hat{N}_{\text{hc}}(t) \cdot \hat{N}_{\text{tn}}(t)$, which fires only when all three streams agree. This is more conservative and produces fewer false positives at the cost of missing subtle transitions.

### 1.5 Smoothing

Raw novelty functions are noisy. Two smoothing strategies:

**Gaussian kernel smoothing**: Convolve with a causal half-Gaussian (we cannot look ahead). A kernel of width $\sigma = 4$ frames (approximately 100 ms at hop=1024/44100) is typical. This removes jitter while preserving sharp peaks.

**Median filter**: A causal median filter of length 5-7 frames removes impulsive noise (e.g., a single anomalous frame due to a transient). Applied after Gaussian smoothing.

Both filters introduce latency equal to half their kernel width. For $\sigma = 4$, the effective latency is approximately 2 frames (~46 ms), which is acceptable for structural-level decisions.

### 1.6 Peak Picking for Section Boundary Detection

After smoothing, section boundaries correspond to peaks in the novelty function. Real-time peak picking uses:

1. **Adaptive threshold**: peak must exceed $\mu + k\sigma$ where $\mu$ and $\sigma$ are the running mean and standard deviation of the novelty function over the past 10 seconds. Typical $k = 1.5$.
2. **Minimum inter-peak distance**: Section boundaries are at least 2 seconds apart (in practice, at least 4 bars). This prevents beat-level events from being classified as structural boundaries.
3. **Look-back confirmation**: A peak at frame $t$ is confirmed at frame $t + \delta$ (where $\delta \approx 5$ frames) by checking that the novelty has decreased below the peak. This adds ~100 ms latency but eliminates false peaks on rising slopes.

### 1.7 C++ Implementation

```cpp
struct NoveltyDetector {
    // Ring buffers for each feature stream
    RingBuffer<float> spectral_flux_history{2048};  // ~47s at 23ms hop
    RingBuffer<float> chroma_novelty_history{2048};
    RingBuffer<float> mfcc_novelty_history{2048};
    RingBuffer<float> fused_novelty{2048};

    // Running statistics for z-score normalization
    RunningStats sf_stats, hc_stats, tn_stats;

    // Smoothing kernels (causal half-Gaussian, precomputed)
    static constexpr int kGaussianWidth = 9;
    float gaussian_kernel[kGaussianWidth];  // Half-Gaussian, normalized

    // Median filter state
    MedianFilter<float, 7> median_filter;

    // Peak picker state
    float adaptive_mean = 0.0f;
    float adaptive_var  = 0.0f;
    float alpha_stats   = 0.01f;   // EMA decay for running stats (~10s window)
    float peak_threshold_k = 1.5f;
    int   min_peak_distance_frames = 128;  // ~3 seconds at 23ms hop
    int   frames_since_last_peak = 0;
    int   lookback_delay = 5;
    float peak_candidate_value = 0.0f;
    int   peak_candidate_age = -1;

    // Fusion weights
    float w_sf = 0.4f, w_hc = 0.35f, w_tn = 0.25f;

    NoveltyDetector() {
        // Precompute causal half-Gaussian kernel
        float sum = 0.0f;
        float sigma = 3.0f;
        for (int i = 0; i < kGaussianWidth; ++i) {
            gaussian_kernel[i] = std::exp(-0.5f * (i * i) / (sigma * sigma));
            sum += gaussian_kernel[i];
        }
        for (int i = 0; i < kGaussianWidth; ++i)
            gaussian_kernel[i] /= sum;
    }

    struct NoveltyResult {
        float fused_novelty;
        bool  section_boundary_detected;
        float boundary_confidence;  // 0..1
    };

    NoveltyResult process(float spectral_flux, float chroma_dist, float mfcc_dist) {
        // Store raw values
        spectral_flux_history.push(spectral_flux);
        chroma_novelty_history.push(chroma_dist);
        mfcc_novelty_history.push(mfcc_dist);

        // Update running stats
        sf_stats.update(spectral_flux);
        hc_stats.update(chroma_dist);
        tn_stats.update(mfcc_dist);

        // Z-score normalize
        float sf_z = sf_stats.stddev() > 1e-6f
            ? (spectral_flux - sf_stats.mean()) / sf_stats.stddev() : 0.0f;
        float hc_z = hc_stats.stddev() > 1e-6f
            ? (chroma_dist - hc_stats.mean()) / hc_stats.stddev() : 0.0f;
        float tn_z = tn_stats.stddev() > 1e-6f
            ? (mfcc_dist - tn_stats.mean()) / tn_stats.stddev() : 0.0f;

        // Fuse
        float raw_fused = w_sf * sf_z + w_hc * hc_z + w_tn * tn_z;

        // Gaussian smooth (causal convolution over ring buffer)
        fused_novelty.push(raw_fused);
        float smoothed = 0.0f;
        for (int i = 0; i < kGaussianWidth; ++i) {
            int idx = fused_novelty.size() - 1 - i;
            if (idx >= 0)
                smoothed += gaussian_kernel[i] * fused_novelty[idx];
        }

        // Median filter
        smoothed = median_filter.process(smoothed);

        // Adaptive threshold
        adaptive_mean += alpha_stats * (smoothed - adaptive_mean);
        float diff = smoothed - adaptive_mean;
        adaptive_var += alpha_stats * (diff * diff - adaptive_var);
        float adaptive_std = std::sqrt(adaptive_var);
        float threshold = adaptive_mean + peak_threshold_k * adaptive_std;

        // Peak picking with look-back confirmation
        frames_since_last_peak++;
        bool boundary = false;
        float confidence = 0.0f;

        if (peak_candidate_age >= 0) {
            peak_candidate_age++;
            if (peak_candidate_age >= lookback_delay) {
                // Confirm: novelty has dropped below candidate
                if (smoothed < peak_candidate_value * 0.8f) {
                    boundary = true;
                    confidence = std::min(1.0f,
                        (peak_candidate_value - threshold) / (adaptive_std + 1e-6f));
                    frames_since_last_peak = 0;
                }
                peak_candidate_age = -1;
            }
        }

        if (smoothed > threshold
            && frames_since_last_peak > min_peak_distance_frames
            && peak_candidate_age < 0) {
            if (peak_candidate_age < 0 || smoothed > peak_candidate_value) {
                peak_candidate_value = smoothed;
                peak_candidate_age = 0;
            }
        }

        return { smoothed, boundary, std::clamp(confidence, 0.0f, 1.0f) };
    }
};
```

---

## 2. Energy Envelope Tracking (Multi-Scale)

Structural perception in music is inherently multi-scale. A listener simultaneously tracks the energy contour at the note level, the phrase level, the section level, and the overall arc of the piece. Our system mirrors this with four parallel exponential moving average (EMA) trackers operating at different time constants.

### 2.1 Time Scale Definitions

| Scale | Time Constant | Musical Level | Typical Use |
|-------|--------------|---------------|-------------|
| Short-term | 100 ms | Syllable / onset | Beat-reactive visuals |
| Medium-term | 1 s | Phrase | Gesture-level animation |
| Long-term | 4 s | Section | Color palette / scene transitions |
| Very long-term | 16 s | Movement / arc | Global intensity, background drift |

### 2.2 Exponential Moving Average

For a time constant $\tau$ (in seconds) and analysis hop $h$ (in seconds):

$$\alpha = 1 - \exp\!\left(-\frac{h}{\tau}\right)$$

The EMA update:

$$E_\tau(t) = \alpha \cdot x(t) + (1 - \alpha) \cdot E_\tau(t-1)$$

where $x(t)$ is the instantaneous RMS energy (or dB energy, depending on the application). The EMA acts as a single-pole IIR lowpass filter with a -3 dB cutoff at $f_c = 1/(2\pi\tau)$.

For visualization, the **ratio** between adjacent scales is often more useful than the absolute values:

- $R_{\text{short/med}}(t) = E_{0.1}(t) / E_{1.0}(t)$ -- peaks during onsets and transients
- $R_{\text{med/long}}(t) = E_{1.0}(t) / E_{4.0}(t)$ -- peaks during phrase climaxes
- $R_{\text{long/vlong}}(t) = E_{4.0}(t) / E_{16.0}(t)$ -- peaks during section climaxes

These ratios naturally self-normalize and provide scale-relative "surprise" signals.

### 2.3 Parallel Tracking Implementation

```cpp
struct MultiScaleEnergy {
    struct EMATracker {
        float alpha;
        float value = 0.0f;

        EMATracker() : alpha(0.0f) {}

        void init(float time_constant_s, float hop_s) {
            alpha = 1.0f - std::exp(-hop_s / time_constant_s);
        }

        float process(float input) {
            value += alpha * (input - value);
            return value;
        }
    };

    EMATracker short_term;    // 100ms
    EMATracker medium_term;   // 1s
    EMATracker long_term;     // 4s
    EMATracker vlong_term;    // 16s

    struct Result {
        float e_short, e_medium, e_long, e_vlong;
        float ratio_short_med;   // onset/transient surprise
        float ratio_med_long;    // phrase climax
        float ratio_long_vlong;  // section climax
    };

    void init(float hop_seconds) {
        short_term.init(0.1f, hop_seconds);
        medium_term.init(1.0f, hop_seconds);
        long_term.init(4.0f, hop_seconds);
        vlong_term.init(16.0f, hop_seconds);
    }

    Result process(float rms_energy) {
        Result r;
        r.e_short  = short_term.process(rms_energy);
        r.e_medium = medium_term.process(rms_energy);
        r.e_long   = long_term.process(rms_energy);
        r.e_vlong  = vlong_term.process(rms_energy);

        const float eps = 1e-10f;
        r.ratio_short_med  = r.e_short  / (r.e_medium + eps);
        r.ratio_med_long   = r.e_medium / (r.e_long + eps);
        r.ratio_long_vlong = r.e_long   / (r.e_vlong + eps);

        return r;
    }
};
```

The computational cost is trivial: 4 multiply-adds per frame. The value of this module lies not in its complexity but in the rich multi-scale dynamic information it provides downstream.

### 2.4 Derivative Signals

For buildup/drop detection (sections 3-4), we also track the **derivative** (rate of change) of each EMA. Since the EMA is itself a smoothed signal, its derivative is low-noise:

$$\dot{E}_\tau(t) = E_\tau(t) - E_\tau(t-1)$$

A positive $\dot{E}_{4.0}$ sustained over multiple seconds indicates a buildup. A large positive spike in $\dot{E}_{0.1}$ after sustained positive $\dot{E}_{4.0}$ indicates a drop.

---

## 3. Buildup Detection

A **buildup** (also called a "riser" in EDM production) is a sustained increase in energy, brightness, and/or rhythmic density over several bars, typically 4-16 bars (approximately 4-30 seconds at 120-140 BPM). Buildups create anticipation and are among the most visually impactful structural moments.

### 3.1 Component Signals

**Rising RMS**: The derivative of the long-term energy EMA ($\tau = 4$s) must be positive for a sustained period. We measure buildup_rms as the fraction of the last $W$ frames (where $W$ corresponds to ~8 seconds) during which $\dot{E}_{4.0} > 0$:

$$B_{\text{rms}}(t) = \frac{1}{W} \sum_{i=0}^{W-1} \mathbf{1}\!\left[\dot{E}_{4.0}(t-i) > 0\right]$$

A value above 0.7 indicates consistent energy growth.

**Rising spectral centroid**: Increasing brightness is a hallmark of buildups. Producers add high-frequency content (noise sweeps, rising filter cutoffs, cymbal rolls). Computed identically to $B_{\text{rms}}$ but using the derivative of a smoothed spectral centroid:

$$B_{\text{bright}}(t) = \frac{1}{W} \sum_{i=0}^{W-1} \mathbf{1}\!\left[\dot{C}_{\text{centroid}}(t-i) > 0\right]$$

**Rising spectral flux**: Increasing rhythmic and textural complexity. More onsets per second, more spectral change per frame:

$$B_{\text{flux}}(t) = \frac{1}{W} \sum_{i=0}^{W-1} \mathbf{1}\!\left[\dot{\overline{\text{SF}}}(t-i) > 0\right]$$

where $\overline{\text{SF}}$ is the 1-second smoothed spectral flux.

### 3.2 Combined Buildup Score

$$B(t) = \frac{w_1 B_{\text{rms}}(t) + w_2 B_{\text{bright}}(t) + w_3 B_{\text{flux}}(t)}{w_1 + w_2 + w_3}$$

Default weights: $w_1 = 0.4$, $w_2 = 0.35$, $w_3 = 0.25$. The score ranges from 0 (no buildup) to 1 (strong buildup on all dimensions).

A buildup is **active** when $B(t) > 0.5$ for at least 2 consecutive seconds.

### 3.3 Anticipation Signaling

The buildup score can be mapped to visual parameters for anticipation effects:

- **Particle spawn rate**: linearly proportional to $B(t)$
- **Camera zoom**: slow exponential zoom-in during buildup, proportional to $B(t)$
- **Color saturation ramp**: desaturate-to-saturate over the buildup duration
- **Strobe frequency**: increase flash rate as $B(t)$ approaches 1.0

See VIDEO_feature_to_visual_mapping.md for the full mapping table.

### 3.4 State Machine

```
                ┌────────────┐
                │   IDLE     │
                │ B(t) < 0.3 │
                └─────┬──────┘
                      │ B(t) > 0.5 for > 1s
                      v
                ┌────────────┐
                │  BUILDUP   │──────── B(t) < 0.3 for > 2s ──> IDLE
                │ B(t) > 0.5 │
                └─────┬──────┘
                      │ drop detected (see section 4)
                      v
                ┌────────────┐
                │   DROP     │
                │ (transient)│──── after 2s ──> SUSTAIN or IDLE
                └────────────┘
```

### 3.5 C++ Implementation

```cpp
struct BuildupDetector {
    static constexpr int kHistoryFrames = 512;  // ~12s at 23ms hop

    RingBuffer<float> rms_deriv{kHistoryFrames};
    RingBuffer<float> centroid_deriv{kHistoryFrames};
    RingBuffer<float> flux_deriv{kHistoryFrames};

    EMATracker rms_smooth, centroid_smooth, flux_smooth;

    float prev_rms_smooth = 0.0f;
    float prev_centroid_smooth = 0.0f;
    float prev_flux_smooth = 0.0f;

    float w_rms = 0.4f, w_bright = 0.35f, w_flux = 0.25f;

    // State
    float buildup_score = 0.0f;
    int   buildup_active_frames = 0;
    bool  buildup_active = false;

    void init(float hop_s) {
        rms_smooth.init(1.0f, hop_s);
        centroid_smooth.init(1.0f, hop_s);
        flux_smooth.init(1.0f, hop_s);
    }

    float fraction_positive(const RingBuffer<float>& buf, int window) {
        int count = 0;
        int n = std::min(window, (int)buf.size());
        for (int i = 0; i < n; ++i) {
            if (buf[buf.size() - 1 - i] > 0.0f) count++;
        }
        return (float)count / (float)std::max(n, 1);
    }

    struct Result {
        float score;         // 0..1
        bool  active;        // buildup currently happening
        float duration_s;    // how long buildup has been active
    };

    Result process(float rms, float spectral_centroid,
                   float spectral_flux, float hop_s) {
        float rs = rms_smooth.process(rms);
        float cs = centroid_smooth.process(spectral_centroid);
        float fs = flux_smooth.process(spectral_flux);

        rms_deriv.push(rs - prev_rms_smooth);
        centroid_deriv.push(cs - prev_centroid_smooth);
        flux_deriv.push(fs - prev_flux_smooth);

        prev_rms_smooth = rs;
        prev_centroid_smooth = cs;
        prev_flux_smooth = fs;

        int window = std::min(kHistoryFrames, (int)(8.0f / hop_s));  // 8s window
        float b_rms    = fraction_positive(rms_deriv, window);
        float b_bright = fraction_positive(centroid_deriv, window);
        float b_flux   = fraction_positive(flux_deriv, window);

        buildup_score = (w_rms * b_rms + w_bright * b_bright
                       + w_flux * b_flux)
                      / (w_rms + w_bright + w_flux);

        if (buildup_score > 0.5f) {
            buildup_active_frames++;
        } else if (buildup_score < 0.3f) {
            buildup_active_frames = 0;
        }

        buildup_active = buildup_active_frames > (int)(2.0f / hop_s);

        return {
            buildup_score,
            buildup_active,
            buildup_active ? buildup_active_frames * hop_s : 0.0f
        };
    }
};
```

---

## 4. Drop Detection

A **drop** is the moment of maximum energy release following a buildup. In EDM, it is the transition from riser to main groove. In rock/pop, it is the entrance of the full band after an intro or bridge. Drops are the single most important structural event for a VJ engine.

### 4.1 Component Signals

**Sudden energy spike**: The short-term energy must jump significantly relative to the long-term energy. We measure this as:

$$D_{\text{energy}}(t) = \frac{E_{0.1}(t) - E_{4.0}(t)}{E_{16.0}(t) + \epsilon}$$

This is the ratio of "how much louder is right now versus recent history" normalized by the overall level of the piece. A drop produces $D_{\text{energy}} > 2.0$ (short-term energy is more than 2x the long-term mean).

**Spectral bandwidth explosion**: Drops introduce content across the full frequency range. Spectral bandwidth (the standard deviation of the spectrum weighted by magnitude) should jump by more than 1.5x its running mean.

**Onset density jump**: The number of detected onsets per second should jump by at least 2x. This captures the rhythmic acceleration that accompanies drops.

### 4.2 Drop Confidence Score

$$D(t) = \begin{cases}
\sigma\!\left(\frac{D_{\text{energy}}(t) - \mu_D}{\sigma_D}\right) \cdot P_{\text{buildup}} & \text{if buildup was recently active} \\
0 & \text{otherwise}
\end{cases}$$

where $\sigma(\cdot)$ is the logistic sigmoid and $P_{\text{buildup}}$ is a decay factor: 1.0 if the buildup ended within the last 0.5 seconds, decaying linearly to 0 over 2 seconds. This ensures drops are only detected following buildups, drastically reducing false positives.

A drop is declared when $D(t) > 0.7$. The detection fires as a one-shot event (not sustained) and triggers a cooldown of at least 4 seconds.

### 4.3 C++ Implementation

```cpp
struct DropDetector {
    float prev_buildup_active = false;
    float buildup_recency = 0.0f;  // 1.0 when buildup just ended, decays to 0
    float decay_rate;               // per frame

    RunningStats energy_ratio_stats;
    float cooldown_frames_remaining = 0;

    void init(float hop_s) {
        decay_rate = hop_s / 2.0f;  // decay to 0 over 2 seconds
    }

    struct Result {
        bool  detected;
        float confidence;
    };

    Result process(const MultiScaleEnergy::Result& energy,
                   float spectral_bandwidth,
                   float spectral_bandwidth_mean,
                   float onset_rate,
                   float onset_rate_mean,
                   bool buildup_currently_active,
                   float hop_s) {
        // Track buildup recency
        if (prev_buildup_active && !buildup_currently_active) {
            buildup_recency = 1.0f;  // buildup just ended
        }
        buildup_recency = std::max(0.0f, buildup_recency - decay_rate);
        prev_buildup_active = buildup_currently_active;

        // Energy ratio
        const float eps = 1e-10f;
        float d_energy = (energy.e_short - energy.e_long) / (energy.e_vlong + eps);

        energy_ratio_stats.update(d_energy);

        // Z-score of energy ratio
        float z = (energy_ratio_stats.stddev() > eps)
            ? (d_energy - energy_ratio_stats.mean()) / energy_ratio_stats.stddev()
            : 0.0f;

        // Sigmoid
        float sig = 1.0f / (1.0f + std::exp(-z));

        // Spectral bandwidth jump
        float bw_ratio = spectral_bandwidth / (spectral_bandwidth_mean + eps);

        // Onset density jump
        float onset_ratio = onset_rate / (onset_rate_mean + eps);

        // Combined confidence
        float raw_confidence = sig * 0.5f
                             + std::min(1.0f, bw_ratio / 2.0f) * 0.25f
                             + std::min(1.0f, onset_ratio / 3.0f) * 0.25f;

        float confidence = raw_confidence * buildup_recency;

        // Cooldown
        cooldown_frames_remaining = std::max(0.0f,
            cooldown_frames_remaining - 1.0f);

        bool detected = (confidence > 0.7f) && (cooldown_frames_remaining <= 0);
        if (detected) {
            cooldown_frames_remaining = 4.0f / hop_s;  // 4 second cooldown
        }

        return { detected, confidence };
    }
};
```

---

## 5. Breakdown Detection

A **breakdown** is a section of reduced energy, rhythmic density, and textural complexity. In EDM, breakdowns strip the arrangement to pads, vocals, or FX. In rock, they may feature a solo instrument or half-time feel. For the VJ engine, breakdowns are opportunities for calm, ambient visuals.

### 5.1 Component Signals

**Sparse energy patterns**: The ratio $E_{0.1} / E_{4.0}$ drops below 0.5 (individual hits are weak relative to the long-term average) AND the absolute energy $E_{4.0}$ is below the running median of $E_{16.0}$.

**Low onset rate**: The onset detection rate (onsets per second) drops below 2.0. Full grooves typically have 4-8+ onsets/second; breakdowns have isolated hits.

**Spectral simplification**: The spectral flatness (see FEATURES_spectral.md) decreases (fewer simultaneous frequency components) and spectral bandwidth narrows.

**Instrument count reduction estimation**: An approximate instrument count can be estimated from the number of prominent spectral peaks (local maxima in the smoothed magnitude spectrum that exceed -20 dB relative to the spectral maximum). During breakdowns, this count drops to 1-3 from a typical 5-10 during full sections.

### 5.2 Breakdown Score

$$K(t) = w_1 \cdot (1 - \text{clamp}(R_{\text{short/med}}, 0, 1)) + w_2 \cdot (1 - \text{clamp}(\text{onset\_rate}/8, 0, 1)) + w_3 \cdot (1 - \text{clamp}(\text{peak\_count}/10, 0, 1))$$

Default weights: $w_1 = 0.4$, $w_2 = 0.35$, $w_3 = 0.25$. The score approaches 1.0 when all indicators suggest a breakdown. A breakdown state is declared when $K(t) > 0.6$ sustained for at least 2 seconds.

### 5.3 State Machine Integration

Breakdowns, buildups, and drops form a cycle common in electronic and pop music:

```
    INTRO ──> VERSE/CHORUS ──> BREAKDOWN ──> BUILDUP ──> DROP ──> VERSE/CHORUS ──> ...
                                    ^                                    |
                                    └────────────────────────────────────┘
```

The structural state machine tracks transitions between these states. Invalid transitions (e.g., DROP immediately following INTRO without BUILDUP) are suppressed, which improves classification accuracy.

---

## 6. Phrase Detection

Phrases are intermediate structural units between individual beats and full sections. A phrase is typically 4, 8, or 16 bars long, depending on the genre and the specific musical content.

### 6.1 Bar Boundary Detection from Beat Tracker

Given a beat tracker (see FEATURES_rhythm_tempo.md) that provides beat positions and a tempo estimate, bar boundaries occur every $N$ beats, where $N$ is the time signature numerator (typically 4 for 4/4 time). Phrase boundaries then occur at every 4th or 8th bar:

```cpp
struct PhraseDetector {
    int beats_per_bar = 4;
    int bars_per_phrase = 8;    // default: 8 bars = 1 phrase
    int beat_count = 0;
    int bar_count = 0;

    struct Result {
        bool beat_boundary;
        bool bar_boundary;
        bool phrase_boundary;
        int  current_beat_in_bar;   // 0..beats_per_bar-1
        int  current_bar_in_phrase; // 0..bars_per_phrase-1
    };

    Result on_beat() {
        Result r;
        r.current_beat_in_bar = beat_count % beats_per_bar;
        r.current_bar_in_phrase = bar_count % bars_per_phrase;
        r.beat_boundary = true;
        r.bar_boundary = (r.current_beat_in_bar == 0);
        r.phrase_boundary = r.bar_boundary && (r.current_bar_in_phrase == 0);

        beat_count++;
        if (r.bar_boundary) bar_count++;

        return r;
    }
};
```

The challenge is **phase alignment**: knowing which beat is beat 1 of the bar. Common heuristics:

- The loudest beat in a 4-beat window is beat 1 (kick on downbeat in most genres).
- A significant novelty peak resets the phrase counter (new phrase after a section boundary).
- Harmonic changes that align with bar boundaries confirm the phase.

### 6.2 Harmonic Cadence Detection

Musical phrases often end with harmonic cadences (e.g., V-I, IV-I). In real-time, exact Roman numeral analysis is infeasible, but we can detect cadential motion by observing:

1. A significant HCDF peak (chord change)
2. Followed by a stable chroma vector (the new chord is sustained)
3. Coinciding with a bar boundary

When all three conditions are met, we boost the confidence that a phrase boundary has occurred. This is tracked as a cadence score that feeds into the phrase boundary confidence.

### 6.3 Repetition Detection via Self-Similarity

Short-term repetition detection supports phrase analysis. If the chroma or MFCC sequence from the last 8 bars closely matches the sequence from bars 9-16 ago, we have evidence of a repeated phrase. This uses the real-time self-similarity system described in section 7.

---

## 7. Self-Similarity in Real-Time

Self-similarity matrices (SSMs) are the foundation of offline structural analysis. An SSM $S$ has entry $S(i,j) = \text{sim}(\mathbf{f}_i, \mathbf{f}_j)$ where $\mathbf{f}_t$ is a feature vector at time $t$. Diagonal stripes indicate repetitions; block structure indicates sections.

### 7.1 Circular Buffer of Feature Vectors

In real-time, we maintain a circular buffer of the most recent $L$ feature snapshots. A typical choice is $L = 2048$ frames at a hop of 23 ms, giving approximately 47 seconds of history.

Each snapshot is a compact feature vector. To keep memory and compute manageable, we use a concatenation of:
- 12-dimensional chroma (L2-normalized)
- 13-dimensional MFCCs (z-score normalized, coefficients 0-12)
- 4-dimensional energy envelope (the four EMA values)

Total: 29 floats per frame. At $L = 2048$: $2048 \times 29 \times 4 = 237$ KB. This fits comfortably in L2 cache.

### 7.2 Real-Time Recurrence Quantification

Computing the full $L \times L$ SSM every frame is prohibitive ($2048^2 = 4.2$ million distance computations). Instead, we compute only the **most recent row** of the SSM at each frame:

$$S(t, j) = \exp\!\left(-\frac{\|\mathbf{f}_t - \mathbf{f}_j\|^2}{2\sigma^2}\right) \quad \text{for } j \in [t-L, t-1]$$

This requires $L = 2048$ distance computations per frame. With 29-dimensional vectors, each distance is 29 multiply-adds, totaling $2048 \times 29 \approx 59,000$ operations. At modern CPU speeds (5+ GFLOPS per core), this takes approximately **12 microseconds** -- negligible.

### 7.3 Repetition Detection

From the most recent SSM row, we look for **off-diagonal peaks**: high similarity values at lags corresponding to musical periods. If $S(t, t - P)$ is high, the music at time $t$ is similar to the music $P$ frames ago. Common periods to check:

- $P = $ 1 phrase (e.g., 8 bars at current tempo)
- $P = $ 2 phrases
- $P = $ 4 phrases

A "recurrence score" accumulates evidence over time:

$$R_P(t) = \alpha \cdot S(t, t-P) + (1-\alpha) \cdot R_P(t-1)$$

When $R_P(t) > 0.7$ for a phrase-length period $P$, we conclude the current music is repeating with period $P$. This is strong evidence that we are within a section (verse or chorus repeating its pattern).

### 7.4 Memory and Computational Budget

| Component | Memory | Compute/Frame |
|-----------|--------|---------------|
| Feature buffer (2048 x 29 float) | 237 KB | 0 (just writes) |
| SSM row computation (2048 distances) | 8 KB (one row) | ~12 us |
| Recurrence score EMA (3 period trackers) | 12 bytes | ~0.1 us |
| **Total** | **~245 KB** | **~12 us** |

This is well within budget. If a larger history is needed (e.g., 120 seconds for long-form pieces), increase $L$ to 4096 or downsample the feature rate (e.g., one snapshot per beat rather than per hop frame).

### 7.5 Lag Matrix for Visual Feedback

As a side benefit, the SSM row can be rendered as a 1D texture or used to drive a "deja vu" visual effect: when repetition is detected, the engine can overlay or morph between the current visual and the visual from $P$ frames ago, creating a visual echo that mirrors the musical structure.

---

## 8. Segment Labeling

Labeling the current segment as intro, verse, chorus, bridge, drop, breakdown, or outro is the highest-level structural classification. In real-time, we use a heuristic approach combining the signals from sections 2-7.

### 8.1 Feature-Based Heuristics

| Segment | Energy Level | Onset Rate | Spectral Complexity | Buildup Score | Repetition | Duration |
|---------|-------------|-----------|-------------------|--------------|-----------|----------|
| Intro | Rising from low | Low-medium | Low-medium | No | Low (novel material) | First 15-30s |
| Verse | Medium | Medium | Medium | No | High (repeated pattern) | 16-32 bars |
| Chorus | High | High | High | No | High (repeated, brighter) | 8-16 bars |
| Bridge | Medium | Variable | Changed timbre | Sometimes | Low (novel material) | 8-16 bars |
| Drop | Very high (sudden) | Very high | Very high | Just ended | New pattern | 8-32 bars |
| Breakdown | Low | Low | Low | No | Variable | 8-32 bars |
| Outro | Declining | Declining | Declining | No | Sometimes | Last 15-30s |

### 8.2 State Machine with Confidence Scores

```cpp
enum class SegmentType {
    UNKNOWN, INTRO, VERSE, CHORUS, BRIDGE, DROP, BREAKDOWN, OUTRO
};

struct SegmentLabeler {
    SegmentType current = SegmentType::UNKNOWN;
    float       confidence = 0.0f;
    float       time_in_segment = 0.0f;
    float       total_elapsed = 0.0f;
    bool        has_had_high_energy = false;

    struct Input {
        float energy_long;         // E_4s
        float energy_vlong;        // E_16s
        float energy_deriv_long;   // d/dt E_4s
        float onset_rate;
        float spectral_complexity; // e.g., spectral flatness
        float buildup_score;
        bool  buildup_active;
        bool  drop_detected;
        float repetition_score;    // from self-similarity
        bool  novelty_boundary;    // from novelty peak picker
        float hop_s;
    };

    SegmentType classify(const Input& in) {
        total_elapsed += in.hop_s;

        // Intro detection: low energy at the start
        if (total_elapsed < 30.0f && !has_had_high_energy
            && in.energy_long < in.energy_vlong * 0.7f) {
            return SegmentType::INTRO;
        }

        if (in.energy_long > in.energy_vlong * 1.2f) {
            has_had_high_energy = true;
        }

        // Drop: immediate, from detector
        if (in.drop_detected) {
            return SegmentType::DROP;
        }

        // Buildup: from detector
        if (in.buildup_active) {
            return SegmentType::BREAKDOWN;  // Often a breakdown precedes buildup
            // or: return a BUILDUP type if you want that granularity
        }

        // Breakdown: low energy, low onset rate
        if (in.energy_long < in.energy_vlong * 0.5f
            && in.onset_rate < 2.0f) {
            return SegmentType::BREAKDOWN;
        }

        // Outro: declining energy late in the piece (heuristic: after
        // significant energy has occurred, energy drops below 40% of peak)
        if (has_had_high_energy
            && in.energy_deriv_long < -0.001f  // sustained decline
            && in.energy_long < in.energy_vlong * 0.4f) {
            return SegmentType::OUTRO;
        }

        // Chorus vs Verse: energy level + repetition
        if (in.energy_long > in.energy_vlong * 1.1f
            && in.repetition_score > 0.5f) {
            return SegmentType::CHORUS;
        }

        if (in.repetition_score > 0.5f) {
            return SegmentType::VERSE;
        }

        // Bridge: novel material at medium energy
        if (in.repetition_score < 0.3f
            && in.energy_long > in.energy_vlong * 0.6f) {
            return SegmentType::BRIDGE;
        }

        return SegmentType::UNKNOWN;
    }

    void process(const Input& in) {
        SegmentType proposed = classify(in);

        if (proposed == current) {
            time_in_segment += in.hop_s;
            confidence = std::min(1.0f, time_in_segment / 4.0f);
        } else {
            // Require boundary evidence or sustained mismatch
            // to prevent rapid oscillation
            if (in.novelty_boundary || time_in_segment > 8.0f) {
                current = proposed;
                time_in_segment = 0.0f;
                confidence = 0.2f;  // initial low confidence
            }
        }
    }
};
```

### 8.3 Limitations of Real-Time Labeling

Real-time segment labeling without future information is inherently uncertain. The system cannot distinguish a verse from a chorus until it has heard both and observed the energy contrast. The first time through, labels will be approximate. On repeated sections (which self-similarity detects), labeling becomes more confident.

For the VJ engine, **approximate labels are sufficient**. The visual mapping does not need to display "VERSE" or "CHORUS" text -- it uses the structural classification to select visual presets, color palettes, and effect intensities. A misclassified verse-as-chorus merely results in slightly more energetic visuals, which is an acceptable error mode.

---

## 9. Implementation Strategy

### 9.1 Ring Buffer Architecture

The entire structural analysis system is built on a ring buffer of **feature snapshots** taken at the analysis hop rate. Each snapshot contains all the features needed for structural reasoning:

```cpp
struct FeatureSnapshot {
    // From FEATURES_spectral.md
    float rms_energy;
    float spectral_centroid;
    float spectral_bandwidth;
    float spectral_flatness;
    float spectral_flux;

    // From FEATURES_mfcc_mel.md
    float mfcc[13];

    // From FEATURES_rhythm_tempo.md
    float chroma[12];
    float onset_strength;

    // Computed in this module
    float multi_scale_energy[4];  // short, medium, long, vlong

    // Timestamp
    int   frame_index;
    float time_seconds;
};

static_assert(sizeof(FeatureSnapshot) <= 160,
              "Keep snapshot compact for cache efficiency");
```

At 160 bytes per snapshot and 2048 snapshots:

$$2048 \times 160 = 320 \text{ KB}$$

This fits in L2 cache on all modern CPUs (typically 256 KB - 1 MB per core). Cache residency is critical because self-similarity computation iterates over the entire buffer every frame.

### 9.2 Processing Pipeline

The structural analysis module runs once per analysis hop (every 512-1024 samples). Its internal pipeline:

```
1. Receive raw features from spectral/MFCC/rhythm modules      [0 us, just reads]
2. Push feature snapshot onto ring buffer                        [< 1 us]
3. Update multi-scale energy EMAs                                [< 1 us]
4. Compute novelty values (spectral flux, chroma, MFCC diffs)   [< 1 us]
5. Run novelty fusion + smoothing + peak picking                 [< 2 us]
6. Run buildup detector                                          [< 2 us]
7. Run drop detector                                             [< 1 us]
8. Run breakdown detector                                        [< 1 us]
9. Compute self-similarity row                                   [~12 us]
10. Update repetition scores                                     [< 1 us]
11. Update phrase counter (if beat detected)                     [< 1 us]
12. Run segment labeler                                          [< 1 us]
─────────────────────────────────────────────────────────────────
Total:                                                           ~22 us
```

At 22 microseconds per hop, the structural analysis module consumes approximately **0.1%** of a single core's budget at a 23 ms hop rate. This leaves ample headroom for the upstream feature extraction (spectral analysis, MFCC computation, beat tracking) and downstream visual rendering.

### 9.3 Thread Safety

The structural analysis module should run on the **audio analysis thread**, not the render thread. Results are published to the render thread via a lock-free single-producer/single-consumer (SPSC) queue or an atomic snapshot:

```cpp
struct StructuralState {
    // Atomically published to render thread
    std::atomic<float>       buildup_score{0.0f};
    std::atomic<float>       breakdown_score{0.0f};
    std::atomic<bool>        drop_detected{false};
    std::atomic<int>         segment_type{0};  // cast from SegmentType
    std::atomic<float>       segment_confidence{0.0f};
    std::atomic<float>       repetition_score{0.0f};
    std::atomic<float>       novelty_value{0.0f};
    std::atomic<bool>        section_boundary{false};

    // Multi-scale energy (read by render thread for intensity mapping)
    std::atomic<float>       energy_short{0.0f};
    std::atomic<float>       energy_medium{0.0f};
    std::atomic<float>       energy_long{0.0f};
    std::atomic<float>       energy_vlong{0.0f};

    // Phrase position (for beat-sync effects)
    std::atomic<int>         beat_in_bar{0};
    std::atomic<int>         bar_in_phrase{0};
};
```

Each `std::atomic<float>` uses `memory_order_relaxed` for stores and loads. There is no need for sequencing guarantees between individual fields -- the render thread reads a "best-effort recent" snapshot, which is sufficient for smooth visual interpolation.

### 9.4 Latency Breakdown

| Source | Latency | Notes |
|--------|---------|-------|
| Audio buffer (input) | 5-23 ms | Hop size dependent |
| Novelty Gaussian smoothing | ~46 ms | Half-width of causal kernel |
| Novelty peak confirmation | ~100 ms | Look-back delay |
| Buildup detection | ~2 s | Minimum sustained duration |
| Drop detection | ~0 ms | Instantaneous after buildup ends |
| Segment labeling | ~4 s | Confidence ramp-up |
| Self-similarity | ~0 ms | Computed on current frame |

For beat-reactive and onset-reactive visuals, the latency is dominated by the audio buffer size (5-23 ms). For structural events (section boundaries, buildups, drops), the inherent latency of causal detection (100 ms - 4 s) is acceptable because these events unfold over seconds, not milliseconds.

### 9.5 Calibration and Adaptation

All thresholds ($k$ for peak picking, weights for fusion, EMA time constants, state machine transition conditions) should be exposed as runtime parameters. The calibration system (see IMPL_calibration_adaptation.md) can adjust these based on:

- **Genre detection**: EDM benefits from aggressive buildup/drop detection with high weights on spectral flux. Jazz benefits from emphasizing harmonic change. Classical benefits from multi-scale energy tracking.
- **Running statistics**: Thresholds that are relative to the running mean and standard deviation (e.g., adaptive peak picking) self-calibrate to the dynamic range of the input material.
- **User presets**: VJs should be able to bias the system toward more or fewer section changes, higher or lower drop sensitivity, etc.

---

## 10. Timing Diagram: Full Structural Event Lifecycle

The following illustrates the temporal relationship between structural signals during a typical EDM buildup-to-drop sequence:

```
Time (seconds):  0    2    4    6    8   10   12   14   16
                 |    |    |    |    |    |    |    |    |

RMS (long-term): ─────────╱╱╱╱╱╱╱╱╱╱╱╱╱╱╱████████████████
Spec centroid:   ─────────╱╱╱╱╱╱╱╱╱╱╱╱╱╱╱╱███████████████
Spectral flux:   ─────────╱╱╱╱╱╱╱╱╱╱╱╱╱╱╱╱╱╱╱████████████
Buildup score:   ──────────────▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓│
Drop confidence: ─────────────────────────────█│──────────
Novelty:         ──────────────────────────────█──────────

Segment label:   [    BREAKDOWN     ][  BUILDUP  ][ DROP ][ CHORUS/DROP ]

Visual response: ambient, calm       zoom,       FLASH    full energy
                 slow particles      particles   strobe   beat-reactive
                                     accelerate
```

The buildup score rises from ~t=4s as the derivatives of energy, centroid, and flux are consistently positive. It crosses the 0.5 threshold at ~t=5s and is confirmed as active after 2s of sustained crossing (~t=7s). The buildup continues until ~t=14s when the drop occurs (energy spike + spectral bandwidth explosion). The novelty detector fires simultaneously, marking a section boundary. The segment labeler transitions from BUILDUP to DROP.

---

## 11. Summary of Exported Signals

The structural analysis module exports the following signals to the visualization engine:

| Signal | Type | Range | Update Rate | Use |
|--------|------|-------|-------------|-----|
| `novelty_value` | float | 0+ (z-scored) | Per hop | Edge glow, flash intensity |
| `section_boundary` | bool | true/false | Per hop | Scene transition trigger |
| `boundary_confidence` | float | 0-1 | Per hop | Transition speed/intensity |
| `energy_short` | float | 0+ | Per hop | Beat-reactive size/brightness |
| `energy_medium` | float | 0+ | Per hop | Gesture-level animation speed |
| `energy_long` | float | 0+ | Per hop | Color intensity, saturation |
| `energy_vlong` | float | 0+ | Per hop | Background drift, global mood |
| `energy_ratio_short_med` | float | 0+ | Per hop | Transient/onset "punch" |
| `buildup_score` | float | 0-1 | Per hop | Anticipation effects |
| `buildup_active` | bool | true/false | Per hop | Enable buildup visual mode |
| `buildup_duration_s` | float | 0+ | Per hop | Scale anticipation effects |
| `drop_detected` | bool | one-shot | Per hop | Trigger drop visual explosion |
| `drop_confidence` | float | 0-1 | Per hop | Drop intensity scaling |
| `breakdown_score` | float | 0-1 | Per hop | Ambient/calm visual mode |
| `segment_type` | enum | 8 values | Per hop | Visual preset selection |
| `segment_confidence` | float | 0-1 | Per hop | Blend between presets |
| `repetition_score` | float | 0-1 | Per hop | Visual echo / deja vu effects |
| `beat_in_bar` | int | 0-3 | Per beat | Beat-positional effects |
| `bar_in_phrase` | int | 0-7/15 | Per bar | Phrase-arc effects |

These signals provide a comprehensive, multi-scale, real-time description of musical structure suitable for driving any visual mapping strategy. The total computational cost is approximately 22 microseconds per analysis frame, using approximately 320 KB of memory for the feature history buffer.

---

## References

- Foote, J. (2000). "Automatic Audio Segmentation Using a Measure of Audio Novelty." IEEE ICME.
- McFee, B. & Ellis, D. (2014). "Analyzing Song Structure with Spectral Clustering." ISMIR.
- Serra, J. et al. (2012). "Unsupervised Detection of Music Boundaries by Time Series Structure Features." AAAI.
- Harte, C. et al. (2006). "Detecting Harmonic Change in Musical Audio." Audio and Music Computing.
- Marolt, M. (2004). "A Connectionist Approach to Automatic Transcription of Polyphonic Piano Music." IEEE TMM.
