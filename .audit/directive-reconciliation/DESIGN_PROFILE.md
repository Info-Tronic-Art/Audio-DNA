# Boris's Design Profile — Audio-DNA

> Extracted from AUDIO_DNA_DIRECTIVE_FULL.md via 3-pass deep analysis.
> Every builder MUST read this before implementing any visual change.
> Written 2026-05-21.

> **Note 2026-05-22:** This document was written 2026-05-21. Some specifics
> have been superseded by later session decisions (notably: P. button
> REMOVED entirely; orange `#ff4500` added as the OVERRIDE state indicator;
> AUTO/OVERRIDE field-level state machine). The canonical current behavioral
> reference is `../../FEATURE_CONNECTIONS.md`, and the canonical visual/
> design system reference for mockups is `../../MOCKUP_BRIEF.md`. The
> design PHILOSOPHY captured in this doc (the 7 principles, anti-patterns,
> "Perfect" Checklist intent) remains canonical.

---

## Core Design Tension

"Density without clutter, power without chaos."

Boris resolves this by showing EVERYTHING but enforcing rigorous visual
hierarchy. The solution is COMPRESSION (everything visible, packed tight)
not MINIMALISM (hiding things).

"Clean and easy to use like a high-performance car. All controls present,
nothing overwhelming." AND "Resolume-style industrial aesthetic — dense,
minimal, practical like a machine." These are not contradictions. They
define the tension every design decision resolves.

**In one sentence:** A VJ mixing desk, not a creative app — dense,
functional, machine-like, with Resolume Arena's exact visual grammar
but Audio-DNA's audio-analysis soul.

---

## 7 Design Principles

### 1. Dense Machine — Functional Density Without Visual Noise

Show everything. Never hide controls. Use consistent visual grammar so
density reads as order, not chaos. Boris's ideal: scan the interface and
know the full system state without clicking anything.

"Reading a layer at a glance: You see BASS is driving opacity, scale,
and effect wet." Layer strips as "living patch diagrams." Rejected:
command palette ("too hidden"), cinema mode ("sacrifices control surface").

### 2. Industrial Brutalism — Not Minimal, Not Decorative

Minimalism removes elements. Brutalism strips all decoration but keeps
all function. The UI should look like it was machined, not designed.
If it could be engraved on an aluminum panel, it belongs. If it needs
a GPU to render (shadows, gradients, glow), it does not.

Square corners everywhere. No shadows. No gradients on chrome. Hairline
borders only. 3% grain (the only decorative element — prevents sterile
flatness). IBM Plex Mono for "instrument panel feel." Buttons touch
with no gaps.

### 3. One Color, Used Precisely — Color as Signal, Not Decoration

Blue #00d9ff means exactly one thing: "active / alive / routing."
Everything else is grayscale. One accent color. No exceptions. No
rainbow UI. Boris tried orange, tried mint, returned to blue.

When you see blue, something is alive. When blue is absent, nothing
is active. Color tells you truth.

### 4. Audio is the Soul — Signal Visibility at Every Zoom Level

Everything in the UI exists to show what audio is doing to visuals.
Three zoom levels visible simultaneously:
- Global: 22px signal quick-ref bar
- Macro: Vertical signal columns on each layer
- Parameter: Triangle next to every control, pulsing with signal

"The interface breathes with the music." "Hidden signal state" is an
explicitly rejected pattern.

### 5. Resolume Grammar, Audio-DNA Soul

Adopt Resolume's STRUCTURAL patterns (inspector rows, section headers,
clip grid, layer strips). Replace Resolume's CONTROL elements with
Audio-DNA's signal-first paradigm.

Inspector rows: Resolume-exact grammar. Clip cells: Resolume sizing.
But: all knobs → signal columns. Triangles on every parameter. Signal
bar always visible. BPM at 48px (Resolume doesn't do this).

### 6. See Everything, Click Nothing — At-a-Glance Readability

Clicking is for acting. Looking is for knowing. The performer should
never have to click to understand what is happening.

Boris's test: in a dark club, under stage lights, at 128BPM, a
performer should know the full system state within 2 seconds.

### 7. Progressive Disclosure via Expand/Collapse — One Pattern

When deeper access IS needed, exactly one interaction pattern:
collapsed summary that expands in place on click. Not modals, not
new windows. Signal drawer: 22px → overlay. Bottom focus bar: thin
→ expands upward. Waveform: condensed → larger controls. Section
headers with disclosure triangles.

---

## Decision Framework

### The Element Classification Test

| Question | If Yes |
|----------|--------|
| Showing signal state? | Must pulse/animate with audio. Use blue. |
| A parameter control? | Vertical signal column + triangle + signal routing support |
| Structural chrome? | Grayscale #1a1a1a-#2a2a2a, 1px hairline borders, zero decoration |
| Information display? | Plex Mono for numbers, Plex Sans for labels |
| Decorative? | Remove it. Only exception: 3% grain |

### The Color Hierarchy (strict order)

1. Blue `#00d9ff` — active, alive, signal-driven, routing-present
2. `#e0e0e0` — values, active text, user-set content
3. `#888888` — labels, dim text, inactive controls
4. `#3a3a3a` — borders, hairlines, structural division
5. `#2a2a2a` — panel backgrounds
6. `#1a1a1a` — app background
7. `#ff4040` — ONLY for REC indicator and error states
8. Nothing else. No orange. No mint. No purple. No green. No yellow.

### Monospace Scope (confirmed)

Plex Mono for: BPM display, inspector values, signal bar values, timestamps,
any numeric readout.

Plex Sans for: clip names, button labels, section headers, menu items,
any text label.

Rule: if it's a NUMBER or MEASUREMENT, it's mono. If it's a NAME or ACTION, it's sans.

### The Tiebreaker Rule

When two valid approaches exist, Boris picks the one that:
1. Shows more information without clicking (over cleaner-but-hides-info)
2. Uses existing grammar (over new interaction pattern)
3. Is denser (over more whitespace)
4. Is more machine-like (over more "designed")
5. Keeps signal visible (over hides-signal-for-cleanliness)
6. Uses single accent color (over introduces second color)

---

## What Boris Finds Beautiful

1. **Compression with clarity** — many things visible simultaneously,
   all readable. Deck tabs, signal columns, layer strips, inspector,
   preview, bottom focus — all coexisting at 1920x1080.

2. **Live animation as information** — triangles pulsing with signal,
   columns filling with audio energy, beat wheel turning. "The interface
   breathes with the music" is what he finds beautiful.

3. **Monochrome discipline with a single accent** — grayscale palette
   with blue as sole punctuation. Blue on dark gray looks like an
   instrument panel in the dark.

4. **Typographic precision** — IBM Plex Mono for numbers = instrument
   readouts. Values at 11px, BPM at 48px, labels uppercase with
   letter-spacing. Aviation/control-room typography.

5. **Hairline structure** — 1px borders at #3a3a3a. Not thick enough
   to be borders, just enough to be structure. The whole UI is held
   together by barely-visible hairlines, like a technical drawing.

6. **Texture without decoration** — 3% SVG grain overlay. Prevents
   flat grayscale from feeling dead. The visual equivalent of the
   slight hiss on vinyl — warmth without content.

---

## What "Clean" Means to Boris

When Boris says "very clean":
- Consistent visual rhythm (equal spacing, uniform row heights)
- Clear visual hierarchy (BPM > transport > labels)
- No orphaned elements (everything docked and aligned to edges)
- Minimal visual weight variation (no element dramatically heavier
  than others, except BPM which is deliberately dominant)

---

## Anti-Patterns (What Boris Hates)

### Visual
- Rounded corners (anywhere, anything, ever)
- Box shadows (any element)
- Gradients on UI chrome
- Multiple accent colors ("rainbow UI")
- Decorative icons or emoji
- Circular knobs
- Thick borders (>1px)
- Color-coded per-parameter backgrounds ("too noisy")
- Labels above values (must be horizontal pairs)

### Interaction
- Hidden controls (command palettes, right-click-only menus)
- Forced symmetry ("fights VJ workflow asymmetry")
- Cinema/preview-dominant layouts (controls > preview)
- Timeline mental model (grid > timeline)
- Touchscreen optimization (desktop mouse precision)

### Systemic
- Static controls without signal visualization
- Hiding Audio-DNA differentiators in menus
- Smooth animation curves on signal display (raw signal, not eased)

---

## The "Perfect" Checklist

Boris says "this is perfect" when ALL of these are true:

### Structure
- Top chrome exactly 98px (44+32+22)
- BPM 48px Plex Mono blue, single largest element
- Signal bar 22px, 15+ live values, always visible
- Structural state pill visible on top bar
- Bottom focus bar present, thin collapsed, expands upward
- Layout asymmetric (left/center/right serve different functions)

### Visual Treatment
- Zero border-radius, zero box-shadows, zero chrome gradients
- All borders 1px #3a3a3a hairlines
- Buttons touch with no gaps
- 3% SVG grain overlay
- Only blue #00d9ff for UI state
- IBM Plex Mono numbers, IBM Plex Sans labels
- ALL-CAPS headers with letter-spacing

### Controls
- Zero circular knobs
- Triangle on every parameter
- Active triangles pulse RAW (no easing, no smoothing — instant brightness)
- Signal column FILLS are SMOOTHED (0.3 alpha EMA — readable, professional)
- This is a SPLIT: triangles = raw truth, columns = smoothed readability
- Vertical signal columns show signal + value simultaneously
- Signal columns on layer strips: 60px wide fixed, 5 visible at typical width
- MIDI/OSC indicator on every column
- Inspector rows follow Resolume-exact grammar
- Beat wheel: 4 SQUARES in a row (14x14px), not circular. Active=kAccent fill.
- Scrollbar thumbs: SQUARE (zero border-radius, no exceptions)
- P. button REMOVED entirely (2026-05-22 update — previously planned for section headers only, now eliminated. Routing happens via the Triangle on each row; section headers keep only the passive 2px blue left-edge bar indicator when routing is present.)

### The 2-Second Test
Within 2 seconds, a performer in a dark club knows:
1. BPM (48px blue number)
2. Structural state (blue pill)
3. Which parameters are signal-driven (pulsing triangles)
4. What audio is doing (signal bar + columns)
5. Which clips are playing (blue borders)
