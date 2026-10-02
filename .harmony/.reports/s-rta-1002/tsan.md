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

