# Feature Connections — Audio-DNA Behavioral Reference

> Version: 1.0 | Date: 2026-05-22
> Source: CONNECTION_DECISIONS_2026-05-22.md (session notes)
> Status: Canonical. This document is the permanent behavioral reference
> for how Audio-DNA's features interact. It supersedes all prior
> session-notes and TBD items on topics it covers.

---

## How to Use This Document

This document answers one question: **"When X happens, what does Y do?"**

It encodes 12 connection scenarios with ~35 individual decisions about how
Audio-DNA's three intent layers, scope hierarchy, state machine, timeline,
and composition file system interact at runtime.

### When to Read What

| You need to know... | Read... |
|---------------------|---------|
| Which layer wins when multiple layers set a parameter | Section 3 (Cross-Cutting Rules) + Scenario 1 |
| How Global vs Layer vs Clip scopes resolve | Section 3 + Scenario 2 |
| What happens when two Hits occupy the same beat | Scenario 3 |
| Whether switching modes interrupts playback | Scenario 4 |
| What the user sees before any content exists | Scenario 5 |
| How a Hit interacts with an active Signal | Scenario 6 |
| How per-parameter envelopes override Hit-level envelopes | Scenario 7 |
| How Signal depth works on a routed parameter | Scenario 8 |
| How cuepoints relate to Hits | Scenario 9 |
| How composition files handle media references | Scenario 10 |
| How the timeline's two lanes work (Hits vs Recording) | Scenario 11 |
| How a Macro moves between scopes | Scenario 12 |
| The AUTO/OVERRIDE state machine | Section 3.3 + Scenario 1 + Scenario 6 |
| Quick lookup tables for scope, state, colors | Section 5 |

### Related Documents

| Document | What it covers |
|----------|---------------|
| AUDIO_DNA_DIRECTIVE_FULL.md | Full design and architecture directive (source of truth) |
| MENTAL_MODELS.md | Conceptual spine: the three intent layers, scope hierarchy, and state machine framing (see that doc for the full mental-model treatment) |
| BORIS_DECISIONS.md | Terse decision log (colors, naming, hard visual rules) |
| DESIGN_PROFILE.md | Boris's design sensibility, aesthetic principles, "perfect" checklist |
| HANDOFF.md | Implementation plan with resolved questions Q1-Q24 |

---

## 1. Canonical Naming

These terms are used consistently throughout this document and across all
Audio-DNA docs. Using different terms for the same concept causes confusion.

| Concept | Canonical Name | Aliases (use once, parenthetically) | Never Use |
|---------|---------------|-------------------------------------|-----------|
| Audio-driven layer | **Signal** | -- | "Signal Groups" alone (always say "Macros") |
| Scheduled-event layer | **Hits** | -- | "Cues," "Clicks," "Q" |
| Real-time user layer | **Live VJ** | -- | "Manual layer" |
| Signal processing chains | **Macros** | (Signal Groups) on first mention | "Signal Groups" without "Macros" |
| Default state | **AUTO mode** | -- | "Reclaim" |
| User-held state | **OVERRIDE mode** | -- | "Manual hold" |
| Override indicator color | **orange `#ff4500`** | -- | "Reclaim button" |
| Scope hierarchy | **Global > Layer > Clip** | -- | Inverted orders |
| Save button (transport stopped) | **Save to Hit** | -- | -- |
| Save button (transport playing) | **Capture Hit** | -- | -- |
| The save/capture button itself | **Unified Save/Capture button** | -- | "Reclaim button" |
| Beat-quantized events | **Hit grid** | -- | -- |
| Frame-quantized recording | **Recording grid** | -- | -- |

---

## 2. The Three Intent Layers (Conceptual Overview)

Audio-DNA composes visual output from three independent layers of intent.
Every UI element either operates ON a layer, visualizes a layer's state,
or switches emphasis between layers (LIVE / PROGRAM / SETUP modes).

```
SIGNAL     Audio-driven. Always on. Signals + Macros modulate parameters
           every frame. Cannot be "beaten" -- it always runs based on
           current state.

HITS       Schedule-driven. Programmed events that fire at beat-quantized
           positions on the timeline. A Hit is a state mutation -- it
           writes new values to parameters at its timestamp.

LIVE VJ    Hands-on. The user touches a parameter in real time. Like a
           Hit, this is a state mutation -- it writes a new value.
           Additionally, it engages OVERRIDE to lock out Hit writes
           on that field.
```

**For the full conceptual treatment** -- including how these layers relate
to each other as a unified mental model, worked analogies, and the
"mixing desk" framing -- see MENTAL_MODELS.md.

---

## 3. Cross-Cutting Rules

These meta-principles apply across all 12 scenarios. They are the rules
behind the rules.

### 3.1 The Universal Field-Level Rule

**Every individually-clickable/draggable UI field has its own independent
AUTO/OVERRIDE state.**

This applies to: parameter values, envelope fields (onset, curve, release),
depth sliders, route on/off toggles, and any other interactive control.

There is no "group override" that locks an entire section. Each field is
independent. The top chrome may carry a passive orange STATUS indicator
showing that overrides exist somewhere in the composition, but it is NOT
a clickable mass-release button.

### 3.2 Layer Precedence

Hits and Live VJ are **equal-priority state mutations** -- last write wins.
The Signal layer is **always-on** and computes from current state. It does
not compete; it always runs based on whatever the state currently is.

| Situation | What Happens |
|-----------|-------------|
| Signal is modulating opacity; VJ touches opacity | VJ write takes effect. Signal continues modulating around the new VJ-set value. Field enters OVERRIDE (Hits blocked). |
| Signal is modulating opacity; Hit fires with opacity=0.8 | Hit write takes effect (if field is in AUTO). Signal continues modulating around 0.8. |
| Hit fires opacity=0.8; VJ then touches opacity | VJ write overwrites Hit's value. Field enters OVERRIDE. Signal continues modulating. |
| VJ releases a field (clicks orange dot) | Field returns to AUTO. Next Hit can write to it again. |

**Meta-rule:** The user's higher-level decision always wins.
Live VJ (most recent) > Hits (scheduled) > Signal (ambient).

### 3.3 The AUTO/OVERRIDE State Machine

Every field starts in AUTO mode. The state machine has exactly two states
and two transitions.

```
                   VJ touches field
    +---------+  ------------------>  +------------+
    |  AUTO   |                       |  OVERRIDE  |
    |         |  <------------------  |            |
    +---------+   VJ clicks orange    +------------+
                   indicator

AUTO mode:
  - Hits CAN write to this field
  - Signal layer modulates this field (always)
  - Field value = baseline set by most recent Hit (or initial value)

OVERRIDE mode:
  - Hits are BLOCKED from writing this field
  - Signal layer CONTINUES to modulate (reads state, adds modulation)
  - Field value = whatever the VJ set
  - Orange indicator visible on the field
```

**Override color:** orange `#ff4500` -- used at three zoom levels:
1. **App-wide:** top-chrome AUTO STATUS indicator (passive, shows overrides exist)
2. **Group-wide:** layer strip header, section header (shows overrides in group)
3. **Per-field:** orange dot at row end (clickable -- returns field to AUTO)

All three use the same color and share the same semantic: "manually held,
not following the show."

**Design system note:** Orange `#ff4500` is a deliberate exception to the
"one accent color" rule. It is the ONLY second meaningful color in the
design system, used exclusively for OVERRIDE state. This is documented
prominently in the design system docs. It is the formerly-retired orange,
intentionally reintroduced for this specific purpose.

### 3.4 Scope Precedence (Signal Layer)

Within the Signal layer, scope precedence follows the mixing-desk model:

**Global > Layer > Clip** (outermost wins, exclusive)

Only ONE scope is active for any parameter at a time. If a Global Macro
routes to opacity, Global runs; Layer and Clip Macros are silent for
opacity. If Global does not route opacity, Layer gets it. If Layer does
not route it either, Clip does.

**Same-scope multi-route:** When multiple Macros at the SAME scope route
to the same parameter, their contributions **SUM**, clipped to [0, 1].

**Multi-route UI:** A standard triangle indicator appears on the row.
Details (which specific Macros contribute) are accessible via the Bottom
Focus Bar when the row is selected.

### 3.5 Runtime Computation

Every frame, for every parameter:

```
output = clamp(baseline + SUM(active_routes * depth), 0, 1)
```

Where:
- `baseline` = the current set value (written by most recent Hit or VJ touch)
- `active_routes` = all Signal routes at the winning scope, each with its
  own depth value
- `depth` = per-route depth slider, 0-100% (see Scenario 8)

### 3.6 Iron Rules (Design System, Confirmed)

These rules are absolute and apply everywhere. Listed here because they
affect the behavioral layer too (e.g., which visual signals indicate state).

| Rule | Detail |
|------|--------|
| No border-radius | Square corners everywhere, no exceptions |
| No box-shadows | Any element |
| No gradients on chrome | Gradient only in SVG thumbnails or waveform rendering |
| No circular knobs | Vertical signal columns universally |
| 1px hairline borders | `#3a3a3a` everywhere |
| Buttons touch | No gaps between adjacent buttons |
| 3% SVG grain | `opacity: 0.03` -- the only decorative element |
| IBM Plex Mono | Numbers only |
| IBM Plex Sans | Labels only |
| ALL-CAPS | Section headers and mode labels |
| Blue `#00d9ff` | "Active / alive / routing" -- the sole accent |
| Red `#ff4040` | REC indicator + errors ONLY |
| Orange `#ff4500` | OVERRIDE state ONLY (deliberate exception to one-accent rule) |

### 3.7 Inspector Grammar (Universal)

Every inspector row everywhere in the app:

```
[triangle] [Label 90px right #888] [Value 50px mono #e0e0e0] [-][+] [slider 14px] [P.]
```

This grammar applies in: inspector panels, effect controls, signal group
editors, Hit control surfaces, Bottom Focus Bar detail view. One visual
language. A user who learns the inspector once can read any control surface.

**Triangle states:**

| State | Appearance |
|-------|-----------|
| Unrouted | Light outline `#3a3a3a` -- barely visible, just enough to know it is clickable |
| Selected / menu open | Solid light fill `#888` |
| Active (signal routed) | Blue `#00d9ff` fill, brightness = signal value 0 to 1 in real time. RAW, no smoothing. |

---

## 4. The Three Intent Layers (Layer Summary)

This section provides a compact summary. For the full conceptual treatment
with analogies and worked mental models, see MENTAL_MODELS.md.

### Signal Layer

- Always on. Runs every frame.
- Reads current parameter state (baseline) and adds modulation.
- Cannot be "beaten" or "overridden" -- it always runs.
- Organized into three tiers: Sources (raw audio features) -> Signals
  (derived/combined) -> Macros (processing chains with single output).
- Macros exist at three scopes (Global, Layer, Clip).
- Scope precedence: Global > Layer > Clip (outermost wins, exclusive).

### Hits Layer

- Schedule-driven. Events fire at beat-quantized positions.
- A Hit is a partial state mutation -- it writes to the parameters it
  touches and leaves everything else unchanged (diff-based override).
- Can only write to fields in AUTO mode. Fields in OVERRIDE are blocked.
- Organized as: Hit -> Hit Group (reusable sequence) -> Composition timeline.
- Maximum density: 1 Hit per beat (4 per bar in 4/4 time).

### Live VJ Layer

- Hands-on. The user touches parameters in real time.
- A touch is a state mutation (same as a Hit).
- Additionally engages OVERRIDE on the touched field.
- OVERRIDE blocks Hits from writing that field until released.
- Signal layer continues modulating around the VJ-set value.
- Release: click the orange indicator on the field -> returns to AUTO.

---

## 5. Quick Lookup Tables

### 5.1 Layer Precedence Summary

| Layer | Behavior | Priority | Can Be Blocked? |
|-------|----------|----------|----------------|
| Signal | Computes from state every frame | Always runs | No -- always modulates |
| Hits | Writes state at scheduled times | Below VJ, above nothing | Yes -- OVERRIDE blocks writes |
| Live VJ | Writes state on touch | Highest write priority | No -- VJ is the user |

### 5.2 Scope Precedence Summary

| Scope | Wins Over | Falls Through When |
|-------|-----------|--------------------|
| Global | Layer, Clip | Global does not route the param |
| Layer | Clip | Layer does not route the param |
| Clip | Nothing below | Always runs if no higher scope routes |

**Same-scope collision:** contributions SUM, clipped to [0, 1].

### 5.3 State Machine Summary (AUTO / OVERRIDE)

| State | Hits Can Write? | Signal Modulates? | VJ Can Write? | Visual Indicator |
|-------|----------------|-------------------|---------------|-----------------|
| AUTO | Yes | Yes | Yes (triggers OVERRIDE) | None (default) |
| OVERRIDE | No (blocked) | Yes (always) | Yes | Orange `#ff4500` dot on field |

| Transition | Trigger | Effect |
|-----------|---------|--------|
| AUTO -> OVERRIDE | VJ touches field | Field locked to VJ value; Hits blocked |
| OVERRIDE -> AUTO | VJ clicks orange dot | Field unlocked; next Hit can write |

### 5.4 Color Token Summary

| Token | Hex | Usage | Scope |
|-------|-----|-------|-------|
| `--accent` | `#00d9ff` | Active, alive, routing, signal-driven | Global -- the only accent |
| `--danger` | `#ff4040` | REC indicator + errors | Narrow -- two use cases only |
| `--override` | `#ff4500` | OVERRIDE state indicator | Three zoom levels (app, group, field) |
| `--bg` | `#1a1a1a` | App background | -- |
| `--panel` | `#2a2a2a` | Panel fills | -- |
| `--section-header` | `#252525` | Section header strips | -- |
| `--separator` | `#383838` | Row separators | -- |
| `--border` | `#3a3a3a` | All hairline borders (1px) | -- |
| `--label` | `#888888` | Dim labels | -- |
| `--value` | `#e0e0e0` | Bright values, active text | -- |

### 5.5 Hit Quantize Settings

| Setting | Hits Per Bar | Minimum Spacing (at 120 BPM) |
|---------|-------------|------------------------------|
| 1 per bar | 1 | 2000ms |
| 2 per bar | 2 | 1000ms |
| 4 per bar (max) | 4 | 500ms |

### 5.6 Envelope Defaults

| Setting | Default | Options |
|---------|---------|---------|
| Onset | Instant | Instant, Fade-in (duration in beats/bars) |
| Curve | Linear | Linear, Ease-in, Ease-out, Ease-in-out, Exponential, S-curve, Step |
| Release | Instant cut | Instant cut, Fade-out (duration) |

---

## 6. Scenarios

### Scenario 1 -- Layer Precedence

**The moment:** The user has a Signal (BASS) driving opacity via a Macro.
A Hit fires at bar 16, setting opacity to 0.3. The VJ then manually grabs
the opacity slider during a breakdown.

**The resolved answer:**

Hits and Live VJ are equal-priority state mutations -- last write wins.
The Signal layer is always-on and computes from current state every frame.
It does not compete with Hits or VJ; it modulates whatever the current
baseline is.

Each field has a per-field state machine: AUTO (default) or OVERRIDE.

- VJ touches a field -> that field enters OVERRIDE.
- VJ clicks the field's orange `#ff4500` indicator -> returns to AUTO.

**Worked example:**

```
Bar 1-15:  opacity baseline = 1.0 (initial), BASS Signal modulating
           output = clamp(1.0 + BASS_value * depth, 0, 1)
           Field is in AUTO.

Bar 16:    Hit fires. Sets opacity baseline to 0.3.
           Field still AUTO. Hit writes succeed.
           output = clamp(0.3 + BASS_value * depth, 0, 1)

Bar 20:    VJ grabs opacity slider, drags to 0.7.
           Field enters OVERRIDE. Orange dot appears.
           baseline = 0.7
           output = clamp(0.7 + BASS_value * depth, 0, 1)
           BASS still modulates -- only the baseline changed.

Bar 24:    Hit fires with opacity = 0.5.
           Field is in OVERRIDE. Hit write BLOCKED.
           baseline stays at 0.7. No change.

Bar 28:    VJ clicks the orange dot on opacity.
           Field returns to AUTO.
           baseline stays at 0.7 (VJ's last value persists until
           next write).

Bar 32:    Hit fires with opacity = 0.9.
           Field is in AUTO. Hit write succeeds.
           baseline = 0.9.
```

**Implications:**

- No global release button exists. Override release is per-field only.
- The top chrome carries a passive orange STATUS indicator showing overrides
  exist somewhere, but it is NOT clickable.
- The button label for returning to auto is **AUTO**. The mode label pair
  is **AUTO / OVERRIDE**. The term "Reclaim button" is rejected.

**Cross-references:** Scenario 6 (Hit-Signal handoff detail),
Section 3.3 (state machine), Section 3.2 (layer precedence rule).

---

### Scenario 2 -- Scope Precedence

**The moment:** The user has a Global Macro routing BASS to opacity AND
a Layer Macro on Layer 1 also routing BASS to opacity. Which one controls
Layer 1's opacity?

**The resolved answer:**

Scope precedence follows the mixing-desk model:
**Global > Layer > Clip** (outermost wins, exclusive).

Only ONE scope is active for any param at a time. If a Global Macro
routes opacity, Global runs; Layer and Clip Macros are silent for that
param. If Global does not route the param, Layer gets it. If Layer does
not route it, Clip does.

When multiple Macros at the SAME scope route to the same parameter,
contributions SUM, clipped to [0, 1].

**Worked example:**

```
Setup:
  Global Macro "BASS PUMP" routes BASS -> opacity (depth 80%)
  Layer 1 Macro "LAYER PULSE" routes BASS -> opacity (depth 50%)
  Clip Macro "LOCAL FADE" routes HIGH -> opacity (depth 30%)

Runtime on Layer 1:
  Global routes opacity -> Global wins.
  Layer 1 Macro is SILENT for opacity.
  Clip Macro is SILENT for opacity.
  output = clamp(baseline + BASS_value * 0.80, 0, 1)

If user removes the BASS->opacity route from the Global Macro:
  Global no longer routes opacity -> falls through.
  Layer 1 Macro takes over.
  output = clamp(baseline + BASS_value * 0.50, 0, 1)

Same-scope example:
  Two Global Macros both route to opacity:
    Macro A: BASS -> opacity (depth 60%)
    Macro B: FLUX -> opacity (depth 40%)
  output = clamp(baseline + BASS_value * 0.60 + FLUX_value * 0.40, 0, 1)
```

**Implications:**

- The triangle indicator on the row shows active routing (blue pulsing).
- Which specific Macros contribute is visible via the Bottom Focus Bar
  when the row is selected. The triangle itself does not differentiate.
- The scope hierarchy is unambiguous: no "merge" or "blend" between scopes.
  One scope wins completely for a given parameter.

**Cross-references:** Section 3.4 (scope precedence),
Scenario 12 (Macro transfer between scopes), Section 3.5 (runtime computation).

---

### Scenario 3 -- Hit Position Conflict

**The moment:** Can two Hits land on the same beat? Can a single Hit have
contradictory values for the same parameter?

**The resolved answer:**

Resolved by design -- no conflict is possible.

- Hits live on a quantize grid. Default: 1 per beat = 4 per bar in 4/4.
- User-adjustable quantize: 1, 2, or 4 per bar (max 4).
- No two Hits share a grid position.
- Within a single Hit, the filter UI prevents internal contradictions:
  one Hit = one snapshot, one value per touched parameter.

**Worked example:**

```
4/4 time, quantize = 4 per bar (1 per beat):

  Bar 1:  [Hit]  [----]  [Hit]  [----]
          beat1   beat2   beat3   beat4

  Bar 2:  [----]  [----]  [----]  [Hit]

  Each slot holds at most ONE Hit.
  Each Hit captures exactly one value per parameter it touches.
  No ambiguity.

  At quantize = 2 per bar:
  Bar 1:  [Hit]  [xxxx]  [Hit]  [xxxx]
          beat1  (locked) beat3  (locked)

  Only beats 1 and 3 are available. Beats 2 and 4 are not selectable.
```

**Implications:**

- The P. button (route-entire-section) has been removed from individual
  parameter rows. Section headers retain the 2px blue left-edge indicator
  as a passive routing presence marker. Per-parameter routing happens via
  the triangle on each row.
- There is no need for conflict resolution logic between same-position
  Hits because the grid prevents same-position Hits.

**Cross-references:** Scenario 11 (timeline model, Hit grid detail),
Section 5.5 (quantize settings table).

---

### Scenario 4 -- Mode-Switch During Playback

**The moment:** The VJ is performing in LIVE mode. Mid-performance, they
switch to PROGRAM mode to check a signal routing. Does playback stop?
Do overrides reset?

**The resolved answer:**

Mode is WORKSPACE only. It never affects runtime.

**Stays running across mode switches (no interruption):**

| What | Persists? |
|------|-----------|
| Playback position + transport state | Yes |
| Audio analysis + Signal computation | Yes |
| Hit firing + envelope interpolation | Yes |
| Clip transports | Yes |
| Live VJ parameter ownership (OVERRIDE state) | Yes |
| MIDI/OSC bindings | Yes |

**Changes on mode switch (workspace only):**

| What | How It Changes |
|------|---------------|
| Layout | Panels rearrange to the mode's last-used template |
| Triangle click behavior | LIVE = quick menu; PROGRAM = full routing workspace; SETUP = signal definition config |
| Inspector exposure | Advanced controls hidden in LIVE |
| Bottom Focus Bar context | Shows mode-appropriate detail |

**Worked example:**

```
State: LIVE mode, bar 64, playing, BASS->opacity active, VJ has
       overridden scale to 1.5.

User switches to PROGRAM mode.

Result:
  - Panels rearrange to last PROGRAM layout (e.g., S1).
  - Bar counter continues: 65, 66, 67...
  - Hits continue firing on schedule.
  - BASS->opacity continues modulating.
  - scale remains in OVERRIDE at 1.5.
  - Clicking a triangle now opens full routing workspace
    (instead of LIVE's quick menu).

User switches back to LIVE.

Result:
  - Panels return to last LIVE layout.
  - Everything still running. No state lost.
```

**SETUP mode apply:** Changes in SETUP mode apply IMMEDIATELY, live.
There is no commit step. The user accepts that mid-show config changes
(e.g., changing a signal definition's threshold) take effect instantly.

**Implications:**

- Mode is purely a workspace concern. Builders must never tie runtime
  behavior to the current mode.
- Layout templates are stored per-mode. Switching LIVE -> PROGRAM -> LIVE
  returns to the exact LIVE layout the user left.

**Cross-references:** Section 4 (three intent layers),
AUDIO_DNA_DIRECTIVE_FULL.md Part 5.4 (mode system).

---

### Scenario 5 -- Empty Composition / Pre-First-Hit Behavior

**The moment:** The user opens Audio-DNA for the first time. No track
loaded. No Hits placed. No clips triggered. What do they see? What works?

**The resolved answer:**

Signal and Live VJ are ALWAYS active, regardless of Hit timeline state.
Hits are optional -- a composition can have zero Hits and function as a
pure improvisation tool.

**Always-on behavior (no Hits required):**

- Audio analysis runs the moment audio input exists.
- Signal Bar shows live values.
- Triangles pulse on any routed parameter.
- VJ can manually trigger clips at any time.
- BPM clock and bar counter always run when transport is playing.
- No Hits = nothing auto-fires, but time advances.

**Empty composition state (confirmed per HANDOFF Q21):**

| Element | State |
|---------|-------|
| Output canvas | Empty / dark |
| BPM display | `---` if no audio in; detected BPM if audio present |
| Bar counter | `1.1.1` |
| Signal Bar | Zeros if no audio; live values if audio input |
| All panels | Visible (not collapsed) |
| Clip grid | Visible, all cells dim |
| Mode | LIVE (default) |
| Active layout | H1 |

**Worked example:**

```
Launch state:
  BPM: ---
  Bar: 1.1.1
  Signal Bar: RMS 0.00 | PEAK 0.00 | LUFS 0.00 | ...
  Clip grid: all cells empty/dim
  Output: dark canvas

User connects audio input (e.g., line-in from DJ mixer):
  BPM: 128 (detected)
  Signal Bar: RMS 0.72 | PEAK 0.91 | BASS 0.85 | ...
  Triangles on routed params begin pulsing.
  No clips fire. No Hits exist. Signal drives nothing unless
  Macros are routed. VJ can manually trigger clips.

User creates a Global Macro routing BASS -> Layer 1 opacity:
  Layer 1 opacity now modulates with bass signal.
  Still zero Hits. Pure improvisation.
```

**Implications:**

- The timeline always advances when transport is playing. This is useful
  for improvisation alignment to track structure even without Hits.
- "Empty composition" is a valid and functional state, not an error state.
- Hits are an additional editorial layer for programmed shows -- they are
  not required for the app to do something useful.

**Cross-references:** Scenario 6 (Hit-Signal handoff),
Section 4 (three intent layers).

---

### Scenario 6 -- Hit-to-Signal Handoff (The State Model)

**The moment:** A Signal is modulating opacity. A Hit fires and sets a
new opacity value. What happens to the Signal? Does the Hit "override"
the Signal? Does the Signal "fight back"?

**The resolved answer:**

Boris's clarification (direct quote):

> "Hit is very similar to the user adjusting those parameters at that
> exact time. The effects are no different. When there is a signal
> going on a parameter, and the user changes it in some way, the
> signal is still going unless the signal is canceled manually or
> with a hit."

**Implication:** Hits and Live VJ are equivalent state mutations. Both
write to per-parameter state. Signal computes based on current state
every frame. Signal never stops unless explicitly canceled.

**Per-parameter state contents:**

| Field | Description |
|-------|-------------|
| `baseline` | Current set value. Modified by Hits (via "set value") or by VJ (via direct touch). |
| `active_routes` | Set of {signal, depth} currently routing to this param. |
| `mode` | AUTO or OVERRIDE per the universal field-level rule. |

**Runtime computation per frame:**

```
output = clamp(baseline + SUM(active_routes * depth), 0, 1)
```

**Worked example:**

```
Setup:
  opacity baseline = 1.0
  BASS -> opacity routed, depth = 0.5
  BASS signal value oscillating 0.0 - 1.0

Frame at bar 10 (BASS value = 0.8):
  output = clamp(1.0 + 0.8 * 0.5, 0, 1) = clamp(1.4, 0, 1) = 1.0

Hit fires at bar 16, sets opacity baseline to 0.3:
  output = clamp(0.3 + BASS_value * 0.5, 0, 1)

  At BASS = 0.0: output = 0.3
  At BASS = 0.8: output = clamp(0.3 + 0.4, 0, 1) = 0.7
  At BASS = 1.0: output = clamp(0.3 + 0.5, 0, 1) = 0.8

  Signal continues. Only the baseline changed.

VJ touches opacity, drags to 0.6:
  Field enters OVERRIDE. baseline = 0.6.
  output = clamp(0.6 + BASS_value * 0.5, 0, 1)
  Signal STILL running. Hits now blocked from writing opacity.
```

**Effect of OVERRIDE on a field:**

| What | Behavior |
|------|----------|
| Hits | BLOCKED from writing this field |
| Signal | CONTINUES to read state and modulate |
| VJ | CONTINUES writing |
| Release | VJ clicks the orange dot -> returns to AUTO |

**Override color signature:** orange `#ff4500`, displayed at three zoom
levels (see Section 3.3). All same color. Same semantic: "manually held,
not following the show." Click to release at that level.

**Implications:**

- There is no concept of Signal "winning" or "losing" against Hits.
  Signal always runs. Hits and VJ change the baseline that Signal
  modulates around.
- OVERRIDE only blocks Hits, not Signals. Signal is always-on by design.
- The orange `#ff4500` color is a deliberate exception to the single-accent
  rule. It must be documented prominently in design-system docs.

**Cross-references:** Scenario 1 (layer precedence),
Section 3.3 (AUTO/OVERRIDE state machine), Section 3.5 (runtime computation).

---

### Scenario 7 -- Per-Parameter Envelope Override

**The moment:** A Hit at bar 16 uses a 4-beat ease-out envelope by default.
But the user wants opacity to fade over 8 beats with an S-curve, while
the Macro route change happens instantly. Can individual parameters have
different envelopes within a single Hit?

**The resolved answer:**

Yes. Field-level inheritance.

- Each Hit has a **Hit-level envelope** (default for all touched parameters):
  onset (instant or duration), curve (linear / ease-in / ease-out /
  S-curve / exp / step), release (instant-cut or duration).
- Each per-payload-item can override ANY SUBSET of envelope fields.
- Unspecified fields inherit from the Hit-level envelope.
- Per the universal field-level rule, each envelope field is independently
  AUTO/OVERRIDE-able.

**Worked example:**

```
Hit at bar 16:
  Hit-level envelope:
    onset = fade-in 4 beats
    curve = ease-out
    release = instant cut

  Per-parameter overrides:
    opacity:
      onset = fade-in 8 beats      (overrides Hit-level)
      curve = S-curve               (overrides Hit-level)
      release = (not specified)     (inherits instant cut from Hit-level)

    macro_route "BASS PUMP":
      onset = instant               (overrides Hit-level)
      curve = (not specified)        (inherits ease-out, but moot for instant)
      release = (not specified)      (inherits instant cut)

    scale:
      (no overrides)                 (inherits all: fade-in 4 beats, ease-out, instant cut)

Result at bar 16:
  - opacity begins fading from previous value to 0.3 over 8 beats,
    following an S-curve. Reaches target at bar 18.
  - BASS PUMP Macro route activates instantly at bar 16.
  - scale begins fading from previous value to new value over 4 beats,
    following ease-out. Reaches target at bar 17.
```

**Implications:**

- The envelope editor in the Hit Inspector (Bottom Focus Bar) shows
  Hit-level defaults at the top, with per-parameter override rows below.
- A parameter with no overrides visually shows "inherited" state --
  it uses the Hit-level values.
- Envelope fields themselves are fields under the universal field-level
  rule, meaning a VJ can OVERRIDE an envelope setting too (e.g., grab
  the onset duration slider mid-fade).

**Cross-references:** Scenario 6 (state model),
AUDIO_DNA_DIRECTIVE_FULL.md Part 2.7 (Hit envelope system),
Section 5.6 (envelope defaults table).

---

### Scenario 8 -- Signal-to-Macro Modulation Depth

**The moment:** BASS is routed to opacity via a Macro. How strongly does
the BASS signal influence opacity? Is it always 100%? Can the user dial
it down? Can a Hit change the depth?

**The resolved answer:**

Per-route depth slider (confirmed by HANDOFF Q16).

- Each route (e.g., BASS -> opacity) has its own depth control, 0-100%.
- Depth is set when routing and is modifiable by Hits and by VJ.
- Per the universal field-level rule, depth is a FIELD -- it is
  independently AUTO/OVERRIDE-able.

**Worked example:**

```
Setup:
  BASS -> opacity routed at depth 80% (0.8)
  opacity baseline = 0.5

Frame (BASS value = 0.6):
  output = clamp(0.5 + 0.6 * 0.8, 0, 1) = clamp(0.98, 0, 1) = 0.98

Hit fires at bar 16, changes depth from 80% to 30%:
  (Depth is a field. Hit writes to it if in AUTO mode.)
  output = clamp(0.5 + 0.6 * 0.3, 0, 1) = clamp(0.68, 0, 1) = 0.68

VJ grabs the depth slider, sets it to 100%:
  Depth field enters OVERRIDE. Orange dot appears on depth control.
  output = clamp(0.5 + 0.6 * 1.0, 0, 1) = clamp(1.1, 0, 1) = 1.0

Next Hit tries to set depth to 50%:
  Depth field is in OVERRIDE. Hit write BLOCKED.
  Depth stays at 100%.

VJ releases depth (clicks orange dot):
  Depth returns to AUTO. Next Hit can change it.
```

**Implications:**

- Depth is treated identically to any other parameter field. It has its
  own AUTO/OVERRIDE state, its own triangle indicator, and its own
  position in the inspector grammar.
- The depth slider lives in the Macro Inspector or the Bottom Focus Bar
  when inspecting a route.
- Multiple routes to the same parameter each have independent depth
  values, and all sum in the runtime computation.

**Cross-references:** Section 3.5 (runtime computation),
Scenario 1 (layer precedence), Section 3.1 (universal field-level rule).

---

### Scenario 9 -- Cuepoint vs Hit Interaction

**The moment:** A clip has cuepoints set within it. A Hit fires that clip.
Does the Hit use the cuepoints? If the same source media is in two cells,
do they share cuepoints?

**The resolved answer:**

Clip cells are independent instances. Cuepoints are NOT referenced by Hits.

**Clip-cell model (Boris's clarification):**

| Property | Behavior |
|----------|----------|
| Each clip cell | An INDEPENDENT instance of source media |
| Same source in multiple cells | Allowed -- each cell is separate |
| Per-cell settings | In point, out point, cuepoints, effects, transforms |
| Editing one cell | Does NOT affect other cells with the same source |

**Cuepoints are:**

1. User-placed positional bookmarks within the clip.
2. Used for: (a) quick-scrub in the clip settings UI when arranging state
   to capture, (b) manual mid-playback jumps, (c) visual reference.
3. **NOT referenced by Hits.** Hits capture exact playhead positions.

**Worked example:**

```
Source media "tunnel_loop.mp4" is in:
  - Layer 1, Column 3 (cell A): in=0:00, out=0:30, cuepoint at 0:15
  - Layer 2, Column 1 (cell B): in=0:10, out=0:25, no cuepoints

Hit fires, targeting cell A:
  - Hit captured cell A's playhead at position 0:05 and in/out at
    moment of capture.
  - On fire: applies captured in=0:00, out=0:30, playhead=0:05 to cell A.
  - Plays from 0:05. Cuepoint at 0:15 is NOT consulted.
  - Cell B is unaffected.

Manual trigger (VJ clicks cell B):
  - Starts at cell B's current in-point (0:10).
  - Cell A is unaffected.
```

**Clip-start behavior:**

| Trigger Type | Start Position |
|-------------|----------------|
| Manual trigger (Live VJ) | Cell's current in-point |
| Hit-fired | Hit's captured playhead position + captured in/out |

**Implications:**

- There is a clean separation: cuepoints are an authoring/navigation
  tool. Hits capture absolute state. No hidden dependency between them.
- When programming a Hit that should start a clip at a specific position,
  the user scrubs to that position (possibly using cuepoints for quick
  nav), then captures the Hit. The Hit stores the playhead position
  directly, not a cuepoint reference.

**Cross-references:** Scenario 11 (timeline model, Hit creation workflows),
AUDIO_DNA_DIRECTIVE_FULL.md Part 2.1 (terminology distinction).

---

### Scenario 10 -- Composition File Portability + Gather Content

**The moment:** The user built a show at home. They want to take it to a
venue. How do they move it? What if the venue computer does not have the
same media files?

**The resolved answer:**

The composition file is small and portable. Large media is referenced by
path, not embedded. A separate "Gather Content" command bundles everything.

**Default composition file contains:**

| Included (DATA, self-contained) | Referenced (PATHS, external) |
|--------------------------------|------------------------------|
| Hit Groups | Video clips |
| Macros (Signal Groups) | MilkDrop presets |
| Routing assignments | Source media files |
| Layouts used | -- |
| Signal definitions | -- |
| MIDI overrides | -- |
| Clip metadata (in/out/cuepoints/effects) | -- |

**"Gather Content" action:**

1. Separate explicit command (File menu or COMP browser tab action).
2. Scans composition for all referenced media.
3. Copies media into a show bundle: composition file + `media/` subfolder.
4. Inside the bundle, composition paths are rewritten to local references.
5. Result: a portable folder for touring.

**Bundle format:** A folder. `My Tour Show 2026/` contains
`show.composition` + `media/` subfolder. User can inspect, swap files
manually. Matches Ableton Live Sets model.

**Missing media on load:**

```
User opens composition on venue computer.
  media/tunnel_loop.mp4 is missing.

Result:
  - Composition loads. All non-affected content works.
  - Affected cells display "media missing" state:
    - Cell drawn as Loaded state (1px border) with filename grayed (--label #888888)
    - Red `×` mark overlay (--danger #ff4040) — canonical error indicator
    - File path shown in cell
  - UI offers "Re-link missing media" to locate the file.
  - User can play unaffected parts while fixing missing files.

  Color rationale: `#ff4500` (--override orange) is reserved exclusively for
  OVERRIDE state per BORIS_DECISIONS canonical rule. Missing-media uses
  `--danger` red — the canonical "errors ONLY" color from MOCKUP_BRIEF §2.1.
```

**Library export (per HANDOFF Q14):**

| Item | Composition-Owned | Library-Shareable |
|------|-------------------|-------------------|
| Macros (Signal Groups) | Yes | Yes (both) |
| Hit Groups | Yes | Yes (both) |
| Layout templates | No | Yes (library-only, app-wide) |
| Signal definitions | No | Yes (library-only) |
| MIDI bindings | No | Yes (library-only, with per-comp override) |
| Routing assignments | Yes | No (composition-only) |
| Clips, cells, decks | Yes | No (composition-only) |

Library export is a SEPARATE action ("Save to Library") from in-composition
scope changes.

**Implications:**

- The composition file stays small and fast to save/load. Media paths
  are relative within a bundle, absolute otherwise.
- "Gather Content" is the export-for-touring workflow. It is not automatic.
- Missing media is a recoverable state, not a fatal error. The show
  degrades gracefully.

**Cross-references:** Scenario 12 (Macro transfer between scopes -- library
export is a related concept), AUDIO_DNA_DIRECTIVE_FULL.md Part 2 (Hit system).

---

### Scenario 11 -- Data Recording vs Hits (Timeline Model)

**The moment:** The timeline shows both Hits (beat-quantized) and
Recording data (frame-quantized). How do these two systems coexist?
How does the user create Hits? How do the lanes interact?

**The resolved answer:**

Two independent grids share one visual timeline. The composition has ONE
active driver at a time (Hits or Recording). Both are always visible and
editable.

### 11.1 The Two Grids

**Hit grid (beat-quantized):**

| Property | Value (at 120 BPM) |
|----------|--------------------|
| Maximum Hits per bar | 4 (1 per beat in 4/4) |
| 1 beat duration | 500ms |
| 1 bar duration | 2000ms |
| Quantize options | 1 per bar, 2 per bar, 4 per bar (max) |

**Recording grid (FPS-quantized):**

| Property | Value |
|----------|-------|
| Internal resolution | 60fps (~16.67ms per frame) |
| Tied to BPM? | No -- FPS is independent |
| Frames per beat (120 BPM) | 30 |
| Captures | Everything continuously: Hit firings, VJ touches, Signal activations, clip triggers, parameter changes |

### 11.2 Two-Lane Shared Timeline

```
DEFAULT (both lanes thin):
+----------------------------------------------------------+
| TIMELINE RULER (beats / time ticks, drag here = zoom)    | 12px
+----------------------------------------------------------+
| WAVEFORM (stereo pair, beat grid overlaid)                | 50px
+----------------------------------------------------------+
| HITS  *|  ||    ||    |      ||                           | 20px (thin)
+----------------------------------------------------------+
| REC   o|  ~~~/^\~~~/^^\~~/^\~                             | 16px (thin)
+----------------------------------------------------------+
```

**Recording lane (default, collapsed):** Continuous density curve (smooth
gray "waveform" of activity). Time-based axis (ticks in seconds).
Decoupled from BPM.

**Hits lane (default, collapsed):** Blue capsule pills at beat positions.
Beat-based axis (bar.beat labels).

Both lanes share the same horizontal position scale (visually aligned)
but each labels its axis in its native units.

### 11.3 Lane Expansion (After Effects Model)

Click a lane handle to expand vertically. Expanded lanes show
per-parameter sub-timelines with hierarchical disclosure:

```
REC (expanded):
  Layer 1 (group disclosure)
      opacity     ~~~~/^^^\~~~~~~~~~/^\~~   (continuous curve)
      scale       ~~~~~~~~/^\~~~~~~~~~~~~~~
      effect.wet  ~~~~~~~~~~~~~/^\~~~~~~~~
  Layer 2 (collapsed group)
  Global (collapsed group)

HITS (expanded):
  Layer 1
      opacity     *=====*/////*  (keyframes + envelope curves)
      scale       *-------------*  (linear ramp)
  Layer 2
  Global
```

**Sub-track visual types:**

| Lane | Visual Type |
|------|------------|
| Recording sub-track | Continuous captured-value curve over time |
| Hits sub-track | Discrete keyframe points + envelope curve lines between consecutive Hits on that param |

### 11.4 Scrubbing and Clicking Rules

| Lane | Drag | Click Empty | Click Existing |
|------|------|-------------|----------------|
| Recording | Scrubs playhead continuously | Jumps playhead to position | N/A |
| Hits | NOT scrubbable (no continuous data between Hits) | Selects beat slot as save target (HIGHLIGHT only; does NOT move playhead) | Jumps playhead to Hit's position + selects Hit + opens Hit Inspector in Bottom Focus Bar |

**Why the Hits lane does not scrub:** Hits are discrete keyframes at
quantized positions. There is no continuous state between them. The
interpolation happens during PLAYBACK, not during scrubbing.

### 11.5 Active Driver

The composition has ONE active driver at a time:

| Driver | What Fires Events | What Is Visible But Passive |
|--------|-------------------|-----------------------------|
| HITS | Hit timeline fires events | Recording visible but does not fire |
| RECORDING | Recording fires its captured events | Hits visible but do not fire |

Switch via the indicator at each lane's left handle.

REC dropdown next to the label picks WHICH recording is displayed
(a composition supports multiple recordings).

**Lanes are editorially independent:** The Hit lane is ALWAYS EDITABLE
regardless of which is the active driver. The VJ can place/edit Hits while
watching a recording play. The driver determines what fires at playback
time; editing is decoupled.

### 11.6 Hit Creation Workflows

**Three ways to create a Hit:**

#### Workflow 1 -- Scrub-and-Save (Offline Programming)

```
1. User scrubs master playhead via recording lane to desired state.
2. Output window shows the state at the scrubbed moment.
3. User clicks target Hit beat slot -> slot is HIGHLIGHTED as save target.
   Playhead STAYS at scrub position (does NOT jump).
4. User presses the Save to Hit button.
5. Hit is created at the highlighted slot with state captured from
   the scrub position.
6. If slot was occupied -> confirm dialog: "Replace this Hit?" Yes/No.
   (NOT merge -- Boris explicitly chose confirm-replace.)
```

#### Workflow 2 -- Two-Hit Automation (Ableton-Style)

```
1. Create Hit 1 at bar 16 with starting state (via any workflow).
2. Create Hit 2 at bar 32 with ending state.
3. Default envelope between them: LINEAR.

   Bar 16          Bar 24          Bar 32
   opacity=0.3 ------- 0.65 ------- 1.0  (linear interpolation)

4. User changes envelope in Hit Manager (Bottom Focus Bar when Hit selected):
   - Instant cut (step, no fade)
   - Linear (default)
   - Ease-in / Ease-out / Ease-in-out
   - S-curve / Exponential
5. Per-parameter envelope override available (see Scenario 7).
```

#### Workflow 3 -- Live Capture (During Performance)

```
1. VJ performing, timeline running.
2. VJ presses the Capture Hit button (same unified button as Workflow 1).
3. Hit placed at quantize-derived position with current LIVE state.
4. Quantize setting (adjacent to button) determines exact placement:
   - Exact beat: closest beat to button press time
   - Next bar: snaps to upcoming bar boundary
   - Closest bar: nearest bar, even if user pressed late (could be backwards)
```

### 11.7 Unified Save/Capture Button

ONE button -- same button whether timeline is moving or paused.

| Context | Behavior |
|---------|----------|
| Slot selected (user clicked an empty or occupied beat slot) | Button saves to that slot (Workflow 1) |
| No slot selected (live performance, timeline running) | Button saves at quantize-derived position (Workflow 3) |

The button is HIGHLY VISIBLE in the transport area (large, cannot miss
under performance pressure).

A **3-state quantize toggle** (or dropdown) lives directly adjacent to
the button.

### 11.8 Envelope Visualization on Timeline

Between consecutive Hits, a thin line shows the envelope curve (per
directive 2.7):

| Envelope Type | Visual |
|-------------|--------|
| Linear (default) | Straight line |
| Ease-in / Ease-out / S-curve | Curved line |
| Instant cut | Vertical step |

Line color: hairline `#3a3a3a` by default. Becomes accent blue `#00d9ff`
while currently playing through that segment.

### 11.9 No Drag and Drop

The interaction model is click-to-select + button-to-save only.
NO drag-and-drop from recording lane to Hit lane.

### 11.10 Zoom

Both timelines support click+drag horizontally to zoom IN to the dragged
range. Standard DAW rubber-band-zoom behavior.

Convention: drag on the TIMELINE RULER (the beat/time scale above the
lanes) zooms. Drag on the LANE ITSELF scrubs (recording) or selects (Hits).

**Implications:**

- The two-lane model means the user can see both their programmed show
  (Hits) and their live performance (Recording) at the same time.
- Switching the active driver is a creative choice: "play back my Hits"
  vs "play back my live performance."
- Hit editing never requires the Hit lane to be the active driver --
  editing is always available.

**Cross-references:** Scenario 3 (Hit position conflict / quantize grid),
Scenario 6 (Hit-Signal handoff), Section 5.5 (quantize settings table),
Section 5.6 (envelope defaults table).

---

### Scenario 12 -- Macro Transfer Between Scopes

**The moment:** The user built a Macro (Signal Group) at Clip scope. They
want it at Layer scope so it affects the whole layer. What happens when
they move it? What happens to routings that no longer make sense?

**The resolved answer:**

Default action is COPY when relocating a Macro between scopes.

| Action | Behavior |
|--------|----------|
| Drag (or change scope in Macro Inspector) | Defaults to COPY |
| Original Macro | Stays in its original scope |
| Copy | Created at the target scope |
| Cmd/Ctrl-drag | Explicit MOVE (removes original) |

**Routing behavior when scope changes:**

| Direction | Effect on Routings |
|-----------|-------------------|
| Scope expansion (Clip -> Layer, Layer -> Global) | All routings persist. Broader scope can still reach narrower targets. |
| Scope narrowing (Global -> Layer, Layer -> Clip) | Routings to out-of-scope params become invalid. Filter + warn. |

**Scope narrowing workflow:**

```
User drags Global Macro "BASS PUMP" to Clip scope on Layer 2, Column 3.

BASS PUMP currently routes to:
  - Layer 1 opacity (Global scope can reach Layer 1)
  - Layer 2 scale (Global scope can reach Layer 2)
  - Layer 3 hue (Global scope can reach Layer 3)

At Clip scope on L2-C3, BASS PUMP can only reach L2-C3's params.

Warning dialog:
  "Moving BASS PUMP to Clip scope will lose these routings:
   - Layer 1 opacity (out of scope)
   - Layer 2 scale (layer-level, not clip-level)
   - Layer 3 hue (out of scope)
  
  Only clip-level routings on L2-C3 will survive.
  Continue?"

  [Cancel]  [Continue]

If user continues: invalid routings dropped silently. Valid ones survive.
```

**Library export (separate action):**

- Browser tab MACROS has a "Save to Library" button.
- Saves the Macro (with its current scope binding info) to the app-wide
  library.
- Available in other compositions.
- This matches HANDOFF Q14 ("both" -- composition-owned + library-shareable).

**Worked example:**

```
Scenario: User has a Layer Macro "PULSE FX" on Layer 1.
          They want it on Layer 2 as well.

Action: User drags "PULSE FX" from Layer 1's Macro area to Layer 2.

Result:
  - "PULSE FX (copy)" appears on Layer 2 at Layer scope.
  - Original "PULSE FX" remains on Layer 1.
  - Both are independent copies. Editing one does not affect the other.
  - All routings from the original are preserved in the copy
    (scope expansion: Layer 1 -> Layer 2 is same scope level, no loss).

If user wanted to MOVE instead:
  - Cmd+drag: "PULSE FX" removed from Layer 1, placed on Layer 2.

If user wants it in the library for other compositions:
  - Select "PULSE FX" in browser -> "Save to Library" button.
  - Now available across all compositions.
```

**Implications:**

- Copy-default is the safe option. Users do not accidentally lose a Macro
  by dragging it.
- Scope narrowing always warns before dropping routings. No silent data loss.
- Library export is intentionally separate from in-composition scope
  changes. "Save to Library" is a deliberate action, not a side effect
  of moving a Macro.

**Cross-references:** Scenario 2 (scope precedence),
Section 3.4 (scope precedence rule), Scenario 10 (composition file
portability -- library export is related).

---

## 7. Cross-Reference Matrix

This matrix maps each scenario to the topics it addresses. Use it to
find all scenarios relevant to a specific topic.

### Scenarios by Topic

| Topic | Relevant Scenarios |
|-------|-------------------|
| Layer precedence (Signal vs Hits vs VJ) | 1, 6 |
| Scope precedence (Global > Layer > Clip) | 2, 12 |
| AUTO/OVERRIDE state machine | 1, 6, 7, 8 |
| Hit grid and quantization | 3, 11 |
| Hit creation workflows | 9, 11 |
| Hit envelopes | 7, 11 |
| Mode system (LIVE/PROGRAM/SETUP) | 4 |
| Empty/initial state | 5 |
| Signal computation (runtime formula) | 1, 2, 6, 8 |
| Signal depth | 8 |
| Cuepoints | 9 |
| Clip cells (independence, instances) | 9, 10 |
| Composition files and portability | 10 |
| Library export | 10, 12 |
| Timeline model (two lanes) | 11 |
| Recording system | 11 |
| Macro scope transfer | 12 |
| Orange `#ff4500` override color | 1, 6 (3.3 cross-cutting) |
| Universal field-level rule | 1, 6, 7, 8 (3.1 cross-cutting) |
| Save/Capture button | 11 |
| No drag-and-drop | 11 |

### Topics by Scenario

| Scenario | Topics Covered |
|----------|---------------|
| 1 -- Layer Precedence | Layer precedence, AUTO/OVERRIDE, per-field state, orange color, no global release, naming (AUTO vs "Reclaim") |
| 2 -- Scope Precedence | Scope hierarchy, exclusive routing, same-scope summing, multi-route UI, triangle indicator, mixing-desk model |
| 3 -- Hit Position Conflict | Quantize grid, no-conflict-by-design, P. button removal, section header indicator |
| 4 -- Mode-Switch During Playback | Mode as workspace-only, runtime persistence, layout memory, SETUP immediate apply |
| 5 -- Empty Composition | Pre-first-Hit state, always-on layers, improvisation mode, empty UI defaults |
| 6 -- Hit-Signal Handoff | State model, baseline+modulation, OVERRIDE effect on each system, orange color levels, per-parameter state struct |
| 7 -- Per-Parameter Envelope Override | Field-level inheritance, Hit-level defaults, per-param overrides, envelope fields as overridable fields |
| 8 -- Signal-to-Macro Depth | Per-route depth slider, depth as a field (AUTO/OVERRIDE-able), runtime computation with depth |
| 9 -- Cuepoint vs Hit | Clip cell independence, cuepoints as bookmarks (not Hit refs), clip-start behavior by trigger type |
| 10 -- Composition Portability | File structure (data vs refs), Gather Content bundle, missing media handling, library export scope |
| 11 -- Data Recording vs Hits | Two grids (beat vs FPS), two-lane timeline, lane expansion, scrubbing rules, active driver, three Hit creation workflows, unified button, envelope visualization, no drag-and-drop, zoom |
| 12 -- Macro Scope Transfer | Copy-default, scope expansion/narrowing, routing validity, warning dialog, library export as separate action |

---

## Appendix A: Decision Inventory

Every decision from the 2026-05-22 session, with the scenario where it
is documented. This serves as a completeness check.

### Layer and State Decisions

| # | Decision | Location |
|---|----------|----------|
| 1 | Hits and Live VJ are equal-priority state mutations (last write wins) | Scenario 1 |
| 2 | Signal layer is always-on and computes from current state (never competes) | Scenario 1, 6 |
| 3 | Per-field AUTO/OVERRIDE state machine with two states, two transitions | Scenario 1, Section 3.3 |
| 4 | VJ touch -> field enters OVERRIDE; VJ clicks orange dot -> returns to AUTO | Scenario 1, Section 3.3 |
| 5 | No global release button; per-field release only | Scenario 1 |
| 6 | Top chrome has passive orange STATUS indicator (not clickable) | Scenario 1 |
| 7 | Button label is "AUTO"; mode label pair is "AUTO / OVERRIDE" | Scenario 1 |
| 8 | "Reclaim button" term rejected | Scenario 1, Section 1 |

### Scope Decisions

| # | Decision | Location |
|---|----------|----------|
| 9 | Global > Layer > Clip (outermost wins, exclusive) | Scenario 2, Section 3.4 |
| 10 | Same-scope multi-route: contributions SUM, clipped to [0,1] | Scenario 2, Section 3.4 |
| 11 | Multi-route UI: triangle on row; details via Bottom Focus Bar | Scenario 2 |

### Hit Grid Decisions

| # | Decision | Location |
|---|----------|----------|
| 12 | Hit grid: max 4 per bar = 1 per beat (4/4 time) | Scenario 3, Section 5.5 |
| 13 | Quantize settings: 1, 2, or 4 per bar | Scenario 3, 11 |
| 14 | No two Hits share a grid position | Scenario 3 |
| 15 | P. button removed from individual rows; section headers keep 2px blue left-edge | Scenario 3 |

### Mode Decisions

| # | Decision | Location |
|---|----------|----------|
| 16 | Mode is workspace only -- never affects runtime | Scenario 4 |
| 17 | Playback, audio, Hits, clips, MIDI, OVERRIDE state all persist across mode switches | Scenario 4 |
| 18 | Layout, triangle behavior, inspector exposure, Bottom Focus Bar context change on mode switch | Scenario 4 |
| 19 | SETUP mode changes apply immediately (no commit step) | Scenario 4 |

### Empty State Decisions

| # | Decision | Location |
|---|----------|----------|
| 20 | Signal + Live VJ always active regardless of Hit state | Scenario 5 |
| 21 | Hits are optional (zero-Hit composition is valid) | Scenario 5 |
| 22 | Timeline always advances when transport playing (even with no Hits) | Scenario 5 |
| 23 | Empty composition: dark canvas, BPM "---", bar 1.1.1, all panels visible, LIVE mode, H1 layout | Scenario 5 |

### State Model Decisions

| # | Decision | Location |
|---|----------|----------|
| 24 | Hits and VJ are equivalent state mutations (both write to baseline) | Scenario 6 |
| 25 | Signal computes from current state every frame (baseline + routes * depth) | Scenario 6, Section 3.5 |
| 26 | OVERRIDE blocks Hits but NOT Signals | Scenario 6 |
| 27 | Orange `#ff4500` used at three zoom levels (app, group, field) -- deliberate exception to one-accent rule | Scenario 6, Section 3.3 |

### Envelope Decisions

| # | Decision | Location |
|---|----------|----------|
| 28 | Hit-level envelope is default; per-payload-item can override any subset | Scenario 7 |
| 29 | Unspecified envelope fields inherit from Hit-level | Scenario 7 |
| 30 | Envelope fields themselves are independently AUTO/OVERRIDE-able | Scenario 7 |

### Depth and Route Decisions

| # | Decision | Location |
|---|----------|----------|
| 31 | Per-route depth slider 0-100% | Scenario 8 |
| 32 | Depth is a field: independently AUTO/OVERRIDE-able, writable by Hits and VJ | Scenario 8 |

### Clip and Cuepoint Decisions

| # | Decision | Location |
|---|----------|----------|
| 33 | Each clip cell is an independent instance (same source, different settings) | Scenario 9 |
| 34 | Cuepoints are bookmarks, NOT referenced by Hits | Scenario 9 |
| 35 | Manual trigger starts at cell's in-point; Hit-fired starts at captured playhead | Scenario 9 |

### Composition File Decisions

| # | Decision | Location |
|---|----------|----------|
| 36 | Composition file: self-contained for data, references large binaries by path | Scenario 10 |
| 37 | "Gather Content" creates a portable show bundle (folder format) | Scenario 10 |
| 38 | Missing media: show broken cells + offer re-link (recoverable, not fatal) | Scenario 10 |
| 39 | Macros and Hit Groups are both composition-owned AND library-shareable | Scenario 10 |
| 40 | Library export is a separate "Save to Library" action | Scenario 10, 12 |

### Timeline Decisions

| # | Decision | Location |
|---|----------|----------|
| 41 | Two grids: Hit (beat-quantized) and Recording (60fps, FPS-quantized) | Scenario 11 |
| 42 | Two-lane shared timeline (Hits lane + Recording lane) | Scenario 11 |
| 43 | Lane expansion: hierarchical per-parameter sub-timelines (After Effects model) | Scenario 11 |
| 44 | Recording lane: drag = scrub, click = jump | Scenario 11 |
| 45 | Hits lane: NOT scrubbable, click empty = select slot (highlight, no playhead move), click Hit = jump + select + inspect | Scenario 11 |
| 46 | One active driver at a time (Hits or Recording); switch via lane handle | Scenario 11 |
| 47 | Hit lane always editable regardless of active driver | Scenario 11 |
| 48 | Three Hit creation workflows: scrub-and-save, two-Hit automation, live capture | Scenario 11 |
| 49 | Unified Save/Capture button (one button, context-sensitive) | Scenario 11 |
| 50 | Occupied slot: confirm-replace dialog (NOT merge) | Scenario 11 |
| 51 | Live capture quantize: 3-state toggle (exact beat / next bar / closest bar) | Scenario 11 |
| 52 | Envelope visualization: thin line between Hits showing curve; blue when playing | Scenario 11 |
| 53 | No drag-and-drop from recording to Hit lane | Scenario 11 |
| 54 | Zoom: click+drag on timeline ruler to rubber-band zoom | Scenario 11 |

### Macro Transfer Decisions

| # | Decision | Location |
|---|----------|----------|
| 55 | Default scope-change action is COPY (not move) | Scenario 12 |
| 56 | Cmd/Ctrl-drag for explicit MOVE | Scenario 12 |
| 57 | Scope expansion: all routings persist | Scenario 12 |
| 58 | Scope narrowing: filter + warn dialog before dropping invalid routings | Scenario 12 |
| 59 | Library export via "Save to Library" button in browser MACROS tab | Scenario 12 |

---

*End of FEATURE_CONNECTIONS.md -- Version 1.0, 2026-05-22.*
