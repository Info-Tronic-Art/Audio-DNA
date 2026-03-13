# Audio-DNA: MVP Task Plan

> Phased milestones from zero to working MVP. Each milestone builds on the previous and produces something visible/testable.

---

## Milestone 1: Window + Audio + Waveform (One Session)

**Goal**: JUCE app opens a window, loads and plays an audio file, shows a live waveform and RMS meter. Proves the audio pipeline works end-to-end from file → callback → ring buffer → analysis → display.

**Definition of Done**:
- App window opens with dark background
- User can load a WAV/AIFF/MP3 file via file chooser
- Audio plays through default output device
- Scrolling waveform displays in real-time
- RMS level bar updates in real-time
- Play/pause/stop transport controls work
- No audio glitches or dropouts during playback

### Tasks

- [ ] **1.1** Create project skeleton: `CMakeLists.txt` with JUCE via `FetchContent`, `juce_add_gui_app()`, C++20, basic compiler warnings
  - Files: `CMakeLists.txt`, `cmake/CompilerWarnings.cmake`

- [ ] **1.2** Create JUCE app entry point and main window with dark `LookAndFeel`
  - Files: `src/Main.cpp`, `src/MainComponent.h`, `src/MainComponent.cpp`, `src/ui/LookAndFeel.h`, `src/ui/LookAndFeel.cpp`

- [ ] **1.3** Implement `AudioEngine`: wraps `AudioDeviceManager`, `AudioTransportSource`, `AudioFormatManager`. Loads a file, plays it, provides transport controls (play/pause/stop)
  - Files: `src/audio/AudioEngine.h`, `src/audio/AudioEngine.cpp`

- [ ] **1.4** Implement `RingBuffer<float>`: SPSC lock-free ring buffer with power-of-two capacity, acquire/release atomics, cache-line padding
  - Files: `src/audio/RingBuffer.h`

- [ ] **1.5** Implement `AudioCallback`: receives samples from JUCE audio callback, mono-downmixes, pushes into ring buffer. Zero allocation.
  - Files: `src/audio/AudioCallback.h`, `src/audio/AudioCallback.cpp`

- [ ] **1.6** Implement `AnalysisThread` (minimal): dedicated thread that reads from ring buffer, computes RMS per block (2048 samples, 512 hop). Publishes to a simple atomic float for now.
  - Files: `src/analysis/AnalysisThread.h`, `src/analysis/AnalysisThread.cpp`, `src/analysis/FeatureSnapshot.h`

- [ ] **1.7** Implement `WaveformDisplay` component: scrolling time-domain waveform using ring buffer data or analysis output. JUCE `Component::paint()` with `repaint()` timer at 30fps.
  - Files: `src/ui/WaveformDisplay.h`, `src/ui/WaveformDisplay.cpp`

- [ ] **1.8** Implement RMS level meter: simple horizontal bar that reads the atomic RMS value
  - Files: `src/ui/AudioReadoutPanel.h`, `src/ui/AudioReadoutPanel.cpp`

- [ ] **1.9** Wire everything together in `MainComponent`: layout with transport buttons + file chooser at top, waveform + RMS meter in center
  - Files: update `src/MainComponent.h`, `src/MainComponent.cpp`

- [ ] **1.10** Build and test on macOS. Verify no audio dropouts, smooth waveform, responsive RMS meter.

---

## Milestone 2: Full Audio Analysis Engine

**Goal**: Extract all audio features in real-time: FFT, all spectral features, band energies, onset detection, BPM/beat tracking, MFCC, chroma, pitch, key, LUFS, structural detection. Display all values as live readouts.

**Definition of Done**:
- All features listed in `FeatureSnapshot` are computed and published via triple-buffer `FeatureBus`
- Left panel shows all feature values updating in real-time
- Spectrum analyzer (bar display) shows frequency content
- Band energy meters (7 bands) update smoothly
- Onset detection flashes on transients
- BPM display shows detected tempo; beat phase indicator pulses
- No analysis dropouts (analysis thread keeps up with audio)

### Tasks

- [ ] **2.1** Implement `FeatureBus` with triple-buffer atomic swap (3 `FeatureSnapshot` buffers, single atomic uint8 state)
  - Files: `src/features/FeatureBus.h`, `src/features/FeatureBus.cpp`

- [ ] **2.2** Implement `FFTProcessor`: wraps `juce::dsp::FFT` (order 11 = 2048-pt), Hann window, produces magnitude spectrum (1025 bins). Pre-allocates all buffers at construction.
  - Files: `src/analysis/FFTProcessor.h`, `src/analysis/FFTProcessor.cpp`

- [ ] **2.3** Implement `SpectralFeatures`: computes centroid, flux, flatness, rolloff from magnitude spectrum. Also computes 7-band energies from magnitude bins.
  - Files: `src/analysis/SpectralFeatures.h`, `src/analysis/SpectralFeatures.cpp`

- [ ] **2.4** Implement `OnsetDetector`: wraps `aubio_onset_t`. Feeds audio per-hop, returns onset flag + strength. Configure with spectral flux method, adaptive threshold.
  - Files: `src/analysis/OnsetDetector.h`, `src/analysis/OnsetDetector.cpp`
  - Dependencies: Add Aubio to `CMakeLists.txt`

- [ ] **2.5** Implement `BPMTracker`: wraps `aubio_tempo_t`. Feeds audio per-hop, returns BPM + beat phase (0–1 sawtooth between beats).
  - Files: `src/analysis/BPMTracker.h`, `src/analysis/BPMTracker.cpp`

- [ ] **2.6** Implement `MFCCExtractor`: Mel filterbank (40 bands, 20–8000 Hz) on magnitude spectrum → log → DCT-II → 13 coefficients.
  - Files: `src/analysis/MFCCExtractor.h`, `src/analysis/MFCCExtractor.cpp`

- [ ] **2.7** Implement `ChromaExtractor`: maps FFT magnitude bins to 12 pitch classes (C through B). Normalize to sum=1.
  - Files: `src/analysis/ChromaExtractor.h`, `src/analysis/ChromaExtractor.cpp`

- [ ] **2.8** Implement `KeyDetector`: Krumhansl-Schmuckler algorithm — correlate 12-element chroma profile with 24 key templates (12 major + 12 minor), report best match.
  - Files: `src/analysis/KeyDetector.h`, `src/analysis/KeyDetector.cpp`

- [ ] **2.9** Implement `LoudnessAnalyzer`: K-weighting (two biquad stages) → 400ms window RMS → LUFS. Also compute dynamic range (crest factor).
  - Files: `src/analysis/LoudnessAnalyzer.h`, `src/analysis/LoudnessAnalyzer.cpp`

- [ ] **2.10** Implement `StructuralDetector`: multi-scale EMA envelopes (100ms, 1s, 4s, 16s), compare short vs long to detect buildup (rising), drop (spike after buildup), breakdown (falling). State machine output.
  - Files: `src/analysis/StructuralDetector.h`, `src/analysis/StructuralDetector.cpp`

- [ ] **2.11** Implement pitch detection: either YIN (custom) or `aubio_pitch_t` wrapper. Report dominant pitch Hz + confidence.
  - Files: `src/analysis/PitchTracker.h`, `src/analysis/PitchTracker.cpp`

- [ ] **2.12** Implement transient density: sliding window onset counter (onsets per second over last 2 seconds).
  - Update: `src/analysis/AnalysisThread.cpp`

- [ ] **2.13** Implement harmonic change detection function (HCDF): frame-to-frame Euclidean distance in chroma space.
  - Update: `src/analysis/ChromaExtractor.h/cpp` or new file

- [ ] **2.14** Upgrade `AnalysisThread` to run full feature pipeline: FFT → spectral features → onset → BPM → MFCC → chroma → key → LUFS → structural → pitch. Publish complete `FeatureSnapshot` to `FeatureBus`.
  - Update: `src/analysis/AnalysisThread.cpp`

- [ ] **2.15** Implement `Smoother` class: EMA with configurable alpha, One-Euro filter with adaptive cutoff. Used by render thread for display smoothing.
  - Files: `src/features/Smoother.h`, `src/features/Smoother.cpp`

- [ ] **2.16** Implement `SpectrumDisplay` component: bar graph of magnitude spectrum or 7-band energies, painted at 30fps.
  - Files: `src/ui/SpectrumDisplay.h`, `src/ui/SpectrumDisplay.cpp`

- [ ] **2.17** Expand `AudioReadoutPanel` to show all features: RMS, peak, LUFS, centroid, flux, flatness, BPM, beat phase, onset indicator, key, structural state, MFCC summary, band meters.
  - Update: `src/ui/AudioReadoutPanel.h`, `src/ui/AudioReadoutPanel.cpp`

- [ ] **2.18** Write unit tests: ring buffer (concurrent push/pop), spectral features (sine wave → expected centroid), feature bus (triple buffer correctness), smoother (convergence).
  - Files: `tests/CMakeLists.txt`, `tests/test_ring_buffer.cpp`, `tests/test_spectral_features.cpp`, `tests/test_feature_bus.cpp`, `tests/test_smoother.cpp`

- [ ] **2.19** Integration test: play a known audio file, verify feature values are in expected ranges (e.g., 440Hz sine → centroid ~440Hz, single band peak).

---

## Milestone 3: OpenGL Image Rendering + First Effects

**Goal**: Load an image, display it via OpenGL, apply 4 shader effects (one from each category) driven by hardcoded audio mappings. Proves the full pipeline: audio → analysis → features → shader uniforms → visual output.

**Definition of Done**:
- User can load an image (PNG/JPEG) which displays in the center preview panel via OpenGL
- 4 effects work: Ripple (warp), Hue Shift (color), RGB Split (glitch), Vignette (blur/post)
- Effects visibly react to audio (hardcoded mappings for demo)
- 60fps rendering without dropped frames
- Effect chain renders correctly (ping-pong FBO)

### Tasks

- [ ] **3.1** Implement `Renderer`: JUCE `OpenGLRenderer` subclass. Creates OpenGL context on component, manages render loop. Fullscreen quad VAO/VBO.
  - Files: `src/render/Renderer.h`, `src/render/Renderer.cpp`, `src/render/FullscreenQuad.h`, `src/render/FullscreenQuad.cpp`

- [ ] **3.2** Implement `ShaderManager`: compile vertex + fragment shaders, link program, cache uniform locations, support hot-reload from `shaders/` directory.
  - Files: `src/render/ShaderManager.h`, `src/render/ShaderManager.cpp`

- [ ] **3.3** Implement `TextureManager`: load image file → OpenGL texture (GL_TEXTURE_2D, RGBA8). Handle different image sizes. Also create FBO textures for effect chain.
  - Files: `src/render/TextureManager.h`, `src/render/TextureManager.cpp`

- [ ] **3.4** Write shared vertex shader: fullscreen quad with UV coordinates.
  - Files: `shaders/passthrough.vert`

- [ ] **3.5** Write first 4 fragment shaders:
  - `shaders/ripple.frag` — UV distortion via sin(distance * freq + time * speed) * intensity
  - `shaders/hue_shift.frag` — RGB→HSV, rotate H, HSV→RGB
  - `shaders/rgb_split.frag` — Sample R/G/B at offset UV positions
  - `shaders/vignette.frag` — Darken edges based on distance from center
  - Files: 4 `.frag` files in `shaders/`

- [ ] **3.6** Implement `Effect` and `EffectChain`: each Effect owns a shader program + parameter list. EffectChain manages ordered list + ping-pong FBOs (render effect N reading texture from FBO A, writing to FBO B, then swap).
  - Files: `src/effects/Effect.h`, `src/effects/Effect.cpp`, `src/effects/EffectChain.h`, `src/effects/EffectChain.cpp`

- [ ] **3.7** Implement `UniformBridge`: takes an Effect's current parameter values and calls `glUniform1f` for each. Also uploads `u_time` and `u_resolution`.
  - Files: `src/effects/UniformBridge.h`, `src/effects/UniformBridge.cpp`

- [ ] **3.8** Implement `PreviewPanel`: JUCE Component that hosts the OpenGL context (via `Renderer`). Sizes to fill center area. Renders loaded image through effect chain.
  - Files: `src/ui/PreviewPanel.h`, `src/ui/PreviewPanel.cpp`

- [ ] **3.9** Add hardcoded demo mappings: RMS → ripple intensity, spectral centroid → hue shift amount, onset strength → RGB split amount, bass energy → vignette intensity. Wire in `Renderer::renderOpenGL()`.

- [ ] **3.10** Test: load an image, play audio, verify all 4 effects respond to audio features visually. Check 60fps with no stutter.

---

## Milestone 4: Mapping Engine + Full Effects Library

**Goal**: Implement the complete mapping system (any feature → any effect parameter with curves and smoothing) and all 17 shader effects. The user can create, edit, and delete mappings.

**Definition of Done**:
- All 17 effects from the architecture doc work as GLSL shaders
- `MappingEngine` routes any audio feature to any effect parameter
- All 5 curve types work (linear, exponential, logarithmic, s-curve, stepped)
- Per-mapping smoothing (EMA with configurable alpha)
- Mappings can be created/edited/deleted via UI
- Multiple mappings can target the same effect parameter (summed)
- Multiple effects can be active simultaneously in a chain

### Tasks

- [ ] **4.1** Implement `MappingEngine`: stores list of `Mapping` structs, processes them each frame (extract source → normalize → curve → range → smooth → write to target). Pre-allocates smoother state per mapping.
  - Files: `src/mapping/MappingEngine.h`, `src/mapping/MappingEngine.cpp`, `src/mapping/MappingTypes.h`

- [ ] **4.2** Implement `CurveTransforms`: linear, exponential (x^p), logarithmic (log(1+9x)/log10), s-curve (smoothstep), stepped (floor(x*N)/N). Pure functions, no state.
  - Files: `src/mapping/CurveTransforms.h`

- [ ] **4.3** Write remaining 13 fragment shaders:
  - Warp: `bulge.frag`, `wave.frag`, `liquid.frag`
  - Color: `saturation.frag`, `brightness.frag`, `duotone.frag`, `chromatic_aberration.frag`
  - Glitch: `pixel_scatter.frag`, `block_glitch.frag`, `scanlines.frag`
  - Blur: `gaussian_blur.frag`, `zoom_blur.frag`, `shake.frag`
  - Files: 13 `.frag` files in `shaders/`

- [ ] **4.4** Implement `EffectLibrary`: registry that creates `Effect` instances from shader files. Each entry defines name, category, parameter names/defaults.
  - Files: `src/effects/EffectLibrary.h`, `src/effects/EffectLibrary.cpp`

- [ ] **4.5** Integrate `MappingEngine` into render loop: after acquiring `FeatureSnapshot`, run all mappings to update effect parameters, then render effect chain.
  - Update: `src/render/Renderer.cpp`

- [ ] **4.6** Implement `MappingEditor` UI: dropdown for source feature, dropdown for curve type, sliders for input/output range and smoothing. "Add Mapping" button per effect parameter.
  - Files: `src/ui/MappingEditor.h`, `src/ui/MappingEditor.cpp`

- [ ] **4.7** Implement `EffectsRackPanel`: list of active effects with enable/disable toggle, parameter knobs showing current value, mapping indicator per knob, drag-to-reorder for effect chain order, "Add Effect" dropdown.
  - Files: `src/ui/EffectsRackPanel.h`, `src/ui/EffectsRackPanel.cpp`

- [ ] **4.8** Write unit tests for mapping engine: verify each curve type output, verify normalization, verify smoothing convergence, verify multiple mappings to same target.
  - Files: `tests/test_mapping_engine.cpp`

- [ ] **4.9** Integration test: create a mapping from RMS → ripple intensity with exponential curve, play audio, verify visual response matches expected exponential behavior.

---

## Milestone 5: VJ-Style UI Polish + Presets

**Goal**: Complete VJ-style dark UI with all panels, knobs, meters, and preset save/load. The app looks and feels like a professional VJ tool.

**Definition of Done**:
- Dark theme with accent colors (neon cyan/magenta on dark gray)
- Left panel: all audio readouts, waveform, spectrum analyzer, band meters
- Center panel: live image preview at maximum size
- Right panel: effects rack with knobs, mapping indicators
- Mapping editor popup/panel
- Preset save/load (JSON): saves active effects, their parameters, and all mappings
- File chooser for audio and image with drag-and-drop support
- Keyboard shortcuts for play/pause/stop, effect toggle
- FPS counter and CPU load indicator

### Tasks

- [ ] **5.1** Implement custom `LookAndFeel`: dark background (#1a1a2e), accent colors, custom slider/knob rendering (rotary style), custom toggle buttons, panel borders.
  - Update: `src/ui/LookAndFeel.h`, `src/ui/LookAndFeel.cpp`

- [ ] **5.2** Implement rotary knob component for effect parameters with value label, name label, and mapping indicator (colored ring showing mapped source).
  - Files: `src/ui/Knob.h`, `src/ui/Knob.cpp`

- [ ] **5.3** Polish `AudioReadoutPanel`: formatted dB readouts, color-coded band meters (sub=red, bass=orange, mid=green, treble=blue), beat phase pulsing indicator, onset flash, structural state label.
  - Update: `src/ui/AudioReadoutPanel.h/cpp`

- [ ] **5.4** Polish `WaveformDisplay`: scrolling with configurable time window, peak hold lines, RMS fill underneath.
  - Update: `src/ui/WaveformDisplay.h/cpp`

- [ ] **5.5** Polish `SpectrumDisplay`: choose between bar graph (linear/log frequency) and waterfall/spectrogram. Smooth bar animation.
  - Update: `src/ui/SpectrumDisplay.h/cpp`

- [ ] **5.6** Implement `PresetManager`: save/load mapping presets as JSON. Serialize: active effects list, effect order, all parameter values, all mappings (source, target, curve, range, smoothing).
  - Files: `src/ui/PresetManager.h`, `src/ui/PresetManager.cpp`

- [ ] **5.7** Add drag-and-drop support for audio files and image files onto the window.
  - Update: `src/MainComponent.cpp`

- [ ] **5.8** Add keyboard shortcuts: Space = play/pause, Escape = stop, 1-9 = toggle effects, Cmd/Ctrl+S = save preset, Cmd/Ctrl+O = load preset.
  - Update: `src/MainComponent.cpp`

- [ ] **5.9** Add FPS counter and analysis CPU load indicator in top bar.
  - Update: `src/MainComponent.cpp`

- [ ] **5.10** Final layout pass: responsive resizing, minimum window size (1280x720), panel proportions (left 20%, center 50%, right 30%).
  - Update: `src/MainComponent.cpp`

- [ ] **5.11** Test full workflow: load song → load image → add effects → create mappings → see audio-reactive visuals → save preset → load preset → verify restored state.

---

## Milestone 6: Quality, Performance, Cross-Platform

**Goal**: Optimize performance, fix edge cases, test on Windows/Linux, prepare for distribution.

**Definition of Done**:
- No audio dropouts at 256-sample buffer size
- 60fps rendering with 8+ active effects on integrated GPU
- Graceful handling of: no audio device, corrupt audio file, corrupt image, GPU shader compile failure
- Builds and runs on macOS (arm64 + x86_64), Windows (MSVC), Linux (GCC)
- Basic CI pipeline (GitHub Actions)

### Tasks

- [ ] **6.1** Profile analysis thread: ensure all feature extraction completes within 2ms per hop. Identify and optimize bottlenecks (likely MFCC or structural detection).

- [ ] **6.2** Profile render thread: ensure full effect chain completes within 8ms. Optimize shader programs if needed (reduce texture lookups, simplify math).

- [ ] **6.3** Add graceful error handling: audio device disconnection → pause + notification, shader compile error → fallback to passthrough + error display, image load failure → show placeholder.

- [ ] **6.4** Add adaptive quality: if render time exceeds 12ms, disable heaviest effect or reduce FBO resolution.

- [ ] **6.5** Test and fix on Windows: WASAPI audio, MSVC compiler, Windows OpenGL drivers.

- [ ] **6.6** Test and fix on Linux: ALSA/JACK audio, GCC compiler, Mesa/NVIDIA OpenGL drivers.

- [ ] **6.7** Set up GitHub Actions CI: build matrix (macOS-arm64, macOS-x86_64, Windows-MSVC, Ubuntu-GCC). Compile + run unit tests.
  - Files: `.github/workflows/build.yml`

- [ ] **6.8** Final pass: fix all compiler warnings, run AddressSanitizer, run ThreadSanitizer, verify no data races.

---

## Summary Timeline

| Milestone | Focus | Key Deliverable |
|-----------|-------|-----------------|
| **M1** | Audio + Window | App plays audio, shows waveform + RMS |
| **M2** | Analysis Engine | All features extracted + displayed as readouts |
| **M3** | OpenGL + 4 Effects | Image renders with 4 audio-reactive effects |
| **M4** | Mapping + All Effects | Full mapping system + 17 effects |
| **M5** | VJ UI + Presets | Polished dark UI, knobs, presets, drag-drop |
| **M6** | Quality + Cross-Platform | Performance, error handling, CI, multi-OS |
