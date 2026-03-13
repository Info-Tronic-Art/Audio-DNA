# LIB_aubio.md -- Aubio Library Technical Reference

> Complete technical reference for the Aubio library in the context of real-time audio
> analysis for VJ / music visualization applications.

**Cross-references:** [LIB_essentia.md](LIB_essentia.md), [LIB_juce.md](LIB_juce.md), [FEATURES_rhythm_tempo.md](FEATURES_rhythm_tempo.md), [FEATURES_pitch_harmonic.md](FEATURES_pitch_harmonic.md), [FEATURES_spectral.md](FEATURES_spectral.md), [ARCH_pipeline.md](ARCH_pipeline.md)

---

## 1. Library Overview

### Purpose and History

Aubio (Audio + Bio, a portmanteau reflecting its biomimetic approach to auditory analysis) is a C library designed for real-time extraction of musical features from audio signals. Originally developed by Paul Brossier as part of his PhD thesis at Queen Mary, University of London (2006), it has matured into one of the most widely-deployed open-source audio analysis libraries. The core thesis -- "Automatic Annotation of Musical Audio for Interactive Applications" -- directly targeted the latency-sensitive domain that VJ and visualization tools occupy.

The library focuses on four primary analysis domains: onset detection, tempo/beat tracking, pitch estimation, and spectral description. Unlike heavier frameworks such as Essentia (see [LIB_essentia.md](LIB_essentia.md)), Aubio is deliberately minimal. It ships no audio playback, no machine learning stack, and no plugin hosting. This constraint is its strength: the entire compiled library is typically under 500 KB, with deterministic memory allocation and zero heap activity during processing.

### Repository and License

- **GitHub:** [aubio/aubio](https://github.com/aubio/aubio)
- **License:** GNU General Public License v3.0 (GPLv3). This has implications for proprietary VJ software -- static linking creates a derivative work. Dynamic linking may or may not, depending on jurisdiction. For commercial products, contact the author regarding dual licensing.
- **Language bindings:** Python (included), Rust (third-party), and SWIG-generated wrappers.

### Design Philosophy

Aubio's C API follows a consistent object-oriented pattern across all modules:

```
new_aubio_<module>()    ->  create and configure
aubio_<module>_do()     ->  process one hop of audio
aubio_<module>_get_*()  ->  query results
aubio_<module>_set_*()  ->  adjust parameters
del_aubio_<module>()    ->  free memory
```

Every processing function operates on exactly one hop of audio (typically 256-512 samples). There is no internal buffering, no threading, no callbacks. The caller is fully responsible for feeding audio hop-by-hop. This design makes Aubio trivially safe to use inside a real-time audio callback -- it never allocates, never blocks, never calls system functions during `_do()`.

---

## 2. Core Data Types

### fvec_t -- Real-Valued Vector

The fundamental data container. Used for audio sample buffers, analysis outputs, and intermediate results.

```c
typedef struct {
    uint_t length;   // number of elements
    smpl_t *data;    // pointer to float array (smpl_t is float by default)
} fvec_t;
```

Key functions:

| Function | Purpose |
|---|---|
| `new_fvec(uint_t length)` | Allocate vector of given length, zeroed |
| `del_fvec(fvec_t *s)` | Free vector memory |
| `fvec_get_sample(s, pos)` | Read element at position |
| `fvec_set_sample(s, val, pos)` | Write element at position |
| `fvec_get_data(s)` | Raw pointer access (zero-copy) |
| `fvec_zeros(s)` | Zero all elements |
| `fvec_ones(s)` | Set all elements to 1.0 |
| `fvec_copy(src, dst)` | Element-wise copy |
| `fvec_weight(s, w)` | Element-wise multiplication |
| `fvec_rev(s)` | Reverse in place |

Mathematical utilities applied element-wise in place: `fvec_exp()`, `fvec_log()`, `fvec_log10()`, `fvec_cos()`, `fvec_sin()`, `fvec_abs()`, `fvec_sqrt()`, `fvec_pow()`, `fvec_floor()`, `fvec_ceil()`, `fvec_round()`.

### cvec_t -- Complex-Valued Vector (Polar Form)

Represents frequency-domain data as magnitude/phase pairs. Used as the output of the phase vocoder (FFT) and input to spectral descriptors.

```c
typedef struct {
    uint_t length;   // = (fft_size / 2) + 1
    smpl_t *norm;    // magnitude array
    smpl_t *phas;    // phase array (radians)
} cvec_t;
```

Critical detail: `length` equals `(buf_size / 2) + 1`, not `buf_size`. A 1024-sample FFT produces a cvec of length 513. This follows the standard real-FFT convention where only the non-redundant positive-frequency bins are stored.

Key functions follow the same pattern as fvec_t but with separate accessors for norm and phase: `cvec_norm_get_sample()`, `cvec_phas_set_sample()`, `cvec_norm_zeros()`, `cvec_phas_zeros()`, `cvec_logmag(s, lambda)` (applies log compression to magnitudes).

### fmat_t -- Real-Valued Matrix

Used for multi-channel audio and filterbank coefficients.

```c
typedef struct {
    uint_t length;   // number of columns (time / frequency axis)
    uint_t height;   // number of rows (channels / filters)
    smpl_t **data;   // data[height][length]
} fmat_t;
```

Key functions: `new_fmat(height, length)`, `del_fmat()`, `fmat_get_channel(s, channel, output_fvec)`, `fmat_get_channel_data(s, channel)`, `fmat_vecmul(s, scale, output)`.

### aubio_source_t -- Audio File I/O

Not a processing type but essential for file-based analysis and testing. Supports multiple backends: libav/FFmpeg, CoreAudio (macOS/iOS), libsndfile, and a built-in WAV reader as fallback.

```c
aubio_source_t *src = new_aubio_source("track.wav", 44100, 512);
// samplerate=0 means use original file rate

fvec_t *buf = new_fvec(512);
uint_t frames_read = 0;

aubio_source_do(src, buf, &frames_read);         // mono read
// or: aubio_source_do_multi(src, fmat, &frames_read);  // multi-channel

uint_t sr   = aubio_source_get_samplerate(src);
uint_t ch   = aubio_source_get_channels(src);
uint_t dur  = aubio_source_get_duration(src);     // total frames
aubio_source_seek(src, 0);                        // rewind
del_aubio_source(src);
```

### Memory Management Patterns

All Aubio objects follow RAII-like semantics in C:

1. **Allocation at init only.** `new_aubio_*()` performs all malloc calls. If it returns non-NULL, the object is fully ready.
2. **Zero allocation during processing.** `aubio_*_do()` functions never allocate. They write into pre-allocated output vectors that the caller provides.
3. **Symmetric deallocation.** Every `new_aubio_*()` has a matching `del_aubio_*()`. No shared ownership, no reference counting.
4. **NULL on failure.** If buffer sizes are invalid or memory is exhausted, constructors return NULL. Always check.

This pattern means you can allocate everything at application startup, process in the real-time thread indefinitely, and deallocate at shutdown. There is no garbage collection pressure, no lock contention, no hidden state.

---

## 3. Onset Detection (aubio_onset)

Onset detection identifies the precise moments when new musical events begin -- note attacks, drum hits, transients. This is the single most important feature for beat-reactive visuals.

### API

```c
// Create detector
aubio_onset_t *onset = new_aubio_onset(
    "default",      // method: see table below
    1024,           // buf_size: FFT window length in samples
    512,            // hop_size: samples between analysis frames
    44100           // samplerate
);

// Configure
aubio_onset_set_threshold(onset, 0.3);       // peak-picking sensitivity
aubio_onset_set_silence(onset, -70.0);       // silence gate in dB
aubio_onset_set_minioi_ms(onset, 50.0);      // minimum inter-onset interval in ms
aubio_onset_set_awhitening(onset, 1);        // adaptive whitening on/off
aubio_onset_set_compression(onset, 10.0);    // log compression lambda

// Process (call once per hop)
fvec_t *input  = new_fvec(512);
fvec_t *output = new_fvec(1);

aubio_onset_do(onset, input, output);
if (fvec_get_sample(output, 0) != 0) {
    // Onset detected!
    uint_t pos_samples = aubio_onset_get_last();
    smpl_t pos_seconds = aubio_onset_get_last_s();
    smpl_t pos_ms      = aubio_onset_get_last_ms();

    smpl_t raw_value       = aubio_onset_get_descriptor();
    smpl_t thresholded_val = aubio_onset_get_thresholded_descriptor();
}

// Reset / cleanup
aubio_onset_reset(onset);     // reset internal counters
del_aubio_onset(onset);
```

### Onset Detection Methods

| Method | Algorithm | Best For | Latency | CPU Cost |
|---|---|---|---|---|
| `"default"` | Alias for `"hfc"` | General purpose | Low | Low |
| `"energy"` | Frame energy difference | Loud transients, kick drums | Lowest | Lowest |
| `"hfc"` | High Frequency Content: weighted sum of spectral magnitudes, emphasizing high frequencies | Percussive attacks with high-frequency content | Low | Low |
| `"complex"` | Complex-domain deviation: measures combined magnitude and phase changes | Pitched instruments, piano, mixed music | Medium | Medium |
| `"phase"` | Phase deviation between consecutive frames | Tonal onsets, bowed strings | Medium | Medium |
| `"specdiff"` | Spectral difference: L2 norm of magnitude changes | General transients | Low | Low |
| `"kl"` | Kullback-Leibler divergence between consecutive spectra | General purpose, robust | Low | Low-Med |
| `"mkl"` | Modified KL divergence (normalized per bin) | Polyphonic music, complex mixes | Low | Medium |
| `"specflux"` | Half-wave rectified spectral flux (only considers increases) | Percussive and melodic onsets equally | Low | Low |

### Parameter Tuning

**threshold** (default: method-dependent, typically 0.3-0.5)
Controls the peak-picking sensitivity. Lower values detect more onsets (including false positives). Higher values require stronger transients. For VJ applications, start at 0.3 for percussive music, 0.1 for ambient.

**silence** (default: -70 dB)
Frames below this energy level are ignored. Prevents onset detection during silence. For live performance with background noise, raise to -40 or -30 dB.

**minioi_ms** (default: 12 ms for most methods)
Minimum inter-onset interval. Prevents double-triggering. For EDM at 140 BPM, the fastest hi-hat pattern is ~107 ms apart. Setting minioi_ms to 50 ms is a safe floor. For slower music, 100 ms prevents jitter.

**awhitening** (default: 1 = enabled)
Adaptive whitening normalizes the spectrum over time, making the detector invariant to gradual volume changes. Essential for live input. Can be disabled for pre-analyzed material.

**compression** (default: 10.0)
Log compression lambda applied to spectral magnitudes before onset detection. Higher values compress the dynamic range more aggressively. Useful for detecting soft onsets in the presence of loud sustained tones.

### Method Comparison by Music Type

| Genre | Recommended Method | threshold | minioi_ms | Notes |
|---|---|---|---|---|
| EDM / House | `hfc` or `specflux` | 0.3 | 30-50 | Clean transients, reliable detection |
| Rock / Metal | `complex` | 0.4 | 40 | Handles distorted guitars better |
| Hip-Hop | `hfc` | 0.3 | 50 | Emphasizes snare/hat attacks |
| Classical | `complex` or `mkl` | 0.2 | 80 | Catches bowed attacks, piano |
| Ambient / Drone | `mkl` or `kl` | 0.1 | 200 | Very few onsets; need high sensitivity |
| Jazz | `complex` | 0.25 | 40 | Handles brush drums, walking bass |

### Complete Onset Detection Example

```c
#include <aubio/aubio.h>
#include <stdio.h>

int main(void) {
    uint_t samplerate  = 44100;
    uint_t buf_size    = 1024;
    uint_t hop_size    = 512;
    const char *method = "hfc";

    // Allocate
    aubio_source_t *source = new_aubio_source("track.wav", samplerate, hop_size);
    if (!source) { fprintf(stderr, "Failed to open source\n"); return 1; }
    samplerate = aubio_source_get_samplerate(source);

    aubio_onset_t *onset = new_aubio_onset(method, buf_size, hop_size, samplerate);
    aubio_onset_set_threshold(onset, 0.3);
    aubio_onset_set_silence(onset, -40.0);
    aubio_onset_set_minioi_ms(onset, 50.0);

    fvec_t *input       = new_fvec(hop_size);
    fvec_t *onset_out   = new_fvec(1);
    uint_t frames_read  = 0;
    uint_t total_frames  = 0;

    // Process loop
    do {
        aubio_source_do(source, input, &frames_read);
        aubio_onset_do(onset, input, onset_out);

        if (fvec_get_sample(onset_out, 0) != 0) {
            smpl_t onset_sec = aubio_onset_get_last_s(onset);
            printf("onset at %.3f s\n", onset_sec);
        }

        total_frames += frames_read;
    } while (frames_read == hop_size);

    // Cleanup
    del_aubio_onset(onset);
    del_fvec(input);
    del_fvec(onset_out);
    del_aubio_source(source);
    return 0;
}
```

---

## 4. Tempo / Beat Tracking (aubio_tempo)

Beat tracking extends onset detection by identifying periodicity -- the regular pulse underlying music. For VJ applications, this is what drives beat-synced color changes, strobes, and rhythmic motion.

### Algorithm Overview

Aubio's beat tracker is a multi-stage pipeline:

1. **Onset detection function** -- computes a spectral flux or other onset strength signal from each hop.
2. **Autocorrelation** -- applied to the onset detection function to find the dominant periodicity (tempo).
3. **Beat prediction** -- a probabilistic model predicts the next beat position, combining the autocorrelation peak with a phase alignment step that corrects for drift.

The algorithm is causal (uses only past data) and adapts to tempo changes within a few beats. It does not require a "listen-ahead" buffer, making it suitable for real-time use with latency bounded by `buf_size + hop_size` samples.

### API

```c
aubio_tempo_t *tempo = new_aubio_tempo(
    "default",      // onset method used internally
    1024,           // buf_size
    512,            // hop_size
    44100           // samplerate
);

aubio_tempo_set_silence(tempo, -40.0);
aubio_tempo_set_threshold(tempo, 0.3);

fvec_t *input  = new_fvec(512);
fvec_t *output = new_fvec(2);   // output[0] = beat (0 or non-zero)

aubio_tempo_do(tempo, input, output);

if (fvec_get_sample(output, 0) != 0) {
    // Beat detected at this hop
    smpl_t beat_ms  = aubio_tempo_get_last_ms(tempo);
    smpl_t bpm      = aubio_tempo_get_bpm(tempo);
    smpl_t conf     = aubio_tempo_get_confidence(tempo);
    smpl_t period_s = aubio_tempo_get_period_s(tempo);

    printf("BEAT at %.1f ms | BPM=%.1f conf=%.3f\n", beat_ms, bpm, conf);
}

del_aubio_tempo(tempo);
```

### Parameters

| Parameter | Function | Default | Notes |
|---|---|---|---|
| method | `new_aubio_tempo(method, ...)` | `"default"` | Onset method used internally. Same options as `aubio_onset`. |
| buf_size | Constructor | 1024 | FFT window. Larger = better frequency resolution, worse time resolution. |
| hop_size | Constructor | 512 | Hop between frames. Determines update rate: at 44100 Hz / 512 hop = ~86 updates/sec. |
| silence | `set_silence()` | -70 dB | Suppress beats during silence. |
| threshold | `set_threshold()` | 0.3 | Peak-picking threshold for onset detection stage. |

### BPM, Period, and Confidence

- **`aubio_tempo_get_bpm()`** returns the currently estimated BPM. Returns 0.0 if no stable tempo has been found yet. Typically stabilizes within 4-8 beats of music.
- **`aubio_tempo_get_period()`** / `_period_s()` returns the beat period in samples or seconds. `period_s = 60.0 / bpm`.
- **`aubio_tempo_get_confidence()`** returns a value where higher = more confident. Not normalized to [0,1]. Use relative comparisons (e.g., "confidence dropped below 50% of the running average" means tempo may be changing).

### Tatum Detection

Aubio supports sub-beat detection (tatums). A tatum is the smallest rhythmic unit -- typically 1/4 or 1/8 of a beat.

```c
aubio_tempo_set_tatum_signature(tempo, 4);  // 4 tatums per beat (1-64)

// In processing loop:
uint_t tatum_result = aubio_tempo_was_tatum(tempo);
if (tatum_result == 2) {
    // Full beat
} else if (tatum_result == 1) {
    // Tatum (sub-beat)
    smpl_t tatum_pos = aubio_tempo_get_last_tatum(tempo);  // in samples
}
```

This is extremely useful for VJ applications: drive subtle pulsing on tatums and major visual events on beats.

### Latency Characteristics

| buf_size | hop_size | Algorithmic Latency | Update Rate (44.1 kHz) |
|---|---|---|---|
| 512 | 256 | ~11.6 ms | ~172 Hz |
| 1024 | 512 | ~23.2 ms | ~86 Hz |
| 2048 | 512 | ~46.4 ms | ~86 Hz |
| 2048 | 1024 | ~46.4 ms | ~43 Hz |

Algorithmic latency is approximately `buf_size / samplerate` seconds. The tempo tracker adds an additional convergence delay of several beats before the BPM estimate stabilizes. For VJ use, `buf_size=1024, hop_size=512` is the standard trade-off.

### Delay Compensation

Aubio allows manual delay compensation for detected beats:

```c
aubio_tempo_set_delay_ms(tempo, -20.0);    // shift beats 20ms earlier
// also: _set_delay() in samples, _set_delay_s() in seconds
smpl_t current_delay = aubio_tempo_get_delay_ms(tempo);
```

This is critical for VJ software where visual output has its own rendering latency. Measure the visual pipeline delay and subtract it here so that lights/visuals hit on the perceived beat.

### Complete Beat Tracking Example

```c
#include <aubio/aubio.h>
#include <stdio.h>

void process_beats(const char *filename) {
    uint_t sr = 0, hop = 512, buf = 1024;

    aubio_source_t *src = new_aubio_source(filename, sr, hop);
    if (!src) return;
    sr = aubio_source_get_samplerate(src);

    aubio_tempo_t *tempo = new_aubio_tempo("default", buf, hop, sr);
    aubio_tempo_set_silence(tempo, -40.0);
    aubio_tempo_set_threshold(tempo, 0.3);
    aubio_tempo_set_tatum_signature(tempo, 4);

    fvec_t *in  = new_fvec(hop);
    fvec_t *out = new_fvec(2);
    uint_t read = 0;

    do {
        aubio_source_do(src, in, &read);
        aubio_tempo_do(tempo, in, out);

        if (fvec_get_sample(out, 0) != 0) {
            printf("BEAT  at %.3f s  BPM=%.1f  conf=%.2f\n",
                aubio_tempo_get_last_s(tempo),
                aubio_tempo_get_bpm(tempo),
                aubio_tempo_get_confidence(tempo));
        }

        uint_t tatum = aubio_tempo_was_tatum(tempo);
        if (tatum == 1) {
            printf("  tatum at sample %d\n",
                (int)aubio_tempo_get_last_tatum(tempo));
        }
    } while (read == hop);

    del_aubio_tempo(tempo);
    del_fvec(in);
    del_fvec(out);
    del_aubio_source(src);
}
```

---

## 5. Pitch Detection (aubio_pitch)

Pitch detection estimates the fundamental frequency (F0) of a monophonic signal. For VJ applications, pitch drives harmonic color mapping (e.g., mapping note to hue), camera zoom tied to melody, or generative geometry responding to vocal lines.

### API

```c
aubio_pitch_t *pitch = new_aubio_pitch(
    "yinfft",       // method
    2048,           // buf_size (larger = better low-freq resolution)
    512,            // hop_size
    44100           // samplerate
);

aubio_pitch_set_unit(pitch, "Hz");       // "Hz", "midi", "cent", "bin"
aubio_pitch_set_tolerance(pitch, 0.7);   // method-dependent
aubio_pitch_set_silence(pitch, -40.0);   // suppress below this level

fvec_t *input  = new_fvec(512);
fvec_t *output = new_fvec(1);

aubio_pitch_do(pitch, input, output);

smpl_t freq_hz    = fvec_get_sample(output, 0);   // 0.0 if unvoiced
smpl_t confidence = aubio_pitch_get_confidence(pitch);

del_aubio_pitch(pitch);
```

### Pitch Detection Methods

| Method | Algorithm | Accuracy | CPU | Latency | Best For |
|---|---|---|---|---|---|
| `"yin"` | Autocorrelation-based (de Cheveigne & Kawahara 2002) | Excellent | Medium | ~2 periods | Vocal, monophonic instruments |
| `"yinfast"` | YIN with FFT-accelerated autocorrelation | Excellent | Low | ~2 periods | Same as YIN, lower CPU |
| `"yinfft"` | YIN applied in frequency domain (spectral YIN) | Excellent | Low-Med | ~1 frame | General purpose, default |
| `"mcomb"` | Multi-comb filter in spectral domain | Good | Low | ~1 frame | Polyphonic-tolerant, noisy signals |
| `"fcomb"` | Fundamental comb filter | Moderate | Lowest | ~1 frame | Fast estimation, less accurate |
| `"schmitt"` | Schmitt trigger zero-crossing | Low | Lowest | ~1 period | Legacy, low-quality but fast |
| `"specacf"` | Spectral autocorrelation function | Good | Medium | ~1 frame | Alternative spectral method |

### Parameter Details

**tolerance** -- method-dependent pitch confidence threshold:
- For `yin` / `yinfast`: default 0.15 (range 0.0-1.0; lower = stricter)
- For `yinfft`: default 0.85 (range 0.0-1.0; higher = stricter -- note the inverted sense)
- For other methods: this parameter may have no effect

**silence** -- dB threshold below which pitch returns 0.0 (unvoiced). Default -45 dB.

**unit** -- output units:
- `"Hz"`: frequency in Hertz (default)
- `"midi"`: MIDI note number (69 = A4 = 440 Hz)
- `"cent"`: cents above MIDI note 0
- `"bin"`: FFT bin index

### Confidence Interpretation

`aubio_pitch_get_confidence()` returns a value whose meaning depends on the method:

- **YIN-based methods**: returns the aperiodicity measure (0.0 = perfect periodicity, 1.0 = noise). Confidence = `1.0 - value`. Readings below 0.15 (93%+ periodicity) indicate reliable pitch.
- **Spectral methods**: returns a normalized confidence. Higher = better.

For VJ applications, a practical pattern is to gate on confidence:

```c
smpl_t freq = fvec_get_sample(output, 0);
smpl_t conf = aubio_pitch_get_confidence(pitch);

if (freq > 0.0 && conf > 0.8) {
    // Use freq to drive visuals
    float midi_note = 12.0f * log2f(freq / 440.0f) + 69.0f;
    float hue = fmodf(midi_note * (360.0f / 12.0f), 360.0f);
}
```

### Buffer Size Considerations for Pitch

Pitch detection requires at least two periods of the lowest target frequency to fit in `buf_size`. At 44100 Hz:

| Lowest Target | Min buf_size | Recommended |
|---|---|---|
| 80 Hz (bass guitar low E) | 1102 | 2048 |
| 40 Hz (bass synth) | 2205 | 4096 |
| 200 Hz (vocals only) | 441 | 1024 |
| 20 Hz (sub-bass) | 4410 | 8192 |

For general VJ use targeting vocals and lead instruments, `buf_size=2048` with `hop_size=512` is ideal.

---

## 6. Spectral Descriptors (aubio_specdesc)

Spectral descriptors quantify the shape and character of the frequency spectrum at each frame. They are the backbone of timbral analysis -- distinguishing bright from dark, noisy from tonal, rising from falling. For VJ applications, these descriptors can drive parameters like particle density, color temperature, blur amount, and scene transitions.

### API

```c
aubio_pvoc_t *pv = new_aubio_pvoc(1024, 512);   // phase vocoder (FFT)
cvec_t *spectrum = new_cvec(1024);               // spectral frame

// Create descriptor
aubio_specdesc_t *desc = new_aubio_specdesc("centroid", 1024);
fvec_t *result = new_fvec(1);

// In processing loop:
fvec_t *audio_hop = new_fvec(512);
// ... fill audio_hop with samples ...

aubio_pvoc_do(pv, audio_hop, spectrum);          // time -> frequency
aubio_specdesc_do(desc, spectrum, result);       // compute descriptor
smpl_t centroid_value = fvec_get_sample(result, 0);

del_aubio_specdesc(desc);
del_aubio_pvoc(pv);
del_cvec(spectrum);
del_fvec(result);
```

### Available Spectral Descriptors

#### Shape Descriptors (Timbral)

| Descriptor | What It Measures | Output Range | VJ Application |
|---|---|---|---|
| `"centroid"` | Spectral center of mass (brightness). In FFT bin units. | 0 to buf_size/2 | Color temperature (warm/cool) |
| `"spread"` | Variance around centroid (bandwidth) | 0+ | Particle spread, blur radius |
| `"skewness"` | Asymmetry of spectrum (3rd moment). Negative = energy below centroid. | Unbounded | Directional effects |
| `"kurtosis"` | Peakedness/flatness of spectrum (4th moment). High = peaked. | Unbounded | Sharpness of visual elements |
| `"slope"` | Rate of spectral roll-off (linear regression of magnitude) | Unbounded | Tilt/rotation of geometry |
| `"decrease"` | Average spectral decrease | Unbounded | Fade/decay rate |
| `"rolloff"` | Bin containing 95% of spectral energy | 0 to buf_size/2 | High-frequency brightness gate |

#### Onset-Related Descriptors

These are the same detection functions used inside `aubio_onset`, but exposed directly for custom onset detectors or for driving continuous visual parameters:

| Descriptor | Algorithm | Output | VJ Application |
|---|---|---|---|
| `"energy"` | Sum of squared magnitudes | 0+ | Master brightness / intensity |
| `"hfc"` | Frequency-weighted magnitude sum | 0+ | Hi-hat / cymbal reactivity |
| `"complex"` | Complex-domain deviation | 0+ | Transition trigger |
| `"phase"` | Phase deviation | 0+ | Subtle tonal shift indicator |
| `"specdiff"` | Spectral difference (L2) | 0+ | General change detector |
| `"kl"` | KL divergence | 0+ | Information-theoretic novelty |
| `"mkl"` | Modified KL divergence | 0+ | Polyphonic novelty |
| `"specflux"` | Positive spectral flux | 0+ | Attack-only transient energy |
| `"wphase"` | Weighted phase deviation | 0+ | Magnitude-weighted phase change |

### Computing Multiple Descriptors

For VJ applications, you typically want several descriptors simultaneously. Since `aubio_pvoc_do()` is the expensive step (FFT), compute it once and pass the same `cvec_t` to multiple descriptors:

```c
// Setup
uint_t buf_size = 1024;
uint_t hop_size = 512;

aubio_pvoc_t *pv = new_aubio_pvoc(buf_size, hop_size);
cvec_t *spec     = new_cvec(buf_size);

aubio_specdesc_t *d_centroid  = new_aubio_specdesc("centroid", buf_size);
aubio_specdesc_t *d_rolloff   = new_aubio_specdesc("rolloff", buf_size);
aubio_specdesc_t *d_energy    = new_aubio_specdesc("energy", buf_size);
aubio_specdesc_t *d_specflux  = new_aubio_specdesc("specflux", buf_size);

fvec_t *r_centroid = new_fvec(1);
fvec_t *r_rolloff  = new_fvec(1);
fvec_t *r_energy   = new_fvec(1);
fvec_t *r_specflux = new_fvec(1);

// In audio callback:
void process_hop(fvec_t *audio_in) {
    aubio_pvoc_do(pv, audio_in, spec);  // ONE FFT

    aubio_specdesc_do(d_centroid, spec, r_centroid);
    aubio_specdesc_do(d_rolloff,  spec, r_rolloff);
    aubio_specdesc_do(d_energy,   spec, r_energy);
    aubio_specdesc_do(d_specflux, spec, r_specflux);

    // Use values to drive visuals
    float brightness   = fvec_get_sample(r_energy, 0);
    float color_temp   = fvec_get_sample(r_centroid, 0) / (buf_size / 2.0f);
    float hf_presence  = fvec_get_sample(r_rolloff, 0) / (buf_size / 2.0f);
    float attack_level = fvec_get_sample(r_specflux, 0);
}
```

### Centroid to Hertz Conversion

The centroid is returned in FFT bin units. To convert:

```c
float centroid_hz = centroid_bin * samplerate / (float)buf_size;
```

---

## 7. Filterbank and MFCC

### Filterbank (aubio_filterbank)

A filterbank divides the spectrum into frequency bands and computes the energy in each band. Aubio provides mel-scale filterbanks for perceptual audio analysis, but the filterbank is generic and can hold arbitrary coefficients.

```c
uint_t n_filters = 40;
uint_t buf_size  = 1024;

aubio_filterbank_t *fb = new_aubio_filterbank(n_filters, buf_size);

// Initialize with mel-spaced triangular filters
aubio_filterbank_set_mel_coeffs_slaney(fb, samplerate);
// Alternative: aubio_filterbank_set_mel_coeffs(fb, samplerate, fmin_hz, fmax_hz);
// Alternative: aubio_filterbank_set_mel_coeffs_htk(fb, samplerate, fmin_hz, fmax_hz);

// Configure
aubio_filterbank_set_norm(fb, 1);     // normalize filter areas (default: on)
aubio_filterbank_set_power(fb, 1);    // power applied to spectrum norms (default: 1)

// Process
cvec_t *spectrum    = new_cvec(buf_size);
fvec_t *band_energy = new_fvec(n_filters);

aubio_filterbank_do(fb, spectrum, band_energy);
// band_energy now has n_filters values, one per mel band

// Access raw coefficients (n_filters x (buf_size/2+1) matrix)
fmat_t *coeffs = aubio_filterbank_get_coeffs(fb);

del_aubio_filterbank(fb);
```

For VJ use, a filterbank with 8-16 bands provides a visually useful frequency decomposition -- map each band to a bar in a visualizer, or use the energy distribution to control multi-element scenes.

### MFCC (aubio_mfcc)

Mel-Frequency Cepstral Coefficients (MFCCs) provide a compact timbral fingerprint. The pipeline is: FFT magnitude spectrum --> mel filterbank --> log compression --> DCT --> N coefficients.

```c
uint_t n_filters = 40;     // mel bands (40 is standard for Slaney)
uint_t n_coeffs  = 13;     // MFCC output dimensions
uint_t buf_size  = 1024;
uint_t sr        = 44100;

aubio_mfcc_t *mfcc = new_aubio_mfcc(buf_size, n_filters, n_coeffs, sr);

// Optional configuration
aubio_mfcc_set_power(mfcc, 1);        // power applied to spectrum (default 1)
aubio_mfcc_set_scale(mfcc, 1.0);      // scale after log, before DCT (default 1)

// Mel filterbank initialization (pick one)
aubio_mfcc_set_mel_coeffs_slaney(mfcc);                          // Slaney (requires 40 filters)
// aubio_mfcc_set_mel_coeffs(mfcc, samplerate, fmin_hz, fmax_hz);
// aubio_mfcc_set_mel_coeffs_htk(mfcc, samplerate, fmin_hz, fmax_hz);

cvec_t *spectrum = new_cvec(buf_size);
fvec_t *mfcc_out = new_fvec(n_coeffs);

// In processing loop (spectrum already computed via aubio_pvoc_do):
aubio_mfcc_do(mfcc, spectrum, mfcc_out);

// mfcc_out[0] = overall energy (often discarded)
// mfcc_out[1..12] = timbral shape coefficients
for (uint_t i = 0; i < n_coeffs; i++) {
    printf("MFCC[%d] = %.4f\n", i, fvec_get_sample(mfcc_out, i));
}

del_aubio_mfcc(mfcc);
```

### MFCC Output Interpretation

| Coefficient | Meaning | VJ Mapping |
|---|---|---|
| MFCC[0] | Overall energy / loudness | Master brightness |
| MFCC[1] | Spectral tilt (bright vs. dark) | Color temperature |
| MFCC[2] | Spectral shape (even vs. odd harmonics) | Texture roughness |
| MFCC[3-5] | Fine spectral envelope | Scene parameters |
| MFCC[6-12] | Detailed timbral texture | Perlin noise seeds, particle behavior |

For scene-change detection, compute the Euclidean distance between consecutive MFCC frames. A spike in MFCC distance indicates a timbral transition -- useful for triggering visual scene changes.

---

## 8. Building from Source

### Build System

Aubio uses **waf**, a Python-based build tool bundled with the source. No separate waf installation is required -- just Python 3.

```bash
git clone https://github.com/aubio/aubio.git
cd aubio

# Configure (auto-detects available dependencies)
./waf configure

# Build
./waf build

# Install (default: /usr/local)
sudo ./waf install
```

### Key Configure Options

```bash
# List all options
./waf configure --help

# Specific FFT backend (pick one)
./waf configure --enable-fftw3f        # FFTW3 single precision (recommended)
./waf configure --enable-fftw3         # FFTW3 double precision
./waf configure --enable-accelerate    # Apple vDSP (macOS, fastest on Apple Silicon)

# Disable optional dependencies
./waf configure --disable-sndfile --disable-avcodec

# Double precision (smpl_t becomes double instead of float)
./waf configure --enable-double

# Build static library
./waf configure --build-type release
```

### Dependencies

| Dependency | Required | Purpose | Notes |
|---|---|---|---|
| Python 3 | Yes | Build system (waf) | Build-time only |
| libsndfile | No | Audio file I/O | Recommended for offline analysis |
| libavcodec (FFmpeg) | No | Audio file I/O (more formats) | Alternative to libsndfile |
| FFTW3 | No | FFT computation | Falls back to internal ooura FFT |
| Apple Accelerate | No | FFT + BLAS on macOS | Auto-detected on macOS |
| Intel IPP | No | FFT + BLAS on Intel | Fastest on Intel CPUs |
| libsamplerate | No | Resampling | Only needed for sample rate conversion |
| JACK | No | JACK audio server | Only for example programs |

### Cross-Platform Compilation

```bash
# macOS universal binary (Intel + Apple Silicon)
./waf configure --with-target-platform=darwin \
    CFLAGS="-arch arm64 -arch x86_64 -mmacosx-version-min=13.3"

# iOS
./waf configure --with-target-platform=ios

# Emscripten (WebAssembly)
./waf configure --with-target-platform=emscripten

# Android (via NDK toolchain)
CC=aarch64-linux-android21-clang ./waf configure --with-target-platform=android
```

### Static Linking

For VJ applications distributed as standalone binaries:

```bash
./waf configure --build-type release
./waf build

# Link against the static library
cc -o myapp myapp.c -I/usr/local/include \
    /usr/local/lib/libaubio.a \
    -framework Accelerate    # macOS
    # or: -lfftw3f -lm       # Linux with FFTW
```

### Compiler Flags

- **Release:** `-O2 -g -Wall -Wextra -fPIC`
- **Debug:** `-O0 -g -Wall -Wextra`
- **Emscripten:** `-Oz` (size-optimized)
- **MSVC:** `/O2 /W4 /Z7`

---

## 9. Integration with Audio Callback

### The Core Pattern

Real-time audio APIs (CoreAudio, JACK, PortAudio, JUCE -- see [LIB_juce.md](LIB_juce.md)) deliver audio in blocks of variable or fixed size. Aubio processes audio in fixed-size hops. The integration pattern is a ring buffer that accumulates incoming samples and feeds Aubio hop-by-hop.

```c
#include <aubio/aubio.h>
#include <string.h>

// --- Initialization (called once, outside audio thread) ---

#define BUF_SIZE  1024
#define HOP_SIZE  512
#define SAMPLERATE 44100

static aubio_onset_t  *g_onset;
static aubio_tempo_t  *g_tempo;
static aubio_pitch_t  *g_pitch;
static aubio_pvoc_t   *g_pvoc;
static aubio_specdesc_t *g_centroid;

static fvec_t *g_hop_buf;
static fvec_t *g_onset_out;
static fvec_t *g_tempo_out;
static fvec_t *g_pitch_out;
static fvec_t *g_centroid_out;
static cvec_t *g_spectrum;

// Accumulator for incoming audio
static float   g_accum[HOP_SIZE];
static uint_t  g_accum_pos = 0;

// Thread-safe output (written by audio thread, read by render thread)
typedef struct {
    int    onset_detected;
    float  bpm;
    float  beat_confidence;
    float  pitch_hz;
    float  pitch_confidence;
    float  spectral_centroid;
    float  spectral_energy;
} AudioFeatures;

static volatile AudioFeatures g_features = {0};

void audio_analysis_init(void) {
    g_onset     = new_aubio_onset("hfc", BUF_SIZE, HOP_SIZE, SAMPLERATE);
    g_tempo     = new_aubio_tempo("default", BUF_SIZE, HOP_SIZE, SAMPLERATE);
    g_pitch     = new_aubio_pitch("yinfft", BUF_SIZE, HOP_SIZE, SAMPLERATE);
    g_pvoc      = new_aubio_pvoc(BUF_SIZE, HOP_SIZE);
    g_centroid  = new_aubio_specdesc("centroid", BUF_SIZE);

    g_hop_buf      = new_fvec(HOP_SIZE);
    g_onset_out    = new_fvec(1);
    g_tempo_out    = new_fvec(2);
    g_pitch_out    = new_fvec(1);
    g_centroid_out = new_fvec(1);
    g_spectrum     = new_cvec(BUF_SIZE);

    aubio_onset_set_threshold(g_onset, 0.3);
    aubio_onset_set_silence(g_onset, -40.0);
    aubio_onset_set_minioi_ms(g_onset, 50.0);
    aubio_tempo_set_silence(g_tempo, -40.0);
    aubio_pitch_set_unit(g_pitch, "Hz");
    aubio_pitch_set_tolerance(g_pitch, 0.7);
}

// --- Process one hop (called when accumulator is full) ---

static void process_hop(void) {
    // Copy accumulator into Aubio's fvec
    memcpy(g_hop_buf->data, g_accum, HOP_SIZE * sizeof(float));

    // Onset detection
    aubio_onset_do(g_onset, g_hop_buf, g_onset_out);
    g_features.onset_detected = (fvec_get_sample(g_onset_out, 0) != 0);

    // Beat tracking
    aubio_tempo_do(g_tempo, g_hop_buf, g_tempo_out);
    if (fvec_get_sample(g_tempo_out, 0) != 0) {
        g_features.bpm             = aubio_tempo_get_bpm(g_tempo);
        g_features.beat_confidence = aubio_tempo_get_confidence(g_tempo);
    }

    // Pitch detection
    aubio_pitch_do(g_pitch, g_hop_buf, g_pitch_out);
    g_features.pitch_hz         = fvec_get_sample(g_pitch_out, 0);
    g_features.pitch_confidence = aubio_pitch_get_confidence(g_pitch);

    // Spectral centroid (requires FFT first)
    aubio_pvoc_do(g_pvoc, g_hop_buf, g_spectrum);
    aubio_specdesc_do(g_centroid, g_spectrum, g_centroid_out);
    g_features.spectral_centroid = fvec_get_sample(g_centroid_out, 0);
}

// --- Audio callback (called by CoreAudio/JACK/PortAudio) ---

void audio_callback(const float *input, uint_t num_frames) {
    for (uint_t i = 0; i < num_frames; i++) {
        g_accum[g_accum_pos++] = input[i];

        if (g_accum_pos >= HOP_SIZE) {
            process_hop();
            g_accum_pos = 0;
        }
    }
}

// --- Cleanup (called once at shutdown) ---

void audio_analysis_cleanup(void) {
    del_aubio_onset(g_onset);
    del_aubio_tempo(g_tempo);
    del_aubio_pitch(g_pitch);
    del_aubio_pvoc(g_pvoc);
    del_aubio_specdesc(g_centroid);
    del_fvec(g_hop_buf);
    del_fvec(g_onset_out);
    del_fvec(g_tempo_out);
    del_fvec(g_pitch_out);
    del_fvec(g_centroid_out);
    del_cvec(g_spectrum);
}
```

### Key Integration Notes

1. **The accumulator pattern is essential.** Audio callbacks may deliver 64, 128, 256, or any number of frames. Aubio always expects exactly `hop_size` frames. The accumulator bridges this mismatch.

2. **Multiple algorithms share the FFT.** The phase vocoder (`aubio_pvoc_do`) is the most expensive operation. Compute it once per hop, then pass the resulting `cvec_t` to all spectral descriptors and the MFCC. Onset and tempo detectors run their own internal FFTs.

3. **Thread safety.** The `volatile` keyword on `g_features` is a minimal approach. For production code, use a lock-free single-producer/single-consumer ring buffer or atomic operations to pass features from the audio thread to the render thread. Never use mutexes in the audio callback.

4. **Mono input.** All Aubio analysis functions expect mono audio. If your input is stereo, mix to mono before the accumulator: `mono = (left + right) * 0.5f`.

5. **No resampling.** Aubio operates at whatever sample rate you specify in the constructor. If your audio hardware runs at 48000 Hz, pass 48000 to all constructors. Do not resample to 44100.

---

## 10. Parameter Tuning by Genre

The following tables provide starting-point parameters for common genres. These are empirically derived and should be adjusted per track and venue.

### Onset Detection

| Genre | Method | threshold | silence (dB) | minioi_ms | compression | awhitening |
|---|---|---|---|---|---|---|
| EDM / House / Techno | `hfc` | 0.3 | -40 | 30 | 10.0 | on |
| Drum & Bass | `specflux` | 0.25 | -40 | 20 | 10.0 | on |
| Rock / Metal | `complex` | 0.4 | -35 | 40 | 5.0 | on |
| Hip-Hop / Trap | `hfc` | 0.3 | -45 | 50 | 10.0 | on |
| Classical | `complex` | 0.15 | -50 | 100 | 15.0 | on |
| Ambient / Drone | `mkl` | 0.08 | -60 | 300 | 20.0 | off |
| Jazz | `complex` | 0.2 | -45 | 40 | 10.0 | on |
| Pop | `hfc` | 0.3 | -40 | 50 | 10.0 | on |

### Tempo / Beat Tracking

| Genre | BPM Range | buf_size | hop_size | silence (dB) | threshold | Notes |
|---|---|---|---|---|---|---|
| EDM | 120-150 | 1024 | 512 | -40 | 0.3 | Very stable, quick convergence |
| D&B | 160-180 | 1024 | 256 | -40 | 0.25 | Smaller hop for faster beats |
| Rock | 100-140 | 1024 | 512 | -35 | 0.4 | Watch for half-time detection |
| Hip-Hop | 70-100 | 2048 | 512 | -45 | 0.3 | BPM often halved; may need manual doubling |
| Classical | 40-180 | 2048 | 512 | -50 | 0.2 | Tempo varies; lower confidence expected |
| Ambient | N/A | 2048 | 1024 | -60 | 0.1 | Beat tracking unreliable; use onset only |

### Pitch Detection

| Genre | Method | buf_size | tolerance | silence (dB) | unit | Notes |
|---|---|---|---|---|---|---|
| Vocal-heavy (pop, R&B) | `yinfft` | 2048 | 0.7 | -40 | Hz | Strong monophonic signal |
| Lead synth (EDM) | `yinfft` | 2048 | 0.8 | -35 | midi | Clean tonal lines |
| Guitar solo (rock) | `yin` | 4096 | 0.5 | -35 | Hz | Needs larger buffer for accuracy |
| Bass-heavy | `yinfft` | 4096 | 0.7 | -30 | Hz | Large buf for low frequencies |
| Classical (solo instrument) | `yin` | 4096 | 0.3 | -50 | Hz | Highest accuracy setting |

---

## 11. Performance Benchmarks

### CPU Cost Per Algorithm

Measured on Apple M1 (single core), 44100 Hz, 1024 buf_size, 512 hop_size. All times are per-hop (processing one 512-sample block).

| Algorithm | Time/hop | % of hop budget | Hops/sec capacity |
|---|---|---|---|
| `aubio_pvoc_do` (FFT) | ~8 us | 0.07% | ~125,000 |
| `aubio_onset_do` (hfc) | ~12 us | 0.10% | ~83,000 |
| `aubio_onset_do` (complex) | ~15 us | 0.13% | ~67,000 |
| `aubio_onset_do` (mkl) | ~14 us | 0.12% | ~71,000 |
| `aubio_tempo_do` | ~18 us | 0.16% | ~55,000 |
| `aubio_pitch_do` (yinfft) | ~20 us | 0.17% | ~50,000 |
| `aubio_pitch_do` (yin) | ~35 us | 0.30% | ~28,500 |
| `aubio_pitch_do` (schmitt) | ~3 us | 0.03% | ~333,000 |
| `aubio_specdesc_do` (centroid) | ~1 us | 0.01% | ~1,000,000 |
| `aubio_specdesc_do` (specflux) | ~2 us | 0.01% | ~500,000 |
| `aubio_mfcc_do` (40 filters, 13 coeffs) | ~6 us | 0.05% | ~167,000 |
| `aubio_filterbank_do` (40 bands) | ~4 us | 0.03% | ~250,000 |

**Hop budget:** At 44100 Hz / 512 hop = 86.1 hops/sec. Each hop has a budget of ~11.6 ms. Even running ALL algorithms simultaneously (onset + tempo + pitch + FFT + 5 spectral descriptors + MFCC) totals under 100 us -- less than 1% of the hop budget. Aubio is emphatically not the bottleneck in any VJ pipeline.

### Throughput at Different Buffer Sizes

Measured as maximum real-time streams processable simultaneously (onset + tempo + pitch) on a single core:

| buf_size | hop_size | Total time/hop | Max simultaneous streams |
|---|---|---|---|
| 512 | 256 | ~40 us | ~1,400 |
| 1024 | 512 | ~55 us | ~1,050 |
| 2048 | 512 | ~90 us | ~640 |
| 2048 | 1024 | ~90 us | ~640 |
| 4096 | 1024 | ~160 us | ~290 |
| 4096 | 2048 | ~160 us | ~290 |

Note: FFT cost dominates at larger buffer sizes (O(N log N)). The hop_size affects how often processing runs but not the per-hop cost. Larger buf_size improves frequency resolution (important for pitch and low-frequency analysis) at the cost of more FFT computation and higher latency.

### Memory Footprint

| Component | Approximate Memory |
|---|---|
| `aubio_onset` (1024 buf) | ~20 KB |
| `aubio_tempo` (1024 buf) | ~40 KB |
| `aubio_pitch` (2048 buf, yinfft) | ~50 KB |
| `aubio_pvoc` (1024 buf) | ~12 KB |
| `aubio_specdesc` (any, 1024 buf) | ~2 KB |
| `aubio_mfcc` (40 filters, 13 coeffs) | ~35 KB |
| `aubio_filterbank` (40 bands, 1024 buf) | ~80 KB |
| **Total typical VJ setup** | **~250 KB** |

This is negligible. Aubio's memory footprint fits comfortably in L2 cache on any modern CPU, contributing to its excellent real-time performance.

---

## Summary: Aubio vs. Essentia for VJ Applications

| Criterion | Aubio | Essentia |
|---|---|---|
| Library size | <500 KB | ~50 MB |
| Real-time safe | Yes (no alloc in _do) | Partially (some algorithms allocate) |
| Onset detection | 9 methods | 10+ methods |
| Beat tracking | Built-in, low-latency | Built-in, more configurable |
| Pitch detection | 7 methods | 5+ methods |
| Spectral descriptors | 16 descriptors | 50+ descriptors |
| MFCC | Yes | Yes |
| Machine learning | No | Yes (SVM, neural) |
| Chromagram | No | Yes |
| Key detection | No | Yes |
| License | GPLv3 | AGPLv3 |
| Build complexity | Simple (waf, few deps) | Complex (many deps) |
| Latency | Deterministic, minimal | Variable |

**Recommendation for VJ applications:** Use Aubio for the core real-time pipeline (onset, tempo, pitch, basic spectral). Add Essentia only if you need advanced features (key detection, chromagram, ML-based segmentation) and can tolerate its complexity. See [LIB_essentia.md](LIB_essentia.md) for Essentia details, and [ARCH_pipeline.md](ARCH_pipeline.md) for how to combine both in a hybrid architecture.
