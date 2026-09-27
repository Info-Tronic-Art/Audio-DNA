# Reviewer Verdict — routine-display round 3
STATUS: DONE
VERDICT: PASS
REVIEWED_COMMIT: e4e9989 (fix-round-2 range b6e49fa..e4e9989), worktree rta0927-w3 branch lane/routine-display-0927

## Scope
Reviewed src/recording/RoutineEngine.{h,cpp}, src/ui/UniversalParamControl.cpp, tests/test_routine_engine.cpp
(D7, 6 sections), tests/test_param_control_routine_cue.cpp (new contrast case), .harmony/probe-routine-display.sh
(d15), docs/claude/recording.md, .harmony/notebook.md — the full b6e49fa..e4e9989 diff (git diff --stat: 13 files,
443/16, plus 4 PNGs). Re-ran the two relevant unit binaries live against the pinned tree (no rebuild needed,
binaries already current): `test_routine_engine "RoutineEngine display D7*"` → 82/82 assertions pass, 1/1 case;
`test_param_control_routine_cue` → 14/14 assertions, 2/2 cases (VERIFIED, not recalled from the report).

## (1) Restart-wait now honours every menu setting, walked through the code
- `tick()`: gate widened from `if (r.pending)` to `if (r.pending || r.restartRequested)` before calling
  `resyncPending` (RoutineEngine.cpp:539) — the SAME live-routine lookup/re-read runs for both waits.
- `resyncPending` (RoutineEngine.cpp:479-503) re-reads `ownSnap` (Quantize), `loop`, `restore` (Restore
  first/Start from now) and `jump` (Start: Ease/Jump) unconditionally on every tick for both `r.pending` and
  restart-wait, from the live `Routine` in the slot (uuid-checked) — before `effectiveSnap()` is computed for
  that same tick's `dueNow()` test, so a Quantize edit changes the boundary the SAME tick it lands (verified by
  D7's Quantize sections: `startsOn` flips at once, and the restart fires on the new grid, not the old one).
- Restore/Ease-Jump transitions during the wait: `easedBefore`/`eased` bracket exactly as before; `eased→!eased`
  releases glides (`releaseGlides`), `!eased→eased` or a Quantize change while eased re-times them. For a restart
  (the `else` branch, RoutineEngine.cpp:490-501) the re-time goes through the NEW `scheduleRestartGlides` helper
  instead of the pending-path's `now`-based one — correctly, because a restart mid-flight has a live position/
  knob-busy-until state a fresh pending start does not.
- Restore first/Start from now ("jump" toggled off entirely, i.e. `restore=false`) during a restart wait: covered
  by D7's last section — `firedRestores()` stays flat, `Touch` count reflects only the start's own glide. Matches.
- All four settings are exercised end-to-end over REST in probe d15 (new), not just in the unit test.

## (2) No timing-semantics change beyond the fix
`scheduleRestartGlides` (RoutineEngine.cpp:461-468) is a byte-for-byte lift of the old inline block that used to
sit in `fire()`'s re-fire branch (confirmed by diffing the removed lines against the new function body — same
three local variables, same `scheduleGlides` call, same lambda). `fire()`'s re-fire branch now just calls the
helper (RoutineEngine.cpp:695). No other call site changed. The only new *timing* is that the restart-wait
resync now uses this same rule when the settings change — the unedited path is provably identical.

## (3) New edge states — checked, none newly disagree
- **Edit lands exactly on the boundary tick**: `resyncPending` runs before `dueNow()` is evaluated in the same
  iteration; if `dueNow()` becomes true this same tick, `startNow()` fires using the just-updated `r.restore`/
  `r.loop`/`r.jump` — no stale read, no double-apply (the reschedule from `resyncPending` is simply overwritten
  by `startNow`'s own restore logic).
- **Delete/Rename during a restart wait**: `resyncPending` early-returns when `live == nullptr || live->uuid !=
  r.uuid` (RoutineEngine.cpp:480-481), same as it always has for `r.pending`. This is pre-existing behaviour
  inherited unchanged by the restart-wait path, not a new hole created by this fix.
- **Stop during a restart wait**: `RoutineEngine::stop()` (line 751) releases glides, stops the player and erases
  the `Running` entry unconditionally, regardless of `pending`/`restartRequested` — unaffected by this change.
- **Menu open across the boundary**: UI-side only; `UniversalParamControl.cpp`'s change here is limited to the
  hint's alpha/colour (paint-only), no interaction with `restartRequested` — not implicated.

## (4) No allocation/lock on render/audio paths
`scheduleRestartGlides`/`resyncPending`/the widened `tick()` gate are message-thread-only RoutineEngine code
(same file/thread as before); no new heap allocation pattern (glides vector reuses existing `scheduleGlides`
machinery), no mutex added. Consistent with the Sacred Rules.

## (5) D7 / d15 were RED on b6e49fa, and actually drive the engine
- D7 (`tests/test_routine_engine.cpp`, 6 sections) — report cites the RED against b6e49fa's engine (scratch
  build): `assertions: 74 | 53 passed | 21 failed`; GREEN after the fix: `82 assertions in 1 test case`.
  Independently reproduced here: GREEN, 82/82.
- d15 (`.harmony/probe-routine-display.sh`) drives the real running app over REST (`/api/routine/set`,
  `/api/routine/fire`), not a mock — report cites RED on the b6e49fa binary (`15 PASS / 1 FAIL`, `FAIL d15
  quantize set to 4bar while the restart waits: (restartPending, startsOn, state) (True, 'bar', 'running')`) and
  GREEN on the fix build (`16 PASS / 0 FAIL`). The script's retry logic (RoutineEngine.cpp not involved; probe
  script only) is honest — it marks a race as inconclusive rather than silently passing it.
- Contrast test (`test_param_control_routine_cue.cpp`) — RED on b6e49fa: `4.37882820501888581 >= 7.0` fails;
  GREEN after: `13.9691:1`. Reproduced here: GREEN, 14/14.

## (6) Nothing stray
`git status --porcelain` in the worktree: only `build-lane/` untracked (a build directory, consistent with prior
rounds' disclosed convention). No stray commits beyond the four listed (beabbff, 14aae66, 8cdfb77, e4e9989). The
report states the hook patch (sha256 4b55ed59…) was applied and reverted (`git apply -R`) each batch, with
`AUDIODNA_DEBUG_` string count back to 0 and `git diff b6e49fa..HEAD -- src/MainComponent.cpp` empty — this diff
range confirms MainComponent.cpp is untouched in this round (git diff --stat has no MainComponent.cpp entry).

## Findings
None MUST. None SHOULD blocking.
- NIT: `docs/claude/recording.md`'s new sentence ("one made while a pressed-again pad's restart waits reaches
  that restart") and the notebook entry accurately describe the code (`r.pending || r.restartRequested` gate) —
  spec-fidelity check passed, no overclaim.

## Verdict
PASS. The round-2 fix correctly extends the existing pending-wait resync to the restart-wait via one added
boolean in the tick() gate and reuses the pre-existing glide-scheduling rule through a mechanically-extracted
helper (no behavioural drift). Edge cases (boundary-tick edit, delete/rename, stop, menu-across-boundary) either
resolve correctly or inherit pre-existing (out-of-scope) behaviour unchanged. D7 and d15 are real RED→GREEN
against the b6e49fa baseline and were independently reproduced GREEN in this review. No stray state.
