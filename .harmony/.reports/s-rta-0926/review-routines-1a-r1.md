# Reviewer Verdict — routines-1a (round 1)
STATUS: DONE
VERDICT: APPROVE

## Scope
Read-only review of lane 1a (worktree `wf_1817fbdf-a5d-1`, branch `worktree-wf_1817fbdf-a5d-1`)
against `plan-routines-s1-final.md` sections 3.1-3.6, 6.1, 7 (lane 1a), 8 (R1-R14). No build run,
no app launched, no debugger. `git diff --stat` and per-file diffs read directly on disk.

## Files reviewed (14 changed, 1705+/105-)
- `CMakeLists.txt`, `tests/CMakeLists.txt`
- `src/model/Routine.h` (new), `src/model/Composition.h`
- `src/recording/RoutineSlice.h` (new), `src/recording/RoutineSlice.cpp` (new)
- `src/recording/Program.h`, `src/recording/Program.cpp`
- `src/recording/Take.h`, `src/recording/Take.cpp`
- `src/recording/RecorderHost.h`, `src/recording/RecorderHost.cpp`
- `tests/test_routine.cpp` (new, 635 lines), `tests/test_take.cpp`

## (5) Fenced files — VERIFIED empty diff, both by grep-list and by explicit per-file diff:
`src/recording/Player.h`, `Player.cpp`, `src/model/Lane.h`/`src/recording/Lane.h`,
`src/model/ControlPath.h`, `src/recording/PerfState.h`, `PerfStateCapture.cpp`,
`RecorderClock.h/.cpp`, `src/MainComponent.h/.cpp`, `src/api`, `src/osc`, `src/binding`, `src/ui`,
`src/render`, `.harmony/probe-step3.sh`, `CLAUDE.md`, `.harmony/APP-INVENTORY.md`. `git status`
on the worktree is clean (no untracked residue); `git diff --stat -- .harmony` is empty. Matches
plan 7's lane-1a "MUST NOT touch" list exactly.

## (6) RED-first commit order — VERIFIED
`git log --reverse main..HEAD`:
`10c6fc6 test(...)` RED tests first → `64c6a9e` Routine model → `0c134e9` take meta →
`8b10f51` sliceRoutine → `cc8cd88` compileLanes/compileRoutine → `a57f848` CMake wiring.
Matches plan 7 lane-1a step order (tests, then model, then meta, then slicer, then compile,
then CMake).

## (1) `compileLanes` refactor — behaviour-neutral: VERIFIED
`src/recording/Program.cpp` diff: the old inline lane loop (pickAt/convertBeatX lambdas, discrete
push, continuous ContLane build, final sorts) is moved byte-equivalent into a new file-local
`compileLanes(lanes, tempo, comp, clock, range, program, maxAt)`; `compile()` now calls
`buildPreamble(...)` then `compileLanes(take.lanes, take.tempo, comp, clock, range, *program,
maxAt)` — same call sequence, same field writes (`program->report.*`, `program->discrete`,
`program->continuous`), same final `std::sort` calls (both discrete-by-(at,seq) and
continuous-by-x0), same signature (`compile(const Take&, const Composition&, DriveClock,
std::optional<Range>)` unchanged, `Program.h` diff shows only an added forward-decl and
`compileRoutine` declaration).

Stamp-rule change (the one behavioural touch, disclosed and gated): old code did
`if (!exact) ++stampMismatches` unconditionally; new code does
`if (!haveStamps && clock != DriveClock::Beat) ++stampMismatches`. On any clock other than Beat
this is identical to before (byte-for-byte). On Beat, `convertBeatX` returns `beatX` unchanged
(identity), so the reconstructed x is exact and no longer flagged `invalid` — reporting-only, per
the plan's own claim that this is what the gate critic verified. I re-derived it independently by
reading `convertBeatX`'s `case DriveClock::Beat: return beatX;` — confirmed identity.

Regression guard: grepped every pre-existing `compile(` call site
(`test_program_preamble.cpp:77,170,215,252,291,302,342`, `test_take_v1_transport.cpp:66`,
`test_program_stamps.cpp:80,93,101,127`) — every one passes `DriveClock::Wall` or `::Sample`
except `test_program_stamps.cpp:101` (`::Beat`), which uses a gesture *with* stamps
(`makeGesture(/*withStamps*/ true)`), so `haveStamps` is true there and the changed branch is
never exercised — no existing test relies on the old "stampless-on-Beat is invalid" behaviour.
`git diff` on all of `test_program_preamble.cpp`, `test_program_stamps.cpp`,
`test_take_v1_transport.cpp`, `test_recorder_host.cpp`, `test_take.cpp` (apart from the new
SECTION-equivalent case) is empty — the Builder's "existing tests unedited" claim is TRUE.

`compileRoutine` reuses the identical `compileLanes` for the routine's own lanes
(`DriveClock::Beat`, an empty `TempoMap{}`, `std::nullopt` range) and sets `program->length =
routine.lengthBeats` (not `maxAt`) and `program->loop = routine.loop` — matches plan 3.4 exactly.
Its own preamble-compile loop (routine.preamble → resolveKey → discrete `Fired`/continuous
`PreambleSet`, Missing → `preambleUnresolved`, non-Missing-non-exact → `addPreambleRebind`) is a
faithful read of plan 3.4's bullet list; verified against `test_routine.cpp`'s `compileRoutine`
test (case 6): resolvedCount/unresolved/preambleUnresolved/preambleCount/report.invalid/length/
loop/clock all assert the exact numbers the plan's table predicts, and the Wall-vs-Beat regression
clause is present in the same test case (lines ~618-635).

Compile call sites: `compileRoutine(` has zero callers outside `tests/test_routine.cpp` — matches
the builder's claim and lane 1a's "behaviour-neutral, no caller outside tests" framing (F7's
fallback-landing argument holds).

## (2) Routine.h + Composition serialization — VERIFIED
- Additive only: `Composition.h` gains `routines`, `routineBank`, `kRoutineBankSize`,
  `routineLoadNote` (not serialized — confirmed no `toVar` entry for it), plus the slot helpers.
  `initDefault()` now also clears the three new members (a disclosed, sensible deviation — new
  compositions must not inherit stale routines; low risk since routines are otherwise always
  empty today).
- `toVar()`: both `routines` and `routineBank` keys always written (even when empty) — matches
  plan 3.2's "ALWAYS" rule.
- `fromVar()`: clears both vectors first, then reads guarded by `hasProperty` on the array level
  (an old file with neither key hits neither `if`, leaving both vectors empty) — VERIFIED against
  test case 1's "legacy" sub-block, which removes both properties from a saved var and asserts
  both vectors come back empty with no note. A back-compat load of an old composition (no
  `routines`/`routineBank` keys at all) is proven, not just claimed.
- `RoutineSlot` orphan handling: a bank slot whose uuid matches no routine, or whose slot index is
  out of `[0, kRoutineBankSize)`, or whose slot is a duplicate, is dropped and counted into
  `routineLoadNote` (never silent) — VERIFIED against test case 1's orphan sub-block (slot 4 with
  `"no-such-routine"` is dropped, `routineLoadNote` non-empty, slot 3 survives).
- No allocation added to any render/audio/analysis hot path: every touched line lives in
  message-thread model code (`Composition`, `Routine`, `RoutineSlice`, `Program::compile`/
  `compileRoutine`) — none of it is reachable from the audio callback, analysis thread, or GL
  render loop. `Program.h`'s forward-declare of `struct Routine;` (rather than including
  `model/Routine.h`, which pulls `model/Clip.h` → `juce_graphics`) is a real, verified constraint:
  `test_record_panel_model` links `juce_core` only (confirmed in `tests/CMakeLists.txt:1255-1260`,
  comment says so explicitly) and its include chain is `RecordPanelModel.h → RecorderHost.h →
  Program.h`; `RecorderHost.h` includes `Program.h` but not `Composition.h`, so the forward-decl
  is what keeps that link juce_core-only. This is a legitimate, correctly-diagnosed constraint,
  not a workaround for a self-inflicted problem.

## (3) `sliceRoutine` semantics vs plan 3.3 — VERIFIED against both code and tests
Read `RoutineSlice.cpp` end-to-end and cross-checked against `test_routine.cpp` cases 2-4 (the
richest test in the suite, ~530 lines total, non-vacuous — it pins exact synthesized numeric
values, e.g. `eval(4) = 0.35`, matching the plan's own worked example in test-table row 2):
- Refusals: `toBeat <= fromBeat`, empty tempo map, and an anchor with `bpm <= 0` bracketing or
  inside `[fromBeat, toBeat]` are all refused with a non-empty `error` and no routine — VERIFIED
  in code (`checkMetered`) and in test case 4 (all four refusal sub-cases, plus the tempo == 0
  bracketing-anchor case).
- Lane filter: `Scope::Macro`/`Scope::Routine` and `Comp` `tempo`/`audio`/`activeDeck`/`quantize`
  lanes are dropped and named in `droppedLanes` — VERIFIED (`isTransportLane`, test case 4).
- `deckRelative = true` is set ONLY for Layer/Clip scope keys, never for Comp keys (which carry no
  deck) — VERIFIED in code and pinned by test case 3's explicit comment + assertion
  (`r.lanes.count(compOpacity) == 1` with the comp key untouched).
- Discrete: keeps `[fromBeat, toBeat)`, rebases `beat -= fromBeat`, zeroes wall/sample stamps,
  keeps `seq` — VERIFIED (test case 2's activeClip lane assertions on `s.t`, `s.sample`, `s.seq`).
- Preamble fallback chain (last point before cut → checkpoint 0 → default) is implemented exactly
  as three ordered attempts (`Source::Lanes` → `checkpointDiscrete`/`checkpointContinuous` →
  `libraryDefault`/scalar default), with `Source::Unknown` counted and no entry emitted — VERIFIED
  against test case 3's three-way fallback (checkpoint 0.4, library default, comp-scalar
  unknown).
- Continuous gesture straddle synthesis (`cutGesture`): synthesizes a first breakpoint at
  `fromBeat` via `curve.eval(fromBeat)` when the gesture starts before the cut and has no existing
  point exactly at `fromBeat`, and a last breakpoint at `toBeat` symmetrically; drops points
  outside the window; rebases `x -= fromBeat`; clears stamps; keeps `grip`. Cross-checked line by
  line against plan 3.3 step 5 and against test case 2's exact numbers (x=0/y=0.35 begin,
  x=3/y=0.8 end for the straddling-start gesture; x=6/y=0.0 through x=8/y=0.5 for the
  straddling-end one) — the numbers match what the code computes by hand-tracing `cutGesture`.
- `covered` (no preamble entry when a gesture's synthesized begin already covers `fromBeat`) is
  correctly gated on `x0 <= fromBeat` for a gesture that already survived the `x1 > fromBeat`
  filter — VERIFIED in code and by test case 2's "layer0Opacity == 0" assertion.
- Whole-bar rounding: `ceil(len/4 - 1e-9) * 4` when `wholeBars` (default true), exact length
  otherwise — VERIFIED (test case 4's 6→8 and 6→6 sub-cases).
- Unknowns never silent: every fallback path increments one of `preambleFromLanes/
  FromCheckpoint/FromDefaults/Unknown`, all four counted and asserted in tests 2 and 3.
- Preamble emission order — layer flags, then activeClip, then clip play/pause, continuous last —
  is enforced by the `flags/activeClips/playing/continuous` four-vector concatenation at the end
  of `sliceRoutine`, VERIFIED in code and pinned by test case 2's `iActive < iPlaying` assertion.
  R5's "restore the STATE the point left" deviation (rather than replaying the point's raw
  discrete action for `playing`) is implemented via `setPlayState` and is the more correct
  behaviour than a literal read of D9's vocabulary (a raw `"reverse"`/`"stop"` action replayed as
  a restore would misbehave) — this is a defensible, disclosed deviation, not a silent one.
- `takeBeatOfBar`: `(bar-1)*4 + fmod(4 - start, 4)`, treating unknown (`-1`) as `0` — VERIFIED
  against test case 5 and matches plan 3.6's formula exactly.

## (4) Take meta `startBeatInBar` — additive, back-compat — VERIFIED
`Meta::startBeatInBar` defaults `-1.0`; `toVar()` only writes the key when `>= 0.0`; `fromVar()`
reads it guarded by `hasProperty`. An old take's JSON (no key) round-trips to `-1.0` unchanged —
VERIFIED directly in the new `test_take.cpp` SECTION-equivalent (`[take][meta]` case, +21 lines,
zero lines removed from the file — confirmed by `git diff` showing only additions). `RecorderHost`
writes `armedStartBeatInBar_` from `ArmOptions::startBeatInBar` at arm, and propagates it into
`meta.startBeatInBar` at all three save sites: the provisional save in `arm()`, the periodic save
in `tick()`, and the final save in `disarm()` — all three confirmed present in the diff. The new
`loadedTake()` accessor is a pure read (`RECORDER_HOST_ASSERT_MESSAGE_THREAD()` guarded, returns
`loadedTake_ ? &*loadedTake_ : nullptr`), no side effects. `perfRecord` is explicitly NOT touched
(deferred to lane 1b) — so every take recorded before 1b lands still carries `-1` (unknown),
exactly matching today's behaviour; this is stated as a deliberate non-change in the builder
report and verified by the diff (no `MainComponent.cpp` changes at all).

## (5) `Player.*` diff and fenced files — EMPTY, confirmed twice (grep list and explicit
per-file `git diff`, see above). `git diff --stat` for the whole lane shows exactly the 14 files
the plan's lane-1a step list names (Routine.h, Composition.h, RoutineSlice.{h,cpp}, Program.{h,cpp},
Take.{h,cpp}, RecorderHost.{h,cpp}, root+test CMakeLists, test_routine.cpp, test_take.cpp) —
no drift, no stray touch.

## (6) Tests honest, RED-first — VERIFIED (see commit-order section above). Beyond commit order:
none of the six `test_routine.cpp` cases nor the `test_take.cpp` addition is vacuous — each pins
concrete numeric/structural expectations (exact breakpoint coordinates, preamble counts, ordering
indices, dropped-lane names, error non-emptiness) rather than just "does not throw" or "compiles".
I independently hand-traced the slice/compile logic against three of the richer assertions (case
2's straddle synthesis, case 3's three-way fallback, case 6's compileRoutine report counts) and
the code's actual behaviour matches the asserted numbers — this is not merely "the test was
written to match the code," the numbers are independently derivable from the plan's own worked
examples in table 6.1.

## (7) Plan deviations — all disclosed, all justified, none silent
Cross-checked every item in the Builder's `deviations` list against the diff:
- Case 13 as a new `TEST_CASE` (not a `SECTION`) in `test_take.cpp` — VERIFIED zero removed lines
  in that file; the stated reason (no existing case may be edited, and ctest-count math needs a
  discoverable case) is sound.
- Step-1 RED commit omits `RoutineSlice.cpp` from the CMake block (added in step 4) — VERIFIED via
  commit-by-commit `tests/CMakeLists.txt` presence; the stated reason (a missing header only
  breaks the new target, a missing source file breaks the whole tree's CMake configure) is a real
  CMake constraint, correctly reasoned around.
- `Program.h` forward-declares `Routine` rather than including `Routine.h` — VERIFIED as a real,
  correctly-diagnosed link constraint (see section 2 above), not an invented one.
- `SliceRequest::takeFolder` addition, `Routine::name` left as `req.name` verbatim, R5's
  `setPlayState` restore-the-state-not-the-action, the Comp-vs-Layer/Clip `deckRelative` split,
  `Composition::initDefault()` clearing routines, `PreambleEntry::toVar` always writing `v` for
  discrete entries — all VERIFIED present in the diff exactly as described, each independently
  defensible.
- `compileLanes` reusing the final sort logic so `compileRoutine` gets it for free — VERIFIED
  (`compileLanes` contains both `std::sort` calls; `compileRoutine` calls it once).

## Risks re-checked against the diff (R6 is the one that matters for this lane)
R6 (the shared `compileLanes` refactor could break every replay): the guard the plan demanded —
"all existing program/host tests green with NO edits" — is verifiable from the diff alone (empty
diffs on every existing test file that calls `compile(`), and the stamp-rule change is provably
reporting-only on the Beat clock (identity `convertBeatX`) and provably a no-op on every other
clock (unchanged conditional). I could not run `probe-step3.sh` (read-only, no build/launch
per task instructions) — that live check is Harmony's gate-1a responsibility, not mine; the
static evidence available on disk fully supports the "no drift" claim for this lane.

## Assessment
This is a clean, disciplined, honest lane-1a build: additive-only model/serialization changes,
a provably behaviour-neutral refactor with the one intentional behavioural change scoped exactly
to what the plan authorized and gated by a regression test, comprehensive non-vacuous tests that
pin the plan's own worked numbers, a fully respected fence list, RED-first commit discipline, and
a deviations list that is honest and each item independently checks out against the diff. No
blocking correctness issues found in slice semantics, serialization back-compat, take-meta
additivity, or the compile refactor.

## Non-blocking nits
- `cutGesture`'s straddle-detection (`bp.x == fromBeat`) uses exact floating equality to detect an
  existing boundary point; this mirrors existing beat-stamped-point handling elsewhere in the
  codebase and is not a new risk introduced by this change, but a future lane touching this code
  should be aware synthesized vs. recorded boundary points are compared by exact `==`.
- `Composition::initDefault()` clearing `routines`/`routineBank`/`routineLoadNote` is not spelled
  out anywhere in plan sections 3.1-3.6 or 7; it is a sensible, low-risk, disclosed addition (no
  routines exist yet in any live composition), but is technically outside the lane's literal scope
  fence ("adds no surface, no UI, no engine"). Worth a one-line mention in the lane-1a merge
  commit message if not already there — non-blocking.

## Reviewer scope note
Per the task instructions I did not build, launch the app, or run ctest/probe-step3.sh myself;
all "VERIFIED" claims above are from reading source and test files on disk and hand-tracing logic
against the plan's own worked numbers, not from execution. The live ctest/probe-step3.sh/
probe-mastersignal.sh re-run belongs to Harmony's Gate 1a, as the plan specifies.
