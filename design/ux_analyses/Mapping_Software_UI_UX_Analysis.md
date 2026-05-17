# Projection Mapping / Show Control UI/UX Deep-Dive: MadMapper, Millumin, HeavyM

## APPLICATION 1: MADMAPPER 6

### 1. Visual Design Language

**Color Palette**: Dark charcoal-gray workspace (#2a2a2a-#3a3a3a), near-black canvas. Blue for selection, green/yellow for active states, white (#e0e0e0) for text. Industrial-professional. Surface wireframes in white/cyan.

**Typography**: System sans-serif (San Francisco on macOS), small sizes. Dense packing. Assumes learned interface.

### 2. Layout Architecture

**Four-Zone Layout**:
1. **Media Panel (left)** -- Thumbnails of imported media, materials, generators. v6 made this always-visible (major workflow win)
2. **Input View (center-left)** -- Source texture with surface boundaries. UV/texture coordinate editing
3. **Output Preview (center-right)** -- Projector's view. Primary spatial editing canvas
4. **Inspector/Properties (right)** -- Context-sensitive properties

**Timeline (v6)**: Bottom-docked. Multi-track with Bezier/Catmull-Rom/linear curves. Montage tracks for NLE-style editing. Audio tracks with FFT visualization.

**Conductor View**: Bird's-eye overview of entire show. All timelines, markers, clips for navigation.

### 3. Core Interaction Patterns

- **Surface Creation**: Click to add quad. Four corner handles for perspective. Grid warping with N x M mesh subdivision. Bezier curves for contours
- **Content Assignment**: Drag from media pool onto surface. Auto-maps to UV space
- **Warping**: Three levels: 4-corner perspective, grid mesh warping, 3D camera+structured light scanning (Spatial Scanner)
- **DMX/LED**: Art-Net, sACN. Visual LED layout editor. MiniMad hardware for standalone installations

### 4-6. Hierarchy, Onboarding, Performance

Spatial information (output preview) prioritized over temporal (timeline). Deep property editing buried in nested panels. Steep learning curve, no in-app wizard. Conductor view provides panic-free show overview. 16K resolution support.

### 7-9. Strengths / Weaknesses / Innovations

**Strengths**: Best spatial calibration (Spatial Scanner), excellent LED pixel mapping, v6 timeline + Conductor, Montage tracks, MiniMad hardware, active update cycle.

**Weaknesses**: Steep learning curve, deep DMX/LED config, occasional stability issues under load, not primarily a VJ tool, media management at scale.

**Innovations**: Spatial Scanner (camera-based structured light calibration), MiniMad (dedicated installation hardware), Conductor View (show-level overview), Montage Tracks (NLE in a mapping tool).

### 10. Rating

| Category | Score |
|----------|-------|
| Visual Design | 7/10 |
| Layout Efficiency | 7.5/10 |
| Interaction Design | 8/10 |
| Information Hierarchy | 7/10 |
| Onboarding | 4/10 |
| Performance UX | 7.5/10 |
| **Overall** | **7.5/10** |

---

## APPLICATION 2: MILLUMIN V5

### 1. Visual Design Language

**Color Palette**: Medium-dark gray (#3c3c3c) panels, lighter content areas. Muted accents -- blue for selection, subtle green for active. Warmer and softer than competitors. "Lighting designer's console" feel.

**Typography**: Clean sans-serif, slightly larger. Good whitespace. Dashboard cells use medium-weight text readable during operation.

### 2. Layout Architecture

**Three-Mode Architecture**:
1. **Dashboard Mode**: Grid sequencer. Rows = layers, columns = cues/moments. Click column to trigger. Simple, powerful, theater-optimized
2. **Timeline Mode**: Linear multi-track with keyframing, segments. For precisely timed sequences
3. **Workspace/Canvas**: Spatial composition with 4 tools (move, mapping, mask, brush)

**Properties**: Right panel, context-sensitive. Properties can be promoted to timeline by clicking labels.

**Dual-Mode Integration**: Timelines embeddable inside dashboard columns. Show mostly cue-triggered (dashboard) with specific precisely-timed cues (timeline).

### 3. Core Interaction Patterns

- **Dashboard**: Click column to launch. Follow actions, transitions, timing. Loop-mode icons visible when zoomed in
- **Timeline**: Drag media onto layers, split segments, set transitions, keyframe
- **Mapping**: 4-corner mapping (M key), slice editor (CMD+J), masks, constraints (v5) for pixel-size locking
- **Protocol Integration**: MIDI, OSC, Art-Net, DMX, timecode, gamepad, Arduino, Kinect, LeapMotion, ATEM, NDI audio

### 4-6. Hierarchy, Onboarding, Performance

Temporal organization prioritized over spatial. Dashboard's column structure: left-to-right = show progression. Incredibly clear mental model. Better onboarding than MadMapper with 4-step tutorial. Built for theatrical reliability -- operators can only advance/retreat through predetermined states.

### 7-9. Strengths / Weaknesses / Innovations

**Strengths**: Dashboard metaphor (genius for cue-based shows), timeline + dashboard integration, deepest protocol integration, After Effects Bridge, data tracks for synchronized lighting, scripting with parameters (v5), excellent documentation.

**Weaknesses**: macOS only, expensive, not for improvisational VJ, spatial mapping less advanced than MadMapper, can struggle on lower hardware.

**Innovations**: Dashboard + Timeline Hybrid, Data Tracks (synchronized DMX from timeline), After Effects Bridge, Script Parameters (v5), Negative Cue Values.

### 10. Rating

| Category | Score |
|----------|-------|
| Visual Design | 7/10 |
| Layout Efficiency | 8.5/10 |
| Interaction Design | 8.5/10 |
| Information Hierarchy | 9/10 |
| Onboarding | 6.5/10 |
| Performance UX | 8/10 |
| **Overall** | **8/10** |

---

## APPLICATION 3: HEAVYM

### 1. Visual Design Language

**Color Palette**: Dark theme with color-coded sections by workflow role. Shape groups assignable from 30-color palette for visual identity on canvas. Modern and accessible.

**Typography**: Clean modern sans-serif, slightly larger with generous spacing. Complete words, not abbreviations.

**Content Panel**: Visual thumbnails showing what each effect looks like. 3D Materials with realistic previews. "See before you apply" philosophy.

### 2. Layout Architecture

**Five-Zone Layout**: Canvas (center), Top Toolbar, Mapping Panels (left: layers + properties), Content Panels (right: effects + sources), Sequencer (bottom).

**Shape Type System**: Groups contain Shapes. Shapes have three types: Faces (generative effects), Players (video/images/text), Masks. Elegant conceptual model.

### 3. Core Interaction Patterns

- **Wizard-Style Workflow**: (1) Draw shapes, (2) Assign effects/media, (3) Calibrate, (4) Sequence, (5) Perform
- **Shape Drawing**: Ready-made shapes or freehand Bezier. Grid snapping
- **Effect Browser**: Visual grid of thumbnails. 1,000+ combinations. Effects auto-adapt to shape
- **Audio Reactivity**: Built-in beat detection. Effects pulse with music. Ableton Link support
- **Sequencer**: Scenes with unique shapes/effects/media. Configurable transitions. Manual, scheduled, or MIDI/OSC/DMX triggered

### 4-6. Hierarchy, Onboarding, Performance

Workflow-driven: Shapes (left) > Content (right) > Sequencing (bottom). Group color system adds secondary hierarchy. Best onboarding in the category -- designed for zero-experience users. "A few minutes to get started." Stable for live performance but not designed for arena scale.

### 7-9. Strengths / Weaknesses / Innovations

**Strengths**: Best onboarding of any mapping software, visual effect library (see before apply), 3D Materials, sound analysis + Ableton Link, group color system, shape type system, cross-platform, approachable pricing.

**Weaknesses**: Limited precision for complex geometry, not suited for large-scale, simpler sequencer, no scripting, smaller community, no user-created shaders, less protocol integration.

**Innovations**: 3D Materials on 2D shapes, Shape Type Taxonomy (Face/Player/Mask), Visual Effect Browser, integrated beat reactivity as first-class feature.

### 10. Rating

| Category | Score |
|----------|-------|
| Visual Design | 7.5/10 |
| Layout Efficiency | 7/10 |
| Interaction Design | 8/10 |
| Information Hierarchy | 7.5/10 |
| Onboarding | 9.5/10 |
| Performance UX | 7/10 |
| **Overall** | **7.5/10** |

---

## Key Takeaways for Audio-DNA

- **Millumin's dashboard grid** (columns = states, rows = layers) is directly analogous to Audio-DNA's deck system
- **HeavyM's visual effect browser** (see the effect before applying) is worth studying for FX browser design
- **MadMapper's Conductor view** could inspire a composition-level performance overview
- **HeavyM's shape type system** parallels Audio-DNA's layer types (Opaque/Transparent/FX Only/Mask)
- **All three validate** dark themes with functional color accents as the standard for live visual tools
