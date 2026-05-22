# Audio-DNA Mental Models

> The conceptual spine of Audio-DNA. Read this to understand WHY the
> system works the way it does. For detailed behavioral rules, see
> [FEATURE_CONNECTIONS.md](./FEATURE_CONNECTIONS.md).

Eight core mental models that shape every design decision. Each model
is a framing that, once internalized, makes the detailed rules feel
obvious rather than arbitrary.

---

## Table of Contents

1. [The Three Intent Layers](#1-the-three-intent-layers)
2. [The Three Scopes (Mixing-Desk Hierarchy)](#2-the-three-scopes)
3. [The Universal AUTO / OVERRIDE State Machine](#3-the-universal-auto--override-state-machine)
4. [Hits as Keyframes (Ableton-Arrangement Model)](#4-hits-as-keyframes)
5. [The State-Mutation Model](#5-the-state-mutation-model)
6. [Parallel Data Recording (Verbatim Capture)](#6-parallel-data-recording)
7. [RAW vs SMOOTHED Signal Visualization](#7-raw-vs-smoothed-signal-visualization)
8. [Resolume Grammar + Audio-DNA Soul](#8-resolume-grammar--audio-dna-soul)

---

## 1. The Three Intent Layers

**Tagline:** Three independent decision-makers can tell the visuals
what to do at any moment.

### Plain language

Three separate forces shape the visuals simultaneously:

1. **Signal layer** -- audio drives visuals automatically. You set up
   routes ("bass drives opacity"), press play, and it runs on its own.
2. **Hits layer** -- scheduled events that fire at specific timestamps.
   Pre-programmed: "at bar 16, switch to drop visuals."
3. **Live VJ layer** -- the performer's real-time actions. Dragging
   faders, clicking clips, tweaking effects mid-show.

These are not modes you switch between -- they coexist. The visual
output is always the combined result of all three.

### Analogy: A jazz trio

The drummer keeps steady rhythm (Signal -- always on, driven by the
music). Sheet music says "key change at bar 32" (Hits -- pre-written
events). The soloist improvises over both (Live VJ -- real-time human
decisions). One unified performance, three independent shapers.

### How they compose

Signal READS state every frame and modulates the output -- it never
writes. Hits and Live VJ both WRITE to state (baselines, routes,
values). They are the same kind of action; the only difference is
timing. When the VJ touches a field that a Hit would also control,
the VJ wins via OVERRIDE mode (see [Model 3](#3-the-universal-auto--override-state-machine)).

```
         ┌─────────────┐
         │  VISUAL OUT  │  Signal reads + modulates
         └──────┬───────┘
         ┌──────┴───────┐
         │  PARAM STATE │  baseline + routes + depth
         └──┬───────┬───┘
     writes │       │ writes
    ┌───────┘       └───────┐
    │ Hits (scheduled)      │ Live VJ (real-time)
```

### Worked example

VJing a techno set: bass drives opacity on Layer 1 (Signal, always
running). A Hit at bar 32 fires the "DROP" column and activates a
sidechain-pump macro (Hits). During the drop, you manually crank
intensity to 100% (Live VJ). All three active simultaneously -- audio
pushes opacity, the pre-programmed scene fires on beat, your hand
responds to the crowd.

### What this means for UI

Every UI element operates ON a layer, visualizes a layer's state, or
switches emphasis between layers. Mode tabs (LIVE / PROGRAM / SETUP)
change the workspace and what triangle clicks do -- but they never
turn off the other layers.

### This enables / prevents

- **Enables:** Pre-programmed shows that breathe with live audio and
  can be overridden on the fly. Or pure improvisation with no Hits.
- **Prevents:** Mode confusion and layers silently fighting.

**Litmus test:** Does this design make the layer relationships MORE
visible or LESS?

---

## 2. The Three Scopes

**Tagline:** Master fader beats channel fader beats plugin volume.

### Plain language

Within the Signal layer, routes exist at three levels:

| Scope | What it is | Mixing-desk analogy |
|-------|-----------|---------------------|
| **Global** | Routes affecting the entire composition | Master fader |
| **Layer** | Routes scoped to one layer | Channel fader |
| **Clip** | Routes scoped to one clip | Plugin volume |

When routes at different scopes target the same parameter, **broadest
scope wins**. Global > Layer > Clip. Only one scope is active per
parameter at a time -- this is exclusive, not additive across scopes.

### Why broadest wins (not narrowest)

The opposite of CSS specificity. "I want bass to drive opacity across
the entire show" (Global) is a stronger statement of intent than "this
clip has bass on opacity" (Clip). The performer's higher-level decision
should not be overridden by a detail buried in a clip.

### Same-scope behavior

Multiple macros at the SAME scope routing to the same parameter SUM
their contributions (clamped to [0, 1]). Cross-scope: no addition.

```
  Precedence (EXCLUSIVE -- one scope wins):
  Global ──► if routing, it wins
     │ falls through
  Layer  ──► if routing, it wins
     │ falls through
  Clip   ──► runs if nothing above routes it
```

### Worked example

Global macro routes bass to opacity (depth 0.8). Layer 2 macro also
routes bass to opacity (depth 0.5). Clip macro routes flux to opacity
(depth 0.3). Result: Global wins at 0.8. Remove it, Layer activates
at 0.5. Remove that, Clip takes over at 0.3.

### What this means for UI

The triangle shows which scope controls a parameter. Details (which
macros contribute, which are silent) appear in the Bottom Focus Bar
when the row is selected. See FEATURE_CONNECTIONS.md for scope rules.

### This enables / prevents

- **Enables:** Master override patterns; clean fallthrough.
- **Prevents:** Unpredictable stacking; "where is this coming from?"

---

## 3. The Universal AUTO / OVERRIDE State Machine

**Tagline:** Every field knows whether it follows the show or the
performer's hand.

### Plain language

Every individually-touchable UI field has its own two-state switch:

- **AUTO** (default) -- Hits can write to it, Signal modulates it, the
  show runs as programmed.
- **OVERRIDE** -- the performer touched it. Hits are blocked. Signal
  still modulates around the manually-set value, but the baseline is
  locked to what the performer chose.

Transitions: touch a field --> OVERRIDE. Click the orange dot --> AUTO.

### Analogy: A thermostat with a manual hold

Your thermostat normally follows a schedule (AUTO). Manually set it to
72 degrees and it enters "hold" -- ignoring the schedule until you
press "resume." Each room has its own independent hold/schedule state.
The HVAC (Signal) keeps running regardless, targeting whatever the
current setpoint is.

### Why field-level

If OVERRIDE were per-layer, grabbing one fader would freeze everything
on that layer. Field-level means the performer controls exactly what
they touched and nothing more.

### The orange signature

Orange `#ff4500` means "manually held" everywhere. It appears at three
zoom levels -- a deliberate exception to the single-accent rule:

| Level | Visual | Meaning |
|-------|--------|---------|
| App-wide | Orange indicator in top chrome | Overrides exist somewhere (passive) |
| Group | Orange tint on layer/section header | This group has overridden fields |
| Per-field | Orange dot at row end | This field is OVERRIDE (click to release) |

Blue = active/alive/routing. Orange = manually held. Two colors, two
meanings.

### Worked example

1. Opacity in AUTO. Hit at bar 16 sets it to 0.8. Bass modulates
   +/-0.2. Output oscillates 0.6--1.0.
2. You drag opacity to 1.0. OVERRIDE. Orange dot appears.
3. Hit at bar 32 tries to set 0.5. Blocked. Opacity stays at 1.0.
4. Bass still modulates: output oscillates 0.8--1.0 around your value.
5. Click orange dot. Returns to AUTO. Hit's 0.5 takes effect.

### What this means for UI

Inspector rows need space for the orange dot (invisible in AUTO, shown
only in OVERRIDE). The performer never wonders "why isn't this
changing?" -- the orange dot is the answer. See FEATURE_CONNECTIONS.md
for the full state machine.

### This enables / prevents

- **Enables:** Selective manual control; safe improvisation; explicit
  release (one click per field).
- **Prevents:** Implicit mode surprises; ghost overrides.

---

## 4. Hits as Keyframes

**Tagline:** Hits are not switches. They are keyframes with envelopes,
like Ableton's arrangement view.

### Plain language

A Hit is a **keyframe** -- a point on the timeline that says "the
composition should be in THIS state at THIS moment," with a
configurable transition (envelope) describing HOW it gets there from
the previous state. Place two keyframes, choose a curve, and the
software interpolates between them.

### Analogy: After Effects keyframes

Place opacity at 100% on frame 0 and 0% on frame 60. The software
draws a curve between them -- linear, eased, or stepped. You set the
endpoints and choose the shape. Hits work identically: bar 16 opacity
1.0, bar 32 opacity 0.5, linear interpolation between them.

### Envelope anatomy

| Setting | Options |
|---------|---------|
| **Onset** | Instant / Fade over N bars or beats |
| **Curve** | Linear (default) / Ease-in / Ease-out / S-curve / Exponential / Step |
| **Release** | Instant cut / Fade-out before next Hit |

Default between consecutive Hits: **linear** (Ableton convention).
Individual parameters inside a Hit can override any envelope field --
same field-level independence as Model 3.

**Interpolation happens during playback, not scrubbing.** Scrubbing
the recording shows captured state per frame. Playing the Hit timeline
computes interpolated values live.

**Hits live on a beat-quantized grid** (max 4 per bar in 4/4) because
they represent musical events -- drops happen on beats, buildups start
on bars.

### Worked example

Programming a buildup-to-drop:
1. **Bar 24:** intensity 0.3, filter open. Envelope: linear.
2. **Bar 31:** intensity 0.9, filter half-closed. Envelope: ease-in.
3. **Bar 32:** intensity 1.0, strobe on, fire DROP column. Envelope:
   instant.

Between 24--31, intensity ramps along an ease-in curve. At 32,
everything snaps instantly. The timeline shows: curved line from 24 to
31, vertical step at 32.

### What this means for UI

Envelope curves are visible as thin connecting lines between Hit pills
on the timeline. Straight = linear. Curved = ease. Vertical = instant.
See FEATURE_CONNECTIONS.md for envelope rules.

### This enables / prevents

- **Enables:** Musical transitions (ramps, snaps, fades); visual
  timeline readability; per-parameter precision.
- **Prevents:** Step-function-only shows; needing dozens of Hits for a
  simple ramp.

---

## 5. The State-Mutation Model

**Tagline:** Hits and Live VJ are both writers to the same state.
Signal reads every frame and never competes.

### Plain language

Every controllable parameter has a small piece of state:

```
Per-parameter state:
  baseline       = current set value (0-1)
  active_routes  = set of {signal, depth} pairs
  mode           = AUTO or OVERRIDE
```

**Hits** and **Live VJ** both WRITE to this state (set baselines,
toggle routes). They are the same kind of action -- only timing
differs. Last write wins.

The **Signal layer** READS state every frame and computes output:

```
output = clamp( baseline + SUM(signal_value * route_depth), 0, 1 )
```

Signal never writes. It cannot conflict with Hits or VJ decisions.

### Analogy: A whiteboard with markers

Hits and the VJ both hold markers and can write a new value on the
whiteboard at any time (last write wins, unless OVERRIDE blocks the
Hit's marker). The Signal layer stands next to the board with a
calculator -- reads every value 60 times per second, adds signal
contributions, announces the output. Never touches the markers.

### Worked example

Layer 1 opacity, bar 15: baseline = 0.3, route = {BASS, depth 0.4}.

```
BASS signal = 0.7:
  output = clamp(0.3 + 0.7*0.4, 0, 1) = 0.58

Hit at bar 16: sets baseline = 0.8, adds {FLUX, depth 0.2}

BASS = 0.7, FLUX = 0.5:
  output = clamp(0.8 + 0.7*0.4 + 0.5*0.2, 0, 1)
         = clamp(1.18, 0, 1) = 1.0  (clamped)
```

### What this means for UI

The inspector row shows the baseline (value column). The triangle
shows signal contribution (pulsing brightness). The combination is
visible in the effect on screen. See FEATURE_CONNECTIONS.md for edge
cases.

### This enables / prevents

- **Enables:** Clean, predictable runtime. Same state = same output.
  Transparent formula for debugging.
- **Prevents:** Signal competing with Hits; mysterious output values.

---

## 6. Parallel Data Recording

**Tagline:** Recording captures everything frame by frame. Hits
curate. Both can become a video.

### Plain language

Two parallel systems live on the timeline:

1. **Hits** -- the editorial layer. Discrete keyframes at musical
   positions. The curated show.
2. **Recording** -- the verbatim layer. Continuous capture at 60fps of
   every action. Everything the performer did, exactly as they did it.

They coexist. You can have both simultaneously. You can render video
from either.

### Analogy: Sheet music vs. a live recording

The pianist has sheet music (Hits -- curated, portable, editable) and
a recording of their performance (Recording -- faithful, complete).
Both represent the same piece.

### What recording captures

Everything, timestamped at frame precision (~16.67ms): Hit firings,
VJ touches, signal values, parameter changes, clip triggers. This is
lightweight data (not video) -- like MIDI recording for the visual
side.

### Two grids, one timeline

| System | Grid | Resolution at 120 BPM |
|--------|------|----------------------|
| Hits | Beat-quantized | 1 beat = 500ms, max 4/bar |
| Recording | FPS-quantized | ~16.67ms/frame, 30 frames/beat |

Both lanes share the same horizontal position. One active driver fires
events during playback; both are always visible.

**Hit derivation from recording:** Scrub a recording to a moment you
like, save that state to a Hit at a chosen beat position. Mine live
performances for keyframes after the fact.

### Worked example

Perform a 5-minute set. Recording captures everything at 60fps. You
fire a few Hits at key moments. After performance, scrub the recording
to bar 48 where you did something cool. Click the Hits lane at bar 48,
press "Save to Hit." That moment is now a reusable keyframe. Load the
Hit Group for your next show -- the curated version plays back with
clean transitions.

### What this means for UI

Timeline shows both lanes (thin, expandable). Recording uses time
labels. Hits uses beat labels. Both always visible. See
FEATURE_CONNECTIONS.md for timeline rules and creation workflows.

### This enables / prevents

- **Enables:** "Perform first, curate later." Two rendering options.
  Continuous capture with zero overhead.
- **Prevents:** Losing happy accidents. Forced choice between
  recording and programming.

---

## 7. RAW vs SMOOTHED Signal Visualization

**Tagline:** Triangles pulse the raw truth. Columns show the readable
trend. Same signal, two treatments.

### Plain language

Two visual elements show signal values with deliberately different
smoothing:

1. **Triangles** (next to every parameter) -- RAW. Brightness = signal
   value 0 to 1, no easing, no transitions. Kick hits, triangle hits
   full brightness instantly. Signal drops, triangle drops instantly.
2. **Vertical signal columns** (knob replacements) -- SMOOTHED. EMA
   with alpha 0.3. The fill rises and falls gently, showing the
   trend without transient spikes.

Same underlying signal. Both visible simultaneously.

### Analogy: Peak meter vs. VU meter

Professional studios used both: a peak meter (needle jumping to exact
instantaneous level -- precise but jittery) and a VU meter (averaged
level -- smooth and readable). Triangle = peak meter. Column = VU
meter.

### Why the split

- **"Is the signal hitting NOW?"** -- look at the triangle.
- **"What's the overall energy?"** -- look at the column.

If both were raw, columns would be jittery and unreadable across a
dark club. If both were smoothed, you'd lose instant kick-hit feedback.

### The formulas

```
Triangles:  display = signal_value           (direct 1:1)
Columns:    smoothed = prev*(1-0.3) + signal*0.3   (EMA, alpha=0.3)
```

### Worked example

Bass signal during 128 BPM four-on-the-floor:

| Moment | Signal | Triangle | Column fill |
|--------|-------:|---------:|------------:|
| Kick hit | 1.0 | 1.0 | ~0.65 |
| +50ms | 0.3 | 0.3 | ~0.55 |
| +100ms | 0.0 | 0.0 | ~0.39 |
| Next kick | 1.0 | 1.0 | ~0.57 |

Triangle snaps between bright and dark. Column gently bobs, showing
energy without strobe-like flicker.

### What this means for UI

Triangles: no CSS transitions ever. Direct 1:1 brightness mapping.
Columns: EMA formula in code, not CSS transitions (different curves).

### This enables / prevents

- **Enables:** Two readability levels in one glance. The interface
  "feels" the music.
- **Prevents:** Jittery noise (columns) and laggy feedback (triangles).

---

## 8. Resolume Grammar + Audio-DNA Soul

**Tagline:** Borrow the structure of a proven tool. Replace the
controls with an audio-first paradigm.

### Plain language

Audio-DNA borrows Resolume Arena's STRUCTURE but replaces its CONTROLS.
Same language (grammar), different message (soul).

**From Resolume (structural grammar):**
- Clip grid layout, inspector row format, section headers with
  disclosure triangles, layer strips, tab-based panels, dense control
  surface visual language

**Replaced by Audio-DNA (audio-first soul):**
- All knobs --> vertical signal columns
- Every parameter --> triangle indicator pulsing with signal
- 22px signal quick-ref bar always visible (Resolume has nothing like
  this)
- BPM at 48px dominant (Resolume does not emphasize BPM)
- The interface breathes with the music (Resolume's UI is mostly
  static)

### Analogy: A car with a different engine

Same body (proven aerodynamics, familiar dashboard layout). Different
engine (electric powertrain with new gauges for battery and motor
state). The driver recognizes the car and finds the controls, but
every gauge and readout is about the new powertrain.

### The industrial brutalist aesthetic

**Structure** (clean, machined): square corners everywhere, 1px
hairlines, no shadows/gradients, buttons touch, IBM Plex Mono for
numbers, 3% SVG grain as the only decorative element.

**Soul** (live, breathing): blue `#00d9ff` = active/alive/routing,
orange `#ff4500` = manually overridden, triangles pulsing raw, columns
filling smoothed, beat wheel turning, the whole interface visualizing
audio state in real time.

### Worked example

A Resolume user opens Audio-DNA:
1. **Clip grid** -- familiar. 100x80px cells, thumbnails, names.
2. **Inspector** -- familiar. Labels, values, sliders, +/- buttons.
3. **Triangles pulsing with music** -- new. Click one: signal routing
   menu. "I can route audio to any parameter."
4. **Signal columns where knobs were** -- new. Live audio energy fills
   up like a meter with a value tick. "I see audio AND my value."
5. **Top bar: BPM at 48px, 15+ live signal values, structural state
   pill** -- all new. None of this in Resolume.

### The 2-second test

Boris's benchmark: in a dark club at 128 BPM, a performer knows the
full system state within 2 seconds. BPM (largest element), structural
state (blue pill), which params are signal-driven (pulsing triangles),
audio energy (signal bar + columns), which clips are playing (blue
borders). Resolume grammar provides the familiar map; Audio-DNA soul
makes the audio visible at every zoom level.

### What this means for UI

Building a new component: first ask "Does Resolume have a structural
equivalent?" Match its layout. Then: "How does Audio-DNA's paradigm
modify it?" Add the triangle, the signal column, the signal-awareness.
See AUDIO_DNA_DIRECTIVE_FULL.md Part 7 for the full differentiator
list.

### This enables / prevents

- **Enables:** Fast VJ onboarding (familiar grammar); Audio-DNA's
  unique value (signal-first controls); design consistency.
- **Prevents:** Reinventing solved UI patterns; losing Audio-DNA
  identity in borrowed grammar; style drift.

---

## How the Models Relate

```
  CONCEPTUAL LAYER           WHAT IT ANSWERS
  ─────────────────────────  ─────────────────────────────
  Models 1-2: Architecture   WHO controls visuals + WHERE?
  (Intent Layers + Scopes)

  Models 3-5: Behavior       HOW do controls interact?
  (AUTO/OVERRIDE +           State machine, envelopes,
   Keyframes + Mutation)     data flow formula.

  Models 6-7: Representation WHAT does the user see/capture?
  (Recording + RAW/SMOOTHED) Parallel systems, dual viz.

  Model 8: Aesthetic         HOW does it look and feel?
  (Resolume + Soul)          Borrowed structure, new controls.
```

Most design decisions touch 2-3 models:

- "Follow Hit timeline or VJ's hand?" -- Models 1 + 3
- "What scope for this macro?" -- Models 2 + 5
- "How should this transition look?" -- Models 4 + 6
- "Smoothed or raw indicator?" -- Model 7
- "Layout for a new panel?" -- Models 8 + 1

---

*For behavioral rules: [FEATURE_CONNECTIONS.md](./FEATURE_CONNECTIONS.md).
For the complete spec: [AUDIO_DNA_DIRECTIVE_FULL.md](./AUDIO_DNA_DIRECTIVE_FULL.md).*
