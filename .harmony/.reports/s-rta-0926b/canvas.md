# s-rta-0926b lane canvas (plan4 A / B1 / B2 / C) -- builder report

## BUILDER REPORT

STATUS: DONE_WITH_CONCERNS
RESULT: Four commits on `lane/canvas-0926b` (base `2d02681`): A `c9c27c8` the composition canvas drives the picture's shape; B1 `dd131f7` fades finish on decks that are not on screen; B2 `f1ec669` video / image-sequence clocks and autopilot keep running there; C `dec1935` F2 was real and is fixed -- a deck switch now fades from the outgoing deck instead of cutting. Report, shots and evidence are committed on top.
FACTS: `src/render/RenderGeometry.h` (canvas geometry, pure) + `tests/test_render_geometry.cpp`; `src/render/Renderer.cpp` (canvas block, presentCanvas, top-of-frame deck-switch detection); `src/render/CompositorEngine.cpp` (rescaleHistory); `src/render/DeckClock.h`, `src/render/LayerClock.h`, `src/model/AutopilotBank.h` + `tests/test_deck_clock.cpp`; `src/media/VideoPlayer.cpp` (advanceClock); live probes `.harmony/probe-canvas.sh` (15 PASS / 0 FAIL final) and `.harmony/probe-deck-clock.sh` (10 PASS / 0 FAIL final); raw logs in `.harmony/.reports/s-rta-0926b/canvas-evidence/`; shots in `.harmony/.reports/s-rta-0926b/canvas-shots/`.
METHOD: I went RED-first on the pre-change app for every new probe row, then built each plan item as its own commit and ran ctest serially. Each commit got its new probe rows GREEN, the existing probes were re-run with no threshold changes, and the perf rows ran only when no compiler was running (load average printed). F2 was reproduced live before any fix. The runtime-history row was checked against a mutant build. tests/visual was run on both the base and the lane, and I compared the sets of failing tests.
CONFIDENCE+VERIFY: High for A/B1/B2/C behaviour (decoded pixels + REST fields, RED on base, GREEN on lane). Verify: `ctest --test-dir build-lane -j1` = 613/613; `AUDIODNA_LOCK_OWNER=<owner> CANVAS_APP=<lane app> bash .harmony/probe-canvas.sh` -> "PY 15 PASS / 0 FAIL", "PROBE-CANVAS GREEN"; `DCLOCK_APP=<lane app> bash .harmony/probe-deck-clock.sh` -> "PY 10 PASS / 0 FAIL"; the same two against `build/.../Audio-DNA.app` (pre-change) are RED (see TESTS).
UNKNOWNS/NOT-DONE: (1) tests/visual Tier-1 is NOT green. It is already red on the base app (167 failed / 164 passed / 6 skipped), and the lane (166 / 165 / 6) adds no new failing test (its failure set is a subset of the base's). (2) probe-render-state `r5_burst` "every attempt captured >= 8 frames" now fails, because 1080p captures are slower; I did not change its threshold. (3) probe-deck-path and probe-lane3 were not run: their launch methods are forbidden by the rig. (4) S7 (the Output window letterbox) is by-eye only and was never opened, per the rig. (5) The tests/visual comparison was done at commit A only.
NUANCE: This rig's display runs at about 120 Hz. The plan's runtime-history fixture (Freeze 0.95, read 0.4 s later) had no teeth here: a mutant with no rescale still passed with d = 4.31. I re-tuned it to Freeze 0.98 read after 0.1 s (mutant d = 81.76 FAIL, fix d = 0.29 PASS) and amended commit A. The probes launch in test mode (`open -g ... --args --test-mode`) because 8080 and inject_features need it.
HANDOFF-NEEDS: none

### SUMMARY
Every frame is now drawn once, offscreen, at the composition's size (default 1920x1080). The lower-left preview shows that picture letterboxed or pillarboxed, and never changes its shape. Recordings, snapshots, render_frame and Syphon get exactly that picture. Changing the resolution keeps Freeze/Echo/feedback histories. Decks that are not on screen keep time: fades finish, videos and image sequences keep playing (the clock advances without decoding), and autopilot keeps changing clips. Deck switches with a transition time now actually fade. The cut was real (F2), and it was proven live on the pre-change app before I fixed it.

**Say this to Boris together with "the preview is fixed" (plan4 caveat, unchanged by this lane):** the second-display Output window still shows only the old single image, not the composition (Q3). S7 only letterboxes that window to the composition's shape.

### FILES CHANGED
Commit A `c9c27c8` (plan4 section 2, S1-S9 + A-opt):
- `src/render/RenderGeometry.h` (new) -- the pure canvas geometry: resolveCanvas / fitCanvas / ringDownscale / presentTaps.
- `src/render/Renderer.h`, `src/render/Renderer.cpp`:
  - The canvas block runs before the first early return: a debounce (two identical reads) on outputWidth/Height, one packed-atomic test lock, ensureCanvasFBO, then a clear.
  - Every pass writes `canvasFBO_`. The legacy image is fitted inside it, never stretched.
  - `presentCanvas` is the box-filter present into the panel.
  - The recorder, Syphon and capture read the canvas.
  - `processPendingCapture()` waits for the frame rendered at the lock's exact size.
  - A-opt: GL_TIME_ELAPSED queries feed gpu_time_ms / peak_gpu_time_ms.
- `src/render/EmbeddedShaders.h` -- the `presentBox` shader.
- `src/render/CompositorEngine.{h,cpp}` -- `resize()` calls `rescaleHistory` (rescale-blits the temporal buffers and feedback, drops rings); rings are sized by `ringDownscale`.
- `src/render/FeedbackProcessor.{h,cpp}` -- `resizePreserving`.
- `src/model/Composition.h` -- guarded outputWidth/outputHeight load (0x0 becomes 1920x1080).
- `src/MainComponent.{h,cpp}` -- retired the per-deck resolutionSelector_/viewportLabel_; the recorder config uses the canvas size; the OutputWindow constructor gets `&composition_`.
- `src/ui/OutputWindow.{h,cpp}` -- S7 letterbox to the composition's shape (fitCanvas).
- `src/test/TestServer.cpp` -- set/get_composition_params take outputWidth+outputHeight (both, ints in [16,7680]x[16,4320]); gpu fields added.
- `src/api/ApiServer.cpp` -- gpu fields on 7070 /api/state.
- `tests/test_render_geometry.cpp` (new), `tests/CMakeLists.txt`.
- `.harmony/probe-canvas.{sh,py,json}` (new).
- Docs: `CLAUDE.md` (render-thread sentence, UI pattern, Pitfall 36 index), `docs/claude/pitfalls.md` (Pitfall 36), `docs/claude/rendering.md` (ring rule, rescale rule), `.harmony/APP-INVENTORY.md`.

Commit B1 `dd131f7` (plan4 T1/T2/T6/T7/T8):
- `src/render/LayerClock.h` (new). `CompositorEngine::advanceCrossfade` now forwards to it.
- `src/render/DeckClock.h` (new).
- `src/render/Renderer.cpp` -- the inactive-deck tick inside the deckActive fence, with a no-op media clock.
- `src/api/ApiServer.cpp` -- previousClipColumn / crossfadeProgress / persistent / playheadPosition.
- `tests/test_deck_clock.cpp` (new, cases a-e), `tests/CMakeLists.txt`.
- `.harmony/probe-deck-clock.{sh,py,json}` (new, B1 rows).
- `docs/claude/performance-controls.md`, `CLAUDE.md` rule 15.

Commit B2 `f1ec669` (T3/T4/T5):
- `src/media/VideoPlayer.{h,cpp}` -- advanceClock plus a shared advanceTransport; advanceFrame's behaviour is unchanged.
- `src/render/Renderer.{h,cpp}` -- syncMedia(decode) / tickMediaClock; `AutopilotBank autopilots_` (active deck chosen by index, inactive decks in the tick).
- `src/model/AutopilotBank.h` (new).
- `tests/test_deck_clock.cpp` -- case (f).
- Probe B2 rows.
- Docs: Pitfall 37, performance-controls.md, CLAUDE.md.

Commit C `dec1935` (F2):
- `src/render/Renderer.cpp`:
  - Deck-switch detection moved to the top of the frame, blitting the previous canvas frame into prevDeckFBO_.
  - The transition pass moved after master opacity.
  - The old detection block after the composite was removed.
- `.harmony/probe-canvas.{py,json}` -- row `f2_deck_transition`.
- `docs/claude/rendering.md` -- P25 note.

Report commit: this file, `canvas-shots/`, `canvas-evidence/`, and a `.harmony/notebook.md` entry.

### TESTS
ctest (serial): A 607/607 (600 base + 7 render_geometry), B1 612/612, B2 613/613, C 613/613 (`canvas-evidence/ctest-*.txt`).

RED by absence: `src/render/RenderGeometry.h`, `DeckClock.h` and `AutopilotBank.h` do not exist at `2d02681`.

Teeth, checked on mutated COPIES; the deliverables' sha256 did not change:
- fitCanvas with the branch flipped (`<=` to `>=`): "test cases: 7 | 5 passed | 2 failed, assertions: 29 | 23 passed | 6 failed".
  - The plan's suggested `<=` to `<` mutant is equivalent: exact-aspect inputs give the same rect on both branches, so it could not fail.
- DeckClock persistent rule flipped: "test cases: 5 | 4 passed | 1 failed" (test_deck_clock.cpp:83/:96).
- Case (f) includes its own teeth section: one shared Autopilot advances only the first deck.

Live probes. Raw summary lines are verbatim; full logs are in `canvas-evidence/`.
- Pre-change app, probe-canvas: "PY 8 PASS / 7 FAIL", "PROBE-CANVAS RED".
  - RED rows:
    - c_default_shape: PNG 756x878.
    - c_4k_shape: 756x878.
    - c_custom_4x3: 756x878.
    - c_runtime_change: set_composition_params returns HTTP 400; frames stay 756x878.
    - c_legacy_image_fit: 756x756.
    - f2_deck_transition: "p per trial [[1.0, 1.0], [0.25, 0.5], [1.0, 1.0]]" -- 2 of 3 switches were cuts. A race decides it: the outgoing copy was taken after the new deck was composited.
  - c_capture_deterministic PASS (guard).
  - The re-tuned runtime row on base: "PY 1 PASS / 2 FAIL" (756x878).
- Pre-change app, probe-deck-clock: "PY 2 PASS / 6 FAIL", "PROBE-DECK-CLOCK RED".
  - The REST fields are absent.
  - Fade back-frame: d(f, refB) = 23.14.
  - Video t1 - t0 = 0.42 s after 4 s away.
  - Image sequence still on frame A.
  - Autopilot column 0.
  - Guards: pending-trigger PASS; hitch 12.80 ms PASS; persistent-single-advance N/A (field absent).
- Lane A: probe-canvas "PY 14 PASS / 1 FAIL". The FAIL is f2, which is still a cut at A (the deck-transition code was only moved, not fixed); C fixes it.
  - 4K box-downsized vs 1080p: d = 0.02.
  - Legacy image: bars mean RGB 0.00, image R 255.0.
  - Runtime row (re-tuned): d 0.29 / 0.01. On the no-rescale mutant it FAILs with 81.76 / 81.76.
- Lane B1: probe-deck-clock "PY 7 PASS / 3 FAIL". The 3 FAILs are the B2 rows, as expected. All B1 rows PASS:
  - fade +0.26, complete by 4.42 s, back-frame d = 0.00.
  - persistent progress 0.50 at T/2.
  - pending trigger stays on column 0.
- Lane B2: probe-deck-clock "PY 10 PASS / 0 FAIL", "PROBE-DECK-CLOCK GREEN".
  - Video t1 - t0 = 4.47 s after 4 s away; playhead 0.552 vs 0.537.
  - Image sequence shows frame B.
  - Autopilot column 1.
  - Return hitch 12.59 ms.
- Final (C): probe-canvas "PY 15 PASS / 0 FAIL", "PROBE-CANVAS GREEN", with f2 "[[0.25, 0.5], [0.26, 0.5], [0.25, 0.5]]". probe-deck-clock "PY 10 PASS / 0 FAIL".

Existing probes, re-run at A and again at C, no threshold edits:

| probe | A | C |
|---|---|---|
| render-state | 30 / 1 | 30 / 1 |
| crossfade | 35 / 0 | 35 / 0 |
| effects-parity | 46 / 0 | 46 / 0 |
| step3 | 94 / 0 | 94 / 0 |
| mastersignal | 22 / 0 | 22 / 0 |
| routines | 74 / 0 | 74 / 0 |
| resync | 16 / 0 | 16 / 0 |
| onset-render | 13 / 0 | 13 / 0 |

- The one render-state FAIL, at A and C, is `r5_burst: every attempt captured >= 8 frames after the switch` (6-7 captured). The row's real assertion, "no blank frame", PASSes.
- B1 ran render-state 29/2: the second FAIL was r1_counts peak 50.82 ms vs 50. It was not reproduced at C (26.07 ms) and B1 cannot affect it (single-deck composition), so it was load noise.

tests/visual (commit A binary vs the pre-change app, same test-mode `open -g` launch):

| app | failed | passed | skipped |
|---|---|---|---|
| lane | 166 | 165 | 6 |
| pre-change | 167 | 164 | 6 |

- The lane's failed-test set is a subset of the base's (`comm` on the two FAILED lists): 0 lane-only failures, 1 fixed on the lane (test_no_discontinuities).
- On the base, 7 tests crash with "ValueError: operands could not be broadcast together with shapes (518,756,3) (756,756,3)": the base's captures come out in varying sizes. The lane has 0 such crashes (exactly 256x256 / 512x512 frames).
- Those aggregate tests now finish on the lane and list content findings: 41 effect params, 514 source params, 81 sources black at some times. I could not compare them item by item with the base, because they crash there.
- The fractal black frames (julia_set 0.00) are the same on the base (I checked live).
- Per plan 2.7, a threshold may only be fixed in a test when it flips from pass to fail. Nothing flipped, so no test was edited.

Perf, same composition (plan 2.8). Load average is printed in the logs (about 3.5-5).

| run | fps mean | frame_time_ms mean | gpu_time_ms mean (peak) |
|---|---|---|---|
| base, panel-sized render (1080p row) | 110.6 | 1.34 | n/a |
| A 1080p (GATE, PASS) | 114.4 | 1.85 | 2.26 (12.66) |
| B1 1080p | 115.2 | 1.76 | -- |
| B2 1080p | 111.2 | 1.70 | 2.27 |
| C 1080p | 111.4 | 2.16 | 2.26 |
| C 4K (REPORT) | 92.2 | 1.89 | 7.98 (9.50) |

- 4K holds more than 30 fps; about 92 on this 120 Hz rig.
- Capture cost: the base (panel-sized) takes 81 ms. The lane takes 1080p 149-178 ms, and 4K 492-508 ms, during which fps drops to about 70 for one second.

### SLIM CHECK
Nothing to cut. Every addition is named in the plan (S1-S9, A-opt, T1-T8, F2). The two temporary things are gone: the screenshot hook was removed (`git diff` on MainComponent.cpp is empty; `strings` on the binary counts 0 for AUDIODNA_DEBUG_COMPTAB) and the no-rescale mutant was restored (sha256 equals HEAD).

### ISSUES
1. **tests/visual Tier-1 does not PASS, on either binary.** The task asked for Tier-1 PASS. The suite is already red in test mode on the pre-change app (167 failed). The lane adds no failing test, fixes one, and removes 7 size-mismatch crashes. Harmony decides whether to accept "no new failures" as the gate.
2. **`r5_burst` capacity sub-check flips.** It needs 8 captures in 0.6 s, and a 1080p capture now takes about 150-180 ms (synchronous readback + per-pixel copy + PNG encode, plan 1F/F4). I did not touch its threshold. The fix is a PBO async readback (F4), which is its own item.
3. **The runtime-history fixture was re-tuned (plan said Freeze 0.95 / 0.4 s).** On this 120 Hz display it had no teeth: mutant d = 4.31 / 2.63, still PASS. I changed it to Freeze 0.98 / 0.1 s, proved it on the mutant (FAIL 81.76) and on the fix (PASS 0.29), and amended commit A. The row also moved its peak read to after the capture. Evidence: `canvas-evidence/TEETH-no-rescale-mutant-runtime-row.txt`, `GREEN-A-runtime-row-retuned.txt`, `RED-base-runtime-row-retuned.txt`.
4. **Launch form.** The new probes and tests/visual launch with `open -g --stdout f --stderr f <App> --args --test-mode`. The rig's allowed form does not list `--args`, but test mode is required by the task: 8080 set_composition_params, 7070 inject_features, and tests/visual. The launch is still background-only (`open -g`), never the Output window, with no synthetic input.
5. **Not run: probe-deck-path.sh and probe-lane3.sh.** The first starts the binary directly (`"$APP" --test-mode &`, line 66), the second uses plain `open "$APPBUNDLE"` (line 133), and both hard-code `$ROOT/build`. The rig forbids both launch forms.
6. **AutopilotBank deviates from the plan on purpose (safety).** The plan had the setters "store and fan out" across the vector from the message thread. That would race the GL thread growing the vector. Instead the setters only store (atomics), and `forIndex()` applies the stored config on the GL thread.
7. **C moved the P25 transition pass after master opacity.** This is required so the first transition frame is exactly the last frame that was shown. For Alpha blend (the default) the result is identical at constant master opacity. For Add / Multiply with master opacity below 1, the cross term now scales by master once rather than differently.
8. **`d_return_hitch` peak was 12.59 ms, under 16, so no catch-up budget was added (T3).** The fixture is a synthetic ramp video that decodes fast; real VJ footage with long GOPs may cost more.

### SKILL_PROPOSALS
- `per-commit-staging-with-markers`: build several commits' worth of probe rows in one working file, marked with `# >>> plan4 X >>>` blocks. A small script (`stage_probes.py` in the scratchpad) writes out each commit's exact version. This let RED-first runs cover every item before the first commit. It is reusable for any multi-commit lane whose probes must go RED-first on a base binary before any commit exists. Trigger: a lane with at least 2 commits sharing one new probe file.

### RISKS
- The capture/snapshot hitch grows with the canvas: about 0.15 s at 1080p and 0.5 s at 4K, on the GL thread. A mid-set snapshot at 4K is a visible stall (medium). The fix is F4 (PBO async).
- Deck-path clips that do not match the canvas shape are STRETCHED to it: square images become ellipses on 16:9 (visible in the shots). This is the plan's default (Q4); a per-clip fit/fill/stretch setting is follow-up work (low-medium).
- Stateful procedural sources (fluid_dynamics, reaction_diffusion, ...) reset on any canvas-size change, including each TestServer width/height capture. This is pre-existing (plan 2.9).
- The Output window shows only the legacy image (caveat, Q3); S7 was never opened by me (rig rule).

### METRICS
- Self-check:
  - builds of build-lane all exit 0; ctest serial 607 / 612 / 613 / 613.
  - live probes as listed; no existing threshold edited.
  - two mutation checks on copies, plus one in-place mutant build proven restored (sha256 `cdcb1761...` equals HEAD).
- Tool calls: about 230. Files read: about 45.

### KNOWLEDGE CONTEXT
- Tools used: grep only (no KNOWLEDGE_TOOLS block in the packet).
- Impact authority: grep, not authoritative. I stayed conservative: I read every caller of setLockedResolution, OutputWindow, advanceCrossfade and captureFrame.
- Risk level: ELEVATED (Renderer::renderOpenGL is the central render path).
- Dependencies discovered:
  - TestServer's lock race: before this lane, captures came from panel-sized frames.
  - A JSON `speed` key is loaded unguarded (`Clip.cpp:214`), so a video clip without it never moves.

### PACKET QUALITY
- Clarity: CLEAR. The plan is precise, with line numbers still valid at `2d02681` for every render file.
- Missing context:
  - The rig display is 120 Hz, which invalidated the plan's 60 fps teeth arithmetic.
  - tests/visual is already red on the base.
  - probe-deck-path and probe-lane3 cannot be launched under the rig rules.
  - 7070 inject_features exists only in test mode.
- Unused context: none.
- Self-assembly: LEGACY (no DEPARTMENT field).
- Self-brief files: `CLAUDE.md` (useful). `.harmony/notebook.md` was 148 KB, read by headings: the render-lane and uitoggle screenshot-hook entries were useful. The plan was the spec.

INBOX-RECHECK: none

### STATUS
DONE_WITH_CONCERNS. All four plan items are built, tested RED-to-GREEN, and committed one per item. The concerns are Issues 1, 2, 4 and 5 (a gate the task named that is red on the base too, one capacity row flipped by capture cost, the launch form, and two probes I could not run).

### NEXT ACTION
Harmony:
1. Rule on the tests/visual gate (Issue 1) and the r5_burst row (Issue 2).
2. Tell Boris the preview now follows the composition, and in the same breath that the Output window still shows the old image (Q3).
3. Ask Boris Q2 with the numbers: 4K holds about 92 fps here, 8.0 ms GPU per frame, and Split/Stutter cells are softer at 4K.
4. Ask Q4 about stretching.
5. Hand S7 to Boris for a by-eye check.
