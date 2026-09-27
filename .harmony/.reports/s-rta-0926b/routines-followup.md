## BUILDER REPORT -- s-rta-0926b lane routines-followup (Items 1-3)

STATUS: DONE_WITH_CONCERNS
RESULT: All three items are built on branch `lane/routines-followup-0926b` (base `41aeb23` = main), one commit each: `f8d1d6e` (Item 1: the TopBar Stop stops routines only), `95a3422` (Item 2: each routine has a restore style, Ease or Jump), `41be0ea` (Item 3: a 2 Bar / 4 Bar prediction ctest, stale comments, docs). The full serial ctest passes (620/620). probe-routines is 98/0 on the new app and 90/8 on the pre-change `build/` app, where the 8 failures are exactly the new Jump rows. probe-manual-bpm (22/0), probe-resync (16/0) and probe-mastersignal (22/0) pass on build-lane.
FACTS: `src/MainComponent.cpp:701` onStop now only calls `routineEngine_.stopAll()`; `src/MainComponent.cpp:7230` GlobalStop binding now routines only; `src/ui/TopBar.cpp:37` tooltip "Stop all routines"; `src/model/Routine.h:27` `RestoreStyle { Ease, Jump }` + JSON "restoreStyle"; `src/recording/RoutineEngine.cpp:136` `Running::jump`, `:539` loop return reads the live style, `:591` Jump re-fire, `:641` no glide scheduled for Jump; `src/api/ApiServer.cpp:1650` `restoreStyle` parsed, 400 on other values; `tests/test_topbar_link_toggle.cpp:95`; `tests/test_routine.cpp:283`; `tests/test_routine_engine.cpp:1376` J1-J4, `:1650` the Item 3a case; `.harmony/probe-routines.sh` row 11j; evidence in `.harmony/.reports/s-rta-0926b/routines-followup-evidence/`.
METHOD: For every item I wrote the tests first and showed them failing on the pre-change code, then built the change, then showed them passing. Item 1: the ctest failed on base (no tooltip). A live witness on the pre-change app plus a scratch-only REST hook gave 9/3; the same witness on the fixed app plus the hook gave 12/0. The hook was never committed. Item 2: on a stage that had only the model field (no JSON, no engine use), 5 ctest cases failed with assertions. The probe rows were RED on `build/` and GREEN on build-lane. Item 3a: the case is new, so it is RED by absence. I compiled two mutants from a scratch COPY of RoutineEngine.cpp and linked them into a scratch test binary; the case catches both. Every live run held `/tmp/audiodna-live.lock` (owner `routines-followup`) and launched with `open -g`.
CONFIDENCE+VERIFY: High. To check: rebuild build-lane and run `ctest --test-dir build-lane -j1` (expect 620/620). Run `.harmony/probe-routines.sh` with `ROUTINES_BUILD_DIR=build-lane ROUTINES_RECORD_PAUSE=1.8`, expecting 98/0; on the pre-change `build/` app expect 90/8, the 8 being the 11j rows. Run `probe-manual-bpm` (22/0), `probe-resync` (16/0) and `probe-mastersignal` (22/0), each with its `*_BUILD_DIR=build-lane`. Look at `routines-followup-evidence/green-jmid.png` (the dim waiting look: nothing eased in) against `red-jmid.png` (mid-glide on the old app).
UNKNOWNS/NOT-DONE: No permanent test proves the TopBar Stop leaves clips alone. MainComponent cannot be linked into a unit test, and no REST, OSC or binding route reaches the button without synthetic input. The ctest pins only the tooltip and the button-to-onStop wiring; the behaviour proof is the scratch-hook live witness. No on-screen Ease/Jump control was built (the packet says it comes with the routine display design). `/api/routine/save` does not take `restoreStyle` (not asked), so a new routine starts as Ease and is changed with `/api/routine/set`.
NUANCE: Fence departures, all needed for the items: (1) `src/model/Routine.h` -- the packet's fence names `src/recording/Routine*`, but the Routine model and its JSON live in `src/model/Routine.h`, and "persisted with the show" is not possible anywhere else. (2) `src/api/ApiServer.h` -- the `RoutineSetOpts` routine section gains one field. (3) `src/ui/TopBar.h` -- one comment line, because the onStop comment ("play/pause/stop the active deck's layers") would otherwise be false. The GlobalStop binding (labelled "Stop") follows the button, as the packet says, so it no longer stops the audio file. The Play / Pause binding still stops a playing audio file, so no ability is lost. A style change reaches a running routine the next time it plans a restore: a fire, a re-fire, or the start of a loop's last beat. A glide already moving is not cut.
HANDOFF-NEEDS: none

INBOX-RECHECK: none

### SUMMARY
**Item 1.** The TopBar Stop (`[]`) now only stops every running and waiting routine, letting go of every control they hold. It no longer stops, pauses or rewinds any clip, and its tooltip reads "Stop all routines". The GlobalStop key/MIDI binding does the same. I checked every other path that could stop something, and none of them reaches this handler (table below).

**Item 2.** Each routine now has a restore style: `Routine::restoreStyle` is Ease (the default: today's merged glide) or Jump (exactly the restore before the glide existed). A Jump routine fires the discrete and continuous halves together in one call ON the boundary, at the start, at every loop return and at a re-fire restart, and never calls `Dispatch::read`. The style is saved with the show; a file without it, or with an unknown value, loads as Ease. `POST /api/routine/set` accepts `"restoreStyle": "ease"|"jump"` (anything else gets a 400), and `/api/routine/status` reports `bank[].restoreStyle`. OSC has no routine-set address, so nothing changes there.

**Item 3.** (a) A ctest pins `beatsUntilBoundary` for 2 Bar and 4 Bar by checking when the glide begins. (b) The stale `linkTick` comments are fixed, and the probe-manual-bpm header now says Tap and Resync are the only realignments. (c) One sentence each for Items 1-2 in `docs/claude/recording.md` and `docs/claude/performance-controls.md`.

**Item 1 -- every Stop path, what I decided, and why:**
| Path | Before | Now | Why |
|---|---|---|---|
| TopBar `[]` button (`topBar_->onStop`) | stopAll + stop/rewind every active-deck clip (recorded as clip points in a take) | `routineEngine_.stopAll()` only; tooltip "Stop all routines" | Boris: "ok we can keep stop for routines only" |
| Key/MIDI binding `GlobalStop` (bind overlay target "Stop") | stopAll + `applyAudioTransport("stop")` (stops and rewinds the audio file, recorded as an `audio` point) | `routineEngine_.stopAll()` only | The packet says a binding labelled as the transport Stop follows the button, and the design treats "Global Stop (TopBar and binding)" as one control. The audio file can still be stopped with the Play / Pause binding (`applyAudioTransport(isPlaying ? "stop" : "play")`). |
| OSC | no stop address (`/audiodna/routine/{slot}` fires; 0 is ignored) | unchanged | nothing reaches the handler |
| REST `/api/perf/stop` | stops the take recording (`perfStop`) | unchanged | a different action (Record), not Stop |
| REST `/api/perf/stop_play` | stops the take replay and its audio | unchanged | a different action (Stop Playback) |
| REST `/api/routine/stop` | routines (slot or all) | unchanged | already routines only |
| Record panel `onStop` / `onStopRoutine` | take stop / one routine | unchanged | different actions |
| `stopAll()` at shutdown and composition swap | routines | unchanged | not a Stop gesture |

### FILES CHANGED
Commit `f8d1d6e` (Item 1):
- `src/MainComponent.cpp` -- `topBar_->onStop` is now `routineEngine_.stopAll()` only (the clip-stop loop is removed); the `GlobalStop` case is now `stopAll()` only (`applyAudioTransport("stop")` removed).
- `src/ui/TopBar.cpp` -- `stopButton_.setTooltip("Stop all routines")`.
- `src/ui/TopBar.h` -- the onStop comment (fence departure 3).
- `tests/test_topbar_link_toggle.cpp` -- new TEST_CASE "the TopBar Stop button says it stops all routines": finds the child TextButton "[]", checks its tooltip, and checks that one click calls `onStop` once.

Commit `95a3422` (Item 2):
- `src/model/Routine.h` -- `enum class RestoreStyle { Ease, Jump }`, the field (default Ease), `restoreStyleToString` / `restoreStyleFromString` (unknown values read as Ease), and `"restoreStyle"` in `toVar` / `fromVar`.
- `src/recording/RoutineEngine.h` -- `Status::Slot::restoreStyle`, and the class comment.
- `src/recording/RoutineEngine.cpp`:
  - new `Running::jump`, set at fire and at a re-fire;
  - re-read from the live model at the loop end and at the loop-return scheduling point;
  - Jump never calls `scheduleGlides`, so `startNow` and the loop end take their existing one-call path;
  - a Jump re-fire runs `releaseGlides` (this matters only when an Ease return glide is already in flight);
  - `refreshBank` fills `restoreStyle`.
- `src/api/ApiServer.h` -- `RoutineSetOpts::restoreStyle` (fence departure 2).
- `src/api/ApiServer.cpp` -- `handleRoutineSet` parses `restoreStyle` and answers 400 unless it is ease or jump.
- `src/MainComponent.cpp` -- `perfRoutineSet` applies the style, and the notice reads "restores first (ease|jump)"; `routineStatusVar` adds `"restoreStyle"`.
- `tests/test_routine.cpp` -- new "[restorestyle]" case: the style round-trips as a JSON string; no key or an unknown value loads as Ease.
- `tests/test_routine_engine.cpp` -- the rig gains `FakeDispatch::reads`; new J1-J4.
- `.harmony/probe-routines.sh` -- header lines, two new `rt.py` commands (`jump`, `jumploop`), and section 11j after 11b. Rows 1-11b are untouched.

Commit `41be0ea` (Item 3):
- `tests/test_routine_engine.cpp` -- new case "the 2 Bar and 4 Bar boundary prediction puts the glide in the last beat before the start" (5 SECTIONs).
- `tests/test_link_sync.cpp:11-12,41` -- the `linkTick` wording is removed from the comments.
- `.harmony/probe-manual-bpm.sh:4-5` -- header comment only.
- `docs/claude/recording.md` and `docs/claude/performance-controls.md` -- one sentence each for Items 1-2.

Report commit: this file and `.harmony/.reports/s-rta-0926b/routines-followup-evidence/` (added with `git add -f`).

### TESTS
**Full serial ctest** (`ctest --test-dir build-lane -j1`):
- base `41aeb23`: `100% tests passed, 0 tests failed out of 613`
- after Item 1: `100% tests passed, 0 tests failed out of 614`
- after Item 2: `100% tests passed, 0 tests failed out of 619`
- after Item 3: `100% tests passed, 0 tests failed out of 620`

**Item 1 ctest.** RED on base (main + the new test only), from `red-item1-ctest.txt`:
- `test_topbar_link_toggle.cpp:112: FAILED: CHECK( stop->getTooltip() == "Stop all routines" ) with expansion: == "Stop all routines" with message: tooltip ""`
- `test cases: 1 | 1 failed` / `assertions: 3 | 2 passed | 1 failed`

GREEN: `All tests passed (3 assertions in 1 test case)`.

**Item 1 live witness** (`stop-witness.sh` / `stop-witness.py`, scratch hook in `scratch-hook.diff`). Setup: fixture loaded, both layers on a playing clip, routine "Witness" (loop) saved from the latest probe-routines take, a take recording armed. Then Stop via the real `topBar_->onStop` and via the real `handleBindingAction(GlobalStop)`.

RED, on base + hook (app sha256 `90e8091c...`): `9 PASS / 3 FAIL`
- `FAIL  button: no clip is stopped -- every active clip still playing on the same column ([(0, 0, False), (1, 0, False)])`
- `FAIL  button: nothing captured into the recording (points 0 -> 2)`
- `FAIL  binding: no audio-transport stop captured into the recording (points 2 -> 3)`

GREEN, on the fix + hook (app sha256 `3fec98c2...`): `12 PASS / 0 FAIL`. The button and the binding both leave the routine idle, the clips `[(0, 0, True), (1, 0, True)]` unchanged, and the recorded points `0 -> 0`.

**Item 2 ctest.** RED on the base engine, with only the model field and the status field added (no JSON, no engine use):
- `test_routine "[restorestyle]"`: `test cases: 1 | 0 passed | 1 failed` / `assertions: 14 | 12 passed | 2 failed`
  - `:299` (the JSON key is missing)
  - `:304 CHECK( back.routineInSlot(0)->restoreStyle == Routine::RestoreStyle::Jump ) with expansion: 0 == 1`
- `test_routine_engine "[restorestyle]"`: `test cases: 4 | 0 passed | 4 failed` / `assertions: 54 | 37 passed | 17 failed`, for example:
  - `:1394 "ease" == "jump"`
  - `:1398 glides 1 == 0`
  - `:1402 REQUIRE( rig.fd.log.size() == at4 + 4 ) 20 == 21` (J1: Ease had been writing since 3.0)
  - `:1504 Touch(op1) 2 == 1` (J2: return glide at 7.0)
  - `:1564 2 == 1` (J3: restart glide at 7.0)
  - J1's Ease guard SECTION passes on base, as designed.

GREEN:
- `All tests passed (14 assertions in 1 test case)`
- `All tests passed (136 assertions in 4 test cases)`
- all 23 `test_routine_engine` cases pass, including G1-G12 unedited: Ease is unchanged.

**Item 2 probe-routines row 11j** (`ROUTINES_RECORD_PAUSE=1.8`):
- RED on the pre-change `/Users/boriskarpman/projects/RealTimeAudio/build/.../Audio-DNA.app`: `90 PASS / 8 FAIL`. The 8 failures are all 11j rows:
  - the `restoreStyle == None` status row;
  - hold 0.1 while waiting (`0.100..0.952`);
  - hard cut (`11 in between: [0.165, 0.242, ... 0.875]`);
  - `glides ['0', '3']`;
  - `mad(jpre, jmid) = 31.375`;
  - the loop holding 0.9 (`0.9..0.996`);
  - the loop landing (`+7.87 s`);
  - the loop in-between values (`[0.924 ... 0.972]`).
  - The 4 other new rows are guards: the jpre frame written, jpre non-blank, jmid written, and 1.0 after the bar.
- GREEN on the Item 2 app: `98 PASS / 0 FAIL` (86 existing + 12 new). Its sha256 `cb875808...` is identical to the final build-lane app, because Item 3 changes no app source. Readings:
  - hold `0.100..0.100` over 48 samples;
  - `0 in between`;
  - `glides ['0']`;
  - `mad(jpre, jmid) = 0.000000`;
  - the loop holds `0.9..0.9` and lands at `+7.96 s`.
- I looked at the frames: `green-jmid.png` is the dim waiting checkerboard (0.1); `red-jmid.png` is the mid-glide brighter checkerboard.

**Item 3a.** The new case passes: `All tests passed (50 assertions in 1 test case)`. Mutation witness (`mutation-3a.py`, a scratch copy compiled with the target's own flags and linked into a scratch binary):
- M1, the 2 Bar extra-bars term dropped: `assertions: 50 | 47 passed | 3 failed` (`:1663 CHECK( rig.fd.count(Ev::Touch, op1) == 0 ) 1 == 0`).
- M2, the 4 Bar parity off by one: `assertions: 50 | 37 passed | 13 failed`.
- `committed RoutineEngine.cpp sha256 before c272fc9b... after c272fc9b... (UNCHANGED)`.

**Final live gates on build-lane** (app sha256 `cb875808...`, one lock hold):
- probe-manual-bpm: `22 PASS / 0 FAIL`
- probe-resync: `16 PASS / 0 FAIL`
- probe-mastersignal: `22 PASS / 0 FAIL`

### SLIM CHECK
Nothing to cut. `Running::jump` is read at every scheduling point. The `releaseGlides` call on a Jump re-fire covers the one case where an Ease return glide is still in flight; without it that restart would skip its continuous restore. `FakeDispatch::reads` is the only witness for "no Dispatch::read". The two string helpers each have callers in the JSON, REST and status code.

### ISSUES
1. **Fence departures** (details in NUANCE): `src/model/Routine.h` (where the model and JSON actually live), `src/api/ApiServer.h` (one `RoutineSetOpts` field), and one comment line in `src/ui/TopBar.h`.
2. **The GlobalStop binding no longer stops the audio file.** This follows the packet's "any binding labelled as the transport Stop follow the button". The bind-overlay label still says "Stop" (`MainComponent.cpp:6945`). Renaming it to "Stop routines" is outside the fence; worth a word to Boris.
3. **When a style change takes effect** (pinned by J4): at fire, at a re-fire, and when a loop's last beat begins (the live read, like loop/restore); the loop end re-reads it too. Switching Ease -> Jump inside a return glide that is already moving does not cut it; it lands on the loop point. Switching Jump -> Ease inside the last beat takes effect at the next loop return.
4. **Notice text** from `/api/routine/set`: "restores first (ease)" / "restores first (jump)" (before: "restores first"). No probe parses it (grep).
5. **Rig mechanics:**
   - The RED and GREEN witness apps and the Item 2 app were snapshotted with `ditto` into the scratch directory, so build-lane could keep building while other lanes held the lock. The sha256 of each is listed above.
   - The scratch hook was removed before commit `f8d1d6e` (`grep -c SCRATCH src/MainComponent.cpp` == 0), and the Item 2 app binary has no `__scratch_stop` string.
   - The witness takes it recorded were copied to scratch and their folders deleted. It read the latest `probe-routines-*.adna-take` without changing it.
   - The `.venv` symlink existed only during live runs and was removed before the Item 3 and report commits. The Item 1 and 2 commits were made before it was created.

### SKILL_PROPOSALS
None.

### RISKS
- **(medium-low) No permanent test covers the Stop handler's behaviour.** If someone re-adds a clip stop to `onStop`, no ctest or probe fails. The tooltip ctest and the scratch witness only cover today's change. The fix would be a REST route or a probe-reachable seam, which is outside this fence.
- **(low) A performer who used the "Stop" binding to rewind a playing audio file loses that.** Play / Pause still stops it.
- **(low) A Jump re-fire during an Ease return glide** (the style switched on a running loop in its last beat) lets the moving knobs go where they are, then restores them in one call at the loop point or the restart. That is never a leak and never a skipped restore.

### METRICS
- Builds: cold-ish full build at base, then incremental. 4 full serial ctest runs (613 / 614 / 619 / 620).
- Live runs, 6 in all, every one under the lock: witness RED and GREEN, probe-routines RED and GREEN, and the three final probes in one hold. One queued witness was cancelled before it got the lock, and restarted against an app snapshot.
- Tool calls: about 110.

### KNOWLEDGE CONTEXT
- Tools used: grep only (no KNOWLEDGE_TOOLS block).
- Impact authority: grep, which is not authoritative, so I deleted nothing except the two clip/audio stop calls the ruling removes.
- Risk level: NORMAL.
- Dependencies discovered: `Binding::Action::GlobalPlayPause` already stops the audio file, which makes the GlobalStop decision safe.
- Queries made: 0.

### PACKET QUALITY
- Clarity: HAD_TO_INFER.
  - The fence names `src/recording/Routine*`, but the Routine model is `src/model/Routine.h`.
  - "any binding labelled as the transport Stop" -- the GlobalStop binding stopped the AUDIO transport, not clips.
  - When a style change takes effect on a running routine was not specified.
- Missing context: no REST route reaches the TopBar Stop, so a committed live row for Item 1 is impossible inside the fence. The scratch hook was the only live route.
- Unused context: routine-ux design-final PART 1 beyond the Ease/Jump and Stop paragraphs.
- Self-assembly: LEGACY (no DEPARTMENT field).
- Self-brief files: `.harmony/notebook.md` headings, the routines entries (useful: the juce_core-only header rule and the scratchpad sharing); `plan3-final.md` §4; `tempo-glide.md`; `BORIS_DECISIONS.md` Playback Behaviour; design-final PART 1 and ADDENDUM. All were present and current.

### STATUS
DONE_WITH_CONCERNS. Every item is built and committed, with RED and GREEN evidence, and all gates are green. The concerns are the three fence departures, the GlobalStop audio-stop removal, and that Item 1's behaviour has no permanent guard.

### NEXT ACTION
Harmony:
1. Gate `lane/routines-followup-0926b` at `41be0ea` plus the report commit.
2. Send the Reviewer ISSUES 1-3.
3. Decide whether the bind-overlay label becomes "Stop routines".
4. Merge.
