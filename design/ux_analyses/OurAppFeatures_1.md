# Audio-DNA — Complete Feature & Control Inventory

---

## Application Overview

Audio-DNA is a live audio-reactive visual performance application. It analyzes audio in real-time, applies shader effects to images/videos/procedural sources, and outputs the result to any display. Users build visual scenes in a deck/layer/clip grid, wire audio features to effect parameters, and perform live with keyboard/MIDI triggers.

---

## 1. Menu Bar

9 menus across the native title bar.

### Audio-DNA
- Preferences — app settings dialog
- Import ISF — import Interactive Shader Format files
- About — version info
- Quit

### Composition
- New / Open / Save / Save As — composition file management
- Undo / Redo — action history
- Copy Effects / Paste Effects — transfer effect chains between clips
- Collect Media — package composition + all referenced media into one folder
- Relocate Files — find and relink missing media files

### Deck
- New / Insert Before / Insert After / Duplicate — deck management
- Rename / Close / Clear Clips / Remove

### Layer
- New / Insert Above / Insert Below / Duplicate — layer management
- Rename / Copy Effects / Paste Effects / Clear Clips / Remove
- Ignore Column Trigger — this layer won't respond to column triggers
- Lock Content — prevent media replacement
- Fold — collapse layer row to save space
- Move Up / Move Down — reorder layers

### Column
- New / Insert Before / Insert After / Duplicate
- Clear Clips / Remove / Remove All Before / Remove All After

### Clip
- Select All / Cut / Copy / Paste
- Copy Effects / Paste Effects
- Rename / Clear / Show in Finder
- New Source — create procedural source clip
- New Effect — add effect to selected clip
- Replace Content — swap media, keep effects and settings
- Lock Content — prevent accidental replacement

### Output
- Disabled — no external output
- Fullscreen on [display name] — one entry per connected display
- Windowed — floating output window
- Identify Displays — flash display names on each monitor
- Test Card — show calibration pattern
- Snapshot — capture PNG screenshot
- Start Recording / Stop Recording — video capture to file

### Shortcuts
- Edit Keyboard Bindings — enter keyboard binding mode
- Edit MIDI Bindings — enter MIDI learn mode
- Stop All — halt all playback
- Export Bindings / Import Bindings — save/load binding presets

### View
- Signal Bar / Deck / Preview / Inspector / Browser / Timing Window — show/hide panels
- FPS Stats — toggle performance overlay
- Programming Mode — expanded signal view
- Save Layout / Load Layout / Reset Layout — workspace layout management

---

## 2. Top Bar (34px, full width)

Left to right:

| Control | Type | Purpose |
|---------|------|---------|
| Audio source | Dropdown | Mic Input or Audio File |
| Gain | Slider (0–4x) | Input level, right-click resets to 1.0 |
| Play / Pause / Stop | 3 buttons | Global transport controls |
| Beat wheel | 4-segment circle | Visual beat position indicator (beats 1-4) |
| Bar / Phrase | Text display | "Bar N" and "Phr 0.XX" next to beat wheel |
| BPM | Large number (18px) | Current tempo, color-coded: gray=searching, yellow=locking, green=locked |
| Tracker state | Label | "SEARCHING", "LOCKING", or "LOCKED" |
| Tap | Button | Tap tempo (accumulates 8 taps, computes BPM) |
| Resync | Button | Reset beat phase to align with current audio |
| Manual | Toggle | Freeze BPM detection, enter manual value |
| BPM edit field | Text input | Manual BPM entry (visible only in manual mode) |
| /4, /2, x1, x2, x4 | 5 buttons | BPM multiplier (÷4, ÷2, ×1, ×2, ×4) |
| Quantize | Dropdown | Off / Next Beat / Next Downbeat — clip launch timing |
| Fade | Slider (0–5s) | Global transition speed between clips |
| Master | Slider (0–1) | Master output brightness/opacity |
| Output | Dropdown | Select output display (Off / Fullscreen / Windowed) |
| FPS | Label | Real-time frame rate |
| DSP | Label | CPU load percentage |

---

## 3. Signal Bar (below top bar, collapsible)

A horizontal strip of audio feature meters. Three display sizes:

| Size | Height | Shows |
|------|--------|-------|
| Minimized | ~20px | Tiny meter bars only |
| Normal | ~80px | Label + meter bar + peak hold + numeric value |
| Expanded | Fill | Full oscilloscope / histogram per signal |

**Controls:**
- [+] button — add new signal (oscillator, envelope, etc.)
- [−] / [+] buttons — shrink/grow all strips
- Click any strip — opens Signal Inspector

**Default signals shown:** RMS, Peak, Bass, Mid, High, Beat Phase, Bar Phase, BPM, Onset Strength, Dominant Pitch, Musical Key, Structural State

---

## 4. Deck Grid (center area)

### Structure
- N layers (rows) × M columns, default 3×12
- Layer strips on the left (250px wide)
- Clip cells fill the rest (90×96px each)
- Column trigger buttons across the top (22px)
- Deck tabs across the bottom (24px)
- Scrollable horizontally and vertically

### Column Trigger Buttons
- Numbered 1, 2, 3... — one per column
- Click = trigger all clips in that column simultaneously (unless layer has "Ignore Column Trigger")
- Active column highlighted cyan

### Deck Tabs
- One tab per deck — click to switch the active deck
- Active deck highlighted

### Layer Display Order
- Highest layer at top of screen, lowest at bottom (like Photoshop/Resolume)

---

## 5. Layer Strip (250px wide, per layer)

Dense Resolume-style header. Each strip contains:

### Row 1: Action Buttons
| Button | Purpose |
|--------|---------|
| X | Clear/stop the active clip on this layer |
| B | Bypass — skip this layer during rendering (dark red when active) |
| S | Solo — render only this layer (olive when active) |

### Row 1: Transport Buttons
| Button | Purpose |
|--------|---------|
| < | Play backward (set reverse) |
| \|\| | Pause playback |
| > | Play forward |
| >| | Fast forward (2x speed) |

### Vertical Sliders (color-coded)
| Slider | Label | Range | Default | Color | Purpose |
|--------|-------|-------|---------|-------|---------|
| S | Speed | 0–4x | 1x | Cyan | Playback speed multiplier |
| K | Key | 0–1 | 0.1 | Amber | Keying threshold |
| V | Opacity | 0–1 | 1.0 | Teal | Layer visibility |
| F | Fade | 0–4s | 0.3s | Blue | Transition speed between clips |

### Dropdowns
| Dropdown | Position | Purpose |
|----------|----------|---------|
| V dropdown | Below V slider | Blend mode (48 modes) + Keying mode (13 modes) |
| F dropdown | Below F slider | Transition mode for clip changes |

### Painted Elements
- Layer name (editable)
- Clip thumbnail
- Clip name + playhead position (cyan vertical line)
- In/out range visualization

### Interactions
- Click layer name → select layer (opens Layer Inspector)
- Drag layer up/down → reorder layers
- Right-click → context menu

---

## 6. Clip Cell (90×96px per cell)

### Visual Elements
- **Thumbnail area** (76px) — image preview, CSS gradient for sources, "SRC" label for procedural sources
- **Name bar** (20px) — clip name, "FX -" prefix for FX-only clips

### Visual States
| State | Indicator |
|-------|-----------|
| Empty | Dark background, no content |
| Loaded | Thumbnail + name |
| Active (playing) | Cyan 2px border |
| Selected (for editing) | White 2px border |
| Missing file | Red border + "!" marker (top-left) |
| Content locked | Orange "L" marker (top-right) |
| Has effects | FX count badge |

### Interactions
- **Click thumbnail** → trigger/retrigger clip
- **Click name bar** → select for inspector editing (Cmd/Shift for multi-select)
- **Drag name bar** → move clip to another cell
- **Drag file from OS** → load image, video, or image sequence
- **Drag from FX Browser** → add effect to clip
- **Drag from Sources Browser** → create procedural source clip
- **Drag from MilkDrop Browser** → create MilkDrop preset clip

---

## 7. Bottom Panels (4 resizable panels side by side)

Separated by 3 draggable vertical dividers. Default proportions: 22% / 28% / 25% / 25%.

### 7A. Preview Panel (left, ~22%)

- **Two tabs:** Preview / Output
- **Preview area:** OpenGL-rendered output of the selected clip or full composition
- **Waveform display** (bottom ~12%): Scrolling time-domain waveform with peak envelope and RMS fill

### 7B. Timing Window (center-left, ~28%)

- **Three tabs:** BPM / Routing / Oscillators
- Placeholder for BPM visualization, signal routing display, and oscillator waveforms

### 7C. Inspector Panel (center-right, ~25%)

- **Four tabs:** Clip / Layer / Composition / Signal
- **Pin button** — prevent auto-tab-switching during performance
- Auto-switches: click clip → Clip tab, click layer → Layer tab, click signal → Signal tab

### 7D. Browser Panel (right, ~25%)

- **Six tabs:** Files / FX / Sources / Comp-Decks / Record / MilkDrop

---

## 8. Clip Inspector

Sections from top to bottom:

### 8.1 Name Bar
- Clip name in bold text

### 8.2 Dashboard (8 Link Knobs)
- 8 rotary knobs in a row
- Each knob: manual value OR connected to a signal source
- Click source button → select signal (Manual / Audio / BPM Sync / Oscillator / Envelope / Clip Position / Macro)
- Labels are renameable

### 8.3 Transport
| Control | Purpose |
|---------|---------|
| Mode dropdown | Timeline or BPM Sync |
| Timeline bar | Draggable in/out points + playhead. Beat markers in BPM Sync mode |
| Play backward / Pause / Play forward | Playback direction |
| Loop mode | Loop / Ping Pong / One Shot |
| Trigger mode | Restart / Continue / Relative |
| Speed slider (0–4x) | Playback rate. ÷2 and ×2 buttons. Reverse toggle |
| Duration slider | Playback length (0.1–300s in Timeline, 1–64 beats in BPM Sync) |
| Beats/Cycle dropdown | BPM Sync only: 1/4 to 16 beats per playback cycle |
| Content Beats slider | BPM Sync only: how many beats the content contains |
| Images/Sec slider | Image sequence only: playback FPS |

### 8.4 Cuepoints
- 8 trigger buttons (numbered 1-8) — click to jump to that cuepoint
- 8 set buttons — click to set cuepoint at current playhead
- Right-click/Ctrl-click trigger button to clear cuepoint

### 8.5 Autopilot
| Control | Purpose |
|---------|---------|
| Action dropdown | Layer Determined / Do Nothing / Play Next / Previous / Random / First / Last / Specific |
| Duration dropdown | 1/4, 1/2, 1, 2, 4, 8, 16 beats, or Layer Determined |
| Beat Snap dropdown | Snap Off / Beat / Bar / 2 Bar / 4 Bar |

### 8.6 Source Parameters (procedural sources only)
- One slider per source parameter
- Each has a signal connect triangle (click to wire audio feature)
- Right-click slider → reset to default value
- Parameter names and ranges vary by source type

### 8.7 Video
| Control | Purpose |
|---------|---------|
| Opacity slider (0–1) | Per-clip transparency. Signal connectable |
| Width / Height | Pixel dimensions with inc/dec buttons |
| Blend Mode dropdown | Layer Determined / Normal / Additive / Screen / Multiply |
| Alpha Type dropdown | Premultiplied / Straight |
| R G B A toggles | Channel visibility toggles |

### 8.8 Transform
All controls have signal connect triangles for audio-driven animation:
| Control | Purpose |
|---------|---------|
| Position X / Y | Pixel offset from center |
| Scale | Size multiplier (1.0 = 100%) |
| Rotation | Degrees |
| Anchor | Pivot point offset |

### 8.9 Effects Stack
- Vertical list of applied effects
- **Per effect row:** [B] bypass toggle, effect name, collapsed first-param value, [expand arrow]
- **Expanded:** Dry/Wet slider + all parameters as signal-connectable sliders
- **Drag-drop:** Drag effects from FX Browser to add
- **Right-click slider** → reset parameter to default

---

## 9. Layer Inspector

Sections from top to bottom:

### 9.1 Name
- Editable layer name — click to rename

### 9.2 Dashboard (8 Link Knobs)
- Same as Clip Inspector dashboard

### 9.3 Autopilot
| Control | Purpose |
|---------|---------|
| Direction buttons | Rewind / Off / Forward / Random |
| Trigger mode | End of Video or On Beat |
| Beat count | 1 / 2 / 4 / 8 / 16 / 32 beats (On Beat mode only) |
| Loops slider (1–99) | Number of loops before advancing |

### 9.4 Layer Master
| Control | Purpose |
|---------|---------|
| Master opacity | Signal-connectable slider (0–1) |
| Persistent toggle | Keep rendering when deck is not active |
| Ignore Column Trigger | Don't respond to column trigger buttons |

### 9.5 Video
| Control | Purpose |
|---------|---------|
| Blend Mode dropdown | 48 blend modes organized in sections (Compositing, Light, Compare, Dodge/Burn, Inversion, Component, Special) |
| Opacity slider (0–1) | Signal-connectable |
| Width / Height | Layer canvas dimensions |
| Auto Size dropdown | Off / Fill / Fit / Stretch / Original |

### 9.6 Transition
| Control | Purpose |
|---------|---------|
| Blend Mode dropdown | Transition style (Cut, Dissolve, Wipe, Push, Zoom, 3D, Color Fade, Creative — 45+ modes) |
| Duration slider (0.1–10s) | Transition length |

### 9.7 Keying (Transparent layers only)
| Control | Purpose |
|---------|---------|
| Mode dropdown | Alpha, Luma Key, Inverted Luma, Luma Is Alpha, Chroma Key, Max RGB, Saturation Key, Edge Detection, Threshold Mask, Channel R/G/B (13 modes) |
| Threshold slider (0–1) | Key cutoff point |
| Softness slider (0–1) | Key edge softness |

### 9.8 Dry/Wet (FX Only layers only)
- Mix slider (0–1) — blend between original accumulator and effects result

### 9.9 3D Controls (3D layers only)
- Rotation X / Y / Z sliders
- Rotation Speed slider
- Scale slider

### 9.10 Transform
- Same as Clip Inspector: Position X/Y, Scale, Rotation, Anchor — all signal-connectable

### 9.11 Feedback (Larsen loop)
| Control | Purpose |
|---------|---------|
| Enable toggle | Turn feedback on/off |
| Preset dropdown | Zoom In / Spiral / Drift / Kaleidoscope / Echo / Stretch |
| Amount slider (0–1) | How much previous frame bleeds through |
| Scale X / Scale Y | Per-frame zoom |
| Rotation | Per-frame rotation |
| Offset X / Offset Y | Per-frame drift |
| Luma Key slider (0–1) | Fade dark areas from feedback |

### 9.12 Layer Effects
- Same as Clip Inspector effects stack — applied after clip compositing

---

## 10. Composition Inspector

### 10.1 Name + Resolution
- Composition name and current output resolution

### 10.2 Dashboard (8 Link Knobs)
- Global-scope macro knobs

### 10.3 Autopilot
| Control | Purpose |
|---------|---------|
| Direction buttons | Rewind / Off / Forward / Random |
| Duration mode | Longest Clip / Clip Transport / Custom |
| Clip Loops slider (1–99) | Loops before advancing |
| Loop toggle | Restart sequence when complete |
| Master Layer dropdown | Which layer drives autopilot timing |

### 10.4 Per-Type Autopilot
| Control | Purpose |
|---------|---------|
| Enable toggle | Activate per-type cycle timers |
| Opaque Beats (1–64) | Beat count for opaque layer cycling |
| Transparent Beats (1–64) | Beat count for transparent layer cycling |
| Transparent Randomize | Random vs sequential clip advance |
| Effect Beats (1–64) | Beat count for FX layer cycling |
| Effect Randomize | Random vs sequential |

### 10.5 Composition Master
- Master slider (0–1) — overall output level, signal-connectable
- Speed slider (0–4x) — global speed multiplier, signal-connectable

### 10.6 Video
- Opacity slider (0–1) — composition-level opacity, signal-connectable

### 10.7 Transform
- Position X/Y, Scale, Rotation, Anchor — applied to final composited output

### 10.8 Global Effects
- Effects stack applied after all layers are composited — same UI as clip/layer stacks

### 10.9 Output Settings
- Resolution dropdown: 1920×1080 / 1280×720 / 2560×1440 / 3840×2160

---

## 11. Signal Inspector

Shown when a signal is selected in the Signal Bar.

### For Audio Signals
| Control | Purpose |
|---------|---------|
| Threshold slider (0–1) | Minimum value to pass through |
| Gain slider (0–4x) | Amplification multiplier |
| Falloff slider (0–1s) | Decay rate when signal drops |

### For Oscillator Signals
| Control | Purpose |
|---------|---------|
| Wave Shape dropdown | Sine / Saw Up / Saw Down / Triangle / Square |
| Beat Duration dropdown | 1/4 / 1/2 / 1 / 2 / 4 / 8 beats per cycle |
| Amplitude slider (0–1) | Output scaling |
| Phase Offset slider (0–1) | Phase shift |

### For Envelope Signals
| Control | Purpose |
|---------|---------|
| Curve Type dropdown | Linear / Exponential / S-Curve |
| Beat Duration dropdown | 1 / 2 / 4 / 8 / 16 beats per cycle |
| Amplitude slider (0–1) | Output scaling |
| Phase slider (0–1) | Phase shift |
| Looping toggle | Repeat envelope |
| One Shot toggle | Play once and hold |
| Curve Editor | Visual envelope with draggable control points |

---

## 12. Browser Panel

### 12A. Files Browser
- Grid view (64px thumbnails) or List view
- Path bar (editable) + Up button
- Search field (case-insensitive filter)
- Favorites system
- Drag files to deck cells to load

### 12B. FX Browser
- 135 effects across 11 categories (collapsible sections)
- Categories: Warp (28), Color (31), Glitch (15), Blur/Post (10), 3D/Depth (10), Pattern (15), Animation (4), Time (6), Audio (7), Blend (5), Composite (3)
- Search field filters by name
- Multi-select with Shift/Ctrl
- Drag effects to cells, inspectors, or effect stacks

### 12C. Sources Browser
- 81 procedural sources across 15 categories (collapsible sections)
- Categories: Noise (3), Fractal (14), Geometric (13), Pattern (8), Organic (2), Nature (5), Audio-Visual (9), 3D (20), Lines (11), Wireframe (7), Text (2), Math (9), Lighting (1), Particle (3), Simulation (3), Routing (1), Special (1)
- Search, multi-select, drag to deck cells

### 12D. Comp-Decks Browser
- Two sections: Compositions and Decks
- Save / Load / Delete for each
- Stores full app state (compositions) or deck templates (decks)

### 12E. Record Panel
| Control | Purpose |
|---------|---------|
| Record button | Start session recording |
| Stop button | End recording |
| Play button | Playback recorded session |
| Save / Load | Export/import as JSON |
| Browse Output Folder | Choose save directory |
| Status / Event count | Recording state display |

### 12F. MilkDrop Browser
- ~9,800 MilkDrop presets via projectM
- **4 sub-tabs:** Curated (30 presets) / Favorites / Recent (last 20) / All (searchable)
- **3 play modes:**
  - Jukebox — auto-advance through presets with configurable timing and blend
  - VJ Clip — click to preview, drag single preset to deck cell
  - Playlist — multi-select presets, drag group to create cycling playlist clip

| Jukebox Control | Purpose |
|-----------------|---------|
| Play / Stop | Start/stop auto-advance |
| Pool dropdown | All / Curated / Favorites |
| Timing | 4 / 8 / 16 / 32 beats or 10 / 30 / 60 seconds |
| Blend slider | Crossfade duration between presets |

| Navigation | Purpose |
|------------|---------|
| < / > buttons | Previous / next preset |
| ? button | Random preset |
| Lock button | Freeze current preset |

---

## 13. Universal Parameter Control (reusable widget)

Appears on every editable parameter throughout the app.

### Collapsed (24px height)
```
[▶] Label          0.50  [-] [+]  [═══════╪═══════]
```
- **Signal triangle** (left) — gray = manual, cyan = connected to signal
- **Label** — parameter name
- **Value** — current numeric value
- **Inc/Dec buttons** — fine adjustment
- **Slider** — drag to adjust, right-click to reset to default

### Expanded (click triangle or label)
- **Source mode button** — Manual / Signal / BPM Sync / Oscillator / Envelope / Clip Position / Timeline / Macro
- **Invert checkbox** — flip 0↔1
- **Range sliders** — Min and Max output range
- **Source value meter** — live visualization of connected signal value

---

## 14. Binding System (Keyboard & MIDI)

### Input Types
- Keyboard (key + Shift/Cmd/Alt modifiers)
- MIDI Note (channel + note number + velocity)
- MIDI CC (channel + CC number, absolute or relative/endless encoder)

### Trigger Modes
- Toggle — press = on, press again = off
- Momentary — press = on, release = off (piano mode)

### Targeting Modes
- By Position — targets clip/layer at grid index
- This Item — targets specific clip by ID (follows if moved)
- Selected — targets current UI selection

### Available Actions
| Action | Purpose |
|--------|---------|
| Trigger Clip | Fire a specific clip |
| Trigger Column | Fire all clips in a column |
| Toggle Layer Bypass/Solo/Mute/Visible | Layer state toggles |
| Toggle Layer Autopilot | Autopilot on/off |
| Layer Transport | Play/pause/reverse |
| Toggle Effect Bypass | Effect on/off |
| Adjust Macro | CC continuous control of dashboard knob |
| Adjust Layer Opacity | CC continuous layer opacity |
| Switch Deck | Switch active deck |
| Tap Tempo / Resync | BPM controls |
| Global Play/Pause / Stop | Transport |
| Master Opacity | CC continuous master level |
| Snapshot | PNG screenshot |
| Toggle Recording | Video record on/off |

### MIDI Velocity
- Optional: MIDI velocity maps to clip opacity on trigger

---

## 15. Signal & Routing System

### Signal Types
| Type | Source | Purpose |
|------|--------|---------|
| Audio | 80+ FeatureSnapshot fields | Real-time audio analysis (RMS, bands, BPM, pitch, etc.) |
| Oscillator | BPM-locked waveform | Sine/Saw/Triangle/Square synced to beat |
| Envelope | Custom curve | Draggable control points, looping/one-shot |
| Clip Position | Playhead normalized 0–1 | Animation tied to clip timeline |
| Chained | Signal modulating signal | Multiply/Add/Gate/ScaleRange one signal by another |

### Routing
- Any signal can drive any parameter at any scope (clip, layer, global)
- Per-route settings: output range, invert, threshold, gain, falloff
- Multiple routes can target the same parameter (values summed)

### 24 Curve Types
- Basic: Linear, Exponential, Logarithmic, S-Curve, Stepped, Hold
- Easing: Circular (In/Out/InOut), Back, Elastic, Bounce, Cubic, Sine (3 each)

### 8 Dashboard Macro Knobs (per scope)
- Each knob: manual value OR driven by a signal
- Each knob distributes its value to linked parameters
- Three scopes: per-clip, per-layer, per-composition

---

## 16. Output & Recording

### Output Window
- Separate fullscreen or windowed display on any connected monitor
- Shows the final composited output
- Escape to close

### Video Recording
- Real-time GL frame capture
- Codecs: H.264, ProRes, MJPEG
- Triple-buffered pixel readback
- Status: frame count, duration, dropped frames

### Syphon Output (macOS)
- Shares GL texture to other applications (Resolume, MadMapper, etc.)

### Snapshot
- One-click PNG capture to configurable directory

### Session Recording
- Captures all parameter changes, clip triggers, transport changes as timestamped JSON
- Playback recreates the entire performance

---

## 17. Preferences

| Tab | Settings |
|-----|----------|
| General | Confirm on quit, Show tooltips |
| Audio | Sample rate, Buffer size, BPM detection range |
| Video | FPS target, Render resolution, MilkDrop preset directory |
| MIDI | (placeholder) |
| Recording | (placeholder) |
| Defaults | (placeholder) |
| Feedback | (placeholder) |
| About | Version, credits |

---

## 18. Layer Types

| Type | Behavior |
|------|----------|
| Opaque | One clip at a time, replaces everything below |
| Transparent | Composited over layers below with blend mode + keying |
| FX Only | No media — applies effects to the composited accumulator |
| 3D | 3D model rendering with rotation controls |
| Mask | Content used as luminance alpha mask for layers below |

---

## 19. Effect Chain Architecture

Effects can be applied at 3 independent levels:

| Level | Where | Applied When |
|-------|-------|-------------|
| Per-clip | Clip Inspector → Effects section | Before keying and layer blend |
| Per-layer | Layer Inspector → Layer Effects section | After clip compositing, before accumulator blend |
| Global | Composition Inspector → Global Effects section | After all layers composited |

---

## 20. Keyboard Shortcuts

| Shortcut | Action |
|----------|--------|
| Cmd+S | Save composition |
| Cmd+O | Open composition |
| Cmd+Z | Undo |
| Cmd+Shift+Z | Redo |
| Cmd+F | Toggle fullscreen output |
| Escape | Close output window / exit binding mode |
| Shift+Cmd+K | Enter keyboard binding mode |
| Shift+Cmd+M | Enter MIDI learn mode |
| Tab | (customizable via bindings) |

---

## 21. Data Hierarchy

```
Composition
├── name, output resolution, master opacity, master speed
├── global transition speed, BPM multiplier, quantize mode
├── genre automation settings (auto-preset, smart autopilot, structural scenes)
├── per-type autopilot config
├── composition transform (position, scale, rotation, anchor)
├── global effects chain
└── Deck[1..N]
    ├── name
    └── Layer[1..M]
        ├── name, type, visible, opacity, bypassed, solo, muted
        ├── blend mode (48 options), keying mode (13 options)
        ├── transition mode + duration
        ├── transform (position, scale, rotation, anchor)
        ├── feedback config (enable, preset, amount, scale, rotation, offset, luma key)
        ├── autopilot settings (action, duration, loops, trigger mode)
        ├── persistent flag, ignore column trigger flag
        ├── layer effects chain
        └── Clip[1..K] (one per column)
            ├── name, media type, media file path
            ├── source type + source parameters (procedural sources)
            ├── transport (mode, speed, reverse, in/out points, loop mode)
            ├── cuepoints[8]
            ├── autopilot (action, duration, beat snap)
            ├── video (opacity, width, height, blend override, alpha type, RGBA toggles)
            ├── transform (position, scale, rotation, anchor)
            ├── per-clip effects chain
            ├── content lock flag
            └── MilkDrop playlist (cycle mode, trigger, blend, entries)
```
