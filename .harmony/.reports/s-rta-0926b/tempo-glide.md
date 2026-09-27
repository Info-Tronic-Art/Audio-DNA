## BUILDER REPORT -- s-rta-0926b lane X (tempo-glide): plan3 ITEM A + ITEM C

STATUS: DONE_WITH_CONCERNS
RESULT: Both items are built, one commit each, on branch `lane/tempo-glide-0926b` (base `fdf46b9`). Commit `2b32536` (ITEM A): a typed BPM, REST `set_bpm` or OSC tempo now changes only the tempo and never moves the beat. Commit `d640b13` (ITEM C): a routine's restore glides onto the bar -- at the start, at every loop return and at a re-fire restart. New tests fail on the old code and pass on the new code; the full serial ctest passes (610/610); all 8 live probe runs gave the expected result.
FACTS: `src/MainComponent.cpp:5182` applyTempoCommand ("manual"/"link" -> followExternalTempo; only "tap" calls setManualBPM); `tests/test_bpm_stabilization.cpp:1057` T1; `.harmony/probe-manual-bpm.sh:238` setbpm mode (S1-S3); `src/recording/RoutineEngine.cpp:217` scheduleGlides, `:266` stepGlides, `:343` releaseGlides, `:352` startNow, `:528` loop-return schedule, `:585` restart schedule; `src/recording/Player.cpp:90` firePreambleDiscrete, `:101` firePreambleContinuous, `:118` holds; `src/MainComponent.cpp:2070` dispatch.read, `:5785` "glides"; `tests/test_routine_engine.cpp:950` G2..`:1327` G11; `tests/test_take.cpp:942` G10; evidence under `.harmony/.reports/s-rta-0926b/tempo-glide-evidence/`.
METHOD: I read the plan in full and re-anchored its cites by function name (main was one docs commit past `50b4bce`). ITEM A: code, then the T1 guard ctest, then probe rows S1-S3 -- RED on the pre-change `build/` app, GREEN on a snapshot of the ITEM-A-only build-lane app. ITEM C: I wrote the G tests first and ran them on the base engine (adding only the two unused header fields `Dispatch::read` and `Slot::glides` so they compile), plus G10 compile-RED on the base Player. Then I implemented, ran ctest GREEN, and ran probe-routines RED on `build/` then GREEN on build-lane. Every live run held `/tmp/audiodna-live.lock` and launched with `open -g`.
CONFIDENCE+VERIFY: high. Rebuild build-lane and run `ctest -j1` (expect 610/610). Run `.harmony/probe-routines.sh` with `ROUTINES_APP=<build-lane app> ROUTINES_RECORD_PAUSE=1.8` (expect 86/0; on the pre-change `build/` app expect 79/7, the 7 being exactly the new glide rows). Run `.harmony/probe-manual-bpm.sh` (expect 22/0; 18/4 on `build/`, S1-S3 failing). Also: probe-resync 16/0, probe-mastersignal 22/0, probe-step3 94/0. LOOK at `.harmony/.reports/s-rta-0926b/tempo-glide-evidence/green-mid.png`.
UNKNOWNS/NOT-DONE: No ctest pins the 2 Bar / 4 Bar boundary prediction (`beatsUntilBoundary`); the plan's G list has no such case, and I checked the arithmetic by hand against the ctest-11 rig. Not fixed because outside the fence: stale `linkTick` comments in `tests/test_link_sync.cpp:11-12,41`, and the header of `.harmony/probe-manual-bpm.sh` (lines 4-5 still say "Tap / a new set_bpm" realign). The BORIS_DECISIONS entry, the `bpm.md:86-88` closure and the three lines for Boris (plan section 6) are Harmony's.
NUANCE: I departed from the plan's C.3 text in six places; three were needed to make its own G tests and probe rows hold. Details in ISSUES #1-#6 below. In both 5g probe runs the app showed "running" about 0.11 s after the probe's predicted bar edge, and in the GREEN run the glide held at 0.981 for about 0.1 s just before the bar. I think the probe's own `render_frame` (mid.png) is blocking the message thread for about 100 ms (it starts about 0.27 s after the request), but that is inferred, not proven. It does not break the glide: it lands the last 2% on the bar.
HANDOFF-NEEDS: none

INBOX-RECHECK: none

### SUMMARY
**ITEM A:** `applyTempoCommand` now handles "manual" (typed BPM) and "link" (REST/OSC `set_bpm`, Link, a replayed value) with `followExternalTempo`, which never realigns the beat. Tap is the only tempo action that calls `setManualBPM` (which does realign). Resync is unchanged. The `linkTick` parameter is gone.

**ITEM C:** The engine now drives each continuous restore entry as a glide: a straight line from what the control shows now (new `Dispatch::read`) to the recorded value, over one beat that ends on the boundary, with a quarter-beat minimum that spills past the boundary when there isn't time. The discrete half still fires on the bar (`Player::firePreambleDiscrete`). The loop return and a re-fire restart use the same rule. A human hand, another routine, or the recording's own move on that knob (`Player::holds`) cancels the glide without a release. Stop, stop-all, the end of a once routine, and loop or restore being switched off at the end all let go of any glide in flight where it is. Status gains `glides`. The start notice no longer shows the growing bar number.

### FILES CHANGED
Commit `2b32536` (ITEM A):
- `src/MainComponent.cpp` -- `applyTempoCommand`: "manual" and "link" call `followExternalTempo`; the `linkTick` parameter and its Link-timer argument are removed.
- `src/MainComponent.h` -- new signature; the comment is rewritten to the plan's sentence.
- `src/analysis/BPMTracker.h` -- comments only: `setManualBPM` is the Tap path; `followExternalTempo` covers every tempo value.
- `tests/test_bpm_stabilization.cpp` -- T1 `[coalesce]` (sections (i) and (ii)); two stale comments that called set_bpm a realign are corrected.
- `.harmony/probe-manual-bpm.sh` -- new `setbpm` mode (S1 same value x3, S2 changed value and back, S3 an OSC value stream) and its driver line between `manual` and `resync`.
- `docs/claude/effects.md` -- one clause added to Manual BPM Mode.
- `docs/claude/performance-controls.md` -- the Link sentence (no `linkTick`) and "Tap and Resync realign; a REST/OSC/typed `set_bpm` never does".

Commit `d640b13` (ITEM C):
- `src/recording/RoutineEngine.h` -- `Dispatch::read`, `Status::Slot::glides`, the constants `kRestoreGlideBeats`, `kRestoreGlideMinBeats`, `kBeatsPerBar`, the private `Glide` type and helpers, and `lastBeatInBar_`.
- `src/recording/RoutineEngine.cpp` -- `Glide`, `busyUntil`, `beatsUntilBoundary`, `scheduleGlides`, `stepGlides`, `releaseGlides`; changes to `startNow`, `tick`, `fire`, `stop`, `stopAll` and `publishStatus`; the notice text. SlotSink is unchanged.
- `src/recording/Player.{h,cpp}` -- additive: `firePreambleDiscrete`, `firePreambleContinuous`, `holds`. `firePreamble` now calls the two halves in the same order.
- `src/MainComponent.cpp` -- the `routineEngine_.dispatch.read` lambda (it never refers to `routineEngine_`) and `"glides"` in `routineStatusVar`.
- `tests/test_routine_engine.cpp` -- rig changes (`values`/`read`, `refuseSet`, `beatInBar`); test 7 rewritten as G1; test 9's re-fire section rewritten as G12; the stacking "restore begins later" pin updated; new G2-G9 and G11.
- `tests/test_take.cpp` -- G10 appended after line 937; lines 866-935 are unedited.
- `.harmony/probe-routines.sh` -- `'glides'` added to `snap()`; new helpers `edge`, `glide` and `restartglide`; rows 5g, 8g, 9g, 11g and stop-while-pending. Launch, pgrep, lock and header lines are untouched.
- `docs/claude/recording.md` -- a restore-glide paragraph in the Fire/restore bullet.

### TESTS
**Full serial ctest**
- ITEM A tree: `100% tests passed, 0 tests failed out of 600`
- ITEM C tree: `100% tests passed, 0 tests failed out of 610`
- The 10 new TEST_CASEs are G2-G9, G11 and G10.

**T1** (`[coalesce]`): passes on base and on the new code (6 assertions). It is a guard by design -- `BPMTracker.cpp` is unchanged.

**ITEM C ctest, RED on the base engine** (the base engine plus only the two unused header fields), from `test_routine_engine`:
`test cases:  18 |   7 passed | 11 failed` / `assertions: 479 | 454 passed | 25 failed`
- Failing: G1 (test 7), test 9 (G12), the stacking update, G2, G3, G4, G5, G6, G7 (section b), G8, G11.
- G9 and G7a pass on base -- they are guards.
- Example failures:
  - `test_routine_engine.cpp:286: FAILED: ev.size() == 2 for: 0 == 2` (G1: no glide at 3.0)
  - `:1079 count(Ev::Release, op1) == 1 for: 2 == 1` (G4)
  - `:1346 count(Ev::Touch, op1) == 2 for: 1 == 2` (G11)
- Full output: `tempo-glide-evidence/red-routine-engine-base.txt`; per case: `red-percase.txt`.

**G10 RED:** `test_take.cpp:964:22: error: no member named 'firePreambleDiscrete' in 'Player'` (also `firePreambleContinuous` and `holds`). File: `red-test-take-compile.txt`.

**ITEM C ctest, GREEN:** all 18 `test_routine_engine` cases pass, and G10 passes (`green-percase.txt`).

**Live runs** (verbatim summary lines; logs in `tempo-glide-evidence/`):
- probe-manual-bpm on the pre-change `build/` app: `18 PASS / 4 FAIL`. The four failures are S1 `0/3 continuous` (step error 0.41 beat), S2 twice (step error 0.415 / 0.428) and S3 `19 jumps ... totalBarCount +0`.
- probe-manual-bpm on the ITEM A app (build-lane-A; its sha256 `b3b571df...` matched build-lane at that moment): `22 PASS / 0 FAIL`. S1 `3/3 continuous`, step errors 0.003-0.006 beat; S2 both PASS; S3 `0 jumps ... totalBarCount +1`.
- probe-resync on the ITEM A app: `16 PASS / 0 FAIL`.
- probe-routines on the pre-change `build/` app (`ROUTINES_RECORD_PAUSE=1.8`): `79 PASS / 7 FAIL`. The 7 are all new rows: 5g glide, 5g glides-counter, 5g mid-frame (`mad(pert, mid) = 0.000000`), 8g glide, 9g, 11g, and stop-while-pending (L0 0.100 at both reads). The other 5 new rows are guards that pass on base: 5g hold, mid.png written, mid.png non-blank, 8g landing, stop-while-pending state.
- probe-routines on build-lane (ITEM C app): `86 PASS / 0 FAIL`. That is the 74 existing rows plus 12 new ones.
  - 5g: over the last beat L0 rises 0.386 -> 0.981; `glides` shows 3 during and 0 after.
  - 5g mid frame: `mad(pert, mid) = 23.5`, `mad(mid, rest) = 73.2`.
  - 8g: rises 0.924 -> 0.993 and lands at +7.91 s.
  - 9g: rises 0.589 -> 0.995.
  - 11g: rises 0.477 -> 0.955.
  - Stop-while-pending: L0 holds 0.693 at both reads.
- probe-mastersignal on build-lane: `22 PASS / 0 FAIL`.
- probe-step3 on build-lane: `94 PASS / 0 FAIL` (same count as before).

**mid.png, looked at:** `green-mid.png` shows L0 still on column 1 (the checkerboard) at a mid opacity and brightness. `green-pert.png` is near-black and `green-rest.png` is the restored image. So the frame is between the two looks, not a cut. `red-mid.png` is identical to pert on the old code.

### SLIM CHECK
Nothing to cut. Every new helper has a caller and a test (`beatsUntilBoundary` through G1/G2/G3; `busyUntil` through G5 and 9g). `glideScheduled` is needed for G7a. `Player::holds` exists only for the cancel rule (G4).

### ISSUES
These are departures from the plan's text; each is covered by a test or a probe row.
1. **`busyUntil` ignores gestures that begin at or after the boundary** (`x0 < endPos`, where `endPos` = the loop length for the loop return, and `boundary - startBeat` for the restart). The plan's version counted every gesture before the loop length. For the restart that would make the glide spill past the restart bar whenever the recording moves that knob again later: the probe routine moves L0 again at beat 13.2, which would have failed 9g. For the loop return the result is identical to the plan's.
2. **At the start, only a glide whose t0 is still ahead is moved to a quarter-beat spill** (the case where the bar came earlier than predicted). A glide that was already due but had no tick yet keeps its own window. The plan's G3 numbers (0.75 at 4.0) need this; moving every unstarted glide would contradict them.
3. **The cancel rule (`Player::holds`) is checked only once a glide is due or started.** Taken literally, the plan's first line would drop G5's op0 spill glide while the recording still holds op0 before that glide is allowed to start.
4. **`Running::glideScheduled` is a flag the plan did not list.** It records "the continuous half of the next restore belongs to the glides". Without it, a glide dropped during the wait (touch refused) would be fired a second time by the one-shot fallback at the start; G7a pins a single touch attempt and `preambleRefused == 1`. The loop end also falls back to the one-shot when no return glide was scheduled (no `read` wired, no beat, or a tick that skipped the whole last beat).
5. **While a restart is requested, the loop return is not scheduled; the restart owns the next restore.** If a loop end comes first, the restart's glides stay pending for the restart bar.
6. **`firePreamble` now calls the two halves in the same order** (same behaviour and count; the `test_take.cpp` preamble tests are unedited and pass). The plan's reviewer grep says "firePreamble body unchanged". I read "firePreamble = the two in that order" as the intended split. If the Reviewer wants the original body kept byte-for-byte, the two halves would have to duplicate its loops.
7. **Probe rows adjusted from the plan's text:**
   - 8g landing: tolerance 0.01, not 0.05 -- a 0.9 -> 1.0 glide is already within 0.05 of 1.0 halfway through, at +7.75 s.
   - Stop-while-pending: compares the reads at +0.1 s and +0.6 s after the stop, not the last read before the stop (that one is 50-100 ms of glide earlier).
   - 11g: adds `max > 0.75`, because the ramp alone is also non-decreasing and the row would otherwise pass without a glide.
   - The plan's figures (1.0 s beat windows, etc.) are unchanged.
8. **Notice text:** `"Routine <name> started."`; with suffixes it reads `"Routine <name> started; 1 control you are holding was left alone."`
9. **Rig mechanics:**
   - Other lanes write into the same scratchpad directory; one overwrote my first `build.sh` while my cold build was running. I moved all my files to `scratchpad/tempo-glide/`; the build finished cleanly.
   - `build-lane-A/` in the worktree holds the ITEM A app snapshot used for the ITEM A GREEN runs. Git ignores it, and it can be deleted.

### SKILL_PROPOSALS
None.

### RISKS
- **(low) Restart fired within a quarter beat of a start spill.** Per the plan's re-aim rule, a knob that is still settling gets pulled toward the restart bar as a slow creep of up to 4 beats instead of finishing its quarter-beat settle. Never a jump, and a hand still takes the knob. Mitigation: leave as is, or re-aim only when the target value changes.
- **(low) 2 Bar / 4 Bar boundary prediction has no ctest** (see UNKNOWNS). A wrong prediction lands the glide early and holds, or turns it into a quarter-beat spill -- never a snap.
- **(low) Message-thread stalls hold a glide's value** (seen once, about 0.1 s, near a `render_frame`). It resumes and lands at the bar.

### METRICS
- Self-check: build-lane cold and incremental builds exit 0 (the only warning is the pre-existing `MainComponent.cpp:1587` unused parameter); ctest 600/600 and then 610/610 (serial); 8 live probe runs as listed.
- Tool calls: about 130.
- Files read: about 30.

### KNOWLEDGE CONTEXT
- Tools used: grep only (the packet had no KNOWLEDGE_TOOLS block).
- Impact authority: grep. I took the conservative route: no deletions, additive Player API.
- Risk level: NORMAL.
- Queries made: 0.

### PACKET QUALITY
- Clarity: CLEAR (the plan is precise). I had to infer the four C.3 edge cases in ISSUES #1-#4.
- Missing context: other lanes share the scratchpad directory, so scripts need unique names. The probe tolerance problems in ISSUE #7 were only found when working the numbers through.
- Unused context: plan section 3 (ITEM B, lane Y).
- Self-assembly: LEGACY (no DEPARTMENT field).
- Self-brief files: `.harmony/notebook.md` (headings only: routines, recorder and bpm entries) was useful. Nothing was missing.

### STATUS
DONE_WITH_CONCERNS. Every success criterion is met, with RED and GREEN evidence for both items. The concerns are the plan-text departures (ISSUES #1-#7), the lack of a 2 Bar / 4 Bar ctest, and stale comments outside the fence.

### NEXT ACTION
Harmony: run the gate on `lane/tempo-glide-0926b` (`d640b13`), send the Reviewer the ISSUES list, then merge. Also fix the stale `linkTick` comments in `test_link_sync.cpp` and the probe-manual-bpm header line when the fence allows.
