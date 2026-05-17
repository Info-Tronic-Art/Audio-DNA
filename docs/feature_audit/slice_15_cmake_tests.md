# Slice 15: CMake Build Flags + Test Infrastructure + Eyes API

**Scope**: CMake options, dependency discovery, warning flags, tests/visual/ Eyes HTTP API, Python vj_controller client, shader/signal verification system, pytest fixtures.

**Primary files audited**:
- `/Users/boriskarpman/Documents/RealTimeAudio/CMakeLists.txt` (425 lines)
- `/Users/boriskarpman/Documents/RealTimeAudio/cmake/CompilerWarnings.cmake` (45 lines)
- `/Users/boriskarpman/Documents/RealTimeAudio/cmake/FindAubio.cmake` (45 lines)
- `/Users/boriskarpman/Documents/RealTimeAudio/cmake/FindFFmpeg.cmake` (45 lines)
- `/Users/boriskarpman/Documents/RealTimeAudio/tests/CMakeLists.txt` (248 lines)
- `/Users/boriskarpman/Documents/RealTimeAudio/tests/visual/TESTING.md` (210 lines)
- `/Users/boriskarpman/Documents/RealTimeAudio/tests/visual/SHADER_VERIFICATION.md` (230 lines)
- `/Users/boriskarpman/Documents/RealTimeAudio/tests/visual/SIGNAL_TEST_SPEC.md` (169 lines)
- `/Users/boriskarpman/Documents/RealTimeAudio/tests/visual/vj_controller.py` (232 lines)
- `/Users/boriskarpman/Documents/RealTimeAudio/tests/visual/conftest.py` (81 lines)
- `/Users/boriskarpman/Documents/RealTimeAudio/src/test/TestServer.h` + `TestServer.cpp` (cross-reference)

---

## Section 9.11: Test Server / CI endpoints (TESTING.md + TestServer.cpp)

All endpoints are gated by `-DAUDIODNA_BUILD_TEST_SERVER=ON` and `--test-mode` CLI flag. Default port 8080 (overridable via `--test-port=N`). Registered in `src/test/TestServer.cpp:setupRoutes()`.

### Core render / state endpoints (TESTING.md:60-151, TestServer.cpp:90-151)

| Method | Endpoint | Purpose | File:Line |
|---|---|---|---|
| GET | `/api/health` | Returns `{status, gl_version, fps, effects_count}` | TESTING.md:64-70, TestServer.cpp:90 |
| POST | `/api/load_image` | Load image file into renderer (`{filepath}`) | TESTING.md:72-78, TestServer.cpp:94 |
| POST | `/api/set_effect` | Enable/disable effect + set params (`{name, enabled, params}`) | TESTING.md:80-94, TestServer.cpp:98 |
| POST | `/api/set_effect_chain` | Configure entire chain (`{effects:[{name,params}]}`) | TESTING.md:96-107, TestServer.cpp:102 |
| POST | `/api/inject_features` | Inject synthetic audio features (bypass analysis thread) | TESTING.md:109-127, TestServer.cpp:106 |
| POST | `/api/render_frame` | Deterministic single-frame capture to PNG (`{output_path, time, width, height}`) | TESTING.md:129-144, TestServer.cpp:110 |
| GET | `/api/state` | Full engine state: effects + params + FPS + deck info | TESTING.md:146-148, TestServer.cpp:114 |
| POST | `/api/reset` | Reset all state (effects, images, features, master level) | TESTING.md:150-151, TestServer.cpp:118 |

### Source endpoints (vj_controller.py, TestServer.cpp:122-134)

| Method | Endpoint | Purpose | File:Line |
|---|---|---|---|
| POST | `/api/load_source` | Load procedural source (`{source_type, params}`) | TestServer.cpp:122, vj_controller.py:173 |
| POST | `/api/update_source_params` | Update active source uniforms (`{params}`) | TestServer.cpp:126, vj_controller.py:181 |
| GET | `/api/sources` | Enumerate all registered sources + their params | TestServer.cpp:130, vj_controller.py:185 |
| POST | `/api/load_milkdrop_preset` | Load MilkDrop `.milk` preset (`{preset_path}`) | TestServer.cpp:134, test_milkdrop.py:20 |

### Signal / routing endpoints (SIGNAL_TEST_SPEC.md:20-56, TestServer.cpp:139-155)

| Method | Endpoint | Purpose | File:Line |
|---|---|---|---|
| GET | `/api/signals` | List all signals with cached values | TestServer.cpp:139, SIGNAL_TEST_SPEC.md:22-29 |
| POST | `/api/add_route` | Create signal→parameter route | TestServer.cpp:143, SIGNAL_TEST_SPEC.md:31-34 |
| POST | `/api/remove_route` | Remove route by ID | TestServer.cpp:147, SIGNAL_TEST_SPEC.md:36-37 |
| GET | `/api/routes` | Enumerate active routes with current output values | TestServer.cpp:151, SIGNAL_TEST_SPEC.md:40-47 |
| POST | `/api/set_macro` | Set macro knob value or source (`{scope, index, source_signal_id|manual_value}`) | TestServer.cpp:155, SIGNAL_TEST_SPEC.md:49-54 |

### Feature injection supported fields (TESTING.md:122-127)

Fields accepted by `/api/inject_features`:
- Amplitude: `rms`, `peak`, `rmsDB`, `lufs`, `dynamicRange`, `transientDensity`
- Spectral: `spectralCentroid`, `spectralFlux`, `spectralFlatness`, `spectralRolloff`
- Rhythm: `bpm`, `beatPhase`, `barPhase`, `phrasePhase`, `barCount`, `beatInBar`, `downbeatDetected`
- Structure: `structuralState`
- Pitch/harmony: `dominantPitch`, `pitchConfidence`, `detectedKey`, `keyIsMajor`, `harmonicChangeDetection`
- Onsets: `onsetDetected`, `onsetStrength`
- Arrays: `bandEnergies[7]`, `chromagram[12]`, `mfccs[13]`

### Render frame determinism (TESTING.md:142-143)

- `time` param overrides `u_time` uniform (deterministic rendering).
- `width`/`height` temporarily override locked resolution. Max 1920x1080.

---

## Section 9.12: Python vj_controller methods

`VJAppController` class in `tests/visual/vj_controller.py`. All methods wrap the Eyes REST API into a Python client used by pytest.

### Lifecycle (vj_controller.py:24-80)

| Method | Purpose | Line |
|---|---|---|
| `__init__(port=8080, executable=None)` | Construct controller with base URL | vj_controller.py:18 |
| `start(timeout=20.0)` | Spawn Audio-DNA subprocess (`--test-mode --test-port=N`), wait for `/api/health` | vj_controller.py:24-63 |
| `stop()` | SIGTERM the subprocess, fall back to kill after 5s | vj_controller.py:65-74 |
| `health() -> dict` | Return `/api/health` JSON (status, gl_version, fps, effects_count) | vj_controller.py:76-80 |

### Media / effects (vj_controller.py:82-114)

| Method | Wraps | Line |
|---|---|---|
| `load_image(filepath)` | POST `/api/load_image` | vj_controller.py:82-88 |
| `set_effect(name, enabled=True, params=None)` | POST `/api/set_effect` | vj_controller.py:90-103 |
| `set_effect_chain(effects: list)` | POST `/api/set_effect_chain` — disables all then enables listed | vj_controller.py:105-114 |

### Audio / feature injection (vj_controller.py:116-123)

| Method | Wraps | Line |
|---|---|---|
| `inject_features(features: dict)` | POST `/api/inject_features` | vj_controller.py:116-123 |

### Rendering / state (vj_controller.py:125-161)

| Method | Wraps | Line |
|---|---|---|
| `render_frame(output_path, time_val=0.0, width=0, height=0)` | POST `/api/render_frame` | vj_controller.py:125-151 |
| `state() -> dict` | GET `/api/state` | vj_controller.py:153-157 |
| `reset() -> dict` | POST `/api/reset` | vj_controller.py:159-161 |

### Sources (vj_controller.py:163-187)

| Method | Wraps | Line |
|---|---|---|
| `load_source(source_type, params=None)` | POST `/api/load_source` | vj_controller.py:163-173 |
| `update_source_params(params: dict)` | POST `/api/update_source_params` | vj_controller.py:175-181 |
| `list_sources() -> dict` | GET `/api/sources` | vj_controller.py:183-187 |

### Signals / routes / macros (P16) (vj_controller.py:189-226)

| Method | Wraps | Line |
|---|---|---|
| `list_signals() -> dict` | GET `/api/signals` | vj_controller.py:191-195 |
| `add_route(route: dict) -> dict` | POST `/api/add_route` — body has source_signal_id, target_effect, target_param, output_min, output_max, threshold, gain, inverted | vj_controller.py:197-204 |
| `remove_route(route_id: int)` | POST `/api/remove_route` (`{id}`) | vj_controller.py:206-208 |
| `list_routes() -> dict` | GET `/api/routes` | vj_controller.py:210-214 |
| `set_macro(scope, index, **kwargs)` | POST `/api/set_macro` (scope="global"/"layer"/"clip", index 0-7, source_signal_id= or manual_value=) | vj_controller.py:216-226 |

### Internal helpers (vj_controller.py:228-232)

- `_post(path, data)` — JSON POST with 15s timeout, raises on HTTP error

**Total Python client methods**: 17 public + 1 private helper.

---

## CMake build flags

### Required toolchain (CMakeLists.txt:1-11)

- `cmake_minimum_required(VERSION 3.24)` — CMakeLists.txt:1
- `project(AudioDNA VERSION 0.1.0 LANGUAGES C CXX OBJC OBJCXX)` — CMakeLists.txt:2 (Obj-C/Obj-C++ required for macOS Syphon .mm files)
- C++20 standard, no extensions, standard required — CMakeLists.txt:4-6
- `CMAKE_EXPORT_COMPILE_COMMANDS ON` — CMakeLists.txt:9 (IDE support)

### Project options (all `AUDIODNA_BUILD_*`)

| Option | Default | Gates | File:Line |
|---|---|---|---|
| `AUDIODNA_BUILD_LINK` | OFF | Ableton Link tempo sync via FetchContent (Link-3.1.2). Adds `AUDIODNA_HAS_LINK=1` + `LINK_PLATFORM_MACOSX=1` compile defs | CMakeLists.txt:35, 385-389 |
| `AUDIODNA_BUILD_TEST_SERVER` | OFF | Eyes HTTP test server (port 8080). Conditionally compiles `src/test/TestServer.h/cpp`. Defines `AUDIODNA_TEST_SERVER=1` | CMakeLists.txt:56, 324-329, 369-371 |
| `AUDIODNA_BUILD_SYPHON` | OFF (APPLE only) | macOS Syphon.framework inter-app GPU texture sharing. Defines `AUDIODNA_HAS_SYPHON=1` if framework located | CMakeLists.txt:60, 374-382 |

### Auto-detected compile-time flags

| Flag | Default | Logic | File:Line |
|---|---|---|---|
| `AUDIODNA_USE_CAMERA` / `AUDIODNA_HAS_CAMERA` / `JUCE_USE_CAMERA` | 1 on APPLE/WIN32, 0 on Linux | `if(APPLE OR WIN32)` | CMakeLists.txt:342-346, 352-353 |
| `AUDIODNA_HAS_PROJECTM` | set if ProjectM_FOUND | `find_package(ProjectM QUIET)` — libprojectM-4 for MilkDrop | CMakeLists.txt:21, 360-363 |
| `AUDIODNA_HAS_LINK` | set if AUDIODNA_BUILD_LINK | gated by option | CMakeLists.txt:386 |
| `AUDIODNA_HAS_SYPHON` | set if Syphon.framework found at /Library/Frameworks | `find_library(SYPHON_FRAMEWORK)` | CMakeLists.txt:375-378 |
| `AUDIODNA_TEST_SERVER` | set if AUDIODNA_BUILD_TEST_SERVER | gated by option | CMakeLists.txt:370 |

### Stub-only / not-implemented-on-platform flags

From CLAUDE.md architecture summary + CMakeLists.txt source list:
- `AUDIODNA_BUILD_NDI` — NOT defined as a CMake option. NDI headers are stubs in `src/output/NdiOutput.h` / `NdiInput.h`. No actual NDI SDK integration.
- `AUDIODNA_BUILD_SPOUT` — NOT defined as a CMake option. Spout is Windows-only stub in `src/output/SpoutOutput.h`. No Windows integration.

### Feature flag → feature mapping

| Flag | User-facing feature unlocked |
|---|---|
| `AUDIODNA_BUILD_LINK=ON` | Menu: enable Ableton Link tempo sync; BPM synchronizes across networked apps. Override BPM tracker via `LinkSync` class. |
| `AUDIODNA_BUILD_TEST_SERVER=ON` | `--test-mode` CLI flag activates; embedded HTTP server on port 8080 for Eyes/pytest automation; load MilkDrop presets via API |
| `AUDIODNA_BUILD_SYPHON=ON` + framework installed | Output > Syphon Out sends GL texture to other Syphon-aware apps (MadMapper/VDMX/OBS); Syphon In receives textures from other apps |
| `AUDIODNA_USE_CAMERA=1` (auto) | Camera as live video source; JUCE_USE_CAMERA links juce_video module |
| `ProjectM_FOUND` (auto) | MilkDrop preset browser + `ProjectMSource` (.milk preset rendering as procedural source) |

### Compile definitions (unconditional) (CMakeLists.txt:348-357)

- `_USE_MATH_DEFINES` — Windows math constants
- `JUCE_WEB_BROWSER=0` — drop CEF web browser module
- `JUCE_USE_CURL=0` — drop libcurl
- `JUCE_DISPLAY_SPLASH_SCREEN=0` — suppress JUCE splash
- `JUCE_APPLICATION_NAME_STRING` + `JUCE_APPLICATION_VERSION_STRING` — from JUCE target properties

### Permission prompts (CMakeLists.txt:64-73, juce_add_gui_app)

- `MICROPHONE_PERMISSION_ENABLED TRUE` + text: "Audio-DNA needs microphone access to analyze audio and drive visual effects."
- `CAMERA_PERMISSION_ENABLED TRUE` + text: "Audio-DNA needs camera access to use live video as a visual source."

### JUCE modules linked (CMakeLists.txt:391-408)

`juce_audio_basics`, `juce_audio_devices`, `juce_audio_formats`, `juce_audio_utils`, `juce_core`, `juce_dsp`, `juce_graphics`, `juce_gui_basics`, `juce_gui_extra`, `juce_opengl`, `juce_osc`, plus `juce_video` if camera enabled.

External targets: `Aubio::Aubio`, `FFmpeg::FFmpeg`, `httplib::httplib` (always), `ProjectM::ProjectM` (if found), Syphon framework (if opted in and found).

### Dependency locations

| Dep | Source | Version | Location | File:Line |
|---|---|---|---|---|
| JUCE | FetchContent (shallow clone) | 8.0.4 | github.com/juce-framework/JUCE | CMakeLists.txt:26-32 |
| cpp-httplib | FetchContent | v0.18.3 | github.com/yhirose/cpp-httplib | CMakeLists.txt:47-53 |
| Ableton Link | FetchContent (optional) | Link-3.1.2 | github.com/Ableton/link | CMakeLists.txt:37-44 |
| Catch2 (tests) | FetchContent | v3.7.1 | github.com/catchorg/Catch2 | tests/CMakeLists.txt:4-10 |
| Aubio | `find_package(Aubio)` → FindAubio.cmake | system | /opt/homebrew/{include,lib}, /usr/local, /usr | FindAubio.cmake:10-24 |
| FFmpeg | `find_package(FFmpeg)` → FindFFmpeg.cmake (libavformat/avcodec/avutil/swscale) | system | /opt/homebrew/opt/ffmpeg, /usr/local/opt/ffmpeg, /usr/local, /usr | FindFFmpeg.cmake:8-26 |
| projectM-4 | `find_package(ProjectM QUIET)` | optional | system | CMakeLists.txt:21 |
| Syphon.framework | `find_library(SYPHON_FRAMEWORK ... PATHS /Library/Frameworks)` | macOS only | /Library/Frameworks | CMakeLists.txt:375 |

### JUCE header warning suppression (CMakeLists.txt:418-424)

GCC/Clang only: `-Wno-old-style-cast -Wno-conversion -Wno-sign-conversion` on the AudioDNA target to silence JUCE headers.

---

## Compiler warnings (cmake/CompilerWarnings.cmake)

Function `set_project_warnings(target_name)` applied at CMakeLists.txt:411.

### MSVC (CompilerWarnings.cmake:4-10)

- `/W4` — max warning level
- Specific warnings promoted to errors/enabled: `/w14242 /w14254 /w14263 /w14265 /w14287 /we4289 /w14296 /w14311 /w14545 /w14546 /w14547 /w14549 /w14555 /w14619 /w14640 /w14826 /w14905 /w14906 /w14928`

### Clang (CompilerWarnings.cmake:12-26)

`-Wall -Wextra -Wpedantic -Wshadow -Wcast-align -Wunused -Woverloaded-virtual -Wnon-virtual-dtor -Wold-style-cast -Wconversion -Wsign-conversion -Wnull-dereference -Wdouble-promotion -Wformat=2 -Wimplicit-fallthrough`

### GCC (CompilerWarnings.cmake:28-35)

All of Clang's plus: `-Wmisleading-indentation -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wuseless-cast`

### Auto-detection (CompilerWarnings.cmake:37-43)

`if(MSVC)` / `elseif(CMAKE_CXX_COMPILER_ID MATCHES ".*Clang")` / `elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")`.

---

## Shader verification system (tests/visual/SHADER_VERIFICATION.md)

### 4-Tier protocol (SHADER_VERIFICATION.md:22-72)

| Tier | Purpose | Gate? | Command |
|---|---|---|---|
| **Tier 1**: Automated Param Sweep | 5-position sweep per param; non-black + has-effect + no-discontinuity | Pre-commit gate | `AUDIODNA_NO_SPAWN=1 pytest tests/visual/test_sources.py test_effects.py -v` |
| **Tier 2**: Range Quality Analysis | 11-position sweep → CSV reports; useful-range >70% | Tuning | `AUDIODNA_NO_SPAWN=1 pytest tests/visual/test_range_quality.py -v -k "source_name"` |
| **Tier 3**: Visual Preview (Claude) | Open `shader_preview.html`, move every slider end-to-end | Claude's visual check | Manual browser |
| **Tier 4**: In-App Validation | User tests in real app, final gate | User's validation | Manual launch |

### Tier 1 test targets (SHADER_VERIFICATION.md:13-21)

| Test file | What it covers | Count |
|---|---|---|
| `test_sources.py` | All procedural sources, auto-discovered via `/api/sources` | 55 (doc) / 81 current |
| `test_effects.py` | All effects, auto-discovered via `/api/state` | 112 (doc) / 135 current |
| `test_audio_reactivity.py` | ~20 audio feature injections → visual change | ~20 features |
| `test_signals.py` | 22+ signals/routes/macros (some stubbed) | — |
| `test_transitions.py` | 15 transitions (progress 0→1) — spec, may need new endpoint | 15 |
| `test_time_sweep.py` | Animated sources render differently at t=0/1/5/10 | all |
| `test_performance.py` | Render time per frame < 16ms (60fps) / 50ms budget with HTTP overhead | all |
| `test_range_quality.py` | 11-position sweep CSVs, 70%+ useful range | all |
| `test_fractals.py` | Fractal-specific param tests (hardcoded, ~185 tests) | 7 2D + 8 3D |
| `test_render_pipeline.py` | Core render/effect/feature sanity | — |
| `test_milkdrop.py` | MilkDrop preset scoring | 80 presets |

### Tier 1 pass criteria (SHADER_VERIFICATION.md:24-31)

- Not black: `mean(pixels) > 5` at every position
- Has effect: param change produces PSNR < 55 vs default
- No discontinuity: adjacent steps PSNR > 8 (no jump-cuts)

### Self-maintaining design (SHADER_VERIFICATION.md:156-168)

Tests discover what to test from the running app's API (`/api/sources`, `/api/state`). Adding a new effect/source to `SourceRegistry.cpp` or `EffectLibrary.cpp` automatically exposes it to the Eyes tests.

### Performance budget (SHADER_VERIFICATION.md:133-145, test_performance.py:76)

- 60fps target means <16ms/frame
- Test threshold is 50ms (includes HTTP overhead)

### Phase coverage matrix (SHADER_VERIFICATION.md:170-190)

- P16: time effects + feedback → `test_effects.py`, `test_time_sweep.py`
- P17-P19: creative/audio/complex sources + effects
- P20: Layer router, FFGL, text, simulations (needs compositor-level tests)
- P21: Live performance controls (needs interaction tests)

### Known gaps (SHADER_VERIFICATION.md:180-190)

1. Feedback system (P16) needs compositor test
2. Layer Router (P20) needs multi-layer API endpoint
3. FFGL plugins need host-level testing
4. Performance under load (multi-layer VJ scenario)
5. Transitions need progress API endpoint

---

## Signal test spec (tests/visual/SIGNAL_TEST_SPEC.md)

### Current state (SIGNAL_TEST_SPEC.md:5-18)

- SignalRegistry, RoutingEngine, MacroBank, Route, Signal types are built and unit-tested (test_routing_engine.cpp 15+ tests).
- Feature injection works — sources consume `u_rms`, `u_beatPhase` directly.
- Signal API endpoints (`/api/signals`, `/api/add_route`, `/api/remove_route`, `/api/routes`, `/api/set_macro`) ARE implemented per TestServer.cpp:139-155 (spec was authored before wire-up).

### Tier 1: Signal basics (unit tests, SIGNAL_TEST_SPEC.md:82-97)

All PASS in `test_routing_engine.cpp`:
- AudioSignal extracts RMS / bandEnergies
- OscillatorSignal 5 shapes (sine/sawUp/sawDown/triangle/square) at various phases
- SignalRegistry evaluateAll caches values
- Route with gain, threshold, invert, output range, EMA smoother, dial-range

### Tier 2: Feature injection → visual (test_signals.py, SIGNAL_TEST_SPEC.md:99-107)

RUNNABLE now. Feature pairs tested:
- RMS 0 vs 0.9 changes source output (u_rms)
- Beat phase 0 vs 0.5 changes output (u_beatPhase)
- Bass vs treble bandEnergies differ
- Spectral centroid low vs high
- Onset detected changes output
- RMS changes effect on image

Sources verified as audio-responsive (test_signals.py:80-83): `mandelbrot`, `julia_set`, `burning_ship`, `mandelbulb`, `audio_waveform`, `kifs`

### Tier 3: Signal→Route→Param→Shader (SIGNAL_TEST_SPEC.md:109-122)

Specification for full end-to-end tests once API endpoints exist:
- Create route Volume→Ripple.intensity
- High RMS → more ripple via route
- Threshold blocks low values
- Invert flips output
- Output range clamps
- Macro drives multiple params
- Oscillator creates time-varying output
- Signal registry has 14+ signals
- Signal values update after inject

### Tier 4: Complex routing (future, SIGNAL_TEST_SPEC.md:124-132)

- Two routes to same parameter sum
- Route to source param (not just effect)
- Macro chain: Signal→Macro→Route→Param (multi-hop)
- 20 simultaneous routes don't lag (performance)
- Route serialization roundtrip
- Route removal cleans up smoothers

### Expected signal minimum (test_signals.py:301-309)

Audio signals: `Volume`, `Sub Bass`, `Bass`, `Mid`, `Air`, `Tempo`, `Beat Position`, `Hit`, `Beat In Bar`, `Bar Position`, `Phrase Position`, `Energy State` (12 audio signals)

Modulation signals: `Mod 1`, `Mod 2` (2 oscillators)

---

## Test fixtures / env vars

### pytest fixtures (conftest.py)

| Fixture | Scope | Purpose | Line |
|---|---|---|---|
| `app` | session | Spawn Audio-DNA in test mode once per session; shared VJAppController | conftest.py:47-74 |
| `reset_between_tests` | autouse | Calls `app.reset()` before each test for isolation | conftest.py:77-81 |

### Environment variables (conftest.py:54-61, TESTING.md:196-200)

| Variable | Default | Effect | File:Line |
|---|---|---|---|
| `AUDIODNA_EXE` | auto-detected build path | Override executable path | conftest.py:59, TESTING.md:198 |
| `AUDIODNA_TEST_PORT` | `"8080"` | Override HTTP port | conftest.py:60, TESTING.md:199 |
| `AUDIODNA_NO_SPAWN` | `"0"` | Set to `"1"` to use already-running app (skip subprocess.Popen) | conftest.py:61, TESTING.md:200 |

### Auto-detected executable path (conftest.py:13-44)

1. macOS bundle: `build/AudioDNA_artefacts/Release/Audio-DNA.app/Contents/MacOS/Audio-DNA`
2. Linux/Windows: `build/AudioDNA_artefacts/Release/Audio-DNA`
3. If neither exists AND no-spawn is not set → `pytest.skip()` with message: "Audio-DNA executable not found. Build with -DAUDIODNA_BUILD_TEST_SERVER=ON"

### CLI arguments (TESTING.md:55-58, parsed in src/Main.cpp:20-22)

| Arg | Default | Effect |
|---|---|---|
| `--test-mode` | off | Enable the Eyes HTTP server |
| `--test-port=N` | 8080 | HTTP server port override |

### Default ports

- Eyes test server (conditional): 8080
- Production REST API server (always-on in P22+, port 7070 per CLAUDE.md): 7070

---

## C++ Catch2 unit tests (tests/CMakeLists.txt)

10 test executables registered via `catch_discover_tests`:

| Target | Coverage | File:Line |
|---|---|---|
| `test_ring_buffer` | SPSC ring buffer | tests/CMakeLists.txt:20-23 |
| `test_spectral_features` | Centroid/flux/flatness/rolloff/7-band | tests/CMakeLists.txt:26-32 |
| `test_feature_bus` | Triple-buffer atomic swap | tests/CMakeLists.txt:35-41 |
| `test_smoother` | EMA + One-Euro filter | tests/CMakeLists.txt:44-47 |
| `test_integration_pipeline` | Full analysis pipeline (FFT→features), links JUCE DSP + Aubio | tests/CMakeLists.txt:52-87 |
| `test_mapping_engine` | Source→curve→scale→target routing; links juce_opengl | tests/CMakeLists.txt:90-119 |
| `test_bpm_stabilization` | BPM range gate/confidence/octave/median/hysteresis | tests/CMakeLists.txt:122-138 |
| `test_downbeat_detector` | Automatic downbeat detection | tests/CMakeLists.txt:141-157 |
| `test_composition` | P3 data model + undo (Clip, Layer, UndoManager) | tests/CMakeLists.txt:160-185 |
| `test_routing_engine` | P3 signals + routing (SignalRegistry, ChainedSignal, RoutingEngine, MappingEngine) | tests/CMakeLists.txt:188-220 |
| `test_compositor` | P3 deck compositing + autopilot | tests/CMakeLists.txt:223-248 |

All test targets apply `-Wno-old-style-cast -Wno-conversion -Wno-sign-conversion` on non-MSVC (tests/CMakeLists.txt:80-86 etc.).

Catch2 module path: `list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)` — tests/CMakeLists.txt:12.

---

## Vision check tooling (referenced in TESTING.md:171-179)

`tests/visual/vision_check.py` (not read line-by-line, but documented):
- `verify_frame(rendered, golden, psnr_threshold=50.0, ssim_threshold=0.99)` → (passed, metrics)
- `compute_psnr(img1, img2)` — used by test_signals.py:42-46

Thresholds (TESTING.md:210): PSNR > 45 / SSIM > 0.98 for cross-machine GPU variance.

Golden frame workflow (TESTING.md:181-192): set up scene → capture → commit to `tests/visual/golden_frames/` → compare on future runs. Regenerate on shader source changes, GPU driver updates, or architecture moves.

Diff directory: `tests/visual/diffs/` for failure diff images.
Reports directory: `tests/visual/reports/` for range quality CSVs.

---

## Section 20 partial: Cross-references

### CMake options → features

- `AUDIODNA_BUILD_SYPHON` → Section 19 (Output/Syphon) — Syphon output/input
- `AUDIODNA_BUILD_LINK` → Section 2 (Audio analysis/BPM) — Ableton Link tempo sync
- `AUDIODNA_BUILD_TEST_SERVER` → Section 9.11 (this slice) — Eyes HTTP API
- `AUDIODNA_USE_CAMERA` (auto) → Section 11 (Media/Sources) — camera input
- `AUDIODNA_HAS_PROJECTM` (auto) → Section 11 (Sources) — MilkDrop visualizer source

### Python API methods ↔ REST endpoints

Every Python method in Section 9.12 maps 1:1 to a REST endpoint in Section 9.11.

### Test tiers ↔ phase validation

Per CLAUDE.md "kick off phase N" protocol step 4, Tier 1 (sources + effects + audio + time + performance) is the pre-commit gate. Tier 2 is for range tuning. Tier 3 is Claude's visual self-check. Tier 4 is the user's in-app validation.

### Signal test spec ↔ TestServer.cpp wiring

SIGNAL_TEST_SPEC.md was authored before the endpoints were wired into TestServer. Endpoints referenced there (`/api/signals`, `/api/add_route`, `/api/remove_route`, `/api/routes`, `/api/set_macro`) ARE now registered at TestServer.cpp:139-155.

### Catch2 tests ↔ source subsystems

- `test_ring_buffer`, `test_spectral_features`, `test_feature_bus`, `test_smoother`, `test_integration_pipeline` → Section 2 (Audio Analysis)
- `test_mapping_engine`, `test_routing_engine` → Section 9 (Signals/Routing)
- `test_bpm_stabilization`, `test_downbeat_detector` → Section 2 (BPM/rhythm)
- `test_composition`, `test_compositor` → Section 3 (Data model) + Section 7 (Rendering)

### Notable surprises

1. **httplib is always linked** (CMakeLists.txt:366) — the test server is conditional, but the dependency is unconditional because `ApiServer` (port 7070) uses it for the production REST API.
2. **Obj-C/Obj-C++ languages enabled in top-level `project()`** (CMakeLists.txt:2) — required even if AUDIODNA_BUILD_SYPHON is OFF, because Syphon .mm stubs compile as no-ops via `__has_include` when the framework is absent.
3. **NDI and Spout CMake options don't exist** — only headers in `src/output/NdiOutput.h`, `NdiInput.h`, `SpoutOutput.h` as not-yet-integrated stubs.
4. **Camera is platform-gated** — Linux auto-disables `juce_video` (no Linux camera support in JUCE).
5. **C++20** required — `set(CMAKE_CXX_STANDARD 20)` with `CMAKE_CXX_STANDARD_REQUIRED ON` (CMakeLists.txt:4-5).
