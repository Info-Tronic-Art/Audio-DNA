# Audio-DNA VJ — Complete Design & Architecture Directive

> For the Claude Code agent working in the repo. This is the definitive reference. It captures Boris's design vision, aesthetic preferences, interaction model, system architecture, and per-mockup feedback. All decisions here are final unless Boris overrides in-session.

---

## Overview

Audio-DNA is a VJ (video jockey) performance application. It analyzes live audio in real time (42+ audio features) and lets users route those audio signals to control visual parameters — effects, transforms, clip behaviors, anything.

**The design philosophy in Boris's words:** "Clean and easy to use like a high-performance car. All controls present, nothing overwhelming." And: "Resolume-style industrial aesthetic — dense, minimal, practical like a machine."

These are not contradictions. They define the tension every design decision resolves: **density without clutter, power without chaos.** The reference is a cockpit or a high-end mixing desk — everything has a place, everything is reachable, nothing decorates.

**In one sentence:** A VJ mixing desk, not a creative app — dense, functional, machine-like, with Resolume Arena's exact visual grammar but Audio-DNA's audio-analysis soul.

> **Last updated 2026-05-22** — Parts 4.1, 4.3, 5.6, 5.7, and 10 revised per
> the session-locked decisions in `FEATURE_CONNECTIONS.md` and
> `MENTAL_MODELS.md`. See those documents for the canonical behavioral spec.

## Table of Contents

- **Overview**
- **Part 1: Signal System Architecture** — Three-tier model, triangle, vertical columns, scopes, structural state
- **Part 2: Hit System** — Timed cues, payloads, creation workflow, envelopes, pill design
- **Part 3: Pulse** — AI assistant
- **Part 4: Visual Design System** — Colors, typography, hard rules, aesthetic principles
- **Part 5: Layout Components** — Top chrome, focus bar, inspector grammar, clip cells, layer strip
- **Part 6: Layout Modes** — 5 Performance + 2 Programming + 8 Hybrid layouts with mockup URLs
- **Part 7: Audio-DNA Differentiators** — What sets Audio-DNA apart from Resolume
- **Part 8: Rejected Patterns** — What NOT to implement
- **Part 9: Resolved Decisions Log** — Quick reference of all decisions by category
- **Part 10: Resolved Decisions** — All open questions resolved with session-locked decisions

---


---

## Part 1: Signal System Architecture

This is the most important system in the application. Everything else serves it. The core idea: **audio drives everything visual. Every display reflects live audio state. Static is the exception.**

### 1.1 Three-Tier Signal Model

**Sources** — Raw audio features extracted in real time: bass, mid, high, flux, centroid, RMS, peak, LUFS, sidechain pump, swing, formant, resonance, reese bass, LFOs, and more (42+ total). These are the building blocks. Resolume has ~3 audio features. Audio-DNA has 42+. This is the core differentiator.

**Signals** — Derived from combining and processing sources. A signal is not just "bass" — it can be "bass + flux, smoothed, inverted." Multiple inputs can be mixed, chained, and wired together through processing to create a single output. More than one audio source can be combined to make a signal.

**Signal Groups (Macros)** — A packaged collection of signals wired into a processing chain with a single defined output. Modeled directly after Ableton Live's Effect Racks: a container holding a chain of signal processing (like an effects chain) where the input is one or more audio sources, the chain modifies the signal through stacked processing, and the output is a single clean signal ready to deploy. One macro control can drive many parameters simultaneously — one input signal, many controls. These are like Ableton's macro knobs but in vertical column form.

Signal groups are named, saveable, movable, and reusable:
- A signal group takes one or more audio sources as input
- The chain modifies the signal through stacked processing (smoothing, inverting, scaling, gating, etc.)
- The output is a single clean signal ready to deploy to any number of parameters
- One signal group output can fan out to any number of parameters across any number of layers
- Any parameter can receive from multiple signal groups simultaneously
- **No limit on routing — users can route indefinitely**
- Signals can be chained and wired in collections to form a signal group
- Each signal group has an output and can be quick-selected and deployed

### 1.2 Three Scopes

Signal groups exist at three levels:

| Scope | Owns | Affects |
|-------|------|---------|
| **Clip** | Signal groups local to that clip | That clip's parameters only |
| **Layer** | Signal groups for the layer | Layer-wide parameters |
| **Global** | Signal groups for the entire composition | Any parameter anywhere |

Any macro can be moved between scopes, saved as a preset, and redeployed elsewhere. Every clip, layer, and global has signal groups (macros) where any macro can be moved and saved. Macros can have input sources of signals or a wired-up mix of signals (a chain).

### 1.3 The Triangle — Universal Signal Access Point

Every single parameter in the application has a small triangle icon to its left. This is the universal gateway to signal routing. The triangle is simultaneously:
- A **status indicator** (does this parameter have a signal routed?)
- A **quick-action menu** (LIVE mode)
- A **doorway** into the full programming surface (PROGRAM mode)

**Triangle states:**

| State | Appearance |
|-------|-----------|
| Unrouted | Very light outline only (~`#3a3a3a`), barely visible — just enough to know it's clickable |
| Selected / menu open | Solid light fill (~`#888`) |
| Signal active | Blue `#00d9ff` fill, **brightness = signal value 0→1 in real time** |

The active state is a direct 1:1 mapping — signal value IS brightness. No easing, no transitions, no animation curves. When the kick hits, the triangle hits full brightness instantly. When the signal drops to zero, it drops to zero instantly. Raw signal. Every active triangle in the interface pulses with the music.

**This means you can scan across any inspector or layer strip and instantly see which parameters are driven by signals and which are manual.** The pulsing triangles draw your eye to exactly where the audio is flowing. Without reading a single label, you can see: "opacity is driven, scale is driven, hue shift is driven — rotation is manual, blur is manual."

The brightness tracking signal strength means you see intensity, not just on/off. A triangle driven by bass pumps hard on every kick. One driven by gentle high-frequency centroid softly flickers. The visual rhythm of the triangles IS the signal.

**Triangle interaction by mode:**

| Mode | Click behavior |
|------|---------------|
| **LIVE** | Quick menu pops up showing all signals already set up for one-click assignment. Menu hierarchy for browsing all available signals. Fast inline routing without leaving performance. |
| **PROGRAM** | Opens the full signal programming workspace (see Section 1.6). Each signal group can be programmed right in live mode by clicking the triangle next to each parameter — it has access to all setup signals on quick menu and all signals through a menu hierarchy. |
| **SETUP** | Configuration-level access to signal definitions, thresholds, source setup. |

### 1.4 Vertical Signal Columns (Knob Replacement)

**There are zero circular knobs anywhere in the entire application.** Every knob — including the dashboard controls (INTENS, COLOR, ZOOM, SPEED, WARP, GLOW, etc.) — is a vertical signal column. This is the knob, just in a new vertical format. It is the universal control element.

Each column is a dual-purpose element:

**Background:** Live signal strength animating in real time — a mini VU meter showing what the audio source is doing right now. Fills from bottom to top with blue `#00d9ff`, brightness proportional to signal value. The power of the signal is seen as a blue column as it moves.

**Foreground:** A horizontal tick/line (small line indicator) the user drags up or down to set the parameter value (0–100). Sits on top of the animated signal fill. Shows how high it is set from 0 to 100.

**MIDI/OSC indicator:** Always visible on every column. This is the control surface — always showing its hardware binding.

**The result:** You always see the relationship between "where I set this" and "what the signal is doing" in a single glance. It looks like an LFO display with a control line inside it. The user can look at the macros and look at the source next to each other and see everything going on on that layer without needing to click inside the layer properties.

**Where columns appear:**
- **Dashboard section:** replaces the circular knob row entirely — same vertical column format
- **Layer strip left side:** all active signal groups for that layer are lined up as vertical columns on the left side of the layer controls
- **Inspector:** anywhere a knob would traditionally appear
- Each source column sits next to the destination columns it controls
- **Auto-populated:** the moment a user routes a signal, the column appears. Remove the route, the column disappears. All used signals are displayed automatically for quick reference and usage as soon as a user creates them.
- **Scroll behavior:** columns scroll horizontally. Room for 4–6 visible at a time per layer.

**Reading a layer at a glance:** You see "BASS is driving opacity, scale, and effect wet. LFO is driving rotation and hue shift." Source columns next to destination columns, all animating in real time. No clicking into property panels required. The layer strip is a living patch diagram.

**In the bottom focus area / macro area:** Instead of just a knob, the signal columns show the MIDI or OSC setting of the "knob" with a small line. The power of the signal is seen as a blue column as it's moving.

### 1.5 Programming Mode Layout

The dedicated workspace for building signal groups:

```
┌────────────────────────────────────────────────────┐
│                   TOP CHROME (98px)                 │
├──────────┬──────────────────────────┬──────────────┤
│          │     CLIP + CLIP          │              │
│  ALL     │     CONTROLS             │  ALL         │
│  SIGNALS │     (center-left/right)  │  PARAMETERS  │
│  (left)  ├──────────────────────────┤  (right)     │
│          │     SIGNAL CONTROLS      │              │
│          │     (below center)       │              │
├──────────┴──────────────────────────┴──────────────┤
```

- Left panel: all available signals — global, layer, and clip level (sources and derived)
- Center: clip and clip controls (left and right of center)
- Below center: signal controls for building chains
- Right panel: all available parameters — global, layer, and clip level (destinations)

This is where signal groups get built — like building an effects chain in Ableton Live. The user wires signal sources on the left to parameter destinations on the right, with processing controls in the center.

### 1.6 Global Signal Drawer

The 22px signal quick-ref bar in the top chrome is interactive:

- **Collapsed (default):** Horizontal strip showing 15+ live signal values at a glance — the feature Resolume doesn't have
- **Expanded:** Click the bar and it drops down as a full-height drawer, **overlaying the entire workspace below it** (dismiss to return). The user can interact with global signals, configure signal preferences, and manage global-scope signal groups.

### 1.7 Bottom Focus Bar

A bar docked at the bottom of the screen. Same expand/collapse interaction pattern as the signal drawer.

- **Collapsed (default):** Shows a summary of whatever is currently selected (e.g., "MACRO: BASS PUMP → 4 params" or "EFFECT: Ripple → L2-C3")
- **Expanded:** Click to expand upward into a detail editing surface for the selected item — macro controls, effect parameters, signal chain editing, Hit programming. Uses the same inspector grammar as the rest of the app.
- **Height:** User-draggable, but defaults to roughly the bottom ~30% of screen (reference: the Audio Analysis / Active Routes area in `v9_brut_stacked_var_b.html`).
- **Context-sensitive:** If you click a macro, the focus area shows that macro's parameter controls. If you click an effect, it shows the effect's controls. The bottom area to the right of the preview window is a focus window — if something is selected in the top-right menu, its logical unit is displayed at the bottom focus area.

The workspace lives between these two drawers — top signal bar overlays down, bottom focus bar expands up. Both get out of the way when not needed.

### 1.8 Three Levels of Signal Visualization

The interface shows signal state at three zoom levels simultaneously — all visible, no clicking required:

| Level | Element | What it shows |
|-------|---------|---------------|
| **Global** | 22px signal quick-ref bar | Aggregate values for all 15+ audio features |
| **Macro** | Vertical signal columns on each layer | Per-signal-group output + mapped destinations |
| **Parameter** | Triangle next to every control | Which individual parameters are alive, pulsing with signal strength |

This means the entire interface becomes a signal visualization at every level. Three zoom levels of the same truth, all live, all visible without clicking. The interface breathes with the music.

### 1.9 Structural State Detection

Audio-DNA detects the current structural state of the music in real time:
- **DROP** / **BUILDUP** / **BREAKDOWN** / **NORMAL**
- Displayed as a blue pill on the top bar — this feature is not in Resolume
- **This is reactive, not predictive** — the system knows "we are IN a drop right now" but cannot predict "a drop is coming in 8 bars"
- Structural state is a label and a signal source, but the user handles timing through the Hit system — they know their tracks, they place Hits where they want events to fire

### 1.10 Per-Layer Signal Display

For each layer, the signals are displayed at the left side of the layer controls. The user can look at the macros and the source next to each other and see everything going on on that layer without needing to click inside of the layer properties.

Under each signal, the user sees the source — for example, if an LFO or bass signal is one of the signals used to control three or four parameters, or if it's used in macros, the user can see the macro and the source next to each other. These are like macros — one input signal and many controls for it.

---

## Part 2: Hit System (Timed Event Cues)

Hits are timed cues that give temporal control to the entire composition. Signals are always on — Hits determine WHEN things happen. Hits are the central organizational unit of the entire show.

**Terminology distinction — this is important:**
- **Cue point** = a positional marker on an individual clip (Resolume-style, exists in the clip inspector)
- **Hit** = a timed event trigger on the song/show timeline with a full control surface underneath — a show-level compositional tool

The name "Hit" was chosen because it's musical (hard hit / soft hit = magnitude built in), action-oriented (hits *fire*, they don't just sit), short (3 chars), and universally understood by music producers. Not "click" (conflicts with click tracks). Not "Q" (could be confused with queue or cue points).

---

### 2.1 What a Hit Is

A Hit is a point on the timeline that captures composition state — like a keyframe in animation, but for the entire VJ composition. It is a sentence in the language of the show.

**A Hit captures three independent payload categories:**

| Payload Type | What it captures |
|--------------|------------------|
| **Signals** | Which signal sources are active (bass, flux, LFOs, derived combos). Determines which audio features are being tracked and made available. |
| **Clips** | What content is firing in cells: individual cell triggers, column triggers (scene changes across all layers), and the effects baked into those clips. |
| **Macros (Signal Groups)** | Which signal-group routing chains are active — e.g., bass→opacity, flux→strobe, sidechain→pump. |

**Signal-as-automation of macros:** A signal connected to a macro can drive the macro's own parameters like an automation lane — for example, an LFO modulating the depth of a PULSE macro over 4 bars. This connection can flow **one-way** (signal → macro, done) or **loop** (cyclical / feedback). The macro itself becomes a dynamic, modulated thing rather than a static patch.

**Beyond these three core categories, a Hit also encodes:** parameter values at the captured moment, effects on/off state, envelope settings, and chain links to other Hits.

**Example: A single Hit at the drop can:**
- Activate the **BASS** + **SIDECHAIN PUMP** + **DROP LFO** signals (signal-level)
- Fire the "DROP" column → entire visual composition changes across all layers instantly (clip-level)
- Activate the "DROP MACRO" → the newly-active signals now route to opacity, strobe, scale, hue (macro-level)
- All from one position on the timeline — three independent layers of change coordinated in one moment

**Each Hit's control surface uses the same inspector grammar as the rest of the app** (label / value / − / + / slider / triangle). Under each Hit is a rock-solid interface to control anything in the composition.

---

### 2.2 System Hierarchy

```
Source      → raw audio feature (always extracted)
Signal      → derived / combined feature, can be activated or muted
Macro       → signal group = processing chain that routes signals to parameters
Hit         → keyframe snapshot of composition state (signals + clips + macros + envelope)
Hit Group   → choreographed timeline of Hits = full programmed show
```

Sources are the alphabet. Signals are the words. Macros are the phrases. Hits are the sentences (with timing and inflection). Hit Groups are the script.

---

### 2.3 Composition Architecture (What a Hit Captures From)

The composition has three nested scopes. Each scope is self-contained — its macros drive its own effects. This is the architecture Hits snapshot from.

| Scope | Contains | Macros control |
|-------|----------|----------------|
| **Global** | Global effects + global macros | Global effect parameters |
| **Layer** | Layer effects + layer macros + the clips on that layer | Layer effect parameters |
| **Clip** | Video / video-like element + effect stack + clip macros | Clip parameters (everything inside the clip) |

**Layering rules:**
- Layers stack vertically — each layer composites on top of layers below
- A **video** in a layer is opaque — it blocks/replaces what's beneath
- An **effect** in a layer is transparent — it modifies whatever output the layers below produced
- A **cell** exists at the intersection of (layer, column) — it can't exist outside a layer
- A **column** is a vertical slice through all layers — firing a column = scene change across the whole stack
- **Global effects** apply to the entire composed output, after all layers compose

**A clip cell can contain:**
- A video (or video-like element) alone
- An effect alone (generative content with no underlying video)
- A video with an effect stack on top (composite)

The clip is the triggerable unit. What's inside it is flexible. The clip's own macros drive its parameters.

---

### 2.4 Hit Creation Workflow

**There is one way to create a Hit, and it works identically in LIVE and PROGRAM modes:**

1. Position the playhead (running in LIVE, manually placed in PROGRAM)
2. Arrange the composition the way you want it at this moment (load clips, activate signals, enable macros, set effects)
3. Press **Capture Hit**
4. The Hit captures the active composition state at the playhead position
5. A **filter step** appears showing what was captured — user unchecks anything they don't want this Hit to control

Anything unchecked is left unaffected by this Hit — the previous state persists.

**Critical architectural principle:** A Hit can ONLY trigger things that already exist in the composition. You cannot "add" something to a Hit that isn't already in your composition — the composition is the source of truth, the Hit is just a snapshot of its state at a moment.

This means there is no "drag-to-attach" or "build a Hit from scratch" workflow. To change what a Hit fires, you change the composition's state at that Hit's timecode, then re-capture (or edit the captured items via the filter).

---

### 2.5 Diff-Based Override Semantics

Each Hit is a partial state change, not a complete state replacement. Hits compose by layering — later Hits override earlier Hits **only for parameters they explicitly touch.**

**Clip-scope macros** automatically discard when the clip changes (different clip = different macros, no carryover needed).

**Layer-scope and Global-scope** persist unless explicitly overridden by a new Hit. The filter step is critical here — if a Hit captured a layer effect being on, but the user unchecks it in the filter, that Hit doesn't change the layer effect state. The previous state persists.

**Per-parameter override:** Each parameter inside a Hit is independent. If Hit A sets opacity=0.5 and Hit B (later) sets only hue=red, opacity stays 0.5. Only explicitly-touched parameters change.

**Pre-Hit state:** Nothing plays before the first Hit unless the VJ manually triggers something live. No default state at position 0.

---

### 2.6 Hit Behaviors

| Behavior | Description |
|----------|-------------|
| **Trigger** | One-shot spike. Something fires at that moment. |
| **Gate (on/off)** | Turns a signal on or off. Can be set to snap or fade/ramp — configurable per Hit. |
| **Signal-dependent** | Only fires if a signal condition is met (e.g., "only when bass > 0.7"). Acts as a gate at a certain volume to allow chained effects underneath. |
| **Chained** | One Hit triggers another, forming sequences. |

---

### 2.7 Hits as Keyframes — Envelope System

Hits are not step-functions — they are **keyframes with envelopes**. Each Hit has transition settings that control how the composition animates TO its state from the previous Hit's state.

**Per-Hit envelope settings:**

| Setting | Options |
|---------|---------|
| **Onset** | Instant / Fade-in over X bars or beats |
| **Curve** | Linear / Ease-in / Ease-out / Ease-in-out / Exponential / S-curve / Step |
| **Release** | Instant cut / Fade-out before next Hit |

**Behavior by payload type:**
- **Clip triggers** are always instant (you can't half-fire a clip). The layer's own fade-in setting handles the clip's visual appearance.
- **Signal activations** can fade in (signal influence ramps from 0 to full over onset time).
- **Macro routes** can fade in similarly.
- **Parameter values** interpolate along the chosen curve between Hit states.
- **Effect activations** fade with the layer's own fade settings.

**Per-parameter envelope override:** The Hit-level envelope is the default, but individual parameters inside the Hit can override. A Hit might use "fade 4 beats / ease-out" as its default, but specify "macro route changes instantly" and "opacity fades over 8 beats with S-curve" for individual items.

**Timeline visualization:** Between two Hits with envelopes, draw a thin line connecting them showing the curve. Instant cuts = vertical step. Smooth fades = curved line. The show's animation becomes visible at a glance on the Hit lane.

---

### 2.8 Hit Composition (Parallel, Sequential, Grouped)

**Parallel** — Multiple Hits at the same timestamp, each controlling different things. A drop moment: clip fires, bass→opacity activates, strobe triggers, scale punches — all simultaneously. Stacked Hits at the same beat can intentionally coexist without conflict because of diff-based override (each owns its own slice of parameters).

**Sequential** — Hits chained in order as a choreographed sequence. A buildup: Hit 1 starts filter sweep, Hit 2 adds hue rotation 4 bars later, Hit 3 fires the drop clip 8 bars later.

**Hit Groups** — A working sequence saved as a reusable group. Build a "DROP SEQUENCE" once, deploy it anywhere on the timeline. **A Hit Group can represent an entire programmed show for a specific track** — load it, play the track, follow along.

**Conflict resolution:** When two Hits at the same bar both touch the same parameter, the later Hit wins for that specific parameter. Untouched parameters from either Hit persist independently.

---

### 2.9 Hit Pill — Visual Representation on the Timeline

Every Hit is rendered as a **uniform vertical pill** on the timeline. The pill stays the same size for every Hit — only the indicators inside change.

```
┌─┐
│●│  ← S (Signals activated)
│●│  ← C (Clips fired — single cells, columns, or scenes)
│●│  ← M (Macros activated)
└─┘
```

**Pill specifications:**
- Small vertical capsule shape (~8px wide × ~24px tall on the timeline)
- 1px hairline `--border` outline
- Dark fill background
- Three indicators inside, vertically stacked
- Each indicator: blue `#00d9ff` when its payload type is present in the Hit, dim/invisible when not
- No border-radius (square corners, per design system)

**Hover state:** Reveals a small summary tooltip showing the Hit's contents — which signals, which cells/columns, which macros, scope (clip/layer/global), envelope settings, and which items are checked vs unchecked in the filter.

**Selected state:** Pill border becomes 2px solid blue.

**Active state (firing right now during playback):** Pill flashes full blue fill for the duration of the onset, then settles back to indicator state.

**Three indicators max.** Beyond that the pill becomes unreadable. The three payload categories (Signal / Clip / Macro) are the canonical organizational units.

---

### 2.10 Hit Chains on the Timeline

Chained Hits (one Hit triggering another) are NOT indicated inside the pill. Instead, they're shown visually on the timeline as **a thin connecting line or arc** between the chained Hit pills. This communicates the chain relationship spatially — much clearer than a flag inside a single pill.

---

### 2.11 Hit Anatomy in the UI (Bottom Focus Bar)

When a user selects a Hit, the bottom focus bar expands and shows the Hit's full anatomy:

- **Signals captured** — which signals are activated/deactivated
- **Clips captured** — which specific cells fire on which layers, and which columns trigger as scene changes
- **Macros captured** — which signal groups activate/deactivate
- **Parameter snapshots** — any specific parameter values to set
- **Envelope** — onset duration, curve type, release behavior (with per-parameter overrides)
- **Power / intensity** (0→1) — magnitude of the Hit's effect
- **Timing** — exact bar/beat/sub-beat position on the timeline
- **Conditions** — signal-dependent gating rules
- **Chain links** — which other Hits this Hit triggers next (and timing offsets)
- **Filter checkboxes** — what's included vs excluded from the captured state

The Hit becomes the most important inspector view in the app for show programming.

---

### 2.12 Hit and the Clip Grid (Column Triggers)

The clip grid columns (intro / verse / drop / break / etc. — see H3 layout) become a **palette of scenes** that Hits draw from. The columns aren't just organizational labels — they're triggerable units that Hits can fire as wholesale scene changes.

Firing a column = firing all cells in that column across all layers = full visual scene change in one action. This is the most powerful single content trigger.

---

### 2.13 Waveform Hit Lane (from v9_wave_02_focus)

The waveform display can show a Hit lane below the stereo pair. In this lane:
- Hit pills are placed at their timeline positions
- User can double-click a Hit pill to open it for editing
- If programming ahead of time, the user places Hits precisely
- If performing in real time, there could be a pattern applied to the Hit lane (like 2-2-3-3) that auto-places rhythmically
- Hit presets come in 1, 2, 4, 8, 16, 32, 64 bar patterns
- The goal is to make this as user-friendly and non-confusing as possible

---

### 2.14 Hits as Show Programming

Users can program a show ahead of time using:
- **A real audio track as the timeline** — for a 5-minute song at 120 BPM, there are a certain number of positions. The user captures Hits at key song moments. They will fire in time with the music and trigger the sequence of events attached.
- **A synthetic waveform** (e.g., sine wave at 120 BPM) to program a house set — the user doesn't need a real track
- Hit presets and patterns saved and recalled for different performances
- Hits can be loaded and triggered manually, or placed on the timeline to fire automatically

**The performer's workflow:** Build a Hit Group for each track (essentially a cue sheet — VJ choreography that follows the music). At performance time, play the track and either trigger Hits manually or let them auto-fire at their timeline positions. The show is pre-choreographed but still live.

---

### 2.15 Pattern Presets

Pre-built rhythmic Hit spacings available in bar lengths: 1, 2, 4, 8, 16, 32, 64 bars. Also irregular patterns (e.g., 2-2-3-3). These can come as presets and can multi-affect many other signals, macros, and parameters.

---

### 2.16 Data Recording (Parallel System)

A separate but related system runs alongside the Hit timeline: **continuous data recording** of every VJ change.

**What it captures:** Every clip trigger, signal activation, macro toggle, parameter sweep — timestamped, lightweight (data only, not video). Like MIDI recording for the visual side.

**How it relates to Hits:**
- Data recording captures the raw performance continuously
- Hits are the editorial layer on top — discrete keyframes at notable moments
- The user can record a session and place Hits at notable moments to define "the show"
- Or skip recording and just place Hits manually for a programmed show
- The system can suggest Hit positions at moments of significant state change in the recording

**Rendering:** Video output can be rendered from either:
- The recording (exact replay of the performance, including all micro-movements)
- The Hits (clean programmed playback, with envelope interpolations between snapshots)

Hits-based rendering is more compact and editable. Recording-based is faithful to the live performance.

---

### 2.17 DJ Software & Hardware Integration (Future)

Potential capability: connecting to DJ software and hardware to see what the DJ has playing in real time — the actual track, BPM, position. If available, Hits can be placed with knowledge of the full track structure. Most users will have only the recorded audio or a pre-loaded waveform.

---

## Part 3: Pulse — AI Assistant

The AI assistant is named **Pulse**. It helps users navigate, manage, and configure the application.

### 3.1 Location & Interface

- **Pulse button** in the top right of the interface
- **Chat interface** — Pulse communicates through a conversational chat UI
- Can also occupy the Signals tab area in the inspector — when Pulse is not active, that tab shows two rows of category headings: Files, Effects, Sources, Compositions, Recordings, MilkDrop
- When Pulse IS active, it replaces those headings and navigates all categories via chat

### 3.2 Capabilities

- **Content navigation:** Browses and surfaces sources, effects, MilkDrop presets, clips, compositions, recordings — navigates all aspects of the content library
- **Signal routing suggestions:** Can suggest signal→parameter mappings
- **Settings management:** Works through settings/show files — modifications Pulse makes may appear to change the interface but are actually loading a settings or show file. This means Pulse can configure complex setups by generating and loading config files.
- **Chat-based interaction:** Users type requests, Pulse responds and acts

### 3.3 When Pulse Is Inactive

If the user is not using Pulse, the AI button area opens up two rows of headings for manual browsing: **Files | Effects | Sources | Compositions | Recordings | MilkDrop**. The user browses these categories manually.

---

## Part 4: Visual Design System

### 4.1 Color System

| CSS Variable | Value | Usage |
|-------------|-------|-------|
| `--bg` | `#1a1a1a` | App background (Resolume-exact) |
| `--panel` | `#2a2a2a` | Panel fills (one step lighter) |
| `--section-header` | `#252525` | Section header strips (subtle separation) |
| `--separator` | `#383838` | Row separators (hairline context) |
| `--border` | `#3a3a3a` | All hairline borders (1px everywhere) |
| `--label` | `#888888` | Dim labels |
| `--value` | `#e0e0e0` | Bright values / active text (off-white) |
| `--accent` | `#00d9ff` | **Cyan/blue — THE ONLY accent color** |
| `--danger` | `#ff4040` | REC indicator + errors only |
| `--override` | `#ff4500` | OVERRIDE state indicator — fields VJ has manually overridden during performance. Intentional second meaningful color in the design system (exception to the "one accent" rule, per session decision). |

**Override color exception:** `#ff4500` (orange) is the SECOND meaningful color
in the design system, used EXCLUSIVELY to indicate fields in OVERRIDE state
(see Universal AUTO/OVERRIDE rule in Part 4.3 and FEATURE_CONNECTIONS.md). This
is a deliberate exception to the "one accent only" rule. It appears at three
zoom levels: app-wide STATUS indicator in top chrome, group-wide indicators
(layer strip headers, section headers), and per-field orange dot at the end of
each row. All same color, same semantic ("manually held, not following the
show"), all clickable to release that scope.

**Accent color history (for context):**
- Original v2/v3: `#00d9ff` (cyan)
- v9_ten series: Changed to `#5fdba7` (mint/Resolume green)
- Pre-v9_ten: `#ff4500` (orange)
- **Final decision: returned to `#00d9ff` (cyan/blue)** — Boris prefers blue

**Accent usage rules (strict):**

**Blue `#00d9ff` ON:**
- Playing clip fill + 2px border
- BPM number (48px, dominant)
- Structural state tag (pill)
- Beat wheel current segment
- Transport play button when active
- Selected row highlight
- Just-changed 200ms flash
- Horizontal bar slider position indicator
- Active triangle fill (the universal per-parameter routing indicator)
- Section header left-edge bar when section has active routing
- Patch-bay selected route line + anchor dots
- Signal column fills (live signal visualization)
- Active triangles (pulsing with signal)
- Active Hit markers on timeline

**Blue `#00d9ff` NEVER:**
- Static labels
- Inactive buttons
- Background fills
- Decorative elements
- Unselected rows

**One accent color. No exceptions. No rainbow UI.**

### 4.2 Typography

**Font stack:**
```css
--font-mono: 'IBM Plex Mono', 'SF Mono', Menlo, monospace;
--font-sans: 'IBM Plex Sans', -apple-system, sans-serif;
```
Plex Mono is the deliberate choice for the "instrument panel" feel. These replaced the earlier `-apple-system` only stack.

**Size hierarchy:**

| Element | Size | Weight | Font | Color |
|---------|------|--------|------|-------|
| BPM (hero number) | 48px | 500 | Mono | blue `#00d9ff` |
| Genre / structural state | 20–24px | 400 | Sans | — |
| Section headers | 12px | 400 | Sans, uppercase-first | `--value` |
| Row labels | 11px | 400 | Sans | `--label` |
| Row values | 11px | 400 | Mono | `--value` |
| Clip names | 9px | 400 | Sans | — |
| Sparkline labels | 10px minimum | 400 | Sans (cannot go below 10px) |
| Signal quick-ref bar | 11px | 400 | Mono | — |

**Typography rules:**
- ALL-CAPS for section headers and mode labels (`LIVE`, `PROGRAM`, `SETUP`, `DYNAMICS`, `SPECTRAL`)
- Monospace ONLY for numbers — never for text labels
- Letter-spacing on all-caps labels: `0.5–1px`
- No italic, no underline (except the mode toggle active-state blue underline)

### 4.3 Hard Visual Rules

These are absolute. No exceptions:

- **No border-radius** — zero, everywhere, no exceptions. Square everything.
- **No box-shadows** — any element
- **No gradients on chrome** — gradient only acceptable inside SVG thumbnails or waveform rendering
- **No circular knobs** — every knob in the app is a vertical signal column, including dashboard controls
- **1px borders only** — hairline `--border` (`#3a3a3a`). Never thick borders for decoration.
- **Buttons touch** — no gaps between adjacent buttons
- **3% SVG grain** — noise overlay on body, `opacity: 0.03`. The only decorative element. Prevents flatness from feeling sterile.
- **No emoji / decorative icons** — only functional glyphs: `▶ ‖ ■ ▼ ≡ M S V`
- **No multiple accent colors** — one color only
- **No rounded corners anywhere, ever**

### 4.4 Core Aesthetic Principles

1. **Machine over decor** — every pixel serves a function. No decorative borders, no gradient chrome, no subtle glows for style.
2. **One accent, used precisely** — blue `#00d9ff` signals "active / alive / routing" and nothing else.
3. **BPM is the north star** — the 48px blue BPM number is always the largest, most prominent piece of information. The eye goes there first.
4. **Density without overwhelm** — the "high-performance car" principle: everything present, nothing surprising. Consistent row heights, consistent label positions, consistent button sizes.
5. **Resolume grammar, Audio-DNA soul** — the inspector, sections, and clip grid follow Resolume exactly. The top bar, signal bar, and audio analysis panels are Audio-DNA's identity.
6. **Square everything** — no border-radius. No exceptions. Buttons touch each other without gaps.
7. **3% grain** — the only decorative element.
8. **Hairlines only** — 1px `#3a3a3a` borders.
9. **Monospace for numbers** — Plex Mono makes meters, BPM, values, and times feel like instrument readouts.
10. **Audio drives everything visual** — the UI is a window into the audio analysis engine. Every display reflects live audio state.

---

## Part 5: Layout Components

### 5.1 Fixed Top Chrome (98px total — never changes between designs)

```
┌──────────────────────────────────────────────── 44px ─┐
│ Logo │ BPM 48px BLUE │ BAR │ PHRASE │ STATE PILL       │
│ Beat Wheel │ Transport │ Quantize │ Fade │ Master │REC │
│ FPS │ [LIVE] [PROGRAM] [SETUP]    [layout hotkeys] [⚡]│
├──────────────────────────────────────────────── 32px ─┤
│ TRANSPORT: ▶ ‖ ■  128 − + |◂ ▸| /2 ×2 TAP RESYNC ↺  │
├──────────────────────────────────────────────── 22px ─┤
│ SIGNAL BAR (clickable → overlays full workspace)       │
│ RMS│PEAK│LUFS│FLUX│CENTROID│BASS│MID│HIGH│...          │
└──────────────────────────────────────────────────────┘
```

The three-strip top bar is an Audio-DNA differentiator:
- **Top bar (44px):** BPM at 48px blue is THE visual anchor — the first thing the eye goes to. The largest element on screen.
- **Transport strip (32px):** Resolume-style, below top bar (not integrated into it — separate strip)
- **Signal quick-ref bar (22px):** 15+ live audio values always visible — this is the feature Resolume doesn't have. Clickable to expand as full-workspace overlay.
- **Layout template hotkeys:** Top right of header bar — user saves layout templates and switches instantly with hotkeys
- **Pulse AI button:** Top right (⚡ or similar icon)

### 5.2 Bottom Focus Bar

Docked at the bottom. Thin bar showing summary of current selection. Expands upward on click. Default expanded height ~30% of screen (user-draggable). Context-sensitive — shows detail for whatever is selected above.

See Part 1, Section 1.7 for full spec.

### 5.3 Consistent Expand/Collapse Pattern

The app uses one interaction pattern for progressive disclosure throughout:

| Element | Location | Collapsed | Expanded |
|---------|----------|-----------|----------|
| Global signal drawer | Top (under chrome) | 22px bar with live values | Overlays full workspace, drops downward |
| Bottom focus bar | Bottom | Thin summary bar | Expands upward, ~30% default, draggable |
| Waveform | Where placed | Condensed stereo pair | Click opens larger waveform control menu below it |
| Signal bar (above clips) | Above clip grid | Mini horizontal bar | Click opens expanded signal controls |

### 5.4 Mode System

| Mode | Tab appearance | Purpose |
|------|---------------|---------|
| `LIVE` | Blue text + 2px blue underline | Performance — triggering, faders, clip grids, quick signal assignment via triangles, quick Hit placement |
| `PROGRAM` | Same when active | Full signal group building — routing, wiring, macro creation, Hit programming |
| `SETUP` | Same when active | Configuration — signal definitions, thresholds, source setup |

**Mode-switching remembers state.** If the user is in LIVE using layout P1, switches to PROGRAM, then switches back to LIVE, the app returns to P1. Each mode remembers its last-used layout.

### 5.5 Layout Templates

Layouts serve as **both starting templates and customizable workspaces**:
- Start from any template
- Customize panel positions and sizes
- Save as named templates
- Assign hotkeys for instant switching (hotkey buttons in top right of header bar)
- **Default layout on launch:** H1 (`v9_brut_stacked_var_b.html`)

### 5.6 Inspector Rows (Resolume-exact grammar — used everywhere)

This is the most specific and settled part of the design. Every inspector row looks identical:

```
[▶ signal triangle] [Label 90px right-aligned #888] [Value 50px mono #e0e0e0] [−][+] [━━━┃━━━ 14px]
```

- Triangle: signal access point, pulsing with signal (see Section 1.3)
- Label: right-aligned, 90px column, Plex Sans 11px, `#888`
- Value: 50px, Plex Mono 11px, `#e0e0e0`
- `−` and `+` buttons: 20×20px square, 1px hairline
- Bar slider: fills remaining width, 14px tall, dark track with blue position indicator line (vertical hairline)

**Routing affordance:** Per-parameter signal routing happens via the Triangle
on each row (left side). No "P." button on rows — that pattern was removed in
the 2026-05-22 session in favor of triangle-only routing. See
FEATURE_CONNECTIONS.md Scenario 3 for context.

**This grammar is used EVERYWHERE:** inspector panels, effect controls, signal group editors, Hit control surfaces, bottom focus bar detail view. One visual language for the entire app. A user who learns the inspector once can read any control surface.

### 5.7 Section Headers (Resolume-exact)

```
▼ Section Name
```

- `▼` triangle: 7px clickable disclosure
- Name: Plex Sans 12px, `#e0e0e0`, uppercase first letter only
- 24px tall strip, `#252525` background
- **2px blue left-edge bar** when the section contains any active routing

**No section routing affordance:** The "route entire section" feature was
removed in the 2026-05-22 session. Section headers only carry the passive 2px
blue left-edge indicator showing routing is present somewhere in the section.
For multi-parameter routing, the VJ uses combined parameters (e.g., X-Y
position as a single combined param controlled by an X-Y signal) — see
FEATURE_CONNECTIONS.md Scenario 3.

**Standard sections (in order, all collapsible):**
Dashboard → Autopilot → Layer → Video → Transition → Transform → Effects → Cuepoints → Transport

### 5.8 Inspector Tabs

The right-side inspector uses tabs: **CLIP | LAYER | COMP | SIGNALS**

Active tab: blue text + blue 2px bottom underline. Inactive: `#888` text.

These tabs can fold down (collapse). Whatever is displayed in the center (output, layer, clip), its parameters are displayed to the right in the appropriate tab.

### 5.9 Clip Cells (base: 100×80px)

Every design uses 100×80px as the base unit. Designs that need more triggering surface scale UP (e.g., 84×52 in Performance Focus for DJ cue pads). They never scale below 80×60.

**Anatomy:**
```
┌──────────────────────┐
│▌                     │  ← 6px left: per-clip mini audio waveform (dim blue)
│▌   thumbnail         │  ← appropriate visual for the content
│▌                     │
├──────────────────────┤
│ clip-name  9px       │  ← 14px bottom bar, hairline top border
└──────────────────────┘
```

- Left 6px: per-clip mini audio waveform, dim blue color
- Main area: **thumbnail shows whatever is appropriate for the content** — rendered frames from the clip, geometric patterns, MilkDrop previews, etc. They should look like what they are.
- Bottom 14px: clip name, Plex Sans 9px, hairline top border

**States:**

| State | Visual |
|-------|--------|
| Playing | 30% blue fill + 2px solid blue border |
| Queued | 2px dashed blue border |
| Selected | 1px solid blue border, no fill |
| Loaded | 1px `--border` solid |
| Empty | 1px dashed `--border` border |

### 5.10 Layer Strip

Each layer row includes:

- **Left side:** vertical signal columns showing all active signal groups for that layer (auto-populated, scrolls horizontally, 4–6 visible at a time)
- **Signal columns:** source next to destinations, all animating in real time
- **Triangle indicators:** next to every parameter, pulsing with signal brightness
- The layer strip is a living patch diagram — readable at a glance without opening property panels

### 5.11 Browser Panel

A library panel on the right side where users pull items into the composition, layer, or clip. Tabs:

- **FILES** — file browser
- **FX** — effects library with thumbnail previews
- **SOURCES** — source material
- **COMP** — compositions
- **RECORD** — recordings
- **MILKDROP** — MilkDrop presets

Signal thumbnails in the browser use **static waveform shapes** — not animated.

When Pulse AI is active, it replaces the category headings and navigates all of the above via chat. When not using Pulse, the headings show as two rows of browsable categories.

### 5.12 Content Organization

- Columns in the clip grid can be named with **labels** for song structure: intro, verse, drop, break, etc.
- These are organizational labels only (not tied to a track's actual timeline)
- Users can apply custom tags to content (e.g., "dark", "techno")
- Content is searchable and filterable by tag
- User can have custom tags to organize their content and search by tag

### 5.13 Waveform Display

Multiple display modes for the audio waveform. **Mode selector buttons on the right side of the waveform** let the user switch between views:

- **Condensed stereo pair** — clean, compact (as seen in `v9_brut_crt_layout.html`)
- **DJ view** — styled like professional DJ equipment (CDJ-style scrolling waveform with beat grid, color-coded frequency bands)
- Additional modes TBD (spectrum, overview, etc.)

Waveform features:
- Clickable — clicking the waveform opens a larger waveform control menu below it
- Hit markers visible on the waveform timeline (Hit lane below stereo pair)
- Horizontal signal bar can sit directly under the waveform
- Same expand behavior for the horizontal signal bar above the clips in its mini form

### 5.14 Central Display Area

The center of the interface shows the output, preview, layer, or clip — with tabs to switch between them. Whatever is displayed, its parameters appear in the right-side inspector.

In some layouts:
- Output image on the left with cue points under it
- Source monitors on the right with their own mini preview windows
- Preview monitor is movable and resizable in some performance modes

### 5.15 Display & Output Configuration

- **Primary interface** lives on one screen for best performance
- **Video output** uses GPU multi-output capabilities — smart routing to projectors, LED walls, etc. via dedicated video card outputs
- Future: DJ software/hardware integration for real-time track data

---

## Part 6: Layout Modes

All layouts share identical top chrome and bottom focus bar. Each targets a specific workflow. Layouts are both **starting templates** and **customizable** — users can modify, save, and assign hotkeys.

**Default on launch:** H1 (`v9_brut_stacked_var_b.html`)

### Performance Modes (LIVE)

- **P1** `file:///Users/boriskarpman/projects/RealTimeAudio/design/mockups/html/v9/v9_ten_03_performance_focus.html`
  Central clip/output/layer display with tabs for each. Parameters on right connected to whatever is displayed. This is LIVE mode #1. Other live modes exist.

- **P2** `file:///Users/boriskarpman/projects/RealTimeAudio/design/mockups/html/v9/v9_brut_vj18_04.html`
  Output monitor can be moved around and resized. Needs more controls at the left display for signals.

- **P3** `file:///Users/boriskarpman/projects/RealTimeAudio/design/mockups/html/v9/v9_brut_modular_02.html`
  Right side: preview image with controls under it and to the right. Clip/layer/comp/signal menu under preview window. Browser for all materials to the right. Needs Pulse AI button top right.

- **P4** `file:///Users/boriskarpman/projects/RealTimeAudio/design/mockups/html/design_08_widescreen.html`
  Widescreen performance mode.

- **P5** `file:///Users/boriskarpman/projects/RealTimeAudio/design/mockups/html/v3_design_01_cinema.html`
  Cinema-style performance mode.

### Programming / Setup Modes

- **S1** `file:///Users/boriskarpman/projects/RealTimeAudio/design/mockups/html/v9/v9_ten_02_programming_studio.html`
  Program mode: all signal parameters displayed on the left, all global/layer/clip parameters displayed on the right. Full routing workspace.

- **S2** `file:///Users/boriskarpman/projects/RealTimeAudio/design/mockups/html/v9/v9_brut_wave_03_inverted_l.html`
  "Perfect" layout. Clip/layer/comp/signal tabs are good. Missing: deeper controls for layers and a preview monitor for output and clip (should go top-left under the audio waveform). Audio signals bottom-left can also be horizontally displayed just under the waveform. When user clicks waveform, larger waveform control menu opens below. Same expand for the horizontal signal bar above clips in mini form.

### Hybrid / Multi-Purpose

- **H1 (DEFAULT LAUNCH LAYOUT)** `file:///Users/boriskarpman/projects/RealTimeAudio/design/mockups/html/v9/v9_brut_stacked_var_b.html`
  **Near-perfect interface. The default layout on app launch.**
  - Left side shows the signals that are running along with all controls. Could be a better left control box.
  - Clean and contained deck tabs.
  - Colors are good in the layout (replace red/orange areas with blue from `file:///Users/boriskarpman/projects/RealTimeAudio/design/mockups/html/v4/v4_16_inverted_l.html`).
  - Clip/layer/comp/signals tabs can be folded down.
  - Bottom focus area to the right of the preview window — contextual detail (click macro → focus shows its params).
  - The right side bottom area is for another type of focus — important to figure out.
  - Signal columns replace all knobs: show MIDI/OSC setting with small line, signal power as blue column moving.
  - Signals tab discussion: could house Pulse AI, or Pulse gets own button and signals tab shows category headings (Files/Effects/Sources/Compositions/Recordings/MilkDrop).

- **H2** `file:///Users/boriskarpman/projects/RealTimeAudio/design/mockups/html/v9/v9_ten_10_wide_inspector.html`
  Bottom tabs: Composition/Layer/Clip click open everything within. Library on right side for pulling items into composition/layer/clip. Good Hit-points and beat snap look.

- **H3** `file:///Users/boriskarpman/projects/RealTimeAudio/design/mockups/html/v9/v9_ten_08_modular_tiles.html`
  Grid outline section to the left with columns having names that can be set for sections: intro, verse, drop, break, etc. User can have custom tags to organize content. Search by tag.

- **H4** `file:///Users/boriskarpman/projects/RealTimeAudio/design/mockups/html/v9/v9_final_01_brutalist.html`
  Clean waveform. Very nice layout for horizontal signals directly under waveform. Good inspector and browser panels on right. To improve: add bottom focus area, saved presets in bottom center and bottom right, move grid down (change layer controls), center becomes preview/display window.

- **H5** `file:///Users/boriskarpman/projects/RealTimeAudio/design/mockups/html/v8/update/v8_20_vj17.html`
  Great layout and look. A lot to work with. Add output and you have a great focus area at the bottom. Room on left under preview/live windows. Lots of room for clips. Missing: signals for each layer to the left of its layer controls.

- **H6** `file:///Users/boriskarpman/projects/RealTimeAudio/design/mockups/html/v9/v9_state_04_wf_max.html`
  Very clean. Output image on left with Hit-points under it. Different sources on right with their own mini monitors. Great top section.

- **H7** `file:///Users/boriskarpman/projects/RealTimeAudio/design/mockups/html/v2_design_03_audio_first.html`
  Dual screens center for output/preview. Blue color liked. Menu underneath, left/right columns for signals and menus. Good audio signal location. One of the best setups. Needs bottom focus + favorites area.

- **H8** `file:///Users/boriskarpman/projects/RealTimeAudio/design/mockups/html/v3_variations/touch_var_a.html`
  Clean look. Good top area and sizing.

### Additional Layout Notes

- `file:///Users/boriskarpman/projects/RealTimeAudio/design/mockups/html/v2_design_07_preview_centered.html` — If preview monitor is pushed right and right menu moved to right of left grid area, it might give lots of area for menus and controls.

### Visual Quality References (specific elements liked — not full layouts)

- `file:///Users/boriskarpman/projects/RealTimeAudio/design/mockups/html/v9/v9_brut_crt_layout.html` — Clean condensed stereo waveform pair. Good grid thumbnails.
- `file:///Users/boriskarpman/projects/RealTimeAudio/design/mockups/html/v9/v9_brutalist_v2c_dual.html` — Grid thumbnail style.
- `file:///Users/boriskarpman/projects/RealTimeAudio/design/mockups/html/v9/v9_brutalist_v2b_program.html` — Grid thumbnail style.
- `file:///Users/boriskarpman/projects/RealTimeAudio/design/mockups/html/v9/v9_alt_resolume_b_01.html` — Good thumbnail style for premade signal previews. Use as signal thumbnail reference.
- `file:///Users/boriskarpman/projects/RealTimeAudio/design/mockups/html/v9/v9_01_sidebar_focus.html` — Signal display in right menu.
- `file:///Users/boriskarpman/projects/RealTimeAudio/design/mockups/html/v9/v9_state_02_both_max.html` — General look good. Waveform needs multiple display modes.
- `file:///Users/boriskarpman/projects/RealTimeAudio/design/mockups/html/v9/v9_wave_02_focus.html` — Great waveform display: stereo pair condensed with Hit/keyframe lane below. User can tap to place events. Double-click Hit for deeper controls. Adjustable Hit power per position. Pattern presets (2-2-3-3). Hit presets 1/2/4/8/16/32/64 bars. Can multi-affect signals/macros/params. Design this to be as user-friendly and non-confusing as possible.
- `file:///Users/boriskarpman/projects/RealTimeAudio/design/mockups/html/v4/v4_16_inverted_l.html` — Blue color reference `#00d9ff`. Use this blue accent to replace red/orange in H1.

### Earlier Design Session Favorites (from v2/v3/v8/v9 mockup reviews)

- `v9_brut_vj18_01.html` — Best clip cell sizing and dimensions.
- `v9_brut_vj16a.html` — Per-clip mini audio waveform on left side + small timeline at bottom of clip. Highly specific detail Boris loved.
- `v9_brut_vj17.html` — Best top section arrangement.
- `v9_brut_d6_modular.html` — Top favorite. Very clean. Best dedicated signals bar implementation.
- `v9_brut_resolume_b_01.html` — Top favorite (just needs waveforms added to top).
- `v9_brut_sidebar_focus.html` — Best condensed left sidebar (tightly condensed, pushed to edge).
- `v9_brut_audio_first.html` — Best expanded left sidebar for deep signal analysis (220px, full audio analysis).
- `v9_brut_three_column.html` — Best cue/autopilot placement (docked right side).
- `v9_brutalist_01_console.html` — Good compact waveform approach for minimized vertical space.
- `v2_design_03_audio_first.html` — Good programming/setup mode.
- `v9_state_03_sig_max.html` — Good programming/setup mode for routing signals to parameters.
- `v9_brutalist_v2a_performance.html` — Good setup mode linking workflow.
- `v9_brutalist_v2b_program.html` — Good setup mode linking workflow.
- `v8_20_vj17.html` — Cleanest overall look of any mockup.
- `v9_brut_wave_05_dense.html` — Good dense grid visual approach.
- `v9_brut_modular_02.html` — Good modular grid visual approach.

---

## Part 7: Audio-DNA Differentiators vs Resolume

These features distinguish Audio-DNA and must be **visible on screen at all times** — never hidden in menus:

| Feature | Design Implication |
|---------|-------------------|
| 42+ audio features (vs Resolume's ~3) | Signal quick-ref bar always visible (22px strip). Audio analysis sidebar in expanded layouts. |
| Real-time genre detection | Genre displayed prominently on top bar / audio panel |
| Structural state (DROP/BUILDUP/BREAKDOWN/NORMAL) — reactive only | Blue pill tag always on top bar — this is not in Resolume |
| Sidechain pump, swing, formant, resonance, reese bass | Available as signal sources everywhere via triangle menus, signal cards, signal chip drag sources |
| Pulse AI assistant (chat interface) | Top right button + signals tab area |
| BPM 48px dominant display | The biggest number on the interface — eye goes there first |
| Beat wheel (4-segment circle) | Always visible on top bar |
| Bar/phrase counter | Always visible alongside BPM |
| Signal groups (Ableton-style racks) | Layer strips, programming mode, triangle quick menus |
| Vertical signal columns (zero circular knobs) | Every control surface in the app — the universal control element |
| Pulsing signal triangles on every parameter | Everywhere — the interface breathes with the music |
| Hit system (timed show cues with full payloads) | Waveform timeline, signal controls, bottom focus bar |
| Content tagging + structural grid columns | Clip grid organization |
| Layout templates + hotkeys | Top right header bar |
| DJ software/hardware integration (future) | Real-time track data when available |

---

## Part 8: Rejected Patterns (Do Not Implement)

### Layout approaches rejected
- Ribbon/strip metaphor — not enough visual hierarchy
- Command palette (Ctrl+Space) — too hidden, VJ needs everything visible
- Timeline view — wrong mental model for clip-based VJ
- Touchscreen-optimized layout — not the primary use case
- Quad-split view — too rigid for variable content
- Cinema/preview-dominant — sacrifices control surface for aesthetics
- Symmetrical layout — forced symmetry fights VJ workflow asymmetry

### Visual rules rejected
- Rounded corners — anywhere, ever
- Box shadows — any element
- Gradient fills on UI chrome
- Multiple accent colors
- Emoji or decorative icons
- Orange `#ff4500` as a general-purpose accent — retired. *Exception:* re-introduced 2026-05-22 EXCLUSIVELY as the `--override` color token (OVERRIDE state indicator). Never use for any other purpose. See Part 4.1.
- Mint accent `#5fdba7` — replaced by blue
- Italic or underline text

### Control patterns rejected
- Circular knobs of any kind — all replaced by vertical signal columns
- Traditional rotary controls
- Hidden signal state — signals must always be visible
- Manual-only controls with no signal visualization
- Dashboard knobs — replaced by vertical signal columns

### Inspector patterns rejected
- Label ABOVE value (horizontal pair only)
- Large knobs in inspector rows (vertical columns only)
- Color-coded per-parameter backgrounds (too noisy)
- Dashed lines for unselected patch-bay routes (use solid gray 1px)
- Patch-bay anchor dots under 12px (too small to grab)

---

## Part 9: Resolved Decisions Log

All items below have been decided and are integrated into the relevant sections above. Listed here for quick reference, organized by category.

### Visual Design

| Decision | Resolution |
|----------|------------|
| Accent color | `#00d9ff` cyan/blue (single accent — no rainbow UI) |
| Accent history | Started cyan → became orange `#ff4500` → became mint `#5fdba7` → returned to cyan `#00d9ff` |
| Circular knobs | Zero in entire app — all replaced by vertical signal columns |
| Clip thumbnails | Appropriate to content (not forced monochrome) |
| Signal thumbnails | Static waveform shapes in browser (not animated) |

### Layout & Interaction

| Decision | Resolution |
|----------|------------|
| Default layout on launch | H1 (`v9_brut_stacked_var_b.html`) |
| Mode-switching | Remembers panel state per mode (LIVE→PROGRAM→LIVE returns to last LIVE layout) |
| Layout system | Templates + customizable + hotkeys in top-right header |
| Display setup | Interface on one screen; GPU multi-output for video output |
| Bottom focus bar | Defaults ~30% screen height, user-draggable |
| Global signal drawer | Overlays entire workspace when expanded |
| Content columns | Organizational labels only; searchable/filterable by tags |

### Signal System

| Decision | Resolution |
|----------|------------|
| Signal column scroll | Horizontal, 4–6 visible per layer |
| MIDI/OSC indicator | Always visible on every column |
| Signal-as-automation | Signals can modulate a macro's own parameters. One-way or looped. |
| Structural state | Reactive only (current state), not predictive |

### Hit System

| Decision | Resolution |
|----------|------------|
| Name | "Hit" (not "click" or "Q") |
| Payload categories | Three: Signals + Clips (cells/columns) + Macros (signal groups) |
| Pill indicators | Three dots inside vertical pill: S / C / M. Uniform footprint, hover for detail. |
| Creation workflow | Single method — Capture Hit button records active composition state at playhead; filter step lets user uncheck unwanted captures. Same in LIVE and PROGRAM. |
| Composition state | The composition is the source of truth — Hits can ONLY trigger what already exists in it |
| State persistence | Diff-based override per parameter. Clip macros reset with clip change. Layer/Global persist unless explicitly overridden. |
| Pre-first-Hit state | Composition dormant until a Hit fires or VJ triggers something live |
| Conflict resolution | Later Hit wins per-parameter. Untouched parameters from earlier Hit persist. |
| Hit as keyframe | Hits have envelopes: onset (instant/fade), curve (linear/ease/exponential/step), release |
| Hit Groups | Reusable choreographed sequences. A Hit Group can represent a full programmed show for a track. |
| Hit groups storage | Inside Hit programming mode |
| Gate behavior | Configurable snap or fade/ramp per Hit |
| Data recording | Parallel system. Continuous lightweight data capture. Hits are editorial layer on top. |

### AI Assistant

| Decision | Resolution |
|----------|------------|
| Name | Pulse |
| Interface | Chat-based |
| Mechanism | Works through settings/show files — modifies the interface by loading configurations |

### Waveform

| Decision | Resolution |
|----------|------------|
| Display modes | Multiple, with selector buttons on the right side of the waveform |
| Required modes | Condensed stereo pair, DJ/CDJ-style view, additional modes TBD |

---

## Part 10: Resolved Decisions

All previously open questions have been resolved during planning sessions
through 2026-05-22. Resolutions below; see `FEATURE_CONNECTIONS.md` for
worked examples and detailed behavior. The session notes that produced
these decisions are in
`.audit/directive-reconciliation/CONNECTION_DECISIONS_2026-05-22.md`.

| # | Original Question | Resolved Decision |
|---|------------------|-------------------|
| 1 | Default Hit envelope settings | LINEAR (Ableton-style default). User can change to instant cut / ease-in / ease-out / ease-in-out / S-curve / exponential / step via the Hit Manager in the bottom focus bar. See FEATURE_CONNECTIONS Scenario 11 Workflow 2. |
| 2 | Signal-to-macro modulation depth control | Per-route depth slider (0–100%), one per route into a macro. Modifiable by Hits, by Live VJ touch, and as a FIELD in the universal AUTO/OVERRIDE model. See FEATURE_CONNECTIONS Scenario 8. |
| 3 | Hit Group nesting | Flat only for V1. Hit Groups contain Hits, not other groups. (Reconsider for V2 if user demand emerges.) |
| 4 | Data recording storage format | JSON for V1. Frame-precise event log at 60fps internal resolution, decoupled from BPM. Each event timestamped + typed (signal activation, clip trigger, macro toggle, parameter change, Hit firing). |
| 5 | Live Hit triggering during playback | Both fire — and Hits + Live VJ are both state mutators. The Universal AUTO/OVERRIDE rule resolves conflicts per field. See FEATURE_CONNECTIONS Scenario 1 and 6. |
| 6 | Hit lane minimum spacing | NO conflict possible — Hits live on a quantize grid (max 4 per bar = 1 per beat in 4/4). Two Hits cannot share a position by design. See FEATURE_CONNECTIONS Scenario 3. |
| 7 | Empty composition state at start | Empty dark stage. BPM "---" if no audio (else detected BPM). Signal Bar shows zeros (or live values if audio). All panels visible. Clip grid visible with cells dim. Default mode: LIVE. Default layout: H1. Signal + Live VJ layers are always-on; Hits are optional. Timeline always advancing when transport plays. See FEATURE_CONNECTIONS Scenario 5. |
| 8 | Pulse AI scope of action | Configuration assistant for V1 (browse / suggest / configure, no live triggering). Detailed implementation deferred to Phase 13. See Part 3 of this directive. |
| 9 | Hit preview without firing | Ghost preview on hover. Semi-transparent overlay shows what the Hit would do without actually firing. No state change to the running composition. |
| 10 | Clip cell flexibility — multiple stacked effects | Already supported — clips have effect stacks. Plus, in the cell-scoped clip model (locked 2026-05-22): same source media can live in multiple cells, each cell maintaining its own in-point, out-point, cuepoints, effects, and transforms. See FEATURE_CONNECTIONS Scenario 9. |

### Additional Decisions Locked 2026-05-22

These were NOT in the original Open Questions list but were resolved in the same session:

| # | Topic | Decision |
|---|-------|----------|
| A | Three intent layers naming | Signal / Hits / Live VJ |
| B | Universal field-level state machine | AUTO ↔ OVERRIDE per field; orange `#ff4500` indicator; click to release per field |
| C | No global "release all overrides" button | Per-field release only. Top chrome carries a passive orange STATUS indicator showing overrides exist; it is NOT a clickable mass-release. |
| D | Scope precedence | Global > Layer > Clip (outermost wins, exclusive — mixing-desk model, not CSS specificity) |
| E | Same-scope multi-route combination | Sum, clipped to [0, 1] |
| F | P. button | REMOVED entirely. Triangle-only routing. Section headers keep only the 2px blue left-edge indicator. |
| G | Hit creation workflows | Three: (1) Scrub-and-save offline, (2) Two-Hit automation, (3) Live capture — all using the same unified Save/Capture button |
| H | Save button behavior | Slot must be selected first; button fires on selected slot. For live capture, quantize setting (Exact beat / Next bar / Closest bar) determines position. |
| I | Occupied Hit slot click | Confirm dialog before replace. (NOT merge.) No drag-and-drop. |
| J | Default Hit-to-Hit envelope | LINEAR (Ableton-style). User changes via Hit Manager. |
| K | Two-lane timeline | HITS lane (beat-quantized, blue pills) + REC lane (time-tick, continuous density curve). Same horizontal positions, different axis labels. Click handle to expand vertically. Click-and-drag to zoom horizontally. After-Effects-style per-parameter sub-tracks when expanded. |
| L | Recording lane | Scrubbable (continuous frames). HITS lane NOT scrubbable (discrete keyframes only); click to jump. |
| M | Active driver | One driver at a time (HITS or RECORDING). Switch via ● / ○ at lane handle. Editorially independent — Hit lane editable regardless of driver. |
| N | Composition portability | Self-contained data + binary media referenced by path. "Gather Content" action bundles into a portable folder. Missing media → broken cells with re-link offer. |
| O | Macro scope transfer | Copy as default (Cmd-drag for move). Filter invalid routings + warn on scope-narrowing. Library export as separate action. |

For full worked examples and implications, see `FEATURE_CONNECTIONS.md`.
For conceptual framings, see `MENTAL_MODELS.md`.
