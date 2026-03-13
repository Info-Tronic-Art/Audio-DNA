# Runtime Calibration and Adaptive Normalization for Live Audio-Visual Performance

> **Scope**: Comprehensive strategies for runtime calibration, adaptive normalization, auto-gain, onset threshold tuning, BPM management, genre detection, silence handling, and preset systems in a real-time audio-visual engine. All algorithms target sub-millisecond overhead per analysis block on modern hardware.
>
> **Cross-references**: [VIDEO_feature_to_visual_mapping.md](VIDEO_feature_to_visual_mapping.md) | [FEATURES_amplitude_dynamics.md](FEATURES_amplitude_dynamics.md) | [FEATURES_rhythm_tempo.md](FEATURES_rhythm_tempo.md) | [REF_genre_parameter_presets.md](REF_genre_parameter_presets.md) | [ARCH_pipeline.md](ARCH_pipeline.md)

---

## Table of Contents

1. [Adaptive RMS Normalization](#1-adaptive-rms-normalization)
2. [Auto-Gain / Input Level Calibration](#2-auto-gain--input-level-calibration)
3. [Onset Threshold Auto-Calibration](#3-onset-threshold-auto-calibration)
4. [BPM Management](#4-bpm-management)
5. [Feature Normalization Engine](#5-feature-normalization-engine)
6. [Genre Detection for Parameter Switching](#6-genre-detection-for-parameter-switching)
7. [Silence / No-Signal Detection](#7-silence--no-signal-detection)
8. [Calibration UI Patterns](#8-calibration-ui-patterns)
9. [Preset System](#9-preset-system)
10. [Runtime Parameter Tuning](#10-runtime-parameter-tuning)

---

## 1. Adaptive RMS Normalization

Raw RMS values are meaningless for visual mapping without context. A quiet folk song and a brick-walled EDM track may differ by 20+ dBFS in average RMS, yet both need to drive visuals across the full 0.0-1.0 range. Adaptive normalization solves this by continuously learning the signal's dynamic range at runtime.

### 1.1 Sliding Window Min/Max Tracking

The simplest adaptive strategy tracks the minimum and maximum RMS observed over a trailing window, then maps the current value linearly into that range:

```
normalized = (rms - rms_min) / (rms_max - rms_min)
```

The window length controls adaptation speed. A 10-second window responds quickly to changes in source material but is susceptible to transient outliers. A 60-second window is more stable but sluggish during transitions. In practice, 15-30 seconds works well for live performance where songs change every 3-5 minutes.

A naive approach stores every RMS value in the window and scans for min/max each frame. This is O(N) per update. A monotonic deque (Lemire's algorithm) reduces this to amortized O(1):

```cpp
#include <deque>
#include <cstdint>

class SlidingMinMax {
public:
    explicit SlidingMinMax(size_t window_size)
        : window_size_(window_size), count_(0) {}

    void push(float value) {
        // Maintain max deque: remove elements smaller than new value
        while (!max_deque_.empty() && max_deque_.back().value <= value)
            max_deque_.pop_back();
        max_deque_.push_back({value, count_});

        // Maintain min deque: remove elements larger than new value
        while (!min_deque_.empty() && min_deque_.back().value >= value)
            min_deque_.pop_back();
        min_deque_.push_back({value, count_});

        ++count_;

        // Evict elements outside the window
        while (max_deque_.front().index + window_size_ <= count_)
            max_deque_.pop_front();
        while (min_deque_.front().index + window_size_ <= count_)
            min_deque_.pop_front();
    }

    float current_max() const { return max_deque_.front().value; }
    float current_min() const { return min_deque_.front().value; }

    float normalize(float value) const {
        float range = current_max() - current_min();
        if (range < 1e-8f) return 0.5f;  // Prevent zero-division
        return std::clamp((value - current_min()) / range, 0.0f, 1.0f);
    }

private:
    struct Entry { float value; uint64_t index; };
    std::deque<Entry> max_deque_;
    std::deque<Entry> min_deque_;
    size_t window_size_;
    uint64_t count_;
};
```

The deque entries are small (8 bytes each) and in practice the deque length stays well under the window size because monotonic eviction removes most entries immediately on insertion. Memory usage is bounded by `window_size * sizeof(Entry)` in the worst case (monotonically increasing/decreasing sequence) but typically uses far less.

### 1.2 Exponential Decay Envelope for Adaptive Range

An alternative to a fixed window is an exponential decay envelope that tracks min and max with asymmetric attack/release times. The max tracker rises instantly on new peaks but decays slowly; the min tracker drops instantly on new lows but rises slowly:

```cpp
class DecayEnvelope {
public:
    DecayEnvelope(float attack_ms, float release_ms, float sample_rate_hz)
        : observed_max_(0.0f), observed_min_(1.0f)
    {
        // Time constants for exponential decay
        float block_rate = sample_rate_hz / 512.0f;  // Assume 512-sample blocks
        alpha_attack_ = 1.0f;  // Instant attack
        alpha_release_max_ = std::exp(-1.0f / (release_ms * 0.001f * block_rate));
        alpha_release_min_ = alpha_release_max_;
    }

    void update(float rms) {
        // Max: instant rise, slow decay toward long-term average
        if (rms > observed_max_) {
            observed_max_ = rms;
        } else {
            observed_max_ = observed_max_ * alpha_release_max_ +
                           rms * (1.0f - alpha_release_max_);
        }

        // Min: instant drop, slow rise toward long-term average
        if (rms < observed_min_) {
            observed_min_ = rms;
        } else {
            observed_min_ = observed_min_ * alpha_release_min_ +
                           rms * (1.0f - alpha_release_min_);
        }
    }

    float normalize(float rms) const {
        float range = observed_max_ - observed_min_;
        if (range < 1e-8f) return 0.5f;
        return std::clamp((rms - observed_min_) / range, 0.0f, 1.0f);
    }

    float observed_max() const { return observed_max_; }
    float observed_min() const { return observed_min_; }

private:
    float observed_max_;
    float observed_min_;
    float alpha_attack_;
    float alpha_release_max_;
    float alpha_release_min_;
};
```

Release times of 5-15 seconds are typical. Shorter release causes the normalization range to "breathe" noticeably during breakdowns; longer release keeps the range stable but adapts slowly to drastically different material.

### 1.3 Percentile-Based Normalization (5th-95th)

Min/max normalization is vulnerable to outliers. A single snare hit can push the max far above the typical range, compressing the rest of the signal into the lower portion of the normalized output. Percentile-based normalization uses the 5th and 95th percentiles as the effective floor and ceiling:

```cpp
#include <algorithm>
#include <array>
#include <cstddef>

class PercentileNormalizer {
public:
    explicit PercentileNormalizer(size_t history_size = 1024,
                                  float low_pct = 0.05f,
                                  float high_pct = 0.95f)
        : history_size_(history_size), write_pos_(0), filled_(0),
          low_pct_(low_pct), high_pct_(high_pct),
          cached_low_(0.0f), cached_high_(1.0f), update_counter_(0)
    {
        history_.resize(history_size, 0.0f);
        sorted_.resize(history_size, 0.0f);
    }

    void push(float value) {
        history_[write_pos_] = value;
        write_pos_ = (write_pos_ + 1) % history_size_;
        if (filled_ < history_size_) ++filled_;

        // Recompute percentiles every 16 pushes to amortize sort cost
        if (++update_counter_ >= 16 && filled_ >= 64) {
            update_counter_ = 0;
            recompute_percentiles();
        }
    }

    float normalize(float value) const {
        float range = cached_high_ - cached_low_;
        if (range < 1e-8f) return 0.5f;
        return std::clamp((value - cached_low_) / range, 0.0f, 1.0f);
    }

private:
    void recompute_percentiles() {
        // Partial copy and sort
        std::copy(history_.begin(), history_.begin() + filled_,
                  sorted_.begin());
        std::sort(sorted_.begin(), sorted_.begin() + filled_);

        size_t lo_idx = static_cast<size_t>(low_pct_ * (filled_ - 1));
        size_t hi_idx = static_cast<size_t>(high_pct_ * (filled_ - 1));
        cached_low_ = sorted_[lo_idx];
        cached_high_ = sorted_[hi_idx];
    }

    std::vector<float> history_;
    std::vector<float> sorted_;
    size_t history_size_;
    size_t write_pos_;
    size_t filled_;
    float low_pct_, high_pct_;
    float cached_low_, cached_high_;
    size_t update_counter_;
};
```

The sort is O(N log N) but only runs every 16 blocks. For a 1024-entry history at 93 Hz block rate, the sort runs roughly 6 times per second. At N=1024, `std::sort` completes in under 10 microseconds on modern hardware -- negligible within the analysis budget. For larger histories, `std::nth_element` can find both percentiles in O(N) average time.

### 1.4 Preventing Zero-Division and Handling Silence

Every normalization path must guard against a degenerate range. The three cases:

1. **Range is zero** (min == max): The signal is constant. Return 0.5 (mid-range) or 0.0 depending on whether the constant signal represents silence or a sustained tone.
2. **Range is near-zero** (< epsilon): Apply a minimum range floor. Use `max(range, epsilon)` where epsilon is typically 1e-6 to 1e-4 in the linear RMS domain (approximately -120 to -80 dBFS).
3. **Signal is silence**: If the RMS drops below the noise floor (see Section 7), freeze the last valid normalization parameters and output 0.0. Do not let the normalizer "learn" silence as the new min -- this causes a massive visual spike when signal returns.

```cpp
float safe_normalize(float value, float observed_min, float observed_max,
                     float noise_floor = 1e-5f) {
    // If we're in silence, output zero -- don't normalize noise
    if (value < noise_floor) return 0.0f;

    float range = observed_max - observed_min;
    // Apply minimum range to prevent division instability
    constexpr float MIN_RANGE = 1e-4f;
    if (range < MIN_RANGE) {
        // Not enough dynamic information yet; scale linearly from zero
        return std::clamp(value / (observed_max + MIN_RANGE), 0.0f, 1.0f);
    }

    return std::clamp((value - observed_min) / range, 0.0f, 1.0f);
}
```

---

## 2. Auto-Gain / Input Level Calibration

### 2.1 Automatic Input Level Detection and Scaling

Different audio sources arrive at wildly different levels. A laptop's built-in microphone picking up room sound may deliver -40 dBFS average RMS, while a direct line-in from a DJ mixer may sit at -6 dBFS. The auto-gain system measures the input level over a calibration window and computes a gain factor to bring the signal to a target operating level.

The calibration sequence:

1. **Measurement phase** (3-10 seconds): Accumulate RMS measurements. Compute the mean and peak RMS in dBFS.
2. **Gain calculation**: `gain_dB = target_dBFS - measured_mean_dBFS`. Clamp gain to a safe range (e.g., -6 dB to +40 dB) to prevent feedback or clipping.
3. **Application**: Apply gain as a linear multiplier in the time domain before any analysis. `gain_linear = pow(10, gain_dB / 20)`.

```cpp
class AutoGain {
public:
    AutoGain(float target_dbfs = -18.0f, float calibration_seconds = 5.0f,
             float sample_rate = 48000.0f, size_t block_size = 512)
        : target_dbfs_(target_dbfs),
          calibration_blocks_(static_cast<size_t>(
              calibration_seconds * sample_rate / block_size)),
          gain_linear_(1.0f), is_calibrated_(false),
          sum_rms_db_(0.0), peak_rms_db_(-100.0f), block_count_(0)
    {}

    // Call once per audio block during calibration
    void feed_block(const float* samples, size_t n) {
        if (is_calibrated_) return;

        float sum_sq = 0.0f;
        for (size_t i = 0; i < n; ++i)
            sum_sq += samples[i] * samples[i];

        float rms = std::sqrt(sum_sq / static_cast<float>(n));
        float rms_db = 20.0f * std::log10(std::max(rms, 1e-10f));

        sum_rms_db_ += rms_db;
        peak_rms_db_ = std::max(peak_rms_db_, rms_db);
        ++block_count_;

        if (block_count_ >= calibration_blocks_) {
            finalize_calibration();
        }
    }

    void finalize_calibration() {
        float mean_rms_db = static_cast<float>(sum_rms_db_ / block_count_);
        float gain_db = target_dbfs_ - mean_rms_db;

        // Clamp gain to prevent damage or instability
        gain_db = std::clamp(gain_db, -6.0f, 40.0f);

        // Check headroom: peak + gain must not exceed 0 dBFS
        float headroom_limit = -peak_rms_db_ - 3.0f;  // 3dB safety margin
        gain_db = std::min(gain_db, headroom_limit);

        gain_linear_ = std::pow(10.0f, gain_db / 20.0f);
        is_calibrated_ = true;
    }

    // Apply gain to a block of samples (in-place)
    void apply(float* samples, size_t n) const {
        for (size_t i = 0; i < n; ++i)
            samples[i] *= gain_linear_;
    }

    float gain_db() const { return 20.0f * std::log10(gain_linear_); }
    float gain_linear() const { return gain_linear_; }
    bool is_calibrated() const { return is_calibrated_; }

    // Allow manual override
    void set_gain_db(float db) {
        gain_linear_ = std::pow(10.0f, db / 20.0f);
        is_calibrated_ = true;
    }

    void reset() {
        is_calibrated_ = false;
        sum_rms_db_ = 0.0;
        peak_rms_db_ = -100.0f;
        block_count_ = 0;
        gain_linear_ = 1.0f;
    }

private:
    float target_dbfs_;
    size_t calibration_blocks_;
    float gain_linear_;
    bool is_calibrated_;
    double sum_rms_db_;
    float peak_rms_db_;
    size_t block_count_;
};
```

### 2.2 Target Loudness Normalization

The target level depends on the analysis chain's expectations. For a visualization engine where feature extractors assume signals in the -24 to -6 dBFS range, a target of -18 dBFS provides adequate headroom (18 dB to clip) while keeping the signal well above quantization noise. Targeting integrated loudness (LUFS) rather than RMS is more perceptually accurate -- see [FEATURES_amplitude_dynamics.md](FEATURES_amplitude_dynamics.md) for the ITU-R BS.1770 loudness algorithm. However, LUFS requires a longer measurement window (at least 400 ms gate) and is more expensive to compute. For a quick auto-gain calibration at startup, RMS is sufficient.

### 2.3 Handling Different Input Sources

| Source | Typical Level | Gain Needed | Notes |
|--------|--------------|-------------|-------|
| Built-in mic (room) | -50 to -30 dBFS | +12 to +32 dB | High noise floor; enable noise gate |
| External mic (close) | -30 to -12 dBFS | 0 to +12 dB | Watch for proximity effect bass boost |
| Line-in (mixer out) | -12 to -3 dBFS | -6 to +6 dB | Cleanest signal; minimal processing |
| System loopback | -20 to -6 dBFS | 0 to +12 dB | Level varies with user volume setting |
| Virtual cable (DAW) | -18 to 0 dBFS | -6 to +12 dB | Often pre-mastered; may be clipping |

The auto-gain system should store a per-source-type gain offset as a starting point, then refine during calibration. This avoids the first few seconds of wild visual behavior while the calibrator measures.

### 2.4 Headroom Management

A critical constraint: the gain must never push the signal above 0 dBFS at any point in the analysis chain. The peak-to-average ratio (crest factor) of music is typically 6-12 dB for compressed modern recordings and 12-20 dB for dynamic acoustic material. The auto-gain system measures both the average RMS and peak level during calibration and ensures that `peak_level_dBFS + gain_dB < -1.0 dBFS` (1 dB safety margin below digital full-scale). If the peak-based headroom constraint limits the achievable gain, the system should report this to the UI so the performer can adjust the source level.

---

## 3. Onset Threshold Auto-Calibration

### 3.1 Adaptive Threshold Based on Running Statistics

A fixed onset detection threshold fails across different material. A threshold tuned for a snare hit in a pop song will either miss every beat in ambient music or fire continuously during a blast-beat. The solution is an adaptive threshold derived from running statistics of the onset detection function (ODF) -- typically the spectral flux or complex spectral difference.

The standard approach: threshold = mean(ODF) + delta, where delta is a multiple of the standard deviation. The mean and standard deviation are computed over a trailing window of ODF values.

### 3.2 Median + N * MAD Threshold

The mean/stddev approach is sensitive to outliers -- a single massive transient inflates the standard deviation and suppresses subsequent detections. The median and Median Absolute Deviation (MAD) are robust alternatives:

```
threshold = median(ODF_window) + N * MAD(ODF_window)
```

where `MAD = median(|ODF_i - median(ODF)|)`. The multiplier N controls sensitivity: N=2.0 is aggressive (more onsets detected), N=4.0 is conservative (only strong transients).

```cpp
#include <vector>
#include <algorithm>
#include <cmath>

class AdaptiveOnsetThreshold {
public:
    AdaptiveOnsetThreshold(size_t window_size = 128, float multiplier = 3.0f)
        : window_size_(window_size), multiplier_(multiplier),
          write_pos_(0), filled_(0)
    {
        odf_history_.resize(window_size, 0.0f);
        sorted_buf_.resize(window_size, 0.0f);
        deviation_buf_.resize(window_size, 0.0f);
    }

    struct ThresholdResult {
        float threshold;
        float median;
        float mad;
        bool onset_detected;
    };

    ThresholdResult update(float odf_value) {
        odf_history_[write_pos_] = odf_value;
        write_pos_ = (write_pos_ + 1) % window_size_;
        if (filled_ < window_size_) ++filled_;

        // Compute median
        std::copy(odf_history_.begin(), odf_history_.begin() + filled_,
                  sorted_buf_.begin());
        std::nth_element(sorted_buf_.begin(),
                        sorted_buf_.begin() + filled_ / 2,
                        sorted_buf_.begin() + filled_);
        float median = sorted_buf_[filled_ / 2];

        // Compute MAD
        for (size_t i = 0; i < filled_; ++i)
            deviation_buf_[i] = std::abs(odf_history_[i] - median);

        std::nth_element(deviation_buf_.begin(),
                        deviation_buf_.begin() + filled_ / 2,
                        deviation_buf_.begin() + filled_);
        float mad = deviation_buf_[filled_ / 2];

        // Scale MAD to approximate stddev for normal distributions
        // (consistency constant 1.4826)
        float scaled_mad = mad * 1.4826f;

        float threshold = median + multiplier_ * scaled_mad;

        // Minimum threshold floor to avoid false positives in silence
        threshold = std::max(threshold, 0.001f);

        return {threshold, median, mad, odf_value > threshold};
    }

    void set_multiplier(float m) { multiplier_ = m; }
    float multiplier() const { return multiplier_; }

    void reset() {
        std::fill(odf_history_.begin(), odf_history_.end(), 0.0f);
        write_pos_ = 0;
        filled_ = 0;
    }

private:
    std::vector<float> odf_history_;
    std::vector<float> sorted_buf_;
    std::vector<float> deviation_buf_;
    size_t window_size_;
    float multiplier_;
    size_t write_pos_;
    size_t filled_;
};
```

The `std::nth_element` calls are O(N) average and avoid a full sort. For a 128-entry window, this is approximately 200 nanoseconds per update -- well within budget.

### 3.3 Sensitivity Adjustment for Different Energy Levels

The multiplier N is the primary sensitivity control. In practice, a single fixed multiplier does not work across all material because the ODF's statistical distribution changes with genre and energy level. An additional adaptation layer adjusts the multiplier based on the onset rate:

- If the onset rate exceeds a maximum (e.g., > 16 onsets/second), increase the multiplier by 0.1 per excess onset/second. This prevents machine-gun triggering during blast-beats or dense electronic passages.
- If the onset rate drops below a minimum (e.g., < 0.5 onsets/second for more than 5 seconds), decrease the multiplier by 0.5 to become more sensitive. This catches subtle onsets in sparse ambient passages.
- Clamp the multiplier to a sane range: [1.5, 6.0].

### 3.4 Per-Genre Sensitivity Presets

Pre-configured multiplier and window-size combinations for common genres:

| Genre | Multiplier (N) | Window Size (blocks) | Min Onset Interval (ms) | Notes |
|-------|----------------|---------------------|------------------------|-------|
| EDM / House | 2.5 | 128 | 80 | Strong transients; lock to kick |
| Drum & Bass | 2.0 | 96 | 50 | Fast breakbeats need shorter interval |
| Ambient | 4.0 | 256 | 200 | Avoid false positives on swells |
| Metal | 2.5 | 96 | 40 | Very dense; may need blast-beat gate |
| Hip-Hop | 3.0 | 128 | 100 | 808 sub-bass can confuse spectral flux |
| Classical | 3.5 | 192 | 150 | Wide dynamic range; pizzicato vs legato |
| Jazz | 3.0 | 160 | 100 | Brushes vs cymbals: different spectral flux |
| Pop (compressed) | 2.0 | 128 | 80 | Low dynamic range; threshold stays tight |

These presets can be selected manually or applied automatically by the genre detector (Section 6).

---

## 4. BPM Management

### 4.1 Tap-Tempo Override

Auto-BPM detection fails or hallucinates in many real-world situations: ambient music with no beat, polyrhythmic passages, songs with tempo changes, or noisy microphone input. A tap-tempo system provides a reliable manual fallback.

```cpp
#include <chrono>
#include <vector>
#include <numeric>

class TapTempo {
public:
    TapTempo(size_t max_taps = 8, float timeout_seconds = 3.0f)
        : max_taps_(max_taps), timeout_seconds_(timeout_seconds) {}

    struct TapResult {
        float bpm;
        int tap_count;
        float confidence;  // 0-1 based on consistency of intervals
        bool valid;
    };

    TapResult tap() {
        auto now = std::chrono::steady_clock::now();

        // Reset if too long since last tap
        if (!tap_times_.empty()) {
            float elapsed = std::chrono::duration<float>(
                now - tap_times_.back()).count();
            if (elapsed > timeout_seconds_) {
                tap_times_.clear();
            }
        }

        tap_times_.push_back(now);

        // Keep only the most recent taps
        while (tap_times_.size() > max_taps_)
            tap_times_.erase(tap_times_.begin());

        if (tap_times_.size() < 2)
            return {0.0f, static_cast<int>(tap_times_.size()), 0.0f, false};

        // Compute intervals
        std::vector<float> intervals;
        for (size_t i = 1; i < tap_times_.size(); ++i) {
            float dt = std::chrono::duration<float>(
                tap_times_[i] - tap_times_[i - 1]).count();
            intervals.push_back(dt);
        }

        // Median interval (robust to one bad tap)
        std::vector<float> sorted_intervals = intervals;
        std::sort(sorted_intervals.begin(), sorted_intervals.end());
        float median_interval = sorted_intervals[sorted_intervals.size() / 2];

        float bpm = 60.0f / median_interval;

        // Confidence based on coefficient of variation
        float mean_interval = std::accumulate(intervals.begin(),
            intervals.end(), 0.0f) / intervals.size();
        float variance = 0.0f;
        for (float iv : intervals)
            variance += (iv - mean_interval) * (iv - mean_interval);
        variance /= intervals.size();
        float cv = std::sqrt(variance) / mean_interval;
        float confidence = std::clamp(1.0f - cv * 5.0f, 0.0f, 1.0f);

        return {bpm, static_cast<int>(tap_times_.size()), confidence,
                tap_times_.size() >= 3};
    }

    void reset() { tap_times_.clear(); }

private:
    std::vector<std::chrono::steady_clock::time_point> tap_times_;
    size_t max_taps_;
    float timeout_seconds_;
};
```

### 4.2 Hybrid Auto/Manual BPM

The production system uses a state machine with three modes:

1. **AUTO**: BPM is fully determined by the beat tracker (see [FEATURES_rhythm_tempo.md](FEATURES_rhythm_tempo.md)). Suitable when the tracker's confidence exceeds a threshold (e.g., > 0.7).
2. **ASSISTED**: Auto-detection runs but the performer can nudge the BPM via tap-tempo. The system uses the manual input to resolve octave ambiguity (half/double) and phase-align the beat grid.
3. **MANUAL**: BPM is entirely manual. Auto-detection still runs in the background for display but does not affect the active BPM.

Transitions between modes should be explicit (button press or MIDI control) rather than automatic, because the performer needs predictable behavior.

### 4.3 BPM Lock: Preventing Half/Double Tempo Jumps

The most common BPM tracker failure mode is octave error: reporting 60 BPM instead of 120, or 240 instead of 120. A BPM lock mechanism prevents these jumps:

```cpp
class BpmLock {
public:
    BpmLock(float initial_bpm = 120.0f, float tolerance = 0.05f)
        : locked_bpm_(initial_bpm), tolerance_(tolerance),
          lock_engaged_(false), confidence_threshold_(0.8f) {}

    float filter(float detected_bpm, float confidence) {
        if (!lock_engaged_) {
            if (confidence > confidence_threshold_) {
                locked_bpm_ = detected_bpm;
                lock_engaged_ = true;
            }
            return detected_bpm;
        }

        // Check if detected BPM is within tolerance of locked BPM
        float ratio = detected_bpm / locked_bpm_;

        // Accept if within tolerance of 1:1
        if (std::abs(ratio - 1.0f) < tolerance_)
            return smooth_update(detected_bpm);

        // Correct half-tempo: detected ~= locked / 2
        if (std::abs(ratio - 0.5f) < tolerance_)
            return smooth_update(detected_bpm * 2.0f);

        // Correct double-tempo: detected ~= locked * 2
        if (std::abs(ratio - 2.0f) < tolerance_)
            return smooth_update(detected_bpm / 2.0f);

        // Correct third-tempo and triple-tempo
        if (std::abs(ratio - 1.0f / 3.0f) < tolerance_)
            return smooth_update(detected_bpm * 3.0f);
        if (std::abs(ratio - 3.0f) < tolerance_)
            return smooth_update(detected_bpm / 3.0f);

        // Reject outlier; return locked BPM
        return locked_bpm_;
    }

    void set_locked_bpm(float bpm) {
        locked_bpm_ = bpm;
        lock_engaged_ = true;
    }

    void unlock() { lock_engaged_ = false; }
    bool is_locked() const { return lock_engaged_; }
    float locked_bpm() const { return locked_bpm_; }

private:
    float smooth_update(float new_bpm) {
        // Exponential smoothing to prevent jitter
        locked_bpm_ = locked_bpm_ * 0.9f + new_bpm * 0.1f;
        return locked_bpm_;
    }

    float locked_bpm_;
    float tolerance_;
    bool lock_engaged_;
    float confidence_threshold_;
};
```

### 4.4 BPM Range Constraints by Genre

The genre system (Section 6) constrains the acceptable BPM range, which drastically reduces octave errors:

| Genre | BPM Range | Typical Center |
|-------|-----------|----------------|
| Ambient | 60 - 100 | 80 |
| Hip-Hop | 70 - 110 | 90 |
| House | 118 - 135 | 125 |
| Techno | 125 - 150 | 138 |
| Drum & Bass | 160 - 180 | 174 |
| Dubstep | 130 - 150 (half-time feel) | 140 |
| Metal | 100 - 220 | 160 |
| Pop | 90 - 140 | 120 |

When the genre is known (either detected or manually selected), the BPM tracker's candidates outside the valid range are octave-corrected (halved or doubled) into range. If no correction maps into range, the candidate is rejected.

### 4.5 BPM History and Confidence Tracking

```cpp
#include <array>

class BpmHistory {
public:
    static constexpr size_t HISTORY_SIZE = 64;

    void push(float bpm, float confidence) {
        bpm_ring_[write_pos_] = bpm;
        conf_ring_[write_pos_] = confidence;
        write_pos_ = (write_pos_ + 1) % HISTORY_SIZE;
        if (filled_ < HISTORY_SIZE) ++filled_;
    }

    // Weighted average BPM using confidence as weight
    float weighted_bpm() const {
        if (filled_ == 0) return 0.0f;
        float sum_wbpm = 0.0f, sum_w = 0.0f;
        for (size_t i = 0; i < filled_; ++i) {
            float w = conf_ring_[i] * conf_ring_[i];  // Square for emphasis
            sum_wbpm += bpm_ring_[i] * w;
            sum_w += w;
        }
        return (sum_w > 1e-8f) ? sum_wbpm / sum_w : 0.0f;
    }

    // Overall confidence: fraction of recent estimates that agree
    float overall_confidence(float tolerance_bpm = 2.0f) const {
        if (filled_ < 4) return 0.0f;
        float ref = weighted_bpm();
        size_t agree = 0;
        for (size_t i = 0; i < filled_; ++i) {
            if (std::abs(bpm_ring_[i] - ref) < tolerance_bpm)
                ++agree;
        }
        return static_cast<float>(agree) / filled_;
    }

    // Stability: is the BPM locked and consistent?
    bool is_stable(float min_confidence = 0.8f) const {
        return filled_ >= 16 && overall_confidence() >= min_confidence;
    }

private:
    std::array<float, HISTORY_SIZE> bpm_ring_{};
    std::array<float, HISTORY_SIZE> conf_ring_{};
    size_t write_pos_ = 0;
    size_t filled_ = 0;
};
```

---

## 5. Feature Normalization Engine

### 5.1 Per-Feature Normalization with Adaptive Bounds

Each extracted feature (RMS, spectral centroid, spectral flux, bass ratio, etc.) has a different natural range and distribution. The normalization engine maintains independent state per feature, tracking the observed range and mapping the raw value into a normalized 0.0-1.0 output suitable for visual parameter driving.

### 5.2 Normalization Curves

Linear normalization maps the raw range uniformly. This is often perceptually wrong -- human perception of loudness is logarithmic, perception of pitch is logarithmic, and perception of brightness is approximately a power function. The normalizer supports multiple mapping curves:

- **Linear**: `out = (x - min) / (max - min)`. Best for features that are already perceptually scaled.
- **Logarithmic**: `out = log(1 + k * linear) / log(1 + k)`. Parameter k controls the curve shape. k=9 gives a mild log curve; k=99 compresses the upper range heavily. Best for RMS, spectral flux, and any energy-domain feature.
- **Power (gamma)**: `out = pow(linear, gamma)`. Gamma < 1 expands the lower range (useful for making quiet passages more visually active). Gamma > 1 compresses the lower range (useful for features like spectral centroid where small changes near the mean are uninteresting).
- **Sigmoid**: `out = 1 / (1 + exp(-k * (linear - 0.5)))`. Creates a soft threshold effect -- values near the center are spread across the full output range while extremes are clipped. Useful for onset detection functions where you want a binary-like response with a soft transition.

### 5.3 Feature History Ring Buffer

Each feature maintains a ring buffer of recent raw values for percentile normalization, statistics, and visualization. The ring buffer size determines the adaptation window:

```cpp
#include <cmath>
#include <algorithm>
#include <vector>

enum class NormCurve {
    LINEAR,
    LOGARITHMIC,
    POWER,
    SIGMOID
};

struct NormConfig {
    NormCurve curve = NormCurve::LINEAR;
    float log_k = 9.0f;           // log curve parameter
    float gamma = 1.0f;           // power curve parameter
    float sigmoid_k = 10.0f;      // sigmoid steepness
    float low_percentile = 0.05f;
    float high_percentile = 0.95f;
    float warmup_seconds = 5.0f;  // period before full adaptation
    float min_range = 1e-6f;      // absolute minimum allowed range
};

class FeatureNormalizer {
public:
    FeatureNormalizer(const std::string& name, NormConfig config,
                     float update_rate_hz, size_t history_seconds = 30)
        : name_(name), config_(config), update_rate_hz_(update_rate_hz),
          warmup_blocks_(static_cast<size_t>(
              config.warmup_seconds * update_rate_hz)),
          block_count_(0)
    {
        size_t history_size = static_cast<size_t>(
            history_seconds * update_rate_hz);
        history_.resize(history_size, 0.0f);
        sorted_buf_.resize(history_size, 0.0f);
        history_size_ = history_size;
    }

    float update(float raw_value) {
        // Store in ring buffer
        history_[write_pos_] = raw_value;
        write_pos_ = (write_pos_ + 1) % history_size_;
        if (filled_ < history_size_) ++filled_;
        ++block_count_;

        // Recompute bounds periodically (every 8 blocks)
        if (block_count_ % 8 == 0 && filled_ >= 16) {
            recompute_bounds();
        }

        return apply_normalization(raw_value);
    }

    float normalize_only(float raw_value) const {
        return apply_normalization(raw_value);
    }

    // Accessors for UI display
    float current_low() const { return cached_low_; }
    float current_high() const { return cached_high_; }
    const std::string& name() const { return name_; }
    const NormConfig& config() const { return config_; }
    void set_config(const NormConfig& c) { config_ = c; }
    bool is_warmed_up() const { return block_count_ >= warmup_blocks_; }

    // Serialization support
    struct State {
        float cached_low;
        float cached_high;
        NormConfig config;
    };

    State save_state() const {
        return {cached_low_, cached_high_, config_};
    }

    void load_state(const State& s) {
        cached_low_ = s.cached_low;
        cached_high_ = s.cached_high;
        config_ = s.config;
    }

private:
    void recompute_bounds() {
        std::copy(history_.begin(), history_.begin() + filled_,
                  sorted_buf_.begin());
        auto begin = sorted_buf_.begin();
        auto end = begin + filled_;

        size_t lo_idx = static_cast<size_t>(
            config_.low_percentile * (filled_ - 1));
        size_t hi_idx = static_cast<size_t>(
            config_.high_percentile * (filled_ - 1));

        std::nth_element(begin, begin + lo_idx, end);
        float new_low = sorted_buf_[lo_idx];

        std::nth_element(begin, begin + hi_idx, end);
        float new_high = sorted_buf_[hi_idx];

        // During warmup, blend toward full adaptation gradually
        if (block_count_ < warmup_blocks_) {
            float t = static_cast<float>(block_count_) / warmup_blocks_;
            // Start with a wide default range, narrow toward measured
            float default_low = 0.0f;
            float default_high = 1.0f;
            cached_low_ = default_low * (1.0f - t) + new_low * t;
            cached_high_ = default_high * (1.0f - t) + new_high * t;
        } else {
            // Smooth transition to new bounds (avoid visual jumps)
            cached_low_ = cached_low_ * 0.95f + new_low * 0.05f;
            cached_high_ = cached_high_ * 0.95f + new_high * 0.05f;
        }
    }

    float apply_normalization(float raw_value) const {
        float range = cached_high_ - cached_low_;
        if (range < config_.min_range) range = config_.min_range;

        float linear = std::clamp(
            (raw_value - cached_low_) / range, 0.0f, 1.0f);

        switch (config_.curve) {
            case NormCurve::LINEAR:
                return linear;

            case NormCurve::LOGARITHMIC: {
                float k = config_.log_k;
                return std::log(1.0f + k * linear) / std::log(1.0f + k);
            }

            case NormCurve::POWER:
                return std::pow(linear, config_.gamma);

            case NormCurve::SIGMOID: {
                float k = config_.sigmoid_k;
                float centered = linear - 0.5f;
                return 1.0f / (1.0f + std::exp(-k * centered));
            }
        }
        return linear;
    }

    std::string name_;
    NormConfig config_;
    float update_rate_hz_;
    size_t warmup_blocks_;
    size_t block_count_ = 0;

    std::vector<float> history_;
    std::vector<float> sorted_buf_;
    size_t history_size_;
    size_t write_pos_ = 0;
    size_t filled_ = 0;

    float cached_low_ = 0.0f;
    float cached_high_ = 1.0f;
};
```

### 5.4 Warm-Up Period Handling

The first 5-10 seconds after startup are problematic: the normalizer has very little data and the percentile estimates are unreliable. During warm-up:

1. The normalization bounds interpolate between a wide default range [0, 1] and the measured percentiles. The interpolation factor ramps linearly from 0 (fully default) to 1 (fully measured) over the warm-up period.
2. Features that are entirely zero during warm-up (e.g., no audio yet) should not update their bounds at all -- the normalizer should remain in its default state until it receives non-silent input.
3. The UI should indicate when each feature normalizer is still warming up (e.g., a progress bar or a yellow indicator).

The warm-up logic is embedded in the `recompute_bounds()` method above. The key insight is that the 0.95/0.05 exponential smoothing on the bounds provides implicit warm-up even after the explicit warm-up period ends: sudden changes in signal character (e.g., switching from ambient to EDM) cause a gradual 2-3 second transition in the normalization bounds rather than an abrupt jump.

### 5.5 C++ Class Design: Orchestrating Multiple Normalizers

```cpp
class FeatureNormalizationEngine {
public:
    // Register a feature with its normalization config
    void register_feature(const std::string& name, NormConfig config) {
        float rate = 48000.0f / 512.0f;  // ~93.75 Hz
        normalizers_.emplace(name,
            FeatureNormalizer(name, config, rate));
    }

    // Update a single feature and get normalized output
    float update(const std::string& name, float raw_value) {
        auto it = normalizers_.find(name);
        if (it == normalizers_.end()) return raw_value;
        return it->second.update(raw_value);
    }

    // Bulk update: pass all features at once
    struct FeatureSnapshot {
        float rms;
        float spectral_centroid;
        float spectral_flux;
        float bass_ratio;
        float high_ratio;
        float onset_strength;
        float bpm_confidence;
    };

    struct NormalizedSnapshot {
        float rms;
        float spectral_centroid;
        float spectral_flux;
        float bass_ratio;
        float high_ratio;
        float onset_strength;
        float bpm_confidence;
    };

    NormalizedSnapshot update_all(const FeatureSnapshot& raw) {
        return {
            update("rms", raw.rms),
            update("spectral_centroid", raw.spectral_centroid),
            update("spectral_flux", raw.spectral_flux),
            update("bass_ratio", raw.bass_ratio),
            update("high_ratio", raw.high_ratio),
            update("onset_strength", raw.onset_strength),
            update("bpm_confidence", raw.bpm_confidence)
        };
    }

    FeatureNormalizer* get(const std::string& name) {
        auto it = normalizers_.find(name);
        return (it != normalizers_.end()) ? &it->second : nullptr;
    }

    // Initialize with sensible defaults
    void init_defaults() {
        register_feature("rms",
            {NormCurve::LOGARITHMIC, 9.0f, 1.0f, 10.0f,
             0.05f, 0.95f, 5.0f, 1e-6f});
        register_feature("spectral_centroid",
            {NormCurve::LOGARITHMIC, 4.0f, 1.0f, 10.0f,
             0.02f, 0.98f, 8.0f, 10.0f});
        register_feature("spectral_flux",
            {NormCurve::POWER, 9.0f, 0.7f, 10.0f,
             0.05f, 0.95f, 5.0f, 1e-6f});
        register_feature("bass_ratio",
            {NormCurve::LINEAR, 9.0f, 1.0f, 10.0f,
             0.02f, 0.98f, 5.0f, 0.01f});
        register_feature("high_ratio",
            {NormCurve::LINEAR, 9.0f, 1.0f, 10.0f,
             0.02f, 0.98f, 5.0f, 0.01f});
        register_feature("onset_strength",
            {NormCurve::SIGMOID, 9.0f, 1.0f, 8.0f,
             0.10f, 0.90f, 5.0f, 1e-6f});
        register_feature("bpm_confidence",
            {NormCurve::LINEAR, 9.0f, 1.0f, 10.0f,
             0.0f, 1.0f, 3.0f, 0.01f});
    }

private:
    std::unordered_map<std::string, FeatureNormalizer> normalizers_;
};
```

---

## 6. Genre Detection for Parameter Switching

### 6.1 Simple Genre Classifier from Audio Features

Full genre classification (MIR research with deep learning) is overkill and too expensive for real-time use. Instead, a lightweight rule-based classifier uses four features aggregated over a 10-30 second window:

1. **Spectral centroid** (mean): Low = bass-heavy genres (hip-hop, dubstep). High = bright genres (metal, pop).
2. **Onset rate** (onsets per second): Low = ambient, high = EDM/metal.
3. **Bass ratio** (energy below 200 Hz / total energy): High = hip-hop, EDM. Low = classical, folk.
4. **BPM range**: The detected BPM, corrected for octave errors, places strong constraints on genre.

```cpp
enum class Genre {
    AMBIENT,
    HIPHOP,
    HOUSE,
    TECHNO,
    DNB,
    DUBSTEP,
    METAL,
    POP,
    CLASSICAL,
    UNKNOWN
};

struct GenreFeatures {
    float spectral_centroid_hz;  // Mean over window
    float onset_rate;            // Onsets per second
    float bass_ratio;            // 0-1
    float bpm;                   // Detected BPM
    float dynamic_range_db;      // Peak-to-average in dB
};

struct GenreResult {
    Genre genre;
    float confidence;  // 0-1
};

class SimpleGenreClassifier {
public:
    GenreResult classify(const GenreFeatures& f) {
        // Score each genre with a simple distance metric
        struct Candidate {
            Genre genre;
            float score;
        };

        std::vector<Candidate> candidates;

        // Ambient: low onset rate, low centroid, slow BPM
        candidates.push_back({Genre::AMBIENT,
            score_feature(f.onset_rate, 0.5f, 1.5f) *
            score_feature(f.spectral_centroid_hz, 1000.0f, 2000.0f) *
            score_bpm_range(f.bpm, 60.0f, 100.0f)});

        // House: moderate onset, moderate bass, 118-135 BPM
        candidates.push_back({Genre::HOUSE,
            score_feature(f.onset_rate, 3.0f, 2.0f) *
            score_feature(f.bass_ratio, 0.3f, 0.15f) *
            score_bpm_range(f.bpm, 118.0f, 135.0f)});

        // Techno: high onset, 125-150 BPM
        candidates.push_back({Genre::TECHNO,
            score_feature(f.onset_rate, 5.0f, 3.0f) *
            score_feature(f.bass_ratio, 0.35f, 0.15f) *
            score_bpm_range(f.bpm, 125.0f, 150.0f)});

        // DnB: very high onset, 160-180 BPM
        candidates.push_back({Genre::DNB,
            score_feature(f.onset_rate, 8.0f, 4.0f) *
            score_bpm_range(f.bpm, 160.0f, 180.0f)});

        // Hip-hop: high bass, moderate onset, 70-110 BPM
        candidates.push_back({Genre::HIPHOP,
            score_feature(f.bass_ratio, 0.45f, 0.15f) *
            score_feature(f.onset_rate, 2.5f, 2.0f) *
            score_bpm_range(f.bpm, 70.0f, 110.0f)});

        // Metal: high centroid, high onset, wide BPM
        candidates.push_back({Genre::METAL,
            score_feature(f.spectral_centroid_hz, 4000.0f, 2000.0f) *
            score_feature(f.onset_rate, 7.0f, 4.0f) *
            score_bpm_range(f.bpm, 100.0f, 220.0f)});

        // Pop: moderate everything, compressed dynamic range
        candidates.push_back({Genre::POP,
            score_feature(f.spectral_centroid_hz, 2500.0f, 1500.0f) *
            score_feature(f.onset_rate, 3.0f, 2.0f) *
            score_feature(f.dynamic_range_db, 8.0f, 5.0f) *
            score_bpm_range(f.bpm, 90.0f, 140.0f)});

        // Classical: high dynamic range, low onset, variable centroid
        candidates.push_back({Genre::CLASSICAL,
            score_feature(f.dynamic_range_db, 20.0f, 8.0f) *
            score_feature(f.onset_rate, 1.5f, 2.0f) *
            score_bpm_range(f.bpm, 40.0f, 180.0f)});

        // Find best match
        auto best = std::max_element(candidates.begin(), candidates.end(),
            [](const Candidate& a, const Candidate& b) {
                return a.score < b.score;
            });

        // Normalize confidence: best score relative to sum
        float total = 0.0f;
        for (auto& c : candidates) total += c.score;
        float confidence = (total > 1e-8f) ?
            best->score / total : 0.0f;

        // Low confidence = UNKNOWN
        if (confidence < 0.25f)
            return {Genre::UNKNOWN, confidence};

        return {best->genre, confidence};
    }

private:
    // Gaussian-like score: peaks at `center`, falls off with `width`
    float score_feature(float value, float center, float width) {
        float d = (value - center) / width;
        return std::exp(-0.5f * d * d);
    }

    // Score 1.0 if BPM is within range, decays outside
    float score_bpm_range(float bpm, float lo, float hi) {
        if (bpm >= lo && bpm <= hi) return 1.0f;
        float dist = (bpm < lo) ? (lo - bpm) : (bpm - hi);
        return std::exp(-0.1f * dist);
    }
};
```

### 6.2 Switching Analysis Parameters Based on Detected Genre

When the genre classifier outputs a new genre with sufficient confidence (> 0.4) and stability (same genre detected for > 10 seconds), the system loads the corresponding parameter preset (see Section 9 and [REF_genre_parameter_presets.md](REF_genre_parameter_presets.md)). The parameters affected include:

- Onset detection multiplier and window size (Section 3.4)
- BPM range constraints (Section 4.4)
- Feature normalization curves and percentile ranges
- Visual mapping sensitivities (see [VIDEO_feature_to_visual_mapping.md](VIDEO_feature_to_visual_mapping.md))

### 6.3 Genre Transition Smoothing

Abrupt parameter changes during genre transitions cause jarring visual discontinuities. All parameter changes are smoothed with a crossfade:

```cpp
class ParameterCrossfader {
public:
    ParameterCrossfader(float transition_seconds = 3.0f, float update_rate_hz = 93.75f)
        : transition_blocks_(static_cast<size_t>(
              transition_seconds * update_rate_hz)),
          blocks_remaining_(0) {}

    void start_transition(float target) {
        source_value_ = current_value_;
        target_value_ = target;
        blocks_remaining_ = transition_blocks_;
    }

    float update() {
        if (blocks_remaining_ == 0) return current_value_;

        float t = 1.0f - static_cast<float>(blocks_remaining_) /
                         transition_blocks_;
        // Smoothstep interpolation for perceptually even transition
        t = t * t * (3.0f - 2.0f * t);

        current_value_ = source_value_ * (1.0f - t) + target_value_ * t;
        --blocks_remaining_;

        return current_value_;
    }

    float value() const { return current_value_; }

private:
    float source_value_ = 0.0f;
    float target_value_ = 0.0f;
    float current_value_ = 0.0f;
    size_t transition_blocks_;
    size_t blocks_remaining_;
};
```

The 3-second smoothstep crossfade ensures that even dramatic parameter changes (e.g., onset multiplier shifting from 2.0 to 4.0 when switching from EDM to ambient) do not produce visible glitches.

---

## 7. Silence / No-Signal Detection

### 7.1 RMS Floor Threshold

The simplest silence detector compares the RMS to a fixed floor. The floor should be set above the system noise level but below the quietest intentional signal. Typical values:

- **Analog microphone**: -60 dBFS (noise floor of a decent condenser mic in a quiet room)
- **Line-in**: -80 dBFS (essentially quantization noise)
- **System loopback**: -90 dBFS (digital silence with dither)

```cpp
class SilenceDetector {
public:
    SilenceDetector(float rms_floor_dbfs = -60.0f,
                    float spectral_flatness_threshold = 0.85f,
                    float hold_seconds = 2.0f,
                    float update_rate_hz = 93.75f)
        : rms_floor_linear_(std::pow(10.0f, rms_floor_dbfs / 20.0f)),
          flatness_threshold_(spectral_flatness_threshold),
          hold_blocks_(static_cast<size_t>(hold_seconds * update_rate_hz)),
          blocks_below_(0), is_silent_(false),
          recovery_confirmed_(false), recovery_blocks_(0)
    {}

    struct SilenceState {
        bool is_silent;
        bool is_recovering;  // Signal just returned
        float silence_duration_seconds;
        float signal_level_db;
    };

    SilenceState update(float rms, float spectral_flatness,
                        float update_rate_hz = 93.75f) {
        float rms_db = 20.0f * std::log10(std::max(rms, 1e-10f));
        bool below_floor = rms < rms_floor_linear_;
        bool spectrally_flat = spectral_flatness > flatness_threshold_;

        // Silence requires BOTH low RMS and flat spectrum
        // (avoids falsely detecting tonal drones as silence)
        bool currently_silent = below_floor && spectrally_flat;

        if (currently_silent) {
            ++blocks_below_;
            recovery_confirmed_ = false;
            recovery_blocks_ = 0;
        } else {
            if (is_silent_ && !recovery_confirmed_) {
                // Require sustained signal for recovery (debounce)
                ++recovery_blocks_;
                if (recovery_blocks_ >= 8) {  // ~85ms
                    recovery_confirmed_ = true;
                    blocks_below_ = 0;
                }
            } else {
                blocks_below_ = 0;
            }
        }

        bool was_silent = is_silent_;
        is_silent_ = blocks_below_ >= hold_blocks_;
        bool recovering = was_silent && recovery_confirmed_;

        return {
            is_silent_,
            recovering,
            static_cast<float>(blocks_below_) / update_rate_hz,
            rms_db
        };
    }

    bool is_silent() const { return is_silent_; }

    void set_floor_dbfs(float db) {
        rms_floor_linear_ = std::pow(10.0f, db / 20.0f);
    }

private:
    float rms_floor_linear_;
    float flatness_threshold_;
    size_t hold_blocks_;
    size_t blocks_below_;
    bool is_silent_;
    bool recovery_confirmed_;
    size_t recovery_blocks_;
};
```

### 7.2 Spectral Silence Detection

RMS alone cannot distinguish between true silence and a very quiet tonal signal (e.g., a fading reverb tail or a sustained pad at low volume). Spectral flatness (the ratio of the geometric mean to the arithmetic mean of the power spectrum) distinguishes noise-like signals (flatness near 1.0) from tonal signals (flatness near 0.0). True silence -- which is actually quantization noise or analog hiss -- has high spectral flatness. A quiet tonal signal has low flatness even at very low RMS.

The silence detector in Section 7.1 requires BOTH low RMS and high spectral flatness to declare silence. This prevents false silence detection during quiet musical passages.

### 7.3 Fade-to-Idle Visual Behavior During Silence

When silence is detected, the visual engine should not abruptly freeze or black out. Instead, a fade-to-idle sequence provides a graceful transition:

1. **0-2 seconds of silence**: Continue displaying the last feature values with slow exponential decay. The visuals gradually dim but maintain their last character.
2. **2-5 seconds**: Cross-fade to an idle animation -- a slowly breathing ambient pattern driven by a sine LFO rather than audio features.
3. **5+ seconds**: Fully in idle mode. The idle animation can be configurable per venue/show.

The fade curve should be stored in the preset system so it can be adjusted per venue.

### 7.4 Signal Recovery Detection

When signal returns after silence, the system must:

1. Suppress the normalizer's adaptation for the first 0.5-1 second -- the initial transient is not representative of the new material's dynamic range.
2. Ramp the visual output from the idle state to the audio-driven state over approximately 0.5 seconds to avoid a visual shock.
3. Reset the BPM tracker (Section 4) because the new material may have a completely different tempo.
4. Optionally trigger a "signal recovered" event for the visual engine to use (e.g., a flash or wipe effect).

---

## 8. Calibration UI Patterns

### 8.1 Input Level Meter with Peak Hold

The input level meter is the most critical UI element. Design:

```
┌─ Input Level ──────────────────────────────────────┐
│                                                     │
│  ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓░░░░░░░░░|░░░░░░ │
│  -60     -48     -36     -24    -12  -6  0  +3 dBFS│
│                                        ↑            │
│                                     Peak Hold       │
│                                                     │
│  Source: [Line-In ▼]    Gain: +6.2 dB  [Auto-Cal]  │
│  RMS: -17.4 dBFS   Peak: -4.1 dBFS   Clip: 0      │
└─────────────────────────────────────────────────────┘
```

Implementation notes:

- The meter bar uses three color zones: green (-60 to -12), yellow (-12 to -3), red (-3 to 0).
- Peak hold indicator (the `|` marker) shows the highest peak in the last 2 seconds. It drops by 1 dB per frame after the hold period.
- The clip counter increments whenever a sample exceeds +/- 0.99. It resets on click.
- The [Auto-Cal] button runs the calibration sequence from Section 2. During calibration, the button text changes to "Calibrating... (3s)" with a countdown.

### 8.2 Feature Range Display

Each normalized feature gets a compact range display:

```
┌─ Feature Normalization ──────────────────────────┐
│                                                   │
│  RMS          [▪▪▪▪▪▪▪▪▪▪▪▪▪▪▪░░░░░]  0.72     │
│               lo: 0.003  hi: 0.142  curve: LOG   │
│                                                   │
│  Centroid     [▪▪▪▪▪▪▪▪▪▪▪░░░░░░░░░]  0.55     │
│               lo: 420 Hz  hi: 8200 Hz  curve: LOG│
│                                                   │
│  Flux         [▪▪▪▪▪▪▪▪▪▪▪▪▪▪▪▪▪░░░]  0.85     │
│               lo: 0.01  hi: 2.4  curve: POW(0.7) │
│                                                   │
│  Bass Ratio   [▪▪▪▪▪▪▪▪░░░░░░░░░░░░]  0.40     │
│               lo: 0.05  hi: 0.65  curve: LIN     │
│                                                   │
│  Onset        [░░░░░░░░░░░░░░░░░░░░░]  0.02     │
│               threshold: 0.34  mult: 3.0         │
│                                                   │
│  BPM          [128.0]  confidence: 0.92  LOCKED  │
│               range: [118 - 135]  genre: HOUSE   │
│                                                   │
│  [Reset All]  [Save Preset]  [Load Preset]       │
└──────────────────────────────────────────────────┘
```

Each feature row is clickable to expand into a detailed editor (Section 8.3).

### 8.3 Manual Normalization Curve Editor

Expanding a feature row reveals a curve editor:

```
┌─ RMS Normalization Curve ────────────────────────┐
│                                                   │
│  1.0 ┤                              ●────────    │
│      │                         ●                  │
│  0.8 ┤                     ●                      │
│      │                 ●                           │
│  0.6 ┤             ●                               │
│      │          ●                                   │
│  0.4 ┤       ●                                      │
│      │     ●                                        │
│  0.2 ┤   ●                                          │
│      │  ●                                            │
│  0.0 ┤●                                              │
│      └──────────────────────────────────────── Raw  │
│      0.0        0.25        0.50       0.75    1.0  │
│                                                     │
│  Curve: [LOG ▼]  k: [9.0 ───●───── ]              │
│  Low %: [5 ──●──── ]  High %: [95 ──────●── ]     │
│                                                     │
│  [Apply]  [Reset to Default]                        │
└─────────────────────────────────────────────────────┘
```

The curve editor draws the transfer function in real-time. A vertical line shows the current raw input value, and a horizontal line shows the corresponding normalized output. Dragging the sliders immediately updates the curve visualization and the live output.

### 8.4 Threshold Visualization

The onset threshold is displayed as an overlay on a scrolling spectrogram or onset detection function plot:

```
┌─ Onset Detection ────────────────────────────────┐
│                                                   │
│  ODF ┤    ╱╲           ╱╲              ╱╲        │
│      │   ╱  ╲    ╱╲   ╱  ╲     ╱╲     ╱  ╲      │
│ ─────┤──╱────╲──╱──╲─╱────╲───╱──╲───╱────╲─── ← threshold
│      │ ╱      ╲╱    ╲╱      ╲ ╱    ╲ ╱      ╲   │
│      │╱                      ╲      ╲         ╲  │
│      └───────────────────────────────────────→ t  │
│                                                   │
│  ✦ = detected onset     ─── = adaptive threshold │
│  Onsets/sec: 2.4    Mult: [3.0 ──●────── ]      │
│  Mode: [Median+MAD ▼]                            │
└──────────────────────────────────────────────────┘
```

Stars (or vertical markers) appear at detected onset positions. The threshold line moves adaptively. The performer can drag the multiplier slider to see the threshold rise/fall in real-time and observe how onset density changes.

### 8.5 One-Button Auto-Calibrate

The [Auto-Calibrate] button runs a combined calibration sequence:

1. Measures input level (3 seconds) and sets auto-gain.
2. Measures feature ranges (5 seconds) and initializes all normalizer bounds.
3. Detects genre (5 seconds) and loads appropriate presets.
4. Estimates BPM and engages BPM lock.
5. Reports results in a summary popup.

Total calibration time: approximately 8 seconds (steps overlap). During calibration, a progress bar shows the current phase. The performer should play representative material -- ideally the loudest and most percussive section of their set.

---

## 9. Preset System

### 9.1 Saving/Loading Calibration Presets

The entire calibration state is serializable to JSON. A preset captures:

- Auto-gain settings (gain_dB, target level)
- Per-feature normalization config (curve type, percentile bounds, min range)
- Per-feature learned bounds (cached_low, cached_high)
- Onset detection parameters (multiplier, window size, min interval)
- BPM settings (locked BPM, range constraints, lock state)
- Genre override (if manually selected)
- Silence detection thresholds
- Visual mapping parameters (see [VIDEO_feature_to_visual_mapping.md](VIDEO_feature_to_visual_mapping.md))

### 9.2 JSON Serialization of Calibration State

```cpp
#include <nlohmann/json.hpp>
#include <fstream>

using json = nlohmann::json;

struct CalibrationPreset {
    std::string name;
    std::string venue;
    std::string input_source;

    // Auto-gain
    float gain_db = 0.0f;
    float target_dbfs = -18.0f;

    // Feature normalizers
    struct FeatureConfig {
        std::string name;
        NormCurve curve;
        float log_k, gamma, sigmoid_k;
        float low_pct, high_pct;
        float cached_low, cached_high;
    };
    std::vector<FeatureConfig> features;

    // Onset detection
    float onset_multiplier = 3.0f;
    size_t onset_window_size = 128;
    float min_onset_interval_ms = 80.0f;

    // BPM
    float locked_bpm = 120.0f;
    float bpm_range_lo = 60.0f;
    float bpm_range_hi = 200.0f;
    bool bpm_locked = false;

    // Genre
    Genre genre_override = Genre::UNKNOWN;

    // Silence
    float silence_floor_dbfs = -60.0f;
    float silence_hold_seconds = 2.0f;

    // Serialize to JSON
    json to_json() const {
        json j;
        j["name"] = name;
        j["venue"] = venue;
        j["input_source"] = input_source;

        j["auto_gain"] = {
            {"gain_db", gain_db},
            {"target_dbfs", target_dbfs}
        };

        json feat_array = json::array();
        for (auto& f : features) {
            feat_array.push_back({
                {"name", f.name},
                {"curve", static_cast<int>(f.curve)},
                {"log_k", f.log_k},
                {"gamma", f.gamma},
                {"sigmoid_k", f.sigmoid_k},
                {"low_percentile", f.low_pct},
                {"high_percentile", f.high_pct},
                {"cached_low", f.cached_low},
                {"cached_high", f.cached_high}
            });
        }
        j["features"] = feat_array;

        j["onset"] = {
            {"multiplier", onset_multiplier},
            {"window_size", onset_window_size},
            {"min_interval_ms", min_onset_interval_ms}
        };

        j["bpm"] = {
            {"locked_bpm", locked_bpm},
            {"range_lo", bpm_range_lo},
            {"range_hi", bpm_range_hi},
            {"locked", bpm_locked}
        };

        j["genre_override"] = static_cast<int>(genre_override);

        j["silence"] = {
            {"floor_dbfs", silence_floor_dbfs},
            {"hold_seconds", silence_hold_seconds}
        };

        return j;
    }

    // Deserialize from JSON
    static CalibrationPreset from_json(const json& j) {
        CalibrationPreset p;
        p.name = j.value("name", "");
        p.venue = j.value("venue", "");
        p.input_source = j.value("input_source", "");

        if (j.contains("auto_gain")) {
            p.gain_db = j["auto_gain"].value("gain_db", 0.0f);
            p.target_dbfs = j["auto_gain"].value("target_dbfs", -18.0f);
        }

        if (j.contains("features")) {
            for (auto& fj : j["features"]) {
                FeatureConfig fc;
                fc.name = fj.value("name", "");
                fc.curve = static_cast<NormCurve>(
                    fj.value("curve", 0));
                fc.log_k = fj.value("log_k", 9.0f);
                fc.gamma = fj.value("gamma", 1.0f);
                fc.sigmoid_k = fj.value("sigmoid_k", 10.0f);
                fc.low_pct = fj.value("low_percentile", 0.05f);
                fc.high_pct = fj.value("high_percentile", 0.95f);
                fc.cached_low = fj.value("cached_low", 0.0f);
                fc.cached_high = fj.value("cached_high", 1.0f);
                p.features.push_back(fc);
            }
        }

        if (j.contains("onset")) {
            p.onset_multiplier = j["onset"].value("multiplier", 3.0f);
            p.onset_window_size = j["onset"].value("window_size", 128);
            p.min_onset_interval_ms = j["onset"].value(
                "min_interval_ms", 80.0f);
        }

        if (j.contains("bpm")) {
            p.locked_bpm = j["bpm"].value("locked_bpm", 120.0f);
            p.bpm_range_lo = j["bpm"].value("range_lo", 60.0f);
            p.bpm_range_hi = j["bpm"].value("range_hi", 200.0f);
            p.bpm_locked = j["bpm"].value("locked", false);
        }

        p.genre_override = static_cast<Genre>(
            j.value("genre_override", static_cast<int>(Genre::UNKNOWN)));

        if (j.contains("silence")) {
            p.silence_floor_dbfs = j["silence"].value(
                "floor_dbfs", -60.0f);
            p.silence_hold_seconds = j["silence"].value(
                "hold_seconds", 2.0f);
        }

        return p;
    }

    // File I/O
    void save_to_file(const std::string& path) const {
        std::ofstream f(path);
        f << to_json().dump(2);
    }

    static CalibrationPreset load_from_file(const std::string& path) {
        std::ifstream f(path);
        json j;
        f >> j;
        return from_json(j);
    }
};
```

### 9.3 Per-Venue Presets

A venue's acoustics, PA system, and monitoring setup create a unique audio environment. A VJ who plays the same venue regularly should store a venue preset that captures:

- **Gain settings**: The venue's mixer output level and the optimal auto-gain compensation.
- **Noise floor**: Different venues have different ambient noise levels. A quiet gallery vs. a noisy club.
- **Bass response**: Venues with heavy sub systems (club) will have much higher bass ratios than a gallery with small monitors. The bass-ratio normalizer bounds should reflect this.
- **Onset sensitivity**: Reverberant venues smear transients, requiring a higher onset multiplier to avoid false triggers from reflections.

The preset manager stores presets in a directory structure:

```
presets/
  venues/
    club_fabric.json
    gallery_tate.json
    festival_main_stage.json
  sources/
    macbook_builtin_mic.json
    focusrite_line_in.json
    system_loopback.json
  genres/
    edm_default.json
    ambient_default.json
    metal_default.json
  custom/
    my_set_2026_03.json
```

### 9.4 Preset Manager

```cpp
#include <filesystem>
#include <map>

namespace fs = std::filesystem;

class PresetManager {
public:
    explicit PresetManager(const std::string& base_dir)
        : base_dir_(base_dir)
    {
        // Create directory structure if it doesn't exist
        for (auto& sub : {"venues", "sources", "genres", "custom"}) {
            fs::create_directories(fs::path(base_dir_) / sub);
        }
        scan_presets();
    }

    void scan_presets() {
        presets_.clear();
        for (auto& entry : fs::recursive_directory_iterator(base_dir_)) {
            if (entry.path().extension() == ".json") {
                std::string key = fs::relative(
                    entry.path(), base_dir_).string();
                try {
                    presets_[key] = CalibrationPreset::load_from_file(
                        entry.path().string());
                } catch (...) {
                    // Skip malformed presets
                }
            }
        }
    }

    CalibrationPreset* get(const std::string& key) {
        auto it = presets_.find(key);
        return (it != presets_.end()) ? &it->second : nullptr;
    }

    void save(const std::string& key,
              const CalibrationPreset& preset) {
        fs::path path = fs::path(base_dir_) / key;
        fs::create_directories(path.parent_path());
        preset.save_to_file(path.string());
        presets_[key] = preset;
    }

    std::vector<std::string> list_presets(
            const std::string& category = "") const {
        std::vector<std::string> result;
        for (auto& [key, _] : presets_) {
            if (category.empty() || key.find(category) == 0)
                result.push_back(key);
        }
        std::sort(result.begin(), result.end());
        return result;
    }

    // Merge two presets: use venue's gain/noise settings with genre's
    // normalization curves
    CalibrationPreset merge(const CalibrationPreset& venue,
                            const CalibrationPreset& genre) {
        CalibrationPreset merged = genre;  // Start with genre settings
        merged.gain_db = venue.gain_db;
        merged.target_dbfs = venue.target_dbfs;
        merged.silence_floor_dbfs = venue.silence_floor_dbfs;
        merged.venue = venue.venue;
        merged.input_source = venue.input_source;
        merged.name = venue.name + " + " + genre.name;
        return merged;
    }

private:
    std::string base_dir_;
    std::map<std::string, CalibrationPreset> presets_;
};
```

---

## 10. Runtime Parameter Tuning

### 10.1 Hot-Reloading Parameters Without Restarting Analysis

The analysis pipeline runs continuously during a performance. Parameters must be changeable without stopping or restarting the audio callback chain. The architecture uses a double-buffered parameter structure:

```cpp
#include <atomic>
#include <memory>

struct AnalysisParams {
    // Onset detection
    float onset_multiplier = 3.0f;
    size_t onset_window_size = 128;
    float min_onset_interval_ms = 80.0f;

    // BPM
    float bpm_range_lo = 60.0f;
    float bpm_range_hi = 200.0f;
    bool bpm_lock_engaged = false;

    // Normalization
    NormCurve rms_curve = NormCurve::LOGARITHMIC;
    float rms_log_k = 9.0f;
    float rms_gamma = 1.0f;

    // Gain
    float gain_linear = 1.0f;

    // Silence
    float silence_floor_dbfs = -60.0f;

    // Feature-specific configs can be added as needed
};

class AtomicParamSwap {
public:
    AtomicParamSwap() {
        buffers_[0] = std::make_unique<AnalysisParams>();
        buffers_[1] = std::make_unique<AnalysisParams>();
        active_.store(0, std::memory_order_relaxed);
    }

    // Called from UI/OSC/MIDI thread: write new params to inactive buffer
    // then swap
    void update(const AnalysisParams& new_params) {
        int inactive = 1 - active_.load(std::memory_order_acquire);
        *buffers_[inactive] = new_params;
        active_.store(inactive, std::memory_order_release);
    }

    // Called from analysis thread: read active params
    const AnalysisParams& read() const {
        int idx = active_.load(std::memory_order_acquire);
        return *buffers_[idx];
    }

private:
    std::unique_ptr<AnalysisParams> buffers_[2];
    std::atomic<int> active_;
};
```

This pattern is lock-free and wait-free on the reader side (the audio analysis thread), which is the real-time critical path. The writer side (UI thread) is not real-time critical and can afford the overhead of copying the parameter struct.

### 10.2 OSC Control of Analysis Parameters

Open Sound Control (OSC) provides a standard protocol for remote parameter control. A VJ can use a tablet running TouchOSC, or a custom Max/MSP patch, to adjust analysis parameters during a performance.

OSC address space for the calibration engine:

```
/calibration/gain/db               float    [-6, +40]
/calibration/gain/auto_calibrate   trigger
/calibration/onset/multiplier      float    [1.5, 6.0]
/calibration/onset/window_size     int      [64, 512]
/calibration/onset/min_interval_ms float    [20, 500]
/calibration/bpm/lock              bool
/calibration/bpm/locked_value      float    [30, 300]
/calibration/bpm/range_lo          float    [30, 200]
/calibration/bpm/range_hi          float    [100, 300]
/calibration/bpm/tap               trigger
/calibration/norm/{feature}/curve  int      [0=LIN, 1=LOG, 2=POW, 3=SIG]
/calibration/norm/{feature}/log_k  float    [1, 100]
/calibration/norm/{feature}/gamma  float    [0.1, 5.0]
/calibration/norm/{feature}/reset  trigger
/calibration/silence/floor_db      float    [-90, -30]
/calibration/genre/override        int      [0-8, -1=auto]
/calibration/preset/save           string   (preset name)
/calibration/preset/load           string   (preset name)
/calibration/preset/list           trigger  → responds with preset names
```

A minimal OSC receiver using `oscpack` (or `liblo`):

```cpp
#include <oscpack/osc/OscReceivedElements.h>
#include <oscpack/ip/UdpSocket.h>
#include <oscpack/osc/OscPacketListener.h>

class CalibrationOscListener : public osc::OscPacketListener {
public:
    CalibrationOscListener(AtomicParamSwap& params, TapTempo& tap)
        : params_(params), tap_(tap) {}

protected:
    void ProcessMessage(const osc::ReceivedMessage& m,
                       const IpEndpointName&) override {
        try {
            std::string addr(m.AddressPattern());
            auto args = m.ArgumentsBegin();

            AnalysisParams p = params_.read();  // Copy current

            if (addr == "/calibration/gain/db") {
                float db = (args++)->AsFloat();
                p.gain_linear = std::pow(10.0f, db / 20.0f);
            }
            else if (addr == "/calibration/onset/multiplier") {
                p.onset_multiplier = std::clamp(
                    (args++)->AsFloat(), 1.5f, 6.0f);
            }
            else if (addr == "/calibration/bpm/lock") {
                p.bpm_lock_engaged = (args++)->AsBool();
            }
            else if (addr == "/calibration/bpm/tap") {
                auto result = tap_.tap();
                if (result.valid) {
                    p.bpm_range_lo = result.bpm - 5.0f;
                    p.bpm_range_hi = result.bpm + 5.0f;
                    p.bpm_lock_engaged = true;
                }
            }
            else if (addr == "/calibration/silence/floor_db") {
                p.silence_floor_dbfs = std::clamp(
                    (args++)->AsFloat(), -90.0f, -30.0f);
            }
            // ... additional handlers

            params_.update(p);

        } catch (osc::Exception& e) {
            // Log error, don't crash
        }
    }

private:
    AtomicParamSwap& params_;
    TapTempo& tap_;
};

// Run the OSC listener on a dedicated thread
void start_osc_listener(AtomicParamSwap& params, TapTempo& tap,
                        int port = 9000) {
    static CalibrationOscListener listener(params, tap);
    static UdpListeningReceiveSocket socket(
        IpEndpointName(IpEndpointName::ANY_ADDRESS, port),
        &listener);
    // Run on a dedicated thread (not shown -- typically std::thread)
    socket.Run();
}
```

### 10.3 MIDI Control of Analysis Parameters

MIDI CC (Control Change) messages provide physical knob/fader control. The mapping is user-configurable, but a default layout for a 16-knob controller:

| CC | Parameter | Range |
|----|-----------|-------|
| CC1 | Gain (dB) | -6 to +40 |
| CC2 | Onset multiplier | 1.5 to 6.0 |
| CC3 | Onset window | 64 to 512 |
| CC4 | BPM range low | 30 to 200 |
| CC5 | BPM range high | 100 to 300 |
| CC6 | RMS curve k | 1 to 100 |
| CC7 | RMS gamma | 0.1 to 5.0 |
| CC8 | Silence floor (dB) | -90 to -30 |
| CC9-CC16 | Per-feature sensitivity | 0 to 1 |

MIDI CC values are 0-127. The mapping converts linearly or logarithmically to the target parameter range:

```cpp
float midi_cc_to_param(int cc_value, float param_min, float param_max,
                       bool log_scale = false) {
    float t = cc_value / 127.0f;
    if (log_scale) {
        // Map linearly in log domain
        float log_min = std::log(std::max(param_min, 1e-6f));
        float log_max = std::log(param_max);
        return std::exp(log_min + t * (log_max - log_min));
    }
    return param_min + t * (param_max - param_min);
}
```

MIDI note messages can trigger discrete actions:

| Note | Action |
|------|--------|
| C3 | Tap tempo |
| D3 | Toggle BPM lock |
| E3 | Auto-calibrate |
| F3 | Reset all normalizers |
| G3-B3 | Load presets 1-5 |

### 10.4 Parameter Smoothing for Control Inputs

Raw MIDI CC and OSC values should not be applied directly -- they cause zipper noise (audible or visible stepping). All control inputs pass through a one-pole lowpass filter before reaching the analysis parameters:

```cpp
class ParamSmoother {
public:
    ParamSmoother(float smoothing_ms = 50.0f, float update_rate_hz = 93.75f)
        : alpha_(1.0f - std::exp(-1.0f /
              (smoothing_ms * 0.001f * update_rate_hz))),
          current_(0.0f), target_(0.0f) {}

    void set_target(float t) { target_ = t; }

    float update() {
        current_ += alpha_ * (target_ - current_);
        return current_;
    }

    float value() const { return current_; }

    void set_immediate(float v) { current_ = v; target_ = v; }

private:
    float alpha_;
    float current_;
    float target_;
};
```

A 50 ms smoothing time eliminates stepping artifacts on MIDI CC faders while keeping the response snappy enough for live performance. For tap-tempo and discrete triggers, bypass the smoother and apply immediately.

---

## Integration Summary

The calibration and adaptation subsystems integrate into the main pipeline (see [ARCH_pipeline.md](ARCH_pipeline.md)) as follows:

```
Audio Callback
    │
    ▼
┌──────────────┐
│  Auto-Gain   │ ← Section 2: gain_linear applied to raw samples
└──────┬───────┘
       │
       ▼
┌──────────────┐
│  Silence     │ ← Section 7: gates the pipeline when no signal
│  Detector    │
└──────┬───────┘
       │ (if not silent)
       ▼
┌──────────────────┐
│  FFT + Feature   │ ← [FEATURES_amplitude_dynamics.md],
│  Extraction      │   [FEATURES_spectral.md]
└──────┬───────────┘
       │
       ▼
┌──────────────────┐
│  Feature         │ ← Section 5: per-feature adaptive normalization
│  Normalization   │
└──────┬───────────┘
       │
       ├──► Onset Detector ← Section 3: adaptive threshold
       ├──► BPM Tracker    ← Section 4: lock, range, tap-tempo
       ├──► Genre Detector ← Section 6: parameter switching
       │
       ▼
┌──────────────────┐
│  Feature Bus     │ → Render thread
│  (normalized)    │   [VIDEO_feature_to_visual_mapping.md]
└──────────────────┘
       ▲
       │
┌──────────────────┐
│  OSC / MIDI /    │ ← Section 10: runtime parameter control
│  UI Controls     │   Section 8: calibration UI
│  Preset System   │   Section 9: save/load state
└──────────────────┘
```

The atomic parameter swap (Section 10.1) ensures that control changes from OSC, MIDI, or the UI reach the analysis thread without locks or priority inversion. The preset system (Section 9) provides persistence across sessions. The genre detector (Section 6) closes the loop by automatically switching presets based on the music itself, with manual override always available.

All code in this document assumes single-precision floating point (`float`), 48 kHz sample rate, 512-sample block size (~10.67 ms per block, ~93.75 Hz analysis rate), and the lock-free architecture described in [ARCH_pipeline.md](ARCH_pipeline.md). Adjust the time constants (smoothing alphas, window sizes, hold times) proportionally if the block size or sample rate differs.
