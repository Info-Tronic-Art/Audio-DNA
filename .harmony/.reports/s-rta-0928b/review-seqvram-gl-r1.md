# Reviewer Verdict — seqvram-gl-r1 (lane seqvram, lens gl, round 1)
STATUS: PARTIAL
VERDICT: FAIL
REVIEWED: 5b78d43..d2b441119720423d8d8a40186dc46f0a0050bce9 (worktree .claude/worktrees/rta0928b-seqvram, branch lane/seqvram)

FILES: src/media/SeqVram.h (new), src/media/ImageSequence.{h,cpp}, src/render/Renderer.{h,cpp},
src/api/ApiServer.cpp, src/test/TestServer.cpp, tests/test_seq_vram.cpp, tests/CMakeLists.txt,
docs/claude/{pitfalls.md,rendering.md}, CLAUDE.md, .harmony/probe-seq-vram.{sh,py,json},
.harmony/.reports/s-rta-0928b/{seqvram.md,seqvram-scratch-rows.diff}

## MUST (1)
R-A/R-B: a real, verified render-thread stall ships with this diff. CompositorEngine.cpp:1093-1104
holds the layer (never calls renderLayerStages/applyTransition) whenever the INCOMING clip is
pending -- confirmed by reading the code: the `if (pending) {...} else { renderLayerStages(...) }`
branch skips the whole transition path, so the OUTGOING sequence's ImageSequence::getCurrentTexture
is never called for 1-3 frames at every crossfade start. Renderer::scanSequenceVram's idle test
(`lastDrawnSerial() + 1 < seqFrameSerial_`) then fires on that outgoing chain, and
ImageSequence::trimToMinimum deletes ~127 textures unbounded in one call under imageSeqMutex_ (no
per-frame cap on glDeleteTextures) -- builder-measured 22-24 ms render callbacks (3/7, 4/5, 3/5 runs
across three build variants) plus 4-11 late frames as the trimmed chain re-decodes mid-fade. The
adopted gate (H8 v8_crossfade_two_long) checks peak_frame_time_ms, which the docs themselves say
"cannot see" a frame-top stall (peak_callback_ms exists specifically to cover pre-renderStart work) --
this is exactly the "green test can't reveal it" pattern: v8 passed GREEN twice while the regression
reproduced in 3/7 runs of the same build. R-B is a second, independent instance of the same
unbounded-bulk-delete pattern at a deck switch (v7c scratch row, 5/5 runs, 22 ms, seq_evictions +127
in one poll).
Fix: this needs a Harmony ruling before merge, not a unilateral redesign (the plan explicitly fences
CompositorEngine.* out of this lane) -- e.g. treat a Pitfall-53-held layer's chains as drawn for
idle purposes, raise the idle threshold to N frames, and/or cap glDeleteTextures per frame in
trimToMinimum so a bulk trim amortizes over several frames. The builder already flagged this
(STATUS: PARTIAL, HANDOFF-NEEDS) and did not fix it -- correct per the fence, but it means the
diff as it stands should not merge silently.

## Verified fidelity (FOCUS items) -- no defects found
- Slots H1 (Full is a defined no-op: acquire() returns {-1, Full} without mutating state; bind/texOf/
  frameOf/release are no-ops on an invalid slot) and H2 (releaseAll() clears slots_ so a new context
  never reuses a name) match code exactly: src/media/SeqVram.h Slots::acquire/releaseAll.
- Evict-before-upload: ImageSequence::getCurrentTexture runs plan.evict (step 3) and slots_.shrink
  before the upload loop (step 4) -- ImageSequence.cpp ~line 340-360.
- lastShown_/cur are excluded from plan()'s eviction candidates and from trimToMinimum's sweep; ctest
  (5)(6)(7)(11) assert this directly, and (11) is a 2,000-step randomized-seek simulation with an
  explicit `REQUIRE(m.resident[shownBefore] != 0)` invariant per step.
- H3 running total: Renderer::syncMedia computes `others = seqResidentTotal_ - min(seqResidentTotal_,
  mine)` before the grant and `seqResidentTotal_ = others + seq->residentBytes()` after
  getCurrentTexture -- a true running total within the frame, re-seeded from scratch each frame top by
  scanSequenceVram. Confirmed in Renderer.cpp.
- Frame-top scan under imageSeqMutex_, no new mutex: scanSequenceVram is called once at the top of
  renderOpenGL (GL thread), takes the SAME imageSeqMutex_ syncMedia already holds per drawn clip; grep
  of the whole diff shows exactly one new std::lock_guard, on that existing mutex. GL deletes
  (glDeleteTextures) happen only inside this GL-thread function, never off-thread.
- Context loss: ImageSequence::releaseGL now drains slots_.releaseAll() (which empties the table) and
  deletes every returned texture name -- matches H2.
- H6 in/out range: Transport carries inFrame/outFrame; SeqVram::setRange and step() were re-derived
  against the ACTUAL Renderer::syncMedia code (read at Renderer.cpp ~1674-1686 and ~1744-1756), which
  enforces only the out point (Loop/PingPong jump to in via seekTo, never reflect there; OneShot
  stops) -- the builder's D2 deviation from the adoption's "PingPong reflects at in and out" is a
  correct, disk-verified re-derivation, not an error, and is honestly logged. ctest (12)/(12b) cover
  wrap, the "past out point" recovery branch, and the eviction-order-outside-range-first case; I
  hand-traced case (12)'s evict={20,195} assertion against the trajectory distances and it holds.
- Pure tests are independently GREEN: I built and ran tests/test_seq_vram directly (not trusting the
  report) -- `./build-lane/tests/test_seq_vram --reporter compact` -> "All tests passed (4889249
  assertions in 17 test cases)". Teeth (mutated copies dropping the lastShown guard, the Loop wrap,
  the PingPong bounce, etc.) are recorded in the committed report with FAIL lines; I confirmed by code
  trace (not a live rebuild -- see LIMITATION below) that removing the `j != lastShown` guard in
  plan()'s candidate filter mechanically breaks ctest (11)'s per-step residency invariant, matching
  the claimed teeth table.
- CLAUDE.md is 24,665 B (< 25,000 cap); docs/claude/pitfalls.md and rendering.md text match the
  shipped code (field list, kBudgetBytes, kMinWindowFrames, in/out semantics) -- no overclaiming found.
- No stray files: no .venv symlink, no leftover env-var instrumentation in shipped src/ or
  .harmony/probe-seq-vram.py (grepped for SEQVRAM_FIXED_WAIT/v7c_one_per_deck: absent from the
  committed probe; those live only in the disclosed scratch diff). No new mutex; audio callback and
  analysis thread untouched (grep of the diff touches only src/media, src/render/Renderer, src/api,
  src/test, tests, docs).

## SHOULD (1)
D7 (self-disclosed by the builder): in the upload loop, `budget.take(it->rgba.size())` runs before
`uploadFrame()`/`slots_.acquire()`, so when acquire() returns Full (H1's rare, legitimate path) that
frame's shared UploadBudget bytes are already spent for an upload that never happened, starving other
sequences'/images' uploads that frame. Low-frequency in practice (allowanceFrames never returns below
the 8-frame floor, which is sized exactly to the demand+look-ahead+arriving count), but the fix is a
one-line reorder (check `slots_.wouldAcquire`-style feasibility, or acquire before take). Not
blocking; file for a follow-up.

## LIMITATION (methodology, disclosed)
I attempted to independently rebuild the pure ctest with a mutated SeqVram.h in a `$TMPDIR` copy of
the worktree to execute (not just trace) one teeth case. The copy's CMakeCache.txt caches an absolute
CMAKE_HOME_DIRECTORY/CMAKE_CACHEFILE_DIR pointing at the ORIGINAL worktree, so `cmake --build
<copy>/build-lane` silently redirected the configure/build step back to the original tree's
build-lane (confirmed via the "Build files have been written to
.../rta0928b-seqvram/build-lane" message) and recompiled the UNMODIFIED original source -- my mutation
was never exercised, and the "all tests passed" result from that step is invalid and was discarded
from the findings above. `git status`/`git diff` on the original worktree confirm no tracked file was
changed by this (only a redundant, idempotent rebuild of an already-untracked build-lane/ occurred);
no source under review was mutated. Given this, the teeth-line confirmation above is by code trace
(high confidence, tied to ctest (11)'s exact assertion) rather than by independent execution -- a
future reviewer of a CMake-based lane should reconfigure a copy from a clean build dir (or use a
side-by-side compile against Catch2 headers directly) rather than reusing a copied build-lane/.

SUMMARY: 17 files reviewed (source + tests + docs + probe), 1 MUST, 1 SHOULD, 0 NIT. Implementation
of the FOCUS items (Slots H1/H2, evict-before-upload, lastShown/cur protection, H3 running total,
mutex-scoped frame-top scan, context loss via releaseAll, H6 in/out range, pure-test correctness) is
careful, disk-verified, and matches the plan + Harmony adoption with only honestly-logged,
code-re-derived deviations (D1, D2, D4). The blocking issue is not a code-quality defect in what was
built but a real, reproducible cross-module regression (R-A/R-B) that the builder found, could not
fix inside this lane's fence, and explicitly flagged for a Harmony ruling -- merging as-is would ship
a musician-visible ~22 ms stall at ordinary crossfade/deck-switch moments that the adopted gate does
not reliably catch. VERIFIED: SeqVram.h/ImageSequence/Renderer logic (read + hand-traced + ctest
executed live), CompositorEngine.cpp:1093-1104 pending-hold branch (read live, confirms R-A's
mechanism), CLAUDE.md byte count, absence of stray probe instrumentation in the shipped file, ctest
865 math (848+17). INFERRED: exact teeth-line failures under mutation (code trace, not executed --
see LIMITATION). Recommend: REQUEST_CHANGES / do-not-merge until Harmony rules on R-A/R-B (or a
mitigation lands); everything else in this lane is ready.
