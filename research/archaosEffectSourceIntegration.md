# ArKaos Effect & Source Integration Plan

> Comprehensive implementation blueprint for integrating ArKaos-inspired effects, sources,
> per-type automation, and FFGL plugin hosting into Audio-DNA v2.
>
> Prepared: 2026-03-20
> Cross-referenced against: ArKaos VJ 3.6.1 Release Notes, Audio-DNA CLAUDE.md,
> EmbeddedShaders.h (67 effects), SourceRegistry.cpp (10 sources), gap analysis.

---

## TABLE OF CONTENTS

1. [Executive Summary](#1-executive-summary)
2. [New Effects (8 shaders)](#2-new-effects-8-shaders)
3. [New Sources (4 generators)](#3-new-sources-4-generators)
4. [Feedback Loop System (Larsen)](#4-feedback-loop-system-larsen)
5. [Per-Type Automation System](#5-per-type-automation-system)
6. [FFGL Plugin Hosting](#6-ffgl-plugin-hosting)
7. [Implementation Order & Dependencies](#7-implementation-order--dependencies)
8. [File Change Manifest](#8-file-change-manifest)
9. [Testing Strategy](#9-testing-strategy)

---

## 1. Executive Summary

### What We're Adding

| Category | Count | Items |
|----------|-------|-------|
| **New Effects** | 8 | Tile Grid, Infinite Zoom, Bump Light, Pop Raster, Screen Split, Spot Zoom, Neon Edge, Cartoon Ink |
| **New Sources** | 4 | Scroll Plane, Rotating Cube Map, Dual Plane Drift, Color Bars |
| **Systems** | 3 | Feedback Loop (Larsen), Per-Type Automation, FFGL Plugin Hosting |

### Naming Philosophy

Every effect and source gets a **descriptive name** that communicates what it does
at a glance. No product-specific names (no "ArKolor", "Turnix3D", "Larsen"). Names
should be evocative and intuitive:

| ArKaos Name | Our Name | Why |
|-------------|----------|-----|
| Tiling | **Tile Grid** | Describes the grid of repeated tiles |
| RotoZoom | **Infinite Zoom** | Captures the endless zooming rotation feel |
| Bumpy Surface | **Bump Light** | Edge-based bump mapping with dynamic light |
| Pop Art | **Pop Raster** | Pop art + raster/halftone mashup |
| Video Split | **Screen Split** | Grid split with animated refresh |
| Target | **Spot Zoom** | Spotlight zoom into a region |
| Neon | **Neon Edge** | Neon glow on detected edges |
| Cartoon | **Cartoon Ink** | Cartoon edge + color quantization |
| Scroller | **Scroll Plane** | Visual scrolling on a plane |
| Cube Inside | **Rotating Cube Map** | Camera inside a rotating cube |
| Plane | **Dual Plane Drift** | Two planes drifting past camera |
| Color Bars | **Color Bars** | Industry standard name, keep it |
| Larsen | **Feedback Loop** | Self-explanatory, what it actually does |
| Stroboscope modes | Enhance existing **Strobe** | Add missing modes to current effect |

---

## 2. New Effects (8 shaders)

Each effect is a single GLSL 410 fragment shader. All parameters [0, 1].
Implementation: add shader to EmbeddedShaders.h, register in EffectLibrary.cpp.

---

### 2.1 Tile Grid

**Category**: `warp`
**ArKaos equivalent**: Tiling
**What it does**: Repeats the input visual in an NxM grid. Supports fractional tile counts
for smooth animation. Tiles can be offset (brick pattern) and individually scaled.

**Shader name**: `tileGrid`
**Uniform prefix**: `u_tilegrid_`

| Parameter | Uniform | Default | Range (mapped) | Description |
|-----------|---------|---------|---------------|-------------|
| Columns | `u_tilegrid_cols` | 0.25 | 1-16 | Number of horizontal tiles |
| Rows | `u_tilegrid_rows` | 0.25 | 1-16 | Number of vertical tiles |
| Offset | `u_tilegrid_offset` | 0.0 | 0-1 | Brick-pattern row offset |
| Zoom | `u_tilegrid_zoom` | 0.5 | 0.1-2.0 | Per-tile zoom factor |

**GLSL implementation approach**:
```glsl
vec2 uv = v_texCoord;
float cols = mix(1.0, 16.0, u_tilegrid_cols);
float rows = mix(1.0, 16.0, u_tilegrid_rows);
// Apply brick offset on odd rows
float rowIdx = floor(uv.y * rows);
float xOffset = mod(rowIdx, 2.0) * u_tilegrid_offset * 0.5;
vec2 tileUV = fract(vec2(uv.x * cols + xOffset, uv.y * rows));
// Per-tile zoom (zoom toward center of each tile)
tileUV = (tileUV - 0.5) / mix(0.1, 2.0, u_tilegrid_zoom) + 0.5;
vec4 color = texture(u_texture, clamp(tileUV, 0.0, 1.0));
```

**Audio mapping ideas**: Columns driven by Bass energy (more tiles = more energy).
Zoom pulsed by Beat Phase. Offset by Bar Phase for rhythmic shifting.

---

### 2.2 Infinite Zoom

**Category**: `warp`
**ArKaos equivalent**: RotoZoom
**What it does**: Creates an infinite rotating zoom effect. The image continuously
zooms in while rotating, creating a hypnotic spiral tunnel effect. Uses UV
feedback — the current frame's center is the previous frame's zoomed region.

**Shader name**: `infiniteZoom`
**Uniform prefix**: `u_infzoom_`

| Parameter | Uniform | Default | Range (mapped) | Description |
|-----------|---------|---------|---------------|-------------|
| Zoom Speed | `u_infzoom_speed` | 0.3 | 0-2.0 | Speed of continuous zoom |
| Rotation Speed | `u_infzoom_rotation` | 0.2 | -2.0 to 2.0 | Rotation speed (neg = CCW) |
| Center X | `u_infzoom_cx` | 0.5 | 0-1 | Zoom focal point X |
| Center Y | `u_infzoom_cy` | 0.5 | 0-1 | Zoom focal point Y |

**GLSL implementation approach**:
```glsl
vec2 uv = v_texCoord - vec2(u_infzoom_cx, u_infzoom_cy);
float zoomAmt = mix(0.0, 2.0, u_infzoom_speed);
float rotAmt = mix(-2.0, 2.0, u_infzoom_rotation);
// Progressive zoom: scale UV inward over time
float scale = 1.0 - zoomAmt * 0.01; // per-frame shrink
uv *= scale;
// Rotation
float angle = rotAmt * 0.01;
float c = cos(angle), s = sin(angle);
uv = mat2(c, -s, s, c) * uv;
uv += vec2(u_infzoom_cx, u_infzoom_cy);
vec4 color = texture(u_texture, uv);
```

**Note**: For true infinite zoom, this effect benefits from the Feedback Loop system
(Section 4). When used standalone, it operates on the current frame only (single-pass
zoom/rotate). When combined with Feedback Loop, it creates the classic ArKaos
infinite tunnel effect.

**Audio mapping ideas**: Zoom Speed driven by RMS. Rotation by Spectral Centroid.
Center position by dominant pitch.

---

### 2.3 Bump Light

**Category**: `pattern`
**ArKaos equivalent**: Bumpy Surface
**What it does**: Performs edge detection on the input, uses the edges as a height map
(bump map), then applies dynamic directional lighting. Creates a 3D embossed look
that responds to a moving light source.

**Shader name**: `bumpLight`
**Uniform prefix**: `u_bumplight_`

| Parameter | Uniform | Default | Range (mapped) | Description |
|-----------|---------|---------|---------------|-------------|
| Light X | `u_bumplight_lx` | 0.5 | 0-1 | Light source horizontal position |
| Light Y | `u_bumplight_ly` | 0.3 | 0-1 | Light source vertical position |
| Intensity | `u_bumplight_intensity` | 0.6 | 0-2.0 | Light brightness |
| Bump Height | `u_bumplight_height` | 0.5 | 0-1 | Height map exaggeration |

**GLSL implementation approach**:
```glsl
// Sobel edge detection for normal map
float tl = luminance(texture(u_texture, uv + vec2(-px, -py)));
float tr = luminance(texture(u_texture, uv + vec2( px, -py)));
float bl = luminance(texture(u_texture, uv + vec2(-px,  py)));
float br = luminance(texture(u_texture, uv + vec2( px,  py)));
float dX = (tr + br) - (tl + bl);
float dY = (bl + br) - (tl + tr);
vec3 normal = normalize(vec3(dX * height, dY * height, 1.0));
// Directional lighting
vec3 lightDir = normalize(vec3(lx - uv.x, ly - uv.y, 0.3));
float diffuse = max(dot(normal, lightDir), 0.0) * intensity;
vec4 base = texture(u_texture, uv);
fragColor = vec4(base.rgb * (0.3 + diffuse), base.a);
```

**Audio mapping ideas**: Light X/Y orbit driven by Bar Phase (circular motion).
Intensity pulsed by Beat Phase. Bump Height by spectral flux.

---

### 2.4 Pop Raster

**Category**: `pattern`
**ArKaos equivalent**: Pop Art
**What it does**: Replaces pixel luminosity bands with pre-defined color palettes and
patterns. Creates a screen-printed poster look. Multiple palette presets (Warhol,
Lichtenstein, CMYK, Neon).

**Shader name**: `popRaster`
**Uniform prefix**: `u_popraster_`

| Parameter | Uniform | Default | Range (mapped) | Description |
|-----------|---------|---------|---------------|-------------|
| Palette | `u_popraster_palette` | 0.0 | 0-4 (stepped) | Color palette preset |
| Bands | `u_popraster_bands` | 0.4 | 2-8 | Number of luminosity bands |
| Pattern Size | `u_popraster_size` | 0.3 | 4-64 px | Raster dot size |
| Mix | `u_popraster_mix` | 0.7 | 0-1 | Blend with original |

**GLSL implementation approach**:
```glsl
float lum = luminance(texture(u_texture, uv).rgb);
int band = int(lum * numBands);
// Palette lookup (4 palettes, each with up to 8 colors)
vec3 palettes[4][8] = { /* Warhol, Lichtenstein, CMYK, Neon */ };
vec3 bandColor = palettes[paletteIdx][band];
// Optional halftone dot pattern overlay
float dotPattern = smoothstep(0.4, 0.6,
    length(fract(uv * patternSize) - 0.5) * 2.0 - (1.0 - lum));
vec3 result = mix(bandColor, bandColor * dotPattern, 0.3);
fragColor = vec4(mix(texture(u_texture, uv).rgb, result, mixAmt), 1.0);
```

**Audio mapping ideas**: Palette cycling on structural state changes.
Band count driven by spectral flatness (noisier = more bands).

---

### 2.5 Screen Split

**Category**: `glitch`
**ArKaos equivalent**: Video Split
**What it does**: Divides the screen into an NxM grid of sub-screens. Each sub-screen
can update at a different rate (running update, simultaneous, or random/frantic).
Creates a surveillance-camera or time-slice effect.

**Shader name**: `screenSplit`
**Uniform prefix**: `u_screensplit_`

| Parameter | Uniform | Default | Range (mapped) | Description |
|-----------|---------|---------|---------------|-------------|
| Columns | `u_screensplit_cols` | 0.25 | 2-8 | Grid columns |
| Rows | `u_screensplit_rows` | 0.25 | 2-8 | Grid rows |
| Refresh Speed | `u_screensplit_speed` | 0.5 | 0-1 | How fast cells update |
| Mode | `u_screensplit_mode` | 0.0 | 0-2 (stepped) | 0=running, 1=simultaneous, 2=random |

**Implementation note**: This effect requires **temporal state** — each cell needs to
freeze its content at a different time. This is best implemented as a multi-FBO
approach where each cell captures the input at its own update time.

**Simpler alternative**: Use `u_time` modulo per-cell offset to create a staggered
freeze effect. Each cell samples from a slightly different time offset, creating the
illusion of asynchronous updates:
```glsl
vec2 cellIdx = floor(uv * vec2(cols, rows));
float cellPhase = fract(u_time * speed + hash(cellIdx) * 0.7);
// Each cell slightly offsets the UV based on its phase
// (approximates temporal offset without actual frame buffer)
vec2 offset = vec2(sin(cellPhase * 6.28), cos(cellPhase * 6.28)) * 0.002;
vec2 cellUV = fract(uv * vec2(cols, rows));
fragColor = texture(u_texture, (cellIdx + cellUV) / vec2(cols, rows) + offset);
// Draw cell borders
float border = step(0.02, cellUV.x) * step(cellUV.x, 0.98)
             * step(0.02, cellUV.y) * step(cellUV.y, 0.98);
fragColor.rgb *= mix(0.3, 1.0, border);
```

**Full implementation (with actual frame freezing)**: Requires a texture array or tiled
FBO where each cell's content is captured at its individual update tick. This is a
Phase 2 enhancement — the UV-offset approximation ships first.

**Audio mapping ideas**: Refresh speed driven by BPM. Mode switched on onset detection.

---

### 2.6 Spot Zoom

**Category**: `warp`
**ArKaos equivalent**: Target
**What it does**: Defines a rectangular or circular region that is zoomed/magnified while
the rest of the screen shows the original (optionally darkened). Great for live cameras —
zoom into the crowd, the DJ, or a detail.

**Shader name**: `spotZoom`
**Uniform prefix**: `u_spotzoom_`

| Parameter | Uniform | Default | Range (mapped) | Description |
|-----------|---------|---------|---------------|-------------|
| Center X | `u_spotzoom_cx` | 0.5 | 0-1 | Zoom region center X |
| Center Y | `u_spotzoom_cy` | 0.5 | 0-1 | Zoom region center Y |
| Size | `u_spotzoom_size` | 0.3 | 0.05-0.8 | Radius of zoom region |
| Zoom | `u_spotzoom_zoom` | 0.7 | 1-10x | Magnification factor |
| Shape | `u_spotzoom_shape` | 0.0 | 0-1 (0=circle, 1=rect) | Region shape |
| Background | `u_spotzoom_bg` | 0.3 | 0-1 | Background brightness |

**GLSL implementation approach**:
```glsl
vec2 center = vec2(u_spotzoom_cx, u_spotzoom_cy);
float dist = length((uv - center) / vec2(aspect, 1.0)); // or rect distance
float size = mix(0.05, 0.8, u_spotzoom_size);
float zoom = mix(1.0, 10.0, u_spotzoom_zoom);
float inRegion = smoothstep(size, size - 0.01, dist);
// Inside: zoom toward center
vec2 zoomedUV = center + (uv - center) / zoom;
// Outside: original with darkening
vec4 zoomedColor = texture(u_texture, zoomedUV);
vec4 bgColor = texture(u_texture, uv) * u_spotzoom_bg;
fragColor = mix(bgColor, zoomedColor, inRegion);
```

**Audio mapping ideas**: Center X/Y driven by dominant pitch (X) and spectral centroid (Y).
Size pulsed by RMS. Zoom by onset strength.

---

### 2.7 Neon Edge

**Category**: `pattern`
**ArKaos equivalent**: Neon
**What it does**: Detects edges in the image, applies a colored glow around them,
and optionally removes the original image leaving only neon outlines. Multi-pass
blur creates the glow bloom.

**Shader name**: `neonEdge`
**Uniform prefix**: `u_neonedge_`

| Parameter | Uniform | Default | Range (mapped) | Description |
|-----------|---------|---------|---------------|-------------|
| Edge Intensity | `u_neonedge_edge` | 0.6 | 0-2.0 | Edge detection sensitivity |
| Glow Radius | `u_neonedge_glow` | 0.5 | 0-1 | Blur radius for glow |
| Color Hue | `u_neonedge_hue` | 0.5 | 0-1 | Neon color (hue wheel) |
| Show Original | `u_neonedge_original` | 0.3 | 0-1 | Blend with original image |

**GLSL implementation approach**:
```glsl
// Sobel edge detection
float edge = sobelMagnitude(u_texture, uv, px, py) * edgeIntensity;
// Neon color from hue
vec3 neonColor = hsv2rgb(vec3(u_neonedge_hue, 1.0, 1.0));
// Glow: sample multiple offsets (poor man's bloom)
float glow = 0.0;
for (int i = -3; i <= 3; i++)
    for (int j = -3; j <= 3; j++)
        glow += sobelMagnitude(u_texture, uv + vec2(i,j) * px * glowRadius, px, py);
glow /= 49.0;
vec3 neonGlow = neonColor * (edge + glow * 0.5);
vec3 original = texture(u_texture, uv).rgb;
fragColor = vec4(mix(neonGlow, original + neonGlow, u_neonedge_original), 1.0);
```

**Audio mapping ideas**: Edge intensity by spectral flux. Glow pulsed by beat.
Hue rotated by bar phase.

---

### 2.8 Cartoon Ink

**Category**: `pattern`
**ArKaos equivalent**: Cartoon
**What it does**: Detects edges (draws them in black ink), then quantizes colors into
a limited palette. Two modes: "Simple" (pure quantization) and "Bold" (thick edges
with color posterization). Creates a comic book / cel-shaded look.

**Shader name**: `cartoonInk`
**Uniform prefix**: `u_cartoonink_`

| Parameter | Uniform | Default | Range (mapped) | Description |
|-----------|---------|---------|---------------|-------------|
| Edge Width | `u_cartoonink_edge` | 0.4 | 0-1 | Ink line thickness |
| Color Steps | `u_cartoonink_steps` | 0.4 | 2-12 | Color quantization levels |
| Ink Strength | `u_cartoonink_ink` | 0.7 | 0-1 | How dark the ink outlines are |
| Saturation | `u_cartoonink_sat` | 0.6 | 0-2 | Color saturation boost |

**GLSL implementation approach**:
```glsl
vec4 base = texture(u_texture, uv);
// Color quantization
float steps = mix(2.0, 12.0, u_cartoonink_steps);
vec3 quantized = floor(base.rgb * steps + 0.5) / steps;
quantized = mix(quantized, quantized * sat, 1.0); // saturation boost
// Edge detection (Sobel)
float edge = sobelMagnitude(u_texture, uv, px * edgeWidth, py * edgeWidth);
float ink = smoothstep(0.1, 0.3, edge) * u_cartoonink_ink;
fragColor = vec4(quantized * (1.0 - ink), base.a);
```

**Audio mapping ideas**: Color steps driven by structural state (fewer in breakdowns,
more in drops). Edge width by transient density.

---

### 2.9 Strobe Enhancement (upgrade existing effect)

Our existing Strobe effect only has `speed` and `intensity` parameters. ArKaos has
a richer stroboscope with multiple modes. **Upgrade the existing shader** rather than
creating a new one.

**New parameters to add** (keeping existing `u_strobe_speed` and `u_strobe_intensity`):

| Parameter | Uniform | Default | Range (mapped) | Description |
|-----------|---------|---------|---------------|-------------|
| Mode | `u_strobe_mode` | 0.0 | 0-3 (stepped) | 0=black, 1=white, 2=invert, 3=background |
| Shape | `u_strobe_shape` | 0.0 | 0-1 (0=square, 1=sine) | Transition shape |

**Mode behavior**:
- **Black** (0): Alternates between visual and black
- **White** (1): Alternates between visual and white
- **Invert** (2): Alternates between visual and inverted colors
- **Background** (3): Alternates between visual and the layer below

**Shape behavior**:
- **Square** (0): Hard cut, no transition
- **Sine** (1): Smooth sinusoidal transition

---

## 3. New Sources (4 generators)

Sources generate content from nothing (or from audio input). Each is a GLSL shader
registered in SourceRegistry.

---

### 3.1 Scroll Plane

**ID**: `scroll_plane`
**Category**: Geometric
**ArKaos equivalent**: Scroller
**What it does**: A textured plane that continuously scrolls in a configurable direction.
When used with a loaded image, creates an infinite scrolling backdrop. When used
standalone, generates scrolling color/noise patterns.

**Shader name**: `sourceScrollPlane`

| Parameter | Uniform | Default | Description |
|-----------|---------|---------|-------------|
| Speed X | `u_src_speedx` | 0.5 | Horizontal scroll speed |
| Speed Y | `u_src_speedy` | 0.0 | Vertical scroll speed |
| Scale | `u_src_scale` | 0.5 | Texture scale (zoom) |
| Warp | `u_src_warp` | 0.0 | Perspective warp amount (0=flat, 1=vanishing point) |

**GLSL approach**:
```glsl
vec2 uv = v_texCoord;
float speedX = (u_src_speedx - 0.5) * 2.0;
float speedY = (u_src_speedy - 0.5) * 2.0;
uv = fract(uv * mix(0.5, 4.0, u_src_scale) + vec2(speedX, speedY) * u_time);
// Optional perspective warp for depth illusion
if (u_src_warp > 0.01) {
    float perspective = mix(1.0, 0.3, uv.y * u_src_warp);
    uv.x = 0.5 + (uv.x - 0.5) / perspective;
}
// Generate a procedural pattern (or sample input texture if compositing)
fragColor = proceduralGrid(uv); // or texture(u_inputTexture, uv)
```

---

### 3.2 Rotating Cube Map

**ID**: `rotating_cube_map`
**Category**: Geometric
**ArKaos equivalent**: Cube Inside
**What it does**: Places the camera inside a rotating cube. Each face displays
the source texture. Camera looks forward, cube rotates around the camera.
Creates an immersive box room effect.

**Shader name**: `sourceRotatingCubeMap`

| Parameter | Uniform | Default | Description |
|-----------|---------|---------|-------------|
| Rotation X | `u_src_rotx` | 0.3 | X-axis rotation speed |
| Rotation Y | `u_src_roty` | 0.5 | Y-axis rotation speed |
| Scale | `u_src_scale` | 0.5 | Visual texture size on cube faces |
| Light | `u_src_light` | 0.5 | Light position rotation |

**GLSL approach**: Ray-march a box SDF from inside, apply rotation matrix
based on u_time * speed, sample texture using the hit face's UV coordinates.

---

### 3.3 Dual Plane Drift

**ID**: `dual_plane_drift`
**Category**: Geometric
**ArKaos equivalent**: Plane
**What it does**: Two textured planes drift past the camera (above and below, or
side by side). Creates a parallax corridor effect. Planes can move at different
speeds for depth perception.

**Shader name**: `sourceDualPlaneDrift`

| Parameter | Uniform | Default | Description |
|-----------|---------|---------|-------------|
| Speed | `u_src_speed` | 0.5 | Plane movement speed |
| Rotation | `u_src_rotation` | 0.0 | Plane tilt/rotation |
| Distance | `u_src_distance` | 0.5 | Gap between planes |
| Scale | `u_src_scale` | 0.5 | Visual texture tiling |

**GLSL approach**: Two horizontal planes at y=+d and y=-d in 3D space.
Camera at origin looking forward. Each plane scrolls at `speed` along Z.
UV coordinates computed from ray-plane intersection.

---

### 3.4 Color Bars

**ID**: `color_bars`
**Category**: Test
**ArKaos equivalent**: Color Bars
**What it does**: Generates SMPTE color bars (or EBU, or simplified). Useful as
a test pattern, but also as a source that can be processed through effects for
a retro broadcast aesthetic.

**Shader name**: `sourceColorBars`

| Parameter | Uniform | Default | Description |
|-----------|---------|---------|-------------|
| Pattern | `u_src_pattern` | 0.0 | 0=SMPTE, 0.5=EBU, 1.0=Simplified |
| Noise | `u_src_noise` | 0.0 | Static noise overlay amount |
| Scanlines | `u_src_scanlines` | 0.0 | Scanline overlay intensity |
| Drift | `u_src_drift` | 0.0 | Horizontal drift/roll for VHS look |

**GLSL approach**: Hard-coded SMPTE bar positions and colors. Simple step
functions based on UV.x position. Noise and scanlines overlaid via standard
patterns.

---

## 4. Feedback Loop System (Larsen)

This is the most architecturally significant addition. ArKaos had three Larsen
variants (Simple, Presets, Full). We implement a single, powerful **Feedback Loop**
system that subsumes all three.

### 4.1 Concept

A feedback loop routes a layer's **output back to its own input** with a
transformation applied. Each frame:

1. Read the previous frame's output from a feedback FBO
2. Apply transform (scale, rotate, translate)
3. Blend with the current input at a given opacity (feedback amount)
4. The blended result becomes the new output AND is stored for next frame

This creates recursive, infinitely deepening visuals — the signature ArKaos
look.

### 4.2 Architecture

**New class**: `FeedbackProcessor` (header-only or small .h/.cpp)

```
src/render/FeedbackProcessor.h
src/render/FeedbackProcessor.cpp
```

**Data model addition** to `Layer`:
```cpp
struct FeedbackConfig {
    bool enabled = false;
    float amount = 0.5f;     // [0,1] — how much of prev frame bleeds through
    float scaleX = 0.98f;    // Per-frame scale X (< 1 = zoom in, > 1 = zoom out)
    float scaleY = 0.98f;    // Per-frame scale Y
    float rotation = 0.0f;   // Per-frame rotation in degrees
    float offsetX = 0.0f;    // Per-frame horizontal drift
    float offsetY = 0.0f;    // Per-frame vertical drift
};
FeedbackConfig feedback;
```

**Render pipeline integration**:

Currently the compositor renders each layer's clip, applies per-layer effects,
then blends into the accumulator. The feedback loop inserts between
"render clip" and "apply effects":

```
[Previous frame FBO] ──transform──┐
                                  ├── blend(feedback.amount) ──► [Current clip]
[Current clip render] ────────────┘
         │
         ▼
    [Apply per-layer effects]
         │
         ▼
    [Store output to feedback FBO for next frame]
         │
         ▼
    [Blend into composition accumulator]
```

**FBO management**: Each layer with feedback enabled gets its own persistent
FBO pair (ping-pong). These are **not** the compositor's ping-pong FBOs — they
are separate, per-layer feedback buffers.

### 4.3 FeedbackProcessor API

```cpp
class FeedbackProcessor
{
public:
    FeedbackProcessor();
    ~FeedbackProcessor();

    // Initialize GL resources (call from openGLContextCreated)
    void initGL(int width, int height);
    void releaseGL();

    // Process one frame of feedback.
    // inputTexture: the current frame's rendered clip texture
    // config: feedback parameters
    // Returns: texture ID of the blended result (input + feedback)
    GLuint process(GLuint inputTexture, const FeedbackConfig& config);

    bool isInitialized() const;

private:
    GLuint fbos_[2] = {};
    GLuint textures_[2] = {};
    int currentBuffer_ = 0;
    int width_ = 0, height_ = 0;
    bool initialized_ = false;

    // Shader for feedback blend + transform
    std::unique_ptr<juce::OpenGLShaderProgram> feedbackShader_;
};
```

### 4.4 Feedback Shader

```glsl
#version 410 core
uniform sampler2D u_currentFrame;
uniform sampler2D u_previousFrame;
uniform float u_feedback_amount;
uniform float u_feedback_scaleX;
uniform float u_feedback_scaleY;
uniform float u_feedback_rotation;
uniform float u_feedback_offsetX;
uniform float u_feedback_offsetY;

in vec2 v_texCoord;
out vec4 fragColor;

void main()
{
    // Transform UV for previous frame sampling
    vec2 uv = v_texCoord - 0.5;

    // Scale
    uv /= vec2(u_feedback_scaleX, u_feedback_scaleY);

    // Rotate
    float angle = radians(u_feedback_rotation);
    float c = cos(angle), s = sin(angle);
    uv = mat2(c, -s, s, c) * uv;

    // Offset
    uv += vec2(u_feedback_offsetX, u_feedback_offsetY);
    uv += 0.5;

    vec4 prev = texture(u_previousFrame, uv);
    vec4 curr = texture(u_currentFrame, v_texCoord);

    // Blend: current frame on top of transformed previous frame
    fragColor = mix(curr, prev, u_feedback_amount);
}
```

### 4.5 UI Integration

Add a **"Feedback"** section to the LayerInspector (between Blend Mode and
Layer Effects):

```
[Feedback]           [Enable toggle]
  Amount    0.50   - + [========|=========]
  Scale X   0.98   - + [================|=]
  Scale Y   0.98   - + [================|=]
  Rotation  0.0    - + [========|=========]
  Offset X  0.0    - + [========|=========]
  Offset Y  0.0    - + [========|=========]
```

Each parameter gets a signal triangle for audio routing.

### 4.6 Presets (Classic Larsen Looks)

| Preset Name | Amount | Scale XY | Rotation | Offset | Look |
|-------------|--------|----------|----------|--------|------|
| Zoom In | 0.85 | 0.97, 0.97 | 0 | 0, 0 | Infinite zoom tunnel |
| Spiral | 0.80 | 0.98, 0.98 | 2.0 | 0, 0 | Rotating spiral |
| Drift Right | 0.75 | 1.0, 1.0 | 0 | 0.01, 0 | Trailing ghosting moving right |
| Kaleidoscope | 0.90 | 0.95, 0.95 | 5.0 | 0, 0 | Rapidly rotating recursive pattern |
| Echo | 0.60 | 1.0, 1.0 | 0 | 0, 0 | Simple motion trail/ghosting |
| Stretch | 0.85 | 1.02, 0.98 | 0 | 0, 0 | Anisotropic zoom (stretch one axis) |

---

## 5. Per-Type Automation System

ArKaos's "Intelligent Automation" categorizes visuals into three types and handles
each differently. This is a significant workflow improvement over our current
autopilot which treats all clips equally.

### 5.1 Concept

Each clip in a deck is classified by its **layer type**:

| Type | Definition | Automation Behavior |
|------|-----------|-------------------|
| **Opaque** | Full-screen visual (fills the screen completely) | Only 1 plays at a time. Cycles after N beats. New one replaces old. |
| **Transparent** | Visual with compositing (alpha, blend mode) | Multiple can play simultaneously. Number of active layers configurable. Cycles independently. |
| **Effect** | Effect-only layer (no media, just FX on accumulator) | Multiple can play. Shorter cycles. Randomization more aggressive. |

### 5.2 Data Model Changes

**In `Composition` struct** (or a new `AutopilotConfig` embedded in Composition):

```cpp
struct PerTypeAutopilotConfig
{
    // Opaque layers
    int opaqueCycleBeats = 16;        // beats before advancing to next opaque clip
    bool opaquePlayUntilEnd = false;  // true = play movie to end before advancing

    // Transparent layers
    int transparentCycleBeats = 8;    // beats per transparent clip cycle
    int transparentMaxLayers = 2;     // max simultaneous transparent layers
    bool transparentRandomize = true; // random selection vs sequential

    // Effect layers
    int effectCycleBeats = 4;         // beats per effect cycle (shorter = more dynamic)
    int effectMaxLayers = 2;          // max simultaneous effect layers
    bool effectRandomize = true;      // random selection vs sequential

    // Global
    bool globalRandomize = false;     // if true, all types use random selection
    bool loopAutopilot = true;        // loop back to start when all clips played
};
```

### 5.3 Autopilot Engine Changes

The current `Autopilot::advance()` method treats all layers uniformly. The
per-type system needs to:

1. **Classify each layer** by its type (read from `Layer::type` enum)
2. **Track per-type state** independently:
   - Which opaque clip is playing, when it started
   - Which transparent clips are active, their individual timers
   - Which effect clips are active, their individual timers
3. **Advance per-type clocks**:
   - On each beat: check if opaque timer expired → advance
   - On each beat: check each transparent layer → advance individually
   - On each beat: check each effect layer → advance individually
4. **Enforce limits**:
   - Only 1 opaque layer active at a time
   - Max N transparent layers active (excess get ejected)
   - Max N effect layers active

### 5.4 UI: Automation Setup Dialog

Add a section to the **CompositionInspector** under Autopilot:

```
[Autopilot]
  Direction: [◀◀] [OFF] [▶▶] [🔀]
  ─────────────────────────────────
  Opaque Visuals
    Cycle: [16] beats  □ Play movies until end
  ─────────────────────────────────
  Transparent Visuals
    Cycle: [8] beats   Layers: [2]
    □ Randomize
  ─────────────────────────────────
  Effect Layers
    Cycle: [4] beats   Layers: [2]
    □ Randomize
  ─────────────────────────────────
  □ Loop   □ Global Randomize
```

### 5.5 Layer Type Detection

Layers already have a `type` field (`LayerType` enum: Opaque, Transparent,
FXOnly, ThreeD, Mask`). The autopilot maps these:

| LayerType | Autopilot Category |
|-----------|-------------------|
| Opaque | Opaque |
| Transparent | Transparent |
| FXOnly | Effect |
| ThreeD | Transparent (treated as composited) |
| Mask | Transparent (treated as composited) |

---

## 6. FFGL Plugin Hosting

FFGL (FreeFrame GL) is the open-source, cross-platform plugin standard for
real-time video effects. ArKaos was one of the first VJ apps to support it.
Resolume uses FFGL 2.x extensively. Adding FFGL support gives Audio-DNA
access to hundreds of community effects.

### 6.1 What is FFGL?

- **FFGL 1.x**: CPU-based frame processing (deprecated)
- **FFGL 2.x**: GPU-based, plugins receive/return OpenGL textures
- Plugins are dynamic libraries (.dll on Windows, .bundle on macOS, .so on Linux)
- Each plugin declares parameters, processes a texture, returns a texture
- Standard: https://github.com/resolume/ffgl

### 6.2 Architecture

**New directory**: `src/plugins/`

```
src/plugins/FFGLHost.h        — Plugin loader and lifecycle management
src/plugins/FFGLHost.cpp
src/plugins/FFGLPlugin.h      — Single plugin instance wrapper
src/plugins/FFGLPlugin.cpp
src/plugins/FFGLEffect.h      — Adapts an FFGL plugin to our Effect interface
src/plugins/FFGLEffect.cpp
```

### 6.3 FFGLHost (Plugin Manager)

```cpp
class FFGLHost
{
public:
    FFGLHost();
    ~FFGLHost();

    // Scan directories for FFGL plugins
    void scanDirectory(const juce::File& dir);
    void scanDefaultDirectories();

    // Get discovered plugins
    struct PluginInfo {
        juce::File path;
        std::string name;
        std::string uniqueId;
        int numParams;
        bool isSource; // true = generator, false = effect
    };
    const std::vector<PluginInfo>& getAvailablePlugins() const;

    // Instantiate a plugin
    std::unique_ptr<FFGLPlugin> instantiate(const PluginInfo& info,
                                             int width, int height);

private:
    std::vector<PluginInfo> plugins_;

    // Platform-specific plugin scanning
    bool probePlugin(const juce::File& path, PluginInfo& info);
};
```

### 6.4 FFGLPlugin (Instance Wrapper)

```cpp
class FFGLPlugin
{
public:
    FFGLPlugin(void* moduleHandle, int width, int height);
    ~FFGLPlugin();

    // Plugin info
    std::string getName() const;
    int getNumParams() const;
    std::string getParamName(int index) const;
    float getParamValue(int index) const;
    void setParamValue(int index, float value);

    // Processing — takes input texture, returns output texture
    GLuint process(GLuint inputTexture);

    // For source plugins — no input texture
    GLuint generate();

    bool isValid() const;

private:
    // FFGL function pointers (loaded from dynamic library)
    // These correspond to the FFGL 2.x API:
    //   plugMain(opCode, inputVal, instanceID)
    void* moduleHandle_ = nullptr;
    void* instanceHandle_ = nullptr;
    int width_, height_;

    // Function pointer type
    using PlugMainFunc = void*(*)(unsigned int, unsigned int, unsigned int);
    PlugMainFunc plugMain_ = nullptr;

    // Internal FBO for receiving plugin output
    GLuint fbo_ = 0;
    GLuint outputTexture_ = 0;
};
```

### 6.5 FFGLEffect (Integration with Effect System)

```cpp
class FFGLEffect : public Effect
{
public:
    FFGLEffect(std::unique_ptr<FFGLPlugin> plugin);

    // Effect interface
    void apply(GLuint inputTexture, GLuint outputFBO) override;
    int getNumParams() const override;
    EffectParam& getParam(int index) override;
    std::string getName() const override;

private:
    std::unique_ptr<FFGLPlugin> plugin_;
    std::vector<EffectParam> params_; // synced from plugin
};
```

### 6.6 Default Plugin Directories

| Platform | Default Scan Path |
|----------|------------------|
| macOS | `/Library/FreeFrame/`, `~/Library/FreeFrame/` |
| Windows | `C:\Program Files\FreeFrame\`, `%APPDATA%\FreeFrame\` |
| Linux | `/usr/lib/freeframe/`, `~/.freeframe/` |

Plus the app's own `plugins/` subdirectory.

### 6.7 UI Integration

**FX Browser** → new sub-tab or category **"Plugins"**:
- Shows discovered FFGL plugins organized by folder
- Drag onto clip or effect stack like any built-in effect
- Parameters appear in the inspector as standard sliders with signal triangles

**Preferences** → new section **"Plugins"**:
- Plugin scan directories (add/remove)
- "Rescan" button
- Plugin blacklist (disable problematic plugins)

### 6.8 Safety Considerations

FFGL plugins are third-party code running in our process. Safety measures:

1. **Crash isolation**: Wrap all FFGL calls in try/catch. If a plugin crashes,
   disable it and log a warning rather than crashing the host.
2. **Timeout**: If a plugin's `process()` takes >100ms, skip it for that frame
   and mark it as slow.
3. **GL state protection**: Save/restore GL state before/after each plugin call
   (`glPushAttrib`/`glPopAttrib` or manual save/restore of blend, viewport,
   shader, FBO state).
4. **Validation**: On first load, probe the plugin in a test context before
   adding to the available list.

### 6.9 FFGL 2.x API Reference (Key Op Codes)

| OpCode | Name | Purpose |
|--------|------|---------|
| 0 | FF_GETINFO | Get plugin name, type, version |
| 1 | FF_INITIALISE | Global init (called once) |
| 2 | FF_DEINITIALISE | Global cleanup |
| 3 | FF_PROCESSFRAME | Process a frame (FFGL 1.x, CPU) |
| 4 | FF_GETNUMPARAMETERS | Get parameter count |
| 5 | FF_GETPARAMETERNAME | Get parameter name |
| 6 | FF_GETPARAMETERDEFAULT | Get parameter default |
| 11 | FF_INSTANTIATE | Create instance |
| 12 | FF_DEINSTANTIATE | Destroy instance |
| 13 | FF_GETPARAMETER | Get current value |
| 14 | FF_SETPARAMETER | Set value |
| 17 | FF_PROCESSFRAMECOPY | Process with separate in/out (FFGL 1.x) |
| 18 | FF_PROCESSOPENGL | Process OpenGL texture (FFGL 2.x) |
| 32 | FF_SETTIME | Set current time |
| 33 | FF_CONNECT | Connect texture input |

---

## 7. Unified Build Plan

All content from Sections 2-6 (ArKaos parity) and Section 10 (creative expansion)
organized into implementation sprints. Batched by complexity tier so each sprint
delivers a complete, testable set of content.

### Complexity Tiers

| Tier | Description | Per-item effort | Architecture needed |
|------|-------------|----------------|-------------------|
| **T1** | Simple fragment shader (single pass, no state) | 30-60 min | None — shader + register |
| **T2** | Medium shader (multi-sample, ray march, or audio uniforms) | 1-2 hours | May need FeatureSnapshot uniforms |
| **T3** | Stateful source (ping-pong FBO) | 2-3 hours | Uses existing stateful infra |
| **T4** | System feature (new classes, pipeline changes) | 1-3 days | Architectural work |

### Process Per Shader (T1-T3)

```
1. Write GLSL in EmbeddedShaders.h (inline const char*)
2. Register in EffectLibrary.cpp (effects) or SourceRegistry.cpp (sources)
3. Build + visual verify (load, sweep params)
4. Done — UI, mapping, presets all automatic
```

---

### Sprint 1: Core Effects + Simple Sources (T1)
**Effort**: 2 sessions
**Dependencies**: None
**Delivers**: 9 new effects + 10 simple sources = 19 new shaders

#### 1A — ArKaos Effects (8 + 1 upgrade)

| # | Name | Type | Category | Tier |
|---|------|------|----------|------|
| 1 | Tile Grid | Effect | warp | T1 |
| 2 | Infinite Zoom | Effect | warp | T1 |
| 3 | Spot Zoom | Effect | warp | T1 |
| 4 | Neon Edge | Effect | pattern | T1 |
| 5 | Cartoon Ink | Effect | pattern | T1 |
| 6 | Bump Light | Effect | pattern | T2 |
| 7 | Pop Raster | Effect | pattern | T1 |
| 8 | Screen Split | Effect | glitch | T1 |
| 9 | Strobe (upgrade) | Effect | animation | T1 |

#### 1B — Simple Geometric Sources

| # | Name | Type | Category | Tier | Section |
|---|------|------|----------|------|---------|
| 10 | Concentric Rings | Source | Geometric | T1 | C.1 |
| 11 | Radial Burst | Source | Geometric | T1 | C.2 |
| 12 | Hex Grid | Source | Geometric | T1 | C.3 |
| 13 | Sacred Geometry | Source | Geometric | T1 | C.6 |
| 14 | Dot Matrix Wave | Source | Geometric | T1 | C.5 |
| 15 | Astral Grid | Source | Geometric | T1 | B.3 |
| 16 | Color Bars | Source | Test | T1 | ArKaos |
| 17 | Lissajous Weaver | Source | Math | T1 | A.3 |
| 18 | Fermat Spiral Garden | Source | Math | T1 | A.4 |
| 19 | Moire Interference | Source | Geometric | T1 | C.4 |

**Validation**: Load each source in a clip cell. Verify rendering. Map RMS to
one parameter per source, verify audio reactivity. Check 60fps at 1080p.

---

### Sprint 2: Medium Sources + Nature + Lighting (T1-T2)
**Effort**: 2 sessions
**Dependencies**: None (parallel with Sprint 1 if desired)
**Delivers**: 15 new sources

#### 2A — Nature & Organic

| # | Name | Type | Tier | Section |
|---|------|------|------|---------|
| 20 | Fire Wall | Source | T1 | F.1 |
| 21 | Water Caustics | Source | T1 | F.2 |
| 22 | Cloud Drift | Source | T1 | F.3 |
| 23 | Electric Arc | Source | T2 | F.4 |

#### 2B — Concert & Club Lighting

| # | Name | Type | Tier | Section |
|---|------|------|------|---------|
| 24 | Laser Scanner | Source | T2 | E.2 |
| 25 | Strobe Bank | Source | T1 | E.3 |
| 26 | Moving Head Beam Array | Source | T2 | E.1 |

#### 2C — 3D Environments (Ray Marching)

| # | Name | Type | Tier | Section |
|---|------|------|------|---------|
| 27 | Infinite Corridor | Source | T2 | B.2 |
| 28 | Crystal Cavern | Source | T2 | B.1 |
| 29 | Orbit Chamber | Source | T2 | B.4 |
| 30 | Scroll Plane | Source | T1 | ArKaos |
| 31 | Rotating Cube Map | Source | T2 | ArKaos |
| 32 | Dual Plane Drift | Source | T1 | ArKaos |

#### 2D — Mathematical

| # | Name | Type | Tier | Section |
|---|------|------|------|---------|
| 33 | Hyperbolic Tiling | Source | T2 | A.2 |
| 34 | Penrose Pulse | Source | T2 | A.5 |

**Validation**: Same as Sprint 1. Extra attention to ray-marched sources —
verify they stay under 3ms at 1080p (may need iteration count limits).

---

### Sprint 3: Audio-Native Sources (T2)
**Effort**: 1 session
**Dependencies**: Requires FeatureSnapshot uniform passthrough in sources
**Delivers**: 5 audio-native sources + 2 typography sources = 7 sources

These sources are unique to Audio-DNA — they consume our 42+ audio features
directly. They need `uploadUniforms()` overrides to bind FeatureSnapshot fields.

#### 3A — Audio Visualizations

| # | Name | Type | Tier | Section | Audio Features Used |
|---|------|------|------|---------|-------------------|
| 35 | Spectrum Landscape | Source | T2 | D.1 | FFT bins (1025), spectral centroid |
| 36 | Chromatic Ring | Source | T2 | D.2 | chromagram[12], detectedKey, HCDF |
| 37 | Band Tower | Source | T2 | D.3 | bandEnergies[7], spectralCentroid |
| 38 | Timbral Nebula | Source | T2 | D.4 | mfccs[13], spectralCentroid, HCDF |
| 39 | Structural Landscape | Source | T2 | D.5 | structuralState, rms, spectralFlux, bpm |

#### 3B — Typography & Data

| # | Name | Type | Tier | Section | Notes |
|---|------|------|------|---------|-------|
| 40 | Scrolling Text Wall | Source | T2 | G.1 | Needs bitmap font texture atlas |
| 41 | Data Rain | Source | T2 | G.2 | Renders actual FeatureSnapshot values |

**Pre-work**: Create a small 8x8 bitmap font texture (128 ASCII chars) and
embed it as a static array in EmbeddedShaders.h. Both text sources use this.

**Validation**: Play music. Verify each source responds to its target features.
Spectrum Landscape should show the FFT. Chromatic Ring should highlight the
detected key. Timbral Nebula shape should change between vocals and drums.

---

### Sprint 4: Stateful Sources (T3)
**Effort**: 1 session
**Dependencies**: Existing ping-pong FBO infrastructure (same as Reaction-Diffusion)
**Delivers**: 3 stateful sources

| # | Name | Type | Tier | Section | FBO Pairs Needed |
|---|------|------|------|---------|-----------------|
| 42 | Strange Attractor Field | Source (stateful) | T3 | A.1 | 1 (trail persistence) |
| 43 | Gravity Well | Source (stateful) | T3 | H.1 | 1 (particle positions) |
| 44 | Fluid Dynamics | Source (stateful) | T3 | H.2 | 3 (velocity, pressure, dye) |

**Implementation notes**:
- Strange Attractor: Store particle trail texture in ping-pong FBO. Each frame:
  read previous trails, fade slightly, add new attractor points.
- Gravity Well: Store particle position/velocity in a texture. Each frame:
  compute gravity forces, update positions, render as point sprites.
- Fluid Dynamics: 3 FBO pairs. Advection → diffusion → pressure solve (Jacobi
  iteration, ~20 passes) → project. Most expensive source — may need 512x512
  internal resolution upscaled to output.

**Validation**: Verify state persistence across frames (trails don't reset).
Verify audio injection (onset → splash in fluid, bass → gravity pulse).
Performance target: <5ms per frame at 512x512 internal resolution.

---

### Sprint 5: Feedback Loop System (T4)
**Effort**: 2-3 sessions
**Dependencies**: CompositorEngine, Layer model, LayerInspector
**Delivers**: Complete feedback loop system with presets

| Task | Description | Files |
|------|-------------|-------|
| 5.1 | Create FeedbackProcessor class (FBO pair + transform shader) | `src/render/FeedbackProcessor.h/cpp` |
| 5.2 | Add FeedbackConfig to Layer data model + serialization | `src/model/Layer.h/cpp` |
| 5.3 | Integrate into CompositorEngine render pipeline | `src/render/CompositorEngine.h/cpp` |
| 5.4 | Add Feedback section to LayerInspector (6 params + signal triangles) | `src/ui/LayerInspector.h/cpp` |
| 5.5 | Add feedback shader to EmbeddedShaders.h | `src/render/EmbeddedShaders.h` |
| 5.6 | Implement 6 presets (Zoom In, Spiral, Drift, Kaleidoscope, Echo, Stretch) | `src/render/FeedbackProcessor.cpp` |
| 5.7 | Test: Infinite Zoom effect + Feedback = classic Larsen tunnel | Manual testing |
| 5.8 | Test: Audio-driven feedback amount (RMS → feedback intensity) | Manual testing |

**This is the highest-impact system addition** — it unlocks an entire category
of recursive/generative visuals that no parameter can achieve alone.

---

### Sprint 6: Per-Type Automation (T4)
**Effort**: 1-2 sessions
**Dependencies**: Autopilot engine, Composition model, CompositionInspector
**Delivers**: Intelligent per-type clip cycling

| Task | Description | Files |
|------|-------------|-------|
| 6.1 | Add PerTypeAutopilotConfig to Composition data model | `src/model/Composition.h` |
| 6.2 | Refactor Autopilot::advance() for per-type tracking | `src/model/Autopilot.h/cpp` |
| 6.3 | Add per-type UI to CompositionInspector (3 subsections) | `src/ui/CompositionInspector.h/cpp` |
| 6.4 | Serialize/deserialize per-type config in composition JSON | `src/model/Composition.cpp` |
| 6.5 | Test with mixed deck: opaque background + transparent overlays + FX | Manual testing |

---

### Sprint 7: FFGL Plugin Hosting (T4)
**Effort**: 3-4 sessions
**Dependencies**: Dynamic library loading, GL state management
**Delivers**: Third-party FFGL 2.x plugin support

| Task | Description | Files |
|------|-------------|-------|
| 7.1 | Implement FFGLHost (directory scanner, plugin probe) | `src/plugins/FFGLHost.h/cpp` |
| 7.2 | Implement FFGLPlugin (instance lifecycle, param get/set, process) | `src/plugins/FFGLPlugin.h/cpp` |
| 7.3 | Implement FFGLEffect (adapts plugin to our Effect interface) | `src/plugins/FFGLEffect.h/cpp` |
| 7.4 | Add "Plugins" category to FX Browser with folder organization | `src/ui/FXBrowser.h/cpp` |
| 7.5 | Add plugin preferences (scan dirs, rescan, blacklist) | `src/ui/PreferencesDialog.h/cpp` |
| 7.6 | GL state save/restore wrapper for plugin safety | `src/plugins/FFGLPlugin.cpp` |
| 7.7 | Crash isolation (try/catch, timeout, disable on failure) | `src/plugins/FFGLPlugin.cpp` |
| 7.8 | Test with Pete Warden FreeFrame pack + community FFGL plugins | Manual testing |
| 7.9 | Update CMakeLists.txt for dynamic loader libs (dlopen/LoadLibrary) | `CMakeLists.txt` |

---

### Dependency Graph

```
Sprint 1 (Effects + Simple Sources) ──────► No dependencies
Sprint 2 (Medium + Nature + 3D) ──────────► No dependencies
Sprint 3 (Audio-Native Sources) ──────────► FeatureSnapshot uniform passthrough
Sprint 4 (Stateful Sources) ──────────────► Existing ping-pong FBO infra
Sprint 5 (Feedback Loop) ────────────────► CompositorEngine + Layer model
Sprint 6 (Per-Type Automation) ──────────► Autopilot engine
Sprint 7 (FFGL Plugin Hosting) ──────────► Dynamic lib loading

Sprints 1-4 are all shader/source work — can be done in any order.
Sprints 5-7 are system work — independent of each other.

Recommended flow:
  [1 + 2] → [3 + 4] → [5] → [6] → [7]
  ~~~~~~~~   ~~~~~~~~   ~~~   ~~~   ~~~
  parallel   parallel   seq   seq   seq
```

### Master Sprint Schedule

| Sprint | Sessions | Delivers | Running Total |
|--------|----------|----------|---------------|
| **1** | 2 | 9 effects + 10 simple sources | 76 effects, 20 sources |
| **2** | 2 | 15 medium/3D sources | 76 effects, 35 sources |
| **3** | 1 | 7 audio-native sources | 76 effects, 42 sources |
| **4** | 1 | 3 stateful sources | 76 effects, 45 sources |
| **5** | 2-3 | Feedback loop system | 76 effects, 45 sources, feedback |
| **6** | 1-2 | Per-type automation | + intelligent automation |
| **7** | 3-4 | FFGL plugin hosting | + unlimited community effects |

**Total: ~13-15 sessions across 7 sprints**
**Final inventory: 76 effects + 45 sources + 3 systems + FFGL**

### Session = One Claude Code Conversation

Each "session" is one conversation with Claude. The instruction for each is:

- **Sprint 1**: "Kick off Sprint 1 — implement the 9 ArKaos effects and 10 simple
  geometric/math sources from archaosEffectSourceIntegration.md Section 7, Sprint 1."
- **Sprint 2**: "Kick off Sprint 2 — implement the 15 medium-complexity sources
  (nature, lighting, 3D, ArKaos geo, math) from the integration doc."
- **Sprint 3**: "Kick off Sprint 3 — implement the 7 audio-native and typography
  sources. These need FeatureSnapshot uniforms. Create the bitmap font texture."
- **Sprint 4**: "Kick off Sprint 4 — implement the 3 stateful sources (Strange
  Attractor, Gravity Well, Fluid Dynamics) using ping-pong FBOs."
- **Sprint 5**: "Kick off Sprint 5 — implement the Feedback Loop system (Section 4
  of the integration doc). FeedbackProcessor class, Layer integration, UI, presets."
- **Sprint 6**: "Kick off Sprint 6 — implement Per-Type Automation (Section 5).
  Opaque/Transparent/Effect cycling with independent timers."
- **Sprint 7**: "Kick off Sprint 7 — implement FFGL Plugin Hosting (Section 6).
  FFGLHost, FFGLPlugin, FFGLEffect, FX Browser integration, safety measures."

---

## 8. File Change Manifest

### New Files

| File | Purpose |
|------|---------|
| `src/render/FeedbackProcessor.h` | Feedback loop FBO management + transform shader |
| `src/render/FeedbackProcessor.cpp` | Implementation |
| `src/plugins/FFGLHost.h` | Plugin scanner + lifecycle |
| `src/plugins/FFGLHost.cpp` | Implementation |
| `src/plugins/FFGLPlugin.h` | Single plugin instance wrapper |
| `src/plugins/FFGLPlugin.cpp` | Implementation |
| `src/plugins/FFGLEffect.h` | Adapts FFGL plugin to Effect interface |
| `src/plugins/FFGLEffect.cpp` | Implementation |

### Modified Files

| File | Changes |
|------|---------|
| `src/render/EmbeddedShaders.h` | +8 new effect shaders, +4 new source shaders, +1 feedback shader, enhanced strobe |
| `src/effects/EffectLibrary.cpp` | Register 8 new effects + enhanced strobe |
| `src/sources/SourceRegistry.cpp` | Register 4 new sources |
| `src/model/Layer.h` | Add FeedbackConfig struct |
| `src/model/Layer.cpp` | Serialize/deserialize feedback config |
| `src/model/Composition.h` | Add PerTypeAutopilotConfig |
| `src/model/Autopilot.h/cpp` | Refactor for per-type advancement |
| `src/render/CompositorEngine.h/cpp` | Integrate FeedbackProcessor per-layer |
| `src/ui/LayerInspector.h/cpp` | Add Feedback section |
| `src/ui/CompositionInspector.h/cpp` | Add per-type autopilot UI |
| `src/ui/FXBrowser.h/cpp` | Add "Plugins" category |
| `src/ui/PreferencesDialog.h/cpp` | Add plugin scan directories |
| `CMakeLists.txt` | Add new source files, link dynamic loader libs |
| `CLAUDE.md` | Document new effects, sources, feedback, FFGL |

---

## 9. Testing Strategy

### Effect/Source Testing

For each new shader:
1. **Visual verification**: Load a test image, enable the effect, sweep each parameter 0→1
2. **Audio reactivity**: Map RMS to primary parameter, verify smooth response
3. **Performance**: Measure frame time with effect enabled (must stay under 2ms for 1080p)
4. **Edge cases**: All params at 0, all at 1, rapid parameter changes

### Feedback Loop Testing

1. **Basic feedback**: Enable on a layer with a static image. Verify recursive zoom-in effect.
2. **Parameter sweep**: Adjust amount from 0→1. At 0 = no feedback. At 1 = pure previous frame.
3. **Rotation**: Set rotation to 5 degrees. Verify spiral pattern forms.
4. **Audio-driven**: Map feedback amount to RMS. Verify feedback increases during loud passages.
5. **Multi-layer**: Enable feedback on 2 layers simultaneously. Verify no FBO conflicts.
6. **Performance**: Feedback adds 1 extra texture read + 1 extra render pass per layer. Budget: <1ms.

### Per-Type Automation Testing

1. **Classification**: Create deck with 3 layer types. Verify autopilot identifies them correctly.
2. **Cycle timing**: Set opaque=16 beats, transparent=8, effect=4. Verify each advances independently.
3. **Layer limits**: Set transparent max=2. Trigger 3 transparent clips. Verify oldest gets ejected.
4. **Randomization**: Enable random. Run for 50 cycles. Verify no clip plays twice before all play once.

### FFGL Testing

1. **Scan**: Place a test FFGL plugin in scan directory. Verify it appears in FX Browser.
2. **Load**: Drag plugin onto clip. Verify parameters appear in inspector.
3. **Process**: Verify plugin output is visually correct.
4. **Crash safety**: Load a known-bad plugin. Verify app doesn't crash (plugin disabled + warning).
5. **Performance**: Verify plugin processing adds <3ms per frame.

---

## Appendix: ArKaos Effect → Audio-DNA Mapping (Complete)

| # | ArKaos Effect | Our Name | Status | Category |
|---|--------------|----------|--------|----------|
| 1 | Ripple | Ripple | HAVE | warp |
| 2 | Basic Transformations | Mirror | HAVE | glitch |
| 3 | Tunnel | Geometric Tunnel | HAVE (source) | geometric |
| 4 | Tiling | **Tile Grid** | NEW | warp |
| 5 | Scroller | **Scroll Plane** | NEW (source) | geometric |
| 6 | Screen Room | Sphere Wrap | HAVE | 3d |
| 7 | RotoZoom | **Infinite Zoom** | NEW | warp |
| 8 | Plane Rotation | Perspective Tilt | HAVE | 3d |
| 9 | Plane | **Dual Plane Drift** | NEW (source) | geometric |
| 10 | Cube Inside | **Rotating Cube Map** | NEW (source) | geometric |
| 11 | 3D Surface | Transform section | HAVE | n/a |
| 12 | 3D Objects (5) | Cylinder/Sphere Wrap | PARTIAL | 3d |
| 13 | Threshold | Posterize | HAVE | color |
| 14 | Sepia Tone | Sepia | HAVE | color |
| 15 | RGB | Gamma Levels | HAVE | color |
| 16 | Pop Art | **Pop Raster** | NEW | pattern |
| 17 | Mirror | Mirror | HAVE | glitch |
| 18 | Irisation | Chromatic Aberration | HAVE | color |
| 19 | Invert | Invert | HAVE | color |
| 20 | Hue | Hue Shift + Saturation | HAVE | color |
| 21 | Grayscale | Saturation (at 0) | HAVE | color |
| 22 | Gaussian Blur | Gaussian Blur | HAVE | blur |
| 23 | Directional Blur | Motion Blur | HAVE | blur |
| 24 | ASCII Art | ASCII Art | HAVE | pattern |
| 25 | Iris (transition) | MixMode transitions | HAVE | transition |
| 26 | Turnix3D | MixMode transitions | HAVE | transition |
| 27 | Scroller Transition | MixMode transitions | HAVE | transition |
| 28 | Slide | MixMode transitions | HAVE | transition |
| 29 | Shutter | MixMode transitions | HAVE | transition |
| 30 | Fade to Black | MixMode transitions | HAVE | transition |
| 31 | Waveform | Audio Waveform | HAVE (source) | audio-visual |
| 32 | Stroboscope | Strobe | **ENHANCE** | animation |
| 33 | Larsen (3 variants) | **Feedback Loop** | NEW (system) | layer feature |
| 34 | Color Bars | **Color Bars** | NEW (source) | test |
| 35 | Bumpy Surface | **Bump Light** | NEW | pattern |
| 36 | Neon | **Neon Edge** | NEW | pattern |
| 37 | Halftone | Color Halftone | HAVE | color |
| 38 | Cartoon | **Cartoon Ink** | NEW | pattern |
| 39 | Digital Noiz | Noise | HAVE | glitch |
| 40 | Edge Detect | Edge Detect | HAVE | blur |
| 41 | ArKolor | Selective Color | HAVE (partial) | color |
| 42 | Motion Blur | Motion Blur | HAVE | blur |
| 43 | Video Split | **Screen Split** | NEW | glitch |
| 44 | Target | **Spot Zoom** | NEW | warp |
| 45 | Contrast | Contrast | HAVE | color |
| 46 | Transition | MixMode transitions | HAVE | transition |
| 47 | Directional Trans. | MixMode transitions | HAVE | transition |
| 48 | Filter | Various color effects | HAVE | color |

**Coverage**: 48 ArKaos features → 30 already implemented, 13 new (this plan), 5 covered by existing systems.
**After implementation**: 100% ArKaos parity + our 42+ audio features advantage.

---

## 10. Creative Expansion — Beyond ArKaos

Everything above achieves ArKaos parity. This section goes beyond — effects and
sources that no VJ tool has, that exploit our 42+ audio features, and that push
into territory that makes performers say "how is it doing that?"

Organized by creative category. Each entry includes implementation complexity,
audio mapping potential (our unique advantage), and visual description.

---

### CATEGORY A: Mathematical Beauty

These sources generate visuals from pure mathematics. They produce the kind of
imagery that makes people stop and stare — infinite detail, perfect symmetry,
organic complexity from simple equations.

---

#### A.1 Strange Attractor Field

**Type**: Source (generator)
**Complexity**: Medium
**What it does**: Renders a strange attractor (Lorenz, Rössler, Halvorsen, Thomas,
Aizawa, or Dadras) as a glowing particle trail in 3D space, projected onto 2D.
Thousands of points trace the attractor's orbit, leaving luminous trails that
fade over time. The attractor rotates slowly in 3D for a living sculpture feel.

**Why it's special**: Strange attractors are chaos made visible. They never repeat,
never settle, but always stay bounded — the visual equivalent of improvised music.
No VJ tool renders these in real-time.

**Parameters**:
| Name | Range | Description |
|------|-------|-------------|
| Attractor Type | 0-5 (stepped) | Lorenz / Rössler / Halvorsen / Thomas / Aizawa / Dadras |
| Speed | 0-1 | Integration step size (slower = smoother, faster = more chaotic) |
| Trail Length | 0-1 | How long trails persist (fade time) |
| Rotation | 0-1 | 3D rotation speed |
| Glow | 0-1 | Trail brightness and bloom |
| Color Mode | 0-1 | 0=velocity-mapped, 0.5=position-mapped, 1=fixed hue |

**Audio mapping**: Speed driven by BPM (attractor orbits in time with music).
Trail length by RMS (louder = longer trails). Color mode shifts with detected key.
Attractor type switches on structural transitions (drop = Lorenz, breakdown = Thomas).

**GLSL approach**: Compute attractor integration in the fragment shader using
the pixel position as a seed for which orbit point to render. Store trail positions
in a texture (ping-pong FBO) for persistence. Project 3D→2D with perspective
matrix. Additive blending for glow.

---

#### A.2 Hyperbolic Tiling

**Type**: Source (generator)
**Complexity**: Medium
**What it does**: Generates Escher-like hyperbolic tilings — the Poincaré disk model
where identical shapes tessellate a curved space, shrinking toward the boundary
in an infinite fractal pattern. Supports triangle, square, and hexagonal base
tilings with configurable symmetry order.

**Why it's special**: Hyperbolic geometry is visually hypnotic — infinite repetition
that curves inward. It's mathematically deep but looks organic and kaleidoscopic.
Animated rotation in hyperbolic space creates a liquid, impossible motion.

**Parameters**:
| Name | Range | Description |
|------|-------|-------------|
| P (polygon sides) | 3-8 (stepped) | Base polygon type |
| Q (vertex order) | 3-8 (stepped) | How many polygons meet at each vertex |
| Rotation | 0-1 | Rotation in hyperbolic space (Möbius transform) |
| Zoom | 0-1 | Zoom into the disk |
| Color Scheme | 0-1 | Alternating colors, gradient, or audio-driven |
| Line Width | 0-1 | Edge thickness (0 = filled, 1 = wireframe) |

**Audio mapping**: Rotation by bar phase (slow, hypnotic orbit). Zoom pulsed by
beat phase. Color scheme driven by chromagram (musical color mapping). P/Q
values changed by structural state for dramatic shifts.

**GLSL approach**: Implement Poincaré disk Möbius transformations. For each pixel,
determine which tile it belongs to via repeated reflections in the fundamental
domain. Color based on tile index. The math is well-documented — the key shader
operations are complex number multiplication and circle inversion.

---

#### A.3 Lissajous Weaver

**Type**: Source (generator)
**Complexity**: Simple
**What it does**: Draws Lissajous curves (the patterns you see on oscilloscopes when
two sine waves are fed to X and Y axes). Multiple curves with different frequency
ratios weave together, creating harmonograph-like patterns. The frequency ratios
can be locked to musical intervals (octave = 2:1, fifth = 3:2, etc.).

**Why it's special**: Lissajous figures are literally the visual shape of musical
intervals. When driven by our detected pitch and key, the patterns ARE the music
made visible — the most direct audio-visual correspondence possible.

**Parameters**:
| Name | Range | Description |
|------|-------|-------------|
| Frequency X | 0-1 | X oscillation frequency (maps to 1-8) |
| Frequency Y | 0-1 | Y oscillation frequency (maps to 1-8) |
| Phase | 0-1 | Phase offset between X and Y |
| Decay | 0-1 | Trail fade rate |
| Harmonics | 0-1 | Number of overlaid curves (1-8) |
| Thickness | 0-1 | Line thickness with glow |

**Audio mapping**: Frequency X from dominant pitch (note → ratio). Phase from
beat phase (rotating figure). Harmonics from spectral flatness (tonal = few
clean curves, noisy = many chaotic curves). Color from detected key.

---

#### A.4 Fermat Spiral Garden

**Type**: Source (generator)
**Complexity**: Simple
**What it does**: Generates a Fermat spiral (golden ratio spiral) populated with
shapes — dots, petals, or geometric forms — arranged in the phyllotaxis pattern
seen in sunflowers and pinecones. Shapes grow outward from center, rotate, and
pulse. It's nature's favorite arrangement visualized.

**Parameters**:
| Name | Range | Description |
|------|-------|-------------|
| Count | 0-1 | Number of elements (50-500) |
| Divergence Angle | 0-1 | Angle between elements (0.5 = golden angle 137.5°) |
| Shape | 0-3 (stepped) | Circle / Square / Triangle / Petal |
| Growth | 0-1 | Outward expansion speed |
| Pulse | 0-1 | Size modulation amount |
| Color Spread | 0-1 | Color variation across the spiral |

**Audio mapping**: Count driven by transient density (more hits = more particles).
Divergence angle by pitch (different notes create different spiral patterns).
Pulse locked to beat phase. Growth by RMS.

---

#### A.5 Penrose Pulse

**Type**: Source (generator)
**Complexity**: Medium
**What it does**: Renders a Penrose tiling — the famous non-periodic tiling with
5-fold symmetry that never repeats. Tiles pulse with energy, with waves of
color rippling outward from the center. The tiling itself can morph between
different Penrose configurations.

**Parameters**:
| Name | Range | Description |
|------|-------|-------------|
| Generation | 0-1 | Subdivision depth (1-7) |
| Ripple Speed | 0-1 | Wavefront propagation speed |
| Color Mode | 0-1 | 0=two-color, 0.5=rainbow, 1=audio-mapped |
| Edge Glow | 0-1 | Brightness of tile edges |
| Morph | 0-1 | Interpolate between tiling variants |

**Audio mapping**: Ripple triggered by onset detection. Color by chromagram.
Edge glow by spectral flux. Generation level by structural state (more complex
during drops).

---

### CATEGORY B: Immersive 3D Environments

Ray-marched scenes that create the feeling of being inside a space.
All single-pass fragment shaders using signed distance functions (SDF).

---

#### B.1 Crystal Cavern

**Type**: Source (generator)
**Complexity**: Medium
**What it does**: Ray-marches through an infinite crystal cave. Faceted, reflective
surfaces catch a moving light source. The cave geometry is generated from
folded space (Menger sponge variant with random offsets). The camera flies
forward through the cave endlessly.

**Parameters**:
| Name | Range | Description |
|------|-------|-------------|
| Speed | 0-1 | Camera movement speed |
| Crystal Size | 0-1 | Scale of crystal formations |
| Reflectivity | 0-1 | Surface mirror quality |
| Light Color | 0-1 | Light hue (color wheel) |
| Fog Density | 0-1 | Atmospheric depth fog |
| Complexity | 0-1 | Fractal iteration count |

**Audio mapping**: Speed by BPM (fly through in time with music). Crystal size
pulsed by bass energy. Light color by detected key. Fog density by spectral
flatness (noisy = thick fog, tonal = clear).

---

#### B.2 Infinite Corridor

**Type**: Source (generator)
**Complexity**: Simple-Medium
**What it does**: An endless hallway stretching to a vanishing point. Walls, floor,
and ceiling tile with a pattern. Lights flash rhythmically along the corridor.
Think sci-fi spaceship interior or a brutalist concrete tunnel. Camera moves
forward or oscillates.

**Parameters**:
| Name | Range | Description |
|------|-------|-------------|
| Speed | 0-1 | Forward camera velocity |
| Width | 0-1 | Corridor width |
| Wall Pattern | 0-3 (stepped) | Grid / Brick / Ribbed / Smooth |
| Light Spacing | 0-1 | Distance between ceiling lights |
| Light Intensity | 0-1 | Brightness of rhythmic lights |
| Color | 0-1 | Wall and light color hue |

**Audio mapping**: Speed by BPM. Lights flash on onset detection (each beat
triggers the next light). Color by detected key. Width oscillates with bar phase
(breathing corridor).

---

#### B.3 Astral Grid

**Type**: Source (generator)
**Complexity**: Simple
**What it does**: An infinite 3D wireframe grid extending in all directions,
rendered in perspective. Think Tron's digital landscape or a vaporwave grid.
The grid plane tilts, scrolls, and can warp (sine deformation on the Y axis
creates rolling hills). Grid intersections can glow.

**Parameters**:
| Name | Range | Description |
|------|-------|-------------|
| Grid Size | 0-1 | Spacing between grid lines |
| Scroll Speed | 0-1 | Grid movement speed |
| Tilt | 0-1 | Camera angle (0=overhead, 1=horizon) |
| Warp | 0-1 | Sine wave terrain deformation |
| Glow | 0-1 | Line brightness and bloom |
| Horizon Color | 0-1 | Background gradient color |

**Audio mapping**: Scroll speed locked to BPM. Warp amplitude by bass energy
(bass makes the grid bounce). Glow pulsed by beat. Grid size by bar count
(evolves over phrases).

---

#### B.4 Orbit Chamber

**Type**: Source (generator)
**Complexity**: Medium
**What it does**: Multiple 3D geometric primitives (spheres, cubes, tori) orbit
a central point in a dark void, lit by a point light. Objects cast soft shadows
on each other. Each object can have a different material (chrome, glass, matte).
The orbital speeds and paths are driven by audio features.

**Parameters**:
| Name | Range | Description |
|------|-------|-------------|
| Object Count | 0-1 | Number of orbiting objects (1-8) |
| Object Type | 0-3 (stepped) | Sphere / Cube / Torus / Mixed |
| Orbit Speed | 0-1 | Orbital velocity |
| Orbit Radius | 0-1 | Distance from center |
| Material | 0-1 | 0=matte, 0.5=chrome, 1=glass |
| Light Orbit | 0-1 | Light source movement speed |

**Audio mapping**: Object count by band energies (each frequency band controls
one object's size). Orbit speed by BPM. Material shifts with structural state.
Light position by spectral centroid (brightness indicator = light position).

---

### CATEGORY C: Geometric & Grid Patterns

Clean, graphic, modernist patterns. These are the bread and butter of
minimal/techno VJ sets — simple shapes that look incredible when driven
by precise audio analysis.

---

#### C.1 Concentric Rings

**Type**: Source (generator)
**Complexity**: Simple
**What it does**: Rings expanding outward from center. Ring width, spacing, and
color alternate. Can be smooth or hard-edged. Expansion speed syncs to BPM.
Individual rings can pulse independently based on different frequency bands.

**Parameters**:
| Name | Range | Description |
|------|-------|-------------|
| Ring Count | 0-1 | Number of visible rings |
| Expansion Speed | 0-1 | Outward movement speed |
| Width | 0-1 | Ring thickness |
| Gap | 0-1 | Space between rings |
| Color Alternate | 0-1 | Color variation between rings |
| Hard Edge | 0-1 | 0=smooth antialiased, 1=pixel-sharp |

**Audio mapping**: Expansion speed = BPM sync. Each ring's brightness = one of
the 7 frequency bands. Width pulsed by beat phase. Color alternate by bar phase.

---

#### C.2 Radial Burst

**Type**: Source (generator)
**Complexity**: Simple
**What it does**: Lines or beams radiating outward from a central point, like a
starburst or sunburst. Number of rays, rotation speed, and ray length are all
controllable. At high ray counts with glow, this creates a light explosion
effect perfect for drops and high-energy moments.

**Parameters**:
| Name | Range | Description |
|------|-------|-------------|
| Ray Count | 0-1 | Number of rays (4-64) |
| Length | 0-1 | Ray length from center |
| Rotation | 0-1 | Spin speed |
| Width | 0-1 | Ray thickness |
| Taper | 0-1 | 0=uniform width, 1=sharp at tips |
| Glow | 0-1 | Bloom around rays |

**Audio mapping**: Ray count by spectral centroid (brighter audio = more rays).
Length pulsed by onset strength. Rotation by bar phase. Width by bass energy.

---

#### C.3 Hex Grid

**Type**: Source (generator)
**Complexity**: Simple
**What it does**: A hexagonal grid covering the screen. Individual hexagons can
light up based on various patterns: random, wave, spiral, or audio-reactive
(each hex responds to a frequency band). Hex edges glow. Grid can zoom and
rotate. Think honeycomb meets LED wall.

**Parameters**:
| Name | Range | Description |
|------|-------|-------------|
| Cell Size | 0-1 | Hexagon radius |
| Pattern | 0-3 (stepped) | Random / Wave / Spiral / Audio |
| Fill | 0-1 | How many cells are lit (0=none, 1=all) |
| Edge Width | 0-1 | Hex border thickness |
| Rotation | 0-1 | Grid rotation angle |
| Color Mode | 0-1 | 0=monochrome, 0.5=rainbow, 1=audio-frequency |

**Audio mapping**: In "Audio" pattern mode, each hexagon ring maps to a frequency
band — center hex = sub bass, next ring = bass, etc. Fill pulsed by RMS. Edge
brightness by spectral flux.

---

#### C.4 Moire Interference

**Type**: Effect (or Source)
**Complexity**: Simple
**What it does**: Overlays two identical line/dot/ring patterns with a slight offset
or rotation difference, creating moiré interference patterns. The patterns shift
and breathe, creating large-scale emergent waves from fine details. Hypnotic
and mathematically elegant.

**Parameters**:
| Name | Range | Description |
|------|-------|-------------|
| Pattern | 0-2 (stepped) | Lines / Dots / Rings |
| Frequency | 0-1 | Pattern density |
| Offset X | 0-1 | Horizontal shift between layers |
| Offset Y | 0-1 | Vertical shift between layers |
| Rotation | 0-1 | Angular offset between layers |
| Zoom | 0-1 | Overall scale |

**Audio mapping**: Offset X/Y by spectral centroid and spectral rolloff (the
interference pattern shifts with audio brightness). Rotation by bar phase.
Frequency by BPM (faster BPM = denser pattern).

---

#### C.5 Dot Matrix Wave

**Type**: Source (generator)
**Complexity**: Simple
**What it does**: A grid of dots (like a Lite-Brite or LED display) where each dot's
size or brightness is driven by a wave equation. Multiple waves can interfere —
ripples from point sources create complex patterns. Each point source can be
audio-triggered (onset = new ripple from random position).

**Parameters**:
| Name | Range | Description |
|------|-------|-------------|
| Grid Density | 0-1 | Dots per row/column (8-64) |
| Wave Speed | 0-1 | Ripple propagation speed |
| Damping | 0-1 | How quickly waves die out |
| Dot Size | 0-1 | Base dot radius |
| Color | 0-1 | Dot color hue |
| Sources | 0-1 | Number of simultaneous wave sources |

**Audio mapping**: Each onset detection spawns a new wave source at a position
derived from the spectral centroid (bright sounds = upper right, dark = lower
left). Wave speed by BPM. Damping by dynamic range.

---

#### C.6 Sacred Geometry

**Type**: Source (generator)
**Complexity**: Simple-Medium
**What it does**: Renders sacred geometry patterns: Flower of Life, Seed of Life,
Metatron's Cube, Sri Yantra, or Fibonacci spiral. Lines are drawn with glow.
Patterns can rotate, breathe (scale oscillation), and have individual elements
that pulse independently.

**Parameters**:
| Name | Range | Description |
|------|-------|-------------|
| Pattern | 0-4 (stepped) | Flower of Life / Seed / Metatron / Sri Yantra / Fibonacci |
| Rotation | 0-1 | Rotation speed |
| Breathe | 0-1 | Scale oscillation amount |
| Line Width | 0-1 | Drawing line thickness |
| Glow | 0-1 | Bloom around lines |
| Reveal | 0-1 | Animates drawing from 0% to 100% |

**Audio mapping**: Reveal driven by phrase phase (pattern draws itself over 8 bars,
then resets). Breathe by beat phase. Rotation by bar phase. Pattern switches on
structural transitions. Glow by RMS.

---

### CATEGORY D: Audio-Native Visualizations

These sources exist specifically because we have 42+ audio features.
They would be impossible or meaningless in Resolume (which only has L/M/H).

---

#### D.1 Spectrum Landscape

**Type**: Source (generator)
**Complexity**: Simple
**What it does**: The full 1025-bin FFT spectrum rendered as a 3D landscape —
frequency on X axis, amplitude on Y, and time scrolling on Z (waterfall display
in 3D). The camera views this from an angle, creating rolling mountains of
sound. Mountains glow at peaks.

**Parameters**:
| Name | Range | Description |
|------|-------|-------------|
| History Depth | 0-1 | How many past frames to show (trail length) |
| Height Scale | 0-1 | Vertical exaggeration |
| Camera Angle | 0-1 | View angle (0=overhead, 1=horizon) |
| Color Mode | 0-1 | 0=amplitude heat, 0.5=frequency rainbow, 1=key-mapped |
| Glow | 0-1 | Peak glow intensity |
| Smoothing | 0-1 | Temporal smoothing |

**Audio mapping**: This IS the audio — every parameter of the spectrum is visible.
Height scale by dynamic range. Camera angle by structural state (overhead in
breakdowns, dramatic angle during drops).

---

#### D.2 Chromatic Ring

**Type**: Source (generator)
**Complexity**: Simple
**What it does**: A circle divided into 12 segments, one per musical pitch class
(C through B). Each segment's brightness shows the real-time chromagram energy.
The dominant key is highlighted. Harmonic changes (HCDF) create ripple effects.
This is a real-time harmonic compass — you can SEE the music's harmonic content.

**Parameters**:
| Name | Range | Description |
|------|-------|-------------|
| Ring Width | 0-1 | Thickness of the chromatic ring |
| Glow | 0-1 | Bloom around bright segments |
| Rotation | 0-1 | Rotate ring (0 = C at top) |
| Show Labels | 0-1 | Note name labels visibility |
| Ripple | 0-1 | HCDF-triggered ripple intensity |
| Inner Display | 0-1 | 0=empty, 0.5=key text, 1=spectrum |

**Audio mapping**: Fully automatic — all 12 chromagram values drive the ring.
Detected key highlights the tonic segment. HCDF triggers ripples. Pitch
confidence controls overall brightness.

---

#### D.3 Band Tower

**Type**: Source (generator)
**Complexity**: Simple
**What it does**: Seven vertical columns, one per frequency band (Sub through
Brilliance), rendered as glowing towers that grow and shrink with band energy.
Towers can be bars, cylinders, or abstract shapes. They stand on a reflective
floor. A simple but powerful real-time frequency visualizer.

**Parameters**:
| Name | Range | Description |
|------|-------|-------------|
| Shape | 0-2 (stepped) | Bars / Cylinders / Abstract |
| Spacing | 0-1 | Gap between towers |
| Reflection | 0-1 | Floor reflection strength |
| Color Mode | 0-1 | 0=per-band unique, 0.5=gradient, 1=monochrome |
| Smoothing | 0-1 | Temporal smoothing |
| 3D Rotation | 0-1 | Viewing angle rotation |

**Audio mapping**: Fully automatic — 7 band energies drive 7 tower heights.
Color can map to spectral centroid (overall brightness hue). Reflection by
dynamic range.

---

#### D.4 Timbral Nebula

**Type**: Source (generator)
**Complexity**: Medium
**What it does**: Visualizes the 13 MFCCs (timbral fingerprint) as a cloud of
colored particles in a 2D space. MFCC values drive particle positions via
a dimensionality-reduction-like mapping. Different instruments and vocal
qualities produce different nebula shapes. Vocals form one shape, drums
another, synths another. It's a visual fingerprint of sound quality.

**Parameters**:
| Name | Range | Description |
|------|-------|-------------|
| Particle Count | 0-1 | Density of the nebula |
| Spread | 0-1 | How far apart features map |
| Trail Length | 0-1 | Particle trail persistence |
| Color Source | 0-1 | 0=spectral centroid, 0.5=pitch, 1=fixed |
| Glow | 0-1 | Particle bloom |
| Sensitivity | 0-1 | MFCC amplification |

**Audio mapping**: Fully automatic — 13 MFCCs drive particle distribution.
Spectral centroid drives color. RMS drives overall brightness. HCDF triggers
position jumps (new timbral event).

---

#### D.5 Structural Landscape

**Type**: Source (generator)
**Complexity**: Simple-Medium
**What it does**: A terrain that evolves based on the structural state detector.
During **normal** passages: gentle rolling hills with ambient color.
During **buildup**: terrain rises, clouds gather, colors intensify.
During **drop**: explosion of peaks, lightning, saturated colors.
During **breakdown**: terrain flattens, fog rolls in, desaturated cool tones.
The terrain remembers its history — you can see the "shape" of the song.

**Parameters**:
| Name | Range | Description |
|------|-------|-------------|
| Terrain Scale | 0-1 | Overall terrain zoom |
| History Length | 0-1 | How much song history is visible |
| Drama | 0-1 | How extreme the state-based changes are |
| Color Palette | 0-2 (stepped) | Earth / Neon / Minimal |
| Fog | 0-1 | Atmospheric depth |
| Camera Height | 0-1 | Viewing altitude |

**Audio mapping**: Fully automatic — structural state drives the entire mood.
RMS and spectral flux add fine detail to terrain height. BPM syncs the camera
movement speed.

---

### CATEGORY E: Concert & Club Lighting

Virtual reproductions of real-world lighting fixtures. These turn
Audio-DNA into a virtual lighting console.

---

#### E.1 Moving Head Beam Array

**Type**: Source (generator)
**Complexity**: Medium
**What it does**: Simulates an array of moving head spotlights (2-8 fixtures).
Each head has independent pan, tilt, color, and gobo (pattern wheel). Beams
cut through a haze layer. Beams can sweep, chase, or respond individually
to different frequency bands.

**Parameters**:
| Name | Range | Description |
|------|-------|-------------|
| Head Count | 0-1 | Number of fixtures (2-8) |
| Sweep Pattern | 0-3 (stepped) | Static / Sweep / Chase / Audio |
| Beam Width | 0-1 | Spot narrowness |
| Gobo | 0-3 (stepped) | None / Circle / Star / Cross |
| Haze Density | 0-1 | Volumetric haze visibility |
| Color Cycle | 0-1 | Color cycling speed |

**Audio mapping**: In "Audio" mode, each head tracks a frequency band —
Sub bass points down, brilliance points up. Sweep speed locked to BPM.
Color cycle by detected key. Haze pulsed by RMS.

---

#### E.2 Laser Scanner

**Type**: Source (generator)
**Complexity**: Simple-Medium
**What it does**: Simulates laser show patterns — straight beams, fans,
tunnels, cones, and abstract shapes drawn by galvanometer-style scanning.
Multiple laser colors (red, green, blue, white). Patterns can be geometric
(circles, squares, spirals) or reactive (audio waveform drawn by laser).

**Parameters**:
| Name | Range | Description |
|------|-------|-------------|
| Pattern | 0-5 (stepped) | Fan / Tunnel / Cone / Wave / Spiral / Abstract |
| Beam Count | 0-1 | Number of beams |
| Color | 0-1 | Laser color hue |
| Speed | 0-1 | Animation speed |
| Spread | 0-1 | Cone/fan angle |
| Flicker | 0-1 | Realistic galvo flicker |

**Audio mapping**: Pattern switches on structural transitions. Speed by BPM.
Spread by bass energy. In "Wave" mode, the laser traces the actual audio
waveform. Beam count by transient density.

---

#### E.3 Strobe Bank

**Type**: Source (generator)
**Complexity**: Simple
**What it does**: A grid of rectangular strobe panels (like Martin Atomic 3000s).
Each panel can fire independently or in patterns (chase, random, all-at-once).
Simulates the blinding intensity of real strobe banks with controllable
flash duration and color temperature.

**Parameters**:
| Name | Range | Description |
|------|-------|-------------|
| Panel Count | 0-1 | Grid size (1x1 to 4x4) |
| Pattern | 0-3 (stepped) | All / Chase / Random / Audio |
| Intensity | 0-1 | Flash brightness |
| Duration | 0-1 | Flash duration (shorter = sharper) |
| Temperature | 0-1 | Color temperature (warm to cool) |
| Rate | 0-1 | Flash rate (or BPM-synced) |

**Audio mapping**: Rate locked to BPM (or double/half time). Pattern by
structural state (all-at-once on drops, chase during builds). Intensity
by onset strength. Temperature by spectral centroid.

---

### CATEGORY F: Organic & Nature

Natural phenomena rendered procedurally. These create moods and
atmospheres rather than geometric precision.

---

#### F.1 Fire Wall

**Type**: Source (generator)
**Complexity**: Simple
**What it does**: Realistic procedural fire covering the screen. Uses FBM noise
with vertical flow, color-mapped through a fire palette (black → red → orange
→ yellow → white). Height, turbulence, and color temperature are controllable.
Can range from candle flame to inferno.

**Parameters**:
| Name | Range | Description |
|------|-------|-------------|
| Height | 0-1 | Flame height |
| Turbulence | 0-1 | Flame chaotic-ness |
| Speed | 0-1 | Rising speed |
| Temperature | 0-1 | Cool (blue) to hot (white) |
| Density | 0-1 | Flame thickness |
| Wind | 0-1 | Horizontal wind force |

**Audio mapping**: Height by RMS (louder = taller flames). Turbulence by
spectral flux (more change = more chaotic). Speed by BPM. Wind by
spectral centroid direction.

---

#### F.2 Water Caustics

**Type**: Source (generator)
**Complexity**: Simple
**What it does**: The light pattern you see at the bottom of a swimming pool —
dancing, refracting caustic patterns. Generated from overlapping sine waves
with different frequencies and directions. Beautiful, calming, hypnotic.

**Parameters**:
| Name | Range | Description |
|------|-------|-------------|
| Complexity | 0-1 | Number of overlapping wave sources |
| Speed | 0-1 | Wave animation speed |
| Brightness | 0-1 | Caustic intensity |
| Color | 0-1 | Water tint hue (0=blue, 0.5=green, 1=warm) |
| Distortion | 0-1 | Refraction strength |
| Scale | 0-1 | Pattern zoom |

**Audio mapping**: Speed by BPM. Complexity by transient density (more
hits = more chaotic water). Brightness pulsed by beat. Color by detected key.

---

#### F.3 Cloud Drift

**Type**: Source (generator)
**Complexity**: Simple
**What it does**: Volumetric-looking clouds generated from layered FBM noise
with atmospheric scattering simulation. Clouds drift across the screen,
lit from behind or above. Can range from wispy cirrus to dense cumulus
to dark storm clouds.

**Parameters**:
| Name | Range | Description |
|------|-------|-------------|
| Coverage | 0-1 | Cloud density (0=clear, 1=overcast) |
| Drift Speed | 0-1 | Movement speed |
| Altitude | 0-1 | Viewing angle (0=below, 1=above) |
| Light Angle | 0-1 | Sun/moon position |
| Storm | 0-1 | Darkness and turbulence |
| Color | 0-1 | 0=natural, 0.5=golden hour, 1=neon |

**Audio mapping**: Coverage by RMS (louder = more clouds). Storm by
structural state (buildup → gathering clouds, drop → storm). Drift
speed by BPM. Color by detected key.

---

#### F.4 Electric Arc

**Type**: Source (generator)
**Complexity**: Simple-Medium
**What it does**: Lightning bolts and electric arcs between configurable points.
Uses recursive midpoint displacement for the jagged lightning shape, with
glow and branching. Arcs can connect between fixed points, random points,
or track audio features.

**Parameters**:
| Name | Range | Description |
|------|-------|-------------|
| Arc Count | 0-1 | Number of simultaneous arcs (1-6) |
| Chaos | 0-1 | Jaggedness of the lightning path |
| Thickness | 0-1 | Main bolt width |
| Branches | 0-1 | Number of side branches |
| Glow | 0-1 | Electrical glow radius |
| Color | 0-1 | Arc color (0=white, 0.5=blue, 1=purple) |

**Audio mapping**: Arc count increases on onset detection (each hit = new
arc). Chaos by spectral flatness (noisy = chaotic). Thickness by onset
strength. New arc positions based on frequency band peaks.

---

### CATEGORY G: Typography & Data

Text and information-based visuals. Unusual in VJ tools but powerful
for branded events and data art.

---

#### G.1 Scrolling Text Wall

**Type**: Source (generator)
**Complexity**: Medium (requires text rendering)
**What it does**: Matrix-rain style falling characters, but configurable — can
be custom text, random characters, binary, hex, or emoji. Columns fall at
different speeds. Characters can glow, change color, and respond to audio.

**Parameters**:
| Name | Range | Description |
|------|-------|-------------|
| Character Set | 0-4 (stepped) | Matrix / Binary / Hex / ASCII / Custom |
| Speed | 0-1 | Fall speed |
| Density | 0-1 | Number of active columns |
| Color | 0-1 | Text color hue |
| Glow | 0-1 | Character glow amount |
| Font Size | 0-1 | Character size |

**Note**: Text rendering in GLSL is done via bitmap font texture atlas.
Ship a simple 8x8 bitmap font texture.

**Audio mapping**: Speed by BPM. Density by RMS. Individual column brightness
by frequency bands.

---

#### G.2 Data Rain

**Type**: Source (generator)
**Complexity**: Simple
**What it does**: Streams of numbers, percentages, and bar graphs cascading
down or scrolling horizontally — like a stock ticker or hacker terminal.
The numbers ARE the actual audio feature values, formatted in real-time.
Turns raw analysis data into visual art.

**Parameters**:
| Name | Range | Description |
|------|-------|-------------|
| Layout | 0-2 (stepped) | Cascade / Ticker / Grid |
| Data Source | 0-2 (stepped) | All Features / Spectrum / Rhythm |
| Speed | 0-1 | Scroll/fall speed |
| Opacity | 0-1 | Text transparency |
| Color | 0-1 | Text color |
| Font Size | 0-1 | Character scale |

**Audio mapping**: Fully automatic — the data IS the audio. BPM, RMS,
spectral centroid, key, MFCC values all rendered as scrolling numbers.

---

### CATEGORY H: Particle & Physics Systems

These require stateful rendering (ping-pong FBOs) but create some of
the most dynamic, organic, and responsive visuals possible.

---

#### H.1 Gravity Well

**Type**: Source (generator, **stateful**)
**Complexity**: Medium
**What it does**: Thousands of particles attracted to a central gravity well.
Particles orbit, collide, form accretion disks. When the bass hits, the well
pulses and particles scatter, then re-collect. Multiple gravity wells can exist.
Think: a visual black hole that breathes with the music.

**Parameters**:
| Name | Range | Description |
|------|-------|-------------|
| Particle Count | 0-1 | Density (256-4096 via texture width) |
| Gravity Strength | 0-1 | Pull toward center |
| Scatter | 0-1 | Outward force on onset |
| Trail Length | 0-1 | Particle trail persistence |
| Wells | 0-1 | Number of gravity sources (1-4) |
| Color Mode | 0-1 | 0=velocity, 0.5=distance, 1=fixed |

**Audio mapping**: Gravity strength by bass energy. Scatter triggered by onset
detection (beat hits = particle explosion). Well positions by chromagram peak
(dominant notes pull particles to different locations). Trail by RMS.

---

#### H.2 Fluid Dynamics

**Type**: Source (generator, **stateful**)
**Complexity**: Medium-Large
**What it does**: Real-time 2D fluid simulation (Navier-Stokes). Audio features
inject colored dye and velocity into the fluid. Bass creates vortices.
Onsets splash new color. The fluid swirls, mixes, and diffuses over time.
One of the most visually stunning effects possible — liquid music.

**Parameters**:
| Name | Range | Description |
|------|-------|-------------|
| Viscosity | 0-1 | Fluid thickness |
| Diffusion | 0-1 | Color spread rate |
| Injection Radius | 0-1 | Size of audio-driven injections |
| Color Mode | 0-1 | 0=audio frequency colors, 0.5=complementary, 1=monochrome |
| Curl | 0-1 | Vorticity confinement (more = tighter swirls) |
| Decay | 0-1 | How quickly the fluid fades |

**Audio mapping**: Bass injects blue dye from bottom. Mid injects green from
sides. Treble injects magenta from top. Onsets create velocity bursts.
Spectral centroid controls injection position. HCDF triggers color changes.

**Implementation**: Requires 3 ping-pong FBO pairs (velocity field, pressure
field, dye field). Jacobi iteration for pressure solve. ~4 render passes per
frame. Well-documented algorithm — Jos Stam's "Stable Fluids" method.

---

### Summary: Total New Content

| Category | Effects | Sources | Systems |
|----------|---------|---------|---------|
| ArKaos Integration (Section 2-6) | 8 | 4 | 3 |
| A: Mathematical Beauty | — | 5 | — |
| B: Immersive 3D | — | 4 | — |
| C: Geometric & Grid | 1 (Moire) | 5 | — |
| D: Audio-Native | — | 5 | — |
| E: Concert Lighting | — | 3 | — |
| F: Organic & Nature | — | 4 | — |
| G: Typography & Data | — | 2 | — |
| H: Particle & Physics | — | 2 (stateful) | — |
| **Total** | **9** | **34** | **3** |

Combined with our existing 67 effects + 10 sources, this would bring us to:

- **76 effects** (67 + 9 new)
- **48 sources** (10 + 4 ArKaos + 34 creative)
- **3 new systems** (Feedback Loop, Per-Type Automation, FFGL Hosting)

This would make Audio-DNA the most comprehensive audio-reactive visual
tool on the market — 42+ audio features driving 76 effects and 48 generative
sources, with professional feedback loop capabilities and FFGL plugin support.
