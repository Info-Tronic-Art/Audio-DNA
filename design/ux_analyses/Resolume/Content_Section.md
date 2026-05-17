# Resolume Manual — Content Section (Complete Research)

Fetched 2026-03-19 from resolume.com/support/en/*

---

## 1. Effects
**Source**: resolume.com/support/en/effects (v7.19, 7.17, 7.16, 7.7, 7.0.4, 6)

### Effect Types & Counts
- Over 100 built-in video effects
- A handful of built-in audio effects
- Third-party support: VST plugins (audio) and FFGL plugins (video)

### Adding Effects
- Drag from the Effects panel (bottom right in default layout) onto Composition, Group, Layer, or Clips panel
- Quick search: double-click panel header or empty space in the effects panel
- Preview: double-click effect names in the effects panel to preview them
- **ALT-drop trick**: Hold ALT while dropping an effect to apply it with opacity turned all the way down

### Effect Application Hierarchy
1. **Clip effects** apply to the individual playing clip
2. **Layer/Group effects** apply after clip effects
3. **Composition effects** apply to the final output after all layer mixing

### Audio Effects
- All audio effects share a **Dry/Wet** slider controlling mix between affected and original audio
- Full right = only effect heard; full left = no effect

### Video Effects
- Always include an **Opacity** slider controlling mixing intensity
- Always include a **Blend Mode** selector — different blend modes create different visual results
- Tip: Select the Alpha blend mode and turn opacity all the way up to see only the effect output

### Presets
- Save all effect settings for quick reapplication
- Presets are snapshots, not live links — updating a preset does NOT affect previously applied instances
- Apply by selecting from the preset menu on any effect instance

### Favorites
- Click the heart icon on any effect to favorite it
- Favorites appear in a dedicated Favorites tab for quick access

### Effect Bypass Animation
- Right-click the "B" (bypass) icon to access animation options
- Allows effects to turn on/off synchronized to the beat

### Stacking / Effect Order
- Effects stack in sequence; each effect processes the output of the previous one
- Order matters significantly — changing order can drastically change the result
- Transform is an effect like any other, so it can be placed anywhere in the effect stack
- Transform presets are supported

### Renaming & Recoloring
- Right-click any effect instance to rename it or change its color for visual organization

### Effect Clips (Adjustment Layers)
- Effects can be added to an empty clip, creating an "Effect Clip"
- Similar to Photoshop/After Effects adjustment layers
- Apply effects to all clips playing in layers underneath
- Support MIDI/keyboard triggers, layer transitions, and duration assignment for Auto Pilot

### Copying Effects
- Right-click source → "Copy Effects"
- Right-click destination → "Paste Effects"
- Copies all settings, presets, and animations

### Third-Party Plugins
- **Video (FFGL)**: Copy .dll (Windows) or .bundle (Mac) to 'Extra effects' subfolder in user folder
- **Audio (VST)**: Similarly support custom folder scanning
- Restart required after adding plugins
- Custom scan folders can be configured
- 64-bit plugins required for Resolume 6+
- Resources: The Juice Bar and forum plugin lists

---

## 2. Video
**Source**: resolume.com/support/en/video (v7.19+)

### Supported File Formats
MOV, AVI, GIF, MP4, MPG, MPEG — on both Mac and Windows.

### Codec Compatibility
"If your system's video player can play the file, so can Resolume." Files playable in Windows Movies & TV or Apple QuickTime work in Resolume.

### Playback Infrastructure (3-tier)
1. **Native playback**: DXV, PhotoJPEG, GIF, and Apple ProRes (on PC) use Resolume's own engine
2. **OS handling**: Windows uses MediaFoundation; Mac uses AVFoundation
3. **Fallback**: FFMPEG processes unrecognized formats

### Transport Section Controls

#### Timeline
- **Playhead**: Drag the blue pointer for DJ-style scratching
- **In/Out Points**: Set clip boundaries with small pointers; hold Shift while dragging to move the range while maintaining its length
- **Time Display**: Click current time to toggle to remaining time view
- **Speed Slider**: Non-linear response — precision between 0-2x, rapid scaling toward 10x
- **Duration**: Set exact playback length; Resolume calculates the required speed automatically

#### BPM Sync Mode
- Synchronizes clip playback to global BPM
- Users specify beat span: 1, 2, 4, 8, 16, 32, 64, 128, etc.
- Speed modifier quantizes to: 0, 1/8, 1/4, 1/2, 1, 2, 4, 8, or 16x

#### Direction Options
- Forward play
- Backward play
- Pause

#### Playmode Settings

| Mode | Behavior |
|------|----------|
| **Loop** | Default; continuous repetition |
| **Ping Pong** | Alternates forward/backward to mask looping artifacts |
| **Random** | Jumps to random timeline points. **Interval** controls jump frequency (lower = more frequent). **Distance** controls jump range (lower = subtle glitchy; higher = abrupt cuts) |
| **Play Once and Eject** | One-shot playback, then ejects |
| **Play Once and Hold** | Retains final frame after playback; useful for logos/intros |

#### Playmode Away (Re-trigger Behavior)
- **Start from beginning** (default)
- **Pick-up**: Resume from previous playback position
- **Relative pick-up**: Match relative position of previously played clip

### Cue Points
- Click left section of cue button to set a point at current playhead location
- Blue = ready; White = keyboard shortcut assigned
- Click main button to jump to the cue point

### Beat Looper
- Available when BPM Sync is enabled
- Auto-loops selected beat ranges
- "Catch Up" option: when disabled, continues from expected playhead position; otherwise continues from current position

### Codec Recommendation
"Converting to DXV is always, always, always the answer" for any playback issues. Resolume respects FFMPEG licensing limitations.

---

## 3. Sources
**Source**: resolume.com/support/en/sources (v7.19, 7.7, 6)

### Overview
Sources are procedural content generators that work as alternatives to video clips. They generate visuals in real-time rather than playing back files.

### Loading Methods
- **Sources Panel**: Drag sources onto empty clips
- **Quick Search**: Double-click an empty clip handle and select from menu
- **Preview**: Double-click sources in the panel to preview them

### Properties
- Once placed, source properties appear in the **Clip tab**
- Each source has shared properties plus source-specific parameters
- Duration is adjustable (seconds or beats depending on playback mode)
- Sources work with Autopilot sequences and clip rendering

### Transform & Composite
Sources support all standard operations: scaling, positioning, blending, effects — same as video or stills.

### Favorites
Heart icon to favorite; accessible via Favorites tab.

### Video Router (Advanced)
- Takes the output of a single layer (or layers below the router) into another layer
- On Arena, groups can serve as inputs
- Preview window output is selectable as input
- Input options show sources by name; selection refers to the index of the selected item
- **Input Bypass/Solo toggles** and **Input Opacity** controls function only with specific layer or group selections
- These act as pre-fader/post-fader switches to prevent duplicate rendering

### Input Source Types
- **Capture Devices**: Hardware inputs like webcams (see Live Inputs)
- **Virtual Patch Cables**: NDI, Spout, and Syphon protocols for virtual video routing between applications

---

## 4. Blend Modes
**Source**: resolume.com/support/en/blend-modes (v7.22.5)

### Definition
Blend modes define how two layers of video are mixed together, controlling how pixels of one layer interact with pixels underneath.

### Application Contexts
- **Arena & Avenue**: Blending layers, effects, and transitions
- **Wire**: Video Mixer nodes, Transition nodes, and Texture Material

### General Modes

| Mode | Behavior |
|------|----------|
| **Alpha (Normal)** | Mixes based on opacity. Transparent pixels reveal underlying content; opaque pixels cover. Default for Wire's Video Mixer. Smooth edge blending but can produce dull gray tones midway through transitions. |
| **AlphaCut** | Hard cuts between signals with no blending. An "anti-blend mode." Useful for harsh transitions, strobing, and triggering ADD mode effects. |

### Darken Modes (all produce darker results)

| Mode | Behavior |
|------|----------|
| **Darken** | Compares brightness pixel-by-pixel, keeps whichever is darker. Good for textures, scratches, film grain. |
| **Multiply** | Multiplies RGB and alpha values. Result always darker unless one input is white. Black removes underlying pixel; white preserves it. Affects color intensity. |
| **Burn** | Aggressively darkens by boosting contrast and reducing brightness. More intense than Multiply. Creates inky, high-contrast shadows. Screen printing aesthetic. |

### Lighten Modes (all produce brighter results)

| Mode | Behavior |
|------|----------|
| **Lighten** | Keeps the brighter pixel from source and destination. Brightness-pass filter. Great for flares, explosions, blooms, neon. |
| **Screen** | Inverts both, multiplies, inverts result. Soft bright clean look with glow. Combines both layers (unlike Lighten which chooses). Works well with black backgrounds for ethereal overlays. |
| **Dodge (Color Dodge)** | Divides destination pixels by inverted source. Far more intense than Screen. Dramatic overexposed effects. Prone to whiteout. Good for light leaks, lightning, strobes. |

### Contrast Modes (combine darkening and lightening)

| Mode | Behavior |
|------|----------|
| **Overlay** | Multiply for pixels darker than 50% gray; Screen for brighter. High contrast. Good for mid-range content like skin tones. |
| **Soft Light** | Gentler version of Overlay. Softer contrast curve. Bright areas slightly lighter, dark slightly darker. |
| **Hard Light** | Reverse of Overlay — examines source brightness not base. Intense, aggressive. Excellent for patterns, strobes, bold mixes. |

### Comparative Modes (mathematical pixel comparisons)

| Mode | Behavior |
|------|----------|
| **Add** | Adds color values of both pixels. Brighter composites with intense highlights. **Arena & Avenue's default blend mode.** Risk of clipping with bright visuals. |
| **Subtract** | Removes source color value from destination. Darker image with unexpected color inversions. Unpredictable. |
| **Difference** | Subtracts brighter from darker. Identical pixels = black; opposite pixels = white. Psychedelic effects with movement. **Difference I** is inverted version. |
| **Exclusion** | Like Difference but softer, more washed-out. Subtle color inversion. Low contrast. "Negative Film" aesthetic. |

### Composite Modes (advanced layering with alpha, luminance, or masking)

| Mode | Behavior |
|------|----------|
| **Luma Key** | Uses brightness of source to determine visibility. Bright = visible; dark = transparent. **Luma Key I** is inverted. |
| **Luma is Alpha** | Converts luminance to alpha channel. Bright = opaque; dark = transparent. Turns B&W video into mask. |
| **Displace** | Does NOT blend visuals. Horizontally shifts pixels based on source layer color values. Displacement map / distortion effect. Creates ripples, warps, glitches. |
| **50Mask** | White = fully visible; black = fully transparent; gray = blend. Alternative to masking layers. Masks and content on same layer with independent blend modes. |
| **RGB** | Gradually removes Blue, Green, and Red channels. Progressively reveals layer as channels are removed. |

### Transition Modes
Various wipes, slides, and dissolves that animate layer transitions:
- Wipe Ellipse, Dissolve, Push Up, and directional variants

---

## 5. LUT (Lookup Tables)
**Source**: resolume.com/support/en/lut (v7.19+)

### Overview
LUTs are color correction/grading tools — "Instagram filters for your footage."

### Arena & Avenue
- Load via "load" button or drag .cube files directly onto parameters
- Treated as effects — have opacity slider and blend mode
- Blend mode allows experimental non-standard LUT usage

### Wire Nodes
1. **LUT 3D In**: Creates interactive input nodes matching Arena/Avenue LUT effect
2. **LUT 3D Resource**: Loads LUTs for non-interactive patches; supports multiple LUT collections
3. **LUT**: Processing node applying LUTs to textures or color collections; enables color grading on color arrays

### Technical Details
- **Only ".cube" format** supported
- Content must be in non-logarithmic color space
- Resolume ships default LUTs (copyright protected, cannot be redistributed)
- Additional free and commercial LUTs available

---

## 6. Live Inputs
**Source**: resolume.com/support/en/live-inputs (v6)

### Overview
Live inputs appear in the Sources tab. Can be dragged onto empty clip slots, triggered/ejected like regular clips, have effects applied, and be composited with other layers. **Audio capture is NOT supported.**

### Capture Devices
- Convert analog video (HDMI, SDI, composite) to digital
- External USB/Thunderbolt boxes or built-in components
- **Mac**: AV Foundation; **PC**: DirectShow

### Native Capture Support (Lowest Latency)

| Manufacturer | Supported |
|-------------|-----------|
| **Blackmagic** | Yes |
| **Datapath** | Yes |
| **AJA** | Yes |

- Native support bypasses CPU, enabling direct device-to-Resolume image transfer
- Latency: PCIe/Thunderbolt = 60-100ms at 1080p60; USB 3.0 = 80-120ms
- "Zero latency is a pipe dream"
- Multiple simultaneous devices supported (including all inputs from multi-input devices)
- Auto-detection of connection type, resolution, color space, and framerate

### Device-Specific Settings
- **All sources**: Deinterlacing
- **Blackmagic/AJA**: Color space and range specification
- **AJA only**: Genlock sync source and frame buffering configuration

### Troubleshooting
- Test with manufacturer's native software first
- Only one application can access a device simultaneously
- Mismatched capture settings prevent proper operation

### Magewell
- Works via DirectShow (PC) / AVFoundation (Mac)
- No unified SDK = no native Resolume integration currently

---

## 7. Syphon / Spout (Texture Sharing)
**Source**: resolume.com/support/en/syphonspout (v7.22, 6)

### Overview
Syphon (Mac) and Spout (PC) are texture sharing tools for routing video output between applications on the same machine.

### Input
- Always enabled — programs broadcasting automatically appear in Sources tab
- Add detected sources to decks like standard live inputs
- Unlimited simultaneous inputs

### Output
- Enable via Output Menu to broadcast main composition output immediately
- Compatible applications auto-detect the broadcast
- Identification: App Name = "Avenue" or "Arena"; Server Name = "Composition" (main) or "Screen 1" (Advanced Output)

### Advanced Output (Arena Only)
- Treats Syphon/Spout as separate physical screens
- Output warping before transmission
- Selective composition routing
- Independent physical screen outputs
- Customizable width and height

---

## 8. NDI Inputs and Outputs
**Source**: resolume.com/support/en/NDI_inputs_and_outputs (v7.19+)

### Overview
NDI enables network-based video transmission — "send video from a Mac to a PC and vice versa, over the Network." Like Syphon/Spout but works across networked computers without specialized hardware.

### Input
- Always enabled by default
- Sources appear in Sources tab alongside other live inputs
- Multiple simultaneous inputs (bandwidth-limited)
- Smartphone camera support via NewTek's NDI camera app

### Basic Output
- Enable via Output Menu; immediately broadcasts main composition output
- Output dimensions match composition specs

### Advanced Output (Arena Only)
- Treats NDI as separate physical screens
- Output warping before transmission
- Selective composition area routing
- Customizable width and height
- Bit-depth and color space selection for bandwidth optimization
- Optional alpha channel disabling
- Each Advanced Output screen = separate NDI source

### PTZ Control
NDI cameras with Pan-Tilt-Zoom can be controlled directly within Resolume.

### Bandwidth
- 60 FPS 1080p requires at least 150 Mbps bandwidth
- No hard-coded connection limits

### Known Issues
- **Network adapter selection**: NDI prioritizes WiFi when available. Disable wireless for optimal performance. NDI Access Manager provides workaround.
- **Discovery vs. Connection**: Sources may appear but show "red Offline mark" and "0x0 in size" due to IP mismatches. Fix by manual IP configuration.
- **Protocol compatibility**: Install NDI tools package; adjust Receive mode in NDI Access Manager Advanced tab.

---

## 9. Stills (Images)
**Source**: resolume.com/support/en/stills

### Supported Formats
.png, .jpg, .jpeg, .tiff, .tif

### Timing Modes
- **Timeline mode**: Duration in seconds, adjustable via +/- or direct input
- **BPM Sync mode**: Duration in beats, with /2 and x2 buttons for multiples
- Both modes support batch adjustment of multiple clips simultaneously

### Why Duration Matters for Stills
- Auto Pilot timing (controls when next action triggers)
- Animating parameters to Clip Position
- Arena: syncing stills to SMPTE timecode

### Image Sequences
- Save sequential images in a folder with consistent naming
- Drag folder into Resolume; auto-detected and treated as single video
- Performance is NOT as good as regular video
- Recommendation: convert to video first using Alley

### Memory & Loading
- Stills are "surprisingly resource intensive" despite being single frames
- **Deferred loading**: stills only consume RAM when triggered
- Potential hiccup when triggering high-res stills (especially 4K+)
- For fast VJ triggering: convert to short DXV-encoded movie files

### Other Uses
- Can function as masks
- Can serve as guides in Advanced Output

---

## 10. Transform
**Source**: resolume.com/support/en/transform

### Overview
Transform is automatically applied to every clip, layer, group, and composition. It controls position, scale, rotation, and anchor point.

### Parameters

| Parameter | Details |
|-----------|---------|
| **Position** | X and Y axes. Range depends on GPU: typically 16384 (16K) or 30720 (32K). |
| **Scale** | Proportional resize by default; expandable for independent width/height. |
| **Rotation** | Degrees around anchor point. Expandable for per-axis X, Y, Z rotation. Changing X or Y rotation disables the Transform Widget. |
| **Anchor Point** | Pivot point for rotation. "Think of your content as a postcard and the anchor point is the pin." |

### Transform Widget
- Direct on-monitor manipulation interface for move, rotate, scale
- Right-click context menu with reset option
- **Disabled** when X/Y rotation or any Anchor Point parameter is manually changed
- Not available on Crossfader and Advanced Output monitors

### Multiple Transforms
- Can apply multiple Transform instances via Effects list
- Each instance has independent opacity and blend modes
- The very first Transform in each panel CANNOT be deleted

### Transform Presets
Frequently-used configurations can be saved as presets.

### Slice Transform (Arena Only)
- Access via View > Show Slices or Effects panel
- Enables content placement across multiple screens/slices

#### Scaling Modes

| Mode | Behavior |
|------|----------|
| **Fill** | Crops edges to fill completely |
| **Fit** | Shows entire content, possible empty pixels |
| **Stretch** | Warps content to match slice shape exactly |
| **Mask / Invert Mask** | Uses slice boundary to show/hide portions |

#### Slice Controls
- **B toggle**: Hide content for specific slice
- **S button**: Solo individual slice
- **X button**: Delete slice assignment
- **Pacman icon**: Cycle through 4 orientations (regular, horizontal mirror, vertical mirror, both)

#### Missing Slices
- Unlocated slices highlighted in red; remaining slices function normally
- Undo restores accidentally deleted slices

---

## 11. Clip Renderer
**Source**: resolume.com/support/en/clip-renderer (v7.21, 7.2.0)

### Overview
Offline rendering of individual clips for creating loops, sharing work, or file conversion. NOT the same as recording.

### Clip Renderer vs. Recording

| Feature | Clip Renderer | Recording |
|---------|--------------|-----------|
| Scope | Individual clips | Entire output |
| Quality | No frame drops, exact length | May capture frame drops |
| Timing | Offline | Real-time during playback |
| Trigger | Right-click or drag to queue | Manual button |

### Rendering Process
1. Right-click a clip → "Render to File", OR drag clip to queue panel
2. Drag files from File Browser for quick format conversion
3. Queue panel shows renders with folder access and removal options
4. Post-render: drag rendered clips onto composition, double-click to preview
5. Folder icon opens render directory; cross icon removes from queue (does not delete file)

### Technical Specs

#### Resolution & Framerate
- All clips render at **composition resolution**
- Composition settings control framerate (default: auto)
- Sources: 30 FPS when framerate = auto
- Files: original framerate with default settings
- Custom framerate applies to both sources and files

#### Speed & Playback
- Renders at individual clip speed
- Speed value of 0 treated as 1
- Group speed and composition speed are **ignored**
- Only BPM Sync or Timeline transport types supported
- BPM Sync clips render using current global Tempo

### Limitations
- No live inputs (capture devices, NDI streams)
- No isolated audio rendering
- No Composition/External FFT (Clip FFT available as alternative)
- Feedback and Video Router sources incompatible

### Codec & Format Options

| Setting | Options |
|---------|---------|
| **Codec** | DXV (recommended for Resolume), MotionJPEG, ProRes (for external editing) |
| **Quality** | High or Normal (high recommended for gradients) |
| **Alpha** | Include or exclude (affects file size and transparency) |
| **Audio sample rate** | Adjustable |
| **Audio bit depth** | Configurable |

---

## 12. Recording
**Source**: resolume.com/support/en/recording

### Overview
Records composition output or advanced output to disk during live playback. Can auto-import recordings into Avenue/Arena without interrupting video output.

### Setup
- Configure recording preferences before starting
- Default location: Documents/Resolume Arena/Recorded (customizable in Preferences)

### Record Panel Components

#### Primary Controls
- **Large record button**: Starts/stops recording based on Start and Duration settings
- **Source selector**: Choose what to record — entire composition, specific layer, group, Crossfader, or Advanced Output screens
- **Media toggle**: Select video, audio, or both

#### Recording Settings
- **"After Recording" option**: Nothing, or auto-placement on empty clip
- **Preset selection** with customization via cogwheel icon
- **Default preset**: QuickTime format, DXV3 codec at normal quality; audio in uncompressed WAV

#### Timing Control
- Start and Duration settings
- Manual control or automation via time or BPM triggering

### Access
View → Show Recordings (if Record Panel not visible)

### Performance
- Maintains functionality during active mixing and effect application
- Supports 4K resolution and beyond (hardware dependent)
- Does NOT interrupt mixing or video output — simultaneous creative work during capture
