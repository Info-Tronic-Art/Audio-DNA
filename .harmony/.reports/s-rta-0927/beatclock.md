# Lane beatclock (s-rta-0927) -- the routine beat clock no longer loses time across a stall

STATUS: DONE
RESULT: all six plan-beatclock.md commits landed on `lane/beatclock-0927` (base dc7adf9). Every new test and probe row was RED first on the pre-fix code or app, then GREEN on this lane's build. Full serial ctest passes 742/742. The regression probes pass on the final build (routine-display 16/0, resync 16/0, manual-bpm 22/0, downbeat-level 14/0, step3 94/0 incl. the 4 T2 alignment rows, tempo witness).
FACTS:
- A 550 ms message-thread stall moves `clockBeat` by 2 x elapsed seconds (error -0.022..+0.019 beat, tolerance 0.10) on both the commit-4 and the final app, 3/3 runs each. On the commit-1 app (old reader) the error was -0.980. Source: `beatclock-evidence/batchA.log`, `batchB.log`, `batchC1.log`.
- probe-routines row 7s (the grid through a 550 ms stall): RED on the commit-1 app, marks at +3.05 / +5.03 / +7.07 s (3 FAIL, 102/3). GREEN 105/0 x3 on the commit-4 app and x3 on the final app. Source: same logs.
- ctest: commit 2 734/734; commit 3 735/741 (exactly the 6 named cases); commit 4 741/741; commit 5 741/742 (exactly the loop case); commit 6 742/742. Source: `beatclock-evidence/ctest-c*.summary`.
METHOD: followed the plan's order: stall hook and probes first, then the snapshot field, then RED tests, then the fix, then RED, then fix again. I snapshotted the app bundle at commits 1, 2, 4 and 6 so each live RED/GREEN ran on the exact binary of its commit. Live runs were in 4 lock batches, and I released the lock between batches.
CONFIDENCE: high for the stall-loss fix (unit + live, RED/GREEN on the same assertions). VERIFY: re-run `.harmony/probe-beatclock.sh` and `probe-routines.sh` (BEATCLOCK_BUILD_DIR / ROUTINES_BUILD_DIR=build-lane, ROUTINES_RECORD_PAUSE=1.8) under the lock; `ctest --test-dir build-lane -j1`.
UNKNOWNS: verify item (c) is proven only at compile level. `ApiServer.cpp` compiled without the define has 0 route strings, but I did not build a full `AUDIODNA_BUILD_TEST_SERVER=OFF` app and see a live 404. ctest was not run by itself at commit 1, because no test target links `ApiServer.cpp`. Class 2 (a pause in the audio device's clock) is filed, not fixed.
NUANCE: CLAUDE.md on dc7adf9 already indexed pitfalls 40 and 41, so only line 42 was added. The `grid` helper in probe-routines gained an optional TAG argument (labels only; the analysis is unchanged).
HANDOFF-NEEDS: Boris decision 2e (catch up vs skip after a freeze; default catch up, quoted below). Harmony: merge `lane/beatclock-0927`, and renumber Pitfall 42 if another lane lands a 42 first.
INBOX-RECHECK: none

## Commits (one per plan item)

| # | commit | plan item | ctest after |
|---|---|---|---|
| 1 | d183070 | probe(beatclock): TEST-ONLY `POST /api/debug/stall_message_thread`; `.harmony/probe-beatclock.sh`; probe-routines row 7s; inventory note | (not run alone -- no test links ApiServer.cpp; see c2) |
| 2 | 19f8afc | feat(analysis): `FeatureSnapshot::totalBeatCount`; BPMTracker counts wraps + second-half realigns; `/api/bpm` + both inject paths; 5 `[bpm][beatcount]` tests | 734/734 |
| 3 | 062b750 | test(recording): RED 5.2 a-d, 5.3 a-c + rig updates | 735/741 (exactly the 6 named) |
| 4 | 59538a6 | fix(recording): RecorderClock integrates `totalBeatCount + beatPhase`; RoutineEngine Beat edge from the count | 741/741 |
| 5 | 9bfad4b | test(recording): RED 5.3e (loop folds one cycle per tick) | 741/742 (exactly the 1 named) |
| 6 | 07d15a0 | fix(recording): loop folds every whole cycle in one tick, one restore; Pitfall 42, CLAUDE.md index 42, recording.md, notebook | 742/742 |
| 7 | (this report) | lane report + evidence | -- |

Files: `src/analysis/{FeatureSnapshot.h,BPMTracker.h,BPMTracker.cpp,AnalysisThread.cpp}`, `src/recording/{RecorderClock.h,RecorderClock.cpp,RoutineEngine.h,RoutineEngine.cpp}`, `src/api/{ApiServer.cpp,ApiServer.h}`, `src/test/TestServer.cpp`, `tests/{test_bpm_stabilization,test_take,test_routine_engine}.cpp`, `.harmony/{probe-beatclock.sh (new),probe-routines.sh,APP-INVENTORY.md,notebook.md}`, `docs/claude/{analysis,architecture,recording,pitfalls}.md`, `CLAUDE.md` (+1 line, 22,590 B).
Untouched (the fence): src/output/*, OutputWindow, MenuBarModel/TopBar output code, and the Renderer/CompositorEngine render paths. Also untouched: the beat-crossing wrap readers (Autopilot.cpp:44, Renderer.cpp:407, MainComponent.cpp advanceSlideshow/beatSyncRandomize) and Take/Lane/Program/Player.

## RED -> GREEN, verbatim

### Unit
5.1 `[bpm][beatcount]` (commit 2). Compile-RED: the new test file compiled against commit 1's `BPMTracker.h` (`beatclock-evidence/red51.log`, `scripts/red51.sh`):
```
tests/test_bpm_stabilization.cpp:1293:21: error: no member named 'totalBeatCount' in 'BPMTracker'; did you mean 'totalBarCount'?
... (20 errors) ... RED51 compile exit 1
```
GREEN at commit 2: `191-195 totalBeatCount ... Passed` (5 cases); `100% tests passed, 0 tests failed out of 734`.

5.2 / 5.3 a-c (commit 3, RED against the old reader). `ctest`: `99% tests passed, 6 tests failed out of 741`
```
246 - RecorderClock: a 1.1-beat tick gap loses nothing (Failed)                         2.10000000149011612 == Approx( 3.1 )
247 - RecorderClock: a 0.7-beat tick gap that begins at phase 0.5 loses nothing (Failed)  2.5 == Approx( 3.2 )
248 - RecorderClock: a 0.6-beat publication burst between two ticks adds 0.6 (Failed)    2.69999998807907104 == Approx( 3.3 )
470 - RoutineEngine: position is exact across a 1.1-beat tick gap ... (Failed)           0.60000002384185791 == Approx( 1.6 ); firedLanePoints 0 == 1
471 - RoutineEngine: a Beat-quantized start survives a tick gap ... (Failed)             "pending" == "running"
472 - RoutineEngine: several points inside one tick gap all fire ... (Failed)            ev.size() 0 == 3
```
5.2d (realign semantics) passed before AND after, as the plan requires. GREEN at commit 4: `100% tests passed, 0 tests failed out of 741`.

5.3e (commit 5, RED against the commit-4 engine): `99% tests passed, 1 tests failed out of 742`,
`473 - RoutineEngine: a looping routine folds every whole cycle a tick gap covers at once, with one restore (Failed)`.
At the resume tick: `cycle 2 == 4`, `position 8.0 == Approx( 0.0 )`, `firedLanePoints 2 == 1`. After `runTo(17)`: `firedLanePoints 4 == 2` and `Touch op1 4 == 2` (two more folds). GREEN at commit 6: `100% tests passed, 0 tests failed out of 742`.

### Live (production port 7070, `open -g`, lock held per batch; load 2.7-5.6 with 0 compilers during every timing row)
RED, batch A, 16:41-16:43 (`beatclock-evidence/batchA.log`):
- Pre-change main app (`build/`): `SKIP: no TEST-ONLY hook in this binary ({"ok": false, "http": 404}) -- b2/b3 not run`; `FAIL  b1: /api/bpm has no totalBeatCount field (absent -- a pre-beat-clock binary)`; `3 PASS / 1 FAIL`
- Commit-1 app (hook, old reader):
  `FAIL  b2: a 550 ms message-thread stall -- clockBeat advanced 0.643 beats over 0.811 s of wall (expected 1.622 at 120 BPM; error -0.980, tolerance 0.10; ...)`
  `FAIL  b3: a 350 ms message-thread stall -- clockBeat advanced 0.491 beats over 0.602 s of wall (expected 1.205 at 120 BPM; error -0.714, tolerance 0.10; ...)`; `3 PASS / 3 FAIL`
- Commit-1 app, probe-routines (ROUTINES_RECORD_PAUSE=1.8): `(7s: stall requested at +1.031 s)`,
  `FAIL  7s: L0 on column 1 first at +3.05 s (expected +2.6 +/- 0.35)`, `FAIL  7s: C1 Brightness ~0.9 first at +5.03 s (expected +4.6 +/- 0.35)`, `FAIL  7s: L0 opacity ~0.9 first at +7.07 s (expected +6.6 +/- 0.35)`; `102 PASS / 3 FAIL`

GREEN, commit-4 app, batch B, 16:45-16:50 (`batchB.log`):
- probe-beatclock x3: `6 PASS / 0 FAIL` x3. b2 errors -0.004 / -0.006 / -0.022; b3 errors +0.004 / -0.011 / +0.018. b1 `advanced 6 over 3.00-3.01 s` x3.
- probe-routines x3: `105 PASS / 0 FAIL` x3 (98 + 7 rows of 7s). 7s marks at +2.49/+4.49/+6.49, +2.55/+4.53/+6.55, +2.50/+4.51/+6.52.
- verify (d): `PASS  (d) 73 non-zero clockBeat deltas over 3 s, each a multiple of one hop (0.021333 beat): worst residue 5.70e-08 beat`. The reader still follows the tracker hop by hop and does not integrate wall time.
- tempo witness: `PASS  tempo witness: take.json 1.5 s after Record -> tempoMap [{"t": 0.0, "beat": 0.0, "sample": 254464, "bpm": 120.0, "why": "start"}]`

GREEN, final build (build-lane = commit 6), batch C1, 16:53-16:59 (`batchC1.log`):
- probe-routines x3: `105 PASS / 0 FAIL` x3 (rows 8 / 8g / 11j, the k = 1 loop, unchanged). 7s marks at +2.56/+4.52/+6.54, +2.53/+4.51/+6.50, +2.55/+4.52/+6.51.
- probe-beatclock x3: `6 PASS / 0 FAIL` x3. b2 errors -0.006 / +0.019 / -0.007; b3 errors +0.007 / -0.001 / -0.003.
- verify (d): `worst residue 6.05e-08 beat` over 72 deltas. Tempo witness PASS (`"bpm": 120.0, "why": "start"`; after stop, 1 anchor `['start']`).

Regressions, final build, batch C2, 17:49-17:56 (`batchC2.log`): probe-routine-display `16 PASS / 0 FAIL` (phase 1, no hook); probe-resync `16 PASS / 0 FAIL`; probe-manual-bpm `22 PASS / 0 FAIL`; probe-downbeat-level `14 PASS / 0 FAIL`; probe-step3 `94 PASS / 0 FAIL`, including `PASS  T2 alignment: drift within +-6.9200ms (... drift=-0.51ms stderr=2.96ms)`, `p95 jitter <=15ms (11.05 ms)`, `mean offset within [0,60]ms (44.95 ms ...)`, `matched markers cover >=90% ... (100.8%)`.

### Verify items (plan section 6)
- (a) `static_assert(offsetof(FeatureSnapshot, totalBeatCount) == 324)` and `sizeof == 384` compile. FeatureBus.h is untouched.
- (b) `grep -n "new \|malloc\|mutex" src/analysis/BPMTracker.cpp` finds 3 matches before and after, all comments (lines 178, 443, 510). No allocation or lock was added. The analysis loop adds one line (the publish).
- (c) Compile-level only (`beatclock-evidence/verify-c.log`): the AudioDNA target's exact `ApiServer.cpp` command gives `ON route string count in ApiServer.o: 1`. With `-DAUDIODNA_TEST_SERVER=1` removed it gives `OFF route string count in ApiServer.o: 0`. I did not build a full OFF app or see a live 404.
- (d) PASS, above. (e) probe-routines x3 after commit 4 and x3 after commit 6, all 105/0. (f) ctest as in the commits table.

## Drift from the plan (each disclosed)
- D1. CLAUDE.md on dc7adf9 already indexes pitfalls 40 and 41 (the plan said the index stops at 39). I added only line 42. Pitfall 42 is appended after 41 in `docs/claude/pitfalls.md`. On 17:10 no other lane branch or worktree adds a 42.
- D2. probe-routines `grid` takes an optional 4th argument TAG (default `grid`), so row 7s prints `7s:` rather than a second set of `grid:` rows. Marks, windows and tolerances are byte-for-byte row 7's.
- D3. Row 7s starts its 8.9 s sampler right after `T1s` (the "running" anchor), as row 7's sampler at `:699` starts after T1. It does not start right after the fire. Starting at the fire would end the sample up to 2 s early and lose the +6.6 mark and the idle tail.
- D4. probe-beatclock on a binary without the hook prints `SKIP` for b2/b3, runs b1 and the teardown, and exits by its FAIL count. On the pre-change app that is exit 1, because b1 FAILs (field absent), which is that binary's RED. The plan said "exit 0 after b1". On a fixed OFF build b1 passes, so it exits 0.
- D5. The 40-minute clock test now derives the phase from the same `trueBeats` double as the count (`phase = trueBeats - floor(trueBeats)`), not from the separate `fmod` accumulator. Otherwise count and phase could disagree by one ulp at a wrap and read as a realign. It is still the same 40-minute, 127.97-vs-128 BPM assertion set, all passing.
- D6. The 5.1 tests use the file's own `WithinAbs` matcher and its existing `quietHop` helper instead of `Approx`, with the same tolerances (1e-4). 5.1d uses a [0.60, 0.66] phase window; its p0 was ~0.63 as planned.
- D7. ctest was not run by itself at commit 1 (see UNKNOWNS). 5.1's RED is a syntax-only compile against commit 1's headers, the "compile failure" the plan names, not a ctest run.
- D8. Main moved dc7adf9 -> 0766f06 (docs-only work log) during the lane. The branch is based on dc7adf9 as dispatched.

## Boris flag (plan 2e, verbatim)
**In plain words:** when the app freezes for a moment (a deck load, a hiccup) while a routine is playing,
the routine now CATCHES UP -- every move that should have happened during the freeze happens the instant
the app comes back (all at once, in the recorded order), and everything after lands back on the beat. If
the freeze was longer than a looping routine's whole cycle, it lands in the right cycle with one restore
and does not replay the skipped cycles. The alternative is to SKIP the missed moves and continue from
where the beat is now. **Default chosen: catch up (skip nothing)** -- a skipped clip change can leave the
wrong picture on screen for bars, while a late one is a half-second stumble; and skipping would be NEW
behaviour (today nothing is skipped either -- it is only late). If Boris prefers skipping: one line in
`RoutineEngine::tick`, `r.player->seek(pos, *r.sink)` before the `advanceTo` when the tick's beat delta
exceeds a threshold (`Player.h:52-66` documents `seek` as exactly that opt-out). FLAG FOR BORIS.

Also open (plan 2d, not in this lane): "when the audio device hiccups for a fraction of a second while you have
tapped/typed a tempo (or Link), should the beat keep going as if nothing happened, or pause with the audio?"

## Found, not fixed
- Class 2 (reppre1): the audio device's clock pausing moves the whole beat grid (bars, quantised clips, routines together). It is a tracker-level decision, filed as plan 2d / commit 7 (needs Boris).
- Beat-crossing wrap readers still read `beatPhase < last - 0.5` and lose one crossing under a stall of 0.5 beat or more: `Autopilot.cpp:44`, `Renderer.cpp:407` (projectM playlist), `MainComponent::advanceSlideshow` / `beatSyncRandomize` (30 Hz). They are listed in Pitfall 42 as the follow-up.
- The 0.5 decision moved from the reader to the writer (risk 1). A Tap or Resync within about one tick of the half-beat point can land a running routine's continuity one beat differently than before. Accepted by the plan and named in the pitfall.

## Screen safety
- Every launch was `open -g` (probes and `scripts/witness.sh`). No Output window was opened by any path, and each run's Quartz full-window-list witness reads `0 Audio-DNA Output windows`.
- No full-screen capture. The only window shots are probe-routine-display's own window-only Quartz shots (phase 1, runs/C2-rd).
- No synthetic input, no lldb/debugger. `test_output_window_level.py` and `tests/visual/` were not run.
- No temporary env-var hook was added: the lane diff has no `AUDIODNA_DEBUG`. The only new hook is the plan's committed TEST-ONLY stall route.
- Quit was graceful osascript every time; pkill was never needed. Lock taken and released 4 times (`beatclock-evidence/lock.log`, >= 45 s between my release and re-acquire). The last batch waited 2963 s for the lock.
- No Audio-DNA I launched is running.
- Takes left in `~/Documents/Audio-DNA/Takes`: 7 `probe-routines-*`, 2 `beatclock-witness-*`, and probe-step3's own.
- `.venv` symlink removed. `build-lane/` kept (untracked, as in the other lanes).

## Boris checks (in the app)
1. Fire a looping routine, then load a deck while it plays. Its moves should land back on the beat right after the load, not half a second late for the rest of the take.
2. Record a take while loading a deck mid-take, then play the take back. The moves after the load should sit on the beat grid.
3. The Resync / Tap buttons and a typed BPM behave exactly as before.

## PACKET QUALITY
- Clarity: CLEAR (the plan is file:line exact; two stale citations, D1 and the `:699` sampler wording in D3)
- Missing context: how to run the "tempo witness" (no script in the repo). I wrote `beatclock-evidence/scripts/witness.sh` from the work-log description ("take.json 1.5 s after Record -> tempoMap start anchor bpm 120").
- Unused context: none
- Self-brief files: CLAUDE.md, .harmony/notebook.md (s-rta-0926 routine-grid, routines-timing and c1-state-fix entries useful), routines-timing.md (skimmed; the plan carried what was needed).

## KNOWLEDGE CONTEXT
- Tools used: grep. Impact authority: grep (not authoritative). I took a conservative posture: every `RecorderClock` user (RecorderHost, RoutineEngine, PerformanceRecorder) and every `beatPhase` writer (AnalysisThread, TestServer, ApiServer inject) was read. Risk level: NORMAL. Queries: n/a.
