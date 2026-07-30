# File Map — Audio-DNA

## Entry Points
- `src/Main.cpp` — JUCE app entry (JUCEApplication subclass)
- `src/MainComponent.h/cpp` — Top-level component, owns all systems, layout, DragAndDropContainer

## Audio Pipeline
- `src/audio/AudioEngine.h/cpp` — AudioDeviceManager + AudioTransportSource + file loading
- `src/audio/AudioCallback.h/cpp` — RT callback → mono downmix → ring buffer push (<100μs)
- `src/audio/RingBuffer.h` — Lock-free SPSC, power-of-two, cache-line padded (16384 floats)

## Analysis Pipeline (14 stages — 14 numbered compute stages, 13 profiled; runs every 10.7ms)
- `src/analysis/AnalysisThread.h/cpp` — Dedicated thread, reads ring buffer, runs pipeline
- `src/analysis/FeatureSnapshot.h` — POD struct, all analysis fields, alignas(64)
- `src/analysis/FFTProcessor.h/cpp` — juce::dsp::FFT wrapper, 2048-pt, Hann window
- `src/analysis/SpectralFeatures.h/cpp` — Centroid, flux, flatness, rolloff, 7-band energies
- `src/analysis/OnsetDetector.h/cpp` — Aubio onset wrapper
- `src/analysis/BPMTracker.h/cpp` — Aubio tempo + stabilization pipeline + downbeat detection
- `src/analysis/MFCCExtractor.h/cpp` — Mel filterbank (40 bands) + DCT → 13 MFCCs
- `src/analysis/ChromaExtractor.h/cpp` — FFT bins → 12 pitch classes + HCDF
- `src/analysis/KeyDetector.h/cpp` — Krumhansl-Schmuckler key detection
- `src/analysis/LoudnessAnalyzer.h/cpp` — K-weighted LUFS (ITU-R BS.1770)
- `src/analysis/StructuralDetector.h/cpp` — Multi-scale EMA → state machine
- `src/analysis/PitchTracker.h/cpp` — Aubio yinfft pitch detection
- `src/analysis/GenreDetector.h/cpp` — 8-genre real-time classification
- `src/analysis/GenreSmoothing.h` — Per-genre EMA attack/release parameters — **DEAD** (never instantiated; GenreDetector does its own smoothing)
- `src/analysis/AdvancedAudioAnalyzer.h/cpp` — Sidechain pump, swing, formant, resonance, reese bass

## Feature Transport
- `src/features/FeatureBus.h/cpp` — Triple-buffer atomic swap (3x FeatureSnapshot)
- `src/features/Smoother.h` — EMA `Smoother` (used) + `OneEuroFilter` (**DEAD** — implemented, never instantiated) (header-only)

## Mapping & Routing
- `src/mapping/MappingEngine.h/cpp` — Source→curve→scale→target routing
- `src/mapping/MappingTypes.h` — Mapping, Source, Curve enums
- `src/mapping/CurveTransforms.h` — 24 curve functions (5 classic linear/exp/log/scurve/stepped + 19 P24 easings)
- `src/mapping/MappingSuggester.h/cpp` — AI mapping suggestions by genre — **GHOST** (never instantiated; no UI/API caller)
- `src/signal/SignalRegistry.h/cpp` — Named signal registry
- `src/signal/ChainedSignal.h/cpp` — Derived signals via math ops — **GHOST** (never instantiated)
- `src/routing/RoutingEngine.h/cpp` — Frame-by-frame signal evaluation

## Visual Effects
- `src/effects/EffectLibrary.h/cpp` — Registry: creates Effect instances from shaders
- `src/effects/Effect.h/cpp` — Single effect: shader + param list
- `src/effects/EffectChain.h/cpp` — Ordered chain with ping-pong FBOs + temporal
- `src/effects/UniformBridge.h/cpp` — Params → glUniform calls
- `src/effects/ISFShaderLoader.h/cpp` — ISF shader import with GLSL 410 conversion

## Render Pipeline
- `src/render/Renderer.h/cpp` — OpenGL 4.1 renderer, frame loop, source rendering
- `src/render/ShaderManager.h/cpp` — Compile, link, hot-reload shaders
- `src/render/TextureManager.h/cpp` — Image → GL_TEXTURE_2D, FBO textures
- `src/render/FullscreenQuad.h/cpp` — VAO/VBO for fullscreen triangle strip
- `src/render/EmbeddedShaders.h` — All shaders inline: 135 effect + 15 transition + ~93 source + helpers (244 embedded strings; 275 compile() calls in Renderer.cpp)
- `src/render/CompositorEngine.h/cpp` — Deck/layer compositing, temporal buffers, feedback, ring buffer
- `src/render/LUTLoader.h/cpp` — Color LUT loading

## Data Model
- `src/model/Clip.h/cpp` — Media content + per-clip effects + transport
- `src/model/Layer.h/cpp` — Layers with clips, effects, opacity, blend mode
- `src/model/Composition.h/cpp` — Top-level: decks, global settings, autopilot config
- `src/model/Autopilot.h/cpp` — Auto-advance clips, smart random, structural triggering
- `src/core/UndoManager.h/cpp` — Command pattern undo/redo — **DEAD** (`perform()` never called; zero Command subclasses; undo/redo keys are no-ops)
- `src/core/Command.h` — Abstract undo/redo command base — **INFRA ONLY** (no concrete subclasses exist)

## Procedural Sources
- `src/sources/SourceRegistry.h/cpp` — Registration of all 108 procedural sources
- `src/sources/ProceduralSource.h/cpp` — Base class, param management, uniform upload

## Media
- `src/media/VideoPlayer.h/cpp` — FFmpeg video decode → GL texture
- `src/media/ImageSequence.h/cpp` — Multi-image playback with configurable FPS

## Performance & Control
- `src/binding/BindingManager.h/cpp` — Keyboard/MIDI binding management
- `src/midi/MidiHandler.h/cpp` — MIDI input processing, learn mode
- `src/midi/MidiOutputHandler.h/cpp` — Launchpad/APC pad feedback
- `src/api/ApiServer.h/cpp` — REST API (port 7070, 22 endpoints — 21 functional, /api/set_bpm is a no-op stub, CORS)
- `src/osc/OscHandler.h/cpp` — OSC input (juce_osc, 11 /audiodna/* patterns) — **INERT** (`startListening()` never called; receiver never connects; only 5/11 callbacks wired)
- `src/sync/LinkSync.h/cpp` — Ableton Link tempo sync (optional)

## Recording
- `src/recording/VideoRecorder.h/cpp` — Real-time video capture (FFmpeg, triple-buffered GL readback)
- `src/recording/SessionRecorder.h/cpp` — Timestamped event recording/playback — **PARTIAL** (only clip triggers captured, 6/7 record* unused; `advancePlayback()` never called → playback dead)

## Output (`src/output/`)
> NOTE: OutputWindow does NOT live here — it is `src/ui/OutputWindow.h/cpp` (see UI section). `src/output/` holds only the external-share outputs below.
- `src/output/SyphonOutput.h/.mm` — macOS Syphon server (zero-copy GPU texture sharing) — **DEAD-WIRED** (`publishTexture()` never called; never init()/setEnabled → publishes nothing)
- `src/output/SyphonInput.h/.mm` — macOS Syphon client — **ORPHANED** (never instantiated/wired)
- `src/output/SpoutOutput.h` — **STUB** (header-only, all no-op, never referenced)
- `src/output/NdiOutput.h`, `src/output/NdiInput.h` — **STUBS** (init returns false, TODO bodies, never referenced)

## UI (`src/ui/`, 37 .h/.cpp pairs = 74 files — key components only)
- `src/ui/LookAndFeel.h/cpp` — Dark VJ-style theme
- `src/ui/TopBar.h/cpp` — BPM display, beat wheel, transport controls (Play/Pause/Stop buttons are UNWIRED/decorative)
- `src/ui/PreviewPanel.h/cpp` — Center OpenGL preview
- `src/ui/OutputWindow.h/cpp` — Fullscreen output display (lives HERE, not src/output/)
- `src/ui/InspectorPanel.h/cpp` — 4-tab inspector (Clip/Layer/Composition/Signal)
- `src/ui/MappingEditor.h/cpp` — Source/curve/range/smoothing editor — **UNREACHABLE in v2** (only opened from the hidden EffectsRackPanel)
- `src/ui/SignalInspector.h/cpp` — Real-time signal display (envelope curve editor is paint-only, not draggable)
- `src/ui/PresetManager.h/cpp` — JSON save/load of effects + mappings + deck
- `src/ui/UniversalParamControl.h/cpp` — ResettableSlider (right-click reset) + param control + source picker
- Hidden/dead v1 panels (compiled, never shown in v2): `EffectsRackPanel`, `AudioReadoutPanel`, `SpectrumDisplay`, `ProgrammingMode`
- NOTE: no `KeyboardPanel`/`KeyEditor` files exist (CLAUDE.md source tree lists them — nonexistent)

## Build
- `CMakeLists.txt` — Root build: JUCE via FetchContent, C++20
- `cmake/CompilerWarnings.cmake` — Per-compiler warning flags
- `cmake/FindAubio.cmake` — Locate libaubio
- `cmake/FindFFmpeg.cmake` — Locate FFmpeg
- `cmake/FindProjectM.cmake` — Locate libprojectM-4 (optional, MilkDrop)
- `tests/CMakeLists.txt` — Catch2 test targets

## Tests
- `tests/test_*.cpp` — 11 Catch2 unit test targets, 113 tests (all PASS, ~1.7s)
- `tests/visual/` — "Eyes" pytest visual harness (~491 cases; needs running app + test-server build)
- `src/test/TestServer.h/cpp` — embedded HTTP test server ("Eyes", port 8080, 17 endpoints); compiled only under `AUDIODNA_BUILD_TEST_SERVER=ON` + `--test-mode` (OFF by default)

## Shaders
- `shaders/*.frag`, `shaders/*.vert` — 5 disk files (hue_shift/rgb_split/ripple/vignette/passthrough) — **DEAD duplicates** of embedded versions; hot-reload is inert (all shipped shaders compile from embedded strings, not files)
- `src/render/EmbeddedShaders.h` — All shaders as inline C++ strings (the compiled, shipping versions)

## Research & Design (read-only reference)
- `research/INDEX.md` — Research document index (35 docs)
- `ARCHITECTURE_V2.md` — Current v2 system design specification
- `PHASE_GUIDE.md` — Phase status tracker
- `design/mockups/` — UI design mockups
- `design/ux_analyses/` — Resolume/competitor UX analysis
