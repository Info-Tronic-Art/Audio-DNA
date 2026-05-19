# Audio-DNA Test Architecture

## Three Test Layers

### 1. Catch2 Unit Tests (C++)
- **Location:** `tests/test_*.cpp`
- **Count:** ~113 tests across 11 files
- **Mode:** No app required — pure unit tests
- **Run:** `cd build && ctest --output-on-failure`
- **What they cover:** ring buffer, feature bus, composition, routing, BPM, spectral analysis, mapping engine, integration pipeline

### 2. Eyes Visual Tests (Python)
- **Location:** `tests/visual/test_*.py`
- **Count:** ~491 test cases across 11 files
- **Mode:** Requires running Audio-DNA app
- **Port:** 8080 (TestServer, test-mode) or 7070 (ApiServer, production)

#### Test-Mode vs Production-Mode

| Mode | Flag | Port | Tests that work |
|------|------|------|----------------|
| Test mode | `--test-mode` | 8080 (TestServer) | All ~491 Eyes tests |
| Production | (default) | 7070 (ApiServer) | ~38 tests (render pipeline + basic API) |

**Why the split:** Test-mode activates TestServer which provides `inject_features` — the ability to inject fake audio analysis data. Most visual tests need controlled audio input to produce deterministic renders. In production mode, live audio analysis overrides injected features, making ~430 tests non-deterministic.

#### Running Eyes Tests

```bash
# Build with test server support
cmake -B build -DAUDIODNA_BUILD_TEST_SERVER=ON
cmake --build build --config Release

# Run all Eyes tests (app spawned automatically by conftest.py)
cd tests/visual && python -m pytest

# Or connect to an already-running app
AUDIODNA_NO_SPAWN=1 AUDIODNA_TEST_PORT=8080 python -m pytest

# Production-mode subset only (connect to running app on port 7070)
AUDIODNA_NO_SPAWN=1 AUDIODNA_TEST_PORT=7070 python -m pytest test_render_pipeline.py
```

#### Python Dependencies

```bash
pip install pytest pytest-timeout opencv-python Pillow requests pyobjc
```

#### conftest.py Gotcha

The `tests/visual/conftest.py` has an `autouse=True` fixture `reset_between_tests` that depends on the `app` fixture. This means ANY new test file placed in `tests/visual/` will automatically try to connect to the running app and be skipped if no executable is found.

**If your test does NOT need the running app** (e.g., `test_ax_inspector.py`), override both fixtures locally:

```python
@pytest.fixture
def app():
    pytest.skip("not needed")

@pytest.fixture(autouse=True)
def reset_between_tests():
    yield
```

### 3. ax_inspector Tests (Python, no app binary needed)
- **Location:** `tests/visual/test_ax_inspector.py`
- **Count:** 19 tests
- **Mode:** Needs app RUNNING (any mode) but not the test binary — reads macOS accessibility tree externally
- **Run:** `python -m pytest test_ax_inspector.py` (with Audio-DNA open)

## Utility Scripts

| Script | Purpose |
|--------|---------|
| `vj_controller.py` | Python client for Eyes API (start/stop/reset/render) |
| `vision_check.py` | PSNR/SSIM image comparison |
| `ax_inspector.py` | macOS accessibility tree reader |
| `scan_all_presets.py` | Smoke-test all presets sequentially |
| `verify_defaults.py` | Validate default preset renders |

## Known Issues

- **Melatonin Inspector FetchContent broken:** Module header not found at build time. Build with `-DAUDIODNA_BUILD_INSPECTOR=OFF` to work around. Inspector is a manual debugging tool (Cmd+Shift+I) — not required for automated testing.
- **ApiServer at 900 lines:** Budget ceiling. Next endpoint addition requires refactoring first.
- **load_image black in production:** ApiServer `load_image` sets a texture but production renderer draws composition layers, not standalone images. Use `load_source("checkerboard")` for effect tests in production mode.
