# Control Features — Gap Fill (Phase 1)

**Date:** 2026-05-20 | **Builder:** 3 of 3 (control/routing/signals domain)
**Source:** GAP_REPORT.md findings + source code investigation
**Scope:** Ghost features (control domain), default signals, dual-mode system

---

## Ghost Features -- Code Without UI

### Ghost 1. MappingSuggester — Genre-Aware AI Mapping Suggestions

**What it does:** Analyzes a live FeatureSnapshot and the detected genre to recommend source-to-effect mappings. Returns ranked suggestions with source name, target effect/param, curve type, relevance score, and a human-readable reason.

**Code location:**
- `src/mapping/MappingSuggester.h` (53 LOC) — class declaration, `Suggestion` struct
- `src/mapping/MappingSuggester.cpp` (266 LOC) — implementation
- **Total: 319 LOC**

**Implementation chain:**
1. `suggestMappings(snapshot, maxSuggestions)` — main entry point
2. `addUniversalSuggestions()` — 13 universal suggestions based on live feature activity (bass, mids, highs, beat, onset, centroid, RMS, phrase)
3. `addGenreSuggestions(genre)` — 4 suggestions per genre, all 8 genres covered (House, Techno, DnB, Hip-Hop, Ambient, Rock, Pop/Electronic, Jazz/Other)
4. Sorts by relevance (highest first), trims to maxSuggestions
5. Also: `suggestGenreMappings(genre)` — genre-only suggestions without snapshot

**Current wiring status:**
- Listed as entry point in FEATURES.md Feature 6 (Audio-Visual Mapping)
- Listed as entry point in FEATURES.md Feature 16 (Genre Detection)
- **No UI exists** — no button, menu item, or panel invokes `suggestMappings()` or `suggestGenreMappings()`
- **No caller in production code** — grep confirms zero calls outside the class itself
- Class is fully implemented and compilable but never instantiated by any UI component

**What's missing for activation:**
- UI panel or popup to display suggestions (e.g., "Suggest Mappings" button in MappingEditor)
- Integration with MappingEngine to apply a suggestion as a new Mapping
- Optional: learning from user acceptance/rejection of suggestions

**Activation Estimate:** LOW (~1 day) — code is complete, needs only a UI trigger button + a function to convert Suggestion to Mapping struct and add it to MappingEngine.

---

### Ghost 2. Clip/Layer MacroBanks — Per-Scope Macro Knobs

**What it does:** MacroBank provides 8 "dashboard link" knobs per scope (Clip, Layer, Global). Each macro can be manual or signal-driven, and distributes its value to linked parameters with per-link range and invert. Equivalent to Resolume's Dashboard Links.

**Code location:**
- `src/routing/MacroBank.h` (97 LOC) — `MacroBank` class, `Macro` struct, `MacroLink` struct
- `src/ui/MacroPanel.h` (49 LOC) — UI panel with 8 knob slots
- `src/ui/MacroPanel.cpp` (~135 LOC) — UI implementation with signal source selector

**Implementation chain:**
1. `MacroBank(Scope)` constructor takes scope enum: `Clip`, `Layer`, or `Global`
2. `updateValues(SignalRegistry)` — for manual macros: currentValue = manualValue; for signal-driven: reads from SignalRegistry
3. `MacroPanel` renders 8 knob slots, each with name label, knob, value display, and signal source popup
4. `EffectStackView` reads macro values to drive effect params when `UniversalParamControl::SourceMode::Macro` is selected

**Current wiring status:**
- **Global scope: FULLY WIRED** — `MainComponent.h:207` creates `MacroBank globalMacroBank_{MacroBank::Scope::Global}`, passes to all inspectors
- **Clip scope: NOT INSTANTIATED** — `MacroBank::Scope::Clip` exists in enum but no `MacroBank(Scope::Clip)` is created anywhere. `Clip` model struct has no `MacroBank` member field.
- **Layer scope: NOT INSTANTIATED** — Same as Clip. `Layer` model struct has no `MacroBank` member field.
- MIDI binding routes to `globalMacroBank_` only (MainComponent.cpp:3842-3843)
- UI always receives `&globalMacroBank_` via `setMacroBank()` — no per-clip or per-layer bank is ever passed

**What's missing for activation:**
- Add `MacroBank macroBank{MacroBank::Scope::Clip}` field to `Clip` struct
- Add `MacroBank macroBank{MacroBank::Scope::Layer}` field to `Layer` struct
- Wire inspector panels to pass clip/layer banks when Clip/Layer inspector tab is active
- Update preset serialization (toVar/fromVar) for Clip and Layer to save/load macro state
- Decide scoping behavior: does a clip macro override a layer macro? Do they stack?

**Activation Estimate:** MEDIUM (~3 days) — model changes to Clip/Layer are straightforward, but serialization, inspector switching, and scoping behavior need careful design. Per-clip macros multiply state significantly (N clips x 8 macros each).

---

### Ghost 3. Crossfader — Deck Blend Control

**What it does:** Intended to provide continuous A/B crossfading between decks with configurable blend mode, curve, and behaviour.

**Code location:**
- `src/model/Composition.h:30-37` (8 LOC) — model fields only

**Model fields defined:**
- `crossfaderPhase` (float, default 0.5) — A/B position [0,1]
- `CrossfaderBlendMode` (enum: Alpha, Add, Multiply)
- `CrossfaderBehaviour` (enum: Cut, Smooth)
- `CrossfaderCurve` (enum: Linear, EaseInOut, SCurve)

**Current wiring status:**
- **Model fields exist** in Composition struct
- **One partial read:** `crossfaderBlendMode` is used ONCE in `Renderer.cpp:508` to set `u_blendMode` uniform during deck transitions — but this is the deck TRANSITION system, not a live crossfader
- **crossfaderPhase: NEVER READ** outside the model. No code uses it to blend between decks.
- **crossfaderBehaviour: NEVER READ** outside the model.
- **crossfaderCurve: NEVER READ** outside the model.
- **No UI control** — no slider, knob, or widget for crossfader position
- **Not serialized** — `toVar()`/`fromVar()` in Composition do not save/load crossfader fields

**What's missing for activation:**
- Render pipeline: `CompositorEngine` needs to blend two deck outputs using `crossfaderPhase` instead of the current binary active-deck switching
- Apply `crossfaderCurve` to phase before blending
- Apply `crossfaderBehaviour` (cut = instant switch, smooth = continuous blend)
- UI: horizontal slider or fader widget in composition inspector or top bar
- MIDI/keyboard binding support for crossfader position
- Serialization in preset save/load

**Activation Estimate:** HIGH (~1 week+) — requires significant render pipeline changes to composite two full decks simultaneously (currently only active deck renders, with brief transition blending). Dual-deck rendering doubles GPU workload. Also needs UI, MIDI binding, and serialization.

---

### Ghost 4. Route TargetScope::Clip/Layer — Per-Scope Signal Routing

**What it does:** The Route struct defines `TargetScope` enum with Clip, Layer, and Global values, allowing signal routing to target effect parameters at any scope level. Currently only Global is implemented.

**Code location:**
- `src/routing/Route.h:19-20` — `TargetScope` enum definition (Clip, Layer, Global)
- `src/routing/Route.h:21-24` — `targetLayerId`, `targetClipId` fields for scoped targeting
- `src/routing/RoutingEngine.cpp:77-126` — `processFrame()` implementation

**Current wiring status:**
- **TargetScope::Global: FULLY WIRED** — `Renderer.cpp:200` checks `route.targetScope == Route::TargetScope::Global` and writes to `effectChain_` (global effects)
- **TargetScope::Clip: NOT WIRED** — `Renderer.cpp:205` has explicit `// TODO: Clip/Layer scope routing needs compositor integration`
- **TargetScope::Layer: NOT WIRED** — same TODO comment
- `Route` struct carries `targetLayerId` and `targetClipId` fields but they are never used in `processFrame()` routing
- `TestServer.cpp:770` hardcodes `Route::TargetScope::Global` when creating routes via API
- No UI in Signal Inspector allows selecting Clip or Layer scope for a route target

**What's missing for activation:**
- `Renderer.cpp` processFrame lambda needs per-clip and per-layer effect parameter writes (requires `CompositorEngine` integration since clip/layer effects are applied during compositing)
- UI: scope selector in route editor (currently assumes Global)
- API: TestServer and ApiServer route creation needs scope parameter support

**Activation Estimate:** MEDIUM (~3 days) — the data model is complete. Implementation requires the render pipeline's route writer callback to resolve clip/layer effect chains from the `CompositorEngine`, which holds per-clip and per-layer effects during compositing.

---

### Ghost 5. LinkSync::requestBeatAtTime() — Force Phase Alignment

**What it does:** Resets the Ableton Link session's beat phase to 0 (downbeat) at the current time, forcing all Link peers to realign to the downbeat.

**Code location:**
- `src/sync/LinkSync.h:47` (1 LOC) — method declaration
- `src/sync/LinkSync.cpp:63-74` (12 LOC) — implementation

**Implementation chain:**
1. Checks if Link is enabled (returns early if not)
2. Captures current Link session state
3. Calls `sessionState.requestBeatAtTime(0.0, now, quantum_)` — requests beat 0 at current time
4. Commits session state — propagates to all Link peers

**Current wiring status:**
- **Fully implemented** in LinkSync.cpp
- **NEVER CALLED** — grep confirms zero callers outside the class definition
- No UI button ("Reset Phase" or "Force Downbeat")
- No keyboard/MIDI binding target for phase reset
- No API endpoint exposes this function

**What's missing for activation:**
- UI: "Reset Phase" or "Force Downbeat" button in the Link section of Composition Inspector
- Binding: add as a bindable action in BindingManager
- API: optional REST endpoint `/api/link/reset_phase`

**Activation Estimate:** LOW (~1 day) — implementation is complete and correct. Needs only a UI button or binding action that calls `linkSync_.requestBeatAtTime()`.

---

## Default Signals — Complete Inventory

**Source:** `src/signal/SignalRegistry.cpp` `initDefaults()` method

**Total: 36 signals** (8 visible + 25 hidden audio + 3 modulation/utility)

### Visible Audio Signals (8)

| # | Name | MappingSource | Category | Notes |
|---|------|--------------|----------|-------|
| 1 | Volume | RMS | Amplitude | Overall loudness |
| 2 | Sub Bass | BandSub | Bands | bandEnergies[0] |
| 3 | Bass | BandBass | Bands | bandEnergies[1] |
| 4 | Mid | BandMid | Bands | bandEnergies[3] |
| 5 | Air | BandBrilliance | Bands | bandEnergies[6] |
| 6 | Tempo | BPM | Rhythm | Detected BPM value |
| 7 | Beat Position | BeatPhase | Rhythm | Phase within current beat [0,1) |
| 8 | Hit | OnsetStrength | Rhythm | Transient detection strength |

### Hidden Audio Signals (25)

| # | Name | MappingSource | Category | Notes |
|---|------|--------------|----------|-------|
| 9 | Peak | Peak | Amplitude | Peak amplitude |
| 10 | Punch | DynamicRange | Amplitude | Dynamic range (crest factor) |
| 11 | Hits Per Second | TransientDensity | Amplitude | Onset count in 2s window |
| 12 | Low Mid | BandLowMid | Bands | bandEnergies[2] |
| 13 | High Mid | BandHighMid | Bands | bandEnergies[4] |
| 14 | Presence | BandPresence | Bands | bandEnergies[5] |
| 15 | Hit Strength | OnsetStrength | Rhythm | Duplicate source with "Hit" (#8) |
| 16 | Bar Position | BarPhase | Rhythm | Phase within current bar [0,1) |
| 17 | Phrase Position | PhrasePhase | Rhythm | Phase within current phrase [0,1) |
| 18 | Bar Count | BarCount | Rhythm | Running bar count |
| 19 | Brightness | SpectralCentroid | Amplitude | Spectral centroid frequency |
| 20 | Change | SpectralFlux | Amplitude | Frame-to-frame spectral change |
| 21 | Noisiness | SpectralFlatness | Amplitude | Tonal vs noise ratio |
| 22 | Note | DominantPitch | Pitch | Detected fundamental pitch |
| 23 | Note Confidence | PitchConfidence | Pitch | Pitch detection confidence |
| 24 | Chord Change | HarmonicChange | Pitch | HCDF harmonic change detection |
| 25 | Sidechain Pump | SidechainPump | Amplitude | P25: bass/mid anti-correlation |
| 26 | Swing | SwingRatio | Rhythm | P25: timing deviation from grid |
| 27 | Vocal Presence | FormantPresence | Amplitude | P25: vocal formant energy |
| 28 | Resonance | ResonancePeak | Amplitude | P25: spectral kurtosis |
| 29 | Reese Bass | ReeseBass | Bands | P25: bass spectral spread |
| 30 | (Waveform) | N/A | N/A | Special — rendered directly, not a Signal subclass |
| 31 | (Energy State) | N/A | N/A | From GenreDetector, not a Signal |
| 32 | (Beat In Bar) | N/A | N/A | Available in FeatureSnapshot but not a registered Signal |
| 33 | (Structural State) | N/A | N/A | Available via MappingSource but not registered as Signal |

Note: Signals 30-33 are referenced in architecture docs as "default visible" but are either special cases (Waveform is a visual display, not a routable value) or available only through the MappingEngine path, not the SignalRegistry.

### Modulation Signals (3)

| # | Name | Type | Category | Notes |
|---|------|------|----------|-------|
| 34 | Mod 1 | OscillatorSignal | Modulation | BPM-locked sine wave, 1-beat cycle. Shapes: Sine/SawUp/SawDown/Triangle/Square |
| 35 | Mod 2 | EnvelopeSignal | Modulation | Custom curve with control points, 4-beat cycle. Curve types: Linear/Exponential/SCurve |
| 36 | Clip Position | ClipPositionSignal | Modulation | Hidden. Tracks active clip playhead [0,1], normalized to in/out range. Updated from render thread |

### Signal Type Breakdown

| Signal Type | Count | Visible | Hidden |
|-------------|-------|---------|--------|
| AudioSignal | 29 | 8 | 21 |
| AudioSignal (P25 advanced) | — | 0 | 5 |
| OscillatorSignal | 1 | 1 | 0 |
| EnvelopeSignal | 1 | 1 | 0 |
| ClipPositionSignal | 1 | 0 | 1 |
| **Total** | **32 registered** | **10** | **22** |

Note: The GAP_REPORT claims 36 signals (8 visible + 25 hidden + 3 modulation). The actual `initDefaults()` registers 32 Signal objects. The discrepancy of 4 comes from counting Waveform, Energy State, Beat In Bar, and Structural State as "signals" in the architecture docs even though they are not registered as `Signal` subclass instances in `SignalRegistry`. They are available through `MappingSource` enum (57 total sources) but not through the signal routing path.

---

## Dual-Mode System — Current State vs Intended Design

### Architecture Intent

Boris's core design tension: sub-features need FULL access during programming but must be HIDDEN WELL in presentation mode. The system should support graduated visibility where some controls are visible and others hidden depending on context.

### Current Implementation (ProgrammingMode)

**Code location:**
- `src/ui/ProgrammingMode.h` (31 LOC)
- `src/ui/ProgrammingMode.cpp` (64 LOC)
- **Total: 95 LOC**

**What exists:**

1. **ProgrammingMode component** (ProgrammingMode.h/cpp):
   - Owns a reference to `SignalBar`
   - `setActive(true)` sets SignalBar to `DisplaySize::Expanded`, shows itself
   - `setActive(false)` sets SignalBar to `DisplaySize::Normal`, hides itself
   - Renders a dark overlay with header label "Programming Mode" and "Exit" button
   - Has NO content of its own -- just overlay chrome around the expanded SignalBar

2. **MainComponent integration** (MainComponent.cpp):
   - Line 511-512: Creates ProgrammingMode, adds as CHILD component (addChildComponent = starts hidden)
   - Line 1312-1313: In `resized()`, **always sets `programmingMode_->setVisible(false)`** -- the ProgrammingMode component overlay is NEVER displayed
   - Line 3199-3210: View menu handler for `kViewProgrammingMode` -- toggles SignalBar expanded/normal **directly**, bypassing ProgrammingMode entirely
   - Line 1316-1322: When SignalBar is expanded, hides ALL other panels: preview, waveform, audio readout, spectrum, effects rack

3. **The toggle mechanism** (MainComponent.cpp:3199-3210):
   ```
   SignalBar expanded? -> set Normal
   SignalBar normal?   -> set Expanded
   When expanded:      -> hide previewPanel, waveformDisplay, audioReadoutPanel, spectrumDisplay, effectsRackPanel
   ```

### Analysis: What IS vs What ISN'T

**What IS:**
- A binary toggle: Normal view OR expanded SignalBar view
- In expanded mode: SignalBar fills available space, all other panels hidden
- Triggered via View menu > Programming Mode

**What IS NOT:**
- No graduated mode (some controls visible, others hidden)
- No per-component mode behavior (only SignalBar responds to the mode)
- No presentation mode (everything except output hidden)
- No programmable visibility per panel/section
- ProgrammingMode component is VESTIGIAL -- created but never shown (`setVisible(false)` in `resized()` at line 1313, and the View menu toggle at line 3199 operates on SignalBar directly, never calling `programmingMode_->setActive()`)

### Specific Code Evidence

| File | Line | Evidence |
|------|------|---------|
| ProgrammingMode.cpp | 3 | Constructor: takes SignalBar reference, creates header + close button |
| ProgrammingMode.cpp | 15-36 | `setActive()`: toggles SignalBar display size, shows/hides self, triggers parent layout |
| MainComponent.cpp | 511 | `programmingMode_ = std::make_unique<ProgrammingMode>(*signalBar_)` |
| MainComponent.cpp | 512 | `addChildComponent(programmingMode_.get())` (starts invisible) |
| MainComponent.cpp | 1313 | `programmingMode_->setVisible(false)` -- ALWAYS hidden in resized() |
| MainComponent.cpp | 3199-3210 | View menu toggles SignalBar directly, never calls `programmingMode_->setActive()` |
| MainComponent.cpp | 1316-1322 | When expanded: hides preview, waveform, readout, spectrum, effects rack |

### Verdict

The dual-mode system that Boris considers "THE core design tension" does not exist as designed. The current implementation is a binary fullscreen-SignalBar toggle. The ProgrammingMode component (95 LOC) is vestigial code -- instantiated but never activated. The actual mode switching is done inline in MainComponent.cpp by toggling SignalBar display size and hiding other panels.

**What works regardless of mode (always running):**
- Keyboard bindings (BindingManager)
- MIDI input (MidiHandler)
- OSC input (OscHandler)
- REST API (port 7070, always on)
- Audio engine, analysis pipeline, render pipeline (background threads)
- Output window, Syphon (independent windows)
- Autopilot (render thread)
- Video recording (independent thread)

**Activation Estimate for full dual-mode:** HIGH (~1 week+) — requires designing the mode system (what is visible when), adding per-component visibility flags, creating a presentation mode layout, and potentially a mode configuration UI. The ProgrammingMode component could serve as a starting point but needs complete redesign.
