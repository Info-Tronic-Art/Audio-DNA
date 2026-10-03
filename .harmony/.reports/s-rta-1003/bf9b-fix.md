# bf9b-fix -- lane report of the FIX stages (s-rta-1003)
Spec: .harmony/.reports/s-rta-1003/ruling-bf9b-merge.md (AM-1..AM-18) + the HARMONY ADOPTION at the end of
plan-bf9b-merge.md. Worktree .claude/worktrees/bf9b, branch lane/bf9b. Scratch: <session scratchpad>/bf9b-fix-FIX-1/.

## STAGE FIX-1 MEMORY (AM-1, AM-2, AM-4, AM-5, AM-6 src, AM-8, adoption item 2 / AS7)
STATUS: PENDING
Started 2026-10-03 14:56:47 from b70ce61 (git status clean).

### Commit 1 -- the gate + the RED run on untouched src (AM-5; AS0 tag, AS5, AS6, AS7)
Files: tests/CMakeLists.txt (ADNA_ASAN_TEST_PROPERTIES; test_show_model also compiles ui/ClipInspector.cpp; a second
catch_discover_tests TEST_SPEC "[asan]" TEST_PREFIX "asan:" only when ADNA_SANITIZE holds "address"),
tests/test_show_model.cpp ([asan] tag on the existing case = AS0, body unedited; AS5, AS6, AS7; helper AppInspectors =
both inspectors wired to the fence as MainComponent wires them AT THIS HEAD), .harmony/probe-asan-unit.sh (new).
`git diff --stat -- src` at commit 1: empty (src untouched).

FM-4 (does a second catch_discover_tests register on this Catch2?): YES. `ctest --test-dir build-asan -N -L asan`:
```
  Test #150: asan:bf9b fix: a fenced edit that moves or resizes the shared layer stack calls onLayerStackMoved, ...
  Test #151: asan:AS5 a model swap inside the fence with both inspectors bound, ...
  Test #152: asan:AS6 Undo of a wide Load Deck while the Clip inspector shows a clip of that deck, ...
  Test #153: asan:AS7 Layer > Clear Clips while the Clip inspector shows a clip of that row that has one effect: ...
Total Tests: 4
```
-> the second-discovery form is used; no dedicated asan binary.

FM-2 (is AS0 RED under ASan through the new script on the pre-fix src?): YES. RED run, raw
(`ADNA_JOBS=6 bash .harmony/probe-asan-unit.sh`, 2026-10-03 15:01:13 -> 15:03:10; full log
bf9b-fix-FIX-1/c1-red-run.log):
```
probe-asan-unit: ctest -L asan finds 4 asan cases (expected 4)
1/4 Test #150: asan:bf9b fix: a fenced edit that moves or resizes the shared layer stack ... ***Failed
==48080==ERROR: AddressSanitizer: heap-use-after-free on address 0x621000003020 at pc 0x00010039c910 bp 0x00016fc817f0 sp 0x00016fc817e8
READ of size 1 at 0x621000003020 thread T0
    #0 UniversalParamControl::setParamValue(float) UniversalParamControl.cpp:349
    #1 LayerInspector::syncFromLayer() LayerInspector.cpp:911
    #2 LayerInspector::setLayer(Layer*, EffectScope) LayerInspector.cpp:734
    #3 UndoService::withDeckDetached(...) UndoService.cpp:78
  freed by: vector<Layer>::insert <- Composition::insertLayer Composition.h:484 <- InsertDeckCmd::execute DeckCommands.h:806
2/4 Test #151: asan:AS5 ... ***Failed
==48097==ERROR: AddressSanitizer: heap-use-after-free on address 0x623000000820 at pc 0x000100618910 bp 0x00016f9fc530 sp 0x00016f9fc528
READ of size 1  #0 UniversalParamControl::setParamValue UniversalParamControl.cpp:349  #1 LayerInspector::syncFromLayer
    #2 LayerInspector::setLayer LayerInspector.cpp:734  #3 UndoService::withDeckDetached UndoService.cpp:78
  freed by: Composition::operator=(Composition&&) Composition.h:41
3/4 Test #152: asan:AS6 ... ***Failed
==48099==ERROR: AddressSanitizer: heap-use-after-free on address 0x61f00000b0c8 at pc 0x00010506c910 bp 0x00016afa8190 sp 0x00016afa8188
READ of size 1  #0 UniversalParamControl::setParamValue UniversalParamControl.cpp:349
    #1 ClipInspector::syncFromClip() ClipInspector.cpp:1467  #2 ClipInspector::setClip(Clip*, EffectScope) ClipInspector.cpp:820
  freed by: vector<ClipRow> destroy <- vector<Deck>::__destruct_at_end <- InsertDeckCmd::undo DeckCommands.h:824
4/4 Test #153: asan:AS7 ... ***Failed
test_show_model.cpp:1677: FAILED:  CHECK( ui.clip.getClip() == nullptr )  with expansion: 0x000062a000006200 == nullptr
==48101==ERROR: AddressSanitizer: heap-use-after-free on address 0x6100000246b8 at pc 0x000102459e30 bp 0x00016dbb8f20 sp 0x00016dbb8f18
READ of size 1  #0 UniversalParamControl::~UniversalParamControl() UniversalParamControl.cpp:122
    <- ~EffectRow <- EffectStackView::~EffectStackView()
  freed by: Clip::EffectSlot::~EffectSlot <- Clip::~Clip <- the Clear Clips lambda <- UndoService::withDeckDetached
0% tests passed, 4 tests failed out of 4
probe-asan-unit: ctest rc=8 2026-10-03 15:03:10
PROBE-ASAN-UNIT RED (4 of 4 failed)
exit=8
```
MEASURED about AS7 (the ruling's R5 called it INFERRED): on the pre-fix src the two timer calls after Layer > Clear
Clips (tickModulation + refresh) produced NO ASan report in this run -- the dead Clip sits in the row's still-allocated
buffer, and the stale effect-chain read was skipped. The case is RED twice over all the same: its CHECK (the inspector
still shows the dead clip) and an ASan heap-use-after-free when the effect row's control is destroyed and reads its
binding into the freed EffectSlot (the same read a row rebuild or a re-bind makes).

### Commit 2 -- the fix, the hook function, the cases, lint B4h, MU1-MU3
SRC (11 files, +1 new):
- AM-1: `LayerInspector::setLayer` / `ClipInspector::setClip` call `forgetScalarBindings()` FIRST (new private
  method: 7 / 6 x `UniversalParamControl::forgetConnection()`); nothing else in them changed; `bindConnection`
  unchanged. Pinned token for lint B4h (ii): `forgetScalarBindings();`.
- AM-2: NEW `src/ui/InspectorRepoint.h` (headless, inline): `compositionOwnsClip(const Composition&, const Clip*)`
  (an address walk over forEachClip: live + retired decks, never dereferences the Clip),
  `clearClipInspectorIfUnowned(ClipInspector&, const Composition&)` (= step 1, "owned, or clear"),
  `repointInspectorsAfterStackMove(ClipInspector&, LayerInspector&, Composition&, int selectedLayerRow)` (step 1, then
  the Layer inspector by the selected row; a stale row clears it). MainComponent's hook statement calls exactly it
  with `deckView_->getSelectedLayerIndex()`. `repointLayerInspector()` stays for refreshAfterUndoRedo.
- ADOPTION ITEM 2: `UndoService::onFencedEdit` (new std::function), one more call site in the hand-over lambda:
  `if (moved) onLayerStackMoved(); else if (onFencedEdit) onFencedEdit();` -- so after ANY fenced edit exactly one of
  the two hooks runs and the "owned, or clear" check runs every time (inside repointInspectorsAfterStackMove on a
  stack move, alone otherwise). MainComponent: `undoService_.onFencedEdit = ... clearClipInspectorIfUnowned(...)`.
  WHY `else` and not an unconditional second call: with both running on a stack move, step (1) of the hook function
  would be dead code and the ruling's pre-registered MU2 (step 1 removed -> AS3b RED) could not bite.
- AM-6 src (test-server block only): the lever also calls `deckView_->selectLayer(n)`; `/api/debug/ui_text` gains
  `inspected_layer`, `inspected_clip` (the bound layer's / clip's name, "" when none) and `inspector_tab` ("Clip" /
  "Layer" / "Composition" / "Signal"), read in the same message-thread hop (ApiServer onDebugInspectedLayer /
  onDebugInspectedClip / onDebugInspectorTab). NOT exercised live in this stage (no app was launched): FIX-4's
  probe-asan-live.sh is their first reader.
- Test seam: `LayerInspector::opacityControlForTest()`.
TESTS: tests/test_show_model.cpp AS1, AS2, AS3b ([asan]), AS3, AS4, N1 (functional); helper `AppInspectors::wire`
now calls the two production functions (the ONE test edit between the commits; AS0 / AS5 / AS6 / AS7 bodies
unedited). tests/test_render_thread_lint.cpp: case "bf9b B4h: ..." (i) / (ii) / (iii) incl. the onFencedEdit wiring
and call; B4f gains ONE pin `{ "ui/InspectorRepoint.h", 1 }` (see DEVIATIONS). probe-asan-unit.sh
EXPECTED_ASAN_CASES 4 -> 7.

NAME MAP (B2 "by name"):
- AS0 = "bf9b fix: a fenced edit that moves or resizes the shared layer stack calls onLayerStackMoved, so a Layer
  inspector re-pointed there never holds a moved or removed Layer (...)"; AS1 / AS2 / AS3 / AS3b / AS4 / AS5 / AS6 /
  AS7 / N1 = the test_show_model cases whose names begin with that id; B4h = "bf9b B4h: the stack-move hook is wired
  to repointInspectorsAfterStackMove, setLayer / setClip forget their scalar bindings first, and the fence hands over
  on both exits".
- ctest count: 1234 (M3 head) + 10 added (AS1, AS2, AS3, AS3b, AS4, AS5, AS6, AS7, N1, B4h) - 0 retired = 1244.
  A Release build registers no "asan:" duplicate (the second discovery is inside `if("address" IN_LIST
  ADNA_SANITIZE)`); build-asan lists 7 under the label.

GREEN (2026-10-03 15:07:16 -> 15:07:29; bf9b-fix-FIX-1/c2-green-run.log):
```
probe-asan-unit: ctest -L asan finds 7 asan cases (expected 7)
100% tests passed, 0 tests failed out of 7
probe-asan-unit: ctest rc=0 2026-10-03 15:07:29
PROBE-ASAN-UNIT GREEN (7 cases, 0 reports)
```
AS0 turned clean with NO edit of its body (the ruling's R3).

MUTANTS (never committed; run once each by bf9b-fix-FIX-1/mutants.sh on the uncommitted commit-2 tree; restored
byte-for-byte: "RESTORED: sha256 of the 4 mutated files equal before / after", sha-before.txt == sha-after.txt; the
script then re-ran the probe: PROBE-ASAN-UNIT GREEN (7 cases, 0 reports)).
- MU1 (the two `forgetScalarBindings();` first statements removed) -- same script (mu1-asan.log):
  ```
  ==50530==ERROR: AddressSanitizer: heap-use-after-free ... READ of size 1   AS0  setParamValue <- syncFromLayer <- setLayer
  ==50541==ERROR: AddressSanitizer: heap-use-after-free ... READ of size 1   AS5  bindConnection <- bindScalarControls <- ClipInspector::setClip
  ==50543==ERROR: AddressSanitizer: heap-use-after-free ... READ of size 1   AS6  bindConnection <- bindScalarControls <- ClipInspector::setClip
  ==50546==ERROR: AddressSanitizer: heap-use-after-free ... READ of size 1   AS1  setParamValue <- syncFromLayer <- setLayer
  ==50548==ERROR: AddressSanitizer: heap-use-after-free ... READ of size 1   AS2  setParamValue <- syncFromLayer <- setLayer
  ==50550==ERROR: AddressSanitizer: heap-use-after-free ... READ of size 1   AS3b bindConnection <- ClipInspector::setClip
  14% tests passed, 6 tests failed out of 7
  PROBE-ASAN-UNIT RED (6 of 7 failed)
  ```
  AS0, AS1, AS2, AS5, AS6 RED as pre-registered; AS3b RED too (the hook's setClip(nullptr) reads the freed row);
  AS7 stays green under MU1 (its dead Clip's scalar connections sit in the row's still-allocated buffer).
  AS2's report is a READ, not the WRITE the ruling wrote: halt_on_error=1 stops at the first report, and the read
  in syncFromLayer comes before bindConnection's release-write.
  N1 under MU1 (mu1-n1.log, the build-asan binary, `"N1*"`): `CHECK( lane.grip.kind == ...::Held ) with expansion:
  0 == 1`, `CHECK( touch.grip.kind == ...::Decaying ) with expansion: 0 == 2`, `test cases: 1 | 1 failed` -- RED
  before the fix. Lint B4h (ii) under MU1 (mu1-lint.log): 2 CHECKs FAILED (setLayer / setClip do not begin with
  the forget).
- MU2 (the `clearClipInspectorIfUnowned(clipInspector, comp);` line removed from repointInspectorsAfterStackMove)
  -- same script (mu2-asan.log):
  ```
  test_show_model.cpp:1789: FAILED:  CHECK( ui.clip.getClip() == nullptr )
  ==50646==ERROR: AddressSanitizer: heap-use-after-free on address 0x62a00001221c ... READ of size 1
      #0 ClipInspector::tickModulation() ClipInspector.cpp:956   #1 test_show_model.cpp:1791
  86% tests passed, 1 tests failed out of 7
  PROBE-ASAN-UNIT RED (1 of 7 failed)          (the failed one: asan:AS3b)
  ```
- MU3 (the `undoService_.onLayerStackMoved = ...` statement deleted from MainComponent.cpp) -- lint B4h, any build
  (mu3-lint.log): `CHECK( count(mc, "undoService_.onLayerStackMoved =") == 1 ) with expansion: 0 == 1` (+ 2 more),
  `test cases: 1 | 0 passed | 1 failed`. Its live arm (ASAN-MU3) is FIX-4's.

FULL ctest, serial, under the mutex (build-lane rebuilt after the mutants were restored: rc 0; 2026-10-03 15:09:56 ->
15:12:06): `100% tests passed, 0 tests failed out of 1244`, `Total Test time (real) = 130.66 sec`.
STOP CLAUSES OF THIS STAGE: (a) AS0 RED on the pre-fix src: YES (FM-2) -- no stop. (b) an ASan report in a case the
ruling does not name: none (the label runs only the named cases; AS7 is the adoption's). (c) an existing test that
asserted a release on a rebind: none -- 1244 / 1244 with the fix, no existing test edited.

### PACKET QUALITY
(pending)

### SLIM CHECK
(pending)
