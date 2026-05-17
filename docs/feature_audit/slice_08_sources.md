# Slice 08 — Procedural Sources, Media Sources, ProjectM, Layer Router, Disk Shaders

**Files audited** (line counts):
- `src/sources/SourceRegistry.h` (50 lines)
- `src/sources/SourceRegistry.cpp` (1301 lines)
- `src/sources/ProceduralSource.h` (118 lines)
- `src/sources/ProceduralSource.cpp` (293 lines)
- `src/sources/PresetSelector.h` (55 lines)
- `src/sources/PresetSelector.cpp` (78 lines)
- `src/sources/ProjectMSource.h` (118 lines)
- `src/sources/ProjectMSource.cpp` (301 lines)
- `src/sources/ProjectMPresetManager.h` (91 lines)
- `src/sources/ProjectMPresetManager.cpp` (387 lines)
- `src/media/VideoPlayer.h` (154 lines)
- `src/media/VideoPlayer.cpp` (529 lines)
- `src/media/ImageSequence.h` (109 lines)
- `src/media/ImageSequence.cpp` (282 lines)
- `shaders/` directory listing

---

## Section 5: Procedural Sources

### Source Registration Mechanism

Sources are registered via `SourceRegistry::registerSource(id, factory)` inside `SourceRegistry::registerDefaults()` (`SourceRegistry.cpp:20`). Each factory is a `std::function` returning `std::unique_ptr<ProceduralSource>`. The registry caches (factory, displayName, category) triples in `registry_` (unordered_map). Every source is instantiated once at registration time to harvest its display name + category via virtual accessors (`SourceRegistry.cpp:9-18`).

**ProceduralSource constructor signature** (`ProceduralSource.cpp:6-12`):
`ProceduralSource(id, displayName, category, shaderName, stateful = false)`

- `stateful = true` → ping-pong FBOs allocated in `initGL` (two RGBA8 FBOs, cleared to RGBA 0/0/0/0, `ProceduralSource.cpp:24-40`). Stateful sources read from `pingTex_[currentPing_]` and write to `pingTex_[1-currentPing_]` each frame (`ProceduralSource.cpp:101-124`).
- `stateful = false` (default) → single `outputFBO_`/`outputTex_` (`ProceduralSource.cpp:42-44`).

### Audio-Reactive Uniform Pipeline

`ProceduralSource::uploadUniforms()` (`ProceduralSource.cpp:141-246`) uploads 42+ audio uniforms to every source shader (locations fetched via `getUniformIDFromName`; silently skipped if <0). Covers:

- Globals: `u_time`, `u_resolution`
- Basic: `u_rms`, `u_bass`, `u_mid`, `u_high`, `u_beatPhase`, `u_spectralCentroid`, `u_onsetStrength`
- P18 extended (lines 178-206): `u_onsetDetected`, `u_barPhase`, `u_phrasePhase`, `u_spectralFlux`, `u_dominantPitch`, `u_pitchConfidence`, `u_detectedKey`, `u_keyIsMajor`, `u_structuralState`, `u_bpm`, `u_hcdf`, `u_bandEnergies[7]`, `u_chromagram[12]`, `u_mfccs[13]`
- P23 genre (lines 208-214): `u_genre`, `u_genreConfidence`, `u_energyState`
- P25 advanced (lines 216-226): `u_sidechainPump`, `u_swingRatio`, `u_formantPresence`, `u_resonancePeak`, `u_reeseBass`
- Feedback texture on unit 1: `u_feedbackTex` (lines 228-236, compositor's previous frame, set via `setFeedbackTexture()`)
- Per-source params via `u_src_*` uniform names

**Therefore: ALL sources are audio-reactive to some extent** (the ones that declare the uniforms react visibly; others ignore them). Explicit "audio-first" sources are in the **Audio-Visual** category.

---

### Source Counts by Category

Grand total: **108 sources** = 101 literal `registerSource("...", lambda)` calls + 7 wireframes via `registerWireframe` helper lambda at `SourceRegistry.cpp:282-296` (called at lines 297-303).

| Category | Count | Sources |
|----------|-------|---------|
| **Noise** | 3 | perlin_noise, plasma, voronoi |
| **Fractal (2D)** | 7 | kaleido_fractal, mandelbrot, julia_set, burning_ship, newton_fractal, sierpinski, apollonian |
| **3D (mixed: ray-marched fractals + torus + other)** | 22 | geometric_tunnel is "Geometric" not "3D". Category "3D" contains: spiral_tunnel, mandelbulb, menger_sponge, kifs, julia_set_3d, burning_ship_3d, newton_3d, sierpinski_tetra, apollonian_3d, striped_torus, spiral_vortex, checker_torus, ribbed_vortex, wormhole_tunnel, twisted_torus, wormhole, torus_hole, crystal_cavern, infinite_corridor, orbit_chamber, scroll_plane, rotating_cube_map, dual_plane_drift, dna_helix |
| **Geometric** | 8 | geometric_tunnel, color_gradient, shape_generator, infinite_zoom, moire_interference, astral_grid, radial_burst, hex_grid, sacred_geometry, radar_sweep, dot_matrix_wave |
| **Audio-Visual** | 9 | audio_waveform, spectrum_landscape, chromatic_ring, band_tower, timbral_nebula, structural_landscape, cymatics, spectral_waterfall, spectral_ring |
| **Nature** | 5 | reaction_diffusion (stateful), cellular_automata (stateful), fire_wall, water_caustics, electric_arc, fire |
| **Utility** | 2 | solid_color, strobe_light |
| **Pattern** | 6 | checkerboard, line_pattern, concentric_rings, sine_oscillator, spiral_pattern, terrain_lines, bump_light, glitch_grid |
| **Organic** | 1 | metaballs |
| **Wireframe** | 7 | wire_sphere, wire_torus, wire_cube, wire_cylinder, wire_cone, wire_icosahedron, wire_wolf |
| **Lines** | 11 | zigzag_lines, star_burst, polygon_lines, waveform_lines, lissajous, spirograph, angular_grid, fractal_tree, laser_scan, moire_lines, line_generator |
| **Math** | 8 | lissajous_weaver, fermat_spiral, hyperbolic_tiling, penrose_pulse, superformula, truchet_labyrinth, rose_curves, fibonacci_spiral |
| **Lighting** | 1 | laser_scanner |
| **Text** | 2 | text_wall, text_animator |
| **Particle** | 3 | lightning_storm, starfield, particle_nebula |
| **Simulation** (stateful) | 3 | strange_attractor, gravity_well, fluid_dynamics |
| **MilkDrop** | 1 | projectm_visualizer |
| **Routing** | 1 | layer_router |

**Stateful sources (need ping-pong FBOs, require continuous frames)**: `reaction_diffusion`, `cellular_automata`, `strange_attractor`, `gravity_well`, `fluid_dynamics` (5 total). All others are stateless fragment-shader-per-frame.

---

### Complete Source Table

Columns: **ID | Display Name | Category | Shader | File:Line | Params (name → uniform → default)** | Stateful?

All params have range [0.0, 1.0] unless noted; defaults are listed. "Audio-reactive" = yes for all (framework uploads audio uniforms to every source).

#### Noise (3)

| ID | Display Name | Shader | Line | Params |
|---|---|---|---|---|
| `perlin_noise` | Perlin Noise | `source_perlin_noise` | 23 | Scale→u_src_scale(0.5), Speed→u_src_speed(0.3), Octaves→u_src_octaves(0.5), Color Shift→u_src_color_shift(0.0) |
| `plasma` | Plasma | `source_plasma` | 33 | Speed(0.4), Complexity(0.5), Color Cycle(0.0), Intensity(0.7) |
| `voronoi` | Voronoi | `source_voronoi` | 43 | Scale(0.4), Speed(0.3), Edge Width(0.3), Color Mode(0.0) |

#### Fractal 2D (7)

| ID | Display Name | Shader | Line | Params |
|---|---|---|---|---|
| `kaleido_fractal` | Kaleidoscopic Fractal | `source_kaleido_fractal` | 53 | Iterations(0.5), Fold Angle(0.4), Zoom(0.5), Rotation(0.0), Color Shift(0.0), Palette(0.6) |
| `mandelbrot` | Mandelbrot / Julia | `source_mandelbrot` | 65 | Dive Speed(0.0), Location(0.0), Zoom(0.0), Center X(0.5), Center Y(0.5), Julia Mix(0.0), Max Iterations(0.3), Power(0.0), Color Speed(0.3), Color Shift(0.0), Palette(0.6) |
| `julia_set` | Julia Set | `source_julia_set` | 415 | Dive Speed(0.0), Location(0.0), C Real(0.35), C Imaginary(0.38), Zoom(0.25), Iterations(0.3), Color Speed(0.3), Color Shift(0.0), Palette(0.6) |
| `burning_ship` | Burning Ship | `source_burning_ship` | 429 | Dive Speed(0.0), Location(0.0), Center X(0.55), Center Y(0.6), Zoom(0.2), Iterations(0.3), Color Speed(0.3), Color Shift(0.0), Palette(0.6) |
| `newton_fractal` | Newton Fractal | `source_newton_fractal` | 443 | Dive Speed(0.0), Power(0.2), Zoom(0.3), Damping(0.5), Color Shift(0.0), Palette(0.6) |
| `sierpinski` | Sierpinski | `source_sierpinski` | 454 | Dive Speed(0.0), Mode(0.0), Zoom(0.0), Iterations(0.5), Rotation(0.5), Color Shift(0.0), Palette(0.6) |
| `apollonian` | Apollonian Gasket | `source_apollonian` | 466 | Dive Speed(0.0), Zoom(0.0), Iterations(0.4), Rotation(0.5), Color Shift(0.0), Palette(0.6) |

#### 3D Ray-Marched Fractals (8)

Shared with per-source additions; common params: Angle X(u_src_rotation_x 0.55), Angle Y(u_src_rotation_y 0.55), Zoom(0.3), Speed(0.55), Color Shift(0.0), Cross Section(u_src_slice 0.5), Slice Count(u_src_slice_count 0.0), Trail Distance(u_src_trail_dist 0.0), Trail Fade(u_src_trail_fade 0.5), Slice Distance(u_src_slice_dist 0.3), Feedback(0.0), Palette(0.6).

| ID | Display Name | Shader | Line | Extra Params |
|---|---|---|---|---|
| `mandelbulb` | Mandelbulb | `source_mandelbulb` | 479 | Power(0.5), Iterations(0.4), Detail(0.5), Glow(0.0) |
| `menger_sponge` | Menger Sponge | `source_menger_sponge` | 500 | Iterations(0.5), Twist(0.0) |
| `kifs` | Kaleidoscopic IFS | `source_kifs` | 519 | Scale(0.4), Iterations(0.4), Fold Type(0.0), Offset(0.5) |
| `julia_set_3d` | Julia Set 3D | `source_julia_set_3d` | 542 | Location(0.0), C Real(0.35), C Imaginary(0.6), Iterations(0.4), Glow(0.0) |
| `burning_ship_3d` | Burning Ship 3D | `source_burning_ship_3d` | 564 | Power(0.5), Glow(0.0) |
| `newton_3d` | Newton 3D | `source_newton_3d` | 583 | Power(0.2), Damping(0.5), Height(0.5) |
| `sierpinski_tetra` | Sierpinski Tetrahedron | `source_sierpinski_tetra` | 602 | Iterations(0.5) |
| `apollonian_3d` | Apollonian 3D | `source_apollonian_3d` | 620 | Scale(0.3), Iterations(0.4) |

#### 3D Torus (8) — shared torus controls via `addTorusControls` lambda at `SourceRegistry.cpp:644-662`

Shared: Orbit(u_src_orbit 0.0), Tilt(0.49), Speed(0.3), Zoom(0.4), Lens Shape(0.0), Lens Rotate(0.0), Depth Fade(0.0), Tube Radius(0.32), Pinch(0.5), Heart(0.5), Shading(0.5), Color Shift(0.0).

| ID | Display Name | Shader | Line | Extra Params |
|---|---|---|---|---|
| `striped_torus` | Striped Torus | `source_striped_torus` | 664 | Stripe Count(0.3), Twist(0.4) |
| `spiral_vortex` | Spiral Vortex | `source_spiral_vortex` | 672 | Twist(0.3), Stripe Count(0.3) |
| `checker_torus` | Checker Torus | `source_checker_torus` | 680 | Grid U(0.3), Grid V(0.3) |
| `ribbed_vortex` | Ribbed Vortex | `source_ribbed_vortex` | 688 | Ridge Count(0.3), Color Mix(0.5) |
| `wormhole_tunnel` | Wormhole Tunnel | `source_wormhole_tunnel` | 696 | Warp(0.3) |
| `twisted_torus` | Twisted Torus | `source_twisted_torus` | 703 | Twist(0.3), Stripe Count(0.3) |
| `wormhole` | Wormhole | `source_wormhole` | 711 | Warp(0.3), Glow(0.4) |
| `torus_hole` | Torus Hole | `source_torus_hole` | 719 | Orbit(0.0), Tilt(0.49), Speed(0.3), Zoom(0.4), Lens Shape(0.0), Lens Rotate(0.0), Depth Fade(0.0), Stripe Count(0.6), Twist(0.5), Stripe Angle(0.0), Stripe Scale(0.33), Stripe Width(0.5), Phi Offset(0.0), Theta Offset(0.0), Tube Radius(0.32), Pinch(0.5), Heart(0.5), Shading(0.5), Color Shift(0.0) |

#### 3D Other (6)

| ID | Display Name | Shader | Line | Params |
|---|---|---|---|---|
| `spiral_tunnel` | Spiral Tunnel | `source_spiral_tunnel` | 271 | Speed(0.3), Arms(0.3), Depth(0.5), Twist(0.4), Color Shift(0.0) |
| `crystal_cavern` | Crystal Cavern | `source_crystal_cavern` | 827 | Speed(0.3), Crystal Size(0.5), Reflectivity(0.5), Light Color(0.0), Fog Density(0.3), Complexity(0.5) |
| `infinite_corridor` | Infinite Corridor | `source_infinite_corridor` | 839 | Speed(0.3), Width(0.5), Wall Pattern(0.0), Light Spacing(0.3), Light Intensity(0.5), Color(0.0) |
| `orbit_chamber` | Orbit Chamber | `source_orbit_chamber` | 851 | Object Count(0.4), Object Type(0.0), Orbit Speed(0.4), Orbit Radius(0.5), Material(0.5), Light Orbit(0.3), Color Shift(0.0) |
| `scroll_plane` | Scroll Plane | `source_scroll_plane` | 962 | Speed X(0.6), Speed Y(0.5), Scale(0.3), Warp(0.0), Color Shift(0.0) |
| `rotating_cube_map` | Rotating Cube Map | `source_rotating_cube_map` | 973 | Rotation X(0.6), Rotation Y(0.55), Scale(0.3), Light(0.5), Color Shift(0.0) |
| `dual_plane_drift` | Dual Plane Drift | `source_dual_plane_drift` | 984 | Speed(0.4), Rotation(0.0), Distance(0.5), Scale(0.3), Color Shift(0.0) |
| `dna_helix` | DNA Helix | `source_dna` | 1184 | Speed(0.3), Zoom(0.5), Glow(0.5), Color Shift(0.0) |

#### Wireframe (7) — all use shader `source_wireframe_3d`, registered via `registerWireframe` helper (`SourceRegistry.cpp:282-303`)

Shared params: Shape(u_src_shape, per-source value), Density(0.3), Rotation X(0.6), Rotation Y(0.6), Rotation Z(0.5), Thickness(0.3), Perspective(0.4), Glow(0.3), Color Shift(0.0).

| ID | Display Name | Shape Value | Line |
|---|---|---|---|
| `wire_sphere` | Wireframe Sphere | 0.072 | 297 |
| `wire_torus` | Wireframe Torus | 0.215 | 298 |
| `wire_cube` | Wireframe Cube | 0.358 | 299 |
| `wire_cylinder` | Wireframe Cylinder | 0.501 | 300 |
| `wire_cone` | Wireframe Cone | 0.644 | 301 |
| `wire_icosahedron` | Wireframe Icosahedron | 0.787 | 302 |
| `wire_wolf` | Wireframe Wolf | 0.930 | 303 |

#### Geometric (10 — includes a few mis-sliced below)

| ID | Display Name | Shader | Line | Params |
|---|---|---|---|---|
| `geometric_tunnel` | Geometric Tunnel | `source_geometric_tunnel` | 82 | Speed(0.4), Segments(0.3), Twist(0.2), Color Shift(0.0) |
| `color_gradient` | Color Gradient | `source_color_gradient` | 92 | Angle(0.25), Speed(0.2), Color 1(0.0), Color 2(0.5) |
| `shape_generator` | Shape Generator | `source_shape_generator` | 238 | Shape(0.0), Size(0.5), Rotation(0.0), Outline(0.0), Color Shift(0.0) |
| `infinite_zoom` | Infinite Zoom | `source_infinite_zoom` | 260 | Speed(0.3), Layers(0.5), Rotation(0.5), Color Shift(0.0) |
| `moire_interference` | Moire Interference | `source_moire_interference` | 814 | Pattern(0.0), Frequency(0.3), Offset X(0.5), Offset Y(0.5), Rotation(0.55), Zoom(0.3), Color Shift(0.0) |
| `astral_grid` | Astral Grid | `source_astral_grid` | 864 | Grid Size(0.3), Scroll Speed(0.4), Tilt(0.5), Warp(0.0), Glow(0.5), Horizon Color(0.6) |
| `radial_burst` | Radial Burst | `source_radial_burst` | 876 | Ray Count(0.3), Length(0.5), Rotation(0.55), Width(0.3), Taper(0.5), Glow(0.3), Color Shift(0.0) |
| `hex_grid` | Hex Grid | `source_hex_grid` | 889 | Cell Size(0.3), Pattern(0.0), Fill(0.5), Edge Width(0.3), Rotation(0.0), Color Mode(0.5) |
| `sacred_geometry` | Sacred Geometry | `source_sacred_geometry` | 901 | Pattern(0.0), Rotation(0.5), Breathe(0.3), Line Width(0.3), Glow(0.3), Reveal(1.0), Color Shift(0.0) |
| `radar_sweep` | Radar Sweep | `source_radar` | 1166 | Speed(0.5), Decay(0.5), Grid(0.3), Color Shift(0.0) |
| `dot_matrix_wave` | Dot Matrix Wave | `source_dot_matrix` | 1193 | Density(0.5), Speed(0.5), Damping(0.5), Dot Size(0.3), Color(0.0), Sources(0.5) |

#### Pattern (8)

| ID | Display Name | Shader | Line | Params |
|---|---|---|---|---|
| `checkerboard` | Checkerboard | `source_checkerboard` | 160 | Columns(0.3), Rows(0.3), Color 1 Hue(0.0), Color 2 Hue(0.0) |
| `line_pattern` | Line Pattern | `source_line_pattern` | 170 | Count(0.3), Width(0.3), Rotation(0.0), Speed(0.2), Color Shift(0.0) |
| `concentric_rings` | Concentric Rings | `source_concentric_rings` | 181 | Count(0.3), Spacing(0.5), Width(0.3), Rotation(0.0), Color Shift(0.0) |
| `sine_oscillator` | Sine Oscillator | `source_sine_oscillator` | 192 | Waves(0.3), Frequency(0.5), Amplitude(0.5), Modulation(0.0), Thickness(0.3), Color Shift(0.0) |
| `spiral_pattern` | Spiral Pattern | `source_spiral_pattern` | 204 | Arms(0.3), Zoom(0.5), Speed(0.3), Distortion(0.0), Color Shift(0.0) |
| `terrain_lines` | Terrain Lines | `source_terrain_lines` | 226 | Height(0.4), Lines(0.4), Jagginess(0.3), Speed(0.2), Tilt(0.3), Color Shift(0.0) |
| `bump_light` | Bump Light | `source_bump_light` | 249 | Light X(0.5), Light Y(0.3), Intensity(0.6), Bumps(0.5), Color Shift(0.0) |
| `glitch_grid` | Glitch Grid | `source_glitch_grid` | 1175 | Grid Size(0.4), Chaos(0.5), Flicker(0.5), Color Shift(0.0) |

#### Organic (1)

| ID | Display Name | Shader | Line | Params |
|---|---|---|---|---|
| `metaballs` | Metaballs | `source_metaballs` | 215 | Count(0.3), Size(0.4), Speed(0.3), Blend(0.5), Color Shift(0.0) |

#### Utility (2)

| ID | Display Name | Shader | Line | Params |
|---|---|---|---|---|
| `solid_color` | Solid Color | `source_solid_color` | 137 | Red(1.0), Green(1.0), Blue(1.0) |
| `strobe_light` | Strobe Light | `source_strobe_light` | 146 | Frequency(0.5), Fade(0.3), Color 1 R(1.0), Color 1 G(1.0), Color 1 B(1.0), Color 2 R(0.0), Color 2 G(0.0), Color 2 B(0.0) |

#### Audio-Visual (9) — explicitly audio-reactive

| ID | Display Name | Shader | Line | Params |
|---|---|---|---|---|
| `audio_waveform` | Audio Waveform | `source_audio_waveform` | 102 | Style(0.0), Thickness(0.3), Glow(0.5), Color Shift(0.5) |
| `spectrum_landscape` | Spectrum Landscape | `source_spectrum_landscape` | 998 | Height Scale(0.5), Camera Angle(0.3), Color Mode(0.0), Glow(0.5), Smoothing(0.5) |
| `chromatic_ring` | Chromatic Ring | `source_chromatic_ring` | 1008 | Ring Width(0.5), Glow(0.5), Rotation(0.0), Ripple(0.5) |
| `band_tower` | Band Tower | `source_band_tower` | 1017 | Shape(0.0), Spacing(0.3), Reflection(0.3), Color Mode(0.0), Smoothing(0.5), 3D Rotation(0.0) |
| `timbral_nebula` | Timbral Nebula | `source_timbral_nebula` | 1028 | Particle Count(0.5), Spread(0.5), Trail Length(0.3), Color Source(0.0), Glow(0.5), Sensitivity(0.5) |
| `structural_landscape` | Structural Landscape | `source_structural_landscape` | 1039 | Terrain Scale(0.5), History Length(0.5), Drama(0.5), Color Palette(0.0), Fog(0.3), Camera Height(0.5) |
| `cymatics` | Cymatics | `source_cymatics` | 1050 | Resonance(0.5), Damping(0.5), Color Shift(0.0) |
| `spectral_waterfall` | Spectral Waterfall | `source_spectral_waterfall` | 1058 | Scroll Speed(0.5), Color Mode(0.0), Log Scale(0.5) |
| `spectral_ring` | Spectral Ring | `source_spectral_ring` | 1066 | Radius(0.5), Thickness(0.3), Glow(0.5), Rotation(0.3), Color Shift(0.0) |

#### Nature (6)

| ID | Display Name | Shader | Line | Params | Stateful |
|---|---|---|---|---|---|
| `reaction_diffusion` | Reaction-Diffusion | `source_reaction_diffusion` | 112 | Feed Rate(0.28), Kill Rate(0.32), Diffusion A(0.5), Diffusion B(0.5) | **YES** |
| `cellular_automata` | Cellular Automata | `source_cellular_automata` | 122 | Mode(0.0), Birth Low(0.25), Birth High(0.25), Survival Low(0.25), Color Shift(0.0) | **YES** |
| `fire_wall` | Fire Wall | `source_fire_wall` | 914 | Height(0.6), Turbulence(0.5), Speed(0.4), Temperature(0.7), Density(0.6), Wind(0.0) | no |
| `water_caustics` | Water Caustics | `source_water_caustics` | 926 | Complexity(0.5), Speed(0.3), Brightness(0.5), Color(0.0), Distortion(0.3), Scale(0.3) | no |
| `electric_arc` | Electric Arc | `source_electric_arc` | 938 | Arc Count(0.2), Chaos(0.5), Thickness(0.3), Branches(0.3), Glow(0.5), Color(0.5) | no |
| `fire` | Fire | `source_fire` | 1139 | Height(0.5), Turbulence(0.5), Speed(0.5), Color Shift(0.0) | no |

#### Lines (11)

| ID | Display Name | Shader | Line | Params |
|---|---|---|---|---|
| `zigzag_lines` | Zigzag Lines | `source_zigzag_lines` | 307 | Count(0.3), Amplitude(0.5), Speed(0.4), Thickness(0.3), Color Shift(0.0) |
| `star_burst` | Star Burst | `source_star_burst` | 317 | Rays(0.4), Thickness(0.3), Speed(0.6), Taper(0.3), Color Shift(0.0) |
| `polygon_lines` | Polygon Lines | `source_polygon_lines` | 327 | Sides(0.3), Size(0.5), Thickness(0.3), Layers(0.3), Speed(0.6), Color Shift(0.0) |
| `waveform_lines` | Waveform Lines | `source_waveform_lines` | 338 | Waveform(0.0), Frequency(0.3), Amplitude(0.5), Count(0.3), Thickness(0.3), Speed(0.4), Color Shift(0.0) |
| `lissajous` | Lissajous | `source_lissajous` | 350 | Ratio X(0.28), Ratio Y(0.42), Phase(0.25), Thickness(0.3), Speed(0.3), Color Shift(0.0) |
| `spirograph` | Spirograph | `source_spirograph` | 361 | Inner Radius(0.4), Offset(0.5), Thickness(0.3), Speed(0.3), Color Shift(0.0) |
| `angular_grid` | Angular Grid | `source_angular_grid` | 371 | Angle(0.25), Count(0.3), Thickness(0.3), Symmetry(0.2), Speed(0.3), Color Shift(0.0) |
| `fractal_tree` | Fractal Tree | `source_fractal_tree` | 382 | Branches(0.5), Angle(0.4), Depth(0.4), Thickness(0.3), Speed(0.3), Color Shift(0.0) |
| `laser_scan` | Laser Scan | `source_laser_scan` | 393 | Beams(0.3), Speed(0.5), Thickness(0.3), Spread(0.4), Color Shift(0.0) |
| `moire_lines` | Moire Lines | `source_moire_lines` | 403 | Density(0.3), Angle Offset(0.1), Speed(0.3), Layers(0.3), Color Shift(0.0) |
| `line_generator` | Line Generator | `source_line_generator` | 749 | Pattern(0.0), Count(0.3), Thickness(0.3), Speed(0.3), Feedback(0.5), Feedback Zoom(0.52), Feedback Rotation(0.52), Feedback Decay(0.7), Color Shift(0.0) |

#### Math (8)

| ID | Display Name | Shader | Line | Params |
|---|---|---|---|---|
| `lissajous_weaver` | Lissajous Weaver | `source_lissajous_weaver` | 766 | Frequency X(0.3), Frequency Y(0.4), Phase(0.25), Decay(0.5), Harmonics(0.3), Thickness(0.3), Color Shift(0.0) |
| `fermat_spiral` | Fermat Spiral Garden | `source_fermat_spiral` | 779 | Count(0.5), Divergence Angle(0.5), Shape(0.0), Growth(0.3), Pulse(0.5), Color Spread(0.5) |
| `hyperbolic_tiling` | Hyperbolic Tiling | `source_hyperbolic_tiling` | 791 | Polygon Sides(0.4), Vertex Order(0.2), Rotation(0.5), Zoom(0.3), Color Scheme(0.5), Line Width(0.3) |
| `penrose_pulse` | Penrose Pulse | `source_penrose_pulse` | 803 | Generation(0.5), Ripple Speed(0.5), Color Mode(0.5), Edge Glow(0.5), Morph(0.0) |
| `superformula` | Superformula | `source_superformula` | 1089 | Symmetry(u_src_m 0.3), Roundness(u_src_n1 0.5), Concavity(u_src_n2 0.5), Blobbiness(u_src_n3 0.5), Size(0.5), Color Shift(0.0) |
| `truchet_labyrinth` | Truchet Labyrinth | `source_truchet` | 1100 | Density(0.5), Style(0.0), Thickness(0.3), Glow(0.5), Color Shift(0.0) |
| `rose_curves` | Rose Curves | `source_rose` | 1110 | Petals(u_src_k 0.5), Thickness(0.3), Layers(0.5), Glow(0.5), Color Shift(0.0) |
| `fibonacci_spiral` | Fibonacci Spiral | `source_fibonacci` | 1120 | Elements(0.5), Size(0.5), Spread(0.5), Glow(0.3), Color Shift(0.0) |

#### Lighting (1)

| ID | Display Name | Shader | Line | Params |
|---|---|---|---|---|
| `laser_scanner` | Laser Scanner | `source_laser_scanner` | 950 | Pattern(0.0), Beam Count(0.3), Color(0.3), Speed(0.5), Spread(0.5), Flicker(0.2) |

#### Particle (3)

| ID | Display Name | Shader | Line | Params |
|---|---|---|---|---|
| `lightning_storm` | Lightning Storm | `source_lightning` | 1130 | Intensity(0.5), Branches(0.5), Glow(0.5), Color Shift(0.0) |
| `starfield` | Starfield | `source_starfield` | 1148 | Speed(0.5), Density(0.5), Streak(0.3), Color Shift(0.0) |
| `particle_nebula` | Particle Nebula | `source_nebula` | 1157 | Density(0.5), Scale(0.5), Speed(0.3), Color Shift(0.0) |

#### Text (2)

| ID | Display Name | Shader | Line | Params |
|---|---|---|---|---|
| `text_wall` | Scrolling Text Wall | `source_text_wall` | 1076 | Speed(0.4), Density(0.6), Size(0.4), Color Shift(0.3) |
| `text_animator` | Text Animator | `source_text_animator` | 1207 | Font Size(0.4), Text Red(0.0), Text Green(1.0), Text Blue(0.5), Animation(0.0), Speed(0.4), Columns(0.3), Spacing(0.5) |

#### Simulation (3) — all stateful

| ID | Display Name | Shader | Line | Params | Stateful |
|---|---|---|---|---|---|
| `strange_attractor` | Strange Attractor | `source_strange_attractor` | 1221 | Attractor Type(0.0), Speed(0.5), Trail Length(0.7), Rotation(0.3), Glow(0.5), Color Mode(0.0) | **YES** |
| `gravity_well` | Gravity Well | `source_gravity_well` | 1233 | Particle Density(0.5), Gravity(0.6), Scatter(0.3), Trail Length(0.6), Wells(0.0), Color Mode(0.0) | **YES** |
| `fluid_dynamics` | Fluid Dynamics | `source_fluid_dynamics` | 1245 | Viscosity(0.4), Diffusion(0.3), Injection Radius(0.2), Color Mode(0.0), Curl(0.6), Decay(0.5) | **YES** |

Note: Strange Attractor supports 6 attractor types via the Attractor Type param (per project memory): Lorenz, Rossler, Halvorsen, Thomas, Aizawa, Dadras.

#### MilkDrop (1)

| ID | Display Name | Shader | Line | Params |
|---|---|---|---|---|
| `projectm_visualizer` | MilkDrop Visualizer | (none — uses libprojectM) | 1257 | Beat Sensitivity(0.5), Speed(0.5), Warp Amount(0.5), Decay(0.5), Gamma(0.5) |

Subclass: `ProjectMSource` extends `ProceduralSource`. Constructor at `ProjectMSource.cpp:7-20` — category literal "MilkDrop", no shader name (empty).

#### Routing (1)

| ID | Display Name | Shader | Line | Params |
|---|---|---|---|---|
| `layer_router` | Layer Router | `source_layer_router` | 1262 | Source Layer(u_src_layer 0.0) — maps [0,1] to layer indices 0-9 |

---

## Media sources (Video, Image Sequence)

### VideoPlayer (`src/media/VideoPlayer.h:29`, `src/media/VideoPlayer.cpp`)

**Decoder**: FFmpeg libavformat + libavcodec + libswscale. Decodes any format supported by FFmpeg; the header doc comment at `VideoPlayer.h:19` lists: **MP4, MOV, QuickTime (H.264/H.265/ProRes), HAP/HAP Alpha, AVI**. No hardcoded list — whatever `avcodec_find_decoder` resolves is accepted (`VideoPlayer.cpp:90`).

**Alpha detection** (`VideoPlayer.cpp:144-152`): flags alpha=true for pixel formats:
- `AV_PIX_FMT_RGBA`, `AV_PIX_FMT_BGRA`, `AV_PIX_FMT_ARGB`, `AV_PIX_FMT_ABGR`
- `AV_PIX_FMT_YUVA420P`, `AV_PIX_FMT_YUVA444P`
- `AV_PIX_FMT_PAL8`
- Or `codec_id == AV_CODEC_ID_HAP` (HAP family always)

**Swscale output**: `AV_PIX_FMT_RGBA` via `SWS_BILINEAR` (`VideoPlayer.cpp:154-158`). Frames flipped vertically for OpenGL in `convertFrameToRGBA()` (`VideoPlayer.cpp:520-528`).

**Threading**: `open()/close()` on message thread, `advanceFrame()/uploadToTexture()` on GL thread (`VideoPlayer.h:22-27`). Transport atomics: `open_`, `hasAlpha_`, `speed_`, `reverse_`, `loopMode_`, `playing_`, `playheadPosition_`, `seekRequested_`, `seekTarget_` (`VideoPlayer.h:124-134`). FFmpeg state lock via `ffmpegMutex_`.

**Decoding**: 2 threads enabled (`codecCtx_->thread_count = 2`, `VideoPlayer.cpp:102`). One frame ahead, CPU-buffered RGBA, lazy upload via `glTexSubImage2D` (`VideoPlayer.cpp:334-337`). Seek backward via `AVSEEK_FLAG_BACKWARD` + `avcodec_flush_buffers` (`VideoPlayer.cpp:453-464`).

**Properties exposed**: width, height, duration (seconds), frameRate (fps), totalFrames (`VideoPlayer.h:46-49`).

**Transport controls**:
- `LoopMode` enum: `Loop`, `PingPong`, `OneShot` (`VideoPlayer.h:53`)
- `setSpeed(float)` — playback speed multiplier (`VideoPlayer.h:55`)
- `setReverse(bool)` — reverse playback (`VideoPlayer.h:58`)
- `setLoopMode(LoopMode)` (`VideoPlayer.h:61`)
- `setPlaying(bool)` — play/pause (`VideoPlayer.h:64`)
- `seekTo(double normalizedPosition)` — [0,1] seek, thread-safe (`VideoPlayer.h:68`)
- `getPlayheadPosition()` — [0,1] current position

**Loop boundary handling** (`VideoPlayer.cpp:262-297`):
- `Loop`: `fmod(currentTime_, duration_)`
- `PingPong`: reflect position, flip `pingPongForward_`
- `OneShot`: clamp to boundary, stop playing

**Thumbnail**: `getThumbnail(maxWidth, maxHeight)` returns a scaled JUCE Image of the first frame (`VideoPlayer.cpp:353-386`).

### ImageSequence (`src/media/ImageSequence.h:17`, `src/media/ImageSequence.cpp`)

**Purpose**: Treats a set of images (PNG/JPG/JPEG/BMP/TIFF/GIF — `ImageSequence.cpp:29-30`) as a video clip.

**Loading**:
- `open(std::vector<juce::File>)` — from discrete file list (`ImageSequence.cpp:17`)
- `openDirectory(juce::File)` — scans via `juce::RangedDirectoryIterator` matching `"*.png;*.jpg;*.jpeg;*.bmp;*.tiff;*.gif"` (`ImageSequence.cpp:77-87`)
- Files sorted by `compareNatural()` for consistent ordering (`ImageSequence.cpp:41-43`).

**Lazy loading**: each frame loaded to GL texture on first access via `loadImageToTexture()` (`ImageSequence.cpp:190-202`). Decoded via `juce::ImageFileFormat::loadFrom`, converted to ARGB → RGBA with Y-flip for GL (`ImageSequence.cpp:237-282`).

**FPS control**: `setFps(float)` clamped to >= 0.1 fps, default **10 fps** (`ImageSequence.h:36, 95`). Duration derived: `files.size() / fps` (`ImageSequence.cpp:102-107`).

**Transport**: Identical API to VideoPlayer — `LoopMode{Loop, PingPong, OneShot}`, speed, reverse, playing, seekTo, playheadPosition (`ImageSequence.h:45-61`). Boundary handling mirrors VideoPlayer (`ImageSequence.cpp:143-177`).

**Thumbnail**: `getThumbnail()` loads first image, rescales (`ImageSequence.cpp:219-235`).

**Release**: `releaseGL()` deletes all uploaded textures (`ImageSequence.cpp:204-217`).

---

## ProjectM / MilkDrop

### Build-conditional

All projectM integration is gated behind `#ifdef AUDIODNA_HAS_PROJECTM` (`ProjectMSource.h:9-11`, `ProjectMSource.cpp:35-56`, 64-70, 88-91, 116-169, 236-252). When disabled, the source renders a placeholder dark-purple clear (`ProjectMSource.cpp:171-177`). Header: `<projectM-4/projectM.h>`.

### `ProjectMSource` (`src/sources/ProjectMSource.h:25`)

- Subclasses `ProceduralSource` with id=`"projectm_visualizer"`, displayName=`"MilkDrop Visualizer"`, category=`"MilkDrop"`, shader=empty (`ProjectMSource.cpp:7-9`).
- Does NOT use a GLSL shader — delegates to projectM's internal renderer which manages its own shaders/feedback/warp mesh.
- **Parameters** (`ProjectMSource.cpp:13-17`): Beat Sensitivity(0.5), Speed(0.5), Warp Amount(0.5), Decay(0.5), Gamma(0.5).
- Parameter application at `applyParams()` (`ProjectMSource.cpp:234-253`):
  - Beat Sensitivity → `projectm_set_beat_sensitivity(pm_, value * 2.0f)` — 0-2 range
  - Speed → `projectm_set_preset_duration(pm_, 5.0 + (1-value)*55.0)` — 5-60 seconds
  - Gamma → stub (placeholder for future API extension)
- projectM config at init (`ProjectMSource.cpp:37-54`): 60 fps, mesh size 48x36, aspect correction true.

### Render cycle (`ProjectMSource.cpp:107-182`)

1. Process pending preset load (queued from non-GL threads via `pendingPresetMutex_`).
2. Apply parameters.
3. If not locked, `presetSelector_.processFrame(snapshot)` for audio-driven switching.
4. Feed PCM to projectM via `projectm_pcm_add_float` (mono, 2048 samples max, from `pcmBuffer_`).
5. Save full GL state (viewport, FBO, program, VAO, textures, blend, depth, stencil, cull, scissor, blend funcs, depth func — `ProjectMSource.cpp:257-275`).
6. Bind default FBO, call `projectm_opengl_render_frame(pm_)`.
7. Blit screen framebuffer → `outputFBO_` via `glBlitFramebuffer`.
8. Restore GL state (`ProjectMSource.cpp:277-301`).

### Audio ingestion (`ProjectMSource::feedAudio`, `ProjectMSource.cpp:184-191`)

- Called from audio/analysis pipeline.
- `pcmMutex_` protects `pcmBuffer_` (fixed 2048-float mono buffer).
- Copies last N samples if numSamples > 2048.

### Preset API (`ProjectMSource.h:51-57`)

- `loadPreset(path, smooth=true)` — queued to GL thread via `pendingPresetMutex_` (projectM compiles shaders on load).
- `nextPreset(bool smooth)`, `prevPreset(bool smooth)`, `randomPreset(bool smooth)` — delegate to `ProjectMPresetManager` then `loadPreset`.
- `setPresetLocked(bool)` — prevents auto-switching.
- `getCurrentPresetName()`, `getCurrentPresetPath()`.

### `ProjectMPresetManager` (`src/sources/ProjectMPresetManager.h:9`)

**`PresetInfo` struct** (`ProjectMPresetManager.h:12-21`):
- `name` — file stem ("Geiss - Soft Flower")
- `path` — absolute file path
- `mood` — one of: Calm, Energetic, Psychedelic, Geometric, Dark, Minimal
- `style` — optional user tag
- `energy` — [0,1] for auto-DJ matching
- `favorite` — user-starred flag
- `userPreset` — user-created/customized flag

**File format support** (`ProjectMPresetManager.cpp:12`): `*.milk;*.prjm` — scanned recursively via `juce::File::findChildFiles` (mode `findFiles`, recursive true).

**API**:
- `scanDirectory(dirPath)` — recursive scan, dedupes, infers mood, sorts by name (`ProjectMPresetManager.cpp:6-38`).
- `setPresetDirectories(vector<string>)`, `getPresetDirectories()`, `rescan()` — multi-dir support (`ProjectMPresetManager.cpp:40-50`).
- `loadManifest(jsonPath)` — reads `presets.json` assigning mood/energy/style to named presets (`ProjectMPresetManager.cpp:52-83`).
- `saveUserData(jsonPath)` / `loadUserData(jsonPath)` — persist favorites + userPreset flags to JSON (`ProjectMPresetManager.cpp:85-154`).
- `getAllPresets()`, `getPresetCount()`, `getPreset(index)`.
- `getPresetsByMood(mood)`, `getFavorites()`, `getUserPresets()`.
- `search(query)` — case-insensitive substring name match (`ProjectMPresetManager.cpp:199-222`).
- `toggleFavorite(index)`.
- `getMoodCounts()` — returns `map<string,int>` for UI grouping.
- Navigation: `nextPreset()`, `prevPreset()`, `randomPreset()`, `randomPresetInMood(mood)` — all set `currentIndex_`, fire `onPresetChanged` callback (`ProjectMPresetManager.cpp:241-299`).

**Mood guessing** (`ProjectMPresetManager::guessMood`, `ProjectMPresetManager.cpp:301-376`) — case-insensitive keyword heuristics on preset name:
- Energetic: "energy", "fire", "blast", "explod", "rave", "strobe", "flash", "chaos"
- Psychedelic: "psyche", "trip", "acid", "warp", "morph", "halluc", "fractal", "kaleid"
- Geometric: "geom", "grid", "line", "cube", "sphere", "tunnel", "spiral"
- Dark: "dark", "shadow", "void", "black", "night"
- Minimal: "minimal", "simple", "clean", "subtle"
- Calm: "calm", "soft", "gentle", "dream", "flow", "water", "ocean", "cloud", "float"
- Fallback: first-letter bucket — <g→Energetic, <m→Geometric, <s→Psychedelic, else Calm

**Mood → energy** (`ProjectMPresetManager.cpp:378-387`):
Energetic=0.9, Psychedelic=0.7, Geometric=0.5, Dark=0.4, Minimal=0.3, Calm=0.2, default=0.5.

### `PresetSelector` — audio-driven auto-switching (`src/sources/PresetSelector.h:11`, `.cpp`)

**Configuration**:
- `setEnabled(bool)` — default false
- `setEnergyMatching(bool)` — match presets to audio energy (default true)
- `setMoodFilter(string)` — restrict to a specific mood (empty = any)
- `setTransitionBars(int)` — default 4 bars between auto-switches
- `kMinBarsBetweenSwitches = 4` (compile-time, `PresetSelector.h:54`)
- Callback: `onAutoSwitch(string presetPath)`

**Algorithm** (`PresetSelector.cpp:4-78`):
1. Bar crossing detection (barPhase wraps from >0.5 back toward 0).
2. Structural-state change detected by comparing `snapshot.structuralState` with `lastStructuralState_`.
3. On structural change after min bars → schedule `pendingSwitch_`.
4. On next bar boundary with pending switch:
   - If moodFilter set → `randomPresetInMood(moodFilter)`
   - Else if energyMatching → pick mood by structural state:
     - state 2 (DROP) → "Energetic"
     - state 1 (BUILDUP) → "Psychedelic"
     - state 3 (BREAKDOWN) → "Calm"
     - state 0 (NORMAL) → any (`randomPreset`)
   - Else → `randomPreset`
5. Fallback periodic switch: every `transitionBars * 4` bars, random preset (`PresetSelector.cpp:68-75`).

---

## Layer Router (`SourceRegistry.cpp:1262`)

- ID: `layer_router`
- Display: "Layer Router"
- Category: "Routing"
- Shader: `source_layer_router`
- Single param: **Source Layer** → `u_src_layer`, default 0.0. Maps [0,1] → layer index 0-9 (per CLAUDE.md).

**Behavior** (per CLAUDE.md Layer Router System § P20, not visible in these files — handled in CompositorEngine / Renderer):
- `CompositorEngine::compositeDeck()` saves each layer's final clip texture (after effects, before keying/blending) into `layerOutputTextures_` keyed by layer ID.
- When a clip's `sourceType == "layer_router"`, `Renderer::renderSource()` intercepts and reads the saved texture for the selected layer index.
- Self-reference safety: routing to own layer returns previous frame (one-frame delay).
- Circular references between two layers produce feedback effects.

---

## Text sources

### `text_wall` (`SourceRegistry.cpp:1076`) — "Scrolling Text Wall"
- Shader: `source_text_wall`
- Category: "Text"
- Params: Speed(0.4), Density(0.6), Size(0.4), Color Shift(0.3)
- No explicit font, text content, or color RGB parameters — characters/colors are procedural (shader-generated).

### `text_animator` (`SourceRegistry.cpp:1207`) — "Text Animator"
- Shader: `source_text_animator`
- Category: "Text"
- Params: Font Size(0.4), Text Red(0.0), Text Green(1.0), Text Blue(0.5), Animation(0.0), Speed(0.4), Columns(0.3), Spacing(0.5)
- Text color is explicit via 3 RGB uniforms; defaults to green-cyan (0, 1, 0.5).
- "Animation" param likely selects animation mode; specific modes in shader code (not in this slice).
- No explicit text-content parameter — procedural characters per shader.

**Neither source exposes runtime-editable text strings or user-selectable fonts through the parameter system.** All rendering is shader-procedural.

---

## Disk shaders on filesystem (`/shaders/`)

Only **5** files present — the rest of the 135 effect shaders and 108 source shaders are embedded in `src/render/EmbeddedShaders.h` as inline strings (per CLAUDE.md).

| File | Type | Purpose |
|---|---|---|
| `passthrough.vert` | Vertex shader | Shared fullscreen-quad UVs (per CLAUDE.md source tree) |
| `hue_shift.frag` | Fragment effect | Color: RGB→HSV rotate H HSV→RGB |
| `rgb_split.frag` | Fragment effect | Glitch: directional RGB offset |
| `ripple.frag` | Fragment effect | Warp: UV distortion via sin(dist*freq+time) |
| `vignette.frag` | Fragment effect | Blur/Post: edge darkening |

All other effect shaders (135 total) and source shaders (108 total) are embedded — not hot-reloadable from disk in the shipped build. The `shaders/` directory appears to be a legacy / reference subset.

---

## Section 20 partial: Cross-references

- **ProceduralSource base class** — `ProceduralSource.h/cpp`. 42+ audio uniforms auto-uploaded (see Section 9 for audio pipeline). Required by ClipInspector for param right-click reset (CLAUDE.md Pitfall 8: `pc->setDefaultValue(sp.defaultValue)` mandatory).
- **ShaderManager** — sources resolve shader by name via `ShaderManager::getProgram(shaderName_.c_str())` (`ProceduralSource.cpp:97`). Shader names have `source_` prefix convention.
- **FullscreenQuad** — all sources render via single quad.draw() on output FBO.
- **FeatureSnapshot** — source audio uniforms pulled from snapshot fields: rms, bandEnergies[7], beatPhase, barPhase, phrasePhase, spectralCentroid/Flux, onsetStrength/Detected, dominantPitch, pitchConfidence, detectedKey, keyIsMajor, structuralState, bpm, hcdf, chromagram[12], mfccs[13], detectedGenre, genreConfidence, energyState, sidechainPump, swingRatio, formantPresence, resonancePeak, reeseBass (see Section 1 — FeatureSnapshot schema).
- **Clip model** — `Clip::mediaType == MediaType::Source` selects a procedural source by `sourceType` string (matches `SourceRegistry` ID). VideoPlayer/ImageSequence are separate media types (`MediaType::Video`, `MediaType::ImageSequence`).
- **Layer Router wiring** — CompositorEngine (Slice 06?) saves per-layer textures in `layerOutputTextures_`; Renderer (Slice 07?) intercepts `source_layer_router` sources.
- **Signal routing** — every source param is a valid target for `u_src_*` uniform routing (SignalRegistry / RoutingEngine in Slice 04).
- **MilkDrop PCM** — `ProjectMSource::feedAudio()` called from the analysis pipeline (Slice 09?) or directly from AudioCallback (Slice 02?) — this slice doesn't reveal the caller.
- **Preset JSON** — `presets.json` manifest schema: top-level object, keys = preset names, values = `{mood, energy, style}`. User data JSON: `{favorites: [names], userPresets: [paths]}`.

---

### Summary of tally

Counting `registerSource("...", lambda)` literal calls: **101**, plus 7 wireframes via the `registerWireframe` helper lambda = **108 unique sources registered**.

---
