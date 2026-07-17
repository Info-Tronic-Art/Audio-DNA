# Builder Notebook — Audio-DNA

<!-- Accumulated Builder knowledge. Each Builder reads this and appends discoveries. -->

(No entries yet. First normalization session 2026-05-18.)

## 2026-05-18 — Eyes + Accessibility Inspection Stack

### How Harmony Inspects Audio-DNA

Three complementary inspection paths, all scriptable:

**1. Eyes API (visual output + engine state)**
- Port 7070 (ApiServer, always on) or 8080 (TestServer, test-mode only)
- Capture rendered frame: `POST /api/render_frame {"output_path": "/tmp/frame.png", "time": 0.0}`
- Query state: `GET /api/status` → FPS, BPM, beat phase, genre
- Query composition: `GET /api/composition` → full deck/layer/clip hierarchy as JSON
- Query features: `GET /api/features` → all audio analysis values
- Python client: `tests/visual/vj_controller.py` (VJAppController class)
- Vision comparison: `tests/visual/vision_check.py` (PSNR/SSIM)

**2. ax_inspector.py (JUCE UI component tree)**
- Reads macOS accessibility tree via AXUIElement API (pyobjc)
- Zero C++ changes — reads from outside the process
- Returns: component hierarchy with roles, titles, values, positions, sizes
- CLI: `python tests/visual/ax_inspector.py --app "Audio-DNA" --depth 5`
- Import: `from ax_inspector import inspect_app; tree = inspect_app("Audio-DNA")`

**3. Melatonin Inspector (manual debugging)**
- Build with: `cmake -DAUDIODNA_BUILD_INSPECTOR=ON`
- Toggle at runtime: Cmd+Shift+I
- Visual only, no export API — for Boris's live debugging

### For Tester Agents
When validating UI changes, use this sequence:
1. `GET /api/status` — confirm app is running
2. `POST /api/render_frame` — capture current visual state
3. `python tests/visual/ax_inspector.py` — capture UI component tree
4. Compare render against expected visual state
5. Compare component tree against expected UI structure

## 2026-05-18 — conftest autouse fixture blocks non-Eyes tests
**Files:** tests/visual/conftest.py, tests/visual/test_ax_inspector.py
**Note:** The conftest.py in tests/visual/ has an `autouse=True` fixture `reset_between_tests` that depends on the `app` fixture, which skips all tests when the Audio-DNA executable is not built. Any new test file in tests/visual/ that does NOT need the running app must override both `app` and `reset_between_tests` fixtures locally to avoid being skipped.
**Valid while:** tests/visual/conftest.py still has autouse=True on reset_between_tests

## 2026-05-18 — set_effect_chain and state endpoints ported to ApiServer
**Files:** src/api/ApiServer.cpp, src/api/ApiServer.h
**Note:** POST /api/set_effect_chain and GET /api/state are now available on port 7070 (ApiServer, always-on) in addition to port 8080 (TestServer, test-mode only). ApiServer version wraps state response in {"ok": true} pattern unlike TestServer's raw JSON. The param write uses direct assignment (fx->getParam(p).value = val) matching ApiServer style, not fx->setParamValue() as in TestServer. ApiServer.cpp is at exactly 900 lines — budget ceiling. Melatonin inspector FetchContent is broken (module header not found); build with -DAUDIODNA_BUILD_INSPECTOR=OFF to work around.
**Valid while:** both ApiServer.cpp and TestServer.cpp contain these endpoints

## 2026-07-17 — triage-execution session learnings (lane-B capture, secondary)

- **Launch-context gate trap**: direct binary exec of the app from an agent shell hangs
  pre-UI (alive, windowless, no sockets) — always `open` the .app for behavioral gates.
  (Also in gotchas.md — canonical.)
- **Don't prescribe concurrency idioms in packets — prescribe the stress test**: packet
  suggested double-buffer for the waveform fix; builder's own torn-read stress test
  proved it tears under reader-lapping and shipped a seqlock instead. The test
  requirement, not the idiom suggestion, produced the correct fix.
- **Probe the variable the write path touches**: /api/status masterLevel reads
  renderer_.getMasterLevel() while OSC /master writes composition_.masterOpacity —
  first probe false-alarmed. Trace write target → pick readback endpoint.
- **Combine contending lanes**: B+D shared MainComponent.cpp + the build dir → one
  combined builder beat two parallel ones (zero races, zero retries).
- **Shared-doc consolidation protocol validated**: parallel builders leave shared-file
  (APP-INVENTORY) edits UNCOMMITTED + flag them; a dedicated doc-sync lane reconciles
  drift (caught 113-vs-114 test count) and commits once. Adopt for all multi-builder waves.
- **Force-add hygiene (process rule)**: force-adds of gitignored-but-tracked-policy files
  get their OWN commit + explicit commit-message mention — never swept into a feature
  commit (W1-A reviewer finding).
