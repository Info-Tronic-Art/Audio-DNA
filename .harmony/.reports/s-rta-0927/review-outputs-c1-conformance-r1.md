# Reviewer Verdict — outputs-c1-conformance-r1
STATUS: DONE
VERDICT: PASS_WITH_NITS
REVIEWED_COMMIT: e14027ceef295f02cf9747ae68f33771fbcf4277 (base 4b0c39a73068cd3208462d84a76b3df5fa8a154a)
LANE: lane/outputs-c1-0927, worktree /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0927-w1

## SCOPE CHECK (C1 only, no C2/C3 creep)
VERIFIED via `git diff --stat`: no OutputManager, OutputMenuModel, OutputTargets, AppSettings, MenuBarModel or
TopBar files touched. C1's fence list (plan5 11) is respected exactly: SharedFrameSet.{h,cpp}, SurfacePool.cpp,
OutputPresenter.{h,cpp}, OutputWindow.{h,cpp}, Renderer.{h,cpp}, MainComponent.{h,cpp} (ctor call, fan-out
deletions, dtor line, live-count calls only), TestServer.{h,cpp}, ApiServer.cpp (state field only), CMakeLists.txt
(2 new sources + IOSurface framework), tests/CMakeLists.txt, 3 new ctests, probe-outputs.*, docs.

## MUST-NOT-CHANGE LIST — all VERIFIED intact
- captureFrame/render_frame semantics: `publishToOutputs` reads `canvasFBO_` read-only and restores
  `GL_FRAMEBUFFER = canvasFBO_` at the end of `SharedFrameSet::publish` (SharedFrameSet.cpp); `processPendingCapture`
  untouched.
- Canvas block + presentCanvas order: tap call sites are additive, placed exactly where drift item D3 says (after
  the deck-transition block, which is now the canvas's last writer post-canvas-lane F2, and before `presentCanvas`);
  both plan4 early-return paths get the tap before `processPendingCapture`. Confirmed by reading
  `Renderer.cpp` lines around the transition block directly (not just the diff).
- Shutdown order: `MainComponent::~MainComponent` — servers stop (`testServer_->stop()`) → main renderer
  `previewPanel_.getRenderer().detach()` → `outputWindow_->detachGL()` → ... → `outputWindow_.reset()` at the end.
  Unchanged relative order; `detachGL()` replaces `getRenderer().detach()` 1:1. `TestServer::stop()` now also calls
  `destroyOutputProbe()`, before the renderer detach — correct per the drift table (D17).
- 7070 shapes additive-only: `ApiServer.cpp` diff adds one new `"outputs"` object to `handleState`; no existing key
  removed or renamed. Same additive shape for 8080 (`TestServer.cpp`).
- Syphon path: zero Syphon files in the diff (`git diff --stat` confirms no `Syphon*` touched).
- MenuBarModel ids: zero `MenuBarModel.*` touched.

## LEGACY REMOVAL — VERIFIED
`grep -rn "OutputRenderer" src/ tests/` → only 3 hits, all comments in files outside the C1 fence
(`EmbeddedShaders.h`, `Renderer.cpp`, `EffectChain.h`) referring to the retired class historically — no dead code,
no live symbol. The 9 image fan-out sites + 1 camera fan-out in `MainComponent.cpp` are all deleted (matches
drift D9's count, cross-checked against the diff hunks). `currentImageFile_` correctly left in place with an
updated, honest comment ("no reader since s-rta-0927 outputs-c1") — matches drift D11 exactly, not a phantom claim.

## TESTS DRIVE REAL CODE, RED FIRST — VERIFIED (not merely trusted from the report)
- Read `tests/test_output_law.cpp` in full: it's a real static-source gate (comments/literals stripped, regex-free
  token search + a bounds-before-visible parse), not a stub. Cross-checked its RED-on-base evidence file
  (`RED-base-tree-test_output_law.txt`) — 3 real failures (`toFront(true)`, `setWantsKeyboardFocus(true)`, missing
  `windowIgnoresKeyPresses`), matching the report's claim verbatim.
- `tests/test_shared_frame_gl.cpp`: 8 TEST_CASEs present exactly as claimed (roundtrip_byte_identical,
  nothing-published, letterbox_geometry, reader_tracks_new_frames, generation_change, reader_destroyed_mid_run,
  writer_context_loss_keeps_last_frame, slot_rotation), with a `SKIP` fallback when no CGL pixel format exists
  (honest, not a silent pass).
- Mutation teeth independently re-checked from the evidence files, not just re-stated: `TEETH-test_output_law-
  guards.txt` shows all 5 named violations caught (1 test failing each time) with sha256 of `OutputWindow.{h,cpp}`
  IDENTICAL before/after every mutation round — the tree was genuinely reverted, not left mutated.
  `TEETH-shared_frame_gl-rotation-mutant.txt` shows the same discipline for `SharedFrameSet.cpp` (rotation
  `% (kSlots-1)` → `slot_rotation` fails; sha256 restored).
- `ctest-*-tail.txt` progression (667 base → 680 C1a → 690 final, 100% pass every time) and the
  `tests-visual-tier1-{BASE,LANE}-outcomes.txt` diff is byte-identical (8 failed/5 passed both trees) — the
  report's "identical pre-existing failure set" claim is independently reproduced, not taken on faith.
- `GREEN-lane-probe-outputs.txt` matches the report's pasted block verbatim, including the `d 0.000` round-trip.
  Visually compared `probe_1920x1080.png` against `f0_render_frame.png` (Read tool, side by side) — identical.

## CODE QUALITY (SharedFrameSet.cpp / OutputPresenter.cpp / OutputWindow.cpp)
- Readable, well-commented, matches house style (plan5 section citations inline, consistent with other recent
  lanes' comment conventions in this codebase).
- One deliberate, tested deviation from the plan's literal pseudocode (8.1): on a context-loss rebind where the
  generation is UNCHANGED, `SharedFrameSet::publish` sets `write_ = front().slot` (continuing rotation past the
  currently-shown slot) instead of the plan's unconditional `write_ = -1` (which would restart at slot 0 and could
  immediately race the currently-displayed slot if front == slot 0). This is a correctness improvement over the
  plan text, not a regression, and it is exercised by `writer_context_loss_keeps_last_frame` (case 6). Documented
  in code with a clear comment. Not a MUST-NOT-CHANGE violation (kSlots, the packing format, and the drop-not-
  delete fence rule are all unchanged).
- `publish()` also adds defensive early-returns not in the plan's pseudocode (surface-creation failure, a
  GL_WAIT_FAILED branch distinct from GL_TIMEOUT_EXPIRED) — reasonable robustness, doesn't change the documented
  protocol's observable behavior.

## SLIM CHECK
No EXCESS_DEAD/EXCESS_VESTIGIAL/EXCESS_DUP/EXCESS_SPEC found. Every new public method on `SharedFrameSet`/
`SurfacePool`/`Renderer` (getSharedFrames, setLiveOutputCount, getLiveOutputCount, setOutputTapForced,
isOutputTapForced, frontWidth/frontHeight) has a live caller in this diff (MainComponent, ApiServer, TestServer).
harness-wiring: the two new ctests and the mac-only ifdef in CMakeLists.txt are correctly gated `if(APPLE)`,
mirroring the existing `syphon-check` precedent cited in the plan.

## DOCS PER SECTION 17 — spot-checked, not just presence
CLAUDE.md pitfall index line 40 added; `docs/claude/pitfalls.md` pitfall 40 body accurately describes the
mechanism (checked against the actual `getDesktopWindowStyleFlags()` override and `releaseGL()` — no overclaim);
`docs/claude/rendering.md` tap paragraph correctly states "after the canvas's last writer (the deck transition,
which runs after master opacity)" — matches the re-verified Renderer.cpp reading, not the plan's stale "after
master opacity" phrasing (this is the drift D3 correction being carried through into the doc honestly).
`docs/claude/testing-eyes.md` / `integration.md` / `architecture.md` / APP-INVENTORY.md updates all check out
against the actual routes/behavior read directly in TestServer.cpp/ApiServer.cpp.

## NITS (non-blocking)
1. `src/render/EmbeddedShaders.h:3` still says "shared between Renderer and OutputRenderer" — stale since
   OutputRenderer no longer exists. File is outside the C1 fence and the comment is harmless/pre-existing-style
   debt, not something this lane introduced logic-wise, but it is now factually wrong. Low priority; fix opportunistically.
2. Plan5 D15 flags a real risk (pitfall #40 could collide with a concurrent lane also claiming #40) but the notebook
   entry added by this lane doesn't restate the "renumber on merge" instruction inline — relies on the drift table
   in the (separate) builder report. Not a code defect; a process note for whoever merges.

## VERDICT RATIONALE
No MUST-severity findings. Plan conformance is unusually well documented and, on independent re-verification
(reading source directly rather than trusting the report's prose), every load-bearing claim held up: the RED-first
evidence is real, the mutation teeth are real (sha256-verified untouched-after), the screen-safety law is enforced
both statically (test_output_law) and live (probe-outputs.sh's Quartz window sampler), and all MUST-NOT-CHANGE
invariants are intact. Two cosmetic NITs only.
