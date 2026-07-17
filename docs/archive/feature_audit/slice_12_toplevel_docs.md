# Slice 12 — Top-Level Docs Feature Audit

Sources audited (all absolute paths):
- `/Users/boriskarpman/Documents/RealTimeAudio/ARCHITECTURE.md` (683 lines, v1 MVP spec)
- `/Users/boriskarpman/Documents/RealTimeAudio/ARCHITECTURE_V2.md` (978 lines, v2 Resolume-style spec)
- `/Users/boriskarpman/Documents/RealTimeAudio/TASKPLAN.md` (405 lines, M1–M7 plan)
- `/Users/boriskarpman/Documents/RealTimeAudio/TASKPLAN_V2.md` (494 lines, P1–P12 plan)
- `/Users/boriskarpman/Documents/RealTimeAudio/PHASE_GUIDE.md` (223 lines, phase tracker + P13–P26 index)
- `/Users/boriskarpman/Documents/RealTimeAudio/BUILDLOG.md` (5 lines, init + M1 only — STALE)
- `/Users/boriskarpman/Documents/RealTimeAudio/VALIDATION_PROTOCOL.md` (126 lines)
- `/Users/boriskarpman/Documents/RealTimeAudio/LESSONS_LEARNED.md` (59 lines, 4 entries only)
- `/Users/boriskarpman/Documents/RealTimeAudio/research/FX_SOURCE_AUDIT.md` (639 lines, 2026-03-23 audit)
- `/Users/boriskarpman/Documents/RealTimeAudio/README.md` (253 lines, user-facing)

---

## Per-Phase Promises (from TASKPLAN_V2.md and PHASE_GUIDE.md)

PHASE_GUIDE.md (`PHASE_GUIDE.md:23-36`, `:189-207`) lists statuses. TASKPLAN_V2.md details tasks. Promises below.

**Pre-v2 milestones (TASKPLAN.md, CLAUDE.md declares all COMPLETE)**:
- **M1** — App window, audio file load (WAV/AIFF/MP3), scrolling waveform, RMS meter, transport controls. (`TASKPLAN.md:11-50`) — COMPLETE.
- **M2** — Full 13-stage analysis pipeline: FFT, spectral, onset, BPM, MFCC, chroma, key, LUFS, structural, pitch, HCDF, transient density, smoother, spectrum display. (`TASKPLAN.md:54-124`) — COMPLETE.
- **M3** — OpenGL image rendering + 4 starter effects (Ripple, Hue Shift, RGB Split, Vignette). (`TASKPLAN.md:127-171`) — COMPLETE.
- **M4** — MappingEngine + 17 effects + 5 curves + EffectLibrary + EffectsRackPanel + MappingEditor. (`TASKPLAN.md:175-218`) — COMPLETE.
- **M5** — Dark VJ theme, knob component, AudioReadout/Waveform/Spectrum polish, PresetManager JSON, drag-drop, keyboard shortcuts, FPS counter. (`TASKPLAN.md:222-269`) — COMPLETE.
- **M6** — Cross-platform (macOS ARM64/x86_64, Windows MSVC, Linux GCC), CI, ASan/TSan. (`TASKPLAN.md:272-301`) — COMPLETE.
- **M7** — 40-key keyboard launcher with 4 transparency modes, video playback (HAP Alpha via FFmpeg), latch/random, CompositorEngine. (`TASKPLAN.md:304-391`) — **SUPERSEDED by v2 (CLAUDE.md marks "SUPERSEDED")**.

**v2 phases (TASKPLAN_V2.md — canonical for P1–P12)**:

- **P1 — BPM Stabilization** (`TASKPLAN_V2.md:8-34`): range gate, confidence gate, octave correction, median filter (48-wide, ~500ms), hysteresis lock (2s), `trackerState` field (searching/locking/locked), color-coded BPM display. **COMPLETE**.
- **P2 — Downbeat Detection** (`TASKPLAN_V2.md:37-62`): downbeat scoring `0.5*bass + 0.3*flux + 0.2*harmonic`, 4-beat window, `beatInBar`, `barPhase`, `downbeatDetected`, 4-beat indicator UI, downbeat flash. **COMPLETE**.
- **P3 — Architecture Foundation** (`TASKPLAN_V2.md:65-125`): Command/UndoManager (⌘Z/⇧⌘Z), Composition/Deck/Layer/Clip/Column model, 5 layer types (Opaque/Transparent/FXOnly/ThreeD/Mask), Signal system (AudioSignal/OscillatorSignal/EnvelopeSignal/SignalRegistry), RoutingEngine, MacroBank (6 knobs per scope), Binding system, multi-layer CompositorEngine, Autopilot. **COMPLETE**.
- **P4 — Signal Bar + Top Bar** (`TASKPLAN_V2.md:129-162`): SignalStrip (3 sizes: minimized/normal/expanded), SignalBar with collapsible categories + `[+]` button, horizontal scroll, top bar BPM multiplier ÷4/÷2/×1/×2/×4, Quantize (Off/Next Beat/Next Downbeat), global Fade, Programming Mode. **COMPLETE**.
- **P5 — Deck View** (`TASKPLAN_V2.md:166-204`): ClipCell (thumbnail+name), LayerStrip (X/B/S/M/A/V + type dropdown + opacity + transport), DeckView (layers×columns), column triggering with quantize + beat snap, retrigger logic. **COMPLETE**.
- **P6 — Inspector** (`TASKPLAN_V2.md:208-258`): UniversalParamControl with source picker (Manual/Signal/Macro), invert/range/dial-range, envelope editor. EffectStackView collapsible rows. MacroPanel (6 knobs). 4 inspector tabs: Clip (Macros/Transport/Autopilot/Beat Snap/Effect Stack/Cuepoints), Layer (Macros/Blend/Keying/Layer Effects/Autopilot Defaults/Transition Speed), Composition (Global Macros/Effects/Master/Transition/Output), Signal (Audio thresh/gain/falloff, Oscillator wave-shape/beat-dur/amp/phase, Envelope curve editor, Routes list). **COMPLETE**.
- **P7 — Browser** (`TASKPLAN_V2.md:262-306`): 5 tabs — Files (folders/search/favorites/thumbnails), FX (8 categories, 76 icons — **PROMISED in task description line 278** — FX Presets nested), Sources ("10 Tier 1 generators for launch" — `TASKPLAN_V2.md:285`), Comp/Decks, Record. **COMPLETE**. (Note: actual shipped FX far exceeds 76 and Sources far exceeds 10 — promises superseded by later phases.)
- **P8 — Preview/Output/Layout/Menus/Prefs** (`TASKPLAN_V2.md:310-340`): [Preview]/[Output] tabs, OutputWindow multi-layer update, 9 menus (Audio-DNA/Composition/Deck/Layer/Column/Clip/Output/Shortcuts/View), Preferences dialog (8 tabs — General/Audio/Video/MIDI/Recording/Defaults/Feedback/About), complete layout integration. **COMPLETE**.
- **P9 — Binding + MIDI** (`TASKPLAN_V2.md:344-365`): Keyboard bind mode (⇧⌘K), MIDI learn mode (⇧⌘M), JUCE MidiInput device selection. **COMPLETE**.
- **P10 — Procedural Sources** (`TASKPLAN_V2.md:369-391`): Source base class, "Tier 1 sources (10)" — Perlin Noise, Plasma, Voronoi, Kaleidoscopic Fractal, Mandelbrot/Julia, Geometric Tunnel, Color Gradient, Audio Waveform, Reaction-Diffusion (ping-pong), Particle System (CPU). **COMPLETE**. (v2 now at 81 sources per CLAUDE.md.)
- **P10.5 — Inspector Overhaul (Resolume-style)** (PHASE_GUIDE.md:34): full inspector with signal triangles, transform section, video section, 8-link dashboard. **COMPLETE**. **Not present in TASKPLAN_V2.md — was inserted mid-stream**.
- **P11 — Video Playback** (`TASKPLAN_V2.md:395-435`): FFmpeg integration (MP4/MOV/AVI/MKV/WebM/M4V/HAP Alpha), ImageSequence playback (0-6 fps config), BPM Sync transport mode, beat division presets 1/4 beat through 16 beats, Content Beats snap 1/2/4/8/16/32/64, beat snap for triggering, CompositorEngine video+imagesequence support. P11.6 cuepoints (8 slots per clip) deferred into P11 as completed. **COMPLETE**.
- **P12 — Phrase Tracking + Polish** (`TASKPLAN_V2.md:439-475`): phrase tracking (barCount, phrasePhase over N bars default 8, reset on structural), beat wheel indicator, BarPhase/PhrasePhase/BarCount in MappingSource + SignalRegistry, SessionRecorder (timestamped event log + JSON save/load + playback), ClipInspector redesign with timeline/in-out handles/playhead triangle/beat markers, section headers. **COMPLETE with two explicit DEFERRED items**:
  - **P12.3 Syphon/NDI texture sharing DEFERRED** (`TASKPLAN_V2.md:456-459`) — re-landed in P22.
  - **P12.5 Cross-platform testing DEFERRED** (`TASKPLAN_V2.md:470-475`) — Windows/Linux CI updates still open.

**v2 effects/sources/features build (PHASE_GUIDE.md:181-208, driven by `research/UNIFIED_BUILD_PLAN.md`)**:
- **P13 — Effect Infrastructure**: dry/wet, temporal, LUT, shared GLSL, categories. **COMPLETE**.
- **P13.5 — Core Bug Fixes & Render Optimization** (10 items). **COMPLETE**.
- **P13.5b — UX Fixes**: FX drag-drop, autopilot, multi-video, manual BPM, tooltips. **COMPLETE**.
- **P14 — Quick-Win Effects**: 20 effect shaders + 15 transition shaders. **COMPLETE**.
- **P15 — Medium Effects + Resolume Sources** (24 items). **COMPLETE**.
- **P15.5 — Fractal UX Overhaul + 3D Fractal Expansion**: 5 new 3D fractals + palettes + zoom + trails + feedback. **COMPLETE**.
- **P16 — Time Effects + Feedback System + Signal Routing**: 6 temporal effects (Echo, Posterize Time, Freeze, Screen Split, Frame Stutter, Channel Delay), 6 feedback presets, signal routing in render loop. **COMPLETE**.
- **P17 — Creative Sources**: Math, 3D, Geometric, Nature, Lighting — 19 sources. **COMPLETE**.
- **P18 — Audio-Native Effects & Sources** (20 items): 10 audio-driven effects, 8 audio-native sources, text wall, audio uniform infra. **COMPLETE**.
- **P19 — Complex Effects + Remaining Sources** (20 items): 8 complex effects, 12 sources, Particle category. **COMPLETE** (135 effects + 76 sources per CLAUDE.md history note).
- **P20 — Systems**: Layer Router, Per-Type Autopilot, Text Animator, 3 simulations (Strange Attractor/Gravity Well/Fluid Dynamics), + 15 items total → 81 sources. **COMPLETE**.
- **P21 — Live Performance Controls**: piano/momentary trigger mode, MIDI velocity-to-opacity, CC relative mode (endless encoders), 3 target modes (ByPosition/ThisItem/Selected), persistent layers across deck switches, Ableton Link (optional `-DAUDIODNA_BUILD_LINK=ON`), beat snap granularity (Off/Beat/Bar/2Bar/4Bar). **COMPLETE**.
- **P22 — Output & Integration**: REST API (port 7070, 20+ endpoints, CORS, always-on), OSC input (juce_osc, /audiodna/* patterns), MIDI output (Launchpad X/Mini MK3 colors, 5 states), video recording (FFmpeg H.264/ProRes/MJPEG, triple-buffered GL readback), Syphon output/input (macOS optional), Spout/NDI **stubs only — NOT fully implemented** (CLAUDE.md: "Spout is Windows-only (requires Spout2 SDK). NDI requires separately downloaded SDK from ndi.video."), snapshot PNG capture. **COMPLETE**.
- **P23 — Smart Audio Features**: 8-genre detection (House/Techno/DnB/HipHop/Ambient/Rock/Pop/Jazz), AI mapping suggestions, smart energy-aware autopilot, structural scene triggering, ISF shader import, per-genre smoothing tuning, smart BPM recovery during silence. **COMPLETE**.
- **P24 — Workflow Polish**: 21 easing functions, signal chaining, clip position signal, content replace/lock, layout presets, missing file relocate, collect media, undo wiring, binding presets, inspector pin, layer fold/reorder. **COMPLETE**.
- **P25 — Advanced Audio Analysis**: 5 advanced features (sidechain pump, swing ratio, formant presence, resonance peak, reese bass), composition-level transform (position/scale/rotation/anchor), cross-deck transitions (3 blend modes Alpha/Add/Multiply). **COMPLETE**.
- **P26 — Comprehensive Tooltips**: every button/slider/dropdown/feature gets a tooltip. **NOT STARTED** (PHASE_GUIDE.md:207) — **OPEN GAP**. CLAUDE.md also calls this out: "Comprehensive tooltip coverage is scheduled for P26 (final build phase)."

---

## ARCHITECTURE_V2 Features (Comprehensive)

Section numbers correspond to `ARCHITECTURE_V2.md`.

- **§1 Product Identity** (`:7-11`): Resolume-class clip/layer/signal tool with 42+ audio features, auto BPM lock, auto downbeat, procedural sources.
- **§2 Terminology** (`:15-30`): Signal, Signal Bar, Macro (6/scope — clip/layer/global), Route, Binding, Composition, Deck, Layer (5 types), Clip, Column, FX Preset, Source.
- **§3 Application Layout** (`:34-77`): Menu bar / Top bar / Signal bar / Deck / Preview+Output | Inspector | Browser.
- **§4.1 Signal Bar** (`:83-141`): 3 display sizes. **Default 12 signals on fresh launch** (`:114`): Waveform, Volume, Sub, Bass, Mid, Air, Tempo, Beat Position, Beat In Bar, Bar Position, Hit, Energy State, Mod 1 (Sine 1-beat), Mod 2 (Envelope linear). Collapsible categories: Amplitude / Bands / Rhythm / Pitch / Chroma / Timbre / Structure / Modulation. Readout format table `:129-142` — 11 signal-type-specific display formats.
- **§4.2 Deck** (`:144-213`): Default 3 layers × 12 columns (unlimited add). 5 layer types with type-specific inspector controls. Clip cell = 80×60px thumbnail + name bar. Layer strip with X/B/S/M/A/V buttons, type dropdown, 24×24 opacity knob, 60×20 transport.
- **§4.3 Inspector** (`:215-268`): 4 tabs, auto-switches based on last clicked. Clip tab sections: Macros, Transport, Autopilot, Beat Snap, Effect Stack, Cuepoints (8 slots). Layer tab: Macros, Blend Mode, Keying, Layer Effects, Autopilot Defaults, Transition Speed, 3D Controls. Composition tab: Global Macros, Global Effects, Master Opacity, Transition Speed, Output Settings. Signal tab: per-type controls + Routes list.
- **§4.4 Browser** (`:270-302`): 5 tabs + Record. Files tab has folder nav/search/favorites/list-grid toggle. FX tab by category with nested FX Presets. Sources tab: "40 generators (10 Tier 1 for launch), each with auto-generated thumbnail" (`:291`) — **docs say 40, code now has 81**. Comp/Decks tab + Record tab.
- **§4.5 Preview/Output** (`:304-316`): [Preview] tab (selection-sensitive) + [Output] tab (external display).
- **§5 Universal Parameter Control** (`:320-378`): source picker (Manual/Signals/Macros), invert, range, dial range, envelope editor.
- **§6 Macro System** (`:380-398`): 6 macros per scope with signal source + multiple parameter links.
- **§7 Rendering Pipeline** (`:400-416`): per-layer bottom-to-top composite with Clip Effects → Keying → Layer Effects → Blend → Opacity → Accumulator, mask layers, global effects, master opacity.
- **§8 BPM / Rhythm System** (`:420-503`): 5-stage stabilization pipeline, free-running sawtooth, downbeat scoring formula, new FeatureSnapshot fields (`beatInBar`, `barPhase`, `downbeatDetected`, `trackerState`, `barCount`, `phrasePhase`), BPM multiplier ÷4/÷2/×1/×2/×4, tracker state color coding, Resync button.
- **§9 Audio Feature Labels** (`:507-544`): 42+ features with intuitive UI names (Volume/Peak/Punch/Hits Per Second/Brightness/Change/Noisiness/Treble Cutoff/7 bands/Tempo/Beat Position/Beat In Bar/Bar Position/Downbeat/Phrase Position/Tracker State/Hit/Hit Strength/Note/Note Confidence/Key/Chord Change/Timbre 1-13/C-B chroma/Energy State).
- **§10 Modulation Bank** (`:548-566`): 6 default oscillators/envelopes. Oscillator: sine/saw-up/saw-down/triangle/square + beat durations 1/4, 1/2, 1, 2, 4, 8 + amp + phase offset. Envelope: curve editor, phase, octaves, curve type, one-shot/loop. Preset save/load with meaningful names ("Gentle Pulse", "Hard Gate").
- **§11 Autopilot** (`:570-579`): per-clip + per-layer, 8 action options, 8 duration options (Layer Determined/1/2/4/8/16/32 beats/Custom), bar-aware.
- **§12 Binding System** (`:583-607`): keyboard + MIDI learn modes. Bindable targets: individual clips, columns, layer controls (B/S/M/A/V + transport), effect bypass, macro knobs, deck switching, tap tempo, resync, global transport, master opacity.
- **§13 Transition System** (`:611-618`): crossfade between clips, global transition speed + per-layer override.
- **§14 Column Triggering** (`:621-628`): clips play, empty cells clear, quantize + beat snap.
- **§15 Clip Trigger Behavior** (`:632-638`): click=trigger, retrigger on same click, empty=clear, no toggle mode.
- **§16 Layer Management** (`:642-650`): via Layer menu only (New ⌘L, Insert Above/Below, Duplicate, Rename, Remove, Copy/Paste Effects, Clear Clips, Ignore Column Trigger, Mask Mode). No right-click context menu on layer strip.
- **§17 Undo/Redo** (`:654-675`): Command pattern. **Tracked**: param changes, clip assignments, effect CRUD/reorder/bypass, layer CRUD/reorder, route CRUD, signal config, macro link changes. **NOT tracked**: clip/column triggering, audio source, output display.
- **§18 Recording** (`:679-691`): **Phase 1 Settings Recording** (events with timestamped JSON — done in P12). **Phase 2 Video Recording** with HAP Alpha codec — ⚠️ **docs promise HAP Alpha but code ships H.264/ProRes/MJPEG per P22** — potential contradiction.
- **§19 Procedural Sources** (`:695-711`): "40 generators across 9 categories" + 10-source Tier 1 launch list. **Docs say 40, code has 81** — counts long superseded by P17–P20.
- **§20 Preferences Dialog** (`:715-726`): 8 tabs (General/Audio/Video/MIDI/Recording/Defaults/Feedback/About).
- **§21 Menu Bar** (`:730-749`): 9 menus detailed per-menu. Output menu includes "Texture Sharing (Syphon), NDI" — NDI is stub-only (see P22 note).
- **§22 Keying Modes** (`:753-771`): **13 modes** — Alpha, Luma Key, Inverted Luma Key, Luma is Alpha, Inverted Luma is Alpha, Chroma Key, Max RGB, Saturation Key, Edge Detection, Threshold 50%, Channel:Red, Channel:Green, Channel:Blue.
- **§23 Blend Modes** (`:775-786`): **17 modes** grouped — Normal; Lighten family (Additive/Screen/Lighten/Color Dodge); Darken family (Multiply/Darken/Color Burn); Contrast family (Overlay/Soft Light/Hard Light/Vivid Light/Linear Light/Pin Light/Hard Mix); Inversion (Difference/Exclusion). **Default: Additive** (VJ standard).
- **§24 Video Codec** (`:790-794`): "Primary: HAP Alpha. Fallback: PNG sequence." ⚠️ **Contradicts actual ship**: code uses FFmpeg generic decode (MP4/MOV/AVI/MKV/WebM/M4V + HAP Alpha per CLAUDE.md tech stack). PNG sequence claim replaced by ImageSequence feature. Flag as stale.
- **§25 Data Model Hierarchy** (`:798-822`): Composition → Deck(s) → Layer(s) → Clip(s), plus Routes / Bindings / Signal Config / Global Effects / Global Macros / FX Presets / Settings.
- **§26 Thread Model** (`:826-839`): unchanged from v1, new: Routing Engine replaces MappingEngine on render thread, multi-layer compositing, deck/inspector/browser UI on message thread.
- **§27 Effect Icons — AI Generation Spec** (`:843-963`): **"Each of our 76 effects needs a 64×64px icon"** — enumerates 76 AI prompts. Code ships 135 effects per CLAUDE.md; **icons for new effects (P14+) are an open gap if the 76-icon spec was meant to be kept current**.
- **§28 Competitive Position** (`:967-979`): feature matrix vs Resolume/VDMX/TouchDesigner.

### ARCHITECTURE_V2 items flagged as may-not-exist / contradict code

1. **"40 procedural sources" / "10 Tier 1 for launch"** (§19, §4.4) — code has 81 sources. Docs stale.
2. **"76 effects" (§27 icon spec)** — code has 135. Icon coverage for effects 77–135 is unspecified.
3. **"HAP Alpha primary video codec" / "PNG sequence fallback"** (§24) — code uses FFmpeg + (optional) HAP; no PNG sequence fallback mentioned in CLAUDE.md.
4. **Video Recording Phase 2: "HAP Alpha codec for alpha support"** (§18) — code recorder ships H.264/ProRes/MJPEG (no HAP Alpha output).
5. **NDI support listed in Output menu (§21)** — code has NDI stub only (no SDK integration).
6. **Spout mention** (CLAUDE.md) — Windows-only + stub only on macOS.
7. **"Vignette/Spotlight was in v1 but can be achieved with a Mask layer + gradient source"** (§22 note) — implies Mask layer works as documented; no explicit UI for Spotlight.

---

## VALIDATION_PROTOCOL.md

Testing gates and validation procedures (`VALIDATION_PROTOCOL.md:1-126`):

- **Two-method gate**: every task must pass both Method A (automated/mechanical) and Method B (independent) before asking human.
- **Build tasks**: `cmake --build` exits 0 + binary exists and `file` identifies it as valid executable/app bundle.
- **Implementation tasks**: full project compiles + static grep for violations (no `new`/`malloc` in audio callback, no `std::mutex` on hot path, no `#include` cycles, FeatureSnapshot POD, shader uniforms `u_` prefix).
- **Shader tasks**: `glslangValidator` or project builds with shader loading; uniform decls match EffectLibrary, uses `u_` prefix, accepts `u_time` + `u_resolution`.
- **UI tasks**: compiles with component integrated; follows JUCE Component patterns, uses timer or async updates, no blocking.
- **Test tasks**: `ctest` or binary runs, all tests pass; coverage — each public method of class under test has ≥1 test.
- **Integration tasks**: builds + runs 10+ seconds without crash; observable output (audio plays, meters move, waveform draws).
- **Commit protocol** (after human PASS): stage specific files (not `git add .`), reference M[N]/task [X.Y] in message, update BUILDLOG.md + CLAUDE.md milestone status, generate next task prompt.
- **Report template** and **next task prompt template** provided at end of doc.

Note: **BUILDLOG.md is stale** — last entry 2026-03-13 for M1 (line 5). Validation protocol step 3 (“Update BUILDLOG.md”) has not been followed for M2+.

---

## LESSONS_LEARNED key entries (relevant to UI design)

Only **4 entries** total (`LESSONS_LEARNED.md`). All verified by human with dates. Strict gate (memory note: must have human confirmation before adding).

- **LL-001** (`:7-17`, M1): `AudioTransportSource::setSource()` needs valid `TimeSliceThread*` for buffered file playback — backend (not UI-user-visible, but affects file load path).
- **LL-002** (`:21-31`, M5): **UI-critical** — JUCE recreates GL context on window move/resize. Any GL renderer must reload textures/shaders/FBOs in `newOpenGLContextCreated()`. Applies to OutputWindow + PreviewPanel + any OpenGL-backed component.
- **LL-003** (`:35-45`, M5): **UI-critical** — macOS native fullscreen (`setFullScreen(true)`) breaks GL context. Use borderless + setBounds + alwaysOnTop instead. Affects fullscreen output.
- **LL-004** (`:49-59`, M5): `EffectChain::render()` overrides `glViewport` — pass viewport params explicitly. Backend-ish but affects preview letterboxing.

Additional lessons are **only captured in CLAUDE.md "Common Pitfalls" section (27 numbered pitfalls P14–P22)**, not in LESSONS_LEARNED.md — the dedicated file is underutilized. This is a documentation gap.

---

## FX_SOURCE_AUDIT summary

Audit date 2026-03-23 (`research/FX_SOURCE_AUDIT.md:3`). Scope: 135 effects + 81 sources vs Resolume Arena 7, After Effects CC, ArKaos GrandVJ, VDMX, TouchDesigner.

**What was audited + fixes applied**:
- 76 effect default values updated in `EffectLibrary.cpp` (FX_SOURCE_AUDIT.md:10-15). Build passed, 109/110 C++ tests pass (1 pre-existing barPhase fail). All 135 effects verified to have visible primary-param defaults via Eyes.
- Effects that should be neutral at 0.5 (Saturation/Brightness/Exposure/Vibrance/Contrast/Color Shift/Shear/Fisheye/Barrel Distort) were NOT changed. Flip uses 0.0 (toggle).

**Rating summary** (`:611-624`):
| Metric | Count |
|---|---|
| Total Effects | 135 |
| Good | 41 (30%) |
| Needs Adjustment | 90 (67%) |
| Missing Params | 2 (1.5%) |
| Broken | 0 |
| Total Sources | 81 |
| Sources Good | 78 (96%) |
| Sources Needs Adjustment | 2 |
| Sources Broken | 1 (Julia Set source shader compilation failed) |

**What remains (open from audit)**:

- **Critical**: `source_julia_set` shader compilation FAILED at startup (`:454`, `:604`). Source unusable. Listed as "Must Fix" — unknown if resolved.
- **Missing params** (🔧 items, 15 total, `:488-506`): Gaussian Blur (two-pass / larger radius), Color Grade (only 1 param vs AE's 20+), Selective Color (saturation control, replacement hue, deselected handling), Wave (wave type, pinning), Sphere Wrap (rotation/lighting/radius), Page Curl (direction/back-page), Chroma Key (spill suppression), Night Vision (grain/scanlines/bloom), VHS (noise/color-bleed/jitter), Film Grain (monochrome toggle), Glow (radius, color), Echo (should be dropdown not slider), Strobe (blend mode, random prob), Pixelate (independent H/V), Shake (speed/frequency).
- **Near-duplicate effects** to consolidate (`:509-516`): Swirl vs Twirl, Liquid vs Liquid Morph, Ripple vs Ripple Pond, Wormhole vs Wormhole Tunnel, Striped Torus vs Twisted Torus vs Spiral Vortex.
- **Renaming** (`:516-519`): Point Zoom→"Feedback", UV Remap→"Noise Displace", Color Shift→"Color Balance".
- **Mapping bugs**: Posterize levels 1.0 = no effect (inverted mapping). Mirror continuous slider should be boolean.
- **Missing effects — industry gap** (`:527-544`): Color Curves (RGB curves), Lumetri-style Grade, Difference blend, Displacement Map (from texture), Rotate/Scale as chainable effects, Edge Glow, Noise Warp proper, Grid Warp, Light Rays / God Rays, Lens Flare, Particle System effect, Stencil/Mask, Bloom, Tilt Shift.
- **Missing sources — industry gap** (`:558-570`): Test Pattern (SMPTE bars), Gradient Ramp (multi-stop), Clock/Timer, Scope/Oscilloscope, Spectrum Bars (classic EQ), Particle Emitter, **Text Source (user-editable, HIGH priority — Resolume has this; our Text Wall/Animator are procedural only)**, Gradient Circle, Grid overlay.
- **Eyes infrastructure issues** (`:598-606`): Render frame size inconsistency (756x756 vs 878x756) causing PSNR failures; 500 errors on render_frame (threading/timing).
- **Naming conventions** (`:574-593`): inconsistent ("amount"/"intensity"/"force"/"strength"; "speed"/"rate"/"frequency"; "softness"/"range"/"tolerance"). Recommendation to standardize.

---

## README.md highlights (user-facing feature list)

README promises (`README.md:1-253`) — this is the **public-facing** doc, and it describes the **v1 keyboard-launcher era workflow**, NOT the v2 Resolume-style workflow. It is **OUT OF DATE**.

- Promises "75+ GLSL shader effects" (`:3`, `:103`) — code has 135.
- Lists 8 effect categories with representative effects (`:107-114`).
- "30+ audio features extracted in real-time" (`:96-102`) — code has 42+.
- Describes **v1 workflow only**: Effects Rack + Mapping Editor + preset slots at bottom (`:54-91`). No mention of decks, layers, clips, signal bar, inspector, browser — all of v2.
- **10 numbered preset slots at bottom bar** (`:78-79`) — v1 concept, now replaced by Deck/Clip grid.
- **Beat-synced random mode**, **Sync button**, **per-effect lock (L)**, **per-effect randomize (R)**, **FX Save** — v1 features described in detail.
- **Deck save/load** — describes v1 `.deck.json` format, not v2 Composition format.
- Build instructions reference JUCE 8.0 (`:242`) — CLAUDE.md tech stack table says JUCE 7.0.12 (contradiction).
- Keyboard shortcuts table (`:145-151`): Escape, 1-9, Cmd+S, Cmd+O, Cmd+F — these are v1 shortcuts; v2 has ⇧⌘K (binding), ⇧⌘M (MIDI learn).
- Does not mention: genre detection, AI mapping, ISF import, REST API, OSC, MIDI output, video recording, Syphon, Ableton Link, per-type autopilot, signal chaining, layer router, cross-deck transitions, advanced audio features (sidechain/swing/formant/resonance/reese), composition transform, 21 easing functions, layout presets, 15 transitions, temporal effects, feedback system, fractal system, torus system.

---

## Section 18 partial: Features in docs but maybe not code (flag these)

1. **P26 Comprehensive Tooltips** — explicitly NOT STARTED (PHASE_GUIDE.md:207). User-facing gap: many UI elements lack tooltips. CLAUDE.md acknowledges this: "Comprehensive tooltip coverage is scheduled for P26 (final build phase)."
2. **P12.3 Syphon/NDI** — marked DEFERRED in P12 (`TASKPLAN_V2.md:456-459`). Syphon shipped in P22 as optional build flag (`-DAUDIODNA_BUILD_SYPHON`) with compile-time stub fallback. **NDI remains stub-only** (CLAUDE.md "NDI requires separately downloaded SDK"). **Spout is stub-only** (Windows-only, not implemented).
3. **P12.5 Cross-platform testing** — DEFERRED (`TASKPLAN_V2.md:470-475`). Windows (MSVC) and Linux (GCC) CI updates for new source files may not be current. README claims cross-platform but actual build status on Windows/Linux unverified.
4. **ARCHITECTURE_V2 §18 Phase 2 Video Recording with "HAP Alpha codec"** — code instead ships H.264/ProRes/MJPEG. HAP Alpha encoding NOT implemented (HAP is decode-only via FFmpeg).
5. **ARCHITECTURE_V2 §24 Primary video codec "HAP Alpha" / fallback "PNG sequence"** — neither fully implemented as primary. Ships FFmpeg-generic decode + ImageSequence as a separate feature (not a codec fallback).
6. **ARCHITECTURE_V2 §27 76-effect icon spec** — icons for effects 77–135 (the P14+ additions) unspecified. Browser may or may not have icons for all.
7. **ARCHITECTURE_V2 §10 Modulation signal presets save/load "Gentle Pulse", "Hard Gate"** — unclear if preset save/load implemented for oscillators/envelopes.
8. **ARCHITECTURE_V2 §5 Envelope editor with draggable control points** — full curve editor with draggable points promised; implementation depth unclear.
9. **ARCHITECTURE_V2 §4.4 Browser FX tab "Each effect has a hand-designed icon"** — icon coverage across all 135 effects unclear.
10. **ARCHITECTURE_V2 §4.4 Sources tab "auto-generated thumbnail"** — implementation status for all 81 sources unclear.
11. **FX_SOURCE_AUDIT Julia Set source shader compilation failure** — listed as critical "must fix" 2026-03-23, resolution status unknown.
12. **FX_SOURCE_AUDIT 15 Missing Params items** (`:488-506`) — none fixed as part of the 2026-03-23 default-values pass; remain open.
13. **FX_SOURCE_AUDIT Missing industry effects** — Color Curves, Displacement Map (texture-based), Tilt Shift, God Rays, Lens Flare, etc. — open feature requests.
14. **FX_SOURCE_AUDIT Missing user-editable Text Source** — `:569` marks as HIGH priority (Resolume has it; our Text Wall/Animator are procedural only).
15. **README "Camera input"** claims live webcam feed (`:45`, `:134`) — verify camera path still wired through v2 deck system.
16. **README "Image slideshow (2-128 beats per image)"** (`:133`) — v1 feature; v2 has image-sequence clips with different granularity.
17. **BUILDLOG.md is frozen at M1** — VALIDATION_PROTOCOL.md requires updates per task; ~24 phases of updates missing.
18. **LESSONS_LEARNED.md has only 4 entries** — CLAUDE.md holds ~27 unformalized "Common Pitfalls" that never made it into LESSONS_LEARNED (gate requires human confirmation).

---

## Section 20 partial: Cross-references to authoritative docs

- **Canonical feature list / current status**: `CLAUDE.md` (root). Updated per-phase. Most accurate source for effect/source counts and shipped features.
- **v2 phase status tracker**: `PHASE_GUIDE.md:23-36` (P1–P12) and `PHASE_GUIDE.md:189-207` (P13–P26). Matches CLAUDE.md memory indices.
- **v2 design spec**: `ARCHITECTURE_V2.md` — single source of truth for architecture but **counts stale** (40 sources / 76 effects / HAP Alpha codec claims).
- **v2 per-phase task detail for P1–P12**: `TASKPLAN_V2.md`.
- **v2 per-phase task detail for P13–P26**: `research/UNIFIED_BUILD_PLAN.md` (referenced from PHASE_GUIDE.md:183 and CLAUDE.md).
- **v1 legacy (superseded M7)**: `TASKPLAN.md:304-391` + `ARCHITECTURE.md` (still present, referenced by legacy README).
- **Testing gates**: `VALIDATION_PROTOCOL.md`.
- **Shader verification protocol**: `tests/visual/SHADER_VERIFICATION.md` (referenced from CLAUDE.md — 4-tier system: auto-sweep / range-quality / browser / in-app).
- **Shader implementation refs**: `research/resolumeEffectSourceIntegration.md`, `research/archaosEffectSourceIntegration.md`.
- **FX/Source audit**: `research/FX_SOURCE_AUDIT.md` (2026-03-23).
- **User-facing README**: `README.md` — **outdated, describes v1**.
- **BUILDLOG.md**: stale (only M1 entry).
- **LESSONS_LEARNED.md**: underused (only 4 entries; bulk of lessons are in CLAUDE.md "Common Pitfalls").

### Documentation contradictions to flag

1. **Source counts**: ARCHITECTURE_V2 §19 "40 generators" vs code "81 sources" vs CLAUDE.md "81 sources".
2. **Effect counts**: ARCHITECTURE_V2 §27 "76 effects" vs code "135 effects" vs README "75+" vs CLAUDE.md "135".
3. **Video codec**: ARCHITECTURE_V2 §24 "HAP Alpha primary / PNG sequence fallback" vs code "FFmpeg generic decode + HAP Alpha via FFmpeg + optional HAP library".
4. **Video recording format**: ARCHITECTURE_V2 §18 "HAP Alpha codec" vs code "H.264/ProRes/MJPEG".
5. **JUCE version**: README "JUCE 8.0" vs CLAUDE.md "JUCE 7.0.12".
6. **README entire workflow description** vs actual v2 deck/layer/signal system.
7. **TASKPLAN.md M7 full description** (405 lines total, last 90 detail M7) vs CLAUDE.md "M7 SUPERSEDED by v2".
8. **LESSONS_LEARNED.md vs CLAUDE.md pitfalls**: bulk of lessons live in CLAUDE.md "Common Pitfalls" section instead of the dedicated file.
