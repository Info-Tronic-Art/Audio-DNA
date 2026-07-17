# Lane 2 — Render + Effects Census (SOURCE truth)

Audit date: 2026-07-16 · Scope: `src/render/`, `src/effects/`, `shaders/`
Method: counts derived directly from source (parsed `EffectLibrary.cpp`,
`Renderer.cpp` compile table, `EmbeddedShaders.h`). Read-only. Evidence = file:line.

---

## 0. AUTHORITATIVE COUNTS (resolves the doc contradiction)

| Quantity | SOURCE TRUTH | Evidence |
|----------|:---:|----------|
| **Total registered effects** | **135** | `EffectLibrary.cpp` — 135 `registerEffect({...})` struct calls (parsed); `getNumEffects()` returns `defs_.size()` |
| **Effect categories** | **11** | warp, color, glitch, blur, 3d, pattern, animation, time, blend, composite, audio |
| **Total effect parameters** | **333** | sum of param entries across all 135 defs |
| **Transitions** | **15** | `EmbeddedShaders::transition*` (15 vars) + 15 `compile("transition_…")` calls in `Renderer.cpp` + 15 cases in `getTransitionShaderName` |
| **Effects with compiled program** | **135 / 135** | every registry shaderkey has a matching `compile()` call in `Renderer.cpp` — 0 orphaned effects |
| **Embedded shader strings** | **244** | `inline const char*` count in `EmbeddedShaders.h` (135 effect + 15 transition + 93 `source*` + ~1 helper/keying set) |
| **Generative `source*` shaders (embedded)** | **93** | `source*` vars in `EmbeddedShaders.h` (source census is L1/other-lane scope; noted here because they live in `src/render/`) |
| **Shaders on disk (`shaders/`)** | **5** | `hue_shift.frag`, `rgb_split.frag`, `ripple.frag`, `vignette.frag`, `passthrough.vert` — all duplicate embedded versions; legacy/unused (see FLAGGED) |
| **`compile()` calls in Renderer.cpp** | **275** | 135 effects + 15 transitions + `deck_transition` + 14 keying variants + ~93 sources + helpers/aliases |

**Bottom line: the codebase ships 135 effects (11 categories, 333 params) + 15
transitions, all embedded in `EmbeddedShaders.h` and all compiled at startup.
"110", "96", and "115" in CLAUDE.md are stale.**

### Per-category effect + parameter counts (SOURCE)

| Category | Effects | Params | Avg |
|----------|:---:|:---:|:---:|
| color | 31 | 64 | 2.06 |
| warp | 27 | 67 | 2.48 |
| pattern | 19 | 50 | 2.63 |
| glitch | 15 | 40 | 2.67 |
| blur | 10 | 21 | 2.10 |
| 3d | 9 | 20 | 2.22 |
| animation | 6 | 23 | 3.83 |
| time | 6 | 14 | 2.33 |
| blend | 5 | 9 | 1.80 |
| audio | 4 | 12 | 3.00 |
| composite | 3 | 13 | 4.33 |
| **Total** | **135** | **333** | **2.47** |

Note on "temporal": 6 effects are in the **time** category, but only **4** carry
the `temporal=true` flag (Echo, Posterize Time, Freeze, Channel Delay —
`EffectLibrary.cpp:649,654,658,792`). Screen Split and Frame Stutter are in the
time category but registered `temporal=false` (they use CompositorEngine ring
buffers, not `u_prev_frame`). CLAUDE.md's "6 temporal time effects" = category
count, not flag count.

---

## 1. FULL EFFECT CENSUS (135)

Source: `src/effects/EffectLibrary.cpp::registerDefaults()`.
Display name = `slot.effectName`; Shader Key = `def.shaderName` (compiled program name).

| # | Effect (display) | Category | Shader Key | Params | Temporal |
|---|------------------|----------|-----------|:------:|:--------:|
| 1 | Ripple | warp | `ripple` | 3 |  |
| 2 | Bulge | warp | `bulge` | 3 |  |
| 3 | Wave | warp | `wave` | 3 |  |
| 4 | Liquid | warp | `liquid` | 2 |  |
| 5 | Hue Shift | color | `hue_shift` | 1 |  |
| 6 | Saturation | color | `saturation` | 1 |  |
| 7 | Brightness | color | `brightness` | 1 |  |
| 8 | Duotone | color | `duotone` | 7 |  |
| 9 | Chromatic Aberration | color | `chromatic_aberration` | 2 |  |
| 10 | Pixel Scatter | glitch | `pixel_scatter` | 2 |  |
| 11 | RGB Split | glitch | `rgb_split` | 2 |  |
| 12 | Block Glitch | glitch | `block_glitch` | 2 |  |
| 13 | Scanlines | glitch | `scanlines` | 2 |  |
| 14 | Gaussian Blur | blur | `gaussian_blur` | 1 |  |
| 15 | Zoom Blur | blur | `zoom_blur` | 3 |  |
| 16 | Shake | blur | `shake` | 2 |  |
| 17 | Vignette | blur | `vignette` | 2 |  |
| 18 | Kaleidoscope | warp | `kaleidoscope` | 2 |  |
| 19 | Fisheye | warp | `fisheye` | 1 |  |
| 20 | Swirl | warp | `swirl` | 2 |  |
| 21 | Invert | color | `invert` | 1 |  |
| 22 | Posterize | color | `posterize` | 1 |  |
| 23 | Color Shift | color | `color_shift` | 3 |  |
| 24 | Thermal | color | `thermal` | 1 |  |
| 25 | Contrast | color | `color_matrix` | 2 |  |
| 26 | Digital Rain | glitch | `digital_rain` | 2 |  |
| 27 | Noise | glitch | `noise_overlay` | 2 |  |
| 28 | Mirror | glitch | `mirror` | 2 |  |
| 29 | Pixelate | glitch | `pixelate` | 1 |  |
| 30 | Motion Blur | blur | `motion_blur` | 2 |  |
| 31 | Glow | blur | `glow` | 2 |  |
| 32 | Edge Detect | blur | `edge_detect` | 1 |  |
| 33 | Perspective Tilt | 3d | `perspective_tilt` | 2 |  |
| 34 | Cylinder Wrap | 3d | `cylinder_wrap` | 2 |  |
| 35 | Sphere Wrap | 3d | `sphere_wrap` | 1 |  |
| 36 | Tunnel | 3d | `tunnel` | 2 |  |
| 37 | Page Curl | 3d | `page_curl` | 2 |  |
| 38 | Parallax Layers | 3d | `parallax_layers` | 2 |  |
| 39 | Polar Coords | warp | `polar_coords` | 1 |  |
| 40 | Twirl | warp | `twirl` | 2 |  |
| 41 | Shear | warp | `shear` | 2 |  |
| 42 | Elastic Bounce | warp | `elastic_bounce` | 2 |  |
| 43 | Ripple Pond | warp | `ripple_pond` | 2 |  |
| 44 | Diamond Distort | warp | `diamond_distort` | 2 |  |
| 45 | Barrel Distort | warp | `barrel_distort` | 1 |  |
| 46 | Sine Grid | warp | `sine_grid` | 2 |  |
| 47 | Glitch Displace | **warp** | `glitch_displace` | 2 |  |
| 48 | Sepia | color | `sepia` | 1 |  |
| 49 | Cross Process | color | `cross_process` | 1 |  |
| 50 | Split Tone | color | `split_tone` | 3 |  |
| 51 | Color Halftone | color | `color_halftone` | 2 |  |
| 52 | Dither | color | `ordered_dither` | 2 |  |
| 53 | Heat Map | color | `heat_map` | 1 |  |
| 54 | Selective Color | color | `selective_color` | 2 |  |
| 55 | Film Grain | color | `film_grain` | 2 |  |
| 56 | Gamma Levels | color | `gamma_levels` | 3 |  |
| 57 | Solarize | color | `solarize` | 2 |  |
| 58 | CRT | pattern | `crt_simulation` | 2 |  |
| 59 | VHS | pattern | `vhs_effect` | 2 |  |
| 60 | ASCII Art | pattern | `ascii_art` | 2 |  |
| 61 | Dot Matrix | pattern | `dot_matrix` | 2 |  |
| 62 | Crosshatch | pattern | `crosshatch` | 2 |  |
| 63 | Emboss | pattern | `emboss` | 2 |  |
| 64 | Oil Paint | pattern | `oil_paint` | 1 |  |
| 65 | Pencil Sketch | pattern | `pencil_sketch` | 2 |  |
| 66 | Voronoi Glass | pattern | `voronoi_glass` | 2 |  |
| 67 | Cross Stitch | pattern | `cross_stitch` | 2 |  |
| 68 | Night Vision | pattern | `night_vision` | 1 |  |
| 69 | Strobe | animation | `strobe` | 2 |  |
| 70 | Pulse | animation | `pulse` | 2 |  |
| 71 | Slit Scan | animation | `slit_scan` | 2 |  |
| 72 | Double Exposure | blend | `double_exposure` | 2 |  |
| 73 | Frosted Glass | blend | `frosted_glass` | 2 |  |
| 74 | Prism | blend | `prism_refract` | 2 |  |
| 75 | Rain on Glass | blend | `rain_on_glass` | 2 |  |
| 76 | Hexagonalize | blend | `hexagonalize` | 1 |  |
| 77 | Greyscale | color | `greyscale` | 2 |  |
| 78 | Threshold | color | `threshold` | 2 |  |
| 79 | Exposure | color | `exposure` | 1 |  |
| 80 | Vibrance | color | `vibrance` | 1 |  |
| 81 | Quad Mirror | warp | `quad_mirror` | 2 |  |
| 82 | Flip | warp | `flip` | 2 |  |
| 83 | Warp Field | warp | `warp_field` | 3 |  |
| 84 | Sharpen | blur | `sharpen` | 2 |  |
| 85 | Pixel Explosion | glitch | `pixel_explosion` | 4 |  |
| 86 | Color Flash | glitch | `color_flash` | 5 |  |
| 87 | Slide Wrap | warp | `slide_wrap` | 2 |  |
| 88 | Dot Field | 3d | `dot_field` | 3 |  |
| 89 | Triangulate | pattern | `triangulate` | 2 |  |
| 90 | Auto Mask | color | `auto_mask` | 3 |  |
| 91 | Chroma Key | color | `chromakey_effect` | 4 |  |
| 92 | Tile Grid | warp | `tile_grid` | 4 |  |
| 93 | Spot Zoom | warp | `spot_zoom` | 6 |  |
| 94 | Neon Edge | pattern | `neon_edge` | 4 |  |
| 95 | Cartoon Ink | pattern | `cartoon_ink` | 4 |  |
| 96 | Pop Raster | pattern | `pop_raster` | 4 |  |
| 97 | Palette Remap | color | `palette_remap` | 3 |  |
| 98 | Color Grade | color | `lut_grade` | 1 |  |
| 99 | Bendoscope | warp | `bendoscope` | 3 |  |
| 100 | UV Remap | warp | `uv_remap` | 3 |  |
| 101 | Liquid Morph | warp | `liquid_morph` | 3 |  |
| 102 | Edge Blur | blur | `edge_blur` | 2 |  |
| 103 | Brush Strokes | pattern | `brush_strokes` | 4 |  |
| 104 | Fragment Burst | glitch | `fragment_burst` | 4 |  |
| 105 | Signal Destroy | glitch | `signal_destroy` | 3 |  |
| 106 | Line Cloner | composite | `line_cloner` | 5 |  |
| 107 | Radial Cloner | composite | `radial_cloner` | 4 |  |
| 108 | Cube Scatter | composite | `cube_scatter` | 4 |  |
| 109 | Zoom Warp | warp | `infinite_zoom` | 4 |  |
| 110 | Bump Light | pattern | `bump_light` | 4 |  |
| 111 | Echo | time | `ghost_trails` | 2 | Y |
| 112 | Posterize Time | time | `frame_hold` | 2 | Y |
| 113 | Freeze | time | `time_freeze` | 1 | Y |
| 114 | Screen Split | time | `screen_split` | 4 |  |
| 115 | Frame Stutter | time | `frame_delay` | 2 |  |
| 116 | Point Zoom | animation | `feedback` | 8 |  |
| 117 | Directional Feedback | animation | `directional_feedback` | 6 |  |
| 118 | Harmonic Displacement | audio | `harmonic_displace` | 3 |  |
| 119 | Timbral Mosaic | audio | `timbral_mosaic` | 3 |  |
| 120 | Structural Morph | audio | `structural_morph` | 3 |  |
| 121 | Pitch Chromatic Shift | color | `pitch_chroma_shift` | 3 |  |
| 122 | Key Palette | color | `key_palette` | 3 |  |
| 123 | Transient Flash | animation | `transient_flash` | 3 |  |
| 124 | Beat Ripple | audio | `beat_ripple` | 3 |  |
| 125 | Rhythm Slice | glitch | `rhythm_slice` | 3 |  |
| 126 | Density Wave | warp | `density_wave` | 3 |  |
| 127 | Chroma Dissolve | color | `chroma_dissolve` | 2 |  |
| 128 | Luminance Terrain | 3d | `luma_terrain` | 3 |  |
| 129 | Voxel Matrix | 3d | `voxel_matrix` | 3 |  |
| 130 | Monitor Wall | pattern | `monitor_wall` | 4 |  |
| 131 | Drop Shadow | blur | `drop_shadow` | 4 |  |
| 132 | Channel Delay | time | `channel_delay` | 3 | Y |
| 133 | Topographic Lines | pattern | `topo_lines` | 4 |  |
| 134 | Data Corruption | glitch | `data_corrupt` | 3 |  |
| 135 | Glitch Sort | glitch | `glitch_sort` | 3 |  |

No duplicate display names, no duplicate shaderkeys. "Contrast" maps to shaderkey
`color_matrix`; "Zoom Warp" maps to `infinite_zoom`; "Point Zoom" maps to
`feedback`; "Prism" maps to `prism_refract` — display name and shaderkey diverge
for these, which is exactly why the display→shaderkey resolution rule matters.

---

## 2. TRANSITION CENSUS (15)

Source: `EmbeddedShaders::transition*` (15 vars) → compiled in `Renderer.cpp`
→ selected by `CompositorEngine::getTransitionShaderName(Layer::MixMode)`
(`CompositorEngine.cpp:1065-1089`) → rendered by `applyTransition()` via
`transitionFBO_` (`CompositorEngine.cpp:1090-1151`, bind at :1121).

| # | MixMode enum | Shader key | Embedded var |
|---|--------------|-----------|--------------|
| 1 | Dissolve (default) | `transition_dissolve` | `transitionDissolve` |
| 2 | Cut | `transition_cut` | `transitionCut` |
| 3 | WipeLeft | `transition_wipe_left` | `transitionWipeLeft` |
| 4 | WipeRight | `transition_wipe_right` | `transitionWipeRight` |
| 5 | WipeUp | `transition_wipe_up` | `transitionWipeUp` |
| 6 | WipeDown | `transition_wipe_down` | `transitionWipeDown` |
| 7 | PushLeft | `transition_push_left` | `transitionPushLeft` |
| 8 | PushRight | `transition_push_right` | `transitionPushRight` |
| 9 | PushUp | `transition_push_up` | `transitionPushUp` |
| 10 | PushDown | `transition_push_down` | `transitionPushDown` |
| 11 | ZoomIn | `transition_zoom_in` | `transitionZoomIn` |
| 12 | ZoomOut | `transition_zoom_out` | `transitionZoomOut` |
| 13 | WipeEllipse | `transition_iris` | `transitionIris` |
| 14 | Flip | `transition_flip_h` | `transitionFlipH` |
| 15 | ToBlack | `transition_fade_black` | `transitionFadeBlack` |

Plus a separate `deck_transition` shader (`deckTransition`, cross-deck A/B
blend) — not counted among the 15 clip-to-clip transitions.

---

## 3. RENDER PIPELINE STAGES

### Renderer (`src/render/Renderer.cpp`, `Renderer.h`)
- **Frame loop entry**: `Renderer::renderOpenGL()` (`Renderer.cpp:106`), a
  `juce::OpenGLRenderer` callback. Pulls audio `FeatureSnapshot`, runs
  `mappingEngine_.processFrame(*snap, effectChain_)` (`Renderer.cpp:210`) — the
  **live** audio→param path. `UniformBridge` is NOT used (see FLAGGED).
- **Shader compilation**: `compileAllShaders()` — 275 `compile("key", EmbeddedShaders::var)`
  calls (`Renderer.cpp:1005-~1290`); a lambda wrapping `shaderMgr_.compileProgram`
  (`Renderer.cpp:998`). All from embedded source strings.
- **Owns**: `CompositorEngine compositor_` (`Renderer.h:268`), `EffectChain effectChain_`
  (global chain, `Renderer.h:176`), `MappingEngine mappingEngine_` (`Renderer.h:178`),
  `compTransformFBO_`/`compTransformTexture_` (composition transform,
  `Renderer.h:235-236`), `prevDeckFBO_`/`prevDeckTexture_` (cross-deck transition,
  `Renderer.h:243-244`).

### CompositorEngine (`src/render/CompositorEngine.cpp`, `.h`) — v2 layer pipeline
- **Entry**: `compositeDeck()` (`CompositorEngine.cpp:637`).
- **Per-clip effect chain**: `applyClipEffects()` (`:232`) ping-pongs between
  `effectFBO_A_`/`effectFBO_B_` (`CompositorEngine.h:106-109`); resolves shaders via
  `effectLibrary_->getEffectDef(slot.effectName)->shaderName` (`:245`, `:258`) then
  `shaderMgr.getProgram(def->shaderName)` (`:312`). **This confirms the
  display-name→shaderkey rule.** Special-cases `screen_split` (`:263`) and
  `frame_delay` (`:276`) → ring buffers.
- **Stages** (matches CLAUDE.md deck-compositing description):
  `getClipTexture` → `applyClipEffects` → `applyTransition` (if crossfading) →
  layer effects (reuses `applyClipEffects`, `:753`) → `applyClipTransform`/`applyLayerTransform`
  → `applyLayerKeying` (if Transparent) → `blendLayerOntoAccumulator`.
  FX-Only layers: `applyFXOnlyLayer` (`:528`). Mask layers: `applyMaskLayer` (`:588`).
- **FBOs owned** (`CompositorEngine.h:98-117`): `accumulatorFBO_/Tex_`,
  `scratchFBO_/Tex_`, `effectFBO_A_/Tex_A_`, `effectFBO_B_/Tex_B_`,
  `transitionFBO_/Tex_`, `feedbackFBO_/Tex_`, plus per-layer output FBO maps
  (`layerOutputFBOs_`, `:255`).
- **Temporal buffers**: `TemporalBuffer` (`saveToTemporalBuffer` `:1211`) for
  trails/freeze; `FrameRingBuffer` (`pushFrameToRing` `:1270`, `getFrameFromRing`
  `:1293`) for Screen Split / Frame Stutter.
- **Audio uniforms**: `uploadAudioUniforms()` (`:1398`) feeds extended audio
  uniforms to shaders (the audio-native effects).

### ShaderManager (`src/render/ShaderManager.cpp`, `.h`)
- `compileProgram(name, vert, frag)` from source strings (used by all embedded
  shaders); `compileProgramFromFiles(name, vertFile, fragFile)` from `shaders/` dir.
- Uniform-location cache per program (`getUniformLocation`, `:89`).
- **Hot-reload**: `reloadAll()` (`:109`) only reloads programs that were compiled
  *from files* (skips any with empty `vertFile`/`fragFile`, `:115-116`). Since
  every shipped shader is compiled from embedded strings, hot-reload is inert in
  practice (see FLAGGED).

### TextureManager (`src/render/TextureManager.cpp`, `.h`)
- Image → `GL_TEXTURE_2D` upload; JUCE ARGB→RGBA conversion + Y-flip;
  `glTexSubImage2D` fast path when size unchanged. Used for image clips + camera.

### UniformBridge (`src/effects/UniformBridge.cpp`, `.h`)
- `applyDemoMappings()` hard-wires 4 effects by index (Ripple/Hue Shift/RGB
  Split/Vignette). **Dead** — superseded by MappingEngine (see FLAGGED).

### FullscreenQuad / LUTLoader
- `FullscreenQuad` — VAO/VBO fullscreen triangle used by every pass.
- `LUTLoader` — loads color LUT images (for `lut_grade` / Color Grade effect).

---

## 4. FEEDBACK SYSTEM + PRESETS

Two coexisting feedback mechanisms (both live):
1. **Per-layer `FeedbackProcessor`** (`src/render/FeedbackProcessor.cpp`) — owns an
   FBO ping-pong pair, blends current+previous frame via the `feedback_blend`
   shader (`:83`). Instantiated per-layer in `CompositorEngine` via
   `getOrCreateFeedbackProcessor()` (`CompositorEngine.cpp:1176`), invoked at
   `CompositorEngine.cpp:743-744` (`feedbackProcessors_` map, `CompositorEngine.h:143`).
2. **`CompositorEngine::updateFeedbackBuffer()`** (`CompositorEngine.cpp:1153`) —
   single-buffer passthrough copy of the accumulator into `feedbackFBO_`, exposed
   as `getFeedbackTexture()` for feedback-as-source use.

**Feedback presets: exactly 6** (`FeedbackProcessor::getPresets()`,
`FeedbackProcessor.cpp:131-142`): Zoom In, Spiral, Drift, Kaleidoscope, Echo,
Stretch. Wired into UI at `LayerInspector.cpp:419-433`. Matches CLAUDE.md "6 presets".

---

## 5. ISF IMPORT PATH (verified)

- **Parse/convert works**: `ISFShaderLoader::parseISFFile` / `parseISFSource`
  (`ISFShaderLoader.cpp:4,15`) extract JSON metadata + GLSL body and normalize ISF
  INPUTS (float/bool/long) to [0,1]. `convertToGLSL` (`:117`) wraps ISF source with
  a GLSL 410 header + ISF compat `#define`s (TIME, RENDERSIZE, IMG_*, gl_FragColor
  rewrite). These are functional.
- **Registration**: driven from `MainComponent::handleImportISF()`
  (`MainComponent.cpp:2400-2454`): parses → converts → builds an `EffectDef`
  (name `"ISF: <name>"`, category `"isf"`, shaderkey `"isf_<name>"`) → calls
  `EffectLibrary::registerDynamic(def)` (`:2443`). The effect DOES appear in the
  registry/FX browser.
- **BROKEN — compile step is a TODO no-op**: the converted `glsl` string
  (`MainComponent.cpp:2426`) is **never compiled or queued** for the GL thread. The
  comment at `:2447-2448` says "ShaderManager needs GL context. Queue for GL thread
  compilation" but no queue exists; the code only prints a debug line + shows a
  "Import Successful" dialog. Result: `shaderMgr.getProgram("isf_<name>")` returns
  null at render → the imported effect is a **phantom** (registered, visible,
  non-rendering).
- **Dead alt-path**: `ISFShaderLoader::registerISFEffect()`
  (`ISFShaderLoader.cpp:210-243`) builds a def, prints, and returns true WITHOUT
  registering or compiling — self-documented as incomplete; never called (MainComponent
  inlines the def + `registerDynamic` instead).

---

## 6. FLAGGED (health) — dead / stubbed / divergent

1. **ISF import compile step is a no-op** (`MainComponent.cpp:2426-2454`) —
   converted GLSL never compiled/queued; imported ISF effects register but never
   render. Success dialog is misleading. CLAUDE.md/FEATURES claim "ISF shader
   import" as a working capability. **HIGH.**
2. **`ISFShaderLoader::registerISFEffect` is dead code** (`ISFShaderLoader.cpp:210-243`)
   — no-op stub, never called. `EffectLibrary::registerDynamic` (`EffectLibrary.h:53`)
   is the real path used by MainComponent.
3. **UniformBridge / `applyDemoMappings` is dead** — `Renderer.h:179` explicitly
   comments `// Kept for reference, no longer used`; `applyDemoMappings` has no caller.
   Superseded by `MappingEngine`.
4. **ShaderManager hot-reload is inert for all shipped shaders** — every effect,
   transition, and source is compiled from embedded strings via `compileProgram`,
   so `reloadAll()` skips them all (`ShaderManager.cpp:115-116`). CLAUDE.md:239
   "hot-reload from shaders/" is not achievable for the real shader set.
5. **5 loose shader files in `shaders/` are orphaned/legacy** — `ripple.frag`,
   `hue_shift.frag`, `rgb_split.frag`, `vignette.frag`, `passthrough.vert`
   duplicate embedded versions and are not wired through `compileProgramFromFiles`
   in the shipping path. Dead artifacts.
6. **Duplicate shader-compile tables** — `Renderer.cpp` (275 `compile()` calls) and
   `src/ui/OutputWindow.cpp` (separate `compile()` table, e.g. `:164-210`) maintain
   parallel shaderkey→embedded lists. Divergence risk if one lags (OutputWindow is
   UI-lane but the duplication is render-relevant).

---

## 7. DOC-VERIFY — discrepancies (doc:line → claim → source truth)

| # | Doc:line | Claim | Source truth |
|---|----------|-------|--------------|
| D1 | CLAUDE.md:7 | "applies **110** GLSL shader effects" | **135** (`EffectLibrary.cpp`). STALE. |
| D2 | CLAUDE.md:242 | "All **96 effect** + 15 transition GLSL shaders" | **135 effect** + 15 transition. Effect count STALE; transitions ✓. |
| D3 | CLAUDE.md:397 | "Effect Categories (**115** total)" | **135**. WRONG header. |
| D4 | CLAUDE.md:547 | "**96** GLSL effects + 15 transitions" | **135** + 15. STALE. |
| D5 | CLAUDE.md:404 (Glitch row) | Glitch count **16** | **15** — `Glitch Displace` is category `warp` (`EffectLibrary.cpp:263`) and is double-listed in both the warp and glitch example rows. |
| D6 | CLAUDE.md:410 (Blur row) | Blur/Post count **9** | **10** (row itself lists 10 examples). |
| D7 | CLAUDE.md:406 (Audio row) | Audio count **3** | **4** (Harmonic Displacement, Timbral Mosaic, Structural Morph, Beat Ripple — row lists 4). CLAUDE.md category-count column sums to 134, not 135. |
| D8 | CLAUDE.md:11 | "**135 effects across 11 categories**… 15 clip-to-clip transitions… 6 presets" | ✓ CORRECT (11 cats, 15 transitions, 6 presets confirmed). |
| D9 | CLAUDE.md:395 | "135 effects across 11 categories + 15 transition shaders" | ✓ CORRECT. |
| D10 | CLAUDE.md:~455 | display-name→shaderkey rule via `getEffectDef(displayName)->shaderName` | ✓ CORRECT (`CompositorEngine.cpp:245,258,312`). |
| D11 | .harmony/FEATURES.md:193,204,226,241,303 | 135 effects, 333 params, 15 transitions, per-category param table | ✓ ACCURATE — per-category param table matches source exactly. |
| D12 | docs/FEATURE_INVENTORY.md:128,145,232 | 135 effects, 15 transitions | ✓ ACCURATE (most reliable lane doc; self-flags other docs' stale "76"). |

**Net**: CLAUDE.md is internally inconsistent — its own §"Effects Library" line
(11 / 395) is correct at 135, but the Identity line (7), tree comment (242),
category-table header (397), and status line (547) are stale (110/96/115), and 3
category sub-counts (glitch/blur/audio) are off. `.harmony/FEATURES.md` and
`docs/FEATURE_INVENTORY.md` are accurate at 135/333/15.

---

## 8. WORKS vs DOESN'T

**WORKS (solid):**
- 135 effects — all registered AND compiled (0 orphaned), 333 params, unique
  names/keys; display→shaderkey resolution correct.
- 15 clip-to-clip transitions — all embedded, compiled, and enum-mapped; dedicated
  `transitionFBO_`.
- Per-layer feedback (6 presets) + per-clip effect ping-pong (effectFBO_A/B) +
  temporal ring/temporal buffers for time effects.

**DOESN'T (broken/dead):**
- ISF import — registers a phantom effect; converted GLSL never compiled → never
  renders.
- Shader hot-reload — inert for the entire shipped shader set (embedded-only).
- UniformBridge — dead; `registerISFEffect` — dead; 5 loose `shaders/` files — dead;
  duplicate compile table in OutputWindow — divergence risk.
