# Attack: plan-vupload.md -- OpenGL / IOSurface / CGL seat

VERDICT: P1/P4a/P4b logic is clean and the render thread never blocks (fence poll timeout 0, matches
CLAUDE Sacred Rule 4). But P3's GL-object lifecycle has two under-specified gaps that will produce silent
black/garbage frames on the exact paths this plan targets (context loss, idle purge), plus one unhandled
fence-failure path -- none caught by the ctest plan as written.

## MUST

1. **Purge/unpurge of a Free slot races that slot's OWN already-live GL rect texture; plan defaults to the
   unsafe path.** R-11: `rectTex_[slot]` is created lazily on first upload and kept for the player's life,
   never deleted on Reading->Free (4.4). R-14 purges Free slots, but a slot that has been shown once already
   has a live `CGLTexImageIOSurface2D` binding from before. `IOSurfaceSetPurgeable(Empty)` then
   `NonVolatile` + a fresh CPU write can change physical pages under an already-bound GL texture; the GPU
   driver's cached page state for that texture object is not guaranteed refreshed. Section 8 risk 4 names
   this ("INFERRED the binding survives") but files the correct fix -- re-run `CGLTexImageIOSurface2D` on
   unpurge -- only as a FALLBACK gated on `test_video_player_gl (5)` passing on the builder's one dev Mac.
   That is backwards: the re-bind costs microseconds and removes a driver/GPU-dependent bug class that may
   pass on Apple Silicon unified memory and fail on a discrete-GPU Intel Mac. Fix: always re-bind on unpurge.

2. **`pollFences()` has no path for `GL_WAIT_FAILED`.** 4.4 only branches on ALREADY_SIGNALED /
   CONDITION_SATISFIED vs implicit TIMEOUT_EXPIRED. The codebase's own precedent, `SharedFrameSet.cpp:59`,
   explicitly handles WAIT_FAILED ("that copy's state is unknown -- never offer it; continue with a fresh
   one"). Here a WAIT_FAILED fence leaves the slot stuck Reading forever: the writer permanently loses a
   ring slot, shrinking kSlots=3 toward 1 and eventually starving `acquireWrite()` (VideoPlayer.cpp
   :634-648's ring-full wait). `test_video_player_gl (3)` only drives the signaled path via `glFinish()`.
   Fix: on WAIT_FAILED, delete the fence and release the slot same as signaled; add a ctest case that forces it.

3. **`releaseGL()`'s rect-texture/FBO handles must be explicitly zeroed, not just deleted.** 4.4 says
   releaseGL deletes rect textures, read FBOs, dst FBO, texture_, and explicitly zeros `fence_ = {}`, but
   never says the GLuint handle fields are reset. `uploadSlot()`'s lazy-create is gated on the field being 0.
   The codebase's own precedent for exactly this lazy-recreate-after-context-loss pattern,
   `SharedFrameSet::releaseGL` (SharedFrameSet.cpp:79-86), zeros `tex_[i]`/`fbo_[i]` in the same loop as the
   delete for exactly this reason: a stale non-zero name from the destroyed context makes the lazy-create
   check skip recreation, and later GL calls silently no-op on the invalid name -- black frame on every
   player, every context loss (not just the held slot). Spell this out in 4.4; the terse pseudocode omits it.

## SHOULD

4. **No `glCheckFramebufferStatus` on the per-slot read FBO.** Both `SharedFrameSet.cpp:113` and
   `OutputPresenter.cpp:64` check completeness after attaching a rect texture and fall back / log on
   failure; 4.4's `uploadSlot()` only checks the `CGLTexImageIOSurface2D` CGLError, then blits. An attach
   that returns `kCGLNoError` but yields an incomplete FBO blits garbage/black with no log and no fallback.

5. **R-9's "re-bind the rect texture" step does not do what it's cited for.** `glBlitFramebuffer` reads the
   bound `GL_READ_FRAMEBUFFER`'s attached texture, not the currently bound texture *unit* --
   `glBindTexture(GL_TEXTURE_RECTANGLE, rect)` before a blit is a no-op for the blit source. The actual
   precedent cited, `OutputPresenter.cpp:96` ("re-bind every frame: the reader-side rule"), rebinds the
   **FBO**, which uploadSlot already does separately. R-9's extra texture-unit rebind is dead code or a
   misdiagnosis of where the real CPU-write/GPU-read coherency guarantee comes from (IOSurfaceLock/Unlock +
   the ring's acquire/release ordering, both already correct). Remove or correct R-9's justification.

## NIT

6. w10's identity gate exercises only the surface path; the malloc fallback code that 4.4 "moves" (today's
   :419-437) is not independently diffed pre/post-lane, so a copy-paste slip there is ungated.

## Strongest counterargument to this paper
On Apple Silicon unified memory, CPU writes via IOSurfaceLock/Unlock and GPU reads of the same physical
pages plausibly need no re-`CGLTexImageIOSurface2D` after unpurge, making finding 1 excess caution with a
real (if tiny) per-upload cost. I hold it anyway: the plan itself flags this INFERRED and names the same
fix as a fallback -- the disagreement is only default-safe vs gated-by-one-dev-Mac's ctest, and that ctest
runs wherever CI happens to build, which may never exercise the discrete-GPU failure mode before it ships.
