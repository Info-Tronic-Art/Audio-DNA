# Review bf9b-merge, lens MEMORY + COMMANDS, round 1
STATUS: DONE
VERDICT: PASS_WITH_NITS (0 MUST)
PIN: lane/bf9b head 4137f6043eb7d177626b73dcd0d60d6658f4da0d (base a7491d4; merge 98d71fe reviewed only for its resolution)
METHOD: read through git only (diff / show / grep at the pinned head). Nothing built, run or launched. Every claim below is
VERIFIED (read by me at the head) unless labelled INFERRED.

## Verdict sentence
No MUST. AM-1, AM-2, AM-3, AM-4, AM-5, AM-6, AM-7, AM-8 and adoption item 2 are implemented as ruled; each memory case has a
recorded RED arm; the hook function clears an unowned Clip inspector after every fenced edit. Three SHOULD/NIT items below.

## Ruling conformance (item by item)
- AM-1 VERIFIED: LayerInspector::setLayer (LayerInspector.cpp:730) and ClipInspector::setClip (ClipInspector.cpp:814) call
  forgetScalarBindings() as their FIRST statement (7 / 6 forgetConnection()); bindConnection unchanged
  (UniversalParamControl.cpp:126). After forgetConnection conn_ is null, so syncFromLayer's setParamValue
  (UniversalParamControl.cpp:342, routineHandHolds() reads conn_) and bindConnection's implicit release (conn_ null) read nothing
  old. forgetLayer / forgetClip / forgetLayers / repointClipInspector do not exist at the head (git grep: 0 hits) - the
  withdrawn plan items were not built.
- AM-2 VERIFIED: src/ui/InspectorRepoint.h: compositionOwnsClip (address walk over Composition::forEachClip const, which walks live
  AND retired decks, Composition.h:403-416; never dereferences the Clip); repointInspectorsAfterStackMove clears an unowned Clip
  inspector first, then re-points the Layer inspector by the selected row (getLayer is bounds-checked, Composition.h:234-239).
  MainComponent.cpp:1801-1810 are the two one-line adapters.
- AM-3/AM-8 VERIFIED: lint B4h (test_render_thread_lint.cpp:376-434) pins exactly one onLayerStackMoved = whose statement holds
  repointInspectorsAfterStackMove( and getSelectedLayerIndex(), exactly one onFencedEdit = with clearClipInspectorIfUnowned(,
  the first statement of both setters, 7 / 6 forgetConnection(), and the hand-over lambda (one call each, handOver(); twice).
- AM-4/AM-5 VERIFIED: AS0 (unedited), AS1, AS2, AS3, AS3b, AS4, AS5, AS6, AS7, N1 exist (test_show_model.cpp:1649-2030). AS1 / AS2
  REQUIRE capacity < 5 after shrink_to_fit and &layers[1] != before; AS3b / AS6 / AS7 REQUIRE the clip died (address walk). tests/
  CMakeLists.txt:3295-3299 + 3512-3517: ADNA_ASAN_TEST_PROPERTIES, second catch_discover_tests inside if("address" IN_LIST
  ADNA_SANITIZE) - a Release build's count is unchanged. probe-asan-unit.sh: EXPECTED_ASAN_CASES=10 == the 10 [asan] tags I counted
  (AS0, AS1, AS2, AS3b, AS5, AS6, AS7, T6f, T6g, T6j); fail-closed exit 3 below it; last-line verdicts as ruled.
- AM-6 VERIFIED (script + src): ui_text gains inspected_layer / inspected_clip / inspector_tab read in the message-thread hop
  (ApiServer.cpp:2081-2108); the ADNA_INSPECT_LAYER lever also calls selectLayer(n) and sits under #if AUDIODNA_TEST_SERVER
  (MainComponent.cpp:2437-2455). probe-asan-live.sh: lock gate, refuse_foreign_start, record_ourpid right after launch, quit_ours
  only; L0-L6 as ruled; verdict lines as ruled. src of FIX-4 == src of FIX-3 (report: git diff 7decfaa HEAD -- src empty).
- AM-7 VERIFIED: DeckCommands.h AddDeckCmd undo = cancelPendingInto + retireOrEraseDeck, redo = restoreRetiredDeck first
  (DeckCommands.h:688-696, 722-730); InsertDeckCmd undo splits on addedLayers_.empty() (retire path returns before the dispose
  loop; the exception path keeps today's erase + eraseLayer + dispose), redo restores a retired deck first only when no layers
  were added (DeckCommands.h:788-800, 843-875). Symmetric with RemoveDeckCmd (execute :935-955, undo :960-985). T6f/g/h/i/j exist.
- Adoption item 2 VERIFIED: UndoService.cpp:64-75 `else if (onFencedEdit) onFencedEdit();` runs on both exits of withDeckDetached;
  hooks run AFTER the ActiveDeckRestoreGuard scope (fence ended) and after onDecksReaped, on the calling (message) thread; the
  reaped Decks are still alive in the local `reaped` while the hook walks addresses.
- Real-time rules VERIFIED: no file under src/audio or src/render changed after the merge; the only hot-path-adjacent change is
  ProjectMSource's three relaxed fetch_add counters (ProjectMSource.cpp:77, 95, 215; no lock, no wait; the mutex there is
  pre-existing). No new mutex; the new std::function hooks are message-thread only. Shared model fields: none added.

## Holders of a Layer*, a Clip* or a pointer into one, at the head (git grep over src, VERIFIED)
Raw members: ClipCell::clip_ (ClipCell.h:115), ClipInspector::clip_ (ClipInspector.h:114), LayerInspector::layer_
(LayerInspector.h:76), LayerStrip::layer_ (LayerStrip.h:130); sub-pointers: EffectStackView::effects_ (EffectStackView.h:145),
UniversalParamControl::conn_ / live_ (UniversalParamControl.h:202-203) in the inspectors' scalar controls, effect rows and
sourceParamControls_. No other `Layer*` / `Clip*` / `ParamConnection*` / `LiveValue*` member exists in src/.
- ClipInspector / LayerInspector (+ their scalar controls, effect rows, source-param rows): covered by AM-1 + the hook.
- LayerStrip::layer_ / ClipCell::clip_: not forgotten (RR-6, accepted); every caller of a stack-moving fenced edit that I read
  rebuilds in the same message-thread turn (kLayerNew/kLayerRemove MainComponent.cpp:6832/6853, Clear Clips :6811/6896, refreshAfterUndoRedo
  :5360-5364, model swap :3091-3095); the live ASan row runs Load / Duplicate / Remove Deck + Undo / Redo + Load Composition with
  strips alive, GREEN x3 (report).
- Hook coverage: every withDeckDetached caller goes through the same hand-over (git grep: MainComponent.cpp only; none from the
  http thread), so Add/Remove Layer, Load/Duplicate Deck and their undo/redo, Clear Clips, column add/remove, cell clear, model swap
  all reach one of the two hooks.

## Findings
(see structured output; summary)
1. SHOULD - ASAN-LIVE passes only because the probe avoids a real race: GET /api/composition (ApiServer.cpp:488 reader, http thread,
   no lock) raced Duplicate Deck's push_back and produced a container-overflow on the UNMUTATED ASan app (report run fh2, STOP ITEM 1 / SF-12).
   The gate is timing-shaped (1.0 s sleep + one read; re-reads at 1 s steps when a VALID clause is not yet true, script lines ~170-195), so
   it can print a false RED on a pre-existing main bug, and its reads of retiredDeckCount / layers[1].activeClip still go through that reader.
2. SHOULD - probe-asan-live.sh waits for staged loads on three of main's on-screen event texts ("Loaded deck:", "Duplicated deck:",
   "Loaded: bf9b-check", via ui_text.file_label). Boris's adopted instruction (adoption item 11) is to drop event texts app-wide; the day any is
   removed the wait times out (15 s / 40 s) and the probe falls back to polling the racy reader of item 1 (STOP ITEM 4). Wait on a model fact
   (a message-thread hop, like ui_text) instead.
3. NIT (INFERRED, not reproduced; pre-existing path) - the "owned or clear" predicate is by clip ADDRESS, so a fenced edit that keeps the
   address but replaces a sub-vector leaves the inspector's source-param rows pointing at the old buffer: Clip::replaceContent does
   `sourceParams = newContent.sourceParams` (Clip.h:~286) inside withDeckDetached (MainComponent.cpp:7173), then only
   inspectorPanel_->refresh() (:7190). Realistic reach is small (a file replaces a Source clip: empty copy-assign keeps the buffer) but
   the ruling does not list this class.
4. NIT - withDeckDetached's exceptional exit skips handOver() (UndoService.cpp:77-84 returns only on the normal paths), so a throwing
   mutation also skips the reap hand-over and both inspector hooks; the file's own FenceResetGuard comment calls a throw "real".
5. NIT - test gaps: AddDeckCmd redo after the retired deck was REAPED (snapshot branch) and undo -> redo -> undo on the retire path have
   no case (T6i covers only InsertDeckCmd's reap-then-redo). Reasoning says both are fine (linear history keeps addedIndex_ valid).
6. SLIM, JUSTIFIED_KEEP reason="ruling AM-6 mandates the field; probe-asan-live.sh reads it into facts and STOP ITEM 2 proposes the
   Clip-inspector step that asserts it" - ui_text.inspected_clip (ApiServer.cpp:2107, MainComponent.cpp:2161) is emitted and asserted by
   nothing today (EXCESS_VESTIGIAL candidate).

Also checked, no finding: forget-first leaves no grip stuck in an ordinary path (N1 pins routine Lane grip and Decaying touch; a Held
human grip needs a mouse button down on that slider, RR-1 reasoning; selection entries are strip/cell click, drop handlers, Undo/Redo,
hook, model swap, Remove Deck - git grep of inspectClip/inspectLayer/onLayerSelected, only the test-server lever is extra);
the test-side showHoldsClipAt duplicates compositionOwnsClip on purpose (independent oracle).
