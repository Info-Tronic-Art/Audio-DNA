# Audio-DNA — Project Instructions for Claude

---

## Project Identity

Audio-DNA is a cross-platform desktop application (C++20 / JUCE / OpenGL) for live audio-reactive visual performance. It analyzes audio in real-time (mic, system audio, or audio file) and applies 110 GLSL shader effects to images, driven by extracted audio features. It is a VJ-style performance tool where music controls visual transformations.

The core concept: audio analysis + visual effects + a mapping system + a keyboard clip launcher, rendered live at 60fps. Users load images (or folders for beat-synced slideshows), wire audio features to effect parameters via mappings with curves and smoothing, and perform live with keyboard-triggered visual scenes.

**Key capabilities**: 110 effects across 9 categories, 15 clip-to-clip transitions, deck/layer/clip compositing with per-level effect chains, fullscreen output to any connected display, beat-synced randomization, instant preset save/recall, camera input, video playback, 22 procedural sources, VJ panel UI.

**What this is NOT**: Not a DAW, not a video editor, not a web app, not a plugin. It is a standalone desktop application for live audio-reactive visual performance.

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
| `barCount` | `uint16_t` | bars since reset | Bars since last phrase reset |
| `phrasePhase` | `float` | [0, 1) | Sawtooth over N bars (configurable, default 8) |

**Mapping** — Routes any audio feature to any effect parameter:

| Field | Type | Purpose |
|-------|------|---------|
| `source` | `Source` enum | Which audio feature (RMS, BeatPhase, BarPhase, PhrasePhase, BarCount, Bass, MFCC0, etc.) |
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

**Clip** — Media content + per-clip effects + transport, placed in a deck cell:

| Field | Type | Purpose |
|-------|------|---------|
| `mediaType` | `MediaType` enum | None, Image, Video, Camera, Source, ImageSequence |
| `inPoint` | `float` | [0,1] playback start position (draggable on timeline) |
| `outPoint` | `float` | [0,1] playback end position (draggable on timeline) |
| `speed` | `float` | Playback speed multiplier |
| `transportMode` | `TransportMode` enum | Timeline or BPMSync |
| `loopMode` | `LoopMode` enum | Loop, PingPong, OneShot |
| `beatDivision` | `float` | BPM Sync: beats per playback cycle |
| `videoBeats` | `float` | Content beats (for BPM speed calc) |
| `beatSnap` | `bool` | Snap playhead to beat on trigger |
| `cuepoints[8]` | `float[8]` | Normalized positions [0,1], up to 8 |
| `playheadPosition` | `mutable double` | [0,1] runtime position (synced from player each frame) |

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

| **FFmpeg** | 8.0 | LGPL/GPL | Video decode: MP4, MOV, QuickTime, AVI, MKV, WebM, M4V, HAP Alpha (libavformat, libavcodec, libavutil, libswscale) | Industry standard video decode. Supports all major codecs including H.264, H.265, ProRes, HAP Alpha. Alternative: GStreamer (heavier, less portable). | `cmake/FindFFmpeg.cmake`, `CMakeLists.txt` |

**Total runtime dependencies: 3 (JUCE, Aubio, FFmpeg). Test-only: 1 (Catch2). Aubio's only transitive dependency is the C math library. JUCE bundles its own deps (freetype, zlib). FFmpeg is located via Homebrew on macOS.**

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
│   ├── FindAubio.cmake                  # [M2] Locate libaubio
│   └── FindFFmpeg.cmake              ✅ # [P11] Locate FFmpeg (libavformat/libavcodec/libavutil/libswscale)
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
│   ├── keyboard/
│   │   └── KeySlot.h                    # [M7] Per-key data model (media, effects, transparency, latch/random)
│   ├── media/
│   │   ├── VideoPlayer.h/cpp         ✅ # [P11] FFmpeg video decode (MP4/MOV/AVI/MKV/HAP Alpha) → GL texture
│   │   └── ImageSequence.h/cpp       ✅ # [P11] Multi-image playback as video clip with configurable FPS
│   ├── recording/
│   │   └── SessionRecorder.h/cpp     ✅ # [P12] Timestamped event recording/playback for performance capture
│   ├── effects/
│   │   ├── EffectLibrary.h/cpp          # [M4] Registry: creates Effect instances from shaders
│   │   ├── Effect.h/cpp              ✅ # Single effect: shader program + param list
│   │   ├── EffectChain.h/cpp         ✅ # Ordered chain with ping-pong FBOs
│   │   └── UniformBridge.h/cpp       ✅ # Maps effect params → glUniform calls
│   ├── render/
│   │   ├── Renderer.h/cpp            ✅ # OpenGLRenderer impl, GL 4.1 core, frame loop
│   │   ├── ShaderManager.h/cpp       ✅ # Compile, link, hot-reload from shaders/
│   │   ├── TextureManager.h/cpp      ✅ # Image → GL_TEXTURE_2D, FBO textures
│   │   ├── FullscreenQuad.h/cpp      ✅ # VAO/VBO for fullscreen triangle strip
│   │   ├── EmbeddedShaders.h         ✅ # All 96 effect + 15 transition GLSL shaders as inline strings
│   │   └── CompositorEngine.h/cpp       # [M7] Multi-layer key compositing pipeline
│   └── ui/
│       ├── LookAndFeel.h/cpp         ✅ # Dark VJ-style theme
│       ├── WaveformDisplay.h/cpp     ✅ # Scrolling time-domain waveform
│       ├── AudioReadoutPanel.h/cpp   ✅ # Left panel: all feature values + meters
│       ├── SpectrumDisplay.h/cpp     ✅ # 7-band color-coded spectrum bars
│       ├── PreviewPanel.h/cpp        ✅ # Center: hosts OpenGL context
│       ├── EffectsRackPanel.h/cpp       # [M4] Right: effect list + knobs + mapping indicators
│       ├── MappingEditor.h/cpp          # [M4] Source/curve/range/smoothing configuration
│       ├── SpectrumDisplay.h/cpp        # [M2] Bar/line spectrum analyzer
│       ├── Knob.h/cpp                   # [M5] Rotary knob with mapping indicator ring
│       ├── PresetManager.h/cpp       ✅ # JSON save/load of effects + mappings + deck
│       ├── OutputWindow.h/cpp        ✅ # Fullscreen output on any display
│       ├── Knob.h/cpp                ✅ # Rotary knob with mapping indicator ring
│       ├── KeyboardPanel.h/cpp          # [M7] 40-key visual keyboard grid
│       └── KeyEditor.h/cpp              # [M7] Per-key media/effects/transparency editor
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
│   ├── test_ring_buffer.cpp          ✅
│   ├── test_spectral_features.cpp    ✅
│   ├── test_feature_bus.cpp          ✅
│   ├── test_smoother.cpp             ✅
│   ├── test_integration_pipeline.cpp ✅
│   ├── test_onset_detector.cpp
│   └── test_mapping_engine.cpp  ✅
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

### UI Text Rules

- **Always display whole words** in the UI — never use abbreviations. For example, "Inverted Luma is Alpha" not "Inv. Luma is Alpha", "Ignore Random" not "Ign. Rnd".
- Labels, button text, dropdown items, and tooltips must all use complete words.

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
| Bar Phase | (beatInBar + beatPhase) / 4 | [0, 1) over 4 beats | `barPhase` |
| Phrase Phase | Bar count mod N bars (default 8), resets on structural transitions | [0, 1) over N bars | `phrasePhase` |
| Bar Count | Bars since last phrase reset | uint16 | `barCount` |

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
14. Phrase tracking: bar count + phrase phase sawtooth over N bars (resets on structural transitions)
```

---

## Effects Library

110 effects across 9 categories + 15 transition shaders. All parameters normalized to [0.0, 1.0] — the shader maps to internal ranges. All shaders are embedded in `src/render/EmbeddedShaders.h`.

### Effect Categories (110 total)

| Category | Count | Examples |
|----------|-------|---------|
| **3D / Depth** | 7 | Perspective Tilt, Cylinder Wrap, Sphere Wrap, Tunnel, Page Curl, Parallax Layers, Dot Field |
| **Warp** | 26 | Ripple, Bulge, Wave, Liquid, Kaleidoscope, Fisheye, Swirl, Polar Coords, Twirl, Shear, Elastic Bounce, Ripple Pond, Diamond Distort, Barrel Distort, Sine Grid, Glitch Displace, Quad Mirror, Flip, Warp Field, Slide Wrap, Tile Grid, Spot Zoom, Bendoscope, UV Remap, Liquid Morph, Infinite Zoom |
| **Color** | 28 | Hue Shift, Saturation, Brightness, Duotone, Chromatic Aberration, Invert, Posterize, Color Shift, Thermal, Contrast, Sepia, Cross Process, Split Tone, Color Halftone, Dither, Heat Map, Selective Color, Film Grain, Gamma Levels, Solarize, Greyscale, Threshold, Exposure, Vibrance, Auto Mask, Chroma Key, Palette Remap, Color Grade |
| **Glitch** | 13 | Pixel Scatter, RGB Split, Block Glitch, Scanlines, Digital Rain, Noise, Mirror, Pixelate, Glitch Displace, Pixel Explosion, Color Flash, Fragment Burst, Signal Destroy |
| **Pattern** | 17 | CRT Simulation, VHS Effect, ASCII Art, Dot Matrix, Crosshatch, Emboss, Oil Paint, Pencil Sketch, Voronoi Glass, Cross Stitch, Night Vision, Triangulate, Neon Edge, Cartoon Ink, Pop Raster, Brush Strokes, Bump Light |
| **Animation** | 3 | Strobe, Pulse, Slit Scan |
| **Blend** | 5 | Double Exposure, Frosted Glass, Prism Refract, Rain on Glass, Hexagonalize |
| **Composite** | 3 | Line Cloner, Radial Cloner, Cube Scatter |
| **Blur/Post** | 8 | Gaussian Blur, Zoom Blur, Shake, Vignette, Motion Blur, Glow, Edge Detect, Sharpen, Edge Blur |

### Transition Shaders (15 total)

Clip-to-clip transitions driven by `layer.crossfadeProgress` (0→1). Selected via `layer.transitionMode`. Rendered by `CompositorEngine::applyTransition()` using a dedicated `transitionFBO_`.

| Type | Transitions |
|------|-------------|
| **Standard** | Dissolve, Cut |
| **Wipe** | Wipe Left/Right/Up/Down |
| **Push** | Push Left/Right/Up/Down |
| **Zoom** | Zoom In, Zoom Out |
| **Other** | Iris Circle, Flip Horizontal, Fade to Black |

### Effect Chain Architecture

Effects can be applied at three independent levels — no layer type change is required:

| Level | Data Location | How to Add |
|-------|---------------|-----------|
| **Per-clip** | `Clip::effects` (vector of `EffectSlot`) | Drag FX from browser onto a cell, or onto the clip inspector effect stack |
| **Per-layer** | `Layer::layerEffects` | Drag FX onto the layer inspector effect stack |
| **Global** | `effectChain_` in Renderer | Via the effects rack or composition inspector |

Cells hold clips (images, image sequences, videos) AND procedural sources. FX are independent of media type and layer type.

**Single-image mode**: Input image → FBO A (Effect 1) → FBO B (Effect 2) → FBO A (Effect 3) → ... → Screen. Ping-pong between two FBOs.

**Deck compositing mode** (v2 rendering pipeline per layer):
```
Clip texture → Per-clip effects → Transition blend (if crossfading) → Per-layer effects → Layer transform → Keying (if Transparent) → Blend onto accumulator
```
FX Only layers apply their clip's effects to the composited accumulator. Mask layers use their content as a luminance alpha mask. Global effects (via `effectChain_` in Renderer) run on the final composited output after all layers.

**Important**: `CompositorEngine::applyClipEffects()` resolves shader names via `EffectLibrary::getEffectDef(displayName)->shaderName`, NOT directly from `slot.effectName`. The `slot.effectName` stores the display name (e.g., "Ripple"), while shaders are compiled under snake_case keys (e.g., "ripple").

### FX Drag-and-Drop

Effects are dragged from the FX Browser and dropped onto:
- **Deck cells** — adds the effect to the clip's per-clip effect chain (`Clip::effects`)
- **ClipInspector** — drops anywhere on the inspector add to clip effects (cyan highlight on hover)
- **LayerInspector** — drops anywhere on the inspector add to layer effects (cyan highlight on hover)
- **EffectStackView** — drops directly onto the effect stack area

Each effect row in EffectStackView has: [B] bypass button, effect name, [X] delete button. Effects can be expanded to show parameter sliders. Each slider supports right-click to reset to default value.

MainComponent inherits `juce::DragAndDropContainer`. FXBrowser's FXListContent initiates drags via `startDragging("fx:effectName", ...)`. ClipCell, ClipInspector, LayerInspector, and EffectStackView all implement `juce::DragAndDropTarget`.

### Autopilot System

Autopilot auto-advances clips in a layer. Two trigger modes:
- **On Beat** — advances after N beats (1/2/4/8/16/32), multiplied by the loops count
- **End of Video** — advances when `clip->playheadPosition >= outPoint` (checked every frame, not just on beat crossings)

The `Autopilot` class runs in `Renderer::renderOpenGL()` via `autopilot_.processFrame()`. When clips advance, `onAutopilotAdvanced_` fires async on the message thread to refresh the DeckView.

Layer autopilot fields: `autopilotEnabled`, `autopilotEndOfVideo`, `autopilotLoops`, `defaultAutopilotAction`, `defaultAutopilotDuration`.

### Manual BPM Mode

TopBar has a "Manual" toggle. When enabled:
- An editable BPM text field appears (type value, press Enter)
- `BPMTracker::setManualMode(true)` freezes the stabilization pipeline
- Beat phase still runs from the manually-set BPM
- All beat-driven features (beatPhase, barPhase, phrasePhase, autopilot) work without audio

### Tooltip System

`juce::TooltipWindow` in MainComponent (600ms delay). Any component with `setTooltip()` shows tooltips on hover. Preferences → General has a "Show Tooltips" toggle. Comprehensive tooltip coverage is scheduled for P26 (final build phase).

### UI Patterns (Mandatory for all new UI)

**ResettableSlider**: ALL sliders in the app MUST use `ResettableSlider` (defined in `UniversalParamControl.h`), not `juce::Slider`. This class overrides `mouseDown` to reset to default value on right-click. Every `ResettableSlider` MUST call `setDefaultValue(val)` at setup time. This applies to sliders in inspectors, top bar, layer strip, mapping editor, signal inspector, macro knobs — everywhere.

**Drag-drop targets**: Any inspector that displays an effect stack MUST implement `juce::DragAndDropTarget` with `isInterestedInDragSource`, `itemDragEnter` (set highlight + repaint), `itemDragExit` (clear highlight + repaint), `itemDropped` (forward to EffectStackView). The highlight is a cyan border + 15% alpha fill.

**Effect display name vs shader key**: `Clip::EffectSlot::effectName` stores the human-readable display name (e.g., "Ripple"). Shaders are compiled under snake_case keys (e.g., "ripple"). Always resolve via `EffectLibrary::getEffectDef(displayName)->shaderName` before calling `ShaderManager::getProgram()`. Never assume display name == shader key.

**Transport state**: `Clip::playing` is `mutable` (render thread writes it for OneShot stop). Retriggering the same clip preserves its play/pause state. Switching to a different clip starts playing only on first activation (`hasBeenTriggered` flag). PingPong and OneShot loop modes require propagating player state back to clip model after `advanceFrame()`.

**PopupMenu**: Always use `showMenuAsync()` with `.withParentComponent(getTopLevelComponent())` to ensure menus dismiss on app switch.

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
| **M2** | Full Audio Analysis Engine | 19 tasks | **COMPLETE** |
| **M3** | OpenGL Image Rendering + First Effects | 10 tasks | **COMPLETE** |
| **M4** | Mapping Engine + Full Effects Library | 9 tasks | **COMPLETE** |
| **M5** | VJ-Style UI Polish + Presets | 11 tasks | **COMPLETE** |
| **M6** | Quality, Performance, Cross-Platform | 8 tasks | **COMPLETE** |
| **M7** | ~~Keyboard Launcher~~ | — | **SUPERSEDED by v2** |

### Milestones 1–6: COMPLETE

Core audio pipeline, full 13-stage analysis engine, OpenGL rendering with 96 GLSL effects + 15 transitions, mapping engine, VJ dark theme, presets, fullscreen output, camera input, CI/CD.

### v2 Architecture Redesign (CURRENT)

**M7 (keyboard launcher) has been superseded** by a comprehensive Resolume-class architecture redesign. The 4×10 keyboard grid is replaced by a flexible deck/layer/column system with universal signal routing, macros, and procedural sources.

**Design documents**:

- **`ARCHITECTURE_V2.md`** — Complete system design specification
- **`TASKPLAN_V2.md`** — 12-phase implementation plan (P1-P12)

**Key changes from v1**:

- Deck (layers × columns) replaces keyboard grid
- Signal Bar (mixer-strip audio features) replaces left audio readout
- Universal per-parameter signal routing replaces MappingEditor popup
- Dashboard system (8 link knobs per clip/layer/global) for parameter aggregation
- Binding system (keyboard + MIDI learn) replaces fixed key mapping
- Inspector (4 tabs: Clip/Layer/Composition/Signal) with Resolume-style sections
- Per-parameter signal connect triangle (click → popup: Manual/Audio/BPM Sync/Oscillator/Envelope/Clip Position/Timeline/Macro)
- Transform section (Position X/Y, Scale, Rotation, Anchor) at clip/layer/composition level
- Video section (Opacity, Width, Height, Blend Mode, Alpha Type, RGBA channel toggles)
- Transition section (Blend Mode, Duration) per layer
- Browser (5 tabs: Files/FX/Sources/Comp-Decks/Record) replaces effects rack
- BPM stabilization pipeline (range gate → confidence → octave → median → hysteresis)
- Automatic downbeat detection (no commercial VJ does this from live audio)
- Phrase tracking (bar count + phrasePhase over configurable N bars, resets on structural transitions)
- Beat wheel indicator in TopBar (4-segment circle, bar/phrase readout next to BPM)
- Clip timeline with draggable in/out points, beat division markers, playhead triangle
- Session recording (timestamped event capture + JSON save/load + playback)
- Undo/redo from the start (Command pattern)
- 40 procedural sources (fractal, noise, geometric, etc.)
- Video playback via FFmpeg (MP4/MOV/AVI/MKV/WebM/HAP Alpha) with transport controls
- Image sequence playback (multi-image drag-drop as video) with configurable FPS
- BPM Sync transport mode for video/image sequences with beat division presets
- Content Beats setting for exact beat-locked timing of authored content

**See TASKPLAN_V2.md for current phase and task details.**

---

## Build Instructions

### Prerequisites (all platforms)

- CMake 3.24+
- C++20-capable compiler
- Git (for FetchContent to download JUCE)
- FFmpeg development libraries (libavformat, libavcodec, libavutil, libswscale)

### macOS (primary development platform)

```bash
git clone <repo-url> AudioDNA && cd AudioDNA
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release -j$(sysctl -n hw.ncpu)
./build/AudioDNA_artefacts/Release/Audio-DNA.app/Contents/MacOS/Audio-DNA
```

Required: Xcode Command Line Tools (`xcode-select --install`). FFmpeg: `brew install ffmpeg`. JUCE is fetched automatically.

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

### Development Workflow — Non-Technical User

The user is not technical. **Do not stop for code-level validation.** Keep developing, building, and self-testing (compile checks, grep for rule violations, verify logic) across multiple tasks until there is a **visual UI change the user can verify by launching the app**. Only then present a validation checkpoint with clear instructions on what to look for in the UI.

Batch multiple tasks together when they are all code/infrastructure. Present one combined validation when there's something visible.

### "Kick off phase N" Protocol (v2)

When the user says **"kick off phase N"**, follow this exact sequence:

1. **Read** these files in order:
   - `CLAUDE.md` (this file) — sacred rules, project context
   - `PHASE_GUIDE.md` — find Phase N, read its specific instructions
   - **For phases 1-12**: `ARCHITECTURE_V2.md` + `TASKPLAN_V2.md`
   - **For phases 13-25**: `research/UNIFIED_BUILD_PLAN.md` — this is the ONLY file needed. It contains all tasks, all files to read, the 3-step shader process, validation criteria, and references to detailed GLSL specs in `research/resolumeEffectSourceIntegration.md` and `research/archaosEffectSourceIntegration.md`.

2. **Read** all source files listed in the phase guide for that phase before changing anything

3. **Execute ALL tasks** in the phase without stopping between tasks. Batch everything.

4. **Self-validate** after completing all tasks:
   - Build: `cmake --build build --config Release` exits 0
   - Tests: all existing + new tests pass
   - Grep: no RT violations (no `new`/`malloc` in audio callback or analysis steady-state, no `std::mutex` on hot paths)
   - Phase-specific checks listed in PHASE_GUIDE.md

5. **Decision point — does this phase have UI changes?**
   - **NO UI changes** (P1, P3): Commit to git, update PHASE_GUIDE.md status to COMPLETE, report done. User does NOT need to validate.
   - **YES UI changes** (P2, P4-P20): Report to user with:

     ```text
     ## Phase N Complete
     **What changed**: <summary>
     **What to look for**: <specific UI elements to verify by launching the app>
     ```

     Wait for user to confirm.

6. **On human PASS**: Commit to git, update PHASE_GUIDE.md status to COMPLETE, then run Step 8.
7. **On human FAIL**: Fix the issue, rebuild, re-validate, report again. After final PASS, run Step 8.

8. **Post-phase documentation (MANDATORY after every phase)**:
   - Update `CLAUDE.md`:
     - Effect/source counts if changed
     - Any new architectural patterns, rendering pipeline changes, or data model changes
     - Add new entries to "Common Pitfalls" if bugs were discovered and fixed
     - Add new entries to "UI Patterns" if new interaction conventions were established
   - Update `research/UNIFIED_BUILD_PLAN.md`: mark phase COMPLETE with summary of what shipped
   - Update `PHASE_GUIDE.md`: mark phase COMPLETE
   - Write a project memory file summarizing what was built and any non-obvious lessons
   - Write feedback memory files for any user preferences discovered during testing
   - **Ask**: "Did we learn anything this phase that should change how future phases work?" If yes, update the relevant docs. If Claude identified patterns (common bug classes, UI conventions the user validated, architectural shortcuts), capture them proactively.

### Phase Dependency Map

```text
P1 (BPM lock) ──→ P2 (downbeat) ──→ P3 (architecture) ──→ P4 (signal bar)
                                                          ──→ P5 (deck)
                                                          ──→ P6 (inspector)
                                                          ──→ P7 (browser)
                                          P4+P5+P6+P7 ──→ P8 (layout)
                                                    P8 ──→ P9 (binding)
                                                P5+P6 ──→ P10 (sources)
                                                P5+P6 ──→ P11 (video)
                                                  All ──→ P12 (polish)
```

### Before Any Work

- Always read this CLAUDE.md before touching any file
- Check PHASE_GUIDE.md for current phase status and what's next
- Read `ARCHITECTURE_V2.md` for the v2 design spec
- Read `TASKPLAN_V2.md` for task details
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

### Common Pitfalls (from P14 development)

These bugs were discovered and fixed during P14. Future phases MUST avoid reintroducing them:

1. **Shader lookup mismatch**: `Clip::EffectSlot::effectName` stores the display name ("Ripple"), but shaders are compiled under snake_case keys ("ripple"). Always resolve via `EffectLibrary::getEffectDef(displayName)->shaderName`. Never use `slot.effectName` directly as a shader key.

2. **Loop mode race condition**: The render thread syncs `clip->playing` to the player every frame. If the player stops itself (OneShot boundary), the render thread immediately restarts it from `clip->playing == true`. Fix: after `advanceFrame()`, read the player's state BACK to the clip model (`clip->playing = player->isPlaying()`). For PingPong, don't override `setReverse()` every frame — PingPong manages direction internally.

3. **FBO conflicts**: `scratchFBO_` is used by keying. `effectFBO_A_/B_` are used by per-clip and per-layer effects (ping-pong). Transitions need their own `transitionFBO_` to avoid overwriting scratch before keying runs.

4. **Demo effects left enabled**: `initEffectChain()` must NOT enable any effects by default. Users build their own effect chains via the FX browser.

5. **JUCE slider right-click**: `juce::Slider` eats right-click events before the parent component's `mouseDown` fires. Use `ResettableSlider` (custom subclass) which overrides `mouseDown` to handle right-click reset directly. Always call `setDefaultValue()` on creation.

6. **Unicode button text**: JUCE's default button font at small sizes (26px buttons) may not render multi-byte Unicode glyphs. Use ASCII characters ("<", ">", "||") instead of Unicode arrows/symbols for small buttons.

7. **Transport state on clip switch**: `triggerClipImmediate()` must NOT force `playing = true` when re-activating a previously-played clip. Use a `hasBeenTriggered` flag to distinguish first activation from returning to a prior clip.

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
