# LANE tempo -- s-rta-0928 take start race (plan-tempo.md + HARMONY ADOPTION A1-A8)

STATUS: DONE
RESULT: The exact handshake is implemented as adopted. A tempo / Tap / Resync command sent just before Record now lands in the take's "start" anchor and bar grid. Live on the lane app: W1 PASS, W2 20/20, W3 20/20, W4 0, W6 witness x20 0/20 + 0/20. On the unmodified app: W1 FAIL, W2 5/20, W3 0/20. ctest 797/797 serial. Every section-7 probe is green, with 0 "take start:" lines.
FACTS:
- E1 and E2 RED on unmodified 6db8d67 with the plan's values; verbatim in the RED section (`build-lane/tests/test_tempo_start`, scratch `ctest-red-commit1.txt`).
- test_tempo_start: 13 cases, all GREEN (`All tests passed (311 assertions in 13 test cases)`).
- Full ctest: `100% tests passed, 0 tests failed out of 797` (`ctest --test-dir build-lane -j1`, 10:22:46).
- Live RED on main's build (byte-identical copy): W1 FAIL, W2 5/20 passed (15 failed), W3 0/20 (`scratchpad/tempo/red-run1.log`).
- Live GREEN on the lane app:
  - 3 stall-assisted runs, 1 natural-timing run and 1 run on the final relinked binary: every run W1 PASS, W2 20/20, W3 20/20, W4 0;
  - W6 x20: start-bpm-0 0/20, unknown grid 0/20 (`green-batch1.log`, `green-w6`, `green-final.log`).
- Section-7 re-runs, every one with 0 FAIL and 0 "take start:" lines: routines (pause 1.8 and 0) 105/105 each, beatclock 6, downbeat 14, resync 16, manual-bpm 22, step3 94, finalize-loop 8 (40 cycles, 0 truncations), onset-render 13.
METHOD:
- Five plan commits on lane/tempo-0928 off main 6db8d67, plus this report.
- The RED ctest ran on the unmodified tree, and the RED probe on a byte-identical copy of main's app.
- The fix is the plan's section 3 as amended by A1-A8.
- Teeth T1-T4 were mutated in place and restored. The restore was checked by sha256 against the pre-mutation copy, and git status stayed clean.
CONFIDENCE: HIGH. VERIFY: `bash .harmony/probe-tempo-start.sh` (lock held, AUDIODNA_LOCK_OWNER set; TEMPOSTART_WITNESS_RUNS=20 for W6) and `build-lane/tests/test_tempo_start`.
UNKNOWNS / NOT DONE:
- The memory order (raise after the write, latch before the reads) cannot be tested single-threaded (R2). Only comments and Pitfall 48 guard it.
- No case covers disarm's emission of waiting-window onsets on a Stop before t = 0.
- No perf number was measured. The analysis-thread cost is one acquire load plus one store per hop, by construction.
NUANCE:
- A5 changed G6: two markers, the first at t = 0 carrying the start tick's sample. The plan body expected one.
- L2 conflicted with the plan's own perfRecord comment text. The comment was reworded; the lint was not weakened.
- probe-finalize-loop.sh launches with a bare `open`. It was run as a scratch copy that differed only by `open -g`.
HANDOFF-NEEDS:
- Harmony: the W1-W4 and W6 x20 re-run (A3).
- Add TempoMap::sampleAt to the loose-ends ledger (section 8 / A7).
- Append the notebook lines below.
- APP-INVENTORY: +1 Catch2 target (13 cases), +1 probe; the REST surface is unchanged.

INBOX-RECHECK: none

## Branch / commits
Worktree `/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0928-w1`, branch `lane/tempo-0928`, base `6db8d67`.

| # | commit | what |
|---|---|---|
| 1 | 69abbb3 | test: RED. test_tempo_start target, E1, E2, F1, fixture `tests/fixtures/take_v3_start_bpm0.json` (the real raced take from 2026-09-27, copied verbatim) |
| 2 | 5063454 | probe: `.harmony/probe-tempo-start.sh` (W1, W1b, W2, W3, W4, W6) |
| 3 | 3d411e5 | feat(analysis): `FeatureSnapshot::trackerRequestSeq` (offset 328, sizeof 384), `kTrackerLocked`, BPMTracker `requestSeq_` / `appliedRequestSeq_` / `postedRequestSeq()`, the AnalysisThread copy; B1, B2, [lint] L1 |
| 4 | 2b457c3 | fix(recording): the start gate, `startDue` / `startClock`, the 0.25 s fallback, Stop before t = 0, the onsets rule (A5), POD `StartWaitTick` (A4), `RecorderClock::started()`, perfRecord arms with `postedRequestSeq()` (not in test mode); G1-G6, [lint] L2, the E1/E2 helper swap |
| 5 | 8d88064 | docs: Pitfall 48, CLAUDE.md index 48 (CLAUDE.md now 23,995 B < 25,000; nothing moved), recording.md "Take start", architecture.md and analysis.md rows |
| 6 | (this) | report |

## RED (unmodified 6db8d67, verbatim)
**ctest** (test_tempo_start at commit 1, sources = 6db8d67):
```
E1 test_tempo_start.cpp:180: CHECK( r.host.status().t == Approx(1.0 / 120.0).margin(1e-6) )  0.01666670000000181 == Approx( 0.00833333333333333 )
E1 :190: CHECK( a.size() == 1 )  2 == 1
E1 :192: CHECK( a[0].bpm == Approx(120.0f) )  0.0f == Approx( 120.0 )
E1 :193: CHECK( take->meta.startBeatInBar == Approx(512.0 / 24000.0) )  -1.0 == Approx( 0.02133333333333333 )
E1 :195: CHECK( slice.error == "" )  "this stretch has no beat; the tempo was unknown while it was recorded" == ""
E1 :196: CHECK( takeBeatOfBar(*take, 1) == Approx(4.0 - 512.0 / 24000.0) )  0.0 == Approx( 3.9786666666666668 )
E2 :224: CHECK( r.host.status().beat == Approx(0.0).margin(1e-9) )  0.43999987840652466 == Approx( 0.0 )   [bus at Record: beatInBar 2 beatPhase 0.56]
E2 :232: CHECK( take->meta.startBeatInBar == Approx(0.0).margin(1e-9) )  2.5600001215934749 == Approx( 0.0 )
E2 :233: CHECK( takeBeatOfBar(*take, 1) == Approx(0.0).margin(1e-9) )  1.4399998784065251 == Approx( 0.0 )
test cases:   3 |   1 passed | 2 failed
assertions: 161 | 152 passed | 9 failed
```
F1 passed (a guard).

**RED against the recorder stub** (commit 4 work: API only, before 3.4/3.5):
- `test cases: 13 | 4 passed | 9 failed`, `assertions: 307 | 279 passed | 28 failed`.
- Every G case was RED:
  - G1: startBeatInBar -1 vs 3.84.
  - G2: t 0.008 vs 0.
  - G3: t 0.1 / 0.2 / 0.24 vs 0; no "take start:" line; lock anchor at t 0.285 vs 0.010.
  - G4: duration 0.008 vs 0; startBeatInBar -1 vs 2.5.
  - G5: t 0.008 vs 0.
  - G6: first marker t 0.004 vs 0.
- L2 was RED: perfRecord still named `startBeatInBar` in its new comment. It was reworded (see Deviations).
- B1, B2 and L1 passed at this point: they are the commit-3 side.

**Live RED** (`red-run1.log`, 09:39:21, load 5.33):
- App: `TEMPOSTART_APP=scratchpad/tempo/red-app/Audio-DNA.app`, a copy of main's build.
- The copy is byte-identical to main's `build/.../Audio-DNA` (sha256 bcc20073...). Main's binary was linked 00:18:32, 22 s after 233eae7 was committed. No code changed from 233eae7 to 6db8d67. The binary contains `writeReplacing`, new in 233eae7, and the stall hook (TEST_SERVER ON).
```
INFO  W1 gap 0.13 ms anchors [('start', 0.0, 0.0), ('lock', 120.0, 0.0245)] startBeatInBar None
INFO  W1 routine save (beats 0-4, slot 7): lastSaved.slot -1 lastError "This part of the take was recorded before the tempo was known, so it has no bars to cut."
FAIL  W1 first take after launch: start anchor start bpm 0.0, lock anchor True, startBeatInBar None, routine from beat 0 refused (...)
PASS  W1b liveness: bpm 120.0, totalBeatCount 1 -> 3 in 1 s
FAIL  W2 start anchor carries the set_bpm sent just before Record: 5/20
FAIL  W3 a Resync sent just before Record is beat 0 of bar 1 (startBeatInBar in [0, 0.25)): 0/20
PASS  W4 0 "take start:" lines in the app's stderr (every take started by the handshake)
PASS 6 / FAIL 3
```
**Credibility rule:** W2 failed 15 of 20 and W3 failed 20 of 20, both at least 5/20, so both count as gates. The 15 W2 misses carry the early "bpm" correction anchor. The W3 stale grid reads mostly 0.60-0.66: the previous cycle's Resync was about 0.6 s earlier. W1 is one take per launch; it failed exactly as in the diagnosis. W6 was not run on the RED app (A2: Harmony's 08:29 baseline is 4/20 and 5/20).

## GREEN
- **test_tempo_start:** `All tests passed (311 assertions in 13 test cases)`.
- **Full ctest:** `100% tests passed, 0 tests failed out of 797`, 18.37 s, serial `-j1`, after the final rebuild (10:22:46). An identical 797/797 ran at 10:05 before the teeth.
- **Live**, lane app, each run PASS 9 / FAIL 0:

| run | time | load | W1 | W2 | W3 | W4 |
|---|---|---|---|---|---|---|
| stall 1 | 10:02:54 | 9.24 | PASS (start 120, no lock, sbib 0.064, routine slot 7 saved, lastError "") | 20/20 | 20/20 | 0 |
| stall 2 | 10:03:16 | 9.05 | PASS (sbib 0.064) | 20/20 | 20/20 | 0 |
| stall 3 | 10:03:38 | 9.72 | PASS (sbib 0.0213) | 20/20 | 20/20 | 0 |
| tight pair (STALL_MS=0) | 10:03:59 | 8.08 | PASS | 20/20 | 20/20 | 0 |
| W6 run (+W1-W4) | 10:05:06 | 8.33 | PASS | 20/20 | 20/20 | 0 |
| final relinked binary (sha b270adab...) | 10:25:03 | 4.63 | PASS | 20/20 | 20/20 | 0 |

- W3 startBeatInBar over the 80 cycles of batch 1: 0.0 x30, 0.0213 x34, 0.0427 x16. That is exactly the 0 / 1 / 2-hop values the plan predicts.
- W6 witness x20 (`PASS W6 witness x20: start-bpm-0 0/20, unknown grid 0/20 (take start: lines 0)`): every run had `start bpm 120.0` and a single "start" anchor. startBeatInBar was 0.0213 or 0.0427.
- The probe deletes its own takes every run (`PASS the probe's takes were removed`). 0 Output-named windows every run.

## TEETH (in-place mutation, restored)
Each file was copied aside, mutated, rebuilt if needed, run, then written back. Every restore was checked by sha256 against the pre-mutation copy (all `True`), and git status stayed clean.

| mutation | failing cases |
|---|---|
| **T1** `startDue` returns true first | E1, E2, G2 (waits), G3 (fallback), G4 (stop), G5 (wrap), G6 (onsets): `13 | 6 passed | 7 failed` |
| **T2** latch line deleted in `runPipeline` | B1, B2, E1, E2: `13 | 9 passed | 4 failed` |
| **T3** AnalysisThread copy deleted, re-run clean | [lint] L1 only: `13 | 12 passed | 1 failed` |
| **T4** perfRecord block deleted, re-run clean | [lint] L2 only: `13 | 12 passed | 1 failed` |

After the restore: `All tests passed (311 assertions in 13 test cases)`.

**Caveat:** B1, B2 and L1 were written in commit 3 together with the analysis change, not literally compile-RED first. Their RED is the teeth above (T2 for B1 and B2, T3 for L1).

The first T3/T4 run still carried T2's mutated object. macOS GNU make compares mtimes at 1-second resolution, and the restore landed in the same second as the mutated compile. So the plain T3/T4 lists above come from a clean re-run, after the file was touched and rebuilt.

## PROBES -- section 7 re-runs on the lane app (A6 env), all 0 FAIL, 0 "take start:" lines
- `ROUTINES_BUILD_DIR=build-lane ROUTINES_RECORD_PAUSE=1.8 probe-routines.sh`: 105 PASS / 0 FAIL.
  - `tempoMap has 1 anchor(s), all with a tempo`;
  - `meta.startBeatInBar present and in [0, 4) (0.064000710844994)`;
  - `every scheduled move went out within 80 ms (worst 0.010 s late)`;
  - its rest.png was looked at: a normal rendered picture.
- `ROUTINES_RECORD_PAUSE=0`: 105 PASS / 0 FAIL (startBeatInBar 0.352000564336777).
- `BEATCLOCK_BUILD_DIR=build-lane probe-beatclock.sh`: 6 PASS / 0 FAIL.
- `DOWNBEAT_BUILD_DIR=build-lane probe-downbeat-level.sh`: 14 PASS / 0 FAIL.
- `RESYNC_BUILD_DIR=build-lane probe-resync.sh`: 16 PASS / 0 FAIL.
- `MANUALBPM_BUILD_DIR=build-lane probe-manual-bpm.sh`: 22 PASS / 0 FAIL.
- `STEP3_BUILD_DIR=build-lane probe-step3.sh`: 94 PASS / 0 FAIL.
  - `provisional take.json exists at arm+2s`;
  - `tempoMap non-empty (5 anchors), first anchor 'start', carries the set_bpm(128) anchor`;
  - T2 `mean_offset_ms=43.74 drift_ms=1.63 p95_jitter_ms=10.92 pct_matched=100.8`, all PASS.
- `FINLOOP_BUILD_DIR=build-lane` probe-finalize-loop (scratch copy with `open -g`; see found_not_fixed): `40 cycles, 0 truncations. 8 PASS / 0 FAIL`. Its 40 takes and assets stay in ~/Documents (Ruling 28, by the probe's design).
- `ONSET_BUILD_DIR=build-lane probe-onset-render.sh`: 13 PASS / 0 FAIL.
- Loads were 9-15 during these runs; other lanes were compiling. No thresholds were touched.

## Deviations from the plan body (all per the adoption or forced by the code)
1. **RED app:** a byte-identical copy of main's build, per the dispatch, instead of building 6db8d67 in build-lane.
2. **A1:** `kStartWaitFallbackSeconds = 0.25` as a constant. RecorderHost does not know the device buffer period without new plumbing (it has only deviceRate). G3 uses the constant by name.
3. **A2:** the probe prints `BLOCKED: stall hook unavailable` and exits 2 when the hook answers 404 with STALL_MS > 0. STALL_MS=0 is labelled "an extra run, never the RED/GATE mode".
4. **A3:** L1 and L2 are named "[lint] ...", carry the `[lint]` tag, and each has a comment saying it is a source-text scan, not behaviour proof. The MainComponent wire and the startBeatInBar source are proven only by the live W1-W3.
5. **A4:** a POD `StartWaitTick` holds the last waiting tick: wall, sample, bpm, beatPhase, totalBeatCount, onsetCount, trackerState and beatInBar. It replaces `unique_ptr<FeatureSnapshot>`. disarm rebuilds a stack `FeatureSnapshot` from it for `startClock`.
6. **A5 (onsets):**
   - The onset baseline stays at the first tick after arm.
   - While t = 0 waits, no marker is emitted.
   - On the t = 0 tick, the accumulated delta is emitted, stamped t = 0 at the start tick's sample.
   - The Stop-before-t = 0 path emits the same way.
   - Synthesized Decaying ends run only once the clock has started, so an expired gesture is ended on the t = 0 tick (deferred, not lost).
   - G6 therefore expects two markers: t 0 at the start sample, then t 0.008. The plan body expected one.
7. **L2 vs the plan's own comment text:** the plan's perfRecord comment says "meta.startBeatInBar", which L2 forbids inside perfRecord. The comment was reworded ("where beat 0 sits in its bar"); the lint is unchanged.
8. **Probe rig gate:** it REFUSES unless `AUDIODNA_LOCK_OWNER` is set and matches (the rig rule). Older probes only check when the variable is set.
9. **Extra asserts:**
   - G1-G4 and G6 also check the anchor and marker `sample` against the start tick's sample.
   - G3 captures std::cerr and asserts the "take start:" line appears at the fallback and not before.
   - B1 also checks that an early-return request (`bpm <= 0`) raises nothing.

## found_not_fixed
1. **TempoMap::sampleAt (A7, plan section 8 verbatim):**
   - A single-anchor map has rate 0, so `sample(t)` stays at the anchor's sample. Otherwise it extrapolates the last segment's delivered-sample slope, which a short segment makes wrong (the diagnosis's 12 ms start/lock pair gives 85,050 samples/s).
   - Its only caller is Program.cpp:451, the Sample-clock fallback for a gesture without parallel stamps. No take the app writes reaches it.
   - Fix: `compile()` passes the resolved asset rate into `convertBeatX`. `sampleAt(t, nominalRate)` uses a segment's own slope only when the segment spans >= 1 s of t, and nominalRate otherwise.
   - RED: a one-anchor map {t 0, sample 150016, bpm 120} plus a stampless gesture at beats {0, 4, 8} must compile on DriveClock::Sample to x = {150016, 246016, 342016}. Today all three are 150016.
   - Also update tests/test_program_stamps.cpp:31-35, which documents rate 0.
2. **`.harmony/probe-finalize-loop.sh:82`** launches with a bare `open` (no `-g`). The rig allows only `open -g`, so this lane ran a scratch copy that differs only in that flag (diff shown in `reruns-b.log`); the copy was deleted afterwards. `probe-step3.sh:1143` has the same bare `open`, but only in the opt-in crash test (`STEP3_RUN_CRASH_TEST=1`), which was not run.
3. **R9 (plan):** checkpoint0.bpm is still the arm-time bus bpm (MainComponent.cpp capturePerfState). It is informational only; left as the plan rules.

## Notebook lines (for Harmony to append to .harmony/notebook.md)
- 2026-09-28 A two-request message-ordering race becomes nearly certain live when both requests queue behind the TEST-ONLY message-thread stall hook (`/api/debug/stall_message_thread`). Count a row as a gate only if it fails >= 5/20 on the unmodified app (here W2 failed 15/20 and W3 20/20). | discovered: .harmony/probe-tempo-start.sh
- 2026-09-28 Teeth by in-place mutation + restore: macOS GNU make compares mtimes at 1-second resolution. A restore written in the same second as the mutated compile leaves the MUTATED object "up to date". `touch` the restored file (after a >= 1 s pause) and rebuild before trusting the next run. | discovered: build-lane test_tempo_start (T2 -> T3)
- 2026-09-28 "Everything sent before X" on the analysis side = `FeatureSnapshot::trackerRequestSeq >= BPMTracker::postedRequestSeq()` read at X (Pitfall 48). Never read the bus right after a tempo command. | discovered: src/recording/RecorderHost.cpp startDue

## PACKET QUALITY
- Clarity: CLEAR. The plan and adoption were executable. Two points had to be settled from the code (A1: no buffer period available; the L2 comment conflict).
- Missing context: the plan's perfRecord comment contradicts its own L2 lint. A5 does not say how many markers G6 should expect.
- Unused context: plan section 1.2-1.4 (rejected alternatives).
- Self-brief files: plan-tempo.md (useful, read in full including HARMONY ADOPTION), lock.sh (useful), the prior tempo-witness.sh (useful for W6), the tempo0 diagnosis fixture (present, 4399 B, copied verbatim).
- Screen safety: no Output window was ever opened (0 Output-named windows after every run). No full-screen capture, no synthetic input, no debugger, no env-var hook. The lock was held per batch (<= ~7 min) and released between batches.
