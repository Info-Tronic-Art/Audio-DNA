# V8 Update Checklist — What Every Mockup Must Fix

## Analysis of V7 Gaps

The V7 mockups captured the visual STYLE from the reference images well (gradients, knobs, badges, scene flow, sources). But they're missing many of our ACTUAL APP FEATURES. Every V8 update must address these gaps.

## Feature Completeness Checklist

Every updated mockup MUST include or clearly indicate ALL of these:

### Top Bar (MUST have all of these)
- [ ] Audio source dropdown (Mic/File)
- [ ] Gain slider
- [ ] Play / Pause / Stop buttons
- [ ] Beat wheel (4 segments)
- [ ] Bar / Phrase display ("Bar 14, Phr 0.72")
- [ ] BPM display (large, color-coded: green=LOCKED)
- [ ] Tracker state label (LOCKED)
- [ ] Tap / Resync buttons
- [ ] Manual toggle + BPM edit field
- [ ] BPM multipliers: /4, /2, x1, x2, x4
- [ ] Quantize dropdown (Off / Next Beat / Next Downbeat)
- [ ] Fade slider (0-5s)
- [ ] Master slider (0-1)
- [ ] Output dropdown
- [ ] FPS + DSP labels

### Deck Grid (MUST have)
- [ ] Column trigger buttons (numbered, across top)
- [ ] SQUARE clip cells (same width and height)
- [ ] Engine type badges (VIDEO orange, IMAGE blue, SYNTH cyan)
- [ ] FX count badge on cells with effects
- [ ] Active cell = cyan border, Selected = white border
- [ ] Layer strips with: X/B/S buttons, transport, S/K/V/F sliders, blend dropdown, layer name, clip name + playhead
- [ ] Deck tabs at bottom

### Inspector (MUST show at least the Clip Inspector with these sections)
- [ ] Name bar with clip name
- [ ] Dashboard — 8 link knobs with scope labels
- [ ] Transport — mode (Timeline/BPM Sync), timeline bar with in/out, play/pause, loop mode, speed, duration
- [ ] Cuepoints — 8 numbered buttons
- [ ] Autopilot — action + duration + beat snap dropdowns
- [ ] Source Parameters (when source clip) — sliders with SIGNAL CONNECT TRIANGLES
- [ ] Video — opacity, width/height, blend mode, alpha type, RGBA toggles
- [ ] Transform — Position X/Y, Scale, Rotation, Anchor — all with signal triangles
- [ ] Effects Stack — bypass toggle, effect name, expand arrow, dry/wet + params when expanded
- [ ] Inspector tabs: Clip / Layer / Composition / Signal
- [ ] Pin button

### Browser (MUST show)
- [ ] Tab bar: Files / FX / Sources / Comp / Record / MilkDrop
- [ ] Search field
- [ ] FX categories with counts (Warp 28, Color 31, Glitch 15, etc.)
- [ ] Collapsible category sections

### Sources Panel (MUST show)
- [ ] Master Audio with spectrum/level visualization
- [ ] Kick Drum with envelope/level
- [ ] LFO — Sine with waveform
- [ ] LFO — Custom with editable curve
- [ ] "+ Add Audio Source" button
- [ ] "Drag a source onto a parameter" hint text
- [ ] Sources / Layer / Clip tabs

### Signal System (MUST indicate)
- [ ] Signal connect triangles (▶) on parameters — gray=manual, cyan=connected
- [ ] At least 3-4 parameters shown as "connected" to demonstrate the routing
- [ ] Signal Bar (minimized as 2-3px line or expanded) below top bar

### Footer (MUST have)
- [ ] Master Sources
- [ ] Audio Player (with play button)
- [ ] Outputs (display info)
- [ ] Save / Load / Help buttons

## UX Improvement Checklist

Beyond feature completeness, each update should improve:

1. **Signal triangles everywhere** — the #1 differentiator. Every slider in the inspector should have one
2. **Dashboard knobs labeled** — not just "1-8", show connected signal names (e.g., "Bass", "BPM", "Zoom")
3. **Effect stack shows routing** — effects with expanded params showing signal connections
4. **Layer type visible** — Opaque/Transparent/FX Only/Mask badge on each layer
5. **Cuepoints visible** — 8 numbered buttons, some "set" (lit), some empty
6. **Feedback section** — at least indicate "Feedback: Spiral" or similar for layers that have it
7. **Real clip/effect/source names** — Mandelbrot, Spectral Ring, Fire Wall, Ripple, Echo, Kaleidoscope
8. **Quantize clearly labeled** — this is a critical performance feature
9. **BPM multipliers clearly labeled** — /4, /2, x1, x2, x4 as distinct buttons

## What NOT to change
- Keep the same overall LAYOUT as the V7 version (that's the starting point)
- Keep the same COLOR SCHEME and visual style
- Keep square clip thumbnails
- Keep rotary knobs with arcs
- Keep engine type badges
- Keep Scene Flow / Sources / Layer Strips structure
