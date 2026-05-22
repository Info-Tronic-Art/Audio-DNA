# Audio-DNA Directive Reconciliation — Implementation Handoff

> Written 2026-05-21. Source of truth for integrating AUDIO_DNA_DIRECTIVE_FULL.md
> into the RealTimeAudio codebase. Self-contained — read this + the directive to execute.
>
> **Companion document:** DESIGN_PROFILE.md (same directory) — Boris's design
> sensibility, aesthetic principles, decision framework, and "perfect" checklist.
> Every builder MUST read DESIGN_PROFILE.md before implementing visual changes.

> **Updated 2026-05-22.** Some decisions have been further refined since
> this handoff was written. The CANONICAL sources for behavioral spec are
> now:
> - `../../FEATURE_CONNECTIONS.md` — behavioral reference with all 12
>   connection-gap scenarios resolved (worked examples, precedence rules,
>   state machine)
> - `../../MENTAL_MODELS.md` — conceptual spine (three intent layers, scopes,
>   AUTO/OVERRIDE state, etc.)
>
> This HANDOFF doc remains the IMPLEMENTATION PLAN (phases, file matrices,
> builder dispatches). Where it conflicts with the canonical docs above,
> the canonical docs win — except where Phase work has already started or
> implementation specifics are needed.
>
> Key refinements since 2026-05-21:
> - "Reclaim button" → renamed to "AUTO" (with orange `#ff4500` indicator)
> - "P. button on section headers" → REMOVED entirely. Triangle-only routing.
> - "Click occupied Hit slot = merge" → REVISED to "confirm dialog before
>   replace" (no merge)
> - The universal AUTO/OVERRIDE state machine is now field-level (not
>   layer-level or scope-level)

---

## 1. OVERVIEW

**What:** AUDIO_DNA_DIRECTIVE_FULL.md (1,052 lines, 10 parts) defines Boris's complete
design vision for the Audio-DNA VJ application. It supersedes all prior design decisions.

**Codebase state:** 58.2K LOC, 206 files, 25 implementation phases complete. Core engine
fully built (42+ audio features, 135 effects, 108 sources, clip/layer/deck compositing,
signal routing, MIDI/OSC, recording). UI needs overhaul to match directive.

**Scope:** 14 phases, ~15-18 sessions, 30+ builder dispatches. Phases 0-4 are visual
foundation. Phases 5-7 are infrastructure. Phases 8-11 are feature completion.

**Constraints from handoff doc (CLAUDE_CODE_RTA_HANDOFF.md):**
- Do NOT modify mockup HTML files in `design/mockups/html/` (historical references)
- Directive is IMMUTABLE — raise issues with Boris, don't modify
- When ambiguous, ASK
- Focused commits, one logical change per commit, reference directive section number

---

## 2. CONFLICT INVENTORY

### 2.1 Color Conflicts (CRITICAL — 407 references across all UI files)

| Current Constant | Current Value | Directive Variable | Target Value | Status |
|-----------------|---------------|-------------------|-------------|--------|
| `kBackground` | `0xff1a1a2e` (navy) | `--bg` | `0xff1a1a1a` | WRONG |
| `kSurface` | `0xff252540` (purple) | `--panel` | `0xff2a2a2a` | WRONG |
| `kSurfaceLight` | `0xff30305a` (purple) | (hover state) | `0xff333333` | WRONG |
| `kAccentCyan` | `0xff00e5ff` | `--accent` | `0xff00d9ff` | WRONG |
| `kAccentMagenta` | `0xffff00e5` | (none) | DELETE | FORBIDDEN |
| `kTextPrimary` | `0xffe0e0e0` | `--value` | `0xffe0e0e0` | CORRECT |
| `kTextSecondary` | `0xff808090` (blue) | `--label` | `0xff888888` | WRONG |
| `kMeterGreen` | `0xff00e676` | (none) | DELETE | FORBIDDEN |
| `kMeterYellow` | `0xffffea00` | (none) | DELETE | FORBIDDEN |
| `kMeterRed` | `0xffff1744` | `--danger` | `0xffff4040` | WRONG |
| `kPanelBorder` | `0xff3a3a5c` (purple) | `--border` | `0xff3a3a3a` | WRONG |
| (missing) | — | `--section-header` | `0xff252525` | ADD |
| (missing) | — | `--separator` | `0xff383838` | ADD |

Additional hardcoded colors found:
- `SignalStrip.cpp:333` — mint `0xff64ffda` (RETIRED per directive)
- `TimingWindow.cpp` — tab colors `0xff3a3a5c`, `0xff1a1a2e` (need update)
- `DeckView/ClipCell` — active border `0xff4a9a8a` (muted teal) → use kAccent
- `LayerStrip` — bypass active `0xff6a3a3a` (reddish) → use kDanger or subtle kBg variant
- `LayerStrip` — solo active `0xff7a7a4a` (yellowish) → use kAccent (active state = blue)
- Various `0xff??????` literals scattered across UI files (~20+ locations)

Component dimension mismatches (current vs directive):
- ClipCell: 90x96px → 100x80px (directive 5.9)
- TopBar: 34px → 98px total (44+32+22, directive 5.1)
- SignalBar minimized: 26px → 22px (directive 5.1)
- BPM font: 18pt → 48px (directive 4.2)
- UniversalParamControl label: 72px → 90px, value: 36px → 50px (directive 5.6)
- LayerStrip: 250px → 250px + signal column area (directive 5.10)
- MacroPanel: 8 rotary Knobs at 110px height → 8 VerticalSignalColumns

### 2.2 Control Conflicts (CRITICAL)

**Circular Rotary Knobs — DIRECTIVE: ZERO IN ENTIRE APP**

| File | Line | What | Replace With |
|------|------|------|-------------|
| `src/ui/Knob.h` | 5-47 | Knob class declaration | VerticalSignalColumn |
| `src/ui/Knob.cpp` | 7 | `RotaryVerticalDrag` style | Vertical meter+slider |
| `src/ui/MacroPanel.cpp` | 9-21 | 8x Knob instantiation | 8x VerticalSignalColumn |
| `src/ui/LookAndFeel.cpp` | 92-147 | `drawRotarySlider()` | Remove entirely |
| `src/ui/LookAndFeel.cpp` | 19-20 | Rotary slider color setup | Remove |

### 2.3 Rounded Corners (CRITICAL — 56 occurrences, directive: ZERO everywhere)

| File | Lines | Count |
|------|-------|-------|
| `MainComponent.cpp` | 1195, 1207, 1212 | 3 |
| `SignalStrip.cpp` | 92, 127, 131, 137, 163, 172, 208, 210, 239, 244 | 10 |
| `LookAndFeel.cpp` | 173, 178, 205, 210, 239, 244, 264, 323, 327, 454, 463 | 11 |
| `AudioReadoutPanel.cpp` | 86, 90, 211, 215, 240, 254, 295, 299, 318, 323, 357, 376, 382, 387 | 14 |
| `SpectrumDisplay.cpp` | 55, 59, 86, 101, 106 | 5 |
| `FilesBrowser.cpp` | 136, 139 | 2 |
| `SourcesBrowser.cpp` | 55, 220, 223 | 3 |
| `MilkDropBrowser.cpp` | 88, 105 | 2 |
| `MappingEditor.cpp` | 208, 211 | 2 |
| `WaveformDisplay.cpp` | 67, 71 | 2 |
| `FXBrowser.cpp` | 251, 254 | 2 |

**Action:** Replace ALL `fillRoundedRectangle(bounds, radius)` with `fillRect(bounds)`.
Replace ALL `drawRoundedRectangle(bounds, radius, thickness)` with `drawRect(bounds, thickness)`.
Scrollbar thumb corner radius → 0 (LookAndFeel.cpp drawScrollbar).

### 2.4 Naming Conflict (DECISION NEEDED)

"Hit" is used for audio onset detection signal (`onsetDetected -> Hit`) AND the
directive's show-level timed cue system. Recommendation: rename audio signal UI label
from "Hit" to "Onset" (internal code already uses `onsetDetected`).

### 2.5 Typography (NO custom fonts exist)

Current: System default sans-serif via `juce::Font(juce::FontOptions(size))`.
Directive: IBM Plex Mono + IBM Plex Sans as explicit font stack.
**Action:** Embed IBM Plex Mono/Sans as JUCE binary resources, load in LookAndFeel.

### 2.6 Aligned Items (no changes needed)

- Signal triangles in UniversalParamControl.cpp (right-pointing, cyan/gray states)
- MacroBank scope enum (Global, Layer, Clip) — structure correct
- Cuepoint terminology (clip-internal markers) — correct per directive
- Session Recorder — foundation for directive's data recording
- Clip/Layer/Column/Deck architecture — Resolume-style grid correct
- Audio analysis (42+ features) — core differentiator intact

---

## CRITICAL FINDING: Programming Studio Mockup Is the Blueprint

**v9_ten_02_programming_studio.html** contains the EXACT CSS variables and dimensions
that became the directive. This mockup IS the implementation blueprint:

```css
/* FROM v9_ten_02 — these became the directive's design system */
--bg: #1a1a1a;        /* directive Part 4.1 --bg */
--panel: #2a2a2a;     /* directive Part 4.1 --panel */
--panel-alt: #252525; /* directive Part 4.1 --section-header */
--row-sep: #383838;   /* directive Part 4.1 --separator */
--hairline: #3a3a3a;  /* directive Part 4.1 --border */
--label: #888;        /* directive Part 4.1 --label */
--value: #e0e0e0;     /* directive Part 4.1 --value */
--mint: #5fdba7;      /* → REPLACED by directive to #00d9ff */
--red: #ff4040;       /* directive Part 4.1 --danger */
```

The directive's 98px top chrome also comes from this mockup:
- Top bar: 44px (matches directive)
- Transport strip: 32px (matches directive)
- Signal bar: 22px (matches directive)
- BPM: 48px font, 300 weight (matches directive)

**Implementation rule:** Use v9_ten_02's layout structure as the reference
implementation. Replace mint (#5fdba7) with cyan (#00d9ff). The other
mockups (H1, P1, etc.) predate the directive's final decisions and use
different accents (orange, older layouts) — those inform LAYOUT but not
COLOR or DIMENSIONS.

Key dimension specs from the mockups:
- Param row label: 82px (v9_ten_02) — directive rounds up to 90px
- Inspector section accent bar: 3px left edge (v9_ten_02)
- Signal anchors: 12px circle with 3px padding = 18px hit target
- Patch bay routes: gray 1px default, accent 2px selected
- Clip wave strip: 4px left edge
- Signal bar: key-value pairs with pipe separators, 9px labels, 10px values
- Effect rows: bypass 16x16, name 12px sans, delete 16x16
- Beat animation: `@keyframes beat` opacity 1→0.55 at 128bpm (0.468s)
- Just-changed flash: `@keyframes justchanged` accent bg→transparent over 0.2s

---

## 3. COLOR MAPPING TABLE

### 3.1 Design System Constants (target for LookAndFeel.h)

```cpp
// Audio-DNA Design System — per AUDIO_DNA_DIRECTIVE_FULL.md Part 4.1
static constexpr juce::uint32 kBg            = 0xff1a1a1a;  // --bg (Resolume-exact)
static constexpr juce::uint32 kPanel          = 0xff2a2a2a;  // --panel
static constexpr juce::uint32 kSectionHeader  = 0xff252525;  // --section-header
static constexpr juce::uint32 kSeparator      = 0xff383838;  // --separator
static constexpr juce::uint32 kBorder         = 0xff3a3a3a;  // --border (1px everywhere)
static constexpr juce::uint32 kLabel          = 0xff888888;  // --label (dim)
static constexpr juce::uint32 kValue          = 0xffe0e0e0;  // --value (bright)
static constexpr juce::uint32 kAccent         = 0xff00d9ff;  // --accent (THE ONLY accent)
static constexpr juce::uint32 kDanger         = 0xffff4040;  // --danger (REC + errors only)

// Derived (not in directive CSS but needed for JUCE)
static constexpr juce::uint32 kPanelHover     = 0xff333333;  // subtle hover/active
static constexpr juce::uint32 kAccentDim      = 0x4000d9ff;  // accent at 25% for backgrounds
```

### 3.2 Migration — Old Name → New Name

```cpp
// DEPRECATED ALIASES (Phase 1a — keep temporarily, remove in Phase 1c)
static constexpr auto kBackground    = kBg;
static constexpr auto kSurface       = kPanel;
static constexpr auto kSurfaceLight  = kPanelHover;
static constexpr auto kAccentCyan    = kAccent;
static constexpr auto kTextPrimary   = kValue;
static constexpr auto kTextSecondary = kLabel;
static constexpr auto kPanelBorder   = kBorder;

// DELETED (Phase 1c — references must be removed first)
// kAccentMagenta — one accent only, no rainbow UI
// kMeterGreen    — use kAccent for all meter fills
// kMeterYellow   — use kAccent for all meter fills
// kMeterRed      — replaced by kDanger (REC/errors only)
```

### 3.3 Accent Color Usage Rules (from directive 4.1)

**Blue `#00d9ff` ON:** Playing clip fill+border, BPM number (48px), structural state pill,
beat wheel segment, transport play when active, selected row highlight, just-changed flash,
slider position indicator, active triangle fill, section header left-edge bar (active routing),
patch-bay selected route, signal column fills, active triangles, active Hit markers.

**Blue `#00d9ff` NEVER:** Static labels, inactive buttons, background fills, decorative
elements, unselected rows.

**Meter color migration:** All signal meters that used green/yellow/red category colors
should use kAccent (blue) only. Signal strength = brightness of blue, not color variation.

---

## 4. FONT SPECIFICATION

### 4.1 Font Stack (from directive 4.2)

```
--font-mono: 'IBM Plex Mono', 'SF Mono', Menlo, monospace;
--font-sans: 'IBM Plex Sans', -apple-system, sans-serif;
```

### 4.2 JUCE Implementation

1. Download IBM Plex Mono (Regular 400, Medium 500) and IBM Plex Sans (Regular 400)
   from Google Fonts or IBM's GitHub
2. Add .ttf files to `resources/fonts/` directory
3. Add to CMakeLists.txt as `juce_add_binary_data(FontData SOURCES resources/fonts/*.ttf)`
4. Load in LookAndFeel constructor:
```cpp
auto monoTypeface = juce::Typeface::createSystemTypefaceFor(
    FontData::IBMPlexMono_Regular_ttf, FontData::IBMPlexMono_Regular_ttfSize);
auto sansTypeface = juce::Typeface::createSystemTypefaceFor(
    FontData::IBMPlexSans_Regular_ttf, FontData::IBMPlexSans_Regular_ttfSize);
```

### 4.3 Size Hierarchy (from directive 4.2)

| Element | Size | Weight | Font | Color |
|---------|------|--------|------|-------|
| BPM (hero number) | 48px | 500 | Mono | kAccent |
| Genre / structural state | 20-24px | 400 | Sans | — |
| Section headers | 12px | 400 | Sans, uppercase-first | kValue |
| Row labels | 11px | 400 | Sans | kLabel |
| Row values | 11px | 400 | Mono | kValue |
| Clip names | 9px | 400 | Sans | — |
| Sparkline labels | 10px min | 400 | Sans | — |
| Signal quick-ref bar | 11px | 400 | Mono | — |

### 4.4 Typography Rules

- ALL-CAPS for section headers and mode labels (LIVE, PROGRAM, SETUP)
- Monospace ONLY for numbers — never for text labels
- Letter-spacing on all-caps: 0.5-1px (use juce::Font::setExtraKerningFactor)
- No italic, no underline (except mode toggle active-state blue underline)

---

## 5. COMPONENT CHANGE MAP

### 5.1 Components to MODIFY

| Component | Files | Changes | Phase |
|-----------|-------|---------|-------|
| **LookAndFeel** | LookAndFeel.h/cpp | New color constants, remove rotary slider, remove rounded corners, add font loading, update all drawing methods | 0, 1 |
| **TopBar** | TopBar.h/cpp | Restructure to 3-strip (44+32+22=98px), BPM at 48px, transport strip, mode buttons | 4 |
| **SignalBar** | SignalBar.h/cpp | 22px collapsed (not 26), overlay behavior (not replace), 30Hz update stays | 4, 7 |
| **SignalStrip** | SignalStrip.h/cpp | Remove category coloring (all blue), remove rounded corners, fix colors | 1 |
| **UniversalParamControl** | UniversalParamControl.h/cpp | Label→90px, Value→50px, slider→14px height, update colors. (Note: P. button REMOVED 2026-05-22 — see header note + Section 10 Q3.) | 3 |
| **MacroPanel** | MacroPanel.h/cpp | Replace 8 Knobs with 8 VerticalSignalColumns | 2 |
| **InspectorPanel** | InspectorPanel.h/cpp | Tab styling (blue text+underline active), update colors | 1, 3 |
| **ClipInspector** | ClipInspector.h/cpp | Section headers to directive spec (12px, uppercase-first, 2px blue left bar). (P. on right REMOVED 2026-05-22.) | 3 |
| **LayerInspector** | LayerInspector.h/cpp | Same section header update | 3 |
| **CompositionInspector** | CompositionInspector.h/cpp | Same section header update | 3 |
| **DeckView** | DeckView.h/cpp | Update colors, deck tab styling | 1 |
| **LayerStrip** | LayerStrip.h/cpp | Add signal column area (left side, auto-populated, horizontal scroll) | 8 |
| **ClipCell** | ClipCell.h/cpp | Update to directive 5.9 spec (100x80 base, 6px waveform, 14px name bar, states) | 14 |
| **WaveformDisplay** | WaveformDisplay.h/cpp | Add display mode selector, Hit lane, click interaction | 11 |
| **MainComponent** | MainComponent.h/cpp | Extract layout to LayoutManager, update resized(), fix colors | 1, 6 |
| **BrowserPanel** | BrowserPanel.h/cpp | Tab update, colors | 1 |
| **FilesBrowser** | FilesBrowser.h/cpp | Remove rounded corners, fix colors | 1 |
| **SourcesBrowser** | SourcesBrowser.h/cpp | Remove rounded corners, fix colors | 1 |
| **FXBrowser** | FXBrowser.h/cpp | Remove rounded corners, fix colors | 1 |
| **MilkDropBrowser** | MilkDropBrowser.h/cpp | Remove rounded corners, fix colors | 1 |
| **MappingEditor** | MappingEditor.h/cpp | Remove rounded corners, fix colors | 1 |
| **SpectrumDisplay** | SpectrumDisplay.h/cpp | Remove rounded corners, fix colors | 1 |
| **AudioReadoutPanel** | AudioReadoutPanel.h/cpp | Remove rounded corners, fix colors | 1 |
| **ProgrammingMode** | ProgrammingMode.h/cpp | Replace with LIVE/PROGRAM/SETUP mode component | 5 |
| **TimingWindow** | TimingWindow.h/cpp | Complete rebuild for Hit system | 10 |
| **MenuBarModel** | MenuBarModel.h/cpp | Add layout template hotkeys, mode switching | 5, 6 |
| **PreferencesDialog** | PreferencesDialog.h/cpp | Fix colors | 1 |
| **BindingOverlay** | BindingOverlay.h/cpp | Fix colors | 1 |
| **MidiLearnOverlay** | MidiLearnOverlay.h/cpp | Fix colors | 1 |

### 5.2 Components to CREATE (new files)

| Component | Files | Purpose | Phase |
|-----------|-------|---------|-------|
| **VerticalSignalColumn** | VerticalSignalColumn.h/cpp | Replaces all Knobs: dual-purpose signal meter + control slider | 2 |
| **LayoutManager** | LayoutManager.h/cpp | Layout template save/load/apply/hotkeys | 6 |
| **LayoutTemplate** | (in LayoutManager.h) | Data model for panel positions | 6 |
| **ModeManager** | ModeManager.h/cpp | LIVE/PROGRAM/SETUP switching + per-mode state | 5 |
| **BottomFocusBar** | BottomFocusBar.h/cpp | Expandable bottom panel, context-sensitive | 7 |
| **SignalDrawer** | (in SignalBar or new) | Full-workspace overlay for signal editing | 7 |
| **Hit** | src/model/Hit.h/cpp | Hit data model + serialization | 9 |
| **HitGroup** | src/model/HitGroup.h/cpp | Hit group/sequence data model | 9 |
| **HitEnvelope** | src/model/HitEnvelope.h/cpp | Envelope system (onset/curve/release) | 9 |
| **HitTimeline** | src/ui/HitTimeline.h/cpp | Timeline visualization with pills | 10 |
| **HitPill** | src/ui/HitPill.h/cpp | Individual Hit pill renderer | 10 |
| **HitInspector** | src/ui/HitInspector.h/cpp | Hit detail view for Bottom Focus Bar | 10 |
| **HitCaptureDialog** | src/ui/HitCaptureDialog.h/cpp | Capture + filter workflow | 10 |

### 5.3 Components to DELETE

| Component | Files | Reason |
|-----------|-------|--------|
| **Knob** | Knob.h/cpp | Replaced by VerticalSignalColumn (directive: zero circular knobs) |

---

## 6. PHASE PLAN

### Phase 0: Font + Design Token Foundation
**Sessions:** 1
**Department:** engineering
**Skill:** feature-build
**Builders:** 1

**Tasks:**
1. Download IBM Plex Mono (Regular, Medium) + IBM Plex Sans (Regular) .ttf files
2. Add to `resources/fonts/` directory
3. Update CMakeLists.txt: add `juce_add_binary_data(FontData ...)`
4. Update LookAndFeel.h: add new color constants per Section 3 above
5. Update LookAndFeel.cpp constructor: load fonts via createSystemTypefaceFor
6. Add helper methods: `getMonoFont(float size)`, `getSansFont(float size)`
7. Keep old constant names as deprecated aliases (for compilation)
8. Verify build compiles with new fonts

**Files modified:** LookAndFeel.h, LookAndFeel.cpp, CMakeLists.txt
**Files created:** resources/fonts/IBMPlexMono-Regular.ttf, IBMPlexMono-Medium.ttf, IBMPlexSans-Regular.ttf
**Success criteria:** App compiles. Fonts load. New constants defined. Old aliases work.

---

### Phase 1: Color + Corners Purge
**Sessions:** 1
**Department:** engineering
**Skill:** feature-build
**Builders:** 4 (parallel)

**Strategy:** Phase 0 added aliases so old names still compile. Now 4 parallel builders
update all references to use new names AND fix visual issues (rounded corners, hardcoded colors).

**Builder A — Top Chrome + Analysis (8 files):**
- TopBar.h/cpp
- SignalBar.h/cpp
- SignalStrip.h/cpp
- SignalInspector.h/cpp
- Tasks: Replace old color names with new. Remove all fillRoundedRectangle.
  Remove category-specific signal colors (all meters use kAccent blue only).
  Fix mint color at SignalStrip.cpp:333. Update font calls to use getMonoFont/getSansFont.

**Builder B — Inspector + Controls (8 files):**
- InspectorPanel.h/cpp
- ClipInspector.h/cpp
- LayerInspector.h/cpp
- CompositionInspector.h/cpp
- Tasks: Replace old color names. Fix tab styling (blue text + 2px underline active).
  Update all paint() methods. Remove hardcoded 0xff?????? colors.

**Builder C — Browsers + Misc (14 files):**
- BrowserPanel.h/cpp
- FilesBrowser.h/cpp
- FXBrowser.h/cpp
- SourcesBrowser.h/cpp
- MilkDropBrowser.h/cpp
- CompDecksBrowser.h/cpp
- RecordPanel.h/cpp
- Tasks: Replace old color names. Remove all fillRoundedRectangle.
  Update font calls. Fix all hardcoded colors.

**Builder D — Remaining files (16 files):**
- MainComponent.h/cpp (input meter: lines 1195, 1207, 1212)
- DeckView.h/cpp
- LayerStrip.h/cpp
- ClipCell.h/cpp
- MappingEditor.h/cpp
- SpectrumDisplay.h/cpp (5 rounded rects)
- AudioReadoutPanel.h/cpp (14 rounded rects)
- WaveformDisplay.h/cpp
- EffectStackView.h/cpp
- PreferencesDialog.h/cpp
- BindingOverlay.h/cpp
- MidiLearnOverlay.h/cpp
- EffectsRackPanel.h/cpp
- OutputWindow.h/cpp
- PresetManager.h/cpp
- MenuBarModel.h/cpp
- Tasks: Replace old color names. Remove all fillRoundedRectangle.
  Remove references to kAccentMagenta (Knob mapping ring — remove entirely,
  or replace with kAccent). Update fonts.

**2026-05-22 update:** The kAccentMagenta deletion stands, but `#ff4500`
(orange) is RE-INTRODUCED as the new `--override` color token. Add a new
constant:

```cpp
static constexpr juce::uint32 kOverride = 0xffff4500;  // --override (OVERRIDE state indicator only)
```

This is the only second-accent color in the design system. Use ONLY for OVERRIDE state visualization (top-chrome STATUS indicator + group-level + per-field orange dot). See `../../FEATURE_CONNECTIONS.md` Scenario 1.

**After all 4 builders complete:**
- Remove deprecated aliases from LookAndFeel.h (Phase 1c cleanup)
- Delete kAccentMagenta, kMeterGreen, kMeterYellow constants
- Verify build compiles with NO old constant names

**Success criteria:** App compiles. Zero fillRoundedRectangle calls in codebase.
Zero references to old color names. All UI uses directive color palette. All meters blue only.

**Verification:** Spawn design-review agent to screenshot and audit visual consistency.

---

### Phase 2: Knob Replacement
**Sessions:** 1
**Department:** engineering
**Skill:** feature-build
**Builders:** 2 (sequential — Builder A creates component, Builder B uses it)

**Builder A — Create VerticalSignalColumn:**
- Create `src/ui/VerticalSignalColumn.h` and `src/ui/VerticalSignalColumn.cpp`
- Based on SignalStrip's meter rendering but dual-purpose:
  - Background: dark fill
  - Signal layer: blue `kAccent` fill from bottom up (0→1), brightness = signal value
  - Control layer: horizontal tick/line user drags up/down for parameter value
  - MIDI/OSC indicator: small text at top or bottom showing binding
  - Label: parameter name at bottom
  - Value: numeric readout
- Interface: `setValue(float)`, `setSignalValue(float)`, `setLabel(string)`,
  `setMidiBinding(string)`, `onValueChange` callback
- Width: ~24-32px, height: variable (fills available space)
- Styling: square corners, kBorder 1px, kBg background, kAccent fill

**Builder B — Replace all Knob instantiations:**
- MacroPanel.h/cpp: Replace 8x `std::unique_ptr<Knob>` with `std::unique_ptr<VerticalSignalColumn>`
- EffectsRackPanel.h/cpp: Replace any Knob usage
- LookAndFeel.cpp: Remove `drawRotarySlider()` method entirely
- LookAndFeel.cpp: Remove rotary slider color setup (lines 19-20)
- Delete Knob.h and Knob.cpp
- Update CMakeLists.txt: remove Knob files, add VerticalSignalColumn files

**Success criteria:** Zero Knob class references. All macro controls are vertical columns.
drawRotarySlider removed. App compiles and runs with new controls.

---

### Phase 3: Inspector Grammar
**Sessions:** 1
**Department:** engineering
**Skill:** feature-build
**Builders:** 2 (parallel)

**Builder A — UniversalParamControl overhaul:**
- UniversalParamControl.h/cpp
- Update dimensions:
  - Triangle: keep at 14px area (8px triangle)
  - Label column: 72px → 90px, right-aligned, Sans 11px, kLabel
  - Value column: 36px → 50px, Mono 11px, kValue
  - Minus/Plus buttons: keep 20x20px
  - Slider: remaining width, height → 14px (not full row height)
- **P. button REMOVED 2026-05-22.** No P. button on inspector rows. Triangle is the sole per-parameter routing affordance. Row width recovers ~20px of horizontal space previously allocated to P. button.
- Slider track: dark with blue position indicator line (vertical hairline, not thumb)
- Row height: 24px per inspector row

**Builder B — Section Headers:**
- ClipInspector.h/cpp, LayerInspector.h/cpp, CompositionInspector.h/cpp
- Update section headers to directive spec (Section 5.7):
  - V triangle: 7px clickable disclosure
  - Name: Sans 12px, kValue, uppercase first letter only
  - Height: 24px, kSectionHeader background
  - 2px kAccent left-edge bar when section has active routing

**Section header grammar 2026-05-22:**

```
▼ Section Name
```

(No P. button. Keep the 2px kAccent left-edge bar when section has active routing — that's the passive indicator.)
- Standard sections in order: Dashboard, Autopilot, Layer, Video, Transition,
  Transform, Effects, Cuepoints, Transport

**Success criteria:** Inspector rows match directive grammar exactly. Section headers
match directive spec. Per-parameter routing via Triangle only (P. button was REMOVED 2026-05-22 — see Phase 3 update notes above).

---

### Phase 4: Top Chrome Redesign
**Sessions:** 1
**Department:** engineering
**Skill:** feature-build
**Builders:** 2 (parallel)

**Builder A — TopBar restructure (3-strip layout):**
- TopBar.h/cpp + MainComponent (resized height 34 → 98)
- Strip 1 (44px): Logo | BPM 48px BLUE (IBM Plex Mono Medium 500) | BAR | PHRASE |
  Structural State Pill (blue) | Beat Wheel | Transport buttons | Quantize | Fade |
  Master | REC indicator (kDanger) | FPS | [LIVE] [PROGRAM] [SETUP] mode tabs |
  Layout hotkey buttons (right) | Pulse AI button (right)
- Strip 2 (32px): Transport strip — play/pause/stop | BPM number | -/+ | prev/next |
  /2 x2 | TAP | RESYNC | loop. Separate from strip 1.
- Strip 3 (22px): Signal quick-ref bar — 15+ live signal values, mono 11px.
  Clickable to expand as overlay. Shows: RMS|PEAK|LUFS|FLUX|CENTROID|BASS|MID|HIGH|...

- BPM display: 48px font size, IBM Plex Mono Medium, kAccent color. THE dominant element.
  Must be the largest text on screen.

**Builder B — Mode tabs + structural state:**
- Mode tabs in Strip 1: [LIVE] [PROGRAM] [SETUP]
  - ALL-CAPS, Sans, letter-spacing 0.5-1px
  - Active: kAccent text + 2px kAccent underline
  - Inactive: kLabel text
  - Click switches mode (integrates with ModeManager from Phase 5)
- Structural state pill: Display current state (DROP/BUILDUP/BREAKDOWN/NORMAL)
  as a blue pill shape (square corners per directive) on strip 1
- Beat wheel: 4 squares in a row (14x14px), active square fills kAccent

**Success criteria:** Top chrome is 98px total (3 strips). BPM at 48px is the largest
element. Signal bar is 22px and shows live values. All typography matches directive.

---

### Phase 5: Mode System
**Sessions:** 1
**Department:** engineering
**Skill:** feature-build
**Builders:** 2 (parallel)

**Builder A — ModeManager core:**
- Create `src/ui/ModeManager.h` and `src/ui/ModeManager.cpp`
- Enum: `Mode { Live, Program, Setup }`
- Per-mode state: last used layout template, panel positions, selected items
- API: `setMode(Mode)`, `getMode()`, `onModeChanged` callback
- Integration with MainComponent: mode switch triggers layout restore
- Remove vestigial ProgrammingMode component (ProgrammingMode.h/cpp)

**Builder B — Mode UI integration:**
- Wire mode tabs in TopBar to ModeManager
- Keyboard shortcuts: Ctrl+1=LIVE, Ctrl+2=PROGRAM, Ctrl+3=SETUP
- View menu: add mode items
- Mode remembers state: LIVE→PROGRAM→LIVE returns to last LIVE layout

**Success criteria:** Three modes work. Each remembers its layout. Mode tabs show
correct active state. Old ProgrammingMode component removed.

---

### Phase 6: Layout Template System
**Sessions:** 2
**Department:** engineering
**Skill:** feature-build + site-architecture (app mode) for IA

**Session 1:**
**Builder A — LayoutTemplate data model:**
- Create `src/ui/LayoutManager.h` and `src/ui/LayoutManager.cpp`
- LayoutTemplate struct: name, panel positions (fractions), panel visibility,
  associated mode, hotkey assignment
- LayoutManager: vector of templates, save/load to JSON, apply to MainComponent
- Extract current fraction-based divider system from MainComponent into LayoutManager
- Default templates: H1 (default launch), P1, S1

**Builder B — Layout persistence:**
- JSON serialization for LayoutTemplate (using juce::var/JSON)
- Save/load to composition file or separate layout file
- Hotkey assignment UI in top bar (layout hotkey buttons per directive 5.5)
- Per-mode last-used template tracking

**Session 2:**
**Builder A — H1 default layout (v9_brut_stacked_var_b equivalent):**
- Implement the default launch layout per directive Section H1
- Left: signal columns for active signals
- Center: deck tabs + clip grid
- Right: inspector tabs (CLIP|LAYER|COMP|SIGNALS, foldable)
- Bottom: focus area (right of preview), contextual detail

**Builder B — P1 performance layout:**
- Central clip/output/layer display with tabs
- Parameters on right connected to displayed content
- Optimized for live performance (larger clip cells, minimal inspector)

**Builder C — S1 programming layout:**
- Left: all signal parameters
- Right: all global/layer/clip parameters
- Center: full routing workspace
- Optimized for signal programming and macro building

**Success criteria:** Layout templates save/load. Hotkeys switch layouts. H1 is default
on launch. Mode switching restores last layout. At least H1, P1, S1 implemented.

---

### Phase 7: Bottom Focus Bar + Signal Drawer
**Sessions:** 1
**Department:** engineering
**Skill:** feature-build
**Builders:** 2 (parallel)

**Builder A — BottomFocusBar:**
- Create `src/ui/BottomFocusBar.h` and `src/ui/BottomFocusBar.cpp`
- Collapsed: thin bar showing summary of current selection
  (e.g., "MACRO: BASS PUMP -> 4 params" or "EFFECT: Ripple -> L2-C3")
- Expanded: click to expand upward, default ~30% of screen, user-draggable height
- Context-sensitive: shows detail for whatever is selected above
- Uses inspector grammar for all controls
- Integration: replaces or works alongside current bottom panel area

**Builder B — Signal Drawer overlay:**
- Modify SignalBar behavior:
  - Collapsed (22px): shows live values (already similar)
  - Expanded: OVERLAYS entire workspace below (not hides)
  - Current behavior: expanded hides all panels (wrong per directive)
  - New behavior: expanded draws ON TOP of workspace, dismiss to return
- Add dismiss interaction (click outside, Escape key, click collapse button)
- Full-height drawer for signal configuration, global-scope signal groups

**Success criteria:** Bottom focus bar shows context-sensitive summary. Expands on click.
Signal drawer overlays workspace (not replaces). Both dismiss properly.

---

### Phase 8: Signal System Completion
**Sessions:** 2
**Department:** engineering
**Skill:** feature-build

**Session 1:**
**Builder A — Clip/Layer MacroBank activation (Ghost Feature 22a):**
- Add MacroBank member fields to Clip and Layer model structs
- Instantiate Clip and Layer scope MacroBanks
- Wire to inspectors (ClipInspector, LayerInspector get setMacroBank)
- Update serialization (toVar/fromVar) for persistence
- MIDI routing for clip/layer macros

**Builder B — Route TargetScope activation (Ghost Feature 22c):**
- Renderer.cpp:200 — add handlers for Clip and Layer scopes
- Access compositor's per-layer effect chains from routing lambda
- Per-clip parameter routing via compositor
- Test: create a Clip-scope route, verify it takes effect

**Session 2:**
**Builder A — Signal Group processing chains:**
- Upgrade MacroBank from "8 knobs" model to "processing chain" model
  per directive Section 1.1 (Signal Groups = Ableton-style racks)
- SignalGroup: input sources (one or more signals), processing chain
  (smoothing, inverting, scaling, gating), single output
- Fan-out: one output can drive many parameters
- Save/recall/move between scopes
- Named, reusable Signal Groups

**Builder B — Per-layer signal columns on LayerStrip:**
- LayerStrip.h/cpp: add signal column area (left side of each layer row)
- Auto-populated: moment a signal is routed, column appears; remove route, column disappears
- Horizontal scroll, 4-6 visible at a time
- Source columns next to destination columns, all animating in real time
- Each column shows VerticalSignalColumn component from Phase 2

**Success criteria:** All three scopes (Global/Layer/Clip) have working signal routing.
Signal Groups save/recall/move. Layer strips show active signal columns.

---

### Phase 9: Hit System Data Model
**Sessions:** 1
**Department:** engineering
**Skill:** feature-build (consider @LBL-code for lifecycle)
**Builders:** 1

**Tasks:**
1. Create `src/model/Hit.h` and `src/model/Hit.cpp`:
```cpp
struct HitPayload {
    struct SignalActivation { int signalId; bool active; };
    struct CellTrigger { int layerIdx; int columnIdx; /* or columnTrigger */ };
    struct MacroActivation { int macroId; MacroBank::Scope scope; bool active; };

    std::vector<SignalActivation> signals;
    std::vector<CellTrigger> clips;       // cells or columns
    std::vector<MacroActivation> macros;
    std::map<std::string, float> parameterValues;  // param ID -> value
    std::map<std::string, bool> effectStates;       // effect ID -> on/off
    std::set<std::string> excluded;                 // filter: unchecked items
};

struct HitEnvelope {
    enum class Onset { Instant, FadeIn };
    enum class Curve { Linear, EaseIn, EaseOut, EaseInOut, Exponential, SCurve, Step };
    enum class Release { InstantCut, FadeOut };

    Onset onset = Onset::Instant;
    float onsetDuration = 0.0f;   // beats or bars
    Curve curve = Curve::Linear;
    Release release = Release::InstantCut;
    float releaseDuration = 0.0f;

    // Per-parameter overrides
    std::map<std::string, HitEnvelope> paramOverrides;
};

struct Hit {
    int id;
    double timestamp;           // bar.beat position
    HitPayload payload;
    HitEnvelope envelope;
    float power = 1.0f;        // 0->1 magnitude
    std::vector<int> chainLinks; // Hit IDs this triggers
    // Conditions
    struct Condition { std::string signalId; float threshold; };
    std::vector<Condition> conditions;
};

struct HitGroup {
    std::string name;
    std::vector<Hit> hits;     // ordered by timestamp
    // A HitGroup can represent a full programmed show for a track
};
```
2. Serialization (toVar/fromVar) for all Hit structures
3. Integration point: Composition model gains `std::vector<HitGroup> hitGroups`
4. Hit evaluation engine: given current playhead position, determine which Hits fire
5. Diff-based override: later Hits override earlier Hits per-parameter only

**Success criteria:** Hit data model compiles. Serialization round-trips. Hit evaluation
returns correct Hits for a given position. Diff-based override works correctly.

---

### Phase 10: Hit System UI
**Sessions:** 3
**Department:** engineering
**Skill:** feature-build

**Session 1 — Core Hit UI:**
**Builder A — Hit creation workflow:**
- Capture Hit button (in TopBar transport area or dedicated)
- On press: snapshot current composition state into HitPayload
- Filter step: dialog/panel showing all captured items with checkboxes
- Uncheck = exclude from this Hit (previous state persists per directive 2.4)
- Works identically in LIVE and PROGRAM modes

**Builder B — Hit pill visualization:**
- Create HitPill component (src/ui/HitPill.h/cpp)
- Small vertical capsule: ~8px wide x ~24px tall
- 1px kBorder outline, kBg fill
- Three indicators inside (S/C/M): kAccent when present, dim when not
- States: default, hover (tooltip), selected (2px kAccent border), active (full kAccent fill flash)
- Render on timeline at Hit position

**2026-05-22 refinements (Phase 10 implementation guidance):**

The Capture Hit workflow is now ONE OF THREE creation workflows, all using a SINGLE UNIFIED Save/Capture BUTTON:

1. **Scrub-and-save** — VJ scrubs recording (master playhead), clicks an empty Hit slot to SELECT it as save target (highlight only — does NOT move playhead), presses the unified Save button → Hit created at selected slot with state captured at the scrub position. If slot was occupied → CONFIRM DIALOG ("Replace this Hit?") — NOT merge.

2. **Two-Hit automation** — two Hits define start/end, envelope between them. DEFAULT ENVELOPE: LINEAR. User can change to instant-cut / ease-in / ease-out / S-curve / exponential / step via the Hit Manager.

3. **Live capture** — same unified button captures at current playhead. ADJACENT QUANTIZE TOGGLE next to button: Exact beat / Next bar / Closest bar.

**No drag-and-drop.** Click-to-select + button-to-save only.

**Hit quantization:** Hits live on a beat grid. Max 4 Hits per bar (1 per beat in 4/4). User-adjustable coarser. No two Hits at same position by design — Scenario 3 conflict is impossible.

See `../../FEATURE_CONNECTIONS.md` Scenario 11 for full workflows.

**Session 2 — Timeline + Inspector:**
**Builder A — Hit lane on WaveformDisplay:**
- Add Hit lane below stereo waveform pair
- Place HitPill components at timeline positions
- Click to select, double-click to edit
- Hit chains shown as thin connecting lines between pills
- Scrollable, zoomable with waveform

**Builder B — Hit inspector in BottomFocusBar:**
- When a Hit is selected, BottomFocusBar shows Hit anatomy:
  - Signals captured, Clips captured, Macros captured
  - Parameter snapshots, Envelope settings
  - Power/intensity slider, Timing display
  - Conditions, Chain links, Filter checkboxes
- Uses inspector grammar throughout

**Session 3 — Groups + Patterns:**
**Builder A — Hit Group management:**
- Hit Group creation, naming, save/recall
- HitGroup as reusable sequence
- Deploy Hit Group to timeline
- Hit Group editor in PROGRAM mode

**Builder B — Hit chains + envelope editor:**
- Chain visualization: thin lines/arcs between chained Hit pills
- Envelope editor: curve visualization between Hits
- Per-parameter envelope override UI

**Builder C — Pattern presets:**
- Pre-built rhythmic Hit spacings: 1, 2, 4, 8, 16, 32, 64 bars
- Irregular patterns (2-2-3-3)
- Pattern application to timeline
- Multi-affect capabilities (signals/macros/params)

**Success criteria:** Hits can be captured, placed on timeline, edited. Hit pills render
correctly. Bottom focus bar shows Hit details. Hit Groups save/deploy. Envelopes interpolate.
Pattern presets available.

---

### Phase 11: Waveform Enhancement
**Sessions:** 1
**Department:** engineering
**Skill:** feature-build
**Builders:** 2 (parallel)

**Builder A — Display modes:**
- Add mode selector buttons on right side of waveform
- Mode 1: Condensed stereo pair (current, as in v9_brut_crt_layout)
- Mode 2: DJ/CDJ-style scrolling waveform with beat grid, color-coded frequency bands
- Additional modes TBD (spectrum, overview)
- Click waveform → opens larger waveform control menu below it

**Builder B — Interaction + Hit integration:**
- Click interaction: click to place Hit, scrub playhead
- Hit markers visible on waveform timeline
- Horizontal signal bar can sit directly under waveform
- Same expand behavior for signal bar in mini form

**Success criteria:** Multiple waveform display modes. Mode selector works.
Click interaction functional. Hit markers visible on timeline.

**2026-05-22 timeline structure additions (likely Phase 10 or 11 work):**

Two-lane timeline structure beyond the waveform:
- **HITS lane** (beat-quantized): blue capsule pills at beat positions; envelope curves between consecutive pills
- **REC lane** (time-tick-labeled, decoupled from BPM): continuous density curve showing event activity; 60fps internal resolution

Both lanes:
- Thin by default; click handle → expand vertically
- Click-and-drag horizontally → zoom IN to dragged range
- After-Effects-style per-parameter sub-tracks when expanded (group → individual disclosure)

Recording lane: SCRUBBABLE (drag to scrub master playhead). Hits lane: NOT scrubbable; click to jump playhead.

Single active driver (HITS or RECORDING). Switch via lane handle dot. Recording lane has dropdown to switch between multiple stored recordings.

See `../../FEATURE_CONNECTIONS.md` Scenario 11.

---

### Phase 12: Documentation Alignment
**Sessions:** 1
**Department:** system
**Skill:** memory-update + manual edits
**Builders:** 1

**Tasks:**
1. CONTEXT.md: Add ~15 directive terms:
   - Hit, HitGroup, HitPayload, HitEnvelope, HitPill
   - Signal Group (Macro as processing chain)
   - Source (first tier of three-tier model)
   - Triangle (universal signal access point)
   - Vertical Signal Column (knob replacement)
   - Pulse (AI assistant)
   - Focus Bar (bottom expandable panel)
   - Signal Drawer (top overlay)
   - Layout Template
   - Mode (LIVE/PROGRAM/SETUP)
2. Rename audio onset "Hit" to "Onset" in CONTEXT.md (if approved)
3. ARCHITECTURE_V2.md: Annotate with directive references or create ARCHITECTURE_V3.md
4. FEATURES.md: Add new features (Hit system, Signal Groups, Layout Templates, Mode System)
5. Cross-reference directive sections to implementation files
6. **Cross-reference the new canonical docs:** Throughout CONTEXT.md, ARCHITECTURE_V2.md, and FEATURES.md, add cross-references to:
   - `../FEATURE_CONNECTIONS.md` (canonical behavioral reference)
   - `../MENTAL_MODELS.md` (conceptual spine)
   - `BORIS_DECISIONS.md` (updated 2026-05-22 with new decisions)

**Success criteria:** CONTEXT.md has all directive terms. ARCHITECTURE doc reflects
current state. FEATURES.md includes new features.

---

### Phase 13: Pulse AI Foundation (DEFERRED)
**Requires:** Open Question #8 from directive (scope of Pulse actions)
**Scope:** Chat interface, content navigation, signal routing suggestions, settings management
**Location:** Top right button in UI
**When Pulse inactive:** Signals tab area shows category headings (Files|Effects|Sources|Compositions|Recordings|MilkDrop)
**Implementation:** TBD after Boris defines scope

---

### Phase 14: Polish + Remaining Layouts
**Sessions:** 3-5 (ongoing)
**Department:** engineering + design
**Skill:** feature-build + design-review

**Tasks (in priority order):**
1. Clip cell styling per directive 5.9 (100x80 base, 6px waveform left, 14px name bar, states)
2. Layer strip signal column integration (full spec from Phase 8)
3. Content organization + tagging system (directive 5.12)
4. Beat wheel styling (4 squares 14x14px, active fills kAccent)
5. 3% SVG grain overlay on body (`opacity: 0.03`) — the only decorative element
6. Additional layouts: P2-P5, S2, H2-H8 (use layout template system from Phase 6)
7. Browser panel update: FILES|FX|SOURCES|COMP|RECORD|MILKDROP tabs per directive 5.11
8. Display & output configuration (directive 5.15)
9. Remaining open questions from directive Part 10

**Verification:** design-review audit after each major polish task.

---

## 7. DEPENDENCY MAP

```
Phase 0 (Fonts + Tokens)
    |
    v
Phase 1 (Colors + Corners)
    |
    +---> Phase 2 (Knob Replacement)
    |         |
    |         v
    +---> Phase 3 (Inspector Grammar)
    |
    +---> Phase 4 (Top Chrome) --------+
    |                                   |
    +---> Phase 5 (Mode System) --------+---> Phase 6 (Layout Templates)
    |                                   |
    +---> Phase 7 (Focus Bar + Drawer)  |
    |                                   |
    +---> Phase 8 (Signal Completion)   |
              |                         |
              v                         |
         Phase 9 (Hit Data Model)       |
              |                         |
              v                         v
         Phase 10 (Hit UI) <----------- Phase 6
              |
              v
         Phase 11 (Waveform Enhancement)
              |
              v
         Phase 12 (Documentation)
              |
              v
         Phase 14 (Polish)
```

**Critical path:** 0 → 1 → 4 → 5 → 6 → 10 → 14
**Parallel paths after Phase 1:** 2, 3, 7, 8 can all start simultaneously

---

## 8. FILE ASSIGNMENT MATRIX (Phase 1 detail — template for all phases)

### Phase 1 Builder A (Top Chrome + Analysis)
```
src/ui/TopBar.h                    src/ui/TopBar.cpp
src/ui/SignalBar.h                 src/ui/SignalBar.cpp
src/ui/SignalStrip.h               src/ui/SignalStrip.cpp
src/ui/SignalInspector.h           src/ui/SignalInspector.cpp
```

### Phase 1 Builder B (Inspector + Controls)
```
src/ui/InspectorPanel.h            src/ui/InspectorPanel.cpp
src/ui/ClipInspector.h             src/ui/ClipInspector.cpp
src/ui/LayerInspector.h            src/ui/LayerInspector.cpp
src/ui/CompositionInspector.h      src/ui/CompositionInspector.cpp
```

### Phase 1 Builder C (Browsers + Recording)
```
src/ui/BrowserPanel.h              src/ui/BrowserPanel.cpp
src/ui/FilesBrowser.h              src/ui/FilesBrowser.cpp
src/ui/FXBrowser.h                 src/ui/FXBrowser.cpp
src/ui/SourcesBrowser.h            src/ui/SourcesBrowser.cpp
src/ui/MilkDropBrowser.h           src/ui/MilkDropBrowser.cpp
src/ui/CompDecksBrowser.h          src/ui/CompDecksBrowser.cpp
src/ui/RecordPanel.h               src/ui/RecordPanel.cpp
```

### Phase 1 Builder D (Remaining)
```
src/MainComponent.h                src/MainComponent.cpp
src/ui/DeckView.h                  src/ui/DeckView.cpp
src/ui/LayerStrip.h                src/ui/LayerStrip.cpp
src/ui/ClipCell.h                  src/ui/ClipCell.cpp
src/ui/Knob.h                      src/ui/Knob.cpp
src/ui/UniversalParamControl.h     src/ui/UniversalParamControl.cpp
src/ui/MacroPanel.h                src/ui/MacroPanel.cpp
src/ui/EffectStackView.h           src/ui/EffectStackView.cpp
src/ui/MappingEditor.h             src/ui/MappingEditor.cpp
src/ui/SpectrumDisplay.h           src/ui/SpectrumDisplay.cpp
src/ui/AudioReadoutPanel.h         src/ui/AudioReadoutPanel.cpp
src/ui/WaveformDisplay.h           src/ui/WaveformDisplay.cpp
src/ui/EffectsRackPanel.h          src/ui/EffectsRackPanel.cpp
src/ui/OutputWindow.h              src/ui/OutputWindow.cpp
src/ui/PreviewPanel.h              src/ui/PreviewPanel.cpp
src/ui/PreferencesDialog.h         src/ui/PreferencesDialog.cpp
src/ui/BindingOverlay.h            src/ui/BindingOverlay.cpp
src/ui/MidiLearnOverlay.h          src/ui/MidiLearnOverlay.cpp
src/ui/ProgrammingMode.h           src/ui/ProgrammingMode.cpp
src/ui/PresetManager.h             src/ui/PresetManager.cpp
src/ui/MenuBarModel.h              src/ui/MenuBarModel.cpp
src/ui/TimingWindow.h              src/ui/TimingWindow.cpp
```

---

## 9. VERIFICATION STRATEGY

### Per-Phase Verification

| Phase | Verification Method |
|-------|-------------------|
| 0 | Build compiles. Font renders in test label. |
| 1 | Zero `fillRoundedRectangle` in grep. Zero old color names. Screenshot audit. |
| 2 | Zero `Knob` in grep. Vertical columns render. MacroPanel functional. |
| 3 | Inspector row measurements match spec. Triangle-only routing functional; no P. button anywhere (REMOVED 2026-05-22). |
| 4 | Top chrome height = 98px. BPM at 48px is largest element. Signal bar 22px. |
| 5 | Mode switching works. Per-mode state persists. |
| 6 | Layout save/load works. Hotkeys switch. H1/P1/S1 functional. |
| 7 | Focus bar expands/collapses. Signal drawer overlays. |
| 8 | Clip/Layer routing functional. Signal Groups save/recall. |
| 9 | Hit data model serializes/deserializes. Evaluation correct. |
| 10 | Hits capture, place, display, edit. Envelopes interpolate. |
| 11 | Waveform modes switch. Click interaction works. |
| 12 | CONTEXT.md has all terms. Architecture doc current. |
| 14 | design-review skill audit passes. |

### Skills for Verification
- **design-review** — after Phases 1, 2, 4, 7, 14 (visual quality audit)
- **test-coverage** — after Phases 8, 9, 10 (new feature test gaps)
- **repo-audit** — after Phase 12 (documentation health)

---

## 10. OPEN QUESTIONS FOR BORIS

> **All Q1-Q28 questions are RESOLVED.** Several have been further refined
> in the 2026-05-22 session. The canonical statement of each resolution
> now lives in `../../FEATURE_CONNECTIONS.md`. Listed here for historical
> context and cross-reference.

### ALL DECISIONS CONFIRMED (2026-05-21)

Boris approved all recommendations. These are final and locked.

**Visual System:**
- Q1: Audio "Hit" signal → renamed to **"Onset"** in UI (code stays `onsetDetected`)
  - ✅ Confirmed 2026-05-22: LINEAR (Ableton-style). See FEATURE_CONNECTIONS Scenario 11 Workflow 2.
- Q2: All signal meters use **blue kAccent only** — no category colors
- Q3: Routing indicator = **triangles only**. P. button on **section headers only** (route-entire-section). Magenta deleted.
  - 🔄 REFINED 2026-05-22: P. button REMOVED entirely (not just from rows). Triangle-only routing. See FEATURE_CONNECTIONS Scenario 3.
- Q4: Signal smoothing **SPLIT** — triangles are RAW (no easing), column fills are SMOOTHED (0.3 alpha)
- Q5: Beat wheel = **4 squares in a row** (14x14px each), not circular. Active square fills kAccent.
- Q6: Signal column width on layer strips = **60px fixed**. 5 visible at typical width.
- Q7: Scrollbar thumbs = **square** (zero border-radius, directive-compliant)
- Q8: Monospace scope: BPM=mono, inspector values=mono, signal bar values=mono, timestamps=mono. Clip names=sans, buttons=sans, section headers=sans.

**Layout System:**
- Q9: Bottom focus bar = **thin docked bar at very bottom**, expands upward over existing panels
- Q10: Signal drawer overlay = **100% workspace coverage**, dismiss via click-outside/Escape/bar-click
- Q11: Layout scope V1 = **3 layouts (H1/P1/S1)**, one per mode. Add more incrementally.
- Q12: v1 dead controls = **delete entirely**. Clean slate.

**Signal System:**
- Q13: Per-layer sparkline = **auto-select primary routed signal**. Right-click to override.
- Q14: Signal Group persistence = **both** (composition owns + global library for sharing)

**Hit System (Directive Open Questions resolved):**
- Q15: Default Hit envelope = **Instant onset + Linear curve**
- Q16: Signal-to-macro modulation depth = **per-macro master depth slider** (one per macro, 0-100%)
- Q17: Hit Group nesting = **flat only** for V1 (groups contain Hits, not other groups)
- Q18: Data recording format = **JSON** for V1 (consistent with SessionRecorder)
- Q19: Live Hit triggering = **both fire, diff-merge** (per directive's per-parameter override)
  - 🔄 REFINED 2026-05-22: Hits and Live VJ are both state mutators (equal priority, last-write-wins). Live VJ touch creates an OVERRIDE that locks Hits from writing that field until AUTO release. Signal continues running. See FEATURE_CONNECTIONS Scenario 1 + 6.
- Q20: Hit lane spacing = **same position allowed** (parallel Hits by design, Section 2.8)
- Q21: Empty composition state = **empty dark stage**, BPM "---", all panels visible, signal bar shows zeros
- Q22: Hit preview = **ghost preview on hover** (semi-transparent composition overlay, no state change)
- Q23: Clip cell effects = **confirmed: already supports multiple effects** (no change needed)
  - ✅ Confirmed 2026-05-22: cell-scoped clip instances — same source media can live in multiple cells, each with own in/out/cuepoints. See FEATURE_CONNECTIONS Scenario 9.
- Q24: Pulse AI scope = **configuration assistant** for V1 (browse/suggest/configure, no live triggering). Deferred to Phase 13.

**Implementation Strategy:**
- Q25: Next session = **HTML mockups for new screens** + JUCE Phase 0 for foundation
- Q26: @LBL-code for Hit system = **yes for data model (Phase 9)**, no for UI (Phase 10)
- Q27: Phase execution priority = **visual first** (Phase 0→1→2→3→4)
- Q28: Session Recorder timestamps = **dual** (wall-clock + beat-relative)

### All Directive Part 10 Open Questions — RESOLVED

2. **Meter color migration:** Current meters use green/yellow/red for signal categories
   (amplitude=green, rhythm=magenta, etc.). Directive says blue only. Should ALL
   signal meters become blue, or keep category colors for the Signal Bar only?
   (Recommended: all blue — directive is explicit about one accent color)

3. **Magenta mapping indicator:** Knob.cpp draws a magenta arc when a parameter is
   mapped to a signal. With kAccentMagenta deleted, what replaces it?
   (Recommended: use kAccent blue — the triangle already indicates routing; P. button was REMOVED 2026-05-22)

4. **Scrollbar styling:** Current scrollbars have fully-rounded thumbs.
   Directive says no border-radius. Make scrollbar thumbs square?
   (Recommended: yes — consistency with directive)

5. **Signal column width in layer strip:** How many pixels wide should each
   vertical signal column be on the layer strip? Directive says "4-6 visible at a time."
   At typical layer width ~800px minus controls, that's ~50-80px per column.
   (Recommended: 60px default, user-resizable)

6. **Hit system scope for LBL-code:** The Hit system (Phases 9-10) is complex enough
   for @LBL-code lifecycle management. Should we use it?
   (Recommended: yes for Phase 9 data model, no for Phase 10 UI — UI is iterative)

7. **Default Hit envelope (Open Question #1 from directive):** When a user captures
   a Hit without specifying envelope, what are the defaults?
   (Recommended: Instant onset + Linear curve — simplest, most predictable)

8. **Signal-to-macro modulation depth (Open Question #2):** Is there a depth control
   when a signal modulates a macro? Or always 100%?
   (Recommended: depth control 0-100%, default 100% — more flexibility)

9. **Hit Group nesting (Open Question #3):** Can Hit Groups contain other Hit Groups?
   (Recommended: flat first, nesting later — simpler data model to start)

10. **Clip cell flexibility (Open Question #10):** Can a clip contain video + MULTIPLE
    stacked effects? Or limited to one effect per cell?
    (This already exists — clips have effect stacks. Confirm with directive.)

### Directive Part 10 Questions — ALL RESOLVED

| # | Question | Decision |
|---|----------|----------|
| 1 | Default Hit envelope | Instant onset + Linear curve |
| 2 | Signal-to-macro modulation depth | Per-macro master depth slider (0-100%) |
| 3 | Hit Group nesting | Flat only V1 |
| 4 | Data recording storage format | JSON V1 |
| 5 | Live Hit triggering during playback | Both fire, diff-merge (per-parameter override) |
| 6 | Hit lane minimum spacing | Same position allowed (parallel Hits) |
| 7 | Empty composition state | Empty dark stage, BPM "---", panels visible |
| 8 | Pulse AI scope | Config assistant V1 (no live triggering) |
| 9 | Hit preview without firing | Ghost preview on hover |
| 10 | Clip cell flexibility | Already supports multiple effects (confirmed) |

### Implementation Decisions (Harmony can resolve)

11. Font embedding: TTF or OTF? (TTF — broader JUCE compatibility)
12. Layout template file format: embedded in Composition or separate file?
    (Separate file — layouts are app-wide, not per-composition)
13. Hit evaluation: per-frame in render thread or on beat boundaries only?
    (Per-frame for smooth envelope interpolation, but with beat-quantized triggering)

---

## 11. QUICK REFERENCE — Directive Section Cross-References

| Directive Section | Implementation Phase | Key Files |
|-------------------|---------------------|-----------|
| Part 1.1 (Three-Tier Signal) | Phase 8 | src/signal/*, src/routing/* |
| Part 1.2 (Three Scopes) | Phase 8 | src/routing/MacroBank.*, Renderer.cpp |
| Part 1.3 (Triangle) | Already exists | src/ui/UniversalParamControl.* |
| Part 1.4 (Vertical Columns) | Phase 2 | NEW: VerticalSignalColumn.* |
| Part 1.5 (Programming Layout) | Phase 6 | LayoutManager.*, S1 template |
| Part 1.6 (Signal Drawer) | Phase 7 | SignalBar.* modification |
| Part 1.7 (Focus Bar) | Phase 7 | NEW: BottomFocusBar.* |
| Part 2.1-2.17 (Hit System) | Phases 9-10 | NEW: src/model/Hit.*, src/ui/Hit*.* |
| Part 3 (Pulse AI) | Phase 13 (deferred) | TBD |
| Part 4.1 (Colors) | Phases 0-1 | LookAndFeel.* + all UI files |
| Part 4.2 (Typography) | Phase 0 | LookAndFeel.*, CMakeLists.txt |
| Part 4.3 (Hard Visual Rules) | Phase 1 | All UI files |
| Part 5.1 (Top Chrome) | Phase 4 | TopBar.* |
| Part 5.2 (Focus Bar) | Phase 7 | NEW: BottomFocusBar.* |
| Part 5.4 (Mode System) | Phase 5 | NEW: ModeManager.* |
| Part 5.5 (Layout Templates) | Phase 6 | NEW: LayoutManager.* |
| Part 5.6 (Inspector Rows) | Phase 3 | UniversalParamControl.* |
| Part 5.7 (Section Headers) | Phase 3 | ClipInspector.*, LayerInspector.* |
| Part 5.9 (Clip Cells) | Phase 14 | ClipCell.* |
| Part 5.10 (Layer Strip) | Phase 8 | LayerStrip.* |
| Part 5.13 (Waveform) | Phase 11 | WaveformDisplay.* |
| Part 6 (Layout Modes) | Phase 6 | LayoutManager.* |
| Part 8 (Rejected Patterns) | All phases | Reference — what NOT to do |
| Part 10 (Open Questions) | Phase 14 | See Section 10 above |

---

## 12. ADDENDUM — Competitive Intelligence & Quick Wins

### 12.1 Design Rationale from Competitive Analysis

The directive's choices are grounded in competitive research (design/ux_analyses/):

**From Resolume Arena:**
- Clip grid paradigm is sacred — don't reinvent, match exactly
- Layer controls (X/B/S/M/A/V) always visible, no navigation required
- Inspector rows = compact horizontal sliders (18-20px) — high density
- Progressive disclosure: grid → properties → deep settings
- Dark charcoal palette with single accent color

**From Ableton Live:**
- Rack/macro concept = Audio-DNA's Signal Groups (processing chains, not just knobs)
- Up to 16 macros per rack, each controlling multiple params with range/invert
- Snapshots enable instant recall — consider adding to Signal Groups
- MIDI mapping UX: 3 actions (click mode → click param → move controller), blue overlay
- **Action:** Signal routing mode should use color overlay showing all assignable targets

**From DJ Software (Traktor/Serato/rekordbox):**
- Waveform color modes encode frequency info (Serato: red=bass, green=mid, blue=treble)
- Color-coded cue points → maps directly to Hit pills
- Smart library features (tags, collections, search) needed for 135 effects + 101 sources
- CDJ-style scrolling waveform with beat grid for DJ view mode

**From GrandMA3 (Lighting):**
- Tile-based reconfigurable workspace → Audio-DNA's layout template system
- Cue stack visualization → Hit Group management pattern
- Systematic color coding reduces cognitive load under pressure
- Dual-input paradigm (GUI + command-line) — aspirational for v2+

### 12.2 Quick Wins (can be activated with minimal work)

| Feature | Ghost Feature | Effort | Impact |
|---------|--------------|--------|--------|
| **MappingSuggester** | 22e | ~1 day | AI mapping suggestions in UI — unique differentiator |
| **LinkSync requestBeatAtTime** | 22d | ~1 day | Ableton Link downbeat sync button |
| **Hidden audio signals** | N/A | ~0.5 day | Surface 21 hidden signals in expanded signal bar |
| **Session Recorder full wiring** | Partial 25 | ~2 days | Wire remaining 6/7 event types for complete recording |

These should be folded into early phases as bonus tasks (no extra sessions needed).

### 12.3 Additional Implementation Details Discovered

**Clip cell dimensions mismatch:**
- Current code: 90x96px (DeckView.cpp)
- Directive spec: 100x80px base unit (Section 5.9)
- Add to Phase 14 clip cell styling task

**Layer strip width:**
- Current: 250px fixed
- Directive: needs signal column area added to left side (Phase 8)
- Signal columns: 4-6 visible, ~60px each = ~240-360px added
- Total layer strip with columns: potentially 500-600px
- Need horizontal scroll for columns per directive

**Session Recorder timestamp model:**
- Current: wall-clock timestamps (milliseconds)
- Directive Hit system: bar/beat timestamps
- Reconciliation needed: recording system should support both timestamp modes
- Phase 9 (Hit data model) should define beat-relative timestamps
- Phase 10 should add wall-clock→beat conversion using current BPM

**Signal color migration detail:**
- `getSignalColour()` at SignalStrip.cpp:311 assigns per-category colors:
  - Amplitude → kMeterGreen, Bands → color wheel, Rhythm → kAccentMagenta,
    Pitch → mint, Chroma → gold, Timbre → lavender, Structure → kMeterYellow,
    Modulation → kAccentCyan
- Directive: ALL meters use kAccent (blue) only. Brightness = signal value.
- This means: delete `getSignalColour()` entirely, always return kAccent
- Signal differentiation shifts from color to POSITION (left-right in bar) and LABEL

**Font embedding verification:**
- Current: system default sans-serif, no monospace anywhere
- No font files in repo. No binary resource loading.
- IBM Plex must be downloaded from Google Fonts:
  - IBM Plex Mono: https://fonts.google.com/specimen/IBM+Plex+Mono
  - IBM Plex Sans: https://fonts.google.com/specimen/IBM+Plex+Sans
- Need Regular (400) + Medium (500) weights for Mono, Regular (400) for Sans
- JUCE binary data embedding adds ~200-400KB to app binary

### 12.4 Reference Documents for Builders

Builders should read these existing project docs for implementation context:

| Document | Location | Relevant For |
|----------|----------|-------------|
| ARCHITECTURE_V2.md | ~/projects/RealTimeAudio/ | Overall system architecture |
| AUDIO_DNA_DIRECTIVE_FULL.md | ~/projects/RealTimeAudio/ | THE design bible |
| MACRO_CONDITIONS_PLAN.md | design/ | Signal Group / MacroBank wiring detail |
| GAP_REPORT.md | design/ | Ghost features, what's missing |
| FEATURES.md | .harmony/ | Complete feature inventory with code paths |
| CONTEXT.md | ~/projects/RealTimeAudio/ | Domain glossary (39 terms) |
| Resolume Screen.png | design/ux_analyses/ | Visual reference for target aesthetic |
| RESOLUME_UI_UX_ANALYSIS.md | design/ux_analyses/ | Why we match Resolume grammar |
| Ableton_Live_12_UI_UX_Analysis.md | design/ux_analyses/ | Rack/macro concepts |
| GrandMA3_UI_UX_Analysis.md | design/ux_analyses/ | Cue stack → Hit system |

### 12.5 Critical Bottom-Area Findings (Previously Missed)

**H1 Bottom Band (245px, position 817-1062px) — THREE COLUMNS:**

1. **LEFT: Preview (380px fixed)** — 320x180 preview window with LIVE badge, orange accent
   bar, now-playing info (title, layer/clip, BPM/bar/loop)

2. **CENTER: Audio Analysis (flex)** — 4-column grid, the complete audio dashboard:
   - Col A (180px): Amplitude (RMS/PEAK/LUFS/DYN) + Rhythm (BAR/BEAT/PHR)
   - Col B (1fr): 7-band spectrum (SUB→BRI) + Transients (ONSET/DENS)
   - Col C (180px): Pitch/Key + Structural badges (NORMAL/BUILDUP/DROP/BREAK) + Genre
   - Col D (200px): Advanced P25 (PUMP/SWING/FORM/RESO/REESE) + 12-bar Chroma

3. **RIGHT: Active Routes (400px fixed)** — 6 route cards showing signal bindings
   (e.g., "BASS -> L1 ZOOM (EXP)", "RMS -> L2 GLOW INT (LIN)")
   - Grid: 4-column (SRC -> ARROW -> TARGET -> CURVE)
   - Selected route highlighted in accent

**THIS is what Boris meant by "bottom focus area to the right of the preview"
(the Audio Analysis grid) and "right side bottom area for another type of focus"
(the Active Routes panel).**

**S2 "PERFECT" LAYOUT (v9_brut_wave_03_inverted_l.html) — NOT PREVIOUSLY ANALYZED:**

Boris calls this the "perfect" layout. Key structural innovations:

- **Top bar: 86px** — BPM at 48px, 2x2 beat wheel (44x44px squares), transport,
  master fader, REC with timer, mode state display, key/genre tags
- **Waveform strip: 80px** — Two-wave display (A+B), procedural bars in blue/cyan,
  beat grid (64 markers), structural zones overlaid, playhead at 46%, onset flashes
- **Deck grid: 460px** — 4 layers, each with:
  - **SPARKLINE SIGNAL METERS per layer** (RMS, BASS, FLUX, HIGH — per layer!)
  - Column triggers with labels
  - Clip cells with effect dot counts and progress bars
- **Bottom section: THREE COLUMNS:**
  - LEFT: Signal Panel (200px) — grouped audio metrics (Amplitude/Bands/Rhythm/Spectral)
    with selection indicators and 6px meter bars
  - CENTER: Inspector (55%) — TWO-COLUMN inspector layout:
    - Left col: clip params (transport, cuepoints, autopilot, video)
    - Right col: preview SVG (320x180) + transform + effects + 4x2 dashboard knobs
  - RIGHT: Browser (flex) — 6-column source grid, search bar, tabs

**S2's design innovations vs H1:**
- Per-layer sparkline signal meters (which signal drives which layer, visible at a glance)
- Two-column inspector (params left, preview+dashboard right) — much more efficient
- Dedicated signal panel as left column vs H1's embedded audio grid
- Dashboard knobs positioned UNDER preview (contextual) vs H1's top placement
- Waveform with structural zones overlaid
- 48px BPM matching directive

**S2 is the LAYOUT template. S1 is the CSS/color template. The directive merges both
with cyan #00d9ff replacing orange/mint.**

### 12.6 Quality References — Boris's "Best" Elements (Pixel Specs)

These are the mockup elements Boris specifically praised. They define the quality bar.

**BEST Signal Bar (v9_brut_d6_modular — "Top favorite, very clean"):**
- Signal rows: exactly 20px height
- Layout: name 82px fixed | bar 5px height flex:1 | value 40px right-aligned
- Active state: `#1a0e05` bg (dark orange-brown), `#ff4500` text
- Group headers: 10px mono, uppercase, 1px letter-spacing, count badge right
- Signal row border: none between rows (density comes from alignment)
- Why "best": minimal whitespace, clean columns (name|bar|value), accent only on active

**BEST Condensed Sidebar (v9_brut_sidebar_focus — "pushed to edge"):**
- Total width: 230px (50px signal meters + 180px quick reference)
- Signal meters: 50px wide, vertical bars 6px width, border-right 1px #333
- Quick ref: 4-column grid for FX/source tiles, 38px tile height, 4px gap
- Padding: 6-8px (very tight)
- Position: absolute left:0 (literally pushed to screen edge)
- Why "best": zero wasted space, tight padding, hard borders, grid-aligned

**BEST Expanded Sidebar (v9_brut_audio_first — "deep signal analysis"):**
- Width: 220px, full height
- Signal rows: 16px min-height, label | 6px bar flex:1 | value | route triangle
- Selected row: bg #1a1a1a, left border 2px #ff4500
- Content: 25+ signals grouped (Amplitude→Bands→Rhythm→Pitch→Structure→Genre→Advanced)
- Inline mini chromagram: 12-note grid, 22px height, key highlighted in accent
- Inline mini sawtooth SVG: 10px height, shows waveform shape
- Why "best": every spectrum analyzed and visible, inline visualizations

**BEST Clip Cells (v9_brut_vj18_01 — "best sizing and dimensions"):**
- Width: 90px, height: flexible (60-80px typical)
- Name bar: position absolute bottom, padding 3px 5px (~14px total)
- Name bar has gradient shadow (readable over thumbnail)
- Clip number: top-left 9px, duration: top-right 9px
- Grid gap: 1px between clips
- Border: 1px #222 default, 1px #ff4500 (accent) when playing
- Why "best": compact, 10+ clips per row at 1920px, name proportional

**Per-Clip Waveform Boris LOVED (v9_brut_vj16a):**
- Mini waveform SVG: exactly 10px height, flex:1 width
- SVG: `viewBox="0 0 60 8"`, sawtooth polyline white 1px stroke
- Playhead: 1px vertical accent line at current position
- Clip playhead bar: 4px height, absolute bottom, accent color, width=progress%
- Background: #141414, border: 1px #2a2a2a
- Why Boris "loved": microformat, 10px is readable but minimal, playhead visible

**Implementation rule: When building any of these components, match these pixel
specs EXACTLY. These are the quality bar. Deviate only when the directive explicitly
overrides (e.g., accent color changes from orange to cyan).**

### 12.7 Mockup Reference Summary

The directive references 15+ HTML mockups as visual targets. Key ones:

| Layout | Mockup | Role |
|--------|--------|------|
| H1 (DEFAULT) | v9/v9_brut_stacked_var_b.html | Default launch layout |
| P1 | v9/v9_ten_03_performance_focus.html | Live performance |
| S1 | v9/v9_ten_02_programming_studio.html | Signal programming |
| S2 | v9/v9_brut_wave_03_inverted_l.html | "Perfect" layout |
| H4 | v9/v9_final_01_brutalist.html | Clean horizontal signals |
| H5 | v8/update/v8_20_vj17.html | Cleanest overall look |
| Color ref | v4/v4_16_inverted_l.html | Blue accent #00d9ff target |
| Waveform | v9/v9_wave_02_focus.html | Hit lane + waveform display |
| Clip cells | v9/v9_brut_vj18_01.html | Best cell sizing |
| Per-clip waveform | v9/v9_brut_vj16a.html | Mini waveform on clip left |
| Top section | v9/v9_brut_vj17.html | Best top arrangement |
| Signal bar | v9/v9_brut_d6_modular.html | Best dedicated signals bar |

Note: these are historical HTML mockups (392 total across v2-v9). They inform the
directive but should NOT be modified. Use as VISUAL REFERENCE for layout implementation.

---

## 13. Canonical Document Map (post-2026-05-22)

For builders working on Audio-DNA implementation, the canonical documents and when to consult each:

| Question | Doc |
|----------|-----|
| What is Audio-DNA's design vision? | `../../AUDIO_DNA_DIRECTIVE_FULL.md` (source-of-truth, updated 2026-05-22) |
| How does Audio-DNA think about itself conceptually? | `../../MENTAL_MODELS.md` (the spine) |
| How should X behave when Y happens? (precedence, edge cases) | `../../FEATURE_CONNECTIONS.md` Scenarios 1-12 |
| What did Boris decide and why? | `../../BORIS_DECISIONS.md` (short-form, updated 2026-05-22) |
| What phase am I in? What changes when? | This HANDOFF.md (implementation roadmap) |
| What's the current state of the code? | `../../.harmony/FEATURES.md`, `../../design/FEATURE_INVENTORY.md` |

When in doubt, FEATURE_CONNECTIONS.md and MENTAL_MODELS.md WIN over older statements in BORIS_DECISIONS, HANDOFF, or even (in narrow cases) the DIRECTIVE.

---

*End of handoff. Start with Phase 0. Execute in order. Parallelize where noted.
Total: 14 phases, ~15-18 sessions, 30+ builder dispatches.*
