# VIDEO_feature_to_visual_mapping.md

## Audio Feature to Visual Parameter Mapping: Theory, Implementation, and Practice

> **Scope**: This document covers the complete pipeline from extracted audio features to rendered visual parameters in a real-time music visualization / VJ performance context. It addresses mapping strategies, smoothing, normalization, perceptual correction, event architecture, and multi-feature combination. All code targets C++17 with real-time constraints (sub-millisecond mapping latency at audio buffer boundaries).

> **Cross-references**: [FEATURES_amplitude_dynamics.md](FEATURES_amplitude_dynamics.md) | [FEATURES_spectral.md](FEATURES_spectral.md) | [FEATURES_rhythm_tempo.md](FEATURES_rhythm_tempo.md) | [FEATURES_psychoacoustic.md](FEATURES_psychoacoustic.md) | [VIDEO_opengl_integration.md](VIDEO_opengl_integration.md) | [IMPL_calibration_adaptation.md](IMPL_calibration_adaptation.md)

---

## 1. Complete Audio-Feature-to-Visual-Parameter Mapping Table

The table below is the canonical reference for mapping every extracted audio feature to one or more visual parameters. Each row describes the feature's nature (continuous state vs. discrete event), its typical value range after extraction, the visual parameters it drives, the recommended mapping curve, and practical notes.

### 1.1 Master Mapping Table

| Audio Feature | Type | Raw Range | Visual Parameter(s) | Mapping Curve | Smoothing | Notes |
|---|---|---|---|---|---|---|
| **RMS Energy** | Continuous | 0.0 -- 1.0 (normalized) | Brightness (global scene exposure) | Power (gamma 0.6) | EMA (alpha 0.15) | Logarithmic perception -- see Section 9. Raw RMS feels "jumpy"; gamma < 1 compresses peaks, lifts low-energy detail. |
| | | | Scale (geometry size) | Linear | EMA (alpha 0.08) | Slower alpha prevents size jitter. Map to 0.8--1.4 range to avoid zero-size or comically large geometry. |
| | | | Opacity (layer/element alpha) | Linear | EMA (alpha 0.12) | Useful for fade-on-energy layers. Clamp to 0.05--1.0 so elements never fully vanish (avoids pop-in artifacts). |
| **Spectral Centroid** | Continuous | ~200 Hz -- 8000 Hz | Color temperature (warm-to-cool) | Logarithmic (base 2) | One-Euro (mincutoff 1.0, beta 0.007) | Log mapping because octave perception is logarithmic. Low centroid = warm amber/red; high centroid = cool blue/white. |
| | | | Hue shift (HSV/HSL hue rotation) | Log then linear to 0--360 | One-Euro | Map log(centroid) linearly onto hue wheel. Consider restricting to a palette arc (e.g., 180--300) for aesthetic coherence. |
| **Spectral Flux** | Continuous (bursty) | 0.0 -- variable | Particle emission rate | Exponential (power 2.0) | EMA (alpha 0.25) | Squared mapping amplifies transient moments. High flux = dense particle bursts; low flux = sparse drift. |
| | | | Visual change rate (morph speed, shader parameter velocity) | Linear | EMA (alpha 0.20) | Drives the *speed* of parameter changes, not the parameters themselves. Meta-mapping. |
| **Onset** | Event | Binary (0 or 1 per frame) | Flash (full-screen white overlay, rapid decay) | Threshold trigger | N/A (event) | Trigger a decaying envelope: opacity starts at 0.8, decays via `exp(-t * 12.0)`. Duration ~80ms. |
| | | | Particle burst (instantaneous spawn) | Threshold trigger | N/A | Spawn 50--200 particles at onset, with velocity proportional to onset strength if available. |
| | | | Scene trigger (advance scene index) | Threshold + cooldown | N/A | Minimum 2-second cooldown between scene changes to prevent strobing through scenes on dense onsets. |
| **Beat** | Event (periodic) | Binary per beat | Rhythmic motion (bounce, sway) | Beat-phase sinusoid | Phase-locked | Use beat phase (0.0--1.0 between beats) to drive `sin(phase * 2pi)` or custom envelope. See Section 6. |
| | | | Pulse (scale bump) | Beat-triggered decay | Critically damped spring | Trigger a spring toward 1.3 at each beat, rest at 1.0. Natural overshoot-then-settle. |
| | | | Strobe | Beat-subdivided toggle | N/A | Toggle on 8th or 16th note subdivisions. Use sparingly -- photosensitivity concern. |
| **Bass Energy** (20--250 Hz) | Continuous | 0.0 -- 1.0 | Camera shake (translation noise) | Power (gamma 1.5) | EMA (alpha 0.10) | Apply Perlin noise offset to camera position, scaled by bass energy^1.5. Gamma > 1 suppresses low-level rumble, emphasizes hits. |
| | | | Low-frequency pulsing (bloom radius, vignette) | Linear | EMA (alpha 0.08) | Slow smooth pulse that breathes with the bass. Map to bloom radius 0.5--3.0. |
| | | | Ground effects (floor plane glow, waveform displacement) | Linear | EMA (alpha 0.12) | Bass literally "shakes the ground." Displace ground mesh vertices by bass energy. |
| **Mid Energy** (250--4000 Hz) | Continuous | 0.0 -- 1.0 | Color saturation (HSV S channel) | S-curve (sigmoid) | EMA (alpha 0.15) | Sigmoid prevents washed-out or over-saturated extremes. Map to 0.3--1.0 range. |
| | | | Mid-layer animation speed | Linear | EMA (alpha 0.10) | Multiplier on animation time for mid-frequency-responsive layers. Range 0.5x--2.0x. |
| **Treble Energy** (4000--20000 Hz) | Continuous | 0.0 -- 1.0 | Sparkle (point-light scintillation, star intensity) | Power (gamma 0.5) | EMA (alpha 0.30) | Gamma < 1 (square root) lifts subtle treble detail. High alpha for responsiveness -- treble is perceptually "fast." |
| | | | Edge sharpness (post-process sharpen kernel radius) | Linear | EMA (alpha 0.20) | Sharpen filter intensity 0.0--1.5. High treble = crisp edges; low treble = soft/blurred. |
| | | | High-frequency detail (texture LOD bias, noise octave count) | Linear | One-Euro | Drive fractal noise octave count (2--8) or texture LOD bias (-1.0 to 1.0). |
| **BPM** | Continuous (slow-changing) | 60 -- 200 | Animation speed (global time multiplier) | Linear (BPM / 120.0) | Heavy EMA (alpha 0.01) | Normalize to 120 BPM = 1.0x speed. Very slow smoothing -- BPM should not jitter. |
| | | | Loop timing (visual loop duration) | Inverse (60.0 / BPM) | Same | Sync visual loops to beat period. At 120 BPM, loop = 0.5s per beat. |
| **Key / Chord** | Event (slow) | Chroma class (0--11), chord type | Color palette selection | Lookup table | Debounce 500ms | Map each key to a curated palette. See Section 1.2. Chord quality (major/minor) shifts palette warmth. |
| **Roughness** | Continuous | 0.0 -- 1.0 | Distortion (post-process barrel/pincushion) | Power (gamma 2.0) | EMA (alpha 0.12) | Squared mapping -- mild roughness has minimal visual effect, high roughness gets aggressive. |
| | | | Glitch intensity (scanline offset, color channel separation) | Exponential | EMA (alpha 0.18) | Chromatic aberration offset 0--15px. Block displacement probability proportional to roughness. |
| **MFCC** (coefficients 1--13) | Continuous (vector) | ~-30 to +30 per coeff | Timbre-driven texture selection | Nearest-neighbor in MFCC space | EMA per coefficient (alpha 0.10) | Pre-cluster textures/shaders by MFCC fingerprint. At runtime, find nearest cluster center. See Section 8. |
| **Loudness** (LUFS / ITU-R BS.1770) | Continuous | -60 to 0 dBFS | Overall visual intensity (master brightness multiplier) | Logarithmic (dB is already log) | EMA (alpha 0.05) | Map -40 dBFS to 0.0 intensity, -6 dBFS to 1.0. Slow smoothing for "background" intensity envelope. |

### 1.2 Key-to-Color-Palette Mapping (Scriabin-Inspired with Modifications)

Based on Alexander Scriabin's synesthetic mappings, adapted for digital color spaces:

| Key | Hue (degrees) | Palette Character | Hex Anchor |
|---|---|---|---|
| C | 0 (Red) | Warm, grounded | #FF2020 |
| C# / Db | 30 (Orange-red) | Intense, bright | #FF6020 |
| D | 60 (Yellow) | Joyful, sunny | #FFD020 |
| D# / Eb | 90 (Yellow-green) | Fresh, pastoral | #A0E030 |
| E | 120 (Green) | Calm, natural | #30C040 |
| F | 150 (Cyan-green) | Serene, watery | #20D0A0 |
| F# / Gb | 180 (Cyan) | Ethereal, electric | #20D0D0 |
| G | 210 (Sky blue) | Open, spacious | #2090E0 |
| G# / Ab | 240 (Blue) | Deep, contemplative | #3030E0 |
| A | 270 (Violet) | Passionate, noble | #8020E0 |
| A# / Bb | 300 (Magenta) | Dramatic, bold | #D020C0 |
| B | 330 (Rose) | Tense, resolving | #E02060 |

Each anchor hue generates a 5-color palette: the anchor, anchor +/- 15 degrees at 70% saturation, and two desaturated variants (30% saturation) for backgrounds.

---

## 2. Smoothing Strategies

Raw audio features are inherently noisy. Even after windowed FFT processing, frame-to-frame fluctuation causes visual jitter that reads as "broken" rather than "reactive." Smoothing is not optional -- it is a core part of the mapping pipeline.

### 2.1 Exponential Moving Average (EMA)

The simplest and most commonly used smoother. Each output sample is a weighted blend of the previous output and the new input.

**Formula:**

```
y[n] = alpha * x[n] + (1 - alpha) * y[n-1]
```

Where `alpha` in (0, 1) controls responsiveness. Higher alpha = more responsive, more jitter. Lower alpha = smoother, more lag.

**Choosing alpha:** For a desired time constant `tau` (in seconds) at a given frame rate `fps`:

```
alpha = 1.0 - exp(-1.0 / (tau * fps))
```

A `tau` of 0.1s at 60fps gives alpha ~ 0.154. This means the output reaches ~63% of a step input within 0.1 seconds.

**C++ Implementation:**

```cpp
class EMA {
public:
    explicit EMA(float alpha = 0.15f) : alpha_(alpha), initialized_(false), value_(0.0f) {}

    float process(float input) {
        if (!initialized_) {
            value_ = input;
            initialized_ = true;
        } else {
            value_ = alpha_ * input + (1.0f - alpha_) * value_;
        }
        return value_;
    }

    void setAlpha(float alpha) { alpha_ = std::clamp(alpha, 0.001f, 1.0f); }
    void setTimeConstant(float tau, float fps) {
        alpha_ = 1.0f - std::exp(-1.0f / (tau * fps));
    }
    void reset() { initialized_ = false; }

private:
    float alpha_;
    bool initialized_;
    float value_;
};
```

**Strengths:** Dead simple, single multiply-add per sample, predictable behavior.
**Weaknesses:** Fixed tradeoff between responsiveness and smoothness. Cannot adapt to signal dynamics.

### 2.2 One-Euro Filter

The One-Euro filter (Casiez et al., 2012) is an adaptive low-pass filter that provides minimal smoothing when the signal changes quickly (preserving responsiveness) and heavy smoothing when the signal is nearly stationary (eliminating jitter). This is precisely the behavior wanted for audio-visual mapping.

**Core idea:** The cutoff frequency of a low-pass filter adapts based on the speed (derivative) of the input signal. When the input changes fast, cutoff rises, letting the change through. When static, cutoff drops, rejecting noise.

**Parameters:**
- `mincutoff`: Minimum cutoff frequency (Hz). Lower = more smoothing at rest. Default: 1.0.
- `beta`: Speed coefficient. Higher = more responsiveness to fast changes. Default: 0.007.
- `dcutoff`: Cutoff for the derivative filter. Usually 1.0.

**C++ Implementation:**

```cpp
class OneEuroFilter {
public:
    OneEuroFilter(float rate, float mincutoff = 1.0f, float beta = 0.007f, float dcutoff = 1.0f)
        : rate_(rate), mincutoff_(mincutoff), beta_(beta), dcutoff_(dcutoff),
          initialized_(false), prev_value_(0.0f), prev_derivative_(0.0f) {}

    float process(float input) {
        if (!initialized_) {
            prev_value_ = input;
            prev_derivative_ = 0.0f;
            initialized_ = true;
            return input;
        }

        // Estimate derivative
        float derivative = (input - prev_value_) * rate_;
        float edcoeff = smoothingFactor(dcutoff_);
        float filtered_derivative = edcoeff * derivative + (1.0f - edcoeff) * prev_derivative_;

        // Adaptive cutoff
        float cutoff = mincutoff_ + beta_ * std::abs(filtered_derivative);
        float ecoeff = smoothingFactor(cutoff);

        // Filter the value
        float filtered = ecoeff * input + (1.0f - ecoeff) * prev_value_;

        prev_value_ = filtered;
        prev_derivative_ = filtered_derivative;
        return filtered;
    }

    void reset() { initialized_ = false; }

private:
    float smoothingFactor(float cutoff) const {
        float tau = 1.0f / (2.0f * M_PI * cutoff);
        float te = 1.0f / rate_;
        return 1.0f / (1.0f + tau / te);
    }

    float rate_, mincutoff_, beta_, dcutoff_;
    bool initialized_;
    float prev_value_, prev_derivative_;
};
```

**When to use:** Spectral centroid, MFCC coefficients, and any continuous feature where you need both low-jitter stationarity and fast tracking of genuine changes.

### 2.3 Critically Damped Spring

Models the visual parameter as a mass on a spring, critically damped so it approaches the target without oscillation. Produces a natural, physically plausible motion feel.

**Equations (continuous):**

```
x'' = -2 * omega * x' - omega^2 * (x - target)
```

Where `omega = 2 * pi / halflife_time * ln(2)` controls the speed. Critical damping ratio zeta = 1.0 exactly.

**Discrete integration (semi-implicit Euler, stable for VJ frame rates):**

```cpp
class CriticallyDampedSpring {
public:
    CriticallyDampedSpring(float halflife = 0.1f)
        : halflife_(halflife), position_(0.0f), velocity_(0.0f), initialized_(false) {}

    float process(float target, float dt) {
        if (!initialized_) {
            position_ = target;
            velocity_ = 0.0f;
            initialized_ = true;
            return position_;
        }

        float omega = 2.0f * std::log(2.0f) / std::max(halflife_, 0.001f);
        float exp_term = std::exp(-omega * dt);

        // Exact solution for critically damped spring
        float delta = position_ - target;
        float new_pos = target + (delta + (velocity_ + omega * delta) * dt) * exp_term;
        float new_vel = (velocity_ - (velocity_ + omega * delta) * omega * dt) * exp_term;

        position_ = new_pos;
        velocity_ = new_vel;
        return position_;
    }

    void setHalflife(float halflife) { halflife_ = halflife; }
    void reset() { initialized_ = false; }

    float getVelocity() const { return velocity_; }

private:
    float halflife_;
    float position_;
    float velocity_;
    bool initialized_;
};
```

**When to use:** Beat-driven pulse (scale bump on beat), camera motion, any parameter where you want overshoot-free but organic-feeling transitions. The `halflife` parameter is intuitive: "how long until we're halfway to the target?"

### 2.4 Smoother Comparison Matrix

| Property | EMA | One-Euro | Critically Damped Spring |
|---|---|---|---|
| Parameters | 1 (alpha) | 3 (mincutoff, beta, dcutoff) | 1 (halflife) |
| Adaptive | No | Yes | No |
| Latency at rest | Fixed | Low (heavy smoothing) | Fixed |
| Latency on transient | Fixed | Low (cutoff rises) | Fixed |
| Overshoot | Never | Never | Never (critically damped) |
| Natural motion feel | Mechanical | Adaptive | Physical/organic |
| CPU cost | Minimal | Low | Low |
| Best for | Simple features (RMS, band energy) | Spectral features (centroid, MFCC) | Motion parameters (position, scale) |

---

## 3. Normalization and Calibration

Audio features arrive in arbitrary or session-dependent ranges. A quiet jazz trio and a wall of noise from a doom metal band will produce wildly different RMS ranges. Normalization ensures the mapping pipeline produces consistent visual output regardless of input level.

### 3.1 Fixed Range Normalization

Map a known input range [a, b] to [0, 1]:

```cpp
float normalizeFixed(float value, float minVal, float maxVal) {
    return std::clamp((value - minVal) / (maxVal - minVal), 0.0f, 1.0f);
}
```

**When:** You know the exact range a priori (e.g., spectral centroid is always 20--20000 Hz). Simple and predictable, but fails if the actual signal uses only a fraction of the theoretical range.

### 3.2 Adaptive Min/Max (Sliding Window)

Track the actual min and max over a rolling window. The normalization adapts to the content.

```cpp
class AdaptiveNormalizer {
public:
    AdaptiveNormalizer(float decayRate = 0.999f, float floor = 0.01f)
        : decayRate_(decayRate), floor_(floor),
          runMin_(std::numeric_limits<float>::max()),
          runMax_(std::numeric_limits<float>::lowest()) {}

    float process(float value) {
        // Expand range instantly, contract slowly
        if (value < runMin_) runMin_ = value;
        else runMin_ = runMin_ + (value - runMin_) * (1.0f - decayRate_);

        if (value > runMax_) runMax_ = value;
        else runMax_ = runMax_ - (runMax_ - value) * (1.0f - decayRate_);

        float range = std::max(runMax_ - runMin_, floor_);
        return std::clamp((value - runMin_) / range, 0.0f, 1.0f);
    }

    void reset() {
        runMin_ = std::numeric_limits<float>::max();
        runMax_ = std::numeric_limits<float>::lowest();
    }

private:
    float decayRate_, floor_;
    float runMin_, runMax_;
};
```

**Key design choice:** Range expands instantly (a loud transient immediately becomes the new max) but contracts slowly via the decay rate. This prevents the normalizer from "forgetting" recent dynamics while still adapting over time. The `floor_` parameter prevents division by near-zero during silence.

### 3.3 Percentile Normalization

Ignore outlier spikes by normalizing to the 5th--95th percentile range rather than the true min/max. Implemented via a sorted circular buffer:

```cpp
class PercentileNormalizer {
public:
    PercentileNormalizer(size_t windowSize = 300, float lowPct = 0.05f, float highPct = 0.95f)
        : windowSize_(windowSize), lowPct_(lowPct), highPct_(highPct) {
        buffer_.reserve(windowSize);
    }

    float process(float value) {
        if (buffer_.size() >= windowSize_) {
            buffer_.erase(buffer_.begin()); // ring buffer via deque is better for production
        }
        buffer_.push_back(value);

        auto sorted = buffer_;
        std::sort(sorted.begin(), sorted.end());

        size_t n = sorted.size();
        float low = sorted[static_cast<size_t>(lowPct_ * (n - 1))];
        float high = sorted[static_cast<size_t>(highPct_ * (n - 1))];
        float range = std::max(high - low, 0.001f);

        return std::clamp((value - low) / range, 0.0f, 1.0f);
    }

private:
    size_t windowSize_;
    float lowPct_, highPct_;
    std::vector<float> buffer_;
};
```

**When:** Live performance with unpredictable dynamic range. A single gunshot sound effect should not pin the normalizer's max for the rest of the set. The 95th percentile gracefully ignores it.

### 3.4 Z-Score Normalization

Express the value as standard deviations from the running mean. Useful when you care about *relative deviation* rather than absolute magnitude.

```cpp
class ZScoreNormalizer {
public:
    ZScoreNormalizer(float decayRate = 0.995f)
        : decay_(decayRate), mean_(0.0f), variance_(1.0f), initialized_(false) {}

    float process(float value) {
        if (!initialized_) {
            mean_ = value;
            variance_ = 1.0f;
            initialized_ = true;
            return 0.0f;
        }

        mean_ = decay_ * mean_ + (1.0f - decay_) * value;
        float diff = value - mean_;
        variance_ = decay_ * variance_ + (1.0f - decay_) * diff * diff;

        float stddev = std::sqrt(std::max(variance_, 1e-8f));
        return diff / stddev; // Typically in range [-3, 3]
    }

private:
    float decay_, mean_, variance_;
    bool initialized_;
};
```

Output is typically in [-3, +3]. Map this to [0, 1] via `(zscore + 3.0) / 6.0` if needed.

### 3.5 Histogram Equalization

Ensures that the output is uniformly distributed across [0, 1], maximizing the visual dynamic range regardless of input distribution. Computed over a sliding window by building a CDF:

```cpp
class HistogramEqualizer {
public:
    HistogramEqualizer(size_t bins = 256, size_t windowSize = 600)
        : bins_(bins), windowSize_(windowSize), histogram_(bins, 0) {}

    float process(float value) {
        // Assume input is already in [0, 1]
        float clamped = std::clamp(value, 0.0f, 1.0f);

        // Update sliding window
        size_t bin = static_cast<size_t>(clamped * (bins_ - 1));
        history_.push_back(bin);
        histogram_[bin]++;

        if (history_.size() > windowSize_) {
            histogram_[history_.front()]--;
            history_.pop_front();
        }

        // Compute CDF at this bin
        size_t cumulative = 0;
        for (size_t i = 0; i <= bin; ++i) {
            cumulative += histogram_[i];
        }

        return static_cast<float>(cumulative) / static_cast<float>(history_.size());
    }

private:
    size_t bins_, windowSize_;
    std::vector<size_t> histogram_;
    std::deque<size_t> history_;
};
```

**When:** You want maximum visual contrast at all times. Particularly effective for spectral centroid or MFCC mappings where the raw distribution may cluster tightly.

---

## 4. Mapping Curves

After normalization (input now in [0, 1]), mapping curves shape the *perceptual relationship* between audio intensity and visual intensity.

### 4.1 Curve Types

```
Signal Flow:
  Raw Feature --> [Normalization] --> [0,1] --> [Mapping Curve] --> [0,1] --> [Scale to Output Range] --> Visual Parameter
```

**Linear:**
```
y = x
```
Direct proportionality. Rarely perceptually correct, but useful as a baseline.

**Logarithmic (perceptual loudness):**
```
y = log(1 + x * (base - 1)) / log(base)
```
Expands low-end detail, compresses high end. Use for brightness mapping (matches Weber-Fechner law). Common bases: e, 2, 10.

**Exponential (emphasis on high values):**
```
y = (exp(x * k) - 1) / (exp(k) - 1)
```
Suppresses low-level content, emphasizes peaks. Good for "hit-driven" effects like camera shake.

**Power / Gamma:**
```
y = x^gamma
```
- gamma < 1: Square-root-like, lifts shadows (good for treble sparkle, subtle detail)
- gamma = 1: Linear
- gamma > 1: Emphasizes peaks, suppresses floor (good for bass hits, roughness)

**S-Curve / Sigmoid:**
```
y = 1 / (1 + exp(-k * (x - 0.5)))    // normalized to [0,1]
```
Compresses extremes, expands midrange contrast. Prevents both "washed out" and "slammed to max" visuals. Ideal for saturation, hue mappings.

**Custom Bezier:**
```
Defined by control points (0,0), (cx1,cy1), (cx2,cy2), (1,1)
```
Solved iteratively (Newton-Raphson on the x polynomial to find t, then evaluate y). Gives full artistic control.

### 4.2 Dead Zones and Saturation Zones

Many mappings benefit from a dead zone (input below threshold maps to 0) and/or a saturation zone (input above threshold maps to 1). This prevents visual noise from background audio and allows features to "max out" cleanly.

```
    Output
    1.0 |            ___________
        |           /
        |          /
        |         /
    0.0 |________/
        0.0  dz   sat       1.0  Input
              |    |
        dead zone  saturation
```

### 4.3 C++ Implementation with Curve Selection

```cpp
enum class MappingCurve {
    Linear,
    Logarithmic,
    Exponential,
    Power,
    SCurve,
    Custom
};

struct MappingConfig {
    MappingCurve curve = MappingCurve::Linear;
    float gamma = 1.0f;         // For Power curve
    float logBase = 2.718f;     // For Logarithmic curve
    float expK = 3.0f;          // For Exponential curve
    float sigmoidK = 10.0f;     // For S-curve steepness
    float deadZone = 0.0f;      // Input below this maps to 0
    float saturationZone = 1.0f; // Input above this maps to 1
    float outputMin = 0.0f;     // Output range minimum
    float outputMax = 1.0f;     // Output range maximum
};

class FeatureMapper {
public:
    explicit FeatureMapper(const MappingConfig& config = {}) : config_(config) {}

    float map(float normalizedInput) const {
        // Apply dead zone and saturation
        float x = normalizedInput;
        if (x <= config_.deadZone) return config_.outputMin;
        if (x >= config_.saturationZone) return config_.outputMax;

        // Remap to [0,1] within active zone
        x = (x - config_.deadZone) / (config_.saturationZone - config_.deadZone);

        // Apply curve
        float y = 0.0f;
        switch (config_.curve) {
            case MappingCurve::Linear:
                y = x;
                break;

            case MappingCurve::Logarithmic:
                y = std::log(1.0f + x * (config_.logBase - 1.0f)) / std::log(config_.logBase);
                break;

            case MappingCurve::Exponential:
                y = (std::exp(x * config_.expK) - 1.0f) / (std::exp(config_.expK) - 1.0f);
                break;

            case MappingCurve::Power:
                y = std::pow(x, config_.gamma);
                break;

            case MappingCurve::SCurve: {
                float raw = 1.0f / (1.0f + std::exp(-config_.sigmoidK * (x - 0.5f)));
                float lo = 1.0f / (1.0f + std::exp(-config_.sigmoidK * -0.5f));
                float hi = 1.0f / (1.0f + std::exp(-config_.sigmoidK * 0.5f));
                y = (raw - lo) / (hi - lo); // Normalize sigmoid to exactly [0,1]
                break;
            }

            default:
                y = x;
                break;
        }

        // Scale to output range
        return config_.outputMin + y * (config_.outputMax - config_.outputMin);
    }

    void setConfig(const MappingConfig& config) { config_ = config; }

private:
    MappingConfig config_;
};
```

---

## 5. Threshold vs. Continuous Mapping

### 5.1 Continuous Mapping

The feature value is always driving the visual parameter. Every frame, the audio feature's current value becomes a visual parameter value (after normalization, smoothing, and curve application).

**Characteristics:**
- Visuals are always moving, always reactive
- Feels "alive" and connected to the music
- Risk of visual fatigue if overused on all parameters simultaneously

**Example:** RMS energy continuously drives scene brightness. There is never a moment when brightness is not modulated by audio.

### 5.2 Threshold Mapping (Event-Driven)

The feature is monitored, but visual action only occurs when it crosses a threshold.

**Characteristics:**
- Discrete, punctuated visual events
- Clean, intentional feel
- Risk of "dead" periods if threshold is too high, or visual spam if too low

**Example:** Onset strength > 0.6 triggers a particle burst. Below 0.6, nothing happens.

### 5.3 Hysteresis for Threshold Stability

A single threshold causes rapid toggling when the signal hovers near the threshold value. Hysteresis introduces two thresholds: `high` (to activate) and `low` (to deactivate).

```
    State
    ON  |         ___________
        |        |           |
    OFF |________|           |__________

    Input:  ... rising above HIGH ... falling below LOW ...

    HIGH threshold (e.g., 0.65): must cross this to turn ON
    LOW  threshold (e.g., 0.45): must cross this to turn OFF
    Gap  (hysteresis band): 0.20
```

**C++ Implementation:**

```cpp
class HysteresisThreshold {
public:
    HysteresisThreshold(float highThreshold, float lowThreshold)
        : high_(highThreshold), low_(lowThreshold), state_(false) {
        assert(highThreshold > lowThreshold);
    }

    struct Result {
        bool state;         // Current on/off state
        bool justTriggered; // Rising edge (just went ON)
        bool justReleased;  // Falling edge (just went OFF)
    };

    Result process(float value) {
        bool prevState = state_;

        if (!state_ && value >= high_) {
            state_ = true;
        } else if (state_ && value <= low_) {
            state_ = false;
        }

        return {state_, state_ && !prevState, !state_ && prevState};
    }

    void setThresholds(float high, float low) {
        high_ = high;
        low_ = low;
    }

private:
    float high_, low_;
    bool state_;
};
```

The `justTriggered` field is the event you fire on. The `state` field can drive continuous behavior during the "on" period. A typical hysteresis band of 15--25% of the threshold value eliminates jitter without introducing perceptible latency.

### 5.4 Cooldown Timer

For event triggers that should not fire too rapidly (scene changes, major visual transitions):

```cpp
class CooldownTrigger {
public:
    CooldownTrigger(float cooldownSeconds) : cooldown_(cooldownSeconds), lastTriggerTime_(-1000.0f) {}

    bool tryTrigger(float currentTime) {
        if (currentTime - lastTriggerTime_ >= cooldown_) {
            lastTriggerTime_ = currentTime;
            return true;
        }
        return false;
    }

private:
    float cooldown_;
    float lastTriggerTime_;
};
```

---

## 6. Beat-Synced Quantized Triggers

Beat synchronization transforms audio-reactive visuals from "responsive" to "musical." Without beat sync, visual events can feel randomly timed even when they are technically audio-driven.

### 6.1 Beat Phase

Given a beat tracker that reports the time of each beat, compute the current **beat phase** (0.0 = on the beat, 0.5 = halfway between beats, approaching 1.0 just before the next beat):

```cpp
class BeatPhaseTracker {
public:
    void onBeat(double beatTimeSeconds) {
        prevBeatTime_ = lastBeatTime_;
        lastBeatTime_ = beatTimeSeconds;
        beatPeriod_ = lastBeatTime_ - prevBeatTime_;
    }

    // Returns phase in [0, 1)
    float getPhase(double currentTime) const {
        if (beatPeriod_ <= 0.0) return 0.0f;
        double elapsed = currentTime - lastBeatTime_;
        double phase = elapsed / beatPeriod_;
        return static_cast<float>(phase - std::floor(phase));
    }

    float getBeatPeriod() const { return static_cast<float>(beatPeriod_); }

    // Get subdivision phase (e.g., subdivision=4 for 16th notes in 4/4)
    float getSubdivisionPhase(double currentTime, int subdivision) const {
        float phase = getPhase(currentTime) * subdivision;
        return phase - std::floor(phase);
    }

    // Quantize an event to the nearest beat subdivision
    bool shouldTrigger(double currentTime, int subdivision, float windowFraction = 0.1f) {
        float subPhase = getSubdivisionPhase(currentTime, subdivision);
        return subPhase < windowFraction || subPhase > (1.0f - windowFraction);
    }

private:
    double prevBeatTime_ = 0.0;
    double lastBeatTime_ = 0.0;
    double beatPeriod_ = 0.5; // Default 120 BPM
};
```

### 6.2 Quantizing Visual Events to the Beat Grid

When an onset or other trigger event is detected, instead of firing immediately, quantize it to the nearest beat subdivision:

```
Audio onset detected at phase 0.38 (between 8th notes at 0.25 and 0.50)
Subdivision = 4 (8th notes in 4/4 at the beat level)
Nearest grid point = 0.50
Delay the visual event by (0.50 - 0.38) * beatPeriod seconds
```

This introduces a small delay (typically 20--60ms) but makes the visual feel locked to the musical grid. The perceptual improvement is dramatic.

### 6.3 Bar-Level Triggers

Some visual events should only occur on the downbeat of a bar (beat 1 of every 4 beats in 4/4 time). Track a bar counter:

```cpp
class BarTracker {
public:
    BarTracker(int beatsPerBar = 4) : beatsPerBar_(beatsPerBar), beatCount_(0) {}

    // Call this on every beat
    bool onBeat() {
        bool isDownbeat = (beatCount_ % beatsPerBar_) == 0;
        beatCount_++;
        return isDownbeat;
    }

    int getCurrentBeatInBar() const { return beatCount_ % beatsPerBar_; }
    int getBarCount() const { return beatCount_ / beatsPerBar_; }

private:
    int beatsPerBar_;
    int beatCount_;
};
```

Bar-level triggers drive major visual changes: scene transitions, palette shifts, geometry swaps. Beat-level triggers drive pulses and bounces. Sub-beat triggers drive stutter and strobe.

### 6.4 Anticipation (Pre-trigger)

Human perception of simultaneity is asymmetric. A visual event that occurs 15--30ms *before* the audio beat is perceived as perfectly synchronized, while a visual event 15--30ms *after* feels late. This is because visual processing is faster than auditory processing in the cortex.

Implementation: offset the trigger window backward in time:

```cpp
float anticipationMs = 20.0f; // Configurable per venue/setup
float anticipationPhase = (anticipationMs / 1000.0f) / beatPeriod;
float adjustedPhase = phase + anticipationPhase; // Shift the window earlier
```

This is particularly important for projector-based setups where display latency adds additional delay.

---

## 7. Event-Based vs. State-Based Mapping Architecture

A well-structured mapping engine separates **state mappings** (always active, continuously driven) from **event mappings** (fire-and-forget triggers).

### 7.1 Signal Flow Diagram

```
                    ┌──────────────────────────────────────────────────┐
                    │              MAPPING ENGINE                      │
                    │                                                  │
  Audio Features    │   ┌─────────────────────┐                       │   Visual Parameters
  ─────────────────►│   │  STATE MAPPINGS      │   ┌──────────┐      │──────────────────►
  (per-frame)       │   │  RMS → brightness    │──►│          │      │  (per-frame)
                    │   │  centroid → hue      │   │  OUTPUT  │      │
                    │   │  bass → bloom        │   │  MIXER   │      │
                    │   │  treble → sparkle    │   │          │      │
                    │   └─────────────────────┘   │  Blends   │      │
                    │                              │  state +  │      │
                    │   ┌─────────────────────┐   │  event    │      │
  Audio Events      │   │  EVENT MAPPINGS      │   │  outputs  │      │
  ─────────────────►│   │  onset → flash       │──►│          │      │
  (sporadic)        │   │  beat → pulse        │   │          │      │
                    │   │  drop → scene chg    │   └──────────┘      │
                    │   └─────────────────────┘                       │
                    └──────────────────────────────────────────────────┘
```

### 7.2 State Mapping

A state mapping is a persistent binding from a continuous feature to a visual parameter. It is evaluated every frame.

```cpp
struct StateMapping {
    int featureIndex;         // Which audio feature to read
    int visualParamIndex;     // Which visual parameter to write
    MappingConfig curveConfig;
    EMA smoother;
    AdaptiveNormalizer normalizer;

    float evaluate(float rawFeatureValue) {
        float normalized = normalizer.process(rawFeatureValue);
        float smoothed = smoother.process(normalized);
        return FeatureMapper(curveConfig).map(smoothed);
    }
};
```

### 7.3 Event Mapping

An event mapping monitors a feature for threshold crossings, then triggers a visual event with its own envelope (attack, sustain, decay).

```cpp
struct EventMapping {
    int featureIndex;
    HysteresisThreshold threshold;
    CooldownTrigger cooldown;

    // The visual effect triggered by this event
    struct VisualEvent {
        float intensity;       // Starting intensity
        float decayRate;       // Exponential decay per second
        float currentValue;    // Current decayed value
        bool active;

        void trigger(float strength) {
            intensity = strength;
            currentValue = strength;
            active = true;
        }

        float update(float dt) {
            if (!active) return 0.0f;
            currentValue *= std::exp(-decayRate * dt);
            if (currentValue < 0.01f) {
                active = false;
                currentValue = 0.0f;
            }
            return currentValue;
        }
    };

    VisualEvent effect{1.0f, 8.0f, 0.0f, false};
};
```

### 7.4 The Output Mixer

When both a state mapping and an event mapping target the same visual parameter (e.g., brightness is driven by RMS *and* flashed on onset), the output mixer must combine them. Common strategies:

- **Additive:** `output = clamp(state + event, 0, 1)` -- Events add on top of the state. Simple, effective.
- **Maximum:** `output = max(state, event)` -- Whichever is stronger wins. Prevents the combined value from exceeding natural bounds.
- **Multiplicative:** `output = state * (1 + event)` -- Events amplify the state. A flash during silence does nothing (intentional for some designs).

```cpp
enum class BlendMode { Additive, Maximum, Multiplicative };

float blendOutputs(float stateValue, float eventValue, BlendMode mode) {
    switch (mode) {
        case BlendMode::Additive:
            return std::clamp(stateValue + eventValue, 0.0f, 1.0f);
        case BlendMode::Maximum:
            return std::max(stateValue, eventValue);
        case BlendMode::Multiplicative:
            return std::clamp(stateValue * (1.0f + eventValue), 0.0f, 1.0f);
    }
    return stateValue;
}
```

---

## 8. Multi-Feature Combination

Complex visual behaviors emerge from combining multiple audio features. Rather than mapping each feature to one visual parameter independently, a feature interaction matrix defines how features combine to drive higher-level visual behaviors.

### 8.1 Feature Interaction Matrix

Each row is a "visual behavior" driven by a weighted combination of features:

| Visual Behavior | RMS | Centroid | Flux | Bass | Mid | Treble | Onset | Beat |
|---|---|---|---|---|---|---|---|---|
| Scene intensity | 0.4 | 0.0 | 0.1 | 0.2 | 0.1 | 0.0 | 0.1 | 0.1 |
| Color warmth | 0.0 | -0.6 | 0.0 | 0.2 | 0.0 | -0.2 | 0.0 | 0.0 |
| Visual chaos | 0.1 | 0.0 | 0.3 | 0.0 | 0.0 | 0.2 | 0.3 | 0.1 |
| Atmosphere (calm) | -0.3 | 0.2 | -0.4 | -0.2 | 0.1 | 0.0 | -0.3 | 0.0 |

Negative weights mean "more of this feature reduces this behavior." The matrix is artist-tunable.

### 8.2 C++ Implementation

```cpp
class FeatureInteractionMatrix {
public:
    FeatureInteractionMatrix(size_t numFeatures, size_t numBehaviors)
        : numFeatures_(numFeatures), numBehaviors_(numBehaviors),
          weights_(numBehaviors * numFeatures, 0.0f) {}

    void setWeight(size_t behavior, size_t feature, float weight) {
        weights_[behavior * numFeatures_ + feature] = weight;
    }

    // features: normalized [0,1] values for each feature
    // output: computed behavior values
    void compute(const std::vector<float>& features, std::vector<float>& output) const {
        output.resize(numBehaviors_);
        for (size_t b = 0; b < numBehaviors_; ++b) {
            float sum = 0.0f;
            for (size_t f = 0; f < numFeatures_; ++f) {
                sum += weights_[b * numFeatures_ + f] * features[f];
            }
            output[b] = std::clamp(sum, 0.0f, 1.0f);
        }
    }

private:
    size_t numFeatures_, numBehaviors_;
    std::vector<float> weights_; // Row-major: [behavior][feature]
};
```

### 8.3 MFCC-Based Timbre Clustering

MFCCs (Mel-Frequency Cepstral Coefficients) encode timbral information as a 13-dimensional vector. By pre-clustering a set of visual presets (textures, shaders, color palettes) against reference MFCC vectors, the system can automatically select visuals that "match" the timbre.

**Offline preparation:**
1. Analyze reference audio clips (e.g., "clean guitar," "distorted synth," "voice," "drums") to extract mean MFCC vectors.
2. Associate each cluster center with a visual preset.
3. Store as a lookup table.

**Runtime:**
1. Extract MFCC from the current audio frame.
2. Compute Euclidean distance to each cluster center.
3. Select the nearest cluster (hard switch) or interpolate between the two nearest (soft blend).

```cpp
struct TimbrePreset {
    std::array<float, 13> mfccCenter;
    int visualPresetIndex;
};

int findNearestTimbre(const std::array<float, 13>& currentMFCC,
                      const std::vector<TimbrePreset>& presets) {
    float bestDist = std::numeric_limits<float>::max();
    int bestIndex = 0;

    for (size_t i = 0; i < presets.size(); ++i) {
        float dist = 0.0f;
        for (int c = 0; c < 13; ++c) {
            float d = currentMFCC[c] - presets[i].mfccCenter[c];
            dist += d * d;
        }
        if (dist < bestDist) {
            bestDist = dist;
            bestIndex = static_cast<int>(i);
        }
    }
    return bestIndex;
}
```

---

## 9. Perceptual Considerations

### 9.1 Weber-Fechner Law

The Weber-Fechner law states that the *perceived* change in a stimulus is proportional to the *logarithm* of the physical change. For audio-visual mapping, this means:

- **Doubling** the audio energy does not feel like doubling the brightness.
- A linear mapping from RMS to brightness will feel "jumpy" at low levels and "sluggish" at high levels.
- **Logarithmic mapping** corrects this, making equal *perceived* changes in audio produce equal *perceived* changes in visual output.

```
Perceived change = k * ln(Stimulus / Stimulus_threshold)
```

This is why audio levels are measured in decibels (a logarithmic scale). Visual parameters should follow the same principle.

### 9.2 Stevens' Power Law

Stevens' power law is a refinement:

```
Perceived magnitude = k * (Stimulus)^n
```

where `n` (the Stevens exponent) varies by modality:

| Modality | Stevens Exponent (n) |
|---|---|
| Brightness (visual) | 0.33 -- 0.50 |
| Loudness (audio) | 0.60 -- 0.67 |
| Electric shock | 3.50 |
| Length of line | 1.00 |
| Heaviness | 1.45 |

**Implication for mapping:** If audio loudness has an exponent of ~0.6 and visual brightness has an exponent of ~0.5, then to produce perceptually proportional output:

```
visual_stimulus = (audio_stimulus ^ 0.6) ^ (1/0.5) = audio_stimulus ^ 1.2
```

In practice, a gamma of 1.2 on the mapping from loudness to brightness produces the most "honest" perceptual correspondence. But artistic intent often overrides -- a gamma of 0.6 (sqrt-like) gives a more dramatic, responsive feel at the cost of perceptual accuracy.

### 9.3 Why Logarithmic Mapping Feels Linear

When a VJ says a mapping "feels linear," they usually mean that equal musical changes produce equal visual changes. But the underlying signals are *not* linear -- both audio energy and visual brightness have logarithmic perceptual scales. Therefore, a mathematically logarithmic mapping curve is required to achieve a perceptually linear result.

This is analogous to gamma correction in display technology: monitors apply a power curve (gamma ~2.2) to voltage-to-brightness. To counteract this, images are stored in gamma-corrected (sRGB) space. Audio-visual mapping needs the same kind of correction layer.

### 9.4 Temporal Perception

Human temporal perception is not uniform:

- **Visual event fusion:** Events closer than ~30ms are perceived as simultaneous.
- **Audio-visual sync tolerance:** Audio leading video by up to 45ms is acceptable; audio lagging video by more than 15ms is noticeable (Vatakis & Spence, 2006).
- **Smooth motion threshold:** Below ~24 fps, motion appears as discrete frames. Above ~60 fps, additional frames have diminishing perceptual return.
- **Flash perception:** A visual flash must last at least 30--50ms (2--3 frames at 60fps) to be consciously perceived. Shorter flashes register subliminally but can cause photosensitivity issues.

These thresholds inform smoothing parameter choices and minimum event durations.

### 9.5 Practical Gamma Recommendations

| Feature-to-Visual Mapping | Recommended Gamma | Rationale |
|---|---|---|
| RMS -> Brightness | 0.5 -- 0.7 | Lift subtle dynamics, compress peaks |
| RMS -> Scale | 0.8 -- 1.0 | Scale is perceived nearly linearly |
| Bass -> Camera shake | 1.5 -- 2.0 | Suppress rumble, emphasize hits |
| Treble -> Sparkle | 0.4 -- 0.6 | Lift faint high-frequency content |
| Roughness -> Glitch | 2.0 -- 3.0 | Glitch should only appear on strong roughness |
| Spectral flux -> Particles | 1.5 -- 2.0 | Concentrate emission on actual transients |
| Mid energy -> Saturation | 1.0 (use S-curve instead) | Sigmoid prevents extreme desaturation/oversaturation |

---

## 10. Complete Mapping Engine: Putting It All Together

The following class sketch integrates all the above into a single mapping engine suitable for a real-time VJ system.

```cpp
class MappingEngine {
public:
    struct FeatureSlot {
        EMA smoother;
        AdaptiveNormalizer normalizer;
        float rawValue = 0.0f;
        float normalizedValue = 0.0f;
        float smoothedValue = 0.0f;
    };

    struct VisualOutput {
        float value = 0.0f;
        float eventOverlay = 0.0f;
        BlendMode blendMode = BlendMode::Additive;

        float getFinal() const {
            return blendOutputs(value, eventOverlay, blendMode);
        }
    };

    MappingEngine(size_t numFeatures, size_t numOutputs)
        : features_(numFeatures), outputs_(numOutputs), beatPhase_() {}

    // Called every audio analysis frame (~10-50ms intervals)
    void updateFeatures(const std::vector<float>& rawFeatures) {
        for (size_t i = 0; i < features_.size() && i < rawFeatures.size(); ++i) {
            features_[i].rawValue = rawFeatures[i];
            features_[i].normalizedValue = features_[i].normalizer.process(rawFeatures[i]);
            features_[i].smoothedValue = features_[i].smoother.process(features_[i].normalizedValue);
        }
    }

    // Called every render frame (16.6ms at 60fps)
    void updateOutputs(float dt) {
        // State mappings
        for (auto& mapping : stateMappings_) {
            float featureVal = features_[mapping.featureIndex].smoothedValue;
            float mapped = mapping.mapper.map(featureVal);
            outputs_[mapping.outputIndex].value = mapped;
        }

        // Event mappings (decay active events)
        for (auto& event : activeEvents_) {
            event.currentValue *= std::exp(-event.decayRate * dt);
            if (event.currentValue < 0.005f) {
                event.active = false;
            }
            if (event.active) {
                outputs_[event.targetOutput].eventOverlay = event.currentValue;
            }
        }

        // Clean up expired events
        activeEvents_.erase(
            std::remove_if(activeEvents_.begin(), activeEvents_.end(),
                          [](const ActiveEvent& e) { return !e.active; }),
            activeEvents_.end()
        );
    }

    void triggerEvent(int outputIndex, float intensity, float decayRate) {
        activeEvents_.push_back({outputIndex, intensity, intensity, decayRate, true});
    }

    float getOutput(int index) const {
        return outputs_[index].getFinal();
    }

private:
    struct StateMappingEntry {
        int featureIndex;
        int outputIndex;
        FeatureMapper mapper;
    };

    struct ActiveEvent {
        int targetOutput;
        float intensity;
        float currentValue;
        float decayRate;
        bool active;
    };

    std::vector<FeatureSlot> features_;
    std::vector<VisualOutput> outputs_;
    std::vector<StateMappingEntry> stateMappings_;
    std::vector<ActiveEvent> activeEvents_;
    BeatPhaseTracker beatPhase_;
};
```

### 10.1 Signal Flow Summary

```
┌─────────────┐    ┌──────────────┐    ┌───────────┐    ┌──────────────┐    ┌──────────────┐
│ Audio Input  │───►│  Feature     │───►│ Normalize │───►│   Smooth     │───►│  Map Curve   │
│ (buffers)    │    │  Extraction  │    │ & Calibrate│   │  (EMA/1Euro/ │    │  (gamma/log/ │
└─────────────┘    └──────────────┘    └───────────┘    │   spring)    │    │   sigmoid)   │
                                                        └──────────────┘    └──────┬───────┘
                                                                                   │
                                                                                   ▼
                                                                           ┌──────────────┐
                                                                           │ Output Mixer │
                    ┌──────────────┐    ┌───────────┐    ┌──────────────┐  │ (state +     │
                    │  Event       │───►│ Threshold │───►│  Trigger     │─►│  event blend) │
                    │  Detection   │    │ + Hyster. │    │  + Cooldown  │  └──────┬───────┘
                    │ (onset/beat) │    └───────────┘    │  + Quantize  │         │
                    └──────────────┘                     └──────────────┘         ▼
                                                                           ┌──────────────┐
                                                                           │ Visual Params│
                                                                           │ → GPU/Shader │
                                                                           └──────────────┘
```

---

## 11. Practical Mapping Presets

### 11.1 Preset: "Minimal Reactive"

For subtle, ambient-responsive visuals:

| Feature | Visual Param | Curve | Gamma | Smoothing Alpha |
|---|---|---|---|---|
| RMS | Brightness | Power | 0.5 | 0.05 |
| Centroid | Hue | Log | -- | One-Euro (mincutoff=0.5) |
| Beat | Slow pulse | Spring | halflife=0.3 | -- |

### 11.2 Preset: "High Energy EDM"

For aggressive, beat-locked visuals:

| Feature | Visual Param | Curve | Gamma | Smoothing Alpha |
|---|---|---|---|---|
| RMS | Brightness | Power | 0.8 | 0.25 |
| Bass | Camera shake | Power | 2.0 | 0.15 |
| Onset | Flash | Threshold (0.5) | -- | -- |
| Beat | Scale pulse | Spring | halflife=0.08 | -- |
| Treble | Sparkle | Power | 0.4 | 0.30 |
| Flux | Particle rate | Exponential | -- | 0.20 |
| Roughness | Glitch | Power | 2.5 | 0.12 |

### 11.3 Preset: "Cinematic / Score"

For orchestral or cinematic content:

| Feature | Visual Param | Curve | Gamma | Smoothing Alpha |
|---|---|---|---|---|
| LUFS Loudness | Master intensity | Log | -- | 0.03 |
| Centroid | Color temperature | Log | -- | One-Euro (mincutoff=0.3) |
| Mid energy | Saturation | S-curve (k=8) | -- | 0.08 |
| MFCC | Texture selection | Nearest-neighbor | -- | 0.10 per coeff |
| Key/Chord | Palette | Lookup table | -- | Debounce 1s |

---

## 12. Testing and Tuning Methodology

### 12.1 Reference Signals for Calibration

Test the mapping engine with known signals before live content:

1. **Sine sweep (20 Hz -- 20 kHz, 10s):** Verifies spectral centroid mapping produces smooth color sweep across the entire range.
2. **Click track (120 BPM quarter notes):** Verifies beat detection and pulse timing. Visual pulse should feel locked.
3. **Pink noise at varying levels (-40 dBFS to -6 dBFS, 5 dB steps):** Verifies RMS normalization and brightness mapping produce even perceptual steps.
4. **Silence:** Verifies no residual visual activity. All parameters should settle to their rest values within 2 seconds.
5. **Impulse (single click):** Verifies onset trigger fires once (not multiple times) and decays cleanly.

### 12.2 Live Tuning UI

Expose all mapping parameters (alpha, gamma, thresholds, dead zones) via OSC or MIDI for live adjustment during soundcheck. The VJ should be able to tune responsiveness to the room, the audio source, and the artistic intent without recompiling. See [IMPL_calibration_adaptation.md](IMPL_calibration_adaptation.md) for OSC parameter binding patterns.

---

## References

- Casiez, G., Roussel, N., Vogel, D. (2012). "1Euro Filter: A Simple Speed-based Low-pass Filter for Noisy Input in Interactive Systems." ACM CHI 2012.
- Stevens, S.S. (1957). "On the psychophysical law." Psychological Review, 64(3), 153-181.
- Vatakis, A., Spence, C. (2006). "Audiovisual synchrony perception." Experimental Brain Research.
- ITU-R BS.1770-4 (2015). "Algorithms to measure audio programme loudness and true-peak audio level."
- Scriabin, A. (1911). Prometheus: The Poem of Fire (synesthetic color-key associations).

---

*Cross-references: [FEATURES_amplitude_dynamics.md](FEATURES_amplitude_dynamics.md) for RMS, loudness, and dynamic range extraction. [FEATURES_spectral.md](FEATURES_spectral.md) for centroid, flux, and MFCC computation. [FEATURES_rhythm_tempo.md](FEATURES_rhythm_tempo.md) for beat detection, BPM estimation, and onset detection. [FEATURES_psychoacoustic.md](FEATURES_psychoacoustic.md) for roughness, loudness models, and perceptual weighting. [VIDEO_opengl_integration.md](VIDEO_opengl_integration.md) for GPU-side parameter binding and shader uniform updates. [IMPL_calibration_adaptation.md](IMPL_calibration_adaptation.md) for runtime parameter tuning and adaptive calibration.*
