# Eyes — Visual Testing Harness

Deterministic visual regression testing for Audio-DNA. A C++ HTTP control layer + Python/OpenCV verification layer.

## Architecture

```
Python (pytest)                    C++ (Audio-DNA)
──────────────                    ────────────────
VJAppController  ── HTTP/JSON ──▶  TestServer
  load_image()                       ↓
  set_effect()                    Renderer (GL thread)
  inject_features()                  ↓
  render_frame()                  glReadPixels → PNG
       ↓
  vision_check.py
  PSNR + SSIM comparison
  against golden frames
```

## Quick Start

### 1. Build with test server enabled

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release -DAUDIODNA_BUILD_TEST_SERVER=ON
cmake --build build --config Release -j$(sysctl -n hw.ncpu)
```

### 2. Install Python dependencies

```bash
pip install -r tests/visual/requirements-test.txt
```

### 3. Run tests

```bash
cd tests/visual
pytest test_render_pipeline.py -v
```

Or run against an already-running app:

```bash
# Terminal 1: Start app manually
./build/AudioDNA_artefacts/Release/Audio-DNA.app/Contents/MacOS/Audio-DNA --test-mode --test-port=8080

# Terminal 2: Run tests without spawning
AUDIODNA_NO_SPAWN=1 pytest tests/visual/ -v
```

## CLI Arguments

| Argument | Default | Description |
|---|---|---|
| `--test-mode` | off | Enable the Eyes HTTP server |
| `--test-port=N` | 8080 | HTTP server port |

## HTTP API Reference

All endpoints use JSON. POST endpoints expect a JSON body.

### GET /api/health

Returns server status.

```json
{"status": "ready", "gl_version": "4.1", "fps": 60.0, "effects_count": 110}
```

### POST /api/load_image

Load an image into the renderer.

```json
{"filepath": "/absolute/path/to/image.png"}
```

### POST /api/set_effect

Enable/disable a single effect and set parameters.

```json
{
  "name": "Ripple",
  "enabled": true,
  "params": {"intensity": 0.5, "frequency": 0.3}
}
```

Effect names are display names (e.g., "Hue Shift", "Chromatic Aberration").
Parameter names match the effect's param names (e.g., "intensity", "shift").
All parameter values are [0, 1].

### POST /api/set_effect_chain

Configure the entire effect chain. Disables all effects first, then enables the listed ones.

```json
{
  "effects": [
    {"name": "Ripple", "params": {"intensity": 0.5}},
    {"name": "Hue Shift", "params": {"shift": 0.3}}
  ]
}
```

### POST /api/inject_features

Inject synthetic audio features (bypasses the analysis thread).

```json
{
  "rms": 0.8,
  "beatPhase": 0.5,
  "spectralCentroid": 2000.0,
  "bandEnergies": [0.1, 0.3, 0.5, 0.7, 0.5, 0.3, 0.1]
}
```

Supported fields: rms, peak, rmsDB, lufs, dynamicRange, transientDensity,
spectralCentroid, spectralFlux, spectralFlatness, spectralRolloff,
bpm, beatPhase, barPhase, phrasePhase, barCount, structuralState,
dominantPitch, pitchConfidence, detectedKey, keyIsMajor,
harmonicChangeDetection, onsetDetected, onsetStrength, beatInBar,
downbeatDetected, bandEnergies[7], chromagram[12], mfccs[13].

### POST /api/render_frame

Render a single frame and save to disk. Blocks until the GL thread completes.

```json
{
  "output_path": "/tmp/frame.png",
  "time": 1.0,
  "width": 1920,
  "height": 1080
}
```

- `time`: Overrides `u_time` uniform for deterministic rendering.
- `width`/`height`: Temporarily sets locked resolution. Max 1920x1080.

### GET /api/state

Returns the full engine state: all effects with parameters, FPS, deck info.

### POST /api/reset

Resets all state: disables effects, clears images, resets features and master level.

## Python API

### VJAppController

```python
from vj_controller import VJAppController

app = VJAppController(port=8080, executable="path/to/Audio-DNA")
app.start(timeout=20)

app.load_image("/path/to/image.png")
app.set_effect("Ripple", enabled=True, params={"intensity": 0.5})
app.inject_features({"rms": 0.8})
app.render_frame("/tmp/output.png", time_val=1.0, width=1920, height=1080)
app.reset()
app.stop()
```

### vision_check

```python
from vision_check import verify_frame

passed, metrics = verify_frame("rendered.png", "golden.png",
                                psnr_threshold=50.0, ssim_threshold=0.99)
print(f"PSNR={metrics['psnr']:.1f} SSIM={metrics['ssim']:.4f}")
```

## Golden Frame Workflow

1. Run the app in test mode
2. Use VJAppController to set up a scene
3. Capture frames to `tests/visual/golden_frames/`
4. Commit golden frames to git
5. Future test runs compare against these references

Regenerate golden frames when:
- Shader source code changes
- GPU driver updates
- Moving to a different GPU architecture

## Environment Variables

| Variable | Description |
|---|---|
| `AUDIODNA_EXE` | Override path to the executable |
| `AUDIODNA_TEST_PORT` | Override HTTP port (default: 8080) |
| `AUDIODNA_NO_SPAWN` | Set to "1" to use an already-running app |

## Troubleshooting

**App doesn't start**: Build with `-DAUDIODNA_BUILD_TEST_SERVER=ON`.

**Port conflict**: Use `--test-port=9090` or set `AUDIODNA_TEST_PORT=9090`.

**Frame capture timeout**: The GL thread may be blocked. Check that the app window exists (even if minimized).

**PSNR/SSIM too low**: GPU differences cause +-1 LSB variance. Use PSNR>45 / SSIM>0.98 as thresholds for cross-machine tests.
