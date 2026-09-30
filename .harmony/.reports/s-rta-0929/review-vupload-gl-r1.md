# Reviewer Verdict — vupload-gl-r1
STATUS: DONE
VERDICT: PASS_WITH_NITS
LANE: vupload (s-rta-0929), lens gl, round 1
BASE: 3e15613  HEAD: 3a9e1663fd47b6d804ecf05c5f2c8b8b0a91c000
WORKTREE: /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0929-vupload

FILES REVIEWED (git diff 3e15613..3a9e166, all 31):
src/media/VideoPlayer.{h,cpp}, src/media/VideoRing.h, src/media/VideoStats.h,
src/media/VideoUploadBudget.h (NEW), src/render/Renderer.{h,cpp},
src/render/GLThreadQos.h (NEW), src/ui/OutputWindow.cpp, src/api/ApiServer.cpp,
src/test/TestServer.{h,cpp}, tests/CMakeLists.txt,
tests/test_video_player_gl.cpp (NEW), tests/test_video_ring.cpp,
tests/test_video_upload_budget.cpp (NEW), .harmony/probe-video.{json,py},
.harmony/probe-vupload.{json,py,sh}, .harmony/probe-vupload-ab.{py,sh},
docs/claude/{rendering,pitfalls,testing-eyes}.md, CLAUDE.md, APP-INVENTORY.md.

## FOCUS (GL / IOSurface lifecycle) -- verified on disk, item by item

- Every GL object created/deleted on the GL thread: rectTex_/readFbo_/dstFbo_/texture_ created lazily inside
  uploadSlot/blitSlot (called only from uploadToTexture, GL thread) and deleted in releaseGL (message thread with
  the context current, same JUCE path as before -- unchanged invariant). VERIFIED.
- releaseGL zeroes every handle (VU3): rectTex_[i]/readFbo_[i]/fence_[i] zeroed in the loop, dstFbo_ zeroed,
  texture_ zeroed; matches SharedFrameSet::releaseGL's rule cited in the plan. VERIFIED (VideoPlayer.cpp releaseGL()).
- Fence poll timeout 0 incl. GL_WAIT_FAILED (VU2): pollFences() calls glClientWaitSync(f, 0, 0) (or the ctest
  override), treats GL_WAIT_FAILED like a signal (delete + release, ++video_fence_failed), matching SharedFrameSet's
  "never offer/never get stuck" rule (SharedFrameSet.cpp's publish() GL_WAIT_FAILED branch is the same idea, not
  byte-identical code -- acceptable, the plan only asks for the same rule). VERIFIED, exercised by
  test_video_player_gl case (3) with an injected fenceWaitOverride_.
- Purge / un-purge re-binds (VU1): unpurge() sets rebind_[i]=true (release) before the slot is next written;
  blitSlot() does rebind_[i].exchange(false, acq_rel) and re-runs CGLTexImageIOSurface2D as the DEFAULT path (not a
  fallback) when rebind is true. VERIFIED, exercised by test_video_player_gl case (5) (both Blit and Malloc arms)
  and the u4b live probe row.
- FBO completeness + malloc fallback (VU3): each per-slot read FBO and the draw FBO are checked once at creation
  (glCheckFramebufferStatus); incomplete -> fallBack() drops the player to Client path, counted
  (video_surface_fallbacks), GL bindings restored via the saved-binding `restore()` lambda before returning. VERIFIED.
- Held slot never released or purged while shown: VideoRing::Retire::held is excluded from contextLost()'s output
  and from purgeFreeSlots() (which only touches slots whose ring header state == Free; Reading != Free).
  dropReady() (idle trim) only demotes Ready slots, never Reading. VERIFIED by test_video_ring's Retire (c)/(e)
  cases and test_video_player_gl case (5).
- Context-loss path: releaseGL drops GL objects, keeps retire_.held, and the FIRST uploadToTexture after a release
  re-uploads that slot's bytes/surface (P4a); RED on the pre-lane VideoPlayer is recorded in
  test_video_player_gl case (2) (build against a copy of main's VideoPlayer, per the plan's method) and reproduced
  live by u4a (a JUCE detach/attach, the same openGLContextClosing -> newOpenGLContextCreated path a real minimise
  takes). VERIFIED. Concern (disclosed, not hidden): a GPU-driven context loss (vs. this test-only detach cycle)
  is NOT produced on this rig -- correctly labelled INFERRED by the lane report and by adoption VU6, not claimed
  as tested.
- Render thread never waits: peek()/pick() never CAS-loop or block; pollFences uses timeout 0 exclusively (no
  glClientWaitSync with a nonzero timeout anywhere in the diff). VERIFIED by inspection; no waiting primitive added.
- QoS in every context (VU11): raiseRenderThreadQos() (GLThreadQos.h) is called from BOTH
  Renderer::newOpenGLContextCreated and OutputWindow::Presenter::newOpenGLContextCreated (the same shared JUCE
  render thread) -- VU11's requirement satisfied, not just the preview's context. VERIFIED.
- Decode thread's IOSurfaceLock use: convertInto() locks/unlocks the slot's surface around sws_scale (unchanged
  shape, extended to the IOSurface case); unpurge()/purgeFreeSlots() run on the decode thread only, guarded by the
  ring's Free-state ownership rule (only the writer leaves Free), so no race with the GL thread's reader-owned
  Ready/Reading transitions. VERIFIED by inspection and the VU10 TSan-flavoured stress test in test_video_ring.cpp
  (a real std::thread writer racing peek()/pick() 100,000 frames, asserting the ordering invariant + accounting
  identity `admitted + charged == uploads`).

Additional correctness notes I verified by tracing rather than trusting the comment:
- `retire_.shown(p.slot, fence_[p.slot] != nullptr)` reads fence_[p.slot] AFTER uploadSlot() has (possibly) set it
  in the same call, so the fenced/unfenced flag passed to Retire is always current for that pick.
- `video_hold_no_texture` can only increment after the Pending/Failed switch cases (which `return 0` early) --
  i.e. only for a previously-shown player (Held/Late/New) that still has texture_ == 0 after the P4a restore
  attempt. This matches the documented "witness must stay 0" semantics exactly (not a loophole that also counts
  legitimately-pending players).
- VU3's correction to the plan's R-9 ("the blit source is the bound READ FBO, not a texture unit") is followed:
  blitSlot() does NOT re-bind the rectangle texture to a texture unit before each blit (only at rect-texture
  creation/rebind) -- correct, since glBlitFramebuffer reads via the READ_FRAMEBUFFER's attachment, not a bound
  texture unit. The plan's original R-9 text would have been a harmless no-op if implemented, but the adoption's
  correction was followed rather than the stale plan text -- good spec-fidelity behavior.
- Memory ordering of the purge/rebind handoff (decode thread `rebind_.store(release)` before `ring_.publish`
  (release) on the same header word; GL thread reads via `pick()`'s acquire load of that header, then reads
  rebind_) is a correct release-sequence pattern (transitivity via program order + the synchronizing pair on the
  ring's state atomic), not a new race.

## Deviations from the plan (disclosed by the builder; my assessment of each)

1. **reseekBehindSec / kWriterLookAhead (commit 90b2ca9, "beyond the plan").** Plan section 6 (MUST NOT CHANGE)
   says `pol.reseekBehindSec = (kSlots + 1) fd` "stays a valid upper bound" -- unchanged. The shipped code computes
   it from `kWriterLookAhead + 1` (`kWriterLookAhead = kSlots - 1`), i.e. `kSlots * fd`, smaller than the plan's
   stated invariant, to fix a real reverse-play regression the P4a held-slot change introduced (measured: g30
   uploads/s 11.2 -> 8.6 before the fix, restored to 11.3 vs 11.2 after). This is a genuine deviation from a
   locked "MUST NOT CHANGE" line, but it is: tested with its own dedicated live/AB gate (u7_reverse_pingpong +
   probe-vupload-ab.py's `--u7` ratio bar, >= 5 interleaved launches per arm, a real pass/fail, not INFO-only);
   honestly reflected in rendering.md and pitfalls.md (both explicitly say "measured from that look-ahead ...
   never from kSlots"); and disclosed by the builder as needing a ruling, with the plan's own suggested fallback
   (kSlots=4) tried and reported as NOT fixing it. I did not find a hidden/undisclosed defect here -- this is a
   design-fork that needs Harmony's explicit sign-off per section 6, not a silently-shipped rule violation.
   SHOULD: Harmony must explicitly rule (adopt VU7's fix as an override of section 6, or reject and require an
   alternative) before this lane merges -- do not let it merge on implicit approval.
2. **w7 (b) heavier tail (2/22 launches over the existing, un-rethresholded bar).** Existing threshold, not
   touched; root cause not settled (QoS-only arm alone showed a similar tail). Disclosed, not gated by a new
   assertion this lane added, so not a "test that cannot fail" concern for THIS lane's own gates -- but it's an
   open regression signal on an existing gate that Harmony should not wave through silently.
3. **fps4kBlitMin stays null (INFO).** Correctly refused per plan 4.9's explicit "never a knife-edge" rule
   (97 < max(A)+8=98.36); the measured numbers are printed in probe-video.json, not hidden. Not a defect.

## Tests -- real code, not synthetic-only

test_video_player_gl.cpp compiles the REAL VideoPlayer.cpp into a private CGL 4.1 core context (no window), drives
real GL calls (glBlitFramebuffer, glFenceSync, glClientWaitSync, CGLTexImageIOSurface2D) and a real per-fixture
FFmpeg decode as the oracle -- this is exactly the kind of live-GL verification the FOCUS calls for, not a fixture
that could be built to match a wrong implementation: case (2) is explicitly RED on the pre-lane VideoPlayer (the
plan's own negative-fixture requirement), and case (3)/(5) inject real GL_WAIT_FAILED / purge-cycle conditions.
test_video_ring.cpp's VU10 case is a genuine two-thread stress test (100k frames), not a mocked race.

## Other checks from the packet

- CLAUDE.md: 24,522 bytes (`wc -c`), under the 25,000 cap; the vupload pitfall-index line is present, and rule 5's
  note / index 47/54/28 exact trims (mentioned in the plan) are already reflected in the working CLAUDE.md.
- No stray artifacts found in the diff (no .venv symlink, no leftover instrumentation): the only new env hook
  (`ADNA_VIDEO_FORCE_FALLBACK`) is gated behind `#if AUDIODNA_TEST_SERVER` and documented as TEST-ONLY in three
  places (VideoPlayer.cpp, rendering.md, probe docstring) -- consistent with the codebase's existing test-lever
  convention (e.g. the TestServer-only gl_context_cycle route, VU12's Q1 ruling).
- Real-time rules: no allocation/lock/syscall added to the audio callback (untouched); no new mutex (the video
  path's std::atomic<bool>/GLsync/void* fields and the existing videoPlayerMutex_ lookup-only lock are unchanged
  in kind); render thread never blocks (confirmed above).
- Docs (rendering.md, pitfalls.md "NN (vupload)", testing-eyes.md, APP-INVENTORY.md) match the shipped code's
  actual behavior on every claim I checked (GL object counts, kTrimIdleMs, kWriterLookAhead wording, the
  video_* field list, the malloc/client fallback env var) -- no overclaiming found.

## Verdict

No MUST-level defect found in the GL/IOSurface lifecycle: object lifetime, fence handling, purge/un-purge,
held-slot protection, context-loss recovery and QoS wiring all match their claims and are exercised by real-GL
tests plus live probes, with negative fixtures where the plan calls for them. The one real deviation (reseekBehindSec)
is disclosed, tested and requires an explicit Harmony ruling rather than being a silently-shipped rule break --
downgraded to SHOULD per the "FAIL only for a MUST... missed plan item" instruction, since it is neither missed
nor hidden.

SUMMARY: 31 files reviewed, 0 blocking issues, 2 SHOULD (ruling required on the reseekBehindSec deviation before
merge; w7(b) tail regression should not be waved through silently), 0 NIT.
METADATA: reviewer=reviewer-vupload-gl-r1, builder_packet=vupload, lens=gl, round=1, date=2026-09-29
