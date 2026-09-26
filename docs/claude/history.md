# Milestone History + Research Documents Reference

> Moved from CLAUDE.md (claudemd-split).

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

Core audio pipeline, full 14-stage analysis engine, OpenGL rendering with 135 GLSL effects + 15 transitions, mapping engine, VJ dark theme, presets, fullscreen output, camera input, CI/CD.

### v2 Architecture Redesign (CURRENT)

**M7 (keyboard launcher) has been superseded** by a comprehensive Resolume-class architecture redesign. The 4×10 keyboard grid is replaced by a flexible deck/layer/column system with universal signal routing, macros, and procedural sources.

**Design documents**:

- **`ARCHITECTURE_V2.md`** — Complete system design specification
- **`TASKPLAN_V2.md`** — 12-phase implementation plan (P1-P12)

**Key changes from v1**:

- Deck (layers × columns) replaces keyboard grid
- Signal Bar (mixer-strip audio features) replaces left audio readout
- Universal per-parameter signal routing replaces MappingEditor popup
- Dashboard system (8 link knobs per clip/layer/global) for parameter aggregation — NOTE: only the Global dashboard/MacroBank is instantiated (8 live macros; per-clip/per-layer dashboards not implemented)
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
- Session recording (timestamped event capture + JSON save/load) — PARTIAL: only clip triggers are captured (6/7 event types never called); playback is DEAD (advancePlayback never called). Save/load work.
- Undo/redo scaffold (Command pattern) — NOT functional: zero concrete Command subclasses, perform() never called, so the undo/redo keys are permanent no-ops
- 108 procedural sources across 18 registry categories (full per-category breakdown in CLAUDE.md's Key Capabilities line and in `.harmony/APP-INVENTORY.md`; SourceRegistry is ground truth)
- Video playback via FFmpeg (MP4/MOV/AVI/MKV/WebM/HAP Alpha) with transport controls
- Image sequence playback (multi-image drag-drop as video) with configurable FPS
- BPM Sync transport mode for video/image sequences with beat division presets
- Content Beats setting for exact beat-locked timing of authored content

**See TASKPLAN_V2.md for current phase and task details.**

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

These are read-only reference material. All decisions have been made and are reflected in ARCHITECTURE_V2.md and this CLAUDE.md.

---
