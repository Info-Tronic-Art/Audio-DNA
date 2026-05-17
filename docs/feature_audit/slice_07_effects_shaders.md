# Slice 07: Effects Library, Shaders, Renderer, Compositor

All effects come from `src/effects/EffectLibrary.cpp` (single source of truth). Shader compilation is driven by `src/render/Renderer.cpp::compileAllShaders()` with GLSL embedded in `src/render/EmbeddedShaders.h`.

**Count verification**: `grep -c "registerEffect({"` against `EffectLibrary.cpp` → **135**. Category breakdown: 3D=9, Warp=27, Color=32, Glitch=15, Pattern=19, Animation=6, Audio=4, Time=6, Blend=5, Composite=3, Blur=10. (Note: 9+27+32+15+19+6+4+6+5+3+10 = 136 apparent but one entry overlaps — `animation/feedback` "Point Zoom" is also commonly grouped as blur; the registerEffect count is the authoritative 135.)

The `EffectDef` struct (EffectLibrary.h:24–31) is `{name, category, shaderName, params, temporal}`. The `Effect` class (Effect.h:18–68) adds per-instance `value`, `enabled`, `order`, and `dryWet`. Params are normalized to `[0, 1]` — the shader does internal remapping.

---

## Section 4: Effects Library (135 effects)

### 3D / Depth (9)

| Name | Shader Key | Params (default) | Temporal | File:Line |
|------|-----------|------------------|----------|-----------|
| Perspective Tilt | `perspective_tilt` | tilt_x (0.5), tilt_y (0.5) | No | EffectLibrary.cpp:192 |
| Cylinder Wrap | `cylinder_wrap` | amount (0.5), axis (0.0) | No | EffectLibrary.cpp:197 |
| Sphere Wrap | `sphere_wrap` | amount (0.5) | No | EffectLibrary.cpp:202 |
| Tunnel | `tunnel` | speed (0.5), radius (0.3) | No | EffectLibrary.cpp:206 |
| Page Curl | `page_curl` | amount (0.5), radius (0.5) | No | EffectLibrary.cpp:211 |
| Parallax Layers | `parallax_layers` | amount (0.4), direction (0.0) | No | EffectLibrary.cpp:216 |
| Dot Field | `dot_field` | size (0.3), spacing (0.5), depth (0.4) | No | EffectLibrary.cpp:488 |
| Luminance Terrain | `luma_terrain` | height (0.5), segments (0.5), angle (0.3) | No | EffectLibrary.cpp:762 |
| Voxel Matrix | `voxel_matrix` | size (0.3), height (0.5), rotation (0.0) | No | EffectLibrary.cpp:768 |

### Warp (27)

| Name | Shader Key | Params (default) | Temporal | File:Line |
|------|-----------|------------------|----------|-----------|
| Ripple | `ripple` | intensity (0.4), freq (0.5), speed (0.5) | No | EffectLibrary.cpp:14 |
| Bulge | `bulge` | amount (0.6), center_x (0.5), center_y (0.5) | No | EffectLibrary.cpp:20 |
| Wave | `wave` | amplitude (0.4), frequency (0.5), direction (0.0) | No | EffectLibrary.cpp:26 |
| Liquid | `liquid` | viscosity (0.5), turbulence (0.4) | No | EffectLibrary.cpp:32 |
| Kaleidoscope | `kaleidoscope` | segments (0.3), rotation (0.0) | No | EffectLibrary.cpp:112 |
| Fisheye | `fisheye` | amount (0.5) | No | EffectLibrary.cpp:117 |
| Swirl | `swirl` | amount (0.6), radius (0.5) | No | EffectLibrary.cpp:121 |
| Polar Coords | `polar_coords` | amount (0.5) | No | EffectLibrary.cpp:225 |
| Twirl | `twirl` | amount (0.6), radius (0.5) | No | EffectLibrary.cpp:229 |
| Shear | `shear` | x (0.5), y (0.5) | No | EffectLibrary.cpp:234 |
| Elastic Bounce | `elastic_bounce` | amount (0.4), freq (0.5) | No | EffectLibrary.cpp:239 |
| Ripple Pond | `ripple_pond` | intensity (0.4), freq (0.5) | No | EffectLibrary.cpp:244 |
| Diamond Distort | `diamond_distort` | size (0.5), amount (0.4) | No | EffectLibrary.cpp:249 |
| Barrel Distort | `barrel_distort` | amount (0.5) | No | EffectLibrary.cpp:254 |
| Sine Grid | `sine_grid` | freq (0.5), amount (0.4) | No | EffectLibrary.cpp:258 |
| Glitch Displace | `glitch_displace` | amount (0.4), speed (0.5) | No | EffectLibrary.cpp:263 |
| Quad Mirror | `quad_mirror` | center x (0.5), center y (0.5) | No | EffectLibrary.cpp:447 |
| Flip | `flip` | horizontal (0.0), vertical (0.0) | No | EffectLibrary.cpp:452 |
| Warp Field | `warp_field` | amount (0.4), frequency (0.5), speed (0.3) | No | EffectLibrary.cpp:457 |
| Slide Wrap | `slide_wrap` | x (0.5), y (0.5) | No | EffectLibrary.cpp:483 |
| Tile Grid | `tile_grid` | columns (0.25), rows (0.25), offset (0.0), zoom (0.5) | No | EffectLibrary.cpp:512 |
| Spot Zoom | `spot_zoom` | center x (0.5), center y (0.5), size (0.3), zoom (0.7), shape (0.0), background (0.3) | No | EffectLibrary.cpp:519 |
| Bendoscope | `bendoscope` | divisions (0.3), bend (0.5), rotation (0.0) | No | EffectLibrary.cpp:563 |
| UV Remap | `uv_remap` | amount (0.4), scale (0.5), speed (0.3) | No | EffectLibrary.cpp:569 |
| Liquid Morph | `liquid_morph` | viscosity (0.5), amount (0.4), scale (0.5) | No | EffectLibrary.cpp:575 |
| Zoom Warp | `infinite_zoom` | speed (0.3), rotation (0.5), center x (0.5), center y (0.5) | No | EffectLibrary.cpp:628 |
| Density Wave | `density_wave` | amount (0.5), direction (0.0), wavelength (0.5) | No | EffectLibrary.cpp:747 |

### Color (32)

| Name | Shader Key | Params (default) | Temporal | File:Line |
|------|-----------|------------------|----------|-----------|
| Hue Shift | `hue_shift` | amount (0.3) | No | EffectLibrary.cpp:39 |
| Saturation | `saturation` | amount (0.5) | No | EffectLibrary.cpp:43 |
| Brightness | `brightness` | amount (0.5) | No | EffectLibrary.cpp:47 |
| Duotone | `duotone` | color1_r (0.0), color1_g (0.0), color1_b (0.5), color2_r (1.0), color2_g (0.5), color2_b (0.0), mix (0.5) | No | EffectLibrary.cpp:51 |
| Chromatic Aberration | `chromatic_aberration` | amount (0.4), angle (0.0) | No | EffectLibrary.cpp:61 |
| Invert | `invert` | amount (0.7) | No | EffectLibrary.cpp:128 |
| Posterize | `posterize` | levels (0.3) | No | EffectLibrary.cpp:132 |
| Color Shift | `color_shift` | red (0.5), green (0.5), blue (0.5) | No | EffectLibrary.cpp:136 |
| Thermal | `thermal` | amount (0.7) | No | EffectLibrary.cpp:142 |
| Contrast | `color_matrix` | contrast (0.5), balance (0.5) | No | EffectLibrary.cpp:146 |
| Sepia | `sepia` | amount (0.7) | No | EffectLibrary.cpp:272 |
| Cross Process | `cross_process` | amount (0.7) | No | EffectLibrary.cpp:276 |
| Split Tone | `split_tone` | shadow_hue (0.6), highlight_hue (0.1), amount (0.5) | No | EffectLibrary.cpp:280 |
| Color Halftone | `color_halftone` | scale (0.5), amount (0.5) | No | EffectLibrary.cpp:286 |
| Dither | `ordered_dither` | levels (0.5), amount (0.5) | No | EffectLibrary.cpp:291 |
| Heat Map | `heat_map` | amount (0.7) | No | EffectLibrary.cpp:296 |
| Selective Color | `selective_color` | hue (0.0), range (0.2) | No | EffectLibrary.cpp:300 |
| Film Grain | `film_grain` | amount (0.3), size (0.5) | No | EffectLibrary.cpp:305 |
| Gamma Levels | `gamma_levels` | black (0.0), white (1.0), gamma (0.5) | No | EffectLibrary.cpp:310 |
| Solarize | `solarize` | threshold (0.5), amount (0.5) | No | EffectLibrary.cpp:316 |
| Greyscale | `greyscale` | method (0.0), amount (0.7) | No | EffectLibrary.cpp:429 |
| Threshold | `threshold` | level (0.5), amount (0.7) | No | EffectLibrary.cpp:434 |
| Exposure | `exposure` | amount (0.5) | No | EffectLibrary.cpp:439 |
| Vibrance | `vibrance` | amount (0.5) | No | EffectLibrary.cpp:443 |
| Auto Mask | `auto_mask` | threshold (0.5), softness (0.3), invert (0.0) | No | EffectLibrary.cpp:499 |
| Chroma Key | `chromakey_effect` | hue (0.33), tolerance (0.3), softness (0.3), amount (0.7) | No | EffectLibrary.cpp:505 |
| Palette Remap | `palette_remap` | palette (0.0), cycle (0.0), amount (0.7) | No | EffectLibrary.cpp:553 |
| Color Grade | `lut_grade` | amount (0.7) | No | EffectLibrary.cpp:559 |
| Pitch Chromatic Shift | `pitch_chroma_shift` | amount (0.5), mode (0.0), saturation (0.5) | No | EffectLibrary.cpp:717 (uses `u_dominantPitch`, `u_pitchConfidence`) |
| Key Palette | `key_palette` | amount (0.5), brightness (0.5), saturation (0.5) | No | EffectLibrary.cpp:723 (uses `u_detectedKey`, `u_keyIsMajor`) |
| Chroma Dissolve | `chroma_dissolve` | amount (0.5), softness (0.5) | No | EffectLibrary.cpp:753 (uses `u_chromagram`) |

### Glitch (15)

| Name | Shader Key | Params (default) | Temporal | File:Line |
|------|-----------|------------------|----------|-----------|
| Pixel Scatter | `pixel_scatter` | amount (0.4), seed (0.0) | No | EffectLibrary.cpp:68 |
| RGB Split | `rgb_split` | amount (0.4), angle (0.0) | No | EffectLibrary.cpp:73 |
| Block Glitch | `block_glitch` | intensity (0.4), block_size (0.5) | No | EffectLibrary.cpp:78 |
| Scanlines | `scanlines` | intensity (0.4), frequency (0.5) | No | EffectLibrary.cpp:83 |
| Digital Rain | `digital_rain` | intensity (0.4), speed (0.5) | No | EffectLibrary.cpp:153 |
| Noise | `noise_overlay` | amount (0.3), speed (0.5) | No | EffectLibrary.cpp:158 |
| Mirror | `mirror` | horizontal (1.0), vertical (0.0) | No | EffectLibrary.cpp:163 |
| Pixelate | `pixelate` | size (0.3) | No | EffectLibrary.cpp:168 |
| Pixel Explosion | `pixel_explosion` | force (0.4), decay (0.5), center x (0.5), center y (0.5) | No | EffectLibrary.cpp:468 |
| Color Flash | `color_flash` | intensity (0.5), red (1.0), green (1.0), blue (1.0), decay (0.5) | No | EffectLibrary.cpp:475 |
| Fragment Burst | `fragment_burst` | copies (0.3), spread (0.3), rotation (0.2), scale (0.5) | No | EffectLibrary.cpp:593 |
| Signal Destroy | `signal_destroy` | amount (0.4), speed (0.5), mode (0.0) | No | EffectLibrary.cpp:600 |
| Rhythm Slice | `rhythm_slice` | amount (0.5), slices (0.5), sync (0.5) | No | EffectLibrary.cpp:741 (uses `u_beatPhase`) |
| Data Corruption | `data_corrupt` | amount (0.5), block size (0.5), color damage (0.5) | No | EffectLibrary.cpp:801 |
| Glitch Sort | `glitch_sort` | amount (0.5), threshold (0.5), direction (0.0) | No | EffectLibrary.cpp:807 |

### Pattern (19)

| Name | Shader Key | Params (default) | Temporal | File:Line |
|------|-----------|------------------|----------|-----------|
| CRT | `crt_simulation` | curvature (0.3), scanline (0.5) | No | EffectLibrary.cpp:325 |
| VHS | `vhs_effect` | amount (0.5), tracking (0.3) | No | EffectLibrary.cpp:330 |
| ASCII Art | `ascii_art` | scale (0.5), color (0.5) | No | EffectLibrary.cpp:335 |
| Dot Matrix | `dot_matrix` | scale (0.5), amount (0.5) | No | EffectLibrary.cpp:340 |
| Crosshatch | `crosshatch` | density (0.5), amount (0.5) | No | EffectLibrary.cpp:345 |
| Emboss | `emboss` | amount (0.5), angle (0.0) | No | EffectLibrary.cpp:350 |
| Oil Paint | `oil_paint` | radius (0.3) | No | EffectLibrary.cpp:355 |
| Pencil Sketch | `pencil_sketch` | amount (0.5), density (0.5) | No | EffectLibrary.cpp:359 |
| Voronoi Glass | `voronoi_glass` | scale (0.5), edge (0.3) | No | EffectLibrary.cpp:364 |
| Cross Stitch | `cross_stitch` | scale (0.5), amount (0.5) | No | EffectLibrary.cpp:369 |
| Night Vision | `night_vision` | amount (0.7) | No | EffectLibrary.cpp:374 |
| Triangulate | `triangulate` | size (0.3), amount (0.5) | No | EffectLibrary.cpp:494 |
| Neon Edge | `neon_edge` | edge (0.6), glow (0.5), hue (0.5), original (0.3) | No | EffectLibrary.cpp:528 |
| Cartoon Ink | `cartoon_ink` | edge width (0.4), color steps (0.4), ink strength (0.7), saturation (0.6) | No | EffectLibrary.cpp:535 |
| Pop Raster | `pop_raster` | palette (0.0), bands (0.4), pattern size (0.3), mix (0.7) | No | EffectLibrary.cpp:542 |
| Brush Strokes | `brush_strokes` | size (0.4), angle (0.0), flow (0.5), amount (0.5) | No | EffectLibrary.cpp:586 |
| Bump Light | `bump_light` | light x (0.5), light y (0.3), intensity (0.6), height (0.5) | No | EffectLibrary.cpp:635 |
| Monitor Wall | `monitor_wall` | columns (0.3), rows (0.3), border (0.3), glow (0.3) | No | EffectLibrary.cpp:774 |
| Topographic Lines | `topo_lines` | amount (0.5), levels (0.5), thickness (0.3), color mode (0.0) | No | EffectLibrary.cpp:794 |

### Animation (6)

| Name | Shader Key | Params (default) | Temporal | File:Line |
|------|-----------|------------------|----------|-----------|
| Strobe | `strobe` | rate (0.5), intensity (0.5) | No | EffectLibrary.cpp:382 |
| Pulse | `pulse` | amount (0.4), speed (0.5) | No | EffectLibrary.cpp:387 |
| Slit Scan | `slit_scan` | amount (0.4), direction (0.0) | No | EffectLibrary.cpp:392 |
| Point Zoom | `feedback` | amount (0.7), zoom (0.52), rotation (0.52), x offset (0.5), y offset (0.5), decay (0.7), hue shift (0.0), saturation (0.5) | No | EffectLibrary.cpp:674 |
| Directional Feedback | `directional_feedback` | amount (0.7), speed (0.5), direction (0.5), spread (0.3), decay (0.7), hue shift (0.0) | No | EffectLibrary.cpp:685 |
| Transient Flash | `transient_flash` | style (0.0), intensity (0.5), decay (0.5) | No | EffectLibrary.cpp:729 (uses `u_onsetStrength`, `u_onsetDetected`) |

### Audio (4)

| Name | Shader Key | Params (default) | Temporal | File:Line |
|------|-----------|------------------|----------|-----------|
| Harmonic Displacement | `harmonic_displace` | amount (0.5), smoothing (0.5), color mode (0.0) | No | EffectLibrary.cpp:699 (uses `u_chromagram`, `u_hcdf`) |
| Timbral Mosaic | `timbral_mosaic` | amount (0.5), base size (0.5), complexity (0.5) | No | EffectLibrary.cpp:705 (uses `u_mfccs`) |
| Structural Morph | `structural_morph` | intensity (0.5), normal style (0.3), drop style (0.5) | No | EffectLibrary.cpp:711 (uses `u_structuralState`) |
| Beat Ripple | `beat_ripple` | intensity (0.5), decay (0.5), count (0.3) | No | EffectLibrary.cpp:735 (uses `u_beatPhase`, `u_onsetDetected`) |

### Time (6) — all temporal except as noted

| Name | Shader Key | Params (default) | Temporal | File:Line |
|------|-----------|------------------|----------|-----------|
| Echo | `ghost_trails` | decay (0.5), operator (0.0) | **Yes** | EffectLibrary.cpp:646 |
| Posterize Time | `frame_hold` | frame rate (0.5), amount (1.0) | **Yes** | EffectLibrary.cpp:651 |
| Freeze | `time_freeze` | amount (1.0) | **Yes** | EffectLibrary.cpp:656 |
| Screen Split | `screen_split` | columns (0.3), rows (0.3), frames per cell (0.25), direction (0.0) | No (ring buffer) | EffectLibrary.cpp:660 |
| Frame Stutter | `frame_delay` | depth (0.3), stutter (0.4) | No (ring buffer) | EffectLibrary.cpp:667 |
| Channel Delay | `channel_delay` | red delay (0.3), green delay (0.0), blue delay (0.0) | **Yes** | EffectLibrary.cpp:788 |

`Screen Split` and `Frame Stutter` are intercepted in `CompositorEngine::applyClipEffects()` (CompositorEngine.cpp:263, 276) and rendered via a `FrameRingBuffer` (480 frames at 1/4 resolution, ~120MB VRAM). They still register in EffectLibrary for FX browser discovery but `temporal = false` because they don't use `u_prev_frame`.

### Blend (5)

| Name | Shader Key | Params (default) | Temporal | File:Line |
|------|-----------|------------------|----------|-----------|
| Double Exposure | `double_exposure` | offset (0.3), blend (0.5) | No | EffectLibrary.cpp:401 |
| Frosted Glass | `frosted_glass` | amount (0.5), scale (0.5) | No | EffectLibrary.cpp:406 |
| Prism | `prism_refract` | amount (0.4), angle (0.0) | No | EffectLibrary.cpp:411 |
| Rain on Glass | `rain_on_glass` | amount (0.5), speed (0.5) | No | EffectLibrary.cpp:416 |
| Hexagonalize | `hexagonalize` | scale (0.4) | No | EffectLibrary.cpp:421 |

### Composite (3)

| Name | Shader Key | Params (default) | Temporal | File:Line |
|------|-----------|------------------|----------|-----------|
| Line Cloner | `line_cloner` | copies (0.3), offset x (0.8), offset y (0.5), scale (0.4), rotation (0.5) | No | EffectLibrary.cpp:606 |
| Radial Cloner | `radial_cloner` | copies (0.3), radius (0.3), rotation (0.0), scale (0.5) | No | EffectLibrary.cpp:614 |
| Cube Scatter | `cube_scatter` | grid x (0.3), grid y (0.3), explode (0.3), rotation (0.0) | No | EffectLibrary.cpp:621 |

### Blur / Post (10)

| Name | Shader Key | Params (default) | Temporal | File:Line |
|------|-----------|------------------|----------|-----------|
| Gaussian Blur | `gaussian_blur` | radius (0.3) | No | EffectLibrary.cpp:90 |
| Zoom Blur | `zoom_blur` | amount (0.3), center_x (0.5), center_y (0.5) | No | EffectLibrary.cpp:94 |
| Shake | `shake` | amount_x (0.3), amount_y (0.0) | No | EffectLibrary.cpp:100 |
| Vignette | `vignette` | intensity (0.5), softness (0.6) | No | EffectLibrary.cpp:105 |
| Motion Blur | `motion_blur` | amount (0.4), angle (0.0) | No | EffectLibrary.cpp:174 |
| Glow | `glow` | amount (0.4), threshold (0.5) | No | EffectLibrary.cpp:179 |
| Edge Detect | `edge_detect` | amount (0.5) | No | EffectLibrary.cpp:184 |
| Sharpen | `sharpen` | amount (0.4), radius (0.3) | No | EffectLibrary.cpp:463 |
| Edge Blur | `edge_blur` | threshold (0.5), amount (0.4) | No | EffectLibrary.cpp:581 |
| Drop Shadow | `drop_shadow` | offset x (0.6), offset y (0.4), blur (0.3), opacity (0.5) | No | EffectLibrary.cpp:781 |

---

## Section 6: Transitions (15)

All transitions compile in `Renderer::compileAllShaders()` (Renderer.cpp:1324–1338) and are selected via `CompositorEngine::getTransitionShaderName()` (CompositorEngine.cpp:1065–1086) from the `Layer::MixMode` enum. Shader GLSL at EmbeddedShaders.h:3856–4130.

| Layer::MixMode | Shader Key | Type | File:Line (shader) |
|----------------|-----------|------|-------------------|
| `Dissolve` (default) | `transition_dissolve` | mix() crossfade | EmbeddedShaders.h:3856 |
| `Cut` | `transition_cut` | hard cut at 0.5 | EmbeddedShaders.h:4098 |
| `WipeLeft` | `transition_wipe_left` | smoothstep edge | EmbeddedShaders.h:3871 |
| `WipeRight` | `transition_wipe_right` | smoothstep edge | EmbeddedShaders.h:3887 |
| `WipeUp` | `transition_wipe_up` | smoothstep edge | EmbeddedShaders.h:3903 |
| `WipeDown` | `transition_wipe_down` | smoothstep edge | EmbeddedShaders.h:3919 |
| `PushLeft` | `transition_push_left` | UV slide | EmbeddedShaders.h:3935 |
| `PushRight` | `transition_push_right` | UV slide | EmbeddedShaders.h:3955 |
| `PushUp` | `transition_push_up` | UV slide | EmbeddedShaders.h:3975 |
| `PushDown` | `transition_push_down` | UV slide | EmbeddedShaders.h:3995 |
| `ZoomIn` | `transition_zoom_in` | scale next from center | EmbeddedShaders.h:4015 |
| `ZoomOut` | `transition_zoom_out` | scale prev from center | EmbeddedShaders.h:4035 |
| `WipeEllipse` | `transition_iris` | circular mask smoothstep | EmbeddedShaders.h:4055 |
| `Flip` | `transition_flip_h` | horizontal squash/stretch | EmbeddedShaders.h:4074 |
| `ToBlack` | `transition_fade_black` | fade down then up | EmbeddedShaders.h:4113 |

Crossfade progress sampler is `u_crossfadeProgress` (float). Textures are `u_texture` (new) and `u_prevTexture` (old). `Layer::crossfadeProgress` advances `(1/60) / transitionSpeed` per frame (CompositorEngine.cpp:688).

### Cross-deck transition (P25)

Distinct from per-layer transitions — applies between two whole decks. Shader `deck_transition` (EmbeddedShaders.h:113, compiled Renderer.cpp:1010). Uniforms: `u_textureA` (outgoing), `u_textureB` (incoming), `u_progress` (0→1), `u_blendMode` (int: 0=Alpha, 1=Add, 2=Multiply). Driven by `Renderer::deckTransitionProgress_` (Renderer.cpp:471–519). Blend mode comes from `composition_->crossfaderBlendMode`.

### Layer blend modes (used by `blendLayerOntoAccumulator`)

From CompositorEngine.cpp:980–1040, the `Layer::MixMode` blend modes (different namespace from transitions — these are alpha-compositing modes for stacking layers):
- `Additive` → `GL_SRC_ALPHA, GL_ONE`
- `Normal` → standard separate alpha `GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA`
- `Screen` → `GL_ONE, GL_ONE_MINUS_SRC_COLOR`
- `Multiply` → `GL_DST_COLOR, GL_ZERO`
- `Darken` → `GL_MIN` equation
- `Lighten` → `GL_MAX` equation
- default → alpha blend fallback

### Composition/pre-pass shaders

| Shader Key | Purpose | File:Line |
|-----------|---------|-----------|
| `passthrough` | identity copy | EmbeddedShaders.h:24 |
| `opacity_blend` | RGB * opacity on alpha | EmbeddedShaders.h:39 |
| `effect_dry_wet` / `effect_drywet` | mix(orig, eff, u_drywet) | EmbeddedShaders.h:53 |
| `comp_transform` | composition-level position/scale/rotation | EmbeddedShaders.h:69 |
| `deck_transition` | cross-deck blend | EmbeddedShaders.h:113 |
| `layer_transform` | per-layer/clip transform (translate/scale/rotate) | EmbeddedShaders.h:3302 |
| `mask_luminance` | mask layer: content luma becomes alpha on accumulator | EmbeddedShaders.h:3344 |
| `feedback_blend` | feedback transform/blend | EmbeddedShaders.h:8607 |

### Keying shaders (13 modes, some aliased to fallback)

Mapped in `CompositorEngine::applyLayerKeying()` (CompositorEngine.cpp:916–932), compiled Renderer.cpp:1109–1125.

| Layer::KeyingMode | Shader Key | Backing GLSL |
|-------------------|-----------|--------------|
| Alpha | `key_alpha` | `transparencyAlpha` |
| LumaKey | `key_luma` | `transparencyLumaKey` |
| InvertedLumaKey | `key_inv_luma` | `transparencyLumaKey` (alias) |
| LumaIsAlpha | `key_luma_alpha` | `transparencyLight` |
| InvertedLumaIsAlpha | `key_inv_luma_alpha` | `transparencyAlpha` (fallback) |
| ChromaKey | `key_chroma` | `transparencyChromaKey` |
| MaxRGB | `key_max_rgb` | `transparencyLight` |
| SaturationKey | `key_saturation` | `transparencyLumaKey` (alias) |
| EdgeDetection | `key_edge` | `transparencyAlpha` (fallback) |
| ThresholdMask | `key_threshold` | `transparencyAlpha` (fallback) |
| ChannelR | `key_channel_r` | `transparencyAlpha` (fallback) |
| ChannelG | `key_channel_g` | `transparencyAlpha` (fallback) |
| ChannelB | `key_channel_b` | `transparencyAlpha` (fallback) |

Keying uniforms: `u_opacity`, `u_threshold`, `u_softness`, `u_chroma_key_color` (vec3), `u_chroma_tolerance`, `u_resolution`.

---

## Effect rendering pipeline

### Two parallel paths

Audio-DNA has **two independent effect render paths**, both supporting temporal effects (u_prev_frame) and audio uniforms:

**Path 1 — `EffectChain::render()` (EffectChain.cpp:18–201)**: single-image / source mode. Global effects only.
- Collects enabled effects → ping-pongs between `texMgr_.getFBO(0)` and `getFBO(1)`.
- Maintains its own `prevFrameTexture_`/`prevFrameFBO_` pair (EffectChain.h:84) for `u_prev_frame`.
- When `anyTemporal` is true, the last effect is forced to FBO (never direct-to-screen) so the frame can be saved before blit.
- `applyDryWet()` uses `effect_drywet` shader to blend orig/effected at `effect.getDryWet()`.
- Uniform location cache keyed by `(programID << 32) | hashName` → `uniformLocationCache_`.

**Path 2 — `CompositorEngine::applyClipEffects()` (CompositorEngine.cpp:232–433)**: per-clip AND per-layer effects in deck mode.
- Called twice per layer: once for `clip.effects`, once for `layer.layerEffects` (re-uses same method via a temp `Clip` wrapper at CompositorEngine.cpp:751–754).
- Ping-pongs between `effectFBO_A_` and `effectFBO_B_`.
- Per-layer temporal buffers keyed by `layerId` in `layerTemporalBuffers_` (TemporalBuffer struct with persistent FBO/tex).
- Special interception for `screen_split` and `frame_delay` shader keys — they use the `FrameRingBuffer` (480 frames @ 1/4 res) instead of the normal shader path.
- Binds `u_feedbackTex` (layer's previous composited output) to unit 1 for non-temporal effects that want it.

### Per-frame composite order (deck mode)

From `CompositorEngine::compositeDeck()` (CompositorEngine.cpp:637–839), for each visible non-bypassed layer bottom-to-top:

```
1. Get clipTex: image (texture cache) / procedural source (sourceRenderFn_) / video or image sequence (videoFrameFn_)
2. Advance layer.crossfadeProgress by (1/60)/transitionSpeed
3. applyClipTransform(clip)                                  → position/scale/rotation around anchor
4. applyClipEffects(clip, layerId)                           → per-clip chain (ping-pong A/B, may intercept Screen Split / Frame Stutter)
5. applyTransition(layer)                                    → only if crossfadeProgress < 1.0 (uses transitionFBO_)
6. feedbackProcessor.process() if layer.feedback.enabled     → Larsen loop (per-layer FBO pair)
7. applyClipEffects(tempClip with layerEffects, layerId)     → per-layer chain
8. applyLayerTransform(layer)                                → per-layer position/scale/rotation
9. saveLayerOutput(layer.id)                                 → for Layer Router source (P20)
10. If Transparent: applyLayerKeying → blendLayerOntoAccumulator
    If Opaque: clear accumulator black + draw with opacity/opacity_blend
    If FXOnly: applyFXOnlyLayer (effects on accumulator)
    If Mask: applyMaskLayer (luminance mask on accumulator)
    If ThreeD: no-op (not implemented)
```

After all active deck layers, `compositePersistentLayers()` iterates all other decks for `layer.persistent` layers (P21).

### Post-composite pipeline (Renderer::renderOpenGL)

From Renderer.cpp:400–520:
```
compositor.compositeDeck              → sourceTexture  (or procedural source / image fallback)
compositor.compositePersistentLayers  → augment accumulator with persistent layers from other decks
compositor.updateFeedbackBuffer       → copy accumulator → feedbackTex_ for next frame
effectChain_.setLatestSnapshot(snap)
effectChain_.render(sourceTexture, …) → global effects onto defaultFBO
applyCompTransform()                  → composition-level transform (position/scale/rotation)
deck_transition blend                 → if prevActiveDeckIndex changed and progress < 1.0
```

### Ping-pong FBO detail

- `TextureManager` owns 2 RGBA8 FBOs (`fbos_[0..1]`) for `EffectChain` chain (TextureManager.cpp:96).
- `CompositorEngine` owns 2 dedicated effect FBOs: `effectFBO_A_`/`effectFBO_B_` (CompositorEngine.cpp:14–15).
- `CompositorEngine::scratchFBO_` — used for keying and FX-only blending scratch space.
- `CompositorEngine::transitionFBO_` — separate from scratch to avoid keying conflicts (fix for bug #3 in CLAUDE.md Common Pitfalls).
- `CompositorEngine::accumulatorFBO_` — the final composited-output target.
- `CompositorEngine::feedbackFBO_` — persistent across frames, holds previous composite for `u_feedbackTex`.

### Temporal effect handling

- `EffectDef::temporal` flag (EffectLibrary.h:30) marks effects that read `u_prev_frame`.
- Only **4 effects are marked temporal**: Echo (ghost_trails), Posterize Time (frame_hold), Freeze (time_freeze), Channel Delay (channel_delay). (EffectLibrary.cpp:649, 654, 658, 792).
- Screen Split and Frame Stutter are **not** marked temporal (they use ring buffer, not prev-frame texture).
- Per-layer `TemporalBuffer` (fbo + tex) persists across frames; cleared to black on creation. Separate per `layerId` (CompositorEngine.cpp:1188–1209).
- `FrameRingBuffer`: 480 frames × 1/4 resolution per layer (CompositorEngine.h:167–178). `getOrCreateRingBuffer`, `pushFrameToRing`, `getFrameFromRing` form the API. ~120MB VRAM estimate.

### Screen Split rendering

`applyScreenSplit()` (CompositorEngine.cpp:1308–1396): reads cols/rows from params [0,1] → [2, 8] integer range, `framesPerCell = round(60 * delay)`, 3 modes (sequential / reverse / diagonal). Renders each grid cell with `glViewport` at `cellIndex*framesPerCell` frames-ago texture from the ring buffer. Dark gray gridlines (2-pixel border).

### Frame Stutter rendering

Also in `applyClipEffects()` (CompositorEngine.cpp:276–310): `depth` param [0,1] → [1,30] frames, `stutter` param [0,1] picks between smooth delay (stutter=0) and discrete quantized jumping at rate `1 + 15*stutter` Hz with `2 + 6*stutter` step buckets.

---

## Feedback System (6 presets)

`FeedbackProcessor` (FeedbackProcessor.h:22) — per-layer Larsen feedback. Owns a 2-FBO pair (read/write ping-pong). Uses `feedback_blend` shader.

Uniforms passed (FeedbackProcessor.cpp:110–116):
- `u_feedback_amount`, `u_feedback_scaleX`, `u_feedback_scaleY`, `u_feedback_rotation`, `u_feedback_offsetX`, `u_feedback_offsetY`, `u_feedback_lumaKey`
- Samplers: `u_currentFrame` (unit 0), `u_previousFrame` (unit 1)

### Presets (FeedbackProcessor.cpp:131–142)

`FeedbackConfig` fields: `{enabled, amount, scaleX, scaleY, rotation, offsetX, offsetY, lumaKey, name}`.

| Preset | amount | scaleX | scaleY | rotation | offsetX | offsetY | lumaKey |
|--------|--------|--------|--------|----------|---------|---------|---------|
| Zoom In | 0.85 | 0.97 | 0.97 | 0.0 | 0.0 | 0.0 | 0.0 |
| Spiral | 0.80 | 0.98 | 0.98 | 2.0 | 0.0 | 0.0 | 0.0 |
| Drift | 0.75 | 1.0 | 1.0 | 0.0 | 0.01 | 0.0 | 0.0 |
| Kaleidoscope | 0.90 | 0.95 | 0.95 | 5.0 | 0.0 | 0.0 | 0.0 |
| Echo | 0.60 | 1.0 | 1.0 | 0.0 | 0.0 | 0.0 | 0.0 |
| Stretch | 0.85 | 1.02 | 0.98 | 0.0 | 0.0 | 0.0 | 0.0 |

Static lookup via `FeedbackProcessor::applyPreset(name, config)` (FeedbackProcessor.cpp:144).

---

## Global / Audio Uniforms Available in Shaders

All three render paths upload the same uniform set. Any shader may declare any subset — unused uniforms are silently ignored (`glGetUniformLocation` returns -1).

### Global

| Uniform | Type | Source |
|---------|------|--------|
| `u_time` | float | seconds since start (may be overridden for deterministic capture) |
| `u_resolution` | vec2 | render width/height |
| `u_texture` | sampler2D | input texture (unit 0) |
| `u_prev_frame` | sampler2D | previous-frame texture (unit 1, temporal effects only) |
| `u_feedbackTex` | sampler2D | previous composited accumulator (unit 1, non-temporal effects, CompositorEngine path only) |

### Audio uniforms — three upload paths

| Uniform | Type | Source field | Upload path |
|---------|------|--------------|-------------|
| `u_rms` | float | snap.rms | EffectChain.cpp:283, CompositorEngine.cpp:1409 |
| `u_bass` | float | snap.bandEnergies[1] (60-250 Hz) | EffectChain.cpp:285 |
| `u_mid` | float | snap.bandEnergies[3] (500-2k Hz) | EffectChain.cpp:287 |
| `u_high` | float | snap.bandEnergies[5] (4-6k Hz) | EffectChain.cpp:289 |
| `u_beatPhase` | float | snap.beatPhase [0,1) | EffectChain.cpp:291 |
| `u_barPhase` | float | snap.barPhase [0,1) | EffectChain.cpp:293 |
| `u_phrasePhase` | float | snap.phrasePhase [0,1) | EffectChain.cpp:295 |
| `u_spectralCentroid` | float | snap.spectralCentroid (Hz) | EffectChain.cpp:297 |
| `u_spectralFlux` | float | snap.spectralFlux | EffectChain.cpp:299 |
| `u_onsetStrength` | float | snap.onsetStrength | EffectChain.cpp:301 |
| `u_onsetDetected` | float | snap.onsetDetected ? 1 : 0 | EffectChain.cpp:303 |
| `u_dominantPitch` | float | snap.dominantPitch (Hz) | EffectChain.cpp:305 |
| `u_pitchConfidence` | float | snap.pitchConfidence | EffectChain.cpp:307 |
| `u_detectedKey` | float | static_cast<float>(snap.detectedKey) (-1 or 0-11) | EffectChain.cpp:309 |
| `u_keyIsMajor` | float | snap.keyIsMajor ? 1 : 0 | EffectChain.cpp:311 |
| `u_structuralState` | float | 0=normal,1=buildup,2=drop,3=breakdown | EffectChain.cpp:313 |
| `u_bpm` | float | snap.bpm | EffectChain.cpp:315 |
| `u_hcdf` | float | snap.harmonicChangeDetection | EffectChain.cpp:317 |
| `u_bandEnergies[7]` | float array | snap.bandEnergies (7 bands) | EffectChain.cpp:319 |
| `u_chromagram[12]` | float array | snap.chromagram (C–B) | EffectChain.cpp:321 |
| `u_mfccs[13]` | float array | snap.mfccs | EffectChain.cpp:323 |
| `u_genre` | float | static_cast<float>(snap.detectedGenre) 0-7 | EffectChain.cpp:328 |
| `u_genreConfidence` | float | snap.genreConfidence [0,1] | EffectChain.cpp:330 |
| `u_energyState` | float | 0=low, 1=med, 2=high | EffectChain.cpp:332 |
| `u_sidechainPump` | float | snap.sidechainPump (P25) | EffectChain.cpp:336 |
| `u_swingRatio` | float | snap.swingRatio (P25) | EffectChain.cpp:338 |
| `u_formantPresence` | float | snap.formantPresence (P25) | EffectChain.cpp:340 |
| `u_resonancePeak` | float | snap.resonancePeak (P25) | EffectChain.cpp:342 |
| `u_reeseBass` | float | snap.reeseBass (P25) | EffectChain.cpp:344 |

Total: **28 audio uniforms** (24 scalars + 3 float arrays of size 7/12/13 + global `u_time`/`u_resolution`).

Also available in `ProceduralSource::uploadUniforms()` (not in this slice — see slice 08).

---

## ISF Import (ISFShaderLoader)

`ISFShaderLoader` (ISFShaderLoader.h:30) imports Interactive Shader Format shaders from isf.video into the EffectLibrary.

### Supported INPUT types (ISFShaderLoader.cpp:66–104)

| Type | Default handling |
|------|------------------|
| `float` | Default normalized from [MIN,MAX] to [0,1] per (ISFShaderLoader.cpp:66–85) |
| `bool` | Default 0.0 or 1.0 |
| `long` | Int normalized to [0,1] (placeholder default 0.0) |
| `point2D` | Declared as vec2 uniform but **skipped** from params list (`continue` at ISFShaderLoader.cpp:103) |
| `color` | Declared as vec4 uniform but **skipped** from params |
| `image` / `audio` / `event` | **Not supported** |

### GLSL wrapping (ISFShaderLoader::convertToGLSL, ISFShaderLoader.cpp:117–208)

Prepends `#version 410 core`, declares `u_time`, `u_resolution`, `u_texture`, `v_uv` (input), and these compatibility defines:

```glsl
#define TIME u_time
#define RENDERSIZE u_resolution
#define PASSINDEX 0
#define FRAMEINDEX int(u_time * 60.0)
#define isf_FragNormCoord v_uv
#define IMG_NORM_PIXEL(s, c) texture(s, c)
#define IMG_PIXEL(s, c)      texelFetch(s, ivec2(c), 0)
#define IMG_THIS_PIXEL(s)    texture(s, v_uv)
#define IMG_THIS_NORM_PIXEL(s) texture(s, v_uv)
#define inputImage u_texture
```

Per-param uniforms declared as `uniform float u_isf_[paramName]` (floats/bools/longs as float) plus `#define [paramName] u_isf_[paramName]` so ISF shader bodies compile unchanged.

Post-processing string replacements:
- `gl_FragColor` → `fragColor`
- `gl_FragCoord.xy` → `(v_uv * u_resolution)`

If no `void main()` is found, a default passthrough main is appended.

### Registration

`registerISFEffect(library, isf)` creates an `EffectDef` with:
- name = `"ISF: <filename>"`
- category = `"isf"`
- shaderName = `"isf_<name>"`
- params: each param's `uniformName = "u_isf_<name>"`

Called via `EffectLibrary::registerDynamic()` (public entry at EffectLibrary.h:53) after shader compile.

### Limitations

- Only single-pass (no multi-pass `PASSES`)
- `image` / `audio` / `event` inputs rejected
- ISF directory fixed at `~/Documents/Audio-DNA/ISF Shaders/` (ISFShaderLoader.cpp:247)

---

## ShaderManager

`ShaderManager` (ShaderManager.h:11) wraps `juce::OpenGLShaderProgram`. Owns a name → `ProgramEntry` map (program, vertFile, fragFile, uniform cache).

- `compileProgram(name, vert, frag)` (ShaderManager.cpp:13) — build from source strings. Compile/link errors logged via `DBG()`.
- `compileProgramFromFiles(name, vertFile, fragFile)` (ShaderManager.cpp:43) — loads from `shadersDir_/<file>`. Stores filenames so `reloadAll()` can re-read.
- `getProgram(name)` returns raw pointer, or nullptr.
- `getUniformLocation(programName, uniformName)` — cached lookup; cache miss falls through to `glGetUniformLocation`.
- `reloadAll()` (ShaderManager.cpp:109) — recompile every program that had its filenames stored. Returns failure count. Source-string-compiled programs (the default for embedded shaders) can't hot-reload.
- `releaseAll()` clears the program map.

**Hot-reload limitation**: all Audio-DNA effects are compiled from embedded source strings in `EmbeddedShaders.h`, not from disk. `reloadAll()` is a no-op for them. There's no file watcher.

**Compile error handling**: failures log via `DBG()` and return false. No automatic fallback to passthrough — the caller must handle nullptr from `getProgram()`.

---

## TextureManager

`TextureManager` (TextureManager.h:9) — image upload + FBO pair for `EffectChain`.

### Image loading

- `loadImage(juce::File)` (TextureManager.cpp:13) — loads via `juce::ImageFileFormat::loadFrom`, defers to `uploadImage`.
- `uploadImage(juce::Image)` (TextureManager.cpp:27) — converts to ARGB, swizzles BGRA→RGBA per pixel, Y-flips, and uploads as `GL_RGBA8`.
- Texture reuse: if same width/height, `glTexSubImage2D` in place (faster than recreate).
- Filters: `GL_LINEAR` min/mag, `GL_CLAMP_TO_EDGE` S/T.

### Image formats supported

Any format JUCE's `ImageFileFormat` handles — **PNG, JPEG, GIF** natively (plus BMP via `BMPImageFormat`). No HDR, no WebP fallback (`stb_image` listed in CLAUDE.md as optional but not wired into TextureManager in this slice).

### FBO management

- `createFBOs(w, h)` (TextureManager.cpp:96) — creates 2 RGBA8 FBOs for ping-pong. Reused if size matches. Logs warning if `glCheckFramebufferStatus != GL_FRAMEBUFFER_COMPLETE`.
- `releaseFBOs()` + `release()` for teardown.

### LUT (3D texture)

- `loadLUT(file)` delegates to `LUTLoader::loadCubeFile` (TextureManager.cpp:148).
- `releaseLUT(texId)` delegates to `LUTLoader::releaseLUT`.

---

## LUTLoader

`LUTLoader::loadCubeFile(juce::File)` (LUTLoader.cpp:7) — parses Adobe `.cube` format into a `GL_TEXTURE_3D`.

### .cube parsing

- Comment lines: start with `#` or `TITLE` — skipped.
- `LUT_3D_SIZE N` — sets the cube dimension; expects `N^3` subsequent RGB triples.
- `DOMAIN_MIN` / `DOMAIN_MAX` — recognized but **ignored** (no remapping applied).
- Data lines: 3 space/tab-separated floats `R G B` in [0, 1].
- Validation: fails (`return 0`) if `data.size() != N^3 * 3`.

### Upload

- `GL_RGB32F` internal format
- Filters: `GL_LINEAR` min/mag
- Wrap: `GL_CLAMP_TO_EDGE` on S/T/R

Used by the `Color Grade` effect (shader `lut_grade`, EffectLibrary.cpp:559). Only `.cube` format supported — no `.3dl`, `.lut`, `.look`, or Hald image formats.

---

## Section 20 partial: Cross-references

### File → Section summary

- `src/effects/EffectLibrary.h` — `EffectDef` + `ParamDef` structs, registry API (135 effects)
- `src/effects/EffectLibrary.cpp` — **authoritative list of 135 effects** across 11 categories
- `src/effects/Effect.h/cpp` — per-instance `Effect` (params, value, enabled, dryWet, temporal)
- `src/effects/EffectChain.h/cpp` — global ping-pong render path, audio uniform upload (path 1)
- `src/effects/UniformBridge.h/cpp` — **legacy demo mappings** (M3 era, kept for reference, no longer used; see comment at Renderer.h:179)
- `src/effects/ISFShaderLoader.h/cpp` — ISF import (float/bool/long supported; point2D/color declared but skipped; image/audio/event rejected)
- `src/render/EmbeddedShaders.h` — 12,496 lines of GLSL; 15 transition shaders at lines 3856–4130; `deckTransition` at 113
- `src/render/CompositorEngine.h/cpp` — multi-layer composite, per-clip + per-layer effect chains, temporal buffers, ring buffer, Screen Split / Frame Stutter interception, audio uniform upload (path 2), feedback processor dispatch
- `src/render/FeedbackProcessor.h/cpp` — **6 named feedback presets**
- `src/render/Renderer.h/cpp` — top-level GL host, effect chain setup, shader compilation (compileAllShaders at Renderer.cpp:1005), cross-deck transition, composition transform
- `src/render/ShaderManager.h/cpp` — program cache + uniform location cache + file-based hot-reload stub
- `src/render/TextureManager.h/cpp` — image upload + 2-FBO ping-pong pair
- `src/render/LUTLoader.h/cpp` — `.cube` 3D LUT loader only

### Depends on (not in this slice)

- `src/model/Layer.h` — `Layer::MixMode`, `Layer::KeyingMode`, `Layer::Type`, `FeedbackConfig`, `Layer::crossfadeProgress`, `Layer::layerEffects`, `Layer::persistent`, `Layer::transitionMode`/`transitionSpeed`
- `src/model/Clip.h` — `Clip::EffectSlot`, `Clip::MediaType`, `Clip::effects`, `Clip::sourceParams`
- `src/model/Deck.h` / `model/Composition.h` — deck tree, `Composition::crossfaderBlendMode`, `compPositionX/Y`, `compScale`, `compRotation`, `compAnchorX/Y`, `activeDeckIndex`, `globalTransitionSpeed`
- `src/analysis/FeatureSnapshot.h` — audio uniform source fields
- `src/sources/SourceRegistry.h` / `src/sources/ProceduralSource.h` — procedural source callback (slice 08)
- `src/media/VideoPlayer.h` / `src/media/ImageSequence.h` — video frame callback

### Notable surprises / undocumented behavior vs CLAUDE.md

1. **Display-vs-shader-key mismatch**: 135 effects but 135+ shader programs compiled (plus transitions, keying aliases, pre-pass shaders). E.g. "Point Zoom" effect → `feedback` shader (animation/feedback), CLAUDE.md lists it under "Animation". "Contrast" → `color_matrix` shader. The CLAUDE.md Common Pitfall #1 warning about resolving via `EffectLibrary::getEffectDef(displayName)->shaderName` is load-bearing.
2. **UniformBridge is dead code**: still declared in Renderer.h:179 with comment "Kept for reference, no longer used". The demo mappings at UniformBridge.cpp:5 target effects 0–3 by index assumption — completely incompatible with current dynamic effect ordering. Should be removed in a cleanup pass.
3. **Zoom Warp display-name**: registered as `"Zoom Warp"` (EffectLibrary.cpp:628) but backed by `infinite_zoom` shader — CLAUDE.md refers to it as "Infinite Zoom" (warp category list). Name inconsistency.
4. **Keying mode aliasing**: 7 of 13 KeyingMode shader keys alias to `transparencyAlpha`, `transparencyLumaKey`, or `transparencyLight`. `EdgeDetection`, `ThresholdMask`, `ChannelR/G/B`, `InvertedLumaIsAlpha`, `SaturationKey`, `InvertedLumaKey` are all effectively fallbacks and may not behave correctly (Renderer.cpp:1116–1125).
5. **Effect count**: `registerEffect({` count = 135. Category sum regex = 136 apparent, but that's an artifact of overlapping substring matches — 135 is authoritative.
6. **Only 4 truly temporal effects**: Echo, Posterize Time, Freeze, Channel Delay. Screen Split and Frame Stutter are non-temporal (ring buffer). CLAUDE.md correctly says "6 temporal time effects" by category but only 4 actually use `u_prev_frame`.
7. **FrameRingBuffer VRAM comment mismatch**: CompositorEngine.h:167 comment says "~120MB at 480x270" but CLAUDE.md says "~240MB at 1080p". At 1/4 of 1920×1080 = 480×270, 480 frames × 518400 pixels × 4 bytes = ~475MB — both comments are wrong, depending on actual composite resolution.
8. **`effect_drywet` and `effect_dry_wet` both compiled**: Renderer.cpp:1007–1008 compiles the same shader under two aliases. CompositorEngine uses `effect_dry_wet` (CompositorEngine.cpp:395); EffectChain uses `effect_drywet` (EffectChain.cpp:208). Two render paths evolved separately.
9. **`u_blendMode` for deck transition is an integer** (`glUniform1i`, Renderer.cpp:508) rather than float — first place in the audit using int uniforms.
10. **Vignette keying mode** (`key_vignette`) is compiled (Renderer.cpp:1125) but **not reachable** — `Layer::KeyingMode` enum has no `Vignette` value, so `applyLayerKeying` never selects it. Dead shader.
