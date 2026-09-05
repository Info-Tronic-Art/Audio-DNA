# Features — Audio-DNA (RealTimeAudio)

Norm: v2 | Last audited: 2026-05-24 | SHA: 4ee10ad

> **Re-verified 2026-07-16 against source (7-lane audit, HEAD 9139dd4); counts: 135 effects / 108 sources / 22 REST endpoints.** Targeted corrections applied to the analysis-pipeline stage list (§2), the GenreSmoothing/One-Euro smoothing claims (§3, §16), SignalInspector (§7b), 3D/MilkDrop source param counts (§source census), and video BPM-sync/in-out ownership (§11). See `.harmony/APP-INVENTORY.md` for the full living surface inventory and consolidated FLAGGED (dead/ghost/stub) list.
>
> **Synced 2026-07-17 to Wave 0+1 reality:** 13 dead/ghost symbols deleted (UniformBridge, MappingSuggester, ChainedSignal, GenreSmoothing, OneEuroFilter, ProgrammingMode, SyphonInput, SpoutOutput, NdiOutput/NdiInput, shaders/ disk files, and orphaned setters/stubs); OSC subsystem now LIVE (UDP 8000, 11/11 callbacks); Syphon OUTPUT wired (build-flag-gated); persistence now COMPLETE (all model fields round-trip); `/api/set_bpm` wired; TopBar transport wired; waveform snapshot now seqlock (torn-read-free); 48kHz warn-only guard added; Prefs collapsed 8→3 tabs. Undo/redo remains a no-op (Wave 2).

<!-- C++20/JUCE/OpenGL desktop application. Framework gating:
     Security/Auth: N/A — local desktop app, no user accounts
     Deployment: N/A — local CMake build only
     Middleware: N/A — not a web app
     Scheduled Jobs: N/A
     Storage is filesystem-only (JSON presets, PNG snapshots, video files) -->

---

## 1. Audio I/O & Capture [R]
**What it does:** Manages audio input devices (mic, system audio, file playback), mono-downmixes incoming samples, and pushes them into a lock-free ring buffer for the analysis thread.

**Entry points:**
- `AudioEngine` class (src/audio/AudioEngine.h:10) — device management, file loading
- `AudioCallback::audioDeviceIOCallbackWithContext()` (src/audio/AudioCallback.h:7) — RT callback
- `RingBuffer` class (src/audio/RingBuffer.h:11) — SPSC lock-free buffer

**Implementation chain:**
1. `AudioEngine` initializes `AudioDeviceManager` + `AudioTransportSource` for file playback
2. OS delivers 128 samples at 48kHz to `AudioCallback` (every 2.67ms, RT priority)
3. `AudioCallback` mono-downmixes stereo to mono
4. `RingBuffer::push()` copies samples into SPSC ring buffer (16384 floats, ~341ms capacity)
5. Analysis thread pulls via `RingBuffer::pop()`

**Data flow:**
```
OS audio driver → AudioCallback (RT thread, <100μs) → SPSC RingBuffer → AnalysisThread
AudioTransportSource (file playback) → same callback path
```

**Dependencies & services:**
- JUCE 8: `AudioDeviceManager`, `AudioTransportSource`, `AudioIODeviceCallback`
- Supported formats via JUCE: WAV, AIFF, FLAC, MP3, OGG

**Storage:** N/A — no persistent audio storage

**Config:**
- Sample rate: 48kHz (hardcoded assumption throughout pipeline)
- Buffer size: 128 samples (OS-configured via AudioDeviceManager)
- Ring buffer: 16384 floats (hardcoded, power-of-two)

**Failure modes:**
- Audio device disconnected → AudioDeviceManager fires change callback (handled)
- Ring buffer overflow (producer faster than consumer) → oldest samples overwritten (handled, by design)
- Ring buffer underrun (consumer faster than producer) → pop returns false, analysis reuses last data (handled)
- Wrong sample rate assumption → pitch/timing drift (unhandled — assumes 48kHz)

**Test coverage:**
- `tests/test_ring_buffer.cpp` — push/pop, overflow, underrun, concurrent access
- Missing: AudioEngine device enumeration, file format edge cases, sample rate mismatch

**Gotchas:**
- Audio callback is sacred: zero heap allocation, zero mutexes, zero system calls, zero exceptions. Budget <100us. Violating this causes audio glitches.
- Ring buffer is power-of-two sized for branchless modular arithmetic (mask instead of modulo).
- Cache-line padding prevents false sharing between producer/consumer indices.
- `monoBuffer_` resize in `audioDeviceAboutToStart()` (AudioCallback.cpp:L49) may allocate on audio thread if vector grows — one-time at device start, but technically violates zero-alloc RT guarantee.
- Ring buffer full → `push()` returns 0, samples silently dropped with no backpressure or error reporting.
- `CombinedCallback` mic mode writes gain-scaled data to output buffers before silencing — uses output buffers as scratch space.
- `const_cast<float* const*>(inputChannelData)` at AudioEngine.h:L105 casts away const to reuse input as output pointer for mono downmix.

---

## 2. Audio Analysis Pipeline [R]
**What it does:** Extracts ~30 audio features (exposed downstream as 58 mapping sources / 40 FeatureSnapshot fields) from raw samples in real-time via a 14-stage pipeline (14 numbered compute stages in code, 13 profiled timing slots) running every 10.7ms (512-sample hop at 48kHz). Features are accessible as mapping sources and signals.

**Entry points:**
- `AnalysisThread` class (src/analysis/AnalysisThread.h:41) — dedicated thread
- `FeatureSnapshot` struct (src/analysis/FeatureSnapshot.h) — POD output, alignas(64)

**Implementation chain:**
1. `AnalysisThread::run()` — loop: pull 512 samples from ring buffer, maintain 2048-sample overlap window
2. Stage 1: Raw time-domain — RMS, peak, rms dB (no ZCR — never computed anywhere in src)
3. Stage 2: `FFTProcessor::process()` — 2048-pt FFT, Hann window → 1025 magnitude bins
4. Stage 3: `SpectralFeatures::process()` — centroid, flux, flatness, rolloff, 7-band energies
5. Stage 4: `OnsetDetector::process()` — Aubio spectral flux thresholding
6. Stage 5: `BPMTracker::process()` — Aubio tempo + downbeat + **phrase tracking** (bar count / phrase phase are computed inside this stage via `feedDownbeatFeatures`, NOT a separate late stage)
7. Stage 6: `MFCCExtractor::process()` — 40-band mel filterbank → log → DCT → 13 MFCCs
8. Stage 7: `ChromaExtractor::process()` — FFT bins → 12 pitch classes + **HCDF** (harmonic change is computed inside this Chroma stage, NOT a separate stage 12)
9. Stage 8: `KeyDetector::process()` — Krumhansl-Kessler: chroma x 24 key templates (**Key runs before Pitch**)
10. Stage 9: `PitchTracker::process()` — Aubio yinfft pitch detection
11. Stage 10: `LoudnessAnalyzer::process()` — K-weighted biquads + 400ms window → LUFS, dynamic range
12. Stage 11: Transient density — onset count in ~2.7s (256-hop) sliding window
13. Stage 12: `StructuralDetector::process()` — 4-scale EMA (0.1s/1s/4s/16s) → drop/buildup/breakdown state machine
14. Stage 13: `GenreDetector::process()` — 8-genre classification + energy state
15. Stage 14: `AdvancedAudioAnalyzer::process()` (P25) — sidechain pump, swing ratio, formant, resonance, reese bass
16. Publishes complete `FeatureSnapshot` to FeatureBus via triple-buffer swap

**Data flow:**
```
RingBuffer → 2048-sample window → FFT → [spectral|onset|BPM|MFCC|chroma|pitch|key|loudness|structural|genre|advanced] → FeatureSnapshot → FeatureBus
```

**Dependencies & services:**
- JUCE juce_dsp: FFT (uses vDSP on macOS, IPP if available)
- Aubio 0.4.9+: onset detection (`aubio_onset`), BPM tracking (`aubio_tempo`), pitch (`aubio_pitch`)

**Config:**
- FFT size: 2048 points (hardcoded)
- Hop size: 512 samples = 10.7ms at 48kHz (hardcoded)
- Mel bands: 40 (20-8kHz) for MFCC
- Genre smoothing: ~2s EMA + ~3s hysteresis
- BPM stabilization: range gate → confidence → octave correction → median filter → hysteresis

**Failure modes:**
- Aubio initialization failure → onset/BPM/pitch features return defaults (handled)
- Silence detection → BPM tracker holds last good BPM, phase free-runs (handled, P23)
- Analysis takes >10.7ms → missed hops, features update at lower rate (unhandled but 5x headroom)
- All Aubio objects pre-allocated at startup → zero allocation in steady state

**Test coverage:**
- `tests/test_spectral_features.cpp` — centroid, flux, flatness, rolloff, band energies, silent input
- `tests/test_integration_pipeline.cpp` — full pipeline with synthetic signals (sine, noise)
- `tests/test_bpm_stabilization.cpp` — BPM tracking accuracy, stability
- `tests/test_downbeat_detector.cpp` — bar phase, downbeat pattern detection (1 FLAKY: barPhase range test)
- Missing: GenreDetector accuracy, AdvancedAudioAnalyzer edge cases, MFCC validation, key detection accuracy

**Gotchas:**
- Analysis thread pre-allocates ALL buffers and Aubio objects at startup. Zero allocation in steady state. Adding `new`/`malloc` here causes RT jitter.
- Pipeline order matters — stages depend on prior results (e.g., chroma needs FFT, key needs chroma).
- BPM stabilization has multiple layers (range/confidence/octave/median/hysteresis) — debugging requires checking each layer.
- Genre detection uses ~2s smoothing — genre won't change instantly on track switch.
- barPhase test is flaky (test #95) — synthetic kick pattern timing sensitivity.
- **Aubio objects created without null checks** — `new_aubio_onset`, `new_aubio_tempo`, `new_aubio_pitch` can return nullptr on failure; used without null guards in constructors. Nullptr dereference in subsequent `process()` calls.
- **kSampleRate hardcoded to 48000** — all frequency calculations, BPM timing, K-weighting coefficients are wrong if device runs at 44100/96000. Wave 1-D added a warn-only startup guard (cerr + one-shot alert when device SR != 48000); the frequency math still assumes 48kHz (no resampling).
- **Waveform buffer torn read — FIXED 2026-07-17 (Wave 1-D)** — the count-release + non-atomic memcpy was replaced with a seqlock (reader retries on version change → strictly torn-read-free). Threaded regression test added (`tests/test_waveform_snapshot.cpp`).
- **PCM snapshot double-buffer has theoretical torn-read risk** under high contention between writer index swap and reader access.
- **Profiling bug** — `stageNames` array has 13 entries but `kNumStages = 14`. 14th timing slot accumulated but never logged.
- **SpectralFeatures adaptive normalization** — `fluxMax_`/`bandMaxEnergy_` decay via 0.9995 multiplier after silence; first frames after audio resume have exaggerated values. No reset mechanism.
- **OnsetDetector and BPMTracker use separate internal 1024-pt FFTs** — redundant with main 2048-pt FFTProcessor. Aubio's internal state cannot share the main FFT result.
- **One-frame lag**: Stage 5 (BPM) uses `prevHCDF_` and `prevStructuralState_` because chroma HCDF (stage 7) and structural (stage 11) aren't computed yet. Fundamental pipeline ordering constraint.
- **barPhase test is flaky** (test_downbeat_detector.cpp, test #95, tag `[downbeat][barphase]`). The "barPhase stays within zero to one range" test uses synthetic kick patterns where timing sensitivity causes intermittent range violations. Root cause: synthetic beat spacing drifts relative to the tracker's internal phase accumulation.
- **`quantum_` in LinkSync is a non-atomic `double`** (LinkSync.h:54). Read by the analysis thread (via `update()` called per-frame) and written by the UI thread (via `setQuantum()`). On x86-64 this is benign (aligned double reads/writes are atomic in practice), but it violates the C++ memory model — technically a torn-read risk on other architectures.


**Storage:** N/A — real-time in-memory processing

---

## 3. Feature Transport [R]
**What it does:** Transfers the complete FeatureSnapshot from analysis thread to render thread via lock-free triple-buffer atomic swap, with optional EMA smoothing.

**Entry points:**
- `FeatureBus` class (src/features/FeatureBus.h:21) — triple-buffer transport
- `Smoother` class (src/features/Smoother.h) — EMA filter (header-only; One-Euro variant removed Wave 0)

**Implementation chain:**
1. Analysis thread calls `FeatureBus::write(snapshot)` — writes to next buffer, atomic index swap
2. Render thread calls `FeatureBus::read()` — reads latest snapshot via atomic index read (~10ns)
3. `Smoother::process()` — applies EMA filter per-mapping for visual smoothness (the `OneEuroFilter` was removed from Smoother.h Wave 0; mapping/routing use the EMA `Smoother` only)

**Data flow:**
```
AnalysisThread → FeatureBus::write() → [atomic triple-buffer] → FeatureBus::read() → RenderThread
Per-mapping: raw value → Smoother (EMA alpha) → smooth value
```

**Dependencies & services:**
- std::atomic for lock-free index management

**Config:**
- Triple buffer: 3x FeatureSnapshot (alignas(64), cache-line aligned)
- Smoother defaults: per-mapping smoothing parameter

**Failure modes:**
- Multiple writes before read → reader gets latest only, intermediate snapshots dropped (by design)
- No new data → reader gets last consumed snapshot via `getLatestRead()` (handled)

**Test coverage:**
- `tests/test_feature_bus.cpp` — single write-read, multiple writes, latest-read, slot independence, writer doesn't clobber reader
- `tests/test_smoother.cpp` — convergence to constant input
- Missing: concurrent stress tests (actual multi-thread contention)

**Gotchas:**
- FeatureSnapshot must remain POD (no pointers, no vtable) for atomic swap correctness.
- Triple buffer means analysis can write every 10.7ms while render reads every 16.67ms — no contention.
- `getLatestRead()` returns the LAST consumed snapshot, not the latest available — different from `read()`.
- (OneEuroFilter removed from Smoother.h Wave 0 — its division-by-zero gotcha no longer applies.)
- **`FeatureSnapshot::clear()` uses `memset(this, 0, sizeof(*this))`** then manually sets non-zero defaults. Adding any non-trivial member (vtable, std::string) would break this.
- **`hasNewData()` uses relaxed memory order** — fine for single reader, but with multiple readers only one will successfully acquire.


**Storage:** N/A — real-time in-memory processing

---

## 4. Visual Effects System [R]
**What it does:** Provides 135 GLSL shader effects across 11 categories + 15 clip-to-clip transitions, with effect chains at per-clip, per-layer, and global levels. Supports ISF shader import.

**Entry points:**
- `EffectLibrary` class (src/effects/EffectLibrary.h:12) — effect registry
- `Effect` class (src/effects/Effect.h) — single effect with shader + params
- `EffectChain` class (src/effects/EffectChain.h:23) — ordered chain with ping-pong FBOs
- (`UniformBridge` removed Wave 0 — was a demo-mapping helper superseded by MappingEngine; effect uniforms upload directly in `EffectChain::render`)
- `ISFShaderLoader` (src/effects/ISFShaderLoader.h) — ISF import with GLSL 410 conversion
- `EmbeddedShaders.h` (src/render/EmbeddedShaders.h) — all shaders as inline strings

**Implementation chain:**
1. `EffectLibrary` registers all 135 effects + 15 transitions at startup
2. User drags effect from FX Browser → `EffectSlot` added to clip/layer/global chain
3. `EffectChain::render()` iterates effects, ping-ponging between FBO A and FBO B
4. For each effect: `EffectChain::render` uploads params as `glUniform1f`, shader renders to target FBO
5. Temporal effects bind `u_prev_frame` from per-layer temporal buffer
6. ISF shaders parsed from JSON metadata, wrapped with compatibility defines, converted to GLSL 410

**Data flow:**
```
Input texture → FBO A (Effect 1) → FBO B (Effect 2) → FBO A (Effect 3) → ... → output
Per-clip effects → transition blend → per-layer effects → layer transform → keying → accumulator → global effects
```

**Dependencies & services:**
- OpenGL 4.1 Core: shader compilation, FBOs, texture units
- GLSL 410: all fragment shaders

**Config:**
- All effect params normalized to [0, 1] — shader maps to internal range
- Shader uniforms: `u_[effectName]_[paramName]` pattern
- ISF import: Menu → Audio-DNA → Import ISF Shader

**Effect inventory — 333 parameters across 135 effects (11 categories):**

| Category | Effects | Parameters | Avg Params/Effect |
|----------|---------|------------|-------------------|
| warp | 27 | 67 | 2.5 |
| color | 31 | 64 | 2.1 |
| pattern | 19 | 50 | 2.6 |
| glitch | 15 | 40 | 2.7 |
| animation | 6 | 23 | 3.8 |
| blur | 10 | 21 | 2.1 |
| 3d | 9 | 20 | 2.2 |
| time | 6 | 14 | 2.3 |
| composite | 3 | 13 | 4.3 |
| audio | 4 | 12 | 3.0 |
| blend | 5 | 9 | 1.8 |
| **Total** | **135** | **333** | **2.5** |

<details>
<summary>Effects by category (click to expand)</summary>

**Warp (27 effects, 67 params):**
Ripple (3), Bulge (3), Wave (3), Liquid (2), Kaleidoscope (2), Fisheye (1), Swirl (2), Polar Coords (1), Twirl (2), Shear (2), Elastic Bounce (2), Ripple Pond (2), Diamond Distort (2), Barrel Distort (1), Sine Grid (2), Glitch Displace (2), Quad Mirror (2), Flip (2), Warp Field (3), Slide Wrap (2), Tile Grid (4), Spot Zoom (6), Bendoscope (3), UV Remap (3), Liquid Morph (3), Zoom Warp (4), Density Wave (3)

**Color (31 effects, 64 params):**
Hue Shift (1), Saturation (1), Brightness (1), Duotone (7), Chromatic Aberration (2), Invert (1), Posterize (1), Color Shift (3), Thermal (1), Contrast (2), Sepia (1), Cross Process (1), Split Tone (3), Color Halftone (2), Dither (2), Heat Map (1), Selective Color (2), Film Grain (2), Gamma Levels (3), Solarize (2), Greyscale (2), Threshold (2), Exposure (1), Vibrance (1), Auto Mask (3), Chroma Key (4), Palette Remap (3), Color Grade (1), Pitch Chromatic Shift (3), Key Palette (3), Chroma Dissolve (2)

**Pattern (19 effects, 50 params):**
CRT (2), VHS (2), ASCII Art (2), Dot Matrix (2), Crosshatch (2), Emboss (2), Oil Paint (1), Pencil Sketch (2), Voronoi Glass (2), Cross Stitch (2), Night Vision (1), Triangulate (2), Neon Edge (4), Cartoon Ink (4), Pop Raster (4), Brush Strokes (4), Bump Light (4), Monitor Wall (4), Topographic Lines (4)

**Glitch (15 effects, 40 params):**
Pixel Scatter (2), RGB Split (2), Block Glitch (2), Scanlines (2), Digital Rain (2), Noise (2), Mirror (2), Pixelate (1), Pixel Explosion (4), Color Flash (5), Fragment Burst (4), Signal Destroy (3), Rhythm Slice (3), Data Corruption (3), Glitch Sort (3)

**Animation (6 effects, 23 params):**
Strobe (2), Pulse (2), Slit Scan (2), Point Zoom (8), Directional Feedback (6), Transient Flash (3)

**Blur (10 effects, 21 params):**
Gaussian Blur (1), Zoom Blur (3), Shake (2), Vignette (2), Motion Blur (2), Glow (2), Edge Detect (1), Sharpen (2), Edge Blur (2), Drop Shadow (4)

**3D (9 effects, 20 params):**
Perspective Tilt (2), Cylinder Wrap (2), Sphere Wrap (1), Tunnel (2), Page Curl (2), Parallax Layers (2), Dot Field (3), Luminance Terrain (3), Voxel Matrix (3)

**Time (6 effects, 14 params):**
Echo (2, temporal), Posterize Time (2, temporal), Freeze (1, temporal), Screen Split (4), Frame Stutter (2), Channel Delay (3, temporal)

**Composite (3 effects, 13 params):**
Line Cloner (5), Radial Cloner (4), Cube Scatter (4)

**Audio (4 effects, 12 params):**
Harmonic Displacement (3), Timbral Mosaic (3), Structural Morph (3), Beat Ripple (3)

**Blend (5 effects, 9 params):**
Double Exposure (2), Frosted Glass (2), Prism (2), Rain on Glass (2), Hexagonalize (1)

</details>

**Failure modes:**
- Shader compilation failure → effect not registered, logged (handled)
- Invalid uniform name → `glGetUniformLocation` returns -1, silently ignored (handled)
- Effect display name vs shader key mismatch → resolve via `EffectLibrary::getEffectDef(displayName)->shaderName` (CRITICAL — Pitfall #1)

**Test coverage:**
- `tests/visual/test_effects.py` — auto-discovers all 112 effects, verifies param changes output
- `tests/visual/test_range_quality.py` — 11-position sweep, 70%+ useful range, no dead zones
- Missing: ISF import edge cases, effect chain ordering bugs, temporal effect two-path validation

**Gotchas:**
- `Clip::EffectSlot::effectName` stores DISPLAY name ("Ripple"), shaders compiled under snake_case ("ripple"). Always resolve via `EffectLibrary::getEffectDef()`. Never use display name as shader key.
- Effect defaults must be VISIBLE on first add (0.3-0.7 for primary param). Defaults at 0.0 make effects invisible.
- Temporal effects need BOTH render paths (EffectChain + CompositorEngine). Fixing only one path = silent failure in the other.
- Multi-select FX drag drops comma-separated names — must split on commas.
- Parameter ranges often need nonlinear remapping in shader (`mix(0.82, 0.995, slider)`) for perceptually linear control.


**Storage:** N/A — real-time in-memory processing

### 4a. FX Browser (UI)

**What it does:** Displays all 135 effects organized by 11 categories in a scrollable, searchable list. Effects can be dragged onto clip cells in the DeckView or into the EffectStackView in inspectors. Supports multi-select and category collapse/expand.

**Key source files:**
- `FXBrowser` class (src/ui/FXBrowser.h:11, src/ui/FXBrowser.cpp)

**Parent component:** BrowserPanel (tab index 1: "FX")

**Controls & interactions:**
- Search field at top — filters effects by name
- Category headers (collapsible) — click to expand/collapse
- Effect rows — click to select, drag to apply to clip cell or effect stack
- Multi-select via Shift/Cmd click
- Double-click or drag triggers `onEffectActivated` callback

**Gotchas:**
- `setEffectLibrary()` must be called before `refresh()` — otherwise the list is empty.
- Effect names in the browser are DISPLAY names; shader resolution requires `EffectLibrary::getEffectDef()`.
- Multi-select drag drops comma-separated names (same pattern as deck cell drops).

---

## 5. Render Pipeline [R]
**What it does:** Manages the OpenGL 4.1 rendering loop: shader compilation, texture management, deck/layer compositing with per-level effect chains, temporal buffers, feedback system, and frame ring buffer.

**Entry points:**
- `Renderer` class (src/render/Renderer.h:37) — OpenGLRenderer impl, frame loop
- `ShaderManager` class (src/render/ShaderManager.h:11) — compile, link, hot-reload
- `TextureManager` class (src/render/TextureManager.h) — image → GL texture, FBO textures
- `CompositorEngine` class (src/render/CompositorEngine.h:27) — deck/layer compositing
- `FullscreenQuad` class (src/render/FullscreenQuad.h) — VAO/VBO for fullscreen triangle strip

**Implementation chain:**
1. `Renderer::renderOpenGL()` called every VSync (~16.67ms)
2. Reads latest FeatureSnapshot from FeatureBus (atomic, ~10ns)
3. `SignalRegistry::evaluateAll()` + `RoutingEngine::processFrame()` — evaluate all signals
4. For single-image mode: `EffectChain::render()` with ping-pong FBOs
5. For deck mode: `CompositorEngine::compositeDeck()`:
   a. Per clip: load texture → apply clip effects → transition blend
   b. Per layer: apply layer effects → transform → keying → blend onto accumulator
   c. Persistent layers from non-active decks composited after active deck
   d. Global effects via `effectChain_` on final output
6. Temporal buffers: per-layer `layerTemporalBuffers_` for `u_prev_frame`
7. Frame ring buffer: 480 frames at 1/4 resolution for Screen Split / Frame Stutter
8. Feedback: `FeedbackProcessor` per-layer Larsen loop (6 presets)
9. Composition transform: position/scale/rotation on final output
10. Swap buffers (VSync present)

**Data flow:**
```
FeatureSnapshot → MappingEngine → effect params → EffectChain::render → glUniform
Clip texture → per-clip FX → transition → per-layer FX → transform → key → blend → accumulator
Accumulator → global FX → composition transform → swap buffers → display
```

**Dependencies & services:**
- OpenGL 4.1 Core (macOS cap): FBOs, VAOs, VBOs, shaders, textures
- JUCE juce_opengl: `OpenGLRenderer`, `OpenGLContext`

**Config:**
- Target: 60fps (VSync)
- Render budget: <8ms for full chain
- Frame ring buffer: 480 frames, 1/4 resolution (~240MB VRAM at 1080p)
- FBO count: ping-pong pair (effectFBO_A/B) + scratchFBO + transitionFBO + temporal buffers + feedback FBOs

### 5a. Clip-to-Clip Transitions

**What it does:** Blends between the outgoing and incoming clip textures during a clip change, using one of 30 transition types defined in the `MixMode` enum. 15 of the 30 types have dedicated GLSL shaders; the remaining 15 fall back to crossfade dissolve. Each layer has its own transition mode (the "F dropdown") independent of its persistent blend mode (the "V dropdown").

**Key source files:**
- `Layer::MixMode` enum, transition subset (src/model/Layer.h:77-92) — 30 transition entries
- `Layer::transitionMode`, `transitionSpeed`, `crossfadeProgress`, `previousClipColumn` (src/model/Layer.h:124-156) — per-layer transition state
- `CompositorEngine::applyTransition()` (src/render/CompositorEngine.cpp:1090) — shader-based blending
- `CompositorEngine::getTransitionShaderName()` (src/render/CompositorEngine.cpp:1065) — enum-to-shader mapping
- `EmbeddedShaders::transition*` (src/render/EmbeddedShaders.h) — 15 transition fragment shaders
- `Renderer` shader compilation (src/render/Renderer.cpp:1324-1338) — compiles all 15 transition shaders at startup

**Transition inventory (30 enum entries, 15 with dedicated shaders):**

| Category | Transition Types | Shader-Mapped |
|----------|-----------------|---------------|
| Instant | Cut | Yes (transition_cut) |
| Fade | Dissolve (default) | Yes (transition_dissolve) |
| Directional Wipe | WipeLeft, WipeRight, WipeUp, WipeDown, WipeEllipse, WipeDiagonal | 5 of 6 (WipeDiagonal falls back to dissolve) |
| Push | PushLeft, PushRight, PushUp, PushDown | All 4 |
| Zoom | ZoomIn, ZoomOut | Both |
| 3D Rotation | RotateX, RotateY, Spin, Cube, Flip, Fold | 1 of 6 (Flip only; others fall back to dissolve) |
| Fade Through Color | ToBlack, ToWhite | 1 of 2 (ToBlack only; ToWhite falls back to dissolve) |
| Creative/VJ | Pixelate, Blur, Noise, RGBSplit, GlitchBlocks, Strobe, Slide, Stretch, Displace | 0 of 9 (all fall back to dissolve) |

**Implementation details:**
1. Clip trigger sets `crossfadeProgress = 0.0` and saves `previousClipColumn` (Layer.h:238-240)
2. Each frame, `compositeDeck()` advances progress: `step = (1/60) / transitionSpeed` (CompositorEngine.cpp:686-691)
3. `applyTransition()` renders both clips (new + previous with effects applied) into `transitionFBO_` using the selected shader
4. Shader receives `u_texture` (new clip), `u_prevTexture` (old clip), and `u_crossfadeProgress` [0,1]
5. When `crossfadeProgress >= 1.0`, transition completes and `previousClipColumn` resets to -1

**Config:**
- `transitionSpeed`: per-layer, in seconds. -1.0 = instant cut (no crossfade). Default fallback: 0.5s
- `transitionMode`: per-layer MixMode enum value, selected via F dropdown in UI
- `transitionBlendMode`: separate per-layer field for transition blend method (Layer.h:125)
- `globalTransitionSpeed`: composition-level default (Composition.h:48), 0.3s

**Behavioral notes:**
- Transitions render into a dedicated `transitionFBO_` to avoid conflicts with `scratchFBO_` (used by keying) and `effectFBO_A_/B_` (used by effect chains)
- Previous clip effects are applied during transition (`applyClipEffects` called on prevClip at CompositorEngine.cpp:1110)
- 15 Creative/VJ and 3D transition enum entries exist as placeholders — selecting them produces a dissolve until shaders are implemented
- Instant cut (`transitionSpeed <= 0` or `transitionSpeed == -1`) sets `crossfadeProgress = 1.0` immediately, skipping the blend entirely
- **Transition shader coverage: 15 of 30 enum entries have dedicated shaders.** Shader-mapped: Cut, Dissolve, WipeLeft/Right/Up/Down, WipeEllipse (iris), PushLeft/Right/Up/Down, ZoomIn/Out, Flip, ToBlack. The remaining 15 (WipeDiagonal, RotateX/Y, Spin, Cube, Fold, ToWhite, Pixelate, Blur, Noise, RGBSplit, GlitchBlocks, Strobe, Slide, Stretch, Displace) fall back to dissolve via the `default:` case in `getTransitionShaderName()` (CompositorEngine.cpp:1083). See Renderer.cpp:1324-1338 for compiled shaders.
- **No test coverage** for transitions — no test file exercises `applyTransition()` or validates per-mode shader output.

### 5b. Keying & Masking System

**What it does:** Applies per-layer alpha keying to control how Transparent and Mask layers composite over layers below. 13 keying modes extract or manipulate alpha from source content using dedicated GLSL shaders, with configurable threshold, softness, and chroma key parameters.

**Key source files:**
- `Layer::KeyingMode` enum (src/model/Layer.h:97-102) — 13 keying modes
- `Layer::keyThreshold`, `keySoftness`, `chromaKey*` fields (src/model/Layer.h:104-107) — per-layer keying parameters
- `CompositorEngine::applyLayerKeying()` (src/render/CompositorEngine.cpp:903) — keying shader dispatch
- `CompositorEngine::applyMaskLayer()` (src/render/CompositorEngine.h:232) — Mask layer type application

**Keying mode inventory (13 modes, `Layer::KeyingMode` enum):**

| Mode | Shader Key | Algorithm |
|------|-----------|-----------|
| Alpha | key_alpha | Pass through source alpha channel as-is |
| LumaKey | key_luma | Luminance-based: dark pixels become transparent (`threshold`/`softness` control cutoff) |
| InvertedLumaKey | key_inv_luma | Inverse of LumaKey: bright pixels become transparent |
| LumaIsAlpha | key_luma_alpha | Source luminance directly replaces alpha (grayscale map) |
| InvertedLumaIsAlpha | key_inv_luma_alpha | Inverted luminance replaces alpha |
| ChromaKey | key_chroma | Green/blue screen removal: distance from `chromaKeyR/G/B` color within `chromaKeyTolerance` |
| MaxRGB | key_max_rgb | Maximum of R/G/B channels determines alpha |
| SaturationKey | key_saturation | Color saturation determines alpha (desaturated = transparent) |
| EdgeDetection | key_edge | Edge-detected content becomes alpha (Sobel or similar, uses `u_resolution`) |
| ThresholdMask | key_threshold | Binary alpha: above threshold = opaque, below = transparent |
| ChannelR | key_channel_r | Red channel value used as alpha |
| ChannelG | key_channel_g | Green channel value used as alpha |
| ChannelB | key_channel_b | Blue channel value used as alpha |

**Implementation details:**
1. Keying applies after clip effects and layer transform, before blending onto the accumulator
2. For Transparent layers: `applyLayerKeying()` renders into `scratchFBO_`, then `blendLayerOntoAccumulator()` composites the keyed result (CompositorEngine.cpp:764-767)
3. For Mask layers: `applyMaskLayer()` uses content as a luminance mask applied to the accumulator
4. Each keying mode maps to a dedicated GLSL shader via switch statement (CompositorEngine.cpp:917-931)
5. If the keying shader fails to compile, falls back to `passthrough` (no keying, full opacity)

**Config (per-layer, src/model/Layer.h:103-107):**
- `keyingMode`: KeyingMode enum, default Alpha
- `keyThreshold`: [0,1], controls cutoff point for luminance/threshold modes. Default 0.1
- `keySoftness`: [0,1], controls edge softness for gradual transparency transitions. Default 0.1
- `chromaKeyR/G/B`: target color for chroma keying. Default green (0.0, 1.0, 0.0)
- `chromaKeyTolerance`: [0,1], how far from target color to key out. Default 0.2

**Uniforms uploaded to keying shaders:**
- `u_texture` (sampler2D) — source texture
- `u_opacity` (float) — layer opacity
- `u_threshold` (float) — keying threshold
- `u_softness` (float) — edge softness
- `u_chroma_key_color` (vec3) — chroma key target color
- `u_chroma_tolerance` (float) — chroma key tolerance
- `u_resolution` (vec2) — viewport resolution (used by EdgeDetection)

**Behavioral notes:**
- Keying only applies to Transparent and Mask layer types. Opaque layers write directly to the accumulator without keying
- The `scratchFBO_` is shared between keying passes but NOT with transitions (which use `transitionFBO_`) or effects (which use `effectFBO_A_/B_`)
- `dryWetMix` on FX Only layers (Layer.h:110) is separate from keying — it controls effect intensity on the accumulator, not alpha
- All keying parameters are uploaded every frame regardless of mode (e.g., chroma uniforms sent even for LumaKey) — the shader ignores irrelevant uniforms
- **Keying shader coverage: only 3 of 13 modes have dedicated shaders.** `key_alpha` (transparencyAlpha), `key_luma` (transparencyLumaKey), and `key_chroma` (transparencyChromaKey) are real implementations. `key_max_rgb` and `key_luma_alpha` reuse `transparencyLight`. `key_inv_luma` and `key_saturation` reuse `transparencyLumaKey` (no actual inversion or saturation logic). The remaining 6 modes (InvertedLumaIsAlpha, EdgeDetection, ThresholdMask, ChannelR/G/B) all fall back to `transparencyAlpha` (plain alpha passthrough) — selecting these modes in the UI has no visual effect beyond passthrough. See Renderer.cpp:1109-1124 for the full mapping.
- **No test coverage** for any keying mode — no test file exercises `applyLayerKeying()` or validates shader outputs per mode.

### 5c. Master Opacity & Composition Transform

**What it does:** Applies composition-level global controls to the final rendered output: master level (brightness/dim/blackout) and spatial transform (position, scale, rotation, anchor point). These are the last processing stages before frame presentation.

**Key source files:**
- `Composition::masterOpacity`, `compPositionX/Y`, `compScale`, `compRotation`, `compAnchorX/Y` (src/model/Composition.h:23-45) — composition-level fields
- `Renderer::applyCompTransform()` (src/render/Renderer.cpp:1600) — spatial transform application
- `Renderer::setMasterLevel()` / `masterLevel_` (src/render/Renderer.h:205, 252) — atomic brightness control
- Master level blend pass (src/render/Renderer.cpp:561-591) — GL constant-color multiply
- `EmbeddedShaders::compTransform` (src/render/EmbeddedShaders.h:69) — transform fragment shader

**Composition-level controls:**

| Field | Type | Default | Range | Description |
|-------|------|---------|-------|-------------|
| `masterOpacity` | float | 1.0 | [0,1] | Serialized to presets but distinct from `masterLevel_` (see notes) |
| `masterLevel_` | atomic\<float\> | 1.0 | [0,1] | Runtime brightness via `setMasterLevel()`. Applied as GL constant-color multiply |
| `compPositionX` | float | 0.0 | [-1,1] | Horizontal offset (normalized, -1=full left, 1=full right) |
| `compPositionY` | float | 0.0 | [-1,1] | Vertical offset (normalized) |
| `compScale` | float | 1.0 | >0 | Scale factor (1.0=100%, clamped to min 0.001 in shader) |
| `compRotation` | float | 0.0 | degrees | Rotation around anchor point (converted to radians in renderer) |
| `compAnchorX` | float | 0.0 | [-1,1] | Anchor point X offset from center |
| `compAnchorY` | float | 0.0 | [-1,1] | Anchor point Y offset from center |

**Implementation details:**

*Master Level (brightness/dim):*
1. `masterLevel_` is an `std::atomic<float>` — set from UI/MIDI thread, read on render thread (Renderer.h:252)
2. Applied after global effects and composition transform (Renderer.cpp:561)
3. Uses `GL_BLEND` with `glBlendFunc(GL_ZERO, GL_CONSTANT_COLOR)` — multiplies existing framebuffer by level value
4. `glBlendColor(level, level, level, 1.0f)` sets uniform RGB dim, preserving alpha
5. Skipped when level >= 0.99 (no-op optimization)

*Composition Transform (position/scale/rotation):*
1. Called after global effect chain, before master level (Renderer.cpp:468)
2. Early-out if all values are default (position ~0, scale ~1, rotation ~0) — checked with epsilon tolerance
3. Copies current framebuffer to `compTransformFBO_` via `glBlitFramebuffer`
4. Renders transform shader to default framebuffer: UV coordinates are centered, anchor-offset, rotated, scaled, repositioned
5. Pixels outside [0,1] UV bounds render as black (hard clip, no wrap)

*Transform shader pipeline (EmbeddedShaders.h:69-108):*
```
UV centered at origin → anchor offset → rotation (2D mat2) → inverse scale → undo anchor → position offset → back to [0,1] → sample or black
```

**Config:**
- `masterOpacity` serialized in presets via `Composition::toVar()` (Composition.h:167)
- Transform fields are NOT serialized in current `toVar()` implementation — runtime only
- `compTransformFBO_` allocated lazily on first use, resized to match viewport

**Behavioral notes:**
- `masterOpacity` (Composition.h:24) and `masterLevel_` (Renderer.h:252) are separate systems: `masterOpacity` is serialized to presets, `masterLevel_` is the runtime atomic used by the render thread. The render thread reads `masterLevel_`, not `masterOpacity` directly
- `masterSpeed` (Composition.h:25) is a global speed multiplier field but is NOT documented here as it affects playback timing, not visual compositing
- Composition transform is applied to the entire output including all layers, global effects, and persistent layers — it is truly the last spatial operation before master level and frame present
- Scale uses inverse mapping in shader (`uv /= scale`): scale > 1.0 zooms in (magnifies), scale < 1.0 zooms out (shrinks). Clamped to minimum 0.001 to prevent division by zero
- **Composition transform fields are NOT serialized (runtime-only).** `compPositionX/Y`, `compScale`, `compRotation`, `compAnchorX/Y` are absent from `Composition::toVar()`/`fromVar()`. Any transform adjustments are lost on preset save/load or application restart. Only `masterOpacity` is serialized.

**Failure modes:**
- GPU can't keep up → frame drops, VSync miss (unhandled — no adaptive quality)
- FBO creation failure → rendering falls back to default framebuffer (partially handled)
- Shader hot-reload failure → keeps previous compiled shader (handled)
- Frame ring buffer VRAM exhaustion → 1/4 downscale mitigates (handled by design)

**Test coverage:**
- `tests/visual/test_render_pipeline.py` — basic render pipeline via Eyes test server
- `tests/visual/test_performance.py` — render time within budget
- Missing: FBO lifecycle edge cases, compositor layer ordering, feedback loop stability

**Gotchas:**
- `scratchFBO_` used by keying conflicts with `effectFBO_A_/B_` used by effects. Transitions need own `transitionFBO_`.
- Layer ID 0 IS VALID (`Deck::initDefault()` assigns id=0). Never use `layerId > 0` as guard — disables features on most-used layer.
- VideoRecorder triple-buffer has NO mutex on GL thread — frames dropped if encoder can't keep up.
- Screen Split / Frame Stutter can't use normal shader pipeline — need ring buffer of N past frames, intercepted before normal rendering.
- Ring buffer at full resolution = 4GB VRAM. 1/4 downscale is essential.


**Storage:** N/A — real-time in-memory processing

---

## 6. Audio-Visual Mapping [R]
**What it does:** Routes any of 58 audio features to any effect parameter via a configurable pipeline: extract → normalize → curve → scale → smooth → write. Supports 24 curve types and genre-aware mapping suggestions.

**Entry points:**
- `MappingEngine` class (src/mapping/MappingEngine.h:22)
- `MappingTypes.h` (src/mapping/MappingTypes.h) — Mapping struct, Source/Curve enums
- `CurveTransforms.h` (src/mapping/CurveTransforms.h) — 24 curve functions
- (`MappingSuggester` removed Wave 0 — was a ghost: fully implemented AI mapping suggester, never instantiated, no UI/API caller)

**Implementation chain:**
1. User clicks "map" on any effect param → MappingEditor opens
2. Configure: source feature, curve type, input range, output range, smoothing
3. Each render frame: `MappingEngine::processAll(snapshot)`
4. For each active mapping: extract source → normalize to [0,1] → apply curve → scale to output range → smooth → write to target param
5. Multiple mappings can target same param (values summed)
6. (`MappingSuggester::suggest()` removed Wave 0 — was genre-aware recommendations, never reachable from UI/API)

### 6a. Mapping Sources (MappingSource enum — 58 entries)

58 entries in MappingSource enum (excluding Count sentinel), organized in 10 groups:

| Group | Count | Enum Range | Description |
|-------|-------|------------|-------------|
| Amplitude | 3 | 0-2 | RMS, Peak, RmsDB |
| Loudness | 3 | 3-5 | LUFS, DynamicRange, TransientDensity |
| Spectral | 4 | 6-9 | Centroid, Flux, Flatness, Rolloff |
| 7-Band Energies | 7 | 10-16 | BandSub through BandBrilliance |
| Onset/Rhythm | 6 | 17-22 | OnsetStrength, BeatPhase, BPM, BarPhase, PhrasePhase, BarCount |
| Structural | 1 | 23 | StructuralState |
| Pitch/Harmony | 4 | 24-27 | DominantPitch, PitchConfidence, DetectedKey, HarmonicChange |
| Timbral/MFCCs | 13 | 28-40 | MFCC0 through MFCC12 |
| Chroma | 12 | 41-52 | ChromaC through ChromaB (12 pitch classes) |
| Advanced/P25 | 5 | 53-57 | SidechainPump, SwingRatio, FormantPresence, ResonancePeak, ReeseBass |

### 6b. Curve Types (MappingCurve enum — 24 entries)

**Original curves (5):**

| # | Enum | Formula | Behavior |
|---|------|---------|----------|
| 0 | Linear | y = x | Direct 1:1 mapping |
| 1 | Exponential | y = x^2 | Emphasizes peaks, compresses lows |
| 2 | Logarithmic | y = log(1+9x)/log(10) | Compresses peaks, lifts lows |
| 3 | SCurve | y = x^2(3-2x) | Smoothstep, concentrates midrange |
| 4 | Stepped | y = floor(x*N)/N | Quantized to N steps (default 4) |

**P24 easing functions (19):**

| # | Enum | Family | Behavior |
|---|------|--------|----------|
| 5-7 | CircularIn/Out/InOut | Circular | Acceleration/deceleration via circular arc |
| 8-10 | BackIn/Out/InOut | Back | Overshoots then settles (c=1.70158) |
| 11-13 | ElasticIn/Out/InOut | Elastic | Spring oscillation with exponential decay |
| 14-16 | BounceIn/Out/InOut | Bounce | Ball-drop 4-stage parabolic arcs (n1=7.5625) |
| 17-19 | CubicIn/Out/InOut | Cubic | x^3 ease, sharper than exponential |
| 20-22 | SineIn/Out/InOut | Sine | Gentle sinusoidal acceleration/deceleration |
| 23 | Hold | Hold | Step function: 0 for all x<1.0, then 1.0 (gate/trigger) |

Dispatch: `CurveTransforms::applyCurve(int curveIndex, float x, int steppedN)` — switch on enum index. `steppedN` applies only to index 4 (Stepped).

**Data flow:**
```
FeatureSnapshot.field → normalize(inputMin/Max) → curve(24 types) → scale(outputMin/Max) → smooth(EMA) → Effect.param
```

**Dependencies & services:**
- No external dependencies — pure math

**Config:**
- Mapping source count: 58 (MappingSource enum, MappingTypes.h:6) (source: hardcoded)
- Curve type count: 24 (MappingCurve enum, MappingTypes.h:90) (source: hardcoded)
- Mapping struct defaults (MappingTypes.h:123):
  - `source`: MappingSource::RMS (source: hardcoded)
  - `curve`: MappingCurve::Linear (source: hardcoded)
  - `inputMin`: 0.0, `inputMax`: 1.0 (source: hardcoded)
  - `outputMin`: 0.0, `outputMax`: 1.0 (source: hardcoded)
  - `smoothing`: 0.15 — EMA alpha, higher = less smoothing (source: hardcoded)
- Stepped curve default N: 4 steps (MappingEngine.h:50, CurveTransforms.h:40) (source: hardcoded)
- Smoother default alpha: 0.3 (Smoother.h:12) (source: hardcoded)
- (OneEuroFilter removed from Smoother.h Wave 0 — its hardcoded defaults no longer exist.)
- Back easing overshoot: c1=1.70158, c2=c1*1.525, c3=c1+1.0 (CurveTransforms.h:76-78) (source: hardcoded)
- Elastic period: c4=2pi/3, c5=2pi/4.5 (CurveTransforms.h:106-107) (source: hardcoded)
- Bounce magic constants: n1=7.5625, d1=2.75 (CurveTransforms.h:139-140) (source: hardcoded)
- Logarithmic curve base: log(1 + 9x) / log(10) (CurveTransforms.h:29) (source: hardcoded)
- Multi-mapping accumulation clamp: [0, 1] (MappingEngine.cpp:214) (source: hardcoded)
- (MappingSuggester config removed Wave 0 — the class and its hardcoded defaults/activity-thresholds/relevance scores were deleted.)

**Failure modes:**
- Invalid source enum → default to 0.0 (handled)
- Target effect removed → orphaned mapping (unhandled — mapping still exists)
- Division by zero in normalize (inputMin == inputMax) → clamped (handled)

**Test coverage:**
- `tests/test_mapping_engine.cpp` — mapping creation, curve transforms, scaling, multi-mapping
- Missing: smoothing behavior, orphaned mapping cleanup

**Gotchas:**
- Smoothing state is per-mapping — not per-source. Two mappings from the same source can have different smoothing.
- (AI suggestions removed Wave 0 — MappingSuggester deleted.)
- **Pass 1 resets ALL targeted params to 0 before accumulation** — any param targeted by at least one mapping has its manual/preset value destroyed each frame.
- **`kSourceNames[]` in PresetManager MISSING P25 advanced sources** (SidechainPump, SwingRatio, etc.) — presets saved with P25 mappings fail to round-trip.
- **`Mapping.smoothing` is named "alpha" but behaves as EMA coefficient** — higher = LESS smoothing (1.0 = passthrough). Opposite of typical "smoothing" semantics.
- **`removeMapping()` uses vector erase** — invalidates all indices >= removed. No stable IDs for mappings.
- **`MappingSuggester` REMOVED 2026-07-17 (Wave 0)** — the ghost class (266 LOC, zero UI/API integration, never instantiated) was deleted. See Feature 22e.
- **`kSourceNames[]` in PresetManager is MISSING P25 advanced sources** — the array has 53 entries (RMS through ChromaB, indices 0-52) but the MappingSource enum has 58 entries (indices 53-57: SidechainPump, SwingRatio, FormantPresence, ResonancePeak, ReeseBass). Presets saved with P25 source mappings will serialize the enum integer, but `sourceToString()` returns "RMS" for out-of-range indices, and `stringToSource()` cannot match P25 names on load — mappings silently degrade to RMS.

### 6c. MappingEditor (UI)

**What it does:** A JUCE Component for configuring a single audio-to-visual Mapping. Provides dropdowns for source feature and curve type, sliders for input/output range and smoothing, enable/disable toggle, and delete button.

**Key source files:**
- `MappingEditor` class (src/ui/MappingEditor.h:18, src/ui/MappingEditor.cpp)

**Parent component:** Opened as a popup from EffectsRackPanel or UniversalParamControl when the user clicks "map" on an effect parameter.

**Controls & interactions:**
- Source feature dropdown (ComboBox) — selects which of 58 audio features drives the mapping
- Curve type dropdown (ComboBox) — selects from 24 curve types
- Input min/max sliders — normalize source range
- Output min/max sliders — scale target range
- Smoothing slider — EMA alpha (higher = less smoothing)
- Enable/Disable toggle — enables or disables the mapping without deleting it
- Delete button — removes the mapping entirely
- Randomize button — randomizes curve and range values
- Close button ("X") — closes the editor popup
- Uses a Listener interface (`mappingEditorChanged`, `mappingEditorDeleteRequested`, `mappingEditorCloseRequested`)

**Gotchas:**
- `setMapping()` copies values in; changes are communicated via Listener callbacks. The editor does not hold a pointer to the Mapping object.
- Source and curve dropdowns are populated from enum names at construction, not dynamically.


**Storage:** N/A — real-time in-memory processing

---

## 7. Signal Routing Engine [R]
**What it does:** Manages 32 named signals (8 visible + 21 hidden audio + 2 modulation + 1 clip position) and evaluates chained signal expressions each frame to feed the render pipeline.

**Entry points:**
- `SignalRegistry` class (src/signal/SignalRegistry.h:17) — named signal storage
- (`ChainedSignal` removed Wave 0 — was a ghost: signal-modulates-signal via math ops, never instantiated; SignalRegistry dynamic_cast wiring removed)
- `RoutingEngine` class (src/routing/RoutingEngine.h:14) — per-frame evaluation

**Implementation chain:**
1. `SignalRegistry::initDefaults()` registers 32 signals at startup
2. (`ChainedSignal` removed Wave 0 — derived-signal math was a ghost, never instantiated)
3. Each frame: `SignalRegistry::evaluateAll()` updates all signal values
4. `RoutingEngine::processFrame()` applies routing rules into the render pipeline
5. REST API exposes 5 signal/routing endpoints for external control

### 7a. Signal Inventory (32 registered signals)

**Visible Audio Signals (8):**

| Signal Name | MappingSource | What it measures |
|-------------|---------------|------------------|
| Volume | RMS | Root mean square — overall loudness |
| Sub Bass | BandSub | bandEnergies[0], ~20-60Hz |
| Bass | BandBass | bandEnergies[1], ~60-250Hz |
| Mid | BandMid | bandEnergies[3], ~500-2kHz |
| Air | BandBrilliance | bandEnergies[6], ~6-20kHz |
| Tempo | BPM | Detected beats per minute |
| Beat Position | BeatPhase | Phase within current beat [0, 1) |
| Hit | OnsetStrength | Transient detection strength |

**Hidden Audio Signals (21):**

| Signal Name | MappingSource | Group | What it measures |
|-------------|---------------|-------|------------------|
| Peak | Peak | Amplitude | Peak sample amplitude |
| Punch | DynamicRange | Amplitude | Dynamic range (peak minus RMS) |
| Hits Per Second | TransientDensity | Amplitude | Onset count in 2s window |
| Low Mid | BandLowMid | Bands | bandEnergies[2], ~250-500Hz |
| High Mid | BandHighMid | Bands | bandEnergies[4], ~2-4kHz |
| Presence | BandPresence | Bands | bandEnergies[5], ~4-6kHz |
| Hit Strength | OnsetStrength | Rhythm | Duplicate source with "Hit" |
| Bar Position | BarPhase | Rhythm | Phase within current bar [0, 1) |
| Phrase Position | PhrasePhase | Rhythm | Phase within current phrase |
| Bar Count | BarCount | Rhythm | Cumulative bar count |
| Brightness | SpectralCentroid | Spectral | Weighted mean frequency |
| Change | SpectralFlux | Spectral | Frame-to-frame spectral difference |
| Noisiness | SpectralFlatness | Spectral | Tonal vs noise ratio |
| Note | DominantPitch | Pitch | Fundamental pitch via Aubio yinfft |
| Note Confidence | PitchConfidence | Pitch | Pitch detection confidence [0, 1] |
| Chord Change | HarmonicChange | Pitch | HCDF harmonic transition detection |
| Sidechain Pump | SidechainPump | P25 Advanced | Bass/mid anti-correlation (EDM pump) |
| Swing | SwingRatio | P25 Advanced | Timing deviation from grid |
| Vocal Presence | FormantPresence | P25 Advanced | Vocal formant energy in 300-3kHz |
| Resonance | ResonancePeak | P25 Advanced | Spectral kurtosis |
| Reese Bass | ReeseBass | P25 Advanced | Bass spectral spread |

**Modulation Signals (2, visible):**

| Signal Name | Type | Behavior |
|-------------|------|----------|
| Mod 1 | OscillatorSignal | BPM-locked waveform generator (src/signal/OscillatorSignal.h). 5 shapes: Sine/SawUp/SawDown/Triangle/Square. `beatDuration` sets period in beats (0.25/0.5/1/2/4/8). Phase derived from `beatPhase + beatInBar`, so cycles lock to detected BPM. `phaseOffset` [0,1] shifts start point. `amplitude` scales output. Default: Sine at 1.0 beat/cycle |
| Mod 2 | EnvelopeSignal | Custom envelope with editable control points (src/signal/EnvelopeSignal.h). Each `ControlPoint` has position [0,1] and value [0,1]; envelope interpolates between points with 3 curve types (Linear/Exponential/SCurve). BPM-locked via `beatDuration` (default 4.0 beats/cycle). Supports `phaseOffset`, `amplitude`, `oneShot` (clamp at end) and `looping` modes. Default shape: ramp up at 0.5, ramp down at 1.0 |

**Clip Position Signal (1, hidden):**

| Signal Name | Type | Behavior |
|-------------|------|----------|
| Clip Position | ClipPositionSignal | Tracks active clip playhead [0, 1], normalized to in/out range (src/signal/ClipPositionSignal.h). Render thread calls `updateFromClip()` each frame, which normalizes `playheadPosition` to the clip's in/out range via `(pos - inPoint) / (outPoint - inPoint)`. Returns 0 when clip is null or has no media. Atomic float store/load — safe for cross-thread reads. P24 addition |

**Data flow:**
```
FeatureSnapshot fields → SignalRegistry (named signals) → RoutingEngine → render params
```

**Dependencies & services:**
- No external — uses std::unordered_map, std::atomic

**Config:**
- Hidden signals registered for all 5 advanced audio features (P25)
- Signal values accessible via REST API
- Route targeting currently supports Global scope only (Clip/Layer scopes defined in Route struct but not wired — see Ghost Features)
- Design direction (2026-05-22): per-field AUTO/OVERRIDE state machine planned. OVERRIDE indicated by orange #ff4500. Per-field release only, no global mass-release. See MENTAL_MODELS.md Model 3 and FEATURE_CONNECTIONS.md for behavioral spec.

**Failure modes:**
- Circular signal dependency → infinite evaluation loop (unhandled — depends on topology)
- Missing signal name → returns default 0.0 (handled)

**Test coverage:**
- `tests/test_routing_engine.cpp` — signal registration, routing, chained evaluation
- Missing: circular dependency detection, performance under high signal count

**Gotchas:**
- Signals and routing evaluate EVERY frame in `Renderer::renderOpenGL()` — must be fast.
- Advanced audio features (P25) are registered as hidden signals — not visible in UI signal list.
- (`ChainedSignal` removed Wave 0 — its stale-cached-value / evaluation-order gotcha no longer applies.)
- **`getCachedValue()` is O(n) linear scan per call** — each route calls once. Fine for ~30 signals, won't scale to hundreds.
- **RoutingEngine does NOT apply curve transforms** — uses gain/threshold/falloff instead. Deliberate v2 design difference from MappingEngine.
- **Clip/Layer scope routes are evaluated but values discarded.** The routing lambda at `Renderer.cpp:198-206` only handles `TargetScope::Global`. Non-Global routes are silently dropped with a TODO comment (line 205). See Feature 22c for full ghost status.

### 7b. SignalInspector (UI)

**What it does:** Displays full settings for the currently selected signal in the InspectorPanel's Signal tab. Adapts its controls based on signal type: audio signals show threshold/gain/falloff sliders; oscillator signals show wave shape, beat duration, amplitude, and phase offset; envelope signals show a **display-only** curve editor (painted preview — NO draggable control points; `SignalInspector.cpp:365` has no mouse handlers) and loop/one-shot toggles.

**Key source files:**
- `SignalInspector` class (src/ui/SignalInspector.h:15, src/ui/SignalInspector.cpp)

**Parent component:** InspectorPanel (tab index 3: "Signal"), displayed inside a scrollable Viewport.

**Controls & interactions:**
- Audio signal controls: threshold slider, gain slider, falloff slider
- Oscillator controls: wave shape selector (5 shapes), beat duration selector, amplitude slider, phase offset slider
- Envelope controls: curve type selector (Linear/Exponential/SCurve), beat duration selector, amplitude slider, phase slider, looping toggle, one-shot toggle
- Curve editor area (100px tall) — visual representation of envelope control points
- All controls are hidden/shown dynamically based on signal type via `hideAllControls()` / `showAudioControls()` / `showOscillatorControls()` / `showEnvelopeControls()`

**Gotchas:**
- Envelope curve editor is paint-only (a painted preview of the control-point curve) — draggable editing is NOT implemented (no mouse handlers, `SignalInspector.cpp:365`).
- `setSignal(nullptr)` hides all controls gracefully.


**Storage:** N/A — real-time in-memory processing

---

## 8. Clip & Layer Composition [R]
**What it does:** Hierarchical data model for VJ performance: Composition → Decks → Layers → Clips, with undo/redo and per-type autopilot automation.

**Entry points:**
- `Clip` struct (src/model/Clip.h:10) — media content + per-clip effects + transport
- `Layer` struct (src/model/Layer.h:27) — layer with clips, effects, opacity, blend mode
- `Composition` struct (src/model/Composition.h:10) — top-level container
- `UndoManager` class (src/core/UndoManager.h:9) — Command pattern undo/redo
- `Autopilot` class (src/model/Autopilot.h:14) — auto-advance clips

**Implementation chain:**
1. `Composition` owns multiple `Deck`s, each containing `Layer`s x columns of `Clip`s
2. Active deck renders; persistent layers from other decks also render
3. `Autopilot::processFrame()` runs in render thread — checks beat/video triggers
4. On advance: `onAutopilotAdvanced_` fires async on message thread to refresh UI
5. Smart random: uses structural state + energy level for intelligent clip selection
6. `UndoManager`: Command pattern, records state changes, supports undo/redo chain

**Data flow:**
```
User action → UndoManager::execute(Command) → Composition state change → Renderer reads next frame
Autopilot: beat/video trigger → advance clip → fire callback → refresh DeckView
```

**Dependencies & services:**
- JUCE: `MessageManager::callAsync()` for thread-safe UI updates

**Storage:**
- `PresetManager` serializes Composition + effects + mappings to JSON
- Presets saved to: `resources/presets/` and user-chosen paths
- Session events recorded to JSON by `SessionRecorder`

**Config:**
- Layer types: Opaque, Transparent, FX Only, Mask
- Autopilot triggers: On Beat (1/2/4/8/16/32 beats x loops) or End of Video
- Per-type autopilot: separate beat counts for Opaque/Transparent/FX Only layers
- Beat snap modes: Off, Beat, Bar, TwoBar, FourBar

**Failure modes:**
- Clip media file missing → falls back to default image or empty (handled)
- Autopilot on empty layer → no advance, no crash (handled)
- Undo stack overflow → oldest commands discarded (handled)
- Transport state race: render thread writes `clip->playing` (mutable) for OneShot stop (documented, by design)

**Test coverage:**
- `tests/test_composition.cpp` — Clip/Layer creation, undo/redo
- `tests/test_compositor.cpp` — deck compositing, autopilot behavior
- Missing: preset save/load round-trip, cross-deck transition, persistent layer rendering

**Gotchas:**
- `Clip::playing` is `mutable` — render thread writes it for OneShot. After `advanceFrame()`, read player state BACK to clip model.
- Retriggering same clip preserves play/pause state. Only first activation auto-plays (`hasBeenTriggered` flag).
- Per-type autopilot `perTypeEnabled` defaults to false — must be explicitly enabled.
- Smart random assumes lower column index = calmer content, higher = more intense.
- **`std::rand()` used in Autopilot without explicit seeding** (Autopilot.cpp:236, 304, 352). Three call sites use `std::rand()` for random clip selection and score jittering. No `std::srand()` call exists in the codebase — seed depends on implementation default. `std::rand()` thread safety is implementation-defined; since Autopilot runs on the render thread, concurrent `std::rand()` calls from other threads could corrupt internal state.

### 8a. CompDecksBrowser (UI)

**What it does:** Shows saved compositions and decks in the BrowserPanel. Two collapsible sections: Compositions (click to load full application state) and Decks (click to switch active deck). Supports save/rename/delete operations.

**Key source files:**
- `CompDecksBrowser` class (src/ui/CompDecksBrowser.h:10, src/ui/CompDecksBrowser.cpp)

**Parent component:** BrowserPanel (tab index 3: "Comp/Decks")

**Controls & interactions:**
- "Save Composition" button — saves current state to JSON
- "Save Deck" button — saves current deck
- Compositions section (collapsible) — lists saved .json files with name and date, click to load
- Decks section (collapsible) — lists saved deck files, click to load
- Scrollable viewport for long lists
- Callbacks: `onCompositionLoad`, `onDeckLoad`, `onCompositionSave`

**Gotchas:**
- `setComposition()` must be called before save operations work.
- Files are scanned from `getCompositionsDir()` and `getDecksDir()` static paths — user document directory.

---

## 9. Procedural Sources [R]
**What it does:** 108 code-generated visual sources across 18 categories: 7 2D fractals, 8 3D ray-marched fractals, 8 torus variants, 9 audio-visual, text, simulations, pattern/noise/geometric/particle/nature sources, 7 wireframe shapes, and a MilkDrop visualizer. Total: 759 parameters at runtime.

**Entry points:**
- `SourceRegistry` class (src/sources/SourceRegistry.h:13) — registration of all sources
- `ProceduralSource` class (src/sources/ProceduralSource.h) — base class, param management

**Implementation chain:**
1. `SourceRegistry::registerAll()` registers all 108 sources with params and shader names
2. User loads source from Sources Browser → `Renderer::setActiveSource()`
3. Each frame: `ProceduralSource::uploadUniforms()` uploads all params + 42 audio uniforms
4. Source shader renders on fullscreen quad — output is the clip texture

**Data flow:**
```
SourceRegistry → ProceduralSource params → uploadUniforms() → GLSL shader → texture output
Audio uniforms (u_rms, u_bass, u_beatPhase, etc.) available in all source shaders
```

**Dependencies & services:**
- OpenGL 4.1: fragment shader rendering
- GLSL 410: all source shaders in EmbeddedShaders.h

**Config:**
- All source params normalized [0, 1]
- 3D fractals share: zoom, speed, angle X/Y, cross section, slice count/distance, glow, trail, feedback, color shift, palette
- 2D fractals share: dive speed, location presets, zoom, center X/Y, iterations, color, palette
- 8 cosine palettes: Fire/Ocean/Neon/Gray/Rainbow/Psyche/Ice/Sunset

**Source inventory — 759 parameters across 108 sources (18 categories):**

| Category | Sources | Parameters | Avg Params/Source |
|----------|---------|------------|-------------------|
| 3D | 24 | 277 | 11.5 |
| Wireframe | 7 | 63 | 9.0 |
| Lines | 11 | 65 | 5.9 |
| Geometric | 11 | 60 | 5.5 |
| Fractal | 7 | 54 | 7.7 |
| Math | 8 | 45 | 5.6 |
| Audio-Visual | 9 | 42 | 4.7 |
| Pattern | 8 | 40 | 5.0 |
| Nature | 6 | 31 | 5.2 |
| Simulation | 3 | 18 | 6.0 |
| Noise | 3 | 12 | 4.0 |
| Text | 2 | 12 | 6.0 |
| Particle | 3 | 12 | 4.0 |
| Utility | 2 | 11 | 5.5 |
| Lighting | 1 | 6 | 6.0 |
| Organic | 1 | 5 | 5.0 |
| Routing | 1 | 1 | 1.0 |
| MilkDrop | 1 | 5 | 5.0 |
| **Total** | **108** | **759** | **7.0** |

Note: Raw `addParam()` grep count is 628. The 754-param shader-registry subtotal expands from 628 because `addTorusControls()` (12 params) is called on 7 torus sources and `registerWireframe()` (9 params) is called on 7 wireframe sources — helper bodies are counted once by grep but expand at runtime. MilkDrop's `ProjectMSource` adds 5 programmatic params (Beat Sensitivity, Speed, Warp, Decay, Gamma; ProjectMSource.cpp:13-17) on top of the registry sources → **grand total 759** (receiver-verified 2026-07-16). Note: 3D was previously stated as 298 (torus shared-control double-count); parse ground-truth is 277.

<details>
<summary>Sources by category (click to expand)</summary>

**3D (24 sources, 277 params):**
Mandelbulb (16), Menger Sponge (14), Kaleidoscopic IFS (16), Julia Set 3D (17), Burning Ship 3D (14), Newton 3D (14), Sierpinski Tetrahedron (13), Apollonian 3D (14), Striped Torus (14), Spiral Vortex (14), Checker Torus (14), Ribbed Vortex (14), Wormhole Tunnel (13), Twisted Torus (14), Wormhole (14), Torus Hole (19), Spiral Tunnel (5), Crystal Cavern (6), Infinite Corridor (6), Orbit Chamber (7), Scroll Plane (5), Rotating Cube Map (5), Dual Plane Drift (5), DNA Helix (4)

**Audio-Visual (9 sources, 42 params):**
Audio Waveform (4), Spectrum Landscape (5), Chromatic Ring (4), Band Tower (6), Timbral Nebula (6), Structural Landscape (6), Cymatics (3), Spectral Waterfall (3), Spectral Ring (5)

**Fractal (7 sources, 54 params):**
Kaleidoscopic Fractal (6), Mandelbrot/Julia (11), Julia Set (9), Burning Ship (9), Newton Fractal (6), Sierpinski (7), Apollonian Gasket (6)

**Geometric (11 sources, 60 params):**
Geometric Tunnel (4), Color Gradient (4), Shape Generator (5), Infinite Zoom (4), Moire Interference (7), Astral Grid (6), Radial Burst (7), Hex Grid (6), Sacred Geometry (7), Radar Sweep (4), Dot Matrix Wave (6)

**Lines (11 sources, 65 params):**
Zigzag Lines (5), Star Burst (5), Polygon Lines (6), Waveform Lines (7), Lissajous (6), Spirograph (5), Angular Grid (6), Fractal Tree (6), Laser Scan (5), Moire Lines (5), Line Generator (9)

**Math (8 sources, 45 params):**
Lissajous Weaver (7), Fermat Spiral Garden (6), Hyperbolic Tiling (6), Penrose Pulse (5), Superformula (6), Truchet Labyrinth (5), Rose Curves (5), Fibonacci Spiral (5)

**Nature (6 sources, 31 params):**
Reaction-Diffusion (4, stateful), Cellular Automata (5, stateful), Fire Wall (6), Water Caustics (6), Electric Arc (6), Fire (4)

**Noise (3 sources, 12 params):**
Perlin Noise (4), Plasma (4), Voronoi (4)

**Organic (1 source, 5 params):**
Metaballs (5)

**Particle (3 sources, 12 params):**
Lightning Storm (4), Starfield (4), Particle Nebula (4)

**Pattern (8 sources, 40 params):**
Checkerboard (4), Line Pattern (5), Concentric Rings (5), Sine Oscillator (6), Spiral Pattern (5), Terrain Lines (6), Bump Light (5), Glitch Grid (4)

**Routing (1 source, 1 param):**
Layer Router (1) — routes another layer's output as source content

**Simulation (3 sources, 18 params):**
Strange Attractor (6, stateful), Gravity Well (6, stateful), Fluid Dynamics (6, stateful)

**Text (2 sources, 12 params):**
Scrolling Text Wall (4), Text Animator (8)

**Utility (2 sources, 11 params):**
Solid Color (3), Strobe Light (8)

**Wireframe (7 sources, 63 params):**
Wireframe Sphere (9), Wireframe Torus (9), Wireframe Cube (9), Wireframe Cylinder (9), Wireframe Cone (9), Wireframe Icosahedron (9), Wireframe Wolf (9)

**MilkDrop (1 source, 5 params):**
MilkDrop Visualizer — via ProjectMSource (see Feature 17). Uses libprojectM-4 with ~9800 presets. 5 params: Beat Sensitivity, Speed, Warp, Decay, Gamma (ProjectMSource.cpp:13-17; the Gamma param is a no-op placeholder, ProjectMSource.cpp:246-250). Build-conditional (AUDIODNA_HAS_PROJECTM); renders a dark-purple placeholder if libprojectM-4 is not linked.

**Lighting (1 source, 6 params):**
Laser Scanner (6)

</details>

**Failure modes:**
- Shader compilation failure → source not available, logged (handled)
- Stateful sources (Strange Attractor, Gravity Well, Fluid Dynamics) appear black in single-frame mode (by design — need continuous frames)

**Test coverage:**
- `tests/visual/test_sources.py` — auto-discovers ALL sources, sweeps every param (non-black, has-effect, no-discontinuity)
- `tests/visual/test_fractals.py` — ~185 parametrized tests for every fractal param
- `tests/visual/test_audio_reactivity.py` — injected audio features change source output
- `tests/visual/test_time_sweep.py` — animated sources change over time
- Missing: Fluid Dynamics audio injection test, text source font rendering edge cases

**Gotchas:**
- Fractal zoom: NEVER use `fract()` — creates visible jump-cuts. Use direct `zoomExp` clamped at max depth.
- Fractal center/location/dive MUST be cleanly separated. Dive controls zoom rate only, never center.
- Mandelbrot power > 4 makes set too small — limit to 2-4 for VJ use. Clamp smooth iteration with `max(si, 0.0)`.
- 3D camera distance `mix(5.0, 0.3, zoom)` — at zoom=1 camera is INSIDE the fractal.
- Layer Router source reads another layer's output — self-reference gets previous frame (1 frame delay).

### 9a. SourcesBrowser (UI)

**What it does:** Displays all 108 procedural sources organized by category in a searchable, scrollable list. Users drag sources onto clip cells in the DeckView to use them as clip content.

**Key source files:**
- `SourcesBrowser` class (src/ui/SourcesBrowser.h:10, src/ui/SourcesBrowser.cpp)

**Parent component:** BrowserPanel (tab index 2: "Sources")

**Controls & interactions:**
- Search box at top — filters sources by name
- Category headers (collapsible) — click to expand/collapse, color-coded per category
- Source rows (28px each) — click to select, drag onto deck clip cells
- Multi-select tracking via `selectedIndices_` set
- Source entries carry both display name and registry ID (e.g., "Perlin Noise" / "perlin_noise")
- `onSourceActivated` callback fires on double-click or drag activation

**Gotchas:**
- Source list is built at construction from hardcoded entries, not dynamically from SourceRegistry. Adding new sources requires updating `buildSourceList()`.
- Source IDs (snake_case) differ from display names (Title Case) — always use `sourceId` for runtime operations.


**Storage:** N/A — real-time in-memory processing

---

## 10. Keyboard & MIDI Performance [R]
**What it does:** Maps keyboard keys and MIDI notes/CC to actions (clip triggers, param control, transport) with 3 targeting modes, toggle/momentary triggers, velocity-to-opacity, and hardware feedback for Launchpad/APC.

**Entry points:**
- `BindingManager` class (src/binding/BindingManager.h:10)
- `MidiHandler` class (src/midi/MidiHandler.h:8) — MIDI input + learn
- `MidiOutputHandler` class (src/midi/MidiOutputHandler.h:23) — pad feedback

**Implementation chain:**
1. Key press → `BindingManager::processKeyPress()` → resolve target via TargetMode → execute action
2. MIDI note → `MidiHandler::handleIncomingMidiMessage()` → `BindingManager::processMidiNote()`
3. MIDI CC → `BindingManager::processMidiCC()` → Absolute (0-127→0-1) or Relative (+-offset)
4. `MidiOutputHandler::timerCallback()` polls deck state ~6Hz → sends note-on/off to controller
5. Key-up routing: `MainComponent::keyStateChanged()` polls all momentary-bound keys

**Data flow:**
```
Keyboard/MIDI → BindingManager → resolve target (ByPosition|ThisItem|Selected) → execute action → update state
Deck state → MidiOutputHandler (6Hz poll) → note-on/off → Launchpad/APC pad LEDs
```

**Dependencies & services:**
- JUCE: `MidiInputCallback`, `MidiOutput`, keyboard events

**Config:**
- Target modes: ByPosition (survives reorder), ThisItem (follows clip ID), Selected (current UI selection)
- Trigger modes: Toggle (press to toggle) or Momentary (held = active)
- CC modes: Absolute (0-127→0-1) or Relative (for endless encoders)
- Launchpad note mapping: `(layer+1)*10 + (column+1)`
- 5 LED states: Empty(off), Loaded(5), Playing(60), Triggered(52), ActiveWithFx(62)

**Failure modes:**
- MIDI device disconnected → MidiHandler stops receiving, no crash (handled)
- Multiple bindings on same key → all fire (by design)
- MIDI learn timeout → cancelled (handled)

**Test coverage:**
- Missing: no dedicated test file for binding system or MIDI handler

**Gotchas:**
- Momentary key release routed via `keyStateChanged()` polling, not keyUp event (JUCE limitation).
- MIDI note-off already routed through `BindingManager::processMidiNoteOff()`.
- Velocity-to-opacity maps MIDI velocity to clip opacity on trigger.
- **`MidiHandler::handleIncomingMidiMessage()` captures `this` in callAsync lambda** — if MidiHandler destroyed while lambdas queued, use-after-free. `stop()` removes callbacks but queued lambdas may still execute.
- **`processKeyDown` returns on FIRST match** — if multiple bindings match same input, only first in vector order fires.
- **Modifier mismatch on keyUp** — if user releases Shift before releasing bound key, modifier mismatch causes momentary release to be missed.
- **Relative CC mode**: accumulated value is per-(channel, CC) globally. Two bindings on same CC with different step sizes share accumulated value.

### 10a. BindingOverlay (UI)

**What it does:** Semi-transparent fullscreen overlay shown when keyboard binding mode is active (Shortcuts > Edit Keyboard, Shift+Cmd+K). Highlights all bindable UI elements (clip cells, macros, transport). User clicks a target, then presses a key to create the binding.

**Key source files:**
- `BindingOverlay` class (src/ui/BindingOverlay.h:10, src/ui/BindingOverlay.cpp)

**Parent component:** MainComponent (child overlay, shown on top of all panels)

**Controls & interactions:**
- Enter via `enterBindingMode()` — shows overlay, begins target highlighting
- Click a highlighted target → enters "waiting for key" state (`waitingForKey_` flag)
- Press any key → creates a `Binding` via `BindingManager`, exits wait state
- Escape exits binding mode entirely
- `setBindableTargets()` receives geometry from MainComponent so targets are drawn at correct positions
- Displays existing binding key labels next to each target
- `onBindingModeExit` callback notifies parent when done

**Gotchas:**
- Implements `juce::KeyListener` to intercept key presses during binding mode.
- Target hit testing is rectangle-based — overlapping UI elements may cause wrong target selection.
- Binding targets are regenerated on each enter (not cached) to match current deck layout.

### 10b. MidiLearnOverlay (UI)

**What it does:** Semi-transparent fullscreen overlay for MIDI learn mode (Shortcuts > Edit MIDI, Shift+Cmd+M). Same workflow as BindingOverlay but listens for MIDI input instead of keyboard. Notes map to triggers (clips, columns); CCs map to continuous controls (sliders, knobs).

**Key source files:**
- `MidiLearnOverlay` class (src/ui/MidiLearnOverlay.h:13, src/ui/MidiLearnOverlay.cpp)

**Parent component:** MainComponent (child overlay, shown on top of all panels)

**Controls & interactions:**
- Enter via `enterLearnMode(AudioDeviceManager*)` — registers as MIDI input callback, shows overlay
- Click a target → enters "waiting for MIDI" state (`waitingForMidi_` flag)
- Send any MIDI note or CC → creates binding, shows last MIDI message info
- Escape exits learn mode
- Reuses `BindingOverlay::BindableTarget` struct for target geometry
- `stopListening()` unregisters from all MIDI inputs on exit

**Gotchas:**
- Implements `juce::MidiInputCallback` — receives MIDI on the MIDI input thread, must dispatch to message thread for UI updates.
- Requires a valid `AudioDeviceManager*` to enumerate and listen on MIDI inputs.
- Shares the same BindableTarget format as BindingOverlay — targets are interchangeable.


**Storage:** N/A — real-time in-memory processing

---

## 11. Video Playback & Media [R]
**What it does:** Decodes video files via FFmpeg to GL textures, and plays multi-image sequences at configurable FPS. `VideoPlayer`/`ImageSequence` are **transport-only and tempo-agnostic** — they expose speed/reverse/{Loop,PingPong,OneShot}/seekTo only. **BPM sync, transport mode (Timeline/BPMSync), in/out points, beat division, and cuepoints are NOT VideoPlayer/ImageSequence capabilities** — they are Clip-struct fields (`Clip.h`) + ClipInspector controls, applied externally by scaling `setSpeed()`/`advanceFrame(dt)`. Codec set = whatever the linked FFmpeg build provides; the `VideoPlayer.h:19` header comment lists MP4/MOV/QuickTime (H.264/H.265/ProRes)/HAP/HAP-Alpha/AVI (MKV/WebM are NOT in the header and not code-guaranteed).

**Entry points:**
- `VideoPlayer` class (src/media/VideoPlayer.h:29) — FFmpeg decode pipeline
- `ImageSequence` class (src/media/ImageSequence.h) — multi-image playback

**Implementation chain:**
1. `VideoPlayer`: open file via `avformat_open_input` → find video stream → `avcodec_alloc_context3`
2. Decode loop: `av_read_frame` → `avcodec_send_packet` → `avcodec_receive_frame`
3. Convert via `sws_scale` → upload to GL texture
4. `ImageSequence`: load image files → cycle at configurable FPS or BPM-synced
5. BPM Sync transport (`beatDivision` x BPM → playback speed) is a **Clip/ClipInspector** behavior that drives `VideoPlayer`/`ImageSequence` externally via `setSpeed()` — neither class holds `beatDivision`, `transportMode`, in/out points, or cuepoints itself

**Data flow:**
```
Video file → FFmpeg decode → sws_scale (pixel format conversion) → glTexImage2D → clip texture
Image folder → load all images → cycle by timer/BPM → clip texture
```

**Dependencies & services:**
- FFmpeg 8.0: libavformat, libavcodec, libavutil, libswscale (LGPL/GPL)

**Config:**
- Loop modes: Loop, PingPong, OneShot — **the only transport enum VideoPlayer/ImageSequence own** (VideoPlayer.h:53, ImageSequence.h:45)
- Applied externally by the Clip layer (Clip-struct fields, NOT held by VideoPlayer/ImageSequence): Transport mode Timeline/BPMSync; in/out points [0,1] (draggable on the ClipInspector timeline); beat-division presets for BPM sync; content-beats for exact timing of authored content; 8 cuepoints

**Failure modes:**
- Unsupported codec → `avcodec_find_decoder` returns null (handled — video not loaded)
- Corrupt video file → decode errors logged, frame skipped (handled)
- FFmpeg not installed → CMake FindFFmpeg fails at build time

**Test coverage:**
- Missing: no dedicated video playback test. Visual tests use images, not video.

**Gotchas:**
- HAP Alpha requires specific FFmpeg codec support.
- BPM Sync with Content Beats requires knowing how many beats the video content represents.

### 11a. FilesBrowser (UI)

**What it does:** Folder navigation panel with thumbnails, search, and favorites. Users browse images, videos, and audio files and drag them onto deck cells to load as clip content. Supports grid view (thumbnails) and list view.

**Key source files:**
- `FilesBrowser` class (src/ui/FilesBrowser.h:9, src/ui/FilesBrowser.cpp)

**Parent component:** BrowserPanel (tab index 0: "Files")

**Controls & interactions:**
- Navigation bar: Up button ("^"), path bar (editable), search field
- View toggle: Grid view (64px thumbnails) / List view buttons
- File entries — click to select, double-click or drag to load into deck
- Multi-select via `selectedIndices_` set
- Directory navigation — double-click folders to enter, Up button to go up
- Favorites system — toggle favorite per file, persisted across sessions
- `onFileActivated` callback fires on activation
- `navigateTo(folder)` — programmatic navigation

**Gotchas:**
- Implements `juce::FileDragAndDropTarget` but returns `false` for `isInterestedInFileDrag()` — receives drags FROM this panel to deck cells, not into it.
- Thumbnail generation (`generateThumbnail`) may be slow for large images — runs on the message thread.
- Favorites stored via `loadFavorites()`/`saveFavorites()` — persistence location is user data directory.


**Storage:** N/A — real-time in-memory processing

---

## 12. Video Recording & Capture [R]
**What it does:** Records the live output to H.264/ProRes/MJPEG video files via FFmpeg with triple-buffered GL readback, and captures PNG snapshots.

**Entry points:**
- `VideoRecorder` class (src/recording/VideoRecorder.h:36) — real-time recording
- `SessionRecorder` class (src/recording/SessionRecorder.h) — timestamped event capture

**Implementation chain:**
1. Start recording: Menu → Output → Start Recording (or binding)
2. `VideoRecorder::submitFrame()` — GL thread does `glReadPixels` into rotating CPU buffer
3. Encoder thread wakes via condition variable, encodes frame to configured codec
4. If encoder can't keep up → frames dropped (counted in `droppedFrames_`)
5. `SessionRecorder` captures timestamped events (clip triggers, param changes) to JSON
6. Snapshot: `Renderer::takeSnapshot()` → PNG to ~/Documents/Audio-DNA/Snapshots/

**Data flow:**
```
GL framebuffer → glReadPixels (triple-buffered) → encoder thread → FFmpeg encode → video file
Performance events → SessionRecorder → JSON file
```

**Dependencies & services:**
- FFmpeg 8.0: encoding (H.264, ProRes, MJPEG)

**Storage:**
- Videos: ~/Documents/Audio-DNA/Recordings/
- Snapshots: ~/Documents/Audio-DNA/Snapshots/
- Session events: JSON files

**Config:**
- Codecs: H.264 (default), ProRes, MJPEG via `VideoRecorder::Config`
- Triple-buffered pixel readback (zero mutex on GL thread)

**Failure modes:**
- Disk full → write fails, recording stops (partially handled)
- Encoder slower than render → frames dropped, counted (handled by design)
- FFmpeg codec unavailable → recording fails to start (handled)

**Test coverage:**
- Missing: no dedicated recording test

**Gotchas:**
- NO mutex on `submitFrame()` (GL thread hot path). Triple-buffer with atomic index rotation only.
- Frame drops are expected under heavy load — counted, not prevented.

---

## 13. Output & Display [R]
**What it does:** Sends rendered output to fullscreen display on any connected monitor, with optional Syphon output (wired Wave 1-A) for inter-app GPU texture sharing on macOS.

**Entry points:**
- `OutputWindow` class (src/output/OutputWindow.h) — fullscreen output
- `SyphonOutput` class (src/output/SyphonOutput.h:21/59) — macOS Syphon server — WIRED Wave 1-A (publishes the final composited frame each frame)
- (`SyphonInput` removed Wave 0 — was an orphaned macOS Syphon client, never instantiated)

**Implementation chain:**
1. `OutputWindow` creates fullscreen window on selected display
2. Renderer output texture shared with OutputWindow's OpenGL context
3. `SyphonOutput`: wraps `SyphonServer`, publishes the final composited GL texture via IOSurface (zero-copy) — WIRED Wave 1-A, gated on enabled+initialized and the build flag; re-binds `defaultFBO` after publish so the capture path is unaffected
4. (`SyphonInput` removed Wave 0 — no texture-receive path)
5. (Spout/NDI stubs removed Wave 0)

**Data flow:**
```
Renderer output → OutputWindow (fullscreen display)
Renderer output → SyphonOutput → IOSurface → MadMapper/VDMX/OBS   (Wave 1-A; build-flag-gated)
```

**Dependencies & services:**
- JUCE: `DocumentWindow` for fullscreen
- Syphon.framework (macOS, optional): BSD license, must be in /Library/Frameworks/

**Config:**
- Syphon: compile flag `-DAUDIODNA_BUILD_SYPHON=ON`
- Uses `__has_include(<Syphon/Syphon.h>)` for compile-time detection — no-op stub if missing

**Failure modes:**
- Syphon framework not installed → compiles as no-op stub (handled)
- Output display disconnected → window closes gracefully (handled by JUCE)
- (Spout/NDI stubs removed Wave 0)

**Test coverage:**
- Missing: no output-specific tests

**Gotchas:**
- Syphon is Obj-C++ (.mm files) — only compiles on macOS.
- `__has_include` detection means build succeeds without Syphon installed, but feature is silently disabled.


**Storage:** N/A — real-time in-memory processing

---

## 14. External Control [R]
**What it does:** REST API (port 7070, 20+ endpoints) and OSC input for external control of all app functions.

**Entry points:**
- `ApiServer` class (src/api/ApiServer.h:31) — cpp-httplib server, always-on
- `OscHandler` class (src/osc/OscHandler.h:27) — juce_osc receiver

**Implementation chain:**
1. `ApiServer`: cpp-httplib on background thread, CORS headers, 20+ endpoints
2. GL mutations via existing thread-safe APIs (atomic config vars, message thread dispatch)
3. `OscHandler`: juce_osc on JUCE message thread (MessageLoopCallback) — LIVE Wave 1-B: `startListening(8000)` at startup, all 11/11 callbacks wired (port hardcoded)
4. OSC patterns: `/audiodna/clip/{layer}/{column}`, `/audiodna/layer/{n}/opacity`, etc.

**Data flow:**
```
HTTP request → ApiServer (background thread) → thread-safe API → state change
OSC message → OscHandler (message thread) → state change
```

**Dependencies & services:**
- cpp-httplib 0.18.3 (MIT): HTTP server (always linked, not test-only)
- JUCE juce_osc: OSC receiver

**Config:**
- REST API: port 7070, always-on
- Eyes test server: port 8080, conditional (`AUDIODNA_BUILD_TEST_SERVER`)
- OSC: configurable UDP port

**Failure modes:**
- Port 7070 in use → ApiServer fails to start, logged (handled)
- Malformed OSC message → ignored (handled by juce_osc)
- Unknown API endpoint → 404 response (handled)

**Test coverage:**
- `tests/visual/test_render_pipeline.py` — uses test server API (port 8080) extensively
- REST endpoints documented in CLAUDE.md with examples
- Missing: load testing, concurrent request handling, OSC edge cases

**Gotchas:**
- cpp-httplib was promoted from test-only to always-linked in P22. `#include <httplib.h>` works everywhere.
- Eyes test server (`render_frame`) doesn't apply global effect chain — known limitation.
- All GL mutations from API must go through thread-safe paths (atomics or message thread dispatch).
- **DATA RACE: `set_layer_opacity` and `set_param` write DIRECTLY from HTTP background thread** — no mutex, no message-thread dispatch. Race with render thread and message thread. Other mutating endpoints (trigger_clip, switch_deck) correctly use `callAsync`.
- **`/api/set_bpm` wired Wave 0** — drives the TopBar manual-BPM override path (setManualMode + setManualBPM, marshalled to the message thread).
- **CORS OPTIONS handler missing** — browser preflight requests get 404. Post-routing handler only adds headers to actual responses, not OPTIONS preflight.
- **`ApiServer` captures `this` in callAsync lambdas** — if server stopped while lambdas queued, use-after-free possible.
- **`handleSnapshot()` blocks HTTP thread** while rendering — stalls other API requests.


**Storage:** N/A — real-time in-memory processing

---

## 15. Ableton Link Sync [R]
**What it does:** Optional tempo synchronization with Ableton Live and other Link-enabled applications.

**Entry points:**
- `LinkSync` class (src/sync/LinkSync.h:16) — wraps `ableton::Link`

**Implementation chain:**
1. `LinkSync` wraps `ableton::Link`, enabled via compile flag
2. Updates cached BPM/phase via atomics from Link's network session
3. When active: overrides BPM tracker output via manual mode
4. Beat phase continues from Link's timeline

**Data flow:**
```
Link network session → LinkSync (atomic BPM/phase) → BPMTracker manual mode → FeatureSnapshot.bpm/beatPhase
```

**Dependencies & services:**
- Ableton Link SDK (optional, via compile flag `-DAUDIODNA_BUILD_LINK=ON`)

**Config:**
- Compile flag: `AUDIODNA_BUILD_LINK`
- When enabled, overrides local BPM detection

**Failure modes:**
- No Link peers → runs standalone, BPM from local detection (handled)
- Network issues → Link auto-recovers (handled by Link SDK)

**Test coverage:**
- Missing: no Link-specific tests (requires network peers)

**Gotchas:**
- Link sync overrides BPM tracker via manual mode — disables local BPM detection while active.
- **`quantum_` is non-atomic `double`** — read by analysis thread, written by UI thread. Technically a torn-read risk (benign on x86-64 but not C++ standard compliant).
- **`update()` must be called explicitly per frame** — Link state is not pushed. Stale `getBeatPhase()` values if `update()` frequency drops.
- **`requestBeatAtTime()` is implemented but never called** — force-aligns all Link peers to downbeat. No UI button or binding exposes it (ghost function).


**Storage:** N/A — real-time in-memory processing

---

## 16. Genre Detection & Smart Features [R]
**What it does:** Real-time 8-genre classification from audio features with smart autopilot and structural scene triggering. (Genre EMA/hysteresis smoothing is done inside `GenreDetector` itself; the separate `GenreSmoothing` class and the `MappingSuggester` "AI suggestions" — both dead/ghost — were REMOVED Wave 0.)

**Entry points:**
- `GenreDetector` class (src/analysis/GenreDetector.h) — 8-genre classifier; does its own ~2s EMA + ~3s hysteresis internally
- (`GenreSmoothing` removed Wave 0 — was dead per-genre EMA presets, never instantiated, did NOT drive MappingEngine/GenreDetector smoothing)
- (`MappingSuggester` removed Wave 0 — was a ghost: genre-aware suggestions, never instantiated, no UI/API caller)

**Implementation chain:**
1. `GenreDetector::process()` — multi-feature scoring (BPM, spectral profile, transient density, chromatic complexity)
2. ~2s EMA smoothing + ~3s hysteresis prevents rapid genre flapping
3. Genre change → `Renderer::onGenreChanged_` callback → auto-preset/deck switch
4. (`MappingSuggester::suggest()` removed Wave 0 — genre-aware recommendations no longer present)
5. Smart random autopilot uses structural state + energy level for clip selection
6. Structural scene triggering: `Renderer::onStructuralStateChanged_` on transitions

**Data flow:**
```
Audio features → GenreDetector (scoring + smoothing) → genre + energy state → auto-preset / smart autopilot / mapping suggestions
Structural transitions → onStructuralStateChanged_ → scene trigger
```

**Dependencies & services:**
- No external — pure heuristic scoring

**Config:**
- 8 genres: House(0), Techno(1), DnB(2), Hip-Hop(3), Ambient(4), Rock(5), Pop/Electronic(6), Jazz/Other(7)
- 3 energy states: Low(0), Medium(1), High(2)
- `composition.autoPresetOnGenre` — auto-switch decks on genre change
- `composition.genreDeckAssignment[8]` — genre→deck mapping
- `composition.structuralSceneEnabled` — structural scene triggers

**Failure modes:**
- Ambiguous genre → highest scoring genre wins, confidence low (handled)
- No audio → genre defaults, no changes triggered (handled)

**Test coverage:**
- Missing: no genre accuracy tests, no smart autopilot behavior tests

**Gotchas:**
- Genre detection uses ~2s smoothing — won't respond instantly to track changes.
- Per-genre smoothing: Techno/DnB = fast attack (0.50-0.55), Ambient = slow (0.12).
- Smart random convention: lower column = calm, higher = intense.
- **`std::rand()` used in autopilot** — not seeded explicitly, non-deterministic across runs. Thread safety of `std::rand()` is implementation-defined.
- **Beat crossing detection threshold** — `beatPhase < lastBeatPhase_ - 0.5f` means only phase wraps from ~1.0 to ~0.0 are detected. BPM tracker skipping a beat (phase jumps >0.5 without wrapping) → crossing missed.


**Storage:** N/A — real-time in-memory processing

---

## 17. MilkDrop/projectM Visualization [R]
**What it does:** Renders classic MilkDrop presets via libprojectM-4, supporting ~9800 .milk/.prjm presets with audio-reactive warp meshes, feedback loops, and per-equation beat sensitivity. Provides a dedicated browser with 3 play modes (Jukebox, VJ Clip, Playlist) and audio-driven auto-switching based on structural transitions and energy matching.

**Entry points:**
- `ProjectMSource` class (src/sources/ProjectMSource.h:25) — ProceduralSource subclass, GL lifecycle + render
- `ProjectMPresetManager` class (src/sources/ProjectMPresetManager.h:9) — preset scanning, categorization, navigation
- `PresetSelector` class (src/sources/PresetSelector.h:10) — audio-driven automatic preset switching
- `MilkDropBrowser` class (src/ui/MilkDropBrowser.h:19) — dedicated browser tab with sub-tabs and play modes

**Implementation chain:**
1. `ProjectMPresetManager::scanDirectory()` recursively finds .milk/.prjm files, assigns mood via heuristic keyword matching (Calm/Energetic/Psychedelic/Geometric/Dark/Minimal)
2. `ProjectMSource::initGL()` creates FBO capture target + projectM instance via `projectm_create()` (conditional on `AUDIODNA_HAS_PROJECTM` compile flag)
3. Each render frame (`ProjectMSource::render()`):
   a. Processes pending preset loads (queued from non-GL threads via mutex)
   b. Applies parameter changes (`applyParams()` → projectM API)
   c. If not locked, `PresetSelector::processFrame()` checks structural transitions for auto-switch
   d. Feeds PCM audio buffer to projectM via `projectm_pcm_add_float()` (mono, 2048 samples)
   e. Saves full GL state (viewport, FBO, program, VAO, texture, blend, depth, stencil, cull, scissor)
   f. projectM renders to default framebuffer (`projectm_opengl_render_frame()`)
   g. Blits result to capture FBO via `glBlitFramebuffer()`
   h. Restores saved GL state
4. `MilkDropBrowser` provides UI with 4 sub-tabs (Curated, Favorites, Recent, All) and 3 play modes:
   - **Jukebox:** classic autopilot — pool selection, timing (beats/seconds), blend slider
   - **VJ Clip:** click to preview, drag single preset to deck cell
   - **Playlist:** multi-select presets, drag group to deck cell as cycling playlist
5. `PresetSelector::processFrame()` detects structural transitions (DROP/BUILDUP/BREAKDOWN) and bar crossings, switches presets on next bar boundary with mood-matching

**Data flow:**
```
.milk/.prjm files → ProjectMPresetManager (scan + categorize)
PCM audio → ProjectMSource::feedAudio() → pcmBuffer_ (mutex-protected)
FeatureSnapshot → PresetSelector (structural state + bar phase → auto-switch decision)
ProjectMSource::render() → projectM internal renderer → glBlitFramebuffer → outputTex_ → clip texture
MilkDropBrowser → drag "milkdrop:{path}" or "milkdrop_playlist:path1|path2|path3" → DeckView
```

**Dependencies & services:**
- libprojectM-4: preset rendering engine (optional, compile flag `AUDIODNA_HAS_PROJECTM`)
- JUCE: file scanning, JSON parsing, UI components
- OpenGL 4.1: FBO capture, blit

**Storage:**
- Preset files: user-configured directories, scanned for .milk/.prjm
- Manifest: presets.json (mood, energy, style metadata per preset)
- User data: favorites and user presets saved to JSON

**Config:**
- projectM mesh size: 48x36 (hardcoded in initGL)
- projectM FPS: 60 (hardcoded)
- PCM buffer: 2048 floats (mono)
- Beat sensitivity: 0-2 range (slider 0-1 mapped to 0-2)
- Speed (preset duration): 5-60 seconds (slider 1.0=5s, 0.0=60s)
- 5 exposed params: Beat Sensitivity, Speed, Warp Amount, Decay, Gamma
- 6 mood categories: Calm, Energetic, Psychedelic, Geometric, Dark, Minimal
- Energy levels per mood: Calm=0.2, Minimal=0.3, Dark=0.4, Geometric=0.5, Psychedelic=0.7, Energetic=0.9
- Minimum bars between auto-switches: 4
- Auto-switch fallback: after `transitionBars * 4` bars without structural change

**Failure modes:**
- projectM not compiled (`AUDIODNA_HAS_PROJECTM` not defined) → renders dark purple placeholder (0.05, 0.0, 0.1) (handled)
- `projectm_create()` returns nullptr → `render()` returns empty outputTex_ (handled)
- Preset load from non-GL thread → queued via mutex, applied next frame (handled)
- No presets found → empty browser, no auto-switch, no crash (handled)
- GL state corruption from projectM → mitigated by full save/restore of 16 GL state variables (handled)
- Mood manifest missing → falls back to heuristic keyword matching from preset name (handled)

**Test coverage:**
- Missing: no dedicated MilkDrop/projectM tests
- Missing: preset scanning, mood classification accuracy, auto-switch behavior, GL state save/restore

**Gotchas:**
- **projectM renders to default framebuffer (FBO 0)**, not to a custom FBO. Result must be blitted to capture FBO. This is a projectM-4 API limitation.
- **GL state save/restore is mandatory** — projectM manages its own shaders, VAOs, blend modes, and depth test settings. Without restore, the rest of the render pipeline gets corrupted state.
- **`pcmMutex_` is a std::mutex on the render thread hot path** — blocks if `feedAudio()` is called simultaneously from audio callback thread.
- **Gamma param is a no-op** — placeholder for future projectM-4 API extension.
- **Warp Amount and Decay params have no mapping** in `applyParams()` — only Beat Sensitivity and Speed are actually applied. 3 of 5 params are decorative.
- **Mood heuristic fallback** for names matching no keyword: classifies by first letter range (a-f=Energetic, g-l=Geometric, m-r=Psychedelic, s-z=Calm). Effectively random.
- **`static std::mt19937 rng`** in `randomPreset()` — static RNG shared across calls, not guaranteed thread-safe by C++ standard.
- **Duplicate detection** during scan is O(n) linear search per file — may be slow with ~9800 presets.

---

## 18. Render Pipeline Time Effects [R]
**What it does:** Five temporal effects that operate on frame history rather than spatial pixel manipulation: Echo (ghost trails), Posterize Time (frame rate reduction), Freeze (frame hold), Screen Split (surveillance grid with per-cell delay), and Frame Stutter (temporal jumping). Screen Split and Frame Stutter use a per-layer ring buffer of 480 frames at 1/4 resolution. Echo, Posterize Time, and Freeze use the standard temporal buffer (`u_prev_frame`).

**Entry points:**
- `EffectLibrary::registerAll()` (src/effects/EffectLibrary.cpp:646-670) — registers all 5 time effects
- `CompositorEngine::applyClipEffects()` (src/render/CompositorEngine.cpp:240) — intercepts screen_split and frame_delay
- `CompositorEngine::applyScreenSplit()` (src/render/CompositorEngine.cpp:1308) — grid rendering with per-cell delay
- `CompositorEngine::FrameRingBuffer` (src/render/CompositorEngine.h:169) — 480-frame ring buffer struct

**Implementation chain:**

*Echo (ghost_trails):*
1. Temporal effect using `u_prev_frame` from per-layer temporal buffer
2. Blends current with decayed previous output
3. Decay remapped: slider [0,1] → [0.82, 0.995] for perceptually linear control
4. 4 blend operators via `u_trail_fade`: Add (0), Screen (0.33), Maximum (0.66), Blend (1.0)

*Posterize Time (frame_hold):*
1. Temporal effect using `u_prev_frame`
2. Rate slider exponential: 0=60fps (smooth), 0.5=~6fps (choppy), 1.0=1fps (extreme)
3. Amount param mixes between live and posterized output

*Freeze (time_freeze):*
1. Temporal effect using `u_prev_frame`
2. `u_freeze_amount=1` = fully frozen, `0` = fully live

*Screen Split (screen_split):*
1. Intercepted in `applyClipEffects()` before normal shader path
2. Uses `FrameRingBuffer` — 480 frames at 1/4 resolution per layer
3. Params: columns (2-8), rows (2-8), frames-per-cell delay (0-60), mode
4. 3 direction modes: Sequential (L-to-R), Reverse (BR-to-TL), Diagonal
5. 2px border between cells, dark gray (0.05) background

*Frame Stutter (frame_delay):*
1. Intercepted in `applyClipEffects()` — uses same FrameRingBuffer
2. Params: depth (1-30 frames back), stutter rate
3. stutter=0: pure constant delay; stutter>0: discrete jumping at 1-16 Hz, 2-8 quantized steps

**Data flow:**
```
Echo/Posterize/Freeze: clip texture → shader (u_texture + u_prev_frame) → output
Screen Split: clip texture → pushFrameToRing() → per-cell getFrameFromRing() → viewport grid → effectFBO_A_
Frame Stutter: clip texture → pushFrameToRing() → getFrameFromRing(framesAgo) → direct texture swap
```

**Dependencies & services:**
- OpenGL 4.1 Core Profile: FBO management (glBindFramebuffer, glFramebufferTexture2D), texture operations (glBindTexture, GL_TEXTURE_2D), viewport manipulation (glViewport), blending control (glDisable/GL_BLEND), clear operations (glClearColor, glClear)
- JUCE juce_opengl: GL function wrappers via `juce::gl` namespace (CompositorEngine.cpp:6)
- ShaderManager: compiles and caches GLSL programs — "ghost_trails", "frame_hold", "time_freeze" (Renderer.cpp:1267-1269), "passthrough" for ring buffer blit and Screen Split rendering
- EmbeddedShaders: GLSL #version 410 fragment shaders — `ghostTrails` (EmbeddedShaders.h:8409), `frameHold` (EmbeddedShaders.h:8463), `timeFreeze` (EmbeddedShaders.h:8502)
- FullscreenQuad: screen-aligned quad for fragment shader dispatch and ring buffer blit
- EffectLibrary: effect registration and definition lookup (EffectLibrary.cpp:646-670)
- CompositorEngine internals: effectFBO_A_/effectFBO_B_ (ping-pong FBOs), TemporalBuffer (u_prev_frame for Echo/Posterize/Freeze), FrameRingBuffer (CompositorEngine.h:169 — per-layer, 480 frames at 1/4 resolution for Screen Split/Frame Stutter)
- No external libraries beyond JUCE and OpenGL

**Config:**
- Ring buffer: 480 frames per layer, 1/4 resolution (~120MB VRAM at 1080p source)
- Echo: decay [0,1]→[0.82,0.995], operator Add/Screen/Max/Blend
- Posterize Time: rate [0,1]→[60,1] fps exponential, amount [0,1]
- Freeze: amount [0,1]
- Screen Split: columns/rows [2,8], frames-per-cell [0,60], mode (Sequential/Reverse/Diagonal)
- Frame Stutter: depth [1,30] frames, stutter [1-16 Hz, 2-8 steps]

**Failure modes:**
- Ring buffer VRAM exhaustion → mitigated by 1/4 downscale (handled by design)
- Ring buffer not yet filled → getFrameFromRing clamps to available frames (handled)

**Test coverage:**
- Missing: no dedicated time effects tests

**Gotchas:**
- **Screen Split and Frame Stutter bypass the normal shader pipeline** — intercepted in `applyClipEffects()` with `continue` statements. The GLSL shaders exist in EmbeddedShaders.h but are fallback/passthrough; real logic is in C++ compositor code using the ring buffer.
- **Ring buffer is per-layer** (`layerRingBuffers_` keyed by layerId).
- **Ring buffer initialization is lazy** — `getOrCreateRingBuffer()` allocates on first use. May cause VRAM spike on first frame.
- **Screen Split writes to effectFBO_A_ directly** — ping-pong state must be forced to write to B next.
- **Echo name collision** — FeedbackProcessor also has an "Echo" preset (layer-level Larsen loop, amount=0.60). Different systems, same name.
- **Posterize Time uses `u_time` for quantization** — timing inconsistent if u_time has jitter or resets.


**Storage:** N/A — real-time in-memory processing

---

## 19. LUT Loader (Ghost Feature) [C]
**Status: Ghost — code exists, no UI path to load external .cube files**

**What it does:** Parses .cube LUT files and uploads them as `GL_TEXTURE_3D` handles for color grading. Supports standard .cube format: `LUT_3D_SIZE N` header followed by N^3 RGB float triples.

**Entry points:**
- `LUTLoader` class (src/render/LUTLoader.h:13) — static methods for .cube parsing and GL upload
- `TextureManager::loadLUT()` (src/render/TextureManager.h:36) — wrapper (never called)

**Implementation chain:**
1. `LUTLoader::loadCubeFile()` (src/render/LUTLoader.cpp:7) parses .cube file: skip comments (#), TITLE, DOMAIN_MIN, DOMAIN_MAX lines; read `LUT_3D_SIZE N`; parse N^3 RGB float triples into `std::vector<float>`
2. Validates: `lutSize > 0` and `data.size() == lutSize^3 * 3`; returns 0 on mismatch (LUTLoader.cpp:56)
3. Creates GL_TEXTURE_3D via `glGenTextures` + `glTexImage3D` with GL_RGB32F internal format, trilinear filtering (`GL_LINEAR` min/mag), clamp-to-edge on all 3 axes (LUTLoader.cpp:63-77)
4. `LUTLoader::releaseLUT()` (LUTLoader.cpp:84) calls `glDeleteTextures` to free the GL handle
5. `TextureManager::loadLUT()` (TextureManager.cpp:148) delegates directly to `LUTLoader::loadCubeFile()` — **never called** anywhere in the codebase
6. `TextureManager::releaseLUT()` (TextureManager.cpp:153) delegates to `LUTLoader::releaseLUT()` — also never called
7. The "Color Grade" effect uses a hardcoded GLSL shader, not a 3D LUT texture

**Data flow:**
1. Caller provides `juce::File` path to a .cube file
2. `LUTLoader::loadCubeFile()` reads entire file into a `juce::String` via `file.loadFileAsString()` (LUTLoader.cpp:15)
3. Tokenizes into lines via `juce::StringArray::addTokens` with newline delimiter (LUTLoader.cpp:22-23)
4. Iterates lines: skips empty, comment (#), TITLE, DOMAIN_MIN/MAX; extracts `LUT_3D_SIZE N`; parses remaining lines as space/tab-separated R G B float triples into `std::vector<float>` (LUTLoader.cpp:25-53)
5. Reserves `N^3 * 3` floats in the vector on seeing LUT_3D_SIZE (LUTLoader.cpp:38)
6. Uploads to GPU: `glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB32F, N, N, N, 0, GL_RGB, GL_FLOAT, data.data())` (LUTLoader.cpp:73-75)
7. Returns `GLuint` texture handle (or 0 on any failure)
8. **Dead end:** no consumer ever calls `loadCubeFile()` or `TextureManager::loadLUT()` — the returned texture handle is never bound to a shader uniform

**Dependencies & services:**
- `juce_opengl` — GL types (`GLuint`), GL function wrappers (`glGenTextures`, `glTexImage3D`, `glTexParameteri`, `glDeleteTextures`, `GL_TEXTURE_3D`, `GL_RGB32F`, etc.) via `juce::gl` namespace
- `juce_core` — `juce::File` (file I/O), `juce::String` / `juce::StringArray` (parsing)
- OpenGL 4.1 — `GL_TEXTURE_3D` and `GL_RGB32F` require OpenGL 1.2+ (satisfied by the project's OpenGL 4.1 minimum)
- No external libraries — pure JUCE + OpenGL, no third-party .cube parser

**Storage:** Reads .cube files from disk via `juce::File::loadFileAsString()`. No persistent writes. The entire file content is held in memory as a `juce::String` during parsing, then discarded after GPU upload.

**Config:**
- Format: .cube only, GL_RGB32F texture, trilinear filtering
- No size cap: 64^3 LUT = 3.1MB VRAM (fine), 256^3 = 192MB VRAM (excessive)

**Failure modes:**
- File not found: `file.existsAsFile()` returns false, logs `[LUTLoader] File not found: {path}` to stderr, returns 0 (LUTLoader.cpp:9-12)
- Empty file: `content.isEmpty()` true, returns 0 silently with no log (LUTLoader.cpp:16-17)
- Missing `LUT_3D_SIZE` header: `lutSize` stays 0, validation fails at line 56, logs `[LUTLoader] Invalid .cube file: size=0, entries={N}` to stderr, returns 0
- Entry count mismatch (truncated or malformed file): `data.size() != lutSize^3 * 3`, same validation failure and log, returns 0
- Malformed float values: `juce::String::getFloatValue()` returns 0.0f for unparseable strings, silently produces wrong LUT data (no error)
- GL upload failure: no error check after `glTexImage3D`; if GL context is missing or out of VRAM, texture may be invalid but non-zero `texId` is still returned
- Thread safety: header comment states "Must be called on the GL thread"; no internal guard. Calling from the wrong thread causes undefined GL behavior

**Test coverage:**
- No dedicated LUT tests exist. `LUTLoader.cpp` is compiled into the test binary (tests/CMakeLists.txt:97, :198) but no test file exercises `loadCubeFile()` or `releaseLUT()`.

**Gotchas:**
- DOMAIN_MIN/MAX lines parsed but ignored — non-[0,1] domains produce incorrect color grades.
- Entire file loaded into memory as a single string before line-by-line parsing. Large .cube files (256^3 = ~50MB text) spike memory briefly.
- No GL error checking after `glTexImage3D` — an out-of-VRAM condition returns a non-zero texture handle that may render garbage.
- `releaseLUT()` guards against texId == 0 but callers have no tracking — passing a stale non-zero handle after prior release to `glDeleteTextures` is undefined.

**Why ghost:** `TextureManager::loadLUT()` is never called. No UI exists to browse/load .cube files. No API endpoint exposes LUT loading. Infrastructure is complete but disconnected from any consumer.

**Activation estimate:** LOW (~1-2 days) — add file picker UI, bind returned texture to a color grading shader uniform, wire through the effect chain or clip pipeline.

---

## 20. Camera Input (Ghost Feature) [C]
**Status: Ghost — code exists, compile-gated behind AUDIODNA_HAS_CAMERA, not connected to v2 compositor**

**What it does:** Defines camera as a clip media type and provides v1-era infrastructure for live camera input (device enumeration, open/close, frame capture to GL texture). Camera frames reach the v1 preview panel and output window renderers but are NOT routed through the v2 deck/layer/clip compositor pipeline.

**Entry points:**
- `Clip::MediaType::Camera` (src/model/Clip.h:17) — enum value in the MediaType enum
- `MainComponent::openCamera()` (src/MainComponent.cpp:2244) — opens a camera device by index
- `MainComponent::closeCamera()` (src/MainComponent.cpp:2273) — tears down the active camera
- `MainComponent::refreshCameraList()` (src/MainComponent.cpp:2232) — enumerates available devices into the combo box
- `MainComponent::imageReceived()` (src/MainComponent.cpp:2143) — `CameraDevice::Listener` callback, dispatches frames
- `Renderer::queueCameraFrame()` (src/render/Renderer.cpp:51) — thread-safe frame queue for v1 preview
- `OutputRenderer::queueCameraFrame()` (src/ui/OutputWindow.cpp:42) — thread-safe frame queue for output window

**Implementation chain:**
1. **Build gate:** Camera code is compile-gated by `AUDIODNA_HAS_CAMERA`, set to 1 on macOS and Windows, 0 on Linux (CMakeLists.txt:354-358). When enabled, `JUCE_USE_CAMERA=1` is also defined, enabling `juce_video` camera support (CMakeLists.txt:364).
2. **Device enumeration:** `refreshCameraList()` calls `juce::CameraDevice::getAvailableDevices()`, populates `cameraSelector_` combo box with "Off" (id=1) followed by device names (id = index+2) (MainComponent.cpp:2232-2241).
3. **Open:** `openCamera(deviceIndex)` calls `closeCamera()` first, then `juce::CameraDevice::openDevice(deviceIndex, 0, 0, 1920, 1080, false)` — min size 0x0 (system default), max size 1920x1080, no high-quality stills (MainComponent.cpp:2244-2270).
4. **Listener registration:** On success, sets `cameraActive_ = true` and calls `cameraDevice_->addListener(this)` to receive frames via the `CameraDevice::Listener` interface (MainComponent.cpp:2265-2268).
5. **Frame dispatch:** `imageReceived(const juce::Image&)` is called by JUCE on the camera thread. It forwards the frame to two destinations: `previewPanel_.queueCameraFrame(image)` and optionally `outputWindow_->getRenderer().queueCameraFrame(image)` (MainComponent.cpp:2143-2151).
6. **Close:** `closeCamera()` removes the listener, resets the `unique_ptr<CameraDevice>`, sets `cameraActive_ = false` (MainComponent.cpp:2273-2281). Also called in destructor (MainComponent.cpp:1177).
7. **UI wiring:** `cameraSelector_.onChange` lambda reads selected ID: 1 = closeCamera(), >1 = openCamera(selected - 2) (MainComponent.cpp:170-176). Camera label and selector are laid out in Row 1 of `resized()` (MainComponent.cpp:1358-1361).

**Data flow:**
1. Camera hardware delivers frames to JUCE's internal camera thread
2. JUCE calls `MainComponent::imageReceived(const juce::Image&)` on the camera thread
3. Frame is forwarded to `Renderer::queueCameraFrame()`: acquires `cameraFrameMutex_`, copies `juce::Image` to `pendingCameraFrame_`, sets `hasPendingCameraFrame_ = true` (Renderer.cpp:51-56)
4. On next GL render pass, `Renderer::renderOpenGL()` acquires `cameraFrameMutex_`, calls `texMgr_.uploadImage(pendingCameraFrame_)` which converts JUCE ARGB to RGBA, flips Y, uploads via `glTexImage2D` (or `glTexSubImage2D` if same size) to `imageTexID_` (Renderer.cpp:127-135, TextureManager.cpp:27-94)
5. The uploaded texture replaces the main image texture (`imageTexID_`) — subsequent v1 effect chain renders use it as input
6. Same path for `OutputRenderer` (OutputWindow.cpp:42-47, 82-90)
7. **Dead end for v2:** `CompositorEngine::getClipTexture()` (CompositorEngine.cpp:1045-1061) handles Image, Source, Video, ImageSequence — **Camera is not handled**, falls through to `return 0`. A clip with `MediaType::Camera` produces no texture in the compositor.
8. `Clip::cameraDeviceIndex` is serialized (Clip.cpp:10, :90) but never read by the compositor or any runtime path other than `replaceContent()`.

**Dependencies & services:**
- `juce_video` — `juce::CameraDevice` (device enumeration, open, listener interface), compile-gated by `JUCE_USE_CAMERA=1`
- `juce_gui_basics` — `juce::ComboBox` for camera selector UI, `juce::Label` for camera label
- `juce_graphics` — `juce::Image` for frame transfer between camera thread and GL thread
- `juce_opengl` — GL texture upload via `TextureManager::uploadImage()` (ARGB-to-RGBA conversion + `glTexImage2D`)
- `juce_core` — `juce::StringArray` for device list
- Platform requirement: macOS or Windows (Linux excluded at build time, CMakeLists.txt:354-357)

**Storage:** N/A — camera frames are transient in-memory images. `Clip::cameraDeviceIndex` is serialized to composition JSON (Clip.cpp:10, :90) but this value is never used at runtime to open a camera.

**Config:**
- Camera resolution: min 0x0 (system default), max 1920x1080 (hardcoded in `openCamera`, MainComponent.cpp:2254-2257)
- High-quality stills: disabled (false parameter in `openDevice`)
- Platform: macOS and Windows only (`AUDIODNA_HAS_CAMERA` compile gate)
- To activate in v2 compositor: needs a Camera case in `CompositorEngine::getClipTexture()` that retrieves the camera frame as a GL texture

**Failure modes:**
- Device index out of range: `openCamera()` returns early without error log (MainComponent.cpp:2249-2250)
- Camera fails to open (permissions denied, device busy): `CameraDevice::openDevice()` returns nullptr, `fileLabel_` displays "Camera failed to open", `cameraActive_` stays false (MainComponent.cpp:2259-2262)
- No devices available: `refreshCameraList()` populates only the "Off" entry, no crash
- Camera thread frame delivery while GL context is being destroyed: `cameraFrameMutex_` protects the handoff, but if `Renderer` is destroyed mid-frame the queued image is silently dropped
- Frame format mismatch: `TextureManager::uploadImage()` converts any JUCE Image to ARGB then RGBA, so format is normalized; invalid images (`!image.isValid()`) return false (TextureManager.cpp:29-30)
- Linux build: all camera code is excluded by `#if AUDIODNA_HAS_CAMERA` guards; `Clip::MediaType::Camera` enum still exists but is inert

**Test coverage:**
- No camera-related tests exist. No test file references `openCamera`, `closeCamera`, `imageReceived`, or `MediaType::Camera`.

**Gotchas:**
- **Camera selector IS visible in v2 layout on macOS/Windows** — it is compile-gated (`#if AUDIODNA_HAS_CAMERA`), not hidden via `setVisible(false)`. When the flag is enabled, the selector appears in Row 1 of the layout (MainComponent.cpp:1358-1361).
- **Camera frames replace the main image texture** — `texMgr_.uploadImage()` overwrites `imageTexID_`, so an active camera clobbers any loaded still image in the v1 pipeline.
- **Frame copy on camera thread** — `imageReceived()` copies the `juce::Image` (reference-counted, shallow copy) into `pendingCameraFrame_` under mutex. The actual pixel data copy happens on the GL thread during `uploadImage()`.
- **Renderer camera queue is always compiled** — `Renderer::queueCameraFrame()`, `pendingCameraFrame_`, `cameraFrameMutex_`, and `hasPendingCameraFrame_` are NOT behind `#if AUDIODNA_HAS_CAMERA`; only the MainComponent camera device/selector are gated.
- **Serialized but unused** — `cameraDeviceIndex` is written to and read from composition JSON, but no code path uses the deserialized value to open a camera at load time.

**Why ghost:**
- `CompositorEngine::getClipTexture()` handles Image, Source, Video, ImageSequence — but **Camera is not handled** (falls through to `return 0`)
- Camera frames go to the v1 preview panel and output window renderers only, not the v2 compositor pipeline
- Even on macOS/Windows where the camera UI compiles in, selecting a camera device feeds frames to the v1 texture path, not to any clip in the v2 deck

**Activation estimate:** MEDIUM (~3-5 days) — add Camera case in `CompositorEngine::getClipTexture()` to retrieve GL texture from camera frame queue, wire camera device lifecycle into clip model (so a Clip with MediaType::Camera opens/closes the device), handle the camera-thread-to-GL-thread frame transfer within the compositor path.

---

## 21. v1/v2 Layout Component Split [C]
**What it does:** Three complete UI components from v1 remain in the codebase but are hidden (`setVisible(false)`) in the v2 layout, plus ~15 hidden v1 controls in MainComponent. Two v2 components (WaveformDisplay, Knob) are conditionally visible based on layout state.

**Hidden v1 components:**

- **AudioReadoutPanel** (src/ui/AudioReadoutPanel.h, 70 LOC header) — left panel showing audio features at 30fps: RMS/peak meters (its "ZCR" label is vestigial — ZCR is not computed anywhere in src), 7-band energy bars, beat phase, bar indicator, onset flash, structural/genre state. Permanently hidden at MainComponent.cpp:1399 (dead in v2).
- **SpectrumDisplay** (src/ui/SpectrumDisplay.h, 47 LOC header) — 7-band energy bars with attack/release smoothing, peak hold (20 frames), color-coded gradient. Hidden at MainComponent.cpp:1400.
- **EffectsRackPanel** (src/ui/EffectsRackPanel.h, 99 LOC header) — right-side effect chain with rotary knobs and mapping controls. Superseded by EffectStackView (v2). Hidden at MainComponent.cpp:1401.

**Active v2 UI components (conditionally visible):**

- **WaveformDisplay** (src/ui/WaveformDisplay.h, src/ui/WaveformDisplay.cpp) — Scrolling waveform with peak envelope and RMS fill. Reads raw audio samples from `AnalysisThread::getWaveformSamples()` at 30fps (`startTimerHz(30)`). Each timer tick computes one column: min/max sample values (peak envelope lines, cyan, 0.7 alpha) and RMS (filled area, cyan, 0.15 alpha). 512-column circular buffer (`kMaxColumns`) scrolls right-to-left. Peak hold lines (yellow) persist for 30 frames (~1s at 30fps) then decay at 0.97/frame. Visible in normal layout below the preview panel (12% of preview area height, min 30px); hidden in expanded/programming mode. Constructed with a reference to `AnalysisThread` — no thread ownership.

- **Knob** (src/ui/Knob.h) — Rotary parameter control for the effects rack. Wraps a `ResettableSlider` with name/value labels. Features a mapping indicator ring: a colored arc drawn behind the knob arc in magenta when a mapping source is assigned (`setMappingIndicator(sourceName)`), dim when unmapped. `isMapped()` returns the current mapping state. Preferred size: 64x80px (`kPreferredWidth`/`kPreferredHeight`).

**Entry points:**
- `AudioReadoutPanel` constructor: `MainComponent.h:140` — stack member `audioReadoutPanel_{analysisThread_, analysisThread_.getFeatureBus()}`
- `SpectrumDisplay` constructor: `MainComponent.h:141` — stack member `spectrumDisplay_{analysisThread_.getFeatureBus()}`
- `EffectsRackPanel` constructor: `MainComponent.cpp` — heap-allocated `std::unique_ptr<EffectsRackPanel>` (`MainComponent.h:146`)
- `WaveformDisplay` constructor: `MainComponent.h:139` — stack member `waveformDisplay_{analysisThread_}`
- `Knob` — instantiated dynamically inside `EffectsRackPanel::rebuildUI()` as `ParamKnob.knob` per effect parameter
- Visibility toggle: `MainComponent::resized()` at lines 1316-1327 (expanded mode hides all), lines 1398-1401 (v1 panels always hidden in normal layout), lines 1467-1469 (waveformDisplay visible in normal layout)

**Implementation chain:**
1. All components are created during `MainComponent` construction (stack members for AudioReadoutPanel/SpectrumDisplay/WaveformDisplay, `unique_ptr` for EffectsRackPanel)
2. `MainComponent::resized()` determines visibility:
   - If `signalBarExpanded` is true (lines 1316-1327): ALL panels hidden including waveform, preview, deck, inspector, browser, timing
   - In normal layout (lines 1398-1401): `audioReadoutPanel_`, `spectrumDisplay_`, and `effectsRackPanel_` are unconditionally `setVisible(false)` — this is the v1/v2 split
   - WaveformDisplay: `setVisible(true)` and positioned below preview panel (line 1467-1469) in normal layout only
3. Knob instances live inside EffectsRackPanel's `EffectSection.paramKnobs` vector — created per-effect on `rebuildUI()`, destroyed on chain change

**Data flow:**
```
AudioReadoutPanel: FeatureBus -> timerCallback() (30fps) -> displaySnap_ (smoothed) -> paint()
SpectrumDisplay:   FeatureBus -> timerCallback() (30fps) -> displayBands_[7] (attack/release smoothed) -> paint()
WaveformDisplay:   AnalysisThread::getWaveformSamples() -> timerCallback() (30fps) -> rawBuffer_ -> Column{min,max,rms} -> columns_[512] circular buffer -> paint()
EffectsRackPanel:  EffectChain + MappingEngine -> timerCallback() -> Knob slider values + mapping indicator state -> paint()
Knob:              Parent sets slider value -> ResettableSlider -> paint() (includes mapping arc if setMappingIndicator called)
```

**Dependencies & services:**
- `juce::Component` — base class for all UI components
- `juce::Timer` — 30fps update tick for AudioReadoutPanel, SpectrumDisplay, WaveformDisplay, EffectsRackPanel
- `FeatureBus` — read-only audio feature data for AudioReadoutPanel and SpectrumDisplay
- `AnalysisThread` — raw waveform sample source for WaveformDisplay (`getWaveformSamples()`)
- `MappingEngine` — mapping state for EffectsRackPanel's per-knob indicator rings
- `EffectChain` + `EffectLibrary` — effect parameters for EffectsRackPanel knob values
- `AudioDNALookAndFeel` — color constants (`kSurface`, `kPanelBorder`, `kAccentCyan`, `kTextPrimary`, `kTextSecondary`, `kMeterYellow`)
- `ResettableSlider` (via `UniversalParamControl.h`) — slider base for Knob
- `MappingEditor` — popup for configuring individual mappings (owned by EffectsRackPanel)

**Storage:** N/A — real-time in-memory rendering, no persistence

**Config:**
- `WaveformDisplay::kMaxColumns = 512` — circular buffer width
- `WaveformDisplay::kPeakHoldFrames = 30` (~1s at 30fps), `kPeakDecayRate = 0.97f`
- `SpectrumDisplay::kPeakHoldFrames = 20` (~0.67s at 30fps), `kPeakDecay = 0.95f`
- `SpectrumDisplay::kAttackAlpha = 0.6f`, `kReleaseAlpha = 0.08f`
- `Knob::kPreferredWidth = 64`, `kPreferredHeight = 80`
- Waveform height: `max(30, int(previewArea.height * 0.12))` — hardcoded in `MainComponent::resized()` line 1467
- Hidden v1 controls (~15): `audioSourceLabel_`, `audioSourceSelector_`, `inputGainLabel_`, `inputGainSlider_`, `masterLevelSlider_`, `masterLevelLabel_`, `displaySelector_`, `outputLabel_`, `fpsLabel_`, `cpuLabel_`, `viewportLabel_`, `resolutionSelector_`, `randomLabel_`, `beatRandomToggle_`, `beatCountSelector_`, `syncButton_` — all `setVisible(false)` at lines 1331-1346
- Band colors: 7-element arrays in both AudioReadoutPanel and SpectrumDisplay (red through purple gradient), defined as `static constexpr`

**Failure modes:**
- Unhiding v1 components without adjusting layout: v1 panels would overlap v2 panels — `resized()` does not allocate space for them in the v2 layout
- EffectsRackPanel shown alongside EffectStackView: both read/write the same `EffectChain` — parameter conflicts possible if both have active knob listeners
- WaveformDisplay shown in expanded mode: `resized()` returns early at line 1327 before reaching the waveform layout code, so it would be visible but at stale bounds

**Test coverage:**
- No dedicated UI component tests for any of these components
- No layout tests verifying visibility states
- WaveformDisplay, AudioReadoutPanel, SpectrumDisplay untested (pure visual components)

**Gotchas:**
- All three hidden components are functional code, just not visible. Could be resurfaced for programming/diagnostic mode.
- EffectsRackPanel is heap-allocated (`unique_ptr`), others are stack members.
- v1 controls add ~200 lines of member declarations to MainComponent.
- WaveformDisplay reads the waveform snapshot via a seqlock (Wave 1-D — strictly torn-read-free; see Feature 2 gotchas).
- Two separate hide paths: lines 1316-1327 (expanded mode) and lines 1398-1401 (normal mode) both hide v1 panels but for different reasons — changing one path without the other creates inconsistency.
- SpectrumDisplay and AudioReadoutPanel both display 7-band energy with independent smoothing parameters — values may visually disagree if both are shown simultaneously.

---

## 22. Ghost Features — Control Domain [C]
**What it does:** Five code-complete features with model/logic but no UI path or incomplete wiring. Each sub-feature has data structures and/or implementation but lacks the UI integration or render pipeline connection needed to activate.

### 22a. MacroBanks — Per-Scope Macro Knobs

**What it does:** 8 "dashboard link" knobs per scope (Clip, Layer, Global). Each macro can be manual or signal-driven, distributes value to linked parameters with per-link range and invert.

**Status:** Global scope fully wired (`MainComponent` creates `globalMacroBank_`, passes to inspectors, MIDI routed). **Clip and Layer scopes NOT INSTANTIATED** — enum values exist but no MacroBank objects created for Clip/Layer. Model structs (Clip, Layer, Deck) lack MacroBank member fields.

**Activation estimate:** MEDIUM (~3 days) — add MacroBank to Clip/Layer structs, wire inspectors, update serialization.

### 22b. Crossfader — Deck Blend Control

**What it does:** Continuous A/B crossfading between decks with blend mode (Alpha/Add/Multiply), behaviour (Cut/Smooth), and curve (Linear/EaseInOut/SCurve).

**Status: NOT FUNCTIONAL for live use.** Model fields exist in Composition struct (`crossfaderPhase`, enums for mode/behaviour/curve). `crossfaderPhase` is NEVER READ outside initialization (default 0.5, no code path reads or writes it at runtime). `crossfaderBlendMode` is read exactly once at `Renderer.cpp:508` — but this is for one-time deck-switch transitions, NOT live crossfading between simultaneously-rendered decks. `crossfaderBehaviour` and `crossfaderCurve` are never read. These fields ARE serialized as of Wave 1-C (present in `Composition::toVar()`/`fromVar()`), though the feature remains model-only. No UI slider exists. No MIDI/OSC binding targets the crossfader. The feature is model-only — no runtime behavior.

**Activation estimate:** HIGH (~1 week+) — requires dual-deck simultaneous rendering (doubles GPU workload), UI, MIDI binding, serialization.

### 22c. Route TargetScope::Clip/Layer

**What it does:** Route struct defines TargetScope enum (Clip, Layer, Global) for scoped signal routing. Currently only Global is wired.

**Status: Routes evaluated but values DISCARDED for Clip/Layer scopes.** `Renderer.cpp:200` only handles `Route::TargetScope::Global` — the routing lambda evaluates signal values for all routes each frame, but non-Global routes silently fall through to the TODO comment at line 205 ("Clip/Layer scope routing needs compositor integration"). The `targetLayerId`/`targetClipId` fields are carried in the Route struct but never read. Users can configure Clip/Layer scoped routes via the data model, but they have zero runtime effect — values are computed then thrown away.

**Design direction (2026-05-22):** Scope precedence locked as Global > Layer > Clip (mixing-desk model). Narrower scope overrides wider. Behavioral rules documented in FEATURE_CONNECTIONS.md and MENTAL_MODELS.md (Model 2: Three Scopes).

**Activation estimate:** MEDIUM (~3 days) — data model complete, needs render pipeline integration.

### 22d. LinkSync::requestBeatAtTime()

**What it does:** Forces all Ableton Link peers to realign to downbeat. Fully implemented (12 LOC in `LinkSync.cpp:63-74`).

**Status:** Never called. No UI button, no binding, no API endpoint. `LinkSync` itself is used: `MainComponent::timerCallback()` calls `linkSync_.update()` and reads `linkSync_.getBPM()` (line 1803-1806), but `requestBeatAtTime()` specifically has zero callers.

**Activation estimate:** LOW (~1 day) — needs only a UI button or binding action.

### 22e. MappingSuggester

**What it does:** Analyzes FeatureSnapshot + genre to recommend source-to-effect mappings. Returns ranked suggestions with scores and reasons. 13 universal + 4 per-genre suggestions.

**Status: REMOVED 2026-07-17 (Wave 0).** Was implemented-but-not-integrated (266 LOC, never instantiated, zero UI/API callers — no button, menu item, keyboard shortcut, or endpoint invoked `suggestMappings()` / `suggestGenreMappings()`). The `.cpp/.h` were deleted; this section is retained as a historical record of the removed ghost.

**Activation estimate:** LOW (~1 day) — needs UI trigger button + conversion from Suggestion to Mapping.

**Entry points:**
- 22a: `MacroBank` class (`src/routing/MacroBank.h:13`). Global instance: `MainComponent.h:207` as `globalMacroBank_{MacroBank::Scope::Global}`. Wired to inspectors at `MainComponent.cpp:901` via `inspectorPanel_->setMacroBank(&globalMacroBank_)`. MIDI write at `MainComponent.cpp:3842-3843`.
- 22b: `Composition` struct fields (`src/model/Composition.h:31-37`): `crossfaderPhase`, `CrossfaderBlendMode`, `CrossfaderBehaviour`, `CrossfaderCurve`. Single read at `Renderer.cpp:508`.
- 22c: `Route` struct (`src/routing/Route.h:8`): `TargetScope` enum (line 19), `targetLayerId` (line 21), `targetClipId` (line 22). Routing callback at `Renderer.cpp:198-206`.
- 22d: `LinkSync::requestBeatAtTime()` (`src/sync/LinkSync.cpp:63`). `LinkSync` instance: `MainComponent.h:209` as `linkSync_`.
- 22e: `MappingSuggester` class (`src/mapping/MappingSuggester.h:15`). Public API: `suggestMappings(snapshot, maxSuggestions)` and `suggestGenreMappings(genre, maxSuggestions)`.

**Implementation chain:**
- 22a MacroBanks: `MacroBank` stores 8 `Macro` structs in `std::array`. `updateValues(SignalRegistry&)` reads signal values for signal-driven macros or uses `manualValue` for manual. `MacroPanel` UI displays 8 knob slots, reads/writes via `setMacroBank()`. `InspectorPanel` distributes the bank pointer to `ClipInspector`, `LayerInspector`, `CompositionInspector`. `UniversalParamControl` populates a macro source dropdown from the bank (line 448). MIDI handler writes `globalMacroBank_.getMacro(idx).manualValue = value`. Per-link distribution (MacroLink vector with `RouteTarget`, `outputMin`, `outputMax`, `inverted`) is defined but no code iterates `links` to apply values to target parameters.
- 22b Crossfader: Fields declared in `Composition.h:31-37`. `crossfaderBlendMode` is read at `Renderer.cpp:508` as a uniform for the deck transition shader — this is a one-time transition blend, not a live crossfader. `crossfaderPhase` (float [0,1]) is never read. `crossfaderBehaviour` and `crossfaderCurve` enums are never read. None of these fields appear in `toVar()`/`fromVar()` — not serialized.
- 22c Route scope: `RoutingEngine::processFrame()` iterates routes and calls a lambda. The lambda at `Renderer.cpp:198-206` checks `route.targetScope == Global` and writes to `effectChain_`. Non-Global scopes fall through to the TODO comment with no action.
- 22d requestBeatAtTime: Captures Ableton Link session state, calls `sessionState.requestBeatAtTime(0.0, now, quantum_)`, commits back. Guarded by `#if AUDIODNA_HAS_LINK` and `enabled_` atomic check.
- 22e MappingSuggester: Stateless utility. `suggestMappings()` calls `addUniversalSuggestions()` (13 mappings based on feature activity levels) + `addGenreSuggestions()` (genre-specific), sorts by relevance descending, truncates to `maxSuggestions`. `suggestGenreMappings()` calls only `addGenreSuggestions()`. Each `Suggestion` carries `sourceName`, `targetCategory`, `targetEffect`, `targetParam`, `curveType`, `reason`, `relevance`.

**Data flow:**
- 22a: `SignalRegistry` (cached signal values) -> `MacroBank::updateValues()` -> `Macro.currentValue` -> (gap: no code distributes to `MacroLink.target` parameters)
- 22b: `Composition.crossfaderPhase` <- never written after init (default 0.5). `Composition.crossfaderBlendMode` -> `Renderer.cpp:508` uniform `u_blendMode` (deck transition shader only)
- 22c: `Signal values` -> `RoutingEngine::processFrame()` -> lambda with `Route` -> only `Global` scope reaches `EffectChain::setParamValue()`. `Clip/Layer` scope routes are evaluated but their values are discarded (no handler)
- 22d: (no data flow — `requestBeatAtTime()` is a one-shot command to Ableton Link peers, no return value)
- 22e: `FeatureSnapshot` + genre -> `MappingSuggester::suggestMappings()` -> `vector<Suggestion>` (sorted by relevance). No downstream consumer exists.

**Dependencies & services:**
- 22a: `SignalRegistry` (signal value cache), `RouteTarget` struct (`Route.h`), JUCE (no direct JUCE deps — pure C++ model)
- 22b: `Composition` struct (model), OpenGL shader uniforms (Renderer reads `crossfaderBlendMode`)
- 22c: `Route` struct, `RoutingEngine`, `Renderer`, `EffectChain` — render pipeline integration needed
- 22d: Ableton Link library (`ableton/Link.hpp`), gated by `AUDIODNA_HAS_LINK` compile flag. Uses `link_.captureAppSessionState()` / `commitAppSessionState()`
- 22e: `FeatureSnapshot` (audio analysis data), `GenreDetector` (genre enum). No JUCE dependency — pure C++ utility

**Storage:** N/A — all in-memory model state. Crossfader fields are not serialized. MacroBank is not serialized. Route scope fields exist in the Route struct but are never persisted (no save/load code handles `targetLayerId`/`targetClipId`).

**Config:**
- 22a: `MacroBank::kNumMacros = 8`. Default macro names: "Link 1" through "Link 8". Default `manualValue = 0.5f`. Scope enum: `{Clip, Layer, Global}`.
- 22b: `crossfaderPhase` default `0.5f`. `CrossfaderBlendMode` enum: `{Alpha, Add, Multiply}`. `CrossfaderBehaviour` enum: `{Cut, Smooth}`. `CrossfaderCurve` enum: `{Linear, EaseInOut, SCurve}`.
- 22c: `Route::TargetScope` enum: `{Clip, Layer, Global}`. `targetLayerId` and `targetClipId` default to 0.
- 22d: `quantum_` default `4.0` (4/4 time). Phase reset target: beat 0.0 (hardcoded in `requestBeatAtTime`).
- 22e: Default `maxSuggestions = 8` for general, `6` for genre-specific. Activity thresholds are per-feature (hardcoded in `addUniversalSuggestions`).

**Failure modes:**
- 22a: Instantiating Clip/Layer MacroBanks without updating serialization would cause data loss on save/load. Inspector UI would show macro knobs pointing to a bank that does not persist.
- 22b: Setting `crossfaderPhase` to 0.0 or 1.0 with no crossfader rendering logic has no effect (value is ignored). Enabling dual-deck render without doubling GPU resources would cause frame drops or OOM.
- 22c: Routing to Clip/Layer scope silently discards the value — no error, no warning. User could configure routes that appear connected but have no effect.
- 22d: Calling `requestBeatAtTime()` when Link is disabled results in early return, no error. Calling with no peers succeeds locally but has no effect.
- 22e: No failure modes — stateless utility returns empty vector if no features are active.

**Test coverage:**
- 22a: No MacroBank unit tests. `test_routing_engine.cpp` tests routing but not macro integration.
- 22b: No crossfader tests. `test_composition.cpp` tests serialization but crossfader fields are not serialized so not covered.
- 22c: `test_routing_engine.cpp` exists but does not test Clip/Layer scope routing.
- 22d: No LinkSync tests.
- 22e: No MappingSuggester tests.

**Gotchas:**
- 22a: `MacroBank::updateValues()` is called but `Macro.links` vector is never iterated to distribute values to linked parameters — the "last mile" distribution is unimplemented despite the data model being complete.
- 22b: `crossfaderBlendMode` at `Renderer.cpp:508` is used for deck transitions (one-time blend during `deckTransitionProgress_`), not live crossfading. Renaming or repurposing this field for live crossfading would break existing deck transition behavior.
- 22c: The routing lambda at `Renderer.cpp:198-206` runs per-frame per-route. Adding Clip/Layer handling requires access to the compositor's per-layer effect chains, which the lambda does not currently have.
- 22e: `MappingSuggester::Suggestion` uses `std::string` for all fields (7 strings per suggestion, up to 8 suggestions) — heap allocation on every call. Not real-time safe but acceptable since it would only be called on user action, not per-frame.
- Across all 5 sub-features: none are referenced by any test file in `tests/`.

---

## 23. TimingWindow (Stub UI) [C]
**What it does:** A tabbed component with 3 tabs (BPM, Routing, Oscillators) positioned in the center-bottom panel area. Tab switching works with visual feedback (active tab accent line). All 3 tabs render only a placeholder text label showing the tab name — no functional content exists in any tab.

**Entry points:**
- `TimingWindow` class: `src/ui/TimingWindow.h:8`, `src/ui/TimingWindow.cpp:3`
- Construction: `MainComponent.cpp:1060` — `timingWindow_ = std::make_unique<TimingWindow>()`
- Added to parent: `MainComponent.cpp:1061` — `addAndMakeVisible(timingWindow_.get())`
- Layout: `MainComponent.cpp:1475-1478` — positioned in `timingArea` bounds, set visible
- Hidden in expanded mode: `MainComponent.cpp:1326` — `timingWindow_->setVisible(false)`
- Menu toggle: `MenuBarModel.h:112` — `kViewTimingWindow` command ID, `MenuBarModel.cpp:178` — "Timing Window" menu item
- Member: `MainComponent.h:241` — `std::unique_ptr<TimingWindow> timingWindow_`

**Implementation chain:**
1. `TimingWindow()` constructor (TimingWindow.cpp:3-18): Creates 3 `juce::TextButton` instances (`bpmTabBtn_`, `routingTabBtn_`, `oscTabBtn_`). Each button's `onClick` calls `setActiveTab(tab)`. All buttons added via `addAndMakeVisible()`. Initial tab colors set via `updateTabButtonColors()`.
2. `setActiveTab(Tab)` (line 77-82): Sets `activeTab_` enum, calls `updateTabButtonColors()` + `repaint()`.
3. `updateTabButtonColors()` (line 84-97): Active tab gets lighter background (`0xff3a3a5c`) and white text (`0xffffffff`). Inactive tabs get dark background (`0xff1a1a2e`) and dim text (`0xff606070`).
4. `resized()` (line 68-75): Tab bar takes top `kTabBarHeight` (26px). Width divided equally among 3 buttons (`tabWidth = width / 3`).
5. `paint()` (line 21-66): Fills background (`0xff1a1a1a`), draws tab bar background (`0xff222222`), draws panel border, draws 3px cyan accent line under active tab, draws placeholder text (tab name) centered in content area below tab bar at 11pt font, secondary text color at 0.4 alpha.

**Data flow:**
```
User clicks tab button -> onClick lambda -> setActiveTab(Tab) -> activeTab_ enum updated
-> updateTabButtonColors() (button colors) + repaint() -> paint() draws accent line + placeholder text
```
No external data flows into TimingWindow — it receives no analysis data, BPM, routing state, or oscillator configuration. It is purely a visual stub.

**Dependencies & services:**
- `juce::Component` — base class
- `juce::TextButton` — 3 tab buttons
- `AudioDNALookAndFeel` — color constants (`kSurface`, `kPanelBorder`, `kAccentCyan`, `kTextPrimary`, `kTextSecondary`)
- No dependencies on analysis, model, routing, or signal systems

**Storage:** N/A — no data to persist

**Config:**
- `kTabBarHeight = 26` (pixels) — height of tab button row
- Tab background: active `0xff3a3a5c`, inactive `0xff1a1a2e`
- Tab text: active `0xffffffff`, inactive `0xff606070`
- Panel background: `0xff1a1a1a`
- Tab bar background: `0xff222222`
- Accent line: 3px height, `kAccentCyan` color
- Placeholder font: 11pt, `kTextSecondary` at 0.4 alpha
- Layout position: determined by `vDividerFrac_` array in MainComponent — second panel in the 4-panel bottom row (preview | timing | inspector | browser)

**Failure modes:**
- No runtime failure modes — the component is a static visual stub with no data dependencies
- If TimingWindow were populated with real controls without updating `resized()`, content would overlap the 26px tab bar or clip outside bounds

**Test coverage:**
- No tests. TimingWindow is not referenced in any test file.

**Gotchas:**
- Tab width calculation `tabBar.getWidth() / 3` uses integer division — if panel width is not divisible by 3, the rightmost tab gets the remainder (could be 1-2px wider or narrower).
- The `Tab` enum is `int`-backed (`Tab : int`) with values 0/1/2 — switch statements have no default case, so adding a 4th tab without updating all switch statements would cause undefined paint behavior.
- The "What is stub" content (BPM detection, routing config, oscillator config) represents intended functionality that has zero implementation — not even data model stubs or interface definitions exist for tab content.
- Panel position depends on the resizable vertical divider system in MainComponent (`vDividerFrac_[3]`) — initial fraction `0.22-0.50` range.
- Design direction (2026-05-22): two-lane timeline structure planned — HITS lane (beat-quantized event pills) + REC lane (time-tick density curve). See BORIS_DECISIONS.md and FEATURE_CONNECTIONS.md Scenario 11 for behavioral specs.

---

## 24. Dual-Mode System (ProgrammingMode — REMOVED Wave 0) [C]
**What it does:** Removed. Before deletion, `ProgrammingMode` was a vestigial always-hidden overlay component that was never shown; the View menu's "Programming Mode" item bypassed it entirely and just toggled the SignalBar between Expanded/Normal display size.

**Status: REMOVED 2026-07-17 (Wave 0)** — the `ProgrammingMode` component (.cpp/.h), its MainComponent construction/wiring, and the View → "Programming Mode" menu item were all deleted. This section is retained as a historical record. (Before removal: a vestigial always-hidden component; the View menu's "Programming Mode" item merely toggled the SignalBar between Expanded/Normal display size, bypassing the component entirely — a binary fullscreen-SignalBar toggle, not a graduated mode system.)

**Entry points:**
- `ProgrammingMode` class: `src/ui/ProgrammingMode.h:10` (31 LOC header), `src/ui/ProgrammingMode.cpp` (64 LOC)
- Construction: `MainComponent.cpp:511` — `programmingMode_ = std::make_unique<ProgrammingMode>(*signalBar_)`
- Added as child (hidden): `MainComponent.cpp:512` — `addChildComponent(programmingMode_.get())` (note: `addChildComponent`, NOT `addAndMakeVisible` — starts invisible)
- Always hidden: `MainComponent.cpp:1312-1313` — unconditional `programmingMode_->setVisible(false)` in `resized()`
- Menu handler: `MainComponent.cpp:3199-3210` — `kViewProgrammingMode` directly toggles `signalBar_->setDisplaySize()` between Expanded/Normal, never touches `programmingMode_`
- Menu item: `MenuBarModel.cpp:181` — "Programming Mode" in View menu
- Member: `MainComponent.h:214` — `std::unique_ptr<ProgrammingMode> programmingMode_`

**Implementation chain:**
1. `ProgrammingMode(SignalBar& signalBar)` constructor (ProgrammingMode.cpp:3-13): Stores reference to SignalBar. Creates header label ("Programming Mode" in cyan, 14pt bold) and "Exit" close button. Both added via `addAndMakeVisible()`. Close button's `onClick` calls `setActive(false)`.
2. `setActive(bool)` (line 15-36): If `active` changed, sets `active_` flag. If activating: calls `signalBar_.setDisplaySize(SignalStrip::DisplaySize::Expanded)`. If deactivating: calls `signalBar_.setDisplaySize(SignalStrip::DisplaySize::Normal)`. Sets own visibility to match `active_`. Triggers parent `resized()`.
3. `paint()` (line 38-49): Dark overlay background (`kBackground` at 0.95 alpha), cyan border at 0.3 alpha.
4. `resized()` (line 51-63): Header row (24px): label on left (200px), close button on right (60px). Remaining area is unused — comment says "signal bar occupies the rest (managed by parent)".

**Actual behavior path (View menu "Programming Mode"):**
1. User selects View > Programming Mode -> `handleMenuCommand(kViewProgrammingMode)` at `MainComponent.cpp:3199`
2. Checks current `signalBar_->getDisplaySize()` — if Expanded, sets Normal; if Normal, sets Expanded
3. Calls `signalBar_->onSizeChanged()` -> triggers `MainComponent::resized()`
4. In `resized()`: `programmingMode_->setVisible(false)` (line 1313, unconditional)
5. If signal bar is expanded (line 1316-1327): all panels hidden (preview, waveform, readout, spectrum, effects rack, deck, inspector, browser, timing) — returns early
6. `ProgrammingMode` component is NEVER shown, NEVER activated, NEVER receives the toggle event

**Data flow:**
```
View menu -> handleMenuCommand(kViewProgrammingMode) -> signalBar_->setDisplaySize(toggle)
-> onSizeChanged callback -> MainComponent::resized() -> expanded check -> hide all panels
```
ProgrammingMode component has no data flow — it is created but never made visible or interacted with. Its `setActive()` method would toggle SignalBar display size if called, but nothing calls it.

**Dependencies & services:**
- `juce::Component` — base class
- `juce::Label` — header label ("Programming Mode")
- `juce::TextButton` — "Exit" close button
- `SignalBar` — reference stored, `setDisplaySize()` would be called by `setActive()` if it were ever invoked
- `SignalStrip::DisplaySize` — enum with at least `Normal` and `Expanded` values
- `AudioDNALookAndFeel` — color constants (`kBackground`, `kAccentCyan`)

**Storage:** N/A — no persistent state

**Config:**
- Header label: "Programming Mode", 14pt bold, `kAccentCyan` color
- Close button text: "Exit", 60px wide
- Header row height: 24px
- Overlay background: `kBackground` color at 0.95 alpha
- Border: `kAccentCyan` at 0.3 alpha, 1px
- Padding: 8px reduced bounds
- `active_` flag: default `false`, never changed at runtime

**Failure modes:**
- Calling `ProgrammingMode::setActive(true)` externally would set `SignalBar` to Expanded and call `setVisible(true)`, but `MainComponent::resized()` immediately sets it back to `setVisible(false)` at line 1313 — the component would flash for one frame then disappear
- The menu command and `ProgrammingMode::setActive()` both toggle SignalBar display size independently — if both paths were active, they could conflict (double-toggle back to original state)

**Test coverage:**
- No tests. ProgrammingMode is not referenced in any test file.

**Gotchas:**
- `addChildComponent` (not `addAndMakeVisible`) at line 512 means ProgrammingMode starts invisible by JUCE convention, AND `resized()` unconditionally hides it — double guarantee that it is never shown.
- The menu handler at line 3199-3210 completely ignores the `ProgrammingMode` component — it talks directly to `signalBar_`. The ProgrammingMode class's `setActive()` method duplicates this logic but is never called.
- `ProgrammingMode::resized()` leaves the area below the header row unused with a comment about SignalBar being "managed by parent" — suggesting the original design intent was for ProgrammingMode to be an overlay with the expanded SignalBar rendered inside its bounds, but this integration was never completed.
- What always runs regardless of mode: keyboard bindings, MIDI, OSC, REST API, audio engine, analysis, render pipeline, output window, Syphon, autopilot, video recording.
- The component is a candidate for removal — it adds dead code with no runtime effect. Its functionality (SignalBar expanded/normal toggle) is fully handled by the menu command handler.

---

## 25. Session Recorder — Performance Event Capture [R]
**What it does:** Records timestamped performance events (parameter changes, clip triggers, transport actions) for session playback and replay. This is event recording, not video — it captures what the performer did, not what the audience saw. Playback reproduces the performance exactly by replaying events at their original timestamps.

**Entry points:**
- `SessionRecorder` class (src/recording/SessionRecorder.h:20) — recording, playback, persistence
- `RecordPanel` class (src/ui/RecordPanel.h) — UI controls (Record/Stop/Play/Save/Load)

**7 event types (`SessionRecorder::EventType` enum):**

| # | Event Type | What it captures | Event fields |
|---|-----------|------------------|--------------|
| 0 | ParameterChange | Effect/source/transform param changed | targetId, paramIndex, value |
| 1 | ClipTrigger | Clip activated in deck | layerIndex, columnIndex |
| 2 | ColumnTrigger | Entire column triggered | columnIndex |
| 3 | MacroChange | Macro knob value changed | macroIndex (via paramIndex), value |
| 4 | TransportChange | Play/pause/stop/speed/reverse | action (string), value |
| 5 | EffectToggle | Effect enabled/disabled/bypassed | effectName (string), enabled (bool) |
| 6 | CuepointJump | Jumped to cuepoint | clipId (via targetId), cuepointIndex (via paramIndex), position (value) |

**Implementation chain:**

*Recording:*
1. User clicks Record in RecordPanel → `SessionRecorder::startRecording()` clears events, saves wall clock start time
2. `recording_` atomic flag set to true
3. Each `record*()` method checks flag, acquires `juce::CriticalSection lock_`, creates `Event` with `timestamp = now - startTime`, appends to `events_` vector
4. User clicks Stop → `stopRecording()` sets flag to false

*Playback:*
1. User clicks Play → `startPlayback()` resets `playbackTime_` and `playbackIndex_` to 0
2. Caller invokes `advancePlayback(dt)` each frame with frame delta time
3. Method accumulates `playbackTime_` and returns pointers to all events whose timestamp <= current time
4. Auto-stops when all events consumed (`playbackIndex_ >= events_.size()`)

*Persistence (JSON):*
1. `saveToFile()` serializes to JSON: `{"version": 1, "events": [...]}` via JUCE `DynamicObject`
2. Each event stores `"t"` (timestamp) + `"type"` (int) + type-specific fields
3. `loadFromFile()` parses JSON, rebuilds `events_` vector, resets playback state

**Data flow:**
```
Performance actions → record*() methods (mutex-protected) → events_ vector → saveToFile() → JSON
JSON → loadFromFile() → events_ vector → advancePlayback(dt) → event pointers → caller applies
```

**Integration:**
- `MainComponent` owns `sessionRecorder_` (stack member, MainComponent.h:208)
- `RecordPanel` receives pointer via `setSessionRecorder()` — provides Record/Stop/Play/Save/Load buttons
- `ApiServer` holds a reference but exposes no REST endpoints for session recording
- **Only 1 of 7 event types is wired:** `recordClipTrigger()` is called from `MainComponent.cpp:2472`. The other 6 `record*()` methods have complete implementations but ZERO callers anywhere in the codebase:

| Event Type | Method | Wired? | Expected Call Site |
|------------|--------|--------|--------------------|
| ClipTrigger | `recordClipTrigger()` | YES (MainComponent.cpp:2472) | Clip activation |
| ParameterChange | `recordParameterChange()` | NO — 0 callers | Effect/source param changes |
| ColumnTrigger | `recordColumnTrigger()` | NO — 0 callers | Column trigger button |
| MacroChange | `recordMacroChange()` | NO — 0 callers | Macro knob changes |
| TransportChange | `recordTransportChange()` | NO — 0 callers | Play/pause/stop/speed |
| EffectToggle | `recordEffectToggle()` | NO — 0 callers | Effect enable/disable |
| CuepointJump | `recordCuepointJump()` | NO — 0 callers | Cuepoint navigation |

**Dependencies & services:**
- JUCE: `CriticalSection` (mutex), `Time::getMillisecondCounterHiRes()` (wall clock), `JSON`/`DynamicObject` (serialization), `File` (I/O)

**Storage:**
- Format: JSON (`{"version": 1, "events": [...]}`)
- Default save location: user-chosen via file dialog (RecordPanel), fallback to ~/Documents
- RecordPanel format selector includes "Video (Future)" placeholder — not implemented

**Config:**
- No configurable parameters — all behavior is fixed
- Timestamps use wall clock delta (millisecond hi-res counter), not audio/beat time

**Failure modes:**
- Save to read-only path → `file.replaceWithText()` returns false, RecordPanel shows "Save failed!" (handled)
- Load malformed JSON → `JSON::parse` returns invalid var, `loadFromFile()` returns false (handled)
- Record called while not recording → early return via atomic flag check (handled)
- Playback with no events → `advancePlayback()` returns empty vector (handled)

**Test coverage:**
- Missing: no dedicated SessionRecorder tests

**Gotchas:**
- `lock_` is `juce::CriticalSection` (recursive mutex) used on every `record*()` call — if recording high-frequency parameter changes, contention possible between UI/MIDI threads and any concurrent readers.
- `advancePlayback()` returns raw pointers into `events_` vector — caller must not modify vector during playback iteration.
- Timestamps are wall-clock-relative, not beat-relative — playback of a session recorded at 120 BPM replayed at 140 BPM will have events at the same wall times, not the same beat positions.
- **Only 1 of 7 event types is wired** — `recordClipTrigger` is the only `record*()` method called from application code (MainComponent.cpp:2472). The other 6 event types have complete implementations but zero callers. See Integration table above for per-type wiring status.
- `Event::action` and `Event::effectName` are `std::string` — heap allocation on every TransportChange/EffectToggle recording event. Not RT-safe if called from audio thread (currently not called from audio thread).
- **High-frequency lock contention risk**: `lock_` (juce::CriticalSection, recursive mutex) is acquired on every `record*()` call. If all 7 event types were wired, high-frequency parameter changes (~60Hz per mapped param) would contend with UI thread, MIDI thread, and playback reader on the same lock. Current single-caller wiring avoids this, but full wiring would need per-type lock-free queues or batching.

---

## 26. Application UI Framework [R]
**What it does:** The UI shell and chrome components that form the application window layout: top bar, preview panel, deck grid, inspector panel, browser panel, signal bar, menu system, preferences, look-and-feel theme, and shared parameter controls. These components orchestrate the domain features (F1-F25) into a usable VJ application interface.

**Entry points:**
- `MainComponent` (src/MainComponent.h) — top-level JUCE component, owns all UI panels
- `AudioDNALookAndFeel` (src/ui/LookAndFeel.h:4) — global visual theme
- `AudioDNAMenuBar` (src/ui/MenuBarModel.h:8) — 9-menu application menu bar

**Implementation chain:**
1. `MainComponent` creates all panels during construction: TopBar, SignalBar, PreviewPanel, DeckView, InspectorPanel, BrowserPanel, TimingWindow, BindingOverlay, MidiLearnOverlay
2. `MainComponent::resized()` performs a 4-region layout: TopBar (top strip), SignalBar (below top bar), main content area (preview | timing | inspector | browser), DeckView (remaining space)
3. `AudioDNALookAndFeel` applied globally — all JUCE widgets inherit the dark theme with cyan/magenta accents
4. `AudioDNAMenuBar` provides 9 menus, each command ID dispatched to `MainComponent::handleMenuCommand()`

**Data flow:**
```
User input → MainComponent → delegates to child panel → panel modifies model/calls back → MainComponent refreshes UI
AudioDNALookAndFeel → all paint() calls use consistent color constants and widget rendering
```

**Dependencies & services:**
- JUCE juce_gui_basics: Component, Timer, LookAndFeel_V4, MenuBarModel, Viewport, DragAndDrop
- JUCE juce_opengl: OpenGLContext (hosted by PreviewPanel's GLHost)

**Config:**
- Layout dividers: `vDividerFrac_[4]` array controls relative widths of 4 bottom panels
- TopBar height: fixed strip at top of window
- SignalBar: 3 display sizes (Minimized ~20px, Normal ~80px, Expanded ~fullscreen)
- DeckView cell size: 90x96px (Resolume-style dense grid)
- All colors defined as `static constexpr` in `AudioDNALookAndFeel`

**Failure modes:**
- Window too small → panels clip or overlap (partially handled — minimum size enforced)
- OpenGL context lost → PreviewPanel re-creates context (handled by JUCE)

**Test coverage:**
- No dedicated UI framework tests. UI components are visual-only.

**Gotchas:**
- `MainComponent::resized()` has two separate code paths: expanded mode (lines 1316-1327, hides everything except SignalBar) and normal mode (remaining code). Changes to one path must be mirrored in the other.
- v1 controls (~15 members) are hidden via `setVisible(false)` in `resized()` but remain as stack/heap members on MainComponent, adding ~200 lines of member declarations.

### 26a. TopBar

**What it does:** The main application toolbar positioned below the menu bar. Contains audio source selector, input gain, transport controls (play/pause/stop), tempo display with animated beat wheel, tap tempo, BPM multiplier buttons (/4, /2, x1, x2, x4), quantize selector, global fade slider, master level slider, output display selector, and FPS/DSP stats.

**Key source files:**
- `TopBar` class (src/ui/TopBar.h:12, src/ui/TopBar.cpp)

**Parent component:** MainComponent (top strip, full window width)

**Controls & interactions:**
- Audio Source section: source selector dropdown, input gain slider
- Transport section: Play/Pause/Stop buttons
- Tempo section: BPM display label, tracker state label, Tap button (8-tap averaging), Resync button
- BPM Multiplier: /4, /2, x1, x2, x4 buttons — multiply detected BPM
- Manual BPM: toggle button + editable text field for manual BPM entry
- Quantize: dropdown selector for beat snap modes
- Fade: global fade slider
- Master Level: master brightness slider
- Output: display selector dropdown
- Stats: FPS label, DSP load label
- Beat wheel: animated circular indicator showing beat phase, painted via `paintBeatWheel()`
- Bar/phrase display: text showing current bar and phrase position
- Updates at 30fps via `juce::Timer`

**Callbacks:** `onTapTempo`, `onResync`, `onBpmMultiplierChanged`, `onQuantizeChanged`, `onManualBpmChanged`

**Gotchas:**
- Tap tempo uses an 8-element circular buffer (`tapTimes_`) with timeout — taps separated by >2s reset the count.
- Beat wheel reads from `displaySnap_` (FeatureBus snapshot) — may show 1-frame-old data.

### 26b. PreviewPanel

**What it does:** Hosts the OpenGL Renderer inside a JUCE Component with [Preview] / [Output] tabs. The Preview tab shows the selected clip/layer solo or full composition; the Output tab shows what goes to the external display. Both render via the same GL renderer — tabs label the context only.

**Key source files:**
- `PreviewPanel` class (src/ui/PreviewPanel.h:12, src/ui/PreviewPanel.cpp)

**Parent component:** MainComponent (first panel in the 4-panel bottom row)

**Controls & interactions:**
- Preview/Output tab buttons (26px tab bar at top)
- GL rendering area (GLHost inner component) — displays the OpenGL output
- Placeholder text "Load an image to see audio-reactive effects" when no image loaded
- Provides accessors: `getRenderer()`, `getMappingEngine()`, `getEffectChain()`
- `loadImage(File)` / `clearImage()` / `queueCameraFrame(Image)` for content loading

**Gotchas:**
- `GLHost` is a separate inner component to isolate GL rendering from JUCE 2D tab bar widgets.
- `Renderer` is a stack member of PreviewPanel — its lifetime is tied to the panel.

### 26c. DeckView

**What it does:** Resolume-style layer x column deck grid. Displays column trigger buttons at top, layer strips on the left, clip cells in the grid, and deck switch tabs at the bottom. Supports horizontal/vertical scrolling when content exceeds the viewport.

**Key source files:**
- `DeckView` class (src/ui/DeckView.h:15, src/ui/DeckView.cpp)

**Parent component:** MainComponent (main content area, below the 4-panel row)

**Controls & interactions:**
- Column trigger buttons (top row, 22px) — click to trigger entire column across all layers
- Layer strips (left side, 250px wide) — per-layer controls and status (see 26d LayerStrip)
- Clip cells (grid, 90x96px each) — per-clip thumbnail and interaction (see 26e ClipCell)
- Deck tabs (bottom row, 24px) — click to switch active deck
- Scrollable viewport when content exceeds visible area
- Multi-selection: `selectCell(layer, column, addToSelection)` with Shift/Cmd support
- Layer selection: `selectLayer(layerIndex)` highlights the layer strip
- `rebuildGrid()` recreates all child components from current deck state
- `refresh()` updates active clip highlights and button states

**Callbacks:** `onClipTriggered`, `onClipSelected`, `onLayerSelected`, `onColumnTriggered`, `onFileDropped`, `onMultiFileDropped`, `onMultiVideoDropped`, `onEffectDropped`, `onSourceDropped`, `onClipMoved`, `onMilkDropDropped`, `onMilkDropPlaylistDropped`, `onDeckSwitched`, `onLayerFoldToggle`, `onLayerReorder`

**Gotchas:**
- Layout constants: `kLayerStripWidth=250`, `kCellWidth=90`, `kCellHeight=96`, `kCellGap=0` — flush grid with 1px borders drawn by cells.
- `setComposition()` must be called before `rebuildGrid()` — reads active deck's layers and clips.
- Deck tabs are dynamic — one per deck in the composition.

### 26d. LayerStrip

**What it does:** Resolume-style layer header displayed in the left side of the DeckView. Shows layer name, clip name, transport controls, clear/bypass/solo buttons, speed slider, keying threshold slider, opacity slider with blend mode dropdown, fade speed with transition mode dropdown, and clip thumbnail with playhead indicator.

**Key source files:**
- `LayerStrip` class (src/ui/LayerStrip.h:23, src/ui/LayerStrip.cpp)

**Parent component:** DeckView (one per layer, 250px wide, in the left column)

**Controls & interactions:**
- Row 1: Clear (X), Bypass (B), Solo (S) buttons + transport controls (Back, Pause, Play, Forward)
- Row 2: Speed slider (S, 0-4x, default 1x), Keying threshold slider (K, 0-1, default 0.1), Opacity slider (V, 0-1, default 1.0) + blend/keying mode dropdown, thumbnail
- Row 3: Layer name label, clip name with playhead indicator, Fade time slider (F, 0-4s, default 0.3s) + transition mode dropdown
- Click on strip background → selects layer for inspection
- Playhead scrubbing via mouse drag on clip name area
- Layer drag reorder support (`onLayerDragReorder`)
- Layer fold toggle (`onFoldToggle`)
- Updates via `juce::Timer` for live playhead position

**Callbacks:** `onSelect`, `onClearClip`, `onBypass`, `onSolo`, `onBlendModeChanged`, `onTransportPlay/Pause/Back/Forward`, `onFoldToggle`, `onLayerDragReorder`

**Gotchas:**
- S/K/V/F slider shorthand: S = Speed (playback rate), K = Key threshold (chroma/alpha key), V = Opacity (layer transparency), F = Fade speed (transition duration). These single-letter labels appear on the strip and in design docs.
- Bypass active state uses a reddish tint (`kBypassActive = 0xff6a3a3a`), Solo uses yellowish (`kSoloActive = 0xff7a7a4a`).
- Blend dropdown is populated from `Layer::MixMode` enum (30 entries including transitions) — only blend subset should be shown for the V dropdown.
- Transition dropdown populated separately for the F dropdown.
- v10 design studies (design/mockups/html/v10/) explore dropping K from the layer strip and creative rearrangements of S/V/F with integrated signal columns — these are design-only, no code changes.

### 26e. ClipCell

**What it does:** A single cell in the deck grid at the intersection of a layer and column. Two interaction zones: thumbnail area (click to trigger/retrigger clip) and name bar (click to select for inspection without triggering). Supports drag-and-drop for images, videos, effects, sources, and MilkDrop presets from browser panels and Finder.

**Key source files:**
- `ClipCell` class (src/ui/ClipCell.h:11, src/ui/ClipCell.cpp)

**Parent component:** DeckView (grid, one per layer x column position)

**Controls & interactions:**
- Thumbnail area: click = trigger clip, displays clip thumbnail or media type icon
- Name bar (20px bottom strip): click = select for inspection, right-click = context menu
- Active state: muted teal border highlight (`kActiveBorder = 0xff4a9a8a`, clip is playing)
- Selected state: white border highlight (user selected for inspection)
- Drag-and-drop receive: files from Finder, effects from FXBrowser, sources from SourcesBrowser, MilkDrop presets from MilkDropBrowser
- Clip-to-clip drag: drag from one cell to another to move/swap clips
- Multi-file drop: image sequences or multiple videos
- Hover states: `dragHover_`, `fxDragHover_`, `sourceDragHover_` for visual feedback

**Callbacks:** `onTrigger`, `onSelect`, `onFileDrop`, `onMultiFileDrop`, `onMultiVideoDrop`, `onEffectDrop`, `onSourceDrop`, `onClipMove`, `onMilkDropDrop`, `onMilkDropPlaylistDrop`

**Gotchas:**
- `kNameBarHeight = 20` — the bottom 20px is the select zone, everything above is the trigger zone.
- Implements both `juce::FileDragAndDropTarget` (Finder files) and `juce::DragAndDropTarget` (internal browser drags) — two separate interfaces for the same visual component.

### 26f. InspectorPanel

**What it does:** 4-tab container panel for inspecting Clip, Layer, Composition, and Signal properties. Auto-switches tabs based on user selection: clicking a clip opens the Clip tab, clicking a layer strip opens the Layer tab, clicking a signal strip opens the Signal tab. Supports "pin" mode to prevent auto-switching during live performance. v10 design direction (MOCKUP_BRIEF.md) specifies foldable tab groups (CLIP/LAYER/COMP/SIGNALS) with section-header grammar from AUDIO_DNA_DIRECTIVE_FULL.md.

**Key source files:**
- `InspectorPanel` class (src/ui/InspectorPanel.h:23, src/ui/InspectorPanel.cpp)

**Parent component:** MainComponent (third panel in the 4-panel bottom row)

**Controls & interactions:**
- Tab buttons: Clip, Layer, Composition, Signal (26px tab bar)
- Pin button — prevents auto-tab-switching during performance
- Each tab contains a scrollable Viewport wrapping its inspector component:
  - Clip tab → ClipInspector (see 26g)
  - Layer tab → LayerInspector (see 26h)
  - Composition tab → CompositionInspector (see 26i)
  - Signal tab → SignalInspector (see 7a)
- `inspectClip(Clip*)` / `inspectLayer(Layer*)` / `inspectSignal(Signal*)` — auto-switch + set data
- `refresh()` refreshes the currently visible tab

**Gotchas:**
- When pinned, `inspectClip/Layer/Signal` calls still update the data but do not switch tabs.
- Each inspector is wrapped in a `juce::Viewport` for scrolling — preferred height is calculated dynamically.

### 26g. ClipInspector

**What it does:** Resolume-style clip properties panel displayed in the InspectorPanel's Clip tab. Sections: Name + Thumbnail, Dashboard (8 macro knobs via MacroPanel), Transport (mode, playhead, loop/trigger, speed, duration), Cuepoints (8 cue buttons with set/jump), Autopilot, Source Parameters (for procedural source clips), Video (opacity, blend mode, RGBA toggles), Transform (position, scale, rotation, anchor), and Effects (EffectStackView).

**Key source files:**
- `ClipInspector` class (src/ui/ClipInspector.h:25, src/ui/ClipInspector.cpp)

**Parent component:** InspectorPanel (Clip tab, inside Viewport)

**Controls & interactions:**
- Transport section: mode selector (Timeline/BPMSync), speed slider, reverse button, half/double speed buttons, loop mode selector, play/pause/back buttons, duration slider, beat division selector (BPMSync mode), content beats slider (BPMSync mode)
- Cuepoints: 8 trigger buttons (jump to cuepoint) + 8 set buttons (set cuepoint at playhead)
- Autopilot: action and duration selectors
- Beat Snap: selector for quantize mode
- Image Sequence FPS: slider (visible for image sequence clips)
- Source Parameters: dynamically built `UniversalParamControl` list for procedural source parameters
- Video: opacity control (UniversalParamControl), width/height sliders, blend mode selector, alpha type selector, RGBA channel toggles
- Transform: position X/Y, scale, rotation, anchor (all UniversalParamControl)
- Effects: full EffectStackView for per-clip effects
- Timeline bar: draggable in/out points and playhead scrubber, beat markers
- Accepts FX drops from browser (DragAndDropTarget)

**Callbacks:** `onSourceParamsChanged`, `onCuepointJump`, `onCuepointSet`

**Gotchas:**
- Section layout uses `kRowHeight=24`, `kSectionGap=8` — compact but scrollable via parent Viewport.
- Timeline in/out point dragging uses hit-test zones (`DragTarget::InPoint/OutPoint/Playhead`).
- Source parameter controls are rebuilt via `buildSourceParamControls()` when clip changes.

### 26h. LayerInspector

**What it does:** Resolume-style layer properties panel displayed in the InspectorPanel's Layer tab. Sections: Name (editable), Dashboard (8 macro knobs), Autopilot (direction, duration, loops), Layer Master (master level, persistent toggle, ignore column trigger), Video (blend mode, opacity, width, height, auto size), Transition (blend mode, duration), Keying (mode, threshold, softness — visible for Transparent layers), DryWet (FX Only layers), 3D Controls (rotation, speed, scale — for ThreeD layers), Transform, Feedback (enable, preset, amount, scale, rotation, offset, luma key), and Layer Effects (EffectStackView).

**Key source files:**
- `LayerInspector` class (src/ui/LayerInspector.h:28, src/ui/LayerInspector.cpp)

**Parent component:** InspectorPanel (Layer tab, inside Viewport)

**Controls & interactions:**
- Editable name label — rename layers inline
- Autopilot: Rewind/Off/Forward/Random direction buttons, trigger mode selector (End of Video / On Beat), beat count selector, loops slider
- Layer Master: UniversalParamControl + persistent toggle + ignore column trigger toggle
- Video: blend mode dropdown, opacity control, width/height sliders, auto-size selector
- Transition: blend mode dropdown, duration slider
- Keying (Transparent type only): mode dropdown (13 keying modes), threshold slider, softness slider
- FX Only: dry/wet mix slider
- 3D Controls: rotation X/Y/Z sliders, speed slider, scale slider
- Transform: position X/Y, scale, rotation, anchor (all UniversalParamControl)
- Feedback: enable toggle, preset dropdown (6 presets), amount/scale X/Y/rotation/offset X/Y/luma key sliders
- Layer Effects: EffectStackView for per-layer effects
- Accepts FX drops from browser

**Callbacks:** `onLayerNameChanged`

**Gotchas:**
- Section visibility is conditional on layer type — keying controls only shown for Transparent, dry/wet for FX Only, 3D controls for ThreeD layers.
- Feedback controls reference `FeedbackProcessor` presets (6 Larsen loop configurations).
- Uses `kRowHeight=22`, `kSectionGap=4` — slightly more compact than ClipInspector.

### 26i. CompositionInspector

**What it does:** Resolume-style composition properties panel displayed in the InspectorPanel's Composition tab. Sections: Name + Resolution, Dashboard (8 macro knobs), Autopilot (direction, duration, loops, master layer, per-type autopilot settings), Composition (master slider, speed slider), Video (opacity), Transform (position, scale, rotation, anchor), Global Effects (EffectStackView), and Output Settings (resolution dropdown).

**Key source files:**
- `CompositionInspector` class (src/ui/CompositionInspector.h:23, src/ui/CompositionInspector.cpp)

**Parent component:** InspectorPanel (Composition tab, inside Viewport)

**Controls & interactions:**
- Autopilot: Rewind/Off/Forward/Random buttons, duration selector, clip loops slider, loop toggle, master layer selector
- Per-Type Autopilot (P20): enabled toggle, cycle sliders for Opaque/Transparent/Effect layers, randomize toggles
- Composition: master slider (UniversalParamControl with signal triangle), speed slider
- Video: opacity slider (UniversalParamControl)
- Transform: position X/Y, scale, rotation, anchor (all UniversalParamControl)
- Global Effects: EffectStackView for composition-level effects
- Output Settings: resolution dropdown

**Gotchas:**
- Per-Type Autopilot section is a P20 addition — only visible when `perTypeEnabledToggle_` is checked.
- Master control has a signal triangle for audio-driven master level.

### 26j. BrowserPanel

**What it does:** 6-tab container for the right-bottom browser area. Houses FilesBrowser, FXBrowser, SourcesBrowser, CompDecksBrowser, RecordPanel, and MilkDropBrowser. Tab switching shows/hides the appropriate child component.

**Key source files:**
- `BrowserPanel` class (src/ui/BrowserPanel.h:16, src/ui/BrowserPanel.cpp)

**Parent component:** MainComponent (fourth/rightmost panel in the 4-panel bottom row)

**Controls & interactions:**
- 6 tab buttons: Files, FX, Sources, Comp/Decks, Record, MilkDrop (26px tab bar)
- Tab switching via `setActiveTab(Tab)` — hides all children, shows selected
- `refresh()` refreshes all child browsers
- `setEffectLibrary()` / `setComposition()` — forwards dependencies to children
- Direct access to each browser: `getFilesBrowser()`, `getFXBrowser()`, `getSourcesBrowser()`, `getCompDecksBrowser()`, `getRecordPanel()`, `getMilkDropBrowser()`

**Gotchas:**
- Tab content components are all stack members (not heap) — always exist even when hidden.
- Dependencies must be set before `refresh()` for the FX and CompDecks tabs to populate.

### 26k. SignalBar & SignalStrip

**What it does:** Horizontal strip of signal meters running full window width, positioned below the TopBar. Displays all visible signals (8 audio + 2 modulation by default) as individual `SignalStrip` meters. Three display size modes: Minimized (~20px tall), Normal (~80px tall), Expanded (fills most of window — "programming mode"). Updates at ~30fps from the FeatureBus.

**Key source files:**
- `SignalBar` class (src/ui/SignalBar.h:15, src/ui/SignalBar.cpp)
- `SignalStrip` class (src/ui/SignalStrip.h:11, src/ui/SignalStrip.cpp)

**Parent component:** MainComponent (below TopBar, full width)

**Controls & interactions:**
- SignalBar: [+] button to add signals via popup menu, shrink/grow buttons for size cycling
- SignalStrip: single vertical meter per signal — click to select signal for SignalInspector
- Three paint modes per strip: `paintMinimized`, `paintNormal`, `paintExpanded` — adaptive detail level
- Each strip shows: signal name, formatted value, colored meter bar, peak hold indicator (30 frames), flash on transients
- `rebuildStrips()` recreates strips from SignalRegistry — call after adding/removing signals
- `onSignalSelected` callback fires when user clicks a strip
- `onSizeChanged` callback triggers parent re-layout

**Gotchas:**
- Expanded SignalBar mode hides other panels — reached via the SignalBar grow button (the View → "Programming Mode" menu item that toggled it was removed Wave 0; see F24).
- `SignalStrip::getSignalColour()` returns per-signal color — used for meter bar and accent.
- Peak hold uses `kPeakHoldFrames=30` (~1s) with `kPeakDecay=0.97f` exponential decay.

### 26l. EffectStackView

**What it does:** Vertical list of effects for the v2 inspector, replacing the v1 EffectsRackPanel. Each effect row is collapsible: collapsed shows bypass button, icon, effect name, and primary param value on a single line; expanded shows all parameters as UniversalParamControl widgets with source triangles, plus a dry/wet control.

**Key source files:**
- `EffectStackView` class (src/ui/EffectStackView.h:25, src/ui/EffectStackView.cpp)

**Parent component:** ClipInspector, LayerInspector, or CompositionInspector (embedded in the Effects section)

**Controls & interactions:**
- Per-effect header row (26px): Bypass button (B), effect name, Delete button (X), click to expand/collapse
- Expanded: dry/wet control + per-parameter UniversalParamControl widgets (with signal triangles, sliders, +/- buttons)
- Accepts FX drops from browser (DragAndDropTarget) — `onEffectAdded` callback
- Effect reordering not yet implemented (rows are static)

**Callbacks:** `onParamChanged`, `onBypassChanged`, `onDryWetChanged`, `onEffectAdded`, `onEffectRemoved`

**Gotchas:**
- `setEffects()` takes a pointer to `vector<Clip::EffectSlot>*` — the same vector is shared between the EffectStackView and the model. Direct mutation.
- `rebuildRows()` recreates all UI controls — expensive, called on every `setEffects()`.
- `kParamIndent = 12` — parameters are indented under the effect header.

### 26m. UniversalParamControl

**What it does:** The standard parameter widget used throughout all inspectors. Collapsed view shows a signal-connect triangle (grey=manual, cyan=connected), parameter label, value display, +/- buttons, and a slider. Expanded view adds a source picker dropdown, invert checkbox, and range min/max sliders. Supports 8 source modes: Manual, Signal, BPMSync, Oscillator, Envelope, ClipPosition, Timeline, and Macro.

**Key source files:**
- `UniversalParamControl` class (src/ui/UniversalParamControl.h:44, src/ui/UniversalParamControl.cpp)
- `ResettableSlider` class (src/ui/UniversalParamControl.h:21) — slider with right-click reset to default

**Parent component:** Used everywhere in ClipInspector, LayerInspector, CompositionInspector, EffectStackView

**Controls & interactions:**
- Signal connect triangle (14px clickable area) — grey when manual, cyan when connected to a source; click opens source picker popup
- Slider with value display — drag to adjust, right-click to reset to default
- +/- buttons for fine adjustment
- Expanded view: source picker button showing current mode, invert toggle, range min/max sliders
- Source picker popup menu: Manual, then grouped signal names from SignalRegistry, BPMSync, Oscillator, Envelope, ClipPosition, Timeline, Macro options
- Mini meter visualization when source drives value

**Callbacks:** `onValueChanged`, `onExpandToggled`, `onSourceChanged`, `onRangeChanged`, `onInvertChanged`

**Gotchas:**
- `kCollapsedHeight=24`, `kExpandedHeight=100` — parent must recalculate layout when expand state changes.
- `ResettableSlider` is a separate class (not a style) — subclasses `juce::Slider` to add right-click reset behavior.
- Source picker requires `setSignalRegistry()` to be called — otherwise the dropdown only shows Manual.

### 26n. MacroPanel

**What it does:** Dashboard section showing 8 macro link knobs in a single row, displayed at the top of Clip, Layer, and Composition inspectors. Each knob is renameable and can be manual or signal-driven. Shows linked parameters beneath each knob.

**Key source files:**
- `MacroPanel` class (src/ui/MacroPanel.h:15, src/ui/MacroPanel.cpp)

**Parent component:** ClipInspector, LayerInspector, CompositionInspector (Dashboard section)

**Controls & interactions:**
- 8 Knob widgets in a single row, each with a source picker button
- Click source button → popup menu to select signal source or manual
- Knob rotation → `onMacroValueChanged` callback
- Source change → `onMacroSourceChanged` callback
- `setMacroBank()` connects to the MacroBank data model
- `kPreferredHeight = 110` — includes header label + 8 knobs

**Gotchas:**
- Each `MacroSlot` owns a `unique_ptr<Knob>` and `unique_ptr<TextButton>` — created at construction, not dynamically.
- Only the Global MacroBank is currently instantiated (see F22a ghost feature).

### 26o. LookAndFeel

**What it does:** Global visual theme for the entire application. Subclasses `juce::LookAndFeel_V4` to provide a dark, professional VJ aesthetic with cyan and magenta accent colors. Overrides rendering for buttons, rotary sliders (knobs), linear sliders, toggle buttons, combo boxes, popup menus, labels, and scrollbars.

**Key source files:**
- `AudioDNALookAndFeel` class (src/ui/LookAndFeel.h:4, src/ui/LookAndFeel.cpp)

**Color palette (static constexpr):**

| Constant | Hex | Usage |
|----------|-----|-------|
| `kBackground` | `0xff1a1a2e` | Window/panel backgrounds |
| `kSurface` | `0xff252540` | Raised surface elements |
| `kSurfaceLight` | `0xff30305a` | Lighter surface variant |
| `kAccentCyan` | `0xff00e5ff` | Primary accent (active states, beat wheel, signal triangles) |
| `kAccentMagenta` | `0xffff00e5` | Secondary accent (mapping indicators) |
| `kTextPrimary` | `0xffe0e0e0` | Primary text |
| `kTextSecondary` | `0xff808090` | Dimmed/secondary text |
| `kMeterGreen` | `0xff00e676` | Meter positive range |
| `kMeterYellow` | `0xffffea00` | Meter warning range |
| `kMeterRed` | `0xffff1744` | Meter peak/danger range |
| `kPanelBorder` | `0xff3a3a5c` | Panel border lines |

**Gotchas:**
- Applied globally via `LookAndFeel::setDefaultLookAndFeel()` — all JUCE widgets inherit these overrides.
- Some components (LayerStrip, TimingWindow) use additional local color constants that should coordinate with these global values.
- v10 design system (MOCKUP_BRIEF.md §2.1-§2.3) defines 31 canonical CSS design tokens for mockups. Orange #ff4500 is reserved exclusively for OVERRIDE state — not counted as an accent color.

### 26p. MenuBarModel

**What it does:** Implements the 9-menu application menu bar: Audio-DNA, Composition, Deck, Layer, Column, Clip, Output, Shortcuts, View. Defines ~60 command IDs and delegates all menu item selection to MainComponent via `onMenuCommand` callback.

**Key source files:**
- `AudioDNAMenuBar` class (src/ui/MenuBarModel.h:8, src/ui/MenuBarModel.cpp)

**Parent component:** MainComponent (JUCE menu bar at window top)

**Menu structure:**
- Audio-DNA: Preferences, Import ISF Shader, About, Quit
- Composition: Undo/Redo, New/Open/Save/SaveAs, Copy/Paste Effects, Collect Media, Relocate Files
- Deck: New, Insert Before/After, Duplicate, Rename, Close, Clear Clips, Remove
- Layer: New, Insert Above/Below, Duplicate, Rename, Copy/Paste Effects, Clear/Remove, Fold, Move Up/Down
- Column: New, Insert Before/After, Duplicate, Clear Clips, Remove, Remove All Before/After
- Clip: Select All, Cut/Copy/Paste, Copy/Paste Effects, Rename, Clear, Show in Finder, New Source/Effect, Replace Content, Lock
- Output: Disabled/Fullscreen (per display)/Windowed, Identify Displays, Test Card, Snapshot, Start/Stop Recording
- Shortcuts: Edit Keyboard, Edit MIDI, Stop, Export/Import Bindings
- View: Signal Bar, Deck, Preview, Inspector, Browser, Timing Window, FPS Stats, Programming Mode, Save/Load/Reset Layout

**Gotchas:**
- Command IDs use ranges: 1000-1099 (Audio-DNA), 1100-1199 (Composition), 1200-1299 (Deck), etc.
- Output fullscreen uses `kOutputFullscreenBase + displayIndex` — dynamic per detected display.

### 26q. PreferencesDialog

**What it does:** 3-tab settings dialog accessed via Audio-DNA > Preferences (collapsed 8→3 in Wave 0; Audio/MIDI/Recording/Defaults/Feedback removed as empty/inert). Tabs: General, Video, About. Toggles apply in-session (no settings store is wired).

**Key source files:**
- `PreferencesDialog` class (src/ui/PreferencesDialog.h:8, src/ui/PreferencesDialog.cpp)

**Parent component:** Shown modally via `PreferencesDialog::show(parent)`, centered on the main window.

**Controls & interactions:**
- General tab: Show Tooltips toggle (genuinely wired Wave 1-D — creates/destroys the shared TooltipWindow, in-session only; Confirm-on-quit toggle removed Wave 0)
- (Audio tab removed Wave 0 — Sample Rate / Buffer Size / BPM-Detection-Range selectors were inert)
- Video tab: MilkDrop Presets directory browser (FPS Target + Render Resolution inert combos removed Wave 0)
- (MIDI/Recording/Defaults/Feedback tabs removed Wave 0 — were placeholder layouts)
- About tab: version label, credits label
- Scrollable content viewport for tabs that need it

**Gotchas:**
- Uses `juce::DialogWindow` for modal presentation — blocks interaction with main window while open.
- `Content` is an inner class — the dialog owns a single Content component that handles all tab switching.
- Most tabs beyond General/Audio/Video/About have placeholder implementations.

### 26r. PresetManager (UI)

**What it does:** Handles save/load of effect chain state + mappings as JSON preset files. Provides static methods for serialization (`savePreset`, `loadPreset`) and directory management for presets, compositions, and recordings.

**Key source files:**
- `PresetManager` class (src/ui/PresetManager.h:22, src/ui/PresetManager.cpp)

**Usage:** Called by MainComponent and CompDecksBrowser for save/load operations. Not a visible UI component itself — provides the serialization layer used by UI panels.

**Serialization format:**
```json
{"name": "preset name", "effects": [...], "mappings": [...]}
```

**Gotchas:**
- `kSourceNames[]` array is MISSING P25 advanced sources (SidechainPump, SwingRatio, etc.) — presets with P25 mappings fail to round-trip correctly.
- Effect names in presets use DISPLAY names — must be resolved via EffectLibrary on load.

**Storage:**
- Presets: `getPresetsDirectory()` (user documents)
- Compositions: `getCompositionsDirectory()` (user documents)
- Recordings: `getRecordingsDirectory()` (user documents)

---

## Data Model

N/A for traditional database — this is a C++ desktop app with in-memory data structures.

### Core Entities (in-memory)

- **Composition**: Top-level container. Owns decks[], global settings, per-type autopilot config, genre-deck assignments, composition transform (position/scale/rotation). Contains crossfader model fields (phase, blend mode, behaviour, curve) but crossfader is not wired.
- **Deck**: Grid container. Owns layers[]. One active deck at a time.
- **Layer**: Row in deck. Owns clips[] (columns), layer effects, opacity, blend mode, transition settings. Types: Opaque, Transparent, FXOnly, Mask. Has: persistent flag, autopilot settings.
- **Clip** (struct): Media content. Fields: mediaType (None/Image/Video/Camera/Source/ImageSequence), effects[] (EffectSlot), inPoint, outPoint, speed, transportMode (Timeline/BPMSync), loopMode (Loop/PingPong/OneShot), beatDivision, beatSnap, cuepoints[8], playheadPosition (mutable).
- **FeatureSnapshot** (POD, alignas(64)): 58 mapping source fields carrying all audio analysis results. Transferred via triple-buffer between threads. No pointers, no vtable.
- **Mapping**: source (Source enum, 58 entries) → targetEffectId → targetParamIndex → curve (24 types) → input/output range → smoothing → enabled.
- **Effect**: name, category, shaderProgram (GLuint), params (vector<EffectParam>), enabled, order. 135 effects, 333 parameters.
- **Binding**: keyCode or MIDI note/CC → action → targetMode (ByPosition/ThisItem/Selected) → triggerMode (Toggle/Momentary).
- **Signal**: name → value (float) → type. 32 registered in SignalRegistry (8 visible + 21 hidden audio + 2 modulation + 1 clip position).
- **ProceduralSource**: 108 sources, 759 parameters at runtime across 18 categories.

### Ownership Hierarchy
```
Composition → Deck[] → Layer[] → Clip[] → EffectSlot[]
                                        → ProceduralSource (if source type)
Composition → globalEffectChain → Effect[]
Each Layer → layerEffects → Effect[]
```

### Thread Safety
- Composition/Deck/Layer/Clip: owned by message thread, read by render thread via atomic config vars
- FeatureSnapshot: written by analysis thread, read by render thread via triple-buffer (lock-free)
- `Clip::playing` and `Clip::playheadPosition` are `mutable` — render thread writes them

### Persistence
- `PresetManager` serializes Composition + effects + mappings + bindings to JSON
- Session events serialized by `SessionRecorder` to JSON

---

## Shared Infrastructure

### Thread Model (4 threads, strict priority)

| Thread | Priority | Period | Budget | Communication |
|--------|----------|--------|--------|---------------|
| Audio Callback | RT (OS-managed) | 2.67ms (128 samples) | <100us | → SPSC ring buffer |
| Analysis Thread | Above-normal (app) | 10.7ms (512 hop) | <2ms | Ring buffer → Triple-buffer |
| Render Thread | Normal (VSync) | 16.67ms (60fps) | <8ms | Triple-buffer → atomics → UI |
| Message Thread | Normal (JUCE UI) | Event-driven | No budget | Atomics ← render/analysis |

### Lock-Free Patterns
1. **SPSC Ring Buffer**: audio → analysis. Power-of-two, cache-line padded, branchless modular.
2. **Triple-Buffer Atomic Swap**: analysis → render. 3x FeatureSnapshot, atomic index rotation.
3. **std::atomic<T> Config Vars**: UI → render/analysis. Slider changes, effect enable/disable.
4. **juce::MessageManager::callAsync()**: render → UI. Thread-safe callback posting.

### JSON Serialization
- `PresetManager` (src/ui/PresetManager.h) — save/load effects + mappings + composition to JSON
- JUCE `var`/`DynamicObject` for JSON serialization

### Build System
- CMake 3.24+, C++20, JUCE via FetchContent
- CI: `.github/workflows/build.yml`
- Test framework: Catch2 v3.7.1 via FetchContent
- Visual test harness: Eyes (pytest + HTTP API on port 8080)

### Environment Variables
| Var | Used By | Format | Validated at startup? |
|-----|---------|--------|-----------------------|
| `AUDIODNA_NO_SPAWN` | Visual tests | boolean flag | NO (test-only) |
| (none required) | -- | -- | Desktop app — no env vars needed for runtime |
