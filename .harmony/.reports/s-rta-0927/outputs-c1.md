## BUILDER REPORT -- lane outputs-c1 (plan5 slice C1: the composition on one output display, through shared IOSurface frames)

STATUS: DONE_WITH_CONCERNS
RESULT: Plan5 slice C1 is built on `lane/outputs-c1-0927`. The main Renderer copies the finished canvas into four shared IOSurface slots, and only while an output is live. The Output window, opened through today's three doors, shows that canvas letterboxed. It sits at normal window level and can never become the key window. The legacy OutputRenderer and its fan-out sites are gone. Probing is offscreen and never opens a window. Commits: C1a `521a16c`, C1b `2f13bd7`, C1c `14d3aea` plus the fix `7ae8e62`, C1d `8ebfda0` (docs), and a final commit with this report and its evidence.
FACTS: `/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0927-w1/src/output/SharedFrameSet.h` (FrontFrame + SurfacePool kSlots=4 + SharedFrameSet); `/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0927-w1/src/output/OutputPresenter.cpp` (presentSharedFrame, the one presenter); `/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0927-w1/src/ui/OutputWindow.cpp` (Presenter, windowIgnoresKeyPresses override, openOnDisplay); `/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0927-w1/src/render/Renderer.cpp` (publishToOutputs, 3 call sites + releaseGL); `/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0927-w1/src/test/TestServer.cpp` (set_output_tap, output_probe, state.outputs); `/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0927-w1/tests/test_output_law.cpp`; `/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0927-w1/tests/test_shared_frame_gl.cpp`; `/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0927-w1/tests/test_surface_pool.cpp`; `/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0927-w1/.harmony/probe-outputs.py`; evidence `/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0927-w1/.harmony/.reports/s-rta-0927/outputs-c1-evidence/`
METHOD: I re-anchored every C1 dependency of the plan at main first (drift: no structural break). Each new test and probe row was run RED first on the base tree or the pre-change app. Guards were checked on mutated COPIES only, and the deliverable sha256 was unchanged before and after. ctest ran serially after each commit. The new probe was run on the base app and the lane app. All 14 existing probes were re-run with no threshold edits. The five Tier-1 files were run on both apps and their failure sets compared. Screen safety: no run ever opened an Output window. A Quartz sampler watched the window list during every live run, and the full window list was checked after every quit.
CONFIDENCE+VERIFY: HIGH for the frame path. Evidence: byte-identical offscreen round trips across three CGL contexts, and in the running app output_probe == render_frame (d 0.000). MEDIUM for the window behaviour (never key, frozen-not-black, refresh-rate pacing, menu-bar strip): the law forbids any automated gate from opening the window, so those are Boris checks 1-2 and 5-9. Verify: `ctest --test-dir build-lane` (690/690, including test_output_law, test_shared_frame_gl and test_surface_pool); `OUTP_APP=<lane app> bash .harmony/probe-outputs.sh` under the live lock (`PY 10 PASS / 0 FAIL`, `PROBE-OUTPUTS GREEN`; on the pre-change app `PY 1 PASS / 9 FAIL`); look at `outputs-c1-evidence/png/`.
UNKNOWNS/NOT-DONE: (1) The window itself was never opened, by law. Focus, frozen frame on preview-hide, pacing, tearing and the menu-bar strip on a projector are INFERRED from JUCE source and need Boris checks 1-2, 5-9. (2) C2/C3 are out of scope: N windows, ticked menu, TopBar button, the ⌘` and ⇧⌘⎋ keys, plain Esc semantics, hot-plug and persistence. (3) plan 8.3 `target()` (DisplayInfo) is left for C2, which is its only user. (4) o_tap_cost is two noisy 5-s samples per size (section 8), not a statistic.
NUANCE: At main the canvas's last writer is the deck transition (the F2 fix moved it after master opacity), and presentCanvas now runs BEFORE the recorder, Syphon and capture. So the tap sits right after the transition and before presentCanvas, not "after master opacity" (drift item D3). CLAUDE.md is at its 25,000-byte cap: the render-thread note went to `docs/claude/rendering.md` and the index line is short (section 9). The pre-change and lane Tier-1 runs have IDENTICAL failure sets (8 of 13, all pre-existing).
HANDOFF-NEEDS: none

INBOX-RECHECK: none (no addendum message received during the lane)

### 1. DRIFT CHECK (first; the plan was written at 5285662, this lane branches from main 4b0c39a, 50 commits later)

Verdict: NO STRUCTURAL BREAK. Every name and meaning C1 rests on exists at main. Two assumptions moved (D3, D11), one plan statement was already false (D12), and one plan figure is stale (D9). None changes the architecture. Line numbers below are at main 4b0c39a (`git show main:<file> | grep -n`).

| # | plan assumed (plan5-final.md) | main has | effect on C1 |
|---|---|---|---|
| D1 | `canvasFBO_` + canvas ints (plan 15) | `Renderer.h:394-397` `canvasFBO_`/`canvasTex_`/`canvasW_`/`canvasH_`; `Renderer.cpp:272` `const RenderGeometry::Size canvas = resolveCanvas(...)`; `:311` `ensureCanvasFBO(canvas.w, canvas.h)` | same meaning; the tap passes `canvas.w/h` |
| D2 | `RenderGeometry::fitCanvas` (plan4 S1) | `src/render/RenderGeometry.h:33-48`, constexpr, same semantics (largest centred rect of the canvas aspect; empty on any input <= 0) | used by the presenter and the probe geometry row |
| D3 | "`presentCanvas` LAST in the frame; the tap after the master-opacity block and before the recorder submit" (8.5, 15) | MOVED: the F2 fix (canvas lane) runs the cross-deck transition AFTER master opacity (`Renderer.cpp:681` opacity, `:707-760` transition), and `presentCanvas` now runs at `:762`, BEFORE the recorder `:827`, Syphon `:834` and capture `:837`. presentCanvas still only READS the canvas (it draws into the window FBO) | not structural. The tap goes after the canvas's LAST writer (the transition) and before presentCanvas (`Renderer.cpp` publishToOutputs call after the transition block). It is still before recorder/Syphon/capture/present, and it sits inside the measured frame window, so its cost shows in frame_time_ms / gpu_time_ms |
| D4 | the two early paths (nothing-to-render, no-content) after the canvas clear | `Renderer.cpp:321-329` and `:630-636`, both after the clear `:316-320`, both `processPendingCapture` + `presentCanvas` | the tap is added to both, before the capture |
| D5 | `set_composition_params outputWidth/outputHeight` (S8) | `src/test/TestServer.cpp:214` route, `:1376-1391` handler (8080, test mode) | used by `o_probe_survives_resolution_change` |
| D6 | `render_frame` = the canvas PNG | `Renderer::captureFrame` `:2009`, `processPendingCapture` reads `canvasFBO_` canvasW_ x canvasH_ (`:2086`) | the probe oracle, unchanged |
| D7 | three open paths into `openOutputOnDisplay` (menu item i / TopBar combo / ⌘F) | menu `MainComponent.cpp:5946-5949` (`kOutputFullscreenBase`), TopBar combo `:582-588`, ⌘F `:3463-3476`, PLUS the hidden MainComponent combo `:435-444` (hidden at `:2482-2483`); `openOutputOnDisplay` `:3772`, `closeOutput` `:3794` | all four reach the same two functions; C1 changes only those two |
| D8 | the Esc branch (`:3232` then) | `MainComponent.cpp:3410-3419`, same shape (closes the output, swallows Esc) | untouched in C1 (its change is C2, 7.3). It still works, and better: the app keeps the keyboard while the output covers it |
| D9 | 11 `loadImage` fan-out sites + 1 camera (8.8) | 9 fan-out sites (`:728-729, 1545-1546, 2669-2670, 3573-3574, 3935-3936, 3965-3966, 4350-4351, 4560-4561, 4620-4621`) + 1 inside `openOutputOnDisplay` (`:3787-3788`) + camera `:3894-3896`. One site went with plan6 `01ad154` (Deck Save/Load retired) | all deleted |
| D10 | plan4 S7 "delete again if written" (8.8) | S7 WAS built: `OutputRenderer(..., const Composition*)`, letterbox in `OutputWindow.cpp:153-170` | deleted with OutputRenderer |
| D11 | `currentImageFile_` "STAYS -- another reader: `deck.imageFile = currentImageFile_`" (8.8) | that reader is gone (plan6 `01ad154`). After C1 the member has no reader | left in place (write-only; comment updated, `MainComponent.h:361`), so this lane does not add a 20-line churn in MainComponent while other lanes edit it; see ISSUES 3 |
| D12 | shutdown law: output detach "`:2351`, before the main renderer's detach `:2346`" and section 14 "servers stop -> outputs close -> main renderer detach" | self-contradictory in the plan (2351 > 2346). At main: servers stop (`:2279`, `:2292`) -> main renderer detach `:2305` -> output detach `:2311-2312` -> `outputWindow_.reset()` `:2329` | kept EXACTLY where it is (`detachGL()` in place of `getRenderer().detach()`); order unchanged. With IOSurfaces the two detaches are order-independent (the output holds CFRetains; no GL name is shared) |
| D13 | "IOSurface is already linked `CMakeLists.txt:539`" | only inside `if(APPLE AND AUDIODNA_BUILD_SYPHON)` (`:446-540`) | `-framework IOSurface` added for APPLE unconditionally (a Syphon-OFF build would not link) |
| D14 | (implicit) the TestServer is built | `option(AUDIODNA_BUILD_TEST_SERVER ... OFF)` (`CMakeLists.txt:62`); the main checkout's build/ is ON | build-lane configured `-DAUDIODNA_BUILD_TEST_SERVER=ON` (the STEP 0 configure line had no flag; set_output_tap/output_probe need it) |
| D15 | pitfall number "next free (38 if plan4 ships 36/37)" | `docs/claude/pitfalls.md` ends at 39 | the new pitfall is **40** (a concurrent lane could also claim 40: renumber on merge, notebook rule) |
| D16 | JUCE 8.0.4 facts (section 3) | re-verified in build-lane/_deps/juce-src: `TopLevelWindow::visibilityChanged` `juce_TopLevelWindow.cpp:91-97`; `canBecomeKeyWindow` `juce_NSViewComponentPeer_mac.mm:1186-1189`; `SharedResourcePointer<RenderThread>` `juce_OpenGLContext.cpp:966`; `setSwapInterval (1)` `:660`; mac swap interval 0/1 only `juce_OpenGL_mac.h:227-236`; `addToDesktop` recreates the peer when the flags differ `juce_Component.cpp:376` | hold |
| D17 | `/api/state` handlers | 7070 `ApiServer.cpp:1283`, 8080 `TestServer.cpp:591`; `TestServer::stop()` `:81` runs at `MainComponent.cpp:2292`, before the renderer detach `:2305` | the probe context dies before the frames it reads |

### 2. SUMMARY
The Output window now shows the COMPOSITION. It used to re-render its own effect chain on the legacy image only (F3). The main Renderer copies its final canvas once per frame into `output::SharedFrameSet`, four IOSurface-backed slots. A slot is offered only once its fence has completed. The window's own GL context blits the newest slot, letterboxed, once per refresh of its display. It uses swap interval 0 and no shader. The window is at normal level and carries `windowIgnoresKeyPresses`, so it can never become key. Its bounds are set before it is shown. A source-reading ctest holds that law. The frame path is proven offscreen in two places: a 3-context CGL ctest, and in the running app through a private GL context (8080 `output_probe`). No window was ever opened.

### 3. COMMITS (on `lane/outputs-c1-0927`, base `4b0c39a`)
| commit | what | gate at the commit |
|---|---|---|
| `521a16c` C1a | `src/output/SharedFrameSet.h/.cpp`, `SurfacePool.cpp`, `OutputPresenter.h/.cpp`; `tests/test_surface_pool.cpp`, `tests/test_shared_frame_gl.cpp` | RED by absence (configure: "Cannot find source file: .../src/output/SurfacePool.cpp"); teeth: rotation `% (kSlots - 1)` on a COPY -> `test cases: 8 \| 7 passed \| 1 failed` (slot_rotation, test_shared_frame_gl.cpp:383/:385); ctest `100% tests passed, 0 tests failed out of 680` |
| `2f13bd7` C1b | Renderer tap + releaseGL; OutputWindow rewrite (OutputRenderer deleted, never-key, bounds-first, toFront(false), no keyPressed, deferred close); MainComponent wiring + fan-out deletions + setLiveOutputCount; EffectChain.h / Renderer.h comments (sec 17); CMake sources + IOSurface; `tests/test_output_law.cpp` | RED on main's OutputWindow: `test cases: 10 \| 7 passed \| 3 failed` (toFront(true), setWantsKeyboardFocus(true), windowIgnoresKeyPresses) -- exactly the plan's three; ctest `... out of 690` |
| `14d3aea` C1c | TestServer `set_output_tap` / `output_probe` (+ probe context teardown in stop()); `/api/state.outputs` on 8080 and 7070; `.harmony/probe-outputs.sh/.py/.json` | RED on the pre-change app `PY 1 PASS / 9 FAIL`, `PROBE-OUTPUTS RED`; GREEN on the lane `PY 10 PASS / 0 FAIL`, `PROBE-OUTPUTS GREEN`; ctest 690/690 |
| `7ae8e62` C1c fix | probe-outputs `o_tap_cost`: macOS `pgrep -x clang++` is an invalid regex (never "busy"); pattern `clang\+\+` | the probe row itself (section 8) |
| `8ebfda0` C1d | docs: `CLAUDE.md` (pitfall index 40, see section 9), `docs/claude/{pitfalls,architecture,rendering,integration,testing-eyes}.md`, `.harmony/APP-INVENTORY.md`, `.harmony/specs/outputwindow-arc-design.md`, `.harmony/notebook.md`; this report + evidence | docs only; final ctest at HEAD `100% tests passed, 0 tests failed out of 690` |

Why C1c has a fix commit: the pgrep bug was found while preparing the perf row, after C1c was committed. It is kept separate so as not to rewrite a gated commit.

### 4. RED FIRST (raw lines, verbatim)
- C1a ctests, RED by absence (`outputs-c1-evidence/RED-C1a-absence-configure.txt`): `CMake Error at tests/CMakeLists.txt:2056 (add_executable): Cannot find source file: .../src/output/SurfacePool.cpp`.
- `test_output_law`, final source, run against a copy of `main:src/ui/OutputWindow.{h,cpp}` (`RED-base-tree-test_output_law.txt`): `output law: never takes the keyboard -- no toFront(true)` FAILED, `output law: never takes the keyboard -- no setWantsKeyboardFocus(true)` FAILED, `output law: the peer carries windowIgnoresKeyPresses` FAILED; `test cases: 10 |  7 passed | 3 failed`. The other seven (always-on-top, NSFloatingWindowLevel, kiosk, setFullScreen(true), toggleFullScreen, grabKeyboardFocus, bounds-before-visible) are guards: green on both trees. Each was proven on a mutated copy (`TEETH-test_output_law-guards.txt`): `setAlwaysOnTop (true);` added -> 1 failed; `toFront (true)` with a space -> 1 failed; shown before bounds -> 1 failed; the flag kept only in the jassert -> 1 failed; the flag removed -> 1 failed; the control copy -> `All tests passed (32 assertions in 10 test cases)`; deliverable sha256 identical before and after (`abf52bd2...` / `fc68352b...`).
- probe-outputs on the PRE-CHANGE app `build/AudioDNA_artefacts/Release/Audio-DNA.app` (`RED-base-probe-outputs.txt`): `FAIL  o_probe_matches_canvas: before any publish output_probe answers 409 ok:false (HTTP 404, {'raw': ''})`, `FAIL  o_probe_matches_canvas: set_output_tap on -> HTTP 404`, `FAIL  o_probe_portrait_target: output_probe probe_portrait_1080x1920: HTTP 404`, `FAIL  o_probe_tracks_change: ... HTTP 404`, `FAIL  o_probe_survives_resolution_change: ... HTTP 404`, `FAIL  o_probe_repeat_stable: 60 probes ... (60 bad: ['#0 no frame (HTTP 404)', ...])`, `FAIL  o_state_outputs: http://[::1]:8080 /api/state.outputs = None`, `FAIL  o_state_outputs: http://127.0.0.1:7070 /api/state.outputs = None`, `FAIL  o_state_outputs: after set_output_tap off, 8080 outputs.tap == false (None)`, `PASS  o_no_window_opened: ...` (the guard); `PY 1 PASS / 9 FAIL`, `PROBE-OUTPUTS RED`.

### 5. GREEN (raw lines, verbatim; lane build-lane at `14d3aea`, identical code at C1d)
- `test_surface_pool`: `All tests passed (63 assertions in 5 test cases)`. `test_shared_frame_gl`: `All tests passed (90 assertions in 8 test cases)`. It ran for real on this rig, not SKIPped: roundtrip_byte_identical, nothing-published-is-black, letterbox_geometry, reader_tracks_new_frames, generation_change, reader_destroyed_mid_run, writer_context_loss_keeps_last_frame, slot_rotation.
- `test_output_law`: `All tests passed (32 assertions in 10 test cases)`.
- probe-outputs (`GREEN-lane-probe-outputs.txt`):
  - `PASS  o_probe_matches_canvas: before any publish output_probe answers 409 ok:false (HTTP 409, {'ok': False, 'error': 'nothing published yet (no live output and the tap is off)'})`
  - `PASS  o_probe_matches_canvas: output_probe 1920x1080 == render_frame 1920x1080 (d 0.000 <= 2.0; gen 1 serial 33 slot 0 canvas 1920x1080)`
  - `PASS  o_probe_portrait_target: 1080x1920, picture rect 1080x607 at PNG row 657 (fitCanvas 0,656,1080,607); bars mean RGB 0.000 <= 2.0; d(inner, f0 resized) 0.013 <= 6.0`
  - `PASS  o_probe_tracks_change: after col1 = B the probe follows: d(probe, f0) 29.431 > 20.0 and d(probe, f_B) 0.000 <= 2.0 (serial 141)`
  - `PASS  o_probe_survives_resolution_change: render_frame 1280x720, probe canvas 1280x720, gen 1 -> 2, d(probe, f_720 upscaled) 0.019 <= 6.0`
  - `PASS  o_probe_repeat_stable: 60 probes in 12.00 s, every PNG decoded, none blank, worst d 0.000 <= 2.0 (0 bad: [])`
  - `PASS  o_state_outputs: http://[::1]:8080 /api/state.outputs = {'live': 0, 'tap': True, 'frame_gen': 3, 'frame_serial': 1788, 'canvas_w': 1920, 'canvas_h': 1080}` (7070 identical)
  - `PASS  o_state_outputs: after set_output_tap off, 8080 outputs.tap == false (...)`
  - `PASS  o_no_window_opened: 68 Quartz samples over the run: Output-named Audio-DNA windows [], max on-screen Audio-DNA layer-0 windows 1 (<= 1 = the main window); window names readable: ['Audio-DNA']`
  - `PY 10 PASS / 0 FAIL`, `PASS  no foreign render_frame / output_probe traffic during the run`, `PASS  app terminated`, `PASS  after quit: 0 Audio-DNA windows in the FULL Quartz window list (no output window survives)`, `PROBE-OUTPUTS GREEN`
- The probe PNG and the render_frame PNG are the same file size (138363 B each) and d 0.000: the output shows exactly what render_frame captures.

### 6. EXISTING BATTERY, tests/visual TIER-1, ctest
- ctest serial: base `100% tests passed, 0 tests failed out of 667`; C1a 680; C1b, C1c and HEAD (after C1d) `100% tests passed, 0 tests failed out of 690` (`outputs-c1-evidence/ctest-*-tail.txt`).
- Existing probes on the lane build (`battery-summary.txt`), unchanged, no threshold edits, each under its own live-lock hold, all GREEN: render-state `PY 31 PASS / 0 FAIL`; crossfade `PY 35 PASS / 0 FAIL`; effects-parity `PY 46 PASS / 0 FAIL`; canvas `PY 15 PASS / 0 FAIL` (c_perf_1080 fps mean 106.0, frame_time_ms 1.77, load avg 4.93); deck-clock `PY 10 PASS / 0 FAIL`; fitmode `PY 10 PASS / 0 FAIL`; deck-tabs `6 PASS / 0 FAIL`; step3 `94 PASS / 0 FAIL`; mastersignal `22 PASS / 0 FAIL`; routines `98 PASS / 0 FAIL` (ROUTINES_RECORD_PAUSE=1.8); resync `16 PASS / 0 FAIL`; onset-render `13 PASS / 0 FAIL`; downbeat-level `14 PASS / 0 FAIL`; manual-bpm `22 PASS / 0 FAIL`. NOT run: deck-path, tempo-silence (direct exec), lane3 and finalize-loop (plain `open`), because their launch forms break the rig rules.
- tests/visual Tier-1: exactly `test_sources.py test_effects.py test_audio_reactivity.py test_time_sweep.py test_performance.py` (13 test ids), test-mode `open -g` launch, `AUDIODNA_NO_SPAWN=1 PYTHONDONTWRITEBYTECODE=1 -p no:cacheprovider`:
  - pre-change app: `8 failed, 5 passed in 1099.19s (0:18:19)`; lane: `8 failed, 5 passed in 1139.78s (0:18:59)`.
  - Failure sets are IDENTICAL (`comm`: 0 lane-only, 0 base-only; same 13 ids). NO NEW FAILURE. The 8 failing ids, all pre-existing: `test_audio_reactivity.py::TestAudioFeaturesAffectEffects::test_rms_affects_effects`, `...::TestAudioFeaturesAffectSources::test_rms_bass_beat_affect_sources[Bass]`, `...[Beat Phase]`, `test_effects.py::TestAllEffectParams::test_all_params_have_effect`, `test_effects.py::TestAllEffectsRender::test_all_effects_non_black`, `test_sources.py::TestAllSourceParams::test_all_params_have_effect`, `test_sources.py::TestAllSourcesRender::test_all_sources_non_black`, `test_time_sweep.py::TestSourcesAnimateOverTime::test_sources_change_over_time` (`tests-visual-tier1-{BASE,LANE}-outcomes.txt`). The "~167" in the dispatch counted the wider tests/visual directory; these five files hold 13 aggregate tests.

### 7. SCREEN SAFETY (output_window_opened = false, proven)
- No code path used by any gate opens the window. The only openers are the Output menu, the TopBar/hidden combos and ⌘F, and nothing here drove them: no synthetic input, no AppleScript UI, no composition or deck file with an output display.
- The Quartz window list was sampled for the whole of every probe-outputs run (68 samples on the GREEN run): no Audio-DNA window named "Output", never more than one on-screen Audio-DNA layer-0 window. A separate sampler ran over the whole existing battery: `window sampler 10:27:18..10:48:27: 2493 samples, Output-named Audio-DNA windows [], max on-screen Audio-DNA layer-0 windows 1` (`battery-window-sampler.txt`). After every quit (probe-outputs RED/GREEN/tap-cost, both Tier-1 runs), the FULL window list held 0 Audio-DNA windows.
- The only capture taken is the app's MAIN window by its Quartz id (`screencapture -x -o -l <id>`, `png/main-window.png`). There was no full-screen capture.
- No lldb/dtrace/Instruments. No env-var hook in any build: `strings build-lane/.../Audio-DNA | grep -c AUDIODNA_DEBUG_` = 0 (and `OutputRenderer` = 0).

### 8. o_tap_cost (REPORT; runs only when `pgrep -x clang` / `pgrep -x 'clang\+\+'` are empty; load average printed)
7070 `/api/state`, one 5-s window with the tap off, then on, per size. The lane binary, a static A clip, the laptop display at ~100-110 fps. Raw lines (`o_tap_cost-run1.txt`, `o_tap_cost-run2.txt`):
- run 1: `REPORT o_tap_cost 1920x1080: tap off frame_time_ms 1.107 peak 9.00 gpu_time_ms 0.974 fps 103.6 | tap on frame_time_ms 1.534 peak 9.53 gpu_time_ms 1.285 fps 93.9 | delta cpu +0.427 ms gpu +0.311 ms (load avg 4.74 4.80 4.63)`
- run 1: `REPORT o_tap_cost 3840x2160: tap off frame_time_ms 1.409 peak 9.80 gpu_time_ms 5.446 fps 103.2 | tap on frame_time_ms 1.478 peak 8.84 gpu_time_ms 5.509 fps 112.1 | delta cpu +0.069 ms gpu +0.063 ms (load avg 4.44 4.73 4.61)`
- run 2: `REPORT o_tap_cost 1920x1080: tap off frame_time_ms 1.389 peak 17.47 gpu_time_ms 1.090 fps 96.1 | tap on frame_time_ms 1.638 peak 18.29 gpu_time_ms 1.342 fps 97.1 | delta cpu +0.248 ms gpu +0.252 ms (load avg 4.91 4.95 4.72)`
- run 2: `REPORT o_tap_cost 3840x2160: tap off frame_time_ms 1.297 peak 11.50 gpu_time_ms 5.331 fps 112.8 | tap on frame_time_ms 1.577 peak 11.82 gpu_time_ms 5.770 fps 100.8 | delta cpu +0.279 ms gpu +0.439 ms (load avg 6.09 5.20 4.81)`
Reading: at 1080p the tap costs ~0.25-0.43 ms CPU and ~0.25-0.31 ms GPU per frame. At 4K the two runs disagree (+0.06 / +0.44 ms GPU): the fps swings (93.9-112.8) are display-link and load noise, larger than the tap. Both runs sit inside the plan's INFERRED 0.1-0.3 ms at 1080p / ~1 ms at 4K. The cost is zero with no output live: the tap returns before any GL work. probe-canvas c_perf_1080 on the lane (tap idle) gives frame_time_ms 1.77, which is inside the canvas lane's recorded 1.52-1.82 ms (`.harmony/.reports/s-rta-0926b/canvas.md:346, :472, :573`). The fps is 106.0 at load 4.93, against their 114-118 at load ~3: INFERRED to be load, not measured apart. The per-output PRESENT cost (the window's own blit) cannot be measured without a window: Boris check 9.

### 9. DOCS (C1d) -- deviations from plan section 17, stated
- `docs/claude/pitfalls.md` **Pitfall 40** (full text: normal level, never key via `windowIgnoresKeyPresses`, one shared GL thread so swap interval 0 and no main-context names, IOSurface frames outlive contexts, drop the fence and never glDeleteSync it); `docs/claude/architecture.md` tree line; `docs/claude/rendering.md` (the output tap: after the canvas's last writer, before presentCanvas, early paths, fence-gated slots); `docs/claude/integration.md` (the Output window next to Syphon); `docs/claude/testing-eyes.md` (`set_output_tap`, `output_probe`); `.harmony/APP-INVENTORY.md` (OutputWindow row; 8080 recounted as 28 routes -- the old "17" was stale, 26 at main + 2; shader-table DUP/LAG row RESOLVED); `.harmony/specs/outputwindow-arc-design.md` residuals; `src/render/Renderer.h` + `src/effects/EffectChain.h` comments (done in C1b, where they became false).
- `CLAUDE.md` is capped at 25,000 bytes (notebook, Harmony boot-cost audit). It was 24,989 B at main. Changes: index line `40. Output windows: normal level, never key -- before touching \`OutputWindow\`.`, plus byte-neutral tightening of three Common Build Issues rows and "(verbatim)" dropped from the index header, giving **24,992 B**. NOT done, because of the cap: the plan's render-thread sentence (it is in `docs/claude/rendering.md` instead) and the Key-capabilities wording "output to any number of connected displays". That wording is C2's truth anyway: C1 still opens ONE window, and "fullscreen output to any connected display" stays accurate.

### 10. BORIS CHECKLIST (plan section 12 items 1-2, 5-9, adapted to C1's doors -- C2 adds the ticked menu, ⌘` and ⇧⌘⎋)
1. Plug in the projector. Output menu -> the `Fullscreen: WxH` item of the projector. The projector should show exactly what the lower-left panel shows, with the same black bars, sharp, and the right size (Retina and not). Output -> `Disabled`: it disappears and nothing stays on any screen.
2. Press Cmd+F (or Output -> `Fullscreen: ... (main)`). The picture covers the laptop screen. Click on the picture, then press a clip key: the clip changes, because the picture never takes the keyboard. Press Cmd+F (or Esc) to close it. The menu bar stays usable the whole time.
5. With the projector on, open the big signal bar (the preview hides). The projector should keep the last picture, frozen, never black. Close the bar and it moves again. Same for minimising the app.
6. Quit with the output open (Cmd+Q). The quit should be clean and every screen clean.
7. The top edge: is there a translucent menu-bar strip across the top of the projector picture ("Displays have separate Spaces" is ON)? If yes: System Settings > Control Center > "Automatically hide and show the menu bar: Always" removes it without any app change.
8. 120 Hz laptop + 60 Hz projector with fast motion: it should be smooth, with no stutter and no horizontal seam.
9. The TopBar fps readout with the output on vs off: that is the per-output cost in his numbers.

### FILES CHANGED
- `src/output/SharedFrameSet.h` (new) -- FrontFrame packing, SurfacePool (IOSurface half), SharedFrameSet (GL half); GL-free header.
- `src/output/SurfacePool.cpp` (new) -- the pool: IOSurface generations, 120-publish retire, retainSurface for readers.
- `src/output/SharedFrameSet.cpp` (new) -- publish (fence-gated offer, strict 4-slot rotation, rebind after context loss continues the rotation), releaseGL (drops the fence), bindCurrentGeneration.
- `src/output/OutputPresenter.h/.cpp` (new) -- PresenterGLState + presentSharedFrame (rebind on generation change, re-bind the read FBO every frame, fitCanvas blit).
- `src/ui/OutputWindow.h/.cpp` -- rewritten (OutputRenderer deleted; Presenter; never-key; openOnDisplay; detachGL; onCloseRequested).
- `src/render/Renderer.h/.cpp` -- sharedFrames_/liveOutputs_/outputTapForced_ + accessors; publishToOutputs at 3 sites; releaseGL in openGLContextClosing; comments.
- `src/MainComponent.h/.cpp` -- window built on the shared frames; live count; 9+1+camera fan-out sites deleted; detachGL; currentImageFile_ comment.
- `src/effects/EffectChain.h` -- two comments (a single consumer remains).
- `src/test/TestServer.h/.cpp` -- set_output_tap, output_probe, destroyOutputProbe, state.outputs.
- `src/api/ApiServer.cpp` -- state.outputs (additive).
- `CMakeLists.txt` -- 5 sources; IOSurface for APPLE.
- `tests/CMakeLists.txt`, `tests/test_surface_pool.cpp`, `tests/test_shared_frame_gl.cpp`, `tests/test_output_law.cpp` (new).
- `.harmony/probe-outputs.sh/.py/.json` (new, `git add -f`).
- docs listed in section 9; `.harmony/notebook.md` (one entry).

### TESTS
- New ctests: test_surface_pool 5 cases PASS; test_shared_frame_gl 8 cases PASS; test_output_law 10 cases PASS. Total serial 690/690 (base 667 + 23).
- Live: probe-outputs 10/0 GREEN; 14 existing probes all GREEN; Tier-1 no new failure.

### SLIM CHECK
- Cut: the plan's `SharedFrameSet::publishCount()` (the plan listed it "for /api/state", but state.outputs uses `frame_serial`, the same count), so it was not added. `OutputWindow::target()` was deferred to C2 (no C1 caller).
- Kept on purpose: `SharedFrameSet::frontWidth/Height` (the only thread-safe source of canvas_w/h for /api/state); `onCloseRequested` (the plan's never-hide-and-forget close path).
- Duplication accepted: the 9-line state.outputs block is in both servers, matching the house pattern ("Same fields as TestServer").

### ISSUES
1. **Fence split in one .cpp more than planned.** Plan 8.1 has one header and one .cpp. The header is kept, and it is GL-free. The IOSurface half is implemented in `src/output/SurfacePool.cpp` so that `test_surface_pool` links with NO GL (plan 10.1). That is one extra new file inside `src/output/`.
2. **Publish on a size change deletes the live pending fence.** The plan's pseudo-code only "drops" it, which would leak one GLsync per resolution change. It IS deleted there, because it belongs to the current live context. `releaseGL()` (context closing) still only drops it, as ruled (0.5). After a context loss, the rebind of the same generation continues the rotation after the displayed slot instead of restarting at 0, which keeps the 2-frame grace. Both points are stated in the code.
3. **`MainComponent::currentImageFile_` is now write-only** (drift D11). It was left in place and its comment updated. Deleting ~20 assignment lines is Harmony's call: other lanes edit MainComponent concurrently.
4. **Stale comment outside the fence**: `src/render/EmbeddedShaders.h:3` still says "shared between Renderer and OutputRenderer". It is not in C1's fence, so it was not edited.
5. **probe-canvas.py `wait_no_compiler` never detects clang++** (the same `pgrep -x clang++` invalid-regex bug fixed in probe-outputs). It is recorded, not edited (out of the fence). Its c_perf numbers above ran with load avg 4.93 and no plain clang.
6. **Deprecation warnings**: `CGLErrorString` (SharedFrameSet.cpp) and `CGLChoosePixelFormat/CGLCreateContext/CGLDestroyContext` (TestServer.cpp). These are the expected class (CLAUDE.md Common Build Issues); test_shared_frame_gl defines GL_SILENCE_DEPRECATION.

### SKILL_PROPOSALS
- `offscreen-window-law-gate`: when a feature's behaviour is a WINDOW that no automated gate may open, split it into (a) a source-reading law ctest with mutated-copy teeth (compile once with a scratch `AUDIODNA_SRC_DIR`) and (b) an offscreen twin of the presenter driven through a private CGL context (ctest with N contexts; in-app via a test-mode route). Trigger: any screen-safety-law surface (output windows, overlays, fullscreen). Existing skills cover neither the static law nor the private-context probe.

### RISKS
- R-a (medium): the window behaviour (never key under a click, focus after show, frozen frame on preview hide, pacing, tearing at swap 0, menu-bar strip) is INFERRED from JUCE/AppKit source, not measured. These are Boris checks 2 and 5-8. Fallbacks are in plan R1/R11 (outputVsync key; the focusable variant).
- R-b (low): reopening an output shows the previous session's last frame for 1-2 frames before the first new frame is offered (+1 frame latency). A stale flash of at most ~33 ms at 60 Hz, only at open.
- R-c (low): the surfaces (4 x canvas, 32 MiB at 1080p) stay allocated after the first output use until the Renderer dies. This is by design (plan 8.6: "once").
- R-d (low): `output_probe`'s response `serial`/`slot` is read after presenting, so it can be one frame newer than the frame presented. `gen` and canvas_w/h are exact. It is a stats field only.
- R-e (merge): Pitfall number 40 may collide with a concurrent lane. Renumber at merge (notebook rule).

### METRICS
- Self-check: build-lane full build exit 0 at each commit; ctest serial 667 -> 680 -> 690 -> 690; strings checks 0; CLAUDE.md 24,992 B.
- Tool calls: ~140. Files read: ~40.

### KNOWLEDGE CONTEXT
- Tools used: grep + direct source reads (JUCE vendored tree, SDK headers). Impact authority: grep (NOT authoritative). Conservative posture: every OutputWindow/OutputRenderer reference was enumerated in src/ before deletion (`grep -rn "OutputRenderer\|OutputWindow" src`). No test links Renderer.cpp/MainComponent.cpp (`tests/CMakeLists.txt:43-44, :169-170`).
- Risk level: ELEVATED (cross-cutting: Renderer + MainComponent + TestServer + ApiServer). Mitigated by the full existing battery.

### PACKET QUALITY
- Clarity: CLEAR (a plan with exact sections, fence and gate).
- Missing context: `AUDIODNA_BUILD_TEST_SERVER` defaults OFF, and the STEP-0 configure line omitted it (found in the notebook; reconfigured ON). IOSurface was linked only under Syphon. The plan's line numbers predate the F2 reorder (the tap position).
- Unused context: plan sections 5 (OutputTargets), 8.4 (OutputManager), 9 (hot-plug) -- C2/C3.
- Self-assembly: LEGACY (no DEPARTMENT field).
- Self-brief files: `CLAUDE.md` (useful: pitfalls, byte cap via notebook), `.harmony/notebook.md` (very useful: TEST_SERVER flag, tests/visual pyc, r1_counts/r5_burst flakes, pitfall renumbering), plan5-final.md (the spec), probe-canvas.sh/.py (the clone source).

### STATUS
DONE_WITH_CONCERNS. Every C1 gate item that can be run without a window is done and green: RED first, ctest 690/690, probe-outputs GREEN, all 14 existing probes GREEN, Tier-1 with no new failure, o_tap_cost numbers, evidence PNGs, and no window ever opened. The concerns: (1) the window-level behaviour (never key under a click, frozen frame, pacing, menu-bar strip) is INFERRED and needs Boris checks 1-2 and 5-9; (2) the CLAUDE.md cap forced two wording items of plan section 17 into docs/claude (section 9); (3) ISSUES 1-3 (the extra SurfacePool.cpp, fence delete on a size change, write-only currentImageFile_) are small deviations for the Reviewer to rule on.

### NEXT ACTION
Harmony: behavioral gate on build-lane (probe-outputs + existing battery), independent review, then merge. Boris runs checklist items 1-2 and 5-9 at the rig. C2 (N windows, ticked menu, TopBar button, keys, level-probe label update) builds on `OutputWindow(SharedFrameSet&)` + `setLiveOutputCount(n)`.

---

## Fix round (outputs-c1-fix, round 1)

Two executors ran this round. Section 1a was written and committed (`5e1b7a1`) by a concurrent executor: no code, a note only. Section 1b below follows Harmony's ruling (probe fixed like with like, the Release never-key check added) and SUPERSEDES 1a wherever the two differ.

### Fix round 1a (outputs-c1-fix, critic round 1: `critic-outputs-c1-r1.md`; stamped 2026-09-27 11:18:22 from `date`)

STATUS: DONE. There is NO CODE CHANGE this round. The one MUST is a false positive, and the pixel evidence below refutes it. The one SHOULD asked only for a note, and the note is below. No gate is affected (no source, test, probe or build file changed), so no ctest, probe or live run was repeated. No app was launched and the live lock was never taken. `output_window_opened` = false.

**MUST (720p "border", `f720_render_frame.png`): REFUTED. The grey edge is authored content of fixture image B, not a capture margin.**
- The critic compared the 720p frame, which shows clip **B**, with 1080p frames that show clip **A** (`f0`, `f0b`, `soak_00`, `soak_59`). The one 1080p capture of clip B, `fB_render_frame.png` (and `probe_after_B.png`), has the SAME (15,15,15) edge. The critic's list said that `fB` "has (0,0,0) reaching every edge pixel". The pixels say it does not.
- Clip B's media is `media/P16_02_Screen_Split_2x2.png` (`.harmony/probe-outputs.json` `imageB`). That file is tracked since `108c8d0`, is byte-identical to the main checkout (sha256 `8b82185a...`), and is a 756x878 Screen-Split 2x2 still. It has a (15,15,15) outer margin and gutter in the file itself.
- Decoded pixels (`outputs-c1-evidence/fixround-720p-border-check.py`, output `fixround-720p-border-check.txt`, verbatim):
  - `fixture imageB P16_02_Screen_Split_2x2.png 756x878: corner (15, 15, 15), frac==(15,15,15) 0.132; margin left 13px (1.72% of 756), top 15px (1.71% of 878)`
  - `fB_render_frame.png 1920x1080: corner (15, 15, 15), frac==(15,15,15) 0.129, margin left 32px (1.67% of 1920), top 18px (1.67% of 1080), d(capture, imageB stretched to 1920x1080) 0.009`
  - `f720_render_frame.png 1280x720: corner (15, 15, 15), frac==(15,15,15) 0.128, margin left 21px (1.64% of 1280), top 12px (1.67% of 720), d(capture, imageB stretched to 1280x720) 0.029`
  - `probe_1920x1080_of_720p.png 1920x1080: corner (15, 15, 15), frac==(15,15,15) 0.123, margin left 31px (1.61% of 1920), top 17px (1.57% of 1080), d(capture, imageB stretched to 1920x1080) 0.066`
  - control: `f0_render_frame.png 1920x1080: corner (0, 0, 0), frac==(15,15,15) 0.000, margin left 0px ..., d(capture, imageB stretched to 1920x1080) 29.431`
- Reading: the margin is ~1.7% of each dimension in the source file, at 1080p and at 720p alike. Both captures equal image B stretched edge-to-edge onto the canvas (d 0.009 / 0.029 on 0-255; clip fit = the default stretch). A capture sized from a Component, or inset by one, would move the content inward and give a large d, as the A-vs-B control does (29.4). So at 720p the canvas IS the composition, full-bleed (Pitfall 37 holds). The output probe reproduces it (d 0.019 in the GREEN run, 0.066 against the fixture after two resamples).
- Also out of this lane: `render_frame` (`Renderer::processPendingCapture`, which reads `canvasFBO_` at canvasW_ x canvasH_) is pre-existing code that C1 did not touch. The lane's own path (output_probe) is shown equal to it.
- Not added: an image-content oracle row in probe-outputs (d(f720, imageB stretched) <= tol). It would gate `render_frame`, which is pre-existing and outside C1. It would also cost a RED/GREEN live pair for a claim the offline decode above already settles. It is Harmony's call if wanted.

**SHOULD (the `jassert` in the OutputWindow ctor is a no-op in Release): NOTED, no code change (as the finding asks).** The never-key guarantee in Release rests entirely on the `getDesktopWindowStyleFlags()` override (`src/ui/OutputWindow.cpp:69-74`). The `jassert` at `:49-50` is a Debug-only tripwire. If the timing of peer creation ever changed, it would give no Release safety net. The override is what `test_output_law` holds. Its teeth include "the flag kept only in the jassert -> 1 failed" (`TEETH-test_output_law-guards.txt`), so a refactor that left only the jassert would go RED in ctest.

Fix-round state: commits = this report and the two evidence files only. ctest unchanged at `100% tests passed, 0 tests failed out of 690` (no source changed since `e14027c`). Shots unchanged (no UI change). No `.venv` link present. Lock not taken. No Audio-DNA launched.

### Fix round 1b (outputs-c1-fix, following Harmony's ruling; stamped 2026-09-27 11:24 from `date`)

STATUS: DONE
RESULT: The critic's 720p MUST was a false positive. Nothing in the app changed for it: the probe row now compares like with like. It triggers A before the resize and asserts that the 720p frame is one full-bleed A (d 0.013). The GL SHOULD is fixed in Release: the OutputWindow constructor checks the peer's never-key flag in every build type, and test_output_law has a new row that holds that check. The conformance NIT 2 note is in the notebook. NIT 1 (`EmbeddedShaders.h:3`) is outside the fence and is left alone.
FACTS: commits `79b4410` (OutputWindow.cpp + test_output_law.cpp), `67bd544` (probe-outputs.py/.json), `52dd980` (notebook + pitfalls.md), plus this report commit. Evidence is in `.harmony/.reports/s-rta-0927/outputs-c1-evidence/fixround-*.txt`. ctest is `100% tests passed, 0 tests failed out of 691`. probe-outputs: RED on the pre-change app `PY 1 PASS / 9 FAIL`, GREEN on the lane app `PY 10 PASS / 0 FAIL`, and the mutant probe `PY 9 PASS / 1 FAIL`.
METHOD: Each gate ran RED first. test_output_law ran against the e14027c OutputWindow.cpp before the code change. probe-outputs ran against the main checkout's pre-change app. Mutants ran on COPIES only, and the deliverables' sha256 was the same before and after. Full ctest ran serially. Each live run took the lock (`outputs-c1-fix`) for that run only. Every PNG was decoded.
CONFIDENCE+VERIFY: HIGH. Re-run `bash .harmony/probe-outputs.sh` with the lock held and OUTP_APP=build-lane. The o_probe_survives_resolution_change line must show `d(f_720, fixture A P16_01_baseline.png stretched to 1280x720)` of about 0.013.
UNKNOWNS/NOT-DONE: The repair branch (log + `addToDesktop()`) is never taken today because the peer already carries the flag. By law it cannot be exercised live, since that would mean creating the Output window. It is held in source by test_output_law, and the string check shows it is compiled into the Release binary.
NUANCE: 1a's claim that the "jassert-only refactor would go RED" was true only for dropping the flag from the override. A ctor with only the jassert was green in round 1. It is RED now (the new row).
HANDOFF-NEEDS: none

**F1. Critic MUST (720p grey border): FALSE. Fixed on the probe side only, per the ruling.**
- Cause: `o_probe_tracks_change` leaves col1 = B showing. Fixture B, `media/P16_02_Screen_Split_2x2.png`, is itself a 2x2 grid on a (15,15,15) frame. So the round-1 f720 was B, and the critic compared it with f0, which is A. Harmony's live diagnosis on main is in `/Users/boriskarpman/projects/RealTimeAudio/.harmony/.reports/s-rta-0927/canvas720-diag.md`.
- Change (`67bd544`): the row runs `trig(0)` and waits `settleAfterTrigger` (1 s) before `set_composition_params 1280x720`. It then asserts `d(f_720, fixture A stretched to 1280x720 BILINEAR) <= contentTol (6)`, and the output line names the reference. The existing checks are unchanged: size, canvas, gen, and d(probe, f_720 upscaled) <= 6. No tolerance was changed. The JSON `_doc`, `resolution._why` and the docstring now say that fixture B is a 2x2 grid and that a B frame should only be compared with a B reference.
- RED on the pre-change app (main checkout build, probe at this tree), verbatim: `FAIL  o_probe_survives_resolution_change: output_probe probe_1920x1080_of_720p: HTTP 404 {'raw': ''}`, then `PY 1 PASS / 9 FAIL`, `PROBE-OUTPUTS RED` (`fixround-RED-base-probe-outputs.txt`).
- GREEN on the lane app, verbatim: `PASS  o_probe_survives_resolution_change: render_frame 1280x720 of col0 = A, d(f_720, fixture A P16_01_baseline.png stretched to 1280x720) 0.013 <= 6.0; probe canvas 1280x720, gen 1 -> 2, d(probe, f_720 upscaled) 0.013 <= 6.0`, then `PY 10 PASS / 0 FAIL`, `PROBE-OUTPUTS GREEN` (`fixround-GREEN-lane-probe-outputs.txt`).
- Teeth: a mutant COPY of the probe with the `trig(0)` line removed, which is round 1's capture of B, run against the lane app. Verbatim: `FAIL  o_probe_survives_resolution_change: render_frame 1280x720 of col0 = A, d(f_720, fixture A P16_01_baseline.png stretched to 1280x720) 29.425 <= 6.0; ...`, then `PY 9 PASS / 1 FAIL`. The deliverable probe's sha256 (`d53ace27…`) was identical before and after (`fixround-TEETH-probe-no-trig0-mutant.txt`). The label "of col0 = A" states what the row intends. In the mutant, B is actually showing, and that is exactly what the FAIL catches.
- Decoded pixels (GREEN run): `f720_render_frame.png` 1280x720 has corner (0,0,0), 0.000 of its pixels at (15,15,15), and an edge-ring mean of 0.00. `probe_1920x1080_of_720p.png` gives the same values. As a control, `fB_render_frame.png` still has 0.129 of its pixels at (15,15,15), which is B's own frame.

**F2. GL SHOULD (the jassert is a Release no-op): FIXED, so the guarantee is checked in every build type (`79b4410`).**
- `src/ui/OutputWindow.cpp`: after `setDropShadowEnabled(false)`, the ctor reads `getPeer()->getStyleFlags()` in every build type. If the peer is missing, or lacks `windowIgnoresKeyPresses`, the ctor logs `[OutputWindow] the peer lacks windowIgnoresKeyPresses -- re-adding the window to the desktop` to stderr and calls `TopLevelWindow::addToDesktop()`, which rebuilds the peer with the override's flags. This is the same re-add that JUCE's `setDropShadowEnabled` already does. The window is not visible at that point, so nothing is shown. The jassert stays after the repair as the Debug tripwire. The string is in the Release binary (`strings … | grep -c "peer lacks windowIgnoresKeyPresses"` = 1).
- `tests/test_output_law.cpp` new row, "the constructor checks the peer's windowIgnoresKeyPresses in every build type": it strips every `jassert(...)` from the ctor body, then requires `getStyleFlags()`, `windowIgnoresKeyPresses` and `addToDesktop(` in what is left.
  - RED on the e14027c OutputWindow.cpp, verbatim: `test cases: 1 | 1 failed` / `assertions: 7 | 4 passed | 3 failed`; whole binary `test cases: 11 | 10 passed | 1 failed`.
  - GREEN: `All tests passed (7 assertions in 1 test case)`; whole binary `All tests passed (39 assertions in 11 test cases)`.
  - Teeth on COPIES: control `All tests passed`; "runtime check removed (jassert only)" `7 | 4 passed | 3 failed`; "check that only logs (no repair)" `7 | 6 passed | 1 failed`; "check that repairs but never reads the flag" `7 | 5 passed | 2 failed`. OutputWindow.{h,cpp} sha256 was identical before and after (`fixround-{RED,GREEN-TEETH}-test_output_law-releasecheck.txt`).
- `docs/claude/pitfalls.md` Pitfall 40 now names the Release check (`52dd980`).

**F3. Conformance NITs.**
- NIT 1 was left alone because it is outside the C1 fence. `src/render/EmbeddedShaders.h:3` still says "shared between Renderer and OutputRenderer", but OutputRenderer no longer exists. The other two mentions, `Renderer.cpp:126` and `EffectChain.h:17`, are accurate history that says the class went away in outputs-c1. Only `EmbeddedShaders.h:3` is stale, and it needs a one-line comment fix from whoever next owns that file.
- NIT 2 is done (`52dd980`). The lane's notebook entry now says inline: if a lane merged first also added a Pitfall 40, renumber this one on merge, in CLAUDE.md, pitfalls.md, integration.md and rendering.md.

**F4. Gates re-run (and why only these).**
- ctest, full and serial, at `79b4410`: `100% tests passed, 0 tests failed out of 691`. That is 690 plus the new row. Later commits change only the probe and docs (`fixround-ctest-tail.txt`).
- probe-outputs RED, GREEN and mutant runs, as above. `o_no_window_opened` passed in every run (22 / 75 / 70 Quartz samples, never more than 1 layer-0 window, no Output-named window). After each quit there were 0 Audio-DNA windows. No run saw foreign REST traffic.
- Not re-run: the existing probe battery and the tests/visual Tier-1 set. The only app change is in the OutputWindow constructor, which runs only when the Output window is created, and no probe or visual test ever creates it (screen-safety law). Render, capture and canvas code are unchanged since the round-1 battery.
- No perf numbers this round.
- Shots: every PNG in `outputs-c1-evidence/png/` now comes from the fix-round GREEN run (`outputs.B6ArlG`). Only three bytes-differ from round 1: `f720_render_frame.png` and `probe_1920x1080_of_720p.png` (now A, full-bleed), and `main-window.png` (a fresh capture). The other eight are byte-identical to round 1. main-window shows clip cA active, the Preview showing A, and the TopBar Output set to "Off".

**F5. Rig.**
- The lock was taken as `outputs-c1-fix` for three runs: 11:22:04-11:22:17, 11:22:22-11:22:48 and 11:22:52-11:23:17. It was released after each one.
- A first background attempt was killed while it was still waiting on routines-timing's lock. It never acquired the lock and never launched anything.
- Launches used only `open -g … --args --test-mode` (inside probe-outputs.sh), and every quit went through osascript.
- There was no Output window, no full-screen capture, no lldb and no synthetic input. No temporary hook was used.
- The `.venv` link was not needed: probe-outputs.sh falls back to the main checkout's `.venv`. No `.venv` exists in the worktree.
- build-lane is kept.

INBOX-RECHECK: 1 addendum folded (Harmony's ruling: the MUST is false, fix on the probe side, take the SHOULD, list NIT 1).
