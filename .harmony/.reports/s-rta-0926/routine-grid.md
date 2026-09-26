# s-rta-0926 lane routine-grid -- why every grid row fired ~0.4 s late at startBeatInBar 3.84

STATUS: DONE_WITH_CONCERNS (fix touches one file OUTSIDE the packet fence -- see "Fence")
Branch: worktree-wf_b3f13eab-924-1 (ff'd to main ebe0d4e first). Builder: Claude Opus 5.5. Harmony gates.

## Verdict (sentence one)

**The APP was wrong, but not in the routine code: the take's beat stamps were wrong.** `RecorderClock` (the
take clock) started its beat count at the tracker's last beat line instead of at Record, so every
`beat` stamp in a take read `frac(startBeatInBar)` beats late (0.84 beats in the gate take). The
routine engine then played each move exactly where the take said it was -- 0.42 s late at 120 BPM.
The probe's fixed expectations (+0.6 / +2.6 / +4.6 / +6.6) are CORRECT under the plan for its
`fromBeat 0` cut and needed no change.

## 1. The gate take, from disk (VERIFIED -- `~/Documents/Audio-DNA/Takes/probe-routines-20260926-161617.adna-take/take.json`)

`meta.startBeatInBar 3.840`; `tempoMap = [{t 0, beat 0, bpm 120, "start"}]` (one anchor, so the
take's own map says beat = 2 t). The points say otherwise:

| move | recorded `t` (s) | beats since Record (2t, the map) | stamped `beat` | stamp - map | place in its bar at Record ((3.84 + 2t) mod 4) |
|---|---|---|---|---|---|
| L0 opacity 0.5 | 0.601 | 1.201 | 2.056 | +0.855 | 1.041 |
| L0 -> column 1 (+ auto-play) | 2.610 | 5.219 | 6.067 | +0.848 | 1.059 |
| C1 Brightness 0.9 | 4.601 | 9.202 | 10.056 | +0.854 | 1.042 |
| L0 opacity 0.9 | 6.602 | 13.203 | 14.045 | +0.842 | 1.043 |

`durationBeats 26.845` vs `duration 12.994 s` (2t = 25.99): the same +0.85. The take violates s167
D1's own loader lint (`|beat - map.beatAt(t)| > 0.05`). 0.85 = the tracker's beatPhase at the first
clock tick after Record (startBeatInBar was sampled ~1 tick earlier: 0.84).

## 2. Root cause (VERIFIED by source + a RED ctest)

`src/recording/RecorderClock.cpp` first tick: `lastPhase_ = snap.beatPhase; wholeBeats_ = 0;
beatOffset_ = 0; current_.beat = 0`. Every later tick computes `beat = wholeBeats_ + lastPhase_ +
beatOffset_`, so on the SECOND tick beat jumps from 0 to `phase0 + dt`. Every existing clock test
seeded at `beatPhase 0.0f` (`tests/test_take.cpp` [recorderclock] cases), which hid it. With
startBeatInBar 0.07 (the 1b / tempomap dev runs) the error was 0.07 beats = 35 ms: invisible.

Fix: seed `beatOffset_ = -snap.beatPhase` on the first tick (one line). Beat is now 0 at Record and
counts beats since Record, agreeing with the "start" anchor. The unmetered-first path is covered too
(the re-lock branch's `target` is then 0).

## 3. What the routine chose and played in the gate run (VERIFIED -- `gate-final/rt/run-20260926-161617-74007/grid.json`)

- Slice: `fromBeat 0, toBeat 16` (the probe's save) -> routine beats = stamped take beats
  2.056 / 6.067 / 10.056 / 14.045; length 16 beats. `sliceRoutine` did exactly what plan 3.3 says.
- Engine: routine started on bar 13 (the next bar); `status.position` is linear in wall time
  (wall-fit of the start: spread 0.036 s -- the beat clock ran at wall speed). Each move was first
  seen between published positions 2.048-2.133, 6.016-6.059, 10.005-10.069, 14.016-14.080 -- i.e. at
  exactly the stamped beats. `RoutineEngine`/`compileRoutine` placed them correctly; the input was off.
- Observed, relative to the probe's T1: +1.00 / +2.98 / +4.97 / +6.99 s (relative to the fitted
  start: +1.06 / +3.05 / +5.04 / +7.05; T1 is ~0.05 s after the true start by polling).

## 4. Correct expected times under the plan (sections 3.3 / 3.5 / 3.6, D4)

Plan 3.3 rebases so take beat `fromBeat` = routine beat 0; 3.5: `pos = clock.beat - startBeat`,
routine beat 0 = the bar the routine starts on. For the probe's `fromBeat 0` cut, take beat 0 IS
the Record instant, so each move plays `(beats since Record) / 2` s after the routine's bar:
**+0.60 / +2.61 / +4.60 / +6.60 s**. Observed +1.00 / +2.98 / +4.97 / +6.99: **do NOT match** (all
+0.42 s = 0.84 beat late, one constant offset = frac(startBeatInBar)).

On "same position within the bar": the plan does NOT promise that for an arbitrary `fromBeat`. It
holds when the cut is on bar lines (`fromBar`/`toBar` -> `takeBeatOfBar`, 3.6). With `fromBeat 0`
and Record at 3.84, the recorded bar position 1.04 replays at 1.20 into the routine's bar -- a
0.16-beat shift inherent in cutting at a non-bar line and starting on a bar (by design). With the
bug, a bar-line cut (bar 1 at take beat 0.16) would have placed the 1.04 move at 1.88 -- so the bug
also broke the "bars 33 to 40" promise. The new ctest pins both readings (section 6).

## 5. Second defect of the same class, in fence (RoutineEngine)

`RoutineEngine::tick` detected a Beat-quantize edge as `floor(clock_.beat)` changing. `clock_` is
a RecorderClock ticked from app start; the app starts unmetered, so at lock the clock absorbs the
tracker's phase into its offset and its whole beats land at `phase_at_lock`, not on the tracker's
beats (VERIFIED by a RED section; pre-existing, independent of the fix above, and the fix above
would also have moved it for an engine seeded mid-beat). Now the Beat edge is the tracker's
`beatPhase` wrap (`phase < last - 0.5`) -- the same rule a Beat-quantized clip uses
(`src/model/Autopilot.cpp:44`), so a routine and a clip set to Beat start on the same beat. Bar /
2 Bar / 4 Bar were unaffected (they use `totalBarCount` / `barCount`).

## 6. Tests (RED first, then GREEN)

| test | file | RED on ebbff22 code | after |
|---|---|---|---|
| `RecorderClock: beat is 0 at Record and counts from there even when Record falls mid-beat` (seeds 0.84 / 0.25 / 0.99; 0.6 s -> 1.2 beats; D1 lint) | `tests/test_take.cpp` | beat 2.04 (seed 0.84), 1.45 (0.25); lint off by 0.84 | pass |
| `Routine: a move keeps its place in the bar when Record fell late in a bar` (real RecorderClock seeded at 0.84, startBeatInBar 3.84, move at +0.6 s; `fromBeat 0` cut -> 1.2; bar-1..4 cut -> 1.04; engine fires it on the tick at downbeat + 1.0625, not at + 1.0) | `tests/test_routine_engine.cpp` [bargrid] | 3 failures (2.056 / 1.88 / not fired) | pass |
| section `Beat starts on the tracker's beat when the tracker locked mid-beat` | `tests/test_routine_engine.cpp` [quantize] | started at 2.8125 (pending CHECK failed) | pass |

Serial `ctest` in `build-lane`: **567/567 passed** (565 at ebbff22 + 2 new test cases; the section
lives in an existing case).

## 7. Dev runs of probe-routines on the build-lane app (ROUTINES_APP = build-lane bundle)

Runs 4-7 used a TEMPORARY copy of the probe with `sleep N` before Record (deleted, never committed)
because the unmodified probe's fixed schedule after `set_bpm` makes Record land at a near-constant
bar position (0.01-0.35 in runs 1-3). `bpm-N.json` = a read-only /api/bpm poller run beside runs 5-7.

| run | pre-Record sleep | startBeatInBar | result | grid (+s vs T1; expected 0.6 / 2.6 / 4.6 / 6.6) | from-now first move | take stamp - 2t (max) |
|---|---|---|---|---|---|---|
| 1 | 0 | 0.245 | 71/3 | +0.35 / 2.03 / 3.91 / 5.91 (FAIL x3) | +0.58 | 0.02 |
| 2 | 0 | 0.011 | **74/0** | +0.52 / 2.55 / 4.51 / 6.53 | +0.56 | 0.02 |
| 3 | 0 | 0.352 | **74/0** | +0.53 / 2.53 / 4.50 / 6.57 | +0.62 | 0.02 |
| 4 | 1.75 | **3.531** | 72/2 | +0.71 / 2.71 / 4.72 / 6.69 (pass) | +0.57 | 0.01 |
| 5 | 1.0 | 1.931 | 52/23 | environment (below) | never ran | 0.02 |
| 6 | 1.1 | **2.016** | **74/0** | +0.56 / 2.54 / 4.55 / 6.56 | +0.61 | 0.02 |
| 7 | 1.8 | **3.445** | **74/0** | +0.51 / 2.51 / 4.52 / 6.50 | +0.56 | 0.02 |

In EVERY run the take's stamps now agree with its map (|beat - 2t| <= 0.02 beats, inside D1's 0.05
lint) and every move was first seen at its stamped routine beat. Two gate-worthy runs with
startBeatInBar > 2 (6: 2.016, 7: 3.445) are 74/0. The failures in runs 1, 4, 5 are the environment:

- **Run 5 (VERIFIED by bpm-5.json):** the tracker's beatPhase hard-reset to 0 at phase ~0.70 about
  every 0.36 s for ~11 s (43 jumps > 0.15 beat; bars of 11.1 s and 12.9 s instead of 2.0 s; the take
  got a `reset` anchor) while the mic heard onsets (rms 0.002-0.006). No bar edge -> routine never
  started -> 23 rows failed. Source: `BPMTracker::updatePhase` hard-resets `phase_` on `beat && conf >=
  kBeatResetConfidence (0.5)` (`src/analysis/BPMTracker.cpp:209`), and the MANUAL branch of
  `runPipeline` (`:77-82`) still calls it with aubio's beat flag -- manual 120 BPM is not immune to
  room sound. Runs 6-7 had 0 jumps (bars 1.97-2.02 s).
- **Run 1 (INFERRED, same mechanism):** the published routine position ran ~13% faster than wall for
  ~3 s (wall-fit spread 0.467 s vs 0.03-0.05 elsewhere) -- what a string of phase resets from ~0.7
  does to the integrated clock (each is a >= 0.5 backward jump, counted as a wrapped beat). Moves still
  fired at their stamped beats; wall time did not track beats. No bpm sampler ran for run 1.
- **Run 4 (INFERRED, same mechanism):** grid passed; the stack step's second routine started 4.55 s
  (not 2 s) after the first although the app logged consecutive bars 29 -> 30: a stretched bar.

Machine load 6-11 during runs 1-5 (other lanes' clang -j4); 4-6 during 6-7.

## 8. Does any row pass vacuously when startBeatInBar is large?

No row CAN'T fail, but several are BLIND to this defect class (they passed on the buggy take):
- `take: meta.startBeatInBar present and in [0, 4)` -- range only; never checked against the stamps
  it describes. `take: tempoMap has N anchor(s), all with a tempo` -- never checks the stamps against
  the map. Neither can catch a take that disagrees with itself.
- The three `stack:` timing rows derive their windows from the take's OWN ramp `x0/x1` -- a
  self-referential oracle: the stamp offset cancels exactly, so they pass whatever the clock does
  (gate take: ramp at 18.85..25.39 = true 18.0..24.5 + 0.85).
- `loop: cycle 2 replays the first move` bounds at `<= T2 + 9.0` for an expected +8.6: it absorbed
  0.33 s of the 0.42 s defect (+8.93 in the gate) -- loose, not vacuous.
- `restored frame captured ... before the first move at +0.6` and `start from now: stays at 0.1 until
  the first move (T4..T4+0.45)` got MORE slack from a late first move -- their premise was false but
  the check held.
- `re-fire: first move played` (fixed `sleep 0.9` for a +0.6 move) passed at +1.03 by ~0.1 s --
  fragile at frac(startBeatInBar) > ~0.9 on the old build.
Recommended probe additions (NOT made -- the probe is not wrong): a take row asserting every point's
`beat` agrees with `tempoMap` (D1 lint, <= 0.05), and a check that `/api/bpm` bars stayed ~2.0 s
through the grid window so a mic-reset run reports "environment", not "grid".

## Fence

Packet fence: RoutineSlice.*, RoutineEngine.*, Program.cpp compileRoutine, MainComponent.cpp
perfRecord's startBeatInBar fill, tests. The root cause is in **`src/recording/RecorderClock.cpp`
(OUTSIDE the fence)**. It is isolated in its own commit (with its ctest) so Harmony can take or drop
it; the `[bargrid]` ctest in `tests/test_routine_engine.cpp` depends on it and goes RED without it.
No in-fence change can fix the take stamps (RoutineSlice re-deriving beats from `t` would be a
workaround that leaves every take on disk self-inconsistent). The RoutineEngine Beat-edge change is
in fence. `probe-routines.sh`, notebook/HANDOFF/gotchas/AGENTS/.harmony-version: untouched.

## Risks

- Every take recorded before this fix carries stamps offset by frac(startBeatInBar) at Record; not
  migrated (a slice of an old take stays late by that much). A loader lint (D1) would flag them.
- RecorderClock also drives the take's `durationBeats`, markers' `beat`, `/api/perf/status` `beat` --
  all now "since Record" (what D1 specifies). No probe reads them (grep of `.harmony/probe-*.sh`).
- The manual-mode onset reset (section 7) is a product question outside this lane: in Manual BPM an
  aubio beat with confidence >= 0.5 still resets the phase, contradicting the P24 comment "No real
  onset can be trusted while the operator has overridden the tracker". It makes probe-routines (and
  any wall-clock probe on manual BPM) flaky whenever the mic hears rhythmic sound.
