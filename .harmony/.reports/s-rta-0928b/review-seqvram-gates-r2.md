# Reviewer Verdict — review-seqvram-gates-r2
STATUS: DONE
VERDICT: PASS_WITH_NITS
COMMIT: 902a3c4 (lane/seqvram, worktree rta0928b-seqvram), fix-round delta d2b4411..902a3c4, base 5b78d43

FILES: src/media/SeqVram.h, src/media/ImageSequence.h/.cpp, src/render/Renderer.h/.cpp,
src/api/ApiServer.cpp, src/test/TestServer.cpp, tests/test_seq_vram.cpp, .harmony/probe-seq-vram.{py,json},
docs/claude/pitfalls.md, docs/claude/rendering.md, .harmony/.reports/s-rta-0928b/seqvram.md

VERIFIED (disk-executed, this review, not recall):
- Read the full fix-round diff d2b4411..902a3c4 for every file in FILES against plan-seqvram.md's
  "HARMONY ADOPTION ADDENDUM -- fix round" F1-F9, line by line.
- F1 (idle-age): `SeqVram::isIdle(lastDrawn, serial)` = `serial > lastDrawn && serial-1-lastDrawn >= kIdleFrames(60)`.
  Hand-traced all 8 boundary cases in ctest (13) against the formula myself; matches. This raises the idle
  threshold from "1 frame gap" (the R-A bug) to 60 frames, so a Pitfall-53 hold (1-3 frames) never trips it --
  closes MUST-1 from review-seqvram-gates-r1.md by construction.
- F2 (delete budget): `SeqVram::DeleteBudget`, `Slots::shrink(cap, maxDeletes)` and `Slots::releaseSome(maxDeletes)`
  hand-traced: both stop handing back textures once `gone.size() >= maxDeletes`, leaving the rest as allocated
  FREE slots (not deleted, not re-created later: `acquire()` still prefers a free slot of matching size). Wired:
  `Renderer::seqDeletes_.reset()` at the frame top, before `drainRetiredMedia()` and `scanSequenceVram()`, SHARED
  by both -- closes MUST-2 from r1 by construction (no more single-frame unbounded `glDeleteTextures` loop).
- F3 (drawn outranks idle): `SeqVram::drawnAllowance` algebra re-derived by hand and matches
  `allowance = max(floor, budget - drawnOthers - idleMinBytes)` exactly as adopted; `trimmedBytes() =
  min(residentBytes(), 2*frameBytesHint_)` is a THEORETICAL floor, not "already trimmed" -- confirmed this is
  the intentional mechanism per the addendum text ("as the drawn sequence grows the total exceeds the budget and
  the frame-top scan trims idle sequences"), not a bug: the design deliberately lets a drawn sequence overshoot
  physical VRAM temporarily, reclaimed on a later frame's trim, bounded by F2's delete-budget rate.
- F4 (canAcquire before budget.take): confirmed in `ImageSequence::getCurrentTexture`'s upload loop -- the
  `canAcquire` check runs and `continue`s (counting `seq_upload_deferred`) BEFORE `budget.take()` is ever called,
  exactly as ruled.
- F5/F6 gates use `peak_callback_ms` (not `peak_frame_time_ms`), closing SHOULD-1 from r1 (the frame-top scan runs
  outside the frame timer, so only the callback metric can see it). F7 reads `seq_drawn_textures` at fade-end+3s
  (not "first sample >= threshold", which the report shows would pass vacuously at 0.003s with the OLD/idle-still-
  counted value). F8 (`v11_retire`) is report-only (INFO prints, no `ok`/`no` assertions) -- matches "not asserted
  this round".
- Independently ran `./build-lane/tests/test_seq_vram`: "All tests passed (4889364 assertions in 21 test cases)"
  (17 round-0 + 4 new: (13)(14)(15)(16), matching F1-F4). Independently ran
  `ctest --test-dir build-lane -j1`: "100% tests passed, 0 tests failed out of 869" -- an EXACT match to the
  report's claimed 869/869, not merely trusted.
- probe-seq-vram.json diff: only additions (`incomingMinTextures`, two new layer-id entries for
  v7c_one_per_deck/v11_retire); every pre-existing threshold (budgetMB, frameMB1080, frameMB4k, minWindowFrames,
  shownTol, lateMaxAfterJump, boxTol, peakMaxMs) is byte-identical to the adoption -- no silent re-threshold.
- probe-seq-vram.py diff: only v8 (extended with (b')/(f)) and the two new rows v7c/v11 changed; v1/v2/v4/v6/v9/
  v7/v7b/v3/v10 bodies are untouched in this diff.
- `git diff 5b78d43..902a3c4 -- docs CLAUDE.md`: the only removed line is the single old sentence replaced 1:1 by
  the expanded description (net additive); CLAUDE.md is not touched at all in the fix round and stays at 24,665 B
  (<= 25,000, `wc -c` re-run). `git diff --stat 5b78d43..902a3c4 -- src/ui src/render/CompositorEngine.* src/model`
  is empty -- the plan's fence is respected across the whole lane, not just the fix round.
- `grep` for `std::mutex`/`lock_guard` in the fix-round diff: only the pre-existing `retiredMediaMutex_` and
  `imageSeqMutex_` (both context lines, not additions) -- no new mutex (Sacred Rule 2). No `sleep`/wait added to
  the GL thread; all new work (isIdle check, DeleteBudget arithmetic, drawnAllowance) is O(1) or O(#sequences).
- `git status --porcelain -uall` on the worktree (excluding build-lane/) is empty: no `.venv` symlink, no stray
  files, no leftover env-var hook (`grep -rn getenv` in the touched media/render files: none).
- Report table (RED/GREEN verbatim lines, F1-F9) cross-checked line by line against the code behavior it claims;
  every RED line matches a code path that existed before the corresponding fix commit, every GREEN line matches
  the fixed code's actual logic (not just re-read as prose).

NOT independently re-run live (rig/time cost; the pure-logic paths that gate them were independently verified
above, so this is INFERRED from the report + code trace, matching r1's own practice for expensive live checks):
the live probe-seq-vram 66/0 double-green run, the F5/F6 5-of-5 A/B timing runs against the d2b4411 app, image-load
37/0 and crossfade 35/0 re-runs. The report's per-run millisecond numbers (5.4-11.3ms fixed vs 21.2-24.9ms broken)
are large enough margins that transcription error is not a plausible confound, and the underlying mechanism (F1
raising the idle threshold to 60 frames, F2 capping deletes to 8/frame) is independently verified correct by code
+ ctest, which is what actually produces those numbers.

ISSUES:
SHOULD-1 (design interaction, not evidenced live -- a note for Harmony, not a blocker): `Renderer::renderOpenGL`
  resets `seqDeletes_` then calls `drainRetiredMedia()` BEFORE `scanSequenceVram()`'s idle trim, and both consume
  the SAME 8-delete/frame budget (F2, `src/render/Renderer.cpp` frame-top sequence). Because F3's `drawnAllowance`
  credits an idle sequence's memory above its 2-frame floor as reclaimable the instant it goes idle -- regardless
  of whether any eviction/shrink has actually run on it yet (`trimmedBytes()` is a theoretical floor, not a
  measured one, see VERIFIED above) -- a drawn sequence's grant can assume memory is free before it physically is.
  Normally F2's per-frame trim reclaims it within a fraction of a second (the report measures ~0.25s for a
  121-frame sequence). But if `drainRetiredMedia` consistently consumes the whole 8-delete budget first (a
  sustained backlog of retired sequences, e.g. several clips closed in quick succession), `scanSequenceVram`'s
  idle-trim loop can see `seqDeletes_.left <= 0` and `break` before evicting ANY idle sequence that frame, for as
  many frames as the drain backlog lasts -- delaying the physical reclaim of memory F3 already optimistically
  lent to a drawn sequence, and pushing the actual physical VRAM further over `kBudgetBytes` than the "one frame's
  uploads" tolerance the plan/H3 describe. This is self-limiting (bounded by however many sequences are mid-retire)
  and not exercised by the adopted v11_retire row (explicitly report-only, F8). Suggested fix if Harmony wants it
  closed: reserve a minimum share of the per-frame delete budget for the idle trim (e.g. split 8 as drain-first-4/
  trim-remainder, or alternate priority by frame parity) so a retire backlog cannot fully starve the reclaim a
  drawn sequence's grant is already counting on.

SUMMARY: 10 files reviewed line-by-line against the fix-round addendum (F1-F9), 0 MUST findings, 1 SHOULD (a
narrow, self-limiting, not-live-evidenced interaction between F2's shared delete budget and F3's optimistic idle
credit, worth a note but not a defect that ships). Both round-1 MUSTs (R-A crossfade-start idle misclassification,
R-B unbounded single-frame bulk delete) are closed by F1/F2, verified by code trace AND by independently running
the pure ctest suite (21/21 new+existing test_seq_vram cases, 869/869 full serial ctest -- exact match to the
report's claimed count, not merely trusted). r1's SHOULD-1 (peak_frame_time_ms blind spot) is closed by F5's
peak_callback_ms gate. Docs are additive, CLAUDE.md is untouched and under the 25,000 B cap, the plan's file fence
is respected across the whole lane, and no new mutex, stray file, or env-var hook was introduced.
METADATA: reviewer=reviewer-seqvram-gates-r2, builder_packet=s-rta-0928b, date=2026-09-28
