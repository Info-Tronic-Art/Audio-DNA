# Reviewer Verdict - bf9b state r2
STATUS: DONE
VERDICT: APPROVE (PASS_WITH_NITS; no MUST)
PINNED: lane/bf9b head a7491d4d633efc3db44d9a9093d7702fbbcf6ecd, base 11820fa; read via git show / git diff only.
FILES: model (ClipRef.h, ClipRow.h, Deck.h, Composition.h, Layer.h/.cpp, ShowMigration.h), core (UndoService, DeckCommands.h,
  TriggerCommands.h, CompositionLoad.h), render (Renderer.cpp, CompositorEngine.cpp, LayerClock.h, LayerStateKey.h),
  Autopilot.cpp, recording (Program, PerfState*, RoutineSlice), ApiServer.cpp, MainComponent.cpp diff, DeckView.cpp, CLAUDE.md.

## Result (sentence one)
The model split is correct and delivered as planned (ownership, ClipRef packing, retired decks and id rules, save / load /
old-file conversion, S2c undo, C1 / C2 / C3, no Persistent left, no stray files). One SHOULD matters most: the round-1 MUST
fix still reads freed memory once, and the lane's own new test trips AddressSanitizer on it.

## What I verified (labels: VERIFIED = read or run by me; INFERRED = reasoned)
- ClipRef packing (VERIFIED, src/model/Layer.h:93-175, ClipRef.h): active / previous = deck<<16 | uint16(column), none = 0xFFFF
  deck; pending = 14-bit deck + 14-bit (column+1) under the 4-bit snap; word stays 16 B, lock-free static_assert kept; every
  tuple-writing entry refuses a ref that is not storable (Layer.h triggerClip / triggerClipImmediate / clearActiveClip /
  releaseMomentary, Composition.h fire / triggerColumn). Deck ids <= 16382 minted, never reused (Composition.h appendDeck /
  canMintDeckId; load renumbers at >= 8192, Composition.h fromVar). test_show_model (29 cases) and test_layer_runtime
  (17 cases) re-run green on the lane's build-lane binaries.
- Ownership (VERIFIED): Composition owns layers + decks + retiredDecks_; Deck owns rows only; rows == layers kept by
  insertLayer / eraseLayer / moveLayer / padRows / normalizeRows over live AND retired decks. Retire = Deck move (nothrow
  static_assert at Deck.h end), every Clip keeps its address. deckIsPlaying counts a previous ref only while the fade runs.
  Every structure writer sits in a fence (UndoService::withDeckDetached; lint B4f pins the sites); reap runs inside every fence.
- Thread ownership (VERIFIED): the GL thread never dereferences activeDeck_.view().ptr (Renderer.cpp:388-393; no
  getActiveDeck / activeDeckIndex left in src/render or Autopilot.cpp); REST reads findDeckIndexById, never the retired
  elements. GL history keys use kShowStackKey (LayerStateKey.h). Real-time rules: nothing added on the audio callback; no
  new mutex (git grep over the diff).
- Save / load (VERIFIED by read, M2 test exists): saved shape = top-level "layers" + decks with "clips"-only rows; legacy
  detection = no top-level "layers" (ShowMigration.h:21-25); first-deck-wins conversion, ids de-duplicated, ONE note only when
  something was dropped; "persistent" / "globalTransitionSpeed" read only in ShowMigration.h (git grep: zero other hits).
- Undo (VERIFIED): TriggerClipCmd addressed by (layer index, ClipRef); RemoveLayerCmd snapshots rows by deck id; deck switch is
  not an Undo step (MainComponent.cpp deckView_->onDeckSwitched = handleDeckSwitch; lint B4g).
- C1 / C2 / C3 (VERIFIED): Composition::playing / playingClip (+const), forEachLayer / forEachClip with ClipSite (+const),
  MainComponent::syncActivatedPlayhead called from handleClipTrigger and handleColumnTrigger; topLayerIndex, Deck::getRow,
  ClipRow::getClipAt as 4.A names them.
- Stray (VERIFIED): no symlink in git ls-tree, no .venv, no mutant left, no instrumentation; the only getenv is the
  sanctioned test-server-only ADNA_INSPECT_LAYER lever (C0, MainComponent.cpp ~2330). CLAUDE.md 24,264 B (cap 25,000).
- Fences (VERIFIED): VideoPlayer::advanceClock deletion is ruling-bf9b amendment 26; the lane did not touch bf2 / ui
  surfaces beyond the sanctioned list; ApiServer /api/debug/undo duplication and Pitfall NN -> 67 are carried (R-S3).

## Findings
1. SHOULD (highest) - the MUST fix leaves a one-shot use-after-free read, reproduced under ASan.
   src/core/UndoService.cpp:61-70 hands onLayerStackMoved to MainComponent::repointLayerInspector (MainComponent.cpp:5304)
   AFTER the mutation freed the old Composition::layers buffer. LayerInspector::setLayer (src/ui/LayerInspector.cpp:728-741)
   calls syncFromLayer (:734) BEFORE bindScalarControls (:738); syncFromLayer reaches UniversalParamControl::setParamValue
   (UniversalParamControl.cpp:349, routineHandHolds) which reads the control's OLD conn_ (a pointer into the freed Layer), and
   bindConnection (UniversalParamControl.cpp:126) then reads / may release() it. VERIFIED: I built test_show_model from
   a7491d4 with -DADNA_SANITIZE=address in a $TMPDIR copy and ran the lane's new case: heap-use-after-free READ at
   UniversalParamControl.cpp:349 <- LayerInspector.cpp:911 <- :734 <- UndoService.cpp:78 <- tests/test_show_model.cpp:1490
   (freed by Composition::insertLayer, Composition.h:484, via InsertDeckCmd DeckCommands.h:806). The other 28 cases are clean.
   The read is benign unless a routine hand (Hand::Lane) grips the inspected layer's scalar: then bindConnection calls
   conn_->release() on freed memory (INFERRED from the code, not run). The same stale-conn_ read already exists at base in
   MainComponent.cpp:2958 (refreshUiAfterModelSwap setLayer(nullptr) after a swap) and in the Remove-Layer-undo repoint, so
   this is not new to the lane; but the lane report says the MUST is FIXED and it is not ASan-clean.
   -> Fix: unbind BEFORE the mutation (a pre-fence hook, or call getLayerInspector().setLayer(nullptr) before
   withDeckDetached's mutation when the stack may move), or make LayerInspector::setLayer / UniversalParamControl::bindConnection
   drop the old conn_ without dereferencing it on a storage move; add an ASan run of this one case to the lane's gates.
2. SHOULD - Undo of Add Deck / Load Deck / Duplicate Deck erases the deck even when a layer plays from it.
   DeckCommands.h:717 (AddDeckCmd::undo) and :824 (InsertDeckCmd::undo) do decks.erase + dispose; they never retire. A layer
   whose active ref names that deck keeps a tuple that names a deck that no longer exists: it draws nothing and the strip is
   empty, which is the "what is in the layer is what plays" gap Q1 closed for Remove Deck. Reachable when the clip was fired
   from the new deck by a non-undoable source (a routine, MIDI momentary, GL queued fire) before Cmd+Z; a human trigger is
   undone first and restores the tuple. INFERRED from the code (not executed). plan R9 covers only "undo of a trigger naming a
   reaped deck". -> Use the same retire-or-erase (Composition::retireOrEraseDeck) in both undos, or state the exception in
   performance-controls.md and pin it in T6.
3. NIT (EXCESS_DEAD, DEBT_FILED) - Composition::crossfaderBlendMode (src/model/Composition.h:101-102, :753, :912) lost its only
   reader (the deck-fade uniform at the old Renderer.cpp:947) in this diff; it now only round-trips through the file, like the
   already-unread crossfaderPhase / crossfaderBehaviour. Not under hooks/ or config wiring (model field; harness set checked:
   no settings.json / config reference). Keep for file back-compat, but list it with the siblings.
4. NIT (EXCESS_DEAD, JUSTIFIED_KEEP as a guard) - src/core/CompositionLoad.h:63-64 `if (c.layers.empty()) return ...` is
   unreachable: decks non-empty (checked first) and every deck has >= 1 row (validateDeck) so normalizeRows leaves >= 1 layer.
5. NIT - ApiServer retiredDeckCount (ApiServer.cpp handleComposition, composition_.getNumRetiredDecks()) reads
   retiredDecks_.size() from the httplib thread while the message thread may push_back inside a fence; the comment above it says
   REST "never reads a retired deck". The lane records it (S2a deviation 17) as tsan-r5's class; make the comment say "except its
   size".
6. NIT - probe-quit-ours.sh quit_ours() (.harmony/probe-quit-ours.sh:33-49) checks "only OURPID running" then osascript-quits by
   name; a TOCTOU window if Boris starts his app in between. probe-video.sh:70-72 still quits by name (outside this lane's
   change; R-N1 carry). Prefer kill -TERM of OURPID only.
7. NIT - DeckView::showDeck (DeckView.cpp, refresh path) keeps selectedCells_ and the Clip inspector on the previous deck's
   Clip* across a switch (selection highlight names the new deck's cell, the inspector edits the old deck's clip). Same as base
   (base handleDeckSwitch did not re-point it either), so not a regression; worth a line in the Boris page.

## Carried to the rebase lane (not defects here; all in the lane report)
R-S1 (H1 m9b_deck_switch_live row), R-S3 (Pitfall NN -> 67; keep ONE /api/debug/undo), R-N1 (every by-name quitter), K5 with
Link on (no driver; the verdict line now prints BLOCKED, rc 3).

SUMMARY: ~30 src / test files read + one ASan build and run; 7 findings (0 blocking, 2 SHOULD, 5 NIT). Confidence: VERIFIED for
packing, ownership, fences, conversion, contracts, stray / size checks and the ASan hit; INFERRED for findings 2 (code read) and
for the routine-held write in finding 1.
METADATA: reviewer=claude-sonnet-5-5, builder_packet=bf9b (round 2, lens state), date=2026-10-02
