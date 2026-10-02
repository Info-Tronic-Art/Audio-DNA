# Reviewer Verdict -- tsan lane, lens tests, round 1
STATUS: PARTIAL
VERDICT: REQUEST_CHANGES (FAIL: 1 MUST)
REVIEWED: lane/tsan head bf5c116e57005a15b9a4d6c172a19cbddf07a3ce vs base 2d38b394b5689c802512c55d77d32320862f1e95
METHOD: read via git show / git diff only; runs of the lane's existing build-lane / build-tsan binaries (read-only); T0 tree
extracted with git archive to $TMPDIR and rebuilt under TSan with the committed .harmony/probe-tsan-unit.sh.

## MUST
M1. R1 / R2 / R3 FALSE-FAIL under CPU load in the NORMAL build (flaky gate; also a vacuity risk).
- tests/test_layer_runtime_race.cpp:168 (R1) and :241 (R2), tests/test_manual_scalar_race.cpp:90 (R3): `CHECK(renderFrames.load() > 0)`.
  The main loop (20,000 / 40,000 / 40,000 iterations) takes ~10 ms in a normal build; the render thread is spawned right
  before it with no handshake, so when it is not scheduled in that window `render frames 0` fails the case (and with 1-2 frames
  the case passes having tested nothing). The plan wording is "post-join: DETERMINISTIC range checks only".
- MEASURED (VERIFIED, build-lane binaries at the lane head, 100+ runs idle = 0 failures each):
  12 busy-loop procs on 10 cores, serial runs: R1 5/30 fail, R2 10/30 fail; 10 parallel copies: R1 11/60, R2 ~50 %, R3 up to 2/15;
  4 parallel copies only: R1 5/160. Every failure is `test_layer_runtime_race.cpp:168 / :241: renderFrames 0 > 0`.
  Under TSan the cases overlap well (R1 14k-45k render frames in 8 runs under the same load) -- the flake is the normal-build arm
  (G1 full ctest, run while another lane builds -- the rig's normal state, see B1's collided-ctest note).
- FIX: a start handshake -- render thread publishes "running" after its first frame; main waits for renderFrames >= 1 before the loop
  and, after the loop, until renderFrames >= (a floor, e.g. 100) with a bounded wait; then the `> 0` check can stay as a real
  vacuity guard. Same for R3. (R4 and the contention / activation-tail tests are paced by handshakes and did not flake:
  0/240 and 0/100 under the same load.)

## SHOULD
S1. ClearActiveClipCmd's first-execute no-op (amendment 6) has no test and no mutant evidence: src/core/DeckCommands.h:390-394 can lose
    its skip and every test in tests/test_undo_commands.cpp (1378-1410 and the kClipClear composite 1110-1160, both idempotent with the
    live clear) still passes; D1 / D1b / D1c and mutant m2 cover TriggerClipCmd only. Add a D1d (live clear, then a GL transition
    -- e.g. a triggerClipImmediate or a fired pending -- then perform; REQUIRE the transition stands) and run the m2-style mutant.
S2. R2's render side is a mirror of the PRE-lane Renderer code (tests/test_layer_runtime_race.cpp:195-222: plain `c->playing = ...`), so
    after T4 the TSan unit gate no longer exercises the real write-back. ClipTransportSync.h is header-only and linkable headless
    (test_clip_transport_sync links only Clip.cpp): switch R2's mirror to ClipTransportSync::pushIntent / writeBack so the
    [tsan] case drives the real CAS write-back against the main-thread triggers. (Spec-conformant as written at T0; stale since T4.)
S3. test_clip_transport_sync has no case that distinguishes the OneShot stop's `if (wrote)` guard or the LOCAL-playhead out-point test
    (src/render/ClipTransportSync.h:60-70): `clip.playing.store(false)` unconditionally at the out-point survives all 3 cases (the
    builder's own m6 note: "the OneShot case passes under the mutant"). Add: intent changed inside the window AND the out-point
    crossed -> the newer intent survives.

## NIT
N1. R4 `deadline` (tests/test_layer_runtime_race.cpp:296) is ONE 60 s budget for all 20,000 triggers, not per wait: a slow-but-live run
    reports "render stalled". Make it per wait (reset after each wait).
N2. R4's I3 has no killing mutant evidence: m1 and m5 fail through I2 (builder notes it). Fine, but I3 is untested teeth.
N3. tests/test_layer_runtime.cpp bounded-exhaustion case (:~190-205) would spin forever, not fail, if maxAttempts were ignored
    (fn changes the tuple every call); give fn a call cap (e.g. stop after 1000 calls and report).
N4. .harmony/probe-tsan-unit.sh:25 hardcodes TARGETS; a future [tsan] target missing from it is not built (ctest then fails "not
    found", not silently green -- acceptable, but derive the list from `ctest -L tsan -N` or document in the script header).
N5. test_render_thread_lint case 2 regex (tests/test_render_thread_lint.cpp, runtimeLoad) does not see captureLayerRuntime( or
    LayerClock::advanceCrossfade(Layer&, float) (LayerClock.h:54, its own load) -- a render-side call through those is outside the pin.

## VERIFIED OK (disk / executed)
- T0 RED, INDEPENDENT: git archive 2c04a03 -> $TMPDIR, `.harmony/probe-tsan-unit.sh` (fresh configure + build + ctest -L tsan): 4/4 Failed
  (`Error regular expression found in output`), rc=8, 122 warnings. Direct runs with the pinned TSAN_OPTIONS: R1 rc=66 95 races,
  R2 rc=66 8 races, R4 rc=66 4 races, R3 rc=66 4 races; R1 / R3 name Layer.h / TriggerCommands.h / ManualWrite.cpp frames; R2 / R4 name
  src/ only via the ALLOCATION stack (Deck.h:38/39, Deck::initDefault) -- inside amendment 2's "ANY stack" bar.
  Normal-build RED facts re-derived: 75 std::cerr statements in the D6 list at 2c04a03; Renderer.cpp plain stores at 1826 / 1833 / 1911 / 1918.
  The builder's normal ctest list (D1 / D1b / D1c / D6 / lint case 1 + R1 under H11; 464/471/472/821/823 = the cross-lane temp-file collision,
  isolated pass) is consistent with that.
- R4 vs amendment 3: Deck-heap Layer (deviation, justified in the file: stack Layer -> TSan reports nothing), 4 clips, step 0.3, triggerClipImmediate(0)
  before start, paced k = 1..5 on renderObs, cyclic triggers, I1-I4 counted, REQUIRE(sum == 0), stall = FAIL. R1: third reader (readAll: tuple +
  playing / playheadPosition / opacity), Layer copy every 17th, per-snapshot range + I1 counter; R2: third reader. All present.
- Registration (amendment 2): tests/CMakeLists.txt ADNA_TSAN_TEST_PROPERTIES = LABELS tsan, TIMEOUT 300, ENVIRONMENT TSAN_OPTIONS=exitcode=66:halt_on_error=0:
  abort_on_error=0:report_signal_unsafe=0:history_size=4, FAIL_REGULAR_EXPRESSION "WARNING: ThreadSanitizer"; confirmed in the generated
  test_layer_runtime_race-*_tests.cmake; `ctest -L tsan` lists R1 R2 R4 R3. probe-tsan-unit.sh is in the T0 commit (2c04a03) and ran.
- D1 / D1b / D1c match amendment 6 exactly (tests/test_undo_commands.cpp ~3070-3160); m2 evidence fails all three.
- Amendments 4 / 5 / 10 tests exist (bounded exhaustion, contention with REQUIRE adopts > 0 and I1 / I2, activation-tail contract, three releaseMomentary
  tests); Layer.h updateRuntime calls beforeCas BEFORE casRuntime (src/model/Layer.h:314-326) -- the review-item order holds; clear tail after the CAS (applyClearTail).
  Amendment-16 table complete: m1-m6 each with failing output; m4 rare (3/30) as the ruling predicted. contention test 0/240 fails under load (stable).
- Type pins (amendment 11): Clip x4, manualRef x3 returning RelaxedFloat&, activeDeckIndex RelaxedInt, LooseTuple concept (+ a positive control on the snapshot),
  all with the pre-change compile RED recorded. test_relaxed covers copy / move / implicit / compareExchange failure-writes-expected / fetchAdd / nothrow pins.
- Lint case 1 (plain-store regex + >= 2 ClipTransportSync:: sites; lane has 4) and case 2 (pins 2/2, 0/1, 1/0, 3/0 equal the real counts at bf5c116:
  CompositorEngine.cpp 1052/1078/1278/1337, Renderer.cpp 602, DeckClock.h 32, Autopilot.cpp 27/69/83); each site's function is in the comment.
- Migrated tests: sampled test_deck_clock / test_autopilot / test_composition / test_program_preamble / test_compositor / the two strip tests -- purely mechanical
  (`x->activeClipColumn` -> `x->runtime().activeClipColumn`; field writes -> read-modify-write block); no assertion or call order changed.
- Nothing stray: no .venv / .new / mutant files in `git ls-tree bf5c116`; diff -U0 shows no hunk in bt2's MainComponent fences (484-494, 2144-2160, 3070-3095,
  5690-5705), src/audio and the bt2 tests untouched, ApiServer hunks at 2 / 94 / 97 / 124 / 360 / 384 / 418-440 / 1488 / 1548 only, tests/CMakeLists.txt
  appended at the end only; no new mutex / allocation / syscall on the audio path; CLAUDE.md 24,006 B (<= 25,000).
- T7: 75 cerr statements in 14 files (list = plan T7 + H3), conversions re-read on AnalysisThread / VideoPlayer: operand lists identical.
- Analyzer re-run on the fresh-RED archive: 25 uniques, 9 launches, families A / B / C / D / E all present (matches the report).

## CAVEATS
- Not re-run: the mutants m1-m6 (builder evidence taken from the report; I did re-derive T0 RED and the flake independently), the G3 app sweep, T8 tooling beyond reading.
- ASSUMED: the rig's concurrent builds make the M1 load level realistic (B1 recorded two lanes' ctest collisions).
