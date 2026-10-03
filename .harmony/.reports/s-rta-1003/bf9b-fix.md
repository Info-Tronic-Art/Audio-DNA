# bf9b-fix -- lane report of the FIX stages (s-rta-1003)
Spec: .harmony/.reports/s-rta-1003/ruling-bf9b-merge.md (AM-1..AM-18) + the HARMONY ADOPTION at the end of
plan-bf9b-merge.md. Worktree .claude/worktrees/bf9b, branch lane/bf9b. Scratch: <session scratchpad>/bf9b-fix-FIX-1/.

## STAGE FIX-1 MEMORY (AM-1, AM-2, AM-4, AM-5, AM-6 src, AM-8, adoption item 2 / AS7)
STATUS: DONE (2026-10-03 14:56:10 -> 15:14:51; concerns = "DEVIATIONS / STOP ITEMS FOR HARMONY" below)
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

### End of stage (head fed5510; tree == the tree the full ctest ran on: `git diff --quiet -- src tests` rc 0)
- Full ctest, serial, under /tmp/audiodna-ctest.lock: `100% tests passed, 0 tests failed out of 1244` (15:09:56 ->
  15:12:06).
- `.harmony/probe-tsan-unit.sh` (unmodified: `git diff --quiet b70ce61 -- .harmony/probe-tsan-unit.sh` rc 0), 15:13:13:
  `probe-tsan-unit: ctest -L tsan finds 5 [tsan] cases (expected 5)`, `100% tests passed, 0 tests failed out of 5`,
  exit 0, 0 "WARNING: ThreadSanitizer" (final-tsan.log).
- `.harmony/probe-asan-unit.sh`, 15:13:20: `PROBE-ASAN-UNIT GREEN (7 cases, 0 reports)`, exit 0 (final-asan.log).
- `wc -c CLAUDE.md` = 24224 (unchanged; no CLAUDE.md line, no pitfall).
- No app was launched in this stage, no lock taken, no Output window, no build left running. New build dir:
  build-asan (gitignored; 300 GB free before it).
- Commits: 8487a57 (commit 1), fed5510 (commit 2), + this report's docs commit.

### FACTS MEASURED
- FM-2: YES -- `1/4 Test #150: asan:bf9b fix: a fenced edit that moves or resizes the shared layer stack ...
  ***Failed` / `==48080==ERROR: AddressSanitizer: heap-use-after-free on address 0x621000003020 ... READ of size 1`
  (UniversalParamControl::setParamValue <- LayerInspector::syncFromLayer <- LayerInspector::setLayer), through
  probe-asan-unit.sh on untouched src: `PROBE-ASAN-UNIT RED (4 of 4 failed)`.
- FM-4: YES -- `ctest --test-dir build-asan -N -L asan` -> `Total Tests: 4` at commit 1 (7 at commit 2) from the
  second catch_discover_tests (TEST_SPEC "[asan]", TEST_PREFIX "asan:"); no dedicated binary needed.

### DEVIATIONS / STOP ITEMS FOR HARMONY
1. B4f (the ruling's "B4f counts unchanged" proof for FIX-1): the 12 existing pins are unchanged, but the lint needed
   ONE NEW pin, `{ "ui/InspectorRepoint.h", 1 }`: AM-2's own `setClip(nullptr)` in the new header matches B4f's
   `setClip(` token (a UI setter, the class the lint's comment already names for ClipInspector / InspectorPanel).
   Without it the lint is RED. Needs Harmony's confirmation (or the ruling's wording corrected).
2. Adoption item 2's call site is an `else` (onLayerStackMoved on a stack move, onFencedEdit otherwise), not a second
   unconditional call. Every fenced edit still runs the check exactly once. Reason: an unconditional call would make
   step (1) of the hook function unkillable and void the pre-registered MU2. If Harmony wants it unconditional: one
   word in UndoService.cpp, and MU2 must then be redefined (remove the check's body -> AS3b and AS7).
3. AS7 on the pre-fix src: RED by its CHECK and by an ASan heap-use-after-free at the effect row control's
   destruction; the two timer calls themselves printed no report (see commit 1). The adoption's wording
   ("..., Clear Layer Clips, tickModulation") is met as a sequence, not as "tickModulation is the reporting frame".
4. AS2 under MU1 reports a READ (first report wins under halt_on_error=1), not the WRITE the ruling wrote.
5. MU1 also turns AS3b RED (not in the ruling's MU1 list; a named case, same cause).
6. The three ui_text fields and the lever's selectLayer line were compiled and read only -- no app was launched
   here. FIX-4's probe-asan-live.sh is their first live reader (L0's VALID clause).
7. The stash-guard hook twice refused a command whose TEXT (a commit message, then this report's text) it read as a
   whole-tree stage; the same work went through with the text in a file. Paths were staged one by one; the guard
   was never overridden.

### FOUND, NOT FIXED
1. A consequence of adoption item 2 worth a note: Column > Add Column / Remove Column (fenced `deck->addColumn()` /
   `removeColumn`) can move a row's clips; before this stage the Clip inspector kept the stale pointer (the same
   freed-memory class, not in the ruling's lists); now the Clip tab goes EMPTY when its clip moved. Memory-safe;
   whether it should instead re-point by the selected cell is Q-C's territory. INFERRED from reading (Deck.h
   addColumn -> ClipRow::ensureColumns resize); no test drives it.
2. LayerStrip has no destructor and strips are not forgotten (ruling SF-2 / RR-6): untouched.
3. docs: integration.md (ui_text's three fields), testing-eyes.md (the lever now selects the row), Pitfall 33's
   sentence, APP-INVENTORY -- AM-14 assigns them to FIX-3 / FIX-4; not written here.

### NOTES FOR THE NEXT STAGES
- FIX-2: T6f / T6g / T6j join the asan label: tag them `[asan]`, raise EXPECTED_ASAN_CASES 7 -> 10 in
  .harmony/probe-asan-unit.sh (TARGETS already holds test_show_model). B4f: DeckCommands.h re-pin as ruled; the
  InspectorRepoint.h pin is item 1 above.
- Tests that need both inspectors: `AppInspectors` (tests/test_show_model.cpp) = both inspectors wired through the
  production functions; `showHoldsClipAt` = the tests' own address walk.
- FIX-4: ui_text now answers inspected_layer / inspected_clip / inspector_tab; the default show's layer names must be
  read live before L0's `"Layer 2"` is trusted (not checked here). ASAN-MU3 = delete the
  `undoService_.onLayerStackMoved = [this]() { ... };` statement (5 lines) in MainComponent.cpp; with it gone the
  `else` branch sends stack moves to onFencedEdit, which still clears the Clip inspector but does NOT re-point the
  Layer inspector -- so L1 stays the expected RED step (INFERRED).
- build-asan exists (RelWithDebInfo, ADNA_SANITIZE=address, only test_show_model built). The ASan APP needs its own
  dir (build-asan-app) or the AudioDNA target built there.

### Notes for .harmony/notebook.md (Harmony appends)
- A destroyed Clip in a ClipRow whose buffer stays allocated (clips.clear() + ensureColumns) is invisible to ASan
  for reads of the Clip's own inline fields (scalarConns is a std::array); only its heap children (effects, strings)
  report. A memory test for "Clear Clips" must give the clip a heap child AND assert the functional outcome.
  | discovered: tests/test_show_model.cpp AS7
- catch_discover_tests can be called twice on one binary: a second call with TEST_SPEC "[tag]" + TEST_PREFIX +
  PROPERTIES registers the tagged cases again under a label; guard it with `if("address" IN_LIST ADNA_SANITIZE)` so
  a normal build's count does not change. | discovered: tests/CMakeLists.txt (FM-4)
- The B4f lint counts every `setClip(` in src/, UI setters included: a new file that calls ClipInspector::setClip
  needs its own pin. | discovered: tests/test_render_thread_lint.cpp
- Harmony's stash-guard hook reads the whole command text, heredocs included: keep commit messages and report text
  in files. | discovered: this stage

INBOX-RECHECK: none (no message channel in this workflow run; nothing relayed after the packet)

### PACKET QUALITY
- Clarity: CLEAR, two HAD_TO_INFER points: (a) how adoption item 2's "one more call site" sits beside the ruling's MU2
  (deviation 2); (b) "B4f counts unchanged" against AM-2's setClip(nullptr) in a new file (deviation 1).
- Missing context: none that blocked. The ruling could not know that the Clear Clips tick does not report under ASan.
- Unused context: review-bf9b-live-r2 / gates-r2 and the plan body (read: state-r2, the ruling, the adoption), the
  live-app rig rules (no app was needed in this stage).
- Self-brief files: ruling-bf9b-merge.md (full), the plan's HARMONY ADOPTION, rulings-bf9b-mergein.md,
  rulings-bf9b-merge.md, bf9b-merge.md's hand-over -- all useful, none stale. No DEPARTMENT / KNOWLEDGE_TOOLS block:
  no knowledge tools -- grep-only (nothing judged dead on "no callers").
- pulse.json: GREEN; claims "general" by two harmony sessions (the dispatcher), no area conflict.

### SLIM CHECK
src: 1 new header (3 inline functions), 2 forget methods + 2 first statements, 1 std::function + 1 `else` branch in
the hand-over, 2 hook statements, 1 lever line, 3 ui_text fields, 1 test seam. tests: 9 cases + 1 lint case + 1 pin;
1 probe script; 1 CMake property set + 1 conditional discovery. Every line traces to AM-1 / AM-2 / AM-4 / AM-5 /
AM-6 src / AM-8 or adoption item 2. Not built (as ruled): forgetLayer / forgetClip / the grip rule / strip forget.
Smells named: AS1 / AS2 repeat their set-up (duplicated code, kept so each case stands alone under the asan label);
MainComponent::repointLayerInspector and repointInspectorsAfterStackMove's step (2) are two spellings of one re-point
(the ruling keeps refreshAfterUndoRedo's own).

## STAGE FIX-2 COMMANDS (AM-7, AM-10, AM-18's comment)
STATUS: PENDING
Started 2026-10-03 15:16:27 from 707818e (git status clean). Scratch: <session scratchpad>/bf9b-fix-FIX-2/.
### Items (appended as each lands)

### Item AM-7 -- Undo of Add / Duplicate / Load Deck (fork F2-C)
SRC: `src/core/DeckCommands.h` only.
- `AddDeckCmd::undo`: `cancelPendingInto(*comp, added_->id)`, then `retireOrEraseDeck(addedIndex_)` in place of the
  erase. Its redo: `restoreRetiredDeck(added_->id, addedIndex_)` first; the snapshot insert only when it is gone.
- `InsertDeckCmd::undo` WHEN `addedLayers_.empty()`: `cancelPendingInto`, `retireOrEraseDeck`; a RETIRED deck's cells
  are not disposed (early return), an erased deck's are (today's loop). Its redo: `restoreRetiredDeck` first, no
  media reconnect; else today's body. WHEN IT ADDED LAYERS: today's body (erase the deck, erase the layers, dispose) --
  the documented exception. The header comments (cancelPendingInto, the deck-ops banner, both classes) say so.
- Redo after a restore sets `activeDeckIndex = findDeckIndexById(added_->id)` (restoreRetiredDeck returns a bool).

TESTS (tests/test_show_model.cpp, after T6e; helper `undoRetiresPlayingDeckAndRedoRestoresIt` = the shared
assertions of T6f / T6g / T6j; the clip is fired with `Composition::fire`, i.e. no Undo step):
- T6f = "T6f Undo of Add Deck while a clip of the new deck plays: the deck is retired, the SAME Clip keeps playing, a
  queue into it is cancelled; redo moves it back under its id (bf9b fix, AM-7)" [show][asan]
- T6g = "T6g Undo of a Load Deck with the show's row count while its clip plays: retired, the SAME Clip keeps playing,
  no cell disposed while retired; redo moves it back and reconnects nothing (bf9b fix, AM-7)" [show][asan]
- T6h = "T6h THE EXCEPTION, pinned: Undo of a Load Deck that ADDED layers takes the deck and those layers back even
  while its clip plays -- every cell disposed once; redo brings the layers and the deck back under its id (bf9b fix,
  AM-7)" [show]
- T6i = "T6i Undo of a Load Deck retires the playing deck; once its clip is replaced a later fenced edit reaps it;
  redo then comes back from the snapshot with every cell reconnected (bf9b fix, AM-7)" [show]
- T6j = "T6j Undo of Duplicate Deck while a clip of the copy plays: the copy is retired, the SAME Clip keeps playing, a
  queue into it is cancelled; redo moves it back under its id (bf9b fix, AM-7)" [show][asan]
`.harmony/probe-asan-unit.sh`: EXPECTED_ASAN_CASES 7 -> 10 (T6f, T6g, T6j).

RED at FIX-1's head (the new tests on untouched src: `git diff --quiet -- src` rc 0; build-lane Release binary,
2026-10-03 15:18:41; logs red-T6*.log):
```
=== T6f rc=42
test_show_model.cpp:652: FAILED:  CHECK( c.getNumRetiredDecks() == 1 )   with expansion: 0 == 1
test_show_model.cpp:653: FAILED:  REQUIRE( c.playingClip(0) == playing ) with expansion: nullptr == 0x000000012d822600
test cases:  1 | 1 failed      assertions: 10 | 8 passed | 2 failed
=== T6g rc=42   (the same two lines; nullptr == 0x000000014e828a00)   assertions: 11 | 9 passed | 2 failed
=== T6h rc=0    All tests passed (18 assertions in 1 test case)       <- GREEN BEFORE, as ruled
=== T6i rc=42
test_show_model.cpp:769: FAILED:  REQUIRE( c.getNumRetiredDecks() == 1 ) with expansion: 0 == 1
=== T6j rc=42   (T6f's two lines; nullptr == 0x000000013f027400)      assertions: 11 | 9 passed | 2 failed
```
(These line numbers are one higher than in the committed file: a stale comment line of the helper was removed
after the RED run. No assertion changed between the RED and the GREEN run.)
GREEN with the fix (15:19:29): T6f 21 assertions, T6g 24, T6h 18 (GREEN AFTER), T6i 17, T6j 22 -- each `All tests
passed`; whole binary `All tests passed (16990 assertions in 43 test cases)`; test_undo_commands `All tests passed
(554 assertions in 79 test cases)` (no existing case edited: the T6 / T7 / M families and test_undo_commands'
AddDeckCmd / InsertDeckCmd cases are untouched and green).

MUTANTS (never committed; in place on DeckCommands.h, restored byte-for-byte: sha256
d4d2f1edfc87f7411d6ce1afba46bec8c0316bcfff3b2a5b23e67a970fb8d9b0 before == after; scripts mutants.sh / mu8.sh /
mutate.py; `test_show_model "T6*"` on build-lane):
- MU6a (AddDeckCmd::undo: retireOrEraseDeck -> decks.erase):
  `T6f ... :651 FAILED: CHECK( c.getNumRetiredDecks() == 1 ) 0 == 1`, `:652 FAILED: REQUIRE( c.playingClip(0) ==
  playing ) nullptr == 0x000000011d821800`; `test cases: 9 | 8 passed | 1 failed`.
- MU6b (InsertDeckCmd::undo: retireOrEraseDeck -> decks.erase): T6g, T6i, T6j FAILED (the same :651 / :652 lines; T6i
  `:768 REQUIRE( c.getNumRetiredDecks() == 1 ) 0 == 1`); `test cases: 9 | 6 passed | 3 failed`. Through the ASan
  script (15:20:23): `probe-asan-unit: ctest -L asan finds 10 asan cases (expected 10)`, `80% tests passed, 2 tests
  failed out of 10` (162 asan:T6g, 163 asan:T6j), `PROBE-ASAN-UNIT RED (2 of 10 failed)`, exit 8. The failures are
  the tests' own REQUIRE (it stops the case before the write through the dead pointer), not an ASan report.
- MU7 (the `addedLayers_.empty()` guard dropped -> `if (true)`): T6h FAILED --
  `:729 CHECK( c.getNumLayers() == 3 ) 5 == 3`, `:731 CHECK( c.getNumRetiredDecks() == 0 ) 1 == 0`,
  `:732 CHECK( c.findDeckById(id) == nullptr )`, `:733 CHECK( c.playingClip(0) == nullptr )`,
  `:734 CHECK( disposed == {900, 911, 940} )`, `:739 CHECK( c.getNumLayers() == 5 ) 7 == 5`,
  `:744 CHECK( disposed.size() == 3 ) 0 == 3`; `test cases: 9 | 8 passed | 1 failed`.
- MU8 (both redos ignore restoreRetiredDeck -> `if (false)`): T6f, T6g, T6j FAILED --
  `:663 CHECK( c.getNumRetiredDecks() == 0 ) 1 == 0`, `:664 REQUIRE( c.playingClip(0) == playing )`
  `nullptr == 0x0000000146020200` (T6f) / `0x000000014602ca00 == 0x0000000146027a00` (T6g, T6j: the snapshot copy
  plays, not the live clip); `test cases: 9 | 6 passed | 3 failed`.
  CAUGHT DURING THE RUN: MU8's FIRST run (mutants.sh) printed MU7's failures -- its build log shows no object
  recompiled (the mutated header's mtime fell in the same second as MU7's object), so it ran MU7's stale binary. That
  run is VOID; mu8.sh re-ran it alone (`MU8 build rc=0 (1 object(s) recompiled)`) with the lines above.
After the restore: `test_show_model "T6*"`: `All tests passed (154 assertions in 9 test cases)`;
`.harmony/probe-asan-unit.sh` (15:21:43): `PROBE-ASAN-UNIT GREEN (10 cases, 0 reports)`, exit 0.

B4f RE-PIN (tests/test_render_thread_lint.cpp): `core/DeckCommands.h` 22 -> 25, every new site inside a command's
runFenced body: AddDeckCmd::undo `retireOrEraseDeck(` replaces `decks.erase(` (+1 -1); AddDeckCmd redo
`restoreRetiredDeck(` (+1); InsertDeckCmd::undo `retireOrEraseDeck(` beside the kept `decks.erase(` of the exception
branch (+1); InsertDeckCmd redo `restoreRetiredDeck(` (+1). The other 12 pins are unchanged (the InspectorRepoint.h
pin of FIX-1 still awaits Harmony). Lint binary: `All tests passed (1026 assertions in 7 test cases)`.

BORIS'S ANSWER (a), verbatim: "Let's not allow control Z to change anything that is live in the layer strip. It
changes anything else". T6h PINS the one place this stage leaves against it (Cmd+Z of a Load Deck that added layers
stops that deck's clip) -- as the ruling's AM-7 and adoption item 8 order (lane BF31 re-registers T6h). Not widened
here.

### Item AM-10 -- the two proofs FIX-2 owes (the six 4.B rows themselves go into FIX-4's final section)
PROOF 1, a positive stopOnLayer test with another deck shown: ABSENT before (D3 fires both routines with deck 0 shown;
D2 fires with deck 1 shown but never calls stopOnLayer) -> ADDED, tests/test_routine_engine.cpp:
"RoutineEngine display D3b: stopOnLayer stops a routine that was fired with another deck shown" -- deck 1 shown at the
fire (`slot(0).deck == 1`), deck 0 shown at the stop, `stopOnLayer(1)` -> idle, no layers, its layer-0 grip released.
It pins behaviour the lane already has, so it is GREEN at once (`"RoutineEngine display D3*"`: `All tests passed (34
assertions in 2 test cases)`); its failing arm is ONE mutant, never committed (mu-am10.sh; the old deck filter put
back: `if (r.deck == 0 && ...)` in RoutineEngine::stopOnLayer; 1 object recompiled):
```
test_routine_engine.cpp:1855: FAILED:  CHECK( rig.slot(0).state == "idle" )   "running" == "idle"
test_routine_engine.cpp:1856: FAILED:  CHECK( rig.slot(0).layers.empty() )
test_routine_engine.cpp:1857: FAILED:  CHECK( rig.fd.count(Ev::Release, opacityKey(0)) == 1 )
test cases:  2 |  0 passed | 2 failed
```
(D3 fails under it too: its routine "b" has no clip target, so its deck is not 0.) Restored: sha256 of
RoutineEngine.cpp equal before / after (07a08f547965ac8cd0779de675642c30a87011d14b8a2df5491571939f4eb7d2); whole
binary after the restore: `All tests passed (1155 assertions in 38 test cases)`.
PROOF 2, `git grep -n -i "corner note\|off-deck" -- docs/claude/recording.md`: NO OUTPUT, rc 1 (0 hits). The
sentences of that file that speak of routines and decks were read against the code (:97 "no pad dims for "another
deck" and the corner never names one"; :99 "Every shared layer a waiting/playing routine drives, whatever deck is
shown, carries a band"; :116-118 "`RoutineEngine::stopOnLayer(layer)` stops every running routine touching that
shared layer, whatever deck it fired from; bands show on the shared layer whatever deck is shown"): each matches
RoutineEngine::stopOnLayer (RoutineEngine.cpp:808) and deriveRoutineDeckView (`pad.onShownDeck = true`,
RoutineDeckView.h). NOTHING corrected in recording.md.

### Item AM-18 -- the retiredDeckCount comment (state-r2 NIT 5)
src/api/ApiServer.cpp, COMMENT ONLY (the sentence lives there, not in DeckCommands.h): "REST never reads a retired
deck (...)" now ends "-- except the retired list's size on the next line, read on this http thread while the message
thread may change it (state-r2 NIT 5; the read itself is filed to tsan-r5, ruling-bf9b-merge SF-8)". No code line
changed (`git show --stat`: 1 file, +3 -1, all comment lines).
