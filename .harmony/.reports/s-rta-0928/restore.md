# restore lane report (s-rta-0928) -- plan-restore.md as adopted (HARMONY ADOPTION D1-D8)

STATUS: DONE
RESULT: Both diagnosed causes of the routine start / loop-return message-thread hold are fixed on lane/restore-0928
(worktree rta0928-w4, rebased onto main 734f011). The engine-measured hold (holdMs) went from 56.7 / 59.5 / 64.9 ms
(the step-B lane app) to 0.136-0.208 ms on the lane app, in 3 of 3 quiet runs. A 600 ms stall across a recorded move
now lands the move (7m GREEN 3/3; RED on main: every sample 1.0). The diag's heartbeat protocol, re-run once on an
instrumented scratch copy of the lane: T1 = no restoring tick >= 1 ms on card / big4k / many (was 52.8-179 ms). T2
medians sit in the idle band. Stall arm 0/5 + 0/5 anomalies (was 5/5 + 0/5). Full ctest 814/814 (-j1).
FACTS:
- Commits on lane/restore-0928 (base main 734f011): ad6c8e3 (A, Player end value), 3608f7b (B, holdMs + probe rows),
  ad7f1ec (C, ClipThumbnails + memo), a6fbfa8 (D, docs), plus this report commit.
  Check: `git -C .claude/worktrees/rta0928-w4 log --oneline 734f011..lane/restore-0928`.
- The ctest count went 784 (lane base 6db8d67) -> 814 (rebased head = main's 797 + the lane's 17).
  Check: `ctest --test-dir build-lane -j1` -> "100% tests passed, 0 tests failed out of 814".
- probe-routines on the lane app: 109 PASS / 0 FAIL x3 (quiet, ROUTINES_RECORD_PAUSE=1.8).
  Evidence: `runs/lane-routines-11{4552,4738,4922}.log` in the evidence root below.
- CLAUDE.md is 24,146 bytes (`wc -c CLAUDE.md`).
METHOD: RED first for every new test and row (on 6db8d67's code or main's app), then GREEN on the lane. Teeth ran once
per mutation: the edit was made to a working copy, restored byte-identical (sha256 printed), then touched and rebuilt.
Live gates ran under the lock with `open -g` only. The temporary protocol ran on a scratch copy (`git archive HEAD` +
`patch -p1` of the diag's instr.diff minus its updateThumbnail hunks) built in a scratch dir. It was never in the worktree.
CONFIDENCE+VERIFY: High. Re-prove with `ctest --test-dir build-lane -j1`, then (under the lock)
`ROUTINES_BUILD_DIR=build-lane ROUTINES_RECORD_PAUSE=1.8 bash .harmony/probe-routines.sh <out>`. Expect 109/0, with rows
5h / 8h / 11h printing <= 1 ms and 7m PASS.
UNKNOWNS-NOT-DONE: Pitfall number left "NN" (D7; main has 48, renderleft plans 48/49): Harmony assigns it at merge.
The Boris check (below) is for Boris. T2 has 2 of 22 individual card events above the per-launch bar; the medians pass
(see TEMPORARY PROTOCOL).
NUANCE: holdMs measures the diagnosed causes inside the engine only (D3). A GREEN is not "no restore hold exists".
HANDOFF-NEEDS: Harmony: merge, assign the pitfall number, append the notebook notes below, and put the D8 Boris-list
items on the page.

INBOX-RECHECK: none

## SUMMARY
- **Cause 2 (commit A):** a tick past a gesture's end now writes the gesture's END value before letting go.
  - This covers a stall over the whole gesture and a stall over its tail.
  - A refused end write releases nothing.
  - The one-line change is in `Player::advanceTo` (`src/recording/Player.cpp`).
- **Measurement (commit B):**
  - `RoutineEngine` publishes `holdMs` / `holdMsMax` on `/api/routine/status`: the steady clock around startNow and
    the loop-return fold.
  - New probe-routines rows 5h / 8h / 11h (hold <= 16 ms) and 7m (a 600 ms stall across the first move still lands it).
- **Cause 1 (commit C):**
  - New header-only `src/ui/ClipThumbnails.h`, owned by `DeckView`. It uses FilesBrowser's pool + callAsync + weak-guard
    pattern over `ThumbnailCache`, with 2 low-priority threads.
  - `get()` never decodes. A path in flight is answered before any stat() (D4).
  - `ClipCell` / `LayerStrip::updateThumbnail` pull from the store and memoize their source.
- **Docs (commit D):**
  - recording.md: a "Restore cost" paragraph.
  - Pitfall NN (new), plus a clause in Pitfall 42.
  - CLAUDE.md index line NN.
  - APP-INVENTORY status keys.

## FILES CHANGED (all inside the FENCE)
- `src/recording/Player.{h,cpp}`: the end-value write at a gesture close, plus comments.
- `src/recording/RoutineEngine.{h,cpp}`:
  - `Status::Slot::holdMs/holdMsMax`, `Running::holdMs/holdMsMax`, `msSince`;
  - timing in startNow and the loop fold; `publishStatus` copies the fields;
  - the SlotSink `yielded` comment brought up to date (D5).
- `src/MainComponent.cpp`: `routineStatusVar` only, +2 lines (`holdMs`, `holdMsMax`).
- `src/ui/ClipThumbnails.h` (NEW).
- `src/ui/ClipCell.{h,cpp}`: `setThumbnails`, memo fields, `updateThumbnail` rewritten, `hasThumbnail()` test seam.
- `src/ui/LayerStrip.{h,cpp}`: the same (placeholder block kept verbatim).
- `src/ui/DeckView.{h,cpp}`:
  - a `thumbnails_` member, declared before the strips and cells;
  - `onLanded -> refresh()`;
  - `setThumbnails` before `setLayer` / `setClip` in rebuildGrid (sites re-grepped per D6);
  - `getThumbnails()`.
- Tests:
  - `tests/test_take.cpp`: FakeSink `order`; C1-C5 `[player][stall]`.
  - `tests/test_routine_engine.cpp`:
    - E1 and D5 `[stall]`, H1 `[hold]`;
    - FakeDispatch `preambleFireSleepMs`;
    - the G5 / J2 deltas, exactly as the plan wrote them.
  - `tests/test_clip_thumbnails.cpp` (NEW): S1-S6.
  - `tests/test_deck_thumbnails.cpp` (NEW): B0-B2.
  - `tests/CMakeLists.txt`: 2 appended blocks. test_deck_thumbnails linked with the layer-strip list + DeckView /
    ClipCell / RoutinePad and needed no extra source (R-8's fallback was not needed).
- `.harmony/probe-routines.sh`: header notes; rt.py `first_move_span`, `stallat`, `movestall`; rows 5h / 8h / 11h / 7m.
- Docs: `docs/claude/recording.md`, `docs/claude/pitfalls.md`, `CLAUDE.md`, `.harmony/APP-INVENTORY.md`.
- NOT touched: `src/render/*`, `src/media/*`, ApiServer / TestServer, `Clip.h`, renderleft's / tempo's files.

## RED / GREEN (verbatim summary lines)

### Cause 2 ctests
RED on 6db8d67's Player.cpp, with the new tests compiled in:
```
test_take "[player][stall]":
  test_take.cpp:630: FAILED: REQUIRE( sink.sets.size() == 1 ) with expansion: 0 == 1        (C1)
  test_take.cpp:644: FAILED: REQUIRE( sink.sets.size() == 2 ) with expansion: 0 == 2        (C2)
  test_take.cpp:661: FAILED: CHECK( sink.sets.back().second == Approx(1.0f) ) with expansion: 0.5f == Approx( 1.0 )  (C3)
  test_take.cpp:690: FAILED: REQUIRE( sink.sets.size() == 2 ) with expansion: 1 == 2        (C5)
  test cases:  5 | 1 passed | 4 failed          (C4 = the pin, GREEN before and after)
test_routine_engine E1: test_routine_engine.cpp:2272: FAILED: REQUIRE( ev.size() == 3 ) with expansion: 2 == 3
  test cases: 1 | 1 failed   assertions: 9 | 8 passed | 1 failed   (the E1 control section passed)
test_routine_engine D5: test_routine_engine.cpp:2331: FAILED: CHECK( rig.slot(0).yielded == 1 ) with expansion: 0 == 1
```
GREEN on the lane:
```
test_take [player][stall]: All tests passed (18 assertions in 5 test cases)
test_routine_engine [stall]: All tests passed (76 assertions in 6 test cases)   (4 old + E1 + D5)
```
No other ctest expectation changed. Only G5 and J2 moved, and exactly as the plan wrote them:
- G5 added `CHECK(gestureLast < 0.9f)`, and ev0 went from [Release, Touch, Set(gestureLast)] to
  [Set 0.9, Release, Touch, Set 0.9].
- J2's ev0 went from [Release, Touch, Set 0.1, Release] to [Set 0.9, Release, Touch, Set 0.1, Release].

### H1 ([hold])
- RED by construction: the field did not exist, so the test cannot compile on 6db8d67.
- GREEN: `All tests passed (17 assertions in 1 test case)`.

### Store and grid ctests
- **B0 RED on the old grid** (accessors only, 6db8d67's ClipCell / LayerStrip):
  ```
  test_deck_thumbnails.cpp:109: FAILED: CHECK_FALSE( c->hasThumbnail() ) with expansion: !true  -- image cell at layer 2, column 1
  test_deck_thumbnails.cpp:109: FAILED: ... image cell at layer 0, column 0
  test_deck_thumbnails.cpp:109: FAILED: ... image cell at layer 0, column 2
  test_deck_thumbnails.cpp:115: FAILED: CHECK_FALSE( s->hasThumbnail() ) with expansion: !true
  test cases:  1 | 1 failed   assertions: 11 | 7 passed | 4 failed
  ```
- **S1-S6** were RED by construction (a new header). GREEN: `All tests passed (112 assertions in 6 test cases)`.
- **B0-B2** GREEN on the wired grid: `All tests passed (44 assertions in 3 test cases)`.

### Live, probe-routines (new rows)
**main's app** (copy of build/…/Audio-DNA.app, 6db8d67). Two runs, 105 PASS / 4 FAIL each: the default run and one with
ROUTINES_RECORD_PAUSE=1.8. All old rows PASS. Lines from the 1.8 run:
```
FAIL  5h: the Ease start held the message thread absent ms (bank[0].holdMs; expected 0..16 -- RED: every deck refresh decoded each image thumbnail on the message thread, restore-diag.md)
FAIL  7m: a 600 ms stall across the first move (positions 0.811 -> 2.027 over [1.195, 1.707]) did NOT land it: L0 opacity [1.0] at 32 samples in 2.027..5.0 (expected 0.5 +/- 0.05; every sample 1.0 = the move was touched and released unwritten -- restore-diag cause 2)
FAIL  8h: the Ease start / loop return held the message thread absent ms at cycle 2 (...)
FAIL  11h: the Jump start / loop return held the message thread absent ms at cycle 2 (...)
105 PASS / 4 FAIL
```
**step-B lane app** (commits A+B, before C): quiet (no clang), load 5.20, RRP 1.8.
```
FAIL  5h: the Ease start held the message thread 56.676 ms (bank[0].holdMs; expected 0..16 -- RED: ...)
FAIL  8h: the Ease start / loop return held the message thread 59.481 ms at cycle 2 (bank[0].holdMsMax; ...)
FAIL  11h: the Jump start / loop return held the message thread 64.939 ms at cycle 2 (holdMsMax; ...)
PASS  7m: a 600 ms stall across the first move (positions 0.853 -> 2.069 over [1.195, 1.707]) still lands it: L0 opacity 0.5 at 31 samples in 2.069..5.0
106 PASS / 3 FAIL
```
**lane app** (rebased head), 3 quiet runs, RRP 1.8, loads 5.69 / 4.18 / 4.87:
```
run 1: 5h 0.177 ms | 8h holdMsMax 0.179 (cycle 2) | 11h 0.136 (cycle 2) | 7m 0.811 -> 2.027 over [1.216, 1.728]: 0.5 at 32 samples | 109 PASS / 0 FAIL
run 2: 5h 0.164 ms | 8h 0.174 | 11h 0.171 | 7m 0.811 -> 2.112 over [1.216, 1.749]: 0.5 at 31 samples | 109 PASS / 0 FAIL
run 3: 5h 0.208 ms | 8h 0.165 | 11h 0.159 | 7m 0.832 -> 2.069 over [1.195, 1.749]: 0.5 at 30 samples | 109 PASS / 0 FAIL
```
Example line: `PASS  5h: the Ease start's restore held the message thread 0.177 ms <= 16 ms (bank[0].holdMs -- the diagnosed
causes only: a GREEN is not 'no restore hold exists')`.

No 7m run was inconclusive, so the one allowed retry was never used.

### Other live gates

| probe | main's app (baseline) | lane app |
|---|---|---|
| probe-routine-display | 16/0 | 16/0 and 16/0 |
| probe-step3 | 94/0 | 94/0 |
| probe-beatclock | 6/0 | 6/0 |
| probe-deck-tabs | 6/0 | 6/0 |

Every `outwins` read "audio-dna windows 0, Output-named 0".

**Step 7.2 check.** probe-routine-display loads the composition and then sleeps 2 s before any shot
(`setup_show`, probe-routine-display.sh:172-178). No shot is taken within 0.5 s of a load.

**Looked at:**
- The lane app's `01-idle-w0.png` (display run 120514). The L1 and L3 image cells and both image strips show the
  test-card thumbnail: the off-thread path lands live.
- `rest.png` of lane run 114922: the test card, restored and non-blank.

## TEETH (each run once; restored byte-identical, then touched + rebuilt)

| id | mutation | result |
|---|---|---|
| A (D1) | `Player.cpp`: `if (!cur.displaced && sink.set(..eval(x1)))` -> `if (!cur.displaced)` | C1 :630, C2 :644, C3 :661, C5 :690 FAIL; G5 :1139, J2 :1523, E1 :2277, D5 :2336 FAIL. Restored sha256 282613e5… |
| H1 | the hold clock starts only after the discrete half (start and loop) | `0.018417 >= 20.0` and `0.000292 >= 20.0` FAIL |
| t1 | `get()` returns `decode_(imageFile)` on a miss | S1 :136/:137 FAIL x10 |
| t2 | `landed()` does not erase `inFlight_` | S3 :195, :199, :203 (`2 == 3`), :204 FAIL |
| t3 | the completion calls a raw captured pointer, not the weak reference | S4: `SIGSEGV - Segmentation violation signal` (no ASan needed) |
| t6 (D4) | stat before the in-flight check | S6 :265 `32 == 2` FAIL |
| t4 | synchronous decode re-inserted in `ClipCell::updateThumbnail` | B0 :186 FAIL x3 (cells) |
| t5a | ClipCell memo removed | B2 :280 `1176 == 216` (+32 x 30), :284 FAIL |
| t5b | LayerStrip memo removed | B2 :280 `432 == 192` (+8 x 30), :284 FAIL |

**Harness finding (t2 / t6).** The first t2 / t6 runs never rebuilt the test. The previous restore's `cp` landed in
the same second as the object file, so make treated the object as up to date. teeth.sh now does `sleep 1; touch`
after both the mutation and the restore, and t2 / t6 were re-run with real rebuilds (results above). The same effect
also hit H1's restore once: the test binary kept the mutated object. It was found when full ctest showed H1 failing;
touching and rebuilding fixed it (ctest 792/792 then).

## TEMPORARY PROTOCOL (D3; diag method, scratch copy, never committed)

**Rig:**
- `restore-lane/diagtree` = `git archive` of commit C as it was before the rebase (2bfc876; rebased = ad7f1ec).
- Instrumentation: `patch -p1` of instr.diff minus the two updateThumbnail decode hunks. The 2 rejected RoutineEngine
  hunks (startNow entry, start notify) were applied by hand.
- Built in `restore-lane/diagbuild`: Release + TEST_SERVER + SYPHON.
- The worktree never held the instrumentation. `strings build-lane/…/Audio-DNA | grep -c '\[DG\]\|\[HB\]\|\[EV\]'` = 0.

**Runs:** `run1.sh` (lane `restore`, the lock per launch, `wait_quiet`, load printed), arms ease,jump,loop,loopjump x5
for FIXVAR card / big / many; then `IDLE=0` stackstall450,stackstall150 x5.

**How to read the table:**
- T1 = the enclosing tickFeaturePipeline scope. The scope logs at >= 1 ms, so "none logged" means < 1 ms.
- T2 = the ±250 ms heartbeat max. The bar is the same launch's per-500 ms-window idle p90 + 5 ms.

| fixture (load at start) | event | n | T1 before (diag med) | T1 after | T2 before (diag med) | T2 after med [min-max] | T2 bar |
|---|---|---|---|---|---|---|---|
| card (4.4-4.9) | start Ease | 5 | 52.8 | none >= 1 ms | 51.7 | 18.4 [12.1-26.0] | 24.98 |
| card | start Jump | 5 | 55.7 | none | 58.8 | 18.7 [17.9-28.3] | 24.98 |
| card | loop return Ease | 5 | 56.1 | none | 54.7 | 18.7 [16.5-20.6] | 24.98 |
| card | loop return Jump | 5 | 56.9 | none | 56.4 | 18.9 [15.4-20.2] | 24.98 |
| big4k (5.2) | start Ease | 5 | 179.1 | none | 178.2 | 19.3 [18.4-19.7] | 24.71 |
| big4k | start Jump | 5 | 176.3 | none | 178.6 | 18.4 [18.1-19.3] | 24.71 |
| big4k | loop return Ease / Jump | 5 / 5 | 178.1 | none | 180.0 | 15.8 [6.5-18.6] / 16.4 [9.1-21.6] | 24.71 |
| many (3.6) | start Ease | 5 | 153.9 | none | 154.8 | 17.0 [16.7-17.4] | 30.48 |
| many | start Jump | 5 | 155.1 | none | 156.7 | 16.2 [15.9-16.7] | 30.48 |
| many | loop return Ease / Jump | 5 / 5 | -- | none | -- | 16.5 [15.6-16.9] / 16.6 [15.6-17.6] | 30.48 |

**Idle background per launch** (per-500 ms-window max: median / p90 / max):
- card: 17.3 / 20.0 / 25.7
- big4k: 17.1 / 19.7 / 21.6
- many: 21.8 / 25.5 / 27.8

**T1: GREEN on all 3 fixtures.** 0 restoring ticks >= 1 ms. The only logged top-level scopes are the fire calls
(0.02-0.17 ms).

**T2: GREEN by median on every row.** At the individual-event level, 2 of 22 card events exceed the 24.98 bar:
- start Ease rep 3 at 26.0;
- start Jump rep 1 at 28.3.

Why these are INFERRED to be background, not a restore hold:
- T1 shows no tick >= 1 ms at those very events.
- The same launch's idle window reached 25.7 ms.
- The non-restoring fire calls reached 26.5 ms.
- This is the diag's found_not_fixed #1 (idle 17-25 ms blocks).

Big4k and many: every event is under its bar.

**Stall arm** (the diag's stackstall):
- 450 ms: anomaly False 5/5. Before: True 5/5. The app logged `gestureSkipped` on 5 reps: the step-over happened, and
  the move still landed.
- 150 ms: anomaly False 5/5, unchanged.
- Rep 4 of the 450 ms arm has one sample at 1.0 among 52; the other 51 are 0.5. INFERRED: a composition read queued
  during the stall and served before the resume tick.

The takes the driver recorded were moved to `restore-lane/diag/takes/`.

## BORIS CHECK (the visible change; CLAUDE.md "Non-Technical User")
1. Load a show whose grid holds several big photos, including one 4K still. The thumbnails appear within a blink: a
   photo never seen before shows the empty-cell look for a split second, once per session.
2. Fire a looping routine that switches a layer back to a photo, with Start: Ease, then again with Start: Jump.
3. At every start and every loop point the app keeps moving: no freeze of the grid, the meters or the faders.

## BORIS LIST (D8)
- At a loop point where a recorded move runs to the very end, the Ease return glide now starts from that move's
  recorded end value (e.g. 0.9) instead of where the last screen tick caught it (0.886). It is the recording's own hand.
- A never-seen image cell shows the empty-cell look for about 10-60 ms, once per file per session (INFERRED; low-priority
  decode threads).

## ISSUES / DEVIATIONS
- **Report location.** The plan's `REPORT_FILE:` line names plan-restore.md itself; the task names this file. I wrote
  this file.
- **.venv symlink.** It was present (untracked, never staged) during commits A and B; it was removed before C, D and
  this report commit. None was committed.
- **Record pause.** The first base and step-B probe-routines runs used no ROUTINES_RECORD_PAUSE. They were re-run with
  1.8, and all numbers above are from 1.8 runs except the first base baseline (105/4 either way).
- **Rebase.** The rebase onto main 734f011 (tempo merge) conflicted only in `tests/CMakeLists.txt`. It was resolved as
  HEAD's file + this lane's two appended blocks. test_tempo_start stays GREEN in the 814/814 run; this proves the
  plan's ASSUMED "tempo cases do not observe gesture ends". main has since moved to 450f669 (notebook / work-log docs
  only; no file of this lane).
- **Grid test link list.** test_deck_thumbnails linked with the plan's list; R-8's fallback was not needed.
- **Pitfall number.** Kept as "NN" in recording.md, pitfalls.md and the CLAUDE.md index (D7). Harmony assigns it at merge.
- **Temporary protocol build.** The scratch copy was taken before the rebase (commit C on 6db8d67). The tempo merge
  touches no routine-restore code (RecorderHost / BPMTracker / FeatureSnapshot only).

## FOUND, NOT FIXED
1. **A missing or undecodable image stats its file on every refresh.** Its cell re-pulls, and `get()` stats once to
   check `failed_` at the current mtime; the pending path costs none (D4). That is ~µs of message-thread I/O per such
   cell per refresh.
2. The plan's FILED list stands:
   - sequence first-frame thumbnails still decode on the message thread at load / drop;
   - `ClipCell::paint` stats every media file on every paint;
   - the RefreshBatch fallback;
   - diag found_not_fixed #1 / #2.

## TESTS
- ctest: 814/814, run serially (`ctest -j1`, 18.08 s).
- New cases (17): C1-C5, E1, D5, H1, S1-S6, B0-B2.
- Live probes: see RED / GREEN above.

## NOTES FOR .harmony/notebook.md (Harmony appends)
- **Teeth restore needs `sleep 1; touch`** (files: any teeth/mutation run).
  - A `cp` restore that lands in the same second as the mutated object file leaves make believing the object is up to
    date. The test binary silently keeps the mutation.
  - Always `sleep 1; touch <file>` after both the mutation and the restore.
  - Seen twice in this lane (H1 restore, t2 / t6).
- **The stash-guard hook blocks `git add -A` even inside a scratch copy's own `git init`** (files: scratch trees).
  - Use `patch -p1` (or plain `git apply` without `--3way`) to put a diag diff onto a scratch copy.
  - Hand-apply the rejects.
- **Probe-driven lock batches: set the EXIT trap only after acquire_lock succeeds** (files: `scratchpad/lib/lock.sh`
  users).
  - Otherwise killing a waiting batch would osascript-quit ANOTHER lane's app.
  - Held in this lane: the trap is installed after `acquire_lock`.

## PACKET QUALITY
- Clarity: CLEAR. The adoption rulings resolved the D4 / D5 forks.
- Missing context: the plan's step numbering predates the rebase. main moved mid-lane (tempo merge); the plan's
  "rebase first" before commit D covered it.
- Self-brief: plan-restore.md (full), restore-diag.md sections 1-5, HANDOFF rig rules. All useful.
- KNOWLEDGE CONTEXT: grep only. Impact was judged conservatively by reading every ClipCell / LayerStrip creator (only
  DeckView).

Evidence root: `/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/cfd08720-0a05-463c-84b1-89ae4354a3fb/scratchpad/restore-lane/`
- `runs/*.log` (every probe run);
- `red-step1-*.txt`, `red-B0.txt`;
- `teeth*.out`;
- `diag/runs/{cardA,bigA,manyA,stallA}`, plus `diag/instr-lane.diff` and `diag/takes/`.
