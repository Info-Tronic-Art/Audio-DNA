# Fractal System + Visual Shader Testing Reference

> Moved from CLAUDE.md (claudemd-split). Browser-based shader testing protocol, then the full fractal parameter/source reference.

---

### Visual Shader Testing Protocol (MANDATORY for procedural sources)

**NEVER ship a new procedural source shader to the app without browser-verified visual testing.**

Complex shaders (raymarched 3D, torus, fractals) MUST be tested in-browser via Playwright before building the C++ app. The browser test loop is ~5 seconds vs ~30 seconds for a full rebuild — iterating in-browser is 6x faster and prevents shipping broken visuals.

**Testing workflow:**
1. Write the shader in `test_torus.html` (or `test_[name].html`) at project root
2. Start HTTP server: `python3 -m http.server [port] &` (run_in_background)
3. Navigate Playwright: `browser_navigate` to `http://localhost:[port]/test_[name].html`
4. Wait 2s, take screenshot with `browser_take_screenshot`
5. Compare screenshot against reference image — verify ALL of:
   - Frame fill (source must fill the viewport, minimal background)
   - Pattern matches reference (stripe density, curvature, convergence)
   - No seam artifacts or visual glitches
   - Correct centering (use `(gl_FragCoord.xy - 0.5*u_resolution) / u_resolution.y`)
6. Iterate shader parameters until **98% visual parity** with reference
7. ONLY THEN port to `EmbeddedShaders.h`, register, compile
8. Clean up test files

**98% parity requirement**: The shader must match the reference image in overall composition, stripe/pattern structure, and fill. Minor differences in exact stripe count or rotation angle are acceptable. Major differences in camera angle, pattern type, or visual artifacts are NOT acceptable.

**Reference**: See `memory/3dSpiral.md` for verified torus parameters, camera setups, and stripe formulas.

---

### Fractal System Reference

All fractal sources, their parameters, design rules, and test infrastructure in one place. Refer here when refining any fractal.

**Source code locations:**
- Shader GLSL: `src/render/EmbeddedShaders.h` — search for `sourceJuliaSet`, `sourceMandelbrot`, `sourceMandelbulb`, etc.
- Parameter registration: `src/sources/SourceRegistry.cpp` — search for `registerSource("mandelbrot"`, etc.
- Browser listing: `src/ui/SourcesBrowser.cpp` — display names and categories
- Shader compilation: `src/render/Renderer.cpp` — `compile("source_mandelbrot", ...)` calls
- Right-click reset: `src/ui/ClipInspector.cpp` line ~769 — `setDefaultValue(sp.defaultValue)`

**Test infrastructure:**
- `tests/visual/test_fractals.py` — ~185 parametrized tests: every param on every fractal
- `tests/visual/shader_preview.html` — Browser-based WebGL shader preview with sliders
- `tests/visual/vj_controller.py` — `load_source()`, `update_source_params()`, `list_sources()`
- Run tests: `AUDIODNA_NO_SPAWN=1 pytest tests/visual/test_fractals.py -v`

**2D Fractals (7 sources):**

| Source ID | Name | Key Params | Notes |
|-----------|------|------------|-------|
| `kaleido_fractal` | Kaleidoscopic Fractal | Iterations, Fold Angle, Zoom, Rotation, Color Shift, Palette | Fold-based IFS |
| `mandelbrot` | Mandelbrot / Julia | Dive Speed, Location (10 presets), Zoom, Center X/Y, Julia Mix, Max Iter, Power (2-4), Color Speed/Shift, Palette | Power > 4 goes black |
| `julia_set` | Julia Set | Dive Speed (morphs c), Location (10 c-presets), C Real/Imag, Zoom, Iterations, Color Speed/Shift, Palette | Dive = c morphing, not just zoom |
| `burning_ship` | Burning Ship | Dive Speed, Location (6 presets), Center X/Y, Zoom, Iterations, Color Speed/Shift, Palette | Center default (-0.75, -0.5) |
| `newton_fractal` | Newton Fractal | Dive Speed, Power (3-8), Zoom, Damping, Color Shift, Palette | Dive targets root boundary |
| `sierpinski` | Sierpinski | Dive Speed, Mode (triangle/carpet), Zoom, Iterations, Rotation, Color Shift, Palette | Triangle = modular arithmetic |
| `apollonian` | Apollonian Gasket | Dive Speed, Zoom, Iterations, Rotation, Color Shift, Palette | Zoom direction: uv /= zoom |

**3D Fractals (8 sources) — all share these controls:**

| Param | Uniform | Range | Default | What it does |
|-------|---------|-------|---------|-------------|
| Zoom | `u_src_zoom` | Camera 5.0→0.3 | 0.3 | Far outside → inside fractal |
| Speed | `u_src_speed` | -1→+1 rotation | 0.55 | Auto-rotate, 0.5=stopped |
| Angle X/Y | `u_src_rotation_x/y` | 0-2pi | 0.55 | Camera orbit angle |
| Cross Section | `u_src_slice` | z-plane pos | 0.5 | 0.5=off, else slices |
| Slice Count | `u_src_slice_count` | 1-5 planes | 0.0 | Multi-slice |
| Slice Distance | `u_src_slice_dist` | 0.1-0.9 spacing | 0.3 | Between slices |
| Glow | `u_src_glow` | volumetric glow | 0.0 | Halo around surface |
| Trail Distance | `u_src_trail_dist` | 0-5 edge copies | 0.0 | Discrete edge feedback |
| Trail Fade | `u_src_trail_fade` | decay rate | 0.5 | 0=tight rings, 1=wide |
| Feedback | `u_src_feedback` | interference rings | 0.0 | Edge echo modulation |
| Color Shift | `u_src_color_shift` | hue offset | 0.0 | Palette phase |
| Palette | `u_src_palette` | 8 cosine palettes | 0.6 | Fire/Ocean/Neon/Gray/Rainbow/Psyche/Ice/Sunset |

| Source ID | Name | Extra Params |
|-----------|------|-------------|
| `mandelbulb` | Mandelbulb | Power (2-16 quadratic), Iterations, Detail |
| `menger_sponge` | Menger Sponge | Iterations, Twist |
| `kifs` | Kaleidoscopic IFS | Scale, Iterations, Fold Type, Offset |
| `julia_set_3d` | Julia Set 3D | Location (6 quaternion presets), C Real/Imag, Iterations |
| `burning_ship_3d` | Burning Ship 3D | Power (2-16) |
| `newton_3d` | Newton 3D | Power, Damping, Height |
| `sierpinski_tetra` | Sierpinski Tetrahedron | Iterations |
| `apollonian_3d` | Apollonian 3D | Scale, Iterations |

**Design rules (see Common Pitfalls 8-12 in `docs/claude/pitfalls.md` for details):**
1. Zoom: direct depth, no `fract()`, dive only adds to zoom rate
2. Center/Location/Dive cleanly separated — dive never moves the center
3. Power: Mandelbrot max 4, clamp smooth iteration count `max(si, 0.0)`
4. All palettes use the `fracPalette(t, idx)` function (copy-pasted per shader)
5. 3D normals via central differences (6 DE calls), trail via near-surface DE sampling

**Verification system:** `tests/visual/SHADER_VERIFICATION.md` — 4-tier verification protocol (param sweep → range quality → browser preview → in-app). Run Tiers 1-2 before every commit. See also `tests/visual/test_range_quality.py` for tuning slider ranges.

**Memory files:** `memory/project_v2_p15_5_fractal_overhaul.md`, `memory/feedback_fractal_zoom_design.md`, `memory/feedback_fractal_controls_separation.md`

---
