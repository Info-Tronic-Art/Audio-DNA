# Resolume Workflow Reference

Comprehensive reference extracted from the Resolume support documentation (https://resolume.com/support/en/). Covers the complete Workflow section: Quickstart, Vocabulary, Clips, Layers, Groups, Decks, Composition, Layouts, Autopilot, Shortcuts, Clip Time Panel, and Notes Panel.

---

## 1. Quickstart Tutorial

Source: https://resolume.com/support/en/quickstart-tutorial

### Core Concepts

- **Composition**: A complete Resolume setup including sets of clips, preprogrammed effects, and all other settings needed for a performance.
- **Clip**: Individual video/audio elements that can be triggered and played.
- **Layer**: Each horizontal row of clips is a separate layer. Each layer can play one clip at a time.

### Main Interface Layout

- Menu bar at top
- Horizontal rows (layers) with clip thumbnails below menu bar
- Output window on left side
- Control tabs on right side: Files, Compositions, Effects, Sources
- Left-side layer controls with sliders (V, A, M)
- Help window in bottom right (contextual hints based on pointer location)

### Triggering Clips

- Click a clip thumbnail to start playback
- Clips synchronize to BPM setting by default
- Playback waits for the next bar to start (not instant) -- this is Beat Snap
- Clicking the Clip tab reveals Transport controls: Forwards, Backwards, Pause icons
- A blue wedge/playhead can be dragged to "scratch" clips manually
- Manual manipulation desynchronizes from BPM (phase offset occurs)
- Re-clicking the thumbnail resynchronizes at next bar start

### Mixing Layers

- Clicking a clip on the **same layer** replaces the current clip at the next bar
- Clicking a clip on a **different layer** adds it (allows simultaneous playback)
- **A slider**: audio fade per layer
- **V slider**: video fade per layer
- **M slider** (master): controls both audio and video simultaneously

### Effects

- Drag effects from Effects tab to Composition tab area
- Drop target shows "four colored corners" around Composition tab
- Effects chain sequentially (each effect processes the output of the previous)
- Multiple effects can be stacked
- Each effect includes an Opacity slider for blending
- Most effects have additional parameter sliders (e.g., Bendoscope has a "divisions" parameter)
- Remove effects by clicking the 'x' next to the effect name

---

## 2. Vocabulary

Source: https://resolume.com/support/en/vocabulary

### Core Terminology

| Term | Definition |
|------|-----------|
| **Clip** | A container holding a video, image, audio file, or a source (e.g., camera input) with adjustable settings and parameters affecting playback and output quality. |
| **Layer** | A structural level where individual clips reside. Only one clip from each layer can play at a time. Layers can be blended together to create the final output. |
| **Group** | Layers can be combined into Groups. Groups are little sub-compositions, where multiple layers are composited together and can be controlled with a single fader. **(Arena only)** |
| **Composition** | A complete setup incorporating sets of clips, assigned effects, parameter settings and control shortcuts. Switching compositions requires significant time. |
| **Deck** | Organizational divisions of clips within a composition. Switching decks is quick and does not interrupt playback, so you can switch between decks while performing. |
| **Effect** | Modifiable elements applied at composition, layer, group, or clip levels that change how the output looks or sounds. |
| **Parameter** | Control elements for all adjustable Resolume features -- clip playback speed/direction, layer scaling/blend modes, effect adjustments, etc. |
| **Shortcut** | Assignable MIDI, OSC, and keyboard controls mapped to parameters, eliminating mouse dependency. |
| **Patch** | Wire-created plugins adding functionality like effects, sources, and blend modes to Arena and Avenue. |

---

## 3. Clips

Source: https://resolume.com/support/en/clips

### Clip Types and Loading

- Clips can be videos, still images, audio files, or combinations thereof
- Can also contain dynamic sources like live cameras or generative plugins
- Load clips via drag-and-drop from OS folders or the Files panel
- Files panel supports favorite folder tagging and thumbnail toggling

### Triggering Mechanisms

**Basic Triggering:**
- Click a clip's thumbnail to play it
- Click the X button to eject content

**Column Triggering:**
- Activate multiple clips simultaneously by triggering an entire vertical column
- Useful for coordinated visual transitions

**Selection vs. Activation:**
- Click the **name handle** beneath thumbnails to select without triggering
- Allows preset adjustments before playback

### Trigger Configuration Options

#### Beat Snap
- Clips can be quantized to sync with musical timing
- Options: next beat, bar, 2-bar, 4-bar intervals, etc.
- Settings apply per-clip or composition-wide via the Beat Snap menu

#### Clip Target Modes
| Mode | Behavior |
|------|----------|
| **Own Layer** | Default; clip plays in its assigned layer |
| **Active Layer** | Plays in whichever layer currently has focus |
| **Free Layer** | Occupies any available layer; pairs with Piano Trigger Style |
| **Composition Determined** | Inherits composition defaults |

#### Trigger Styles
| Style | Behavior |
|-------|----------|
| **Normal** | Trigger starts playback; re-trigger restarts |
| **Toggle** | Functions as on/off switch |
| **Piano** | Requires continuous input; releasing stops playback |

#### Fader Start
- When enabled, clips restart from beginning whenever their layer fades up
- Combined with pick-up playmode, becomes **Fader Stop** -- pausing and resuming from the same position

#### Ignore Column Trigger
- Locks clips or layers so they persist when other columns activate
- Ideal for backgrounds, persistent audio, or recording scenarios

### Visual and Layout Features

**Resize Options:**
| Mode | Behavior |
|------|----------|
| **Fill** | Crops to fill while maintaining aspect ratio |
| **Fit** | Black borders to maintain aspect ratio |
| **Stretch** | Distorts if needed to fill |
| **Original** | Native dimensions |

**Thumbnails:**
- Update from current frame, with or without applied effects
- Load custom stills from disk
- Revert to original thumbnail

**Clip Colors:**
- Assign pastel highlight colors for visual organization

### Clip Management

- **Cut/Copy/Paste/Rename/Clear**: Standard editing operations
- **Paste Effects**: Copies only applied effects to other clips without replacing content
- **Persistent Clips**: Mark clips to carry across decks; covered clips display as small thumbnails in the top-left corner, draggable to free slots
- **Show in Finder/Explorer**: Opens file system location
- **Show in File Browser**: Opens in Resolume's native browser
- **Strip audio or video tracks**: Warning -- removing both deletes the clip

### Auto Pilot (per-clip)

Actions when clip completes:
- Advance to next, previous, random, first, last, or specific clip
- Supports loop count before action triggers
- **Layer Determined**: Default mode where clips inherit layer-level Auto Pilot settings; individual clip settings override this

### Snapshot

- Captures current frame as PNG
- Saves to Recorded folder
- Imports to bottom layer

### Audio Properties

- Visible only for clips containing audio
- Volume balancing across layer clips
- Individual panning
- Audio track deletion via X button

### Video Properties

- **Scaling Options**: Cycles through Fill, Fit, Stretch, Original via button clicks
- **Content Replacement**: Drag video clips over the blue video area to swap content while preserving effects and parameters
- **RGB Channel Toggles**: Subtract color channels (primary and secondary colors)
- **Alpha Toggle**: Disables transparency when available
- **Clear video while preserving audio**

**Extended Properties:**
| Property | Detail |
|----------|--------|
| **Opacity** | Fade visibility to black |
| **Width & Height** | Pixel-precise resolution adjustment |
| **Blend Mode** | Override layer blend mode per-clip |
| **Alpha Type** | Premultiplied (After Effects default) vs. Straight; "Straight" resolves halo artifacts |

### Transition Controls

- Per-clip transition settings override layer defaults when enabled
- Configure blend mode and duration individually
- Defaults are "Layer Determined"

### Clip Properties Panel

- Displays technical metadata: codec, fps, length, default BPM
- Properties toggle to reveal audio and video fine-tuning sections

---

## 4. Layers

Source: https://resolume.com/support/en/layers

### Core Concept

Layers function like painting layers, built from bottom to top on a blank canvas. The first layer is the foundation, with subsequent layers composited on top at 60 frames per second.

### Layer Management

**Creation and Organization:**
- Add layers via Layer > New (top of stack), Layer > Insert (relative positioning), or Ctrl+L / Cmd+L
- Reorder by dragging layer name handles
- Fold/unfold layers using +/- symbols, right-click menu, or double-click
- Cascade fold with Shift+click
- Rename layers (supports # character for dynamic position display)
- Duplicate, clear content, or remove layers entirely

### Blending and Compositing

#### Blend Modes

| Category | Modes |
|----------|-------|
| **Brightening** | Add, Lighten, Screen |
| **Darkening** | Subtract, Darken, Multiply |
| **Special** | Difference I, Dodge |
| **Transition** | MultiTask (Cover Flow effect), Shift RGB (channel separation), to White |
| **50 Blends** | Allow 100% opacity mixing; ideal for AV mixing with faders fully open |
| **Random** | Random transition option available |
| **Custom** | Creatable via Wire mixer |

Default blend mode is **Add** (pixel values combined).

#### Alpha Blending
- Recommended approach using content with transparency channels
- Produces clean, crisp compositing without color mixing

#### Composition Strategy -- Background/Focus/Frame
1. **Layer 1 (Background)**: Texture/pattern loops
2. **Layer 2 (Focus)**: Recognizable subject (faces, logos)
3. **Layer 3 (Frame)**: Mask layer that creates depth, reveals/hides content

### Layer Controls (Left Strip)

Three primary buttons:
| Button | Function |
|--------|----------|
| **X** (Eject) | Removes currently playing clip |
| **B** (Bypass/Blind) | Temporarily hides layer from output |
| **S** (Solo) | Displays only selected layer, hiding all others |

**Warning:** Simultaneous Solo + Bypass = blank output

### Faders

| Fader | Function |
|-------|----------|
| **V** | Video fade (0-100%) |
| **A** | Audio fade (0-100%) |
| **M** | Master fade (controls both simultaneously) |

### Layer Properties Panel

#### Common Section
- Master fader controlling both audio and video simultaneously without affecting individual levels

#### Audio Section
- Volume control
- Pan control

#### Video Section
| Property | Detail |
|----------|--------|
| **Blend Mode** | Dropdown selection of blending algorithm |
| **Opacity** | Fade layer from 0-100% (same as V slider) |
| **Width & Height** | Custom render resolution (performance optimization possible) |
| **Auto Size** | Off (original), Fill (crop), Fit (black borders), Stretch (distort) |

### Layer Transitions

- Access: View > Show Layer Transition Controls
- Transition time: 0-10 seconds (adjustable via vertical slider or Layer panel)
- Default mode: Alpha (crossfade)
- Can select any available blend or transition
- Precision timing available down to milliseconds in Layer panel

### Navigation Controls (with transitions enabled)

- **Previous/Next buttons**: Trigger adjacent clips in layer
- Assignable to MIDI and keyboard shortcuts
- Can force Auto Pilot to advance prematurely

### Layer Auto Pilot

- Automatically sequences clips with configurable duration options
- Can target first clip, specific clip, or last clip
- Premature triggering via Next/Previous forces immediate advance

### Layer Trigger Options

| Option | Behavior |
|--------|----------|
| **Ignore Column Trigger** | Layer clips persist when column is triggered (useful for live inputs, backgrounds) |
| **Trigger First Clip on Play** | Automatically plays first clip when composition transport starts; repeats on transport restart |
| **Fader Start** | Retriggers current clip when opacity fader is raised |

### Layer Transport Controls

- Access: View > Show Layer Transport Controls
- Controls for the active clip: speed, direction, loop mode, playhead scrubbing
- BPM Sync mode displays beats instead of speed percentage
- Multi-layer simultaneous control with synchronized updates

### Mask Layer

| Mode | Behavior |
|------|----------|
| **All Below** | Applies mask to all underlying layers (equivalent to Mask50) |
| **One Below** | Applies mask to single layer beneath (equivalent to AE Luma Track Matte) |
| **Disabled** | Reverts to normal layer |

- Greyscale thumbnail display indicates active mask status
- Transparency checkerboard shows mask status
- Black/white content values determine visibility
- Alpha treated as black

### Lock Content

- Prevents accidental clip ejection or triggering of different content
- Allows parameter adjustments and layer controls to continue working

### Visual Indicators

- Currently selected layer highlighted in blue
- Greyscale thumbnails indicate mask mode
- Transparency checkerboard shows mask status
- Lock icon displays over locked layer thumbnails

---

## 5. Groups (Arena Only)

Source: https://resolume.com/support/en/groups

### Overview

Groups are available exclusively in **Arena**. They function as "little sub-compositions" allowing users to group layers together and treat them as one big, new layer.

### Creation and Layer Management

**Creating Groups:**
- Right-click a layer, select "Group > New"
- Grouped layers appear indented with a dedicated group panel

**Adding/Removing Layers:**
- Drag layers by their name handle into or out of groups
- A group cannot remain empty -- removing the final layer automatically creates a replacement empty layer to preserve effects and settings

**Menu Options:**
- **Group x**: Moves selected layer to another existing group
- **None**: Removes layer from its group
- **New**: Creates a new group with the selected layer

### Group Properties and Controls

Groups possess hybrid characteristics combining composition and layer functionality:
- Individual column triggers
- Master speed slider and playback controls
- Master bypass, solo, and eject buttons
- Support for transforms, effects, and masks at group level
- Ignore Column Trigger and Lock options applying to all contained layers

**Containment Principle:** "What happens in the group, never leaves that group." Prevents routing layers to external Layer Routers or crossfading outside the group's bus.

**Nesting Restriction:** Groups cannot contain other groups.

### Group Folding

- Fold/unfold using +/- symbols next to group names
- Right-click menu options
- Double-click the group

### Group Blending

- Default mode: Alpha blend at 100% opacity
- First layer renders on transparent black background (no blending with underlying layers)
- Subsequent layers blend only with group layers
- Final result flattens before blending with composition using group's blend mode and opacity

### Advanced Output Routing

- Groups function as slice inputs in Advanced Output
- Enables routing multiple layers to a single slice
- Note: Group blending mode is bypassed and composition effects are excluded when routing to slices

---

## 6. Decks

Source: https://resolume.com/support/en/decks

### Core Concept

Decks function as organizational containers for media within compositions, comparable to "records in a DJ's record bag." Each composition can contain multiple decks, accessed via buttons below clip layers. **Switching decks is quick and does not interrupt playback.**

### Adding Content

**Methods:**
- Drag-and-drop from OS file browser
- Built-in browser (Files tab on right side)

**Browser Features:**
- Navigate folders by double-clicking
- Search box for filtering long file lists
- Dropdown menu showing root drives and favorites
- Heart icon to add/remove favorites
- Toggle for thumbnail visibility
- Double-click clips to preview before loading

**Loading Media:**
- Drag files into deck slots
- Combine audio and video on same slot: "Resolume will automatically transpose the video to the length of the audio"

### Managing Clips within Decks

**Clip Manipulation:**
- Click and drag clip names to reorder
- Dragging over existing clips swaps positions
- Dragging beside clips inserts between them
- Ctrl/Cmd + drag creates copies
- Ctrl/Cmd + C/V/X supports copy/cut/paste
- Shift + Ctrl/Cmd selects multiple clips for batch operations

**Organization Tips:**
- Place similar clips on same layer (horizontal) for easy switching
- Arrange clips that work together in same column (vertical) for simultaneous triggering

### Using Decks

**Loading Individual Decks:**
- Double-click decks in Composition browser to add to current composition without loading entire composition

**Master Composition Strategy:**
- Maintain "master compositions" containing pre-organized footage by theme (resolution, content type, etc.)
- Add new material to existing decks or create new ones as needed
- During performance, double-click deck from master composition to load with all settings intact

**Available Operations:**
- Set clips to BPM Sync or Timeline mode
- Apply effects
- Arrange clips in columns

### Organizing Decks

**Creation:**
- Deck > New (adds to end)
- Deck > Insert Before/After
- Duplicate current deck

**Reordering:** Drag decks to change order within composition

**Deck States:**
| State | Behavior |
|-------|----------|
| **Closing** | Removes from view but retains contents |
| **Clearing** | Removes all clips but preserves empty deck |
| **Removing** | Deletes deck entirely |

**Naming and Customization:**
- Default names derive from source folder
- Use Rename option for clarity
- Apply color coding for visual marking

**Deck Menu:**
- Hamburger icon between scroll buttons
- Opens menu providing quick access to all open and closed decks

---

## 7. Composition

Source: https://resolume.com/support/en/composition

### Core Concept

A composition represents the complete performance, functioning as a "stage" where you arrange content, layers, and effects, then composite them together for output to screens or other applications via Spout/Syphon.

### Settings Panel (Composition > Settings)

| Setting | Detail |
|---------|--------|
| **Name** | Identifier for the composition; used as filename when saving |
| **Description** | Metadata field for notes about the composition |
| **Size** | Rendering resolution: common presets, connected monitor resolutions, or custom entries |
| **FrameRate** | Auto mode syncs to monitor refresh rate (recommended). Manual: 30fps recommended when resources limited. Match content framerate for optimal smoothness |
| **Bit Depth** | Toggle 8-bit vs 16-bit per channel rendering for enhanced color fidelity during complex composites; 16-bit requires more processing power |

### Top-Left Interface Controls

| Control | Function |
|---------|----------|
| **Composition button** | Preview composition even when faded down |
| **X button** | Eject all playing content simultaneously |
| **B button** | Bypass entire output |
| **M slider** | Master visibility control (displays red when below 100%) |
| **S slider** | Global speed for all playing clips (doesn't affect BPM-synced clips) |
| **Arrow icons** | Trigger previous/next columns |

### Properties Sections

| Section | Controls |
|---------|----------|
| **Common** | Master opacity and clip speed controls |
| **Audio** | Master volume and pan adjustment |
| **Video** | Independent video fade control from audio |

### Crossfader System

Allows assignment of layers/groups to "A" or "B" buses for rapid blending between layer combinations using a single slider.

**Configuration:**
| Option | Values |
|--------|--------|
| **Blend Mode** | Multiple composite blend options |
| **Behaviour** | Jump, Jump & Return, Cut, Fade (Legacy over 4 beats) |
| **Curve** | Adjustable mixer curve for crossfader responsiveness |

**Critical rule:** Bused layers must be positioned consecutively in the stack. Unassigned layers between assigned ones produce unexpected results.

### Direction Controls

- Global controls to switch all playing clips between forwards, pause, or backwards playback
- Access: View > Show Layer Transport Controls

### Trigger Options

- Composition-level trigger settings applicable to all clips
- Overridable by individual clip settings

---

## 8. Layouts

Source: https://resolume.com/support/en/layouts

### Color Customization

**Arena and Avenue:**
- Assign colors to clips, groups, columns, decks, and layers
- Right-click any element and select a color from the dropdown menu's bottom section

**Wire:**
- Node colors customizable via right-click context menu or the node panel
- Comment nodes display the selected color
- Can toggle fill visibility to create "hollow" comment boxes

### Panel Management

**Hiding Panels:**
- Right-click panel headers to hide them
- Hidden panels reappear through the View menu
- Arena and Avenue allow hiding sub-panels like Dashboard and Autopilot

**Moving Panels:**
- Drag-and-drop functionality enables custom layout creation by repositioning panels

### Multiple Screens Configuration

- Panels undock via right-click context menu selection
- Creates independent windows positionable on secondary monitors
- Multiple panels can undock simultaneously
- Restore panels by dragging them back into existing windows

### Multiple Monitors

Users duplicate existing monitors through right-click options. Each monitor's cogwheel settings control display source:

| Source Option | Description |
|---------------|-------------|
| **Composition** | Full composited output |
| **Preview** | Preview output |
| **Groups** | Group output |
| **Layers** | Layer output |
| **Crossfader Mix** | Crossfader mixed output |
| **Crossfader A or B** | Individual crossfader bus |
| **Selected clip, layer, or group** | Follows selection |
| **Advanced Output Screens** | Arena only |

- Opacity handling: traditional blocked backgrounds or fully black alternatives per monitor
- Crossfader options display composition cross-fades only; layer cross-fades within groups are invisible

### Layout Presets

- Save via View > Layout submenu
- Stored as XML files in Documents\Resolume Arena\Presets\Interface folder
- Transferable between users and machines

---

## 9. Autopilot

Source: https://resolume.com/support/en/autopilot

### Overview

Autopilot enables automated clip sequencing without manual intervention. Useful for lighting technicians, ad scheduling, or taking breaks during performances.

### Enabling

- View > Show Autopilot to display the Autopilot tab
- Available in Composition, Group, Layer, and Clip panels
- Navigate to Composition Panel and activate using the play forward button
- Trigger a column to begin playback

### Direction Settings

| Mode | Behavior |
|------|----------|
| **Forward** | Plays columns sequentially forward |
| **Backward** | Plays columns sequentially backward |
| **Random: Other** (default) | Jumps to different clips/columns without retriggering current |
| **Random: Any** | Can retrigger the current clip or column |
| **Random: Bag** | Cycles through all available clips/columns once before reshuffling and repeating |

### Duration Configuration

| Mode | Behavior |
|------|----------|
| **Seconds** | Custom time intervals trigger the next clip when elapsed |
| **Beats** | Advances after specified beat count, synchronized to global BPM |
| **Clip Transport** | Moves to next clip when current clip finishes (default for Layer panel) |
| **Longest Clip** | Uses duration of longest clip in column; shorter clips loop until longest completes |
| **Shortest Clip** | Uses duration of shortest clip in column |
| **Top Clip** | References uppermost clip with content, ignoring empty slots |
| **Bottom Clip** | References lowermost clip with content, ignoring empty slots |

### Looping Behavior

- **Groups/Composition**: Default loops after final column; disabling repeats the last column indefinitely
- **Layers**: Configurable loop count determines how many times clips repeat before advancing

### Clip Actions

Individual clips can override layer settings with custom actions:
- Trigger previous, next, random, first, last, or specific clip
- "Do Nothing" to bypass autopilot

### Column Actions

- Exclusive to Composition and Group Autopilots
- Decide what happens when the Autopilot advances past a column

### Master Layer

- Groups and Compositions can designate one layer as Master Layer
- The master layer drives all dependent Autopilots simultaneously for coordinated show control

### Priority System

Manual triggers override all automation.

**Hierarchy (highest to lowest):**
1. Manual input
2. Clip settings
3. Layer settings
4. Group settings
5. Composition settings

(Same-frame triggering only)

---

## 10. Shortcuts

Source: https://resolume.com/support/en/shortcuts

### General Shortcuts

| Action | Windows | Mac |
|--------|---------|-----|
| Undo | Ctrl+Z | Cmd+Z |
| Redo | Ctrl+Y | Cmd+Y |

### Composition Management

| Action | Windows | Mac |
|--------|---------|-----|
| New Composition | Ctrl+N | Cmd+N |
| New Deck | Ctrl+K | Cmd+K |
| New Layer | Ctrl+L | Cmd+L |
| New Group | Ctrl+G | Cmd+G |
| New Column | Ctrl+T | Cmd+T |
| Open File | Ctrl+O | Cmd+O |
| Save File | Ctrl+S | Cmd+S |
| Composition Settings | Ctrl+Shift+C | Cmd+Shift+C |

### Clip Editing

| Action | Control |
|--------|---------|
| Select multiple clips | Ctrl+left-click (Cmd+left-click on Mac) |
| Select range | Shift+click |
| Duplicate while dragging | Alt+left-click drag |

### Envelope Editor (Keyframe Control)

| Action | Control |
|--------|---------|
| Nudge keyframes | Arrow keys |
| Move keyframes | Shift+arrows |
| Select multiple keyframes | Shift+left-click |

### Advanced Output (Visual Manipulation)

| Action | Control |
|--------|---------|
| Constrain movement | Shift+left-click |
| Duplicate slices | Alt+left-click |
| Disable snapping | Ctrl+left-click |
| Constrain aspect ratio (scaling) | Shift |
| Center-point scaling | Alt |
| Constrain rotation to 45 degrees | Shift |
| Pan view | Spacebar+left-click |
| Nudge points/slices (1px) | Arrow keys |
| Nudge points/slices (10px) | Shift+arrows |
| Add/remove mask points | Double-click |

### Shortcut Configuration

| Action | Windows | Mac |
|--------|---------|-----|
| Keyboard shortcuts | Shift+Ctrl+K | Shift+Cmd+K |
| MIDI shortcuts | Shift+Ctrl+M | Shift+Cmd+M |
| OSC shortcuts | Shift+Ctrl+O | Shift+Cmd+O |
| DMX shortcuts | Shift+Ctrl+X | Shift+Cmd+X |
| Exit editing mode | Esc | Esc |

### Output Control

| Action | Windows | Mac |
|--------|---------|-----|
| Disable output | Ctrl+Shift+D | Cmd+Shift+D |
| Advanced output settings | Ctrl+Shift+A | Cmd+Shift+A |
| Enable test card | Ctrl+Shift+L | Cmd+Shift+L |
| Capture snapshot | Ctrl+Shift+P | Cmd+Shift+P |
| Fullscreen specific output | Ctrl+Shift+# | Cmd+Shift+# |

### Wire Shortcuts

#### Interface Navigation

| Action | Control |
|--------|---------|
| Panning | Spacebar+left-click or middle-click |
| Zooming | Scroll wheel or two-finger trackpad |
| Fit to view | Ctrl+0 (Cmd+0) |
| 100% zoom | Ctrl+1 (Cmd+1) |
| Edit text | Ctrl+E (Cmd+E) |

#### Node Operations

| Action | Windows | Mac |
|--------|---------|-----|
| Copy | Ctrl+C | Cmd+C |
| Cut | Ctrl+X | Cmd+X |
| Paste | Ctrl+V | Cmd+V |
| Duplicate | Ctrl+D | Cmd+D |
| Destroy | Backspace/Delete | Backspace/Delete |
| Duplicate and Connect | Shift+D | Shift+D |
| Rename | Ctrl+Shift+R | Cmd+Shift+R |
| Select All | Ctrl+A | Cmd+A |
| Select Unused | Ctrl+Shift+A | Cmd+Shift+A |
| Node Finder | Ctrl+Enter | Cmd+Enter |

#### Quick Node Insertion

| Node | Shortcut |
|------|----------|
| Join | Ctrl+J (Cmd+J) |
| Pack | Ctrl+P (Cmd+P) |
| Comment | Ctrl+M (Cmd+M) |
| Add | Shift++ |
| Multiply | Shift+* |
| Subtract | - |
| Divide | / |

#### Node Organization

| Action | Windows | Mac |
|--------|---------|-----|
| Auto Layout | Ctrl+L | Cmd+L |
| Distribution | Ctrl+Shift+} | Cmd+Shift+} |
| Stacking | Ctrl+Shift+{ | Cmd+Shift+{ |
| Alignment | Ctrl+Shift+arrows | Cmd+Shift+arrows |
| Toggle thumbnails | Ctrl+T | Cmd+T |
| Update thumbnails | Ctrl+Shift+T | Cmd+Shift+T |

---

## 11. Clip Time Panel

Source: https://resolume.com/support/en/clip-time-panel

### Overview

The Clip Time Panel displays system time and clip timing information. Useful for "tightly scripted shows where everybody needs to get their timing right."

### Access

- View > Show Clip Time
- Can be docked within the interface or undocked to a separate screen

### Display Features

- Current system time
- Clip duration
- Remaining time of playing clips

### Configuration

- Cogwheel icon provides settings to enable or disable specific clock displays

### Display Modes

| Mode | Behavior |
|------|----------|
| **Default** | Time runs forward matching clip playhead duration |
| **Remaining Time** | Click the time display to toggle; indicated by minus (-) prefix; shows how much time remains |

### Behavior

- Synchronizes with clip playback
- Functions as a large, visible clock for performance use

---

## 12. Notes Panel

Source: https://resolume.com/support/en/notes-panel

### Overview

The Notes Panel (v7.15+) is designed for recording information such as line-ups, time tables, and other notes.

### Access

- View > Show Notes

### Features and Controls

| Feature | Detail |
|---------|--------|
| **Text Color** | Selectable via control in the right top corner of the panel |
| **Font Size** | Double-T icon can be dragged to increase or decrease font size |

### Use Cases

- Can be combined with the Clip Time Panel and monitor configurations
- Creates monitoring setups for directors, managers, and supervisory personnel during productions

---

## Key Architectural Patterns Summary

### Hierarchy
```
Composition
  +-- Decks (organizational, non-interrupting switch)
  +-- Groups (Arena only, sub-compositions)
  |     +-- Layers
  |           +-- Clips (one active per layer)
  +-- Layers (ungrouped)
        +-- Clips (one active per layer)
```

### Compositing Order
- Bottom layer renders first (foundation)
- Each subsequent layer composites on top
- Groups flatten internally before blending with main composition
- Blend modes control how layers combine
- 60fps render rate

### One-Clip-Per-Layer Rule
- Only one clip from each layer can play at a time
- Triggering a new clip on the same layer replaces the current one
- Column triggers activate one clip per layer simultaneously

### Beat Synchronization
- Clips quantize to BPM by default
- Beat Snap determines when triggers actually fire (next beat, bar, 2-bar, etc.)
- Manual scrubbing desynchronizes; re-trigger resynchronizes

### Effects Apply at Multiple Levels
- Composition level (global)
- Group level
- Layer level
- Clip level
- Each effect has opacity and parameters
- Effects chain sequentially

### Crossfader
- Assigns layers/groups to A/B buses
- Single slider blends between buses
- Multiple behavior modes (Jump, Cut, Fade)
- Bused layers must be consecutive in the stack

### Autopilot Priority
Manual > Clip > Layer > Group > Composition

### Control Mapping
- Keyboard, MIDI, OSC, and DMX all mappable to any parameter
- Configured via dedicated shortcut editors
