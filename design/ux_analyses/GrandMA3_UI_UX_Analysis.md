# Professional Lighting Control UI/UX Deep-Dive: GrandMA3

## GrandMA3 (MA Lighting)

### 1. Visual Design Language

**Tile-Based Workspace**: Every screen divided into a reconfigurable grid of rectangular tiles. Each tile hosts any window type: fixture sheet, sequence sheet, layout view, 3D viewer, color picker, executor bar, command line feedback. Full-size console: 3 integrated touchscreens + 2 external monitors. onPC: up to 6 virtual screens. Most flexible UI architecture in professional entertainment control.

**Color Coding**: Systematic color language. Yellow = currently selected fixtures. Orange = active selection set. Green = inverted attributes. Fixture types use configurable color backgrounds. Attributes use feature-based coloring: Dimmer = white, Color = actual output color, Position = green, Gobo = yellow, Beam = cyan. Deeply consistent and learnable.

**Command Line Aesthetic**: Bottom of primary screen, always visible. Monospaced font with syntax highlighting: keywords white, object references cyan, values yellow, errors red. Separate scrolling feedback window with timestamps. Borrowed from Unix terminal design.

### 2. Layout Architecture

**Fixture Sheet**: Spreadsheet of all patched fixtures and current attribute values. Columns = attributes, Rows = fixtures. Cells update in real-time. Color cells show actual output color. Sortable by selection order, ID, or type.

**Sequence/Cue Sheet**: Grid showing all cues. Columns: cue number, name, fade times, trigger type. Color coding: white = hard value, blue = tracked, magenta = blocked.

**3D Viewer**: Built-in stage visualization with GDTF fixture profiles (accurate 3D models). Real-time beam rendering. Rotation, zoom, pan, click-to-select beams.

**Executor Pages**: On-screen faders/buttons mapped to physical console faders. Executor pages for banking. Virtual executors extend physical surface.

### 3. Core Interaction Patterns

**Dual-Input Paradigm (Command Line + GUI)**: GrandMA3's deepest design principle. Every GUI action translates to a command. Power users type: `Fixture 1 Thru 10 At 50`. Keywords abbreviatable: `Fix 1 T 10 A 50`. Tab completion, history, macros. Both inputs fully equivalent -- unique in lighting control.

**Fixture Selection**: Click in fixture sheet, click beams in 3D viewer, lasso in layout views, or type numbers. Selection Grid for structured patterns. MAtricks for effect generation across groups.

**Pickers**: Color (HSB/wheel/CIE/swatch), Gobo (visual thumbnails from GDTF), Position (2D grid drag/encoder wheels/direct value).

**Multi-User Collaboration**: Multiple consoles share show file via MA-Net3. Role-based permissions (admin/programmer/operator). Real-time change propagation. User profiles with individual preferences.

### 4-6. Hierarchy, Onboarding, Performance

Operator-defined hierarchy (tile system). Default: command feedback + fixture sheet (primary) > sequence/cue sheet (secondary) > preset pools/layout/3D (tertiary). Executor bar always visible. Steepest learning curve in entertainment control -- MA University offers multi-day courses. Free onPC software for practice. Built for extreme pressure: muscle-memory command line, view recall, session redundancy, thousands of parameters at 60Hz.

### 7-9. Strengths / Weaknesses / Innovations

**Strengths**: Unmatched workspace customization, command-line + GUI dual-input, real-time multi-user collaboration, systematic color coding, Phaser engine with MAtricks (most powerful effect generation in lighting), GDTF integration, free onPC, cue tracking, view recall.

**Weaknesses**: Steepest learning curve in industry (months to years), some GrandMA2 features still missing, 3D viewer not competitive with dedicated previs, patch screen friction, prohibitive console cost ($40K-$130K+), poor configurations hurt users.

**Innovations**: Phaser Engine + MAtricks + Selection Grid (generative effect system), tile-based multi-screen workspace, command-line/GUI symmetry, session-based multi-user collaboration, data pools and world filters, GDTF standard adoption.

### 10. Rating

| Category | Score |
|----------|-------|
| Visual Design | 7/10 |
| Layout Efficiency | 9.5/10 |
| Interaction Design | 9/10 |
| Information Hierarchy | 8.5/10 |
| Onboarding | 4/10 |
| Performance UX | 9/10 |
| **Overall** | **8/10** |

---

## Key Takeaways for Audio-DNA

- **Tile-based workspace** allows every operator to build their ideal interface -- consider offering layout presets with customization
- **Dual-input paradigm** (GUI for discovery, command line for speed) serves both beginners and experts simultaneously
- **Systematic color coding** across all views reduces cognitive load under performance pressure
- **Session redundancy** and multi-user support are aspirational features for professional VJ tools
- **View recall** (instant workspace switching per show section) is directly applicable to VJ preset/scene management
