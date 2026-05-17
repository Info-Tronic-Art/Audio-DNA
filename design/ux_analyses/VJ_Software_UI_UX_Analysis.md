# VJ Software UI/UX Deep-Dive: VDMX6 and ArKaos GrandVJ XT

## APPLICATION 1: VDMX6 (Vidvox)

### 1. Visual Design Language

**Color Palette**: Dark interface base with contextual color coding. Yellow outlines for selected items (customizable), light green for active data receivers, bright blue for hardware learn mode, red for drag feedback. Utilitarian, closer to a development environment than a polished consumer app.

**Typography**: System-standard macOS fonts at small sizes. Functional and dense, not decorative.

**Iconography**: Minimal custom iconography. Relies on text labels, dropdown menus, and standard macOS widgets. Pragmatic: function over form.

**Information Density**: Extremely high. Every slider can show its value, data receivers, senders, math expressions, envelope settings. A single element can expand into a multi-section inspector.

### 2. Layout Architecture

**Modular/Floating Panel System**: The defining feature. No fixed layout -- collection of plugin windows arranged freely. Each plugin (Media Bin, Preview, Clock, Audio Analysis, Control Surface, LFO, Step Sequencer) in its own window. Windows support tabbed organization, snap together, move as connected groups.

**Custom Interface Building**: Users start from templates (Simple Player, 4 Layer VJ Starter) then add/remove/rearrange. Control Surface plugin allows building completely custom UI layouts with sliders, buttons, color wheels.

**Workspace Save/Recall**: Templates stored in `~/Library/Application Support/VDMX/templates`. Shareable. Store window arrangement and plugins, but NOT media files.

### 3. Core Interaction Patterns

- **Media Bin**: Drag files or folders into bins. Click to trigger on assigned layer. Keyboard/MIDI/OSC triggering
- **Layer Controls**: One source per layer + FX chain + composition. Opacity, blend mode, position/size. Layer Groups for hierarchical composition
- **FX Management**: ISF/GLSL, CoreImage, FreeFrame/FreeFrameGL, Quartz Composer. Per-effect on/off, wet/dry, composition mode
- **Data Source Connections**: VDMX's most sophisticated system. Every control supports multiple simultaneous receivers (MIDI, OSC, DMX, internal). Float receivers support math expression chains, envelope scaling, endless-mode for encoders. Hardware Learn Mode (Cmd-L). Senders with soft-takeover modes
- **ISF Integration**: First-class citizen. ~200 ISF files shipped. Auto-generated UI from shader JSON metadata

### 4-6. Hierarchy, Onboarding, Performance

Flat hierarchy -- everything is a plugin window at the same level. Workspace Inspector acts as master index. Weak onboarding: no interactive tutorial, no guided tour. Performance-ready with fullscreen multi-display, Master Blackout, Kill All, Metal rendering, HAP codec.

### 7-9. Strengths / Weaknesses / Innovations

**Strengths**: Unmatched modularity, sophisticated data routing with math expressions, ISF ecosystem, protocol coverage (MIDI/OSC/DMX/Syphon/NDI/Ableton Link/HID), layer groups.

**Weaknesses**: macOS only, steep learning curve, dated visual design, no fixed layout option, FX chain per-layer only (no per-clip, no global).

**Innovations**: Control Surface plugin (in-app UI construction), data receiver math expressions, Video Tracking plugin (ML body/face/hand), OCR plugin, ISF as open standard.

### 10. Rating

| Category | Score |
|----------|-------|
| Visual Design | 4/10 |
| Layout Efficiency | 7/10 |
| Interaction Design | 8/10 |
| Information Hierarchy | 5/10 |
| Onboarding | 3/10 |
| Performance UX | 7/10 |
| **Overall** | **6/10** |

---

## APPLICATION 2: ArKaos GrandVJ XT

### 1. Visual Design Language

**Color Palette**: Dark charcoal/black (#1a1a1a-#2a2a2a) with orange accent for selection/active states. Cell parameters in blue, layer parameters in orange. Active bank = red indicator dot. Restrained palette.

**Typography**: Clean sans-serif at readable sizes. More generous than many VJ apps. Section headers (Visual, Effect, Mixing, Position/Size) use bold on dark panel headers.

**Grid-Centric Design**: Cell matrix is the visual heart. Cells display thumbnail previews with keyboard shortcut overlay. Structured, organized feel.

### 2. Layout Architecture

**Fixed Three-Panel Layout**: Left = Browser (6 tabs: Files, Effects, Transitions, Sources, Visual Library, Mappings). Center = Master Preview + Cell Matrix. Right/Bottom = Parameter Panels (4 tabs: Visual, Effect, Mixing, Position/Size). Bottom toolbar = mode switches, kill/blackout, fullscreen, audio meters, GPU/CPU monitor.

**Mixer Mode**: Adds Layer Element panel (16 layers stacked vertically with transparency sliders, A/B deck assignment, play/pause/stop) and Mixer Element panel (A/B crossfader, transition controls).

**Simplicity Philosophy**: One opinionated layout. Cannot rearrange panels. Zero setup time. Every tutorial applies to every user.

### 3. Core Interaction Patterns

- **Keyboard Triggering**: Synth mode = press key, cell plays while held (unless Latch). Mixer mode = triggering copies cell to layer. Matrix resizable to match any controller
- **Effects**: Drag-drop from browser. One effect per cell with up to 4 params. No FX chain/stacking
- **Layer Mixing**: 16 layers bottom-to-top. Per-layer: A/B deck buttons, transparency, play/pause/stop. Crossfader between decks with 10 transition presets
- **MIDI/DMX**: Learn mode (toggle, click param, move controller). Pre-built templates for popular controllers. Bidirectional feedback

### 4-6. Hierarchy, Onboarding, Performance

Clear shallow hierarchy: Two modes (Synth/Mixer) > Cell matrix > Parameter tabs. Good onboarding -- fixed layout, Help Box with hover text, visible keyboard shortcuts. Built for reliability: Kill All, Master Blackout, GPU/CPU monitoring, fixed layout means nothing gets lost.

### 7-9. Strengths / Weaknesses / Innovations

**Strengths**: Immediate playability, clean visual hierarchy, dual-mode architecture, 16-layer mixing, strong controller support, A/B deck with crossfader, affordable price.

**Weaknesses**: Single effect per cell (biggest limitation), no per-layer effects in synth mode, fixed layout, no shader extensibility, limited audio reactivity, dated visual design, basic source browser, no node patching.

**Innovations**: Synth mode stacking (musical metaphor), cell priority system, dual bank architecture, parameter copy modes, Kling-Net protocol, integrated LEDMapper.

### 10. Rating

| Category | Score |
|----------|-------|
| Visual Design | 5/10 |
| Layout Efficiency | 8/10 |
| Interaction Design | 7/10 |
| Information Hierarchy | 8/10 |
| Onboarding | 7/10 |
| Performance UX | 8/10 |
| **Overall** | **7/10** |

---

## Key Takeaways for Audio-DNA

- **GrandVJ validates** the cell/bank/keyboard-triggering paradigm as immediately intuitive
- **VDMX validates** that deep parameter routing and shader extensibility are what power users demand
- **Audio-DNA's architecture** already synthesizes the best of both: deck/layer/clip with per-clip and per-layer FX chains, signal routing per parameter, GLSL effects, keyboard bindings
- Main gaps to close: A/B deck crossfader with transitions (vs GrandVJ), data-source math expressions and multi-receiver-per-parameter (vs VDMX)
