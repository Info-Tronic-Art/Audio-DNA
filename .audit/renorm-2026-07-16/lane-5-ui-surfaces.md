# Lane L5 — UI Surface Census (src/ui/, MainComponent.h/.cpp)

Audit: renorm-2026-07-16. Read-only. Evidence = file:line. Scope: every window,
panel, overlay, tab, and user-operable control in src/ui/ + MainComponent.

**Method:** all 38 src/ui .h/.cpp pairs + MainComponent.h/.cpp read in full.
Display-only widgets (AudioReadoutPanel, SpectrumDisplay, WaveformDisplay) have no
operable controls and are listed as readouts.

---

## VERDICT

The v2 UI is the live surface: a single maximized main window (native menu bar +
TopBar + SignalBar + Deck + Preview + Waveform + TimingWindow + 4-tab Inspector +
6-tab Browser), plus an OutputWindow and an 8-tab Preferences dialog. It is broad and
mostly wired, BUT a whole v1 layer is compiled-in-yet-permanently-hidden
(EffectsRackPanel, MappingEditor, AudioReadoutPanel, SpectrumDisplay, ProgrammingMode),
~35 menu-bar items are no-op stubs, and 4 of 8 Preferences tabs plus TimingWindow's 3
tabs are empty placeholders. FEATURE_INVENTORY.md already documents most of this
accurately; CLAUDE.md's source tree is badly stale.

---

## 1. KEY COUNTS

- **Windows (top-level):** 3 — Main window (`Main.cpp:40` MainWindow/DocumentWindow),
  OutputWindow (`OutputWindow.h:69`, borderless fullscreen per-display), PreferencesDialog
  (`PreferencesDialog.h:8`, DialogWindow). Plus the native macOS menu bar (`Main.cpp:58`).
- **Panels:** ~28 distinct panel classes (18 shown in v2, 4 orphaned/hidden, rest are
  sub-components: LayerStrip, ClipCell, SignalStrip, MacroPanel, EffectStackView,
  UniversalParamControl, Knob).
- **Overlays / popups / modals:** BindingOverlay, MidiLearnOverlay, ProgrammingMode
  overlay (orphaned), MappingEditor (popup, orphaned), 3 PopupMenu builders (source
  picker, macro picker, add-signal), ComboBox dropdowns everywhere, ~15 native
  FileChoosers, 1 AlertWindow (ISF import). 9-menu menu bar (~80 items).
- **Tabs:** 27 — PreviewPanel 2, TimingWindow 3, InspectorPanel 4, BrowserPanel 6,
  MilkDropBrowser sub-tabs 4, PreferencesDialog 8. (+ MilkDrop 3 play modes.)
- **User-operable controls:** ~300+ (itemized below).

---

## 2. SURFACE CENSUS

### 2.1 WINDOWS

| Surface | Reach / trigger | User-visible functions |
|---|---|---|
| **Main window** (`Main.cpp:40`) | App launch; opens maximized to primary display, native title bar, resizable 1280×720–3840×2160 | Contains all main-window panels below. Global keyboard shortcuts + file-drop target (see §3). |
| **Native menu bar** (`MenuBarModel.cpp`, set `Main.cpp:58`) | Top of screen (macOS) / window (Win/Linux) | 9 menus (§2.5). |
| **OutputWindow** (`OutputWindow.h:69`, `OutputWindow.cpp:260`) | Menu Output→Fullscreen:display / TopBar+v1 Output combo / Cmd+F. Borderless, always-on-top, covers a chosen display; renders shared EffectChain in its own GL context | Escape closes (`OutputWindow.cpp:317`); closeButtonPressed hides (`:284`). No on-surface controls (pure output). Its OutputRenderer compiles ~80 shaders (`:157`) — a DUPLICATE, LAGGING shader list vs main Renderer. |
| **PreferencesDialog** (`PreferencesDialog.h:8`, `.cpp:7`) | Menu Audio-DNA→Preferences / →About. Modal DialogWindow, 8 tabs | See §2.6. |

### 2.2 MAIN-WINDOW PANELS — v2 (SHOWN)

Layout in `MainComponent::resized()` (`MainComponent.cpp:1279`): TopBar (top, 34px) →
SignalBar → row1 v1 toolbar (24px) → Deck (resizable via H-divider) → bottom row of
4 panels (Preview|Timing|Inspector|Browser) split by 3 draggable V-dividers → preset-slot
bar (28px, bottom). WaveformDisplay sits under Preview.

#### TopBar (`TopBar.h:12`, `TopBar.cpp`)
Trigger: always visible. Controls:
- Audio source ComboBox (Mic/File; wired `MainComponent.cpp:400`)
- Input gain ResettableSlider (`TopBar.cpp:20`)
- Transport: Play `>`, Pause `||`, Stop `[]` buttons (`TopBar.h:63`) — **not wired to callbacks** (no onClick set; decorative)
- Tap button (tap-tempo, `TopBar.cpp:47`), Resync button (`:74`)
- Manual BPM toggle + BPM TextEditor edit field (Enter to set, `:121`)
- 5 BPM multiplier buttons /4 /2 x1 x2 x4 (`:134`)
- Quantize ComboBox (Off/Next Beat/Next Downbeat, `:150`)
- Fade ResettableSlider (`:167`), Master level ResettableSlider (`:184`)
- Output display ComboBox (`:196`; wired `MainComponent.cpp:469`)
- Beat wheel (4-segment, display-only `:345`), Bar/Phrase readout (display `:401`), FPS/DSP labels

#### SignalBar (`SignalBar.h:15`, `SignalBar.cpp`)
Trigger: always visible (3 size modes: Minimized 26px / Normal 84px / Expanded fill).
- `[+]` add-signal button → PopupMenu of hidden signals (`SignalBar.cpp:179`)
- Shrink `▲` / Grow `▼` size buttons (`:30`)
- N × **SignalStrip** children (one per visible signal). Each: click = select for Signal
  inspector (`SignalStrip.cpp:78`); display-only meter (min/normal/expanded paint).

#### DeckView (`DeckView.h:15`, `DeckView.cpp`) — Resolume-style layer×column grid
Trigger: main content area. Scrollable Viewport. Contains:
- **Column trigger buttons** (top row, N buttons, `DeckView.cpp:291`) — click triggers column
- **Deck tab buttons** (bottom, one per deck, `:373`) — click switches deck
- **LayerStrip** per layer (§2.2.1)
- **ClipCell** per layer×column (§2.2.2)

##### 2.2.1 LayerStrip (`LayerStrip.h:23`, `LayerStrip.cpp`)
Per-layer header, ~15 operable controls:
- X clear, B bypass, S solo buttons (`LayerStrip.cpp:330-344`)
- Transport `<` `||` `>` `>|` buttons (`:356-379`)
- Speed vertical slider (S), Keying threshold slider (K), Opacity slider (V) (`:382-424`)
- Blend/keying ComboBox "V dropdown" — 13 keying modes + ~55 mix modes (`:855`, `populateMixModes :7`)
- Fade-speed slider (F) + Transition-mode ComboBox (caret-only) (`:451-475`)
- Name box click = select layer (`:706`); clip-name/transport bar click+drag = scrub playhead (`:709/721`)
- Right-click: **no context menu** (falls through, `:715`)

##### 2.2.2 ClipCell (`ClipCell.h:11`, `ClipCell.cpp`)
Two zones + drag/drop:
- Thumbnail-area click = trigger/retrigger clip (`ClipCell.cpp:185`)
- Name-bar click = select for inspector; Cmd/Shift+click = add to selection (`:190`)
- Name-bar drag = move clip to another cell (`clip:L:C` desc, `:196`)
- Right-click: **no-op** (returns early `:182`)
- Drop targets: files (image/video), internal `files:`, `fx:`, `source:`, `clip:`,
  `milkdrop:`, `milkdrop_playlist:` (`isInterestedInDragSource :365`; `itemDropped :392`).
  Multi-image→sequence, multi-video→sequential cells (`filesDropped :236`).

#### PreviewPanel (`PreviewPanel.h:12`, `.cpp`)
- Preview / Output tab buttons (`PreviewPanel.cpp:10`) — both render same GL host (labels only)
- GL host (display; "Load an image…" placeholder)

#### WaveformDisplay (`WaveformDisplay.h:14`)
Readout only (scrolling waveform). No controls.

#### TimingWindow (`TimingWindow.h:8`, `.cpp`)
- BPM / Routing / Oscillators tab buttons (`TimingWindow.cpp:14`)
- **All 3 tabs are EMPTY placeholders** — paint() only draws the tab name centered (`:51-65`).

#### InspectorPanel (`InspectorPanel.h:23`, `.cpp`) — 4-tab
- Tab buttons: Clip / Layer / Composition / Signal (`InspectorPanel.cpp:15`)
- Pin button (P24.11 — prevents auto-switch, `:25`)
- Auto-switches tab on clip/layer/signal selection unless pinned (`:133-149`)
- Each tab = scrollable Viewport wrapping one inspector (§2.3)

#### BrowserPanel (`BrowserPanel.h:16`, `.cpp`) — 6-tab
- Tab buttons: Files / FX / Sources / Comp/Decks / Record / MilkDrop (`BrowserPanel.cpp:15`)
- Each tab = one browser panel (§2.4)

#### Row1 v1 toolbar (`MainComponent.cpp:1350`, VISIBLE) + preset slots
Shown alongside v2 chrome:
- Open Image, Image Folder buttons; Beats-per-image ComboBox (`:334`)
- Save / Load (FX preset), FX Save, Deck Save, Deck Load buttons (`:25/70/180`); file label
- **10 preset slots** at bottom: each a numbered button (load slot) + a ComboBox (assign
  FX_Save file) (`:186-236`, layout `:1381`)

### 2.3 INSPECTOR TABS (inside InspectorPanel)

#### ClipInspector (`ClipInspector.h:25`, `.cpp`) — Clip tab
Sections top→bottom, all in one scrollable column:
- **Dashboard**: MacroPanel = 8 knobs + 8 source buttons (§2.2 sub) (`ClipInspector.cpp:5`)
- **Transport**: mode ComboBox (Timeline/BPM Sync); `◀ ⏸ ▶` buttons; Loop ComboBox
  (Loop/PingPong/OneShot); Trigger ComboBox (Restart/Continue/Relative); Speed slider +
  ÷2/×2 + Reverse; Duration slider + /2/×2 (`:7-124`)
- Conditional row: Images/Sec slider (image-seq) OR Beat-Division ComboBox + Content-Beats
  slider (BPM Sync) (`:176-274`)
- **Cuepoints**: 8 numbered trigger buttons (jump; Ctrl+click clears) + 8 "Set" buttons
  (`:277-330`)
- **Autopilot**: Action ComboBox (8), Duration ComboBox (7), Beat-Snap ComboBox (`:127-174`)
- **Source Parameters** (Source clips only): N UniversalParamControls (`buildSourceParamControls :764`)
- **Video**: Opacity UPC; Width/Height inc-dec sliders; Blend-mode ComboBox; Alpha-type
  ComboBox; R G B A channel toggles (`:332-388`)
- **Transform**: Position X/Y, Scale, Rotation, Anchor UPCs; P. button decorative (`:390-408`)
- **Effects**: EffectStackView (§2.2 sub)
- **Timeline bar**: interactive drag of In-point / Out-point / Playhead (`mouseDown :1063`,
  `mouseDrag :1094`); FX-drop target (`itemDropped :1237`)

#### LayerInspector (`LayerInspector.h:28`, `.cpp`) — Layer tab
- Editable **name label** (click to rename, `LayerInspector.cpp:726`)
- Dashboard (MacroPanel)
- **Autopilot**: 4 direction buttons `◀◀ OFF ▶▶ 🔀`; Trigger-mode ComboBox (End of
  Video/On Beat); Beat-count ComboBox; Loops inc-dec slider (`:31-142`)
- **Layer**: Master UPC; Persistent toggle; Ignore-Column-Trigger toggle (`:145-170`)
- **Video**: Blend-mode ComboBox (25); Opacity UPC; Width/Height sliders; Auto-Size ComboBox (5)
- **Transition**: Blend ComboBox (~55 modes); Duration slider (`:224-334`)
- **Keying** (Transparent type only): Mode ComboBox (13); Threshold + Softness sliders
- **Dry/Wet** (FX-Only type only): 1 slider
- **3D Controls** (ThreeD type only): Rot X/Y/Z, Speed, Scale sliders (`:365-383`)
- **Transform**: 5 UPCs; **Feedback**: Enable toggle + Preset ComboBox + 7 sliders
  (amount/scaleX/scaleY/rotation/offsetX/offsetY/lumaKey, `:406-488`)
- **Layer Effects**: EffectStackView; FX-drop target (`:994`)

#### CompositionInspector (`CompositionInspector.h:23`, `.cpp`) — Composition tab
- Dashboard (MacroPanel)
- **Autopilot**: 4 direction buttons; Duration ComboBox (3); Clip-Loops slider; Loop
  toggle; Master-Layer ComboBox (Off + 8) (`:8-81`)
- **Per-Type Autopilot** (P20): enable toggle; Opaque/Transparent/Effect cycle sliders;
  Transparent/Effect randomize toggles (`:84-132`)
- **Composition**: Master + Speed UPCs; **Video**: Opacity UPC; **Transform**: 5 UPCs
- **Global Effects**: EffectStackView; **Output**: Resolution ComboBox (4) (`:184-200`)
- Section collapse triangles + P. button are **decorative only** (`:457` "always expanded")

#### SignalInspector (`SignalInspector.h:15`, `.cpp`) — Signal tab
Type-adaptive:
- Audio: Threshold / Gain / Falloff sliders (`SignalInspector.cpp:19-24`)
- Oscillator: Wave-shape ComboBox (5); Beat-duration ComboBox (6); Amplitude + Phase sliders
- Envelope: Curve-type ComboBox (3); Beat-duration ComboBox (5); Amplitude + Phase sliders;
  Looping + One-Shot toggles; **curve editor DISPLAY-ONLY** — no mouse handlers
  (`paintCurveEditor :365`; contrast doc claim of "draggable points").

### 2.4 BROWSER TABS (inside BrowserPanel)

| Tab | Class | Controls |
|---|---|---|
| **Files** | `FilesBrowser.cpp` | Up `^` button; editable path bar; search field; Grid/List toggle buttons; file grid: click=select, dbl-click=activate/load, drag=`files:` to cell, **right-click=toggle favorite** (`:33`), directory click=navigate |
| **FX** | `FXBrowser.cpp` | Search field; 11 collapsible category headers (`buildCategoryList :396`); effect rows: click select, Cmd/Shift multi-select, drag=`fx:name,name` to cell/stack (`:220`) |
| **Sources** | `SourcesBrowser.cpp` | Search box; 18 category headers; **103 source rows** (`buildSourceList :317`, 103 push_backs); click/multi-select; drag=`source:id,id`. NOTE: categories "Simulation" & "Routing" have zero sources → never render |
| **Comp/Decks** | `CompDecksBrowser.cpp` | Save-Composition + Save-Deck buttons; 2 collapsible sections; entry rows: click=load, **right-click=delete file** (`:98/:131`) |
| **Record** | `RecordPanel.cpp` | Record / Stop / Play / Save / Load / Output-Folder buttons; Format ComboBox (JSON/Video-future); status + event-count labels |
| **MilkDrop** | `MilkDropBrowser.cpp` | 4 sub-tab buttons (Curated/Favorites/Recent/All); search (All tab); Prev `<` / Next `>` / Random `?` / Lock nav buttons; 3 play-mode buttons (Jukebox/VJ Clip/Playlist); **Jukebox**: Play + Pool/Mode/Timing ComboBoxes + Blend slider; **Playlist**: Cycle/Timing ComboBoxes + Blend slider; preset rows: click=preview/load, Shift/Cmd multi-select (Playlist mode), drag=`milkdrop:`/`milkdrop_playlist:`, right-click=toggle favorite (`:46`) |

### 2.5 MENU BAR (`MenuBarModel.cpp`) — 9 menus

Audio-DNA (Preferences, Import ISF, About, Quit) | Composition (Undo, Redo, New, Open,
Save, Save As, Copy/Paste Global Effects, Collect Media, Relocate Missing Files) | Deck
(New, Insert Before/After, Duplicate, Rename, Clear Clips, Close, Remove) | Layer (New,
Insert Above/Below, Duplicate, Rename, Copy/Paste Effects, Clear Clips, Remove, Ignore
Column Trigger, Lock Content, Fold, Move Up/Down) | Column (New, Insert Before/After,
Duplicate, Clear Clips, Remove, Remove All Before/After) | Clip (Select All, Cut, Copy,
Paste, Copy/Paste Effects, Rename, Clear, Show in Finder, New Source, New Effect, Replace
Content, Lock Content) | Output (Disabled, per-display Fullscreen, Windowed, Identify
Displays, Test Card, Snapshot, Start/Stop Recording) | Shortcuts (Edit Keyboard, Edit
MIDI, Stop All, Export/Import Bindings) | View (Signal Bar, Deck, Preview, Inspector,
Browser, Timing Window, FPS and Stats, Programming Mode, Save/Load/Reset Layout).

Handled in `MainComponent::handleMenuCommand` (`:2723`). **~35 items fall to the default
`DBG("not yet implemented")` stub** (`:3273`): all Deck Insert/Dup/Rename/Close; Layer
Dup/Rename/Copy-Paste-Fx/IgnoreCol/Lock; Column Dup/ClearClips/RemoveAllBefore-After;
all Clip Select/Cut/Copy/Paste/CopyFx/PasteFx/Rename/ShowInFinder/NewSource/NewEffect;
Comp Copy/Paste Effects; Output Windowed/Identify/TestCard; View Signal Bar/Deck/Preview/
Inspector/Browser/Timing/FPS (only Programming Mode + Save/Load/Reset Layout work).

### 2.6 PREFERENCES DIALOG (`PreferencesDialog.cpp`) — 8 tabs

- Tab buttons: General / Audio / Video / MIDI / Recording / Defaults / Feedback / About (`:49`)
- **General**: Confirm-on-quit toggle (no handler), Show-Tooltips toggle (wired → onTooltipToggled `:66`)
- **Audio**: Sample-Rate / Buffer-Size / BPM-Range ComboBoxes — **no onChange handlers** (inert)
- **Video**: FPS-Target / Render-Resolution ComboBoxes (inert); MilkDrop preset dir edit +
  Browse button (Browse wired `:123`)
- **About**: version + credits labels
- **MIDI / Recording / Defaults / Feedback**: **empty placeholders** — `layoutPlaceholderTab`
  is a no-op (`:378`) and paint() draws nothing for them.

### 2.7 OVERLAYS / POPUPS / POPUP-MENUS / DIALOGS

| Surface | Reach | Controls |
|---|---|---|
| **BindingOverlay** (`BindingOverlay.cpp`) | Shortcuts→Edit Keyboard / Shift+Cmd+K (`MainComponent.cpp:1614`) | Fullscreen; click a highlighted target then press a key to bind; Escape exits (`:149`). Targets built in `buildBindableTargets :3461` (global actions + column/clip/layer/deck targets) |
| **MidiLearnOverlay** (`MidiLearnOverlay.cpp`) | Shortcuts→Edit MIDI / Shift+Cmd+M (`:1621`) | Same workflow; listens for MIDI note/CC; Escape exits |
| **MappingEditor** (`MappingEditor.cpp`) | Only via EffectsRackPanel map buttons — **UNREACHABLE in v2** (rack hidden) | Source ComboBox (50), Curve ComboBox (24), In/Out Min/Max + Smoothing sliders, Enabled toggle, Randomize / Delete / Close `X` buttons |
| **UniversalParamControl source picker** (`UniversalParamControl.cpp:311`) | Click the connect triangle or expanded source button on any inspector param | Cascading PopupMenu: Manual / Audio signals / BPM Sync (4 shapes × 6 divisions) / Oscillators / Envelopes / Clip Position / Timeline / Macros (8). Also: right-click any param = reset to default (`:246`); [-]/[+] nudge; Invert toggle + Range min/max sliders when expanded |
| **MacroPanel source picker** (`MacroPanel.cpp:109`) | Click a dashboard knob's source button | PopupMenu: Manual + Signals submenu |
| **SignalBar add-signal menu** (`SignalBar.cpp:179`) | `[+]` button | PopupMenu of currently-hidden signals |
| **ProgrammingMode overlay** (`ProgrammingMode.cpp`) | **ORPHANED** — `setActive()` never called; only ever setVisible(false) | Header label + Exit button (never seen) |
| **Native FileChoosers** (~15) | Open Image (`:1512`), Image Folder (`:2154`), Save/Load Preset (`:1535/1564`), Save/Load Deck (`:1960/2014`), Collect Media (`:2777`), Relocate Files (`:2835`), Replace Content (`:3042`), Export/Import Bindings (`:3171/3186`), Save/Load Layout (`:3218/3240`), Record Save/Load, MilkDrop dir, Audio file | Native OS dialogs |
| **AlertWindow** | ISF import success/failure (`:2418/2450`) | OK |

---

## 3. GLOBAL SYSTEMS

**Keyboard shortcuts** (`MainComponent::keyPressed :1591`): Shift+Cmd+I inspector
(if built), Shift+Cmd+K keyboard-bind, Shift+Cmd+M MIDI-learn, Escape close-output,
Cmd+Z undo / Cmd+Shift+Z redo, Cmd+S save preset, Cmd+F toggle fullscreen output,
Cmd+O load preset. Non-Cmd keys → BindingManager (user bindings). Key-up → momentary
bindings (`keyStateChanged :1698`).

**Drag-and-drop matrix** (MainComponent is `DragAndDropContainer` + `FileDragAndDropTarget`):
- Finder files → main window (`filesDropped :1740`, audio→engine, image→preview) and → ClipCell
- FXBrowser `fx:` → ClipCell / EffectStackView (Clip/Layer/Composition inspectors) / ClipInspector / LayerInspector
- SourcesBrowser `source:` → ClipCell
- FilesBrowser `files:` → ClipCell
- MilkDropBrowser `milkdrop:` / `milkdrop_playlist:` → ClipCell
- ClipCell `clip:` → ClipCell (move/swap)
- DragAndDropTarget implementers: ClipCell, ClipInspector, LayerInspector, EffectStackView
  (CLAUDE.md:457 claim CONFIRMED). CompositionInspector is not a target, but its embedded
  EffectStackView is.

**Tooltip system**: `juce::TooltipWindow` (600ms) in MainComponent (`:380`); `setTooltip`
used in TopBar, ClipInspector, LayerInspector; toggle in Preferences→General. Coverage
is sparse (3 files).

**LookAndFeel / theme** (`LookAndFeel.h/.cpp`): single `AudioDNALookAndFeel` dark theme
(cyan/magenta accents) set on MainComponent. LayerStrip uses 7 additional inline LAF
subclasses for its flat sliders/combos (`LayerStrip.cpp:94-311`).

**Undo/redo reach**: `undoManager_` wired to Cmd+Z/Cmd+Shift+Z and Composition→Undo/Redo
only. No panel pushes undo transactions — undo has effectively no populated stack from UI edits.

---

## 4. DOC-VERIFY

| Doc:line | Claim | Code truth |
|---|---|---|
| CLAUDE.md:248 & :252 | `SpectrumDisplay.h/cpp` listed | **Duplicate row** — appears twice in source tree |
| CLAUDE.md:253 & :256 | `Knob.h/cpp` listed | **Duplicate row** — appears twice |
| CLAUDE.md:257-258 | `KeyboardPanel.h/cpp`, `KeyEditor.h/cpp` [M7] | **Nonexistent** — no such files in src/ui (verified full dir listing) |
| CLAUDE.md:244-258 (ui/ tree) | Lists ~14 ui files | **Grossly incomplete** — actual src/ui has 38 .h/.cpp pairs. Omits TopBar, SignalBar, SignalStrip, DeckView, LayerStrip, ClipCell, InspectorPanel + 4 inspectors, BrowserPanel + 6 browsers, RecordPanel, EffectStackView, UniversalParamControl, MacroPanel, BindingOverlay, MidiLearnOverlay, MenuBarModel, PreferencesDialog, ProgrammingMode, TimingWindow, CompDecksBrowser, MilkDropBrowser |
| CLAUDE.md:483 | ResettableSlider rule: ALL sliders MUST be ResettableSlider "everywhere" | **Violated**: MilkDropBrowser jukeboxBlendSlider_/playlistBlendSlider_ are plain `juce::Slider` (`MilkDropBrowser.h:98,105`); MainComponent v1 masterLevelSlider_/inputGainSlider_ are `juce::Slider` (`MainComponent.h:130,189`) |
| CLAUDE.md:491 | PopupMenu rule: always `.withParentComponent(getTopLevelComponent())` | **Violated**: MacroPanel (`MacroPanel.cpp:127`) and SignalBar (`SignalBar.cpp:200`) call showMenuAsync with only `.withTargetComponent(...)`, no withParentComponent. Only UniversalParamControl follows the rule (`:324`) |
| CLAUDE.md:455-457 | FX drag-and-drop: EffectStackView row = [B]/name/[X], expand→sliders, right-click reset; FXBrowser `startDragging("fx:effectName")`; 4 DragAndDropTargets | **Accurate** (EffectStackView.cpp; FXBrowser.cpp:239 uses `fx:` comma-joined) |
| .harmony/FEATURES.md:798-800 (7b) | SignalInspector envelope = "curve editor with draggable control points" | **False** — SignalInspector has no mouse handlers; curve editor is paint-only (`SignalInspector.cpp:365`). Same overclaim in SignalInspector.h:11 header comment |
| .harmony/FEATURES.md:301-303 (4a) | FX Browser = "135 effects organized by 11 categories" | **11 categories CONFIRMED** (`FXBrowser.cpp:396` builds 11). Effect count (135) is L2's lane. FXBrowser.h:8 header comment still says "76+ effects…(8 categories)" — stale source comment |
| docs/FEATURE_INVENTORY.md:538 | "9 menus, ~70 items", IDs 1000-1852 | **Accurate** (9 menus; ~80 static items + dynamic displays) |
| docs/FEATURE_INVENTORY.md:552 / :634 | Preferences 8 tabs, 5 placeholders, most controls unwired | **Accurate** — MIDI/Recording/Defaults/Feedback empty; Audio/Video combos inert |
| docs/FEATURE_INVENTORY.md:565-566 / :731-732 | ProgrammingMode permanently hidden; TimingWindow 3 empty tabs | **Accurate** (confirmed §5) |
| docs/FEATURE_INVENTORY.md:288 | "Two parallel Output selectors (v1 hidden + TopBar v2)"; OutputWindow shaders lag | **Accurate** — v1 displaySelector_ hidden, TopBar displaySelector_ live; both wired to closeOutput/openOutputOnDisplay |
| ClipInspector.h:18 / LayerInspector.h:15 / CompositionInspector.h:14 | name bar has "search + gear icons" | **Not implemented** — paint() draws only the name; no such controls exist |
| SourcesBrowser.h:8 | "10 Tier 1 generators for launch" | **Stale** — browser lists 103 source entries across 18 categories |

---

## 5. HEALTH READ — WORKS vs DOESN'T / FLAGGED

### WORKS (solid, wired)
- Deck grid: clip trigger/select/drag-drop/move, column & deck switching, layer strip
  controls — all wired end-to-end (`DeckView`/`LayerStrip`/`ClipCell` → MainComponent handlers).
- 4-tab Inspector with auto-switch + pin; per-param UniversalParamControl signal routing
  (source picker, right-click reset, [-]/[+], range/invert).
- 6-tab Browser: Files nav/search/fav/drag, FX/Sources/MilkDrop drag-to-cell, Comp/Decks
  save-load-delete, Record start/stop/save/load.
- TopBar tempo (tap/resync/manual/multiplier/quantize), audio-source + output + gain +
  master + fade. SignalBar meters + resize + add-signal.
- BindingOverlay + MidiLearnOverlay full click-target→bind workflow; keyboard shortcuts;
  OutputWindow fullscreen; ISF import; layout save/load.

### DOESN'T WORK / ORPHANED — FLAGGED
1. **EffectsRackPanel — built but permanently hidden.** addAndMakeVisible in ctor
   (`MainComponent.cpp:377`) but resized() only ever `setVisible(false)` (`:1401`, `:1322`);
   no setVisible(true) anywhere. Its Knobs, per-effect lock/randomize, and the whole
   Effects-Rack "Random" button are unreachable in v2.
2. **MappingEditor — unreachable.** Only opened from EffectsRackPanel map buttons
   (`EffectsRackPanel.cpp:498`); since the rack is hidden, the MappingEditor popup can
   never be shown. (v2 replaced it with per-param routing per CLAUDE.md:562.)
3. **AudioReadoutPanel + SpectrumDisplay — built but permanently hidden** (`:1399-1400`,
   `:1319-1321`). The left feature-readout panel and 7-band spectrum never render in v2.
4. **ProgrammingMode overlay — orphaned.** `setActive()` never called; only setVisible(false)
   (`:1313`). View→Programming Mode instead just toggles SignalBar to Expanded size
   (`:3199`); the ProgrammingMode header/Exit-button component is dead.
5. **~35 menu-bar items are no-op stubs** (default DBG, `:3273`) — most Deck/Layer/Column/
   Clip edit ops and all View panel-toggle items do nothing.
6. **Preferences: 4 of 8 tabs empty + most controls inert** — only tooltip toggle and
   MilkDrop-dir Browse actually do anything; sample-rate/buffer/BPM/FPS/render-res never
   applied or persisted.
7. **TimingWindow: 3 tabs are empty placeholders** (no content or controls).
8. **TopBar transport buttons (Play/Pause/Stop) not wired** — declared, added, but no
   onClick handler; clicking does nothing (`TopBar.h:63`, no wiring in TopBar.cpp).
9. **v1/v2 duplicate controls coexist** — row1 toolbar (Open Image/Folder, Save/Load, FX
   Save, Deck Save/Load) + 10 preset slots are shown simultaneously with the v2 TopBar,
   which duplicates audio-source/output/master. Two parallel control sets for the same
   functions.
10. **SourcesBrowser dead categories** — "Simulation" and "Routing" categories defined
    (`SourcesBrowser.cpp:339-340`) with zero assigned sources → headers never render.
11. **Inspector section P. buttons + collapse triangles are decorative** (non-interactive);
    inspector name-bar "search + gear" from header comments don't exist.

---

## 6. CENSUS COMPLETENESS

All 38 src/ui .h/.cpp pairs + MainComponent.h/.cpp read in full. Display-only (no
operable controls): AudioReadoutPanel, SpectrumDisplay, WaveformDisplay (Timer+paint only).
Knob = ResettableSlider + name/value labels (interactive only as MacroPanel/EffectsRack sub).
