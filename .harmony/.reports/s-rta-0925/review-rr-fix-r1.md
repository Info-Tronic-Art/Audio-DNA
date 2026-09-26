# Reviewer Verdict — s-rta-0925 rr-fix
STATUS: DONE
VERDICT: APPROVE

## Scope
Independent review (did not build) of `lane/0925-rr-fix` (3 commits: RED test, fix, diagnosis doc)
against main, plus `.harmony/.reports/s-rta-0925/rr-fix-diagnosis.md`. Diff touches exactly:
`src/recording/Program.cpp` (+8/-9 net in `buildPreamble()`), `tests/test_program_preamble.cpp`
(updated test 1, added test 6), and the diagnosis doc itself. No probe file changed
(`git diff main...lane/0925-rr-fix -- .harmony/probe-step3.sh` is empty — VERIFIED).

## Diagnosis fidelity — all claims independently re-verified on disk

1. **Checkpoint facts** (`~/Documents/Audio-DNA/Takes/step3gate1.adna-take/take.json`): read
   directly — `checkpoint0.decks[0].layers[0] = {activeClipColumn:-1, opacity:1.0}`, and lane 0's
   first point is `seq=2, t=1.9487s, v=0`. Matches the report's section 1/3 exactly (VERIFIED).
2. **Pre-fix code**: read `src/recording/Program.cpp` on `main` — the `buildPreamble()` opacity
   block has the `if (std::abs(norm - def.defaultNorm) > 1e-4f)` gate exactly as quoted, immediately
   below five unconditional `emitFlag(...)` calls with no such gate. `layerScalarDefs()[Opacity]`
   uses `ScalarMath::identity` with `defaultNorm = 1.0f` (`src/connect/ScalarParams.h`) — confirmed
   checkpoint opacity 1.0 always fails the `>1e-4` test, so the continuous entry is never pushed
   (VERIFIED).
3. **activeClipColumn discrete path has no such gate**: confirmed in `src/MainComponent.cpp` around
   line 1938-1942 — `if (f.p.v < 0) applyClearActiveClip(...)` unconditionally, no default check
   (VERIFIED, matches report's citation).
4. **Probe timing mechanics**: read `.harmony/probe-step3.sh` — `check_snapback_restored` loops 15x
   `sleep 0.1` (nominal 1.5s budget) plus a `comp_col_op` curl+python round-trip per iteration
   (real cost per iteration > 100ms); `START_T` is set once before calling
   `check_snapback_restored` and reused unchanged by the next long poll loop, exactly as the report
   describes (VERIFIED, code read). The specific "~3-4s of real wall time" figure is attributed to
   a live-gate log not present on disk in this worktree — I could not re-verify that exact number,
   but it is not load-bearing for the fix's correctness, only for explaining the FAIL row's
   observed `col=0` value (INFERRED/unverified, flagged, non-blocking).
5. **Fix**: unconditional emission of the layer-opacity continuous entry, matching the five
   discrete flags immediately above it in emission order — minimal, surgical, single-file change
   (VERIFIED via diff read).

## Independent RED/GREEN reproduction (not taken on the report's word)

Built `test_program_preamble` from a `$TMPDIR` copy of the lane (via `git archive`, no shared
`.git`/working-tree mutation), configured against the main checkout's already-populated
`build/_deps/{juce,httplib,catch2,syphon}-src` (same recipe the diagnosis used, independently
re-run):

- **GREEN** (lane code as-is): `All tests passed (169 assertions in 6 test cases)` — matches the
  report's cited number exactly.
- **RED** (swapped `Program.cpp` for main's pre-fix version via `git show main:... >`, nothing else
  changed, same test binary rebuilt): `6 test cases | 4 passed | 2 failed`, `145 assertions | 143
  passed | 2 failed` — the two named failures are exactly:
  - `test_program_preamble.cpp:125: REQUIRE( p->preambleContinuous.size() == 5 )` → `3 == 5`
  - `test_program_preamble.cpp:356: CHECK( foundOpacityRestore )` → `false`

  This matches the diagnosis's RED numbers exactly and confirms the new test 6 (and the updated
  test 1) genuinely gate the described bug — not a tautology, not preconditioned on a `.new`/staged
  artifact, and it fails cleanly on the pre-fix code while passing on the fix.

## Test-6 review (dimensions 1-6)

- Readability/patterns: matches existing Catch2 style in the file, comment explains the live-gate
  provenance and why `Program::compile` intentionally has no runtime-perturbation awareness.
- Complexity: minimal — asserts exactly the two invariants the bug broke (continuous opacity
  restore present at default value; discrete activeClip clear still present, isolating the bug to
  the continuous path only). No over-engineering.
- DRY: reuses `makeComposition()`/`capturePerfState()` helpers already in the file.
- Fix itself: removes a bespoke asymmetric optimization that had no counterpart on the five
  boolean flags directly above it — restores symmetry, does not introduce a new one.

## SLIM

No new files, no dead code introduced. The removed `if` branch deletes an incorrect optimization;
`def` stays referenced (still used for `toNorm`). Nothing to flag under EXCESS_* — this is a
straightforward bug-fix diff.

## Verdict rationale

Diagnosis is accurate on every checkable claim (checkpoint values, pre-fix code text, gate
asymmetry, discrete-vs-continuous distinction, probe-unchanged). The one unverifiable claim (exact
~3-4s wall-clock figure from a live-gate log not present in this worktree) is explanatory context
only and does not affect whether the fix is correct — the RED/GREEN reproduction independently
confirms the code bug and the fix. No blocking issues.

## Files reviewed
- /Users/boriskarpman/projects/RealTimeAudio/src/recording/Program.cpp
- /Users/boriskarpman/projects/RealTimeAudio/tests/test_program_preamble.cpp
- /Users/boriskarpman/projects/RealTimeAudio/.harmony/.reports/s-rta-0925/rr-fix-diagnosis.md
- /Users/boriskarpman/projects/RealTimeAudio/.harmony/probe-step3.sh (confirmed unchanged)
- /Users/boriskarpman/projects/RealTimeAudio/src/connect/ScalarParams.h (defaultNorm confirmation)
- /Users/boriskarpman/projects/RealTimeAudio/src/MainComponent.cpp (activeClip discrete-clear path)

METADATA: reviewer=reviewer-agent, builder_packet=rr-fix, date=2026-09-25
