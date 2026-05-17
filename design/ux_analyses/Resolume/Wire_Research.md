# Resolume Wire — Complete Research Notes

Sourced from all 13 pages of the Resolume Wire documentation (resolume.com/support/en/wire-*).

---

## 1. Introduction (wire-introduction)

Wire is Resolume's node-based visual programming environment for creating custom effects, sources, and mixers that integrate with Arena and Avenue. It is described as "a programming language for visual design" where users connect nodes (small blocks of code) through visual connections, analogous to a telephone switchboard.

**Key points:**
- Compatible with Arena and Avenue from version 7.4 onwards.
- Also runs as a standalone application for installations and multimedia art.
- Users create custom effects, sources, and blend modes.
- Patches generate interactive UI elements: sliders, color inputs, triggers.
- Supports OSC and MIDI protocols.
- Integrates with Resolume's Slices and FFT signals.
- Data modulation across virtually all parameters.
- Wire is the "workshop" for creating tools; Arena/Avenue is the performance instrument.
- Patches designed in Wire appear as integrated UI elements in Arena/Avenue during live performance.
- Example use case: an enhanced Colorize effect that generates analogous color palettes while preserving black and white values.

---

## 2. Installing Wire (wire-installing)

- Download installer from Resolume website; site auto-detects OS (Mac/PC).
- Manual OS switch available in top-right corner.
- All historical Wire versions accessible via dropdown at page bottom.
- Wire requires specific hardware due to graphical acceleration (GPU-dependent). Users should check the specs page before installing.

---

## 3. User Interface (wire-user-interface)

### Welcome Screen
Three primary tabs:
- **Patches tab**: Recently used patches; toggle chronological/alphabetical sorting.
- **Tutorial tab**: Growing list of tutorials, foundational through advanced.
- **Examples tab**: Documented example patches demonstrating Wire capabilities.
- **Templates tab**: Preconfigured setups for Sources, Mixers, and Effects.
- Welcome popup visibility is toggleable via bottom-left controls.

### Canvas Navigation
- **Pan**: Spacebar + click-drag, or middle mouse button hold.
- **Zoom**: Middle mouse scroll, or Ctrl/Cmd + +/-.
- **Zoom menu** (top-right): Preset increments, fit-to-screen (Ctrl/Cmd+0), 100% reset (Ctrl/Cmd+1).

### Node Creation and Selection
- **Create node**: Double-click empty canvas space to open searchable node browser with descriptions and examples.
- **Library tab**: Drag-and-drop node addition.
- **Select**: Single-click, Ctrl/Cmd-click for multi-select, click-drag for selection box.
- **Select all**: Ctrl/Cmd+A.
- **Select unused**: Ctrl/Cmd+Shift+A — selects all nodes that have nothing connected to them (useful for cleanup).
- **Navigate**: Arrow keys to move between adjacent selected nodes.

### Movement and Organization
- **Move nodes**: Click-drag selected nodes.
- **Align**: Right-click provides left/top alignment for multiple nodes.
- **Auto-layout**: Ctrl/Cmd+L sorts selected nodes "as neat as possible."
- **Duplicate**: Right-click > Duplicate, Ctrl/Cmd+D, or Alt-drag.

### Copy/Paste
- Standard Ctrl/Cmd+C/X/V.
- **Copy For Sharing**: Exports patches as shareable text (clipboard).
- **Copy/Paste Settings**: Duplicates node configurations between different node types.

### Renaming
- Double-click node names to rename, or Ctrl/Cmd+Shift+R.
- Input node names display in Dashboard and Resolume integration.
- Multi-edit icons support batch renaming.

### Connections
- Click outlet then inlet to connect (left-to-right convention).
- To relocate a connection, click the inlet wire.
- Disconnect via right-click or Del/Backspace.

### Node Deletion and Undo
- Wire uses "destroy" terminology (right-click or Del/Backspace).
- Undo: Ctrl/Cmd+Z. Redo: Ctrl/Cmd+Shift+Z or Ctrl/Cmd+Y.

### Audio Control
- When Spectrum In nodes exist, a volume slider appears alongside zoom indicators.
- Warning triangle signals missing audio device; configure via Wire > Settings > Audio.

### Monitor Panel (bottom-right)
- Previews selected node output.
- Pin button locks node to preview regardless of navigation.
- Right-click: hide, duplicate, or undock monitor.
- Right-click image: copy frame.
- Cogwheel: snapshots (convert frame to image node), background color (transparent/black).

### Patch Panel
- Displays patch metadata: name, description, category (source/effect/mixer).
- Adjust resolution and texture bit depth.
- Credential fields for attribution.
- License information covered separately.

### Node Panel
- Appears after node selection.
- Functions: rename, manual inlet value adjustment, color customization (applied to all selected nodes), attribute configuration for data I/O types and instance counts.
- Example link at panel bottom.

### Library Panel
- Searchable node collection.
- Double-click or drag to add nodes.

### Resources Tab
- Catalogs external patch dependencies: clips, images, ISF files.
- "Consolidation" copies resources to patch directory for sharing/archival.

### Notes Panel
- Quick note-taking, patch storage via "copy for sharing," to-do lists.

### Log Panel
- Displays error messages.
- The Print node sends custom info to the Log.

### Dashboard
- Manages all input nodes and controls their order (matches Arena/Avenue parameter pickup sequence).
- Drag-and-drop reordering, cogwheel sorting options.
- **Input groups**: Create foldable sections in Arena/Avenue.
- Clicking nodes centers canvas on off-screen parameters.
- P-icon launches preset management.

### Presets
- Preset names cannot use "Default" (reserved).
- Preset manager shows Arena/Avenue-created presets as read-only.

### Stats Panel
- Per-node system load information for debugging.
- Sortable columns: NAME, CATEGORY, LOAD, VERSION.
- Click node names to select and center view.

### Node Search
- Ctrl+F / Cmd+F activates search.
- Highlights matching nodes and shows them in Node panel.

---

## 4. Node Anatomy (wire-node-anatomy)

### Four Node Types

1. **Input Nodes** — Take data from outside into Wire. Includes host info like FFT, parametric data, slices. Examples: Texture Input, Trigger Input.
2. **Output Nodes** — Send patch data externally. Example: Texture Output sends images to host.
3. **General Nodes** — "Where the magic happens." Handle data manipulation, textures, arrays, shapes, modulation, displacement, math operations.
4. **Comment Nodes** — No inlets/outlets. For documentation. Toggle "fill" to make hollow (organize patches into sections). Adjustable text size and alignment.

### Inlets and Outlets
- Inlets on node left side; outlets on right side (represented as dots).
- Hovering reveals possible data types and previews flowing values.
- Manual value adjustment via click, drag, or panel entry.
- Click inlet/outlet to start connection.
- Dragging cords onto existing connections replaces them.
- Alt+drag adds connections in event flow.

### Expose Inputs
- Right-click option creates interactive UI elements:
  - Value boxes for numerical inputs
  - Booleans for toggles
  - Trigger nodes for triggers

### Visibility Control
- Right-click to show/hide specific inlets/outlets.
- Ctrl+Shift+H: hide all.
- Ctrl+Shift+Alt+H: show all.

### Thumbnails
- Visual representations on nodes.
- Ctrl+T: toggle thumbnail.
- Ctrl+Shift+T: update thumbnail.

### Three Flow Types

1. **Signal Flow** — Transmitted at frame rate. Most common. Denoted by **circular** inlets/outlets.
2. **Event Flow** — Transmitted when user triggers or value changes. Independent from frame rate. Denoted by **rectangular** inlets/outlets.
3. **Attribute Flow** — Only transmitted while patch compiles. Prevents runtime animation. Denoted by **diamond-shaped** inlets/outlets.

**Conversion rules:**
- Event and Attribute flow can connect to Signal inputs.
- The **OnChange** node converts Signal to Event flow.

---

## 5. Resolume Integration (wire-resolume-integration)

### Core Requirements
- Patches must be saved AND assigned a category (Source, Effect, or Mixer) to appear in Arena/Avenue.
- **Texture bit depth**: 8-32 bits. 8-bit patches upgrade to 16-bit in 16-bit compositions; 16+ bit patches maintain depth.
- Patch resolution follows incoming texture size when nodes are configured accordingly.

### GUI Creation — Interactive Elements

Input nodes generate interactive controls in Arena/Avenue. Order matches vertical order in Dashboard. Rename via double-click or Dashboard editing.

**Slider Controls:**
- Float or Integer input nodes create sliders.
- Configurable min/max in node panel.
- Optimization tips:
  - Use Map nodes to remap ranges without adjusting slider endpoints.
  - Apply Negate or Map nodes for negative sweetspot values.
  - Use 1-X nodes to reverse 0-1 inputs to 1-0 ranges.
- Units/suffixes display in sliders (pixels, percentages, degrees, etc.) via node panel.

**Toggle Controls:**
- Bool In nodes create toggles with saved state persistence.
- Combining Trigger In + Toggle node creates trigger-based toggles.

**Menu / Selector Controls:**
- Int In nodes create buttons or dropdown menus.
- "Options counts" attribute defines selectable items (words, letters, numbers).
- Output integers correspond to selected options (first = 0).
- Switch between signal/event/attribute flow.
- Choose between button or dropdown presentation.

**Color Selection:**
- Color input nodes display standard color picker with RGB, HSB, and palette.

**Triggers:**
- Trigger input nodes require no additional configuration.

### FFT Integration
- Spectrum input nodes access Resolume's FFT data.
- Related nodes: Falloff, Frequency to Pitch, Frequency to BPM.

### MIDI and OSC
- Patches loaded in the host use the host's MIDI/OSC settings and infrastructure.

### Disconnection Rule
- Arena/Avenue ignore attribute flow changes that would break connections (e.g., changing Gradient node types), unlike Wire which permits such modifications.

---

## 6. Saving and Consolidating (wire-saving-consolidating)

### Save Location
- Patches stored in `~/(My) Documents/Resolume Wire/Patches/`.
- Saved patches listed on Wire welcome window for quick access.

### Consolidation
- Packages patch + all associated resources into a single folder.
- Automatically redirects resource nodes to consolidated files.

### Copy for Sharing
- Select nodes > right-click > "Copy for Sharing."
- Converts patch to clipboard text (a "big mess of text").
- Resources (images, shaders, video) are **excluded**.

### Video Export
- Patch exporter via patch dropdown menu.
- Converts patches to video format.
- Requires a Texture Out node as source.

### Compilation

Two compiled formats:
- **.wired** (editable): Users can modify patches and access resources.
- **.cwired** (locked): Resources and editing restricted.

**Compilation metadata required:**
- Patch name
- Credits and contact information
- License type (Creative Commons, AGPL, proprietary, etc.)
- Dashboard presets

**Juicebar Integration:**
- Toggle "Compile for Juicebar" for marketplace distribution.
- Compiled patches lack Wire watermark in Avenue/Arena.

### Command Line Compilation

```
wire[.exe] compile <patch file> [options]
  -o, --output     Output filename
  -e, --editable   Enable user editing
  -j, --juicebar   Activate Juicebar licensing
```

**Executable paths:**
- macOS: `/Applications/Resolume Wire/Wire.app/Contents/MacOS/Wire`
- Windows: `C:\Program Files\Resolume Wire\Wire.exe`

---

## 7. Fast Patching (fast-patching)

### Core Techniques

**Drag Duplicating:**
- Alt-drag selected nodes to duplicate with all parameters and attributes preserved.

**Create From Inlet/Outlet:**
- Click inlet/outlet to start cord, then click canvas to open filtered node search (only compatible nodes shown).

**Chaining Nodes:**
- Cmd/Ctrl+Enter creates and connects multiple nodes sequentially.
- Example: "chain Texture In into Transform into Colorize into Bright.Contrast into Texture Out."

**Auto-Layout:**
- Cmd/Ctrl+L organizes selected nodes left-to-right. Feedback loops may behave unpredictably.

### Node Management Shortcuts

| Shortcut | Action |
|----------|--------|
| Double-click connection | Insert node mid-connection (filtered search) |
| Cmd/Ctrl+J | Insert Join (combine multiple matching-type outputs) |
| Cmd/Ctrl+P | Insert Pack (merge 2-4 Float outputs into Float2/3/4) |
| Right-click inlet > Expose as Input | Create input node with correct naming/range |
| Cmd/Ctrl+M | Wrap in Comment (hollow comment around selected nodes) |
| Shift+Cmd/Ctrl+A | Select Unused Nodes (non-contributing, excluding comments) |

### Input and Value Editing

**Math in input boxes:**
- Support addition, subtraction, multiplication, division in node inputs, node panel, and dashboard.

**Multi-component editing:**
- Shift-drag on Float2/Float3/Float4 values adjusts all components simultaneously.
- Shift+Enter applies single value to all components.

---

## 8. Slices (wire-slices)

### Overview
Wire can access slices from Resolume Arena (v7.4+) for projection mapping and composition work. Capabilities include creating outlines, chasers, and converting slices to shapes.

### Slice In Node
- Foundation for importing slices.
- Default: displays 5 test dummy "Stage Droid" slices with selectable presets (grids, masked versions).
- Not bound to instance count of dummies — manipulate freely with Wire's node suite.

### Workflow
- When opening a source/effect/mixer with a Slice In node, Arena auto-generates a "slices tab" in that effect.
- Users select which slices to use via drag-and-drop.
- Slices tab may be hidden by default; enable via View > Show Slices.
- Requires prior slice creation in Arena's Advanced Output.
- Processed slices exported as textures via Texture Out node.

---

## 9. MIDI (wire-midi)

### Setup
- Connect MIDI devices, install drivers.
- Configure in Wire Preferences > MIDI section.
- Toggle output box on for device functionality.
- **MIDI Monitor**: Displays all incoming messages from enabled controllers (for troubleshooting).

### Reading MIDI

| Node | Purpose |
|------|---------|
| **MIDI In** | Receives all MIDI data across all channels (hub node) |
| **MIDI Read** | Outputs pitch, velocity, CC value, controller, and channel |
| **MIDI Filter** | Targets specific notes by pitch and channel |
| **Pitch** (filter) | Extracts pitch data |
| **CC Value** (filter) | Extracts CC values |
| **Pitch Bend** (filter) | Extracts pitch bend data |

**Logic nodes:** Within, Equal, Outside — for range checking on MIDI Read output.
**Boolean nodes:** "Is CC", "Is Pitch Bend", "Is On/Off" — boolean outputs from MIDI signals.

### Writing MIDI
- **MIDI Out** node generates outgoing MIDI data.
- Supports MIDI On, MIDI Off, CC value messages.
- Enables creation of MIDI delays, chord generators, sequencers.

### MIDI Channels
- Advanced MIDI Channels node enables polyphonic operation by writing to multiple channels simultaneously.

---

## 10. OSC (wire-osc)

### Reading OSC
- **OSC In** node connected to **Read OSC** node.
- Specify address; system processes incoming data.

### Writing OSC
- **Write OSC** node + **OSC Out** node exports data from patches.
- Example: "send the luminescence of your output to Ableton where it will modulate a filter."

### Configuration
- Number of parameters adjustable through inspector panel.
- Common use: custom TouchOSC apps on iPad/phone to control patches.

### Preferences (Wire > Settings, Ctrl+comma)
- **Monitoring**: Display incoming OSC in real-time.
- **Input Enablement**: Must activate OSC Input before use.
- **Network**: Same network and port as sending device required.
- **Use Bundles**: Packs all messages into one bundle. Performance varies by app compatibility.

---

## 11. FFT (wire-fft)

### Purpose
FFT transmits frequency data from Arena/Avenue to Wire for audio-responsive visuals. "Get the amplitude (loudness) of a set frequency band or range."

### Technical Details
- Wire receives FFT as an **array of 1024 float values**.
- Each value = distinct frequency band.
- Lower indices = lower frequencies.
- **Maximum frequency** depends on sample rate (configurable in preferences).
- **Band width** = (sample rate / 2) / 1024.
- At 48kHz: ~23Hz per band.
- Human hearing range: 15-17kHz (index 675+ at 48kHz).

### Spectrum In Node
- Creates audio input within Wire patches.
- Provides the 1024 float values for frequency-based manipulation.

### Resolume Integration
- Patches with Spectrum In nodes get a "spectrum in" dropdown tab with three source options:
  - **Local**: Audio from current clip.
  - **Composition**: All audio in the composition.
  - **External**: FFT source configured in preferences.

### Related Nodes
- Falloff
- Frequency to Pitch
- Frequency to BPM

---

## 12. Syphon and Spout (wire-syphon-and-spout)

### Purpose
Route visual output between applications on the same computer. Syphon (Mac) / Spout (PC).

### Use Cases
- Integrate external software (Processing, Max/MSP) with Wire's effects and surface mapping.

### Critical Requirement
- Both applications must run in "high-performance mode" (especially laptops) since texture sharing depends on GPU resources.

### Getting Syphon/Spout Into Wire
1. Configure source app to output Syphon or Spout.
2. Create Texture In node in Wire.
3. Select Syphon or Spout as texture input in node panel.

### Sending Syphon/Spout Out of Wire
1. Create Texture Out node.
2. Select Syphon or Spout in node panel.
3. Configure receiving app per its instructions.

---

## 13. ISF — Interactive Shader Format (isf)

### What is ISF
A programming language for creating custom shaders — "little programs that create visuals or effects by running instructions directly on your GPU."

### Core Nodes

| Node | Purpose |
|------|---------|
| **ISF Node** | Primary shader execution. Non-functional until shader assigned via Fragment Shader attribute in node panel. |
| **ISF Resource Node** | Holds ISF resources routable to multiple ISF nodes. Enables rapid shader switching via Read node. |

### Creating Shaders
Three methods:
1. Create new shader.
2. Create from clipboard content.
3. Browse device files.

Text editor opens automatically; also accessible via View menu or Ctrl+T.

### Shader Input Parameters (JSON-defined in code header)
Supported types:
- **Float** (with optional MIN/MAX attributes)
- **Bool**
- **Color**
- **Event**

Optional JSON fields (Credit, Categories) can be omitted without affecting functionality.

### Shader Editing Layout
- Specialized layout via View > Layout > Shader Editing.

### Limitations
- **Vertex shaders not supported** in Wire 7.22 (2D mode only).

### Version Support
Wire v7.22 and v7.8.

---

## Summary — Key Architectural Concepts for Audio-DNA Reference

### Wire's Data Flow Model
- **Signal flow** (per-frame, circular connectors) — analogous to our render-thread uniform updates
- **Event flow** (on-change, rectangular connectors) — analogous to our UI triggers / MIDI events
- **Attribute flow** (compile-time only, diamond connectors) — analogous to our static configuration

### Wire's FFT Integration
- 1024 float array, ~23Hz per band at 48kHz
- Three source options: Local (clip), Composition (all), External
- Related nodes convert frequency data to pitch and BPM

### Wire's Parameter System
- All inputs become UI controls in host (sliders, toggles, menus, color pickers, triggers)
- Dashboard controls ordering (matches host pickup sequence)
- Input groups create foldable sections
- Presets save/recall parameter states
- Map nodes remap ranges without changing slider endpoints

### Wire's Compilation/Distribution
- .wired (editable) and .cwired (locked) formats
- Juicebar marketplace integration
- Command-line compilation supported
- Copy-for-sharing as text (no resources)
- Consolidation packages resources

### Wire's External I/O
- MIDI: Full read/write with filtering, logic, polyphonic channels
- OSC: Read/write with address specification, bundling option
- Syphon/Spout: GPU texture sharing between apps
- ISF: Custom GLSL shaders with JSON-defined parameters (fragment only, no vertex)
- Slices: Arena projection mapping shapes accessible in Wire
