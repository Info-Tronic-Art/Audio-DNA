# Connection-Gap Decisions — Session 2026-05-22

Source-of-truth capture for all behavioral decisions made during the
pre-mockup connection-scenarios discussion. To be merged into
FEATURE_CONNECTIONS.md (canonical) and AUDIO_DNA_DIRECTIVE_FULL.md Part 10
during the doc reconciliation phase.

This document captures ~35 decisions across 12 connection scenarios that
were unresolved or under-specified in the prior docs. Companion to
DESIGN_PROFILE.md and HANDOFF.md.

---

## The Three Intent Layers (Naming)

Audio-DNA composes visual output from three independent layers of intent:

1. **Signal** — audio layer (automatic, always-on, Signals + Macros)
2. **Hits** — schedule layer (programmed events firing at timestamps)
3. **Live VJ** — hands-on layer (real-time user actions)

Every UI element either operates ON a layer, visualizes a layer's state,
or switches emphasis between layers (LIVE / PROGRAM / SETUP modes).

---

## Scenario 1 — Layer Precedence

**Resolution:** Hits and Live VJ are equal-priority STATE MUTATIONS
(last write wins). Signal layer is ALWAYS-ON and computes from current
state — it doesn't compete; it always runs based on whatever the state is.

**Per-field state machine:**
- **AUTO mode** (default) — Hits + Signal drive the field
- **OVERRIDE mode** — VJ touched the field; Hits are now BLOCKED from
  writing it; Signal layer continues modulating around the VJ-set value

**Transitions:**
- VJ touches a field → that field enters OVERRIDE
- VJ clicks the field's orange indicator → returns to AUTO

**Universal field-level rule:** Every individually-clickable/draggable
UI field has its own independent AUTO/OVERRIDE state. Applies to:
parameter values, envelope fields, depths, route on/off, etc.

**No global release button.** Per-field release only. The top chrome
may carry a passive orange STATUS indicator showing overrides exist
somewhere, but it is NOT a clickable mass-release button.

**Reclaim button name:** rejected. The button label is **AUTO**.
The mode label pair is **AUTO ↔ OVERRIDE**.

---

## Scenario 2 — Scope Precedence

**Resolution:** Within the Signal layer, scope precedence follows the
mixing-desk model:

**Global > Layer > Clip** (outermost wins, exclusive)

Only ONE scope is active for any param at a time. If a Global macro
routes opacity → Global runs; Layer/Clip macros are silent for that
param. Falls through cleanly if outer scopes don't route the param.

**Same-scope multi-route:** when multiple macros at the SAME scope
route to the same parameter, contributions SUM, clipped to [0, 1].

**Multi-route UI:** standard triangle only on the row. Details (which
specific macros contribute) accessible via Bottom Focus Bar when the
row is selected.

**Meta-rule:** the user's higher-level decision wins. Live VJ (most
recent) > Hits (scheduled) > Signal (ambient). Global scope (broader)
> Layer > Clip (most local).

---

## Scenario 3 — Hit Position Conflict

**Resolved by design — no conflict possible:**

- Hits live on a quantize grid (default 1 per beat = 4 per bar in 4/4)
- User-adjustable quantize coarser (1, 2, 4 per bar) but max 4
- No two Hits share a grid position
- Within a single Hit, filter UI prevents internal contradictions
  (one Hit = one snapshot, one value per touched parameter)

**P. button removed.** Section headers retain the 2px blue left-edge
indicator (passive routing presence). Per-parameter routing happens
via the Triangle on each row. No "route entire section" affordance.

---

## Scenario 4 — Mode-Switch During Playback

**Resolution:** Mode is WORKSPACE only — never affects runtime.

**Stays running across mode switches:**
- Playback position + transport state
- Audio analysis + Signal computation
- Hit firing & envelope interpolation
- Clip transports
- Live VJ parameter ownership (OVERRIDE state preserved)
- MIDI/OSC bindings

**Changes on mode switch:**
- Layout (panels rearrange to mode's last-used template)
- Triangle click behavior (LIVE = quick menu; PROGRAM = full routing
  workspace; SETUP = signal definition config)
- Inspector exposure (advanced controls hidden in LIVE)
- Bottom Focus Bar context

**SETUP mode apply:** changes apply IMMEDIATELY, live. No commit step.
User accepts that mid-show config changes are their responsibility.

---

## Scenario 5 — Empty Composition / Pre-First-Hit Behavior

**Resolution:**

- **Always-on layers:** Signal + Live VJ are ALWAYS active, regardless
  of Hit timeline state. Audio analysis runs the moment audio input
  exists. Signal Bar shows live values. Triangles pulse on any routed
  param. VJ can manually trigger clips at any time.
- **Hits are optional:** A composition can be used with zero Hits
  placed (pure improvisation mode). Hits are an additional editorial
  layer for programmed shows.
- **Timeline always advancing:** BPM clock and bar counter always run
  when transport is playing. No Hits = nothing auto-fires, but time
  advances. Useful for improvisation alignment to track structure.

**Empty composition state (per HANDOFF Q21):**
- Output canvas: empty / dark
- BPM display: "---" if no audio in, else detected BPM
- Bar counter: "1.1.1"
- Signal Bar: zeros if no audio; live values if audio
- All panels visible (not collapsed)
- Clip grid visible, all cells dim
- Mode: LIVE (default)
- Active layout: H1

---

## Scenario 6 — Hit→Signal Handoff (the state model)

**The corrected model (Boris's clarification):**

> "Hit is very similar to the user adjusting those parameters at that
> exact time. The effects are no different. When there is a signal
> going on a parameter, and the user changes it in some way, the
> signal is still going unless the signal is canceled manually or
> with a hit."

**Implication:** Hits and Live VJ are equivalent state mutations.
Both write to per-parameter state. Signal computes based on current
state every frame.

**Per-parameter state contents:**
- `baseline` — current set value (modified by Hits via "set value" or
  by VJ via direct touch)
- `active_routes` — set of {signal, depth} currently routing
- `mode` — AUTO or OVERRIDE per the per-field rule

**Runtime computation per frame:**
```
output = clamp(baseline + Σ(active_routes × depth), 0, 1)
```

**Effect of an OVERRIDE field:** Hits are blocked from writing that
field. Signal layer continues to read state and modulate. VJ touches
continue writing. Released when VJ clicks the field's orange dot.

**Override color signature:** orange `#ff4500` — added as a 2nd
meaningful color in the design system (deliberate exception to the
"one accent" rule). Used at three zoom levels:
- App-wide: top-chrome AUTO STATUS indicator
- Group-wide: layer strip header, section header
- Per-field: orange dot at row end

All same color. Same semantic ("manually held, not following the
show"). Click to release at that level.

---

## Scenario 7 — Per-Parameter Envelope Override

**Resolution:** Field-level inheritance.

- Hit has a Hit-level envelope (default for all touched params):
  onset (instant / duration), curve (linear / ease-in / ease-out /
  S-curve / exp / step), release (instant-cut / duration)
- Each per-payload-item can override ANY SUBSET of envelope fields
- Unspecified fields inherit from the Hit-level envelope
- Per the universal field-level rule: each envelope field is
  independently AUTO/OVERRIDE-able

---

## Scenario 8 — Signal-to-Macro Modulation Depth

**Resolution:** Per-route depth slider (already resolved HANDOFF Q16).

- Each route (e.g., BASS→opacity) has its own depth control 0–100%
- Depth set when routing; modifiable by Hits and by VJ
- Per the universal field-level rule, depth is a FIELD —
  independently AUTO/OVERRIDE-able

---

## Scenario 9 — Cuepoint vs Hit Interaction

**Clip-cell model (Boris's clarification):**

- Each clip cell is an INDEPENDENT instance of source media
- Same source media can live in multiple cells
- Each cell has its own: in point, out point, cuepoints, effects,
  transforms (per-cell settings)
- Editing in/out/cuepoints in one cell does not affect other cells

**Cuepoints:**
- User-placed positional bookmarks within the clip
- Used for: (1) quick-scrub in the clip settings UI when arranging
  state to capture, (2) manual mid-playback jumps, (3) visual ref
- **NOT referenced by Hits** — Hits capture exact playhead positions

**Clip-start behavior:**
- **Manual trigger** (Live VJ layer, not via Hit): starts at cell's
  current in-point
- **Hit-fired:** Hit captured the playhead position + in/out at
  moment of capture; on fire, applies those captured values to the
  cell and plays from the captured playhead

---

## Scenario 10 — Composition File Portability + Gather Content

**Default composition file:**

- Self-contained for DATA: Hit Groups, Macros, routing, layouts used,
  signal definitions, MIDI overrides, clip metadata (in/out/cuepoints
  /effects)
- References LARGE BINARIES by file path: video clips, MilkDrop
  presets, source media
- Small file, portable as long as referenced media exists

**"Gather Content" action:**

- Separate explicit command (File menu, or COMP browser tab action)
- Scans composition for all referenced media
- Copies them into a "show bundle" — composition file + media/
  subfolder
- Inside the bundle, composition paths are rewritten to local refs
- Result: a portable folder for touring

**Bundle format: folder.** `My Tour Show 2026/` contains
`show.composition` + `media/` subfolder. User can inspect, swap files
manually. Matches Ableton Live Sets.

**Missing media on load: show broken cells + offer to re-link.**
Composition loads. Affected cells display a "media missing" state
(orange-tinted, path shown). UI offers "Re-link missing media" to
locate the file. User can play unaffected parts while fixing.

**Library export (HANDOFF Q14 confirmed):**
- Macros are BOTH composition-owned AND library-shareable
- Hit Groups: both
- Layout templates: library-only (app-wide)
- Signal definitions: library-only
- MIDI bindings: library-only (with per-comp override possible)
- Routing assignments: composition-only
- Clips & cells & decks: composition-only

Library export is a SEPARATE action ("Save to Library") from
in-composition scope changes.

---

## Scenario 11 — Data Recording vs Hits (TIMELINE MODEL)

### The two grids

**Hit grid (beat-quantized):**
- Maximum 4 Hits per bar = 1 per beat (4/4 time)
- At 120 BPM: 1 beat = 500ms, 1 bar = 2000ms
- Quantize setting: 1 per bar / 2 per bar / 4 per bar (max)

**Recording grid (FPS-quantized):**
- 60fps internal timestamp resolution (~16.67ms per frame)
- NOT tied to BPM
- 30 frames per beat at 120 BPM
- Captures everything continuously: Hit firings + Live VJ touches +
  Signal activations + clip triggers + parameter changes

### Two-lane shared timeline

```
DEFAULT (both lanes thin):
┌──────────────────────────────────────────────────────────┐
│ TIMELINE RULER (beats / time ticks, drag here = zoom)    │ 12px
├──────────────────────────────────────────────────────────┤
│ WAVEFORM (stereo pair, beat grid overlaid)               │ 50px
├──────────────────────────────────────────────────────────┤
│ HITS  ●│  ▌▌    ▌▌    ▌      ▌▌                          │ 20px (thin)
├──────────────────────────────────────────────────────────┤
│ REC   ○│  ∼∼∼/¯\∼∼∼/¯¯\∼∼/¯\∼                            │ 16px (thin)
└──────────────────────────────────────────────────────────┘
```

**Recording lane visualization (default, collapsed):**
- Continuous density curve (smooth gray "waveform" of activity)
- Time-based axis (ticks in seconds)
- Decoupled from BPM

**Hits lane visualization (default, collapsed):**
- Blue capsule pills at beat positions
- Beat-based axis (bar.beat labels)

Both lanes share the same horizontal position scale (so visually
aligned), but each labels its axis in its native units (beats vs
time ticks).

### Lane expansion (After Effects model)

**Click lane handle → expand vertically.**

When expanded, the lane drills into PER-PARAMETER SUB-TIMELINES.
Hierarchical disclosure like After Effects:

```
REC ▼ (expanded)
  ▼ Layer 1 (group disclosure)
      opacity     ∼∼∼∼/¯¯¯\∼∼∼∼∼∼∼∼∼/¯\∼∼   (continuous curve)
      scale       ∼∼∼∼∼∼∼∼/¯\∼∼∼∼∼∼∼∼∼∼∼∼∼∼
      effect.wet  ∼∼∼∼∼∼∼∼∼∼∼∼∼/¯\∼∼∼∼∼∼∼∼
  ▶ Layer 2 (collapsed group)
  ▶ Global (collapsed group)

HITS ▼ (expanded)
  ▼ Layer 1
      opacity     ●━━━━━●╱╱╱╱╱●  (keyframes + envelope curves)
      scale       ●─────────────●  (linear ramp)
  ▶ Layer 2
  ▶ Global
```

Click any group disclosure (▶ / ▼) to expand/collapse.
Expansion at multiple levels: lane → group → individual parameter.

**Sub-track visual types:**
- Recording sub-track: continuous captured-value curve over time
- Hits sub-track: discrete keyframe points + envelope curve lines
  between consecutive Hits on that param

### Zoom: click and drag

**Both timelines support click+drag horizontally to zoom IN to the
dragged range.** Standard DAW rubber-band-zoom behavior.

Convention: drag on the TIMELINE RULER (the beat/time scale above
the lanes) zooms; drag on the LANE ITSELF scrubs (recording) or
selects (Hits).

### Scrubbing & clicking rules

| Lane | Drag | Click empty | Click existing |
|------|------|------------|----------------|
| Recording | Scrubs playhead continuously | Jumps playhead to position | n/a |
| Hits | NOT scrubbable (no continuous data between Hits) | Selects beat slot as save target (HIGHLIGHT only; does NOT move playhead) | Jumps playhead to Hit's position + selects Hit + opens Hit Inspector in bottom focus bar |

**Why Hits lane doesn't scrub:** Hits are discrete keyframes at
quantized positions. There is no continuous state between them. The
interpolation happens during PLAYBACK, not during scrubbing.

### Active driver

Composition has ONE active driver at a time:
- **Driver = HITS:** Hit timeline fires events; Recording is visible
  but doesn't fire
- **Driver = RECORDING:** Recording fires its captured events; Hits
  are visible but don't fire

Switch via the ● / ○ indicator at each lane's left handle.

REC dropdown next to label: picks WHICH recording is displayed
(composition supports multiple recordings).

### Lanes are editorially independent

Hit lane is ALWAYS EDITABLE regardless of which is the active driver.
VJ can place/edit Hits while watching a recording play. The "driver"
just determines what fires events at playback time; editing is
decoupled.

### Hit creation workflows

**Three ways to create a Hit:**

#### Workflow 1 — Scrub-and-save (offline programming)

1. User scrubs master playhead via recording lane to desired state
   position
2. Output window shows the state at that scrubbed moment
3. User clicks target Hit beat slot → slot is HIGHLIGHTED as save
   target (playhead STAYS at scrub position)
4. User presses the **Save to Hit** button
5. Hit is created at the highlighted slot with state captured from
   the scrub position
6. If slot was occupied → **confirm dialog:** "Replace this Hit?"
   Yes/No (NOT merge — Boris explicitly chose confirm-replace)

#### Workflow 2 — Two-Hit automation (Ableton-style)

1. Create Hit 1 at bar 16 with starting state (via any workflow)
2. Create Hit 2 at bar 32 with ending state
3. **Default envelope between them: LINEAR**
4. User can change envelope easily in the Hit Manager (Bottom Focus
   Bar when Hit selected):
   - **Instant cut** (step, no fade)
   - Linear (default)
   - Ease-in / Ease-out / Ease-in-out
   - S-curve / Exponential
5. Per-parameter envelope override available (field-level
   inheritance per Scenario 7)

#### Workflow 3 — Live capture (during performance)

1. VJ performing, timeline running
2. Press the **Capture Hit** button (the SAME unified button as
   Workflow 1's "Save to Hit")
3. Hit placed at quantize-derived position with current LIVE state
4. **Quantize setting next to the Capture button** determines exact
   placement:
   - **Exact beat** — closest beat to button press time
   - **Next bar** — snaps to upcoming bar boundary
   - **Closest bar** — nearest bar, even if user pressed late
     (could be backwards)

### Unified Save/Capture button

ONE button — same button whether timeline is moving or paused.

- **Slot selected** (user clicked an empty or occupied beat slot) →
  button saves to that slot (Workflow 1)
- **No slot selected** (live performance, timeline running) → button
  saves at quantize-derived position (Workflow 3)

The button is HIGHLY VISIBLE in the transport area (large, can't
miss under performance pressure).

A **3-state quantize toggle** (or dropdown) lives directly adjacent
to the button.

### Envelope visualization on timeline

Per directive 2.7: between consecutive Hits, draw a thin line
showing the envelope curve.
- Straight line = linear (default)
- Curved = ease-in / ease-out / S-curve
- Vertical step = instant cut
- Hairline `#3a3a3a` default
- Becomes accent blue while currently playing through that segment

This makes the show's animation visible at a glance.

### No drag and drop

The interaction model is click-to-select + button-to-save only.
NO drag-and-drop from recording to Hit lane.

---

## Scenario 12 — Macro Transfer Between Scopes

**Default action: COPY** when relocating a macro between scopes.

- Drag (or change scope in Macro Inspector) defaults to COPY
- Original Macro stays in its original scope
- A copy is created at the target scope
- Cmd/Ctrl-drag for explicit MOVE

**Routing behavior when scope changes:**
- **Scope expansion** (Clip → Layer, Layer → Global): all routings
  persist (broader scope can still reach narrower targets)
- **Scope narrowing** (Global → Layer, Layer → Clip): routings to
  out-of-scope params become invalid. **Filter + warn:**
  - Warning dialog shows what routings will be lost
  - User confirms before action
  - Invalid routings dropped silently; valid ones survive

**Library export: separate explicit action.**
- Browser tab MACROS has a "Save to Library" button
- Saves the Macro (with its current scope binding info) to the
  app-wide library
- Available in other compositions

This matches HANDOFF Q14 ("both" — composition + library).

---

## Cross-Cutting Decisions

### Design system update

**New color added:** `#ff4500` (orange) — exclusively for AUTO-off /
OVERRIDE state.
- This is the FORMERLY RETIRED orange, intentionally reintroduced
- Goes in design tokens as `--override` or similar
- Used at three levels (app-wide AUTO status, group, per-field)
- DELIBERATE EXCEPTION to the "one accent" rule — must be documented
  prominently in design-system docs

### Iron rules confirmed

- No border-radius (square corners everywhere)
- No box-shadows
- No gradients on chrome
- No circular knobs (vertical signal columns universally)
- 1px hairline borders only
- Buttons touch (no gaps)
- 3% SVG grain (only decorative element)
- IBM Plex Mono for numbers, IBM Plex Sans for labels
- ALL-CAPS section headers and mode labels
- Single accent blue `#00d9ff` for "active / alive / routing"
- `#ff4040` danger color: REC indicator + errors ONLY
- `#ff4500` override color: OVERRIDE state ONLY (NEW)

### Universal patterns confirmed

- **Inspector grammar:** `[▶ triangle] [Label 90px right #888]
  [Value 50px mono #e0e0e0] [−][+] [━━━┃━━━ 14px]` — used EVERYWHERE
- **Triangle states:** Unrouted (light outline) / Selected (solid
  light fill) / Active (blue fill, brightness = signal value 0→1,
  RAW no smoothing)
- **Vertical signal columns:** universal knob replacement (zero
  circular knobs in the app)
- **Expand/collapse:** universal pattern for progressive disclosure
  (signal drawer, bottom focus bar, waveform, signal bar, timeline
  lanes)

---

## Next Steps (post-decision)

1. **FEATURE_CONNECTIONS.md** — canonicalize this content as the
   permanent behavioral reference (replaces this session-notes file)
2. **MENTAL_MODELS.md** — articulate the three intent layers + scope
   hierarchy + state machine framing as the conceptual spine
3. **Update AUDIO_DNA_DIRECTIVE_FULL.md Part 10** — change "Open
   Questions" to "Resolved Decisions" referencing FEATURE_CONNECTIONS
4. **Update BORIS_DECISIONS.md** — incorporate orange color, AUTO/
   OVERRIDE, scope rules; remove TBD items resolved here
5. **Update HANDOFF.md** — mark TBDs resolved; cross-reference
6. **Build MOCKUP_BRIEF.md template** — Builder reference for mockup
   tasks
7. **Mockups** — H1 first

---

*End of session notes — 2026-05-22.*
