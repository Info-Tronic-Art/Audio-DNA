# PLAN lane bf9 -- RESEARCH INPUT for bf9b + Stage P "Persistent removed" (the only buildable part)

Architect (Fable), 2026-10-02 ~14:55, repo /Users/boriskarpman/projects/RealTimeAudio, main HEAD 5e47d17.
Every code claim cites file:line read this session (VERIFIED) or is marked INFERRED / ASSUMED.

## 0. READ FIRST -- the dispatch premise was overtaken while this plan was being written

- Dispatch premise (Boris 14:30 answer): "deck change stops the old deck; Persistent removed; ignore-column stays".
- 14:44:36 Boris clarified (BORIS_DECISIONS.md:350-356, verbatim): "when I switch between decks, do not change the clips
  playing in the layers or how they are playing. treat the decks as just a box of clips and I can switch between 20 decks
  looking for a clip and the playing will not be affected. does that make sense?" Harmony recorded
  (.harmony/boris-feedback-backlog.md, last entry): "The running plan-bf9 (premise 'deck change stops the old deck') is
  SUPERSEDED -> re-planned as bf9b; plan-bf9.md is research input only." binding-decisions.md "BF9 CLARIFIED": the
  come-back follow-up (resume vs empty) is moot.
- Therefore this file:
  - DOES NOT plan (and nobody may build from it): stopping / pausing / freezing the old deck, a "what a deck shows when
    you come back" policy point, stopping routines on a deck change, a single-autopilot rework, or a Rule 15 rewrite to
    "a deck off screen is paused". Under "decks are boxes of clips" those are wrong (the playing layers must not be
    touched by a deck switch). They are listed once in section 4 "NOT TO BUILD".
  - PLANS Stage P (premise-independent, ships alone): remove Persistent end to end and prove Ignore Column Trigger
    untouched. Both Boris messages keep it ("I want to remove the persistent. The only thing remotely persistent should
    be to ignore column controls"; bf9b record: "Persistent removed (redundant); ignore-column stays"). Offered to bf9b
    as its Stage 0, or to ship alone in bf9's build slot (first).
  - HANDS bf9b a VERIFIED map (sections 2.3-2.6): what runs per deck today, every deck-switch path, and the per-deck
    couplings a shared playing stack must break. Facts, not a bf9b design.

## 1. GOAL

Stage P: the Persistent layer feature no longer exists anywhere (model, file format, compositor, inspector, REST, tests,
probes, docs); shows saved with it load cleanly with the flag ignored; Ignore Column Trigger keeps exactly its behaviour
and becomes the only "keep this layer" control.
Boris (2026-10-02, verbatim): "I'm going back on what I asked for before and I want to remove the persistent. The only
thing remotely persistent should be to ignore column controls so if we want to have something stay on a certain layer
and we want to play with the other layers in the composition, that stays in the other layers can be switched by a
column switch."
Not Stage P's goal (bf9b's): "Whatever is in the layer should be what is playing and there should be nothing else" /
"treat the decks as just a box of clips".

## 2. ESTABLISHED FACTS

### 2.1 Persistent, end to end (all VERIFIED)
- Model: `bool persistent` Layer.h:182; `static constexpr bool canBePersistent(Type)` Layer.h:161-172 (Opaque /
  Transparent / FXOnly). Comments that use the word for the blend list: Layer.h:186, :191.
- File format: toVar writes "persistent" Layer.cpp:39; fromVar reads it unconditionally Layer.cpp:156.
- Render: CompositorEngine::compositePersistentLayers decl CompositorEngine.h:150-161, def CompositorEngine.cpp:1306-1439
  (fade tick, renderLayerStages, keying, blend; Opaque blends over via the alpha key; Mask / 3D skipped);
  hasPersistentContent h:165-169 / cpp:1266-1283; beginEmptyActiveDeck h:171-176 / cpp:1285-1304. Renderer.cpp:739-766:
  empty active deck + another deck's persistent content -> beginEmptyActiveDeck, then compositePersistentLayers for every
  other deck. DeckClock.h:30-31, :35-36: the off-screen tick skips persistent layers ("owned elsewhere").
  Comments naming it: CompositorEngine.h:185-186, :327-330, :484-487, :529-541; CompositorEngine.cpp:1139-1141;
  Renderer.cpp:691-695, :768-772, :791, :799-801; Renderer.h:130, :366-367, :585; LayerClock.h:6; DeckClock.h:12-14;
  LayerStateKey.h:9-14.
- UI: LayerInspector.h:94 `persistentToggle_{"Persistent"}`; LayerInspector.cpp:155-162 (construct; tooltip "Keep this
  layer rendering when switching to another deck"; onClick writes layer_->persistent), :638 (LEFT half of the row),
  :922-945 (sync, enable rule `canBePersistent || persistent`, three tooltips).
- REST: ApiServer.cpp:428 (`/api/composition` layer field). No setter on 7070 / 8080, no OSC / MIDI / binding / undo
  command / recorder lane: a grep of src/ finds no other use (VERIFIED).
- Tests: tests/test_layer_inspector_persistent_toggle.cpp (target tests/CMakeLists.txt:2199-2247; named in a comment at
  :3086-3087); test_compositor.cpp:423-437 (canBePersistent); test_deck_clock.cpp:71-102 case (b) + header :12-16;
  test_undo_commands.cpp:188 (Layer operator==); test_layer_state_key.cpp:10, :71-72 (comments);
  test_render_thread_lint.cpp:58-66 + pins :81-86 (CompositorEngine.cpp runtime() 2 / getActiveClip( 2 -- one of each
  is the persistent path); tool_uitoggle_snapshot.cpp:122-165 (three LayerInspector shots set layer.persistent).
- Probes: probe-render-state.py -- 14 rows build a persistent layer on another deck: r2_temporal r2_ring
  r4_clip_transform r4_clip_opacity r4_layer_effects r4_layer_transform r4_feedback r4_transition r4_opaque_opacity
  r4_opaque_overlay r4_fxonly_persistent r4_fxonly_medialess r4_mask_skipped r4_empty_active_deck (row table :640-656,
  helper persist_setup :240-245, fixture keys r2 / r4 / r4_transition / r4_opaque / r4_fxonly / r4_empty_active_deck in
  probe-render-state.json, .sh header :3-8). probe-deck-clock.py d_persistent_single_advance (:22-24, :256-271, :415;
  probe-deck-clock.json :10-11; probe-deck-clock.sh :5). tol = 1.5 (probe-render-state.json "tol").
- Docs: performance-controls.md:16, :42-47, :49 (three clauses); pitfalls.md:115 (Pitfall 53 parenthetical);
  CLAUDE.md:14 ("persistent layers across deck switches, "), :248 ("persistent layers, "); APP-INVENTORY.md:73
  ("Layer (master/persistent/ignore-column)"); .harmony/FEATURES.md:355, :539, :838, :877, :2434, :2694.
- No checked-in composition outside probe fixtures / evidence folders carries the key (grep *.json, VERIFIED).

### 2.2 Ignore Column Trigger, end to end (VERIFIED) -- must not change
- Field Layer.h:181; file format Layer.cpp:38 / :155.
- Behaviour: Deck::triggerColumn skips the layer (Deck.h:113-125); handleColumnTrigger mirrors the skip for undo
  (MainComponent.cpp:4842-4853).
- UI: LayerInspector.h:95; LayerInspector.cpp:164-171 (tooltip "This layer ignores column trigger buttons"), :639 (RIGHT
  half of the row today), :946 (sync).
- Existing guards: test_undo_commands.cpp:2421-2430, :2794. Not reported by /api/composition (ApiServer.cpp:416-430).
- REST POST /api/trigger_column {"column": N} (ApiServer.cpp:516-536) drives a column trigger live.

### 2.3 What runs for a deck that is NOT on screen at HEAD (VERIFIED) -- input for bf9b
- The renderer composites ONE deck, the published active-deck pointer (Renderer.cpp:389-391; compositeDeck(*deck)
  :735-737), plus other decks' persistent layers (:760-766).
- Off-screen decks tick every frame inside the deckActive fence (Renderer.cpp:773-789): DeckClock::tick advances each
  visible layer's crossfade and the media CLOCK of its active clip and (mid-fade) the outgoing clip (DeckClock.h:26-44)
  via Renderer::tickMediaClock -> syncMedia(decode=false) -> VideoPlayer::advanceClock (Renderer.h:696-701,
  Renderer.cpp:1830-1839, VideoPlayer.cpp:410-431): no decode, no upload; sequences skip their texture load (:1909-1910).
- Autopilot runs for EVERY deck, one instance per deck INDEX (AutopilotBank.h:20-49; Renderer.cpp:546-561 active,
  :781-788 others). Its only per-instance state is the beat baseline (Autopilot.h:69; consumed Autopilot.cpp:65); clip
  beat counts live in the model (Clip::beatsPlayed, Autopilot.cpp:98). Beat-snapped pending triggers are fired inside
  processFrame (Autopilot.cpp:71-80), so on main a queued trigger on an off-screen deck CAN fire there.
- A video player that is not drawn parks its decode thread after 250 ms (VideoRing.h:263) and drops its Ready slots
  after kTrimIdleMs (VideoPlayer.cpp:1899-1912); the shown slot is never purged.
- MilkDrop preset-playlist cycling: active deck only (Renderer.cpp:595-684); one shared projectM source.
- ConnectionEngine evaluates EVERY deck's layer / clip / effect connections each 120 Hz tick (ConnectionEngine.cpp:339-)
  with a nullptr clip playhead (":351-355 ... treats as position 0.0" -- the BF6 site).
- Routines keep running on the deck they resolved on; the pad dims and the corner names that deck
  (RoutineDeckView.h:160, :190-193, :226-238). Footprint deck = first resolved target's deck INDEX at fire
  (RoutineEngine.cpp:54-77, :741-744); composition-scope targets keep deck -1 (Program.cpp:100-101).
  test_routine_engine.cpp:1759-1778 (D2) pins "a deck switch after the fire does not move it".
- Hidden / bypassed / soloed-out layers of the ACTIVE deck freeze (no fade tick, no media clock): compositeDeck's gate
  CompositorEngine.cpp:1072-1082.
- Clips carry no audio: VideoPlayer has no audio-stream path; Layer::muted has no audio consumer (grep, VERIFIED).

### 2.4 Every path that changes the deck on screen (VERIFIED) -- input for bf9b
| # | Path | Site | Cancels the left deck's queued triggers (L5) | Preview-fallback refresh |
|---|------|------|---------------------------------------------|--------------------------|
| 1 | Deck tab click | DeckView.cpp:491 -> MainComponent.cpp:1411-1442 (+ SwitchDeckCmd) | yes :1429-1430, :5529-5533 | yes :5547-5548 |
| 2 | REST /api/switch_deck | ApiServer.cpp:205, :755-759 -> MainComponent.cpp:1914 | yes (handleDeckSwitch) | yes |
| 3 | OSC switch deck | MainComponent.cpp:2214-2216 | yes | yes |
| 4 | Keyboard / MIDI SwitchDeck binding | MainComponent.cpp:7695-7698 | yes | yes |
| 5 | Genre auto-switch (Origin::Engine) | MainComponent.cpp:680-700 | yes | yes |
| 6 | Take replay "activeDeck" lane (Origin::Replay) | MainComponent.cpp:1977-1981 | yes | yes |
| 7 | Undo / redo of Switch Deck | DeckCommands.h:1037-1077; hook MainComponent.cpp:5102-5109 | restores / re-imposes the recorded cancel | NO |
| 8 | New Deck | newDeck MainComponent.cpp:3564-3578 -> AddDeckCmd DeckCommands.h:687-782 | yes :740-741 | first do yes :3576-3577; undo/redo NO |
| 9 | Load / Duplicate Deck | finishStagedLoad MainComponent.cpp:3187-, :3228-3251 -> InsertDeckCmd DeckCommands.h:798-883 | yes :836-837 | first do yes :3250-3251; undo/redo NO |
| 10 | Remove Deck (active -> neighbour) and its undo | removeDeck MainComponent.cpp:3750-3794 -> RemoveDeckCmd DeckCommands.h:900-1014 | undo only :980-982 | first do yes :3792-3794; undo/redo NO |
| 11 | Composition load / New | MainComponent.cpp:2997-3009 (routineEngine_.stopAll :2997; withDeckDetached) | n/a (model replaced) | yes :2984-2985 |
The GL detects a change only by deck INDEX derived from the pointer (Renderer.cpp:473-500; Pitfall 63 rule 7).

### 2.5 Per-deck couplings a "shared playing stack" must break (facts VERIFIED; implications INFERRED) -- not a design
1. Each Deck owns its layers (Deck.h:21); each Layer owns its clips row (Layer.h:293), its settings (Layer.h:159-209 read;
   the rest of the block not re-read) and its trigger tuple (Layer.h:580 runtime_, Pitfall 63) whose columns index that
   layer's OWN row. INFERRED: a layer cannot today play a clip from another deck's row.
2. The renderer draws one deck and derives its index from the pointer (2.3, Renderer.cpp:473-476).
3. GL history (temporal buffer, frame ring, feedback) is keyed by (deckId, layerId, chain) (LayerStateKey.h:9-20,
   CompositorEngine.h:325-345; Pitfalls 35 / 36). INFERRED: under a shared stack the key must follow the SHARED layer, or
   Boris's ruling "clip-to-clip transitions carry time effects over" (BORIS_DECISIONS.md:322-325) breaks whenever the
   incoming clip comes from another deck.
4. L5 cancel-on-leave (2.4 column 4; cancelPendingTriggers DeckCommands.h:202-231, applyPendingTriggerCancellation
   :233-256). INFERRED conflict with "the playing will not be affected": a clip queued from deck A must still land on its
   beat after switching to deck B.
5. Cross-deck transition (P25): at the switch frame the canvas (the old deck's LAST picture) is blitted ONCE into
   prevDeckFBO_ (Renderer.cpp:464-500) and blended into the live new deck over globalTransitionSpeed (default 0.3 s,
   Composition.h:114; Renderer.cpp:910-965). The old deck is never rendered during it: the dispatch's "both decks render
   during the transition" is FALSE at HEAD. (bf9b's default removes the transition.)
6. AutopilotBank exists only because decks play off screen (AutopilotBank.h:7-15). INFERRED hazard if per-deck
   instances are kept but only one is called per frame: a deck coming back consumes every beat since its last frame in
   one delta (OnsetPulse::consume, src/features/OnsetPulse.h:22-28) -> beatsPlayed jumps -> an immediate advance. One
   instance for one playing stack is correct by construction.
7. DeckClock (DeckClock.h), Renderer::tickMediaClock / syncMedia(decode=false) (Renderer.h:696-701) and
   VideoPlayer::advanceClock (VideoPlayer.h:99-105) exist only for off-screen decks (sole caller chain
   Renderer.cpp:778-779). INFERRED: orphans under a shared stack (also used by test_deck_clock.cpp,
   test_layer_runtime_race.cpp:27 / :169).
8. Routine layer targets resolve by deck INDEX (Program.cpp:96-117); the "playing on another deck" pad display assumes
   per-deck layers (2.3).
9. Take recording captures the active deck as a lane (MainComponent.cpp:5502-5512) and controls by (deck, layer[, col])
   paths; replay passes an explicit deck (MainComponent.cpp:4600-4619).
10. Undo commands re-resolve by (deckIndex, layerIndex[, col]) (MainComponent.cpp:5010-5015).
11. Fallback preview (refreshPreviewFromActiveClip MainComponent.cpp:4972-5006) mirrors the first active Image / Source
    clip of the active deck, hidden layers included (no `visible` check).
12. Column triggers act on the shown deck's layers and skip ignore-column layers (Deck.h:113-125).
13. /api/composition is shaped decks[] -> layers[] -> clips[] (ApiServer.cpp:405-450).
14. BORIS_DECISIONS.md:370-371 "Decks keep playing while off screen (2026-09-26)" carries no SUPERSEDED / CLARIFIED mark
    (Harmony's call).

### 2.6 The pitfalls the dispatch named -- meaning after Stage P / under bf9b
- Pitfall 35 (two chains per crossfading layer, per-chain keys): unchanged by Stage P; under bf9b still true, the
  (deck, layer) half changes owner (2.5 #3).
- Pitfall 36 (unique deck ids; history keyed by (deckId, layerId)): unchanged by Stage P -- the deck half of the key is
  still what keeps deck B's layer id 0 off deck A's layer id 0 history after a switch.
- Pitfall 38 (one baseline per Autopilot instance; per-deck bank): unchanged by Stage P (bank and DeckClock stay).
  bf9b: 2.5 #6.
- Pitfall 53's parenthetical names compositePersistentLayers (pitfalls.md:115) -> edited.
- Pitfall 63: Stage P removes one GL-read field and edits DeckClock (a GL tuple writer) -> "probe-tsan-unit.sh is
  REQUIRED" applies (G4).

### 2.7 Numbers
CLAUDE.md = 24,006 bytes (wc -c; cap 25,000). Stage P removes 59 bytes from it (-> 23,947).

## 3. DESIGN FORKS (Stage P)

F1 Delete the field vs keep it inert for file compatibility. CHOICE: delete. Evidence: fromVar reads keys by name
   (Layer.cpp:144-170), so an unread "persistent" key is ignored; an older build reading a newer file gets a missing key
   -> void var -> false (INFERRED from juce::var; the existing "Backward compatibility: old-format presets load with
   struct defaults" case, test_composition.cpp:516, relies on the same rule). Runner-up loses: a dead field nobody may
   set keeps a GL-read field in tsan-r5's scope and a key that means nothing.
F2 A loaded persistent=true: silent vs a notice. CHOICE: silent -- the layer is an ordinary layer of its own deck.
   Runner-up loses: nothing for Boris to do; he asked for the removal.
F3 The Layer section's row. CHOICE: "Ignore Column Trigger" takes the full row (LayerInspector.cpp:639 ->
   `ignoreColumnToggle_.setBounds(area.getX(), y, area.getWidth(), kRowHeight);`). Runner-up (keep the right half, an
   empty left half) loses: a hole where a control was reads as a missing control. Text, tooltip, behaviour unchanged.
F4 applyLayerKeying's explicit `mode` (added for the persistent Opaque path, CompositorEngine.h:484-490). CHOICE: keep
   the signature, reword the comment. Runner-up (revert) touches compositeDeck's call (:1152) that bf1 / bf9b may edit,
   for zero behaviour.
F5 Empty active deck. CHOICE: compositeDeck's own rule again -- nothing to draw -> the fallback, which every switch to
   an empty deck clears (MainComponent.cpp:5000-5005) -> black (Renderer.cpp:836-844). No alternative:
   beginEmptyActiveDeck existed only to put persistent layers over an empty deck.
F6 DeckClock's persistent exception (DeckClock.h:30-31, :35-36). CHOICE: delete the exception (every visible layer of an
   off-screen deck is ticked); DeckClock itself stays until bf9b. Runner-up (delete DeckClock now) loses: that is bf9b's
   design call, and alone it would change keep-time behaviour Stage P must not touch.
Superseded forks (recorded once, NOT to build): come-back policy (resume / restart / empty) -- moot; routines stopped on
a deck change -- wrong under bf9b; one autopilot instance -- bf9b input 2.5 #6.

## 4. ITEMS + BUILD STAGES

### Stage P -- ONE builder context; ships alone (pure removal + one layout line); merge FIRST (bf9's slot)

P1 Model + file format
- Files: src/model/Layer.h -- delete :161-172 (canBePersistent + comment) and :182; reword :186 -> "V dropdown picks a
  MixMode for the layer's blend." and :191 -> "=== Standard Compositing (layer blend modes) ===". src/model/Layer.cpp --
  delete :39 and :156.
- Behaviour: a show saved with "persistent": true loads with no error; saving it writes no "persistent" key.
- RED first: tests/test_composition.cpp NEW TEST_CASE "Layer: persistent=true from an older file loads cleanly and is
  never written back; ignoreColumnTrigger round-trips (bf9 P1)" [composition][serialization][backcompat]: take
  Layer{}.toVar(), set "persistent" = true and "ignoreColumnTrigger" = true on its DynamicObject, fromVar into a fresh
  Layer; CHECK(loaded.ignoreColumnTrigger); CHECK(toVar has "ignoreColumnTrigger"); CHECK_FALSE(toVar has
  "persistent"). FAILS on main at the last CHECK (Layer.cpp:39 writes it); compiles on main. SECTION 2 (guard, GREEN on
  both): Composition::loadFromFile of a 2-deck file whose layers carry "persistent": true returns true with both decks.
- GREEN: the case and every [composition] case pass.

P2 Compositor + renderer
- Files: CompositorEngine.h delete :150-161, :165-169, :171-176; reword :185-186 ("Call once per frame, AFTER
  compositeDeck()"), :327-330 (layer ids restart per deck), :484-487 (mode param), :529-541 (renderLayerStages = "the
  active deck's layer loop"). CompositorEngine.cpp delete :1266-1283, :1285-1304, :1306-1439; reword :1139-1141.
  Renderer.cpp delete :739 and :742-766 (the "P21: Composite persistent layers" comment, the empty-active-deck start
  :742-758 and the compositePersistentLayers loop :760-766) but KEEP :740-741 `if (composition_) {` and its closing
  brace :790 -- the DeckClock loop :768-789 stays inside it; reword :691-695, :768-772 (drop the persistent clauses),
  :791, :799-801. Renderer.h reword :130, :366-367, :585. DeckClock.h delete :30-31 and :35-36, :33 ->
  `if (!LayerClock::tick(layer, rt, dt)) ++adopts;`, delete the ownership paragraph :12-14. LayerClock.h:6 ->
  "CompositorEngine (the active deck)". LayerStateKey.h:9-14 rationale -> "layer ids restart at 0 on every deck, so
  keying by layer id alone would hand deck A's layer-0 history to deck B's layer 0 after a deck switch" (key unchanged).
- Behaviour: no layer of another deck is ever drawn; an empty active deck shows black / the fallback, as before R4.
- RED first: (a) tests/test_render_thread_lint.cpp case 2: pin CompositorEngine.cpp {runtime() 1, getActiveClip( 1}
  (was {2, 2}); rewrite the site comment :58-66 -- FAILS on main (2 / 2). (b) NEW case in the same file "no
  Persistent-feature identifier left in src/": walk AUDIODNA_SRC_DIR recursively (*.h *.cpp *.mm), strip line comments,
  regex `canBePersistent|compositePersistentLayers|hasPersistentContent|beginEmptyActiveDeck|persistentToggle_|(\.|->)persistent\b|"persistent"`
  -> 0 hits, file:line listed on failure -- FAILS on main (Layer.h:169, Layer.cpp:39 / :156, LayerInspector.cpp:156-160
  / :922-931, CompositorEngine.cpp:1266-1348, DeckClock.h:30-31, ApiServer.cpp:428, Renderer.cpp:751-763).
  (c) delete test_compositor.cpp:423-437 and test_deck_clock.cpp:71-102 (case (b)); reword test_deck_clock.cpp:12-16 and
  test_layer_state_key.cpp:10, :71-72.
- GREEN: (a)(b) pass; test_deck_clock (a)(c)(d)(e)(f) unchanged and passing.
- Live RED: P5 rows.

P3 Layer tab
- Files: src/ui/LayerInspector.h:94 delete; LayerInspector.cpp delete :155-162, :638, :922-945; :639 -> full row (F3).
- RED first: tests/test_layer_inspector_layer_row.cpp REPLACES test_layer_inspector_persistent_toggle.cpp (target renamed
  in tests/CMakeLists.txt:2199-2247, same link closure; comment :3086-3087 updated). Same headless idiom
  (ScopedJuceInitialiser_GUI, find toggles by button text, click = setToggleState(x, juce::sendNotification)):
  (1) no child ToggleButton titled "Persistent" -- FAILS on main;
  (2) Ignore Column Trigger untouched -- for each Layer::Type (Opaque, Transparent, FXOnly, ThreeD, Mask): present,
      enabled, mirrors layer.ignoreColumnTrigger after setLayer (both values), a click writes the field, tooltip ==
      "This layer ignores column trigger buttons" -- GREEN on both (the proof);
  (3) after setSize(300, 900) + setLayer + resized(): the toggle's x and width equal the inspector's full-width rows
      (getLocalBounds().reduced(4, 0), LayerInspector.cpp:578) -- FAILS on main (right half).
- tests/tool_uitoggle_snapshot.cpp:122-165: keep ONE LayerInspector shot (Opaque, no persistent line) named
  layerinspector-layer-row-headless.png; drop the two Mask shots.

P4 REST
- ApiServer.cpp:428 delete; :425 comment -> "each layer's fade state, witnessable over REST".
- RED: probe row p_api_no_field.

P5 Probes (live RED rows; retirements)
- .harmony/probe-render-state.py: RETIRE the 14 rows of 2.1 and their fixture keys (their subject no longer exists);
  keep r1_* and r5_*. ADD:
  p_flag_ignored -- deck 0: layer 0 Opaque A, triggered; deck 1: layer id 5 Transparent, "persistent": true, clip B
    covering the frame, primed with persist_setup(). Bar: nonblank(f) and d(f, A_only) <= tol (1.5). Main: B is drawn
    over A -> FAIL.
  p_flag_ignored_empty -- deck 0: one layer whose clip is never triggered; deck 1 as above, primed. Bar: mean RGB(f)
    <= 2.0. Main: beginEmptyActiveDeck + B -> FAIL.
  p_api_no_field -- after loading the p_flag_ignored file: no decks[].layers[] object of /api/composition has
    "persistent"; each still has id / visible / activeClipColumn. Main: present -> FAIL.
  p_ignore_column -- deck 0: 2 layers x 2 image columns, layer 1 "ignoreColumnTrigger": true; trigger_clip(1, 0);
    POST /api/trigger_column {"column": 1}; 0.5 s later layer 0 activeClipColumn == 1 and layer 1 == 0. GREEN on both.
  Update the .py docstring row list and probe-render-state.sh :3-8.
- .harmony/probe-deck-clock.py: RETIRE d_persistent_single_advance (+ its json key, docstring rows, .sh :5). Every other
  row is unchanged (Stage P does not touch keep-time).

P6 Docs -- section 6.

Order inside the stage: P1 -> P2 -> P3 -> P4 -> P5 -> P6 (P2-P4 need P1's field gone to compile). Ships alone: yes.

### NOT TO BUILD (superseded 14:44:36; bf9b decides)
S1 the renderer stops ticking off-screen decks (Renderer.cpp:768-789); S2 one autopilot instance; S3 routines stopped
off the screen deck; S4 Rule 15 -> "a deck off screen is paused"; S5 the come-back policy point.

### Files Stage P shares with other lanes (exact hunks) -- merge Stage P first
- src/model/Layer.h :161-191; src/model/Layer.cpp :38-39, :155-156 -- bf7 (bar values) and bf9b add / move fields nearby.
- src/ui/LayerInspector.{h,cpp}: ctor :155-171, resized() :636-640, sync :921-946 -- bf7 edits the autopilot beat-count
  rows (:615-622, :905-908): different hunks.
- src/render/CompositorEngine.{h,cpp}: h:150-186, cpp:1266-1439 -- bf1 (layer output to a clip) may touch compositeDeck /
  saveLayerOutput (:1145-1146): different hunks.
- src/render/Renderer.cpp :739-801 -- bf9b rewrites this block; bf1 touches the recorder submit after master opacity
  (~:880-905): different hunk. src/render/DeckClock.h -- bf9b likely deletes it.
- src/api/ApiServer.cpp :425-428 -- ui lane (BF3 codec per clip) adds clip fields at ~:436-450: adjacent.
- tests/test_render_thread_lint.cpp pins (bf9b changes them again); tests/CMakeLists.txt :2199-2247.
- docs/claude/performance-controls.md :16, :42-49 -- ui lane BF8 edits :51; bf9b rewrites :49.
- CLAUDE.md :14, :248; .harmony/APP-INVENTORY.md :73 (bf7 likely edits the same row).

## 5. GATES (Harmony, after the merge; pre-registered)
G1 Build: cmake --build build --config Release -j$(sysctl -n hw.ncpu) -- 0 errors, no new warnings in touched files.
G2 Unit: ctest --test-dir build -j8 --output-on-failure -- 0 failures; P1, P2(a)(b), P3(1)(2)(3) present and passing;
   record the test count (before - removed + added).
G3 Identifier grep (any hit = FAIL): grep -rnE 'canBePersistent|compositePersistentLayers|hasPersistentContent|beginEmptyActiveDeck|persistentToggle_|(\.|->)persistent\b|"persistent"' src
   -> 0 lines. Ignore-column untouched (changed lines only, never context):
   `git diff -U0 5e47d17..HEAD -- src/model/Deck.h src/model/Layer.h src/model/Layer.cpp src/MainComponent.cpp | grep -cE '^[+-][^+-].*ignoreColumn'`
   -> 0; the same on src/ui/LayerInspector.cpp -> exactly 2 (the old and the new setBounds line).
G4 TSan unit (Pitfall 63): .harmony/probe-tsan-unit.sh -- every [tsan] case PASS, zero "WARNING: ThreadSanitizer".
G5 Live (one app under the lock helper; open -g ... --args --test-mode; never an Output window):
   probe-render-state.sh p_flag_ignored,p_flag_ignored_empty,p_api_no_field,p_ignore_column + r1_*, r5_* -> all PASS;
   probe-deck-clock.sh surviving rows -> all PASS (keep-time untouched). RED evidence in the lane report: the three p_*
   rows FAIL on a 5e47d17 build, p_ignore_column PASSES on both.
G6 Perf: none -- Stage P only removes per-frame work (an A/B needs a persistent fixture that exists only on main; its
   teeth would equal the drift: INFO at most).
G7 VISUAL WORK GATE (the Layer tab changed): a TEMPORARY env hook ADNA_BF9_INSPECT_LAYER=<n> (after startup:
   deckView_->onLayerSelected(n) + inspectorPanel_->setActiveTab(InspectorPanel::Tab::Layer); reverted before merge) on
   the lane build and a 5e47d17 build; `open -g --env ADNA_BF9_INSPECT_LAYER=0 ... --args --test-mode`; window-only
   capture with probe-deck-tabs.sh's shot() pattern (screencapture -x -o -l <CGWindowID>); decode; critic panel
   (visual-design, UX, graphic-design, logic + an interaction-logic critic for the toggle state) on BEFORE / AFTER.
   Pass: the Layer section shows Master then ONE full-width "Ignore Column Trigger" row, left edge aligned with Master,
   no gap, nothing below moved, the tick mirrors the model. Then an artifact page for Boris (before / after, plain words).

## 6. DOCS (Stage P)
- docs/claude/performance-controls.md: :16 -> "Only the active deck's layers publish output." (drop the persistent
  clause). :42-47 -> one paragraph: "**Persistent layers: removed** (2026-10-02, Boris: "I want to remove the
  persistent"). A layer saved with `persistent: true` loads as an ordinary layer of its own deck; the only "keep this
  layer" control is **Ignore Column Trigger** (`Layer::ignoreColumnTrigger`, the Layer tab's Layer section;
  `Deck::triggerColumn` skips it)." :49 -> drop "like `compositePersistentLayers`", drop the sentence "Persistent layers
  are owned by `compositePersistentLayers` ... never advanced twice.", drop "/ `persistent`" from the REST field list
  (the rest of :49 is bf9b's).
- docs/claude/pitfalls.md:115 (Pitfall 53): "(`compositeDeck` / `compositePersistentLayers`)" -> "(`compositeDeck`)".
- CLAUDE.md:14 delete "persistent layers across deck switches, "; :248 "(bindings, persistent layers, beat snap
  granularity, Ableton Link)" -> "(bindings, beat snap granularity, Ableton Link)". Net -59 bytes. Rule 15 untouched
  (bf9b's).
- .harmony/APP-INVENTORY.md:73 "Layer (master/persistent/ignore-column)" -> "Layer (master/ignore-column)".
- .harmony/FEATURES.md:355, :539, :838, :877, :2434, :2694 -- drop the persistent clauses.
- BORIS_DECISIONS.md: nothing new (Harmony already marked the 2026-09-26 entries). New pitfall: none.

## 7. RISKS (cheapest discriminating test)
R1 A show Boris built with Persistent layers looks different (the layer no longer shows over other decks). Accepted --
   he asked. Test: load it, switch decks (8.1).
R2 Coverage: the 14 retired rows proved "persistent layer = the same layer on its own deck"; their subject is gone;
   r1_* / r5_* keep the crossfade-history and hold coverage. Test: G5.
R3 Merge collisions on the shared hunks (section 4 table). Mitigation: Stage P merges first; every hunk is a deletion or
   one line.
R4 Stage P alone does NOT fix "kept playing ... invisible": off-screen decks keep time until bf9b lands. Say so on
   Boris's page so he does not test the wrong thing.
R5 tsan-r5 fence: Stage P removes Layer::persistent (plain bool; writers LayerInspector.cpp:160 and Layer::fromVar; GL
   readers CompositorEngine.cpp:1276, :1329, DeckClock.h:30-31). No other field changes thread. bf9b will change which
   Layer / Clip objects the GL writes (Pitfall 38's note): re-scope tsan-r5 against post-bf9b code.
R6 Strongest counterargument to shipping Stage P before bf9b: bf9b may delete DeckClock and reshape CompositorEngine, so
   editing them now is churn. It loses: the edits there are four deleted DeckClock lines and pure deletions in
   CompositorEngine; landing first shrinks bf9b's diff and removes a cross-deck render path bf9b would otherwise have to
   reason about.

## 8. WHAT ONLY BORIS CAN CHECK
8.1 A show that used Persistent loads; that layer now shows only on its own deck.
8.2 Layer tab: "Persistent" is gone; "Ignore Column Trigger" sits alone on its row and reads clearly.
8.3 His workflow: tick Ignore Column Trigger on a layer, fire columns -- that layer keeps its clip while the others change.

## 9. QUESTIONS FOR BORIS
None for Stage P (ruled twice). The earlier follow-up "what does a deck show when you come back" is moot
(binding-decisions.md "BF9 CLARIFIED").

STATUS: FINAL -- research input for bf9b + Stage P (Persistent removed) buildable; deck-stop premise superseded, nothing else to build from this file
