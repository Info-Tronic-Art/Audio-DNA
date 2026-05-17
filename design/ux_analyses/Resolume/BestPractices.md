# Resolume Manual — Best Practices & System Features

Fetched 2026-03-19 from resolume.com/support/en/*

---

## 1. Preparing Media

**Source:** https://resolume.com/support/en/preparing-media

### System Setup for Performance
- Recommended to create a dedicated performance system: either a separate OS instance optimized for performance with minimal software, or a user account with reduced startup programs.
- The performance account should have administrator rights.

### Codec Recommendations
- **DXV Codec** is the primary recommendation. It is the fastest codec because Resolume decompresses video frames on the GPU rather than the CPU.
- Avoid installing codec packs (e.g., K-Lite). Only install codecs you actually need.

### Audio and Video Separation
Resolume strongly recommends separating audio and video into different files, then combining them within Avenue. Benefits:
- Flexibility to modify audio without re-rendering video.
- Better workflow across different software/creators.
- Eliminates BPM quantization issues that occur with embedded audio.

**Quantization example:** Using PAL video (25 fps), a one-bar loop at 90 BPM cannot be created perfectly. The closest achievable is approximately 90.9 BPM in a 66-frame clip. Avenue adjusts video length to match audio for perfect synchronization.

### Audio Compression Guidelines
**"Don't compress audio. Period. Don't do it."**

- Format: Uncompressed linear PCM `.wav` files
- Sample rate: 44,100 Hz
- Bit depth: 16-bit
- Rationale: Uncompressed audio provides faster data access and file sizes remain small relative to video.

---

## 2. Media Manager

**Source:** https://resolume.com/support/en/media-manager

### Core Purpose
The Media Manager helps relocate files when they have been moved on the computer, preventing compositions from losing track of their media assets. Compositions store file locations (paths) rather than embedding the files themselves.

### Visual Indicators
- Missing files appear marked in **red** with a "Relocate" button in the Path column.

### Clip Reconnect Workflow
1. Click the "Relocate" button to open Media Manager.
2. The system builds a complete list of all files in the composition.
3. Missing files are highlighted in red.
4. Navigate to the new file location via the file browser.

**Important:** The Media Manager always saves the composition before it can operate. It prompts before saving automatically.

### Intelligent File Discovery ("Sibling Files")
When you relocate one file, Media Manager automatically searches for other missing files by looking at:
- Files in the same folder
- Files in parallel subfolders
- Files in matching parent folder structures

**Example:** Moving `Old/Sub/file1.mov` to `New/Sub/file1.mov` will cause Media Manager to find all relocated files that maintain the same relative structure.

After discovery, options are: "Fix All", fix only the initial file, or cancel.

### Replace File Function
- Accessible via the File menu even when no files are missing.
- Right-click any file and select "Set Path" to swap a file with an updated version without the full relocate workflow.

### Collect Media Feature
Gathers all composition files into one organized location for portable delivery or archiving.

**Process:**
- Select "Collect Media" from the bottom left.
- Displays file count and total storage space required.
- User selects a destination folder.
- System creates a folder structure with a "Media" parent directory.

**Organization:**
- Creates subfolders for each deck.
- Non-deck files (e.g., masks on composition) go into separate folders.
- Same file used twice in one deck: copies only once.
- Same file across different decks: copies multiple times (one per deck).
- Creates a new composition copy with updated file paths alongside the Media folder.

**Critical:** "The Media Manager always copies! Resolume will never try to delete files."

### Limitations
- Collect Media handles only content files. For comprehensive backup, manually copy the entire Resolume folder from Documents (contains all user settings, preferences, and presets).
- Changes apply when closing Media Manager but do not auto-save the composition. You must save manually after Media Manager closes.
- macOS system dialogs do not display filenames in the browser window.

---

## 3. Dashboard

**Source:** https://resolume.com/support/en/dashboard

### Overview
The Dashboard provides quick access to frequently-used parameters across Clip, Layer, Group, and Composition panels. It functions like a car dashboard -- displaying essential controls without requiring deep navigation through effect stacks.

### Adding Parameters
- Drag parameters directly onto dashboard dials.
- Alternatively, select "Dashboard" from the animation dropdown menu of any parameter.
- Supports all parameter types: numeric values, toggles, dropdowns, and color palettes.
- Once added, the dial automatically renames itself to match the parameter name. The name is customizable via double-click.

### Multi-Parameter Assignment
A single dial can control multiple parameters simultaneously, useful for creating cohesive visual effects or complex looks.

### Configuration Options

**Parameter In/Out Points:**
These boundaries map the full dial range to only a portion of a parameter's range. Particularly valuable for sensitive controls requiring fine adjustment.

**Invert Function:**
Reverses parameter direction -- moving the dial downward increases the parameter value and vice versa.

**Dial Range:**
Sets which portion of the dial controls each parameter. For example, limiting the dial range to 33% allows the first third of the dial to access the complete parameter range, leaving the remaining space for additional parameters on the same dial.

**Removal:**
Right-click a parameter to remove it from the dial.

### Advanced Techniques
The documentation highlights using envelopes, effect bypasses, and inverted settings to create "mutex dials" that cycle through different effect configurations.

---

## 4. Envelopes

**Source:** https://resolume.com/support/en/envelopes

### Overview
Envelopes allow precise parameter flow control, enabling easing curves, bounce behavior, and multi-keyframe animations instead of linear progression.

### Enabling
Right-click any parameter and select "Envelope" from the dropdown menu. The default setting applies linear interpolation (no visible change initially).

### Easing Functions
Right-click the second keyframe to access easing options:

**Basic Easing:**
- Quadratic
- Sine
- Circular
- Exponential (varying strength levels)

**Advanced Animation:**
- Elastic
- Back
- Bounce (for complex animation principles)

**Special:**
- Hold (maintains previous keyframe value until the current one is reached)

**Directional Modifiers:**
- In (curve at interpolation start)
- Out (curve at end)
- In/Out (both sides equally)

### Interpolation Behavior
The visual display shows a large vertical line representing input and a smaller line showing actual output after envelope application. Envelopes apply to **all** parameter inputs: manual control, Timeline/BPM Sync animation, FFT, MIDI, and OSC data.

### Keyframe Management

| Action | Method |
|--------|--------|
| Add keyframe | Double-click the curve at desired location |
| Remove keyframe | Double-click an existing keyframe |
| Adjust keyframe | Click and drag |
| Precise nudging | Arrow keys (Shift for larger increments) |
| Multi-select | Shift+click |
| Manual entry | Input phase (x-axis position) and output (y-axis value) |

**Phase values** express position independent of time, accommodating beat-based or FFT animation modes.

### Special Parameter Types
Toggles, dropdowns, and palettes display grey indicators showing state transitions. Keyframe adjustments snap to defined positions automatically.

### Envelope Presets
Access preset management via the "P" button at the envelope's bottom-right corner:
- Save custom presets
- Browse pre-made options
- Delete/rename via the "Manage..." option

---

## 5. BPM

**Source:** https://resolume.com/support/en/bpm

### Core Concepts
- "A BPM of 120 means you have a beat every 0.5 second."
- Genre ranges: hip-hop (~100 BPM), EDM (~130), trance (~140), harder styles (160+).
- Rhythmic structure: 4 beats = 1 bar, 8 bars = 1 phrase.

### Setting BPM

**Manual Input:** Click the BPM display field and enter a known BPM value directly.

**Tap Tempo:** Click the "Tap" button rhythmically with the music across multiple beats. The software calculates the tempo automatically. This is the primary recommended approach.

**Resync:** "When you press 'Resync', everything in Resolume that is set to BPM Sync will jump back to the first beat of the first bar."

### Visual Feedback
- A blue square moving clockwise around a slightly bigger blue square acts as the beat indicator.
- Correct synchronization: the marker hits the top-left corner on phrase boundaries.
- The display brightens every 16 beats for verification.

### Fine-Tuning Controls
- **Plus/Minus buttons:** Incrementally shift tempo.
- **Nudge Up/Nudge Down buttons:** Temporarily adjust speed while held, then revert when released.
- **Re-tap:** Tap out new tempos for significant corrections.

### Content Synchronization Requirements
- Video events must occur in multiples of 2 (4, 8, 16, or 32 steps/events) for automatic beat detection when BPM Sync is enabled.
- Videos with non-standard event counts (e.g., 12-step animation) require manual adjustment or trimming.

### Additional Features
- Headphone metronome output available for video-only VJs to beat-match without audio sync.
- Advanced synchronization via MIDI clock and Ableton Link.

---

## 6. Edge Blending

**Source:** https://resolume.com/support/en/edge-blending

### Overview
Edge blending is exclusive to **Resolume Arena**. It seamlessly stitches together outputs from multiple overlapping projectors by gradually fading out the overlap area.

### Setup Process

**Step 1 -- Overlap the Input:**
- Slices must partially cover the same composition area, mirroring physical projector overlap.
- **Recommended minimum overlap: 15%.**
- Use the test card (`Output > Show Test Card`) to align slices.
- The test card includes diagonal lines to help identify correct grid positioning.

**Step 2 -- Align the Output:**
- Perspective warp each output using four corner points (not linear warping options).
- Project the test card and adjust corner points until grid alignment is perfect.

**Step 3 -- Enable Edge Blending:**
- Turn on edge blending for each slice individually.
- The system automatically blends edges in the middle.

### Blending Parameters

| Parameter | Description |
|-----------|-------------|
| **Gamma Red** | Controls brightness of the red channel in the overlap area |
| **Gamma Green** | Controls brightness of the green channel in the overlap area |
| **Gamma Blue** | Controls brightness of the blue channel in the overlap area |
| **Power** | Adjusts slope of the edge blend curve; higher values create steeper curves at the fade center |
| **Luminance** | Controls brightness at the fade center point, allows further slope adjustment |
| **Gamma** | Overall brightness of the fade area |

### Black Level and Brightness Compensation
Projectors cannot project true black (only deep grey), so overlapping areas appear lighter than desired. Black level compensation makes the non-overlapping areas slightly brighter to compensate. This setting is located on the output tab.

---

## 7. Slice Routing

**Source:** https://resolume.com/support/en/slice-routing

### Core Functionality
Slice routing enables routing various composition elements directly to individual slices, allowing complex multi-display setups with efficient resource management. Available in both Avenue and Arena.

### Routing Options

**Layer to Slice:**
Individual layers can be assigned as direct slice inputs via the "Input Source" dropdown. This bypasses the full composition, allowing separate warping and transformation per layer. Very useful for displaying different content across multiple HD displays without rendering a massive composition.

**Group Routing (Arena only):**
Groups function similarly to layers with identical opacity and bypass/solo options.

**Preview Routing:**
The preview output can be routed to external displays independently.

**Screen-to-Slice Routing:**
Physical, virtual, NDI, or Spout/Syphon screen outputs can feed into slices, creating multi-phase warping processes for edge blending workflows.

### Key Controls

**Ignore Opacity and Bypass/Solo Toggle:**
Users can disable the layer's opacity settings and bypass/solo behavior independently. This enables content visibility in specific slices while hiding it in the main composition.

### Critical Limitations
- All layer and clip effects are still applied, but **blend modes and composition effects are NOT applied** to routed slices.
- Sliced content renders with straight alpha blending only.
- The manual strongly discourages using layer-to-slice routing for positioning content; use the "Slice Transform" effect instead.

---

## 8. Undo/Redo

**Source:** https://resolume.com/support/en/undo

### Keyboard Shortcuts
- **Undo:** Ctrl/Cmd+Z
- **Redo:** Ctrl/Cmd+Shift+Z
- Toolbar buttons also display messages indicating what action was undone.

### Undoable Actions
All mouse-based interactions are captured:
- Slider movements
- Button clicks
- Dropdown menu selections
- Drag operations (clips, effects, layers, decks)
- Composition state changes

### Non-Undoable Actions
Certain live application behaviors cannot be undone:
- **External input changes** via MIDI, OSC, or other external sources.
- **Clip triggering** -- once triggered to screen, it cannot be reversed.
- **Automated actions** including parameter automation and autopilot triggers.

### Scope Behavior
When undoing, Resolume automatically navigates the interface to the affected area:
- Selects relevant clips and displays them in the clip panel.
- Switches decks if the action occurred in a different deck.
- Ensures visibility of changes to prevent accidental blind undos.

### Stack Management
Undo/Redo operates with **separate histories** for different interface sections:
- **Advanced Output:** Maintains an independent undo stack when open.
- **Main Interface:** Maintains a separate undo stack.
- Switching between sections preserves both histories.
- This isolation prevents unintended undos of changes in unviewed areas.

---

## 9. Ableton Link

**Source:** https://resolume.com/support/en/link

### Overview
Ableton Link is an open-source protocol that helps musicians keep in time. It synchronizes tempo across devices and software on the same network.

### How It Works
- Operates "like an open jam session" -- connected devices on the same network automatically join Link sessions and maintain synchronized timing.
- Once enabled, Resolume continuously syncs BPM and measure position with other connected software.
- Changes made in one application propagate to all others. Adjusting tempo in Ableton causes Resolume to follow, and vice versa.

### Enabling Link
- Access via: `View > Show Ableton Link`
- This reveals a toolbar button to activate the feature.
- The interface displays the number of currently connected devices in the session.

### Behavioral Constraints When Active
- **Resync and Pause buttons for BPM are disabled** while Link is active.
- Hard resets and complete pauses are "considered bad musical practice" during synchronized sessions.

### Equality Principle
All participants have equal control. Any user can adjust BPM with the same immediacy as others, affecting the entire session simultaneously.

### Related Topics
BPM, MIDI Shortcuts, and SMPTE are cross-referenced as related features.

---

## Key Themes

- **Performance optimization**: GPU-decoded codecs (DXV), uncompressed audio, dedicated performance systems.
- **Separation of concerns**: Audio and video as separate files; slice routing isolates layers from composition effects.
- **BPM sync architecture**: Tap tempo, visual beat indicator (blue square), resync to downbeat, content must use power-of-2 event counts.
- **Envelope system**: Applies easing/curves to any parameter input (manual, MIDI, FFT, OSC), with phase-based keyframes independent of time.
- **Dashboard**: Macro-style multi-parameter dials with range mapping, inversion, and in/out points.
- **Undo**: Separate stacks for main interface vs. Advanced Output; external/automated actions are not undoable; navigation follows undo context.
- **Link protocol**: Network-based tempo sync with equal control for all participants; disables manual resync.
