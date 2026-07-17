# Audio-DNA Complete Feature Inventory

> **SUPERSEDED 2026-07-16** by `.harmony/APP-INVENTORY.md` + `.harmony/FEATURES.md`
> (both newer and reconciled against source truth). Archived for historical reference;
> counts here may be stale. Do not treat as current.

**Generated**: 2026-04-19
**Target**: Complete line-by-line inventory of every user-facing feature, control, signal, parameter, binding, endpoint, and capability — zero gaps for UI redesign.

**Sources audited (15 disjoint slices, fully parallel)**:

| # | Slice | Scope | Fragment |
|---|---|---|---|
| 01 | Audio + Analysis | `src/audio/`, `src/analysis/`, `src/features/` | [slice_01_audio_analysis.md](.feature_audit/slice_01_audio_analysis.md) |
| 02 | Signal + Mapping + Routing | `src/signal/`, `src/mapping/`, `src/routing/` | [slice_02_signal_mapping_routing.md](.feature_audit/slice_02_signal_mapping_routing.md) |
| 03 | Data Model | `src/model/`, `src/core/` | [slice_03_data_model.md](.feature_audit/slice_03_data_model.md) |
| 04 | Inspector UIs | `src/ui/*Inspector*`, `MappingEditor`, `MacroPanel`, `UniversalParamControl`, `PresetManager` | [slice_04_inspectors.md](.feature_audit/slice_04_inspectors.md) |
| 05 | Top Chrome | `MainComponent`, `TopBar`, `MenuBarModel`, `PreferencesDialog`, readout/waveform/spectrum/timing, overlays, windows, `Knob`, `LookAndFeel` | [slice_05_top_chrome.md](.feature_audit/slice_05_top_chrome.md) |
| 06 | Browsers + Deck UI | `BrowserPanel`, `FilesBrowser`, `FXBrowser`, `SourcesBrowser`, `CompDecksBrowser`, `MilkDropBrowser`, `RecordPanel`, `DeckView`, `ClipCell`, `LayerStrip`, `SignalBar`, `SignalStrip`, `EffectStackView`, `EffectsRackPanel` | [slice_06_browsers_deck_ui.md](.feature_audit/slice_06_browsers_deck_ui.md) |
| 07 | Effects + Shaders + Render | `src/effects/`, `src/render/` (incl. `EmbeddedShaders.h`, `CompositorEngine`, `FeedbackProcessor`) | [slice_07_effects_shaders.md](.feature_audit/slice_07_effects_shaders.md) |
| 08 | Sources | `src/sources/`, `src/media/` (video, image-seq) + disk `shaders/` | [slice_08_sources.md](.feature_audit/slice_08_sources.md) |
| 09 | Bindings + MIDI | `src/binding/`, `src/midi/` | [slice_09_bindings_midi.md](.feature_audit/slice_09_bindings_midi.md) |
| 10 | OSC + API + Recording + Output | `src/osc/`, `src/api/`, `src/recording/`, `src/output/`, `src/test/` | [slice_10_osc_api_recording_output.md](.feature_audit/slice_10_osc_api_recording_output.md) |
| 11 | BPM + Smart + Sync | `BPMTracker`, `GenreDetector`, `GenreSmoothing`, `AdvancedAudioAnalyzer`, `LinkSync` | [slice_11_tempo_smart.md](.feature_audit/slice_11_tempo_smart.md) |
| 12 | Top-level docs | `ARCHITECTURE*.md`, `TASKPLAN*.md`, `PHASE_GUIDE.md`, `BUILDLOG.md`, `VALIDATION_PROTOCOL.md`, `LESSONS_LEARNED.md`, `FX_SOURCE_AUDIT.md`, `README.md` | [slice_12_toplevel_docs.md](.feature_audit/slice_12_toplevel_docs.md) |
| 13 | Research part A | `ARCH_*`, `FEATURES_*`, `LIB_*`, `REF_*`, `BPM_STABILITY_RESEARCH`, `INDEX.md` | [slice_13_research_partA.md](.feature_audit/slice_13_research_partA.md) |
| 14 | Research part B | `UNIFIED_BUILD_PLAN`, `OurAppFeatures`, `SOURCES_procedural_generators`, Resolume/ArKaos integration, VIDEO_*, IMPL_*, UI/UX analyses | [slice_14_research_partB.md](.feature_audit/slice_14_research_partB.md) |
| 15 | Tests + CMake | `tests/visual/`, `CMakeLists.txt`, `cmake/`, Python `vj_controller` | [slice_15_cmake_tests.md](.feature_audit/slice_15_cmake_tests.md) |

**Files read by subagents**: ~200 source files + ~45 markdown documents + `CMakeLists.txt` and Python test harness.

---

## How to read this document

- Each section below has a **short digest** followed by a **pointer to the authoritative slice fragment** (in `.feature_audit/`). The slice fragments are the line-by-line source of truth (~7,700 lines of citations). Keeping them as separate files avoids one giant unreadable document while preserving traceability.
- Every fact in the digest and the fragment is cited `file:line`.
- Sections 1-17 are inventory. Sections 18 & 19 are reconciliation (contradictions found). Section 20 is cross-reference index.

---

# Section 1: Audio Analysis Features

**Summary**: The `FeatureSnapshot` struct (`src/analysis/FeatureSnapshot.h:5`, `alignas(64)` POD) holds **40 rows** (33 unique C++ member declarations; 7 band-energy sub-entries counted separately). Produced every 10.67 ms (512-sample hop @ 48 kHz) by `AnalysisThread::run()`, published through a lock-free triple-buffer `FeatureBus`.

**Field families (counts)**:
- Amplitude & dynamics: `rms`, `peak`, `rmsDB`, `lufs`, `dynamicRange`, `transientDensity` — 6
- Spectral: `spectralCentroid`, `spectralFlux`, `spectralFlatness`, `spectralRolloff` — 4
- 7-band energies: `bandEnergies[0..6]` (Sub / Bass / LowMid / Mid / HighMid / Presence / Brilliance) — 7
- Onset + rhythm: `onsetDetected`, `onsetStrength`, `bpm`, `beatPhase`, `trackerState`, `beatInBar`, `barPhase`, `downbeatDetected`, `barCount`, `phrasePhase` — 10
- Structural: `structuralState` (0/1/2/3 = normal/buildup/drop/breakdown) — 1
- Chroma + harmony: `chromagram[12]`, `harmonicChangeDetection`, `dominantPitch`, `pitchConfidence`, `detectedKey`, `keyIsMajor` — 17
- Timbre: `mfccs[13]` — 13
- Genre (P23): `detectedGenre`, `genreConfidence`, `energyState`, `genreScores[8]` — 11
- Advanced (P25): `sidechainPump`, `swingRatio`, `formantPresence`, `resonancePeak`, `reeseBass` — 5

**Pipeline**: 14 stages executed sequentially in `AnalysisThread::run` (`AnalysisThread.cpp:99-295`). Header docstring (`:28-42`) still says 13; profiler logs 13 labels but timing is 14.

**Audio input**: `AudioEngine::SourceMode` = `{File, MicInput}` (`AudioEngine.h:36`) — no explicit system-audio-loopback mode. File formats via `juce::AudioFormatManager::registerBasicFormats()`: WAV, AIFF, FLAC, Ogg Vorbis, MP3. Output always silenced (`AudioEngine.h:127`).

**Key constants**: Block=2048, Hop=512, Sample rate=48kHz, FFT 2048-pt, 40 mel bands → 13 MFCCs over 20-8000Hz, 7 band edges `{20,60,250,500,2000,4000,6000,20000}` Hz.

**Full details**: [.feature_audit/slice_01_audio_analysis.md](.feature_audit/slice_01_audio_analysis.md) — 413 lines.

**Notable surprises**:
1. `kNumStages = 14` but header docstring and profiler labels say 13 (`AnalysisThread.h:28-42`, `AnalysisThread.cpp:317-321`).
2. `FeatureSnapshot::clear()` doesn't reset `detectedGenre` back to its default `6` — after clear it becomes `0` (House) — possible latent bug (`FeatureSnapshot.h:81-89`).
3. `AnalysisThread` owns a secondary PCM snapshot path (`kPCMSnapshotSize=512`, double-buffer) for projectM — undocumented in CLAUDE.md (`AnalysisThread.h:70-73`).
4. Default `inputGain = 1.0f`, input level release coefficient `0.92f` per block.

---

# Section 2: Signal System

**Summary**: 58 audio feature sources + 5 signal types + 24 macros + 23 curve types + 17-field routes.

- **MappingSource enum**: **58 real values** + 1 `Count` sentinel (`MappingTypes.h:6-87`). Groups: Amplitude 3, Loudness 3, Spectral 4, 7-Band 7, Rhythm 6, Structural 1, Pitch 4, Timbre 13 (MFCC0..12), Chroma 12, Advanced P25 5.
- **Signal base class** (`Signal.h:9`) with `Type = {Audio, Oscillator, Envelope}` and `Category = {Amplitude, Bands, Rhythm, Pitch, Chroma, Timbre, Structure, Modulation}` (8).
- **Default signal registration** (`SignalRegistry::initDefaults`, `SignalRegistry.cpp:4-84`): 32 signals — 8 visible audio (Volume, Sub Bass, Bass, Mid, Air, Tempo, Beat Position, Hit), 21 hidden audio, 1 Oscillator ("Mod 1"), 1 Envelope ("Mod 2"), 1 hidden ClipPosition. 27 MappingSources (LUFS, RmsDB, SpectralRolloff, StructuralState, DetectedKey, all 13 MFCCs, all 12 Chroma) are **not** exposed as signals by default — routable only via raw Mapping.
- **Oscillators** (`OscillatorSignal.h`): 5 shapes (Sine, SawUp, SawDown, Triangle, Square). Params: shape, beatDuration (1.0 default), amplitude (1.0), phaseOffset (0.0).
- **Envelopes** (`EnvelopeSignal.h`): Free-form control points (default 3-point ramp 0→1→0). 3 curve types per segment: Linear, Exponential, SCurve. Fields: points, beatDuration (4.0), amplitude (1.0), phaseOffset (0.0), curveType, oneShot (false), looping (true). **No ADSR, no external trigger** — always BPM-locked.
- **Macros** (`MacroBank.h`): `Scope = {Clip, Layer, Global}` × `kNumMacros = 8` = 24 macros total. `Macro` fields: name, manualValue (0.5), currentValue (0.5), sourceSignalId (0 = Manual), links vector, id. `MacroLink` fields: target RouteTarget, outputMin (0.0), outputMax (1.0), inverted (false).
- **Curve types**: **23 usable + 1 Count sentinel** = 24 enum (`MappingTypes.h:90-120`, impl `CurveTransforms.h:222-252`). 5 classic (Linear, Exponential, Logarithmic, SCurve, Stepped) + 18 P24 easings (Circular/Back/Elastic/Bounce/Cubic/Sine × In/Out/InOut) + Hold step.
- **Mapping fields** (10): source, targetEffectId, targetParamIndex, curve (Linear), inputMin (0.0), inputMax (1.0), outputMin (0.0), outputMax (1.0), smoothing (0.15 EMA), enabled (true). Pipeline: Extract → Normalize → Curve → Scale → Smooth → **Accumulate** (not assign) → Clamp.
- **Route fields** (17): id, sourceType (Signal/Macro), sourceId, targetScope (Clip/Layer/Global), targetLayerId, targetClipId, targetEffectIndex, targetParamIndex, outputMin (0.0), outputMax (1.0), inverted (false), dialRangeMin (0.0), dialRangeMax (1.0), threshold (0.0), gain (1.0), falloff (0.1), enabled (true).
- **ChainedSignal**: 4 chain modes — `Multiply, Add, Gate, ScaleRange`. Params: carrierSignalId, modulatorSignalId, chainMode (Multiply default), gain (1.0), modulationDepth (1.0), gateThreshold (0.1).
- **Signal connect popup sources** (7 conceptual): Manual (sourceSignalId=0), Audio Signal, Oscillator Signal, Envelope Signal, Clip Position Signal, Macro (via Route::SourceType), Chained Signal.

**Full details**: [.feature_audit/slice_02_signal_mapping_routing.md](.feature_audit/slice_02_signal_mapping_routing.md) — 527 lines.

**Notable surprises**:
1. "Hit" visible signal and "Hit Strength" hidden signal both wrap `OnsetStrength` — duplicate source.
2. Envelope has no external trigger — always BPM-locked off `beatPhase + beatInBar`, no ADSR, no onset-triggered mode.
3. `MappingSuggester` uses `snapshot.trackerState == 2` as beat relevance gate — not the `confirmedGenre` directly (see Section 11).
4. `MappingEngine::processFrame` accumulates multiple mappings to the same param (sums then clamps); `RoutingEngine::processFrame` uses a caller-provided `ParamWriter` callback and applies per-route falloff state.
5. `SignalRegistry::getCachedValue` is O(N) linear scan per lookup.

---

# Section 3: Deck / Layer / Clip Model

**Summary**: `Layer::MixMode` is a single unified **55-value enum** covering both blend modes (25) and transitions (30) — `Dissolve` bridges both. Total ~143 enum values across 20 enums.

**Deck** (`src/model/Deck.h`): 4 public fields + 12 methods.

**Layer** (`src/model/Layer.h/cpp`): ~40 fields + 8 feedback sub-fields. Layer types: **5** (`Layer::Type`). Per-layer config: opacity, blend mode (25 values), transition mode (30 values), keying mode (13 options), feedback preset (6 presets), 3D rotation, autopilot settings, persistent, ignoreColumnTrigger, lockContent, fold flags.

**Clip** (`src/model/Clip.h/cpp`): ~45 fields with 8-slot cuepoint array. `Clip::MediaType` = 6 (None, Image, Video, Camera, Source, ImageSequence). `TransportMode` = 2 (Timeline, BPMSync). `LoopMode` = 3 (Loop, PingPong, OneShot). `BeatSnapMode` = 5 (Off, Beat, Bar, TwoBar, FourBar).

**Composition** (`src/model/Composition.h`): ~30 fields + 11-field `PerTypeAutopilotConfig`. Cross-deck `CrossfaderBlendMode` = 3 (Alpha, Add, Multiply). Composition-level transform (compPositionX/Y, compScale, compRotation, compAnchorX/Y), globalTransitionSpeed, autoPresetOnGenre, genreDeckAssignment[8], structuralSceneEnabled.

**Autopilot** (`src/model/Autopilot.h/cpp`): `AutopilotAction` = 8, `AutopilotDuration` = 8, `AutopilotDirection`, etc. On-Beat vs End-of-Video trigger modes.

**Undo/Redo**: `src/core/Command.h` + `UndoManager.h/cpp` — infrastructure exists but **zero concrete Command subclasses exist**. Undo/Redo is live but unused.

**Keying modes (13)**: Full enum listing in slice; includes Chroma, Luma, Alpha, Inverse variants, ChannelR/G/B, etc.

**Feedback presets (6)**: Zoom In, Spiral, Drift, Kaleidoscope, Echo, Stretch (exact config values at `FeedbackProcessor.cpp:131-142`).

**Full details**: [.feature_audit/slice_03_data_model.md](.feature_audit/slice_03_data_model.md) — 781 lines.

**Notable surprises**:
1. **`Layer::MixMode` is one 55-value enum**, not separate blend+transition enums. 25 blend + 30 transition entries; `Dissolve` appears in both ranges.
2. **Undo infrastructure has zero Command subclasses** — non-functional despite being wired.
3. **`FeedbackConfig` is NOT serialized** to presets.
4. Composition serialization silently drops `compOpacity`, crossfader state, comp transform, per-type autopilot, and all genre-aware fields.

---

# Section 4: Effects Library

**Summary**: **135 effects** registered in `EffectLibrary.cpp` (grep-confirmed `registerEffect({` count), shaders embedded in `src/render/EmbeddedShaders.h`.

**Category breakdown**:

| Category | Count |
|----------|-------|
| 3D / Depth | 9 |
| Warp | 27 |
| Color | 32 |
| Glitch | 15 |
| Pattern | 19 |
| Animation | 6 |
| Audio | 4 |
| Time | 6 |
| Blend | 5 |
| Composite | 3 |
| Blur / Post | 10 |
| **Total** | **135** |

**Each effect** has: display name, category string, shader key (snake_case), parameter list with defaults + ranges, temporal flag.

**Temporal effects (4 truly temporal, not 6)**: Echo, Posterize Time, Freeze, Channel Delay — bind `u_prev_frame`. Screen Split + Frame Stutter use a **480-frame ring buffer at 1/4 resolution** instead; do not use `u_prev_frame`.

**Feedback presets (6)**: Zoom In, Spiral, Drift, Kaleidoscope, Echo, Stretch — `FeedbackProcessor.cpp:131-142`.

**Audio uniforms available in all shaders (28)**: 24 scalars + 3 arrays. Scalars: `u_rms, u_bass, u_mid, u_high, u_beatPhase, u_barPhase, u_phrasePhase, u_spectralCentroid, u_spectralFlux, u_onsetStrength, u_onsetDetected, u_dominantPitch, u_pitchConfidence, u_detectedKey, u_keyIsMajor, u_structuralState, u_bpm, u_hcdf, u_genre, u_genreConfidence, u_energyState, u_sidechainPump, u_swingRatio, u_formantPresence, u_resonancePeak, u_reeseBass`. Arrays: `u_bandEnergies[7], u_chromagram[12], u_mfccs[13]`. Global uniforms: `u_time, u_resolution`.

**Effect rendering pipeline**:
- Per-clip FX chain (in `Clip::EffectSlot`) → Transition blend (if crossfading) → Per-layer FX chain → Layer transform → Keying → Blend onto accumulator → Global FX chain → Composition transform → Master level.
- Ping-pong FBOs (`effectFBO_A_/B_`) — used by per-clip and per-layer effects.
- Temporal path: save frame after chain, bind `u_prev_frame` next frame.

**ISF Import** (`ISFShaderLoader`): Parses JSON header, accepts float/bool/long params, wraps with GLSL 410 compat defines (`TIME`, `RENDERSIZE`, `isf_FragNormCoord`, `IMG_NORM_PIXEL`). Menu: Audio-DNA > Import ISF Shader.

**Full details**: [.feature_audit/slice_07_effects_shaders.md](.feature_audit/slice_07_effects_shaders.md) — 607 lines.

**Notable surprises**:
1. Only **4 truly temporal effects** (not 6 as CLAUDE.md implies) — Screen Split and Frame Stutter use the ring buffer, not `u_prev_frame`.
2. **UniformBridge is dead code** (still declared but comment marks it unused).
3. **7 of 13 keying modes alias to fallback shaders** (EdgeDetection, ThresholdMask, ChannelR/G/B, etc.) — may not work as documented.
4. `key_vignette` shader is compiled but unreachable (no enum value selects it).
5. Effect "Zoom Warp" display name backed by `infinite_zoom` shader — name inconsistency vs CLAUDE.md.
6. `effect_drywet` and `effect_dry_wet` both compiled as aliases for the same shader (two render paths evolved separately).
7. Cross-deck transition uses integer `u_blendMode` uniform (unusual — first int uniform in the codebase).
8. LUT support is **`.cube` only** — no `.3dl`, `.look`, `.lut`, or Hald formats. DOMAIN_MIN/MAX are parsed but ignored.
9. `ShaderManager::reloadAll()` is effectively a no-op — all 200+ programs compile from embedded strings, not files.

---

# Section 5: Procedural Sources

**Summary**: **108 total sources** (not 103 as CLAUDE.md estimates) — 101 literal `registerSource` calls + 7 wireframes via `registerWireframe` helper.

**By category (from `SourceRegistry.cpp`)**:

| Category | Count |
|----------|-------|
| Noise | 3 |
| Fractal 2D | 7 |
| Fractal 3D (ray-marched) | 8 |
| 3D Torus | 8 |
| 3D Other | 8 |
| Wireframe | 7 (sphere, torus, cube, cylinder, cone, icosahedron, wolf) |
| Geometric | 11 |
| Pattern | 8 |
| Organic | 1 |
| Utility | 2 |
| Audio-Visual | 9 |
| Nature | 6 |
| Lines | 11 |
| Math | 8 |
| Lighting | 1 |
| Particle | 3 |
| Text | 2 (text_wall, text_animator) |
| Simulation | 3 (strange_attractor, gravity_well, fluid_dynamics) |
| MilkDrop | 1 (projectM preset player) |
| Routing | 1 (layer_router) |
| **Total** | **108** |

**Stateful sources (5, ping-pong FBOs)**: reaction_diffusion, cellular_automata, strange_attractor, gravity_well, fluid_dynamics. Need continuous frames to develop visible output.

**VideoPlayer** (`src/media/VideoPlayer.h/cpp`): Any FFmpeg-supported format — MP4/MOV/QuickTime/H.264/H.265/ProRes/HAP/HAP Alpha/AVI. Alpha detection: RGBA/BGRA/ARGB/ABGR/YUVA420P/YUVA444P/PAL8/HAP. `LoopMode = {Loop, PingPong, OneShot}`. Thread-safe transport via atomics.

**ImageSequence** (`src/media/ImageSequence.h/cpp`): PNG/JPG/JPEG/BMP/TIFF/GIF. Default 10 fps. Same `LoopMode` enum. Lazy GL upload per frame.

**ProjectM/MilkDrop**: Scans `*.milk;*.prjm`. 6 moods (Calm/Energetic/Psychedelic/Geometric/Dark/Minimal) via keyword heuristics. `PresetInfo` struct: `{name, path, mood, style, energy, favorite, userPreset}`. `PresetSelector` auto-switches on structural transitions every ≥4 bars, queued via `randomPresetInMood` per DROP/BUILDUP/BREAKDOWN state. **5 exposed params**: Beat Sensitivity, Speed, Warp, Decay, Gamma.

**Disk shaders**: Only **5 files on disk** (`passthrough.vert`, `hue_shift.frag`, `rgb_split.frag`, `ripple.frag`, `vignette.frag`). All other 243+ shaders are embedded in `EmbeddedShaders.h` — **not hot-reloadable** as CLAUDE.md implies.

**Full details**: [.feature_audit/slice_08_sources.md](.feature_audit/slice_08_sources.md) — 536 lines.

**Notable surprises**:
1. Actual count **108**, not CLAUDE.md's "81" or "103" — exceeds all cited numbers.
2. **Disk shaders is only 5 files** — hot-reload mechanism effectively broken for current app.
3. Text sources (text_wall, text_animator) do NOT expose runtime-editable text strings or fonts — fully procedural.
4. Category inconsistency: `infinite_zoom` is "Geometric" but `spiral_tunnel` is "3D"; `bump_light` appears in "Pattern" despite being light-model.
5. `torus_hole` registers torus controls manually (19 params) instead of using the `addTorusControls` helper.

---

# Section 6: Transitions

**Summary**: **15 clip-to-clip transition shaders** (compiled from `EmbeddedShaders.h:3856-4130`) + **25 standard blend modes** + **30 transition entries in MixMode** = 55-entry unified enum.

**15 transitions**: Dissolve, Cut, Wipe Left/Right/Up/Down (4), Push Left/Right/Up/Down (4), Zoom In, Zoom Out, Iris Circle, Flip Horizontal, Fade to Black.

**25 standard blend modes** (per-layer `blendMode` in `Layer::MixMode`): full list in slice 03 — includes Normal, Multiply, Screen, Overlay, Soft Light, Hard Light, Color Dodge, Color Burn, Linear Dodge, Linear Burn, Darken, Lighten, Difference, Exclusion, Hue, Saturation, Color, Luminosity, Add, Subtract, Average, Negation, Divide, Reflect, Glow + Dissolve (shared).

**30 transition entries** in `Layer::MixMode` (encoded with offset 101 vs blend modes per `LayerStrip` dropdown).

**Full details**: [.feature_audit/slice_03_data_model.md](.feature_audit/slice_03_data_model.md) + [.feature_audit/slice_07_effects_shaders.md](.feature_audit/slice_07_effects_shaders.md).

---

# Section 7: Composition / Global

**Summary**: `Composition` has ~30 fields including composition-level transform, cross-deck transition config, per-type autopilot, and genre awareness.

- **Composition transform**: `compPositionX`, `compPositionY`, `compScale`, `compRotation`, `compAnchorX`, `compAnchorY`. Rendered via `comp_transform` shader after the effect chain, before master level dim.
- **Cross-deck transitions**: `crossfaderBlendMode` = `{Alpha, Add, Multiply}` (3 modes). `globalTransitionSpeed` seconds. On deck switch: Renderer saves current frame as "outgoing" (`prevDeckTexture_`) and blends to new deck via `deck_transition` shader.
- **Per-type autopilot** (`PerTypeAutopilotConfig`, 11 fields): `perTypeEnabled` (default `false`), per-layer-type beat counts (Opaque default 16, Transparent default 8, FX Only default 4), per-type randomize flags, action enum.
- **Genre → deck assignment**: `genreDeckAssignment[8]` array + `autoPresetOnGenre` flag.
- **Structural scene triggering**: `structuralSceneEnabled` flag + callback on structural state changes.
- **Master**: `compOpacity` (master dim), `globalTransitionSpeed`, `crossfaderBlendMode`.

**Full details**: [.feature_audit/slice_03_data_model.md](.feature_audit/slice_03_data_model.md) — Composition subsection.

---

# Section 8: Transport / BPM / Tempo

**Summary**: 5-stage BPM stabilization pipeline + silence recovery + automatic downbeat detection + optional Ableton Link.

**BPMTracker** (`src/analysis/BPMTracker.h/cpp`):
- Range gate: `kMinBPM = 60`, `kMaxBPM = 200`.
- Confidence gate: `kConfidenceThreshold = 0.1` (very permissive).
- Octave correction: 1.8-2.2× / 0.45-0.55× ratio windows.
- Median filter: `kMedianWindowSize = 48` (~500 ms).
- Hysteresis: `kHysteresisHops = 200` (~2.1 s), `kBPMChangeThreshold = 2.0` BPM.
- Silence detection: threshold `0.005`, entry 300 ms, exit 100 ms. Phase continues free-running.
- Downbeat scoring: 0.5 bass + 0.3 flux + 0.2 HCDF, 16-beat circular buffer (`kBeatScoreBufferSize = 16`), locks after ≥8 consistent beats (`kDownbeatLockThreshold = 8`).
- Phrase bars: default 8, range [1, 32].
- 3 tracker states: `SEARCHING=0`, `LOCKING=1`, `LOCKED=2`.
- Manual override: `setManualBPM()`, `setManualMode(bool)`, `isManualMode()`.
- **No tap-tempo averaging/nudge/multiplier in the tracker itself** — UI must handle averaging externally.

**LinkSync** (`src/sync/LinkSync.h/cpp`): Thin atomic-backed wrapper around `ableton::Link`. Compile flag `AUDIODNA_BUILD_LINK` + auto-detected `AUDIODNA_HAS_LINK`. Default quantum = 4, default tempo = 120. Exposes peer count, link tempo. **No UI exposure** despite being fully wired in `MainComponent::timerCallback` (see Slice 05 surprises).

**TopBar chrome controls (26 total)**: BPM display, Play/Pause/Stop (no onClick wired), tap tempo, manual BPM toggle + text field, resync, Ableton Link toggle (not shown), beat wheel (4-segment), bar/phrase readout, master level, BPM multiplier + Quantize callbacks (no consumers).

**Full details**: [.feature_audit/slice_11_tempo_smart.md](.feature_audit/slice_11_tempo_smart.md) + [.feature_audit/slice_05_top_chrome.md](.feature_audit/slice_05_top_chrome.md).

---

# Section 9: Input / Output

## 9.1 Audio Input
See Section 1. Source modes: File, MicInput. Formats: WAV/AIFF/FLAC/Ogg/MP3. Input gain slider, input level meter.

## 9.2 Display Output
Fullscreen/windowed, display selection, test card, blackout, identify displays — managed by `OutputWindow` (`src/ui/OutputWindow.h/cpp`). **Two parallel selectors exist** (v1 hidden + TopBar v2). OutputWindow's shader list duplicates and LAGS the main Renderer — time effects and P16-P25 shaders are **not compiled** in the output GL context.

## 9.3 OSC Input (`src/osc/OscHandler.h/cpp`)
**11 address patterns**, UDP, port configurable (no compiled default):
- `/audiodna/clip/{layer}/{column}` — trigger clip
- `/audiodna/layer/{n}/opacity` — set layer opacity
- `/audiodna/layer/{n}/bypass` — toggle bypass
- `/audiodna/layer/{n}/solo` — toggle solo
- `/audiodna/layer/{n}/mute` — toggle mute
- `/audiodna/deck/{n}` — switch deck
- `/audiodna/master` — set master level
- `/audiodna/bpm` — set BPM
- `/audiodna/snapshot` — capture PNG
- `/audiodna/macro/{n}` — set macro
- `/audiodna/effect/{name}/{param}` — set effect param

## 9.4 REST API — Production (`src/api/ApiServer.h/cpp`, port 7070)
**20 handlers**, always-on, binds `0.0.0.0` (world-reachable), CORS `*` on all responses.
- `GET /api/health`
- `GET /api/status`
- `GET /api/composition` (full deck/layer/clip tree)
- `POST /api/trigger_clip`
- `POST /api/trigger_column`
- `POST /api/set_param`
- `POST /api/set_layer_opacity`
- `POST /api/switch_deck`
- `POST /api/snapshot`
- `GET /api/bpm`
- `POST /api/set_bpm` (documented **no-op stub** at `ApiServer.cpp:515`)
- `GET /api/features`
- `POST /api/inject_features`
- `POST /api/load_image`
- `POST /api/load_source`
- `POST /api/set_effect`
- `GET /api/effects`
- `GET /api/sources`
- `POST /api/render_frame` (bypasses width/height, unlike test variant)
- `POST /api/reset`

## 9.5 REST API — Test Server (`src/test/TestServer.h/cpp`, port 8080)
**17 handlers**, gated by `AUDIODNA_BUILD_TEST_SERVER=ON` + `--test-mode`, binds `localhost` only, no CORS. Endpoints include variants of the above with additional test-only features: `set_locked_resolution`, `set_effect_chain`, `list_signals`, `add_route`, `remove_route`, `list_routes`, `set_macro` (**documented stub**), MilkDrop query, golden-frame render.

## 9.6 Video Recording (`src/recording/VideoRecorder.h/cpp`)
**3 codecs**:
- **H.264** (libx264): CRF 23 default, `.mp4`
- **ProRes** (prores_ks): profile 0-5, `.mov`
- **MJPEG**: qmin=2, qmax=31, `.mp4`

Triple-buffered GL readback, no mutex on hot path (drops frames rather than blocks). Menu: Output > Start/Stop Recording. Saves to `~/Documents/Audio-DNA/Recordings/`.

## 9.7 Session Recording (`src/recording/SessionRecorder.h/cpp`)
**7 event types**, JSON format `{version, events:[]}`, time via `juce::Time::getMillisecondCounterHiRes`. Start/stop controls. Load/save `.session` files.

## 9.8 Syphon Output (`src/output/SyphonOutput.h/.mm`, macOS only optional)
Obj-C++ wrapper around `SyphonServer`. Zero-copy GPU texture sharing via IOSurface. Default server name `"Audio-DNA"`. `__has_include(<Syphon/Syphon.h>)` gated.

## 9.9 Syphon Input (`src/output/SyphonInput.h/.mm`, macOS only)
Obj-C++ wrapper around `SyphonClient`. Lists available servers via `SyphonServerDirectory`.

## 9.10 Spout Output / NDI Input+Output
Stub-only headers. No compile-time wiring. Spout requires Spout2 SDK (Windows). NDI requires separate NDI SDK download.

## 9.11 Snapshot
`Renderer::takeSnapshot()` saves timestamped PNG to `~/Documents/Audio-DNA/Snapshots/`. Bindable via `Binding::Action::Snapshot`. Available via REST (`POST /api/snapshot`) and OSC (`/audiodna/snapshot`).

## 9.12 Python test client (`tests/visual/vj_controller.py`)
**17 public methods** on `VJAppController`: health, load_image, set_effect, set_effect_chain, inject_features, render_frame, state, reset, load_source, update_source_params, list_sources, list_signals, add_route, remove_route, list_routes, set_macro + start/stop lifecycle. Env vars: `AUDIODNA_EXE`, `AUDIODNA_TEST_PORT` (default 8080), `AUDIODNA_NO_SPAWN`.

**Full details**: [.feature_audit/slice_10_osc_api_recording_output.md](.feature_audit/slice_10_osc_api_recording_output.md) + [.feature_audit/slice_15_cmake_tests.md](.feature_audit/slice_15_cmake_tests.md).

**Notable surprises**:
1. `POST /api/set_bpm` is a **no-op stub** — documented at `ApiServer.cpp:515`.
2. Production API binds `0.0.0.0` (world-reachable) vs test API `localhost` — deliberate asymmetry.
3. No mutex on VideoRecorder submission — drops frames rather than blocks.

---

# Section 10: Bindings (Keyboard + MIDI)

**Summary**: 19 binding actions + 3 trigger/CC/targeting mode axes + MIDI learn/keyboard learn flows.

**`Binding::Action` (19 values)** — `src/binding/Binding.h`:
1. TriggerClip
2. TriggerColumn
3. ToggleLayerBypass
4. ToggleLayerSolo
5. ToggleLayerMute
6. ToggleLayerAutopilot
7. ToggleLayerVisible
8. LayerTransport
9. ToggleEffectBypass
10. AdjustMacro
11. AdjustLayerOpacity
12. SwitchDeck
13. TapTempo
14. Resync
15. GlobalPlayPause
16. GlobalStop
17. MasterOpacity
18. Snapshot
19. ToggleRecording

**Trigger modes (2)**: Toggle (default — press to toggle) vs Momentary (held = active, release = deactivate).

**CC modes (2)**: Absolute (0-127 → 0-1) vs Relative (< 64 = decrement, > 64 = increment; for endless encoders). Step size field.

**Targeting modes (3)**: ByPosition (survives reorder), ThisItem (follows clip by ID), Selected (current UI selection).

**Additional fields**: `velocityToOpacity` (maps MIDI velocity to clip opacity on trigger), keyCode, midiChannel, midiNote, midiCC.

**MIDI Output** (`src/midi/MidiOutputHandler.h/cpp`): Polls deck state ~6 Hz. Note formula: `(layer+1)*10 + (column+1)` (Launchpad grid layout, but **overflows for columns ≥ 8** — allowed up to 20). **Hardcoded channel 1**. 5 velocity codes: Empty=0, Loaded=5, Playing=60, Triggered=52, ActiveWithFx=62. Supported controllers: **Launchpad X + Launchpad Mini MK3** (shared velocity table). APC40 mentioned in header comment but no APC-specific code.

**MIDI Input** (`src/midi/MidiHandler.h/cpp`): Auto-enables every input device (no opt-in). Non-note/non-CC messages silently dropped.

**Key-up routing**: `MainComponent::keyStateChanged()` polls all momentary-bound keys and fires release actions.

**Binding persistence**: Relative CC state and `id` are NOT serialized. `Binding::Action` enum stored as raw int — reorder breaks saved presets.

**No default bindings seeded** anywhere in code — app starts with zero bindings.

**Full details**: [.feature_audit/slice_09_bindings_midi.md](.feature_audit/slice_09_bindings_midi.md) — 390 lines.

---

# Section 11: Smart / AI Features

**Summary**: Real-time genre classification + advanced audio analysis + AI mapping suggestions + smart BPM recovery.

**Genre Detection (`src/analysis/GenreDetector.h/cpp`)**: 8 genres, scoring from BPM range + spectral profile + transient density + chromatic complexity. EMA smoothing ~2s, hysteresis ~3s, confidence gate 0.15, 3-level energy state.

**All 8 genres** (`GenreDetector.h:26-33`):
| ID | Constant | Display |
|----|----------|---------|
| 0 | `kHouse` | House |
| 1 | `kTechno` | Techno |
| 2 | `kDnB` | DnB |
| 3 | `kHipHop` | HipHop |
| 4 | `kAmbient` | Ambient |
| 5 | `kRock` | Rock |
| 6 | `kPopElectronic` | Pop/Electronic (default) |
| 7 | `kJazzOther` | Jazz/Other |

**GenreSmoothing (`src/analysis/GenreSmoothing.h`)**: Per-genre EMA attack/release alphas. Techno/DnB fast attack (0.50-0.55), Ambient slow (0.12), HipHop punchy attack + smooth release. One-Euro params also provided.

**Advanced Audio Analysis (`src/analysis/AdvancedAudioAnalyzer.h/cpp`)** — 5 P25 features:
1. `sidechainPump` — Pearson correlation of bass vs mid envelopes (64-hop window ~680 ms)
2. `swingRatio` — IOI histogram, 32-onset window, 0.5=straight, 0.67=swung
3. `formantPresence` — energy ratio in 300-3000 Hz vs total, adaptive normalization
4. `resonancePeak` — spectral kurtosis in 200-8000 Hz
5. `reeseBass` — spectral spread (weighted std dev) in 30-200 Hz

All 5 routable via `MappingSource` enum, shader uniforms (`u_sidechainPump`, etc.), and hidden signals in `SignalRegistry`.

**Mapping Suggester (`src/mapping/MappingSuggester.h/cpp`)**: 8 genres × 4 rules each = 32 genre rules + ~11 universal rules (fire conditionally on bass/onset/centroid/rms/mid/high activity). Returns sorted suggestions with `{sourceName, targetCategory, targetEffect, targetParam, curveType, reason, relevance}`.

**Smart BPM Recovery**: `BPMTracker::feedSilenceDetection(rms)` detects silence (RMS < 0.005 for 300 ms) and holds last good BPM during DJ transitions/track endings. Phase continues free-running during silence. Resumes after 100 ms of audio above threshold.

**ISF Shader Import** (`src/effects/ISFShaderLoader.h/cpp`): Imports `.isf/.fs` shaders from isf.video. Parses JSON metadata, extracts param definitions (float/bool/long), wraps with GLSL 410 compat defines. Registers as effects in EffectLibrary with "ISF" category.

**Full details**: [.feature_audit/slice_11_tempo_smart.md](.feature_audit/slice_11_tempo_smart.md) — 380 lines.

**Notable surprises**:
1. `GenreDetector` exposes **no `onGenreChanged` callback** — UI must poll.
2. `kConfidenceThreshold = 0.1f` is very permissive.

---

# Section 12: Browser / Library

**Summary**: `BrowserPanel` (`src/ui/BrowserPanel.h:39`) hosts exactly **6 top-level tabs** (Files/FX/Sources/Comp-Decks/Record/MilkDrop).

## 12.1 Files Browser (`src/ui/FilesBrowser.h/cpp`)
File tree navigation, favorites, recent, filters. Drag-drop format: `"files:p1|p2"`.

## 12.2 FX Browser (`src/ui/FXBrowser.h/cpp`)
**11 categories** (matches effects library). Multi-select supported. Drag-drop format: `"fx:Name1,Name2,Name3"` (comma-separated).

## 12.3 Sources Browser (`src/ui/SourcesBrowser.h/cpp`)
**18 categories** enumerated (CLAUDE.md says 15 — discrepancy). ~96 sources visible in browser (vs 108 total registered). Drag-drop format: `"source:id1,id2"`.

## 12.4 Comp-Decks Browser (`src/ui/CompDecksBrowser.h/cpp`)
Deck list, deck switch, save/load. Right-click deletes files **without confirmation** (risky).

## 12.5 MilkDrop Browser (`src/ui/MilkDropBrowser.h/cpp`) — only sub-tabbed browser
**4 sub-tabs**: Curated, Favorites, Recent, All.
**3 play modes**: Jukebox, VJClip, Playlist.
Drag-drop format: `"milkdrop:path"` or `"milkdrop_playlist:p1|p2"`.
`playlistTimingSelector_.onChange` is **never wired**.

## 12.6 Record Panel (`src/ui/RecordPanel.h/cpp`)
Record/playback session controls, save/load `.session` files.

**Drag-drop source summary** (6 distinct formats): `"fx:..."`, `"source:..."`, `"files:..."`, `"clip:layer:col"`, `"milkdrop:..."`, `"milkdrop_playlist:..."`.

**ClipCell** (`src/ui/ClipCell.h/cpp`): Thumbnail, play/stop indicator. **Right-click explicitly early-returns** — the clip right-click menu (copy/paste/clear/rename/cuepoints) described in CLAUDE.md is NOT implemented.

**LayerStrip** (`src/ui/LayerStrip.h/cpp`): X/B/S cluster (bypass/solo/mute), 4-button transport, scrub bar, 4 vertical sliders (S/K/V/F = Opacity/Keying?/Blend/Transition), V dropdown has **68 options** (55 mix modes + 13 keying modes, encoded with offset 101), F dropdown has 55.

**Full details**: [.feature_audit/slice_06_browsers_deck_ui.md](.feature_audit/slice_06_browsers_deck_ui.md) — 616 lines.

**Notable surprises**:
1. **Zero true right-click menus** — ClipCell early-returns on right-click; right-click is used only for binary toggles (favorite / delete) elsewhere.
2. `EffectStackView` has **no drag-reorder**.
3. `EffectsRackPanel` is legacy and knows only 8 of the current 11 FX categories.
4. Category counts in UI (FX 11, Sources 18) vs code (FX 11, Sources ~20) vs CLAUDE.md (Sources "15") all disagree.

---

# Section 13: Inspector Tabs and Sections

**Summary**: `InspectorPanel` hosts 4 tabs (Clip / Layer / Composition / Signal) + pin control.

**Inspector section counts**:
- **ClipInspector**: 9 sections (Transport, Transform, Video, Effects, Source Parameters, Cuepoints, Beat Division, ...)
- **LayerInspector**: 10-12 sections (type-conditional) — Opacity, Blend Mode, Transform, Keying, Feedback, Transition, Effects, 3D Rotation, Autopilot, Persistent flags, ...
- **CompositionInspector**: 9 sections
- **SignalInspector**: 1 conditional-with-sub-panel (header comment promises routes list but **not implemented**)

**Control totals**: ~21 ComboBoxes (including LayerInspector's 53-entry `transitionBlendSelector` with 14 section headings + 25-entry `blendModeSelector`), ~40 ResettableSliders with ranges/defaults/bindings, ~25 TextButtons, ~12 ToggleButtons.

**UniversalParamControl source-picker popup** (the "signal connect triangle"):
- Manual
- Audio (→ submenu of all visible audio signals)
- BPM Sync → 4 wave shapes × 6 beat divisions
- Oscillator
- Envelope
- Clip Position
- Timeline
- Macro (→ submenu of 8 macros)

**MappingEditor** (`src/ui/MappingEditor.h/cpp`): Standalone popup with source dropdown, target effect/param dropdown, curve dropdown (23 options), inputMin/inputMax sliders, outputMin/outputMax sliders, smoothing slider, enabled toggle.

**MacroPanel** (`src/ui/MacroPanel.h/cpp`): 8 macro knobs per scope × 3 scopes. Each macro: name edit, manualValue knob, signal source dropdown, inverted toggle, link list, delete.

**PresetManager** (`src/ui/PresetManager.h/cpp`): **Static-only**, no UI in this file. Save/load preset format is JSON.

**Full details**: [.feature_audit/slice_04_inspectors.md](.feature_audit/slice_04_inspectors.md) — 694 lines.

**Notable surprises**:
1. **SignalInspector promises a routes list but doesn't implement it.**
2. **No right-click context menus anywhere** — only reset-to-default.
3. ClipInspector's `triggerDropdown_` has **NO onChange handler wired**.
4. CompositionInspector's UniversalParamControls **skip `setDefaultValue()` calls** — right-click reset won't work.
5. ClipInspector uses **Ctrl/Cmd-click** (not right-click) to clear cuepoints.
6. LayerInspector's name label editing is triggered programmatically via mouseDown rather than `setEditable(true, ...)`.

---

# Section 14: Menu Bar

**Summary**: **9 menus, ~70 items total**, command IDs 1000-1852 in `MenuBarModel.cpp:9-10`. Menu names: **Audio-DNA / Composition / Deck / Layer / Column / Clip / Output / Shortcuts / View**.

**~35 of ~70 items fall to `default: DBG("not yet implemented")`** at `MainComponent.cpp:3258` — effectively placeholder entries in menus.

**Full item-by-item list with command IDs, hotkeys, and implementation status**: [.feature_audit/slice_05_top_chrome.md](.feature_audit/slice_05_top_chrome.md) — 802 lines (extensive subsections 14.1 through 14.9).

**Notable surprises**:
1. **Keyboard shortcuts are hardcoded in `MainComponent::keyPressed`** and **never shown in menu items** — users can't discover them through menus.
2. ~50% of menu items are unimplemented stubs.

---

# Section 15: Preferences Dialog

**Summary**: **8 tabs** but **only 14 interactive controls** total. **5 tabs are placeholders** with no change handlers wired: MIDI, Recording, Defaults, Feedback + most of Audio/Video.

**Tabs**: General, Audio, Video, Display, MIDI, Recording, Defaults, Feedback.

**Full control inventory per tab**: [.feature_audit/slice_05_top_chrome.md](.feature_audit/slice_05_top_chrome.md) — Section 15 subsections.

---

# Section 16: Dialogs and Modal Surfaces

**Summary**: BindingOverlay, MidiLearnOverlay, ProgrammingMode, TimingWindow (3 empty tabs), OutputWindow, PreferencesDialog, MappingEditor (popup), Rename dialog, Replace Content, Import ISF, Save/Load Layout, Collect Media, Relocate Missing Files, Edit Keyboard Shortcuts, Edit MIDI Mappings.

**Notable surprises**:
- `ProgrammingMode` class is instantiated but **permanently hidden** (never shown).
- `TimingWindow`'s 3 tabs are **empty placeholders**.

**Full details**: [.feature_audit/slice_05_top_chrome.md](.feature_audit/slice_05_top_chrome.md) + [.feature_audit/slice_04_inspectors.md](.feature_audit/slice_04_inspectors.md).

---

# Section 17: Right-click / Context Menus

**Summary**: Despite CLAUDE.md references to right-click menus, **most are not implemented** in the current UI.

Actual right-click behavior per surface:
- **Sliders (ResettableSlider)**: right-click = reset to default (universal).
- **ClipCell**: right-click **explicitly early-returns** — no menu.
- **Inspectors**: no right-click menus; use Ctrl/Cmd-click for certain actions (e.g., clear cuepoint).
- **CompDecksBrowser**: right-click deletes file without confirmation.
- **Browser item favorites**: right-click toggles favorite (binary).

**No copy/paste/rename/replace/cuepoint-management right-click menu exists on clip cells** — despite being cited in CLAUDE.md.

**Full details**: [.feature_audit/slice_06_browsers_deck_ui.md](.feature_audit/slice_06_browsers_deck_ui.md) + [.feature_audit/slice_04_inspectors.md](.feature_audit/slice_04_inspectors.md).

---

# Section 18: Features Mentioned in Docs but NOT in Code (or Incomplete)

This section flags every mismatch between the aspirational docs (CLAUDE.md, ARCHITECTURE_V2.md, UNIFIED_BUILD_PLAN.md, research/*, OurAppFeatures_1.md) and the actual code. Designers should treat these as gaps when planning the new UI.

## 18.1 Phase-level promises partly or fully missing in code

From [.feature_audit/slice_12_toplevel_docs.md](.feature_audit/slice_12_toplevel_docs.md):

1. **P26 Comprehensive Tooltips** — NOT STARTED (PHASE_GUIDE.md:207). Many UI elements still lack tooltips.
2. **P12.3 Syphon/NDI** — only **partially landed** in P22: Syphon is optional compile flag with compile-time stub, NDI is header-only stub, Spout is Windows-only stub.
3. **P12.5 Cross-platform testing** — DEFERRED. Windows/Linux CI not known to be current.
4. **P22 promised HAP Alpha video recording** (ARCHITECTURE_V2 §18/§24); actual code ships H.264/ProRes/MJPEG only. Direct contradiction.
5. **P25.6 Audio stem separation (Demucs ML)** — planned, not shipped.
6. **P20 Part C FFGL Plugin Hosting** — deferred.

## 18.2 ARCHITECTURE_V2 counts are stale

- §19 says **"40 sources"** — actual is **108** (Slice 08 count) — doc is 2.7× too low.
- §27 says **"76 effects"** — actual is **135** — doc is 1.8× too low.
- §24 says **"HAP Alpha primary codec"** — actual is FFmpeg-generic, no HAP codec in VideoRecorder.

## 18.3 README.md is entirely v1

Still describes the keyboard launcher workflow, not the v2 deck/layer/signal system. Also says JUCE 8.0 where CLAUDE.md says 7.0.12 (neither matches the current 8.0.4 per slice 15).

## 18.4 Audio features in research but possibly not in code

From [.feature_audit/slice_13_research_partA.md](.feature_audit/slice_13_research_partA.md): Research describes 100+ features; code implements ~40. **Likely missing**:

- **Amplitude/dynamics**: True Peak, LRA, noise floor, SNR, clipping flag
- **Spectral**: 8 features — contrast, spread, skewness, kurtosis (full-spectrum), entropy, irregularity, decrease, slope
- **Pitch/harmony**: polyphonic detection (NMF / Harmonic Summation / Basic Pitch / CREPE), Tonnetz, chord recognition
- **Bands**: full 24-band Bark, mel spectrogram (40 bands as spectrum), ERB, CQT, octave (1/1, 1/3, 1/6)
- **MFCC**: delta MFCCs, MFCC+delta+delta² bundle
- **Texture**: ZCR, HNR, inharmonicity, HPSS outputs
- **Psychoacoustic**: Zwicker/Moore-Glasberg loudness (sone), Aures sharpness (acum), Fastl/Daniel roughness (asper), fluctuation strength, stereo width
- **Structure**: novelty curves, self-similarity matrix, segment labeling, beat prediction, tatum detection, polyphony estimate
- **Onset**: 6 ODFs mentioned (only specflux implemented via Aubio)

## 18.5 OurAppFeatures_1.md claims vs shipped

- "45+ transition modes" — actual is **15 shipped shaders** (30 entries in MixMode but many are blend+transition duplicates).
- "80+ FeatureSnapshot fields" — actual is **40 rows** / 33 declarations.
- Sources "15 categories" — actual is **17 enumerated in UI** / ~20 in code.

## 18.6 Preferences placeholder tabs

Per [.feature_audit/slice_05_top_chrome.md](.feature_audit/slice_05_top_chrome.md): MIDI / Recording / Defaults / Feedback tabs in Preferences have **no change handlers wired** — the controls exist but don't persist or take effect.

## 18.7 TopBar controls without consumers

- **Play/Pause/Stop** buttons have no `onClick` handlers.
- BPM multiplier and Quantize callbacks have no consumers.
- **Ableton Link has zero UI exposure** despite being fully wired in `timerCallback`.

## 18.8 Menu bar: ~50% unimplemented

From [.feature_audit/slice_05_top_chrome.md](.feature_audit/slice_05_top_chrome.md): **~35 of ~70 menu items fall to `default: DBG("not yet implemented")`**.

## 18.9 FX_SOURCE_AUDIT open items

- **Julia Set source shader compile failure (critical)** — marked 💀.
- **15 effects with missing parameters** (registered without full param lists).
- **Near-duplicate effects**: Swirl/Twirl, Liquid/Liquid Morph, Ripple/Ripple Pond, two Wormholes, three torus variants.
- **Missing industry-standard effects**: Color Curves, Displacement Map, Tilt Shift, God Rays, Lens Flare.
- **Missing sources**: runtime-editable Text source (text_wall/animator are procedural-only).

## 18.10 Research-only features (never implemented)

From [.feature_audit/slice_14_research_partB.md](.feature_audit/slice_14_research_partB.md):

- **Flame Fractals** and **L-Systems** sources — researched, never shipped.
- **11 Resolume effects explicitly SKIPPED**: Keystone×3, Expand, Dilate, Reducto, Circles, Snow, Twitch, Sparkles, Smooth Transform, Pixel High Pass, Displace.
- **3 sources SKIPPED**: Text Block, Test Card, Slice Outline.
- **Command palette** / **visual effect thumbnails** / **signal routing visualization on hover** / **EQ-colored waveform** — UI/UX Must-Have list items, partially or not adopted.

---

# Section 19: Features Present in Code but NOT in Docs

Surprises the subagents found that are in the code but not explained in CLAUDE.md or research — the flip side of Section 18.

## 19.1 Analysis / audio features

- **`pcmSnapshot_` / `getPCMSamples()` path** — 512-sample lock-free double-buffer for external consumers like projectM — undocumented. (`AnalysisThread.h:70-73, .cpp:345-372`)
- **CPU load EMA** and **per-stage profiler** printing to stderr every 500 hops — always-on dev surface. (`AnalysisThread.cpp:315-335`)
- **`FeatureSnapshot::clear()` leaves `detectedGenre = 0` (House) instead of default 6** — latent bug.
- **Raw BPM diagnostic** (`rawBPM()` accessor) and **test-only `processRawBPM(rawBpm, conf, beat)`** — undocumented diagnostic surfaces.

## 19.2 Signal / routing

- `MappingEngine::processFrame` **accumulates** multiple mappings to the same param (sums, then clamps) — users can chain multiple signals into one target without knowing this.
- `SignalRegistry::getCachedValue` is **O(N) linear scan** — not indexed. Perf risk with many signals/routes.
- **Hidden audio signals for advanced analysis** (P25): Sidechain Pump, Swing, Vocal Presence, Resonance, Reese Bass are registered hidden — UI can surface these.
- **ClipPositionSignal** is registered hidden by default — can be made visible.
- **`ChainedSignal` has 4 chain modes** (Multiply, Add, Gate, ScaleRange) — a signal math system that isn't advertised.

## 19.3 Data model

- **Layer::MixMode is one 55-value enum** for both blend + transition — not two separate enums as docs imply.
- **Undo/Redo infrastructure is wired but has zero concrete Command subclasses**.
- **`FeedbackConfig` is NOT serialized** to presets — feedback settings are lost on save.
- **Composition serialization silently drops 5+ fields** — compOpacity, crossfader state, comp transform, per-type autopilot, genre-aware fields.

## 19.4 Effects / rendering

- **UniformBridge is dead code** — declared but unused.
- **7 of 13 keying modes alias to fallback shaders** — may not produce the documented result.
- **`key_vignette` shader is compiled but unreachable** (no enum value selects it).
- **`effect_drywet` and `effect_dry_wet` are aliased** — two render paths that evolved separately.
- **Cross-deck transition uses integer `u_blendMode` uniform** — the only int uniform in the codebase.
- **Only `.cube` LUT format supported** — no `.3dl`, `.look`, `.lut`, Hald; DOMAIN_MIN/MAX parsed but ignored.
- **`ShaderManager::reloadAll()` is a no-op** — 200+ programs compile from embedded strings, not disk.

## 19.5 Sources

- **Only 5 shader files on disk** (vs 200+ embedded in `EmbeddedShaders.h`) — disk shaders directory is nearly empty; hot-reload mechanism is effectively unused.
- `torus_hole` uses **bespoke 19-param registration** instead of the `addTorusControls` helper.

## 19.6 Bindings / MIDI

- **Note formula overflows for columns ≥ 8** — `(layer+1)*10 + (col+1)` is allowed up to column 20 but Launchpad is 8×8.
- **MIDI output hardcoded to channel 1** — no config.
- **`MidiHandler::start()` auto-enables every input device** with no opt-in.
- **Relative CC state and binding `id` not serialized** — presets can't round-trip cleanly.
- **`Binding::Action` stored as raw int** — reordering the enum breaks saved presets.
- **No default bindings seeded** — app starts with zero bindings.

## 19.7 OSC / API / output

- **`POST /api/set_bpm` is a documented no-op stub**.
- **Production API binds `0.0.0.0`** — world-reachable. Test API binds `localhost`. Deliberate asymmetry.
- **VideoRecorder has no mutex on submit path** — drops frames rather than blocks.
- **httplib is always linked** — CLAUDE.md says it was test-only, but it's now always linked for ApiServer.

## 19.8 UI / interaction

- **ClipCell's right-click early-returns** — no clip cell right-click menu exists.
- **Many sliders lack `setDefaultValue()` calls** — right-click reset won't work on those sliders (CompositionInspector UniversalParamControls, etc.).
- **ClipInspector `triggerDropdown_.onChange` is not wired** — changes don't take effect.
- **Keyboard shortcuts are hardcoded and not shown in menus** — undiscoverable.
- **LayerStrip V/F dropdowns use offset 101 to pack two enums into one ID space** — non-obvious.
- **ProgrammingMode class instantiated but permanently hidden**.
- **TimingWindow has 3 empty placeholder tabs**.
- **Two parallel display selectors exist** — v1 hidden + TopBar v2.

---

# Section 20: Cross-References

**Per major capability, authoritative file locations**:

| Capability | Primary file:line |
|------------|-------------------|
| Audio callback (RT) | `src/audio/AudioCallback.cpp:8-45` |
| SPSC ring buffer | `src/audio/RingBuffer.h:10-83` |
| Analysis pipeline orchestration | `src/analysis/AnalysisThread.cpp:53-355` |
| FeatureSnapshot POD | `src/analysis/FeatureSnapshot.h:5-90` |
| FeatureBus triple-buffer | `src/features/FeatureBus.h:21-72`, `.cpp:1-82` |
| MappingSource enum (58 values) | `src/mapping/MappingTypes.h:6-87` |
| MappingCurve enum (23 curves) | `src/mapping/MappingTypes.h:90-120` |
| CurveTransforms impl | `src/mapping/CurveTransforms.h:13-252` |
| Mapping struct (10 fields) | `src/mapping/MappingTypes.h:123-140` |
| MappingEngine pipeline | `src/mapping/MappingEngine.cpp:135-216` |
| Signal base | `src/signal/Signal.h:9-54` |
| OscillatorSignal (5 waves) | `src/signal/OscillatorSignal.h:8-77` |
| EnvelopeSignal (3 curves, points) | `src/signal/EnvelopeSignal.h:10-126` |
| ClipPositionSignal | `src/signal/ClipPositionSignal.h:8-45` |
| ChainedSignal (4 chain modes) | `src/signal/ChainedSignal.h:10-54` |
| SignalRegistry::initDefaults (32 signals) | `src/signal/SignalRegistry.cpp:4-84` |
| Route struct (17 fields) | `src/routing/Route.h:8-41` |
| RoutingEngine pipeline | `src/routing/RoutingEngine.cpp:77-126` |
| MacroBank (8×3 = 24) | `src/routing/MacroBank.h:13-97` |
| Clip data model | `src/model/Clip.h/cpp` |
| Layer data model (MixMode 55-enum) | `src/model/Layer.h/cpp` |
| Deck data model | `src/model/Deck.h` |
| Composition data model | `src/model/Composition.h` |
| Autopilot | `src/model/Autopilot.h/cpp` |
| Undo/Redo infrastructure (unused) | `src/core/Command.h`, `UndoManager.h/cpp` |
| EffectLibrary (135 effects) | `src/effects/EffectLibrary.cpp` |
| EmbeddedShaders (200+ GLSL strings) | `src/render/EmbeddedShaders.h` |
| CompositorEngine (deck compositing) | `src/render/CompositorEngine.cpp` |
| FeedbackProcessor (6 presets) | `src/render/FeedbackProcessor.cpp:131-142` |
| Renderer (top-level) | `src/render/Renderer.h/cpp` |
| ISFShaderLoader | `src/effects/ISFShaderLoader.cpp` |
| LUTLoader (.cube only) | `src/render/LUTLoader.cpp` |
| SourceRegistry (108 sources) | `src/sources/SourceRegistry.cpp` |
| ProceduralSource | `src/sources/ProceduralSource.cpp` |
| VideoPlayer (FFmpeg) | `src/media/VideoPlayer.cpp` |
| ImageSequence | `src/media/ImageSequence.cpp` |
| ProjectMSource (MilkDrop) | `src/sources/ProjectMSource.cpp` |
| BPMTracker (5-stage pipeline) | `src/analysis/BPMTracker.h/cpp` |
| GenreDetector (8 genres) | `src/analysis/GenreDetector.h/cpp` |
| AdvancedAudioAnalyzer (5 P25 features) | `src/analysis/AdvancedAudioAnalyzer.h/cpp` |
| LinkSync (Ableton Link) | `src/sync/LinkSync.h/cpp` |
| Binding / BindingManager | `src/binding/Binding.h`, `BindingManager.h/cpp` |
| MidiHandler (input) | `src/midi/MidiHandler.cpp` |
| MidiOutputHandler (Launchpad) | `src/midi/MidiOutputHandler.cpp` |
| OscHandler (11 addresses) | `src/osc/OscHandler.cpp` |
| ApiServer (REST, port 7070) | `src/api/ApiServer.cpp` |
| TestServer (port 8080, conditional) | `src/test/TestServer.cpp` |
| VideoRecorder (H264/ProRes/MJPEG) | `src/recording/VideoRecorder.cpp` |
| SessionRecorder | `src/recording/SessionRecorder.cpp` |
| SyphonOutput (.mm, macOS) | `src/output/SyphonOutput.mm` |
| SyphonInput | `src/output/SyphonInput.mm` |
| Menu bar (9 menus, ~70 items) | `src/ui/MenuBarModel.cpp` |
| PreferencesDialog (8 tabs) | `src/ui/PreferencesDialog.cpp` |
| TopBar (26 controls) | `src/ui/TopBar.cpp` |
| BrowserPanel (6 tabs) | `src/ui/BrowserPanel.h:39` |
| ClipInspector (9 sections) | `src/ui/ClipInspector.cpp` |
| LayerInspector (10-12 sections) | `src/ui/LayerInspector.cpp` |
| CompositionInspector (9 sections) | `src/ui/CompositionInspector.cpp` |
| SignalInspector | `src/ui/SignalInspector.cpp` |
| MappingEditor | `src/ui/MappingEditor.cpp` |
| UniversalParamControl (picker popup) | `src/ui/UniversalParamControl.cpp` |
| ResettableSlider (right-click reset) | `src/ui/UniversalParamControl.h` |
| DeckView (grid) | `src/ui/DeckView.cpp` |
| ClipCell | `src/ui/ClipCell.cpp` |
| LayerStrip | `src/ui/LayerStrip.cpp` |
| EffectStackView | `src/ui/EffectStackView.cpp` |
| MainComponent (layout, keyPressed) | `src/MainComponent.cpp` |
| CMake build options | `CMakeLists.txt` |

**Build flags**: `AUDIODNA_BUILD_LINK` (Ableton Link, OFF), `AUDIODNA_BUILD_TEST_SERVER` (Eyes API, OFF), `AUDIODNA_BUILD_SYPHON` (macOS, OFF). Auto-detected: `AUDIODNA_USE_CAMERA` (APPLE/WIN32), `AUDIODNA_HAS_PROJECTM`, `AUDIODNA_HAS_LINK`, `AUDIODNA_HAS_SYPHON`.

---

# Appendix: Per-slice fragment index

All slice fragments are in `/Users/boriskarpman/Documents/RealTimeAudio/.feature_audit/`. Read a fragment when you need the full line-by-line detail for a section:

| Section | Fragment | Lines |
|---------|----------|-------|
| 1 Audio Analysis | slice_01_audio_analysis.md | 413 |
| 2 Signal/Mapping/Routing | slice_02_signal_mapping_routing.md | 527 |
| 3 Data Model | slice_03_data_model.md | 781 |
| 4 Inspectors | slice_04_inspectors.md | 694 |
| 5, 14, 15, 16 Top Chrome | slice_05_top_chrome.md | 802 |
| 12 Browsers + Deck | slice_06_browsers_deck_ui.md | 616 |
| 4 Effects | slice_07_effects_shaders.md | 607 |
| 5 Sources | slice_08_sources.md | 536 |
| 10 Bindings | slice_09_bindings_midi.md | 390 |
| 9 I/O | slice_10_osc_api_recording_output.md | 318 |
| 8, 11 BPM + Smart | slice_11_tempo_smart.md | 380 |
| 18 Doc gaps | slice_12_toplevel_docs.md | 249 |
| 1 Audio research | slice_13_research_partA.md | 393 |
| 5, 18 Source research | slice_14_research_partB.md | 566 |
| 9, 15 CMake + Tests | slice_15_cmake_tests.md | 467 |
| **Total** | | **7,739** |

---

*End of FEATURE_INVENTORY.md. Designer: start with the summary in each section, drill into fragments for line-level detail, pay attention to Sections 18 & 19 for gaps/surprises.*
