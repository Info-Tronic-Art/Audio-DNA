# Reviewer Verdict — seqvram-gl-r2 (lane seqvram, lens gl, round 2)
STATUS: DONE
VERDICT: PASS_WITH_NITS
REVIEWED: d2b4411..902a3c4 (fix round 1 delta), lane 5b78d43..902a3c4, worktree .claude/worktrees/rta0928b-seqvram, branch lane/seqvram

FILES: src/media/SeqVram.h, src/media/ImageSequence.{h,cpp}, src/render/Renderer.{h,cpp}, src/api/ApiServer.cpp,
src/test/TestServer.cpp, tests/test_seq_vram.cpp, .harmony/probe-seq-vram.{py,json}, docs/claude/{pitfalls.md,rendering.md},
.harmony/.reports/s-rta-0928b/seqvram.md

## Verified fidelity (FOCUS: F1-F4, ordering, texture lifetime, GL-thread-only) — no defects found
- F1 idle-age rule: `SeqVram::isIdle(lastDrawnSerial, frameSerial)` = `frameSerial > lastDrawnSerial && frameSerial - 1
  - lastDrawnSerial >= kIdleFrames(60)`. Hand-verified the 59/60 boundary against the code AND independently ran the
  shipped ctest binary (`.../build-lane/tests/test_seq_vram --reporter compact` → "All tests passed (4889364
  assertions in 21 test cases)", 21 = 17 (round 0) + 4 new (13)-(16)). `scanSequenceVram` uses `isIdle` (not the old
  `lastDrawnSerial()+1 < seqFrameSerial_ && residentSlots()>2`), so a Pitfall 53 pending hold (1-3 frames) never
  qualifies — this closes review-gl-r1's MUST (R-A).
- F2 shared delete budget: `Renderer::seqDeletes_` (`SeqVram::DeleteBudget`, kMaxDeletesPerFrame=8) is reset once at
  the top of `renderOpenGL` before `drainRetiredMedia()`/`scanSequenceVram()`, and threaded into every per-sequence
  `getCurrentTexture` call via `Grant::deletes`. `Slots::shrink(cap, maxDeletes)` and the new `releaseSome(maxDeletes)`
  both `break`/`return` BEFORE erasing/popping the slot once `gone.size() >= maxDeletes`, so a texture not deleted
  this call stays allocated (free, reusable, still counted in `allocatedBytes()`) — confirmed by adversarial
  compile-and-run of the actual header in a $TMPDIR copy (not the shipped code path — see METHOD) plus the shipped
  ctest (14). Context loss (`drainRetiredMedia(true)`) still calls `releaseGL()`/`releaseAll()` unconditionally, so
  nothing is throttled at teardown. This closes R-B (the ~22ms unbounded bulk delete).
- F3 drawn-outranks-idle: `SeqVram::drawnAllowance(total, mine, idleBytes, idleMinBytes, floor)` = `allowance(drawnOthers
  + idleMinBytes, floor)`. `Renderer::scanSequenceVram` publishes `seqIdleBytes_`/`seqIdleMinBytes_` (post-trim, so
  already reflects whatever F2 budget managed to delete this frame); `syncMedia` self-corrects both sums for a
  sequence that was idle at the frame-top scan but is drawn this same frame (subtracts its own `mine`/`trimmedBytes()`
  before computing its own grant), so multiple such sequences in one frame don't double-count. H10 floors-win is
  preserved (`allowance()` always returns `>= minBytes`). No allocation, no new mutex — reuses `imageSeqMutex_`
  (grepped the whole fix-round diff for `mutex|lock_guard`: only 2 lock sites, one on the pre-existing
  `retiredMediaMutex_`, one on the pre-existing `imageSeqMutex_`).
- F4 canAcquire-before-take: `Slots::canAcquire(frame, cap)` is checked in the upload loop BEFORE `budget.take()`, and
  I confirmed by reading `acquire()` that `canAcquire`'s three conditions (`slotOf>=0 || size<cap || any free`) are the
  exact negation of `acquire`'s `Full` path — a Full result never spends the frame's upload budget.
- Ordering: evict (step 3, before any shrink) → shrink under the shared delete budget → upload (step 4, canAcquire
  gate first) → request (step 5) — unchanged from round-0's verified order, still true after the fix-round edits.
- Texture lifetime / GL-thread-only: every `glDeleteTextures` call in the diff funnels through the new
  `ImageSequence::deleteTextures` helper, called only from `trimToMinimum` and `releaseGLWithin`, both GL-thread-only
  (invoked from `Renderer::scanSequenceVram`/`drainRetiredMedia`, which run inside `renderOpenGL`). No new thread
  touches a GL object.

## Adversarial testing (disk-verified, $TMPDIR copy of the actual shipped header — no mutation of the reviewed tree)
Copied `src/media/SeqVram.h` verbatim into a scratch dir and compiled a standalone harness (`g++ -std=c++20
-fsanitize=address,undefined`) exercising cases beyond the shipped ctest: `drawnAllowance` with `total < mine` (an
input that can't occur but shouldn't UB) → clamps to the full budget, no underflow; inconsistent idle sums (`idle >
others`) → clamps to `idleMin`, matches ctest (15)'s "inconsistent sums never underflow" row; `isIdle` at the exact
59/60 boundary → matches; `Slots::shrink`/`releaseSome` with `maxDeletes=0`, `maxDeletes` larger than the table, and a
table of all-occupied slots (`releaseSome` must delete "occupied or not") → all behaved per spec, ASan/UBSan silent.
One narrowing-cast curiosity, not exploitable (see NIT below).

## Gate soundness (fail-closed, not toothless)
- `v8` (f) explicitly FAILs (never skips) both when the fade never completes within 8s AND when
  `seq_drawn_textures` is absent from `/api/state` on an app that predates it (`.harmony/probe-seq-vram.py` v8:
  `no(f"... /api/state has no seq_drawn_textures ...")`) — read at fade-end + 3s (not "first sample >= 64", which the
  report shows would pass vacuously while the not-yet-idle outgoing chain is still counted).
- F5/F6/F7 RED-arm evidence in `seqvram.md`'s table is by value against the actual d2b4411 app (22-24ms / 21-25ms /
  "holds 8" with F3 disabled via an in-place teeth build, sha256-restored) — these are real fail-capable gates, not
  gates that only ever pass.
- Docs (`docs/claude/pitfalls.md` NN, `docs/claude/rendering.md`) accurately describe the shipped mechanism (kIdleFrames
  = 60, kMaxDeletesPerFrame = 8, `SeqVram::drawnAllowance`) — no overclaiming found. `git diff 5b78d43..902a3c4 -- docs
  CLAUDE.md` removes only the pre-plan legacy "Image sequences" sentence (the intended, documented replacement);
  CLAUDE.md is 24,665 B (< 25,000 cap, unchanged this round). No `.venv` symlink, no env-var hook, no leftover
  instrumentation in the shipped diff (grepped for `SEQVRAM_FIXED_WAIT|getenv|os.environ`: the only hit is a report
  sentence stating their absence).

## NIT (1)
`SeqVram::DeleteBudget::spend(size_t n)` does `left = std::max(0, left - static_cast<int>(n))` — narrowing a `size_t`
to `int` without a bounds check. Not exploitable today (every caller passes `gone.size()` bounded by a sequence's
slot count, realistically well under 1000), demonstrated in the adversarial harness that an astronomically large `n`
wraps mod 2^32 rather than saturating, which could silently under-spend the budget if a caller ever grew. Not
blocking; a one-line `std::min<size_t>(n, static_cast<size_t>(std::numeric_limits<int>::max()))` clamp before the cast
would future-proof it. `src/media/SeqVram.h`.

## Residual (disclosed by builder, not a review MUST)
2 single-run late-frame flakes on the FIXED app (v3 1-of-8, v8 1-of-12), neither repeating in follow-up runs, one
correlated with a Firefox/GPU-helper CPU burst recorded by the report's sampler. Explicitly flagged in
`seqvram.md`'s HANDOFF-NEEDS for a Harmony ruling — correctly disclosed rather than re-thresholded away; does not
block this round's MUST-gate (R-A/R-B are closed with 5/5 live evidence).

SUMMARY: 13 files reviewed. 0 MUST, 0 SHOULD, 1 NIT. Both prior review MUSTs (R-A idle-during-pending-hold, R-B
unbounded bulk delete) are closed with code that matches the addendum's F1-F4 rulings exactly, verified by direct
source reading, an independent run of the shipped ctest binary (21/21, 4.89M assertions), and an adversarial
recompile of the actual header in a $TMPDIR copy. Docs are additive and accurate; no stray files; no new mutex; GL
deletes remain GL-thread-only and bounded. VERIFIED: SeqVram.h/ImageSequence.cpp/Renderer.cpp logic (read +
hand-traced + adversarially recompiled + ctest executed live), CLAUDE.md byte count, docs diff scope, mutex count.
INFERRED (trusted from the builder's detailed before/after ms tables, not independently re-run in this review — a
full GUI live A/B was out of scope for a read-only source review): the 5-of-5 live timing numbers for F5/F6/F7.
METADATA: reviewer=reviewer-agent, builder_packet=s-rta-0928b-seqvram-gl-r2, date=2026-09-28
