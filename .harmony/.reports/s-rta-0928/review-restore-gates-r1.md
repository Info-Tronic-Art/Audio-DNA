# Reviewer Verdict — s-rta-0928 restore lane (lane/restore-0928), round 1
STATUS: DONE
VERDICT: PASS_WITH_NITS

REVIEWED: worktree /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0928-w4,
git diff 734f011..a232c34 (never the working tree, never main). Plan:
.harmony/.reports/s-rta-0928/plan-restore.md incl. HARMONY ADOPTION D1-D8.

## Summary

Every product/test diff traced against the plan's literal code blocks (Player.cpp/.h,
RoutineEngine.cpp/.h, MainComponent.cpp, ClipThumbnails.h, ClipCell.{h,cpp},
LayerStrip.{h,cpp}, DeckView.{h,cpp}, tests/CMakeLists.txt, test_clip_thumbnails.cpp,
test_deck_thumbnails.cpp, test_take.cpp, test_routine_engine.cpp, probe-routines.sh,
docs/claude/{recording,pitfalls}.md, CLAUDE.md, APP-INVENTORY.md) — all match verbatim
or with adopted (D1-D8) deviations, correctly applied. All 23 changed files are inside
the plan's FENCE; nothing touches src/render/*, src/media/*, ApiServer.cpp, TestServer.cpp,
Clip.h, or renderleft's files.

Cross-checked RED/GREEN/teeth claims in the lane report against real saved evidence on
disk (not taken on faith):
- red-step1-engine.txt / red-B0.txt / teethA.out / teeth-store.out / teeth-store2.out /
  teeth-grid.out / teethH1.out all reproduce the exact FAILED lines, expansions and
  line numbers the report cites (C1-C5, E1, D5, G5 :1139, J2 :1523, B0, S1/S3/S4/S6,
  t5a/t5b).
- runs/lane-routines-114552.log and runs/base-routines-114000.log show the exact 5h/8h/
  11h/7m PASS (lane, 0.136-0.208 ms) / FAIL (main's app, "absent" / stuck-at-1.0) lines
  quoted in the report, and 109/0 vs 105/4.
- runs/lane-display-120514.log's 01-idle-w0.png screenshot visually confirms the L1/L3
  image cells and strips show the decoded test-card thumbnail (Boris-check evidence).
- probe-display/step3/beatclock/decktabs lane logs all show the claimed N/0 counts.
Ctest count arithmetic (784 lane-base + 13 tempo-merge + 17 new-this-lane = 814) is
self-consistent but I found no standalone saved "814/814" ctest transcript in the
evidence root (only per-suite subsets); see NIT below.

## Load-bearing logic traced line-by-line

- `Player::advanceTo` close branch: `if (!cur.displaced && sink.set(lane.key,
  g.curve.eval(g.x1))) sink.release(lane.key);` — a refused end-write correctly skips
  release and leaves `cur.displaced` semantics intact for the next gesture (matches
  plan Step 2.1 verbatim, including the comment).
- `RoutineEngine::SlotSink::set` ownership-refusal path is unchanged and is what the
  new end-value write flows through — traced the D5 stacking test (gesture-end refused
  when a later routine holds the key) tick-by-tick against `Player.cpp` + `SlotSink`
  and it is internally consistent (yielded==1 only, no stray Set/Release logged since
  `FakeDispatch` only logs an accepted `dispatch.set` call).
- `ClipThumbnails::get()` — D4 ("no stat while in flight") verified structurally:
  `inFlight_.count(path)` (a `std::set` lookup, no I/O) is checked and returned on
  BEFORE any `getLastModificationTime()` call; test S6 asserts `fileStats()` does not
  grow across 30 pending `get()` calls, against the real production method (not a
  mock) — this satisfies the "real code, not synthetic fixture" bar for a new
  correctness-asserting test.
- `ClipCell`/`LayerStrip::updateThumbnail` memo conditions: traced both the
  "still pending → re-pull every refresh" path and the "landed → skip re-derive" path;
  matches B0-B2 test expectations and plan Step 6 code blocks essentially verbatim
  (only cosmetic: `repaint()` moved to unconditional tail, per plan's own pseudocode).
- `holdMs`/`holdMsMax` instrumentation in `RoutineEngine::startNow` / `tick()` /
  `publishStatus` and `MainComponent::routineStatusVar` — exact match to plan Step 3.
- Sacred rules: no audio/analysis/render thread files touched; `ClipThumbnails` uses a
  2-thread low-priority `juce::ThreadPool` + `MessageManager::callAsync`, no new mutex
  (message-thread-only mutation of `inFlight_`/`failed_`/`cache_`, guarded by
  `jassert` on-message-thread checks) — consistent with CLAUDE.md Sacred Rule 2.

## Findings

None are MUST (no shipped defect, no missed FENCE/plan item, no un-failable test found).

### SHOULD
1. No standalone full-`ctest` transcript ("100% tests passed, 0 tests failed out of 814")
   was found saved under the evidence root (only per-suite RED/GREEN excerpts and teeth
   outputs). The 814 arithmetic is self-consistent (784 lane-base + 13 from the tempo
   rebase + 17 new this lane) and every individual new-test RED/GREEN transition is
   independently verified on disk, so this is a documentation-completeness gap, not a
   correctness doubt. Fix: save one `ctest --test-dir build-lane -j1` tail alongside the
   other evidence files next time.

### NIT
1. `ClipThumbnails::get()` calls `imageFile.getLastModificationTime()` twice on a
   cache-miss, not-in-flight, not-failed path (once for the `failedKey`, once inside
   `ThumbnailCache::get()`) — two real stat() syscalls instead of one. Correctness is
   unaffected (D4 only requires no stat while *pending*, which holds), and avoiding it
   would need changing `ThumbnailCache`'s public API (out of this lane's FENCE), so this
   is acceptable as shipped.

## SLIM

No `EXCESS_DEAD`/`EXCESS_VESTIGIAL`/`EXCESS_DUP`/`EXCESS_SPEC` found. The new counters
(`decodesQueued()`, `lookups()`, `fileStats()`) on `ClipThumbnails` are all exercised by
production-code-driving tests (S1/S6, B1/B2) that assert the plan's stated invariants
directly against the real `get()`/`landed()` implementation — `JUSTIFIED_KEEP` (they are
the mechanism by which D4 and the memoization contract are proven against real code, not
a mock).

## Scope completeness (locked decisions)

HARMONY ADOPTION D1-D8 all checked against cited evidence, not just plan text:
- D1 (teeth also flip G5/J2 RED): confirmed in teethA.out.
- D2 (5g/8g/9g/11g citation, not 11j): probe-routines.sh diff touches none of those rows.
- D3 (row text "diagnosed causes only" + T1/T2 re-run): row text present verbatim in
  probe-routines.sh and in the live logs; T1/T2 table present in restore.md with a
  scratch-copy build, `strings` count 0 confirmed by report (not independently re-run
  by me — accepted as a live/build-time claim outside static review scope).
- D4 (no stat while pending): verified structurally + by S6 test against real code.
- D5 (stacking test): present, traced logically, matches existing stacking-test shape.
- D6 (re-grep citations): DeckView.cpp/ClipCell.cpp comments cite current line numbers;
  spot-checked against the diff context, consistent.
- D7 (pitfall number "NN" until rebase): confirmed in CLAUDE.md, pitfalls.md, recording.md.
- D8 (Boris list): both items present in restore.md's BORIS LIST section.

## Files reviewed (all 23 changed, full diff read)
src/recording/Player.{h,cpp}, src/recording/RoutineEngine.{h,cpp}, src/MainComponent.cpp,
src/ui/ClipThumbnails.h (new), src/ui/ClipCell.{h,cpp}, src/ui/LayerStrip.{h,cpp},
src/ui/DeckView.{h,cpp}, tests/CMakeLists.txt, tests/test_clip_thumbnails.cpp (new),
tests/test_deck_thumbnails.cpp (new), tests/test_take.cpp, tests/test_routine_engine.cpp,
.harmony/probe-routines.sh, docs/claude/recording.md, docs/claude/pitfalls.md, CLAUDE.md,
.harmony/APP-INVENTORY.md, .harmony/.reports/s-rta-0928/restore.md.

METADATA: reviewer=reviewer-agent, builder_packet=plan-restore.md (HARMONY ADOPTION),
date=2026-09-28
