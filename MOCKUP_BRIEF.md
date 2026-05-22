# Audio-DNA Mockup Brief

> Universal design standards + per-mockup template for all v10 HTML mockups.
> Every Builder producing a mockup reads this document. Every Tester
> verifying a mockup checks against this document.
>
> **Last updated:** 2026-05-22

---

## Table of Contents

1. [Introduction](#1-introduction)
2. [Universal Design Tokens](#2-universal-design-tokens)
3. [Universal Mockup Conventions](#3-universal-mockup-conventions)
4. [Iron Rules (Anti-Patterns)](#4-iron-rules-anti-patterns)
5. [Quality Bar -- Pixel Specs from Best Elements](#5-quality-bar----pixel-specs-from-best-elements)
6. [Universal Components -- How to Build](#6-universal-components----how-to-build)
7. [Per-Mockup Brief Template](#7-per-mockup-brief-template)
8. [Workflow](#8-workflow)
9. [The 10-Mockup V1 Plan](#9-the-10-mockup-v1-plan)

### Canonical References

| Question                          | Document                                        |
|-----------------------------------|-------------------------------------------------|
| Full design vision                | `AUDIO_DNA_DIRECTIVE_FULL.md` (source of truth) |
| Behavioral spec (edge cases)      | `FEATURE_CONNECTIONS.md`                         |
| Conceptual framing                | `MENTAL_MODELS.md`                               |
| Short-form decisions              | `BORIS_DECISIONS.md`                             |
| Design philosophy + checklist     | `.audit/directive-reconciliation/DESIGN_PROFILE.md` |
| Implementation plan + pixel specs | `.audit/directive-reconciliation/HANDOFF.md`     |

---

## 1. Introduction

### What This Document Is

This is the canonical standards document for all Audio-DNA v10 HTML mockups.
It has two layers:

1. **Universal standards** (Sections 2--6) -- design tokens, file conventions,
   iron rules, quality bar, and canonical component patterns. These apply to
   EVERY mockup. A Builder should never need to look up a color value, font
   size, or component pattern outside this document.

2. **Per-mockup brief template** (Section 7) -- a fill-in structure that gets
   instantiated for each specific mockup task. Harmony writes a thin work
   packet referencing this doc; the Builder fills in the template and builds.

### Who Reads This

- **Builders** producing HTML mockups. Read Sections 2--6 before starting.
   Use Section 7 as your checklist.
- **Testers** verifying mockups. Check every iron rule (Section 4), every
   design token (Section 2), and the per-mockup verification criteria.
- **Harmony** dispatching mockup tasks. Use Section 7 template in thin
   work packets. Reference Section 9 for the V1 plan.

### Workflow Summary

```
Harmony writes thin per-mockup packet (references this doc + fills template)
  --> Builder reads this doc + packet, produces HTML mockup
    --> Tester verifies mockup against this doc's standards + packet criteria
      --> Feedback loops back to Boris for approval
```

### Design Philosophy (One Paragraph)

Audio-DNA is a VJ mixing desk, not a creative app. The aesthetic is
industrial brutalist: dense, functional, machine-like. Every pixel serves a
function. The interface breathes with the music through pulsing signal
triangles, animated vertical columns, and a single blue accent color that
means "active / alive / routing." The reference is Resolume Arena's visual
grammar with Audio-DNA's audio-analysis soul. See `MENTAL_MODELS.md` for
the full conceptual spine and `DESIGN_PROFILE.md` for Boris's aesthetic
principles.

---

## 2. Universal Design Tokens

### 2.1 Color System

Copy these CSS variables into every mockup:

```css
:root {
  /* ── Backgrounds ── */
  --bg:             #1a1a1a;  /* App background (Resolume-exact) */
  --panel:          #2a2a2a;  /* Panel fills (one step lighter) */
  --section-header: #252525;  /* Section header strips */
  --separator:      #383838;  /* Row separators (hairline context) */
  --border:         #3a3a3a;  /* All hairline borders (1px everywhere) */

  /* ── Text ── */
  --label:          #888888;  /* Dim labels */
  --value:          #e0e0e0;  /* Bright values / active text (off-white) */

  /* ── Accent ── */
  --accent:         #00d9ff;  /* Cyan/blue -- THE ONLY accent color */

  /* ── Semantic ── */
  --danger:         #ff4040;  /* REC indicator + errors ONLY */
  --override:       #ff4500;  /* OVERRIDE state indicator ONLY */
}
```

**Accent usage -- where blue `#00d9ff` appears:**

| YES (active/alive)                        | NEVER                         |
|-------------------------------------------|-------------------------------|
| Playing clip fill + 2px border            | Static labels                 |
| BPM number (48px, dominant)               | Inactive buttons              |
| Structural state pill                     | Background fills              |
| Beat wheel active segment                 | Decorative elements           |
| Transport play button (active)            | Unselected rows               |
| Selected row highlight                    |                               |
| Just-changed 200ms flash                  |                               |
| Horizontal bar slider position indicator  |                               |
| Active triangle fill (pulsing w/ signal)  |                               |
| Section header 2px left-edge bar (routed) |                               |
| Signal column fills (live visualization)  |                               |
| Active Hit markers on timeline            |                               |

**Override color `#ff4500`:** Used EXCLUSIVELY for OVERRIDE state --
per-field orange dot, group-level indicators, top-chrome STATUS indicator.
Deliberate exception to the "one accent" rule. Never use for any other
purpose.

### 2.2 Typography

**Font stack:**

```css
:root {
  --font-mono: 'IBM Plex Mono', 'SF Mono', Menlo, monospace;
  --font-sans: 'IBM Plex Sans', -apple-system, sans-serif;
}
```

**Size hierarchy:**

| Element                  | Size  | Weight | Font  | Color         |
|--------------------------|-------|--------|-------|---------------|
| BPM (hero number)        | 48px  | 500    | Mono  | `--accent`    |
| Genre / structural state | 22px  | 400  | Sans  | `--value`     |
| Section headers          | 12px  | 400    | Sans  | `--value`     |
| Row labels               | 11px  | 400    | Sans  | `--label`     |
| Row values               | 11px  | 400    | Mono  | `--value`     |
| Clip names               | 9px   | 400    | Sans  | `--label`     |
| Sparkline labels         | 10px  | 400    | Sans  | `--label`     |
| Signal quick-ref bar     | 11px  | 400    | Mono  | `--value`     |

**Typography rules:**

- ALL-CAPS for section headers and mode labels (`LIVE`, `PROGRAM`, `SETUP`,
  `DYNAMICS`, `SPECTRAL`)
- Letter-spacing on ALL-CAPS labels: `0.5px` to `1px`
- Monospace ONLY for numbers -- never for text labels
- No italic, no underline (except active mode tab blue underline)
- If it is a NUMBER or MEASUREMENT: Plex Mono. If it is a NAME or ACTION:
  Plex Sans.

### 2.3 Spacing Constants

```css
:root {
  /* ── Borders ── */
  --hairline:              1px;  /* The ONLY border width */

  /* ── Row heights ── */
  --row-height-inspector:  24px; /* Inspector rows */
  --row-height-signal:     20px; /* Signal bar rows */
  --row-height-section:    24px; /* Section header strips */

  /* ── Inspector row columns ── */
  --label-width:           90px; /* Right-aligned label column */
  --value-width:           50px; /* Mono value column */
  --button-size:           20px; /* Minus/plus buttons */
  --slider-height:         14px; /* Bar slider height */

  /* ── Clip cells ── */
  --clip-width:            100px;
  --clip-height:           80px;
  --clip-name-height:      14px; /* Bottom name bar */
  --clip-wave-width:       6px;  /* Left waveform strip */

  /* ── Top chrome ── */
  --chrome-top:            44px; /* Top bar */
  --chrome-transport:      32px; /* Transport strip */
  --chrome-signal:         22px; /* Signal quick-ref bar */
  --chrome-total:          98px; /* Sum: 44 + 32 + 22 */

  /* ── Signal columns ── */
  --signal-column-width:   60px; /* Fixed width per column */

  /* ── Hit pill ── */
  --pill-width:            8px;
  --pill-height:           24px;
}
```

---

## 3. Universal Mockup Conventions

### 3.1 File Location and Naming

- **Directory:** `design/mockups/html/v10/`
- **Naming:** `v10_<tag>_<short-description>.html`
- **Tag** matches the mockup plan ID (h1, p1, s2, top_chrome, inspector, etc.)
- Examples:
  - `v10_h1_default_layout.html`
  - `v10_top_chrome_98px.html`
  - `v10_inspector_grammar.html`
  - `v10_p1_performance.html`
  - `v10_hit_composite.html`
  - `v10_clip_cell_detail.html`

### 3.2 HTML Structure

Every mockup is a **single self-contained HTML file**:

```html
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Audio-DNA v10 — [Mockup Title]</title>
  <link href="https://fonts.googleapis.com/css2?family=IBM+Plex+Mono:wght@400;500&family=IBM+Plex+Sans:wght@400;500&display=swap" rel="stylesheet">
  <style>
    /* Design tokens (copy from Section 2) */
    :root { ... }

    /* Reset */
    *, *::before, *::after { margin: 0; padding: 0; box-sizing: border-box; }

    body { background: var(--bg); color: var(--value);
           font-family: var(--font-sans); overflow: hidden; }
    /* 3% SVG grain overlay — MANDATORY */
    body::after { content: ''; position: fixed; inset: 0;
      pointer-events: none; z-index: 9999; opacity: 0.03;
      background-image: url("data:image/svg+xml,%3Csvg viewBox='0 0 256 256' xmlns='http://www.w3.org/2000/svg'%3E%3Cfilter id='n'%3E%3CfeTurbulence type='fractalNoise' baseFrequency='0.9' numOctaves='4' stitchTiles='stitch'/%3E%3C/filter%3E%3Crect width='100%25' height='100%25' filter='url(%23n)'/%3E%3C/svg%3E"); }
    /* Component styles below... */
  </style>
</head>
<body>
  <!-- Layout structure -->
</body>
</html>
```

### 3.3 Viewport and Scaling

- **Baseline:** 1920x1080 (full HD)
- **4K-friendly:** Use CSS-native units (px at 1x, scales naturally)
- Set `overflow: hidden` on body (mockups are fixed-viewport compositions)

### 3.4 Dependencies

- **Google Fonts CDN** for IBM Plex Mono + IBM Plex Sans: acceptable
- **No other external dependencies** -- no CDN JS, no frameworks, no icon fonts
- All CSS inline in `<style>` tag
- Minimal JS only for interactive demonstrations (hover states, signal
  animation, expand/collapse). Keep JS under 200 lines.

### 3.5 Z-Index Conventions

| Layer                              | z-index |
|------------------------------------|---------|
| Base layout panels                 | 1       |
| Clip grid, inspector, layer strips | 10      |
| Bottom focus bar (expanded)        | 100     |
| Signal drawer overlay (expanded)   | 200     |
| Tooltips / hover cards             | 500     |
| SVG grain overlay                  | 9999    |

### 3.6 Animation Conventions

For mockups demonstrating live signal behavior:

```css
/* Beat flash: opacity pulse at ~128 BPM (0.468s period) */
@keyframes beat {
  0%   { opacity: 1; }
  100% { opacity: 0.55; }
}

/* Just-changed flash: accent bg fades out over 200ms */
@keyframes justchanged {
  0%   { background-color: var(--accent); }
  100% { background-color: transparent; }
}
```

- Signal triangle brightness: direct 1:1 mapping to signal value
  (above 15% opacity floor — see §6.7 for formula).
  No easing, no transition curves. RAW.
- Signal column fills: SMOOTHED (0.3 alpha EMA for readable,
  professional animation). This is a deliberate split from triangle
  behavior.

---

## 4. Iron Rules (Anti-Patterns)

Violating ANY rule below makes the mockup **non-compliant**. There are no
"soft" versions of these rules. No exceptions. No "just this once."

### 4.1 Geometry

| Rule                        | Detail                                                    |
|-----------------------------|-----------------------------------------------------------|
| No border-radius            | Zero, everywhere, every element. Square everything.       |
| No box-shadows              | Any element, any state, any context.                      |
| No gradients on UI chrome   | Gradients acceptable ONLY inside SVG thumbnails/waveforms.|
| 1px hairline borders ONLY   | Never thicker for decoration. `border: 1px solid #3a3a3a` |
| Buttons touch with no gaps  | Adjacent buttons share borders, no margin between them.   |

### 4.2 Color

| Rule                        | Detail                                                    |
|-----------------------------|-----------------------------------------------------------|
| ONE accent (active/alive)   | Blue `#00d9ff`. Used for routing, active state, signal.   |
| Orange = OVERRIDE only      | `#ff4500` reserved for OVERRIDE state. NOT counted as an accent — deliberate exception, never used for any other purpose. |
| No rainbow UI               | No green, purple, yellow, mint, teal in UI chrome.        |
| Red is REC/error only       | `#ff4040` never used for emphasis or decoration.          |

### 4.3 Typography

| Rule                        | Detail                                                    |
|-----------------------------|-----------------------------------------------------------|
| ALL-CAPS section headers    | With `letter-spacing: 0.5px` to `1px`.                   |
| ALL-CAPS mode labels        | `LIVE`, `PROGRAM`, `SETUP`.                               |
| Monospace ONLY for numbers  | Never Plex Mono for text labels.                          |
| No italic                   | Anywhere, any element.                                    |
| No underline                | Exception: active mode tab 2px blue underline.            |

### 4.4 Controls

| Rule                        | Detail                                                    |
|-----------------------------|-----------------------------------------------------------|
| No circular knobs           | All controls are vertical signal columns. No exceptions.  |
| No P. button                | Removed entirely. Triangle-only routing on every row.     |
| No section routing button   | Section headers carry only the passive 2px blue left bar. |
| No emoji / decorative icons | Functional glyphs ONLY: `▶ ‖ ■ ▼ ≡ M S V`               |

### 4.5 Layout

| Rule                        | Detail                                                    |
|-----------------------------|-----------------------------------------------------------|
| No labels above values      | Inspector rows are HORIZONTAL pairs only (label left, value right). |
| No color-coded backgrounds  | Per-parameter colored backgrounds are "too noisy."        |
| No dashed unselected routes | Patch-bay routes: solid gray 1px default, accent 2px selected.|

### 4.6 Quick Self-Check

Before submitting any mockup, scan for these common violations:

```
grep -i "border-radius"  --> must return 0 results (or 0px only)
grep -i "box-shadow"     --> must return 0 results
grep -i "gradient"       --> only inside SVG/waveform context
grep    "italic"         --> must return 0 results
grep    "#ff4500"        --> only in OVERRIDE-related styles
grep    "#ff4040"        --> only in REC/error styles
```

---

## 5. Quality Bar -- Pixel Specs from Best Elements

These are the mockup elements Boris specifically praised. They define the
quality bar for v10 mockups. Match these pixel specs. Deviate only when the
directive explicitly overrides (e.g., accent color cyan replaces orange
from older mockups).

### 5.1 BEST Signal Bar

**Source:** `v9_brut_d6_modular.html` -- "Top favorite, very clean"

| Property                | Value                                          |
|-------------------------|------------------------------------------------|
| Signal row height       | 20px exactly                                   |
| Layout                  | name 82px fixed | bar 5px height flex:1 | value 40px right-aligned |
| Active state background | `#1a3040` (dark cyan-tinted, replacing old orange-brown) |
| Active state text       | `var(--accent)` (cyan `#00d9ff`)               |
| Group headers           | 10px Mono, uppercase, 1px letter-spacing, count badge right |
| Row borders             | None between rows (density via alignment)      |

**Why "best":** Minimal whitespace, clean columnar alignment (name | bar |
value), accent only on active rows.

### 5.2 BEST Condensed Sidebar

**Source:** `v9_brut_sidebar_focus.html` -- "pushed to edge"

| Property           | Value                                              |
|--------------------|----------------------------------------------------|
| Total width        | 230px (50px signal meters + 180px quick reference) |
| Signal meters      | 50px wide, vertical bars 6px width, border-right 1px `#333` |
| Quick ref grid     | 4-column grid for FX/source tiles, 38px tile height, 4px gap |
| Padding            | 6-8px (very tight)                                 |
| Position           | Absolute left:0 (literally pushed to screen edge)  |

**Production reconciliation:** Signal meters listed at 50px reflect
the v9 reference. Production canonical signal column width = 60px
(see `--signal-column-width` token in §6.4). §6 production tokens
win on conflict.

**Why "best":** Zero wasted space, tight padding, hard borders, grid-aligned.

### 5.3 BEST Expanded Sidebar

**Source:** `v9_brut_audio_first.html` -- "deep signal analysis"

| Property            | Value                                             |
|---------------------|---------------------------------------------------|
| Width               | 220px, full height                                |
| Signal row          | 16px min-height, label | 6px bar flex:1 | value | route triangle |
| Selected row        | bg `#1a1a1a`, left border 2px `var(--accent)`     |
| Content             | 25+ signals grouped: Amplitude, Bands, Rhythm, Pitch, Structure, Genre, Advanced |
| Inline chromagram   | 12-note grid, 22px height, key highlighted in accent |
| Inline waveform SVG | 10px height, shows waveform shape                 |

**Why "best":** Every spectrum analyzed and visible, inline micro-visualizations.

### 5.4 BEST Clip Cells

**Source:** `v9_brut_vj18_01.html` -- "best sizing and dimensions"

| Property           | Value                                              |
|--------------------|----------------------------------------------------|
| Cell width         | 90-100px (base unit)                               |
| Cell height        | Flexible 60-80px typical                           |
| Name bar           | Position absolute bottom, padding 3px 5px (~14px total) |
| Clip number        | Top-left 9px                                       |
| Duration           | Top-right 9px                                      |
| Grid gap           | 1px between clips                                  |
| Default border     | 1px `#222`                                         |
| Playing border     | 2px `var(--accent)` + 30% accent fill              |

**Why "best":** Compact, 10+ clips per row at 1920px, name remains proportional.

### 5.5 Per-Clip Waveform (Boris "Loved")

**Source:** `v9_brut_vj16a.html`

| Property           | Value                                              |
|--------------------|----------------------------------------------------|
| Mini waveform SVG  | 10px height, flex:1 width                          |
| SVG viewBox        | `0 0 60 8`, polyline stroke white 1px              |
| Playhead           | 1px vertical accent line at current position       |
| Clip playhead bar  | 4px height, absolute bottom, accent color, width = progress% |
| Background         | `#141414`                                          |
| Border             | 1px `#2a2a2a`                                      |

**Why Boris "loved" it:** Micro-format, 10px is readable but minimal, playhead
visible at all times.

### 5.6 Top Chrome 98px

**Source:** `v9_ten_02_programming_studio.html` (CSS/dimensions source)

| Strip             | Height | Key elements                                       |
|-------------------|--------|----------------------------------------------------|
| Top bar           | 44px   | BPM 48px Mono 500-weight blue, bar counter, phrase counter, structural state pill, beat wheel, transport mini, mode tabs, layout hotkeys, Pulse button |
| Transport strip   | 32px   | `▶ ‖ ■` buttons, BPM `128`, `- +`, `|◂ ▸|`, `/2 x2`, TAP, RESYNC, loop. Resolume-style, separate strip. |
| Signal quick-ref  | 22px   | 15+ live audio values: RMS, PEAK, LUFS, FLUX, CENTROID, BASS, MID, HIGH... Clickable to expand as full-workspace overlay. 11px Mono. |

**Dimension details from v9_ten_02:**
- Param row label: 82px in mockup, directive rounds to 90px (use 90px)
- Section accent bar: 2px left edge (directive spec; mockup had 3px)
- Beat animation: `@keyframes beat` opacity 1 to 0.55 at 128 BPM (0.468s)

### Implementation Rule

When building any of these components, match the pixel specs above. These
are the quality bar. If the directive states a different value from the
mockup (e.g., label width 90px vs 82px), the **directive wins**. These
pixel specs show the SHAPE and FEEL Boris praised; directive dimensions
are the final measurements. **When §5 quality-bar pixel specs differ from §6 production tokens,
§6 wins.**

---

## 6. Universal Components -- How to Build

Canonical CSS/HTML patterns for components that appear across multiple
mockups. Copy and adapt; do not reinvent.

### 6.1 Top Chrome (98px)

Three horizontal strips, total height exactly 98px.

```
+---------------------------------------------------------------------- 44px --+
| Logo | BPM 48px BLUE | BAR | PHRASE | STATE PILL   Beat   Transport    Mode  |
|      |               |     |        |              Wheel  mini-ctrls   tabs  |
|      |               |     |        |                                FPS REC|
+---------------------------------------------------------------------- 32px --+
| TRANSPORT: [>] [||] [#]  128 [-][+] [|<][>|] [/2][x2] [TAP] [RESYNC] [loop] |
+---------------------------------------------------------------------- 22px --+
| RMS 0.72 | PEAK 0.91 | LUFS -14 | FLUX 0.33 | BASS 0.81 | MID 0.45 | ...   |
+----------------------------------------------------------------------------- +
```

```css
.top-chrome       { position: fixed; top: 0; left: 0; right: 0;
                    height: var(--chrome-total); z-index: 10;
                    display: flex; flex-direction: column; }
.top-bar          { height: var(--chrome-top); background: var(--panel);
                    border-bottom: var(--hairline) solid var(--border);
                    display: flex; align-items: center; padding: 0 12px; }
.transport-strip  { height: var(--chrome-transport); background: var(--bg);
                    border-bottom: var(--hairline) solid var(--border);
                    display: flex; align-items: center; padding: 0 12px; gap: 0; }
.signal-bar       { height: var(--chrome-signal); background: var(--bg);
                    border-bottom: var(--hairline) solid var(--border);
                    display: flex; align-items: center; padding: 0 8px;
                    font: 11px var(--font-mono); color: var(--value); cursor: pointer; }
.bpm-display      { font: 500 48px var(--font-mono); color: var(--accent); line-height: 1; }
.state-pill       { background: var(--accent); color: var(--bg);
                    font: 500 10px var(--font-sans); text-transform: uppercase;
                    letter-spacing: 1px; padding: 2px 8px; }
.mode-tab         { font: 11px var(--font-sans); text-transform: uppercase;
                    letter-spacing: 0.5px; color: var(--label);
                    padding: 4px 12px; cursor: pointer; border: none; background: none; }
.mode-tab.active  { color: var(--accent); border-bottom: 2px solid var(--accent); }
```

### 6.2 Inspector Row (Resolume-Exact Grammar)

The most settled, most reused component. Every inspector row is identical:

```
[triangle] [Label 90px right #888] [Value 50px mono #e0e0e0] [-][+] [====|==== 14px]
```

```html
<div class="inspector-row">
  <span class="signal-triangle" data-state="unrouted"></span>
  <span class="row-label">Opacity</span>
  <span class="row-value">75</span>
  <button class="row-btn">-</button><button class="row-btn">+</button>
  <div class="row-slider">
    <div class="row-slider-fill" style="width: 75%"></div>
    <div class="row-slider-thumb" style="left: 75%"></div>
  </div>
</div>
```

```css
.inspector-row    { display: flex; align-items: center;
                    height: var(--row-height-inspector);
                    padding: 0 4px; border-bottom: var(--hairline) solid var(--separator); }
.signal-triangle  { width: 7px; height: 7px; margin-right: 4px; flex-shrink: 0;
                    clip-path: polygon(0% 0%, 100% 50%, 0% 100%); }
.signal-triangle[data-state="unrouted"]  { background: var(--border); }
.signal-triangle[data-state="selected"]  { background: var(--label); }
.signal-triangle[data-state="active"]    { background: var(--accent);
                    /* opacity floor 15% — set via JS:
                       el.style.opacity = 0.15 + val * 0.85; */ }
.row-label        { width: var(--label-width); text-align: right;
                    font: 11px var(--font-sans); color: var(--label);
                    padding-right: 8px; flex-shrink: 0;
                    overflow: hidden; text-overflow: ellipsis; white-space: nowrap; }
.row-value        { width: var(--value-width); text-align: right;
                    font: 11px var(--font-mono); color: var(--value);
                    padding-right: 4px; flex-shrink: 0; }
.row-btn          { width: var(--button-size); height: var(--button-size);
                    background: var(--bg); color: var(--label);
                    border: var(--hairline) solid var(--border);
                    font: 11px var(--font-sans); cursor: pointer;
                    display: flex; align-items: center; justify-content: center;
                    flex-shrink: 0; margin: 0; /* buttons touch */ }
.row-slider       { flex: 1; height: var(--slider-height); background: var(--bg);
                    border: var(--hairline) solid var(--border);
                    position: relative; margin-left: 4px; }
.row-slider-fill  { height: 100%; background: var(--panel); }
.row-slider-thumb { position: absolute; top: 0; bottom: 0; width: 1px;
                    background: var(--accent); }
```

### 6.3 Section Header

```
[triangle] SECTION NAME                              [2px blue left bar if routed]
```

```html
<div class="section-header" data-routed="true">
  <span class="disclosure-triangle">&#9660;</span>
  <span class="section-name">TRANSFORM</span>
</div>
```

```css
.section-header   { display: flex; align-items: center;
                    height: var(--row-height-section); background: var(--section-header);
                    padding: 0 8px; position: relative; cursor: pointer; }
.section-header[data-routed="true"]::before {
                    content: ''; position: absolute; left: 0; top: 0; bottom: 0;
                    width: 2px; background: var(--accent); }
.disclosure-triangle { font-size: 7px; color: var(--label);
                    margin-right: 6px; flex-shrink: 0; }
.section-name     { font: 12px var(--font-sans); color: var(--value);
                    text-transform: uppercase; letter-spacing: 0.5px; }
```

**No P. button. No route-entire-section button.** Section headers carry
ONLY the passive 2px blue left-edge indicator.

### 6.4 Vertical Signal Column

The universal control element. Replaces all knobs.

```
    +-------+
    |       |  <-- signal fill (blue, bottom-to-top, smoothed)
    | ===== |  <-- user value tick (draggable, white/light line)
    |#######|  <-- filled portion
    |       |
    +-------+
    MIDI:C1     <-- always-visible MIDI/OSC binding
```

```css
.signal-column      { width: var(--signal-column-width); height: 120px;
                      background: var(--bg); border: var(--hairline) solid var(--border);
                      position: relative; display: flex; flex-direction: column; }
.signal-column-fill { position: absolute; bottom: 0; left: 0; right: 0;
                      background: var(--accent); opacity: 0.4;
                      transition: height 0.08s linear; /* smoothed fill */ }
.signal-column-tick { position: absolute; left: 2px; right: 2px;
                      height: 2px; background: var(--value); }
.signal-column-midi { position: absolute; bottom: 2px; left: 0; right: 0;
                      text-align: center; font: 8px var(--font-mono); color: var(--label); }
```

**Where columns appear:**
- Dashboard section (replaces circular knob row entirely)
- Layer strip left side (auto-populated, scrolls horizontally, 4-6 visible)
- Inspector (anywhere a knob would traditionally be)
- Bottom focus bar / macro area

### 6.5 Clip Cell (100x80 base)

```
+-----------------------+
|W|                     |    W = 6px per-clip waveform (dim blue)
|W|    thumbnail        |    Thumbnail = content-appropriate visual
|W|                     |
+-----------------------+
| clip-name  9px        |    Name bar = 14px, hairline top border
+-----------------------+
```

```css
.clip-cell        { width: var(--clip-width); height: var(--clip-height);
                    background: var(--bg); border: var(--hairline) solid var(--border);
                    position: relative; overflow: hidden; display: flex; }
.clip-waveform    { width: var(--clip-wave-width); background: var(--bg); flex-shrink: 0; }
.clip-waveform svg { width: 100%; height: 100%; fill: none;
                    stroke: var(--accent); stroke-width: 1; opacity: 0.4; }
.clip-thumbnail   { flex: 1; background-size: cover; background-position: center; }
.clip-name        { position: absolute; bottom: 0; left: 0; right: 0;
                    height: var(--clip-name-height);
                    border-top: var(--hairline) solid var(--border);
                    padding: 0 4px; font: 9px var(--font-sans); color: var(--label);
                    display: flex; align-items: center; background: rgba(26,26,26,0.85); }
/* States */
.clip-cell.playing  { background: rgba(0,217,255,0.3); border: 2px solid var(--accent); }
.clip-cell.queued   { border: 2px dashed var(--accent); }
.clip-cell.selected { border: 1px solid var(--accent); }
.clip-cell.empty    { border: 1px dashed var(--border); }
```

### 6.6 Hit Pill (8x24 capsule)

Uniform-size vertical capsule on the timeline. Three S/C/M indicators inside.

```
+--+
|* |  <-- S (Signals) blue dot when present
|* |  <-- C (Clips) blue dot when present
|* |  <-- M (Macros) blue dot when present
+--+
```

```css
.hit-pill           { width: var(--pill-width); height: var(--pill-height);
                      background: var(--bg); border: var(--hairline) solid var(--border);
                      display: flex; flex-direction: column; align-items: center;
                      justify-content: space-evenly; cursor: pointer; }
.hit-pill.selected  { border: 2px solid var(--accent); }
.hit-pill.active    { background: var(--accent); }
.hit-indicator      { width: 4px; height: 4px; /* square, no border-radius */ }
.hit-indicator.present { background: var(--accent); }
.hit-indicator.absent  { background: transparent; }
```

### 6.7 Triangle States (Summary)

All three states for the universal signal access triangle:

| State    | Appearance                                                  |
|----------|-------------------------------------------------------------|
| Unrouted | Very light outline (`--border` ~#3a3a3a), barely visible    |
| Selected | Solid light fill (`--label` ~#888)                          |
| Active   | Blue `--accent` fill, opacity = `0.15 + (signal_value × 0.85)`. Minimum visible at signal=0, ramps to full at signal=1. RAW 1:1 mapping above the floor — no easing, no smoothing. |

Active state is RAW 1:1 mapping above a 15% opacity floor. A routed
parameter at signal=0 stays dimly visible (so the user can see what's
routed) and ramps to full opacity at signal=1 with no easing or
smoothing. The opacity floor is the ONLY constant added to the otherwise
raw mapping.

### 6.8 Orange Override Dot

Per-field indicator at row end when field is in OVERRIDE state.

```css
.override-dot     { width: 6px; height: 6px; background: var(--override);
                    margin-left: auto; flex-shrink: 0; cursor: pointer; }
```

Appears at three zoom levels:
1. **Per-field:** orange dot at right end of the inspector row
2. **Group-level:** orange indicator on layer strip header or section header
3. **App-wide:** passive STATUS indicator in top chrome (not clickable for mass-release)

---

## 7. Per-Mockup Brief Template

Copy this template for each new mockup. Fill in the placeholders.

```markdown
## Mockup: <TAG> -- <Title>

**File:** design/mockups/html/v10/v10_<tag>_<slug>.html
**Reference mockups to study:** <list of v9_brut_* files to study before building>
**Layout type:** <Performance / Setup / Hybrid / Detail>
**Status:** <Draft / In Review / Approved>

### Layout Structure

<ASCII diagram of areas with rough pixel dimensions>

### Feature Coverage Checklist

All features below must be visible in this mockup:

- [ ] Feature 1
- [ ] Feature 2
- [ ] ...

### State Variations to Render

- Default state (what the user sees on launch or in normal operation)
- Active state (with visible signal/Hit activity, pulsing triangles)
- OVERRIDE state with orange indicators —
  **REQUIRED for: h1, p1, inspector, bottom_focus, signal_drawer.
  Optional for: top_chrome, s2, hit_system, clip_detail, states.**
- (Optional) Hover state

### What to Get Right

<Specific guidance for this mockup: what Boris will look at first,
what the mockup is PROVING, what traps to avoid>

### Verification Criteria

Universal checks (apply to ALL mockups):

- [ ] All design tokens match Section 2 exactly (colors, fonts, spacing)
- [ ] No border-radius anywhere (grep check)
- [ ] No box-shadows anywhere (grep check)
- [ ] No gradients on UI chrome (grep check)
- [ ] No circular knobs -- vertical signal columns only
- [ ] Triangle-only routing (no P. button on rows or section headers)
- [ ] ALL-CAPS section headers with letter-spacing
- [ ] Monospace for numbers, Sans for labels
- [ ] BPM at 48px Plex Mono 500-weight blue (if top chrome visible)
- [ ] 3% grain overlay present (body::after)
- [ ] No italic, no underline (except active mode tab)
- [ ] No emoji / decorative icons
- [ ] Buttons touch with no gaps
- [ ] 1px hairline borders only

Mockup-specific checks:

- [ ] <Specific check 1>
- [ ] <Specific check 2>
- [ ] <Specific check 3>
```

---

## 8. Workflow

### 8.1 How Harmony Dispatches a Mockup Builder

1. **Harmony selects a mockup** from the V1 plan (Section 9) based on
   priority and Boris's approval.

2. **Harmony writes a thin work packet** containing:
   - The filled-in per-mockup brief template (Section 7)
   - Reference: "Read MOCKUP_BRIEF.md Sections 2-6 for universal standards"
   - Any Boris-specific guidance for this mockup
   - Target file path in `design/mockups/html/v10/`

3. **Harmony dispatches a Builder** with the work packet.

### 8.2 How the Builder Produces the Mockup

1. **Read MOCKUP_BRIEF.md** Sections 2-6 (universal standards). Do not
   skip -- these are your constraints.

2. **Read the per-mockup brief** in the work packet. Study the reference
   mockups listed (they are in `design/mockups/html/v9/` or earlier
   directories -- READ-ONLY, never modify).

3. **Build the HTML file** following:
   - HTML structure from Section 3.2
   - Design tokens from Section 2 (copy the CSS variables block)
   - Component patterns from Section 6 (copy and adapt)
   - Iron rules from Section 4 (violate nothing)

4. **Self-check** against the verification criteria in the brief before
   reporting done. Run the grep checks from Section 4.6.

### 8.3 How the Tester Verifies

1. **Open the mockup file** and read the HTML/CSS.

2. **Run the Section 4.6 grep checks** for iron rule violations.

3. **Check every universal verification criterion** from the template
   (Section 7).

4. **Check every mockup-specific criterion** from the work packet.

5. **Compare against the Quality Bar** (Section 5) for any components
   that overlap with Boris's "best" elements.

6. **Report:** PASS / PASS_WITH_CONCERNS / FAIL with specific violations.

### 8.4 Feedback Loop

- Tester FAIL or CONCERNS: Builder fixes, Tester re-verifies.
- Tester PASS: Harmony presents to Boris for review.
- Boris feedback: Harmony creates follow-up work packet referencing
  this doc + Boris's notes.

### 8.5 Token Sync Verification

Because every v10 mockup is a self-contained HTML file (§3.2), the design
token `:root` block is duplicated across 10 files. To prevent drift when
a token value changes:

**Canonical source:** MOCKUP_BRIEF §2.1 (color tokens), §2.2 (font tokens),
and §2.3 (spacing tokens). These are the ONLY authoritative token values.

**Tester procedure (run for every v10 mockup PR):**

1. Extract the `:root { ... }` block from the mockup file.
2. Compare each declared CSS variable name + value against the canonical
   list from MOCKUP_BRIEF §2.1, §2.2, §2.3.
3. Every canonical variable MUST appear in the mockup's `:root`.
4. Every value MUST match the canonical exactly (no rounding, no
   substitution).
5. Extra variables (mockup-specific) are allowed if prefixed `--mockup-`
   or `--local-`. Otherwise FAIL.

**On failure:** Tester reports the diff (variable name + canonical value
+ mockup value). Builder updates the mockup to match canonical.

**On canonical change:** if Boris approves a token edit in MOCKUP_BRIEF,
a follow-up Builder propagates the change to every existing v10 mockup.
The Tester check then re-runs against the new canonical.

A scripted version of this check is a follow-up task — for V1 the check
is performed manually by the Tester against the canonical blocks.

---

## 9. The 10-Mockup V1 Plan

Planned mockups for the v10 series. Per-mockup brief details are NOT in
this document -- they will be created as Boris approves each one.

| #  | Tag            | Title                                    | Type    | What It Proves                                         |
|----|----------------|------------------------------------------|---------|--------------------------------------------------------|
| 1  | `h1`           | Default Launch Layout                    | Hybrid  | Full composition: top chrome + clip grid + inspector + layer strips + bottom focus + signal columns. The "everything works together" proof. |
| 2  | `top_chrome`   | Top Chrome 98px Detail                   | Detail  | 3-strip structure at pixel-perfect scale. BPM, transport, signal bar, mode tabs, beat wheel, state pill. |
| 3  | `inspector`    | Inspector Grammar + VerticalSignalColumn | Detail  | Control language close-up: inspector rows, section headers with routing indicator, signal columns, triangle states. |
| 4  | `s2`           | "Perfect" Programming Layout             | Setup   | Full signal programming workspace: signals left, clips center, parameters right, routing visualization. |
| 5  | `p1`           | Performance Layout                       | Perf    | LIVE mode: large clip grid, output preview, performance-optimized controls, quick signal access. |
| 6  | `bottom_focus` | Bottom Focus Bar (3 States)              | Detail  | Collapsed summary, expanded macro view, expanded Hit anatomy. Three contextual states of the same component. |
| 7  | `hit_system`   | Hit System Composite                     | Detail  | Timeline with pills, capture dialog, envelope curves between Hits, chain lines, two-lane (Hits + REC). |
| 8  | `signal_drawer`| Signal Drawer Overlay                    | Detail  | Expanded signal drawer overlaying full workspace. All 42+ signals, grouping, interaction. |
| 9  | `clip_detail`  | Clip Cell + Waveform Detail              | Detail  | All cell states (playing/queued/selected/loaded/empty), per-clip waveform, name bar, thumbnail. |
| 10 | `states`       | Empty vs Active States                   | Split   | Split view: left = empty composition launch state, right = active performance with signals flowing. |

### Priority Order

Start with H1 (the full composition) -- it forces every component to work
together and surfaces integration issues early. Then Top Chrome and Inspector
(the most reused components). Then specific layouts and detail views.

### File Naming Preview

```
design/mockups/html/v10/
  v10_h1_default_layout.html
  v10_top_chrome_98px.html
  v10_inspector_grammar.html
  v10_s2_programming.html
  v10_p1_performance.html
  v10_bottom_focus_states.html
  v10_hit_system_composite.html
  v10_signal_drawer_overlay.html
  v10_clip_detail_states.html
  v10_states_empty_vs_active.html
```
