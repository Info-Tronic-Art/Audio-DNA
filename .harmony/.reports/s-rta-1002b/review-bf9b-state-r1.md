# Reviewer Verdict - bf9b state r1 (lens: model split correctness)
STATUS: PARTIAL
VERDICT: FAIL (1 MUST)
PINNED: base 11820fa, head eef400c30dee24f487efacb19cff4b84b04b0e40 (src/ and tests/ byte-identical to e1cd314; the prebuilt build-lane binaries at 21:36 match the head src)

## Answer
The model split is correct on every point of the focus list except one pointer-lifetime regression: loading (or inserting) a deck with more rows than the show has layers grows the shared `Composition::layers` vector while the Layer inspector keeps a raw `Layer*` into it. That path is new in bf9b and nothing re-points the inspector after it.

## MUST
M1. UAF on Load Deck / Duplicate Deck of a deck wider than the show (plan F6, ruling amendment 8, test M3 itself).
 - InsertDeckCmd::execute adds shared layers with `comp->insertLayer(...)` (src/core/DeckCommands.h:~812-816) -> `Composition::layers.insert` (src/model/Composition.h insertLayer).
 - VERIFIED: a std::vector<Layer> of 3 (capacity 4 after 3 push_back, run on this toolchain: /private/tmp/claude-501/rv/cap.cpp) moves its storage at the 2nd added layer, i.e. a 5-row deck into the default 3-layer show (the M3 case) invalidates every Layer*.
 - VERIFIED: LayerInspector keeps `layer_` (src/ui/LayerInspector.cpp:731 setLayer; set by onLayerSelected, MainComponent.cpp:726-729, whenever Boris clicks a layer strip) and EffectStackView::effects_ points into layer->layerEffects. InspectorPanel::tickModulation runs EVERY timer tick regardless of tab (src/ui/InspectorPanel.cpp:170-178, MainComponent.cpp:4186) and refresh() runs at ~10 Hz for the Layer tab (MainComponent.cpp:2946 comment says exactly this hazard).
 - VERIFIED: the do-path landing in finishStagedLoad (MainComponent.cpp:~3300-3315) pushes the InsertDeckCmd and calls only `deckView_->rebuildGrid()`; it neither nulls nor re-points the inspectors. Only undo/redo (refreshAfterUndoRedo, :5287-5293) and removeDeck (:3831) re-point/null.
 - Why bf9b owns it: before bf9b a Layer lived inside its Deck, so a deck append (decks.push_back) moved Deck objects but never a Layer's address. Moving Layer into one shared vector (F1) made every layer add a hazard for every Layer*; F6 added a new, default-path producer (Load Deck with extra rows). Add Layer (menu) had the same shape before and still does.
 - Fix: after the InsertDeckCmd lands with addedLayerCount() > 0 (and in the AddLayer handler), null or re-point the inspectors exactly as refreshAfterUndoRedo does (resolveLayer(selLayer)); or have LayerInspector re-resolve by its EffectScope index each tick. Add a headless test: select a layer in a LayerInspector bound to comp.layers[i], run InsertDeckCmd with a 5-row deck, assert the inspector holds comp.getLayer(i) or null.
 - Confidence: no-re-point and the pointer ownership are VERIFIED by reading; the crash itself was not reproduced in the app (INFERRED).

## SHOULD
S1. Adoption item 2 / H1 (acceptance row m9b_deck_switch_live, 20 decks, MilkDrop on deck 0 L0) is recorded only in S2b deviation 11 (.harmony/.reports/s-rta-1002b/bf9b.md:970). It is absent from the S4 found_not_fixed list (:1693-1701) and the Resume point (:1710-1717). bf10 has since merged to main (be23460), so Harmony can now add it to probe-milkdrop.py; carry it into the resume point so it is not lost at the rebase.
S2. probe-canvas f2_deck_transition is RED on the BF9B arm (reported as the STOP item, bf9b.md:1607). Harmony must rule (proposed RETIRED) before merge; also .harmony/probe-canvas.json:20 `_why` still names compositeDeck.
S3. Rebase hazards the builder already named (bf9b.md:1714-1716): main now owns Pitfall 64-66 (the lane's "NN" must become 67+) and main has its own /api/debug/undo route, while the lane adds /api/debug/undo (src/api/ApiServer.cpp:~323). A duplicate Post registration is silent in httplib (first wins), so check which handler survives and re-run probe-boxes K9 after the rebase.

## NIT
N1. removeDeck nulls the Layer inspector (MainComponent.cpp:3829-3832) although shared layers are untouched by a deck removal: the Layer tab goes blank while the strip stays selected. Only the Clip inspector needs nulling now.
N2. DRY / invariant by hand: "pending cleared" = three assignments (column -1, deck kNoDeck, snap Off) repeated at Layer.h (immediateNext, clearedNext, releaseMomentary) and DeckCommands.h:~202, :259, :278. The invariant "column -1 iff deckId kNoDeck" decides tuple equality (changed(), undo exactness); a helper (Layer::withoutPending(r)) would make a missed site impossible. All current sites pair correctly (grep-verified).
N3. Composition.h insertLayer / eraseLayer / moveLayer each repeat a live-then-retired lambda pair; forEachClip has a const and non-const copy. A tiny forEachDeck(fn) would remove four copies.
N4. A queued (pending) ref into a deck that Add/Insert Deck UNDO erases (not retires) is never cancelled: processPendingTrigger returns the tuple unchanged forever because rows.hasCell() is false (Layer.h processPendingTrigger). Reachable only via a non-undoable fire (routine/REST/OSC) from a freshly inserted deck; harmless but leaves a stuck queued state. INFERRED.

## Verified clean (focus list)
- Ownership: Composition::layers (settings + tuple, no clips) / Deck::rows (ClipRow, clips only); every deck and retired deck kept at rows == layers (padRows, normalizeRows, insertLayer, eraseLayer, moveLayer); Deck nothrow-move static_assert kept, retire moves buffers so Clip addresses stay.
- ClipRef packing (src/model/Layer.h:104-190, ClipRef.h): active/previous = deck<<16 | uint16 column; pending = deck<<14 | (col+1) under the 4-bit snap; 0xFFFF / 0x3FFF = no deck; kMaxDeckId 0x3FFE, kMaxColumn 0x3FFE; out of range refused at every tuple-writing entry (triggerClip, triggerClipImmediate, releaseMomentary, clearActiveClip, fire, triggerColumn) and jassert in pack. test_layer_runtime 17 cases, test_show_model 28 cases pass (executed from build-lane binaries).
- Id rules: ids never reused (appendDeck refuses past kMaxDeckId; load renumbers when any id > kMaxDeckId or max >= 8192; duplicates re-minted); duplicateDeck copies rows, id 0 re-minted by appendDeck; retired decks reaped inside every withDeckDetached after `mutation` and before the restore, media disposed after the fence (UndoService.cpp:70-112).
- Old-file conversion (ShowMigration.h): first deck with row i wins, ids re-minted, one note only when settings differ, "persistent" / "globalTransitionSpeed" read only there (B4a grep over src: only those 2 literals). Round trip M2 passes; Load Deck decides per row by "type".
- Undo exactness S2c: SwitchDeckCmd gone (grep: only lint comments), onDeckSwitched body is `handleDeckSwitch(deckIdx);`, handleDeckSwitch does index + fence token + showDeck + take capture only.
- Thread ownership: GL thread only reads layers / rows / retired decks inside the deckActive gate (Renderer.cpp:388-392; pointer never dereferenced; zero getActiveDeck( / activeDeckIndex in render/* and Autopilot.cpp); no new mutex, lock, allocation or syscall on the audio callback or render path (diff grep); activeDeckIndex is RelaxedInt.
- C1 (playing / playingClip), C2 (forEachLayer / forEachClip, ClipSite), C3 (syncActivatedPlayhead called from handleClipTrigger and handleColumnTrigger) match the contract names and shapes.
- Persistent: no identifier left in src (lint B4e allow-lists ShowMigration.h only).
- Stray: no .venv, no mutant, no instrumentation; the only env hook is ADNA_INSPECT_LAYER (MainComponent.cpp:2326), the planned S0 lever, gated by AUDIODNA_TEST_SERVER and inert unset. CLAUDE.md 24,264 B (<= 25,000), rule 15 verbatim.
- Fences: edits to VideoPlayer, ApiServer, DeckView/DeckTabRow, TopBar are all in plan 4.C / ruling amendment 26.

METADATA: reviewer=claude-sonnet-5-5, builder_packet=bf9b, date=2026-10-02

CORRECTION to S3: "a duplicate Post registration is silent in httplib (first wins)" is INFERRED from httplib's registration-order matching, not executed; line numbers cited for finishStagedLoad / ApiServer are approximate (+/- 5) at head eef400c.
