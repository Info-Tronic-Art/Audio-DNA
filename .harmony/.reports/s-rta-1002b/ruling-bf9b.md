# RULING lane bf9b (s-rta-1002b) -- the blind council's attacks on plan-bf9b.md (decks are boxes; one shared layer stack)

Architect (Fable), 2026-10-02, repo /Users/boriskarpman/projects/RealTimeAudio, HEAD c6720a0. Seat papers, verbatim:
.harmony/.reports/s-rta-1002b/attack-bf9b-papers.md (4 seats, 42 attacks). Inputs read: plan-bf9b.md (628 lines),
.harmony/boris-feedback-backlog.md, .harmony/binding-decisions.md "2026-10-02 (s-rta-1002b)", BORIS_DECISIONS.md:325-381,
ruling-bf9.md (Stage P + K1-K8), ruling-bf6.md AM-6, plan-tsan-r5.md, and the src / tests named below.
Labels: VERIFIED = read or run this session (file:line, or a scratch run); INFERRED = derived, not run; ASSUMED = stated
as such. Seat ids: LS = live-show, SC = state / concurrency, GT = gates / tests, UX = UX / visual.

## 0. VERDICT
The plan's architecture stands (F1 restructure, F2 ClipRef in the 16-byte tuple, F3 retired decks, Stage P as S0). The
seats found real gaps in four places, and the amendments close them: (1) the new GL read set (refs into ANY live or
retired deck) needs an enumerated fence audit, a pinned lint and a mandatory TSAN case; (2) "nothing plays unseen" and
"a re-fire resumes" need decode-side and live gates, not only REST playheads; (3) several claims could not fail as
gated (compile-only REDs, K readers that exist on one arm only, a text-only lint, a visual gate without a bar); (4) the
only on-screen cue (a deck name suffix in a 76-px row) would be truncated. Two seat premises are wrong and are
rejected with evidence: routines already resolve their targets once at the press (R-F3), and a box clip's decode
thread already parks (R-F4). One default flips: a deck switch stops being an Undo step (Q4). 42 attacks: 20 ACCEPT,
21 ACCEPT-IN-PART, 1 REJECT. 27 amendments override the plan body. ready_to_build: YES (S0 now; S1 onward under this
ruling). Boris questions: Q1, Q2, Q4 (default flipped), Q5 (new); Q3 withdrawn.

## 1. FACTS RE-DERIVED FOR THIS RULING
R-F1 The plan's citations hold at HEAD: `git diff --stat eff2b1c..c6720a0 -- src tests .harmony/*.sh .harmony/*.py
     CLAUDE.md docs` is empty (the two commits since are .harmony/.reports docs). VERIFIED.
R-F2 The fence is global, not active-deck-scoped. withDeckDetached stores the fenced mark into the GL thread's ONE
     deck slot and drains a frame (UndoService.h:21-24, :76-87; FencedPtrSlot.h:5-14); every GL read of any deck,
     including today's off-screen DeckClock loop, sits inside `if (deckActive)` (Renderer.cpp:389-391, :546, :595,
     :731, :769); a fenced frame HOLDS the previous canvas 1-2 frames (Renderer.cpp:407-431). Commands fence
     unconditionally whatever deck they edit (ClipCommands.h:41-61 SetClipCmd / SwapClipsCmd "fenced unconditionally";
     DeckCommands.h:91-94 RemoveColumnCmd; MainComponent.cpp:827 ... :7033, 26 sites). Today the GL thread already reads
     every deck's playing clips (DeckClock), so an unfenced vector writer of ANY deck would already race. The unfenced
     writes tsan-r5 found are FIELD writes (mediaFile relink, plan-tsan-r5.md section 2), not vector structure. VERIFIED.
R-F3 Routines resolve their targets ONCE, at the press: RoutineEngine.cpp:732 `r.program = compileRoutine(*routine,
     comp); // targets resolved once, on the active deck (D2)`; the compiled ResolvedTarget holds a deck INDEX
     (Program.h ResolvedTarget, "validated COORDINATES only"), replayed through dispatch.fire with f.target.deck
     (MainComponent.cpp:1968-2011). A later deck switch does not retarget; an Insert / Remove Deck below the target
     shifts the index. VERIFIED.
R-F4 A clip no layer draws burns no decode: "Rule 15: a player that is not drawn ... decodes nothing" -- the decode
     thread parks (VideoPlayer.cpp:784-792; park() decrements threadsAwake, :762-767); every video clip of every
     loaded deck gets a player at load, "its thread parks: nothing draws it" (MainComponent.cpp:3142-3146, :3177);
     /api/state exposes video_players, video_threads, video_threads_awake (not parked), video_frames_decoded
     (ApiServer.cpp:1437-1443). VERIFIED.
R-F5 Leaving a layer today: a REPLACED clip keeps its playing flag; only a clear sets playing = false (Layer.h:568-577);
     the activation tail sets playing = true only on a first activation (Layer.h:535-544); the renderer writes the
     model playhead FROM the player every frame (MainComponent.cpp:4699-4711 comment) and a new activation does not
     seek the player, so a re-fired clip RESUMES (ruling-bf6.md:211-242, AM-6); CLAUDE.md "Transport state" documents
     it. VERIFIED.
R-F6 A crossfade's completion clears `previous` on the GL thread (LayerClock.h:19-27), so "retired while an active OR
     previous ref names it" keeps a fading-out deck alive exactly until its fade completes. VERIFIED.
R-F7 Only the tab click makes a deck switch an Undo step (onDeckSwitched -> SwitchDeckCmd, MainComponent.cpp:1411-1441);
     REST (:1914), OSC (:2215), bindings (:7697), genre (:699) and replay (:1979) call handleDeckSwitch with no undo.
     SwitchDeckCmd has no other user (grep: DeckCommands.h, MainComponent.cpp:1437, tests). VERIFIED.
R-F8 A column fire clears every non-ignoring layer whose cell is empty: Deck::triggerColumn (Deck.h:113-127) ->
     Layer::triggerClip's empty-cell branch -> clearActiveClip (Layer.h:403-406). VERIFIED.
R-F9 No column edit shifts another column: Remove Column removes only the LAST column (MainComponent.cpp:6857-6880
     `col = numColumns - 1`; DeckCommands.h:109-118 invariant jassert); Add Column appends (:6848); no column insert
     exists. SwapClipsCmd edits one deck (ClipCommands.h:181-197) and every cell-edit entry resolves the SHOWN deck
     (e.g. :6858). VERIFIED.
R-F10 Bindings: TriggerClip ByPosition fires (targetLayerIndex, targetColumn) on the SHOWN deck (MainComponent.cpp:
     7582-7597); targetDeckIndex is read only by SwitchDeck (:7695-7697); ThisItem's velocity write uses
     composition_.activeDeckIndex (:7594). VERIFIED.
R-F11 globalTransitionSpeed's only readers / writers: TopBar.cpp:197, :204; Renderer.cpp:483; Composition.h:114, :189,
     :316, :449 -- no REST or OSC field (GET /api/composition fields, ApiServer.cpp:388-440). A layer's transitionSpeed
     -1 falls back to 0.5 s (LayerClock.h:22), never to the deck fade. VERIFIED.
R-F12 genreDeckAssignment has no UI or REST writer: written only by Composition::fromVar (Composition.h:547-551), read
     only at MainComponent.cpp:692. Boris's one saved show (~/Library/AudioDNA/compositions/test with harry.json) has
     all eight = -1. VERIFIED.
R-F13 Boris's show under F7: 2 decks x 3 layers; rows 0-1 identical on both decks; row 2 differs only in blendMode (46
     vs 1) and Deck 2's row 2 holds no clip; no layer connections, no layer effects; globalTransitionSpeed 0.3.
     VERIFIED (python read). The conversion changes nothing visible in that show.
R-F14 The tuple word is written only through pack (Layer.h:92-131: ctor / store / compareExchange; w_ private) and is
     never serialized (Layer.cpp:29-139 writes no tuple field). The planned encoding round-trips over deck in {0, 1,
     100, 0x3FFE, kNoDeck} x column in {-1, 0, 1, 9999, 10000, 0x3FFE} x snap 0-4 (150 cases), and an out-of-range
     pending column silently aliases another valid ref: (deck 4, column 0x4000) decodes as (deck 5, column 0).
     VERIFIED by emulation (scratch pack.py).
R-F15 tests/test_layer_runtime.cpp:45-65 loops cols[] = {-1, 0, 9999, 10000, INT_MAX} through active AND previous and
     pins kMaxPendingColumn == 268435454 (:51); INT_MAX cannot live in a 16-bit column field. VERIFIED.
R-F16 Nothing outside the composition file names a deck id: PerfState keys decks by NAME (PerfState.h DeckRuntime{std::
     string deck}) and keeps activeDeckIndex; ControlPath keys by index + name (Program.cpp:25-60); a composition load
     stops all routines and clears undo (MainComponent.cpp:2996, :3029); a deck append is an undoable InsertDeckCmd
     (:3232-3246). VERIFIED.
R-F17 Old deck rows always carry "type" (Layer::toVar, Layer.cpp:31); plan S2.1's new rows carry only "clips". A layer
     added today is Transparent (Deck::addLayer, Deck.h:53-61). VERIFIED.
R-F18 Layer scalar connections and layer effects live ON the Layer (Layer.h:274 scalarConns, :282 layerEffects;
     Layer.cpp:90-126), so a dropped per-deck settings set drops its own connections. PerTypeAutopilotConfig is
     composition-level (Composition.h:131-152). VERIFIED.
R-F19 The strip's clip-name row is 76 x 20 px: strip 250 wide, row 96 (DeckView.h:163-167); mainH = 96 - 20, thumbW =
     mainH, clipNameBounds_ = (thumbX, 76, 76, 20) (LayerStrip.cpp:613-692); 10-pt centred text with ellipsis
     (:586-588); routine bands paint over the TOP of the thumbnail (:531, :867). A folded row is 22 px with no visible
     name row (DeckView.cpp:332, :350). VERIFIED.
R-F20 Deck tabs show names, not numbers (DeckView.cpp:477-498); tab and column-header highlights share 0xff3a5a4a
     (:290-305); the column header = the last column fired (MainComponent.cpp:4914; reset :2964, :3790). VERIFIED.
R-F21 Remove Deck has no confirmation: the tab menu is headed by the deck's name (DeckView.cpp:523-525); removal pushes
     RemoveDeckCmd and shows "Undo Remove "<name>"" (MainComponent.cpp:3750-3799). The strip's X clears the layer and
     stops its routines (:754-769). VERIFIED.
R-F22 Notice surfaces that exist: audioDeviceNotice_, a yellow header label (MainComponent.cpp:487-489, :2635-2638,
     :3094-3097), and the Record panel's notice line (:2033-2042). No composition-load notice path (ruling-bf9 R-F12).
     VERIFIED.
R-F23 BORIS_DECISIONS.md:370 "Decks keep playing while off screen (2026-09-26)" carries no SUPERSEDED / CLARIFIED
     mark. VERIFIED.
R-F24 REST: switch_deck -> handleDeckSwitch (MainComponent.cpp:1914); /api/routine/fire exists (ApiServer.cpp:334);
     test routes /api/debug/load_deck, duplicate_deck exist (:320-321); no remove-deck or undo route; /api/debug/
     ui_text returns file_label + audio_notice only (:2013-2042). VERIFIED.
R-F25 tickMediaClock and VideoPlayer::advanceClock are reached only through DeckClock (grep: Renderer.h:701,
     Renderer.cpp:1833, DeckClock.h; no test names them). VERIFIED.

## 2. ATTACK RULINGS (42)

### Seat live-show / real-time (LS)
LS-A1 [MUST] ACCEPT-IN-PART -> amendment 6. Premise REJECTED: targets resolve once at the press (R-F3), so a running or
      queued routine is never retargeted by browsing. ACCEPTED: the pin test (T13) and the one real hole -- the
      compiled target is a deck INDEX, which Insert / Remove Deck shifts; targets now carry the deck id. No K row: the
      live fire path is handleClipTrigger, which K8b drives, and probe-routines' unchanged rows replay through it.
LS-A2 [MUST] ACCEPT-IN-PART -> amendment 3. Premise corrected (R-F2): the fence detaches the GL thread's whole deck
      work, and today's GL thread already reads every deck. ACCEPTED: an enumerated audit table, the B4f pinned lint,
      and a MANDATORY TSAN case with non-shown-deck row edits plus retire / restore / reap.
LS-A3 [MUST] ACCEPT-IN-PART -> amendment 18. REJECT Q5 "keep the Fade?": under one shared stack a deck fade blends a
      frozen canvas into the same live layers (a stutter on video that "or how they are playing" forbids, K1b / K1c),
      and a Fade slider with nothing to fade is a dead control; the removal is a consequence default already put to
      Boris (binding-decisions.md:603-605). There is no REST / OSC key (R-F11). ACCEPTED: S3.3 is its own commit; the
      old-show note names a dropped fade where one could be seen; the Boris page states why.
LS-A4 [SHOULD] ACCEPT -> amendment 10. Q4's default flips to "Undo skips deck switches"; separable commit S2c.
LS-A5 [SHOULD] ACCEPT -> amendment 11. Q5 to Boris (default: clear, as today), K4b, page 8.4.
LS-A6 [SHOULD] ACCEPT -> amendment 4(b)(c)(f), K9b. The plan's "active OR previous" rule already holds a fading-out
      deck (R-F6); now it is tested in the unit suite and live.
LS-A7 [SHOULD] ACCEPT-IN-PART -> amendment 5, K2v. Premise REJECTED: an undrawn player's decode thread parks (R-F4);
      players for every deck's video clips exist at load today and after bf9b alike. ACCEPTED: a decode-side BAR
      (video_threads_awake, video_frames_decoded) -- the cause, measured directly -- instead of noisy process CPU.
LS-A8 [SHOULD] ACCEPT -> amendment 5, K10.
LS-A9 [SHOULD] ACCEPT-IN-PART. (1) REJECT: K5 already runs with Link on (ruling-bf9.md:498-499; plan S4.1
      k5_queue_link_on). (2) REJECT: ByPosition TriggerClip fires the SHOWN deck; only SwitchDeck stores a deck index
      (R-F10), now harmless (a switch changes only the grid; SF-4). (3) ACCEPT as a statement, not a question: saving
      never kept what plays (Layer.cpp:29-139); docs + Boris page 8.10.
LS-A10 [SHOULD] ACCEPT-IN-PART -> amendment 12. REJECT call-count hooks in production code (a counted call can be a
      no-op; an uncounted path can still change state). ACCEPTED: T1's state fingerprint over every headless switch
      entry with a running routine, B4g (the tab path is literally handleDeckSwitch), T14 (pad lights).
LS-A11 [NIT] ACCEPT-IN-PART -> amendment 15. REJECT the per-frame deck cache (at most 2 refs x layers x decks integer
      compares, sub-microsecond). ACCEPT B6(ii) as a BAR.
LS-A12 [NIT] ACCEPT -> amendment 23 (number at merge; rule 15's text is true once amendment 6 lands).

### Seat STATE / CONCURRENCY (SC)
SC-A1 [MUST] ACCEPT-IN-PART -> amendment 5. ACCEPT: the leave rule written down and gated (K2v decode counters, K10
      resume). REJECT playing = false on replacement: every returning clip would come back PAUSED (R-F5), against
      CLAUDE.md "Transport state" and bf6 AM-6's "a re-fired clip still resumes".
SC-A2 [MUST] ACCEPT-IN-PART -> amendment 3 (as LS-A2).
SC-A3 [MUST] ACCEPT-IN-PART -> amendment 21. REJECT the ref fix-up for bf9b: no column edit shifts indices (R-F9) and
      every cell edit reaches only the shown deck, so grid edits cannot touch a layer playing another deck's clip;
      editing the playing cell itself acts as today (plan R8, pre-existing). ACCEPT pinning tests (T7b) and SF-2.
SC-A4 [MUST] ACCEPT-IN-PART -> amendment 7. ACCEPT: no deck id is ever reused in a session (refuse at the cap; renumber
      at load only). REJECT "takes / routines store deck ids" (R-F16).
SC-A5 [MUST] ACCEPT -> amendment 8 (a per-row discriminator: the "type" key, R-F17).
SC-A6 [SHOULD] ACCEPT-IN-PART -> amendment 9(a)-(c). ACCEPT the store table, counts in the note, the M-cases.
      Correction: bindings and routines do not key on deck-local Layer objects (R-F10; Program.cpp:25-60).
SC-A7 [SHOULD] ACCEPT-IN-PART -> amendment 4. ACCEPT earlier reaping (inside ANY fenced edit) and the save statement.
      REJECT skipping connection ticks on retired clips (the clip playing from a retired deck needs its connections).
      REJECT a reap "at the moment the last ref clears": that moment is a GL-thread CAS, and a message-thread timer
      reap would cost an unrequested 1-2 frame canvas hold mid-set (R-F2).
SC-A8 [SHOULD] ACCEPT -> amendments 12, 13.
SC-A9 [SHOULD] ACCEPT-IN-PART -> amendment 2. ACCEPT the bijection test, one "none", refusal at every trigger entry.
      REJECT the deck-0 / legacy-small-int ambiguity: pack is the only writer and the tuple is never serialized (R-F14).
SC-A10 [SHOULD] ACCEPT -> amendment 22.
SC-A11 [NIT] ACCEPT-IN-PART -> amendments 19, 20. Genre: no control exists to grey out (R-F12); Q3 withdrawn. ACCEPT
      T16 (a Selected binding whose source is retired).

### Seat gates / tests (GT)
GT-A1 [MUST] ACCEPT -> amendment 2(b)(c) (R-F14, R-F15).
GT-A2 [MUST] ACCEPT -> amendment 14 (per-arm readers + VALID positive controls).
GT-A3 [MUST] ACCEPT -> K1d (speed, reverse / ping-pong, opacity + blend, a connected clip parameter).
GT-A4 [MUST] ACCEPT -> K8b (browse-and-fire), keyboard path stated as unit-only.
GT-A5 [SHOULD] ACCEPT-IN-PART -> amendment 12. ACCEPT: T1 through every headless entry including Add / Insert / Remove
      Deck execute / undo / redo; B4d declared a SMOKE lint; B4g. handleDeckSwitch is driven live by K1 / K8 (REST);
      a MainComponent test seam is not built (MainComponent is not headless).
GT-A6 [SHOULD] ACCEPT -> amendment 17 (headless machine checks, BEFORE capped, contrast).
GT-A7 [SHOULD] ACCEPT -> amendments 3(c), 4(g), K9.
GT-A8 [SHOULD] ACCEPT -> amendment 13.
GT-A9 [SHOULD] ACCEPT-IN-PART -> amendment 1. No citation is stale today (R-F1); the symbol-first rule protects later
      rebases.
GT-A10 [NIT] ACCEPT -> amendment 15 (walk INFO).

### Seat UX / visual (UX)
UX-A1 [MUST] ACCEPT -> amendment 16(a) (R-F19: a 76-px row cuts the suffix first).
UX-A2 [MUST] ACCEPT -> amendment 16(b)(c).
UX-A3 [MUST] ACCEPT-IN-PART -> amendment 17. ACCEPT measurable bars, the extra states, the fail rule. REJECT a live
      mid-switch capture (screencapture cannot time one); the headless snapshot walk plus the same-object check prove
      the strip does not change.
UX-A4 [MUST] ACCEPT-IN-PART -> amendments 9(d), 18, 19. Fade: REJECT block-until-answered (forced by K1, already put
      to Boris). Genre: REJECT grey-out (no control exists, R-F12). ACCEPT: the visible load notice names a dropped
      fade where one existed.
UX-A5 [SHOULD] ACCEPT -> amendment 9(d).
UX-A6 [SHOULD] REJECT -> SF-1. Boris's own words are "ignore column controls"; bf9b changes neither the control's
      place nor its label, and Stage P's G7 critics judge both. A strip indicator is a new UX feature outside this
      lane.
UX-A7 [SHOULD] ACCEPT-IN-PART -> amendment 16(d). No new dialog (Remove Deck has none today, R-F21; a modal step
      mid-show); the existing undo hint names the layers still playing; the X is the stop and B7 checks it is visible.
UX-A8 [SHOULD] ACCEPT-IN-PART -> amendment 16(e), B7 state 5. REJECT a new colour (pre-existing palette, R-F20).
UX-A9 [NIT] ACCEPT -> amendment 24 (a numbered test show; expected results per step).

## ARCHITECT RULING (s-rta-1002b)

### Amendments -- numbered; each OVERRIDES the plan body where they differ

1. BASE AND LINE PINS (GT-A9). The plan's citations hold at c6720a0 (R-F1). The builder records BF9B_BASE and resolves
   every :NNNN in the plan by SYMBOL first (line numbers are hints). S1's first commit message carries a symbol ->
   file:line table for every 4.C block (grep -n of each named function at BF9B_BASE).

2. CLIPREF RANGE, ONE "NONE", AND THE ROUND-TRIP TEST (GT-A1, SC-A9).
   (a) S1 adds TEST_CASE "ClipRef packing is a bijection on the valid domain (bf9b S1)" [layer_runtime]: deck in {0, 1,
       100, 0x3FFE, kNoDeck} x column in {-1, 0, 1, 9999, 10000, 0x3FFE}, in the active, previous AND pending slots,
       x snap {Off, Beat, Bar, TwoBar, FourBar}; unpack(pack(x)) == x for each; deck id 0 and kNoDeck decode distinct.
   (b) 4.B: the plan's ":48-51" row is REPLACED by this row for tests/test_layer_runtime.cpp:45-65 (quote Q-C):
       cols[] INT_MAX -> ClipRef::kMaxColumn; pendings[] LayerRuntimeCell::kMaxPendingColumn -> ClipRef::kMaxColumn;
       `REQUIRE(LayerRuntimeCell::kMaxPendingColumn == 268435454)` -> `REQUIRE(LayerRuntimeCell::kMaxPendingColumn ==
       ClipRef::kMaxColumn); REQUIRE(ClipRef::kMaxColumn == 16382);`; `CHECK(checked == 5 * 5 * 4 * 5)` unchanged.
   (c) Out of range is REFUSED, never clamped (an out-of-range column aliases another valid ref, R-F14). From S2 on,
       every tuple-writing entry -- Layer::triggerClip, triggerClipImmediate, releaseMomentary, clearActiveClip
       (onlyIfActive), Composition::fire, Composition::triggerColumn -- returns the unchanged transition for a ref with
       column > kMaxColumn, deck > kMaxDeckId, or deck == kNoDeck with column >= 0; LayerRuntimeCell::pack jasserts the
       same. NEW T15 asserts the refusal per case (tuple byte-identical). The deck-less form survives only as S1's inert
       interim and in S1's tests.

3. FENCE AUDIT, PINNED LINT, MANDATORY RACE CASE (LS-A2, SC-A2, GT-A7). Correction to plan F5 / R3: the fence is global
   (R-F2). What bf9b changes is the GL read set (refs into any live or retired deck) and five new vector writers. Scope:
   STRUCTURE only -- field-level writes stay tsan-r5's.
   (a) FENCE AUDIT TABLE (lane report): every src call site that changes the size or element storage of
       Composition::decks, Composition::retiredDecks_, Composition::layers, Deck::rows or ClipRow::clips -- the
       container operations insert / erase / push_back / emplace_back / resize / assign / clear / swap / whole-object
       assignment, and the model methods that perform them: appendDeck, addDeck, retireOrEraseDeck,
       restoreRetiredDeck, reapRetiredDecks, insertLayer, eraseLayer, moveLayer, Deck::addColumn / removeColumn /
       ensureColumns / setClip / clearCell, ClipRow::setClip / clearCell / ensureColumns / fromVar -- each with its
       fence: the enclosing withDeckDetached / DeckFenceHook call (file:line), or "staged, unpublished composition
       (Pitfall 58)", or "headless test".
   (b) B4f LINT: a TEST_CASE in tests/test_render_thread_lint.cpp, "bf9b: box / stack structure writers sit only in
       audited sites (pinned counts)". Over codeLines() of src/**/*.{h,cpp,mm}, excluding src/model/ClipRow.h, Deck.h,
       Composition.h and ShowMigration.h, count per file
       `\b(appendDeck|addDeck|retireOrEraseDeck|restoreRetiredDeck|reapRetiredDecks|insertLayer|eraseLayer|moveLayer|addColumn|removeColumn|ensureColumns|setClip|clearCell)\s*\(`
       plus `\b(decks|retiredDecks_|layers|rows|clips)\s*\.\s*(push_back|emplace_back|insert|erase|resize|assign|clear|swap)\s*\(`
       and REQUIRE the per-file counts to equal a pinned map whose sites are exactly the audit table's. A new site
       fails until it is audited (the tsan ruling's pinned-count pattern).
   (c) MANDATORY TSAN CASE, tests/test_layer_runtime_race.cpp: TEST_CASE "R-bf9b fenced box and stack edits vs the GL
       resolve of refs into any deck" [tsan][layer_runtime].
       - Setup: 3 decks x 3 rows x 6 columns of video-type clips; layer 0 plays (deck 2, col 1), layer 1 plays
         (deck 0, col 3), layer 2 mid-crossfade from (deck 1, col 0) to (deck 0, col 0); a FencedPtrSlot<Deck> stands in
         for the renderer's slot.
       - GL driver thread, per frame: `v = slot.view()`; fenced or null -> count a held frame, continue; else per layer
         ONE runtime() load, resolve active and previous through Composition::clipAt (live, then retired decks), read
         inPoint / playing / effects.size(), LayerClock::tick, the show Autopilot's processFrame; frames++.
       - Message thread, 2,000 iterations, every mutation inside an emulated fence (detachFenced(); record f0 = frames;
         wait until frames >= f0 + 2; mutate; set(ptr)): SetClip on the NON-shown deck's playing cell (deck 2, col 1)
         and its neighbour; addColumn / removeColumn (last) on a non-shown deck; ClearLayerClips(deck 1, row 2); Remove
         Deck 2 while layer 0 plays from it (retire), then its undo (restore); retire, re-fire layer 0 elsewhere, reap;
         insertLayer / eraseLayer / moveLayer. Unfenced triggers (CAS, as the app) interleave.
       - PASS: zero "WARNING: ThreadSanitizer" (the existing FAIL_REGULAR_EXPRESSION), frames > 100, every resolved
         clip pointer inside a live or retired deck's storage.
       .harmony/probe-tsan-unit.sh: EXPECTED_TSAN_CASES 4 -> 5 (TARGETS unchanged). Plan R3's "if B3 reports anything"
       is deleted.

4. RETIRED DECKS: WHEN THEY GO (SC-A7, LS-A6, GT-A7). Replaces F3's "inside the next deck-structure command".
   (a) UndoService::withDeckDetached, after `mutation` returns and before the restore, calls
       Composition::reapRetiredDecks() (every retired deck no layer's active or previous ref names); after the restore
       it hands the reaped decks to a new hook MainComponent sets, which disposes their media through the per-clip path
       RemoveDeckCmd uses for an erased deck (its liveness scan walks forEachClip, live and retired). So ANY fenced
       edit reaps, and a composition swap disposes retired decks with the old model. No timer reap: a reap needs a
       fence, and a fence holds the canvas 1-2 frames (R-F2); parked players lingering until the next edit cost
       nothing on screen (R-F4).
   (b) A fade OUT of a retired deck keeps it alive until the fade completes (previous ref, R-F6).
   (c) RemoveDeckCmd cancels every pending ref into the deck it retires or erases (cancelPendingInto) inside its fence.
       Nothing creates a pending ref into a retired deck afterwards: fires target live decks, and F10's autopilot does
       not advance from a retired source.
   (d) Connection ticks on retired clips STAY (forEachClip): the clip playing from a retired deck needs them.
   (e) Saving: retired decks are not saved (they are deleted decks) and no layer's playing state is saved (unchanged,
       Layer.cpp:29-139); after opening a show every layer starts empty, as today.
   (f) T6 gains: T6c Remove Deck mid-crossfade with the PREVIOUS ref in the removed deck -- retired; LayerClock ticks
       finish the fade on the same OUT -> IN clips; no fenced edit reaps it while progress < 1; the first fenced edit
       after completion does. T6d a pending ref into the removed deck is cancelled; a pending ref into another deck is
       untouched. T6e through UndoService::withDeckDetached (headless pass-through fence + the hook): any fenced
       mutation reaps an unnamed retired deck and hands it to the hook exactly once.
   (g) GET /api/composition gains top-level "retiredDeckCount". Two test-server-only routes, the /api/debug/load_deck
       pattern: POST /api/debug/remove_deck {"deck": i} -> MainComponent::removeDeck(i) (the tab menu's function), and
       POST /api/debug/undo -> the app's Undo (the Cmd+Z function). Gate K9.

5. A CLIP THAT LEAVES ITS LAYER (SC-A1, LS-A7, LS-A8). Written into plan F4 and performance-controls.md: a clip that no
   layer's active or previous ref names is not drawn, so nothing advances it -- its model playhead freezes (no
   DeckClock), its decode thread parks and its ring trims after 1 s (R-F4); its playing flag is left as it was (R-F5).
   Fired again later it RESUMES from the frozen position (today, bf6 AM-6; C3 / F13 make the model agree); a retrigger
   of the active cell restarts at the in-point; a beat-snapped clip seeks to the beat phase (MainComponent.cpp:
   4677-4697); a never-played clip starts at its in-point. playing = false on replacement is REJECTED (returning clips
   would come back paused). Gated by K2v (a BAR) and K10.

6. ROUTINES ARE PINNED TO THEIR DECK BY ID (LS-A1). Targets keep resolving once at the press (R-F3). ResolvedTarget
   gains `uint32_t deckId`, set by Program::compile / compileRoutine from comp.decks[idx].id. dispatch.fire
   (MainComponent.cpp:1958-2011) resolves Clip-scope events through Composition::findDeckIndexById(deckId); not found
   (removed or retired) -> the event is skipped and the routine's notify line says so once; Layer-scope events ignore
   the deck (F9). The RoutineEngine footprint keeps the deck id (labels only). NEW T13: fire a routine with deck A
   shown, switch to B -> its activeClip events fire A's cells into their rows' layers; Insert Deck before A -> still
   A; Remove Deck A -> its Clip-scope events are skipped, none lands in another deck.

7. DECK IDS ARE NEVER REUSED IN A SESSION (SC-A4). Replaces S2.1's "the lowest free id once the counter passes it" and
   plan T11.
   (a) appendDeck / addDeck with the mint past kMaxDeckId REFUSES: no deck is added; the caller shows "This show has
       used all its deck numbers. Save it and open it again to add more decks." in the load-notice label (9(d)).
   (b) Load: when any deck id in the file is > kMaxDeckId, or the largest is >= kDeckIdCompactAt (8192), every deck is
       renumbered 100, 101, ... in file order; otherwise ids are kept and duplicates re-minted as today
       (test_composition.cpp:665-712 unchanged). Safe at load: nothing outside the file names a deck id and a load
       clears undo (R-F16).
   (c) So no ref, undo snapshot or RemoveDeckCmd snapshot can alias another deck; an undo naming a reaped deck
       resolves to nothing (plan R9 stands).
   (d) T11 becomes: deck ids {50000, 7} load as {100, 101}; {100, 250} are kept and the next mint is 251; appendDeck
       with the mint at kMaxDeckId + 1 refuses and leaves `decks` unchanged; remove -> reap -> append -> undo never
       resolves a ref into the wrong deck.

8. LOAD DECK: OLD OR NEW ROWS, DECIDED PER ROW (SC-A5). A row object with a "type" property is a legacy row (Layer::
   toVar always wrote it, R-F17); its settings apply ONLY when that row ADDS a shared layer (F6); existing shared layers
   keep the show's settings. A row without "type" is a new row: clips only; a layer it adds is what Add Layer makes
   today (Transparent, "Layer N", a fresh id; Deck.h:53-61). A row with neither "clips" nor "type" is an empty row.
   Composition files keep the plan's rule (no top-level "layers" = old show). NEW M6: (a) a new-format 5-row deck into
   a 3-layer show adds 2 default layers; (b) an old-format 5-row deck adds 2 layers with the file's row-3 / row-4
   settings and leaves layers 0-2 untouched; (c) a mixed file is read per row; (d) {} and {"clips": []} rows load as
   empty rows; (e) undo removes the deck and the added layers.

9. OLD SHOWS: WHAT IS KEPT, WHAT THE NOTE SAYS, WHERE BORIS SEES IT (SC-A6, UX-A4, UX-A5).
   (a) STORE TABLE (performance-controls.md + lane report): layer settings including layer effects, layer scalar
       connections and per-layer autopilot settings live ON the Layer (R-F18) -> the winning set keeps its own, a
       dropped set loses its own; ByPosition / Selected bindings address (layer index, column) on the shown deck and
       ThisItem a clip id (R-F10) -> no migration; routine / take keys are deck index + name and layer index + name
       -> Layer scope resolves the shared layer by position or name (F9); PerfState v1 -> M5; PerTypeAutopilotConfig is
       composition-level -> untouched.
   (b) THE NOTE names a dropped row ONLY when its settings differ from the winning row's (Layer::toVar minus "clips",
       "id", "name", compared as JSON), with the count of layer effects and connected scalars it carried; names
       "persistent" only for layers whose file value was true; names the deck fade only when the show had >= 2 decks.
       Nothing dropped -> no note.
   (c) M1 adds a Deck-2 row-1 layer scalar connection and one layer effect -> the note says "Deck 2 row 2: 1 layer
       effect, 1 connection dropped". NEW M7: a 1-deck old show with no "persistent": true produces NO note; a file
       shaped like Boris's (R-F13) produces exactly one row line (Deck 2 row 3) plus the fade.
   (d) DISPLAY PATH: a `loadNotice_` label in the header row beside audioDeviceNotice_ (same yellow, same right-aligned
       row-1 slot; both visible share it). Text "Old show converted -- layer looks now come from the first deck (hover
       for details)"; tooltip = the full migrationNote. It also shows Composition::routineLoadNote when non-empty
       (closes ruling-bf9 SF-1) and amendment 7(a)'s refusal. Visible until the next save, the next load, or a click
       on it; never takes focus. /api/debug/ui_text gains "load_notice". B5 checks it.

10. UNDO SKIPS DECK SWITCHES (LS-A4; Q4's default flips). With a switch changing nothing on screen, an undo step that
    only moves the grid makes Cmd+Z after a browse do nothing visible (21 presses to reach a misfire after a 20-deck
    browse), and only the tab click was ever undoable (R-F7). Default: a deck switch is never an Undo step. S2c, ONE
    separable commit after S2b: onDeckSwitched's body becomes exactly `handleDeckSwitch(deckIdx);`; SwitchDeckCmd and
    its cases are deleted. 4.B (authority: Q4's default, asked): test_undo_commands.cpp:1821 and :1849 cases RETIRED;
    :1945-1947 (the SwitchDeckCmd lines inside "Deck commands no-op on stale coordinates") deleted, the case stays;
    :2840 RETIRED (S2b's "queue untouched" version lives until S2c). Docs say it. A "keep" answer = `git revert` of
    S2c, which restores S2a's SwitchDeckCmd (index + activate hook, no queue cancel).

11. COLUMN FIRE ON AN EMPTY CELL (LS-A5). F12 stays the default (R-F8): a column fire clears every non-ignoring layer
    whose cell in that column is empty -- now also a layer playing another deck's clip. It becomes asked (Q5), gated
    (K4b) and on the page (8.4). A "leave it" answer = one condition in Composition::triggerColumn (skip empty cells)
    with T4 and K4b flipped.

12. T1 AND EVERY SWITCH ENTRY (LS-A10, GT-A5, SC-A8). T1 is REPLACED: fingerprint(comp) = for every shared layer the
    raw tuple -- active ref, previous ref, crossfadeProgress, pending ref, snap override, each compared explicitly --
    plus, for every clip of every live and retired deck, playing / playheadPosition / beatsPlayed / hasBeenTriggered,
    plus UndoManager::historySize() and the RoutineEngine's running set. It must stay byte-identical across: the
    model-level shown-deck walk 0 -> 1 -> 0 and a 20-deck walk; a headless DeckView::showDeck walk 0 -> 5 -> 0 (every
    LayerStrip the same object); AddDeckCmd, InsertDeckCmd and RemoveDeckCmd (of a deck no ref names) execute / undo /
    redo; all with a routine running (fired through RoutineEngine with a stub dispatch) and queued triggers on two
    layers. B4d is declared a SMOKE lint (text, one level). NEW B4g: onDeckSwitched's body is exactly
    `handleDeckSwitch(deckIdx);` (after S2c), so every switch entry is the one function K1 / K8 drive live (R-F7,
    R-F24). NEW T14: MidiOutputHandler's pad rule becomes a pure static `padStateFor(const Composition&, int shownDeck,
    int row, int col)`, tested headless: a layer playing deck B's clip lights no pad while A is shown and lights (row,
    col) when B is shown; a retired source lights nothing.

13. MUTATION SMOKES (SC-A8, GT-A8). Before S2b's review, each mutation is applied on a scratch branch (never merged);
    the named tests must FAIL; the lane report lists mutation -> failing test names. A mutation that leaves its tests
    green is a STOP to Harmony.
    MS1 the shown-deck switch cancels the leaving deck's queued triggers -> T5, T1.
    MS2 Composition::clipAt ignores ref.deckId and resolves in the shown deck -> T2, T9.
    MS3 LayerRuntimeCell::pack writes the no-deck field in every slot -> the S1 packing case, the bijection case.
    MS4 the switch clears `previous` (or sets progress 1) -> T1.
    MS5 reapRetiredDecks ignores previous refs -> T6c.
    MS6 dispatch.fire resolves by target index instead of deckId -> T13.
    MS7 the strip badge shows the shown deck instead of the ref's deck -> the S3 badge case.

14. K ROWS: READERS, CONTROLS, NEW ROWS (GT-A2, GT-A3, GT-A4, LS-A5, LS-A6, LS-A7, LS-A8, GT-A7). Per-arm readers and
    VALID positive controls on every K row that reads REST; new rows K1d, K2v, K4b, K8b, K9, K10; K1's drivers gain
    the deck-command routes. Exact bars in the gate list. ruling-bf9 amendment 15 holds: no K1-K8 bar is weakened or
    dropped.

15. PERF (LS-A11, GT-A10). REJECT a per-frame deck cache. B6(ii) becomes a BAR on BF9B alone (gate list); STAGE_P (ii)
    and a walk sample stay INFO.

16. THE STRIP, THE TABS, THE GRID (UX-A1, UX-A2, UX-A7, UX-A8). Replaces S3.1's "<clip> - <deck>" text.
    (a) BADGE. The clip-name row keeps the clip name only (as today). The source deck is a badge on the thumbnail's
        BOTTOM-LEFT corner (routine bands own the top, R-F19): the deck's 1-based tab position (1 +
        findDeckIndexById(ref.deckId)), or "x" for a retired deck (ASCII, Pitfall 6); opaque background; width = text
        width + 6 px, so it never truncates; DIM text when that deck is the shown one, normal text when another; tooltip
        "From deck '<name>' (tab N)" / "From a removed deck". Drawn only when the thumbnail is >= 40 px (a folded row
        shows none). Set in refresh() compare-before-set (Pitfall 41); repaints only the badge rect (Pitfall 57); never
        AudioDNALookAndFeel::kRoutineCue.
    (b) TAB DOT. A deck tab shows a small dot when some shared layer's active or previous ref names that deck
        (DeckView::refresh, compare-before-set, repaint only that tab).
    (c) BADGE CLICK switches the grid to that deck (handleDeckSwitch(findDeckIndexById(id)); a no-op for a retired
        deck; not an Undo step); it does not select the layer.
    (d) REMOVE DECK: no new dialog. The existing undo hint (MainComponent.cpp:3799) reads "Undo Remove "<name>" --
        Layer N keeps playing its clip" when layers play from it. The strip's X (MainComponent.cpp:754-769) clears that
        layer; B7 state 3 checks the X is visible beside the badge.
    (e) COLUMN HEADER: lit = the last column fired, shown only on the deck it was fired from (the plan's rule, now the
        definition). No new colour.

17. B7 VISUAL WORK GATE (UX-A3, GT-A6, UX-A9). Headless machine checks in ctest + live captures for the critic panel;
    the exact list is B7 in the gate list. A critic "no" returns the lane to S3, never to Boris.

18. THE DECK FADE (LS-A3, UX-A4). No question: the Renderer half is deleted in S2a (K1 needs it gone); S3.3 (the
    TopBar half: label, slider, fadeSliderBoundsForTest, test_master_signal_link.cpp re-anchored) is its own commit.
    The old-show note names a dropped fade when the show had >= 2 decks (9(b)); integration.md says globalTransitionSpeed
    was never a REST / OSC field (R-F11); Boris page 8.8 gives the reason.

19. GENRE AUTO-SWITCH (UX-A4, SC-A11). F15 stands (inert, one logLine per session). Q3 is WITHDRAWN: nothing in the
    app can set a genre deck (R-F12) and Boris's show has none. effects.md says it is inert and why.

20. BINDINGS (LS-A9, SC-A11). ThisItem's velocity write targets the deck the clip was found in, not activeDeckIndex
    (MainComponent.cpp:7594). NEW T16: a Selected binding whose first playing layer's source is retired -> no fire, no
    crash, tuple unchanged; ThisItem finds a clip on a non-shown deck, fires it from that deck into its row's layer, and
    its velocity lands on that clip.

21. CELL EDITS (SC-A3). No ref fix-up in bf9b (R-F9; plan R8 stands). NEW T7b pins today's semantics in the new
    model: removing the last column leaves every ref into another column or deck unchanged; a ref into the removed
    column leaves its layer empty; a swap on the shown deck: the ref follows the CELL. SF-2 files the follow-up.

22. LAYER STRUCTURE UNDER QUEUES AND FADES (SC-A10). T7 adds: MoveLayerCmd with a queued trigger and a crossfade in
    flight -> afterwards the moved layer's active, previous and pending refs resolve to the same Clip objects (rows
    move in step in every live and retired deck); RemoveLayerCmd undo inserts the snapshot row into every deck that
    still exists, an EMPTY row into any deck the snapshot lacks, and drops snapshot rows of reaped decks -- rows ==
    layers everywhere, asserted.

23. DOCS (LS-A12, LS-A9). Pitfall number assigned at merge (plan section 6). Rule 15's text is true once amendment 6
    lands. Add: performance-controls.md -- the leave rule (5), retired decks and when they go (4), Undo skips deck
    switches (10), the badge / tab dot / badge click (16), column fire on an empty cell (11), what saving keeps (4(e)),
    the store table (9(a)); integration.md -- /api/composition's top-level layers + retiredDeckCount, the two debug
    routes, ui_text "load_notice", no REST / OSC fade field; recording.md -- routine targets pinned by deck id (6);
    architecture.md -- reaping inside withDeckDetached (4(a)); BORIS_DECISIONS.md:370 gains "[CLARIFIED 2026-10-02 --
    see "Decks are boxes of clips": clips in layers keep playing on screen; nothing plays unseen]" (R-F23).
    CLAUDE.md: plan section 6 only, measured with wc -c (<= 25,000).

24. SECTION 8 (BORIS PAGE) is replaced by "WHAT ONLY BORIS CAN CHECK" below (UX-A9).

25. SECTION 9 is replaced by "BORIS QUESTIONS" below.

26. ORPHANS AND SHARED FILES. With DeckClock deleted, Renderer::tickMediaClock, syncMedia's decode = false branch and
    VideoPlayer::advanceClock have no caller (R-F25): delete them, and reword VideoPlayer.cpp:784's "(its deck off
    screen)" and VideoPlayer.h:101-105. B4a's zero-hit list gains `tickMediaClock|advanceClock`. 4.C gains: src/media/
    VideoPlayer.{h,cpp} (that deletion; mkvidx rebases), src/core/UndoService.{h,cpp} (4(a)), src/api/ApiServer.cpp
    (4(g), 9(d)), src/recording/Program.{h,cpp} + RoutineEngine.* (6), src/midi/MidiOutputHandler.* (12), DeckView's
    DeckTabButton (16(b); ui lane rebases), MainComponent's header row (9(d)).

27. GATES. Plan section 5 is replaced by the FINAL CONSOLIDATED GATE LIST below.

### FINAL BUILD STAGES (one builder context each, dependency order; main receives everything in ONE merge)
| Stage | Content | Depends on | Builds alone? |
|---|---|---|---|
| S0 | Stage P = ruling-bf9.md FINAL BUILD STAGES, VERBATIM (C0-C4; G0-G7 at STAGE_P_HEAD) | -- | yes; never merged alone |
| S1 | plan S1 + amendments 1, 2(a)(b) | S0 | yes (inert) |
| S2a | plan S2.1-S2.11 (src) + amendments 2(c), 3(a) writers fenced, 4, 5, 6, 7, 8, 9(a)-(c), 12 (padStateFor), 18 (Renderer half), 19, 20, 22, 26 | S1 | no -- the sanctioned non-building window opens |
| S2b | plan S2.12 (tests) + T1 (12), T6c-e (4), T7 + T7b (21, 22), T11 (7), T13-T16, M1 + M6 + M7, the R-bf9b TSAN case (3(c)), B4f (3(b)); mutation smokes MS1-MS7 recorded (13) | S2a | window closes; app + tests build |
| S2c | amendment 10 (Undo skips deck switches) -- ONE separable commit | S2b | yes |
| S3 | S3.1 badge / tab dot / badge click (16(a)-(c)); S3.2 grid (16(e)); S3.3 TopBar fade removal -- its own commit (18); S3.4 load-notice label (9(d)) + undo hint (16(d)); B7 headless machine checks; B7 live captures + critics | S2c | yes, per commit |
| S4 | probe-boxes (plan S4.1 rows + k1d_*, k2v_decode, k4b_empty_cell, k8b_browse_fire, k9a/b/c_remove_playing, k10_fresh_and_resume; per-arm readers); docs (23); the Boris test show + page (24); lane report | S3 | yes |

### FINAL CONSOLIDATED GATE LIST (pre-registered; Harmony copies gate strings only from here)
Rig: ONE live app under .harmony/.reports/s-rta-1002b/wf/lock.sh acquire_quiet_lock; `open -g <app> --args --test-mode`;
never an Output window; no synthetic input; captures by POST /api/render_frame (the canvas) unless stated; window
captures only by Quartz window id of our pid; HTTP Connection: close; quit_ours only; ps checked for CPU burners before
any perf verdict; background runs <= 2 h. Arms: STAGE_P = the STAGE_P_HEAD app copied to a scratch path before any later
rebuild; BF9B = the BF9B_HEAD app. d(X, Y) = mean |X - Y| over RGB, 0..255; noise = d of two captures of one steady scene
0.5 s apart, same run; floor = max(1.5, 4 x noise); tol = 1.5; t(frame) for ramp12.mp4 = 12 x meanG / 255.
Per-arm readers: STAGE_P reads decks[activeDeck].layers[*].activeClipColumn / previousClipColumn; BF9B reads top-level
layers[*].activeClip / previousClip {deckId, column, retired}. "BF9B-only" = the driver or the field does not exist on
STAGE_P (no RED arm); "guard" = expected GREEN on both arms.

G0-G7  ruling-bf9.md "FINAL CONSOLIDATED GATE LIST" (:321-454), VERBATIM, at STAGE_P_HEAD.

K1  ruling-bf9.md K1 / K1a / K1b / K1c / K1t, bars VERBATIM. Drivers: REST /api/switch_deck; POST /api/debug/
    duplicate_deck and /api/debug/load_deck (each shows the new deck; a deck file with the show's row count); POST
    /api/debug/remove_deck of a deck no layer plays from, and POST /api/debug/undo of that removal (BF9B-only); the
    take-replay activeDeck lane (K7). RED on STAGE_P where the driver exists.
K1d (RED on STAGE_P by behaviour; GREEN on BF9B). Each sub-row: switch 0 -> 1 at W - 0.5 s and 1 -> 0 at W + 0.75 s.
    i-a  deck 0 layer 0 = ramp12.mp4, speed 0.5, Loop, inPoint 0, transition Cut, fired at T0; W = T0 + 4.0 s;
         captures at W and W + 1.5 s. PASS iff t(W + 1.5) - t(W) in [0.25, 1.25] and the REST playheadPosition moved
         by 0.0625 +- 0.06 (BF9B; on STAGE_P deck 1 is shown at W -> RED).
    i-b  the same clip with PingPong, speed 1.0, fired at T0 (the forward leg ends at T0 + 12 s); W = T0 + 13.0 s. PASS
         iff t(W + 1.5) - t(W) in [-2.0, -1.0] and the REST playheadPosition moved by -0.125 +- 0.06. A window that
         straddles a wrap (decoded at W) is re-run once, then FAIL.
    ii   deck 0: layer 0 Opaque image A; layer 1 Transparent image B, opacity 0.5, blendMode Screen; capture A-only
         before firing layer 1, then fire it; settle 1 s; reference R. VALID iff d(R, A-only) >= 5. PASS iff every
         capture at W, W + 0.5 s, W + 2.0 s is within floor of R.
    iii  deck 0 layer 0 Opaque image A whose CLIP opacity is connected (Clip "conns", as .harmony/probe-lane3-clip.json)
         to a source the probe drives deterministically (a beat-clock LFO via inject_features beatPhase +
         totalBeatCount, Pitfall 42, or rms via inject_features; the builder names it), range 0.2 .. 1.0. Deck 0 shown:
         drive to max -> Rmax, to min -> Rmin. VALID iff d(Rmax, Rmin) >= 5. Show deck 1; drive max -> Cmax, min ->
         Cmin. PASS iff d(Cmax, Rmax) <= floor and d(Cmin, Rmin) <= floor. No deterministic source -> STOP to Harmony
         (never dropped).
K2  ruling-bf9.md K2, refined: fixture deck 0 row 0 and deck 1 row 1 = copies of ramp12.mp4 (the second fired with deck
    1 shown); 5 switches 0 <-> 1 within 10 s; every clip's playheadPosition read twice 1 s apart (BF9B: the F8 mirror
    decks[*].layers[*].clips[*]); advancing = delta > 0.005. "In a layer": per-arm reader (on STAGE_P = on the SHOWN
    deck's layers). VALID iff >= 2 clips advance. PASS iff (advancing AND not in a layer) == 0. Expected STAGE_P FAIL,
    BF9B PASS.
K2v (BF9B-only; BAR). 20-deck show; every deck's rows 0-2 hold copies of ramp12.mp4 (60 video clips). VALID iff
    /api/state video_players == 60. Deck 4's row-0 clip plays 3 s in layer 0, then is replaced (the "left" clip); then
    layer 0 <- deck 0 col 0, layer 1 <- deck 7 col 0, layer 2 <- deck 13 col 0 (each fired with its deck shown); show
    deck 19; settle 3 s; sample /api/state every 0.5 s for 5 s (10 samples) and /api/composition at the first and last
    sample. PASS iff (1) every sample video_threads_awake <= 4 and >= 8 of 10 samples == 3; (2) the 5-s delta of
    video_frames_decoded is in [3 x fps x 5 x 0.5, 3 x fps x 5 x 1.5 + 30] (fps = the fixture's, recorded); (3) the
    left clip's and every non-layer clip's playheadPosition moved <= 0.001.
K3  ruling-bf9.md K3, VERBATIM bars; "a clip outside a playing layer" by the per-arm reader.
K4  ruling-bf9.md K4, VERBATIM bars; "L still plays X" by the per-arm reader (BF9B: layers[L].activeClip == {deck0.id,
    colX}) plus its decoded region within floor.
K4b (BF9B-only; Q5's default). Deck 0: layers 0-2 play deck 0's column 0, each in its own region (layer transforms);
    layer 2 has Ignore Column Trigger on. Deck 1: column 1 holds a clip in row 0 only. Show deck 1; POST
    /api/trigger_column {"column": 1}. PASS iff 0.5 s later layers[0].activeClip == {deck1.id, 1}; layers[1].activeClip
    is none and layer 1's region equals the same capture with layer 1 cleared beforehand within floor;
    layers[2].activeClip == {deck0.id, 0} and its region is within floor of before.
K5  ruling-bf9.md K5, VERBATIM (Link off and Link on), per-arm readers.
K6  ruling-bf9.md K6 dispositions, VERBATIM; the lane report also lists K1d, K2v, K4b, K8b, K9, K10 as new rows.
K7  ruling-bf9.md K7, VERBATIM, plus: after loading the old show GET /api/debug/ui_text "load_notice" is non-empty and
    the "old show converted:" logLine appears exactly once; after save + reload there is no note and load_notice is
    empty.
K8  ruling-bf9.md K8, VERBATIM bars.
K8b (BF9B-only). K8's show; layer 0 = deck 0's image in the left-half region, layer 1's region on the right (layer
    transforms, as probe-fitmode.py's layerScale fixture). Reference Rright = deck 7 col 2 fired in a pre-step and
    captured, then cleared. Walk switch_deck 1..7; on deck 7 POST /api/trigger_clip {"layer": 1, "column": 2}; walk
    8..19 and back to 0. PASS iff at every switch the left region is within floor of its pre-walk capture; after the
    fire, the right region is within floor of Rright and layers[1].activeClip == {deck7.id, 2} at every remaining
    switch; peak_frame_time_ms <= 50 ms at every switch. The keyboard launcher path is covered by unit tests only.
K9  (BF9B-only).
    a) layer 1 plays deck 2's ramp12.mp4 (fired with deck 2 shown); show deck 0; POST /api/debug/remove_deck {"deck":
       2}. PASS iff numDecks drops by 1, retiredDeckCount == 1, layers[1].activeClip.retired == true, and K1b's bar
       holds on layer 1 (t at +2 s within 0.5 s of t_remove + 2.0).
    b) layer 1 crossfading (Dissolve, transitionSpeed 4 s) FROM deck 2's image TO deck 0's image; deck 2 also holds 2
       video clips; remove deck 2 at p ~ 0.3. PASS iff K1c's bars hold (OUT -> IN line, residual <= tol; p rises >= 0.15
       between +0.5 s and +2 s; complete by T + 1 s); retiredDeckCount == 1 while p < 1; then POST /api/debug/
       duplicate_deck {"deck": 0} (a fenced edit) -> retiredDeckCount == 0 and video_players drops by 2.
    c) repeat a), then POST /api/debug/undo. PASS iff numDecks is restored, deck 2 is back at index 2 with its id,
       retiredDeckCount == 0, layers[1].activeClip == {deck2.id, col} not retired, and K1b's bar holds across the undo.
K10 (BF9B-only; guard for C3 and the leave rule). Deck 3 row 0 = ramp12.mp4, never played, inPoint 0; layer 0
    transition Cut. Show deck 3, fire it, show deck 0 within 0.2 s. PASS(i) iff the first capture <= 0.3 s after the
    fire has t <= 0.5. At t ~ 4 fire deck 0's image into layer 0 (t_r = t of the last capture before it); wait 5 s;
    show deck 3; re-fire. PASS(ii) iff the first capture <= 0.3 s after the re-fire has |t - t_r| <= 0.5 and the clip's
    REST playheadPosition moved <= 0.001 during the 5 s.

B1 BUILD: `cmake --build build --config Release -j$(sysctl -n hw.ncpu)` -> rc 0 at BF9B_HEAD, all targets; every commit
   outside S2a..S2b builds the app and the tests (rc per commit in the lane report); S2c and S3.3 each build alone.
B2 UNIT: `ctest --test-dir build -j8 --output-on-failure` -> 0 failures; count(BF9B_HEAD) == count(STAGE_P_HEAD) + added
   - retired (both lists in the lane report). Required present and passing: the S1 packing case and the bijection case;
   T1 (amendment 12); T2-T5; T6 incl. T6c-e; T7 incl. T7b; T8-T10; T11 (amendment 7); T12; T13-T16; M1-M7; the S3
   badge / tab-dot / grid / snapshot / contrast cases; test_layer_clock (e); test_render_thread_lint incl. B4f, with its
   pins re-justified in the commit. MS1-MS7: each listed mutation made its named tests FAIL (lane report).
B3 TSAN (Pitfall 63): `.harmony/probe-tsan-unit.sh` -> exit 0 and `ctest -L tsan` all PASS with zero "WARNING:
   ThreadSanitizer": at S1's end 4 / 4; at BF9B_HEAD 5 / 5 (EXPECTED_TSAN_CASES 5, incl. "R-bf9b fenced box and stack
   edits ..."). Full output pasted.
B4 TEXT (each hit listed file:line; code lines with comments stripped, ruling-bf9 amendment 9):
   a) src/: zero `DeckClock|AutopilotBank|deck_transition|deckTransition|prevDeckFBO_|globalTransitionSpeed|
      cancelPendingTriggers|compositeDeck\b|tickMediaClock|advanceClock`, except the literal JSON key strings in
      src/model/ShowMigration.h; after S2c also zero `SwitchDeckCmd`.
   b) src/render/* and src/model/Autopilot.cpp: zero `getActiveDeck(` and zero `activeDeckIndex`.
   c) Renderer.cpp: the pointer from `activeDeck_.view()` is never dereferenced (null / fenced tests only).
   d) SMOKE: the bodies of handleDeckSwitch, onDeckSwitched and DeckView::showDeck contain none of
      `triggerClip|clearActiveClip|setRuntime|updateRuntime|cancelPending|refreshPreview` (one level, text only; the
      behavioural proofs are T1, K1, K8).
   e) Stage P's "no Persistent-feature identifier left in src/" lint passes with exactly ONE allow-listed file,
      src/model/ShowMigration.h.
   f) B4f (amendment 3(b)) passes in ctest, and the lane report's fence audit table lists exactly its pinned sites.
   g) after S2c, onDeckSwitched's body is exactly `handleDeckSwitch(deckIdx);`.
B5 OLD FILES (live; K7 + M1-M7): POST /api/load_composition with (i) a 2-deck pre-bf9b show (differing layer settings,
   one "persistent": true, a Deck-2 layer connection; built with probe-crossfade's JSON helpers) and (ii) a show holding
   a pre-bf9b take with an activeDeck lane. PASS iff GET /api/composition shows the first deck's layer settings, exactly
   one "old show converted:" logLine, /api/debug/ui_text load_notice non-empty, the take fires without error and its
   activeDeck lane changes only activeDeck; then save + reload: no note, load_notice empty.
B6 PERF: interleaved A/B, >= 5 runs per arm alternating, 60 s each, mean frame_time_ms + SD.
   (i) INFO: STAGE_P vs BF9B, a 1-deck 3-layer steady scene; a BF9B regression > max(1.0 ms, 3 x pooled SD) STOPS the
       merge for a look.
   (ii) BAR: BF9B 20-deck show with 3 playing layers (K2v's fixture) vs BF9B 1-deck show with the same 3 clips:
       mean(20-deck) - mean(1-deck) <= max(1.0 ms, 3 x pooled SD). STAGE_P's (ii) is INFO.
   (iii) INFO: during K8's walk sample /api/state every 100 ms; report mean frame_time_ms and the number of samples
       whose peak_frame_time_ms > 25 ms, both arms.
B7 VISUAL WORK GATE.
   MACHINE (headless, ctest, tests/test_layer_strip_source_deck.cpp; components setVisible, Pitfall 34):
   M-a lit cells == {(row, col) : layers[row].activeRef == (shown.id, col)} for 3 fixtures (same deck, other deck,
       retired).
   M-b each strip's badge text == 1 + findDeckIndexById(ref.deckId), "x" when retired, none when clear; dim iff the
       ref's deck is the shown deck.
   M-c createComponentSnapshot of the strip column after showDeck(0), showDeck(5), showDeck(0): byte-equal outside the
       badge rects; every LayerStrip the same object.
   M-d badge rect inside the thumbnail rect, clear of the routine-band rows; badge width >= text width of "20" + 6 px;
       the clip-name row never contains the deck name; a 30-char clip name and a 20-char deck name change nothing but
       the name's ellipsis.
   M-e 20-deck tab row: a tab's dot is shown iff some layer's active or previous ref names that deck.
   M-f contrast (WCAG relative luminance) against the opaque badge background: dim text >= 3.0:1, normal >= 7.0:1,
       normal > dim.
   LIVE captures (`screencapture -x -o -l <CGWindowID>`, largest on-screen window of our pid; decoded) for the critic
   panel. States: (1) deck 0 shown, layer 1 playing deck 1's clip (badge "2" normal, no lit cell in row 1, dot on deck
   2's tab); (2) deck 1 shown (badge dim, cell lit); (3) a removed deck's clip playing (badge "x", the X visible); (4)
   TopBar without "Fade:"; (5) column 3 fired on deck 0 while an Ignore Column layer keeps deck 1's clip (header 3 lit on
   deck 0 only; that row's cell unlit; its badge "2"); (6) ruling-bf9 G7's Layer tab; (7) 20 decks with two dots, a
   30-char clip name, a 20-char deck name; (8) a folded layer playing another deck's clip (no badge; the dot shows);
   (9) the load notice after opening an old show. BEFORE (STAGE_P) only for (4), (6) and a plain "deck 0 shown, layer 0
   playing" grid -- the only states STAGE_P can render.
   CRITICS (yes / no + reason): visual-design "Is the badge legible at the strip's size, quieter when it is the shown
   deck, never clipped, and clear of the routine bands?"; UX "From the strips and the tabs alone, can Boris tell which
   box each playing clip came from, and get there in one click?"; graphic-design "Does the TopBar close the Fade gap with
   no orphan space, and do the badges and tab dots sit consistently?"; logic "Do the captures match M-a / M-b's model
   values for the same fixtures?"; interaction-logic "After 0 -> 1 -> 0, is every strip, fader and highlight where it
   was, with only the grid cells, the badge dimness and the tab highlight changed?".
   PASS = every machine check AND five yes. A "no" returns the lane to S3, never to Boris. Then ONE artifact page for
   Boris (amendment 24).
B8 POST-MERGE on main: B1, B2, B3, then K1-K10 once more on the merged build.

### WHAT ONLY BORIS CAN CHECK (replaces plan section 8; one artifact page, after B7)
Ship a generated test show "bf9b-check.json" (S4): 20 decks; every clip a picture with big numerals "D<deck> C<column>"
(drawn with PIL, the probes' fixture tool) and two ramp videos with a running time code. Easiest first; each step: do
-> expect -> what wrong looks like.
8.1 Fire D1 C1 into layer 1, show deck 3 and fire D3 C2 into layer 2, then click through all 20 decks and back.
    Expect: the output keeps D1 C1 and D3 C2 and the time code keeps counting; only the grid changes. Wrong: a numeral
    changes, a flash, a time-code jump.
8.2 Look at the layer strips: the small number on each picture is the deck tab its clip came from (dim when that deck
    is the one shown); the tabs of decks 1 and 3 show a dot. Click the number: the grid jumps to that deck. Taste: easy
    to read, not loud?
8.3 Ignore Column: tick it on layer 2 (Layer tab), show deck 5, fire column 4: layer 2 keeps D3 C2, the others show
    D5 C4.
8.4 Show deck 6 (its column 2 has nothing in row 2) and fire column 2: layer 2 goes empty -- Q5 asks if that is right.
8.5 Remove deck 3 while D3 C2 plays: it keeps playing, its strip shows "x", the undo hint names the layer; Cmd+Z brings
    deck 3 back with the clip still playing.
8.6 Fire a clip, click 5 deck tabs, press Cmd+Z once: the fired clip is undone (deck clicks are not Undo steps) -- Q4.
8.7 Layer tab: "Persistent" is gone; "Ignore Column Trigger" sits alone on its row.
8.8 The TopBar has no "Fade:" -- a deck change never changes the picture, so there is nothing to fade.
8.9 Open "test with harry.json": a yellow note says it was converted; your layers look as they did.
8.10 Save, close, reopen: the boxes and the layer looks come back; the layers start empty (as today -- what is playing
    is not saved).
8.11 A video you played, replaced, then fire again continues from where it was; a video you never played starts at
    its start.

### BORIS QUESTIONS (replaces plan section 9; plain words; each has a default, so the build never waits)
Q1 "If you delete a deck while one of its clips is playing, should that clip keep playing until you fire something else
   on that layer?" DEFAULT: yes, it keeps playing.
Q2 "If you load a deck that has more rows than your show has layers, should the show add the missing (empty) layers so
   you can see all of that deck's clips?" DEFAULT: yes.
Q3 WITHDRAWN (genre auto-switch: nothing in the app can set it, your show does not use it; it stays off -- amendment 19).
Q4 "Clicking a deck tab used to be an Undo step (Cmd+Z flipped you back a deck). Now that switching decks never changes
   the picture, Undo skips deck clicks, so Cmd+Z always undoes your last real change, like a clip you fired. OK?"
   DEFAULT: Undo skips deck clicks.
Q5 "If you fire a column on a deck where some rows of that column are empty, should those layers go empty (as today) or
   keep what they are playing?" DEFAULT: go empty, as today (Ignore Column still keeps a layer).

### SIDE FINDINGS (for Harmony; outside this lane)
SF-1 (UX-A6) Ignore Column is set only in the Layer tab; a lock mark on the layer strip would make it visible on stage.
     A UX follow-up, Harmony's call whether to ask Boris.
SF-2 (SC-A3) Swapping, moving or pasting over the PLAYING cell changes what that layer plays (the ref follows the cell;
     pre-existing). Follow-up: "a ref follows its clip on a move / swap", which needs a cross-row rule. Candidate
     question: "If you drag the clip that is playing to another cell, should it keep playing?"
SF-3 tsan-r5 keeps the field-level writes (mediaFile relink, config scalars, the httplib reader); bf9b's audit is
     structure-only. tsan-r5 re-scopes against post-bf9b code (plan 4.A).
SF-4 SwitchDeck bindings, routine / take activeDeck lanes and PerfState's activeDeckIndex store a deck INDEX (R-F10,
     R-F16): after an Insert / Remove Deck they name another tab. Harmless now (a switch changes only the grid).
SF-5 ruling-bf9 SF-1 (routineLoadNote never shown) is closed by amendment 9(d).

### RISKS OF THIS RULING
R-1 STRONGEST COUNTERARGUMENT: the amendments add roughly a quarter to the lane every other lane rebases on (two debug
    routes, a label, a badge, tab dots, six new K rows, a TSAN case, a lint, seven mutation smokes) and can stall it. It
    loses: each addition either makes an existing claim checkable (K2v, K9, K10, the TSAN case, B4f, MS1-MS7) or is the
    minimum UI that makes the model visible (badge, tab dot, load notice); the only behaviour Boris did not ask for is
    Q4's default, and it is one revertible commit. The K rows reuse the K1b / K1c drivers.
R-2 K2v's "== 3 on >= 8 of 10 samples" can catch an idle-trim wake of a parked thread mid-sample; the bar tolerates
    one transient (<= 4 on every sample). Flakiness beyond that is a STOP to Harmony, never a loosened bar.
R-3 B4f over-counts benign calls (clear() on a local vector named `rows`); they are pinned too, costing a review line,
    not a missed site. It cannot see a writer reached through a reference alias; the TSAN case is the behavioural net.
R-4 Reaping inside any fenced edit makes plan R9 (an undo naming a reaped deck shows an empty layer until Remove Deck is
    undone) reachable sooner; ids are never reused, so it can never show a WRONG clip.
R-5 Q4's flip removes a habit Boris may have; page 8.6 shows it and a "keep" is one revert.
R-6 Badge numbers follow tab positions, which shift on Insert / Remove Deck; the tab dot, the click and the tooltip
    name carry the identity.
R-7 K1d-iii depends on a deterministic connection source; if neither a bus-driven LFO nor rms is deterministic, it is a
    STOP, never a dropped row.
R-8 The badge's hit rect shadows a small part of the thumbnail, where a press today selects the layer (VERIFIED
    LayerStrip.cpp:964-967). The badge intercepts its own rect first, exactly as a routine band's x does (:942-955); a
    press anywhere else on the strip still selects.

STATUS: FINAL -- 42 attacks ruled (20 ACCEPT / 21 ACCEPT-IN-PART / 1 REJECT: UX-A6), 27 amendments; stages S0 (Stage P verbatim) -> S1 -> S2a/S2b -> S2c (Undo skips deck switches, separable) -> S3 (badge, tab dot, load notice, fade-removal commit, B7) -> S4; gates G0-G7 + K1-K10 (new K1d, K2v, K4b, K8b, K9, K10) + B1-B8; ready_to_build YES; Boris Q1 yes-keeps-playing, Q2 yes-add-layers, Q3 withdrawn, Q4 Undo-skips-deck-clicks (default flipped), Q5 empty-cell-clears (new)
