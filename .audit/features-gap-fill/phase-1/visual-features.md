# Visual/Render Features — Gap Fill

Generated: 2026-05-20 | Builder: visual-features (Builder 1 of 3)
Source: design/GAP_REPORT.md findings, verified against source code

---

## 17. MilkDrop/projectM Visualization System

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
5. `PresetSelector::processFrame()` detects structural transitions (DROP/BUILDUP/BREAKDOWN) and bar crossings, switches presets on next bar boundary with mood-matching (energyMatching maps structural state to mood category)

**Data flow:**
```
.milk/.prjm files → ProjectMPresetManager (scan + categorize)
PCM audio → ProjectMSource::feedAudio() → pcmBuffer_ (mutex-protected)
FeatureSnapshot → PresetSelector (structural state + bar phase → auto-switch decision)
ProjectMSource::render() → projectM internal renderer → glBlitFramebuffer → outputTex_ → clip texture
MilkDropBrowser → drag description "milkdrop:{path}" or "milkdrop_playlist:path1|path2|path3" → DeckView
```

**Dependencies & services:**
- libprojectM-4: preset rendering engine (optional, compile flag `AUDIODNA_HAS_PROJECTM`)
- JUCE: file scanning, JSON parsing, UI components
- OpenGL 4.1: FBO capture, blit

**Config:**
- projectM mesh size: 48x36 (hardcoded in initGL)
- projectM FPS: 60 (hardcoded)
- PCM buffer: 2048 floats (mono)
- Beat sensitivity: 0-2 range (param "u_src_beat_sens", slider 0-1 mapped to 0-2)
- Speed (preset duration): 5-60 seconds (param "u_src_speed", slider 1.0=5s, 0.0=60s)
- 5 exposed params: Beat Sensitivity, Speed, Warp Amount, Decay, Gamma
- Mood categories: 6 (Calm, Energetic, Psychedelic, Geometric, Dark, Minimal)
- Energy levels per mood: Calm=0.2, Minimal=0.3, Dark=0.4, Geometric=0.5, Psychedelic=0.7, Energetic=0.9
- Minimum bars between auto-switches: 4
- Auto-switch fallback: after `transitionBars * 4` bars without structural change
- Maximum recent presets tracked: 20

**Storage:**
- Preset files: user-configured directories, scanned for .milk/.prjm
- Manifest: presets.json (mood, energy, style metadata per preset)
- User data: favorites and user presets saved to JSON
- Supported formats: .milk (MilkDrop), .prjm (projectM native)

**Failure modes:**
- projectM not compiled (`AUDIODNA_HAS_PROJECTM` not defined) → renders dark purple placeholder (0.05, 0.0, 0.1) (handled)
- `projectm_create()` returns nullptr → `render()` returns empty outputTex_ (handled — null check)
- Preset load from non-GL thread → queued via mutex, applied next frame (handled)
- No presets found → empty browser, no auto-switch, no crash (handled)
- GL state corruption from projectM → mitigated by full save/restore of 16 GL state variables (handled)
- Mood manifest missing → falls back to heuristic keyword matching from preset name (handled)

**Test coverage:**
- Missing: no dedicated MilkDrop/projectM tests
- Missing: preset scanning, mood classification accuracy, auto-switch behavior, GL state save/restore

**Gotchas:**
- **projectM renders to default framebuffer (FBO 0)**, not to a custom FBO. The result must be blitted to the capture FBO. This is a projectM-4 API limitation.
- **GL state save/restore is mandatory** — projectM manages its own shaders, VAOs, blend modes, and depth test settings. Without restore, the rest of the render pipeline gets corrupted state.
- **`pcmMutex_` is a std::mutex on the render thread hot path** — blocks if `feedAudio()` is called simultaneously from the audio callback thread. Budget impact depends on contention frequency.
- **Gamma param is a no-op** — `applyParams()` has a comment "Gamma not directly settable in all projectM-4 builds; placeholder for future API extension."
- **Warp Amount and Decay params have no mapping** in `applyParams()` — only Beat Sensitivity and Speed are actually applied to the projectM instance. 3 of 5 params are decorative.
- **Mood heuristic fallback** for names that match no keyword: classifies by first letter range (a-f=Energetic, g-l=Geometric, m-r=Psychedelic, s-z=Calm). This is effectively random.
- **`static std::mt19937 rng`** in `randomPreset()` and `randomPresetInMood()` — static RNG shared across all calls, thread-safe on most platforms but not guaranteed by C++ standard.
- **Duplicate detection** during scan is O(n) linear search per new file — may be slow with ~9800 presets.
- **Preset navigation (next/prev/random) fires `onPresetChanged` callback** but the callback is not always wired — MilkDropBrowser sets it, but if ProjectMSource is used standalone, callback may be null (function object, null = no crash, just no notification).

**LOC:** ~2166 total (ProjectMSource: 419, ProjectMPresetManager: 478, PresetSelector: 133, MilkDropBrowser: 1136)

---

## 18. Render Pipeline Time Effects

**What it does:** Five temporal effects that operate on frame history rather than spatial pixel manipulation: Echo (ghost trails), Posterize Time (frame rate reduction), Freeze (frame hold), Screen Split (surveillance grid with per-cell delay), and Frame Stutter (temporal jumping). Screen Split and Frame Stutter use a per-layer ring buffer of 480 frames at 1/4 resolution. Echo, Posterize Time, and Freeze use the standard temporal buffer (`u_prev_frame`).

**Entry points:**
- `EffectLibrary::registerAll()` (src/effects/EffectLibrary.cpp:646-670) — registers all 5 time effects
- `CompositorEngine::applyClipEffects()` (src/render/CompositorEngine.cpp:240) — intercepts screen_split and frame_delay for special handling
- `CompositorEngine::applyScreenSplit()` (src/render/CompositorEngine.cpp:1308) — grid rendering with per-cell delay
- `CompositorEngine::FrameRingBuffer` (src/render/CompositorEngine.h:169) — 480-frame ring buffer struct
- `EmbeddedShaders::ghostTrails` (src/render/EmbeddedShaders.h:8409) — Echo GLSL shader
- `EmbeddedShaders::frameHold` (src/render/EmbeddedShaders.h:8463) — Posterize Time GLSL shader
- `EmbeddedShaders::timeFreeze` (src/render/EmbeddedShaders.h:8502) — Freeze GLSL shader
- `EmbeddedShaders::screenSplit` (src/render/EmbeddedShaders.h:8521) — Screen Split GLSL shader (fallback, not primary path)
- `EmbeddedShaders::frameDelay` (src/render/EmbeddedShaders.h:8591) — Frame Stutter passthrough (logic in compositor)

**Implementation chain:**

*Echo (ghost_trails):*
1. Temporal effect — uses `u_prev_frame` from per-layer temporal buffer
2. Each frame: blends current with decayed previous output
3. Decay range remapped: slider [0,1] maps to [0.82, 0.995] for perceptually linear control
4. 4 blend operators via `u_trail_fade`: Add (0), Screen (0.33), Maximum (0.66), Blend (1.0)
5. Output is clamped [0,1] to prevent blowout

*Posterize Time (frame_hold):*
1. Temporal effect — uses `u_prev_frame` from per-layer temporal buffer
2. Maps rate slider exponentially: slider 0=60fps (smooth), 0.5=~6fps (choppy), 1.0=1fps (extreme)
3. Quantizes time to target frame rate, shows held frame between updates
4. Amount param mixes between live and posterized output

*Freeze (time_freeze):*
1. Temporal effect — uses `u_prev_frame` from per-layer temporal buffer
2. Simple mix between current and frozen frame
3. `u_freeze_amount=1` = fully frozen, `u_freeze_amount=0` = fully live

*Screen Split (screen_split):*
1. Intercepted in `applyClipEffects()` before normal shader path (CompositorEngine.cpp:263)
2. Uses `FrameRingBuffer` — 480 frames at 1/4 resolution per layer
3. Parameters: columns (2-8), rows (2-8), frames-per-cell delay (0-60 frames), mode
4. 3 direction modes: Sequential (L-to-R), Reverse (BR-to-TL), Diagonal
5. Renders grid by setting per-cell GL viewport and drawing delayed textures from ring buffer
6. 2px border between cells, dark gray (0.05) background
7. Output written to effectFBO_A_

*Frame Stutter (frame_delay):*
1. Intercepted in `applyClipEffects()` (CompositorEngine.cpp:276)
2. Uses same `FrameRingBuffer` as Screen Split
3. Parameters: depth (1-30 frames back), stutter rate
4. stutter=0: pure constant delay — shows frame from N frames ago
5. stutter>0: discrete jumping between past frames at 1-16 Hz, 2-8 quantized steps
6. Returns delayed texture directly as currentInput — passthrough shader is never actually used

**Data flow:**
```
Echo/Posterize/Freeze: clip texture → shader (u_texture + u_prev_frame from temporal buffer) → output
Screen Split: clip texture → pushFrameToRing() → per-cell getFrameFromRing() → viewport grid → effectFBO_A_
Frame Stutter: clip texture → pushFrameToRing() → getFrameFromRing(framesAgo) → direct texture swap
```

**Dependencies & services:**
- OpenGL 4.1: FBOs, textures, viewports, blitting
- GLSL 410: all time effect shaders

**Config:**
- Ring buffer: 480 frames per layer, 1/4 resolution (~120MB VRAM at 1080p source = 480x270 per frame)
- Echo params: decay [0,1]→[0.82,0.995], operator [0,1] (Add/Screen/Max/Blend)
- Posterize Time params: frame rate [0,1]→[60,1] fps exponential, amount [0,1]
- Freeze params: amount [0,1]
- Screen Split params: columns [0,1]→[2,8], rows [0,1]→[2,8], frames-per-cell [0,1]→[0,60], mode [0,1] (Sequential/Reverse/Diagonal)
- Frame Stutter params: depth [0,1]→[1,30] frames, stutter [0,1]→[1-16 Hz, 2-8 steps]

**Failure modes:**
- Ring buffer VRAM exhaustion → mitigated by 1/4 downscale (handled by design)
- Missing passthrough shader → ring buffer push fails silently, returns clipTex (partially handled)
- Ring buffer not yet filled → getFrameFromRing clamps to available frames (handled)

**Test coverage:**
- Missing: no dedicated time effects tests
- Missing: ring buffer lifecycle, Screen Split grid accuracy, Frame Stutter timing, Echo blend mode verification

**Gotchas:**
- **Screen Split and Frame Stutter bypass the normal shader pipeline** — they are intercepted in `applyClipEffects()` with `continue` statements. The GLSL shaders for screen_split and frame_delay exist in EmbeddedShaders.h but the Screen Split shader is a simpler fallback (1-frame delay via u_prev_frame), and the Frame Stutter shader is a pure passthrough. The real logic is in C++ compositor code using the ring buffer.
- **Ring buffer is per-layer** (`layerRingBuffers_` keyed by layerId) — each layer with Screen Split or Frame Stutter gets its own 480-frame buffer.
- **Ring buffer initialization is lazy** — `getOrCreateRingBuffer()` allocates on first use. This means first frame with these effects may cause a VRAM allocation spike.
- **Screen Split writes to effectFBO_A_ directly** — after applyScreenSplit, the ping-pong state is forced to write to B next. Getting this wrong corrupts the effect chain.
- **Echo "Echo" name collision** — there is also an "Echo" feedback preset in FeedbackProcessor (src/render/FeedbackProcessor.cpp:138). The FeedbackProcessor Echo is a layer-level Larsen feedback loop (amount=0.60, no scale/rotation), while the EffectLibrary Echo is a clip-level temporal ghost trail effect. Different systems, same name.
- **Posterize Time uses `u_time` for quantization** — if u_time has jitter or resets, the frame hold timing will be inconsistent.

---

## 19. LUT Loader

**Status: Ghost — code exists, no UI path to load external .cube files**

**What it does:** Parses .cube LUT (Look-Up Table) files and uploads them as `GL_TEXTURE_3D` handles for color grading. Supports the standard .cube format: `LUT_3D_SIZE N` header followed by N^3 RGB float triples (B fastest, G middle, R slowest ordering).

**Entry points:**
- `LUTLoader` class (src/render/LUTLoader.h:13) — static methods for .cube file parsing and GL upload
- `TextureManager::loadLUT()` (src/render/TextureManager.h:36) — wrapper that delegates to LUTLoader
- `TextureManager::releaseLUT()` (src/render/TextureManager.cpp:155) — cleanup wrapper

**Implementation chain:**
1. `LUTLoader::loadCubeFile()` parses .cube file line-by-line:
   a. Skips comments (#), TITLE, DOMAIN_MIN/MAX lines
   b. Reads `LUT_3D_SIZE N` to determine dimensions
   c. Parses N^3 RGB float triples into `std::vector<float>`
   d. Validates: size > 0 and data.size() == N^3 * 3
   e. Creates GL_TEXTURE_3D with GL_RGB32F internal format
   f. Sets trilinear filtering (GL_LINEAR min/mag) and clamp-to-edge wrapping
   g. Uploads via `glTexImage3D()`
2. `LUTLoader::releaseLUT()` — calls `glDeleteTextures()`

**Data flow:**
```
.cube file → LUTLoader::loadCubeFile() → parse → GL_TEXTURE_3D → (NO CONSUMER — TextureManager::loadLUT() is never called)
```

**Why it is a ghost feature:**
- `TextureManager::loadLUT()` wraps `LUTLoader::loadCubeFile()` but is **never called** anywhere in the codebase
- The "Color Grade" effect (EffectLibrary.cpp:559) uses a built-in GLSL shader (`lutGrade` in EmbeddedShaders.h:4171) that does NOT sample a 3D LUT texture — it applies a hardcoded warm/cool cross-process grade
- No UI exists to browse/load .cube files
- No API endpoint exposes LUT loading
- The infrastructure is complete and functional but disconnected from any consumer

**Dependencies & services:**
- OpenGL 4.1: GL_TEXTURE_3D, glTexImage3D
- JUCE: file I/O, string parsing

**Config:**
- Supported format: .cube only
- Texture format: GL_RGB32F (high precision)
- Filtering: trilinear (GL_LINEAR)
- Wrapping: clamp-to-edge on all 3 axes

**Failure modes:**
- File not found → returns 0, logs error to stderr (handled)
- Empty file → returns 0 (handled)
- Invalid LUT_3D_SIZE or mismatched entry count → returns 0, logs error (handled)
- No LUT_3D_SIZE line → lutSize stays 0, returns 0 (handled)

**Test coverage:**
- Missing: no tests for LUT loading, parsing, or GL texture creation

**Gotchas:**
- **GL_RGB32F is high-bandwidth** — each texel is 12 bytes. A 64x64x64 LUT = 3.1MB, which is fine. A 256x256x256 LUT = 192MB, which is excessive. No size cap enforced.
- **DOMAIN_MIN/MAX lines are parsed but ignored** — non-[0,1] domains will produce incorrect color grades.
- **Entire file loaded into memory as string** before parsing — large LUT files could spike memory temporarily.

**LOC:** 111 (LUTLoader.h: 23, LUTLoader.cpp: 88)

---

## 20. Camera Input

**Status: Ghost — code exists, no UI in v2 layout**

**What it does:** Defines camera as a clip media type and provides v1-era infrastructure for live camera input (device enumeration, open/close, frame capture), but the camera path is not connected to the v2 deck/layer/clip compositor.

**Entry points:**
- `Clip::MediaType::Camera` (src/model/Clip.h:17) — enum value in media type
- `Clip::cameraDeviceIndex` (src/model/Clip.h:20) — field for camera device selection
- `MainComponent::openCamera()` (src/MainComponent.cpp:2244) — opens camera device
- `MainComponent::closeCamera()` (src/MainComponent.cpp:2273) — closes camera device
- `MainComponent::refreshCameraList()` (src/MainComponent.cpp:2232) — enumerates available devices
- `MainComponent` implements `juce::CameraDevice::Listener` (src/MainComponent.h:53)

**Implementation chain (v1, partially connected):**
1. `refreshCameraList()` calls `juce::CameraDevice::getAvailableDevices()` to populate selector
2. User selects camera from dropdown → `openCamera(deviceIndex)`
3. `juce::CameraDevice::openDevice()` opens the camera at 320x240 resolution
4. Camera frames delivered via `CameraDevice::Listener` callback
5. Frames queued to `previewPanel_` and `outputWindow_->getRenderer()` via `queueCameraFrame()`
6. `OutputWindow` has `queueCameraFrame()` and `pendingCameraFrame_` for frame delivery

**Why it is a ghost feature:**
- `CompositorEngine::getClipTexture()` (src/render/CompositorEngine.cpp:1045-1061) handles Image, Source, Video, and ImageSequence media types — but **Camera is not handled** (falls through to `return 0`)
- No v2 UI component references Camera: not in ClipInspector, not in DeckView, not in BrowserPanel
- Camera selector UI is part of the v1 controls that are `setVisible(false)` in MainComponent::resized() (line ~1331)
- `Clip::cameraDeviceIndex` field exists and is copied in `setContent()` (line 197) and cleared in `clearContent()` (line 248), but no code ever sets it
- Camera frames go to the legacy preview panel and output window, not through the v2 compositor pipeline

**Data flow (v1, disconnected):**
```
juce::CameraDevice → CameraDevice::Listener callback → MainComponent
  → previewPanel_.queueCameraFrame(image)
  → outputWindow_->getRenderer().queueCameraFrame(image)
(NOT connected to: CompositorEngine → deck/layer pipeline)
```

**Dependencies & services:**
- JUCE: `CameraDevice`, `CameraDevice::Listener`

**Config:**
- Camera resolution: 320x240 (hardcoded in openCamera)
- Default device index: -1 (none selected)

**Failure modes:**
- Camera fails to open → sets fileLabel_ to "Camera failed to open" (handled)
- No camera devices available → selector empty (handled)

**Test coverage:**
- Missing: no camera-related tests

**Gotchas:**
- **To make Camera work in v2**, `CompositorEngine::getClipTexture()` needs a Camera case that receives frames from the camera device and uploads them as GL textures. The current v1 path goes through JUCE Image, not GL textures.
- **Camera resolution 320x240 is very low** — likely a placeholder. Full HD camera input would need async frame upload to avoid stalling the render thread.
- **Camera selector is hidden** in v2 layout alongside other v1 controls — all set to `setVisible(false)` at MainComponent.cpp line ~1331.

**LOC:** Camera-specific code is spread across MainComponent (~50 lines for open/close/refresh/callback). Clip model adds 3 lines (enum value, field, copy/clear).

---

## 21. v1/v2 Layout Component Split

**What it does:** Three complete UI components from the v1 layout remain in the codebase but are hidden (`setVisible(false)`) in the v2 layout. They are fully functional code that could be surfaced again. Additionally, ~15 v1 controls in MainComponent are hidden.

**Hidden v1 components:**

### AudioReadoutPanel (src/ui/AudioReadoutPanel.h:9, src/ui/AudioReadoutPanel.cpp)
- **Purpose:** Left panel showing ALL audio feature values updating in real-time at 30fps via FeatureBus
- **Capabilities:** Section-based display with meters for RMS, peak, ZCR, spectral features, 7-band energy bars, beat phase visualization, bar indicator, onset flash animation, downbeat flash, structural state display (NORMAL/BUILDUP/DROP/BREAKDOWN), genre state display
- **Rendering:** Full custom paint with helper methods: drawSection, drawMeter, drawDbMeter, drawLabel, drawBandMeters, drawBeatPhase, drawBarIndicator, drawOnsetIndicator, drawStructuralState, drawGenreState
- **Dependencies:** AnalysisThread (constructor reference), FeatureBus (reads snapshots)
- **LOC:** 697 (AudioReadoutPanel.h: 70, AudioReadoutPanel.cpp: 627)
- **Hidden at:** MainComponent.cpp line 1320 — `audioReadoutPanel_.setVisible(false)`

### SpectrumDisplay (src/ui/SpectrumDisplay.h:8, src/ui/SpectrumDisplay.cpp)
- **Purpose:** 7-band energy visualization with smooth animation, peak hold markers, and gradient fills
- **Capabilities:** Per-band attack/release smoothing (fast rise 0.6, slow fall 0.08), peak hold for 20 frames (~0.67s) with 0.95 decay, color-coded gradient (Sub=red, Bass=orange, LMid=yellow, Mid=green, HMid=cyan, Pres=blue, Bril=purple)
- **Update rate:** 30fps via Timer
- **Dependencies:** FeatureBus
- **LOC:** 171 (SpectrumDisplay.h: 47, SpectrumDisplay.cpp: 124)
- **Hidden at:** MainComponent.cpp line 1321 — `spectrumDisplay_.setVisible(false)`

### EffectsRackPanel (src/ui/EffectsRackPanel.h:20, src/ui/EffectsRackPanel.cpp)
- **Purpose:** Right-side panel showing the effect chain with per-parameter rotary knobs and mapping controls
- **Capabilities:** Per-effect enable/disable toggle, rotary Knob components with mapping indicator ring, click-to-map opens MappingEditor popup, per-effect lock toggle (protects from randomization), rebuilds UI from effect chain state on preset load
- **Superseded by:** EffectStackView (v2 component)
- **Dependencies:** MappingEngine, EffectChain, EffectLibrary, MappingEditor, Knob
- **LOC:** 670 (EffectsRackPanel.h: 99, EffectsRackPanel.cpp: 571)
- **Hidden at:** MainComponent.cpp line 1322 — `if (effectsRackPanel_) effectsRackPanel_->setVisible(false)`

### Hidden v1 controls in MainComponent (all `setVisible(false)`)
- `audioSourceLabel_`, `audioSourceSelector_` — audio source selection
- `inputGainLabel_`, `inputGainSlider_` — input gain control
- `masterLevelSlider_`, `masterLevelLabel_` — master output level
- `displaySelector_` — display mode selection
- `resolutionSelector_` — resolution selection
- `randomLabel_`, `beatRandomToggle_`, `beatCountSelector_` — beat-synced randomization
- `syncButton_` — sync toggle
- `fpsLabel_`, `cpuLabel_` — performance readouts
- `openImageButton_`, `fileLabel_`, `imageSequenceButton_` — v1 file loading
- `cameraLabel_` — camera label (part of Camera ghost feature)
- v1 preset slots (10 buttons + dropdowns)

**Hidden at:** MainComponent.cpp lines 1318-1337 and the v2 `resized()` method which only positions v2 components

**Failure modes:**
- N/A — components are functional, just not visible. Making them visible would require layout changes in MainComponent::resized().

**Test coverage:**
- Missing: no tests for v1 components (hidden, so not exercised in visual tests)

**Gotchas:**
- **AudioReadoutPanel references AnalysisThread by const reference** in constructor — the AnalysisThread must outlive the panel. Since both are owned by MainComponent, this is safe.
- **EffectsRackPanel is heap-allocated** (`std::unique_ptr<EffectsRackPanel>`) while AudioReadoutPanel and SpectrumDisplay are stack members of MainComponent. This means EffectsRackPanel can be nullptr (checked with `if (effectsRackPanel_)` before setVisible).
- **EffectsRackPanel was superseded by EffectStackView** (the v2 inspector-based effect editor). Both systems work, but EffectsRackPanel's knob-based UI is the v1 approach while EffectStackView uses sliders in the inspector panel.
- **v1 controls add ~200 lines of member declarations and initialization** to MainComponent, contributing to its god-object status (24 edges in graph, 0.10 cohesion).
- **These components are candidates for removal** if v1 compatibility is no longer needed, or for **resurfacing** if the dual-mode design (GAP_REPORT Section 4) is implemented — AudioReadoutPanel and SpectrumDisplay could serve as programming mode diagnostic panels.
