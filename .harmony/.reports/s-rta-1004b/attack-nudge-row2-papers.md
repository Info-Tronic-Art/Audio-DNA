# attack-nudge-row2-papers.md -- the blind council's seat papers, VERBATIM

Lane: nudge-row2 (the beat-nudge lane, second delta on Boris's answers 125-129, 135-137; readings R81, R85).
Plan attacked: `.harmony/.reports/s-rta-1004b/plan-nudge-row2.md`.
Seats returned: 2 of 2 (`gates`, `stage-hands`). 16 attacks (GA-1..GA-8, ST-1..ST-8).
Source: the workflow harness's task text, 17809 characters of JSON, a closed array (checked: the
transcription below was re-parsed and its raw length is 17809). Nothing is edited: the JSON is only
pretty-printed (indent 2, key order kept, no character changed), one section per seat.
The architect's ruling on these attacks: `.harmony/.reports/s-rta-1004b/ruling-nudge-row2.md`.

## Seat: gates  (8 attacks)

```json
{
  "seat": "gates",
  "attacks": [
    {
      "id": "GA-1",
      "target": "quantize-out T-Q4 / MQ1 (the legacy beat-phase seek), QO-1 exit line",
      "claim": "T-Q4 is listed as a unit case that goes RED on the old tree and for which 'MQ1 RED' is claimed. The seek is not in Layer or Composition, so a model-level case cannot reach it. No mutant app carries MQ1, and nothing in section 5 runs the QO-0 RED arm for it.",
      "evidence": "M:src/MainComponent.cpp:4866-4885 is the seek. It sits in the clip handler, calls analysisThread_.getFeatureBus().read(), and seeks via previewPanel_.getRenderer().getVideoPlayer / getImageSequence. Plan lines 472-473 give T-Q4 'a snapshot whose beatPhase is 0.6' but name no handler seam (contrast T-S's 'the headless MainComponent seam', plan line 509). The only live check is QL1's 'playheadPosition within 0.05' clause (plan line 553), and QL1's stated RED arm (line 555-556) comes from the queue, not from the seek.",
      "severity": "MUST",
      "proposed_change": "Either name the seam T-Q4 drives (the handler, a FeatureBus snapshot, a playable clip) and add MQ1 to a mutant app, or drop 'MQ1 RED' from the unit list. Give QL1's playhead clause its own RED arm: a build with the seek left in must print 'FAIL QL1: playhead'."
    },
    {
      "id": "GA-2",
      "target": "quantize-out T-Q5 lint (the K2 safety argument)",
      "claim": "T-Q5 forbids tokens in src that the same plan keeps in src, so the lint cannot pass as written. Because K2's 'the pending slot can never be armed again' rests on T-Q5, the builder is pushed to improvise or loosen the token list.",
      "evidence": "Plan lines 474-477 forbid `forcedSnap` and `beatSnap` anywhere in src. NB-6 Q-h (line 258) keeps passing `RoutineSnap::Off` as the routine engine's forced snap. M:src/recording/RoutineEngine.h:120,123,130 and RoutineEngine.cpp:476-766 declare and use a `forcedSnap` parameter, and Q-h deletes none of them. Q-b/Q-c (lines 243-247) have the LOAD read the string keys \"quantizeMode\", \"beatSnap\" and \"beatSnapMode\" and drop them, so those literals stay in Clip.cpp and Composition.h. Line 476 allows `quantizeMode` only in PerfState files and 'the one load line of Composition.h', which omits the Clip.cpp literals. Comment-stripping does not remove string literals.",
      "severity": "MUST",
      "proposed_change": "Rewrite T-Q5 to check what can be proved. Scope `forcedSnap` to Layer.h, Composition.h, Autopilot.cpp and MainComponent.cpp. Exempt string literals in the two load functions by name. Assert that no function in src writes `pendingTriggerColumn` with a value other than -1. Count the surviving matches in the RED run."
    },
    {
      "id": "GA-3",
      "target": "QL4 and mutant MQ4 (routines keep their own quantize)",
      "claim": "MQ4 mutates only the routine TICK, but QL4's RED text 'the off routine waited' describes the FIRE path. With the tick mutated, an 'off' routine fired through the API still starts inside fire(), so QL4 is not shown to go RED.",
      "evidence": "Plan lines 487-488 define MQ4 as 'the routine tick passes Bar as the forced snap'. QL4's RED arm is 'MQ4's app (FAIL QL4: the off routine waited)', plan lines 568-569. M:RoutineEngine.cpp:766 decides the start mode inside fire() from the forcedSnap that fire() received, and MainComponent.cpp:6255 is the fire call site. The tick uses its forcedSnap only for already-pending slots (RoutineEngine.cpp:557-558) and for restarts (:476-478). An 'off' routine is never pending, so a tick-side mutation does not delay it.",
      "severity": "SHOULD",
      "proposed_change": "Make MQ4 mutate the fire call at :6255 (Bar as forced), or both lines. Name the RED line per call site, one for fire and one for tick. Keep T-Q6's guard role."
    },
    {
      "id": "GA-4",
      "target": "RB1 clearAllLayers: LR2 (a), T-S1..T-S4, mutant list; and B-3's 'the picture is empty AT ONCE'",
      "claim": "The only picture-level promise of Stop is left to Boris. No mutant and no row catches a clearAllLayers that omits `refreshPreviewFromShow()`, which is exactly the bug the X button once had. LR2 (a) and T-S1 read the model only (activeClip null), so they pass while a shader or MilkDrop source keeps rendering through the empty-compositor fallback.",
      "evidence": "M:src/MainComponent.cpp:741-745 documents the 2026-07-30 'A1 fix': clearActiveClip resets only the model, and without refreshPreviewFromShow a source kept re-rendering. Plan RB1 (line 133) calls refreshPreviewFromShow once. The mutants MS1, MS4-MS8 (lines 542-545) do not include omitting it. LR2 (a) (lines 601-608) reads `GET /api/composition` and `/api/routine/status` only. `POST /api/render_frame` exists (ApiServer.cpp:269) and renders offscreen: it is neither a screen capture nor synthetic input. Pitfall 28 notes Eyes does not drive the deck compositor, so the row must read a frame of the real compositor, not an Eyes set_effect frame.",
      "severity": "SHOULD",
      "proposed_change": "Add mutant MS9 (clearAllLayers skips refreshPreviewFromShow) to build-mut-stop. Add an LR2 (a) clause: with a source clip on a layer, a render_frame after Stop is the empty-composition frame, compared against the frame of the same fixture with no clip. Say in B-3 that the machine row proves 'empty' and Boris judges only how it looks."
    },
    {
      "id": "GA-5",
      "target": "QL1 / B-1 (every clip fire lands at once, 'mouse, your keys and your pads')",
      "claim": "QL1 drives only the clip-fire handler. Boris's keys and pads go through the column handler, which is a separate code site with its own gate, and nothing live covers it. A leftover gate there would still pass QL1.",
      "evidence": "Plan line 551 uses 30 fires by `POST /api/trigger_clip`. Q-d (line 250-252) names two separate edits, M:MainComponent.cpp:4849 and :5078. :5078 is the column trigger, `composition_.triggerColumn(..., forcedSnap, ...)` (viewed at :5075-5082). `POST /api/trigger_column` exists (ApiServer.cpp:182). T-Q2 covers only `Composition::fire`, and MQ2 is caught only by the T-Q5 lint (GA-2).",
      "severity": "SHOULD",
      "proposed_change": "Add a QL1 arm: 10 fires by `POST /api/trigger_column` on the OLD-show fixture, with the same 'at once' bar and a pendingClip null in every poll. Add a mutant that leaves a forced snap in the column handler and is RED on that arm. Add a T-Q case for `Composition::triggerColumn` beside T-Q2."
    },
    {
      "id": "GA-6",
      "target": "nudge L3b ('a fire never waits', replaces RN L3d)",
      "claim": "L3b has no RED arm of its own in this lane. It is a regression guard that passes on any tree where Quantize defaults to Off. The plan borrows QL1's RED on main and does not say so as a limit.",
      "evidence": "Plan line 590: 'L3b's RED arm is QL1's (main)'. The L3b fixture (lines 583-584) names no snap or Quantize setting. The Composition default is Off, so on the lane before quantize-out merges, L3b passes. The row does not read nudge code. No mutant in section 5 makes a nudged fire wait.",
      "severity": "SHOULD",
      "proposed_change": "Declare L3b a guard, not a RED-first row, in the strings Harmony copies. Or give it a mutant: a build where the fire handler reads the shifted beat (for example re-adding a forced snap Bar) goes RED on 'at least 40 of 60 at beatPhase >= 0.15', with the nudge set."
    },
    {
      "id": "GA-7",
      "target": "LR2 (d) 'layer 2 (never fired, autopilot on) is STILL empty 2.5 s after play'",
      "claim": "The clause cannot detect an autopilot refill, which is what its fixture suggests it tests. The autopilot cannot advance an empty layer, so the clause is vacuous in rounds 1, 2 and 4. Only the routine half has teeth, and only in rounds 3 and 5.",
      "evidence": "M:src/model/Autopilot.cpp, in the beat-based loop after `beatCrossings_.consume`: `if (clip == nullptr || !clip->playing) continue;`. An empty layer is skipped, and play's first edge makes no advance there. Plan lines 611-612 and 596-598 describe the fixture as an autopilot 'On Beat 4 beats' layer plus routines.",
      "severity": "NIT",
      "proposed_change": "Reword the clause so its claim matches what it can fail on ('no routine refills the layer'), and say it has teeth only in the routine rounds. Or assert 'no clip appears on any layer' in rounds 3 and 5 with MS7 as the RED arm, and drop the autopilot wording."
    },
    {
      "id": "GA-8",
      "target": "NB-8 fit: T-RB4 / T-RW1 / VG-0 'bar stated, then the design does not change'",
      "claim": "The fit gates feed back the plan's own assumptions, and VG-0's exit bar has no consequence. T-RB4 passes by construction with the label fixed at 86. The '33 px to spare' rests on assumed widths for the state word, Manual and Link, and on the label. VG-0 says that if the label is over 119 'the design does not change', so the measured manifest cannot go RED on the design.",
      "evidence": "Plan lines 359-360: 'ASSUMED widths: the Master Signal label 86, the nudge text box 88', and the state word 62, Manual 82 and Link 56 are 'taken from RR and not re-derived'. T-RB4 (lines 527-530) takes the label as an argument. Line 407: 'Harmony re-reads NB-8's sums before S3m -- the design does not change'. I recomputed 240+46+673+6+200+522 = 1687, and 1720-1687 = 33; the arithmetic is right, the inputs are the weak part.",
      "severity": "SHOULD",
      "proposed_change": "State, before VG-0, what a miss does: if the measured label is over 119, or the inner height is not 36, then compact becomes the 1728 default or cell sizes are re-ruled, with the architect named. Have T-RW1 read the real measured widths of the state word, Manual, Link and label from the built widgets (the manifest), not constants."
    }
  ],
  "strongest_point": "GA-2 and GA-1 together. The plan's safety argument for leaving the pending slot dead in the layer word (K2) rests on lint T-Q5 and on T-Q4's 'MQ1 RED', and neither can work as written. T-Q5 forbids `forcedSnap` and `beatSnap` in src while the same plan keeps `forcedSnap` in RoutineEngine (Q-h; RoutineEngine.h:120) and keeps the \"beatSnap\" string literals in the load functions. T-Q4 claims a unit case for a seek that lives in MainComponent's handler (MainComponent.cpp:4866), and no mutant app carries MQ1.",
  "citations_rechecked": true
}
```

## Seat: stage-hands  (8 attacks)

```json
{
  "seat": "stage-hands",
  "attacks": [
    {
      "id": "ST-1",
      "target": "RB1 STOP (clearAllLayers), T-S3, B-3, LR2 (b)(d), T8",
      "claim": "Stop leaves every cleared clip with playing=false, and a clip fired before (hasBeenTriggered true) is NOT set playing again on its next fire. So after Stop, re-firing a clip he has used stands on its first frame: a video does not move, and a BPM Sync clip stays frozen even after Play.",
      "evidence": "Layer.h:587-592 applyClearTail sets playing=false. Layer.h:553-556 applyActivationTail sets playing=true only if !hasBeenTriggered ('returning clips keep their state'); MainComponent.cpp:4866 sets hasBeenTriggered on every human fire. ClipTransportSync.h:33-38: playing=false -> player.setPlaying(false). Pitfall 7 (pitfalls.md:23). The plan's T-S3 PINS playing false after the clear; B-3 promises 'Fire a clip that is not in BPM Sync -> it plays'. Verified by reading, not run.",
      "severity": "MUST",
      "proposed_change": "Make the stop clear leave clips fire-ready: record each layer's clip playing before the clear and restore it after (a hand-paused clip stays paused), or reset hasBeenTriggered for cleared clips that were playing. Invert T-S3. Add T-S3b: fire, Stop, re-fire the same clip -> playing true. Make LR2 (b)/(d) re-fire a clip already fired earlier in the round."
    },
    {
      "id": "ST-2",
      "target": "RB1 stopAll + section 9 parked R83/HR-9; B-3 'WRONG' list",
      "claim": "A routine pad pressed while Stopped (or Paused) with its default Bar lead-in has no bar edge to wait for. The pad looks dead while stopped, then its clips start a bar after he presses play, on layers he emptied. That is the 'clips come back by themselves after play' that B-3 lists as WRONG.",
      "evidence": "RoutineEngine.cpp:535-546: the bar edge is a change of snap.totalBarCount. A stopped beat holds the count; plan NB-3 says startFromOne counts a beat and NOT a bar. The plan parks 'a pad pressed while stopped' (plan:780-784) and keeps pads' own quantize (question 147 default A, plan:720-723); B-3 text plan:687-688.",
      "severity": "SHOULD",
      "proposed_change": "Either say it in B-3 and add a Boris check 'press a pad while stopped', or give a pad press while the beat is not Running an Off lead-in (one line beside the old F18-style rule). Do not leave B-3's WRONG list contradicted by the plan's own parked case."
    },
    {
      "id": "ST-3",
      "target": "RB3 'A REPLAYED Resync while held starts the beat'; B-9",
      "claim": "He presses Stop, then replays a take that contains a Resync. The replayed Resync starts his stopped beat from the 1 by itself, while the layers are empty. B-9 and R104 only promise a replay will not pause or stop his beat; nothing tells him a replay can start it.",
      "evidence": "plan:197 ('A REPLAYED Resync ... while held: starts the beat too and leaves the nudge'); B-9 plan:706-709 and R104 plan:731 cover pause/stop only. FeatureSnapshot.h:117-120 lists 'take replay' among Resync sources.",
      "severity": "SHOULD",
      "proposed_change": "Make a Resync with Origin::Replay not start a non-Running beat (it still re-seats the 1 as today), at the same spot that already keeps the nudge for replays. Otherwise add the sentence to B-9/R104 and a clause in LR7."
    },
    {
      "id": "ST-4",
      "target": "B-3 'the picture is empty AT ONCE'; RB1 'global stack untouched'",
      "claim": "After Stop the wall is promised empty at once, but the global effect stack and its history are left alone. A global Freeze/Echo/feedback effect can keep showing the last picture or a trail. INFERRED, not run.",
      "evidence": "plan:683-684 (B-3) and plan:144-145 (global stack untouched; 'what the global stack makes of nothing'). CompositorEngine.cpp:552-559 binds feedbackTex_, the previous composited frame, for feedback effects; CompositorEngine.cpp:146-147 describes Freeze/Echo/feedback history kept across resizes. LR2 (a) checks strips and the API only, no pixel.",
      "severity": "SHOULD",
      "proposed_change": "Add a window-only render_frame luma check to LR2 (a) with a global Echo and a layer feedback preset on, plus a VG state (T7 with Echo on). If a ghost shows, say so in B-3 or have the clear reset that history. Do not promise 'empty at once' unmeasured."
    },
    {
      "id": "ST-5",
      "target": "NB-6 Q-c / Q-b: removing the clip's Snap and dropping its saved keys",
      "claim": "His verbatim words remove Quantize only 'from the top bar'. The clip's Snap goes on an uncorrected reading, and the save drops its keys. If he corrects R87 later, every show re-saved in between has lost his per-clip Snap values for good. A show he built so a clip lands on the bar silently fires at once.",
      "evidence": "BD:971 ('remove it from the top bar'); BD:1025-1027 (his R87 line shows a panel, says nothing about snap); plan:245-249 (Q-c 'rests on INFERRED consent'); K10 plan:775-776 admits the values do not come back; Q-b/Q-c drop the keys on load.",
      "severity": "SHOULD",
      "proposed_change": "Hide the Snap combo and make the engine ignore it now, but keep round-tripping the three keys inertly in the save until he answers R87 in words. Delete the keys in a later lane, or ask him as a numbered question first."
    },
    {
      "id": "ST-6",
      "target": "S4r bindings: 'Stop' (GlobalStop) kept beside 'Beat stop' (HR-3 left to the naming lane)",
      "claim": "The learn overlay offers a 'Stop' that only stops routines and a 'Beat stop' that blanks the wall; likewise 'Play / Pause' (audio file) beside the new Play / Pause. A pad learned on the wrong one does nothing visible mid-set: no clips leave, no beat stops.",
      "evidence": "plan:411 (S4r: old bindable Stop and Play/Pause NOT touched); plan:453 (HR-3 STANDS; wording is the naming lane's); Binding.h:40-41 GlobalPlayPause, GlobalStop; MainComponent.cpp:624-627 (that Stop stops routines 'and nothing else').",
      "severity": "SHOULD",
      "proposed_change": "Have S4r make the overlay titles pairwise distinct for the five Stop/Play/Pause targets, or hide the routine-only Stop, and put that in the T16/T17 VG states. Do not wait for the naming lane."
    },
    {
      "id": "ST-7",
      "target": "B-4 'Only Resync puts it back to 0'",
      "claim": "B-4 says only Resync changes the nudge number. Opening another show mid-set also silently replaces it with that show's saved number. If that was 0, his room alignment drops to 0 with no sign, which is the 'number silently returns to 0' case. Stop and Play are not the only events he must be told leave it alone.",
      "evidence": "plan:689-691 (B-4); ruling-nudge-row.md:381-383 (NA-8: the opened show's nudge amount still takes over and glides); BD:916-917 (his answer 63).",
      "severity": "NIT",
      "proposed_change": "Add to B-4: 'Opening a show sets the number to that show's own; only Resync, or opening a show, changes it.' Add a check line: open a show saved at a different nudge and watch the number."
    },
    {
      "id": "ST-8",
      "target": "NB-8 / question 145: nudge text placed between '<<' and '>>'",
      "claim": "His order is 'nudgeBack nudgeForward /2' (adjacent, as his picture), and R101 reads 'square cells side by side'. The default puts an 88 px text cell between the two nudge cells, breaking the pair he drew. It stands in the rhythm of the row until he answers 145.",
      "evidence": "BD:913-914; boris-clarify-135-143.md:94-96 (R101); plan:350-352 and plan:714-716 (145 default A puts the text between).",
      "severity": "NIT",
      "proposed_change": "Make B (the pair side by side, the text after RESYNC) the default, which matches his picture and words, since 145 asks him anyway. Cost: the rowOrder() line and T-RB3/T-RW1, as the plan itself says."
    }
  ],
  "strongest_point": "ST-1: Stop clears each layer through the existing clear, which sets the cleared clip's playing to false (Layer.h:587-592). The fire rule only auto-plays a clip that was never triggered (Layer.h:553-556, MainComponent.cpp:4866, Pitfall 7). So for any clip he has used, the first thing he does after Stop (fire it again) puts it on its first frame, paused. A BPM Sync clip then never moves, even after Play. The plan's T-S3 pins the flag at false, and B-3 and LR2 (b)/(d) promise the opposite.",
  "citations_rechecked": true
}
```

<!-- round-trip: compact re-serialisation equals the raw input: True; raw length 17809 -->
