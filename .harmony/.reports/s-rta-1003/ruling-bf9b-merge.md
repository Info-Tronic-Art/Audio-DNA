# ruling-bf9b-merge -- ARCHITECT RULING on the blind council's attacks on plan-bf9b-merge.md (s-rta-1003, 2026-10-03)
Author: architect (opus, read-only). Plan: .harmony/.reports/s-rta-1003/plan-bf9b-merge.md. Seat papers, verbatim:
.harmony/.reports/s-rta-1003/attack-bf9b-merge-papers.md (4 seats, 34 attacks). Lane read ONLY at the pin a7491d4
(git show / git grep); main as `git show main:<path>`. Nothing was built, run or launched. Labels: VERIFIED = I read it
at the pin (file:line); INFERRED = reasoned from what I read; ASSUMED = not checked. Line numbers are the pin's and
shift after the merge-in: builders resolve by symbol. Boris is quoted only verbatim; Harmony's requirements are
labelled "Harmony constraint:".
NAMES: AM-n = an amendment of this ruling. AS0..AS6, N1 = the memory test cases (the plan called them A1..A8).
ASAN-LIVE = the live call-chain row (the plan's "A2-live").

## 0 VERDICT
The plan NEEDS REVISION and is buildable today with the 18 amendments below. 34 attacks: 21 ACCEPT, 10 PARTIAL,
3 REJECT. The plan's direction (fix the freed-memory read, the Undo gap, the untested wiring line, the badge) stands;
its size does not. What changes:
1. P1 MEMORY. The fix moves INSIDE LayerInspector::setLayer / ClipInspector::setClip: they drop their scalar bindings
   first, so no re-point can read the pointer it leaves, from any caller. This is the plan's own runner-up (F1-B),
   taken by the plan's own fallback clause, because the council found one more site (ME-1). The stack-move hook becomes
   ONE named function the tests call; it also clears a Clip inspector whose clip the model no longer owns. The strip
   "forget" and the grip rule are withdrawn (no finding asked for them; they are filed).
2. THE LIVE ASAN ROW stays a gate (Harmony constraint: a full-tier gate exercises the real call chain) but is rebuilt:
   it starts from the default show, checks before each step that the Layer tab is bound, needs ONE ASan app, and its
   failing arm is the wiring-line mutant on that app.
3. P2 UNDO. Undo of Add / Duplicate / Load Deck retires a still-playing deck only when the command added no layers.
   A Load Deck that added layers keeps today's behaviour as a documented, pinned exception. No live row (the proposed
   driver was wrong: a REST fire IS an Undo step).
4. P7 SCREEN. The strip badge AND the tab dot are removed (Boris: "drop"). Nothing shows which deck a clip came from.
   What remains is ruled in AM-12.
5. P4. K5 with Link on stays BLOCKED and is reported as not run; it is never called covered.
6. Harmony constraint: K10 and the C3 resume contract stay as built in this lane; the Boris page drops step 8.11.

## 1 FACTS RE-DERIVED (at a7491d4 unless "main:" is written)
R1  The hook. UndoService::withDeckDetached records layers.data() / size() before the edit and calls
    onLayerStackMoved after the fence when either changed (src/core/UndoService.cpp:61-70). It is wired to
    repointLayerInspector (src/MainComponent.cpp:1800), which re-points the Layer inspector by the DeckView's
    selected layer row (:5304-5320; resolveLayer = the bounds-checked Composition::getLayer, UndoService.h:61-64).
    VERIFIED.
R2  The read. LayerInspector::setLayer (src/ui/LayerInspector.cpp:728-741) = layer_ = layer; setEffects;
    syncFromLayer() BEFORE bindScalarControls(). syncScalar -> setParamValue (:903-909) -> routineHandHolds
    (UniversalParamControl.cpp:355-359) reads the OLD conn_; bindConnection (:126-129) reads it and releases ANY
    active grip (any kind, any rank). ClipInspector::setClip (ClipInspector.cpp:788-808) has the same shape for its 6
    scalar controls; its source-param controls and both effect stacks already forget first (ClipInspector.cpp:801-803,
    :837-848; EffectStackView.cpp:256-271). VERIFIED.
R3  The existing fix case (tests/test_show_model.cpp:1462-1531) installs its own lambda, but that lambda calls the
    REAL LayerInspector::setLayer, and the case already forces the move (shrink_to_fit, REQUIRE(capacity() < 5),
    REQUIRE(&c.layers[1] != before)). So a fix INSIDE setLayer turns this very case clean with no test edit; a fix at
    the call site (plan F1-C) leaves it reporting until the test is rewritten. VERIFIED by reading; the ASan report
    itself is the reviewer's run (state-r2 :49-51), not mine.
R4  ME-1's path. Layer > Remove Layer (MainComponent.cpp, case kLayerRemove, :6731-6750) -> RemoveLayerCmd ->
    Composition::eraseLayer (src/model/Composition.h:519-533): layers[index] and row `index` of every live and
    retired deck are erased, so that row's clip buffer -- which holds the Clip objects themselves -- is freed
    (ClipRow.h:13). The handler then calls only rebuildGrid; pushCommands (:5220-5238) re-points nothing; the hook
    re-points only the Layer inspector. InspectorPanel::tickModulation ticks the Clip inspector unconditionally
    (InspectorPanel.cpp:170-177); ClipInspector::tickModulation (:900-904) ticks its effect stack and reads clip_. A
    cell select binds it (MainComponent.cpp:700-708). So: Clip inspector on a clip of the LAST layer, then Remove
    Layer = a dangling Clip* read at timer rate. main has the same handler (main: src/MainComponent.cpp:6853).
    VERIFIED by reading; not run. A layer INSERT does not kill clips (the rows move, each keeps its buffer) --
    INFERRED (ClipRow.h:13, Composition.h:480-494, gates-r2 :8).
R5  Kin the hook cannot see. Layer > Clear Clips (case kLayerClearClips, :6751-6797: `row->clips.clear()` in its own
    fence, then refreshPreviewFromShow + rebuildGrid) and Deck > Clear Clips (case kDeckClearClips, :6669-6713) destroy clips without
    moving the stack and touch no inspector. Same shape on main (main: src/MainComponent.cpp:6873-6909,
    `layer->clips.clear()`). VERIFIED by reading; that ASan reports it is INFERRED (the row buffer stays allocated, so
    it needs the dead clip to have owned a heap block: an effect, a source param).
R6  Grips. A slider drag = Held rank 3 on mouse-down, released on mouse-up through conn_ (UniversalParamControl.cpp:
    27-28) or through layer_ (LayerStrip.cpp:446-451). Held "never expires on its own" (ParamConnection.h:105-109).
    A Decaying touch expires after gripHoldMs = 250 ms (Composition.h:138). A move keeps a grip, a copy clears it
    (ParamConnection.h:142-168). LayerStrip has NO destructor (git grep `~LayerStrip`: no hit): a strip destroyed
    mid-drag never releases -- on every rebuildGrid, today, on main as well. VERIFIED. No test at the pin asserts a
    release on an inspector rebind (git grep over tests/: no hit) -- INFERRED from the grep.
R7  Routine keys. resolveLayer (src/recording/Program.cpp:49-61) returns key.layer whenever that index is in range
    (a name mismatch is only "PositionOnly"); the name is used only when the index is out of range.
    Composition::moveLayer keeps size and data() (Composition.h:537-557), so the hook does not fire. After Move Layer a
    routine's release resolves the OLD index. VERIFIED by reading (pre-existing).
R8  A REST / OSC fire IS an Undo step. apiServer_->onTriggerClip = handleClipTrigger(layer, column)
    (MainComponent.cpp:1877) with the default Origin::Human (MainComponent.h:542); the push is gated on Human and
    "the runtime changed" (MainComponent.cpp:4873-4880). A queued fire changes the pending ref, so it pushes too
    (INFERRED). Fires that push no Undo step: Origin::Replay only (take replay, routines: ApiServer.cpp:289-295,
    :339-344) and the GL autopilot. VERIFIED.
R9  Deck commands. AddDeckCmd::undo (src/core/DeckCommands.h:708-719) and InsertDeckCmd::undo (:815-833) erase;
    InsertDeckCmd::undo erases the deck, THEN the layers it added, then disposes every snapshot cell. RemoveDeckCmd
    (:892-942) = cancelPendingInto + retireOrEraseDeck (dispose only an erased deck) / restoreRetiredDeck, else the
    snapshot. VERIFIED.
R10 Model swap and lever. refreshUiAfterModelSwap (MainComponent.cpp:2946-2987) = setClip(nullptr);
    setLayer(nullptr) AFTER the swap, then clearSelection, selectLayer(-1), rebuildGrid. The lever (:2330-2344) runs
    once at startup, calls onLayerSelected(n) and never DeckView::selectLayer (selectedLayerIndex_ starts -1,
    DeckView.h:199). Composition::initDefault gives 3 layers by push_back (Composition.h:199-212).
    /api/debug/ui_text returns only file_label, audio_notice, load_notice (src/api/ApiServer.cpp:2086-2090).
    LayerInspector::tickModulation only null-tests layer_; the read of the layer is LayerInspector::refresh ->
    syncFromLayer, which InspectorPanel::refresh runs for the ACTIVE tab at ~10 Hz (LayerInspector.cpp:781-788,
    InspectorPanel.cpp:179-187, MainComponent.cpp:4240-4242). VERIFIED.
R11 The grid. rebuildGrid (src/ui/DeckView.cpp:114-262) = hideUndoHint (:117), every strip / cell / header / tab
    recreated, bands re-fanned, selection pruned (:252-257). It never re-applies the selected-layer highlight
    (selectedLayerIndex_ is written only in selectLayer, :475-479) nor the lit header (setupColumnTriggers :427-450
    makes every header unlit; refresh :296-303 colours them). showDeck (:349-365) rebuilds when the shape differs,
    else refresh. The Move Layer handlers re-apply the highlight by hand (cases kLayerMoveUp / kLayerMoveDown,
    MainComponent.cpp:6844, :6865); Add Layer, Remove Layer and Load Deck do not. VERIFIED.
R12 The undo hint. kUndoHintMs = 10000 (DeckView.h:190). showUndoHint has ONE caller, removeDeck
    (MainComponent.cpp:3885). DeckTabRow::layout (src/ui/DeckTabRow.h:22-36) draws the hint only when
    rowWidth - hintWidth >= plus.x + 24 + 8; DeckView::resized hides it otherwise (DeckView.cpp:95-102). Once the tabs
    shrink to fill the row, plus.x + 24 is within numDecks px of rowWidth, so the hint needs every tab at 100 px:
    rowWidth >= n x 102 + 32 + hintWidth. For 19 tabs that is about 2350 px with a ~380 px hint (hint width INFERRED,
    not measured); the app opens at 1280 x 800 (MainComponent.cpp:2329). removeDeck also writes the file label
    `Removed deck "<name>"` (:3886), which /api/debug/ui_text returns as file_label. The lane's own capture of state
    (3) was a 4-deck fixture (lane report :1326, "numDecks 3"). VERIFIED (arithmetic re-derived from the function).
R13 The tab dot at the pin: DeckView::syncTabDots (DeckView.cpp:323, :328-340); DeckView.h:53-61 (declaration,
    tabDotShownForTest); DeckTabButton::dot / kDotColour / dotBounds / the paint branch (DeckView.h:163-176);
    DeckView.cpp:530 (`btn->dot = ...`); MainComponent.cpp:4277 (the 30 Hz call); the test case at
    tests/test_layer_strip_source_deck.cpp:372 (the only test file that names the dot or the badge). Composition::
    deckIsPlaying (Composition.h:608-618) is ALSO the retire / reap predicate (:628, :657): it stays. VERIFIED.
R14 The badge at the pin: the plan's F12 list holds (git grep). Test cases by line in
    tests/test_layer_strip_source_deck.cpp: :169 (M-b), :221 (M-d geometry), :250 (name row), :282 (M-f), :331 (badge
    click), :372 (M-e), :414 (M-a), :453 (header), :477 (M-c), :520 (hint text), :530 (load notice). M-c compares raw
    pointers (`CHECK(now == before)`, :504, :515). Text naming the badge or the dot: docs/claude/
    performance-controls.md:44, :50, :65; docs/claude/pitfalls.md:139; tests/CMakeLists.txt:3453-3454;
    .harmony/APP-INVENTORY.md:59. CLAUDE.md: no hit. VERIFIED.
R15 Sanitizers. ADNA_TSAN_TEST_PROPERTIES pins LABELS / TIMEOUT / ENVIRONMENT / FAIL_REGULAR_EXPRESSION and is
    applied to DEDICATED binaries (tests/CMakeLists.txt:3236-3241, :3274, :3318); probe-tsan-unit.sh exits 3 below
    EXPECTED_TSAN_CASES and otherwise with ctest's code. apply_sanitizers(AudioDNA) is at CMakeLists.txt:614. VERIFIED.
R16 probe-boxes. The .py lists its rows by hand (probe-boxes.py:1518-1529, 24 rows at the pin) and prints
    `PY <p> PASS / <f> FAIL / <b> BLOCKED (arm X)`; the .sh prints only a COUNT of blocked rows (probe-boxes.sh:83-87).
    VERIFIED.
R17 Tempo. REST set_bpm = applyTempoCommand("link", bpm, Human) (MainComponent.cpp:1885-1888). The Link tick sends the
    same command only when linkSync_.isEnabled(), which a default build never is (:4244-4251). VERIFIED.
R18 Perf read-outs. frame_time_ms is an EMA (0.1) of renderStart..renderEnd (src/render/Renderer.cpp:685, :828-834);
    the show autopilot runs before renderStart (:506-520); peak_callback_ms covers the whole callback
    (Renderer.cpp:296-299, Renderer.h:437-440, reset on read); gpu_time_ms exists (Renderer.h:450-452). VERIFIED.
R19 plan-bf9b citations. The duplicateDeck sentence is at plan-bf9b.md:269; "a deck has no layers any more" at
    :441-442; the stopOnLayer sentence at :332-333; the bands sentence at :334-335; the TriggerClipCmd sentence at
    :271-272; the PerfStateCapture sentence at :340-341; the ":1607 InsertDeckCmd" row (:464) is a
    test_undo_commands.cpp row. The 4.B omissions are the lane report's "S2b deviations" item 3 (:941-955). VERIFIED.
R20 probe-milkdrop on main prints per row `PASS  <row>: ...` / `FAIL  <row>: ...`, under MILKDROP_MODE=pre `RED-OK` /
    `VOID`, and ends `PROBE-MILKDROP GREEN` / `PROBE-MILKDROP RED` (main: .harmony/probe-milkdrop.py:112-119; the
    last line of probe-milkdrop.sh). VERIFIED. The m9b row is M3's; I did not read it (the worktree is off limits).
R21 Menus: "Audio-DNA", "Composition", "Deck", "Layer", ... (src/ui/MenuBarModel.cpp:7-9); Load Deck... is in the Deck
    menu (:86); its chooser opens in CompDecksBrowser::getDecksDir() with *.json (MainComponent.cpp:3656-3661).
    VERIFIED.
R22 CLAUDE.md is 24,264 B at the pin and 23,962 B on main (cap 25,000). VERIFIED (wc -c).

## 2 ATTACK RULINGS (34)
| id | sev | verdict | the line that decides | goes to |
|---|---|---|---|---|
| ME-1 | MUST | PARTIAL | TRUE (R4): Remove Layer frees the last row's clips and nothing re-points the Clip inspector. Cure = the hook also clears a Clip inspector whose clip the model no longer owns (not a re-point by coordinate: a Load Deck must not change what the Clip tab shows). Clear Layer / Deck Clips are the same class, pre-existing, not hook-reachable (R5): filed. | AM-2, AM-3, AM-4 (AS3b), SF-1 |
| ME-2 | MUST | ACCEPT | A hook body kept in a MainComponent lambda cannot be driven by a test (plan :241 says so of the old one). The whole body becomes one named function. The grip rule it was aimed at is withdrawn. | AM-2, AM-8 |
| ME-3 | SHOULD | PARTIAL | TRUE (EffectStackView.cpp:256-271 forgets the rows; a held effect slider would stay Held). The rule it extends is withdrawn, so there is nothing to extend; the class is filed whole. | AM-3, SF-2 |
| ME-4 | SHOULD | ACCEPT | TRUE (R7): a routine key is positional; "never stuck" is false after Move Layer. The sentence is struck; the case is filed with a test. | AM-3, SF-2 |
| ME-5 | SHOULD | ACCEPT | Erasing the last Layer frees no heap block (Composition.h:523); the plan's A3 / A4 are functional guards. Memory cases are named and each asserts its storage moved. | AM-4 |
| ME-6 | NIT | ACCEPT | A load is a model swap (R10): the RED would land on the swap, not on Load Deck. The live row no longer loads a show first; the RED step is re-registered. | AM-6 |
| ME-7 | SHOULD | ACCEPT | R8: the REST fire is the Undo step, so the undo after it never reaches the deck command. Every live step gets a VALID clause; the retire / restore run is Remove Deck + Undo. | AM-6 |
| GA-1 | MUST | ACCEPT | R10: after any load the Layer tab is unbound and the selected row is -1, so the steps after it are vacuous and the wiring mutant stays GREEN. | AM-6 |
| GA-2 | MUST | ACCEPT | Same defect as ME-2, same cure. The "endHumanHolds" part falls with the grip rule. | AM-2, AM-3, AM-8 |
| GA-3 | SHOULD | ACCEPT | R15: the tsan precedent pins the environment and the fail regex in CTest; the M3 tree has no asan label, so "the same script" there cannot print a report. | AM-5 |
| GA-4 | SHOULD | ACCEPT | A memory test must prove the storage moved or died (the lane's own case does, R3). | AM-4 |
| GA-5 | SHOULD | ACCEPT | R8 VERIFIED: REST and OSC fires push TriggerClipCmd. k9d as written would FAIL its own VALID clause. No honest REST driver exists short of a take / routine replay; the row is not registered. | AM-7, SF-5 |
| GA-6 | SHOULD | PARTIAL | R18 TRUE. The pre-registered bar stays as written (never loosened); the SD is defined, callback and GPU read-outs are added as "stop for a look", BLOCKED needs a waiver. An injected-cost mutant app is Harmony's call (FM-6). | AM-15 |
| GA-7 | SHOULD | PARTIAL | (a), (c) and the wording ACCEPT (R16). (b) is moot: the tempo-feed row is dropped (SC-5). | AM-9 |
| GA-8 | SHOULD | ACCEPT | R14: a raw pointer compare can pass after destroy-and-recreate. Identity is proven with juce::Component::SafePointer. K8 is not a P6 gate. | AM-11, AM-12 |
| GA-9 | SHOULD | ACCEPT | R19: three citations were off; the two conditions had no gate. | AM-10 |
| GA-10 | SHOULD | ACCEPT | (a) H1's strings are pre-registered from the script's own vocabulary (R20); (b) the G-1 pattern and its allowed hits are named; (c) a MAIN0 row expected RED that is GREEN is a STOP; (d) BLOCKED needs a written waiver; (e) the retired set is pinned by name. | AM-16 |
| LI-1 | MUST | PARTIAL | R12 VERIFIED: 10 s, and never drawn on a 20-deck show at the default window. ACCEPT the text correction, the page rewrite, and state (3) on the 20-deck show. The file label carries the sentence (one line). REJECT a lasting mark on the strip or the tab: Boris, "The layer strip does not need to show the deck a clip is playing from." and, of the tab dot, "drop". | AM-12, AM-17, SF-4 |
| LI-2 | SHOULD | ACCEPT | R21: the menu is Deck, the chooser opens in the decks library. | AM-17 |
| LI-3 | SHOULD | ACCEPT | A rebuild inside one message-thread turn shows no blink (DeckView.cpp:359-365); the step could not fail. Rewritten; the wide deck leaves the page. | AM-11, AM-17 |
| LI-4 | SHOULD | PARTIAL | R11 TRUE. ACCEPT one line: rebuildGrid re-applies the selected-layer highlight (case G3). The "strip releases its grip when destroyed" part is filed. | AM-11, SF-2 |
| LI-5 | SHOULD | REJECT | Moot: Boris, asked "keep or drop?": "drop". There is no dot. deckIsPlaying stays as the retire predicate only (R13). | AM-12 |
| LI-6 | SHOULD | PARTIAL | ACCEPT the wording of Q-B (it now names the wide case). REJECT a new cue: the strip shows the clip. | AM-7, section 7 |
| LI-7 | NIT | REJECT | No new switch path is built (the split is filed), so the prune is today's behaviour; it is recorded in SF-3. | SF-3 |
| LI-8 | NIT | REJECT | Moot: forgetLayers is withdrawn. Residual risk stated in RR-6; both layer handlers rebuild at once (R4, R11). | AM-3 |
| SC-1 | SHOULD | ACCEPT | Plan :113 "It is the fallback if the council finds one more after-mutation site" and its R2 (:554-555); ME-1 found one. R3 makes it the only form that fixes the existing case untouched. | AM-1 |
| SC-2 | SHOULD | ACCEPT | No review finding asked for either; strips are not read in the gap (gates-r2 :8); the stuck grip on a rebuild is today's class (R6). | AM-3, SF-2 |
| SC-3 | SHOULD | ACCEPT | Both reviewers allowed "document it" (state-r2 :65-66, live-r2 :21); the plan's R1 names this fallback; R8 narrows the reach to a routine or a take replay. | AM-7 |
| SC-4 | SHOULD | PARTIAL | ACCEPT cutting the rebuildCells split (a NIT answered with a refactor of a shared file). Keep the header predicate; add LI-4's line. The split is filed to the ui lane. | AM-11, SF-3 |
| SC-5 | SHOULD | ACCEPT | The row has no failing arm of its own and guards a path the sync lanes rewrite next; a pinned count in MainComponent.cpp would false-RED them. | AM-9, SF-6 |
| SC-6 | SHOULD | ACCEPT | Pitfall 33 already states the rule (pitfalls.md:75); R14 lists the lines the plan missed. | AM-14 |
| SC-7 | SHOULD | PARTIAL | ruling-bf9b's B8 ("K1-K10 once more") is a pre-registered bar: it stays. The rows THIS ruling adds are conditional on the tree diff. | AM-16 |
| SC-8 | SHOULD | PARTIAL | ACCEPT one ASan app and no pre-fix app. REJECT "non-blocking": Harmony constraint. The failing arm is the wiring mutant in the same build dir (an incremental build). | AM-6 |
| SC-9 | NIT | ACCEPT | Both are kept: the unit ASan proof with a failing arm, and Boris's removal. | AM-4, AM-5, AM-12 |

RECONCILIATIONS (where seats disagreed)
- SC-1 against ME-2 / GA-2: both hold. The fix is inside setLayer / setClip AND the hook body is one test-driven
  function with a lint on the one line that cannot be driven.
- SC-2 against ME-3 / GA-2 / LI-4 / LI-8: the grip rule and forgetLayers are withdrawn, not extended. Every
  stuck-grip variant the seats found goes into ONE filed item (SF-2) for a lane that can prove it under TSan.
- SC-8 against GA-1 / ME-6: the pre-fix ASan app is dropped; the wiring mutant on the ASan app is a REQUIRED failing
  arm; the row stays a gate.
- SC-5 against GA-7: the tempo-feed row is dropped rather than given a mutant app; the verdict line names its blocked
  rows and the row count is pinned.
- SC-3 against LI-6 / GA-5: F2-C; Q-B tells Boris about the wide case; no cue, no live row.
- SC-4 against LI-3 / LI-4 / GA-8: no split; header predicate + highlight line; identity by SafePointer on the
  same-width walk; a pixel check on the differing-width walk.

## 3 AMENDMENTS (each OVERRIDES the plan body where they differ)
AM-1  P1 THE FIX (SC-1). LayerInspector::setLayer and ClipInspector::setClip drop the bindings of their 7 / 6 scalar
    controls FIRST, with UniversalParamControl::forgetConnection(), before any other call. Nothing else in them
    changes; UniversalParamControl::bindConnection is NOT changed (other owners keep its release). Plan P1 items 1-3
    (forgetLayer, forgetClip, LayerStrip::forgetLayer, DeckView::forgetLayers) and the edits of
    refreshUiAfterModelSwap and refreshAfterUndoRedo are NOT built: their setClip(nullptr) / setLayer(nullptr) /
    setClip(fresh) calls become safe as they are. removeDeck's pre-mutation nulling is unchanged.
    BEHAVIOUR CHANGE, stated and pinned (case N1): a re-point never releases a grip on the connection it leaves.
    A routine's grip is no longer dropped when another layer or clip is selected; a touch (250 ms) ends by itself.
    If an existing test asserts a release on setLayer / setClip (none found, R6), that is an expectation change of
    this amendment: STOP and report it, do not edit it silently.
AM-2  P1 THE HOOK (ME-1, ME-2, GA-2). NEW src/ui/InspectorRepoint.h, headless:
      bool compositionOwnsClip(const Composition&, const Clip*);   // an address walk; never dereferences the Clip
      void repointInspectorsAfterStackMove(ClipInspector&, LayerInspector&, Composition&, int selectedLayerRow);
    Body, in this order: (1) a Clip inspector whose clip the composition no longer owns (live and retired decks) is
    cleared (setClip(nullptr)); one it still owns is left alone; (2) the Layer inspector is re-pointed by the selected
    row (a stale row clears it). The hook statement in MainComponent calls exactly this function with the DeckView's
    selected layer row; that one-line adapter is the only code no test drives, and lint B4h pins it.
    refreshAfterUndoRedo keeps its own re-point by coordinate.
AM-3  P1 WITHDRAWN AND CORRECTED (SC-2, ME-3, ME-4, LI-8). Withdrawn: the grip rule (plan P1 item 6), the Layer
    static_assert as a STOP condition (case AS2 is the tooth), the plan's cases A7 and A8, its mutants MU2, MU4, MU5.
    Corrected text: (a) plan :9-10 becomes "after a layer-stack move neither inspector holds a pointer the model no
    longer owns, and no re-point reads the pointer it leaves; strips and cells are rebuilt by the caller in the same
    message-thread turn"; (b) F7 gains ClipInspector's clip_ (ClipInspector.h:95), its 6 scalar controls, its effect
    stack, and ClipCell::clip_ (ClipCell.h:95); (c) P6(c) "NOT a lasting dangling pointer" is true for Remove Deck and
    Undo / Redo, was false for Remove Layer (fixed here) and is false for Clear Layer / Deck Clips (SF-1);
    (d) "so it is never stuck" (plan :138-139) is struck.
AM-4  P1 TESTS (ME-5, GA-4). In tests/test_show_model.cpp, or another target that links both inspectors (builder's
    choice; every target that hosts an asan case is in probe-asan-unit.sh's TARGETS). Every memory case REQUIREs that
    its storage moved or died (capacity bound before, address differs after); a failed precondition is a failure,
    never a pass.
    AS0  the EXISTING case at tests/test_show_model.cpp:1462, unedited.            ASan-RED on the pre-fix src.
    AS1  Load Deck of a 5-row deck, Layer inspector on layer 1, through repointInspectorsAfterStackMove; then
         tickModulation + refresh.                                                 ASan-RED under MU1.
    AS2  AS1 with a Hand::Lane grip on layer 1's Opacity: afterwards the grip is still held on c.layers[1]'s
         connection and the control is bound to it.                                ASan-RED (a WRITE) under MU1.
    AS3  Remove Layer of the inspected last layer -> empty; undo -> re-pointed.    Functional only.
    AS3b Clip inspector on a clip of the LAST layer; RemoveLayerCmd through the real fence and the production hook
         function; then the Clip inspector's tickModulation + refresh; its clip is null.
                                                                                   ASan-RED under MU2.
    AS4  Add Layer + undo.                                                         Functional only.
    AS5  model swap inside the fence with both inspectors bound, then setClip(nullptr); setLayer(nullptr) and the
         timer calls (the app's order).                                            ASan-RED on the pre-fix src.
    AS6  Undo of a wide Load Deck while the Clip inspector shows a clip of that deck, then the app's
         setClip(fresh) order.                                                     ASan-RED on the pre-fix src.
    N1   the pinned behaviour of AM-1: setLayer(B) leaves a Lane grip and a Decaying touch on A's connection
         untouched; the control is bound to B's.                                   Functional (RED before the fix).
    MUTANTS (never committed; each must turn the named cases RED): MU1 the forget-first statements removed from
    setLayer / setClip -> AS0, AS1, AS2, AS5, AS6 (ASan) and N1; MU2 step (1) removed from the hook function -> AS3b
    (ASan); MU3 the wiring line deleted -> lint B4h (any build) AND the live row (AM-6).
AM-5  B3b ASAN UNIT GATE (GA-3). tests/CMakeLists.txt gains ADNA_ASAN_TEST_PROPERTIES in the tsan set's shape: LABELS
    asan; TIMEOUT 300; ENVIRONMENT "ASAN_OPTIONS=abort_on_error=0:halt_on_error=1"; FAIL_REGULAR_EXPRESSION
    "ERROR: AddressSanitizer". The asan cases carry a Catch2 tag [asan]; they are registered under the label either by
    a second catch_discover_tests of the same binary (TEST_SPEC "[asan]", a TEST_PREFIX, only when ADNA_SANITIZE holds
    "address") or, if that does not work in this Catch2 (ASSUMED, FM-4), by one dedicated binary as the tsan lane did.
    A Release build's test count must not change because of the label. NEW .harmony/probe-asan-unit.sh = the tsan
    script's shape: build dir build-asan, -DADNA_SANITIZE=address, TARGETS, EXPECTED_ASAN_CASES pinned, fail-closed
    exit 3 below it, `ctest -L asan --no-tests=error --output-on-failure`, last line `PROBE-ASAN-UNIT GREEN (<n> cases,
    0 reports)` or `PROBE-ASAN-UNIT RED (<k> of <n> failed)`, exit = ctest's code. Cases under the label at the final
    head: AS0, AS1, AS2, AS3b, AS5, AS6, T6f, T6g, T6j. RED ARM = the same script at FIX-1's first commit (tests +
    label + script, src untouched): `PROBE-ASAN-UNIT RED`, failed set containing AS0, AS5 and AS6, with the raw
    "ERROR: AddressSanitizer: heap-use-after-free" lines pasted. AS1, AS2 and AS3b arrive with the fix (they call the
    new function); their failing arms are MU1 / MU2 through the same script.
AM-6  THE LIVE CALL-CHAIN ROW "ASAN-LIVE" (GA-1, ME-6, ME-7, SC-8). Harmony constraint: a full-tier gate exercises the
    real call chain. It stays a gate.
    SRC (test-server block only): the lever also calls deckView_->selectLayer(n) (plan P1 item 7); /api/debug/ui_text
    gains "inspected_layer" and "inspected_clip" (the bound layer's / clip's name, "" when none) and "inspector_tab"
    (the active tab's name), read on the message thread like its other fields.
    ONE ASan app, ASAN-FH, built from the head whose src is final (FIX-3's head; FIX-4 must change no file under src/,
    its `git diff --stat -- src` attached, else the app is rebuilt). NO pre-fix ASan app.
    NEW .harmony/probe-asan-live.sh (quit_ours, app lock, no Output window, no synthetic input). Launch: `open -g
    --env ADNA_INSPECT_LAYER=1 --env ASAN_OPTIONS=abort_on_error=0:halt_on_error=1:log_path=<abs out>/asan <asan app>
    --args --test-mode` -- the DEFAULT 3-layer show, nothing loaded. Each step is followed by 1.0 s and
    GET /api/health == 200; each VALID clause is read by GET; a failed one prints INVALID and exits non-zero.
      L0 VALID ui_text.inspected_layer == "Layer 2" and inspector_tab == "Layer" (the tab must be the active one:
         its 10 Hz refresh is what reads the layer, R10). Both are required again at L1, L2 and L3.
      L1 POST /api/debug/load_deck of nine-rows.json (9 rows, an image in row 1 column 0). VALID layers == 9,
         numDecks == 2, inspected_layer == "Layer 2".
      L2 POST /api/debug/undo. VALID layers == 3, numDecks == 1, inspected_layer == "Layer 2".
      L3 POST /api/debug/undo {redo}. VALID layers == 9, numDecks == 2, inspected_layer == "Layer 2".
      L4 POST /api/debug/duplicate_deck {0}; undo; redo. VALID numDecks 3 -> 2 -> 3.
      L5 show deck 1, fire its row-1 clip (REST), show deck 0, POST /api/debug/remove_deck {1}. VALID numDecks 2,
         retiredDeckCount == 1, layers[1].activeClip.retired == true. Then POST /api/debug/undo. VALID numDecks 3,
         retiredDeckCount == 0, not retired.
      L6 POST /api/load_composition of a show file (the model swap; LAST, because it unbinds the Layer tab).
         VALID the show loaded, inspected_layer == "".
    Verdict lines, exactly:
      `PROBE-ASAN-LIVE GREEN (7 steps, 0 INVALID, 0 "ERROR: AddressSanitizer", app alive at the end)`
      `PROBE-ASAN-LIVE RED (step L<k>)`      -- a report file <out>/asan.* or a dead app
      `PROBE-ASAN-LIVE INVALID (step L<k>: <clause>)`
      `PROBE-ASAN-LIVE BLOCKED (<reason>)`   -- exit 3; the ASan app does not build or start
    FAILING ARM (REQUIRED; an H-5 exception, because MAIN0 has no lever): ASAN-MU3 = the same build dir with the
    wiring line deleted (an uncommitted edit, one file recompiled, the app bundle copied to scratch, the edit
    reverted; `git diff --quiet -- src tests` must then hold and B4h must pass in the next ctest). Pre-registered:
    `PROBE-ASAN-LIVE RED (step L1)` (INFERRED from R10: the active Layer tab's 10 Hz refresh reads the freed layer
    within the second). GREEN on ASAN-MU3, or RED at another step, is a STOP item to Harmony, never explained away.
    WHAT THE ROW DOES NOT REACH, said in the verdict: the Clip inspector (no REST route selects a cell) and Add /
    Remove Layer (no route): unit cases AS3, AS3b, AS4, AS5, AS6 only.
AM-7  P2 UNDO OF ADD / DUPLICATE / LOAD DECK (SC-3, GA-5, LI-6): fork F2-C. src/core/DeckCommands.h only.
    AddDeckCmd::undo: cancelPendingInto, then retireOrEraseDeck in place of the erase. Its redo: restoreRetiredDeck
    first, the snapshot insert only if it is gone.
    InsertDeckCmd::undo WHEN THE COMMAND ADDED NO LAYERS: cancelPendingInto, retireOrEraseDeck; dispose the cells only
    of an ERASED deck. Its redo: restoreRetiredDeck first (no media reconnect: nothing was disposed); else today's
    body. WHEN IT ADDED LAYERS: today's body, unchanged -- the documented exception.
    TESTS (T6 family, tests/test_show_model.cpp, RED first): T6f Add Deck, a clip fired with no Undo step, undo:
    retired 1, the SAME Clip address plays, a queue into it cancelled; redo: back at its index with its id, same
    address, retired 0. T6g Load Deck with the show's row count: the same assertions; dispose NOT called while
    retired. T6h THE EXCEPTION, pinned: Load Deck of a 5-row deck into a 3-layer show, layer 0 plays its row-0 clip,
    undo: 3 layers, retired 0, dispose called once per cell; redo: 5 layers, the deck back under its id. T6i undo with
    retire, the clip is then replaced, a later fenced edit reaps; redo comes back from the snapshot with every cell
    reconnected. T6j Duplicate Deck: T6f's assertions.
    MUTANTS: MU6 retireOrEraseDeck -> erase in either undo -> T6f, T6g, T6j; MU7 the "added no layers" guard dropped
    -> T6h; MU8 redo ignores restoreRetiredDeck -> the address checks. B4f's pinned counts for DeckCommands.h are
    re-pinned and re-justified in the commit.
    NOT BUILT: probe row k9d_undo_load_playing (its driver pushes an Undo step, R8) and B7 state 9. The plan's F2-A
    text for the wide case is kept as the design of the follow-up (SF-5).
AM-8  P3 LINT B4h (tests/test_render_thread_lint.cpp, a new case in the file's style, comments stripped):
    (i)   MainComponent.cpp holds exactly ONE `undoService_.onLayerStackMoved =` and its statement holds
          `repointInspectorsAfterStackMove(`;
    (ii)  the first call in the body of LayerInspector::setLayer and of ClipInspector::setClip is the forget of the
          scalar bindings (the token the builder names, pinned);
    (iii) UndoService.cpp: the hand-over lambda holds the one `onLayerStackMoved(` call and is called on both exits
          of withDeckDetached (counts 1 and 2).
    The plan's lint (ii) and (iii) (forgetLayer / forgetClip / repointClipInspector tokens) are replaced by these.
AM-9  P4 K5 WITH LINK ON (GA-7, SC-5). The bar did not run and cannot (no Link build). It prints BLOCKED and is
    recorded as "K5 Link-on: NOT RUN (BLOCKED)". It is never written "covered". Whether the merge proceeds past it is a
    waiver Harmony writes (section 5 rule 1); the evidence she may cite is T5, k5_queue_link_off and B4i.
    B4i (lint): `LinkSync|linkSync_|AUDIODNA_HAS_LINK` has zero hits in src/model, src/render, src/core,
    src/ui/DeckView.cpp and in the bodies of handleDeckSwitch / handleClipTrigger / handleColumnTrigger. NO count pin
    on MainComponent.cpp. MU10 (name linkSync_ inside handleDeckSwitch) must fail it.
    NOT BUILT: k5_queue_tempo_feed (SF-6).
    probe-boxes: the .py prints the names of its blocked rows; the .sh prints them in the verdict line and compares
    the number of `--- <row>` headers of a full run with a pinned EXPECTED_ROWS (a mismatch = RED).
AM-10 P5 THE SIX 4.B OMISSIONS (GA-9): all ACCEPT, entered as late 4.B rows with these sentences (plan-bf9b.md):
    1 test_routine_engine D3 step removed -- :332-333 "`RoutineEngine::stopOnLayer(int layer)` (was (deck, layer)):
      stops every running routine touching that shared layer, whatever deck it fired from."
    2 test_routine_deck_view "off-deck" case -- :334-335 "deriveRoutineDeckView: bands for every running routine on its
      shared layer, whatever deck is shown (shownDeck stays for labels only)."
    3 test_composition duplicateDeck -- the queued-trigger half: :269 "there is no tuple to clear any more"; the
      layer-id half: :441-442 "a deck has no layers any more". The ":1607" reference is struck.
    4 test_undo_commands "stale DECK index" sub-steps -- :271-272 "TriggerClipCmd: addressed by shared layer index".
    5 test_layer_state_key case 1 -- :441-442 (as 3); the two-half key stays pinned (Pitfall 35).
    6 test_recorder_host / test_program_preamble -- :340-341 "PerfStateCapture captures the shared layers once and,
      per deck, only clip runtime."
    FIX-2 PROVES, by name in the lane report: a positive test in which stopOnLayer(L) stops a routine fired with
    another deck shown (added if absent); the output of `git grep -n -i "corner note\|off-deck" -- docs/claude/
    recording.md` with any sentence the code no longer matches corrected. The review round lists the six hunks by
    file and test name.
AM-11 P6 THE GRID (SC-4, LI-3, LI-4, GA-8).
    BUILT: (a) setupColumnTriggers colours each header with the predicate refresh() uses (one private helper), so a
    header is lit at once after ANY rebuild; (b) rebuildGrid re-applies the selected-layer highlight to the strips it
    has just created.
    NOT BUILT: the rebuildCells split (SF-3). A switch between decks of different widths still rebuilds strips and
    tabs; with (a) and (b) what Boris sees does not change. make-bf9b-check.py keeps every deck at 4 columns; K8's
    fixture is unchanged.
    TESTS (tests/test_layer_strip_source_deck.cpp, RED first):
    G1' a fixture with one 8-column deck among 4-column decks: across showDeck(0) / (wide) / (0) the strip-column
        snapshot is byte-equal in all three and the selected layer's highlight is kept. Object identity is NOT
        asserted here (SF-3). If byte-equality cannot be met after (b), STOP to Harmony with the diff image (FM-5).
    G2  column 3 fired on deck 0, showDeck(wide), showDeck(0): header 3 is lit at once, no extra refresh; after a
        full rebuildGrid() it is lit too.
    G3  after rebuildGrid() the selected layer's strip is highlighted.
    M-c (restated in AM-12) proves identity on the same-width walk with juce::Component::SafePointer.
    MUTANTS: MU11 showDeck always rebuilds -> M-c; MU12 the predicate dropped from setupColumnTriggers -> G2.
    K8 is not a gate for P6.
AM-12 P7 THE FINAL SCREEN DESIGN. Boris: "The layer strip does not need to show the deck a clip is playing from." and,
    asked whether the deck-tab dot stays, "drop". ruling-bf9b amendment 16(a), 16(b), 16(c) are removed.
    GOES: the plan's P7 code list, PLUS the dot: DeckView::syncTabDots and its call in refresh(), tabDotShownForTest,
    DeckTabButton's dot / kDotColour / dotBounds / paint branch, the `btn->dot =` line in the tab setup, and
    MainComponent's 30 Hz syncTabDots call (R13). Composition::deckIsPlaying STAYS (retire / reap).
    TESTS RETIRED, exactly these five (tests/test_layer_strip_source_deck.cpp): :169 "S3.1 the strip badge names the
    deck a playing clip came from ...", :221 "S3.1 a folded row draws no badge ...", :282 "S3.1 badge contrast ...",
    :331 "S3.1 a badge click ...", :372 "S3.1 a deck tab shows a dot iff ...". Mutation smokes MS7 / MS7b retire with
    them. Machine checks M-b, M-d (badge geometry), M-e and M-f of ruling-bf9b are retired.
    WHAT MUST REMAIN, so that a removed deck's still-playing clip and "no lit cell on the shown deck" do not read as
    a fault:
    (1) THE STRIP IS THE TRUTH. A layer strip shows the clip its layer plays -- picture, name, the X to clear it --
        identically whether the clip's deck is the shown one, another one, or a removed one. Pinned by the BF14 pin
        (the case at :250, rewritten): two strips playing the same clip content from deck 0, from deck 5 and from a
        removed deck are byte-equal; a 20-char deck name changes no pixel; a 30-char clip name changes only the name
        row's ellipsis; VALID: each differs from an empty layer's strip. RED at the pre-fix head. MU13: any
        deck-dependent paint in the strip fails it.
    (2) THE GRID IS THE BOX. M-a, unchanged: lit cells == {(row, col) : layers[row].activeRef == (shown.id, col)}
        for 3 fixtures (same deck, other deck, retired). The column header is lit only on the deck it was fired from
        (16(e), unchanged, now at once after a rebuild). A playing layer with no lit cell on the shown deck is the
        normal state of browsing boxes.
    (3) THE TABS SAY NOTHING ABOUT PLAYING. NEW pin M-t: the tab row's snapshot is byte-equal whether or not layers
        play from its decks (same shown deck). RED at the pre-fix head. MU14: any play-dependent paint in a tab
        fails it.
    (4) THE REMOVAL IS SAID IN WORDS. Harmony constraint: the Remove Deck undo hint naming the layers stays. It stays
        as built (a 10 s button, drawn only when it fits, R12). Because it does not fit on a 20-deck show, the file
        label carries the same sentence: `Removed deck "<name>" -- <Layer N> keeps playing its clip` (one line in
        removeDeck, text from the same pure builder; pinned in the case at :520). Nothing else is added: no mark on a
        strip or a tab.
    M-c RESTATED: createComponentSnapshot of the WHOLE strip column after showDeck(0), showDeck(5), showDeck(0) on the
    same-width fixture: byte-equal in all three; every LayerStrip the same object, proven by a
    juce::Component::SafePointer per strip (all non-null after the walk).
    LINT B4j: src/ holds zero `SourceBadge|sourceBadge|onSourceDeckClicked|kBadge|syncTabDots|tabDotShownForTest|
    kDotColour|dotBounds`.
    B7 LIVE STATES (replace ruling-bf9b's and the plan's; captures by Quartz window id of our pid, decoded):
    (1) deck 0 shown, layer 1 playing deck 1's clip: no lit cell in row 1; the strip shows the clip's picture and
        name; no mark on any tab but the shown deck's highlight.
    (2) deck 1 shown: that cell lit; the strip-column crop equals (1)'s within the capture noise floor.
    (3) the 20-deck check show, a removed deck's clip playing: the strip-column crop equals the capture before the
        removal within the floor; the X visible; VALID ui_text.file_label == the sentence of (4) above.
    (3b) a 4-deck fixture: the "Undo Remove" button with its text. VALID: the button is on screen.
    (4) TopBar without "Fade:".
    (5) column 3 fired on deck 0 while an Ignore Column layer keeps deck 1's clip: header 3 lit on deck 0 only; that
        row's cell unlit.
    (6) the Layer tab (ADNA_INSPECT_LAYER).
    (7) 20 decks, a 30-char clip name, a 20-char deck name on a tab, one folded layer playing another deck's clip: no
        number, no dot anywhere.
    (8) the load notice after an old show.
    (10) a switch between a 4-column and an 8-column deck: the strip column unchanged within the floor, the selected
        layer's highlight kept, the header state right.
    State 9 does not exist. BEFORE captures: (4) and a plain "deck 0 shown, layer 0 playing" grid on MAIN0. For (6)
    MAIN0 cannot open the Layer tab (it has no lever: git grep ADNA_INSPECT_LAYER on main, no hit), so the lane's own
    before-s6-layer-tab.png (lane report :1328) is cited as the BEFORE.
    CRITIC QUESTIONS (yes / no + reason)
    visual-design: "Do the layer strips and the deck tabs carry no number, dot or other mark for the deck a clip came
      from, and does everything else on them look as it did before this lane?"
    UX: "From a layer strip alone, can Boris always tell WHAT that layer is playing and clear it with the X -- also
      when the clip's deck is not the one shown, or was removed? After Remove Deck, does the text line tell him the
      clip keeps playing?"
    graphic-design: "Does the TopBar close the Fade gap with no orphan space, and are the strips and the tab row free
      of leftover gaps where the badge and the dot were?"
    logic: "Do the captures match M-a's model values for the same fixtures (lit cells = the layers' refs into the shown
      deck), and is every strip and every tab free of any deck-dependent mark in every state?"
    interaction-logic: "After 0 -> 1 -> 0, and after a switch between decks of different widths, does every strip,
      fader, band and the selected-layer highlight look as before, with only the grid cells, the lit header and the
      tab highlight changed?"
AM-13 TRANSPORT (Harmony constraint). Boris, asked whether a video that was replaced continues or restarts when fired
    again: "restart". That change is NOT built in this lane: a transport lane builds it right after the merge with a
    pre-registered expectation change for K10 (ii). In THIS lane contract C3 (resume) and row K10 stay as built (main
    behaves the same today). The Boris page drops ruling-bf9b's step 8.11.
AM-14 DOCS (SC-6). NO new pitfall and NO new CLAUDE.md line. docs/claude/pitfalls.md: Pitfall 33 gains one sentence
    ("the Layer and Clip inspectors' scalar controls too: setLayer / setClip forget their bindings first, so a
    re-point after a storage move never reads what it leaves; the stack-move hook is pinned by B4h"); the lane's own
    pitfall (:139, number 67 per R-S3) loses "strip badge". docs/claude/performance-controls.md: :44 drops "a strip
    badge"; :50 "On screen" is rewritten for AM-12 (1)-(4) with Boris's two sentences quoted; the Remove Deck bullet
    gains the label sentence and AM-7's rule with its exception; the resume sentence stays (it is the code) and gains
    "(Boris 2026-10-03: "restart" -- changed by the transport lane)"; :65 Guards -> "grid; no deck on a strip or a
    tab" + T6f-T6j + the two asan probes; the Link paragraph gains "K5 with Link on has no live driver: BLOCKED in
    probe-boxes". docs/claude/integration.md: ui_text's three new fields. .harmony/APP-INVENTORY.md: the LayerStrip and
    tab rows, the new probes. tests/CMakeLists.txt:3453-3454: the comment. docs/claude/recording.md: only per AM-10.
    The builder prints `wc -c CLAUDE.md` after the merge-in (cap 25,000).
AM-15 B6 PERF (GA-6). The bar is ruling-bf9b's, unchanged. Defined here: a run's value = the mean of frame_time_ms
    sampled every 0.5 s after a 5 s warm-up; SD = the standard deviation of one arm's RUN MEANS; pooled SD =
    sqrt((SD_a^2 + SD_b^2) / 2); THR = max(1.0 ms, 3 x pooled SD). Added read-outs on the same runs, same THR rule, as
    "stop for a look" (INFO, not a bar): gpu_time_ms, and callback cost = the mean of peak_callback_ms read every
    0.5 s (reading resets it, so each sample is that window's peak). The driver has `--selftest` (recorded numbers
    shifted by 2 ms must print FAIL). The fallback "the lane's reading" (plan R12) is struck: BLOCKED is BLOCKED.
AM-16 GATE HYGIENE (GA-10, SC-7). Section 5 is the only source of gate strings. H1's and U1's strings come from the
    scripts' existing vocabulary; plan R11 ("M3's report wins") is struck: a mismatch is a STOP. The G-1 pattern and
    its allowed hits are named. A MAIN0 row expected RED that is GREEN is a STOP. BLOCKED needs a written waiver.
    B2's retired set is the five names of AM-12. B8 keeps ruling-bf9b's rows; this ruling's new rows are conditional.
AM-17 THE BORIS PAGE (LI-1, LI-2, LI-3). Section 6 replaces the plan's section 7; section 7 replaces its section 8.
AM-18 P8 NITS: as the plan (state-r2 NIT 3 FILE; NIT 4 DROP; NIT 5 fix the comment, file the read; gates-r2 NIT 8 DROP).
    gates-r2 NIT 5: the Boris quote at k1b_duplicate and at the k7 save checks (no new probe-boxes row is added).

## 4 FINAL BUILD STAGES (after M1 / M2 / M3; one builder context each; one worktree, strictly in this order)
Every stage: RED first (raw lines in the lane report), then GREEN; `cmake --build` rc 0 for the app and every test
target; ctest serial, 0 failures, added / retired names listed; no cd in a command; mutants never committed; probes
quit only their own pid; `wc -c CLAUDE.md` printed.
NOT BUILT, so no builder builds it from the plan body: forgetLayer / forgetClip / LayerStrip::forgetLayer /
DeckView::forgetLayers; the grip rule; the Layer static_assert as a STOP; rebuildCells; the 8-column deck in
make-bf9b-check.py and K8; probe rows k9d and k5_queue_tempo_feed; B7 state 9; a new Pitfall or CLAUDE.md line; a
pre-fix ASan app.
FIX-1 MEMORY (AM-1, AM-2, AM-4, AM-5, AM-6's src, AM-8). Commit 1: the asan CMake properties, probe-asan-unit.sh, the
  [asan] tag on the existing case (AS0), cases AS5 and AS6 -- src untouched -- and the RED run through the script.
  Commit 2: the two forget-first statements, InspectorRepoint.h, the hook statement, the lever line, the three ui_text
  fields, cases AS1, AS2, AS3, AS3b, AS4, N1, lint B4h, MU1-MU3 (MU3 against the lint).
  PROVES before FIX-2: the RED verdict and report lines of commit 1; `PROBE-ASAN-UNIT GREEN` at commit 2; MU1 and MU2
  RED through the same script; probe-tsan-unit.sh not modified; B4f counts unchanged.
  STOP: AS0 is not RED on the pre-fix src (FM-2); an ASan report in a case this ruling does not name; an existing test
  that asserted a release on a rebind (AM-1).
FIX-2 COMMANDS (AM-7, AM-10, AM-18's comment). DeckCommands.h; T6f-T6j; MU6-MU8; B4f re-pinned; AM-10's two proofs.
  PROVES before FIX-3: T6f, T6g, T6j RED at FIX-1's head and GREEN after; T6h GREEN before and after and RED under MU7;
  the T6 / T7 / M families untouched and green; `PROBE-ASAN-UNIT GREEN` with T6f, T6g, T6j under the label.
FIX-3 SCREEN (AM-11, AM-12, AM-14's screen lines, AM-17). Badge and dot removal; the header predicate; the highlight
  line; the label sentence; the BF14 pin, M-t, M-c, G1', G2, G3; B4j; MU11-MU14; the docs lines that describe the
  screen; make-bf9b-check.py writes five-rows.json and nine-rows.json beside the show; section 6's page text.
  PROVES before FIX-4: the five retired cases listed by name and no other; the BF14 pin, M-t, G2, G3 RED at FIX-2's
  head; a headless snapshot of the strip column and of the tab row attached (decoded, looked at). The ASan app build
  of this stage's head starts in the background once its src is final.
FIX-4 PROBES, GATES, DOCS (AM-6's script, AM-9, AM-15, the rest of AM-14). No file under src/. B4i + MU10; probe-boxes
  verdict names and the row-count pin; probe-asan-live.sh on ASAN-FH and on ASAN-MU3; probe-boxes-perf.sh with
  --selftest; the remaining docs; the lane report's FIX section with the final gate table, the six 4.B rows, the
  FILED list (section 8).
  PROVES (hand-over to Harmony): probe-boxes on the final head prints section 5's exact K line; the two ASAN-LIVE
  lines (GREEN on ASAN-FH, RED (step L1) on ASAN-MU3), raw; `git diff --quiet -- src tests` after the mutant; the perf
  driver's selftest line and one short dry pass (no verdict).
Then ONE pinned review round a7491d4..FH (H-2), then section 5.

## 5 FINAL CONSOLIDATED GATE LIST (pre-registered; Harmony copies gate strings ONLY from here; FH = final lane head)
RIG (Harmony constraint): live rows under the app lock (lock.sh acquire_quiet_lock); `open -g <app> --args
--test-mode`; never an Output window; no synthetic input; canvas captures by POST /api/render_frame, window captures
only by Quartz window id of our pid; quit_ours only; HTTP Connection: close. Perf verdicts only on a quiet machine
(ps checked), arms interleaved, >= 5 runs per arm; a bar whose teeth equal the run-to-run drift is INFO.
ARMS: MAIN0 = a copy of the PRE-MERGE main app (main 5abdf01's build) taken BEFORE the merge (H-5). FH = the final
head's Release app. ASAN-FH / ASAN-MU3 = AM-6's two apps. d(), noise, floor, tol, t(frame): ruling-bf9b's rig text
(:468-469). Per-arm readers: MAIN0 reads decks[activeDeck].layers[*].activeClipColumn / previousClipColumn (INFERRED:
main has the pre-bf9b REST shape); FH reads top-level layers[*].activeClip / previousClip {deckId, column, retired}.
RULES
 1 BLOCKED is never a pass. A row that prints BLOCKED stops the list until it runs or Harmony writes
   `WAIVER <row>: <reason> -- <who>, <time>` in the session log. No waiver is pre-authorised here.
 2 INVALID (a VALID clause failed) is RED.
 3 A row marked "RED on MAIN0" that is GREEN on MAIN0 has no failing arm: a STOP item (Harmony rules in writing: keep
   with the reason, or fix the row). A RED for another reason than the lane report's STAGE_P table is LISTED with the
   reason, never re-thresholded (H-5).
 4 No bar is loosened. A bar that cannot be met is reported as not met.
BY REFERENCE, NOT RE-RUN: G0-G7 of ruling-bf9.md at STAGE_P_HEAD (passed in the lane's S0; lane report "Gates G0-G7").
ORDER (stop at the first RED; nothing live before G-0 and G-1)
 G-0 ARMS: MAIN0 copied and its binary sha256 recorded; `git diff --stat a7491d4 FH` attached to the review.
 G-1 SAFETY: run list = probe-boxes.sh, probe-milkdrop.sh, probe-ui-files-rename.sh, probe-asan-live.sh,
     probe-boxes-perf.sh, the B7 capture script, and probe-quit-ours.sh (sourced). `grep -nE
     "osascript.*Audio-DNA|pkill|killall|adna_kill" <run list>`: every hit listed. Allowed: a hit inside a block that
     first proved the only running Audio-DNA pid is the one the script launched (quit_ours in probe-quit-ours.sh,
     H-3; probe-milkdrop.sh's own guarded quit). Any other hit = RED.
 B1  BUILD at FH: `cmake --build build --config Release -j$(sysctl -n hw.ncpu)` rc 0, all targets.
 B2  UNIT: `ctest --test-dir build --output-on-failure`, serial (this session's rule; ruling-bf9b wrote -j8), 0
     failures. count(FH) == count(M3 head) + added - retired. RETIRED = exactly the five names of AM-12; any other
     missing name = RED. Required present and passing, by name: AS0, AS1, AS2, AS3, AS3b, AS4, AS5, AS6, N1; T6f, T6g,
     T6h, T6i, T6j; G1', G2, G3; the BF14 pin; M-t; M-a; M-c; B4h, B4i, B4j; and everything ruling-bf9b's B2 names
     except the five retired cases. Recorded as biting: MU1, MU2, MU3, MU6, MU7, MU8, MU10, MU11, MU12, MU13, MU14.
 B3  TSAN: `.harmony/probe-tsan-unit.sh` exit 0, every [tsan] case of the merged script passes, zero
     "WARNING: ThreadSanitizer" (as written; the expected count is the merged script's EXPECTED_TSAN_CASES).
 B3b ASAN UNIT (NEW): `.harmony/probe-asan-unit.sh` exit 0, last line `PROBE-ASAN-UNIT GREEN (<n> cases, 0 reports)`
     with n == the script's EXPECTED_ASAN_CASES. Its RED arm (FIX-1 commit 1, in the lane report): `PROBE-ASAN-UNIT
     RED (<k> of <n> failed)`, AS0 / AS5 / AS6 failed, >= 1 "ERROR: AddressSanitizer: heap-use-after-free".
 B4  TEXT: a-g as written in ruling-bf9b (B4f's counts = FIX-2's re-justified ones); h = AM-8; i = AM-9; j = AM-12.
     Plus the docs grep: `git grep -n -i -E "strip badge|source-deck badge|badge, dots|tab dot|shows a dot" FH --
     docs/claude CLAUDE.md .harmony/APP-INVENTORY.md tests/CMakeLists.txt` -> 0 hits.
 ASAN-LIVE (NEW; the plan's A2-live): `.harmony/probe-asan-live.sh` on ASAN-FH prints exactly
     `PROBE-ASAN-LIVE GREEN (7 steps, 0 INVALID, 0 "ERROR: AddressSanitizer", app alive at the end)`; on ASAN-MU3
     exactly `PROBE-ASAN-LIVE RED (step L1)`. `PROBE-ASAN-LIVE BLOCKED (...)` -> rule 1; B3b still stands. A Release
     run of the same steps is INFO only (a Release app reads freed memory silently).
 K   PROBE-BOXES, MAIN0 first (k7_old_take records the take), then FH with BOXES_OLD_TAKE. Bars of K1 / K1a-d / K1t,
     K2, K2v, K3, K4, K4b, K5 (Link off), K6, K7, K8, K8b, K9a-c, K10: as written in ruling-bf9b. No fixture change,
     no new row. FH verdict line, exactly:
     `PROBE-BOXES BLOCKED 1 [k5_queue_link_on] (0 FAIL; 1 pre-registered bar(s) did not run -- not a pass)`, rc 3,
     and rows run == the script's EXPECTED_ROWS (every K row of ruling-bf9b's list + k1b_duplicate, names in the lane
     report). Any FAIL, any other blocked row, a row-count mismatch = RED. The one BLOCKED row falls under rule 1
     ("K5 Link-on: NOT RUN (BLOCKED)", AM-9). MAIN0: RED expected on k1a (switch_deck, OSC, load_deck),
     k1b_switch_video, k1b_duplicate, k1c_switch_midfade, k1t_history_freeze, k1t_history_feedback, k1d_ia, k1d_ib,
     k1d_ii, k1d_iii, k2_nothing_unseen, k3_autopilot, k4_ignore_column_across, k5_queue_link_off, k7_old_show,
     k7_old_take, k8_twenty_decks (the lane report's STAGE_P table, :1431-1458); rule 3 applies. BF9B-only rows print
     N/A there.
     Harmony constraint: K10 (i) and (ii) and contract C3 stay as built in this lane. Boris's "restart" is built by
     the transport lane right after the merge, with a pre-registered expectation change for K10 (ii).
 H1  m9b_deck_switch_live in .harmony/probe-milkdrop.py (R-S1; built in M3). Bar: 20 decks, MilkDrop on deck 0 layer
     0 with a preset playlist; walk switch_deck 1..19 and back while the playlist's interval passes at least twice.
     PASS iff (H3) the preset advanced at least once while a deck other than 0 was shown, (H2) no loadPreset / resize /
     releaseGL is caused by a switch, and every capture during the walk is a MilkDrop frame (d against black >= 5).
     Strings: on FH a line beginning `PASS  m9b_deck_switch_live` and the last line `PROBE-MILKDROP GREEN`; on MAIN0
     (MILKDROP_MODE=pre, that row) a line beginning `RED-OK  m9b_deck_switch_live`. `VOID`, or other tokens, = STOP.
     The other probe-milkdrop rows stay green on FH (m9_deck_roundtrip: per M3's disposition).
 U1  (H-6) `.harmony/probe-ui-files-rename.sh` on FH: GREEN including the positive-control row (+1); on MAIN0 rows
     R3 / R4a / R9a RED on the build-count clause only. A different pattern = STOP.
 B5  OLD FILES: as written in ruling-bf9b; its save + reload half is driven by /api/debug/save_composition.
 B6  PERF: `.harmony/probe-boxes-perf.sh` (definitions: AM-15). (i) INFO, MAIN0 vs FH, 1 deck 3 layers: FH - MAIN0 >
     THR prints `B6(i) STOP-FOR-A-LOOK`. (ii) BAR, FH 20 decks (K2v's fixture) vs FH 1 deck with the same 3 clips:
     `B6(ii) PASS` iff mean(20) - mean(1) <= THR; `B6(ii) INFO-NOISY` when either arm's (max - min) of run means
     >= THR (re-run quiet, never PASS); else `B6(ii) FAIL`. (ii-b) the same comparison on gpu_time_ms and on callback
     cost: a difference > THR prints `B6(ii-b) STOP-FOR-A-LOOK`. (iii) INFO: K8's walk sampled every 100 ms (mean
     frame_time_ms; samples with peak_frame_time_ms > 25 ms), both arms. Not quiet: `PROBE-BOXES-PERF BLOCKED (machine
     not quiet: <ps line>)` -> rule 1. A PASS is reported with "(metric sensitivity not proven)" unless FM-6 was run.
 B7  VISUAL WORK GATE. MACHINE (ctest): M-a, M-c, M-t, the BF14 pin, G1', G2, G3 (AM-11, AM-12). LIVE: AM-12's states
     on FH with its BEFORE captures; decoded and looked at. CRITICS: AM-12's five questions. PASS = every machine
     check AND five yes. A "no" returns the lane to FIX-3, never to Boris. Then ONE page for Boris (section 6).
 REVIEW: the one pinned round a7491d4..FH is APPROVE with no MUST; its checklist names AM-10's six hunks.
 MERGE: Harmony merges lane/bf9b into main (H-1: no conflict expected).
 B8  POST-MERGE on main: B1, B2, B3, then the K batch once more on the merged build (ruling-bf9b's B8; same expected
     verdict line). The rows this ruling adds -- B3b, ASAN-LIVE, H1, U1 -- are re-run on main only if `git diff --stat
     FH main -- src tests cmake CMakeLists.txt .harmony` is non-empty; if it is empty their FH results stand and the
     empty diff is attached. If Harmony does not re-run K on an identical tree she records `B8-K NOT RE-RUN (tree
     identical to FH, diff attached)`: a reported bar and her decision, not a pass string.
FACTS HARMONY MUST MEASURE (nobody can establish them by reading)
 FM-1 Does an ASan Audio-DNA link, start in --test-mode and end on an ASan error without a crash dialog? Test: build
      it, launch with the lever, GET /api/health, quit_ours; then the ASAN-MU3 run. Yes -> AM-6 as written. No ->
      `PROBE-ASAN-LIVE BLOCKED`, rule 1; B3b stands.
 FM-2 Is the existing case (AS0) RED under ASan through the new script on the pre-fix src? Test: FIX-1 commit 1. Yes
      -> proceed. No -> STOP: the unit gate has no failing arm.
 FM-3 Is ASAN-MU3 RED at step L1? Yes -> the live row has teeth. GREEN -> STOP: the row proves nothing. RED at
      another step -> STOP: the step map is wrong.
 FM-4 Does a second catch_discover_tests (TEST_SPEC "[asan]", TEST_PREFIX) register on this Catch2? Test: configure
      build-asan, `ctest -N -L asan`. Yes -> that form. No -> one dedicated asan binary.
 FM-5 Is the strip column byte-equal across a rebuildGrid once the highlight is re-applied? Test: G1'. Yes -> as
      written. No -> STOP with the diff image; the difference is a fact for SF-3, the assert is not loosened.
 FM-6 Can frame_time_ms see a deck-scaling cost at all? Test (Harmony's call, one scratch Release build): a 2 ms busy
      loop per deck in the composite pass must make B6(ii) FAIL. Not run -> a B6(ii) PASS carries "(metric
      sensitivity not proven)".
 FM-7 How many deck tabs leave room for the "Undo Remove" button at the capture window's width? Test: B7 state (3b)'s
      VALID clause. It sizes that fixture and section 7's Q-D.
 FM-8 Did main move in src / tests / cmake / .harmony after the merge-in? Test: B8's diff. Empty -> the new rows'
      FH results stand. Non-empty -> they re-run on main.

## 6 WHAT ONLY BORIS CAN CHECK (one page, after B7; plain words)
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
    Expect: D3 C2 keeps playing on the right and its strip looks the same; the Deck 3 tab is gone; the text line at
    the top (where the file name shows) reads: Removed deck "Deck 3" -- Layer 2 keeps playing its clip. Press Cmd+Z:
    the Deck 3 tab comes back, D3 C2 still playing. Wrong: the right half goes black, or the strip goes empty. (With
    only a few decks an "Undo Remove" button also shows at the end of the tab row for 10 seconds; with 20 decks there
    is no room for it.)
8.6 Click "D2 C1" on Deck 2 (Layer 1 row), then click 5 different deck tabs, then press Cmd+Z once. Expect: the D2 C1
    you fired is undone; deck clicks are not Undo steps.
8.7 Select a layer, Layer tab: "Persistent" is gone; "Ignore Column Trigger" sits alone on its row.
8.8 The top bar has no "Fade:" control: a deck change never changes the picture, so there is nothing to fade.
8.9 Open your show "test with harry": a yellow note in the top row says it was converted (hover it for the details);
    your layers look as they did.
8.10 Save "bf9b-check", close it, open it again: the decks and the layer looks come back; the layers start empty (as
    before: what is playing is not saved).
8.11 REMOVED. It asked you to confirm that a replaced video continues where it was. You answered "restart"; the next
    build changes it, so there is nothing to confirm here.
8.12 Click the Layer 2 strip (it gets a highlight) and open the Layer tab. Then in the menu bar: Deck -> Load
    Deck...; in the file window go to the bf9b-check folder and pick five-rows.json (a deck with five rows; your show
    has three layers). Expect: a new deck tab appears and is shown, two more layers appear in the grid, the Layer tab
    still shows Layer 2, the Layer 2 strip is still highlighted, and the picture does not change. Press Cmd+Z: the new
    deck and the two layers go away again; the Layer tab still shows Layer 2. Wrong: the app quits, the Layer tab
    shows another layer or nonsense, the highlight disappears.
NOT on the page: Cmd+Z of a duplicated or loaded deck while a routine plays one of its clips. A clip you fire by hand
    is itself an Undo step, so Cmd+Z takes your clip back first. The case is machine-checked (T6f-T6j) and asked as
    Q-B.

## 7 BORIS QUESTIONS (each has a default; the build never waits)
Q-B "You duplicate a deck, or load one, and a routine starts one of its clips. Then you press Cmd+Z, which takes the
    deck back. Should that clip keep playing (only the deck's tab goes away), the same as when you delete a deck?"
    DEFAULT: yes, it keeps playing. "One exception in this build: if the deck you loaded had more rows than your show
    had layers, Cmd+Z also takes those extra layers back and that deck's clips stop. Is that acceptable for now, or
    should the clip keep playing there too?" DEFAULT: acceptable for now.
Q-C "You have a clip selected, so the Clip tab shows its settings, and you click another deck tab. Should the Clip tab
    stay on the clip you were working on, or jump to the clip in the same spot of the new deck?" DEFAULT: it stays,
    as today (nothing changes in this build; your answer goes to a later one).
Q-D "After you delete a deck, an 'Undo Remove' button shows for 10 seconds at the end of the tab row, but only when
    there is room. With many decks it does not fit, and the message appears only in the text line at the top. Is the
    text line enough, or do you want the button to always show?" DEFAULT: the text line is enough.
(The earlier question about the deck-tab dot is answered: "drop".)

## 8 SIDE FINDINGS (outside this lane; for Harmony to schedule)
SF-1 A Clip inspector can still hold a dead clip after an edit that destroys clips without moving the layer stack:
     Layer > Clear Clips and Deck > Clear Clips (R5), read at timer rate. Pre-existing, the same on main; no REST
     driver, so no live gate sees it. Cure: run AM-2's "owned, or clear" check after EVERY fenced edit (one more call
     site in UndoService's hand-over), with an ASan case (inspect a clip that has one effect, Clear Layer Clips,
     tickModulation). About 20 lines; recommended for the first follow-up lane. If Harmony wants it in this lane it
     joins FIX-1 with its own case under the asan label.
SF-2 Grip lifetime against widget lifetime, one item: (a) a strip or an inspector slider destroyed or re-pointed while
     a mouse button holds it leaves a Held grip until that control is dragged again (R6: LayerStrip has no destructor;
     effect rows forget); (b) a routine's grip on a layer that Move Layer re-indexed is released at the old index
     (R7); (c) the plan's grip rule (its P1 item 6), extended to effect-row connections (ME-3), is the candidate cure.
     It writes model grips outside the fence and needs a TSan case before it ships.
SF-3 A switch between decks of different widths rebuilds every strip and tab (R11). After AM-11 nothing Boris looks at
     changes, but the Undo Remove button hides early, an open rename box closes, a fader drag is dropped, and cells
     selected beyond the narrower deck's width are deselected. Cure: the plan's rebuildCells split (its F6-B), in the
     ui lane, with G1' upgraded to object identity.
SF-4 The "Undo Remove" button is never drawn once the tabs fill the row (R12): on a 20-deck show ruling-bf9b's 16(d)
     is carried only by the text line. ui lane: let it truncate, or move it.
SF-5 Undo of a Load Deck that added layers still erases a playing deck (AM-7's exception). The design is the plan's
     F2-A text; a live row needs a fire that is not an Undo step (a take or routine replay). Natural home: the
     transport lane, which re-registers K10.
SF-6 K5 with Link on needs a Link build and a toggle route; k5_queue_tempo_feed belongs with it and needs its own
     failing arm. Home: the sync lanes (Boris's sync feedback of 2026-10-03 rewrites that path).
SF-7 Stable Layer storage (the plan's F1-D) removes the whole class: a model change, its own lane.
SF-8 ApiServer reads retiredDecks_.size() from the http thread: tsan-r5 (the comment is fixed here, AM-18).
SF-9 Composition::crossfaderBlendMode has no reader left: listed with crossfaderPhase / crossfaderBehaviour as
     file-only fields.
SF-10 After a deck switch the Clip tab still edits the previous deck's clip while the grid highlights the shown
     deck's cell (state-r2 NIT 7): a behaviour choice, asked as Q-C.
Carried: ruling-bf9b's SF-1, SF-2, SF-3, SF-4 stand.

## 9 RISKS OF THIS RULING
RR-1 THE STRONGEST COUNTERARGUMENT. "Moving the fix inside setLayer / setClip changes what EVERY ordinary select does,
     on merge day: the old code released a grip when the inspector moved on, and the plan's author rejected exactly
     this for that reason. Three of four seats attacked the plan on the assumption that F1-C stays." It loses on what
     the release actually protected (R2, R6): of the three grips a re-point could release, a routine's must NOT be
     released by a selection (the old code dropped it), a touch ends by itself after 250 ms, and a human hold exists
     only while a mouse button is down on that very slider -- so it meets a re-point only through a keyboard or remote
     action mid-drag (Cmd+Z, a REST edit), where the old code either wrote to freed memory or released. The trade is a
     visible, self-clearing stuck slider in a two-handed corner against a silent read of freed memory at any site
     nobody listed -- and the council found such a site (ME-1). The plan itself names this fallback (:113, its R2).
     Cheapest test that could refute it: N1 plus the live row; a grip left behind after an ordinary select would show
     in N1. The inspector entries are reached only from a strip or cell click, the drop handlers, Undo / Redo, the
     hook, the model swap and Remove Deck (git grep of inspectLayer / inspectClip / setLayer / setClip / selectLayer
     in MainComponent.cpp at the pin): no keyboard or MIDI selection exists there. Fallback if Boris ever reports a
     stuck slider: keep forget-first and add the release at the two user-selection entries (inspectLayer /
     inspectClip), where the old storage is alive by construction.
RR-2 The hook clears the Clip inspector by an address match. If an edit freed a clip and created another at the same
     address before the hook ran, the inspector would stay on a live but different clip: memory-safe, semantically
     off. None of the three stack-moving commands allocates clips after freeing them (R4, R9; read, not run).
RR-3 The ASan app is ASSUMED to build, start and exit cleanly (FM-1). If not, the live row is BLOCKED and the merge
     needs a written waiver; the unit gate still has its failing arm.
RR-4 ASAN-MU3 is an uncommitted edit in the lane worktree. A leftover would fail lint B4h in the next ctest, and
     `git diff --quiet -- src tests` is a required proof line.
RR-5 AM-7 leaves two behaviours for Cmd+Z of Load Deck. It is pinned (T6h), documented and asked (Q-B); the reach
     needs a routine or a take replay (R8).
RR-6 Strips are not forgotten: a future caller that moves the stack and does not rebuild the grid would leave a strip
     reading freed memory. Today every caller rebuilds in the same turn (gates-r2 :8; the two layer handlers
     re-read); the live row runs Load / Duplicate / Remove Deck and Undo / Redo under ASan; SF-7 is the root cure.
RR-7 P2 has no live row. Its primitives run live in K9a-c and in ASAN-LIVE's L5; the two changed undo bodies are
     unit-only.
RR-8 The label sentence of AM-12 (4) is overwritten by the next file-label write (any clip fire). It is a message,
     not a state; a lasting mark was rejected on Boris's words.
RR-9 I could not read M1-M3's results (the worktree is off limits while it is merged). H1's and U1's strings are
     taken from main's script vocabulary and from H-6; "a mismatch is a STOP" covers a wrong guess.
RR-10 B6 may print BLOCKED or INFO-NOISY on merge day (Boris uses the machine), and its metric's sensitivity is
     unproven unless FM-6 is run.
RR-11 The second test discovery under the asan label is ASSUMED to work (FM-4); the dedicated binary is the fallback.
RR-12 G1' may reveal that a rebuilt strip differs by some pixel I did not foresee (FM-5). That is a STOP, not a
     reason to weaken the assert.

STATUS: DONE
