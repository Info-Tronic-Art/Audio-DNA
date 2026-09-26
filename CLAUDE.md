# Audio-DNA — Project Instructions for Claude

> Canonical counts + full surface/function inventory: `.harmony/APP-INVENTORY.md`
> (source of truth — this file references, does not restate).

---

## Project Identity

Audio-DNA is a cross-platform desktop application (C++20 / JUCE / OpenGL) for live audio-reactive visual performance. It analyzes audio in real-time (mic, system audio, or audio file) and applies 135 GLSL shader effects to images, driven by extracted audio features. It is a VJ-style performance tool where music controls visual transformations.

The core concept: audio analysis + visual effects + a mapping system + a keyboard clip launcher, rendered live at 60fps. Users load images (or folders for beat-synced slideshows), wire audio features to effect parameters via mappings with curves and smoothing, and perform live with keyboard-triggered visual scenes.

**Key capabilities**: 135 effects across 11 categories (6 temporal, 3 audio-native), 15 clip-to-clip transitions, per-layer feedback system (6 presets), deck/layer/clip compositing with per-level effect chains, fullscreen output to any connected display, beat-synced randomization, instant preset save/recall, camera input, video playback, 108 procedural sources across 18 registry categories (3D 24, Geometric 11, Lines 11, Audio-Visual 9, Math 8, Pattern 8, Fractal 7, Wireframe 7, Nature 6, Noise 3, Particle 3, Simulation 3, Text 2, Utility 2, Lighting 1, MilkDrop 1, Organic 1, Routing 1), per-type autopilot automation, signal routing engine wired into render loop, VJ panel UI, piano/momentary keyboard+MIDI mode, MIDI velocity-to-opacity, CC relative mode for endless encoders, 3 binding targeting modes (ByPosition/ThisItem/Selected), persistent layers across deck switches, Ableton Link tempo sync (optional, off by default), per-clip beat snap granularity, saved performance routines (fire a piece of a recorded take from an 8-slot bank, restore-then-replay on the next bar, loop/once), production REST API (port 7070), OSC input (UDP 8000), MIDI output for Launchpad/APC pad feedback, real-time video recording (FFmpeg H.264/ProRes/MJPEG), PNG snapshot capture, Syphon output (macOS, optional, build-flag-gated), real-time genre detection (8 genres), smart energy-aware autopilot, structural scene triggering, ISF shader import (phantom — doesn't render), smart BPM recovery during silence, advanced audio analysis (sidechain pump, swing ratio, formant tracking, resonance peaks, reese bass detection), composition-level transform (position/scale/rotation), cross-deck transitions with 3 blend modes.

**What this is NOT**: Not a DAW, not a video editor, not a web app, not a plugin. It is a standalone desktop application for live audio-reactive visual performance.

---

## Architecture Summary

### The 4-Thread Model

**Audio Callback (OS-managed, REAL-TIME priority)**
Runs every 2.67ms (128 samples @ 48kHz). Receives samples from JUCE's `AudioIODeviceCallback`, mono-downmixes them, and pushes into the SPSC ring buffer. This is the sacred thread — it must NEVER allocate heap memory, acquire mutexes, make system calls, or do any DSP. Just `memcpy` to ring buffer and return. Budget: <100μs. Communicates forward to the analysis thread via the SPSC ring buffer.

**Analysis Thread (App-managed, ABOVE-NORMAL priority)**
Runs every ~10.7ms (512-sample hop @ 48kHz). Pulls device-rate samples from the ring buffer and resamples them to the fixed internal 48 kHz (`AnalysisResampler`, R13) before anything else — a bit-identical bypass when the device already runs at 48 kHz. Maintains a 2048-sample overlap window, runs FFT, and extracts all audio features in a fixed pipeline order. Pre-allocates all buffers and Aubio objects at startup — zero allocation in steady state, including at a device rate change (the resampler reconfigures a fixed-size interpolator in O(1), no aubio object is ever re-created). Budget: <2ms per hop (5x headroom). Publishes a complete `FeatureSnapshot` to the Feature Bus via atomic triple-buffer swap.

**Render Thread (OpenGL, NORMAL priority, VSync)**
Runs every 16.67ms (60fps). Reads the latest `FeatureSnapshot` from the triple buffer (lock-free atomic read). Runs all active mappings (source → curve → scale → target), uploads uniforms to GPU, and renders the effect chain on a fullscreen quad with the loaded image texture. Uses ping-pong FBOs for multi-effect chains. Budget: <8ms for full chain. Communicates display values back to UI via `juce::MessageManager::callAsync()`.

**Message Thread (JUCE UI, NORMAL priority)**
Runs on user events. Handles all UI interaction — sliders, buttons, file choosers, mapping editor. Writes configuration changes (effect enable/disable, parameter values, mapping settings) via `std::atomic<T>` config variables that the render and analysis threads read. Never blocks the other threads.

### Latency Budget

| Stage | Operation | Latency |
|-------|-----------|---------|
| 1. Audio buffer delivery | OS delivers 128 samples @ 48kHz | 2.67ms (period) |
| 2. Ring buffer push | `memcpy` into SPSC | ~50ns |
| 3. Hop accumulation | Wait for 512 samples (1 hop) | 10.7ms (hop period) |
| 3b. Resample to 48 kHz (R13, non-48 kHz devices only) | `AnalysisResampler`: 5-tap Lagrange interpolation + anti-alias biquads when upsampling; bypass (0µs) when the device is already 48 kHz | ~20-60μs/hop |
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

## Build Essentials

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

### Common Build Issues

| Issue | Fix |
|-------|-----|
| `FetchContent` download fails | Check internet connection; JUCE repo is ~200MB |
| macOS: "OpenGL deprecated" warnings | Expected — Apple deprecated GL but 4.1 still works. Suppress with `-Wno-deprecated` |
| Linux: missing X11/ALSA headers | Install the `apt` packages listed in `docs/claude/build-other-platforms.md`'s Linux section |
| Windows: long path errors | Enable long paths: `git config --system core.longpaths true` |

---

## Development Rules

### Sacred Rules (violating these causes real-time audio failures)

1. **The audio callback is sacred**: No heap allocation (`new`, `malloc`, `vector::push_back`), no mutexes (`std::mutex`, `std::lock_guard`), no system calls (`printf`, file I/O, `std::cout`), no exceptions. It copies samples to the ring buffer and returns. Budget: <100μs.

2. **All inter-thread data goes through established lock-free channels**: Audio→Analysis via SPSC ring buffer. Analysis→Render via triple-buffer FeatureBus. UI→hot path via `std::atomic<T>`. Never add a new mutex without explicit discussion.

3. **Analysis thread pre-allocates everything**: All FFT plans, work buffers, Aubio objects, filter states created at startup. Zero allocation in the steady-state loop.

4. **Render thread never waits for analysis**: If no new snapshot is available, reuse the previous one. The render loop runs at VSync regardless of analysis rate.

### Shader Rules

5. **Every new effect is a GLSL file in `/shaders`**: Never hardcode effect logic in C++. One `.frag` file per effect. Shaders are hot-reloadable from disk. **NOTE (actual shipped model)**: the 135 shipped effects are embedded as inline strings in `src/render/EmbeddedShaders.h` and compiled at startup — the `shaders/` dir of legacy duplicate disk files was removed Wave 0. ShaderManager hot-reload only reloads file-compiled shaders, so it is inert for the shipped (embedded) set.

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

## UI Patterns (Mandatory for all new UI)

**ResettableSlider**: ALL sliders in the app MUST use `ResettableSlider` (defined in `UniversalParamControl.h`), not `juce::Slider`. This class overrides `mouseDown` to reset to default value on right-click. Every `ResettableSlider` MUST call `setDefaultValue(val)` at setup time. This applies to sliders in inspectors, top bar, layer strip, mapping editor, signal inspector, macro knobs — everywhere. Right-click also resets from the text box of IncDecButtons/TextBox sliders (nested-child relay, s-rta-0925); `UniversalParamControl` arms its inner slider with 0.5 by default, so owners that skip `setDefaultValue()` get a 0.5 reset rather than a dead click — still call it with the real default.

**Drag-drop targets**: Any inspector that displays an effect stack MUST implement `juce::DragAndDropTarget` with `isInterestedInDragSource`, `itemDragEnter` (set highlight + repaint), `itemDragExit` (clear highlight + repaint), `itemDropped` (forward to EffectStackView). The highlight is a cyan border + 15% alpha fill.

**Effect display name vs shader key**: `Clip::EffectSlot::effectName` stores the human-readable display name (e.g., "Ripple"). Shaders are compiled under snake_case keys (e.g., "ripple"). Always resolve via `EffectLibrary::getEffectDef(displayName)->shaderName` before calling `ShaderManager::getProgram()`. Never assume display name == shader key.

**Transport state**: `Clip::playing` is `mutable` (render thread writes it for OneShot stop). Retriggering the same clip preserves its play/pause state. Switching to a different clip starts playing only on first activation (`hasBeenTriggered` flag). PingPong and OneShot loop modes require propagating player state back to clip model after `advanceFrame()`.

**PopupMenu**: Always use `showMenuAsync()` with `.withParentComponent(getTopLevelComponent())` to ensure menus dismiss on app switch.

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
   - **Shader verification (MANDATORY if sources/effects/shaders changed)**: Follow the 4-tier system in `tests/visual/SHADER_VERIFICATION.md`:
     - **Tier 1 — Sources**: `AUDIODNA_NO_SPAWN=1 pytest tests/visual/test_sources.py -v` — auto-discovers ALL sources, sweeps every param, checks non-black + has-effect + no-discontinuity
     - **Tier 1 — Effects**: `AUDIODNA_NO_SPAWN=1 pytest tests/visual/test_effects.py -v` — auto-discovers ALL 112 effects, verifies each param changes output
     - **Tier 1 — Audio**: `AUDIODNA_NO_SPAWN=1 pytest tests/visual/test_audio_reactivity.py -v` — verifies injected audio features change source/effect output
     - **Tier 1 — Time**: `AUDIODNA_NO_SPAWN=1 pytest tests/visual/test_time_sweep.py -v` — verifies animated sources change over time
     - **Tier 1 — Performance**: `AUDIODNA_NO_SPAWN=1 pytest tests/visual/test_performance.py -v` — verifies render time within budget
     - **Tier 2**: `AUDIODNA_NO_SPAWN=1 pytest tests/visual/test_range_quality.py -v` — 11-position sweep, CSV reports, 70%+ useful range, no dead zones
     - **Tier 3**: Open `tests/visual/shader_preview.html` in browser, move every slider end-to-end
     - **Tier 4**: User tests in the actual app
     - Fix ALL Tier 1 failures before reporting. Tier 2 for tuning. Tier 3 is Claude's visual check. Tier 4 is user's.
   - **Quick run all visual tests**: `AUDIODNA_NO_SPAWN=1 pytest tests/visual/ -v --ignore=tests/visual/test_range_quality.py` (range quality is slow, run separately for tuning)
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
     - Add new entries to `docs/claude/pitfalls.md` (and its one-line index in CLAUDE.md) if bugs were discovered and fixed
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
- Note: `ARCHITECTURE.md` (v1 keyboard launcher) is archived at `docs/archive/v1/ARCHITECTURE_V1.md`. The current design spec is `ARCHITECTURE_V2.md`.
- Note: `TASKPLAN_V2.md` is archived at `docs/archive/TASKPLAN_V2.md` — all phases P1-P25 are complete.
- Read existing source files before modifying them

---

### Common Pitfalls Index

Full detail (verbatim) for every numbered pitfall lives in `docs/claude/pitfalls.md` --
numbers are stable and cited elsewhere as "Pitfall N". Read the full entry before touching
the named area; this index is triage-only.

1. Shader lookup mismatch -- before resolving an effect display name to a shader key.
2. Loop mode race condition -- before touching clip transport / play-state sync each frame.
3. FBO conflicts -- before adding a new FBO to the render/compositor pipeline.
4. Demo effects left enabled -- before touching `initEffectChain()`.
5. JUCE slider right-click -- before adding any new slider (must be `ResettableSlider`).
6. Unicode button text -- before adding button glyphs at small sizes.
7. Transport state on clip switch -- before touching `triggerClipImmediate()` / `hasBeenTriggered`.
8. Source param right-click reset -- before adding source param controls in ClipInspector.
9. Fractal zoom design -- before touching any fractal zoom/dive shader.
10. Fractal center vs location vs dive -- before wiring fractal center/location/dive controls.
11. 2D fractal power range -- before changing Mandelbrot/Julia power range.
12. 3D fractal zoom range -- before changing a 3D fractal's camera distance.
13. Temporal effects need TWO render paths -- before touching `u_prev_frame` / temporal effects.
14. EffectChain temporal save requires FBO rendering -- before touching `EffectChain::render()`'s last-effect path.
15. Layer ID 0 is valid -- before guarding any layerId-based feature.
16. Multi-select FX drag-drop -- before touching `EffectStackView::itemDropped()`.
17. Effect parameter defaults must be noticeable -- before setting a new effect's default param values.
18. Parameter range remapping for nonlinear perception -- before wiring a decay/fps-like parameter.
19. Effects needing frame history -- before adding a Screen-Split/Frame-Stutter-style effect.
20. Ring buffer VRAM budget -- before changing `FrameRingBuffer` resolution/depth.
21. Layer Router renders black without other layers -- before debugging Layer Router output.
22. Stateful simulation sources need continuous frames -- before testing ping-pong FBO sources in single-frame mode.
23. Per-type autopilot must be explicitly enabled -- before touching `PerTypeAutopilotConfig`.
24. httplib is always linked, not test-only -- before gating httplib behind a build flag.
25. VideoRecorder triple-buffer has no mutex on GL thread -- before touching `VideoRecorder::submitFrame()`.
26. Syphon uses `__has_include` for compile-time detection -- before changing Syphon build/detection logic.
27. Effect defaults must be visible on first add -- before setting any new effect's primary param default.
28. Eyes render_frame doesn't apply effect chain -- before relying on Eyes captures to verify effect rendering.
29. Two rate domains, never assume they are the same (R13) -- before comparing device-rate and analysis-rate sample counts.
30. Render-side onset consumers must act on the `onsetCount` delta -- before reading `onsetDetected` on the render side.
31. Bodyless POST must be answered immediately -- before touching the cpp-httplib version/config.
32. `downbeatDetected` is a beat-long LEVEL, not a pulse -- before reading `downbeatDetected` as an edge/pulse.
33. Effect/source-param rows are engine-driven -- before writing to `paramValues`/`sourceParams[].value` directly.
34. A JUCE `Component` is invisible by default -- before writing a headless visibility-gated widget test.

---

## Updating This Document

When you add a new feature, effect, or audio analysis capability, update the relevant section of this CLAUDE.md to reflect it. This document must always be the current truth.

---

## Trigger Table

When you are doing X, read the named doc (all under `docs/claude/`) before making changes --
these are NOT @-imported, so they cost nothing at boot and are read on demand.

| When you are doing X | Read |
|---|---|
| Touching `FeatureSnapshot`/`Mapping`/`Effect`/`Clip` struct fields, the lock-free communication chain, the technology-stack table, the source tree/file layout, naming a new file/class/method/uniform (Naming Conventions), or debugging audio-thread/threading issues | `docs/claude/architecture.md` |
| Adding/changing an audio analysis feature (amplitude, spectral, rhythm/onset, pitch/harmony, structural) or the 14-stage analysis pipeline order | `docs/claude/analysis.md` |
| Adding/changing a GLSL effect, a transition shader, the effect-chain architecture, FX drag-and-drop, the Autopilot System, Manual BPM Mode, or the Tooltip System | `docs/claude/effects.md` |
| Touching Mapping System internals (curve/scale/smooth pipeline, Master Signal), temporal/time effects (P16), the audio uniform system (P18), composition-level transform or cross-deck transitions (P25), or debugging visual/render issues | `docs/claude/rendering.md` |
| Touching the Layer Router, Per-Type Autopilot, or Live Performance Controls (bindings, persistent layers, beat snap granularity, Ableton Link) (P20-P21) | `docs/claude/performance-controls.md` |
| Touching the Audio Store or the performance take recorder (Ruling 28) | `docs/claude/recording.md` |
| Touching the production REST API, OSC input, MIDI output, video recording, Syphon output, genre detection, ISF shader import, smart BPM recovery, or Advanced Audio Analysis (P22/P23/P25) | `docs/claude/integration.md` |
| Running or debugging the Eyes visual test harness or its REST endpoints | `docs/claude/testing-eyes.md` |
| Adding/tuning a procedural fractal source, or doing browser-based shader testing before porting a shader into `EmbeddedShaders.h` | `docs/claude/fractals.md` |
| Hitting a bug that might already be a known pitfall (check the one-line index above first) | `docs/claude/pitfalls.md` |
| Building on Windows/Linux, adding Aubio, or adding any new project dependency | `docs/claude/build-other-platforms.md` |
| Needing milestone history, the v2 redesign rationale, or the `research/` document index | `docs/claude/history.md` |
