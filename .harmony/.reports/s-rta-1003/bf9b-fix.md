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

## STAGE FIX-4 PROBES, GATES, DOCS (AM-6's script, AM-9, AM-15, the rest of AM-14; H-7; adoption items 3, 9-11 probe half)
STATUS: DONE_WITH_CONCERNS (every item built and run; 10 stop items for Harmony, the first one an ASan report in main's REST reader)
Started 2026-10-03 15:57:16 on 8202802 (branch lane/bf9b). No file under src/ changes in this stage. Scratch: <scratch>/bf9b-fix-FIX-4/.
df at start: 294 GiB free on /System/Volumes/Data.

### Items (appended as each lands)

### Item AM-9 lint B4i + MU10 (tests/test_render_thread_lint.cpp, Case 9)
New case "bf9b B4i: no Link identifier in the model, render, core, the deck grid or the switch / trigger handlers":
`LinkSync|linkSync_|AUDIODNA_HAS_LINK` has zero hits in src/model, src/render, src/core (recursive, code lines),
src/ui/DeckView.cpp and in the bodies of handleDeckSwitch / handleClipTrigger / handleColumnTrigger. No count pin on
MainComponent.cpp.
- GREEN (16:01:16, build-lane): `All tests passed (74 assertions in 1 test case)`.
- RED = MU10 (`(void) linkSync_;` as the first statement of handleDeckSwitch; the lint reads src at run time, no
  build): `test cases:  1 |  0 passed | 1 failed` / `assertions: 74 | 73 passed | 1 failed` (the failing CHECK_FALSE
  prints handleDeckSwitch's body). Restored with cp -p: sha256 a6addb03b949... before == after;
  `git diff --quiet -- src rc 0`; the case green again.
- ctest count: +1 (1248 -> 1249).

### Item AM-6 the live call-chain row: .harmony/probe-asan-live.sh, facts FM-1 and FM-3
ASAN-FH = build-asan-app (NEW build dir, configured like build-asan: -DADNA_SANITIZE=address RelWithDebInfo,
TEST_SERVER ON, SYPHON ON, FetchContent sources from the main checkout's build/_deps; configure rc 0), target
AudioDNA built in the FOREGROUND 15:57:51 -> 15:59:12, rc 0. `otool -L` shows @rpath/libclang_rt.asan_osx_dynamic.dylib.
src == 7decfaa's (`git diff --stat 7decfaa -- src` empty).
Script: the ruling's launch line, steps L0-L6 and four verdict lines. Fixtures come from make-bf9b-check.py into the
run's fresh out dir (absolute media paths). quit_ours only; lock gate; no G-1 pattern hit.
RUNS (raw, <scratch>/bf9b-fix-FIX-4/asan-live/*.log):
1. fh (16:03:30) `PROBE-ASAN-LIVE INVALID (step L5: a deck named "Nine Rows" exists (decks ['Deck 1', 'nine-rows',
   'Deck 1 copy']))` -- MY script's bug: a loaded deck is named after its FILE, and it sits at index 1 as the ruling
   says. Fixed (deck 1, as the ruling writes it).
2. fh2 (16:04:54) `PROBE-ASAN-LIVE RED (step L4)` on the UNMUTATED app -- A REAL REPORT, NOT of the memory fix:
   `==10293==ERROR: AddressSanitizer: container-overflow on address 0x6120002076e8` / `READ of size 4 ... thread T33`
   / `#0 ApiServer::handleComposition(...) ApiServer.cpp:488` ; the region was `allocated by thread T0` in
   `Composition::appendDeck Composition.h:592 <- InsertDeckCmd::execute <- finishStagedLoad <- stageDeckDuplicate`.
   CAUSE (read): GET /api/composition runs on the http thread and walks composition_.decks with no lock; my script
   polled it every 0.2 s while Duplicate Deck's push_back ran on the message thread. The same handler shape is on
   main (`git show main:src/api/ApiServer.cpp` :395). It is the SF-8 class (the http thread reads the model), one
   reader wider. NOT fixed (no src change in FIX-4; STOP ITEM 1). THE PROBE now never polls that reader while a
   command may still change the model: a staged command is awaited on the top text line (/api/debug/ui_text, read on
   the message thread: "Loaded deck:", "Duplicated deck:", "Loaded: bf9b-check"), then the ruling's 1.0 s, health, and
   ONE read (re-read at 1 s steps only when the VALID clause does not hold yet).
3. fh3 (16:06:59) and, after the mutant was reverted and the app rebuilt, fh4 (16:10:20, binary sha256
   16f73991b95f7647):
   `PASS  L0: inspected_layer "Layer 2", inspector_tab "Layer", default show (layers 3, numDecks 1)`
   `PASS  L1: load_deck nine-rows.json -> layers 9, numDecks 2, inspected_layer "Layer 2", decks ['Deck 1', 'nine-rows']`
   `PASS  L2: undo -> layers 3, numDecks 1, inspected_layer "Layer 2"`
   `PASS  L3: redo -> layers 9, numDecks 2, inspected_layer "Layer 2"`
   `PASS  L4: duplicate_deck 0 / undo / redo -> numDecks 3 -> 2 -> 3, decks ['Deck 1', 'nine-rows', 'Deck 1 copy']`
   `PASS  L5: remove_deck 1 (its row-1 clip playing) -> numDecks 2, retiredDeckCount 1, layers[1].activeClip.retired true; undo -> numDecks 3, retiredDeckCount 0, not retired`
   `PASS  L6: load_composition bf9b-check.json -> 20 decks, 3 layers, inspected_layer ""`
   `PROBE-ASAN-LIVE GREEN (7 steps, 0 INVALID, 0 "ERROR: AddressSanitizer", app alive at the end)`  rc 0 (both runs)
4. ASAN-MU3 (16:08:28; the five lines of `undoService_.onLayerStackMoved = ...` deleted, ONE object recompiled
   (`Building CXX object CMakeFiles/AudioDNA.dir/src/MainComponent.cpp.o`), binary sha256 7c504523bfb4a89f; the
   source was put back BEFORE the run -- the mutant lived only in the build dir; no bundle was copied or re-signed):
   `PASS  L0: inspected_layer "Layer 2", inspector_tab "Layer", default show (layers 3, numDecks 1)`
   `RED   L1: an AddressSanitizer report was written; 1 "ERROR: AddressSanitizer" line(s): asan.11495: ==11495==ERROR: AddressSanitizer: heap-use-after-free on address 0x62300002c337 ...`
   `PROBE-ASAN-LIVE RED (step L1)`  rc 1
   After the revert: source sha256 a6addb03b949... (== before), touched, ONE object recompiled, rebuild rc 0,
   `git diff --quiet -- src rc 0` (tests/ then still carried this stage's uncommitted B4i case; the src+tests line
   is printed again at the end of the stage).
FM-1 (VERIFIED by the runs): the ASan app links, starts in --test-mode under the lock, answers /api/health, and on a
  report ENDS (abort_on_error=0, log_path=<out>/asan -> asan.<pid>) with no crash dialog: UserNotificationCenter
  windows 0 before and >= 16 s after every one of the five runs; Output-named windows 0.
FM-3 (VERIFIED): ASAN-MU3 is RED at step L1, the pre-registered step.
NOT REACHED by the row (as the ruling says): the Clip inspector, Layer > Add / Remove Layer. NOTE: the merged tree has
POST /api/debug/inspect_clip (the ui lane's route), so "no REST route selects a cell" is no longer true; the row was
built as ruled (STOP ITEM 2 asks whether a Clip-inspector step should be added).

### Item AM-9 probe-boxes verdict names + the row-count pin; adoption items 9-11, the PROBE half (e0f9321)
CHANGED ROWS (each changed ONLY the named clause; Boris's sentence is at the row):
| probe | row | clause removed | what stays |
|---|---|---|---|
| probe-boxes.py | k7_old_show | "/api/debug/ui_text load_notice is non-empty" | first deck's settings win; exactly one "old show converted:" logLine |
| probe-boxes.py | k7_old_show save (B5) | "the save retires the load notice" | the saved file's new shape |
| probe-boxes.py | k7_old_show reload (B5) | "load_notice empty" | no new logLine; the first deck's settings come back |
| probe-boxes.py | k7_old_show new-format | "load_notice empty" | no new logLine |
| probe-deck-tabs.sh | Phase 2 state remove_hint:0 | the capture was named 11-remove-hint (the button) -> 11-after-remove | its REST clause (decks B,C, active 0). NOT RUN: Phase 2 needs the temporary hook build that is never committed |
Quote at the k7 rows -- Boris: "We don't need any text indicating what has happened or what has happened. That is
something that happens online and is not necessary in this application. It is extra overhead and bloat. Please remove
it cleanly and completely." At probe-deck-tabs -- Boris: "I don't wanna see an under removed button at all. We just
use control Z."
NOT CHANGED, read: probe-boxes k9a / k9b / k9c read no hint and no sentence (REST state + frames only);
probe-ui-files-rename's "undo" field is the Undo HISTORY (top / index / size), not the button.
Also (AM-18, gates-r2 NIT 5): the Boris quote as a comment at k1b_duplicate and at k7_save_reload.
VERDICT NAMES + PIN: the .py prints `PY-ROWS registered <n>` and `PY-BLOCKED-ROWS [<names>]`; the .sh puts the names
in the verdict line and, on a FULL run, fails unless `--- <row>` headers == registered == EXPECTED_ROWS (25).
RED / GREEN:
- changed row on the PRE-bf9b main app (16:32:52, rows k7_old_show,k7_old_take; it also recorded the old take):
  `FAIL  k7_old_show: exactly one 'old show converted:' logLine for the load (0)` /
  `FAIL  k7_old_take: the lane changes only the grid -- every capture during the replay within floor (max 63.84, floor 1.50, 17 captures)` /
  `PY 5 PASS / 2 FAIL / 0 BLOCKED (arm STAGE_P)` / `PROBE-BOXES RED` (the row still fails there on a remaining clause).
- the pin's RED arm (16:39:14; a scratch COPY of the probe whose list holds 2 rows, full run, lane app):
  `FAIL  rows run 2, registered 2 != EXPECTED_ROWS 25 (full run)` / `PROBE-BOXES RED`, rc 1.
- FULL run on the lane app (16:34:21 -> 16:38:12, ONE launch, BOXES_OLD_TAKE = the take recorded above):
  `PY-BLOCKED-ROWS [k5_queue_link_on]` / `PY 63 PASS / 0 FAIL / 1 BLOCKED (arm BF9B)` /
  `PASS  rows run 25 == EXPECTED_ROWS 25 (registered 25)` /
  `PROBE-BOXES BLOCKED 1 [k5_queue_link_on] (0 FAIL; 1 pre-registered bar(s) did not run -- not a pass)`  rc 3.
  63 PASS, not M3's 65: the two checks that were ONLY a load_notice clause are gone. k7's logLine on the lane:
  `PASS  k7_old_show: exactly one 'old show converted:' logLine for the load (1)`.
  Frame looked at: k9c_after2.png (one flat green of the ramp video, as the row's t-from-green oracle expects).

### Item (7) m9b_deck_switch_live [H2] teeth (the row has a behavioural RED arm now)
Mutant (never committed; scratch Release dir <scratch>/bf9b-fix-FIX-4/build-mut, outside git): in Renderer's frame,
when the shown deck (the fence token) changes, the MilkDrop source is resized to 64 x 64 (8 lines after
`const auto deckView = activeDeck_.view();`). Row on the mutant app (16:14:34):
`FAIL  m9b_deck_switch_live[H2]: (load_preset, resize, release_gl) across the solid walk (0, 40, 0), the live walk (0, 40, 0) (bar (0, 0, 0) each), the [H3] switch + one beat (1, 2, 0) (bar (1, 0, 0)); every capture at the canvas size: True; CONTROL load_milkdrop_preset + canvas round trip (1, 2, 0), + gl_context_cycle (HTTP 200) (2, 2, 1) (bar >= 1 each): alive`
/ `PROBE-MILKDROP RED` rc 1 ([H1] and [H3] PASS under it: the next frame resizes back). Source restored: sha256
50c4433af9da... before == after, `git diff --quiet -- src rc 0`.

### Item AM-15 .harmony/probe-boxes-perf.sh / .py; the dry pass; fact FM-6 (9e16730)
Driver: definitions as AM-15 (run value, SD of run means, pooled SD, THR); B6(i) / (ii) / (ii-b) / (iii); --selftest;
--dry; `PROBE-BOXES-PERF BLOCKED (machine not quiet: <ps line>)` rc 3. It imports probe-boxes.py for the fixtures
(K2v's 60 videos, K8's 20 picture decks), so the fixture code is the K rows' own.
- selftest RED (before any number was recorded): `SELFTEST FAIL (no recorded numbers in this script)` rc 1.
- DRY PASS (16:22:43 -> 16:26:05, lane app, quiet lock, PERF_PARTS=ii PERF_RUNS=5 PERF_SECS=12; no verdict):
  `B6(ii) DRY (no verdict) -- mean(20 decks) 2.403 ms - mean(1 deck) 2.387 ms = +0.016 ms; SD 0.052 / 0.098, pooled 0.078, THR 1.000 ms; widest arm spread (max - min) 0.272 ms; run means 20: [2.432, 2.367, 2.421, 2.46, 2.333] 1: [2.541, 2.376, 2.269, 2.368, 2.382]`
  `B6(ii-b) DRY (no verdict) (gpu_time_ms) -- mean(20) 2.258 - mean(1) 2.255 = +0.004 ms; THR 1.000 ms`
  `B6(ii-b) DRY (no verdict) (callback cost) -- mean(20) 10.898 - mean(1) 11.326 = -0.428 ms; THR 1.424 ms`
  `PROBE-BOXES-PERF DONE (dry pass: no verdict)`; ps before run 1:
  `47.9 WindowServer | 46.1 00:06 Audio-DNA | 15.8 coreaudiod | 8.9 Firefox GPU Helper`.
  A first dry pass (16:19:34) printed `PROBE-BOXES-PERF BLOCKED (machine not quiet: 51.0 07-03:38:28 .../WindowServer)`
  after 6 runs: WindowServer runs at 47-51 % BECAUSE the app renders, so it is exempt now (said in the docstring).
- selftest GREEN (the dry pass's run means are the recorded numbers):
  `SELFTEST PASS (recorded numbers: B6(ii) PASS; shifted by 2 ms: B6(ii) FAIL; expected PASS and FAIL)` rc 0.
- FM-6 (16:27:07 -> 16:31:49; mutant = a 2 ms busy loop per deck right before compositor_.compositeShow, built into
  the scratch dir, 1 object, never committed; quiet lock; PERF_PARTS=ii, 5 runs per arm, PERF_SECS=20):
  `RUN   ii FH one run 1: n 40 mean frame_time_ms 3.94 gpu_time_ms 1.967 callback 10.734`
  `RUN   ii FH twenty run 1: n 40 mean frame_time_ms 40.988 gpu_time_ms 1.303 callback 41.246`
  `B6(ii) FAIL -- mean(20 decks) 41.014 ms - mean(1 deck) 3.974 ms = +37.040 ms; SD 0.017 / 0.057, pooled 0.042, THR 1.000 ms; widest arm spread (max - min) 0.141 ms; run means 20: [40.988, 41.02, 41.019, 41.011, 41.033] 1: [3.94, 3.959, 4.073, 3.969, 3.932]`
  `B6(ii-b) STOP-FOR-A-LOOK (callback cost) -- mean(20) 41.299 - mean(1) 10.933 = +30.366 ms; THR 1.000 ms`
  ps before the first two runs: `65.3 Audio-DNA | 47.3 WindowServer | 14.6 coreaudiod | 9.8 Firefox GPU Helper` /
  `100.7 Audio-DNA | 17.2 WindowServer | 10.2 coreaudiod | 7.0 Firefox GPU Helper`.
  Source restored: sha256 50c4433af9da... before == after; `git diff --quiet -- src rc 0`. The script's FM6_PROVEN
  is set, so a B6(ii) PASS no longer carries "(metric sensitivity not proven)".
  The mutant's 1-deck arm reads 3.97 ms against the lane's 2.39 ms: one deck's 2 ms loop minus rounding -- the
  metric sees the loop on both arms.

### Item (6) H-7: the archived evidence scripts (05f4c82)
ONE inserted line 2 in each of 14 scripts (`echo "ARCHIVED RECORD (R-N1, s-rta-1003): this script quits Audio-DNA by
name -- never run or source it; use .harmony/probe-quit-ours.sh" >&2; exit 64` -- for the 7 sourced lock.sh helpers
`return 64 2>/dev/null || exit 64`); `git show --shortstat`: 14 files changed, 14 insertions(+). Exercised: a run
script `rc 64`; a sourced helper `source rc 64; quit_app defined: no`.
The by-name grep over .harmony/.reports script files (pattern `tell application "Audio-DNA" to quit|quit app
"Audio-DNA"|osascript.*Audio-DNA|pkill|killall|adna_kill|kill $(adna`), every remaining hit by file:
```
2 hit(s)  s-rta-0926b/render-evidence/run_diag.sh  -> NEUTRALISED (line 2 stops: exit / return 64)
2 hit(s)  s-rta-0926b/routines-followup-evidence/stop-witness.sh  -> NEUTRALISED
2 hit(s)  s-rta-0927/beatclock-evidence/scripts/witness.sh  -> NEUTRALISED
3 hit(s)  s-rta-0927/renderperf-evidence/fix/scripts/capture_race.sh  -> NEUTRALISED
2 hit(s)  s-rta-0927/renderperf-evidence/fix/scripts/lock.sh  -> NEUTRALISED
2 hit(s)  s-rta-0927/renderperf-evidence/scripts/lock.sh  -> NEUTRALISED
9 hit(s)  s-rta-0927/routines-timing-evidence/scripts/probe-routines-timed.sh  -> NEUTRALISED
2 hit(s)  s-rta-0927/routines-timing-evidence/scripts/run-t2.sh  -> NEUTRALISED
2 hit(s)  s-rta-0927/source-defects-evidence/live.sh  -> NEUTRALISED
2 hit(s)  s-rta-0928/gate-scripts/lock.sh  -> NEUTRALISED
2 hit(s)  s-rta-0928b/wf/lock.sh  -> NEUTRALISED
2 hit(s)  s-rta-0929/wf/lock.sh  -> NEUTRALISED
2 hit(s)  s-rta-0929b/wf/lock.sh  -> NEUTRALISED
2 hit(s)  s-rta-0930/wf/lock.sh  -> NEUTRALISED
2 hit(s)  s-rta-1002b/wf/lock.sh  -> only-ours guard: :51 `if [ -z "$ours" ] || [ "$run" != "$ours" ]; then echo "REFUSE quit: ...`
```
H-7 says 15 scripts: M2's list of 15 INCLUDES s-rta-1002b/wf/lock.sh, which H-7 itself exempts -> 14 neutralised +
1 guarded. s-rta-1003/wf/lock.sh is not in this worktree (the main checkout's file; not mine to touch). Other hits
are prose in .md / .log / .txt / lane.js records, not commands.

### Item AM-14 the remaining docs (3f8c3bf)
performance-controls.md: the Remove Deck bullet gains AM-7's rule and its exception (T6h); the resume sentence gains
`(Boris 2026-10-03: "restart" -- changed by the transport lane)`; Guards name T6f-T6j, AS0-AS7 / N1, B4a-B4j, the two
asan probes, the row-count pin and the perf driver; the Link paragraph gains "K5 with Link on has no live driver:
BLOCKED in probe-boxes". pitfalls.md: Pitfall 33 gains AM-14's sentence. integration.md + APP-INVENTORY: ui_text's
`inspected_layer` / `inspected_clip` / `inspector_tab`; test count 1249; the three new probes. No CLAUDE.md line, no
pitfall: `wc -c CLAUDE.md` 24224. Docs grep of gate B4 (`strip badge|source-deck badge|badge, dots|tab dot|shows a
dot` over docs/claude CLAUDE.md APP-INVENTORY tests/CMakeLists.txt): 0 hits; "Undo Remove" in those files: ONE hit,
performance-controls.md:49, the Composition menu's wording `Undo Remove Deck`.

### FINAL GATE TABLE (ruling section 5 ids -> this stage's raw lines; FH = this report's commit, src == 7decfaa)
| id | result at FH | raw line |
|---|---|---|
| G-1 | list for Harmony | the pattern over probe-boxes.sh/.py, probe-milkdrop.sh, probe-ui-files-rename.sh, probe-asan-live.sh, probe-boxes-perf.sh/.py, probe-quit-ours.sh: 2 hits, both probe-quit-ours.sh (:60 quit_ours, :84 ask_ours_to_quit; H-3 / H-8) |
| B1 | rc 0 | `cmake --build build-lane -j8` all targets rc 0 (16:47; 0 objects left to compile) |
| B2 | 1249 / 1249 | `100% tests passed, 0 tests failed out of 1249` (serial, 131.58 s, 16:47:11 -> 16:49:23) = 1248 + B4i |
| B3 | green | `probe-tsan-unit: ctest -L tsan finds 5 [tsan] cases (expected 5)` / `100% tests passed, 0 tests failed out of 5`, rc 0, 0 "WARNING: ThreadSanitizer" |
| B3b | green | `PROBE-ASAN-UNIT GREEN (10 cases, 0 reports)` rc 0 |
| B4 h / i / j | green | tests #1109 B4h, #1111 B4i, #1110 B4j passed in the full ctest; MU10 fails B4i; docs grep 0 hits |
| ASAN-LIVE | GREEN / RED L1 | `PROBE-ASAN-LIVE GREEN (7 steps, 0 INVALID, 0 "ERROR: AddressSanitizer", app alive at the end)` (3 runs; the last at 16:49:37 on the final binary, sha256 9c635c97726e0532) ; ASAN-MU3: `PROBE-ASAN-LIVE RED (step L1)` |
| K | exact line | `PROBE-BOXES BLOCKED 1 [k5_queue_link_on] (0 FAIL; 1 pre-registered bar(s) did not run -- not a pass)` rc 3, `rows run 25 == EXPECTED_ROWS 25` (lane app sha256 87a10c736d13d228). MAIN0's full K batch: NOT re-run here (only its k7 rows) |
| H1 | green | `PASS  m9b_deck_switch_live[H1]` / `[H3]` / `[H2] ... solid walk (0, 0, 0), the live walk (0, 0, 0) ... (1, 0, 0) ... alive` ; `PROBE-MILKDROP GREEN` (full run 16:40:25, every row PASS). MAIN0 `RED-OK` line: M3's, not re-run. [H2] under a mutant: FAIL (above) |
| U1 | green | `PASS  R11 CONTROL duplicate_deck 0 -> 4 tabs, builds +1 (the counter is alive)` / `54 PASS / 0 FAIL` (16:44:23). MAIN0 arm: M3's, not re-run (the probe is unchanged since) |
| B5 | in K | k7_old_show's save / reload clauses PASS in the K run (load_notice clauses removed by adoption item 11) |
| B6 | NOT RUN as a gate | driver + selftest + dry pass + FM-6 only; the 5 x 60 s verdict run with MAIN0 is Harmony's |
| B7 | machine part in B2 | live states and critics: Harmony's. One window capture looked at (ui probe C1): tabs A / B / C and "+", no dot, no badge, no button, no notice |
Windows after every live batch: `audio-dna windows 0, Output-named 0`, `UserNotificationCenter windows (OptionAll): 0`
(each read >= 16 s after the quit). Lock released after every batch. No .venv symlink was created.

### THE SIX 4.B ROWS (AM-10; late rows of plan-bf9b 4.B, with the sentence that licenses each)
1. test_routine_engine, D3's "another deck" step removed -- plan-bf9b :332-333 "`RoutineEngine::stopOnLayer(int
   layer)` (was (deck, layer)): stops every running routine touching that shared layer, whatever deck it fired from."
   (positive test D3b added in FIX-2, RED under its mutant.)
2. test_routine_deck_view, the "off-deck" case -- :334-335 "deriveRoutineDeckView: bands for every running routine on
   its shared layer, whatever deck is shown (shownDeck stays for labels only)."
3. test_composition, duplicateDeck -- the queued-trigger half: :269 "there is no tuple to clear any more"; the layer-id
   half: :441-442 "a deck has no layers any more". (The ":1607" reference is struck.)
4. test_undo_commands, the "stale DECK index" sub-steps -- :271-272 "TriggerClipCmd: addressed by shared layer index".
5. test_layer_state_key, case 1 -- :441-442 (as 3); the two-half key stays pinned (Pitfall 35).
6. test_recorder_host / test_program_preamble -- :340-341 "PerfStateCapture captures the shared layers once and, per
   deck, only clip runtime."
FIX-2's two proofs are in its section (D3b; the recording.md grep: 0 hits, nothing to correct).

### FILED (ruling section 8 + what the fix stages found; for Harmony to schedule)
SF-1 DONE in this lane (adoption item 2, FIX-1: the owned-or-clear check after every fenced edit, case AS7).
SF-2 grip lifetime against widget lifetime (strip / inspector slider destroyed or re-pointed mid-drag; a routine's
     grip after Move Layer; the grip rule extended to effect rows) -- needs a TSan case.
SF-3 the rebuildCells split (a switch between decks of different widths rebuilds strips and tabs); G1' -> identity.
SF-4 MOOT: the Undo Remove button is gone (adoption item 9).
SF-5 Undo of a Load Deck that added layers still erases a playing deck (T6h) -> lane BF31 ("Undo never changes what
     is live", adoption item 8) re-registers T6h and the trigger-undo tests.
SF-6 K5 with Link on needs a Link build + a toggle route; k5_queue_tempo_feed with it (sync lanes).
SF-7 stable Layer storage (the root cure of the whole class).
SF-8 ApiServer reads retiredDecks_.size() on the http thread -- AND, NEW from this stage (SF-12 below).
SF-9 Composition::crossfaderBlendMode has no reader. SF-10 the Clip tab after a deck switch: Boris answered "yes it
     stays in clip tab regardless of deck" -- nothing to build.
SF-11 (H-11) after Add / Remove Column the Clip tab goes empty when its clip moved in memory (ui lane).
SF-12 NEW: GET /api/composition (ApiServer::handleComposition, the http thread) walks composition_.decks with no
     lock while the message thread appends a deck: `AddressSanitizer: container-overflow ... ApiServer.cpp:488`
     (run fh2). Same handler shape on main. Any REST client that polls during a load / duplicate / remove can meet
     it; in a Release build it is a silent torn read.
ALSO: transport lane (Boris "restart": K10 (ii) re-registered); Composition::routineLoadNote has no reader left
(FIX-3 stop 6: the empty pads are said nowhere, not even in the log -- src was frozen in FIX-4, so no logLine was
added); the event-text inventory of FIX-3's FOUND 1 (main's texts; probe-asan-live now WAITS on three of them, see
stop item 4); BORIS_DECISIONS.md:358 stale badge sentence; B4f's pin { "ui/InspectorRepoint.h", 1 } (H-10: confirmed).

### FACTS MEASURED (FIX-4)
FM-1 VERIFIED -- the ASan app links (libclang_rt.asan_osx_dynamic.dylib), starts in --test-mode under the lock,
     answers /api/health, and ends on a report with no crash dialog (abort_on_error=0, log_path): UNC windows 0.
FM-3 VERIFIED -- `PROBE-ASAN-LIVE RED (step L1)` on ASAN-MU3 (heap-use-after-free, asan.11495).
FM-6 VERIFIED -- `B6(ii) FAIL -- mean(20 decks) 41.014 ms - mean(1 deck) 3.974 ms = +37.040 ms ... THR 1.000 ms`.
FM-7 fell away (adoption item 9). FM-8 is Harmony's (B8's diff).

### STOP ITEMS FOR HARMONY (FIX-4)
1. SF-12: the unmutated ASan app printed `PROBE-ASAN-LIVE RED (step L4)` ONCE (run fh2), a container-overflow in
   GET /api/composition's reader racing Duplicate Deck. It is NOT the lane's memory fix and it is on main too, but it
   IS an ASan report on the final src. I changed the PROBE (it no longer polls that reader while a command may still
   change the model) and 3 runs after that are GREEN; I did not change src. Harmony rules: accept the probe's wait,
   or fix the reader (a message-thread hop like ui_text) in this lane.
2. AM-6 says "no REST route selects a cell"; the merged tree has POST /api/debug/inspect_clip. The row was built as
   ruled (it does not reach the Clip inspector). A Clip-inspector step is possible now -- not built, no ruling.
3. H-7's "15 scripts" = 14 neutralised + s-rta-1002b/wf/lock.sh (guarded, exempt by H-7's own text).
4. probe-asan-live waits for a staged command on the top text line ("Loaded deck:", "Duplicated deck:", "Loaded:"),
   three of main's event texts. If a later lane removes them (Boris's sentence, app-wide), the waits time out
   (15 s / 40 s) and the probe falls back to 1 s re-reads: slower, and it would poll the SF-12 reader again.
5. B6(ii)'s order of tests: I read section 5 as FAIL when the difference exceeds THR even on a noisy arm, INFO-NOISY
   only where a PASS would otherwise be printed. The quiet rule's numbers are mine (a build tool running, or any
   process but Audio-DNA and WindowServer at >= 50 % CPU): the ruling gives none.
6. FM-6 ran with 20 s runs (5 per arm), not 60 s: +37 ms against a 1 ms threshold does not need the length; the
   gate run keeps 60 s.
7. The K line was produced before this report's commits; src, tests and the probe files are byte-identical at the
   final head (`git diff --quiet 7decfaa HEAD -- src` rc 0; the final build compiled 0 objects; same binary).
8. MAIN0 arms NOT re-run in this stage: the full K batch, m9b's RED-OK, U1's three RED rows (M3 has them; the ui and
   milkdrop probes are unchanged since M3; probe-boxes' changed row was run on main: RED above).
9. probe-deck-tabs Phase 2 was not run (it needs a temporary hook build); only its capture name changed.
10. Timeline: one cooldown message says "re-acquire cooldown"; no lock was ever taken from another owner.

### HAND-OVER TO HARMONY
- Final head: this report's commit on lane/bf9b (git -C <WT> log -1). src == 7decfaa; tests == 7decfaa + B4i.
- WT = /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/bf9b. Build dirs: build-lane = the Release lane
  app (build-lane/AudioDNA_artefacts/Release/Audio-DNA.app, sha256 87a10c736d13d228) + every test; build-asan-app =
  ASAN-FH (build-asan-app/AudioDNA_artefacts/RelWithDebInfo/Audio-DNA.app, sha256 9c635c97726e0532; app target
  only); build-asan = the asan unit gate; build-tsan = the tsan unit gate. The scratch mutant dir
  (<scratch>/bf9b-fix-FIX-4/build-mut) holds the FM-6 MUTANT app: never use it as an arm; it can be deleted.
- Commands (lock helper: LANE=<name> . <scratch>/lib/lock.sh; acquire_lock / acquire_quiet_lock ... release_lock):
  B1  cmake --build WT/build-lane -j8
  B2  until mkdir /tmp/audiodna-ctest.lock 2>/dev/null; do sleep 15; done; ctest --test-dir WT/build-lane
      --output-on-failure; rm -rf /tmp/audiodna-ctest.lock
  B3  bash WT/.harmony/probe-tsan-unit.sh          B3b  bash WT/.harmony/probe-asan-unit.sh
  ASAN-LIVE  ASAN_LIVE_APP=WT/build-asan-app/AudioDNA_artefacts/RelWithDebInfo/Audio-DNA.app bash
      WT/.harmony/probe-asan-live.sh <out-base>     (ASAN-MU3: delete the 5 lines of `undoService_.onLayerStackMoved
      = ...` in src/MainComponent.cpp, cmake --build WT/build-asan-app --target AudioDNA, put the source back, touch
      it, run, rebuild; <scratch>/bf9b-fix-FIX-4/mu3_build.sh does exactly this)
  K   MAIN0 first: BOXES_APP=<MAIN0> bash WT/.harmony/probe-boxes.sh <out-base> (its k7_old_take prints "recorded
      take <folder>"); then BOXES_OLD_TAKE=<folder> BOXES_APP=WT/build-lane/AudioDNA_artefacts/Release/Audio-DNA.app
      bash WT/.harmony/probe-boxes.sh <out-base>     (NO row list: the pin needs a full run; ~4 min)
  H1  MILKDROP_APP=<lane app> bash WT/.harmony/probe-milkdrop.sh <out-base>; MAIN0: MILKDROP_MODE=pre
      MILKDROP_APP=<MAIN0> bash WT/.harmony/probe-milkdrop.sh <out-base> m9b_deck_switch_live
  U1  UIFR_APP=<app> bash WT/.harmony/probe-ui-files-rename.sh <out-dir> --shots
  B6  bash WT/.harmony/probe-boxes-perf.sh --selftest; then under acquire_quiet_lock, no commit meanwhile:
      PERF_FH_APP=<lane app> PERF_MAIN_APP=<MAIN0> bash WT/.harmony/probe-boxes-perf.sh <out-base>   (~30 min)
- Evidence: <scratch>/bf9b-fix-FIX-4/{asan-live,boxes,md,ui,perf}/*.log, ctest.log, tsan-unit.log, asan-unit.log.

### Notes for .harmony/notebook.md (Harmony appends)
- GET /api/composition is served on the http thread and reads the deck list unlocked: a probe must not poll it while
  a staged load / duplicate may still land (ASan: container-overflow). Wait on /api/debug/ui_text (message thread),
  then read once. | discovered: .harmony/probe-asan-live.sh, src/api/ApiServer.cpp handleComposition
- A deck loaded with Load Deck is named after its FILE ("nine-rows"), not the "name" in the file; Duplicate Deck
  appends the copy at the END. | discovered: .harmony/probe-asan-live.sh L4 / L5
- WindowServer sits at 47-51 % CPU while Audio-DNA renders: a "machine quiet" rule must exempt it. | discovered:
  .harmony/probe-boxes-perf.py
- A hook that guards whole-tree commits matches the words of a heredoc: a script text holding "commit" and a later
  "-a" flag was blocked. Keep those words out of generated script comments. | discovered: this stage
- An ASan Audio-DNA app builds in ~80 s on this machine (RelWithDebInfo, -j6) and starts as fast as Release.

INBOX-RECHECK: none (no message channel in this workflow run)

### PACKET QUALITY (FIX-4)
- Clarity: CLEAR, three HAD_TO_INFER: the B6(ii) test order (stop item 5); "15 scripts" against H-7's own exemption
  (stop item 3); whether the K line "on the FINAL head" needs a run after the last docs commit (stop item 7).
- Missing context: that /api/composition is read off the message thread; that a loaded deck takes its file's name;
  that /api/debug/inspect_clip exists after the merge-in.
- Unused context: the three r2 reviews, plan-bf9b-merge's body, rulings-bf9b-merge.md (1002b).
- Self-brief files: ruling-bf9b-merge.md (full), the plan's HARMONY ADOPTION (full), rulings-bf9b-mergein.md,
  bf9b-merge.md's hand-over, this report's FIX-3 end sections, ruling-bf9b's B6 text -- useful, none stale.
  No DEPARTMENT / KNOWLEDGE_TOOLS block: no knowledge tools -- grep-only; nothing judged dead on "no callers".
  pulse.json not read (the rig gives this worktree to this lane alone).

### SLIM CHECK (FIX-4)
src: 0 lines. tests: +47 (one lint case). Probes: probe-asan-live.sh (new), probe-boxes-perf.sh / .py (new),
probe-boxes.py -26 net (four clauses out, names + pin in), probe-boxes.sh +21, probe-deck-tabs.sh +3. Docs: 8 lines
over 4 files. Archived scripts: 14 inserted lines. Not built (as ruled): k9d, k5_queue_tempo_feed, B7 state 9 / 3b,
any CLAUDE.md line, a Clip-inspector step, an src fix for SF-12. What I would cut if asked: probe-boxes-perf's
PERF_PARTS switch (kept: FM-6 and the dry pass need part ii alone). Smell named: probe-boxes-perf.py imports
probe-boxes.py by path and sets sys.argv for it (inappropriate intimacy) -- cheaper than a second copy of the
fixtures, and the K rows' fixture code is then the perf rows' own.

Ended 2026-10-03 16:52:39.
