# Resolume "Controlling Resolume" -- Complete Research Document

Sourced from: https://resolume.com/support/en/ (10 pages)
Date: 2026-03-19

---

## 1. Parameters

Source: https://resolume.com/support/en/parameters

### Parameter Types

**Sliders**
- Drag left/right to set values.
- Click the numerical value to type a precise number.
- Supports math expressions in the text field (e.g., typing `1920 / 3` auto-calculates).

**Dingetje ("Thingy")**
- A popup helper that appears while dragging a slider.
- Provides a text field for quick numeric entry plus arrow buttons for increment/decrement.
- Name is Dutch for "thingy" or "gizmo."

**Compound Sliders**
- Expandable sliders that reveal sub-parameters.
- Example: Rotation expands to Rotation X, Y, Z for 3D axis control.

**Reset**
- Right-click any slider or parameter name to restore default values.

### Button Controls

**Toggle Buttons**
- Binary on/off.
- Examples: Layer Bypass (B), Solo (S), effect bypass toggles, checkboxes (Flip Horizontal/Vertical).

**Event Buttons**
- Fire-and-forget triggers that reset after action.
- Examples: Layer eject (X), Cube Tiles randomize.

**Radio Buttons**
- Grouped -- only one active at a time.
- Examples: clip playhead direction (forward/backward/pause), layer Autopilot settings.

### Dropdowns
- Expandable option lists.
- Example: blend modes (very long list), clip playback modes.

### Color Parameters

**Color Picker** -- Standard visual picker.

**Eyedropper Tool** -- Sample any on-screen pixel, including output monitors and actual performance output. Press ESC to exit.

**RGB/HSB Sliders** -- Numerical input via R/G/B or H/S/B. Includes alpha channel slider. Supports parameter animation.

**Palette System**
- Store favorite colors in swatches.
- Drag colors from preview to empty palette slots.
- Right-click swatches for update/delete.
- Sort and Flip options reorder by various filters.
- Built-in presets via P dropdown.
- Custom presets via Save/Manage.

**Minimalist Mode** -- Click mode icons to hide controls; disable all for maximum minimalism.

### Automation
All parameters can be automated via Parameter Animation, OSC, MIDI, keyboard shortcuts.

---

## 2. Parameter Animation

Source: https://resolume.com/support/en/parameter-animation

### Access
Click the grey cogwheel icon next to any parameter name. The animation menu varies by parameter type (clip, layer, group, composition).

### Animation Modes

#### Timeline
- Interface similar to a video timeline.
- Forward/backward playback, speed control, duration adjustment.
- In/out points for trimming.
- Looping, ping-pong, random frame selection, one-shot modes.
- **Duration behavior:** By default, moving in/out points updates duration. Once you manually set duration, Resolume locks it and won't auto-update. Toggle via "Duration Changes with In & Out Points" in Animation Settings.

#### BPM Sync
- Time-based animation expressed in musical beats instead of seconds.
- Changing in/out points doesn't auto-update beat duration by default (toggleable in Animation Settings).

#### Dashboard
- Special animation mode with its own dedicated UI section.

#### Clip Position (clips only)
- Synchronizes parameter to the clip's playhead position.
- Responds to scrubbing, skipping, speed changes, direction reversals, beatloops.

### Audio Analysis (FFT)

**External FFT**
- Drives parameters from external audio devices (DJ feed, mic, band).
- Two-step setup: (1) select audio input device in preferences top-right, (2) choose specific channels in preferences bottom-left.

**Composition FFT** -- Uses any audio playing within the composition.

**Clip/Layer/Group FFT** -- Uses audio from specific clip, layer, or group.

**FFT Controls:**
| Control | Function |
|---------|----------|
| L/M/H buttons | Select low, mid, or high frequency range |
| In/Out points | Fine-tune which part of the frequency spectrum is used |
| Gain | Amplify signal strength |
| Fall | Controls decay speed from peaks |
| Direction (> < - +) | Drive parameter low-to-high, high-to-low, or modulate speed |

**Tip:** Master gain available via View > Show Audio Gain on center toolbar.

### Crossfader Phase
- Available on Composition, Layer, and Group parameters.
- Expresses crossfader position as 0-100% slider.
- Typically paired with envelope for A-to-B crossfader animations.

### Parameter Presets
Presets store: playback mode, loopback mode, timeline mode, duration, envelope, start settings, animation settings. In/out points are **excluded**.

### Envelope
Applies to all animation types. Provides curve-based shaping of the animated value.

### Special Animation Targets

**Toggles & Events**
- Timeline shows color shifts at switch points.
- Envelope points snap to toggle values.
- Effect bypass buttons support animation via right-click menu.

**Dropdowns**
- Timeline shows each option as alternating grey bands.
- Envelope selects specific options, ignoring others.

**Color Palettes**
- Timeline cycles through palette colors.
- Envelope refines color selection and sequencing.
- **Color Interpolation:** Enable in Animation Settings for FFT-driven palettes -- creates smooth rainbow transitions between colors.

**Basic Mode**
- Uses a single slider to control the "phase" of a toggle or dropdown.
- Values below 0.5 = off/first option; above 0.5 = on/second option.

### Parameter Start Settings

**Animation Triggers** -- Control when animations begin:
- Composition defaults: load from disk.
- Layer defaults: load from disk + clip trigger.
- Clip defaults: clip trigger only.
- Multiple conditions can be active simultaneously.
- Manual start option for button-triggering effects.

**BPM Phase Lock**
- Disable in Start Settings to "free" parameters from Master BPM phase synchronization.
- Parameter remains tempo-synced but runs independently.
- Useful for starting BPM-synced clip animations simultaneously with clip playback.

---

## 3. Keyboard Shortcuts

Source: https://resolume.com/support/en/keyboard-shortcuts

### Setup
- Menu: Shortcuts > Edit Keyboard.
- Hotkey to enter edit mode: Ctrl+Shift+K.
- All blue-highlighted UI elements can receive shortcuts.
- Press Escape to exit edit mode.

### Shortcut Modes

**Toggle** -- Basic on/off. Default for binary controls like Composition Bypass.

**Piano (Momentary)** -- Control active while key held, deactivates on release. Optional invert: OFF while held, ON when released.

**Range** -- For sliders. Sets min value on press, max value on release. Values can be dragged or typed. Piano option available for pulsing effects. Example: strobing opacity.

**Value** -- Sets slider to a specific absolute value on press, regardless of current state. Useful for parameter reset.

**Mouse** -- Keyboard-only feature. Hold key to control parameter via mouse movement. Horizontal or vertical direction options. Range values define movement bounds. Example: P key for clip position X/Y control.

### Multiple Shortcuts
- Can assign multiple shortcuts to a single parameter.
- Duplicate via right-click menu.
- Each shortcut can have independent settings (different rate/value combos).

### Shortcut Targets

| Target | Behavior |
|--------|----------|
| **By Position** | Applies by clip/layer order number. Survives reordering. Default for layer/group panels. |
| **This Clip/Layer/Group** | Follows the specific item if moved. Deleted when item is deleted. |
| **Selected Clip/Layer/Group** | Applies to currently selected item. Default for clip panel. |

### Shortcut Groups (multi-option controls)

**Direct Assignment** -- Individual keys for specific group items. Piano mode: switches on press, reverts on release.

**Next/Previous/Random** -- Single key cycles through all items.

**Cycling Specific Items** -- Duplicate shortcuts assigned to different items. Single key toggles between the assigned items.

### Shortcut Presets
- Save/load shortcut layouts.
- Stored as XML files.
- Shareable via drag-and-drop.
- Management: New / Save as / Remove / Rename.
- List view shows all shortcuts, sortable by name or value.
- Red highlighting = duplicate assignments.
- Delete via backspace or right-click.

### Default Layout
Ships with default keyboard shortcuts. Downloadable QWERTY layout image available.

---

## 4. MIDI Shortcuts

Source: https://resolume.com/support/en/midi-shortcuts

### Setup
- Preferences > MIDI tab.
- Toggle "MIDI Input" and "MIDI Output" per device.
- MIDI Monitor (right side of MIDI Preferences) shows all incoming/outgoing messages in real-time.

### Shortcut Assignment
1. Shortcuts menu > select MIDI protocol.
2. Interface changes color to indicate MIDI edit mode.
3. Click target control in UI.
4. Press the button/knob/fader on the controller.
5. Escape to exit.

### Modes for MIDI Notes (buttons/pads)

| Mode | Behavior |
|------|----------|
| **Toggle** | Press toggles on/off. |
| **Piano** | Active while held, off on release. Invert option available. |
| **Range** | Jump between min/max slider values on press/release. |
| **Value** | Set slider to one specific value. |
| **Velocity** | Uses force of press (0-127). Harder = higher value. Configurable range. |

### Modes for MIDI CC (faders/rotaries)

**Absolute Mode**
- Standard fader: 0-127 maps to parameter range.
- Can invert or set specific sub-range.

**Button Mode**
- Treats CC output as button presses (for controllers like Korg NanoKontrol that send CC for buttons).
- Enables Toggle/Value modes on CC messages.

**Relative Mode**
- For endless rotaries (no physical stops).
- Configure via "Steps" or "Step Size."
- Loop option: parameter wraps at boundaries.
- Supports invert.
- Useful for scratching playheads, BPM adjustment.

**Fake Relative Mode**
- Converts standard (non-endless) rotaries to act like endless rotaries.
- Applies relative mode options to non-relative hardware.

### Shortcut Groups with MIDI CC
- CC values (0-127) distributed across group items.
- Available for clips, layers, and columns.
- Example: 4 options = values 0-31, 32-63, 64-95, 96-127.

### Shortcut Targets
Same three modes as keyboard: By Position, This Clip/Layer/Group, Selected Clip/Layer/Group.

### MIDI Clock Synchronization
- Master device sends Clock Start, Clock Tick, Clock Stop.
- Resolume listens and adjusts BPM automatically.
- On Clock Stop: "Start/Stop" (halts BPM) or "Switch to Manual" (continues at last tempo).
- **Limitation:** MIDI Clock described as "notoriously wavy" and less accurate. Ableton Link recommended for tighter sync.

### MIDI Output / Feedback

**Colored Pad Control**
- Controllers with RGB pads respond to velocity values sent from Resolume.
- Different velocities = different pad colors.
- Lookup tables provided for major controllers (APC40Mk2 has color swatches).

**State Feedback**
- Toggle/Value shortcuts: configurable Off and On velocity values.
- Clip Triggers: 5 separate states, each with individual color assignments.

**Output Routing**
- Default: feedback to same device as input.
- Can redirect to a different device, all devices, or disable output entirely.

### Multiple MIDI Devices
- Resolume distinguishes devices by connection order.
- Same button on different controllers can trigger different functions.
- No need for MIDI routing software (Bomes, etc.).
- **Caution:** Device order determined by detection sequence. Reconnecting in different order swaps assignments.

### Default Presets Provided
- APC40Mk2
- APC Mini
- NanoKontrol 2

---

## 5. DMX Shortcuts (Arena only)

Source: https://resolume.com/support/en/dmx-shortcuts

### Setup
- Requires Art-Net protocol (physical DMX needs a DMX-to-Art-Net converter box).
- Configured in Preferences > DMX tab.
- Creates virtual "Lumiverses" for signal organization before connecting to actual Art-Net nodes.
- Shortcuts persist even when nodes disconnect.

### Lumiverse Configuration
- Default: Subnet 0, Universe 0.
- Supports multiple Lumiverses for >512 channels.
- Channel offset available to avoid conflicts on shared universes.
- Desk auto-appears if it supports Artpoll.

### Shortcut Assignment
- **DMX Learn:** Press the button to assign.
- **Manual:** Right-click > "Create DMX Shortcut."
- Options: Invert, 16-bit control, manual channel, min/max range, Lumiverse selection.

### Shortcut Targets
Same three modes: By Position, This Clip/Layer/Group, Selected Clip/Layer/Group.

### Shortcut Groups with DMX
DMX values distributed across options:
- 4-option example (Clip Direction): Values 1-63, 64-128, 128-191, 191-255.
- Special Clip Trigger control for triggering multiple clips via single channel.

### Network Configuration
- Device and computer must share IP range.
- Recommended ranges: 10.x.x.x and 2.x.x.x (per Art-Net spec).
- Subnet mask: typically 255.0.0.0 or 255.255.255.0.
- Network adapter selectable in DMX Preferences (affects both input and output).
- Useful for segregating Art-Net from other protocols (NDI, OSC).
- Localhost option available but officially discouraged as unreliable.

### Art-Net Universe Counting
Formula: **Subnet x 16 + Universe + 1**
- 16 Subnets x 16 Universes = 256 possible universes.
- Resolume follows Art-Net spec (Subnet/Universe designation, starting at 0).

### Art-Net Monitor
- Built-in diagnostic tool (access via arrow in DMX Preferences).
- Displays 16 Subnets x 16 Universes grid.
- Active subnets/universes shown in dark green.
- Shows per-channel data and history.
- No activity anywhere = network/source problem.
- Activity present but no response = verify Lumiverse settings and shortcut config.

### Node Identification
- Default name: "Arena" + computer name.
- Customizable for network identification of multiple instances.

### Presets
Same XML-based save/load/share system as keyboard and MIDI presets.

---

## 6. OSC (Open Sound Control)

Source: https://resolume.com/support/en/osc

### Core Concept
Network-based control with finer precision than MIDI (float 0.0-1.0 vs MIDI 0-127) or DMX. Messages are sent to fixed addresses without requiring preset configuration.

### OSC Address System

**Structure:** Every UI element has a fixed address.
Example: `/composition/layers/1/video/opacity 0.25` sets layer 1 opacity to 25%.

**Finding Addresses:** Shortcuts > Edit OSC mode. Click any UI element; the Shortcuts panel shows its address for copying.

**Note:** "The list of addresses changes depending on how you have your composition configured." A complete static list is impractical.

**Absolute vs Relative Addresses**
- Absolute: Controls a specific item (e.g., Goo effect on layer 1 only).
- Relative: Controls the same parameter on the currently selected layer. Safe to send even if the target doesn't exist on the selected layer.

### Data Types

| Type Tag | Description | Example |
|----------|-------------|---------|
| **Float** | 0.0-1.0, mapped to parameter range | `/composition/video/effects/transform/scale 0.5` (maps to 0%-1000%) |
| **Int** | Whole numbers select options | Blend modes 0-50 (51 total, 0-indexed) |
| **Color** | Type tag "r", 32-bit RGBA unsigned int via bitshifting | -- |
| **String** | Text input (type tag "s") | Text Block source content |

### Value Specification Methods

**Absolute Values:** Prefix with "a":
`/composition/layers/1/clips/1/video/effects/transform/positionx "a" 320` -- sets x-position to 320 pixels.

**Relative Operators:**
| Operator | Effect | Example |
|----------|--------|---------|
| `"+"` | Add | `"+" 50` adds 50px |
| `"-"` | Subtract | `"-" 30` subtracts 30px |
| `"*"` | Multiply | `"*" 2` doubles value |

**String Names:** Send parameter option names as strings:
`/composition/selectedlayer/video/mixer/blendmode "Alpha"` selects alpha blend mode.

### Network Configuration
- Computers must be on same network.
- OSC messages are small; WiFi is fine unless sending thousands/sec.
- **Service Discovery:** Resolume announces via ZeroConf (Bonjour on Mac). Compatible apps auto-detect.
- **Manual:** Enter IP address and port (default port: **7000**).
- Resolume shows its IP in OSC Preferences.

### OSC Input
- Enabled via Preferences > OSC tab.
- Logs last 200 received/sent messages in a foldable debug window.

### OSC Output

**"Output All OSC Messages" Preset:** Sends every interface change -- clip triggers, mouse input, MIDI, parameter animation, clip playhead position.

**Custom Presets:** Shortcuts > Edit OSC > New... to selectively enable output for specific elements, reducing network traffic.

**Right-Click Enabling:** Right-click UI elements to enable output, or use OSC Shortcuts panel.

**Wildcard Scoping:** After creating output for one layer/clip, change scope to "All Layers" or "All Clips" to batch-enable output.

**Custom Addresses:** Select "Custom Address" from dropdown; Resolume sends values to a user-specified address.

**Destinations:**
- Networked apps via ZeroConf/Bonjour.
- Localhost (this computer only).
- Broadcast (all network computers).
- Manual IP with custom outgoing port.

**Bundling:** Toggle "Use Bundles" to pack multiple messages into a single bundle. Most OSC apps don't care either way.

### Polling
Query any parameter by sending `"?"` to its address:
`/composition/layers/1/video/mixer/blendmode "?"` returns the current value.

### Compatible Applications
TouchOSC, Vezer, Processing, Max/MSP, Pure Data, openFrameworks, Arduino, TouchDesigner, JUCE, Max4Live (Ableton Live), JavaScript browser libraries, Resolume Wire.

### Related Protocols
REST API & Webserver, Websocket API.

---

## 7. SMPTE (Arena only)

Source: https://resolume.com/support/en/smpte

### Overview
Resolume Arena syncs video playback to external SMPTE timecode signals. Can monitor two simultaneous SMPTE inputs. Used for synchronized control of audio, video, lights, pyro, lasers in professional shows.

### Technical Details
- SMPTE is an audio signal containing frequency variations interpreted as timing data.
- Connection: SMPTE source via audio input (line-in).
- **Warning:** Do NOT use speaker playback with onboard microphones -- this will fail.

### Setup
- Audio tab in Preferences for initial configuration.
- SMPTE panel accessible via View menu.
- Cogwheel icon allows color customization of timecode display.

### Key Parameters

| Parameter | Details |
|-----------|---------|
| **Framerate** | Must match incoming signal (commonly 25 or 29.97 fps). Wrong framerate causes regular 1-second playhead jumps. |
| **Offset** | Starting timecode for individual clips. Convention: sequential 1-hour offsets per show (Show 1 = 01:00:00:00, Show 2 = 02:00:00:00). |
| **Delay Compensation** | Frame-based adjustment. Negative values compensate for audio traveling slower than light on large stages. |

### Operation
- Timeline dropdown designates clips for SMPTE 1 or SMPTE 2.
- Special icon indicates active timecode listening.
- Bottom-right buttons for rapid input switching between sources.
- **Limitation:** SMPTE unavailable on clips containing audio tracks.
- Clip triggering itself doesn't transmit via SMPTE; clip must be active in a layer.

### Use Cases
- Automated sync services: Timecode Live, TC Supply, ProDJLink.
- DIY: Split-channel technique (music on left, SMPTE on right).

### Important Caveat
SMPTE is playhead synchronization for pre-finished content, not a creative timing tool. Requires pre-prepared video and audio tracks.

### Tool
SMPTE audio files can be generated at: http://elteesee.pehrhovey.net/

---

## 8. Sync to Denon DJ Players (Arena only)

Source: https://resolume.com/support/en/sync-to-denon-players

### Overview
Arena v6+ automatically syncs video playback to Denon DJ players. Monitors loaded tracks and triggers corresponding videos.

### Supported Hardware
- X1800 Mixer
- SC5000(M), SC6000(M) Players
- Prime 4 (Arena 6.1.2+)
- Prime 2 (Arena 7.1.1+)
- Prime GO (Arena 7.1.1+)
- Up to 4 players simultaneously.

### Network Setup
- Players and Resolume on same network.
- Stage connection: Use "PC" connection on Denon X1800 mixer.
- Front of house: Computer must share IP range and subnet mask.
- Recommended: DHCP router for automatic IP.
- Firewall: Resolume indicates required ports; full disable suggested.
- Access via View menu > Denon StageLinQ.

### Toolbar Display
Shows per-player: fader opacity, player number (1-4), play/pause/stop status, time remaining, album artwork, track/artist info.

### Linking Clips to Tracks
- Drag player from toolbar onto clip.
- Video playhead syncs to audio timeline automatically.
- Video triggers when track loads on player.
- Follows all DJ manipulations: scratches, loops, slices.
- Layer locks during playback (prevents accidental ejection).
- If video ends before audio, Resolume ejects the clip.
- Track identified by ID3 title (filename fallback).
- Manual setup: Switch playback mode to Denon, enter track name manually.
- **Restriction:** Only available on clips without audio tracks.

### Configuration Options

| Option | Description |
|--------|-------------|
| **Player Selection** | Restrict to specific player (1-4) or "Any" |
| **Fader Control** | Link video layer opacity to mixer fader. Video invisible until audio audible. Includes crossfader. Can be disabled. |
| **Offset** | Timing compensation for audio/video start point discrepancies |

### Player Layer Target
- Automatically routes videos to predetermined layers based on which player plays.
- Setup: Right-click player in toolbar > assign target layer per player.
- Clip configuration: Set "Clip Target" to "Denon Player Determined" via right-click.
- Can apply at composition level for all newly imported content.

---

## 9. Sync to Pioneer DJ Players (Arena only)

Source: https://resolume.com/support/en/sync-to-pioneer-dj-players

### Overview
Arena v7.12+ syncs with Pioneer DJ equipment for automatic video-to-audio sync.

### Supported Hardware
- CDJ-3000, CDJ-2000NXS2
- DJM-V10, DJM-V10-LF, DJM-900NXS2
- Up to 4 players.

### Setup (more complex than Denon)
- Ethernet hub connecting players, mixer, and Resolume machine.
- **Requires Pioneer's Pro DJ Link Bridge application** (free) OR ShowKontrol.
- Best practice: Install Bridge on same machine as Arena.
- Select correct network interface in Bridge (IPs typically "192.254...").
- Configure both "PRO DJ Link" and "TCNET" tabs in Bridge.
- Match network interfaces between Bridge and Arena (Preferences > General > Pioneer DJ Network Interface).
- Disable WiFi; maintain single active wired interface.
- Open firewall ports or disable entirely.
- Enable via View menu > Pioneer DJ TCnet.

### Toolbar Display
Same as Denon: fader opacity, player number, play/pause/stop, time remaining, artwork, track/artist.

### Linking & Behavior
- Drag player from toolbar onto clip.
- Syncs playhead to audio. Triggers on track load.
- Follows scratching, looping, slicing.
- Layer locks during playback.
- Video won't be stretched to match audio length.
- Track ID by player title (filename fallback).
- Manual setup: Switch playback mode to Pioneer, enter track name.
- **Restriction:** Only on clips without audio tracks.

### Configuration
Same as Denon: Player selection (1-4 or Any), Fader control (opacity linked to mixer fader), Offset compensation.

### Player Layer Target
Same system as Denon: right-click player > assign layer, set clips to "Pioneer Player Determined."

---

## 10. Sync with a DJ (Overview)

Source: https://resolume.com/support/en/sync-with-dj

### Concept
Resolume can synchronize visuals with DJ performances. Used by "DJ Mag top DJs." Requires substantial preparation -- cannot rely on improvisation.

### Requirements
- Pre-prepared videos matching each track in the DJ set.
- Videos must align precisely with music timing (e.g., lyrics appearing synchronously with vocals).

### Three Synchronization Methods

| Method | Notes |
|--------|-------|
| **Denon Players/Mixers** | Easiest, built-in Arena support |
| **Pioneer DJ Equipment** | Built-in, requires Bridge app |
| **SMPTE** | Alternative for non-Denon/Pioneer setups |

---

## Cross-Cutting Themes

### Universal Shortcut System
All control protocols (keyboard, MIDI, DMX, OSC) share the same targeting system:
1. **By Position** -- Targets by index number, survives reordering.
2. **This Clip/Layer/Group** -- Follows specific item, deleted with item.
3. **Selected Clip/Layer/Group** -- Targets current selection, adapts dynamically.

### Universal Preset System
All protocols use XML-based presets: save, load, share via drag-and-drop. Sortable lists with duplicate detection (red highlighting).

### Parameter Automation Hierarchy
Parameters can be driven by:
1. Manual UI interaction
2. Keyboard/MIDI/DMX/OSC shortcuts
3. Timeline animation (time-based or BPM-synced)
4. FFT audio analysis (external, composition, clip, layer, or group)
5. Clip position (playhead-synced)
6. Crossfader phase
7. Dashboard controls

### FFT Audio Analysis Details
- Frequency selection: Low, Mid, High buttons + in/out fine-tuning.
- Gain amplification.
- Fall (decay speed from peaks).
- Direction: low-to-high (>), high-to-low (<), speed modulation (- and +).
- Sources: External audio device, composition audio, clip/layer/group audio.
- Color interpolation available for FFT-driven palette animations.

### Sync Restrictions
- SMPTE, Denon sync, and Pioneer sync are all **only available on clips without audio tracks**.
- All sync methods lock the layer during playback to prevent accidental changes.
- Video must match audio duration; Resolume won't stretch content.
