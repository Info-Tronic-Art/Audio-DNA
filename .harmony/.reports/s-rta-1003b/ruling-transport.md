# ruling-transport -- architect ruling on the blind council's attacks on plan-transport.md (s-rta-1003b)

Author: architect (opus, max effort), two rounds and a fix round. Read-only. Source read ONLY at main 34179a2
(`git show 34179a2:<path>`, `git grep ... 34179a2`). Nothing built, run, launched or probed. Nothing under
`.claude/worktrees/` read.
Labels: VERIFIED = read at 34179a2 at the cited line. INFERRED = reasoned from lines read, not run. ASSUMED = not read.
Boris is quoted only verbatim. "Harmony constraint:" marks what Harmony requires.
Plan: .harmony/.reports/s-rta-1003b/plan-transport.md. Papers: .harmony/.reports/s-rta-1003b/attack-transport-papers.md
(all five papers whole, 45 attacks; written in round 2 from attack-transport-papers-full.json).

STATUS: DONE (round 2, the completion round, 2026-10-03; then the fix round on its completeness check, same day).
All 45 attacks of the five papers are ruled. All 7 findings of the completeness check are verified and fixed in place.

WHAT WAS MISSING, NOW CLOSED. Round 1 was PARTIAL for one reason: the dispatch cut the seat papers at 70,000
characters (.harmony/.reports/s-rta-1003b/wf/transport-plan.js:73 `JSON.stringify(papers).slice(0, 70000)`), so the
fifth paper, "scope (MINIMALIST)", ended inside SC-6. Round 2 read the five papers whole from
.harmony/.reports/s-rta-1003b/attack-transport-papers-full.json. Checked field by field: every word that reached
round 1 (ST-1..ST-10, UN-1..UN-8, RE-1..RE-9, GA-1..GA-10, SC-1..SC-5, and SC-6's target, claim, evidence, severity)
is the same in the full file. New in round 2: SC-6's proposed change, SC-7, SC-8, the seat's strongest point -- all
four ruled in section 2 -- and Boris's words of 2026-10-03 20:59:50, checked against sections 3-7 (AM-27).
A round-2 change of a round-1 line is marked "round 2" or cites a round-2 number; section 0 lists every one.
A fix-round change is marked "fix round" where its place does not say so; section 0 lists every one.

## 0 VERDICT

BUILDABLE, from the plan body plus this ruling, this ruling overriding. No section of the plan has to be rewritten
first: every part the amendments change beyond a patch is replaced whole INSIDE this ruling -- plan section 4 by
section 4 here, plan section 5 by section 5, plan sections 7 and 8 by sections 6 and 7, T3's "THE CHANGE" by AM-11
and AM-13, T5's "EVERY COMMAND" table by the table in section 10. Section 10 is the reading map, stage by stage.
(Round 1 said "NEEDS REVISION, then buildable": the revision is the amendments, and it is complete.)

The plan's shape holds: one lane; the restart decided in the layer's activation tail and carried out by the render
thread; BPM Sync as a rate; Undo that leaves what plays alone. As written it would have shipped defects on stage,
and its headline unit gate could pass without running a test.
45 attacks: 28 ACCEPT, 16 PARTIAL, 1 REJECT. 30 amendments. The ones that change the build most:
- A queued re-fire would restart the clip at the click, not on the bar (RE-1). Fixed in the tail hook (AM-1).
- Undo could still change a playing clip: its direction, speed, trim (ST-1), its media file (UN-1), and it could put
  one clip id in two cells (UN-2), duplicate a column (UN-3), rewind a live clip (UN-4), cancel a queued fire (UN-5).
  ONE landing rule for every clip value that lands on a cell (AM-6), a refused Redo instead of a spent one, because
  a spent one leaves later Redo steps one row off (AM-7, my own finding on top of UN-3), ONE "live" test (AM-8).
- The restart counter could swallow or invent a restart after a value copy (GA-6). It becomes a stamp from one
  process-wide counter, consumed only when newer (AM-2).
- The drag's hold had a race that leaves the clip paused after the drop (RE-2) and could stick on a live clip (RE-3).
  Boris did not ask for a hold (SC-2). The drag is built WITHOUT it; the hold is a question with a pre-specified
  stage (AM-11, AM-12).
- "Every fire plays" (F1) is NOT Boris's word (RE-9, GA-5). It stays as Harmony's default, relabelled, because the
  code read says the opposite rule leaves dead fires; one measurement on the frozen app decides (AM-3, FM-2).
- Gates: unit gate by test NAME with a pinned count (AM-19); the drag row drives the real mouse handlers (AM-20);
  RED strings corrected; S4a no longer breaks the build (AM-15).
Four facts need a run (section 5.6). Ten questions go to Boris, each with a default, and two notes; nothing waits
on them. Four decisions are Harmony's, each with a default (end of section 4).

WHAT ROUND 2 CHANGED
- SC-6, ruled again with its proposed change, stays PARTIAL (AM-18 rewritten). Accepted: the REST trim (it is the
  seat's own test, and what AM-18 did); this lane does not void the bars ruling's relabels (decision H-1); the bar's
  beat lines are the old loop fed with the new beats, now pinned by TL-U17. Not followed: the inert "Restart /
  Continue / Relative" combo still goes in S4b (decision H-4 gives Harmony the other way).
- A finding of my own while re-deriving SC-6 (AM-26): the inert Duration row is PAINTED "Beats" in BPM Sync and has
  its own "/2" pair. Beside the new Beats row it would be a second, dead "Beats" row. S4b removes it unless the bars
  lane already has (decision H-2). The plan's "either order is safe" was wrong.
- SC-7 PARTIAL (AM-28): the five critic seats and the eleven states stay -- the five seats are Harmony's own
  requirement for this gate. What a number can decide moved from a seat to a bar (B4 widened, B9 new, B3 and B7
  tightened); the critics' packet follows the rig rule.
- SC-8 PARTIAL (AM-29): S1 alone may be built before the sync-dial lane merges, at Harmony's choice (H-3).
- The scope seat's strongest point: its attack (SC-1) was accepted in round 1; its "must not be cut" item -- the
  render-thread restart mechanism -- is cut by nothing here and is now protected by name (AM-30).
- Boris, 2026-10-03: "ok. the only fail message will be a failed save. remove all others". Sections 3-7 held no line
  that adds an event or a failure text. Now explicit and gated: AM-27, gate G-N1, bar B7, Boris check 23.
- AM-3 and AM-7 gained one line each (the take capture's test follows the tail in both forms; a refused Redo is
  silent).
- Counts: 54 unit cases and 17 live rows unchanged; one new gate row (G-N1); bars B1-B9; 23 Boris checks; 10
  questions, 2 notes; 4 Harmony decisions.

CHANGED IN PLACE (every round-1 line that round 2 touched; everything else reads as in round 1)
- Header and section 0: the status, the verdict's wording, 43 -> 45 attacks, 25 -> 30 amendments.
- Section 1: R27-R36 added. Section 2: the SC-6 row rewritten; the SC-7 and SC-8 rows, the re-check of SC-1..SC-5
  and the strongest point added.
- Section 3: AM-3 (one bullet added), AM-7 (one bullet added), AM-18 (rewritten), AM-26..AM-30 (new), one sentence
  under the forks line.
- Section 4: the first paragraph (AM-29); S0 (two lines added); the amendment lists of S4a, S4b, S5; the bars-lane
  bullet; the "Duration" row bullet; decisions H-1..H-4 (new).
- Section 5: the preamble's constraint ("or a failure"); 5.1 TL-U17's name (the beat-line clause); 5.2 G-N1 (new);
  5.3 TR13a's parenthesis; 5.5 V10's wording, bars B3, B4, B7 (changed), B9 (new), the critics' packet line, the
  logic seat's question, "Pass = B1-B9".
- Section 6: check 23 (new). Section 7: notes N1, N2 (new); the ten questions unchanged.
- Section 8: SF-1 reworded; SF-12..SF-14 new. Section 9: K8 replaced; K11-K13 new. Section 10: new.
- Untouched: every round-1 verdict; AM-1, AM-2, AM-4..AM-6, AM-8..AM-17, AM-19..AM-25; the 54 unit-case ids; the
  bars of the 17 live rows; FM-1..FM-4; Q1-Q10; checks 1-22.

WHAT THE FIX ROUND CHANGED (the completeness check, review-ruling-transport-complete.md: no MUST, 3 SHOULD, 4 NIT.
Each finding was verified against this file and the pin; all seven hold. The list is at the end of section 2.)
- No verdict, no amendment number, no fork and no question changed. 30 amendments; 54 unit cases, 17 live rows,
  bars B1-B9, 23 checks, 10 questions, 2 notes, 4 Harmony decisions: as before.
- F1: a stage no longer has to prove a field that a later stage adds. Three S1 cases get their clause in S2 or S4a;
  TL-L1 has an S2 form; the TSan pin is given per stage. MU-1, MU-28 and MU-29 show on the lane head that the three
  clauses are there.
- F3: the two conditional stages have registered gates. S2b: TL-U6b, two probes whole, FM-1 again. S3h: six cases,
  MU-H1 .. MU-H3, live row TR8. Each pin has its rule for them.
- F2: every gate row says whether it has a RED arm, and which. Verifying it found two teeth that did not bite as
  written: TR7b now places the playhead before its press, and TR7c names the variant that MU-10 turns RED.
- N1-N4: TR13a reads a field that exists (`reveal_target`); the "--" reading is Harmony's to sign, the other way
  specified; the menu's Redo item after a refused Redo is said, accepted and handed to the Edit-menu lane (K15); the
  combo's "what Boris ruled out" is narrowed to "Continue".
- Changed in place: the header; this block; section 1 (R37-R40); section 2 (the list at its end); AM-7, AM-12,
  AM-18, AM-21, AM-27; section 4 (S1, S2, S2b, S3, S3h, S4a, the Timeline-lane and Edit-menu bullets); 5.1 (its
  heading, three S1 names and the note under them, TL-L1 by stage, the conditional stages); 5.2 (G-U1, G-U1b, G-U2,
  G-U3, G-U4, G-U5, G-N1's numbers); 5.3 (the pin line, TR7b, TR7c, TR8, TR13a); 5.4; 5.5 (two lines before the
  seats); 5.6 (FM-1); SF-15; K8, K14, K15; section 10 (two table rows, the closing lines). All else reads as in
  round 2.

## 1 FACTS RE-DERIVED (at 34179a2; every line below read by me)

Firing and the queue
- R1 VERIFIED `Layer::activate` runs `applyActivationTail` before every compare-exchange whose result has `ref` as
  the active ref (src/model/Layer.h:572-576; the hook contract :338-349), and once more after a call that changed
  nothing while `ref` is active (:577-579). Today `triggerClip` keeps a re-fire of the active ref out of the queue
  (:424), so the hook never sees "active ref unchanged, pending slot set to the same ref".
- R2 VERIFIED the release runs the tail when the pending ref becomes or is the active ref (Layer.h:484-487);
  `immediateNext` clears the pending slot and returns the tuple otherwise unchanged for the active ref (:361-367).
- R3 VERIFIED the tail: playhead = in-point, `beatsPlayed = 0`, `playing = true` only for a new activation of a clip
  whose `hasBeenTriggered` is false (Layer.h:551-557). The clear tail sets the OLD active clip `playing = false`
  (:587-592); `releaseMomentary` runs it (:509-526) and tests the active ref before the pending ref (:514-516).
- R4 VERIFIED `hasBeenTriggered` is written only at src/MainComponent.cpp:4866 (cell path). INFERRED from R3 + R4:
  a clip that was fired, then cleared from its layer (X button, an empty cell in a column fire, a momentary release)
  and fired again by a cell click stays `playing == false` today. Not run: FM-2.
- R5 VERIFIED Quantize is Off unless the tracker is LOCKED (MainComponent.cpp:36-43). REST fires call the handlers
  with `Origin::Human` (MainComponent.cpp:1886-1887; defaults at src/MainComponent.h:541, :543), and a Human fire
  pushes a TriggerClipCmd (:4976-4983).
- R6 VERIFIED bindings: `processKeyDown` calls the action with 1.0 on every key-down, no held test
  (src/binding/BindingManager.cpp:52-74); a CC in absolute mode calls it on every message (:189-197); the action
  fires on `value > 0` (MainComponent.cpp:7704-7716, :7737-7753); `keyPressed` filters nothing
  (MainComponent.cpp:4100-4108); only releases of Momentary keys are polled (:4120-4145). INFERRED (JUCE source in
  build/_deps, not in the repo, not run): `redirectKeyDown` has no auto-repeat filter
  (juce_NSViewComponentPeer_mac.mm:918-926). So a held bound key re-fires at the OS repeat rate, and for a cell
  binding it already re-seeks the video each time today (MainComponent.cpp:4890-4914).
- R7 VERIFIED a layer that is hidden, bypassed, or muted by another layer's solo is skipped before any clip texture
  call (src/render/CompositorEngine.cpp:1079-1080; the calls at :1119, :1226), so `syncMedia` does not run for it.
- R8 VERIFIED readers of the pending slot outside Layer.h and DeckCommands.h: src/model/Autopilot.cpp:83,
  src/api/ApiServer.cpp:455 (`pendingClip`), src/recording/PerfState.cpp:86, :121 and PerfStateCapture.cpp:70.
  No file under src/ui reads it (grep `pendingRef()|pendingTriggerColumn|pendingDeckId`): the plan's R4 "pad's queued
  look" does not exist at the pin.

Transport sync and players
- R9 VERIFIED `pushIntent` returns only the intent it read (src/render/ClipTransportSync.h:32-41); `writeBack` is
  level-triggered on `ph >= outPoint` (:56): OneShot stops (:58-66), every other loop mode seeks to the in-point
  (:67-71) -- a trimmed PingPong wraps like a Loop.
- R10 VERIFIED `syncMedia` reads, on the GL thread every drawn frame, `reverse`, `loopMode`, `transportMode`,
  `beatDivision`, `videoBeats`, `speed`, `sequenceFps` (src/render/Renderer.cpp:1647-1671, :1705-1733). These are
  PLAIN fields (src/model/Clip.h:32-41, :118-128); only `playing`, `playheadPosition`, `beatsPlayed`,
  `hasBeenTriggered` are Relaxed (:237-240). Pitfall 63 rule 1 forbids ADDING a plain shared field
  (docs/claude/pitfalls.md:137); tests/test_shared_field_types.cpp:29-34 pins the four runtime fields.
- R11 VERIFIED a video in BPM Sync plays at `videoBeats / beatDivision`; the tempo is only a gate
  (Renderer.cpp:1657-1667). A sequence in BPM Sync already takes its fps from the raw `snap.bpm` every frame
  (:1718-1733). `FeatureSnapshot::bpm` = "stabilized/locked tempo estimate (BPM), 0 if unknown"
  (src/analysis/FeatureSnapshot.h:40).
- R12 VERIFIED a re-opened player is a new object: `installVideoPlayer` retires the old one and inserts the new
  (Renderer.cpp:1355-1383); `openVideoForClip` takes only a clip id and a file (:1346-1353). The media hook re-opens
  a video only when no player exists or the loaded file differs (MainComponent.cpp:5217-5227,
  src/core/MediaReconnect.h:12-17); a sequence only when none exists (:5228-5235).
- R13 VERIFIED no frame ready = HOLD the last shown frame (pitfalls.md:121, Pitfall 56); a pending clip holds the
  layer's last picture (CompositorEngine.cpp:1122-1136). INFERRED: a clip that left its layer and is fired again
  shows the frame it left on until the decode thread delivers its start. How long: FM-1.
- R14 VERIFIED `seekTo` is a request; `advanceFrame` consumes it before the transport step and bumps the generation
  (src/media/VideoPlayer.cpp:354-386); a OneShot stops at either end (:431-434, :449-452).

The drag
- R15 VERIFIED the strip writes only the model (src/ui/LayerStrip.cpp:979-989) and re-resolves `playingClip()` on
  every mouse event (:982). The Clip tab grabs a marker within 8 px (src/ui/ClipInspector.cpp:1361-1364), else writes
  the model and calls `onCuepointJump` (:1368-1375, :1395-1399), wired to `seekTo` (MainComponent.cpp:1464-1477);
  in / out drags write the model with no Undo step (:1387-1393).
- R16 VERIFIED a test-only component hook driven by a debug route already exists: `/api/debug/tab_click` ->
  `deckView_->clickTabForTests` (ApiServer.cpp:338-339, MainComponent.cpp:2245-2246).

Undo
- R17 VERIFIED SetClipCmd and SwapClipsCmd address a cell by INDICES (deckIndex, layerIndex, column) and land a
  whole Clip by value, runtime fields included (src/core/ClipCommands.h:83-91, :108-131, :246-263). An FX drop pushes
  whole-cell SetClipCmds (MainComponent.cpp:1013-1025). Replace Content mutates the clip in place, keeps its id, and
  pushes a SetClipCmd (MainComponent.cpp:7167-7177); `replaceContent` copies the media fields and keeps the
  transport settings (Clip.h:254-335). A plain file drop puts a NEW clip (new id) into the cell, also as a
  SetClipCmd (`commitDrop`, MainComponent.cpp:5470-5479, :5494-5499).
- R18 VERIFIED `UndoManager::redo()` runs `execute()` and advances the index unconditionally
  (src/core/UndoManager.cpp:61-72); `undo()` likewise (:48-59). `pushCommands` performs a single child directly and
  wraps several in a CompositeCommand (MainComponent.cpp:5323-5338).
- R19 VERIFIED RemoveColumnCmd removes only the LAST column (src/core/DeckCommands.h:109-118); its undo inserts the
  saved cells unconditionally and sets the count (:141-160). AddLayerCmd appends (:499-501) and its undo erases
  `addedIndex_` unconditionally (:506-513). InsertDeckCmd appends its added layers (:819-831). RemoveLayerCmd has
  `erased_` (:585-587, :610). ClearLayerClipsCmd assigns the whole row and calls `setRuntime` (:336-339).
  RemoveLayerCmd and RemoveColumnCmd are each pushed alone (MainComponent.cpp:6840-6845, :7008-7014); Remove Layer
  always removes the LAST layer (:6838). InsertDeckCmd's Redo re-adds its layers at the END (:800-801); AddLayerCmd's
  Redo re-inserts at its recorded index (:495).
- R20 VERIFIED three "live" tests disagree: `tupleNamesDeck` = active or pending (DeckCommands.h:226-230);
  `Composition::deckIsPlaying` = active, or previous while the fade runs, never pending
  (src/model/Composition.h:608-618), used by `retireOrEraseDeck` (:624-635) and the reap (:652-666).
  `cancelPendingInto` writes the pending slot through `updateRuntime` (DeckCommands.h:195-212) and is called by
  AddDeckCmd::undo (:727), InsertDeckCmd::undo (:854), RemoveDeckCmd::execute (:946). `RowClips` resolves a retired
  deck's cells (Composition.h:270-276).
- R21 VERIFIED bypass and solo are Undo steps (MainComponent.cpp:749-767; ToggleLayerFlagCmd,
  DeckCommands.h:435-469); MoveLayerCmd reorders the shared stack (:619-647).
- R22 VERIFIED `/api/set_clip_param` accepts only `fitMode` and is "Not undo-recorded, like every other per-clip
  field write" (ApiServer.cpp:712-748). No REST route adds an effect to a deck clip as an Undo step (every POST
  route of ApiServer.cpp:163-344 read; `/api/set_effect*` drive the legacy chain, Pitfall 28). `/api/perf/load`,
  `/api/perf/play` and `/api/routine/fire` exist: a fire with no Undo step can be driven by REST.

Gates and widgets
- R23 VERIFIED `catch_discover_tests(...)` runs without TEST_PREFIX for the suites this lane touches
  (tests/CMakeLists.txt:389, :594, :3079, :3177, :3395, :3436, :3511, :3597, :3627), so a ctest name is the Catch2
  TEST_CASE name, e.g. "T2 firing a cell of a deck that is not shown ..." (tests/test_show_model.cpp:194),
  "LayerRuntimeCell: pack / unpack round trip ..." (tests/test_layer_runtime.cpp:66). ruling-bf6.md:563-565 already
  recorded that a `-R` regex on target names can select 0 tests.
- R24 VERIFIED the Transport rows' labels are painted (`g.drawText("Speed" ...)`, ClipInspector.cpp:542, :546);
  Reverse sits in the Speed row (:642); the Clip tab scrolls inside a Viewport (src/ui/InspectorPanel.cpp:29-30).
- R25 VERIFIED `beatDivision` / `videoBeats` are used by src/ui/ClipInspector.cpp (76 lines), ClipInspector.h and
  three tests: tests/test_clip_inspector_paint_key.cpp:62, tests/test_composition.cpp:224-225, :274-275, :548-549,
  tests/test_undo_commands.cpp:116, :344-345.
- R26 VERIFIED probe-boxes k10 `resumeTol` is 0.5 (.harmony/probe-boxes.json:25); k10 (ii) at
  .harmony/probe-boxes.py:1523-1525. The pinned retrigger SECTION starts at tests/test_layer_runtime.cpp:280; its
  `playing == false` assert is at :293 (the plan says :299). T6h: tests/test_show_model.cpp:706-734.
Plan facts re-read and found as stated: E1, E3-E7, E9, E11-E12, E15-E17 (rows named), E19-E23 (line fix above).
NOT re-read (the plan's label stands): ImageSequence.cpp, RoutineEngine.cpp, the decode-thread seek, PerfState's
restore path, Autopilot.cpp beyond its three `triggerClip` calls (:326, :355, :417) and the release (:85).

Round 2 -- re-derived for SC-6, SC-7, SC-8, the strongest point and Boris's new words
- R27 VERIFIED the inert combo: `triggerDropdown_` gets three items "Restart", "Continue", "Relative" and id 1
  selected (src/ui/ClipInspector.cpp:69-73); no onChange, no model field. Its only other lines in src are its bounds
  -- the right half of the transport-buttons row, beside the Loop dropdown (:632-634) -- and its declaration
  (src/ui/ClipInspector.h:133). INFERRED (grep over .harmony/*.md, the reports of s-rta-1002b, s-rta-1003,
  s-rta-1003b and BORIS_DECISIONS.md): no ruling gives its removal to a lane;
  .harmony/.reports/s-rta-1003/facts-transport.md:188 says the same.
- R28 VERIFIED the inert Duration row: `durationSlider_` has no onValueChange (ClipInspector.cpp:90-98); its pair is
  `durHalfBtn_` "/2" and `durDoubleBtn_` (a multiplication sign and "2") (:102-103); bounds :656-660. Its painted
  label is "Beats" in BPM Sync and "Duration" in Timeline (:546). The Speed row's pair reads a division sign and
  "2", a multiplication sign and "2" (:100-101). ruling-bf7 AM4 removes the row as the bars lane's item I13
  (.harmony/.reports/s-rta-1002b/ruling-bf7.md:254-265); at 34179a2 the row is still in the tree.
- R29 VERIFIED the bar's beat lines: drawn only in BPM Sync with `beatDivision > 0`; `numBeats =
  int(beatDivision)`; for numBeats >= 2 one line at each b / numBeats of the in..out range, b = 1 .. numBeats - 1
  (ClipInspector.cpp:1265-1277).
- R30 VERIFIED REST: `/api/set_clip_param` accepts only `fitMode` (src/api/ApiServer.cpp:729) and answers "ok" for
  any well-formed request before the write has landed (:714-717, :742-747). `/api/composition` reports per clip
  `id`, `name`, `column`, `playing`, `fitMode`, `playheadPosition`, `mediaType`, `sourceType`, `mediaMissing`
  (:502-512): no transport setting, no in or out point. Routes that exist: `/api/debug/drop_files` (:307),
  `/api/debug/load_deck` (:320), `/api/debug/tab_click` (:338), `/api/debug/undo` (:340), `/api/debug/clip_media`
  (:342), `/api/debug/inspect_clip` (:344).
- R31 VERIFIED ruling-bf7 AM6 relabels "Beats/ Cycle" to "Loop length" and "Content Beats" to "Content length",
  each with a tooltip (ruling-bf7.md:272-277; its reasoning :81-85). The labels sit at ClipInspector.cpp:204, :237.
- R32 VERIFIED what the scope seat's "must not be cut" rests on: `writeBack` stores the player's position into the
  model on every sync (src/render/ClipTransportSync.h:47-48); `Autopilot::processFrame` has one caller, the render
  thread (src/render/Renderer.cpp:513), and it runs the quantized release (src/model/Autopilot.cpp:85) and the
  advances (:326, :355, :417, read in round 1).
- R33 VERIFIED the four Undo / Redo call sites act only on `true` (`if (undoManager_.redo())
  refreshAfterUndoRedo(...)`, MainComponent.cpp:4055-4056, :6622-6623; undo :4060-4061, :6616-6617): a Redo that
  returns false writes nothing on screen. `triggerWillAutoPlay` (:192-198) repeats the tail's rule of today: not the
  active ref, never fired, not playing.
- R34 VERIFIED the app's ways to put an event or a failure on screen
  (.harmony/.reports/s-rta-1003/facts-notices.md:27, :209-210): `setFileLabel(` (46 lines under src at 34179a2; one
  widget that carries state and events), `AlertWindow::` (22 lines), `showMessageBox` (13), the Record tab's
  `setNotice`. The fire handler's own label writes name the fired clip's file (MainComponent.cpp:4922-4958);
  facts-notices.md:132 files that kind of write as state (C01; its line numbers are of its own pin -- INFERRED the
  same writes).
- R35 VERIFIED Harmony's requirement for this lane's visual gate: "the questions for five critic seats
  (visual-design, UX, graphic-design, logic, interaction-logic)" (wf/transport-plan.js:31). .harmony/RIG-RULES.md:
  19-20: the capture builder writes a manifest with model facts per state, and critics get Boris's binding words
  verbatim and the list of pre-existing texts outside the lane; :28: the gate script runs a capture builder, then 5
  critics.
- R36 The files that the sync-dial lane's two reports in this folder name (rulings-bf2.md,
  ruling-bf2-gates-restated.md; names only -- bf2's source was NOT read): MainComponent.cpp/.h, api/ApiServer.cpp,
  sync/SyncVenues.h, sync/SyncOffsetController.h, model/Composition.h, ui/TopBar.h/.cpp, ui/DeckView.cpp,
  ui/OutputWindow.cpp, ui/ClipInspector.cpp, core/StagedLoad.h. None of core/ClipCommands.h, DeckCommands.h,
  TriggerCommands.h, Command.h, UndoManager.cpp, CompositeCommand.h, model/Clip.h, model/Layer.h. INFERRED: S1's
  command files do not meet bf2; MainComponent.cpp, ApiServer.cpp and Composition.h do.
- R25 again: exactly ten test lines name the two fields (the three files of R25); under src only Clip.h (8 lines),
  Clip.cpp (6), Renderer.cpp (9), ClipInspector.cpp (76), ClipInspector.h (5); docs/claude/architecture.md 2.

Fix round -- re-derived for the completeness check
- R37 VERIFIED `restartStamp`, `clipBpm`, `mintRestartStamp`, `scrubClip`, `deckIsLive`, `landClipInCell` and
  `lastRunRefused` occur nowhere under src or tests. `seekTo(` in src/MainComponent.cpp: six lines -- :1470, :1475
  (the `onCuepointJump` wiring), :4881, :4886 (the beat-phase seek of a Snap clip), :4907, :4912 (the re-fire seek).
- R38 VERIFIED `/api/debug/clip_media` reports `reveal_target` = `d.revealTarget.getFullPathName()`
  (MainComponent.cpp:2272), "what Show in Finder selects" (src/ui/ClipMediaText.h:23, :31); it reads the cell of
  the deck the grid shows (:2257-2258).
- R39 VERIFIED `canRedo()` is the stack's position alone (src/core/UndoManager.cpp:79-82); the menu's Redo state is
  (`redoDescription()`, `canRedo()`) (MainComponent.cpp:1772-1775) and its text "Redo " + the description
  (src/ui/MenuBarModel.cpp:59; the item sits in the Composition menu, src/ui/MenuBarModel.h:130).
- R40 VERIFIED a Clip-tab press within 8 px of a marker only chooses the drag target and writes nothing
  (ClipInspector.cpp:1361-1364; the writes are in mouseDrag, :1387-1393). `writeBack` enforces the out-point only
  when `outPoint < 1` (ClipTransportSync.h:56). k10 waits until the clip's content time is >= 4.0 before it leaves
  the clip (.harmony/probe-boxes.py:1506-1511) and checks (ii) at :1523-1525. probe-asan-unit.sh: pin 10 (:31),
  exit 3 below the pin (:6, :60-61), last line (:74); probe-tsan-unit.sh: pin 5 (:29); probe-video.sh:74 and
  probe-media-open.sh:71 print `PROBE-VIDEO GREEN` and `PROBE-MEDIA-OPEN GREEN`. G-N1's five strings count 46, 22,
  13, 3, 6 lines under src. The papers file: each of its five JSON blocks decodes equal to its paper in
  attack-transport-papers-full.json (45 attacks).

## 2 ATTACK RULINGS

| id | sev | verdict | the line that decides it | amendment |
|---|---|---|---|---|
| ST-1 | MUST | ACCEPT | R17: a whole-Clip value lands; R15: in / out drags are no Undo step. Undo of an FX drop brings back the old reverse / speed / trim of a playing clip. | AM-6 |
| ST-2 | MUST | PARTIAL | R9: a drop clamped to just inside the out-point wraps on the next advance, so the plan's "no drop can be sent elsewhere" is false and U15 cannot fail. The seat's preferred fix (leave the playhead where it was) is the jump back Boris named; rejected. The clamp stays, said truthfully, tested with time. | AM-13, Q10 |
| ST-3 | SHOULD | ACCEPT | R6: no held-key state, no edge test; a held key re-fires. Already true for cell bindings today; the lane adds the column path. | AM-4 |
| ST-4 | SHOULD | PARTIAL | R3: the release tests the active ref first. The outcome is stated and pinned: it clears, as the same press and release do today. The seat's outcome (leave it playing) would change what a momentary release does; Boris asked "What does this mean:" about that line, he set no rule. | AM-1 |
| ST-5 | SHOULD | PARTIAL | R7: true, and already true of every clock: a layer that is not drawn does not advance. The restart lands on the first drawn frame. Stated, unit-pinned, on Boris's list. No code. | AM-5 |
| ST-6 | SHOULD | ACCEPT | R15, R16: the route must go through the real handlers and the wiring. | AM-20 |
| ST-7 | SHOULD | ACCEPT | R11: an old BPM-synced VIDEO ignores the tempo today and will follow it. Check 10 was false. | AM-16 |
| ST-8 | SHOULD | ACCEPT | R24: the pair sits where the Speed pair sits, with the opposite effect on motion. His words do not say what is doubled. | AM-17, Q7 |
| ST-9 | SHOULD | ACCEPT | R5: no lock, no Quantize. Check 3 could not be performed as written. | AM-23 |
| ST-10 | SHOULD | ACCEPT | R21: the plan's line is a reading. One wider question; the reading is labelled. | AM-10, Q3 |
| UN-1 | MUST | ACCEPT | R17, R12: Undo of Replace Content on a playing cell re-opens the old file under the layer. | AM-6 |
| UN-2 | MUST | ACCEPT | R17, R18: re-derived by hand; step 6 of the seat's sequence lands clip A in cell 1 while A plays in cell 2. | AM-6, AM-19 (TL-UC16) |
| UN-3 | MUST | ACCEPT | R19: the unconditional insert. Fixed by REFUSING the Redo (the index does not move), not by a flag alone. | AM-7 |
| UN-4 | MUST | ACCEPT | R19: a whole-row assignment cannot leave a live cell and brings stale transport. | AM-6, AM-2 |
| UN-5 | MUST | ACCEPT | R20. | AM-8 |
| UN-6 | SHOULD | ACCEPT | Boris never answered this outcome; it is asked and gated. | AM-9, Q8 |
| UN-7 | SHOULD | PARTIAL | Same question as ST-10. The seats differ on the default. The default stays today's behaviour (no new behaviour on a guess); the literal one is pre-specified; it goes on Boris's page before the merge. | AM-10, Q3 |
| UN-8 | SHOULD | ACCEPT | R5 + ruling-bf9b-merge.md:152: on the frozen app the one undo undoes the FIRE. | AM-20 (TR12) |
| RE-1 | MUST | ACCEPT | R1. | AM-1 |
| RE-2 | MUST | ACCEPT | R9: two reads of the hold in one sync. Binding on the hold if it is ever built. | AM-12 |
| RE-3 | MUST | ACCEPT | R15: the strip re-resolves the clip per event. The session owns the clip from the press on (built now); the rest binds the hold. | AM-11, AM-12 |
| RE-4 | MUST | ACCEPT | Same as ST-6. | AM-20 |
| RE-5 | MUST | ACCEPT | R10: Pitfall 63 rule 1. | AM-14 |
| RE-6 | SHOULD | PARTIAL | R13: real, size unknown without a run. Pre-roll as proposed covers only queued fires. | AM-21, FM-1 |
| RE-7 | SHOULD | PARTIAL | R12: with AM-6 an Undo never re-opens a live clip's player (other media: the cell is left; same media: no re-open). The fresh-player rule stays; the test moves to the render level. Seeding at open is rejected (R12: the opener has no Clip). | AM-2, AM-6 |
| RE-8 | SHOULD | PARTIAL | R11: TR9 cannot see wobble or drift; raw `bpm` per frame already drives sequences. Two measurements and a question, no build change on a guess. | AM-22, FM-3, FM-4, Q9 |
| RE-9 | SHOULD | PARTIAL | His "restart" answers a question about position. The label is fixed. The narrow default is rejected for now: R3 + R4 say it leaves dead fires. FM-2 decides. | AM-3, Q1 |
| GA-1 | MUST | ACCEPT | R23. | AM-19 |
| GA-2 | MUST | ACCEPT | Plan lines 605, 624-625 against 292: the bar contradicts the clamp. TR8 leaves the list with the hold; if S3h is built its numbers are in range. | AM-12, AM-20 |
| GA-3 | MUST | PARTIAL | Accepted as ST-6. Rejected: "run it on the frozen app" -- the route is new, the frozen app has none; teeth are mutants. | AM-20 |
| GA-4 | MUST | ACCEPT | Plan line 636. The seat's driver is wrong (R22: `set_clip_param` is no Undo step); the rows use `/api/debug/drop_files` on the PLAYING cell and one new test route. | AM-20 (TR13a, TR13b) |
| GA-5 | MUST | PARTIAL | Same as RE-9: the attribution is wrong and is fixed; Harmony signs the default. | AM-3, Q1 |
| GA-6 | MUST | ACCEPT | Players persist per clip id (Renderer.cpp:1633-1638); a rewound counter can meet the seen value. | AM-2 |
| GA-7 | SHOULD | ACCEPT | R20: the lint misses `updateRuntime`. A behaviour test over every command replaces trust in spelling. | AM-8 |
| GA-8 | SHOULD | PARTIAL | Same as UN-6. The seat's alternative (erase the idle added layers one by one) is rejected for this lane: it multiplies the command's states (each layer kept or erased x deck present, retired or reaped). | AM-9, Q8 |
| GA-9 | SHOULD | ACCEPT | R24. | AM-17 |
| GA-10 | SHOULD | ACCEPT | E6 + TR4: the RED arm reads about 2.3 s, not 7.3 s. TR6 is driven by injected beats and judged by order. No silent downgrade. Rows added for the two GL-thread paths. | AM-20 |
| SC-1 | MUST | ACCEPT | R25. | AM-15 |
| SC-2 | SHOULD | ACCEPT | Boris: "I wanted to keep playing at the same speed, but play from wherever I drop the play head." No word about holding still. The hold is where RE-2 and RE-3 live. | AM-11, Q2 |
| SC-3 | SHOULD | PARTIAL | TR1 is cut (the re-run rows m6, w3, w3b, w6b already guard a cell re-fire). TR7 stays, driven through the handlers. TR8 leaves with the hold. TR13 is replaced. Mutants: one per rule, not per test. | AM-20 |
| SC-4 | SHOULD | REJECT | Boris: "if we are in Qantize mode, it is triggered on time by the Qantize method." A column re-fire that restarts some layers now and switches the others on the bar is not on time. RE-1's fix makes F2 correct; R8 shows the readers of the pending slot are few. | AM-1 |
| SC-5 | SHOULD | PARTIAL | Same as RE-7. | AM-2, AM-6 |
| SC-6 | SHOULD | PARTIAL | Round 2, with its proposed change. Accepted: the REST trim (R30: a field stays only when a gate row reads or writes it -- the seat's own test, and what AM-18 did); "Ask the chair to rule on bf7's relabels" (R31: this lane voids nothing; decision H-1); the beat lines computed from the new beats (R29: that is the plan's rule, made exact). Rejected: leaving the inert combo (R27) -- its three choices name what a fire does to the position, the one thing this lane rules and Boris answered "restart"; no lane owns it; 8 lines, no reader. | AM-18, AM-26 |
| SC-7 | NIT | PARTIAL | R35: the five seats, by name, are Harmony's requirement for this gate -- the seat's own condition ("If five seats is Harmony's standing rule for visual work, keep it and state so"). The states stay: the bars the seat keeps (B5, B6, B7) read the dumps of V5, V10, V3, which it cuts. Accepted: the logic seat's equation is a number, so it is a bar now (B4 over every state, B9). | AM-28 |
| SC-8 | NIT | PARTIAL | R36: none of S1's command files is named by bf2's reports, and S1 is proved by unit cases on its own base, so S1 MAY start before bf2 merges. Rejected for S2, S3, S4a, S4b: they edit or read what S0 asks about (the two handlers, `syncMedia`, the release's beat clock, `bpm`, the route table), and S3's lint needs S2. The RED arm is still frozen from the merged main. | AM-29 |

Seats reconciled, not relayed: ST-6 / RE-4 / GA-3 / SC-3 (the drag rows); ST-10 / UN-7 (the default of Q3);
RE-9 / GA-5 (F1); RE-7 / SC-5 (re-opened player); UN-6 / GA-8 (F18); RE-2 / RE-3 / SC-2 / GA-2 (the hold);
SC-4 against RE-1 / ST-4 (F2).

Round 2. SC-1..SC-5 re-checked against the whole paper: each has its row above, and its text in the full file is
the text ruled in round 1. One clarification, no verdict changed: SC-2 also asked to drop TR7 and the test route;
that part is ruled under SC-3 (TR7 stays, driven through the real handlers).
The scope seat's strongest point, weighed. (a) It names SC-1 as its strongest attack: accepted in round 1 (AM-15);
its citations hold (R25, R27-R29). (b) Its "ONE thing that must NOT be cut is T1's render-thread restart mechanism",
with TR2, TR3 and TR4 as its teeth: re-derived (R32) and true -- a model-only reset is overwritten on the next drawn
frame, and two fire paths never pass a handler. No amendment cuts it: AM-1 and AM-2 change how the stamp is minted
and consumed, not that the render thread carries the restart out before the intent is pushed (TL-U5, MU-2); TR2,
TR3, TR4 stand with corrected RED strings; TR6b and TR14 were added for exactly the two GL-thread paths. AM-30 makes
it a rule for every later trim.
45 rows: ST 10, UN 8, RE 9, GA 10, SC 8.

COMPLETENESS FIXES (fix round). The check: .harmony/.reports/s-rta-1003b/review-ruling-transport-complete.md -- no
MUST, 3 SHOULD, 4 NIT. Each finding verified against this file and, where it names source, at 34179a2 (R37-R40).
- F1 ACCEPT. S1's cases named `restartStamp` (S2) and `clipBpm` (S4a); neither exists at S1 (R37). The clauses move,
  not the field: TL-UC2b and TL-UC16 get their stamp clause in S2, TL-UC3b its clip-BPM clause in S4a (5.1, section
  4; MU-1, MU-28, MU-29; K14). Not taken: minting the stamp in S1 -- that edits the tail in Layer.h, which is S2's
  work; nor three new case ids in S2 / S4a -- G-U1 would see those by id, but the pin of 54 would move, and the
  three mutants give the same proof on the lane head. Same class, found by sweeping every stage's exit line: TL-L1
  named `scrubClip`, which comes with S3 (its S2 form is written in 5.1), and the TSan pin was one number for three
  stages (G-U4).
- F2 ACCEPT. G-U1b, G-U2, the 5.4 re-runs and B1-B9 now say they are guards; G-U5 and k10 (ii) had a RED arm that
  was not written (5.2, 5.4, 5.5). TR7b, TR7c: labelled, and their teeth made true (R40) -- as written MU-11 was not
  sure to turn TR7b RED (the playhead was never placed; the right half holds under it), and MU-10 turns only TR7c's
  OneShot variant RED.
- F3 ACCEPT. S3h: TL-U11, TL-U11b, TL-U11c [tsan], TL-U12, TL-U12b, TL-U12c, MU-H1 .. MU-H3 and live row TR8
  (5.1, 5.3; AM-12 cites them). S2b: TL-U6b, the two probes whole, FM-1 again, and the STOP if it is still over its
  bar (5.1, 5.6; AM-21). Pins: G-U1 54 + 1 (S2b) + 6 (S3h); G-U4 7 + 1 (S3h); EXPECTED_ROWS 17 + 1 (S3h).
- N1 ACCEPT. TR13a reads `reveal_target` (R38); the ASSUMED clause and its fallback are gone.
- N2 ACCEPT. AM-27: the "--" reading is Harmony's to sign; the other way (an empty, disabled box) is specified.
- N3 ACCEPT. After a refused Redo the menu's item stays named and enabled (R39). Accepted for this lane and said
  (AM-7, K15); the Edit-menu lane gets it in section 4.
- N4 ACCEPT. Boris's "restart" answered Harmony's question "continue where it left off (today) or restart?"
  (boris-feedback-backlog.md:200): it rules out "Continue", not "Relative". AM-18 and K8 reworded; the SC-6 row
  stands (the removal rests on R27 and H-4).
REJECT: none.

## 3 AMENDMENTS (each OVERRIDES the plan body where they differ)

AM-1 The tail never runs for a transition that only queues (RE-1, ST-4; F2 kept, SC-4 rejected).
- `Layer::activate`'s hook (Layer.h:572-576) and its post-step (:577-579) run `applyActivationTail` only when the
  resulting tuple has `ref` active AND does not hold `ref` in the pending slot. `triggerClip`'s queue test loses
  `&& ref != r.activeRef()` (F2). The release keeps running the tail (:484-487).
- So: a queued re-fire changes the pending slot and nothing else; a second re-fire while the first is queued changes
  nothing; the restart happens on the release.
- Momentary: a release of a pad whose clip is the active one clears the layer, as today, also when the press only
  queued a restart (the queue goes with the clear). Pinned by TL-U3c. The plan's table row says so.
- The plan's R4 list is replaced by R8. S0 re-lists the readers on the merged tree.

AM-2 The restart stamp (GA-6, UN-4, RE-7, SC-5). Every `restartSeq` in the plan reads `restartStamp`.
- `Relaxed<uint32_t> Clip::restartStamp` (runtime, not saved). The tail sets it from ONE process-wide counter:
  `static uint32_t Clip::mintRestartStamp() noexcept` (never 0; 0 = never fired).
- `ClipTransportSync::applyRestart(const Clip&, Player&)`: restart only when the clip's stamp is NEWER than the
  player's seen stamp (wrap-safe signed difference), then store it. An equal or an older stamp never restarts. A
  fresh player's seen stamp is 0.
- Consequences, to be written into the docs: a value copy of a Clip (a command, a take restore, Duplicate Deck, a
  composition load) can neither cause a restart nor swallow the next one; a newly opened player under a fired clip
  starts at the clip's start on its first drawn frame (a first fire; a direct Replace Content on a playing clip).
- `restart()` is called only from src/render and src/media (lint TL-L1b).
- For the builder: the tail runs once per compare-exchange attempt (Layer.h:338-349), so it can mint more than once
  in one call; only the last stamp matters. The tail's comment "idempotent" is corrected.

AM-3 F1 "every fire plays" stays as HARMONY'S DEFAULT, not Boris's word (RE-9, GA-5).
- In T6, the "Boris" cell of every row that depends on the play state reads "Harmony default; Q1 open". His
  "restart" backs only the position. Harmony signs the default in the lane report. Q1 is on Boris's page before the
  merge.
- Why the narrow rule (keep the pause) is not the default: R3 + R4. A clear, a momentary release and a OneShot end
  all leave `playing == false`, and the tail cannot tell those from a pause Boris made; under "keep" each of them
  turns the next fire into a frozen picture.
- FM-2 (frozen app, REST only) decides the fallback. Reads `playing == false` after clear + re-fire: F1 stands, and
  the reading is shown to Boris with Q1. Reads `true`: the dead fire does not exist as read; the lane ships the
  narrow rule until Q1 is answered -- the tail sets `playing = true` only for a clip never fired before (stamp 0) or
  a OneShot, and keeps the play state otherwise; TL-U1, TL-U2 and TR5b then change their expectation and say so.
- Round 2: `triggerWillAutoPlay` (MainComponent.cpp:192-198, R33) must say what the tail will do, in either form.
  F1: the target exists and is not playing (the plan's line). The narrow rule: and it was never fired (stamp 0) or is
  a OneShot. Else a take records a "resume" point for a fire that did not turn the clip on.
- `hasBeenTriggered` is deleted either way ("never fired" = stamp 0).

AM-4 A held key and a controller fire once (ST-3). In `BindingManager`: a keyboard binding fires on the first
key-down of a press; further key-downs of that key are ignored until its key-up (a held-key set; keys no longer down
leave it in `MainComponent::keyStateChanged`, next to its existing poll, which keeps firing release actions for
Momentary bindings only). A MIDI CC bound to Trigger Clip or Trigger Column fires when its value rises from 0. MIDI
notes unchanged. The plan's path table gains the row "a held key, a moving controller: one fire per press, per rise".

AM-5 A fire on a layer that is not drawn (ST-5). No code. The path table gains: "a layer that is bypassed, hidden,
or muted by another layer's solo: the fire lands at once in the model; the clip starts from its start on the first
frame the layer is drawn again". TL-U5c; Boris check 21; docs.

AM-6 ONE landing rule for a clip value landing on a cell (ST-1, UN-1, UN-2, UN-4).
- `enum class CellLanding { Applied, AppliedKeepingTransport, LeftLive, LeftDuplicate };`
  `CellLanding landClipInCell(std::optional<Clip>& cell, const std::optional<Clip>& state, bool cellIsLive,
  const std::function<bool(uint32_t clipId)>& idHeldByAnotherCell);`
  Used by SetClipCmd, SwapClipsCmd and ClearLayerClipsCmd (cell by cell: never a whole-row assignment).
- The rule, in order:
  1. The cell is live and the state is empty, another clip id, or the same id with OTHER media: LeftLive. Nothing
     is written; no media hook; no dispose.
  2. The state's clip id sits in a cell this command does not write: LeftDuplicate. Nothing is written.
  3. The cell is live, same id, same media: the state lands and the live clip's transport is put back --
     AppliedKeepingTransport.
  4. The cell is idle: the state lands whole, as today.
- `bool Clip::sameMedia(const Clip&) const`: mediaType, mediaFile, cameraDeviceIndex, sourceType, sequenceFiles.
- `Clip::TransportState Clip::transportState() const; void Clip::setTransportState(const TransportState&);` holds
  `playing`, `playheadPosition`, `beatsPlayed`, `restartStamp` (from S2), `transportMode`, `loopMode`, `speed`,
  `reverse`, `inPoint`, `outPoint`, `sequenceFps`, `clipBpm` (from S4a). It replaces `carryLiveTransportFrom`.
- SwapClipsCmd: when either landing would be LeftLive or LeftDuplicate the whole command is left (both cells, the
  column count). ClearLayerClipsCmd and LayerClipsSnapshot lose the tuple (plan, unchanged).
- The rule filters Undo and Redo only. A command's FIRST execute (`UndoManager::perform`) is Boris's own action and
  is never filtered.
- Disposal after an Undo or Redo landing: the dispose hook gets the command's recorded leaving clip (as today) AND
  the clip the cell really held before the landing -- a skipped step can make the recorded one untrue. Nothing is
  disposed for a landing that was left. The hook's own re-scan of the model (MainComponent.cpp:5260-5266) stays
  the last word.
- Every write of `Deck::numColumns` by an Undo or a Redo (SetColumnCountCmd, SwapClipsCmd) is floored at the
  highest live column of that deck + 1 (the plan gave this floor to SetColumnCountCmd only; SwapClipsCmd restores a
  column count too, ClipCommands.h:226).
- F13 "live" now reads: the layer's tuple (active clip, previous clip while its fade runs, queued clip) and, for
  those clips, WHICH MEDIA plays and its whole transport (play state, playhead, direction, speed, loop mode, in / out
  points, mode, clip BPM). A step whose own change was one of those on a live clip is spent without effect.

AM-7 A Redo that would remove a live layer or column is REFUSED, not spent (UN-3, plus my own finding).
- `virtual bool Command::lastRunRefused() const noexcept { return false; }`. `UndoManager::redo()` leaves the index
  where it is and returns false when the command refused (today it advances unconditionally, R18).
- RemoveLayerCmd and RemoveColumnCmd refuse -- nothing changes, nothing is disposed -- when they run as a Redo over
  a live layer / a column a tuple names in that deck. Their first execute never refuses.
- CompositeCommand reports a refusal of a child; both refusing commands are pushed alone (R19); a jassert in
  CompositeCommand keeps it so.
- Why not "spent" with a flag (the seat's fix): later steps name rows by INDEX (R17). Remove Layer and Remove
  Column take the last one (R19), so a spent Redo shifts nothing by itself -- but it leaves the stack one layer
  taller than every later step was recorded on. Reachable today (INFERRED from R17, R19; not run): 4 layers, layer
  3's row holds clips Z0 and Z1. Remove Layer; Load Deck of 5 rows (it adds two layers at rows 3 and 4); on the
  first deck drop Y into row 3, column 1; undo all three; fire Z0. Redo three times: the spent Redo keeps layer 3,
  the Load Deck appends its layers at rows 4 and 5, and Y lands on row 3, column 1 -- over Z1, which no step
  recorded. A refused Redo leaves the later steps unreached; a new edit clears them as usual.
- The Undo direction needs no refusal: every collision there is "the live part stays, the step is spent", and it
  cannot shift a row or column number -- AddLayerCmd and InsertDeckCmd append their layers at the end (R19),
  SetColumnCountCmd shortens only the tail, and the other Undos only put things back.
- AddLayerCmd and InsertDeckCmd find their added layers by layer ID (the plan said so for InsertDeckCmd only).
- Round 2: a refused Redo is SILENT -- no text, no alert (AM-27). The call sites already do nothing on `false`
  (R33). Fix round (R39): the menu's item still reads "Redo <step>" and stays enabled -- `canRedo()` is the stack's
  position. That is the current state, not an event; it is also a control that looks usable and does nothing while
  the layer plays. ACCEPTED for this lane, knowingly (K15). Greying it while its step would be refused is the
  Edit-menu lane's choice (section 4); a disabled item is a state (AM-27).
- F14 and F16 read accordingly.

AM-8 ONE "live" test; no Undo or Redo writes a tuple (UN-5, GA-7).
- `bool Composition::deckIsLive(uint32_t id) const` = a layer's active ref names the deck, or its previous ref while
  `crossfadeProgress < 1`, or its pending ref. `retireOrEraseDeck` and `reapRetiredDecks` use it. The plan's
  `layerIsLive`, `cellIsLive`, `highestLiveColumn` test the same three slots.
- AddDeckCmd::undo and InsertDeckCmd::undo lose `cancelPendingInto` (DeckCommands.h:727, :854): a deck a queued
  trigger names is retired and the trigger fires from it on its beat (R20). InsertDeckCmd's "anything live" is
  `deckIsLive(its deck) || an added layer is live`.
- The ONE tuple write left in a command: RemoveDeckCmd::execute keeps `cancelPendingInto` (:946). Remove Deck, done
  or redone, cancels a trigger still queued into that deck (ruling-bf9b amendment 4(c), unchanged). F13 says so.
- Lint L2 also bans, in src/core/*Commands.h, `updateRuntime(`, `clearTupleForRow(`, `triggerClip(`,
  `clearActiveClip(` inside any Command class, and allows `cancelPendingInto(` only in RemoveDeckCmd::execute.
- TL-UC14: a table test over EVERY Command class (E19), each run through undo and redo on a show with an active, a
  fading and a queued clip: every tuple and those clips' transport are bit-identical, except the one write above.

AM-9 F18 stays, asked and gated (UN-6, GA-8). When anything of a Load Deck is live at Undo, the deck is retired and
ALL the layers it added stay, idle ones included. Labelled ASSUMED against Boris's "It changes anything else"; Q8.
TL-UC12b pins Redo after the retired deck was reaped while its layers were kept.

AM-10 Q3 is widened; its default is today's behaviour (ST-10, UN-7). The plan's line -- effects, bypass, solo,
layer order of a playing layer are still undone -- is a reading of "anything that is live in the layer strip",
labelled ASSUMED. If Boris answers "leave them alone": ToggleLayerFlagCmd (Bypassed, Solo) is spent without effect
on a live layer (about ten lines, one unit case). Layer ORDER is different: a spent Move Layer shifts row numbers
under older steps (R17), so it could only be refused, which blocks Cmd+Z behind it; that needs its own small plan.

AM-11 The drag is built WITHOUT a hold (SC-2; RE-3's first half). This replaces the plan's F6 choice, `scrubHeld`,
the `pushIntent` / `writeBack` change, `ScrubPhase`, `POST /api/debug/scrub`, U11, U12, TR8, MU-T3b.
- ONE function `void MainComponent::scrubClip(uint32_t clipId, double normalized)`: finds the clip by id, clamps to
  [inPoint, outPoint - 1e-4], requests the seek on its player or sequence FIRST (the only `seekTo` in
  MainComponent.cpp, lint TL-L1), then writes the model playhead. `onCuepointJump` goes through it.
- `std::function<void(uint32_t clipId, double normalized)> LayerStrip::onScrub`, forwarded by DeckView, assigned in
  MainComponent. `mouseDown` keeps the id of the layer's playing clip; `mouseDrag` scrubs THAT id and does nothing
  once the layer plays another clip; `mouseUp` forgets it. No direct playhead write in LayerStrip.cpp.
- ClipInspector: its handlers call the same callback with `clip_->id`; a marker is grabbed only on its drawn tab
  (ClipInspector.cpp:1285-1314 per the plan), the rest of the bar scrubs.
- While the mouse is down the clip keeps playing from each drag position, as the Clip tab does today. No seek on
  mouseUp: after a drag the clip is already playing from the last position.
- Test hooks in the pattern of `DeckView::clickTabForTests` (R16): `void LayerStrip::scrubForTests(float normalized,
  int phase)` and `void ClipInspector::timelinePressForTests(int xInBar, int yInBar, int phase)` run the bodies of
  the real mouse handlers; test-mode routes `POST /api/debug/strip_scrub {layer, pos, phase}` and
  `POST /api/debug/clip_timeline {x, y, phase}` call them on the live components (phase 0 down, 1 drag, 2 up).

AM-12 The hold, ONLY if Boris answers Q2 "wait while I hold" -- stage S3h (RE-2, RE-3, GA-2).
- Not a Clip field. One process-wide `std::atomic<uint64_t>` record = (clip id, the clip's restart stamp at the
  press); 0 = none. One mouse, one hold.
- `struct ClipTransportSync::Intent { bool wanted; bool held; };` `pushIntent` reads the record ONCE, pushes
  `wanted && !held`, returns the Intent; `writeBack` takes it and never reads the record: while `held` it skips the
  play-state compare-exchange and the OneShot stop.
- `held` = the record names this clip AND the clip's stamp equals the recorded one: any fire of the clip ends the
  hold. Up, a new Down, `setClip`, a destroyed strip or inspector clear the record.
- Cases (fix round: ids reserved in 5.1, "Conditional stages"): TL-U11b the hold cleared between `pushIntent` and
  `writeBack` leaves `clip.playing == true` (the in-window pattern of tests/test_clip_transport_sync.cpp:35);
  TL-U12b "a fire during a hold ends it"; TL-U12c "the layer's playing clip changes mid-drag: no clip is left
  held"; the plan's U11 and U12 return as TL-U11 and TL-U12, read against the record, not `scrubHeld`; TL-U11c
  [tsan] is the record's race case. Live row TR8 (5.3) with an in-point of 0.3: Down 0.62; four reads over 1 s
  within one frame of 0.62, `playing` true; the read 0.5 s after Up > 0.64.
- If S3h is built, two lines read differently: AM-11's fourth bullet -- "while the mouse is down the picture waits
  on the frame under it"; Boris check 7's second sentence -- "If you keep the mouse down and hold still, the picture
  waits on that frame until you let go." TL-L1, the two test routes and TR7's bars do not change.

AM-13 F7 said truthfully (ST-2). The clamp stays. Struck: "so no drop can be sent elsewhere". Written instead: a
drop at or past the out marker is a drop at the END of the range -- on the next advance a Loop is at its start (so
is a trimmed PingPong, R9) and a OneShot has ended; a drop before the in marker lands on the in marker. TL-U15 is
timed. TR7c. Q10.

AM-14 `RelaxedFloat Clip::clipBpm = 120` (RE-5). A static_assert pin in tests/test_shared_field_types.cpp; TL-U19b
[tsan]: a message-thread write against the GL-side rate read. Save / load use `.load()`. The Beats box and REST
write only this field.

AM-15 S4a is additive (SC-1). `beatDivision` and `videoBeats` stay in the struct, saved and loaded as today,
through S4a: the renderer stops reading them, the old widgets still write them (dead for one stage, never merged
like that). S4b deletes both with the widgets and changes, in the same stage, the tests of R25 -- added to T6:
- tests/test_clip_inspector_paint_key.cpp:62 "beat division" -> the two new texts and the buttons' enabled state;
- tests/test_composition.cpp:224-225, :274-275 -> the round trip of `clipBpm`; :548-549 -> default `clipBpm` 120;
- tests/test_undo_commands.cpp:116, :344-345 -> the clip-equality helper and its fixture use `clipBpm`.
The "no legacy key after a save" half of U18 moves to S4b (TL-U18d).

AM-16 Old shows, said truthfully (ST-7). The conversion of the plan stands. The plan's check 10 and TR10 change:
at a tempo of 120 every old BPM-synced clip moves as before; at another tempo an old BPM-synced VIDEO now follows
the tempo (it ignored it), a sequence behaves as before. Boris: "If it is BPM synced, regardless of the length, it
is synced to the current playing BPM".

AM-17 The widgets (ST-8, GA-9).
- Reverse stays reachable in BPM Sync (the renderer applies `reverse` in both modes, Renderer.cpp:1647-1648): the
  BPM row carries the Reverse button at the bounds it has in the Speed row.
- Bar B2: "/2" and "x2" on the Beats row have the same x, width and height as the Speed row's pair (+/- 1 px).
- What "x2" means is Q7; the default is the plan's (it doubles the beats, so the clip takes twice as long).
- `const juce::StringArray& ClipInspector::paintedTransportStrings() const`: every string the Transport section
  paints, refreshed by each paint in test mode only; the dump route reports it. B3 and B7 are read from it.
- B1 is measured against the viewport (section 5.5). V6 has a fixture. A state that cannot be captured is BLOCKED.
- One more test route so no state needs synthetic input: `POST /api/debug/clip_transport_do {action, text}` with
  action = type_bpm | type_beats | half | double | reset_bpm | reset_beats, calling the widgets' own handlers.

AM-18 Scope of T4's extras (SC-6; rewritten in round 2).
- REST, by one test: a field is added only when a gate row reads or writes it. `/api/composition` gains per clip
  `transportMode` (the read-back of a `set_clip_param` write, which answers "ok" before it lands, R30), `clipBpm`
  (TR10, TR13b, B4, B9), `inPoint` and `outPoint` (TR7b, B4) -- not `speed`, `loopMode`, `reverse`.
  `/api/set_clip_param` gains `transportMode` and `clipBpm` -- not `beats`.
- The bar's beat lines: the loop of R29 fed with the derived beats instead of `beatDivision` -- one line at each
  whole beat b with 1 <= b < beats - 0.001, at inP + (outP - inP) x b / beats; drawn only in BPM Sync, only when the
  length is known and 2 <= beats <= 64. Nothing else of the bar's painting changes. The count is a pure function
  beside the beats math, `int beatLineCount(double beats)`, written in S4a and pinned by TL-U17; S4b's paint uses it.
- The inert combo (R27) is removed in S4b under the visual gate: its three items, its member, its bounds. The Loop
  dropdown keeps its bounds (the right half of that row stays empty); if the visual-design seat raises the gap as a
  MUST, the fix is the Loop dropdown taking the rest of the row. Precondition: the builder's grep shows no other
  reference to `triggerDropdown_` in src or tests. Why here, against the seat: after S2 one of its three choices
  ("Restart") is what the app does; "Continue" is what Boris ruled out ("restart" -- his answer to Harmony's
  question "continue where it left off (today) or restart?", boris-feedback-backlog.md:200); "Relative" names
  nothing the app does, and his answer does not reach it (fix round: that part is a reading, not his word). It sits
  in the section this lane's own UX seat is asked about (a control that looks usable and does nothing); no ruling
  gives it to another lane (R27).
  Boris asked for no such removal: note N2 goes on his page (section 7). Decision H-4 is Harmony's.
- The bars ruling's relabels are NOT voided here: the words "are void" of plan 4.2 are struck. Decision H-1.

AM-19 Unit gates by NAME (GA-1). Every new case is a TEST_CASE (never only a SECTION) whose name begins with its
id from section 5.1 ("TL-U1 ...", "TL-UC3 ..."). G-U1 selects by `^TL-` with a pinned count; the touched binaries
are also run whole, by path.

AM-20 Live rows re-registered (ST-6, RE-4, GA-3, GA-4, GA-10, UN-8, SC-3): section 5.3 is the list. In short: TR1
cut; TR7 through the real handlers, plus TR7b (the ends of the bar) and TR7c (past the out marker); TR8 out with the
hold; TR13 becomes TR13a (replace on the PLAYING cell; the frozen app fails it) and TR13b (an effect edit on the
playing clip; teeth = MU-18); TR5b (fire after a clear); TR6 by injected beats, judged by order; TR6b and TR14 for
the two GL-thread paths; RED strings of TR2, TR3, TR12 corrected; tolerance 0.5 s, this probe's own number; no
automatic INFO downgrade. Paths with no live row are named in section 5.3 with what proves them.

AM-21 The stale frame after a re-fire (RE-6). Measured once after S2 (FM-1). Within the bar: nothing changes. Over
the bar: stage S2b -- a restart of a player whose shown frame is not the restart frame makes the player PENDING until
a frame of the restart generation is up; the compositor's pending branch then holds the layer's last picture
(CompositorEngine.cpp:1122-1136). S2b touches Pitfall 56's "a hold is not pending". Its gate (fix round): unit case
TL-U6b (5.1, "Conditional stages"); `.harmony/probe-video.sh` and `.harmony/probe-media-open.sh` run whole
(`PROBE-VIDEO GREEN`, `PROBE-MEDIA-OPEN GREEN`); FM-1 measured again, and a STOP if it is still over its bar (5.6).

AM-22 Rate, wobble, drift (RE-8). F9 stands. TR9 stays the gate for the formula.
- The plan's F19 store ("the last value above 0"): if S0 confirms that a typed, tapped or Link tempo reports
  LOCKED, the store keeps the last value read while the tracker was LOCKED -- so a tracker that is still locking
  cannot swing the speed (TL-U16 covers it). If S0 cannot confirm it, the plan's store stays.
- FM-3 measures the tempo's wobble while locked: over the bar, the rate is slewed (at most 2 % per second) -- a
  small S4a follow-up with its own unit case; within the bar, nothing.
- FM-4 measures the drift over five minutes: it is reported to Boris with Q9; no build change without his answer.

AM-23 Plan section 7 check 3 and the path table (ST-9): "with no steady beat the app cannot wait for a bar: a
re-fire restarts at once" is stated in both; the check names its preconditions (section 6, check 3).

AM-24 The S0 list grows (section 4).

AM-25 T6 corrections: the retrigger SECTION's assert is at tests/test_layer_runtime.cpp:293; k10 (ii) becomes
`abs(t - el2) <= resumeTol` (its ramp's in-point is 0); the rows of AM-15 are added; every "Boris" cell that leaned
on "restart" for the play state reads as AM-3 says.

AM-26 The inert Duration row cannot stay beside the new Beats row (my finding, round 2; plan 4.4's "Either order
is safe" is wrong).
- R28: in BPM Sync that row is PAINTED "Beats" and has its own "/2" pair. With AM-17 the BPM row takes the Speed
  row's place, so the new Beats row belongs exactly where the dead one is. Left alone, the Clip tab shows two rows
  named Beats, one of them dead, each with a "/2" pair.
- S0 records whether the row is still in the tree. If it is, S4b removes it as ruling-bf7 AM4 lists
  (ruling-bf7.md:254-265): the members `durationSlider_`, `durHalfBtn_`, `durDoubleBtn_`, their constructor code,
  their layout block and its y advance, the painted label and its ty advance; paint() and resized() change together;
  any height total that counts the row loses one row; the same precondition (the builder's grep finds no other
  reference in src or tests). If the bars lane has landed first there is nothing to do.
- Decision H-2 is Harmony's: the bars lane's item I13 is then done here and struck from its list. Note N1.
- Bar B3 fails on a second "Beats" and on any "Duration" (section 5.5).

AM-27 No event text, no failure text. Boris, 2026-10-03: "ok. the only fail message will be a failed save. remove
all others".
Harmony constraint: this lane adds no on-screen text that announces an event or a failure. A display of a current
value is not such a text (a number box, "--" for a length not known, a disabled button, the menu's Undo and Redo
item names).
- Silent, each of them: a refused Redo (AM-7); a step that is left, spent or done in part (AM-6, AM-9); a drop
  clamped to the range (AM-13); a typed value clamped to its range (the box then shows the clamped number); "/2" or
  "x2" at its limit (disabled; nothing happens); a fire on a layer that is not drawn (AM-5); a held key (AM-4); an
  old show whose BPM-synced video now follows the tempo (AM-16).
- No stage adds a call of `setFileLabel(`, `AlertWindow`, `showMessageBox`, `showOkCancelBox` or `setNotice(`
  (R34), and no caption or tooltip of this lane changes with an event. Gate G-N1 (5.2); bar B7 (5.5).
- This lane removes and re-words NO existing event or failure text: the two save-failed alerts (facts-notices.md
  B06, B08) and every other such text are the notices lane's work. The fire handlers' label writes (R34) stay as they are.
- Round-2 reading of sections 3-7 against his words: no line added such a text. What was unsaid is said here, in
  AM-7 and in Boris check 23.
- Fix round: the "--" is the one line here that a strict reader could call a failure sign -- it stands for a length
  the app does not have (V6's fixture is a missing file). This ruling reads it as the state of the box: no value.
  Harmony signs that reading in the lane report. The other way, specified: the Beats box is empty and disabled, and
  TL-U20's last clause and V6 read "an empty, disabled box".

AM-28 The visual gate (SC-7).
- Five seats stay. Harmony constraint (her dispatch of the plan, wf/transport-plan.js:31): "the questions for five
  critic seats (visual-design, UX, graphic-design, logic, interaction-logic)". The eleven states stay: bars B5, B6,
  B7, B8 read the dumps of V5, V10, V3, V11.
- What a number can decide is a bar, not a seat's question: B4 now covers every state with a known length, B9 is new
  (each BPM box shows what the model holds), B3 and B7 are tightened. The logic seat keeps the question no number
  answers (5.5).
- The critics' packet (RIG-RULES.md:19-20): the capture builder's manifest with model facts per state (the fixture's
  file length, `inPoint`, `outPoint`, `clipBpm`, `transportMode`); the BORIS block of plan T4, verbatim; and the
  list of what exists in the Transport section before the lane and is outside it -- the mode combo, the three
  transport buttons, the Loop dropdown, the "Speed" caption, the Speed slider and its pair (which read a division
  sign and "2", a multiplication sign and "2", R28), Reverse, the bar and its marker tabs; plus the inert Duration
  row or the inert combo wherever H-2 or H-4 is decided against its default.

AM-29 S1 may start before the sync-dial lane merges (SC-8). Harmony's choice (decision H-3); the default is the
order of section 4.
- S1 only. Its base is then 34179a2, where R17-R22 and R33 are its S0 rows, already read; before the first case the
  builder re-reads the push sites of AddLayerCmd and the path of each test binary.
- After bf2 has merged: the builder rebases the lane onto the merged main (conflicts are expected in
  MainComponent.cpp and in the route table of ApiServer.cpp, R36); S0's table is written in full; every S1 case and
  lint is run again on the rebased head; only then S2 starts.
- Not S2, S3, S4a, S4b (the row of SC-8 says why).
- Unchanged: the RED arm is frozen from the merged main; FM-2 runs on it before S2; every gate of section 5 runs on
  the lane head. Rig (RIG-RULES.md, B): at most 3 build lanes; a full ctest while another lane runs takes the ctest
  lock.

AM-30 What no later trim may cut (the scope seat's strongest point). The restart mechanism -- the tail mints the
stamp, the render thread consumes it before the intent is pushed (AM-1, AM-2) -- with TL-U1, TL-U5, TL-U5b, TL-U8,
MU-1, MU-2 and the live rows TR2, TR3, TR4, TR6b, TR14. R32 is why. These rows are no candidates for a waiver or a
downgrade: if one of them cannot be made GREEN the lane stops at S2 and reports. No fallback of this ruling (AM-3's
narrow rule, K1's fallback, a BLOCKED row elsewhere) removes one of them.

Plan forks after this ruling: F1 kept as Harmony's default (AM-3). F2 kept (AM-1). F3 kept with the stamp (AM-2).
F4, F5 kept. F6 replaced by "no hold" (AM-11). F7 kept, restated (AM-13). F8-F12 kept. F13 widened (AM-6) and made
one test (AM-8). F14, F16 amended (AM-7). F15, F17, F18, F19 kept.
Round 2 changes no fork; AM-29 only lets S1 start early inside the one lane (F17).

## 4 FINAL BUILD STAGES

ONE lane (the plan's F17 stands), started after the sync-dial lane (bf2) has merged (S1 alone may
start earlier: AM-29). One builder context per stage, in
this order. Every stage: its cases are written first and shown RED by name on the stage's base (the builder's log; a
case that never failed is struck from the list and reported), then GREEN at its head.

- S0 RE-BASE NOTES (no code). Re-read on the merged main and write the file:line table into the lane report:
  the two trigger handlers; the `onCuepointJump` wiring; the `apiServer_` wiring block; the ApiServer route table and
  the `/api/composition` writer; `FeatureSnapshot` (does bf2 add a second tempo, change what `bpm` means, or change
  which beat clock the GL-thread release reads -- BeatLead?); `Renderer::syncMedia`; the pinned counts of
  tests/test_render_thread_lint.cpp. Added by this ruling: `UndoManager::undo / redo`; every push site of
  RemoveLayerCmd, RemoveColumnCmd, AddLayerCmd (alone or inside a composite); the readers of the pending slot (R8);
  PerfState's restore path (does it write a tuple or `playing` directly?); ImageSequence's seek and PingPong leg;
  whether a typed or tapped tempo reports LOCKED; that still no REST route adds an effect to a deck clip as an Undo
  step (R22);
  the path of each test binary; every probe that fires a clip twice and expects it to resume.
  Added in round 2: whether the inert Duration row (R28) and the inert combo (R27) are still in the tree and whether
  the bars lane has landed (AM-26, AM-18); the line counts of G-N1's five strings at the lane's base (AM-27).
  Harmony constraint: freeze the RED arm -- a copy of the pre-lane main app bundle. Harmony runs FM-2 on it before S2.
  Proves: the table.
- S1 UNDO-LIVE. Plan T5 with AM-6, AM-7, AM-8, AM-9, AM-10 (default only), and T5's rows of T6.
  Files: core/ClipCommands.h, core/DeckCommands.h, core/TriggerCommands.h (deleted), core/Command.h,
  core/UndoManager.cpp, core/CompositeCommand.h, model/Clip.h (`sameMedia`, `TransportState`), model/Composition.h
  (`deckIsLive`), MainComponent.cpp (push sites, hooks), the test route `POST /api/debug/add_clip_effect {layer,
  column, effect}` (it calls the FX-drop handler of MainComponent.cpp:1013-1025, so it pushes the same SetClipCmd),
  tests. TL-UC16 (the walk) and TL-UC14 (the table) are written FIRST.
  Proves: TL-UC1 .. TL-UC16 except TL-UC4 -- TL-UC2b and TL-UC16 without their stamp clause and TL-UC3b without its
  clip-BPM clause: those fields come with S2 and S4a (5.1); lint TL-L2; the undo and show binaries whole; the ASan
  unit gate (G-U5: 11 cases).
- S2 RESTART. Plan T1 + T2 with AM-1 .. AM-5 (AM-3 in the form FM-2 decided).
  Files: model/Clip.h, model/Layer.h, render/ClipTransportSync.h, media/VideoPlayer.*, media/ImageSequence.*,
  render/Renderer.cpp (two calls), binding/BindingManager.*, MainComponent.cpp (handlers, keyStateChanged), tests.
  Proves: TL-U1 .. TL-U9b, TL-UC4; TL-UC2b and TL-UC16 gain their stamp clause (`TransportState` takes
  `restartStamp`); lints TL-L1 in its S2 form (5.1), TL-L1b, TL-L4; the TSan unit gate (G-U4: 6 cases); probe-boxes
  k10 re-registered. Then Harmony measures FM-1.
- S2b (only if FM-1 is over its bar): AM-21. Proves: TL-U6b (5.1); `.harmony/probe-video.sh` and
  `.harmony/probe-media-open.sh` whole; then Harmony measures FM-1 again (5.6).
- S3 DRAG. AM-11, AM-13. Files: MainComponent.cpp/.h, ui/ClipInspector.*, ui/LayerStrip.*, ui/DeckView.* (the
  forward), the two test routes, tests. ClipTransportSync.h and Clip.h are NOT touched.
  Proves: TL-U10, TL-U13, TL-U14, TL-U15; lint TL-L3; lint TL-L1 in its final form (5.1).
- S3h (only if Boris answers Q2 with the hold; any time after S3): AM-12. Proves: TL-U11 .. TL-U12c and MU-H1 ..
  MU-H3 (5.1); the TSan unit gate with one more case (G-U4); live row TR8 (5.3).
- S4a BPM ENGINE, additive. AM-14, AM-15, AM-16, AM-18 (REST, `beatLineCount`), AM-22's store.
  Files: model/Clip.h/.cpp, render/Renderer.h/.cpp, api/ApiServer.cpp, tests.
  Proves: TL-U16, TL-U17, TL-U18a-c, TL-U19, TL-U19b; TL-UC3b gains its clip-BPM clause (`TransportState` takes
  `clipBpm`); the TSan unit gate (G-U4: 7 cases).
- S4b BPM WIDGETS; the two old fields leave. AM-15's test rows, AM-17, AM-18 (the combo, the beat lines), AM-26 (the
  inert Duration row, if it is still in the tree), AM-27, AM-28.
  Files: ui/ClipInspector.*, MainComponent.cpp (the length provider), model/Clip.h/.cpp, the dump and "do" test
  routes, the tests of R25. Proves: TL-U18d, TL-U20. The stage ends at the VISUAL WORK GATE (5.5), not at a commit.
- S5 PROBE + DOCS. `.harmony/probe-transport.sh/.py/.json` (17 rows, 3 measurements), the docs of plan section 6
  with the wording of AM-2, AM-3, AM-6, AM-7, AM-8, AM-27, Boris's page (sections 6 and 7 here, the notes N1 and N2
  included), the lane report (G-N1's ten numbers in it).
  Proves: a full local run on the lane build and the RED table.

Re-read after the sync-dial lane merges: exactly S0's list. A stage that starts without S0's table builds on stale
lines.
What the other lanes get from this one (plan 4.2-4.4, amended):
- bars lane (bf7): a re-fire is queued like any fire and the tail runs at the release only (AM-1); the per-clip
  beat-phase seek is gone; decisions H-1 and H-2 below; a clip's length is typed in beats.
- Timeline lane (bf6): after a fire the model playhead is the start; `Clip::restartPosition()`, `restartStamp`,
  the length provider and `clipBpm`; there is NO `scrubHeld` -- if S3h is built the hold is AM-12's record, not a
  Clip field.
- Edit menu (BF33): `UndoManager::redo()` can return false with the step still redoable (AM-7); a spent Undo step
  leaves the menu like any undone step. After a refused Redo the item still reads "Redo <step>", enabled, and does
  nothing while the layer plays (R39): whether to grey it -- a `const` query of the refusing command, asked when the
  menu is built -- is that lane's decision.
- "Duration" row (plan 4.4): replaced in round 2 by AM-26 and decision H-2 -- "either order" is not safe.

Decisions only Harmony can take (round 2; each has a default, and the builder builds the default unless told
otherwise):
- H-1 before the bars lane builds: ruling-bf7 AM6's two ClipInspector relabels and their tooltips (R31). Default:
  withdrawn -- S4b deletes the two widgets they label, on Boris's answers 5-7 of 2026-10-03. If the bars lane lands
  first, S4b deletes the relabelled rows by member name (`beatDivisionSelector_`, `videoBeatsSlider_` and their
  labels), whatever their text reads by then.
- H-2 before S4b: the inert Duration row (AM-26). Default: S4b removes it if it is still in the tree, and the bars
  lane's I13 is struck. The other way -- the bars lane removes it before S4b starts -- is as good. Leaving it in
  place through S4b is not: two rows named Beats.
- H-3 before S1: S1 early (AM-29). Default: no; the lane starts after bf2 has merged.
- H-4 before S4b: the inert combo (AM-18). Default: removed in S4b, note N2. The other way: it stays, goes on the
  critics' list of what is outside the lane (AM-28) and on the ui-polish ledger, and B3 drops its last clause.

## 5 FINAL CONSOLIDATED GATE LIST (pre-registered; Harmony copies gate strings ONLY from here)

Harmony constraint: RED arm = the frozen pre-lane main app; a flake verdict needs >= 5 runs per arm; no on-screen
text announces an event or a failure (AM-27). A pre-registered bar is never loosened: it is met, or reported. A row whose bar turns out
to sit inside 4 x the measured noise is BLOCKED until Harmony waives it in writing -- never downgraded silently.

### 5.1 Unit cases (each a TEST_CASE whose name begins with the id; 49 cases + 5 lint cases; conditional stages: + 1, + 6)

S1 (21): TL-UC1 "a fire pushes no Undo step" | TL-UC2 "Undo of Clear Layer Clips puts the clips back and leaves the
tuple alone; Redo clears the idle cells and leaves a live cell" | TL-UC2b "Clear, Undo, fire X, Redo, Undo: X keeps
its play state and playhead" (S2 adds: its stamp) | TL-UC3 "a clip landing on a live cell: empty or another clip id leaves the
cell; the same id and media lands the edit and keeps the live transport" | TL-UC3b "an effect edit undone and redone
on a playing clip keeps its reverse, speed, loop mode, in and out points and mode" (S4a adds: its clip BPM) | TL-UC3c "Undo and
Redo of Replace Content on a playing cell leave its media, name and player; on an idle cell the media reverts" |
TL-UC5 "SwapClipsCmd with a live cell is left whole; with idle cells it swaps back" | TL-UC5b "Drop A, Move A, fire,
Undo, Undo, Redo, Redo: every clip id sits in exactly one cell" | TL-UC6 "Redo of Remove Column under a live clip is
refused: the index stays, the next Undo undoes the step before it, columns and ids unchanged" | TL-UC6b "a
column-count shrink by Undo or Redo stops above the highest live column" | TL-UC7 "Undo of Add Layer while the layer
plays keeps it and Redo adds nothing; idle: erased by id, Redo re-adds" | TL-UC8 "Undo of Remove Layer brings the
layer back with an empty tuple; Redo over a live layer is refused and later Redo steps stay unreached" | TL-UC9 "T6h
Undo of a Load Deck that added layers while its clip plays: the deck is retired, the same Clip keeps playing, the
added layers stay; Redo restores the deck under its id" (5 layers, 1 retired deck, 0 disposals; redo: 2 decks, 5
layers, no reconnect) | TL-UC10 "T6h-b nothing live: the deck and its added layers go, every cell disposed once" |
TL-UC11 "T6h-c an added layer plays another deck's clip: the added layers stay, the deck goes by its own test" |
TL-UC12 "T6h-d after a keep, older Remove Layer and Load Deck steps still resolve their own layers by id; rows equal
layers" | TL-UC12b "Redo of a Load Deck after its retired deck was reaped while its layers were kept: no layer added
twice; rows equal layers" | TL-UC13 [asan] "the Layer and Clip inspectors bound across a keep-undo" | TL-UC14 "no
command's Undo or Redo changes a tuple or the transport of an active, fading or queued clip (every Command class)" |
TL-UC15 "Undo of Add Deck and of Load Deck with a trigger queued into the deck retires it and the trigger fires; with
its clip fading out the deck is retired" | TL-UC16 "200 random Undo and Redo steps with fires between them, three
seeds: unique clip ids across live and retired decks, rows equal layers, every live ref resolves to a clip in a
shown column, no step changed a tuple" (S2 adds: every fire's stamp is newer than every stamp before it).
A clause in brackets tests a field S1 does not have (`restartStamp` comes with S2, `clipBpm` with S4a: AM-6). S1
writes the case without it. The named stage adds the clause to the SAME case -- the TEST_CASE name stays -- before
the code that satisfies it, and shows the case RED by its id in that stage's RED log (a clause that never failed is
struck and reported, like a case). On the lane head MU-1, MU-28 and MU-29 show that the three clauses are there.
TL-UC3 and TL-UC14 compare `transportState()` whole (`bool operator==(const TransportState&) const = default;`), so
they take a new field with the struct.

S2 (15): TL-U1 "a fire of the active clip plays it and mints a newer stamp" | TL-U2 "a new activation of a paused,
previously fired clip plays it" | TL-U3 "with a forced snap a re-fire of the active clip is queued: stamp, play
state and playhead stay until the release, which restarts it and leaves the fade alone" | TL-U3b "a second re-fire
while the first is queued changes nothing" | TL-U3c "a momentary release while a restart of the active clip is
queued clears the layer" | TL-U4 "a reversed clip restarts just inside its out-point; a PingPong restarts on its
first leg at the in-point" | TL-U5 "a newer stamp restarts the player once, before the intent is pushed; an equal or
older stamp never does" | TL-U5b "a fresh player under a fired clip starts at the clip's start" | TL-U5c "a stamp
minted while the clip is not drawn is applied on its first drawn frame, once" | TL-U6 "restart resets a PingPong to
its first leg" | TL-U7 "stamps come from one counter: a copied old clip state never makes a later fire's stamp meet a
seen one" | TL-U8 [tsan] "message-thread fires against the GL-side consume of the stamp" | TL-U9 "T4b a column fired
twice restarts every layer that plays it; the empty-cell layer stays cleared; the Ignore Column layer is untouched" |
TL-U9b "a held key fires its binding once; a CC bound to a trigger fires on its rise from 0" | TL-UC4 "render level:
Undo and Redo of an edit on a playing clip make no restart and no re-open".
tests/test_composition.cpp "Retrigger resets playhead" keeps passing.

S3 (4): TL-U10 "a model-only playhead write is undone by the write-back; a scrub seeks the player and the write-back
keeps the position" | TL-U13 "Clip tab: a press 3 px inside either end of the bar, below the marker tab, scrubs; a
press on a marker tab drags the marker" | TL-U14 "layer strip: a press in the transport bar scrubs the clip it was
pressed on; after the layer's playing clip changes the drag does nothing; no direct playhead write" | TL-U15 "a drop
past the out marker lands just inside it; two frames later a Loop is at its start and a OneShot has ended; a drop
before the in marker lands on it".

S4a (7): TL-U16 "the BPM Sync rate: 120 / 120 = 1, 90 / 120 = 0.75, 120 / 240 = 0.5; a show tempo of 0 uses the last
known one; 120 before any" | TL-U17 "beats and clip BPM through the length: 7.3 s at 120 is 14.6 beats; 16 beats is
131.5068 BPM; x2 and /2 are exact inverses and refuse at the limits; a trimmed clip uses its in..out length;
beat lines: 19 for 20 beats, 14 for 14.6, 1 for 2, none below 2 or above 64" |
TL-U18a "an old BPM-synced video 8 / 4 loads as clip BPM 60" | TL-U18b "an old BPM-synced sequence of 8 frames at
2.5 fps over 4 beats loads as clip BPM 75" | TL-U18c "an old Timeline clip loads as clip BPM 120" | TL-U19 "in BPM
Sync the speed pushed to the player is the rate and ignores clip speed and master speed; Timeline is unchanged" |
TL-U19b [tsan] "a message-thread clip BPM write against the GL-side rate read".

S4b (2): TL-U18d "load, save, load, save: the two saves are equal and hold no beatDivision or videoBeats key" |
TL-U20 "Clip tab: the BPM and Beats rows exist only in BPM Sync, the Speed row only in Timeline, Reverse in both;
typing Beats sets the clip BPM; a moved in-point changes the Beats text on the next refresh and nothing repaints
when nothing changed; right-click gives 120; no length gives --".

Lints (5, tests/test_render_thread_lint.cpp): TL-L1 "`seekTo(` occurs in src/MainComponent.cpp only inside
scrubClip (2); `hasBeenTriggered` and `syncActivatedPlayhead` occur nowhere in src" | TL-L1b "a player's `restart(`
is called only from src/render and src/media" | TL-L2 "in src/core/*Commands.h: `setRuntime(` only as
`setRuntime(LayerRuntimeSnapshot{})`; no `->playing =`, no `playheadPosition =`; `updateRuntime(` only inside the
bodies of cancelPendingInto and clearTupleForRow; `cancelPendingInto(` called only by RemoveDeckCmd::execute;
`clearTupleForRow(`, `triggerClip(`, `clearActiveClip(` called by no Command; `TriggerClipCmd` and
`ClearActiveClipCmd` nowhere in src" (counts pinned at S1's head; the old B4f counts re-pinned once) | TL-L3
"`onScrub` is assigned in src/MainComponent.cpp and forwarded in src/ui/DeckView.cpp; src/ui/LayerStrip.cpp has no
`playheadPosition =`" | TL-L4 "`Composition::fire` and `triggerColumn` are called only by the two trigger handlers;
`Layer::triggerClip` outside Composition.h only by Autopilot.cpp (3)".

TL-L1 by stage (fix round): `scrubClip` comes with S3. At S2's head the case pins `seekTo(` in src/MainComponent.cpp
at exactly 2 lines, both inside the `onCuepointJump` wiring, and `hasBeenTriggered` and `syncActivatedPlayhead`
nowhere in src (R37: of the 6 lines at 34179a2, S2 removes the four of the cell handler). S3 moves the two into
`scrubClip` and the pin becomes the text above. The TEST_CASE name stays.

Conditional stages (fix round). The ids are reserved; a case exists only if its stage is built, and AM-19 then binds
it like any other.
S2b (1): TL-U6b "after a restart the player is pending until a frame of the restart generation is up; a player that
only holds its last frame is not pending". RED arm: S2's head (the first clause fails there).
S3h (6): TL-U11 "while a clip is held its player is paused and the clip's play state stays true; after Up it plays on
from the held position" | TL-U11b "a hold cleared between pushIntent and writeBack leaves the clip playing" |
TL-U11c [tsan] "message-thread Down and Up against the GL-side read of the hold record" (RED arm: the record declared
as a plain `uint64_t`, builder's log) | TL-U12 "a OneShot held at its last frame is not stopped by the hold" |
TL-U12b "a fire of the held clip ends the hold" | TL-U12c "the layer's playing clip changes mid-drag: no clip is left
held".
Mutants of S3h (G-U3's rule): MU-H1 `writeBack` reads the record itself -> TL-U11b. MU-H2 Up does not clear the
record -> TL-U11 (live: TR8). MU-H3 `held` ignores the stamp -> TL-U12b.

T6 (expectation changes) = the plan's table with AM-3, AM-15, AM-25. Nothing else changes an expectation.

### 5.2 Unit gates (Harmony, on the lane head)

- G-U1 `ctest --test-dir build -R '^TL-' --no-tests=error --output-on-failure` -> `100% tests passed, 0 tests failed
  out of N` with N >= 54, AND each of the 54 ids of 5.1 appears as Passed in the output. Extra TL- cases are listed in
  the lane report. RED arm: the builder's per-stage RED log, by id (S2's log also lists TL-UC2b and TL-UC16, S4a's
  TL-UC3b: the clauses they gain, 5.1). A conditional stage adds to the pin and to the id list, never replaces them:
  + 1 if S2b is built (TL-U6b), + 6 if S3h is built (TL-U11 .. TL-U12c) -- N >= 55, 60 or 61.
- G-U1b each touched binary run whole BY PATH, exit 0: test_layer_runtime, test_layer_runtime_race,
  test_clip_transport_sync, test_show_model, test_undo_commands, test_composition, test_clip_inspector_paint_key,
  test_layer_strip_transport_view, test_render_thread_lint, test_shared_field_types, and any new binary. The lane
  report gives each binary's `--list-tests` count next to its pre-lane count. GUARD on both arms, no RED arm (the
  binaries pass before the lane too); the two counts keep a binary from losing cases unseen.
- G-U2 full `ctest` -> 0 failed; the lane report gives three numbers: cases before, cases deleted (TriggerClipCmd,
  ClearActiveClipCmd), cases added. GUARD on both arms, no RED arm.
- G-U3 mutants, each applied alone to the lane head, turns the named case RED (builder's table; Harmony re-runs
  five of her choice, always MU-4, MU-18, MU-21):
  MU-1 the tail mints no stamp -> TL-U1, TL-UC16 (live: TR2). MU-2 `applyRestart` after the intent is pushed -> TL-U5.
  MU-3 the queue test keeps `ref != activeRef` -> TL-U3. MU-4 the tail runs for a queue-only transition -> TL-U3,
  TL-U3b (live: TR6). MU-5 the tail keeps the first-activation condition -> TL-U2 (live: TR5b). MU-6 the consume
  rule is "differs", not "newer" -> TL-U7. MU-7 `triggerColumn` skips a layer already on the column's clip -> TL-U9
  (live: TR2). MU-8 `scrubClip` skips the seek -> TL-U10 (live: TR7). MU-9 `onScrub` not assigned in MainComponent
  -> TL-L3 (live: TR7). MU-10 no clamp -> TL-U15 (live: TR7c). MU-11 the 8 px marker rule kept -> TL-U13 (live:
  TR7b). MU-12 rate = clip BPM / show BPM -> TL-U16 (live: TR9). MU-13 beats use the whole file -> TL-U17.
  MU-14 the old-show conversion always gives 120 -> TL-U18a, TL-U18b (live: TR10). MU-15 the refresh sets a box
  every tick -> TL-U20. MU-16 a fire still pushes a step -> TL-UC1 (live: TR11). MU-17 ClearLayerClipsCmd writes
  the tuple -> TL-UC2, TL-UC14, TL-L2. MU-18 no transport put back on a same-clip landing -> TL-UC3, TL-UC3b
  (live: TR13b). MU-19 a landing with other media is applied on a live cell -> TL-UC3c (live: TR13a). MU-20 no
  duplicate-id test -> TL-UC5b, TL-UC16. MU-21 a Redo of Remove Column / Remove Layer over a live target is spent
  -> TL-UC6, TL-UC8. MU-22 AddLayerCmd ignores the live test -> TL-UC7. MU-23 InsertDeckCmd erases by position ->
  TL-UC12. MU-24 the previous clip of a running fade is not live -> TL-UC15. MU-25 AddDeckCmd::undo cancels the
  queue -> TL-UC15, TL-UC14. MU-26 `clipBpm` a plain float -> TL-U19b under TSan and the static_assert. MU-27 a
  held key re-fires -> TL-U9b. Fix round: MU-28 `TransportState` leaves out `restartStamp` -> TL-UC2b. MU-29
  `TransportState` leaves out `clipBpm` -> TL-UC3b (live: TR13b). If S3h is built: MU-H1 .. MU-H3 (5.1).
- G-U4 `.harmony/probe-tsan-unit.sh` -> exit 0 with `EXPECTED_TSAN_CASES` raised from 5 to 7 (TL-U8, TL-U19b). By
  stage: 6 at S2's head, 7 at S4a's; one more from S3h if it is built (TL-U11c: 8 on the lane head). RED: the stamp
  declared as a plain `uint32_t` makes TL-U8 report a race (builder's log).
- G-U5 `.harmony/probe-asan-unit.sh` -> last line `PROBE-ASAN-UNIT GREEN (11 cases, 0 reports)` (10 -> 11: TL-UC13).
  RED arm: with the pin at 11 the script exits 3 on a tree that has 10 cases (R40), and TL-UC13 is in S1's RED log
  by its own assertion that both inspectors are still bound to the kept layer and its clip (INFERRED: on S1's base
  that Undo erases the layer). The "0 reports" half is a GUARD: no ASan report is claimed for the base.
- G-N1 no new event or failure text (Harmony constraint; AM-27). For each of the five strings `setFileLabel(`,
  `AlertWindow::`, `showMessageBox`, `showOkCancelBox`, `setNotice(`: the number of lines under src that hold it
  (`git grep -F -c '<string>' <commit> -- src`, summed over the files) at the lane head is <= the number at
  `git merge-base <lane head> main`. The lane report prints the ten numbers (at 34179a2 the five read 46, 22, 13, 3,
  6). A guard on both arms; teeth: the builder shows once that one added `setFileLabel(` line turns it RED.

### 5.3 Live rows -- `.harmony/probe-transport.sh`

Final line `PROBE-TRANSPORT GREEN`; row-count pin `EXPECTED_ROWS=17` (18 if S3h is built: TR8 below; S2b adds no
row); `PROBE-TRANSPORT BLOCKED <n>` when a bar had no driver.
Launch rules (Harmony constraint): `--test-mode`, `open -g`, never an Output window, no synthetic OS input,
quit by own pid. Fixtures: probe-video's frame-coded ramp clips (30 fps, 10 s; decoded pixels give the frame number),
in-point 0.5 unless a row says otherwise. "t" = content time decoded from pixels. "el" = seconds from the fire's HTTP
reply to the capture. Tolerance tol = 0.5 s (this probe's own number, equal to k10's `resumeTol`). The lane report
states the noise of t and el measured over 5 runs.
- TR2 column_refire_restart. Two layers, column 0 = two ramps. `trigger_column`; 2 s; `trigger_column`; at el ~ 0.3:
  the top layer's pixels |t - (5.0 + el)| <= tol, and both layers' REST playheads in [0.5, 0.5 + (el + tol) / 10].
  RED arm: t ~ 2.3 s, playheads ~ 0.23 (a first fire starts at file position 0 and the column re-fire seeks nothing).
- TR3 replaced_refire_restart. Fire A; 2 s; fire B on the same layer; 1 s; fire A; capture at el ~ 0.3.
  Bar: |t - (5.0 + el)| <= tol. RED arm: t ~ 2.3 s (A resumes where it left).
- TR4 first_fire_at_start. A never-fired clip; fire; capture at el ~ 0.3. Bar: frame code in [150, 174].
  RED arm: code < 30.
- TR5 ended_oneshot_refire_plays. OneShot ramp, out-point 0.7; fire; wait (<= 10 s: the old app starts at file
  position 0) until `/api/composition` shows
  `playing` false; fire; at el 0.5: `playing` true and two playhead reads 0.2 s apart differ by >= 0.01.
  RED arm: `playing` false (INFERRED; if the RED arm passes, the row is reported as a guard).
- TR5b cleared_refire_plays. Layer 0: column 0 a ramp, column 1 empty. `trigger_clip(0, 0)`; 1 s;
  `trigger_column(1)` (the empty cell clears the layer); 0.5 s; `trigger_clip(0, 0)`; at el 0.5: `playing` true, two
  playhead reads 0.2 s apart differ by >= 0.01, |t - (5.0 + el)| <= tol. RED arm: FM-2's reading (expected
  `playing` false, INFERRED). If AM-3's fallback is taken the row expects what the narrow rule gives and says so.
- TR6 quantized_refire. A ramp whose own Snap is Bar, in-point 0.2 (probe-boxes k5's driver: injected beats).
  Fire; inject the downbeat (it plays); inject beat 1; fire again; then: `pendingClip` equals the active ref;
  inject beats 2 and 3 -- after each, `pendingClip` still set and the playhead not below the previous read; inject
  the downbeat; within 0.5 s `pendingClip` is empty and a playhead read is in [0.2, 0.26] and below the read before
  the downbeat. RED arm: `pendingClip` is never set, and right after the second fire the playhead has already
  jumped (the old app seeks a Snap clip to the injected beat phase as a file position, MainComponent.cpp:4868-4889).
- TR6b queued_fire_of_a_left_clip_starts_at_its_start. A (Snap Bar, in-point 0.5) fired and released on a downbeat;
  2 s; B (Snap Off) fired on the same layer; 1 s; A fired (queued); inject beats up to the downbeat; capture at
  el ~ 0.3 after it. Bar: |t - (5.0 + el)| <= tol. RED arm: t ~ where A left (about 2 s).
- TR7 scrub_drop_plays_on (through the real handlers; in 0 / out 1). (a) strip: `/api/debug/strip_scrub` down 0.6,
  drag 0.7, up; capture 0.5 s after up: |t - (7.0 + 0.5)| <= tol, and five playhead reads over that 0.5 s are never
  below 0.69. (b) Clip tab (the clip inspected by `/api/debug/inspect_clip`): the same through
  `/api/debug/clip_timeline` at the x of 0.6 and 0.7. GUARD for the RED arm (the routes are new). Teeth on the lane
  build: MU-8 and MU-9 each turn (a) RED (builder's log; Harmony re-runs one).
- TR7b scrub_at_the_ends. In 0 / out 1. The probe first waits for a playhead read in [0.3, 0.6]. Clip tab press 3 px
  inside the left end of the bar, below the marker tab, up: `inPoint` still 0 and a playhead read within 0.2 s is in
  [0, 0.06]. Press 3 px inside the right end, up: `outPoint` still 1. GUARD: no RED arm (the route is new). Teeth on
  the lane build: MU-11 turns the left half RED -- the press grabs the marker, nothing is scrubbed, and the playhead
  read stays above 0.3 (builder's log). The right half holds under MU-11 too (a press that grabs a marker and lets
  go writes nothing, R40): it is a guard, and the right end's teeth are TL-U13.
- TR7c scrub_past_the_out_marker. Loop ramp, in 0.2, out 0.8: strip down 0.95, up; within 0.5 s a playhead read is
  in [0.2, 0.3] and no read is above 0.81. OneShot variant: `playing` false and the playhead <= 0.81. GUARD: no RED
  arm (the route is new). Teeth on the lane build: MU-10 turns the OneShot variant RED -- its playhead rests near
  0.95 (INFERRED from R9; builder's log). The Loop variant wraps with or without the clamp (R9): it guards AM-13's
  wording.
- (only if S3h is built, AM-12) TR8 scrub_hold; the pin is then `EXPECTED_ROWS=18`. Ramp, in-point 0.3, out 1.
  `/api/debug/strip_scrub` down 0.62; four playhead reads over 1 s, each within one frame (1 / 300) of 0.62 and with
  `playing` true; up; the read 0.5 s after up is > 0.64. GUARD: no RED arm (the route is new). Teeth on the lane
  build: MU-H2 -- the read after up stays at 0.62.
- TR9 bpm_rate_follows_tempo. Composition with old keys (`transportMode` 1, 4 / 4: both arms load it);
  `set_bpm 120`; fire; r1 = content seconds per second over 2 s; `set_bpm 90`; 0.5 s; r2 over 2 s.
  Bar: r1 in [0.97, 1.03], r2 in [0.72, 0.78]. RED arm: r2 ~ 1.0.
- TR10 old_show_speeds. (a) Old BPM-synced sequence (8 frames, 2.5 fps, 4 beats), `set_bpm 120`: `clipBpm` == 75
  (lane arm) and one cycle = 2.0 s +/- 5 % on BOTH arms. (b) Old BPM-synced video 8 / 4: `clipBpm` == 60 (lane arm);
  at `set_bpm 120` the rate is in [1.94, 2.06] on BOTH arms; at `set_bpm 90` the lane arm's rate is in [1.45, 1.55]
  and the RED arm's stays in [1.94, 2.06] -- the stated change (AM-16).
- TR11 undo_leaves_the_fire. Fire a ramp; 1 s; `POST /api/debug/undo`; 0.5 s. Bar: the layer's `activeClip`
  unchanged, `playing` true, t advanced by >= 0.3 s. RED arm: the layer is empty after the undo.
- TR12 undo_load_deck_keeps_playing. 3-layer show; `/api/debug/load_deck` a 5-row deck; fire its row-0 ramp; 1 s;
  undo. Bar: 5 layers, the deck absent from `decks`, the pixels still the ramp and advancing; redo: the deck back
  under its id, t monotonic. RED arm: the deck still present, 5 layers, the layer EMPTY -- on the old app the one
  undo undoes the fire. (T6h itself is shown RED by TL-UC9 on S1's base, not by this row.)
- TR13a undo_replace_on_the_playing_cell. Two ramps that pixels tell apart (R1, R2). R1 playing in a cell;
  `/api/debug/drop_files` R2 onto the SAME cell (the picture becomes R2's; playing or paused is not judged, SF-6);
  1 s; undo; 0.5 s; redo; 0.5 s. Bar: after the undo and after the redo the picture is R2's and
  `/api/debug/clip_media` gives R2's file as `reveal_target` (fix round, R38: the full path of what Show in Finder
  selects, for the cell of the deck the grid shows). RED arm: after the undo the picture and the file are R1's.
- TR13b undo_effect_edit_on_the_playing_clip. BPM-synced ramp, `set_bpm 120`, playing; add an effect to it through
  the Undo-step test route; `set_clip_param clipBpm 240`; rate r1 over 1 s; undo; r2 over 1 s; redo; r3 over 1 s.
  Bar: r1, r2, r3 in [0.47, 0.53], `clipBpm` == 240 throughout, t never steps back. GUARD for the RED arm (routes
  are new). Teeth: MU-18 gives r2 ~ 1.0.
- TR14 autopilot_advance_restarts. probe-boxes k3's driver (PlayNext every 4 beats over ramps A, B, C; injected
  beats): capture at el ~ 0.3 after the advance that returns to A. Bar: |t - (5.0 + el)| <= tol.
  RED arm: A resumes (t ~ where A left, far from 5.0 s).
- Paths with no live row, and what proves them: REST = the driver of every row. OSC, key, MIDI, piano / momentary,
  take and routine replay, the checkpoint-0 restore all enter the two handlers (lint TL-L4) and end in the tail
  (TL-U1 .. TL-U3c). A take replay CAN be driven by REST (`/api/perf/load`, `/api/perf/play`, R22): if S5 can author
  a one-fire take file it adds INFO row TR12b -- a Load Deck that added layers, its clip fired by the replay, one
  undo: lane arm as TR12; RED arm 3 layers and the ramp gone (T6h itself). INFO, outside EXPECTED_ROWS; the lane
  report says whether it was built.
- Measurements, printed as `MEASURE <name> <value>`, never part of the verdict line: M1 (FM-1), M2 (FM-3),
  M3 (FM-4).
- Flake rule: a row that fails once is re-run 5 times on each arm before a verdict.

### 5.4 Re-runs and idle paint

- `.harmony/probe-boxes.sh` -> `PROBE-BOXES GREEN`, `EXPECTED_ROWS=25`, with k10 re-registered as
  `k10_fresh_and_restart`: (ii) "the re-fired clip restarts": `abs(t - el2) <= resumeTol`; "(ii) its REST playhead
  stays still while it is in no layer" unchanged. RED arm of the new (ii): the frozen app resumes -- t is where the
  clip left, at least 4 s (the row waits for that, R40), so `abs(t - el2)` is far over the bar. The other 24 rows:
  GUARD on both arms.
- `.harmony/probe-video.sh` rows w3_retrigger_midgop_1080, w3b, w6b_retrigger_mid_fade, w4_deck_return_1080 and
  `.harmony/probe-media-open.sh` row m6_retrigger_seek -> their own bars (they are the guard of a cell re-fire):
  GUARD on both arms, no RED arm.
- The deck change's post-merge gate script (.harmony/.reports/s-rta-1003/wf/gateC2.sh, added in 34179a2) run whole on
  the lane head: every row whose verdict changes is listed with its cause (a re-fired clip restarts now). GUARD, no
  RED arm.
- `.harmony/probe-idle-paint.sh` with the Clip tab open on a playing BPM-synced clip -> its existing bars (INFO
  unless it regresses; no RED arm); the two boxes add no repaint while their text is unchanged (TL-U20 is the gate).

### 5.5 VISUAL WORK GATE for T4's widgets (stage S4b)

Captures by the app's own capture route at 1728 x 1117 and 1280 x 720. Every bar is read from
`GET /api/debug/clip_transport_ui` (widgets: bounds, text, enabled, visible; the painted strings of the Transport
section; the viewport's visible area and scroll position), never from pixels alone. States are reached with
`/api/set_clip_param`, `/api/debug/clip_transport_do` and loaded compositions -- no synthetic input. A state that
cannot be captured is BLOCKED and the gate fails.
States (PNG + dump each): V1 Timeline mode (Speed row with Reverse; no BPM rows). V2 BPM Sync, default (BPM "120",
Beats = 2 x L). V3 after x2 (BPM "240"). V4 at the upper limit (x2 disabled). V5 at the lower limit (/2 disabled).
V6 length not known -- fixture: a BPM-synced video clip whose file does not exist (Beats "--", disabled; BPM, /2, x2
work). V7 a typed BPM "127.5". V8 trimmed in / out (Beats smaller, BPM the same). V9 the same clip in the narrowest
inspector the window allows. V10 an image clip (no BPM row, no Beats row; what else
the section shows for an image today is outside the lane). V11 x2 then /2 (the texts of V2 again).
Bars (widgets and painted strings from the dump; `clipBpm`, `inPoint`, `outPoint` from `/api/composition`; the file
length from the capture builder's manifest; L = (outPoint - inPoint) x file length): B1 no widget of the Transport
section is cut by the Clip inspector's own bounds; at 1728 x 1117 every one lies inside the viewport's visible area
at scroll 0; at 1280 x 720 the scroll position that shows the Beats row whole is within the viewport's range and the
PNG is taken there. B2 "/2" and "x2" have the x, width and height of the Speed row's pair in V1 (+/- 1 px); Reverse
has the bounds it has in V1. B3 (round 2) the section's captions (painted strings and label widgets together) hold, as
exact strings, "BPM" once and "Beats" once and no "Speed" in the BPM Sync states (V2-V9, V11); "Speed" once and no "BPM", no
"Beats" in V1; no "BPM" and no "Beats" in V10; "Duration" in no state (AM-26); the Beats row's buttons read exactly "/2"
and "x2"; no widget of the section shows the text "Restart" (AM-18, H-4). B4 (round 2) in every state whose length
is known (V2-V5, V7-V9, V11): |Beats text - L x clipBpm / 60| <= 0.01; in V2 also Beats text == 2 x L to 2 decimals.
B5 V4 / V5: the named button `enabled == false`, the other true. B6 no widget of a hidden row is
`visible == true`. B7 (round 2) nothing in the captions, labels or tooltips of the section changes except the two
numbers (a) between the dumps before and after x2 and (b) before and after `type_bpm` with a value below the range
(the BPM text becomes the lower limit); (c) before and after `double` at the upper limit the two dumps are equal
(Harmony constraint: no text announces an event or a failure). B8 V11's two texts equal V2's. B9 (round 2) in every
BPM Sync state (V2-V9, V11) the BPM text is the model's `clipBpm` with at most 2 decimals, trailing zeros cut.
B1-B9 are GUARDS with no RED arm (fix round): each is read from a route S4b adds, so the frozen app cannot be read.
Teeth on the lane build, shown once in S4b's log: MU-13 turns B4 RED in V8.
Five critic seats (Harmony constraint, R35), each given the PNGs, the dumps, the manifest, this section, the packet
of AM-28, BORIS_DECISIONS.md "Inspector Grammar" and "Rejected":
- visual-design: do the two rows read as one group with the mode combo? Is anything misaligned against the rows
  above and below, Reverse included? Is the disabled Beats box ("--") told apart from an enabled one without colour
  alone?
- UX: can a first-time user tell which box to type in to make the clip twice as slow? Is it clear the two numbers
  move together? Coming from the Speed row, will x2 here be read as "faster" (it makes the clip slower)? Is there
  any state where a control looks usable and does nothing?
- graphic-design: type sizes, number formatting ("120" against "120.00"), spacing of "/2" "x2" against the box; does
  the block match the Speed row it replaces?
- logic (round 2): B4 and B9 are given to you as numbers. Is there a state whose numbers pass and still tell the
  user something false -- a Beats text that reads as a whole number when the loop is not a whole number of beats, a
  BPM text that hides a clamp, a pair of values that cannot both be reached by typing?
- interaction-logic (driven through `/api/debug/clip_transport_do`): type in Beats -> BPM changes; x2 then /2
  returns the same text; reset on either box -> "120"; a moved out marker changes Beats while BPM stays; the
  refresh never takes the keyboard focus from a box being typed in; Reverse works in BPM Sync.
Pass = B1-B9 and no seat raising a MUST. A MUST is fixed and the whole gate re-run.

### 5.6 Facts Harmony must measure (none can be settled by reading)

- FM-1 the stale frame (after S2, lane build). A 1080p long-GOP clip with its in-point mid-GOP, and the ramp
  fixture: fire A; 3 s; fire B on the same layer (Cut); 3 s; fire A; take 10 `/api/snapshot` pictures as fast as
  the route answers, each time-stamped. Count the pictures that still show the frame A left on.
  Bar: none later than 50 ms after the fire reply. Met: no change. Not met: stage S2b (AM-21), then re-measure.
  Still not met after S2b: STOP and report -- no further change is pre-decided.
  Also (b), the column: 8 layers, one column of 1080p long-GOP clips, `trigger_column` twice 2 s apart; read
  `/api/state` peak frame time and `video_late_frames` before and after. Bar: peak frame time <= 50 ms (probe-boxes
  k8's bar). Not met: STOP for a look before S3 -- no change is pre-decided.
- FM-2 the dead fire (frozen app, before S2). `trigger_clip(0, 0)` on a video; 1 s; `trigger_column` of a column
  whose cell on layer 0 is empty; `trigger_clip(0, 0)`; read `playing` and two playheads 0.2 s apart.
  `playing` false: F1 stands and the reading goes to Boris with Q1. `playing` true: AM-3's fallback.
- FM-3 the tempo's wobble (frozen app or lane build). `POST /api/audio/source` a music file; poll `GET /api/bpm`
  at 20 Hz for 60 s while the tracker reports locked. Bar: (max - min) <= 0.5 % of the mean. Met: the raw read
  stays. Not met: AM-22's slew.
- FM-4 the drift (lane build). The same file; a BPM-synced ramp fired on a downbeat with Quantize on; after 5
  minutes compare the clip's position inside its loop with the beat clock. Bar: within a quarter of a beat.
  Either way no build change: over the bar it is reported to Boris with Q9.

## 6 WHAT ONLY BORIS CAN CHECK (do -> expect -> what wrong looks like)

1. Fire a video, let it run, fire another clip on the same layer, fire the first again. -> It starts from its
   beginning. Wrong: it carries on from where it was.
2. Fire a column, wait, fire the same column again. -> Every video in it starts over together. Wrong: some keep
   running.
3. Play music until the beat counter at the top runs steadily. Set Quantize to "Next Downbeat". Pick a column where
   every layer has a clip. Fire it, then fire it again in the middle of a bar. -> Nothing moves until the next "1",
   then they all start over together. With "Next Beat" they start over on the next beat. Wrong: they jump at once,
   or never. (With no steady beat the app cannot wait for a bar, so the videos start over at once. That is expected.)
4. Pause a clip, then fire it. -> It plays from its beginning (question 1). Wrong: it sits on its first frame.
5. Clear a layer with its X, then fire the same clip again. -> It plays from its beginning. Wrong: it sits frozen.
6. Let a One Shot clip run to its end, then fire it again. -> It plays again. Wrong: nothing moves.
7. Grab the playhead in the layer strip, and then in the Clip tab; drag; let go. -> The picture follows your hand
   and the clip plays on from where you let go, at the same speed. If you keep the mouse down and hold still, the
   clip keeps playing from under your hand (question 2). Wrong: it springs back to where it was.
8. Do the same on a reversed clip and a ping-pong clip. -> It carries on in the direction it had. Wrong: it turns
   round.
9. In the Clip tab, press at the very left or right end of the bar, below the small marker tab. -> The playhead
   goes there; the marker does not move. Drag the marker tab itself. -> The marker moves.
10. On a clip with a trimmed end, drop the playhead beyond the end marker. -> It stops at the end marker and the
    loop starts over (question 10).
11. Set a clip to BPM Sync. -> BPM shows 120 and Beats shows the clip's length in beats at 120. Press x2: both
    numbers double and the clip takes twice as long (it looks slower); /2 brings it back (question 7). The Reverse
    button is still there. Tap a faster tempo: the clip speeds up with it. Wrong: the speed does not follow your
    taps, or the numbers disagree with what you see.
12. Type a number into Beats (for example 16). -> BPM changes to match and the clip loops in exactly 16 beats.
13. Play a BPM-synced clip and a speed clip side by side and change the tempo. -> Only the BPM-synced one changes.
14. Open one of your saved shows that has BPM-synced clips. -> At a tempo of 120 they move as they did before. At
    another tempo a BPM-synced VIDEO now follows the tempo (before, it ignored it). Image sequences behave as before.
15. Fire a clip, then press Cmd+Z. -> The clip keeps playing; Cmd+Z undid your last edit instead (or nothing).
    Wrong: the clip stops or the old one comes back.
16. Drag an effect onto the cell that is playing, switch that clip's Reverse on, press Cmd+Z. -> The effect goes;
    the clip keeps running backwards. Wrong: the direction flips back, or the clip jumps.
17. Drop a new file onto the cell that is playing, press Cmd+Z. -> The picture does not change. Do the same on a
    cell that is not playing. -> The old file comes back.
18. Load a deck that adds layers, fire one of its clips, press Cmd+Z. -> The deck leaves the tab row, the clip keeps
    playing, the added layers stay (question 8). Wrong: the picture goes away.
19. Remove a layer by mistake, press Cmd+Z. -> The layer and its clips are back, not playing until you fire one
    (question 5).
20. Bypass a layer that is playing, make another edit, press Cmd+Z twice. -> The second Cmd+Z brings the layer back
    on (question 3).
21. Fire a clip on a bypassed layer, wait, switch the bypass off. -> The clip starts from its beginning when it
    appears.
22. Hold down a key that fires a clip or a column. -> It fires once; the video plays on while you hold.
    Wrong: it keeps jumping back to its start.
23. Remove a layer, press Cmd+Z (it is back, playing nothing), fire a clip on it, press Cmd+Shift+Z. -> Nothing
    happens: the layer stays, the clip keeps playing, and nothing is written on screen. Once that layer plays
    nothing, Cmd+Shift+Z removes it. Wrong: the layer goes away while it plays, or a message appears.

## 7 BORIS QUESTIONS (each has a default; nothing waits)

- Q1 You paused a clip and then fire it. Should it play from its beginning (default), or go to its beginning and
  stay paused?
- Q2 While you keep the mouse down on the playhead and hold still: should the clip keep playing from under your
  hand (default), or should the picture wait on that frame until you let go?
- Q3 Cmd+Z and a layer that is playing: bypass, solo, the order of the layers and effects are still undone
  (default, as today). Or should Cmd+Z leave some of these alone while the layer plays -- which ones?
- Q4 BPM and Beats are two ways of saying one thing: change one and the other follows, and the whole clip is
  stretched to fit (default). Or should Beats cut the loop shorter while BPM stays?
- Q5 You remove a layer by mistake and press Cmd+Z: it comes back with its clips, not playing (default). Or should
  it come back playing what it was playing?
- Q6 A clip set to play backwards: when you fire it, it starts from its end and runs back (default). Or should it
  start from its beginning?
- Q7 The x2 button next to Beats doubles the beats, so the clip takes twice as long and looks slower (default). Or
  should x2 make the clip twice as fast, like x2 next to Speed?
- Q8 You load a deck that adds 2 layers, fire one of its clips and press Cmd+Z: the clip keeps playing and both
  added layers stay (default). Or should the added layer that plays nothing go away?
- Q9 A BPM-synced clip left running for many minutes follows the tempo; to line it up with the beat again you fire
  it (default). Or should the app keep nudging it back onto the beat by itself?
- Q10 You drop the playhead beyond a clip's end marker: it stops at the end marker and the loop starts over
  (default). Or should it play the part outside the markers once?

NOTES FOR BORIS (round 2; not questions -- they go on his page with the checks)
- N1 (with AM-26; the bars ruling's own sentence) "the Beats/Duration row in the clip panel never did anything; it
  is gone."
- N2 (with AM-18) The "Restart / Continue / Relative" menu beside Loop in the clip panel never did anything; it is
  gone. Every clip now starts from its beginning when it is fired. If some clips should carry on from where they
  were instead, say so.

## 8 SIDE FINDINGS (outside this lane)

- SF-1 The workflow script cuts the seat papers at 70,000 characters (wf/transport-plan.js:73). Round 1 lost the
  end of the fifth paper; round 2 read the papers from a file and closed it. A file handed over by path is not cut.
- SF-2 A clip's transport SETTINGS are plain fields read by the GL thread every drawn frame and written by the
  message thread (R10; e.g. the in / out drag, ClipInspector.cpp:1388, :1392). Pitfall 63 rule 1 class, there before
  this lane. This lane only makes its one new field Relaxed.
- SF-3 INFERRED (R3, R4): today a clip re-fired by a cell click after its layer was cleared stays paused. FM-2.
- SF-4 INFERRED (R6): today a held key bound to Trigger Clip re-seeks its video on every auto-repeat.
- SF-5 A trimmed PingPong wraps like a Loop at the out marker (R9); reverse inside a trimmed range is not enforced
  (the plan's R8).
- SF-6 INFERRED: after a direct Replace Content (`playing = newContent.playing`, Clip.h:330) or a file dropped onto
  a playing cell (a new clip that was never fired, R17) the layer may show the new clip paused. It bears on TR13a
  (which judges the media, not the motion) and on Q1.
- SF-7 A SetClipCmd snapshot is the whole clip, so Undo of one edit also brings back every un-stepped tweak made
  since -- on an idle cell that stays true after this lane (the header already notes it for ToggleClipLockCmd,
  ClipCommands.h:143-145).
- SF-8 docs/claude/performance-controls.md:81 still says Cmd+Z undoes "a clip fired"; :48 names the resume.
  Plan section 6 covers both.
- SF-9 docs/claude/architecture.md:265 calls UndoManager dead (the plan's finding; not re-read here).
- SF-10 Cells past `numColumns` still resolve for a tuple (`rowCellsFn` uses the row's own count,
  Composition.h:1198-1204), so a clip in a hidden column keeps playing. The floor of AM-6 keeps Undo from hiding one.
- SF-11 Learnings for Harmony to log (a project-repo boot has no log-event fence): (a) in an Undo stack whose
  commands address rows by index, a step that is skipped but counted as done leaves later steps recorded against
  another model -- refuse it (the index stays) or address by id; (b) a workflow that passes seat papers inline must not slice them;
  (c) a restart counter compared for inequality can be rewound into a collision by a value copy -- mint from one
  monotonic counter and consume only when newer.
- SF-12 The Speed row's pair reads a division sign and "2", a multiplication sign and "2" (R28); the new pair reads
  "/2" and "x2" (Boris: "a x2 and /2 control to double or half easily"; Pitfall 6 is about glyphs on small buttons).
  The two modes then spell one pair two ways. Existing text, outside this lane; on the critics' list (AM-28); a
  candidate for the ui-polish ledger.
- SF-13 In round 2 the harness refused two of my read-only greps (its removal check misread a brace quantifier in
  the pattern). The same facts were read with the file reader and a plain grep. Nothing was removed or attempted.
- SF-14 One more learning for Harmony to log: a plan that adds a row beside an inert row must read the inert row's
  PAINTED label in every mode -- the dead "Duration" row is painted "Beats" in the one mode this lane adds a Beats
  row to (AM-26).
- SF-15 (fix round) Two more learnings for Harmony to log: (a) when a gate list is staged, sweep each stage's exit
  line for a field or a function that a later stage adds -- three S1 cases and one S2 lint named one; (b) a row's
  "Teeth: MU-n" is a claim to derive, not to write: read what the row measures under the mutant -- of the three
  teeth lines checked in the fix round, two did not bite as written (TR7b, TR7c's Loop variant).

## 9 RISKS OF THIS RULING (the strongest counterargument first)

- K1 THE STRONGEST. Undo-live is now the most intricate part of the lane: one landing rule with four outcomes, a
  refused Redo, a duplicate-id test, one live test across decks, layers, cells. Reading found a hazard the council
  did not (a spent Remove Layer redo leaves later Redo steps one row off); there may be others. The simple, sound
  rule is to REFUSE any step that would touch what is live. Why this ruling still keeps partial steps: with a
  plain refusal, one mis-drop on a playing cell blocks every older Cmd+Z for as long as that clip plays, which is
  most of a show, and nothing may say why (BF32). Boris: "It changes anything else". Cheapest refutation: TL-UC16
  and TL-UC14, written before any command is changed. If the walk cannot be made GREEN inside S1, the fallback is
  the plan's own: refuse for layers and columns in both directions, keep the landing rule for cells and the retire
  for decks -- and say so to Boris.
- K2 The drag ships without the hold. The plan's author chose the hold on purpose; a playhead that slides away under
  a resting hand may read as a bug to Boris. Why it holds: his words ask for nothing while the mouse rests; the hold
  is where both of the real-time seat's stage failures live; a freeze on the output is what a hold shows the room.
  Refutation: check 7 and Q2; S3h is specified and small.
- K3 F1 ships as a default against a MUST (GA-5). If FM-2 shows the dead fire is real, the default rests on a
  measurement, not on his word -- and it still reverses Pitfall 7. If he answers Q1 "stay paused", the tail gets
  the narrow rule and a clear must stop marking its clip paused, or check 5 fails; that follow-up is not specified
  here.
- K4 The restart shows a stale frame for as long as the decoder needs (R13). The rows at el 0.3 cannot see it; only
  FM-1 can. If S2b is needed it touches the player's hold / pending logic (Pitfalls 53, 56, 60), the riskiest file
  of the lane.
- K5 Rate, not position (F9). FM-4 may show a drift Boris reads as "not synced". The ruling does not build a fix.
- K6 The queue now counts as live for decks (AM-8): a deck Cmd+Z takes away can still start a clip on the next bar.
  That follows from "a fire is not undone", but Boris has not seen it; it reverses a line of ruling-bf9b-merge AM-7.
- K7 The held-key and CC edge rules (AM-4) change binding behaviour nobody asked about. Small, unit-pinned, and the
  alternative is a column that restarts many times a second under a held key; check 22.
- K8 (round 2; the cut paper is closed) The two dead controls leave the Clip tab in this lane (AM-18, AM-26) though
  Boris asked for neither, and the minimalist argued to leave the combo. Why it holds: the Duration row is forced --
  it is painted "Beats" beside the new Beats row; the combo offers "Continue", which he ruled out ("restart"), and
  "Relative", which the app never had, on the lane that makes "restart" the rule, and no lane owns it. He said of
  another dead control "Does the keying slider actually do anything? Maybe we get rid of it." -- about the keying
  slider, not about these two. Refutation: notes N1, N2 and his answer; H-2 and H-4 are Harmony's, and each way is
  specified.
- K9 TL-UC3b and the carry list name fields by hand; a transport field added later and forgotten there is undone
  under a live clip. The case lists the fields; the docs name the rule (new Pitfall, plan section 6).
- K10 Line numbers are those of 34179a2. bf2 moves them; S0 exists for that.
- K11 (round 2) S1 early (AM-29) puts a rebase over bf2 under the most intricate stage of the lane. Why it is still
  offered: the files S1 rewrites are named by none of bf2's reports (R36, INFERRED -- bf2's source was not read),
  and every S1 case runs again after the rebase. The default is not to do it.
- K12 (round 2) G-N1 counts five strings; an event text built another way (a caption set from a handler, a Label's
  `setText`) passes it. B7 covers the Transport section only. Outside that section no stage of this lane writes a
  caption; the reviewers' packets should say so in one line.
- K13 (round 2) Section 10 restates T5's command table. If the table and an amendment ever seem to differ, the
  amendment (AM-6 .. AM-9) is the text and the table the summary.
- K14 (fix round) Three S1 cases get a clause from a later stage (5.1). G-U1 selects by id, so a clause that was
  never added still passes it. What catches it: the later stage's RED log, by id, and on the lane head MU-1
  (TL-UC16), MU-28 (TL-UC2b), MU-29 (TL-UC3b; live TR13b).
- K15 (fix round) After a refused Redo the menu's Redo item is enabled and does nothing while the layer plays
  (AM-7). Accepted here: the step is still there and runs once the layer plays nothing. Not built here: the native
  menu is rebuilt when the history changes (MainComponent.cpp:1766-1767, :1776 at 34179a2: the comment and the
  `onHistoryChanged` hook; the hook's body was not read), and after S1 a fire or a clear
  is no history change -- a greyed item would go stale unless something refreshes it when a layer starts or stops
  playing, or the menu asks again each time it opens (not read: JUCE's source is not in the repo). The Edit-menu
  lane rebuilds this menu and may grey it.

## 10 HOW A BUILDER READS THE PLAN (round 2)

The plan is built from its body plus this ruling; where they differ this ruling wins. Nothing in the plan is
rewritten first. A stage packet = the plan parts and amendments of its row below, sections 4 and 5 here, and S0's
table. R23 (a ctest name is the TEST_CASE name) binds every stage.

Dead in the plan -- never read, never copied from:
- plan section 4 (stages; 4.1 stays as background; 4.2-4.4 are replaced by section 4 here, AM-18 and AM-26);
- plan section 5 (gates), and every test id (U1.., U-C1..), mutant id (MU-T..) and row number (TR..) of the plan:
  the ids, names and bars of section 5 here are the only ones;
- plan sections 7 and 8 (Boris's checks and questions): sections 6 and 7 here;
- T3 "THE CHANGE" and "TESTS": AM-11, AM-13 and 5.1 (S3); fork F6 is "no hold";
- T5 "EVERY COMMAND": the table below; T5 "LIVE" (F13): AM-6's last bullet and AM-8;
- every `scrubHeld`, `ScrubPhase`, `/api/debug/scrub` and "hold" line, also in plan sections 4.3 and 6 (they return
  only with S3h, as AM-12 writes them);
- plan section 9's R4 list and R7: R8 here and AM-12.
Read differently: plan section 1's G-C "shows the frame under the mouse while held" reads "follows the mouse; the
clip keeps playing from each drag position" (AM-11); plan section 2 stands where section 1 here does not correct it
(E23's ":299" is ":293").
Renamed everywhere: `restartSeq` is `restartStamp` (minted from one counter, consumed when newer: AM-2);
`carryLiveTransportFrom` is `transportState` / `setTransportState` (AM-6).

| stage | plan parts that stand | with | facts |
|---|---|---|---|
| S1 | T5: BORIS, the collision rule (F14), F15, F18, "THE CHANGE"; T6's Undo rows | AM-6 .. AM-10, AM-27; the table below; 5.1 S1 | R17-R22, R33 |
| S2 | T1: the rule, F1-F4, "THE CHANGE", the path table; T2; T6's restart rows | AM-1 .. AM-5, AM-23, AM-25, AM-30; the rows added below; 5.1 S2 | R1-R14, R32 |
| S3 | T3: "TODAY", the causes C-A .. C-D, the paragraph on each transport mode after a drop, BORIS, F7 | AM-11, AM-13; 5.1 S3 | R15, R16 |
| S4a | T4: "THE MODEL", F8-F11, "THE CHANGE" (model, render, REST) | AM-14, AM-15, AM-16, AM-18, AM-22; 5.1 S4a | R10, R11, R30 |
| S4b | T4: "THE WIDGETS", "THE CHANGE" (ClipInspector, the length provider, the dump route) | AM-15, AM-17, AM-18, AM-26, AM-27, AM-28; 5.1 S4b; 5.5 | R24, R25, R27-R29 |
| S5 | plan section 6 (docs) | the wording of AM-2, AM-3, AM-6, AM-7, AM-8, AM-27; no hold; sections 5.3-5.6, 6 and 7 here | R26 |
| S2b, only if FM-1 is over its bar | none | AM-21; 5.1 "Conditional stages"; 5.6 FM-1 | R12-R14 |
| S3h, only if Q2 is answered with the hold | T3's F6, as background only | AM-12; 5.1 "Conditional stages"; 5.3 TR8 | R9, R15 |

The path table of T1 stands with these rows added: "a held key, a moving controller: one fire per press, per rise"
(AM-4); "a layer that is bypassed, hidden, or muted by another layer's solo: the fire lands at once in the model;
the clip starts from its start on the first frame the layer is drawn again" (AM-5); "with no steady beat the app
cannot wait for a bar: a re-fire restarts at once" (AM-23); the momentary row reads "release = clear, also when the
press only queued a restart" (AM-1).

T5's command table, final (Undo and Redo only; a command's first execute is Boris's own action and is never
filtered or refused):

| command | Undo | Redo |
|---|---|---|
| TriggerClipCmd, ClearActiveClipCmd | deleted: a fire and a layer's X push no step; the clear-cell composite keeps its SetClipCmd child | -- |
| SetClipCmd | the saved clip lands through `landClipInCell` (AM-6): a live cell is left when the saved state is empty, another clip id or other media; the cell is left when the saved id sits in a cell this command does not write; the same id and media on a live cell lands the edit and keeps the live transport; an idle cell takes it whole | same |
| ClearLayerClipsCmd | each saved clip lands cell by cell through the same rule; no tuple write | each cell is emptied through the same rule: an idle cell is cleared, a live cell stays |
| SwapClipsCmd | left whole (both cells, the column count) when either landing would be left; a column count it restores is floored at the highest live column + 1 | same |
| SetColumnCountCmd | a shrink stops above the highest live column of that deck | same |
| RemoveColumnCmd | re-inserts the saved cells; nothing starts | REFUSED when a tuple names that column of that deck: the index stays (AM-7) |
| AddLayerCmd | a live layer stays and the step is spent; an idle one is erased, found by layer id | re-adds only what the Undo erased |
| RemoveLayerCmd | the layer and its clips return with an empty tuple (F15) | REFUSED over a live layer: the index stays (AM-7) |
| InsertDeckCmd | nothing of it live: the deck and its added layers go (layers by id). Anything live (`deckIsLive`, or an added layer is live): the deck goes through `retireOrEraseDeck` and ALL added layers stay (AM-9); no tuple write (AM-8) | restores the retired deck if it is still there, else re-inserts it under its id; re-adds only the layers the Undo erased |
| AddDeckCmd | `retireOrEraseDeck` by `deckIsLive`; no `cancelPendingInto` (AM-8) | as today |
| RemoveDeckCmd | as today | as today; its execute keeps `cancelPendingInto`, the one tuple write left (AM-8) |
| MoveLayerCmd, ToggleLayerFlagCmd, RenameDeckCmd, ToggleClipLockCmd, EffectStackCmd | as today (Q3's default, AM-10) | as today |
| CompositeCommand | per child | per child; it reports a child's refusal; the two refusing commands are never its children (AM-7) |

Open by design, none blocks a stage: Boris's Q1-Q10 (each has a default), Harmony's H-1..H-4 (each has a default),
the measurements FM-1..FM-4 (each has a pre-decided consequence).
45 of 45 attacks ruled: 28 ACCEPT, 16 PARTIAL, 1 REJECT. 30 amendments. Sections 0-10 complete. The papers file
holds all five papers whole (R40).
Fix round: 7 of 7 findings of the completeness check verified, accepted and fixed in place (end of section 2).
BUILDABLE FROM THE PLAN BODY PLUS THIS RULING, this ruling overriding. No section of the plan is rewritten first: the
parts the amendments replace whole are replaced inside this ruling (section 0), and section 10 is the reading map.

STATUS: DONE
