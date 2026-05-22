# UI Detail Specs -- Audio-DNA Gap-Fill Areas

> Companion document to AUDIO_DNA_DIRECTIVE_FULL.md. Provides internal
> layout, content hierarchy, controls, state variations, and behavioral
> specs for four under-specced UI areas that block mockups #5--#8 in the
> MOCKUP_BRIEF V1 plan.
>
> **Date:** 2026-05-22
> **Status:** Draft -- pending Boris review before mockup dispatch.
>
> **Canonical references:**
> - AUDIO_DNA_DIRECTIVE_FULL.md (source of truth)
> - FEATURE_CONNECTIONS.md (behavioral rules)
> - MENTAL_MODELS.md (conceptual spine)
> - BORIS_DECISIONS.md (locked decisions)
> - MOCKUP_BRIEF.md (mockup standards + V1 plan)

---

## Table of Contents

1. [Area 1 -- Hit Inspector Internal Layout](#area-1----hit-inspector-internal-layout)
2. [Area 2 -- Signal Drawer Configuration UI](#area-2----signal-drawer-configuration-ui)
3. [Area 3 -- Output Window Tabs Structure](#area-3----output-window-tabs-structure)
4. [Area 4 -- Macro / Signal Group Detail Editor](#area-4----macro--signal-group-detail-editor)

---

## Area 1 -- Hit Inspector Internal Layout

### Purpose

The Hit Inspector is the primary editing surface for show programming.
When a user selects a Hit on the timeline, the Bottom Focus Bar expands
to reveal the Hit's full anatomy -- every payload category, every
envelope setting, every condition -- using the same inspector grammar
as the rest of the application (AUDIO_DNA_DIRECTIVE_FULL.md Section 5.6).

Architecturally, the Hit Inspector is the editing surface for the **Hits
intent layer** -- one of three named intent layers (Signal / Hits / Live VJ)
that organize all timeline-driven behavior. See BORIS_DECISIONS layer model
and FEATURE_CONNECTIONS Scenario 11. Edits made in this Inspector affect
the Hits layer specifically; Signal and Live VJ layers have their own
editing surfaces elsewhere in the application.

### Trigger

- User clicks an existing Hit pill on the Hits timeline lane. Per
  FEATURE_CONNECTIONS.md Scenario 11 Section 11.4: "Click existing Hit
  = jumps playhead to Hit's position + selects Hit + opens Hit Inspector
  in Bottom Focus Bar."
- Bottom Focus Bar expands upward from its collapsed summary state.
  Default expanded height approximately 30% of screen, user-draggable
  (AUDIO_DNA_DIRECTIVE_FULL.md Section 1.7).

### Layout

The Hit Inspector occupies the full width of the Bottom Focus Bar's
expanded area. It is organized as a horizontal arrangement of vertical
panels, read left to right, following the order of the Hit's anatomy
as listed in AUDIO_DNA_DIRECTIVE_FULL.md Section 2.11.

```
+-----------------------------------------------------------------------------+
| HIT HEADER BAR (24px)                                                       |
| [Hit name/ID] [Position: bar.beat] [Power: 0-1 slider] [Envelope summary]  |
+-------+--------+--------+--------+---------+---------+---------------------+
|       |        |        |        |         |         |                     |
| SGNLS | CLIPS  | MACROS | PARAMS | ENVLPE  | TIMING  | CONDITIONS + CHAIN |
| panel | panel  | panel  | panel  | panel   | panel   | panel              |
|       |        |        |        |         |         |                     |
| (scrl)| (scrl) | (scrl) | (scrl) |         |         |                     |
+-------+--------+--------+--------+---------+---------+---------------------+
| FILTER BAR (24px, bottom-docked)                                            |
| [Select All] [Deselect All] [Show: Checked / Unchecked / All]              |
+-----------------------------------------------------------------------------+
```

**Vertical structure (3 fixed strips + scrollable content):**

1. **Hit Header Bar (24px, fixed top):** Identity + summary. Always
   visible, does not scroll.
2. **Content area (remaining height, scrollable per-panel):** The seven
   content panels arranged horizontally. Each panel scrolls vertically
   independently when its content exceeds the available height.
3. **Filter Bar (24px, fixed bottom):** Global filter actions for the
   Hit's filter checkboxes.

**Horizontal panel arrangement:** Seven panels, each separated by 1px
`--border` vertical hairlines. Panels have the following width behavior:

- **Signals, Clips, Macros, Params:** Flexible width, approximately
  equal share of available space. Minimum width to show one inspector
  row without horizontal truncation (approximately 250px each).
- **Envelope:** Fixed width approximately 200px (contains curve controls).
- **Timing + Conditions + Chain:** Fixed width approximately 250px
  (grouped together as the rightmost panel since these are smaller
  content areas).

If the total width exceeds the available space, the content area scrolls
horizontally. At 1920px with Bottom Focus Bar at full width, all panels
fit without horizontal scroll.

**Panel collapse/expand:** Each panel has a section header (24px,
`--section-header` background, disclosure triangle) that collapses its
content. Collapsed panels shrink to header-only width (approximately
24px rotated or the header text width), freeing space for adjacent
panels.

### Content per region

**Hit Header Bar:**
- Hit name/ID: editable text field, Plex Sans 12px `--value`
- Position: bar.beat display, Plex Mono 11px `--accent` (read-only,
  derived from the Hit's grid position)
- Power/intensity: 0 to 1 slider using the standard inspector row
  grammar (triangle + label + value + minus/plus + slider). This is
  the global magnitude of the Hit's effect
  (AUDIO_DNA_DIRECTIVE_FULL.md Section 2.11)
- Envelope summary: compact text showing current Hit-level envelope
  ("LINEAR 4 beats" or "INSTANT"), Plex Sans 11px `--label`. Clicking
  opens the Envelope panel

**Signals panel (section header: SIGNALS CAPTURED):**
- List of all signal sources in the Hit's snapshot
- Each signal: one inspector row per the universal grammar
  - Triangle: shows if signal is active in this Hit
  - Label: signal name (e.g., BASS, FLUX, SIDECHAIN PUMP)
  - Value: on/off state
  - Filter checkbox: right end of row (checked = included in Hit,
    unchecked = excluded)
- Grouped by signal category (Amplitude, Bands, Rhythm, etc.) using
  standard section headers with disclosure triangles
- Activations shown in blue `--accent`; deactivations shown in
  `--label` dim

**Clips panel (section header: CLIPS CAPTURED):**
- Two sub-sections with section headers:
  - **Cell triggers:** individual cells fired. Each row shows
    layer/column label (e.g., "L1-C3"), clip name, thumbnail (mini,
    approximately 24x18px). Filter checkbox per row.
  - **Column triggers:** full column (scene) triggers. Each row shows
    column name/label (e.g., "DROP"), number of layers affected. Filter
    checkbox per row.
- Rows use inspector grammar adapted for clip content (thumbnail
  replaces the triangle position)

**Macros panel (section header: MACROS CAPTURED):**
- List of macro (signal group) activations/deactivations captured by
  the Hit
- Each row: macro name, scope badge (G/L/C for Global/Layer/Clip,
  Plex Mono 9px), activation state (on/off), filter checkbox
- Grouped by scope (Global first, then Layer, then Clip) using section
  headers. Follows mixing-desk hierarchy
  (MENTAL_MODELS.md Section 2: Global > Layer > Clip)

**Parameters panel (section header: PARAMETER SNAPSHOTS):**
- List of all parameter values captured in this Hit
- Each row: standard inspector row grammar (triangle + label + value +
  minus/plus + slider + filter checkbox)
- Grouped by scope, then by parameter category (Transform, Effects,
  etc.) using section headers
- Values are editable in-place -- changing a value here changes what
  the Hit will set when it fires
- Per-parameter envelope override indicator: small icon (curve glyph)
  appears after the slider if this parameter has an envelope override.
  Clicking it expands a sub-row showing the override fields
  (FEATURE_CONNECTIONS.md Scenario 7)

**Envelope panel (section header: ENVELOPE):**
- Hit-level defaults at the top:
  - Onset: dropdown/selector (Instant / Fade-in) + duration field
    (beats or bars, Plex Mono)
  - Curve: selector (Linear / Ease-in / Ease-out / Ease-in-out /
    S-curve / Exponential / Step). Default: LINEAR
    (FEATURE_CONNECTIONS.md Section 5.6)
  - Release: dropdown (Instant cut / Fade-out) + duration field
- Curve preview: small visual showing the selected curve shape
  (approximately 60x40px, using hairline stroke in `--accent`)
- Below the defaults: per-parameter override list. Shows only
  parameters that have overrides. Each override row shows the parameter
  name and only the fields that differ from Hit-level defaults.
  Unspecified fields show "inherited" in `--label` dim
  (FEATURE_CONNECTIONS.md Scenario 7)
- Each envelope field (onset, curve, release) has its own AUTO/OVERRIDE
  state per the universal field-level rule
  (FEATURE_CONNECTIONS.md Section 3.1)

**Timing panel (section header: TIMING):**
- Position: bar.beat.sub-beat, Plex Mono 11px (read-only, same as
  header bar but with sub-beat precision)
- Quantize setting active when this Hit was placed: Exact beat /
  Next bar / Closest bar (informational, `--label`)
- Duration: if this Hit has onset/release fade, shows the total
  transition duration

**Conditions panel (section header: CONDITIONS):**
- Signal-dependent gating rules
  (AUDIO_DNA_DIRECTIVE_FULL.md Section 2.6)
- Each condition row: signal name + operator (>, <, =) + threshold
  value. Standard inspector row grammar for the threshold slider.
- Example: "BASS > 0.7" -- this Hit only fires if BASS exceeds 0.7
- Multiple conditions combine with AND logic (all must be met)

**Chain Links panel (section header: CHAIN LINKS):**
- List of other Hits that this Hit triggers next
  (AUDIO_DNA_DIRECTIVE_FULL.md Section 2.10)
- Each row: target Hit name/ID + timing offset (in beats/bars,
  Plex Mono) + remove button (functional glyph only, no emoji)
- Chains are visualized on the timeline as connecting lines between
  Hit pills. This panel is the edit surface for those connections.

**Filter Bar (bottom-docked):**
- Select All / Deselect All buttons (standard button styling, buttons
  touch)
- Show filter: toggle between Checked / Unchecked / All to filter
  visible rows in the panels above
- Filter checkboxes are the mechanism from
  AUDIO_DNA_DIRECTIVE_FULL.md Section 2.4: "A filter step appears
  showing what was captured -- user unchecks anything they don't want
  this Hit to control"

### Controls + interactions

- **Panel collapse/expand:** Click section header disclosure triangle to
  collapse/expand individual panels
- **Filter checkboxes:** Per-row checkboxes in Signals, Clips, Macros,
  Params panels. Checked = this item is included in the Hit's effect.
  Unchecked = this item is excluded (left unaffected when Hit fires)
- **Value editing:** Inspector row values (sliders, minus/plus buttons)
  in the Params panel are editable. Changing a value changes the Hit's
  captured state.
- **Envelope editing:** Dropdown selectors and duration fields in the
  Envelope panel. Per-parameter overrides added by clicking the curve
  glyph on a parameter row.
- **Condition editing:** Add/remove conditions via buttons in the
  Conditions panel. Threshold values adjustable via inspector row
  sliders.
- **Chain editing:** Add chain links via a "Link Hit" action (opens a
  Hit picker -- list of other Hits in the composition). Remove via
  the remove button on each chain row.
- **Bottom Focus Bar resize:** Drag the top edge of the expanded Bar
  to resize vertically (existing behavior per
  AUDIO_DNA_DIRECTIVE_FULL.md Section 1.7).
- **Keyboard:** Tab navigates between panels. Arrow keys navigate
  rows within a panel. Enter toggles filter checkbox on focused row.
  Escape closes the expanded Bottom Focus Bar.

### State variations

**Empty state (Hit with no captured content):**
- All panels show "No [items] captured" placeholder text in `--label`
  dim. This state occurs immediately after the capture button is
  pressed and before the filter step is confirmed (or if a user
  creates a Hit at an empty composition moment).

**Populated state (normal):**
- All panels show their content rows. Checked items have standard row
  styling. Unchecked items have reduced opacity (approximately 40%)
  and the filter checkbox is unchecked.

**Error state:**
- Invalid condition (e.g., condition references a signal that no
  longer exists): row background tints slightly with `--danger` at
  10% opacity. Text shows the missing signal name in `--danger`.
- Broken chain link (target Hit deleted): chain row shows "Missing:
  [Hit name]" in `--danger`.

**Loading state:**
- When opening a large Hit (many parameters), panels show a loading
  indicator: a thin `--accent` progress bar at the top of each panel
  filling left to right.

**OVERRIDE conflict state:**
- If a parameter in this Hit is currently in OVERRIDE mode (VJ has
  manually held it), the parameter row in the Params panel shows the
  orange `--override` dot at the row end, matching the standard
  per-field override indicator
  (FEATURE_CONNECTIONS.md Section 3.3). This communicates: "this Hit
  will try to set this value, but it is currently blocked by an
  override."

### Edge cases / gotchas

- A Hit can capture parameters across all three scopes (Global, Layer,
  Clip). The Params panel must group by scope and respect the
  mixing-desk hierarchy display order: Global first, then Layer, then
  Clip.
- Column triggers and individual cell triggers can coexist in the same
  Hit. If a column trigger and an individual cell trigger on the same
  layer conflict, the column trigger takes precedence (it fires all
  cells in the column). The Clips panel should visually indicate when
  a cell trigger is redundant because its column is already triggered.
- Per-parameter envelope overrides apply only to the parameter they
  are set on. The Envelope panel must clearly distinguish Hit-level
  defaults from per-parameter overrides -- mixing them causes confusion
  in show programming.
- The "Two-Lane Shared Timeline" model
  (FEATURE_CONNECTIONS.md Scenario 11, Section 11.2) means the Hit
  Inspector can be open while the Recording lane is the active driver.
  Hits are always editable regardless of active driver
  (FEATURE_CONNECTIONS.md Section 11.5).
- Filter checkboxes are per-item, not per-panel. There is no way to
  "check all signals but uncheck all clips" in one action beyond using
  panel-level select-all within each panel.

### Open questions

1. **Panel width ratio at different screen sizes:** At 1920px, seven
   panels fit. At narrower viewports (or when the Bottom Focus Bar is
   not full-width), should panels stack vertically instead of scrolling
   horizontally? Or should the horizontal scroll be the universal
   fallback?

2. **Per-parameter envelope override discovery:** The current spec uses
   a small curve glyph on the parameter row to indicate an override
   exists. Is this discoverable enough for new users? Alternative:
   a dedicated "Parameter Overrides" sub-section within the Envelope
   panel that lists all parameters with overrides.

3. **Condition logic:** The spec above uses AND logic for multiple
   conditions ("all must be met"). Should OR logic be available?
   This would enable "fire if BASS > 0.7 OR if FLUX > 0.5" which is
   a different compositional tool than "fire if BASS > 0.7 AND FLUX
   > 0.5."

4. **Chain link timing:** Chain links have a timing offset (in
   beats/bars). Can the offset be zero (simultaneous firing of chained
   Hits)? If so, how does this differ from stacking parallel Hits at
   the same position (which the quantize grid prevents per
   FEATURE_CONNECTIONS.md Scenario 3)?

5. **Collapsed Bottom Focus Bar summary for Hits:** When the Bottom
   Focus Bar is collapsed, it shows a summary line. For a selected
   Hit, the directive gives the example "MACRO: BASS PUMP -> 4 params."
   What is the canonical summary format for a Hit? Proposed:
   "HIT [bar.beat]: [N] signals, [M] clips, [K] macros"
   -- but Boris may want a different format.

---

## Area 2 -- Signal Drawer Configuration UI

### Purpose

The Signal Drawer is the expanded overlay for managing and configuring
all audio signals at the global level. It is the primary surface for
seeing all 42+ audio features at once, organizing them into groups,
controlling per-signal behavior, and managing global-scope signal
routing -- the core differentiator feature that Resolume does not have
(AUDIO_DNA_DIRECTIVE_FULL.md Section 1.6, Section 7).

### Trigger

- User clicks the 22px signal quick-ref bar in the top chrome
  (AUDIO_DNA_DIRECTIVE_FULL.md Section 1.6).
- The bar "drops down as a full-height drawer, overlaying the entire
  workspace below it" (AUDIO_DNA_DIRECTIVE_FULL.md Section 1.6).
  Dismiss by clicking outside the drawer, pressing Escape, or
  clicking the signal bar again.
- Z-index: 200 (above Bottom Focus Bar at 100, below tooltips at 500)
  per MOCKUP_BRIEF.md Section 3.5.

### Layout

The Signal Drawer overlays from the top chrome downward, covering the
full workspace. It does not push content down -- it overlays.

```
+-----------------------------------------------------------------------------+
| TOP CHROME (98px, always visible above the drawer)                          |
+=============================================================================+
| SIGNAL DRAWER OVERLAY                                                       |
+------+----------------------------------------------------------------------+
|      |                                                                      |
| CAT  |  SIGNAL CONTENT AREA                                                |
| NAV  |                                                                      |
| BAR  |  +------------------+  +------------------+  +------------------+   |
|      |  | CATEGORY GROUP 1 |  | CATEGORY GROUP 2 |  | CATEGORY GROUP 3 |   |
| 60px |  | (expandable)     |  | (expandable)     |  | (expandable)     |   |
|      |  +------------------+  +------------------+  +------------------+   |
|      |                                                                      |
|      |  +------------------+  +------------------+  +------------------+   |
|      |  | CATEGORY GROUP 4 |  | CATEGORY GROUP 5 |  | CATEGORY GROUP 6 |   |
|      |  |                  |  |                  |  |                  |   |
|      |  +------------------+  +------------------+  +------------------+   |
|      |                                                                      |
+------+----------------------------------------------------------------------+
| SCOPE FILTER BAR (24px, bottom-docked)                                     |
| [ALL] [GLOBAL] [LAYER ▼] [CLIP ▼]                                         |
+-----------------------------------------------------------------------------+
```

**Vertical structure (3 fixed strips + scrollable content):**

1. **Drawer header:** Implicit -- the 22px signal bar in the top chrome
   acts as the header. When expanded, it changes from compact values to
   a close/collapse affordance.
2. **Content area (remaining height minus scope filter bar):** Two
   sub-regions side by side:
   - Category navigation bar (left, 60px wide)
   - Signal content area (remaining width)
3. **Scope filter bar (24px, fixed bottom):** Scope filtering controls.

**Content area arrangement:** The signal content area uses a grid
layout of category groups. Groups are arranged in a responsive grid:
3 columns at 1920px width, 2 columns at narrower widths. Each group is
a collapsible card-like region (no border-radius, 1px `--border`).

### Content per region

**Category navigation bar (left, 60px):**
- Vertical list of category icons/labels for quick-jump navigation
- Categories match the signal grouping from the expanded sidebar
  reference (MOCKUP_BRIEF.md Section 5.3):
  - AMPLITUDE (RMS, Peak, LUFS)
  - BANDS (Bass, Mid, High, Sub-bass)
  - RHYTHM (BPM, Beat phase, Swing, Sidechain pump)
  - PITCH (Centroid, Formant, Resonance)
  - STRUCTURE (Drop/Buildup/Breakdown/Normal state)
  - SPECTRAL (Flux, Rolloff, Flatness)
  - ADVANCED (Reese bass, custom derived signals)
- Each category label: Plex Sans 9px, uppercase, `--label`. Active
  category highlighted with `--accent` left-edge 2px bar.
- Click a category to scroll the content area to that group.

**Category groups (in the content area):**

Each category group contains:

- **Group header (24px):** Section header using standard section header
  pattern (disclosure triangle + category name + signal count badge).
  Background `--section-header`.
- **Signal rows:** One row per signal in the category. Each row uses a
  modified inspector row grammar:

```
[triangle] [Label] [mini bar 5px-h] [Value mono] [Scope badge] [Active toggle]
```

  - Triangle: signal routing indicator (pulsing if signal is currently
    active/routed anywhere in the composition)
  - Label: signal name, Plex Sans 11px `--label`
  - Mini bar: horizontal bar showing current live signal value, 5px
    height, fills with `--accent`. Updates in real time. This matches
    the signal bar row pattern from MOCKUP_BRIEF.md Section 5.1
    (active state background `#1a3040`, active text `--accent`).
  - Value: current numeric value, Plex Mono 11px `--value`
  - Scope badge: shows which scopes this signal is routed in. Small
    badges (G/L/C) in Plex Mono 9px. Dim `--label` when not routed at
    that scope, `--accent` when routed.
  - Active toggle: on/off toggle for whether this signal source is
    being analyzed. Inactive signals are not extracted (saves CPU).

**Signal detail (expanded row):**

Clicking a signal row expands it in-place to show:

- **Full-width live visualization:** Larger version of the mini bar
  (approximately 30px height) showing the signal's recent history as a
  mini waveform/sparkline (last approximately 4 seconds). Stroke
  `--accent`, background `--bg`.
- **Configuration controls:**
  - Smoothing: inspector row slider (0 to 100%, controlling the EMA
    alpha). Default varies by signal type.
  - Threshold: inspector row slider (0 to 1). Below threshold, signal
    reports 0.
  - Invert: toggle button. When on, signal value = 1 - raw value.
  - Range: min/max clamp values (two inspector row sliders).
- **Routing summary:** List of parameters this signal is routed to
  (across all scopes). Each routing shows the destination parameter
  name + scope badge + depth value.

**Scope filter bar (bottom-docked, 24px):**
- Filter buttons: ALL (shows every signal), GLOBAL (shows only signals
  with global-scope routing), LAYER (dropdown to select a layer, shows
  signals routed to that layer), CLIP (dropdown to select a clip, shows
  signals routed to that clip).
- Filtering does not hide signals -- it highlights matching signals and
  dims non-matching ones to approximately 40% opacity.
- Follows mixing-desk scope hierarchy
  (MENTAL_MODELS.md Section 2: Global > Layer > Clip).

### Controls + interactions

- **Expand/collapse drawer:** Click the 22px signal bar to toggle.
  Expansion is immediate (no animation per the brutalist aesthetic --
  though a fast slide of approximately 100ms is acceptable for spatial
  orientation).
- **Category quick-jump:** Click a category in the left navigation bar
  to scroll the content area to that category group.
- **Signal row expand:** Click a signal row to expand its detail view
  in-place. Only one signal detail is expanded at a time (clicking
  another collapses the current).
- **Signal toggle:** Click the active toggle to enable/disable a signal
  source. Disabled signals stop being extracted from the audio input.
- **Configuration controls:** Standard inspector row interactions
  (triangle click, slider drag, minus/plus buttons) within expanded
  signal detail rows.
- **Scope filtering:** Click filter buttons to highlight signals by
  scope. Layer/Clip dropdowns show available layers/clips.
- **Dismiss:** Click outside the drawer overlay, press Escape, or
  click the signal bar. The drawer slides up and the signal bar returns
  to its compact 22px display.
- **Keyboard:** Arrow keys navigate signal rows within a category.
  Tab moves between categories. Enter expands/collapses the selected
  signal row.

### State variations

**Empty state (no audio input):**
- All signal values show 0.00 in `--label` dim
- Mini bars are empty (no fill)
- Sparklines in expanded rows show flat lines
- Category headers show count badges at full count (all signals
  exist, they just have zero values)
- Toggle states are preserved (user's signal on/off choices persist
  even without audio)

**Populated state (audio flowing):**
- Signal values update in real time
- Mini bars fill with `--accent` proportional to value
- Active (routed) signals have their rows highlighted with `#1a3040`
  background tint (the dark cyan-tinted active state from
  MOCKUP_BRIEF.md Section 5.1)
- Triangles pulse on routed signals (RAW, no smoothing per
  MENTAL_MODELS.md Section 7)

**Heavy routing state (many signals routed):**
- Scope badges light up on multiple signals
- The scope filter becomes essential for navigating -- GLOBAL filter
  shows only globally-routed signals, reducing visual noise.

**Error state:**
- Audio input disconnected mid-session: all values freeze at last
  known value. A small "NO INPUT" indicator appears in the drawer
  header area (using `--danger` color, per the REC/error-only rule).

**Loading state:**
- When the drawer first opens with a large composition, signal rows
  populate progressively (top categories first). A thin `--accent`
  progress bar at the top of the content area shows loading progress.

### Edge cases / gotchas

- The 42+ signals is a large number. At 24px per row, a single
  category with 10 signals takes 240px. With 7 categories, the full
  list needs approximately 1000px of vertical space. The grid layout
  (3 columns of category groups) reduces this to approximately 350px,
  which fits within the overlay. If categories have uneven signal
  counts, the grid reflows.
- Signal configuration (smoothing, threshold, range) is per the SETUP
  mode concerns (AUDIO_DNA_DIRECTIVE_FULL.md Section 1.3: triangle
  SETUP mode = "Configuration-level access to signal definitions,
  thresholds, source setup"). The Signal Drawer provides this
  configuration accessible from any mode, not just SETUP. This is
  intentional -- the drawer is a shortcut to signal configuration
  without switching modes.
- The scope filter operates on routing presence, not signal value. A
  signal with a high value but no routing is dimmed in GLOBAL/LAYER/CLIP
  filter views. A signal with zero value but active routing is
  highlighted.
- Category navigation bar occupies only 60px width. At this width,
  category labels must be abbreviated or use icons. Using abbreviated
  ALL-CAPS labels (AMP, BND, RHY, PIT, STR, SPC, ADV) at 9px rotated
  or stacked vertically.

### Open questions

1. **Signal configuration in the drawer vs SETUP mode:** The Signal
   Drawer provides configuration controls (smoothing, threshold, range)
   accessible from any mode. Does this make the SETUP mode's signal
   configuration redundant? Or should the drawer provide read-only
   access to these values, with editing restricted to SETUP mode?

2. **Derived signal creation:** The directive mentions derived signals
   (e.g., "bass + flux, smoothed, inverted" in
   AUDIO_DNA_DIRECTIVE_FULL.md Section 1.1). Where does the user
   CREATE a derived signal? The drawer seems like the natural location
   (an "Add Derived Signal" action at the bottom of the ADVANCED
   category), but this is not specified in the canonical docs.

3. **Category navigation bar width:** 60px is tight for text labels.
   Should this be icon-only (no text), abbreviated text, or a wider
   panel (80px)? The brutalist aesthetic suggests functional density
   favors abbreviated text over icons (no decorative icons rule per
   AUDIO_DNA_DIRECTIVE_FULL.md Section 4.3).

4. **Signal drawer height:** The directive says "full-height drawer,
   overlaying the entire workspace below it." At 1080px total screen
   with 98px top chrome, the overlay is 982px tall. Is this the
   intended behavior, or should the drawer have a maximum height
   (e.g., 70% of screen) with the workspace visible below?

5. **Global signal drawer vs Programming Mode left panel:** In
   Programming Mode (AUDIO_DNA_DIRECTIVE_FULL.md Section 1.5), the
   left panel shows "all available signals -- global, layer, and clip
   level." The Signal Drawer also shows signals. Are these the same
   content displayed in different containers, or do they serve
   different purposes (drawer = quick reference + config, programming
   left panel = routing workspace)?

---

## Area 3 -- Output Window Tabs Structure

### Purpose

The central display area in P1 (Performance Layout) provides tabbed
access to the composition's visual output, preview rendering, per-layer
isolation, and per-clip detail. The tabs allow the performer to switch
context within the central viewing area without changing the surrounding
control surfaces (AUDIO_DNA_DIRECTIVE_FULL.md Section 5.14).

### Trigger

- Visible by default in P1 (Performance Layout) as the central region
  of the interface (AUDIO_DNA_DIRECTIVE_FULL.md Section 6: "P1 --
  Central clip/output/layer display with tabs for each. Parameters on
  right connected to whatever is displayed").
- Tab switching: user clicks a tab label. The selected tab's content
  fills the central display area. The right-side inspector updates to
  show parameters for whatever is displayed.

### Layout

```
+-----------------------------------------------------------------------------+
| TAB BAR (28px)                                                              |
| [OUTPUT] [PREVIEW] [LAYER] [CLIP]                                          |
+=============================================================================+
|                                                                             |
|                                                                             |
|                        CONTENT AREA                                         |
|                   (fills remaining space)                                   |
|                                                                             |
|                                                                             |
|                                                                             |
+-----------------------------------------------------------------------------+
| CONTENT STATUS BAR (20px, bottom)                                           |
| [Resolution] [FPS] [Active layer/clip indicator]                            |
+-----------------------------------------------------------------------------+
```

**Vertical structure:**

1. **Tab bar (28px, fixed top of the central display region):** Four
   tabs in a horizontal row.
2. **Content area (remaining height):** Displays the selected tab's
   content. Aspect ratio preserving -- the visual output scales to fit
   within the available rectangle, letterboxing if necessary.
3. **Content status bar (20px, fixed bottom):** Contextual information
   about the displayed content.

### Content per region

**Tab bar:**
- Four tabs arranged left to right: OUTPUT, PREVIEW, LAYER, CLIP
- Tab styling matches the mode tab pattern:
  - Active tab: `--accent` text + 2px `--accent` bottom underline
  - Inactive tabs: `--label` text, no underline
  - Font: Plex Sans 11px, ALL-CAPS, letter-spacing 0.5px
  - Tabs touch (no gaps, buttons-touch rule)
  - Background: `--panel`
  - 1px `--border` bottom hairline separating tab bar from content

**Tab order rationale:** OUTPUT first because it is the most-used view
during performance (what the audience sees). PREVIEW second because it
is the next most common (previewing upcoming content). LAYER and CLIP
are deeper inspection views.

**OUTPUT tab content:**
- Full composition output -- what is being sent to the projector/LED
  wall. This is the final composited result of all layers, all effects,
  all signal modulation.
- Renders at the composition's output resolution, scaled to fit the
  available display area with letterboxing (black bars) if the aspect
  ratio does not match.
- Always live -- updates every frame.
- Right-side inspector shows: COMP tab (composition-level parameters).

**PREVIEW tab content:**
- Preview rendering of a selected or queued clip/layer/effect before
  it goes live. Shows what WILL happen when triggered.
- Content depends on what is selected in the clip grid or layer strip:
  - Selected clip: shows that clip's output in isolation
  - Queued clip: shows the queued clip ready to fire
  - No selection: shows "Select a clip or layer to preview" placeholder
    in `--label` dim, centered
- Smaller than OUTPUT by default. The preview pane occupies the full
  content area but renders at a reduced resolution for performance.
- Right-side inspector shows: parameters for the previewed item (CLIP
  or LAYER tab, depending on selection).

**LAYER tab content:**
- Isolated output of a single layer. Strips away all other layers to
  show what one layer contributes to the composition.
- Layer selector: a narrow horizontal strip (20px) at the top of the
  content area showing layer labels (L1, L2, L3...) as clickable tabs.
  Active layer highlighted with `--accent`. This is a sub-tab within
  the LAYER tab.
- Content: the selected layer's visual output, isolated. Rendered
  against a transparency checkerboard (standard alpha visualization)
  so the user can see what is transparent vs opaque.
- Right-side inspector shows: LAYER tab with the selected layer's
  parameters.

**CLIP tab content:**
- Isolated output of a single clip at its current playback state. Shows
  the clip with its effect stack applied but without layer compositing.
- Clip selector: a narrow strip at the top showing the clip name and
  cell position (e.g., "L1-C3: tunnel_loop"). Arrow buttons to cycle
  through clips.
- Content: the clip's visual output, including its effect stack.
- Below the clip display: clip-specific controls in a thin strip:
  - Playhead position (timeline scrubber)
  - In-point / out-point markers
  - Cuepoint markers (user-placed bookmarks per
    FEATURE_CONNECTIONS.md Scenario 9)
  - Loop toggle
- Right-side inspector shows: CLIP tab with the selected clip's
  parameters (effects, transforms, etc.).

**Content status bar:**
- Resolution: output resolution, Plex Mono 11px `--label`
  (e.g., "1920x1080")
- FPS: current render FPS, Plex Mono 11px `--value` (e.g., "60fps").
  Matches the FPS display in top chrome.
- Active indicator: text showing what is displayed:
  - OUTPUT tab: "LIVE OUTPUT"
  - PREVIEW tab: "PREVIEW: [clip/layer name]"
  - LAYER tab: "LAYER [N]: [layer name]"
  - CLIP tab: "CLIP: [clip name] @ [cell position]"
- Font: Plex Sans 11px, `--label`

### Controls + interactions

- **Tab switching:** Click a tab label. Instant switch (no animation).
  The right-side inspector updates to show the appropriate tab (COMP,
  LAYER, or CLIP) automatically when the central display tab changes.
- **Layer sub-tab switching (LAYER tab):** Click a layer label in the
  sub-tab strip. Inspector updates to that layer's parameters.
- **Clip navigation (CLIP tab):** Arrow buttons cycle through clips.
  Click the clip name to open a clip picker dropdown.
- **Clip playhead scrub (CLIP tab):** Drag the playhead position
  marker in the clip controls strip. This scrubs the clip's playback
  position for preview purposes.
- **Cuepoint interaction (CLIP tab):** Click a cuepoint marker to
  jump clip playback to that position. Cuepoints are user-placed
  bookmarks, not referenced by Hits
  (FEATURE_CONNECTIONS.md Scenario 9).
- **Resize:** The central display area resizes with the overall layout.
  In some layouts the output monitor is movable and resizable
  (AUDIO_DNA_DIRECTIVE_FULL.md Section 6: P2 layout).
- **Keyboard:** Number keys 1-4 switch tabs (1=OUTPUT, 2=PREVIEW,
  3=LAYER, 4=CLIP). Left/Right arrows navigate clips in CLIP tab.

### State variations

**Empty state (no content loaded):**
- OUTPUT tab: dark canvas, centered text "No output" in `--label`
- PREVIEW tab: centered text "Select a clip or layer to preview"
  in `--label`
- LAYER tab: layer sub-tabs visible but all layers empty. Content
  shows dark canvas per layer.
- CLIP tab: centered text "No clip selected" in `--label`
- Status bar: resolution shows composition default, FPS shows 0 or
  idle indicator

**Active state (normal performance):**
- OUTPUT tab: live output rendering, updating every frame
- PREVIEW tab: shows selected/queued content
- LAYER tab: shows isolated layer output with transparency
  checkerboard
- CLIP tab: shows clip with effect stack

**Signal-active state:**
- The output display itself shows signal-driven visuals (this is
  the visual output, not a UI state). The tab bar and status bar
  are unaffected by signal activity.
- Right-side inspector rows pulsing with signal triangles, reflecting
  what is driving the displayed content.

**Error state:**
- GPU rendering failure: content area shows "Render Error" in
  `--danger`, status bar FPS shows "ERR"
- Missing media in a clip: clip display shows the broken cell state
  (orange-tinted per FEATURE_CONNECTIONS.md Scenario 10) with the
  file path.

### Edge cases / gotchas

- The right-side inspector must stay synchronized with the central
  display tab. When the user switches from OUTPUT (COMP inspector) to
  CLIP (CLIP inspector), the inspector tab changes automatically. This
  is specified behavior: "Whatever is displayed, its parameters appear
  in the right-side inspector"
  (AUDIO_DNA_DIRECTIVE_FULL.md Section 5.14).
- LAYER tab isolation renders against a transparency checkerboard.
  This is a departure from the dark `--bg` background used elsewhere.
  The checkerboard must use subtle alternating squares
  (approximately `#1a1a1a` and `#222222`) to avoid visual noise while
  still communicating transparency.
- The CLIP tab's cuepoint markers and playhead scrubber are clip-level
  navigation tools. They do NOT affect the main composition's playhead
  or trigger Hits.
- In layouts where the output monitor is movable/resizable (P2), the
  tab bar stays attached to the output monitor and moves with it.

### Open questions

1. **Tab order:** The spec above proposes OUTPUT, PREVIEW, LAYER, CLIP
   (most-used to least-used during performance). Boris may prefer a
   different order. The directive (Section 5.14) says "output, preview,
   layer, or clip" but does not specify tab order.

2. **PREVIEW tab content when nothing is selected:** The spec above
   shows a placeholder message. An alternative: PREVIEW defaults to
   showing the next queued clip (if any), or a live preview of what
   would happen if the user triggered the currently-hovered clip cell.

3. **Multiple monitors:** In a multi-output setup, the OUTPUT tab shows
   the primary output. Should there be a sub-tab or selector for
   secondary outputs, or is that handled elsewhere (SETUP mode)?

4. **LAYER tab transparency checkerboard:** Is a checkerboard the
   right transparency indicator for the brutalist aesthetic? Alternative:
   solid dark background with a subtle "ISOLATED" label overlay.

5. **Content status bar necessity:** Is the 20px status bar redundant
   with information already shown in the top chrome (FPS, resolution)?
   It could be omitted to save vertical space for the visual output.

---

## Area 4 -- Macro / Signal Group Detail Editor

### Purpose

The Macro Detail Editor is the Bottom Focus Bar's expanded view when a
macro (signal group) is selected. It shows the macro's complete signal
processing chain -- from input sources through processing stages to
the single output signal -- and provides controls for editing the chain,
adjusting per-route parameters, and managing scope
(AUDIO_DNA_DIRECTIVE_FULL.md Sections 1.1--1.5, 1.7).

This is the "effects chain in Ableton Live" editing surface
(AUDIO_DNA_DIRECTIVE_FULL.md Section 1.1: "Modeled directly after
Ableton Live's Effect Racks").

### Trigger

- User selects a macro in any of: layer strip signal columns, inspector
  panel, Programming Mode center area, or any macro reference in the UI.
- Bottom Focus Bar shows the collapsed summary: e.g., "MACRO: BASS
  PUMP -> 4 params" (AUDIO_DNA_DIRECTIVE_FULL.md Section 1.7).
- Click to expand upward. The Macro Detail Editor fills the expanded
  Bottom Focus Bar area (default approximately 30% screen height,
  user-draggable).

### Layout

The Macro Detail Editor uses a left-to-right flow matching the signal
processing chain: sources on the left, processing in the center, output
and routing on the right. This mirrors the Programming Mode layout
(AUDIO_DNA_DIRECTIVE_FULL.md Section 1.5) but in the constrained
Bottom Focus Bar space.

```
+-----------------------------------------------------------------------------+
| MACRO HEADER BAR (24px)                                                     |
| [Macro name (editable)] [Scope: G/L/C badge] [Move/Copy scope] [Delete]    |
+=======+=================+===========+=============================+=========+
|       |                 |           |                             |         |
| INPUT |  PROCESSING     |  OUTPUT   |  ROUTING TABLE              | MACRO   |
| PANEL |  CHAIN          |  COLUMN   |                             | COLUMN  |
|       |                 |           |                             |         |
| 180px |  flex           |  80px     |  flex                       | 80px    |
|       |                 |           |                             |         |
+-------+-----------------+-----------+-----------------------------+---------+
```

**Horizontal regions (left to right, separated by 1px `--border`
vertical hairlines):**

1. **Input Panel (approximately 180px):** Signal sources feeding into
   this macro.
2. **Processing Chain (flexible width):** The chain of processing
   stages applied to the input signals.
3. **Output Column (approximately 80px):** The macro's single output
   signal, displayed as a vertical signal column.
4. **Routing Table (flexible width):** All parameters this macro's
   output is routed to.
5. **Macro Column (approximately 80px):** The macro itself displayed
   as a vertical signal column with its MIDI/OSC binding.

### Content per region

**Macro Header Bar (24px):**
- Macro name: editable text field, Plex Sans 12px `--value`
- Scope badge: G (Global), L (Layer), or C (Clip) in Plex Mono 11px.
  Badge background `--section-header`, text `--accent` for the active
  scope. This shows where this macro lives in the scope hierarchy
  (FEATURE_CONNECTIONS.md Scenario 2, Scenario 12).
- Move/Copy scope action: button that opens a scope selector dropdown
  (Global / Layer [which layer] / Clip [which clip]). Default action
  is COPY; Cmd-click for MOVE
  (FEATURE_CONNECTIONS.md Scenario 12: "Default action is COPY when
  relocating a Macro between scopes").
- Delete: functional glyph button to remove the macro. Confirmation
  dialog before deletion.

**Input Panel (SOURCES):**

Section header: SOURCES (24px, standard section header).

- List of all signal sources feeding into this macro's processing chain
- Each source is an inspector row:
  - Triangle: pulsing with the source's current live signal value
    (RAW, per MENTAL_MODELS.md Section 7)
  - Label: source signal name (e.g., BASS, FLUX, SIDECHAIN PUMP)
  - Value: current signal value, Plex Mono 11px
  - Remove button: functional glyph to disconnect this source from
    the macro

- **Add Source action:** A button at the bottom of the sources list
  that opens a signal picker (hierarchical menu showing all available
  signals organized by category, matching the Signal Drawer's
  categories). Multiple sources can be added -- the macro takes one or
  more audio sources as input (AUDIO_DNA_DIRECTIVE_FULL.md Section 1.1).

- When multiple sources are present, a **mix mode** indicator shows how
  they combine before entering the processing chain: SUM (default),
  AVERAGE, MAX, MIN. Displayed as a small label below the source list,
  clickable to cycle through modes.

**Processing Chain (center):**

Section header: PROCESSING (24px, standard section header).

The processing chain is a vertical stack of processing stages, read
top to bottom (signal flows downward through the chain). Each stage is
a self-contained block:

```
+-------------------------------------------+
| STAGE HEADER (20px)                       |
| [Reorder handle ≡] [Stage name] [Bypass] [Remove] |
+-------------------------------------------+
| Stage parameters (inspector rows)         |
| [triangle] [Smoothing    ] [0.30] [-][+] [====|====] |
| [triangle] [Threshold    ] [0.50] [-][+] [====|====] |
| [triangle] [Gain         ] [1.00] [-][+] [====|====] |
+-------------------------------------------+
```

- **Stage types** (the processing operations available):
  - **Smooth:** EMA smoothing (alpha parameter)
  - **Invert:** flips signal (1 - value)
  - **Scale:** multiply signal by a gain factor
  - **Gate:** signal passes only above a threshold
  - **Clamp:** restrict to a min/max range
  - **Map:** remap input range to output range
  - **LFO modulation:** modulate the signal with an internal LFO (rate,
    depth, waveform parameters)

- Each stage has:
  - Reorder handle: functional glyph (three horizontal lines) for
    drag-to-reorder within the chain
  - Stage name: Plex Sans 11px `--value`
  - Bypass toggle: dims the stage (signal passes through unmodified).
    Bypassed stages show reduced opacity (approximately 40%)
  - Remove button: functional glyph to delete the stage
  - Parameter rows: standard inspector row grammar. Each parameter has
    a triangle (for routing a signal to the processing parameter
    itself -- signal-as-automation of macros, see
    AUDIO_DNA_DIRECTIVE_FULL.md Section 2.1)

- Between stages, a thin connecting line (1px `--accent` when active,
  1px `--border` when signal is zero) shows signal flow.

- **Add Stage action:** A button at the bottom of the chain that opens
  a stage type picker (list of available processing operations). New
  stages are appended at the bottom. User can reorder after adding.

- The chain scrolls vertically if it exceeds the available height.

**Output Column (right of chain):**

- A single vertical signal column (60px wide per `--signal-column-width`
  in MOCKUP_BRIEF.md Section 2.3) showing the macro's processed output
  signal.
- Background fill: `--accent` fill from bottom, brightness proportional
  to output signal value (SMOOTHED, EMA alpha 0.3 per
  MENTAL_MODELS.md Section 7).
- User value tick: the output level/gain control for the macro.
- MIDI/OSC binding label at the bottom.
- Label above the column: "OUTPUT" in Plex Sans 9px `--label`.

**Routing Table (right of output):**

Section header: ROUTES (24px, standard section header).

- List of all parameters this macro's output is routed to
- Each route is an inspector row:
  - Triangle: pulsing with the output signal (shows the modulation
    reaching the destination)
  - Label: destination parameter name + scope context
    (e.g., "L1 opacity", "Global blur")
  - Depth slider: per-route depth control, 0-100%
    (FEATURE_CONNECTIONS.md Scenario 8). This is a standard inspector
    row slider.
  - AUTO/OVERRIDE indicator: orange `--override` dot appears if the
    depth is currently in OVERRIDE (VJ manually holds the depth value).
    Per the universal field-level rule, depth is a FIELD and is
    independently AUTO/OVERRIDE-able
    (FEATURE_CONNECTIONS.md Scenario 8).
  - Remove button: functional glyph to disconnect this route

- Routes grouped by scope (Global routes first, then Layer, then Clip)
  using section sub-headers. Follows mixing-desk hierarchy.

- **Add Route action:** A button at the bottom that opens a parameter
  picker (hierarchical: Global > Layers > Clips > Parameters). Shows
  only parameters at the macro's scope or narrower (a Global macro can
  route to anything; a Clip macro can only route to its own clip's
  parameters per AUDIO_DNA_DIRECTIVE_FULL.md Section 1.2).

**Macro Column (rightmost):**

- The macro itself displayed as a vertical signal column. This is the
  same column that appears in the layer strip or dashboard.
- Shows the macro's overall output level + MIDI/OSC binding.
- Draggable tick for the macro's master output level.
- Label above: macro name in Plex Sans 9px `--label`.
- This column mirrors what the user sees in the layer strip, providing
  visual confirmation that "this is the same macro I clicked."

### Controls + interactions

- **Source management:** Add sources via signal picker. Remove via
  per-row remove button. Mix mode cycling via label click.
- **Chain editing:** Add stages via stage type picker. Remove/bypass
  per-stage buttons. Reorder via drag handles. Parameter editing via
  standard inspector row controls.
- **Route management:** Add routes via parameter picker. Remove via
  per-row button. Depth adjustment via per-route inspector row slider.
- **Scope transfer:** Click the scope badge or Move/Copy button in the
  header. Select target scope from dropdown. Default is COPY. Cmd-click
  for MOVE. Scope narrowing triggers the warning dialog
  (FEATURE_CONNECTIONS.md Scenario 12: "Warning dialog: 'Moving BASS
  PUMP to Clip scope will lose these routings...'").
- **Chain reorder:** Drag a stage's reorder handle (three horizontal
  lines) vertically to move it within the chain. The connecting lines
  redraw to reflect the new order.
- **Signal-as-automation:** Click the triangle on a processing stage's
  parameter row to route a signal to that parameter. This allows a
  signal to modulate the processing chain itself -- e.g., an LFO
  modulating the smoothing alpha of a SMOOTH stage
  (AUDIO_DNA_DIRECTIVE_FULL.md Section 2.1: "A signal connected to a
  macro can drive the macro's own parameters like an automation lane").
- **Keyboard:** Tab navigates between regions (Sources, Chain, Routes).
  Arrow keys navigate rows within a region. Enter toggles bypass on
  a chain stage.

### State variations

**Empty state (new macro, no chain configured):**
- Sources panel: "No sources. Click + to add." in `--label` dim
- Processing chain: empty area with centered "Add first processing
  stage" call-to-action in `--label`
- Output column: shows zero fill (no signal)
- Routes panel: "No routes. Click + to add." in `--label`
- Macro column: shows zero fill

**Populated state (normal):**
- Sources show live signal values (mini bars or triangle pulsing)
- Chain stages show their parameters, connecting lines between stages
  pulse with signal flow
- Output column fills with the processed output signal
- Routes show destinations with depth sliders

**Signal-active state:**
- All triangles in source rows pulse with their respective signals
  (RAW per MENTAL_MODELS.md Section 7)
- Connecting lines between chain stages pulse with signal flow
  (using `--accent` at proportional opacity)
- Output and macro columns fill with smoothed signal (EMA per
  MENTAL_MODELS.md Section 7)
- Route triangles pulse showing modulation reaching destinations

**OVERRIDE state:**
- Depth sliders in the Routes panel show orange `--override` dot when
  the VJ has manually held a depth value
- Processing parameter values show orange dot when in OVERRIDE
  (VJ manually set a processing parameter)
- Header scope badge shows orange tint if the macro's scope is
  affected by overrides in its routes

**Error state:**
- Source references a signal that no longer exists (e.g., custom signal
  deleted): source row shows signal name in `--danger` with "Missing"
  label. Chain continues processing with remaining valid sources.
- Route references a parameter on a clip that has been removed: route
  row shows "Invalid: [param name]" in `--danger`.

**Scope conflict state (per FEATURE_CONNECTIONS.md Scenario 2):**
- If this macro routes to a parameter that is also routed by a
  higher-scope macro, the route row shows a dim "(SILENCED)" label in
  `--label`. The macro's route exists but is not active because a
  higher scope wins (Global > Layer > Clip, exclusive precedence per
  MENTAL_MODELS.md Section 2).

### Edge cases / gotchas

- A macro can have multiple input sources that combine before entering
  the processing chain (AUDIO_DNA_DIRECTIVE_FULL.md Section 1.1:
  "Multiple inputs can be mixed, chained, and wired together through
  processing to create a single output"). The mix mode (SUM/AVG/MAX/MIN)
  determines how they combine. The editor must show this clearly.
- Processing chain stages can themselves have signal-routed parameters
  (signal-as-automation). This creates a recursive visual: triangles
  pulsing on rows inside the processing chain. The editor must handle
  this without visual confusion -- the "signal driving processing"
  triangles should be visually identical to triangles elsewhere (same
  grammar, same behavior).
- Scope transfer with Cmd-drag (MOVE) physically removes the macro
  from its original location. If the macro is referenced by Hits at
  the original scope, those Hit references become invalid. The warning
  dialog should mention this.
- The Processing Chain section is the most novel UI area -- there is
  no direct Resolume equivalent. Following the Resolume Grammar +
  Audio-DNA Soul model (MENTAL_MODELS.md Section 8): the chain layout
  should use familiar inspector row grammar for each stage's parameters,
  with the "audio-DNA soul" being the live signal flow visualization
  between stages.
- A macro with zero processing stages is valid -- signal passes through
  unmodified from sources to output. This is a simple "pass-through"
  macro used when the user wants to route a raw signal to multiple
  parameters without any processing.

### Open questions

1. **Source mix mode UI:** The spec proposes a clickable label
   (SUM/AVG/MAX/MIN) below the sources list. Is this the right
   interaction, or should each source have its own mix weight slider
   (more Ableton-like), with the combination happening as a weighted
   sum?

2. **Processing stage library:** The spec lists 7 stage types (Smooth,
   Invert, Scale, Gate, Clamp, Map, LFO). Is this the canonical list
   for V1? Are there additional processing types Boris envisions?

3. **Chain flow direction:** The spec proposes vertical top-to-bottom
   flow for the processing chain (natural reading order, fits the
   vertical Bottom Focus Bar). Alternative: horizontal left-to-right
   flow, matching the overall left-to-right source-to-output layout.
   Vertical may be better for the constrained height of the Bottom
   Focus Bar.

4. **Macro column vs Output column:** The spec includes both an Output
   column (showing the processed signal) and a Macro column (showing
   the same macro as it appears in the layer strip). Is the Macro
   column redundant, or does it serve a necessary "this is the same
   thing you see in the layer strip" confirmation?

5. **Signal-as-automation feedback loop:** The directive mentions that
   signal-to-macro modulation can flow "one-way (signal to macro, done)
   or loop (cyclical / feedback)"
   (AUDIO_DNA_DIRECTIVE_FULL.md Section 2.1). How is a feedback loop
   visualized in the chain editor? A circular connecting line? A
   special indicator on the looping stage? And what prevents
   infinite-gain runaway in a feedback loop?

6. **Processing chain reorder interaction:** The spec proposes drag
   handles for reordering. Given the "no drag-and-drop" decision for
   the Hit timeline (FEATURE_CONNECTIONS.md Section 11.9), does the
   no-drag-and-drop rule apply only to the timeline, or is it a
   broader UI principle? The chain editor is not a timeline -- it is
   a stack of processing stages where reorder is a primary interaction.

---

*End of UI Detail Specs. All four areas are draft-pending Boris review.
Open questions in each section must be resolved before dispatching
mockup Builders for mockups #5--#8 in the MOCKUP_BRIEF V1 plan.*
