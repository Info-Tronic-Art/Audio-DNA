# plan-bf9b-merge -- lane bf9b FIX + MERGE-GATE delta plan (architect position, s-rta-1003, 2026-10-03)
Author: architect (read-only). Pin: lane/bf9b a7491d4 (read with git show / git grep at the pin only); main read as
`git show main:<path>`. Nothing was built, run or launched for this plan. Labels: VERIFIED = I read it at the pin or on
main (file:line given); INFERRED = reasoned from what I read; ASSUMED = not checked, named where it is used.

## 1 GOAL
Lane bf9b ("decks are boxes of clips") merges into main today. Every open round-2 item is either fixed with a test that
can fail, or ruled out with evidence, and Harmony has ONE pre-registered gate list for before and after the merge.
Verdict in one line: fix P1 (one "forget, then re-point" rule for every holder of a pointer into a Layer or a Clip, proven
under AddressSanitizer headless AND through the real call chain), P2 (Undo of Add / Load / Duplicate Deck retires a deck
that is still playing, like Remove Deck), P3 (the wiring line pinned by a lint + the live ASan row), P6 a + b (a deck
switch never rebuilds strips or tabs; the header is lit at once), P7 (BF14: the strip badge and its click are removed);
rule P4 "covered by, still printed BLOCKED", P5 six ACCEPTs, P6 c FILED with a Boris question, P8 one line each.
NOT in this lane: stable Layer storage (a model change), a Link build / Link driver, the selection-follows-deck rule
(P6 c), the retiredDeckCount cross-thread read (tsan-r5), SF-1 / SF-2 / SF-3 of ruling-bf9b, any rename of
test_layer_strip_source_deck.

## 2 ESTABLISHED FACTS (verified at a7491d4 unless "main:" is written)
F1  UndoService::withDeckDetached records layers.data() / size() before the mutation and calls onLayerStackMoved AFTER
    the fence and after onDecksReaped, in the headless and the renderer branch (src/core/UndoService.cpp:57-80).
F2  The hook is wired at src/MainComponent.cpp:1800 (`undoService_.onLayerStackMoved = [this]() {
    repointLayerInspector(); };`); repointLayerInspector is MainComponent.cpp:5304-5320 and calls
    LayerInspector::setLayer(freshLayer, scope) by the DeckView's selected layer row.
F3  LayerInspector::setLayer (src/ui/LayerInspector.cpp:728-741) = layer_ = layer; setEffects; syncFromLayer();
    bindScalarControls(). syncFromLayer's syncScalar calls UniversalParamControl::setParamValue (:906-913), which calls
    routineHandHolds() (UniversalParamControl.cpp:343-349, :355-359) = a READ of the control's OLD conn_->grip.
F4  UniversalParamControl::bindConnection (UniversalParamControl.cpp:126-146) first does `if (conn_ && conn_->grip.kind
    != None) conn_->release(...)` on the OLD conn_: a READ always, and a WRITE whenever ANY grip is active (human Held,
    a Decaying touch, or a routine's Lane grip) -- not only a routine grip. The destructor does the same (:120-124).
F5  setLayer(nullptr) also runs bindScalarControls -> bindConnection(nullptr, nullptr) -> F4 (LayerInspector.cpp:743-756).
    So refreshUiAfterModelSwap's `setLayer(nullptr)` AFTER the swap (MainComponent.cpp:2955-2959) reads freed memory;
    the same line exists on main (main: src/MainComponent.cpp:3038). ClipInspector::setClip(nullptr) has the same shape
    for its 6 scalar controls (src/ui/ClipInspector.cpp:788-824; its source-param controls already forget, :803, :837).
F6  The project already has the no-dereference drop: UniversalParamControl::forgetConnection() { conn_ = nullptr;
    live_ = nullptr; } (UniversalParamControl.h:146-151, s-rta-0925), used by EffectStackView::rebuildRows before it
    touches anything (EffectStackView.cpp:256-280) and by ClipInspector's source-param controls. So
    EffectStackView::setEffects (:130-134) is already safe on a dead vector.
F7  Holders of a pointer to / into a Layer (git grep of pointer members in src at the pin): LayerInspector::layer_
    (LayerInspector.h:74); its 7 scalar UniversalParamControls' conn_ / live_ (UniversalParamControl.h:202-203, bound
    at LayerInspector.cpp:744-755 to &layer_->scalarConns[s] / scalarLive[s] -- storage INSIDE the Layer); its
    EffectStackView::effects_ (EffectStackView.h:145, = &layer->layerEffects) and that view's row controls (bound to
    fx.paramConns[p] / fx.dryWetConn, EffectStackView.cpp:369, :424); LayerStrip::layer_ (LayerStrip.h:166; set only in
    DeckView::rebuildGrid, DeckView.cpp:151). No other member holds one: commands, Program and RoutineEngine resolve per
    call (DeckCommands.h:416, :452; TriggerCommands.h:122; Program.cpp:202). CompositionInspector binds into the
    Composition, a stable member (CompositionInspector.cpp:403; MainComponent.cpp:5178-5181 comment).
F8  ParamConnection: a MOVE keeps grip and state, a COPY clears them (src/connect/ParamConnection.h:146-168); release()
    sets grip None (:191). A human slider drag = gripHeld rank 3 (:181), released on drag end through conn_
    (UniversalParamControl.cpp:28; LayerStrip.cpp:450 through layer_). A routine releases by ControlPath key through its
    sink (src/recording/RoutineEngine.cpp:355, :386, :401), never through a widget.
F9  AddDeckCmd::undo and InsertDeckCmd::undo erase the deck (`comp->decks.erase`), InsertDeckCmd::undo then erases the
    layers it added and disposes every occupied cell of its snapshot; neither calls retireOrEraseDeck
    (src/core/DeckCommands.h: AddDeckCmd::undo and InsertDeckCmd::undo bodies, the review's :717 / :824). RemoveDeckCmd
    does cancelPendingInto + retireOrEraseDeck on execute and restoreRetiredDeck-else-snapshot on undo (same file,
    RemoveDeckCmd::execute / undo). Composition has retireOrEraseDeck (:624), restoreRetiredDeck (:639),
    reapRetiredDecks (:652), deckIsPlaying (:608), insertDeckKeepingId (:597).
F10 DeckView::showDeck (src/ui/DeckView.cpp:349-365) calls rebuildGrid() when the strip count, the column-trigger count
    or any row's cell count differs from the shown deck; rebuildGrid (:114-262) calls hideUndoHint() (:117), clears and
    recreates layerStrips_, clipCells_, columnTriggers_, deckTabs_, re-fans the routine bands (fanRoutineBands), prunes
    the cell selection, and never re-applies the lit column (setupColumnTriggers :427-450 creates every header unlit;
    only refresh() :296-303 colours them) nor the selected-layer highlight (no setSelected call in :114-262).
F11 MainComponent::removeDeck nulls both inspectors BEFORE the mutation (MainComponent.cpp:3838-3842);
    refreshAfterUndoRedo re-points the Clip inspector by the selected cell's coordinates on the shown deck and then
    calls repointLayerInspector (:5250-5275). handleDeckSwitch (:5572-5602) re-points neither.
F12 The strip badge at the pin: LayerStrip.h:79, :89-105, :111, :162-164; LayerStrip.cpp:532, :730, :740, :787,
    :936-943, :972-980, :1006-1065; DeckView.h:87-89; DeckView.cpp:177-178; MainComponent.cpp:1389-1391; docs
    performance-controls.md:44 ("a strip badge"), :50 ("On screen"), :65 ("badge, dots, grid"); tests
    tests/test_layer_strip_source_deck.cpp cases at :169 (M-b), :221 and :250 (M-d), :282 (M-f), :331 (badge click),
    :477 (M-c "outside the badge rects"). probe-boxes.py has no badge reference (git grep).
F13 Link: AUDIODNA_BUILD_LINK is OFF by default (CMakeLists.txt:36). linkSync_ appears only at MainComponent.cpp:598-601
    (the TopBar toggle), :4246-4251 (`applyTempoCommand("link", bpm, ...)`) and MainComponent.h:450; no Link identifier
    exists in src/model, src/render, src/ui/DeckView.cpp or the trigger path (git grep of LinkSync / linkSync_ /
    AUDIODNA_HAS_LINK). docs/claude/performance-controls.md:72 "Tempo only, never the phase". T5 exists
    (tests/test_show_model.cpp:471); probe-boxes k5_queue_link_on prints BLOCKED (probe-boxes.py:81, :1076-1079).
    The Link tick sends the tempo command "link"; the same command is sent from two other MainComponent sites
    (MainComponent.cpp:1887, :2245 -- INFERRED from the comment at :5657 to be REST / OSC set_bpm); "link" is a tempo
    VALUE that never realigns the beat (MainComponent.cpp:5657-5703; BPMTracker::followExternalTempo, src/analysis/
    BPMTracker.h:149-156, BPMTracker.cpp:611).
F14 Sanitizers: cmake/Sanitizers.cmake (ADNA_SANITIZE = address / undefined / thread); apply_sanitizers(AudioDNA) is
    in CMakeLists.txt:614 and on 120 test targets (tests/CMakeLists.txt), so -DADNA_SANITIZE=address instruments the
    APP too. .harmony/probe-tsan-unit.sh:31-45 is the configure recipe (deps from the main checkout's build/_deps,
    FETCHCONTENT_FULLY_DISCONNECTED=ON, RelWithDebInfo). The lane already launched with `open -g --env
    ADNA_INSPECT_LAYER=0 <app> --args --test-mode` (lane report "G7 VISUAL WORK GATE captures").
F15 The ADNA_INSPECT_LAYER lever (MainComponent.cpp:2330-2344) calls deckView_->onLayerSelected(n) + setActiveTab(Layer)
    but NOT DeckView::selectLayer; DeckView::selectedLayerIndex_ starts -1 (DeckView.h:199); onLayerSelected
    (MainComponent.cpp:725-729) only calls inspectLayer. So under the lever a re-point resolves "no selected row".
F16 main: /api/debug/load_deck, /api/debug/duplicate_deck and /api/debug/undo {redo} exist (main:
    src/api/ApiServer.cpp:320, :321, :334; ApiServer.h:221); ADNA_INSPECT_LAYER does not (git grep on main: no hit).
    main's .harmony/probe-milkdrop.py has rows m1..m10 incl. m9_deck_roundtrip (:623), no m9b.
F17 No B6 driver exists at the pin: no .harmony/*.sh / *.py runs an interleaved 60 s A/B of frame_time_ms (git grep
    -l frame_time_ms over .harmony/*.py and *.sh: probe-boxes, -canvas, -fitmode, -image-load, -outputs, -render-state
    rows only; probe-vupload-ab.{py,sh} is the interleaved-arms pattern for another quantity).
F18 Boris, verbatim (binding-decisions.md "2026-10-03 (s-rta-1003)"): "The layer strip does not need to show the deck a
    clip is playing from." and (BORIS_DECISIONS.md "Decks are boxes of clips") "when I switch between decks, do not
    change the clips playing in the layers or how they are playing. treat the decks as just a box of clips and I can
    switch between 20 decks looking for a clip and the playing will not be affected. does that make sense?"

## 3 ITEMS P1-P8

### P1 USE-AFTER-FREE READ after a layer-stack move
FINDING TRUE (VERIFIED by reading, F3-F5; the reviewer's ASan trace matches the code path exactly). Two corrections to
the reviews: (1) the write to freed memory needs ANY active grip, not only a routine's (F4); (2) "only two raw Layer*
holders" (gates-r2) is right for Layer* but leaves out the pointers INTO a Layer -- 7 conn_ / live_ pairs and
EffectStackView::effects_ (F7) -- and the Clip inspector's 6 scalar controls have the same shape after a deck erase or
a model swap (F5).
FORKS
 F1-A unbind BEFORE the mutation (a pre-fence hook). Loses: before the edit nobody knows whether the stack will move, so
      it would unbind on EVERY fenced edit (every clip drop, effect add) and rebind after -- the inspector's effect rows
      collapse on every edit, or the hook must save and restore pointers (more state than the bug). Right only where the
      caller knows everything dies (removeDeck already does it, F11; kept).
 F1-B make LayerInspector::setLayer / ClipInspector::setClip ALWAYS forget first (never release on a rebind). RUNNER-UP:
      one place per inspector, cannot be missed by a future caller. Loses today because it changes grip behaviour on
      every ordinary "select another layer" (a Decaying touch would no longer be ended at once) with no test asking for
      it, on merge day. It is the fallback if the council finds one more after-mutation site.
 F1-C (CHOICE) "forget, then re-point" at the after-mutation sites, using the project's own forgetConnection pattern (F6).
 F1-D stable Layer storage (unique_ptr per layer, or a deque). Kills the class at the root; it changes the container
      the GL thread walks and every fence writer. Not a merge-day change; FILED.
THE CHANGE (no code; names are the builder's to keep or better)
 1. src/ui/LayerInspector.{h,cpp}: `void forgetLayer();` -- dereferences NOTHING it is leaving: layer_ = nullptr;
    forgetConnection() on the 7 scalar controls; effectStackView_.setEffects(nullptr, EffectScope::none()) (safe, F6);
    resized / repaint.
 2. src/ui/ClipInspector.{h,cpp}: `void forgetClip();` -- the same for clip_, the 6 scalar controls, the source-param
    controls (already forget) and the effect stack.
 3. src/ui/LayerStrip.{h,cpp} + DeckView: `void LayerStrip::forgetLayer();` (layer_ = nullptr, nothing read) and
    `void DeckView::forgetLayers();` (every strip). A forgotten strip is inert until the caller's rebuildGrid (its
    refresh already returns on a null layer_, MainComponent.cpp:2969-2971 quotes it; paint / timer null-safety is
    INFERRED -- test A7 proves it).
 4. NEW src/ui/InspectorRepoint.h (pure, headless): `void repointLayerInspector(LayerInspector&, const DeckView&,
    Composition&);` and `void repointClipInspector(ClipInspector&, const DeckView&, Composition&);` -- each calls
    forget FIRST, then resolves by the selection's coordinates (a stale row / cell -> stays forgotten), then setLayer /
    setClip. MainComponent::repointLayerInspector and refreshAfterUndoRedo's clip re-point become one-line calls of
    these, so the tests drive the code the app runs (P3).
 5. src/MainComponent.cpp: the hook body becomes: deckView_->forgetLayers(); the grip rule (6); repointLayerInspector().
    refreshUiAfterModelSwap: forgetClip() / forgetLayer() replace setClip(nullptr) / setLayer(nullptr) (order and its
    comment unchanged). refreshAfterUndoRedo: through the two pure functions. removeDeck (:3838-3842) is unchanged
    (it runs before the mutation, memory valid).
 6. THE GRIP RULE (what replaces the release() the old code called on freed memory):
    - A routine hand's grip (Hand::Lane) travels WITH the layer: a vector growth moves each Layer, a move keeps the grip
      (F8). The routine lets go by ControlPath key through its sink (F8), which resolves the live connection -- so it is
      never stuck, and the inspector no longer drops a routine's grip as a side effect of a re-point.
    - A Decaying touch travels too and expires by itself.
    - A human Held grip (rank 3) exists only while a mouse button is down on a strip fader or an inspector slider. A
      stack move destroys the strips (their mouse-up never comes) and may put another layer under the inspector's row.
      Rule: "a layer-stack move ends every human slider hold on a layer scalar": in the hook, for every shared layer's
      scalarConns, a grip that is Held with rank 3 is released (message thread, after the fence -- the same thread and
      the same kind of write as UniversalParamControl.cpp:28). No drag in progress = no-op.
    - A removed layer's connections die with it: nothing to release.
    - To make "travels with the layer" a compile-time fact: `static_assert(std::is_nothrow_move_constructible_v<Layer>)`
      beside Deck's (src/model/Deck.h:164). If it does not compile, STOP and report (a growth would then COPY layers:
      grips cleared, effect buffers reallocated -- a new finding, not a thing to patch around).
 7. The test lever (F15), inside its AUDIODNA_TEST_SERVER block: also deckView_->selectLayer(layerIdx), so the live row
    re-points to a real layer instead of clearing.
TESTS, RED FIRST (functional asserts in every build; the memory teeth only under ASan -> gate B3b)
 In tests/test_show_model.cpp (links UndoService + LayerInspector already) and tests/test_layer_strip_source_deck.cpp
 (DeckView + strips), each case labelled `asan` in tests/CMakeLists.txt:
 A1 Load Deck of a 5-row deck into a 3-layer show, Layer inspector on layer 1, through the PRODUCTION
    repointLayerInspector; then tickModulation + refresh. (The reviewer's trace; RED at the pre-fix head.)
 A2 the same with a Hand::Lane grip on layer 1's Opacity before the move: after it the grip is still held on
    c.layers[1]'s connection; a release through the fresh connection clears it. (Pre-fix: a WRITE to freed memory.)
 A3 Remove Layer of the inspected last layer -> inspector empty; undo -> re-pointed. A4 Add Layer + undo.
 A5 model swap with both inspectors bound -> forgetLayer / forgetClip, then timer ticks.
 A6 Undo of Load Deck while the Clip inspector shows a clip of that deck -> production repointClipInspector.
 A7 a DeckView with strips, a stack move, hook body, then every strip's refresh / timer / paint BEFORE rebuildGrid.
 A8 a strip fader held (gripHeld on layer 0's Opacity), a stack move -> the grip is None. (RED without rule 6 in any
    build.)
 MUTANTS (scratch copy, never merged; each must turn the named case RED): MU1 drop forgetLayer() from the pure re-point
 -> A1, A2 (ASan); MU2 hook without forgetLayers -> A7 (ASan); MU3 delete the wiring line -> P3 lint + live row A2-live;
 MU4 drop rule 6 -> A8; MU5 refreshUiAfterModelSwap back to setLayer(nullptr) -> P3 lint, and A5's mutant twin (ASan).
 NEW .harmony/probe-asan-unit.sh, a copy of probe-tsan-unit.sh's shape: build dir build-asan, -DADNA_SANITIZE=address,
 TARGETS = the two test targets, EXPECTED_ASAN_CASES pinned, run with ASAN_OPTIONS=abort_on_error=0 (macOS has no
 leak checker: none requested), exit 0 iff every [asan] case passes AND the output holds zero "ERROR: AddressSanitizer".
LIVE CALL-CHAIN GATE (Harmony constraint: a full-tier gate exercises the real call chain): FEASIBLE and WORTH IT.
 Feasible: the app target is already wired for ASan (F14); env reaches the app through `open --env` (F14); the driver
 is REST only (F16). ASSUMED, to be proven by the builder on the RED arm before Harmony relies on it: (a) an ASan
 AudioDNA links and starts in --test-mode; (b) with abort_on_error=0 an ASan error ends the process by exit, with no
 macOS crash dialog (check: the lane's own "UserNotificationCenter windows: 0" line 16 s after). Worth it: it is the
 only gate through ApiServer -> finishStagedLoad -> pushCommands -> the fence -> the hook -> the 30 Hz timers, and the
 only one that can find a holder nobody listed. Cost: two ASan app builds (ASSUMED 10-25 min each at -j3) + a 2-minute
 batch per arm.
 NEW .harmony/probe-asan-live.sh (quit_ours, app lock, never an Output window, no synthetic input): launch `open -g
 --env ADNA_INSPECT_LAYER=1 --env ASAN_OPTIONS=abort_on_error=0:log_path=<abs out>/asan <asan app> --args --test-mode`;
 steps, each followed by 1.0 s (>= 30 timer ticks) and GET /api/health == 200: L1 load a 3-layer show; L2 POST
 /api/debug/load_deck of a 5-row deck; L3 POST /api/debug/undo; L4 undo {redo}; L5 /api/debug/duplicate_deck; L6 fire a
 clip of the new deck (REST), switch to deck 0, /api/debug/undo (P2's path); L7 /api/debug/remove_deck + undo; L8 POST
 /api/load_composition (the model swap). Add / Remove Layer have no REST driver (VERIFIED: git grep finds no layer add / remove route; the /api/debug list is
 ApiServer.cpp:303-334) -> unit A3 / A4 only, said in the verdict.
 PASS line: `PROBE-ASAN-LIVE GREEN (8 steps, 0 "ERROR: AddressSanitizer", app alive at the end)`; any report file
 <out>/asan.* or a dead app = RED. RED ARM (H-5 exception, listed): the ASan build of the lane's PRE-FIX head (the M3
 head, copied before FIX-1 edits), because pre-merge main has no lever to open the Layer tab (F16); expected RED at L2.
 Second tooth: MU3 on the ASan app.
GATE: B3b (unit) + A2-live, section 5.

### P2 UNDO of Add Deck / Load Deck / Duplicate Deck while a layer plays from that deck
FINDING TRUE by the code (F9); not run by anyone (INFERRED reach: a clip fired from the new deck by a source that pushes
no Undo step -- REST / OSC / a routine / a queued or autopilot fire -- then Cmd+Z).
FORKS
 F2-A (CHOICE) symmetric with Remove Deck: the undo retires a deck that is still playing, erases it otherwise.
 F2-B RUNNER-UP: a documented exception ("Undo takes the whole box back, a playing clip included") pinned in T6.
      Strongest case: Undo means "as if it never happened", a human-fired clip is undone first anyway, and F2-A's redo
      for a WIDER deck is the most intricate new logic of this plan. It loses on the stage: one Cmd+Z blanking a
      playing layer is exactly what Boris's rule forbids ("Whatever is in the layer should be what is playing"), and
      the retire / restore primitives already exist and are tested (T6).
 F2-C retire only when the deck added no layers, exception otherwise. Keeps the simple half; two behaviours for one
      key. The fallback if the council rejects the wide-deck redo.
THE CHANGE (src/core/DeckCommands.h only)
 AddDeckCmd::undo: cancelPendingInto(comp, id), then retireOrEraseDeck(addedIndex_) in place of decks.erase.
 AddDeckCmd::execute (redo branch): restoreRetiredDeck(id, addedIndex_) first; the snapshot insert only if it is gone.
 InsertDeckCmd::undo, in this order: (1) erase the layers the command added (their rows leave every live and retired
 deck: Composition::eraseLayer walks retiredDecks_ too, Composition.h:518-535, its retiredDecks_ loop at :530); (2) cancelPendingInto; (3) retireOrEraseDeck -- evaluated
 AFTER (1), so a clip that played only in an added layer does not keep the deck; (4) shown index restored; (5) dispose:
 erased -> every occupied cell of the snapshot (as today); retired -> ONLY the snapshot cells of the rows that left in
 (1) (nothing can play them any more; the dispose hook's own liveness scan still guards).
 InsertDeckCmd::execute (redo branch): re-insert the added layers; if restoreRetiredDeck succeeds, put the snapshot's
 rows for those layers back into the restored deck and reconnect only those cells; else the snapshot insert +
 reconnect of every cell (as today). Either way the deck returns at addedIndex_ with its id.
 Redo of a retired deck keeps the live playheads (the same Clip objects); a deck reaped meanwhile comes back from the
 snapshot (as Remove Deck's undo does).
TESTS (T6 family, tests/test_show_model.cpp), RED first at the pre-fix head
 T6f Add Deck, fire its clip with no Undo step, undo: retired 1, not in decks, playingClip(L) is the SAME Clip address,
     a queue into it cancelled; redo: back at its index with its id, same address, retired 0.
 T6g Load Deck of a 5-row deck into a 3-layer show, layer 0 plays its row-0 clip, undo: 3 layers, retired 1, same Clip
     address, dispose called exactly for the rows-3-4 cells; redo: 5 layers, rows 3-4 hold the snapshot's clips again,
     media hook called exactly for them, same playing address.
 T6h the same but the playing clip is in ADDED layer 4: undo erases (retired 0), dispose called for every cell once.
 T6i undo with retire, the clip is then replaced, a later fenced edit reaps; redo comes back from the snapshot with
     every cell reconnected.
 T6j Duplicate Deck (InsertDeckCmd, same row count): T6f's assertions.
 MUTANTS: MU6 retireOrEraseDeck -> erase in either undo -> T6f / T6g / T6j; MU7 dispose everything when retired -> T6g;
 MU8 redo ignores restoreRetiredDeck -> the address checks; MU9 step order (3) before (1) -> T6h.
 B4f's pinned structure-writer counts for DeckCommands.h change (new retireOrEraseDeck / restoreRetiredDeck sites):
 re-pinned and re-justified in the commit.
PROBE ROW (probe-boxes) k9d_undo_load_playing: K9's fixture; POST /api/debug/load_deck (a deck with ramp12.mp4 in row
 1), fire it into layer 1 by REST, show deck 0, POST /api/debug/undo. VALID iff numDecks dropped by 1 after ONE undo
 (else the fire was an Undo step: FAIL with that message, never a silent pass). PASS iff retiredDeckCount == 1,
 layers[1].activeClip.retired == true and K1b's bar holds (t at +2 s within 0.5 s of t_undo + 2.0). Then undo {redo}:
 numDecks restored, retiredDeckCount == 0, not retired, K1b's bar holds. Expected: RED on pre-merge main (its picture
 changes at the switch already) and RED at the lane's pre-fix head (the layer goes empty at the undo); GREEN at the
 final head.
GATE: B2 names T6f-T6j; probe row k9d in the K batch.

### P3 THE WIRING LINE IS UNTESTED
FINDING TRUE (F2; the headless case installs its own lambda -- both reviews).
FORKS: F3-A text lint only (cheap, proves presence, not behaviour); F3-B live row only (behaviour, but slow and not in
ctest); F3-C (CHOICE) the pure functions of P1.4 (the body is now test-driven) + a lint for the one line that cannot be
driven headless + the live ASan row as its behavioural tooth. Runner-up F3-A: it loses alone because a lint cannot
tell a wired-but-wrong body from a right one.
THE CHANGE: tests/test_render_thread_lint.cpp, a new case in the file's own style ("bf9b fix: the layer-stack hook is
wired and every after-mutation re-point forgets first"), code lines with comments stripped:
 (i) MainComponent.cpp holds exactly ONE `undoService_.onLayerStackMoved =` and its statement holds `forgetLayers(`
     and `repointLayerInspector(`;
 (ii) the body of refreshUiAfterModelSwap holds `forgetLayer(` and `forgetClip(` and neither `setLayer(nullptr)` nor
     `setClip(nullptr)`;
 (iii) the body of refreshAfterUndoRedo holds `repointClipInspector(` and `repointLayerInspector(`;
 (iv) UndoService.cpp: the hand-over lambda holds the one `onLayerStackMoved(` call and is called on both exits of withDeckDetached (counts pinned: 1 and 2).
MUTANT MU3 (delete the wiring line): the lint case FAILS and A2-live turns RED at L2.
GATE: B4h (in ctest) + A2-live.

### P4 K5 WITH LINK ON
STATE (VERIFIED F13): no Link build on either arm, no driver; the row prints BLOCKED.
THE CASE FOR A DRIVER: the bar is pre-registered ("with Link off and with Link on"); a RED arm exists (pre-merge main
 cancels the queue on a switch, with Link on as with it off); it needs a Link source fetch (network; the lanes build
 disconnected), a test-only REST toggle (new src on merge day), and TWO Link-built apps (main and lane). With no peer a
 Link session runs the local tempo, so what the row would add over k5_queue_link_off is one fact: the Link branch
 (MainComponent.cpp:4246-4251) does not disturb a queued trigger.
THE CASE FOR "COVERED BY" (CHOICE): that branch does one thing -- applyTempoCommand("link", bpm) -- and no Link
 identifier exists anywhere in the trigger, the model, the render path or the switch path (F13); a queued trigger fires
 from beatInBar / the bar count (T5 drives exactly that across a switch), and Link changes "tempo only, never the phase"
 (performance-controls.md:72). So Link-on differs from Link-off only by the tempo's source, which k5_queue_link_off and
 T5 do not depend on. Runner-up (the driver) loses on cost against one inferred fact that a lint can hold.
A CHEAP DRIVER FOR THE ONLY THING LINK-ON ADDS (new row, no Link build): the Link tick's whole effect is the tempo
 command "link" re-sent ~30 times a second, and REST set_bpm sends that same command (F13). probe-boxes row
 k5_queue_tempo_feed = k5_queue_link_off's fixture and bar, with POST set_bpm (the running BPM, unchanged) re-sent every
 100 ms from the queueing until after the bar. PASS iff it fires on that bar (REST and frame). Expected RED on pre-merge
 main (the switch cancels the queue), GREEN at the final head. If the builder finds set_bpm does NOT send "link", the
 row is dropped and that is reported (the rest of P4 stands).
WHAT IS ADDED so the "covered by" can fail: lint B4i in test_render_thread_lint.cpp -- `LinkSync|linkSync_|
 AUDIODNA_HAS_LINK` has zero hits in src/model, src/render, src/ui/DeckView.cpp, src/core, and in the bodies of
 handleDeckSwitch / handleClipTrigger / handleColumnTrigger; the only MainComponent.cpp hits are the toggle and the
 tempo feed (count pinned at the merged head). MU10: name linkSync_ inside handleDeckSwitch -> B4i FAILS.
WHAT THE MERGE GATE PRINTS: unchanged -- `PROBE-BOXES BLOCKED 1 (0 FAIL; 1 pre-registered bar(s) did not run -- not a
 pass)`, rc 3 (R-N3). Harmony's gate accepts rc 3 ONLY when the BLOCKED set is exactly {k5_queue_link_on} and writes
 "K5 Link-on: BLOCKED, ruled covered by T5 + k5_queue_link_off + k5_queue_tempo_feed + B4i (plan-bf9b-merge P4)". Never
 GREEN. A Link-driver lane is FILED (what it needs: the three things above). What stays unproven without it: Link's own
 library code (session start, peers) -- none of which is compiled into the app Boris runs (F13).
GATE: B4i; k5_queue_tempo_feed; the K batch's verdict rule above.

### P5 THE S2b 4.B OMISSIONS (six groups)
Rule of decision: plan-bf9b 4.B says "any OTHER changed assertion = STOP and report". The builder re-pointed instead of
stopping -- a process slip, recorded. On substance each old assertion states something the plan BODY removes, so a
REVERT would re-assert the impossible. All six: ACCEPT, entered as late 4.B rows (with the quote), and the one review
round a7491d4..final reads those six hunks by name.
 1 test_routine_engine D3 "another deck: nothing" step removed -- ACCEPT. Clause: plan F9 (:167) "Layer-scope targets
   resolve to the shared layer (any deck part ignored)". Condition: a positive twin exists (stopOnLayer stops the routine
   on the shared layer whatever deck is shown); if not, FIX-2 adds it.
 2 test_routine_deck_view "off-deck" case -> bands on the shared layers, no corner note -- ACCEPT. Clause: plan F9 +
   S2.9 (:330); Q-B. Condition: docs/claude/recording.md "Surfaces" says the same (builder greps the corner note).
 3 test_composition duplicateDeck layer-id / queued-trigger checks removed -- ACCEPT. Clause: plan S2.3 (:260): a deck
   holds no layers and no tuple; the surviving half is 4.B's own row ":1607 InsertDeckCmd ... untouched".
 4 test_undo_commands "stale DECK index" sub-steps -> stale composition / stale layer index -- ACCEPT. Clause: plan S2.4
   (:271) a trigger addresses the shared layer; R9 (:588).
 5 test_layer_state_key case 1 (layer ids repeat across decks) -- ACCEPT. Clause: the shared stack (plan F4, :145) makes
   the premise impossible; the two-half key stays pinned over the show's layer ids (Pitfall 35).
 6 test_recorder_host [perfstate] / test_program_preamble rr-fix read shared-layer settings from PerfState v2 "layers"
   -- ACCEPT. Clause: plan S2.9 (:330) "PerfStateCapture captures the shared layers once and, per deck, only clip
   runtime".
GATE: none new; the review round's checklist names the six hunks.

### P6 THE GRID ON A DECK SWITCH
(a) TRUE (F10): rebuildGrid leaves the fired column's header unlit until the next refresh.
(b) TRUE (F10): a switch to a deck of another width rebuilds every strip and tab. What a rebuilt strip loses today
    (by reading :114-262): the Remove-Deck undo hint (hideUndoHint); the selected-layer highlight (INFERRED: nothing
    re-applies it); a rename box open on a strip or a tab (Pitfall 65); a fader drag in progress (its mouse-up never
    comes -> a stuck human grip); hover / tooltip state. What survives: the routine bands (re-fanned), fader positions
    and the thumbnail (re-read from the model). That is "something on screen moves" on a switch.
(c) NOT a lasting dangling pointer (F11): every path that destroys a deck's clips either empties the inspectors first
    (removeDeck) or re-points them in the same message-thread turn (refreshAfterUndoRedo); the one-shot stale read
    inside that re-point is P1's class and is closed by repointClipInspector (test A6). What remains is the UX
    mismatch: after a switch the highlight names the shown deck's cell while the Clip tab edits the previous deck's
    clip. Same as main; it is a behaviour choice -> FILED with question Q-C (section 8), default "leave as it is".
FORKS for (a) + (b)
 F6-A call refresh() after rebuildGrid in showDeck (live-r2's cheap fix): cures (a), keeps the rebuild (b).
 F6-B (CHOICE) a switch rebuilds only what belongs to the deck: split rebuildGrid into the strips / tabs half and a
      cells half (`void DeckView::rebuildCells();` = clip cells + column headers + selection prune + layout); showDeck
      calls rebuildCells + refresh when only the width differs, and the full rebuildGrid only when the layer count
      differs (cannot happen on a switch: the strips are the shared layers). setupColumnTriggers colours each header by
      the same predicate refresh() uses (one small private helper), which cures (a) on every path.
 Runner-up F6-A: one line, but it keeps the rebuild Boris would see and the lost undo hint.
TESTS (tests/test_layer_strip_source_deck.cpp), RED first
 G1 a fixture whose deck 5 has 8 columns against 4: across showDeck(0) / (5) / (0) every LayerStrip AND every deck tab
    is the same object (RED today), the strip column is byte-equal in all three snapshots (with P7), the undo hint
    shown before the walk is still shown, the selected layer's highlight is kept.
 G2 column 3 fired on deck 0, showDeck(5), showDeck(0): header 3 is lit immediately, with no extra refresh (RED today
    whenever the width differs); after a full rebuildGrid() the header is lit too (the Remove Deck case of S3's
    found_not_fixed).
 MUTANTS: MU11 showDeck back to rebuildGrid -> G1; MU12 the header predicate dropped from setupColumnTriggers -> G2.
 PROBE: make-bf9b-check.py gives deck 6 eight columns (every other deck 4); K8's walk therefore crosses a width change;
 bar unchanged (peak_frame_time_ms <= 50 ms at every switch). B4d's smoke lint on showDeck's body still passes.
GATE: B2 names G1, G2; K8; B7 state 10.

### P7 BF14 -- "The layer strip does not need to show the deck a clip is playing from."
Harmony's consequence (binding-decisions.md 2026-10-03): amendment 16(a) and 16(c) are removed; 16(b) (the tab dot)
stays by default.
WHAT GOES
 Code (F12): the SourceBadge struct, sourceBadgeOf, getSourceBadge, sourceBadgeBounds, badgeFont, badge_,
 updateSourceBadge, paintSourceBadge, the kBadge* constants, the tooltip branch, the click branch and
 onSourceDeckClicked in LayerStrip.{h,cpp}; DeckView::onSourceDeckClicked (DeckView.h:87-89, DeckView.cpp:177-178);
 MainComponent.cpp:1389-1391 (the handler). The strip's 30 Hz timer stays (Pitfall 41 faders); only its badge call goes.
 Tests: the cases at :169 (M-b), :221 (M-d's badge geometry), :282 (M-f), :331 (badge click) are RETIRED; mutation
 smokes MS7 / MS7b are retired with them.
 Gates: B7 machine checks M-b, M-d, M-f; B7 live states and critic questions that name the badge (rewritten below).
 Docs: performance-controls.md:44 (drop "a strip badge" from the list of switch entries), :50 (the "On screen"
 paragraph loses its badge sentences and the "Badge and" of its last sentence), :65 ("badge, dots, grid" -> "dots, grid,
 no deck on the strip"); .harmony/APP-INVENTORY.md:59 if its LayerStrip row names the badge (builder greps).
 Boris page: step 8.2's number-on-the-strip text and its "Click the number"; step 8.5's "its strip number turns into x".
WHAT STAYS OR CHANGES
 M-a lit cells: unchanged. M-e tab dots: unchanged. The undo hint naming the layers and the load notice: unchanged.
 The case at :250 is KEPT and becomes the BF14 pin: "a layer strip paints nothing that depends on the source deck" --
 two strips playing the same clip content from deck 0 and from deck 5 (and from a removed deck) are byte-equal; a
 20-char deck name changes no pixel; a 30-char clip name changes only the name row's ellipsis. RED at the pre-fix head
 (the badge pixels differ), GREEN after. MU13: any deck-dependent paint in the strip -> the pin FAILS.
 M-c becomes: the strip column is byte-equal over the WHOLE column across showDeck(0) / showDeck(5) / showDeck(0) -- all
 three snapshots equal, every LayerStrip the same object -- on P6's differing-width fixture (case G1).
 New lint B4j: src/ holds zero `SourceBadge|sourceBadge|onSourceDeckClicked|kBadge`.
 A removed deck's still-playing clip now looks like any playing clip: its picture and name in the strip, the strip's X
 to clear it, no lit cell on any deck, no dot on any tab (its tab is gone), and the undo hint `Undo Remove "<name>" --
 Layer N keeps playing its clip` until the next structural change. That is the rule itself ("Whatever is in the layer
 should be what is playing"), so no replacement mark is proposed.
B7 LIVE STATES (replaces ruling-bf9b's list; window captures by Quartz window id of our pid, decoded)
 (1) deck 0 shown, layer 1 playing deck 1's clip: no lit cell in row 1, a dot on deck 2's tab, the strip shows picture
     and clip name only. (2) deck 1 shown: that cell lit; the strip column crop equals (1)'s within the capture noise
     floor. (3) a removed deck's clip playing: the strip column crop equals the capture before the removal; the X
     visible; the undo hint text; no dot for it. (4) TopBar without "Fade:". (5) column 3 fired on deck 0 while an
     Ignore Column layer keeps deck 1's clip: header 3 lit on deck 0 only; that row's cell unlit. (6) the Layer tab
     (ADNA_INSPECT_LAYER). (7) 20 decks, two dots, a 30-char clip name, a 20-char deck name on a tab, one folded layer
     playing another deck's clip (its dot shows). (8) the load notice after an old show. (9) Cmd+Z of Load Deck while
     its clip plays (P2): the layer still shows the clip; the tab is gone. (10) a switch between a 4-column and an
     8-column deck: strips, tabs and undo hint unchanged, the header state right. BEFORE (pre-merge main) only for (4),
     (6) and a plain "deck 0 shown, layer 0 playing" grid.
CRITIC QUESTIONS (yes / no + reason)
 visual-design: "Does the layer strip look exactly as it did before this lane -- no number, no mark for the deck -- and
   is the tab dot small, legible and never clipped by a long tab name?"
 UX: "From the tabs alone can Boris see which boxes have a clip playing, and reach one with one click on its tab?"
 graphic-design: "Does the TopBar close the Fade gap with no orphan space, and do the tab dots sit consistently on
   narrow and wide tabs?"
 logic: "Do the captures match M-a's and M-e's model values for the same fixtures, and is the strip free of any
   deck-dependent mark in every state?"
 interaction-logic: "After 0 -> 1 -> 0, and after a switch between decks of different widths, is every strip, fader,
   band, highlight, tab and the undo hint where it was, with only the grid cells, the lit header and the tab highlight
   changed?"
GATE: B7 (section 5).

### P8 NIT DISPOSITIONS
 state-r2 NIT 3 (crossfaderBlendMode lost its reader): FILE -- listed with crossfaderPhase / crossfaderBehaviour as
   file-only fields in the lane report's debt list; no code change (file back-compat).
 state-r2 NIT 4 (unreachable guard, CompositionLoad.h:63-64): DROP -- kept as a guard, as the reviewer labelled it.
 state-r2 NIT 5 (retiredDeckCount read from the httplib thread; the comment says "never reads a retired deck"): FIX the
   comment here ("except its size"), FILE the read itself to tsan-r5 (ruling-bf9b SF-3's class).
 gates-r2 NIT 5 (no Boris quote at the new rows): FIX here -- the quote at k1b_duplicate, the k7 save checks and at
   every row this plan adds (k9d, the K8 width change), in FIX-4.
 gates-r2 NIT 8 (save_composition does not force .json): DROP -- a TEST_SERVER-only localhost route with the trust of
   load_deck / drop_files; integration.md already documents it.

## 4 BUILD STAGES (P10) -- after M1 / M2 / M3; one builder context each; one worktree, so strictly in this order
Every stage: RED first (raw lines in the lane report), then GREEN; `cmake --build` rc 0 for the app and every test
target; ctest SERIAL 0 failures with the added / retired names listed; no cd in a command; mutants only on a scratch
copy; probes quit only their own pid; CLAUDE.md byte count printed.
FIX-1 MEMORY (P1 + P3). FIRST, before any edit: configure build-asan (-DADNA_SANITIZE=address, probe-tsan-unit.sh's
 recipe), build AudioDNA + test_show_model + test_layer_strip_source_deck at the M3 head, copy the app to scratch as
 ASAN-PRE, and record the reviewer's trace again with the existing fix case (RED). Then: forgetLayer / forgetClip /
 LayerStrip::forgetLayer / DeckView::forgetLayers; InspectorRepoint.h; the hook body, refreshUiAfterModelSwap,
 refreshAfterUndoRedo; the grip rule; the Layer static_assert; the lever's selectLayer; cases A1-A8; the B4h lint;
 probe-asan-unit.sh + the `asan` label; MU1-MU5.
 PROVES before FIX-2: probe-asan-unit.sh exit 0 with zero reports; every A case RED on ASAN-PRE's tree where it can be
 (A1, A2, A3, A5-twin, A6, A7) and A8 RED without rule 6; probe-tsan-unit.sh unchanged; B4f counts unchanged.
 STOP conditions: the static_assert does not compile; an ASan report in a case this plan does not name.
FIX-2 COMMANDS (P2 + P5's conditions + P8 state-r2 NIT 5's comment). DeckCommands.h; T6f-T6j; MU6-MU9; B4f re-pinned;
 P5 item 1's positive twin if missing; P5 item 2's doc check.
 PROVES before FIX-3: T6f-T6j RED at FIX-1's head, GREEN after; the T6 / T7 / M families untouched and green; the ASan
 unit gate still clean (T6g runs under it too: add it to the `asan` label).
FIX-3 UI (P7 + P6 a / b). The badge removal; rebuildCells + the header predicate; the BF14 pin, G1, G2, the retired
 cases; B4j; MU11-MU13; docs lines of section 6 that describe the UI; make-bf9b-check.py's 8-column deck and a deck file
 "five-rows" beside the show (step 8.12); the Boris page text of section 7.
 PROVES before FIX-4: the retired cases are listed by name; G1 / G2 / the BF14 pin RED at FIX-2's head; B7 machine
 checks M-a, M-c (whole column), M-e green; a headless snapshot of the strip column attached (decoded, looked at).
FIX-4 PROBES, GATES, DOCS (P4 + the probe rows + P8 gates-r2 NIT 5). B4i + MU10; probe-boxes rows k9d,
 k5_queue_tempo_feed and the quotes;
 probe-asan-live.sh, ASAN-FH app, the RED run on ASAN-PRE (and the "no crash dialog" proof), MU3 on the ASan app;
 .harmony/probe-boxes-perf.sh (B6's driver, F17); the remaining docs; the lane report's FIX section with the final
 gate table, the six 4.B rows, the FILED list.
 PROVES (hand-over to Harmony): probe-boxes on the final head prints BLOCKED 1 with exactly k5_queue_link_on; A2-live
 RED on ASAN-PRE and GREEN on ASAN-FH, raw lines; B6's driver runs one short interleaved dry pass (no verdict).
Then: ONE pinned review round a7491d4..final head (H-2), then section 5.

## 5 HARMONY'S MERGE GATE LIST (P9) -- pre-registered; FH = the final lane head
RIG (Harmony constraint): live rows under the app lock (lock.sh acquire_quiet_lock); `open -g <app> --args
--test-mode`; never an Output window; no synthetic input; canvas captures by POST /api/render_frame, window captures
only by Quartz window id of our pid; quit_ours only; HTTP Connection: close. Perf verdicts only on a quiet machine (ps
checked), arms interleaved, >= 5 runs per arm; a bar whose teeth equal the run-to-run drift is INFO, never PASS / FAIL.
ARMS: MAIN0 = a copy of the PRE-MERGE main app (main 5abdf01's build) taken BEFORE the merge (H-5); FH = the final
head's app; ASAN-PRE / ASAN-FH = P1's two ASan apps. d(), noise, floor, tol, t(frame) as ruling-bf9b's rig text.
Per-arm readers: MAIN0 reads decks[activeDeck].layers[*].activeClipColumn / previousClipColumn (INFERRED: main has the
pre-bf9b REST shape STAGE_P had); FH reads top-level layers[*].activeClip / previousClip {deckId, column, retired}.
Where MAIN0's RED pattern differs from the lane report's STAGE_P table (MAIN0 has no Stage P: Persistent and the deck
fade still exist) the row is LISTED with the reason, never re-thresholded (H-5).
ORDER (stop at the first RED; nothing live before G-0 and G-1)
 G-0 ARMS: MAIN0 copied and its binary sha recorded; `git diff --stat a7491d4 FH` attached to the review.
 G-1 SAFETY: every script a gate below runs quits only its own pid (M2's sweep); grep of the run list for by-name quits
     == 0 hits.
 B1  BUILD at FH: `cmake --build build --config Release -j$(sysctl -n hw.ncpu)` rc 0, all targets. (as written)
 B2  UNIT: `ctest --test-dir build --output-on-failure` SERIAL, 0 failures; count(FH) == count(M3 head) + added -
     retired, both lists in the lane report. Required present and passing, by name: A1-A8, T6f-T6j, G1, G2, the BF14
     pin, B4h, B4i, B4j, plus everything ruling-bf9b's B2 names EXCEPT the retired badge cases (M-b, M-d geometry, M-f,
     badge click). MU1-MU13 each recorded as biting.
 B3  TSAN: `.harmony/probe-tsan-unit.sh` exit 0, every [tsan] case of the merged script passes, zero "WARNING:
     ThreadSanitizer". (as written; the expected count is the merged script's EXPECTED_TSAN_CASES)
 B3b ASAN UNIT (NEW): `.harmony/probe-asan-unit.sh` exit 0; every [asan] case passes; `grep -c "ERROR:
     AddressSanitizer"` == 0. RED arm: the same script on the M3 head's tree (>= 1 report, the reviewer's trace).
 B4  TEXT a-g as written in ruling-bf9b, plus: h (P3 lint), i (P4 Link lint), j (zero badge identifiers in src/). B4f's
     pinned counts are the re-justified ones of FIX-2.
 A2-live ASAN CALL CHAIN (NEW): `.harmony/probe-asan-live.sh` on ASAN-FH prints `PROBE-ASAN-LIVE GREEN (8 steps, 0
     "ERROR: AddressSanitizer", app alive at the end)`; on ASAN-PRE it is RED at step L2 (H-5 exception: MAIN0 cannot
     open the Layer tab, F16). If an ASan app cannot be built or started, the row prints BLOCKED (never GREEN) and
     Harmony rules; B3b still stands.
 K   PROBE-BOXES, MAIN0 first (k7_old_take records the take), then FH with BOXES_OLD_TAKE. Bars of K1 / K1a-d / K1t,
     K2, K2v, K3, K4, K4b, K5 (Link off), K6, K7, K8, K8b, K9a-c, K10: as written in ruling-bf9b. CHANGED: K8's fixture
     has one 8-column deck (bar unchanged); NEW k9d_undo_load_playing (P2) and k5_queue_tempo_feed (P4). Expected FH verdict line, exactly:
     `PROBE-BOXES BLOCKED 1 (0 FAIL; 1 pre-registered bar(s) did not run -- not a pass)`, rc 3, the BLOCKED set ==
     {k5_queue_link_on} (ruled covered, P4). Any FAIL, any other BLOCKED = RED. MAIN0: RED on the rows that had a RED
     STAGE_P arm; BF9B-only rows print N/A there.
 H1  m9b_deck_switch_live in .harmony/probe-milkdrop.py (R-S1; built in M3 -- its exact strings are M3's and are
     copied from M3's report; the bar pre-registered here): 20 decks, MilkDrop on deck 0 layer 0 with a preset
     playlist; walk switch_deck 1..19 and back while the playlist's interval passes at least twice. PASS iff (H3) the
     preset advanced at least once while a deck other than 0 was shown, (H2) no loadPreset / resize / releaseGL is
     caused by a switch, and every capture during the walk is a MilkDrop frame (d against black >= 5). RED on MAIN0,
     GREEN on FH. The rest of probe-milkdrop's rows stay green on FH (m9_deck_roundtrip: per M3's disposition).
 B5  OLD FILES: as written; its save + reload half is driven by /api/debug/save_composition (k7's save checks).
 B6  PERF: no driver existed at the pin (F17); FIX-4's `.harmony/probe-boxes-perf.sh` is the driver. Interleaved, >= 5
     runs per arm alternating, 60 s each, mean frame_time_ms + SD. (i) INFO, MAIN0 vs FH, 1 deck 3 layers: an FH
     regression > max(1.0 ms, 3 x pooled SD) STOPS the merge for a look. (ii) BAR, FH 20 decks (K2v's fixture) vs FH 1
     deck with the same 3 clips: mean(20) - mean(1) <= max(1.0 ms, 3 x pooled SD); if either arm's spread of run means
     (max - min) >= that threshold the verdict is INFO-NOISY: re-run quiet, never PASS. (iii) INFO: K8's walk sampled
     every 100 ms. If the machine is not quiet, B6 prints BLOCKED and Harmony decides whether the merge waits.
 B7  VISUAL WORK GATE: MACHINE (ctest): M-a, M-c (whole strip column byte-equal across showDeck 0 / 5 / 0, same
     objects, differing widths), M-e, the BF14 pin, G1, G2. LIVE: the ten states of P7 on FH, BEFORE on MAIN0 for (4),
     (6) and the plain grid; decoded and looked at. CRITICS: P7's five questions; PASS = every machine check AND five
     yes. A "no" returns the lane to FIX-3, never to Boris. Then ONE page for Boris (section 7).
 REVIEW: the one pinned round a7491d4..FH is APPROVE with no MUST.
 MERGE: Harmony merges lane/bf9b into main (H-1: no conflict expected).
 B8  POST-MERGE on main: B1, B2, B3, B3b; then the K batch on the merged build (same expected verdict line), H1, and
     A2-live once on an ASan build of main UNLESS `git diff --stat FH main -- src tests cmake CMakeLists.txt` is empty,
     in which case ASAN-FH's run stands and is cited.

## 6 DOCS
docs/claude/performance-controls.md: :44 drop "a strip badge" from the switch entries and add "a switch never rebuilds
 a strip or a tab, also between decks of different widths"; :50 "On screen" rewritten without the badge (tab dot, lit
 cell, lit header, "the strip shows the clip, never its deck" with Boris's sentence quoted verbatim); the Remove Deck
 line gains "Undo of Add / Load / Duplicate Deck follows the same rule: a deck a layer still plays from is retired, not
 erased; Redo brings the same deck back"; :65 Guards -> "dots, grid, no deck on the strip" + T6f-T6j +
 `.harmony/probe-asan-unit.sh` / `probe-asan-live.sh`; :72 neighbourhood: one line "K5 with Link on has no live driver;
 covered by T5, k5_queue_link_off and the B4i lint".
docs/claude/pitfalls.md: ONE new entry (number assigned by Harmony at the merge; 67 is the lane's, 68 is reserved for
 bf2): "A pointer into a Layer or a Clip dies when its storage moves or goes: after the mutation FORGET (forgetLayer /
 forgetClip / forgetConnection), never unbind -- bindConnection and setLayer(nullptr) read the old pointer; a routine's
 grip travels with the layer". CLAUDE.md: its one-line index entry (about 180 bytes). CLAUDE.md is 24,264 B at the pin
 (cap 25,000); the builder prints `wc -c CLAUDE.md` after the merge-in and after the line; if it would pass 25,000 the
 index line is cut to <= 110 bytes, the full text stays in pitfalls.md.
docs/claude/recording.md "Surfaces": only if P5 item 2's check finds the off-deck corner note still described.
.harmony/APP-INVENTORY.md: the LayerStrip row (no badge), the new probes. docs/claude/integration.md: no change (no new
 route).

## 7 WHAT ONLY BORIS CAN CHECK (one page, after B7; replaces the lane report's page steps 8.2 and 8.5, adds 8.12)
Open the test show "bf9b-check" (Harmony names the folder). Each step: do -> expect -> what wrong looks like.
8.1 (unchanged) Fire D1 C1 into Layer 1 and the clock video into Layer 3, go to Deck 3 and fire D3 C2 into Layer 2,
    then click through all 20 deck tabs and back. Expect: the picture never changes and the clock keeps counting; only
    the grid changes. Wrong: a picture changes, a flash, the clock jumps or stops.
8.2 (new text) Look at the layer strips on the left while you click through the tabs. Expect: they do not change at all
    -- no number, no mark for the deck -- and the tabs of Deck 1 and Deck 3 carry a small dot (a clip from that deck is
    playing). Click a dotted tab to see its clip lit in the grid. Deck 6 is wider than the others: going to it and back
    must not make the strips blink or jump. Wrong: a strip flickers, a fader moves, a number appears. Taste: is the dot
    worth keeping?
8.3, 8.4 (unchanged) Ignore Column; firing a column with an empty row.
8.5 (new text) On Deck 3 fire D3 C2 into Layer 2, click the Deck 1 tab, right-click the Deck 3 tab -> Remove Deck.
    Expect: D3 C2 keeps playing and its strip looks the same; the button at the end of the tab row reads: Undo Remove
    "Deck 3" -- Layer 2 keeps playing its clip. Cmd+Z: Deck 3 is back, D3 C2 still playing. Wrong: the right half goes
    black.
8.6-8.11 (unchanged) Undo skips deck clicks; the Layer tab; no "Fade:"; the old show's yellow note; save and reopen;
    a replaced video resumes.
8.12 (new) Select a layer, open the Layer tab, then File -> Load Deck and pick the deck file "five-rows" in the same
    folder (it has more rows than the show). Expect: two more layers appear, the Layer tab still shows your layer, the
    app keeps running. Press Cmd+Z: the two layers and the deck go away again. Wrong: the app quits, or the Layer tab
    shows nonsense. (The machine checks this too; this is the hands-on confirmation.)
NOT on the page: Cmd+Z of Load / Duplicate Deck while its clip plays (P2). Every way Boris fires a clip by hand is
    itself an Undo step, so Cmd+Z takes his clip back first; the case needs a routine, the autopilot or a queued fire.
    It is machine-checked (T6f-T6j, k9d) and asked as Q-B.

## 8 QUESTIONS FOR BORIS (each has a default; the build never waits)
Q-A (already asked; default KEEP) "Each deck tab shows a small dot while one of its clips is playing in a layer. Keep
    the dot?"
Q-B "You load or duplicate a deck, and a routine or the autopilot starts one of its clips. Then you press Cmd+Z and the
    deck is taken back. Should that clip keep playing (only the deck's tab goes away), the same as when you delete a
    deck -- or should it stop with the deck?" DEFAULT: it keeps playing.
Q-C "You have a clip selected, so the Clip tab shows its settings, and you click another deck tab. Should the Clip tab
    stay on the clip you were working on, or jump to the clip in the same spot of the new deck?" DEFAULT: it stays, as
    today (nothing changes in this build; your answer goes to a later one).

## 9 RISKS
R1 THE STRONGEST COUNTERARGUMENT: "this is too much new logic for a merge-day delta; ship the smallest P1 (one
   forgetLayer in repointLayerInspector), document P2 as an exception, file the rest." It loses because (i) the small P1
   leaves the same read at refreshUiAfterModelSwap and in the Clip inspector, which the ASan gate Harmony asks for
   would then report on the first load -- the gate cannot be both real and green without the class fix; (ii) P2's
   exception is a layer going black on one Cmd+Z, against the rule this lane exists for. But the counterargument is
   right about ONE piece: P2's wide-deck redo (rows put back from the snapshot). Cheapest refutation: T6g. If T6g is
   not RED-then-GREEN within FIX-2's context, fall back to F2-C (retire only when the deck added no layers; exception
   documented and pinned for the wide case) and say so in the report.
R2 F1-C can miss a site (an after-mutation re-point nobody listed). Cheapest test: A2-live -- ASan sees any holder, listed
   or not. If it reports one, switch to F1-B (always forget in setLayer / setClip), a three-line change.
R3 The ASan app is ASSUMED to build, start and exit without a crash dialog. Cheapest test: FIX-1's first step and the
   RED run on ASAN-PRE. If it fails, A2-live prints BLOCKED and the call-chain gate falls back to the Release live row
   gates-r2 proposed (Layer tab open, Load Deck of a wider deck, /api/health after 30 ticks) -- weaker (a Release app
   reads freed memory silently), said so.
R4 `Layer` may not be nothrow-movable. Cheapest test: the static_assert. A failure is a STOP, not a workaround.
R5 The grip rule releases a human hold mid-drag on a stack move; the slider then moves without a grip until mouse-up
   (the signal may fight the hand for that moment). Rare (a stack edit while a mouse button is down) and self-healing;
   A8 pins the rule. Its write to grip is on the message thread outside the fence, like every widget release today
   (INFERRED: the GL thread reads grip the same way it does now; no new race class, but not TSAN-proven).
R6 forgetLayers leaves blank strips if some caller does not rebuild the grid after a stack move. That is the intended
   trade (visible instead of undefined); A7 and B7's captures after Load Deck would show it. gates-r2 read every
   caller as rebuilding; I did not re-read all of them (INFERRED).
R7 P6's rebuildCells splits a function other paths rely on (selection prune, layout, bands). Cheapest test: G1 / G2 plus
   the existing DeckView cases; MU11.
R8 P4's "covered by" leaves Link's own library unexercised. That code is not compiled into the shipped app (F13); the
   tempo path it feeds is VERIFIED never to realign the phase (F13) and is driven live by k5_queue_tempo_feed. Cheapest
   refutation: that row RED at the final head. The literal bar stays BLOCKED until a Link lane exists.
R9 k9d's driver assumes a REST fire pushes no Undo step (INFERRED from both reviews). The row's VALID clause turns a
   wrong assumption into a loud FAIL, not a pass; the fallback driver is a queued (bar-snapped) fire, which the GL
   thread performs.
R10 MAIN0 as the RED arm (H-5) differs from STAGE_P (it still has Persistent and the deck fade): some K rows may be RED
   for another reason than the lane report's table. Listed per row, never re-thresholded.
R11 H1's exact strings are M3's; this plan pre-registers only its bar. If M3's row measures something else, M3's report
   wins and the difference is listed.
R12 B6 has never run (no driver until FIX-4) and needs a quiet machine Boris is using. It may print BLOCKED on merge
   day; the fallback evidence is the lane's reading (the pass is strictly cheaper: 19 off-screen deck ticks deleted) --
   INFERRED, not measured.

STATUS: DONE

## HARMONY ADOPTION (s-rta-1003, 2026-10-03 14:42:07) — overrides the ruling, which overrides the plan body
1. ADOPTED: .harmony/.reports/s-rta-1003/ruling-bf9b-merge.md IN FULL — amendments AM-1..AM-18, build stages FIX-1 -> FIX-2 ->
   FIX-3 -> FIX-4 (one builder context each, after M1 / M2 / M3, one worktree, this order), section 5's gate list (the only
   source of gate strings), section 6 (Boris page) and section 7 (Q-B, Q-C, Q-D: defaults until he answers).
2. ADDED to FIX-1 (the ruling's SF-1, which it offers for this lane): the hook function's "owned, or clear" check runs after
   EVERY fenced edit (one more call site in UndoService's hand-over), so a Clip inspector never keeps a clip that Layer >
   Clear Clips / Deck > Clear Clips destroyed. Its ASan case AS7 ("inspect a clip that has one effect, Clear Layer Clips,
   tickModulation") is in commit 1's RED run and GREEN at commit 2; B2 and EXPECTED_ASAN_CASES count it. Reason: it is the
   same freed-memory class, read at timer rate, on main today; a follow-up lane is days away.
3. ADDED to FIX-4 (the ruling's FM-6): the perf driver's teeth proof. A scratch Release build of the lane head with a 2 ms
   busy loop per deck in the composite pass must make .harmony/probe-boxes-perf.sh print "B6(ii) FAIL"; the mutant is never
   committed (git diff --quiet -- src tests after). If the machine is not quiet the driver prints BLOCKED and Harmony runs
   the proof herself. Without it a B6(ii) PASS is reported as "(metric sensitivity not proven)".
4. K5 with Link on: no waiver is written now. Fact for the gate-time decision: the app Boris runs is main's build, and its
   cache reads AUDIODNA_BUILD_LINK=OFF (Link code is not in his binary when OFF).
5. REVIEWS: ONE pinned round a7491d4..FH after FIX-4, four lenses (memory + commands; gates + probes incl. the by-name-quit
   sweep, H1, U1 and AM-10's six hunks by name; merge resolution = both sides' behaviour kept; screen = AM-11 / AM-12 and
   nothing left that shows a source deck); <= 1 fix round.
6. Boris's "restart" (answer 2) is NOT built here: K10 and contract C3 stay as built; the Boris page has no step 8.11.
7. Harmony constraint: BORIS USES THIS MACHINE AND THIS APP — an Audio-DNA a lane did not start is his: never quit / kill /
   touch it; the lock helper waits for it; every probe quits only the pid it launched. No Output window, no synthetic input,
   no full-screen capture. The ASan apps run with abort_on_error=0 and never leave a crash dialog on his screen; a dialog of
   ours is dismissed only by SIGTERM to that UserNotificationCenter pid. MERGE by Harmony.
8. (2026-10-03 14:55:32) Boris answered Q-B / Q-C: "Let's not allow control Z to change anything that is live in the layer strip. It changes
   anything else" and "yes it stays in clip tab regardless of deck". In THIS lane nothing changes in code (AM-7 as ruled:
   no regression — a trigger being an Undo step and the wide-Load-Deck exception are both main's behaviour today); the rule
   "Undo never changes what is live" is built in its own lane right after the merge (BF31), which re-registers T6h and the
   trigger-undo tests. The Boris page (ruling section 6) DROPS step 8.6 ("the D2 C1 you fired is undone") and the "NOT on
   the page" note about Q-B; step 8.5's Cmd+Z stays (it brings a deck back and changes nothing live).
9. (2026-10-03 14:58:10) OVERRIDE FOR FIX-3 — Boris (verbatim): "I don't wanna see an under removed button at all. We just use control Z.
   The only place that we will see undo remove, will be in the top edit menu." FIX-3 ALSO removes the "Undo Remove" button
   from the deck tab row entirely (ruling-bf9b amendment 16(d)'s button: DeckTabRow's undo-remove hint, its 10-second timer,
   its layout slot and its click). Its test cases are retired BY NAME and listed next to AM-12's five (the B2 "retired" list
   grows by exactly those names, nothing else); any probe row that asserts the button (probe-ui-files-rename reads an
   "undo" field of the tab row; probe-boxes k9 rows may read the hint) changes ONLY that clause, with Boris's quote at the
   row, and each changed row is listed. B7 state (3b), fact FM-7 and question Q-D fall away. What STAYS: Cmd+Z brings the
   deck back; the information sentence in the top text line (Removed deck "..." -- Layer N keeps playing its clip: AM-12) —
   it is not a control and does not say "Undo Remove". The Edit menu: after a Remove Deck its Undo item must read "Undo
   Remove Deck" (or the app's existing wording for that action). Verify at the head: if the Edit menu's Undo item already
   names the action, pin it with a test; if it does NOT name actions at all, do not redesign the menu — report it in
   stop_items_for_harmony. Docs lines and the Boris page (step 8.5) lose the button sentence. The docs grep of gate B4 also
   covers "Undo Remove" as a button (0 hits outside the Edit-menu wording).
10. (2026-10-03 14:59:28) OVERRIDE FOR FIX-3 — Boris (verbatim, asked whether the one-line sentence at the top after deleting a deck should
   go too): "yes remove the visible line. not needed". So item 9's "What STAYS: the information sentence" is WITHDRAWN:
   FIX-3 does NOT add AM-12's Remove Deck sentence to the file label, and removes any existing on-screen text that
   announces a removed deck or names the layers still playing it (ruling-bf9b amendment 16(d) in full: button AND text).
   After a Remove Deck the screen shows only the tab gone; a still-playing clip of that deck keeps playing and its strip
   looks like any playing clip. Cmd+Z / Edit > Undo bring the deck back. Tests of the sentence are retired BY NAME (listed
   with the others); probe rows that read it change only that clause, with this quote, each listed; the Boris page step
   8.5 reads: remove the deck -> the clip keeps playing, the tab is gone; Cmd+Z -> the tab is back, still playing. The
   "old show converted" load notice is a different text and STAYS (ruling-bf9b amendment 9(d)).
11. (2026-10-03 15:30:49) OVERRIDE FOR FIX-3 — Boris (verbatim): "We don't need any text indicating what has happened or what has happened.
   That is something that happens online and is not necessary in this application. It is extra overhead and bloat. Please
   remove it cleanly and completely." So item 10's last sentence ("the load notice STAYS") is WITHDRAWN. FIX-3 removes,
   CLEANLY AND COMPLETELY, every on-screen text this lane added that announces an event: the load-notice label of S3.4
   (ruling-bf9b amendment 9(d): "old show converted", "routine pads left empty", the deck-id refusal text), the Undo Remove
   button (item 9) and the Remove Deck sentence (item 10) — the widgets, their members, timers, layout slots, setters,
   call sites, the /api/debug/ui_text "load_notice" field, their tests (retired BY NAME, listed) and their docs lines. No
   dead member, no empty label left in the layout, no orphaned string. What STAYS: the behaviour behind each text (an old
   show still converts; a refused deck is still refused; Cmd+Z still brings a deck back) and the app-log lines (logLine:
   "old show converted:" exactly once per load — that is the machine-readable trace the gates read).
   GATE CHANGES BY THIS RULING (Harmony): K7 and B5 drop their load_notice clauses ("load_notice non-empty" / "load_notice
   empty after save + reload") and keep the logLine clause and every other clause; B7 loses state (9) (the load notice) and
   state (3b); the Boris page loses step 8.9's "a yellow note says it was converted" (it becomes: open "test with harry" ->
   your layers look as they did). Probe rows change ONLY those clauses, with this quote at each row, each listed.
   Texts that were in the app BEFORE this lane (on main at 5abdf01) are NOT touched here: list any you meet in
   found_not_fixed (file:line, the text) for the inventory Harmony owes Boris.
