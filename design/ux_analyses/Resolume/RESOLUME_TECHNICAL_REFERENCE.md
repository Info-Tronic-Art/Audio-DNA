# Resolume Technical Reference — Nerds Section & FAQ

Comprehensive extraction from Resolume's support documentation (March 2026).

---

## 1. Tech Specs — System Requirements

### Minimum Specifications

**Windows:**
- OS: Windows 10
- Graphics: Nvidia or AMD graphics card
- RAM: 8 GB

**macOS:**
- OS: macOS 10.15 Catalina
- Graphics: AMD, Iris Pro, or Apple Silicon
- RAM: 8 GB

### Recommended Specifications

**Windows:**
- OS: Windows 11
- Processor: i7, 6-core, 3.7 GHz
- Graphics: Nvidia RTX 4070
- Storage: M.2 SSD
- RAM: 16 GB

**macOS:**
- OS: macOS 14 Sonoma
- Processor: Apple M2 Max
- Storage: SSD
- RAM: 16 GB

### Software Products
- **Avenue** — VJ software for live visual performance
- **Arena** — Avenue's big brother with projection mapping, DMX, and multi-output
- **Wire** — Node-based visual patching environment
- **Alley** — Free DXV encoding tool
- **DXV Codec** — GPU-accelerated video codec

---

## 2. Directory List — File Locations

### Application Folder

| Platform | Path |
|----------|------|
| macOS | `/Applications/Resolume Avenue/` (or Arena) |
| Windows | `C:\Program Files\Resolume Avenue\` (or Arena) |

Contains: application binary, licensing, fixtures, presets, plugins, documentation, included media.

### User Documents

| Platform | Path |
|----------|------|
| macOS | `~/Documents/Resolume Avenue/` |
| Windows | `C:\Users\[username]\Documents\Resolume Avenue\` |

**Subfolders:**
- `Compositions/` — User-created project files
- `Extra Effects/` — Third-party FFGL plugins (always scanned)
- `Fixture Library/` — Custom fixture profiles
- `Preferences/` — Interface layout, shortcuts, last composition
- `Presets/` — Sources, effects, GUI, output settings presets
- `Recorded/` — Exported videos and stills
- `Shortcuts/` — MIDI, keyboard, OSC, DMX configuration files

### Log Files

| Platform | Path |
|----------|------|
| macOS | `~/Library/Logs/Resolume Avenue/` |
| Windows | `C:\Users\[username]\AppData\Local\Resolume Avenue\` |

### Thumbnail Previews & Text Atlas

| Platform | Path |
|----------|------|
| macOS | `~/Library/Application Support/Resolume Avenue/` |
| Windows | `C:\Users\[username]\AppData\Local\Resolume Avenue\` |

### Registration Files

| Platform | Path |
|----------|------|
| macOS | `/Library/Application Support/Resolume Avenue/registration` |
| Windows (v7.3.0+) | `C:\ProgramData\Resolume Avenue\registration\` |
| Windows (pre-v7.3.0) | `C:\Users\Public\Public Documents\Resolume Avenue\registration\` |

### DXV Plugins

QuickTime, Adobe (After Effects/Premiere), and QuickLook plugins are stored in system-level Adobe and QuickTime plugin directories on both platforms.

---

## 3. Preferences — All Settings

### General Preferences

| Setting | Description |
|---------|-------------|
| **Clip Panel** | Controls whether the clip panel updates when a clip is triggered. Useful to disable for external triggering (MIDI, OSC, DMX) while editing effects elsewhere. |
| **Clip Start Offset** | Millisecond offset for clip start — compensates for MIDI triggering delays or cable latency. |
| **Scrolling** | Auto-scroll the deck to show newly selected clips or layers. |
| **Font (Arial Unicode)** | Enables proper display of non-Latin characters (CJK, etc.). |
| **Software Updates** | Toggle notifications for available updates. |
| **Quit Confirmation** | Requires Ctrl/Cmd+Q confirmation before shutdown. Disable for fixed installations where the computer starts/shuts down automatically. Combine with "Trigger First Clip when Composition Has Loaded" (Layer menu) for unattended operation. |

### Audio Preferences

| Setting | Description |
|---------|-------------|
| **Audio Output Device** | Select output device. ASIO devices show additional settings button. |
| **Master Output Channels** | Choose which channels carry the main output. |
| **Preview Output Channels** | Separate preview channels (multi-channel devices). |
| **Sample Rate** | Higher = better quality, more CPU. **44100 recommended.** |
| **Buffer Size** | **512 or 1024** typically prevent glitches while minimizing latency. |
| **Audio Input Device** | Select incoming audio source. |
| **Audio FFT** | Configure external audio analysis channels with gain adjustment. "View > Show FFT Gain" exposes this in the main interface. |
| **SMPTE** | (Arena only) Select channels for SMPTE timecode sync source. |
| **VST Directories** | Specify folders containing VST audio effects and sources. |

### Video Preferences

| Setting | Description |
|---------|-------------|
| **FFGL Directories** | Directories for FreeFrame GL effects and sources. Resolume always checks the `Extra Effects` folder in user Documents. |
| **DMA Textures** | Advanced GPU setting. **"Force ON" recommended** (default). Switch to "Force OFF" only for older/lower-end GPUs with performance issues. |

### MIDI Preferences

| Setting | Description |
|---------|-------------|
| **MIDI Devices** | Enable/disable MIDI input/output devices. Monitor icon shows all incoming/outgoing messages. |
| **Middle C Convention** | Selects note display standard — C3 vs C4. "Behind the scenes, MIDI doesn't actually use musical notes, just numbers. Middle C refers to note number 60." Display-only; does not affect actual MIDI shortcut behavior. |

### OSC Preferences

- Enable/disable OSC input/output
- Configure ports and IP addresses
- Monitor icon displays all messages

### DMX Preferences (Arena Only)

- "The Network Adapter switches both Art-Net input and output to the selected adapter!" — changing the adapter affects both directions simultaneously.

### Recording

- Configure storage directory for recorded output
- Select video and audio codecs
- Set recording preferences

### Clip Rendering

- Custom output directory
- Codec selection for clip renderer output

### Defaults

| Setting | Description |
|---------|-------------|
| **Default Transport and Play Mode** | Applies to newly imported files only. Video and audio can have separate defaults. |
| **Default Blend Mode** | Sets the blend mode for newly created layers. |
| **Tempo Nudge Range** | Adjusts BPM shift percentage. **10% for audio, 25% for visual** syncing. |

### Webserver

- Activate/deactivate REST API and webserver
- Configure port and listen address
- Default ports: **Arena/Avenue = 8080**, **Wire = 8081**
- Default listen address: **0.0.0.0** (all network interfaces) or 127.0.0.1 (localhost only)

### Registration

- License management
- Computer-to-computer license transfer

---

## 4. Rendering to DXV

### DXV Codec Overview

The Resolume DXV Video Codec is **GPU-accelerated** — "decompression of video frames directly on the video card." This provides superior performance at higher resolutions with lower CPU and RAM usage.

**Critical limitation:** Hardware acceleration occurs **exclusively in Resolume playback**. "When a DXV video is played with any other software, it is not rendered by the videocard." Performance benefits do not transfer to other players.

### Quality Settings

| Setting | Description |
|---------|-------------|
| **Normal Quality** | Recommended default. Fast, small files. |
| **High Quality** | Only use when gradient banding is visible. **Doubles file size.** |
| **With Alpha** | Stores alpha channel. Source must already contain alpha; this option does not create transparency. |
| **No Alpha** | Use when source has no alpha. Minimizes file size. |

**Warning:** Do not re-encode existing DXV2 files to DXV3 HQ — artifacts from the original encoding are already baked in and will not be improved.

The codec comes "pre-configured to be as fast as possible" — no keyframe management needed.

### Export Methods

1. **Resolume Alley** — Free standalone tool for simplified DXV encoding.
2. **Adobe After Effects / Premiere / Media Encoder** — Plugins install automatically with Resolume (or via separate Alley installer). Export DXV3 directly from Adobe apps.
3. **QuickTime Player 7 Pro** — Supports DXV 3 compression via standard export.

---

## 5. WebSocket API

### Connection

- **URL:** `ws://address:port/api/v1`
- Same port and address as the REST API
- Must enable "Enable Webserver & REST API" in Preferences > Webserver
- All messages must be valid JSON

### Initial Connection Messages

Upon connecting, the server automatically sends three messages:

1. **Composition State** — Full composition structure (matches `GET /composition` response)
2. **Sources Update** — `{ "type": "sources_update", "value": <sources> }`
3. **Effects Update** — `{ "type": "effects_update", "value": <effects> }`

These messages are re-sent whenever structural changes occur (columns/layers added/removed, sources/effects installed).

### Parameter Actions (6 actions)

| Action | Description | Response |
|--------|-------------|----------|
| **subscribe** | Enable continuous updates for a parameter | `parameter_subscribed` |
| **unsubscribe** | Disable updates for a parameter | `parameter_unsubscribed` |
| **set** | Modify a parameter value | `parameter_set` |
| **get** | Retrieve current parameter value | `parameter_get` |
| **reset** | Restore default parameter value | — |
| **trigger** | Execute an action (fire-and-forget) | No response |

### Message Format

**Request:**
```json
{
  "action": "<action>",
  "parameter": "<parameter path>",
  "value": "<new_value>"  // only for "set"
}
```

**Response:**
```json
{
  "param": "<parameter info>",
  "type": "<response type>",
  "path": "<parameter path>"
}
```

Response types: `parameter_subscribed`, `parameter_unsubscribed`, `parameter_get`, `parameter_set`, `parameter_update` (for subscribed parameters that change).

### Parameter Path Formats

Two addressing methods:

1. **By ID:** `/parameter/by-id/<parameter_id>`
2. **Logical Path:** Based on composition hierarchy, with parameter name appended

**Examples:**
- `/composition/columns/1/name`
- `/composition/columns/2/colorid`
- `/composition/layers/1/clips/1/transport/position`

### Composition Operations via WebSocket

For adding/removing composition objects (layers, columns, effects):

**Request:**
```json
{
  "action": "post",        // or "remove"
  "id": "my_new_layer",   // optional, for response tracking
  "path": "/composition/layers/add",
  "body": {}               // optional, depends on action
}
```

**Response:**
```json
{
  "id": "my_new_layer",
  "error": null
}
```

### API Documentation References

- Arena REST API: `https://resolume.com/docs/restapi/`
- Wire REST API: `https://resolume.com/docs/wirerestapi/`
- Working example in Preferences > Webserver, source code on public GitLab

---

## 6. REST API

### Overview

The REST API (v7.8) enables external applications to communicate with Arena, Avenue, and Wire as web servers.

### Capabilities

- Add/Remove/Clear Columns, Layers, and Groups
- Manage Effects for composition elements
- Retrieve clip thumbnails
- List available effects and sources
- Subscribe to composition changes via WebSocket

### Configuration

| Setting | Default |
|---------|---------|
| Arena/Avenue Port | **8080** |
| Wire Port | **8081** |
| Default Listen Address | **0.0.0.0** (all network interfaces) |
| Localhost Alternative | **127.0.0.1** |

Enable in Preferences > Web Server tab.

**Constraint:** The root directory cannot contain a folder named "API."

### API Documentation

- Arena/Avenue: `https://resolume.com/docs/restapi/` (Swagger UI — requires JavaScript rendering)
- Wire: `https://resolume.com/docs/wirerestapi/`
- Interactive testing available directly from the documentation pages
- Browser-based example applications provided

**Note:** The Swagger docs are JavaScript-rendered and require a browser to view the full endpoint listings. Key known endpoints include:
- `GET /api/v1/composition` — Full composition state
- Endpoints for layers, columns, clips, effects, sources, groups
- Thumbnail retrieval for clips
- Parameter get/set via REST paths matching the WebSocket parameter paths

---

## 7. Expressions

### Purpose

Resolume Arena, Avenue, and Wire support mathematical expressions in numeric input boxes, allowing direct calculations instead of external computation.

### Available Functions

| Function | Description |
|----------|-------------|
| `min(a, b)` | Minimum value |
| `max(a, b)` | Maximum value |
| `sin(x)` | Sine |
| `cos(x)` | Cosine |
| `tan(x)` | Tangent |
| `abs(x)` | Absolute value |
| `radians(x)` | Convert degrees to radians |
| `sqrt(x)` | Square root |

### Available Constants

| Constant | Value | Description |
|----------|-------|-------------|
| `pi` | 3.14159... | Pi |
| `tau` | 6.28318... | 2 * Pi |
| `phi` | 1.61803... | Golden ratio |

### Operators

Standard arithmetic: `+`, `-`, `*`, `/`

### Practical Examples

- **Arena positioning:** For a 1920x1080 composition with content scaled to 50%, the expression `-1920/4` achieves pixel-perfect left-side positioning.
- **Wire patching:** Constants like `pi`, `phi`, and `tau` speed up workflow and reduce unnecessary nodes.

---

## 8. Modifiers (Keyboard Shortcuts & Key Modifiers)

This section covers keyboard shortcuts and modifiers for the Advanced Output interface (Input Selection and Output Transformation views).

### General Shortcuts

| Shortcut | Action |
|----------|--------|
| Ctrl/Cmd + Z | Undo |
| Ctrl/Cmd + Y | Redo |
| Ctrl/Cmd + +/- | Zoom in/out |
| Mouse scroll wheel | Zoom |
| Spacebar + drag | Pan |

### Movement & Transformation Modifiers

| Modifier | Context | Effect |
|----------|---------|--------|
| **Shift** | Dragging | Constrain movement to X or Y axis |
| **Alt** | Dragging | Create a duplicate |
| **Ctrl** | Dragging | Temporarily disable snapping |
| **Shift** | Scaling | Constrain aspect ratio |
| **Alt** | Scaling | Scale from center (not opposite edge) |
| **Shift** | Rotating | Constrain rotation to 45-degree increments |

### Nudging

| Input | Movement |
|-------|----------|
| Arrow keys | 1 pixel per press |
| Shift + Arrow keys | 10 pixels per press |

### Slice Operations

| Shortcut | Action |
|----------|--------|
| Ctrl/Cmd + D | Duplicate slice; repeats prior transformation when used again |
| Backspace | Delete selected slice(s) |

### Mask & Polygon Point Editing

| Action | Effect |
|--------|--------|
| Double-click path | Add point |
| Double-click point | Delete point |
| Shift-click point | Toggle point selection |

### Context Menu Options

- Center X, Center Y, Mirror X, Mirror Y
- Half-area selections
- Shape creation: Triangle, Circle
- Layer ordering
- Duplication
- "Match Input / Output Shape" — aligns slices between input and output stages

### View & Guide Features

| Feature | Description |
|---------|-------------|
| **Input/Output Guides** | Load still images as background references with adjustable opacity |
| **Show Slice Grid** | Display outlines and grid for selected slices |
| **Show 8x8 LED Grid** | Enable pixel-based snapping (useful for LED pixel maps) |
| **Identify Slices** | Highlight slices on mouseover |
| **Show in Output** | Display guides in actual output with red crosshairs |

---

## 9. Fixture Editor

### Purpose

Create custom light fixture personalities when a specific fixture type is not in the preset list. Access via the gear icon next to the Fixture selection dropdown.

### Creating Fixtures

- Click the plus icon to create a new fixture
- Default: 1 RGB pixel fixture called "New Fixture"
- Naming convention: manufacturer name + pixel count

**Critical:** "If you update an existing fixture, and this fixture is already being used on the Input Selection stage, you will update all instances." Always duplicate a fixture before modifying it to avoid updating existing uses.

### Pixel Configuration

| Example | Width | Height |
|---------|-------|--------|
| LED strip (16 pixels) | 16 | 1 |
| Square tile (64 pixels) | 8 | 8 |

### Dummy Channels / Parameters

Additional channels accommodate fixture-specific features (e.g., chase preset switching, dimmer channels). Add via the plus icon in the Parameters tab; reorder with drag handles.

**Important DMX concept:** "Sending nothing is not a concept that exists in DMX." The protocol requires constant channel values. Resolume sends the "Default" value for each parameter. For dimmer channels, **set default to 255** to keep fixtures on.

### Channel Distribution

16 distribution patterns define how pixels "snake" through the fixture grid:
- Left-to-right, top-to-bottom (book-style reading order)
- Zigzag / back-and-forth patterns
- Various start corner and direction combinations

**Limitations:** Grid-based distributions only; single pixel block maximum.

### Color Space Options

| Mode | Channels/Pixel | Description |
|------|----------------|-------------|
| **RGB** | 3 | Full color — separate R, G, B channels |
| **L (Luminance)** | 1 | Monochrome — weighted luminance |
| **Alpha** | 1 | Uses video content presence (not luminance) to control channel |
| **CMY** | 3 | Converts RGB to Cyan, Magenta, Yellow |
| **RGBW** | 4 | RGB + White — approximates colors by dimming RGB and using white channel |
| **RGBWA** | 5 | RGB + White + Amber |

**Unsupported:** 16-bit color, UV channels, exotic color spaces.

### Gamma Correction

Default: **2.5** (suits most LED types). Adjust for unusual LED behavior.

---

## 10. Avenue vs Arena Differences

### Shared Features (Both Versions)

- Video and audio file playback
- Live camera integration
- Over 100 effects and sources
- MIDI and OSC controller compatibility
- VJ performance-focused design

### Arena-Exclusive Features (10 capabilities)

| Feature | Description |
|---------|-------------|
| **Projection Mapping** | Map visuals onto 3D surfaces |
| **Edge Blending** | Stitch multiple projectors seamlessly |
| **SMPTE Timecode Input** | Synchronize to external timecode |
| **Denon StageLinq** | Integration with Denon DJ hardware |
| **Pioneer Pro DJ Link** | Integration with Pioneer DJ hardware |
| **DMX Control** | Receive DMX input for parameter control |
| **DMX Fixture Output** | Output DMX to control lighting fixtures |
| **Capture Card Output** | Route output to capture cards (Blackmagic, etc.) |
| **Groups** | Group layers for collective control |
| **Slice Transforms** | Advanced output slice manipulation |

### Demo

Both versions offer fully functional demos. Limitations: visual watermark on output and periodic audio reminders.

### Upgrade Path

Buy Avenue first, upgrade to Arena later by paying the price difference only.

---

## 11. Wire Patch Compatibility

### Version Support

Wire patches work with both Avenue and Arena starting from **version 7.4 and later**.

### Determining Minimum Version

- Open the patch file and check the **Patch panel** for the "minimum required version"
- The **Stats panel** shows per-node version requirements
- Minimum version is determined by "the nodes/features used in the patch," not the Wire compiler version

### Compatibility Direction

| Direction | Supported? | Notes |
|-----------|------------|-------|
| Forward (newer Wire opens older patches) | Yes | Always works |
| Backward (older Wire opens newer patches) | No | Generally fails |

### Breaking Changes

When Wire undergoes breaking changes, the system "will do its best to automatically insert conversion nodes" to maintain original patch behavior. Example: when the Y-axis was inverted, affected patches automatically received Transform nodes.

### Distribution Strategy

For selling or sharing patches: **"build the patch in that target version from the start."** Do not prototype in the latest version then attempt to copy backward — it is unreliable.

### File Formats

| Extension | Description |
|-----------|-------------|
| `.wired` | Editable compiled patch — can be opened in Wire and modified |
| `.cwired` | Non-editable compiled patch — runs in Avenue/Arena but cannot be opened in Wire |

Users do **not** need to own Wire to run compiled patches in Avenue/Arena.

---

## Appendix: Key Technical Takeaways for Audio-DNA v2

### Relevant to our architecture:

1. **REST/WebSocket API pattern** — Resolume exposes full composition state via REST (port 8080) and WebSocket (`ws://host:port/api/v1`). Parameter addressing uses logical paths (`/composition/layers/1/clips/1/...`) or by-ID (`/parameter/by-id/<id>`). The WebSocket supports subscribe/unsubscribe for real-time parameter monitoring. This is a mature pattern worth studying if we ever add remote control.

2. **Audio defaults** — Sample rate 44100, buffer size 512 or 1024. Our 48kHz / 128 buffer is more aggressive but appropriate for analysis (not playback).

3. **DXV/HAP codec strategy** — GPU-accelerated decode only works inside Resolume. The codec choice (DXV3 Normal vs High Quality) trades file size for gradient quality. Alpha support requires the source to already have alpha. Relevant for our HAP Alpha video support in P11.

4. **Fixture/DMX model** — DMX requires constant output values; "sending nothing" is not possible. Gamma default 2.5. Color spaces: RGB, L, Alpha, CMY, RGBW, RGBWA. Relevant if we ever add DMX output.

5. **Expression system** — Simple math expressions in numeric fields (min, max, sin, cos, tan, abs, radians, sqrt, pi, tau, phi). Lightweight but useful UX feature.

6. **Directory structure** — Clean separation: application folder (read-only), user documents (compositions, presets, shortcuts, recordings), system-level (logs, thumbnails, registration). Worth emulating for our preset/config organization.

7. **Arena vs Avenue tiering** — 10 features gate the upgrade: projection mapping, edge blending, SMPTE, DJ link (Denon/Pioneer), DMX in/out, capture card output, groups, slice transforms. Core VJ functionality is in the base product.
