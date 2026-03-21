# Resolume Effect & Source Integration Plan

> Complete implementation plan for integrating all Resolume Arena effects and sources into Audio-DNA v2.
> Every Resolume effect and source is accounted for — either mapped to an existing Audio-DNA equivalent,
> planned as a new addition with a better name, or explicitly marked as skipped with justification.
>
> This document is implementation-ready. Each new effect/source includes: shader architecture,
> parameter definitions with uniform names, GLSL implementation notes, and registration instructions.

---

## TABLE OF CONTENTS

1. [Status Legend](#1-status-legend)
2. [Per-Effect Infrastructure (Before Individual Effects)](#2-per-effect-infrastructure)
3. [Effects: Complete Resolume-to-AudioDNA Mapping](#3-effects-mapping)
4. [New Effects to Implement (32 total)](#4-new-effects)
5. [Sources: Complete Resolume-to-AudioDNA Mapping](#5-sources-mapping)
6. [New Sources to Implement (12 total)](#6-new-sources)
7. [Implementation Order & Phases](#7-implementation-order)
8. [Shader Architecture Notes](#8-shader-architecture)
9. [Registration & File Conventions](#9-registration-conventions)

---

## 1. STATUS LEGEND

| Symbol | Meaning |
|--------|---------|
| HAVE | Already implemented in Audio-DNA |
| NEW | Must implement — does not exist yet |
| RENAME | We have the equivalent but under a different name |
| SKIP | Not implementing — justification provided |
| INFRA | Requires infrastructure change, not just a shader |

---

## 2. PER-EFFECT INFRASTRUCTURE

These are architectural changes that must be completed BEFORE adding individual effects, because multiple new effects depend on them.

### 2.1 Per-Effect Dry/Wet Mix (INFRA — CRITICAL)

**What**: Every Resolume effect has an Opacity slider that blends between the unaffected and affected image. Audio-DNA effects currently have no universal dry/wet control.

**Implementation**:
- Add `float dryWet = 1.0f;` field to `Effect` struct in `src/effects/Effect.h`
- In `EffectChain::render()`, after rendering each effect:
  - If `dryWet < 1.0`, blend the effect output with the pre-effect input using `mix(original, effected, dryWet)`
  - This requires keeping a copy of the pre-effect texture (already available from the ping-pong FBO — it's the "read" FBO before swap)
- Add a compositing shader `effect_drywet`:
  ```glsl
  uniform sampler2D u_texture;     // effected
  uniform sampler2D u_original;    // pre-effect
  uniform float u_drywet;
  void main() {
      vec4 eff = texture(u_texture, v_uv);
      vec4 orig = texture(u_original, v_uv);
      fragColor = mix(orig, eff, u_drywet);
  }
  ```
- In the Inspector, show the dry/wet slider as the FIRST parameter of every effect (above the effect-specific params)
- The dry/wet slider gets a signal routing triangle like any other parameter

**Files to modify**: `Effect.h`, `EffectChain.h/cpp`, `EmbeddedShaders.h`, `ClipInspector.cpp`

### 2.2 Temporal Effect Buffer System (INFRA — REQUIRED FOR TIME EFFECTS)

**What**: Resolume has Delay, Trails, Feedback, and Stop Motion effects that need access to PREVIOUS FRAMES. Our current effect chain only sees the current frame.

**Implementation**:
- Add a `FrameHistory` class that maintains a ring buffer of N previous frame textures (configurable, default 60 = 1 second at 60fps)
- Each frame, after the composition is rendered, copy the output to the history ring buffer
- Time-based effects receive `u_prev_frame_N` uniforms (or a texture array) pointing into the history
- The `FrameHistory` is owned by `Renderer` and passed to `EffectChain` during render
- Memory cost: 60 frames × 1920×1080 × 4 bytes = ~475 MB at 1080p. Optimize by:
  - Only allocating history when a time-based effect is active
  - Using quarter-resolution history textures (960×540 = ~119 MB)
  - Configurable depth per effect (Trails needs 1-10 frames, Delay needs up to 60)

**Alternative (simpler for Trails/Feedback)**: Use the ping-pong FBO system — the "previous output" is already in the other FBO. For Trails, just don't fully clear the output FBO between frames. For Feedback, read from the previous frame's FBO as a secondary texture input.

**Recommended approach**: Start with the simpler ping-pong approach for Trails and Feedback. Add the full FrameHistory ring buffer only if Delay (arbitrary frame offset) is needed.

**Files to create**: `src/render/FrameHistory.h/cpp`
**Files to modify**: `Renderer.h/cpp`, `EffectChain.h/cpp`, `EmbeddedShaders.h`

### 2.3 LUT Loading System (INFRA — REQUIRED FOR LUT EFFECT)

**What**: Load `.cube` files and upload as 3D textures for color grading.

**Implementation**:
- Parse `.cube` file format (simple text: header lines starting with `#`, `TITLE`, `LUT_3D_SIZE N`, then N×N×N lines of `R G B` floats)
- Upload parsed data as `GL_TEXTURE_3D` with `GL_RGB32F` internal format
- The LUT effect shader samples this 3D texture using the input pixel's RGB as 3D coordinates
- LUT effect acts like any other effect with dry/wet + blend mode
- Store loaded LUTs in `TextureManager` with a `loadLUT(filePath)` method

**Files to create**: `src/render/LUTLoader.h/cpp`
**Files to modify**: `TextureManager.h/cpp`, `EmbeddedShaders.h`, `EffectLibrary.cpp`

---

## 3. EFFECTS: COMPLETE RESOLUME-TO-AUDIODNA MAPPING

Every confirmed Resolume effect mapped to our status.

### 3A. Color Effects

| # | Resolume Name | Audio-DNA Name | Status | Notes |
|---|--------------|---------------|--------|-------|
| 1 | AddSubtract | **Color Shift** | HAVE | Our Color Shift adds/subtracts per-channel. Same concept. |
| 2 | Bright.Contrast | **Brightness** + **Contrast** | HAVE | We have separate Brightness and Contrast effects. Resolume combines them. Keep separate — more flexible. |
| 3 | Colorize | **Duotone** | HAVE | Our Duotone maps luminance to two colors. Same function. |
| 4 | Color Pass | **Selective Color** | HAVE | Our Selective Color keeps a hue range, desaturates rest. Identical. |
| 5 | Greyscale | — | NEW | See Section 4, #C1 |
| 6 | Hue Rotate | **Hue Shift** | HAVE | Identical function. |
| 7 | Invert RGB | **Invert** | HAVE | Our Invert has an amount parameter (0=normal, 1=fully inverted). |
| 8 | Levels | **Gamma Levels** | HAVE | Our Gamma Levels has black/white/gamma. Same as Resolume Levels. |
| 9 | Posterize | **Posterize** | HAVE | Identical. |
| 10 | Saturation | **Saturation** | HAVE | Identical. |
| 11 | Threshold | — | NEW | See Section 4, #C2 |
| 12 | Exposure | — | NEW | See Section 4, #C3 |
| 13 | Vibrance | — | NEW | See Section 4, #C4 |
| 14 | Tint | **Split Tone** | HAVE | Our Split Tone tints shadows/highlights separately. Superset. |
| 15 | Solarize | **Solarize** | HAVE | Identical. |
| 16 | Recolour | — | NEW | See Section 4, #C5 |
| 17 | LUT | — | NEW (INFRA) | See Section 4, #C6. Requires LUT loading infrastructure. |

### 3B. Warp / Distortion Effects

| # | Resolume Name | Audio-DNA Name | Status | Notes |
|---|--------------|---------------|--------|-------|
| 1 | Distortion | **Liquid** | HAVE | Our Liquid does turbulent displacement. Similar to their broken-TV Distortion. |
| 2 | Ripples | **Ripple** | HAVE | Identical concentric wave distortion. |
| 3 | Wave Warp | **Wave** | HAVE | Identical directional sine wave. |
| 4 | Kaleidoscope | **Kaleidoscope** | HAVE | Identical. |
| 5 | Bendoscope | — | NEW | See Section 4, #W1 |
| 6 | Polar Kaleidoscope | **Polar Coords** | HAVE | Our Polar Coords does polar coordinate transformation. Similar. |
| 7 | Mirror | **Mirror** | HAVE | Identical. |
| 8 | Mirror Quad | — | NEW | See Section 4, #W2 |
| 9 | Flip | — | NEW | See Section 4, #W3 |
| 10 | Suckr | **Bulge** | HAVE | Our Bulge does pinch/bulge from center. Same concept, different name. |
| 11 | Fish Eye | **Fisheye** | HAVE | Identical. |
| 12 | Twirl | **Twirl** | HAVE | Identical. |
| 13 | Twisted | **Swirl** | HAVE | Our Swirl does rotational displacement. Same as Twisted. |
| 14 | Keystone | — | SKIP | Keystone is for projection mapping correction. Not relevant for us. |
| 15 | Keystone Crop | — | SKIP | Same — projection mapping tool. |
| 16 | Keystone Mask | — | SKIP | Same — projection mapping tool. |
| 17 | UV Map | — | NEW | See Section 4, #W4 |
| 18 | Displace | — | RENAME of existing | Our **Glitch Displace** is UV displacement. Resolume's Displace uses luminance of another layer, which requires INFRA (second texture input). SKIP for now — Video Router addresses this use case. |
| 19 | Goo | — | NEW | See Section 4, #W5 |
| 20 | Space Warper | — | NEW | See Section 4, #W6 |
| 21 | Shifty | — | SKIP | Very similar to our Shear + Wave. Low unique value. |

### 3C. Blur Effects

| # | Resolume Name | Audio-DNA Name | Status | Notes |
|---|--------------|---------------|--------|-------|
| 1 | Blur | **Gaussian Blur** | HAVE | Identical. |
| 2 | Pixel Blur | **Pixelate** | HAVE | Our Pixelate does block/mosaic blur. Same result. |
| 3 | Radial Blur | **Zoom Blur** | HAVE | Our Zoom Blur is radial blur from center. Identical concept. |
| 4 | Edge Blur | — | NEW | See Section 4, #B1 |
| 5 | Sharpen | — | NEW | See Section 4, #B2 |

### 3D. Stylize Effects

| # | Resolume Name | Audio-DNA Name | Status | Notes |
|---|--------------|---------------|--------|-------|
| 1 | LoRez | **Pixelate** | HAVE | Our Pixelate does low-res mosaic. Resolume's LoRez adds color reduction. Consider adding a color reduction param to Pixelate. |
| 2 | Dither | **Dither** | HAVE | Identical ordered dithering. |
| 3 | Dot Screen | **Dot Matrix** | HAVE | Our Dot Matrix is halftone dot pattern. Same concept. |
| 4 | Raster | **CRT** | HAVE | Our CRT has scanline simulation. Same aesthetic. |
| 5 | Hatched | **Crosshatch** | HAVE | Identical cross-hatching. |
| 6 | Edge Detection | **Edge Detect** | HAVE | Identical. |
| 7 | Bloom | **Glow** | HAVE | Our Glow does bright-area bloom. Same concept. |
| 8 | Static | **Noise** | HAVE | Our Noise overlay does the same TV static. |
| 9 | Vignette | **Vignette** | HAVE | Identical. |
| 10 | Expand | — | SKIP | Dilate/expand bright areas. Very niche. Low priority. |
| 11 | Dilate | — | SKIP | Thicken image regions. Niche. |
| 12 | Kuwahara | **Oil Paint** | HAVE | Our Oil Paint uses a similar smoothing kernel. |
| 13 | VHSifyer | **VHS** | HAVE | Identical VHS look. |
| 14 | Strokes | — | NEW | See Section 4, #S1 |
| 15 | CRT (Wire) | **CRT** | HAVE | Identical CRT look. |
| 16 | Reducto | — | SKIP | Lo-fi reduction. Overlap with our Pixelate + Posterize combo. |

### 3E. Glitch / Shift Effects

| # | Resolume Name | Audio-DNA Name | Status | Notes |
|---|--------------|---------------|--------|-------|
| 1 | Shift Glitch | **Block Glitch** | HAVE | Our Block Glitch does random block displacement. Same concept. |
| 2 | Shift RGB | **RGB Split** | HAVE | Our RGB Split offsets channels. Identical. |
| 3 | Fragment | — | NEW | See Section 4, #G1 |
| 4 | Freeze | — | NEW (INFRA) | See Section 4, #G2. Requires temporal buffer. |
| 5 | Blow | — | NEW | See Section 4, #G3 |
| 6 | Flash | — | NEW | See Section 4, #G4 |
| 7 | Noise | **Noise** | HAVE | Identical noise overlay. |
| 8 | TVA (Total Visual Annihilation) | — | NEW | See Section 4, #G5 |

### 3F. Composite / Clone Effects

| # | Resolume Name | Audio-DNA Name | Status | Notes |
|---|--------------|---------------|--------|-------|
| 1 | Linear Cloner | — | NEW | See Section 4, #X1 |
| 2 | Radial Cloner | — | NEW | See Section 4, #X2 |
| 3 | Cube Tiles | — | NEW | See Section 4, #X3 |

### 3G. Time-Based Effects

| # | Resolume Name | Audio-DNA Name | Status | Notes |
|---|--------------|---------------|--------|-------|
| 1 | Delay | — | NEW (INFRA) | See Section 4, #T1. Requires FrameHistory. |
| 2 | Delay RGB | — | NEW (INFRA) | See Section 4, #T2. Requires FrameHistory. |
| 3 | Trails | — | NEW (INFRA) | See Section 4, #T3. Can use ping-pong FBO approach. |
| 4 | Feedback | — | NEW (INFRA) | See Section 4, #T4. Can use ping-pong FBO approach. |
| 5 | Strobe | **Strobe** | HAVE | Identical. |
| 6 | Stop Motion | — | NEW (INFRA) | See Section 4, #T5. Requires frame hold. |
| 7 | Stroboscope (effect version) | — | SKIP | The source version (Stroboscope generator) covers this. |

### 3H. Transform Effects

| # | Resolume Name | Audio-DNA Name | Status | Notes |
|---|--------------|---------------|--------|-------|
| 1 | Transform | — | SKIP | We handle transform at the clip/layer level via the Transform section in the Inspector, not as an effect. This is architecturally better. |
| 2 | Tile | — | NEW | See Section 4, #R1 |
| 3 | Slide | — | NEW | See Section 4, #R2 |
| 4 | Iterate | — | NEW | See Section 4, #R3 |

### 3I. 3D / Geometry Effects

| # | Resolume Name | Audio-DNA Name | Status | Notes |
|---|--------------|---------------|--------|-------|
| 1 | Stingy Sphere | **Sphere Wrap** | HAVE | Identical 3D sphere mapping. |
| 2 | Cube Tiles | — | NEW | See Section 4, #X3 (listed under Composite). |
| 3 | Luma Waves | — | NEW | See Section 4, #D1 |
| 4 | Pixels In Space | — | NEW | See Section 4, #D2 |
| 5 | Point Grid | — | NEW | See Section 4, #D3 |

### 3J. Special / Unique Effects

| # | Resolume Name | Audio-DNA Name | Status | Notes |
|---|--------------|---------------|--------|-------|
| 1 | Circles | — | SKIP | Video as concentric circles. Very niche, similar to our Dot Matrix. |
| 2 | Snow | — | SKIP | Particle effect. Better as a source/generator than an effect. |
| 3 | Videowall | — | NEW | See Section 4, #J1 |
| 4 | Radar | — | SKIP | Better as a source. We can add a radar source later. |
| 5 | Triangulate | — | NEW | See Section 4, #J2 |
| 6 | Twitch | — | SKIP | Random jitter. Overlap with our Shake effect. |
| 7 | Sparkles | — | SKIP | Particle overlay. Better as a source. |
| 8 | Drop Shadow | — | NEW | See Section 4, #J3 |
| 9 | Auto Mask | — | NEW | See Section 4, #J4 |
| 10 | Chromakey (effect) | — | HAVE (utility) | We have `transparencyChromaKey` as a keying shader. Not a user-facing effect. Consider exposing it. See Section 4, #J5. |
| 11 | Particle System | — | SKIP for effects | Better as a source. See Sources section. |
| 12 | Smooth Transform | — | SKIP | We handle smooth transitions in the routing/signal system, not per-transform. |
| 13 | Screen Shake | **Shake** | HAVE | Our Shake does the same thing. |
| 14 | FeedbackPro | — | NEW | See Section 4, #T4 (combined with Feedback). |
| 15 | Slit Scanner | **Slit Scan** | HAVE | Identical. |
| 16 | Pixel High Pass | — | SKIP | Very niche per-channel high pass. Low value. |

---

## 4. NEW EFFECTS TO IMPLEMENT (32 total)

Each entry includes: display name (with renamed Resolume effects getting cooler/more descriptive names), shader key, category, parameters with uniforms and defaults, GLSL implementation approach, and registration code.

### Color Category — New Effects

#### C1: Greyscale
- **Display Name**: Greyscale
- **Shader Key**: `greyscale`
- **Category**: `color`
- **Resolume Equivalent**: Greyscale
- **Parameters**:
  - `method` (`u_grey_method`, default 0.0) — 0.0-0.33 = luminance (Rec.709), 0.33-0.66 = average, 0.66-1.0 = desaturate (min/max)
  - `amount` (`u_grey_amount`, default 0.0) — mix between original and greyscale
- **GLSL Approach**:
  ```glsl
  float luma = dot(col.rgb, vec3(0.2126, 0.7152, 0.0722));  // Rec.709
  float avg = (col.r + col.g + col.b) / 3.0;
  float desat = (max(max(col.r, col.g), col.b) + min(min(col.r, col.g), col.b)) / 2.0;
  float grey = mix(mix(luma, avg, clamp((method - 0.33) * 3.0, 0.0, 1.0)),
                   desat, clamp((method - 0.66) * 3.0, 0.0, 1.0));
  fragColor = vec4(mix(col.rgb, vec3(grey), amount), col.a);
  ```
- **Complexity**: Small

#### C2: Threshold
- **Display Name**: Threshold
- **Shader Key**: `threshold`
- **Category**: `color`
- **Resolume Equivalent**: Threshold
- **Parameters**:
  - `level` (`u_threshold_level`, default 0.5) — brightness cutoff
  - `amount` (`u_threshold_amount`, default 0.0) — mix with original
- **GLSL Approach**:
  ```glsl
  float luma = dot(col.rgb, vec3(0.2126, 0.7152, 0.0722));
  float bw = step(level, luma);
  fragColor = vec4(mix(col.rgb, vec3(bw), amount), col.a);
  ```
- **Complexity**: Small

#### C3: Exposure
- **Display Name**: Exposure
- **Shader Key**: `exposure`
- **Category**: `color`
- **Resolume Equivalent**: Exposure
- **Parameters**:
  - `amount` (`u_exposure_amount`, default 0.5) — 0.0 = -3 stops, 0.5 = no change, 1.0 = +3 stops
- **GLSL Approach**:
  ```glsl
  float ev = (amount - 0.5) * 6.0; // -3 to +3 stops
  fragColor = vec4(col.rgb * pow(2.0, ev), col.a);
  ```
- **Complexity**: Small

#### C4: Vibrance
- **Display Name**: Vibrance
- **Shader Key**: `vibrance`
- **Category**: `color`
- **Resolume Equivalent**: Vibrance
- **Parameters**:
  - `amount` (`u_vibrance_amount`, default 0.5) — 0 = desaturate, 0.5 = no change, 1.0 = max vibrance
- **GLSL Approach**:
  ```glsl
  float maxC = max(max(col.r, col.g), col.b);
  float minC = min(min(col.r, col.g), col.b);
  float sat = maxC - minC;
  float strength = (amount - 0.5) * 2.0; // -1 to +1
  // Boost less-saturated pixels more, leave already-saturated pixels alone
  float adjust = strength * (1.0 - sat) * 0.5;
  float luma = dot(col.rgb, vec3(0.2126, 0.7152, 0.0722));
  fragColor = vec4(mix(vec3(luma), col.rgb, 1.0 + adjust), col.a);
  ```
- **Complexity**: Small

#### C5: Palette Remap
- **Display Name**: Palette Remap (Resolume: "Recolour")
- **Shader Key**: `palette_remap`
- **Category**: `color`
- **Parameters**:
  - `palette` (`u_palette_index`, default 0.0) — selects from built-in palettes (0-1 maps across ~8 palettes: neon, sunset, ocean, fire, pastel, matrix, thermal, ice)
  - `cycle` (`u_palette_cycle`, default 0.0) — shifts the palette mapping
  - `amount` (`u_palette_amount`, default 0.0) — mix with original
- **GLSL Approach**: Use luminance to index into a procedural palette via `cos()` color cycling:
  ```glsl
  float luma = dot(col.rgb, vec3(0.2126, 0.7152, 0.0722));
  float t = luma + cycle;
  // Palette selection via cosine harmonics (IQ palette technique)
  vec3 a, b, c, d; // per-palette coefficients
  // ... select a/b/c/d based on palette_index ...
  vec3 mapped = a + b * cos(6.28318 * (c * t + d));
  fragColor = vec4(mix(col.rgb, mapped, amount), col.a);
  ```
- **Complexity**: Medium (needs ~8 palette definitions)

#### C6: Color Grade (LUT)
- **Display Name**: Color Grade
- **Shader Key**: `lut_grade`
- **Category**: `color`
- **Resolume Equivalent**: LUT
- **Requires**: LUT Loading Infrastructure (Section 2.3)
- **Parameters**:
  - `amount` (`u_lut_amount`, default 0.0) — mix with original
- **GLSL Approach**:
  ```glsl
  uniform sampler3D u_lut;
  vec3 graded = texture(u_lut, col.rgb).rgb;
  fragColor = vec4(mix(col.rgb, graded, amount), col.a);
  ```
- **Note**: The LUT texture is loaded separately via the LUT infrastructure. The effect just samples it.
- **Complexity**: Small (shader), Medium (infrastructure)

### Warp Category — New Effects

#### W1: Bendoscope
- **Display Name**: Bendoscope
- **Shader Key**: `bendoscope`
- **Category**: `warp`
- **Resolume Equivalent**: Bendoscope
- **Parameters**:
  - `divisions` (`u_bendo_divisions`, default 0.3) — maps to 2-12 segments
  - `bend` (`u_bendo_bend`, default 0.5) — curvature amount
  - `rotation` (`u_bendo_rotation`, default 0.0) — rotation angle
- **GLSL Approach**: Convert to polar, apply modulo for segments (like kaleidoscope), but add a sine-based bend to the radius before reflecting:
  ```glsl
  vec2 p = v_uv - 0.5;
  float angle = atan(p.y, p.x);
  float radius = length(p);
  int segs = int(divisions * 10.0) + 2;
  float segAngle = 6.28318 / float(segs);
  angle = mod(angle + rotation * 6.28318, segAngle);
  if (angle > segAngle * 0.5) angle = segAngle - angle; // reflect
  radius += sin(angle * 3.0) * bend * 0.2; // bend
  vec2 uv = vec2(cos(angle), sin(angle)) * radius + 0.5;
  ```
- **Complexity**: Small

#### W2: Quad Mirror
- **Display Name**: Quad Mirror (Resolume: "Mirror Quad")
- **Shader Key**: `quad_mirror`
- **Category**: `warp`
- **Parameters**:
  - `center_x` (`u_quadmir_cx`, default 0.5) — mirror center X
  - `center_y` (`u_quadmir_cy`, default 0.5) — mirror center Y
- **GLSL Approach**:
  ```glsl
  vec2 uv = v_uv;
  uv.x = (uv.x < center_x) ? uv.x : 2.0 * center_x - uv.x;
  uv.y = (uv.y < center_y) ? uv.y : 2.0 * center_y - uv.y;
  ```
- **Complexity**: Small

#### W3: Flip
- **Display Name**: Flip
- **Shader Key**: `flip`
- **Category**: `warp`
- **Resolume Equivalent**: Flip
- **Parameters**:
  - `horizontal` (`u_flip_h`, default 0.0) — 0 = normal, 1 = flipped horizontally
  - `vertical` (`u_flip_v`, default 0.0) — 0 = normal, 1 = flipped vertically
- **GLSL Approach**:
  ```glsl
  vec2 uv = v_uv;
  if (horizontal > 0.5) uv.x = 1.0 - uv.x;
  if (vertical > 0.5) uv.y = 1.0 - uv.y;
  ```
- **Complexity**: Small

#### W4: UV Remap
- **Display Name**: UV Remap (Resolume: "UV Map")
- **Shader Key**: `uv_remap`
- **Category**: `warp`
- **Parameters**:
  - `amount` (`u_uvremap_amount`, default 0.0) — displacement strength
  - `scale` (`u_uvremap_scale`, default 0.5) — UV noise scale
  - `speed` (`u_uvremap_speed`, default 0.3) — animation speed
- **GLSL Approach**: Use animated Perlin noise as a displacement map:
  ```glsl
  vec2 noise = vec2(
      snoise(vec3(v_uv * scale * 4.0, u_time * speed)),
      snoise(vec3(v_uv * scale * 4.0 + 100.0, u_time * speed))
  );
  vec2 uv = v_uv + noise * amount * 0.2;
  ```
- **Complexity**: Small

#### W5: Liquid Morph
- **Display Name**: Liquid Morph (Resolume: "Goo")
- **Shader Key**: `liquid_morph`
- **Category**: `warp`
- **Parameters**:
  - `viscosity` (`u_goo_viscosity`, default 0.5) — flow speed
  - `amount` (`u_goo_amount`, default 0.0) — distortion strength
  - `scale` (`u_goo_scale`, default 0.5) — pattern scale
- **GLSL Approach**: Multi-octave simplex noise displacement with time-varying flow field:
  ```glsl
  float t = u_time * viscosity;
  vec2 flow = vec2(
      fbm(vec3(v_uv * scale * 3.0, t * 0.7)),
      fbm(vec3(v_uv * scale * 3.0 + 50.0, t * 0.7 + 20.0))
  );
  vec2 uv = v_uv + flow * amount * 0.15;
  ```
- **Complexity**: Medium (needs fbm/noise functions)

#### W6: Warp Field
- **Display Name**: Warp Field (Resolume: "Space Warper")
- **Shader Key**: `warp_field`
- **Category**: `warp`
- **Parameters**:
  - `amount` (`u_warpfield_amount`, default 0.0) — warp intensity
  - `frequency` (`u_warpfield_freq`, default 0.5) — warp frequency
  - `speed` (`u_warpfield_speed`, default 0.3) — animation speed
- **GLSL Approach**: Gravity-like warping using multiple attractor points animated by time:
  ```glsl
  vec2 uv = v_uv;
  for (int i = 0; i < 4; i++) {
      vec2 attractor = vec2(
          0.5 + 0.3 * sin(u_time * speed + float(i) * 1.5),
          0.5 + 0.3 * cos(u_time * speed * 0.7 + float(i) * 2.1)
      );
      vec2 diff = uv - attractor;
      float dist = length(diff);
      uv += diff / (dist * dist + 0.1) * amount * 0.01 * freq;
  }
  ```
- **Complexity**: Small

### Blur Category — New Effects

#### B1: Edge Blur
- **Display Name**: Edge Blur
- **Shader Key**: `edge_blur`
- **Category**: `blur`
- **Resolume Equivalent**: Edge Blur
- **Parameters**:
  - `threshold` (`u_edgeblur_threshold`, default 0.5) — edge detection sensitivity
  - `amount` (`u_edgeblur_amount`, default 0.0) — blur radius on edges
- **GLSL Approach**: Sobel edge detection → use edge magnitude as blur kernel radius:
  ```glsl
  // Detect edges via Sobel
  float edge = sobelMagnitude(v_uv);
  // Apply blur only where edge > threshold
  float blurAmount = smoothstep(threshold, 1.0, edge) * amount;
  vec4 blurred = blur5x5(v_uv, blurAmount * 5.0 / u_resolution);
  fragColor = mix(col, blurred, blurAmount);
  ```
- **Complexity**: Medium

#### B2: Sharpen
- **Display Name**: Sharpen
- **Shader Key**: `sharpen`
- **Category**: `blur`
- **Resolume Equivalent**: Sharpen
- **Parameters**:
  - `amount` (`u_sharpen_amount`, default 0.0) — sharpening intensity (0-1 maps to 0-3x)
  - `radius` (`u_sharpen_radius`, default 0.3) — kernel radius
- **GLSL Approach**: Unsharp mask — subtract blurred version from original:
  ```glsl
  vec4 blurred = blur3x3(v_uv, radius * 3.0 / u_resolution);
  vec4 sharpened = col + (col - blurred) * amount * 3.0;
  fragColor = clamp(sharpened, 0.0, 1.0);
  ```
- **Complexity**: Small

### Stylize Category — New Effect

#### S1: Brush Strokes
- **Display Name**: Brush Strokes (Resolume: "Strokes")
- **Shader Key**: `brush_strokes`
- **Category**: `pattern`
- **Parameters**:
  - `size` (`u_brush_size`, default 0.4) — brush stroke size
  - `angle` (`u_brush_angle`, default 0.0) — base stroke angle
  - `flow` (`u_brush_flow`, default 0.5) — stroke flow/direction variation
  - `amount` (`u_brush_amount`, default 0.0) — effect intensity
- **GLSL Approach**: Sample image at directional offsets based on local gradient, then blend with a brush texture pattern:
  ```glsl
  // Compute gradient direction at this pixel
  vec2 grad = sobelGradient(v_uv);
  float angle_local = atan(grad.y, grad.x) + angle * 6.28;
  // Sample along the stroke direction
  vec2 dir = vec2(cos(angle_local), sin(angle_local));
  vec4 stroke = vec4(0.0);
  for (int i = -3; i <= 3; i++) {
      vec2 offset = dir * float(i) * size * 0.01;
      stroke += texture(u_texture, v_uv + offset);
  }
  stroke /= 7.0;
  fragColor = mix(col, stroke, amount);
  ```
- **Complexity**: Medium

### Glitch Category — New Effects

#### G1: Fragment Burst
- **Display Name**: Fragment Burst (Resolume: "Fragment")
- **Shader Key**: `fragment_burst`
- **Category**: `glitch`
- **Parameters**:
  - `copies` (`u_frag_copies`, default 0.3) — number of copies (maps to 2-8)
  - `spread` (`u_frag_spread`, default 0.0) — how far copies spread
  - `rotation` (`u_frag_rotation`, default 0.0) — per-copy rotation offset
  - `scale` (`u_frag_scale`, default 0.5) — per-copy scale
- **GLSL Approach**: Render the image multiple times at different positions arranged in a circle:
  ```glsl
  int n = int(copies * 6.0) + 2;
  vec4 result = vec4(0.0);
  for (int i = 0; i < n; i++) {
      float a = float(i) / float(n) * 6.28318;
      vec2 offset = vec2(cos(a), sin(a)) * spread * 0.3;
      float rot = rotation * float(i) * 0.5;
      float s = 1.0 / (1.0 + float(i) * (1.0 - scale));
      vec2 uv = rotateUV(v_uv - 0.5 - offset, rot) / s + 0.5;
      result += texture(u_texture, uv) / float(n);
  }
  ```
- **Complexity**: Small

#### G2: Time Freeze
- **Display Name**: Time Freeze (Resolume: "Freeze")
- **Shader Key**: `time_freeze`
- **Category**: `glitch`
- **Requires**: Temporal buffer (ping-pong FBO — keep previous frame)
- **Parameters**:
  - `freeze_x` (`u_freeze_x`, default 0.5) — X position of freeze region
  - `freeze_y` (`u_freeze_y`, default 0.5) — Y position of freeze region
  - `size` (`u_freeze_size`, default 0.3) — frozen region size
  - `amount` (`u_freeze_amount`, default 0.0) — 0 = no freeze, 1 = fully frozen
- **GLSL Approach**:
  ```glsl
  uniform sampler2D u_prev_frame;
  float dist = distance(v_uv, vec2(freeze_x, freeze_y));
  float mask = smoothstep(size, size - 0.05, dist) * amount;
  vec4 frozen = texture(u_prev_frame, v_uv);
  fragColor = mix(col, frozen, mask);
  ```
- **Complexity**: Medium (needs previous frame access)

#### G3: Pixel Explosion
- **Display Name**: Pixel Explosion (Resolume: "Blow")
- **Shader Key**: `pixel_explosion`
- **Category**: `glitch`
- **Parameters**:
  - `force` (`u_explode_force`, default 0.0) — explosion intensity
  - `decay` (`u_explode_decay`, default 0.5) — how quickly explosion fades with distance
  - `center_x` (`u_explode_cx`, default 0.5)
  - `center_y` (`u_explode_cy`, default 0.5)
- **GLSL Approach**: Push pixels away from center based on their brightness:
  ```glsl
  vec2 dir = v_uv - vec2(center_x, center_y);
  float dist = length(dir);
  float luma = dot(texture(u_texture, v_uv).rgb, vec3(0.299, 0.587, 0.114));
  vec2 offset = normalize(dir) * force * luma * 0.2 * exp(-dist * decay * 5.0);
  vec2 uv = v_uv - offset;
  ```
- **Complexity**: Small

#### G4: Color Flash
- **Display Name**: Color Flash (Resolume: "Flash")
- **Shader Key**: `color_flash`
- **Category**: `glitch`
- **Parameters**:
  - `intensity` (`u_flash_intensity`, default 0.0) — flash brightness (0 = off, 1 = full white)
  - `color_r` (`u_flash_r`, default 1.0) — flash color R
  - `color_g` (`u_flash_g`, default 1.0) — flash color G
  - `color_b` (`u_flash_b`, default 1.0) — flash color B
  - `decay` (`u_flash_decay`, default 0.5) — flash decay speed
- **GLSL Approach**:
  ```glsl
  vec3 flashColor = vec3(color_r, color_g, color_b);
  // Use beatPhase for rhythmic flashing, or direct intensity for manual
  float flash = intensity * pow(1.0 - fract(u_time * (1.0 + decay * 10.0)), 3.0);
  fragColor = vec4(mix(col.rgb, flashColor, flash), col.a);
  ```
- **Complexity**: Small

#### G5: Signal Destroy
- **Display Name**: Signal Destroy (Resolume: "TVA — Total Visual Annihilation")
- **Shader Key**: `signal_destroy`
- **Category**: `glitch`
- **Parameters**:
  - `amount` (`u_destroy_amount`, default 0.0) — destruction intensity
  - `speed` (`u_destroy_speed`, default 0.5) — animation speed
  - `mode` (`u_destroy_mode`, default 0.0) — 0-0.33 = pixel scatter, 0.33-0.66 = channel destroy, 0.66-1.0 = full corruption
- **GLSL Approach**: Layer multiple glitch techniques based on mode:
  ```glsl
  // Combines: random UV offset, channel separation, block corruption, scan line glitch
  float n = hash(floor(v_uv * 20.0) + floor(u_time * speed * 30.0));
  vec2 uv = v_uv;
  // UV scatter
  uv += (vec2(hash(n), hash(n + 1.0)) - 0.5) * amount * 0.3;
  // Channel split
  float r = texture(u_texture, uv + vec2(amount * 0.05 * n, 0.0)).r;
  float g = texture(u_texture, uv).g;
  float b = texture(u_texture, uv - vec2(amount * 0.05 * n, 0.0)).b;
  // Block corruption
  if (n > 1.0 - amount * 0.3) { r = n; g = 1.0 - n; b = hash(n + 2.0); }
  fragColor = vec4(r, g, b, col.a);
  ```
- **Complexity**: Medium

### Composite Category — New Effects

#### X1: Line Cloner
- **Display Name**: Line Cloner (Resolume: "Linear Cloner")
- **Shader Key**: `line_cloner`
- **Category**: `composite`
- **Parameters**:
  - `copies` (`u_lineclone_copies`, default 0.3) — number of copies (maps to 2-10)
  - `offset_x` (`u_lineclone_ox`, default 0.5) — X spacing per copy
  - `offset_y` (`u_lineclone_oy`, default 0.5) — Y spacing per copy
  - `scale` (`u_lineclone_scale`, default 0.5) — per-copy scale factor
  - `rotation` (`u_lineclone_rotation`, default 0.0) — per-copy rotation increment
- **GLSL Approach**: Render N copies by iterating UV transforms:
  ```glsl
  int n = int(copies * 8.0) + 2;
  vec4 result = vec4(0.0);
  float maxAlpha = 0.0;
  for (int i = 0; i < n; i++) {
      float fi = float(i) - float(n - 1) * 0.5;
      vec2 offset = vec2((offset_x - 0.5) * fi * 0.2, (offset_y - 0.5) * fi * 0.2);
      float rot = (rotation - 0.5) * fi * 0.5;
      float s = pow(scale * 2.0, fi * 0.1);
      vec2 uv = rotateUV((v_uv - 0.5 - offset) / s, rot) + 0.5;
      vec4 sample = texture(u_texture, uv);
      result = result + sample * (1.0 - result.a); // over compositing
  }
  fragColor = result;
  ```
- **Complexity**: Medium

#### X2: Radial Cloner
- **Display Name**: Radial Cloner (Resolume: "Radial Cloner")
- **Shader Key**: `radial_cloner`
- **Category**: `composite`
- **Parameters**:
  - `copies` (`u_radclone_copies`, default 0.3) — number of copies (maps to 2-12)
  - `radius` (`u_radclone_radius`, default 0.3) — distance from center
  - `rotation` (`u_radclone_rotation`, default 0.0) — base rotation
  - `scale` (`u_radclone_scale`, default 0.5) — per-copy scale
- **GLSL Approach**: Place copies around a circle:
  ```glsl
  int n = int(copies * 10.0) + 2;
  vec4 result = vec4(0.0);
  for (int i = 0; i < n; i++) {
      float angle = float(i) / float(n) * 6.28318 + rotation * 6.28318;
      vec2 center = vec2(0.5) + vec2(cos(angle), sin(angle)) * radius * 0.4;
      float s = scale * 2.0;
      vec2 uv = (v_uv - center) / s + 0.5;
      if (uv.x >= 0.0 && uv.x <= 1.0 && uv.y >= 0.0 && uv.y <= 1.0) {
          vec4 sample = texture(u_texture, uv);
          result = result + sample * (1.0 - result.a);
      }
  }
  fragColor = max(result, texture(u_texture, v_uv)); // additive with original
  ```
- **Complexity**: Medium

#### X3: Cube Scatter
- **Display Name**: Cube Scatter (Resolume: "Cube Tiles")
- **Shader Key**: `cube_scatter`
- **Category**: `composite`
- **Parameters**:
  - `grid_x` (`u_cubescat_gx`, default 0.3) — grid columns (2-8)
  - `grid_y` (`u_cubescat_gy`, default 0.3) — grid rows (2-8)
  - `explode` (`u_cubescat_explode`, default 0.0) — how far tiles scatter
  - `rotation` (`u_cubescat_rotation`, default 0.0) — per-tile rotation
- **GLSL Approach**: Divide screen into grid, offset each tile with pseudo-random rotation:
  ```glsl
  int gx = int(grid_x * 6.0) + 2;
  int gy = int(grid_y * 6.0) + 2;
  vec2 cell = floor(v_uv * vec2(float(gx), float(gy)));
  vec2 cellUV = fract(v_uv * vec2(float(gx), float(gy)));
  float rnd = hash(cell);
  // Apply per-tile rotation and offset
  vec2 offset = (vec2(hash(cell + 0.1), hash(cell + 0.2)) - 0.5) * explode * 0.5;
  float rot = (rnd - 0.5) * rotation * 3.14;
  vec2 uv = rotateUV(cellUV - 0.5, rot) + 0.5 + offset;
  // Map back to original UV space for this cell
  vec2 originalUV = (cell + uv) / vec2(float(gx), float(gy));
  ```
- **Complexity**: Medium

### Time-Based Category — New Effects

#### T1: Frame Delay
- **Display Name**: Frame Delay (Resolume: "Delay")
- **Shader Key**: `frame_delay`
- **Category**: `time`
- **Requires**: FrameHistory ring buffer (Section 2.2)
- **Parameters**:
  - `amount` (`u_delay_amount`, default 0.0) — delay in frames (0-1 maps to 0-60 frames)
  - `blend` (`u_delay_blend`, default 0.0) — mix between current and delayed frame
- **GLSL Approach**:
  ```glsl
  uniform sampler2D u_delayed_frame; // Set by CPU from FrameHistory
  fragColor = mix(col, texture(u_delayed_frame, v_uv), blend);
  ```
- **Note**: The CPU side selects the appropriate frame from the ring buffer based on `amount`.
- **Complexity**: Medium (shader simple, infrastructure complex)

#### T2: Channel Delay
- **Display Name**: Channel Delay (Resolume: "Delay RGB")
- **Shader Key**: `channel_delay`
- **Category**: `time`
- **Requires**: FrameHistory ring buffer
- **Parameters**:
  - `red_delay` (`u_delay_r`, default 0.0) — red channel delay (0-30 frames)
  - `green_delay` (`u_delay_g`, default 0.0) — green channel delay (0-30 frames)
  - `blue_delay` (`u_delay_b`, default 0.0) — blue channel delay (0-30 frames)
- **GLSL Approach**:
  ```glsl
  uniform sampler2D u_frame_r; // delayed frame for red
  uniform sampler2D u_frame_g; // delayed frame for green
  uniform sampler2D u_frame_b; // delayed frame for blue
  float r = texture(u_frame_r, v_uv).r;
  float g = texture(u_frame_g, v_uv).g;
  float b = texture(u_frame_b, v_uv).b;
  fragColor = vec4(r, g, b, col.a);
  ```
- **Complexity**: Medium

#### T3: Ghost Trails
- **Display Name**: Ghost Trails (Resolume: "Trails")
- **Shader Key**: `ghost_trails`
- **Category**: `time`
- **Requires**: Previous frame access (ping-pong FBO approach — DON'T clear the output FBO)
- **Parameters**:
  - `length` (`u_trail_length`, default 0.0) — trail persistence (0 = no trail, 1 = infinite)
  - `fade` (`u_trail_fade`, default 0.5) — fade-out speed
- **GLSL Approach**:
  ```glsl
  uniform sampler2D u_prev_frame;
  vec4 prev = texture(u_prev_frame, v_uv);
  float persistence = length * 0.98; // decay factor
  vec4 trail = prev * persistence;
  fragColor = max(col, trail); // keep brighter of current or trail
  ```
- **Note**: This effect needs special handling in `EffectChain` — the output FBO must NOT be cleared between frames when this effect is active. Instead, the previous frame is blended in.
- **Complexity**: Medium (shader simple, rendering pipeline change needed)

#### T4: Infinite Feedback
- **Display Name**: Infinite Feedback (Resolume: "FeedbackPro")
- **Shader Key**: `infinite_feedback`
- **Category**: `time`
- **Requires**: Previous frame access (ping-pong FBO)
- **Parameters**:
  - `amount` (`u_feedback_amount`, default 0.0) — feedback intensity
  - `zoom` (`u_feedback_zoom`, default 0.5) — per-frame zoom (0.5 = no zoom, 1 = zoom in)
  - `rotation` (`u_feedback_rotation`, default 0.5) — per-frame rotation (0.5 = none)
  - `hue_shift` (`u_feedback_hue`, default 0.0) — per-frame hue rotation
  - `luma_key` (`u_feedback_lumakey`, default 0.0) — brightness threshold for feedback (dark areas fade)
- **GLSL Approach**:
  ```glsl
  uniform sampler2D u_prev_frame;
  // Transform the previous frame
  vec2 uv = v_uv - 0.5;
  float s = 1.0 + (zoom - 0.5) * 0.1; // slight zoom per frame
  float r = (rotation - 0.5) * 0.05;   // slight rotation per frame
  uv = mat2(cos(r)*s, -sin(r)*s, sin(r)*s, cos(r)*s) * uv;
  uv += 0.5;
  vec4 prev = texture(u_prev_frame, uv);
  // Hue shift the feedback
  prev.rgb = hueRotate(prev.rgb, hue_shift * 0.1);
  // Luma key — dark areas fade to prevent infinite buildup
  float luma = dot(prev.rgb, vec3(0.299, 0.587, 0.114));
  prev *= smoothstep(luma_key, luma_key + 0.1, luma);
  // Blend feedback with current frame
  fragColor = mix(col, max(col, prev), amount);
  ```
- **Complexity**: Medium

#### T5: Frame Hold
- **Display Name**: Frame Hold (Resolume: "Stop Motion")
- **Shader Key**: `frame_hold`
- **Category**: `time`
- **Requires**: Frame counter + held texture
- **Parameters**:
  - `rate` (`u_hold_rate`, default 0.5) — hold duration (0 = every frame, 1 = hold for 30 frames)
  - `amount` (`u_hold_amount`, default 0.0) — mix between live and held frame
- **GLSL Approach**: The CPU side manages this — every N frames, it copies the current output to a "held" texture. The shader just blends:
  ```glsl
  uniform sampler2D u_held_frame;
  fragColor = mix(col, texture(u_held_frame, v_uv), amount);
  ```
- **Note**: The frame counter logic (`if (frameCount % holdInterval == 0) copyToHeldTexture()`) lives in the C++ render code, not the shader.
- **Complexity**: Medium

### Transform Category — New Effects

#### R1: Tile Grid
- **Display Name**: Tile Grid (Resolume: "Tile")
- **Shader Key**: `tile_grid`
- **Category**: `warp`
- **Parameters**:
  - `columns` (`u_tile_cols`, default 0.0) — tile columns (1-8)
  - `rows` (`u_tile_rows`, default 0.0) — tile rows (1-8)
  - `skew` (`u_tile_skew`, default 0.5) — horizontal skew per row
  - `rotation` (`u_tile_rotation`, default 0.0) — per-tile rotation
- **GLSL Approach**:
  ```glsl
  float cols = floor(columns * 7.0) + 1.0;
  float rows_f = floor(rows * 7.0) + 1.0;
  vec2 uv = v_uv;
  uv.x += floor(uv.y * rows_f) * (skew - 0.5) * 0.3;
  uv = fract(uv * vec2(cols, rows_f));
  // Optional per-tile rotation
  uv = rotateUV(uv - 0.5, rotation * sin(floor(v_uv.x * cols) + floor(v_uv.y * rows_f))) + 0.5;
  ```
- **Complexity**: Small

#### R2: Slide Wrap
- **Display Name**: Slide Wrap (Resolume: "Slide")
- **Shader Key**: `slide_wrap`
- **Category**: `warp`
- **Parameters**:
  - `x` (`u_slide_x`, default 0.5) — horizontal slide (0.5 = center)
  - `y` (`u_slide_y`, default 0.5) — vertical slide (0.5 = center)
- **GLSL Approach**:
  ```glsl
  vec2 uv = fract(v_uv + vec2(x - 0.5, y - 0.5));
  ```
- **Complexity**: Small

#### R3: Fractal Iterate
- **Display Name**: Fractal Iterate (Resolume: "Iterate")
- **Shader Key**: `fractal_iterate`
- **Category**: `composite`
- **Parameters**:
  - `copies` (`u_iterate_copies`, default 0.3) — iteration count (2-8)
  - `scale` (`u_iterate_scale`, default 0.5) — per-iteration scale
  - `offset_x` (`u_iterate_ox`, default 0.5) — per-iteration X offset
  - `offset_y` (`u_iterate_oy`, default 0.5) — per-iteration Y offset
  - `rotation` (`u_iterate_rotation`, default 0.0) — per-iteration rotation
  - `opacity_decay` (`u_iterate_decay`, default 0.5) — opacity reduction per copy
- **GLSL Approach**:
  ```glsl
  int n = int(copies * 6.0) + 2;
  vec4 result = vec4(0.0);
  vec2 uv = v_uv;
  float alpha = 1.0;
  for (int i = 0; i < n; i++) {
      vec4 sample = texture(u_texture, uv) * alpha;
      result = result + sample * (1.0 - result.a);
      // Transform for next iteration
      uv = (uv - 0.5) * (1.0 + (scale - 0.5) * 0.3);
      uv += vec2(offset_x - 0.5, offset_y - 0.5) * 0.1;
      uv = rotateUV(uv, (rotation - 0.5) * 0.3);
      uv += 0.5;
      alpha *= opacity_decay;
  }
  ```
- **Complexity**: Medium

### 3D / Depth Category — New Effects

#### D1: Luminance Terrain
- **Display Name**: Luminance Terrain (Resolume: "Luma Waves")
- **Shader Key**: `luma_terrain`
- **Category**: `3d_depth`
- **Parameters**:
  - `height` (`u_lumaterrain_height`, default 0.0) — extrusion height
  - `segments` (`u_lumaterrain_segments`, default 0.5) — strip count
  - `angle` (`u_lumaterrain_angle`, default 0.3) — viewing angle / tilt
- **GLSL Approach**: Render image as horizontal scan lines offset vertically by luminance:
  ```glsl
  float numLines = floor(segments * 50.0) + 5.0;
  float lineY = floor(v_uv.y * numLines) / numLines;
  vec4 lineColor = texture(u_texture, vec2(v_uv.x, lineY));
  float luma = dot(lineColor.rgb, vec3(0.299, 0.587, 0.114));
  float offsetY = luma * height * 0.3;
  float inLine = abs(v_uv.y - lineY - offsetY) < (0.5 / numLines) ? 1.0 : 0.0;
  fragColor = lineColor * inLine;
  ```
- **Complexity**: Medium

#### D2: Voxel Matrix
- **Display Name**: Voxel Matrix (Resolume: "Pixels In Space")
- **Shader Key**: `voxel_matrix`
- **Category**: `3d_depth`
- **Parameters**:
  - `size` (`u_voxel_size`, default 0.3) — cube/pixel size
  - `height` (`u_voxel_height`, default 0.0) — extrusion by brightness
  - `rotation` (`u_voxel_rotation`, default 0.0) — 3D rotation
- **GLSL Approach**: Divide image into blocks, render each as a raised square whose height = luminance:
  ```glsl
  float blockSize = (size * 0.05) + 0.005;
  vec2 blockPos = floor(v_uv / blockSize) * blockSize;
  vec2 inBlock = (v_uv - blockPos) / blockSize; // 0-1 within block
  vec4 blockColor = texture(u_texture, blockPos + blockSize * 0.5);
  float luma = dot(blockColor.rgb, vec3(0.299, 0.587, 0.114));
  // Fake 3D: shift based on height, darken sides
  float raised = luma * height;
  float shade = 1.0 - raised * 0.5 * (1.0 - inBlock.y);
  float border = step(0.05, inBlock.x) * step(inBlock.x, 0.95) *
                 step(0.05, inBlock.y) * step(inBlock.y, 0.95);
  fragColor = blockColor * shade * border;
  ```
- **Complexity**: Medium

#### D3: Dot Field
- **Display Name**: Dot Field (Resolume: "Point Grid")
- **Shader Key**: `dot_field`
- **Category**: `3d_depth`
- **Parameters**:
  - `size` (`u_dotfield_size`, default 0.3) — dot size
  - `spacing` (`u_dotfield_spacing`, default 0.5) — grid spacing
  - `depth` (`u_dotfield_depth`, default 0.0) — 3D depth effect
- **GLSL Approach**: Render image as grid of circles whose size = luminance:
  ```glsl
  float gridSize = (spacing * 0.04) + 0.01;
  vec2 gridPos = floor(v_uv / gridSize) * gridSize + gridSize * 0.5;
  vec4 gridColor = texture(u_texture, gridPos);
  float luma = dot(gridColor.rgb, vec3(0.299, 0.587, 0.114));
  float dist = distance(v_uv, gridPos);
  float dotRadius = luma * size * gridSize * 0.6;
  float dot = smoothstep(dotRadius, dotRadius - 0.001, dist);
  fragColor = gridColor * dot;
  ```
- **Complexity**: Small

### Special Category — New Effects

#### J1: Monitor Wall
- **Display Name**: Monitor Wall (Resolume: "Videowall")
- **Shader Key**: `monitor_wall`
- **Category**: `pattern`
- **Parameters**:
  - `columns` (`u_monwall_cols`, default 0.3) — monitor columns (2-8)
  - `rows` (`u_monwall_rows`, default 0.3) — monitor rows (2-6)
  - `border` (`u_monwall_border`, default 0.3) — border/bezel thickness
  - `glow` (`u_monwall_glow`, default 0.3) — screen glow amount
- **GLSL Approach**: Tile the image into a grid with dark bezels between tiles:
  ```glsl
  float cols = floor(columns * 6.0) + 2.0;
  float rows_f = floor(rows * 4.0) + 2.0;
  vec2 cell = fract(v_uv * vec2(cols, rows_f));
  float borderW = border * 0.1;
  float mask = step(borderW, cell.x) * step(cell.x, 1.0 - borderW) *
               step(borderW, cell.y) * step(cell.y, 1.0 - borderW);
  vec2 uv = v_uv; // Each tile shows the full image
  vec4 content = texture(u_texture, uv);
  // Add screen glow at edges
  float edge = 1.0 - smoothstep(0.0, borderW * 2.0, min(min(cell.x, 1.0 - cell.x), min(cell.y, 1.0 - cell.y)));
  content.rgb += content.rgb * edge * glow;
  fragColor = content * mask;
  ```
- **Complexity**: Small

#### J2: Triangulate
- **Display Name**: Triangulate
- **Shader Key**: `triangulate`
- **Category**: `pattern`
- **Resolume Equivalent**: Triangulate
- **Parameters**:
  - `size` (`u_tri_size`, default 0.3) — triangle size
  - `amount` (`u_tri_amount`, default 0.0) — effect intensity
- **GLSL Approach**: Divide into triangular grid, sample center of each triangle:
  ```glsl
  float s = (size * 0.08) + 0.01;
  vec2 pos = v_uv / s;
  // Triangular grid
  float row = floor(pos.y);
  float col = floor(pos.x - mod(row, 2.0) * 0.5);
  vec2 center = vec2(col + mod(row, 2.0) * 0.5 + 0.5, row + 0.5) * s;
  vec4 triColor = texture(u_texture, center);
  fragColor = mix(texture(u_texture, v_uv), triColor, amount);
  ```
- **Complexity**: Small

#### J3: Drop Shadow
- **Display Name**: Drop Shadow
- **Shader Key**: `drop_shadow`
- **Category**: `blur`
- **Resolume Equivalent**: Drop Shadow
- **Parameters**:
  - `offset_x` (`u_shadow_ox`, default 0.6) — shadow X offset
  - `offset_y` (`u_shadow_oy`, default 0.4) — shadow Y offset
  - `blur` (`u_shadow_blur`, default 0.3) — shadow softness
  - `opacity` (`u_shadow_opacity`, default 0.0) — shadow opacity
- **GLSL Approach**:
  ```glsl
  vec2 shadowOffset = (vec2(offset_x, offset_y) - 0.5) * 0.1;
  vec4 shadowSample = texture(u_texture, v_uv - shadowOffset);
  float shadowAlpha = dot(shadowSample.rgb, vec3(0.299, 0.587, 0.114));
  // Blur the shadow (simplified box blur)
  float blurSize = blur * 0.02;
  float shadow = 0.0;
  for (int x = -2; x <= 2; x++)
      for (int y = -2; y <= 2; y++)
          shadow += dot(texture(u_texture, v_uv - shadowOffset + vec2(float(x), float(y)) * blurSize).rgb, vec3(0.333));
  shadow /= 25.0;
  vec4 result = col;
  result.rgb = mix(result.rgb, vec3(0.0), shadow * opacity * (1.0 - col.a));
  fragColor = result;
  ```
- **Note**: Works best on content with transparency (alpha channel).
- **Complexity**: Medium

#### J4: Auto Mask
- **Display Name**: Auto Mask
- **Shader Key**: `auto_mask`
- **Category**: `color`
- **Resolume Equivalent**: Auto Mask
- **Parameters**:
  - `threshold` (`u_automask_threshold`, default 0.5) — luminance cutoff
  - `softness` (`u_automask_softness`, default 0.3) — edge softness
  - `invert` (`u_automask_invert`, default 0.0) — 0 = bright visible, 1 = dark visible
- **GLSL Approach**:
  ```glsl
  float luma = dot(col.rgb, vec3(0.2126, 0.7152, 0.0722));
  float mask = smoothstep(threshold - softness * 0.5, threshold + softness * 0.5, luma);
  if (invert > 0.5) mask = 1.0 - mask;
  fragColor = vec4(col.rgb, col.a * mask);
  ```
- **Complexity**: Small

#### J5: Chroma Key (Effect version)
- **Display Name**: Chroma Key
- **Shader Key**: `chromakey_effect`
- **Category**: `color`
- **Note**: We already have `transparencyChromaKey` as a keying shader. This promotes it to a user-facing effect.
- **Parameters**:
  - `hue` (`u_chromakey_hue`, default 0.33) — target hue (0.33 = green)
  - `tolerance` (`u_chromakey_tolerance`, default 0.3) — hue range to key
  - `softness` (`u_chromakey_softness`, default 0.3) — edge softness
  - `amount` (`u_chromakey_amount`, default 0.0) — effect strength
- **GLSL Approach**: Already implemented in `transparencyChromaKey`. Extract shader code and add `amount` mix parameter.
- **Complexity**: Small (mostly re-registration)

---

## 5. SOURCES: COMPLETE RESOLUME-TO-AUDIODNA MAPPING

### Resolume Sources → Audio-DNA Status

| # | Resolume Source | Audio-DNA Equivalent | Status |
|---|----------------|---------------------|--------|
| 1 | Solid Color | — | NEW (Source #S1) |
| 2 | Gradient | **Color Gradient** | HAVE |
| 3 | Stroboscope | — | NEW (Source #S2) |
| 4 | Text Animator | — | NEW (Source #S3) |
| 5 | Text Block | — | SKIP (Text Animator covers this) |
| 6 | Shaper | — | NEW (Source #S4) |
| 7 | Checkered | — | NEW (Source #S5) |
| 8 | Lines | — | NEW (Source #S6) |
| 9 | Rings | — | NEW (Source #S7) |
| 10 | Sine Wave | — | NEW (Source #S8) |
| 11 | Spiral | — | NEW (Source #S9) |
| 12 | Metaballs | — | NEW (Source #S10) |
| 13 | Line Scape | — | NEW (Source #S11) |
| 14 | Test Card | — | SKIP (utility, not creative) |
| 15 | Video Router | — | NEW (Source #S12 — INFRA) |
| 16 | Abstract Field | **Perlin Noise** + **Plasma** | HAVE (our noise sources cover this) |
| 17 | Tunnelines | **Geometric Tunnel** | HAVE |
| 18 | Slice Outline | — | SKIP (projection mapping utility) |

---

## 6. NEW SOURCES TO IMPLEMENT (12 total)

### S1: Solid Color
- **Display Name**: Solid Color
- **Source ID**: `solid_color`
- **Shader Key**: `source_solid_color`
- **Category**: Utility
- **Parameters**:
  - `red` (`u_src_red`, default 1.0) — red channel
  - `green` (`u_src_green`, default 1.0) — green channel
  - `blue` (`u_src_blue`, default 1.0) — blue channel
- **GLSL**: `fragColor = vec4(red, green, blue, 1.0);`
- **Complexity**: Trivial
- **Audio Reactive**: Color channels routable to audio signals. E.g., route bass to red for pulsing color.

### S2: Strobe Light
- **Display Name**: Strobe Light (Resolume: "Stroboscope")
- **Source ID**: `strobe_light`
- **Shader Key**: `source_strobe_light`
- **Category**: Animation
- **Parameters**:
  - `frequency` (`u_src_frequency`, default 0.5) — flash rate (maps to 1-30 Hz)
  - `fade` (`u_src_fade`, default 0.3) — fade-out smoothness
  - `color1_r/g/b` (`u_src_c1r/g/b`, default 1.0/1.0/1.0) — flash color
  - `color2_r/g/b` (`u_src_c2r/g/b`, default 0.0/0.0/0.0) — background color
- **GLSL**: Step function with optional smooth fade between two colors, synced to `u_beatPhase`.
- **Complexity**: Small

### S3: Text Animator
- **Display Name**: Text Animator
- **Source ID**: `text_animator`
- **Category**: Text
- **Note**: This is the ONLY source that cannot be a pure GLSL shader — it requires text rasterization on the CPU side.
- **Implementation**: Render text to a texture using JUCE's `Graphics::drawText()` into an `Image`, upload to GL texture, then apply animation in a fragment shader.
- **Parameters**:
  - `text` (string, default "AUDIO-DNA") — text content
  - `font_size` (`u_src_font_size`, default 0.5) — size
  - `color_r/g/b` (`u_src_text_r/g/b`, default 1.0/1.0/1.0) — text color
  - `animation` (`u_src_text_anim`, default 0.0) — animation type (0-0.25 = scroll, 0.25-0.5 = pulse, 0.5-0.75 = wave, 0.75-1.0 = typewriter)
  - `speed` (`u_src_text_speed`, default 0.5) — animation speed
- **Architecture**: New class `TextSource` extending `ProceduralSource` that:
  1. Rasterizes text to `juce::Image` when text/font changes
  2. Uploads to GL texture
  3. Fragment shader applies animation (UV offset for scroll, scale pulse, per-character wave)
- **Complexity**: Large (requires CPU-GPU text pipeline)

### S4: Shape Generator
- **Display Name**: Shape Generator (Resolume: "Shaper")
- **Source ID**: `shape_generator`
- **Shader Key**: `source_shape_generator`
- **Category**: Geometric
- **Parameters**:
  - `shape` (`u_src_shape`, default 0.0) — shape type (0-0.14 = circle, 0.14-0.28 = square, 0.28-0.42 = triangle, 0.42-0.56 = hexagon, 0.56-0.7 = star, 0.7-0.84 = ring, 0.84-1.0 = cross)
  - `size` (`u_src_size`, default 0.5) — shape size
  - `rotation` (`u_src_rotation`, default 0.0) — rotation angle
  - `outline` (`u_src_outline`, default 0.0) — 0 = filled, 1 = outline only
  - `color_shift` (`u_src_color_shift`, default 0.0) — hue offset
- **GLSL**: Use SDF (signed distance fields) for each shape type:
  ```glsl
  float d;
  if (shape < 0.14) d = sdCircle(p, size);
  else if (shape < 0.28) d = sdBox(p, vec2(size));
  // ... etc
  float fill = outline > 0.5 ? abs(d) - 0.02 : d;
  float mask = smoothstep(0.01, -0.01, fill);
  ```
- **Complexity**: Medium (needs ~7 SDF implementations)

### S5: Checkerboard
- **Display Name**: Checkerboard (Resolume: "Checkered")
- **Source ID**: `checkerboard`
- **Shader Key**: `source_checkerboard`
- **Category**: Pattern
- **Parameters**:
  - `columns` (`u_src_columns`, default 0.3) — grid X (2-16)
  - `rows` (`u_src_rows`, default 0.3) — grid Y (2-16)
  - `color1_hue` (`u_src_c1_hue`, default 0.0) — first color hue
  - `color2_hue` (`u_src_c2_hue`, default 0.0) — second color hue (0 = black)
- **GLSL**:
  ```glsl
  float cols = floor(columns * 14.0) + 2.0;
  float rows_f = floor(rows * 14.0) + 2.0;
  float check = mod(floor(v_uv.x * cols) + floor(v_uv.y * rows_f), 2.0);
  ```
- **Complexity**: Small

### S6: Line Pattern
- **Display Name**: Line Pattern (Resolume: "Lines")
- **Source ID**: `line_pattern`
- **Shader Key**: `source_line_pattern`
- **Category**: Pattern
- **Parameters**:
  - `count` (`u_src_count`, default 0.3) — number of lines (4-40)
  - `width` (`u_src_width`, default 0.3) — line thickness
  - `rotation` (`u_src_rotation`, default 0.0) — line angle
  - `speed` (`u_src_speed`, default 0.2) — scroll speed
  - `color_shift` (`u_src_color_shift`, default 0.0) — hue offset
- **GLSL**: Rotate UV, compute distance to nearest line using `fract()`:
  ```glsl
  vec2 uv = rotateUV(v_uv - 0.5, rotation * 3.14159) + 0.5;
  uv.y += u_time * speed;
  float lines = floor(count * 36.0) + 4.0;
  float line = abs(fract(uv.y * lines) - 0.5);
  float mask = smoothstep(width * 0.5, width * 0.5 - 0.02, line);
  ```
- **Complexity**: Small

### S7: Concentric Rings
- **Display Name**: Concentric Rings (Resolume: "Rings")
- **Source ID**: `concentric_rings`
- **Shader Key**: `source_concentric_rings`
- **Category**: Pattern
- **Parameters**:
  - `count` (`u_src_count`, default 0.3) — number of rings (3-20)
  - `spacing` (`u_src_spacing`, default 0.5) — ring spacing
  - `width` (`u_src_width`, default 0.3) — ring thickness
  - `rotation` (`u_src_rotation`, default 0.0) — rotation speed
  - `color_shift` (`u_src_color_shift`, default 0.0) — hue offset
- **GLSL**: Distance from center, modulo for rings:
  ```glsl
  float dist = distance(v_uv, vec2(0.5));
  float rings = floor(count * 17.0) + 3.0;
  float ring = abs(fract(dist * rings + u_time * rotation) - 0.5);
  float mask = smoothstep(width * 0.5, width * 0.5 - 0.02, ring);
  ```
- **Complexity**: Small

### S8: Sine Oscillator
- **Display Name**: Sine Oscillator (Resolume: "Sine Wave")
- **Source ID**: `sine_oscillator`
- **Shader Key**: `source_sine_oscillator`
- **Category**: Pattern
- **Parameters**:
  - `waves` (`u_src_waves`, default 0.3) — wave count (1-10)
  - `frequency` (`u_src_frequency`, default 0.5) — oscillation frequency
  - `amplitude` (`u_src_amplitude`, default 0.5) — wave height
  - `modulation` (`u_src_modulation`, default 0.0) — frequency modulation depth
  - `thickness` (`u_src_thickness`, default 0.3) — line thickness
  - `color_shift` (`u_src_color_shift`, default 0.0) — hue offset
- **GLSL**: Multiple sine waves with FM modulation:
  ```glsl
  float n = floor(waves * 9.0) + 1.0;
  float mask = 0.0;
  for (float i = 0.0; i < n; i++) {
      float phase = v_uv.x * (frequency * 10.0 + 1.0) + u_time * 2.0 + i * 0.5;
      float mod_sig = sin(phase * modulation * 5.0) * 0.3;
      float wave = sin(phase + mod_sig) * amplitude * 0.3;
      float y = 0.5 + wave + (i - n * 0.5) * 0.15;
      float dist = abs(v_uv.y - y);
      mask += smoothstep(thickness * 0.05, 0.0, dist);
  }
  ```
- **Complexity**: Medium

### S9: Spiral Pattern
- **Display Name**: Spiral Pattern (Resolume: "Spiral")
- **Source ID**: `spiral_pattern`
- **Shader Key**: `source_spiral_pattern`
- **Category**: Pattern
- **Parameters**:
  - `arms` (`u_src_arms`, default 0.3) — spiral arm count (1-8)
  - `zoom` (`u_src_zoom`, default 0.5) — spiral tightness
  - `speed` (`u_src_speed`, default 0.3) — rotation speed
  - `distortion` (`u_src_distortion`, default 0.0) — arm wobble
  - `color_shift` (`u_src_color_shift`, default 0.0) — hue offset
- **GLSL**: Polar coordinates with log spiral:
  ```glsl
  vec2 p = v_uv - 0.5;
  float angle = atan(p.y, p.x);
  float radius = length(p);
  float n = floor(arms * 7.0) + 1.0;
  float spiral = fract((angle / 6.28318 * n + log(radius + 0.001) * zoom * 5.0 + u_time * speed));
  spiral += sin(radius * 20.0) * distortion * 0.2;
  ```
- **Complexity**: Small

### S10: Metaballs
- **Display Name**: Metaballs
- **Source ID**: `metaballs`
- **Shader Key**: `source_metaballs`
- **Category**: Organic
- **Parameters**:
  - `count` (`u_src_count`, default 0.3) — blob count (3-10)
  - `size` (`u_src_size`, default 0.4) — blob radius
  - `speed` (`u_src_speed`, default 0.3) — animation speed
  - `blend` (`u_src_blend`, default 0.5) — merge threshold (lower = more separate, higher = more merged)
  - `color_shift` (`u_src_color_shift`, default 0.0) — hue offset
- **GLSL**: Sum inverse-square distances from animated points:
  ```glsl
  int n = int(count * 7.0) + 3;
  float field = 0.0;
  for (int i = 0; i < n; i++) {
      float fi = float(i);
      vec2 pos = vec2(
          0.5 + 0.3 * sin(u_time * speed + fi * 1.7),
          0.5 + 0.3 * cos(u_time * speed * 0.8 + fi * 2.3)
      );
      float dist = distance(v_uv, pos);
      field += (size * 0.1) / (dist * dist + 0.001);
  }
  float mask = smoothstep(blend * 5.0 + 1.0, blend * 5.0 + 1.5, field);
  ```
- **Complexity**: Small

### S11: Terrain Lines
- **Display Name**: Terrain Lines (Resolume: "Line Scape")
- **Source ID**: `terrain_lines`
- **Shader Key**: `source_terrain_lines`
- **Category**: Pattern
- **Parameters**:
  - `height` (`u_src_height`, default 0.4) — wave amplitude
  - `lines` (`u_src_lines`, default 0.4) — number of lines (5-40)
  - `jagginess` (`u_src_jagginess`, default 0.3) — noise roughness
  - `speed` (`u_src_speed`, default 0.2) — scroll speed
  - `tilt` (`u_src_tilt`, default 0.3) — perspective tilt
  - `color_shift` (`u_src_color_shift`, default 0.0) — hue offset
- **GLSL**: Horizontal lines displaced vertically by noise:
  ```glsl
  float numLines = floor(lines * 35.0) + 5.0;
  float mask = 0.0;
  for (float i = 0.0; i < numLines; i++) {
      float baseY = i / numLines;
      float perspective = 1.0 + (baseY - 0.5) * tilt;
      float noiseVal = snoise(vec2(v_uv.x * (jagginess * 10.0 + 1.0), i * 0.3 + u_time * speed));
      float lineY = baseY + noiseVal * height * 0.15 * perspective;
      float dist = abs(v_uv.y - lineY);
      mask += smoothstep(0.003, 0.0, dist);
  }
  ```
- **Complexity**: Medium

### S12: Layer Router (INFRA)
- **Display Name**: Layer Router (Resolume: "Video Router / Feedback")
- **Source ID**: `layer_router`
- **Category**: Routing
- **Note**: This is NOT a GLSL shader source — it's an infrastructure component that routes one layer's rendered output as the input texture for another layer.
- **Implementation**:
  - Add a `RouterSource` class that extends `ProceduralSource`
  - `RouterSource` holds a reference to a source layer index
  - During rendering, the compositor saves each layer's output texture
  - `RouterSource::getTexture()` returns the saved texture from the source layer
  - UI: In the Sources panel or clip inspector, a "Layer Router" source with a dropdown to select the source layer
  - Enables feedback loops when a higher layer routes from a lower layer
  - **Safety**: Prevent self-referencing (layer routing to itself) and infinite loops
- **Files to create**: `src/sources/RouterSource.h/cpp`
- **Files to modify**: `Renderer.cpp` (save per-layer output textures), `CompositorEngine.cpp` (provide textures to router), `SourceRegistry.cpp`
- **Complexity**: Large (architectural change)

---

## 7. IMPLEMENTATION ORDER & PHASES

### Phase A: Infrastructure (Must be first)
1. Per-effect dry/wet mix (Section 2.1)
2. Temporal buffer system — ping-pong FBO approach (Section 2.2, simplified)
3. LUT loading system (Section 2.3)

### Phase B: Quick Wins — Small Effects (20 effects, all Small complexity)
- C1: Greyscale
- C2: Threshold
- C3: Exposure
- C4: Vibrance
- W2: Quad Mirror
- W3: Flip
- W6: Warp Field
- B2: Sharpen
- G3: Pixel Explosion
- G4: Color Flash
- R1: Tile Grid
- R2: Slide Wrap
- D3: Dot Field
- J2: Triangulate
- J4: Auto Mask
- J5: Chroma Key (promote existing shader)
- J1: Monitor Wall
- S1: Solid Color (source)
- S2: Strobe Light (source)
- S5: Checkerboard (source)

### Phase C: Medium Effects (12 effects, all Medium complexity)
- C5: Palette Remap
- C6: Color Grade (LUT)
- W1: Bendoscope
- W4: UV Remap
- W5: Liquid Morph
- B1: Edge Blur
- S1: Brush Strokes
- G1: Fragment Burst
- G5: Signal Destroy
- X1: Line Cloner
- X2: Radial Cloner
- X3: Cube Scatter

### Phase D: Time-Based Effects (5 effects, require temporal infrastructure)
- T3: Ghost Trails
- T4: Infinite Feedback
- T5: Frame Hold
- T1: Frame Delay (requires full FrameHistory ring buffer)
- T2: Channel Delay (requires full FrameHistory ring buffer)

### Phase E: Remaining Sources (10 sources)
- S6: Line Pattern
- S7: Concentric Rings
- S8: Sine Oscillator
- S9: Spiral Pattern
- S10: Metaballs
- S11: Terrain Lines
- S4: Shape Generator
- S12: Layer Router (infrastructure)
- S3: Text Animator (largest source, requires CPU text pipeline)

### Phase F: Complex Effects & Polish
- R3: Fractal Iterate
- D1: Luminance Terrain
- D2: Voxel Matrix
- G2: Time Freeze
- J3: Drop Shadow

---

## 8. SHADER ARCHITECTURE NOTES

### Uniform Naming Convention
All new effects follow the existing convention:
- Effect uniforms: `u_[shaderKey]_[paramName]` (e.g., `u_greyscale_amount`)
- Source uniforms: `u_src_[paramName]` (e.g., `u_src_frequency`)
- Global uniforms: `u_time`, `u_resolution`, `u_rms`, `u_beatPhase`

### Parameter Range
All parameters are `[0.0, 1.0]` as per CLAUDE.md Rule 6. The shader maps to internal ranges.

### Noise Functions
Multiple new effects need noise functions. Add shared noise utility code to `EmbeddedShaders.h`:
```glsl
// Include at the top of any shader needing noise
float hash(vec2 p) { return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453); }
float hash(float n) { return fract(sin(n) * 43758.5453); }
// 2D simplex noise (snoise), 3D simplex noise, fbm
```
Consider a `#define NOISE_FUNCTIONS` block that gets prepended to shaders that need it.

### Rotate UV Helper
Many effects need UV rotation. Add shared helper:
```glsl
vec2 rotateUV(vec2 uv, float angle) {
    float c = cos(angle), s = sin(angle);
    return mat2(c, -s, s, c) * uv;
}
```

### Hue Rotate Helper
Feedback and palette effects need hue rotation:
```glsl
vec3 hueRotate(vec3 col, float angle) {
    float c = cos(angle * 6.28318), s = sin(angle * 6.28318);
    mat3 m = mat3(
        0.299+0.701*c+0.168*s, 0.587-0.587*c+0.330*s, 0.114-0.114*c-0.497*s,
        0.299-0.299*c-0.328*s, 0.587+0.413*c+0.035*s, 0.114-0.114*c+0.292*s,
        0.299-0.299*c+1.250*s, 0.587-0.587*c-1.050*s, 0.114+0.886*c-0.203*s);
    return m * col;
}
```

---

## 9. REGISTRATION & FILE CONVENTIONS

### Adding a New Effect

1. **Shader**: Add the fragment shader string to `EmbeddedShaders.h` following the existing pattern:
   ```cpp
   static const char* shaderKey_frag = R"(
   #version 410 core
   // ... shader code ...
   )";
   ```

2. **Registration**: Add to `EffectLibrary.cpp` in `registerBuiltInEffects()`:
   ```cpp
   registerEffect("Display Name", "category", "shaderKey", {
       {"param_name", "u_uniform_name", defaultValue},
       // ...
   });
   ```

3. **Compilation**: Add to `Renderer::compileEffectShaders()` in `Renderer.cpp`:
   ```cpp
   compileShader("shaderKey", EmbeddedShaders::passthrough_vert, EmbeddedShaders::shaderKey_frag);
   ```

### Adding a New Source

1. **Shader**: Add to `EmbeddedShaders.h` with prefix `source_`:
   ```cpp
   static const char* source_shaderKey_frag = R"(
   #version 410 core
   // ... source shader code (receives u_time, u_resolution, u_rms, etc.) ...
   )";
   ```

2. **Registration**: Add to `SourceRegistry.cpp` in `registerSources()`:
   ```cpp
   registerSource("Display Name", "source_id", "Category", "source_shaderKey", false, {
       {"Param Name", "u_src_param", defaultValue},
       // ...
   });
   ```
   The `false` = not stateful. Set `true` for sources that read their previous output (ping-pong).

3. **Compilation**: Add to `Renderer::compileSourceShaders()`:
   ```cpp
   compileShader("source_shaderKey", EmbeddedShaders::passthrough_vert, EmbeddedShaders::source_shaderKey_frag);
   ```

---

## SUMMARY

| Category | Existing | New | Total After |
|----------|----------|-----|------------|
| **Effects** | 76 | 32 | **108** |
| **Sources** | 10 | 12 | **22** |
| **Infrastructure** | — | 3 | 3 |
| **Grand Total** | 86 | 47 | **133** |

### Resolume Coverage After Implementation

| Resolume Feature | Coverage |
|-----------------|----------|
| All ~90 confirmed effects | **100%** — every effect is either matched, implemented, or explicitly skipped with justification |
| All 18 generator sources | **100%** — every generator is either matched or implemented |
| Per-effect dry/wet | **Yes** — infrastructure addition |
| LUT support | **Yes** — via Color Grade effect |
| Temporal effects (Delay, Trails, Feedback) | **Yes** — via FrameHistory infrastructure |
| Layer feedback routing | **Yes** — via Layer Router source |

### Our Unique Additions (not in Resolume)
- All 76 existing effects remain (many don't exist in Resolume: ASCII Art, Rain on Glass, Pencil Sketch, etc.)
- All 10 existing sources remain (Mandelbrot/Julia, Reaction-Diffusion, Cellular Automata, etc.)
- Deep audio reactivity on every parameter (42+ signals vs Resolume's L/M/H)
- Automatic BPM-synced animations on all sources

---

## 10. BEYOND RESOLUME — Original Effects & Sources for Discussion

These are effects and sources that **neither Resolume nor any mainstream VJ tool offers**. They exploit our unique advantage (42+ audio features, automatic BPM/downbeat, structural detection) and draw from shader art, demoscene, generative art, mathematics, and physics simulation. Organized by category with full implementation detail.

### ORIGINAL EFFECTS (25 ideas)

---

#### OE1: Spectral Smear
- **Category**: `time` (temporal)
- **Concept**: Each frequency band of the image (separated by luminance zones) moves at a different speed. Bright areas update instantly, mid-tones lag slightly, dark areas trail behind like a long-exposure photograph — but the exposure time is driven by the spectral centroid.
- **Parameters**:
  - `amount` (`u_specsmear_amount`, default 0.0) — smear intensity
  - `bands` (`u_specsmear_bands`, default 0.5) — number of luminance zones (2-8)
  - `speed` (`u_specsmear_speed`, default 0.5) — how fast each band catches up
- **Audio Link**: Spectral centroid controls which luminance zones lag most. High-pitched audio = bright pixels move fast. Bass-heavy = dark pixels move fast.
- **GLSL Approach**: Per-pixel temporal blending where the blend factor depends on the pixel's luminance. Uses previous frame.
- **Complexity**: Medium (needs previous frame)
- **Why it's unique**: No VJ tool links spectral content to temporal behavior per-luminance-zone.

#### OE2: Harmonic Displacement
- **Category**: `warp`
- **Concept**: Displacement driven by the 12-note chromagram. Each pitch class pushes pixels in a different angular direction (C = 0°, C# = 30°, D = 60°, ... B = 330°). When a chord plays, the image displaces in the directions of all active notes simultaneously, creating harmonic interference patterns.
- **Parameters**:
  - `amount` (`u_harmdisplace_amount`, default 0.0) — displacement strength
  - `smoothing` (`u_harmdisplace_smooth`, default 0.5) — temporal smoothing
  - `color_mode` (`u_harmdisplace_color`, default 0.0) — 0 = displace only, 0.5 = tint by note, 1.0 = full chroma coloring
- **Audio Link**: All 12 chromagram values directly drive 12 displacement vectors. Harmonic Change Detection triggers sharpness of the displacement.
- **GLSL Approach**: Sum 12 directional displacement vectors weighted by chromagram energy, apply as UV offset.
- **Why it's unique**: Directly translates musical harmony into visual displacement. A C major chord creates a completely different pattern than A minor. No tool does this.
- **Complexity**: Medium

#### OE3: Beat Ripple
- **Category**: `warp`
- **Concept**: Every detected onset drops a ripple into the image from a random location. Multiple ripples interfere with each other over time. The ripple frequency is tuned to the detected BPM so they reinforce on downbeats.
- **Parameters**:
  - `intensity` (`u_beatripple_intensity`, default 0.0) — ripple amplitude
  - `decay` (`u_beatripple_decay`, default 0.5) — how fast ripples fade
  - `count` (`u_beatripple_count`, default 0.3) — max simultaneous ripples (2-8)
- **Audio Link**: Onset detection triggers new ripples. Onset strength controls amplitude. Beat phase ensures ripples sync musically.
- **Requires**: Small state buffer (ring of 8 ripple positions + ages, updated via uniforms from CPU)
- **Complexity**: Medium

#### OE4: Timbral Mosaic
- **Category**: `pattern`
- **Concept**: Divides the image into a mosaic where each tile's visual treatment depends on the MFCC coefficients. MFCC[0] (overall energy) controls tile size. MFCC[1] (spectral balance) controls tile shape. Higher MFCCs control color shift, rotation, and border style. The mosaic literally sounds different — when the timbre changes, the mosaic changes.
- **Parameters**:
  - `amount` (`u_timbremosaic_amount`, default 0.0) — effect intensity
  - `base_size` (`u_timbremosaic_size`, default 0.5) — base tile size
  - `complexity` (`u_timbremosaic_complexity`, default 0.5) — how many MFCCs influence the mosaic
- **Audio Link**: All 13 MFCCs drive different mosaic properties. This is the only effect in any VJ tool that uses timbral fingerprint.
- **GLSL Approach**: Voronoi-like tiling where cell properties are modulated by MFCC uniforms.
- **Complexity**: Medium
- **Why it's unique**: Exploits our exclusive MFCC analysis. Impossible in Resolume.

#### OE5: Structural Morph
- **Category**: `warp`
- **Concept**: The image smoothly morphs between 4 different distortion states based on the structural state detector: Normal (no distortion), Buildup (increasing swirl), Drop (explosion outward), Breakdown (slow drift inward). The transitions between states are automatic — the music controls the visual transformation.
- **Parameters**:
  - `intensity` (`u_structmorph_intensity`, default 0.0) — how dramatic the morphs are
  - `normal_style` (`u_structmorph_normal`, default 0.0) — distortion for normal sections
  - `drop_style` (`u_structmorph_drop`, default 0.5) — distortion for drops
- **Audio Link**: `structuralState` (0=normal, 1=buildup, 2=drop, 3=breakdown) + smooth interpolation between states. Energy state is the primary driver — no manual sync needed.
- **GLSL Approach**: Four UV distortion functions blended by a smooth state interpolation value.
- **Complexity**: Medium
- **Why it's unique**: Auto-adapts to song structure. Resolume cannot detect buildups/drops.

#### OE6: Pitch Chromatic Shift
- **Category**: `color`
- **Concept**: Shifts the image's hue based on the detected musical pitch. Maps the 12 pitch classes to the 12 hue positions on the color wheel. When the singer hits an A, the image shifts toward orange. When the bass drops to E, it shifts toward green. Key detection smooths the overall color palette.
- **Parameters**:
  - `amount` (`u_pitchcolor_amount`, default 0.0) — shift intensity
  - `mode` (`u_pitchcolor_mode`, default 0.0) — 0 = note-based (jumpy), 0.5 = key-based (smooth), 1.0 = chromagram-weighted (rich)
  - `saturation` (`u_pitchcolor_sat`, default 0.5) — color intensity
- **Audio Link**: `dominantPitch`, `detectedKey`, or `chromagram[12]` depending on mode.
- **Complexity**: Small
- **Why it's unique**: Musical key → visual color. Synesthesia made real.

#### OE7: Density Wave
- **Category**: `warp`
- **Concept**: The image compresses and expands in waves timed to the beat. Like a sound wave moving through the visual medium — areas of compression (squeezed pixels) and rarefaction (stretched pixels) propagate across the image at the speed of the BPM.
- **Parameters**:
  - `amount` (`u_densitywave_amount`, default 0.0) — wave amplitude
  - `direction` (`u_densitywave_dir`, default 0.0) — propagation direction
  - `wavelength` (`u_densitywave_wl`, default 0.5) — wave size
- **Audio Link**: Beat phase drives wave position. RMS controls amplitude. Direction can be linked to spectral centroid.
- **GLSL Approach**: Non-linear UV remapping where `uv.x = uv.x + sin(uv.x * freq + beatPhase * 6.28) * amount`.
- **Complexity**: Small

#### OE8: Rhythm Slice
- **Category**: `glitch`
- **Concept**: Slices the image into horizontal strips that shift left/right on the beat. Each strip syncs to a different beat division — top strip on whole notes, middle on half notes, bottom on quarter notes. Creates a cascading rhythmic visual that directly mirrors the metrical hierarchy.
- **Parameters**:
  - `amount` (`u_rhythmslice_amount`, default 0.0) — slice displacement
  - `slices` (`u_rhythmslice_count`, default 0.5) — number of slices (4-16)
  - `sync` (`u_rhythmslice_sync`, default 0.5) — 0 = random timing, 1 = strict beat sync
- **Audio Link**: `beatPhase`, `barPhase`, `phrasePhase` drive different slices. Beat-in-bar determines which slices are active.
- **Complexity**: Small

#### OE9: Key Palette
- **Category**: `color`
- **Concept**: Automatically selects a color palette based on the detected musical key. Major keys get warm, saturated palettes. Minor keys get cool, desaturated palettes. Key changes trigger smooth palette transitions. Based on music-color synesthesia research (Scriabin, Rimington).
- **Parameters**:
  - `amount` (`u_keypalette_amount`, default 0.0) — remap intensity
  - `brightness` (`u_keypalette_bright`, default 0.5) — palette brightness
  - `saturation` (`u_keypalette_sat`, default 0.5) — palette saturation
- **Audio Link**: `detectedKey` (0-11) + `keyIsMajor`. 24 predefined palettes (12 major, 12 minor).
- **Complexity**: Medium (needs 24 palette definitions)
- **Why it's unique**: Music theory → color theory. Automatic synesthesia.

#### OE10: Transient Flash
- **Category**: `animation`
- **Concept**: On every detected onset (drum hit, percussive transient), the image briefly flashes with a visual effect — white flash, color inversion, edge glow, or zoom punch — then instantly decays. The onset strength controls the flash intensity. Different from Strobe because it's tied to actual audio transients, not a fixed rate.
- **Parameters**:
  - `style` (`u_transflash_style`, default 0.0) — 0-0.25 = white flash, 0.25-0.5 = invert flash, 0.5-0.75 = edge glow flash, 0.75-1.0 = zoom punch
  - `intensity` (`u_transflash_intensity`, default 0.0) — flash strength
  - `decay` (`u_transflash_decay`, default 0.5) — how fast the flash fades
- **Audio Link**: `onsetDetected` triggers, `onsetStrength` scales intensity. Purely audio-reactive — no clock needed.
- **Complexity**: Small

#### OE11: Gravity Warp
- **Category**: `warp`
- **Concept**: Simulates gravitational lensing — pixels bend around invisible mass points. The mass of each attractor is driven by different frequency bands. Bass creates a heavy warp at the bottom, treble creates a light warp at the top. Creates a "space-time" distortion that mirrors the frequency content.
- **Parameters**:
  - `strength` (`u_gravity_strength`, default 0.0) — warp intensity
  - `attractors` (`u_gravity_count`, default 0.3) — number of gravity wells (2-7, one per band)
- **Audio Link**: 7 band energies (Sub through Air) each control one attractor's mass and vertical position.
- **Complexity**: Medium

#### OE12: Crystallize
- **Category**: `pattern`
- **Concept**: Breaks the image into irregular crystalline facets (Voronoi cells) that grow and shatter based on audio energy. Quiet passages → large crystals (calm). Loud passages → small shattered crystals (intense). Onsets crack existing crystals into smaller pieces.
- **Parameters**:
  - `amount` (`u_crystal_amount`, default 0.0) — effect intensity
  - `base_size` (`u_crystal_size`, default 0.5) — base crystal size
  - `edge_glow` (`u_crystal_edge`, default 0.3) — brightness of crystal edges
- **Audio Link**: RMS inversely controls cell count (loud = more cells = smaller crystals). Onset triggers crack animation.
- **Complexity**: Medium

#### OE13: Dimension Fold
- **Category**: `3d_depth`
- **Concept**: Folds the 2D image into 3D space along multiple axes, creating an origami-like effect. The fold angles are driven by different audio bands — bass folds the bottom, treble folds the top, mids fold the center.
- **Parameters**:
  - `folds` (`u_dimfold_folds`, default 0.3) — number of fold lines (2-6)
  - `amount` (`u_dimfold_amount`, default 0.0) — fold angle
  - `perspective` (`u_dimfold_persp`, default 0.5) — 3D perspective strength
- **Audio Link**: Per-fold angle driven by band energies. Creates a multi-dimensional response to the frequency spectrum.
- **Complexity**: Medium

#### OE14: Echo Cascade
- **Category**: `time`
- **Concept**: Creates a cascade of time-delayed copies of the image, each progressively smaller, rotated, and color-shifted — like seeing the image echoing through time and space simultaneously. Each echo is one beat behind the previous.
- **Parameters**:
  - `echoes` (`u_echocascade_count`, default 0.3) — number of echoes (2-8)
  - `decay` (`u_echocascade_decay`, default 0.5) — opacity reduction per echo
  - `zoom` (`u_echocascade_zoom`, default 0.5) — size reduction per echo
  - `rotation` (`u_echocascade_rot`, default 0.0) — rotation per echo
  - `hue_shift` (`u_echocascade_hue`, default 0.0) — hue shift per echo
- **Audio Link**: Beat phase determines echo timing. Each echo fades differently based on current RMS.
- **Requires**: FrameHistory (N frames of history for N echoes)
- **Complexity**: Medium

#### OE15: Waveform Carve
- **Category**: `warp`
- **Concept**: Carves the image using the actual audio waveform shape. The waveform acts as a displacement map that scrolls across the image. Loud bass creates large rolling displacements; quiet passages barely ripple the surface.
- **Parameters**:
  - `amount` (`u_wavecarve_amount`, default 0.0) — carve depth
  - `direction` (`u_wavecarve_dir`, default 0.0) — carve direction (horizontal/vertical/radial)
  - `width` (`u_wavecarve_width`, default 0.5) — waveform influence width
- **Audio Link**: Raw RMS/waveform shape drives displacement. Different from static sine waves — uses the actual music's amplitude contour.
- **Complexity**: Small

#### OE16: Chroma Dissolve
- **Category**: `color`
- **Concept**: Dissolves the image by removing colors in order of the chromagram. Whichever musical note is loudest, that hue dissolves first (becomes transparent). Creates a color-reactive reveal where the dominant musical pitch literally removes its corresponding color from the image.
- **Parameters**:
  - `amount` (`u_chromadiss_amount`, default 0.0) — dissolve intensity
  - `softness` (`u_chromadiss_softness`, default 0.5) — transition softness
- **Audio Link**: `chromagram[12]` — each pitch class energy maps to a hue range. Higher energy = more of that hue is dissolved.
- **Complexity**: Medium
- **Why it's unique**: Musical pitch directly controls which colors are visible. Impossible without chromagram analysis.

#### OE17: Phase Grid
- **Category**: `pattern`
- **Concept**: Overlays a grid where each cell pulses at a different beat subdivision. Cell (0,0) pulses on whole notes, cell (1,0) on half notes, cell (0,1) on quarter notes, etc. Creates a visual representation of the metrical hierarchy — you can SEE the rhythm structure.
- **Parameters**:
  - `amount` (`u_phasegrid_amount`, default 0.0) — grid overlay opacity
  - `grid_size` (`u_phasegrid_size`, default 0.5) — cell size
  - `style` (`u_phasegrid_style`, default 0.0) — 0 = brightness pulse, 0.5 = scale pulse, 1.0 = color cycle
- **Audio Link**: `beatPhase`, `barPhase`, `phrasePhase` drive different cells at different rates.
- **Complexity**: Small

#### OE18: Neural Glow
- **Category**: `blur`
- **Concept**: Selective glow that only blooms around areas of high visual contrast that coincide with high audio energy. When the bass hits, only the bass-colored areas (warm tones) glow. When hi-hats sizzle, bright/white areas glow. Creates an intelligent, audio-aware bloom.
- **Parameters**:
  - `amount` (`u_neuralglow_amount`, default 0.0) — glow intensity
  - `selectivity` (`u_neuralglow_select`, default 0.5) — how selective the glow is (0 = everything glows, 1 = only audio-matched areas)
  - `spread` (`u_neuralglow_spread`, default 0.5) — glow radius
- **Audio Link**: 7-band energy maps to 7 luminance/hue zones. Each zone glows proportionally to its corresponding band energy.
- **Complexity**: Medium

#### OE19: Horizon Scanner
- **Category**: `warp`
- **Concept**: A scanning line sweeps across the image at the BPM rate. Everything behind the scan line shows the current frame; everything ahead shows a frozen or time-delayed version. Creates a temporal divide that travels through the image, revealing the present and showing the past simultaneously.
- **Parameters**:
  - `direction` (`u_horizscan_dir`, default 0.0) — scan direction (0 = left→right, 0.25 = top→bottom, 0.5 = right→left, 0.75 = bottom→top)
  - `width` (`u_horizscan_width`, default 0.3) — transition zone width
  - `delay` (`u_horizscan_delay`, default 0.5) — how far in the past the "ahead" side shows
- **Audio Link**: Beat phase drives the scan position. Downbeat resets.
- **Requires**: Previous frame access
- **Complexity**: Medium

#### OE20: Topographic Lines
- **Category**: `pattern`
- **Concept**: Renders the image as a topographic map — iso-luminance contour lines drawn at regular brightness intervals, creating a terrain-map aesthetic. The contour density changes with transient density (busy passages = more contour lines = more detail).
- **Parameters**:
  - `amount` (`u_topo_amount`, default 0.0) — effect intensity (0 = original, 1 = pure contour lines)
  - `levels` (`u_topo_levels`, default 0.5) — number of contour levels (4-20)
  - `thickness` (`u_topo_thickness`, default 0.3) — line thickness
  - `color_mode` (`u_topo_color`, default 0.0) — 0 = white lines on dark, 0.5 = colored by height, 1.0 = overlay on image
- **Audio Link**: Transient density controls contour count. Spectral centroid shifts color mapping.
- **GLSL Approach**: `float contour = fract(luma * levels); float line = smoothstep(0.02, 0.0, abs(contour - 0.5) - 0.48);`
- **Complexity**: Small

#### OE21: Polar Explosion
- **Category**: `warp`
- **Concept**: Converts the image to polar coordinates, stretches it, then converts back — creating an explosion/implosion effect that radiates from the center. The conversion amount pulses with the beat, creating a rhythmic breathing effect.
- **Parameters**:
  - `amount` (`u_polarexplode_amount`, default 0.0) — polar distortion amount
  - `center_x` (`u_polarexplode_cx`, default 0.5)
  - `center_y` (`u_polarexplode_cy`, default 0.5)
  - `twist` (`u_polarexplode_twist`, default 0.0) — angular twist during conversion
- **Audio Link**: Beat phase drives amount oscillation. Onset strength boosts the explosion peak.
- **Complexity**: Small

#### OE22: Data Corruption
- **Category**: `glitch`
- **Concept**: Simulates JPEG/MPEG compression artifacts driven by audio. Introduces macroblocking, quantization noise, color banding, and DCT ringing that intensify with RMS. Quiet = clean image. Loud = increasingly corrupted data stream.
- **Parameters**:
  - `amount` (`u_datacorrupt_amount`, default 0.0) — corruption level
  - `block_size` (`u_datacorrupt_block`, default 0.5) — macroblock size
  - `color_damage` (`u_datacorrupt_color`, default 0.5) — chroma subsampling simulation
- **Audio Link**: RMS → overall corruption. Spectral flux → glitch rate. Onset → block displacement.
- **GLSL Approach**: Quantize UV to blocks, posterize within blocks, offset random blocks horizontally, reduce chroma resolution.
- **Complexity**: Medium

#### OE23: Ribbon Wrap
- **Category**: `3d_depth`
- **Concept**: Wraps the image around itself like a ribbon or Möbius strip. The wrap angle and twist are driven by audio, creating a constantly morphing dimensional fold.
- **Parameters**:
  - `wrap` (`u_ribbon_wrap`, default 0.0) — wrap amount (0 = flat, 1 = full loop)
  - `twist` (`u_ribbon_twist`, default 0.0) — Möbius twist amount
  - `perspective` (`u_ribbon_persp`, default 0.5) — 3D perspective depth
- **Audio Link**: Beat phase drives wrap oscillation. Spectral centroid controls twist.
- **Complexity**: Medium

#### OE24: Glitch Sort
- **Category**: `glitch`
- **Concept**: Pixel sorting — sorts pixels within rows by brightness, creating the iconic "data-bent" aesthetic. Sort direction and amount are driven by audio energy.
- **Parameters**:
  - `amount` (`u_glitchsort_amount`, default 0.0) — how much of each row is sorted
  - `threshold` (`u_glitchsort_threshold`, default 0.5) — brightness threshold for sorting
  - `direction` (`u_glitchsort_dir`, default 0.0) — 0 = horizontal, 0.5 = vertical, 1.0 = diagonal
- **Audio Link**: Spectral flux controls sort amount (more spectral change = more sorting). RMS sets the threshold.
- **GLSL Approach**: Approximate pixel sorting by shifting pixels toward their brightness-sorted position using a series of compare-and-swap operations (bitonic sort approximation in shader).
- **Complexity**: Large (pixel sorting in GLSL is non-trivial, but approximations work well)

#### OE25: Thermal Vision
- **Category**: `color`
- **Concept**: Enhanced thermal camera look that uses the actual audio frequency spectrum to color different luminance zones. Bass frequencies color the dark/warm areas, treble colors the bright/cool areas — creating a thermal map that literally represents the audio frequency distribution as visual heat.
- **Parameters**:
  - `amount` (`u_thermalvision_amount`, default 0.0) — effect intensity
  - `contrast` (`u_thermalvision_contrast`, default 0.5) — thermal contrast
- **Audio Link**: 7 band energies map to 7 color zones of the thermal palette. The palette itself shifts with the audio.
- **Complexity**: Small

---

### ORIGINAL SOURCES (25 ideas)

---

#### OS1: Cymatics
- **Category**: Audio-Visual
- **Concept**: Simulates Chladni figures — the standing wave patterns that form on vibrating surfaces (like sand on a speaker). The pattern is mathematically computed from the dominant pitch frequency. When the music changes pitch, the pattern reorganizes, just like real cymatics.
- **Parameters**:
  - `resonance` (`u_src_resonance`, default 0.5) — pattern sharpness
  - `damping` (`u_src_damping`, default 0.5) — how quickly patterns shift
  - `color_shift` (`u_src_color_shift`, default 0.0)
- **Audio Link**: `dominantPitch` drives the vibrational frequency. `pitchConfidence` controls pattern stability. `rms` controls brightness.
- **GLSL**: `float pattern = sin(freq * p.x) * sin(freq * p.y) + sin(freq * (p.x + p.y));` where freq derives from pitch.
- **Complexity**: Medium
- **Why it's unique**: Real physics-based audio visualization. The image IS the sound.

#### OS2: Spectral Waterfall
- **Category**: Audio-Visual
- **Concept**: Real-time spectrogram that scrolls vertically. Frequency on the X axis, time on the Y axis, color = amplitude. Unlike a standard spectrum analyzer, this creates a flowing river of color that shows the temporal evolution of the audio spectrum.
- **Parameters**:
  - `scroll_speed` (`u_src_scroll`, default 0.5) — waterfall scroll rate
  - `color_mode` (`u_src_color_mode`, default 0.0) — 0 = thermal, 0.5 = neon, 1.0 = monochrome
  - `log_scale` (`u_src_log_scale`, default 0.5) — frequency axis scaling (linear vs log)
- **Audio Link**: 7 band energies (or more granular FFT bins via additional uniforms) paint each column.
- **Requires**: Stateful (ping-pong FBO to scroll previous data downward)
- **Complexity**: Medium

#### OS3: Spectral Ring
- **Category**: Audio-Visual
- **Concept**: FFT spectrum displayed as a circular ring. Each frequency bin maps to an angle around the circle, with amplitude shown as distance from the ring center. Glowing, pulsing, and rotating with the beat.
- **Parameters**:
  - `radius` (`u_src_radius`, default 0.5) — ring radius
  - `thickness` (`u_src_thickness`, default 0.3) — bar thickness
  - `glow` (`u_src_glow`, default 0.5) — neon glow amount
  - `rotation` (`u_src_rotation`, default 0.0) — rotation speed
  - `color_shift` (`u_src_color_shift`, default 0.0)
- **Audio Link**: 7 band energies drive bar heights. Beat phase rotates the ring. Onset triggers pulse.
- **Complexity**: Small

#### OS4: Lissajous Sculpture
- **Category**: Mathematical
- **Concept**: Animated Lissajous curves where the frequency ratios are driven by the detected musical intervals. An octave (2:1) creates a figure-8. A fifth (3:2) creates a 3-lobe pattern. The harmonics of the actual music generate the geometry.
- **Parameters**:
  - `thickness` (`u_src_thickness`, default 0.3) — line thickness
  - `glow` (`u_src_glow`, default 0.5) — glow intensity
  - `trails` (`u_src_trails`, default 0.5) — trail persistence
  - `color_shift` (`u_src_color_shift`, default 0.0)
- **Audio Link**: `dominantPitch` and `chromagram` determine the X:Y frequency ratio. `rms` controls amplitude. `beatPhase` controls animation phase.
- **GLSL**: Parametric `x = sin(a*t + phase), y = sin(b*t)` with a/b derived from chromagram peaks.
- **Complexity**: Medium

#### OS5: Moiré Engine
- **Category**: Mathematical
- **Concept**: Two overlapping grids (lines, circles, or dots) at slightly different scales/rotations create moiré interference patterns. The offset between grids is driven by audio — bass shifts one grid, treble shifts the other. The resulting patterns are hypnotic and complexly reactive.
- **Parameters**:
  - `grid_type` (`u_src_grid_type`, default 0.0) — 0 = lines, 0.33 = circles, 0.66 = dots
  - `density` (`u_src_density`, default 0.5) — grid line density
  - `offset` (`u_src_offset`, default 0.0) — base offset between grids
  - `color_shift` (`u_src_color_shift`, default 0.0)
- **Audio Link**: Bass shifts grid A, treble shifts grid B. Beat phase rotates both.
- **Complexity**: Small

#### OS6: Superformula
- **Category**: Mathematical
- **Concept**: Johan Gielis's superformula generates an infinite family of shapes — circles, stars, flowers, organic forms — all controlled by 6 parameters. Route 6 audio features to the 6 superformula parameters and watch the shape morph continuously with the music.
- **Parameters**:
  - `m` (`u_src_m`, default 0.3) — symmetry (3 = triangle, 4 = square, 5 = pentagon, 6 = hex, etc.)
  - `n1` (`u_src_n1`, default 0.5) — roundness
  - `n2` (`u_src_n2`, default 0.5) — concavity
  - `n3` (`u_src_n3`, default 0.5) — blobbiness
  - `size` (`u_src_size`, default 0.5) — overall scale
  - `color_shift` (`u_src_color_shift`, default 0.0)
- **Audio Link**: Route bands to m/n1/n2/n3 for shape morphing. RMS → size. Beat phase → rotation.
- **GLSL**: `float r = pow(pow(abs(cos(m*theta/4.0)/a), n2) + pow(abs(sin(m*theta/4.0)/b), n3), -1.0/n1);`
- **Complexity**: Small
- **Why it's unique**: One formula, infinite shapes, all music-driven.

#### OS7: Sacred Geometry
- **Category**: Mathematical
- **Concept**: Generates sacred geometry patterns — Flower of Life, Seed of Life, Metatron's Cube, Sri Yantra — from intersecting circles. The number of circles and their radii are driven by audio harmonics, creating patterns that literally grow and breathe with the music.
- **Parameters**:
  - `pattern` (`u_src_pattern`, default 0.0) — 0 = Flower of Life, 0.33 = Seed of Life, 0.66 = Metatron's Cube
  - `layers` (`u_src_layers`, default 0.5) — complexity / number of generations
  - `glow` (`u_src_glow`, default 0.5) — line glow
  - `rotation` (`u_src_rotation`, default 0.0) — rotation speed
  - `color_shift` (`u_src_color_shift`, default 0.0)
- **Audio Link**: RMS → radius. Beat phase → rotation. Onset → new layer generation.
- **Complexity**: Medium

#### OS8: Truchet Labyrinth
- **Category**: Mathematical
- **Concept**: Truchet tiles — squares containing arcs that create continuous paths when placed randomly. The random seed is driven by beat divisions, so the labyrinth reshuffles on each bar. Paths glow and pulse with the beat.
- **Parameters**:
  - `density` (`u_src_density`, default 0.5) — tile density (4-20 tiles across)
  - `style` (`u_src_style`, default 0.0) — 0 = quarter circles, 0.5 = diagonal lines, 1.0 = triangles
  - `thickness` (`u_src_thickness`, default 0.3) — path thickness
  - `glow` (`u_src_glow`, default 0.5) — path glow
  - `color_shift` (`u_src_color_shift`, default 0.0)
- **Audio Link**: `barPhase` or `onsetDetected` reshuffles tiles. Band energies color different regions. Beat phase animates flow along paths.
- **Complexity**: Medium

#### OS9: Hexagonal Grid
- **Category**: Geometric
- **Concept**: Hexagonal tiling where each hexagon cell reacts independently to a different frequency band. Creates a honeycomb display where you can literally see which frequencies are active — each hex lights up for its assigned band.
- **Parameters**:
  - `scale` (`u_src_scale`, default 0.5) — hex size
  - `gap` (`u_src_gap`, default 0.3) — gap between hexagons
  - `glow` (`u_src_glow`, default 0.5) — cell glow
  - `color_shift` (`u_src_color_shift`, default 0.0)
- **Audio Link**: 7 band energies control 7 hex regions. Each hex's brightness/color = its band energy.
- **Complexity**: Medium

#### OS10: Particle Nebula
- **Category**: Particle-like
- **Concept**: A cosmic nebula made from layered noise and Kaliset fractals. Color palette shifts with key detection. Density pulses with bass. Brightness spikes on onsets. Creates a vast, living space scene that breathes with the music.
- **Parameters**:
  - `density` (`u_src_density`, default 0.5) — nebula density
  - `scale` (`u_src_scale`, default 0.5) — zoom level
  - `speed` (`u_src_speed`, default 0.3) — drift speed
  - `color_shift` (`u_src_color_shift`, default 0.0)
- **Audio Link**: Bass → density. Centroid → color temperature. Key → palette. Onset → brightness flash.
- **GLSL**: Multi-octave noise with `abs(noise)` for wispy look, colored by temperature gradient.
- **Complexity**: Medium

#### OS11: Lightning Storm
- **Category**: Nature
- **Concept**: Generates branching lightning bolts that strike on audio onsets. The bolt position, intensity, and branching depth are determined by onset strength and spectral content. Multiple simultaneous bolts possible during dense transient passages.
- **Parameters**:
  - `intensity` (`u_src_intensity`, default 0.5) — bolt brightness
  - `branches` (`u_src_branches`, default 0.5) — branching complexity
  - `glow` (`u_src_glow`, default 0.5) — glow spread
  - `color_shift` (`u_src_color_shift`, default 0.0)
- **Audio Link**: `onsetDetected` triggers new bolts. `onsetStrength` controls intensity. `transientDensity` controls how many bolts are active. `spectralCentroid` controls bolt color (warm bass bolts, cool treble bolts).
- **GLSL**: Recursive noise-displaced line from top to random bottom point, with exponential falloff glow.
- **Complexity**: Medium

#### OS12: Fire
- **Category**: Nature
- **Concept**: Realistic fire simulation using upward heat diffusion with palette lookup. Flames dance with audio — bass drives the base intensity, onsets create flare-ups, spectral centroid shifts the flame color from deep red to bright white-blue.
- **Parameters**:
  - `height` (`u_src_height`, default 0.5) — flame height
  - `turbulence` (`u_src_turbulence`, default 0.5) — flame turbulence/wildness
  - `speed` (`u_src_speed`, default 0.5) — animation speed
  - `color_shift` (`u_src_color_shift`, default 0.0) — 0 = orange fire, 0.5 = blue fire, 1.0 = green fire
- **Audio Link**: Bass → flame height. Onset → flare-up. Centroid → color temperature.
- **Requires**: Stateful (heat buffer via ping-pong FBO)
- **Complexity**: Medium

#### OS13: Fluid Flow
- **Category**: Simulation
- **Concept**: 2D fluid simulation where audio injects energy. Bass injects downward force (like pressing the fluid from above), treble injects swirling vortices, onsets create explosive bursts. The fluid carries color that shifts with key detection.
- **Parameters**:
  - `viscosity` (`u_src_viscosity`, default 0.5) — fluid thickness
  - `diffusion` (`u_src_diffusion`, default 0.5) — color spread rate
  - `speed` (`u_src_speed`, default 0.5) — overall simulation speed
  - `color_shift` (`u_src_color_shift`, default 0.0)
- **Audio Link**: Band energies inject forces in different directions. Onset creates splashes. RMS controls overall energy.
- **Requires**: Stateful (multiple ping-pong FBOs for velocity + density fields)
- **Complexity**: Large (Navier-Stokes approximation in GLSL)

#### OS14: Starfield
- **Category**: Particle-like
- **Concept**: Classic starfield with parallax layers. Stars streak toward the viewer at BPM-synced speed. Onsets create speed bursts (warp drive). Star density controlled by transient density.
- **Parameters**:
  - `speed` (`u_src_speed`, default 0.5) — base speed
  - `density` (`u_src_density`, default 0.5) — star count
  - `streak` (`u_src_streak`, default 0.3) — motion blur / streak length
  - `color_shift` (`u_src_color_shift`, default 0.0)
- **Audio Link**: `beatPhase` pulses speed. Onset → warp burst. RMS → brightness.
- **Complexity**: Small

#### OS15: Rotating Cube
- **Category**: 3D (Ray-marched)
- **Concept**: A ray-marched 3D cube that rotates, pulses, and morphs. Faces can display colors from different frequency bands. Edges glow. The cube morphs toward a sphere on beat drops (using SDF smooth blend).
- **Parameters**:
  - `morph` (`u_src_morph`, default 0.0) — 0 = sharp cube, 1 = sphere
  - `size` (`u_src_size`, default 0.5)
  - `rotation_speed` (`u_src_rot_speed`, default 0.3)
  - `edge_glow` (`u_src_edge_glow`, default 0.5)
  - `color_shift` (`u_src_color_shift`, default 0.0)
- **Audio Link**: Beat phase → rotation sync. RMS → size pulse. Bass → morph toward sphere on drops. Band energies → per-face color.
- **Complexity**: Medium

#### OS16: Morphing Platonic
- **Category**: 3D (Ray-marched)
- **Concept**: Smoothly morphs between the 5 Platonic solids (tetrahedron → cube → octahedron → dodecahedron → icosahedron) driven by audio features. Each solid represents a different energy state.
- **Parameters**:
  - `morph` (`u_src_morph`, default 0.5) — shape selection (0-1 maps through 5 solids)
  - `size` (`u_src_size`, default 0.5)
  - `rotation` (`u_src_rotation`, default 0.3)
  - `metallic` (`u_src_metallic`, default 0.5) — surface reflectivity
  - `color_shift` (`u_src_color_shift`, default 0.0)
- **Audio Link**: `structuralState` selects shape range (normal = simple, drop = complex). Spectral centroid → morph position. Beat → rotation.
- **Complexity**: Large (5 SDF implementations + smooth blending)

#### OS17: Twisted Torus
- **Category**: 3D (Ray-marched)
- **Concept**: A torus that twists along its tube, creating Möbius-like geometry. The twist amount is driven by audio, and the torus breathes (radius changes) with the beat.
- **Parameters**:
  - `twist` (`u_src_twist`, default 0.0) — twist amount
  - `radius` (`u_src_radius`, default 0.5) — major radius
  - `tube` (`u_src_tube`, default 0.3) — tube radius
  - `rotation` (`u_src_rotation`, default 0.3)
  - `color_shift` (`u_src_color_shift`, default 0.0)
- **Audio Link**: Bass → radius pulse. Mid → twist amount. Beat phase → rotation.
- **GLSL**: Ray-march with `sdTorus(rotateY(p, p.z * twist), vec2(radius, tube))`.
- **Complexity**: Medium

#### OS18: Wormhole
- **Category**: 3D (Ray-marched)
- **Concept**: A spiraling tunnel with warped space that pulls the viewer inward. The tunnel cross-section morphs between shapes (circle → square → star) based on audio. Speed syncs to BPM. Creates an intense, hypnotic flythrough.
- **Parameters**:
  - `speed` (`u_src_speed`, default 0.5) — travel speed
  - `warp` (`u_src_warp`, default 0.5) — space warping amount
  - `cross_section` (`u_src_cross`, default 0.0) — tunnel shape (0 = round, 0.5 = square, 1.0 = star)
  - `color_shift` (`u_src_color_shift`, default 0.0)
- **Audio Link**: BPM → speed. Bass → warp. Spectral centroid → cross-section. Beat phase → pulsing.
- **Complexity**: Medium

#### OS19: Physarum Slime
- **Category**: Simulation
- **Concept**: Agent-based slime mold simulation. Thousands of virtual agents leave chemical trails as they explore, creating organic vein-like networks. Audio onsets inject new agents. Band energies control agent behavior (turn speed, sensor distance). The result looks like a living organism responding to music.
- **Parameters**:
  - `agents` (`u_src_agents`, default 0.5) — agent density
  - `trail_decay` (`u_src_trail_decay`, default 0.5) — how fast trails fade
  - `sensor_angle` (`u_src_sensor_angle`, default 0.5) — agent turning behavior
  - `speed` (`u_src_speed`, default 0.5)
  - `color_shift` (`u_src_color_shift`, default 0.0)
- **Audio Link**: Onset → inject agents. Band energies → sensor parameters. RMS → speed.
- **Requires**: Stateful (agent positions in texture, trail buffer in separate FBO)
- **Complexity**: Large (but stunning results)

#### OS20: DNA Helix
- **Category**: 3D
- **Concept**: Double helix structure (like DNA) rotating in 3D space. The helix rungs light up individually based on the 12 chromagram notes — each rung represents a pitch class. Creates a beautiful intersection of biological structure and musical harmony.
- **Parameters**:
  - `speed` (`u_src_speed`, default 0.3) — rotation speed
  - `zoom` (`u_src_zoom`, default 0.5) — camera distance
  - `glow` (`u_src_glow`, default 0.5) — rung glow intensity
  - `color_shift` (`u_src_color_shift`, default 0.0)
- **Audio Link**: `chromagram[12]` → 12 rung brightness levels. Beat phase → rotation. RMS → glow.
- **Complexity**: Medium

#### OS21: Fibonacci Spiral
- **Category**: Mathematical
- **Concept**: Golden ratio spiral with dots/elements placed at Fibonacci angles. The number of elements grows with RMS. Each element's size pulses with a different beat subdivision. Creates organic, flower-like patterns that unfurl with the music.
- **Parameters**:
  - `elements` (`u_src_elements`, default 0.5) — max element count (20-200)
  - `size` (`u_src_size`, default 0.5) — element size
  - `spread` (`u_src_spread`, default 0.5) — spiral tightness
  - `glow` (`u_src_glow`, default 0.3)
  - `color_shift` (`u_src_color_shift`, default 0.0)
- **Audio Link**: RMS → active element count. Beat phase → element pulse. Key → color palette.
- **Complexity**: Small

#### OS22: Radar Sweep
- **Category**: Geometric
- **Concept**: Radar screen with rotating sweep line. Audio features create "blips" at different positions — bass blips near center, treble blips near edge. Onsets create bright flashes on the sweep. Creates a sci-fi sonar aesthetic that maps the audio spectrum spatially.
- **Parameters**:
  - `speed` (`u_src_speed`, default 0.5) — sweep speed (syncs to BPM)
  - `decay` (`u_src_decay`, default 0.5) — blip persistence
  - `grid` (`u_src_grid`, default 0.3) — range ring visibility
  - `color_shift` (`u_src_color_shift`, default 0.0) — 0 = classic green, 0.5 = blue, 1.0 = orange
- **Audio Link**: Band energies → blip positions (radial distance). Onset → bright flash. Beat phase → sweep angle.
- **Complexity**: Small

#### OS23: Kaleidoscopic IFS
- **Category**: Fractal
- **Concept**: 3D kaleidoscopic Iterated Function System (IFS) fractal. The fold angles are driven by audio features, creating infinitely complex structures that morph with the music. Different from our existing 2D kaleido fractal — this is a fully ray-marched 3D structure with lighting and reflections.
- **Parameters**:
  - `fold_x` (`u_src_fold_x`, default 0.5) — X fold angle
  - `fold_y` (`u_src_fold_y`, default 0.5) — Y fold angle
  - `fold_z` (`u_src_fold_z`, default 0.5) — Z fold angle
  - `iterations` (`u_src_iterations`, default 0.5) — fractal depth
  - `camera_distance` (`u_src_cam_dist`, default 0.5)
  - `color_shift` (`u_src_color_shift`, default 0.0)
- **Audio Link**: 3 fold angles from 3 band groups (low/mid/high). Beat phase → camera orbit. Onset → iteration spike.
- **Complexity**: Large

#### OS24: Glitch Grid
- **Category**: Pattern
- **Concept**: A grid of rectangles that glitch, flicker, and shuffle based on audio. Each cell randomly flips color, shifts position, or strobes at different beat divisions. Creates a controlled-chaos digital aesthetic — like a malfunctioning LED wall that dances to music.
- **Parameters**:
  - `grid_size` (`u_src_grid`, default 0.4) — cell count
  - `chaos` (`u_src_chaos`, default 0.5) — glitch intensity
  - `flicker` (`u_src_flicker`, default 0.5) — flicker rate
  - `color_shift` (`u_src_color_shift`, default 0.0)
- **Audio Link**: Onset → grid shuffle. RMS → brightness. Transient density → flicker rate. Band energies → per-cell color.
- **Complexity**: Small

#### OS25: Rose Curves
- **Category**: Mathematical
- **Concept**: Animated rose curves (`r = cos(k * theta)`) where the k parameter (petal count) is driven by detected pitch ratios. Simple intervals create simple flowers; complex harmonics create complex multi-petaled forms. The flower literally blooms with the music's harmonic content.
- **Parameters**:
  - `k` (`u_src_k`, default 0.5) — petal parameter (auto from pitch, or manual)
  - `thickness` (`u_src_thickness`, default 0.3) — petal thickness
  - `layers` (`u_src_layers`, default 0.5) — overlapping rose layers
  - `glow` (`u_src_glow`, default 0.5)
  - `color_shift` (`u_src_color_shift`, default 0.0)
- **Audio Link**: `dominantPitch` → k parameter (maps frequency ratios to petal count). RMS → size. Beat phase → rotation.
- **Complexity**: Small

---

### SUMMARY OF ORIGINAL ADDITIONS

| Category | Effects | Sources |
|----------|---------|---------|
| Audio-Feature-Driven (unique to us) | 10 (OE1-OE10) | 4 (OS1-OS4) |
| Mathematical / Geometric | 3 (OE17, OE20, OE21) | 6 (OS5-OS9, OS21, OS22, OS25) |
| 3D / Ray-Marched | 2 (OE13, OE23) | 5 (OS15-OS18, OS23) |
| Time / Temporal | 2 (OE14, OE19) | — |
| Glitch / Data | 2 (OE22, OE24) | 1 (OS24) |
| Simulation / Nature | — | 3 (OS11-OS13, OS19) |
| Pattern / Stylize | 3 (OE4, OE12, OE18) | 1 (OS8) |
| Color | 4 (OE6, OE9, OE16, OE25) | — |
| Warp / Distortion | 4 (OE2, OE3, OE5, OE7, OE11, OE15) | — |
| **TOTAL** | **25** | **25** |

### Grand Total with Originals

| | Before | Resolume Integration | Originals | **Final Total** |
|--|--------|---------------------|-----------|----------------|
| Effects | 76 | +32 | +25 | **133** |
| Sources | 10 | +12 | +25 | **47** |
| **Combined** | 86 | 44 | 50 | **180** |

This gives Audio-DNA **133 effects and 47 sources = 180 visual tools**, dwarfing Resolume's ~90 effects + 22 sources = 112. More importantly, approximately 50 of our tools directly exploit audio features that Resolume cannot access (chromagram, MFCCs, key detection, structural state, pitch tracking), making them impossible to replicate in any competing tool.
