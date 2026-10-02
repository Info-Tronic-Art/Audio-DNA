# RULING lane bf1 (s-rta-1002b) -- the blind council's attacks on plan-bf1, ruled

Architect (Fable), 2026-10-02. Base: main fa9604d (src identical in meaning to the plan's 5e47d17). Inputs: the plan
.harmony/.reports/s-rta-1002b/plan-bf1.md (NOT edited -- this ruling overrides it wherever an amendment names a section),
the three seat papers VERBATIM in .harmony/.reports/s-rta-1002b/attack-bf1-papers.md, Boris's words (backlog BF1;
binding-decisions.md:574-577 "whole ouput of select layer, stop start and snap to bar, next empty cell on top layer no need
to select it, video files yes"; the 14:44 clarification :598-606 and BORIS_DECISIONS.md:350-356 "decks are boxes of
clips"). Every new claim below was read at fa9604d (file:line). No app launched; no scratch run needed.

## 0. VERDICT

The plan holds, with 17 amendments (section A). None changes what Boris asked for: the whole output or the selected
layer, press to start and press to end, both on the bar, the take landing in the top layer's next empty cell with no
selecting, video files. All 35 attacks ruled: 27 ACCEPT, 8 PARTIAL (a named part rejected with evidence), 0 REJECT. What
changes:
- The take lands in the deck where REC was PRESSED. Both stateful seats asked for this; Boris Q6.
- The bar clock is "bpm > 0", not "trackerState == LOCKED".
- A Layer take never records black.
- A press while waiting is ignored (the routine pads' rule), with a 250 ms debounce.
- Two binding actions: Record Output and Record Selected Layer.
- The GL thread never notifies.
- S2 waits until bf9b is MERGED.
- The gate list is rebuilt. Self-found: the plan's G4 could not see the tap's cost (N7).
ready_to_build: S1 YES now (new files only). S2 BLOCKED until lane bf9b is merged (plan-bf9b.md is still a skeleton).
S3 after S2 and lane ui.

## 1. NEW EVIDENCE (read at fa9604d for this ruling)

N1 The bar clock. FeatureSnapshot::bpm = BPMTracker::lockedBPM_ (AnalysisThread.cpp:203; "0 if unknown",
   FeatureSnapshot.h:40). lockedBPM_ is only ever assigned positive values (BPMTracker.cpp:158, :184, :615), on a
   tracker built once (AnalysisThread.cpp:36). So once it is above 0 it stays above 0. While it is above 0 the phase
   free-runs and every whole beat is counted (BPMTracker.cpp:213-232), so bar edges keep coming.
   - Manual BPM, Link and REST set_bpm all post a tempo request that sets STATE_LOCKED (BPMTracker.cpp:609-621;
     MainComponent.cpp:5622-5647). Manual mode then skips the state machine (BPMTracker.cpp:99-104).
   - Silence keeps the state (:125-130).
   - The one real gap is a tempo change: the state reads LOCKING while lockedBPM_ > 0, until 200 consistent confident
     hops (>= ~2.1 s; BPMTracker.cpp:170-196, BPMTracker.h:48). The bar clock keeps running, but MainComponent.cpp:31
     calls it "unlocked".
N2 The lock rule is applied once, when the trigger is pressed (MainComponent.cpp:27-34). Once queued, a bar-snapped trigger
   fires on any later beat crossing with no further lock check (Autopilot.cpp:65-80).
N3 Test mode. The analysis thread does not run in test mode (MainComponent.cpp:1859-1862; TestServer.cpp:483-484).
   - The beat clock moves ONLY when a probe injects a snapshot. The HTTP thread publishes it with no message-thread hop
     (TestServer.cpp:56-75).
   - No REST route sets the global quantize; a composition file does (Composition.h:318, :451).
   - /api/debug/gl_context_cycle is on 8080 (TestServer.cpp:255).
   - No REST route saves a composition, moves or removes a layer, selects a cell, or runs undo / redo
     (ApiServer.cpp:163-338).
   - 7070 /api/state has peak_callback_ms, video_late_frames, fence_hold_frames and peak_message_stall_ms
     (ApiServer.cpp:1390, :1448, :1490, :1508). These reset when read, so each probe must be their only reader.
N4 Fences. A fenced, deck-less frame HOLDS the last canvas and re-publishes it (fence_hold_frames). Black is counted only
   when the fence comes before the first canvas exists (fence_black_frames) (Renderer.cpp:415-430). The fenced frame
   returns before the autopilot (:553) and before the off-screen-deck tick (:773-782). Those are the GL thread's only
   readers of non-active cells, and Autopilot::advanceClip walks every column (Autopilot.cpp:228-251).
   - Writing a cell of ANY deck inside withDeckDetached is therefore safe.
   - Writing a cell outside a fence races the GL thread (Pitfall 55 (2); Deck::setClip can grow the vector,
     MainComponent.cpp:5302-5306).
N5 The drop commit reads the ACTIVE deck (prepareFileDrop / commitDrop, MainComponent.cpp:5241, :5287-5296). The undo
   commands already take a deck index (SetColumnCountCmd and makeSetClipCmd, MainComponent.cpp:846-853, :5137-5143).
N6 Undo / redo of a video cell:
   - Undo: SetClipCmd's dispose hook closes the leaving clip's player if no cell still holds its id
     (MainComponent.cpp:5052-5080).
   - Redo: the media hook reopens it with openVideoForClip on the message thread (:5025-5049).
N7 Self-found gap in the plan's G4 metric:
   - peak_frame_time_ms times renderStart (Renderer.cpp:726) to renderEnd (:980). The plan's normal-path tap runs after
     the old recorder block (:1035-1039), so G4's planned "peak frame ms" could never see the tap.
   - peak_callback_ms times the whole callback (Renderer.cpp:298-310).
   - Adaptive quality disables a legacy-chain effect after 30 frames over 12 ms inside the measured window
     (Renderer.cpp:990-1008, Renderer.h:565-566). A tap placed inside that window could switch an effect off mid-show.
N8 What the GL thread already does:
   - VideoRecorder's GL side notifies its condition variable without the mutex (VideoRecorder.cpp:139). Its encoder waits
     with a 10 ms timeout (:162).
   - The GL thread already takes the stdio lock for its periodic Render Profile (logLinef, Renderer.cpp:1027) and the
     callAsync queue lock (Renderer.cpp:559).
N9 Layer pictures:
   - A decoding (pending) picture re-shows the layer's last saved output and re-touches its owner. With nothing to hold,
     the layer draws nothing that frame (CompositorEngine.cpp:1113-1126).
   - Hidden, bypassed, soloed-out and empty layers save no picture (:1070-1085). Neither does an FX-only clip (:1130-1136).
N10 Other facts used below:
   - Routines drop deck switches, tempo and quantize when saved (RoutineSlice.cpp:30-38; recording.md:114).
   - Every clip path is stored absolute (Clip.cpp:32, :175).
   - A dropped video gets playing = true (MainComponent.cpp:5265-5266). That is the transport flag, not "triggered".
   - BPM Sync speed = videoBeats / beatDivision (Renderer.cpp:1814-1823).
N11 Self-found gap: layer ids restart at 0 on every deck (Pitfall 36). A newly loaded composition can therefore have a
   layer with the recorded layer's id. The plan's "is the layer still in the stack" check would then keep a Layer take
   recording a DIFFERENT layer after a load.
N12 House patterns reused:
   - epoch + deck-id resolution: stagedload::resolveDuplicateSource (StagedLoad.h:184-191), used at
     MainComponent.cpp:3303.
   - A posted landing destroys its unpublished player where it lands if the owner is gone; a hung open is leaked after
     5 s (MediaOpener.h:13-21).
   - TEST_SERVER-only debug routes (ApiServer.cpp:298-330).
   - [tsan] targets are listed in probe-tsan-unit.sh TARGETS and counted in EXPECTED_TSAN_CASES (4 today).
   - Free disk space: juce::File::getBytesFreeOnVolume (juce_File.h:831).
   - VideoPlayer decodes with sws defaults and makes no colourspace call (VideoPlayer.cpp:216).
   - Teardown order: ~MainComponent stops apiServer_ (:2391), stops routines (:2407), detaches the GL (:2420), then calls
     videoRecorder_.stopRecording() (:2442).
   - swapCompositionModel bumps modelEpoch_ and calls routineEngine_.stopAll() (:2988-2997).
N13 Self-found naming clash: the preview panel's 26 px bar already has an "Output" TAB (PreviewPanel.h:68-69). The plan's
   REC switch would put a second, different "Output" button in the same bar.

## 2. RULINGS, ATTACK BY ATTACK

### Seat live-show / real-time (LS)
LS-A1 ACCEPT (fix accepted; premise corrected). The premise that manual BPM and silence lose the lock is wrong:
  - Manual BPM and Link set STATE_LOCKED, and silence keeps it (N1).
  The real gap is a confirming tempo change (LOCKING while bpm > 0, N1). There the plan would start off the bar while the
  bar clock visibly runs. -> AM1 (bar clock = bpm > 0), TC14, rc18 (manual BPM, normal launch), AM14 ("REC now").
LS-A2 ACCEPT. -> AM2 (deck pinned at the press; dropped after a composition swap; falls back to the deck on screen if
  the deck was deleted), RG8, rc4b, rc10, Q6.
LS-A3 PARTIAL.
  - Accepted: measure the landing. rc4 gains fence_black_frames delta 0, fence_hold_frames delta <= 3 and a callback
    peak bar.
  - Rejected: "skip the fence when the row has a free cell". Every cell write must be fenced: the GL autopilot reads
    every cell of a layer (Autopilot.cpp:228-251), and Pitfall 55 (2) fences "a setClip, a column growth, the swap".
  - The premise is also wrong: a fenced frame is a HOLD of the last canvas, not a black frame (N4).
LS-A4 ACCEPT. -> AM12 (x264 threads 2), AM8 (tap witnesses), G4 rebuilt:
  - the 4K row's relative bars are hard;
  - a heavy-fixture row;
  - non-demotable floors;
  - peak_callback_ms replaces peak_frame_time_ms (N7).
  The back-pressure rule already held (plan-bf1.md:256): a full pool drops the take frame and the GL thread never waits.
  AM7 restates it as an invariant.
LS-A5 ACCEPT. -> AM4 (a no-picture frame repeats the last capture; a take that never saw a picture fails, it never lands
  black), TC12, rc22, rc23, rc24. The plan's "opaque black" rule and RT5 "srcFBO 0" are deleted.
LS-A6 ACCEPT. -> AM5:
  - a press while waiting is ignored (the routine pads' rule, CLAUDE.md "Routine pads and bands");
  - a 250 ms debounce;
  - "Next at bar" while Saving with the next take armed;
  - right-click REC > "Cancel recording".
  TC5, rc26.
LS-A7 PARTIAL.
  Accepted:
  - the tempo-change row: TC11, rc25 (bars = counted edges, the name = counted bars);
  - the autopilot row: rc22;
  - undo: rc16 (with SC-A1);
  - the folder-move fact documented: AM11(c).
  Self-found and added: a Layer take ENDS at a composition swap (AM10, N11).
  Rejected:
  - "a routine can fire a deck switch": routines drop deck switches and tempo when saved (N10).
  - "a composition saved mid-take stores a clip pointing to a missing file": no clip exists before the landing, and the
    landing follows the final rename (plan-bf1.md:278-280).
  - "relative paths for takes": every clip path is absolute (Clip.cpp:32, :175). A takes-only exception would split the
    relink story.
LS-A8 PARTIAL.
  - Accepted: the disk floor plus a disk_low end (AM11b, CR7), "REC failed" on the button (AM14), and the take size in
    Q3's text.
  - Rejected: a 16-bar default cap. The cap only catches a forgotten stop, and a 32-bar phrase is a normal sample. Boris
    decides in Q3; the default stays 64.
LS-A9 ACCEPT. -> AM6: two binding actions, Record Output and Record Selected Layer. The on-screen switch never affects a
  pad.
LS-A10 PARTIAL. Accepted:
  - the RT print becomes a hard budget (RT5);
  - in-app tapUs floors (AM8, G4);
  - the grep becomes a ctest lint (RL1, AM7);
  - TSan covers the tap's cross-thread slot protocol (TS2);
  - G6 checks layout at the narrowest window (V12).
  Rejected: the rc3 variant with a 100 ms hitch.
  - The autopilot and the Gate read the same frameSnap_ in the same renderOpenGL call (Renderer.cpp:401-405, :553), so a
    late frame changes no ordering.
  - The Gate's hitch arithmetic is already TC3 (a 100 ms hitch -> 6 repeats).
  - A GL-stall hook would add test surface to the production Renderer and find no new failure.
  - The strengthened rc3 (GT-A2) tests both boundaries.
LS-A11 PARTIAL.
  - Accepted: a cell-wide band carrying the button's text (AM14).
  - Rejected: the TopBar mirror. F10 stands: the TopBar is crowded and bf2 / bf7 also work there. The REC button in the
    preview bar is visible on every deck.
LS-A12 ACCEPT. -> AM7: the GL thread never notifies; the worker polls. The precedent is cited (N8).

### Seat state / concurrency (SC)
SC-A1 PARTIAL.
  Accepted:
  - rc16 (undo / redo live, through the new TEST_SERVER undo / redo routes);
  - rc17 (reload of a probe-written composition; no REST save exists, N3);
  - RG6 (the made clip round-trips through Clip / Composition toVar / fromVar);
  - the player's lifetime written down: undo closes it, redo reopens it (N6, the existing drop path).
  Rejected: clearing lastTake.landed on undo. Status is a record of what happened; /api/composition is where a cell's
  truth lives (AM13). Coupling the recorder to the undo stack buys nothing.
SC-A2 ACCEPT. -> AM2, Q6.
SC-A3 ACCEPT. -> AM2 (the epoch rule), AM3 (a FIFO of landings, the WeakReference rule, a bounded join, beginShutdown),
  rc19, rc20, rc21.
SC-A4 ACCEPT. -> AM9:
  - S2 starts only after bf9b is merged;
  - every pre-bf9b branch is deleted;
  - the key is the shared layer's id;
  - the owner check is rewritten against the merged model;
  - the anchors are re-verified first;
  - a stop rule if bf9b's model differs.
  rc10's bar is fixed now.
SC-A5 ACCEPT. -> AM9: std::optional for the layer key and the fresh picture. CR5; rc9 records the layer whose id is 0.
SC-A6 PARTIAL.
  - Accepted: resolve the layer at the press (AM6, RB2), and Waiting + layer removed -> cancel (TC15).
  - Rejected: a live reorder / remove row. No REST route moves or removes a layer (N3). The identity rule is pure and
    unit-tested.
SC-A7 ACCEPT. -> AM7 (no GL notify, worker poll, lint RL1) and section E (D1: the two mutexes as an explicit decision).
SC-A8 PARTIAL.
  - Rejected: the premise. workerFree turns true at the worker's Landed POST (plan-bf1.md:278-280). A staged load holds
    only the message-thread landing (AM3), so it can never hold the Gate.
  - Accepted: make slips visible. TC13 (startSlips), "Next at bar", status armedNext.
SC-A9 ACCEPT.
  - rc13 now requires the final file to exist and be valid.
  - rc10's bar is pre-registered.
  - Pacing is stated: test mode has no analysis thread, so the clock moves only on injection (N3); the probe drives it
    with a scheduler and pre-flight P0 checks it.
  - The exact counts live in the headless tests (TC4, CR1: exactly 240).
SC-A10 ACCEPT. -> RG7: Video, playing == true exactly as a dropped video (N10), videoBeats == beatDivision (BPM Sync speed
  1). "playing" is the transport flag; Q2's default (not triggered) holds.
SC-A11 ACCEPT. -> AM11(a): sweep stale temps at every arm. CR8, rc8(c).

### Seat gates-tests (GT)
GT-A1 ACCEPT. The arithmetic is right: with downbeats at 0, 2, 4 s, a press at 0.3 s and a stop at 3.1 s give 1 bar.
  CR1 is rewritten (exactly 240 frames for 2 bars, derivation in the test comment), rc2 likewise, mutant M1 added.
GT-A2 ACCEPT. -> rc3 rewritten: blue / red / green by bar, a 1-bar all-red take, both boundaries checked, the arm and the
  triggers ordered before the injected edge. The exact frame count lives in the headless TC4 / CR1 (rc3 is +-2 live).
GT-A3 ACCEPT. -> G6 rewritten:
  - states V1-V12 with exact strings and the kMeterRed pixel test;
  - a frozen injected clock, plus a hold hook for Saving;
  - decoded take frames in the panel;
  - five named critics with pass conditions;
  - at most 3 fix rounds; Harmony owns it.
GT-A4 ACCEPT. -> pure decidePress (TC5) and recordRequestFor (RB2). rc26 drives the real binding dispatch through
  TEST_SERVER /api/debug/binding_action. The seat's example "a press on Waiting cancels" is superseded by LS-A6 / AM5:
  it is ignored.
GT-A5 ACCEPT. -> pre-flights P0-P2. rc1b is re-sequenced (arm acknowledged, then the edge injected) instead of a ms window.
  Quantize comes from the composition file; the hooks are verified (N3).
GT-A6 ACCEPT. -> G4: floors that can never be demoted, the A/A drift rule (a bar becomes INFO only if its margin is
  <= 3 x D after one re-run), and a void-run rule.
GT-A7 ACCEPT.
  - rc9 records layer id 0.
  - rc9b: 50 % opacity plus an Add blend over another layer records the layer's own colour.
  - rc9c: refusal rows.
  - rc24: an empty layer fails "no_picture"; it never lands black (AM4 supersedes "opaque black").
  - rc10's bar is fixed for bf9b.
GT-A8 ACCEPT.
  - rc5 uses a patch / ramp / fine-line fixture with stated bars.
  - CE7 checks the colour tags (AM12).
  - rc12 and every live frame bar count from the injected edges, not wall polling.
GT-A9 ACCEPT. -> AM15: pure frameInFrom, decidePress, resolveLandingDeck and recordRequestFor; mutants M1-M7 with a
  result table.
GT-A10 ACCEPT.
  - rc4b: the deck on screen differs from the pinned deck.
  - Selection independence is structural: the landing functions take no selection argument (RG1, RG8), and no REST route
    selects a cell (N3).
  - RV5: the UI default is Whole (the output).
  - rc16: undo / redo.
  - rc20: landing during a staged load.
GT-A11 ACCEPT. -> G7: baselines from main before each merge, a probe set per stage, pass rules.
GT-A12 ACCEPT. -> G1 exact counts (S1 +40, S2 +8, S3 +5). The anchors are re-verified after bf9b (AM9 step 0).

## ARCHITECT RULING (s-rta-1002b)

### A. AMENDMENTS (each OVERRIDES the plan text it names; everything not named stands)

AM1 Bar clock. Overrides F3's "Not locked (E7's routines rule)", I1 FrameIn.locked, and the wording of TC2 / TC9 / rc14.
  - The bar clock exists iff the frame's snapshot has bpm > 0 (N1). trackerState is never consulted.
  - FrameIn.locked is renamed barClock.
  - Each press decides by that frame's bar clock: with a clock it waits for the bar line, without one it acts on the next
    frame. The button says which (AM14, "REC now").
  - TC9 stays as the defensive case: if an injection drops bpm to 0 while waiting or ending, the take acts on the next
    frame.
  - New: TC14 and rc18.

AM2 The landing deck is pinned at the press. Overrides F6's "deck on screen when the take lands", I6's "active deck",
  I8 RV4, and rc4 / rc10.
  - The press records {modelEpoch_, the id of the deck on screen}. REST and bindings use the deck on screen when the
    message thread handles the press.
  - At landing, RecordTarget::resolveLandingDeck (pure) decides:
    - epoch changed -> DROP: the file is kept, the unpublished player released, lastTake.landed null, landedReason
      "composition_changed";
    - same epoch and the pinned deck id is live -> that deck;
    - same epoch and the deck is gone -> the deck on screen;
    - no deck at all -> DROP "no_deck".
  - The column is chosen at landing: the first empty cell from the left in the top layer. A full row grows one column on
    every layer of THAT deck.
  - The grid mark shows only while the pinned deck is on screen. Status and tooltip name the deck.
  - The commit and the undo entry target the resolved deck index:
    - new MainComponent::commitDropInto(deckIndex, prepared); commitDrop(prepared) delegates with the active index, so
      drops do not change;
    - SetColumnCountCmd / makeSetClipCmd get that index (N5);
    - one withDeckDetached fence makes writing a non-active deck safe (N4);
    - rebuildGrid() only if that deck is on screen.

AM3 Pending landings, owner gone, teardown. Overrides F7's single pendingLanding_, I4 shutdown, I6 teardown.
  - pendingLandings_ is a FIFO (at most 4) of Landed {file, player, ..., epoch, pinnedDeckId}. While staged_ is set it is
    retried on the 30 Hz tick; after that each entry resolves by AM2.
  - Every non-landing exit (a drop, quit, the owner gone) destroys the unpublished player where it lands. The worker
    posts through a WeakReference poster (MediaOpener's rule, N12); the installed clip id is never minted for a dropped
    landing.
  - Teardown, step 1: clipRecorder_.beginShutdown() is the FIRST line of ~MainComponent, before apiServer_->stop()
    (:2391). It is an atomic flag: no landing from then on, and the Gate is asked to end any take (reason Quit).
  - Teardown, step 2: clipRecorder_.shutdown() after the GL detach (:2420), beside videoRecorder_.stopRecording()
    (:2442). It finishes a live take into a valid file, never opens or posts a player, and bounds the worker join at
    5 s, then leaks it (MediaOpener AL2).

AM4 A Layer take never records black. Overrides F5's black rule, I3 "srcFBO 0 = clear opaque black" plus RT5, and I4
  Sources.
  - A frame where the recorded layer has no fresh picture (N9: hidden, bypassed, soloed out, empty, an FX-only clip,
    pending with nothing held) captures NOTHING.
  - Its ticks repeat the last capture (F2). Ticks before the first picture repeat the first one.
  - A take that never had a picture finishes with 0 captures: failed "no_picture", no file, no landing, "REC failed"
    shown.
  - Status counts noPictureTicks.
  - Output takes are unchanged: black at the nothing-to-render and no-content exits is what every output shows
    (Renderer.cpp:518-526, :836-844).

AM5 Press semantics. Overrides the F11 table.
  | phase               | press (button, key, MIDI, REST press)              | Momentary release / REST stop | REST cancel / right-click "Cancel recording" |
  | Idle                | arm: bar clock -> Waiting; none -> start next frame | --                            | --                                           |
  | Waiting             | IGNORED                                            | cancel (nothing written)      | cancel                                       |
  | Recording           | Ending (bar line; no clock: next frame)            | Ending                        | -- (a control never discards a take)         |
  | Ending              | ignored                                            | --                            | --                                           |
  | Saving              | arm the next take ("Next at bar")                  | --                            | --                                           |
  | Saving + next armed | ignored                                            | cancel the next take          | cancel the next take                         |
  - REST "start" = a press from Idle or Saving only; anywhere else it is a no-op with lastError "busy". REST "stop" from
    Idle is a no-op.
  - Debounce: the message-thread funnel drops a press (any surface, incl. REST "press") within 250 ms of the previous
    ACCEPTED press. Releases and REST start / stop / cancel are never debounced.
  - The table is take::decidePress (pure, TakeClock.h). The Gate applies it on the GL thread to that frame's true phase.
    The message thread never decides from a status copy that may be a frame old.

AM6 Two binding actions; the layer is resolved at the press. Overrides I7's single RecordClip and its use of
  recordSource_ for bindings.
  - Binding.h: append RecordOutput and RecordLayer after whatever value is last at merge (TriggerRoutine at plan time,
    Binding.h:46). APPEND ONLY.
  - Labels "Record Output" and "Record Selected Layer" in BindingOverlay.cpp, MidiLearnOverlay.cpp and the
    buildBindableTargets global row.
  - handleBindingAction: a press (value > 0) is a funnel press; a Momentary release is a funnel release.
  - RecordTarget.h recordRequestFor(source, selected index or REST "layer", the deck on screen's layers) is pure. It
    returns {source, std::optional<LayerKey>} or a refusal: "no_layer", or "not_recordable" for FX-only / Mask / ThreeD
    (Layer.h:151-158). It is resolved AT THE PRESS, never later.
  - The on-screen REC records what its [Whole | Layer] switch says (AM14). The switch never affects a binding or REST.

AM7 Thread model. Overrides plan section 3 "Thread model" and R18's grep.
  - The GL tap path is RecordTapFrame.cpp (capture, poll), TapSlots.h and TakeClock.h. It never locks, waits, notifies,
    allocates, builds a string or logs per frame. One-off GL-side event logs (a failure, once) use logLinef (N8).
  - Arm-time GL setup (ensure: pool generation, textures, FBOs) and releaseGL live in RecordTap.cpp, not per frame.
  - Lint RL1 (tests/test_record_tap_lint.cpp, a comment-stripped token scan in the style of test_log_line_lint) fails if
    RecordTapFrame.cpp, TapSlots.h or TakeClock.h contain: mutex, lock_guard, unique_lock, scoped_lock,
    condition_variable, notify_one, notify_all, new, malloc, push_back, emplace_back, std::string, logLine(, callAsync,
    sleep_for, wait(.
  - Worker: one persistent thread (QOS_CLASS_UTILITY).
    - From the press until its Landed post it wakes every 2 ms (condition_variable::wait_for(2 ms)) and drains the slot
      queue.
    - Otherwise it blocks on the condition variable, which only the message thread notifies (commands, shutdown).
    - Record to Clip does NOT copy VideoRecorder's GL-side notify (VideoRecorder.cpp:139).
  - The two mutexes (Sacred rule 2) are decision D1:
    - M1, the worker's condition-variable mutex: worker and message thread only.
    - M2, the status copy: written by the worker (its results) and by the message thread's 30 Hz tick (composing GL-side
      relaxed atomics); read by httplib.
    - Neither is ever taken on the GL, audio or analysis thread.
  - Back-pressure invariant: the GL thread never waits for a take. A frame with no free slot is dropped from the take
    (droppedNoSlot) and its tick filled by repeats. The take loses frames before the show does.

AM8 Tap placement and cost witnesses. Overrides I3's INFO print and I5's placement note; the reason is made binding.
  - The normal-path tap stays after the old recorder block (Renderer.cpp:1035-1039), outside renderStart (:726) to
    renderEnd (:980). Its cost must never count toward Adaptive quality's effect disabling (N7).
  - The fenced (:415-428), nothing-to-render (:518-526) and no-content (:836-844) exits call the shim before their
    return.
  - tapClipRecorder times itself on every call (two steady_clock reads, relaxed atomics): status tapUsMean (per take)
    and tapUsMax.
  - RT5 (unit, private context, 1920x1080, 120 captures): capture() + poll() wall-time median <= 0.5 ms, p95 <= 2 ms.
  - G4's floors use tapUs measured in the real context.

AM9 S2 waits for bf9b; layer identity. Overrides I4's LayerKey, I5's freshLayerOutput, F9's pre-bf9b branch, rc10 and
  R12.
  - S2 code starts only on a main that contains the MERGED bf9b.
  - S2 step 0: re-read on that main, and record new line numbers in the lane report, for:
    - renderOpenGL's four exits and the frameSnap_ read;
    - the autopilot call and the old recorder block;
    - saveLayerOutput's owner;
    - the shared stack's layer identity;
    - DeckView's selection API;
    - swapCompositionModel and the ~MainComponent order.
  - Stop rule: if merged bf9b does not give ONE shared playing stack whose layer ids stay stable for the life of a
    composition, S2 stops and Harmony re-dispatches the Architect for an S2 delta.
  - LayerKey = { uint32_t layerId }, a struct in RecordTarget.h (S1). std::optional<LayerKey> is used wherever a layer
    may be absent. No 0 or -1 sentinels: layer id 0 is a real layer (Pitfall 15). The Gate never inspects the key.
  - CompositorEngine.h: std::optional<FreshOutput> freshLayerOutput(LayerKey) const. It returns the layer's saved or held
    picture iff the owner was written in the current composite pass (owner.frameSerial == frameSerial_). The owner's deck
    part is compared only if merged bf9b keeps a per-deck owner.
  - Deleted: the "deck left the screen" end, rc10's pre-bf9b expectation and R12's dual-model test. A Layer take ends
    SourceGone only when its layer leaves the rendered stack (removed), on a non-fenced frame, or at a swap (AM10).

AM10 Changes during a take. Overrides F9.
  - Composition swap (New or Load; N12). Beside routineEngine_.stopAll() in swapCompositionModel, call
    clipRecorder_.onCompositionSwapped():
    - Waiting -> cancelled ("composition_changed");
    - a live Layer take ends on the next frame (cut "composition_changed", file kept);
    - an Output take records on.
    Every landing pressed before the swap is dropped by AM2. Reason: layer ids repeat across compositions (N11).
  - Waiting with the Layer source's layer removed -> cancel ("layer_removed", nothing written). TC15.
  - Tempo change (Link, manual, the tracker): bars = bar edges the Gate counted; ticks = wall time; the name uses the
    counted bars. TC11, rc25.
  - The autopilot or a routine changing the recorded layer's clip is recorded, never cut: routines cannot switch decks or
    tempo (N10) and the autopilot never removes a layer. rc22.
  - Deck switch and deck deletion: both kinds of take record on; the landing follows AM2. rc10, rc4b.
  - The cap: 64 counted bar edges with a bar clock, 300 s without one (Q3).

AM11 Files. Adds to F8; F8's folder and name rules otherwise stand.
  - (a) At every arm, before the temp file is opened, delete ".rec-*.mp4" files in the take folder modified before this
    process started (crash leftovers; a live temp is always newer). CR8, rc8(c).
  - (b) Refuse an arm when the take folder's volume has < 2 GiB free: lastError "disk_full", "REC failed" shown.
    During a take the worker checks every 2 s; below 512 MiB it ends the take on the next frame (cut "disk_low";
    finished, kept, landed). A free-bytes seam (std::function, default getBytesFreeOnVolume) lets tests drive this. CR7.
  - (c) Paths stay absolute like every clip's (N10). recording.md and Boris's page say that a moved show folder needs its
    media relinked, takes included.

AM12 Encoder. Overrides F4's "threads 4"; adds tags.
  - x264 threads 2 at every size.
  - The stream is tagged color_range MPEG ("tv") and colorspace SMPTE170M (BT.601), which is what sws's defaults write
    and VideoPlayer's defaults read (N12), so outside players agree. CE7.
  - Pre-registered fallbacks, decided only by G4:
    - a failing 1080p relative CPU-driven bar -> ProRes 422 Proxy via VideoToolbox (F4's fallback), re-run G4 + G5,
      record the codec decision;
    - R3 (4K) failing only its repeats floor while its relative bars pass -> x264 threads 4 above 2560x1440 only, re-run
      R3.

AM13 Status and workerFree. Overrides I4's status list. The workerFree definition is restated, not changed.
  - workerFree turns true when the worker has finished and renamed the file, opened the unpublished player and posted
    Landed (plan-bf1.md:278-280). It never waits for the message-thread landing.
  - Status gains: barClock; targetDeck {id, name}; layer {id, name} (no index); startBeat, startBeatInBar,
    startDelayFrames; pendingLandings; noPictureTicks; startSlips; armedNext; tapUsMean, tapUsMax; lastError.
  - lastTake gains landed {deck, layer, column} | null, landedReason ("landed" | "composition_changed" | "no_deck" |
    "quit") and failedAtMs.
  - lastTake.landed is history; undo does not change it. A cell's truth is /api/composition.

AM14 On screen. Overrides I8's text and states.
  - The source switch reads [Whole | Layer], not "Output", because the bar already has an "Output" tab (N13). Default
    Whole at launch. API names stay "output" / "layer".
  - The REC label table (pre-registered for RV1 and G6; red = kMeterRed, LookAndFeel.h:31):
    - Idle with a bar clock: "REC", no red.
    - Idle without one: "REC now", no red; tooltip "No beat lock: recording starts and ends the moment you press".
    - Waiting: "Starts at bar", red outline.
    - Recording: "REC <bar>.<beat>", counted from the take's start (first beat = "REC 1.1"), solid red. Without a bar
      clock: "REC <s> s".
    - Ending: "Ends at bar", solid red.
    - Saving: "Saving", grey, no red.
    - Saving with the next take armed: "Next at bar", red outline.
    - After a failed take: "REC failed" (red text, no fill) for 3 s from lastTake.failedAtMs; tooltip = the reason in
      plain words.
    - Layer with nothing selected: disabled; tooltip "Select a layer (click its name) to record it".
    - A selected FX / Mask / 3D layer: disabled; tooltip "This layer has no picture of its own".
  - Right-click REC: "Cancel recording", enabled only in Waiting and in Saving with the next take armed.
  - The grid mark appears only while the PINNED deck is on screen. It is a cell-wide band on the target cell with the
    button's text (Armed "Starts at bar", Live "REC 2.3", Saving "SAVING"); G6's critics size it. No TopBar indicator
    (F10 stands).
  - Idle tooltip: "Record into a new clip: starts and ends on the next bar line, lands in the top layer's first empty
    cell of this deck".

AM15 Pure seams and mutants. Overrides the RED notes of I1 / I5 / I6 / I7.
  - Pure seams; Renderer::tapClipRecorder and MainComponent only gather inputs and forward:
    - take::frameInFrom (TakeClock.h) builds FrameIn from the snapshot's bpm / totalBeatCount / beatInBar, the Gate's
      own OnsetPulse baseline, the exit kind (Normal, Fenced, NothingToRender, NoContent), the source, layerInStack and
      the optional fresh picture. TC14.
    - take::decidePress. TC5.
    - RecordTarget::resolveLandingDeck. RG8.
    - RecordTarget::recordRequestFor: header in S1 with its own source enum (no Binding.h); tested in S2 by RB2.
  - Mutants: temporary patches, never committed. Each must be killed by the named case; the lane report gets a result
    table; a surviving mutant fails G1-MUT.
    - M1 start at the press (no edge wait) -> TC1, CR1.
    - M2 the end-edge frame included -> TC4, CR1 (exactly 240).
    - M3 one encoded frame per render frame -> TC3.
    - M4 bar clock = trackerState == LOCKED -> TC14.
    - M5 land on the deck on screen -> RG8.
    - M6 a press while Waiting cancels -> TC5.
    - M7 a no-picture frame captured as black -> TC12.

AM16 Test hooks. TEST_SERVER-only, inside ApiServer.cpp's #if AUDIODNA_TEST_SERVER block (N12), absent from a non-test
  build:
  - POST /api/debug/binding_action {"action": "record_output" | "record_layer", "edge": "press" | "release", "mode":
    "toggle" | "momentary"} -> handleBindingAction with a synthetic Binding, on the message thread. S2.
  - POST /api/debug/record_hold_finish {"hold": true | false} -> ClipRecorder's finish gate for tests: while held, the
    worker waits before finishing. S2.
  - POST /api/debug/undo and POST /api/debug/redo -> undoManager_ undo / redo plus refreshAfterUndoRedo, on the message
    thread. S2.
  - GET /api/debug/record_view -> the REC control's last painted {text, style, enabled, tooltip} and the grid mark
    {deck, layer, column, mark}. S3.
  - No GL-stall hook (LS-A10's hitch row is rejected).

AM17 Docs. Section 6 stands, with these changes:
  - recording.md "Record to Clip": AM1-AM14 as rules; the press table is AM5's; the landing deck is AM2's.
  - pitfalls.md Pitfall NN (Harmony numbers it; next free is 64): "A recorded frame is picked by the wall-clock tick,
    never one per render frame; a take's bars come from the frame's own snapshot -- the totalBeatCount delta with
    beatInBar 0 in the recorder's own baseline, and the bar clock is bpm > 0, never trackerState; the end-edge frame is
    not in the take; the GL side of the tap never locks, waits, notifies, allocates or logs per frame (lint RL1) -- the
    worker polls; a Layer take never records black; a take lands in the deck where REC was pressed and is dropped after
    a composition swap".
  - Pitfall 56 is amended as planned.
  - APP-INVENTORY: binding actions 21 -> 23; production REST 41 -> 43 (the 5 TEST_SERVER routes are not counted).
  - Proposed BORIS_DECISIONS text: "...it lands, ready to trigger, in the first empty cell of the top layer of the deck
    where REC was pressed...", plus Q6's answer when it comes.

### B. FINAL BUILD STAGES (one builder context each)

S1 -- headless core. BUILDABLE NOW: new files only, and no lane shares them.
- NEW:
  - src/recording/TakeClock.h: Gate, decidePress, frameInFrom, tick math.
  - src/recording/TapSlots.h: the Free / Fenced / Queued slot protocol plus the SPSC queue, no GL.
  - src/recording/ClipEncoder.{h,cpp}.
  - src/recording/ClipRecorder.{h,cpp}: FrameSink seam, the polling worker, WeakReference poster, finish gate for tests,
    free-bytes seam, beginShutdown / shutdown, onCompositionSwapped.
  - src/recording/RecordTarget.h: LayerKey, topLayerIndex, firstEmptyColumn, layerRecordable, takeFolder, takeName,
    staleTemps, makeClip, resolveLandingDeck, recordRequestFor.
- Tests:
  - test_take_clock: TC1-TC15.
  - test_clip_encoder: CE1-CE7.
  - test_clip_recorder: CR1-CR3, CR5-CR8.
  - test_clip_recorder_race [tsan]: CR4.
  - test_record_target: RG1-RG8.
  - test_tap_slots: TS1.
  - test_tap_slots_race [tsan]: TS2.
- Edits:
  - CMakeLists.txt: sources.
  - tests/CMakeLists.txt: 7 targets, 2 of them LABELS tsan.
  - tests/test_log_line_lint.cpp: ClipEncoder.cpp and ClipRecorder.cpp added to its list.
  - .harmony/probe-tsan-unit.sh: TARGETS += test_clip_recorder_race test_tap_slots_race; EXPECTED_TSAN_CASES += 2.
- Gates: G1 (S1), G1-MUT (M1-M4, M5 via RG8, M6, M7), G2. Ships alone: yes; it is inert, nothing calls it.

S2 -- GL tap, wiring, controls, probes. BLOCKED until lane bf9b is merged. AM9 step 0 comes first.
- NEW:
  - src/render/RecordTap.{h,cpp}: arm-time GL.
  - src/render/RecordTapFrame.cpp: per frame, capture and poll.
- Edits:
  - src/render/Renderer.{h,cpp}: setClipRecorder and the member; the shim at the 4 exits; GL context created / closing.
  - src/render/CompositorEngine.h: freshLayerOutput.
  - src/MainComponent.{h,cpp}:
    - the member and ctor wiring;
    - recordClipPress (funnel + debounce);
    - landRecordedTake (pinned deck, FIFO, epoch);
    - commitDropInto, with commitDrop delegating to it;
    - onCompositionSwapped beside routineEngine_.stopAll() (:2997);
    - beginShutdown first in ~MainComponent and shutdown after the GL detach;
    - buildBindableTargets; 2 handleBindingAction cases;
    - timerCallback: compose the status, retry landings;
    - the 4 S2 debug-route targets.
  - src/binding/Binding.h: append 2.
  - src/ui/BindingOverlay.cpp, src/ui/MidiLearnOverlay.cpp: labels.
  - src/api/ApiServer.{h,cpp}: 2 production routes and 4 debug routes.
  - .harmony/probe-record-clip.sh + .py (G3); .harmony/probe-record-clip-ab.py (G4); scene u13 in probe-vupload.py (G5).
- Tests:
  - test_record_tap_gl: RT1-RT5.
  - test_record_tap_lint: RL1.
  - test_record_clip_binding: RB1-RB2.
- Gates: G1 (S2), G1-MUT re-run, G2 re-run, G3, G4, G5, G7 (the S2 set).
- Ships alone: yes (REST / key / MIDI, no button). Harmony may hold it for S3.

S3 -- UI and docs. After S2 and lane ui (ClipCell).
- NEW: src/ui/RecordClipView.h, src/ui/RecordClipControl.{h,cpp}.
- Edits:
  - src/ui/PreviewPanel.{h,cpp}: the [Whole | Layer] switch and REC at the right end of the tab bar.
  - src/ui/ClipCell.{h,cpp}: the cell-wide band.
  - src/ui/DeckView.{h,cpp}: the mark, for the pinned deck only.
  - MainComponent timerCallback: push the views.
  - GET /api/debug/record_view.
  - Docs: section 6 plus AM17.
- Tests: test_record_clip_view, RV1-RV5.
- Gates: G1 (S3), G6, G7 (the S3 set).

Shared-files table: the plan's stands, with these additions:
- MainComponent.cpp: swapCompositionModel, one line (bf9b and asyncload owners); the top of ~MainComponent, one line.
- ApiServer.cpp: the TEST_SERVER block, 4 + 1 routes (any lane adding debug routes).
- .harmony/probe-tsan-unit.sh: TARGETS / EXPECTED_TSAN_CASES (lane tsan-r5).
- PreviewPanel.h: switch labels (no other lane known).

### C. FINAL GATES (pre-registered; Harmony copies gate strings ONLY from this list)

G1 UNIT:
- Release build, serial ctest. Every case passes.
- New TEST_CASEs, exactly:
  - S1 +40: test_take_clock TC1-TC15, test_clip_encoder CE1-CE7, test_clip_recorder CR1-CR3 + CR5-CR8,
    test_clip_recorder_race CR4, test_record_target RG1-RG8, test_tap_slots TS1, test_tap_slots_race TS2.
  - S2 +8: test_record_tap_gl RT1-RT5, test_record_tap_lint RL1, test_record_clip_binding RB1-RB2.
  - S3 +5: test_record_clip_view RV1-RV5.
- ctest total == base + those counts. Any deviation is explained in the lane report and accepted by Harmony before the
  merge.
- S1's cases also pass in the ASan / UBSan build.
- test_record_tap_gl SKIPPING (no GL pixel format) on Boris's Mac = FAIL.
- Case contents (each case is one TEST_CASE; sections allowed):
  - TC1: a press mid-bar starts exactly on the next bar-edge frame; tick 0 is captured there.
  - TC2: bpm 0 -> the take starts on the next frame and a stop ends it on the next frame.
  - TC3: 60 fps from 120 Hz +-1 ms (every tick once, no repeat); 60 Hz +-3 ms (no skip / repeat pair); 50 Hz
    (10 repeats/s); a 100 ms hitch (6 repeats).
  - TC4: downbeats at 0/2/4/6 s at 120 BPM. A press at 0.3 s and a stop at 5.1 s -> start on the 2.0 s edge, end on the
    6.0 s edge (excluded), totalTicks == 240 exactly.
  - TC5: AM5's table, every cell, plus the debounce (< 250 ms ignored, >= 250 ms accepted).
  - TC6: fenced frames count edges, capture nothing, are filled by repeats, never SourceGone.
  - TC7: SourceGone ends a Layer take on the first non-fenced frame its layer is absent.
  - TC8: the cap at the 64th counted edge; 300 s with no bar clock.
  - TC9: bpm -> 0 while Waiting or Ending acts on the next frame.
  - TC10: a start on a fenced frame -> earlier ticks repeat the first capture.
  - TC11: the beat interval goes 0.5 s -> 0.4 s inside bar 2 -> bars == 2 and totalTicks == round(span x 60) exactly.
  - TC12: AM4. No-picture frames capture nothing and repeat the last capture; backfill before the first picture; zero
    pictures -> failed "no_picture".
  - TC13: Saving with the next take armed and workerFree false across 3 edges -> startSlips == 3; the take starts on
    the 4th edge.
  - TC14: frameInFrom. LOCKING + bpm 120 -> barClock true; SEARCHING + bpm 0 -> false; an edge = own-baseline
    delta > 0 with beatInBar 0; a fenced exit -> captureAllowed false and sourceGone false; a Layer source with no
    picture -> captureAllowed false.
  - TC15: Waiting + the layer absent -> cancelled "layer_removed", nothing written.
  - CE1-CE6: as the plan's I2.
  - CE7: the produced file reads back color_range tv and color_space smpte170m.
  - CR1: TC4's clock -> one Landed: ok, frames == 240, bars 2, a player opened at the take size, a valid thumbnail,
    name "Output 2 bars <HHMMSS>.mp4". The derivation is in the test comment.
  - CR2: a cancel while Waiting -> no file, no landing, no temp.
  - CR3: shutdown mid-take -> a valid file (decodes, frames > 0), no landing, join <= 5 s.
  - CR4 [tsan]: a GL-driver thread, a press / stop / status thread and the worker -> no report.
  - CR5: Landed carries the press epoch, the pinned deck id and an optional LayerKey; layer id 0 round-trips as a real
    id.
  - CR6: the poster delivers after the owner is gone -> the player is destroyed where it lands, ASan clean.
  - CR7: free bytes < 2 GiB at arm -> refused "disk_full", nothing created; < 512 MiB mid-take -> cut "disk_low",
    finished and landed.
  - CR8: a ".rec-x.mp4" older than the process start is removed at the next arm; a newer one is untouched.
  - RG1-RG5: as the plan's I6.
  - RG6: makeClip -> Clip::toVar / fromVar field-equal; in a Composition round trip the clip keeps its cell.
  - RG7: makeClip gives Video, playing == true (as a dropped video), videoBeats == beatDivision, loop Loop, beat snap Off,
    the take's size and thumbnail.
  - RG8: resolveLandingDeck. Same epoch + live id -> that index; same epoch + id gone -> the active index; a changed
    epoch -> Drop "composition_changed"; no decks -> Drop "no_deck".
  - TS1: 4 slots; claim / fence / signal / pop / release; the fifth claim refused and counted; a dropped fence frees its
    slot and is counted.
  - TS2 [tsan]: a 120 Hz claimer, a worker popping and releasing, and a status reader -> no report, every claimed slot
    returned.
  - RT1-RT4: as the plan's I3 RT1-RT4.
  - RT5: AM8's timing bar.
  - RL1: AM7's lint.
  - RB1: RecordOutput and RecordLayer are appended right after the previous last value; TriggerRoutine's value is
    unchanged; a BindingManager save / load round trip of a Momentary RecordLayer binding.
  - RB2: recordRequestFor. Nothing selected -> "no_layer"; an FX-only / Mask / ThreeD selection -> "not_recordable";
    index i -> layers[i].id resolved at the press.
  - RV1: AM14's label table, every string (incl. "REC now", "Next at bar", "REC failed" for 3 s then "REC").
  - RV2: refused Layer states and their tooltips.
  - RV3: paintKey changes at most once per beat while recording and never while idle.
  - RV4: the mark is (the pinned deck's top layer, first empty column) only while the pinned deck is on screen; none
    when idle.
  - RV5: the launch default is Whole.
G1-MUT: in the S1 and S2 lane reports, mutants M1-M7 (AM15) are each killed by their named case. Any survivor = FAIL.
G2 TSAN: .harmony/probe-tsan-unit.sh, with TARGETS including test_clip_recorder_race and test_tap_slots_race and
EXPECTED_TSAN_CASES = base + 2. Bar: exit 0 and zero "WARNING: ThreadSanitizer".
G3 LIVE (S2): .harmony/probe-record-clip.sh. Setup for every row:
- TEST_SERVER Release build, launched with open -g ... --args --test-mode (except rc18).
- HTTP clients send Connection: close. One app at a time (wf/lock.sh). Never an Output window. A ps check first.
- The 7070 /api/state fields are read by this probe alone.
- The probe drives the beat clock with a scheduled injector: beat n at T0 + n x 0.5 s, totalBeatCount n, beatInBar
  n mod 4, bpm 120, trackerState 2, unless a row says otherwise.
- tInj(k) = the probe's timestamp when the injection of downbeat k returned.
- A row whose action ack arrives after its next scheduled beat is re-run once (rig), not failed.
Pre-flights:
- P0: over 8 injected beats, status.beatInBar follows each injection within 100 ms and status.barClock == true. Else
  the run is VOID.
- P1: POST /api/record_clip, GET /api/record_clip/status, 8080 /api/debug/gl_context_cycle, /api/debug/binding_action,
  /api/debug/record_hold_finish, /api/debug/undo and /api/debug/redo all answer 200.
- P2: the row's composition file sets quantizeMode Bar; a trigger_clip sent mid-bar is not active in /api/composition
  until the next injected downbeat.
Rows:
- rc1: a press after beat 1 of a bar -> status.phase "waiting" within 250 ms; status.startBeat == the totalBeatCount of
  the next injected downbeat; startBeatInBar == 0.
- rc1b: press; poll until "waiting"; inject the downbeat at once -> startBeat == that downbeat. startRepeats <= 2 on the
  process's SECOND take (INFO on the first).
- rc2: press after beat 1 of bar A; stop after beat 1 of bar A+2. Bars:
  - lastTake.bars == 2;
  - frames == round((tInj(A+3) - tInj(A+1)) x 60) +-2;
  - ffprobe: h264, every packet key, r_frame_rate 60/1, even width and height, color_range tv;
  - name "Output 2 bars <HHMMSS>.mp4".
- rc3: alignment. Composition: L0 C0 a blue still playing, L0 C1 red still, L0 C2 green still (each triggered once at
  setup and confirmed by render_frame), L1 (top) empty, layer transition cut. Sequence:
  - in bar N-1, after beat 1: trigger_clip L0 C1 (queued) and REC start; wait for "waiting" + 100 ms; inject bar N's
    downbeat;
  - in bar N, after beat 1: trigger_clip L0 C2 (queued) and REC stop; inject bar N+1's downbeat.
  Bars: a 1-bar take; decoded frame 0 red (R >= 200, G <= 40, B <= 40); the last decoded frame red; no decoded frame
  with B >= 200 or G >= 200; frames == round((tInj(N+1) - tInj(N)) x 60) +-2.
- rc4: landing. After rc2's end edge, /api/composition shows a Video clip at (pinned deck = the deck on screen at the
  press, top layer, first empty column) with mediaFile == lastTake.file, within 1.5 s of the end-edge injection. Across
  the 2 s window holding the landing (polled every 100 ms, heartbeat on):
  - fence_black_frames delta == 0;
  - fence_hold_frames delta <= 3;
  - peak_message_stall_ms <= 50;
  - max peak_callback_ms <= (max over the preceding 2 s) + 8.
- rc4b: press on deck 0; switch_deck to deck 1 before the end edge -> the take lands in deck 0's top layer, first empty
  column; lastTake.landed.deck == deck 0's id; deck 1's cells are unchanged (/api/composition before vs after).
- rc5: picture truth. Fixture: a 1920x1080 still with eight flat patches (R, G, B, C, M, Y, white, 18 % grey), a 0-255
  horizontal grey ramp, a 0-32 dark ramp and a field of 1-px alternating red / blue vertical lines. Take 1 bar of it ->
  trigger the landed clip -> render_frame (complete) vs render_frame of the still at the same canvas:
  - each patch's mean abs diff <= 6/255 per channel (8 px inset);
  - grey ramp max abs diff <= 8/255 (2 px border excluded);
  - dark ramp mean abs diff <= 4/255;
  - line field: luma PSNR INFO; the crop goes to G6.
- rc6: trigger the landed clip -> its playhead advances 1.0 s of content per 1.0 s wall +-3 % over 4 s.
- rc7: a full top row -> the take lands in column old numColumns, and numColumns + 1 on every layer of the pinned deck.
  /api/debug/undo -> numColumns restored and the cell gone (one undo entry).
- rc8:
  - (a) press -> "waiting"; press again 300 ms later -> still "waiting" (ignored).
  - (b) REST cancel -> "idle" within 250 ms; no landing; no new file and no ".rec-*" in the take folder.
  - (c) a ".rec-old.mp4" with mtime before the launch is gone after the next arm; one with mtime after the launch is
    untouched.
- rc9: L0 has id 0 (asserted from /api/composition) and is a green still; L1 (top) is a red still. REC source layer,
  layer 0 -> status.layer.id == 0; decoded frame 0 green (G >= 200, R <= 40, B <= 40).
- rc9b: L0 red still; L1 green still at opacity 0.5 with blend Add (written in the file); L2 (top) empty. A Layer take
  of L1 -> every decoded frame green (G >= 200, R <= 40). Control: an Output take of the same composition has R >= 100.
- rc9c: a Layer press on an FX-only layer and on a Mask layer -> lastError "not_recordable", phase idle, nothing
  written. binding_action record_layer on a fresh launch (nothing selected) -> refused "no_layer".
- rc10 (bf9b model): one Output take and one Layer take, each with a switch_deck mid-take -> cutReason "stop" (no cut);
  frames == round(edge span x 60) +-2; both land in the PINNED deck; the deck on screen at landing gains nothing.
- rc11: a GL context cycle mid-take -> cutReason "context_lost"; the file is ffprobe-valid; it lands; /api/health
  answers 200 for 5 s after.
- rc12: canvas 1280x720, set_composition_params 1920x1080 mid-take -> the file is 1280x720 and frames ==
  round((tInj(end) - tInj(start)) x 60) +-2.
- rc13 (own launch): quit (the async-load probe's quit method) >= 1 s into Recording -> exactly one new final file in
  the take folder, ffprobe-valid, duration >= 1.0 s; no ".rec-*"; no new .ips crash report; exit <= 10 s; the
  composition file on disk unchanged.
- rc14: inject bpm 0 and trackerState 0 -> barClock false; a press -> recording with startDelayFrames <= 2; a stop ->
  it ends within 2 frames; name "Output <S.s> s <HHMMSS>.mp4".
- rc15 INFO: Output > Start Recording still records independently.
- rc16: land a take.
  - /api/debug/undo -> the cell is empty, numColumns as before the landing, the video_players count down 1, the file
    still on disk.
  - /api/debug/redo -> the clip is back at the same cell; triggered, render_frame is complete with mean abs diff
    <= 4/255 vs the pre-undo capture.
- rc17: the probe writes a composition file whose L1 C0 is the take (mediaType Video, mediaFile = lastTake.file) ->
  load_composition -> trigger -> render_frame complete, mean abs diff <= 4/255 vs the pre-reload capture.
- rc18 (NORMAL launch, TEST_SERVER build, no --test-mode): click track and transport as probe-manual-bpm.sh; POST
  /api/set_bpm 120. Then:
  - barClock true;
  - a press after beatInBar 1 is seen (/api/bpm) -> phase "waiting" (not "recording") within 250 ms;
  - the take starts with startBeatInBar == 0;
  - a stop after beatInBar 1 two bars later -> bars == 2 and frames == 240 +-7 (2.00 s per bar +-3 %).
- rc19: load during Recording.
  - (a) Output take; load_composition of another file mid-take -> it records on until the stop; landed null with
    landedReason "composition_changed"; the file exists; the new composition holds no clip with that mediaFile;
    video_players unchanged by the drop.
  - (b) Layer take; a load mid-take -> cutReason "composition_changed"; landed null; the file valid.
- rc20: landing during a staged load. Hold the finish (record_hold_finish true); stop; inject the end edge; start a
  slow staged load (the async-load probe's 4K fixture); release the hold -> pendingLandings == 1 while staged.
  - (a) /api/debug/cancel_load -> it lands in the pinned deck within 1 s.
  - (b) Repeat and let the load swap -> landed null, "composition_changed"; video_players returns to the post-load
    count (no leaked player).
- rc21 (own launch): stop, then quit within 100 ms of the end edge (Saving) -> exactly one final file, ffprobe-valid;
  no ".rec-*"; no new .ips; exit <= 10 s.
- rc22: L0 is recorded, with autopilot on (4 beats per clip), C0 a blue still and C1 a red still, both decoded at
  setup. A 2-bar Layer take of L0 -> cutReason "stop"; noPictureTicks == 0; both colours appear (some frame B >= 200,
  some frame R >= 200); every decoded frame's mean luma >= 16/255.
- rc23: L0 is empty at the start; 1 beat into a Layer take, trigger a bright video (mean luma >= 100) on L0 -> every
  decoded frame's mean luma >= 16/255; noPictureTicks > 0 (INFO).
- rc24: a 1-bar Layer take of an empty layer -> lastTake.ok false, error "no_picture"; no new file; no landing.
- rc25: the injected interval is 0.5 s in the take's bar 1 and 0.4 s in bar 2 -> bars == 2; frames ==
  round((tInj(end) - tInj(start)) x 60) +-2; name "Output 2 bars ...".
- rc26: through /api/debug/binding_action.
  - (a) record_output toggle press -> "waiting".
  - (b) record_output momentary press, then release before the edge -> "idle", no file.
  - (c) momentary press; inject the edge; release after beat 1 of the next bar -> a 1-bar take ending on the edge after
    that.
  - (d) record_layer on a fresh launch -> refused "no_layer".
  - (e) from Idle, two toggle presses 100 ms apart -> "waiting" (the second press is debounced); another press 300 ms
    later -> still "waiting" (Waiting ignores presses).
G4 PERF (S2): .harmony/probe-record-clip-ab.py.
- Launch: NORMAL launch of the TEST_SERVER Release build, heartbeat on, manual BPM 120 via /api/set_bpm, never an
  Output window, a ps check first.
- At least 5 launches per arm, interleaved A B A B. Per launch: a 5 s settle, then a 20 s window. Arm A = no take; arm
  B = an Output take spanning the window.
- Metrics per launch:
  - render fps median (fps polled at 4 Hz);
  - max peak_callback_ms;
  - max peak_frame_time_ms;
  - video_late_frames delta;
  - max peak_message_stall_ms;
  - arm B only: droppedNoSlot, repeats, startRepeats, ticks, tapUsMean, tapUsMax.
  Bars apply to medians across launches.
- Rows:
  - R1: 1080p canvas, fixture F1 = 2 x 1080p30 H.264 loops + 1 procedural source + 2 effects.
  - R2: 1080p canvas, F2 = 4 x 1080p30 H.264 loops + 2 procedural sources + 4 effects.
  - R3: 3840x2160 canvas, F1, a 30 fps take.
- Relative bars (every row):
  - fps B >= A - 3;
  - peak_callback_ms B <= A + 2.0;
  - peak_frame_time_ms B <= A + 1.0;
  - video_late_frames B - A <= 5 per 20 s;
  - peak_message_stall_ms B <= A + 10.
- A/A (R1 and R3, run FIRST, at least 5 launches per arm):
  - D = |median(A1) - median(A2)| per metric.
  - A bar whose margin is <= 3 x D re-runs that row's A/A once with 8 launches per arm. Still <= 3 x D -> that one bar
    is INFO.
  - R2 uses R1's D.
  - A launch whose fps median is < 50 % of its arm's median is a rig fault: re-run it, at most 2 per arm, else the run
    is VOID.
- Floors (never demoted, every row):
  - droppedNoSlot == 0;
  - repeats - startRepeats <= 1 % of ticks;
  - tapUsMean <= 300 and tapUsMax <= 2000 (R3: <= 600 and <= 4000);
  - fps B >= 60 (R3: fps B >= min(60, A - 3)).
- INFO: process CPU B - A.
- Decisions: AM12's two fallbacks, nothing else.
G5 PLAYBACK (S2): probe-vupload.py scene u13, interleaved runner, at least 5 per arm.
- Forward uploads/s >= 58.
- Reverse and ping-pong >= 55 with late <= 10 per 5 s, WITH mkvidx item 3 merged. Without it: INFO, expected per E16.
- video_reverse_nonmonotonic == 0.
- CPU per playing take: INFO.
G6 VISUAL (S3, before Boris sees anything):
- Captures: decoded window captures by the main window's Quartz window id only (never full screen, never an Output
  window). The app is never brought to the front; no synthetic input.
- Driving: REST and the injector. The clock is frozen by not injecting; Saving is held with record_hold_finish.
- Each state asserts GET /api/debug/record_view's text / style / enabled, and the kMeterRed pixel count inside the REC
  button rect ("red" = >= 40 px within +-8 per channel of kMeterRed):
  - V1 Idle, Whole: "REC", 0 red.
  - V2 Idle, bpm 0: "REC now", 0 red.
  - V3 Idle, Layer, nothing selected: disabled; tooltip "Select a layer (click its name) to record it".
  - V4 Waiting: "Starts at bar", red; the band "Starts at bar" on the target cell.
  - V5 Recording, held at bar 2 beat 3: "REC 2.3", red; the band "REC 2.3".
  - V6 Ending: "Ends at bar", red.
  - V7 Saving (held): "Saving", 0 red; the band "SAVING".
  - V8 Saving with the next take armed: "Next at bar", red.
  - V9 After rc24's failure: "REC failed" at +1 s; "REC" again by +4 s.
  - V10 The landed cell: thumbnail and name.
  - V11 Right-click menu while Waiting: "Cancel recording" enabled.
  - V12 The [Whole | Layer] switch and REC beside the Preview / Output tabs at the app's narrowest window width: no
    overlap, no clipped text.
- Also in the panel: rc5's line-field crop, rc3's frame 0, and one frame of a procedural fractal Output take.
- Critics (independent seats; each finding is blocking / should / nit):
  - visual-design: legibility in the 26 px bar, contrast, fixed width;
  - UX: the states read at a glance; "REC now" vs "Starts at bar" are honest; R13's two-recorder confusion;
  - graphic-design: kMeterRed is the only recording colour; no new accent;
  - logic: the labels match AM5 and AM14; the band matches AM2;
  - interaction-logic: AM5's table incl. Waiting-ignored, right-click Cancel, Momentary, the debounce, "Next at bar"
    and the refusals.
- Pass = zero blocking findings across the five seats. At most 3 fix rounds, each re-shot and re-reviewed. After
  round 3, any blocking finding goes on Boris's page as a named open item and Harmony decides hold or ship. Harmony
  runs the panel; the S3 builder fixes.
- Then .harmony/.reports/s-rta-1002b/bf1-boris.html: the captures, one paragraph per state, "what to try", and Q1-Q6.
G7 REGRESSION: Harmony runs it after each stage merge.
- Baselines: run on main just before the stage merges, saved to .harmony/.reports/s-rta-1002b/bf1-g7-baseline/
  <probe>.txt.
- S2 set: probe-video.sh, probe-vupload.sh, probe-media-open.sh, probe-async-load.sh, probe-outputs.sh,
  probe-capture.sh.
- S3 set: probe-idle-paint.sh, probe-routine-display.sh, probe-deck-tabs.sh.
- Pass:
  - every row that PASSES on its baseline still PASSES;
  - fence_black_frames == 0 wherever reported;
  - each probe's own numeric bars hold;
  - probe-idle-paint: zero added timer repaints while REC is idle.

### D. FINAL QUESTIONS FOR BORIS (plain words; each has a default, so the build never waits)

- Q1 When a recording lands in the top row, should it go in the first empty spot from the left, or always after the
  last clip in that row? DEFAULT: the first empty spot from the left.
- Q2 When a recording is done, should it just appear in its spot ready for you to play, or start playing by itself
  right away? DEFAULT: appear, ready to play.
- Q3 If you forget to end a recording, when should it end by itself? DEFAULT: after 64 bars (about 2 minutes at
  128 BPM; up to about 0.8 GB at 1080p).
- Q4 If you record one layer that has see-through parts (like a logo), should the recording stay see-through?
  DEFAULT: no -- see-through parts become black (smaller files, lighter playback).
- Q5 When you play a recording later, should it wait for the next bar so it lines up the way it was recorded, or start
  the moment you press it like your other clips? DEFAULT: like your other clips (your Quantize setting decides).
- Q6 If you switch decks while a recording is running, which deck should the finished clip go into? DEFAULT: the deck
  you were on when you pressed REC (its top row, first empty spot).

### E. DECISIONS FOR HARMONY

- D1 Sacred rule 2: approve the two off-hot-path mutexes, M1 and M2 (AM7). Neither is ever on the GL, audio or analysis
  thread. Architect recommends YES.
- D2 S2 starts only after lane bf9b is merged, with AM9's stop rule.
- D3 Five TEST_SERVER-only debug routes (AM16), the house pattern. They are absent from a non-test build.
- D4 Gate cost: G3 is 30 rows + 3 pre-flights; G4 is about 50 launches (about 30 min). Each is deterministic or
  pre-registered.

### F. RISKS OF THIS RULING (the strongest counterargument to each big call, and why it loses)

- Pinned deck (AM2). Counter: a performer browsing decks may want the clip where he is looking. Loses: the take's
  intent is fixed at the press, and landing in a browsed deck edits a deck he did not choose. Q6 lets Boris flip it;
  the change is one call site in landRecordedTake.
- Waiting ignores a press (AM5). Counter: an accidental arm can no longer be taken back with the same button. Loses:
  it matches the routine pads Boris already uses, and a double-tap can no longer lose an arm. Cancel exists:
  right-click, Momentary release, REST.
- Bar clock = bpm > 0 (AM1). Counter: during a confirming tempo change a quantized CLIP trigger fires at once while REC
  waits for the bar. Loses: in that window bar-snapped triggers do not exist anyway, and REC still lands on a bar,
  which is what Boris asked for.
- No black (AM4). Counter: black is the "truer" picture of a hidden layer. Loses: a take is a sample to replay, and a
  frozen frame is the smaller artefact. It is counted (noPictureTicks). A take of nothing fails instead of landing a
  black clip.
- x264 threads 2 (AM12). Risk: 4K throughput. Guarded by the R3 repeats floor and its pre-registered fallback.
- Worker polling (AM7). Cost: about 500 wakes/s during a take only (INFERRED < 0.5 % of a core); G4 measures process
  CPU.
- S2 depends on a model not yet written (bf9b). Bounded by AM9's stop rule. No S2 code is built on a guess.
- The probes grow (30 live rows). This is accepted. Every row maps to a seat's MUST / SHOULD or a self-found gap (N7,
  N11, N13).

### G. FOUND, NOT FIXED (for the loose-ends ledger)

- E5: the menu recorder's speed, gaps and resize read (plan).
- E21: video BPM Sync ignores tempo changes (plan).
- N6: redo of ANY video drop reopens FFmpeg on the message thread (MainComponent.cpp:5025-5049). Pre-existing; every
  video drop has it.

STATUS: COMPLETE -- ruling-bf1 ready for Harmony: 17 amendments, final gates G1-G7 pre-registered, Q1-Q6 with defaults; S1 buildable now, S2 after bf9b merges, S3 after S2 + lane ui.
