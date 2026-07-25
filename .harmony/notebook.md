# Builder Notebook — Audio-DNA

<!-- Accumulated Builder knowledge. Each Builder reads this and appends discoveries. -->

## 2026-07-20 — Renderer video/sequence/image resources are keyed by clip id; only video content-swaps
**Files:** src/render/Renderer.cpp (videoPlayers_), src/render/CompositorEngine.cpp:1047
**Note:** Videos (videoPlayers_[id]) and image sequences (imageSequences_[id]) are keyed by clip id and NEVER closed. Static images are keyed by FILE PATH via getKeyTexture(clip.mediaFile) — self-healing on undo. Only VIDEO can be content-swapped under an EXISTING id: kClipReplaceContent calls openVideoForClip(existing->id, newFile), overwriting the player while replaceContent keeps the id. So a reconnect-if-MISSING media guard misses replace-undo (player exists → skip → decodes wrong file). Fix: compare loaded file vs clip.mediaFile (Renderer::getVideoPlayerFile + VideoPlayer::getFile + pure needsVideoReopen() in core/MediaReconnect.h), reopen on mismatch. Sequences can't hit it (replace produces only Image/Video; sequences always get a fresh s_nextClipId).
**Valid while:** replaceContent reuses the clip id and video players are keyed by id

## 2026-07-19 — Undo v1 Step 2: clip commands decoupled from Renderer via hooks
**Files:** src/core/ClipCommands.h, src/core/UndoService.h, tests/test_undo_commands.cpp
**Note:** Spec §7 wants commands unit-tested headless against a bare Composition. Achieved by making commands (SetClipCmd/ToggleClipLockCmd) depend ONLY on injected std::function hooks — `ClipLayerResolver` (re-resolve Layer by coord) + `ClipMediaHook` (video/seq reconnect) — NOT on concrete Renderer. The app wires hooks from UndoService+renderer; tests wire a bare-Composition resolver + no-op media. Also made `UndoService::resolve*/setCollaborators` INLINE in the header (renderer-free, only touch Composition) so `test_undo_commands` links WITHOUT UndoService.cpp/Renderer (which pulls juce_opengl + CompositorEngine + video/syphon — too heavy for a unit test). Pattern to reuse for step 3-8 command tests.
**Valid while:** ClipCommands.h uses hook typedefs and UndoService resolvers stay inline

## 2026-07-19 — Wrapping entry sites: capture before → mutate → capture after → perform (idempotent re-apply)
**Files:** src/MainComponent.cpp (handleFileDrop, onEffectDropped, kClipClear, etc.)
**Note:** UndoManager has only perform() (executes). To wrap an existing handler without double-side-effects: capture `before` at the TOP (before any mutation), let the existing code mutate + do its rich UI/player setup, capture `after`, then perform() a SetClipCmd(before,after). perform()→execute()→apply(after) RE-APPLIES after — this is idempotent for SetClipCmd (cell already == after; media hook reconnect no-ops since player already open). No UndoManager API change needed. Multi-cell gestures (multi-FX-to-empty-cells, multi-select clear) use CompositeCommand via pushClipEdits()/pushCommands() which guards isEmpty() before perform.
**Valid while:** UndoManager exposes only perform() and SetClipCmd.apply is idempotent

## 2026-07-19 — Undo v1 Step 1: GL-fence validated + headless-test MessageManager assert
**Files:** src/core/UndoManager.cpp, src/core/UndoService.cpp, tests/CMakeLists.txt
**Note:** (1) GL fence empirically validated: blocking `glContext_.executeOnGLThread(noop,/*block*/true)` from the MESSAGE thread does NOT deadlock the render thread (setComponentPaintingEnabled(false) → GL thread never takes the msg lock). 100 iters, max 15.6ms / mean 2.8ms round-trip, app stayed responsive (spec risk #9 resolved). (2) `jassert(MessageManager::existsAndIsCurrentThread())` would ABORT headless Catch2 tests (no MM instance) if ever built Debug — guard it `getInstanceWithoutCreating()==nullptr || existsAndIsCurrentThread()`. (3) MessageManager lives in juce_events; test_composition only linked juce_core+juce_graphics, so any UndoManager.cpp use of MessageManager requires adding `juce::juce_events` to that test target.
**Valid while:** UndoManager/UndoService exist and tests build Release with these link libs

## 2026-07-19 — App launch can transiently stall in CoreAudio device init (TCC), NOT a code bug
**Files:** src/MainComponent.cpp (AudioDeviceManager::initialiseWithDefaultDevices in ctor)
**Note:** First `open` of the .app sometimes hangs in the constructor at `AudioDeviceManager::initialiseWithDefaultDevices` → CoreAudio `start()` → HALC_ProxyObject::SetPropertyData (TCC/coreaudiod). Symptom is identical to the documented direct-exec hang: process alive, ZERO windows, ZERO listening sockets, no port 7070. Fix: `pkill -9 -f Audio-DNA` and re-`open` — the second launch bound port 7070 in ~4s. `sample <pid>` on the stuck process shows the exact ctor frame, which is the fast way to distinguish an env/audio stall from an app-code hang.
**Valid while:** MainComponent constructs AudioDeviceManager before finishing init

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

## 2026-07-22 — lane-B meta-learnings (Undo v1 s4-7 secondary)
- **pre-declared-budget-rider**: declare the disposition rule ("beyond-NIT finding → HOLD commit") in the ledger AT DISPATCH TIME, before results exist — turns a mid-flight judgment call into a pre-committed policy and removes rationalization pressure when the finding lands. Applied s7; fired correctly.
- **remedy-b-doc-truth**: when a MAJOR is "code comments overclaim vs a PRE-EXISTING mechanism gap", the NIT-cost remedy is comment-truth + tracked follow-up, not smuggling a shared-path fix into the step. Ask the reviewer to rule on the disposition explicitly and INVITE overrule — independence is worth more than speed.
- **reviewer-rotation-at-2-packets**: rotate reviewers on the same load rule as builders (~2 full packets + 1 targeted); fresh eyes caught that a claimed "precedent" (headless-silent invariant violation in tests) never actually existed.
- **env-wedge-single-recheck**: when an env blocker (coreaudiod stall) worsens with remedy cycles, switch to single-attempt rechecks per gate + a hard stop — retry loops actively degrade the environment (4-attempt evidence, sample-verified).

## 2026-07-25 — lane-B meta-learnings (Undo v1 s8-9 secondary, lane close)
- **screencapture-tcc-diagnosis**: headless app-launch "stalls" can be an invisible TCC dialog — `screencapture -x /tmp/x.png` + image read sees what sample/lsof/log probes cannot. The 2-session "coreaudiod wedge" was an unanswered mic prompt. (Also log-event'd mid-session; universal candidate.)
- **codesign-check-on-recurring-tcc**: when a TCC prompt seems to re-fire "randomly", run `codesign -dv` — ad-hoc signing (no TeamIdentifier) changes cdhash per rebuild → TCC re-prompts every build. Durable fix = stable signing identity (Boris-ratified for this repo).
- **pre-adjudicated-trivial-fix**: reviewer-PRESCRIBED comment-truth edits skip the re-review pass (edit is pre-adjudicated); Harmony's gate re-run covers. Used twice this session (s8 MINOR, s9 MINOR) at near-zero cost — but ONLY for doc-only, reviewer-verbatim edits.
- **content-verify-already-covered-claims**: a builder's "already covered elsewhere, not duplicating" claim gets CONTENT-verified (read the covering test), not existence-verified (grep the name) — receiver disk-verify + reviewer both re-checked s9's A1 claim before accepting non-duplication.
- **user-initiated-vs-autonomous undo doctrine**: remote gestures (REST/OSC/MIDI pad hits) are USER actions → undoable through the shared handlers; autonomous mutations (autopilot, genre-auto) NEVER create commands. Contrast with deck-switch (step 6) where spec put remote call sites OUTSIDE the wrap — the spec row governs per-op; check it, don't assume a global rule.
