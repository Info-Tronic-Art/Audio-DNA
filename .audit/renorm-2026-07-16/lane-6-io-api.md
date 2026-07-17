# Lane 6 Census — API / Output / Recording / Core / Main

Scope: `src/api/`, `src/output/`, `src/recording/`, `src/core/`, `src/Main.cpp`
Method: read-only source audit. Evidence = file:line. No build/run.
Date: 2026-07-16

---

## 1. CENSUS

### 1a. REST API — Production server (`src/api/ApiServer.cpp`, port 7070, always-on)

Registered in `ApiServer::setupRoutes()` (ApiServer.cpp:99-204). **22 endpoints** (CORS
via `set_post_routing_handler`, ApiServer.cpp:102-106; no explicit OPTIONS route).

| # | Method | Path | Params (JSON body / none) | What it does | Handler |
|---|--------|------|------|------|------|
| 1 | GET | /api/health | — | ok, status, version 0.1.0, fps, effects_count | :208 |
| 2 | GET | /api/status | — | fps, frameTimeMs, masterLevel, activeDeck + bpm/phase/genre/energy from FeatureBus | :219 |
| 3 | GET | /api/composition | — | full deck→layer→clip tree (opacity, blendMode, clips, etc.) | :245 |
| 4 | POST | /api/trigger_clip | layer, column | async `onTriggerClip` callback → MainComponent | :305 |
| 5 | POST | /api/trigger_column | column | async `onTriggerColumn` callback | :327 |
| 6 | POST | /api/set_param | effect, param, value, [layer, column] | set clip-effect param (if layer/col) or global-chain param | :348 |
| 7 | POST | /api/set_layer_opacity | layer, opacity | sets active deck layer opacity | :432 |
| 8 | POST | /api/switch_deck | deck | async `onSwitchDeck` callback | :462 |
| 9 | POST | /api/snapshot | — | `renderer_.takeSnapshot()` (blocks), returns file path | :483 |
| 10 | GET | /api/bpm | — | bpm, beatPhase, barPhase, phrasePhase, beatInBar, barCount | :493 |
| 11 | POST | /api/set_bpm | bpm | **NO-OP STUB** — validates then just returns ok (ApiServer.cpp:521 comment) | :510 |
| 12 | GET | /api/features | — | full FeatureSnapshot dump (rms, spectral, bands[7], chroma[12], genre…) | :525 |
| 13 | POST | /api/inject_features | any FeatureSnapshot fields | writes to FeatureBus triple buffer (test/automation injection) | :566 |
| 14 | POST | /api/load_image | filepath | `renderer_.loadImage()` + 100ms sleep | :612 |
| 15 | POST | /api/load_source | source_type, [params] | `renderer_.setActiveSource()` | :638 |
| 16 | POST | /api/set_effect | name, enabled, [params] | enable/disable + set params on global-chain effect by name | :665 |
| 17 | GET | /api/effects | — | list global-chain effects (name, enabled, params) | :719 |
| 18 | GET | /api/sources | — | list registered source ids (id, name, category) | :750 |
| 19 | POST | /api/render_frame | output_path, [time] | `renderer_.captureFrame()` to path | :769 |
| 20 | POST | /api/reset | — | clearImage + clearActiveSource + disable all effects | :788 |
| 21 | POST | /api/set_effect_chain | effects[] {name, params} | batch: disable all, enable+configure requested | :802 |
| 22 | GET | /api/state | — | fps, frame_time_ms, master_level, effects[] (incl dry_wet/default), decks | :869 |

Callbacks wired in MainComponent.cpp:1117-1124 (onTriggerClip/Column/SwitchDeck/Snapshot).
Constructed MainComponent.cpp:1106-1116 with port 7070. `sessionRecorder_` is passed to the
ctor (ApiServer.cpp:35) but **never referenced by any handler** — dead dependency.

### 1a-bis. Eyes TEST server (NOT in this lane — lives in `src/test/TestServer.cpp`, port 8080)

Out of lane (src/test/, not src/api/). Noting for the "does it live here?" question: **it does
not**. Separate class `TestServer` (src/test/TestServer.h/.cpp, ~936 lines). **17 endpoints**
(TestServer.cpp:90-155), gated by CMake `AUDIODNA_BUILD_TEST_SERVER=1` (CMakeLists.txt:380-382,
OFF by default) + `--test-mode` runtime flag (Main.cpp:14-27; MainComponent.cpp:1088-1103). Binds
localhost. Superset/variant routes: health, load_image, set_effect, set_effect_chain,
inject_features, render_frame, state, reset, load_source, update_source_params, sources,
load_milkdrop_preset, signals, add_route, remove_route, routes, set_macro. Distinct from the
production API (adds routing/signals/milkdrop/macro; production adds composition/trigger/bpm/etc).

### 1b. Outputs (`src/output/`)

| Class | File | Status | Evidence |
|-------|------|--------|----------|
| SyphonOutput | SyphonOutput.h/.mm | **IMPLEMENTED but DEAD-WIRED** — full Obj-C++ `SyphonServer` wrapper (.mm:32-81); `__has_include(<Syphon/Syphon.h>)` compile-gate (.mm:10-17); non-macOS stub (.h:59-69). BUT `publishTexture()` is **never called** anywhere, and the instance is **never `init()`'d and never `setEnabled(true)`'d**. See health note. | .mm:51-61, MainComponent.cpp:390 |
| SyphonInput | SyphonInput.h/.mm | **IMPLEMENTED but ORPHANED** — full `SyphonClient` wrapper, listServers/connect/getLatestTexture all real (.mm:27-163). BUT **zero references anywhere outside src/output/** — never instantiated/wired. | grep: no refs outside output/ |
| SpoutOutput | SpoutOutput.h | **STUB** — header-only, all methods no-op, `isInitialized()` always false (.h:18-28). Never referenced. | SpoutOutput.h:18-28 |
| NdiOutput | NdiOutput.h | **STUB** — `init()` always returns false, sendFrame/shutdown are `// TODO` even under `AUDIODNA_HAS_NDI` (.h:30-56). Never referenced. | NdiOutput.h:30-60 |
| NdiInput | NdiInput.h | **STUB** — listSources returns {}, connect returns false, getLatestFrame returns nullptr (.h:25-63). Never referenced. | NdiInput.h:25-63 |

Fullscreen output: NOT in this lane — implemented in `src/ui/OutputWindow.h/.cpp` (real,
separate output window / secondary-display path). Noted for completeness only.

### 1c. Recording (`src/recording/`)

| Class | File | Status | Notes |
|-------|------|--------|-------|
| VideoRecorder | VideoRecorder.h/.cpp | **REAL, LIVE** | FFmpeg H.264(libx264)/ProRes(prores_ks)/MJPEG (VideoRecorder.cpp:205-208, 243-262). Triple-buffered GL readback (kNumBuffers=3, .h:91-101); GL thread `glReadPixels`→rotating CPU buffer, drops frame if buffer busy (no mutex, counts droppedFrames_, .cpp:101-139); encoder thread sws_scale RGBA→YUV + FFmpeg encode (.cpp:141-178, 370-419); vertical flip via negative stride (.cpp:377-378). Wired: Renderer::submitFrame at Renderer.cpp:648-649; start/stop at MainComponent.cpp:3129-3152, 3863-3877 (Output menu, H.264, saves to ~/Documents/Audio-DNA/Recordings). Video only, no audio (.h:34-35). |
| PNG snapshot | (Renderer) | **REAL, LIVE** | `Renderer::takeSnapshot()` (Renderer.cpp:1540) / `captureFrame()` (Renderer.cpp:1424). Exposed via POST /api/snapshot (:483), OSC, and MainComponent bindings (MainComponent.cpp:1122,1143,3123,3853). Renderer file, not this lane, but is the recording-adjacent surface. |
| SessionRecorder | SessionRecorder.h/.cpp | **PARTIAL — capture near-empty, playback DEAD** | 7 EventTypes + 7 record* methods + JSON save/load all implemented (.cpp:21-104, 165-279). BUT only `recordClipTrigger` is ever called (MainComponent.cpp:2472). The other 6 record* methods (ParameterChange/Column/Macro/Transport/EffectToggle/Cuepoint) are **never called** → captured session = clip triggers only. `advancePlayback()` is **never called anywhere** → `startPlayback()` (RecordPanel.cpp:38) fires nothing. save/load wired in RecordPanel.cpp:59,78. |

### 1d. Core (`src/core/`) — undocumented dir in CLAUDE.md source tree

| Class | File | Role | Status |
|-------|------|------|--------|
| Command | Command.h | Abstract base for undo/redo command pattern: execute()/undo()/description()/canMergeWith/mergeWith (Command.h:7-25). | **INFRA ONLY** — **zero concrete subclasses exist** (grep `: public Command` = none). |
| UndoManager | UndoManager.h/.cpp | Linear undo/redo history, merge support, kMaxHistory=500 (.cpp:3-89). | **DEAD** — `perform()` is **never called** (grep across src = none) → history always empty. `undo()/redo()` wired to keys (MainComponent.cpp:1642-1644, 2754-2757) are permanent no-ops. |

### 1e. App lifecycle (`src/Main.cpp`)

`AudioDNAApplication : juce::JUCEApplication` (Main.cpp:4). App name "Audio-DNA", version
"0.1.0", single-instance (Main.cpp:7-9). `initialise()` parses `--test-mode` and
`--test-port=` (default 8080) (Main.cpp:14-27), then creates `MainWindow` → `MainComponent`
(Main.cpp:26, 48). Native macOS menu bar (Main.cpp:56-62); opens maximized to primary display
(Main.cpp:64-75); resize limits 1280x720–3840x2160. `START_JUCE_APPLICATION` (Main.cpp:99).

---

## 2. DOC-VERIFY (lane claims vs source)

| Doc:line | Claim | Code truth |
|----------|-------|-----------|
| CLAUDE.md:226 | `SyphonOutput.h/.mm ✅ macOS Syphon server (zero-copy GPU texture sharing)` — ✅ = working | Wired to Renderer but `publishTexture()` NEVER called; instance never init()/setEnabled → **publishes nothing**. Non-functional. (Renderer.cpp: no syphonOutput_ use; grep publishTexture = 0 callers) |
| CLAUDE.md:227 | `SyphonInput.h/.mm ✅ macOS Syphon client` — ✅ = working | Class real but **never instantiated/wired** (zero refs outside src/output/). Orphaned. |
| CLAUDE.md:577 | `Undo/redo from the start (Command pattern)` | Infra only; **zero Command subclasses, perform() never called** → undo/redo are no-ops. (UndoManager.cpp; MainComponent.cpp:1642) |
| CLAUDE.md:576 | `Session recording (…capture + JSON save/load + playback)` | Capture = clip triggers only (6/7 record* unused, MainComponent.cpp:2472); **playback dead** (advancePlayback never called). save/load real. |
| CLAUDE.md:1113 | Lists 20 production endpoints, "20+ endpoints" | Actual = **22**; doc list **omits /api/set_effect_chain (:202) and /api/state (:203)**. Header CLAUDE.md:222 "20+" is fine. |
| CLAUDE.md:1113 | `/api/set_bpm` listed plainly among functional endpoints | **No-op stub** (ApiServer.cpp:510-522, comment :521). CLAUDE.md does not flag it. |
| CLAUDE.md:228-230 | Spout/NDI = stubs (no ✅) | Accurate. |
| CLAUDE.md:162 | httplib serves API 7070 + Eyes 8080 | Accurate (CMakeLists.txt:377,382). |
| docs/FEATURE_INVENTORY.md:110,120,316,359,348 | Undo zero-subclasses; set_bpm no-op; Spout/NDI stubs | **Accurate** — this doc is honest; does NOT catch Syphon-output-never-published or session-playback-dead. |
| docs/FEATURE_INVENTORY.md:327-328 | Test server = 17 handlers, port 8080, gated | Accurate (TestServer.cpp: 17 routes). |
| .harmony/FEATURES.md:2137 | "ApiServer holds a reference but exposes no REST endpoints for session recording" | Accurate — matches dead `sessionRecorder_` dependency. |
| .harmony/FEATURES.md:1324,1331 | REST API "20+ endpoints" | OK as ">=20"; exact = 22. |

---

## 3. HEALTH READ

### SOLID (real + live)
- Production REST API: 22 endpoints, live on 7070, CORS, callbacks wired. (1 stub endpoint — see below)
- VideoRecorder: real FFmpeg pipeline, triple-buffered, wired into render loop + Output menu.
- PNG snapshot (Renderer): real, exposed via REST/OSC/menu.
- UndoManager/Command/SessionRecorder *mechanisms*: correctly implemented in isolation.
- Main.cpp lifecycle: clean.

### FLAGGED (stub/dead/orphaned/presented-as-working)
1. **SyphonOutput never publishes** — wired to Renderer (MainComponent.cpp:390) but `publishTexture()` has **0 callers**, and the instance is never `init()`'d or `setEnabled(true)`'d. Marked ✅ in CLAUDE.md:226. Undocumented in all 3 docs. **Highest-impact: a headline "Syphon output" feature emits nothing.**
2. **UndoManager dead** — `perform()` never called, zero Command subclasses; undo/redo keys are no-ops. Contradicts CLAUDE.md:577. (Documented in FEATURE_INVENTORY:110,120.)
3. **SessionRecorder playback dead + capture near-empty** — `advancePlayback()` never called (playback fires nothing); only clip-triggers captured (6/7 event types unused). Contradicts CLAUDE.md:576 "playback". Undocumented.
4. **SyphonInput fully orphaned** — real class, never instantiated. Marked ✅ CLAUDE.md:227.
5. **`/api/set_bpm` no-op stub** — returns ok, changes nothing (ApiServer.cpp:521). Listed as a normal endpoint in CLAUDE.md:1113. (Documented in FEATURE_INVENTORY:316.)
6. **ApiServer `sessionRecorder_` dead dependency** — ctor takes it (ApiServer.cpp:35), no handler uses it; no REST surface for session recording. (Documented FEATURES.md:2137.)

Minor: SpoutOutput/NdiOutput/NdiInput are honest stubs (correctly labeled). No explicit OPTIONS
route on the production API — CORS preflight relies on post_routing_handler headers only
(low priority).

### WORKS vs DOESN'T (bottom line)
- WORKS: production REST control (21 of 22 endpoints do real work), video recording, PNG snapshot, app boot.
- DOESN'T: Syphon output (never publishes), Syphon input (never wired), undo/redo (no commands), session playback (never advanced) + full session capture (only clip triggers), /api/set_bpm, Spout/NDI (stubs).
</content>
