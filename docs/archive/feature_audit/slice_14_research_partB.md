# Slice 14 — Research Documents Part B — Feature Audit

Scope: UNIFIED_BUILD_PLAN, resolume*/archaos* integration specs, OurAppFeatures_1, SOURCES_procedural_generators, VIDEO_*, IMPL_*, UI_UX_* analysis docs. Exclusions: research/mockups/ (directory not opened per instructions).

Listing of `research/Resolume/` (markdown contents summarized only; non-.md files left unread):
- `BestPractices.md`
- `Content_Section.md`
- `ControllingResolume.md`
- `OutputSystem.md`
- `RESOLUME_TECHNICAL_REFERENCE.md`
- `Resolume Arena` (directory)
- `Resolume Wire` (directory)
- `Wire_Research.md`
- `Workflow_Reference.md`

---

## UNIFIED_BUILD_PLAN.md — phase features

Authoritative phase-by-phase roadmap for P13–P25, with phase statuses tracked in a table. All phases P13–P25 marked COMPLETE in the master table (`UNIFIED_BUILD_PLAN.md:29-43`).

### Common infrastructure rules (UNIFIED_BUILD_PLAN.md:46-197)
- 3-step effect process: GLSL shader in `EmbeddedShaders.h` → register in `EffectLibrary.cpp` → compile in `Renderer.cpp::compileEffectShaders()` (`:52-82`).
- 3-step source process: shader in `EmbeddedShaders.h` → register in `SourceRegistry.cpp` → compile in `Renderer.cpp::compileSourceShaders()` (`:86-130`).
- Self-validation checklist: Build, CTest (113+ tests), launch, new content visible, 60fps regression check (`:148-160`).
- Test files per phase: P13.5 adds compositor tests; P14/P15 add `test_effect_registration.cpp`; P16 adds feedback serialization; P18 adds `test_audio_effects.cpp`; P20 adds `test_ffgl_host.cpp`; P21 adds `test_binding_modes.cpp`; P23 adds `test_genre_detection.cpp` (`:178-187`).

### Phase 13 — Effect Infrastructure (COMPLETE) (`:200-310`)
- P13.1 Per-effect dry/wet mix with `effect_drywet_frag` compositing shader, `Effect::dryWet` field, first slider in every effect (`:219-237`).
- P13.2 Temporal buffer / previous frame access via `EffectChain::getPreviousFrameTexture()`, `u_prev_frame` uniform, `EffectDef::temporal = true` (`:239-243`).
- P13.3 LUT loading system (`LUTLoader.h/cpp`, `.cube` parser, GL_TEXTURE_3D, `TextureManager::loadLUT()`) (`:245-250`).
- P13.4 Shared GLSL utility functions: `glsl_noise_functions`, `glsl_util_functions`, `glsl_sdf_functions` including `hash`, `hash2`, `snoise`, `fbm`, `rotateUV`, `hueRotate`, `sdCircle`, `sdBox`, `sdHexagon`, `sdStar` (`:252-286`).
- P13.5 New browser categories: FX adds Time (teal), Composite (pink), Audio (gold); Sources adds Math, 3D, Organic, Pattern, Particle, Utility, Lighting, Simulation, Text, Routing (`:288-307`).

### Phase 13.5 — Core Bug Fixes & Render Optimization (COMPLETE) (`:312-384`)
- P13.5.1 Per-clip effect chain rendering (run clip's own effect chain before layer compositing) (`:331-336`).
- P13.5.2 FXOnly layer type (apply clip's effects to accumulator) (`:338-341`).
- P13.5.3 Mask layer type (greyscale clip → alpha mask on accumulator) (`:343-346`).
- P13.5.4 REMOVED — transitions moved to P14 (`:348-350`).
- P13.5.5 Layer transform application (positionX/Y, layerScale, layerRotation, layerAnchorX/Y via transform shader) (`:352-355`).
- P13.5.6 Connect tap tempo to `BPMTracker::tap()` (`:357-359`).
- P13.5.7 Connect resync to `BPMTracker::resync()` (`:361-363`).
- P13.5.8 Enforce beat snap on clip triggering (`:365-367`).
- P13.5.9 Wire autopilot clip triggering (`:369-371`).
- P13.5.10 Remove `glFinish()` — use async timing with `GL_TIME_ELAPSED` query (`:373-375`).
- P13.5.11 Cache uniform locations in EffectChain (eliminates ~78,600 lookups/sec) (`:377-380`).

### Phase 14 — Quick-Win Effects + Transitions (COMPLETE) (`:386-443`)
20 effects: Greyscale, Threshold, Exposure, Vibrance, Quad Mirror, Flip, Warp Field, Sharpen, Pixel Explosion, Color Flash, Slide Wrap, Dot Field, Triangulate, Auto Mask, Chroma Key, Tile Grid, Spot Zoom, Neon Edge, Cartoon Ink, Pop Raster (`:396-417`).

15 transitions (`:419-441`): Dissolve, Wipe Left/Right/Up/Down, Push Left/Right/Up/Down, Zoom In/Out, Iris Circle, Flip Horizontal, Cut, Fade to Black.

### Phase 15 — Medium Effects + Resolume Sources (COMPLETE) (`:447-489`)
12 effects: Palette Remap, Color Grade (LUT), Bendoscope, UV Remap, Liquid Morph, Edge Blur, Brush Strokes, Fragment Burst, Signal Destroy, Line Cloner, Radial Cloner, Cube Scatter (`:457-470`).

12 sources: Solid Color, Strobe Light, Checkerboard, Line Pattern, Concentric Rings, Sine Oscillator, Spiral Pattern, Metaballs, Terrain Lines, Shape Generator, Bump Light, Infinite Zoom (`:474-487`).

### Phase 16 — Time Effects + Feedback + Signal Routing (COMPLETE) (`:493-538`)
5 time effects (temporal=true): Ghost Trails, Frame Hold, Time Freeze, Screen Split, Frame Delay (`:509-515`).

Feedback Loop system (`:517-525`): `FeedbackProcessor` class, FBO pair + transform shader, `FeedbackConfig` struct (enabled, amount, zoom, rotation, hueShift, lumaKey), LayerInspector Feedback section, 6 presets (Zoom In, Spiral, Drift, Kaleidoscope, Echo, Stretch).

Signal routing integration (`:527-534`): wire `RoutingEngine::processFrame()` into `Renderer::renderOpenGL()`, 5 REST endpoints (signals, add_route, remove_route, routes, set_macro), Python `VJAppController` methods, pass signal tests.

### Phase 17 — Creative Sources: Math / 3D / Geometric / Nature / Lighting (COMPLETE) (`:542-576`)
19 sources: Lissajous Weaver, Fermat Spiral Garden, Hyperbolic Tiling, Penrose Pulse, Moire Interference, Crystal Cavern, Infinite Corridor, Orbit Chamber, Astral Grid, Radial Burst, Hex Grid, Sacred Geometry, Fire Wall, Water Caustics, Electric Arc, Laser Scanner, Scroll Plane, Rotating Cube Map, Dual Plane Drift (`:554-572`).

### Phase 18 — Audio-Native Effects & Sources (COMPLETE) (`:580-627`)
10 audio-driven effects: Harmonic Displacement, Timbral Mosaic, Structural Morph, Pitch Chromatic Shift, Key Palette, Transient Flash, Beat Ripple, Rhythm Slice, Density Wave, Chroma Dissolve (`:594-605`).

8 audio-native sources: Spectrum Landscape, Chromatic Ring, Band Tower, Timbral Nebula, Structural Landscape, Cymatics, Spectral Waterfall, Spectral Ring (`:611-618`).

Text pre-work: 8×8 bitmap font atlas in `EmbeddedShaders.h`; Scrolling Text Wall source `text_wall` (`:622-625`).

### Phase 19 — Complex Effects + Remaining Sources (COMPLETE) (`:631-671`)
8 effects: Luminance Terrain, Voxel Matrix, Drop Shadow, Monitor Wall, Channel Delay (temporal), Data Corruption, Glitch Sort, Topographic Lines (`:643-652`).

12 sources: Superformula, Truchet Labyrinth, Rose Curves, Fibonacci Spiral, Lightning Storm, Fire (stateful), Starfield, Particle Nebula, Radar Sweep, Glitch Grid, DNA Helix, Dot Matrix Wave (`:656-669`).

### Phase 20 — Systems: Layer Router / Per-Type Automation / Text / Simulations (COMPLETE) (`:675-741`)
- Part A — Layer Router Source: `RouterSource` class, per-layer saved textures, self-reference + circular-reference safety (`:687-695`).
- Part B — Per-Type Automation: `PerTypeAutopilotConfig`, separate Opaque/Transparent/Effect timers, CompositionInspector UI (`:697-707`).
- Part C — FFGL Plugin Hosting (deferred per note in phase spec): `FFGLHost`, `FFGLPlugin`, `FFGLEffect`, "Plugins" FX category, Preferences plugin dirs, GL state save/restore, crash isolation + timeout, dlopen/LoadLibrary support (`:709-720`).
- Part D — Text Animator Source: `TextSource` class, JUCE `Graphics::drawText()` rasterization, scroll/pulse/wave/typewriter animations (`:722-729`).
- Part E — Stateful Simulation Sources: Strange Attractor Field (1 FBO), Gravity Well (1 FBO), Fluid Dynamics (3 FBOs) (`:731-740`).

### Phase 20.5 — projectM MilkDrop Integration (COMPLETE) (`:745-825`)
10 tasks: libprojectM-4 dependency, `ProjectMSource` class, GL state save/restore, PCM audio feed, `ProjectMPresetManager` (mood tags, favorites, locking), register `projectm_visualizer` in SourceRegistry with 6 params, `MilkDropBrowser` 6th tab in BrowserPanel, `PresetSelector` helper (auto-switch on `structuralState==DROP`/`BREAKDOWN`), Preferences projectM preset directory, bundle ~100 curated presets with `presets.json` metadata.

### Phase 21 — Live Performance Controls (COMPLETE) (`:829-849`)
10 features: Piano/Momentary keyboard mode, Piano/Momentary MIDI mode, MIDI Velocity mode, MIDI CC Relative mode, 3 Targeting Modes (ByPosition/ThisItem/Selected), Persistent clips across deck switches, Non-interrupting deck switches, Ableton Link integration, Right-click reset to default, Per-clip Beat Snap granularity (Off/Beat/Bar/2-Bar/4-Bar).

### Phase 22 — Output & Integration (COMPLETE) (`:852-870`)
10 features: Syphon output (macOS), Spout output (Windows), Syphon/Spout input, NDI output, NDI input, Video recording (H.264/ProRes/MJPEG), Snapshot (PNG capture), Basic REST API, OSC input support, MIDI Output / pad feedback (5 clip state colors for Launchpad/APC).

### Phase 23 — Smart Audio Features (COMPLETE) (`:873-891`)
8 features: Genre detection (8 genres), Auto-preset selection on genre change, AI mapping suggestions, Smart Random autopilot (energy-aware), Structural scene triggering, ISF shader import, Per-genre FFT tuning, Smart BPM recovery during silence.

### Phase 24 — Workflow Polish (COMPLETE) (`:894-914`)
13 features: 21 easing functions (Elastic/Back/Bounce/Circular × In/Out/InOut + Hold), Signal chaining, Clip Position signal source, Content replacement, Lock Content, Layout presets, Missing file indicators + relocate, Collect Media, Undo improvements, Binding presets export/import, Clip panel update lock, Layer folding, Layer drag reorder.

### Phase 25 — Advanced Audio Analysis (COMPLETE per CLAUDE.md) (`:918-934`)
8 features: Sidechain detection (cross-correlation bass+mid), Swing detection (inter-onset histogram), Formant tracking (spectral flatness 300-3000 Hz), Resonance peak tracking (spectral kurtosis 200-8000 Hz), Reese bass detection (spectral spread 30-200 Hz), Audio stem separation (Demucs ML — flagged for research, not implemented), Composition-level transform, Cross-deck transitions.

**Flagged future/not-implemented item**: P25.6 Audio stem separation (Demucs ML) remains in planning — not indicated as shipped in CLAUDE.md's P25 complete summary.

Running totals per UNIFIED_BUILD_PLAN master schedule: 131 FX + 68 src + 8 systems + 59 features (`:978`).

---

## OurAppFeatures_1.md — master feature list

Master feature document for Audio-DNA. Ordered by UI section.

### 1. Menu Bar — 9 menus (`OurAppFeatures_1.md:11-73`)
- **Audio-DNA**: Preferences, Import ISF, About, Quit (`:15-19`).
- **Composition**: New / Open / Save / Save As, Undo / Redo, Copy Effects / Paste Effects, Collect Media, Relocate Files (`:21-27`).
- **Deck**: New / Insert Before / Insert After / Duplicate, Rename / Close / Clear Clips / Remove (`:29-31`).
- **Layer**: New / Insert Above / Insert Below / Duplicate, Rename / Copy Effects / Paste Effects / Clear Clips / Remove, Ignore Column Trigger, Lock Content, Fold, Move Up / Down (`:33-39`).
- **Column**: New / Insert Before / Insert After / Duplicate, Clear Clips / Remove / Remove All Before / Remove All After (`:41-43`).
- **Clip**: Select All / Cut / Copy / Paste, Copy/Paste Effects, Rename / Clear / Show in Finder, New Source, New Effect, Replace Content, Lock Content (`:45-52`).
- **Output**: Disabled, Fullscreen on [display], Windowed, Identify Displays, Test Card, Snapshot, Start/Stop Recording (`:54-60`).
- **Shortcuts**: Edit Keyboard Bindings, Edit MIDI Bindings, Stop All, Export/Import Bindings (`:62-66`).
- **View**: Panel visibility toggles (Signal Bar / Deck / Preview / Inspector / Browser / Timing Window), FPS Stats, Programming Mode, Save/Load/Reset Layout (`:68-72`).

### 2. Top Bar — 34px full width (`:76-99`)
Controls: Audio source dropdown, Gain slider 0–4x, Play/Pause/Stop, Beat wheel (4-segment), Bar / Phrase display, BPM (color-coded gray/yellow/green), Tracker state, Tap, Resync, Manual toggle, BPM edit field, /4 /2 x1 x2 x4 multipliers, Quantize dropdown (Off/Next Beat/Next Downbeat), Fade slider 0–5s, Master slider 0–1, Output dropdown, FPS, DSP.

### 3. Signal Bar (`:103-118`)
3 display sizes (Minimized / Normal / Expanded), [+] add signal, [−][+] shrink/grow, click opens Signal Inspector. Default signals: RMS, Peak, Bass, Mid, High, Beat Phase, Bar Phase, BPM, Onset Strength, Dominant Pitch, Musical Key, Structural State.

### 4. Deck Grid (`:122-143`)
- Default 3×12 (layers × columns), 250px layer strips, 90×96px clip cells, 22px column triggers, 24px deck tabs, horizontal + vertical scrolling.
- Column trigger buttons: numbered, clicking triggers all clips in column, active cyan highlight.
- Deck tabs: click to switch, active highlighted.
- Layer order: highest at top (Photoshop/Resolume style).

### 5. Layer Strip — 250px per layer (`:146-189`)
- Action: X (clear), B (bypass — dark red), S (solo — olive).
- Transport: `<` (reverse), `||` (pause), `>` (forward), `>|` (2x ff).
- Vertical sliders: S=Speed cyan (0–4x, default 1), K=Key amber (0–1, default 0.1), V=Opacity teal (0–1, default 1.0), F=Fade blue (0–4s, default 0.3s).
- Dropdowns: V-dropdown (48 blend + 13 keying modes), F-dropdown (transition mode).
- Painted: layer name (editable), clip thumbnail, clip name + cyan playhead, in/out range.

### 6. Clip Cell — 90×96px (`:193-216`)
- Thumbnail area 76px, name bar 20px ("FX -" prefix for FX-only).
- States: Empty, Loaded, Active (cyan 2px border), Selected (white 2px border), Missing (red + "!" top-left), Content locked (orange "L" top-right), Has effects (FX count badge).
- Interactions: click thumbnail (trigger/retrigger), click name (select; Cmd/Shift multi-select), drag name (move), drag file from OS (load image/video/sequence), drag from FX/Sources/MilkDrop Browser.

### 7. Bottom Panels — 4 resizable (`:220-244`)
Default proportions 22%/28%/25%/25%: Preview (Preview/Output tabs + waveform), Timing Window (BPM/Routing/Oscillators), Inspector (Clip/Layer/Composition/Signal + pin button), Browser (Files/FX/Sources/Comp-Decks/Record/MilkDrop — 6 tabs).

### 8. Clip Inspector (`:248-316`)
Sections: Name bar, Dashboard (8 link knobs), Transport (Mode Timeline/BPM Sync, timeline bar, playback direction, loop mode Loop/PingPong/OneShot, trigger mode Restart/Continue/Relative, Speed 0–4x with ÷2/×2, Duration slider, Beats/Cycle, Content Beats, Images/Sec for sequences), Cuepoints (8 trigger + 8 set buttons), Autopilot (Action / Duration / Beat Snap dropdowns), Source Parameters, Video (Opacity, Width/Height, Blend Mode, Alpha Type, R G B A toggles), Transform (Position X/Y, Scale, Rotation, Anchor), Effects Stack (per-effect [B] bypass, first-param value, expand arrow, Dry/Wet slider + params).

### 9. Layer Inspector (`:319-389`)
Sections: Name, Dashboard (8 knobs), Autopilot (direction Rewind/Off/Forward/Random, trigger End of Video or On Beat, beat count 1-32, Loops 1-99), Layer Master (opacity, Persistent toggle, Ignore Column Trigger), Video (48 Blend Mode modes in 7 sections — Compositing/Light/Compare/Dodge-Burn/Inversion/Component/Special, Opacity, Width/Height, Auto Size Off/Fill/Fit/Stretch/Original), Transition (Blend Mode across Cut/Dissolve/Wipe/Push/Zoom/3D/Color Fade/Creative — 45+ modes, Duration 0.1–10s), Keying (13 modes: Alpha/Luma Key/Inverted Luma/Luma Is Alpha/Chroma Key/Max RGB/Saturation Key/Edge Detection/Threshold Mask/Channel R/G/B, Threshold, Softness), Dry/Wet (FX Only only), 3D Controls (Rotation X/Y/Z + Speed + Scale), Transform, Feedback (Enable, Preset dropdown, Amount, Scale X/Y, Rotation, Offset X/Y, Luma Key), Layer Effects.

### 10. Composition Inspector (`:392-434`)
Name + Resolution, Dashboard (8 knobs), Autopilot (direction, Duration mode Longest Clip/Clip Transport/Custom, Clip Loops 1-99, Loop toggle, Master Layer), Per-Type Autopilot (Enable, Opaque Beats 1-64, Transparent Beats, Transparent Randomize, Effect Beats, Effect Randomize), Composition Master (0-1) + Speed (0-4x), Video (Opacity), Transform, Global Effects, Output Settings (Resolution dropdown).

### 11. Signal Inspector (`:437-465`)
For Audio: Threshold, Gain 0-4x, Falloff 0-1s. For Oscillator: Wave Shape (Sine/Saw Up/Saw Down/Triangle/Square), Beat Duration (1/4 to 8 beats), Amplitude, Phase Offset. For Envelope: Curve Type (Linear/Exponential/S-Curve), Beat Duration (1-16), Amplitude, Phase, Looping, One Shot, Curve Editor with draggable points.

### 12. Browser Panel (`:469-525`)
- **Files Browser** (`:471-476`): Grid 64px thumbnails or List, path bar, Up, search, favorites.
- **FX Browser** (`:478-483`): **135 effects in 11 categories** — Warp (28), Color (31), Glitch (15), Blur/Post (10), 3D/Depth (10), Pattern (15), Animation (4), Time (6), Audio (7), Blend (5), Composite (3). Search, multi-select, drag.
- **Sources Browser** (`:485-488`): **81 procedural sources in 15 categories** — Noise (3), Fractal (14), Geometric (13), Pattern (8), Organic (2), Nature (5), Audio-Visual (9), 3D (20), Lines (11), Wireframe (7), Text (2), Math (9), Lighting (1), Particle (3), Simulation (3), Routing (1), Special (1). **Note: total categories listed is 17, not 15**.
- **Comp-Decks Browser** (`:490-493`): Compositions + Decks, Save/Load/Delete.
- **Record Panel** (`:495-503`): Record/Stop/Play, Save/Load JSON, Browse Output Folder, Status / Event count.
- **MilkDrop Browser** (`:505-525`): ~9,800 projectM presets, 4 sub-tabs (Curated 30 / Favorites / Recent 20 / All), 3 play modes (Jukebox with Pool + Timing 4/8/16/32 beats or 10/30/60s + Blend; VJ Clip click-preview + drag-single; Playlist multi-select drag-group), Navigation (< / > / ? / Lock).

### 13. Universal Parameter Control (`:528-546`)
Collapsed (24px): signal triangle (gray=manual, cyan=connected), label, value, [-]/[+] inc/dec, slider with right-click reset.
Expanded: Source mode button (Manual/Signal/BPM Sync/Oscillator/Envelope/Clip Position/Timeline/Macro), Invert checkbox, Range Min/Max sliders, Source value meter.

### 14. Binding System (`:550-586`)
- Input types: Keyboard+modifiers, MIDI Note (channel+note+velocity), MIDI CC (absolute or Relative/endless).
- Trigger modes: Toggle, Momentary (piano).
- Targeting modes: By Position, This Item, Selected.
- Actions: Trigger Clip, Trigger Column, Toggle Layer Bypass/Solo/Mute/Visible, Toggle Layer Autopilot, Layer Transport, Toggle Effect Bypass, Adjust Macro, Adjust Layer Opacity, Switch Deck, Tap Tempo, Resync, Global Play/Pause/Stop, Master Opacity, Snapshot, Toggle Recording.
- MIDI Velocity → clip opacity on trigger.

### 15. Signal & Routing System (`:590-612`)
- Signal types: Audio (80+ FeatureSnapshot fields), Oscillator (BPM-locked Sine/Saw/Triangle/Square), Envelope, Clip Position (0-1), Chained (Multiply/Add/Gate/ScaleRange).
- Per-route: output range, invert, threshold, gain, falloff.
- **24 curve types**: Linear, Exponential, Logarithmic, S-Curve, Stepped, Hold, Circular (In/Out/InOut), Back, Elastic, Bounce, Cubic, Sine (3 each).
- 8 dashboard macro knobs per scope (clip / layer / composition).

### 16. Output & Recording (`:616-637`)
Output Window (fullscreen or windowed, Escape to close), Video Recording (H.264/ProRes/MJPEG, triple-buffered readback, dropped-frame counter), Syphon Output (macOS), Snapshot, Session Recording (JSON save/load).

### 17. Preferences (`:641-652`)
Tabs: General (Confirm on quit, Show tooltips), Audio (Sample rate, Buffer size, BPM detection range), Video (FPS target, Render resolution, MilkDrop preset directory), MIDI (placeholder), Recording (placeholder), Defaults (placeholder), Feedback (placeholder), About (version, credits). **Flag**: multiple tabs marked as placeholder = partial implementation.

### 18. Layer Types (`:656-664`)
5 types: Opaque, Transparent, FX Only, 3D, Mask.

### 19. Effect Chain Architecture (`:668-676`)
3 levels: Per-clip (before keying + layer blend), Per-layer (after clip comp, before accumulator blend), Global (after all layers).

### 20. Keyboard Shortcuts (`:680-692`)
Cmd+S / Cmd+O / Cmd+Z / Cmd+Shift+Z / Cmd+F / Escape / Shift+Cmd+K (keyboard bind mode) / Shift+Cmd+M (MIDI learn mode) / Tab (customizable).

### 21. Data Hierarchy (`:696-728`)
Composition → Deck[1..N] → Layer[1..M] → Clip[1..K]. Full field list per level including genre automation, per-type autopilot config, composition transform, cuepoints[8], content lock flag, MilkDrop playlist (cycle mode / trigger / blend / entries).

### OurAppFeatures count
Approximate top-level distinct features / controls: **~280** (menu items + top-bar controls + layer strip controls + inspector sections + binding actions + signal types + curve types + effect-chain levels + shortcuts + data hierarchy items).

---

## SOURCES_procedural_generators.md — source catalog

This is the research landscape document — NOT an Audio-DNA inventory. It documents every source type available industry-wide, used to plan Audio-DNA's 81-source implementation.

### 1. Industry survey (SOURCES_procedural_generators.md:28-106)

**Resolume Wire sources** (`:31-35`): 2D Shape System (Circles, rectangles, morphing blobs, psychedelic patterns), Text Rendering, ISF Shader Generators, Beat-responsive Sources.

**TouchDesigner Generator TOPs** (`:38-48`): Noise TOP (Perlin/Simplex/Sparse/Alligator), Ramp TOP (linear/radial/circular), Circle TOP, Rectangle TOP, Constant TOP, Text TOP, Render TOP, Function TOP, Pattern TOP (grid/stripe/checkerboard), CHOP-to-TOP.

**ISF generators listed** (`:49-100`): Audio Waveform Shape, Basic Shape, Bounce, Brick Pattern, Checkerboard, Circle, City Lights, Color Bars, Color Organ Polyphonic, Color Scales, Color Schemes, Color Test Grid, Crazy Parametric Fun, Digital Clock, Doodler, FFT Color Lines, FFT Filled Waveform, FFT Spectrogram, Graph Paper, Heart, Life (Conway's GoL), Line Group, Lines, Linear Gradient, Multi Gradient, Noise, Polar Function, Poly Star, Radial Gradient, Radial Spectrogram, Random Characters, Random Checkerboard, Random Lines, Random Shape Blast, Random Shape, Random Squares, Random Stripes, Solid Color, Spiral, Star, Stripes, Test Pattern Generator, Triangle, Triangles, Truchet Tile, TV Static, VU Meter. **(46 listed ISF generators)**.

**Max/MSP + Jitter**: `jit.gl.gridshape`, `jit.gl.mesh`, `jit.gl.shader`, `jit.gl.pix` (Gen-based GLSL editor).

### 2. Mathematical / Fractal Sources (`:110-295`)

1. **Mandelbrot Set** (§2.1 `:112-144`) — zoom, centerX/Y, maxIterations, colorSpeed, colorOffset, power.
2. **Julia Set** (§2.2 `:146-162`) — cReal, cImag, zoom, rotation, maxIterations, colorScheme.
3. **Lorenz Attractor** (§2.3 `:164-194`) — sigma, rho, beta, rotation, trailLength, lineWidth, colorMode.
4. **Strange Attractors (Clifford, De Jong, Rössler, Aizawa, Thomas, Halvorsen)** (§2.4 `:196-211`).
5. **Flame Fractals (IFS)** (§2.5 `:213-222`).
6. **Reaction-Diffusion (Gray-Scott)** (§2.6 `:224-255`) — feedRate, killRate, diffusionU, diffusionV, dt, colorScheme, seedMode.
7. **Cellular Automata** (§2.7 `:257-283`) — Game of Life, Rule 110 / Elementary CA, Brian's Brain, Wireworld, Langton's Ant, Lenia.
8. **L-Systems** (§2.8 `:285-294`).

### 3. Geometric Pattern Sources (`:298-456`)

1. Lissajous / Oscilloscope (§3.1 `:300-328`).
2. Spirograph / Hypotrochoid (§3.2 `:330-338`).
3. **Sacred Geometry** (§3.3 `:340-357`) — Flower of Life, Metatron's Cube, Sri Yantra, Seed of Life, Tree of Life, Vesica Piscis, Torus / Tube Torus.
4. Voronoi Patterns (§3.4 `:359-382`).
5. Moire Patterns (§3.5 `:384-400`).
6. Kaleidoscope Generator (§3.6 `:402-419`).
7. **Grid Patterns** (§3.7 `:421-437`) — Dot grid, Line grid, Crosshatch, Checkerboard, Brick, Hexagonal.
8. Concentric Circles / Rings (§3.8 `:439-445`).
9. Radial Burst / Starburst (§3.9 `:447-455`).

### 4. Particle Systems (`:459-530`)
4.1 Fragment vs Compute analysis (`:461-475`).
4.2 Simple Particle Emitter (CPU-driven).
4.3 Fragment shader fake particles (`:495-515`).
4.4 Specific particle types (`:519-530`): Fireworks, Snow, Rain, Sparkles, Smoke/Fog, Confetti, Bubbles.

### 5. 3D Primitives (Ray Marching) (`:533-599`)
Basic SDF primitives (`:542-548`): Sphere, Box, Round Box, Torus, Cylinder, Cone, Plane, Capsule, Box Frame, Capped Torus, Link, Hexagonal Prism, Rounded Cylinder, Capped Cone, Pyramid, Octahedron, Death Star, Solid Angle, Cut Sphere, Rhombus, Vesica Segment.

Specific 3D sources (`:550-588`): Rotating Primitives, Wireframe Rendering, Multiple Objects (domain repetition), Tunnel, Terrain.

### 6. Noise / Organic Sources (`:602-727`)
1. Perlin Noise (2D, 3D) + FBM.
2. Simplex Noise.
3. Worley / Cellular Noise (F1, F2, F2-F1).
4. Plasma Effect (demoscene classic).
5. Fire / Flame Simulation.
6. Water Caustics.
7. Clouds.

### 7. Concert Lighting Simulations (`:730-852`)
1. Moving Head Beams.
2. Laser Beam Patterns (Fan, Tunnel, Grid, Spiral, Star).
3. LED Wall Patterns.
4. Spotlight with Gobos.
5. Strobe Arrays.
6. Atmospheric Haze with Beams.
7. Par Can Wash.

### 8. Text / Typography Sources (`:855-899`)
1. Text Rendering via JUCE.
2. Scrolling Text.
3. Text with Effects (glow, outline, shadow, RGB split, glitch displacement).
4. Character Matrix (Matrix Rain).

### 9. Audio-Visual Sources (`:902-991`)
1. Waveform Display.
2. Spectrum Analyzer Bars.
3. Circular Spectrum.
4. Audio-Driven Particle Emission.
5. Chromagram Display.
6. Beat Pulse.

### 10. ArKaos Generator Effects (`:994-1021`)
True Generators: Waveform, Digital Noiz (5 presets), Stroboscope, Color Bars.
Pseudo-Generators: Tunnel, Screen Room, Cube Inside, 3D Objects (plane, cube, sphere, cylinder, donut), Plane, RotoZoom, Larsen (3 variants).

### 11. Priority Matrix (`:1024-1106`)
Tier 1 (10 sources): Plasma, Noise/FBM, Tunnel, Waveform Display, Spectrum Bars, Concentric Rings, Radial Burst, Moving Head Beams, Strobe Flash, Voronoi. Tier 2 (11-20), Tier 3 (21-30), Tier 4 (31-40).

### Source catalog — full name listing

All source names mentioned across SOURCES_procedural_generators.md + its reference to the Audio-DNA 81-source set (via the browser listing and the phase plans):

**Mathematical / Fractal (8 families + variants)**: Mandelbrot Set, Julia Set, Lorenz Attractor, Clifford Attractor, De Jong Attractor, Rössler Attractor, Aizawa Attractor, Thomas Attractor, Halvorsen Attractor, Flame Fractals, Reaction-Diffusion (Gray-Scott), Game of Life, Rule 110, Elementary CA, Brian's Brain, Wireworld, Langton's Ant, Lenia, L-Systems.

**Geometric Pattern (21)**: Lissajous, Spirograph, Flower of Life, Metatron's Cube, Sri Yantra, Seed of Life, Tree of Life, Vesica Piscis, Voronoi, Moire, Kaleidoscope Generator, Dot grid, Line grid, Crosshatch, Checkerboard, Brick pattern, Hexagonal grid, Concentric Circles, Radial Burst.

**Particle Systems (7 variants + emitter)**: Particle Emitter, Fireworks, Snow, Rain, Sparkles, Smoke / Fog, Confetti, Bubbles.

**3D Primitives (ray march)**: Sphere, Box, Round Box, Torus, Cylinder, Cone, Plane, Capsule, Box Frame, Capped Torus, Link, Hexagonal Prism, Rounded Cylinder, Capped Cone, Pyramid, Octahedron, Death Star, Solid Angle, Cut Sphere, Rhombus, Vesica Segment, Tunnel, Terrain.

**Noise / Organic (7)**: Perlin Noise, Simplex Noise, FBM, Worley / Cellular Noise, Plasma, Fire / Flame, Water Caustics, Clouds.

**Concert Lighting (7)**: Moving Head Beams, Laser Beam Patterns (Fan/Tunnel/Grid/Spiral/Star), LED Wall, Spotlight with Gobos, Strobe Arrays, Atmospheric Haze, Par Can Wash.

**Text / Typography (4)**: Text Rendering (JUCE-rasterized), Scrolling Text, Text with Effects, Matrix Rain.

**Audio-Visual (6)**: Waveform Display, Spectrum Analyzer Bars, Circular Spectrum, Audio-Driven Particle Emission, Chromagram Display, Beat Pulse.

**ArKaos generator & pseudo-generators (11)**: Waveform, Digital Noiz, Stroboscope, Color Bars (true). Tunnel, Screen Room, Cube Inside, 3D Objects, Plane (Dual Plane Drift), RotoZoom (Infinite Zoom), Larsen (Feedback Loop).

**Total distinct source types enumerated across SOURCES_procedural_generators.md**: ~100+ (matches the CLAUDE.md project identity claim of "Audio-DNA should have ~103" in research). **Audio-DNA ships 81** per OurAppFeatures; research documents a superset of ideas.

---

## Effect/source integration specs (arkaos + resolume)

### resolumeEffectSourceIntegration.md

Infrastructure (Section 2, `resolumeEffectSourceIntegration.md:40-101`):
- 2.1 Per-Effect Dry/Wet Mix (CRITICAL — every Resolume effect has Opacity).
- 2.2 Temporal Effect Buffer System — needed for Delay, Trails, Feedback, Stop Motion.
- 2.3 LUT Loading System — `.cube` parser → GL_TEXTURE_3D GL_RGB32F.

Resolume mapping (Section 3, `:104-259`):
- 3A Color (17 effects) — AddSubtract, Bright.Contrast, Colorize, Color Pass, Greyscale, Hue Rotate, Invert RGB, Levels, Posterize, Saturation, Threshold, Exposure, Vibrance, Tint, Solarize, Recolour, LUT.
- 3B Warp / Distortion (21 effects) — Distortion, Ripples, Wave Warp, Kaleidoscope, Bendoscope, Polar Kaleidoscope, Mirror, Mirror Quad, Flip, Suckr, Fish Eye, Twirl, Twisted, Keystone (x3 — SKIP), UV Map, Displace, Goo, Space Warper, Shifty.
- 3C Blur (5 effects) — Blur, Pixel Blur, Radial Blur, Edge Blur, Sharpen.
- 3D Stylize (16 effects) — LoRez, Dither, Dot Screen, Raster, Hatched, Edge Detection, Bloom, Static, Vignette, Expand, Dilate, Kuwahara, VHSifyer, Strokes, CRT, Reducto.
- 3E Glitch / Shift (8 effects) — Shift Glitch, Shift RGB, Fragment, Freeze, Blow, Flash, Noise, TVA.
- 3F Composite / Clone (3 effects) — Linear Cloner, Radial Cloner, Cube Tiles.
- 3G Time-Based (7 effects) — Delay, Delay RGB, Trails, Feedback, Strobe, Stop Motion, Stroboscope.
- 3H Transform (4 effects) — Transform, Tile, Slide, Iterate.
- 3I 3D / Geometry (5 effects) — Stingy Sphere, Cube Tiles, Luma Waves, Pixels In Space, Point Grid.
- 3J Special (16 effects) — Circles, Snow, Videowall, Radar, Triangulate, Twitch, Sparkles, Drop Shadow, Auto Mask, Chromakey, Particle System, Smooth Transform, Screen Shake, FeedbackPro, Slit Scanner, Pixel High Pass.

**32 new effects specified in Section 4** (`:263-1091`): C1-C6 (Greyscale, Threshold, Exposure, Vibrance, Palette Remap, Color Grade LUT), W1-W6 (Bendoscope, Quad Mirror, Flip, UV Remap, Liquid Morph, Warp Field), B1-B2 (Edge Blur, Sharpen), S1 (Brush Strokes), G1-G5 (Fragment Burst, Time Freeze, Pixel Explosion, Color Flash, Signal Destroy), X1-X3 (Line Cloner, Radial Cloner, Cube Scatter), T1-T5 (Frame Delay, Channel Delay, Ghost Trails, Infinite Feedback, Frame Hold), R1-R3 (Tile Grid, Slide Wrap, Fractal Iterate), D1-D3 (Luminance Terrain, Voxel Matrix, Dot Field), J1-J5 (Monitor Wall, Triangulate, Drop Shadow, Auto Mask, Chroma Key).

**18 Resolume sources** (`resolumeEffectSourceIntegration.md:1096-1117`): Solid Color, Gradient, Stroboscope, Text Animator, Text Block (SKIP), Shaper, Checkered, Lines, Rings, Sine Wave, Spiral, Metaballs, Line Scape, Test Card (SKIP), Video Router, Abstract Field, Tunnelines, Slice Outline (SKIP).

**12 new sources in Section 6** (`:1122-1368`): S1-S12 — Solid Color, Strobe Light, Text Animator, Shape Generator, Checkerboard, Line Pattern, Concentric Rings, Sine Oscillator, Spiral Pattern, Metaballs, Terrain Lines, Layer Router (INFRA).

Implementation phases A-E (`:1372-onwards`): Phase A = Infrastructure, Phase B = 20 small effects + sources (quick wins), further phases for medium/complex items.

### archaosEffectSourceIntegration.md

8 new effects (`archaosEffectSourceIntegration.md:61-385`):
1. **Tile Grid** (warp) — ArKaos Tiling, 4 params.
2. **Infinite Zoom** (warp) — ArKaos RotoZoom, 4 params.
3. **Bump Light** (pattern) — ArKaos Bumpy Surface, 4 params.
4. **Pop Raster** (pattern) — ArKaos Pop Art, 4 params.
5. **Screen Split** (glitch) — ArKaos Video Split, 4 params.
6. **Spot Zoom** (warp) — ArKaos Target, 6 params.
7. **Neon Edge** (pattern) — ArKaos Neon, 4 params.
8. **Cartoon Ink** (pattern) — ArKaos Cartoon, 4 params.
Plus Strobe upgrade (add Mode + Shape params).

4 new sources (`:413-523`):
1. **Scroll Plane** (Geometric) — ArKaos Scroller, 4 params.
2. **Rotating Cube Map** (Geometric) — ArKaos Cube Inside, 4 params.
3. **Dual Plane Drift** (Geometric) — ArKaos Plane, 4 params.
4. **Color Bars** (Test) — 4 params (Pattern/Noise/Scanlines/Drift).

Systems (`:526-976`):
- **Feedback Loop (Larsen)** (`:526-694`) — `FeedbackProcessor` class + FBO pair + feedback shader + 6 presets (Zoom In, Spiral, Drift Right, Kaleidoscope, Echo, Stretch).
- **Per-Type Automation** (`:697-793`) — opaque/transparent/effect independent cycling, layer-type to autopilot-category mapping.
- **FFGL Plugin Hosting** (`:796-976`) — FFGLHost/FFGLPlugin/FFGLEffect, FFGL 2.x OpCode reference (0–33), platform-specific scan paths, GL state protection, crash isolation.

Sprint schedule (`:1007-1256`): 7 sprints, 13-15 sessions total.

Section 10 creative expansion (34 sources) covers categories A-H (Math, 3D, Geometric, Audio-Visual, Lighting, Nature, Typography, Simulation).

---

## UI/UX design patterns adopted

Drawn from UI_UX_* analyses. Features/patterns that the Audio-DNA v2 implements or explicitly models on these apps:

### From Resolume (RESOLUME_UI_UX_ANALYSIS.md, Creative_Tools, UI_UX_MASTER_COMPARISON.md)
- **Parameter animation triangle (cogwheel)** — one click to connect parameter to signal (adopted as UniversalParamControl signal triangle) (`UI_UX_MASTER_COMPARISON.md:59-60`).
- **Clip grid as centerpiece** — 40-50% of screen (adopted as deck grid) (`:60-61`, `RESOLUME_UI_UX_ANALYSIS.md:124-127`).
- **Beat Snap quantization** (adopted: Off/Beat/Bar/2Bar/4Bar in P21) (`UI_UX_MASTER_COMPARISON.md:62`).
- **Dashboard dials (8 macro knobs per scope)** adopted in Clip/Layer/Composition Inspectors (`RESOLUME_UI_UX_ANALYSIS.md:196-200`).
- **Right-click to reset parameter** (adopted as ResettableSlider) (`RESOLUME_UI_UX_ANALYSIS.md:192-195`).
- **Industrial-utilitarian dark UI** / flat / dense / muted / consistent (adopted: #1a1a1a-#2a2a2a backgrounds, #e0e0e0 text) (`RESOLUME_UI_UX_ANALYSIS.md:57-64`).
- **Layer strip controls** X/B/S + V/A/M faders (adopted in Layer Strip with X/B/S + S/K/V/F sliders).
- **Layer folding** (adopted in P24).
- **Beat-snap + Quantize** (adopted in top bar).
- **Composition → Deck → Layer → Clip hierarchy** (adopted as the v2 data model).

### From Ableton Live 12 (Ableton_Live_12_UI_UX_Analysis.md, UI_UX_MASTER_COMPARISON.md)
- **Session View clip grid** (foundation of deck grid) (`UI_UX_MASTER_COMPARISON.md:65-66`).
- **MIDI learn speed** — 3-action learn (adopted as Shift+Cmd+M mode) (`:68`).
- **Parametric color engine / dark gray background** (`Ableton_Live_12_UI_UX_Analysis.md:15`).
- **Flat design, no gradients/skeuomorphism** (adopted).
- **MIDI Map / Key Map mode full-interface color washes** (adopted as binding modes).
- **70-color clip palette** (Audio-DNA supports clip color assignment in deck).
- **Progressive disclosure** — 6 levels (Surface / Inspector / Signal / Deep settings) — adopted via panel system.

### From Serato DJ Pro (DJ_Software_UI_UX_Analysis.md, UI_UX_MASTER_COMPARISON.md)
- **Minimal chrome** — "if it's not information or a control, it shouldn't be visible" (`UI_UX_MASTER_COMPARISON.md:72`).
- **Lock Playing Deck** (adopted: Lock Content on clip/layer in P24).
- **EQ-colored waveform** — multiple data dimensions in one element (inspiration for signal-bar + waveform display).
- **Dark cockpit / low-light readability** (adopted).

### From GrandMA3 (GrandMA3_UI_UX_Analysis.md, UI_UX_MASTER_COMPARISON.md)
- **Tile-based workspace / layout presets** — inspired `View → Save Layout / Load Layout / Reset Layout` (`UI_UX_MASTER_COMPARISON.md:76-77`).
- **Systematic color coding** (adopted: cyan=active, red=error, orange=bypass, olive=solo).
- **Dark cockpit philosophy** (adopted).
- **Command line + GUI dual-input** (NOT adopted — no command line in Audio-DNA).

### From TouchDesigner (TouchDesigner_UI_UX_Analysis.md, UI_UX_MASTER_COMPARISON.md)
- **Operator family color system** (partially adopted as category colors in FX and Sources Browsers).
- **Signal routing visualization** — hover parameter to see connections (`UI_UX_MASTER_COMPARISON.md:175-176`).
- **Value ladder / middle-click precision** — not adopted.
- **Live thumbnails on every node** — not adopted (single preview panel instead).

### From Cables.gl (Creative_Tools_UI_UX_Analysis.md, UI_UX_MASTER_COMPARISON.md)
- **Command palette for creation** (NOT YET adopted — flagged as "Must-Have" in UI_UX_MASTER_COMPARISON Must-Have list) (`UI_UX_MASTER_COMPARISON.md:169-171`).
- **Color-coded data types on wires** (partially adopted via signal source categorization).
- **Flow Mode** — signal propagation visualization (not adopted).

### From Notch / Smode (Notch_Smode_UI_UX_Analysis.md, UI_UX_MASTER_COMPARISON.md)
- **Profiler overlay for bottleneck identification** (partially — top bar FPS/DSP display).
- **Resource Inspector (pre-import preview)** (partially — Files Browser with thumbnails).
- **Instantiator (Ctrl+Space universal search)** (NOT adopted — flagged by master comparison).
- **AE-familiar layer model** (adopted — our layer model is layer-based, not node-based).

### From Traktor / rekordbox (DJ_Software_UI_UX_Analysis.md, UI_UX_MASTER_COMPARISON.md)
- **Layout presets** — Essential/Extended/Browser (adopted in P24).
- **Flexible beatgrids** — tempo-changing content (partially — Manual BPM mode exists).
- **Column View browser** — macOS Finder-style (adopted in Files Browser as List/Grid views).
- **AI-powered features (auto-cue, vocal detection)** (partially — genre detection P23, smart autopilot).

### From HeavyM / Mapping tools (Mapping_Software_UI_UX_Analysis.md)
- **Visual effect browser with previews** — thumbnails showing effect behavior before apply (flagged as Must-Have in UI_UX_MASTER_COMPARISON, partially adopted in FX Browser).
- **Wizard-style workflow** / "a few minutes to get started" (NOT adopted — no wizards).
- **Dashboard columns = states, rows = layers** (adopted in column trigger metaphor).

### From VDMX / ArKaos GrandVJ (VJ_Software_UI_UX_Analysis.md)
- **Modular/floating panel system** — VDMX (partially — undockable via View menu).
- **Data receiver math expressions** — VDMX (NOT adopted).
- **Synth/Mixer dual mode** — ArKaos (NOT adopted).
- **Single opinionated layout** — ArKaos (adopted — default layout exists, customizable).

### Universal UI/UX Live Performance principles (UI_UX_LIVE_PERFORMANCE.md:1-200)
- **Zero-tolerance for confusion** — no confirmation dialogs on hot path (`:13-18`).
- **Low-light readability** — dark themes as functional requirement (`:21-27`).
- **Split-attention operation** — glanceability (`:30-35`).
- **Muscle memory / spatial stability** — nothing moves (`:38-43`).
- **Latency perception** — <50ms target for instantaneous, <80ms audio-visual (`:47-56`).
- **Panic scenarios** — kill button, recoverable state, system health always visible (`:59-66`).
- **Dark themes** — dark gray not pure black (#1a1a1a-#2a2a2a) (`:72-83`).
- **Desaturation on dark backgrounds** (20-40%) (`:91-92`).
- **Sans-serif only**, 10px minimum, monospaced for numerics (`:108-119`).

---

## Section 18 partial: Features in research but maybe not in code

Items in research docs that read as planned / future / TODO / deferred / not-yet-shipped:

### Explicitly deferred or not shipped
- **FFGL Plugin Hosting** — P20 Part C in `UNIFIED_BUILD_PLAN.md:709-720`, flagged in P20 complete note at `:679` as "deferred to a future phase (requires external SDK + plugin ecosystem)". Full design exists in `archaosEffectSourceIntegration.md:796-976`.
- **Audio stem separation (Demucs ML)** — P25.6 in `UNIFIED_BUILD_PLAN.md:931` listed in phase tasks but CLAUDE.md P25 complete summary ships only 5 advanced features + transform + cross-deck — stems NOT listed as shipped.
- **P13.5.4 Transition blend mode shaders** — moved from P13.5 to P14.21-P14.35 (`UNIFIED_BUILD_PLAN.md:348-350`).
- **Text Block source (Resolume)** — explicitly SKIPPED (`resolumeEffectSourceIntegration.md:1107`).
- **Test Card source (Resolume)** — explicitly SKIPPED (`resolumeEffectSourceIntegration.md:1116`).
- **Slice Outline source (Resolume)** — SKIPPED (projection mapping utility, `:1118`).
- **Keystone effects (3)** — SKIPPED (projection mapping), `resolumeEffectSourceIntegration.md:146-149`.
- **Displace (Resolume)** — SKIP for now, marked as "Video Router addresses this use case" (`:151`).
- **Expand / Dilate / Reducto / Circles / Snow / Radar / Twitch / Sparkles / Particle System / Smooth Transform / Pixel High Pass** — all SKIP (11 effects), `resolumeEffectSourceIntegration.md` Section 3D/3E/3J.
- **Command palette (Ctrl+Space instantiator)** — in UI_UX_MASTER_COMPARISON "Must-Have" list, not in OurAppFeatures_1.md.
- **Visual effect browser with previews (thumbnails)** — in UI_UX_MASTER_COMPARISON "Must-Have" list, browsers show names/categories, not preview thumbnails.
- **Signal routing visualization on hover** — in UI_UX_MASTER_COMPARISON "Should-Have" list, not confirmed in OurAppFeatures.
- **EQ-colored waveform equivalent** — in UI_UX_MASTER_COMPARISON "Nice-to-Have" list, not in OurAppFeatures.
- **Workspace save/recall per show/venue** — in UI_UX_MASTER_COMPARISON "Nice-to-Have" list; partial support via View → Save/Load/Reset Layout.

### Preferences panel placeholders (OurAppFeatures_1.md:641-652)
- **MIDI tab** — placeholder.
- **Recording tab** — placeholder.
- **Defaults tab** — placeholder.
- **Feedback tab** — placeholder.

### Content flagged as incomplete in research docs
- **`SOURCES_procedural_generators.md` Tier 4 sources** (31-40): Strange Attractors, Lorenz, Flame Fractals, L-Systems, Terrain, Beat Pulse, Scrolling Text, Worley, Par Can Wash, Haze/Fog — most shipped, but Flame Fractals and L-Systems NOT in 81-source browser.
- **`SOURCES_procedural_generators.md` §2.5 Flame Fractals** — "Extremely difficult as a fragment shader. CPU + GPU accumulation. High-quality renders need millions of samples" — not shipped.
- **`SOURCES_procedural_generators.md` §2.8 L-Systems** — "Not practical as a pure fragment shader. Must be computed on CPU" — not shipped.

### OurAppFeatures sections that may not match code 1:1
- **Browser Sources categories** claim "15 categories" but list 17 (extra Simulation + Routing) (`OurAppFeatures_1.md:486-487`) — minor documentation inconsistency.
- **Composition Inspector "Per-Type Autopilot"** includes Transparent Randomize + Effect Randomize toggles — confirm in code vs. `PerTypeAutopilotConfig` in the memory references.

---

## Section 20 partial: Cross-references

### Audio-DNA current counts vs. research promises

| Metric | OurAppFeatures_1.md | UNIFIED_BUILD_PLAN.md totals | Notes |
|--------|--------------------|-------------------------------|-------|
| Effects | 135 in 11 categories | 131 FX master schedule | Off by 4; CLAUDE.md says 135 (correct current) |
| Sources | 81 in 15 categories | 68 src master schedule | 81 matches CLAUDE.md; plan under-estimated |
| Blend modes | 48 across 7 sections | N/A | From layer inspector |
| Keying modes | 13 | N/A | From layer inspector |
| Transitions | (45+ modes in Transition dropdown) / 15 shaders | 15 in P14 | OurAppFeatures says "45+ modes", but P14 shipped 15 shaders — flag: counting discrepancy |
| Curve types | 24 | 21 easing functions in P24 | 24 total including basic 6 + 21 easing — roughly matches |
| Genres detected | 8 | 8 | P23 |
| Dashboard macro knobs | 8 per scope (clip/layer/composition) | 8 | Matches |
| Default signals in Signal Bar | 12 (RMS, Peak, Bass, Mid, High, Beat Phase, Bar Phase, BPM, Onset Strength, Dominant Pitch, Musical Key, Structural State) | — | from OurAppFeatures_1.md:118 |
| Signal route types | 5 (Audio, Oscillator, Envelope, Clip Position, Chained) | — | from OurAppFeatures_1.md:592-598 |
| Oscillator wave shapes | 5 (Sine/Saw Up/Saw Down/Triangle/Square) | — | from OurAppFeatures_1.md:452 |
| Envelope curve types | 3 (Linear/Exponential/S-Curve) | — | from OurAppFeatures_1.md:459 |
| Binding actions | ~17 distinct | — | from OurAppFeatures_1.md:567-583 |
| Layer types | 5 (Opaque/Transparent/FXOnly/3D/Mask) | — | from OurAppFeatures_1.md:656-664 |
| Effect chain levels | 3 (per-clip/per-layer/global) | — | from OurAppFeatures_1.md:668-676 |
| Feedback presets | 6 (Zoom In/Spiral/Drift/Kaleidoscope/Echo/Stretch) | 6 | P16, matches archaos doc |

### Cross-document references

- **UNIFIED_BUILD_PLAN.md** — authoritative for phases; references `research/resolumeEffectSourceIntegration.md` Section 4+6+10, `research/archaosEffectSourceIntegration.md` Sections 2+3+4+5+6+10 (`UNIFIED_BUILD_PLAN.md:134-147`).
- **OurAppFeatures_1.md** — most comprehensive single-file user-visible inventory; no code file citations, pure feature description.
- **SOURCES_procedural_generators.md** — research landscape only, references industry tools (Resolume Wire, TouchDesigner, ISF, Max/Jitter, ArKaos), and `VIDEO_opengl_integration.md` + `VIDEO_vj_frameworks.md`.
- **resolumeEffectSourceIntegration.md** — per-effect GLSL specs, Section numbers referenced from UNIFIED_BUILD_PLAN phases P14/P15/P18/P19.
- **archaosEffectSourceIntegration.md** — Sections 2/3/4/5/6/10 referenced from UNIFIED_BUILD_PLAN phases P14/P15/P17/P20.
- **VIDEO_feature_to_visual_mapping.md** — master mapping table with every feature (RMS, Spectral Centroid, Spectral Flux, Onset, Beat, Bass/Mid/Treble, BPM, Key/Chord, Roughness, MFCC, Loudness) → visual parameters, curves, smoothing.
- **VIDEO_opengl_integration.md** — UBO/SSBO/texture upload details for GPU; cross-refs ARCH_pipeline, LIB_juce.
- **VIDEO_vj_frameworks.md** — evaluations of openFrameworks, Cinder, etc. (comparative research, architecture not adopted directly).
- **IMPL_calibration_adaptation.md** — runtime calibration (Sliding Window, Decay Envelope, Percentile, Z-Score, Histogram Equalization, Genre Detection, Silence Handling, Preset System) — partially adopted (P23 genre detection, P23 BPM recovery during silence).
- **IMPL_minimal_prototype.md** — pre-v1 prototype plan with miniaudio + KissFFT (superseded by JUCE + juce::dsp::FFT).
- **IMPL_project_setup.md** — original source tree and CMake pattern (largely adopted).
- **IMPL_testing_validation.md** — test signal generation, validation criteria (adopted in visual test harness).
- **UI_UX_MASTER_COMPARISON.md** — 19-app ranking + design-pattern shopping list; links to 10 per-app analyses.
- **UI_UX_LIVE_PERFORMANCE.md** — universal principles, not tied to a specific app.
- **RESOLUME_UI_UX_ANALYSIS.md** — primary reference for deck/layer/clip grid model.
- **Ableton_Live_12_UI_UX_Analysis.md** — Session View clip grid, MIDI learn, parametric color.
- **DJ_Software_UI_UX_Analysis.md** — Traktor layout presets, Serato minimal chrome + EQ waveform, rekordbox Column View.
- **GrandMA3_UI_UX_Analysis.md** — tile workspace, systematic color coding (partially adopted).
- **Mapping_Software_UI_UX_Analysis.md** — MadMapper spatial calibration, Millumin dashboard, HeavyM visual effect browser.
- **Notch_Smode_UI_UX_Analysis.md** — Notch node + profiler, Smode instantiator.
- **TouchDesigner_UI_UX_Analysis.md** — operator family color, parameter mode colors, live thumbnails.
- **Creative_Tools_UI_UX_Analysis.md** — Magic Music Visuals node patching, Cables.gl command palette + Flow Mode, Processing minimalism, ISF Editor.
- **VJ_Software_UI_UX_Analysis.md** — VDMX modular panels, ArKaos GrandVJ fixed 3-panel.

### Gaps / inconsistencies worth investigating elsewhere
1. Transition count mismatch: OurAppFeatures says "45+ modes" for transition dropdown vs. 15 shipped transitions — likely includes unshipped variants or the 7 transition-style categories.
2. Sources categories listed as "15 categories" but 17 enumerated in OurAppFeatures (Simulation, Routing appear to be extras).
3. P25 Audio stem separation listed in phase plan but not in CLAUDE.md P25 shipped summary — confirm via code audit.
4. P20 Part C FFGL Plugin Hosting has full design (archaos Section 6) but is flagged as deferred — confirm no skeleton/stubs in code.
5. Ableton Link is optional (`AUDIODNA_BUILD_LINK=ON`) per CLAUDE.md — verify the flag is actually wired.
6. Preferences MIDI/Recording/Defaults/Feedback tabs marked as placeholders in OurAppFeatures.
7. Audio signal count: OurAppFeatures claims "80+ FeatureSnapshot fields" for Audio signal type; FeatureSnapshot table in CLAUDE.md lists ~45 fields — either over-counted or includes derived/smoothed variants.

---

Summary: see the required brief summary at the end of this audit message below.
