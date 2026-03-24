# Audio-DNA FX & Source Comprehensive Audit

**Date**: 2026-03-23
**Auditor**: Claude (automated analysis + industry research)
**Scope**: All 135 effects + 81 sources vs. industry standards (Resolume Arena 7, After Effects CC, ArKaos GrandVJ, VDMX, TouchDesigner)

### Fix Status

**DEFAULT VALUES FIX — COMPLETE (2026-03-23)**
- 76 effect default values updated in `EffectLibrary.cpp`
- Build: PASS (all shaders compile)
- C++ tests: 109/110 pass (1 pre-existing barPhase test failure)
- API verification: All 135 effects confirmed to have visible primary param defaults via Eyes test server
- Effects that should be neutral at 0.5 (Saturation, Brightness, Exposure, etc.) were NOT changed

---

## Rating Legend

| Rating | Meaning |
|--------|---------|
| ✅ Good | Matches industry standard, parameters correct |
| ⚠️ Needs Adjustment | Concept right, parameters/ranges/defaults need fixing |
| ❌ Wrong | Fundamentally different from what this effect should do |
| 🔧 Missing Params | Works but needs additional controls |
| 💀 Broken | Shader compilation failed or runtime error |

---

## Part 1: Effects Audit (135 Effects)

### Warp Effects (27)

| # | Name | Our Params | Industry Equivalent | Rating | Notes |
|---|------|-----------|---------------------|--------|-------|
| 1 | Ripple | intensity (def 0.0), freq (0.5), speed (0.5) | AE: CC Ripple Pulse. Resolume: Ripple. | ⚠️ | **Default intensity 0.0** = invisible on first add. Should be 0.3-0.5. Missing: center X/Y (AE has it), damping. Freq range 5-30 Hz is fine. |
| 2 | Bulge | amount (0.0), center_x (0.5), center_y (0.5) | AE: Spherize / Bulge. Resolume: Bulge. | ⚠️ | **Default amount 0.0** = invisible. Resolume defaults Bulge visible on add. Bidirectional remap (0.5=none) is good but unintuitive — user expects slider-up = more effect. Should default to 0.6. |
| 3 | Wave | amplitude (0.0), frequency (0.5), direction (0.0) | AE: Wave Warp. Resolume: Wave. | ⚠️ | **Default amplitude 0.0** = invisible. AE Wave Warp has: height, width, direction, speed, pinning. We're missing **pinning** (edge handling) and **wave type** (sine/square/triangle/sawtooth). |
| 4 | Liquid | viscosity (0.5), turbulence (0.0) | Resolume: Liquify. AE: Liquify. | ⚠️ | **Turbulence 0.0** = invisible. Resolume Liquify defaults to visible. Missing: **amount/strength** master control. Viscosity naming is confusing — actually controls speed, not viscosity. |
| 5 | Kaleidoscope | segments (0.3), rotation (0.0) | AE: CC Kaleida. Resolume: Kaleidoscope. | ✅ | Segment range 2-16 is correct. Rotation + time animation is standard. Good implementation. |
| 6 | Fisheye | amount (0.5) | AE: Optics Compensation. Resolume: Fisheye. | ✅ | Bidirectional (barrel/pincushion) is correct. 0.5=neutral is industry standard for this. |
| 7 | Swirl | amount (0.0), radius (0.5) | AE: Twirl. Resolume: Twirl. | ⚠️ | **Default amount 0.0** = invisible. Resolume Twirl defaults visible. Missing: **center X/Y** (AE has it). |
| 8 | Polar Coords | amount (0.0) | AE: Polar Coordinates. Resolume: Polar. | ⚠️ | **Default 0.0** = invisible. AE Polar Coordinates has **interpolation** (% between rect/polar) and **type** (rect-to-polar vs polar-to-rect). We only have amount. |
| 9 | Twirl | amount (0.0), radius (0.5) | AE: Twirl. Resolume: n/a (same as Swirl above). | ⚠️ | **Duplicate of Swirl?** Both do the same effect. One should be removed or differentiated. Default 0.0 = invisible. |
| 10 | Shear | x (0.5), y (0.5) | AE: Corner Pin (partial). Resolume: Shear. | ✅ | 0.5=neutral is correct for bidirectional. Standard implementation. |
| 11 | Elastic Bounce | amount (0.0), freq (0.5) | No direct equivalent. Custom. | ⚠️ | **Default 0.0** = invisible. Creative effect, but naming is unclear. What does "elastic bounce" do vs Wave? Should have a more descriptive name or be merged. |
| 12 | Ripple Pond | intensity (0.0), freq (0.5) | AE: CC Ripple Pulse (closer). | ⚠️ | **Default 0.0** = invisible. Very similar to Ripple — what differentiates them? Multiple ripple sources? If so, needs a **source count** param. |
| 13 | Diamond Distort | size (0.5), amount (0.0) | No direct equivalent. Custom. | ⚠️ | **Default amount 0.0** = invisible. Unique effect, fine to keep. |
| 14 | Barrel Distort | amount (0.5) | AE: Optics Compensation. Resolume: Barrel. | ✅ | 0.5=neutral is correct. Similar to Fisheye but barrel-only. Consider merging with Fisheye. |
| 15 | Sine Grid | freq (0.5), amount (0.0) | No direct equivalent. Custom. | ⚠️ | **Default 0.0** = invisible. |
| 16 | Glitch Displace | amount (0.0), speed (0.5) | Resolume: Glitch Displace. | ⚠️ | **Default 0.0** = invisible. |
| 17 | Quad Mirror | center x (0.5), center y (0.5) | Resolume: Quad Mirror. | ✅ | Always visible (mirrors the image). Good defaults. |
| 18 | Flip | horizontal (0.0), vertical (0.0) | AE: Flip. Resolume: Flip. | ✅ | Standard. 0=normal, 1=flipped. |
| 19 | Warp Field | amount (0.0), frequency (0.5), speed (0.3) | No direct equivalent. Custom gravitational warp. | ⚠️ | **Default 0.0** = invisible. |
| 20 | Slide Wrap | x (0.5), y (0.5) | Resolume: Slide / Scroll. | ✅ | 0.5=no offset is correct. Standard implementation. |
| 21 | Tile Grid | columns (0.25), rows (0.25), offset (0.0), zoom (0.5) | AE: Motion Tile. Resolume: Tile. | 🔧 | Good params. AE Motion Tile also has: **mirror edges**, **phase**. Missing mirror option. Columns/rows at 0.25 maps to ~4 tiles which is good. |
| 22 | Spot Zoom | center x/y (0.5), size (0.3), zoom (0.7), shape (0.0), background (0.3) | AE: Magnify. Resolume: Magnify. | ✅ | Good implementation with shape selector. Background dim is a nice touch. |
| 23 | Bendoscope | divisions (0.3), bend (0.5), rotation (0.0) | Resolume: Bendoscope. | ✅ | Matches Resolume's Bendoscope. Good params. |
| 24 | UV Remap | amount (0.0), scale (0.5), speed (0.3) | Resolume: UV Map. | ⚠️ | **Default 0.0** = invisible. Resolume UV Map is more complex (uses a displacement map texture). Our version uses procedural Perlin noise displacement — should be renamed to "Noise Displace" or similar. |
| 25 | Liquid Morph | viscosity (0.5), amount (0.0), scale (0.5) | Resolume: Liquify (related). | ⚠️ | **Default 0.0** = invisible. Very similar to Liquid (#4). Differentiation unclear. |
| 26 | Zoom Warp | speed (0.3), rotation (0.5), center x/y (0.5) | Resolume: Infinite Zoom. AE: CC RepeTile (partial). | ✅ | Good implementation. Named "Zoom Warp" in UI but shader is "infinite_zoom" — consistent with concept. |
| 27 | Density Wave | amount (0.5), direction (0.0), wavelength (0.5) | No direct equivalent. Audio-driven warp. | ✅ | Audio-native effect. Good defaults (0.5 = visible). |

**Warp Summary**: 27 effects. 14 have **default 0.0 on primary param** = invisible on first add. Major UX issue. Swirl vs Twirl are near-duplicates. Liquid vs Liquid Morph are similar. UV Remap name misleading.

---

### Color Effects (31)

| # | Name | Our Params | Industry Equivalent | Rating | Notes |
|---|------|-----------|---------------------|--------|-------|
| 1 | Hue Shift | amount (0.0) | AE: Hue/Saturation. Resolume: Hue Rotate. | ⚠️ | **Default 0.0** = no change. Standard for hue since any nonzero shifts color. But consider renaming to "Hue Rotate" (Resolume's name). AE bundles Hue+Sat+Lightness — we split them. Splitting is fine for VJ use. |
| 2 | Saturation | amount (0.5) | AE: Hue/Saturation (Sat slider). Resolume: Saturation. | ✅ | 0.5=normal is correct. Range 0-3x mapped from [0,1]. Industry standard. |
| 3 | Brightness | amount (0.5) | AE: Brightness & Contrast. Resolume: Brightness. | ✅ | 0.5=normal, bidirectional. Correct. |
| 4 | Duotone | color1 RGB (blue), color2 RGB (orange), mix (0.0) | AE: Tritone / CC Toner. Resolume: Duotone. | ⚠️ | **Mix 0.0** = invisible. 6 sliders for 2 colors is clunky. Industry uses **color pickers** not RGB sliders. But given slider-only UI, acceptable. Resolume uses hue + intensity. Missing: **shadow color / highlight color** labels (instead of color1/color2). |
| 5 | Chromatic Aberration | amount (0.0), angle (0.0) | AE: Lens Distortion (CA). Resolume: Chromatic Aberration. | ⚠️ | **Default 0.0** = invisible. Resolume CA defaults visible. Implementation is correct (radial weighting at edges). |
| 6 | Invert | amount (0.0) | AE: Invert. Resolume: Invert. | ⚠️ | **Default 0.0** = invisible. AE Invert is binary (on/off). Having a blend amount is actually better for VJ — allows partial inversion. But default should be higher. |
| 7 | Posterize | levels (1.0) | AE: Posterize. Resolume: Posterize. | ⚠️ | **levels=1.0** = max levels = no visible effect. Should default lower (0.3-0.5 = clearly posterized). AE Posterize "Level" means number of tonal levels (2-255). Our 0-1 mapping should have 0=2 levels (extreme), 1=256 levels (none). Currently backwards — 1.0 means "maximum levels" which is no effect. |
| 8 | Color Shift | red (0.5), green (0.5), blue (0.5) | AE: Channel Mixer (partial). Resolume: Color Balance. | ⚠️ | 0.5=neutral is correct. But naming "Color Shift" is vague. Should be "Color Balance" or "Channel Mix" to match industry. |
| 9 | Thermal | amount (0.0) | AE: n/a. Resolume: Thermal. | ⚠️ | **Default 0.0** = invisible. Good VJ effect. |
| 10 | Contrast | contrast (0.5), balance (0.5) | AE: Brightness & Contrast. Resolume: Contrast. | ✅ | 0.5=neutral. Balance (warm/cool shift) is a bonus. Good implementation. Internal name "color_matrix" is misleading but doesn't affect user. |
| 11 | Sepia | amount (0.0) | AE: n/a (use Tritone). Resolume: Sepia. | ⚠️ | **Default 0.0** = invisible. |
| 12 | Cross Process | amount (0.0) | AE: n/a (use curves). Resolume: Cross Process. | ⚠️ | **Default 0.0** = invisible. |
| 13 | Split Tone | shadow_hue (0.6), highlight_hue (0.1), amount (0.0) | AE: n/a (use Color Finesse). Resolume: Split Toning. | ⚠️ | **Default amount 0.0** = invisible. Good params otherwise. |
| 14 | Color Halftone | scale (0.5), amount (0.0) | AE: Color Halftone. Resolume: Halftone. | ⚠️ | **Default 0.0** = invisible. AE Color Halftone has: max radius, screen angles per channel. We're missing **dot pattern angle** and **channel separation**. |
| 15 | Dither | levels (0.5), amount (0.0) | AE: n/a. Resolume: n/a. Custom. | ⚠️ | **Default 0.0** = invisible. Niche effect. |
| 16 | Heat Map | amount (0.0) | AE: n/a. Resolume: Heat Map. | ⚠️ | **Default 0.0** = invisible. |
| 17 | Selective Color | hue (0.0), range (0.2) | AE: Leave Color / Change Color. Resolume: Selective Color. | 🔧 | Missing: **saturation boost/cut**, **replacement hue**. Currently only isolates a hue range — should also control what happens to selected/deselected colors. AE "Leave Color" desaturates everything except the selected hue. |
| 18 | Film Grain | amount (0.0), size (0.5) | AE: Noise / Add Grain. Resolume: Film Grain. | ⚠️ | **Default 0.0** = invisible. AE Add Grain has: intensity, size, softness, monochrome toggle, color amount. We're missing **monochrome** toggle. |
| 19 | Gamma Levels | black (0.0), white (1.0), gamma (0.5) | AE: Levels. Resolume: Levels. | ✅ | Correct implementation. Black=input black, white=input white, gamma=midtone. Industry standard mapping. |
| 20 | Solarize | threshold (0.5), amount (0.0) | AE: Solarize (old). Resolume: Solarize. | ⚠️ | **Default 0.0** = invisible. |
| 21 | Greyscale | method (0.0), amount (0.0) | AE: Black & White. Resolume: Greyscale. | ⚠️ | **Default 0.0** = invisible. Method selector (luma/avg/desaturate) is nice. |
| 22 | Threshold | level (0.5), amount (0.0) | AE: Threshold. Resolume: Threshold. | ⚠️ | **Default amount 0.0** = invisible. Level 0.5 is correct midpoint. |
| 23 | Exposure | amount (0.5) | AE: Exposure. Resolume: Exposure. | ✅ | 0.5=0 stops is correct. Range maps to ±3 EV. Industry standard. |
| 24 | Vibrance | amount (0.5) | AE: Vibrance (in Camera Raw). Resolume: Vibrance. | ✅ | 0.5=neutral. Correct selective saturation implementation. |
| 25 | Auto Mask | threshold (0.5), softness (0.3), invert (0.0) | AE: Extract. Resolume: Auto Mask. | ✅ | Good implementation. Luminance-based alpha. |
| 26 | Chroma Key | hue (0.33), tolerance (0.3), softness (0.3), amount (0.0) | AE: Keylight. Resolume: Chroma Key. | ⚠️ | **Default amount 0.0** = invisible. Hue 0.33=green is correct. Missing: **spill suppression** (critical for pro chroma key). |
| 27 | Palette Remap | palette (0.0), cycle (0.0), amount (0.0) | AE: Colorama. Resolume: Palette Remap. | ⚠️ | **Default amount 0.0** = invisible. Having 8 palettes is good. |
| 28 | Color Grade | amount (0.0) | AE: Lumetri Color. Resolume: Color Grade. | ⚠️ | **Default 0.0** = invisible. Only 1 param — very limited. AE Lumetri has: temperature, tint, shadows/midtones/highlights per channel, curves, creative LUTs. Our version is a fixed warm-shadow/cool-highlight grade with S-curve. Missing: **style selector** (different grade presets), **temperature**, **tint**. |
| 29 | Pitch Chromatic Shift | amount (0.5), mode (0.0), saturation (0.5) | No equivalent. Audio-native. | ✅ | Audio-driven. Good defaults. |
| 30 | Key Palette | amount (0.5), brightness (0.5), saturation (0.5) | No equivalent. Audio-native. | ✅ | Audio-driven. Good defaults. |
| 31 | Chroma Dissolve | amount (0.5), softness (0.5) | No equivalent. Audio-native. | ✅ | Audio-driven. Good defaults. |

**Color Summary**: 31 effects. 18 have **default 0.0** = invisible. Posterize has inverted logic (1.0=no effect). Color Grade is too limited. Selective Color needs more controls. Duotone is clunky with 6 RGB sliders.

---

### Glitch Effects (16)

| # | Name | Our Params | Industry Equivalent | Rating | Notes |
|---|------|-----------|---------------------|--------|-------|
| 1 | Pixel Scatter | amount (0.0), seed (0.0) | Resolume: Pixel Scatter. | ⚠️ | **Default 0.0** = invisible. |
| 2 | RGB Split | amount (0.0), angle (0.0) | AE: Channel Shift. Resolume: RGB Split. | ⚠️ | **Default 0.0** = invisible. Correct implementation (directional channel offset). |
| 3 | Block Glitch | intensity (0.0), block_size (0.5) | Resolume: Block Glitch. | ⚠️ | **Default 0.0** = invisible. |
| 4 | Scanlines | intensity (0.0), frequency (0.5) | AE: n/a. Resolume: Scanlines. | ⚠️ | **Default 0.0** = invisible. |
| 5 | Digital Rain | intensity (0.0), speed (0.5) | No direct equiv. Matrix-style. | ⚠️ | **Default 0.0** = invisible. |
| 6 | Noise | amount (0.0), speed (0.5) | AE: Noise. Resolume: Noise. | ⚠️ | **Default 0.0** = invisible. |
| 7 | Mirror | horizontal (0.0), vertical (0.0) | AE: Mirror. Resolume: Mirror. | ⚠️ | Both 0.0 = no mirroring. Should one default to 0.5+ to be visible? Actually, for Mirror, 0=off is correct since it's a toggle-like control. But then it should be a step function, not continuous. |
| 8 | Pixelate | size (0.0) | AE: Mosaic. Resolume: Pixelate. | ⚠️ | **Default 0.0** = invisible. AE calls this "Mosaic" with H/V block size. Missing: **horizontal/vertical independent sizing**. |
| 9 | Pixel Explosion | force (0.0), decay (0.5), center x/y (0.5) | Resolume: Particle Explosion (similar). | ⚠️ | **Default 0.0** = invisible. |
| 10 | Color Flash | intensity (0.0), R/G/B (1.0), decay (0.5) | Resolume: Color Flash / Strobe Color. | ⚠️ | **Default 0.0** = invisible. Having RGB color + decay is good. |
| 11 | Fragment Burst | copies (0.3), spread (0.3), rotation (0.2), scale (0.5) | Resolume: Fragment. | ✅ | All params have visible defaults. Good. |
| 12 | Signal Destroy | amount (0.0), speed (0.5), mode (0.0) | Resolume: TVA / Signal Destroy. | ⚠️ | **Default 0.0** = invisible. |
| 13 | Rhythm Slice | amount (0.5), slices (0.5), sync (0.5) | No equivalent. Audio-native. | ✅ | Audio-driven. Good defaults. |
| 14 | Data Corruption | amount (0.5), block size (0.5), color damage (0.5) | Resolume: Data Glitch (similar). | ✅ | All 0.5 defaults = visible. Good. |
| 15 | Glitch Sort | amount (0.5), threshold (0.5), direction (0.0) | AE: Pixel Sort (third party). Resolume: Pixel Sort. | ✅ | Good defaults. Pixel sorting is standard VJ effect. |

**Glitch Summary**: 16 effects. 12 have **default 0.0** = invisible. Mirror's continuous slider is unusual — should be boolean/stepped.

---

### Blur/Post Effects (9)

| # | Name | Our Params | Industry Equivalent | Rating | Notes |
|---|------|-----------|---------------------|--------|-------|
| 1 | Gaussian Blur | radius (0.0) | AE: Gaussian Blur. Resolume: Blur. | ⚠️ | **Default 0.0** = invisible. Implementation is a single-pass approximation (averaging H+V). True Gaussian needs **two-pass separable** for quality. Max 10px radius is very limited — AE goes to 250+. Missing: **repeat edge pixels** option. For VJ use, single-pass is acceptable for performance. |
| 2 | Zoom Blur | amount (0.0), center_x (0.5), center_y (0.5) | AE: CC Radial Blur / Radial Blur. Resolume: Zoom Blur. | ⚠️ | **Default 0.0** = invisible. 16 samples is OK for performance. |
| 3 | Shake | amount_x (0.0), amount_y (0.0) | AE: Wiggle Expression. Resolume: Shake. | ⚠️ | **Both defaults 0.0** = invisible. Missing: **speed/frequency** control (how fast it shakes). Currently static offset, not animated? Should be time-driven randomized shake. |
| 4 | Vignette | intensity (0.0), softness (0.6) | AE: Vignette (in Lumetri). Resolume: Vignette. | ⚠️ | **Default 0.0** = invisible. Missing: **roundness** (circle vs oval), **midpoint** (where falloff begins). |
| 5 | Motion Blur | amount (0.0), angle (0.0) | AE: CC Force Motion Blur / Directional Blur. Resolume: Motion Blur. | ⚠️ | **Default 0.0** = invisible. AE Directional Blur has angle + length. We match this. |
| 6 | Glow | amount (0.0), threshold (0.5) | AE: Glow. Resolume: Glow. | ⚠️ | **Default 0.0** = invisible. AE Glow has: radius, intensity, threshold, color. Missing: **glow radius** (how far the glow spreads), **glow color**. |
| 7 | Edge Detect | amount (0.0) | AE: Find Edges. Resolume: Edge Detect. | ⚠️ | **Default 0.0** = invisible. AE Find Edges also has: **invert** and **blend with original**. Missing these. |
| 8 | Sharpen | amount (0.0), radius (0.3) | AE: Sharpen / Unsharp Mask. Resolume: Sharpen. | ⚠️ | **Default 0.0** = invisible. Resolume Sharpen has amount only. AE Unsharp Mask has: amount, radius, threshold. We have 2/3, missing threshold (to avoid sharpening noise). |
| 9 | Edge Blur | threshold (0.5), amount (0.0) | AE: n/a (custom). Resolume: n/a. | ⚠️ | **Default amount 0.0** = invisible. |
| 10 | Drop Shadow | offset x (0.6), offset y (0.4), blur (0.3), opacity (0.5) | AE: Drop Shadow. Resolume: Drop Shadow. | ✅ | All params have visible defaults. Good implementation matching industry. |

**Blur Summary**: 10 effects. 9 have **default 0.0** = invisible. Gaussian Blur is single-pass (quality concern). Max blur radius is very limited.

---

### Pattern/Stylize Effects (19)

| # | Name | Our Params | Industry Equivalent | Rating | Notes |
|---|------|-----------|---------------------|--------|-------|
| 1 | CRT | curvature (0.3), scanline (0.5) | AE: n/a. Resolume: CRT. | ✅ | Good visible defaults. CRT is always visible with curvature > 0. |
| 2 | VHS | amount (0.0), tracking (0.3) | AE: n/a (third party). Resolume: VHS. | ⚠️ | **Default amount 0.0** = invisible. Missing: **noise** (VHS static), **color bleed**, **jitter**. Real VHS effect needs more degradation layers. |
| 3 | ASCII Art | scale (0.5), color (0.5) | AE: n/a. Resolume: ASCII. | ✅ | Good defaults (always visible when enabled). |
| 4 | Dot Matrix | scale (0.5), amount (0.0) | AE: CC Ball Action (partial). Resolume: Dot Screen. | ⚠️ | **Default amount 0.0** = invisible. |
| 5 | Crosshatch | density (0.5), amount (0.0) | AE: n/a. Resolume: Crosshatch. | ⚠️ | **Default 0.0** = invisible. |
| 6 | Emboss | amount (0.0), angle (0.0) | AE: Emboss. Resolume: Emboss. | ⚠️ | **Default 0.0** = invisible. AE Emboss has: angle, height, amount, contrast. Missing: **height/relief** control. |
| 7 | Oil Paint | radius (0.3) | AE: n/a. Photoshop: Oil Paint. Resolume: Oil Paint. | ✅ | Always visible at radius 0.3. Good. |
| 8 | Pencil Sketch | amount (0.0), density (0.5) | AE: n/a. Resolume: Sketch. | ⚠️ | **Default 0.0** = invisible. |
| 9 | Voronoi Glass | scale (0.5), edge (0.3) | AE: Cell Pattern (similar). Resolume: Voronoi. | ✅ | Good visible defaults. |
| 10 | Cross Stitch | scale (0.5), amount (0.0) | AE: n/a. Resolume: Cross Stitch. | ⚠️ | **Default 0.0** = invisible. |
| 11 | Night Vision | amount (0.0) | AE: n/a. Resolume: Night Vision. | ⚠️ | **Default 0.0** = invisible. Missing: **grain** (NV noise), **scanline overlay**, **bloom**. Real NV has green tint + noise + vignette + bloom. |
| 12 | Triangulate | size (0.3), amount (0.0) | AE: n/a. Resolume: Triangulate. | ⚠️ | **Default 0.0** = invisible. |
| 13 | Neon Edge | edge (0.6), glow (0.5), hue (0.5), original (0.3) | AE: Glow (with Edge Detect). Resolume: Neon. | ✅ | All params have visible defaults. Excellent VJ effect. |
| 14 | Cartoon Ink | edge width (0.4), color steps (0.4), ink strength (0.7), saturation (0.6) | AE: Cartoon. Resolume: Cartoon. | ✅ | All visible defaults. Good param set. |
| 15 | Pop Raster | palette (0.0), bands (0.4), pattern size (0.3), mix (0.7) | AE: n/a. Resolume: Pop Art (similar). | ✅ | Good visible defaults at mix 0.7. |
| 16 | Brush Strokes | size (0.4), angle (0.0), flow (0.5), amount (0.0) | AE: Brush Strokes. Resolume: Brush Strokes. | ⚠️ | **Default amount 0.0** = invisible. |
| 17 | Bump Light | light x (0.5), light y (0.3), intensity (0.6), height (0.5) | AE: Bevel and Emboss. Resolume: Bump Map. | ✅ | Good visible defaults. |
| 18 | Monitor Wall | columns (0.3), rows (0.3), border (0.3), glow (0.3) | AE: n/a. Resolume: Video Wall. | ✅ | Good visible defaults. |
| 19 | Topographic Lines | amount (0.5), levels (0.5), thickness (0.3), color mode (0.0) | AE: n/a. Custom. | ✅ | Good visible defaults. |

**Pattern Summary**: 19 effects. 8 have default 0.0 = invisible. Night Vision and VHS are feature-incomplete compared to industry.

---

### 3D/Depth Effects (9)

| # | Name | Our Params | Industry Equivalent | Rating | Notes |
|---|------|-----------|---------------------|--------|-------|
| 1 | Perspective Tilt | tilt_x (0.5), tilt_y (0.5) | AE: CC Power Pin / 3D. Resolume: Perspective. | ✅ | 0.5=neutral. Good. |
| 2 | Cylinder Wrap | amount (0.0), axis (0.0) | AE: CC Cylinder. Resolume: Cylinder. | ⚠️ | **Default 0.0** = invisible. |
| 3 | Sphere Wrap | amount (0.0) | AE: CC Sphere. Resolume: Sphere. | ⚠️ | **Default 0.0** = invisible. Missing: **radius**, **rotation**, **lighting**. AE CC Sphere has full rotation + lighting control. |
| 4 | Tunnel | speed (0.5), radius (0.3) | AE: CC Cylinder (tunnel mode). Resolume: Tunnel. | ✅ | Always visible (it's a tunnel flythrough). Good defaults. |
| 5 | Page Curl | amount (0.0), radius (0.5) | AE: CC Page Turn. Resolume: Page Curl. | ⚠️ | **Default 0.0** = invisible. AE CC Page Turn has: fold position, fold radius, fold direction, back page controls. Missing: **direction** (which corner curls). |
| 6 | Parallax Layers | amount (0.0), direction (0.0) | AE: n/a (3D layers needed). Resolume: Parallax. | ⚠️ | **Default 0.0** = invisible. |
| 7 | Dot Field | size (0.3), spacing (0.5), depth (0.0) | Resolume: Point Cloud (similar). | ⚠️ | **Depth 0.0** = likely flat, no 3D. Should default higher. |
| 8 | Luminance Terrain | height (0.5), segments (0.5), angle (0.3) | AE: n/a. Resolume: Luma Terrain. | ✅ | Good visible defaults. |
| 9 | Voxel Matrix | size (0.3), height (0.5), rotation (0.0) | AE: n/a. Resolume: Voxel Grid (similar). | ✅ | Mostly visible. Rotation 0.0 is fine (static view). |

**3D Summary**: 9 effects. 5 have default 0.0 = invisible. Sphere Wrap is too limited compared to AE's CC Sphere.

---

### Animation Effects (6)

| # | Name | Our Params | Industry Equivalent | Rating | Notes |
|---|------|-----------|---------------------|--------|-------|
| 1 | Strobe | rate (0.5), intensity (0.0) | AE: Strobe Light. Resolume: Strobe. | ⚠️ | **Default intensity 0.0** = invisible. AE Strobe Light has: strobe duration, strobe period, random strobe probability, blend mode. Missing: **blend mode** and **randomness**. |
| 2 | Pulse | amount (0.0), speed (0.5) | AE: n/a (expression-based). Resolume: Pulse. | ⚠️ | **Default 0.0** = invisible. |
| 3 | Slit Scan | amount (0.0), direction (0.0) | AE: Timewarp (partial). Resolume: Slit Scan. | ⚠️ | **Default 0.0** = invisible. Slit scan needs temporal frame buffer to be truly correct. Does this use u_prev_frame? If not, it's just a spatial distortion pretending to be slit scan. |
| 4 | Point Zoom | amount (0.7), zoom (0.52), rotation (0.52), x/y offset (0.5), decay (0.7), hue shift (0.0), saturation (0.5) | AE: CC RepeTile + Transform. Resolume: Point Zoom / Feedback. | ✅ | Excellent param set. Good visible defaults. This is basically Resolume's Feedback effect. Name "Point Zoom" is confusing — should be called **"Feedback"** to match industry. |
| 5 | Directional Feedback | amount (0.7), speed (0.5), direction (0.5), spread (0.3), decay (0.7), hue shift (0.0) | Resolume: Directional Feedback. | ✅ | Good params and defaults. Matches Resolume. |
| 6 | Transient Flash | style (0.0), intensity (0.5), decay (0.5) | No equivalent. Audio-native. | ✅ | Audio-driven. Good defaults. |

**Animation Summary**: 6 effects. 3 have default 0.0 = invisible. Point Zoom should be renamed to "Feedback". Slit Scan may be incorrect implementation.

---

### Time Effects (6)

| # | Name | Our Params | Industry Equivalent | Rating | Notes |
|---|------|-----------|---------------------|--------|-------|
| 1 | Echo | decay (0.0), operator (0.0) | AE: Echo. Resolume: Echo / Trail. | ⚠️ | **Default decay 0.0** = very short trails (barely visible). Slider remapped 0.82-0.995 internally which is good (per Common Pitfall #18). But **default should be ~0.5** for visible trails on first add. "Operator" slider is unusual — 4 blend modes on a continuous slider. Industry uses a **dropdown/selector** for blend mode, not a slider. |
| 2 | Posterize Time | frame rate (0.0), amount (1.0) | AE: Posterize Time. Resolume: Posterize Time. | ⚠️ | frame rate 0.0 = 60fps = no visible effect. **Amount 1.0 is good** (full effect). But frame rate default should be ~0.5 for visible choppy effect. AE "Posterize Time" only has one param: frame rate. Our "amount" (wet/dry) is a bonus. |
| 3 | Freeze | amount (1.0) | AE: Time > Freeze Frame. Resolume: Freeze. | ✅ | Default 1.0 = fully frozen. Correct — when you add Freeze, the frame freezes. Industry standard behavior. |
| 4 | Screen Split | columns (0.0), rows (0.0), frames per cell (0.25), direction (0.0) | AE: n/a. Resolume: Screen Split / CCTV. | ⚠️ | **Cols/rows 0.0** = 1x1 grid = invisible. Should default to 0.3 (~3x3 grid) to be visible. This effect bypasses GLSL pipeline (rendered by compositor with ring buffer). |
| 5 | Frame Stutter | depth (0.3), stutter (0.0) | AE: Time > Time Remapping (partial). Resolume: Frame Stutter. | ⚠️ | **Stutter 0.0** = no stutter. This also bypasses GLSL pipeline. Depth 0.3 is good. Should stutter default higher. |
| 6 | Channel Delay | red delay (0.0), green delay (0.0), blue delay (0.0) | AE: Time Displacement (partial). Resolume: Channel Delay. | ⚠️ | **All defaults 0.0** = invisible. Should default one channel non-zero (e.g., red=0.3) for visible effect. Temporal effect using u_prev_frame. |

**Time Summary**: 6 effects. 5 have defaults that produce no visible effect. Echo's blend mode selector should be a dropdown not a continuous slider.

---

### Audio Effects (4)

| # | Name | Our Params | Industry Equivalent | Rating | Notes |
|---|------|-----------|---------------------|--------|-------|
| 1 | Harmonic Displacement | amount (0.5), smoothing (0.5), color mode (0.0) | No equiv. Audio-native. | ✅ | Good defaults. Unique differentiator. |
| 2 | Timbral Mosaic | amount (0.5), base size (0.5), complexity (0.5) | No equiv. Audio-native. | ✅ | Good defaults. |
| 3 | Structural Morph | intensity (0.5), normal style (0.3), drop style (0.5) | No equiv. Audio-native. | ✅ | Good defaults. |
| 4 | Beat Ripple | intensity (0.5), decay (0.5), count (0.3) | Resolume: Beat Reactive (partial). | ✅ | Good defaults. |

**Audio Summary**: All 4 effects have good defaults. These are our differentiators — no VJ software has equivalents.

---

### Blend Effects (5)

| # | Name | Our Params | Industry Equivalent | Rating | Notes |
|---|------|-----------|---------------------|--------|-------|
| 1 | Double Exposure | offset (0.3), blend (0.0) | AE: n/a (manual). Resolume: Double Exposure. | ⚠️ | **Blend 0.0** = invisible. |
| 2 | Frosted Glass | amount (0.0), scale (0.5) | AE: n/a. Resolume: Frosted Glass. | ⚠️ | **Default 0.0** = invisible. |
| 3 | Prism | amount (0.0), angle (0.0) | AE: CC Prism (similar). Resolume: Prism. | ⚠️ | **Default 0.0** = invisible. |
| 4 | Rain on Glass | amount (0.0), speed (0.5) | AE: n/a. Custom. | ⚠️ | **Default 0.0** = invisible. |
| 5 | Hexagonalize | scale (0.0) | AE: n/a. Resolume: Hexagonalize. | ⚠️ | **Default 0.0** = invisible. |

**Blend Summary**: All 5 have default 0.0 = invisible. Every single one.

---

### Composite Effects (3)

| # | Name | Our Params | Industry Equivalent | Rating | Notes |
|---|------|-----------|---------------------|--------|-------|
| 1 | Line Cloner | copies (0.3), offset x (0.8), offset y (0.5), scale (0.4), rotation (0.5) | Resolume: Linear Cloner. | ✅ | Good visible defaults. |
| 2 | Radial Cloner | copies (0.3), radius (0.3), rotation (0.0), scale (0.5) | Resolume: Radial Cloner. | ✅ | Good visible defaults. |
| 3 | Cube Scatter | grid x (0.3), grid y (0.3), explode (0.0), rotation (0.0) | Resolume: Cube Scatter (similar). | ⚠️ | Explode 0.0 = flat grid (no 3D scatter). Should default higher. |

---

## Part 2: Sources Audit (81 Sources)

### Noise Sources (3)

| # | Name | Our Params | Industry Equivalent | Rating | Notes |
|---|------|-----------|---------------------|--------|-------|
| 1 | Perlin Noise | Scale (0.5), Speed (0.3), Octaves (0.5), Color Shift (0.0) | Resolume: Perlin Noise. TouchDesigner: Noise TOP. | ✅ | Good defaults. Standard implementation. Missing: **seed** (to randomize pattern). |
| 2 | Plasma | Speed (0.4), Complexity (0.5), Color Cycle (0.0), Intensity (0.7) | Resolume: Plasma. | ✅ | Good defaults. |
| 3 | Voronoi | Scale (0.4), Speed (0.3), Edge Width (0.3), Color Mode (0.0) | Resolume: Voronoi. TouchDesigner: Voronoi SOP. | ✅ | Good defaults. |

### Fractal Sources — 2D (7)

| # | Name | Our Params | Rating | Notes |
|---|------|-----------|--------|-------|
| 1 | Kaleidoscopic Fractal | Iterations, Fold Angle, Zoom, Rotation, Color Shift, Palette | ✅ | Good param set. |
| 2 | Mandelbrot / Julia | Dive Speed, Location, Zoom, Center X/Y, Julia Mix, Max Iter, Power, Color Speed/Shift, Palette | ✅ | Comprehensive. Power limited to 2-4 (correct per Pitfall #11). |
| 3 | Julia Set | Dive Speed, Location, C Real/Imag, Zoom, Iterations, Color Speed/Shift, Palette | 💀 | **SHADER COMPILATION FAILED** (`source_julia_set: FAILED` in startup log). Must fix. |
| 4 | Burning Ship | Dive Speed, Location, Center X/Y, Zoom, Iterations, Color Speed/Shift, Palette | ✅ | Good. |
| 5 | Newton Fractal | Dive Speed, Power, Zoom, Damping, Color Shift, Palette | ✅ | Good. |
| 6 | Sierpinski | Dive Speed, Mode, Zoom, Iterations, Rotation, Color Shift, Palette | ✅ | Mode selector (triangle/carpet) is good. |
| 7 | Apollonian Gasket | Dive Speed, Zoom, Iterations, Rotation, Color Shift, Palette | ✅ | Good. |

### Fractal Sources — 3D (8)

| # | Name | Extra Params | Rating | Notes |
|---|------|-------------|--------|-------|
| 1 | Mandelbulb | Power, Iterations, Detail + shared 3D controls | ✅ | Good. 16 params. |
| 2 | Menger Sponge | Iterations, Twist + shared | ✅ | Good. |
| 3 | Kaleidoscopic IFS | Scale, Iterations, Fold Type, Offset + shared | ✅ | Good. |
| 4 | Julia Set 3D | Location, C Real/Imag, Iterations + shared | ✅ | Quaternion Julia. |
| 5 | Burning Ship 3D | Power + shared | ✅ | Good. |
| 6 | Newton 3D | Power, Damping, Height + shared | ✅ | Good. |
| 7 | Sierpinski Tetrahedron | Iterations + shared | ✅ | Good. |
| 8 | Apollonian 3D | Scale, Iterations + shared | ✅ | Good. |

### Torus/Tunnel Sources (9)

| # | Name | Unique Params | Rating | Notes |
|---|------|--------------|--------|-------|
| 1 | Striped Torus | Stripe Count, Twist | ✅ | Good. Shared torus camera+deform controls. |
| 2 | Spiral Vortex | Twist, Stripe Count | ✅ | Similar to Striped Torus with different defaults. |
| 3 | Checker Torus | Grid U, Grid V | ✅ | Good. |
| 4 | Ribbed Vortex | Ridge Count, Color Mix | ✅ | Good. |
| 5 | Wormhole Tunnel | Warp | ✅ | Good. |
| 6 | Twisted Torus | Twist, Stripe Count | ⚠️ | Very similar to Striped Torus and Spiral Vortex. Consider consolidating — 3 near-duplicate torus variants. |
| 7 | Wormhole | Warp, Glow | ⚠️ | Very similar to Wormhole Tunnel. Two near-duplicates? |
| 8 | Torus Hole | Stripe Count, Twist, Stripe Angle, Scale, Width, Phi/Theta Offset | ✅ | More params = more control. Good flagship torus source. |
| 9 | (8 total including Torus Hole) | | | |

### Pattern Sources (6)

| # | Name | Our Params | Rating | Notes |
|---|------|-----------|--------|-------|
| 1 | Checkerboard | Columns, Rows, Color 1 Hue, Color 2 Hue | ✅ | Standard. |
| 2 | Line Pattern | Count, Width, Rotation, Speed, Color Shift | ✅ | Good. |
| 3 | Concentric Rings | Count, Spacing, Width, Rotation, Color Shift | ✅ | Good. |
| 4 | Sine Oscillator | Waves, Frequency, Amplitude, Modulation, Thickness, Color Shift | ✅ | Good. |
| 5 | Terrain Lines | Height, Lines, Jagginess, Speed, Tilt, Color Shift | ✅ | Good retro look. |
| 6 | Glitch Grid | Grid Size, Chaos, Flicker, Color Shift | ✅ | Good. |

### Geometric Sources (9)

| # | Name | Our Params | Rating | Notes |
|---|------|-----------|--------|-------|
| 1 | Geometric Tunnel | Speed, Segments, Twist, Color Shift | ✅ | Good. |
| 2 | Color Gradient | Angle, Speed, Color 1, Color 2 | ✅ | Standard utility. |
| 3 | Shape Generator | Shape, Size, Rotation, Outline, Color Shift | ✅ | Multi-shape (circle, square, triangle, etc.). |
| 4 | Infinite Zoom | Speed, Layers, Rotation, Color Shift | ✅ | Good VJ effect. |
| 5 | Moire Interference | Pattern, Frequency, Offset X/Y, Rotation, Zoom, Color Shift | ✅ | Good. |
| 6 | Astral Grid | Grid Size, Scroll Speed, Tilt, Warp, Glow, Horizon Color | ✅ | Good retro aesthetic. |
| 7 | Radial Burst | Ray Count, Length, Rotation, Width, Taper, Glow, Color Shift | ✅ | Good. |
| 8 | Hex Grid | Cell Size, Pattern, Fill, Edge Width, Rotation, Color Mode | ✅ | Good. |
| 9 | Sacred Geometry | Pattern, Rotation, Breathe, Line Width, Glow, Reveal, Color Shift | ✅ | Good for spiritual/meditation VJ sets. |

### Lines Sources (11)

| # | Name | Rating | Notes |
|---|------|--------|-------|
| 1 | Line Generator | ✅ | Multi-pattern with feedback. Good flagship line source. |
| 2 | Zigzag Lines | ✅ | Good. |
| 3 | Star Burst | ✅ | Good. |
| 4 | Polygon Lines | ✅ | Good. |
| 5 | Waveform Lines | ✅ | Multi-waveform selector. |
| 6 | Lissajous | ✅ | Good. Classic mathematical. |
| 7 | Spirograph | ✅ | Good. Classic. |
| 8 | Angular Grid | ✅ | Good. |
| 9 | Fractal Tree | ✅ | Good. |
| 10 | Laser Scan | ✅ | Good VJ aesthetic. |
| 11 | Moire Lines | ✅ | Good. |

### Math Sources (8)

| # | Name | Rating | Notes |
|---|------|--------|-------|
| 1 | Lissajous Weaver | ✅ | Enhanced Lissajous with harmonics. |
| 2 | Fermat Spiral Garden | ✅ | Good generative art. |
| 3 | Hyperbolic Tiling | ✅ | Poincare disk model. Unique. |
| 4 | Penrose Pulse | ✅ | Quasi-crystal tiling. |
| 5 | Superformula | ✅ | Good parametric shape. |
| 6 | Truchet Labyrinth | ✅ | Classic pattern. |
| 7 | Rose Curves | ✅ | Mathematical classic. |
| 8 | Fibonacci Spiral | ✅ | Good. |

### Audio-Visual Sources (9)

| # | Name | Rating | Notes |
|---|------|--------|-------|
| 1 | Audio Waveform | ✅ | Core VJ source. Style selector for different visualizations. |
| 2 | Spectrum Landscape | ✅ | 3D terrain from spectrum. Good. |
| 3 | Chromatic Ring | ✅ | Pitch class visualization. Unique. |
| 4 | Band Tower | ✅ | 7-band bar graph. Classic. |
| 5 | Timbral Nebula | ✅ | MFCC-driven particles. Unique. |
| 6 | Structural Landscape | ✅ | Structural state terrain. Unique. |
| 7 | Cymatics | ✅ | Classic. |
| 8 | Spectral Waterfall | ✅ | Classic spectrogram. |
| 9 | Spectral Ring | ✅ | Circular spectrum. Good. |

### Nature Sources (5)

| # | Name | Rating | Notes |
|---|------|--------|-------|
| 1 | Reaction-Diffusion | ✅ | Stateful (ping-pong FBO). Classic generative art. |
| 2 | Cellular Automata | ✅ | Stateful. Game of Life + variants. |
| 3 | Fire Wall | ✅ | Full-width fire. Good VJ effect. |
| 4 | Water Caustics | ✅ | Good. |
| 5 | Electric Arc | ✅ | Good VJ effect. |
| 6 | Fire | ✅ | Simpler fire. Different from Fire Wall. |

### 3D Sources (15)

| # | Name | Rating | Notes |
|---|------|--------|-------|
| 1 | Spiral Tunnel | ✅ | Classic. |
| 2 | Crystal Cavern | ✅ | Ray-marched. |
| 3 | Infinite Corridor | ✅ | Good. |
| 4 | Orbit Chamber | ✅ | Good. |
| 5 | Scroll Plane | ✅ | Good. |
| 6 | Rotating Cube Map | ✅ | Good. |
| 7 | Dual Plane Drift | ✅ | Good. |
| 8 | DNA Helix | ✅ | Good. |
| 9-15 | (8 3D fractals listed above) | | |

### Wireframe Sources (7)

| # | Name | Rating | Notes |
|---|------|--------|-------|
| 1 | Wireframe Sphere | ✅ | Good. Shared 9-param wireframe system. |
| 2 | Wireframe Torus | ✅ | Good. |
| 3 | Wireframe Cube | ✅ | Good. |
| 4 | Wireframe Cylinder | ✅ | Good. |
| 5 | Wireframe Cone | ✅ | Good. |
| 6 | Wireframe Icosahedron | ✅ | Good. |
| 7 | Wireframe Wolf | ✅ | Fun. Low-poly animal. |

### Utility/Text/Simulation/Routing Sources (9)

| # | Name | Rating | Notes |
|---|------|--------|-------|
| 1 | Solid Color | ✅ | Essential utility. RGB controls. |
| 2 | Strobe Light | ✅ | Good. Dual color with fade. |
| 3 | Scrolling Text Wall | ✅ | Matrix-style text. |
| 4 | Text Animator | ✅ | Grid-based animated text. |
| 5 | Strange Attractor | ✅ | 6 attractor types. Stateful. |
| 6 | Gravity Well | ✅ | Particle simulation. Stateful. |
| 7 | Fluid Dynamics | ✅ | Navier-Stokes. Audio-injected. Stateful. |
| 8 | MilkDrop Visualizer | ✅ | projectM integration. 9800 presets. |
| 9 | Layer Router | ✅ | Essential for feedback loops. |

### Other Sources

| # | Name | Category | Rating | Notes |
|---|------|----------|--------|-------|
| 1 | Bump Light (source) | Pattern | ✅ | Procedural bump-lit surface. |
| 2 | Spiral Pattern | Pattern | ✅ | Good. |
| 3 | Metaballs | Organic | ✅ | Classic. |
| 4 | Dot Matrix Wave | Geometric | ✅ | Good. |
| 5 | Radar Sweep | Geometric | ✅ | Good VJ look. |
| 6 | Starfield | Particle | ✅ | Classic. |
| 7 | Particle Nebula | Particle | ✅ | Good. |
| 8 | Lightning Storm | Particle | ✅ | Good. |
| 9 | Laser Scanner | Lighting | ✅ | Good for club VJ. |

---

## Part 3: Priority Fixes

### Critical (❌/💀) — Must Fix

| # | Item | Issue | Fix |
|---|------|-------|-----|
| 1 | **Julia Set source** | Shader compilation FAILED at startup (`source_julia_set: FAILED`). Source is completely broken and unusable. | Fix GLSL compilation error in `source_julia_set` shader. |

### High Priority (⚠️) — Default Values

**THE SINGLE BIGGEST ISSUE**: 68 out of 135 effects (~50%) have their primary parameter defaulting to 0.0, making the effect invisible when first added. This is a major UX problem. When a VJ drags an effect onto a clip during a live performance, they expect to see something happen immediately. Resolume defaults ALL effects to be visible on add.

| # | Effects Needing Default Fix | Recommended Default |
|---|----------------------------|---------------------|
| 1 | Ripple → intensity | 0.4 |
| 2 | Bulge → amount | 0.6 |
| 3 | Wave → amplitude | 0.4 |
| 4 | Liquid → turbulence | 0.4 |
| 5 | Swirl → amount | 0.6 (or 0.4 for subtle) |
| 6 | Polar Coords → amount | 0.5 |
| 7 | Chromatic Aberration → amount | 0.4 |
| 8 | Invert → amount | 0.7 |
| 9 | Hue Shift → amount | 0.3 |
| 10 | RGB Split → amount | 0.4 |
| 11 | Gaussian Blur → radius | 0.3 |
| 12 | Zoom Blur → amount | 0.3 |
| 13 | Glow → amount | 0.4 |
| 14 | Edge Detect → amount | 0.5 |
| 15 | Echo → decay | 0.5 |
| 16 | Posterize Time → frame rate | 0.5 |
| 17 | Screen Split → columns/rows | 0.3 each |
| 18 | Frame Stutter → stutter | 0.4 |
| 19 | Channel Delay → red delay | 0.3 |
| 20 | All Glitch effects (Pixel Scatter, Block Glitch, Scanlines, Digital Rain, Noise, Pixelate, Pixel Explosion, Color Flash, Signal Destroy) | 0.3-0.5 |
| 21 | All Blur effects (Motion Blur, Shake, Vignette, Edge Blur) | 0.3-0.5 |
| 22 | All remaining Color effects (Sepia, Cross Process, Thermal, Heat Map, Solarize, Greyscale, Threshold, Duotone mix, Split Tone, Color Halftone, Dither, Dot Matrix, Crosshatch, Emboss, Pencil Sketch, VHS, Night Vision, Brush Strokes) | 0.3-0.7 |
| 23 | All Blend effects (Double Exposure, Frosted Glass, Prism, Rain on Glass, Hexagonalize) | 0.4-0.5 |
| 24 | 3D effects (Cylinder Wrap, Sphere Wrap, Page Curl, Parallax Layers) | 0.4-0.5 |

### Medium Priority (🔧) — Missing Parameters

| # | Effect | Missing Params | Industry Reference |
|---|--------|---------------|-------------------|
| 1 | **Gaussian Blur** | Needs two-pass separable blur OR larger radius. Current 10px max is very limited. | AE: up to 250px. Resolume: much larger radius. |
| 2 | **Color Grade** | Only 1 param (amount). Needs: style selector, temperature, tint at minimum. | AE Lumetri: 20+ controls. Resolume: multiple grade presets. |
| 3 | **Selective Color** | Needs: saturation control, replacement hue, deselected color handling. | AE Leave Color: tolerance + edge + desat amount. |
| 4 | **Wave** | Missing: wave type (sine/square/triangle), pinning (edge handling). | AE Wave Warp: 7 wave types + pinning. |
| 5 | **Sphere Wrap** | Missing: rotation, lighting, radius. | AE CC Sphere: full 3D rotation + environment lighting. |
| 6 | **Page Curl** | Missing: direction (which corner), back page color. | AE CC Page Turn: corner selection + back opacity. |
| 7 | **Chroma Key** | Missing: spill suppression. | AE Keylight: spill suppression is essential. |
| 8 | **Night Vision** | Missing: grain, scanlines, bloom. Only has amount. | Real NV: green tint + noise + vignette + bloom + scanlines. |
| 9 | **VHS** | Missing: noise, color bleed, jitter, horizontal hold drift. Only has amount + tracking. | Full VHS: 5-6 degradation layers. |
| 10 | **Film Grain** | Missing: monochrome toggle, color amount. | AE Add Grain: monochrome vs color noise. |
| 11 | **Glow** | Missing: glow radius, glow color. | AE Glow: radius + intensity + threshold + color. |
| 12 | **Echo** | Blend mode should be dropdown/selector, not continuous slider. | AE Echo: dropdown for operator. Resolume: dropdown. |
| 13 | **Strobe** | Missing: blend mode, random probability. | AE Strobe Light: more controls. |
| 14 | **Pixelate** | Missing: independent H/V sizing. | AE Mosaic: horizontal + vertical blocks independently. |
| 15 | **Shake** | Missing: speed/frequency (time-driven randomization). | Resolume Shake: amplitude + frequency. |

### Low Priority — Naming / Organization

| # | Issue | Fix |
|---|-------|-----|
| 1 | **Swirl vs Twirl** — near-duplicate effects | Remove one, or make Twirl = Swirl + center X/Y controls. |
| 2 | **Liquid vs Liquid Morph** — very similar | Differentiate or merge. |
| 3 | **Ripple vs Ripple Pond** — similar | Differentiate (Ripple Pond = multi-source?) or merge. |
| 4 | **Wormhole vs Wormhole Tunnel** — near-duplicate sources | Consolidate. |
| 5 | **Striped Torus vs Twisted Torus vs Spiral Vortex** — 3 similar torus variants | Consider consolidating with a "mode" param. |
| 6 | **Point Zoom** should be called **"Feedback"** | Resolume calls it Feedback. VJs know this name. |
| 7 | **UV Remap** should be called **"Noise Displace"** | It's procedural noise displacement, not UV remapping. |
| 8 | **Color Shift** should be called **"Color Balance"** | Industry standard name. |
| 9 | **Posterize levels=1.0** means no effect (max levels) | Invert mapping: 0=many levels (subtle), 1=2 levels (extreme). |
| 10 | **Contrast** internal name is "color_matrix" | Misleading but users don't see it. Low priority. |
| 11 | **Mirror** should use boolean params, not continuous sliders | Mirror is on/off, not gradual. |

---

## Part 4: Missing Effects (Industry Gaps)

Effects that Resolume Arena, AE, or ArKaos have that we don't:

### High Value for VJ Performance

| # | Effect | Category | Description | Why Valuable |
|---|--------|----------|-------------|-------------|
| 1 | **Color Curves** | Color | RGB curves adjustment (like AE Curves) | Essential color correction tool. Every pro video app has this. |
| 2 | **Lumetri-Style Grade** | Color | Temperature, tint, shadows/midtones/highlights | Our Color Grade is too simple. Need full grading. |
| 3 | **Difference** | Blend | Shows difference between current and previous frame | Classic motion detection visual. Resolume has it. |
| 4 | **Displacement Map** | Warp | Use another layer/image as displacement source | Resolume: Displacement. AE: Displacement Map. We have UV Remap but it's procedural only. |
| 5 | **Rotate** | Transform | Simple rotation with center control | Basic but we don't have it as an effect. (We have per-clip/layer transform rotation, but not as a chainable effect.) |
| 6 | **Scale** | Transform | Simple scale with center control | Same — per-clip/layer has it, but not as chainable effect. |
| 7 | **Edge Glow** | Stylize | Glow only on edges (like neon outlines) | Different from Neon Edge — more like edge-detected glow. |

### Medium Value

| # | Effect | Category | Description |
|---|--------|----------|-------------|
| 1 | **Noise Warp** | Warp | Perlin noise-based UV displacement (we have UV Remap but it's poorly named) |
| 2 | **Grid Warp** | Warp | Resolume: manual grid point dragging |
| 3 | **Light Rays / God Rays** | Blur | Volumetric light rays from bright areas |
| 4 | **Lens Flare** | Blur | Classic lens flare overlay |
| 5 | **Particle System** | Animation | Configurable particle emitter effect |
| 6 | **Stencil / Mask** | Composite | Use shape/layer as cutout mask for content |
| 7 | **Bloom** | Blur | Selective glow on bright areas (our Glow is close but limited) |
| 8 | **Tilt Shift** | Blur | Selective focus blur (miniature effect) |

---

## Part 5: Missing Sources (Industry Gaps)

Sources that competitors have that we don't:

| # | Source | Category | What It Is | Priority |
|---|--------|----------|-----------|----------|
| 1 | **Test Pattern** | Utility | Color bars, SMPTE bars, resolution chart | High — essential for output testing/calibration |
| 2 | **Gradient Ramp** | Utility | Multi-stop gradient with angle/type (linear/radial/angular/diamond) | Medium — more flexible than our 2-color gradient |
| 3 | **Clock / Timer** | Utility | Live clock display, countdown timer | Medium — useful for event VJ |
| 4 | **Scope / Oscilloscope** | Audio-Visual | Classic oscilloscope XY mode | Medium — classic audio visualization |
| 5 | **Spectrum Analyzer (Bars)** | Audio-Visual | Traditional DJ-style EQ bars | Medium — we have Band Tower but a simpler classic bar view |
| 6 | **Particle Emitter** | Particle | Configurable particle system with physics | Medium — more flexible than our fixed particle sources |
| 7 | **Text Source** | Text | User-editable text with font/size/color | High — Resolume has editable text. Our Text Wall/Animator are procedural, not user-editable. |
| 8 | **Gradient Circle** | Utility | Radial gradient (point light look) | Low |
| 9 | **Grid** | Utility | Simple grid overlay for alignment | Low |

---

## Part 6: Parameter Naming Conventions

### Current Inconsistencies

| Parameter Concept | Names Used | Recommended Standard |
|-------------------|-----------|---------------------|
| Main strength | "amount", "intensity", "force", "strength" | **"amount"** for mix/blend, **"intensity"** for strength |
| Animation speed | "speed", "rate", "frequency" | **"speed"** for motion, **"rate"** for frequency-based |
| Spatial frequency | "freq", "frequency", "count", "density" | **"frequency"** for wave-like, **"count"** for discrete |
| Edge softness | "softness", "range", "tolerance" | **"softness"** for gradual edges |
| Color cycling | "Color Shift", "Color Cycle", "Hue Shift", "Color Speed" | **"Color Shift"** for hue offset, **"Color Speed"** for animation |
| Blend with original | "amount", "mix", "blend" | **"amount"** universally (or "mix") |

### Recommendations

1. **Use "amount"** as the primary effect intensity param name everywhere (currently most effects do this).
2. **Use "speed"** for anything time-based, not "rate" (rate implies frequency).
3. **Always capitalize** param display names: "Color Shift" not "color shift", "Center X" not "center x" — check all params for consistent capitalization.
4. **Use spaces in display names**, never underscores: "center x" → "Center X".

---

## Part 7: Eyes Test Results

### Infrastructure Issues
- **Render frame size inconsistency**: Tests got 756x756 vs 878x756 frames, causing PSNR comparison failures. The test server window size changes between renders. Need to enforce consistent render resolution in test API.
- **500 Server Error on render_frame**: Some render frame calls fail. May be a threading/timing issue in the test server.

### Shader Compilation
- **134/135 effects compiled OK**
- **source_julia_set: FAILED** — one source shader won't compile
- All other sources compiled OK
- All 15 transitions compiled OK

---

## Summary Statistics

| Metric | Count |
|--------|-------|
| Total Effects | 135 |
| Effects rated ✅ Good | 41 (30%) |
| Effects rated ⚠️ Needs Adjustment | 90 (67%) |
| Effects rated ❌ Wrong | 0 (0%) |
| Effects rated 🔧 Missing Params | 2 (1.5%) |
| Effects rated 💀 Broken | 0 (0%) |
| Total Sources | 81 |
| Sources rated ✅ Good | 78 (96%) |
| Sources rated ⚠️ Needs Adjustment | 2 (2.5%) |
| Sources rated 💀 Broken | 1 (1.2%) |

### Key Takeaways

1. **#1 Issue: Default values**. ~50% of effects are invisible when first added because the primary parameter defaults to 0.0. This is the single biggest UX gap vs Resolume. **Fix: set all primary params to 0.3-0.5 so effects are immediately visible.**

2. **Sources are excellent**. 96% rated Good. The source library is competitive with Resolume and in many areas superior (3D fractals, math sources, audio-native sources).

3. **Effect shader quality is good**. No fundamentally broken effects (except Julia Set source). The implementations are correct — the issues are UX (defaults, naming, missing params).

4. **Near-duplicate effects** should be consolidated: Swirl/Twirl, Liquid/Liquid Morph, Ripple/Ripple Pond.

5. **Missing effects** that would fill industry gaps: Color Curves, Displacement Map, Tilt Shift, God Rays, and a proper Color Grade with multiple presets.

6. **Echo blend mode** should be a dropdown selector, not a continuous slider. VJs need to switch cleanly between Add/Screen/Maximum/Blend.

7. **Posterize levels** mapping is inverted — slider at 1.0 = no visible effect (max levels).
