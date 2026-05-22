# Boris's Decisions — Audio-DNA

Tight reference of decisions made during the design session. For full context, see `AUDIO_DNA_DIRECTIVE_FULL.md`.

> **Last updated 2026-05-22.** Canonical behavioral reference is now
> `FEATURE_CONNECTIONS.md`. Conceptual spine is `MENTAL_MODELS.md`.
> This doc is the SHORT-FORM SUMMARY of decisions in Boris's voice;
> the two canonical docs above hold the worked examples and detailed
> rules.

---

## Colors

- Accent color: `#00d9ff` (cyan/blue)
- Killed: orange `#ff4500` (retired), mint `#5fdba7` (replaced)
- One accent only — no rainbow UI
- Background `#1a1a1a`, panels `#2a2a2a`, borders `#3a3a3a`, labels `#888`, values `#e0e0e0`
- REC/danger only: `#ff4040`
- **Orange `#ff4500`** — RE-INTRODUCED 2026-05-22 EXCLUSIVELY as the `--override` color (OVERRIDE state indicator). Used at three zoom levels: top-chrome STATUS indicator, group-level indicators (layer strips, section headers), per-field orange dot. Click to release that level back to AUTO. Deliberate exception to the "one accent" rule.

---

## Naming

- Timed cues = **Hit** (not "click" / not "Q" / not "cue")
- AI assistant = **Pulse**
- Cue points = stay as "cue points" only for clip-internal markers (distinct from Hits)
- Signal groups = **Macros** (Ableton-rack model)

---

## The Three Intent Layers

Three independent decision-makers can control visuals at any moment:

- **Signal** — audio drives parameters automatically (set-once, runs-forever)
- **Hits** — programmed events fire at scheduled timestamps
- **Live VJ** — real-time user actions during performance

See `MENTAL_MODELS.md` for the framing. See `FEATURE_CONNECTIONS.md` Scenario 1 for precedence rules.

---

## AUTO / OVERRIDE

Every individual UI field has its own state:

- **AUTO** (default) — Hits + Signal drive the field
- **OVERRIDE** — VJ touched the field; Hits are blocked; Signal still modulates

Touch any field → OVERRIDE. Click its orange `#ff4500` dot → back to AUTO. Per-field granularity throughout. No global "release all" — per-field release only. The top chrome carries a passive STATUS indicator (orange when any field is overridden); it is not a clickable mass-release.

See `FEATURE_CONNECTIONS.md` Scenario 1 + 6 for the full state-mutation model.

---

## Hard Visual Rules

- No border-radius anywhere
- No box-shadows
- No gradients on chrome
- No circular knobs — all knobs are vertical signal columns
- 1px hairline borders only
- Buttons touch (no gaps)
- 3% SVG grain overlay (only decorative element allowed)
- No emoji / decorative icons
- ALL-CAPS for headers and mode labels
- Monospace only for numbers

---

## Signal System

- Three tiers: Sources → Signals → Macros (Signal Groups)
- Three scopes: Clip / Layer / Global — at every scope
- Triangle next to every parameter (universal signal gateway)
  - Unrouted: light outline
  - Selected: solid fill
  - Active: blue fill, brightness = signal value 0→1 in real time (direct 1:1, no easing)
- Vertical signal columns replace ALL knobs (including dashboard)
  - Background = live signal fill (blue, bottom→top)
  - Foreground = draggable tick for user value (0-100)
  - MIDI/OSC indicator always visible on column
- Per-layer signal columns auto-populate on left of layer controls
- Scroll horizontally, 4-6 visible per layer
- Signals can modulate macros as automation (one-way or looped)

---

## Hit System

- Hits are keyframes (not step-functions)
- Hit payload = 3 categories: **Signals + Clips + Macros**
- Pill visual: small vertical capsule with 3 dots inside (S / C / M), blue when present
- Pill uniform size for all Hits — hover for detail
- Chains shown as connecting lines between pills (not inside pill)
- Creation: ONE method = "Capture Hit" button records active composition state at playhead
  - Same in LIVE and PROGRAM modes
  - Filter step after capture: user unchecks unwanted items
- Hits can only trigger what already exists in the composition
- Diff-based override: per-parameter, later wins, untouched persists
- Clip-scope macros discard with clip change; Layer/Global persist
- Nothing plays before first Hit (no default state)
- Envelope per Hit: onset (instant/fade), curve (linear/ease/expo/step), release
- Per-parameter envelope override allowed
- Pattern presets: 1, 2, 4, 8, 16, 32, 64 bar lengths + irregular (2-2-3-3)
- Data recording = parallel continuous capture system; Hits = editorial layer on top
- Both data recording and Hits can render to video
- Hits live on a quantize grid: max 1 per beat (4 per bar in 4/4). User-adjustable quantize (1/2/4 per bar). No two Hits share a position by design.
- Default envelope between consecutive Hits: LINEAR (Ableton-style). User changes in the Hit Manager (bottom focus bar) — onset (instant / fade-in), curve (linear / ease-in / ease-out / S-curve / exponential / step / instant-cut), release (instant / fade-out).
- Three Hit creation workflows, all using the SAME unified Save/Capture button:
  1. **Scrub-and-save** — scrub recording, click target slot to select, press button → Hit created at selected slot with scrubbed state. (Confirm dialog before replacing occupied slot — NOT merge.)
  2. **Two-Hit automation** — two Hits with envelope between them defines automation
  3. **Live capture** — same button captures at current playhead. Quantize toggle adjacent to button: Exact beat / Next bar / Closest bar.
- Per-Hit envelope override: field-level inheritance (per-param can override any subset of envelope fields)
- No drag-and-drop. Click-to-select + button-to-save only.
- Envelope curves visible as thin lines between Hits on timeline.

---

## Composition Architecture

- Global / Layer / Clip = three nested scopes
- Each scope has its own effects + macros
- Clip can contain: video, effect, or video + effect stack
- Videos block what's beneath (opaque)
- Effects modify what's beneath (transparent)
- Cell = (layer × column) intersection
- Column = vertical slice across all layers (triggering = scene change)
- Global effects = only "unlayered" visual unit
- Cells are cell-scoped clip instances: same source media can live in multiple cells, each with its own in-point, out-point, cuepoints, effects, transforms. Cuepoints are user-placed positional bookmarks — NOT referenced by Hits (Hits capture exact playhead positions).
- Composition file is portable for DATA (Hits, Macros, routing, layouts), references binary media (clips, MilkDrop) by path. "Gather Content" action bundles composition + only-used media into a portable folder for touring. Missing media on load → broken cells with re-link offer.

---

## Pulse AI

- Name: Pulse
- Chat-based interface
- Top right of header bar
- Navigates Files/Effects/Sources/Compositions/Recordings/MilkDrop
- Can suggest signal routings
- Works by loading settings/show files (modifications appear in interface but are file loads)
- When inactive: same area shows 2 rows of category headings for manual browsing

---

## Layout

- Default launch layout = H1 (`v9_brut_stacked_var_b.html`)
- Mode-switching remembers panel state per mode (LIVE → PROGRAM → LIVE returns to last LIVE layout)
- Layout templates + hotkeys in top-right header
- Interface on ONE screen for performance
- GPU multi-output for video to projectors/walls

---

## Top Chrome (98px fixed)

- 44px: Logo / BPM 48px blue / Bar / Phrase / State pill / Beat wheel / Transport / Quantize / Fade / Master / REC / FPS / [LIVE][PROGRAM][SETUP] / hotkeys / Pulse button
- 32px: Resolume-style transport strip
- 22px: Signal quick-ref bar (15+ live values) — CLICKABLE → overlays full workspace when expanded
- **Unified Save/Capture button** (large, prominent, transport area) — one button serves both live capture and scrub-and-save workflows. Adjacent 3-state quantize toggle: Exact beat / Next bar / Closest bar.
- **Passive AUTO STATUS indicator** — turns orange `#ff4500` when any field anywhere is in OVERRIDE. Not a clickable mass-release (per-field release only).

---

## Bottom Focus Bar

- Thin collapsed bar at bottom
- Shows summary of selected item when collapsed
- Click to expand upward
- Default expanded height ~30% screen, user-draggable
- Detail editor uses same inspector grammar as rest of app
- Context-sensitive: click macro = shows macro params, click effect = shows effect params, click Hit = shows Hit anatomy

---

## Inspector Grammar (everywhere)

```
[▶ triangle] [Label 90px right #888] [Value 50px mono #e0e0e0] [−][+] [━━━┃━━━ 14px]
```

(P. button REMOVED entirely — 2026-05-22 decision. Triangle-only routing. See `FEATURE_CONNECTIONS.md` Scenario 3.)

Same in: inspector panels, effect controls, signal editors, Hit surfaces, bottom focus bar. One visual language.

---

## Section Headers

- 24px tall, `#252525` bg
- `▼` 7px disclosure triangle
- Sans 12px, uppercase-first
- 2px blue left-edge bar when section has any active routing

---

## Inspector Tabs

- Right side: **CLIP | LAYER | COMP | SIGNALS**
- Foldable (collapsible)
- Active tab = blue text + 2px blue bottom underline

---

## Clip Cells

- Base 100×80px (never < 80×60, performance mode up to 84×52)
- Left 6px: per-clip mini waveform (dim blue)
- Main area: appropriate thumbnail for content (video, effect, MilkDrop, etc.)
- Bottom 14px: clip name 9px sans
- States: Playing (30% blue fill + 2px solid border), Queued (2px dashed blue), Selected (1px solid blue), Loaded (1px border), Empty (1px dashed)

---

## Layer Strip

- Left side: vertical signal columns (auto-populated, horizontal scroll, 4-6 visible)
- Source columns adjacent to destination columns
- Triangles next to every param pulsing with signal
- Living patch diagram — no clicks needed to read

---

## Browser Panel (right)

- Tabs: FILES / FX / SOURCES / COMP / RECORD / MILKDROP
- Signal thumbnails = static waveform shapes (NOT animated)
- Pulse replaces headings when active

---

## Content Organization

- Grid columns = named labels (intro / verse / drop / break / etc.)
- Labels are organizational only (not tied to actual track timeline)
- Custom tags on content (e.g., "dark", "techno")
- Searchable and filterable by tag

---

## Timeline

Two thin lanes share a horizontal timeline (same physical positions, different axis labels):

- **HITS lane** — beat-quantized; blue capsule pills at beat positions; envelope curves drawn between consecutive pills.
- **REC lane** — time-tick-labeled (not BPM-tied); continuous density curve showing event activity over time. Recording at 60fps internal resolution.

Each lane:
- Thin by default; click handle → expand vertically
- Click-and-drag horizontally → zoom IN to dragged range
- Expanded view: After-Effects-style per-parameter sub-tracks (group → individual disclosure)

Interactions:
- Recording lane: SCRUBBABLE (drag = scrub playhead). Click anywhere to jump.
- Hits lane: NOT SCRUBBABLE (no continuous data between Hits). Click empty slot to SELECT (highlight as save target, does NOT move playhead). Click existing Hit pill to JUMP playhead there and select Hit.

One active driver at a time (HITS or RECORDING). Switch via lane handle dot. Recording lane has a dropdown to pick WHICH recording (multiple recordings per composition allowed).

Lanes are editorially INDEPENDENT — Hit lane is always editable regardless of active driver.

See `FEATURE_CONNECTIONS.md` Scenario 11 for full details.

---

## Per-Mockup Feedback (Reference Quality Targets)

### Layouts I picked as named modes

| Tag | Mockup | What I liked |
|-----|--------|--------------|
| **P1 (LIVE default)** | `v9_ten_03_performance_focus.html` | Central display + tabs (clip/output/layer), params on right connected to displayed item |
| **P2** | `v9_brut_vj18_04.html` | Movable/resizable output monitor. Needs more signal controls on left. |
| **P3** | `v9_brut_modular_02.html` | Preview right + controls under it. Browser right. Pulse button top right. |
| **P4** | `design_08_widescreen.html` | Widescreen performance |
| **P5** | `v3_design_01_cinema.html` | Cinema-style performance |
| **S1** | `v9_ten_02_programming_studio.html` | Signal params left, all global/layer/clip params right |
| **S2** | `v9_brut_wave_03_inverted_l.html` | "Perfect" inverted-L layout. Need preview monitors top-left under waveform. Audio signals horizontal under waveform. Click waveform → larger controls below. |
| **H1 (LAUNCH DEFAULT)** | `v9_brut_stacked_var_b.html` | **Near-perfect.** Left = running signals + controls. Clean deck tabs. Foldable clip/layer/comp/signals tabs. Bottom focus area right of preview. Replace red with blue. Signal columns replace all knobs. |
| **H2** | `v9_ten_10_wide_inspector.html` | Bottom tabs Comp/Layer/Clip + Library right. Good Q-points and beat snap. |
| **H3** | `v9_ten_08_modular_tiles.html` | Grid outline left with named structural columns (intro/verse/drop/break). Tag-based search. |
| **H4** | `v9_final_01_brutalist.html` | Clean waveform + horizontal signals under it. Inspector + browser right. Needs: bottom focus, presets bottom-center/right, center = preview, grid moved down. |
| **H5** | `v8_20_vj17.html` | Great layout. Cleanest overall look. Add output + bottom focus. Missing: layer signals left of layer controls. |
| **H6** | `v9_state_04_wf_max.html` | Output left + cue points under. Sources right with mini monitors. Great top section. |
| **H7** | `v2_design_03_audio_first.html` | Dual center screens for output/preview. Blue color. One of the best setups. Needs bottom focus + favorites. |
| **H8** | `v3_variations/touch_var_a.html` | Clean look. Good top + sizing. |

### Liked specific elements (not full layouts)

| Mockup | What was liked |
|--------|---------------|
| `v9_brut_crt_layout.html` | Clean condensed stereo waveform. Good grid thumbnails. |
| `v9_brut_vj18_01.html` | Best clip cell sizing |
| `v9_brut_vj16a.html` | Per-clip mini waveform left + timeline bottom |
| `v9_brut_vj17.html` | Best top section |
| `v9_brut_d6_modular.html` | Top favorite. Best signals bar. |
| `v9_brut_resolume_b_01.html` | Top favorite (needs waveforms added top) |
| `v9_brut_sidebar_focus.html` | Best condensed left sidebar |
| `v9_brut_audio_first.html` | Best expanded left sidebar |
| `v9_brut_three_column.html` | Best cue/autopilot placement (right) |
| `v9_brutalist_01_console.html` | Good compact waveform |
| `v9_brutalist_v2c_dual.html` | Good grid thumbnails |
| `v9_brutalist_v2b_program.html` | Good grid thumbnails + setup linking |
| `v9_brutalist_v2a_performance.html` | Good setup linking workflow |
| `v9_alt_resolume_b_01.html` | Good thumbnail style for premade signals |
| `v9_01_sidebar_focus.html` | Good signal display in right menu |
| `v9_state_02_both_max.html` | General look good, waveform needs multiple modes |
| `v9_state_03_sig_max.html` | Good programming/setup mode |
| `v9_wave_02_focus.html` | Waveform stereo pair condensed + Hit lane below |
| `v4_16_inverted_l.html` | Blue color reference |
| `v9_brut_wave_05_dense.html` | Good dense grid |
| `v9_brut_modular_02.html` | Good modular grid |
| `v2_design_07_preview_centered.html` | Idea: preview right, menus right of left grid for more control room |

---

## Rejected (Do Not Implement)

- Ribbon/strip layouts
- Command palette
- Timeline view (wrong mental model)
- Touchscreen-first
- Quad-split
- Cinema/preview-dominant
- Symmetrical layouts
- Rounded corners (anywhere, ever)
- Box shadows
- Gradient chrome
- Multiple accent colors
- Decorative icons / emoji
- Orange `#ff4500` as a GENERAL-PURPOSE accent. Exception: re-introduced 2026-05-22 EXCLUSIVELY as the `--override` color (OVERRIDE state indicator only). Never use orange for any other purpose. See `FEATURE_CONNECTIONS.md` Scenario 1.
- Mint (`#5fdba7`) accent
- Italic / underline (except active mode tab)
- Circular knobs (any kind)
- Label-above-value layouts
- Per-row knobs in inspectors
- Color-coded parameter backgrounds
- Dashed unselected patch routes
- Patch-bay anchors < 12px
- Hidden signal state
- Manual-only controls (no signal visualization)

---

## Future / Considered

- DJ software/hardware integration (real-time track data when available)
