# Genre-Specific Algorithm Parameter Presets & Real-Time Genre Detection

> **Scope**: Complete parameter preset definitions tuned for eight major music genres, a real-time genre classifier for automatic preset switching, and the auto-switch architecture with hysteresis. All algorithms are constrained to causal (real-time) operation with sub-16ms latency budgets for a VJ / music-visualization engine.
>
> **Cross-references**: [FEATURES_rhythm_tempo.md](FEATURES_rhythm_tempo.md) | [FEATURES_spectral.md](FEATURES_spectral.md) | [FEATURES_amplitude_dynamics.md](FEATURES_amplitude_dynamics.md) | [IMPL_calibration_adaptation.md](IMPL_calibration_adaptation.md) | [LIB_aubio.md](LIB_aubio.md)

---

## Table of Contents

1. [Techno / Electronic](#1-techno--electronic-125-135-bpm)
2. [House](#2-house-120-130-bpm)
3. [Drum & Bass](#3-drum--bass-170-180-bpm)
4. [Ambient](#4-ambient-no-fixed-bpm)
5. [Hip Hop](#5-hip-hop-80-100-bpm)
6. [Rock / Metal](#6-rock--metal-100-180-bpm)
7. [Classical](#7-classical-variable-tempo)
8. [Pop](#8-pop-100-130-bpm)
9. [Consolidated Parameter Tables](#9-consolidated-parameter-tables)
10. [Real-Time Genre Detection](#10-real-time-genre-detection)
11. [Auto-Switch System](#11-auto-switch-system)

---

## 1. Techno / Electronic (125-135 BPM)

### 1.1 Rhythmic Profile

Techno is defined by a relentless four-on-the-floor kick pattern. The kick drum occupies the sub-bass (40-80 Hz fundamental) and low-mid (80-200 Hz body) bands. Inter-onset intervals are almost perfectly regular at the beat period, typically 444-500 ms (120-135 BPM). The rhythmic certainty of techno makes it the ideal baseline genre for beat-locked visualizations.

**BPM range constraint**: 120-140 BPM. Set the tempo tracker's candidate range accordingly:

```
bpm_min = 120.0
bpm_max = 140.0
period_min_samples = (60.0 / bpm_max) * sample_rate  // ~18,900 at 44100
period_max_samples = (60.0 / bpm_min) * sample_rate  // ~22,050 at 44100
```

**Beat tracking confidence**: For techno, beat-tracking autocorrelation peaks should exceed 0.85. If confidence drops below 0.6, the track is likely in a breakdown or transitional section -- hold the last locked tempo rather than searching.

### 1.2 Onset Detection Configuration

Use the **spectral flux** method with emphasis on the low-frequency region (0-200 Hz). Standard broadband spectral flux will fire on hi-hats and percussive transients throughout the bar, but for kick isolation, restrict the flux computation to FFT bins below 200 Hz:

```
bin_max_kick = floor(200.0 / (sample_rate / fft_size))
// For fft_size=2048, sr=44100: bin 9
```

The onset detection threshold should be set relatively high because kick transients are strong and unambiguous:

```
onset_threshold = 1.5      // multiples of median ODF over trailing 2s window
onset_silence_threshold = -60.0  // dBFS below which no onset fires
min_inter_onset_ms = 200   // suppress double-triggers (corresponds to 300 BPM ceiling)
```

For hi-hat onsets (used for secondary visual triggering), apply a separate high-frequency spectral flux (8-16 kHz) with a lower threshold of 0.8.

### 1.3 Bass Analysis

Sub-bass (20-60 Hz) is the dominant spectral region. Techno kick drums are typically tuned to specific pitches (commonly E1 = 41.2 Hz, F1 = 43.7 Hz, G1 = 49.0 Hz). The sub-bass energy contour drives the primary visual pulse.

**Sidechain detection**: In produced techno, sidechain compression creates a characteristic "pumping" envelope on pads and synths. This manifests as a periodic dip in mid-range (200-2000 Hz) energy synchronized to the kick. Detect sidechain by measuring the cross-correlation between inverted sub-bass energy and mid-range energy:

```
sidechain_depth = correlate(invert(rms_subbass), rms_midrange)
```

A correlation above 0.7 indicates active sidechain compression, which is a strong visual cue (use it to modulate background brightness inversely to kick energy).

**Recommended FFT size**: 2048 at 44100 Hz. This yields a bin resolution of 21.5 Hz, sufficient to resolve the fundamental kick frequency from the first harmonic. An FFT of 1024 gives 43 Hz resolution -- marginal for sub-bass analysis. 4096 provides better resolution (10.7 Hz/bin) but increases latency to 92.9 ms, which is unacceptable for real-time visualization at 60 fps.

### 1.4 Spectral Focus

Techno features prominent filter sweeps (low-pass and band-pass) and resonance peaks. Track the **spectral centroid** over time to detect sweeps:

```
sweep_rate = d(spectral_centroid) / dt
```

A rising centroid over 1-4 seconds at a rate exceeding 500 Hz/s strongly indicates a filter opening. Resonance appears as a narrow spectral peak that moves in frequency -- detect via spectral kurtosis or by tracking the peak magnitude bin in the 200-8000 Hz range.

### 1.5 Visual Mapping

| Audio Feature | Visual Parameter | Mapping |
|---|---|---|
| Sub-bass RMS (20-60 Hz) | Global scale / pulse | Linear, attack 5ms, release 80ms |
| Kick onset | Flash / strobe trigger | Binary, 30ms hold |
| Spectral centroid | Hue rotation | Linear map to 0-360 degrees |
| Centroid derivative (sweep rate) | Color sweep speed | Proportional |
| Hi-hat onset rate | Particle density | Linear, clamped 0-16 per beat |
| Sidechain depth | Background modulation | Inverted pump, 0-50% brightness |
| Mid-high RMS (2-8 kHz) | Texture detail | Logarithmic |

---

## 2. House (120-130 BPM)

### 2.1 Rhythmic Profile

House music shares techno's 4/4 kick pattern but typically sits at a slightly lower tempo range (120-128 BPM, with some deep house as low as 118). The key distinguishing features are greater mid-range content (piano chords, organ stabs, vocal hooks) and prominent open hi-hat patterns (usually on the off-beat eighth notes).

**BPM range**: 118-132. The beat tracker can use the same 4/4 lock strategy as techno, but with a tighter confidence threshold of 0.80 since house grooves occasionally introduce swing.

### 2.2 Vocal Detection via Spectral Formant Tracking

Vocals are a primary element in house music. Detect vocal presence by tracking the first two formant frequencies (F1 and F2):

```
F1 range: 300-900 Hz   (vowel openness)
F2 range: 800-2500 Hz  (vowel frontness)
```

Implementation: compute the spectral envelope via LPC (Linear Predictive Coding) analysis, order 12-16, on the 200-4000 Hz band. The LPC coefficients yield formant candidates via polynomial root finding. When two stable formants persist for more than 200 ms within the F1/F2 ranges above, flag vocal presence.

A simpler proxy: compute the **spectral flatness** in the 300-3000 Hz band. Vocal content is harmonically rich (low flatness, < 0.2), while noise/drums are spectrally flat (> 0.5). Combine with an energy threshold to distinguish vocals from silence.

```cpp
float vocal_likelihood(const float* spectrum, int bin_300hz, int bin_3000hz) {
    float geometric_mean = 0.0f;
    float arithmetic_mean = 0.0f;
    int count = bin_3000hz - bin_300hz;

    for (int k = bin_300hz; k < bin_3000hz; ++k) {
        float mag = std::max(spectrum[k], 1e-10f);
        geometric_mean += std::log(mag);
        arithmetic_mean += mag;
    }
    geometric_mean = std::exp(geometric_mean / count);
    arithmetic_mean /= count;

    float flatness = geometric_mean / (arithmetic_mean + 1e-10f);
    // Low flatness + sufficient energy = vocal likely
    float energy = arithmetic_mean;
    float vocal_score = (1.0f - flatness) * std::min(energy / 0.01f, 1.0f);
    return vocal_score;  // 0.0 = no vocal, 1.0 = strong vocal presence
}
```

### 2.3 Hi-Hat Pattern Analysis

House hi-hat patterns are crucial rhythmic identifiers. Measure onset density in the 6-16 kHz band over one bar (approximately 2 seconds at 120 BPM). Typical patterns:

- **Straight eighths**: 8 onsets per bar, evenly spaced at half-beat intervals
- **Off-beat open hi-hat**: Strong onset on every off-beat (the "and" of each beat), with spectral energy sustaining 50-100 ms (open hat character)
- **16th-note pattern**: 16 onsets per bar, characteristic of disco-influenced house

Classify the hi-hat pattern by computing the inter-onset interval histogram within the high-frequency band and checking for peaks at beat/2, beat/4, or 3*beat/4 (for shuffle).

### 2.4 Chord Progression Tracking via Chroma

House music features recognizable harmonic progressions (often I-V-vi-IV or ii-V-I patterns). Extract a 12-dimensional chroma vector from each frame:

```
chroma[p] = sum over octaves of |X(k)|^2 where pitch_class(k) == p
```

Where `pitch_class(k) = round(12 * log2(f_k / 440)) mod 12`. Smooth the chroma vector with a 500 ms EMA to avoid transient noise. The chroma profile can drive color palette selection (major key = warm colors, minor key = cool colors).

### 2.5 Visual Mapping

| Audio Feature | Visual Parameter | Mapping |
|---|---|---|
| Sub-bass RMS (20-80 Hz) | Pulse scale | Same as techno, slightly softer release (120ms) |
| Vocal likelihood | Spotlight / focus effect | Proportional, attack 50ms |
| Hi-hat onset density | Background animation speed | Linear, 0.5x to 3x base speed |
| Chroma major/minor ratio | Color temperature | Warm/cool palette blend |
| Mid-range energy (200-2000 Hz) | Glow intensity | Logarithmic |
| Off-beat accent strength | Sway / oscillation amplitude | Proportional |

---

## 3. Drum & Bass (170-180 BPM)

### 3.1 Rhythmic Profile

Drum & Bass operates at 165-185 BPM with complex syncopated breakbeats. Unlike techno's steady kick, DnB drums are dense, polyrhythmic, and feature rapid-fire snare rolls, ghost notes, and offbeat kicks. The onset density is the highest of any genre considered here, regularly exceeding 12-16 onsets per beat.

**BPM range constraint**: 165-185. A critical implementation detail: many beat trackers will lock to half-tempo (82-92 BPM) because the "boom-bap" skeleton suggests a slower pulse. To avoid this:

```cpp
// Force DnB tempo range: reject candidates below 150 BPM
float correct_dnb_tempo(float estimated_bpm) {
    while (estimated_bpm < 150.0f) estimated_bpm *= 2.0f;
    while (estimated_bpm > 195.0f) estimated_bpm /= 2.0f;
    return estimated_bpm;
}
```

**Beat tracking confidence**: Lower threshold to 0.55 because DnB breakbeats are inherently less periodic than 4/4 kicks. The half-bar (2-beat) period is often more stable than the beat period. Consider tracking at the half-bar level and subdividing.

### 3.2 Complex Breakbeats: High Onset Density

The onset detection function must handle rapid successive transients without suppressing legitimate hits. Reduce the minimum inter-onset interval:

```
min_inter_onset_ms = 40    // allows up to 25 onsets/second (1500/minute)
onset_threshold = 0.8      // lower threshold to catch ghost notes
```

Use a broadband spectral flux ODF rather than a band-limited one -- DnB percussion spans the entire spectrum. Consider running three parallel onset detectors:

1. **Low-band (20-200 Hz)**: kick detection, threshold 1.2
2. **Mid-band (200-4000 Hz)**: snare detection, threshold 1.0
3. **High-band (4000-16000 Hz)**: hi-hat / cymbal, threshold 0.8

The combined onset stream provides a multi-channel trigger map for visual layers.

### 3.3 Reese Bass: Rich Harmonic Sub-Bass

The Reese bass is a hallmark of DnB -- a detuned saw-wave bass creating dense beating and phasing in the 30-200 Hz range. Unlike techno's clean sub-bass, Reese bass has significant harmonic content extending to 1-2 kHz.

Detect Reese bass by measuring:
- **Spectral spread** in the 30-200 Hz band (high spread = detuned harmonics)
- **Amplitude modulation rate** in the sub-bass envelope (beating frequency 1-10 Hz from detuned oscillators)
- **Spectral flux** in the bass band (high flux = phasing movement)

```
reese_score = bass_spectral_spread * bass_am_rate * bass_flux
```

Normalize each factor to [0,1] before multiplication. A reese_score above 0.3 indicates Reese-type bass.

### 3.4 Fast Envelope Followers

DnB requires aggressive envelope follower settings because transients are close together and the visual mapping must keep up:

```
envelope_attack_ms = 1.0    // near-instantaneous attack
envelope_release_ms = 30.0  // fast release to avoid smearing successive hits
```

Compare with techno (attack 5ms, release 80ms) -- the DnB settings are 5x and 2.7x faster respectively. The alpha coefficients for a sample-rate EMA:

```
alpha_attack  = 1 - exp(-1 / (0.001 * sample_rate))   // ~0.95 at 44100
alpha_release = 1 - exp(-1 / (0.030 * sample_rate))    // ~0.00075 at 44100
```

### 3.5 Transient Density as Primary Visual Driver

In DnB, transient density (onsets per second) is more visually informative than individual onset triggers. Compute it as a running count of onsets within a 500 ms sliding window:

```
transient_density = onset_count_in_window / window_duration_seconds
```

Typical values: quiet section 2-4/s, standard beat 8-12/s, drum roll 16-25/s. Map this directly to visual complexity (particle count, fractal iteration depth, polygon subdivision level).

### 3.6 Visual Mapping

| Audio Feature | Visual Parameter | Mapping |
|---|---|---|
| Transient density | Visual complexity / particle count | Exponential, 2^(density/4) |
| Low-band onset | Geometry distortion pulse | Binary, 20ms hold |
| Mid-band onset | Color flash | Binary, 15ms hold |
| Reese bass score | Phasing / interference pattern | Proportional, slow modulation |
| Sub-bass RMS | Scale pulse | Linear, attack 1ms, release 30ms |
| High-band onset rate | Sparkle / glitch density | Linear |

---

## 4. Ambient (No Fixed BPM)

### 4.1 Rhythmic Profile

Ambient music lacks a consistent pulse. Beat tracking is counterproductive here -- it will either report random tempi or lock onto spurious periodicities in pad modulations. The correct strategy is to **disable beat tracking entirely** or set the confidence threshold so high that it effectively never locks:

```
beat_confidence_threshold = 0.95  // almost never met in ambient
fallback_mode = FREERUN           // no tempo-locked animation
```

### 4.2 Spectral Centroid Drift

The spectral centroid (brightness) evolves slowly over many seconds in ambient music. Track it with a long smoothing window:

```
centroid_ema_alpha = 1 - exp(-1 / (2.0 * fps))  // 2-second time constant at given fps
```

The centroid drift rate (`d(centroid)/dt` over 4-second windows) is the primary movement indicator. A derivative exceeding 100 Hz/s suggests a textural transition (e.g., new pad layer entering, filter movement). Map centroid to visual hue and drift rate to animation speed.

### 4.3 Harmonic Content and Pad Detection

Ambient pads are spectrally rich but temporally stable. Detect them by looking for:
- **Low spectral flux** (< 0.05 normalized): minimal frame-to-frame change
- **Low spectral flatness** (< 0.3): tonal content, not noise
- **High spectral spread**: energy distributed across harmonics

When all three conditions are met for more than 2 seconds, flag "pad present" and engage the slow-evolution visual mode.

### 4.4 Slow Envelope Analysis

Standard envelope followers (50-200 ms windows) are too fast for ambient. Use analysis windows of 4-16 seconds:

```
envelope_window_samples = 16.0 * sample_rate  // 705,600 at 44100 Hz
envelope_hop_samples = 0.5 * sample_rate      // update every 500 ms
```

This captures the macro-dynamic contour of the piece: slow crescendos, fade-ins, textural density changes. The computational cost is negligible (one RMS per 500 ms).

### 4.5 Chroma Features and Key Detection

Ambient music often stays in a single key for extended periods, or modulates slowly between related keys. Compute chroma with a 4-second window (2x longer than other genres) and apply median filtering over 16 seconds to eliminate transient chroma fluctuations.

Key detection via the Krumhansl-Schmuckler algorithm: correlate the chroma profile with the 24 major/minor key profiles, select the highest correlation. Map the detected key to a fixed color palette:

```
// Key-to-hue mapping (circle of fifths = color wheel)
float key_to_hue(int key_index) {
    // key_index 0-11 = C,C#,D,...,B major; 12-23 = minor
    float base_hue = (key_index % 12) * 30.0f;  // 30 degrees per semitone
    return fmod(base_hue, 360.0f);
}
```

### 4.6 Visual Mapping

| Audio Feature | Visual Parameter | Mapping |
|---|---|---|
| Spectral centroid | Hue | Linear, 200Hz=blue, 4000Hz=red |
| Centroid drift rate | Animation speed | Logarithmic, 0.1x to 1.0x |
| RMS (16s window) | Global brightness | Linear, slow |
| Spectral flatness | Texture grain | Proportional (flat=grainy, tonal=smooth) |
| Chroma key | Color palette | Fixed palette per key |
| Pad presence flag | Blur / glow intensity | Binary with 4s fade-in/out |
| Harmonic density | Layer count | Step function, 1-4 layers |

---

## 5. Hip Hop (80-100 BPM)

### 5.1 Rhythmic Profile

Hip hop operates at 75-110 BPM with significant swing. The kick-snare pattern follows a "boom-bap" template: kick on beat 1 (and often the "and" of 2 or 3), snare on beats 2 and 4. Unlike techno's metronomic precision, hip hop grooves are intentionally imprecise, with micro-timing offsets of 10-40 ms creating a "lazy" feel.

**Swing detection**: Measure the deviation of onset times from a strict grid. Divide each beat into 16th-note slots. In a swing pattern, even-numbered 16th notes are displaced toward the following odd 16th note. The swing ratio is:

```
swing_ratio = (long_16th_duration) / (short_16th_duration)
```

A ratio of 1.0 = straight; 1.5 = light swing; 2.0 = triplet shuffle (hard swing). Compute by building an inter-onset histogram modulo the beat period, fitting a two-Gaussian model to the 16th-note peaks, and measuring the distance ratio between peaks.

### 5.2 808 Bass Detection

The TR-808 kick drum is ubiquitous in hip hop, especially trap sub-genres. Its defining characteristic is a long sub-bass sustain (200-800 ms) with a sine-wave fundamental at 30-60 Hz and minimal harmonics.

Detect 808 bass by checking:
- **Sub-bass energy** (20-80 Hz) exceeding -20 dBFS
- **Spectral flatness** in 20-80 Hz band below 0.1 (pure tone, not noise)
- **Temporal sustain**: sub-bass energy above -30 dBFS for more than 150 ms after onset

```cpp
struct Bass808Detector {
    float sustain_threshold_db = -30.0f;
    float min_sustain_ms = 150.0f;
    float max_flatness = 0.1f;

    bool is_808(float subbass_rms_db, float subbass_flatness, float sustain_ms) {
        return subbass_rms_db > sustain_threshold_db
            && subbass_flatness < max_flatness
            && sustain_ms > min_sustain_ms;
    }
};
```

### 5.3 Vocal Rhythm as Onset Source

In hip hop, the vocal (rap) rhythm is a major rhythmic element independent of the drum pattern. Detect vocal onsets by running onset detection exclusively on the vocal-frequency band (200-4000 Hz) with the drum transients suppressed. One approach: subtract the low-band (20-200 Hz) and high-band (8-16 kHz) onset signals from the broadband onset signal to isolate mid-band onsets that correlate with vocal phrasing.

The vocal onset density per bar is a useful feature: 8-12 onsets/bar suggests a relaxed flow, 16-24 suggests a rapid-fire delivery, 24+ suggests a double-time or choppy style.

### 5.4 Snare Detection on 2 and 4

Verify snare placement by correlating mid-band (200-4000 Hz) onsets with beat positions 2 and 4 (within a tolerance window of +/- 30 ms). A correlation above 0.7 confirms the classic backbeat pattern. Map snare hits to accent visuals (flash, shape deformation).

### 5.5 Visual Mapping

| Audio Feature | Visual Parameter | Mapping |
|---|---|---|
| Sub-bass RMS (20-80 Hz) | Bass pulse (slow, heavy) | Linear, attack 5ms, release 200ms |
| 808 sustain detection | Sustained glow | Binary, hold for sustain duration |
| Swing ratio | Animation easing curve | 1.0=linear, 2.0=ease-in-out |
| Snare onset (beat 2,4) | Flash / accent | Binary, 40ms hold |
| Vocal onset density | Text/lyric particle density | Linear |
| Kick onset | Geometry push | Binary, 50ms hold |

---

## 6. Rock / Metal (100-180 BPM)

### 6.1 Rhythmic Profile

Rock and metal span a wide tempo range (100-180 BPM for rock, up to 220+ for extreme metal). Unlike electronic genres, tempo is not fixed by a sequencer -- live drummers introduce natural timing variations of 2-5%. The beat tracker must tolerate tempo drift:

```
tempo_drift_tolerance_percent = 5.0
tempo_update_rate_beats = 4    // re-estimate every 4 beats
```

Tempo changes (half-time breakdowns, double-time sections) are common and deliberate. The beat tracker should allow tempo jumps of up to 2x between sections without losing lock.

### 6.2 Guitar Harmonic Detection (Distortion Handling)

Distorted electric guitars generate dense harmonic spectra that challenge standard pitch detection. A clean guitar note at 82 Hz (low E) produces harmonics at 164, 246, 328, ... Hz with decreasing amplitude. A distorted guitar produces the same fundamentals but with harmonic amplitudes that are nearly flat or even increase at higher orders, extending usable harmonics to 5-8 kHz.

**Distortion detection**: Compute the spectral slope (linear regression of log-magnitude vs. log-frequency) in the 100-5000 Hz band:

```
spectral_slope = linear_regression_slope(log(f), log(|X(f)|))
```

Clean signal: slope ~ -3 to -6 dB/octave. Distorted signal: slope ~ 0 to -1 dB/octave (nearly flat). Use spectral slope as a distortion intensity proxy for visual "aggression" mapping.

### 6.3 Crash Cymbal Detection

Crash cymbals produce broadband high-frequency transients (4-16 kHz) with a distinctive long decay (1-3 seconds). Detect by:

1. High-frequency onset (> 4 kHz) with energy exceeding -15 dBFS
2. Slow decay envelope: energy at 500 ms after onset > 30% of peak energy
3. High spectral flatness in the 4-16 kHz band (crashes are noise-like, not tonal)

Crashes typically mark section boundaries (verse-to-chorus transitions, downbeats of new sections). Use them to trigger major visual transitions.

### 6.4 Wide Dynamic Range

Rock and metal have high crest factors (10-18 dB) compared to heavily compressed electronic music (6-10 dB). The visualization system must handle sudden jumps from quiet verses to loud choruses. Use a compressor-style auto-gain:

```cpp
float auto_gain(float rms_db, float target_db, float ratio) {
    float excess = rms_db - target_db;
    if (excess > 0.0f)
        return target_db + excess / ratio;  // compress loud sections
    else
        return rms_db + std::min(-excess * 0.3f, 12.0f);  // expand quiet sections, max 12dB
}
```

### 6.5 Visual Mapping

| Audio Feature | Visual Parameter | Mapping |
|---|---|---|
| Kick onset | Ground shake / distortion | Binary, 30ms hold |
| Snare onset | Flash | Binary, 25ms hold |
| Spectral slope (distortion) | Visual aggression / saturation | Linear, 0=-6dB/oct, 1=0dB/oct |
| Crash cymbal onset | Scene transition trigger | Binary, 500ms cooldown |
| RMS energy | Global brightness | Compressed logarithmic |
| Guitar band energy (80-2000 Hz) | Glow / fire effect intensity | Linear |
| Tempo change detection | Animation tempo reset | Discrete event |

---

## 7. Classical (Variable Tempo)

### 7.1 Rhythmic Profile

Classical music presents the most challenging scenario for beat tracking. Tempo rubato (expressive timing) means the instantaneous tempo varies continuously. A Romantic-era piano piece may range from 60-120 BPM within a single phrase. The beat tracker should operate in a low-confidence advisory mode:

```
beat_confidence_threshold = 0.40  // accept weak estimates
tempo_smoothing_window_beats = 8  // long smoothing
allow_tempo_range = [40, 200]     // very wide
```

For highly rubato passages, switch to an onset-driven mode where each detected note onset triggers a visual event regardless of beat position.

### 7.2 Dynamic Range: pp to ff

Classical music has the widest dynamic range of any genre (up to 60 dB between pianissimo and fortissimo in orchestral works). The auto-gain system must be more aggressive:

```
dynamic_range_expected_db = 50.0
gain_smoothing_time_s = 4.0       // slow adaptation to avoid pumping during crescendos
```

Map dynamic level to visual intensity, but with a logarithmic curve to prevent the quiet passages from being invisible:

```
visual_intensity = pow(linear_amplitude / max_amplitude, 0.3)  // gamma 0.3
```

### 7.3 Harmonic Richness as Primary Feature

In the absence of reliable beat tracking, harmonic content becomes the primary driver. Compute:

- **Chroma vector** (12-dimensional, 2-second window): drives color
- **Harmonic-to-noise ratio** (HNR): high in sustained tones, low in percussive attacks
- **Spectral centroid**: brightness tracks orchestration density (strings+winds+brass = high centroid)
- **Number of simultaneous pitches**: estimate via multi-f0 detection or chroma peak counting (count chroma bins exceeding 50% of max)

```cpp
int estimate_polyphony(const float* chroma, int n_chroma = 12) {
    float max_val = *std::max_element(chroma, chroma + n_chroma);
    float threshold = max_val * 0.5f;
    int count = 0;
    for (int i = 0; i < n_chroma; ++i)
        if (chroma[i] > threshold) ++count;
    return count;  // 1-2 = solo, 3-5 = small ensemble, 6+ = full orchestra/chord
}
```

### 7.4 Section Detection (Movements)

Classical works have distinct sections. Detect section boundaries by monitoring:
- **Sudden silence** (> 500 ms below -50 dBFS): movement boundary
- **Large spectral centroid jump** (> 1000 Hz in < 500 ms): orchestration change
- **Key change**: chroma correlation with previous 30 seconds drops below 0.5
- **Dynamic reset**: RMS drops by more than 20 dB

Trigger a major visual scene change on section boundaries.

### 7.5 Instrument Separation Approximation

True source separation requires deep-learning models, but rough approximation is possible via band energy ratios:

| Instrument Group | Frequency Range | Detection Method |
|---|---|---|
| Double bass / cello | 65-500 Hz | Low-band energy, low spectral flatness |
| Violin / viola | 200-4000 Hz | Mid-band energy, high HNR |
| Woodwinds | 250-3000 Hz | Mid-band, moderate HNR, spectral centroid 800-1500 Hz |
| Brass | 100-5000 Hz | Broad band, high energy, centroid > 1000 Hz |
| Percussion | Broadband | High spectral flatness, sharp onset |

### 7.6 Visual Mapping

| Audio Feature | Visual Parameter | Mapping |
|---|---|---|
| Chroma vector | Color palette (12 colors) | Direct mapping, smoothed |
| Polyphony estimate | Layer/complexity count | Step function |
| Dynamic level (pp-ff) | Brightness / opacity | Gamma 0.3 logarithmic |
| Spectral centroid | Hue warmth | 200Hz=deep blue, 4000Hz=gold |
| Section boundary | Scene transition | Discrete event, 2s crossfade |
| HNR | Smoothness / grain | Proportional |
| Onset (note attack) | Ripple / pulse | Per-event, 100ms decay |

---

## 8. Pop (100-130 BPM)

### 8.1 Rhythmic Profile

Pop music is typically 100-130 BPM with a clear 4/4 beat, moderate swing, and high production density. Beat tracking confidence should be high (> 0.75). The defining structural feature is the verse-chorus dynamic contrast.

**BPM range**: 95-135.

### 8.2 Vocal Prominence Detection

Pop is vocal-centric. Compute vocal prominence as the ratio of mid-band energy (300-3000 Hz) to total energy, combined with the spectral flatness check described in the House section:

```
vocal_prominence = (energy_300_3000 / energy_total) * (1.0 - flatness_300_3000)
```

Values above 0.4 indicate a vocal-dominant section (verses, choruses with lead vocal). Values below 0.2 indicate an instrumental section (intro, bridge, solo).

### 8.3 Verse/Chorus Energy Difference

Pop songs have a characteristic energy profile: verses at -18 to -12 dBFS RMS, choruses at -10 to -6 dBFS RMS, with a 3-8 dB jump at the chorus. Detect the verse-chorus transition by monitoring RMS over 4-bar windows (approximately 8 seconds at 120 BPM):

```
section_energy_ratio = rms_current_4bars / rms_previous_4bars
chorus_detected = (section_energy_ratio > 1.5)  // ~3.5 dB increase
```

### 8.4 Drop/Chorus Impact Detection

The "drop" (common in modern pop influenced by EDM) is a sudden onset of full-spectrum energy after a buildup or breakdown. Detect by:

1. Monitoring the spectral centroid variance over 2-second windows
2. Detecting a low-energy period (buildup) followed by a sudden energy increase (> 6 dB in < 200 ms)
3. Sub-bass returning after absence (bass drop)

```cpp
struct DropDetector {
    float energy_history[128];  // 2 seconds at ~64 fps
    int write_pos = 0;
    float buildup_threshold_db = -15.0f;
    float drop_jump_db = 6.0f;

    bool detect_drop(float current_rms_db) {
        energy_history[write_pos++ % 128] = current_rms_db;

        // Check if previous 1 second was quiet and current is loud
        float prev_avg = mean(&energy_history[(write_pos - 64) % 128], 32);
        float recent_avg = mean(&energy_history[(write_pos - 8) % 128], 8);
        return (prev_avg < buildup_threshold_db)
            && (recent_avg - prev_avg > drop_jump_db);
    }
};
```

### 8.5 Visual Mapping

| Audio Feature | Visual Parameter | Mapping |
|---|---|---|
| Kick onset | Pulse | Binary, 30ms hold |
| Vocal prominence | Spotlight / foreground focus | Proportional, smooth |
| Chorus detection | Energy boost / palette shift | Binary, 4s crossfade |
| Drop detection | Major visual impact event | Binary, 1s cooldown |
| Spectral centroid | Hue | Linear |
| Hi-hat pattern | Background texture speed | Linear |
| Bass RMS | Scale / push | Linear, attack 3ms, release 100ms |

---

## 9. Consolidated Parameter Tables

### 9.1 FFT and Analysis Parameters

| Parameter | Techno | House | DnB | Ambient | Hip Hop | Rock | Classical | Pop |
|---|---|---|---|---|---|---|---|---|
| FFT size | 2048 | 2048 | 2048 | 4096 | 2048 | 2048 | 4096 | 2048 |
| Hop size | 512 | 512 | 256 | 1024 | 512 | 512 | 1024 | 512 |
| Window type | Hann | Hann | Hann | Hann | Hann | Blackman-Harris | Hann | Hann |
| Sample rate | 44100 | 44100 | 44100 | 44100 | 44100 | 44100 | 44100 | 44100 |
| Frame rate (Hz) | 86.1 | 86.1 | 172.3 | 43.1 | 86.1 | 86.1 | 43.1 | 86.1 |
| Freq resolution (Hz) | 21.5 | 21.5 | 21.5 | 10.8 | 21.5 | 21.5 | 10.8 | 21.5 |
| Time resolution (ms) | 11.6 | 11.6 | 5.8 | 23.2 | 11.6 | 11.6 | 23.2 | 11.6 |

### 9.2 BPM and Beat Tracking Parameters

| Parameter | Techno | House | DnB | Ambient | Hip Hop | Rock | Classical | Pop |
|---|---|---|---|---|---|---|---|---|
| BPM min | 120 | 118 | 165 | N/A | 75 | 100 | 40 | 95 |
| BPM max | 140 | 132 | 185 | N/A | 110 | 220 | 200 | 135 |
| Beat confidence threshold | 0.85 | 0.80 | 0.55 | 0.95 | 0.65 | 0.60 | 0.40 | 0.75 |
| Tempo drift tolerance (%) | 1.0 | 1.5 | 2.0 | N/A | 3.0 | 5.0 | 15.0 | 2.0 |
| Meter | 4/4 lock | 4/4 lock | 4/4 half-bar | Free | 4/4 | Variable | Variable | 4/4 lock |
| Half-tempo rejection | No | No | Yes | N/A | No | No | No | No |

### 9.3 Onset Detection Parameters

| Parameter | Techno | House | DnB | Ambient | Hip Hop | Rock | Classical | Pop |
|---|---|---|---|---|---|---|---|---|
| Method | Band-limited SF | Band-limited SF | Broadband SF | Disabled | Band-limited SF | Broadband SF | Broadband HFC | Broadband SF |
| Threshold (median mult.) | 1.5 | 1.3 | 0.8 | N/A | 1.2 | 1.0 | 0.6 | 1.2 |
| Min inter-onset (ms) | 200 | 200 | 40 | N/A | 100 | 60 | 80 | 150 |
| Silence threshold (dBFS) | -60 | -60 | -55 | N/A | -55 | -50 | -65 | -55 |
| Band focus (Hz) | 0-200 | 0-200 | Broadband | N/A | 20-200 | Broadband | Broadband | 0-200 |

*SF = Spectral Flux, HFC = High Frequency Content*

### 9.4 Band Energy Weighting

Weights for each frequency band controlling visual emphasis. All weights sum to 1.0 within each genre.

| Band | Techno | House | DnB | Ambient | Hip Hop | Rock | Classical | Pop |
|---|---|---|---|---|---|---|---|---|
| Sub-bass (20-60 Hz) | 0.30 | 0.20 | 0.25 | 0.10 | 0.35 | 0.10 | 0.05 | 0.15 |
| Bass (60-250 Hz) | 0.25 | 0.20 | 0.20 | 0.10 | 0.25 | 0.20 | 0.10 | 0.20 |
| Low-mid (250-1000 Hz) | 0.15 | 0.20 | 0.15 | 0.20 | 0.15 | 0.25 | 0.25 | 0.20 |
| Mid (1-4 kHz) | 0.15 | 0.25 | 0.15 | 0.25 | 0.15 | 0.20 | 0.30 | 0.25 |
| High-mid (4-8 kHz) | 0.10 | 0.10 | 0.15 | 0.20 | 0.05 | 0.15 | 0.20 | 0.15 |
| High (8-16 kHz) | 0.05 | 0.05 | 0.10 | 0.15 | 0.05 | 0.10 | 0.10 | 0.05 |

### 9.5 Envelope Follower Parameters

| Parameter | Techno | House | DnB | Ambient | Hip Hop | Rock | Classical | Pop |
|---|---|---|---|---|---|---|---|---|
| Attack (ms) | 5.0 | 5.0 | 1.0 | 50.0 | 5.0 | 3.0 | 10.0 | 5.0 |
| Release (ms) | 80.0 | 120.0 | 30.0 | 4000.0 | 200.0 | 60.0 | 500.0 | 100.0 |
| Smoothing window (s) | 0.5 | 0.5 | 0.25 | 16.0 | 1.0 | 0.5 | 4.0 | 0.5 |

### 9.6 Feature Normalization Bounds

Expected ranges for normalizing features to [0, 1] for visual mapping. Derived from analysis of genre-representative test material.

| Feature | Techno | House | DnB | Ambient | Hip Hop | Rock | Classical | Pop |
|---|---|---|---|---|---|---|---|---|
| RMS (dBFS) range | [-30, -6] | [-30, -6] | [-25, -6] | [-50, -20] | [-30, -8] | [-35, -6] | [-55, -10] | [-25, -6] |
| Spectral centroid (Hz) | [200, 6000] | [300, 7000] | [200, 8000] | [100, 4000] | [200, 5000] | [300, 8000] | [200, 6000] | [300, 7000] |
| Onset rate (onsets/s) | [1, 8] | [1, 8] | [4, 25] | [0, 2] | [2, 12] | [2, 16] | [0.5, 10] | [1, 8] |
| Bass ratio (sub/total) | [0.2, 0.8] | [0.1, 0.6] | [0.15, 0.7] | [0.0, 0.3] | [0.3, 0.9] | [0.05, 0.4] | [0.0, 0.2] | [0.1, 0.5] |

### 9.7 Visual Mapping Emphasis Summary

| Genre | Primary Driver | Secondary Driver | Tertiary Driver |
|---|---|---|---|
| Techno | Sub-bass pulse | Spectral centroid sweep | Hi-hat density |
| House | Mid-range energy | Vocal presence | Chroma (harmony) |
| DnB | Transient density | Reese bass phase | Multi-band onset triggers |
| Ambient | Spectral centroid drift | Chroma/key | Dynamic level |
| Hip Hop | 808 bass sustain | Swing/groove | Vocal rhythm |
| Rock | Spectral slope (distortion) | Dynamic range | Crash triggers |
| Classical | Harmonic richness (chroma) | Dynamic level (pp-ff) | Section boundaries |
| Pop | Vocal prominence | Verse/chorus energy | Drop detection |

---

## 10. Real-Time Genre Detection

### 10.1 Feature Vector Definition

A genre classifier must operate on features that are computationally cheap and genre-discriminative. Extract the following 8-dimensional feature vector over a 4-second analysis window, updated every 500 ms:

| Index | Feature | Computation | Unit |
|---|---|---|---|
| 0 | Spectral centroid mean | Mean of per-frame centroids over 4s | Hz |
| 1 | Spectral centroid std | Standard deviation over 4s | Hz |
| 2 | Onset rate | Onset count / window duration | onsets/s |
| 3 | Bass ratio | Energy(20-200Hz) / Energy(20-16kHz) | ratio |
| 4 | Estimated BPM | From autocorrelation peak | BPM |
| 5 | Spectral flatness mean | Geometric/arithmetic mean ratio | ratio |
| 6 | Dynamic range | max(RMS) - min(RMS) over 4s | dB |
| 7 | Beat confidence | Autocorrelation peak height | ratio |

### 10.2 Genre Feature Profiles

Typical feature values for each genre (median from training data):

```
Genre          Centroid  Cent.Std  OnsetRate  BassRatio  BPM    Flatness  DynRange  BeatConf
Techno         1800      400       4.5        0.55       128    0.15      8         0.92
House          2200      600       4.0        0.40       124    0.20      10        0.88
DnB            2500      800       12.0       0.45       174    0.25      12        0.65
Ambient        1200      200       0.5        0.15       0      0.35      6         0.15
Hip Hop        1500      500       3.0        0.65       90     0.18      14        0.70
Rock           3000      900       6.0        0.20       130    0.30      20        0.60
Classical      2000      700       2.0        0.10       0      0.12      35        0.30
Pop            2500      500       4.0        0.30       115    0.22      12        0.80
```

### 10.3 k-NN Classifier

A k-NN classifier with k=5 and Euclidean distance on z-score-normalized features provides baseline genre classification. Each feature must be normalized to zero mean and unit variance using precomputed statistics from the training set.

```cpp
#include <vector>
#include <array>
#include <algorithm>
#include <cmath>
#include <string>
#include <numeric>

enum class Genre : int {
    TECHNO = 0, HOUSE, DNB, AMBIENT, HIPHOP, ROCK, CLASSICAL, POP,
    NUM_GENRES
};

static const char* genre_names[] = {
    "Techno", "House", "DnB", "Ambient", "Hip Hop", "Rock", "Classical", "Pop"
};

static constexpr int NUM_FEATURES = 8;

struct GenreClassifier {
    struct TrainingPoint {
        std::array<float, NUM_FEATURES> features;
        Genre label;
    };

    // Precomputed normalization: mean and std for each feature
    std::array<float, NUM_FEATURES> feature_mean;
    std::array<float, NUM_FEATURES> feature_std;

    // Training data (genre centroids from profiles above, plus variations)
    std::vector<TrainingPoint> training_data;

    int k = 5;

    void normalize(std::array<float, NUM_FEATURES>& f) const {
        for (int i = 0; i < NUM_FEATURES; ++i) {
            f[i] = (f[i] - feature_mean[i]) / (feature_std[i] + 1e-6f);
        }
    }

    float distance(const std::array<float, NUM_FEATURES>& a,
                   const std::array<float, NUM_FEATURES>& b) const {
        float sum = 0.0f;
        for (int i = 0; i < NUM_FEATURES; ++i) {
            float d = a[i] - b[i];
            sum += d * d;
        }
        return sum;  // squared Euclidean; sqrt unnecessary for comparison
    }

    Genre classify(std::array<float, NUM_FEATURES> query) const {
        normalize(const_cast<std::array<float, NUM_FEATURES>&>(query));

        // Compute distances to all training points
        struct DistLabel {
            float dist;
            Genre label;
        };

        std::vector<DistLabel> distances(training_data.size());
        for (size_t i = 0; i < training_data.size(); ++i) {
            distances[i] = {distance(query, training_data[i].features),
                            training_data[i].label};
        }

        // Partial sort to find k nearest
        std::partial_sort(distances.begin(), distances.begin() + k,
                          distances.end(),
                          [](const DistLabel& a, const DistLabel& b) {
                              return a.dist < b.dist;
                          });

        // Vote
        std::array<int, (int)Genre::NUM_GENRES> votes{};
        for (int i = 0; i < k; ++i) {
            votes[(int)distances[i].label]++;
        }

        // Return genre with most votes (tie-break: nearest neighbor wins)
        Genre best = distances[0].label;
        int best_votes = 0;
        for (int g = 0; g < (int)Genre::NUM_GENRES; ++g) {
            if (votes[g] > best_votes) {
                best_votes = votes[g];
                best = (Genre)g;
            }
        }
        return best;
    }
};
```

### 10.4 Decision Tree Alternative

A decision tree is faster (O(log N) vs O(N*k) for k-NN) and more interpretable. Hand-crafted decision tree based on the genre profiles:

```cpp
Genre classify_decision_tree(const std::array<float, NUM_FEATURES>& f) {
    float centroid     = f[0];
    float centroid_std = f[1];
    float onset_rate   = f[2];
    float bass_ratio   = f[3];
    float bpm          = f[4];
    float flatness     = f[5];
    float dyn_range    = f[6];
    float beat_conf    = f[7];

    // Level 1: Beat confidence separates ambient & classical from rhythmic genres
    if (beat_conf < 0.35f) {
        // No clear beat
        if (onset_rate < 1.0f && dyn_range < 10.0f)
            return Genre::AMBIENT;
        else
            return Genre::CLASSICAL;
    }

    // Level 2: BPM separates DnB from everything else
    if (bpm > 155.0f && onset_rate > 8.0f)
        return Genre::DNB;

    // Level 3: Slow tempo with high bass = hip hop
    if (bpm < 115.0f && bass_ratio > 0.45f)
        return Genre::HIPHOP;

    // Level 4: High dynamic range + high centroid = rock
    if (dyn_range > 16.0f && centroid > 2500.0f)
        return Genre::ROCK;

    // Level 5: Distinguish techno, house, pop
    if (bass_ratio > 0.40f && centroid < 2000.0f && beat_conf > 0.85f)
        return Genre::TECHNO;

    if (centroid > 2200.0f && flatness > 0.20f)
        return Genre::POP;

    return Genre::HOUSE;  // default for mid-tempo, mid-bass, rhythmic
}
```

### 10.5 Training Data Requirements

For a k-NN classifier: minimum 50 labeled 4-second segments per genre (400 total). For a trained decision tree (e.g., C4.5 or CART): minimum 200 segments per genre (1600 total). Sources:

- **GTZAN dataset**: 1000 clips x 30 seconds, 10 genres (not exactly our 8, but overlapping). Extract multiple 4-second windows per clip.
- **FMA (Free Music Archive)**: 106,574 tracks, 161 genres. Filter to our 8 target genres.
- **Custom collection**: 20 representative tracks per genre, extract 10 windows each = 1600 segments.

Feature extraction pipeline for training:

```
Audio file → Resample to 44100 → Segment into 4s windows (2s hop)
    → For each window:
        STFT (2048/512) → Spectral centroid, flatness, onset detection
        Onset stream → Onset rate
        Band energy → Bass ratio
        Autocorrelation → BPM, beat confidence
        RMS series → Dynamic range
    → Output: 8-dim feature vector + genre label
```

### 10.6 Transition Smoothing Between Genres

Raw classifier output will oscillate between genres, especially at genre boundaries (e.g., techno vs. house). Apply two layers of smoothing:

**Layer 1: Temporal voting**. Maintain a circular buffer of the last 8 classification results (covering 4 seconds of analysis). The reported genre is the mode of the buffer.

**Layer 2: Hysteresis**. Only change the active genre if the new genre has been the mode for at least N consecutive voting cycles:

```cpp
struct GenreSmoothing {
    static constexpr int HISTORY_SIZE = 8;
    static constexpr int SWITCH_THRESHOLD = 6;  // 6 out of 8 must agree

    Genre history[HISTORY_SIZE] = {};
    int write_pos = 0;
    Genre current_genre = Genre::TECHNO;

    Genre update(Genre detected) {
        history[write_pos++ % HISTORY_SIZE] = detected;

        // Count votes
        std::array<int, (int)Genre::NUM_GENRES> votes{};
        for (int i = 0; i < HISTORY_SIZE; ++i)
            votes[(int)history[i]]++;

        // Find mode
        Genre mode = current_genre;
        int max_votes = 0;
        for (int g = 0; g < (int)Genre::NUM_GENRES; ++g) {
            if (votes[g] > max_votes) {
                max_votes = votes[g];
                mode = (Genre)g;
            }
        }

        // Only switch if mode has strong majority and is different from current
        if (mode != current_genre && max_votes >= SWITCH_THRESHOLD)
            current_genre = mode;

        return current_genre;
    }
};
```

This produces a stable genre output that changes only after ~3 seconds of consistent detection, preventing visual jarring from rapid preset switches.

---

## 11. Auto-Switch System

### 11.1 Architecture Overview

```
                         ┌─────────────────────────────┐
                         │     Audio Input (real-time)  │
                         └──────────────┬──────────────┘
                                        │
                    ┌───────────────────┴───────────────────┐
                    │         Feature Extractor              │
                    │  (spectral, rhythmic, amplitude)       │
                    └───────┬───────────────────┬───────────┘
                            │                   │
                  ┌─────────▼─────────┐  ┌──────▼──────────┐
                  │  Genre Classifier  │  │  Visual Engine   │
                  │  (4s window, 0.5s  │  │  (per-frame      │
                  │   update rate)     │  │   feature read)  │
                  └─────────┬─────────┘  └──────▲──────────┘
                            │                   │
                  ┌─────────▼─────────┐         │
                  │  Genre Smoother    │         │
                  │  (hysteresis +     │         │
                  │   voting)          │         │
                  └─────────┬─────────┘         │
                            │                   │
                  ┌─────────▼─────────┐         │
                  │  Preset Manager    ├─────────┘
                  │  (interpolates     │
                  │   between presets) │
                  └─────────┬─────────┘
                            │
                  ┌─────────▼─────────┐
                  │  Manual Override   │
                  │  (UI / MIDI CC)    │
                  └───────────────────┘
```

### 11.2 Preset Manager Implementation

The preset manager holds the current active preset and a target preset. When the genre smoother outputs a genre change, the manager begins a crossfade from the current preset parameters to the target preset parameters over a configurable transition duration.

```cpp
#include <array>
#include <chrono>

struct GenrePreset {
    // FFT parameters
    int fft_size;
    int hop_size;

    // BPM tracking
    float bpm_min, bpm_max;
    float beat_confidence_threshold;
    float tempo_drift_tolerance;

    // Onset detection
    float onset_threshold;
    float min_inter_onset_ms;
    int onset_band_low_hz, onset_band_high_hz;

    // Envelope
    float envelope_attack_ms;
    float envelope_release_ms;

    // Band weights (6 bands)
    std::array<float, 6> band_weights;

    // Normalization bounds
    float rms_min_db, rms_max_db;
    float centroid_min_hz, centroid_max_hz;
    float onset_rate_min, onset_rate_max;

    // Visual emphasis
    int primary_feature_index;    // index into feature array
    int secondary_feature_index;
};

// Presets stored as a static array indexed by Genre enum
extern const GenrePreset GENRE_PRESETS[(int)Genre::NUM_GENRES];

class PresetManager {
public:
    void set_transition_time(float seconds) {
        transition_duration_s_ = seconds;
    }

    void request_genre_change(Genre new_genre) {
        if (new_genre == target_genre_) return;

        source_preset_ = current_interpolated_;
        target_genre_ = new_genre;
        target_preset_ = GENRE_PRESETS[(int)new_genre];
        transition_start_ = std::chrono::steady_clock::now();
        transitioning_ = true;
    }

    void force_genre(Genre genre) {
        // Manual override: instant switch, no transition
        target_genre_ = genre;
        current_interpolated_ = GENRE_PRESETS[(int)genre];
        transitioning_ = false;
        manual_override_ = true;
    }

    void release_override() {
        manual_override_ = false;
    }

    bool is_manual_override() const { return manual_override_; }

    const GenrePreset& get_current_preset() {
        if (transitioning_) {
            auto now = std::chrono::steady_clock::now();
            float elapsed = std::chrono::duration<float>(
                now - transition_start_).count();
            float t = std::min(elapsed / transition_duration_s_, 1.0f);

            // Smooth step interpolation (ease in/out)
            t = t * t * (3.0f - 2.0f * t);

            interpolate_presets(source_preset_, target_preset_, t,
                                current_interpolated_);

            if (t >= 1.0f) transitioning_ = false;
        }
        return current_interpolated_;
    }

    Genre get_active_genre() const { return target_genre_; }

private:
    GenrePreset source_preset_{};
    GenrePreset target_preset_{};
    GenrePreset current_interpolated_{};
    Genre target_genre_ = Genre::TECHNO;

    float transition_duration_s_ = 3.0f;  // 3-second crossfade default
    std::chrono::steady_clock::time_point transition_start_;
    bool transitioning_ = false;
    bool manual_override_ = false;

    static void interpolate_presets(const GenrePreset& a, const GenrePreset& b,
                                    float t, GenrePreset& out) {
        // Discrete parameters: snap at t=0.5
        out.fft_size = (t < 0.5f) ? a.fft_size : b.fft_size;
        out.hop_size = (t < 0.5f) ? a.hop_size : b.hop_size;
        out.onset_band_low_hz = (t < 0.5f) ? a.onset_band_low_hz : b.onset_band_low_hz;
        out.onset_band_high_hz = (t < 0.5f) ? a.onset_band_high_hz : b.onset_band_high_hz;
        out.primary_feature_index = (t < 0.5f) ? a.primary_feature_index
                                                : b.primary_feature_index;
        out.secondary_feature_index = (t < 0.5f) ? a.secondary_feature_index
                                                  : b.secondary_feature_index;

        // Continuous parameters: linear interpolation
        auto lerp = [](float a, float b, float t) { return a + (b - a) * t; };

        out.bpm_min = lerp(a.bpm_min, b.bpm_min, t);
        out.bpm_max = lerp(a.bpm_max, b.bpm_max, t);
        out.beat_confidence_threshold = lerp(a.beat_confidence_threshold,
                                             b.beat_confidence_threshold, t);
        out.tempo_drift_tolerance = lerp(a.tempo_drift_tolerance,
                                         b.tempo_drift_tolerance, t);
        out.onset_threshold = lerp(a.onset_threshold, b.onset_threshold, t);
        out.min_inter_onset_ms = lerp(a.min_inter_onset_ms,
                                      b.min_inter_onset_ms, t);
        out.envelope_attack_ms = lerp(a.envelope_attack_ms,
                                      b.envelope_attack_ms, t);
        out.envelope_release_ms = lerp(a.envelope_release_ms,
                                       b.envelope_release_ms, t);
        out.rms_min_db = lerp(a.rms_min_db, b.rms_min_db, t);
        out.rms_max_db = lerp(a.rms_max_db, b.rms_max_db, t);
        out.centroid_min_hz = lerp(a.centroid_min_hz, b.centroid_min_hz, t);
        out.centroid_max_hz = lerp(a.centroid_max_hz, b.centroid_max_hz, t);
        out.onset_rate_min = lerp(a.onset_rate_min, b.onset_rate_min, t);
        out.onset_rate_max = lerp(a.onset_rate_max, b.onset_rate_max, t);

        for (int i = 0; i < 6; ++i)
            out.band_weights[i] = lerp(a.band_weights[i],
                                       b.band_weights[i], t);
    }
};
```

### 11.3 Hysteresis and Stability

The hysteresis system prevents rapid genre oscillation. Three levels of protection:

1. **Voting buffer** (Section 10.6): 8-slot circular buffer, mode with 6/8 majority required to propose a change. Effective time constant: ~3 seconds.

2. **Cooldown timer**: After a genre switch, suppress further switches for a minimum cooldown period:

```cpp
float genre_switch_cooldown_s = 10.0f;  // minimum 10s between switches
```

3. **Confidence gating**: Only consider a genre switch if the classifier confidence (inverse of distance to nearest training point) exceeds a threshold:

```cpp
float min_classification_confidence = 0.6f;
```

Combined, these three mechanisms ensure that genre switches occur only when there is strong, sustained evidence of a new genre. In practice, this means genre changes happen at natural mix transitions (DJ set) or track boundaries (playlist), not during brief breakdowns or intros.

### 11.4 Manual Override

The preset manager supports a manual override via the `force_genre()` method. In a live VJ scenario, the operator may know the genre better than the classifier (e.g., the DJ announced the next track). Manual override:

- Takes effect immediately (no transition delay from hysteresis)
- Optionally uses the same smooth crossfade as automatic switches, or an instant snap
- Remains active until `release_override()` is called, at which point the classifier resumes control
- Can be triggered by MIDI CC messages (e.g., CC 80 values 0-7 map to Genre enum, CC 81 = release override)

```cpp
void handle_midi_cc(int cc_number, int value) {
    if (cc_number == 80 && value < (int)Genre::NUM_GENRES) {
        preset_manager.force_genre((Genre)value);
    } else if (cc_number == 81 && value > 63) {
        preset_manager.release_override();
    }
}
```

### 11.5 Parameter Hot-Swap Considerations

Some parameters cannot be changed seamlessly at runtime:

| Parameter | Hot-Swappable | Notes |
|---|---|---|
| FFT size | No | Requires re-allocating FFT buffers. Crossfade at buffer boundary. |
| Hop size | No | Changes analysis frame rate. Switch at next buffer boundary. |
| BPM range | Yes | Beat tracker adapts within 2-4 beats. |
| Onset threshold | Yes | Immediate effect. |
| Envelope attack/release | Yes | EMA coefficients update instantly. |
| Band weights | Yes | Smooth interpolation. |
| Normalization bounds | Yes | May cause brief visual jump; interpolate over 1s. |

For non-hot-swappable parameters (FFT size, hop size), maintain two parallel analysis chains during transitions. Run both the old and new FFT configurations simultaneously for the duration of the crossfade, then release the old chain. This doubles the CPU cost during transitions but avoids audible/visible glitches:

```
Transition timeline (3 seconds):
t=0.0s  Genre change detected. Allocate new FFT chain.
t=0.0s  Begin running both chains in parallel.
t=0.0s  Visual output reads from old chain (weight 1.0 old, 0.0 new).
t=1.5s  Midpoint: weight 0.5 old, 0.5 new.
t=3.0s  Full transition: weight 0.0 old, 1.0 new.
t=3.0s  Deallocate old FFT chain.
```

### 11.6 Genre Comparison Chart

A high-level comparison of genre characteristics that drive preset differences:

```
                  Bass      Onset     Beat       Dynamic   Spectral
Genre            Energy    Density   Confidence  Range     Complexity
─────────────────────────────────────────────────────────────────────
Techno           █████     ██        █████       ██        ██
House            ████      ██        ████        ███       ███
DnB              ████      █████     ███         ███       ███
Ambient          ██        █         █           ██        ████
Hip Hop          █████     ██        ███         ████      ██
Rock             ███       ███       ███         █████     ████
Classical        ██        ██        ██          █████     █████
Pop              ███       ██        ████        ███       ███

█ = 1 (low), █████ = 5 (high)
```

### 11.7 Integration with Calibration System

The genre detection system feeds into the calibration pipeline (see [IMPL_calibration_adaptation.md](IMPL_calibration_adaptation.md)). When a genre change occurs:

1. The normalization bounds update to the new genre's expected ranges
2. The adaptive calibration system resets its running statistics over a 4-second window
3. The visual mapping emphasis shifts to the new genre's primary/secondary drivers
4. The envelope follower coefficients re-compute from the new attack/release times

The calibration system should treat the first 4 seconds after a genre switch as a "learning period" where it builds new baseline statistics. During this period, use the preset's default normalization bounds rather than adaptive bounds.

---

## Appendix A: Complete Preset Initialization Code

```cpp
const GenrePreset GENRE_PRESETS[(int)Genre::NUM_GENRES] = {
    // TECHNO
    {
        .fft_size = 2048, .hop_size = 512,
        .bpm_min = 120, .bpm_max = 140,
        .beat_confidence_threshold = 0.85f, .tempo_drift_tolerance = 0.01f,
        .onset_threshold = 1.5f, .min_inter_onset_ms = 200.0f,
        .onset_band_low_hz = 0, .onset_band_high_hz = 200,
        .envelope_attack_ms = 5.0f, .envelope_release_ms = 80.0f,
        .band_weights = {0.30f, 0.25f, 0.15f, 0.15f, 0.10f, 0.05f},
        .rms_min_db = -30, .rms_max_db = -6,
        .centroid_min_hz = 200, .centroid_max_hz = 6000,
        .onset_rate_min = 1, .onset_rate_max = 8,
        .primary_feature_index = 0,   // sub-bass RMS
        .secondary_feature_index = 3  // spectral centroid
    },
    // HOUSE
    {
        .fft_size = 2048, .hop_size = 512,
        .bpm_min = 118, .bpm_max = 132,
        .beat_confidence_threshold = 0.80f, .tempo_drift_tolerance = 0.015f,
        .onset_threshold = 1.3f, .min_inter_onset_ms = 200.0f,
        .onset_band_low_hz = 0, .onset_band_high_hz = 200,
        .envelope_attack_ms = 5.0f, .envelope_release_ms = 120.0f,
        .band_weights = {0.20f, 0.20f, 0.20f, 0.25f, 0.10f, 0.05f},
        .rms_min_db = -30, .rms_max_db = -6,
        .centroid_min_hz = 300, .centroid_max_hz = 7000,
        .onset_rate_min = 1, .onset_rate_max = 8,
        .primary_feature_index = 3,   // mid-range energy
        .secondary_feature_index = 5  // vocal presence
    },
    // DNB
    {
        .fft_size = 2048, .hop_size = 256,
        .bpm_min = 165, .bpm_max = 185,
        .beat_confidence_threshold = 0.55f, .tempo_drift_tolerance = 0.02f,
        .onset_threshold = 0.8f, .min_inter_onset_ms = 40.0f,
        .onset_band_low_hz = 0, .onset_band_high_hz = 16000,
        .envelope_attack_ms = 1.0f, .envelope_release_ms = 30.0f,
        .band_weights = {0.25f, 0.20f, 0.15f, 0.15f, 0.15f, 0.10f},
        .rms_min_db = -25, .rms_max_db = -6,
        .centroid_min_hz = 200, .centroid_max_hz = 8000,
        .onset_rate_min = 4, .onset_rate_max = 25,
        .primary_feature_index = 6,   // transient density
        .secondary_feature_index = 0  // sub-bass (Reese)
    },
    // AMBIENT
    {
        .fft_size = 4096, .hop_size = 1024,
        .bpm_min = 0, .bpm_max = 0,  // disabled
        .beat_confidence_threshold = 0.95f, .tempo_drift_tolerance = 0.0f,
        .onset_threshold = 2.0f, .min_inter_onset_ms = 500.0f,
        .onset_band_low_hz = 0, .onset_band_high_hz = 0,  // disabled
        .envelope_attack_ms = 50.0f, .envelope_release_ms = 4000.0f,
        .band_weights = {0.10f, 0.10f, 0.20f, 0.25f, 0.20f, 0.15f},
        .rms_min_db = -50, .rms_max_db = -20,
        .centroid_min_hz = 100, .centroid_max_hz = 4000,
        .onset_rate_min = 0, .onset_rate_max = 2,
        .primary_feature_index = 3,   // spectral centroid
        .secondary_feature_index = 4  // chroma/key
    },
    // HIPHOP
    {
        .fft_size = 2048, .hop_size = 512,
        .bpm_min = 75, .bpm_max = 110,
        .beat_confidence_threshold = 0.65f, .tempo_drift_tolerance = 0.03f,
        .onset_threshold = 1.2f, .min_inter_onset_ms = 100.0f,
        .onset_band_low_hz = 20, .onset_band_high_hz = 200,
        .envelope_attack_ms = 5.0f, .envelope_release_ms = 200.0f,
        .band_weights = {0.35f, 0.25f, 0.15f, 0.15f, 0.05f, 0.05f},
        .rms_min_db = -30, .rms_max_db = -8,
        .centroid_min_hz = 200, .centroid_max_hz = 5000,
        .onset_rate_min = 2, .onset_rate_max = 12,
        .primary_feature_index = 0,   // 808 bass
        .secondary_feature_index = 7  // swing/groove
    },
    // ROCK
    {
        .fft_size = 2048, .hop_size = 512,
        .bpm_min = 100, .bpm_max = 220,
        .beat_confidence_threshold = 0.60f, .tempo_drift_tolerance = 0.05f,
        .onset_threshold = 1.0f, .min_inter_onset_ms = 60.0f,
        .onset_band_low_hz = 0, .onset_band_high_hz = 16000,
        .envelope_attack_ms = 3.0f, .envelope_release_ms = 60.0f,
        .band_weights = {0.10f, 0.20f, 0.25f, 0.20f, 0.15f, 0.10f},
        .rms_min_db = -35, .rms_max_db = -6,
        .centroid_min_hz = 300, .centroid_max_hz = 8000,
        .onset_rate_min = 2, .onset_rate_max = 16,
        .primary_feature_index = 2,   // spectral slope (distortion)
        .secondary_feature_index = 1  // dynamic range
    },
    // CLASSICAL
    {
        .fft_size = 4096, .hop_size = 1024,
        .bpm_min = 40, .bpm_max = 200,
        .beat_confidence_threshold = 0.40f, .tempo_drift_tolerance = 0.15f,
        .onset_threshold = 0.6f, .min_inter_onset_ms = 80.0f,
        .onset_band_low_hz = 0, .onset_band_high_hz = 16000,
        .envelope_attack_ms = 10.0f, .envelope_release_ms = 500.0f,
        .band_weights = {0.05f, 0.10f, 0.25f, 0.30f, 0.20f, 0.10f},
        .rms_min_db = -55, .rms_max_db = -10,
        .centroid_min_hz = 200, .centroid_max_hz = 6000,
        .onset_rate_min = 0.5f, .onset_rate_max = 10,
        .primary_feature_index = 4,   // chroma/harmonic richness
        .secondary_feature_index = 1  // dynamic level
    },
    // POP
    {
        .fft_size = 2048, .hop_size = 512,
        .bpm_min = 95, .bpm_max = 135,
        .beat_confidence_threshold = 0.75f, .tempo_drift_tolerance = 0.02f,
        .onset_threshold = 1.2f, .min_inter_onset_ms = 150.0f,
        .onset_band_low_hz = 0, .onset_band_high_hz = 200,
        .envelope_attack_ms = 5.0f, .envelope_release_ms = 100.0f,
        .band_weights = {0.15f, 0.20f, 0.20f, 0.25f, 0.15f, 0.05f},
        .rms_min_db = -25, .rms_max_db = -6,
        .centroid_min_hz = 300, .centroid_max_hz = 7000,
        .onset_rate_min = 1, .onset_rate_max = 8,
        .primary_feature_index = 5,   // vocal prominence
        .secondary_feature_index = 1  // verse/chorus energy
    }
};
```

---

## Appendix B: Genre Detection Integration Pseudocode

End-to-end integration of the genre detection and auto-switch system within the main audio processing loop:

```cpp
// Main audio callback (called every buffer, e.g., 512 samples at 44100 Hz)
void audio_callback(const float* input, int num_samples) {
    // 1. Feature extraction (always runs, using current preset parameters)
    feature_extractor.process(input, num_samples, preset_manager.get_current_preset());

    // 2. Genre classification (runs every 500ms, not every callback)
    genre_timer += num_samples;
    if (genre_timer >= genre_update_interval_samples) {
        genre_timer = 0;

        // Build feature vector from accumulated statistics
        auto features = feature_extractor.get_genre_features();

        if (!preset_manager.is_manual_override()) {
            // Classify
            Genre raw_genre = classifier.classify(features);

            // Smooth
            Genre stable_genre = genre_smoother.update(raw_genre);

            // Apply (preset manager handles cooldown and transition internally)
            preset_manager.request_genre_change(stable_genre);
        }
    }

    // 3. Visual parameter output (every callback)
    auto& preset = preset_manager.get_current_preset();
    visual_params.bass_pulse = feature_extractor.get_band_rms(0)
        * preset.band_weights[0];
    visual_params.mid_energy = feature_extractor.get_band_rms(2)
        * preset.band_weights[2];
    // ... etc for all visual parameters
    visual_engine.update(visual_params);
}
```

---

## Appendix C: Genre-Specific FFT Bin Ranges

Quick-reference table for common analysis bands at each supported FFT size and 44100 Hz sample rate:

```
Band              FFT=1024  FFT=2048  FFT=4096
                  (43.1 Hz) (21.5 Hz) (10.8 Hz)
─────────────────────────────────────────────────
Sub-bass 20-60Hz   1-1       1-2       2-5
Bass 60-250Hz      1-5       3-11      6-23
Low-mid 250-1kHz   6-23      12-46     24-92
Mid 1-4kHz         24-92     47-185    93-371
High-mid 4-8kHz    93-185    186-371   372-743
High 8-16kHz       186-371   372-743   744-1486

Bin index = floor(freq_hz / bin_resolution)
Bin resolution = sample_rate / fft_size
```
