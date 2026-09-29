# Reviewer Verdict — s-rta-0928b video, lens gl, round 1
STATUS: DONE
VERDICT: PASS_WITH_NITS
REVIEWED: lane/video, base 328301d..90cdf55, worktree .claude/worktrees/rta0928b-video (git diff/show only, no working-tree edits)

## Scope
Read every changed file in the diff (23 files, +2228/-255): src/media/VideoRing.h (new, pure), VideoPlayer.h/.cpp,
VideoStats.h (new), src/render/Renderer.cpp/.h, CompositorEngine.cpp/.h, ApiServer.cpp, TestServer.cpp,
tests/test_video_ring.cpp + CMakeLists.txt registration, .harmony/probe-video.{sh,py,json}, and the doc set
(CLAUDE.md, pitfalls.md, rendering.md, performance-controls.md, effects.md, architecture.md, testing-eyes.md).

## FOCUS checks (plan-video.md + HARMONY ADOPTION V1-V7)
- Ring protocol (VideoRing.h): every CAS states its memory order explicitly (V3) — acq_rel on success / acquire on
  failure, verified by reading `cas()`. Producer publishes fields then `state.store(Ready, release)`; consumer loads
  `state` with `acquire` before touching fields — correct release/acquire pairing, single writer / single reader per
  slot, no slot ever has two owners (confirmed by code reading, the stress ctest, and my own execution below).
- No allocation / FFmpeg call / lock on the GL thread per frame: `advanceFrame`/`advanceClock`/`uploadToTexture`/
  `releaseGL` touch only atomics, the ring, and GL calls (glGenTextures only once via `textureCreated_` gate,
  glTexSubImage2D per frame). `gl_video_decode_calls` is never incremented anywhere in the new code (grep confirmed)
  — it stays 0 by construction, which is the intended witness, not dead code (probe-video.py asserts the delta == 0
  strictly, with a `video_threads == 1` companion check so a vacuous PASS is caught — verified in probe source).
- everShown_ (V1): `VideoRing::ShownState` — `onUpload` sets `everShown=true`, `onReleaseGL()` clears only
  `lastUploadedSeq` (forces a re-upload) and leaves `everShown` alone; `neverShown()` reads `!shown_.everShown`, never
  `texture_ != 0`. Matches the plan exactly; ctest "V1" exercises judge() after a simulated release.
- Idle parking inside the ring-full wait (V2): `onDecoded`'s acquireWrite retry loop calls `VideoRing::idleStep` and
  `park()` before each `thread_.wait(20)`, so a full ring does not block a player whose deck left the screen. ctest
  "V2" + probe row w4b (both arms, ring-full AND mid-catch-up) assert `video_threads_awake == 0` with a `video_threads
  == 1` companion (not vacuous).
- Close/retire without a join on a hot thread: `close()` only signals + notifies + returns; `drainRetiredMedia` only
  destroys a player once `threadDone()` is true (thread already exited), pushing not-yet-exited players back onto the
  retire list for the next frame — confirmed by reading the diff. The only join (`stopThread(3000)`) is in
  `~VideoPlayer`, reached with an already-exited thread except at app shutdown, as the plan explicitly permits (R-9).
- `videoPlayerMutex_` lookup-only: every message-thread and GL-thread site (`getVideoPlayer`, `getVideoPlayerFile`,
  `closeMediaForClip`, `installVideoPlayer`, `syncMedia`) takes the lock only to find/insert/erase the map entry and
  releases it before touching the `VideoPlayer*`; `msg_video_lock_wait_max_ms` is timed at exactly those sites.
  (`openGLContextClosing`'s loop still holds the mutex across every player's `releaseGL()` — pre-existing behaviour
  from before this lane (F8), a rare context-loss path, not a new violation; noted as a NIT below.)
- Context loss: `releaseGL()` clears the texture and forces a re-upload but keeps `everShown`; a paused player also
  bumps `gen_` and notifies the thread so it re-decodes the current frame (a paused clip would otherwise never pick a
  new frame). No live row exists for this (screen-safety law blocks Output-window triggers) — correctly disclosed in
  the report rather than silently skipped.
- Crossfade two players / the NONREF predicate (V4/V5): `neverShown()` is unaffected by a mid-fade retrigger's seek
  (only `open()` resets `everShown`), so `incomingImagePending` stays false and the fade is never paused by a
  retrigger — matches the "keep playing" ruling (V4). `kSkipNonRefInCatchUp` is a named `static constexpr` (on by
  default, V5), applied only via `VideoRing::useSkipNonRef` while >10 frames from the target — matches R-13.

## Executed verification (not taken on recall)
1. Ran the lane's own pre-built ctest binary: `build-lane/tests/test_video_ring` → "All tests passed (105 assertions
   in 14 test cases)" — matches the lane report's claim exactly.
2. In a $TMPDIR copy (never the tree under review), reproduced two of the report's "teeth" claims by mutating a copy
   of VideoRing.h and rebuilding against the lane's already-built Catch2 static libs:
   - Removed `pick()`'s stale-generation free/skip → reproduced `test_video_ring.cpp:305: FAILED:
     CHECK( staleGenPicks.load() == 0 ) with expansion: 269 == 0` (report cites the same line, a different run's
     count) plus a companion `orderViolations` failure and two more test-case failures (stale-gen ctest #2, the
     "untouched by a second pick" ctest #5) — matches the report's claimed failure shape.
   - Swapped `decide()`'s `reseekBehindSec`/`reseekAheadSec` → reproduced failures at `test_video_ring.cpp:154` and
     `:157`, exactly the lines the report cites for this mutation.
   Both confirm the gate has real teeth (a wrong ring/decide implementation is caught), not a synthetic-only suite.
3. Confirmed `tests/CMakeLists.txt` registers `test_video_ring` correctly (mirrors `test_seq_vram`'s pattern) and
   that the file has exactly 14 `TEST_CASE` macros (grep), matching the "14 new ctests" claim.
4. Confirmed CLAUDE.md is 24,731 bytes (`wc -c`), under the 25,000 budget the plan requires.
5. Confirmed no stray env-var hooks / instrumentation survive in the tree (`grep AUDIODNA_VIDEO_` → no hits) and
   `git status --porcelain` shows only the expected untracked `build-lane/` build directory.
6. Confirmed every existing caller of `openVideoForClip` compiles against the unchanged signature (MainComponent.cpp
   x5); `installVideoPlayer` is a new but non-dead seam (called internally, documented as the async-open hook).

## Plan / scope-completeness cross-check
All 6 planned commits are present in the stated order (counters+probe, b1, VideoRing.h+ctests, decode thread +
Renderer wiring, w4b ring-full-first probe fix, docs). `/api/state` fields match the plan's exact names and comments
in BOTH servers (ApiServer.cpp, TestServer.cpp), inserted at the plan-specified offsets (after `master_level` /
`onset_pulse_frames`, clear of the seqvram lane's insertion points). `CompositorEngine`'s pending provider was
correctly generalised from `SequencePendingFn`/`setSequencePendingProvider` to `MediaPendingFn`/
`setMediaPendingProvider` and both call sites (Renderer's provider, `incomingImagePending`) updated together.
Docs (rendering.md "Video playback", pitfalls.md "NN", CLAUDE.md index, performance-controls.md B2 wording,
testing-eyes.md render_frame row, architecture.md tree, effects.md hot-reload note) all read as accurate summaries
of the code I read above — no overclaiming found (e.g. rendering.md correctly says HOLD "may capture" during
render_frame, matching the code's judge()-based Held/Late/Pending, not an unconditional claim).

## Findings
- [SHOULD] `openGLContextClosing` (Renderer.cpp, pre-existing, not introduced by this lane): holds `videoPlayerMutex_`
  across every live player's `releaseGL()` call in a loop, which is broader than the "lookup only" rule R-10 states
  for the hot path. This is a rare context-loss event (not per-frame), inherited from before this lane (F8), and out
  of this lane's stated scope — flagging as a pre-existing note for a future lane, not a defect in this diff.
- [NIT] Pitfall placeholder "NN" is left literally in both `pitfalls.md` and `CLAUDE.md`'s index, pending Harmony's
  number assignment (55, since 54 is already taken by the merged seqvram lane) — this matches the established
  pattern from the seqvram lane (assigned in a separate follow-up commit after merge), not an omission.
- [NIT] `found_not_fixed` #3 (a video whose first-frame decode never lands stays PENDING forever, blocking a
  crossfade onto it and the render_frame gate) is a disclosed, low-probability edge case consistent with the design
  (R-6: pending is reserved for "a broken first frame"); worth a DEBT_FILED tracking line for a future lane, not
  blocking here.
- Two design/measurement findings (w2 4x4K plateaus at 84-90 fps vs the plan's INFERRED ">=110" bar; w1's fps read
  is bimodal under the probe's poller) are correctly self-reported as open items for Harmony/Boris's call rather
  than silently re-thresholded. Per the plan's own HARMONY ADOPTION V7, GREEN is defined as "4.7 + w4b + w6b" —
  which does NOT include these two fps rows — so this is not a missed plan item or a defect that ships; it is a
  legitimate product-feel decision correctly escalated, not a code-quality issue.

## Verdict rationale
No MUST-level defect found: the ring protocol is memory-model-correct (verified by reading + live execution +
adversarial mutation), the real-time/threading rules for this lane's surfaces are honored, the probe suite is
non-toothless (verified with a strict-equality check plus a vacuous-pass guard), docs match code, and every plan
item + HARMONY ADOPTION ruling I checked is faithfully implemented. The open items are handoff/product decisions
already surfaced by the builder's own report, not something this diff is hiding. PASS_WITH_NITS.
