# Slice 06: Browsers, Deck Grid, Clip Cells, Layer Strips, Signal Bar/Strip, Effect Stack

Audit scope: `src/ui/BrowserPanel.*`, `FilesBrowser.*`, `FXBrowser.*`, `SourcesBrowser.*`, `CompDecksBrowser.*`, `MilkDropBrowser.*`, `RecordPanel.*`, `DeckView.*`, `ClipCell.*`, `LayerStrip.*`, `SignalBar.*`, `SignalStrip.*`, `EffectStackView.*`, `EffectsRackPanel.*`.

---

## Section 12: Browser / Library

### 12.1 Tab Structure

**Top-level browser panel** — `src/ui/BrowserPanel.h:39` defines `enum Tab { Files=0, FX=1, Sources=2, CompDecks=3, Record=4, MilkDrop=5 }`. Six tabs total, rendered as a horizontal tab bar 26px tall (`BrowserPanel.h:62 kTabBarHeight=26`). Tab buttons constructed at `BrowserPanel.h:47-52`. Tab bar layout: 6 equal-width buttons (`BrowserPanel.cpp:70-76`). Active tab gets a 3px cyan underline (`BrowserPanel.cpp:59-61`); active tab color `0xff3a3a5c` white text, inactive `0xff1a1a2e` dim text (`BrowserPanel.cpp:118-129`). One tab is shown at a time via `showActiveTab()` (`BrowserPanel.cpp:140-148`).

**Sub-tabs** only exist inside:
- **MilkDropBrowser**: 4 sub-tabs (Curated, Favorites, Recent, All) + 3 play modes (Jukebox, VJ Clip, Playlist).
- **FilesBrowser**: grid/list view toggle (not a sub-tab but a view mode).
- **FXBrowser**: 11 collapsible category sections (not sub-tabs).
- **SourcesBrowser**: 18 collapsible category sections.
- **CompDecksBrowser**: 2 collapsible sections (Compositions / Decks).
- **RecordPanel**: single panel, no sub-tabs.

### 12.2 Files Browser — Controls, Filters, Drag-Drop, Right-Click

**File**: `src/ui/FilesBrowser.h`, `src/ui/FilesBrowser.cpp`.

Navigation controls:
- `upButton_{"^"}` — `FilesBrowser.h:35`; click goes to parent directory (`FilesBrowser.cpp:329-332`). 24px wide (`FilesBrowser.cpp:377`).
- `pathBar_` (`juce::TextEditor`, `FilesBrowser.h:36`) — shows current path; pressing Enter navigates to typed path (`FilesBrowser.cpp:339-344`).
- `searchField_` (`juce::TextEditor`, `FilesBrowser.h:37`) — placeholder "Search..." (`FilesBrowser.cpp:352`); live filter on text change (`FilesBrowser.cpp:353` → `filterBySearch()` at `FilesBrowser.cpp:443-479`).

View toggle:
- `gridViewBtn_{"Grid"}` — `FilesBrowser.h:41`, wired at `FilesBrowser.cpp:358`, switches to grid mode.
- `listViewBtn_{"List"}` — `FilesBrowser.h:42`, wired at `FilesBrowser.cpp:359`.

File list:
- `FileEntry` struct — `FilesBrowser.h:45-51`: file, thumbnail, isDirectory, isFavorite.
- Grid view shows 64px thumbnails with folder icon (U+1F4C1) for directories and name label below (`FilesBrowser.cpp:188-249`).
- List view: 20px alternating-striped rows with "D" icon for directory, "*" for favorite (`FilesBrowser.cpp:252-292`).
- Home directory is default start (`FilesBrowser.cpp:362`).

Supported media file extensions (`FilesBrowser.cpp:481-489`): png, jpg, jpeg, gif, bmp, tiff, tif, wav, aiff, aif, mp3, flac, ogg, avi, mov, mp4. Hidden files (starting with `.`) are skipped.

Multi-selection:
- Cmd+click toggles individual selection (`FilesBrowser.cpp:60-67`).
- Shift+click extends range from last anchor (`FilesBrowser.cpp:68-77`).
- Plain click replaces selection (`FilesBrowser.cpp:78-83`).
- Double-click activates `onFileActivated` callback (`FilesBrowser.cpp:87-95`).

**Right-click behavior** (`FilesBrowser.cpp:33-41`): right-click on a file entry toggles favorite status (no popup menu). Favorites are saved to `~/Application Support/AudioDNA/browser_favorites.txt` (`FilesBrowser.cpp:504-522`).

**Drag-drop source**: drags all selected files as `"files:path1|path2|path3"` description (`FilesBrowser.cpp:119-124`). Drag threshold 5px (`FilesBrowser.cpp:102-105`). Drag image: 140×24 label with count or filename (`FilesBrowser.cpp:131-148`).

**Drag-drop target**: `FileDragAndDropTarget` interface implemented but rejects all drags (`FilesBrowser.h:29-30`, `isInterestedInFileDrag` returns false).

### 12.3 FX Browser — Categories, Multi-Select, Drag-Drop

**File**: `src/ui/FXBrowser.h`, `src/ui/FXBrowser.cpp`.

**11 categories** (`FXBrowser.cpp:396-412` in `buildCategoryList()`):

| # | Internal | Display Name | Color |
|---|----------|--------------|-------|
| 1 | warp | Warp | 0xff4fc3f7 (cyan) |
| 2 | color | Color | 0xffff7043 (orange) |
| 3 | glitch | Glitch | 0xffab47bc (purple) |
| 4 | blur | Blur / Post | 0xff66bb6a (green) |
| 5 | 3d | 3D / Depth | 0xffffca28 (yellow) |
| 6 | pattern | Pattern | 0xff26c6da (teal) |
| 7 | animation | Animation | 0xffef5350 (red) |
| 8 | blend | Blend | 0xff8d6e63 (brown) |
| 9 | time | Time | 0xff00897b (teal) |
| 10 | composite | Composite | 0xffec407a (pink) |
| 11 | audio | Audio | 0xffffd54f (gold) |

Controls:
- `searchField_` (`FXBrowser.h:58`) — placeholder "Search effects...", filters live (`FXBrowser.cpp:321-333`).
- 22px category headers (`FXBrowser.h:64 kCategoryHeaderHeight=22`), 24px effect rows (`FXBrowser.h:65 kEffectRowHeight=24`).
- Category header click toggles expand/collapse (`FXBrowser.cpp:145-156` via `toggleCategory()` at `FXBrowser.cpp:414-427`).
- Search mode auto-expands all categories with matches (`FXBrowser.cpp:65,112`).
- Header shows category count: `name (N)` (`FXBrowser.cpp:58`).

Multi-select:
- Cmd+click toggles individual (`FXBrowser.cpp:164-172`).
- Shift+click selects visible range from last anchor (`FXBrowser.cpp:173-201`).
- Plain click replaces (`FXBrowser.cpp:202-208`).

**Drag-drop format**: `"fx:Name1,Name2,Name3"` — comma-separated effect names (`FXBrowser.cpp:233-240`). Drag image: 140×24 purple-bordered (`0xff8866cc`) (`FXBrowser.cpp:246-262`). Drag threshold 5px (`FXBrowser.cpp:225`).

**No right-click menu** on FX browser rows.

### 12.4 Sources Browser

**File**: `src/ui/SourcesBrowser.h`, `src/ui/SourcesBrowser.cpp`.

**18 categories** (`SourcesBrowser.cpp:323-340`):
Input, Fractal, Noise, Geometric, Audio-Visual, Nature, 3D, Organic, Pattern, Utility, Wireframe, Lines, Math, Lighting, Text, Particle, Simulation, Routing.

**~96 sources registered** across these categories (`SourcesBrowser.cpp:343-482` — enumerated list). Notables:

- **Input**: Camera Input.
- **Fractal (2D)**: Mandelbrot/Julia, Kaleidoscopic, Julia Set, Burning Ship, Newton, Sierpinski, Apollonian.
- **3D Fractals**: Mandelbulb, Menger Sponge, KIFS, Julia Set 3D, Burning Ship 3D, Newton 3D, Sierpinski Tetra, Apollonian 3D.
- **3D Torus**: Striped/Checker/Twisted Torus, Spiral Vortex, Ribbed Vortex, Wormhole, Wormhole Tunnel, Torus Hole.
- **Audio-Visual**: Audio Waveform, Spectrum Landscape, Chromatic Ring, Band Tower, Timbral Nebula, Structural Landscape, Cymatics, Spectral Waterfall, Spectral Ring.
- **Wireframe**: Sphere, Torus, Cube, Cylinder, Cone, Icosahedron, Wolf.
- **Lines**: Line Generator, Zigzag, Star Burst, Polygon, Waveform, Lissajous, Spirograph, Angular Grid, Fractal Tree, Laser Scan, Moire.
- **Text**: Scrolling Text Wall.
- **Particle**: Lightning Storm, Starfield, Particle Nebula.
- **Simulation** (stateful): absent in enumerated list but mentioned in CLAUDE.md — may be registered elsewhere.

Controls:
- `searchBox_` (`SourcesBrowser.h:41`) — placeholder "Search sources..." (`SourcesBrowser.cpp:273`).
- 22px category headers, 28px source rows (`SourcesBrowser.h:53-54`).
- Color indicator 16×16px rounded rect on each row (`SourcesBrowser.cpp:54-56`).
- Category expand/collapse via header click (`SourcesBrowser.cpp:118-125` → `toggleCategory()` at `SourcesBrowser.cpp:484-496`).

Multi-select: same pattern as FX (Cmd=toggle, Shift=range, plain=replace) at `SourcesBrowser.cpp:135-175`.

**Drag-drop format**: `"source:id1,id2,id3"` (`SourcesBrowser.cpp:201-209`). Drag image: 140×24 lavender-bordered (`0xffbb88ff`) (`SourcesBrowser.cpp:215-232`).

**No right-click menu** on Sources browser rows.

### 12.5 Comp-Decks Browser

**File**: `src/ui/CompDecksBrowser.h`, `src/ui/CompDecksBrowser.cpp`.

Two fixed sections:
- **Compositions** (accent cyan), stored in `~/Application Support/AudioDNA/compositions/*.json` (`CompDecksBrowser.cpp:274-278`).
- **Decks** (accent magenta), stored in `~/Application Support/AudioDNA/decks/*.json` (`CompDecksBrowser.cpp:280-284`).

Entry struct (`CompDecksBrowser.h:34-39`): `file`, `name`, `dateStr`.

Controls (`CompDecksBrowser.h:48-49`):
- `saveCompBtn_{"Save Composition"}` — fires `onCompositionSave` callback (`CompDecksBrowser.cpp:184-186`).
- `saveDeckBtn_{"Save Deck"}` — directly writes active deck to file as JSON (`CompDecksBrowser.cpp:188-200`).

Section layout:
- 28px button bar (`CompDecksBrowser.h:61 kButtonBarHeight`).
- 22px section headers with accent color strip, expand arrow `v/>`, name + count (`CompDecksBrowser.cpp:36-48`).
- 24px entry rows with name left + last-modified date right (`CompDecksBrowser.cpp:62-69`).

Mouse handling (`CompDecksBrowser.cpp:78-146`):
- Click section header → toggle expand (`CompDecksBrowser.cpp:83-90, 116-122`).
- Left-click entry → fires `onCompositionLoad` or `onDeckLoad` callback.
- **Right-click entry** → delete the file (destructive, no confirmation) (`CompDecksBrowser.cpp:98-104, 131-136`).

No search, no multi-select, no drag-drop source/target. Scan directory via `scanForFiles()` (`CompDecksBrowser.cpp:236-272`).

### 12.6 MilkDrop Browser

**File**: `src/ui/MilkDropBrowser.h`, `src/ui/MilkDropBrowser.cpp`.

**4 sub-tabs** (`MilkDropBrowser.h:61`): Curated, Favorites, Recent, All. Buttons at `MilkDropBrowser.h:69-72`. Layout: equal-width 24px tall (`MilkDropBrowser.h:133 kSubTabHeight=24`).

**3 play modes** (`MilkDropBrowser.h:65`): Jukebox, VJClip, Playlist. Buttons at `MilkDropBrowser.h:88-90`, 24px tall (`MilkDropBrowser.h:138 kModeBarHeight=24`).

**Search box** (`MilkDropBrowser.h:75`) — placeholder "Search presets..." (`MilkDropBrowser.cpp:392`); only visible in All sub-tab (`MilkDropBrowser.cpp:605-607`).

**Navigation bar** (26px, `MilkDropBrowser.h:137 kNavBarHeight`): four equal-width buttons:
- `prevBtn_{"<"}` → `presetManager_->prevPreset()` (`MilkDropBrowser.cpp:416-420`).
- `nextBtn_{">"}` → `presetManager_->nextPreset()` (`MilkDropBrowser.cpp:421-425`).
- `randomBtn_{"?"}` → `presetManager_->randomPreset()` (`MilkDropBrowser.cpp:426-430`).
- `lockBtn_{"Lock"}` → toggles `presetSelector_->setEnabled()`, button text flips to "Locked" in red (`MilkDropBrowser.cpp:431-439`).

**Jukebox mode controls** (`MilkDropBrowser.h:92-99`):
- `jukeboxPlayBtn_` — Play/Stop toggle, green when idle / red when playing (`MilkDropBrowser.cpp:846-849`).
- `jukeboxPoolSelector_` ComboBox: All (1), Curated (2), Favorites (3) (`MilkDropBrowser.cpp:463-467`).
- `jukeboxModeSelector_` ComboBox: Bag, Random, Sequential (`MilkDropBrowser.cpp:470-475`).
- `jukeboxTimingSelector_` ComboBox: 4 beats, 8 beats, 16 beats, 32 beats, 30 sec, 60 sec (`MilkDropBrowser.cpp:477-491`).
- `jukeboxBlendSlider_` (0.5–5.0s, default 2.0, suffix "s") — transition blend time (`MilkDropBrowser.cpp:498-508`).

**Playlist mode controls** (`MilkDropBrowser.h:101-105`):
- `playlistInfoLabel_` — shows "N selected - drag to cell" (`MilkDropBrowser.cpp:670-676`).
- `playlistCycleSelector_` ComboBox: Bag, Random, Sequential (`MilkDropBrowser.cpp:515-519`).
- `playlistTimingSelector_` ComboBox: 4/8/16/32 beats (`MilkDropBrowser.cpp:522-527`).
- `playlistBlendSlider_` (0.3–3.0s, default 1.5) (`MilkDropBrowser.cpp:534-544`).

**VJClip mode**: no controls — the preset list IS the interface, click to preview, drag single to cell (`MilkDropBrowser.cpp:661-666`).

**6 Mood Sections** (`MilkDropBrowser.cpp:729-736` in `initSections()`): Energetic (red), Psychedelic (magenta), Geometric (teal), Calm (blue), Dark (dark-purple), Minimal (gray). Preset rows 20px tall (`MilkDropBrowser.h:136 kPresetRowHeight=20`) with favorite star, mood dot, preset name (`MilkDropBrowser.cpp:119-150`).

Mouse interaction (`MilkDropBrowser.cpp:39-59`):
- **Right-click on preset** → toggles favorite via `presetManager_->toggleFavorite(idx)` (`MilkDropBrowser.cpp:46-51`).
- Left-click on preset → single-select (non-playlist mode) or multi-toggle (Playlist mode) (`MilkDropBrowser.cpp:792-823`).

**Drag-drop formats**:
- Single: `"milkdrop:path/to/preset.milk"` (`MilkDropBrowser.cpp:101`, parsed at `MilkDropBrowser.cpp:884-889`).
- Playlist: `"milkdrop_playlist:path1|path2|path3"` (`MilkDropBrowser.cpp:79-84`, parsed at `MilkDropBrowser.cpp:891-904`).

Recent presets: deque max 20 entries (`MilkDropBrowser.h:130 kMaxRecent=20`), added via `addToRecent()` (`MilkDropBrowser.cpp:715-725`). Persisted in memory only.

### 12.7 Record Panel

**File**: `src/ui/RecordPanel.h`, `src/ui/RecordPanel.cpp`.

Records session events (parameter changes + clip triggers) as timestamped JSON.

**Buttons** (`RecordPanel.h:37-42`):
- `recordBtn_{"Record"}` — dark-red bg (`0xff442222`); starts recording (`RecordPanel.cpp:9-17`). Disabled while recording.
- `stopBtn_{"Stop"}` — stops and shows N events count (`RecordPanel.cpp:21-31`). Disabled when not recording.
- `playBtn_{"Play"}` — plays back recorded events (`RecordPanel.cpp:34-43`). Green status while playing.
- `saveBtn_{"Save"}` — launches FileChooser for `*.json` save (`RecordPanel.cpp:47-65`).
- `loadBtn_{"Load"}` — launches FileChooser for `*.json` open (`RecordPanel.cpp:69-86`).
- `browseOutputBtn_{"Output Folder..."}` — choose output directory (`RecordPanel.cpp:108-119`).

**Controls**:
- `formatSelector_` ComboBox: "JSON Events" (1), "Video (Future)" (2, disabled placeholder) (`RecordPanel.cpp:90-93`).
- `statusLabel_` — "Ready" / "Recording..." (red) / "Playing..." (green) / "Stopped (N events)" / "Saved: filename" / "Loaded: ..." (`RecordPanel.cpp:97-99`).
- `eventCountLabel_` — "N events, X.X s" updated via `refresh()` (`RecordPanel.cpp:127-146`).
- `outputDirLabel_` — displays directory name, "(default)" if none (`RecordPanel.cpp:122-124`).

Layout (`RecordPanel.cpp:153-192`):
- Row 1: status label (14px).
- Row 2: [Record 56px][Stop 44px][Play 44px][Save 44px][Load 44px] (28px tall).
- Row 3: event count label.
- Row 4: format selector (180px wide).
- Row 5: [Output Folder... 120px][output dir label fills remainder].

No right-click menu, no drag-drop, no search.

---

## Deck UI

### DeckView Layout

**File**: `src/ui/DeckView.h`, `src/ui/DeckView.cpp`.

Layout dimensions (`DeckView.h:86-92`):
- `kLayerStripWidth = 250` — layer strip on the left.
- `kColumnTriggerHeight = 22` — column trigger row at top.
- `kCellWidth = 90`.
- `kCellHeight = 96` — standard 3-row layer.
- `kDeckTabHeight = 24` — deck tabs at bottom.
- `kCellGap = 0` (flush, borders drawn by cells).
- Folded layer height `kFoldedHeight = 22` (`DeckView.cpp:239, 256`).

Structure — 4 elements:
1. **Column trigger row** at top (`DeckView.cpp:26-33`) — one `juce::TextButton` per column showing `"1", "2", ...` — firing `onColumnTriggered(col)` (`DeckView.cpp:306-309`). Active column highlight via `refresh()` setting color `0xff3a5a4a` (`DeckView.cpp:207-213`).
2. **Grid viewport** with vertical + horizontal scroll (`DeckView.cpp:9-11`).
3. **Layer strips + clip cells** inside viewport. Layers rendered **top-to-bottom in REVERSE order** (highest layer index at top of screen) (`DeckView.cpp:89-92`).
4. **Deck tabs row** at bottom, 100px per tab, 2px gap (`DeckView.cpp:49-54`). Active deck highlighted same as column (`DeckView.cpp:216-222`).

Grid rebuild (`DeckView.cpp:66-177`) — called on `setComposition()`:
- Creates `LayerStrip` per layer, wires `onSelect` (`DeckView.cpp:101-104`) and `onClearClip` (`DeckView.cpp:105-114`).
- Creates `ClipCell` per (layer, col), wires 10 callbacks: `onTrigger`, `onSelect`, `onFileDrop`, `onMultiFileDrop`, `onMultiVideoDrop`, `onEffectDrop`, `onSourceDrop`, `onMilkDropDrop`, `onMilkDropPlaylistDrop`, `onClipMove` (`DeckView.cpp:131-162`).

Callbacks exposed to parent (`DeckView.h:34-48`):
`onClipTriggered`, `onClipSelected`, `onLayerSelected`, `onColumnTriggered`, `onFileDropped`, `onMultiFileDropped`, `onMultiVideoDropped`, `onEffectDropped`, `onSourceDropped`, `onClipMoved`, `onMilkDropDropped`, `onMilkDropPlaylistDropped`, `onDeckSwitched`, `onLayerFoldToggle` (P24.12), `onLayerReorder` (P24.13).

**CellPos multi-selection** (`DeckView.h:55-59`): `selectedCells_` vector. `clearSelection()`, `selectCell(layer, col, addToSelection)` — addToSelection toggles if already selected (`DeckView.cpp:322-337`).

**Layer selection**: single integer `selectedLayerIndex_` (`DeckView.h:81`), `selectLayer()` updates all layer strips' visual (`DeckView.cpp:339-344`).

**Natural height calculation** (`DeckView.cpp:233-247`): triggerHeight + sum of (folded? 22 : 96)+gap per layer + tabHeight.

### ClipCell Controls + Right-Click Menu

**File**: `src/ui/ClipCell.h`, `src/ui/ClipCell.cpp`.

**Zones** (`ClipCell.cpp:346-361`):
- Thumbnail area: upper part, excludes name bar.
- Name bar: bottom 20px (`ClipCell.h:84 kNameBarHeight=20`).

**Mouse behavior** (`ClipCell.cpp:180-206`):
- Left-click in thumbnail → `onTrigger(layer, column)` fired → plays/retriggers clip.
- Left-click in name bar → `onSelect(layer, column, addToSel)` — Cmd/Shift for multi-select (`ClipCell.cpp:191`).
- Right-click → **no menu — handler explicitly returns** (`ClipCell.cpp:182-183`). **Right-click context menus are NOT present in ClipCell** — the file explicitly bails out on right-click.
- Drag from name bar (5px threshold) → starts drag with description `"clip:layerIdx:col"` to move clip to another cell (`ClipCell.cpp:196-206`).

**Visual states**:
- Normal: dark gray bg `0xff1e1e1e` with 1px border `0xff1a1a1a` (`ClipCell.cpp:5-7`).
- `active_` (playing): muted teal 2px border `0xff4a9a8a` (`ClipCell.cpp:7, 119-123`).
- `selected_` (inspecting): white 2px border — takes priority over active (`ClipCell.cpp:114-118`).
- `dragHover_` (file drop): teal overlay (`ClipCell.cpp:131-135`).
- `fxDragHover_` (FX drop): `0xff8866cc` purple overlay + 2px border (`ClipCell.cpp:138-144`).
- `sourceDragHover_` (source drop): `0xffbb88ff` lavender overlay + 2px border (`ClipCell.cpp:147-153`).

**Content indicators**:
- Procedural source: blue-to-purple gradient with "SRC" label + sourceType text (`ClipCell.cpp:27-47`).
- Video/ImageSequence: thumbnail or name fallback (`ClipCell.cpp:48-67`).
- Image: thumbnail rescaled to 90×72 (`ClipCell.cpp:338-344`).
- FX-only (effects but no media): purple-tinted bg with "FX" label + purple "FX - " prefix in name (`ClipCell.cpp:74-81, 93-102`).
- **Content lock indicator** (P24.5): orange "L" in top-right if `clip->contentLocked` (`ClipCell.cpp:156-162`).
- **Missing file indicator** (P24.7): red 2px border + red "!" in top-left if media file no longer exists (`ClipCell.cpp:165-175`).

**Drag-drop target** — `FileDragAndDropTarget` + `DragAndDropTarget`:

File-drag accepted extensions (`ClipCell.cpp:210-222`): png, jpg, jpeg, gif, bmp, tiff, mov, avi, mp4, mkv, webm, m4v.

File drop logic (`ClipCell.cpp:236-293`):
- Separates images vs videos by extension.
- Single video → `onFileDrop`.
- Multiple videos → sorted naturally, placed in sequential cells via `onMultiVideoDrop`.
- Multiple images → `onMultiFileDrop` (image sequence).
- Single image → `onFileDrop`.

Internal drag formats accepted (`ClipCell.cpp:365-370`): `fx:`, `source:`, `clip:`, `files:`, `milkdrop:`, `milkdrop_playlist:`. Each processed in `itemDropped()` (`ClipCell.cpp:392-496`):
- `fx:Name1,Name2` → `onEffectDrop` called once per name (implicitly — actually passes full comma list to handler).
- `source:id` → `onSourceDrop`.
- `clip:srcLayer:srcCol` → `onClipMove(src, dst)`.
- `files:path1|path2` → separates images/videos, handles multi with offset (videos start at `column_+1` if images present).
- `milkdrop:path` → `onMilkDropDrop`.
- `milkdrop_playlist:p1|p2` → `onMilkDropPlaylistDrop`.

**Note**: No right-click context menu items are defined in ClipCell.cpp — the file explicitly early-returns on right-click. Clip right-click menus (copy/paste/clear/rename/replace/cuepoint operations) listed in the slice brief are NOT implemented here.

### LayerStrip — Every Control

**File**: `src/ui/LayerStrip.h`, `src/ui/LayerStrip.cpp`.

ASCII layout documented at `LayerStrip.h:10-17`. Controls arranged in a compact Resolume-style strip, 250px wide × 96px tall (standard), 22px wide when folded.

**Left column — button cluster** (buttons 26px square, `LayerStrip.cpp:578`):
- `clearBtn_{"X"}` (`LayerStrip.h:66`) — clears active clip (`LayerStrip.cpp:330-332` → `onClearClip(layerIndex)`).
- `bypassBtn_{"B"}` (`LayerStrip.h:67`) — toggles `layer->bypassed`; dark-red `0xff6a3a3a` when active (`LayerStrip.cpp:333-338, 757-760`).
- `soloBtn_{"S"}` (`LayerStrip.h:68`) — toggles `layer->solo`; olive `0xff7a7a4a` when active (`LayerStrip.cpp:339-344, 762-765`).

**Transport row** (row 2, 26px tall):
- `transportBackBtn_{"<"}` (`LayerStrip.h:71`) — sets clip reverse=true, playing=true, fires `onTransportBack` (`LayerStrip.cpp:356-361`).
- `transportPauseBtn_{"||"}` (`LayerStrip.h:72`) — sets `playing=false`, fires `onTransportPause` (`LayerStrip.cpp:362-367`).
- `transportPlayBtn_{">"}` (`LayerStrip.h:73`) — reverse=false, playing=true, fires `onTransportPlay` (`LayerStrip.cpp:368-373`).
- `transportForwardBtn_{">|"}` (`LayerStrip.h:74`) — reverse=false, playing=true, doubles speed (capped at 4.0×), fires `onTransportForward` (`LayerStrip.cpp:374-379`).

**Transport playhead scrub bar** (`LayerStrip.cpp:504-532`): fills area below transport buttons. Shows in/out region as dark teal, playhead line in cyan. Click or drag to scrub → writes `clip->playheadPosition` (`LayerStrip.cpp:729-739`). 30Hz repaint via timer (`LayerStrip.cpp:324, 697-704`).

**Right section — 4 vertical sliders + thumbnail**:

- **S — Speed slider** (`LayerStrip.h:77`, 22px wide): Range 0–1 mapped to 0×–4× clip speed, default 0.25 (=1×). Cyan fill `0xff3a6a7a`, "S" label at top (`LayerStrip.cpp:382-398, 167-194`). Writes `clip->speed = value × 4.0`.
- **K — Keying threshold slider** (`LayerStrip.h:80`): Range 0–1, default 0.1. Amber fill `0xff7a6a3a`, "K" label (`LayerStrip.cpp:401-411, 197-224`). Writes `layer->keyThreshold`. **No dropdown** — this is threshold-only, keying MODE is in V dropdown.
- **V — Opacity slider** (`LayerStrip.h:83`): Range 0–1, default 1.0. Teal fill `0xff4a7a6a`, "V" label (`LayerStrip.cpp:414-424, 107-134`). Writes `layer->opacity`.
- **V Dropdown — blend mode + keying mode combined** (`LayerStrip.h:84`): positioned below V slider, shows teal caret arrow only (no text) via `FlatComboBoxLookAndFeel` (`LayerStrip.cpp:287-311`). Selected ID scheme: 1–100 = MixMode, 101+ = KeyingMode (offset `kKeyingIdOffset=101`, `LayerStrip.cpp:4`). Contents populated by `populateBlendDropdown()` (`LayerStrip.cpp:855-881`).
- **Thumbnail** (square, `mainH × mainH` = 96×96 by default, `LayerStrip.cpp:586-588`): paints active clip's image/video thumbnail, or "SRC" / "FX" placeholder.
- **F — Fade speed slider** (`LayerStrip.h:98`): Range 0–4s, default 0.3. Slate fill `0xff5a6a7a`, "F" label (`LayerStrip.cpp:451-461, 137-164`). Writes `layer->transitionSpeed`.
- **F Dropdown — transition mix mode** (`LayerStrip.h:99`): caret-only, fires on change to set `layer->transitionMode` (`LayerStrip.cpp:465-475`). Populated with mix modes (`LayerStrip.cpp:883-889`).

**Bottom row — name fields** (20px tall `dropdownH`):
- **Layer name box** (`LayerStrip.h:94 layerName_`): painted manually 13px text, shows `layer->name` (`LayerStrip.cpp:537-545`).
- **Blend dropdown** (V dropdown): spans the S/K/V slider widths (3×22=66px) under those sliders.
- **Clip name box** (`LayerStrip.h:95 clipName_`): painted manually 10px text, shows clip filename / source type / FX-only name (`LayerStrip.cpp:548-558, 823-853`). Overlaid with playhead animation.
- **Transition dropdown** (F dropdown, 22px).

**Blend dropdown options** (`LayerStrip.cpp:855-881` via `populateMixModes()` at `LayerStrip.cpp:7-91`):

Section headings + options:
- Keying (13 modes): Alpha, Luma Key, Inverted Luma Key, Luma Is Alpha, Inverted Luma Is Alpha, Chroma Key, Max RGB, Saturation Key, Edge Detection, Threshold Mask, Channel Red, Channel Green, Channel Blue.
- Compositing (5): Alpha, Add, Screen, Multiply, Overlay.
- Light (6): Soft Light, Hard Light, Vivid Light, Linear Light, Pin Light, Hard Mix.
- Compare (4): Darken, Lighten, Darker Color, Lighter Color.
- Dodge/Burn (2): Color Dodge, Color Burn.
- Inversion (3): Difference, Exclusion, Subtract.
- Component (4): Hue, Saturation, Color, Luminosity.
- Special (2): Dissolve, Cut.
- Wipe (6): Left, Right, Up, Down, Ellipse, Diagonal.
- Push (4): Left, Right, Up, Down.
- Zoom (2): In, Out.
- 3D (6): Rotate X, Rotate Y, Spin, Cube, Flip, Fold.
- Color Fade (2): To Black, To White.
- Creative (9): Pixelate, Blur, Noise, RGB Split, Glitch Blocks, Strobe, Slide, Stretch, Displace.

**Total mix modes: 55 + 13 keying modes = 68 selectable options** in V dropdown.

**Transition dropdown** uses same mix modes (no keying), 55 options (`LayerStrip.cpp:883-889`).

**Selection visual** (`LayerStrip.cpp:561-564`): 2px cyan border around layer name box when `selected_`.

**Callbacks** (`LayerStrip.h:42-52`): `onSelect`, `onClearClip`, `onBypass(layer, bool)`, `onSolo(layer, bool)`, `onBlendModeChanged(layer, MixMode)`, `onTransportPlay`, `onTransportPause`, `onTransportBack`, `onTransportForward`, `onFoldToggle` (P24.12 — declared but not wired in this class), `onLayerDragReorder(from, to)` (P24.13 — declared but not wired).

**Mouse behavior** (`LayerStrip.cpp:706-727`):
- Click in transport bar → scrub playhead.
- Left-click elsewhere → `onSelect(layerIndex)`.
- Right-click: no special handling — no context menu.

---

## Signal Bar / Strip

### SignalBar

**File**: `src/ui/SignalBar.h`, `src/ui/SignalBar.cpp`.

Horizontal strip full window width, below TopBar. Updates at 30Hz from `FeatureBus` (`SignalBar.cpp:34, 111-132`).

**3 display sizes** (`SignalStrip.h:14`):
- `Minimized`: 26px tall, 36px per strip (`SignalBar.cpp:104`, `SignalStrip.cpp:48`).
- `Normal`: 84px tall, 40px per strip (default) (`SignalBar.cpp:105`, `SignalStrip.cpp:49`).
- `Expanded`: fills remaining space (`-1`), 120px per strip (`SignalBar.cpp:106`, `SignalStrip.cpp:50`).

**Controls**:
- `addButton_{"+"}` (`SignalBar.h:56`) — opens PopupMenu of hidden signals; selecting makes one visible via `setVisible(true)` and rebuilds strips (`SignalBar.cpp:179-222`). If all visible, shows disabled "All signals visible" item.
- `shrinkButton_` (▲ up arrow, `SignalBar.h:60`) — cycles Expanded→Normal→Minimized (`SignalBar.cpp:68-82`).
- `growButton_` (▼ down arrow, `SignalBar.h:61`) — cycles Minimized→Normal→Expanded (`SignalBar.cpp:84-98`).

Size buttons 20px wide, half the panel height each (`SignalBar.cpp:152-157`). Positioned at right edge.

Strip layout: horizontal, left-to-right, 2px gap (`SignalBar.cpp:163-171`). `[+]` button appended after last strip (`SignalBar.cpp:174-176`).

`rebuildStrips()` (`SignalBar.cpp:37-58`) iterates `registry_.getSignalAt(i)` and creates a `SignalStrip` for each visible signal. Rebuild fires on add, or explicitly after registry changes.

`onSignalSelected(Signal&)` callback fires when user clicks a strip (`SignalBar.cpp:50-53`) — caller opens the signal in the inspector.

### SignalStrip

**File**: `src/ui/SignalStrip.h`, `src/ui/SignalStrip.cpp`.

One vertical meter per registered `Signal`. Three paint modes (`SignalStrip.cpp:66-76`).

**State variables** (`SignalStrip.h:51-55`):
- `displayValue_` — smoothed with `smoothAlpha=0.3` (`SignalStrip.cpp:13`).
- `smoothedValue_` — EMA state.
- `peakValue_` — peak-hold with decay `kPeakDecay=0.97`, hold frames `kPeakHoldFrames=30` (~1s at 30fps) (`SignalStrip.h:57-58`).
- `flashAlpha_` — flashes on trigger signals like "Hit" when value > 0.3 (`SignalStrip.cpp:32-36`).

**Click** (`SignalStrip.cpp:78-82`): fires `onSelected(signal_)` callback.

**Minimized paint** (`SignalStrip.cpp:86-119`): 10px indicator square with bottom-fill, 3-char abbreviated name.

**Normal paint** (`SignalStrip.cpp:121-200`):
- Value readout at top (9px bold, 2 decimal places or integer).
- Vertical meter bar (max 20px wide) centered, filled bottom-to-top with signal's category color.
- Peak hold line.
- Abbreviated name at bottom.
- Flash overlay on Hit signals.
- Name abbreviations: "Beat Position"→"Beat", "Sub Bass"→"Sub", "Hits Per Second"→"HPS", "Beat In Bar"→"Bar", "Hit Strength"→"HStr", "Note Confidence"→"NtCf", "Chord Change"→"Chrd" (`SignalStrip.cpp:187-193`).

**Expanded paint** (`SignalStrip.cpp:202-275`):
- Full signal name at top, 11px bold, in category color.
- Big value readout (14px bold).
- Large vertical meter (max 30px wide).
- Category label at bottom: Amplitude / Bands / Rhythm / Pitch / Chroma / Timbre / Structure / Modulation (`SignalStrip.cpp:260-270`).

**Value formatting** (`SignalStrip.cpp:279-309`):
- "Tempo" → integer BPM or "---".
- "Beat In Bar" → 1–4 integer.
- "Hit" → filled circle (●) if >0.3, else empty circle (○).
- "Beat Position" / "Bar Position" → 2-decimal places.
- Default: 2 decimals for [0,1], integer if ≥10.

**Category colors** (`SignalStrip.cpp:311-344`):
- Amplitude: green.
- Bands: band-specific — Sub Bass red, Bass orange, Low Mid amber, Mid green, High Mid teal, Presence blue, Air purple.
- Rhythm: magenta.
- Pitch: mint.
- Chroma: gold.
- Timbre: lavender.
- Structure: yellow.
- Modulation: cyan.

No right-click menu. No drag-drop on signal strips (strips are receivers in mapping UI elsewhere).

---

## Effect Stack View

**File**: `src/ui/EffectStackView.h`, `src/ui/EffectStackView.cpp`.

Vertical list of effect rows, each collapsible. Used inside ClipInspector and LayerInspector.

**Per-row controls** (`EffectStackView.h:67-81`):
- `[B]` bypass button (`EffectStackView.h:72`), 24×20px top-left of header (`EffectStackView.cpp:97`). Dark-red `0xff6a3a3a` when bypassed, gray `0xff333333` otherwise (`EffectStackView.cpp:260-261, 148-149`). Click toggles `slot.bypassed`, fires `onBypassChanged(index, bypassed)` (`EffectStackView.cpp:266-272`).
- `[X]` delete button (`EffectStackView.h:72` `deleteBtn`), 24×20px top-right (`EffectStackView.cpp:98`). Red text `0xffcc5555` on gray bg (`EffectStackView.cpp:276-277`). Click erases the slot and fires `onEffectRemoved(index)` (`EffectStackView.cpp:278-286`).
- Header 26px tall (`EffectStackView.h:89 kHeaderHeight=26`).
- Expand triangle (right side): down for expanded, right for collapsed (`EffectStackView.cpp:64-81`).

**Header display** (`EffectStackView.cpp:38-82`):
- Gray bg `0xff2a2a2a` (dark red-brown `0xff2a2020` when bypassed).
- Effect display name (12px).
- When collapsed, shows first param value on the right (11px, 2 decimals).
- Click header (excluding buttons, x > 28) toggles expand via `toggleExpand()` (`EffectStackView.cpp:362-371, 378-386`).

**Expanded row** (`EffectStackView.cpp:101-127`):
- First: Dry/Wet control (`UniversalParamControl`), always present, default 1.0 (`EffectStackView.cpp:290-311`).
- Then: one `UniversalParamControl` per effect parameter. Param name + default value looked up from `EffectLibrary::EffectDef::params` (`EffectStackView.cpp:313-353`). Fallback to "Param N" if no def.
- Indented 12px (`EffectStackView.h:90 kParamIndent`).

**Parameter control value writes** (`EffectStackView.cpp:337-343`): `UniversalParamControl::onValueChanged` writes `slot.paramValues[p] = val` and fires `onParamChanged(effectIdx, paramIdx, val)`.

**Signal-driven modulation** (`EffectStackView.cpp:155-213`): If param has a connected signal source, every `refresh()` pulls the signal's cached value from `SignalRegistry` (or macro value from `MacroBank`) and overwrites `fx.paramValues[p]`. Supported source modes: Signal, Oscillator, Envelope, Macro.

**Drag-drop target** (FX drop from browser) — `DragAndDropTarget` interface (`EffectStackView.h:52-56`):
- Accepts description starting with `"fx:"` (`EffectStackView.cpp:392-395`).
- On enter → purple highlight `0xff8866cc` (`EffectStackView.cpp:397-401, 12-17`).
- On drop (`EffectStackView.cpp:409-450`): parses `"fx:Echo,Ripple,Freeze"`, splits by comma, resolves each via `EffectLibrary::getEffectDef()`, appends `EffectSlot` with default param values, fires `onEffectAdded(name)` per effect, rebuilds rows.

**NO drag-reorder** inside the stack — effect reorder is not implemented in this class.
**NO right-click context menu** on effect rows — header click only toggles expand.

**Callbacks** (`EffectStackView.h:59-63`):
`onParamChanged(effectIdx, paramIdx, value)`, `onBypassChanged(effectIdx, bypassed)`, `onDryWetChanged(effectIdx, dryWet)`, `onEffectAdded(name)`, `onEffectRemoved(index)`.

---

## EffectsRackPanel (legacy / global effects rack)

**File**: `src/ui/EffectsRackPanel.h`, `src/ui/EffectsRackPanel.cpp`.

Right-side panel showing the global `EffectChain` (single-image mode) with per-param knobs and mapping controls. Differs from `EffectStackView` in that it owns the `MappingEditor` popup and supports random preset generation.

**Top-level controls**:
- Title "Effects Rack" (14px cyan bold, `EffectsRackPanel.cpp:97-101`).
- `randomizeButton_{"Random"}` (`EffectsRackPanel.h:75`, 58×22px at top-right) — randomizes between 3–8 unlocked effects + creates random mappings with sources RMS/Peak/SpectralCentroid/SpectralFlux/BandSub/BandBass/BandMid/OnsetStrength/BeatPhase/TransientDensity/HarmonicChange and 4 curves Linear/Exponential/Logarithmic/SCurve (`EffectsRackPanel.cpp:15-81`).

**Per-effect section** (`EffectsRackPanel.h:56-65`):
- Enable toggle (22×22 `juce::ToggleButton`, `EffectsRackPanel.cpp:350-364`).
- Effect name label (13px bold).
- `lockButton_{"L"}` — protects effect from randomization; transparent when unlocked, green bg when locked (`EffectsRackPanel.cpp:367-396`).
- `randButton_{"R"}` — per-effect randomize: randomizes all params, replaces mappings (`EffectsRackPanel.cpp:399-450`).
- Parameter knobs (`Knob` widget), arranged in a grid `knobsPerRow = contentWidth / Knob::kPreferredWidth` (`EffectsRackPanel.cpp:112`).
- Map button `"+"` or `"M"` below each knob — opens `MappingEditor` popup (`EffectsRackPanel.cpp:478-487, 498-533`).

**Category headers** (`EffectsRackPanel.h:68-72`, `EffectsRackPanel.cpp:287-307`) — 8 categories with colors:
- 3D (purple `0xffb088f9`, label "3D / DEPTH")
- warp (pink `0xffff6b9d`)
- color (teal `0xff4ecdc4`)
- glitch (yellow `0xffffe66d`)
- pattern (orange `0xffff9f43`, label "PATTERN / STYLE")
- animation (green `0xff78e08f`)
- blend (sky blue `0xff82ccdd`, label "BLEND / COMPOSITE")
- blur (mint `0xff95e1d3`, label "BLUR / POST")

Note: This legacy panel only knows 8 of the current 11 categories (missing time, composite, audio from the current FXBrowser list).

**MappingEditor integration** (`EffectsRackPanel.cpp:498-545`): inline popup child component, overlaid on the rack. One `activeMappingEditor_` at a time. `MappingEditor::Listener` interface methods fire on change / delete / close (`EffectsRackPanel.cpp:40-43, 240-270`). New mapping defaults: source=RMS, curve=Linear, smoothing=0.15.

**Timer** at 10Hz (`EffectsRackPanel.cpp:84, 194-238`):
- Detects effect count changes → rebuilds UI.
- Syncs knob value from `Effect::getParam()` unless being dragged.
- Updates map button text: "M" if mapped, "+" if not.
- Sets mapping indicator ring text on Knob.

**Scrollable viewport** (`EffectsRackPanel.h:90-91`) holds all sections.

No drag-drop source or target on this panel. No right-click menu.

---

## Section 17 partial: Right-Click Menus (Browsers + Clip Cells)

Exhaustive enumeration across this slice:

| Component | Right-Click Target | Action / Menu Items |
|-----------|--------------------|---------------------|
| `FilesBrowser` list | File/folder entry | **Toggles favorite** on that file (no popup menu) — `FilesBrowser.cpp:33-41`. Saves to `browser_favorites.txt`. |
| `FXBrowser` | Effect row | **No handling** — default click falls through (`FXBrowser.cpp:124` does not branch on right-click). |
| `SourcesBrowser` | Source row | **No handling** — mouseDown treats it same as left-click (`SourcesBrowser.cpp:99-180`). |
| `CompDecksBrowser` | Composition entry | **Deletes file** (destructive, no confirmation) — `CompDecksBrowser.cpp:98-104`. |
| `CompDecksBrowser` | Deck entry | **Deletes file** (destructive, no confirmation) — `CompDecksBrowser.cpp:131-136`. |
| `MilkDropBrowser` | Preset row | **Toggles favorite** via `presetManager_->toggleFavorite(idx)` — `MilkDropBrowser.cpp:46-51`. |
| `MilkDropBrowser` | Section header | No right-click handling. |
| `RecordPanel` | Any | No right-click handling. |
| `ClipCell` | Any | **Explicit early-return, no menu** — `ClipCell.cpp:180-183`. The clip right-click menu (copy/paste/clear/rename/replace/cuepoint operations) referenced in the slice brief is NOT implemented in ClipCell; if present, it lives elsewhere (likely in `ClipInspector` or `MainComponent::mouseDown`). |
| `LayerStrip` | Any | No right-click handling beyond the default onSelect bypass (`LayerStrip.cpp:715-718`). |
| `SignalStrip` | Any | No right-click handling (`SignalStrip.cpp:78-82` only fires onSelected on any click). |
| `EffectStackView` row | Any | No right-click handling. |
| `EffectsRackPanel` | Any | No right-click menu. |

**Total right-click menu items across this slice: 0 true menu items.** Right-click is used only for binary toggles (favorite / delete), never for popups.

---

## Section 20 partial: Cross-References

| This component | Fires callback / emits | Consumed by |
|----------------|----------------------|-------------|
| `BrowserPanel` | Contains 6 sub-browsers | Parent is `MainComponent` which wires the sub-browser callbacks |
| `FilesBrowser` | `onFileActivated(File)` | `MainComponent` loads the file |
| `FilesBrowser` drag source | `"files:p1\|p2\|..."` | `ClipCell::itemDropped` (this slice), possibly `EffectStackView` does NOT accept files |
| `FXBrowser` drag source | `"fx:Name1,Name2"` | `ClipCell::itemDropped`, `EffectStackView::itemDropped`, `ClipInspector`, `LayerInspector` |
| `FXBrowser` callback | `onEffectActivated(name)` | `MainComponent` / inspector (unused in most paths) |
| `SourcesBrowser` drag source | `"source:id1,id2"` | `ClipCell::itemDropped` (only the first ID is used in drop handler — see `ClipCell.cpp:406-411`) |
| `CompDecksBrowser` callbacks | `onCompositionLoad(File)`, `onDeckLoad(File)`, `onCompositionSave()` | `MainComponent` |
| `MilkDropBrowser` drag sources | `"milkdrop:path"`, `"milkdrop_playlist:p1\|p2"` | `ClipCell::itemDropped` |
| `MilkDropBrowser` | `onPresetSelected(path)` | `PresetSelector` / renderer |
| `RecordPanel` | `onStartRecording`, `onStopRecording`, `onPlayRecording` | `MainComponent` |
| `DeckView` | 13 callbacks listed in `DeckView.h:34-48` | `MainComponent` |
| `ClipCell` | 10 callbacks to DeckView, which forwards to MainComponent | `MainComponent` performs all file/FX/source loading |
| `ClipCell` drag source | `"clip:layerIdx:col"` | Other `ClipCell` (clip move), `EffectStackView` rejects |
| `LayerStrip` | `onSelect`, `onClearClip`, `onBypass`, `onSolo`, `onBlendModeChanged`, transport callbacks, P24.12/P24.13 reorder | DeckView forwards to MainComponent |
| `SignalBar` | `onSignalSelected(Signal&)`, `onSizeChanged()` | `MainComponent` opens inspector / re-layouts |
| `SignalStrip` | `onSelected(Signal&)` | Forwarded by SignalBar |
| `EffectStackView` | `onParamChanged`, `onBypassChanged`, `onDryWetChanged`, `onEffectAdded`, `onEffectRemoved` | `ClipInspector` / `LayerInspector` — triggers `Renderer` uniform upload |
| `EffectStackView` drag target | accepts `"fx:"` | Sources: `FXBrowser` |
| `EffectsRackPanel` | Owns `MappingEditor` popup | `MappingEngine` + `EffectChain` (global, single-image mode only) |

**Signal registry** (`SignalRegistry`) and **MacroBank** are shared upward dependencies for `EffectStackView` param signal-driven modulation (`EffectStackView.cpp:164-203`).

**FeatureBus** feeds `SignalBar` at 30Hz (`SignalBar.cpp:111-132`), which in turn drives signal meter updates and calls `registry_.evaluateAll(snap)` each tick.

---

## Surprises / Gaps Found

1. **No ClipCell right-click context menu**. The slice brief called out "copy, paste, clear, rename, replace, cuepoint operations" — these are NOT in ClipCell.cpp. Right-click is explicitly ignored there. If any such menu exists in the app, it is implemented in `MainComponent`, `ClipInspector`, or DeckView's parent wiring — not audited here.

2. **No effect drag-reorder in EffectStackView**. The slice brief mentioned "drag-reorder" — not implemented.

3. **`EffectsRackPanel` is legacy**. It's the pre-v2 global rack. The v2 per-clip/per-layer effect stacks use `EffectStackView` instead. The rack knows only 8 of the 11 current FX categories (missing time, composite, audio).

4. **FXBrowser drag description is comma-joined but `ClipCell::onEffectDrop` is fired ONCE with the full comma string** (`ClipCell.cpp:402-405`). The receiver must split commas (consistent with CLAUDE.md pitfall #16).

5. **SourcesBrowser drag is also comma-joined** (`SourcesBrowser.cpp:202-207`), but `ClipCell` passes the whole string to `onSourceDrop` as a single `sourceId` — likely means dropping multiple sources only registers the first one via `substring(7)` in ClipCell.

6. **CompDecksBrowser right-click is destructive without confirmation** — deletes the file immediately.

7. **RecordPanel "Video (Future)" format** item is listed but marked as future work.

8. **LayerStrip V dropdown combines keying modes with mix modes** via `kKeyingIdOffset=101` — an unusual single-ID-space encoding for two enum types.

9. **No tooltip setup** visible in any browser panel or LayerStrip — global tooltip coverage is scheduled for P26.

10. **SignalBar `+` button only shows currently hidden signals** — there's no UI to permanently add or remove signals from the registry.

11. **Folded layer height (22px) and `onLayerFoldToggle` callback are declared in DeckView/LayerStrip** but the actual fold-toggle button is not visible in LayerStrip.cpp — may be triggered by parent UI elsewhere.

12. **MilkDrop `playlistTimingSelector_`** `onChange` is not wired to any backend — selecting "4 beats" does nothing.

---
