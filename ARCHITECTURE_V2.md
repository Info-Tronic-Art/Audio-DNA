# Audio-DNA v2: System Architecture & Design Document

> Complete redesign from keyboard-grid VJ tool to Resolume-class clip/layer/signal performance system. This document is the single source of truth for the v2 architecture.

---

## 1. Product Identity

Audio-DNA v2 is a cross-platform desktop application (C++20 / JUCE / OpenGL) for live audio-reactive visual performance. It combines the deepest real-time audio analysis engine in any VJ tool (42+ features, automatic BPM lock, automatic downbeat detection) with a Resolume-style clip/layer/column deck system, universal signal routing, and procedural content generation.

**What changed from v1**: The fixed 4×10 keyboard grid is replaced by a flexible layer×column deck. The mapping popup is replaced by universal per-parameter signal routing. Audio features, oscillators, and envelopes are unified as "Signals" in a mixer-style Signal Bar. Macros aggregate signal-to-parameter connections. The entire UI is restructured around the deck as the central workspace.

---

## 2. Terminology

| Term | Definition |
|------|-----------|
| **Signal** | Any value stream — audio features (Volume, Bass, Beat Position), oscillators, envelopes. Displayed in the Signal Bar. |
| **Signal Bar** | Horizontal strip across the top of the app containing all active Signals as vertical meter strips. |
| **Macro** | An intermediary knob (6 per scope: clip/layer/global) that aggregates signals and distributes to multiple parameters. Lives in the Inspector, NOT in the Signal Bar. |
| **Route** | A connection from a Signal or Macro to a parameter, with range/invert/dial-range settings. |
| **Binding** | A keyboard key or MIDI note/CC assigned to an app function (trigger clip, adjust parameter, etc.). |
| **Composition** | The complete app state saved to disk — all decks, routes, bindings, settings. |
| **Deck** | The clip grid: layers × columns. Multiple decks can be saved within a composition. |
| **Layer** | A row in the deck. Types: Opaque, Transparent, FX Only, 3D, Mask. One clip active per layer at a time. |
| **Clip** | Media (image/video/camera/procedural source) + per-clip effects + transport + autopilot, placed in a deck cell. |
| **Column** | A vertical slice of the deck. Triggering a column activates clips in that column across all layers. Empty cells clear the layer. |
| **FX Preset** | A saved parameter state for a specific effect. Nested under the effect in the FX browser. |
| **Source** | A procedural/algorithmic content generator (fractal, noise, geometry, etc.) that can be placed in a clip slot. |

---

## 3. Application Layout

```
┌─────────────────────────────────────────────────────────────────────────────────┐
│ MENU BAR                                                                        │
│ [Audio-DNA] [Composition] [Deck] [Layer] [Column] [Clip] [Output] [Shortcuts]  │
│ [View]                                                                          │
├─────────────────────────────────────────────────────────────────────────────────┤
│ TOP BAR                                                                         │
│ Audio:[Mic▼] Gain:[═══] │ ▶ ⏸ ■ │ Tempo:128 [Tap] [Resync]                   │
│ [÷4][÷2][×1][×2][×4] Quant:[Beat▼] │ Fade:[═══] │ FPS:60 DSP:2%              │
├─────────────────────────────────────────────────────────────────────────────────┤
│ SIGNAL BAR (full width, 3 display sizes: minimized/normal/expanded)            │
│ ┌──┐┌──┐┌──┐┌──┐┌──┐┌──┐┌──┐┌──┐┌──┐┌──┐┌──┐┌──┐                    [+]    │
│ │∿ ││▓▓││▓▓││▓▓││▓ ││  ││▓▓││▓▓││1 ││▓▓││▓ ││BU││∿ ││╱╲│                    │
│ Wav Vol Sub Bas Mid Air Tmp Beat BtBr Bar Hit Stat M1  M2                      │
├─────────────────────────────────────────────────────────────────────────────────┤
│ DECK (layers × columns)                                                         │
│      [▼1] [▼2] [▼3] [▼4] [▼5] [▼6] [▼7] [▼8] [▼9] [▼10][▼11][▼12]          │
│ ┌──────────────────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┬────┐    │
│ │L3 [XBSMAV][FX▼]  │     │     │     │     │     │     │     │     │    │    │
│ │   [⊙]opa [◀⏸▶]   │     │     │     │     │     │     │     │     │    │    │
│ ├──────────────────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┼────┤    │
│ │L2 [XBSMAV][Trn▼] │     │     │     │     │     │     │     │     │    │    │
│ │   [⊙]opa [◀⏸▶]   │     │     │     │     │     │     │     │     │    │    │
│ ├──────────────────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┼─────┼────┤    │
│ │L1 [XBSMAV][Opq▼] │     │     │     │     │     │     │     │     │    │    │
│ │   [⊙]opa [◀⏸▶]   │     │     │     │     │     │     │     │     │    │    │
│ └──────────────────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┴─────┴────┘    │
│ [Deck 1: "Opener"] [Deck 2: "Build"] [Deck 3: "Drop"] [+]                     │
├───────────────────────┬─────────────────────┬───────────────────────────────────┤
│ PREVIEW / OUTPUT      │ INSPECTOR           │ BROWSER                           │
│ [Preview] [Output]    │ [Clip][Layer][Comp] │ [Files][FX][Sources][Comp/Decks]  │
│                       │ [Signal]            │ [Record]                          │
│ ┌───────────────────┐ │                     │                                   │
│ │ Composition       │ │ (context-sensitive  │ ┌─────────────────────────┐       │
│ │ Monitor           │ │  based on selected  │ │ Browsable content       │       │
│ │                   │ │  tab and item)      │ │                         │       │
│ │                   │ │                     │ │ Drag from here into     │       │
│ └───────────────────┘ │                     │ │ deck cells or effect    │       │
│                       │                     │ │ stacks in inspector     │       │
│ Tempo: 128 [LOCKED]   │                     │ └─────────────────────────┘       │
└───────────────────────┴─────────────────────┴───────────────────────────────────┘
```

---

## 4. Component Architecture

### 4.1 Signal Bar

A horizontal strip of **vertical meter strips**, one per active Signal. Positioned below the top bar, spanning full window width.

**Three display sizes** (user toggleable):

**Minimized** (~20px tall): Square indicator + value + name. Flashes on trigger based on threshold settings. Blank when not triggering.
```
┌──┐┌──┐┌──┐┌──┐
│▓▓││  ││▓▓││BU│
│.72││--││128││  │
│Vol││Pk││Tmp││St│
└──┘└──┘└──┘└──┘
```

**Normal** (~80px tall): Vertical meter bar with value readout at top, name at bottom. Click a strip → Inspector switches to Signal tab showing that signal's full settings.
```
┌──┐
│  │ 0.72
│▓▓│
│▓▓│
│▓▓│
│  │
└──┘
 Vol
```

**Expanded / Programming Mode** (fills downward, can take full screen): Each signal becomes a full column showing its meter + all parameters it routes to + controls for editing. [+ Add Signal] button to add from dropdown. Only configured signals appear — keeps it clean.

**Default signals on fresh launch** (12):
```
Waveform, Volume, Sub, Bass, Mid, Air, Tempo, Beat Position, Beat In Bar,
Bar Position, Hit, Energy State, Mod 1 (Sine 1-beat), Mod 2 (Envelope linear)
```

**Signal categories** (collapsible groups):
- **Amplitude**: Volume, Peak, Punch, Hits Per Second
- **Bands**: Sub, Bass, Low Mid, Mid, High Mid, Presence, Air
- **Rhythm**: Tempo, Beat Position, Beat In Bar, Bar Position, Downbeat, Phrase Position, Tracker State, Hit, Hit Strength
- **Pitch**: Note, Note Confidence, Key, Chord Change
- **Chroma**: C, C#, D, D#, E, F, F#, G, G#, A, A#, B (collapsed by default)
- **Timbre**: Timbre 1-13 (collapsed by default)
- **Structure**: Energy State
- **Modulation**: User-created oscillators and envelopes (6 default, add/remove)

**Readout format per signal type**:
| Signal Type | Readout Format | Meter Behavior |
|-------------|---------------|----------------|
| Continuous (Volume, Bass) | `0.72` | Proportional fill |
| BPM | `128` | Solid when locked |
| Beat Position | `0.75` | Animated sawtooth |
| Beat In Bar | `1` / `2` / `3` / `4` | Highlight current |
| Note | `C#4` | Pitch name |
| Key | `A Minor` | Key name |
| Energy State | `Buildup` | State name |
| Hit | `●` / `○` | Flash on trigger |
| Tracker State | `LOCKED` | Green/yellow/red |
| Oscillator | `∿ 0.65` | Animated wave |
| Envelope | `╱╲ 0.30` | Animated curve |

### 4.2 Deck

A Resolume-style grid of **Layers (rows) × Columns**. Default: 3 layers, 12 columns. Layers can be added/removed (unlimited). Columns can be added/removed.

**Column triggers** sit at the top of each column. Clicking a column trigger activates all clips in that column. Empty cells in a triggered column clear/stop whatever is playing on that layer.

**Quantize options** (top bar dropdown):
- Off (immediate trigger)
- Next Beat
- Next Downbeat (bar-aware, waits for beat 1)

**Deck tabs** at the bottom for quick-switching between saved decks.

#### Clip Cell

Each clip cell has two interaction zones:
```
┌────────────────┐
│                │
│   THUMBNAIL    │  ← Click = trigger/retrigger clip. ~80×60px.
│                │
├────────────────┤
│ clip name      │  ← Click = select for inspection (no trigger).
└────────────────┘     Right-click = context menu. Drag = move/duplicate.
```

- **Has media**: Shows thumbnail of image/video/source
- **Has FX only (no media)**: Shows top effect's icon
- **Empty**: Dark/blank cell
- Click same clip again = retrigger from start
- Click empty cell = stop/clear that layer
- Option-drag from name bar = duplicate
- ⌘X = cut, ⌘C/⌘V = copy/paste

#### Layer Strip (Left Side of Each Row)

Compact controls, one row per layer (~45px tall):

```
┌──────────────────────────────────────────────────────────────────┐
│ [X][B][S] [M][A][V]  [Opq▼]  [⊙] opacity  [◀⏸▶]  clip name   │
│ Layer 1                                                          │
└──────────────────────────────────────────────────────────────────┘
```

| Control | Size | Purpose |
|---------|------|---------|
| X | 20×20 | Clear playing clip from layer |
| B | 20×20 | Bypass layer output |
| S | 20×20 | Solo (show only this layer) |
| M | 20×20 | Mute audio from clips on this layer |
| A | 20×20 | Autopilot on/off |
| V | 20×20 | Visible toggle |
| Type ▼ | dropdown | Opaque / Transparent / FX Only / 3D / Mask |
| ⊙ | 24×24 | Opacity knob with mini preview |
| ◀⏸▶ | 60×20 | Transport (reverse, pause, play) |
| clip name | text | Currently active clip name |

Click layer strip → Inspector switches to Layer tab.

#### Layer Types

Each type changes what controls appear in the **Layer Inspector tab**:

| Type | Behavior | Extra Inspector Controls |
|------|----------|-------------------------|
| **Opaque** | One clip at a time, new replaces old | — |
| **Transparent** | One clip at a time, composited over layers below | Blend mode (17 modes), Keying mode (13 modes) + threshold/softness |
| **FX Only** | Clips contain only effects applied to layers below | Dry/Wet mix |
| **3D** | Clips are 3D models/surfaces | Rotation X/Y/Z, Rotation Speed, Scale |
| **Mask** | Content becomes alpha mask for layers below | — |

### 4.3 Inspector (Center-Bottom Panel)

Four tabs, always accessible:

```
[Clip] [Layer] [Composition] [Signal]
```

Auto-switches based on what user last clicked:
- Click a clip → Clip tab
- Click a layer strip → Layer tab
- Click a signal strip → Signal tab
- Composition tab always available manually

#### Clip Inspector Tab

| Section | Contents |
|---------|----------|
| **Macros** | 6 knobs, each with source picker button (select signal or manual) |
| **Transport** | Mode dropdown (Timeline/BPM Sync), scrubber, ◀⏸▶, loop mode, direction, speed, duration with ÷2/×2 |
| **Autopilot** | Action dropdown (Layer Determined / Do Nothing / Play Next / Previous / Random / First / Last / Specific), Duration dropdown |
| **Beat Snap** | On/Off (default On for video clips) |
| **Effects Stack** | Draggable list, each effect: [B] bypass + [icon] + name + main value. Click to expand inline showing all parameters with universal control widget. Each FX has presets nested below it. |
| **Cuepoints** | 8 slots for saved video positions |

#### Layer Inspector Tab

| Section | Contents |
|---------|----------|
| **Macros** | 6 knobs for this layer |
| **Blend Mode** | Dropdown (17 modes, only for Transparent type) |
| **Keying** | Mode dropdown (13 modes) + threshold/softness/chroma controls (only for Transparent type) |
| **Layer Effects** | Effect stack applied to any active clip on this layer |
| **Autopilot Defaults** | Action + Duration defaults for clips choosing "Layer Determined" |
| **Transition Speed** | Per-layer crossfade speed (global sets default, layer overrides) |
| **3D Controls** | Rotation X/Y/Z, speed, scale (only for 3D type) |

#### Composition Inspector Tab

| Section | Contents |
|---------|----------|
| **Global Macros** | 6 knobs for composition-wide control |
| **Global Effects** | Effect chain applied after all layers composite |
| **Master Opacity** | — |
| **Transition Speed** | Global default (changing updates all linked layers) |
| **Output Settings** | Resolution, display selection |

#### Signal Inspector Tab

Shows full settings for the selected signal:
- For **audio signals**: Threshold, Gain, Falloff (per-route settings displayed here for reference)
- For **oscillators**: Wave shape (sine/saw up/saw down/triangle/square), Beat duration (1/4, 1/2, 1, 2, 4, 8), Amplitude, Phase offset
- For **envelopes**: Curve editor with draggable control points, Phase, Octaves, Curve type (Linear/Exponential/etc.), One-shot vs looping
- **Routes list**: Where this signal is connected (which parameters it drives)

### 4.4 Browser (Right-Bottom Panel)

Five tabs plus Record:
```
[Files] [FX] [Sources] [Comp/Decks] [Record]
```

#### Files Tab
- Folder navigation with path bar, search, favorites heart icon, list/grid toggle
- Shows images, videos with thumbnails
- Multi-select drag onto deck cells (auto-fills columns sequentially)

#### FX Tab
- Effect icons organized by category (Warp, Color, Glitch, Blur, Pattern, Animation, Blend, 3D/Depth)
- Each effect expandable to show FX Presets nested below
- Drag effect onto clip cell or into inspector effect stack
- Drag FX Preset to load with saved settings
- Each effect has a hand-designed icon

#### Sources Tab
- Procedural generators organized by category (Fractal, Geometric, Noise, Lighting, Audio-Visual, Nature, Text, 3D)
- 40 generators (10 Tier 1 for launch), each with auto-generated thumbnail
- Drag onto clip cell to use as clip content

#### Comp/Decks Tab
- Saved compositions (click to load full composition)
- Saved decks (click to switch deck in current composition)

#### Record Tab
- Settings recording controls (start/stop, playback)
- Recording output directory
- Recording format settings
- Video recording (later phase)

### 4.5 Preview / Output (Left-Bottom Panel)

Two tabs:
```
[Preview] [Output]
```

**Preview**: Context-sensitive display:
- Nothing selected → full composition output (all layers composited)
- Clip selected → that clip rendered solo with its effects
- Layer selected → that layer's output solo

**Output**: Shows what goes to external display. Has display selector if no external display active.

---

## 5. Universal Parameter Control

**Every adjustable value in the entire app** uses the same control widget:

```
Collapsed:
[Label]  [0.50]  [-] [+]  [═══════╪═══════]

Expanded (click to expand):
  [Source button] → dropdown:
    Manual              — direct slider value
    Signals:
      Volume, Sub, Bass, Mid, Air, ...
      Tempo, Beat Position, Beat In Bar, Bar Position, ...
      Note, Key, Chord Change, ...
      Mod 1, Mod 2, ... (oscillators/envelopes)
    Macros:
      Clip Macro 1-6
      Layer Macro 1-6
      Global Macro 1-6

  [Invert] checkbox
  [Range] min/max sliders — output range of parameter
  [Dial Range] min/max — input sensitivity range

  If source is Envelope:
    [Curve editor with draggable control points]
    Phase, Octaves, Curve type
```

This applies to:
- Clip effect parameters
- Layer controls (opacity, blend amount, keying threshold, transport speed)
- Global effect parameters
- Macro knob values (a macro can be driven by a signal)
- Any continuous control anywhere

### Effects in Inspector — Collapsed/Expanded

```
Collapsed (single line):
┌─────────────────────────────────────────┐
│ [B] [icon] Ripple                  0.72 │
└─────────────────────────────────────────┘

Expanded (click to expand):
┌─────────────────────────────────────────┐
│ [B] [icon] Ripple                       │
│   Frequency  0.50  [-][+] [═══╪══════] │
│     ← Volume ▓▓▓▓▓░░░░░░ (source viz)  │
│   Amplitude  0.72  [-][+] [══════╪═══] │
│     ← Beat Position ▓▓▓▓▓▓▓░░░ (viz)  │
│   Speed      0.30  [-][+] [══╪═══════] │
│     ← Manual                            │
└─────────────────────────────────────────┘
```

Each parameter shows its current source and a small inline visualization of how that source drives the value.

---

## 6. Macro System

6 Macro knobs per scope (Clip, Layer, Global). Displayed in the Inspector, NOT in the Signal Bar.

Each Macro has:
- A **source picker button** — click to select which Signal feeds it (or Manual)
- The knob itself — adjustable value
- **Links** to multiple parameters, each with individual Invert, Range, and Dial Range

Flow:
```
Signal (Bass) ──→ Macro 1 ──→ Parameter A (Ripple.freq, range 0-0.8)
                           ──→ Parameter B (Blur.radius, inverted, range 0-0.5)
                           ──→ Parameter C (Layer opacity, range 0.2-1.0)
```

Macros can also be selected as sources when routing any parameter. So a parameter's source dropdown shows both Signals and Macros.

---

## 7. Rendering Pipeline

```
For each layer (bottom to top, respecting layer order):
  Active clip media → Clip Effects → Clip Keying (if Transparent layer) →
  Layer Effects → Layer Blend Mode → Layer Opacity →
  Accumulator FBO

Mask layers: content becomes alpha mask applied to accumulator before next layer

Global Effects → Master Opacity → Screen / Fullscreen Output
```

FX Only layers: clip content (effects) applied to the current accumulator state.

3D layers: clip content rendered as 3D geometry with rotation/scale, then composited.

---

## 8. BPM / Rhythm System

### 8.1 BPM Stabilization Pipeline

Applied inside `BPMTracker` on top of aubio's raw output. No new dependencies. All buffers pre-allocated (~512 bytes).

```
aubio_tempo_get_bpm()  [raw, jittery]
        │
        ▼
   BPM Range Gate  [reject < 60 or > 200, fold via halving/doubling]
        │
        ▼
   Confidence Gate  [reject if confidence < threshold; hold last good value]
        │
        ▼
   Octave Error Correction  [snap to locked octave if within 2x/0.5x]
        │
        ▼
   Median Filter  [window of 48 estimates (~500ms)]
        │
        ▼
   Hysteresis Lock  [only change if new median persists ~2-4 seconds]
        │
        ▼
   snapshot.bpm  [stable, locked]
```

### 8.2 Beat Phase — Free-Running Sawtooth

Driven by locked BPM, not aubio's raw period. Hard-resets on high-confidence beat detections:

```cpp
float lockedPeriodSamples = (sampleRate_ * 60.0f) / lockedBPM_;
phase_ += hopSize_ / lockedPeriodSamples;
if (phase_ >= 1.0f) phase_ -= 1.0f;
if (beatDetected && confidence > 0.5f) phase_ = 0.0f;
```

### 8.3 Downbeat Detection

No commercial VJ software does this from live audio. We score each beat using existing features:

```
downbeatScore = 0.5 * bassEnergy + 0.3 * spectralFlux + 0.2 * harmonicChange
```

Track scores over 4-beat windows. Lock downbeat position after 8+ consistent beats.

### 8.4 New FeatureSnapshot Fields

```cpp
// Stabilized (existing fields, improved):
float    bpm;              // Locked via pipeline
float    beatPhase;        // Free-running smooth sawtooth

// NEW — Metrical hierarchy:
uint8_t  beatInBar;        // 0-3 (0 = downbeat)
float    barPhase;         // [0,1) over 4 beats
bool     downbeatDetected; // trigger on beat 1
uint8_t  trackerState;     // 0=searching, 1=locking, 2=locked

// NEW — Phrase (Phase 4):
uint16_t barCount;         // bars since last phrase reset
float    phrasePhase;      // [0,1) over N bars
```

### 8.5 BPM Divisor/Multiplier

Top bar controls: `[÷4] [÷2] [×1] [×2] [×4]`

The BPM engine always tracks the TRUE tempo. The multiplier scales how oscillators, autopilot, quantize, and rhythm signals interpret beats. At ×2, a "1 beat" oscillator completes in half the time.

### 8.6 Tracker State in UI

```
SEARCHING:  Tempo: ---  (red, no reliable BPM)
LOCKING:    Tempo: ~128 (yellow, candidate being confirmed)
LOCKED:     Tempo: 128  (green/cyan, solid)
```

### 8.7 Resync Button

Resets: beat phase to 0, downbeat detector, bar/phrase counters. Manual fallback for when automatic detection misaligns.

---

## 9. Audio Feature Labels (Intuitive Names)

| Internal Name | UI Label | Category |
|---------------|----------|----------|
| rms | Volume | Amplitude |
| peak | Peak | Amplitude |
| dynamicRange | Punch | Amplitude |
| transientDensity | Hits Per Second | Amplitude |
| spectralCentroid | Brightness | Spectral |
| spectralFlux | Change | Spectral |
| spectralFlatness | Noisiness | Spectral |
| spectralRolloff | Treble Cutoff | Spectral |
| bandEnergies[0] | Sub Bass | Bands |
| bandEnergies[1] | Bass | Bands |
| bandEnergies[2] | Low Mid | Bands |
| bandEnergies[3] | Mid | Bands |
| bandEnergies[4] | High Mid | Bands |
| bandEnergies[5] | Presence | Bands |
| bandEnergies[6] | Air | Bands |
| bpm | Tempo | Rhythm |
| beatPhase | Beat Position | Rhythm |
| beatInBar | Beat In Bar | Rhythm |
| barPhase | Bar Position | Rhythm |
| downbeatDetected | Downbeat | Rhythm |
| phrasePhase | Phrase Position | Rhythm |
| trackerState | Tracker State | Rhythm |
| onsetDetected | Hit | Rhythm |
| onsetStrength | Hit Strength | Rhythm |
| dominantPitch | Note | Pitch |
| pitchConfidence | Note Confidence | Pitch |
| detectedKey | Key | Pitch |
| harmonicChangeDetection | Chord Change | Pitch |
| mfccs[0] | Timbre: Energy | Timbre |
| mfccs[1] | Timbre: Balance | Timbre |
| mfccs[2-12] | Timbre 3-13 | Timbre |
| chromagram[0-11] | C, C#, D, ... B | Chroma |
| structuralState | Energy State | Structure |

---

## 10. Modulation Bank

6 modulation signals by default (add/remove freely). Each can be an **Oscillator** or **Envelope**.

**Oscillator**: Continuous repeating waveform, BPM-locked.
- Wave shape: Sine, Saw Up, Saw Down, Triangle, Square
- Beat duration: 1/4, 1/2, 1, 2, 4, 8 beats
- Amplitude: [0, 1]
- Phase offset: [0°, 360°]
- Period is locked to detected BPM. Changes when BPM changes.

**Envelope**: Custom curve, BPM-locked or free-running.
- Curve editor with draggable control points
- Phase, Octaves, Curve type (Linear, Exponential, S-Curve)
- One-shot or looping
- Amplitude, Density

Modulation signals appear in the Signal Bar as vertical meter strips, just like audio signals. Click to edit in the Signal Inspector tab.

Modulation signal presets can be saved/loaded with meaningful names (e.g., "Gentle Pulse", "Hard Gate").

---

## 11. Autopilot System

### Per-Clip Autopilot
- **Action**: Layer Determined / Do Nothing / Play Next / Play Previous / Play Random / Play First / Play Last / Play Specific Clip
- **Duration**: Layer Determined / 1 Beat / 2 Beats / 4 Beats (1 Bar) / 8 Beats (2 Bars) / 16 Beats (4 Bars) / 32 Beats (8 Bars) / Custom

### Per-Layer Autopilot
Sets defaults for clips that choose "Layer Determined."

Bar-aware durations use the downbeat detector — clip changes align to beat 1 when possible.

---

## 12. Binding System

### Keyboard Binding Mode
1. Enter binding mode (Shortcuts → Edit Keyboard, ⇧⌘K)
2. Click any bindable UI element
3. Press a physical key → binding created
4. Visual indicator shows bound key on/near element
5. Exit binding mode

### MIDI Learn Mode
1. Enter MIDI learn mode (Shortcuts → Edit MIDI, ⇧⌘M)
2. Click any bindable UI element
3. Send MIDI note/CC → binding created
4. Notes → triggers (clips, columns), CCs → continuous controls (sliders, knobs)

### Bindable Targets
- Individual clips (trigger)
- Columns (trigger)
- Layer controls (B, S, M, A, V, transport)
- Effect toggles (bypass)
- Macro knobs
- Deck switching
- Tap tempo, Resync
- Global transport (play/pause/stop)
- Master opacity

---

## 13. Transition System

- **Crossfade** between clips on the same layer when switching
- **Global transition speed** slider in top bar — sets default for all layers
- **Per-layer transition speed** in Layer Inspector — overrides global
- Moving global updates all layers. Moving individual layer only updates that layer.
- Speed = 0: hard cut (instant). Speed > 0: smooth crossfade over that duration.

---

## 14. Column Triggering

- Click column trigger button → activates clips in that column across all layers
- **Clips present**: Load and play
- **Empty cells**: Stop/clear whatever is playing on that layer
- **Quantize**: If on, waits for next beat/downbeat boundary
- **Beat Snap**: Video clips snap playhead to current beat position in the clip
- Active column gets a subtle vertical highlight

---

## 15. Clip Trigger Behavior

- **Click clip thumbnail**: Load and play from start. Replaces previous clip on that layer.
- **Click same clip again**: Retrigger from start (restart)
- **Click empty cell**: Stop/clear that layer
- **Column trigger**: Same rules — clips play, empty cells clear
- No toggle mode. One behavior. Predictable.

---

## 16. Layer Management

All via top menu (Layer menu) or keyboard shortcuts:
- New (⌘L), Insert Above, Insert Below, Duplicate, Rename, Remove
- Copy Effects, Paste Effects
- Clear Clips
- Ignore Column Trigger
- Mask Mode submenu
- No right-click context menu on layer strip — click = select for inspection

---

## 17. Undo/Redo

Architected from the start using Command pattern.

Every state change is a Command object with `execute()` and `undo()`. History stack with index. ⌘Z / ⇧⌘Z.

**Tracked** (undoable):
- Parameter value changes
- Clip assignments/removals
- Effect add/remove/reorder/bypass
- Layer add/remove/reorder
- Route creation/deletion
- Signal configuration changes
- Macro link changes

**Not tracked** (performance actions):
- Clip triggering
- Column triggering
- Audio source changes
- Output display selection

The command stream also serves as the foundation for **settings recording**.

---

## 18. Recording

### Phase 1: Settings Recording
- Record all parameter changes, clip triggers, column triggers as timestamped events
- Low processing overhead — event logging only
- Playback reproduces the performance exactly
- Stored as timestamped JSON or compact binary

### Phase 2: Video Recording (Later)
- Capture rendered output as video file
- HAP Alpha codec for alpha support
- Configurable resolution, codec, format

---

## 19. Procedural Sources

40 generators across 9 categories (see `research/SOURCES_procedural_generators.md`). 10 Tier 1 for launch:

| Source | Category | GLSL-only |
|--------|----------|-----------|
| Perlin Noise Field | Noise | Yes |
| Plasma / Color Cycling | Noise | Yes |
| Voronoi Cells | Geometric | Yes |
| Kaleidoscopic Fractal | Fractal | Yes |
| Mandelbrot / Julia | Fractal | Yes |
| Geometric Tunnel | 3D | Yes |
| Reaction-Diffusion | Simulation | Ping-pong FBO |
| Audio Waveform | Audio-Visual | 1D texture upload |
| Particle System | Simulation | CPU + texture |
| Color Gradient | Utility | Yes |

108 sources total (as of P25). Most are single fragment shaders; ping-pong FBO sources include Strange Attractor, Gravity Well, Fluid Dynamics; CPU-computation sources include Layer Router.

---

## 20. Preferences Dialog

```
General:     Clip panel behavior, clip start offset, scrolling, updates, quit confirmation, splash
Audio:       Input device, sample rate, buffer size, FFT input gain, BPM detection range
Video:       Render resolution, FPS target, texture quality
MIDI:        Device list, refresh, middle C convention
Recording:   Output directories, format, codec
Defaults:    Default transport, play mode, blend mode, tempo nudge range
Feedback:    Name, email, feedback text, log inclusion
About:       Version, credits
```

---

## 21. Menu Bar

```
[Audio-DNA]   Preferences, About, Quit
[Composition] New, Open, Open Recent, Save, Save As, Media Manager, Copy/Paste Effects,
              Global Transport, Beat Snap, Settings
[Deck]        New, Insert Before/After, Duplicate, Rename, Close, Clear Clips, Remove
[Layer]       New, Insert Above/Below, Duplicate, Rename, Copy/Paste Effects, Fold,
              Clear Clips, Remove, Ignore Column Trigger, Mask Mode, Lock Content
[Column]      New, Insert Before/After, Duplicate, Rename, Clear Clips, Remove,
              Remove All Before/After
[Clip]        Beat Snap, Transport, Trigger Style, Ignore Column Trigger,
              Select All, Cut/Copy/Paste, Copy/Paste Effects, Rename, Clear,
              Show in Finder, Snapshot, Render to File, New Source, New Effect
[Output]      Disabled, Fullscreen (per display), Windowed, Advanced,
              Texture Sharing (Syphon), NDI, Identify Displays, Test Card, Snapshot
[Shortcuts]   Edit Keyboard, Edit MIDI, Stop
[View]        Show/Hide: Signal Bar, Deck, each Inspector tab, each Browser tab,
              FPS & Stats, Programming Mode, Layout save/load
```

---

## 22. Keying Modes (13)

Applied per-clip or per-layer (Transparent type):

1. Alpha (source alpha channel)
2. Luma Key (dark → transparent)
3. Inverted Luma Key (bright → transparent)
4. Luma is Alpha (luminance = opacity)
5. Inverted Luma is Alpha
6. Chroma Key (specific color → transparent)
7. Max RGB (max(R,G,B) as alpha)
8. Saturation Key (saturated = opaque)
9. Edge Detection (edges = opaque)
10. Threshold 50% (binary cutoff)
11. Channel: Red
12. Channel: Green
13. Channel: Blue

Vignette/Spotlight was in v1 but can be achieved with a Mask layer + gradient source.

---

## 23. Blend Modes (17)

Applied per-layer (Transparent type):

**Normal**: Normal (standard alpha blend)
**Lighten**: Additive, Screen, Lighten, Color Dodge
**Darken**: Multiply, Darken, Color Burn
**Contrast**: Overlay, Soft Light, Hard Light, Vivid Light, Linear Light, Pin Light, Hard Mix
**Inversion**: Difference, Exclusion

Default: **Additive** (VJ standard).

---

## 24. Video Codec

**Implemented** (P11/P22): H.264 (default), ProRes, MJPEG via FFmpeg libavcodec. HAP Alpha is not implemented.
**Fallback**: PNG sequence (excellent random access, universal)

Alpha channel in video clips → layer keying can use "Alpha" mode (source alpha).

---

## 25. Data Model Hierarchy

```
Composition (saved to disk — everything)
├── Deck 1 ("Opener")
│   ├── Layer 1 (type: Opaque)
│   │   ├── Layer Controls: opacity, blend, keying, transport, autopilot defaults
│   │   ├── Layer Macros: 6 knobs
│   │   ├── Layer Effects: chain applied to any active clip
│   │   ├── Clip [Col 1]: media + clip effects + clip macros + transport + autopilot + cuepoints
│   │   ├── Clip [Col 2]: ...
│   │   └── Clip [Col N]: ...
│   ├── Layer 2 (type: Transparent)
│   │   └── ...
│   └── Layer N
├── Deck 2 ("Build")
│   └── ...
├── Routes (signal/macro → parameters at clip/layer/global level)
├── Bindings (keyboard/MIDI → functions)
├── Signal Configuration (which signals are active, modulation settings)
├── Global Effects (post-composite chain)
├── Global Macros: 6 knobs
├── FX Presets (per-effect saved states)
└── Settings (audio source, resolution, output, transition speed, BPM multiplier)
```

---

## 26. Thread Model (Unchanged from v1)

The 4-thread model remains:

1. **Audio Callback** (RT priority): Mono downmix → SPSC ring buffer. <100μs.
2. **Analysis Thread** (above-normal): Ring buffer → FFT → all features → FeatureSnapshot → triple buffer. <2ms per hop.
3. **Render Thread** (normal, VSync): Read snapshot → apply routes → render layers → composite → display. <8ms.
4. **Message Thread** (JUCE UI): All UI interaction → atomic config writes.

### New in v2
- Analysis thread also runs BPM stabilization pipeline and downbeat detection
- Render thread runs the new Routing Engine (replaces MappingEngine)
- Render thread handles multi-layer compositing (replaces single-image EffectChain)
- Message thread manages the new deck/inspector/browser UI

---

## 27. Effect Icons — Specification for AI Generation

Each of our 135 effects needs a 64×64px icon. Style: monochrome cyan lines/shapes on transparent/dark background, consistent with our VJ dark theme.

### Warp Effects (16)

| Effect | Icon Description | AI Prompt |
|--------|-----------------|-----------|
| Ripple | Concentric circles radiating from center | "Minimalist icon of concentric water ripple circles, cyan lines on dark background, 64x64, clean vector style" |
| Bulge | Circle with magnified/expanded center | "Minimalist icon of a convex lens bulge distortion, center expanded outward, cyan on dark, 64x64" |
| Wave | Horizontal sine wave displacing a grid | "Minimalist icon of a grid with horizontal sine wave distortion, cyan lines on dark, 64x64" |
| Liquid | Turbulent flowing distortion lines | "Minimalist icon of turbulent liquid flow displacement, wavy distorted lines, cyan on dark, 64x64" |
| Kaleidoscope | Radial symmetry pattern | "Minimalist icon of kaleidoscope radial symmetry, 6-fold pattern, cyan on dark, 64x64" |
| Fisheye | Barrel distortion of a grid | "Minimalist icon of fisheye barrel distortion on a grid, center bulging, cyan on dark, 64x64" |
| Swirl | Spiral twisting from center | "Minimalist icon of a spiral swirl vortex from center, cyan on dark, 64x64" |
| Polar Coords | Rectangular to polar transformation | "Minimalist icon of polar coordinate transformation, rectangle bending into circle, cyan on dark, 64x64" |
| Twirl | Rotational twist around center | "Minimalist icon of rotational twirl distortion, twisted square, cyan on dark, 64x64" |
| Shear | Parallelogram skew of a square | "Minimalist icon of shear transformation, skewed parallelogram from square, cyan on dark, 64x64" |
| Elastic Bounce | Springy bouncing distortion | "Minimalist icon of elastic spring bounce, compressed and stretched shape, cyan on dark, 64x64" |
| Ripple Pond | Multiple overlapping ripple sources | "Minimalist icon of two overlapping ripple circles on water, cyan on dark, 64x64" |
| Diamond Distort | Diamond-shaped distortion | "Minimalist icon of diamond-shaped geometric distortion, cyan on dark, 64x64" |
| Barrel Distort | Barrel/pincushion grid | "Minimalist icon of barrel distortion on a straight grid, edges curving outward, cyan on dark, 64x64" |
| Sine Grid | Grid distorted by sine waves in X and Y | "Minimalist icon of a grid distorted by sine waves in both axes, cyan on dark, 64x64" |
| Glitch Displace | Offset rectangular blocks | "Minimalist icon of horizontal block displacement glitch, shifted rectangles, cyan on dark, 64x64" |

### Color Effects (20)

| Effect | Icon Description | AI Prompt |
|--------|-----------------|-----------|
| Hue Shift | Color wheel with rotation arrow | "Minimalist icon of a color wheel with rotation arrow, hue shift, cyan on dark, 64x64" |
| Saturation | Gradient from gray to vivid | "Minimalist icon of saturation adjustment, gray-to-colorful gradient bar, cyan on dark, 64x64" |
| Brightness | Sun/circle with rays | "Minimalist icon of brightness, circle with radiating lines like sun, cyan on dark, 64x64" |
| Duotone | Two overlapping color blocks | "Minimalist icon of duotone effect, two overlapping color rectangles, cyan on dark, 64x64" |
| Chromatic Aberration | RGB channels offset | "Minimalist icon of chromatic aberration, three offset circles (R/G/B channels), cyan on dark, 64x64" |
| Invert | Yin-yang or negative symbol | "Minimalist icon of color inversion, half-filled circle, positive/negative, cyan on dark, 64x64" |
| Posterize | Stepped gradient bands | "Minimalist icon of posterization, 4 distinct gradient steps, cyan on dark, 64x64" |
| Color Shift | Three overlapping shifted circles | "Minimalist icon of RGB color channel shifting, three circles slightly offset, cyan on dark, 64x64" |
| Thermal | Heat gradient bars | "Minimalist icon of thermal heat map, vertical gradient bars cold-to-hot, cyan on dark, 64x64" |
| Contrast | S-curve graph | "Minimalist icon of contrast adjustment, S-curve on small graph, cyan on dark, 64x64" |
| Sepia | Old photograph corner | "Minimalist icon of sepia tone, vintage photo frame corner, cyan on dark, 64x64" |
| Cross Process | Crossed diagonal colors | "Minimalist icon of cross-processing, two diagonal color strips crossed, cyan on dark, 64x64" |
| Split Tone | Half circle split into two tones | "Minimalist icon of split-toning, circle split into highlight and shadow halves, cyan on dark, 64x64" |
| Color Halftone | Dot matrix pattern | "Minimalist icon of color halftone dots, CMYK-style offset dot grid, cyan on dark, 64x64" |
| Dither | Noise dot pattern | "Minimalist icon of dithering, scattered noise dots gradient, cyan on dark, 64x64" |
| Heat Map | Infrared-style gradient | "Minimalist icon of heat map visualization, blobs of heat, cyan on dark, 64x64" |
| Selective Color | Color picker target | "Minimalist icon of selective color, eyedropper on one colored area, cyan on dark, 64x64" |
| Film Grain | Scattered fine dots | "Minimalist icon of film grain texture, fine scattered noise particles, cyan on dark, 64x64" |
| Gamma Levels | Three-point levels graph | "Minimalist icon of gamma levels adjustment, input/output curve with three points, cyan on dark, 64x64" |
| Solarize | Partially inverted gradient | "Minimalist icon of solarization, gradient that partially inverts, cyan on dark, 64x64" |

### Glitch Effects (9)

| Effect | Icon Description | AI Prompt |
|--------|-----------------|-----------|
| Pixel Scatter | Dispersed pixel squares | "Minimalist icon of pixel scatter, small squares dispersed from center, cyan on dark, 64x64" |
| RGB Split | Three offset color channels | "Minimalist icon of RGB split, three horizontal lines offset vertically, cyan on dark, 64x64" |
| Block Glitch | Rectangular displaced blocks | "Minimalist icon of block glitch, rectangular sections shifted horizontally, cyan on dark, 64x64" |
| Scanlines | Horizontal line overlay | "Minimalist icon of CRT scanlines, thin horizontal lines across a square, cyan on dark, 64x64" |
| Digital Rain | Falling character columns | "Minimalist icon of digital rain, vertical columns of small dots falling, cyan on dark, 64x64" |
| Noise | Static noise pattern | "Minimalist icon of visual noise/static, random pixel pattern, cyan on dark, 64x64" |
| Mirror | Reflected symmetry | "Minimalist icon of mirror reflection, shape reflected along vertical axis, cyan on dark, 64x64" |
| Pixelate | Large mosaic blocks | "Minimalist icon of pixelation, large square mosaic blocks, cyan on dark, 64x64" |
| Glitch Displace | Data corruption lines | "Minimalist icon of data corruption glitch, horizontal tear/displacement lines, cyan on dark, 64x64" |

### Pattern Effects (11)

| Effect | Icon Description | AI Prompt |
|--------|-----------------|-----------|
| CRT Simulation | Curved screen with scan lines | "Minimalist icon of CRT monitor, curved screen with phosphor dots, cyan on dark, 64x64" |
| VHS Effect | Tape distortion lines | "Minimalist icon of VHS tape distortion, wavy horizontal tracking lines, cyan on dark, 64x64" |
| ASCII Art | Letter characters grid | "Minimalist icon of ASCII art, grid of different-sized characters forming image, cyan on dark, 64x64" |
| Dot Matrix | Regular dot grid | "Minimalist icon of dot matrix printer output, regular grid of varying-size dots, cyan on dark, 64x64" |
| Crosshatch | Cross-hatched shading lines | "Minimalist icon of crosshatch shading, diagonal crossing lines of varying density, cyan on dark, 64x64" |
| Emboss | Raised 3D surface edge | "Minimalist icon of emboss effect, raised edge on flat surface, directional lighting, cyan on dark, 64x64" |
| Oil Paint | Swirly brush strokes | "Minimalist icon of oil paint effect, swirly brush stroke texture, cyan on dark, 64x64" |
| Pencil Sketch | Hand-drawn sketch lines | "Minimalist icon of pencil sketch, loose hand-drawn lines on paper texture, cyan on dark, 64x64" |
| Voronoi Glass | Irregular cell pattern | "Minimalist icon of voronoi glass cells, irregular polygonal mosaic, cyan on dark, 64x64" |
| Cross Stitch | Fabric X-pattern grid | "Minimalist icon of cross stitch embroidery, X-shaped stitches in grid, cyan on dark, 64x64" |
| Night Vision | Green-tinted with noise | "Minimalist icon of night vision, green-tinted circle with grain noise, cyan on dark, 64x64" |

### Animation Effects (3)

| Effect | Icon Description | AI Prompt |
|--------|-----------------|-----------|
| Strobe | Lightning bolt / flash | "Minimalist icon of strobe flash, lightning bolt or bright flash burst, cyan on dark, 64x64" |
| Pulse | Pulsing circle (heartbeat) | "Minimalist icon of pulse/heartbeat, expanding concentric circles like sonar, cyan on dark, 64x64" |
| Slit Scan | Time-displaced vertical slices | "Minimalist icon of slit scan, vertical strips at different time offsets, cyan on dark, 64x64" |

### Blend Effects (5)

| Effect | Icon Description | AI Prompt |
|--------|-----------------|-----------|
| Double Exposure | Two overlapping images | "Minimalist icon of double exposure, two overlapping transparent images, cyan on dark, 64x64" |
| Frosted Glass | Blurred through textured surface | "Minimalist icon of frosted glass, blurred shape behind textured surface, cyan on dark, 64x64" |
| Prism Refract | Triangular prism splitting light | "Minimalist icon of prism refraction, triangle splitting beam into spectrum, cyan on dark, 64x64" |
| Rain on Glass | Droplets with refraction | "Minimalist icon of rain droplets on glass, small circles with refraction, cyan on dark, 64x64" |
| Hexagonalize | Hexagonal tile grid | "Minimalist icon of hexagonal tiling, honeycomb hexagon grid, cyan on dark, 64x64" |

### 3D/Depth Effects (6)

| Effect | Icon Description | AI Prompt |
|--------|-----------------|-----------|
| Perspective Tilt | Trapezoid (3D perspective) | "Minimalist icon of perspective tilt, rectangle transforming into trapezoid, cyan on dark, 64x64" |
| Cylinder Wrap | Image curved around cylinder | "Minimalist icon of cylinder wrap, flat surface curved around cylinder, cyan on dark, 64x64" |
| Sphere Wrap | Image mapped on sphere | "Minimalist icon of sphere wrap, flat surface mapped onto sphere, cyan on dark, 64x64" |
| Tunnel | Receding tunnel depth | "Minimalist icon of tunnel, concentric rectangles receding to vanishing point, cyan on dark, 64x64" |
| Page Curl | Corner peeling up | "Minimalist icon of page curl, paper corner peeling up revealing underneath, cyan on dark, 64x64" |
| Parallax Layers | Layered depth planes | "Minimalist icon of parallax depth, multiple offset layers at different depths, cyan on dark, 64x64" |

### Blur/Post Effects (6)

| Effect | Icon Description | AI Prompt |
|--------|-----------------|-----------|
| Gaussian Blur | Soft focus circle | "Minimalist icon of gaussian blur, sharp circle fading to soft blur, cyan on dark, 64x64" |
| Zoom Blur | Radial speed lines | "Minimalist icon of zoom blur, radial speed lines from center, cyan on dark, 64x64" |
| Shake | Offset trembling square | "Minimalist icon of camera shake, square with motion offset trails, cyan on dark, 64x64" |
| Vignette | Dark corners frame | "Minimalist icon of vignette, darkened corners around bright center, cyan on dark, 64x64" |
| Motion Blur | Directional streak | "Minimalist icon of motion blur, horizontal streak/smear of a shape, cyan on dark, 64x64" |
| Glow | Bright bloom around center | "Minimalist icon of glow/bloom, bright center radiating soft light, cyan on dark, 64x64" |
| Edge Detect | Outline edges of shape | "Minimalist icon of edge detection, outline edges on dark background, cyan on dark, 64x64" |

---

## 28. Competitive Position

| Feature | Resolume Arena | VDMX | TouchDesigner | **Audio-DNA v2** |
|---------|---------------|------|---------------|-----------------|
| BPM detection | Manual/tap only | Waveclock plugin | Manual/Link | **Auto from live audio, locked** |
| Beat phase | Manual sync | Clock source | Manual | **Auto, smooth sawtooth** |
| Downbeat detection | None | None | None | **Auto from spectral analysis** |
| Bar phase | None | Clock plugin | Configurable | **Auto bar-level sawtooth** |
| Audio analysis signals | 3 FFT bands | Basic FFT | Basic FFT | **42+ features, all routable** |
| Procedural sources | Wire (node-based) | ISF | GLSL/Python | **40 built-in, GLSL** |
| Layer system | Yes (full) | Yes (layers) | Yes (TOPs) | **Yes (Resolume-style)** |
| Signal routing | Dashboard + FFT | Data sources | CHOPs | **Universal per-parameter** |
| Price | $299-799 | $199-399 | $600+ | **Open source (GPLv3)** |
