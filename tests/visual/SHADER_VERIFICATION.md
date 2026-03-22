# Shader Verification System

Mandatory verification protocol for all procedural source shaders. Every shader MUST pass all 4 verification tiers before shipping.

## The Problem This Solves

Shaders compile but produce broken visuals: black screens, jump-cuts, dead controls, wrong zoom direction, unusable parameter ranges. These bugs are invisible to the C++ compiler and unit tests — they only appear when a human sees the rendered output. This system catches them automatically.

## 4-Tier Verification

### Tier 1: Parameter Sweep (automated, ~2 min per source)

For every source, sweep each parameter across its full range in 5 steps (0.0, 0.25, 0.5, 0.75, 1.0) and verify:

1. **Not black**: `mean(pixels) > 5` at every parameter position
2. **Not frozen**: Changing a parameter changes the output (PSNR < 55 vs the default)
3. **No discontinuities**: Adjacent steps shouldn't differ too much (PSNR > 8 — prevents jump-cuts)

```bash
# Run against the live app:
AUDIODNA_NO_SPAWN=1 pytest tests/visual/test_fractals.py -v

# Or with the app spawned automatically:
pytest tests/visual/test_fractals.py -v
```

**When to run**: After ANY shader edit, before committing. This is the gate.

### Tier 2: Range Quality (automated, generates report)

For each parameter, render at 11 positions (0.0, 0.1, ..., 1.0) and compute:

- **Brightness curve**: `mean(pixels)` at each position → should be smooth, never drop below 5
- **Variety score**: Average PSNR between adjacent steps → should be 20-45 (too low = discontinuity, too high = no effect)
- **Useful range**: What fraction of the 0-1 range produces distinct, non-black output → should be > 70%

This generates a CSV report: `tests/visual/reports/{source_id}_range_quality.csv`

```bash
pytest tests/visual/test_range_quality.py -v --report
```

**When to run**: After tuning slider ranges or default values. Identifies dead zones.

### Tier 3: Browser Preview (manual, ~30 sec per source)

Open `tests/visual/shader_preview.html` in Chrome. For each source:

1. Load the shader
2. Move EVERY slider end-to-end
3. Verify: smooth transitions, no black, no jumps, interesting visuals at all positions
4. Check that default values show something beautiful

This catches things automated tests miss: "technically not black but boring", "zoom goes the wrong direction", "the pattern is ugly".

**When to run**: Before the first commit of any new or rewritten shader.

### Tier 4: In-App Validation (user, ~1 min per source)

Build the app, load the source, test each slider. This is the final gate.

**When to run**: After Tiers 1-3 pass, before marking the task complete.

## Test File Structure

```
tests/visual/
├── TESTING.md                    # Eyes harness docs
├── SHADER_VERIFICATION.md        # This file
├── conftest.py                   # Pytest fixtures (app spawn/reset)
├── vj_controller.py              # Python HTTP client for Eyes API
├── vision_check.py               # PSNR/SSIM image comparison
├── shader_preview.html           # Browser WebGL shader tester
├── test_render_pipeline.py       # Core render tests (effects, features)
├── test_fractals.py              # Tier 1: param sweep for all fractals
├── test_range_quality.py         # Tier 2: range quality reports
├── golden_frames/                # Reference images for regression
├── diffs/                        # Amplified diff images on failure
└── reports/                      # Range quality CSV reports
```

## Adding a New Source: Verification Checklist

When adding a new procedural source shader:

1. **Write the shader** in `EmbeddedShaders.h`
2. **Register** in `SourceRegistry.cpp` with sensible defaults
3. **Add to browser** in `SourcesBrowser.cpp`
4. **Add to Renderer** compile list in `Renderer.cpp`
5. **Add test definitions** to `test_fractals.py` — one entry per parameter
6. **Build**: `cmake --build build --config Release`
7. **Run Tier 1**: `AUDIODNA_NO_SPAWN=1 pytest tests/visual/test_fractals.py -k "new_source_id" -v`
8. **Fix** any failures (black frames, dead controls, discontinuities)
9. **Run Tier 2**: `AUDIODNA_NO_SPAWN=1 pytest tests/visual/test_range_quality.py -k "new_source_id" -v`
10. **Review** the range quality report — tune defaults and ranges
11. **Browser preview** (Tier 3): Open `shader_preview.html`, load shader, test every slider
12. **In-app validation** (Tier 4): Build, launch, test in the actual app
13. **Commit** only after all 4 tiers pass

## Tuning Slider Ranges: The Range Quality Protocol

When a slider "doesn't do anything" or "goes black", the problem is the mapping from [0,1] to the shader's internal range. Use this process:

1. **Run Tier 2** to get the brightness curve and variety score
2. **Identify dead zones**: positions where brightness < 5 or variety < 10
3. **Identify discontinuities**: positions where variety > 50 (jump-cut)
4. **Fix the mapping**: Adjust the shader's `float param = u_src_foo * range + offset` formula
5. **Re-run Tier 2** to verify the fix
6. **Adjust default**: Set the default to the position with the highest variety score

**Common mapping fixes:**
- Dead zone at high end → reduce range: `param * 4.0` → `param * 2.0`
- Dead zone at low end → add offset: `param * range` → `offset + param * range`
- Jump-cut at a specific position → use smoothstep or quadratic: `param * param * range`
- Most of the range is boring → remap to the interesting region only

## Integration with Build Workflow

The "kick off phase N" protocol in CLAUDE.md should include:

> After completing all tasks and before reporting to the user:
> 1. Build: `cmake --build build --config Release`
> 2. C++ tests: `ctest --test-dir build`
> 3. **Visual tests** (if source shaders were added/modified):
>    `AUDIODNA_NO_SPAWN=1 pytest tests/visual/test_fractals.py -v`
> 4. Fix any visual test failures before proceeding

This makes Tier 1 part of the standard self-validation step.
