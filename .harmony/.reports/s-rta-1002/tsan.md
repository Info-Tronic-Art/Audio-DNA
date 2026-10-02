# LANE tsan (s-rta-1002) -- report

## B1 (T0 + T1 + T2) -- Builder, started 2026-10-02 08:11 EDT

STATUS: DONE_WITH_CONCERNS (B1 items T0, T1, T2 committed; concerns: T0 normal-build bar wording -- R1 / R4 also RED on the base; the amendment-4 contention test moves to B2 with T3)
T0 sha: 2c04a03e59304efca134df617f7e4ff05675bb8f
Base: 2d38b39 (main head; plan base 02b2913 + docs only)

### Build state
- Normal: <wt>/build-lane, Release, TEST_SERVER ON, SYPHON ON, FETCHCONTENT_FULLY_DISCONNECTED, deps from main's
  build/_deps (configure 08:11:41-08:12:14, first full build done 08:41:31, rc=0).
- TSan: <wt>/build-tsan (H7), RelWithDebInfo, -DADNA_SANITIZE=thread, same deps; only the [tsan] targets are built
  (configure 08:18:01-08:18:35, race targets built 08:20:22, rc=0).
- Disk before the new build dirs: 310 GiB free on /System/Volumes/Data.

### T0 -- RED-first tests (commit 2c04a03e59304efca134df617f7e4ff05675bb8f)
Files: tests/test_layer_runtime_race.cpp (NEW: R1, R2, R4), tests/test_manual_scalar_race.cpp (NEW: R3),
tests/test_undo_commands.cpp (+ D1, D1b, D1c), tests/test_log_line_lint.cpp (NEW: D6), tests/test_render_thread_lint.cpp
(NEW: lint case 1), tests/CMakeLists.txt (4 targets appended at the END of the file, H5), .harmony/probe-tsan-unit.sh
(NEW, amendment 11; lands in the T0 commit so G2's RED arm can run it on this sha).
Registration (amendment 2): `catch_discover_tests(<t> PROPERTIES LABELS tsan TIMEOUT 300 ENVIRONMENT
"TSAN_OPTIONS=exitcode=66:halt_on_error=0:abort_on_error=0:report_signal_unsafe=0:history_size=4"
FAIL_REGULAR_EXPRESSION "WARNING: ThreadSanitizer")`, via one CMake list ADNA_TSAN_TEST_PROPERTIES.

`ctest -L tsan -N` (TSan build, verbatim):
```
Test project /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/tsan/build-tsan
  Test #110: R1 message-thread triggers vs render clock / autopilot on one deck
  Test #111: R2 clip runtime fields: trigger writes vs render transport write-back
  Test #112: R4 tuple consistency and no lost fade under a paced trigger storm
  Test #113: R3 manual scalar writes vs eff() reads

Total Tests: 4
```
Normal build: the same 4 cases (#1058-#1061); `ctest -N` total 1064 = 1055 + 9.
Case count per new target at T0: test_layer_runtime_race 3, test_manual_scalar_race 1, test_log_line_lint 1,
test_render_thread_lint 1 (case 2 is B2's, T3), test_undo_commands +3 (D1, D1b, D1c).

RED bar, TSan build -- each case run directly with the ENVIRONMENT options (exit code = the process's), verbatim
(the src column counts reports whose stacks -- access, location / allocation, or thread creation -- name a src/ file
basename; TSan prints basenames):
```
R1 rc=66 time=3.58s races=87 | test cases:    1 |    0 passed | 1 failed | src: reports=87 kinds={'data race': 87} reports_with_src_frame=87 src_files={'Deck.h': 87, 'Layer.h': 77, 'Autopilot.cpp': 68, 'TriggerCommands.h': 31, 'UndoManager.cpp': 31}
R2 rc=66 time=0.33s races=8 | All tests passed (5 assertions in 1 test case) | src: reports=8 kinds={'data race': 8} reports_with_src_frame=8 src_files={'Deck.h': 8}
R4 rc=66 time=11.87s races=6 | test cases: 1 | 1 failed | src: reports=6 kinds={'data race': 6} reports_with_src_frame=6 src_files={'Deck.h': 6}
R3 rc=66 time=0.39s races=4 | All tests passed (9 assertions in 1 test case) | src: reports=4 kinds={'data race': 4} reports_with_src_frame=4 src_files={'ManualWrite.cpp': 4, 'Composition.h': 3, 'Deck.h': 3, 'Layer.cpp': 2, 'Clip.cpp': 1}
```
RED bar, TSan build through ctest (G2's command, no TSAN_OPTIONS in the shell), verbatim:
```
1/4 Test #110: R1 message-thread triggers vs render clock / autopilot on one deck  ... ***Failed  Error regular expression found in output. Regex=[WARNING: ThreadSanitizer] 10.80 sec
2/4 Test #111: R2 clip runtime fields: trigger writes vs render transport write-back  ... ***Failed  Error regular expression found in output. Regex=[WARNING: ThreadSanitizer]  0.27 sec
3/4 Test #112: R4 tuple consistency and no lost fade under a paced trigger storm  ... ***Failed  Error regular expression found in output. Regex=[WARNING: ThreadSanitizer] 16.47 sec
4/4 Test #113: R3 manual scalar writes vs eff() reads  ... ***Failed  Error regular expression found in output. Regex=[WARNING: ThreadSanitizer]  0.34 sec
0% tests passed, 4 tests failed out of 4
reports=119 kinds={'data race': 119} reports_with_src_frame=119 src_files={'Deck.h': 118, 'Layer.h': 88, 'Autopilot.cpp': 80, 'UndoManager.cpp': 36, 'TriggerCommands.h': 36, 'ManualWrite.cpp': 4, 'Composition.h': 3, 'Layer.cpp': 2, 'Clip.cpp': 1}
```
Per-case times: TSan 0.27-16.47 s (all <= 60 s); normal build 0.01 s each (<= 5 s). Iterations: R1 20,000 main
iterations; R2 40,000; R3 40,000; R4 20,000 paced triggers.

RED bar, normal build (full ctest -j1 on the T0 tree, 08:43:03-08:44:08), verbatim:
```
99% tests passed, 11 tests failed out of 1064
The following tests FAILED:
	388 - D1 a trigger's first perform does not restart a running fade (Failed)
	389 - D1b a trigger's first perform after the fade ended keeps the ended fade (Failed)
	390 - D1c a trigger's first perform after its queued trigger fired keeps it fired (Failed)
	464 - T2: mapping resolves by key when the display name changes but shaderName is kept (Failed)
	471 - Deck round-trip: mapping resolution survives the embedded-fx save/load path (Failed)
	472 - Deck files never carry an output display (plan5 R7) (Failed)
	821 - AppSettings: update is read-modify-write -- two keys, both kept (Failed)
	823 - AppSettings: a corrupt file reads as empty and the next update rewrites a valid object (Failed)
	1058 - R1 message-thread triggers vs render clock / autopilot on one deck (Failed) tsan
	1062 - no std::cerr in code that runs off the message thread (Failed)
	1063 - Renderer.cpp: no plain store to a clip's playing; syncMedia goes through ClipTransportSync (Failed)
```
- D1 / D1b / D1c / D6 / lint case 1 fail as pre-registered. Failing output (verbatim):
```
test_undo_commands.cpp:3080: FAILED:  CHECK( captureLayerRuntime(L).crossfadeProgress == Catch::Approx(0.2f) )  with expansion:  0.0f == Approx( 0.20000000298023224 )
test_undo_commands.cpp:3108: FAILED:  CHECK( captureLayerRuntime(L) == ended )
test_undo_commands.cpp:3130: FAILED:  CHECK( captureLayerRuntime(L).activeClipColumn == 1 )  with expansion:  0 == 1
test_undo_commands.cpp:3131: FAILED:  CHECK( captureLayerRuntime(L).pendingTriggerColumn == -1 )  with expansion:  1 == -1
test_log_line_lint: total std::cerr in the worker-thread list: 75  (assertions: 29 | 14 passed | 15 failed)
test_render_thread_lint: plain stores to playing: Renderer.cpp:1826, Renderer.cpp:1833, Renderer.cpp:1911, Renderer.cpp:1918
                         ClipTransportSync:: call sites: 0
```
- DEVIATION from amendment 2's normal-build bar ("everything else passes"): R1 also fails in the NORMAL build on
  the base, by its value checks (I1 / range on render snapshots): 5/5 direct runs + the ctest run, e.g.
  `test_layer_runtime_race.cpp:169: FAILED: CHECK( violations.load() == 0 ) with expansion: 10 == 0`. R4 failed 4 of
  5 direct normal runs on the base (I3 = a lost trigger: `I1 0 I2 0 I3 1`, `I3 4`) and passed in the ctest run. Both
  are true RED (the base tears the tuple and loses a trigger under a fade tick), so the tests were NOT weakened;
  Harmony rules whether the bar's wording stands.
- 464 / 471 / 472 / 821 / 823 are NOT this lane's: each passed on an isolated re-run (ctest -I 464,464 and
  -I 821,821: "100% tests passed"). INFERRED cause: the other lane's concurrent ctest -- test_preset_manager writes
  fixed names in the user temp dir (`tempPresetFile`: getChildFile("preset_manager_test_" + suffix + ".json")),
  test_app_settings uses getNonexistentChildFile("audiodna-test-app-settings") (a check-then-create across
  processes). Not a flake verdict (no 5-run arms).
- Measured while sizing (kept in the test as a comment): with the R4 Layer on the main thread's STACK this TSan
  runtime reported 0 races in 4/4 base runs although the value checks saw 1,643-4,387 torn tuples per run; on the
  heap it reports. R4 now uses a Deck (heap, allocated in Deck.h) so its reports carry a src/ (allocation) frame.

probe-tsan-unit.sh from a FRESH dir on the T0 tree (configure-if-absent path; `env -u TSAN_OPTIONS`), verbatim:
```
probe rc=8
probe-tsan-unit: configure .../scratchpad/tsan-B1/build-tsan-probe (ADNA_SANITIZE=thread, RelWithDebInfo) 2026-10-02 08:45:26
probe-tsan-unit: build test_layer_runtime_race test_manual_scalar_race 2026-10-02 08:45:56
probe-tsan-unit: ctest -L tsan 2026-10-02 08:47:28
1/4 Test #110: R1 ... ***Failed  Error regular expression found in output. Regex=[WARNING: ThreadSanitizer]  6.43 sec
2/4 Test #111: R2 ... ***Failed  Error regular expression found in output. Regex=[WARNING: ThreadSanitizer]  0.28 sec
3/4 Test #112: R4 ... ***Failed  Error regular expression found in output. Regex=[WARNING: ThreadSanitizer] 16.83 sec
4/4 Test #113: R3 ... ***Failed  Error regular expression found in output. Regex=[WARNING: ThreadSanitizer]  3.55 sec
0% tests passed, 4 tests failed out of 4
probe-tsan-unit: ctest rc=8 2026-10-02 08:47:55
```
(the scratch build dir was deleted after the run).

Evidence (scratch, not committed): <scratchpad>/tsan-B1/evidence/{t0-tsan-direct2/*.log, ctest-tsan-t0.log,
ctest-normal-t0.log}.


### T1 -- Relaxed.h, LogLine.h, test_relaxed (commit 7422e10d38e5f9ce049724ee0d50b9d525dfd950)
Files: src/model/Relaxed.h (NEW), src/core/LogLine.h (NEW), tests/test_relaxed.cpp (NEW, GREEN-only per plan T1),
tests/CMakeLists.txt (test_relaxed appended at the end).
- Relaxed<T>: implicit `Relaxed(T)` + `operator T()`, noexcept copy / move ctor + assign (relaxed load / store),
  operator=(T), load(), store(), compareExchange(T& expected, T desired) (strong, relaxed / relaxed), fetchAdd(T) for
  integral non-bool T (enable_if), static_assert(std::atomic<T>::is_always_lock_free), NO compound operators.
  Aliases RelaxedFloat / RelaxedDouble / RelaxedBool / RelaxedInt. Not yet used by any model field (T4 / T5 / T6).
- logLine(const A&... a): local std::ostringstream, fold `os << ... << a`, '\n', ONE std::fwrite to stderr.
GREEN (normal build), verbatim:
```
3/4 Test #1066: Relaxed: compareExchange succeeds on a match and reports the current value on a mismatch ...   Passed    0.00 sec
4/4 Test #1067: Relaxed: fetchAdd is one read-modify-write returning the previous value ....................   Passed    0.00 sec
100% tests passed, 0 tests failed out of 4
```
logLine has no ctest case (plan T1 lists none); scratch check (clang++ -fsanitize=thread, not committed): the
same arguments through logLine and through std::cerr print the identical line `[Renderer] fps=59.94 w=   42 ok`
(std::setw passes as an argument), and two threads x 2,000 logLine calls gave 4,000 whole lines, 0 TSan warnings.


### T2 -- LayerRuntimeCell (commit 3fec609a4e1c4b23f0ec7e9b710322a99d7d30f8)
Files (src): src/model/Layer.h, src/core/DeckCommands.h, src/core/TriggerCommands.h, src/model/Deck.h,
src/MainComponent.cpp, src/core/CompositionLoad.h, src/recording/PerfStateCapture.cpp, src/api/ApiServer.cpp (418-421
only, H5), src/midi/MidiOutputHandler.cpp, src/ui/DeckView.cpp; compile-forced render reads (T3's files, minimal):
src/render/LayerClock.h, src/render/DeckClock.h, src/render/CompositorEngine.cpp, src/render/Renderer.cpp (:593),
src/model/Autopilot.cpp. Tests: tests/test_layer_runtime.cpp (NEW, 10 cases), tests/test_shared_field_types.cpp (NEW,
1 case), 10 migrated test files, tests/CMakeLists.txt (2 targets appended at the end).
Fences: `git diff -U0` hunk heads in MainComponent.cpp: 206, 620, 630, 757-765, 4612-4660, 4831-4876, 5567, 6908-6916,
7539-7542, 7589-7596, 7628-7633, 7767-7795; ApiServer.cpp: 418-422. None in bt2's 484-494 / 2144-2160 / 3070-3095 /
5690-5705, ApiServer ~323 / ~2086+.

What landed (the spec, with the ruling's amendments):
- Layer.h: LayerRuntimeSnapshot (moved from DeckCommands.h, same 5 fields + operator==), LayerRuntimeTransition
  {before, after, applied; changed()}, LayerRuntimeCell (alignas(16) Word {int32 active, int32 previous, float
  progress, uint32 pendingPacked = (pending+1) & 0x0FFFFFFF | snap << 28}; kMaxPendingColumn 268,435,454; load =
  acquire, store = release, compareExchange = strong acq_rel / acquire-on-failure; relaxed copy / move;
  static_assert(is_always_lock_free) under __APPLE__; static_assert(sizeof(Word) == 16)). Layer: private
  `LayerRuntimeCell runtime_` (declared after `clips`), runtime() / setRuntime() / casRuntime() / updateRuntime(fn,
  maxAttempts = 0 [, beforeCas]) returning {before, after, applied} (applied = false only on bounded exhaustion).
- Pure tuple functions immediateNext() / clearedNext(). triggerClip(col, forcedSnap, maxAttempts = 0),
  triggerClipImmediate(col, maxAttempts = 0), processPendingTrigger(beatInBar, barCount, maxAttempts = 16, amendment
  7), clearActiveClip(optional<int> onlyIfActive, maxAttempts = 0), releaseMomentary(col) (amendment 10) all return
  the exact transition. getActiveClip(): ONE load (+ a const getClipAt overload).
- Amendment 5: the activation tail (playhead = inPoint, beatsPlayed = 0, playing = true only for a NEW activation of a
  never-triggered clip) runs in updateRuntime's beforeCas, i.e. before EVERY CAS attempt, recomputed per attempt; a
  retrigger with nothing queued changes no tuple field, so its tail runs once after (no CAS to publish). The clear
  tail (old active clip's playing = false) runs once after the successful CAS (applyClearTail).
- Amendment 6: TriggerClipCmd and ClearActiveClipCmd: the first execute() is a no-op (bool firstExecute_); redo =
  setRuntime(after_) (+ playing restore for TriggerClipCmd); undo = setRuntime(before_). sameIntent never introduced.
  Doc sentence in TriggerCommands.h ("Undo and redo are VALUE RESTORES ... supersede any GL-thread transition since
  the gesture ... as main always did"); the "status-quo field-level race" comments in TriggerCommands.h and
  DeckCommands.h (ClearActiveClipCmd) retired.
- cancelPendingTriggers / applyPendingTriggerCancellation: one updateRuntime per layer; the returned entries come
  from t.before (the exact CAS pair). captureLayerRuntime / applyLayerRuntime kept as the compat surface (= runtime()
  / setRuntime()).
- Deck::triggerColumn(col, forcedSnap, std::vector<std::optional<LayerRuntimeTransition>>* out = nullptr).
- MainComponent: handleClipTrigger (rtBefore / rtAfter = t.before / t.after; wasRetrigger = t.before.active ==
  column; the preview block uses getClipAt(t.after.activeClipColumn)); handleColumnTrigger (pairs from triggerColumn's
  out vector; the capture loop keeps playBefore / autoPlays); X clear and Clear Clip(s) (clearActiveClip(cell.column),
  one CAS) push the returned pair; both momentary release sites call releaseMomentary with no pre-check;
  applyClearActiveClip keeps a one-load early-out (comment: a GL transition can only make a clip active, never clear).
- Compile-forced (B2's T3 replaces them): LayerClock::advanceCrossfade = runtime() + ONE casRuntime (adopt-on-fail);
  DeckClock / CompositorEngine (incomingImagePending, renderLayerStages' observe, applyTransition) / Renderer.cpp:593 /
  Autopilot read runtime() per site (today's read count, not yet one load per layer); Autopilot's 3 triggerClip calls
  pass kRenderTriggerAttempts = 16 (so no commit leaves an unbounded GL-thread loop).
- Static check (base 02b2913 headers, -fsyntax-only): std::is_nothrow_move_constructible_v<Layer> holds on the base,
  so test_layer_runtime keeps the static_assert.

RED (pre-change tree = T0 commit): R1 / R4 under TSan and D1 / D1b / D1c in the normal build -- see T0.
GREEN (normal build, full ctest -j1, 08:54:09-08:55:40), verbatim:
```
99% tests passed, 3 tests failed out of 1079
The following tests FAILED:
	1062 - no std::cerr in code that runs off the message thread (Failed)
	1063 - Renderer.cpp: no plain store to a clip's playing; syncMedia goes through ClipTransportSync (Failed)
	1074 - activation tail before the CAS: the GL thread never sees a fresh trigger's stale beatsPlayed (Failed)
```
- 1062 / 1063 are T7 / T4 RED (B3 / B2). 1074 was a start race in MY new test (the GL-side baseline was taken after
  the first trigger, so it waited for an activation it had already folded into its baseline: "activations seen 0");
  fixed with a ready handshake before the first trigger, then 5/5 and later 30/30 pass. D1 / D1b / D1c, R1 and R4
  (normal) and every migrated test pass.
GREEN (TSan build, the [tsan] cases at T2, direct runs), verbatim:
```
R1 rc=66 time=3.12s races=19 | All tests passed (1182 assertions in 1 test case) | src: ... src_files={'Deck.h': 19, 'Layer.h': 18, 'Autopilot.cpp': 16, 'TriggerCommands.h': 3, 'UndoManager.cpp': 3}
R2 rc=66 time=9.06s races=5 | All tests passed (5 assertions in 1 test case) | src: ... src_files={'Deck.h': 5, 'Layer.h': 4}
R4 rc=0 time=0.21s races=0 | All tests passed (2 assertions in 1 test case) | src: reports=0 kinds={} reports_with_src_frame=0 src_files={}
R3 rc=66 time=3.70s races=4 | All tests passed (9 assertions in 1 test case) | src: ... src_files={'ManualWrite.cpp': 4, ...}
```
- R4 (the tuple-only stress) is GREEN under TSan: 0 reports. R1's value checks now pass in both builds.
- Every remaining R1 / R2 report is a CLIP runtime field, not the tuple (first-frame classification of the 19 + 5:
  write size 8 = playheadPosition, 4 = beatsPlayed, 1 = playing at Layer.h:320 (the beforeCas activation tail inside
  updateRuntime), Layer.h:543 (the retrigger tail), TriggerCommands.h:83 (the playing restore), Autopilot.cpp:84
  (beatsPlayed += beats)); no 16-byte access is reported. They close at T4 (B2), R3 at T5 (B2).
- The activation-tail contract test under TSan: 3/3 runs rc=0, 0 warnings (HB from the release CAS to the acquire load
  holds for the clip fields).
Case counts at T2: test_layer_runtime 10, test_shared_field_types 1; ctest total 1079 (= 1055 + 24 new).

Amendment 4: the bounded-exhaustion test landed here (`updateRuntime: a bounded update gives up after exactly
maxAttempts and keeps the concurrent tuple`: 16 calls, applied false, the concurrent value intact). The CONTENTION test
is NOT here: it must REQUIRE "tick adopts > 0", which needs T3's `bool LayerClock::tick(Layer&, LayerRuntimeSnapshot&,
float)` (today's advanceCrossfade returns void) -- B2 writes it with T3.


### Mutants (amendment 16, B1's items) -- each from a normal build of a modified tree in build-lane, run once,
restored byte-identically (sha256 checked by the runner; `git diff --stat` empty after each)
- m1 casRuntime as a plain store (`runtime_.store(desired); return true;`), verbatim:
```
test_layer_runtime.cpp:145: FAILED:  CHECK_FALSE( L.casRuntime(stale, ticked) )
test_layer_runtime.cpp:146: FAILED:  CHECK( stale == trig.after )
test_layer_runtime.cpp:147: FAILED:  CHECK( L.runtime() == trig.after )
test_layer_runtime.cpp:148: FAILED:  CHECK( L.runtime() == LayerRuntimeSnapshot{ 2, 1, 0.0f, -1, Clip::BeatSnapMode::Off } )
test_layer_runtime.cpp:202: FAILED:  CHECK_FALSE( t.applied )
test_layer_runtime.cpp:203: FAILED:  CHECK( calls == 16 )  with expansion:  1 == 16
test_layer_runtime.cpp:204: FAILED:  CHECK( L.runtime().activeClipColumn == 116 )  with expansion:  0 == 116
test cases:   10 |    8 passed | 2 failed
R4 (normal build) x3:  render observations 311534; I1 0 I2 4540 I3 0 I4 0 / 267746; I2 2790 / 337881; I2 3204 -- test cases: 1 | 1 failed (3/3)
```
- m2 TriggerClipCmd::execute() without the first-call skip, verbatim:
```
D1 ...  test_undo_commands.cpp:3108: FAILED:  CHECK( captureLayerRuntime(L).crossfadeProgress == Catch::Approx(0.2f) )  with expansion:  0.0f == Approx( 0.20000000298023224 )
D1b ... test_undo_commands.cpp:3136: FAILED:  CHECK( captureLayerRuntime(L) == ended )
D1c ... test_undo_commands.cpp:3158: FAILED:  CHECK( captureLayerRuntime(L).activeClipColumn == 1 )  with expansion:  0 == 1
        test_undo_commands.cpp:3159: FAILED:  CHECK( captureLayerRuntime(L).pendingTriggerColumn == -1 )  with expansion:  1 == -1
test cases:  80 |  77 passed | 3 failed
```
- m3 releaseMomentary without its pending branch, verbatim:
```
releaseMomentary: a release before the beat cancels the queued trigger, the ...
test_layer_runtime.cpp:272: FAILED:  CHECK( t.after == LayerRuntimeSnapshot{ 0, -1, 1.0f, -1, Clip::BeatSnapMode::Off } )
test_layer_runtime.cpp:275: FAILED:  CHECK( L.runtime().activeClipColumn == 0 )  with expansion:  1 == 0
test_layer_runtime.cpp:276: FAILED:  CHECK( L.clips[1]->playing == false )
test cases:  3 |  2 passed | 1 failed
```
- m4 the activation tail moved AFTER the CAS (in updateRuntime: beforeCas called only after a successful CAS): it
  fired, rarely, as the ruling predicted -- 3 of 30 runs (and 4 of 20 in a first batch):
  `activations seen 20000, stale beatsPlayed seen 1 -- test cases: 1 | 1 failed`; the lane: 30/30 pass. The order is
  therefore ALSO a review item (Layer::updateRuntime calls beforeCas before casRuntime).
- Rig incident (fixed, recorded): after m1-m4 the runner's restore-then-rebuild happened inside the same wall-clock
  second as the mutant object, and this make's 1-second mtime check left the m4 object in place (the lane's
  activation-tail test then "failed" 3/20 -- the disassembly showed the tail stores after `caspal`, the mutant's
  shape). Fixed by `touch` + rebuild (disassembly re-checked: `str wzr, [x5, #0x688]` before `caspal`; 30/30 pass) and
  the runner now sleeps 1.2 s and touches the restored file. All lane numbers in this report come from clean builds.


### Full normal ctest (serial, stage end; `ctest --test-dir <wt>/build-lane -j1`, 09:07:18-09:08:20 after a full
build 09:05:24-09:07:18 rc=0), verbatim:
```
99% tests passed, 2 tests failed out of 1079
Total Test time (real) =  61.34 sec
The following tests FAILED:
	1062 - no std::cerr in code that runs off the message thread (Failed)
	1063 - Renderer.cpp: no plain store to a clip's playing; syncMedia goes through ClipTransportSync (Failed)
```
Both are pre-registered RED that later items turn GREEN: 1062 = D6 (T7, B3), 1063 = lint case 1 (T4, B2). Count
1079 = 1055 base + 24 new (T0 9: D1 / D1b / D1c, R1 / R2 / R4 / R3, D6, lint case 1; T1 4; T2 11).

### Non-mechanical test migrations
none. Every edit to an existing test was mechanical and compiler-driven (scratch fixreads.py / fixwrites.py): 175
tuple reads became `<layer>.runtime().<field>` (209 compiler-located member accesses minus the 34 writes); 34 tuple
writes (in 24 consecutive groups) became one
read-modify-write block `{ LayerRuntimeSnapshot rt = <layer>.runtime(); rt.<f> = <v>; ... <layer>.setRuntime(rt); }`
that keeps every other tuple field. No assertion, expected value or call order changed. The behaviour change of
amendment 6 (first execute() a no-op) changed no existing test's outcome: every existing TriggerClipCmd /
ClearActiveClipCmd test live-applies before perform (the ruling's M-A4 list), and the stale-coordinate tests are
no-ops either way.

### Notebook notes (for Harmony to append)
- 2026-10-02 -- mutant restore inside the build's second leaves the mutant object | Files: any mutant run against
  build-lane | this make (Apple GNU make 3.81, CMake Unix Makefiles) compares mtimes at 1-second resolution: restoring
  a mutated header in the SAME second the mutant object was written leaves that object "up to date", so the "clean"
  rebuild relinks the mutant. Sleep >= 1.2 s before restoring and `touch` the restored file; verify by disassembly or
  a re-run. | discovered: s-rta-1002 B1 m4 (Layer.h).
- 2026-10-02 -- TSan race tests: keep the raced object on the HEAP, allocated by src/ code | Files:
  tests/test_layer_runtime_race.cpp | Apple clang 17's TSan reported 0 races on a Layer living on the main thread's
  stack (4/4 runs, thousands of torn tuples seen by value checks); on the heap it reports. With every Layer call
  inlined, a report's access stacks name only the test file, so "a src/ frame in any stack" comes from the allocation
  stack (Deck::initDefault, Deck.h). | discovered: R4 sizing.
- 2026-10-02 -- two worktrees' ctest at once collide on fixed temp files | Files: tests/test_preset_manager.cpp
  (tempPresetFile), tests/test_app_settings.cpp (getNonexistentChildFile) | 5 cases failed once in a full ctest while
  the bt2 lane ran its own, and passed on isolated re-runs (INFERRED cause). | discovered: T0 normal RED run.
- 2026-10-02 -- the Layer trigger tuple is one private atomic word | Files: src/model/Layer.h | read it with ONE
  runtime() per use, write it only through the Layer API (trigger*/clearActiveClip/releaseMomentary/updateRuntime/
  setRuntime); a test that needs a state writes `L.setRuntime({...})`. captureLayerRuntime / applyLayerRuntime are
  the compat wrappers. | discovered: T2.

### next_stage_notes for B2 (T3 + T4 + T5)
- Heads: T0 = 2c04a03e59304efca134df617f7e4ff05675bb8f, T1 = 7422e10d38e5f9ce049724ee0d50b9d525dfd950, T2 =
  3fec609a4e1c4b23f0ec7e9b710322a99d7d30f8, then the report commit (docs) on top. Branch lane/tsan, not merged/pushed.
- Build dirs: <wt>/build-lane (normal Release, full build current at the T2 tree, ctest 1077/1079: the 2 RED are T4's
  lint case 1 and T7's D6) and <wt>/build-tsan (TSan RelWithDebInfo; only test_layer_runtime_race,
  test_manual_scalar_race, test_layer_runtime built). Reconfigure after a tests/CMakeLists.txt change. Scratch helper:
  <scratchpad>/tsan-B1/run-tsan-cases.sh <build-dir> <out-dir> (direct runs with exit codes + src-frame counts).
- Left of T2: the amendment-4 CONTENTION test (needs T3's `bool LayerClock::tick(Layer&, LayerRuntimeSnapshot&, float)`
  to count adopts) -- write it in tests/test_layer_runtime.cpp with T3.
- T3 starting point: LayerClock::advanceCrossfade is ALREADY load + ONE casRuntime (adopt-on-fail), inline; split it
  into advanced() / tick() per plan T3. CompositorEngine (incomingImagePending, renderLayerStages' observe,
  applyTransition), DeckClock.h, Renderer.cpp:593 and Autopilot.cpp currently take runtime() PER SITE (compile-forced,
  today's read count) -- T3 makes it one load per layer + lint case 2 (pinned counts). Autopilot's 3 triggerClip calls
  already pass kRenderTriggerAttempts = 16 and processPendingTrigger defaults to 16 (amendment 7);
  render_pending_fired / render_autopilot_advances need Autopilot to report t.changed() (amendment 13, T3).
- T4: test_render_thread_lint case 1 counts `ClipTransportSync::<name>(` call sites (>= 2) and forbids `.playing =`,
  `->playing =`, `.playing.store(` / `->playing.store(` in Renderer.cpp (line comments stripped) -- so make
  ClipTransportSync a namespace (or a class with static entry points) and call it qualified. Extend
  tests/test_shared_field_types.cpp with the Clip pins. The activation tail (Layer::applyActivationTail) and
  TriggerClipCmd's playing restore are the remaining R1 / R2 TSan reports (clip fields, write sizes 8 / 4 / 1) and
  must close with the Relaxed fields; the `c->beatsPlayed = 1000` writes in test_layer_runtime's contract test become
  Relaxed stores (no test change needed).
- T5: R3 (test_manual_scalar_race) is the only [tsan] case for family D.
- .harmony/probe-tsan-unit.sh hardcodes the [tsan] targets (TARGETS=(test_layer_runtime_race test_manual_scalar_race)):
  add any new tsan-labelled target there.
- Gotchas: (1) the mutant-restore mtime trap above (sleep 1.2 s + touch); (2) Layer::runtime_ is PRIVATE -- tests use
  runtime()/setRuntime(); (3) LayerRuntimeSnapshot now lives in Layer.h (DeckCommands.h only wraps it); (4) the T0
  normal-build bar deviation (R1 / R4 also RED on the base by value checks) is Harmony's to rule.



## B2 (T3 + T4 + T5) -- Builder, started 2026-10-02 09:11 EDT

STATUS: DONE (B2 items T3, T4, T5 committed; all four [tsan] cases GREEN; full normal ctest 1084/1085 -- the one
failure is D6, T7's pre-registered RED for B3)
Base for B2: 738af45 (B1 report commit). INBOX-RECHECK: 0 addenda folded (none received).
B2 heads: T3 = 9d9444e, T4 = 3400c08, T5 = 2e72850, then this report commit (docs) on top. Branch lane/tsan; not
merged, not pushed. No app was launched in B2 (no live lock taken; no Output window; no UNC check needed).

### T3 -- render path (one load per layer, one CAS per fade tick) (commit 9d9444e)
Files (src): src/render/LayerClock.h, src/render/DeckClock.h, src/render/CompositorEngine.{h,cpp},
src/render/CrossfadeHistory.h (comment only), src/render/Renderer.{h,cpp}, src/model/Autopilot.{h,cpp},
src/api/ApiServer.cpp (/api/state, one hunk at :1487 -- inside this lane's 1370-1544), src/test/TestServer.cpp (the
/api/state twin). Tests: tests/test_layer_runtime.cpp (+2 cases), tests/test_render_thread_lint.cpp (+ case 2).
What landed:
- LayerClock: `advanced(rt, transitionSpeed, dt)` (pure: the old math), `bool tick(Layer&, LayerRuntimeSnapshot& rt,
  float dt)` (next == rt -> true with no CAS; else ONE casRuntime: success rt = next, failure rt = the concurrent
  tuple and false = adopt), `bool advanceCrossfade(Layer&, float)` = load + tick (tests only now).
- CompositorEngine: compositeDeck and compositePersistentLayers take ONE `layer.runtime()` per layer where the first
  read sat (Fork 6): `clip = getClipAt(rt.active)`; `if (!incomingImagePending(layer, rt, clip) &&
  !advanceCrossfade(layer, rt, dt)) clip = getClipAt(rt.active)` (an adopt re-fetches the clip). renderLayerStages /
  applyTransition take `const LayerRuntimeSnapshot& rt` (observe(rt.previous, rt.active, rt.progress); the outgoing /
  incoming clips via getClipAt(rt.previous / rt.active); u_crossfadeProgress = rt.progress). advanceCrossfade is now a
  member: LayerClock::tick + `++tupleAdopts_` on false; takeTupleAdopts() (GL thread) drains it. hasActiveLayers /
  hasPersistentContent keep getActiveClip() (one load each, R-A9).
- DeckClock::tick: one runtime() per layer, tick unless fadeOwnedElsewhere, active / outgoing clips from the same rt;
  returns the adopt count.
- Renderer.cpp: the MilkDrop playlist loop drops its runtime() pre-check (getActiveClip() alone, null when inactive:
  same result, one load). Amendment 13: `std::atomic<uint64_t> renderPendingFired_ / renderAutopilotAdvances_ /
  renderTupleAdopts_` (relaxed fetch_add on the GL thread; getters), fed by Autopilot::FrameReport (both
  processFrame calls), DeckClock::tick's return and compositor_.takeTupleAdopts() once per frame (inside deckActive).
  /api/state (ApiServer AND TestServer, which mirrors it): render_pending_fired, render_autopilot_advances,
  render_tuple_adopts.
- Autopilot: one runtime() per layer per pass (end-of-video, pending, beat); advanceClip / smartAdvanceClip take the
  column from it (PlaySpecific: getClipAt(currentCol), no second load) and return t.changed(); processFrame(deck,
  snap, FrameReport* = nullptr) counts pendingFired (processPendingTrigger(...).changed()) and advances. Its triggers
  keep maxAttempts 16; processPendingTrigger's default 16 (amendment 7). The fade tick stays ONE attempt.
- CrossfadeHistory.h: the "Layer fields written without atomics / every write ordering" comment rewritten (one
  tuple per layer per frame; an adopt hands observe() a new pair).
- beatsPlayed stays a plain int at T3 (`++` / `+=`): its fetchAdd lands with the Relaxed type at T4.

New tests (GREEN-only; teeth = the amendment-16 mutant "tick as a plain store", below):
- test_layer_runtime `LayerClock::tick on a stale tuple after a concurrent trigger adopts it and keeps the new
  fade's previous` (the stale tick would end the fade: (1,-1,1.0); it must return false, rt == trigger (2,1,0.0),
  previous stays 1, the next tick advances to 0.2; no fade = no CAS).
- test_layer_runtime `contention: GL-thread fade ticks and beat-fired triggers vs a message-thread trigger storm`
  (amendment 4, B1's hand-over): start barrier; GL thread = load, R4 I1 / I2, LayerClock::tick (adopts counted),
  processPendingTrigger(0, 0, 16); message thread = cyclic triggers, alternately immediate / queued (forced Beat),
  waiting for each queued one to fire; every transition == its intent; REQUIRE adopts > 0; I1 == I2 == 0. Normal
  build, verbatim: `triggers 2, GL ticks 3358, adopts 1, fired 1; intent mismatches 0, I1 0 I2 0` -- All tests passed.
- test_render_thread_lint case 2 `render thread: one trigger-tuple load per layer per pass (pinned counts)`:
  CompositorEngine.cpp runtime() 2 / getActiveClip( 2; Renderer.cpp 0 / 1; DeckClock.h 1 / 0; Autopilot.cpp 3 / 0
  (`.` or `->` runtime(), line comments stripped; each site's function in the file comment).

RED (lint case 2, the current test compiled against the PRE-CHANGE sources of 738af45 copied to scratch), verbatim:
```
  CHECK( runtimeLoads == pin.runtimeLoads )  with expansion:  3 == 2   render/CompositorEngine.cpp: runtime() 3, getActiveClip( 5 at lines 401 991 ...
  CHECK( activeClipLoads == pin.activeClipLoads )  with expansion:  5 == 2
  CHECK( runtimeLoads == pin.runtimeLoads )  with expansion:  1 == 0   render/Renderer.cpp: runtime() 1, getActiveClip( 1 at lines 593 595
  CHECK( activeClipLoads == pin.activeClipLoads )  with expansion:  1 == 0   render/DeckClock.h: runtime() 1, getActiveClip( 1 at lines 33 37
  CHECK( runtimeLoads == pin.runtimeLoads )  with expansion:  5 == 3   model/Autopilot.cpp: runtime() 5, getActiveClip( 3 at lines 21 41 43 61 74
  CHECK( activeClipLoads == pin.activeClipLoads )  with expansion:  3 == 0
test cases:  1 | 1 failed
assertions: 12 | 6 passed | 6 failed
```
GREEN (normal build, full build 09:17:59-09:18:25 rc=0; targeted test binaries), verbatim:
```
test_layer_runtime         All tests passed (1081 assertions in 12 test cases)
test_render_thread_lint    test cases:  2 |  1 passed | 1 failed      <- case 1 = T4's pre-registered RED; case 2 alone: All tests passed (12 assertions in 1 test case)
test_deck_clock            All tests passed (42 assertions in 6 test cases)
test_autopilot             All tests passed (49 assertions in 11 test cases)
test_crossfade_history     All tests passed (39 assertions in 7 test cases)
test_compositor            All tests passed (47 assertions in 9 test cases)
test_layer_runtime_race    All tests passed (1189 assertions in 3 test cases)
test_undo_commands         All tests passed (557 assertions in 80 test cases)
```
TSan build (direct runs with the ENVIRONMENT options) after T3, verbatim:
```
R1 rc=66 time=3.17s races=23 | All tests passed (1182 assertions in 1 test case) | src: reports=23 kinds={'data race': 23} reports_with_src_frame=23 src_files={'Deck.h': 23, 'Layer.h': 22, 'Autopilot.cpp': 20, 'UndoManager.cpp': 3, 'TriggerCommands.h': 3}
R2 rc=66 time=3.47s races=5 | All tests passed (5 assertions in 1 test case) | src: reports=5 kinds={'data race': 5} reports_with_src_frame=5 src_files={'Deck.h': 5, 'Layer.h': 2}
R4 rc=0 time=0.21s races=0 | All tests passed (2 assertions in 1 test case) | src: reports=0 kinds={} reports_with_src_frame=0 src_files={}
R3 rc=66 time=3.60s races=4 | All tests passed (9 assertions in 1 test case) | src: ...
test_layer_runtime [layer_clock] rc=0 races=0 | All tests passed (16 assertions in 2 test cases)
```
T3 closes no [tsan] case by itself: every remaining R1 / R2 report is a CLIP runtime field write (access sizes 1 / 4 / 8
= playing / beatsPlayed / playheadPosition; frames Layer.h:320 / :537 / :543 (the activation tail), Layer.h:401,
Autopilot.cpp:70 (processPendingTrigger's tail) / :93 (beatsPlayed +=), TriggerCommands.h:83) -> T4. R4 stays GREEN.

### T4 -- Clip runtime fields + ClipTransportSync (commit 3400c08)
Files (src): src/model/Clip.h, NEW src/render/ClipTransportSync.h, src/render/Renderer.cpp (syncMedia: video and
sequence branches; + include), src/model/Autopilot.cpp (fetchAdd), src/ui/LayerStrip.cpp (atomic_ref -> .load() +
comment), src/connect/ConnectionEngine.h (the L5 comment), src/api/ApiServer.cpp (/api/composition :437 / :439
`clip.playing.load()` / `clip.playheadPosition.load()` -- juce::var has no conversion from Relaxed<T>; inside this
lane's 380-440). Tests: NEW tests/test_clip_transport_sync.cpp (3 cases), tests/test_shared_field_types.cpp (+4 Clip
pins, + #include "model/Relaxed.h"), tests/CMakeLists.txt (test_clip_transport_sync appended at the END).
What landed:
- Clip: `mutable RelaxedBool playing`, `mutable RelaxedDouble playheadPosition`, `RelaxedInt beatsPlayed`,
  `RelaxedBool hasBeenTriggered` (+ a comment: per-field atomics, never a consistent unit; the tuple is). Everything
  else compiled through the implicit conversions; the only compile fallout was ApiServer :437 / :439.
- ClipTransportSync (namespace, pure, templated on the player): `bool pushIntent(const Clip&, Player&)` (read the
  intent ONCE, push it as before) and `void writeBack(const Clip&, Player&, bool wanted)` (ph = player playhead ->
  store; `bool e = wanted; wrote = playing.compareExchange(e, player.isPlaying())`; the out-point test on the LOCAL ph;
  OneShot stop = CAS of the value just written (only if this sync wrote it) -> false + player.setPlaying(false); Loop /
  PingPong = seek to inPoint + store). The M-A8 ABA note is in the header (grep-verified: outside src/media the only
  setPlaying callers are syncMedia's, now this header's).
- Renderer::syncMedia: video AND sequence branch each call `ClipTransportSync::pushIntent` + `ClipTransportSync::
  writeBack` (4 qualified call sites; no plain store to `playing` left). Call order inside each branch unchanged.
- Autopilot: `clip->beatsPlayed.fetchAdd(1) + 1` (end-of-video loop counter) and `fetchAdd(beats) + beats` (beat pass);
  the threshold tests use the fetchAdd result (no re-load).
Parity note: with no concurrent intent change the CAS always succeeds (expected == the value read), so the model ends
exactly as the old plain stores left it, incl. the OneShot stop.

RED: R1 / R2 under TSan (the T3 run above: 23 / 5 clip-field reports) and lint case 1 (T0). Type pins: the CURRENT
test_shared_field_types.cpp compiled (-fsyntax-only, the target's own compile command) against the src/ of 9d9444e
(T3 head) -- verbatim:
```
test_shared_field_types.cpp:28:15: error: static assertion failed due to requirement 'std::is_same_v<bool, Relaxed<bool>>': Clip::playing must be RelaxedBool (Pitfall 63)
test_shared_field_types.cpp:29:15: error: static assertion failed due to requirement 'std::is_same_v<double, Relaxed<double>>': Clip::playheadPosition must be RelaxedDouble (Pitfall 63)
test_shared_field_types.cpp:31:15: error: static assertion failed due to requirement 'std::is_same_v<int, Relaxed<int>>': Clip::beatsPlayed must be RelaxedInt (Pitfall 63)
test_shared_field_types.cpp:32:15: error: static assertion failed due to requirement 'std::is_same_v<bool, Relaxed<bool>>': Clip::hasBeenTriggered must be RelaxedBool (Pitfall 63)
compile exit: 1
```
test_clip_transport_sync is GREEN-only (its header does not exist before T4); its teeth = mutant m6 below.
GREEN (normal build: reconfigure 09:23:34-09:23:35, full build 09:25:10-09:26:53 rc=0), verbatim:
```
test_clip_transport_sync   All tests passed (13 assertions in 3 test cases)
test_shared_field_types    All tests passed (1 assertion in 1 test case)
test_render_thread_lint    All tests passed (15 assertions in 2 test cases)      <- lint case 1 (T0 RED) now GREEN
test_layer_runtime         All tests passed (1081 assertions in 12 test cases)
test_layer_runtime_race    All tests passed (1189 assertions in 3 test cases)
test_autopilot             All tests passed (49 assertions in 11 test cases)
test_deck_clock            All tests passed (42 assertions in 6 test cases)
test_undo_commands         All tests passed (557 assertions in 80 test cases)
test_compositor            All tests passed (47 assertions in 9 test cases)
```
GREEN (TSan build: reconfigure 09:27:27, targets built 09:27:29-09:28:03 rc=0), direct runs, verbatim:
```
R1 rc=0 time=0.32s races=0 | All tests passed (1182 assertions in 1 test case) | src: reports=0 kinds={} reports_with_src_frame=0 src_files={}
R2 rc=0 time=0.28s races=0 | All tests passed (5 assertions in 1 test case) | src: reports=0 kinds={} reports_with_src_frame=0 src_files={}
R4 rc=0 time=0.23s races=0 | All tests passed (2 assertions in 1 test case) | src: reports=0 kinds={} reports_with_src_frame=0 src_files={}
R3 rc=66 time=3.73s races=4 | All tests passed (9 assertions in 1 test case) | src: reports=4 ... src_files={'ManualWrite.cpp': 4, 'Deck.h': 3, 'Composition.h': 3, 'Layer.cpp': 2, 'Clip.cpp': 1}
test_layer_runtime rc=0 races=0 | All tests passed (1081 assertions in 12 test cases)
test_clip_transport_sync rc=0 races=0 | All tests passed (13 assertions in 3 test cases)
```
and through ctest (G2's shape, `env -u TSAN_OPTIONS ctest --test-dir <wt>/build-tsan -L tsan --output-on-failure`):
```
1/4 Test #110: R1 message-thread triggers vs render clock / autopilot on one deck ......   Passed    0.29 sec
2/4 Test #111: R2 clip runtime fields: trigger writes vs render transport write-back ...   Passed    0.25 sec
3/4 Test #112: R4 tuple consistency and no lost fade under a paced trigger storm .......   Passed    0.22 sec
4/4 Test #113: R3 manual scalar writes vs eff() reads ..................................***Failed  Error regular expression found in output. Regex=[WARNING: ThreadSanitizer]  0.35 sec
75% tests passed, 1 tests failed out of 4
```
R1 / R2 (families B + C) are GREEN at T4. The short TSan times are real work, not skipped threads: `-s` runs show
`render frames 16267 / 17108 / 13289` (R1) and `104793 / 102427 / 96358` (R2) overlapping the 20,000 / 40,000 main
iterations; with zero reports TSan no longer pays for report symbolization (R1 was 3.17 s with 23 reports at T3).

### T5 -- manual scalars (family D) (commit 2e72850)
Files (src): src/model/Layer.h / Layer.cpp (7 fields + manualRef), src/model/Clip.h / Clip.cpp (7 + manualRef),
src/model/Composition.h (9 + the inline manualRef), src/connect/ManualWrite.h (ManualSlot; + #include
"model/Relaxed.h") / ManualWrite.cpp (:187 store), src/MainComponent.cpp (:2056 only -- the routine engine's read
lambda; outside every bt2 fence: `git diff -U0` hunk head `@@ -2056 +2056 @@`). Compile-forced `.load()` (mechanical:
a ternary `cond ? float : RelaxedFloat` is ambiguous, `std::max(0.01f, RelaxedFloat)` cannot deduce):
src/ui/LayerStrip.cpp:818, src/ui/TopBar.cpp:399 / :410, src/ui/ClipInspector.cpp:1375, src/ui/LayerInspector.cpp:970.
src/connect/ConnectionEngine.cpp needed no edit (toNorm(manualRef(...)) converts implicitly). Tests:
tests/test_shared_field_types.cpp (+3 manualRef return-type pins, + #include "model/Composition.h").
What landed:
- The 23 manualRef fields are RelaxedFloat: Layer opacity / positionX / positionY / layerScale / layerRotation /
  layerAnchorX / layerAnchorY; Clip clipOpacity / positionX / positionY / scale / rotation / anchorX / anchorY;
  Composition masterOpacity / masterSpeed / masterSignal / compPositionX / compPositionY / compScale / compRotation /
  compAnchorX / compAnchorY. The three manualRef overloads return RelaxedFloat& (their unreachable dummy too).
- ControlRef::manual is a ManualSlot {RelaxedFloat* atomicField; float* plainField; implicit ctors from either
  pointer; load(); store() (const: it writes THROUGH the pointer, as `*r.manual = ...` did through a const
  ControlRef); explicit operator bool; operator== against either pointer type}. Effect params / dryWet / source
  params / macros keep the plain pointer (R5). manualWriteCore: `r.manual.store(...)`; the routine read lambda:
  `ref->manual.load()`. eff() keeps its shape (its argument is now one relaxed load).
- The ManualSlot operator== lets tests/test_manual_write.cpp's 9 `REQUIRE(ref->manual == &<field>)` lines compile
  UNCHANGED (they still check which field a control resolves to) -- no test edit.

RED: R3 under TSan (T0, and still RED after T4: 4 reports, ManualWrite.cpp / Layer.cpp / Clip.cpp / Composition.h).
Type pins: the CURRENT test_shared_field_types.cpp compiled against the src/ of 3400c08 (T4 head), verbatim:
```
test_shared_field_types.cpp:39:15: error: static assertion failed due to requirement 'std::is_same_v<float &, Relaxed<float> &>': manualRef(Clip&) must return RelaxedFloat& (Pitfall 63)
test_shared_field_types.cpp:41:15: error: static assertion failed due to requirement 'std::is_same_v<float &, Relaxed<float> &>': manualRef(Layer&) must return RelaxedFloat& (Pitfall 63)
test_shared_field_types.cpp:43:15: error: static assertion failed due to requirement 'std::is_same_v<float &, Relaxed<float> &>': manualRef(Composition&) must return RelaxedFloat& (Pitfall 63)
compile exit: 1
```
GREEN (normal build: keep-going builds 09:31:12-09:34:11, rc=0 after the 5 compile-forced .load() edits; app binary
09:34), verbatim:
```
test_manual_write              All tests passed (83 assertions in 11 test cases)
test_manual_scalar_race        All tests passed (9 assertions in 1 test case)
test_shared_field_types        All tests passed (1 assertion in 1 test case)
test_connection                All tests passed (159 assertions in 39 test cases)
test_composition_tier_oracle   All tests passed (25 assertions in 5 test cases)
test_layer_runtime_race        All tests passed (1189 assertions in 3 test cases)
test_layer_runtime             All tests passed (1081 assertions in 12 test cases)
test_clip_transport_sync       All tests passed (13 assertions in 3 test cases)
```
GREEN (TSan build, targets built 09:34:46-09:35:01 rc=0), direct runs + ctest, verbatim:
```
R1 rc=0 time=0.28s races=0 | All tests passed (1182 assertions in 1 test case) | src: reports=0 kinds={} reports_with_src_frame=0 src_files={}
R2 rc=0 time=0.23s races=0 | All tests passed (5 assertions in 1 test case) | src: reports=0 kinds={} reports_with_src_frame=0 src_files={}
R4 rc=0 time=0.21s races=0 | All tests passed (2 assertions in 1 test case) | src: reports=0 kinds={} reports_with_src_frame=0 src_files={}
R3 rc=0 time=0.24s races=0 | All tests passed (9 assertions in 1 test case) | src: reports=0 kinds={} reports_with_src_frame=0 src_files={}
1/4 Test #110: R1 message-thread triggers vs render clock / autopilot on one deck ......   Passed    0.27 sec
2/4 Test #111: R2 clip runtime fields: trigger writes vs render transport write-back ...   Passed    0.23 sec
3/4 Test #112: R4 tuple consistency and no lost fade under a paced trigger storm .......   Passed    0.21 sec
4/4 Test #113: R3 manual scalar writes vs eff() reads ..................................   Passed    0.23 sec
100% tests passed, 0 tests failed out of 4
```
and the committed runner on the lane TSan dir (`env -u TSAN_OPTIONS bash .harmony/probe-tsan-unit.sh <wt>/build-tsan`):
`100% tests passed, 0 tests failed out of 4` / `probe-tsan-unit: ctest rc=0 2026-10-02 09:35:10`.
ALL FOUR [tsan] cases (R1, R2, R3, R4) are GREEN at the end of B2. (probe-tsan-unit.sh's TARGETS list needed no
change: B2 added no tsan-labelled target.)

### Mutants (amendment 16, B2's items) -- each a normal build of a modified tree in build-lane, run once,
restored byte-identically (sha256 checked by the runner, sleep 1.2 s + touch before the clean rebuild; `git diff --stat
-- src tests` empty after each; the lane binaries re-run green after the restore)
- m5 tick as a plain store (LayerClock::tick: `layer.setRuntime(next); rt = next; return true;`), verbatim:
```
test_layer_runtime.cpp:325: FAILED:  CHECK_FALSE( LayerClock::tick(L, rt, 0.1f) )
test_layer_runtime.cpp:326: FAILED:  CHECK( rt == trig.after )
test_layer_runtime.cpp:327: FAILED:  CHECK( L.runtime() == trig.after )
test_layer_runtime.cpp:328: FAILED:  CHECK( L.runtime().previousClipColumn == 1 )  with expansion:  -1 == 1
test_layer_runtime.cpp:331: FAILED:  CHECK( rt == LayerRuntimeSnapshot{ 2, 1, 0.2f, -1, Clip::BeatSnapMode::Off } )
contention: ... test_layer_runtime.cpp:402: FAILED:  REQUIRE_FALSE( stalled )  with message:  triggers 6, GL ticks 978144323, adopts 0, fired 2; intent mismatches 0, I1 0
test cases:  2 | 2 failed
R4 (normal build) x3:
  render observations 327798; I1 0 I2 11725 I3 0 I4 0   test cases: 1 | 1 failed
  render observations 321304; I1 0 I2 12053 I3 0 I4 0   test cases: 1 | 1 failed
  render observations 326460; I1 0 I2 12112 I3 0 I4 0   test cases: 1 | 1 failed
```
  NOTE vs the ruling's wording ("fails R4 I3"): R4 fails 3/3, but through I2, not I3 -- the plain tick overwrites the
  trigger before the render ever observes it (a LOST activation, so no "first observation of a new active" exists for
  I3 to judge); the next cyclic trigger then installs previous = the column before the lost one, which I2 counts. The
  contention test stalls under the mutant (a fired queued trigger is overwritten, so the message thread waits for an
  activation that never stands) and fails by REQUIRE_FALSE(stalled).
- m6 the syncMedia write-back as a plain store (ClipTransportSync::writeBack: `clip.playing.store(now); wrote = true`),
  verbatim:
```
test_clip_transport_sync.cpp:48: FAILED:  CHECK( clip.playing.load() == true )  with expansion:  false == true
test_clip_transport_sync.cpp:51: FAILED:  CHECK( next == true )  with expansion:  false == true
test_clip_transport_sync.cpp:52: FAILED:  CHECK( player.isPlaying() )  with expansion:  false
test_clip_transport_sync.cpp:99: FAILED:  CHECK( clip.playing.load() == false )  with expansion:  true == false
test_clip_transport_sync.cpp:101: FAILED:  CHECK_FALSE( player.isPlaying() )
test cases:  3 | 1 passed | 2 failed
assertions: 13 | 8 passed | 5 failed
```
  (the trigger-in-window and pause-in-window cases fail; the OneShot case passes under the mutant, as expected: it
  has no concurrent intent change).

### Full normal ctest (B2 stage end; serial, under the H12 cross-lane mutex /tmp/audiodna-ctest.lock;
`ctest --test-dir <wt>/build-lane -j1`, build 09:36:06-09:36:10 rc=0 (no-op), ctest 09:36:10-09:37:16), verbatim:
```
99% tests passed, 1 tests failed out of 1085
Total Test time (real) =  65.83 sec
The following tests FAILED:
	1062 - no std::cerr in code that runs off the message thread (Failed)
```
1062 = D6, T7's pre-registered RED (B3). Lint case 1 (1063 at B1) is now GREEN (T4). Count 1085 = 1079 (B1) + 6
(T3: test_layer_runtime +2, test_render_thread_lint case 2 +1; T4: test_clip_transport_sync 3). Per-target case counts
of the lane's targets now (G1): test_layer_runtime_race 3, test_manual_scalar_race 1, test_layer_runtime 12,
test_clip_transport_sync 3, test_relaxed 4, test_log_line_lint 1, test_render_thread_lint 2, test_shared_field_types 1
(+ test_undo_commands' D1 / D1b / D1c = 3 in an existing target). 1085 = 1055 + 30.

### Non-mechanical test migrations (B2)
none. B2 edited no existing test's body. Two existing-test notes (no edit, meaning unchanged):
- tests/test_manual_write.cpp: its 9 `REQUIRE(ref->manual == &<field>)` lines now compare through
  ManualSlot::operator== (RelaxedFloat* / float* overloads) -- still "which field does this control resolve to".
- tests/test_layer_runtime_race.cpp R2: its plain-syntax mirror (`c->playing = ...`, `c->beatsPlayed =
  c->beatsPlayed + 1`) now compiles to relaxed atomic operations through Relaxed<T>, exactly as T0 designed it.

### Rig notes (B2)
- 09:21 an accidental `bash configure.sh` with NO arguments (a typo'd `| head -0` pipe): checked at once -- no
  CMakeCache.txt / CMakeFiles appeared in the main checkout, its build/ (CMakeCache.txt still Sep 25 20:41) or the
  worktree root, and `git -C <main> status` is identical to the session start. Harmless; recorded for completeness.
- Observation (not this builder's): main's build/CMakeFiles directory mtime is 2026-10-02 08:11 (before B2 started,
  matching B1's first configure at 08:11:41 that reads build/_deps as FETCHCONTENT_SOURCE_DIR_*). Harmony may want to
  check that a FETCHCONTENT_SOURCE_DIR configure does not touch main's build/.
- A post-commit "[graphify hook] launching background rebuild" printed after each commit (a user-level hook); the
  worktree stayed clean (`git status` showed only the report file and build-lane/).

### Notebook notes (for Harmony to append)
- 2026-10-02 -- Relaxed<T> compile fallout patterns | Files: src/model/Relaxed.h and any model field converted to it |
  juce::var has NO conversion from Relaxed<T> (`setProperty("x", clip.playing)` fails: write `.load()`); a ternary
  `cond ? float : RelaxedFloat` is ambiguous (both convert); `std::max(0.01f, relaxedField)` cannot deduce -- all
  three need `.load()`. Plain reads, arithmetic, assignment and Catch2 CHECK / Approx compile unchanged. |
  discovered: s-rta-1002 B2 T4 / T5 (ApiServer.cpp:437/439, TopBar.cpp:399/410, LayerStrip.cpp:818,
  ClipInspector.cpp:1375, LayerInspector.cpp:970).
- 2026-10-02 -- the GL thread reads a layer's trigger tuple ONCE per layer per pass | Files: src/render/
  CompositorEngine.cpp, src/render/DeckClock.h, src/render/Renderer.cpp, src/model/Autopilot.cpp,
  tests/test_render_thread_lint.cpp (case 2) | pass the loaded LayerRuntimeSnapshot down (renderLayerStages /
  applyTransition / incomingImagePending take `rt`); publish the fade with LayerClock::tick(layer, rt, dt) (ONE CAS;
  false = adopt, rt = the trigger's tuple, re-fetch the clip). A new `.runtime()` / `getActiveClip(` in those four
  files fails the pinned-count lint until re-justified. | discovered: T3.
- 2026-10-02 -- a render write-back of a message-thread intent is a CAS on the value read | Files:
  src/render/ClipTransportSync.h, src/render/Renderer.cpp (syncMedia) | pushIntent reads `playing` once; writeBack
  CASes it to the player's state, tests the out-point on the local playhead, and a OneShot stop CASes only the value
  this sync wrote. Never store `playing` plainly in Renderer.cpp (test_render_thread_lint case 1). | discovered: T4.

### next_stage_notes for B3 (T6 + T7 + T8 + docs + mutant evidence + lane smoke)
- Heads: T3 9d9444e, T4 3400c08, T5 2e72850, then the B2 report commit. Build dirs: <wt>/build-lane (normal Release,
  full build current at T5, ctest 1084/1085: only 1062 D6 RED = T7) and <wt>/build-tsan (TSan RelWithDebInfo,
  reconfigured at 09:27 after T4's CMakeLists change; built targets: test_layer_runtime_race, test_manual_scalar_race,
  test_layer_runtime, test_clip_transport_sync). Reconfigure both after any tests/CMakeLists.txt change.
- [tsan] state: R1, R2, R3, R4 all GREEN (0 warnings, ctest -L tsan 4/4, probe-tsan-unit.sh rc=0 on build-tsan).
  probe-tsan-unit.sh's TARGETS list is unchanged (B2 added no tsan-labelled target); add one there if B3 does.
- T6 (amendment 9): Composition::activeDeckIndex -> RelaxedInt, getActiveDeck() one load, juce::var sites need
  `.load()` (var has no Relaxed conversion -- see the notebook note; ApiServer :360 / :384 / :1540, Composition.h
  :306), Renderer.cpp:467-495 derives the index from the acquire-loaded deck pointer. Pin it in
  tests/test_shared_field_types.cpp (that file now already includes model/Composition.h). The Renderer.cpp:449-451
  "house class" comment is T6's to rewrite.
- T7: test_log_line_lint (1062) is the only RED left in the normal ctest. H3: VideoPlayer.cpp is in the list whole.
- Amendment-16 mutant evidence is COMPLETE across B1 + B2: m1 casRuntime plain store, m2 execute() without the
  first-call skip, m3 releaseMomentary without its pending branch, m4 activation tail after the CAS (fired 3/30),
  m5 tick as a plain store, m6 syncMedia write-back as a plain store. B3 only collects them for the docs / review.
  NOTE for the pitfall / docs wording: m5 fails R4 through I2 (a LOST activation), not I3 as the ruling worded it.
- Amendment 13 is wired: /api/state (ApiServer AND TestServer) render_pending_fired / render_autopilot_advances /
  render_tuple_adopts (Renderer::getRender*; Autopilot::FrameReport out-param; DeckClock::tick returns adopts;
  CompositorEngine::takeTupleAdopts). Not yet seen live: B2 launched no app. H8 / G3.4's lane-d smoke is the first
  live read (Manual BPM via POST /api/set_bpm gives the beats). render_tuple_adopts is INFO only (G3.6).
- Docs inputs (amendment 14 + plan (6)) already true in code: CrossfadeHistory.h comment rewritten; LayerStrip.cpp
  :747 and ConnectionEngine.h L5 comments rewritten (playheadPosition is RelaxedDouble); ClipTransportSync.h carries
  the M-A8 ABA note; ManualSlot documents the R5 plain residue (effect params / dryWet / source params / macros).
  integration.md needs the three /api/state fields.
- Gotchas: the mutant-restore mtime trap (B1 note; B2's runner kept sleep 1.2 s + touch); ControlRef::manual is a
  ManualSlot (use load() / store(); `!ref->manual` still works); Relaxed<T> fallout patterns (notebook note).



## B3 (T6 + T7 + T8 + docs + lane evidence) -- Builder, started 2026-10-02 09:39 EDT

STATUS: PENDING
Base for B3: adeece2 (B2 report commit). INBOX-RECHECK: (pending)

### T6 -- activeDeckIndex (family E) (amendment 9)
Files (src): src/model/Composition.h (field + getActiveDeck const / non-const + toVar :308), src/render/Renderer.cpp
(the deck-switch detection block + the :449-451 "house class" comment), src/api/ApiServer.cpp (:360 /api/status, :384
/api/composition, :1547 /api/state -- `.load()`; inside this lane's 360 / 384 / 1370-1544+ hunks), src/test/TestServer.cpp
(:816, its /api/state twin, `.load()`), src/core/DeckCommands.h (:930), src/MainComponent.cpp (6 compile-forced `.load()`
in `(deckIndex < 0) ? composition_.activeDeckIndex : deckIndex` ternaries: hunk heads 4607, 4826, 5572, 6343, 6368, 6402
-- none inside bt2's fences 484-494 / 2144-2160 / 3070-3095 / 5690-5705). Tests: tests/test_shared_field_types.cpp (+1 pin).
What landed:
- `RelaxedInt activeDeckIndex = 0;` (+ a comment: message-thread writer, httplib readers, the GL never reads it).
- getActiveDeck() const and non-const: `const int idx = activeDeckIndex.load();` ONCE, then range check + subscript on idx.
- Renderer::renderOpenGL deck-switch detection: `if (composition_ && deck && !decks.empty() && deck in [data, data+size))
  currentDeckIdx = deck - decks.data()` (the autopilot block's arithmetic, :543-546), compared with prevActiveDeckIndex_
  exactly as before; no deck this frame keeps prevActiveDeckIndex_. No GL-thread read of the field remains (grep of
  src/render, src/model/Autopilot.cpp, src/output, src/analysis: only a comment names it).
- DEVIATION (mechanical, the ruling said "grep found no compound operators"): DeckCommands.h:930 `--comp->activeDeckIndex;`
  (RemoveDeckCmd, inside its fence) does not compile with Relaxed<T> (no compound operators by design) and became
  `comp->activeDeckIndex = comp->activeDeckIndex - 1;` (the same single-writer arithmetic).
- Every other reference (129 src / 92 test) compiled through the implicit conversions; the only fallout was the 6
  ternaries (the B2 notebook's ambiguous-ternary pattern) and the 4 juce::var sites.

RED (the pin, the current test_shared_field_types.cpp compiled -fsyntax-only with the target's own compile command against
the pre-T6 src = 2e72850 + the T6 test edit only), verbatim:
```
/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/tsan/tests/test_shared_field_types.cpp:48:15: error: static assertion failed due to requirement 'std::is_same_v<int, Relaxed<int>>': Composition::activeDeckIndex must be RelaxedInt (Pitfall 63)
1 error generated.
compile exit: 1
```
Family E's behavioural RED is the app sweep (Renderer.cpp:471 vs MainComponent.cpp:5525, fresh RED 3/3 c launches): no
[tsan] unit case covers it (Harmony's G3).
GREEN (normal build: build 09:41:40-09:43:07 rc=0), targeted binaries, verbatim:
```
test_composition           All tests passed (364 assertions in 27 test cases)
test_program_preamble      All tests passed (169 assertions in 6 test cases)
test_recorder_host         All tests passed (1806 assertions in 40 test cases)
test_routine_engine        All tests passed (1145 assertions in 37 test cases)
test_shared_field_types    All tests passed (1 assertion in 1 test case)
test_tempo_start           All tests passed (311 assertions in 13 test cases)
test_undo_commands         All tests passed (557 assertions in 80 test cases)
test_deck_clock            All tests passed (42 assertions in 6 test cases)
test_autopilot             All tests passed (49 assertions in 11 test cases)
test_compositor            All tests passed (47 assertions in 9 test cases)
```
(the tests that name activeDeckIndex / getActiveDeck, plus the render-adjacent targets). The renderer's deck-switch
detection is not linkable headless: its live witness is the H8 smoke's switch_deck steps + Harmony's G4
probe-deck-tabs / probe-deck-path.

### T7 -- std::cerr on worker threads -> logLine (family A; widened by H3)
Files (src, 14): analysis/AnalysisThread.cpp (4), api/ApiServer.cpp (3: :94 / :97 in the server-thread lambda, :124 stop()),
test/TestServer.cpp (4), render/Renderer.cpp (20), render/ImageDecode.h (2), render/TextureManager.cpp (1),
render/LUTLoader.cpp (3), recording/VideoRecorder.cpp (16), core/MediaOpener.cpp (1), media/ImageSequence.cpp (1),
output/SyphonOutput.mm (2), output/SharedFrameSet.cpp (2), output/OutputPresenter.cpp (2), media/VideoPlayer.cpp (14 =
EVERY statement in the file, H3) -- 75 statements, each file gains `#include "core/LogLine.h"` after its first #include.
src/audio/* untouched (`git diff --stat -- src/audio` empty). tests/test_log_line_lint.cpp needed no edit: its list
already holds VideoPlayer.cpp whole (B1 wrote it to H3) and its comment names the R8 residual (AudioEngine.cpp +
DeviceGuard.cpp, bt2's).
- Conversion (scratch cerr_convert.py, mechanical): `std::cerr << a << b ... << std::endl;` -> `logLine(a, b, ...);` with the
  top-level `<<` split outside strings / parentheses; multi-line statements keep their line breaks. The one statement
  that ended in a literal newline instead of std::endl (AnalysisThread.cpp:82, `<< " Hz\n";`) drops the `\n` from the
  literal (logLine appends it). No statement built a line across several cerr statements; no manipulator was used.
- Text byte-identity: a second script (cerr_verify.py) re-parses every new logLine call and compares its argument list
  with the HEAD statement's `<<` operands (endl / trailing \n normalized). Verbatim summary:
```
analysis/AnalysisThread.cpp: std::cerr statements on HEAD 4, logLine statements now 4, argument lists identical: True, std::cerr left: 0
api/ApiServer.cpp: std::cerr statements on HEAD 3, logLine statements now 3, argument lists identical: True, std::cerr left: 0
test/TestServer.cpp: std::cerr statements on HEAD 4, logLine statements now 4, argument lists identical: True, std::cerr left: 0
render/Renderer.cpp: std::cerr statements on HEAD 20, logLine statements now 20, argument lists identical: True, std::cerr left: 0
render/ImageDecode.h: std::cerr statements on HEAD 2, logLine statements now 2, argument lists identical: True, std::cerr left: 0
render/TextureManager.cpp: std::cerr statements on HEAD 1, logLine statements now 1, argument lists identical: True, std::cerr left: 0
render/LUTLoader.cpp: std::cerr statements on HEAD 3, logLine statements now 3, argument lists identical: True, std::cerr left: 0
recording/VideoRecorder.cpp: std::cerr statements on HEAD 16, logLine statements now 16, argument lists identical: True, std::cerr left: 0
core/MediaOpener.cpp: std::cerr statements on HEAD 1, logLine statements now 1, argument lists identical: True, std::cerr left: 0
media/ImageSequence.cpp: std::cerr statements on HEAD 1, logLine statements now 1, argument lists identical: True, std::cerr left: 0
output/SyphonOutput.mm: std::cerr statements on HEAD 2, logLine statements now 2, argument lists identical: True, std::cerr left: 0
output/SharedFrameSet.cpp: std::cerr statements on HEAD 2, logLine statements now 2, argument lists identical: True, std::cerr left: 0
output/OutputPresenter.cpp: std::cerr statements on HEAD 2, logLine statements now 2, argument lists identical: True, std::cerr left: 0
media/VideoPlayer.cpp: std::cerr statements on HEAD 14, logLine statements now 14, argument lists identical: True, std::cerr left: 0
total 75 files with mismatch 0
```
  With identical operands in identical order, logLine's `(os << ... << a)` + '\n' prints what the cerr chain + endl
  printed (the same operator<< overloads, incl. juce::String's and `const unsigned char*` for glGetString); T1's
  scratch check printed identical lines through both. The only difference: each line is one fwrite (stderr is unbuffered,
  so std::endl's flush has no counterpart to lose).
- `#include <iostream>` stays in every file that had it (other iostream users / transitive includes not audited: left as
  is, no drive-by removal).
- Thread context: the list is plan T7's + H3's (converted whole as directed: "if unsure, convert"). ApiServer :124 /
  TestServer :104 (stop()) and the Renderer / VideoRecorder start / stop lines run on the message thread too -- converting
  them is harmless (same text).
- Merge surface: ApiServer.cpp hunks `@@ -1,0 +2 @@` (the include), `@@ -94 +95 @@`, `@@ -97 +98 @@`, `@@ -124 +125 @@` --
  the include line and :124 are new hunks beside H5's :93-96 list; neither is near bt2's ~323 / ~2086+.

RED (D6, the pre-change tree): B1's T0 run (`test_log_line_lint: total std::cerr in the worker-thread list: 75
(assertions: 29 | 14 passed | 15 failed)`) and B2's stage-end full ctest on adeece2^ (`1062 - no std::cerr in code that
runs off the message thread (Failed)`, the only failure of 1085).
GREEN (normal build: build 09:45:12-09:46:02 rc=0), targeted binaries, verbatim:
```
test_log_line_lint             All tests passed (29 assertions in 1 test case)
test_image_decode              All tests passed (39 assertions in 5 test cases)
test_image_sequence_open       All tests passed (58 assertions in 2 test cases)
test_media_opener              All tests passed (58 assertions in 7 test cases)
test_video_player_open         All tests passed (10 assertions in 2 test cases)
test_video_player_gl           All tests passed (284 assertions in 5 test cases)
test_video_decode_trace        All tests passed (510 assertions in 5 test cases)
test_video_ring                All tests passed (683 assertions in 31 test cases)
test_video_upload_budget       All tests passed (180902 assertions in 7 test cases)
test_shared_frame_gl           All tests passed (90 assertions in 8 test cases)
test_gop_cache_store           All tests passed (662 assertions in 26 test cases)
test_clip_replace_media_retire All tests passed (18 assertions in 6 test cases)
test_seq_vram                  All tests passed (4889364 assertions in 21 test cases)
```
Family A has no [tsan] unit case (its behavioural RED / GREEN is the app sweep, G3; the H8 smoke below is the first
lane read).

### T8 -- probe-tsan tooling
(pending)

### Docs
(pending)

### Lane final evidence
(pending)

### Amendment-16 mutant evidence (B1-B3)
(pending)
