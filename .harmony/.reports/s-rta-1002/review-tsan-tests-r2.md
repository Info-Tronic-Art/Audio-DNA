# Reviewer Verdict -- tsan lane, lens tests, round 2
STATUS: DONE
VERDICT: APPROVE (no MUST; 3 NITs)
REVIEWED: lane/tsan head a70f8ecca1a4921f9a451300e2410e51430c3c8f (fix diff bf5c116..a70f8ec) against tsan-fix-rulings.txt F1-F8

## Result per ruled item (all done as ruled)
- F1 VERIFIED: tests/test_layer_runtime_race.cpp:110 waitForFrames (bounded 10 s); R1 :184 / R2 :269 wait for renderFrames >= 1 else
  FAIL "render thread never started" (stop + join first); post-loop floor :211 / :283 (>= 100, bounded); `> 0` vacuity guard kept :217 / :289;
  R3 the same in tests/test_manual_scalar_race.cpp. Independent loaded run (10 busy-loop burners, pids recorded, killed on EXIT, normal
  build-lane binaries): R1 0/30, R2 0/30, R3 0/30, R4 0/10, F2 stress 0/30; also 24 burners + 6 parallel stress copies x6: no failure.
  After: no burner and no test process left in ps.
- F2 (memmodel, tests side) VERIFIED: Layer.h:548-556 guard inside the pure function; 3 deterministic cases + bounded stress. My own mutant
  (guard lines removed, $TMPDIR build, header override): 3 of 15 cases fail (test_layer_runtime.cpp:435-439, :458-460, :561).
- F4 VERIFIED: tests/test_undo_commands.cpp D1d (live clear, triggerClipImmediate(1), perform, REQUIRE landed stands; undo / redo checked).
  DeckCommands.h:390-394 skip is what it pins; the builder's mutant fails :3182. (Not rebuilt by me: test_undo_commands is the full link; reasoned
  from the code: removing the skip makes execute() apply `after` over `landed`.)
- F5 VERIFIED: R2 render side = captureLayerRuntime + ClipTransportSync::pushIntent / writeBack + FakePlayer (:229-:262); outPoint 0.9 both clips
  so the out-point branch runs. TSan build-tsan binaries at head: R1 R2 R4 R3 x3 each rc=0, 0 warnings.
- F6 VERIFIED: tests/test_clip_transport_sync.cpp:85-107. My own mutant (OneShot stop as an unconditional playing.store(false)): fails :124,
  4 cases | 3 passed | 1 failed -- equals the builder's evidence.
- F7 VERIFIED: probe-tsan-unit.sh counts `ctest -L tsan -N` "Total Tests:", exit 3 below EXPECTED_TSAN_CASES=4, ctest --no-tests=error. `ctest -N -L tsan`
  in build-tsan = 4. Builder's fake-dir RED/GREEN evidence read; I did not re-run the full probe (it rebuilds in the tree under review).
- F8 VERIFIED: tests/test_layer_runtime_race.cpp:345-352 deadline inside the loop, per wait; stall still REQUIRE_FALSE(stalled).
- Counts VERIFIED: ctest -N build-lane = 1092; tsan label = 4; test_layer_runtime 15, test_clip_transport_sync 4, test_log_line_lint 3,
  test_undo_commands 81 cases. src/audio untouched (diff empty), no stray files in git status.
- F3 test side: logLinef tests pin the old text (streamed() oracle) + truncation + lint (0 logLine( / 4 logLinef( in AnalysisThread.cpp).

## NIT (none blocks)
N1. R1/R2/R3 ignore the result of the post-loop waitForFrames(..., 100) (:211 / :283; R3 likewise): a render thread that managed only 1-99 frames
    still passes, so the overlap floor is not enforced -- only the `> 0` guard is. Ruled as written ("bounded wait; > 0 stays the guard"), so not a defect.
N2. R2 under TSan cannot tell the CAS write-back from an atomic plain store: my mutant (CompareExchange -> clip.playing.store(now)) stays at 0 warnings x3,
    as expected (an atomic store is race-free). The lost-update teeth live only in test_clip_transport_sync (deterministic) and the Renderer lint, which is
    adequate; just do not read R2 as covering the CAS semantics.
N3. The F2 stress case has `REQUIRE(advances > 0)` with no post-loop overlap floor (unlike R1-R3). 0 failures in my 66 loaded runs and 0 by construction for the
    CHECKs, so a theoretical false-fail only under a starved render thread; a `waitForFrames`-style floor on `advances` before the clear loop would close it.

## CAVEATS
- Not re-run: the F4 mutant (full test_undo_commands link), the F8 demo, full probe-tsan-unit (rebuilds), the full ctest (cross-lane mutex).
- Mutant rig: header override via -I ahead of src in a $TMPDIR compile, linking the lane's existing objects; tree under review not edited.
