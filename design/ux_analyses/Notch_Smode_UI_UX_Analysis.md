# Real-Time Compositor UI/UX Deep-Dive: Notch and Smode

## APPLICATION 1: NOTCH (notch.one)

### 1. Visual Design Language

**Color Palette**: Dark-themed default (light theme available). Deep charcoal/near-black for node graph canvas and viewport. Lighter gray for panels. Accent colors for Position/Rotation/Scale (red/green/blue for X/Y/Z). Material and Video nodes display inline thumbnails (toggleable).

**Node Graph Aesthetic**: Rectangular blocks with input ports (left), output ports (right), and special parent connector (upper left for hierarchy). v1.0 brought complete visual redesign with updated shapes, better legibility, cleaner routing. Node List groups by category with search.

**3D Viewport**: GPU-powered real-time rendering via NURA architecture (rasterization, ray tracing, path tracing unified). Standard DCC conventions with grid, orbit, gizmos.

**Timeline**: Segments with mini-curves for in-place keyframe editing. Bezier, Linear, TCB, Stepped interpolation. Curve Editor with overlaid multi-curve view.

### 2. Layout Architecture

**Triple Layout**: Viewport (3D/2D preview) + Nodegraph (node canvas) + Timeline/Curve Editor.

**Property Panels**: Inspector for node parameter editing. Properties for object-level attributes. v1.0 Resource Inspector: pre-import panel for previewing, rotating, scaling, flipping, trimming 3D assets before placing in graph.

**3D+2D Coexistence**: Native through node architecture. 80+ image effect nodes for 2D compositing. Full PBR 3D engine. Video-to-particles-to-cloners-to-post without conversion nodes.

**Supporting Panels**: Asset Browser, Material Browser, Scopes, Log, Profiler (per-node timing overlaid on graph), Render Queue. Modular docking system.

### 3. Core Interaction Patterns

- **Node Creation**: Searchable categorized Node List. v1.0 Floating Search at cursor position. Node Insertion splices into existing connections. Node Removal preserves upstream/downstream. Embeds for grouping. Auto-Alignment
- **3D Manipulation**: Alt+Mouse for orbit. Placement Tool for dropping objects onto surfaces
- **Keyframing**: Any property keyframeable. Transform Tool for re-timing. Mini-curves for quick editing
- **External Control**: Parameters exposable to media servers. OSC, MIDI, ArtNet paths

### 4-6. Hierarchy, Onboarding, Performance

Hierarchy: Nodegraph (spatial) > Timeline (temporal) > Inspector (parametric). Profiler overlay for bottleneck identification. Onboarding via Notch Academy and structured courses. Node abstraction is higher-level than TouchDesigner, easier to learn. Built for live events (Dua Lipa, XR productions). NURA is entirely GPU-powered. Exports to 14 media server platforms.

### 7-9. Strengths / Weaknesses / Innovations

**Strengths**: Perfectly calibrated node abstraction level, NURA unified rendering, Resource Inspector, Floating Search + Node Insertion, 14 media server integrations, Profiler overlay, quality Curve Editor, GPU particles at scale.

**Weaknesses**: Windows-only, no modeling tools, small community, expensive ($279-$3,780/year), closed ecosystem, Root node confusion, heavy GPU requirements (RTX 2080 min).

**Innovations**: NURA unified rendering (4 modes switchable in real-time), Resource Inspector (pre-import pipeline), media server native integration, Node Insertion on connection, "anything-to-anything" connectivity.

### 10. Rating

| Category | Score |
|----------|-------|
| Visual Design | 7.5/10 |
| Layout Efficiency | 8/10 |
| Interaction Design | 8.5/10 |
| Information Hierarchy | 7.5/10 |
| Onboarding | 6.5/10 |
| Performance UX | 9/10 |
| **Overall** | **7.8/10** |

---

## APPLICATION 2: SMODE (smode.io)

### 1. Visual Design Language

**Aesthetic**: After Effects-inspired layer compositor. "No scripts, no graph-based program, no need to know shaders' magic -- it's all about layers, modifiers and masks." Dark theme default. Medium-dark gray panels. Utilitarian but clean.

### 2. Layout Architecture

**Default 6-Panel Workspace**: Left = Media Directories + Devices + Documentation. Center = Viewport. Right = Element Tree (hierarchical layer browser). Bottom-Right = Parameters Editor. Bottom = Timeline Editor.

**Element Tree**: Core organizational metaphor. Compositions stack layers. Each layer contains Generators, Renderers, Modifiers, Masks. Contextual right-click menus show relevant options per element type.

**Workspace Customization (v9.3+)**: Complete panel rearrangement with saveable presets. Export/import configurations.

### 3. Core Interaction Patterns

- **Layer Creation**: Right-click context menu, Instantiator (Ctrl+Space type-ahead search -- elegant universal command palette), or drag-drop import
- **Effects as Modifiers**: Non-destructive stack. Color, Distort, Stylize, Blur, Utility, AI (Stream Diffusion, upscale, denoise)
- **Keyframing**: Drag parameters into Timeline Editor
- **Integration**: Notch blocks, TouchDesigner .tox, ISF, ShaderToy, Substance, Unreal Engine connections
- **Live Show Control**: Cue list, MIDI/OSC, StreamDeck, Art-Net/sACN, PJLink, ATEM, LTC/MTC timecode, tracking, password-protected UI lock

### 4-6. Hierarchy, Onboarding, Performance

Layer-centric hierarchy: Element Tree primary, everything references it. Modifier-stack model (Generator > Modifier > Mask > Renderer) provides clear pipeline. Good onboarding for AE users. Free Community Edition (1080p, no watermark). Show control + media server + compositor in one app.

### 7-9. Strengths / Weaknesses / Innovations

**Strengths**: AE familiarity (shortest learning curve for largest user pool), layer-based accessibility, modifier-stack architecture, free Community Edition, affordable pro tier (300 EUR/year vs Notch $1,399+), deep integration ecosystem, all-in-one show control, Instantiator, real-time 2D/3D, AI modifiers.

**Weaknesses**: Windows-only, smaller community, documentation gaps, less customizable than node tools, less established brand, Studio tier pricing jump (300 to 3,000 EUR), limited offline docs, 3D capabilities lag behind Notch.

**Innovations**: AE-to-Live pipeline, Instantiator (Ctrl+Space), modifier stack with masks in real-time, third-party layer integration (Notch/TD/ISF/ShaderToy/Unreal), AI in modifier stack, unified compositor+media server+show controller, password-protected UI lock.

### 10. Rating

| Category | Score |
|----------|-------|
| Visual Design | 6.5/10 |
| Layout Efficiency | 8/10 |
| Interaction Design | 7.5/10 |
| Information Hierarchy | 8/10 |
| Onboarding | 7.5/10 |
| Performance UX | 7.5/10 |
| **Overall** | **7.5/10** |

---

## Key Takeaways for Audio-DNA

- **Notch's Resource Inspector** (pre-import pipeline) could inspire a media preparation workflow
- **Smode's Instantiator** (Ctrl+Space universal search/create) is worth studying for quick FX/source addition
- **Smode's AE-familiar layer model** validates that layer-based approaches are more accessible than node graphs for most users
- **Notch's Profiler overlay** (per-node timing on the graph) demonstrates excellent performance debugging UX
