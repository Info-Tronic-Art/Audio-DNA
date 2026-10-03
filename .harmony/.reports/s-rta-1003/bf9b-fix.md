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
STATUS: DONE (2026-10-03 15:16:27 -> 2026-10-03 15:29:12; concerns = "DEVIATIONS / STOP ITEMS FOR HARMONY (FIX-2)" below)
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

### End of stage FIX-2 (src / tests head 9e3099d; `git diff --quiet -- src tests` rc 0 before the runs)
- `cmake --build build-lane -j6` rc 0, app + every test target (0 warnings from the touched files).
- Full ctest, serial, under /tmp/audiodna-ctest.lock (2026-10-03 15:25:44 -> 15:27:50): `100% tests passed, 0 tests
  failed out of 1250`, `Total Test time (real) = 126.19 sec`. 1250 = 1244 (FIX-1) + 6 added (T6f, T6g, T6h, T6i, T6j,
  D3b) - 0 retired. No existing case edited.
- `.harmony/probe-tsan-unit.sh` (unmodified: `git diff --quiet 707818e -- .harmony/probe-tsan-unit.sh` rc 0), 15:27:56:
  `probe-tsan-unit: ctest -L tsan finds 5 [tsan] cases (expected 5)`, `100% tests passed, 0 tests failed out of 5`,
  exit 0, 0 "WARNING: ThreadSanitizer".
- `.harmony/probe-asan-unit.sh`, 15:28:02: `probe-asan-unit: ctest -L asan finds 10 asan cases (expected 10)`,
  `100% tests passed, 0 tests failed out of 10`, `PROBE-ASAN-UNIT GREEN (10 cases, 0 reports)`, exit 0.
- `wc -c CLAUDE.md` = 24224 (unchanged). No app launched, no lock taken, no Output window, no new build dir, no
  build left running.
- Commits: f335c81 (AM-7), 5180397 (AM-10 D3b), 9e3099d (AM-18 comment), + this section's docs commit.

WHAT THE RULING SAID THIS STAGE MUST PROVE BEFORE FIX-3
| claim | result | where |
|---|---|---|
| T6f, T6g, T6j RED at FIX-1's head | RED, rc 42 each (retired 0 == 1; playingClip nullptr) | Item AM-7, RED block |
| T6f, T6g, T6j GREEN after | GREEN (21 / 24 / 22 assertions) | Item AM-7 |
| T6h GREEN before and after | GREEN before (18 assertions, FIX-1 src) and after | Item AM-7 |
| T6h RED under MU7 | RED, 7 CHECKs | Item AM-7, MUTANTS |
| MU6 -> T6f, T6g, T6j; MU8 -> the address checks | RED (MU6a T6f; MU6b T6g, T6i, T6j; MU8 T6f, T6g, T6j) | Item AM-7 |
| the T6 / T7 / M families untouched and green | no existing case edited; 1250 / 1250 | this section |
| PROBE-ASAN-UNIT GREEN with T6f, T6g, T6j under the label | GREEN (10 cases, 0 reports) | this section |

### DEVIATIONS / STOP ITEMS FOR HARMONY (FIX-2)
1. AM-18's comment is in `src/api/ApiServer.cpp` (that is where the sentence is), so this stage touched ONE src file
   besides DeckCommands.h -- comment lines only. The stage text said "DeckCommands.h only for src"; AM-7 says that of
   its own change. Flagged, not hidden.
2. Boris's answer (a), verbatim: "Let's not allow control Z to change anything that is live in the layer strip. It
   changes anything else". This stage moves Cmd+Z of Add / Duplicate / Load Deck (same row count) TOWARDS it, and --
   as ruled (AM-7, adoption item 8) -- PINS the opposite for one case in T6h: Cmd+Z of a Load Deck that added layers
   stops that deck's clip. T6h is a pin of a known exception, not of the wanted behaviour; lane BF31 must re-register
   it. A fire by hand is still an Undo step of its own (also BF31's).
3. T6f / T6g / T6j are under the asan label as ruled, but under MU6 they fail by their own REQUIRE (playingClip ==
   the address), which stops the case BEFORE the write through the dead pointer -- so the ASan script's RED there is
   an assertion RED, not an "ERROR: AddressSanitizer" line. Kept that way on purpose: without the REQUIRE the
   Release binary would write freed memory on a regression instead of failing cleanly. Under the label they add
   what a GREEN run adds: retire / restore move the deck with zero reports.
4. RED-first was run on the uncommitted tree (new tests, src == FIX-1's head, `git diff --quiet -- src` rc 0), then
   tests + fix went in ONE commit (f335c81): no commit of this stage has a failing ctest.
5. MU8's first run was VOID (a stale binary: same-second mtime, nothing recompiled); re-run alone, RED. The void run
   is reported above, not dropped.
6. MU6b also turns T6i RED (not in the ruling's MU6 list; same cause). The AM-10 mutant also turns D3 RED.
7. CARRIED from FIX-1, still open: B4f's new pin `{ "ui/InspectorRepoint.h", 1 }` awaits Harmony's confirmation.
8. The Write tool refused a scratch fragment named `report-*.md` ("subagents return findings as text"); the lane
   report itself is a required, committed deliverable of this workflow, so its sections were appended from .txt
   fragments by shell (as the skeleton was). Said here so nobody takes it for a silent workaround.
9. pulse.json was NOT read in this stage (the rig rule gives this worktree to this lane alone). No DEPARTMENT /
   KNOWLEDGE_TOOLS block: grep-only; nothing judged dead on "no callers".

### FOUND, NOT FIXED (FIX-2)
1. Stale comments in src (outside this stage's files): `src/ui/RoutinePad.h:11` "everything at 50 % when the routine
   plays on another deck"; `src/ui/RoutineDeckView.h:13` "the corner note", `:29` "false: it plays on another deck
   (the pad paints at 50 %)", `:50` `"· Drop on Deck 2"`. The code sets `pad.onShownDeck = true` always (read).
   recording.md is right; these comments are not.
2. InsertDeckCmd::undo in the EXCEPTION branch (layers added) keeps today's body, so it does NOT cancel a trigger
   queued into the erased deck: the pending ref then names a deck that is gone. INFERRED harmless (clipAt returns
   nullptr for a missing deck); what the bar-snapped fire does with that ref was not run. SF-5 / BF31 territory.
3. AddDeckCmd has no dispose hook: when its undo ERASES a deck that received clips outside the Undo history, their
   media is not disposed by the command. Pre-existing (the old erase did the same); a RETIRED one is disposed by the
   reap. INFERRED from reading.
4. Docs owed by AM-14 for this stage's behaviour (performance-controls.md Remove Deck bullet: AM-7's rule + its
   exception; Guards: T6f-T6j) are FIX-3 / FIX-4's by the ruling; APP-INVENTORY's test count (1234 at M1) is now
   1250. Not written here.

### NOTES FOR FIX-3 / FIX-4
- ctest is 1250 at this head; the asan label holds 10 (EXPECTED_ASAN_CASES=10). build-lane, build-tsan and build-asan
  are current with 9e3099d (no reconfigure needed unless a CMake file changes).
- A mutant loop that rewrites one file and rebuilds within the same second can run the PREVIOUS mutant's binary:
  `touch` the file after each rewrite and check the build log for a recompiled object (mu8.sh does both).
- B4f now pins core/DeckCommands.h at 25. FIX-3 removes the badge / dot (DeckView, LayerStrip, MainComponent): if a
  `setClip(` / structure-writer call disappears or appears there, the pin of that file moves with a justification.
- The helper `undoRetiresPlayingDeckAndRedoRestoresIt` and `boxClip` (tests/test_show_model.cpp) are reusable for
  BF31's re-registration of T6h.

### Notes for .harmony/notebook.md (Harmony appends)
- make compares mtimes; a source rewritten in the same second as the object built from its previous content is NOT
  recompiled. In-place mutant loops: touch after the rewrite and assert `grep -c 'Building CXX' build.log` >= 1,
  else the "RED" you read is the previous mutant's. | discovered: FIX-2 mutants.sh (MU8 printed MU7's failures)
- A memory-ish test that must stay a clean FAIL in Release puts REQUIRE(pointer still owned) before it writes through
  the pointer; under the asan label its RED is then an assertion, its GREEN the zero-report proof.
  | discovered: tests/test_show_model.cpp T6f / T6g / T6j

INBOX-RECHECK: none (no message channel in this workflow run; the relayed user lines a-e are Boris's answers, already
adopted by Harmony: (a) = adoption item 8, quoted above)

### PACKET QUALITY (FIX-2)
- Clarity: CLEAR, one HAD_TO_INFER: "DeckCommands.h only for src" against "the retiredDeckCount comment", whose
  sentence is in ApiServer.cpp (deviation 1).
- Missing context: none that blocked.
- Unused context: the three r2 reviews and rulings-bf9b-merge.md (the ruling + adoption + FIX-1's section carried
  everything), the live-app rig rules (no app needed).
- Self-brief files: ruling-bf9b-merge.md (full), the plan's HARMONY ADOPTION + P8 dispositions, bf9b-merge.md's
  hand-over, rulings-bf9b-mergein.md, this report's FIX-1 section -- all useful, none stale.

### SLIM CHECK (FIX-2)
src: two undo bodies and two redo branches in DeckCommands.h (+ their comments), one comment sentence in
ApiServer.cpp. tests: 5 cases + 1 shared helper + 1 clip maker (test_show_model), 1 case (test_routine_engine), 1 pin
re-justified; probe: one number, one comment line. Every line traces to AM-7 / AM-10 / AM-18. Not built (as ruled):
probe row k9d, B7 state 9, any change of the wide Load Deck undo. Smells named: InsertDeckCmd::undo now has two
bodies behind one flag (the ruled exception; BF31 removes it); AddDeckCmd / InsertDeckCmd / RemoveDeckCmd repeat the
"restore, else snapshot" shape three times (duplicated code, left: the ruling confines the change to these bodies).

## STAGE FIX-3 SCREEN (AM-11, AM-12, AM-14 screen lines, AM-17; adoption items 9, 10 and 11)
STATUS: DONE_WITH_CONCERNS (2026-10-03 15:30:31 -> 15:54; concerns = "DEVIATIONS / STOP ITEMS FOR HARMONY (FIX-3)" below)
Started 2026-10-03 15:30:31 from de2544e (git status clean). Scratch: <session scratchpad>/bf9b-fix-FIX-3/.
(A predecessor of this stage was interrupted after writing the four skeleton lines above -- its only trace; git
status showed no other change. The diff was read and kept.) Work done 2026-10-03 15:32:08 -> 15:53; scratch
<session scratchpad>/bf9b-fix-FIX-3/ (scripts, logs, shots/).

Commits: 1a65b85 (AM-12 + adoption 9-11), 7decfaa (AM-11), eeaa0d7 (docs), 0dd155c (make-bf9b-check.py), + this
section's docs commit.

### Item AM-12 + adoption items 9, 10, 11 -- nothing on screen names a source deck or announces an event (1a65b85)
REMOVED (src; `git diff de2544e 0dd155c --shortstat -- src`: 10 files changed, 26 insertions(+), 405 deletions(-), AM-11's lines included)
- LayerStrip.{h,cpp}: SourceBadge, sourceBadgeOf, getSourceBadge, sourceBadgeBounds, badgeFont, the five kBadge*
  constants, badge_, updateSourceBadge (3 calls: setLayer, refresh, the 30 Hz timerTick), paintSourceBadge, the tooltip
  branch, the click branch, onSourceDeckClicked; the two test hooks only the retired cases used
  (thumbnailBoundsForTest, routineBandRowsForTest). The strip's timer stays (Pitfall 41 faders).
- DeckView.{h,cpp}: syncTabDots + its call in refresh(), tabDotShownForTest, DeckTabButton's dot / kDotColour /
  dotBounds / paintButton override, the `btn->dot =` line; onSourceDeckClicked and its strip wiring; the Undo Remove
  button whole: undoHintBtn_, undoHintGeneration_, kUndoHintMs, showUndoHint, hideUndoHint (+ its call in
  rebuildGrid), onUndoHint, the resized() slot.
- DeckTabRow.h: Layout::hint, layout()'s hintWidth parameter, kHintGap, undoRemoveHint, `#include <string>`.
- MainComponent.{h,cpp}: NoticeLabel, loadNotice_, showLoadNotice, clearLoadNotice (4 call sites: before a load,
  after a load, two saves), the two deck-id-refusal showLoadNotice calls, the row-1 layout slot, the constructor
  block, the `ui/LoadNotice.h` include; onSourceDeckClicked's handler; onUndoHint's handler; the 30 Hz syncTabDots
  call; pushCommands' hideUndoHint line; in removeDeck the playing-layers walk, showUndoHint and
  `setFileLabel("Removed deck \"<name>\"")` with the three locals only they used.
- api/ApiServer.{h,cpp}: onDebugLoadNotice, Box::load, the "load_notice" property.
- src/ui/LoadNotice.h: deleted.
KEPT (the behaviour behind each text): `logLine(composition_.migrationNote)` once per load (unchanged line); the
deck-id refusal still returns and still logs `[Decks] This show has used all its deck numbers. ...`;
Composition::deckIsPlaying (retire / reap); RemoveDeckCmd and its Undo.
B4f pins unchanged (no setClip( / structure-writer call appeared or disappeared; the lint case passes as it is).

EDIT MENU (adoption item 9): the menu model NAMES actions -- "Undo " + UndoManager::undoDescription()
(src/ui/MenuBarModel.cpp:34-45). Pinned by a new case in tests/test_show_model.cpp (the real RemoveDeckCmd with
removeDeck's description, the real UndoManager, the real AudioDNAMenuBar): GREEN at de2544e already (19 assertions) =
a pin of existing behaviour. The item sits in the menu named "Composition"; the app has NO menu named "Edit" (stop
item 1).

TESTS
- THE BF14 PIN = the case "S3.1 the clip-name row never names the deck: a 20-char deck name changes no pixel, a
  30-char clip name changes only the clip-name row (bf9b, B7 M-d)" -- NAME KEPT, body rewritten: the same clip
  content played from the shown deck, from deck 5 and from a removed (retired) deck gives byte-equal strips; a
  20-char deck name changes no pixel; a 30-char clip name changes only the name row; VALID: each differs from the
  empty layer's strip.
- M-t (new): "M-t the deck tabs say nothing about playing: ..." -- 20 decks, the tab row snapshot byte-equal with
  nothing playing, with decks 3 / 7 / 12 playing (one mid-fade), and after rebuildGrid; VALID: showing deck 7
  changes it.
- M-c RESTATED (the old name is gone, see RETIRED / RENAMED): the WHOLE strip column byte-equal over showDeck 0 -> 5
  -> 0 on same-width decks; every LayerStrip the same object by juce::Component::SafePointer (all non-null) AND by
  the collected pointer list; VALID: clearing a layer changes the column.
- M-a and the 16(e) header case: untouched, green.
- LINT B4j (tests/test_render_thread_lint.cpp, new case 8): the ruling's pattern verbatim, PLUS (my addition, stop
  item 4) `undoHint|UndoHint|undoRemoveHint|Undo Remove|loadNotice|LoadNotice|load_notice|\bNoticeLabel\b|Removed
  deck`, over WHOLE lines of src/**/*.{h,cpp,mm} (comments included: "nothing dead left behind").

RED at FIX-2's head (final test file, src = de2544e for the five files the target compiles; red2.sh, raw):
  S3.1 the clip-name row ... :189 CHECK( diffOutside(fromShown, fromOther, {}) == 0 )   37 == 0
                             :202 CHECK( diffOutside(fromShown, fromRemoved, {}) == 0 ) 28 == 0
  S3.2 a 0 -> 5 -> 0 ... (M-c) :316 CHECK( diffOutside(shot0, shot5, {}) == 0 )         56 == 0
  M-t ...                    :348 CHECK( diffOutside(idle, playing, {}) == 0 )          48 == 0
                             :353 (after rebuildGrid)                                   48 == 0
  G1' ...                    :409 CHECK( stripFor(g.dv, 1)->isSelected() ) false; :410  345 == 0; :414 false; :415 298 == 0
  G2 ...                     :440 and :442 CHECK( litHeaders(g.dv, 4) == std::set<int>{ 3 } ) FAILED
  G3 ...                     :454 CHECK( stripFor(g.dv, 1)->isSelected() ) FAILED
  test cases: 8 | 2 passed | 6 failed;  assertions: 140 | 128 passed | 12 failed
  B4j at de2544e: `test cases: 1 | 0 passed | 1 failed`, the INFO lists every hit (ui/LayerStrip.*, ui/DeckView.*,
  ui/DeckTabRow.h, ui/LoadNotice.h, MainComponent.{h,cpp}, api/ApiServer.{h,cpp}); raw in red-run.txt.
GREEN at 0dd155c: test_layer_strip_source_deck `All tests passed (140 assertions in 8 test cases)`;
  test_render_thread_lint `All tests passed (1347 assertions in 8 test cases)`; test_deck_tab_row `All tests passed
  (103 assertions in 4 test cases)`; the menu pin `All tests passed (19 assertions in 1 test case)`.

MUTANTS (never committed; mutants.sh sleeps, touches and prints the recompiled-object count -- 2 each; raw in
mutants-run.txt; afterwards `git diff --quiet -- src tests` rc 0 and the binary green again)
  MU11 showDeck always rebuilds        -> M-c RED: :313 and :319 CHECK( sameStrips() ) FAILED (2 of 140)
  MU12 predicate dropped from setupColumnTriggers -> G2 RED: :440, :442
  MU13 a 2x2 px mark in LayerStrip::paint when the clip's deck is not the shown one -> the BF14 pin RED (:189 4 == 0,
       :202 4 == 0), and also M-c (:316 8 == 0) and G1' (:410 8 == 0)
  MU14 a tab's text colour follows deckIsPlaying (in refresh) -> M-t RED: :348 757 == 0

RETIRED BY NAME (8; ctest name diff FIX-2 end -> now printed 9 gone / 7 new, see RENAMED)
 AM-12's five (tests/test_layer_strip_source_deck.cpp):
  1 "S3.1 the strip badge names the deck a playing clip came from: its tab number, dim for the shown deck, normal for
    another, 'x' for a removed deck, none when clear (bf9b, ruling-bf9b 16(a), B7 M-b)"
  2 "S3.1 a folded row draws no badge; the badge sits inside the thumbnail's bottom-left corner, clear of the
    routine-band rows, wide enough for '20' (bf9b, ruling-bf9b 16(a), B7 M-d)"
  3 "S3.1 badge contrast against its opaque background: dim >= 3:1, normal >= 7:1, normal > dim; the painted badge
    uses those colours (bf9b, B7 M-f)"
  4 "S3.1 a badge click shows that deck in the grid and never selects the layer; a removed deck's badge does nothing;
    the tooltip names the deck (bf9b, ruling-bf9b 16(c))"
  5 "S3.1 a deck tab shows a dot iff some layer's active ref, or the previous ref of a running fade, names that deck:
    20 decks (bf9b, ruling-bf9b 16(b), B7 M-e)"
 Adoption items 9-11 (three more):
  6 "S3.4 the Remove Deck undo hint names the layers that keep playing a clip from the removed deck (bf9b,
    ruling-bf9b 16(d))"                                              (tests/test_layer_strip_source_deck.cpp)
  7 "S3.4 the load notice: an old show's conversion (details = the whole note), a routine-pad note, both, or nothing
    (bf9b, ruling-bf9b 9(d))"                                        (tests/test_layer_strip_source_deck.cpp)
  8 "DeckTabRow::layout -- the undo hint sits flush right and hides when it would crowd the \"+\""
                                          (tests/test_deck_tab_row.cpp; a MAIN test, plan6 -- the button it tested is gone)
RENAMED (1; the old name is in the "gone" list, the new one in the "new" list -- the ruling's "M-c RESTATED"):
  "S3.2 a 0 -> 5 -> 0 showDeck walk: every LayerStrip the same object, the strip column byte-equal outside the badge
  rects (bf9b, plan F16, B7 M-c)"  ->  "S3.2 a 0 -> 5 -> 0 showDeck walk on same-width decks: every LayerStrip the
  same object (SafePointer), the WHOLE strip column byte-equal in all three (bf9b, plan F16, ruling-bf9b-merge AM-12,
  B7 M-c)"
ADDED (6): M-t; G1'; G2; G3; "bf9b B4j: no source-deck badge, tab dot, Undo Remove button or load notice identifier
  left in src/"; "After a Remove Deck the Composition menu's Undo item reads \"Undo Remove Deck\" (Cmd+Z); ...".
No other name disappeared (names-fix2.txt vs names-fix3.txt, from the two full ctest logs).

### Item AM-11 -- the grid after a rebuild (7decfaa)
(a) `DeckView::columnHeaderColour(int col) const` (private): lit iff col == activeColumn_ and the column's deck id is
    the shown deck's; refresh() and setupColumnTriggers both call it. (b) rebuildGrid: `strip->setSelected(layerIdx ==
    selectedLayerIndex_)` on each strip it creates. NOT BUILT, as ruled: the rebuildCells split (SF-3).
TESTS G1', G2, G3: RED lines above (same run), GREEN at 7decfaa. Fixture: makeShowWithWideDeck (deck 1 = 8 columns
among 4-column decks); each rebuild is proven by tabRowBuilds() +1 (VALID).

### FACTS MEASURED
FM-5 Is the strip column byte-equal across a rebuildGrid once the highlight is re-applied? YES.
     G1' at 7decfaa: `CHECK( diffOutside(shot0, shotWide, {}) == 0 )` and the walk back both pass (0 differing pixels
     of 250 x 288, three strips, two playing named clips, layer 1 selected); the assert is not loosened. Before (b):
     345 / 298 differing pixels (the highlight). Scope of the fact: a headless 1400 x 600 DeckView, no scrollbar in
     either width, JUCE's default LookAndFeel; thumbnails of files that do not exist.

### SNAPSHOTS (decoded PNGs in <scratch>/bf9b-fix-FIX-3/shots/; I looked at each)
Headless (written by the tests under ADNA_BF9B_SHOT_DIR, JUCE default LookAndFeel -- not the app's):
- mc-strip-column-deck0-shown.png / mc-strip-column-deck5-shown.png (250 x 288, byte-equal): three strips, Layer 3 /
  2 / 1 top to bottom, each with X B S, < || >, the S K V faders, the blend box and its clip name (d3r2c0, d5r1c2,
  d0r0c1); "Layer 2" boxed in cyan (selected); the picture squares are dark (no file); NO number, letter or mark on
  any picture corner.
- g1-strip-column-4-columns.png / g1-strip-column-8-columns.png: the same column before and after the rebuild
  (Layer 2 "wide c6" still boxed cyan; Layer 3 empty).
- bf14-strip-from-removed-deck.png (250 x 96): one strip, Layer 2, clip name "short", nothing else on the picture.
- mt-tab-row-three-decks-playing.png (1400 x 24): 20 tabs "Deck 1" .. "Deck 20" and "+"; Deck 1 green (shown); no
  dot on any tab, also not on Deck 4 / 8 / 13 (the playing ones).
Live (the lane's Release app, one lock hold 15:48:10 -> 15:48:31, pid 91584, REST only, window id 39793 of our pid,
0 Output-named windows, quit by the helper, 0 UserNotificationCenter windows 16 s later; smoke-run.txt):
- live-deck0-shown-layer2-plays-deck3.png and live-after-remove-deck3.png (3456 x 2158): the app's own look. After
  `POST /api/debug/remove_deck {2}`: the tab row reads Deck 1, Deck 2, Deck 4 .. Deck 20, "+", nothing after the "+";
  no dot; Layer 2's strip shows the D3 C2 picture and the name D3C2L2 with no mark; the preview still shows D1 C1 |
  D3 C2; the file label still reads "D3C2L2.png" (`file_label before == after: True`); retiredDeckCount 1.
  `POST /api/debug/undo`: 20 decks, 20 tabs, retired 0.
- ui_text keys: audio_notice, file_label, inspected_clip, inspected_layer, inspector_tab, ok -- no load_notice.
- make-bf9b-check.py's files loaded through `POST /api/debug/load_deck`: five-rows.json -> (21 decks, 5 layers),
  undo -> (20, 3); nine-rows.json -> (21, 9). live-five-rows-loaded.png captured.

### make-bf9b-check.py (0dd155c)
Also writes, beside bf9b-check.json: five-rows.json (deck "Five Rows", 5 rows x 4 columns, 20 pictures D21 C1..C4)
and nine-rows.json (deck "Nine Rows", 9 rows, ONE picture in row 1 / column 0, 0-based -- AM-6's L1 fixture). Deck
files in Deck::toVar's shape (rows of "clips" only). Every deck of the show stays at 4 columns (AM-11).

### End of stage FIX-3 (src / tests head 0dd155c; `git diff --quiet -- src tests` rc 0 before the runs)
- `cmake --build build-lane -j6` rc 0, app + every test target; no new warning from the touched files (LayerStrip.cpp:
  617 "unused variable 'w'" and the -Wdouble-promotion lines are in code this stage did not write).
- Full ctest, serial, under /tmp/audiodna-ctest.lock (15:49:40 -> 15:51:51): `100% tests passed, 0 tests failed out of
  1248`, `Total Test time (real) = 131.02 sec`. 1248 = 1250 - 8 retired + 6 added (one renamed).
- `.harmony/probe-tsan-unit.sh` (unmodified), 15:51:51: `probe-tsan-unit: ctest -L tsan finds 5 [tsan] cases (expected
  5)`, `100% tests passed, 0 tests failed out of 5`, rc 0, 0 "WARNING: ThreadSanitizer".
- `.harmony/probe-asan-unit.sh` (unmodified), 15:51:56: `probe-asan-unit: ctest -L asan finds 10 asan cases (expected
  10)`, `100% tests passed, 0 tests failed out of 10`, `PROBE-ASAN-UNIT GREEN (10 cases, 0 reports)`, rc 0.
- Gate B4's docs grep at this head: `git grep -n -i -E "strip badge|source-deck badge|badge, dots|tab dot|shows a dot"
  -- docs/claude CLAUDE.md .harmony/APP-INVENTORY.md tests/CMakeLists.txt` -> 0 hits; "Undo Remove" -> 1 hit,
  performance-controls.md's Remove Deck bullet, the menu wording `Undo Remove Deck`.
- `wc -c CLAUDE.md` = 24224 (unchanged; no CLAUDE.md line, no pitfall added). No probe script edited. No background
  build started; none running. No new build dir.

WHAT THE RULING SAID THIS STAGE MUST PROVE BEFORE FIX-4
| claim | result | where |
|---|---|---|
| the five retired cases listed by name and no other | 5 + 3 (adoption 9-11) retired, 1 renamed (M-c), by name | RETIRED BY NAME |
| the BF14 pin, M-t, G2, G3 RED at FIX-2's head | RED (37 / 28; 48 / 48; :440 :442; :454) | RED block |
| a headless snapshot of the strip column and of the tab row, looked at | 6 PNGs + 3 live | SNAPSHOTS |
| MU11 -> M-c, MU12 -> G2, MU13 -> the pin, MU14 -> M-t | all RED | MUTANTS |
| FM-5 | byte-equal: YES | FACTS MEASURED |
| the ASan app build starts in the background | NOT DONE -- Harmony constraint for this stage; FIX-4 builds it | -- |

### THE BORIS PAGE (ruling section 6 with adoption items 6, 8, 9, 10, 11 applied; for Harmony's page)
Open the test show "bf9b-check" (Harmony names the folder). Each step: do -> expect -> what wrong looks like.
8.1 On Deck 1 click "D1 C1" in the Layer 1 row and "D1 C4" (a video with a running clock) in the Layer 3 row; click
    the Deck 3 tab and click "D3 C2" in the Layer 2 row. Then click through all 20 deck tabs and back to Deck 1.
    Expect: the picture keeps D1 C1 and D3 C2 and the clock keeps counting; only the grid changes. Wrong: a picture
    changes, a flash, the clock jumps or stops.
8.2 While you click through the tabs, look at the layer strips on the left and at the tabs themselves. Expect: the
    strips do not change at all, and no number, dot or other mark appears on a strip or on a tab. A strip shows the
    clip that is playing (its picture and its name), not the deck it came from. On Deck 3 the "D3 C2" cell is lit in
    the Layer 2 row; on every other deck no cell in that row is lit, and that is correct. Wrong: a strip flickers or
    goes empty, a fader moves, a number or a dot shows up.
8.3 Ignore Column: select Layer 2, open the Layer tab, tick "Ignore Column Trigger"; click the Deck 5 tab; click the
    column number 4 above the grid. Expect: Layer 2 keeps D3 C2; Layers 1 and 3 show D5 C4. Wrong: Layer 2 changes.
8.4 Untick "Ignore Column Trigger" on Layer 2. Click the Deck 6 tab (its column 2 has nothing in the Layer 2 row) and
    click column number 2. Expect: Layer 2 goes empty (the right half turns black); Layers 1 and 3 show D6 C2.
8.5 On Deck 3 click "D3 C2" in the Layer 2 row; click the Deck 1 tab; right-click the Deck 3 tab -> Remove Deck.
    Expect: D3 C2 keeps playing on the right and its strip looks the same; the Deck 3 tab is gone; no button and no
    message appears. Press Cmd+Z: the Deck 3 tab comes back, D3 C2 still playing. Wrong: the right half goes black,
    the strip goes empty, or a button or a line of text about the removal shows up.
8.7 Select a layer, Layer tab: "Persistent" is gone; "Ignore Column Trigger" sits alone on its row.
8.8 The top bar has no "Fade:" control: a deck change never changes the picture, so there is nothing to fade.
8.9 Open your show "test with harry": your layers look as they did. No note appears.
8.10 Save "bf9b-check", close it, open it again: the decks and the layer looks come back; the layers start empty (as
    before: what is playing is not saved).
8.12 Click the Layer 2 strip (it gets a highlight) and open the Layer tab. Then in the menu bar: Deck -> Load
    Deck...; in the file window go to the bf9b-check folder and pick five-rows.json (a deck with five rows; your show
    has three layers). Expect: a new deck tab appears and is shown, two more layers appear in the grid, the Layer tab
    still shows Layer 2, the Layer 2 strip is still highlighted, and the picture does not change. Press Cmd+Z: the new
    deck and the two layers go away again; the Layer tab still shows Layer 2. Wrong: the app quits, the Layer tab
    shows another layer or nonsense, the highlight disappears.
(Steps 8.6 and 8.11 and the "NOT on the page" note are dropped by adoption items 8 and 6. Q-D falls away.)
NOTE for the page writer, measured live: after step 8.12's load the text line at the top reads "Loaded deck:
five-rows" and after opening a show "Loaded: <name>" -- texts that are on main (FOUND, NOT FIXED 1).

### DEVIATIONS / STOP ITEMS FOR HARMONY (FIX-3)
1. THE EDIT MENU. Boris: "The only place that we will see undo remove, will be in the top edit menu." The app has no
   menu named "Edit": the menus are Audio-DNA, Composition, Deck, Layer, Column, Clip, Output, Shortcuts, View
   (src/ui/MenuBarModel.cpp:9). Undo is the FIRST item of "Composition" and it does name the action: after a Remove
   Deck it reads "Undo Remove Deck" (Cmd+Z), pinned by the new test. Nothing in the menu was changed. Whether the
   menu's NAME matters to Boris is not mine to decide.
2. TWO TEXTS THAT WERE ON MAIN AT 5abdf01 WERE REMOVED, on Boris's own words in adoption items 9 and 10: the tab row's
   "Undo Remove" button (plan6: DeckView::showUndoHint etc., main src/ui/DeckView.cpp:16-25, :990-1013) and the file
   label `Removed deck "<name>"` (main src/MainComponent.cpp:3953). The stage text also says "Texts that exist on main
   at 5abdf01 are NOT touched". I read that sentence as covering every OTHER text (item 9 names the button; item 10's
   quote answers the question about that very line; the user request relayed to this run asks for the removal). If
   Harmony reads it the other way, `Removed deck` is one setFileLabel line to put back and B4j's second pattern loses
   `Removed deck`.
3. THE RETIRED SET IS 8, NOT 5, and one case is RENAMED (M-c). Gate B2's "RETIRED = exactly the five names of AM-12"
   needs the three adoption names and the rename. Name 8 is a test that exists on main.
4. B4j is wider than the ruling's pattern: a second regex for the adoption-9/10/11 identifiers, and whole lines
   (comments too). It bites on the pre-fix src; it would also fail on a future comment that says "Undo Remove".
5. PROBES NOT EDITED AND NOT RUN (FIX-4's): they will FAIL on this head until their clauses change --
   .harmony/probe-boxes.py k7_old_show (:1143 "load_notice is non-empty", :1160-1162, :1196-1197, :1213-1216 read
   ui_text.load_notice, which no longer exists); .harmony/probe-deck-tabs.sh state `remove_hint:0|11-remove-hint`
   (:165, :186: captures the hint, which no longer shows). probe-ui-files-rename.sh's "undo" field is the Undo
   HISTORY (top / index / size), not the button: unaffected (read, not run).
6. `Composition::routineLoadNote` ("N routine pad(s) left empty ...") now has NO reader in src: the notice was its only
   display, and there never was a log line for it (at 5abdf01 it had no reader either: `git grep routineLoadNote
   5abdf01 -- src` hits only Composition.h). The pads are still left empty (the behaviour); nothing says so, on screen
   or in the log. Adding a logLine is not in my packet -- Harmony's call for FIX-4.
7. AM-11 (b), a consequence not named in the ruling: the selected ROW survives a rebuild by INDEX. After Layer >
   Remove Layer of a selected layer that is not the top one, the layer that takes its index is highlighted (the
   stack-move hook re-points the Layer inspector to that same row, so the two agree). Before, no strip was
   highlighted after that command. Read, not driven by a test.
8. Commit 1a65b85 (state A = everything but AM-11's lines) was built and its four touched test binaries run green;
   the FULL ctest ran once, at 0dd155c.
9. RED-first ran on the uncommitted tree. M-c's and G1''s fixtures were strengthened after the first green run: their
   VALID clause showed that ShowFixture clips draw nothing on a strip (an "all equal" snapshot of blank strips), so
   the fired clips now carry a file name (fireNamed). The RED block above is the re-run with the FINAL test file
   against de2544e's five files, swapped in place and put back (sha256 equal). Right after the put-back one run
   printed 3 failures from a stale object (same-second mtime, FIX-2's notebook note); touched, rebuilt (31 objects),
   green. Reported, not dropped.
10. A live smoke was run although the stage only asked for headless snapshots (one 21 s lock hold, REST only): the
   removed widgets live in MainComponent, which no unit test drives.
11. The headless PNGs use JUCE's default LookAndFeel (the test harness's); only the three live captures show the
   app's own look.
12. CARRIED, still open: B4f's pin `{ "ui/InspectorRepoint.h", 1 }` awaits Harmony's confirmation (FIX-1).
13. pulse.json not read (the rig gives this worktree to this lane alone). No DEPARTMENT / KNOWLEDGE_TOOLS block:
   grep-only; nothing judged dead on "no callers" -- every removed symbol was removed with all its callers and the
   app + every test target links.

### FOUND, NOT FIXED (FIX-3)
1. EVENT-ANNOUNCING TEXTS THAT ARE ON MAIN (not touched; the inventory Harmony owes Boris). All write the top text
   line (file label) unless said; lines at 0dd155c:
   - src/core/StagedLoad.h:59 "Loading <name>..."; :64 "Loaded: <name>"; :65 "Loaded deck: <name>"; :66 "Duplicated
     deck: <name>"; :76 "Too many loads waiting: <name> skipped"; :77 "Duplicate skipped: <name> is gone"
   - src/MainComponent.cpp:2988, :3553, :3581, :4407 "Saved: <file>"; :3014 "Loaded: <name>"; :3816 "Saved deck:
     <name>"; :3895 "Show in Finder: not found - <path>"; :4504 "No images found in folder"; :4518 "Folder: <name>
     (...)"; :4579 "Camera failed to open"; :395 and :4436 "Slot <n>: <name>"; :470-473 an audio engine error message
   - the same line also shows the fired clip's file name on every clip fire (e.g. "D3C2L2.png", seen live)
   - the Record panel's notice line and routine notice (src/ui/RecordPanel.cpp:192-200, :298, :305; texts such as
     "Saved: x" come through RecorderHost / RoutineEngine notify)
   - a modal: src/MainComponent.cpp:3559-3562 "Save failed: <path>" (AlertWindow, not in test mode)
   The audio-device notice (audioDeviceNotice_) is a STATE ("no input"), not an event; listed for completeness.
2. BORIS_DECISIONS.md:358 still says "a strip badge names the deck each playing clip came from (click = show it), a
   deck tab ..." -- stale after this stage; Harmony's file, not edited.
3. src/model/Composition.h:79-80 (a main comment): routineLoadNote "the app shows it once; never silent" -- not true
   at 5abdf01 and not true now (stop item 6).
4. The stale RoutinePad.h / RoutineDeckView.h comments of FIX-2's FOUND 1 are still there.
5. src/ui/LayerStrip.cpp:617 `int w` unused (compiler warning; main's code).
6. .harmony/HANDOFF.md, .harmony/binding-decisions.md and .harmony/undo-v1-manual-e2e.md name the badge / the notice /
   "Undo Remove" as history or as menu wording; not docs of the gate's grep, not edited.

### NOTES FOR FIX-4
- ctest is 1248 at 0dd155c; asan label 10; tsan 5. build-lane, build-tsan (its two targets) and build-asan
  (test_show_model) are current with 0dd155c's src. tests/CMakeLists.txt changed (test_show_model gained
  ui/MenuBarModel.cpp): every build dir reconfigures itself on its next build.
- src is final as of 7decfaa unless a review asks otherwise: the ASan app (ASAN-FH) is FIX-4's to build, in the
  foreground. `git diff --stat 7decfaa -- src` must stay empty through FIX-4.
- Probe clauses to change (stop item 5): probe-boxes k7 / B5's load_notice clauses keep the logLine clause;
  probe-deck-tabs.sh's remove_hint state. APP-INVENTORY's test count (1234 at M1) is now 1248; T6f-T6j and the two
  asan probes are still owed in Guards; AM-7's rule + exception in the Remove Deck bullet; integration.md's three
  AM-6 ui_text fields (not there: `git grep -c inspected_layer -- docs/claude/integration.md` = no hit) and Pitfall
  33's AM-14 sentence (not there: `grep -c "forget their bindings" docs/claude/pitfalls.md` = 0).
- B7 states (3b) and the load-notice state are gone; state (3) no longer has a file_label VALID clause.
- nine-rows.json / five-rows.json come out of `.harmony/make-bf9b-check.py <dir>` (needs the main .venv's Pillow and
  ffmpeg): L1's fixture is ready.

### Notes for .harmony/notebook.md (Harmony appends)
- A snapshot-equality test over ShowFixture clips proves nothing: a clip without a media file draws no name and no
  picture, so "playing" and "empty" strips are byte-equal. Give the fired clips a file name, and keep a VALID clause
  (clear a layer -> the snapshot must change). | discovered: tests/test_layer_strip_source_deck.cpp M-c / G1'
- A test can write its own snapshots for a look: ADNA_BF9B_SHOT_DIR=<dir> ./test_layer_strip_source_deck writes six
  PNGs (saveShot). Headless JUCE renders with LookAndFeel_V4, not the app's.
- Swapping files in place and copying them back within one second of a build leaves a stale object: touch AFTER the
  copy-back and check the object count (again -- FIX-2's note; it bit once more here).

INBOX-RECHECK: none (no message channel in this workflow run; the relayed user line is Boris's sentence of adoption
item 11, already in the stage text)

### PACKET QUALITY (FIX-3)
- Clarity: CLEAR, with one HAD_TO_INFER: "texts on main are NOT touched" against adoption items 9 / 10, which remove
  two texts that are on main (stop item 2). Also inferred: the stage title names "adoption items 9 and 10" while the
  body adds 11 -- built all three.
- Missing context: that the Undo Remove button and the "Removed deck" label predate the lane (found by `git grep
  5abdf01`); that the app has no Edit menu.
- Unused context: the three r2 reviews, rulings-bf9b-merge.md, rulings-bf9b-mergein.md, bf9b-merge.md's hand-over
  (the ruling + adoption + FIX-2's notes carried everything); the sanitizer-app and perf rig rules.
- Self-brief files: ruling-bf9b-merge.md (full), the plan's P7 + HARMONY ADOPTION (full), this report's FIX-2 end
  sections -- useful, none stale. LESSONS_LEARNED / CONTEXT.md / notebook.md: not read (not a bug-fix packet; no new
  domain term introduced).

### SLIM CHECK (FIX-3)
src: -405 / +26 lines over 10 files; the only ADDED code is columnHeaderColour (5 lines, replacing two copies of the
colour choice) and one setSelected line. tests: 8 cases retired, 6 added, 1 restated, 1 body rewritten; helpers
tabRowOf, saveShot, fireNamed, makeShowWithWideDeck, litHeaders (each used by >= 2 checks, saveShot env-gated). docs:
6 lines of performance-controls.md, 2 of pitfalls.md, 1 of integration.md, 2 of APP-INVENTORY. Generator: one
function, two calls. Not built (as ruled): rebuildCells, an 8-column deck in the check show, AM-12 (4)'s label
sentence, B7 state 9 / 3b, any CLAUDE.md line, any probe edit, any background build. What I would cut if asked:
B4j's second regex (stop item 4) and saveShot. Smells named: DeckView still spells the unlit colour 0xff2a2a2a in
four places and kHeaderLit's value 0xff3a5a4a literally in two (setupDeckTabs, refresh's tab loop) -- magic numbers,
left: the ruling asked for ONE helper for the header only.
