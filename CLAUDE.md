# Audio-DNA — Project Instructions for Claude

---

## Project Identity

Audio-DNA is a cross-platform desktop application (C++20 / JUCE / OpenGL) that analyzes audio in real-time and applies a library of GLSL shader effects to a loaded image, driven by extracted audio features. It is a VJ-style performance tool where music controls visual transformations — the louder the bass, the more the image warps; the higher the spectral centroid, the more the hue shifts.

The core concept: two libraries (audio analysis + visual effects) connected by a mapping system, rendered live on a VJ panel. Users load a song and an image, wire audio features to effect parameters via mappings with curves and smoothing, and see the image react to the music in real-time at 60fps.

**What this is NOT**: Not a DAW, not a video editor, not a web app, not a plugin. It is a standalone desktop application for live audio-reactive image effects.

---

## Architecture Summary

### The 4-Thread Model

**Audio Callback (OS-managed, REAL-TIME priority)**
Runs every 2.67ms (128 samples @ 48kHz). Receives samples from JUCE's `AudioIODeviceCallback`, mono-downmixes them, and pushes into the SPSC ring buffer. This is the sacred thread — it must NEVER allocate heap memory, acquire mutexes, make system calls, or do any DSP. Just `memcpy` to ring buffer and return. Budget: <100μs. Communicates forward to the analysis thread via the SPSC ring buffer.

**Analysis Thread (App-managed, ABOVE-NORMAL priority)**
Runs every ~10.7ms (512-sample hop @ 48kHz). Pulls samples from the ring buffer, maintains a 2048-sample overlap window, runs FFT, and extracts all audio features in a fixed pipeline order. Pre-allocates all buffers and Aubio objects at startup — zero allocation in steady state. Budget: <2ms per hop (5x headroom). Publishes a complete `FeatureSnapshot` to the Feature Bus via atomic triple-buffer swap.

**Render Thread (OpenGL, NORMAL priority, VSync)**
Runs every 16.67ms (60fps). Reads the latest `FeatureSnapshot` from the triple buffer (lock-free atomic read). Runs all active mappings (source → curve → scale → target), uploads uniforms to GPU, and renders the effect chain on a fullscreen quad with the loaded image texture. Uses ping-pong FBOs for multi-effect chains. Budget: <8ms for full chain. Communicates display values back to UI via `juce::MessageManager::callAsync()`.

**Message Thread (JUCE UI, NORMAL priority)**
Runs on user events. Handles all UI interaction — sliders, buttons, file choosers, mapping editor. Writes configuration changes (effect enable/disable, parameter values, mapping settings) via `std::atomic<T>` config variables that the render and analysis threads read. Never blocks the other threads.

### Core Data Structures

**FeatureSnapshot** — The unit of transfer between Analysis and Render threads. Fixed-size POD, cache-line aligned (`alignas(64)`). Contains:

| Field | Type | Range | Purpose |
|-------|------|-------|---------|
| `timestamp` | `uint64_t` | sample clock | Timing reference |
| `wallClockSeconds` | `double` | seconds | Render interpolation |
| `rms` | `float` | [0, 1] | Root mean square amplitude |
| `peak` | `float` | [0, 1] | Peak amplitude |
| `rmsDB` | `float` | [-100, 0] dBFS | RMS in decibels |
| `lufs` | `float` | LUFS | Momentary loudness (ITU-R BS.1770) |
| `dynamicRange` | `float` | ratio | Crest factor |
| `transientDensity` | `float` | onsets/sec | Sliding window onset count |
| `spectralCentroid` | `float` | Hz | Brightness indicator |
| `spectralFlux` | `float` | normalized | Frame-to-frame spectral change |
| `spectralFlatness` | `float` | [0, 1] | Tonal vs noisy (Wiener entropy) |
| `spectralRolloff` | `float` | Hz | Frequency below 85% energy |
| `onsetDetected` | `bool` | flag | Transient this frame |
| `onsetStrength` | `float` | detection value | Onset detection function output |
| `bpm` | `float` | BPM | Current tempo estimate |
| `beatPhase` | `float` | [0, 1) | Sawtooth synced to beat |
| `structuralState` | `uint8_t` | 0-3 | 0=normal, 1=buildup, 2=drop, 3=breakdown |
| `bandEnergies[7]` | `float[7]` | normalized | Sub/Bass/LowMid/Mid/HighMid/Presence/Brilliance |
| `chromagram[12]` | `float[12]` | normalized | C through B pitch classes |
| `dominantPitch` | `float` | Hz | Detected fundamental frequency |
| `pitchConfidence` | `float` | [0, 1] | Pitch reliability |
| `detectedKey` | `int` | 0-11, -1 | Musical key (C=0), -1=unknown |
| `keyIsMajor` | `bool` | flag | Major vs minor |
| `mfccs[13]` | `float[13]` | coefficients | Timbral fingerprint |
| `harmonicChangeDetection` | `float` | HCDF value | Harmonic change rate |

**Mapping** — Routes any audio feature to any effect parameter:

| Field | Type | Purpose |
|-------|------|---------|
| `source` | `Source` enum | Which audio feature (RMS, BeatPhase, Bass, MFCC0, etc.) |
| `targetEffectId` | `uint32_t` | Which effect in the chain |
| `targetParamIndex` | `uint32_t` | Which parameter on that effect |
| `curve` | `Curve` enum | Linear, Exponential, Logarithmic, SCurve, Stepped |
| `inputMin/inputMax` | `float` | Source normalization range |
| `outputMin/outputMax` | `float` | Target output range (default [0, 1]) |
| `smoothing` | `float` | EMA alpha or One-Euro beta |
| `enabled` | `bool` | Active flag |

**Effect** — A named GLSL shader with typed parameters:

| Field | Type | Purpose |
|-------|------|---------|
| `name` | `string` | Display name ("Ripple", "Hue Shift") |
| `category` | `string` | "warp", "color", "glitch", "blur" |
| `shaderProgram` | `GLuint` | Compiled shader handle |
| `params` | `vector<EffectParam>` | Parameters with name, value, default (all [0, 1]) |
| `enabled` | `bool` | Active in chain |
| `order` | `int` | Position in effect chain |

### Lock-Free Communication Chain

```
Audio Callback ──SPSC Ring Buffer (16384 floats, ~341ms @ 48kHz)──▶ Analysis Thread
Analysis Thread ──Triple-Buffer Atomic Swap (3× FeatureSnapshot)──▶ Render Thread
UI Thread ──std::atomic<T> config variables──▶ Analysis/Render Threads
Render Thread ──juce::MessageManager::callAsync()──▶ UI Thread
```

All data flows forward. No backward dependencies on the hot path.

### Latency Budget

| Stage | Operation | Latency |
|-------|-----------|---------|
| 1. Audio buffer delivery | OS delivers 128 samples @ 48kHz | 2.67ms (period) |
| 2. Ring buffer push | `memcpy` into SPSC | ~50ns |
| 3. Hop accumulation | Wait for 512 samples (1 hop) | 10.7ms (hop period) |
| 4. Window + FFT | Hann window, 2048-pt FFT | ~20μs |
| 5. Feature extraction | All spectral + temporal features | ~100μs |
| 6. Feature bus publish | Atomic triple-buffer swap | ~10ns |
| 7. Render acquire | Atomic read of latest snapshot | ~10ns |
| 8. Mapping engine | Apply curves, smoothing | ~5μs |
| 9. Uniform upload | glUniform + UBO update | ~2μs |
| 10. Shader render | Effect chain on fullscreen quad | ~1-3ms |
| 11. Swap buffers | VSync present | 0-16.67ms |

**Total audio-to-visual latency: ~15-25ms** (well within the ±80ms perceptual sync window).

---

## Technology Stack

| Library | Version | License | What It Owns | Why Chosen Over Alternatives | Configured In |
|---------|---------|---------|-------------|---------------------------|---------------|
| **JUCE** | 7.0.12 | GPLv3 | Audio I/O, file playback (WAV/AIFF/FLAC/MP3/OGG), windowing, OpenGL context, UI widgets, message thread | Single framework for audio + UI + OpenGL. `AudioTransportSource` for file playback with background disk I/O. `AudioDeviceManager` for device enum/hot-plug across CoreAudio/WASAPI/ALSA/JACK. Alternatives: SDL2+ImGui (no audio file playback), Qt (poor RT audio). | `CMakeLists.txt` line 16-22, FetchContent |
| **Aubio** | 0.4.9+ | GPLv3 | BPM tracking (`aubio_tempo`), onset detection (`aubio_onset`), pitch detection (`aubio_pitch`) | Battle-tested beat/onset algorithms that beat custom implementations. Small C footprint. Alternative: Essentia (AGPL, massive dependency tree including FFTW/TagLib/yaml-cpp). | `CMakeLists.txt` (to be added in M2) |
| **juce::dsp::FFT** | (bundled) | GPLv3 | 2048-point FFT, magnitude spectrum (1025 bins) | Uses vDSP on macOS, IPP if available. No extra dependency. Adequate for 2048-pt. Alternatives: FFTW (GPL, overkill at 2048), KissFFT (slower). | JUCE module `juce_dsp` |
| **OpenGL 4.1 Core** | 4.1 | — | All image effects rendering via GLSL fragment shaders | macOS caps at 4.1 (Apple deprecated GL). Sufficient for 2D image effects on fullscreen quads. No compute shaders (require 4.3). Alternatives: Vulkan (overkill for 2D), Metal (macOS-only). | JUCE module `juce_opengl` |
| **GLSL 410** | 410 | — | All effect shaders, hot-reloadable from `shaders/` directory | Matches OpenGL 4.1 target. | `shaders/*.frag`, `shaders/*.vert` |
| **Catch2** | 3.x | BSL-1.0 | Unit/integration tests | Header-only, BDD-style, integrates with CMake/CTest. Test-only dependency. | `tests/CMakeLists.txt` (to be added in M2) |
| **stb_image** | latest | Public domain | Fallback image loading for formats JUCE doesn't handle | Single header. JUCE handles PNG/JPEG/GIF natively. | `third_party/` (optional) |
| **CMake** | 3.24+ | — | Build system | JUCE 7+ has first-class CMake support (`juce_add_gui_app`). Industry standard. Alternative: Projucer (deprecated). | `CMakeLists.txt` |

**Total runtime dependencies: 2 (JUCE, Aubio). Test-only: 1 (Catch2). Aubio's only transitive dependency is the C math library. JUCE bundles its own deps (freetype, zlib).**

---

## Source Tree

```
AudioDNA/
├── CLAUDE.md                            ← YOU ARE HERE
├── ARCHITECTURE.md                      # Full system architecture document
├── TASKPLAN.md                          # All milestones and tasks
├── CMakeLists.txt                       # Root build: JUCE via FetchContent, C++20
├── cmake/
│   ├── CompilerWarnings.cmake           # Per-compiler warning flags (-Wall -Wextra etc.)
│   └── FindAubio.cmake                  # [M2] Locate libaubio
├── src/
│   ├── Main.cpp                         # JUCE app entry point (JUCEApplication subclass)
│   ├── MainComponent.h/cpp              # Top-level component, owns all systems, layout
│   ├── audio/
│   │   ├── AudioEngine.h/cpp         ✅ # AudioDeviceManager + AudioTransportSource + file loading
│   │   ├── AudioCallback.h/cpp       ✅ # RT callback → mono downmix → ring buffer push
│   │   └── RingBuffer.h              ✅ # Lock-free SPSC, power-of-two, cache-line padded
│   ├── analysis/
│   │   ├── AnalysisThread.h/cpp      ✅ # Dedicated thread, reads ring buffer, runs feature pipeline
│   │   ├── FeatureSnapshot.h         ✅ # POD struct, all analysis fields, alignas(64)
│   │   ├── FFTProcessor.h/cpp        ✅ # juce::dsp::FFT wrapper, 2048-pt, Hann window
│   │   ├── SpectralFeatures.h/cpp    ✅ # Centroid, flux, flatness, rolloff, 7-band energies
│   │   ├── OnsetDetector.h/cpp       ✅ # Aubio onset wrapper
│   │   ├── BPMTracker.h/cpp          ✅ # Aubio tempo wrapper
│   │   ├── MFCCExtractor.h/cpp       ✅ # Mel filterbank (40 bands, 20-8kHz) + DCT → 13 coefficients
│   │   ├── ChromaExtractor.h/cpp     ✅ # FFT bins → 12 pitch classes, HCDF
│   │   ├── KeyDetector.h/cpp         ✅ # Krumhansl-Schmuckler: chroma × 24 key templates
│   │   ├── LoudnessAnalyzer.h/cpp    ✅ # K-weighting biquads + 400ms window → LUFS
│   │   ├── StructuralDetector.h/cpp  ✅ # Multi-scale EMA envelopes → state machine
│   │   └── PitchTracker.h/cpp        ✅ # Aubio yinfft pitch detection
│   ├── features/
│   │   ├── FeatureBus.h/cpp          ✅ # Triple-buffer atomic swap (3× FeatureSnapshot)
│   │   └── Smoother.h               ✅ # EMA + One-Euro filter (header-only)
│   ├── mapping/
│   │   ├── MappingEngine.h/cpp          # [M4] Source→curve→scale→target routing
│   │   ├── MappingTypes.h               # [M4] Mapping, Source, Curve enums
│   │   └── CurveTransforms.h            # [M4] lin/exp/log/sigmoid/step pure functions
│   ├── effects/
│   │   ├── EffectLibrary.h/cpp          # [M4] Registry: creates Effect instances from shaders
│   │   ├── Effect.h/cpp                 # [M3] Single effect: shader program + param list
│   │   ├── EffectChain.h/cpp            # [M3] Ordered chain with ping-pong FBOs
│   │   └── UniformBridge.h/cpp          # [M3] Maps effect params → glUniform calls
│   ├── render/
│   │   ├── Renderer.h/cpp               # [M3] OpenGLRenderer impl, frame loop
│   │   ├── ShaderManager.h/cpp          # [M3] Compile, link, hot-reload from shaders/
│   │   ├── TextureManager.h/cpp         # [M3] Image → GL_TEXTURE_2D, FBO textures
│   │   └── FullscreenQuad.h/cpp         # [M3] VAO/VBO for fullscreen triangle strip
│   └── ui/
│       ├── LookAndFeel.h/cpp         ✅ # Dark VJ-style theme
│       ├── WaveformDisplay.h/cpp     ✅ # Scrolling time-domain waveform
│       ├── AudioReadoutPanel.h/cpp   ✅ # Left panel: all feature values + meters
│       ├── SpectrumDisplay.h/cpp     ✅ # 7-band color-coded spectrum bars
│       ├── PreviewPanel.h/cpp           # [M3] Center: hosts OpenGL context
│       ├── EffectsRackPanel.h/cpp       # [M4] Right: effect list + knobs + mapping indicators
│       ├── MappingEditor.h/cpp          # [M4] Source/curve/range/smoothing configuration
│       ├── SpectrumDisplay.h/cpp        # [M2] Bar/line spectrum analyzer
│       ├── Knob.h/cpp                   # [M5] Rotary knob with mapping indicator ring
│       └── PresetManager.h/cpp          # [M5] JSON save/load of effects + mappings
├── shaders/                             # [M3+] All GLSL fragment shaders
│   ├── passthrough.vert                 # Shared vertex shader (fullscreen quad UVs)
│   ├── ripple.frag                      # Warp: UV distortion via sin(dist * freq + time)
│   ├── bulge.frag                       # Warp: radial magnification
│   ├── wave.frag                        # Warp: directional sine wave
│   ├── liquid.frag                      # Warp: turbulent displacement
│   ├── hue_shift.frag                   # Color: RGB→HSV rotate H HSV→RGB
│   ├── saturation.frag                  # Color: saturation multiply
│   ├── brightness.frag                  # Color: brightness offset
│   ├── duotone.frag                     # Color: two-color remap
│   ├── chromatic_aberration.frag        # Color: RGB channel UV offset
│   ├── pixel_scatter.frag               # Glitch: random pixel displacement
│   ├── rgb_split.frag                   # Glitch: directional RGB offset
│   ├── block_glitch.frag               # Glitch: rectangular block displacement
│   ├── scanlines.frag                   # Glitch: horizontal line overlay
│   ├── gaussian_blur.frag               # Blur: Gaussian kernel
│   ├── zoom_blur.frag                   # Blur: radial blur from center
│   ├── shake.frag                       # Blur: image translation offset
│   └── vignette.frag                    # Blur: edge darkening
├── tests/                               # [M2+]
│   ├── CMakeLists.txt
│   ├── test_ring_buffer.cpp
│   ├── test_spectral_features.cpp
│   ├── test_onset_detector.cpp
│   ├── test_mapping_engine.cpp
│   ├── test_smoother.cpp
│   └── test_feature_bus.cpp
├── resources/
│   ├── default_image.png                # Fallback test image
│   └── presets/
│       └── default_mappings.json        # Default mapping preset
└── research/                            # 30 research documents (read-only reference)
    ├── INDEX.md                         # Research document index
    ├── ARCH_*.md                        # Architecture deep-dives (pipeline, audio I/O, RT constraints)
    ├── FEATURES_*.md                    # Audio feature algorithms (spectral, rhythm, pitch, etc.)
    ├── LIB_*.md                         # Library evaluations (JUCE, Aubio, Essentia, FFT, etc.)
    ├── VIDEO_*.md                       # OpenGL integration, VJ frameworks, visual mapping
    ├── IMPL_*.md                        # Project setup, testing, calibration, prototype
    └── REF_*.md                         # Math reference, latency numbers, genre presets
```

### Naming Conventions

- **Files**: `PascalCase.h/cpp` for classes, `snake_case.frag/vert` for shaders
- **Classes**: `PascalCase` — `AnalysisThread`, `FeatureBus`, `MappingEngine`
- **Methods**: `camelCase` — `processBlock()`, `publishSnapshot()`, `applyMapping()`
- **Shader uniforms**: `u_[featureName]` — `u_rms`, `u_beatPhase`, `u_spectralCentroid`, `u_ripple_intensity`
- **Global uniforms**: `u_time`, `u_resolution`
- **Effect-specific uniforms**: `u_[effectName]_[paramName]` — `u_ripple_freq`, `u_hue_shift`

---

## Audio Analysis Features

All features are computed per hop (512 samples = 10.7ms @ 48kHz) in the analysis thread.

### Amplitude & Dynamics

| Feature | Algorithm | Output Range | FeatureSnapshot Field | Update Rate |
|---------|-----------|-------------|----------------------|-------------|
| RMS | Root mean square of 2048-sample window | [0, 1] | `rms` | Per hop |
| Peak | Max absolute sample value | [0, 1] | `peak` | Per hop |
| RMS dB | 20 * log10(rms) | [-100, 0] dBFS | `rmsDB` | Per hop |
| LUFS | K-weighted RMS, 400ms window (ITU-R BS.1770) | LUFS scale | `lufs` | Per hop |
| Dynamic Range | Crest factor (peak/RMS) | ratio | `dynamicRange` | Per hop |
| Transient Density | Onset count in 2-second sliding window | onsets/sec | `transientDensity` | Per hop |

### Spectral (from 2048-pt FFT → 1025 magnitude bins)

| Feature | Algorithm | Output Range | FeatureSnapshot Field |
|---------|-----------|-------------|----------------------|
| Spectral Centroid | Weighted average frequency | Hz | `spectralCentroid` |
| Spectral Flux | Half-wave rectified frame-to-frame magnitude diff | Normalized per-session | `spectralFlux` |
| Spectral Flatness | Geometric mean / arithmetic mean of magnitudes | [0, 1] | `spectralFlatness` |
| Spectral Rolloff | Frequency below 85% of total energy | Hz | `spectralRolloff` |
| 7-Band Energies | Sum magnitudes per band (Sub 20-60, Bass 60-250, LowMid 250-500, Mid 500-2k, HighMid 2k-4k, Presence 4k-6k, Brilliance 6k-20k Hz) | Normalized | `bandEnergies[0..6]` |

### Rhythm & Onset

| Feature | Source Library | Output | FeatureSnapshot Field |
|---------|---------------|--------|----------------------|
| Onset Detection | Aubio `aubio_onset` (spectral flux method, adaptive threshold) | bool flag + strength | `onsetDetected`, `onsetStrength` |
| BPM | Aubio `aubio_tempo` (autocorrelation of onset accumulator) | BPM float | `bpm` |
| Beat Phase | Derived from BPM tracker | [0, 1) sawtooth | `beatPhase` |

### Pitch & Harmony

| Feature | Algorithm | Output | FeatureSnapshot Field |
|---------|-----------|--------|----------------------|
| Chroma | FFT magnitude bins → 12 pitch classes (C–B), sum=1 | float[12] | `chromagram[0..11]` |
| Dominant Pitch | YIN or `aubio_pitch` on time-domain signal | Hz | `dominantPitch` |
| Pitch Confidence | YIN confidence measure | [0, 1] | `pitchConfidence` |
| Key Detection | Krumhansl-Schmuckler: chroma × 24 key templates | key + mode | `detectedKey`, `keyIsMajor` |
| MFCC | Mel filterbank (40 bands, 20-8kHz) → log → DCT-II → 13 coefficients | float[13] | `mfccs[0..12]` |
| HCDF | Euclidean distance between consecutive chroma frames | float | `harmonicChangeDetection` |

### Structural

| Feature | Algorithm | Output | FeatureSnapshot Field |
|---------|-----------|--------|----------------------|
| Structural State | Multi-scale EMA envelopes (100ms/1s/4s/16s), short vs long comparison → state machine | 0=normal, 1=buildup, 2=drop, 3=breakdown | `structuralState` |

### Analysis Pipeline Order (each step depends on prior results)

```
1. Raw time-domain: RMS, peak, ZCR
2. FFT → magnitude spectrum (2048-pt, Hann window)
3. From magnitude: centroid, flux, flatness, rolloff, band energies
4. Onset detection: spectral flux thresholding (adaptive median + offset)
5. BPM tracking: onset accumulator → autocorrelation → tempo + beat phase
6. MFCC: mel filterbank → log → DCT → 13 coefficients
7. Chroma: magnitude bins → 12 pitch classes
8. Pitch: YIN or aubio_pitch on time-domain signal
9. Key detection: chroma profile → Krumhansl-Schmuckler correlation
10. LUFS: K-weighted RMS over 400ms window
11. Structural: multi-scale EMA → buildup/drop/breakdown state machine
12. HCDF: chroma difference function
13. Transient density: onset count in sliding window
```

---

## Effects Library

17 effects across 4 categories. All parameters normalized to [0.0, 1.0] — the shader maps to internal ranges.

### Warp Effects

| Effect | Shader File | Parameters | Typical Audio Mapping |
|--------|-------------|-----------|----------------------|
| Ripple | `ripple.frag` | intensity (`u_ripple_intensity`), frequency (`u_ripple_freq`), speed (`u_ripple_speed`) | RMS → intensity, Bass → frequency |
| Bulge | `bulge.frag` | amount (`u_bulge_amount`), center_x/y (`u_bulge_center`) | OnsetStrength → amount |
| Wave | `wave.frag` | amplitude (`u_wave_amp`), frequency (`u_wave_freq`), direction | BeatPhase → amplitude |
| Liquid | `liquid.frag` | viscosity (`u_liquid_visc`), turbulence (`u_liquid_turb`) | SpectralFlux → turbulence |

### Color Effects

| Effect | Shader File | Parameters | Typical Audio Mapping |
|--------|-------------|-----------|----------------------|
| Hue Shift | `hue_shift.frag` | amount (`u_hue_shift`) | SpectralCentroid → amount |
| Saturation | `saturation.frag` | amount (`u_saturation`) | SpectralFlatness → amount |
| Brightness | `brightness.frag` | amount (`u_brightness`) | RMS → amount |
| Duotone | `duotone.frag` | color1 (`u_duotone_a`), color2 (`u_duotone_b`), mix | StructuralState → mix |
| Chromatic Aberration | `chromatic_aberration.frag` | amount (`u_chroma_amount`), angle (`u_chroma_angle`) | OnsetStrength → amount |

### Glitch Effects

| Effect | Shader File | Parameters | Typical Audio Mapping |
|--------|-------------|-----------|----------------------|
| Pixel Scatter | `pixel_scatter.frag` | amount (`u_scatter_amount`), seed | SpectralFlux → amount |
| RGB Split | `rgb_split.frag` | amount (`u_rgb_split`), angle | OnsetStrength → amount |
| Block Glitch | `block_glitch.frag` | intensity (`u_block_glitch_int`), block_size | TransientDensity → intensity |
| Scanlines | `scanlines.frag` | intensity (`u_scanline_int`), frequency (`u_scanline_freq`) | BeatPhase → intensity |

### Blur/Post Effects

| Effect | Shader File | Parameters | Typical Audio Mapping |
|--------|-------------|-----------|----------------------|
| Gaussian Blur | `gaussian_blur.frag` | radius (`u_blur_radius`) | inverse RMS → radius |
| Zoom Blur | `zoom_blur.frag` | amount (`u_zoom_blur`), center_x/y | OnsetStrength → amount |
| Shake | `shake.frag` | amount_x/y (`u_shake`) | OnsetDetected → trigger |
| Vignette | `vignette.frag` | intensity (`u_vignette_int`), softness (`u_vignette_soft`) | Bass → intensity |

### Effect Chain Architecture

Input image → FBO A (Effect 1) → FBO B (Effect 2) → FBO A (Effect 3) → ... → Screen. Ping-pong between two FBOs. Each effect reads from the previous FBO's texture and writes to the other.

---

## The Mapping System

### How Mappings Work

```
Audio Feature (source) → Normalize to [0,1] → Apply Curve → Scale to output range → Smooth → Effect Parameter (target)
```

1. **Extract**: Read source value from `FeatureSnapshot` (e.g., `snapshot.rms`)
2. **Normalize**: `(raw - inputMin) / (inputMax - inputMin)` → [0, 1]
3. **Curve**: Apply transform function:
   - **Linear**: `y = x`
   - **Exponential**: `y = x^2.0` (emphasizes peaks)
   - **Logarithmic**: `y = log(1 + x * 9) / log(10)` (compresses peaks, lifts lows)
   - **S-Curve**: `y = x² * (3 - 2x)` (smoothstep, de-emphasizes extremes)
   - **Stepped**: `y = floor(x * N) / N` (quantized to N steps)
4. **Scale**: `outputMin + curved * (outputMax - outputMin)`
5. **Smooth**: EMA or One-Euro filter with per-mapping state
6. **Write**: Set on target effect's parameter slot

### Render Thread Consumption

Each frame, the render thread:
1. Acquires latest `FeatureSnapshot` from triple buffer (atomic read, ~10ns)
2. Iterates all active `Mapping` objects, running the pipeline above
3. Writes computed values to each target `Effect`'s parameter slots
4. `UniformBridge` uploads all effect parameters as `glUniform1f` calls
5. Renders the effect chain (ping-pong FBOs)

### User Creates/Edits/Saves Mappings

- **Create**: Click "▼map" on any effect parameter → opens `MappingEditor`
- **Edit**: Select source feature dropdown, curve type dropdown, adjust input/output range sliders, smoothing knob
- **Save**: `PresetManager` serializes all effects + mappings to JSON
- Multiple mappings can target the same parameter (values are summed)

---

## Milestone Status

| # | Milestone | Tasks | Status |
|---|-----------|-------|--------|
| **M1** | Window + Audio + Waveform | 10 tasks | **COMPLETE** |
| M2 | Full Audio Analysis Engine | 19 tasks | **CURRENT** |
| M3 | OpenGL Image Rendering + First Effects | 10 tasks | Not started |
| M4 | Mapping Engine + Full Effects Library | 9 tasks | Not started |
| M5 | VJ-Style UI Polish + Presets | 11 tasks | Not started |
| M6 | Quality, Performance, Cross-Platform | 8 tasks | Not started |

### Milestone 1: COMPLETE

All 10 tasks done. App builds, loads/plays audio, waveform + RMS/Peak meters work, transport controls functional.

### Current Milestone: M2 — Full Audio Analysis Engine

**Goal**: Extract all audio features in real-time. Display all values as live readouts.

**Completed**: 2.1–2.17 — All analysis + UI tasks complete. Full 12-stage pipeline: FFT, spectral features, onset detection, BPM/beat tracking, MFCC, chroma, key detection, pitch tracking, LUFS, structural detection, transient density, HCDF. Expanded AudioReadoutPanel + SpectrumDisplay show all features live. Smoother class (EMA + One-Euro) implemented.

**Next task**: 2.18 — Write unit tests.

### What M2 Unlocks

Full audio analysis: FFT, all spectral features, band energies, onset detection, BPM/beat tracking, MFCC, chroma, pitch, key, LUFS, structural detection — all displayed as live readouts.

---

## Build Instructions

### Prerequisites (all platforms)

- CMake 3.24+
- C++20-capable compiler
- Git (for FetchContent to download JUCE)

### macOS (primary development platform)

```bash
git clone <repo-url> AudioDNA && cd AudioDNA
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j$(sysctl -n hw.ncpu)
./build/AudioDNA_artefacts/Release/Audio-DNA.app/Contents/MacOS/Audio-DNA
```

Required: Xcode Command Line Tools (`xcode-select --install`). No other dependencies — JUCE is fetched automatically.

### Windows

```bash
cmake -B build -G "Visual Studio 17 2022"
cmake --build build --config Release
build\AudioDNA_artefacts\Release\Audio-DNA.exe
```

Required: Visual Studio 2022 with C++ workload.

### Linux

```bash
# Ubuntu/Debian — install JUCE dependencies
sudo apt install libasound2-dev libcurl4-openssl-dev libfreetype6-dev \
  libx11-dev libxcomposite-dev libxcursor-dev libxinerama-dev libxrandr-dev \
  libxrender-dev libwebkit2gtk-4.0-dev libglu1-mesa-dev mesa-common-dev

cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j$(nproc)
./build/AudioDNA_artefacts/Release/Audio-DNA
```

### Common Build Issues

| Issue | Fix |
|-------|-----|
| `FetchContent` download fails | Check internet connection; JUCE repo is ~200MB |
| macOS: "OpenGL deprecated" warnings | Expected — Apple deprecated GL but 4.1 still works. Suppress with `-Wno-deprecated` |
| Linux: missing X11/ALSA headers | Install the `apt` packages listed above |
| Windows: long path errors | Enable long paths: `git config --system core.longpaths true` |

### Adding Aubio (Milestone 2)

Aubio will be added via system install or FetchContent. On macOS: `brew install aubio`. On Linux: `sudo apt install libaubio-dev`. The `FindAubio.cmake` module will locate it.

---

## Development Rules

### Sacred Rules (violating these causes real-time audio failures)

1. **The audio callback is sacred**: No heap allocation (`new`, `malloc`, `vector::push_back`), no mutexes (`std::mutex`, `std::lock_guard`), no system calls (`printf`, file I/O, `std::cout`), no exceptions. It copies samples to the ring buffer and returns. Budget: <100μs.

2. **All inter-thread data goes through established lock-free channels**: Audio→Analysis via SPSC ring buffer. Analysis→Render via triple-buffer FeatureBus. UI→hot path via `std::atomic<T>`. Never add a new mutex without explicit discussion.

3. **Analysis thread pre-allocates everything**: All FFT plans, work buffers, Aubio objects, filter states created at startup. Zero allocation in the steady-state loop.

4. **Render thread never waits for analysis**: If no new snapshot is available, reuse the previous one. The render loop runs at VSync regardless of analysis rate.

### Shader Rules

5. **Every new effect is a GLSL file in `/shaders`**: Never hardcode effect logic in C++. One `.frag` file per effect. Shaders are hot-reloadable from disk.

6. **All effect parameters are [0, 1]**: The mapping engine and UI work in normalized space. The shader maps `[0, 1]` to its internal range.

7. **Shader uniforms follow naming convention**: `u_[effectName]_[paramName]` for effect-specific, `u_time` and `u_resolution` for globals. Uniform names must match FeatureSnapshot field names when directly mapped (e.g., `u_rms`, `u_beatPhase`).

### Feature Addition Rules

8. **Every new audio feature must be added to FeatureSnapshot before use anywhere**: The snapshot is the single source of truth for analysis→render data transfer.

9. **When adding a new effect**: (1) Write the GLSL shader in `/shaders`, (2) Register in `EffectLibrary`, (3) Add parameters to the mapping UI — in that order, never out of order.

10. **When adding a new audio feature**: (1) Add field to `FeatureSnapshot`, (2) Compute in `AnalysisThread` pipeline, (3) Expose in the UI readout panel — in that order.

### Code Quality Rules

11. **OpenGL 4.1 minimum**: macOS constraint. No compute shaders (require 4.3). All effects are fragment shaders on fullscreen quads.

12. **Keep the audio thread under 100μs**: If an algorithm exceeds this, move it to the analysis thread.

13. **Prefer extending existing systems over adding new ones**: The architecture has clear boundaries — work within them.

14. **When this document says something, it overrides any default behavior**: If CLAUDE.md and a research doc disagree, CLAUDE.md wins (research docs are pre-decision references).

---

## Claude Working Instructions

### "Kick off the next task [X.Y]" Protocol

When the user says **"kick off the next task [X.Y]"**, follow this exact sequence:

1. **Read** CLAUDE.md (this file) and TASKPLAN.md to find task [X.Y]
2. **Read** all source files relevant to the task before changing anything
3. **Execute** the task — write code, fix bugs, configure builds, whatever the task requires
4. **Self-validate with two independent methods** before asking the human:
   - **Build tasks**: (A) `cmake --build` exits 0 with zero errors, (B) built binary exists and `file` confirms valid executable
   - **Implementation tasks**: (A) full project compiles after changes, (B) grep source for rule violations (no `new`/`malloc` in audio callback, no `std::mutex` on hot path, POD snapshot, `u_` uniform names)
   - **Shader tasks**: (A) project builds and shader loads, (B) uniforms match EffectLibrary registration with `u_` prefix
   - **UI tasks**: (A) project compiles with component integrated, (B) class follows JUCE patterns (inherits Component, implements paint()/resized(), timer/async updates)
   - **Test tasks**: (A) all tests pass, (B) every public method of class under test has at least one test case
5. **Report** results to human using this format:
   ```
   ## Task [X.Y] Validation Report
   **Task**: <description>
   **Files**: <list>
   ### Validation A: <result PASS/FAIL + evidence>
   ### Validation B: <result PASS/FAIL + evidence>
   ### For you to check: <what human should look for>
   ```
6. **Wait** for human to say it passes
7. **On human PASS**: commit to git, update CLAUDE.md milestone status, append to BUILDLOG.md, then output:
   ```
   Ready for next task. Say: kick off the next task [X.Y+1]
   ```
8. **On human FAIL**: fix the issue, re-run both validations, report again

### Before Any Work

- Always read this CLAUDE.md before touching any file
- Check TASKPLAN.md to understand which milestone you're in and what tasks remain
- Read existing source files before modifying them

### Debugging Audio Issues

1. Check the SPSC ring buffer fill level first — if it's consistently full or empty, the producer/consumer balance is wrong
2. Check sample rate assumptions — the system assumes 48kHz; mismatches cause pitch/timing errors
3. Check thread priority — if analysis can't keep up, features lag behind audio

### Debugging Visual Issues

1. Check shader uniform names match FeatureSnapshot field names exactly
2. Check that the effect is registered in EffectLibrary and enabled in the chain
3. Check FBO ping-pong: if effects look wrong when chained, the read/write FBOs may be swapped
4. Use shader hot-reload to iterate without restarting the app

### Threading Deep-Dives

Before refactoring any threading code, read these research documents first:
- `research/ARCH_realtime_constraints.md` — golden rules of RT audio
- `research/ARCH_pipeline.md` — lock-free communication chain details

### Adding Dependencies

- Check `CMakeLists.txt` before adding any dependency
- Prefer JUCE built-in functionality over new libraries
- Any new runtime dependency must be justified against the "Why not X" column in the tech stack table above

### Updating This Document

When you add a new feature, effect, or audio analysis capability, update the relevant section of this CLAUDE.md to reflect it. This document must always be the current truth.

---

## Research Documents Reference

The `research/` directory contains 30 documents organized by prefix:

| Prefix | Topic | Key Documents |
|--------|-------|---------------|
| `ARCH_` | Architecture deep-dives | `pipeline.md` (lock-free chain), `audio_io.md` (platform APIs), `realtime_constraints.md` (RT rules) |
| `FEATURES_` | Audio feature algorithms | `spectral.md` (14 features), `rhythm_tempo.md` (onset/BPM), `pitch_harmonic.md` (YIN, chroma, key), `mfcc_mel.md`, `amplitude_dynamics.md`, `frequency_bands.md`, `transients_texture.md`, `structural.md`, `psychoacoustic.md` |
| `LIB_` | Library evaluations | `juce.md`, `aubio.md`, `essentia.md` (rejected), `fft_comparison.md`, `rtaudio_miniaudio.md`, `rust_ecosystem.md` (rejected) |
| `VIDEO_` | Visual rendering | `opengl_integration.md` (UBOs, FBOs, GLSL patterns), `feature_to_visual_mapping.md` (mapping theory), `vj_frameworks.md` (framework comparison) |
| `IMPL_` | Implementation guides | `project_setup.md` (CMake/CI), `minimal_prototype.md` (380-line prototype), `testing_validation.md` (Catch2, test signals), `calibration_adaptation.md` (auto-tuning) |
| `REF_` | Reference material | `math_reference.md` (DFT, biquads, window functions), `latency_numbers.md` (per-stage budgets), `genre_parameter_presets.md` (8 genre profiles), `resources_links.md` (papers, datasets) |

These are read-only reference material. All decisions have been made and are reflected in ARCHITECTURE.md and this CLAUDE.md.
