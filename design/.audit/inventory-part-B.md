# FEATURE INVENTORY — Part B: Visual Content, Effects & Processing, Output & Recording

Domains 2, 3, 7 of 9 | Builder agent B | 2026-05-19

---

# Domain 2: Visual Content

## 2.1 Procedural Sources

**Level:** PRIMARY
**Current UI:** Sources Browser (Browser Panel tab "Sources") + Clip Inspector section 8.6 (Source Parameters)
**Sub-features:**

108 procedural sources across 17 categories, 628 total parameters. Each source is a GLSL 410 fragment shader rendered on a fullscreen quad. All source params normalized [0,1]. All sources receive 42 audio uniforms (u_rms, u_bass, u_beatPhase, etc.) automatically.

### Category: Noise (3 sources, 12 params)

- Perlin Noise [PRESENTATION-HIDDEN] — Noise. Params: Scale, Speed, Octaves, Color Shift (4)
- Plasma [PRESENTATION-HIDDEN] — Noise. Params: Speed, Complexity, Color Cycle, Intensity (4)
- Voronoi [PRESENTATION-HIDDEN] — Noise. Params: Scale, Speed, Edge Width, Color Mode (4)

### Category: Fractal — 2D (7 sources, 52 params)

- Kaleidoscopic Fractal [PRESENTATION-HIDDEN] — Fractal. Params: Iterations, Fold Angle, Zoom, Rotation, Color Shift, Palette (6)
- Mandelbrot / Julia [PRESENTATION-HIDDEN] — Fractal. Params: Dive Speed, Location, Zoom, Center X, Center Y, Julia Mix, Max Iterations, Power, Color Speed, Color Shift, Palette (11)
- Julia Set [PRESENTATION-HIDDEN] — Fractal. Params: Dive Speed, Location, C Real, C Imaginary, Zoom, Iterations, Color Speed, Color Shift, Palette (9)
- Burning Ship [PRESENTATION-HIDDEN] — Fractal. Params: Dive Speed, Location, Center X, Center Y, Zoom, Iterations, Color Speed, Color Shift, Palette (9)
- Newton Fractal [PRESENTATION-HIDDEN] — Fractal. Params: Dive Speed, Power, Zoom, Damping, Color Shift, Palette (6)
- Sierpinski [PRESENTATION-HIDDEN] — Fractal. Params: Dive Speed, Mode, Zoom, Iterations, Rotation, Color Shift, Palette (7)
- Apollonian Gasket [PRESENTATION-HIDDEN] — Fractal. Params: Dive Speed, Zoom, Iterations, Rotation, Color Shift, Palette (6)

### Category: 3D — Ray-Marched Fractals (8 sources, 109 params)

- Mandelbulb [PRESENTATION-HIDDEN] — 3D. Params: Power, Iterations, Angle X, Angle Y, Zoom, Speed, Detail, Color Shift, Cross Section, Slice Count, Glow, Trail Distance, Trail Fade, Slice Distance, Feedback, Palette (16)
- Menger Sponge [PRESENTATION-HIDDEN] — 3D. Params: Iterations, Angle X, Angle Y, Zoom, Speed, Twist, Color Shift, Cross Section, Slice Count, Trail Distance, Trail Fade, Slice Distance, Feedback, Palette (14)
- Kaleidoscopic IFS [PRESENTATION-HIDDEN] — 3D. Params: Scale, Iterations, Fold Type, Angle X, Angle Y, Zoom, Speed, Offset, Color Shift, Cross Section, Slice Count, Trail Distance, Trail Fade, Slice Distance, Feedback, Palette (16)
- Julia Set 3D [PRESENTATION-HIDDEN] — 3D. Params: Location, C Real, C Imaginary, Angle X, Angle Y, Zoom, Speed, Iterations, Color Shift, Cross Section, Slice Count, Glow, Trail Distance, Trail Fade, Slice Distance, Feedback, Palette (17)
- Burning Ship 3D [PRESENTATION-HIDDEN] — 3D. Params: Power, Angle X, Angle Y, Zoom, Speed, Color Shift, Cross Section, Slice Count, Glow, Trail Distance, Trail Fade, Slice Distance, Feedback, Palette (14)
- Newton 3D [PRESENTATION-HIDDEN] — 3D. Params: Power, Angle X, Angle Y, Zoom, Speed, Damping, Height, Color Shift, Trail Distance, Trail Fade, Slice Count, Slice Distance, Feedback, Palette (14)
- Sierpinski Tetrahedron [PRESENTATION-HIDDEN] — 3D. Params: Iterations, Angle X, Angle Y, Zoom, Speed, Color Shift, Cross Section, Slice Count, Trail Distance, Trail Fade, Slice Distance, Feedback, Palette (13)
- Apollonian 3D [PRESENTATION-HIDDEN] — 3D. Params: Scale, Iterations, Angle X, Angle Y, Zoom, Speed, Color Shift, Cross Section, Slice Count, Trail Distance, Trail Fade, Slice Distance, Feedback, Palette (14)

### Category: 3D — Torus / Tunnel (8 sources, 110 params)

Shared torus controls (12 params each): Orbit, Tilt, Speed, Zoom, Lens Shape, Lens Rotate, Depth Fade, Tube Radius, Pinch, Heart, Shading, Color Shift. Plus per-source unique params.

- Striped Torus [PRESENTATION-HIDDEN] — 3D. Unique: Stripe Count, Twist + 12 shared (14)
- Spiral Vortex [PRESENTATION-HIDDEN] — 3D. Unique: Twist, Stripe Count + 12 shared (14)
- Checker Torus [PRESENTATION-HIDDEN] — 3D. Unique: Grid U, Grid V + 12 shared (14)
- Ribbed Vortex [PRESENTATION-HIDDEN] — 3D. Unique: Ridge Count, Color Mix + 12 shared (14)
- Wormhole Tunnel [PRESENTATION-HIDDEN] — 3D. Unique: Warp + 12 shared (13)
- Twisted Torus [PRESENTATION-HIDDEN] — 3D. Unique: Twist, Stripe Count + 12 shared (14)
- Wormhole [PRESENTATION-HIDDEN] — 3D. Unique: Warp, Glow + 12 shared (14)
- Torus Hole [PRESENTATION-HIDDEN] — 3D. Unique: Stripe Count, Twist, Stripe Angle, Stripe Scale, Stripe Width, Phi Offset, Theta Offset + Camera(4) + Lens(3) + Tube Radius + Pinch + Heart + Shading + Color Shift (20)

### Category: 3D — Other (8 sources, 42 params)

- Spiral Tunnel [PRESENTATION-HIDDEN] — 3D. Params: Speed, Arms, Depth, Twist, Color Shift (5)
- Crystal Cavern [PRESENTATION-HIDDEN] — 3D. Params: Speed, Crystal Size, Reflectivity, Light Color, Fog Density, Complexity (6)
- Infinite Corridor [PRESENTATION-HIDDEN] — 3D. Params: Speed, Width, Wall Pattern, Light Spacing, Light Intensity, Color (6)
- Orbit Chamber [PRESENTATION-HIDDEN] — 3D. Params: Object Count, Object Type, Orbit Speed, Orbit Radius, Material, Light Orbit, Color Shift (7)
- Scroll Plane [PRESENTATION-HIDDEN] — 3D. Params: Speed X, Speed Y, Scale, Warp, Color Shift (5)
- Rotating Cube Map [PRESENTATION-HIDDEN] — 3D. Params: Rotation X, Rotation Y, Scale, Light, Color Shift (5)
- Dual Plane Drift [PRESENTATION-HIDDEN] — 3D. Params: Speed, Rotation, Distance, Scale, Color Shift (5)
- DNA Helix [PRESENTATION-HIDDEN] — 3D. Params: Speed, Zoom, Glow, Color Shift (4)

### Category: Geometric (11 sources, 60 params)

- Geometric Tunnel [PRESENTATION-HIDDEN] — Geometric. Params: Speed, Segments, Twist, Color Shift (4)
- Color Gradient [PRESENTATION-HIDDEN] — Geometric. Params: Angle, Speed, Color 1, Color 2 (4)
- Shape Generator [PRESENTATION-HIDDEN] — Geometric. Params: Shape, Size, Rotation, Outline, Color Shift (5)
- Infinite Zoom [PRESENTATION-HIDDEN] — Geometric. Params: Speed, Layers, Rotation, Color Shift (4)
- Moire Interference [PRESENTATION-HIDDEN] — Geometric. Params: Pattern, Frequency, Offset X, Offset Y, Rotation, Zoom, Color Shift (7)
- Astral Grid [PRESENTATION-HIDDEN] — Geometric. Params: Grid Size, Scroll Speed, Tilt, Warp, Glow, Horizon Color (6)
- Radial Burst [PRESENTATION-HIDDEN] — Geometric. Params: Ray Count, Length, Rotation, Width, Taper, Glow, Color Shift (7)
- Hex Grid [PRESENTATION-HIDDEN] — Geometric. Params: Cell Size, Pattern, Fill, Edge Width, Rotation, Color Mode (6)
- Sacred Geometry [PRESENTATION-HIDDEN] — Geometric. Params: Pattern, Rotation, Breathe, Line Width, Glow, Reveal, Color Shift (7)
- Radar Sweep [PRESENTATION-HIDDEN] — Geometric. Params: Speed, Decay, Grid, Color Shift (4)
- Dot Matrix Wave [PRESENTATION-HIDDEN] — Geometric. Params: Density, Speed, Damping, Dot Size, Color, Sources (6)

### Category: Pattern (8 sources, 38 params)

- Checkerboard [PRESENTATION-HIDDEN] — Pattern. Params: Columns, Rows, Color 1 Hue, Color 2 Hue (4)
- Line Pattern [PRESENTATION-HIDDEN] — Pattern. Params: Count, Width, Rotation, Speed, Color Shift (5)
- Concentric Rings [PRESENTATION-HIDDEN] — Pattern. Params: Count, Spacing, Width, Rotation, Color Shift (5)
- Sine Oscillator [PRESENTATION-HIDDEN] — Pattern. Params: Waves, Frequency, Amplitude, Modulation, Thickness, Color Shift (6)
- Spiral Pattern [PRESENTATION-HIDDEN] — Pattern. Params: Arms, Zoom, Speed, Distortion, Color Shift (5)
- Terrain Lines [PRESENTATION-HIDDEN] — Pattern. Params: Height, Lines, Jagginess, Speed, Tilt, Color Shift (6)
- Bump Light [PRESENTATION-HIDDEN] — Pattern. Params: Light X, Light Y, Intensity, Bumps, Color Shift (5)
- Glitch Grid [PRESENTATION-HIDDEN] — Pattern. Params: Grid Size, Chaos, Flicker, Color Shift (4)

### Category: Lines (11 sources, 62 params)

- Zigzag Lines [PRESENTATION-HIDDEN] — Lines. Params: Count, Amplitude, Speed, Thickness, Color Shift (5)
- Star Burst [PRESENTATION-HIDDEN] — Lines. Params: Rays, Thickness, Speed, Taper, Color Shift (5)
- Polygon Lines [PRESENTATION-HIDDEN] — Lines. Params: Sides, Size, Thickness, Layers, Speed, Color Shift (6)
- Waveform Lines [PRESENTATION-HIDDEN] — Lines. Params: Waveform, Frequency, Amplitude, Count, Thickness, Speed, Color Shift (7)
- Lissajous [PRESENTATION-HIDDEN] — Lines. Params: Ratio X, Ratio Y, Phase, Thickness, Speed, Color Shift (6)
- Spirograph [PRESENTATION-HIDDEN] — Lines. Params: Inner Radius, Offset, Thickness, Speed, Color Shift (5)
- Angular Grid [PRESENTATION-HIDDEN] — Lines. Params: Angle, Count, Thickness, Symmetry, Speed, Color Shift (6)
- Fractal Tree [PRESENTATION-HIDDEN] — Lines. Params: Branches, Angle, Depth, Thickness, Speed, Color Shift (6)
- Laser Scan [PRESENTATION-HIDDEN] — Lines. Params: Beams, Speed, Thickness, Spread, Color Shift (5)
- Moire Lines [PRESENTATION-HIDDEN] — Lines. Params: Density, Angle Offset, Speed, Layers, Color Shift (5)
- Line Generator [PRESENTATION-HIDDEN] — Lines. Params: Pattern, Count, Thickness, Speed, Feedback, Feedback Zoom, Feedback Rotation, Feedback Decay, Color Shift (9)

### Category: Wireframe (7 sources, 63 params)

All share 9 params: Shape, Density, Rotation X, Rotation Y, Rotation Z, Thickness, Perspective, Glow, Color Shift.

- Wireframe Sphere [PRESENTATION-HIDDEN] — Wireframe. Params: (9)
- Wireframe Torus [PRESENTATION-HIDDEN] — Wireframe. Params: (9)
- Wireframe Cube [PRESENTATION-HIDDEN] — Wireframe. Params: (9)
- Wireframe Cylinder [PRESENTATION-HIDDEN] — Wireframe. Params: (9)
- Wireframe Cone [PRESENTATION-HIDDEN] — Wireframe. Params: (9)
- Wireframe Icosahedron [PRESENTATION-HIDDEN] — Wireframe. Params: (9)
- Wireframe Wolf [PRESENTATION-HIDDEN] — Wireframe. Params: (9)

### Category: Audio-Visual (9 sources, 42 params)

- Audio Waveform [MINIMAL-IN-PRESENTATION] — Audio-Visual. Params: Style, Thickness, Glow, Color Shift (4)
- Spectrum Landscape [MINIMAL-IN-PRESENTATION] — Audio-Visual. Params: Height Scale, Camera Angle, Color Mode, Glow, Smoothing (5)
- Chromatic Ring [MINIMAL-IN-PRESENTATION] — Audio-Visual. Params: Ring Width, Glow, Rotation, Ripple (4)
- Band Tower [MINIMAL-IN-PRESENTATION] — Audio-Visual. Params: Shape, Spacing, Reflection, Color Mode, Smoothing, 3D Rotation (6)
- Timbral Nebula [MINIMAL-IN-PRESENTATION] — Audio-Visual. Params: Particle Count, Spread, Trail Length, Color Source, Glow, Sensitivity (6)
- Structural Landscape [MINIMAL-IN-PRESENTATION] — Audio-Visual. Params: Terrain Scale, History Length, Drama, Color Palette, Fog, Camera Height (6)
- Cymatics [MINIMAL-IN-PRESENTATION] — Audio-Visual. Params: Resonance, Damping, Color Shift (3)
- Spectral Waterfall [MINIMAL-IN-PRESENTATION] — Audio-Visual. Params: Scroll Speed, Color Mode, Log Scale (3)
- Spectral Ring [MINIMAL-IN-PRESENTATION] — Audio-Visual. Params: Radius, Thickness, Glow, Rotation, Color Shift (5)

### Category: Nature (6 sources, 26 params)

- Reaction-Diffusion [PRESENTATION-HIDDEN] — Nature. Stateful (ping-pong FBOs). Params: Feed Rate, Kill Rate, Diffusion A, Diffusion B (4)
- Cellular Automata [PRESENTATION-HIDDEN] — Nature. Stateful (ping-pong FBOs). Params: Mode, Birth Low, Birth High, Survival Low, Color Shift (5)
- Fire Wall [PRESENTATION-HIDDEN] — Nature. Params: Height, Turbulence, Speed, Temperature, Density, Wind (6)
- Water Caustics [PRESENTATION-HIDDEN] — Nature. Params: Complexity, Speed, Brightness, Color, Distortion, Scale (6)
- Electric Arc [PRESENTATION-HIDDEN] — Nature. Params: Arc Count, Chaos, Thickness, Branches, Glow, Color (6)
- Fire [PRESENTATION-HIDDEN] — Nature. Params: Height, Turbulence, Speed, Color Shift (4)

### Category: Organic (1 source, 5 params)

- Metaballs [PRESENTATION-HIDDEN] — Organic. Params: Count, Size, Speed, Blend, Color Shift (5)

### Category: Utility (2 sources, 11 params)

- Solid Color [PROGRAMMING-ONLY] — Utility. Params: Red, Green, Blue (3)
- Strobe Light [MINIMAL-IN-PRESENTATION] — Utility. Params: Frequency, Fade, Color 1 Red, Color 1 Green, Color 1 Blue, Color 2 Red, Color 2 Green, Color 2 Blue (8)

### Category: Text (2 sources, 12 params)

- Scrolling Text Wall [PRESENTATION-HIDDEN] — Text. Params: Speed, Density, Size, Color Shift (4)
- Text Animator [PRESENTATION-HIDDEN] — Text. Params: Font Size, Text Red, Text Green, Text Blue, Animation, Speed, Columns, Spacing (8)

### Category: Math (8 sources, 46 params)

- Lissajous Weaver [PRESENTATION-HIDDEN] — Math. Params: Frequency X, Frequency Y, Phase, Decay, Harmonics, Thickness, Color Shift (7)
- Fermat Spiral Garden [PRESENTATION-HIDDEN] — Math. Params: Count, Divergence Angle, Shape, Growth, Pulse, Color Spread (6)
- Hyperbolic Tiling [PRESENTATION-HIDDEN] — Math. Params: Polygon Sides, Vertex Order, Rotation, Zoom, Color Scheme, Line Width (6)
- Penrose Pulse [PRESENTATION-HIDDEN] — Math. Params: Generation, Ripple Speed, Color Mode, Edge Glow, Morph (5)
- Superformula [PRESENTATION-HIDDEN] — Math. Params: Symmetry, Roundness, Concavity, Blobbiness, Size, Color Shift (6)
- Truchet Labyrinth [PRESENTATION-HIDDEN] — Math. Params: Density, Style, Thickness, Glow, Color Shift (5)
- Rose Curves [PRESENTATION-HIDDEN] — Math. Params: Petals, Thickness, Layers, Glow, Color Shift (5)
- Fibonacci Spiral [PRESENTATION-HIDDEN] — Math. Params: Elements, Size, Spread, Glow, Color Shift (5)

### Category: Simulation (3 sources, 18 params)

All stateful (ping-pong FBOs).

- Strange Attractor [PRESENTATION-HIDDEN] — Simulation. Params: Attractor Type, Speed, Trail Length, Rotation, Glow, Color Mode (6)
- Gravity Well [PRESENTATION-HIDDEN] — Simulation. Params: Particle Density, Gravity, Scatter, Trail Length, Wells, Color Mode (6)
- Fluid Dynamics [PRESENTATION-HIDDEN] — Simulation. Params: Viscosity, Diffusion, Injection Radius, Color Mode, Curl, Decay (6)

### Category: Particle (3 sources, 12 params)

- Lightning Storm [PRESENTATION-HIDDEN] — Particle. Params: Intensity, Branches, Glow, Color Shift (4)
- Starfield [PRESENTATION-HIDDEN] — Particle. Params: Speed, Density, Streak, Color Shift (4)
- Particle Nebula [PRESENTATION-HIDDEN] — Particle. Params: Density, Scale, Speed, Color Shift (4)

### Category: Lighting (1 source, 6 params)

- Laser Scanner [PRESENTATION-HIDDEN] — Lighting. Params: Pattern, Beam Count, Color, Speed, Spread, Flicker (6)

### Category: Routing (1 source, 1 param)

- Layer Router [PROGRAMMING-ONLY] — Routing. Routes another layer's output as source content. Params: Source Layer (1)

**Total procedural sources: 108** (101 direct registerSource + 7 via registerWireframe helper)
**Total procedural source parameters: 628**

**Parameters:** All 628 source parameters listed above
**Bindings:** All source parameters are signal-connectable via UniversalParamControl (can bind to audio features, oscillators, envelopes, macros, BPM sync, clip position)
**Programming mode:** Sources Browser shows all 109 sources in 17 collapsible categories with search. Clip Inspector section 8.6 shows per-source parameter sliders.
**Presentation mode:** HIDDEN — source selection and parameter tuning are programming activities. During performance, sources are pre-configured and triggered via clip cells or autopilot.
**Dependencies:** OpenGL 4.1 Core, GLSL 410, FeatureSnapshot (audio uniforms), ShaderManager

---

## 2.2 MilkDrop / projectM Visualizer

**Level:** PRIMARY
**Current UI:** MilkDrop Browser (Browser Panel tab "MilkDrop") + VJ Clip mode in Deck Grid
**Sub-features:**
  - Preset Library [PRESENTATION-HIDDEN] — ~9,800 MilkDrop presets via libprojectM-4. 4 sub-tabs: Curated (30), Favorites, Recent (last 20), All (searchable)
  - Jukebox Mode [ALWAYS-VISIBLE] — Auto-advance through presets with configurable timing (4/8/16/32 beats or 10/30/60 seconds) and blend crossfade. Pool selection: All / Curated / Favorites. Play/Stop, navigation (</>/?/Lock)
  - VJ Clip Mode [ALWAYS-VISIBLE] — Click to preview, drag single preset to deck cell as a clip
  - Playlist Mode [PRESENTATION-HIDDEN] — Multi-select presets, drag group to create cycling playlist clip
  - Audio Feed [ALWAYS-VISIBLE] — PCM audio buffer (2048 samples) fed to projectM each frame for audio-reactive visualization
  - Preset Selector [PRESENTATION-HIDDEN] — Audio-driven preset selection system
  - GL State Save/Restore [ALWAYS-VISIBLE] — projectM manages its own shaders and feedback loops; Audio-DNA saves/restores full GL state around each render call
**Parameters:** Jukebox timing, Jukebox blend, Jukebox pool, preset lock
**Bindings:** Next/Prev/Random preset via keyboard/MIDI bindings. Lock toggle via binding.
**Programming mode:** Full MilkDrop Browser with all 4 sub-tabs, search, drag-to-deck
**Presentation mode:** MINIMAL — Jukebox auto-play or pre-loaded VJ clips. Next/Prev/Random/Lock via bindings.
**Dependencies:** libprojectM-4 (optional, compile flag `AUDIODNA_HAS_PROJECTM`), MilkDrop preset directory

---

## 2.3 Video Playback

**Level:** PRIMARY
**Current UI:** Clip Inspector section 8.3 (Transport) + section 8.7 (Video)
**Sub-features:**
  - FFmpeg Decode Pipeline [ALWAYS-VISIBLE] — Opens video via avformat_open_input, finds video stream, decodes via avcodec_send_packet/receive_frame, converts via sws_scale, uploads to GL texture
  - Codec Support [ALWAYS-VISIBLE] — H.264, H.265, ProRes, HAP (including HAP Alpha), MJPEG, and any codec supported by FFmpeg
  - Container Support [ALWAYS-VISIBLE] — MP4, MOV, AVI, MKV, WebM
  - Transport Modes [ALWAYS-VISIBLE] — Timeline (time-based) or BPM Sync (beat-locked). In BPM Sync, beatDivision x BPM determines playback speed
  - Loop Modes [ALWAYS-VISIBLE] — Loop, Ping-Pong, One Shot
  - In/Out Points [MINIMAL-IN-PRESENTATION] — Draggable on timeline [0,1]
  - Speed Control [MINIMAL-IN-PRESENTATION] — 0-4x playback rate, divide/multiply buttons, reverse toggle
  - Content Beats [PROGRAMMING-ONLY] — BPM Sync only: how many beats the video content represents (for exact timing of authored content)
  - Beats/Cycle [PROGRAMMING-ONLY] — BPM Sync only: 1/4 to 16 beats per playback cycle
  - Cuepoints [ALWAYS-VISIBLE] — 8 named cuepoints per clip. Set at current playhead, trigger to jump. Clear via right-click.
  - Trigger Modes [PROGRAMMING-ONLY] — Restart / Continue / Relative (how clip responds when re-triggered)
**Parameters:** Speed (0-4x), Duration (0.1-300s timeline / 1-64 beats BPM sync), In Point, Out Point, Content Beats, Beats/Cycle, Opacity, Width, Height, Blend Mode, Alpha Type, R/G/B/A channel toggles
**Bindings:** Play/Pause/Stop, cuepoint triggers, speed via MIDI CC
**Programming mode:** Full transport controls, in/out editing, BPM sync configuration, content beats
**Presentation mode:** MINIMAL — transport runs automatically via autopilot or bindings. Speed and cuepoints accessible via bindings.
**Dependencies:** FFmpeg 8.0 (libavformat, libavcodec, libavutil, libswscale)

---

## 2.4 Image Sequences

**Level:** SECONDARY
**Current UI:** Clip Inspector section 8.3 (Transport) — Images/Sec slider appears for ImageSequence type
**Sub-features:**
  - Multi-Image Loading [PROGRAMMING-ONLY] — Load folder of images as a clip
  - Configurable FPS [MINIMAL-IN-PRESENTATION] — Images/Sec slider controls playback rate
  - BPM Sync Playback [ALWAYS-VISIBLE] — Same BPM sync transport as video: beat division controls cycle speed
  - Loop Modes [ALWAYS-VISIBLE] — Loop, Ping-Pong, One Shot (shared with video transport)
**Parameters:** Images/Sec (FPS), same transport parameters as video (speed, duration, in/out, loop mode, BPM sync)
**Bindings:** Same transport bindings as video playback
**Programming mode:** Load image folder, configure FPS and BPM sync
**Presentation mode:** HIDDEN — pre-configured during setup, runs automatically
**Dependencies:** JUCE image loading (PNG, JPEG, BMP, GIF, TIFF)

---

## 2.5 Media Management

**Level:** UTILITY
**Current UI:** Composition menu (Menu Bar)
**Sub-features:**
  - Collect Media [PROGRAMMING-ONLY] — Menu > Composition > Collect Media. Packages composition + all referenced media files into one folder for portability.
  - Relocate Files [PROGRAMMING-ONLY] — Menu > Composition > Relocate Files. Finds and relinks missing media files when a composition is opened on a different machine or after moving files.
**Parameters:** None (menu actions)
**Bindings:** None
**Programming mode:** Available via menu
**Presentation mode:** HIDDEN
**Dependencies:** Filesystem access, JUCE File utilities

---

## 2.6 Camera Input [GHOST]

**Level:** SECONDARY
**Current UI:** NO UI — code only. `Clip::MediaType::Camera` enum value exists. `cameraDeviceIndex` field in Clip model (-1 = none). Not wired to UI.
**Sub-features:**
  - Camera Device Selection [GHOST] — `cameraDeviceIndex` field defined but no device enumeration UI
  - Live Feed as Clip Source [GHOST] — MediaType::Camera defined in enum but no rendering path implemented
**Parameters:** cameraDeviceIndex (integer, model only)
**Bindings:** None
**Programming mode:** Not accessible
**Presentation mode:** Not accessible
**Dependencies:** Would require JUCE CameraDevice or platform-specific capture API
**Note:** Boris decided INCLUDE in design — needs UI and rendering implementation.

---

# Domain 3: Effects & Processing

## 3.1 Visual Effects Library

**Level:** PRIMARY
**Current UI:** FX Browser (Browser Panel tab "FX") + Clip Inspector section 8.9 (Effects Stack) + Layer Inspector section 9.12 + Composition Inspector section 10.8
**Sub-features:**

135 effects across 11 categories, 333 total parameters. Each effect is a GLSL 410 fragment shader. All effect params normalized [0,1] and signal-connectable. Effects applied via ping-pong FBO chain.

### Category: Warp (27 effects, 67 params)

- Ripple [PRESENTATION-HIDDEN] — Warp. Params: intensity, freq, speed (3)
- Bulge [PRESENTATION-HIDDEN] — Warp. Params: amount, center_x, center_y (3)
- Wave [PRESENTATION-HIDDEN] — Warp. Params: amplitude, frequency, direction (3)
- Liquid [PRESENTATION-HIDDEN] — Warp. Params: viscosity, turbulence (2)
- Kaleidoscope [PRESENTATION-HIDDEN] — Warp. Params: segments, rotation (2)
- Fisheye [PRESENTATION-HIDDEN] — Warp. Params: amount (1)
- Swirl [PRESENTATION-HIDDEN] — Warp. Params: amount, radius (2)
- Polar Coords [PRESENTATION-HIDDEN] — Warp. Params: amount (1)
- Twirl [PRESENTATION-HIDDEN] — Warp. Params: amount, radius (2)
- Shear [PRESENTATION-HIDDEN] — Warp. Params: x, y (2)
- Elastic Bounce [PRESENTATION-HIDDEN] — Warp. Params: amount, freq (2)
- Ripple Pond [PRESENTATION-HIDDEN] — Warp. Params: intensity, freq (2)
- Diamond Distort [PRESENTATION-HIDDEN] — Warp. Params: size, amount (2)
- Barrel Distort [PRESENTATION-HIDDEN] — Warp. Params: amount (1)
- Sine Grid [PRESENTATION-HIDDEN] — Warp. Params: freq, amount (2)
- Glitch Displace [PRESENTATION-HIDDEN] — Warp. Params: amount, speed (2)
- Quad Mirror [PRESENTATION-HIDDEN] — Warp. Params: center x, center y (2)
- Flip [PRESENTATION-HIDDEN] — Warp. Params: horizontal, vertical (2)
- Warp Field [PRESENTATION-HIDDEN] — Warp. Params: amount, frequency, speed (3)
- Slide Wrap [PRESENTATION-HIDDEN] — Warp. Params: x, y (2)
- Tile Grid [PRESENTATION-HIDDEN] — Warp. Params: columns, rows, offset, zoom (4)
- Spot Zoom [PRESENTATION-HIDDEN] — Warp. Params: center x, center y, size, zoom, shape, background (6)
- Bendoscope [PRESENTATION-HIDDEN] — Warp. Params: divisions, bend, rotation (3)
- UV Remap [PRESENTATION-HIDDEN] — Warp. Params: amount, scale, speed (3)
- Liquid Morph [PRESENTATION-HIDDEN] — Warp. Params: viscosity, amount, scale (3)
- Zoom Warp [PRESENTATION-HIDDEN] — Warp. Params: speed, rotation, center x, center y (4)
- Density Wave [PRESENTATION-HIDDEN] — Warp (audio). Params: amount, direction, wavelength (3)

### Category: Color (31 effects, 64 params)

- Hue Shift [PRESENTATION-HIDDEN] — Color. Params: amount (1)
- Saturation [PRESENTATION-HIDDEN] — Color. Params: amount (1)
- Brightness [PRESENTATION-HIDDEN] — Color. Params: amount (1)
- Duotone [PRESENTATION-HIDDEN] — Color. Params: color1_r, color1_g, color1_b, color2_r, color2_g, color2_b, mix (7)
- Chromatic Aberration [PRESENTATION-HIDDEN] — Color. Params: amount, angle (2)
- Invert [PRESENTATION-HIDDEN] — Color. Params: amount (1)
- Posterize [PRESENTATION-HIDDEN] — Color. Params: levels (1)
- Color Shift [PRESENTATION-HIDDEN] — Color. Params: red, green, blue (3)
- Thermal [PRESENTATION-HIDDEN] — Color. Params: amount (1)
- Contrast [PRESENTATION-HIDDEN] — Color. Params: contrast, balance (2)
- Sepia [PRESENTATION-HIDDEN] — Color. Params: amount (1)
- Cross Process [PRESENTATION-HIDDEN] — Color. Params: amount (1)
- Split Tone [PRESENTATION-HIDDEN] — Color. Params: shadow_hue, highlight_hue, amount (3)
- Color Halftone [PRESENTATION-HIDDEN] — Color. Params: scale, amount (2)
- Dither [PRESENTATION-HIDDEN] — Color. Params: levels, amount (2)
- Heat Map [PRESENTATION-HIDDEN] — Color. Params: amount (1)
- Selective Color [PRESENTATION-HIDDEN] — Color. Params: hue, range (2)
- Film Grain [PRESENTATION-HIDDEN] — Color. Params: amount, size (2)
- Gamma Levels [PRESENTATION-HIDDEN] — Color. Params: black, white, gamma (3)
- Solarize [PRESENTATION-HIDDEN] — Color. Params: threshold, amount (2)
- Greyscale [PRESENTATION-HIDDEN] — Color. Params: method, amount (2)
- Threshold [PRESENTATION-HIDDEN] — Color. Params: level, amount (2)
- Exposure [PRESENTATION-HIDDEN] — Color. Params: amount (1)
- Vibrance [PRESENTATION-HIDDEN] — Color. Params: amount (1)
- Auto Mask [PRESENTATION-HIDDEN] — Color. Params: threshold, softness, invert (3)
- Chroma Key [PRESENTATION-HIDDEN] — Color. Params: hue, tolerance, softness, amount (4)
- Palette Remap [PRESENTATION-HIDDEN] — Color. Params: palette, cycle, amount (3)
- Color Grade [PRESENTATION-HIDDEN] — Color. Uses LUT texture (LUTLoader). Params: amount (1)
- Pitch Chromatic Shift [PRESENTATION-HIDDEN] — Color (audio). Params: amount, mode, saturation (3)
- Key Palette [PRESENTATION-HIDDEN] — Color (audio). Params: amount, brightness, saturation (3)
- Chroma Dissolve [PRESENTATION-HIDDEN] — Color (audio). Params: amount, softness (2)

### Category: Glitch (15 effects, 40 params)

- Pixel Scatter [PRESENTATION-HIDDEN] — Glitch. Params: amount, seed (2)
- RGB Split [PRESENTATION-HIDDEN] — Glitch. Params: amount, angle (2)
- Block Glitch [PRESENTATION-HIDDEN] — Glitch. Params: intensity, block_size (2)
- Scanlines [PRESENTATION-HIDDEN] — Glitch. Params: intensity, frequency (2)
- Digital Rain [PRESENTATION-HIDDEN] — Glitch. Params: intensity, speed (2)
- Noise [PRESENTATION-HIDDEN] — Glitch. Params: amount, speed (2)
- Mirror [PRESENTATION-HIDDEN] — Glitch. Params: horizontal, vertical (2)
- Pixelate [PRESENTATION-HIDDEN] — Glitch. Params: size (1)
- Pixel Explosion [PRESENTATION-HIDDEN] — Glitch. Params: force, decay, center x, center y (4)
- Color Flash [PRESENTATION-HIDDEN] — Glitch. Params: intensity, red, green, blue, decay (5)
- Fragment Burst [PRESENTATION-HIDDEN] — Glitch. Params: copies, spread, rotation, scale (4)
- Signal Destroy [PRESENTATION-HIDDEN] — Glitch. Params: amount, speed, mode (3)
- Rhythm Slice [PRESENTATION-HIDDEN] — Glitch (audio). Params: amount, slices, sync (3)
- Data Corruption [PRESENTATION-HIDDEN] — Glitch. Params: amount, block size, color damage (3)
- Glitch Sort [PRESENTATION-HIDDEN] — Glitch. Params: amount, threshold, direction (3)

### Category: Blur / Post (10 effects, 21 params)

- Gaussian Blur [PRESENTATION-HIDDEN] — Blur. Params: radius (1)
- Zoom Blur [PRESENTATION-HIDDEN] — Blur. Params: amount, center_x, center_y (3)
- Shake [PRESENTATION-HIDDEN] — Blur. Params: amount_x, amount_y (2)
- Vignette [PRESENTATION-HIDDEN] — Blur. Params: intensity, softness (2)
- Motion Blur [PRESENTATION-HIDDEN] — Blur. Params: amount, angle (2)
- Glow [PRESENTATION-HIDDEN] — Blur. Params: amount, threshold (2)
- Edge Detect [PRESENTATION-HIDDEN] — Blur. Params: amount (1)
- Sharpen [PRESENTATION-HIDDEN] — Blur. Params: amount, radius (2)
- Edge Blur [PRESENTATION-HIDDEN] — Blur. Params: threshold, amount (2)
- Drop Shadow [PRESENTATION-HIDDEN] — Blur. Params: offset x, offset y, blur, opacity (4)

### Category: 3D / Depth (9 effects, 20 params)

- Perspective Tilt [PRESENTATION-HIDDEN] — 3D. Params: tilt_x, tilt_y (2)
- Cylinder Wrap [PRESENTATION-HIDDEN] — 3D. Params: amount, axis (2)
- Sphere Wrap [PRESENTATION-HIDDEN] — 3D. Params: amount (1)
- Tunnel [PRESENTATION-HIDDEN] — 3D. Params: speed, radius (2)
- Page Curl [PRESENTATION-HIDDEN] — 3D. Params: amount, radius (2)
- Parallax Layers [PRESENTATION-HIDDEN] — 3D. Params: amount, direction (2)
- Dot Field [PRESENTATION-HIDDEN] — 3D. Params: size, spacing, depth (3)
- Luminance Terrain [PRESENTATION-HIDDEN] — 3D. Params: height, segments, angle (3)
- Voxel Matrix [PRESENTATION-HIDDEN] — 3D. Params: size, height, rotation (3)

### Category: Pattern / Stylization (19 effects, 50 params)

- CRT [PRESENTATION-HIDDEN] — Pattern. Params: curvature, scanline (2)
- VHS [PRESENTATION-HIDDEN] — Pattern. Params: amount, tracking (2)
- ASCII Art [PRESENTATION-HIDDEN] — Pattern. Params: scale, color (2)
- Dot Matrix [PRESENTATION-HIDDEN] — Pattern. Params: scale, amount (2)
- Crosshatch [PRESENTATION-HIDDEN] — Pattern. Params: density, amount (2)
- Emboss [PRESENTATION-HIDDEN] — Pattern. Params: amount, angle (2)
- Oil Paint [PRESENTATION-HIDDEN] — Pattern. Params: radius (1)
- Pencil Sketch [PRESENTATION-HIDDEN] — Pattern. Params: amount, density (2)
- Voronoi Glass [PRESENTATION-HIDDEN] — Pattern. Params: scale, edge (2)
- Cross Stitch [PRESENTATION-HIDDEN] — Pattern. Params: scale, amount (2)
- Night Vision [PRESENTATION-HIDDEN] — Pattern. Params: amount (1)
- Triangulate [PRESENTATION-HIDDEN] — Pattern. Params: size, amount (2)
- Neon Edge [PRESENTATION-HIDDEN] — Pattern. Params: edge, glow, hue, original (4)
- Cartoon Ink [PRESENTATION-HIDDEN] — Pattern. Params: edge width, color steps, ink strength, saturation (4)
- Pop Raster [PRESENTATION-HIDDEN] — Pattern. Params: palette, bands, pattern size, mix (4)
- Brush Strokes [PRESENTATION-HIDDEN] — Pattern. Params: size, angle, flow, amount (4)
- Bump Light [PRESENTATION-HIDDEN] — Pattern. Params: light x, light y, intensity, height (4)
- Monitor Wall [PRESENTATION-HIDDEN] — Pattern. Params: columns, rows, border, glow (4)
- Topographic Lines [PRESENTATION-HIDDEN] — Pattern. Params: amount, levels, thickness, color mode (4)

### Category: Animation (6 effects, 23 params)

- Strobe [ALWAYS-VISIBLE] — Animation. Params: rate, intensity (2)
- Pulse [ALWAYS-VISIBLE] — Animation. Params: amount, speed (2)
- Slit Scan [PRESENTATION-HIDDEN] — Animation. Params: amount, direction (2)
- Transient Flash [ALWAYS-VISIBLE] — Animation (audio). Params: style, intensity, decay (3)
- Point Zoom [ALWAYS-VISIBLE] — Animation (feedback). Params: amount, zoom, rotation, x offset, y offset, decay, hue shift, saturation (8)
- Directional Feedback [ALWAYS-VISIBLE] — Animation (feedback). Params: amount, speed, direction, spread, decay, hue shift (6)

### Category: Time / Temporal (6 effects, 14 params)

- Echo [ALWAYS-VISIBLE] — Time. Temporal (uses u_prev_frame). Params: decay, operator (2)
- Posterize Time [PRESENTATION-HIDDEN] — Time. Temporal (uses u_prev_frame). Params: frame rate, amount (2)
- Freeze [ALWAYS-VISIBLE] — Time. Temporal (uses u_prev_frame). Params: amount (1)
- Screen Split [ALWAYS-VISIBLE] — Time. Uses frame ring buffer (480 frames at 1/4 res). Params: columns, rows, frames per cell, direction (4)
- Frame Stutter [ALWAYS-VISIBLE] — Time. Uses frame ring buffer. Params: depth, stutter (2)
- Channel Delay [PRESENTATION-HIDDEN] — Time. Temporal. Params: red delay, green delay, blue delay (3)

### Category: Audio-Reactive (4 effects, 12 params)

These effects use extended audio uniforms and are Audio-DNA's differentiator.

- Harmonic Displacement [PRESENTATION-HIDDEN] — Audio. Params: amount, smoothing, color mode (3)
- Timbral Mosaic [PRESENTATION-HIDDEN] — Audio. Params: amount, base size, complexity (3)
- Structural Morph [PRESENTATION-HIDDEN] — Audio. Params: intensity, normal style, drop style (3)
- Beat Ripple [PRESENTATION-HIDDEN] — Audio. Params: intensity, decay, count (3)

### Category: Blend / Composite (5 effects, 9 params)

- Double Exposure [PRESENTATION-HIDDEN] — Blend. Params: offset, blend (2)
- Frosted Glass [PRESENTATION-HIDDEN] — Blend. Params: amount, scale (2)
- Prism [PRESENTATION-HIDDEN] — Blend. Params: amount, angle (2)
- Rain on Glass [PRESENTATION-HIDDEN] — Blend. Params: amount, speed (2)
- Hexagonalize [PRESENTATION-HIDDEN] — Blend. Params: scale (1)

### Category: Composite / Cloner (3 effects, 13 params)

- Line Cloner [PRESENTATION-HIDDEN] — Composite. Params: copies, offset x, offset y, scale, rotation (5)
- Radial Cloner [PRESENTATION-HIDDEN] — Composite. Params: copies, radius, rotation, scale (4)
- Cube Scatter [PRESENTATION-HIDDEN] — Composite. Params: grid x, grid y, explode, rotation (4)

**VERIFIED TOTALS: 135 effects, 333 parameters**

Note: The category counts in the FX Browser (per Boris's doc) list slightly different groupings because some effects serve dual purposes. The authoritative grouping is from EffectLibrary.cpp `category` field.

**Parameters:** All 333 effect parameters listed above, plus Dry/Wet per effect, plus Bypass per effect
**Bindings:** All effect parameters are signal-connectable via UniversalParamControl. Bypass can be bound to keyboard/MIDI.
**Programming mode:** FX Browser shows all 135 effects in collapsible category sections with search. Effects Stack in inspector shows all params as signal-connectable sliders.
**Presentation mode:** HIDDEN for effect selection/tuning. Pre-configured effects run automatically. Individual effects can be toggled via bypass bindings.
**Dependencies:** OpenGL 4.1 Core, GLSL 410, ShaderManager, EffectChain (ping-pong FBOs), UniformBridge

---

## 3.2 Effect Chain Architecture

**Level:** PRIMARY
**Current UI:** Clip Inspector section 8.9, Layer Inspector section 9.12, Composition Inspector section 10.8
**Sub-features:**
  - Per-Clip Effect Chain [PRESENTATION-HIDDEN] — Effects applied to individual clip output before keying and layer blend. Each clip can have unlimited effects in ordered chain.
  - Per-Layer Effect Chain [PRESENTATION-HIDDEN] — Effects applied after all clips in a layer are composited, before accumulator blend. Accessed via Layer Inspector.
  - Global Effect Chain [MINIMAL-IN-PRESENTATION] — Effects applied after all layers are composited. Accessed via Composition Inspector. Often the "performance" effects.
  - Ping-Pong FBO Rendering [ALWAYS-VISIBLE] — EffectChain renders by alternating between FBO A and FBO B. Input texture -> Effect 1 -> FBO B -> Effect 2 -> FBO A -> ...
  - Effect Ordering [PROGRAMMING-ONLY] — Drag-reorder effects in the stack. Order determines processing order.
  - Per-Effect Bypass [ALWAYS-VISIBLE] — Toggle [B] button per effect to skip it without removing
  - Per-Effect Dry/Wet [PRESENTATION-HIDDEN] — Mix slider (0-1) blending between original and effected output
  - ISF Shader Import [PROGRAMMING-ONLY] — Menu > Audio-DNA > Import ISF. Parses ISF (Interactive Shader Format) JSON metadata, wraps with compatibility defines, converts to GLSL 410.
**Parameters:** Per effect: Dry/Wet, Bypass. Chain level: order.
**Bindings:** Bypass toggle per effect via keyboard/MIDI
**Programming mode:** Full effect chain editing — add, remove, reorder, configure params, ISF import
**Presentation mode:** MINIMAL — effects run automatically. Bypass toggles via bindings for live control.
**Dependencies:** EffectChain, Effect, UniformBridge, ISFShaderLoader, ping-pong FBOs (effectFBO_A_, effectFBO_B_)

---

## 3.3 Temporal Effects

**Level:** PRIMARY
**Current UI:** FX Browser (Time category) + Effects Stack in inspector
**Sub-features:**
  - Echo / Ghost Trails [ALWAYS-VISIBLE] — Blends current frame with previous frame. Temporal flag = true, uses u_prev_frame texture from per-layer temporal buffer. Params: decay (trail length), operator (fade mode). (2 params)
  - Freeze [ALWAYS-VISIBLE] — Holds current frame, ignoring new input. Temporal. Params: amount. (1 param)
  - Posterize Time / Frame Hold [PRESENTATION-HIDDEN] — Reduces effective frame rate by holding frames. Temporal. Params: frame rate, amount. (2 params)
  - Frame Stutter [ALWAYS-VISIBLE] — Plays back frames from the 480-frame ring buffer with configurable delay. Non-temporal (reads ring buffer). Params: depth, stutter. (2 params)
  - Screen Split [ALWAYS-VISIBLE] — Divides output into grid cells, each showing a different delayed frame from ring buffer. Non-temporal. Params: columns, rows, frames per cell, direction. (4 params)
  - Channel Delay [PRESENTATION-HIDDEN] — Delays individual R/G/B channels by different frame counts. Temporal. Params: red delay, green delay, blue delay. (3 params)
**Parameters:** 14 total (listed above per effect)
**Bindings:** All params signal-connectable. Bypass per effect.
**Programming mode:** Configure temporal effects in effect stack, tune decay/depth params
**Presentation mode:** ALWAYS-VISIBLE for Echo, Freeze, Frame Stutter, Screen Split (core performance tools). Frame Hold and Channel Delay PRESENTATION-HIDDEN.
**Dependencies:** Per-layer temporal buffers (layerTemporalBuffers_ in CompositorEngine), Frame ring buffer (480 frames at 1/4 resolution, ~240MB VRAM at 1080p). Temporal effects require BOTH EffectChain and CompositorEngine render paths.

---

## 3.4 Feedback System (Larsen Loop)

**Level:** PRIMARY
**Current UI:** Layer Inspector section 9.11 (Feedback)
**Sub-features:**
  - Feedback Enable [ALWAYS-VISIBLE] — Toggle per-layer feedback on/off
  - Feedback Presets [MINIMAL-IN-PRESENTATION] — 6 named presets with pre-configured transforms:
    - Zoom In — amount: 0.85, scaleX: 0.97, scaleY: 0.97, rotation: 0, offsetX: 0, offsetY: 0, lumaKey: 0
    - Spiral — amount: 0.80, scaleX: 0.98, scaleY: 0.98, rotation: 2.0, offsetX: 0, offsetY: 0, lumaKey: 0
    - Drift — amount: 0.75, scaleX: 1.0, scaleY: 1.0, rotation: 0, offsetX: 0.01, offsetY: 0, lumaKey: 0
    - Kaleidoscope — amount: 0.90, scaleX: 0.95, scaleY: 0.95, rotation: 5.0, offsetX: 0, offsetY: 0, lumaKey: 0
    - Echo — amount: 0.60, scaleX: 1.0, scaleY: 1.0, rotation: 0, offsetX: 0, offsetY: 0, lumaKey: 0
    - Stretch — amount: 0.85, scaleX: 1.02, scaleY: 0.98, rotation: 0, offsetX: 0, offsetY: 0, lumaKey: 0
  - Feedback Amount [ALWAYS-VISIBLE] — How much previous frame bleeds through (0-1)
  - Feedback Scale X / Scale Y [PRESENTATION-HIDDEN] — Per-frame zoom transform
  - Feedback Rotation [PRESENTATION-HIDDEN] — Per-frame rotation (degrees)
  - Feedback Offset X / Offset Y [PRESENTATION-HIDDEN] — Per-frame drift
  - Feedback Luma Key [PRESENTATION-HIDDEN] — Fade dark areas from feedback loop (0-1)
**Parameters:** 7 per layer: enabled, amount, scaleX, scaleY, rotation, offsetX, offsetY, lumaKey
**Bindings:** All feedback params are signal-connectable (can map audio features to feedback transform)
**Programming mode:** Full parameter editing with preset selection dropdown
**Presentation mode:** MINIMAL — Feedback enable toggle and amount slider. Preset selection via binding.
**Dependencies:** FeedbackProcessor (per-layer FBO pair), ShaderManager (feedback blend shader), FullscreenQuad

---

## 3.5 LUT Color Grading [GHOST]

**Level:** SECONDARY
**Current UI:** NO UI — code only. LUTLoader (src/render/LUTLoader.h, 111 LOC) loads .cube LUT files into GL_TEXTURE_3D. Used by the "Color Grade" effect in the EffectLibrary.
**Sub-features:**
  - LUT File Loading [GHOST] — Parses .cube format files (LUT_3D_SIZE N, N^3 RGB entries), uploads to GL_TEXTURE_3D
  - Color Grade Effect [ALWAYS-VISIBLE] — Registered in EffectLibrary as "Color Grade" (category: color, shader: lut_grade). Params: amount (1). The effect shader IS registered and can be applied, but there is no UI to browse/select/load LUT files.
  - LUT Texture Release [GHOST] — Static method to release GL textures
**Parameters:** Color Grade effect: amount (1). LUT file selection: no UI.
**Bindings:** Color Grade amount is signal-connectable (standard effect param)
**Programming mode:** Color Grade effect can be dragged from FX Browser and its amount adjusted. But no way to load custom .cube LUT files — only the default/embedded LUT applies.
**Presentation mode:** Same as programming (effect functions, LUT selection does not)
**Dependencies:** OpenGL 4.1 (GL_TEXTURE_3D), .cube file format parser
**Note:** Boris decided INCLUDE in design — needs LUT browser/loader UI.

---

# Domain 7: Output & Recording

## 7.1 Display Output

**Level:** PRIMARY
**Current UI:** Output menu (Menu Bar) + Top Bar Output dropdown
**Sub-features:**
  - Fullscreen Output [ALWAYS-VISIBLE] — Menu > Output > Fullscreen on [display]. Creates borderless fullscreen window on selected display via JUCE DocumentWindow. One entry per connected display. Does NOT use macOS native fullscreen (avoids Space creation).
  - Windowed Output [ALWAYS-VISIBLE] — Menu > Output > Windowed. Floating resizable output window.
  - Output Disabled [PROGRAMMING-ONLY] — Menu > Output > Disabled. No external output window.
  - Test Card [PROGRAMMING-ONLY] — Menu > Output > Test Card. Shows calibration pattern for display alignment.
  - Identify Displays [PROGRAMMING-ONLY] — Menu > Output > Identify Displays. Flashes display names on each connected monitor.
  - Output Resolution [PROGRAMMING-ONLY] — Composition Inspector section 10.9. Dropdown: 1920x1080 / 1280x720 / 2560x1440 / 3840x2160.
  - Escape to Close [ALWAYS-VISIBLE] — Pressing Escape closes the output window.
**Parameters:** Output mode (disabled/fullscreen/windowed), target display index, output resolution
**Bindings:** Output mode selectable via keyboard/MIDI binding
**Programming mode:** Full output configuration — display selection, resolution, test card, identify
**Presentation mode:** ALWAYS-VISIBLE — output window is the primary performance output. Selection via top bar dropdown.
**Dependencies:** JUCE DocumentWindow, OpenGL context sharing (renderer output texture shared with output window)

---

## 7.2 Syphon Output (macOS)

**Level:** SECONDARY
**Current UI:** Enabled via compile flag. When active, publishes GL texture automatically.
**Sub-features:**
  - Syphon Server [ALWAYS-VISIBLE] — Publishes rendered composition as a Syphon server named "Audio-DNA". Zero-copy GPU texture sharing via IOSurface.
  - Server Name [PROGRAMMING-ONLY] — Configurable server name (default: "Audio-DNA"). Visible to Syphon clients.
  - Enable/Disable [PROGRAMMING-ONLY] — Toggle publishing without destroying the server. Atomic flag.
  - Client Discovery [ALWAYS-VISIBLE] — Other applications (MadMapper, VDMX, OBS, Resolume) can discover and receive the texture
**Parameters:** Server name, enabled flag
**Bindings:** Enable/disable toggle could be bound (not currently wired)
**Programming mode:** Configure server name, enable/disable
**Presentation mode:** ALWAYS-VISIBLE — runs automatically when enabled, zero user interaction needed
**Dependencies:** Syphon.framework (macOS only, BSD license, must be in /Library/Frameworks/). Compile flag: `-DAUDIODNA_BUILD_SYPHON=ON`. Uses `__has_include(<Syphon/Syphon.h>)` for compile-time detection; compiles as no-op stub if missing. Obj-C++ (.mm files) — macOS only.

---

## 7.3 Syphon Input [GHOST — DEFER]

**Level:** SECONDARY
**Current UI:** NO UI — code exists. SyphonInput class (src/output/SyphonInput.h) wraps SyphonClient. Can list servers, connect to a source, receive textures via IOSurface.
**Sub-features:**
  - Server Discovery [GHOST] — `listServers()` returns available Syphon servers on the system
  - Connect to Source [GHOST] — `connectToServer(appName, serverName)` connects to a specific server
  - Texture Receive [GHOST] — `getLatestTexture()` returns GL texture ID from connected server
  - Disconnect [GHOST] — Clean disconnection from source
**Parameters:** Source app name, server name
**Bindings:** None
**Programming mode:** Not accessible (no UI)
**Presentation mode:** Not accessible
**Dependencies:** Syphon.framework (macOS only)
**Note:** Boris decided DEFER to future. Code exists but will not be included in current design phase.

---

## 7.4 NDI Output [STUB]

**Level:** UTILITY
**Current UI:** NO UI — stub code only. NdiOutput class (src/output/NdiOutput.h) has interface but all methods are no-ops unless `AUDIODNA_HAS_NDI` is defined.
**Sub-features:**
  - NDI Sender [STUB] — Interface defined: init(sourceName), sendFrame(rgba, width, height), shutdown(). All return false / no-op.
  - Network Discovery [STUB] — Would allow NDI-compatible receivers to find Audio-DNA on the network
**Parameters:** Source name (string)
**Bindings:** None
**Programming mode:** Not accessible
**Presentation mode:** Not accessible
**Dependencies:** NDI SDK (not included, must be installed separately from https://ndi.video/download-ndi-sdk/)
**Note:** Not implemented. Stub only.

---

## 7.5 NDI Input [STUB]

**Level:** UTILITY
**Current UI:** NO UI — stub code only. NdiInput class (src/output/NdiInput.h).
**Sub-features:**
  - Source Discovery [STUB] — `listSources()` returns empty vector
  - Connect to Source [STUB] — `connectToSource()` returns false
  - Frame Receive [STUB] — `getLatestFrame()` returns nullptr
**Parameters:** None functional
**Bindings:** None
**Programming mode:** Not accessible
**Presentation mode:** Not accessible
**Dependencies:** NDI SDK (not included)
**Note:** Not implemented. Stub only.

---

## 7.6 Spout Output [STUB]

**Level:** UTILITY
**Current UI:** NO UI — stub code only. SpoutOutput class (src/output/SpoutOutput.h). Windows equivalent of Syphon. All methods are no-ops.
**Sub-features:**
  - Spout Sender [STUB] — Interface defined but all methods are no-ops. `isInitialized()` always returns false.
**Parameters:** Server name (string)
**Bindings:** None
**Programming mode:** Not accessible
**Presentation mode:** Not accessible
**Dependencies:** Spout2 SDK (Windows, not included). On macOS, this class is a no-op.
**Note:** Not implemented. Stub only.

---

## 7.7 Video Recording

**Level:** PRIMARY
**Current UI:** Output menu (Start/Stop Recording) + Record Panel (Browser tab "Record")
**Sub-features:**
  - Real-Time GL Capture [ALWAYS-VISIBLE] — `VideoRecorder::submitFrame()` does glReadPixels from GL framebuffer into rotating CPU-side triple buffer. Zero mutex on GL thread.
  - Triple-Buffered Readback [ALWAYS-VISIBLE] — 3 CPU pixel buffers with atomic index rotation. GL thread writes to one buffer while encoder thread reads another. Frame drops if encoder can't keep up (counted, not prevented).
  - Codec Selection [PROGRAMMING-ONLY] — H.264 (libx264, default, best compatibility), ProRes (prores_ks, best quality for editing), MJPEG (fast encoding, large files)
  - Recording Configuration [PROGRAMMING-ONLY] — Width, height, FPS (default 30), quality (CRF 0-51 for H264, 0-5 for ProRes), container (mp4/mov, auto-selected by codec)
  - Start/Stop Controls [ALWAYS-VISIBLE] — Menu > Output > Start/Stop Recording. Also available in Record Panel.
  - Frame Drop Counting [ALWAYS-VISIBLE] — Tracks dropped frames when encoder can't keep up with render rate. Displayed in Record Panel status.
  - Video Only [ALWAYS-VISIBLE] — Records video only, no audio track. Audio capture is handled separately by Session Recording.
**Parameters:** Codec (H264/ProRes/MJPEG), width, height, FPS, quality/CRF, output directory
**Bindings:** Start/Stop recording via keyboard/MIDI binding
**Programming mode:** Configure codec, resolution, quality before recording. Browse output folder.
**Presentation mode:** MINIMAL — Start/Stop via binding or menu. Status visible in Record Panel.
**Dependencies:** FFmpeg 8.0 (libavformat, libavcodec, libavutil, libswscale). Encoder thread (separate from GL and analysis threads).
**Storage:** ~/Documents/Audio-DNA/Recordings/

---

## 7.8 Session Recording

**Level:** SECONDARY
**Current UI:** Record Panel (Browser tab "Record")
**Sub-features:**
  - Event Capture [ALWAYS-VISIBLE] — Records timestamped events during performance: ParameterChange, ClipTrigger, ColumnTrigger, MacroChange, TransportChange, EffectToggle, CuepointJump. Thread-safe via mutex.
  - Session Playback [PROGRAMMING-ONLY] — Replays recorded events at their timestamps to recreate the entire performance. `advancePlayback(dt)` returns events to fire each frame.
  - Start/Stop Recording [ALWAYS-VISIBLE] — Record button in Record Panel. Atomic flag for state.
  - Save/Load JSON [PROGRAMMING-ONLY] — Export/import session as JSON file. Human-readable format.
  - Event Count Display [ALWAYS-VISIBLE] — Status showing recording state and event count
  - Duration Tracking [ALWAYS-VISIBLE] — Tracks recording duration from start time
**Parameters:** None (records all events automatically when active)
**Bindings:** Start/Stop session recording via keyboard/MIDI binding
**Programming mode:** Full Record Panel: start, stop, playback, save, load, browse output folder
**Presentation mode:** MINIMAL — Start/Stop via binding. Event count visible.
**Dependencies:** JUCE CriticalSection (thread safety), JUCE File (JSON save/load)
**Storage:** JSON files in user-chosen directory

---

## 7.9 Snapshots

**Level:** UTILITY
**Current UI:** Output menu (Snapshot) + keyboard shortcut
**Sub-features:**
  - PNG Capture [ALWAYS-VISIBLE] — One-click capture of current rendered output to PNG file. `Renderer::takeSnapshot()` reads GL framebuffer and saves to configured directory.
  - Configurable Directory [PROGRAMMING-ONLY] — `setSnapshotDir()` sets save location
  - Capture Callback [ALWAYS-VISIBLE] — `onSnapshotTaken` callback fires with file path after save
**Parameters:** Snapshot directory path
**Bindings:** Snapshot trigger via keyboard/MIDI binding
**Programming mode:** Configure snapshot directory
**Presentation mode:** ALWAYS-VISIBLE — one-press capture via binding
**Dependencies:** OpenGL (glReadPixels), JUCE Image/PNG writer
**Storage:** ~/Documents/Audio-DNA/Snapshots/ (default)

---

# Totals Summary

| Domain | Items | Parameters |
|--------|-------|------------|
| Procedural Sources | 108 sources | 628 params |
| MilkDrop/projectM | ~9,800 presets, 3 modes | ~8 config params |
| Video Playback | 1 system | ~15 transport params |
| Image Sequences | 1 system | ~12 transport params |
| Media Management | 2 menu actions | 0 |
| Camera Input [GHOST] | 1 system | 1 (model only) |
| Visual Effects | 135 effects | 333 params |
| Effect Chain Architecture | 3 levels | per-effect: bypass + dry/wet |
| Temporal Effects | 6 effects (subset of 135) | 14 params |
| Feedback System | 6 presets | 7 params per layer |
| LUT Color Grading [GHOST] | 1 loader + 1 effect | 1 param (effect) |
| Display Output | 5 output modes | ~4 config params |
| Syphon Output | 1 server | 2 params |
| Syphon Input [GHOST-DEFER] | 1 client | 2 params |
| NDI Output [STUB] | 1 stub | 1 param |
| NDI Input [STUB] | 1 stub | 0 |
| Spout Output [STUB] | 1 stub | 1 param |
| Video Recording | 3 codecs | ~6 config params |
| Session Recording | 7 event types | 0 (auto-captures) |
| Snapshots | 1 capture system | 1 param (directory) |
