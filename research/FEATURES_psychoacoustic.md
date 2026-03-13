# Psychoacoustic Features for Perceptually Meaningful Visual Mapping

> **Scope**: Extraction, modeling, and real-time approximation of psychoacoustic descriptors for a music visualization / VJ engine.
> **Cross-references**: [FEATURES_amplitude_dynamics.md](FEATURES_amplitude_dynamics.md), [FEATURES_spectral.md](FEATURES_spectral.md), [FEATURES_frequency_bands.md](FEATURES_frequency_bands.md), [VIDEO_feature_to_visual_mapping.md](VIDEO_feature_to_visual_mapping.md), [REF_math_reference.md](REF_math_reference.md)

---

## 0. Rationale: Why Psychoacoustic Features

Raw signal features (RMS, spectral centroid, FFT bin magnitudes) measure *physical* properties of the waveform. Human hearing is nonlinear: equal increments of SPL do not produce equal increments of perceived loudness; equal increments of frequency do not produce equal increments of perceived pitch. When driving visuals from audio, mapping raw features yields results that feel "disconnected" from the music because the visual system is receiving a signal that does not correspond to what the auditory system perceives.

Psychoacoustic features close this gap. They model the auditory periphery (outer/middle ear filtering, basilar membrane, hair cell transduction) and higher-level perceptual grouping. The result is a set of descriptors whose *values* track *perception*, making them superior control signals for cross-modal mapping.

This document covers the core psychoacoustic descriptors, their formal models, real-time approximation strategies, and concrete C++ implementations targeting a 44.1 kHz / 48 kHz stereo pipeline with <5 ms latency budget per feature.

---

## 1. Perceived Loudness

### 1.1 The Problem with dBFS and RMS

A -20 dBFS sine at 3.5 kHz sounds substantially louder than a -20 dBFS sine at 100 Hz or 12 kHz. RMS and peak meters are frequency-blind. A-weighting approximates the equal-loudness contour at low SPL but collapses a family of curves (ISO 226:2003) onto a single filter, losing level-dependent behavior. For visualization, we need a loudness estimate that tracks perceived intensity across the full frequency and level range.

### 1.2 Zwicker Loudness Model (DIN 45631 / ISO 532-1)

Eberhard Zwicker's model (1960, refined through 1990s) is the foundational loudness model and the basis of ISO 532-1 (Method B for stationary sounds, Method A for time-varying). The processing chain:

**Step 1 — Outer/Middle Ear Transfer Function**
Apply the free-field-to-eardrum transfer function. This boosts the 2-5 kHz region by ~10-15 dB (ear canal resonance) and rolls off low frequencies.

**Step 2 — Critical Band Decomposition (Bark Scale)**
Decompose the spectrum into 24 critical bands (Bark bands). The Bark scale maps frequency to perceptual pitch distance:

$$z(\text{Bark}) = 13 \arctan(0.00076 f) + 3.5 \arctan\left(\frac{f}{7500}\right)^2$$

| Bark Band | Center Freq (Hz) | Bandwidth (Hz) |
|-----------|------------------|-----------------|
| 1         | 50               | 80              |
| 2         | 150              | 100             |
| 3         | 250              | 100             |
| 4         | 350              | 100             |
| 5         | 450              | 110             |
| 6         | 570              | 120             |
| 7         | 700              | 140             |
| 8         | 840              | 150             |
| 9         | 1000             | 160             |
| 10        | 1170             | 190             |
| 11        | 1370             | 210             |
| 12        | 1600             | 240             |
| 13        | 1850             | 280             |
| 14        | 2150             | 320             |
| 15        | 2500             | 380             |
| 16        | 2900             | 450             |
| 17        | 3400             | 550             |
| 18        | 4000             | 700             |
| 19        | 4800             | 900             |
| 20        | 5800             | 1100            |
| 21        | 7000             | 1300            |
| 22        | 8500             | 1800            |
| 23        | 10500            | 2500            |
| 24        | 13500            | 3500            |

**Step 3 — Excitation Patterns and Masking**
Each critical band's excitation spreads into neighboring bands via upward and downward masking slopes. The upper slope is level-dependent (shallower at higher levels, ~25 dB/Bark at low levels to ~5 dB/Bark at high levels). The lower slope is relatively constant at ~27 dB/Bark. The excitation pattern E(z) at Bark position z given an excitation at band z_0:

$$E(z) = E_0 \cdot \begin{cases} 10^{S_l(z_0 - z)/10} & z < z_0 \\ 10^{S_u(z - z_0)/10} & z \geq z_0 \end{cases}$$

where S_l = -27 dB/Bark (lower slope) and S_u = -(24 + 0.23 f/kHz - 0.2 L_{dB}) dB/Bark (upper slope, level-dependent).

**Step 4 — Specific Loudness**
Convert excitation in each band to specific loudness N'(z) in sone/Bark using a compressive power law:

$$N'(z) = 0.08 \left(\frac{E_{TQ}(z)}{s(z)}\right)^{0.23} \left[\left(0.5 + 0.5 \frac{E(z)}{E_{TQ}(z)}\right)^{0.23} - 1\right]$$

where E_TQ(z) is the excitation at threshold in quiet and s(z) is a band-dependent normalization factor.

**Step 5 — Total Loudness**
Integrate specific loudness over all Bark bands:

$$N = \int_0^{24} N'(z)\, dz \approx \sum_{i=1}^{24} N'(z_i) \cdot \Delta z_i \quad \text{[sone]}$$

The sone scale: 1 sone = loudness of a 1 kHz tone at 40 dB SPL. Doubling of sone value = doubling of perceived loudness. The phon scale maps level to equal-loudness contours (phon = dB SPL of equally loud 1 kHz tone).

Conversion: $N = 2^{(L_N - 40)/10}$ sone, where L_N is loudness level in phon (above 40 phon).

### 1.3 Moore-Glasberg Loudness Model (ISO 532-2)

Brian Moore and Brian Glasberg (1996, 1997, 2006) developed a refined model using the ERB (Equivalent Rectangular Bandwidth) scale instead of Bark:

$$\text{ERB}(f) = 24.7 \cdot (4.37 f/1000 + 1)$$

$$\text{ERB number: } E = 21.4 \log_{10}(0.00437 f + 1)$$

Key improvements over Zwicker:
- **Better low-frequency accuracy**: Revised threshold-in-quiet and excitation pattern at low frequencies.
- **Improved level-dependent behavior**: More accurate at very low and very high levels.
- **Binaural loudness summation**: Models the ~6 dB advantage of binaural over monaural listening.
- **Time-varying extension** (Glasberg & Moore, 2002): Uses a dual-path temporal integration (attack time constant ~22 ms, release ~50 ms), closely matching temporal loudness data.

The ERB scale provides finer resolution at low frequencies compared to Bark (approximately 39 ERB bands vs. 24 Bark bands over the audible range), yielding smoother specific loudness patterns.

### 1.4 ISO 532 Standards

- **ISO 532-1:2017** (Zwicker method): Method A (time-varying), Method B (stationary). Reference implementation exists. Computationally heavier due to masking pattern convolution.
- **ISO 532-2:2017** (Moore-Glasberg method): Generally considered more accurate. Reference C code published by Moore's group.

### 1.5 Simplified Real-Time Approximation

For real-time visualization at <5 ms latency, the full Zwicker/Moore-Glasberg pipeline is feasible but heavy. A pragmatic approximation:

1. Apply outer/middle ear weighting filter (IIR biquad cascade approximating the free-field transfer function — or simply use an A/C-weighting filter for a cruder version).
2. Compute energy in ~24 Bark bands (or 32 1/3-octave bands) from FFT output.
3. Apply a compressive power law (exponent ~0.23) per band to approximate specific loudness.
4. Sum across bands.
5. Apply temporal smoothing: attack ~5 ms, release ~50 ms (one-pole IIR per band).

This skips explicit masking computation (the biggest CPU cost) but captures the two most important perceptual effects: frequency weighting and compressive loudness growth.

### 1.6 C++ Implementation — Simplified Perceived Loudness

```cpp
#include <cmath>
#include <array>
#include <algorithm>

// Bark band edge frequencies (Hz) — 24 bands + upper edge
static constexpr std::array<float, 25> kBarkEdges = {
    20, 100, 200, 300, 400, 510, 630, 770, 920, 1080,
    1270, 1480, 1720, 2000, 2320, 2700, 3150, 3700, 4400,
    5300, 6400, 7700, 9500, 12000, 15500
};

// Outer/middle ear weighting (dB) per Bark band — simplified from ISO 226
// Positive = boost, negative = attenuation relative to 1 kHz
static constexpr std::array<float, 24> kEarWeightDB = {
    -30.0f, -20.0f, -14.0f, -10.0f, -7.0f, -5.0f, -3.5f, -2.0f,
    0.0f, 0.5f, 0.5f, 0.0f, -0.5f, 0.0f, 2.0f, 4.0f,
    5.0f, 3.0f, 0.0f, -3.0f, -7.0f, -12.0f, -18.0f, -25.0f
};

struct LoudnessEstimator {
    static constexpr int kNumBands = 24;
    static constexpr float kCompressiveExponent = 0.23f;
    // Threshold in quiet, energy units (approximate, per-band)
    static constexpr float kThresholdFloor = 1e-10f;

    // Temporal smoothing state
    std::array<float, kNumBands> specificLoudnessSmoothed{};
    float totalLoudnessSmoothed = 0.0f;

    // Attack/release coefficients (pre-computed for given sample rate / hop size)
    float attackCoeff = 0.0f;
    float releaseCoeff = 0.0f;

    void init(float sampleRate, int hopSize) {
        float dt = static_cast<float>(hopSize) / sampleRate;
        attackCoeff  = 1.0f - std::exp(-dt / 0.005f);  // ~5 ms attack
        releaseCoeff = 1.0f - std::exp(-dt / 0.050f);   // ~50 ms release
    }

    /// Compute perceived loudness from FFT magnitude spectrum.
    /// @param magnitudes  FFT magnitude bins (linear scale, N/2+1 bins)
    /// @param numBins     number of magnitude bins (N/2+1)
    /// @param sampleRate  audio sample rate
    /// @return            total loudness in approximate sone
    float process(const float* magnitudes, int numBins, float sampleRate) {
        float binWidth = sampleRate / static_cast<float>((numBins - 1) * 2);

        // Step 1: Accumulate energy in each Bark band with ear weighting
        std::array<float, kNumBands> bandEnergy{};
        for (int b = 0; b < kNumBands; ++b) {
            int loIdx = std::max(1, static_cast<int>(kBarkEdges[b] / binWidth));
            int hiIdx = std::min(numBins - 1,
                                 static_cast<int>(kBarkEdges[b + 1] / binWidth));
            float earGain = std::pow(10.0f, kEarWeightDB[b] / 20.0f);
            float energy = 0.0f;
            for (int i = loIdx; i <= hiIdx; ++i) {
                float m = magnitudes[i] * earGain;
                energy += m * m;
            }
            bandEnergy[b] = energy;
        }

        // Step 2: Compute specific loudness (compressive power law)
        float totalLoudness = 0.0f;
        for (int b = 0; b < kNumBands; ++b) {
            float E = std::max(bandEnergy[b], kThresholdFloor);
            // Specific loudness ~ E^0.23 (Stevens' power law approximation)
            float N_prime = std::pow(E, kCompressiveExponent);

            // Temporal smoothing (asymmetric attack/release)
            float coeff = (N_prime > specificLoudnessSmoothed[b])
                          ? attackCoeff : releaseCoeff;
            specificLoudnessSmoothed[b] +=
                coeff * (N_prime - specificLoudnessSmoothed[b]);

            totalLoudness += specificLoudnessSmoothed[b];
        }

        // Step 3: Smooth total loudness
        float coeff = (totalLoudness > totalLoudnessSmoothed)
                      ? attackCoeff : releaseCoeff;
        totalLoudnessSmoothed +=
            coeff * (totalLoudness - totalLoudnessSmoothed);

        return totalLoudnessSmoothed;
    }
};
```

---

## 2. Sharpness

### 2.1 Definition and Perceptual Meaning

Sharpness quantifies the proportion of high-frequency energy in a sound's spectrum, weighted by its perceptual salience. High sharpness correlates with sounds perceived as "bright," "cutting," or "piercing." The unit is the **acum**; 1 acum is defined as the sharpness of a narrow-band noise at 1 kHz, 60 dB SPL, with a bandwidth of one critical band.

### 2.2 Aures Model (DIN 45692:2009)

The standard sharpness calculation (von Bismarck 1974, refined by Aures 1985) operates on the specific loudness pattern:

$$S = k \cdot \frac{\int_0^{24\text{ Bark}} N'(z) \cdot g(z) \cdot z \, dz}{\int_0^{24\text{ Bark}} N'(z) \, dz} \quad \text{[acum]}$$

where:
- N'(z) = specific loudness at Bark band z (sone/Bark)
- g(z) = weighting function emphasizing high Bark bands
- k = normalization constant (typically 0.11 for the von Bismarck weighting)

**Weighting function g(z):**

$$g(z) = \begin{cases} 1 & z \leq 15.8 \text{ Bark} \quad (\approx 3150\text{ Hz}) \\ 0.066 \cdot e^{0.171 z} & z > 15.8 \text{ Bark} \end{cases}$$

This gives bands above ~3 kHz exponentially increasing weight. The Aures refinement additionally makes the weighting loudness-dependent, penalizing sharpness at low overall loudness levels (where high-frequency content is less salient):

$$g_{\text{Aures}}(z) = 0.078 \cdot \frac{e^{0.171 z}}{z} \cdot \frac{\ln(N_{\text{total}} + 1)}{\ln(N_{\text{total}})}$$

### 2.3 DIN 45692:2009

The DIN standard formalizes the von Bismarck/Aures sharpness computation. Key details:
- Input: specific loudness from Zwicker model (ISO 532-1).
- g(z) per the piecewise function above.
- Normalization constant k = 0.11.
- Reference: narrowband noise centered at 1 kHz, bandwidth = 1 Bark, level = 60 dB SPL yields S = 1 acum.

### 2.4 C++ Implementation

```cpp
struct SharpnessEstimator {
    static constexpr float kNormK = 0.11f;
    static constexpr float kBarkThreshold = 15.8f;

    /// Compute sharpness from specific loudness array (24 Bark bands).
    /// @param specificLoudness  N'(z), 24 values in sone/Bark
    /// @return                  sharpness in acum
    float compute(const float* specificLoudness, int numBands = 24) {
        float numerator   = 0.0f;
        float denominator = 0.0f;

        for (int i = 0; i < numBands; ++i) {
            float z = static_cast<float>(i) + 0.5f;  // band center in Bark
            float N_prime = specificLoudness[i];

            float g;
            if (z <= kBarkThreshold) {
                g = 1.0f;
            } else {
                g = 0.066f * std::exp(0.171f * z);
            }

            numerator   += N_prime * g * z;
            denominator += N_prime;
        }

        if (denominator < 1e-12f) return 0.0f;

        return kNormK * numerator / denominator;
    }
};
```

**Typical sharpness values:**

| Sound                    | Sharpness (acum) |
|--------------------------|-------------------|
| Male speech (average)    | 0.8 - 1.2        |
| Female speech (average)  | 1.0 - 1.6        |
| Hi-hat                   | 2.5 - 4.0        |
| Ride cymbal              | 2.0 - 3.5        |
| Distorted guitar         | 1.5 - 2.5        |
| Bass guitar              | 0.3 - 0.8        |
| Synthesizer pad (bright) | 1.8 - 3.0        |
| Full mix (rock)          | 1.2 - 2.0        |
| Full mix (electronic)    | 1.5 - 2.5        |

---

## 3. Roughness

### 3.1 Perceptual Definition

Roughness is the sensation produced by rapid amplitude or frequency modulation in the range ~15-300 Hz (modulation frequency). It is distinct from flutter (very slow AM) and pitch (very fast modulation that fuses into a new frequency). The unit is the **asper**; 1 asper = roughness of a 1 kHz tone, 60 dB SPL, 100% AM at 70 Hz modulation rate.

Roughness is a key contributor to musical "distortion," "grit," "growl," and "aggression." In electronic music, roughness correlates with synthesizer detuning, FM synthesis depth, waveshaping distortion, and beating between closely spaced partials.

### 3.2 Fastl & Zwicker Model

Hugo Fastl and Eberhard Zwicker (*Psychoacoustics: Facts and Models*, 3rd ed., 2007) describe roughness as arising from temporal envelope fluctuations within critical bands:

$$R = 0.3 \cdot f_{\text{mod}} \cdot \sum_{i=1}^{24} \Delta L_i \quad \text{[asper]}$$

where:
- f_mod = dominant modulation frequency (Hz) — roughness peaks around 70 Hz
- Delta_L_i = modulation depth (in dB) of the temporal envelope in critical band i

The modulation transfer function (MTF) for roughness shows a bandpass characteristic peaking at ~70 Hz:

$$H_R(f_{\text{mod}}) = \frac{(f_{\text{mod}} / 70)}{1 + (f_{\text{mod}} / 70)^2}$$

### 3.3 Daniel & Weber Model (1997)

Peter Daniel and Reinhard Weber proposed a more detailed computational model:

1. Filter the signal into 47 overlapping critical bands (0.5 Bark spacing).
2. Extract the temporal envelope of each band (half-wave rectification + low-pass at 500 Hz).
3. Compute the modulation spectrum of each envelope via short-time FFT.
4. Weight modulation components by the roughness modulation transfer function (bandpass, peak at 70 Hz).
5. Cross-band interaction: roughness contributions from adjacent bands are partially correlated; apply a cross-correlation correction factor.
6. Integrate across bands and modulation frequencies.

This model is computationally expensive. The critical innovation is explicit handling of cross-band modulation correlation, which the simpler Fastl/Zwicker summation model ignores.

### 3.4 Real-Time Approximation Strategies

For real-time visualization, approximate roughness as follows:

**Method A — Envelope Modulation Energy (Recommended)**

1. Decompose into ~24 Bark bands (reuse loudness filterbank).
2. Extract amplitude envelope per band (magnitude of analytic signal via Hilbert transform, or simple low-pass of |x[n]|).
3. Compute short-time FFT of each envelope (window ~50-100 ms, hop ~10 ms).
4. Sum energy of envelope spectrum in the 15-300 Hz range, weighted by the bandpass MTF peaking at 70 Hz.
5. Sum across critical bands.

**Method B — Subband Temporal Variation**

1. Compute specific loudness per Bark band each frame.
2. Measure frame-to-frame variation (absolute difference or variance over a short window).
3. Apply the roughness MTF weighting based on the variation rate.

Method B is cheaper and often sufficient for visualization; it tracks roughness-correlated energy without explicit modulation-domain analysis.

```cpp
struct RoughnessEstimator {
    static constexpr int kNumBands = 24;
    static constexpr int kHistoryLen = 16;  // ~10-50 ms of envelope history

    // Circular buffer of specific loudness per band
    float envelopeHistory[kNumBands][kHistoryLen]{};
    int writeIdx = 0;

    float smoothedRoughness = 0.0f;
    float releaseCoeff = 0.95f;

    /// Feed per-band specific loudness each frame, get roughness estimate.
    float process(const float* specificLoudness) {
        // Store current frame
        for (int b = 0; b < kNumBands; ++b)
            envelopeHistory[b][writeIdx] = specificLoudness[b];
        writeIdx = (writeIdx + 1) % kHistoryLen;

        // Compute temporal variance of each band's envelope
        float totalRoughness = 0.0f;
        for (int b = 0; b < kNumBands; ++b) {
            float mean = 0.0f;
            for (int k = 0; k < kHistoryLen; ++k)
                mean += envelopeHistory[b][k];
            mean /= kHistoryLen;

            float variance = 0.0f;
            for (int k = 0; k < kHistoryLen; ++k) {
                float d = envelopeHistory[b][k] - mean;
                variance += d * d;
            }
            variance /= kHistoryLen;

            // Weight: bands 5-18 (~450 Hz - 4400 Hz) contribute most
            float bandWeight = (b >= 4 && b <= 17) ? 1.0f : 0.3f;
            totalRoughness += variance * bandWeight;
        }

        // Smooth output
        smoothedRoughness = std::max(totalRoughness,
            smoothedRoughness * releaseCoeff);
        return smoothedRoughness;
    }
};
```

### 3.5 Typical Roughness Values

| Sound / Condition               | Roughness (asper) |
|---------------------------------|--------------------|
| Pure tone (no modulation)       | 0.0                |
| 1 kHz, 70 Hz AM, 100% depth    | 1.0 (reference)    |
| Clean electric guitar chord     | 0.1 - 0.3          |
| Distorted guitar power chord    | 0.5 - 1.5          |
| Detuned synth unison            | 0.3 - 0.8          |
| FM synthesis (high index)       | 0.8 - 2.0          |
| Death metal blast beat          | 1.0 - 2.5          |
| EDM supersaw lead               | 0.4 - 1.2          |
| Orchestral string section       | 0.1 - 0.4          |
| White noise                     | 0.5 - 0.8          |

---

## 4. Fluctuation Strength

### 4.1 Definition

Fluctuation strength quantifies the perception of slow temporal modulations in the range ~0.5-20 Hz — the regime below roughness. The unit is the **vacil**; 1 vacil = fluctuation strength of a 1 kHz tone, 60 dB SPL, 100% AM at 4 Hz modulation rate.

Fluctuation strength peaks at ~4 Hz modulation rate, which corresponds to the syllabic rate of speech and the typical rhythmic pulse rate of music (120 BPM = 2 Hz quarter notes, but 16th-note subdivisions at 8 Hz).

### 4.2 Model

The Fastl/Zwicker model:

$$F = \frac{0.008 \cdot \int_0^{24} \Delta L(z) \, dz}{\left(\frac{f_{\text{mod}}}{4}\right) + \left(\frac{4}{f_{\text{mod}}}\right)} \quad \text{[vacil]}$$

where:
- Delta_L(z) = modulation depth (dB) in Bark band z
- f_mod = modulation frequency (Hz)

The denominator creates a bandpass shape peaking at f_mod = 4 Hz. This captures the perceptual reality that modulations around 4 Hz are maximally salient for the fluctuation sensation.

### 4.3 Temporal Envelope Analysis

Real-time computation:

1. Compute per-band temporal envelopes (same as roughness pipeline).
2. Compute modulation spectrum of each envelope.
3. Weight by the fluctuation strength MTF (bandpass peaking at 4 Hz, bandwidth ~0.5-20 Hz).
4. Integrate across bands.

Alternatively, extract the broadband temporal envelope (RMS over ~5-20 ms windows), compute its modulation spectrum, and weight by the 4 Hz bandpass. This single-band approach is much cheaper and correlates well with perceived groove/pulse.

### 4.4 Relationship to Groove Perception

Fluctuation strength in the 1-8 Hz range directly correlates with perceived "groove" or rhythmic drive. Madison & Sioros (2014) showed that:
- Peak fluctuation at ~2 Hz (quarter note at 120 BPM) correlates with perceived "pulse."
- Fluctuation at ~4-8 Hz correlates with perceived rhythmic subdivision and "swing."
- Sub-1 Hz fluctuation corresponds to phrase-level dynamics (not groove but narrative arc).

For visualization, fluctuation strength is an excellent driver for rhythmic visual pulsation — it captures the rate at which the listener *feels* the beat, not merely the onset locations.

### 4.5 Typical Values

| Sound                        | Fluctuation Strength (vacil) |
|------------------------------|-------------------------------|
| Steady-state tone            | 0.0                          |
| Speech (normal)              | 0.4 - 0.8                    |
| 4-on-the-floor kick (house)  | 0.5 - 1.2                    |
| Tremolo guitar (6 Hz)        | 0.3 - 0.7                    |
| Sidechain-pumped pad         | 0.8 - 1.5                    |
| Breakbeat pattern            | 0.3 - 0.6                    |
| Ambient pad (very slow mod)  | 0.05 - 0.2                   |

---

## 5. Sensory Pleasantness

### 5.1 Composite Model

Sensory pleasantness (or sensory consonance/euphony) is not a single acoustic property but a composite perceptual dimension. The model by Fastl (1999, 2006) and Zwicker & Fastl (2007, ch. 16) defines pleasantness as a weighted combination:

$$P = k_1 - k_2 \cdot R - k_3 \cdot S + k_4 \cdot T - k_5 \cdot |N - N_{\text{pref}}|$$

where:
- R = roughness (asper)
- S = sharpness (acum)
- T = tonalness (0 to 1)
- N = total loudness (sone)
- N_pref = preferred loudness level (~10-20 sone for many contexts)
- k_1...k_5 = weighting constants fitted to subjective data

Interpretation:
- **Higher tonalness** increases pleasantness (harmonic, pitched content preferred).
- **Higher roughness** decreases pleasantness (dissonance, harshness).
- **Higher sharpness** decreases pleasantness (too bright / piercing).
- **Loudness far from preferred range** decreases pleasantness (too quiet or too loud).

### 5.2 Application to Visualization

Sensory pleasantness can drive a "mood" axis in visual mapping:
- Low pleasantness: harsh textures, fragmented geometry, high-contrast edges.
- High pleasantness: smooth gradients, flowing curves, harmonic color palettes.

This provides an aggregate aesthetic descriptor that abstracts away the individual contributors, useful as a global visual "tone" control.

---

## 6. Tonalness

### 6.1 Definition

Tonalness (also: tonality, tone-to-noise ratio) measures the degree to which a sound is perceived as tonal (pitched, harmonic) versus noisy (broadband, inharmonic). It ranges conceptually from 0 (pure noise) to 1 (pure tone).

### 6.2 Ratio of Tonal to Noise Components

The Aures (1985) tonalness model:

1. **Peak detection**: Identify spectral peaks that stand above the local noise floor by a threshold (e.g., >7 dB above local spectral median within a critical bandwidth).
2. **Tonal energy**: Sum energy of all identified tonal peaks.
3. **Noise energy**: Total energy minus tonal energy.
4. **Tone-to-noise ratio** (dB):

$$\text{TNR} = 10 \log_{10} \frac{W_{\text{tonal}}}{W_{\text{noise}}}$$

5. **Tonalness** mapped to 0-1:

$$T = \frac{1}{1 + e^{-\alpha(\text{TNR} - \beta)}}$$

where alpha and beta are sigmoid parameters fitted to perceptual data (typically alpha ~ 0.5, beta ~ 5 dB).

### 6.3 Harmonic Series Detection

For music, tonalness can be enhanced by harmonic template matching:

1. Identify candidate fundamental frequencies (f0 candidates).
2. For each candidate, check energy at harmonics: f0, 2f0, 3f0, ..., nf0.
3. A strong harmonic series (many harmonics with energy above the noise floor) yields high tonalness.
4. Weight by the number and strength of confirmed harmonics.

This approach differentiates a pitched note (high tonalness) from broadband percussive hits (low tonalness) even if both have comparable spectral peaks.

### 6.4 Pitch Salience

Pitch salience is closely related to tonalness and refers to how clearly a pitch is perceived. The MPEG-7 AudioSpectrumFlatness descriptor captures a related property. For visualization:

- High pitch salience (tonal) suggests mapping to structured, periodic visual patterns.
- Low pitch salience (noisy) suggests mapping to stochastic, particle-like textures.

### 6.5 Real-Time Estimation

```cpp
struct TonalnessEstimator {
    /// Estimate tonalness from FFT magnitudes.
    /// @param magnitudes  FFT magnitude bins (linear), N/2+1 bins
    /// @param numBins     number of bins
    /// @param sampleRate  in Hz
    /// @return            tonalness in [0, 1]
    float compute(const float* magnitudes, int numBins, float sampleRate) {
        float binWidth = sampleRate / static_cast<float>((numBins - 1) * 2);

        // Compute local spectral median in sliding window (~1 Bark wide)
        // and identify tonal peaks
        float tonalEnergy = 0.0f;
        float totalEnergy = 0.0f;

        constexpr int kMedianHalfWin = 12;  // ~half critical band at mid freqs

        for (int i = kMedianHalfWin; i < numBins - kMedianHalfWin; ++i) {
            float mag = magnitudes[i];
            float magSq = mag * mag;
            totalEnergy += magSq;

            // Compute local median energy (approximated by local mean
            // for efficiency — true median requires sorting)
            float localSum = 0.0f;
            for (int j = -kMedianHalfWin; j <= kMedianHalfWin; ++j) {
                if (j != 0) {
                    float m = magnitudes[i + j];
                    localSum += m * m;
                }
            }
            float localMean = localSum / (2.0f * kMedianHalfWin);

            // Peak stands >7 dB above local floor?
            if (magSq > localMean * 5.0f) {  // 5x energy ~ 7 dB
                tonalEnergy += magSq;
            }
        }

        if (totalEnergy < 1e-20f) return 0.0f;

        float tnr_linear = tonalEnergy / std::max(totalEnergy - tonalEnergy, 1e-20f);
        float tnr_dB = 10.0f * std::log10(std::max(tnr_linear, 1e-10f));

        // Sigmoid mapping to [0, 1]
        float tonalness = 1.0f / (1.0f + std::exp(-0.5f * (tnr_dB - 5.0f)));
        return tonalness;
    }
};
```

---

## 7. Perceptual Brightness and Warmth

### 7.1 Brightness

Perceptual brightness is the timbral dimension corresponding to the proportion of high-frequency energy. It is operationalized as the **loudness-weighted spectral centroid**:

$$B = \frac{\sum_{i=1}^{K} f_i \cdot N'(f_i)}{\sum_{i=1}^{K} N'(f_i)}$$

where f_i is the center frequency of bin/band i and N'(f_i) is the specific loudness at that frequency. This differs from the raw spectral centroid (which uses magnitude or power weights) by accounting for equal-loudness contour shaping — a 10 kHz component with moderate energy contributes less to brightness than the raw centroid would suggest, because it is perceptually attenuated at moderate listening levels.

**Relation to sharpness**: Brightness and sharpness are correlated (r ~ 0.85 in typical music signals) but not identical. Sharpness emphasizes frequencies above 3 kHz via the g(z) weighting function, while brightness is a spectral balance measure across the full range. A signal can be moderately bright without being sharp (warm synth pad with gentle HF rolloff) or sharp without being exceptionally bright (narrowband noise at 6 kHz).

### 7.2 Warmth

Warmth is the complementary timbral dimension: the perception of rich low-mid energy. It can be operationalized as:

$$W = \frac{\sum_{z=2}^{8} N'(z)}{\sum_{z=1}^{24} N'(z)}$$

This is the ratio of specific loudness in Bark bands 2-8 (~100-920 Hz) to total specific loudness. High warmth corresponds to sounds with strong fundamental and low-order harmonics: acoustic bass, cello, baritone voice, Rhodes piano.

### 7.3 Timbral Space for Visualization

Brightness and warmth span a two-dimensional timbral space useful for visual mapping:

```
High Brightness, Low Warmth:  Hi-hat, crash cymbal, synth arp
High Brightness, High Warmth: Acoustic guitar strumming (full range)
Low Brightness, High Warmth:  Upright bass, sub-bass
Low Brightness, Low Warmth:   Muffled/filtered sounds
```

These can drive color temperature (warm = orange/amber, bright = white/cyan), visual density, or material properties in 3D scenes.

---

## 8. Stereo Features

### 8.1 Stereo Width / Spread

Stereo width quantifies how broadly a sound image spans the stereo field. Two standard measures:

**8.1.1 Cross-Correlation Coefficient**

The normalized cross-correlation of left and right channels at zero lag:

$$r_{LR} = \frac{\sum_{n} L[n] \cdot R[n]}{\sqrt{\sum_{n} L[n]^2 \cdot \sum_{n} R[n]^2}}$$

- r_LR = +1: identical (mono, center-panned) — minimum width.
- r_LR = 0: uncorrelated (independent signals) — wide stereo.
- r_LR = -1: inverted polarity — "outside the speakers," maximum perceived width (but phase issues).

Stereo width as a normalized 0-1 value:

$$W_{\text{stereo}} = \frac{1 - r_{LR}}{2}$$

**8.1.2 Mid/Side Ratio**

Compute mid (M = L + R) and side (S = L - R) signals:

$$\text{MS Ratio} = \frac{E_S}{E_M + E_S}$$

where E_M and E_S are RMS energies of M and S respectively. A mono signal has MS Ratio = 0; a fully decorrelated signal has MS Ratio = 0.5; an out-of-phase signal approaches 1.

### 8.2 Stereo Imbalance

$$\text{Balance} = \frac{E_R - E_L}{E_R + E_L} \in [-1, +1]$$

where E_L = RMS energy of left, E_R = RMS energy of right. Negative = left-heavy, positive = right-heavy.

### 8.3 Binaural Cues

Though our input is stereo (not binaural), the following cues are derivable and perceptually meaningful:

**8.3.1 Interaural Time Difference (ITD)**

The time delay between left and right channels, estimated via peak of the cross-correlation function:

$$\text{ITD} = \arg\max_{\tau} \sum_{n} L[n] \cdot R[n + \tau]$$

For stereo music, ITD is not a true binaural cue but reflects panning via time-difference techniques (e.g., Blumlein pair recording). Typical range: +/- 1 ms (corresponds to +/- 30 cm path difference at 343 m/s speed of sound).

**8.3.2 Interaural Level Difference (ILD)**

$$\text{ILD}(f) = 20 \log_{10} \frac{|R(f)|}{|L(f)|} \quad \text{[dB]}$$

Computed per-frequency from the STFT. ILD is frequency-dependent in natural binaural signals (head shadow increases with frequency) but in stereo mixes reflects panning decisions per frequency band. A frequency-band ILD profile reveals spatial placement of different instruments.

### 8.4 Spatial Impression Metrics

**Apparent Source Width (ASW)**: Correlated with early lateral reflections in room acoustics, but in stereo mixes can be approximated by frequency-dependent correlation:

$$\text{ASW}(z) = 1 - r_{LR}(z)$$

where r_LR(z) is the cross-correlation computed within Bark band z. Elements panned differently in different frequency bands create a wide ASW even if the broadband correlation is moderate.

**Listener Envelopment (LEV)**: Correlated with late lateral energy. In stereo mixes, approximated by decorrelation in the low-frequency region (<500 Hz) — often driven by reverb tails.

### 8.5 C++ Implementation — Stereo Width

```cpp
struct StereoAnalyzer {
    /// Compute stereo width, balance, and correlation from interleaved stereo.
    /// @param samples     interleaved L,R,L,R,...
    /// @param numFrames   number of stereo frames
    struct Result {
        float correlation;  // [-1, +1]
        float width;        // [0, 1]
        float balance;      // [-1, +1] (negative = left)
        float msRatio;      // [0, 1]
    };

    Result process(const float* samples, int numFrames) {
        float sumLL = 0.0f, sumRR = 0.0f, sumLR = 0.0f;
        float sumL = 0.0f, sumR = 0.0f;

        for (int i = 0; i < numFrames; ++i) {
            float L = samples[i * 2];
            float R = samples[i * 2 + 1];
            sumLL += L * L;
            sumRR += R * R;
            sumLR += L * R;
            sumL  += std::abs(L);
            sumR  += std::abs(R);
        }

        Result r{};

        // Correlation
        float denom = std::sqrt(sumLL * sumRR);
        r.correlation = (denom > 1e-20f) ? (sumLR / denom) : 1.0f;
        r.correlation = std::clamp(r.correlation, -1.0f, 1.0f);

        // Width (0 = mono, 1 = fully decorrelated)
        r.width = (1.0f - r.correlation) * 0.5f;

        // Balance
        float totalEnergy = sumLL + sumRR;
        r.balance = (totalEnergy > 1e-20f)
            ? (sumRR - sumLL) / totalEnergy : 0.0f;

        // Mid/Side ratio
        // M = L+R, S = L-R
        // E_M = sumLL + sumRR + 2*sumLR
        // E_S = sumLL + sumRR - 2*sumLR
        float EM = sumLL + sumRR + 2.0f * sumLR;
        float ES = sumLL + sumRR - 2.0f * sumLR;
        EM = std::max(EM, 0.0f);
        ES = std::max(ES, 0.0f);
        r.msRatio = (EM + ES > 1e-20f) ? (ES / (EM + ES)) : 0.0f;

        return r;
    }
};
```

### 8.6 Frequency-Dependent Stereo Analysis

For richer visual mapping, compute per-band stereo width from STFT:

```cpp
/// Per-bin stereo correlation from complex STFT frames.
/// @param leftFFT   complex FFT of left channel (N/2+1 bins)
/// @param rightFFT  complex FFT of right channel
/// @param corrOut   output correlation per bin
/// @param numBins   N/2+1
void perBinCorrelation(const std::complex<float>* leftFFT,
                       const std::complex<float>* rightFFT,
                       float* corrOut, int numBins) {
    for (int i = 0; i < numBins; ++i) {
        auto L = leftFFT[i];
        auto R = rightFFT[i];
        float crossReal = L.real() * R.real() + L.imag() * R.imag();
        float magL = std::abs(L);
        float magR = std::abs(R);
        float denom = magL * magR;
        corrOut[i] = (denom > 1e-20f) ? (crossReal / denom) : 1.0f;
    }
}
```

---

## 9. Perceptually Meaningful Visual Mapping

### 9.1 Why Psychoacoustic Features Create Better Mappings

Cross-modal correspondence research (Spence 2011, Marks 1987, Walker 2002) demonstrates systematic associations between auditory and visual dimensions:

- **Loudness and brightness** are universally linked (louder = brighter).
- **Pitch and spatial position** are linked (higher pitch = higher vertical position).
- **Roughness/dissonance and visual angularity** are linked (rough sounds map to jagged shapes — the "bouba/kiki" effect, Ramachandran & Hubbard 2001).
- **Sharpness and visual edge contrast** are linked (sharp sounds map to high-contrast, high-spatial-frequency visuals).

Using raw signal features (RMS for loudness, spectral centroid for brightness) creates mappings that often feel "off" because the visual response does not track perception. For example:
- RMS responds equally to a 50 Hz sub-bass rumble and a 3.5 kHz sine. But the listener perceives the 3.5 kHz sine as dramatically louder at the same RMS. Using perceived loudness for visual intensity corrects this.
- Raw spectral centroid weights all frequencies linearly, but the ear's sensitivity drops above 8 kHz and below 200 Hz. A loudness-weighted centroid (brightness) better tracks perceived timbral color.

### 9.2 Mapping Table

| Psychoacoustic Feature        | Visual Parameter         | Mapping Shape    | Rationale                                                                                      |
|-------------------------------|--------------------------|------------------|-----------------------------------------------------------------------------------------------|
| Perceived loudness (sone)     | Global brightness / glow | Power (gamma 0.6)| Perceptual loudness already compressive; additional gamma for aesthetic range compression       |
| Sharpness (acum)              | Edge contrast / detail   | Linear           | Sharp sounds = crisp visuals. Drive high-pass or detail enhancement shaders                    |
| Roughness (asper)             | Distortion / noise       | Exponential      | Low roughness = clean; high roughness = heavy distortion. Nonlinear mapping for impact         |
| Fluctuation strength (vacil) | Pulse / rhythmic motion  | Direct modulator | Drive periodic visual oscillation at the fluctuation rate itself (4 Hz visual pulse = groove)  |
| Tonalness                     | Geometric order          | Linear           | Tonal = structured patterns (grids, crystals); noisy = particles, clouds, noise textures       |
| Brightness (Hz)               | Color temperature        | Linear map to hue| High brightness = cool whites/blues; low brightness = warm ambers/reds                         |
| Warmth                        | Color saturation (warm)  | Linear           | High warmth = saturated warm tones; low warmth = desaturated or cool tones                     |
| Sensory pleasantness          | Visual harmony           | Sigmoid          | Pleasant = smooth curves, consonant color palettes; unpleasant = angular, clashing              |
| Stereo width                  | Visual spread / panorama | Linear           | Wide stereo = wide visual field; mono = focused center. Drive FOV or particle spread           |
| Stereo balance                | Visual position          | Linear           | Left-heavy audio = left-weighted visual mass                                                    |

### 9.3 Mapping Design Principles

**Principle 1: Match Perceptual Scales**
The mapping function should transform the psychoacoustic feature (already in perceptual units) to the visual parameter's perceptual scale. For visual brightness, this means accounting for display gamma and the Stevens power law for luminance perception (perceived brightness ~ L^0.33 to L^0.5). Since loudness is already perceptually scaled, a simple gamma curve provides display correction without additional perceptual nonlinearity.

**Principle 2: Temporal Alignment**
Psychoacoustic features have intrinsic temporal resolutions:
- Loudness: ~2-5 ms temporal integration (attack), ~50-100 ms release.
- Roughness: requires ~50-100 ms window for modulation analysis.
- Fluctuation strength: requires ~200-500 ms to resolve sub-20 Hz modulations.

Visual updates must respect these timescales. Driving a visual parameter from a feature faster than its temporal resolution produces noise, not signal.

**Principle 3: Cross-Modal Correspondence Strength**
Some audio-visual mappings are more "natural" (congruent) than others. Loudness-to-brightness is extremely strong (near-universal). Roughness-to-angularity is moderate (culturally validated but less universal). Unusual mappings (e.g., roughness-to-color-hue) can be artistically interesting but violate cross-modal expectations and may feel arbitrary to audiences.

**Principle 4: Avoid Redundant Mapping**
Do not map multiple correlated audio features to the same visual parameter. Loudness and sharpness are partially correlated (louder sounds often sharper). Map them to independent visual dimensions to maximize information transfer.

### 9.4 Multi-Feature Visual State Vector

For a complete visual state at each frame, compute the following vector:

```
V(t) = [
    loudness(t),           // 0-1 normalized sone → brightness
    sharpness(t),          // 0-1 normalized acum → edge detail
    roughness(t),          // 0-1 normalized asper → distortion amount
    fluctuation(t),        // 0-1 normalized vacil → pulse amplitude
    tonalness(t),          // 0-1 → structure/chaos slider
    brightness(t),         // 0-1 normalized spectral centroid → color temp
    warmth(t),             // 0-1 → color saturation
    pleasantness(t),       // 0-1 → aesthetic harmony
    stereoWidth(t),        // 0-1 → spatial spread
    stereoBalance(t)       // -1..+1 → L/R position
]
```

This 10-dimensional vector drives the rendering engine at each audio analysis frame. Individual elements can be routed to shader uniforms, particle system parameters, camera control, or generative geometry.

---

## 10. References

### Foundational Psychoacoustic Texts

1. **Zwicker, E. & Fastl, H.** (2007). *Psychoacoustics: Facts and Models*, 3rd Edition. Springer. — The definitive reference. Covers loudness, sharpness, roughness, fluctuation strength, and their computational models.

2. **Moore, B.C.J.** (2012). *An Introduction to the Psychology of Hearing*, 6th Edition. Brill. — Authoritative on auditory perception, critical bands, masking, and loudness.

3. **Fastl, H.** (2006). "Psychoacoustic basis of sound quality evaluation and sound engineering." *Proceedings of the 13th International Congress on Sound and Vibration (ICSV13)*.

### Loudness

4. **Zwicker, E.** (1960). "Ein Verfahren zur Berechnung der Lautstärke." *Acustica*, 10, 304-308. — Original Zwicker loudness model.

5. **Moore, B.C.J. & Glasberg, B.R.** (1996). "A revision of Zwicker's loudness model." *Acustica*, 82, 335-345.

6. **Glasberg, B.R. & Moore, B.C.J.** (2002). "A model of loudness applicable to time-varying sounds." *Journal of the Audio Engineering Society*, 50(5), 331-342.

7. **ISO 532-1:2017**. Acoustics — Methods for calculating loudness — Part 1: Zwicker method.

8. **ISO 532-2:2017**. Acoustics — Methods for calculating loudness — Part 2: Moore-Glasberg method.

### Sharpness

9. **von Bismarck, G.** (1974). "Sharpness as an attribute of the timbre of steady sounds." *Acustica*, 30, 159-172.

10. **Aures, W.** (1985). "Berechnungsverfahren für den sensorischen Wohlklang beliebiger Schallsignale." *Acustica*, 59, 130-141. — Tonalness, sharpness, pleasantness models.

11. **DIN 45692:2009**. Measurement technique for the simulation of the auditory sensation of sharpness.

### Roughness and Fluctuation Strength

12. **Daniel, P. & Weber, R.** (1997). "Psychoacoustical roughness: Implementation of an optimized model." *Acustica*, 83, 113-123.

13. **Vassilakis, P.N.** (2005). "Auditory roughness as a means of musical expression." *Selected Reports in Ethnomusicology*, 12, 119-144.

### Tonalness and Pitch

14. **Terhardt, E., Stoll, G., & Seewann, M.** (1982). "Algorithm for extraction of pitch and pitch salience from complex tonal signals." *Journal of the Acoustical Society of America*, 71(3), 679-688.

15. **ECMA-74:2019**. Measurement of airborne noise emitted by information technology and telecommunications equipment. — Contains prominence ratio and tone-to-noise ratio methods.

### Cross-Modal Correspondence

16. **Spence, C.** (2011). "Crossmodal correspondences: A tutorial review." *Attention, Perception, & Psychophysics*, 73, 971-995.

17. **Marks, L.E.** (1987). "On cross-modal similarity: Auditory-visual interactions in speeded discrimination." *Journal of Experimental Psychology: Human Perception and Performance*, 13(3), 384-394.

18. **Ramachandran, V.S. & Hubbard, E.M.** (2001). "Synaesthesia — A window into perception, thought and language." *Journal of Consciousness Studies*, 8(12), 3-34.

19. **Walker, P.** (2002). "Cross-sensory correspondences: A model of shared quality." *Perception*, 31, 893-917.

20. **Giannakis, K. & Smith, M.** (2001). "Imaging soundscapes: Identifying cognitive associations between auditory and visual dimensions." In *Music, Gestalt, and Computing* (Springer), 161-179.

### Stereo and Spatial Audio

21. **Blauert, J.** (1997). *Spatial Hearing: The Psychophysics of Human Sound Localization*. MIT Press.

22. **Rumsey, F.** (2001). *Spatial Audio*. Focal Press.

### ISO Standards

23. **ISO 226:2003**. Acoustics — Normal equal-loudness-level contours.

24. **ISO 389-7:2005**. Acoustics — Reference zero for the calibration of audiometric equipment — Part 7: Reference threshold of hearing under free-field and diffuse-field listening conditions.

---

## Appendix A: Bark-to-Frequency and ERB-to-Frequency Conversion Tables

### Bark Scale (Zwicker)

| Bark | Lower (Hz) | Center (Hz) | Upper (Hz) | Bandwidth (Hz) |
|------|------------|-------------|------------|-----------------|
| 0.5  | 20         | 50          | 100        | 80              |
| 1.5  | 100        | 150         | 200        | 100             |
| 2.5  | 200        | 250         | 300        | 100             |
| 3.5  | 300        | 350         | 400        | 100             |
| 4.5  | 400        | 450         | 510        | 110             |
| 5.5  | 510        | 570         | 630        | 120             |
| 6.5  | 630        | 700         | 770        | 140             |
| 7.5  | 770        | 840         | 920        | 150             |
| 8.5  | 920        | 1000        | 1080       | 160             |
| 9.5  | 1080       | 1170        | 1270       | 190             |
| 10.5 | 1270       | 1370        | 1480       | 210             |
| 11.5 | 1480       | 1600        | 1720       | 240             |
| 12.5 | 1720       | 1850        | 2000       | 280             |
| 13.5 | 2000       | 2150        | 2320       | 320             |
| 14.5 | 2320       | 2500        | 2700       | 380             |
| 15.5 | 2700       | 2900        | 3150       | 450             |
| 16.5 | 3150       | 3400        | 3700       | 550             |
| 17.5 | 3700       | 4000        | 4400       | 700             |
| 18.5 | 4400       | 4800        | 5300       | 900             |
| 19.5 | 5300       | 5800        | 6400       | 1100            |
| 20.5 | 6400       | 7000        | 7700       | 1300            |
| 21.5 | 7700       | 8500        | 9500       | 1800            |
| 22.5 | 9500       | 10500       | 12000      | 2500            |
| 23.5 | 12000      | 13500       | 15500      | 3500            |

### ERB Scale (Moore-Glasberg)

$$\text{ERB}(f) = 24.7 \cdot (4.37 \cdot f/1000 + 1) \quad \text{[Hz]}$$

| Freq (Hz) | ERB (Hz) | ERB Number |
|-----------|----------|------------|
| 50        | 30.2     | 1.9        |
| 100       | 35.6     | 3.4        |
| 200       | 46.3     | 6.0        |
| 500       | 78.7     | 11.2       |
| 1000      | 132.6    | 15.6       |
| 2000      | 240.5    | 20.2       |
| 4000      | 456.3    | 25.1       |
| 8000      | 887.9    | 30.5       |
| 16000     | 1751.1   | 36.2       |

---

## Appendix B: Perceptual Loudness Scale Reference

| dB SPL (1 kHz) | Phon | Sone   | Perceived                        |
|----------------|------|--------|----------------------------------|
| 0              | 0    | 0.0    | Threshold of hearing             |
| 10             | 10   | 0.02   | Barely audible                   |
| 20             | 20   | 0.07   | Very quiet (recording studio)    |
| 30             | 30   | 0.18   | Quiet room                       |
| 40             | 40   | 1.0    | Reference: 1 sone                |
| 50             | 50   | 2.0    | Moderate conversation            |
| 60             | 60   | 4.0    | Normal conversation              |
| 70             | 70   | 8.0    | Busy traffic                     |
| 80             | 80   | 16.0   | Loud music (club)                |
| 90             | 90   | 32.0   | Very loud (approaching pain)     |
| 100            | 100  | 64.0   | Extremely loud                   |
| 110            | 110  | 128.0  | Threshold of discomfort          |
| 120            | 120  | 256.0  | Threshold of pain                |

*Above 40 phon:* $N = 2^{(L_{\text{phon}} - 40)/10}$ sone

*Below 40 phon:* The relationship is steeper; loudness drops more rapidly toward threshold.

---

## Appendix C: Complete Feature Extraction Pipeline (Pseudocode)

```
per_frame(left[], right[], N):
    // 1. Windowed FFT (both channels)
    L_fft = FFT(left * hann_window)
    R_fft = FFT(right * hann_window)
    L_mag = abs(L_fft)
    R_mag = abs(R_fft)
    mono_mag = (L_mag + R_mag) / 2

    // 2. Bark band decomposition (shared across features)
    bark_energy[24] = accumulate_bark_bands(mono_mag, sampleRate)
    bark_energy_weighted[24] = apply_ear_weighting(bark_energy)

    // 3. Specific loudness
    specific_loudness[24] = compressive_power(bark_energy_weighted, 0.23)

    // 4. Features
    loudness       = sum(specific_loudness)
    sharpness      = compute_sharpness(specific_loudness)
    roughness      = compute_roughness(specific_loudness)  // needs history
    fluctuation    = compute_fluctuation(specific_loudness) // needs longer history
    tonalness      = compute_tonalness(mono_mag, numBins, sampleRate)
    brightness     = loudness_weighted_centroid(mono_mag, specific_loudness)
    warmth         = sum(specific_loudness[1:8]) / loudness
    pleasantness   = composite(tonalness, roughness, sharpness, loudness)

    // 5. Stereo features
    stereo         = compute_stereo(left, right, N)
    // stereo.width, stereo.balance, stereo.correlation, stereo.msRatio

    // 6. Normalize to [0,1] using running min/max or fixed ranges
    feature_vector = normalize_all(loudness, sharpness, roughness,
                                   fluctuation, tonalness, brightness,
                                   warmth, pleasantness,
                                   stereo.width, stereo.balance)

    return feature_vector  // → visual engine
```

---

*End of document. For spectral feature extraction details, see [FEATURES_spectral.md](FEATURES_spectral.md). For amplitude/dynamics features, see [FEATURES_amplitude_dynamics.md](FEATURES_amplitude_dynamics.md). For frequency band decomposition strategies, see [FEATURES_frequency_bands.md](FEATURES_frequency_bands.md). For visual mapping implementation, see [VIDEO_feature_to_visual_mapping.md](VIDEO_feature_to_visual_mapping.md). For mathematical foundations (FFT, window functions, filter design), see [REF_math_reference.md](REF_math_reference.md).*
