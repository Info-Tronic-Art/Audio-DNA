STATUS: DONE
RESULT: Executed plan-sampleat's adopted fix (A1-A8) for `TempoMap::sampleAt`, FILED by
tempo.md section 8: `compile()` now threads the resolved asset's sample rate (`nominalRate`)
through to `TempoMap::sampleAt`, which uses a bracketing segment's own delivered-sample slope
only when that segment spans >= 1 s of `t`, and `nominalRate` otherwise (a single-anchor map has
no segment at all, so it always falls back to `nominalRate` -- never rate 0). RED captured on the
worktree's own pre-fix logic (temporary in-place mutation, restored before commit, see METHOD);
GREEN after the fix. Full serial ctest: 848/848 passed (846 baseline + 2 new TEST_CASEs), 19.16 s.
FACTS (disk-cited, this worktree unless noted):
- src/recording/TempoMap.h:47-55 -- `sampleAt(double t, double nominalRate) const`, doc comment
  states the >= 1 s rule and the nominalRate fallback.
- src/recording/TempoMap.cpp:81-109 -- implementation: `double rate = nominalRate;` initial value,
  overwritten by a neighbour segment's own slope only `if (dt >= 1.0)` (both branches).
- src/recording/Program.h:128-136 -- `compile(..., std::optional<Range> range = {}, double
  nominalRate = 0.0)`, doc comment names the new param.
- src/recording/Program.cpp:424-426 (`compileLanes` signature), :452 (`convertBeatX`'s
  `DriveClock::Sample` case calls `tempo.sampleAt(tempo.tAt(beatX), nominalRate)`), :545-559
  (`compile()` threads `nominalRate` into `compileLanes`), :602 (`compileRoutine` passes `0.0` --
  its `TempoMap{}` is never read on the Beat clock, comment unchanged and still true).
- src/recording/RecorderHost.cpp:784-787 -- `play()` calls `compile(*loadedTake_, comp, clock, {},
  loadedAudio_.asset.rate)`; the early return at :773-777 (`audio not resolved`) means
  `loadedAudio_.asset.rate` is always the real resolved rate whenever `clock == DriveClock::Sample`.
- tests/test_program_stamps.cpp:143-202 -- two new TEST_CASEs (A2 through `compile()`; A3+A4
  through `TempoMap::sampleAt` directly); tests/test_program_stamps.cpp:31-38 -- A5, the stale
  "rate 0" comment corrected.
- tests/test_take.cpp:723-727 -- the one other direct `sampleAt` caller in the tree, updated to
  the 2-arg form with a comment on why nominalRate (48000.0) is inert there (every periodic
  segment in that test spans >= 1 s).
- A6 reachability (verified by reading, this session): `grep -rn "\.sampleAt(" src/ tests/` finds
  exactly one production caller -- Program.cpp:452 (`convertBeatX`'s Sample-clock case, reached
  only when `!haveStamps`, Program.cpp ~line 512). PerformanceRecorder.cpp:100-140
  (`finishGesture`/`set`) pushes a `Stamp` for every `curve.pts` push -- `stamps.size() ==
  curve.pts.size()` always for a live-recorded gesture. Take.cpp's v1 importer (:396-436) only
  ever creates `Lane::Kind::Discrete` lanes (never Continuous), so it can't reach the Continuous
  fallback path at all. `compileRoutine` is hard-coded to `DriveClock::Beat` (Program.cpp:601),
  where `convertBeatX`'s Beat case is the identity and never calls `sampleAt`. No other production
  call site exists. Confirms tempo.md's claim; A6 holds, no BLOCKED report needed.
- docs/claude/recording.md (A8): grepped for "sampleAt"/"TempoMap"/"Sample-clock
  fallback"/"stampless" -- no hits. Nothing to correct; CLAUDE.md untouched (per A8 and rig rules).
METHOD: Read plan-sampleat.md (main checkout, read-only) and its two sources (plan-tempo.md
section 8, tempo.md section "found_not_fixed" item 1 + PACKET QUALITY), plus tempo0-diag.md line
17 for the "~85,000 samples/s" characterization used in the A3 test. Implemented the signature +
threshold change directly (not a separate mechanical-then-behavioral commit). For RED evidence
(rig rule "RED FIRST ... for every new probe row/test"), temporarily mutated
TempoMap.cpp's rate-selection back to the exact pre-fix form (`double rate = 0.0;` / `if (dt >
1e-9)` in both branches, new signature kept so it still compiled) with a `TEMP MUTATION` comment,
rebuilt `test_program_stamps` only, ran the 2 new cases -> both FAILED, output captured verbatim
below. Restored the fix (re-edited back to the `nominalRate` / `dt >= 1.0` form, confirmed by
`grep` that no mutation text remained), waited past the mtime-resolution second boundary (notebook
gotcha: macOS make's 1 s mtime resolution can leave a same-second restore looking "up to date"),
touched the file, rebuilt, reran -> both PASSED (verbatim below). Built every target that compiles
`Program.cpp`/`TempoMap.cpp`/`RecorderHost.cpp` individually first (test_program_stamps, test_take,
test_take_v1_transport, test_routine, test_program_preamble, test_recorder_host, test_tempo_start),
all green, then did a full default-target build (`cmake --build build-lane -j3`, exit 0, ~24 min
wall under other lanes' concurrent load) and a full serial `ctest -j1` (848/848, 19.16 s). No live
app was launched (A7: no observable live effect since A6 holds); this is a ctest-only SMALL lane.
CONFIDENCE+VERIFY: High. The fix is a pure, deterministic unit-level change (no threading, no live
app surface) with a RED-then-GREEN cycle captured on this exact worktree, plus a full 848/848
serial ctest and a full app+test default build both exiting 0. VERIFY: `ctest --test-dir
<wt>/build-lane -j1 -R "sampleAt|nominalRate|single-anchor"` (2/2), or the full `ctest -j1`.
UNKNOWNS-NOT-DONE: None against the adopted scope (A1-A8 all addressed). Not attempted (correctly,
per A7): no live probe -- A6 proves nothing observable changes live, since no take the app writes
reaches this path.
NUANCE: The A3 test's anchors are a representative 12 ms start/lock pair (matching the diagnosis's
*characterization*, "~85,000 samples/s"), not the diagnosis's literal captured sample values (the
diagnosis names a "lock" *time* per run but not its exact sample count, only 4 "start" sample
values from different, unrelated runs) -- constructing a synthetic pair with the same order-of-
magnitude old-slope bug was the accurate-but-honest option, and the test comment says so.
HANDOFF-NEEDS: none.

## SUMMARY
Executed the FILED "TempoMap::sampleAt" fix exactly as adopted (A1-A8), a small, self-contained,
deterministic ctest-only change: `Program::compile`'s Sample-clock fallback (the only path that
ever calls `TempoMap::sampleAt`, reached only by a stampless gesture -- unreachable by anything the
app itself writes, per A6) now extrapolates at the resolved audio asset's own sample rate whenever
the bracketing tempo-map segment is too short (<1 s) or absent (a single-anchor map) to trust its
own slope, instead of silently using rate 0 or an implausible short-segment slope.

## FILES CHANGED
- `src/recording/TempoMap.h` -- `sampleAt` signature + doc comment (nominalRate, the >= 1 s rule).
- `src/recording/TempoMap.cpp` -- `sampleAt` body: `rate` initializes to `nominalRate`; both
  neighbour-segment branches only overwrite it `if (dt >= 1.0)` (was `dt > 1e-9`, unconditional).
- `src/recording/Program.h` -- `compile()` gains `double nominalRate = 0.0` (5th param, defaulted
  so every existing Wall/Beat-clock caller is unaffected); doc comment added.
- `src/recording/Program.cpp` -- `compileLanes` gains the `nominalRate` param; `convertBeatX`'s
  Sample case passes it to `sampleAt`; `compile()` threads it through; `compileRoutine` passes
  `0.0` (dead, its TempoMap is always empty and its clock is always Beat).
- `src/recording/RecorderHost.cpp` -- `play()`'s `compile()` call now passes
  `loadedAudio_.asset.rate` as `nominalRate` (with an explanatory comment); Wall-clock plays are
  unaffected (the param is only read on the Sample clock).
- `tests/test_program_stamps.cpp` -- corrected the stale "sampleAt(t) == 0 for a single anchor"
  comment (A5); added 2 new TEST_CASEs for A2 (through `compile()`) and A3+A4 (direct
  `TempoMap::sampleAt` calls).
- `tests/test_take.cpp` -- updated the one other direct `sampleAt` call site to the 2-arg form,
  with a comment on why the passed nominalRate is inert there.

## TESTS
- New: `tests/test_program_stamps.cpp` two TEST_CASEs --
  - "Program::compile's Sample-clock fallback uses nominalRate for a single-anchor tempo map" (A2):
    a `{t 0, sample 150016, bpm 120}` one-anchor map + a stampless gesture at beats {0,4,8}
    compiled on `DriveClock::Sample` with `nominalRate=48000` gives x = {150016, 246016, 342016}.
  - "TempoMap::sampleAt uses nominalRate under 1 s, its own slope at 1 s or more" (A3+A4): a 12 ms
    two-anchor "start"/"lock" pair extrapolates at nominalRate past 1 s of query time (198461, not
    the old-slope 235099); a 2 s two-anchor segment still uses its own slope (44100), never
    nominalRate (48000).
- RED (verbatim, temporary in-place mutation of TempoMap.cpp back to the pre-fix rate-selection,
  new signature retained so it still compiled -- restored before any commit, confirmed by grep for
  leftover mutation text):
  ```
  tests/test_program_stamps.cpp:172: FAILED:
    REQUIRE( cg.curve.pts[1].x == Approx(246016.0).margin(1e-9) )
  with expansion:
    150016.0 == Approx( 246016.0 )

  tests/test_program_stamps.cpp:189: FAILED:
    REQUIRE( static_cast<double>(tempo.sampleAt(1.0, 48000.0)) == Approx(151037.0 + 0.988 * 48000.0).margin(1.0) )
  with expansion:
    235099.0 == Approx( 198461.0 )

  test cases: 2 | 2 failed
  assertions: 4 | 2 passed | 2 failed
  ```
- GREEN (verbatim, after restoring the fix):
  ```
  All tests passed (6 assertions in 2 test cases)
  ```
- Individually green after the fix: test_program_stamps, test_take (643/643), test_take_v1_transport
  (40/40), test_routine (201/201), test_program_preamble (169/169), test_recorder_host (1806/1806).
- Full default build (`cmake --build build-lane -j3`, includes the AudioDNA app target and all 90
  ctest targets): exit code 0, no warnings-as-errors tripped (RecorderHost.cpp compiles under the
  app's stricter `-Wall -Wextra ... -Wconversion` flag set too).
- Full serial ctest: `ctest --test-dir build-lane -j1` -- **848/848 passed, 0 failed, 19.16 s**
  (846 baseline count from HANDOFF.md + 2 new TEST_CASEs).
- No live probe (per A7 / A6's reachability proof).

## ISSUES
None found beyond the FILED item itself.

## SKILL_PROPOSALS
None -- this followed the existing test-authoring craft (RED-then-GREEN via a documented,
restored, verified-clean temporary mutation) already covered by this agent's standing guidance.

## RISKS
None identified beyond what the plan/adoption already named (A1-A8 cover the design questions).
The change is additive/defaulted at every existing call site (`nominalRate = 0.0` default on
`compile()`, unused on Wall/Beat clocks and by every stamped gesture), so it cannot change behavior
for any take the app currently writes or replays (A6).

## METRICS
- Files changed: 7 (5 source, 2 test).
- Lines: +101 / -17 (`git diff --stat`).
- New TEST_CASEs: 2 (6 new assertions).
- ctest: 848/848 passed (was 846), 19.16 s real.
- Full build wall time: ~24 min under concurrent load from sibling lanes (idle, media) sharing the
  10-core machine (load average ~6.8 during the build).

## KNOWLEDGE CONTEXT
- Tools used: grep only (no KNOWLEDGE_TOOLS block in the work packet; no graphify graph consulted).
- Impact authority: grep (not authoritative) -- treated conservatively; A6's "only caller" claim
  was independently re-verified by grep across `src/` and `tests/` in this worktree (not inherited
  from tempo.md), and by reading every intermediate caller (PerformanceRecorder.cpp,
  Take.cpp's v1 importer, `compileRoutine`) to confirm none of them can reach the fallback path.
- God nodes in scope: n/a (no graph).
- Risk level: NORMAL (small, additive, unit-tested, no live-reachable path per A6).
- Dependencies discovered: none beyond what the plan named.
- Queries made: 0 (grep-only).

## PACKET QUALITY
- Clarity: CLEAR. The adoption's A1-A8 fully specified the fix, the RED cases (with a stated
  today-value for A2), the >= 1 s rule, and the doc/CLAUDE.md scope.
- Missing context: the diagnosis's exact "lock" sample value for the 12 ms pair (only its *time*
  and a different run's *start* sample are on record) -- built a representative fixture instead
  (documented in NUANCE and in the test comment).
- Unused context: none -- every adoption bullet (A1-A8) was actionable and used.
- Self-brief files: plan-sampleat.md (main checkout, read-only per rig rules -- useful, read in
  full); plan-tempo.md section 8 (useful, the FILED item's original text); tempo.md's
  found_not_fixed item 1 + PACKET QUALITY (useful); tempo0-diag.md (useful for the A3
  characterization, though not for literal fixture values -- see NUANCE).
- Screen safety / live-app: not applicable -- no live app was launched (ctest-only SMALL lane, A7).

## STATUS
DONE

## NEXT ACTION
None required. Ready for Harmony's independent Reviewer (source) and behavioral gate; no live
surface exists to gate (A6/A7).

INBOX-RECHECK: 0 addenda folded | none
