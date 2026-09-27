# Architecture Reference

> Moved from CLAUDE.md (claudemd-split). Core data structures, tech stack, source tree, threading/debugging notes.

---

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
| `trackerState` | `uint8_t` | 0-2 | BPM lock state: 0=searching, 1=locking, 2=locked |
| `beatInBar` | `uint8_t` | 0-3 | Which beat in the bar (0=downbeat) |
| `barPhase` | `float` | [0, 1) | Bar-level sawtooth over 4 beats |
| `downbeatDetected` | `bool` | level | Held true for the WHOLE first beat of the bar (assigned at beat events, never cleared per hop -- not a one-hop pulse); its rising edge is what totalBarCount counts |
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
| `detectedGenre` | `uint8_t` | 0-7 | Genre classification (P23) |
| `genreConfidence` | `float` | [0, 1] | How dominant the top genre is |
| `energyState` | `uint8_t` | 0-2 | 0=low, 1=medium, 2=high energy |
| `genreScores[8]` | `float[8]` | normalized | Smoothed scores for all 8 genres |
| `sidechainPump` | `float` | [0, 1] | Bass/mid anti-correlation (sidechain compression) |
| `swingRatio` | `float` | [0.5, ~0.67] | 0.5=straight, >0.5=swung timing |
| `formantPresence` | `float` | [0, 1] | Vocal formant energy concentration (300-3000 Hz) |
| `resonancePeak` | `float` | [0, 1] | Spectral kurtosis (sharp resonance peaks) |
| `reeseBass` | `float` | [0, 1] | Bass spectral spread (reese/wobble detection) |
| `sourceSampleRate` | `float` | Hz, 0=unknown | R13 provenance: the DEVICE rate analysis was actually fed from (0 in test mode/no device). Analysis itself always runs at the fixed internal `AnalysisThread::kSampleRate` (48 kHz) — `AnalysisResampler` bridges the two |
| `bandValidMask` | `uint8_t` | bitmask, bit b = `bandEnergies[b]` | R13: bit set when that band is meaningful at the source rate; bands mostly above the device Nyquist read 0 with the bit clear (e.g. 16 kHz Bluetooth HFP clears bit 6, Brilliance) |

**Mapping** — Routes any audio feature to any effect parameter:

| Field | Type | Purpose |
|-------|------|---------|
| `source` | `Source` enum | Which audio feature (RMS, BeatPhase, BarPhase, PhrasePhase, BarCount, Bass, MFCC0, etc.) |
| `targetEffectId` | `uint32_t` | Which effect in the chain |
| `targetParamIndex` | `uint32_t` | Which parameter on that effect |
| `curve` | `Curve` enum | Linear, Exponential, Logarithmic, SCurve, Stepped |
| `inputMin/inputMax` | `float` | Source normalization range |
| `outputMin/outputMax` | `float` | Target output range (default [0, 1]) |
| `smoothing` | `float` | EMA alpha |
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

---

### Lock-Free Communication Chain

```
Audio Callback ──SPSC Ring Buffer (16384 floats, ~341ms @ 48kHz)──▶ Analysis Thread
Analysis Thread ──Triple-Buffer Atomic Swap (3× FeatureSnapshot)──▶ Render Thread
UI Thread ──std::atomic<T> config variables──▶ Analysis/Render Threads
Render Thread ──juce::MessageManager::callAsync()──▶ UI Thread
```

All data flows forward. No backward dependencies on the hot path.

---

## Technology Stack

| Library | Version | License | What It Owns | Why Chosen Over Alternatives | Configured In |
|---------|---------|---------|-------------|---------------------------|---------------|
| **JUCE** | 8.0.4 | GPLv3 | Audio I/O, file playback (WAV/AIFF/FLAC/MP3/OGG), windowing, OpenGL context, UI widgets, message thread | Single framework for audio + UI + OpenGL. `AudioTransportSource` for file playback with background disk I/O. `AudioDeviceManager` for device enum/hot-plug across CoreAudio/WASAPI/ALSA/JACK. Alternatives: SDL2+ImGui (no audio file playback), Qt (poor RT audio). | `CMakeLists.txt` line 16-22, FetchContent |
| **Aubio** | 0.4.9+ | GPLv3 | BPM tracking (`aubio_tempo`), onset detection (`aubio_onset`), pitch detection (`aubio_pitch`) | Battle-tested beat/onset algorithms that beat custom implementations. Small C footprint. Alternative: Essentia (AGPL, massive dependency tree including FFTW/TagLib/yaml-cpp). | `CMakeLists.txt` (to be added in M2) |
| **juce::dsp::FFT** | (bundled) | GPLv3 | 2048-point FFT, magnitude spectrum (1025 bins) | Uses vDSP on macOS, IPP if available. No extra dependency. Adequate for 2048-pt. Alternatives: FFTW (GPL, overkill at 2048), KissFFT (slower). | JUCE module `juce_dsp` |
| **OpenGL 4.1 Core** | 4.1 | — | All image effects rendering via GLSL fragment shaders | macOS caps at 4.1 (Apple deprecated GL). Sufficient for 2D image effects on fullscreen quads. No compute shaders (require 4.3). Alternatives: Vulkan (overkill for 2D), Metal (macOS-only). | JUCE module `juce_opengl` |
| **GLSL 410** | 410 | — | All effect shaders (shipped set is embedded in `EmbeddedShaders.h`; hot-reload is inert — no disk shader files) | Matches OpenGL 4.1 target. | `src/render/EmbeddedShaders.h` (shaders/ dir removed Wave 0) |
| **Catch2** | 3.x | BSL-1.0 | Unit/integration tests | Header-only, BDD-style, integrates with CMake/CTest. Test-only dependency. | `tests/CMakeLists.txt` (to be added in M2) |
| **stb_image** | latest | Public domain | Fallback image loading for formats JUCE doesn't handle | Single header. JUCE handles PNG/JPEG/GIF natively. | `third_party/` (optional) |
| **CMake** | 3.24+ | — | Build system | JUCE 7+ has first-class CMake support (`juce_add_gui_app`). Industry standard. Alternative: Projucer (deprecated). | `CMakeLists.txt` |

| **FFmpeg** | 8.0 | LGPL/GPL | Video decode AND encode: MP4, MOV, QuickTime, AVI, MKV, WebM, M4V, HAP Alpha (libavformat, libavcodec, libavutil, libswscale). P22: Also used for real-time video recording (H.264/ProRes/MJPEG encoding). | Industry standard video codec. Supports all major codecs including H.264, H.265, ProRes, HAP Alpha. Alternative: GStreamer (heavier, less portable). | `cmake/FindFFmpeg.cmake`, `CMakeLists.txt` |

| **cpp-httplib** | 0.57.1 | MIT | HTTP server for production REST API (port 7070) and Eyes test server (port 8080) | Single-header C++ HTTP library. Always linked (promoted from test-only in P22). | `CMakeLists.txt` FetchContent |
| **juce_osc** | (bundled) | GPLv3 | OSC message receiving for external control (TouchOSC, Max/MSP, etc.) | JUCE built-in OSC module. | JUCE module `juce_osc` |
| **Syphon** | latest | BSD | macOS inter-app GPU texture sharing (zero-copy via IOSurface). Optional. | Enables sending/receiving textures to/from MadMapper, VDMX, OBS. Requires Syphon.framework in /Library/Frameworks/. | `CMakeLists.txt`, `AUDIODNA_BUILD_SYPHON` option |

**Total runtime dependencies: 4 (JUCE, Aubio, FFmpeg, cpp-httplib). Test-only: 1 (Catch2). Aubio's only transitive dependency is the C math library. JUCE bundles its own deps (freetype, zlib). FFmpeg is located via Homebrew on macOS.**

---

## Source Tree

```
AudioDNA/
├── CLAUDE.md                            ← YOU ARE HERE
├── ARCHITECTURE_V2.md                   # Current v2 system design specification
├── docs/archive/v1/ARCHITECTURE_V1.md  # [ARCHIVED] v1 keyboard launcher design
├── docs/archive/v1/TASKPLAN_V1.md      # [ARCHIVED] v1 milestone plan
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
│   │   ├── PitchTracker.h/cpp        ✅ # Aubio yinfft pitch detection
│   │   ├── GenreDetector.h/cpp       ✅ # [P23] 8-genre real-time classification from audio features
│   │   ├── GenreSmoothing.h             # REMOVED 2026-07-17 (Wave 0) — was dead (never instantiated)
│   │   └── AdvancedAudioAnalyzer.h/cpp ✅ # [P25] Sidechain pump, swing, formant, resonance, reese bass
│   ├── features/
│   │   ├── FeatureBus.h/cpp          ✅ # Triple-buffer atomic swap (3× FeatureSnapshot)
│   │   └── Smoother.h               ✅ # EMA smoother (header-only; One-Euro variant removed Wave 0)
│   ├── mapping/
│   │   ├── MappingEngine.h/cpp          # [M4] Source→curve→scale→target routing (58 sources, 24 curves)
│   │   ├── MappingTypes.h               # [M4] Mapping, Source, Curve enums
│   │   ├── CurveTransforms.h            # [M4] lin/exp/log/sigmoid/step + 19 P24 easings (pure fns)
│   │   └── MappingSuggester.h/cpp       # REMOVED 2026-07-17 (Wave 0) — was ghost (never instantiated, no UI/API caller)
│   ├── model/                           # [v2] Core data model
│   │   ├── Clip.h/cpp                    # Media + per-clip effects/transport/transform/cuepoints/autopilot
│   │   ├── Layer.h/cpp                   # Row of columns: type, opacity, blend/keying, layer effects, feedback
│   │   ├── Deck.h                        # Grid of layers × columns; one active at a time
│   │   ├── Composition.h                 # Top container: decks, master, crossfader, per-type/smart autopilot
│   │   └── Autopilot.h/cpp               # Beat / end-of-video / per-type / smart-energy clip advancement
│   ├── signal/                          # [v2] Signal system (feeds RoutingEngine)
│   │   ├── Signal.h + AudioSignal/OscillatorSignal/EnvelopeSignal/ClipPositionSignal.h  # Concrete signal types
│   │   ├── ChainedSignal.h/cpp           # REMOVED 2026-07-17 (Wave 0) — was ghost (never instantiated); SignalRegistry wiring removed
│   │   └── SignalRegistry.h/cpp          # 32 default signals (29 audio + 3 modulation), per-frame cache
│   ├── routing/                         # [v2] Universal signal routing (wired into render loop)
│   │   ├── Route.h + RoutingEngine.h/cpp # dial-range → threshold → gain → invert → smooth → ParamWriter
│   │   └── MacroBank.h                    # Dashboard links (8 macros/scope) — only the Global bank is instantiated (8 live, not 24)
│   ├── binding/                         # [v2] Keyboard + MIDI-learn bindings
│   │   ├── Binding.h                     # 21 actions (TriggerRoutine appended s-rta-0926 routines slice 1), 3 target modes, Toggle/Momentary, Abs/Rel CC
│   │   └── BindingManager.h/cpp          # id-keyed store, MIDI-learn capture, JSON presets
│   ├── midi/
│   │   ├── MidiHandler.h/cpp             # MIDI input, hot-plug → BindingManager (wired)
│   │   └── MidiOutputHandler.h/cpp       # Launchpad/APC pad-state feedback, change-diffed (wired)
│   ├── sync/
│   │   └── LinkSync.h/cpp                # Ableton Link wrapper — AUDIODNA_BUILD_LINK OFF by default → compiled no-op
│   ├── core/                            # [v2] Undo/redo scaffold
│   │   ├── Command.h                     # Abstract command base — DEAD: zero concrete subclasses
│   │   └── UndoManager.h/cpp             # History stack — DEAD: perform() never called; undo/redo keys are no-ops
│   ├── sources/                         # [P10+] Procedural sources
│   │   ├── SourceRegistry.h/cpp          # 108 sources across 18 categories, 759 params (ground truth)
│   │   ├── ProceduralSource.h/cpp        # Shader-backed source w/ ping-pong FBOs for stateful sims
│   │   └── ProjectMSource + ProjectMPresetManager + PresetSelector.h/cpp  # MilkDrop (build-conditional on libprojectM-4)
│   ├── media/
│   │   ├── VideoPlayer.h/cpp         ✅ # [P11] FFmpeg video decode (H.264/H.265/ProRes/HAP Alpha; container per linked FFmpeg) → GL texture
│   │   └── ImageSequence.h/cpp       ✅ # [P11] Multi-image playback as video clip with configurable FPS
│   ├── recording/                      # SessionRecorder REMOVED (s168, 3736f02) -- replaced by the performance take recorder below
│   │   ├── AudioTap.h/cpp            ✅ # Second fan-out in CombinedCallback; writes take audio, re-patches the WAV header every 10s (kHeaderFlushSeconds)
│   │   ├── AudioStore.h/cpp          ✅ # [Ruling 28] Shared audio store (~/Documents/Audio-DNA/Audio/<id>.adna-audio/) -- take format v3 references it by id; see "Audio Store" in docs/claude/recording.md
│   │   ├── RecorderClock.h/cpp       ✅ # Monotonic beat/sample/wall timebase shared by capture and replay
│   │   ├── PerformanceRecorder.h/cpp ✅ # touch/set/release gesture capture -> Lane
│   │   ├── Take.h/cpp + Lane.h + TempoMap.h + PerfState.h/PerfStateCapture.cpp ✅ # v3 take envelope (lanes/tempoMap/checkpoint0[+audio][+markers]), per-control lanes, tempo map, restore-point snapshot
│   │   ├── Program.h/cpp + Player.h/cpp ✅ # Compile a Take into a schedule (incl. the D4 preamble) and play it back
│   │   ├── RecorderHost.h/cpp        ✅ # Owns the whole record/replay lifecycle; backs RecordPanel + `/api/perf/*`
│   │   ├── RoutineSlice.h/cpp        ✅ # [s-rta-0926 routines slice 1] sliceRoutine(): cuts a take's lanes into a Routine (rebase to beat 0, straddling breakpoints, preamble fallback chain)
│   │   ├── RoutineEngine.h/cpp       ✅ # [s-rta-0926 routines slice 1] Runs each fired routine as its own Player on a shared beat clock; next-bar start, loop/once, gesture-begin stacking arbitration -- see "Routines" in docs/claude/recording.md
│   │   └── VideoRecorder.h/cpp       ✅ # [P22] Real-time video recording (FFmpeg H.264/ProRes/MJPEG, triple-buffered GL readback)
│   ├── model/
│   │   ├── ControlPath.h                # Deck/layer/clip/comp addressing shared by connections and recorded lanes
│   │   └── Routine.h                    # [s-rta-0926 routines slice 1] Composition-owned lane set + preamble + settings (loop/restoreState/quantize); Composition gains routines[]/routineBank[8]
│   ├── connect/
│   │   └── AutomationCurve.h            # Shared curve struct/evaluator: a hand-drawn Envelope (D6) and a recorded lane Gesture (D7) both store one
│   ├── api/
│   │   └── ApiServer.h/cpp           ✅ # [P22] Production REST API (port 7070, 41 registered routes -- all functional; 27 core + 7 `/api/perf/*` + `/api/audio/source` + 6 `/api/routine/*` [s-rta-0926 routines slice 1]; /api/set_bpm wired Wave 0; CORS, always-on)
│   ├── osc/
│   │   └── OscHandler.h/cpp             # [P22] OSC input receiver — LIVE 2026-07-17 (Wave 1-B): startListening(8000) at startup; 14/14 callbacks wired (`/audiodna/routine/{slot}` added s-rta-0926 routines slice 1)
│   ├── output/
│   │   ├── OutputManager.h/.cpp     ✅ # s-rta-0927 outputs-c2: the output windows, one per display; the Output menu / TopBar "Outputs" item list
│   │   ├── OutputMenuModel.h        ✅ # s-rta-0927 outputs-c2: pure -- buildOutputMenu, the button text, classifyOutputKey (unit-tested)
│   │   └── SyphonOutput.h/.mm       ✅ # [P22] macOS Syphon server — WIRED 2026-07-17 (Wave 1-A): publishes final composited frame each frame; no-op unless built -DAUDIODNA_BUILD_SYPHON=ON + Syphon.framework
│   │                                    #   (SyphonInput.h/.mm, SpoutOutput.h, NdiOutput.h, NdiInput.h REMOVED 2026-07-17 (Wave 0) — were orphaned/no-op stubs)
│   ├── test/                           # Build-gated (AUDIODNA_BUILD_TEST_SERVER=ON)
│   │   └── TestServer.h/cpp             # "Eyes" HTTP test server (port 8080, 17 endpoints) — OFF in default build
│   ├── effects/
│   │   ├── EffectLibrary.h/cpp          # [M4] Registry: 135 effects / 11 categories / 333 params
│   │   ├── Effect.h/cpp              ✅ # Single effect: shader program + param list
│   │   ├── EffectChain.h/cpp         ✅ # Ordered chain with ping-pong FBOs
│   │   ├── UniformBridge.h/cpp          # REMOVED 2026-07-17 (Wave 0) — was dead (superseded by MappingEngine, no caller)
│   │   └── ISFShaderLoader.h/cpp        # [P23] ISF parse/convert works, but MainComponent never compiles the GLSL → imported effects DON'T render
│   ├── render/
│   │   ├── Renderer.h/cpp            ✅ # OpenGLRenderer impl, GL 4.1 core, frame loop; compiles 275 embedded programs at startup
│   │   ├── ShaderManager.h/cpp       ✅ # Compile/link; hot-reload only for file-compiled shaders → INERT (all shipped shaders are embedded)
│   │   ├── TextureManager.h/cpp      ✅ # Image → GL_TEXTURE_2D, FBO textures
│   │   ├── FullscreenQuad.h/cpp      ✅ # VAO/VBO for fullscreen triangle
│   │   ├── FeedbackProcessor.h/cpp   ✅ # Per-layer Larsen feedback (6 presets)
│   │   ├── LUTLoader.h/cpp           ✅ # Loads color LUT images (Color Grade effect)
│   │   ├── EmbeddedShaders.h         ✅ # 135 effect + 15 transition + 93 source shaders (244 embedded strings)
│   │   ├── ScratchPool.h             ✅ # [s-rta-0926 xfade] pickEffectTarget(readTex, holdTex): the one rule for CompositorEngine's shared effect scratch FBOs -- never render into a texture a pass samples or its caller still holds
│   │   └── CompositorEngine.h/cpp    ✅ # [v2] Deck/layer compositing: clip FX → transition → layer FX → transform → keying → blend
│   └── ui/                              # [v2] Performance UI (~37 panels — actual files on disk, grouped by area)
│       ├── TopBar, SignalBar, SignalStrip           # Top chrome: tempo/transport + audio-feature meter strips
│       ├── DeckView, LayerStrip, ClipCell           # Resolume-style layer × column deck grid
│       ├── InspectorPanel + Clip/Layer/Composition/Signal Inspector  # 4-tab inspector
│       ├── BrowserPanel + Files/FX/Sources/CompDecks/MilkDrop browsers + RecordPanel  # Browser tabs
│       ├── PreviewPanel, OutputWindow               # Center preview + the Output window (presents the shared canvas frames, never key)
│       ├── EffectsRackPanel, EffectStackView, UniversalParamControl, Knob, MacroPanel, MappingEditor  # FX + param controls
│       ├── BindingOverlay, MidiLearnOverlay           # Bind-mode + MIDI-learn overlays (ProgrammingMode removed Wave 0)
│       ├── AudioReadoutPanel, WaveformDisplay, SpectrumDisplay, TimingWindow  # Audio readouts
│       ├── MenuBarModel, PreferencesDialog, PresetManager  # Menus, preferences, preset save/load
│       └── LookAndFeel                              # Dark VJ theme
├── shaders/                             # REMOVED 2026-07-17 (Wave 0) — dir deleted; was 5 dead duplicate disk files (hue_shift/rgb_split/ripple/vignette .frag + passthrough.vert). All shipped shaders are embedded strings in src/render/EmbeddedShaders.h
├── tests/                               # 53 Catch2 unit targets (565 tests registered -- `ctest -N` count, s-rta-0926 docs pass; NOT executed by this pass, see .harmony/VALIDATION.md; run: `cd build && ctest`; counts derived, not inherited -- see tests/CMakeLists.txt)
│   ├── CMakeLists.txt
│   ├── test_ring_buffer.cpp          ✅ # Lock-free SPSC ring buffer
│   ├── test_spectral_features.cpp    ✅ # Centroid/flux/flatness/rolloff/bands
│   ├── test_feature_bus.cpp          ✅ # Triple-buffer atomic swap
│   ├── test_smoother.cpp             ✅ # EMA smoother
│   ├── test_integration_pipeline.cpp ✅ # Full analysis pipeline on synthetic audio
│   ├── test_mapping_engine.cpp       ✅ # Source→curve→scale→target mapping
│   ├── test_bpm_stabilization.cpp    ✅ # BPM lock/stabilization
│   ├── test_downbeat_detector.cpp    ✅ # Downbeat detection
│   ├── test_composition.cpp          ✅ # Data model + serialization roundtrip/back-compat (Wave 1-C)
│   ├── test_routing_engine.cpp       ✅ # Signals + routing engine
│   ├── test_compositor.cpp           ✅ # Deck/layer compositing + autopilot
│   ├── test_waveform_snapshot.cpp   ✅ # Seqlock waveform snapshot (torn-read regression, Wave 1-D)
│   ├── test_take.cpp                 ✅ # [recorder] Take v3 envelope + Program/Player schedule + playback
│   ├── test_audio_tap_sync.cpp       ✅ # [recorder] AudioTap stop()/push() concurrency + T2 timing
│   ├── test_audio_store.cpp          ✅ # [recorder] AudioStore fingerprint/resolve/abandon (Ruling 28)
│   ├── test_recorder_host.cpp        ✅ # [recorder] RecorderHost record/replay lifecycle, preamble restore
│   ├── test_program_stamps.cpp       ✅ # [recorder] continuous breakpoint x from its own gesture's Stamp
│   ├── test_program_preamble.cpp     ✅ # [recorder] checkpoint0 -> Program preamble compile order (s-rta-0925)
│   ├── test_routine.cpp              ✅ # [recorder, s-rta-0926 routines slice 1] Routine model round-trip + sliceRoutine + compileRoutine (lane 1a)
│   ├── test_routine_engine.cpp       ✅ # [recorder, s-rta-0926 routines slice 1] RoutineEngine scheduling, loop/stop, gesture-begin stacking, quantize parity, Binding round-trip (lane 1b)
│   ├── test_take_v1_transport.cpp    ✅ # [recorder] removed-SessionRecorder-shape (v1) -> v2 bridge (D12 rule 5)
│   ├── test_recorder_double_touch.cpp ✅ # [recorder] PerformanceRecorder::touch() double-open guard
│   ├── test_record_panel_model.cpp   ✅ # [recorder] Record panel state machine
│   ├── test_onset_pulse.cpp          ✅ # OnsetPulse monotonic-counter delta (render-side onset consumers)
│   ├── test_bt_device_shapes.cpp     ✅ # Bluetooth-shaped device rate handling (no Bluetooth audio on the rig; shapes only)
│   ├── test_httplib_bodyless_post.cpp ✅ # cpp-httplib bodyless-POST regression (>= v0.28.0)
│   └── visual/                          # "Eyes" pytest harness (11 test_*.py) — needs running app + AUDIODNA_BUILD_TEST_SERVER build
├── resources/
│   ├── default_image.png                # Fallback test image
│   └── presets/
│       └── default_mappings.json        # Default mapping preset
└── research/                            # 35 research documents (+ INDEX.md, read-only reference)
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

### Debugging Audio Issues

1. Check the SPSC ring buffer fill level first — if it's consistently full or empty, the producer/consumer balance is wrong
2. R13: the ANALYSIS domain is always 48 kHz; the DEVICE/recorder domain is the device's own rate — never assume either is the other. `AnalysisResampler` bridges device rate → 48 kHz on the analysis thread (bypass when the device already is 48 kHz); the recorder/take/audio-store path stays entirely in the device domain (`FeatureSnapshot::sourceSampleRate` and `RecorderHost::Status::deviceRate`/`rateChangedSinceArm` publish which domain you are looking at)
3. Check thread priority — if analysis can't keep up, features lag behind audio

---

### Threading Deep-Dives

Before refactoring any threading code, read these research documents first:
- `research/ARCH_realtime_constraints.md` — golden rules of RT audio
- `research/ARCH_pipeline.md` — lock-free communication chain details

---
