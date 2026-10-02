# RULING lane bf9 (s-rta-1002b) -- the blind council's attacks on plan-bf9.md (Stage P "Persistent removed")

Architect, 2026-10-02, repo /Users/boriskarpman/projects/RealTimeAudio, main HEAD fa9604d. The plan was written against
5e47d17; the hyg merge between them edits MainComponent.cpp in place only (:2310, :3067), so every src line the plan cites
is unchanged (VERIFIED, `git diff 5e47d17..HEAD`). Seat papers, verbatim: .harmony/.reports/s-rta-1002b/attack-bf9-papers.md.
Inputs read: plan-bf9.md, plan-bf9b.md (skeleton), .harmony/boris-feedback-backlog.md, .harmony/binding-decisions.md
"2026-10-02 (s-rta-1002b)", BORIS_DECISIONS.md:337-381, .harmony/s-rta-1002b-work.md. VERIFIED = read or run this session;
INFERRED = derived, not run.

## 0. VERDICT
Stage P's CONTENT is sound and stays (Persistent removed end to end; Ignore Column Trigger untouched). Its SHIP ORDER is
not: all three seats are right that a main with Stage P and without bf9b delivers none of the playing behaviour Boris asked
for while deleting the only path that draws another deck's layers. Stage P becomes bf9b's Stage 0: built first, now, on
bf9b's branch, gated at its own head, merged to main only inside bf9b's merge. 26 attacks: 16 ACCEPT, 8 ACCEPT-IN-PART,
2 REJECT. 17 amendments override the plan body. ready_to_build: YES (on bf9b's branch). Boris questions: none.

## 1. FACTS RE-DERIVED FOR THIS RULING
R-F1 No saved show of Boris's carries persistent=true. ~/Library/AudioDNA/compositions/ (the app's save dir,
     CompDecksBrowser.cpp:322-326) holds one file, "test with harry.json" (2026-10-01): 6 layers, all "persistent": false.
     ~/Library/AudioDNA/Decks/*.deck.json (4 files, 2026-03-15) carry no such key. A bounded search (maxdepth 4) of
     ~/Documents, ~/Desktop, ~/Downloads, ~/Music, ~/Movies finds no show with persistent=true. VERIFIED for those places;
     INFERRED for any other path he saved to.
R-F2 The plan's lint regex misses the declaration. Emulating the lint over src (codeLines = cut at the first //): plan
     regex 32 hits; bare-word regex `\bpersistent\b|canBePersistent|compositePersistentLayers|hasPersistentContent|
     beginEmptyActiveDeck|persistentToggle_` 33 hits -- the extra one is Layer.h:182 `bool persistent = false;`. Zero hits
     inside block comments; zero hits outside Stage P's deletion targets (VERIFIED, scratch run).
R-F3 `grep -rni persistent src` = 103 lines: 16 use the word in another sense (the G3c allow-list) and 87 describe the
     removed feature. The plan's edit lists cover 84 of the 87; missing: MainComponent.cpp:3008, MainComponent.cpp:3235,
     CompositorEngine.h:513 (VERIFIED, scratch run against the plan's line ranges).
R-F4 tests/test_undo_commands.cpp:188 reads `a.persistent == b.persistent`; no Stage P item edits it, so the undo test
     target stops compiling (VERIFIED).
R-F5 Deleting CompositorEngine.cpp:1266-1439 leaves runtime() 1 / getActiveClip( 1 in that file (was 2 / 2; the sites
     at :1278 and :1337 go) -- VERIFIED by emulating test_render_thread_lint's counter.
R-F6 Two unit cases already put an ignoring and a non-ignoring layer through one column trigger:
     test_undo_commands.cpp:2412-2462 "TriggerColumnCmd composite: triggers all non-ignoring layers, excludes ignoring, one
     slot" (immediate; layer 1 stays -1) and :2788-2803 "Deck::triggerColumn: forced snap queues on every non-ignoring
     layer, skips ignoring ones" (VERIFIED).
R-F7 Every live column-trigger entry calls MainComponent::handleColumnTrigger -> Deck::triggerColumn (:4864): the deck
     view button :718-719, REST :1913, the key / MIDI binding :7617-7636. OSC has no column trigger (OscHandler.cpp:83 is
     per clip). Live capture records a column trigger as per-layer activeClip points sharing a group (RecorderHost.h:170;
     Take.cpp:367-373); a legacy V1 column trigger loads as an inert Comp-scope "columnTrigger" marker (Take.cpp:374-376;
     no consumer in src). ignoreColumnTrigger is never consulted on replay. No "persistent" in src/recording (Take,
     Program, RoutineEngine, PerfState) or src/model/Routine.h (VERIFIED, grep).
R-F8 The [tsan] case R1 "message-thread triggers vs render clock / autopilot on one deck" runs DeckClock::tick against
     message-thread triggers (test_layer_runtime_race.cpp:124, :169) with no persistent layer, so after Stage P it covers
     all of DeckClock::tick's remaining path (VERIFIED).
R-F9 Live active-deck coverage of the stages the retired rows touched (fixtures grepped, VERIFIED): clip transform
     probe-crossfade.json:38 (scale 0.5); clip opacity probe-crossfade.json:44 (clipOpacity 0.5); layer transform
     probe-crossfade.json:59-60 and probe-fitmode.py:340 (layerScale 0.5); layer effects probe-effects-parity (V1 / V5 /
     V6) and probe-crossfade; clip-to-clip transitions probe-crossfade (a-f, k-l); an off-screen fade probe-deck-clock
     d_fade_finishes. NO other probe covers: layer feedback (feedbackEnabled), an FX Only layer (type 2), a media-less
     effect clip on a Transparent layer. The per-deck history key at the GL call sites was covered live only by r2_*
     (test_layer_state_key.cpp:13-15 says so).
R-F10 The R4 rows' only active-deck assertion was "the fixture exercises the stage" (d(reference, absent) >= 5); their
     subject, the persistent path, is what Stage P deletes (probe-render-state.py:267-291, :572-593) (VERIFIED).
R-F11 probe-render-state.sh:80-82 and probe-deck-clock.sh:72-74 quit by app NAME (osascript) and then kill EVERY
     Audio-DNA pid (adna_kill): a Boris app started mid-run would be quit or killed -- the incident class of
     s-rta-1002b-work.md 14:40:58. probe-deck-tabs.sh:84 picks windows to capture by owner NAME (VERIFIED).
R-F12 Composition::routineLoadNote is set by fromVar (Composition.h:670-672) and documented "the app shows it once; never
     silent" (Composition.h:50-52), but nothing in src reads it: there is no load-notice display path today (VERIFIED).
R-F13 Test-only env levers have one house pattern: `#if AUDIODNA_TEST_SERVER` + getenv (VideoPlayer.cpp:204-208,
     GopCacheStore.h:45-49); build/CMakeCache.txt:31 AUDIODNA_BUILD_TEST_SERVER=ON. DeckView::onLayerSelected (DeckView.h:54)
     and InspectorPanel::setActiveTab(Tab::Layer) (InspectorPanel.h:71-72) exist (VERIFIED).
R-F14 LayerInspector.cpp:636-640: Master is a UniversalParamControl (masterControl_, LayerInspector.h:91) laid out full
     width in area = getLocalBounds().reduced(4, 0) (:578). At setSize(300, 900) a full row is x 4, width 292; the
     ignore toggle today is x 150, width 146 (VERIFIED by reading; arithmetic).
R-F15 Docs and test prose the plan's P6 misses (VERIFIED, grep): docs/claude/rendering.md:71, :146; docs/claude/pitfalls.md:115
     (a second mention, "(persistent / nothing to hold)"); .harmony/FEATURES.md:382, :2424; CONTEXT.md:110;
     design/FEATURE_INVENTORY.md:1569, :1779, :1789; tests/CMakeLists.txt:122-126; tests/test_layer_state_key.cpp:8-15;
     tests/test_deck_clock.cpp:52 (case title); .harmony/probe-canvas.json:20. Pitfall 55 does not cite the loop.
     PHASE_GUIDE.md:207 is history (kept).
R-F16 The plan's in-stage order note is inverted: P1 deletes the field that P2-P4's sites read, so "P1 first" cannot
     compile; the readers go first (or one code commit).
R-F17 CLAUDE.md is 24,002 B at fa9604d; the two deletions are 40 + 19 B -> 23,943 B (VERIFIED, wc / grep -o).

## 2. ATTACK RULINGS (26)

### Seat attack-bf9-live
L-A1 [MUST] ship alone -> ACCEPT (amendment 1). Precision: no file of Boris's changes (R-F1). The harm is that an interim
     main gives him nothing he asked for -- the deck model is untouched -- while it deletes the only path that draws
     another deck's layers (Renderer.cpp:760-766). For anyone testing deck switches, that moves main AWAY from his 14:44
     model.
L-A2 [MUST] the invisible playing is left untouched -> ACCEPT (amendments 2, 15). The requirement-to-gate table pre-registers
     K2 and K3 as the RED rows for "nothing plays unseen" and "an off-screen autopilot advances a clip". G5's keep-time rows
     are relabelled "Stage P must not change keep-time"; bf9b inverts them per K6.
L-A3 [MUST] persistent=true is dropped silently -> REJECT for Stage P.
     - No show of Boris's carries the flag (R-F1).
     - Mapping it to ignoreColumnTrigger would silently change a DIFFERENT behaviour: the column skip (Deck.h:113-125)
       is not the cross-deck draw (Renderer.cpp:760-766).
     - A one-time load notice needs a display path that does not exist (R-F12).
     - Under amendment 1 there is no main state in which a formerly persistent layer "goes black on the first switch":
       the first main state is bf9b's shared stack, where no layer leaves the screen on a switch.
     The audit trail moves to where the real data loss happens: bf9b's old-show conversion ("the first deck's layer
     settings win") logs every dropped per-deck setting and every layer whose file carried "persistent": true (K7).
L-A4 [SHOULD] ignore-column under deck switch / MIDI / routine / replay -> ACCEPT-IN-PART. The deck-switch case becomes
     bf9b contract K4 (RED on main). REJECT the MIDI / routine / replay rows for Stage P: every live entry is one function
     Stage P does not touch, and replay never consults the flag (R-F7). Such rows are GREEN on both builds and cannot test
     Stage P. K4 requires every entry to give the same result under bf9b.
L-A5 [SHOULD] perf "none" and a first-frame flash -> ACCEPT-IN-PART. ACCEPT: G6 records /api/state frame_time_ms and
     peak_frame_time_ms (ApiServer.cpp:1381, :1388) on both arms, as INFO. REJECT a ceiling: the change only removes work,
     so any honest bar would sit inside run-to-run noise. REJECT the empty-deck "first 3 frames black" probe for Stage P:
     for a show without persistent layers (every show of Boris's, R-F1), an empty-deck switch takes the fallback path
     (Renderer.cpp:829-844) before and after Stage P; the deleted block runs only when another deck hasPersistentContent
     (Renderer.cpp:742-758). The first frames after a switch are K1's bar. The hidden-layer fallback the seat cites is a
     pre-existing defect, filed as SF-3.
L-A6 [SHOULD] the lint is weak -> ACCEPT-IN-PART. ACCEPT the bare-word regex (amendment 9; R-F2: it catches Layer.h:182
     and has zero false hits). REJECT block-comment stripping: there are zero block-comment hits, and such a hit could only
     false-FAIL. REJECT the feedback-buffer false-hit concern: all 16 other uses are // comments, which the lint strips.
     REJECT a GL unit render test: p_flag_ignored is the render proof, and the lint proves the cross-deck path is gone.
L-A7 [SHOULD] the single-advance test is lost -> ACCEPT. Amendment 10 replaces test_deck_clock case (b) with a
     fromVar-persistent case that is RED on the base. Amendment 12(d) converts d_persistent_single_advance to d_single_advance.
L-A8 [NIT] Link / undo / take / routine paths -> ACCEPT-IN-PART. The operator== edit is amendment 6. REJECT take and
     routine round-trip tests for Stage P: there is no "persistent" in src/recording or Routine.h, and their file formats
     are untouched (R-F7). Link-snapped and quantized triggers across a switch go to K5; "a pre-bf9b take replays" goes to K7.

### Seat STATE / CONCURRENCY
S-A1 [MUST] ship alone -> ACCEPT (amendment 1). One correction to the evidence: F6 is not "more invisible work" net. On
     main, compositePersistentLayers RENDERED those layers every frame (decode, upload, every stage). After Stage P they are
     clock-only (DeckClock, no decode). The seat's real point, that they become unseen, stands.
S-A2 [MUST] test_undo_commands.cpp:188 breaks the compile -> ACCEPT (amendment 6; R-F4).
S-A3 [MUST] stale comments, and the lint misses them -> ACCEPT-IN-PART. ACCEPT the rewords (amendment 7). They include a
     third miss the seat did not name, CompositorEngine.h:510-513 (R-F3). ACCEPT a one-time stale-prose gate with an exact
     phrase allow-list (G3c). REJECT a PERMANENT comment-scanning lint: the word is ordinary English in this tree. There are
     16 other uses in 11 files (R-F3), where the seat's list named 4. A permanent lint would need an ever-growing
     allow-list, and the stale-prose risk exists only once, at removal.
S-A4 [SHOULD] full round trip plus a load log -> ACCEPT-IN-PART. ACCEPT the full-Layer round trip (amendment 5). It
     replaces SECTION 2 and is RED on the base. REJECT the load log for Stage P, on L-A3's evidence; it moves to K7.
S-A5 [SHOULD] ignore-column across decks -> ACCEPT as bf9b contract K4 (RED on main).
S-A6 [SHOULD] tsan coverage for F6 -> ACCEPT-IN-PART. ACCEPT the test_deck_clock case (amendment 10) and pasting the full
     probe-tsan-unit.sh output (G4). REJECT a new [tsan] case: R1 already runs the changed DeckClock::tick under TSan, and
     after Stage P no field is left to vary (R-F8).
S-A7 [SHOULD] the count pins are brittle -> REJECT. The pins are the tsan ruling's review trigger by design
     (test_render_thread_lint.cpp:59-60: "a changed count is a new (or a lost) GL-thread load of the tuple and must be
     re-justified in review"). Stage P removes one load of each kind (R-F5), so the existing test fails unless the pins move
     to 1 / 1. Dropping them would remove a Pitfall 63 guard. bf9b re-justifying them later is the mechanism working.
     The pins are kept, relabelled as a pin update rather than a behavioural RED (amendment 8).
S-A8 [NIT] the empty-deck row depends on the fallback -> ACCEPT (amendment 12(c)). Every p_flag row runs against a
     same-session CONTROL: the same composition without the key. The control proves the fallback was cleared (mean
     RGB <= 2.0); if it is not, the row is INVALID.
S-A9 [NIT] docs misses -> ACCEPT (amendment 13; R-F15).

### Seat gates-tests
G-A1 [MUST] no gate covers the complaint -> ACCEPT (amendment 15): a requirement-to-gate table covering every quoted
     clause, with K1-K8 pre-registered (RED on main where marked).
G-A2 [MUST] the retired rows' coverage was not checked -> ACCEPT (amendment 12(a), (b)). The per-row map (R-F9, R-F10)
     replaces plan R2. The three stages with no other live coverage become active-deck rows a4_* with an A/B bar against the
     base arm: layer feedback, FX Only, and the media-less effect clip. K1t succeeds r2_*.
G-A3 [MUST] the G3 diff range is wrong -> ACCEPT. G3b diffs the stage's own commits (STAGE_P_BASE..STAGE_P_HEAD), and G2
     names the two existing ignore-column cases as required. Both already cover both layer kinds in one trigger (R-F6), so
     no new Deck::triggerColumn case is needed.
G-A4 [SHOULD] bare-word lint -> ACCEPT (amendment 9; R-F2).
G-A5 [SHOULD] the probe bars are not pre-registered -> ACCEPT. A_only = a_only() (probe-render-state.py:517-521). Controls
     and noise floors per amendment 12(c) and G5. Base-FAIL and branch-PASS numbers are pasted per row.
G-A6 [SHOULD] the test idioms -> ACCEPT (amendment 11). A positive control is added. The layout check compares against the
     Master control from the same layout pass and also checks the explicit numbers x 4, width 292 (R-F14). SECTION 2 is
     dropped in favour of amendment 5's round trip.
G-A7 [SHOULD] the visual gate -> ACCEPT-IN-PART.
     - ACCEPT the critic rubric (G7).
     - ACCEPT the hook-safety concern, in a stronger form: the hook becomes a permanent test-server-only lever
       (amendment 4, R-F13), so there is nothing to revert. G3d checks it sits inside the guard.
     - ACCEPT window selection by the launched pid (R-F11).
     - REJECT the decoded-pixel assertion. P3 case 3 proves the geometry headless from the same layout pass, and "nothing
       below moved" holds by construction: LayerInspector.cpp:640 `y += kRowHeight + kSectionGap` is not edited.
G-A8 [SHOULD] the keep-time probes must flip -> ACCEPT (K6 gives each row's disposition).
G-A9 [NIT] the expected test count -> ACCEPT. G2 requires count(STAGE_P_HEAD) = count(STAGE_P_BASE) + 3, i.e. 1114 -> 1117
     if the base is fa9604d. The 1114 is INFERRED from s-rta-1002b-work.md 14:48 "ctest 1114/1114".

## ARCHITECT RULING (s-rta-1002b)

### Amendments -- numbered; each OVERRIDES the plan body where they differ

1. SHIP ORDER (L-A1, S-A1).
   - Stage P is bf9b's Stage 0. Build it FIRST, now, in bf9b's lane worktree on bf9b's branch (name per the bf9b ruling;
     default lane/bf9b), as a contiguous series of commits.
   - Record STAGE_P_BASE (the parent of its first commit) and STAGE_P_HEAD (its last commit) in the lane report. G0-G7
     run at STAGE_P_HEAD.
   - Stage P reaches main ONLY inside bf9b's merge, never alone.
   - Plan text overridden: the header's "ships alone" / "or to ship alone in bf9's build slot (first)"; section 4's
     "Ships alone: yes" and "merge FIRST (bf9's slot)"; R3's mitigation; R6.
   - The session's pre-registered build-order slot "bf9" becomes "bf9b (Stage 0 = Stage P)".
   - If bf9b stalls, Stage P waits on its branch. After any rebase onto a moved main: re-record STAGE_P_BASE / HEAD and
     re-run G1-G4 and G3b. Also re-run G5 when the rebase resolved a conflict in src/render/*, src/model/Layer.*,
     src/ui/LayerInspector.* or src/api/ApiServer.cpp.
   - A standalone merge requires Boris's "now" to the conditional question below.
2. WHAT STAGE P DELIVERS (L-A2). Replace plan R4 with this text, and put the same text in the lane report: "Stage P
   delivers only the two clauses both of Boris's answers share: 'I want to remove the persistent' and 'the only thing
   remotely persistent should be to ignore column controls'. It delivers neither the 14:30 reading (a deck change stops
   the old deck) nor the 14:44 model (decks are boxes of clips). What plays across a deck switch is bf9b's, gated by K1-K8."
3. COMMIT ORDER (R-F16). Replaces section 4's "Order inside the stage".
   - C0 = amendment 4's lever.
   - C1 = every RED or changed test: amendments 5, 6, 8, 9, 10, 11; delete test_compositor.cpp:423-437; replace
     tests/test_layer_inspector_persistent_toggle.cpp and its CMake target; the tool_uitoggle_snapshot.cpp edits. C1
     builds on the base code, and ctest at C1 shows exactly G2's RED set failing.
   - C2 = code removal, readers first: P4 (ApiServer.cpp) -> P3 (LayerInspector) -> P2 (CompositorEngine, Renderer,
     DeckClock, rewords) -> P1 (Layer.h, Layer.cpp). One commit or four; every commit builds the app and the tests.
   - C3 = P5 (probes). C4 = P6 (docs). STAGE_P_HEAD = C4.
4. TEST-ONLY LEVER (G-A7). Replaces G7's temporary ADNA_BF9_INSPECT_LAYER hook.
   - In MainComponent, inside `#if AUDIODNA_TEST_SERVER`, following the house pattern at VideoPlayer.cpp:204-208:
     `if (const char* e = std::getenv("ADNA_INSPECT_LAYER"))`. After startup (MessageManager::callAsync with a
     Component::SafePointer), call deckView_->onLayerSelected(std::atoi(e)) and
     inspectorPanel_->setActiveTab(InspectorPanel::Tab::Layer).
   - Comment: "TEST-ONLY (test-server builds): visual gates of the Layer tab".
   - The lever is permanent and inert unless the variable is set.
   - It is C0, so the C0 build is base behaviour plus the lever. That build is G5's BASE arm and G7's BEFORE.
5. P1 RED TEST (S-A4, G-A6). Replaces the plan's P1 test and its SECTION 2.
   - In tests/test_composition.cpp, ONE TEST_CASE "Layer: a file's persistent key is ignored and never written back;
     every other field round-trips (bf9 Stage P)" [composition][serialization][backcompat].
   - Setup: a Layer L with ignoreColumnTrigger = true, opacity 0.4, blendMode != 0, transitionSpeed 2.5.
   - v0 = L.toVar(); v0.getDynamicObject()->removeProperty("persistent") (a no-op after the removal).
   - v1 = JSON::parse(JSON::toString(v0)), then set "persistent" = true on v1.
   - Layer fresh; fresh.fromVar(v1); out = fresh.toVar().
   - CHECK(fresh.ignoreColumnTrigger); CHECK_FALSE(out has "persistent"); CHECK(JSON::toString(out) == JSON::toString(v0)).
   - It compiles on the base and FAILS there: fromVar reads the key and toVar writes it back as true.
   - If the string comparison fails on the branch for any field other than "persistent", compare property by property
     and name that field in the lane report. That would be a pre-existing round-trip gap, not Stage P's.
6. P1 FILES (S-A2, L-A8). Add tests/test_undo_commands.cpp:188: delete `&& a.persistent == b.persistent` and keep
   `a.ignoreColumnTrigger == b.ignoreColumnTrigger` (it is the undo-exactness comparator). C1 carries this edit; it
   compiles on the base.
7. REWORDS ADDED TO P2 (S-A3, S-A9; R-F3, R-F15).
   - MainComponent.cpp:3007-3009 -> "(Renderer::renderOpenGL()'s off-screen-deck loop over composition_->decks --
     DeckClock::tick and each deck's autopilot -- is nested under the `deckActive` check, so nulling activeDeck_ fences it
     as well)".
   - MainComponent.cpp:3235: "renderOpenGL()'s P21 persistent-layer loop" -> "renderOpenGL()'s off-screen-deck loop
     (DeckClock / autopilot)".
   - CompositorEngine.h:510-513: drop "(s-rta-0926b: shared with hasPersistentContent)".
   - tests/test_layer_state_key.cpp:8-15 -> the bug it pins is layer ids repeating across decks (history must not cross
     a deck switch), plus: "the live per-deck call-site rows r2_* were retired with Persistent (bf9 Stage P); successor:
     bf9b gate K1t". tests/CMakeLists.txt:122-126 gets the same change.
   - tests/test_deck_clock.cpp:52, the case title -> "(a) an inactive deck's layer finishes its fade at the real rate".
   - .harmony/probe-canvas.json:20: drop ", no persistent content".
   - src carries no new comment that names the removed feature; the explanation lives in performance-controls.md.
   - With these, all 87 feature lines of R-F3 are covered.
8. P2(a) RELABEL (S-A7). P2(a) is a pin update the tsan lint requires: CompositorEngine.cpp {runtime() 1, getActiveClip(
   1} (R-F5), with the site comment at :58-66 rewritten (one load of each, both compositeDeck's). It FAILS at C1 and
   passes at C2. It is not a behavioural RED.
9. P2(b) LINT (G-A4, L-A6).
   - Regex `\bpersistent\b|canBePersistent|compositePersistentLayers|hasPersistentContent|beginEmptyActiveDeck|persistentToggle_`.
   - Applied to codeLines() of every *.h / *.cpp / *.mm under AUDIODNA_SRC_DIR, recursively, with an empty allow-list;
     every hit is listed as file:line.
   - At C1: 33 hits (R-F2), so it FAILS. At C2: 0.
10. test_deck_clock CASE (b) REPLACED (L-A7, S-A6).
    - Delete :71-102. Add TEST_CASE "(b) a layer loaded with \"persistent\": true is an ordinary layer: DeckClock::tick
      advances its fade once per tick and clocks its media (bf9 Stage P)" [deck_clock], with two SECTIONs: Transparent
      and Mask.
    - Steps: v = deck.getLayer(1)->toVar(); v.getDynamicObject()->setProperty("persistent", true);
      deck.getLayer(1)->fromVar(v); startFade(deck, 1, 2.0f, Video, Video); set the type; Recorder rec;
      DeckClock::tick(deck, 0.5f, std::ref(rec)).
    - Checks: CHECK(crossfadeProgress == Approx(0.25f)); REQUIRE(rec.ticked.size() == 2); ticked[0] == getClipAt(1);
      ticked[1] == getClipAt(0).
    - It FAILS at C1 (progress 0.0 in both sections) and passes at C2. The header at :12-16 is reworded per the plan.
11. P3 TESTS (G-A6). tests/test_layer_inspector_layer_row.cpp, with EXACTLY 3 TEST_CASEs. Children are found as in the old
    file (getChildren + dynamic_cast).
    - (1) REQUIRE a ToggleButton "Ignore Column Trigger" (the positive control), THEN CHECK there is no ToggleButton
      "Persistent". FAILS at C1.
    - (2) As plan P3(2): for every Layer::Type, the toggle mirrors both values, a click writes the field, and the tooltip
      is "This layer ignores column trigger buttons". Passes at C1 and at C2.
    - (3) setSize(300, 900), setLayer(an Opaque layer), resized(). The toggle's x == 4 and width == 292, AND they equal the
      x and width of the UniversalParamControl child whose getBottom() == toggle.getY() (the Master control, from the same
      layout pass). FAILS at C1 (x 150, width 146).
    - P3(4) is amendment 4.
12. P5 PROBES (G-A2, G-A5, S-A8, L-A7; R-F11).
    (a) RETIRE these rows: r2_temporal, r2_ring, r4_clip_transform, r4_clip_opacity, r4_layer_effects, r4_layer_transform,
        r4_feedback, r4_transition, r4_opaque_opacity, r4_opaque_overlay, r4_fxonly_persistent, r4_fxonly_medialess,
        r4_mask_skipped, r4_empty_active_deck. RETIRE these fixture keys: r2, r4, r4_transition, r4_opaque, r4_fxonly,
        r4_empty_active_deck.
        The docstring records the R-F9 / R-F10 map: which probe now covers each stage; r2_*'s successor is K1t; r4_opaque_*,
        r4_mask_skipped, r4_empty_active_deck and r4_transition tested persistent-only behaviour. That map replaces plan R2.
    (b) ADD a4_feedback, a4_fxonly, a4_fxonly_medialess. Deck 0 only. New key "a4" = {"feedback": <the old r4.feedback>,
        "fx": [["Invert", 1.0]]}.
        - Fixture: layer 0 Opaque A, plus layer id 5 = a Transparent B with the feedback values (a4_feedback) | type 2 with
          the media-less fx clip (a4_fxonly) | type 1 with the media-less fx clip (a4_fxonly_medialess).
        - Steps: trig(0, 0), trig(1, 0), settle 2.5 s, cap1, wait 0.5 s, cap2.
        - New env RSTATE_REF=<dir>: when set, the row also loads <dir>/<row>_cap1.png (the BASE arm's frame) for bar (ii).
    (c) ADD p_flag_ignored, p_flag_ignored_empty, p_api_no_field, p_ignore_column (bars in G5). New key "p_flag" =
        {"lock": [756, 878]}, moved from r4_empty_active_deck.
    (d) probe-deck-clock: d_persistent_single_advance -> d_single_advance. The fixture is unchanged: the layer JSON keeps
        persistent=True, which is ignored after Stage P. JSON key "persistent" -> "single_advance" (T 4.0, range
        [0.35, 0.65]); the docstring and .sh :5 are updated.
    (e) QUIT ONLY WHAT YOU LAUNCHED (rig safety). New helper .harmony/probe-quit-ours.sh, sourced by both scripts.
        - OURPID = the ucomm "Audio-DNA" pid read right after /api/health answers. The pre-launch refuse guarantees it is
          ours.
        - quit_ours(), when the running Audio-DNA pids == OURPID: osascript quit by name, wait up to 30 s, then kill OURPID
          only.
        - quit_ours(), when any other Audio-DNA pid runs: never osascript by name; kill -TERM OURPID (-KILL after 10 s);
          print "FOREIGN Audio-DNA pid <p> running -- untouched".
        - "app terminated" checks OURPID only. adna_kill (all pids) is deleted from both scripts.
    (f) The .py docstrings and .sh headers are updated: rows, fixtures, calibration notes.
13. P6 DOCS (S-A9; R-F15). Plan section 6, plus:
    - docs/claude/rendering.md:71: "a persistent layer, or a layer with nothing to hold" -> "a layer with nothing to hold".
    - rendering.md:146: "(active deck, persistent layers, the outgoing clip of a crossfade)" -> "(the active deck's clips
      and the outgoing clip of a crossfade)".
    - docs/claude/pitfalls.md:115: also "(persistent / nothing to hold)" -> "(nothing to hold)".
    - .harmony/FEATURES.md:382: "its persistent blend mode" -> "its blend mode". FEATURES.md:2424: drop "persistent toggle, ".
    - CONTEXT.md:110: delete the "Persistent Layer" glossary line.
    - design/FEATURE_INVENTORY.md:1569 -> "only the active deck renders"; delete :1779 and :1789.
    - CLAUDE.md goes from 24,002 to 23,943 B (R-F17).
    - PHASE_GUIDE.md and BORIS_DECISIONS.md are untouched (history).
14. GATES. Plan section 5 is replaced by the FINAL CONSOLIDATED GATE LIST below.
15. bf9b CONTRACT (G-A1, L-A2, G-A8, L-A4 / S-A5, L-A8, L-A3 / S-A4). The requirement-to-gate table and K1-K8 below go to
    the bf9b ruling verbatim. bf9b may refine fixtures and drivers but never weaken a bar or drop a row. Its lane report
    records each K row's RED numbers on the STAGE_P_HEAD build and GREEN numbers on bf9b's head.
16. tsan-r5 SCOPE. tsan-r5 does not convert Layer::persistent to Relaxed<T>, because Stage P deletes it (a plain bool
    today, written by the message thread and read by GL). Plan R5's "re-scope tsan-r5 against post-bf9b code" stands.
17. BORIS PAGE. There is no checkpoint for Stage P alone. Plan section 8 (8.1-8.3) and G7's before / after move into bf9b's
    Boris page. Section 9 stays "none" (see the questions below).

### FINAL BUILD STAGES
Stage P = bf9b Stage 0. One builder context, in bf9b's worktree / branch; build now:
  C0  the lever (amendment 4)
  C1  RED tests and test edits: amendments 5, 6, 8, 9, 10, 11; test_compositor.cpp:423-437 deleted; the test target renamed
      in tests/CMakeLists.txt:2199-2247, plus the comment at :3086-3087; tool_uitoggle_snapshot.cpp keeps ONE LayerInspector
      shot, "layerinspector-layer-row-headless.png"
  C2  code removal, readers first: P4 -> P3 (incl. F3's full row) -> P2 (+ amendment 7) -> P1. Plan text for each; forks
      F1-F6 unchanged
  C3  P5 probes (amendment 12)
  C4  P6 docs (amendment 13)
  Gates G0-G7 run at STAGE_P_HEAD (C4). There is no merge on its own (amendment 1).
bf9b Stages 1..n: per the bf9b ruling, on the same branch, gated by bf9b's own gates plus K1-K8. Main receives Stage P and
bf9b in ONE merge.

### FINAL CONSOLIDATED GATE LIST (pre-registered; Harmony copies gate strings only from here)
Arms: BASE = the C0 build, copied to a scratch path before any rebuild; BRANCH = the STAGE_P_HEAD build.
Live work: run inside lock.sh acquire_quiet_lock (.harmony/.reports/s-rta-1002b/wf/lock.sh -- it waits while any
Audio-DNA it did not start is running). open -g only; never an Output window; never touch a foreign app.

G0 RIG SAFETY (before any live run).
  (a) In .harmony/probe-render-state.sh and .harmony/probe-deck-clock.sh, `grep -n 'tell application "Audio-DNA" to quit\|adna_kill'`
      -> every hit is inside quit_ours() (.harmony/probe-quit-ours.sh), on its only-ours branch; adna_kill is gone.
  (b) Decoy self-test. Run a process whose ucomm is "Audio-DNA" (a copy of /bin/sleep named Audio-DNA under $TMPDIR, or a
      one-line C stub), and set OURPID to some other pid. Expected: quit_ours exits non-zero and prints FOREIGN, and
      `kill -0 <decoy>` still succeeds. Afterwards, kill the decoy by its own pid.
  PASS = (a) and (b), with the output pasted in the lane report.
G1 BUILD.
  - `cmake --build build --config Release -j$(sysctl -n hw.ncpu)` at STAGE_P_HEAD -> rc 0.
  - Every Stage P commit builds the app and the tests (rc listed per commit).
  - Compiler warnings in Stage P-touched files: count at HEAD <= count at STAGE_P_BASE (from clean rebuild logs of both).
G2 UNIT.
  - `ctest --test-dir build -j8 --output-on-failure` at STAGE_P_HEAD -> 0 failures, and count(HEAD) == count(STAGE_P_BASE)
    + 3 (1117 if the base is fa9604d at 1114).
  - Required present and passing:
    - the P1 round-trip case (amendment 5);
    - test_render_thread_lint "render thread: one trigger-tuple load per layer per pass (pinned counts)", with
      CompositorEngine.cpp at {1, 1};
    - the new lint case "no Persistent-feature identifier left in src/";
    - test_deck_clock "(b) a layer loaded with \"persistent\": true is an ordinary layer: ...";
    - test_layer_inspector_layer_row's 3 cases;
    - test_undo_commands "TriggerColumnCmd composite: triggers all non-ignoring layers, excludes ignoring, one slot";
    - test_undo_commands "Deck::triggerColumn: forced snap queues on every non-ignoring layer, skips ignoring ones".
  - RED record at C1, exactly these FAIL: the P1 round trip, the pinned-counts case, the identifier case, deck_clock (b),
    layer_row (1), layer_row (3). layer_row (2) PASSES.
G3 TEXT (at STAGE_P_HEAD).
  (a) `grep -rnE 'canBePersistent|compositePersistentLayers|hasPersistentContent|beginEmptyActiveDeck|persistentToggle_|(\.|->)persistent\b|"persistent"' src`
      -> 0 lines (comments included).
  (b) Ignore-column untouched:
      - `git diff -U0 STAGE_P_BASE..STAGE_P_HEAD -- src/model/Deck.h src/model/Layer.h src/model/Layer.cpp src/MainComponent.cpp src/ui/LayerInspector.h | grep -cE '^[+-][^+-].*ignoreColumn'`
        -> 0.
      - The same command on src/ui/LayerInspector.cpp -> exactly 2 (the old and new ignoreColumnToggle_.setBounds lines).
      - `grep -c 'a.ignoreColumnTrigger == b.ignoreColumnTrigger' tests/test_undo_commands.cpp` -> 1.
  (c) Stale prose in src. ALLOW = the 14 fixed strings below, one per line:
        persistent no-input / no-device notice
        is a persistent notice
        nothing persistent references them
        swing is a persistent style feature
        persistent feedback buffer
        Persistent feedback buffer
        persistent prev-frame buffer
        Persistent library for compositor
        persistent VideoPlayer/ImageSequence state
        crossfadeProgress is persistent
        persistent frame-to-frame feedback
        Previous frame (persistent)
        the PERSISTENT key
        persistent EMA memory
      - `grep -rni persistent src | grep -v -F -f ALLOW` -> 0 lines.
      - `grep -rni persistent src | grep -c -F -f ALLOW` == the same count at STAGE_P_BASE (16 at fa9604d).
  (d) `grep -n -B4 -A6 'ADNA_INSPECT_LAYER' src/MainComponent.cpp` shows the getenv line between `#if AUDIODNA_TEST_SERVER`
      and its `#endif`. No other src file names the variable.
  (e) `wc -c CLAUDE.md` == the size at STAGE_P_BASE minus 59 (24,002 -> 23,943), and <= 25,000.
  (f) Stale prose in docs. DOCALLOW = these fixed strings:
        no persistent audio storage
        No persistent writes
        no persistent state
        persistent-intent field
        to persistent storage
      `grep -n -i persistent CLAUDE.md CONTEXT.md docs/claude/*.md .harmony/APP-INVENTORY.md .harmony/FEATURES.md design/FEATURE_INVENTORY.md | grep -v -F -f DOCALLOW`
      -> every remaining line contains "removed" (only performance-controls.md's "Persistent layers: removed" paragraph).
G4 TSAN (Pitfall 63). `.harmony/probe-tsan-unit.sh` at STAGE_P_HEAD -> exit 0; `ctest -L tsan` 4 / 4 PASS
   (EXPECTED_TSAN_CASES = 4); zero "WARNING: ThreadSanitizer". Paste the full output in the lane report (R1 runs the changed
   DeckClock::tick).
G5 LIVE (G0 done first).
  Run 1, BASE:
    `RSTATE_APP=<BASE app> .harmony/probe-render-state.sh <out-base> p_flag_ignored,p_flag_ignored_empty,p_api_no_field,p_ignore_column,a4_feedback,a4_fxonly,a4_fxonly_medialess`
    -> expected: p_flag_ignored FAIL, p_flag_ignored_empty FAIL, p_api_no_field FAIL (the RED evidence); p_ignore_column
    PASS; a4_* bar (i) PASS. Keep this run's out dir as REF.
  Run 2, BRANCH:
    `RSTATE_APP=<BRANCH app> RSTATE_REF=<Run 1 out dir> .harmony/probe-render-state.sh <out-base> p_flag_ignored,p_flag_ignored_empty,p_api_no_field,p_ignore_column,a4_feedback,a4_fxonly,a4_fxonly_medialess,r1_temporal,r1_control,r1_ring,r1_retrigger,r1_counts,r1_cells,r5_hold,r5_burst`
    -> every row PASS.
  Run 3, BRANCH:
    `DCLOCK_APP=<BRANCH app> .harmony/probe-deck-clock.sh <out-base> d_fade_finishes,d_single_advance,d_pending_trigger_still_cancelled,d_video_keeps_time,d_imageseq_keeps_time,d_autopilot_keeps_time,d_return_hitch`
    -> every row PASS. Stage P must not change keep-time; bf9b inverts these rows per K6.
  Run 4, BASE: `DCLOCK_APP=<BASE app> .harmony/probe-deck-clock.sh <out-base> d_single_advance` -> PASS (a guard).
  Bars. tol = 1.5; noise = d(cap1, cap2) for two captures 0.5 s apart; floor = max(1.5, 4 x noise).
    p_flag_ignored
      - subject: deck 0 layer 0 Opaque A; deck 1 layer id 5 Transparent, "persistent": true, clip B (covers the frame).
      - control: the same file without the key.
      - Each: load, persist_setup(), settle 2.5 s, cap.
      - VALID iff d(control, a_only("pfi")) <= tol (a_only is probe-render-state.py:517-521).
      - PASS iff nonblank(subject) and d(subject, control) <= tol.
      - BASE expected: d(subject, control) ~ d(B, A) (FAIL).
    p_flag_ignored_empty
      - subject: deck 0 = one Opaque layer whose clip A is never triggered; deck 1 as above.
      - control: the same file without the key.
      - Each: load, persist_setup(active_layer_idx_list=()), settle 2.0 s, cap at lock 756x878.
      - VALID iff mean RGB(control) <= 2.0. That means switch(0)'s refresh cleared the fallback (MainComponent.cpp:5000-5005),
        so the frame is black (Renderer.cpp:836-844).
      - PASS iff d(subject, control) <= tol.
      - BASE expected: black + B (FAIL).
    p_api_no_field
      - After loading the p_flag_ignored subject, no decks[].layers[] object of GET /api/composition has "persistent", and
        every one has id, visible and activeClipColumn.
      - BASE: the key is present (FAIL).
    p_ignore_column
      - Deck 0: 2 layers x 2 image columns; layer 1 has "ignoreColumnTrigger": true.
      - trigger_clip(1, 0); POST /api/trigger_column {"column": 1}; 0.5 s later, layer 0 activeClipColumn == 1 and
        layer 1 == 0.
      - PASS on both arms.
    a4_<stage>
      - (i) d(cap1, absent) >= 5. absent = a_only() for the two fx rows, and the same file with feedbackEnabled false for
        a4_feedback.
      - (ii) BRANCH only: d(cap1, REF cap1) <= max(1.5, 4 x the REF run's noise).
    d_single_advance: crossfadeProgress at T/2 after the switch is in [0.35, 0.65]. A double advance reads ~1.0
      (probe-deck-clock.py:22-24).
    All other rows: their existing bars.
  Paste both arms' numbers for every row in the lane report.
G6 PERF (INFO, no bar). Read /api/state frame_time_ms and peak_frame_time_ms at the end of p_flag_ignored on both arms, and
   print them.
G7 VISUAL WORK GATE.
  - BEFORE = the BASE app; AFTER = the BRANCH app. Launch each as `open -g --env ADNA_INSPECT_LAYER=0 <app> --args --test-mode`,
    record its pid, and quit it under quit_ours() rules.
  - Capture window-only with `screencapture -x -o -l <CGWindowID>`: the largest on-screen window whose kCGWindowOwnerPID ==
    that pid (never by owner name). Decode both captures.
  - Also capture tool_uitoggle_snapshot's layerinspector-layer-row-headless.png (supplementary).
  - Critic panel on BEFORE / AFTER. Each critic answers its question yes / no, with a reason:
    - visual-design: "Does the Layer section show the Master control and then ONE full-width Ignore Column Trigger row,
      with no gap, overlap or orphaned space where Persistent was?"
    - UX: "From the label and tooltip alone, can Boris tell what Ignore Column Trigger does, and does nothing suggest a
      missing control?"
    - graphic-design: "Are the toggle's left edge, tick box and text aligned with the Master row and consistent with the
      rows below?"
    - logic: "Does every control in the Layer section still map to a live model field (no dead control)?"
    - interaction-logic: "Is the toggle's tick the same in BEFORE and AFTER for the same default layer (unticked), i.e.
      is the model mirrored?"
  - PASS = five yes. The captures and verdicts go into bf9b's Boris page (amendment 17).

### bf9b CONTRACT GATES (pre-registered; RED measured on the STAGE_P_HEAD build, GREEN on bf9b's head)
Requirement -> gate (Boris's words verbatim; user request item 6 and BORIS_DECISIONS.md:350-352):

| Boris (verbatim) | Gate(s) | Owner | RED before bf9b |
|---|---|---|---|
| "I'm going back on what I asked for before and I want to remove the persistent." | G2 (P1 round trip, lint, layer_row 1), G3a / G3c, G5 p_flag_ignored, p_flag_ignored_empty, p_api_no_field | Stage P | yes, on the C0 build |
| "The only thing remotely persistent should be to ignore column controls so if we want to have something stay on a certain layer and we want to play with the other layers in the composition, that stays in the other layers can be switched by a column switch." | Within one deck: G2's named undo cases, layer_row (2), G3b, G5 p_ignore_column. Across decks: K4 | Stage P / bf9b | K4: yes |
| "When I change decks, they clipped kept playing, and it was invisible. This is not good." | K2, K3 | bf9b | yes |
| "Whatever is in the layer should be what is playing and there should be nothing else." | K2, K3 | bf9b | yes |
| "when I switch between decks, do not change the clips playing in the layers or how they are playing." | K1, K1t, K5 | bf9b | yes |
| "treat the decks as just a box of clips and I can switch between 20 decks looking for a clip" | K8 | bf9b | yes |
| "and the playing will not be affected." | K1, K3, K5, K8 | bf9b | yes |

floor = max(1.5, 4 x noise) throughout, with noise = d between two captures of the same steady scene 0.5 s apart in the
same run.

K1 A deck switch changes nothing on screen.
   Drivers: every deck-switch path in plan 2.4 rows 1-10 that has a non-synthetic driver (REST /api/switch_deck, OSC, the
   take-replay activeDeck lane, the deck commands' REST / undo entries). A path with no driver must be shown to call the same
   function, by a unit test or a call-site lint.
   K1a: deck 0 layer 0 = static image A, fired. Deck 1's cells hold other clips; none fired. Switch 0 -> 1. Capture: the
        first frame requested right after the switch request returns, then +0.5 s, then +2 s. PASS iff every
        d(after, before) <= floor.
   K1b: deck 0 layer 0 = ramp12.mp4, fired, with t >= 4 s at the switch (t = 12 x mean G / 255). PASS iff at +2 s,
        |t - (t_switch + 2.0)| <= 0.5 s, and the REST playheadPosition moved by 2.0 / 12 within 0.06.
   K1c: layer 0 is mid-crossfade (Dissolve, T = 4 s) at the switch. PASS iff the frames after the switch lie on the OUT -> IN
        line (fit residual <= tol, as in the retired r4_transition), p rises by >= 0.15 between +0.5 s and +2 s, and the
        fade completes by T + 1 s.
K1t History follows the layer (the successor of r2_*). Run K1a with the clip carrying [Freeze 0.5], and again with the a4
    feedback fixture. PASS iff d(after, before) <= floor at +0.5 s and at +2 s.
K2 Nothing plays unseen. Fire clips from deck 0 and from deck 1 into different layers, then make 5 switches 0 <-> 1 within
   10 s. Then read every clip's playhead twice, 1 s apart (GET /api/composition, or bf9b's documented successor field).
   PASS iff every clip whose playhead advanced is playing in a layer (or is the outgoing clip of a fade in progress); the
   count of advancing clips not in a layer must be 0.
K3 A switch does not affect autopilot. Layer 0 autopilot: PlayNext, Beat4, over columns A, B, C (the d_autopilot_keeps_time
   fixture). Inject 12 beat crossings (inject_features; totalBeatCount moves with the phase, Pitfall 42). Run R1 with no
   switch, and R2 with switches at beats 3 and 9. PASS iff R2's decoded frames at beats 4, 8 and 12 equal R1's within floor,
   AND no clip outside a playing layer changes activeClipColumn, playhead or beatsPlayed.
K4 Ignore-column across decks. Layer L (Ignore Column Trigger on) plays X, fired from deck 0; the other layers play deck 0's
   column 0. Switch to deck 1. Fire column 1 via REST /api/trigger_column and via every other entry that has a driver.
   All entries share handleColumnTrigger (MainComponent.cpp:719, :1913, :7636), and bf9b keeps ONE entry function.
   PASS iff L still plays X (REST, plus its decoded region within floor) and every other layer plays deck 1's column-1 clip
   for its row.
K5 Queued triggers survive a switch. Queue a Bar-snapped trigger on deck 0, then switch to deck 1 before the bar.
   PASS iff it fires on that bar (REST and frame), with Link off and with Link on.
K6 probe-deck-clock disposition. bf9b's lane report lists every row's new name and bar.
   - d_fade_finishes -> INVERTED (K1c).
   - d_single_advance -> RETIRED if bf9b deletes DeckClock (plan 2.5 #7), otherwise kept.
   - d_pending_trigger_still_cancelled -> INVERTED (K5).
   - d_video_keeps_time, d_imageseq_keeps_time -> INVERTED (K1b, K2).
   - d_autopilot_keeps_time -> INVERTED (K3).
   - d_return_hitch -> RETIRED; its 50 ms bar moves to K8.
K7 Old shows and takes.
   - A show saved before bf9b loads: 2 decks whose layers differ in settings, and one layer carrying "persistent": true.
   - bf9b's conversion (binding-decisions BF9 CLARIFIED default: the first deck's layer settings win) is applied, and ONE
     logLine names every dropped per-deck layer-setting set and every layer whose file carried "persistent": true.
   - bf9b's converter is then the only src reader of that key; add it to the Stage P lint's allow-list by file name.
   - A take recorded before bf9b, with an activeDeck lane, replays without error, and that lane changes only the grid.
   - A user-visible notice first needs a display path (R-F12).
K8 20 decks. In a 20-deck show, REST switch_deck through all 20 within 10 s. At every switch: GET /api/state activeDeck ==
   the target (the grid follows), K1a's bar holds, and peak_frame_time_ms across the switch <= 50 ms.

### BORIS QUESTIONS (with defaults)
None for Stage P. He ruled it twice ("I want to remove the persistent"; ignore-column stays), and no show of his uses it
(R-F1).
Pre-written; ask ONLY if Harmony wants Stage P on main before bf9b (amendment 1): "Removing 'Persistent' can go in now, or
together with the 'decks are boxes of clips' change. If it goes in now, then until that change lands, a deck switch shows
only the new deck's layers -- exactly like today for any layer without Persistent (your saved show has none). Now, or
together?" Default: together.

### SIDE FINDINGS (for Harmony; outside Stage P)
SF-1 Composition::routineLoadNote is never displayed (R-F12). A file whose routine pads were dropped loads silently,
     despite Composition.h:50-52 "the app shows it once; never silent".
SF-2 Probe scripts that copy the render-state launcher likely quit by name and kill every Audio-DNA pid (R-F11). Stage P
     fixes the two it runs (amendment 12(e)); the rest stays with the FILED audit (s-rta-1002b-work.md 14:40:58).
     probe-deck-tabs.sh:84 picks the windows to capture by owner name.
SF-3 (INFERRED) A pre-existing defect: if a deck's only active Image / Source clip sits on a HIDDEN (or bypassed or
     soloed-out) layer, that clip still shows, through the legacy fallback. compositeDeck returns 0 (CompositorEngine.cpp:
     1048-1059), and refreshPreviewFromActiveClip mirrors the first active clip with no visible check (MainComponent.cpp:
     4972-5006). Cheapest test: a one-layer deck; trigger an image; hide the layer; render_frame -> expected black. bf9b
     re-derives the fallback rule in any case.

### RISKS OF THIS RULING
R-1 Strongest counterargument to amendment 1: Boris said "remove the persistent", and holding it delays his instruction. It
    loses. Alone, the removal gives him nothing he asked for (the deck model does not change); no show of his uses it
    (R-F1); and the interim main would show the opposite of his 14:44 model to anyone testing deck switches. It lands in the
    same delivery as the behaviour it is redundant with.
R-2 Branch drift while bf9b is built: bf7, ui and bf1 may merge first. bf7 touches Layer.h near :181 and the LayerInspector
    rows; ui touches ApiServer.cpp ~:436-450 and performance-controls.md:51; bf1 touches compositeDeck / saveLayerOutput and
    Renderer.cpp ~:880-905. Stage P's hunks are deletions plus one setBounds line, and amendment 1's re-run rule covers
    rebases.
R-3 The a4_* A/B depends on deterministic static captures. Layer feedback converges geometrically, so the 2.5 s steady
    state should repeat. If the REF run's noise is large, the floor widens and the row loses teeth: the builder reports the
    noise, and Harmony judges.
R-4 The K gates constrain bf9b's design only through Boris's verbatim clauses. If bf9b's ruling finds a K bar unreachable,
    that becomes a question for Boris, never a silent drop.
R-5 The lever is permanent code in test-server builds, and the build Boris runs is one (build/CMakeCache.txt:31). It is
    inert unless ADNA_INSPECT_LAYER is set.
R-6 G0's decoy may not get ucomm "Audio-DNA" if macOS refuses to run a copied system binary. In that case, use a compiled
    one-line C stub named Audio-DNA.

STATUS: FINAL -- 26 attacks ruled (16 ACCEPT / 8 ACCEPT-IN-PART / 2 REJECT), 17 amendments; Stage P = bf9b Stage 0, ready to build now on bf9b's branch, merges to main only with bf9b; Boris questions: none
