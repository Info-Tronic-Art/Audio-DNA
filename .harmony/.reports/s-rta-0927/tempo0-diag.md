## BUILDER REPORT -- tempo0-diag (s-rta-0927)

STATUS: DONE_WITH_CONCERNS
RESULT: VERIFIED. A take's "start" tempo entry can have bpm 0, and this is a race that already existed before the beat-clock change. `/api/set_bpm` is applied by the analysis thread on its next hop (up to about 10.7 ms later), but the recorder's first tick after `/api/perf/record` can land before that hop publishes. In that case the first snapshot the recorder reads still says bpm 0 (no lock yet). Rate in the witness sequence: 4 of 34 runs (12%). By build: main 1/15, pre-beat-clock 2/14, instrumented main 1/5. A sibling race lost 9 of 34 runs (26%): Record's arm-time bar-position read happens before the tempo is applied, so the take's bar grid is "unknown". Effect on replay: timing is barely touched (the beat is frozen for 9-12 ms, then continuous). The real harm is routine cutting. Slicing from beat 0, or from "bar 1" on the unknown grid, is refused with "this stretch has no beat; the tempo was unknown while it was recorded". Fix proposal is below; no fix was committed.
FACTS:
- Instrumented trace of a bpm-0 run (`/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/92434c6b-4aab-4138-83af-d5f37c430398/scratchpad/tempo0/runs/instr-03/app.err`), in ms after the set_bpm HTTP request. http_record arrives at +9.62. The message thread handles set_bpm late (+10.26) and runs arm at +10.31 (snapBpm 0). The recorder's first tick is at +11.09 (snapBpm 0, anchors=1). The analysis thread applies the request at +11.10 and publishes bpm 120 at +11.14. The race was lost by 0.06 ms. The next tick (+19.89) writes the "lock" anchor.
- The tempo is applied asynchronously. `src/api/ApiServer.cpp:770` callAsync -> `src/MainComponent.cpp:5160` applyTempoCommand("link") -> `src/analysis/BPMTracker.cpp:74` (the request is applied only at the start of the next hop's runPipeline). The recorder's first tick takes `snap.bpm` as-is for "start": `src/recording/RecorderClock.cpp:26`, called from `src/recording/RecorderHost.cpp:438`.
- Tallies are in `/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/92434c6b-4aab-4138-83af-d5f37c430398/scratchpad/tempo0/analysis.json` and `batch.log` (same dir). bpm-0 runs: main-11, pre-02, pre-06, instr-03. Each is followed by a "lock" at 120 at t = 0.0107 / 0.0097 / 0.0099 / 0.0088 s. Start samples: 152064 / 149504 / 151040 / 153600.
- Attribution: the same race reproduces on the pre-beat-clock build 1636785 (2/14). Post-beat-clock code (main + instr) is 2/20. Fisher exact p = 1.0, so there is no evidence of any difference. The "start" branch of `RecorderClock::tick` reads `snap.bpm` exactly as before; the beat-clock diff did not touch it (`git diff 1636785 6dfa41c -- src/recording/RecorderClock.cpp`). A bpm-0 witness take from before the beat clock is still on disk: `witness-base-190906` (2026-09-26 19:09), with a "lock" at 120 at t = 0.043 s.
- Replay consequence, reproduced on the REAL takes by a scratch Catch2 test (`/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/92434c6b-4aab-4138-83af-d5f37c430398/scratchpad/tempo0/test_tempo0_scratch.cpp`, log `scratch-test.log`, 15/15 assertions pass):
  - The bpm-0 take refuses a slice of beats [0,2), and also "bars 1..1", with "this stretch has no beat; the tempo was unknown while it was recorded". The 120 take accepts [0,2), and so does the bpm-0 take for [1e-6, 2).
  - The cause is the second loop of `src/recording/RoutineSlice.cpp:66`: the "start" anchor sits at beat 0 with bpm 0.
  - The bpm-0 take has startBeatInBar = -1, so `takeBeatOfBar(bar 1)` = 0.
METHOD: Ran the witness sequence under the live lock, one run per acquisition, interleaving three builds so machine load hit them equally: main, pre-beat-clock 1636785, and main plus temporary stderr timestamps. Each run was captured with the final take.json. I read the replay and slice code, then reproduced the consequences in a scratch test that loads the two real takes. No fix was written.
CONFIDENCE+VERIFY: High on the mechanism (a timestamped trace of a failing run, plus 4 passing traces that show the margin). High on the attribution (reproduced on 1636785). Medium on the exact rate: 34 runs, the 95% CI for 4/34 is about 3-27%. VERIFY: re-run `/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/92434c6b-4aab-4138-83af-d5f37c430398/scratchpad/tempo0/run1.sh <App> <name> <outdir>` under the lock and read `<outdir>/take-final.json` tempoMap[0].bpm and meta.startBeatInBar. After a fix, 20 runs should give 0 bpm-0 and 0 unknown startBeatInBar.
UNKNOWNS/NOT-DONE: 34 runs, not the planned 40 (+instr). I stopped at 00:04 because of lock starvation: the followups lane held the lock about 20-25 min per turn, and I got one run between turns. Counts are main 15/20 and pre 14/20. The 2 bpm-0 runs on main were enough for the mechanism and the attribution. The fix is not implemented or tested. The Sample-clock fallback defect named in NUANCE is not fixed.
NUANCE: The bpm-0 "start" entry is the designed "unmetered" state (`RecorderClock.h:30-32`). It is correct when the tracker really is unlocked at Record, as in the 1.5-1.9 s unmetered starts of `onsetrender` and `step3gate1`. The bug is that a tempo command sent BEFORE Record is not yet visible at t = 0. So the fix belongs on the arm/first-tick side, not in `checkMetered`. The main binary was rebuilt at 21:47 (f630336, source-defects merge) during the batch: main-01..04 ran on the 6dfa41c build and main-05..15 on the f630336 build. That merge touches only composition-load code in MainComponent.cpp, none of the tempo/record path; main-11 (the bpm-0 run on main) ran on f630336. A pre-existing defect that is not caused by bpm 0: `TempoMap::sampleAt` with a single anchor has rate 0. The 120-bpm take's sampleAt stays at 150016 for every t. With the 12-ms "lock" pair, the bpm-0 take extrapolates at 85,050 samples/s. Both are wrong, but only on Program's Sample-clock fallback for gestures without stamps (`src/recording/Program.cpp:451`).
HANDOFF-NEEDS: none
INBOX-RECHECK: none

### SUMMARY
The bpm-0 start is a pre-existing race between the asynchronous tempo command (applied on the analysis thread's next hop) and the recorder's first tick (and, separately, Record's arm-time bar-position read). It is not caused by the beat-clock change. It does not break replay timing (at most one frozen tick of 8-12 ms, absorbed as an offset). It does make the take un-sliceable from beat 0 or bar 1, and in 26% of witness runs it leaves the bar grid unknown.

### Rate (witness sequence: production launch, health, +2 s, set_bpm 120, Record, 1.5 s)

| build | runs | start bpm 0 | startBeatInBar unknown |
|---|---|---|---|
| main (6dfa41c x4, f630336 x11) | 15 | 1 (main-11) | 4 (02, 09, 11, 12) |
| pre-beat-clock 1636785 | 14 | 2 (pre-02, pre-06) | 3 (02, 06, 07) |
| main + stderr timestamps | 5 | 1 (instr-03) | 2 (01, 03) |
| **total** | **34** | **4 (12%)** | **9 (26%)** |

Plus Harmony's own runs: 1/4 today (harmony-tempo) and 1 of the historical witness takes (`witness-base-190906`, 2026-09-26, before the beat clock). Median set_bpm->record curl pair time: 29 ms for every build.

### Mechanism (VERIFIED by trace)
Instrumented timelines (ms after the set_bpm HTTP request; `analysis.json` field `t`):

| run | setbpm_msg | http_record | arm | tempo_apply -> pub 120 | first rec tick | margin | start bpm | bar grid |
|---|---|---|---|---|---|---|---|---|
| 01 | 0.05 | 9.04 | 9.11 (snap 0) | 9.86 -> 9.90 | 23.71 | +13.8 | 120 | unknown |
| 02 | 5.95 | 11.61 | 11.68 | 6.03 -> 6.08 | 21.96 | +15.9 | 120 | known |
| 03 | **10.26** | 9.62 | 10.31 (snap 0) | 11.10 -> 11.14 | **11.09 (snap 0)** | **-0.06** | **0** | unknown |
| 09 | 0.05 | 10.15 | 14.98 | 9.64 -> 9.70 | 15.83 | +6.1 | 120 | known |
| 13 | 0.04 | 11.16 | 11.25 | 9.63 -> 9.69 | 19.34 | +9.7 | 120 | known |

- **Start bpm 0.** This happens when no analysis hop publishes between the set_bpm message and the recorder's first tick. It needs the message thread to be about 10 ms late: both HTTP messages queue, then run back to back, and the 120 Hz tick fires within 1 ms. The hop phase then decides the outcome. Run 03 shows exactly this sequence.
- **Unknown bar grid.** This happens when arm (`src/MainComponent.cpp:5285-5287`, which reads the FeatureBus once and needs trackerState LOCKED) runs before the first hop after set_bpm. Arm comes about 9-15 ms after set_bpm, so this race is lost about half the time the hop falls late (runs 01 and 03).
- Why a human never hits it: nobody taps or types a tempo and presses Record within 10 ms. Scripted and remote clients do: REST (the witness, the probes), OSC, and possibly a MIDI pad bound to both actions. A take started while the tracker is truly unlocked is a different, correct case.

### Replay consequence (VERIFIED on the real takes; scratch test)
- **Take playback timing:** negligible.
  - `TempoMap::beatAt` is frozen for 0-12 ms, then runs at 120. `tAt(beat)` is shifted by +12 ms: bad take `tAt(1)` = 0.51204 vs good 0.5.
  - Beat stamps carry a constant one-tick offset. RecorderClock test over 20 s at 120 Hz: max |beat difference| = 0.0167 beat, constant, no drift.
  - Playback drives on Wall or Sample stamps (`src/recording/RecorderHost.cpp:697`). It consults the tempo map only for stampless gestures.
- **Routine cutting (the real harm):**
  - `sliceRoutine` refuses any range starting at beat 0: "this stretch has no beat; the tempo was unknown while it was recorded". This comes from `checkMetered`'s second loop, `src/recording/RoutineSlice.cpp:66-68`.
  - The bpm-0 take is usually also bar-grid-unknown. `takeBeatOfBar(bar 1)` = 0 (`src/recording/RoutineSlice.cpp:457-458`), so "Save bars 1..N as a routine" is refused, and the notice says "bars counted from the start of the take" (`src/MainComponent.cpp:5606-5607` on f630336).
  - A range starting even 1e-6 beat later is accepted.
- **Bar grid unknown (26%, even when start bpm is 120):** bar numbers count from the take's start instead of the musical bar. That is the "bars 33 to 40" feature (s-rta-0926 plan 3.6) silently degrading.

### Fix proposal (not committed; Harmony routes it)
**A (recommended): Record's t = 0 waits for the analysis hop that is sure to contain every command sent before Record.**
1. `src/MainComponent.cpp:5285-5287` (perfRecord): stop computing startBeatInBar from the arm-time snapshot. Pass `firstTickNotBefore = armSnap.timestamp + 2 * AnalysisThread::kHopSize` (`src/analysis/AnalysisThread.h:47`) in ArmOptions (`src/recording/RecorderHost.h:113`, next to startBeatInBar). Two hops are needed because the hop in progress at arm may have checked the request flag before it was posted (`src/analysis/BPMTracker.cpp:74`). The next hop cannot miss it.
2. `src/recording/RecorderHost.cpp:438`: while recording, skip `clock_.tick` until `snap.timestamp >= firstTickNotBefore_`. Add a wall fallback (tick anyway after about 50 ms) so a stalled or absent device never leaves a take without t = 0.
3. At that first tick, set `armedStartBeatInBar_` (`src/recording/RecorderHost.h:316`) from the same snapshot, only if trackerState == LOCKED. The provisional save at arm keeps -1 (it already has an empty tempo map). Bonus: startBeatInBar now describes the same instant as beat 0, instead of an instant 0-25 ms earlier.
- Cost: t = 0 moves from 0-8 ms after arm to about 11-21 ms after arm. This fixes REST, OSC and MIDI alike, and both symptoms.
- Tests:
  - RecorderHost unit: first tick at ts < notBefore with bpm 0 -> no anchor. Tick at ts >= notBefore with bpm 120 -> "start" at 120 and startBeatInBar known.
  - Wall fallback: fires with no hop progress.
  - Live: this witness x20 -> 0/20 bpm 0 and 0/20 unknown bar grid.

**B (smaller, REST-only):** `src/api/ApiServer.cpp:755-776` handleSetBpm waits on the HTTP thread (poll the FeatureBus, 1 ms steps, cap 50 ms) until a published snapshot has the new bpm, then answers. This fixes the witness and every REST script, but not OSC or MIDI. The same rule would be needed on `/api/resync`.

**Not recommended:** having `checkMetered` ignore a zero-beat-length unmetered segment (`src/recording/RoutineSlice.cpp:61-68`). That hides the symptom for slicing only and leaves the bar grid unknown.

### FILES CHANGED
- `.harmony/.reports/s-rta-0927/tempo0-diag.md`: this report (the only file written in the main checkout).
- Worktree `.claude/worktrees/rta0927-w12` (temporary, reversed): stderr timestamps in `src/api/ApiServer.cpp`, `src/MainComponent.cpp`, `src/analysis/AnalysisThread.cpp`, `src/analysis/BPMTracker.cpp`, `src/recording/RecorderHost.cpp`, plus a scratch test target in `tests/CMakeLists.txt`. The full diff is saved at `/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/92434c6b-4aab-4138-83af-d5f37c430398/scratchpad/tempo0/instr-final.diff`. I reversed it with `git apply -R` and returned the worktree to detached 1636785. `git status` shows only `?? build-pre/`.

### TESTS
- Scratch `test_tempo0_scratch`: 3 cases, 15 assertions, all PASS. They cover the real-take slice refusal, the real-take tempo-map queries, and the RecorderClock bpm-0-first-tick offset. Output: `/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/92434c6b-4aab-4138-83af-d5f37c430398/scratchpad/tempo0/scratch-test.log`.
- Live: 34 witness runs plus 1 smoke run; results in the rate table above.
- ctest was not run (nothing was changed in the product).

### SLIM CHECK
Nothing to cut: no product diff. The scratch instrumentation is already reversed.

### ISSUES
- Lock starvation: 5 acquisitions waited 1040-2420 s each (`lock.log`). The followups lane and Harmony's battery took the lock inside my 45 s back-off again and again, so the run count stopped at 34.
- Batch 2's helper reused a global `i`. As a result the instrumented runs after round 3 are named instr-09 and instr-13, not 08 and 12. This affects labels only.
- The w12 `build-pre/` directory now holds the INSTRUMENTED main build (incremental rebuild). The clean pre-beat-clock bundle used for the A/B is the copy at `.../scratchpad/tempo0/apps/pre/Audio-DNA.app` (sha256 3d987391...). Delete `build-pre/` or re-use it knowingly.

### RISKS
- Fix A moves every take's t = 0 by about 10-20 ms. Any probe that checks a start-offset window tightly (probe-routines windows, step3 T2 alignment mean-offset [0,60] ms) should be re-run after the fix.
- If only the bpm-0 half is fixed (for example, by deferring the start anchor alone), the more frequent unknown-bar-grid race stays.

### PACKET QUALITY
- Clarity: CLEAR
- Missing context: none. The pre-beat-clock worktree was ready to build. I pointed FetchContent at `build/_deps/*-src` (read-only use), so nothing had to be downloaded.
- Unused context: none
- Self-brief files: CLAUDE.md (thread model, useful); `.harmony/.reports/s-rta-0927/beatclock.md` (useful for the clock semantics); the witness script and logs in scratchpad/gate (essential).

### KNOWLEDGE CONTEXT
- Tools used: grep. Impact authority: grep (not authoritative); no deletion or claims of dead code. Risk level: NORMAL. Queries: n/a.

### Rig compliance
- Every launch used `open -g --stdout/--stderr <App>` (production, no --test-mode). No Output window was opened, and there was no lldb, full-screen capture or synthetic input.
- Every quit was the osascript quit. `PKILL needed` appears 0 times in any run.log.
- The lock was taken once per run and released after each run (`lock.log`), with a 45 s back-off. The lock is NOT held now: `/tmp/audiodna-live.lock/owner` = followups. No Audio-DNA I launched is running (the one running is w11's build-lane).
- Takes: my 35 folders (`tempo0-smoke-01` plus 34 `tempo0-{main,pre,instr}-NN`), listed in `created-takes-uniq.txt`, matched the on-disk `tempo0-*` set exactly and were deleted. No other take was touched; Harmony's `harmony-tempo*` takes remain. The two real takes used by the test were COPIED to `.../scratchpad/tempo0/fixtures/`.
- The main checkout was not edited apart from this report.

### STATUS
DONE_WITH_CONCERNS. The mechanism, attribution and consequence are verified. The concerns are the run count (34 of the planned 40, because of lock starvation) and that the fix is only proposed.

### NEXT ACTION
Harmony: route Fix A (or B as a stopgap) to a lane. Add a probe row: witness x20 -> 0 bpm-0 and 0 unknown startBeatInBar. Consider a notebook line: "a tempo command before Record is visible to the recorder only after the next analysis hop".
