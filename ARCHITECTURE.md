# Audio-DNA: System Architecture

> Cross-platform desktop application (C++ / JUCE / OpenGL) that analyzes audio in real-time and applies a library of GLSL shader effects to a loaded image, driven by audio features. VJ-style performance UI.

---

## 1. Component Diagram

```
┌──────────────────────────────────────────────────────────────────────────────┐
│                              AUDIO-DNA APPLICATION                           │
│                                                                              │
│  ┌─────────────────────────────────────────────────────────────────────────┐ │
│  │                         JUCE Application Shell                          │ │
│  │  AudioDeviceManager · DocumentWindow · OpenGLContext · MessageManager   │ │
│  └─────────────────────────────────────────────────────────────────────────┘ │
│                                                                              │
│  ┌──────────────┐   ┌──────────────────┐   ┌────────────────────────────┐   │
│  │ AUDIO I/O    │   │ ANALYSIS ENGINE  │   │ EFFECTS / RENDER ENGINE    │   │
│  │              │   │                  │   │                            │   │
│  │ File Reader  │   │ FFT Processor    │   │ Image Loader (stb/JUCE)   │   │
│  │ (JUCE Audio  │──▶│ Band Analyzer    │──▶│ Shader Effect Library     │   │
│  │  Transport)  │   │ Onset Detector   │   │ Effect Chain Manager      │   │
│  │              │   │ BPM Tracker      │   │ Uniform Bridge            │   │
│  │ Device Input │   │ Spectral Feats   │   │ OpenGL Renderer           │   │
│  │ (mic/line-in)│   │ Pitch/Chroma     │   │                            │   │
│  │              │   │ MFCC Extractor   │   │                            │   │
│  └──────┬───────┘   │ Structural Det.  │   └─────────────┬──────────────┘   │
│         │           │ Loudness (LUFS)  │                 │                   │
│         │           └────────┬─────────┘                 │                   │
│         │                    │                           │                   │
│  ┌──────▼───────┐   ┌───────▼────────┐   ┌─────────────▼──────────────┐    │
│  │ SPSC Ring    │   │ Feature Bus    │   │ Mapping Engine             │    │
│  │ Buffer       │   │ (Triple Buffer)│   │                            │    │
│  │ (Audio→Anal) │   │ (Anal→Render)  │   │ Source (audio feature)     │    │
│  └──────────────┘   └────────────────┘   │ Target (effect parameter)  │    │
│                                           │ Curve (lin/exp/step/s)     │    │
│  ┌──────────────────────────────────────┐ │ Range (min/max)            │    │
│  │          UI LAYER                    │ │ Smoothing (EMA/OneEuro)    │    │
│  │                                      │ └────────────────────────────┘    │
│  │ Left: Audio Readouts + Waveform/Spec │                                   │
│  │ Center: Live Image Preview (OpenGL)  │                                   │
│  │ Right: Effects Rack + Mapping Editor │                                   │
│  └──────────────────────────────────────┘                                   │
└──────────────────────────────────────────────────────────────────────────────┘
```

---

## 2. Thread Model

Four threads with strict priority ordering. No mutexes on any hot path.

```
┌─────────────────┐     ┌─────────────────┐     ┌─────────────────┐     ┌─────────────────┐
│  AUDIO CALLBACK │     │ ANALYSIS THREAD  │     │  RENDER THREAD  │     │  MESSAGE THREAD  │
│  (OS-managed)   │     │ (App-managed)    │     │  (OpenGL)       │     │  (JUCE UI)       │
│                 │     │                  │     │                 │     │                  │
│ Priority:       │     │ Priority:        │     │ Priority:       │     │ Priority:        │
│  REAL-TIME      │     │  ABOVE-NORMAL    │     │  NORMAL (VSync) │     │  NORMAL          │
│                 │     │                  │     │                 │     │                  │
│ Runs every:     │     │ Runs every:      │     │ Runs every:     │     │ Runs on:         │
│  2.67ms (128    │     │  ~10.7ms (512    │     │  16.67ms (60fps)│     │  User events     │
│  samples@48kHz) │     │  hop@48kHz)      │     │                 │     │                  │
│                 │     │                  │     │                 │     │                  │
│ Does:           │     │ Does:            │     │ Does:           │     │ Does:            │
│  Copy samples   │     │  FFT + features  │     │  Read features  │     │  Handle input    │
│  into ring buf  │     │  BPM tracking    │     │  Apply mappings │     │  Update params   │
│                 │     │  Onset detection │     │  Upload uniforms│     │  Paint controls  │
│                 │     │  Publish snapshot│     │  Render shaders │     │                  │
│ NEVER:          │     │                  │     │  Swap buffers   │     │                  │
│  allocate       │     │ NEVER:           │     │                 │     │                  │
│  lock           │     │  allocate in     │     │                 │     │                  │
│  syscall        │     │  steady-state    │     │                 │     │                  │
└────────┬────────┘     └────────┬─────────┘     └────────┬────────┘     └────────┬─────────┘
         │                       │                         │                       │
         │   SPSC Ring Buffer    │   Triple-Buffer Swap    │   Atomic Config Vars  │
         ├───────────────────────▶├─────────────────────────▶                       │
         │   (lock-free)         │   (lock-free)           │◀──────────────────────┤
         │                       │                         │                       │
```

### Thread Communication

| From | To | Mechanism | Data |
|------|-----|-----------|------|
| Audio Callback | Analysis | SPSC Ring Buffer (wait-free) | Raw float samples |
| Analysis | Render | Triple-buffer atomic swap (lock-free) | `FeatureSnapshot` struct |
| UI (Message) | Analysis/Render | `std::atomic<T>` config variables | Parameters, enable flags |
| Render | UI (Message) | `juce::MessageManager::callAsync()` | Display values, FPS |

### Thread Priority Configuration

| Platform | Audio Callback | Analysis Thread | Render Thread |
|----------|---------------|-----------------|---------------|
| macOS | CoreAudio IOProc (RT managed by HAL) | `pthread_setschedparam(SCHED_RR, 47)` | Normal (VSync) |
| Windows | WASAPI event thread (MMCSS "Pro Audio") | `SetThreadPriority(ABOVE_NORMAL)` | Normal (VSync) |
| Linux | JACK SCHED_FIFO / ALSA RT | `pthread_setschedparam(SCHED_FIFO, 50)` | Normal (VSync) |

---

## 3. Data Flow

```
                        DATA FLOW: Audio In → Shader Uniform Out

 ┌──────────┐    ┌────────────┐    ┌─────────────┐    ┌──────────────┐    ┌──────────┐
 │  Audio   │    │   SPSC     │    │  Analysis   │    │  Feature     │    │  Render  │
 │  Source  │───▶│   Ring     │───▶│  Thread     │───▶│  Bus         │───▶│  Thread  │
 │          │    │  Buffer    │    │             │    │ (TripleBuf)  │    │          │
 │ Song file│    │ 16384 flt  │    │ Window+FFT  │    │              │    │ Read snap│
 │ or mic   │    │ (~341ms    │    │ Mag spectrum│    │ FeatureSnap  │    │ Mappings │
 │          │    │  @48kHz)   │    │ Features    │    │ → publish()  │    │ → curves │
 └──────────┘    └────────────┘    │ BPM/onset   │    │              │    │ → ranges │
                                   └─────────────┘    └──────────────┘    │ Uniforms │
                                                                          │ → shader │
                                                                          │ Draw quad│
                                                                          │ with img │
                                                                          └──────────┘
```

### Detailed per-frame data path (worst case ~23ms end-to-end):

| Stage | Operation | Latency |
|-------|-----------|---------|
| 1. Audio buffer delivery | OS delivers 128 samples @48kHz | 2.67ms (period) |
| 2. Ring buffer push | `memcpy` into SPSC | ~50ns |
| 3. Hop accumulation | Wait for 512 samples (1 hop) | 10.7ms (hop period) |
| 4. Window + FFT | Hann window, 2048-pt FFT | ~20μs |
| 5. Feature extraction | All spectral + temporal features | ~100μs |
| 6. Feature bus publish | Atomic triple-buffer swap | ~10ns |
| 7. Render acquire | Atomic read of latest snapshot | ~10ns |
| 8. Mapping engine | Apply curves, smoothing to all mappings | ~5μs |
| 9. Uniform upload | glUniform + UBO update | ~2μs |
| 10. Shader render | Effect chain on fullscreen quad + image | ~1-3ms |
| 11. Swap buffers | VSync present | 0-16.67ms |

**Total audio-to-visual latency: ~15-25ms** (well within the ±80ms perceptual sync window).

---

## 4. Core Data Structures

### 4.1 FeatureSnapshot

The unit of transfer between Analysis and Render threads. Fixed-size POD, cache-line aligned.

```cpp
struct alignas(64) FeatureSnapshot {
    // Timing
    uint64_t timestamp;              // Sample clock
    double   wallClockSeconds;       // For render interpolation

    // Amplitude & Dynamics
    float rms;                       // [0, 1]
    float peak;                      // [0, 1]
    float rmsDB;                     // dBFS [-100, 0]
    float lufs;                      // LUFS momentary
    float dynamicRange;              // Crest factor or similar
    float transientDensity;          // Onsets per second (windowed)

    // Spectral
    float spectralCentroid;          // Hz
    float spectralFlux;              // Unbounded, normalized per-session
    float spectralFlatness;          // [0, 1]
    float spectralRolloff;           // Hz

    // Rhythm
    bool  onsetDetected;             // This frame
    float onsetStrength;             // Detection function value
    float bpm;                       // Current tempo estimate
    float beatPhase;                 // [0, 1) sawtooth synced to beat

    // Structural
    uint8_t structuralState;         // 0=normal, 1=buildup, 2=drop, 3=breakdown

    // Band Energies (7 bands)
    // Sub(20-60), Bass(60-250), LowMid(250-500), Mid(500-2k),
    // HighMid(2k-4k), Presence(4k-6k), Brilliance(6k-20k)
    float bandEnergies[7];

    // Pitch / Chroma
    float chromagram[12];            // C, C#, D, ... B
    float dominantPitch;             // Hz
    float pitchConfidence;           // [0, 1]
    int   detectedKey;               // 0-11 (C=0), -1 = unknown
    bool  keyIsMajor;                // true=major, false=minor

    // MFCC
    float mfccs[13];                 // Coefficients 0-12

    // Harmonic
    float harmonicChangeDetection;   // HCDF value
};
```

### 4.2 Mapping

The core creative data structure: routes any audio feature to any effect parameter.

```cpp
struct Mapping {
    enum class Source : uint8_t {
        RMS, Peak, SpectralCentroid, SpectralFlux, SpectralFlatness,
        BeatPhase, OnsetStrength, BPM, Bass, LowMid, Mid, HighMid,
        Treble, LUFS, TransientDensity, Pitch, ChromaSum,
        StructuralState, DynamicRange, MFCC0, /* ... */ MFCC12
    };

    enum class Curve : uint8_t {
        Linear, Exponential, Logarithmic, SCurve, Stepped
    };

    Source   source;
    uint32_t targetEffectId;         // Which effect
    uint32_t targetParamIndex;       // Which parameter on that effect
    Curve    curve;
    float    inputMin, inputMax;     // Source range (auto or manual)
    float    outputMin, outputMax;   // Target range [0, 1] default
    float    smoothing;              // EMA alpha or OneEuro beta
    bool     enabled;
};
```

### 4.3 Effect

Each effect is a named GLSL shader with typed parameters.

```cpp
struct EffectParam {
    std::string name;                // "intensity", "frequency", "amount"
    float       value;               // Current value [0, 1]
    float       defaultValue;
};

struct Effect {
    std::string           name;      // "Ripple", "Chromatic Aberration"
    std::string           category;  // "warp", "color", "glitch", "blur"
    GLuint                shaderProgram;
    std::vector<EffectParam> params;
    bool                  enabled;
    int                   order;     // Position in effect chain
};
```

---

## 5. System Components — Detail

### 5.1 Audio I/O (JUCE AudioDeviceManager)

**Why JUCE for audio I/O** (not raw miniaudio):
- JUCE provides `AudioTransportSource` for file playback with read-ahead buffering, format decoding (WAV, AIFF, FLAC, MP3, OGG), and transport controls — critical for the "load a song" use case.
- JUCE's `AudioDeviceManager` handles device enumeration, hot-plugging, sample rate negotiation, and format conversion across CoreAudio / WASAPI / ALSA / JACK.
- JUCE's `AudioIODeviceCallback` runs on the OS real-time thread — same guarantees as raw CoreAudio/WASAPI.
- Single framework for audio I/O + UI + OpenGL avoids glue code between separate libraries.

**Audio callback contract**: The callback receives interleaved or deinterleaved float samples. It does exactly one thing — pushes mono-downmixed samples into the SPSC ring buffer. No DSP, no allocation, no locks.

**File playback path**: `AudioFormatReader` → `AudioFormatReaderSource` → `AudioTransportSource` (background read thread) → audio callback → ring buffer. The transport source handles disk I/O on a background thread; the audio callback only reads from an internal FIFO.

### 5.2 Analysis Engine

Runs on a dedicated thread. Pulls samples from the ring buffer in hop-sized chunks (512 samples), maintains an overlap buffer (2048 samples for FFT), and extracts all features per hop.

**FFT**: `juce::dsp::FFT` (uses vDSP on macOS, IPP if available, fallback internal). 2048-point with Hann window. Produces 1025 complex bins → magnitude spectrum.

**Feature extraction order** (each depends on prior results):

```
1. Raw time-domain: RMS, peak, ZCR
2. FFT → magnitude spectrum
3. From magnitude: centroid, flux, flatness, rolloff, band energies
4. Onset detection: spectral flux thresholding (adaptive median + offset)
5. BPM tracking: onset accumulator → autocorrelation → tempo estimate + beat phase
6. MFCC: mel filterbank on magnitude → log → DCT → 13 coefficients
7. Chroma: magnitude bins → 12 pitch classes
8. Pitch: YIN or aubio_pitch on time-domain signal
9. Key detection: chroma profile → Krumhansl-Schmuckler correlation
10. LUFS: K-weighted RMS over 400ms window
11. Structural: multi-scale EMA envelopes → buildup/drop/breakdown state machine
12. HCDF: chroma difference function for harmonic change
13. Transient density: onset count in sliding window
```

**Aubio integration**: Used specifically for BPM tracking (`aubio_tempo`) and onset detection (`aubio_onset`) — these algorithms are battle-tested and beat custom implementations. Aubio objects created at startup, fed samples per-hop, results read via getters. License: GPL — acceptable for this application.

**Why not Essentia**: Essentia's streaming mode is powerful but adds significant build complexity (AGPL license, large dependency tree including FFTW, TagLib, yaml-cpp). For MVP, JUCE's built-in FFT + Aubio for rhythm covers all features. Essentia can be added later for advanced features (key detection, chord recognition) if needed.

### 5.3 Effects Library

Each effect is a GLSL fragment shader that takes the input image as a texture and produces a modified output. Effects are chained via ping-pong FBOs.

**Effect chain architecture**:

```
Input Image Texture
        │
        ▼
┌───────────────┐
│  FBO A        │ ← Render Effect 1 (e.g., Ripple)
│  (texture A)  │
└───────┬───────┘
        │ texture A as input
        ▼
┌───────────────┐
│  FBO B        │ ← Render Effect 2 (e.g., Hue Shift)
│  (texture B)  │
└───────┬───────┘
        │ texture B as input
        ▼
┌───────────────┐
│  FBO A        │ ← Render Effect 3 (e.g., Scanlines)
│  (texture A)  │
└───────┬───────┘
        │ Final output
        ▼
    Screen / Preview Component
```

**Effect categories and shaders** (MVP set):

| Category | Effect | Parameters | GLSL Uniform(s) |
|----------|--------|------------|-----------------|
| **Warp** | Ripple | intensity, frequency, speed | `u_ripple_intensity`, `u_ripple_freq`, `u_ripple_speed` |
| | Bulge | amount, center_x, center_y | `u_bulge_amount`, `u_bulge_center` |
| | Wave | amplitude, frequency, direction | `u_wave_amp`, `u_wave_freq` |
| | Liquid | viscosity, turbulence | `u_liquid_visc`, `u_liquid_turb` |
| **Color** | Hue Shift | amount | `u_hue_shift` |
| | Saturation | amount | `u_saturation` |
| | Brightness | amount | `u_brightness` |
| | Duotone | color1, color2, mix | `u_duotone_a`, `u_duotone_b` |
| | Chromatic Aberration | amount, angle | `u_chroma_amount`, `u_chroma_angle` |
| **Glitch** | Pixel Scatter | amount, seed | `u_scatter_amount` |
| | RGB Split | amount, angle | `u_rgb_split` |
| | Block Glitch | intensity, block_size | `u_block_glitch_int` |
| | Scanlines | intensity, frequency | `u_scanline_int`, `u_scanline_freq` |
| **Blur** | Gaussian Blur | radius | `u_blur_radius` |
| | Zoom Blur | amount, center_x, center_y | `u_zoom_blur` |
| | Shake | amount_x, amount_y | `u_shake` |
| | Vignette | intensity, softness | `u_vignette_int`, `u_vignette_soft` |

All parameters normalized to [0.0, 1.0]. The shader maps this to its internal range.

### 5.4 Mapping Engine

The mapping engine sits between the Feature Bus and the Uniform Bridge. Each frame:

1. Read latest `FeatureSnapshot` from triple buffer
2. For each active `Mapping`:
   a. Extract source value from snapshot
   b. Normalize: `(raw - inputMin) / (inputMax - inputMin)` → [0, 1]
   c. Apply curve transform (linear, exp, log, sigmoid, stepped)
   d. Scale to output range: `outputMin + curved * (outputMax - outputMin)`
   e. Apply smoothing (EMA or One-Euro filter, per-mapping state)
   f. Write result to the target effect's parameter slot
3. Pass all effect parameters to the Uniform Bridge for GPU upload

**Curve transforms**:
- **Linear**: `y = x`
- **Exponential**: `y = x^power` (power = 2.0 default, emphasizes peaks)
- **Logarithmic**: `y = log(1 + x * 9) / log(10)` (compresses peaks, lifts lows)
- **S-Curve**: `y = x^2 * (3 - 2x)` (smoothstep, de-emphasizes extremes)
- **Stepped**: `y = floor(x * steps) / steps` (quantized, N steps)

### 5.5 UI Layout

```
┌──────────────────────────────────────────────────────────────────────────────┐
│  [File: song.wav ▼]  [Image: photo.jpg ▼]  [BPM: 128.0]  [Key: Am]  [FPS] │
├──────────────┬───────────────────────────────┬───────────────────────────────┤
│              │                               │                              │
│  AUDIO       │      LIVE PREVIEW             │   EFFECTS RACK               │
│  READOUTS    │                               │                              │
│              │   ┌───────────────────────┐   │   ┌────────────────────────┐ │
│  RMS: ████░  │   │                       │   │   │ [x] Ripple      ▼map  │ │
│  Peak: █████ │   │                       │   │   │  intensity ●────── 0.7│ │
│  LUFS: -14   │   │     Image with        │   │   │    ← RMS [lin]        │ │
│              │   │     effects applied    │   │   │  frequency ●────── 0.3│ │
│  Centroid:   │   │     in real-time       │   │   │    ← Bass [exp]       │ │
│   2.4kHz     │   │                       │   │   ├────────────────────────┤ │
│  Flux: 0.34  │   │                       │   │   │ [x] Hue Shift   ▼map  │ │
│              │   │                       │   │   │  amount ●──────── 0.5 │ │
│  ┌─────────┐ │   └───────────────────────┘   │   │    ← Centroid [log]   │ │
│  │ waveform│ │                               │   ├────────────────────────┤ │
│  └─────────┘ │                               │   │ [ ] RGB Split    ▼map  │ │
│  ┌─────────┐ │                               │   │  amount ●──────── 0.0 │ │
│  │ spectrum│ │                               │   │    ← (unmapped)       │ │
│  └─────────┘ │                               │   ├────────────────────────┤ │
│              │                               │   │ [+ Add Effect]         │ │
│  Bands:      │                               │   │                        │ │
│  Sub  ███░░  │                               │   │  MAPPING EDITOR:       │ │
│  Bass ████░  │                               │   │  Source: [RMS ▼]       │ │
│  Mid  ██░░░  │                               │   │  Curve:  [Linear ▼]   │ │
│  High █░░░░  │                               │   │  Range:  0.0 ── 1.0   │ │
│              │                               │   │  Smooth: ●──── 0.15   │ │
│  Onset: *    │                               │   │                        │ │
│  Beat: ♩     │                               │   └────────────────────────┘ │
│  BPM: 128    │                               │                              │
│  Phase: 0.72 │                               │                              │
│  State: DROP │                               │                              │
│  MFCC[0]: .. │                               │                              │
└──────────────┴───────────────────────────────┴──────────────────────────────┘
```

---

## 6. Technology Decisions

| Component | Choice | Justification | Alternative Considered |
|-----------|--------|---------------|----------------------|
| **App Framework** | JUCE 7/8 | Single framework for audio I/O, UI, OpenGL, file playback. Cross-platform. Battle-tested in pro audio. GPLv3 or commercial license. | SDL2+ImGui (no audio file playback), Qt (heavy, poor RT audio) |
| **Audio I/O** | JUCE AudioDeviceManager | Wraps CoreAudio/WASAPI/ALSA/JACK. Handles device enum, hot-plug, format conversion. Needed anyway for file playback. | miniaudio (no file decode, no UI), RtAudio (no file decode) |
| **File Playback** | JUCE AudioTransportSource | Background disk read, format decode (WAV/AIFF/FLAC/MP3/OGG), transport controls, sample rate conversion. | libsndfile + custom transport (more code, fewer formats) |
| **FFT** | juce::dsp::FFT | Uses vDSP (macOS) / IPP (if available) / internal fallback. No extra dependency. Adequate for 2048-pt. | FFTW (GPL, faster for large N but overkill at 2048), KissFFT (slower, extra dep) |
| **BPM / Onset** | Aubio (libaubio) | Best-in-class beat tracking and onset detection. C API, small footprint. Genre-tunable parameters. | Custom spectral flux (adequate for onset, poor BPM), Essentia (heavier) |
| **Spectral Features** | Custom C++ | Centroid, flux, flatness, rolloff, band energies are 5-30 lines each from magnitude spectrum. No library needed. | Essentia (overkill for these), Aubio spectral descriptors (subset only) |
| **MFCC / Chroma** | Custom C++ | Mel filterbank + DCT for MFCC, bin-to-pitch-class mapping for chroma. Well-documented algorithms, ~100 lines each. | Essentia (adds AGPL + large deps for two algorithms) |
| **Key Detection** | Custom C++ (Krumhansl-Schmuckler) | Correlation of chroma profile with 24 key templates. ~50 lines. | Essentia KeyExtractor (heavier), music21 (Python only) |
| **LUFS** | Custom C++ (ITU-R BS.1770) | K-weighting biquad + 400ms window RMS. Well-specified standard. ~80 lines. | libebur128 (extra dep for one feature) |
| **Rendering** | OpenGL 4.1 Core Profile via JUCE | JUCE's OpenGLContext runs on dedicated render thread. macOS caps at 4.1 (Apple deprecated GL). Sufficient for 2D image effects. | Vulkan (overkill for 2D), Metal (macOS-only), WebGPU (immature) |
| **Shaders** | GLSL 410 | Matches OpenGL 4.1 target. Hot-reloadable from disk. | SPIR-V (requires compilation step), HLSL (Windows-only) |
| **Image Loading** | JUCE Image / stb_image | JUCE handles PNG/JPEG/GIF natively. stb_image as fallback for additional formats. | OpenCV (massive overkill), FreeImage (extra dep) |
| **Build System** | CMake 3.24+ | JUCE 7+ has first-class CMake support (`juce_add_gui_app`). Industry standard. | Projucer (JUCE-only, being deprecated), Meson (less JUCE support) |
| **Testing** | Catch2 | Header-only, BDD-style, integrates with CMake/CTest. | Google Test (heavier), doctest (similar, less adoption) |

---

## 7. Source Tree

```
AudioDNA/
├── CMakeLists.txt                    # Root build: JUCE + Aubio + Catch2
├── cmake/
│   ├── FindAubio.cmake               # Locate libaubio
│   └── CompilerWarnings.cmake        # Per-compiler warning flags
├── src/
│   ├── Main.cpp                      # JUCE app entry point
│   ├── MainComponent.h/cpp           # Top-level component, owns all systems
│   ├── audio/
│   │   ├── AudioEngine.h/cpp         # AudioDeviceManager + file transport
│   │   ├── AudioCallback.h/cpp       # RT callback → ring buffer
│   │   └── RingBuffer.h              # Lock-free SPSC
│   ├── analysis/
│   │   ├── AnalysisThread.h/cpp      # Dedicated thread, feature pipeline
│   │   ├── FFTProcessor.h/cpp        # juce::dsp::FFT wrapper
│   │   ├── SpectralFeatures.h/cpp    # Centroid, flux, flatness, rolloff, bands
│   │   ├── OnsetDetector.h/cpp       # Aubio onset wrapper
│   │   ├── BPMTracker.h/cpp          # Aubio tempo wrapper
│   │   ├── MFCCExtractor.h/cpp       # Mel filterbank + DCT
│   │   ├── ChromaExtractor.h/cpp     # 12-bin chroma from FFT
│   │   ├── KeyDetector.h/cpp         # Krumhansl-Schmuckler
│   │   ├── LoudnessAnalyzer.h/cpp    # LUFS (ITU-R BS.1770)
│   │   ├── StructuralDetector.h/cpp  # Buildup/drop/breakdown state machine
│   │   └── FeatureSnapshot.h         # POD struct definition
│   ├── features/
│   │   ├── FeatureBus.h/cpp          # Triple-buffer atomic swap
│   │   └── Smoother.h/cpp            # EMA + One-Euro filter
│   ├── mapping/
│   │   ├── MappingEngine.h/cpp       # Source→Target routing + curves
│   │   ├── MappingTypes.h            # Mapping, Source, Curve enums
│   │   └── CurveTransforms.h         # lin/exp/log/sigmoid/step functions
│   ├── effects/
│   │   ├── EffectLibrary.h/cpp       # Registry of all available effects
│   │   ├── Effect.h/cpp              # Single effect: shader + params
│   │   ├── EffectChain.h/cpp         # Ordered chain with ping-pong FBOs
│   │   └── UniformBridge.h/cpp       # Maps effect params → GL uniforms
│   ├── render/
│   │   ├── Renderer.h/cpp            # OpenGLRenderer impl, frame loop
│   │   ├── ShaderManager.h/cpp       # Compile, link, hot-reload shaders
│   │   ├── TextureManager.h/cpp      # Image texture upload + FBO textures
│   │   └── FullscreenQuad.h/cpp      # VAO/VBO for fullscreen triangle strip
│   └── ui/
│       ├── AudioReadoutPanel.h/cpp   # Left panel: feature values + viz
│       ├── PreviewPanel.h/cpp        # Center: OpenGL preview component
│       ├── EffectsRackPanel.h/cpp    # Right panel: effect list + params
│       ├── MappingEditor.h/cpp       # Mapping configuration UI
│       ├── WaveformDisplay.h/cpp     # Scrolling waveform
│       ├── SpectrumDisplay.h/cpp     # Bar/line spectrum analyzer
│       └── LookAndFeel.h/cpp         # Dark VJ-style theme
├── shaders/
│   ├── passthrough.vert              # Shared vertex shader (fullscreen quad)
│   ├── ripple.frag
│   ├── bulge.frag
│   ├── wave.frag
│   ├── liquid.frag
│   ├── hue_shift.frag
│   ├── saturation.frag
│   ├── brightness.frag
│   ├── duotone.frag
│   ├── chromatic_aberration.frag
│   ├── pixel_scatter.frag
│   ├── rgb_split.frag
│   ├── block_glitch.frag
│   ├── scanlines.frag
│   ├── gaussian_blur.frag
│   ├── zoom_blur.frag
│   ├── shake.frag
│   └── vignette.frag
├── tests/
│   ├── CMakeLists.txt
│   ├── test_ring_buffer.cpp
│   ├── test_spectral_features.cpp
│   ├── test_onset_detector.cpp
│   ├── test_mapping_engine.cpp
│   ├── test_smoother.cpp
│   └── test_feature_bus.cpp
└── resources/
    ├── default_image.png             # Fallback test image
    └── presets/
        └── default_mappings.json     # Default mapping preset
```

---

## 8. Key Constraints & Invariants

1. **Audio callback is sacred**: Never allocate, lock, or syscall inside `audioDeviceIOCallbackWithContext`. It copies samples to ring buffer and returns.

2. **Analysis thread pre-allocates everything**: All FFT plans, work buffers, Aubio objects, filter states created at startup. Zero allocation in steady-state loop.

3. **Render thread never waits for analysis**: If no new snapshot is available, reuse the previous one. The render loop runs at VSync regardless of analysis rate.

4. **All inter-thread data flows forward**: Audio → Analysis → Render. No backward dependencies. UI writes config atomically; hot path reads it.

5. **OpenGL 4.1 minimum**: macOS constraint (Apple deprecated GL). No compute shaders (require 4.3). All effects are fragment shaders on fullscreen quads.

6. **Effect parameters are always [0, 1]**: The mapping engine and UI both work in normalized space. The shader maps to its internal range.

7. **Feature snapshot is a value type**: Copied by value into the triple buffer. No pointers, no heap references. POD with `memcpy` semantics.

8. **Shaders are hot-reloadable**: Loaded from `shaders/` directory at runtime. File watcher triggers recompile without app restart.

---

## 9. Dependency Summary

| Dependency | Version | License | Integration |
|------------|---------|---------|-------------|
| JUCE | 7.x / 8.x | GPLv3 or Commercial | CMake `FetchContent` or git submodule |
| Aubio | 0.4.9+ | GPLv3 | System install or CMake `FetchContent` |
| Catch2 | 3.x | BSL-1.0 | CMake `FetchContent` (test only) |
| stb_image | latest | Public domain | Single header in `third_party/` (optional, JUCE handles most formats) |

**Total external dependencies: 2 runtime (JUCE, Aubio) + 1 test-only (Catch2).**

Aubio's only transitive dependency is a C math library. JUCE bundles its own dependencies (freetype, zlib, etc.). The build is self-contained.

---

## 10. Performance Budget

| Resource | Budget | Notes |
|----------|--------|-------|
| Audio callback | < 100μs per invocation | Just `memcpy` to ring buffer |
| Analysis per hop | < 2ms | FFT + all features. Hop period is 10.7ms — 5x headroom |
| Render per frame | < 8ms | Effect chain + uniform upload. VSync period is 16.67ms |
| Memory (steady state) | < 50MB | Ring buffer (~64KB), triple buffer (~3KB), textures (~16MB for 4K image), FBOs (~32MB for 1080p pair) |
| CPU cores | 3 active | Audio callback (intermittent), analysis (continuous), render (continuous). UI uses render thread or message thread. |
