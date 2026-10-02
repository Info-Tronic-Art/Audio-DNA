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

STATUS: DONE (B3 items T6, T7, T8 + docs committed; full normal ctest 1085/1085 serial; probe-tsan-unit 4/4 GREEN with 0
TSan warnings; full TSan ctest 1085/1085; H8 lane-d smoke valid, 0 warnings, pending_fired 1 / autopilot_advances 26;
the analyzer keys all 25 fresh-RED uniques into A-E)
Base for B3: adeece2 (B2 report commit). INBOX-RECHECK: 0 addenda folded (none received).
T0 sha: 2c04a03e59304efca134df617f7e4ff05675bb8f

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

### T8 -- the TSan app-sweep tooling (in-repo; plan T8 + amendment 12 + G3.2 + H3)
Files: NEW .harmony/probe-tsan.sh, NEW .harmony/probe-tsan.py, NEW .harmony/probe-tsan-analyze.py (adapted from
.harmony/.reports/s-rta-0930/tsan-harness/ run_batch.sh / scen.py / mkfix.sh / analyze.py).
- probe-tsan.sh `<out-dir> "<N:scen@arm> ..."`: the PROBE RIG GATE + adna_pids / adna_running / adna_kill cloned from
  probe-crossfade.sh (refuses without /tmp/audiodna-live.lock/owner; AUDIODNA_LOCK_OWNER must match); the app per arm
  from TSAN_APP_<arm> (refuses a bundle without libclang_rt.tsan, a reused run N, a running Audio-DNA, a 7070 / 8080
  listener); fixtures via its mkfix (ffmpeg) into TSAN_MEDIA (default <out>/media); TSAN_OPTIONS
  `halt_on_error=0:abort_on_error=0:exitcode=0:report_signal_unsafe=0:history_size=<TSAN_HISTORY, default 4>:log_path=
  <run>/tsan` (G3); `open -g` only; per launch: built-in audio pre-check, health, the scenario under a 240 s alarm,
  alive at end, Output-named windows (must be 0), graceful quit (osascript; kill only after 30 s = invalid); per
  batch: UNC windows >= 15 s after the last quit and new Audio-DNA .ips; launches.tsv gains the arm + the validity
  fields + the three /api/state witnesses; a d launch whose final /api/state carries the witnesses is INVALID unless
  render_pending_fired >= 1 AND render_autopilot_advances >= 1 (G3.4); exit 0 only when every launch + the batch is
  valid.
- probe-tsan.py: scenarios a / b / c as the harness (unchanged REST sequence), d per amendment 12 (fixture: 2 decks x 3
  layers x 4 columns of v1080 videos + images, every layer transitionSpeed 0.5, deck 0 layer 1 autopilotEnabled +
  PlayNext + Beat1, deck 0 L0 col 2 beatSnapMode Beat, deck 1 L2 col 1 Bar, layer 2 col 3 empty on both decks, all at
  load; set_bpm 240; 48 steps 0.25 s apart: opacity every step (rotating), set_master_signal every other step,
  switch_deck every 8th, one trigger per step cycling trigger_column / a new cell / the same cell again / the empty cell
  / layer 1 / layer 2; /api/health only while stepping; ONE /api/state at the end -> state.json) and INFO e (d fixture
  + perf/record -> 3 triggers -> perf/stop -> perf/load -> perf/play -> perf/stop_play; removes the take folder it
  recorded). Every request is a fresh connection (requests.get/post per call, `Connection: close`). Choice of mine,
  named: d's composition sets globalTransitionSpeed 0.5 (the harness used 0.0) so the deck switches run the
  cross-deck transition branch T6 touched; amendment 12 does not fix this value.
- probe-tsan-analyze.py `<sweep> [--src ARM=PATH|ARM=git:REV] [--deps DIR] [--default-arm NAME] [--json F]`: key = the
  harness's (kind + top-4 Audio-DNA frames over src/ + _deps basenames of both access stacks); classes APP /
  UNATTRIBUTED / JUCE-SYSTEM per G3.2 (APP: a src/ frame in either access stack, the location's heap-allocation stack
  or either access thread's creation stack); FAMILY keys A-E from the top src/ frame of each access stack (source
  line + function, printed for every unique), A = location std::__1::cerr or a libc++ ostream frame with a side (access
  frame, else creation frame) in T7's list, PRE-EXISTING-R8 = both sides in audio/AudioEngine.cpp /
  audio/DeviceGuard.cpp only (H3), A-UNLISTED = any other cerr race; prints per launch, per unique (with stacks' top
  src lines), per arm x family x scenario, per arm x scenario x class. The archive format (rows without an arm) is
  read with --default-arm.
- Fix while sizing (caught on the first archive run): the access-header regex missed TSan's capitalised "Atomic read of
  size 8" (F3 / F10 keyed one side only); now `(Previous )?([Aa]tomic )?(Read|Write|read|write) of size`.

RED evidence (H8 part 2): the analyzer over the fresh-RED archive of the pre-change app
(.harmony/.reports/s-rta-1002/evidence-0930/sweep-red1-archive, 655d232, 9 launches a / b / c), source lines from
`--src main=git:655d232`, verbatim summary:
```
probe-tsan-analyze: /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/tsan/.harmony/.reports/s-rta-1002/evidence-0930/sweep-red1-archive -- 9 launches with reports, 63 warnings, 25 unique (src trees: main=git:655d232)
  A                uniques 4   launches 9    a 3/3 b 3/3 c 3/3
  B                uniques 12  launches 4    b 1/3 c 3/3
  C                uniques 7   launches 6    b 3/3 c 3/3
  D                uniques 1   launches 2    c 2/3
  E                uniques 1   launches 3    c 3/3
  reports by scenario x class: a/APP 3, b/APP 19, c/APP 41
```
All 25 uniques are APP and keyed into families A-E (none "-", none UNATTRIBUTED / JUCE-SYSTEM); the per-family launch
rates equal the ruling's G-A2 table (A 9/9; B c 3/3, b 1/3; C b + c 6/6; D c 2/3; E c 3/3):

| F | family | class | launches (scen) | side 1: top src frame, source line at 655d232 | side 2 |
|---|---|---|---|---|---|
| F1 | A | APP | 1(a),2(b),3(c),4(a),5(b),6(c),7(a),8(b),9(c) | ApiServer.cpp:93 [creation] `serverThread_ = std::thread([this, bindAddress]() {` | AnalysisThread.cpp:84 `<< ", bandwidth " << static_cast<int>(resampler_.inputBandwidthHz()) <` |
| F2 | A | APP | 2(b),5(b) | VideoPlayer.cpp:308 `<< " (" << width_ << "x" << height_` | VideoPlayer.cpp:307 `std::cerr << "[VideoPlayer] Opened: " << path` |
| F3 | C | APP | 2(b),3(c),5(b),6(c),8(b),9(c) | Renderer.cpp:1823 `clip->playheadPosition = player->getPlayheadPosition();` | LayerStrip.cpp:763 `const auto tv = transportViewOf(layer_, transportBounds_);` |
| F4 | A | APP | 3(c) | VideoPlayer.cpp:307 `std::cerr << "[VideoPlayer] Opened: " << path` | VideoPlayer.cpp:310 `<< ", " << duration_ << "s"` |
| F5 | B | APP | 3(c) | Renderer.cpp:593 `if (!layer // layer->activeClipColumn < 0) continue;` | TriggerCommands.h:68 `void execute() override { apply(after_, targetPlayingAfter_); }` |
| F6 | B | APP | 3(c),9(c) | CompositorEngine.cpp:401 `if (layer.crossfadeProgress >= 1.0f // layer.previousClipColumn < 0)` | TriggerCommands.h:68 `void execute() override { apply(after_, targetPlayingAfter_); }` |
| F7 | B | APP | 3(c),5(b),6(c),9(c) | Layer.h:250 `triggerClipImmediate(column);` | Renderer.cpp:593 `if (!layer // layer->activeClipColumn < 0) continue;` |
| F8 | B | APP | 3(c),5(b) | CompositorEngine.cpp:401 `if (layer.crossfadeProgress >= 1.0f // layer.previousClipColumn < 0)` | TriggerCommands.h:68 `void execute() override { apply(after_, targetPlayingAfter_); }` |
| F9 | E | APP | 3(c),6(c),9(c) | MainComponent.cpp:5525 `composition_.activeDeckIndex = deckIndex;` | Renderer.cpp:471 `const int currentDeckIdx = composition_->activeDeckIndex;` |
| F10 | C | APP | 3(c),6(c),9(c) | LayerStrip.cpp:763 `const auto tv = transportViewOf(layer_, transportBounds_);` | Renderer.cpp:1823 `clip->playheadPosition = player->getPlayheadPosition();` |
| F11 | B | APP | 3(c),6(c) | Layer.h:250 `triggerClipImmediate(column);` | CompositorEngine.cpp:991 `if (crossfadeStart_[clipKey].observe(layer.previousClipColumn, layer.a` |
| F12 | B | APP | 3(c),6(c) | Layer.h:250 `triggerClipImmediate(column);` | CompositorEngine.cpp:401 `if (layer.crossfadeProgress >= 1.0f // layer.previousClipColumn < 0)` |
| F13 | C | APP | 3(c) | Renderer.cpp:1823 `clip->playheadPosition = player->getPlayheadPosition();` | LayerStrip.cpp:763 `const auto tv = transportViewOf(layer_, transportBounds_);` |
| F14 | B | APP | 3(c),6(c) | Layer.h:250 `triggerClipImmediate(column);` | CompositorEngine.cpp:992 `layer.crossfadeProgress))` |
| F15 | C | APP | 3(c) | LayerStrip.cpp:763 `const auto tv = transportViewOf(layer_, transportBounds_);` | Renderer.cpp:1823 `clip->playheadPosition = player->getPlayheadPosition();` |
| F16 | B | APP | 5(b) | Renderer.cpp:1787 `if (clip->playing && !player->isPlaying())` | TriggerCommands.h:68 `void execute() override { apply(after_, targetPlayingAfter_); }` |
| F17 | C (also B) | APP | 5(b) | Renderer.cpp:1823 `clip->playheadPosition = player->getPlayheadPosition();` | Layer.h:250 `triggerClipImmediate(column);` |
| F18 | C | APP | 6(c) | Renderer.cpp:1823 `clip->playheadPosition = player->getPlayheadPosition();` | LayerStrip.cpp:763 `const auto tv = transportViewOf(layer_, transportBounds_);` |
| F19 | D | APP | 6(c),9(c) | Layer.cpp:23 `return scalarLive[static_cast<size_t>(s)].effective(manualRef(const_ca` | ManualWrite.cpp:187 `*r.manual = r.toModel ? r.toModel(valueNorm) : valueNorm;` |
| F20 | B | APP | 6(c) | Layer.h:250 `triggerClipImmediate(column);` | CompositorEngine.cpp:1072 `const Clip* clip = layer.getActiveClip();` |
| F21 | A | APP | 8(b) | VideoPlayer.cpp:307 `std::cerr << "[VideoPlayer] Opened: " << path` | VideoPlayer.cpp:316 `<< (intraOnly_ ? ", intra-only" : "") << ", firstPts=" << firstPts_ <<` |
| F22 | B | APP | 9(c) | Renderer.cpp:1787 `if (clip->playing && !player->isPlaying())` | TriggerCommands.h:68 `void execute() override { apply(after_, targetPlayingAfter_); }` |
| F23 | B | APP | 9(c) | Layer.h:250 `triggerClipImmediate(column);` | CompositorEngine.cpp:991 `if (crossfadeStart_[clipKey].observe(layer.previousClipColumn, layer.a` |
| F24 | B | APP | 9(c) | Layer.h:250 `triggerClipImmediate(column);` | CompositorEngine.cpp:992 `layer.crossfadeProgress))` |
| F25 | C | APP | 9(c) | Renderer.cpp:1823 `clip->playheadPosition = player->getPlayheadPosition();` | LayerStrip.cpp:763 `const auto tv = transportViewOf(layer_, transportBounds_);` |

(F-numbers are this analyzer's order, not tsan-red-main-655d232.txt's; the 25 keys are the same set.)
GREEN for the lane: the H8 smoke below (one lane-d launch). The app sweep's full RED / GREEN arms are Harmony's G3.
Syntax: `bash -n .harmony/probe-tsan.sh` ok; py_compile of both .py ok (main .venv python).

### Docs (plan (6) + amendment 14 + H4)
- docs/claude/architecture.md: Clip table -- `playheadPosition` = `mutable RelaxedDouble`, new row `playing` = `mutable
  RelaxedBool`; Lock-Free Communication Chain gains "Model fields shared across threads" (Relaxed<T> single-writer
  scalars; the tuple = one 16-byte CAS word written by both threads; one load per layer per frame + one-CAS fade tick,
  render triggers <= 16; render write-back = CAS on the value read; structure behind withDeckDetached, the GL derives
  the deck index from the deck pointer; worker threads log with logLine); Threading Deep-Dives points at Pitfall 63.
- docs/claude/pitfalls.md: NEW Pitfall 63 appended after 62 (Pitfall 61 untouched, H5) -- the five families, rules
  (1)-(10) incl. amendment 14's (probe-tsan-unit.sh REQUIRED, first execute() a no-op, activation tails before the CAS,
  activeDeckIndex Relaxed, a live Layer copy is per-field atomic), the m5 wording (I2, not I3), the R5 / R7 scope note,
  the 16-byte portability note, the guards.
- docs/claude/performance-controls.md: Beat Snap paragraph (a queued trigger lives in the tuple word; cancel vs fire
  are both CASes, a cancel is never outrun by a beat; GL fire <= 16 attempts) and Key-up routing (a momentary release =
  releaseMomentary, one CAS; a release before its beat cancels its own queued trigger -- amendment 10 / H2).
- docs/claude/rendering.md :67 (Crossfades): observe() gets one consistent tuple per layer per frame (+ the adopt case);
  R11 (per-layer consistency only, the torn column frame kept) and R-A9 (hasActiveLayers' own load) beside it.
- docs/claude/integration.md: a bullet after the `/api/state` outputs bullet: render_pending_fired /
  render_autopilot_advances / render_tuple_adopts (meaning, monotonic since launch, adopts INFO only, scenario d's bar).
- CLAUDE.md: Sacred Rule 2 + " (model fields: `Relaxed<T>`, Pitfall 63)"; pitfall index "63. The Layer trigger tuple is
  one CAS word; shared model fields are `Relaxed<T>` -- before touching Layer runtime fields or a render write-back.";
  paid for by shrinking the Deck tab row paragraph to a pointer (its full text is performance-controls.md:51).
  `wc -c CLAUDE.md`: 23,996 B on the base -> 24,006 B (bar <= 24,999 B, H4).
- Code comments (plan (6)): TriggerCommands.h / DeckCommands.h (B1), CrossfadeHistory.h / LayerStrip.cpp /
  ConnectionEngine.h (B2), Renderer.cpp:449-451 (T6, this stage). .harmony/APP-INVENTORY.md counts: Harmony's (not
  touched).
- For Harmony (not a repo doc): plan (8)'s Boris live check gains "a momentary pad released before its beat does not
  latch on" (amendment 14 / H2) -- the plan file is Harmony's.

### Lane final evidence (B3)
(1) Full normal ctest, serial, under the H12 mutex (`ctest --test-dir <wt>/build-lane -j1`; build 09:57:45-09:57:50
rc=0, ctest 09:57:50-09:58:57), verbatim:
```
100% tests passed, 0 tests failed out of 1085
Total Test time (real) =  66.82 sec
```
G1 per-target case counts (each binary's `--list-tests`): test_layer_runtime_race 3, test_manual_scalar_race 1,
test_layer_runtime 12, test_clip_transport_sync 3, test_relaxed 4, test_log_line_lint 1, test_render_thread_lint 2,
test_shared_field_types 1 (= 27) + test_undo_commands' D1 / D1b / D1c 3 (target now 80 cases) = 30 new cases;
1055 + 30 = 1085. B3 added no case (T6's pin is a static_assert in the existing case; T7 turned 1062 D6 GREEN).

(2) `.harmony/probe-tsan-unit.sh <wt>/build-tsan` (no TSAN_OPTIONS in the shell; re-run after the full TSan build,
10:51), verbatim:
```
probe rc=0
probe-tsan-unit: build test_layer_runtime_race test_manual_scalar_race 2026-10-02 10:51:15
probe-tsan-unit: ctest -L tsan 2026-10-02 10:51:16
1/4 Test #1058: R1 message-thread triggers vs render clock / autopilot on one deck  ...    Passed    0.29 sec
2/4 Test #1059: R2 clip runtime fields: trigger writes vs render transport write-back  ...    Passed    0.26 sec
3/4 Test #1060: R4 tuple consistency and no lost fade under a paced trigger storm  ...    Passed    0.22 sec
4/4 Test #1061: R3 manual scalar writes vs eff() reads  ...    Passed    0.26 sec
100% tests passed, 0 tests failed out of 4
probe-tsan-unit: ctest rc=0 2026-10-02 10:51:17
WARNING: ThreadSanitizer count: 0
```
(the first run at 09:59:13-09:59:21, before the full TSan build, gave the same 4/4 Passed, rc=0, 0 warnings, as #110-#113.)
probe-tsan-unit.sh's TARGETS list is unchanged: B3 added no tsan-labelled target.

(3) INFO: the full TSan ctest (`env -u TSAN_OPTIONS ctest --test-dir <wt>/build-tsan -j1 --timeout 600`, under the H12
mutex; full TSan build 10:00-10:32:26 rc=0; ctest 10:32:58-10:50:46), verbatim:
```
100% tests passed, 0 tests failed out of 1085
Label Time Summary:
tsan    =   1.04 sec*proc (4 tests)
Total Test time (real) = 1068.11 sec
```
No failing case, so there is nothing to compare at the base. (The non-tsan targets run with TSan's default
exitcode 66, so any race in them would have failed its case.)

(4) H8 lane smoke under the live lock (LANE=tsan-B3; lock 09:59:49-10:00:35): ONE lane-d launch of
<wt>/build-tsan/AudioDNA_artefacts/RelWithDebInfo/Audio-DNA.app (built 09:56, after T7; links
libclang_rt.tsan_osx_dynamic.dylib) through the in-repo tooling: `TSAN_APP_lane=<that app> TSAN_MEDIA=<scratch>/media
bash .harmony/probe-tsan.sh <scratch>/smoke "1:d@lane"` (history_size=4). ps top before: a clang at 100 % (00:02 old;
another lane's build, not ours), mds_stores, mediaanalysisd. Verbatim:
```
UNC windows (OptionAll) before: 0
--- tsan-1 scenario d arm lane  09:59:51  app .../build-tsan/AudioDNA_artefacts/RelWithDebInfo/Audio-DNA.app
health after 3 s gone=0
7070 listener: Audio-DNA; pids: 57832 ; Output-named / Audio-DNA windows: 0 2
scenario rc=0
alive at end: yes; Output-named / Audio-DNA windows: 0 2
graceful=yes
Output-named / Audio-DNA windows after quit: 0 0
VALIDITY tsan-1 d@lane: yes  warnings=0 render_pending_fired=1 render_autopilot_advances=26 render_tuple_adopts=0
last quit 10:00:18
UNC windows (OptionAll) >= 15 s after the last quit: 0
new Audio-DNA .ips: none
=== probe-tsan batch 20261002-095950 done 2026-10-02 10:00:34  batch-valid=yes
```
launches.tsv row: `tsan 1 d lane healthS=3 gone=0 alive_end=yes graceful=yes sanfiles=0 warnings=0 output_named=0
valid=yes pending_fired=1 autopilot_advances=26 tuple_adopts=0`. Audio pre-check: `Default Input Device: Yes;
Transport: Built-in; Default Output Device: Yes; Default System Output Device: Yes; Transport: Built-in`.
Final /api/state (state.json): render_pending_fired 1, render_autopilot_advances 26, render_tuple_adopts 0 (INFO),
fps 118.6, active_deck 0. G3.4's lane-d bar (pending >= 1 AND advances >= 1) holds: the render writers ran (Manual BPM
via set_bpm 240 gave the beats). scen.log: 126 REST steps at 0.25 s spacing, every answer 200, worst lateness 0.093 s.
The app's stderr shows whole logLine lines (9 `[VideoPlayer] Opened: ...` from concurrent MediaOpen threads, each one
line) and 0 "ThreadSanitizer" lines; no tsan.* report file (0 warnings). Analyzer over the smoke dir: `0 launches
with reports, 0 warnings, 0 unique` (`tsan-1 d@lane warnings 0 {} {} valid=yes`).
Caveat (named): 0 warnings in ONE launch is evidence, not proof (R10); the positive control for the same tooling is the
archive keying above, and the full RED / GREEN arms are Harmony's G3.
After the batch: lock released, no Audio-DNA running, `audio-dna windows 0, Output-named 0`. The .venv symlink was
created for the run and removed at 10:00 (before any commit).

H8 part 2 (the analyzer over the fresh-RED archive keying all 25 uniques into A-E): see T8 above (table pasted there).

T8 follow-up (in this commit): probe-tsan-analyze.py lists only the arms its launches name (the smoke printed an empty
default "main" arm); the archive output is byte-identical before / after (diff empty).

### Non-mechanical test migrations (B3)
none. B3 edited one existing test file: tests/test_shared_field_types.cpp (+1 static_assert, the T6 pin; amendment 11
names this extension). No assertion, expected value or call order of any other test changed; every activeDeckIndex
reference in tests (92) compiled unchanged through Relaxed<int>'s conversions.

### Rig notes (B3)
- No app other than the one smoke launch; no Output window by any path; no synthetic input; no screen capture; no
  lldb / sample / dtrace. No TCC prompt and no unexpected dialog (UNC 0 before and after).
- The main checkout and its build/ were only read (FETCHCONTENT sources, the .venv via a symlink, git objects for
  655d232 / 2d38b39 through the worktree's git).
- A post-commit "[graphify hook] launching background rebuild" printed after each commit (user-level hook); the
  worktree stayed clean.

### Notebook notes (for Harmony to append)
- 2026-10-02 -- a std::cerr chain converts to logLine mechanically | Files: src/core/LogLine.h, tests/test_log_line_lint.cpp
  | `std::cerr << a << b << std::endl;` -> `logLine(a, b);` (split on top-level `<<` only); a chain ending in a
  `"...\n"` literal drops the `\n` (logLine appends it). Verify by re-parsing both operand lists (scratch
  cerr_verify.py in s-rta-1002 B3), not by eye. | discovered: T7 (75 statements, 14 files).
- 2026-10-02 -- TSan prints "Atomic read of size N" with a CAPITAL A | Files: .harmony/probe-tsan-analyze.py | an
  access-header regex that only accepts "Previous atomic ..." drops one side of every mixed atomic / plain race (the
  LayerStrip atomic_ref read vs the plain playheadPosition write) | discovered: T8 first archive run.
- 2026-10-02 -- Relaxed<T> has no compound operators by design | Files: src/model/Relaxed.h | `--field` / `++field` /
  `field += x` do not compile: spell a single-writer decrement as `f = f - 1` (DeckCommands.h:930) or use fetchAdd for a
  shared counter | discovered: T6.

### next_stage_notes (lane end -> Harmony)
- Lane heads: T0 2c04a03, T1 7422e10, T2 3fec609, T3 9d9444e, T4 3400c08, T5 2e72850, T6 bdb9824, T7 ea00c93, T8 0cdda8e,
  docs 0314fbf, then this report commit. Branch lane/tsan; NOT merged, NOT pushed.
- Build dirs: build-lane (normal Release, current, ctest 1085/1085) and build-tsan (TSan RelWithDebInfo, ALL targets
  built at the lane head minus the docs-only commits, ctest 1085/1085, app at AudioDNA_artefacts/RelWithDebInfo).
- Harmony's gates: G1 (count 1085 = 1055 + 30, per-target counts above), G2 (RED arm = a worktree at the T0 sha
  2c04a03e59304efca134df617f7e4ff05675bb8f), G3 with .harmony/probe-tsan.sh (main arm = a TSan build of the actual
  pre-merge commit; `--src main=git:<sha>` for the analyzer), G4 probes, G5 review (test_render_thread_lint case 2),
  G6 perf.
- Merge surface vs bt2: MainComponent.cpp hunks of this stage at 4607 / 4826 / 5572 / 6343 / 6368 / 6402 (one-token
  `.load()` edits); ApiServer.cpp: the LogLine include at line 2, :94 / :97 / :124 (cerr), :360 / :384 / :1547 (.load());
  pitfalls.md: Pitfall 63 appended after 62; CLAUDE.md: Sacred Rule 2, the index line 63, the Deck tab row pointer.
- Filed (H10): tsan-r5 (R5 config scalars + perf/play preamble writes, R7 unfenced httplib reader) from scenario e.

### Final per-target case counts (G1; normal build, lane head)
| target | cases |
|---|---|
| test_layer_runtime_race | 3 |
| test_manual_scalar_race | 1 |
| test_layer_runtime | 12 |
| test_clip_transport_sync | 3 |
| test_relaxed | 4 |
| test_log_line_lint | 1 |
| test_render_thread_lint | 2 |
| test_shared_field_types | 1 |
| test_undo_commands (+ D1 / D1b / D1c) | +3 (80 total) |
| new cases | 30 -> ctest 1055 + 30 = 1085 |

T0 sha: 2c04a03e59304efca134df617f7e4ff05675bb8f

### Amendment-16 mutant evidence (B1-B3) -- ONE table
Each mutant: a normal cmake build of a modified tree in build-lane, run once (m4 30x), restored byte-identically (sha256
checked; B2's runner sleeps 1.2 s + touches the file, the mtime trap); `git diff` empty after each. B3's items (T6, T7,
T8) carry no amendment-16 mutant; their RED-first teeth are the T6 type pin (compile RED on the pre-T6 src) and D6 (RED
on every tree before T7).

| # | mutant (amendment 16) | stage | failed | verbatim evidence (excerpt; full lines in B1 / B2 above) |
|---|---|---|---|---|
| m1 | casRuntime as a plain store | B1 (T2) | test_layer_runtime 2 of 10 cases; R4 (normal) 3/3 | `test_layer_runtime.cpp:145: FAILED: CHECK_FALSE( L.casRuntime(stale, ticked) )`; `:203: FAILED: CHECK( calls == 16 ) with expansion: 1 == 16`; R4 `I1 0 I2 4540 I3 0 I4 0` / `I2 2790` / `I2 3204` |
| m2 | TriggerClipCmd::execute() without the first-call skip | B1 (T2) | test_undo_commands D1, D1b, D1c (77 of 80 pass) | `test_undo_commands.cpp:3108: FAILED: CHECK( captureLayerRuntime(L).crossfadeProgress == Catch::Approx(0.2f) ) with expansion: 0.0f == Approx( 0.20000000298023224 )`; `:3136: FAILED: CHECK( captureLayerRuntime(L) == ended )`; `:3158: ... 0 == 1` |
| m3 | releaseMomentary without its pending branch | B1 (T2) | test_layer_runtime "a release before the beat cancels ..." | `test_layer_runtime.cpp:275: FAILED: CHECK( L.runtime().activeClipColumn == 0 ) with expansion: 1 == 0`; `:276: FAILED: CHECK( L.clips[1]->playing == false )` |
| m4 | activation tail moved after the CAS | B1 (T2) | contract test, rarely: 3 of 30 runs (4 of 20 in a first batch); lane 30/30 pass | `activations seen 20000, stale beatsPlayed seen 1 -- test cases: 1 / 1 failed` (fired, as the ruling predicted: rare, so the order is also a review item) |
| m5 | LayerClock::tick as a plain store | B2 (T3) | stale-tick test + contention test; R4 (normal) 3/3 | `test_layer_runtime.cpp:325: FAILED: CHECK_FALSE( LayerClock::tick(L, rt, 0.1f) )`; `:402: FAILED: REQUIRE_FALSE( stalled ) ... adopts 0`; R4 `I1 0 I2 11725 I3 0 I4 0` / `I2 12053` / `I2 12112` -- through I2 (a LOST activation), not I3 as the ruling worded it |
| m6 | syncMedia write-back as a plain store | B2 (T4) | test_clip_transport_sync 2 of 3 cases (OneShot passes, no concurrent intent) | `test_clip_transport_sync.cpp:48: FAILED: CHECK( clip.playing.load() == true ) with expansion: false == true`; `:99: FAILED: CHECK( clip.playing.load() == false ) with expansion: true == false` |

## Fix round (Harmony rulings) -- Builder tsan-fix, started 2026-10-02 11:08 EDT
STATUS: DONE -- F1-F8 done as ruled (each finding re-verified against the code first; none was wrong), one-line NITs
fixed, the rest listed with reasons. Commits on lane/tsan after bf5c116: eef55fe F1, d13f66a F2, 1e7abbc F3, 6de81ba F4,
ef686f5 F5, 0562907 F6, 525bdbe F7, d59a903 F8, fb2916b NITs, then this report commit. Full normal ctest 1092/1092
(serial, mutex); probe-tsan-unit 4/4 (finds 4, 0 TSan warnings). base_commit 2d38b39. NOT merged, NOT pushed.
Inputs read in full: plan-tsan.md (incl. HARMONY ADOPTION H1-H13), ruling-tsan.md "ARCHITECT RULING" (amendments 1-18 +
FINAL GATES), tsan-fix-rulings.txt (F1-F8), review-tsan-{memmodel,tests,render}-r1.md. Continue from bf5c116 (no reset).

### F1 -- R1 / R2 / R3 start handshake (commit eef55fe)
Finding VERIFIED before fixing (tests M1): at bf5c116 the render thread is spawned right before main's ~10 ms loop with
no handshake. Fix: `waitForFrames(renderFrames, atLeast, 10 s)` (tests/test_layer_runtime_race.cpp and
tests/test_manual_scalar_race.cpp); after `go`, main waits for renderFrames >= 1 (else stop + join + `FAIL("render thread
never started")`), and after its loop waits (bounded 10 s) for renderFrames >= 100 before `stop`; `CHECK(renderFrames > 0)`
stays as the vacuity guard. The render thread "publishes running" by counting its first frame.
Evidence: scratch script tsan-fix/load-runs.sh -- starts 10 busy-loop burners `( while :; do :; done ) &`, records their
pids, runs each case 30x serially (normal build, build-lane), KILLS the burners on EXIT and prints the survivors + top 5 CPU.
RED (bf5c116 binaries, 11:12:53-11:12:56), verbatim:
```
burner pids: 40331 40332 40333 40334 40335 40336 40337 40338 40339 40340
R1: 14 / 30 failed
R2: 14 / 30 failed
R3: 1 / 30 failed
burners left alive: 0
```
every failure: `test_layer_runtime_race.cpp:168: FAILED: CHECK( renderFrames.load() > 0 ) with expansion: render frames 0, ...`
(R1), `:241` (R2), the R3 analogue in test_manual_scalar_race.cpp:90.
GREEN (eef55fe, 11:13:34-11:13:38), verbatim:
```
burner pids: 41229 41230 41231 41232 41233 41234 41235 41236 41237 41238
R1: 0 / 30 failed
R2: 0 / 30 failed
R3: 0 / 30 failed
burners left alive: 0
 76.6 .../Metadata.framework/Versions/A/Support/mds_stores
 70.9 .../Contacts.framework/Support/contactsd
 69.2 .../CoreSuggestions.framework/Versions/A/Support/suggestd
 62.5 .../mediaanalysisd-access.xpc/Contents/MacOS/mediaanalysisd-access
 49.8 .../MediaAnalysis.framework/Versions/A/mediaanalysisd
```
(`ps -Ao pcpu=,comm= | sort -rn | head -5` after the kill: no burner left; the top entries are macOS indexers.) A passing
run now overlaps for real (`-s` output): R1 `render frames 16447`, R2 `render frames 47625`, R3 `render frames 216196`.
(First RED attempt at 11:12:36 had a script bug -- R2's case name holds a ':' my splitter cut on, so R2 "failed" 30/30 as
"no test matched"; fixed the separator to '|' and re-ran; the numbers above are the re-run.)
### F2 -- a render-side trigger decided from a snapshot is conditioned on its column (commit d13f66a)
Finding VERIFIED before fixing (memmodel S1): Autopilot::processFrame loads `activeCol` once (Autopilot.cpp:27 / :83 at
bf5c116), decides, and calls advanceClip / smartAdvanceClip -> `triggerClip(next, Off, 16)` whose pure function
(Layer.h:403-411 / immediateNext) never looked at the column it decided from. Reproduced below by the call-site mutant
(= bf5c116's behaviour on that path): 4895 of 5000 clears re-activated 300 yields later. The only render-side trigger
callers in src are those three Autopilot sites (grep `triggerClip(Immediate)?\(|triggerColumn\(` outside the
message-thread dirs); processPendingTrigger already fires from the tuple it CASes (its pure fn reads the pending column).
Fix (src/model/Layer.h, src/model/Autopilot.{h,cpp}):
- `Layer::triggerClip(column, forcedSnap, maxAttempts, std::optional<int> onlyIfActive = nullopt)`; `activate()` wraps
  the pure fn: when `onlyIfActive` is set and `r.activeClipColumn != *onlyIfActive` it returns `r` unchanged -- inside
  every CAS attempt, so it holds against a clear / trigger landing at any point. The post-CAS retrigger tail is skipped
  when the guard rejected (`*onlyIfActive == column` required), and an advance onto an EMPTY cell passes the guard on to
  `clearActiveClip(onlyIfActive, ...)`. So a rejected advance changes no tuple field AND no clip field.
- Autopilot's 3 sites pass `currentCol` (advanceClip :304, smartAdvanceClip :326 / :387). This also stops an autopilot
  advance overriding a user trigger that landed first. A rejected advance returns changed() == false, so it is not
  counted in render_autopilot_advances.
- Docs: the Layer.h clear-tail comment now states WHY no guard is needed (the advance is conditioned); Pitfall 63 item
  4 and architecture.md ("Model fields shared across threads") state the guarantee. CLAUDE.md untouched.
- tests/CMakeLists.txt: test_layer_runtime gains `${SRC_DIR}/model/Autopilot.cpp` (its own lane block; reconfigured).
Tests (tests/test_layer_runtime.cpp, GREEN-only; the teeth are the mutants), 3 new cases (target 12 -> 15):
- "a render-side advance decided before a clear leaves the layer cleared": snapshot -> clearActiveClip -> the stale
  advance (exactly advanceClip's call) -> tuple == the clear's `after`, the would-be clip's playing / beatsPlayed /
  playhead untouched; plus a positive control (an advance decided from the CURRENT tuple applies).
- "a render-side advance decided before a user trigger leaves the user's clip active": sections k = 2 (another
  column), k = 1 (the very column the advance wanted: no second activation tail), and the empty-cell advance.
- "stress: a real autopilot advancing every frame never re-activates a layer the message thread cleared": the
  reviewer's repro, bounded (5000 clears or 3 s; ~0.25 s): GL thread = real Autopilot::processFrame with a beat crossing
  every frame (PlayNext / Beat1); main = trigger, yield, clear, check right after and 300 yields later; start
  handshake; vacuity guards `clears > 0` and `applied advances > 0`. GREEN run: `clears 5000, GL frames 26514432,
  applied advances 11709` and 0 / 0 re-activations. (0 by construction with the guard, so it cannot false-fail.)
GREEN: `test_layer_runtime: All tests passed (1107 assertions in 15 test cases)`; test_autopilot `All tests passed (49
assertions in 11 test cases)`; test_render_thread_lint `All tests passed (15 assertions in 2 test cases)` (case-2 load
counts unchanged: no runtime() / getActiveClip( added).
Mutants (scratch tsan-fix/mutant.sh: backup, one textual replacement, build test_layer_runtime in build-lane, run
`[autopilot]`, restore, sha256 equal, rebuild, `git diff` empty -- all four printed `restore sha256 equal: yes` and
`git diff after restore: []`):
| mutant | result (verbatim excerpt) |
|---|---|
| guard removed (the 2-line `onlyIfActive` check in activate's wrapper) -- the ruled mutant | `test cases: 3 \| 0 passed \| 3 failed`; `:433 FAILED: CHECK_FALSE( t.changed() )`, `:434 CHECK( L.runtime() == cleared.after )`, `:435 CHECK( L.clips[1]->playing == false )`, `:436 beatsPlayed == 9`, `:437 playheadPosition == 0.8`, `:456 / :457 / :458` (k = 2), `:559 FAILED: CHECK( activeLater == 0 ) ... clears 5000, GL frames 11736068, applied advances 11519845; active right after the clear 0, 300 yields later 4961` |
| Autopilot advanceClip not passing currentCol (= bf5c116's path) | `:559 FAILED: CHECK( activeLater == 0 ) ... clears 5000, GL frames 11959991, applied advances 11396792; active right after the clear 0, 300 yields later 4895`; `test cases: 3 \| 2 passed \| 1 failed` |
| post-CAS retrigger tail without the guard condition | `the user triggered the very column the advance wanted (k = 1) ... :471 FAILED: CHECK( L.clips[1]->beatsPlayed == 3 )`, `:472 playheadPosition == 0.6` |
| empty-cell advance clears unguarded (`clearActiveClip(std::nullopt, ...)`) | `a stale advance onto an EMPTY cell ... :481 FAILED: CHECK_FALSE( t.changed() )`, `:482 CHECK( L.runtime() == user.after )` |
Residual (named, not in the ruling): the guard compares the ACTIVE column only, as ruled; a user trigger that was
QUEUED (beat snap) after the autopilot's load leaves the active column unchanged, so the advance (an immediate trigger)
still clears that queued trigger -- identical to bf5c116 and the base (an immediate trigger always cancels the queue).
### F3 -- zero-heap logLinef on the analysis thread (+ the GL periodic lines) (commit 1e7abbc)
Finding VERIFIED before fixing (render SHOULD-1): at bf5c116 AnalysisThread.cpp:83 / :367 / :372 / :374 call logLine,
which builds a std::ostringstream + std::string. Measured (scratch tsan-fix/heap: a DYLD_INSERT_LIBRARIES interposer
counting malloc / calloc / realloc / malloc_zone_malloc / malloc_zone_calloc while armed, 2 warm-up calls then 10
counted calls each; controls first), verbatim:
```
CONTROL malloc(64)                       mallocs in 10 calls: 10
CONTROL new char[64]                     mallocs in 10 calls: 10
logLine (the old profile TOTAL line)     mallocs in 10 calls: 20
logLine (rate-change line, 90+ chars)    mallocs in 10 calls: 30
logLinef (rate-change line, 90+ chars)   mallocs in 10 calls: 0
logLinef (Render Profile, %g + %s)       mallocs in 10 calls: 0
logLine (ostringstream, ints + strings)  mallocs in 10 calls: 0
logLinef (%s %d)                         mallocs in 10 calls: 0
logLinef (%g double)                     mallocs in 10 calls: 0
logLinef (%g double 16)                  mallocs in 10 calls: 0
```
(a short logLine line fits the small-string buffer and does not allocate; the real profile / rate-change lines do: 2-3
per call -> 16+ allocations per 5.3 s profile on the analysis thread, as the reviewer said.)
Fix:
- src/core/LogLine.h: `logLinef(const char* fmt, ...)` -- vsnprintf into a STACK `char[kLogLineMax = 512]`, ONE
  std::fwrite of the line + '\n' to stderr; a cut line ends with `kLogLineTruncated` = "...[truncated]"; a bad format
  writes `kLogLineBadFormat`. Plus `vformatLogLinef` / `formatLogLinef(char* out, size_t cap, fmt, ...)` (caller
  buffer; returns the length incl. '\n') and `logLinefTo(FILE*, fmt, ...)`. printf-format attributes (clang / gcc)
  check every call's arguments; `format(printf, 3, 0)` on the va_list one keeps -Wformat-nonliteral quiet (the first
  build printed that warning 19x; fixed, the final build has 0 LogLine.h warnings).
- src/analysis/AnalysisThread.cpp: all four statements (rate change, profile header, per-stage, TOTAL) -> logLinef.
- src/render/Renderer.cpp: the two GL lines the ruling named -- Render Profile (~1027, every 300 frames) and Adaptive
  quality (~1002) -> logLinef (trivial: %g / %d / %s; `fx->getName().toRawUTF8()` is exactly what JUCE's
  `operator<<(std::ostream&, const String&)` streams, juce_String.h:1511-1513). So Pitfall 63 needs no "allocates once
  per ~5 s" note; it now names logLinef for threads that must not allocate.
- Text identity: every line is byte-identical to the old std::cerr text (no difference to list). Checked: (a) the unit
  test compares logLinef's output with an ostringstream of the old operands; (b) scratch tsan-fix/heap/g.cpp compared
  `os << v` with `%g` for 4,000,002 doubles of the two shapes (k / 100.0 and k / 10.0, k = 0..2,000,000):
  `compared 4000002 values, 0 differ`.
Tests (tests/test_log_line_lint.cpp; its CMake block gains `target_include_directories(... ${SRC_DIR})`; 1 -> 3 cases):
- "threads that must not allocate log with the zero-heap logLinef" [tsan_lint][lint]: AnalysisThread.cpp has 0
  `logLine(` and exactly 4 `logLinef(`; the Renderer.cpp code line holding "[Render Profile] Avg frame" and the one
  holding "Adaptive quality: disabled" each hold `logLinef(`.
  RED on bf5c116's two source files (scratch red-f3.sh: copy bf5c116's AnalysisThread.cpp + Renderer.cpp over the tree,
  run the lint binary -- it reads the source at run time -- restore, `shasum -c` OK on both), verbatim excerpt:
  ```
  test_log_line_lint.cpp:114: FAILED:  CHECK( ostreamLines.empty() )
    analysis/AnalysisThread.cpp logLine( at: analysis/AnalysisThread.cpp:83, analysis/AnalysisThread.cpp:367,
    analysis/AnalysisThread.cpp:372, analysis/AnalysisThread.cpp:374
  test_log_line_lint.cpp:115: FAILED:  CHECK( hits("analysis/AnalysisThread.cpp", "logLinef(").size() == 4 )
  test_log_line_lint.cpp:122: FAILED:  CHECK( lines[0].find("logLinef(") != std::string::npos )  ("[Render Profile] Avg frame")
  test_log_line_lint.cpp:122: FAILED:  ... ("Adaptive quality: disabled")
  test cases:  1 | 1 failed
  assertions: 10 | 6 passed | 4 failed
  ```
- "logLinef: one whole line in a stack buffer, the old std::cerr text, truncation marked" [log_line]: the four analysis
  lines and the Render Profile line (8.33 / 16 / 0.5 / 123.45) equal the streamed text; a 24-byte buffer keeps 22
  characters ending "...[truncated]" + '\n' + NUL; an exactly-fitting line is unmarked; a too-small buffer writes 0;
  logLinefTo(tmpfile) writes exactly the line. Mutant "no truncation marker" (the memcpy removed; mutant.sh, restore
  sha256 equal: yes), verbatim: `test_log_line_lint.cpp:151: FAILED: CHECK( std::string(small, n) == std::string("abcdefgh")
  + kLogLineTruncated + "\n" )` / `test cases: 1 | 0 passed | 1 failed`.
GREEN: `test_log_line_lint: All tests passed (54 assertions in 3 test cases)`; D6 (the std::cerr lint) still green.
Docs: Pitfall 63 (9) and architecture.md name logLinef for threads that must not allocate. CLAUDE.md untouched.
### F4 -- D1d: ClearActiveClipCmd's first execute() is a no-op (commit 6de81ba)
Finding VERIFIED before fixing (tests S1): at bf5c116 DeckCommands.h:388-396 ClearActiveClipCmd has the first-call skip
but no test pins it (D1 / D1b / D1c and mutant m2 are TriggerClipCmd only; the existing clear tests are idempotent with
the live clear). New case in tests/test_undo_commands.cpp (target 80 -> 81 cases): "D1d a clear's first perform keeps a
transition that landed after the live clear" -- live `clearActiveClip()` (before / after captured), then
`triggerClipImmediate(1)` lands (the ruling's example), then `UndoManager::perform(ClearActiveClipCmd(before, after))`:
REQUIRE the tuple is still the landed one (active 1); undo -> before; redo -> after.
GREEN-only on bf5c116 too (the skip exists there); the teeth are the ruled mutant "remove its first-call skip"
(mutant.sh on src/core/DeckCommands.h: the 5-line `if (firstExecute_) {...}` removed, test_undo_commands rebuilt; restore
sha256 equal: yes; git diff after restore: []), verbatim:
```
test_undo_commands.cpp:3182: FAILED:
  CHECK( captureLayerRuntime(L) == landed )
test cases:  81 |  80 passed | 1 failed
assertions: 562 | 561 passed | 1 failed
```
GREEN: `test_undo_commands: All tests passed (562 assertions in 81 test cases)`.
### F5 -- R2 drives the real ClipTransportSync (commit ef686f5)
Finding VERIFIED before fixing (tests S2 = memmodel S3): at bf5c116 R2's render side (test_layer_runtime_race.cpp:188-220)
is a plain-syntax mirror of the PRE-lane syncMedia (`c->playing = playerPlaying` etc.), so since T4 the [tsan] case
did not exercise ClipTransportSync's CAS write-back. Fix: R2's render thread now does, per frame, ONE
`captureLayerRuntime`, picks the active column's FakePlayer (one per column, GL-owned like Renderer's players),
`ClipTransportSync::pushIntent(*c, player)` -> `player.advance(0.01)` -> `ClipTransportSync::writeBack(*c, player,
wanted)` (the playhead store, the CAS write-back, the out-point on the local playhead: both clips get outPoint 0.9 so
the branch runs; clip 0 loops back, clip 1 is a OneShot) and `c->beatsPlayed.fetchAdd(1)` (Autopilot's real shape).
The main thread (triggerClipImmediate / clearActiveClip / hasBeenTriggered) and the third reader are unchanged. The
file header now says R2 drives the real entry points and that the file no longer compiles against the pre-lane tree.
As ruled, R2's T0 RED stays as recorded (B1 section). GREEN under TSan on the fix head (build-tsan, target rebuilt
11:26:42; the ruling's TSAN_OPTIONS), 5 runs, verbatim:
```
R2 tsan run 1 rc=0 warnings=0 All tests passed (5 assertions in 1 test case)
R2 tsan run 2 rc=0 warnings=0 All tests passed (5 assertions in 1 test case)
R2 tsan run 3 rc=0 warnings=0 All tests passed (5 assertions in 1 test case)
R2 tsan run 4 rc=0 warnings=0 All tests passed (5 assertions in 1 test case)
R2 tsan run 5 rc=0 warnings=0 All tests passed (5 assertions in 1 test case)
```
Normal build: `test_layer_runtime_race` rc=0 `All tests passed (1189 assertions in 3 test cases)` x3; a passing R2 run
overlaps for real (`render frames 48881`). probe-tsan-unit.sh (final evidence) runs it again.
### F6 -- the newer intent survives an out-point crossing (commit 0562907)
Finding VERIFIED before fixing (tests S3): the 3 bf5c116 cases never reach the OneShot branch with `wrote == false`
(B2's m6 note: "the OneShot case passes under the mutant"). New case in tests/test_clip_transport_sync.cpp (3 -> 4
cases): "an intent that changed inside the sync window survives an out-point crossing (OneShot)" -- a OneShot clip
(in 0.1, out 0.5) sits stopped past its out-point (player head 0.75); pushIntent reads "stopped"; inside the window the
message thread sets playing = true; the player (stopped) does not move; writeBack: the CAS fails (intent changed), the
out-point branch runs; CHECK playing == true (the newer intent), playhead 0.75, the player held stopped this frame.
GREEN-only on bf5c116 (its writeBack has the `if (wrote)` guard); the teeth are the ruled mutant "OneShot stop as an
unconditional playing.store(false)" (mutant.sh: the 5-line `if (wrote) {...}` in ClipTransportSync.h -> 
`clip.playing.store(false);`; restore sha256 equal: yes; git diff after restore: []), verbatim:
```
test_clip_transport_sync.cpp:124: FAILED:
  CHECK( clip.playing.load() == true )
with expansion:
  false == true
test cases:  4 |  3 passed | 1 failed
assertions: 17 | 16 passed | 1 failed
```
GREEN: `test_clip_transport_sync: All tests passed (17 assertions in 4 test cases)`.
### F7 -- probe-tsan-unit.sh fails closed (commit 525bdbe)
Finding VERIFIED before fixing (memmodel S2): bf5c116's script ends `ctest --test-dir "$B" -L tsan --output-on-failure`
and exits with its code; with no tsan-labelled test ctest prints "No tests were found!!!" and exits 0 (shown below).
Fix (.harmony/probe-tsan-unit.sh): `EXPECTED_TSAN_CASES=4` (R1 / R2 / R4 + R3); after the build it counts
`ctest -L tsan -N` ("Total Tests:"), prints `probe-tsan-unit: ctest -L tsan finds N [tsan] cases (expected 4)`, and
exits 3 with `FAIL -- N [tsan] cases, expected 4 (label dropped or discovery failed?)` when N < 4; the ctest run adds
`--no-tests=error`. Header documents the fail-closed rule and that a new [tsan] target goes into TARGETS AND the count
(tests N4). RED / GREEN on scratch fake build dirs (tsan-fix/f7: a 10-line CMake project with ADNA_SANITIZE=thread in
its cache, the two target names as custom targets, 0 or 3 tsan-labelled `cmake -E true` tests + one untagged), verbatim:
```
=== fake dir with 0 [tsan] cases -- bf5c116 script
probe-tsan-unit: ctest -L tsan 2026-10-02 11:28:06
No tests were found!!!
probe-tsan-unit: ctest rc=0 2026-10-02 11:28:06
rc=0
=== fake dir with 0 [tsan] cases -- fix-round script
probe-tsan-unit: ctest -L tsan finds 0 [tsan] cases (expected 4)
probe-tsan-unit: FAIL -- 0 [tsan] cases, expected 4 (label dropped or discovery failed?)
rc=3
=== fake dir with 3 [tsan] cases -- bf5c116 script
probe-tsan-unit: ctest -L tsan 2026-10-02 11:28:06
probe-tsan-unit: ctest rc=0 2026-10-02 11:28:06
rc=0
=== fake dir with 3 [tsan] cases -- fix-round script
probe-tsan-unit: ctest -L tsan finds 3 [tsan] cases (expected 4)
probe-tsan-unit: FAIL -- 3 [tsan] cases, expected 4 (label dropped or discovery failed?)
rc=3
```
GREEN on the real build-tsan: "Fix-round final evidence" below (finds 4, 4/4 passed).
### F8 -- R4's deadline per wait (commit d59a903)
Finding VERIFIED before fixing (tests N1): bf5c116 test_layer_runtime_race.cpp:296 set ONE `deadline = now + 60 s` before
the 20,000-trigger loop. Fix: the deadline is computed inside the loop, before each wait (`// 60 s PER WAIT`); the
stall path (`stalled = true` -> `REQUIRE_FALSE(stalled)`) is unchanged. No new test, so no RED arm; instead a scratch
demonstration on a modified copy (tsan-fix/f8-demo.sh: per-wait budget cut to 1 s plus an injected render pause,
build, run, restore -- `restore sha256 equal: yes`), verbatim:
```
=== slow-live 11:29:05        (1 s per wait; the render pauses 0.4 s every 4000 observations: slow but live)
rc=0
All tests passed (2 assertions in 1 test case)
real 43.59
=== one-stall 11:29:52        (1 s per wait; the render pauses 2 s once)
rc=42
test_layer_runtime_race.cpp:363: FAILED:
  REQUIRE_FALSE( stalled )
  render observations 5000; I1 0 I2 0 I3 0 I4 0
real 2.02
```
(The slow-live run took 43.6 s with a 1 s budget: a single run-wide budget of that size would have reported "stalled" --
INFERRED from the old code, the old arm was not run.) GREEN at 60 s: R4 rc=0 `All tests passed (2 assertions in 1 test
case)` x3.
### NITs (commit fb2916b, plus N4 of tests in F7's commit 525bdbe)
Fixed (one-line / safe), each re-read against the code first:
- memmodel N1 (activation tail not reverted when the CAS never lands): VERIFIED by reading Layer.h updateRuntime /
  activate (beforeCas runs per attempt, nothing undoes it on a lost / exhausted CAS). Comment on applyActivationTail
  + Pitfall 63 item 4 now say it is harmless (inactive clip; its next activation re-runs the tail).
- memmodel N2 (pushIntent comment): VERIFIED -- the code restarts a stopped player whenever `wanted` (as the base). The
  comment now says so and that writeBack's OneShot stop clears the intent in that same sync.
- memmodel N3: `static_assert(static_cast<int>(Clip::BeatSnapMode::FourBar) < 16, ...)` beside the Word size assert
  (FourBar is the last enumerator, Clip.h:131-138).
- memmodel N4: VERIFIED in scratch (tsan-fix/x86/a.cpp, `clang++ -std=c++20 -O2 -arch x86_64`, no -mcx16): the
  static_assert is_always_lock_free compiles and the load is `lock cmpxchg16b (%rdi)` (Apple clang 17.0.0). Layer.h
  comment + Pitfall 63 (2) reworded: Apple clang arm64 and x86_64 lock-free (x86_64's load writes the line); GCC x86_64
  needs -mcx16; MSVC is not lock-free.
- memmodel N5: `/build-lane/` added to .gitignore beside `/build-tsan/` (git status no longer lists it).
- tests N3 (bounded-exhaustion test would spin if the bound were ignored): fn stops changing the tuple after 1000 calls.
  Mutant "updateRuntime ignores maxAttempts" (loop condition dropped; restore sha256 equal: yes): FAILS instead of
  hanging -- `test_layer_runtime.cpp:209: FAILED: CHECK_FALSE( t.applied ) ... !true`, `:210: FAILED: CHECK( calls == 16 )
  ... 1001 (0x3e9) == 16`, `:211: FAILED: CHECK( L.runtime().activeClipColumn == 116 ) ... 1100 (0x44c) == 116`.
- tests N4 (TARGETS hardcoded): documented in the probe-tsan-unit.sh header; the F7 count check fails a missing one.
- tests N5 (lint case 2 blind to captureLayerRuntime( / LayerClock::advanceCrossfade(Layer&, float)): both spellings now
  counted per file and pinned at 0 (VERIFIED 0 today in CompositorEngine.cpp / Renderer.cpp / DeckClock.h /
  Autopilot.cpp; CompositorEngine's own member `advanceCrossfade(layer, rt, dt)` takes the loaded tuple and is not
  matched). test_render_thread_lint 15 -> 19 assertions. Mutant (DeckClock.h's `layer.runtime()` -> `captureLayerRuntime(layer)`;
  restore sha256 equal: yes): `test_render_thread_lint.cpp:114: FAILED: CHECK( otherLoads == 0 ) ... render/DeckClock.h:
  runtime() 0, getActiveClip( 0, captureLayerRuntime( / LayerClock::advanceCrossfade( 1 at lines 32`.
Extra evidence the reviews asked for (no code change):
- render "PROPOSED" mutant (a second `layer.runtime()` in compositeDeck): `test_render_thread_lint.cpp:112: FAILED:
  CHECK( runtimeLoads == pin.runtimeLoads ) ... render/CompositorEngine.cpp: runtime() 3, getActiveClip( 2, ... at lines
  1052 1078` (restore sha256 equal: yes; git diff after restore: []).
- tests N2 (R4's I3 had no killing mutant): mutant "immediateNext keeps the old progress" (Layer.h, the
  `r.crossfadeProgress = ...` line dropped; R4 normal build; restore sha256 equal: yes): `test_layer_runtime_race.cpp:363:
  FAILED: REQUIRE( v1.load() + v2.load() + v3.load() + v4.load() == 0 ) ... render observations 357062; I1 0 I2 0 I3
  19999 I4 0` -- I3 alone kills it.
Listed, not fixed (reason):
- render 3 (16-byte static_assert `__APPLE__`-only; a #warning / #error off Apple): not one-line-safe -- an #error
  breaks every non-Apple build where the word is not lock-free, a #warning fires in every TU including Layer.h (and
  fails -Werror builds); the plan names it risk R2 (no non-Apple build here). Kept as documented risk.
- render 4 (probe-tsan-analyze.py family regexes match common words): fail-closed (a misfiled report FAILS G3.3(a),
  never hides); a precise keying needs a new negative-fixture set, not a one-line change. Proposed for the tsan-r5 follow-up (not filed by me).
- render 5 (R2 / R4 T0 RED names src/ only via allocation stacks): evidence note for G2, no code; R2's render side is now
  real ClipTransportSync code (F5), which adds src/render frames to any future R2 report.
- render 2 (GL Render Profile allocation): fixed under F3 (logLinef), not listed.
### Fix-round final evidence
(1) Full normal build (build-lane, 11:34:30-11:36:25, rc=0; 0 warnings from LogLine.h; no new warning in a touched
file) and full normal ctest, serial, under the H12 cross-lane mutex (scratch tsan-fix/ctest-full.sh: `until mkdir
/tmp/audiodna-ctest.lock ...; ctest --test-dir <wt>/build-lane -j1; rm -rf /tmp/audiodna-ctest.lock`; mutex acquired
11:36:31, ctest rc=0, released 11:37:38), verbatim:
```
100% tests passed, 0 tests failed out of 1092
Label Time Summary:
tsan    =   0.06 sec*proc (4 tests)
Total Test time (real) =  66.82 sec
```
New total 1092 = 1085 (bf5c116) + 7 new cases: test_layer_runtime +3 (F2: 12 -> 15), test_clip_transport_sync +1 (F6:
3 -> 4), test_log_line_lint +2 (F3: 1 -> 3), test_undo_commands +1 (F4 D1d: 80 -> 81). Per-target counts
(`--list-tests --verbosity quiet`): test_layer_runtime_race 3, test_manual_scalar_race 1, test_layer_runtime 15,
test_clip_transport_sync 4, test_relaxed 4, test_log_line_lint 3, test_render_thread_lint 2, test_shared_field_types 1,
test_undo_commands 81. Lane new cases vs base 2d38b39: 30 + 7 = 37 -> G1 total 1055 + 37 = 1092.
(2) `.harmony/probe-tsan-unit.sh <wt>/build-tsan` (no TSAN_OPTIONS in the shell, `env -u TSAN_OPTIONS`; 11:37:52-11:38:02,
probe rc=0; the two targets rebuilt: 9 "Building CXX" lines), verbatim:
```
probe-tsan-unit: build test_layer_runtime_race test_manual_scalar_race 2026-10-02 11:37:52
probe-tsan-unit: ctest -L tsan finds 4 [tsan] cases (expected 4)
probe-tsan-unit: ctest -L tsan 2026-10-02 11:38:01
1/4 Test #1058: R1 message-thread triggers vs render clock / autopilot on one deck ......   Passed    0.28 sec
2/4 Test #1059: R2 clip runtime fields: trigger writes vs render transport write-back ...   Passed    0.22 sec
3/4 Test #1060: R4 tuple consistency and no lost fade under a paced trigger storm .......   Passed    0.20 sec
4/4 Test #1061: R3 manual scalar writes vs eff() reads ..................................   Passed    0.23 sec
100% tests passed, 0 tests failed out of 4
Label Time Summary:
tsan    =   0.93 sec*proc (4 tests)
Total Test time (real) =   0.95 sec
probe-tsan-unit: ctest rc=0 2026-10-02 11:38:02
WARNING: ThreadSanitizer count: 0
```
(3) INFO: the fix round's other new tests built in build-tsan and run once with the ruling's TSAN_OPTIONS: 
`test_layer_runtime TSan rc=0 warnings=0 All tests passed (1107 assertions in 15 test cases)` (incl. the F2 stress
through the real Autopilot), `test_log_line_lint ... (54 assertions in 3 test cases)`, `test_clip_transport_sync ...
(17 assertions in 4 test cases)`. The full TSan ctest was NOT re-run this round (B3's 1085/1085 stands for the
untouched targets).
(4) Harmony constraints: no mutex added (grep of the diff: no std::mutex / lock_guard; logLinef's single fwrite takes
the stdio FILE lock exactly as logLine / std::cerr did before); the render never waits (the F2 guard runs inside the
existing bounded 16-attempt CAS loop and returns the tuple unchanged -- no retry, no wait); the audio callback is
untouched (`git diff bf5c116 HEAD -- src/audio` empty); the analysis thread allocates nothing in steady state
(F3: its four statements are logLinef, measured 0 mallocs; test_log_line_lint pins 0 logLine( in AnalysisThread.cpp).
(5) Rig: no app launched this round (no live lock taken; no Output window by any path; no synthetic input; no screen
capture; no debugger / sample / dtrace). The load runs used 10 busy-loop burners recorded by pid and killed on EXIT
(`burners left alive: 0` after each batch). No .venv symlink was created. Main checkout and its build/ only read
(git show of bf5c116 files into scratch / over my own tree for the F3 RED, restored sha256-checked). CLAUDE.md
unchanged (24,006 B). src/audio, test_device_policy, test_audio_engine_devices, probe-btguard.sh, Pitfall 61: untouched.

### Notebook notes (for Harmony to append)
- 2026-10-02 -- a threaded Catch2 case needs a START handshake | Files: tests/test_layer_runtime_race.cpp,
  tests/test_manual_scalar_race.cpp | a render thread spawned right before a ~10 ms main loop is often not scheduled
  inside it on a loaded machine: `render frames 0` (R1 14/30, R2 14/30 under 10 burners); wait (bounded) for its first
  frame before the loop and for a floor after it | discovered: s-rta-1002 fix round F1.
- 2026-10-02 -- a GL-thread trigger decided from a tuple snapshot must be conditioned on it | Files: src/model/Layer.h,
  src/model/Autopilot.cpp | `triggerClip(col, snap, 16, onlyIfActive = decidedFrom)`: the check sits INSIDE the pure
  CAS function (not before the call), and the post-CAS retrigger tail must also respect it | discovered: F2.
- 2026-10-02 -- logLine allocates, logLinef does not | Files: src/core/LogLine.h | an ostringstream line of 20+ chars
  costs 2-3 mallocs (measured with a DYLD_INSERT_LIBRARIES interposer); a thread under Sacred Rule 3 uses
  logLinef(fmt, ...) (stack buffer); `%g` prints a double byte-identically to an ostream (4M values compared) |
  discovered: F3.
- 2026-10-02 -- `ctest -L <label>` with no matching test exits 0 | Files: .harmony/probe-tsan-unit.sh | count with
  `ctest -L <label> -N` ("Total Tests:") and pass `--no-tests=error` | discovered: F7.
