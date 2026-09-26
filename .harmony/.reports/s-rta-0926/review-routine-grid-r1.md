# Reviewer Verdict — routine-grid r1
STATUS: DONE
VERDICT: APPROVE

## Scope reviewed
Pinned worktree `.claude/worktrees/wf_b3f13eab-924-1` @ 84c3637 (branch worktree-wf_b3f13eab-924-1).
`git diff ebbff22..84c3637 --stat` (lane-relevant subset, excluding prior-commit doc churn from ebe0d4e):
- src/recording/RecorderClock.cpp (5 lines, commit cb1c4f0 — OUTSIDE the packet fence)
- src/recording/RoutineEngine.{h,cpp} (12 lines, commit 54e252f — IN fence)
- tests/test_take.cpp (+31), tests/test_routine_engine.cpp (+108) (both fence commits)
- .harmony/.reports/s-rta-0926/routine-grid.md (+166, docs commit 84c3637)
- .harmony/probe-routines.sh: VERIFIED unchanged (`git diff` empty), matching the report's claim.

## VERDICT ON THE LANE'S OWN VERDICT: correct
"App bug, not probe bug, not routine code" — VERIFIED independently, not just re-stated:

1. **RecorderClock root cause (cb1c4f0), traced by hand**: `beat = wholeBeats_ + lastPhase_ +
   beatOffset_`. Pre-fix, `beatOffset_` seeds to 0 on the first tick even though `lastPhase_` seeds
   to the tracker's live phase, so every later tick's `beat` carries that phase as a standing offset.
   I re-derived the gate take's numbers from the formula: seed phase 0.84, 0.6s at 120 BPM (1.2 true
   beats, 2 phase wraps) gives buggy `beat = wholeBeats_(2) + phase(0.04) = 2.04` — matches the
   report's stated RED value (2.056, same mechanism, negligible fixture-vs-real-snapshot
   difference) and is inside the reported family (+0.84 to +0.86 on all 4 gate-take stamps).
   Post-fix (`beatOffset_ = -seedPhase = -0.84`): `beat = 2 + 0.04 - 0.84 = 1.2` — exact.
2. **Second defect (54e252f), traced by hand**: pre-fix `RoutineEngine::tick` used
   `floor(clock_.beat)` for the Beat edge. `clock_` is the engine's OWN RecorderClock, ticked from
   app start (usually unmetered) and its "re-lock" branch (`wasUnmetered` in RecorderClock.cpp)
   unconditionally absorbs the phase at lock into `beatOffset_` — this absorption happens
   independently of cb1c4f0 (the lock-branch already computed `target - wholeBeats_ - phase`
   before this session). So after a lock at raw beat 0.8125, `clock_.beat` runs continuously as
   `rawBeat - 0.8125`; `floor(clock_.beat)` therefore wraps at raw beats 1.8125, 2.8125, 3.8125 —
   not at the tracker's true 1.0/2.0/3.0. I reproduced this arithmetic by hand against the new
   `test_routine_engine.cpp` [quantize] section and it lands on exactly the reported RED value
   (2.8125), and the post-fix rule (`snap.beatPhase < lastBeatPhase_ - 0.5f`, wrapping at true
   integer beats) gives the reported GREEN (start at 3.0). This is the same edge rule already used
   by `src/model/Autopilot.cpp:44` — confirmed by reading that file; the "same rule a Beat-quantized
   clip uses" claim is literally true, not just similar.
3. **Plan-fidelity check (sections 3.3/3.5/3.6 of `plan-routines-s1-final.md`)**: read directly.
   `sliceRoutine`'s `beat -= fromBeat` (3.3 step 4) and the engine's `pos = clock.beat - startBeat`
   (3.5) together mean, for a `fromBeat 0` cut, routine beat 0 == the take's Record instant, and a
   move at take beat X plays X/2 seconds after the bar the routine starts on. That reproduces the
   report's "correct expected" +0.60/+2.61/+4.60/+6.60 exactly from the plan text, independent of
   the report's own restatement. The "same place in the bar" guarantee is explicitly scoped to
   bar-line cuts (`takeBeatOfBar`, 3.6) in the plan text itself — the report's claim that a
   `fromBeat 0` cut is NOT covered by that guarantee is a correct reading of the plan, not a
   post-hoc excuse.
4. `takeBeatOfBar` (`src/recording/RoutineSlice.cpp:442-446`, untouched by this fix) matches the
   plan's formula verbatim: `(bar-1)*4 + fmod(4-start, 4)`. With `start=3.84`, bar 1 = 0.16 — matches
   the new ctest's own `CHECK(takeBeatOfBar(take, 1) == Approx(0.16))`, an independent re-derivation
   from the (unmodified) formula, not circular against the app's buggy output.

## Test quality (RED-first, non-vacuous) — VERIFIED by re-deriving expected values, not by re-reading the claim
- `tests/test_take.cpp` [recorderclock] new case: seeds phase at 0.84/0.25/0.99, drives 72 ticks of a
  real synthetic clock, checks `beat == 1.2` and the D1 lint. Hand-traced above: genuinely RED
  pre-fix (computed 2.04 for seed 0.84), genuinely GREEN post-fix. Not vacuous — the expected value
  (1.2) is a closed-form fact of the tempo (120 BPM × 0.6s), not derived from the code under test.
- `tests/test_routine_engine.cpp` [bargrid] "a move keeps its place in the bar...": drives a REAL
  `RecorderClock` (not the `Rig`'s synthetic shortcut) to generate the take stamp, then checks both a
  `fromBeat 0` cut (expect 1.2) and a bar-line cut (expect 1.04, derived from the untouched
  `takeBeatOfBar`/known Record phase — not from the app's output) and that the engine fires the
  bar-cut routine at the correct downbeat-relative tick (5.0625, not 5.0). Non-circular: the oracle
  values are hand/formula-derived, never read back from the buggy path.
- `tests/test_routine_engine.cpp` [quantize] new section: hand-traced above as genuinely RED (2.8125)
  pre-fix, GREEN (3.0, the true next beat) post-fix.
- Coverage of the fix's domain: RecorderClock's fix depends only on `beatPhase` (always in [0,1)), so
  sweeping seed phases 0.84/0.25/0.99 covers the full domain regardless of `startBeatInBar`'s literal
  [0,4) range (which is just `frac(startBeatInBar)` for this purpose). The bar-grid ctest spot-checks
  one deliberately-nontrivial `startBeatInBar` (3.84, near the bar wrap) rather than sweeping [0,4);
  combined with the dev-run table (section 7 of the report) exercising real `startBeatInBar` values
  spanning 0.011 to 3.531 on the actual build-lane app, this is adequate evidence without needing an
  exhaustive unit sweep — the underlying arithmetic (`fmod`) has no discontinuity the spot-check could
  miss.

## Fence discipline and honesty of the report
- The report explicitly discloses (not hides) that `54e252f`'s own `[bargrid]` ctest depends on
  `cb1c4f0` (which sits OUTSIDE the fence and is offered as separately droppable) and will go RED if
  `cb1c4f0` is dropped without also adjusting that test. This is the correct way to flag a real
  coupling — VERIFIED by hand-tracing (dropping cb1c4f0 leaves `beatOffset_` seeded to 0, and the
  bar-grid test's `moveAt` computation would then reproduce the ~0.84-beat-late stamp, failing the
  `Approx(1.2)`/`Approx(1.04)` checks). No action needed beyond what's already in the report; this is
  a disclosed risk, not a silent gap.
- "No probe change needed": correct per the analysis above. The report additionally lists concrete,
  well-reasoned vacuous/blind-row findings (self-referential oracles in the "stack" timing rows, loose
  slack absorbing part of the defect in the loop-cycle row, a fragile fixed-sleep re-fire row) and
  recommends probe additions without making them — appropriate for a diagnosis-only docs commit that
  does not touch the probe.

## Minor / non-blocking
- `RecorderClock.cpp`'s fix is described as "one line" but the diff also updates 4 lines of comment;
  the code change itself is indeed one line (`beatOffset_ = -static_cast<double>(snap.beatPhase);`).
  No issue, just a note that "one-line" refers to the code, not the diff.
- Section 7's dev-run table documents using a temporary, uncommitted, deleted probe variant (`sleep N`
  before Record) to get non-trivial `startBeatInBar` values. This is disclosed as deliberately not
  committed; fine for a diagnosis session, but if a future lane wants repeatable coverage of
  wrap-around `startBeatInBar` it would need to land as a real probe/test fixture rather than a
  throwaway local script — not blocking here since the report is explicit about this being ad hoc.

## Files reviewed
- /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/wf_b3f13eab-924-1/src/recording/RecorderClock.cpp
- /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/wf_b3f13eab-924-1/src/recording/RoutineEngine.cpp
- /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/wf_b3f13eab-924-1/src/recording/RoutineEngine.h
- /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/wf_b3f13eab-924-1/tests/test_take.cpp
- /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/wf_b3f13eab-924-1/tests/test_routine_engine.cpp
- /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/wf_b3f13eab-924-1/.harmony/.reports/s-rta-0926/routine-grid.md
- /Users/boriskarpman/projects/RealTimeAudio/.harmony/.reports/s-rta-0926/plan-routines-s1-final.md (reference)
- /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/wf_b3f13eab-924-1/src/recording/RoutineSlice.cpp (reference, unchanged)
- /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/wf_b3f13eab-924-1/src/model/Autopilot.cpp (reference, unchanged)

## Caveat
No build/ctest was run by this reviewer (disallowed by the packet: "No build, no app, no debugger").
The 567/567 ctest count and the run-1..7 live probe results in the report are taken on trust
(labeled per the report as VERIFIED/INFERRED by the builder); the RED/GREEN behavior of the new
tests was independently confirmed by hand-tracing the arithmetic against the actual source, not by
executing anything.

METADATA: reviewer=reviewer-agent, builder_packet=routine-grid-r1, date=2026-09-26
