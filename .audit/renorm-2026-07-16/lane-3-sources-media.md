# Lane 3 Census — Sources + Media (src/sources/, src/media/)
Renorm audit 2026-07-16 · read-only · evidence = file:line · SOURCE = ground truth

## VERDICT
CLAUDE.md line 11 ("108 procedural sources") is CORRECT. The v2-section claim of
"64 procedural sources" (CLAUDE.md:578) is STALE/WRONG. Authoritative registry =
**108 sources across 18 categories, 759 params** (parser ground-truth; FEATURES.md
claims 754 — ~0.7% drift). No dead sources: every source shader referenced by the
registry is compiled in Renderer.cpp. Media layer (VideoPlayer/ImageSequence) is
transport-only and tempo-agnostic — BPM sync / in-out points / cuepoints do NOT
live here; they are Clip-struct + ClipInspector features applied externally.

## PRIORITY ANSWER — procedural source count
- **ACTUAL = 108** = 101 literal `registerSource("id",...)` calls (SourceRegistry.cpp,
  incl. projectm_visualizer) + 7 `registerWireframe(...)` calls (SourceRegistry.cpp:297-303).
- CLAUDE.md:11 "108" = RIGHT (headline). Its parenthetical sub-breakdown is loose:
  says "8 audio-visual" but the Audio-Visual *category* has 9 (audio_waveform +
  8 Phase-18). The functional groups it names (7 2D fractals, 8 3D ray-marched, 8
  torus, 3 simulation, 1 text, 1 text animator, 1 layer router) DO verify.
- CLAUDE.md:578 "64 procedural sources" = STALE by 44. Wrong.
- .harmony/FEATURES.md:910/931/2684 "108 sources, 18 categories" = CORRECT.
- docs/FEATURE_INVENTORY.md:775 "108 sources" CORRECT; :466 "~96 visible in browser
  vs 108 registered" and :606 "§19 says 40 — actual 108" both flag known drift.

## AUTHORITATIVE PER-CATEGORY BREAKDOWN (registry `category` field)
3D 24 | Geometric 11 | Lines 11 | Audio-Visual 9 | Math 8 | Pattern 8 | Fractal 7 |
Wireframe 7 | Nature 6 | Noise 3 | Particle 3 | Simulation 3 | Text 2 | Utility 2 |
Lighting 1 | MilkDrop 1 | Organic 1 | Routing 1  = 108
(FEATURES.md per-category counts all MATCH this. Functional cross-cut used by CLAUDE.md:
the "3D" category (24) = 8 ray-marched 3D-fractals + 8 torus/tunnel + 8 other 3D.)

## DOC-VERIFY (discrepancies: doc-file:line -> claim -> code truth)
1. CLAUDE.md:578 -> "64 procedural sources" -> ACTUAL 108 (SourceRegistry.cpp, 101+7). STALE.
2. CLAUDE.md:11 -> "8 audio-visual sources" -> category has 9 (SourceRegistry.cpp:103 audio_waveform
   + :998-1074 eight Phase-18). Off by 1.
3. .harmony/FEATURES.md:931/2684 -> "754 parameters" -> parser ground-truth 759. Minor (~+5).
4. .harmony/FEATURES.md:960 -> "3D (24 sources, 298 params)" -> 24 src OK, params=277 by parse
   (delta 21; likely torus shared-control counting). Source count fine; param count drifts.
5. .harmony/FEATURES.md:1008 -> "MilkDrop (1 source, 0 standard params)" -> ProjectMSource
   adds 5 params (ProjectMSource.cpp:13-17: Beat Sensitivity, Speed, Warp, Decay, Gamma). Undercount.
6. docs/FEATURE_INVENTORY.md:466 -> "~96 sources visible in browser (vs 108 registered)" ->
   CONFIRMED direction: SourcesBrowser.cpp uses NO SourceRegistry (0 refs); hand-maintained
   list. Exact gap = 6 registered ids absent from browser (see FLAGGED #1).
7. CLAUDE.md:578 / FEATURES.md:1159 -> "Video ... (MP4/MOV/AVI/MKV/WebM/HAP Alpha)" ->
   VideoPlayer.h:19 header comment lists only "MP4, MOV, QuickTime (H.264/H.265/ProRes),
   HAP/HAP Alpha, AVI" (no MKV/WebM). Actual container/codec support = whatever the linked
   FFmpeg build provides (generic avformat_open_input + avcodec_find_decoder, VideoPlayer.cpp:55,90).
   MKV/WebM likely work but are NOT code-guaranteed and contradict the in-code comment. UNVERIFIED.
8. FEATURES.md:1159/1170/1184-1186 -> media playback section credits VideoPlayer/ImageSequence
   with "BPM sync", "In/out points: draggable on timeline", "Beat division presets" ->
   NEITHER class has beatDivision, inPoint/outPoint, transportMode, or cuepoints. Transport
   enums are Loop/PingPong/OneShot only (VideoPlayer.h:53, ImageSequence.h:45). These are
   Clip-struct fields (FEATURES.md:2678) driven externally via setSpeed()/advanceFrame(dt).
   Doc attributes system-level features to the wrong layer.

## HEALTH READ — WORKS
- All 108 sources register cleanly; every referenced source_* shader (101 unique) is
  compiled in Renderer.cpp:1128+ (0 dead/missing shaders). Full API/registry enumeration
  works (ApiServer.cpp:756, TestServer.cpp:674 via getRegisteredIds()).
- Stateful sources (reaction_diffusion, cellular_automata, strange_attractor, gravity_well,
  fluid_dynamics + Julia/Menger/etc. trail/feedback) use ping-pong FBOs correctly
  (ProceduralSource.cpp:24-48, 101-125).
- VideoPlayer: FFmpeg decode -> RGBA -> GL, transport (speed/reverse/Loop|PingPong|OneShot/
  seek), thumbnail, alpha detect incl. HAP (VideoPlayer.cpp). ImageSequence: lazy per-frame
  GL upload, natural-sort, configurable fps, same transport (ImageSequence.cpp).
- Camera input is REAL but OUTSIDE this lane: juce::CameraDevice in MainComponent.cpp:2237-2254
  (no camera code in src/media).

## HEALTH READ — DOESN'T / FLAGGED
1. SELECTABILITY GAP: 6 registered sources have NO id token in SourcesBrowser.cpp
   (hand-maintained, 0 SourceRegistry refs): strange_attractor, gravity_well, fluid_dynamics,
   text_animator, layer_router, projectm_visualizer (all Phase-20 "system" sources). They are
   registered + API-selectable but appear NOT reachable via the GUI browser. layer_router &
   projectm_visualizer MAY be intentionally hidden (dedicated UI); the other 4 look like
   genuine omissions. Cross-lane (src/ui) — flag for UI lane to confirm.
2. projectm_visualizer is BUILD-CONDITIONAL: gated on AUDIODNA_HAS_PROJECTM
   (CMakeLists.txt:20,371-373; ProjectMSource.h:9). Without libprojectM-4 linked, render()
   outputs a flat dark-purple placeholder (ProjectMSource.cpp:170-178) — registered but a no-op
   visual. Also requires an external feedAudio() caller + scanned .milk presets to do anything.
3. ORPHAN SHADER: source_fluid_display is compiled in Renderer.cpp but referenced by NO source
   (fluid_dynamics uses shaderName "source_fluid_dynamics"; base ProceduralSource renders a
   single shader). Likely an intended-but-unwired 2nd display pass. Compiled dead code.
4. ProjectMSource applyParams: "Gamma" param is a no-op placeholder (ProjectMSource.cpp:246-250)
   — exposed in inspector, does nothing.
5. Cuepoint transport semantics exist only at Clip/UI level; SessionRecorder recordCuepointJump()
   has 0 callers (FEATURES.md:2148) — dead recording path (adjacent, src/recording lane).

## FULL CENSUS (108 sources, one row each) + MEDIA LAYER
| id | display name | category | #params | shader | notes |
|---|---|---|---:|---|---|
| apollonian_3d | Apollonian 3D | 3D | 14 | source_apollonian_3d |  |
| burning_ship_3d | Burning Ship 3D | 3D | 14 | source_burning_ship_3d |  |
| checker_torus | Checker Torus | 3D | 14 | source_checker_torus |  |
| crystal_cavern | Crystal Cavern | 3D | 6 | source_crystal_cavern |  |
| dna_helix | DNA Helix | 3D | 4 | source_dna |  |
| dual_plane_drift | Dual Plane Drift | 3D | 5 | source_dual_plane_drift |  |
| infinite_corridor | Infinite Corridor | 3D | 6 | source_infinite_corridor |  |
| julia_set_3d | Julia Set 3D | 3D | 17 | source_julia_set_3d |  |
| kifs | Kaleidoscopic IFS | 3D | 16 | source_kifs |  |
| mandelbulb | Mandelbulb | 3D | 16 | source_mandelbulb |  |
| menger_sponge | Menger Sponge | 3D | 14 | source_menger_sponge |  |
| newton_3d | Newton 3D | 3D | 14 | source_newton_3d |  |
| orbit_chamber | Orbit Chamber | 3D | 7 | source_orbit_chamber |  |
| ribbed_vortex | Ribbed Vortex | 3D | 14 | source_ribbed_vortex |  |
| rotating_cube_map | Rotating Cube Map | 3D | 5 | source_rotating_cube_map |  |
| scroll_plane | Scroll Plane | 3D | 5 | source_scroll_plane |  |
| sierpinski_tetra | Sierpinski Tetrahedron | 3D | 13 | source_sierpinski_tetra |  |
| spiral_tunnel | Spiral Tunnel | 3D | 5 | source_spiral_tunnel |  |
| spiral_vortex | Spiral Vortex | 3D | 14 | source_spiral_vortex |  |
| striped_torus | Striped Torus | 3D | 14 | source_striped_torus |  |
| torus_hole | Torus Hole | 3D | 19 | source_torus_hole |  |
| twisted_torus | Twisted Torus | 3D | 14 | source_twisted_torus |  |
| wormhole | Wormhole | 3D | 14 | source_wormhole |  |
| wormhole_tunnel | Wormhole Tunnel | 3D | 13 | source_wormhole_tunnel |  |
| audio_waveform | Audio Waveform | Audio-Visual | 4 | source_audio_waveform |  |
| band_tower | Band Tower | Audio-Visual | 6 | source_band_tower |  |
| chromatic_ring | Chromatic Ring | Audio-Visual | 4 | source_chromatic_ring |  |
| cymatics | Cymatics | Audio-Visual | 3 | source_cymatics |  |
| spectral_ring | Spectral Ring | Audio-Visual | 5 | source_spectral_ring |  |
| spectral_waterfall | Spectral Waterfall | Audio-Visual | 3 | source_spectral_waterfall |  |
| spectrum_landscape | Spectrum Landscape | Audio-Visual | 5 | source_spectrum_landscape |  |
| structural_landscape | Structural Landscape | Audio-Visual | 6 | source_structural_landscape |  |
| timbral_nebula | Timbral Nebula | Audio-Visual | 6 | source_timbral_nebula |  |
| apollonian | Apollonian Gasket | Fractal | 6 | source_apollonian |  |
| burning_ship | Burning Ship | Fractal | 9 | source_burning_ship |  |
| julia_set | Julia Set | Fractal | 9 | source_julia_set |  |
| kaleido_fractal | Kaleidoscopic Fractal | Fractal | 6 | source_kaleido_fractal |  |
| mandelbrot | Mandelbrot / Julia | Fractal | 11 | source_mandelbrot |  |
| newton_fractal | Newton Fractal | Fractal | 6 | source_newton_fractal |  |
| sierpinski | Sierpinski | Fractal | 7 | source_sierpinski |  |
| astral_grid | Astral Grid | Geometric | 6 | source_astral_grid |  |
| color_gradient | Color Gradient | Geometric | 4 | source_color_gradient |  |
| dot_matrix_wave | Dot Matrix Wave | Geometric | 6 | source_dot_matrix |  |
| geometric_tunnel | Geometric Tunnel | Geometric | 4 | source_geometric_tunnel |  |
| hex_grid | Hex Grid | Geometric | 6 | source_hex_grid |  |
| infinite_zoom | Infinite Zoom | Geometric | 4 | source_infinite_zoom |  |
| moire_interference | Moire Interference | Geometric | 7 | source_moire_interference |  |
| radar_sweep | Radar Sweep | Geometric | 4 | source_radar |  |
| radial_burst | Radial Burst | Geometric | 7 | source_radial_burst |  |
| sacred_geometry | Sacred Geometry | Geometric | 7 | source_sacred_geometry |  |
| shape_generator | Shape Generator | Geometric | 5 | source_shape_generator |  |
| laser_scanner | Laser Scanner | Lighting | 6 | source_laser_scanner |  |
| angular_grid | Angular Grid | Lines | 6 | source_angular_grid |  |
| fractal_tree | Fractal Tree | Lines | 6 | source_fractal_tree |  |
| laser_scan | Laser Scan | Lines | 5 | source_laser_scan |  |
| line_generator | Line Generator | Lines | 9 | source_line_generator |  |
| lissajous | Lissajous | Lines | 6 | source_lissajous |  |
| moire_lines | Moire Lines | Lines | 5 | source_moire_lines |  |
| polygon_lines | Polygon Lines | Lines | 6 | source_polygon_lines |  |
| spirograph | Spirograph | Lines | 5 | source_spirograph |  |
| star_burst | Star Burst | Lines | 5 | source_star_burst |  |
| waveform_lines | Waveform Lines | Lines | 7 | source_waveform_lines |  |
| zigzag_lines | Zigzag Lines | Lines | 5 | source_zigzag_lines |  |
| fermat_spiral | Fermat Spiral Garden | Math | 6 | source_fermat_spiral |  |
| fibonacci_spiral | Fibonacci Spiral | Math | 5 | source_fibonacci |  |
| hyperbolic_tiling | Hyperbolic Tiling | Math | 6 | source_hyperbolic_tiling |  |
| lissajous_weaver | Lissajous Weaver | Math | 7 | source_lissajous_weaver |  |
| penrose_pulse | Penrose Pulse | Math | 5 | source_penrose_pulse |  |
| rose_curves | Rose Curves | Math | 5 | source_rose |  |
| superformula | Superformula | Math | 6 | source_superformula |  |
| truchet_labyrinth | Truchet Labyrinth | Math | 5 | source_truchet |  |
| projectm_visualizer | MilkDrop Visualizer | MilkDrop | 5 | (none-libprojectM) | optional dep |
| cellular_automata | Cellular Automata | Nature | 5 | source_cellular_automata | stateful |
| electric_arc | Electric Arc | Nature | 6 | source_electric_arc |  |
| fire | Fire | Nature | 4 | source_fire |  |
| fire_wall | Fire Wall | Nature | 6 | source_fire_wall |  |
| reaction_diffusion | Reaction-Diffusion | Nature | 4 | source_reaction_diffusion | stateful |
| water_caustics | Water Caustics | Nature | 6 | source_water_caustics |  |
| perlin_noise | Perlin Noise | Noise | 4 | source_perlin_noise |  |
| plasma | Plasma | Noise | 4 | source_plasma |  |
| voronoi | Voronoi | Noise | 4 | source_voronoi |  |
| metaballs | Metaballs | Organic | 5 | source_metaballs |  |
| lightning_storm | Lightning Storm | Particle | 4 | source_lightning |  |
| particle_nebula | Particle Nebula | Particle | 4 | source_nebula |  |
| starfield | Starfield | Particle | 4 | source_starfield |  |
| bump_light | Bump Light | Pattern | 5 | source_bump_light |  |
| checkerboard | Checkerboard | Pattern | 4 | source_checkerboard |  |
| concentric_rings | Concentric Rings | Pattern | 5 | source_concentric_rings |  |
| glitch_grid | Glitch Grid | Pattern | 4 | source_glitch_grid |  |
| line_pattern | Line Pattern | Pattern | 5 | source_line_pattern |  |
| sine_oscillator | Sine Oscillator | Pattern | 6 | source_sine_oscillator |  |
| spiral_pattern | Spiral Pattern | Pattern | 5 | source_spiral_pattern |  |
| terrain_lines | Terrain Lines | Pattern | 6 | source_terrain_lines |  |
| layer_router | Layer Router | Routing | 1 | source_layer_router |  |
| fluid_dynamics | Fluid Dynamics | Simulation | 6 | source_fluid_dynamics | stateful |
| gravity_well | Gravity Well | Simulation | 6 | source_gravity_well | stateful |
| strange_attractor | Strange Attractor | Simulation | 6 | source_strange_attractor | stateful |
| text_animator | Text Animator | Text | 8 | source_text_animator |  |
| text_wall | Scrolling Text Wall | Text | 4 | source_text_wall |  |
| solid_color | Solid Color | Utility | 3 | source_solid_color |  |
| strobe_light | Strobe Light | Utility | 8 | source_strobe_light |  |
| wire_cone | Wireframe Cone | Wireframe | 9 | source_wireframe_3d |  |
| wire_cube | Wireframe Cube | Wireframe | 9 | source_wireframe_3d |  |
| wire_cylinder | Wireframe Cylinder | Wireframe | 9 | source_wireframe_3d |  |
| wire_icosahedron | Wireframe Icosahedron | Wireframe | 9 | source_wireframe_3d |  |
| wire_sphere | Wireframe Sphere | Wireframe | 9 | source_wireframe_3d |  |
| wire_torus | Wireframe Torus | Wireframe | 9 | source_wireframe_3d |  |
| wire_wolf | Wireframe Wolf | Wireframe | 9 | source_wireframe_3d |  |

## MEDIA LAYER (src/media/)
| component | file | responsibilities | notes |
|---|---|---|---|
| VideoPlayer | src/media/VideoPlayer.{h,cpp} | FFmpeg demux/decode (avformat/avcodec/swscale) -> RGBA -> GL texture (glTexSubImage2D); transport speed/reverse/{Loop,PingPong,OneShot}/seekTo[0,1]; alpha detect (RGBA/BGRA/ARGB/ABGR/YUVA/PAL8/HAP); 2-thread decode; thumbnail | NO BPM sync / in-out / cuepoints / transportMode; codec set = linked FFmpeg build; MKV/WebM not in header comment |
| ImageSequence | src/media/ImageSequence.{h,cpp} | folder/multi-file as clip; formats .png/.jpg/.jpeg/.bmp/.tiff/.gif; natural-sort; lazy per-frame GL upload+cache; configurable fps (default 10); same transport as VideoPlayer | juce::ImageFileFormat load; duration = frames/fps |
| ProjectMSource (in src/sources) | src/sources/ProjectMSource.{h,cpp} | MilkDrop via libprojectM-4; PCM feed; GL state save/restore; preset next/prev/random/lock; audio-driven auto-switch | build-conditional; placeholder if unbuilt |
| ProjectMPresetManager | src/sources/ProjectMPresetManager.{h,cpp} | scan .milk dirs, mood/energy tags, favorites, JSON manifest, search/nav | |
| PresetSelector | src/sources/PresetSelector.{h,cpp} | audio-driven MilkDrop preset switching (structural/energy/bar-phase gated, min 4 bars) | |
| camera input | NOT in src/media | juce::CameraDevice — MainComponent.cpp:2237-2254 | out-of-lane |
| static image load | NOT in src/media | juce::ImageFileFormat (TextureManager / ImageSequence) | |

## VIDEOPLAYER TRANSPORT/CODEC DETAIL
- Codecs: generic — avcodec_find_decoder(codec_id) (VideoPlayer.cpp:90); container via
  avformat_open_input (:55). Header claims H.264/H.265/ProRes/HAP/HAP-Alpha/AVI (VideoPlayer.h:19).
- Transport modes: LoopMode{Loop,PingPong,OneShot} (VideoPlayer.h:53). speed(float), reverse(bool),
  playing(bool), seekTo(normalized[0,1]) — all atomics for msg/GL thread split.
- BPM sync / in-out points / cuepoints: ABSENT (belong to Clip struct + ClipInspector, applied
  externally by scaling setSpeed()/advanceFrame(dt)).
- Alpha: hasAlpha via pix_fmt or AV_CODEC_ID_HAP (VideoPlayer.cpp:144-152).

