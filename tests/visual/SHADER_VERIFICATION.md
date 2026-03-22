# Shader & Visual Verification System

Mandatory verification for ALL visual elements: effects, sources, feedback, transitions, compositor. Every shader MUST pass verification before shipping.

## The Problem

Shaders compile but produce broken visuals. C++ tests can't catch: black screens, dead controls, wrong ranges, jump-cuts, ugly defaults, performance drops. This system catches them automatically across 4 tiers.

## What Gets Tested

| Component | Count | Test File | What's Verified |
|-----------|-------|-----------|-----------------|
| **Procedural Sources** | 55 | `test_sources.py` | Every param at 5+ positions, non-black, has-effect, no-discontinuity |
| **Effects (FX)** | 112 | `test_effects.py` | Every param at 5+ positions on a test image, non-black, has-effect |
| **Audio Reactivity** | ~20 features | `test_audio_reactivity.py` | Injected features change effect/source output |
| **Transitions** | 15 | `test_transitions.py` | Progress 0→1 produces smooth crossfade |
| **Time Dependence** | all animated | `test_time_sweep.py` | Rendering at t=0, 1, 5, 10 produces different non-black frames |
| **Performance** | all | `test_performance.py` | Render time per frame < 16ms (60fps target) |
| **Range Quality** | all params | `test_range_quality.py` | 11-position sweep, CSV reports, 70%+ useful range |

## 4-Tier Verification Protocol

### Tier 1: Automated Param Sweep (gate — must pass before commit)

For **every source** and **every effect**, sweep each parameter at 5 positions (0.0, 0.25, 0.5, 0.75, 1.0):

1. **Not black**: `mean(pixels) > 5` at every position
2. **Has effect**: Changing the param changes the output (PSNR < 55 vs default)
3. **No discontinuity**: Adjacent steps have PSNR > 8 (no jump-cuts)

```bash
# Sources:
AUDIODNA_NO_SPAWN=1 pytest tests/visual/test_sources.py -v

# Effects:
AUDIODNA_NO_SPAWN=1 pytest tests/visual/test_effects.py -v

# Or run everything:
AUDIODNA_NO_SPAWN=1 pytest tests/visual/ -v
```

### Tier 2: Range Quality Analysis (tuning — run when adjusting ranges)

11-position sweep per param. Generates CSV reports with:
- Brightness curve at each position
- Variety score between adjacent steps
- Useful range percentage (non-black AND distinct)
- Discontinuity detection

```bash
AUDIODNA_NO_SPAWN=1 pytest tests/visual/test_range_quality.py -v -k "source_name"
# Reports: tests/visual/reports/{source}_{uniform}.csv
```

**Tuning workflow:**
1. Run Tier 2 for the source/effect you're tuning
2. Open the CSV — find dead zones (brightness < 5) and jump-cuts (PSNR < 8)
3. Fix the shader mapping: adjust `param * range + offset`
4. Re-run Tier 2, verify improvement
5. Repeat until useful_range > 70% and no discontinuities

### Tier 3: Visual Preview (Claude does this before reporting to user)

- **Sources**: Open `tests/visual/shader_preview.html`, move every slider end-to-end
- **Effects**: Load test_card.png, enable effect, move every slider
- Verify: smooth transitions, interesting visuals, good defaults

### Tier 4: In-App Validation (user does this)

Build app, load source/effect, test each slider. Final gate.

## Test Architecture

### Sources Test (`test_sources.py`)

Tests ALL procedural sources. Param definitions live in a registry that mirrors `SourceRegistry.cpp`:

```python
# Auto-discover: query /api/sources to get all sources and their params
def test_all_source_params(app):
    sources = app.list_sources()["sources"]
    for src in sources:
        for param in src["params"]:
            # Test at default, 0.0, 0.5, 1.0
            ...
```

This is self-maintaining — when a new source is added to SourceRegistry.cpp, it automatically appears in the API and gets tested. No manual test definitions needed.

### Effects Test (`test_effects.py`)

Tests ALL 112 effects. Uses the `/api/state` endpoint to discover effects and params:

```python
def test_all_effect_params(app):
    state = app.state()
    for fx in state["effects"]:
        for param in fx["params"]:
            # Enable effect, set param, render, verify
            ...
```

Also self-maintaining — new effects automatically get tested.

### Audio Reactivity Test (`test_audio_reactivity.py`)

Injects synthetic audio features and verifies they change the output:

```python
FEATURE_TESTS = [
    {"rms": 0.0} vs {"rms": 1.0},
    {"beatPhase": 0.0} vs {"beatPhase": 0.5},
    {"spectralCentroid": 200.0} vs {"spectralCentroid": 8000.0},
    {"bandEnergies": [1,0,0,0,0,0,0]} vs {"bandEnergies": [0,0,0,0,0,0,1]},
]
```

For each pair: load a source, inject features, render, verify frames are different.

### Time Sweep Test (`test_time_sweep.py`)

For every animated source/effect, render at t=0, t=1, t=5, t=10 and verify frames change:

```python
def test_time_changes_output(app, source_id):
    frames = [render_at_time(app, source_id, t) for t in [0.0, 1.0, 5.0, 10.0]]
    # At least 2 pairs must differ
    ...
```

### Performance Test (`test_performance.py`)

Render 10 frames per source/effect, measure average render time:

```python
def test_render_performance(app, source_id):
    times = []
    for i in range(10):
        start = time.time()
        app.render_frame(...)
        times.append(time.time() - start)
    avg_ms = np.mean(times) * 1000
    assert avg_ms < 50, f"{source_id} takes {avg_ms:.0f}ms (budget: 50ms including HTTP overhead)"
```

### Transition Test (`test_transitions.py`)

For all 15 transitions, render at progress 0.0, 0.25, 0.5, 0.75, 1.0 between two images:

```python
# Requires: POST /api/set_transition {"type": "dissolve", "progress": 0.5}
# (may need new API endpoint)
```

## Self-Maintaining Design

The key principle: **tests discover what to test from the running app's API**, not from hardcoded lists. When a new effect or source is added:

1. It gets registered in `SourceRegistry.cpp` or `EffectLibrary.cpp`
2. The Eyes API exposes it via `/api/sources` or `/api/state`
3. The test auto-discovers it and sweeps all its params

The only hardcoded test data is:
- Threshold values (brightness > 5, PSNR ranges)
- Audio feature test pairs (which features to inject)
- Known exceptions (effects that are intentionally black at certain params)

## Phase Coverage Matrix

| Phase | What's Added | Verified By |
|-------|-------------|-------------|
| **P16** | 5 time effects + feedback system | `test_effects.py` (params), `test_time_sweep.py` (temporal), feedback needs compositor test |
| **P17** | 19 creative sources | `test_sources.py` (auto-discovered), `test_range_quality.py` (tuning) |
| **P18** | 10 audio effects + 8 audio sources + text | `test_effects.py`, `test_sources.py`, `test_audio_reactivity.py` (feature injection) |
| **P19** | 8 complex effects + 12 sources | `test_effects.py`, `test_sources.py` |
| **P20** | Layer router, FFGL, text, simulations | Needs compositor-level tests (not shader-level) |
| **P21** | Live performance controls | Needs interaction tests (binding/MIDI), not visual |

### Gaps That Need Additional Work

1. **Feedback system (P16)** — `FeedbackProcessor` operates at compositor level, not shader level. Needs a compositor test: enable feedback on a layer, render 10 frames, verify the feedback accumulates (frame 10 should differ from frame 1 in a specific way).

2. **Layer Router (P20)** — Tests need multi-layer setup via the API. May need a new endpoint: `POST /api/set_layer_source`.

3. **FFGL Plugins (P20)** — External binaries, need host-level testing, not shader testing.

4. **Performance under load** — Current perf test uses one source at a time. Need multi-layer perf test (4 layers with effects = realistic VJ load).

5. **Transitions (P16-19)** — Need a new API endpoint to test transitions between two images at various progress values.

## Integration with Build Workflow

In CLAUDE.md "kick off phase N" protocol, step 4 (self-validate):

```
4. Self-validate:
   - Build passes
   - C++ tests pass
   - IF sources/effects/shaders changed:
     - Tier 1: pytest tests/visual/test_sources.py test_effects.py -v
     - Tier 2: pytest tests/visual/test_range_quality.py -v (for changed items)
     - Fix ALL failures before proceeding
   - IF audio-reactive features changed:
     - pytest tests/visual/test_audio_reactivity.py -v
```

## File Structure

```
tests/visual/
├── TESTING.md                     # Eyes harness docs
├── SHADER_VERIFICATION.md         # THIS FILE — verification system design
├── conftest.py                    # App spawn/reset fixtures
├── vj_controller.py               # Python HTTP client
├── vision_check.py                # PSNR/SSIM comparison
├── shader_preview.html            # Browser WebGL tester
├── test_render_pipeline.py        # Core render/effect/feature tests
├── test_fractals.py               # Fractal-specific param tests (hardcoded)
├── test_sources.py                # ALL sources auto-discovered param sweep
├── test_effects.py                # ALL effects auto-discovered param sweep
├── test_range_quality.py          # 11-position range quality with CSV reports
├── test_audio_reactivity.py       # Feature injection verification
├── test_time_sweep.py             # Temporal animation verification
├── test_performance.py            # Render time budget verification
├── test_transitions.py            # Transition progress sweep
├── golden_frames/                 # Reference images
├── diffs/                         # Failure diff images
└── reports/                       # Range quality CSVs
```
