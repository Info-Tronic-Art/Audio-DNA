# Resolume Output System -- Complete Reference

Research compiled from the official Resolume manual (support pages, v6/v7).

---

## 1. Advanced Output (Overview)

The Advanced Output is Resolume's central hub for managing all physical and virtual outputs. It handles:

- **Physical outputs**: DVI/HDMI to projectors, LED screens, capture cards (Blackmagic Intensity), DMX signals to pixel strips
- **Virtual outputs**: Syphon (Mac), Spout (PC), NDI (network)
- **Projection mapping**: Aligning projected content to physical objects
- **LED processor routing**: Scaling/resizing output to fit processor requirements
- **DMX fixture targeting**: Selective routing to lighting fixtures

**Availability**: Both Avenue and Arena editions, but full features (poly slices, edge blending, DMX) require Arena.

### Preset Management

- Presets saved as `.xml` files in Resolume's documents folder
- Presets are **composition-independent** -- reusable across different compositions
- Cross-platform compatible (Mac <-> PC)
- Switched via dropdown menu in Advanced Output window

---

## 2. Screens

Screens are the top-level output containers. Each screen represents one output destination.

### Screen Types

**Connected Outputs (Monitors/Projectors)**
- Auto-detected from OS display configuration
- Shows display name and resolution
- Requires extended desktop mode (not mirrored)
- One screen per physical output device
- Emergency kill: `Ctrl+Shift+D` (Windows) / `Cmd+Shift+D` (Mac) disables all outputs

**Playback Cards (Arena only)**
- Supports Blackmagic, Datapath, AJA hardware
- Displays port selection and video format options
- Critical: match update frequency with content fps, composition fps, and display refresh rate
- Some cards support full-duplex (simultaneous input/output)

**Syphon/Spout (Texture Sharing)**
- Syphon on Mac, Spout on PC -- GPU-level texture sharing between applications
- Screen name becomes the Server/Sender name (default: "Screen 1")
- Application name shown as "Avenue" or "Arena"
- Width and height adjustable for resolution control

**NDI (Network Device Interface)**
- Network-based video protocol
- Automatically announces on network (zero-config discovery)
- Use case: "send a single 1080p output from a VJ laptop to a master server"
- Works across multiple computers on the same network

**Virtual Outputs**
- Internal routing: one screen can use another screen's output as its input
- Latency: 0 or 1 frame depending on load
- Width and height adjustable

### Per-Screen Controls

| Control | Range | Purpose |
|---------|-------|---------|
| Opacity | 0-100% | Overall screen transparency |
| Brightness | -100 to +100 | Brightness offset |
| Contrast | -100 to +100 | Contrast adjustment |
| Red | -100 to +100 | Red channel adjustment |
| Green | -100 to +100 | Green channel adjustment |
| Blue | -100 to +100 | Blue channel adjustment |
| Delay | 0-100ms | Compensate signal chain latency |

### Visibility Controls
- Toggle switches enable/disable output per screen
- Fold/unfold arrows hide slices while maintaining output active

### Virtual Screens for Pre-Show Prep
- Create virtual screens matching venue resolution before physical connection
- Configure entire show, then swap to real outputs at venue
- Created via Plus+ menu

---

## 3. Input Selection (Slices)

Input Selection determines which portion of the composition goes to each output. Slices are the fundamental unit -- each slice selects a rectangular (or polygonal) region of the composition.

### Slice Manipulation

- **Drag**: Resize, reposition, rotate on the stage
- **Snap**: Slices snap to stage edges, center, and other slices
- **Precision**: Right-side numerical fields for exact dimensions
- **Math**: Enter calculations in fields (e.g., "/3" to divide current value by three)
- **Nudge**: Arrow keys for 1-pixel movement, Shift+Arrow for 10-pixel
- **Duplicate**: Via plus+ menu or right-click

### Slice Masks (Arena only)
- Refine rectangular slices into custom shapes
- Preset shapes available plus freeform drawing
- Inversion toggle: show inside vs. outside the mask

### Polygon Slices (Arena only)
- Complete shape freedom via triangulated control points
- Create by clicking points on stage, double-click to close the polygon
- Auto-triangulation; invalid shapes highlighted in red
- Editing modes:
  - **Transform**: Move, scale, rotate the whole polygon
  - **Edit Points**: Adjust individual vertices, add/remove points, switch between linear and bezier curves

### Multi-Select
- Lasso-drag or Shift-click to select multiple slices
- Bounding box appears for simultaneous transformation of group

---

## 4. Input Maps (LED & Projection Mapping Workflows)

Input Maps define how composition content maps to physical output devices. This is the practical workflow layer on top of slices.

### Single LED Screen Workflow
1. Create a slice matching the LED wall's pixel resolution (e.g., 384x192)
2. Position it in the top-left corner of the output stage (where LED processors expect data)
3. On the input side, drag and scale the slice to cover the desired composition area
4. Use Shift+Alt while scaling to preserve aspect ratio

### Multi-Screen LED Wall (Single Processor)
- 4 screens controlled by a single processor = 1 signal from computer
- LED supplier provides output mapping specs (pixel positions per panel)
- Recreate processor's expected layout using slices in Output Transformation
- Select all input slices, "Match Input Shape", then reposition to match physical layout
- Scale to fill canvas while maintaining physical arrangement

### Slice Transform Effect
- Places specific content on individual screens within a multi-screen setup
- Allows per-screen content targeting within a unified composition

### Structure-Based Projection Mapping (Geometric)
Example: 8-cube structure, 1080x1080 composition, 360x360 per cube:
1. Create input map slices matching cube positions
2. Build layers: fullscreen layer + per-cube content layers + effects layer
3. At venue: "Match Input Shape" on all slices, then manually adjust corners to align with physical projection
4. Bypass buttons enable per-cube content variations

### Non-Geometric Object Mapping
- Requires pre-production content design based on actual object measurements
- Divide object into poly slices representing features (e.g., skull: forehead, eyes, cheeks, teeth, chin)
- Fine-tune each poly slice's position, scale, and corner points to physical surface

### Key Principles
- **Mixed pixel pitch**: Lower-res tiles can be scaled larger on input stage for correct perspective
- **Reusability**: Accurate input maps transfer between venues -- only output stage needs adjustment
- **Custom content**: Content created at matching dimensions (e.g., 360x360 cubes in 1080x1080 comp) aligns perfectly

---

## 5. Output Setup

### Computer Display Configuration

**Windows**: Settings > System > Display. Ensure "Extended desktop" mode with 2+ active displays.

**macOS**: System Settings > Displays. Select output display, choose "Extended display" from "Use as" dropdown.

### Output Modes

| Mode | Behavior |
|------|----------|
| Fullscreen | Fills selected display completely (recommended for single-screen) |
| Windowed | Composition displayed in window matching its dimensions |
| Disabled | Stops all output |

Emergency disable: `Ctrl+Shift+D` / `Cmd+Shift+D`

### Composition Output Sharing
- **Syphon** (Mac) / **Spout** (PC): Share output texture with other apps on same machine
- **NDI**: Network streaming to other computers on same network

### Display Utilities

| Utility | Function |
|---------|----------|
| Identify Displays | Overlays numbered, colored identifiers matching OS assignments |
| System Display Preferences | Quick link to OS display settings |
| Show FPS | Frame rate counter (target: above 30fps, cap below monitor refresh) |
| Show Test Card | Color bars, resolution info, time, moving line |
| Show Display Info | EDID data, GPU connections, rendering GPU info |
| Snapshot | Saves PNG still to Recordings folder and imports to composition |

---

## 6. Output Transformation (Arena only)

Output Transformation is where physical pixel alignment happens -- projection mapping warping, masking, and per-slice color correction.

### Warping Types

**Perspective Warping**
- Four large corner points
- Maintains correct perspective geometry automatically
- Compensates for projection angle: content at greater distance is scaled up to appear uniform
- Works only on regular (rectangular) slices, NOT poly slices

**Linear Warping**
- Smaller corner points inside the perspective corner points
- No perspective compensation
- Can add additional horizontal/vertical control points for complex adjustments
- Works on both regular and poly slices

**Bezier Warping**
- Switch from linear mode to display bezier handles
- Creates smooth curved mappings
- Can combine with perspective warping for "smooth curved mappings while maintaining correct perspective"
- Does NOT work on poly slices

### Masking
- Hides output portions without distorting content
- Multiple masks per slice
- Invertible (cut inside or outside)
- Preset shapes + custom freeform shapes
- Double-click outline to add points, double-click existing points to remove
- Bezier point mode for rounded corners
- Masks apply to ALL underlying slices

### Per-Slice Options

| Option | Function |
|--------|----------|
| Flip Horizontal | Mirror content horizontally (Pacman icon) |
| Flip Vertical | Mirror content vertically |
| Flip Both | Combined mirror |
| Color Correction | Per-slice brightness, contrast, R/G/B channels |
| Is Key | Replace alpha with luminance (white=visible, black=transparent) for broadcast mixer alpha output |
| Black BG | Force black background on slice regardless of content (prevents bleed from underlying slices) |

Multi-select slices for simultaneous color correction adjustments. Use cases: balancing mismatched projectors, calibrating LED panel brightness.

---

## 7. DMX Output (Arena only)

DMX output allows sending color/brightness data to LED strips and lighting fixtures via Art-Net protocol.

### DMX Lumiverses

A Lumiverse is a virtual container of DMX universes that exists only within Arena. Each universe = 512 channels.

**Properties per Lumiverse:**
- Opacity, Brightness, Contrast, R/G/B color controls (same as Screens)
- **Auto Span** (default: ON): Automatically expands to additional universes when fixtures exceed 512 channels. Sequential universes spawn from the lumiverse's Subnet.Universe.
- **Align Output**: Prevents pixel data from splitting across universe boundaries (most devices require complete pixels within a single universe)

### DMX Fixtures

- New lumiverses start with one default fixture (1 RGB pixel)
- Fixture presets available via dropdown menu
- Input selection area scales with fixture pixel count (16-pixel fixture = 16x wider)
- Center pixel of each input square is sampled for RGB channel assignment
- Preview indicators show exact transmitted colors

**Fixture Operations:**
- Move, scale, rotate fixture inputs to match physical stage positions
- Duplicate fixtures via right-click context menu
- New fixtures default to previously used fixture type

### Sending DMX Output

**Device Selection**: Right-click lumiverse to see detected Art-Net nodes on network.

**Network Interface**: Configure in Preferences > DMX tab (select correct NIC: wired vs. wireless).

**Manual IP**: Select "IP Address" from Target IP dropdown for custom devices (Arduino, etc.). Enter IP, Universe, Subnet manually.

**Broadcast**: Transmits to ALL network devices without specific IP. Requires manual Subnet/Universe config. Suitable for <30 universes.

### Patching

- DMX Output tab shows fixture arrangement and start channels
- Example: 16-pixel RGB fixture = 48 channels (16 pixels x 3 RGB values)
- Start channels adjustable via drag or manual input
- Overlapping channels highlighted in red as warning
- Per-fixture brightness/contrast and flip (horizontal/vertical) available

### ArtSync Protocol

Synchronizes all DMX receivers to display frames simultaneously:

| Setting | Function |
|---------|----------|
| Framerate | Output frequency (Hz) for lumiverse data. Higher = more responsive, more CPU/bandwidth |
| Delay | Milliseconds between frame render and DMX output. Default 40ms (syncs with projectors). 0ms for immediate (pixel strips / lights-only) |

Both settings are global (apply to all universes via DMX preferences).

### USB DMX
- Arena 6+ dropped Enttec DMX USB support (proprietary protocols)
- Art-Net only (industry standard, unlimited universes vs. USB's 1-2)

---

## 8. 10-Bit Color Output

10-bit color provides 1.07 billion colors (vs. 16.7 million in 8-bit), eliminating visible color banding in gradients.

### Resolume Configuration
- Composition > Settings > Color Depth: change from **8 bpc** to **16 bpc**
- This ensures internal processing has sufficient precision for 10-bit output

### Supported Content

| Format | 10-bit Support |
|--------|---------------|
| Apple ProRes 4444 | Yes |
| Apple ProRes 422 (all variants) | Yes |
| H.265/HEVC | No (can transcode in Alley but won't play) |
| DXV | No (8-bit only) |
| 16-bit PNG | Yes |
| 16-bit TIFF | Yes |
| JPEG, GIF | No |
| NDI | Yes |
| Spout | Yes |
| Syphon | No |

### Hardware Chain
**Every component must support 10-bit**: GPU, connector, cable, splitters, processors, display. Any 8-bit link reduces the entire chain to 8-bit.

### Connector Requirements
- DisplayPort 1.2+ (required)
- HDMI 2.0+ (required)
- Thunderbolt 3/4 in DisplayPort mode (OK)
- USB-C with DisplayPort Alt Mode (OK)
- DVI, VGA, early HDMI (NOT supported for 10-bit)

### Cable Requirements
- **DisplayPort**: Certified DP 1.4 cables (supports 8K@60Hz with 10-bit)
- **HDMI**: Premium or Ultra High Speed HDMI cables
- Avoid long runs with generic cables; test short direct connections first

### GPU Configuration

**NVIDIA**: Display > Change resolution > "Use NVIDIA color settings" > Desktop color depth: "SDR (30-bit)" or "Highest (32-bit)" > Output color format: RGB > Dynamic range: Full > Output color depth: 10 bpc

**AMD**: Settings > Display > Select display > Color Depth: 10 bpc > Pixel Format: RGB 4:4:4 (Full Range)

**macOS**: Automatic if display supports it; no configuration needed.

### Testing
Create gradient-heavy test content (linear gradients, radial spotlights, sky/fog photos). Export in both 8-bit and 10-bit formats. Load into 16 bpc composition and compare. Smooth gradients = working 10-bit; visible stepping = bottleneck in chain.

---

## 9. Streaming

Resolume does not stream directly -- it requires external software (OBS Studio).

### Output Routing to OBS

**Via NDI**: Enable "Network Streaming (NDI)" in Resolume's Output menu. In OBS, add NDI Source and select "Resolume".

**Via Syphon (Mac only)**: Enable "Texture Sharing (Syphon)" in Resolume. OBS natively supports Syphon sources. Lower latency than NDI.

### Required Software
- OBS Studio (obsproject.com)
- NDI Plugin for OBS (github.com/Palakis/obs-ndi)
- NDI Tools Virtual Input (for conference apps)

### Streaming Platforms
Configure platform stream keys in OBS Auto-Configuration Wizard for YouTube, Twitch, Facebook Live.

### Conference Apps (Windows only)
For Skype, Google Meet, Jitsi, Slack, Zoom:
1. Install NDI Tools Virtual Input
2. Run Virtual Input app
3. Send Resolume output via NDI
4. Right-click NDI Virtual Input, select Resolume
5. Select "NewTek NDI Video" as camera in conference software
6. Important: close conference apps before configuring NDI Virtual Input

### Streaming the Resolume Interface
Use OBS "Display Capture" (not Window Capture -- dropdown menus fail to render with Window Capture).

**Pro tip**: Set up multiple OBS scenes (screen capture + NDI input) with Studio Mode for crossfading.

### Receiving Streams INTO Resolume
1. Add Browser source in OBS with stream URL
2. Check "Control audio via OBS"
3. Alt+drag bounding box to crop
4. Enable NDI output in OBS Tools menu
5. In Resolume sources, select corresponding NDI server

---

## 10. Multiple Outputs (Scaling Guide)

Solutions ranked by performance and reliability:

### 1. Single GPU with Native Outputs (Best)
- Most reliable, best performance
- Max ~6 outputs on current market GPUs (5 outputs + 1 control monitor)

### 2. Single GPU + Datapath Extenders (Recommended)
- Extends single ports to multiple outputs (e.g., Datapath Fx4)
- Can daisy-chain or distribute units
- Must monitor data bandwidth (combination of refresh rate, resolution, scanning method)
- Cannot exceed max output resolution of the GPU port

### 3. Single GPU + Blackmagic Cards (Good for SDI)
- 1, 2, 4, or 8 outputs depending on model
- SDI output support (3G, 6G, 12G)
- Downside: 60ms+ latency, broadcast resolutions only (no XGA, WXGA, WUXGA)

### 4. Single GPU + MST Hubs (Budget)
- Cost-effective but "very hit or miss" compatibility
- Does NOT increase native port capacity
- Cannot exceed GPU's native port count (e.g., cannot use three 3-way hubs on a 4-port GPU to get 9 outputs)

### 5. Multiple GPUs (Not Recommended)
- Significant performance penalty (data must cross PCIe lanes)
- Not recommended for critical applications

### 6. Multiple Computers via External Protocol (Scalable but Complex)
- Most scalable approach
- Requires "considerable tinkering"
- May not achieve true frame-synchronization
- Potential timing drift between machines

### Important Warnings
- **SLI/Crossfire**: NOT supported in Resolume. Does not increase output count -- slave card outputs are disabled.
- **MST Hub limitation**: Physical GPU port maximum applies regardless of hub connections.

### Frame-Sync
Critical when displays form a "stitched" surface. Without sync, outputs refresh at slightly different times causing horizontal tearing between screens. Single-GPU solutions naturally frame-sync; multi-GPU and multi-computer solutions require explicit synchronization.

---

## Key Architectural Takeaways for Audio-DNA

1. **Screen abstraction**: A "screen" is a destination (physical display, network stream, texture share, virtual) -- the same compositor output can go to many screen types simultaneously.

2. **Slice-based routing**: The composition canvas is partitioned by slices. Each slice selects a region of the composition and routes it to a screen. This is more flexible than 1:1 fullscreen mapping.

3. **Two-stage transform**: Input Selection (what part of composition) is separate from Output Transformation (how it maps to physical pixels). This separation is powerful for projection mapping.

4. **Per-output color correction**: Brightness, contrast, and per-channel RGB adjustments per screen/slice -- essential for matching mismatched displays.

5. **Virtual outputs for internal routing**: Screens can consume other screens as inputs with 0-1 frame latency.

6. **Preset system**: Output configurations saved independently from compositions as XML, enabling venue-specific presets.

7. **DMX as first-class output**: LED strips and lighting fixtures treated as another output type alongside video displays.

8. **Texture sharing (Syphon/Spout/NDI)**: Zero-copy or network-based output to other applications is a standard feature, not an add-on.

9. **10-bit pipeline**: Requires 16 bpc internal processing, appropriate codecs, and full hardware chain support.

10. **Scaling strategy**: Single GPU with port extenders preferred over multi-GPU. Frame-sync is critical for stitched surfaces.
