# PLAN lane bf9b -- decks are boxes of clips; the layers are ONE shared playing stack
Base: main eff2b1c at planning (the dispatch named 5e47d17; the recon sheets 9a832a0). The builder records BF9B_BASE.
Author: architect (Fable), s-rta-1002b. Supersedes lane bf9's premise ("a deck change stops the old deck").
Binding inputs satisfied: ruling-bf9.md (Stage P = S0 verbatim, amendments 1-17; K1-K8 + the requirement table carried
into section 5) and ruling-bf6.md (C1 / C2 / C3 provided in 4.A).

## 1. GOAL
One shared layer stack for the whole show: what is in a layer is what plays and is on screen; a deck is only a box of
clips; switching decks changes which box the grid shows and nothing else -- no playing clip, position, speed, fade or
effect changes, and nothing ever plays unseen.
Serves, verbatim:
- "when I switch between decks, do not change the clips playing in the layers or how they are playing. treat the decks
  as just a box of clips and I can switch between 20 decks looking for a clip and the playing will not be affected."
- "When I change decks, they clipped kept playing, and it was invisible. This is not good. Whatever is in the layer
  should be what is playing and there should be nothing else."
- "I'm going back on what I asked for before and I want to remove the persistent." / "The only thing remotely
  persistent should be to ignore column controls so if we want to have something stay on a certain layer and we want to
  play with the other layers in the composition, that stays in the other layers can be switched by a column switch."

## 2. ESTABLISHED FACTS
Labels: VERIFIED-A = re-read by this architect (file:line); VERIFIED-R = from the dispatch's recon fact sheets (read-only
readers, cited there, not re-read here); INFERRED = reasoned, evidence named. Paths under src/ unless noted.

2.1 Ownership today
- Composition owns `std::vector<Deck> decks` + `RelaxedInt activeDeckIndex`; Deck owns `std::vector<Layer> layers`;
  Layer owns `std::vector<std::optional<Clip>> clips` + the private `LayerRuntimeCell runtime_` (VERIFIED-R
  model/Composition.h:34-38, model/Deck.h:21-22, model/Layer.h:293, :580).
- The tuple Word is `{int32 active; int32 previous; float progress; uint32 pendingPacked}`, alignas(16); pending is
  stored +1 in the low 28 bits, the snap override in the high 4; `kMaxPendingColumn = 0x0FFFFFFF - 1` (VERIFIED-A
  model/Layer.h:83-103); tests/test_layer_runtime.cpp:48-51 pins 268,435,454 (VERIFIED-A). Not serialized (VERIFIED-R).
- `Layer::triggerClip(int column, forcedSnap, maxAttempts, std::optional<int> onlyIfActive)` reads its OWN `clips` for
  the empty-cell check and the snap mode (VERIFIED-A Layer.h:400-412); processPendingTrigger, clearActiveClip and the
  activation / clear tails read the layer's own clips too (VERIFIED-R Layer.h:437-475, :526-577).
- Layer ids repeat across decks (initDefault 0,1,2; nextLayerId_ per deck from 100) (VERIFIED-R Deck.h:29-38, :57,
  :208). Deck ids: Composition::nextDeckId_ (100 up, not serialized, re-synced to max + 1 on load), unique per
  composition (VERIFIED-R Composition.h:213-241, :571-589; Pitfall 36). Clip ids are re-minted from MainComponent's
  `s_nextClipId` on every composition load, deck load and duplicate (VERIFIED-R MainComponent.cpp:17, :3340-3341,
  :3500-3520).

2.2 What runs per frame today (GL thread, Renderer::renderOpenGL)
- The deck pointer and the fence flag come from ONE `activeDeck_.view()` (FencedPtrSlot<Deck>) (VERIFIED-R
  render/Renderer.cpp:388-391). compositeDeck(*deck) renders the shown deck (VERIFIED-A CompositorEngine.h:144); every
  OTHER deck gets DeckClock::tick + its own Autopilot (AutopilotBank, by deck INDEX) inside the deckActive gate
  (VERIFIED-R Renderer.cpp:773-791, model/AutopilotBank.h:5-30) -- the cause of "kept playing ... invisible". Stage 0
  deletes the persistent pass (Renderer.cpp:739-767).
- The P25 deck fade: switch detected by pointer-derived index vs prevActiveDeckIndex_, canvas blitted to prevDeckFBO_,
  `deck_transition` drawn after master opacity (VERIFIED-R Renderer.cpp:464-497, :910-965; VERIFIED-A program fetched at
  Renderer.cpp:930, compiled at :1962). Its only setting, `Composition::globalTransitionSpeed`, is read by Renderer.cpp:483
  and written only by TopBar's Fade slider (VERIFIED-A grep: TopBar.cpp:197, :204; Composition.h:114, :189, :316, :449).
  crossfaderPhase / crossfaderBehaviour have no reader outside Composition.h (VERIFIED-A grep).
- GL history keys are clipChain / outgoingChain / layerChain(deckId, layerId) (VERIFIED-R render/LayerStateKey.h:39-62);
  renderLayerStages(Layer&, rt, uint32_t deckId, const Clip&, ...), applyTransition(Layer&, rt, outgoingKey, ...),
  saveLayerOutput(layerId, deckId, ...), heldLayerOutput(layerId, deckId) (VERIFIED-A CompositorEngine.h:543, :524,
  :566, :287). The Layer Router reads `activeDeck_.get()->layers[idx].id` (VERIFIED-R Renderer.cpp:1443-1455).
- `Autopilot::processFrame(Deck&, const FeatureSnapshot&, FrameReport*)`, advanceClip(Layer&, int currentCol, ...),
  smartAdvanceClip (VERIFIED-A model/Autopilot.h:33, :54, :64); processFrame is the ONLY non-test caller of
  processPendingTrigger (VERIFIED-R Autopilot.cpp:75).

2.3 Message-thread paths today
- handleDeckSwitch cancels the leaving deck's queued triggers, sets activeDeckIndex, re-points the renderer, calls
  refreshPreviewFromActiveClip(*deck), rebuilds the grid (VERIFIED-R MainComponent.cpp:5488-5549);
  refreshPreviewFromActiveClip loads the first active Image / Source clip of that deck into the preview renderer
  (VERIFIED-A :4972-5004).
- handleClipTrigger(layer, column, origin, deckIndex = -1, immediate) and handleColumnTrigger(column, origin,
  deckIndex = -1) are the fire entries every keyboard / MIDI / OSC / REST / replay path ends in (VERIFIED-R
  :714-720, :1912-1914, :1958-1975, :2207-2216, :4605-4960, :7586-7650). hasBeenTriggered, the beat-snap / retrigger
  player seeks and the preview refresh run ONLY when the fired deck is the shown deck (VERIFIED-R :4654-4772).
- Binding Selected = the first layer of the SHOWN deck with an active clip; ThisItem scans only the shown deck and
  silently falls back to ByPosition (VERIFIED-A :7543-7577).
- MIDI pad feedback `midiOutputHandler_.updateFromDeck(composition_.getActiveDeck())` (VERIFIED-A :4170); routine bands
  `deriveRoutineDeckView(status, composition_.activeDeckIndex, deckNames, shownDeckLayerNames)` (VERIFIED-A :4190,
  ui/RoutineDeckView.h:139); `routineEngine_.stopOnLayer(composition_.activeDeckIndex, layerIdx)` (VERIFIED-A :762,
  recording/RoutineEngine.h:134, .cpp:807).
- ConnectionEngine::tick walks `for deck : comp.decks` / `for layer : deck.layers` (VERIFIED-A
  connect/ConnectionEngine.cpp:339-341).
- Routines: Program::resolveDeck maps deck-relative keys to `comp.activeDeckIndex` (VERIFIED-A recording/Program.cpp:25-48);
  the preamble walks every captured deck (Program.cpp:187-223) and restores the captured active deck as a point
  (:231-245). PerfState = `DeckRuntime{deck, map<int, LayerRuntime> layers}`, LayerRuntime = tuple fields + opacity /
  visible / bypassed / solo / muted / autopilot + layer-effect params + column -> ClipRuntime (VERIFIED-A
  recording/PerfState.h:20-82); PerfStateCapture walks every deck and layer (VERIFIED-A PerfStateCapture.cpp:52-116).
- SwapClipsCmd / SetClipCmd never touch the tuple -- the layer follows the CELL (VERIFIED-A grep core/ClipCommands.h);
  RemoveColumnCmd has no tuple fix-up (VERIFIED-A grep DeckCommands.h, no runtime hit in :95-180); ClearLayerClipsCmd
  restores the row AND the layer runtime (VERIFIED-A DeckCommands.h:318-330).
- The Layer strip already draws the playing clip's thumbnail and name (VERIFIED-A ui/LayerStrip.cpp:372-406, :512-524,
  :692 clipNameBounds_, :718-744, all via layer_->getActiveClip()). TopBar's Fade section: TopBar.h:58-61, :120-122;
  TopBar.cpp:191-205 and :624-625; tests/test_master_signal_link.cpp:189, :300, :340 use fadeSliderBoundsForTest() as
  the Master Signal's left neighbour (VERIFIED-A). VideoPlayer / ImageSequence expose getPlayheadPosition() (VERIFIED-A
  media/VideoPlayer.h:93, media/ImageSequence.h:65). REST test routes /api/debug/load_deck and /api/debug/duplicate_deck
  exist; no add / remove deck route (VERIFIED-A ApiServer.cpp:320-321).

2.4 Scale (VERIFIED-A grep counts, src / tests): `.layers` 65 / 145; getClipAt 53 / 43; getActiveClip 23 / 34;
`runtime()` 31 / 247; triggerClip(Immediate) 24 / 80; getLayer( 52 / 158; activeDeckIndex 125 / 87. Heaviest src
files: MainComponent.cpp 178, DeckCommands.h 48, CompositorEngine.cpp 13, Autopilot.cpp 13, CompositionLoad.h 13,
Program.cpp 12, DeckView.cpp 11, Composition.h 10, ApiServer.cpp 8. ctest holds ~1,114 tests (VERIFIED-R ruling-bf9 G2).

2.5 Probes reading the old JSON: probe-routines.sh reads `decks[0].layers[0].opacity` / `activeClipColumn` (VERIFIED-A
:148, :568, :706, :739, :775, :906-985); probe-crossfade.py POSTs an OLD-format show (`"decks":[{"layers":[layer]}]`,
VERIFIED-A :99); probe-deck-tabs R5 checks only activeDeck after switch_deck (VERIFIED-A :12, :139-142); probe-deck-path
is single-deck pixel work (VERIFIED-A header :2-20); probe-deck-clock rows d_fade_finishes, d_persistent_single_advance,
d_pending_trigger_still_cancelled, d_video_keeps_time, d_imageseq_keeps_time, d_autopilot_keeps_time, d_return_hitch
(VERIFIED-A def lines :213-390).

2.6 Docs anchors (VERIFIED-A): CLAUDE.md 24,002 B; rule 15 at :106; pitfall index 35 / 36 / 38 / 63 at :199 / :200 /
:202 / :227; trigger table :247 (cross-deck transitions P25), :248 (persistent layers). performance-controls.md :16
(router "only the active deck"), :42-47 (Persistent), :49 (inactive decks keep time). rendering.md :158-164 (P25).
BORIS_DECISIONS.md :337, :339, :380 already SUPERSEDED; :342 CLARIFIED; :350 "Decks are boxes of clips".

## 3. DESIGN FORKS (every defensible alternative; the choice; why the runner-up loses)
F1 WHERE THE SHARED STACK LIVES -- CHOSEN (A) RESTRUCTURE. Composition owns `std::vector<Layer> layers` (all layer
   settings + the playing tuple, NO clips). Deck owns `std::vector<ClipRow> rows` (clips only). Row N of every deck feeds
   shared layer N; every deck (live or retired) always has exactly `layers.size()` rows.
   - (B) keep per-deck Layer containers for clips and lift only settings + tuple into a composition stack. LOSES: the
     per-deck Layer objects keep compiling settings and tuple fields nothing renders any more; every reader the lift
     misses (2.4: ~400 src sites) silently reads a dead per-deck copy -- the quiet-failure class. (A)'s type split turns
     each into a compile error resolved by a written rule (S2 rulebook).
   - (C) copy-on-fire (the layer owns a copy of the playing clip). LOSES: replacing a Clip the GL thread reads needs a
     fence (a held frame) per trigger, the GL thread fires on its own (autopilot / queued triggers, 2.2), and inspector
     edits would land on the box copy, not the one playing.
   Strongest counterargument to (A): churn (~600 test sites). It loses: the churn is mechanical and compiler-enumerated;
   (B)'s risk is silent and unbounded. S2b's ShowFixture keeps the test rewrite to one pattern.
F2 HOW A LAYER NAMES ITS CLIP -- CHOSEN ClipRef = (deckId, column) in each tuple slot (active / previous / pending); the
   row is the layer's index. Packed into the SAME 16-byte word (the lock-free maximum the Apple static_assert guards,
   VERIFIED-R Layer.h:134-138): active / previous int32 = -1 or (deck << 16 | column); pending 28 bits = 0 or
   ((deck << 14 | column) + 1). Deck field 14 bits: valid ids 0..16382 (`ClipRef::kMaxDeckId`); 0x3FFF encodes "no deck"
   so every pre-bf9b snapshot round-trips; column 0..16382 (validateDeck already caps 10000, VERIFIED-R
   CompositionLoad.h:18-44).
   - clip-id refs: LOSE -- a scan per resolve, autopilot needs the location anyway, and any path that copies an id
     (paste, undo snapshots) makes a ref ambiguous.
   - deck-INDEX refs: LOSE -- every remove / insert / undo shifts indices; every stored tuple (undo snapshots too) would
     need a fix-up.
   - a wider tuple: impossible (lock-free bound); a separate per-slot deck atomic tears against the tuple (the exact
     bug class Pitfall 63 exists for).
F3 A DECK IS REMOVED WHILE ONE OF ITS CLIPS PLAYS -- CHOSEN (dispatch default; Q1) it keeps playing until replaced.
   The Deck moves from `decks` to `Composition::retiredDecks_` (not shown, not saved, not indexed; its id stays reserved)
   while any layer's active or previous ref names it; queued triggers into it are cancelled (a clip from a box you can
   no longer see must not start later). Retired decks are reaped -- media disposed -- inside the next deck-structure
   command or composition swap once no ref names them. Undo of Remove Deck moves the retired deck back (live playheads
   kept); if it was already reaped, it re-inserts the snapshot as today.
   - a "held clip" slot per layer (move only the playing clip): LOSES -- moves clips the GL thread may be fading, needs a
     sentinel ref and an outgoing-clip special case. Retiring moves nothing a ref points at: a Deck move transfers its
     vectors' buffers, so every Clip keeps its address.
   - stop on remove: simplest; contradicts the dispatch default; Q1's alternative (R13: ~30 lines).
   Cell-level deletes keep TODAY's behaviour (the ref follows the cell, 2.3): clear a cell / remove a column / swap cells
   act as now; ClearLayerClipsCmd stops its layer only when the layer plays from that row of that deck.
F4 RENDER -- CHOSEN one pass over the shared stack (`CompositorEngine::compositeShow`) and one Autopilot for the show.
   Deleted: DeckClock (its job, keeping off-screen decks' clocks, no longer exists: nothing plays off screen),
   AutopilotBank, the P25 deck fade (nothing to fade), and (S0) Persistent. GL history is keyed by the shared layer --
   clipChain(kShowStackKey = 0, layer.id) etc. -- so a deck switch cannot touch history (K1t by construction).
   - keep DeckClock so box clips keep time: LOSES -- K2 forbids exactly that ("nothing plays unseen").
F5 THE FENCE TOKEN -- CHOSEN keep `FencedPtrSlot<Deck> activeDeck_` and UndoService::withDeckDetached unchanged as the
   fence token; the GL thread no longer dereferences the pointer (lint B4c). Re-typing the slot to Composition* buys
   nothing and churns UndoService and its tests.
F6 LOAD DECK WITH MORE ROWS THAN THE SHOW HAS LAYERS -- CHOSEN (Q2 default) add shared layers so every row shows
   (settings from the file's row when it is an old file, else defaults); other decks get empty rows. Fewer rows: padded.
   - hide the extra rows until the show has that many layers: LOSES -- Boris loads a deck and cannot see its clips.
   - drop them: never (loses clips).
F7 OLD SHOWS -- CHOSEN the binding default "the first deck's layer settings win": shared count = the most rows any deck
   has; layer i's settings = the first deck (file order) that has row i; every deck padded; colliding layer ids
   re-minted. "persistent" and "globalTransitionSpeed" are read ONLY by the converter (src/model/ShowMigration.h), to
   name them in ONE note (K7). Detection: a composition without a top-level "layers" array is an old show (no version
   key exists today, VERIFIED-R recon section 7; adding one buys nothing now).
F8 REST SHAPE -- CHOSEN add a top-level `layers` array (the truth: settings + activeClip / previousClip / pendingClip as
   {deck, deckId, column, clipId, retired} + crossfadeProgress) and KEEP `decks[d].layers[l]` as a legacy mirror (the
   shared layer's settings + activeClipColumn / previousClipColumn derived for deck d, -1 when the ref names another
   deck + crossfadeProgress + deck d's row-l clips). A breaking reshape LOSES: it breaks probe-routines /
   probe-crossfade / probe-deck-clock readers (2.5) whose rows the ruling does not touch.
F9 ROUTINES -- CHOSEN: Layer-scope targets resolve to the shared layer (any deck part ignored); Clip-scope targets and
   activeClip fires resolve on the deck SHOWN when the routine fires (D2 unchanged) and land in that row's layer;
   stopOnLayer matches the layer only; bands show on the shared layer whatever deck is shown; an activeDeck lane or
   preamble point changes only the grid (K7). PerfState v2 adds a shared `layers` map; an old take restores the shared
   layers from its captured ACTIVE deck (the only deck that was visible when it was recorded).
F10 AUTOPILOT ADVANCE SOURCE -- CHOSEN the deck the playing clip came from (a retired or empty source: no advance).
   The shown deck LOSES: a deck switch would change what autopilot plays next -- "how they are playing".
F11 QUEUED TRIGGERS -- CHOSEN survive a deck switch and fire on their bar (K5); the L5 "leaving a deck cancels its
   queued triggers" rule is deleted on every path (it existed so a hidden deck could not fire invisibly; a queued
   trigger now lands in the shared, visible stack).
F12 COLUMN FIRE ON AN EMPTY CELL -- unchanged: that layer is cleared (Deck::triggerColumn today). Ignore Column is the
   one way to keep a layer through a column fire -- exactly Boris's framing.
F13 C3 (bf6 contract) -- CHOSEN bf6 AM-6 "Item 8" semantics, owned here: a NEWLY activated (not retrigger, not queued)
   playable clip whose player / sequence already exists gets `clip->playheadPosition = player->getPlayheadPosition()`
   (sequence: `seq->getPlayheadPosition()`) in the same message-thread call, for clip AND column fires. A re-fire still
   RESUMES (no visible change). GL-thread activations stay INFO (same-frame write-back, ruling-bf6 AM-6).
F14 DECK SWITCH AND UNDO -- CHOSEN unchanged (a tab click is still an Undo step, minus the queue cancel). Q4 asks.
F15 GENRE AUTO-SWITCH -- CHOSEN (Q3 default) inert: a detected genre no longer moves the grid (it would only yank the box
   Boris is browsing); assignments stay in the file; one logLine per session says so.
F16 THE GRID ON A SWITCH -- CHOSEN keep the layer strips (they ARE the same shared layers) and re-point only the cells
   (`DeckView::showDeck`); a full rebuildGrid only when the layer count changed. Rebuilding strips per switch LOSES: it
   resets strip widgets 20 times in a browse and risks K8's 50 ms bar.

## 4. ITEMS + BUILD STAGES
Branch lane/bf9b (one worktree). Order S0 -> S1 -> S2 (2a src, 2b tests) -> S3 -> S4; main receives everything in ONE
merge (ruling-bf9 amendment 1). RED convention (the ruling's own): unit tests on NEW model API are RED as "does not
compile on STAGE_P_HEAD (missing symbol named)"; the behavioural RED proof is K1-K8, measured on the STAGE_P_HEAD build.

### S0 -- Stage P = ruling-bf9.md Stage P with amendments 1-17, VERBATIM (one builder context)
C0 lever (ADNA_INSPECT_LAYER) -> C1 RED tests (amendments 5, 6, 8, 9, 10, 11; test_compositor.cpp:423-437 deleted; the
test-target rename; tool_uitoggle_snapshot) -> C2 removal readers-first (P4 ApiServer -> P3 LayerInspector -> P2
Compositor / Renderer / DeckClock + amendment-7 rewords -> P1 Layer.h/.cpp) -> C3 probes (amendment 12) -> C4 docs
(amendment 13). Record STAGE_P_BASE / STAGE_P_HEAD. Gates G0-G7 at STAGE_P_HEAD. Never merged alone.
Later stages deliberately supersede three S0 artifacts: test_deck_clock case (b) and probe row d_single_advance
(DeckClock is deleted in S2), and the persistent-key lint gains exactly ONE allow-listed file (ShowMigration.h, K7).

### S1 -- the tuple names a deck (one builder context; inert)
S1.1 NEW src/model/ClipRef.h: `struct ClipRef { uint32_t deckId = kNoDeck; int column = -1; bool valid() const;
     bool operator==(const ClipRef&) const = default; static constexpr uint32_t kNoDeck = 0xFFFFFFFFu;
     static constexpr uint32_t kMaxDeckId = 0x3FFE; static constexpr int kMaxColumn = 0x3FFE; }`.
     model/Layer.h: LayerRuntimeSnapshot gains `uint32_t activeDeckId, previousDeckId, pendingDeckId` (default kNoDeck)
     and `activeRef() / previousRef() / pendingRef()`; LayerRuntimeCell::pack / unpack encode them per F2 (0x3FFF <->
     kNoDeck); kMaxPendingColumn becomes ClipRef::kMaxColumn. No caller changes; word size and the static_assert stay.
     RED: tests/test_layer_runtime.cpp NEW "LayerRuntimeCell packs a ClipRef per slot: deck ids and columns round-trip
     at 0 and the limits, a ref without a deck round-trips, pending + snap still share the last word (bf9b S1)"
     [layer_runtime] -- does not compile on STAGE_P_HEAD (no activeDeckId). The pinned :48-51 case changes (4.B).
     GREEN: passes; every other existing test unchanged; `.harmony/probe-tsan-unit.sh` exit 0 (Pitfall 63).
     Risk: an off-by-one in a 14-bit field. Test: the limit cases (deck 0x3FFE + column 0x3FFE in the pending slot =
     0x0FFFBFFF, under the 28-bit mask).

### S2 -- THE SWITCH (one stage; builder context 2a = src/, 2b = tests/; one review unit)
A type split: the app target builds at 2a's end, the test targets at 2b's end -- the one sanctioned non-building
window, never merged; every other commit builds both. SITE RULEBOOK -- every compile error is resolved by exactly one
rule; a site no rule decides is a STOP (back to Harmony for an architect amendment), never a guess:
  R1 a layer SETTING reached through a deck -> `comp.layers[i]`.
  R2 the TUPLE reached through a deck -> `comp.layers[i]`; any column compare is a ClipRef compare
     (`rt.activeRef() == ClipRef{deck.id, col}`), never `activeClipColumn == col` alone.
  R3 clip STORAGE -> `deck.rows[i]` (`deck.getRow(i)->getClipAt(c)`).
  R4 "the clip layer i is playing" -> `comp.playing(i)` / `comp.playingClip(i)` (C1).
  R5 a loop over WHAT PLAYS (render, autopilot, preset playlists, layer connections, pad lights, preview) -> the shared
     stack (`forEachLayer`); a loop over EVERY CLIP (media presence / reopen / dispose, thumbnails, clip connections,
     image preload, id re-mint) -> `forEachClip` (live AND retired decks).
  R6 `getActiveDeck()` / `activeDeckIndex` in a PLAYING context -> the shared stack; in a BOX context (grid, the fire
     entries resolving a cell, column fire, Save / Rename / Duplicate / Remove Deck, ByPosition bindings) -> unchanged.
  R7 a deck-switch path does exactly: set activeDeckIndex, setActiveDeck (fence token), DeckView::showDeck, the
     existing take capture. Nothing else.
S2.1 MODEL TYPES (src/model).
  - NEW src/model/ClipRow.h: `struct ClipRow { std::vector<std::optional<Clip>> clips; getClipAt (const + non-const),
    setClip, clearCell, ensureColumns, getNumColumns; juce::var toVar() const /* {"clips":[...]} */; void
    fromVar(const juce::var&) /* reads "clips" only */; }` -- the clip half of today's Layer, moved verbatim.
  - Layer.h/.cpp: `clips` and the clip helpers leave Layer; toVar / fromVar carry settings only. UI pointers (`Layer*`)
    now point into `Composition::layers`.
  - Deck.h: `std::vector<ClipRow> rows` replaces `layers`; getRow(i), getNumRows(); initDefault(int numRows);
    add / removeColumn act on every row; addLayer / removeLayer / moveLayer / triggerColumn / nextLayerId_ leave Deck.
    toVar writes {"name","id","numColumns","layers":[{"clips":[...]}]} (the key "layers" is kept so Load Deck's shape
    check and one reader serve old and new deck files). The nothrow-move static_assert stays.
  - Composition.h: `std::vector<Layer> layers`; getLayer(i), getNumLayers(), topLayerIndex(); private nextLayerId_
    (100 up) and retiredDecks_; findDeckById(id) (live, then retired), findDeckIndexById(id) (live only, else -1);
    `Clip* clipAt(ClipRef, int row)`; playing(i) / playingClip(i) (C1); rowClips(i); forEachLayer / forEachClip (C2);
    fire(layer, deckIndex, column, forced, immediate); triggerColumn(deckIndex, column, forced); structure ops
    insertLayer / eraseLayer / moveLayer (every live + retired deck's rows kept in step), appendDeck (pads rows; mints an
    id <= kMaxDeckId, the lowest free id once the counter passes it), retireOrEraseDeck, restoreRetiredDeck,
    reapRetiredDecks (returns the reaped decks; the caller disposes their media); migrationNote (not serialized, like
    routineLoadNote). initDefault: 3 shared layers (ids 0,1,2; layer 0 Opaque, others Transparent, as Deck::initDefault
    today) + one deck "Deck 1" with 3 rows.
  - RowClips (ClipRef.h): `{ Clip* (*fn)(void* ctx, uint32_t deckId, int row, int column); void* ctx; int row;
    Clip* at(ClipRef) const; }` -- trivially copyable, never std::function (the GL thread calls it).
S2.2 TRIGGER API (Layer.h): triggerClip(ClipRef, const RowClips&, forcedSnap, maxAttempts, std::optional<ClipRef>
  onlyIfActive); triggerClipImmediate(ClipRef, const RowClips&, maxAttempts); processPendingTrigger(beatInBar, barCount,
  const RowClips&, maxAttempts = 16); clearActiveClip(const RowClips&, onlyIfActive, maxAttempts); releaseMomentary(
  ClipRef, const RowClips&). The CAS machinery, the 16-attempt GL bound, the tails and their order are unchanged; the
  tails resolve through RowClips; retrigger = the same ClipRef (the same column from another deck is a NEW clip and
  crossfades); an empty target cell still clears the layer.
S2.3 FILES (Composition.h, Deck.h, NEW src/model/ShowMigration.h, core/CompositionLoad.h, MainComponent load paths).
  - New composition shape: top-level "layers" (settings) + "decks" (rows of clips); "persistent" and
    "globalTransitionSpeed" are never written; ShowMigration is the ONLY src reader of either legacy key.
  - Old composition: F7; ONE note in migrationNote, logged once by loadComposition with logLine, e.g. "old show
    converted: layer settings from Deck 1 (Deck 2 rows 0-2 settings dropped: <names>); 'persistent' ignored on: Deck 1 /
    Layer 2; deck fade 1.20 s dropped".
  - Load Deck: old deck files keep their clips per row; F6 for extra rows; new deck files carry rows only.
  - validateComposition: >= 1 layer; every deck rows == layers (pad); deck ids <= kMaxDeckId (re-mint above it, like the
    duplicate-id re-mint, VERIFIED-R Composition.h:577-589). validateDeck: >= 1 row; numColumns <= 10000 (unchanged).
  - compload::duplicateDeck: rows copied, clip ids re-minted (Pitfall 36); there is no tuple to clear any more.
  - compload::imagePaths: playing clips first (any deck), then the shown deck's other cells, then the other decks.
S2.4 COMMANDS (core/DeckCommands.h, TriggerCommands.h, ClipCommands.h, UndoService.h, EffectScope.h).
  - TriggerClipCmd: addressed by shared layer index; before / after tuples carry deck ids; playing flags resolved via
    the composition; merges with the same layer only.
  - cancelPendingTriggers(Deck&), its restore helper and every caller (SwitchDeckCmd, AddDeckCmd, InsertDeckCmd,
    RemoveDeckCmd, onDeckSwitched, handleDeckSwitch) are deleted (F11). RemoveDeckCmd alone uses the new
    `cancelPendingInto(Composition&, uint32_t deckId)` for queues into the deck it retires or erases.
  - ClearLayerClipsCmd(deckIndex, layerIndex): clears that deck's row; snapshots row + shared tuple; clears the tuple
    ONLY when its active ref is in that row of that deck.
  - ClearActiveClipCmd / ToggleLayerFlagCmd / AddLayerCmd / RemoveLayerCmd / MoveLayerCmd: the shared layer by index;
    Remove snapshots the Layer + every deck's row (keyed by deck id, live and retired); all fenced as today.
  - AddDeckCmd / InsertDeckCmd: no queue cancel; InsertDeckCmd adds shared layers for a wider deck (F6), undo removes
    them. RemoveDeckCmd: retire-or-erase (F3) + reap; undo restores the retired deck or re-inserts the snapshot.
  - SwitchDeckCmd(before, after, label): index + activate hook only.
  - UndoService (VERIFIED-A UndoService.h:50-68): resolveLayer(int layerIndex) (shared), resolveRow(deckIndex, row),
    resolveClip(deckIndex, row, column). EffectScope::layer: the shared layer (deckIndex ignored).
S2.5 RENDER (render/CompositorEngine.{h,cpp}, Renderer.{h,cpp}, LayerStateKey.h, EmbeddedShaders.h; render/DeckClock.h
  and model/AutopilotBank.h deleted).
  - `GLuint compositeShow(Composition& show, ...)` replaces compositeDeck(Deck&) (CompositorEngine.h:144): per layer i
    ONE `rt = layer.runtime()`, `clip = show.clipAt(rt.activeRef(), i)`; renderLayerStages / applyTransition (:543 /
    :524) take the RowClips to resolve the outgoing clip; keys `clipChain(kShowStackKey, layer.id)` etc.; saveLayerOutput
    / heldLayerOutput (:566 / :287) with kShowStackKey. Layer gate, order, blend and keying unchanged.
  - Renderer::renderOpenGL: delete the deck-switch detection (:464-497) and the deck fade (:910-965) with prevDeckFBO_ /
    prevDeckTexture_ / deckTransitionProgress_ / deckTransitionSpeed_ / prevActiveDeckIndex_, the compile at :1962 and
    EmbeddedShaders::deckTransition; delete the off-screen-deck loop (:773-791); one `Autopilot showAutopilot_` (the
    bank's per-type config + smart-random forwarding moves into Renderer), called once per frame inside the deckActive
    gate; the preset-playlist loop (:595-686) walks the shared layers' playing clips; the Layer Router (:1443-1455) reads
    `composition_->layers[idx].id`. The view()'s pointer is a fence flag only.
S2.6 AUTOPILOT (model/Autopilot.{h,cpp}): `processFrame(Composition&, const FeatureSnapshot&, FrameReport*)`; per shared
  layer: end-of-video on playingClip(i); queued fires through rowClips(i); advanceClip / smartAdvanceClip step within the
  playing clip's SOURCE deck row (F10), firing ClipRef{source.id, next} with onlyIfActive = rt.activeRef() and
  kRenderTriggerAttempts.
S2.7 MESSAGE-THREAD PATHS (MainComponent.cpp; lines VERIFIED-R unless marked).
  - handleClipTrigger (:4600-4772): resolve the cell on the given / shown deck, fire into `comp.layers[layer]` via
    comp.fire. The `resolvedDeckIndex == activeDeckIndex` gate (:4654) goes: hasBeenTriggered, the beat-snap seek
    (:4677), the retrigger seek (:4699), the NEW C3 branch (F13, new member `syncActivatedPlayhead(Clip&)`) and the
    preview refresh run for EVERY fire (every fire lands in the stack on screen).
  - handleColumnTrigger (:4825-4960): comp.triggerColumn(deckIdx, column, forced); undo composite and capture as today;
    C3 per newly activated layer; UI refresh unconditional; the column header remembers {deckId, column}.
  - applyClearActiveClip (:5565-5590): the shared layer.
  - handleDeckSwitch (:5488-5549) and onDeckSwitched (:1417-1441): R7 exactly (no queue cancel, no preview refresh).
    Deck tabs, their menus, Save / Rename unchanged (a tab click is handleDeckSwitch).
  - refreshPreviewFromActiveClip(Deck&) (VERIFIED-A :4972) -> refreshPreviewFromShow(): the first playing Image /
    Source clip of the shared stack; called after fires / clears / swaps, never on a deck switch.
  - Genre auto-switch (:688-700): F15 inert + one logLine per session.
  - onLayerSelected (:746-754): inspectLayer(&comp.layers[i], EffectScope::layer(-1, i)); X button (VERIFIED-A :762):
    routineEngine_.stopOnLayer(layerIdx); after-undo re-point (:5220-5229): resolveLayer(selLayer).
  - swapCompositionModel (:2988-3033): old playable ids via forEachClip (retired decks included, so their media
    closes). removeDeck (:3750-3775): RemoveDeckCmd with the media hook for reaped decks. duplicateDeck / loadDeck /
    finishStagedLoad (:3195-3250, :3475-3558, :3704-3748): rows only; F6. Drop handlers (:1157, :1286, :1345, :1607):
    deck rows (R3). dispatch.fire (:1958-1996): activeClip unchanged call; applyLayerFlag -> the shared layer;
    activeDeck -> handleDeckSwitch (grid only, K7).
S2.8 BINDINGS / OSC / REST (handleBindingAction :7538-7700; api/ApiServer.cpp).
  - Selected (VERIFIED-A :7543-7557): the first shared layer with an active clip; fires that clip's own ref (deck =
    findDeckIndexById(activeDeckId); a retired source is skipped).
  - ThisItem (VERIFIED-A :7560-7577): search EVERY live deck for targetClipId and fire it from its own deck into its
    row's layer ("firing a clip from ANY deck"); not found -> ByPosition on the shown deck (today's fallback).
  - Momentary release (:7599-7613, :7638-7650): the press records the ClipRef it fired per layer; the release releases
    exactly those refs (a deck switch between press and release can no longer strand a clip on).
  - OSC routes and REST trigger / switch routes: unchanged calls (shown deck). GET /api/composition (:389-437): F8.
S2.9 RECORDING (recording/RoutineEngine.{h,cpp}, Program.cpp, PerfState.{h,cpp}, PerfStateCapture.cpp,
  ui/RoutineDeckView.h).
  - `RoutineEngine::stopOnLayer(int layer)` (was (deck, layer)): stops every running routine touching that shared
    layer, whatever deck it fired from.
  - deriveRoutineDeckView: bands for every running routine on its shared layer, whatever deck is shown (shownDeck stays
    for labels only).
  - Program::compile: Layer-scope keys resolve to the shared layer by name / position (deck ignored); Clip scope as today
    (D2) on rows. Preamble (Program.cpp:187-245): shared-layer runtime + flags from PerfState v2 `layers`, or for an old
    take from `decks[activeDeckIndex]` only; clip runtime per deck as today; the activeDeck point stays (grid only).
  - PerfState v2: + `std::map<int, LayerRuntime> layers` (LayerRuntime + `int activeDeck = -1`, `std::string
    activeDeckName`) written as "layers"; old takes have none. PerfStateCapture captures the shared layers once and,
    per deck, only clip runtime.
S2.10 ConnectionEngine::tick (VERIFIED-A ConnectionEngine.cpp:339-341) -> forEachLayer + forEachClip (C2; the reference
  user for bf6 / bf45). MidiOutputHandler::updateFromDeck(const Composition&, int shownDeckIndex) (caller VERIFIED-A
  MainComponent.cpp:4170): a pad lights iff that shown-deck cell is the active ref of its row's layer.
S2.11 UI WIRING, no new visuals. DeckView::rebuildGrid / refresh (VERIFIED-R DeckView.cpp:130-324): strips over
  comp.layers (`strip->setLayer(&comp.layers[i], i, &comp)`), cells from the shown deck's rows; a cell is active iff
  `rt.activeRef() == ClipRef{shown.id, col}`; viewport height from comp.layers. NEW DeckView::showDeck() (F16): keeps
  the strips, re-points the cells. LayerStrip::setLayer(Layer*, int, const Composition*): every
  `layer_->getActiveClip()` (VERIFIED-A LayerStrip.cpp:372-406, :718, :744) -> `show_->playingClip(index_)`.
  LayerInspector: same Layer* type, now a shared layer.
S2.12 TESTS (2b). NEW tests/ShowFixture.h (makeShow(decks, layers, columns), row(comp, d, i), fire(comp, i, d, c)) so
  the ~600 test sites follow one pattern. NEW tests/test_show_model.cpp [show] and tests/test_show_migration.cpp
  [show][backcompat] (CMake targets; RED = do not compile on STAGE_P_HEAD):
  T1 SwitchDeckCmd execute / undo / redo and a 0->1->0 index walk leave every tuple, every clip's playing /
     playheadPosition / beatsPlayed and every queued trigger byte-identical.
  T2 firing (deck 1, row 1, col 2) while deck 0 is shown puts ClipRef{deck1.id, 2} in layer 1; playingClip(1) is that
     cell.
  T3 layer 0 plays (deck 0, col 2); firing (deck 1, col 2) is a crossfade (previous = deck 0 / col 2), not a retrigger.
  T4 triggerColumn(shown, c) fires the shown deck's column into every non-ignoring layer and leaves an Ignore Column
     layer playing its other-deck clip (K4 at unit level); an empty cell clears its layer (F12).
  T5 a Bar-snapped queue on deck 0 survives a switch to deck 1 and fires on processPendingTrigger(beatInBar 0) (K5).
  T6 Remove Deck while its clip plays: retired, still resolves, a queue into it cancelled, playingClip unchanged; undo
     moves it back with the live playhead; after the layer is replaced and a deck-structure command runs, it is reaped.
  T7 Add / Remove / Move layer keep rows == layers in every live and retired deck; RemoveLayerCmd undo restores the
     layer and every deck's row deep-equal.
  T8 Duplicate Deck: new deck id, re-minted clip ids, no ref names the copy; the source keeps playing.
  T9 C1: one runtime() load; nullptr when clear or the cell is empty; points into a non-shown and into a retired deck.
  T10 C2: forEachLayer once each; forEachClip once each, live decks by index then retired, rows / columns ascending.
  T11 deck-id cap: a file deck id 50000 loads <= kMaxDeckId; appendDeck past the counter cap takes the lowest free id.
  T12 Autopilot (F10): advances within the playing clip's source deck while another deck is shown, not when the source
     is retired; one show autopilot (Pitfall 38 successor): processFrame once per frame advances each layer once over
     4 beats.
  M1 a 2-deck legacy show (3 vs 4 layers, different opacity / blend / type, one "persistent": true, deck fade 1.2 s)
     -> 4 shared layers; 0-2 = Deck 1's settings, 3 = Deck 2's layer 3; every deck 4 rows; ONE note naming the dropped
     sets, the persistent layer and the dropped fade.
  M2 legacy -> load -> save -> load -> save: the two saves are equal; the saved form has top-level "layers", rows with
     only "clips", no "persistent", no "globalTransitionSpeed" (the round-trip test K7 asks for).
  M3 Load Deck of an old 5-row deck into a 3-layer show adds 2 shared layers with that file's settings; a new-format
     deck adds layers with defaults; other decks padded; undo removes the deck and the added layers.
  M4 colliding legacy layer ids across decks are re-minted unique (Pitfall 15: id 0 stays valid).
  M5 an old take (PerfState without "layers") restores shared layers from its captured active deck; a v2 take
     round-trips "layers".

### S3 -- UI visuals (one builder context) + VISUAL WORK GATE
S3.1 LayerStrip (bf9b's file per ruling-ui): the existing clip-name row (VERIFIED-A clipNameBounds_, LayerStrip.cpp:692)
     names the box too: "<clip> - <deck>"; the deck part dim when it is the shown deck, normal text colour when it is
     another deck, "(removed deck)" when retired; set from refresh() compare-before-set (Pitfall 41); repaint only the
     name rect on change (Pitfall 57); never AudioDNALookAndFeel::kRoutineCue (reserved).
     RED: tests/test_layer_strip_source_deck.cpp "LayerStrip names the deck a playing clip came from: dim for the shown
     deck, normal for another, '(removed deck)' for a retired one" -- does not compile on STAGE_P_HEAD (3-arg setLayer).
S3.2 Grid: a clip playing from another deck lights no cell in the shown deck; active / queued styles only for refs into
     the shown deck; the column-header highlight shows only on the deck it was fired from; showDeck keeps strip widgets
     untouched across a switch (F16). RED: headless DeckView cases in the same new file (setVisible, Pitfall 34),
     including "a 0 -> 1 -> 0 showDeck walk leaves every LayerStrip child the same object".
S3.3 TopBar: delete the Fade label + slider (VERIFIED-A TopBar.h:120-122; TopBar.cpp:191-205, :624-625) and
     fadeSliderBoundsForTest (TopBar.h:61); the Master Signal keeps its order after its new left neighbour;
     test_master_signal_link.cpp :189 / :300 / :340 re-anchored to that neighbour (4.B).

### S4 -- probes, docs, lane report (one builder context)
S4.1 Rename (history kept) .harmony/probe-deck-clock.{sh,py,json} to .harmony/probe-boxes.{sh,py,json}. Rows:
     k1a_switch_static, k1b_switch_video, k1c_switch_midfade, k1t_history_freeze, k1t_history_feedback,
     k2_nothing_unseen, k3_autopilot, k4_ignore_column_across, k5_queue_link_off, k5_queue_link_on, k7_old_show,
     k7_old_take, k8_twenty_decks (old-row map in 4.B). Drivers: REST switch_deck / trigger_clip / trigger_column,
     /api/debug/load_deck and /api/debug/duplicate_deck (they activate the new deck), inject_features for beats; a
     switch path with no non-synthetic driver (OSC, MIDI binding, add / remove deck) is covered by T1 / T6 + lint B4d
     (K1's own rule). quit_ours from S0.
S4.2 Docs per section 6. S4.3 Lane report: STAGE_P_BASE / HEAD, BF9B_HEAD, every K row's RED (STAGE_P) / GREEN (BF9B)
     numbers, B2's added / retired lists, Q1-Q4 defaults taken.

### BUILD STAGES (one builder context each, dependency order)
| Stage | Depends on | Ships alone? |
|---|---|---|
| S0 Stage P | -- | No (ruling-bf9 amendment 1) |
| S1 tuple ClipRef | S0 | Inert, so technically yes; it merges inside bf9b's single merge |
| S2a src switch | S1 | No -- it IS the behaviour (with S2b) |
| S2b tests | S2a | No |
| S3 UI visuals + B7 | S2b | No |
| S4 probes + docs + report | S3 | No |

### 4.A CONTRACTS FOR LATER LANES (bf9b is built FIRST; after it, these names are the API)
- C1 (bf6) the playing-clip accessor: `const Clip* Composition::playingClip(int layerIndex) const` (+ non-const) and
  `Composition::PlayingClip Composition::playing(int layerIndex) const` -> `{ Clip* clip; ClipRef ref; int deckIndex
  /* -1 = retired or none */; bool retired; }`. ONE `runtime()` load; the INCOMING (active) clip; nullptr when the layer
  is clear or its cell is empty; may point into ANY deck box, retired ones included. Message thread (and the GL thread
  inside its deckActive gate).
- C2 (bf6, bf45) the walk: `template <class Fn> void Composition::forEachLayer(Fn&&)` -> fn(Layer&, int layerIndex), each
  shared layer exactly once, bottom to top; `template <class Fn> void Composition::forEachClip(Fn&&)` -> fn(Clip&, const
  ClipSite&), `ClipSite{ int deckIndex /* -1 retired */; uint32_t deckId; int row; int column; bool retired; }`, each
  clip of every deck box exactly once: live decks by index, then retired decks; rows and columns ascending. Const
  overloads. ConnectionEngine::tick (S2.10) is the reference user; bf45's ConnVisitor / describeWhere mirror it.
- C3 (bf6) `MainComponent::syncActivatedPlayhead(Clip&)`, called by handleClipTrigger and handleColumnTrigger for every
  NEWLY activated playable clip whose player / sequence exists (F13). bf6 does NOT build its Item 8.
- bf1 "the top layer's row in the shown deck" = `comp.getActiveDeck()->getRow(comp.topLayerIndex())`
  (`Composition::topLayerIndex()` = layers.size() - 1). Of which deck: the deck SHOWN when recording was pressed, pinned
  by `Deck::id` (never by index); at landing, `comp.findDeckIndexById(pinnedId) == -1` means removed or retired -- never
  land in a retired deck (bf1 owns the fallback; recommended: the shown deck). bf1's LayerKey {deckId, layerId}
  reduces to the shared layer id (bf1 R12): a layer is the same layer on every deck.
- Firing from code: `handleClipTrigger(layer, column, origin, deckIndex = -1 /* shown */, immediate)` and
  `handleColumnTrigger(column, origin, deckIndex = -1)` stay the ONLY entries (K4). Model level: `Composition::fire`,
  `Composition::triggerColumn`; Layer methods take a ClipRef + `Composition::rowClips(i)`.
- Clip storage `Deck::rows`, `Deck::getRow(i)`, `ClipRow::getClipAt(c)`; layer settings `Composition::layers` /
  `getLayer(i)` -- a deck has no layers any more.
- tsan-r5: re-scope against post-bf9b code (ruling-bf9 R5 stands): layer settings live in Composition::layers; the
  httplib reader walks `layers` + the live decks' rows (REST never reads retired decks).
- Everyone: what a layer plays is `Composition::playing(i)`, never `getActiveDeck()->rows[i]` (new Pitfall NN).

### 4.B EXPECTATION CHANGES BECAUSE OF THE RULING (pre-registered; any OTHER changed assertion = STOP and report)
Q-A = "when I switch between decks, do not change the clips playing in the layers or how they are playing."
Q-B = "Whatever is in the layer should be what is playing and there should be nothing else."
Q-C = "treat the decks as just a box of clips and I can switch between 20 decks looking for a clip and the playing will
not be affected."
| Row / case | Old expectation | New expectation | Quote |
|---|---|---|---|
| probe-deck-clock d_fade_finishes | an off-screen deck's fade finishes | INVERTED -> k1c_switch_midfade (K1c: the fade continues on screen across the switch, completes by T + 1 s) | Q-A |
| d_single_advance (S0 name) | one advance per tick | RETIRED with DeckClock; successor K1c's "p rises >= 0.15" | Q-A |
| d_pending_trigger_still_cancelled | the left deck's queue is cancelled | INVERTED -> k5_queue_link_off / _on (K5: fires on its bar) | Q-A |
| d_video_keeps_time, d_imageseq_keeps_time | an off-screen deck's media keeps time | INVERTED -> k1b_switch_video (the layer's clip keeps time on screen) + k2_nothing_unseen (a box clip not in a layer never advances) | Q-B |
| d_autopilot_keeps_time | an off-screen deck's autopilot runs | INVERTED -> k3_autopilot | Q-C |
| d_return_hitch | no hitch coming back to a deck | RETIRED; its 50 ms bar moves to k8_twenty_decks | Q-C |
| probe-render-state p_flag_ignored | VALID iff control == A only | VALID iff control == what the shared layers play after persist_setup (A alone when B's row is A's row, else A over B: B stays in its layer across the switches); PASS clause unchanged (subject == control). The builder records which case the fixture is | Q-A |
| probe-render-state p_flag_ignored_empty | VALID iff control is black | VALID iff control == B's picture when B's layer is not replaced (else black); PASS clause unchanged | Q-A |
| tests/test_layer_runtime.cpp:48-51 | kMaxPendingColumn == 268,435,454 | == ClipRef::kMaxColumn (16,382); the pendings[] list follows | Q-C (a layer must name the box its clip came from) |
| test_undo_commands.cpp:2840 SwitchDeckCmd cancels... | cancel / restore / re-cancel | execute / undo / redo leave the queue untouched | Q-A |
| :1607 InsertDeckCmd, :2909 AddDeckCmd cancel... | cancel | untouched | Q-A |
| :2946, :2989 RemoveDeckCmd undo cancels on reactivation | cancel | untouched (a reactivation changes only the grid); NEW T6 for queues INTO a removed deck | Q-A |
| :886, :973 ClearLayerClipsCmd | row + runtime cleared | that deck's row cleared; the tuple only when it plays from that row of that deck; NEW SECTION: another deck's clip keeps playing | Q-B |
| :1207, :1231, :1311, :2614, :2654 Add / Remove / Move layer | one deck's layer | the show's layer + every deck's row | consequence text "every deck shows the show's layer rows" |
| test_deck_clock.cpp (a), (b), (c), (d) | DeckClock behaviour | RETIRED with DeckClock (successors T1, K1c, K2; (c)'s gate lives only in compositeShow, unchanged) | Q-A, Q-B |
| test_deck_clock.cpp (e) LayerClock | -- | KEPT, moved to tests/test_layer_clock.cpp, unchanged | -- |
| test_deck_clock.cpp (f) AutopilotBank | per-deck baselines | RETIRED -> T12 (one show autopilot) | Q-C |
| test_layer_runtime.cpp:156 | cancelPendingTriggers(deck): never fires | same expectation via cancelPendingInto(comp, deckId) | -- |
| test_layer_runtime_race.cpp:124 R1 | GL driver DeckClock::tick | driver LayerClock::tick per shared layer + the show autopilot; expectation unchanged | -- |
| test_composition.cpp:1322 imagePaths | the active deck and its active clips first | playing clips first (any deck), then the shown deck, then the others | Q-B |
| test_master_signal_link.cpp:189 / :300 / :340 | left neighbour = the Fade slider | left neighbour = the control before the old Fade section | consequence text "the deck-to-deck transition fade is REMOVED" |
| test_program_preamble.cpp:196 | a Layer-scope key with a missing deck is unresolved | Layer scope resolves the shared layer (only Clip scope can miss a deck); the builder confirms per SECTION | Q-B |
Must stay green AS THEY ARE: every other test (fixture rewrite only); probe-crossfade (an OLD-format POST, so it now also
exercises the converter), probe-deck-path, probe-deck-tabs (R5 still activeDeck == 2), probe-routines and
probe-routine-display (they read decks[0].layers[*] through the F8 mirror), probe-render-state's other rows
(p_api_no_field: the mirror keeps id / visible / activeClipColumn and has no "persistent"), probe-tsan.

### 4.C FILES SHARED WITH OTHER LANES (bf9b merges first; the others rebase onto it)
| File | bf9b blocks | Other lanes |
|---|---|---|
| src/MainComponent.cpp | handleClipTrigger :4600-4772, handleColumnTrigger :4825-4960, handleDeckSwitch :5488-5549, onDeckSwitched :1417-1441, onLayerSelected + X :746-762, refreshPreview* :4972, genre :688-700, swap :2988-3033, deck load / duplicate / remove :3195-3250 and :3475-3775, dispatch.fire :1958-1996, timerCallback :4170 / :4190, handleBindingAction :7538-7700, drop handlers | bf6 (Timeline; its Item 8 is now bf9b's C3), bf1 (landRecordedTake, buildBindableTargets, ApiServer hooks), ui (rename paths), tsan-r5 |
| src/ui/DeckView.{h,cpp} | rebuildGrid / refresh :130-324, NEW showDeck, column-header state | ui (tab rename: setupDeckTabs, DeckTabButton), bf1 (setRecordMark) |
| src/ui/LayerStrip.{h,cpp} | setLayer, every getActiveClip site, the name row | none (bf9b's file per ruling-ui) |
| src/ui/TopBar.{h,cpp} | Fade section (TopBar.cpp:191-205, :624-625; TopBar.h:58-61, :120-122) | bf2 (sync dial), tsan-r5 |
| src/render/CompositorEngine.{h,cpp} | compositeDeck -> compositeShow; renderLayerStages / applyTransition / save / held signatures; keys | bf1 (one const accessor), tsan-r5 |
| src/render/Renderer.{h,cpp}, EmbeddedShaders.h | renderOpenGL :388-970 regions, router :1423-1455, :1962; the deckTransition string | bf1 (tapClipRecorder at the exits), bf10, tsan-r5 |
| src/model/Layer.{h,cpp}, Deck.h, Composition.h | the split (bf9b does NOT edit Clip.h) | bf6 / bf45 (Clip.h), tsan-r5 |
| src/connect/ConnectionEngine.cpp | tick traversal :339-341 | bf6, bf45 |
| src/api/ApiServer.cpp | /api/composition :389-437 | bf1 routes, ui, tsan-r5 |
| src/recording/* + src/ui/RoutineDeckView.h, src/midi/MidiOutputHandler.* | stopOnLayer, Program, PerfState*, bands, pad lights | none listed |

## 5. GATES (Harmony, on the merge-ready head; pre-registered)
Rig: ONE live app under lock.sh acquire_quiet_lock; `open -g ... --args --test-mode`; never an Output window; no
synthetic input; window-only captures by Quartz window id; HTTP Connection: close; quit_ours only; ps checked for CPU
burners before any perf verdict; background runs <= 2 h. Arms: STAGE_P = the STAGE_P_HEAD app copied to a scratch path
before any later rebuild; BF9B = the BF9B_HEAD app.
G0-G7  Stage 0's gates, VERBATIM from ruling-bf9.md "FINAL CONSOLIDATED GATE LIST" (:321-454), at STAGE_P_HEAD.
K1-K8  ruling-bf9.md "bf9b CONTRACT GATES" (:455-516) with their requirement-to-gate table, VERBATIM bars (floor =
       max(1.5, 4 x noise)); RED on STAGE_P, GREEN on BF9B; driven by .harmony/probe-boxes (S4.1). bf9b refines only
       fixtures / drivers: K2 reads every clip's playheadPosition from GET /api/composition decks[*].layers[*].clips[*]
       (the F8 mirror) and "is in a layer" from the NEW top-level layers[*].activeClip / previousClip; K7's logLine
       pattern is "old show converted:" (one line per load).
B1 BUILD: `cmake --build build --config Release -j$(sysctl -n hw.ncpu)` -> rc 0 at BF9B_HEAD, all targets; every commit
   outside S2's sanctioned window builds the app and the tests (rc per commit in the lane report).
B2 UNIT: `ctest --test-dir build -j8 --output-on-failure` -> 0 failures; count(BF9B_HEAD) == count(STAGE_P_HEAD) +
   added - retired, both lists in the lane report. Required present and passing: the S1 packing case, T1-T12, M1-M5,
   the S3 strip + grid cases, test_layer_clock (e), test_render_thread_lint with its pins re-justified in the commit.
B3 TSAN (Pitfall 63): `.harmony/probe-tsan-unit.sh` -> exit 0 and `ctest -L tsan` all PASS, at S1's end AND at BF9B_HEAD.
B4 TEXT (grep gates; each hit listed file:line):
   a) src/: zero `DeckClock|AutopilotBank|deck_transition|deckTransition|prevDeckFBO_|globalTransitionSpeed|
      cancelPendingTriggers|compositeDeck\b`, except the literal JSON key strings in src/model/ShowMigration.h.
   b) src/render/* and src/model/Autopilot.cpp: zero `getActiveDeck(` and zero `activeDeckIndex`.
   c) Renderer.cpp: the pointer from `activeDeck_.view()` is never dereferenced (null / fenced tests only).
   d) the bodies of handleDeckSwitch, onDeckSwitched, SwitchDeckCmd::apply and DeckView::showDeck contain none of
      `triggerClip|clearActiveClip|setRuntime|updateRuntime|cancelPending|refreshPreview`.
   e) S0's lint "no Persistent-feature identifier left in src/" passes with exactly ONE allow-listed file,
      src/model/ShowMigration.h.
B5 OLD FILES (live; K7 + M1-M5): POST /api/load_composition with (i) a 2-deck pre-bf9b show (differing layer settings,
   one "persistent": true; built with probe-crossfade's JSON helpers) and (ii) a show holding a pre-bf9b take with an
   activeDeck lane. PASS iff GET /api/composition shows the first deck's layer settings, exactly one "old show
   converted:" logLine, the take fires without error and its activeDeck lane changes only activeDeck.
B6 PERF (INFO): INTERLEAVED A/B, STAGE_P vs BF9B, >= 5 runs per arm alternating, 60 s each: (i) a 1-deck, 3-layer steady
   scene, mean frame_time_ms; (ii) a 20-deck show with 3 playing layers (STAGE_P also ticks 19 off-screen decks).
   Report means + SD. Rule: INFO; a BF9B regression on (i) > max(1.0 ms, 3 x pooled SD) STOPS the merge for a look;
   (ii) is expected to improve and is never a bar.
B7 VISUAL WORK GATE: BEFORE = STAGE_P app, AFTER = BF9B app; states from composition files + REST (+ S0's
   ADNA_INSPECT_LAYER lever); `screencapture -x -o -l <CGWindowID>` of the largest on-screen window whose owner pid is
   ours; decoded. States: (1) deck 0 shown, layer 1 playing deck 1's clip (strip names it, no lit cell in that row);
   (2) deck 1 shown (that cell lit); (3) a removed deck's clip still playing ("(removed deck)"); (4) the TopBar without
   "Fade:"; (5) the column header lit only on the deck it was fired from; (6) ruling-bf9 G7's Layer-tab before / after
   (amendment 17). Critic panel BEFORE Boris -- visual-design: "is the deck name legible but quieter than the clip name,
   with no clipping at the strip's narrowest width?"; UX: "can Boris tell, from the strip alone, which box each playing
   clip came from and that the grid shows a different box?"; graphic-design: "does the TopBar close the Fade gap with no
   orphan space, and is the name row aligned with the thumbnail?"; logic: "does every lit cell and strip name match GET
   /api/composition?"; interaction-logic (dedicated -- deck switching is navigational and stateful): "after 0 -> 1 -> 0,
   is every highlight, strip name and fader where it was, and does nothing on screen move on a switch except the grid's
   cells?". Then ONE artifact page for Boris (section 8).
B8 POST-MERGE on main: B1, B2, B3, then K1-K8 once more on the merged build.

## 6. DOCS
- CLAUDE.md (S0 leaves 23,943 B; cap 25,000): rule 15 -> "15. **Decks are boxes; the layers play**: one shared layer
  stack (`Composition::layers`) plays whatever deck the grid shows; a deck switch changes only the grid; a layer names
  its clip by (deck id, column) (`docs/claude/performance-controls.md`)."; Project Identity: drop "cross-deck transitions
  with 3 blend modes", add "decks are boxes of clips over one shared layer stack"; trigger table :247 drop "or
  cross-deck transitions (P25)", :248 drop "persistent layers," if S0 left it; pitfall index 35 / 36 / 38 / 63 reworded
  + one NEW line. Net ~ +120 B (~24,065 B). Measure with wc -c; if over, move text into docs/claude.
- pitfalls.md: 35 -> per-chain history is keyed by the SHARED layer (`kShowStackKey`); no GL key carries a deck. 36 ->
  deck ids <= ClipRef::kMaxDeckId (16,382; the tuple packs them); a duplicate's re-minted clips never play (no ref names
  the copy). 38 -> one Autopilot for the show; never call processFrame twice in a frame. 63 -> the tuple packs (deck id,
  column) per slot; compare ClipRefs, never columns alone. NEW "Pitfall NN" (Harmony assigns; 64 looks claimed by mkvidx
  -- INFERRED from commit 9a832a0's subject): "The shown deck is the grid, never the screen -- before resolving what a
  layer plays: use `Composition::playing(i)` (a (deck id, column) ref, possibly another or a retired deck), never
  `getActiveDeck()->rows[i]`."
- performance-controls.md: replace "Inactive decks keep time" (:49-53) and S0's Persistent remnant with "Decks are boxes
  of clips (bf9b)": the model; fire rules (any deck -> its row's layer; a column fire = the shown deck, Ignore Column
  skips); queued triggers survive a switch; autopilot steps in the source deck; removed decks (retired); old shows; the
  strip's deck name; Layer Router (:16): "the shared stack's layers".
- rendering.md: delete "Cross-Deck Transitions (P25)" (:158 ff); the crossfade-history paragraph's keys.
- architecture.md: data model (Composition -> layers + decks -> rows -> clips; the GL thread resolves refs into any deck);
  Source Tree (+ ClipRef.h, ClipRow.h, ShowMigration.h; - DeckClock.h, AutopilotBank.h).
- recording.md: routines on shared layers (F9), stopOnLayer, bands on any deck, PerfState v2, old takes.
- integration.md: the /api/composition shape (F8); REST / OSC / MIDI fire the shown deck's cells.
- effects.md: the Autopilot section (one show autopilot; the source-deck rule; genre auto-switch inert, F15).
- APP-INVENTORY.md: deck / layer model lines; P25 and the TopBar Fade removed; the REST shape.
- BORIS_DECISIONS.md (:350) and .harmony/binding-decisions.md: "Built (bf9b)" under "Decks are boxes of clips", with the
  Q1-Q4 defaults taken. FEATURES.md / CONTEXT.md / design/FEATURE_INVENTORY.md: grep "cross-deck" / "deck fade" lines
  and reword or remove them.

## 7. RISKS (cheapest discriminating test first)
R1 A missed reader resolves "what plays" through the SHOWN deck -- this lane's bug class. Test: B4b / B4c, T1, K1a, K2.
   The type split turns most such sites into compile errors; rulebook R6 decides the rest.
R2 A tuple packing error (Pitfall 63). Test: S1's limit cases + B3 at S1's end, before anything uses deck ids.
R3 The GL thread reads a retired deck while it is reaped. Reaping happens only inside fenced mutations (RemoveDeckCmd,
   AddDeckCmd, InsertDeckCmd, the swap). Test: T6 + B3; extend test_layer_runtime_race R1 with a RemoveDeck / undo pair
   between fences if B3 reports anything.
R4 Strip churn on fast switching (K8's 50 ms). F16's showDeck keeps the strips. Test: K8 + S3.2's same-object case.
R5 Old shows whose decks gave one row different layer TYPES now play deck 2's clips through deck 1's type (a Mask row
   becomes Transparent). Inherent to "the first deck's layer settings win"; the ONE note names every dropped set.
   Test: M1 asserts the type source; Boris check 8.5.
R6 Old takes restore the wrong layer state. Test: M5 + probe-routines' unchanged rows (they replay takes).
R7 Autopilot on a layer whose clip came from a removed (retired) deck stops advancing until that layer is fired.
   Documented (F10); T12's retired SECTION.
R8 Pre-existing and unchanged: removing a column or swapping cells changes what a layer plays (the ref follows the
   CELL, 2.3). Not bf9b's to fix; a follow-up bug lane.
R9 Undo of a trigger whose ref names an already-reaped retired deck shows an empty layer until Remove Deck is undone
   (runtime undo is documented imperfect, DeckCommands.h:368-377). Accepted; T6 pins the normal path.
R10 Merge collisions in the 4.C files. bf9b merges first; the blocks are named; C1 / C2 / C3 names are fixed here.
R11 The F8 mirror could keep an old probe green over a model bug. Test: K2 and K4 read the NEW top-level layers.
R12 STRONGEST COUNTERARGUMENT to this plan: "(B) is half the diff; a ~1,200-site type split in a live performance app,
   built by agents, will regress something untested (bindings, pad lights, routines)." It loses: (B)'s failures are
   silent by construction, (A)'s are compile errors resolved by a written rulebook with a STOP rule, and the net is
   wide -- K1-K8, the B4 lints, every unchanged probe row, ~1,100 unit tests rewritten through one fixture. If Harmony
   wants a smaller first step, S1 alone is inert and safe; nothing smaller delivers Boris's behaviour.
R13 If Q1 comes back "stop": delete retireOrEraseDeck's retire branch and clear refs into the deck under the same fence
   (a cut); T6 flips; ~30 lines; nothing else moves.

## 8. WHAT ONLY BORIS CAN CHECK (one artifact page, after B7's critics)
8.1 Fire clips, then flip through 20 decks and back: the picture never changes -- fades finish, videos keep running,
    autopilot keeps its beat, a bar-snapped clip still lands on its bar.
8.2 Fire clips from two decks into two layers: each layer strip names the deck its clip came from; the grid lights a
    cell only on that clip's own deck. Is the deck name easy to read but not loud? (taste)
8.3 Ignore Column across decks: tick it on a layer, fire columns from other decks -- that layer keeps its clip.
8.4 Layer tab: "Persistent" is gone; "Ignore Column Trigger" sits alone on its row (from plan-bf9 8.2).
8.5 An old show (one that used Persistent, or gave decks different layer looks) loads with the FIRST deck's layer look.
8.6 The TopBar no longer has the deck "Fade:" control (nothing fades between decks any more).

## 9. QUESTIONS FOR BORIS (plain words; each has a default, so the build never waits)
Q1 "If you delete a deck while one of its clips is playing, should that clip keep playing until you fire something else
   on that layer?" DEFAULT: yes, it keeps playing.
Q2 "If you load a deck that has more rows than your show has layers, should the show add the missing (empty) layers so
   you can see all of that deck's clips?" DEFAULT: yes.
Q3 "Genre detection can be set to switch decks. Now that switching decks no longer changes the picture, should a genre
   change do nothing, or fire the first column of that genre's deck?" DEFAULT: do nothing (the grid stays where you
   left it).
Q4 "Clicking between decks counts as an Undo step today (Cmd+Z flips you back one deck). Keep that, or should Undo skip
   deck flips?" DEFAULT: keep it as it is.

ADDENDA (part of the body above):
- 4.B: tests/test_compositor.cpp:239-255 comments name the deleted deck-transition advance and globalTransitionSpeed;
  reword them -- no expectation change (the transitionProgressStep cases still pin the clip crossfade step).
- B4a / B4b scan code lines with comments stripped (S0's codeLines() helper, ruling-bf9 amendment 9); comment hits are
  reworded per ruling-bf9 amendment 7 ("src carries no new comment that names the removed feature").

STATUS: FINAL -- plan-bf9b: F1 restructure (Composition::layers + Deck::rows), F2 ClipRef (deckId, column) in the 16-byte tuple, F3 retired decks; stages S0 (Stage P verbatim) -> S1 -> S2a/S2b -> S3 (+B7 visual gate) -> S4; contracts C1/C2/C3 + bf1's top row named; gates G0-G7 + K1-K8 + B1-B8; Boris Q1-Q4 with defaults.

## HARMONY ADOPTION (s-rta-1002b, 2026-10-02 16:57:00) — overrides the ruling, which overrides the plan body
1. ADOPTED: .harmony/.reports/s-rta-1002b/ruling-bf9b.md IN FULL (27 amendments; stages S0 (= Stage P of plan-bf9.md as
   ruled in ruling-bf9.md, C0-C4, amendments 1-17) -> S1 -> S2a / S2b -> S2c -> S3 -> S4; the final gate list incl. K1-K8).
2. ADDED (relayed verbatim from ruling-bf10.md "HANDOFF to bf9b"): H1 the acceptance row m9b_deck_switch_live (20 decks,
   MilkDrop on deck 0 L0) — added to bf10's probe-milkdrop.py when bf10 has merged, else recorded as a follow-up row; H2 a
   deck switch never calls ProjectMSource::loadPreset / resize / releaseGL and never changes the canvas; H3 the MilkDrop
   preset-playlist advance (Renderer.cpp:594-683, today inside if (deckActive) over the ACTIVE deck's layers) must walk
   the PLAYING layers whatever deck is shown — otherwise browsing decks freezes a playing MilkDrop clip's playlist (it
   changes "how they are playing"); H4 note only (one shared ProjectMSource; filed F1).
3. BORIS QUESTIONS — defaults until he answers: Q1 a deleted deck's playing clip keeps playing until replaced; Q2 a loaded
   deck with more rows adds layers; Q4 Undo skips deck switches; Q5 firing a column with empty rows empties those layers.
4. PITFALLS: write "Pitfall NN" (Harmony assigns at merge; 64 = mkvidx). CLAUDE.md rule 15 rewording is in scope.
5. FENCE (concurrent BUILD lanes at launch): ui (VideoInfo, DeckTabRow / DeckView / MainComponent rename + Show in Finder,
   ApiServer test routes, ClipInspector info line), bf2 (analysis / beat clock / FeatureBus snapshot ring / AppSettings
   venues / ApiServer + OSC sync routes), mkvidx (VideoPlayer decode / seek). You will rebase over ui / mkvidx / bf2 as they
   merge: Harmony tells you when; source conflicts are resolved on your branch by you, preserving both sides' behaviour.
6. Harmony constraint: BORIS USES THIS MACHINE AND THIS APP — an Audio-DNA your lane did not start is his (s-rta-1002b
   incident): never quit / kill / touch it; the lock helper waits for it; if start_app refuses, stop the batch and release.
   The probes you run must quit ONLY the app they launched (ruling-bf9 amendment: probe safety). Visual gate (B7) on
   Harmony's decoded captures before Boris sees it. MERGE by Harmony.
