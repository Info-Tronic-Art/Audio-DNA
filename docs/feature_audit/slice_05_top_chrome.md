# Slice 05: Top Chrome Feature Audit

**Scope**: MainComponent, TopBar, MenuBarModel, PreferencesDialog, AudioReadoutPanel, Waveform/Spectrum/Timing displays, overlays (BindingOverlay, MidiLearnOverlay, ProgrammingMode), OutputWindow, PreviewPanel, LookAndFeel, Knob, Main.cpp.

**Files audited** (absolute paths):
- `/Users/boriskarpman/Documents/RealTimeAudio/src/Main.cpp` (100 lines)
- `/Users/boriskarpman/Documents/RealTimeAudio/src/MainComponent.h` (279 lines)
- `/Users/boriskarpman/Documents/RealTimeAudio/src/MainComponent.cpp` (3868 lines)
- `/Users/boriskarpman/Documents/RealTimeAudio/src/ui/TopBar.h` (113 lines)
- `/Users/boriskarpman/Documents/RealTimeAudio/src/ui/TopBar.cpp` (522 lines)
- `/Users/boriskarpman/Documents/RealTimeAudio/src/ui/MenuBarModel.h` (127 lines)
- `/Users/boriskarpman/Documents/RealTimeAudio/src/ui/MenuBarModel.cpp` (200 lines)
- `/Users/boriskarpman/Documents/RealTimeAudio/src/ui/PreferencesDialog.h` (96 lines)
- `/Users/boriskarpman/Documents/RealTimeAudio/src/ui/PreferencesDialog.cpp` (383 lines)
- `/Users/boriskarpman/Documents/RealTimeAudio/src/ui/AudioReadoutPanel.h` (70 lines)
- `/Users/boriskarpman/Documents/RealTimeAudio/src/ui/AudioReadoutPanel.cpp` (627 lines)
- `/Users/boriskarpman/Documents/RealTimeAudio/src/ui/WaveformDisplay.h/cpp` (48/171 lines)
- `/Users/boriskarpman/Documents/RealTimeAudio/src/ui/SpectrumDisplay.h/cpp` (47/124 lines)
- `/Users/boriskarpman/Documents/RealTimeAudio/src/ui/TimingWindow.h/cpp` (32/98 lines)
- `/Users/boriskarpman/Documents/RealTimeAudio/src/ui/BindingOverlay.h/cpp` (67/299 lines)
- `/Users/boriskarpman/Documents/RealTimeAudio/src/ui/MidiLearnOverlay.h/cpp` (65/345 lines)
- `/Users/boriskarpman/Documents/RealTimeAudio/src/ui/ProgrammingMode.h/cpp` (31/64 lines)
- `/Users/boriskarpman/Documents/RealTimeAudio/src/ui/OutputWindow.h/cpp` (108/326 lines)
- `/Users/boriskarpman/Documents/RealTimeAudio/src/ui/PreviewPanel.h/cpp` (76/112 lines)
- `/Users/boriskarpman/Documents/RealTimeAudio/src/ui/LookAndFeel.h/cpp` (68/469 lines)
- `/Users/boriskarpman/Documents/RealTimeAudio/src/ui/Knob.h/cpp` (48/96 lines)

---

## Section 14: Menu Bar

Menu model declared in `AudioDNAMenuBar` at `src/ui/MenuBarModel.h:8`. Menu bar names returned at `src/ui/MenuBarModel.cpp:9-10`:
> `{ "Audio-DNA", "Composition", "Deck", "Layer", "Column", "Clip", "Output", "Shortcuts", "View" }` — 9 menus total.

On macOS it is installed via `juce::MenuBarModel::setMacMainMenu` at `src/Main.cpp:58`. On Windows/Linux it is attached to the window via `setMenuBar` at `src/Main.cpp:61`.

No keyboard shortcuts are defined on menu items in `getPopupMenu` — `addItem` is called with `(id, text, isActive=true, isTicked=false)`. All shortcuts are handled in `MainComponent::keyPressed` (see Section 20).

### 14.1 Audio-DNA (index 0) — `src/ui/MenuBarModel.cpp:20-30`

| Item | Command ID (constant) | ID value | File:Line | Handler | Hotkey |
|---|---|---|---|---|---|
| Preferences... | `kPreferences` | 1000 | `MenuBarModel.cpp:22` | `PreferencesDialog::show(this)` at `MainComponent.cpp:2723-2725` | — |
| Import ISF Shader... | `kImportISF` | 1003 | `MenuBarModel.cpp:24` | `handleImportISF()` at `MainComponent.cpp:2733-2735` | — |
| About Audio-DNA | `kAbout` | 1001 | `MenuBarModel.cpp:26` | Opens PreferencesDialog (TODO: switch to About tab) `MainComponent.cpp:2726-2729` | — |
| Quit | `kQuit` | 1002 | `MenuBarModel.cpp:28` | `JUCEApplication::getInstance()->systemRequestedQuit()` at `MainComponent.cpp:2730-2732` | — |

Separators between Preferences / Import ISF / About / Quit (4 separators).

### 14.2 Composition (index 1) — `src/ui/MenuBarModel.cpp:32-49`

| Item | Command ID | ID value | File:Line | Handler | Hotkey |
|---|---|---|---|---|---|
| Undo | `kCompUndo` | 1098 | `MenuBarModel.cpp:34` | `undoManager_.undo()` at `MainComponent.cpp:2738-2740` | Cmd+Z (`MainComponent.cpp:1624`) |
| Redo | `kCompRedo` | 1099 | `MenuBarModel.cpp:35` | `undoManager_.redo()` at `MainComponent.cpp:2741-2743` | Cmd+Shift+Z (`MainComponent.cpp:1626`) |
| New Composition | `kCompNew` | 1100 | `MenuBarModel.cpp:37` | `composition_.initDefault()` at `MainComponent.cpp:2744-2748` | — |
| Open... | `kCompOpen` | 1101 | `MenuBarModel.cpp:38` | `loadPreset()` at `MainComponent.cpp:2749-2751` | — |
| Save | `kCompSave` | 1102 | `MenuBarModel.cpp:40` | `savePreset()` at `MainComponent.cpp:2752-2754` | Cmd+S (`MainComponent.cpp:1634`) |
| Save As... | `kCompSaveAs` | 1103 | `MenuBarModel.cpp:41` | `savePreset()` at `MainComponent.cpp:2755-2757` | — |
| Copy Global Effects | `kCompCopyEffects` | 1104 | `MenuBarModel.cpp:43` | Not implemented (falls to default) `MainComponent.cpp:3258-3260` | — |
| Paste Global Effects | `kCompPasteEffects` | 1105 | `MenuBarModel.cpp:44` | Not implemented | — |
| Collect Media... | `kCompCollectMedia` | 1106 | `MenuBarModel.cpp:46` | FileChooser → copies all clip media files into `<comp_name>_media/` dir, relinks clips, saves composition JSON. `MainComponent.cpp:2759-2815` | — |
| Relocate Missing Files... | `kCompRelocateFiles` | 1107 | `MenuBarModel.cpp:47` | FileChooser → searches target dir (including subdirs) for missing clip mediaFile names, relinks. `MainComponent.cpp:2817-2869` | — |

### 14.3 Deck (index 2) — `src/ui/MenuBarModel.cpp:51-64`

| Item | Command ID | ID value | File:Line | Handler |
|---|---|---|---|---|
| New Deck | `kDeckNew` | 1200 | `MenuBarModel.cpp:53` | Pushes new `Deck` named "Deck N+1", sets active. `MainComponent.cpp:2872-2880` |
| Insert Before | `kDeckInsertBefore` | 1201 | `MenuBarModel.cpp:54` | Not implemented |
| Insert After | `kDeckInsertAfter` | 1202 | `MenuBarModel.cpp:55` | Not implemented |
| Duplicate | `kDeckDuplicate` | 1203 | `MenuBarModel.cpp:56` | Not implemented |
| Rename... | `kDeckRename` | 1204 | `MenuBarModel.cpp:58` | Not implemented |
| Clear Clips | `kDeckClearClips` | 1206 | `MenuBarModel.cpp:59` | Clears all `layer.clips` vectors in active deck, resizes to numColumns, calls `clearActiveClip`. `MainComponent.cpp:2891-2903` |
| Close Deck | `kDeckClose` | 1205 | `MenuBarModel.cpp:61` | Not implemented |
| Remove Deck | `kDeckRemove` | 1207 | `MenuBarModel.cpp:62` | Erases active deck if more than 1, clamps activeDeckIndex. `MainComponent.cpp:2881-2890` |

### 14.4 Layer (index 3) — `src/ui/MenuBarModel.cpp:66-87`

| Item | Command ID | ID value | File:Line | Handler |
|---|---|---|---|---|
| New Layer | `kLayerNew` | 1300 | `MenuBarModel.cpp:68` | `deck->addLayer()` `MainComponent.cpp:2906-2914` |
| Insert Above | `kLayerInsertAbove` | 1301 | `MenuBarModel.cpp:69` | Falls through to `addLayer()` (same as New) |
| Insert Below | `kLayerInsertBelow` | 1302 | `MenuBarModel.cpp:70` | Falls through to `addLayer()` |
| Duplicate | `kLayerDuplicate` | 1303 | `MenuBarModel.cpp:71` | Not implemented |
| Rename... | `kLayerRename` | 1304 | `MenuBarModel.cpp:73` | Not implemented |
| Copy Effects | `kLayerCopyEffects` | 1305 | `MenuBarModel.cpp:74` | Not implemented |
| Paste Effects | `kLayerPasteEffects` | 1306 | `MenuBarModel.cpp:75` | Not implemented |
| Clear Clips | `kLayerClearClips` | 1307 | `MenuBarModel.cpp:77` | Clears all layers' clips (same as Deck Clear Clips per impl) `MainComponent.cpp:2925-2937` |
| Remove Layer | `kLayerRemove` | 1308 | `MenuBarModel.cpp:78` | Removes last layer if > 1 layers. `MainComponent.cpp:2915-2924` |
| Ignore Column Trigger | `kLayerIgnoreColumnTrigger` | 1309 | `MenuBarModel.cpp:80` | Not implemented |
| Lock Content | `kLayerLockContent` | 1310 | `MenuBarModel.cpp:81` | Not implemented |
| Fold/Unfold Layer | `kLayerFold` | 1311 | `MenuBarModel.cpp:83` | Toggles `layer.folded` on selected layer `MainComponent.cpp:2939-2955` |
| Move Layer Up | `kLayerMoveUp` | 1312 | `MenuBarModel.cpp:84` | `deck->moveLayer(sel, sel-1)` `MainComponent.cpp:2956-2969` |
| Move Layer Down | `kLayerMoveDown` | 1313 | `MenuBarModel.cpp:85` | `deck->moveLayer(sel, sel+1)` `MainComponent.cpp:2970-2983` |

### 14.5 Column (index 4) — `src/ui/MenuBarModel.cpp:89-102`

| Item | Command ID | ID value | File:Line | Handler |
|---|---|---|---|---|
| New Column | `kColumnNew` | 1400 | `MenuBarModel.cpp:91` | `deck->addColumn()` `MainComponent.cpp:2986-2994` |
| Insert Before | `kColumnInsertBefore` | 1401 | `MenuBarModel.cpp:92` | Falls through to addColumn() |
| Insert After | `kColumnInsertAfter` | 1402 | `MenuBarModel.cpp:93` | Falls through to addColumn() |
| Duplicate | `kColumnDuplicate` | 1403 | `MenuBarModel.cpp:94` | Not implemented |
| Clear Clips | `kColumnClearClips` | 1404 | `MenuBarModel.cpp:96` | Not implemented |
| Remove Column | `kColumnRemove` | 1405 | `MenuBarModel.cpp:97` | Removes last column if > 1. `MainComponent.cpp:2995-3004` |
| Remove All Before | `kColumnRemoveAllBefore` | 1406 | `MenuBarModel.cpp:99` | Not implemented |
| Remove All After | `kColumnRemoveAllAfter` | 1407 | `MenuBarModel.cpp:100` | Not implemented |

### 14.6 Clip (index 5) — `src/ui/MenuBarModel.cpp:104-125`

| Item | Command ID | ID value | File:Line | Handler |
|---|---|---|---|---|
| Select All | `kClipSelectAll` | 1500 | `MenuBarModel.cpp:106` | Not implemented |
| Cut | `kClipCut` | 1501 | `MenuBarModel.cpp:108` | Not implemented |
| Copy | `kClipCopy` | 1502 | `MenuBarModel.cpp:109` | Not implemented |
| Paste | `kClipPaste` | 1503 | `MenuBarModel.cpp:110` | Not implemented |
| Copy Effects | `kClipCopyEffects` | 1504 | `MenuBarModel.cpp:112` | Not implemented |
| Paste Effects | `kClipPasteEffects` | 1505 | `MenuBarModel.cpp:113` | Not implemented |
| Rename... | `kClipRename` | 1506 | `MenuBarModel.cpp:115` | Not implemented |
| Clear | `kClipClear` | 1507 | `MenuBarModel.cpp:116` | For each selected cell: `deck->setClip(layer, col, Clip{})`. `MainComponent.cpp:3007-3019` |
| Show in Finder | `kClipShowInFinder` | 1508 | `MenuBarModel.cpp:117` | Not implemented |
| New Procedural Source | `kClipNewSource` | 1509 | `MenuBarModel.cpp:119` | Not implemented |
| New Effect Clip | `kClipNewEffect` | 1510 | `MenuBarModel.cpp:120` | Not implemented |
| Replace Content... | `kClipReplaceContent` | 1511 | `MenuBarModel.cpp:122` | FileChooser → `clip->replaceContent(newContent)` (keeps effects). Supports image/video. `MainComponent.cpp:3021-3077` |
| Lock Content | `kClipLockContent` | 1512 | `MenuBarModel.cpp:123` | Toggles `clip->contentLocked` on all selected cells. `MainComponent.cpp:3079-3097` |

### 14.7 Output (index 6) — `src/ui/MenuBarModel.cpp:127-157`

Dynamically includes one item per connected display.

| Item | Command ID | ID value | File:Line | Handler |
|---|---|---|---|---|
| Disabled | `kOutputDisabled` | 1600 | `MenuBarModel.cpp:129` | `closeOutput()` `MainComponent.cpp:3100-3102` |
| Fullscreen: WxH (main/display N) | `kOutputFullscreenBase + i` | 1601+ | `MenuBarModel.cpp:134-145` (per-display loop) | `openOutputOnDisplay(displayIdx)` at `MainComponent.cpp:2713-2718` |
| Windowed | `kOutputWindowed` | 1690 | `MenuBarModel.cpp:148` | Not implemented |
| Identify Displays | `kOutputIdentifyDisplays` | 1691 | `MenuBarModel.cpp:150` | Not implemented |
| Test Card | `kOutputTestCard` | 1692 | `MenuBarModel.cpp:151` | Not implemented |
| Snapshot | `kOutputSnapshot` | 1693 | `MenuBarModel.cpp:152` | `std::thread([&]{ renderer.takeSnapshot(); })` detached. `MainComponent.cpp:3103-3111` |
| Start Recording | `kOutputStartRecording` | 1694 | `MenuBarModel.cpp:154` | Creates ~/Documents/Audio-DNA/Recordings/recording_YYYYMMDD_HHMMSS.mp4, starts `videoRecorder_` with H264/1920x1080/30fps/CRF 23. `MainComponent.cpp:3112-3133` |
| Stop Recording | `kOutputStopRecording` | 1695 | `MenuBarModel.cpp:155` | `videoRecorder_.stopRecording()` `MainComponent.cpp:3134-3139` |

Fullscreen command IDs range from 1601 to 1689 inclusive (filtered at `MainComponent.cpp:2713`).

### 14.8 Shortcuts (index 7) — `src/ui/MenuBarModel.cpp:159-169`

| Item | Command ID | ID value | File:Line | Handler | Hotkey |
|---|---|---|---|---|---|
| Edit Keyboard Shortcuts... | `kShortcutsEditKeyboard` | 1700 | `MenuBarModel.cpp:161` | `enterKeyboardBindingMode()` `MainComponent.cpp:3142-3144` | Cmd+Shift+K (`MainComponent.cpp:1599`) |
| Edit MIDI Mappings... | `kShortcutsEditMIDI` | 1701 | `MenuBarModel.cpp:162` | `enterMidiLearnMode()` `MainComponent.cpp:3145-3147` | Cmd+Shift+M (`MainComponent.cpp:1606`) |
| Stop All | `kShortcutsStop` | 1702 | `MenuBarModel.cpp:164` | `exitAllBindingModes()` `MainComponent.cpp:3148-3150` | — |
| Export Bindings... | `kShortcutsExportBindings` | 1703 | `MenuBarModel.cpp:166` | FileChooser → `bindingManager_.saveToFile()`. Default dir: `~/Library/Application Support/Audio-DNA/bindings/`. `MainComponent.cpp:3151-3166` |
| Import Bindings... | `kShortcutsImportBindings` | 1704 | `MenuBarModel.cpp:167` | FileChooser → `bindingManager_.loadFromFile()` `MainComponent.cpp:3167-3181` |

### 14.9 View (index 8) — `src/ui/MenuBarModel.cpp:171-187`

| Item | Command ID | ID value | File:Line | Handler |
|---|---|---|---|---|
| Signal Bar | `kViewSignalBar` | 1800 | `MenuBarModel.cpp:173` | Not implemented |
| Deck | `kViewDeck` | 1801 | `MenuBarModel.cpp:174` | Not implemented |
| Preview | `kViewPreview` | 1802 | `MenuBarModel.cpp:175` | Not implemented |
| Inspector | `kViewInspector` | 1803 | `MenuBarModel.cpp:176` | Not implemented |
| Browser | `kViewBrowser` | 1804 | `MenuBarModel.cpp:177` | Not implemented |
| Timing Window | `kViewTimingWindow` | 1805 | `MenuBarModel.cpp:178` | Not implemented |
| FPS and Stats | `kViewFpsStats` | 1806 | `MenuBarModel.cpp:180` | Not implemented |
| Programming Mode | `kViewProgrammingMode` | 1807 | `MenuBarModel.cpp:181` | Toggles `signalBar_->setDisplaySize()` between Expanded/Normal. `MainComponent.cpp:3184-3195` |
| Save Layout... | `kViewSaveLayout` | 1850 | `MenuBarModel.cpp:183` | FileChooser → writes JSON `{ deckDividerY, vDividerFrac[3] }` to `~/Library/Application Support/Audio-DNA/layouts/*.json`. `MainComponent.cpp:3198-3220` |
| Load Layout... | `kViewLoadLayout` | 1851 | `MenuBarModel.cpp:184` | FileChooser → parses JSON, restores divider positions, calls `resized()`. `MainComponent.cpp:3221-3247` |
| Reset Layout | `kViewResetLayout` | 1852 | `MenuBarModel.cpp:185` | Sets `deckDividerY_=-1`, `vDividerFrac_ = {0.22, 0.50, 0.75}`, `resized()`. `MainComponent.cpp:3248-3256` |

**Total menu items (including dynamic display entries): ~70** (9 menus × avg 7-14 items, including up to 8 displays in Output). Unimplemented (fall to `default: DBG("not yet implemented")` at `MainComponent.cpp:3258-3260`): Deck Insert Before/After/Duplicate/Rename/Close; Layer Duplicate/Rename/Copy-Paste FX/Ignore Column Trigger/Lock Content; Column Insert Before/After/Duplicate/Clear Clips/Remove All Before/After; Clip Select All/Cut/Copy/Paste/Copy-Paste FX/Rename/Show in Finder/New Source/New Effect Clip; Output Windowed/Identify Displays/Test Card; all 7 View panel-toggle items (Signal Bar / Deck / Preview / Inspector / Browser / Timing Window / FPS and Stats).

---

## Section 15: Preferences Dialog

Class declared at `src/ui/PreferencesDialog.h:8`. `juce::DialogWindow` with native title bar (`.cpp:12`), resizable 500x400 to 1200x900 (`.cpp:15`), starts centered 700x500 (`.cpp:16`).

Opened from the Audio-DNA menu → Preferences... via `PreferencesDialog::show(parent)` at `.cpp:24-35`.

**8 tabs**, enum `Tab { General, Audio, Video, MIDI, Recording, Defaults, Feedback, About }` at `.h:29-32`. Tab buttons declared at `.h:40-47`. The tab bar height is 30px (`.h:37`). Tab button widths = `tabBar.getWidth() / 8` (`.cpp:193`). Active tab button styled with `kSurfaceLight` background + `kAccentCyan` text (`.cpp:234-238`); inactive uses `kSurface` background + `kTextSecondary` text.

### 15.1 General tab — `PreferencesDialog.cpp:308-320`

| Control | Type | Default | Initialized | Behavior |
|---|---|---|---|---|
| "Confirm on quit:" | Label + ToggleButton (`quitConfirmToggle_`) | ON | `.cpp:61` | Wired but no action handler (value not used anywhere) |
| "Show Tooltips:" | Label + ToggleButton (`tooltipToggle_`) | ON | `.cpp:65` | `onTooltipToggled` callback at `.h:56`, wired via `.cpp:66-69` — fires when toggled, not wired to anything in MainComponent (no consumer found) |

Layout: two rows of 28px height, label 150w + 8 gap + toggle 28w.

### 15.2 Audio tab — `PreferencesDialog.cpp:322-343`

| Control | Type | Options / Default |
|---|---|---|
| "Sample Rate:" | ComboBox (`sampleRateSelector_`) | "44100", "48000" (default, id=2), "96000" — `.cpp:74-77` |
| "Buffer Size:" | ComboBox (`bufferSizeSelector_`) | "64", "128" (default, id=2), "256", "512", "1024" — `.cpp:81-86` |
| "BPM Detection Range:" | ComboBox (`bpmRangeSelector_`) | "60-200 (Standard)" (default), "80-180 (DJ)", "40-240 (Extended)" — `.cpp:90-93` |

All values currently **set but not wired** — no `onChange` handlers applied to these combos. Pure display.

### 15.3 Video tab — `PreferencesDialog.cpp:345-368`

| Control | Type | Options / Default |
|---|---|---|
| "FPS Target:" | ComboBox (`fpsTargetSelector_`) | "30", "60" (default, id=2), "120" — `.cpp:98-101` |
| "Render Resolution:" | ComboBox (`renderResSelector_`) | "Auto" (default), "1280x720", "1920x1080", "2560x1440", "3840x2160" — `.cpp:105-110` |
| "MilkDrop Presets:" | TextEditor (`milkDropDirEdit_`) + "Browse..." TextButton | Empty default; placeholder "Path to .milk preset folder..." at `.cpp:118`. Browse button launches directory-select FileChooser at `.cpp:123-133`. No change handler — path is entered but not applied. |

### 15.4 MIDI tab — `PreferencesDialog.cpp:210`

Placeholder. Shows message "MIDI settings will be available when MIDI support is added." via `layoutPlaceholderTab` (which is currently a no-op — message not actually drawn, see `.cpp:378-383`).

### 15.5 Recording tab — `PreferencesDialog.cpp:211`

Placeholder. Message "Recording settings will be available when recording is implemented." — no controls.

### 15.6 Defaults tab — `PreferencesDialog.cpp:212`

Placeholder. Message "Default transport, play mode, and blend mode settings." — no controls.

### 15.7 Feedback tab — `PreferencesDialog.cpp:213`

Placeholder. Message "Send feedback about Audio-DNA." — no controls.

### 15.8 About tab — `PreferencesDialog.cpp:370-376`

| Element | Content | Style |
|---|---|---|
| Version label (`versionLabel_`) | "Audio-DNA v0.1.0" | 18pt bold, kAccentCyan, centred. `.cpp:137-141` |
| Credits label (`creditsLabel_`) | "Audio-reactive visual performance tool.\n\nBuilt with JUCE, Aubio, OpenGL 4.1.\n\nC++20 / macOS / Windows / Linux" | kTextSecondary, centred. `.cpp:144-151` |

**Total Preferences controls: 8 interactive (2 on General + 3 Audio + 3 Video) + 1 text editor + 1 browse button + 2 About labels = 14 items.** 5 of 8 tabs are placeholders (MIDI, Recording, Defaults, Feedback, + Browse-button handler that doesn't save anywhere).

---

## Section 9 partial: Display Output

### OutputWindow — `src/ui/OutputWindow.h:69`

- Borderless `juce::DocumentWindow`, title "Audio-DNA Output", no title bar buttons, black background. `.cpp:260-266`
- Uses its own `OutputRenderer` at `.h:17` — a separate `OpenGLRenderer` sharing `FeatureBus`, `MappingEngine`, and `EffectChain` with the main Renderer (`.h:11-16`).
- Owns its own `FullscreenQuad`, `ShaderManager`, `TextureManager` — GL objects are per-context. `.h:48-50`
- Compiles **all effect shaders** into its own GL context in `initShaders()` — explicit list at `.cpp:157-253`. Duplicates 75 effect shaders (ripple, bulge, wave, liquid, kaleidoscope, fisheye, swirl, hue_shift, saturation, brightness, duotone, chromatic_aberration, invert, posterize, color_shift, thermal, color_matrix, pixel_scatter, rgb_split, block_glitch, scanlines, digital_rain, noise_overlay, mirror, pixelate, gaussian_blur, zoom_blur, shake, vignette, motion_blur, glow, edge_detect, perspective_tilt, cylinder_wrap, sphere_wrap, tunnel, page_curl, parallax_layers, polar_coords, twirl, shear, elastic_bounce, ripple_pond, diamond_distort, barrel_distort, sine_grid, glitch_displace, sepia, cross_process, split_tone, color_halftone, ordered_dither, heat_map, selective_color, film_grain, gamma_levels, solarize, crt_simulation, vhs_effect, ascii_art, dot_matrix, crosshatch, emboss, oil_paint, pencil_sketch, voronoi_glass, cross_stitch, night_vision, strobe, pulse, slit_scan, double_exposure, frosted_glass, prism_refract, rain_on_glass, hexagonalize). **Note: this shader list is stale relative to the main Renderer** — does not include time effects, audio-native effects, 3D fractals, torus sources, etc. from P16-P25.
- Thread-safe queue for pending image (`pendingImageFile_`) via mutex; same for pending camera frames (`pendingCameraFrame_`). `.h:54-61`
- Saves `lastImageFile_` to requeue image after GL context recreation. `.h:64`, `.cpp:58-63`
- `loadImage(File)` queues an image load. `.h:84`, `.cpp:34-40`
- Letterbox viewport computed from image aspect ratio. `.cpp:126-138`
- Escape key closes the window (`keyPressed` at `.h:90`, `.cpp:317-326`). Sets `setAlwaysOnTop(false)`, `setVisible(false)`.
- `closeButtonPressed()` — also `setAlwaysOnTop(false)`, `setVisible(false)`. `.cpp:284-288`
- `goFullscreenOnDisplay(display)` — **does NOT use native macOS fullscreen** (would create a new Space and GL context transition can fail). Instead: `setBounds(display.totalArea)`, `setAlwaysOnTop(true)`, `toFront(true)`. `.cpp:296-310`

### Display selection (MainComponent)

Two display selectors exist in parallel:

1. **TopBar `displaySelector_`** — populated in MainComponent constructor at `MainComponent.cpp:449-468`. Items: "Off" (id=1) + one per display "WxH (main)" or "WxH (display N)". `onChange` at `.cpp:469-475`: `openOutputOnDisplay(selected - 2)` or `closeOutput()`.
2. **v1 `displaySelector_`** member in MainComponent — populated by `refreshDisplayList()` at `MainComponent.cpp:1815-1833`. Items: "Off" + "Display N (WxH)[ main]". Hidden in v2 layout (`setVisible(false)` at `MainComponent.cpp:1334`), but still updated.

`openOutputOnDisplay(displayIndex)` at `MainComponent.cpp:1835-1854` — creates the `OutputWindow` on first call, loads current image, goes fullscreen on the chosen display.

`closeOutput()` at `MainComponent.cpp:1856-1863` — hides window and `reset()`s the unique_ptr.

### Test Card, Identify Displays, Blackout, Windowed

**None of these are implemented.** Menu items `kOutputTestCard` (1692), `kOutputIdentifyDisplays` (1691), `kOutputWindowed` (1690) exist but have no handlers — fall through to `default: DBG("not yet implemented")` at `MainComponent.cpp:3258-3260`. No blackout feature found.

### OutputRenderer bugs / notes

- The shader compile list at `OutputWindow.cpp:157-253` is an independent, hand-maintained duplicate of the main Renderer's shader list. Temporal effects (Echo, Freeze, Frame Stutter, Screen Split), audio-native sources, fractals, and torus sources are **not** compiled in the OutputWindow's GL context. The OutputRenderer still calls `effectChain_.render(...)` at `.cpp:143-147` so shaders used by the chain must exist in the OutputWindow's ShaderManager or they'll fail silently.

---

## Section 8 partial: BPM / Tempo (TopBar controls)

TopBar is a `juce::Component` + `juce::Timer` at `TopBar.h:12`, runs at 15Hz (`.cpp:208`). Takes `FeatureBus&` and `Composition&` refs (`.h:15`).

Reads the latest FeatureSnapshot on each timer tick (`.cpp:211-231`) and stores `bpm`, `trackerState`, `beatInBar`, `barPhase`, `beatPhase`, `downbeatDetected`, `phrasePhase`, `barCount`.

### Tempo display — `TopBar.cpp:33-42`, layout at `.cpp:464-469`

- Label `tempoLabel_` — 18pt bold, displays integer BPM ("120") or "---" when no detection. Text color mirrors tracker state:
  - State 0 SEARCHING → red (`kMeterRed`)
  - State 1 LOCKING → yellow (`kMeterYellow`)
  - State 2 LOCKED → green (`kMeterGreen`)
  - `.cpp:246-277`
- "Tempo" word label painted manually above the BPM number (`TopBar.cpp:330-336`).
- Tracker state label `trackerStateLabel_` — 9pt, text = "SEARCHING"/"LOCKING"/"LOCKED"/"?" in matching color. `.cpp:41-43`, `.cpp:466-468`.

### Manual BPM toggle — `TopBar.cpp:87-110`, layout at `.cpp:476-483`

- `manualModeBtn_` — ToggleButton labeled "Manual". `TopBar.h:103`
- When toggled ON:
  - `bpmEditField_` becomes visible, tracker state label hidden.
  - `onManualBpmChanged(true, currentOrFallback120)` fires.
  - Edit field grabs keyboard focus and selects all.
- Edit field `bpmEditField_` — `juce::TextEditor` restricted to 6 chars of "0123456789." (`.cpp:117`). Pressing Enter calls `onManualBpmChanged(true, parsedBpm)`. `.cpp:121-125`
- Wired in MainComponent at `.cpp:482-489`: sets `BPMTracker::setManualMode()` and `setManualBPM()`.

### Tap Tempo — `TopBar.cpp:45-70`

- `tapButton_` (label "Tap"). Tooltip "Tap rhythmically to set BPM manually" (`.cpp:46`).
- Stores up to 8 tap times in `tapTimes_[8]` (`.h:108`). Resets if tap-gap > 2s (`.cpp:50-51`).
- Requires ≥ 2 taps. Computes `tappedBPM = 60 / ((last-first) / (n-1))` and fires `onTapTempo(tappedBPM)` (`.cpp:59-69`).
- Wired in MainComponent at `.cpp:477-480` → `BPMTracker::setManualBPM(tappedBPM)`.

### Resync — `TopBar.cpp:72-85`

- `resyncButton_` (label "Resync"). Tooltip "Reset beat phase to sync with the music".
- Fires `onResync` callback. Button briefly flashes cyan for 200ms.
- Wired in MainComponent at `.cpp:491-501`: resets `beatCounter_`, `lastBeatPhase_`, and calls `BPMTracker::resetBeatPhase()` + `BPMTracker::resetPhrase()`.

### BPM Multiplier buttons — `TopBar.h:74-78`, `TopBar.cpp:128-143`, layout at `.cpp:486-497`

Five TextButtons: `"/4"`, `"/2"`, `"x1"`, `"x2"`, `"x4"`. Default `x1` highlighted with `kAccentCyan.withAlpha(0.3f)`. Clicking any button:
1. Calls `handleMultiplierButton(mult)` where mult is -4/-2/1/2/4 (negative = division).
2. Sets `composition_.bpmMultiplier = mult` (`.cpp:291`).
3. Updates button highlights (other buttons reset, this one highlighted).
4. Fires `onBpmMultiplierChanged(multiplier)` (`.cpp:313-314`) — **not wired to any consumer in MainComponent**.

### Quantize selector — `TopBar.cpp:144-160`, layout at `.cpp:499-502`

`quantizeSelector_` ComboBox with 3 items:
- "Off" (id=1, enum `QuantizeMode::Off = 0`)
- "Next Beat" (id=2)
- "Next Downbeat" (id=3)

Default "Off". Writes to `composition_.quantizeMode` via `onChange` at `.cpp:154-160`. Fires `onQuantizeChanged` — **not wired in MainComponent**.

### Fade slider — `TopBar.cpp:162-177`, layout at `.cpp:504-506`

`fadeSlider_` (ResettableSlider) — range [0.0, 5.0] step 0.01, initial = `composition_.globalTransitionSpeed`, default 0.5 (for right-click reset). Text box on right, 35x20px. Writes to `composition_.globalTransitionSpeed` on change (`.cpp:174-177`).

### Master level slider — `TopBar.cpp:179-189`, layout at `.cpp:520-521`

`masterLevelSlider_` (ResettableSlider) — range [0.0, 1.0] step 0.01, default 1.0. No text box. Wired in MainComponent at `.cpp:443-446` → `renderer.setMasterLevel()`.

### Audio source selector — `TopBar.cpp:15-26`, layout at `.cpp:434-435`

`audioSourceSelector_` ComboBox. Populated in MainComponent at `.cpp:397-399`: "Mic Input" (id=1), "Audio File" (id=2). Wired at `.cpp:400-435` (switches AudioEngine source mode, opens FileChooser for Audio File mode).

### Input gain slider — `TopBar.cpp:16-25`, layout at `.cpp:437-438`

`inputGainSlider_` (ResettableSlider) — range [0.0, 4.0] step 0.01, default 1.0, LinearHorizontal, no text box. Wired in MainComponent at `.cpp:438-440` → `AudioEngine::setInputGain()`.

### Transport buttons — `TopBar.h:63-65`, layout at `.cpp:445-451`

Three TextButtons: `playButton_` (">"), `pauseButton_` ("||"), `stopButton_` ("[]"). **No `onClick` handlers wired** in TopBar or MainComponent — they are declared and shown but do nothing when clicked.

### Beat wheel — `TopBar.cpp:345-399`, layout at `.cpp:457`

- 4-segment circular indicator at `beatWheelBounds_`. Drawn in `paintBeatWheel` (`.cpp:345-399`).
- Radius = min(width,height)*0.45, inner radius = radius*0.5, gap between segments = 0.12 rad.
- Segment at top (12 o'clock) = beat 0; goes clockwise.
- Current beat segment colored `kAccentCyan` with brightness = `1 - beatPhase*0.5` (bright at start of beat).
- Other segments dim (`0xff333333`).
- Only animates when `bpm > 0 && trackerState >= 1` (LOCKING or LOCKED).

### Bar/Phrase readout — `TopBar.cpp:401-427`, layout at `.cpp:461`

Small text panel at `barPhraseBounds_`. Two 9pt lines:
- Top: `"Bar N"` where N = `barCount + 1` (or "Bar -" if no BPM).
- Bottom: `"Phr 0.XX"` where XX = `phrasePhase` formatted to 2 decimals (or "Phr -").

### Ableton Link — no TopBar UI

**Ableton Link is NOT exposed via TopBar.** There is no Link toggle, peer count indicator, or similar in `TopBar.h/cpp`. Link is driven via `linkSync_` in MainComponent's `timerCallback` at `.cpp:1787-1800`: if enabled, overrides BPM tracker with Link tempo. Link enable/disable is not UI-exposed — only programmatically (no menu item, no binding action found).

### Output display selector — `TopBar.h:93-94`, layout at `.cpp:516-518`

`displaySelector_` ComboBox. Populated in MainComponent at `.cpp:449-468` (see Section 9). Wired at `.cpp:469-475`.

### Stats labels — `TopBar.h:97-100`, layout at `.cpp:512-514`

- `fpsLabel_` — 10pt, text "FPS:N" where N = integer fps. Updated in `updateBpmDisplay` at `.cpp:280-281`.
- `dspLabel_` — 10pt, text "DSP:X.Y%". Updated at `.cpp:282-283`.
- Updated via `setFps(fps)` and `setDspLoad(percent)` called from `MainComponent::timerCallback` at `MainComponent.cpp:1773-1775`.

### Separators and layout — `TopBar::resized` at `.cpp:429-522`

Left-to-right: Audio label (38) + AudioSrc combo (90) + 4 gap + Gain label (30) + Gain slider (70) + 6 gap → separator (2) → Play (24) + 1 + Pause (24) + 1 + Stop (24) + 6 gap → separator (2) → Beat wheel (26) + 2 gap → Bar/Phrase (44) + 2 gap → Tempo label (50) + TrackerState label (60 overlay) → 64 gap → Tap (32) + 2 + Resync (50) + 2 + Manual (80) + 2 → (conditional) BPM edit field (60) + 4 gap → Mult buttons (5 × 26 + 4 gaps) + 6 gap → Quantize label (55) + Quantize combo (100) + 6 gap → Fade label (30) + Fade slider (100) + 6 gap → (right-aligned) DSP label (55) + FPS label (45) + 6 gap + DisplaySelector (100) + OutputLabel (42) + 4 gap + MasterSlider (70) + MasterLabel (42).

---

## Section 16 partial: Overlays and modal surfaces

### BindingOverlay — `src/ui/BindingOverlay.h:10`

Full-window semi-transparent overlay (`0xcc000000`) shown when keyboard binding mode is active.

- `enterBindingMode()` at `.cpp:16-30`: sets `active_=true`, `waitingForKey_=false`, `selectedTargetIndex_=-1`, calls `bindingManager_.setBindingMode(true)`, adds self as key listener on top-level component, grabs keyboard focus.
- `exitBindingMode()` at `.cpp:32-45`: reverse of above, fires `onBindingModeExit` callback.
- Entered via keyboard shortcut **Cmd+Shift+K** (`MainComponent.cpp:1599`) or menu Shortcuts → Edit Keyboard Shortcuts.

**Title text** (`.cpp:63-70`):
- Default: "Keyboard Binding Mode — Click a target, then press a key" (18pt bold, kAccentCyan).
- After clicking a target: "Press a key to bind..."
- Below title (12pt, kTextSecondary): "Press Escape to exit" (`.cpp:72-75`).

**Bindable targets** — supplied via `setBindableTargets(targets)`. `BindableTarget` struct at `.h:33-43` contains bounds, label, action enum, and target indices (layer/column/deck/effect/macro).

Target rendering (`.cpp:77-117`):
- Selected target: `kAccentCyan` fill at 0.4f alpha + 2px border.
- Unselected: `kSurfaceLight` 0.5f alpha fill + `kAccentCyan` 0.6f alpha 1px border.
- 11pt label drawn centered (truncates if necessary).
- If already bound: magenta tag bar at bottom showing the current binding via `getKeyDescription` (e.g., "Cmd+Shift+K", "Space", "Left", "5") or MIDI "Note N" / "CC N".

**Mouse behavior** (`.cpp:124-142`): click on a target selects it (`selectedTargetIndex_`), sets `waitingForKey_=true`. Click outside any target deselects.

**Key handling** (`.cpp:144-208`):
- Escape → `exitBindingMode()`.
- Any other key (when waiting for key and a target is selected): removes any existing binding with the exact same key-combo, then adds a new `Binding` with the selected target's action and indices. Resets to select-next-target mode.
- Arrow keys are explicitly allowed at `.cpp:163-169`.
- Always returns `true` — consumes all keys while active.

**Target list** is built by `MainComponent::buildBindableTargets` at `MainComponent.cpp:3446-3541`. Layout:
- Global actions row at y=70, each 100x30: "Tap Tempo", "Resync", "Play / Pause", "Stop", "Master Opacity".
- Column triggers row below at colY=120, each 80x24: "Column 1..20".
- Per layer (top layer = highest index, Resolume-style): 5 layer controls (32px wide each): "L N Bypass", "L N Solo", "L N Mute", "L N Auto", "L N Visible". Then clip cells 80x50 labeled "L N C N".
- Deck switch targets at bottom, each 80x28: "Deck N" (up to 10 decks).

### MidiLearnOverlay — `src/ui/MidiLearnOverlay.h:13`

Nearly identical to BindingOverlay but listens for MIDI notes/CCs instead of keys.

- Constructor at `.cpp:3-8`. Full-window overlay with magenta tint (`0xcc100010` background, `.cpp:86`).
- `enterLearnMode(AudioDeviceManager*)` at `.cpp:17-34`: starts listening on **all available MIDI input devices** (enables if not already, adds self as callback).
- `exitLearnMode()` at `.cpp:36-50`: stops listening, removes callbacks.
- Entered via **Cmd+Shift+M** (`MainComponent.cpp:1606`) or menu Shortcuts → Edit MIDI Mappings.

**Title text** (`.cpp:89-105`):
- Default: "MIDI Learn Mode — Click a target, then send MIDI" (kAccentMagenta, 18pt bold).
- After clicking a target: "Send a MIDI note or CC..."
- Hint below (12pt): "Press Escape to exit  |  Last: <message>".

**Last MIDI message display** (`lastMidiMessage_`) — updated on every incoming MIDI message to show e.g. "Note 60 vel=100 ch=1" or "CC 7 val=64 ch=1".

**MIDI handler** (`handleIncomingMidiMessage`) at `.cpp:190-288`:
- Posted to message thread via `MessageManager::callAsync`.
- For Note On (`.cpp:196-241`): captures noteNumber + velocity + channel, removes any existing binding for this note, adds new `Binding{ MidiNote, targetAction, targetIndices }`.
- For CC (`.cpp:242-287`): captures controllerNumber + value + channel, removes existing binding for this CC, adds new `Binding{ MidiCC, ... }`.
- Updates `lastMidiMessage_` and repaints.

**Escape key** exits learn mode (`.cpp:177-188`); all other keys consumed.

### ProgrammingMode — `src/ui/ProgrammingMode.h:10`

An overlay that expands the SignalBar to full-screen for editing signal routing in detail.

- Owns a reference to `SignalBar&` at `.h:22`.
- `setActive(active)` at `.cpp:15-36`:
  - When active: `signalBar_->setDisplaySize(Expanded)`, shows self, triggers parent `resized()`.
  - When inactive: `setDisplaySize(Normal)`.
- **No menu item wired directly to ProgrammingMode.** Menu View → Programming Mode (`kViewProgrammingMode`, 1807) toggles `signalBar_->getDisplaySize()` between Expanded/Normal directly at `MainComponent.cpp:3184-3195`, without going through the ProgrammingMode class.
- ProgrammingMode is instantiated at `MainComponent.cpp:511` but added as hidden child (`addChildComponent(programmingMode_.get())` at `.cpp:512`). In `resized()` at `.cpp:1309-1310` it is always set invisible. **The class exists but is effectively unused** — the actual programming-mode UI is driven by `SignalBar::setDisplaySize()` alone.
- Contains a header label "Programming Mode" (14pt bold, kAccentCyan) and an "Exit" close button that calls `setActive(false)`. Header 24px tall.

---

## TopBar controls (complete, left to right)

Based on `TopBar::resized` at `src/ui/TopBar.cpp:429-522`:

1. **Audio label** — `audioSourceLabel_` "Audio:" (11pt, kTextSecondary), 38px wide. `TopBar.cpp:11-13`
2. **Audio source combo** — `audioSourceSelector_`, 90px wide. Items "Mic Input", "Audio File" (populated in MainComponent).
3. **Gain label** — "Gain:" (11pt, kTextSecondary), 30px wide.
4. **Input gain slider** — `inputGainSlider_` (ResettableSlider, LinearHorizontal, no textbox), range [0, 4], default 1.0, 70px wide.
5. **Play button** `">"`, 24px wide. No handler wired.
6. **Pause button** `"||"`, 24px wide. No handler wired.
7. **Stop button** `"[]"`, 24px wide. No handler wired.
8. **Beat wheel** — 4-segment circle, 26px wide.
9. **Bar/Phrase readout** — Two text rows, 44px wide: "Bar N" + "Phr 0.XX".
10. **Tempo label** — `tempoLabel_` (18pt bold), 50px wide, with "Tempo" superscript above. Color = tracker state color.
11. **Tracker state label** — `trackerStateLabel_` (9pt), overlays 60px to right of tempo. Shows "SEARCHING"/"LOCKING"/"LOCKED".
12. **Tap button** — "Tap" TextButton, 32px wide. Tooltip: "Tap rhythmically to set BPM manually".
13. **Resync button** — "Resync" TextButton, 50px wide. Tooltip: "Reset beat phase to sync with the music". Flashes cyan on click.
14. **Manual toggle** — `manualModeBtn_` ToggleButton "Manual", 80px wide. Tooltip: "Switch between auto-detect and manual BPM".
15. **BPM edit field** (conditional — only visible when Manual is on) — `bpmEditField_`, 60px wide, 14pt bold. Restricted to "0123456789." (6 chars max). Enter key applies.
16. **Multiplier buttons** — 5 buttons, 26px each: `/4`, `/2`, `x1`, `x2`, `x4`. Active highlight = `kAccentCyan.withAlpha(0.3f)`.
17. **Quantize label** — "Quantize:" (11pt), 55px wide.
18. **Quantize combo** — `quantizeSelector_`, 100px wide. Items: "Off", "Next Beat", "Next Downbeat".
19. **Fade label** — "Fade:" (11pt), 30px wide.
20. **Fade slider** — `fadeSlider_` (ResettableSlider, LinearHorizontal, text box 35x20px right), range [0, 5] step 0.01, default 0.5, 100px wide.
21. **(right-aligned) DSP label** — `dspLabel_` "DSP:X.Y%" (10pt), 55px wide.
22. **FPS label** — `fpsLabel_` "FPS:N" (10pt), 45px wide.
23. **Output display selector** — `displaySelector_`, 100px wide. "Off" + one per display.
24. **Output label** — "Output:" (11pt), 42px wide.
25. **Master level slider** — `masterLevelSlider_` (ResettableSlider, LinearHorizontal, no text box), range [0, 1], default 1.0, 70px wide.
26. **Master label** — "Master:" (11pt), 42px wide.

**Controls NOT present** (but might be expected): Ableton Link toggle, Link peer count display, Record/Snapshot buttons, deck-switch tabs, genre/structural state indicator.

---

## Main.cpp command-line flags

`src/Main.cpp:11-24`. Parsed in `AudioDNAApplication::initialise` via `StringArray::fromTokens(commandLine, " ", "\"")`:

| Flag | Type | Default | Effect |
|---|---|---|---|
| `--test-mode` | boolean | `false` | Sets `testMode = true`. Disables AnalysisThread startup (`MainComponent.cpp:1085-1086`), enables TestServer if `AUDIODNA_TEST_SERVER` build flag set. |
| `--test-port=N` | integer | `8080` | Parsed from `arg.fromFirstOccurrenceOf("=", false, false).getIntValue()`. Passed to `TestServer` constructor. |

**Only 2 command-line flags.** No flags for composition file, audio device, log level, headless mode, or production API port (hardcoded 7070 at `MainComponent.cpp:1116`).

App metadata (`src/Main.cpp:6-9`):
- `getApplicationName()` → "Audio-DNA"
- `getApplicationVersion()` → "0.1.0"
- `moreThanOneInstanceAllowed()` → `false` (single-instance).

Window (`src/Main.cpp:42-76`):
- `MainWindow` extends `juce::DocumentWindow` with `allButtons` and background `0xff1a1a2e`.
- Native title bar (`setUsingNativeTitleBar(true)`).
- Resizable 1280x720 to 3840x2160 (`setResizeLimits` at `.cpp:53`).
- Opens maximized to fill the primary display's `userArea` (`.cpp:65-69`).
- Menu bar installed: on macOS via `setMacMainMenu`, else via `setMenuBar` (`.cpp:56-62`).
- Close button → `systemRequestedQuit()` (`.cpp:87-90`).

---

## MainComponent — layout regions, top-level state, keyboard shortcuts

### Layout regions (MainComponent::resized — `.cpp:1276-1506`)

Top-down:
1. **Top Bar** — full width, 34px tall + 1px gap. `.cpp:1281-1285`.
2. **Signal Bar** — full width, dynamic height (`getPreferredHeight()`). Special case: if `sbHeight < 0`, Signal Bar becomes expanded and consumes all remaining area (Programming Mode) — everything else is hidden (`.cpp:1289-1325`).
3. **Row 1 (v1 compat, 24px tall)** — `.cpp:1347-1369`: "Open Image" (70) + "Image Folder" (75) + "Beats per Image" label (80) + beat count combo (50) + "Camera" label (42, conditional) + camera combo (100) + "Save" (40) + "Load" (40) + "FX Save" (50) + "Deck Save" (60) + "Deck Load" (60) + file label (rest).
4. **Preset slots bar** (bottom, 28px) — `.cpp:1378-1393`: 10 slots each with button (1/3 width) + dropdown (2/3 width).
5. **Deck view** — from top of remaining area. Natural height = `deckView_->getNaturalHeight()`, clamped by user-dragged `deckDividerY_`. `.cpp:1400-1424`.
6. **Horizontal divider** (5px) — `kDividerHeight = 5`. `.cpp:1432`.
7. **Bottom panel area**, split by 3 vertical dividers (5px each) at fractions `vDividerFrac_[3]` (default 0.22, 0.50, 0.75) `.cpp:1442-1460`:
   - **Preview panel** (left fraction) — `previewPanel_` + `waveformDisplay_` below at 12% of preview height (min 30px).
   - **Timing Window** — `timingWindow_` (center-left).
   - **Inspector** — `inspectorPanel_` (center-right).
   - **Browser** — `browserPanel_` (right).

Constraints:
- `kMinDeckHeight = 120`, `kMinBottomHeight = 100` (both `MainComponent.h:234-235`).
- `kMinPanelWidth = 120` (`.h:244`).
- `kVDividerWidth = 5` (`.h:243`).

### Top-level state (MainComponent members)

Owns many managers and singleton-ish objects declared at `MainComponent.h:107-276`:
- `AudioDNALookAndFeel lookAndFeel_`
- `RingBuffer<float> ringBuffer_{16384}`
- `AudioEngine audioEngine_`, `AnalysisThread analysisThread_`
- v1 UI: `WaveformDisplay`, `AudioReadoutPanel`, `SpectrumDisplay`, `PreviewPanel` (hosts OpenGL context)
- `EffectLibrary effectLibrary_` (registerDefaults called at `.cpp:370`)
- `EffectsRackPanel` (v1)
- Slideshow: `slideshowImages_` Array, `slideshowIndex_`, `slideshowBeats_=8`, `slideshowBeatCounter_`, `lastSlideshowBeatPhase_`
- Beat-sync randomize: `beatRandomCount_=4`, `beatCounter_`, `lastBeatPhase_`, `uiUpdateCounter_`
- FX Save: `fastSaveCounter_=1` (scanned from existing `FX_Save_*.json` files at `.cpp:74-87`)
- 10 preset slots at `.h:172`
- Output: `displaySelector_`, `outputWindow_ unique_ptr`, `currentImageFile_`, `currentAudioFile_`
- Camera (conditional on `AUDIODNA_HAS_CAMERA`): `cameraDevice_`, `cameraActive_`, `cameraSelector_`
- v2: `Composition composition_`, `UndoManager undoManager_`, `SignalRegistry signalRegistry_`, `MacroBank globalMacroBank_`, `SessionRecorder sessionRecorder_`, `LinkSync linkSync_`
- v2 panels: `TopBar`, `SignalBar`, `ProgrammingMode`, `DeckView`, `InspectorPanel`, `BrowserPanel`, `TimingWindow`
- `AudioDNAMenuBar menuBarModel_`
- `BindingManager bindingManager_`, `BindingOverlay`, `MidiLearnOverlay`, `MidiHandler`
- `TooltipWindow tooltipWindow_` (600ms delay, `.cpp:380`)
- `tooltipsEnabled_ = true`
- Divider state: `deckDividerY_ = -1` (auto), `draggingDivider_`, `dividerBounds_`, `hoveringHDivider_`, `vDividerFrac_[3] = {0.22, 0.50, 0.75}`, `vDividerBounds_[3]`, `draggingVDivider_`, `hoveringVDivider_`
- Test mode: `testMode_`, `testPort_ = 8080`, optional `TestServer`
- P22: `ApiServer apiServer_` (port 7070), `OscHandler oscHandler_`, `MidiOutputHandler midiOutputHandler_`, `VideoRecorder videoRecorder_`, `SyphonOutput syphonOutput_`

### Keyboard shortcuts (MainComponent::keyPressed — `.cpp:1588-1676`)

All shortcuts are hardcoded in `keyPressed()` and are **not shown in menu items** (menu items are built without `shortcut` argument to `addItem`).

| Shortcut | Line | Action |
|---|---|---|
| **Cmd+Shift+K** | 1599 | Toggle Keyboard binding mode (`enterKeyboardBindingMode()`) |
| **Cmd+Shift+M** | 1606 | Toggle MIDI learn mode (`enterMidiLearnMode()`) |
| **Escape** | 1613 | Close output window if visible + reset display selector to "Off" |
| **Cmd+Z** | 1624 | Undo (`undoManager_.undo()`) |
| **Cmd+Shift+Z** | 1626 | Redo (`undoManager_.redo()`) |
| **Cmd+S** | 1634 | Save preset |
| **Cmd+F** | 1641 | Toggle fullscreen output on primary display |
| **Cmd+O** | 1657 | Load preset |
| Any other key (no Cmd) | 1664-1672 | Delegated to `bindingManager_.processKeyDown()` for custom bindings |

**No shortcuts for**: Cut/Copy/Paste, New deck/layer/column, other common menu commands — menu items have no shortcut column populated.

`keyStateChanged(bool isKeyDown)` at `.cpp:1683-1708` — on key release, polls all momentary keyboard bindings for released keys and fires release action (0.0). JUCE doesn't deliver which key released, so this iterates all bound keys.

### Drag-and-drop (File)

`isInterestedInFileDrag` at `.cpp:1710-1723` — accepts `.wav .aiff .aif .mp3 .flac .ogg .png .jpg .jpeg .gif .bmp .tiff`.

`filesDropped` at `.cpp:1725-1754` — audio files → `audioEngine_.loadFile()` + switch to File source mode + play. Image files → `previewPanel_.loadImage()` + update output window.

### Timer (MainComponent::timerCallback — `.cpp:1756-1813`)

Runs at 30Hz (`startTimerHz(30)` at `.cpp:247`). Every 8th tick (~4Hz):
- Updates `fpsLabel_` (v1, hidden) and `topBar_` FPS/DSP stats.
- At uiUpdateCounter%3==0 (~10Hz): `inspectorPanel_->refresh()` for signal-driven values.

Always:
- Updates Ableton Link state if enabled (`linkSync_.update()` → writes BPM into tracker if > 0).
- At uiUpdateCounter==0 (~4Hz): Updates MIDI output pad feedback via `midiOutputHandler_.updateFromDeck()`.
- If `beatRandomToggle_` is on → `beatSyncRandomize()`.
- If slideshow images loaded → `advanceSlideshow()`.

### Output integrations wiring (MainComponent constructor)

- `apiServer_ = std::make_unique<ApiServer>(...)` + `apiServer_->start()` (port 7070) at `.cpp:1106-1124`.
- `oscHandler_` callbacks (not auto-started — must be started from preferences) at `.cpp:1127-1144`.
- `videoRecorder_.onRecordingFinished` callback at `.cpp:1147-1152`.
- Destructor at `.cpp:1160-1180` stops apiServer, oscHandler, midiOutputHandler, videoRecorder, testServer.

---

## Additional detail — AudioReadoutPanel, Waveform, Spectrum, Timing

### AudioReadoutPanel (left panel) — `src/ui/AudioReadoutPanel.cpp`

Updated at 30Hz (`.cpp:12`). Smooths most fields with EMA alpha = 0.3 (`.cpp:23`). BPM and beat/bar phase use no smoothing (sawtooth values). Onset and downbeat each have a flash decay value (`0.85^frame`) at `.cpp:66-76`.

**Sections rendered in order** (`paint` at `.cpp:81-178`):

**Amplitude section** (`.cpp:96-103`):
- RMS meter (green `kMeterGreen`) — `drawMeter` (horizontal bar) with value text to 2 decimals. `.cpp:97-98`.
- Peak meter (yellow `kMeterYellow`) — same style. `.cpp:99-100`.
- dBFS meter (`.cpp:101`) via `drawDbMeter` — range [-60, 0]. Color = green if < 60% of range, yellow if < 85%, red otherwise.
- LUFS label with value to 1 decimal. `.cpp:102`.
- Crest label (dynamic range, 1 decimal). `.cpp:103`.

**Frequency Bands section** (`.cpp:107-109`):
- `drawBandMeters` (`.cpp:282-305`) — 7 horizontal bars with labels `kBandNames[] = {"Sub", "Bass", "LMid", "Mid", "HMid", "Pres", "Bril"}`, colors `kBandColors[]` (red/orange/amber/green/teal/blue/purple) — `.cpp:57-65`. Each bar: label (30w) + meter background + filled width = band energy clamped to [0,1]. No peak hold markers.

**Rhythm section** (`.cpp:114-148`):
- BPM label with **color coded by tracker state** (SEARCHING=red / LOCKING=yellow / LOCKED=green). 11pt bold integer value, plus state name text (9pt). `.cpp:117-142`.
- **Beat phase meter** via `drawBeatPhase` (`.cpp:307-331`) — magenta bar filling from left to right as `beatPhase` wraps 0→1 with fading alpha. Includes an 8px magenta pip at the current position.
- **Bar indicator** via `drawBarIndicator` (`.cpp:333-398`) — 4 rectangular beat boxes. Beat 1 = downbeat in orange (`0xffff6d00`) with flash; other current beat = magenta. Non-current beats dim outline. Beat number label inside each box (bold for downbeat).
- **Onset indicator** via `drawOnsetIndicator` (`.cpp:400-432`) — circular pulse with cyan glow when `onsetFlash_ > 0.1`. Shows onset strength to 2 decimals.
- **Transient density** label "N/s" to 1 decimal.

**Spectral section** (`.cpp:154-157`):
- Centroid (Hz integer), Flux (2 decimals), Flatness (3 decimals), Rolloff (Hz integer).

**Pitch & Key section** (`.cpp:163-167`):
- Pitch (Hz integer), Confidence (2 decimals), Key (e.g., "C maj" via `keyName` at `.cpp:462-473`), HCDF (3 decimals).

**Structure section** (`.cpp:172-173`):
- `drawStructuralState` (`.cpp:434-458`) — 8px dot + 11pt bold state name ("Normal"/"Buildup"/"Drop"/"Breakdown") in matching color (secondary gray / yellow / red / cyan).

**Genre section** (`.cpp:176-177`, `drawGenreState` at `.cpp:523-584`):
- Genre name with colored dot. Colors at `.cpp:602-616`: House=green, Techno=red, DnB=orange, Hip-Hop=amber, Ambient=light blue, Rock=dark red, Pop/Electronic=purple, Jazz/Other=teal.
- Confidence bar (dark bar + genre-colored fill, 0.7 alpha).
- Energy state label ("Low"/"Medium"/"High") in light blue / amber / red.

### WaveformDisplay — `src/ui/WaveformDisplay.cpp`

Scrolling right-to-left waveform, updated at 30Hz (`.cpp:9`). Pulls `kWaveformBufferSize` samples from `AnalysisThread::getWaveformSamples`.

Per frame, builds one `Column { minVal, maxVal, rms }` from the current raw buffer and pushes into circular buffer `columns_[512]` (`kMaxColumns = 512`, `.h:33`).

Peak hold: tracks max abs amplitude; holds for 30 frames (~1s at 30fps) then decays at 0.97 per frame. `.h:42-43`.

**Display options**:
- Peak envelope drawn as 0.7-alpha cyan strokes (1px) — min + max lines.
- RMS filled area: 0.15-alpha cyan filled path.
- Center horizontal line at 0.2 alpha kTextSecondary.
- Peak hold lines: yellow (0.6 alpha kMeterYellow).
- Rounded corner fill (4px radius, `kSurface` bg + `kPanelBorder` 1px border).

No mode switch (always shows peak + RMS + peak hold).

### SpectrumDisplay — `src/ui/SpectrumDisplay.cpp`

Updated at 30Hz (`.cpp:12`). 7 bars.

**Attack/release envelope smoothing**: `kAttackAlpha = 0.6` fast attack, `kReleaseAlpha = 0.08` slow release (`.h:29-30`).

**Peak hold**: 20 frames hold (~0.67s at 30fps), decay 0.95 per frame (`.h:25-26`).

Bar style (`.cpp:77-123`):
- Title "Spectrum" at top (14pt bold, kTextPrimary).
- Each bar: bottom-up fill with gradient (bright at top, dim at bottom); 3px bright cap at top; 2px-thick peak hold marker line.
- Gap between bars = 2px.
- Label at bottom (9pt, kTextSecondary): "Sub", "Bass", "LMid", "Mid", "HMid", "Pres", "Bril" (`.h:33-35`).

Band colors (`.h:38-46`) slightly different from AudioReadoutPanel: Sub red `0xffff1744`, Bass orange `0xffff6d00`, LowMid **yellow** `0xffffea00` (vs amber in readout), Mid green `0xff00e676`, HighMid **cyan** `0xff00e5ff` (vs teal), Presence blue `0xff2979ff`, Brilliance **purple** `0xffaa00ff` (vs `0xff7c4dff`).

No mode switch — always bars (no line mode).

### TimingWindow — `src/ui/TimingWindow.cpp`

Center-bottom panel with 3 tabs. Tab bar height = 26px (`.h:27`).

**3 tabs** (enum `Tab { BPM, Routing, Oscillators }` at `.h:16`):
- **BPM** (default active) — placeholder, shows "BPM" text centered, 11pt kTextSecondary at 0.4 alpha.
- **Routing** — placeholder, same style.
- **Oscillators** — placeholder, same style.

Active tab visual: 3px cyan accent line under tab (`.cpp:43-45`), button styled with `0xff3a3a5c` bg + white text; inactive = `0xff1a1a2e` bg + `0xff606070` text.

**No actual content** — all three tabs are placeholders (`.cpp:51-65` draws centered placeholder text only). No controls, no BPM graph, no routing display, no oscillator editor implemented.

---

## Look and Feel + Knob details

### AudioDNALookAndFeel — `src/ui/LookAndFeel.cpp`

Color palette (`LookAndFeel.h:10-20`):
- `kBackground = 0xff1a1a2e` (deep navy)
- `kSurface = 0xff252540`, `kSurfaceLight = 0xff30305a`
- `kAccentCyan = 0xff00e5ff`, `kAccentMagenta = 0xffff00e5`
- `kTextPrimary = 0xffe0e0e0`, `kTextSecondary = 0xff808090`
- `kMeterGreen = 0xff00e676`, `kMeterYellow = 0xffffea00`, `kMeterRed = 0xff1744` ← note the 4th byte: `kMeterRed = 0xffff1744`
- `kPanelBorder = 0xff3a3a5c`

Sets default typeface to 14pt sans-serif.

Custom drawing for: `drawButtonBackground` (flat rect + panel border, brighter when down/hover), `drawButtonText` (14pt, respects button's textColourOffId), `drawRotarySlider` (cyan filled arc + inner circle + pointer), `drawLinearSlider` (horizontal and vertical, 4px track + 14px circular thumb + hover glow), `drawToggleButton` (16px square with checkmark when toggled), `drawComboBox` (flat rect + cyan arrow triangle), `drawPopupMenuBackground` + `drawPopupMenuItem` (separator lines, highlight with 15% cyan, shortcut text in kTextSecondary, submenu arrow), `drawLabel`, `drawScrollbar` (rounded thumb, hover/down colors).

### Knob — `src/ui/Knob.cpp`

Rotary slider parameter control.

- Preferred size: 64x80px (`Knob.h:36-37`).
- Internal `ResettableSlider` (RotaryVerticalDrag, no text box, range [0, 1] step 0.001, default 0.0). Scroll wheel disabled (`.cpp:11`). Fill color `kAccentCyan`, outline `kSurfaceLight`.
- Name label at bottom (10pt, kTextSecondary, centredTop).
- Value label above name (10pt, kTextPrimary, centred) — auto-updates to 2 decimals on value change (`.cpp:34-38`).
- **Mapping indicator ring** — drawn behind the slider when `mapped_ = true`. Magenta arc (alpha 0.6f, 2px stroke) from 1.25π to 2.75π (i.e., a full 270° arc matching the rotary range). Set via `setMappingIndicator(sourceName)` where non-empty name enables the ring. `.cpp:44-70`.

---

## PreviewPanel details

`src/ui/PreviewPanel.h:12` — hosts the main OpenGL Renderer.

**2 tabs** (enum at `.h:38`): `Preview` (default), `Output`. Tab buttons:
- "Preview" `previewTabBtn_` and "Output" `outputTabBtn_`. Each 80px wide max (`.cpp:45`). Tab bar 26px tall.
- Active tab: `kSurfaceLight` bg + `kAccentCyan` text. Inactive: `kSurface` bg + `kTextSecondary` text. `.cpp:91-112`.
- Both tabs currently render the **same content** (single renderer) per `.cpp:51` comment: "Preview = selected clip solo, Output = full composition (Both render from the same renderer for now)".

Owns a `Renderer renderer_` and inner `GLHost` component that hosts the GL context (so the tab bar stays above the GL surface; standard JUCE pattern for mixing 2D widgets with GL).

API:
- `loadImage(File)` — renderer.loadImage + sets `imageLoaded_` state.
- `clearImage()` — renderer.clearImage + clears state.
- `queueCameraFrame(Image)` — passes frame to renderer; marks as loaded.
- Accessors: `getMappingEngine()`, `getEffectChain()`, `getRenderer()`.

GLHost draws "Load an image to see audio-reactive effects" message (14pt, `0xff666666`, `0xff0a0a14` background) when no image is loaded (`.h:49-60`).

---

## Section 20 partial: Cross-references

### Menu commands that delegate to other slices
- **Audio-DNA → Preferences** → `PreferencesDialog::show()` (this slice, `PreferencesDialog.cpp`).
- **Audio-DNA → Import ISF Shader** → `ISFShaderLoader::parseISFFile` (effects slice) + `EffectLibrary::registerDynamic` (effects slice).
- **Composition → Undo/Redo** → `core/UndoManager` (core slice).
- **Composition → Save/Open** → `PresetManager` (preset/file slice).
- **Composition → Collect Media / Relocate Files** → iterates `composition_.decks`, rewrites `clip.mediaFile` (model slice).
- **Deck/Layer/Column/Clip menus** → `Deck`, `Layer`, `Clip` model methods (model slice).
- **Output → Fullscreen N** → `OutputWindow::goFullscreenOnDisplay` (this slice).
- **Output → Snapshot** → `Renderer::takeSnapshot` on detached thread (render slice).
- **Output → Start/Stop Recording** → `VideoRecorder` (recording slice).
- **Shortcuts → Edit Keyboard/MIDI** → `BindingOverlay` / `MidiLearnOverlay` (this slice).
- **Shortcuts → Export/Import Bindings** → `BindingManager::saveToFile` / `loadFromFile` (binding slice).
- **View → Programming Mode** → `SignalBar::setDisplaySize` (signal slice).
- **View → Save/Load/Reset Layout** → local divider state + JSON file (layout slice).

### Top-level state consumers
- `composition_` is referenced by `topBar_`, `deckView_`, `inspectorPanel_`, `browserPanel_`, `bindingOverlay_`, `midiLearnOverlay_`, `apiServer_`, `oscHandler_`, `Renderer` (via `setComposition`, `setActiveDeck`, `setPerTypeAutopilotConfig`).
- `analysisThread_` feeds: `audioReadoutPanel_`, `spectrumDisplay_`, `topBar_`, `previewPanel_` (via FeatureBus), `apiServer_`, `testServer_` (via FeatureBus).
- `signalRegistry_` feeds: `signalBar_`, `inspectorPanel_`, `programmingMode_` (via signalBar), `Renderer` (setSignalRegistry), `apiServer_`, `testServer_`.
- `bindingManager_` feeds: `bindingOverlay_`, `midiLearnOverlay_`, `midiHandler_`, and is called into by `keyPressed` (keyboard) and `midiOutputHandler_` (MIDI).

### External integration surfaces
- REST API on port 7070 (always on) — `ApiServer` wired at `.cpp:1106-1124`. Callbacks for `onTriggerClip`, `onTriggerColumn`, `onSwitchDeck`, `onSnapshot`.
- OSC input (not auto-started; starts on demand from preferences) — `OscHandler` wired at `.cpp:1127-1144`. Callbacks for `onTriggerClip`, `onSwitchDeck`, `onSetMaster`, `onSetLayerOpacity`, `onSnapshot`.
- MIDI output to Launchpad — polled from `midiOutputHandler_.updateFromDeck(composition_.getActiveDeck())` at ~6Hz in timer.
- Video recording — `VideoRecorder videoRecorder_`, saves to `~/Documents/Audio-DNA/Recordings/recording_YYYYMMDD_HHMMSS.mp4` (H264/1920x1080/30fps/CRF 23).
- Syphon output — `SyphonOutput syphonOutput_` plumbed via `renderer.setSyphonOutput()` at `.cpp:390`.
- Test server (conditional build) — starts on `--test-mode` on the `--test-port` (default 8080).

### Effect shader lists (stale duplication)
- `OutputRenderer::initShaders` at `OutputWindow.cpp:157-253` compiles a **hand-maintained, stale shader list** (~75 shaders) that does not match the main Renderer's library. Time effects (Echo, Freeze, Screen Split, Frame Stutter, Posterize Time, Channel Delay), audio-native effects/sources, 3D fractals, torus sources, and many P15-P25 additions are **not in the output renderer's compile list**.

### Surprises / notable gaps
1. **Ableton Link is not UI-exposed.** Works programmatically via `linkSync_.setEnabled()` but no toggle in TopBar, no menu item, no preferences control. Peer count is not shown.
2. **Transport buttons in TopBar are dead.** Play/Pause/Stop buttons have no onClick handlers.
3. **ProgrammingMode class is unused.** Instantiated but always hidden. View → Programming Mode directly manipulates SignalBar, bypassing this class.
4. **Menu items carry no keyboard shortcuts in their display.** All shortcuts are hardcoded in `MainComponent::keyPressed` — menu shortcutKeyText column is always empty.
5. **~35 menu items fall to "not yet implemented" default.** Including most Clip menu items (Select All/Cut/Copy/Paste/Rename/Show in Finder/New Source/New Effect Clip), several Column/Deck/Layer items, and 7 of 12 View items (all the panel-visibility toggles).
6. **5 of 8 Preferences tabs are placeholders.** MIDI, Recording, Defaults, Feedback are pure placeholders; even Audio tab's controls are not wired to anywhere. Only MilkDrop preset path has a functional Browse button but no save/apply.
7. **Two display selectors exist in parallel** — v1 `displaySelector_` in MainComponent (hidden but still updated) and the v2 one in TopBar.
8. **TimingWindow has 3 tabs but zero content.** BPM/Routing/Oscillators tabs all show placeholder text only.
9. **BPM multiplier `onBpmMultiplierChanged` callback is wired up in TopBar but has no consumer** in MainComponent — the multiplier only writes to `composition_.bpmMultiplier` directly.
10. **Quantize `onQuantizeChanged` callback also has no consumer** — only writes to `composition_.quantizeMode`.
11. **Preview panel's "Output" tab is a visual duplicate** of the Preview tab (same renderer drives both).
12. **Escape in the main window resets the v1 displaySelector to id=1** (`MainComponent.cpp:1618`) but does not reset the TopBar display selector — inconsistent state after Escape.
13. **About menu item opens Preferences without switching to About tab** — a TODO comment acknowledges this at `MainComponent.cpp:2728`.
14. **The production REST API port (7070) is hardcoded** with no preferences/config to change it.
15. **No Preferences save/load mechanism.** All Preferences values are reset on each app launch — there is no settings persistence.
