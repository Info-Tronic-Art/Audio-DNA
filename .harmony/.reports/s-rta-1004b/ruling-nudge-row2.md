# RULING nudge-row2 -- architect ruling on the blind council's attacks on plan-nudge-row2.md (lane "nudge-row2": the beat-nudge lane, SECOND DELTA on Boris's answers 125-129, 135-137; readings R81, R85; s-rta-1004b)
Architect (opus, max effort; Fable is out of usage), 2026-10-04. Harmony decides after this; this is her working document.
Read-only. Pin: `git -C /Users/boriskarpman/projects/RealTimeAudio diff --stat 7bc6df7 -- src tests docs CMakeLists.txt`
printed nothing (main HEAD is 8b464a6, docs only; plain-file reads below are reads of 7bc6df7). The nudge worktree (stage
S1, being built) was NOT read. The two stopped sync-dial worktrees were NOT read: nothing of this delta comes from them.
Nothing was built, no test was run, no app or probe was launched. Every width below is arithmetic on cited lines.
Shorthands: M: = main at 7bc6df7 (paths under /Users/boriskarpman/projects/RealTimeAudio). PL2: = plan-nudge-row2.md, the
plan ruled on (RB1..RB10 its items, NB-1..NB-8 its amendments, E1..E21 its facts). RR: = ruling-nudge-row.md (NA-1..NA-18,
HR-1..HR-14, LR1..LR7). RN: = ruling-nudge.md (A1..A24). PL: = plan-nudge-row.md. FQ: = s-rta-1004b/facts-quantize.md (its
VERIFICATION section overrides its body). RA: = ruling-transport-answers.md. BD: / BL: = .harmony/binding-decisions.md /
boris-feedback-backlog.md.
Labels: VERIFIED (read at the pin by me), COMPUTED, INFERRED, ASSUMED.
Precedence: Boris's verbatim words > Harmony's adoption blocks > THIS RULING (amendments NC-1..NC-19) > PL2 > RR > RN > the
older plans. Where this ruling is silent PL2 stands; where PL2 is silent RR stands; where RR is silent RN stands.
Papers: .harmony/.reports/s-rta-1004b/attack-nudge-row2-papers.md (2 of 2 seats, 16 attacks; 17809 characters as dispatched
and as transcribed; the array is closed; the pretty-printed copy re-serialises to the dispatched text).
NOT READ by me, and nothing below rests on them except where it says so: ruling-transport-delta2.md; the bodies of
ruling-bf2-stops.md (its section list is mirrored) and plan-nudge.md; RN sections 1, 2 and most of 5; facts-actions-today.md;
facts-beat-controls.md; RIG-RULES section B; the pictures other than resolume-tempo-bar.png and resolume-bpm-sync-panel.png.

## 0 VERDICT
The plan NEEDS REVISION; its position stands. 16 attacks ruled: 13 ACCEPT, 3 PARTIAL, 0 REJECT. 19 amendments
(NC-1..NC-19): 14 answer the attacks (three of them also carry a finding no seat raised: NC-5's queue, NC-14's bar
size, NC-17's tooltip), 3 come only from re-reading the code the plan rests on (NC-2, NC-3, NC-6), 2 are bookkeeping
(NC-18, NC-19).

RULED FIRST

1. WHAT THE ROW'S STOP, PAUSE AND PLAY DO
 His words (BD:952-953, :1007, :1009): "126 and 127 stop clears all clips from layer strips, pause stops them, tempo
 setting stays the same. Make thee new buttons as I asked"; "135 the beat stops but tempo is not lost, just not
 playing"; "136 b". They REPLACE his answer to 111 (BD:918-920) for STOP (stop now also takes the clips off) and KEEP it
 for PAUSE (what is not BPM-based plays on).
 | what | STOP | PAUSE | PLAY |
 |---|---|---|---|
 | the nine beat fields | the "1": beatPhase 0.0, beatInBar 0, barPhase 0.0, the downbeat level true, barCount 0, phrasePhase 0.0, resyncBarOrigin = totalBarCount; totalBeatCount and totalBarCount do NOT change (a stop is never an edge). RR section 0 item 1 STANDS whole. | all nine held bit for bit. RR STANDS. | from stop: that hop is beat 1 -- totalBeatCount + 1 (ONE edge), beatInBar 0, no bar counted. From pause: the beat runs on from the held place, nothing is counted. RR STANDS. |
 | the tempo number | live, never 0 | live | live |
 | every clip on every layer | OFF, BPM-synced or not: the layer's tuple is cleared (a cut; a fade in flight ends; a queued trigger cannot exist). NO clip field is written (NC-1). | a BPM Sync clip gets dt 0 at its advance and stands on its frame; a clip that is not in BPM Sync plays on. NO clip field is written (RR NA-1 STANDS). | nothing is fired, nothing is written. After stop the layers stay empty until he fires. After pause the held clips run on from their frame. |
 | a clip's OWN pause (his answer 72, BD:904-905) | never written. A clip he had paused is still paused when he fires it again; a clip that was playing plays when he fires it again (NC-1: this is where the plan was wrong). | never written (the hold is not a stored state). | never written. |
 | beat-driven effects, signals, mappings | stand on the "1" | stand where they were | run again |
 | autopilot | beat steps wait (no edge); it has no clip to advance on an empty layer; its switches are untouched | beat steps wait; an end-of-clip advance of a clip that is not held still happens | counts on from the next edge |
 | a routine, playing or waiting | stopped IF it holds a layer -- the layer X's own rule, applied to every layer (NC-2; HB-1). A routine that holds no layer waits with the beat. | waits in place (its clock is the beat: RR NA-15) | goes on from where it stood |
 | a take being REPLAYED | not stopped; its next recorded fire puts a clip back; a Resync inside it does NOT start the beat (NC-5) | not held; the same | untouched |
 | a take being RECORDED | gets no point: not for the timer (RR NA-4), not for the clears | no point | no point |
 | the nudge number | untouched ("129 b") | untouched | untouched |
 | Undo | Stop pushes no step (NC-3 says what Cmd+Z still does on main) | none | none |
 WHERE "HELD BY THE ROW" LIVES: nowhere in the model. It is computed on the render thread each frame from the published
 byte (`beatRunning(snap.beatShiftState)`) and the clip's transport mode, and replaces that frame's dt by 0.0 (RR NA-1's
 `ClipTransportSync::syncDt`). No clip field, no layer field, no show key, no settings key, no Undo step. So play
 restores exactly what ran before (nothing was changed) and nothing of it is saved. PL2 RB2 STANDS whole.
 WHAT PLAY RESTORES: after PAUSE everything, because nothing was changed. After STOP nothing: the clips are gone by his
 word; the beat starts from the "1".

2. TAP AND RESYNC WHILE STOPPED OR PAUSED
 His words (BD:957-958): "128 tapping tempo does not start anything but when click resync, that is the 1 and it begins
 on that button push".
 A TAP: the tempo is taken; the state, the nine beat fields and the counters do not move.
 A RESYNC BY HAND (the row's RESYNC, a bound key or pad, REST, OSC): on the hop that applies it the beat runs from the
 "1" -- state Running, ONE edge, beatInBar 0, the nudge reads 0 as after any hand Resync. It travels in the same ordered
 queue as play / pause / stop, so "stop, then Resync" inside one hop ends running and "Resync, then stop" ends stopped
 (NC-5). With the nudge engaged the published count is the last published count + 1 EXACTLY (NC-6: the plan's case for
 this could not go green on the rule it cited).
 A REPLAYED Resync (a take): while the beat is not running it does NOTHING to the beat -- no field, no state, no
 counter. The timer is the stage's, not the recording's (NC-5; ST-3).
 WHAT A RESYNC DOES TO CLIPS THE PAUSE IS HOLDING: they run on from their held frame, with no jump -- main has no bar
 lock (M:Renderer.cpp:1657-1667, :1718-1736). When the transport lane's lock exists it cuts them into time on that
 press ("121 a", BD:943): the hop is published as a Resync AND a start, which is what that lane's rule reads.

3. WHERE LIVE QUANTIZE GOES, WHAT GOES, WHAT STAYS
 His words (BD:971-972): "This is not for live usage. We should remove it from the top bar and use it in the recording
 review screen which we have yet to create."
 WHERE: a lane of its own, "quantize-out" (branch lane/quantize-out from main), merged to main BEFORE the packet of this
 lane's S2 and BEFORE the transport lane's S1. Stages QO-P (the probe), QO-0 (Harmony's RED arms on main), QO-1 (engine
 and model), QO-2 (surfaces, probes, docs), VQ. PL2's W2 STANDS; its order is corrected (the probe must exist before
 QO-0 can run: NC-19).
 GOES: the top-bar "Quantize:" label and combo; the show's `quantizeMode` (member, enum, save line, load line); the
 Clip inspector's Snap combo; both gates (`Layer::triggerClip`'s forced-snap parameter and queue branch; the same
 parameter of `Composition::fire` / `triggerColumn`; `quantizeModeToForcedSnap`); the drain
 (`Layer::processPendingTrigger` and its call); the legacy beat-phase seek; a take's preamble row `comp/quantize`.
 STAYS: the pending slot's three fields inside the layer's tuple word, never set (Pitfall 63: no re-pack); the enum type
 `Clip::BeatSnapMode`; a routine's own quantize, its pad menu, its engine and its `forcedSnap` parameter (the two call
 lines pass `RoutineSnap::Off`); the take checkpoint's struct and keys; AND -- changed by this ruling -- each clip's two
 saved keys "beatSnap" / "beatSnapMode" and their two fields, read by nothing (NC-7: his verbatim words name the top
 bar; the clip's Snap goes on reading R87 / R100, so its control and its effect go now and its saved values are not
 destroyed until he has confirmed it in words).
 "The snap override of a binding": there is none; no Binding carries a snap (M:src/binding/Binding.h:24-47).
 A REPLAYED FIRE (a take's, a routine's) goes through the same handler, which has no gate: it lands at its recorded
 press time. An old take whose checkpoint carries Quantize replays each press where he pressed.
 A ROUTINE'S OWN QUANTIZE breaks no rule of his: his words name "the top bar"; reading R86 names "every clip you fire";
 the pads are replaced by actions (R94). It is not asked (NC-18) and not touched.
 WHICH EARLIER RULES THESE WORDS REPLACE: BF20's Quantize menu as a live top-bar control (BD:976-977); RR's reading R81
 (a quantised fire waits while the timer is held) and every row built on it. His line of 12:31:21 (BD:859-861) STANDS
 whole; its last clause ("unless it's set to be quantized") has no subject left, because nothing can be set to be
 quantised live any more. RR's question 127 default A (the routines stop stays as "R[]") is replaced by R85 confirmed;
 RR's 128 default A by his words on 128; RR's 129 default A by "129 b".
 WHAT THE NUDGE STILL MOVES: every reader of the published beat (PL2 RB7 STANDS whole). A fire and a knob are not
 readers; nothing of the nudge's arithmetic changes with Quantize gone.
 THIS LANE'S ADOPTED ROWS, stage by stage:
 | stage | void | amended | untouched |
 |---|---|---|---|
 | S1 | -- | T-N5b uses its two-line rule (a NOTE, not a STOP: section 4) | everything else |
 | S1r | RR NA-14; the constants `kGesturesStartTimer`, `kNudgeAfterStop`; T-G5's second arm | T-G6, T-G7, T-N17, T-N17b; the Resync site (NC-5, NC-6) | the five gated sites, `applyStop`, `startFromOne`, the hand tempo, T-G1..T-G4c, T-G8..T-G13 |
 | S2 | RN L3d; RN L3b as a row of its own | L3a, L3c (their reader); step 0 takes main in | the wiring, the save key, the route, the anchor, L1, L2, L4, L6, L9, L10 |
 | S2r | RR LR2 (b), (c) and the routine half of (d); RR NA-16's clip sentence; RR B-R5 | LR2, LR3, LR7, T-G14; `clearAllLayers`; no nudge statement in the Stop path | NA-1's seam, NA-15's clock, LR1, LR4a |
 | S3a | -- | the "topbar" dump has no Quantize widget | the rest |
 | S3m | "the Quantize: caption" in the give-up list; "R[]" | `rowOrder()`, `rowTexts()`, the widths, `fitTopBar` (NC-14, NC-15) | `nudgeText`, `parseNudge`, `bpmText`, `parseBpm` |
 | S3r | the routines stop; the combo "never given up" | the cluster ends at Link; the three old buttons leave; the cells | the editors, `setManualShown`, `pressRowControlForTest` |
 | S4 | -- | -- | all of it |
 | S4r | -- | "Beat stop" empties the layers; two old overlay titles (NC-16) | the seven targets |
 | S5 | every sentence on Quantize, on a waiting fire, on a routine pad's start | the pitfall sentence | the rest |

4. THE TOP ROW, AGAINST HIS PICTURE AND THE ROOM THERE IS
 His words (BD:1021-1022): "r85 go with the defined stop play pause that we discussed in 135 and place them at the top.
 use this similar layout across the top row where it fits: [Image #8]". The picture (looked at): one row of thirteen
 dark cells, flush, square except the wide ones -- circle, play (lit, filled light green), pause, stop, "BPM 256", "-",
 "+", two arrow-and-bar glyphs, "/2", "x2", "TAP", "RESYNC". No nudge text, no bar text.
 THE BAR THE APP HAS (VERIFIED, and not what the plan assumed): the top bar is 34 px high, not 40
 (M:src/MainComponent.cpp:2688), and at a 1728-wide window it is 1720 wide (:2683); `TopBar::resized()` takes 4 px off
 each side and 2 off top and bottom (M:src/ui/TopBar.cpp:531). So the inner area is 1712 x 30. The plan's 36 px cells
 and its "1687 of 1720" are re-based (NC-14).
 WHAT STAYS IN THE BAR, left to right: the Audio block (source, gain) | "Bar N" | THE THIRTEEN CELLS | the text "nudge X
 ms" | the state word (LOCKED / SEARCHING) | Manual | Link | free space | Master Signal | Master | Outputs | FPS | DSP.
 THE CELLS: his thirteen, in his order, 30 x 30, 1 px apart, corners square; "BPM" and the number share one cell 96
 wide; TAP 48; RESYNC 68. COMPUTED: 10 x 30 + 96 + 48 + 68 + 12 gaps = 524. The nudge text (88, after a 4 px gap) sits
 AFTER RESYNC by default, so the two nudge cells stay side by side as he drew and listed them (NC-15; question 145).
 Exactly one of play / pause / stop is lit, from the published byte.
 THE FIT (COMPUTED; the Master Signal label 86 and the text box 88 are ASSUMED until VG-0): 240 + 46 + 524 + 92 + 6 +
 200 + 522 = 1630 of 1712 at his 1728: nothing is given up, 82 px to spare; the label would have to be wider than 168
 to cost him anything. At 1512: DSP, FPS and the state word are given up (34 to spare). At 1280: those and both Master
 faders (118 to spare), as RR already ruled for 1280. No "compact cells" step exists any more.
 WHAT THE LEAVING FREES, against today's bar (1553 of 1712 with Manual off): the Quantize cluster 161, the three old
 buttons 82, the five multiplier buttons 140, the Manual BPM field 64.
 VG-0 is still needed (the baseline pictures; the label's real width; the bar's real size in pixels) and now has a
 decision table (NC-14). The visual states are re-stated in section 5.

WHAT CHANGES
 - The plan's Stop left every clip it took off PAUSED: the layer's own clear sets the old clip's `playing` false, and a
   clip that was ever fired is not set playing again by a fire (ST-1; V2, V3). Its own case T-S3 pinned that flag at
   false, and its promise to Boris said the opposite. Stop now clears the tuple only, through the layer's own pure
   function, and writes no clip field (NC-1).
 - The plan's reason for calling "stop all routines" from Stop was a misreading: the per-layer call the strip's X makes
   also reaches a pad that is waiting (V5). Stop is the X on every layer, no more (NC-2).
 - "Cmd+Z brings nothing back" is false on main: a fire is an Undo step whose undo restores the layer (V6). This lane
   promises only that Stop adds no step (NC-3).
 - The plan's safety argument for the dead pending slot rested on a lint that could not pass and a unit case that could
   not reach its subject (GA-2, GA-1): both are rewritten to what can be proved (NC-8, NC-9).
 - "Resync starts a held beat" had no mechanism and no order against Stop; its nudged case asserted a count the cited
   rule does not give; and a replayed Resync would have started his stopped beat (ST-3): NC-5, NC-6.
 - The row was drawn for a 40 px bar the app does not have (GA-8; V14): NC-14.
 - Two PARTIALs keep the plan's choice and change what is destroyed or built: the clip's Snap values stay in the file
   (ST-5: NC-7); a pad pressed while stopped is said to him, not re-built (ST-2: NC-17).
Nothing here loosens a pre-registered bar. RN's L3b / L3c / L3d and RR's LR2 (b) / (c) are VOID because their subject (a
fire that waits) is removed by his words; the rows that replace them are stated before any run, each with a RED arm.
What no run has established: everything. Every rule here is from reading 7bc6df7, the two rulings and his record.

## 1 FACTS RE-DERIVED (every seat citation and every plan fact a ruling rests on, re-read or re-computed)
V1  VERIFIED M:src/model/Layer.h:186 (`struct Layer`: its members are public down to :532), :332-355 (`updateRuntime(fn,
    maxAttempts)`: fn is a pure function of the tuple, retried until its compare-exchange lands; it runs NO clip tail),
    :380-390 (`static clearedNext`: previous = the active ref, active = none, crossfadeProgress = 1.0, the pending slot
    emptied -- one word). So a layer can be cleared through its own API without any clip write.
V2  VERIFIED M:Layer.h:492-503 (`clearActiveClip` = `updateRuntime(clearedNext)` THEN `applyClearTail`), :587-592 (the
    tail: the old active clip's `playing = false`). M:src/render/ClipTransportSync.h:33-41 (`pushIntent`: not wanted ->
    `player.setPlaying(false)`). ST-1's first two citations hold.
V3  VERIFIED M:Layer.h:548-557 (`applyActivationTail`: playhead to the in-point, beatsPlayed 0, and `playing = true`
    ONLY `if (to.activeRef() != from.activeRef() && !clip->hasBeenTriggered)`); M:src/MainComponent.cpp:4866
    (`clip->hasBeenTriggered = true` for the clip active after a clip fire -- the only write of that flag in src: grep);
    the column handler (:5040-5133) never sets it; M:src/core/TriggerCommands.h:52-56 records the same hazard for
    "first-ever-trigger -> undo -> re-trigger"; docs/claude/pitfalls.md:23 (Pitfall 7). COMPUTED: fire a clip by a
    cell, a key or `/api/trigger_clip`; clear its layer by `clearActiveClip`; fire it again -> it is the layer's clip
    with `playing` false. ST-1 holds. INFERRED, not run: the strip's X does this on main today (SF-C1).
V4  VERIFIED M:src/ui/ClipCell.cpp:151-152 and M:src/ui/DeckView.cpp:291 (a cell is lit from the layer's active ref, not
    from `playing`); M:src/ui/LayerStrip.cpp:372-406, :719-763 (the strip reads `playingClip()`); M:src/model/
    Autopilot.cpp:100 (`if (clip == nullptr || !clip->playing) continue;`). A clip that is not on a layer and still has
    `playing` true is what every clip REPLACED by another fire already is (no tail runs there: Layer.h:361-374).
V5  VERIFIED M:src/recording/RoutineEngine.cpp:173-202 (`struct Running`: `bool pending = true`, `std::vector<int>
    layers`), :729-749 (a fired routine is pushed into `running_` with its footprint BEFORE it starts), :808-817
    (`stopOnLayer` walks `running_` and stops every routine whose footprint holds the layer), RoutineEngine.h:131-134
    ("for every pending/running routine whose footprint holds shared layer"), :793-806 (`stopAll` also resets every
    pad's last-run marks). M:MainComponent.cpp:730 (the X calls `stopOnLayer` first); M:LayerStrip.cpp:345-346 (the X's
    tooltip says so). PL2 E5's "`stopOnLayer` stops only RUNNING routines" is WRONG: a waiting pad is in `running_`.
V6  VERIFIED M:TriggerCommands.h:85 (`undo()` = `apply(before_, ...)`), :119-130 (`apply` = `layer->setRuntime(runtime)`
    + the target's `playing`), :97-117 (same-layer fires merge and keep the ORIGINAL before-state); M:MainComponent.cpp:
    4976-4983 (a human fire pushes a `TriggerClipCmd`). COMPUTED: fire A, fire B on one layer, Stop, Cmd+Z -> the layer's
    tuple is the one from before the run. PL2 B-3's "Press Cmd+Z -> nothing comes back", R107 and the second clause of
    T-S2 are false whenever the last undoable step is a fire. His rule (BD:764-766: "cmd-z does not affect anything in
    layer strip"; "If a clip is triggered and plays, it is not affected.") is not built on main; it belongs to the
    transport / undo lane.
V7  VERIFIED M:MainComponent.cpp:723-748 (the strip's X: `stopOnLayer`, `clearActiveClip`, a `ClearActiveClipCmd` pushed,
    then `refreshPreviewFromShow()` with the 2026-07-30 comment: without it a shader or MilkDrop source keeps
    rendering through the empty-compositor fallback; no `capture(` in the lambda). M:src/api/ApiServer.cpp:269-271 and
    :1303-1322 (`POST /api/render_frame` {"output_path"} = `Renderer::captureFrame(..., completeFrame = true)`: an
    offscreen frame written to a PNG; neither a screen capture nor input). GA-4's citations hold.
V8  VERIFIED M:src/render/CompositorEngine.cpp:145-150 (Freeze / Echo / feedback pictures are kept histories) and
    :551-561 (`feedbackTex_`, the previous composited frame, is bound for an effect that reads it). ST-4's citations
    hold; what a history shows after the layers empty is NOT established by reading.
V9  VERIFIED M:MainComponent.cpp:36-43 (`quantizeModeToForcedSnap`), :4848-4850 and :5078-5080 (the two handlers: the
    column one is its own site, `composition_.triggerColumn(..., forcedSnap, ...)`), :4279-4281 and :6252-6256 (the
    routine tick and the routine fire), :2012-2015 (a take's preamble row). M:ApiServer.cpp:178, :182 (`trigger_clip`,
    `trigger_column`). GA-5's citations hold (its ":5078" is the snap's computation; the call is :5080).
V10 VERIFIED M:MainComponent.cpp:4863-4889: the legacy seek sits in `handleClipTrigger`, on the clip that is active
    AFTER the fire; it reads `analysisThread_.getFeatureBus().read()` and seeks through `previewPanel_.getRenderer()`.
    A fire that was only queued leaves the OLD clip active, so the seek then runs on that one; a re-fire of the ACTIVE
    clip is never queued (Layer.h:424) and is the one path on which the seek reliably hits its own clip. GA-1 holds: no
    case over Layer or Composition can reach it.
V11 VERIFIED by grep of src at the pin (comments included): `forcedSnap` -- MainComponent.cpp 4, RoutineEngine.cpp 13,
    RoutineEngine.h 5, Layer.h 3, Composition.h 4; `beatSnap` as a whole word -- ClipInspector.cpp 1, MainComponent.cpp
    1, Layer.h 1, Clip.h 2, Clip.cpp 4; `beatSnapMode` -- ClipInspector.cpp 2, Layer.h 5, Clip.h 5, Clip.cpp 5;
    `quantizeMode` -- TopBar.cpp 1, MainComponent.cpp 9, Program.cpp 1, PerfState.cpp 2, PerfStateCapture.cpp 1,
    PerfState.h 1, Composition.h 4 (+ one comment each in RoutineEngine.h / .cpp). M:src/model/Clip.cpp:72-73 (the
    two keys written), :229-233 (read, with the legacy upgrade); M:src/model/Composition.h:742 / :880. PL2 Q-h keeps
    the routine engine's parameter, so PL2 T-Q5's "none of `forcedSnap` ... occurs in src" cannot pass. GA-2 holds.
V12 VERIFIED M:RoutineEngine.cpp:763-771 (`fire`: mode = `effectiveSnap(forcedSnap, own)`; it starts inside the call
    only when mode is Off, else the routine stays pending) and :551-566 (`tick`: for a pending routine mode is
    recomputed from the TICK's forced snap and `dueNow(mode, ...)` decides). COMPUTED: with only the tick mutated to
    Bar an "off" routine still starts inside `fire` -- GA-3 holds. And with only the FIRE call mutated to Bar it is
    pending for one tick and then starts (the tick computes Off): the seat's first remedy would not go RED either.
    Both lines must be mutated for "the off routine waited".
V13 VERIFIED M:Autopilot.cpp:74-76, :96-101: the beat loop skips a layer with no clip. GA-7 holds.
V14 VERIFIED M:MainComponent.cpp:2681-2690 (`area = getLocalBounds().reduced(4)`; `topBar_->setBounds(area.
    removeFromTop(34))`); M:TopBar.cpp:529-631 (`resized()`: `reduced(4, 2)`; Audio 38 + 90 + 4 + 30 + 70 + 6 + 2 = 240;
    the three buttons 24 + 1 + 24 + 1 + 24 + 6 + 2 = 82; wheel 26 + 2; "Bar N" 44 + 2; tempo 50 + 64; Tap 32 + 2,
    Resync 50 + 2, Manual 80 + 2, Link 50 + 2; the Manual field 60 + 4; five buttons 26 x 5 + 4 + 6 = 140; Quantize
    55 + 100 + 6 = 161; right block DSP 55, FPS 45, 6, Outputs 100, 4, Master 90 + 42, 4, Master Signal 90 + its measured
    label). COMPUTED: today's left block 1031 (Manual off), right block 436 + the label; at a 1728 window the inner
    area is 1712 x 30. M:tests/test_master_signal_link.cpp:187, :300, :340, :373 and test_topbar_link_toggle.cpp:54,
    :103 size the bar 1728 x 40 or 1280 x 40 -- the tests' own size, not the app's. PL2 E7's sums hold; PL2 NB-8's
    "the bar is 40 high; the inner area is 36" and RR V15's "inner width 1720" do not. GA-8's doubt was right.
V15 VERIFIED M:MainComponent.cpp:5783-5788 (every Resync -- the button :601, REST :1900, OSC :2358, a binding :7849 --
    reaches `applyTempoCommand("resync", ...)`; a replayed point reaches it with `Origin::Replay` at :1980); M:src/
    analysis/FeatureSnapshot.h:118-120 (a take replay is a Resync source). RN A2 (RN:279-286) puts the nudge's zero
    request in that branch "only when origin is not Replay", BEFORE the tracker call. ST-3's citations hold.
V16 COMPUTED from RN A1 / A2 (RN:239-296) and PL:219-250, for a hand Resync that starts a held beat with the nudge
    engaged (F = the count last published; the tracker's `startFromOne` adds 1 to its own count):
    from STOPPED: the stop hop adopted the published count into the tracker, so S = F + 1 and A2's clamp gives F + 1;
    from PAUSED, nudge LATER, the published count one BEHIND: S = F + 2, the clamp gives F + 1;
    from PAUSED, nudge EARLIER, the published count one AHEAD: S = F, the clamp gives F -- NO edge.
    PL2's T-N17b second arm (D = +250 at phase 0.7, "totalBeatCount' == F's + 1 EXACTLY") cannot be made green on the
    rule PL2 cites; RN A24 would stop the builder. A written rule is owed (NC-6).
V17 VERIFIED M:src/analysis/BPMTracker.cpp ordering as RR V2 records it: a pending tempo request is applied at the START
    of a hop (:79-84), a pending Resync LAST (:357-362). PL:229-234 takes the run queue "where it takes the tempo
    request". COMPUTED: with Resync on today's path, "Resync, then Stop" inside one hop applies the Stop first and the
    Resync after it -- and under PL2 NB-3 that Resync would START the beat he had just stopped.
V18 VERIFIED M:src/binding/Binding.h:24-47 (`GlobalPlayPause`, `GlobalStop`; append only); M:MainComponent.cpp:7584-7588
    (overlay titles "Play / Pause", "Stop"), :7852-7862 (the first runs the audio file, the second stops routines);
    PL:416-422 (the seven new titles "Beat play", "Beat pause", "Beat stop", "Tempo -", "Tempo +", "Tempo /2", "Tempo
    x2"; "Stop" renamed "Stop routines": RR HR-3, adopted at its default, its wording left to the naming lane).
V19 VERIFIED PL:406-411: the row's tooltips; stop's is "Stop the BPM timer on the 1. Everything set to BPM waits until
    play." -- untrue once stop takes the clips off. PL2 says "RR NA-5's tooltips stand" and misses it.
V20 VERIFIED BD:971-972 (his words name "the top bar"), boris-clarify-135-143.md:43-45 (R86, R87 told 21:03:09),
    BD:1025-1027 and BL:793 (his line on r87 is a picture of Resolume's clip panel; looked at: Speed, Beats, back /
    pause / play, two menus, no snap), boris-clarify-135-143.md:91-93 (R100, told with his LAST message: he has not had
    a turn to correct it). ST-5's reading of the record holds.
V21 VERIFIED BD:913-914 (his order ends "bpm- bpm+ nudgeBack nudgeForward /2 *2 tap resync"), boris-clarify-135-143.md:
    94-96 (R101: "nudge back, nudge forward" side by side), BD:911 ("61 b but lets call it "nudge X ms""). ST-8 holds.
V22 VERIFIED RR:377-383 (NA-8: an opened show's nudge amount takes over), BD:916 ("good (this is just nudge amount)"),
    RN's S2 row ("New -> 0"). ST-7 holds.
V23 VERIFIED RA:361-381 (TB-7): after the transport lane a clip's own pause is `paused` / `pausedAt`, written only
    through TransportPause.h; "NOTHING ELSE writes the pause: no fire, no column fire, no clear of a layer ... no beat
    timer". A stop that writes no clip field needs nothing from that lane and breaks nothing in it.
V24 VERIFIED FQ section 1 Q2 and VERIFICATION A.14, C-3: a replayed fire goes through the same handler; the take
    checkpoint's pending pair is restored by nothing. FQ Q4 + C-1: the tests and probes a removal turns red (PL2's list
    under RB6 matches it; not re-counted here).

## 2 ATTACK RULINGS (one row per attack; "decided by" = the line that settles it)
| id | sev | verdict | decided by | amendment |
|---|---|---|---|---|
| GA-1 | MUST | ACCEPT | V10: the seek lives in the handler and needs a bus read and a player; no mutant app carries MQ1; QL1's playhead clause fails on main for the queue's reason, not the seek's. | NC-9 |
| GA-2 | MUST | ACCEPT | V11: PL2 keeps `forcedSnap` in RoutineEngine (Q-h) and forbids the token in all of src. | NC-8 |
| GA-3 | SHOULD | ACCEPT | V12: a tick-only mutant leaves an "off" routine starting inside `fire`. Re-derived further: a fire-only mutant is not RED either; both lines, plus a lint on the two literals. | NC-11 |
| GA-4 | SHOULD | ACCEPT | V7: the X's own history shows the bug; no row reads a frame; `render_frame` exists on main. | NC-4 |
| GA-5 | SHOULD | PARTIAL | V9: the column handler is its own site and no live arm drives it -- ACCEPTED (an arm of QL1 and a unit case). The extra mutant is not needed: the tree before the change IS "a forced snap left in the column handler", and it is that arm's RED arm. | NC-10 |
| GA-6 | SHOULD | ACCEPT | PL2:583-590: L3b reads no nudge code and has no RED arm of its own. It stops being a row: QL1 is re-run on the lane with the nudge set. | NC-12 |
| GA-7 | NIT | ACCEPT | V13. | NC-13 |
| GA-8 | SHOULD | ACCEPT | V14: the inner height is 30, not 36, and the inner width at his window 1712, not 1720; PL2:407's "the design does not change" had no consequence. | NC-14 |
| ST-1 | MUST | ACCEPT | V2, V3: the cleared clip is left `playing` false and a fired-before clip is not set playing by its next fire. | NC-1 |
| ST-2 | SHOULD | PARTIAL | RoutineEngine.cpp:542 (a bar edge is a change of totalBarCount) and RR NA-16: a pad pressed while stopped starts one bar after play -- ACCEPTED as a thing he must be told (B-3, reading R111). REJECTED: an Off lead-in for such a press is a routine clause; Harmony constraint: none is built in this lane. | NC-17 |
| ST-3 | SHOULD | ACCEPT | V15, V17; RR NA-4 / LR7 BAR 3 already promise a replay never changes the timer; his words name "click" and "button push". Ruled stricter than the seat asked: a replayed Resync on a held beat moves nothing at all. | NC-5 |
| ST-4 | SHOULD | ACCEPT | V8: histories exist; nothing read says what they show after a stop. A frame is read for the plain case; the effect case is measured, both outcomes ruled; what stop should do to a held picture is his. | NC-4 |
| ST-5 | SHOULD | PARTIAL | V20: his verbatim words carry the top bar; the clip's Snap rests on a reading he has not confirmed in words. ACCEPTED: the two clip keys keep round-tripping, read by nothing. REJECTED: asking him again -- R87 / R100 stand as told; nothing is destroyed while they wait. | NC-7 |
| ST-6 | SHOULD | ACCEPT | V18: "Stop" beside "Beat stop", "Play / Pause" beside "Beat play" in one list; HR-3's rename was adopted and then left without a stage. | NC-16 |
| ST-7 | NIT | ACCEPT | V22. | NC-17 |
| ST-8 | NIT | ACCEPT | V21: his list, his picture and reading R101 all put the two nudge cells side by side. | NC-15 |
Reconciliation: GA-4 and ST-4 both want a frame read after Stop and get one clause and one measurement. GA-8 and ST-8 both
change the row's arithmetic and are ruled together in NC-14 / NC-15. No two seats conflict.

## 3 AMENDMENTS (numbered; each OVERRIDES the plan body where they differ)
The real-time rules hold in every amendment: nothing touches the audio callback; the analysis thread gains branches and
one more value of a 2-bit command (no allocation, no lock, no system call); the render thread reads the byte RR already
gave it and never waits; no mutex is added; every new function runs on the message thread except where it says "analysis
thread". No slider and no popup menu is added. No text announces an event or a failure: a stopped beat is a lit cell and
empty strips.

NC-1  STOP'S CLEAR WRITES NO CLIP FIELD (ST-1). REPLACES the layer call in PL2:131-134, PL2:516-517 (T-S3), and gives
      PL2:145's "A clip's own pause: untouched" its proof.
      - src/model/Composition.h gains, beside `fire` / `triggerColumn`:
        `int Composition::takeAllClipsOff() noexcept;`
        -- message thread. For every shared layer whose tuple names an active clip: ONE `updateRuntime` whose pure
        function returns `Layer::clearedNext(r)` while r's active ref is valid and r itself otherwise (V1). Nothing
        else: no clear tail, no clip field read or written. A layer with no clip is not touched, so a second call
        changes nothing. Returns how many layers' tuples changed.
      - Why not `clearActiveClip`: V2 + V3 -- its tail leaves the clip `playing` false and the next fire does not set it
        again. Why not "read `playing`, clear, write it back": two more writes of a field the render thread
        compare-exchanges every frame (M:ClipTransportSync.h:44-53), for a value that need never change. Why not reset
        `hasBeenTriggered`: it changes what a take captures at the next fire (`triggerWillAutoPlay`, M:MainComponent.
        cpp:192-198).
      - What a clip taken off by Stop then is: exactly what a clip REPLACED by another fire already is (V4) -- off its
        layer, its own play state as he left it. Fired again it plays if it was playing, and is paused if he had paused
        it (BD:748: "when you pause a clip and then fire it, it stays, paused").
      - `void MainComponent::clearAllLayers(Origin origin);` (PL2's name and signature stand) = NC-2's routine call for
        every layer, then `composition_.takeAllClipsOff()`, then ONE `refreshPreviewFromShow()`, then ONE deck-view
        refresh. No Command, no `capture(`. `setBeatTimer(BeatRun::Stopped, origin)` calls it FIRST, then
        `requestRun(Stopped)`; it is the only caller.
      - The strip's X is NOT changed: it keeps its tail (SF-C1; the transport lane's TB-7 owns it, V23).
      - Pitfall 63: the tuple is written through the Layer API only, one compare-exchange per layer; no bit of the word
        moves. Harmony runs `.harmony/probe-tsan-unit.sh` after S2r all the same (a new message-thread writer).
      - Cases T-S1, T-S3 (inverted), T-S3b, T-S5; mutant MS10; live LR2 (b).
NC-2  STOP AND ROUTINES: THE X'S OWN RULE, ON EVERY LAYER (own finding; V5). REPLACES PL2 RB1 fork (b) and
      "`routineEngine_.stopAll()`" in PL2:132, T-S4, reading R103, risk K3 and the default of PL2's HB-1.
      - `clearAllLayers` calls `routineEngine_.stopOnLayer(l)` for every shared layer l, BEFORE `takeAllClipsOff()` --
        what the X does (M:MainComponent.cpp:730), once per layer. It reaches a running routine and a waiting pad alike.
      - It is not a new routine clause: no line of the routine engine changes, and "stop all routines" (the old "[]"
        button's job, `stopAll()`) is NOT re-homed in the row. A routine whose footprint holds no layer is left alone
        and waits with the beat (RR NA-15).
      - Why not no call at all (PL2's B3): a routine that fires clips would put them back after play, against what
        question 135's option A told him (the question's words, not his: "the layers stay empty until you fire clips").
      - T-S4 is STRUCK as a unit case (it needs the app). Its promises are proved by lint T-G14 (the body of
        `clearAllLayers` holds `stopOnLayer(` before `takeAllClipsOff(`, and no `capture(`), by LR2 (a) / (d) in the
        routine rounds, by LR7 BAR 1, and by mutant MS7 (the per-layer call removed).
      - HB-1 is Harmony's: default this; alternative `stopAll()`; alternative none.
NC-3  STOP IS NOT AN UNDO STEP, AND THAT IS ALL THIS LANE PROMISES ABOUT CMD+Z (own finding; V6). REPLACES PL2:514-515
      (T-S2), the Cmd+Z sentences of PL2 B-3 (PL2:685, :688) and reading R107 (PL2:735).
      - Stop pushes no Command (PL2's C1 stands; BD:764-766 is its ground). Lint T-G14: the body of `clearAllLayers`
        holds no `pushCommands(` and no `Cmd>(`. Mutant MS5 is RED on that lint.
      - T-S2's second clause is STRUCK: on main a fire IS an Undo step, and its undo restores the layer's tuple from
        before the run (V6). No case of this lane asserts what Cmd+Z does after a stop.
      - B-3 carries a Cmd+Z sentence only when the build he checks contains the transport lane's Undo change for the
        layer strip (HB-8). Until then reading R107 says what is true.
NC-4  THE PICTURE AFTER STOP: A FRAME IS READ; WHAT AN EFFECT HOLDS IS MEASURED AND ASKED (GA-4, ST-4). ADDS to PL2 LR2
      (a), to the mutant list, to B-3; question 147.
      - LR2's fixture gains a procedural source clip on a fourth layer (the path the X's 2026-07-30 fix is about: V7).
        E0 = a `POST /api/render_frame` right after the load, before any fire. LR2 (a) gains: a `render_frame` taken
        300 ms after the applied poll differs from E0 by a mean absolute difference of at most 2.0 (of 255) per channel.
        Mutant MS9 (app): `clearAllLayers` without `refreshPreviewFromShow()`.
      - FM-S2 (Harmony; an INFO arm of LR2, after S2r): the same round once each with (i) a global Echo, (ii) a global
        effect that reads the feedback texture, (iii) a layer feedback preset, (iv) a global Freeze switched on; the mean
        absolute difference from E0 (of 255, per channel) of a `render_frame` 0.3 s, 1 s, 3 s and 10 s after the stop.
        "INFO  LR2 stop under <effect>: diff <a> <b> <c> <d>". No bar.
        Outcome A (arms i-iii at most 2.0 by 3 s): B-3 as worded in section 6; nothing is built.
        Outcome B (a picture still stands at 10 s in an arm other than Freeze): reported to Boris with question 147;
        nothing is built without his answer.
        Freeze holding its picture is what Freeze is for; B-3 says so under either outcome.
      - No history is reset by Stop in this lane (the X resets none; he did not ask; his word is "clears all clips from
        layer strips"). If he answers 147 B it is one architect line: which histories, cleared on which thread.
      - VG state T7b (section 5).
NC-5  A HAND RESYNC IS A COMMAND OF THE RUN QUEUE; A REPLAYED RESYNC NEVER MOVES A HELD BEAT (ST-3; own finding V17).
      REPLACES PL2 RB3's rule (PL2:185-199), RR NA-14, PL's site (4) (PL:205-206), T-G6 and T-G5's second arm.
      - No constant is declared: `kGesturesStartTimer`, `kTapStartsTimer` and `kResyncStartsTimer` do not exist.
        Question 128 is answered in his words; there is nothing left to switch.
      - src/model/BeatTimer.h: the queue's 2-bit command gets its fourth value:
        `enum class RunCommand : uint8_t { Play, Pause, Stop, ResyncByHand };`
        (pack / unpack are over this enum; `BeatRun` stays the three STATES).
      - `void BPMTracker::requestResyncByHand() noexcept;` -- pushes ResyncByHand onto the SAME queue word as
        `requestRun` (one compare-exchange loop; the request sequence raised after the write, Pitfall 48).
        `requestResync()` keeps its signature and its place in the hop (applied last) and is from now on the REPLAY's
        entry.
      - The queue is taken at the start of a hop and its commands are applied in order. ResyncByHand with the timer
        Running: it sets the tracker's own pending-Resync flag, so the Resync is applied last in THIS hop by the same
        function main uses. ResyncByHand with the timer not Running: `startFromOne()` (state Running,
        `++totalBeatCount_`, the "1" block, RR NA-13's start-hop flag), `appliedResyncs()` + 1 and `appliedStarts()`
        + 1; `appliedStops()` unchanged.
      - A pending Resync that reaches the END of a hop while the timer is not Running is DROPPED: no field, no state,
        no counter. That is a replayed Resync on a held beat -- or a hand Resync followed by a Stop inside one hop,
        where the stop's own "1" block stands.
      - A TAP while held: PL's site (3) stands (the tempo is applied, its realign skipped).
      - Message thread, the "resync" branch of `applyTempoCommand` (M:MainComponent.cpp:5783-5788), from stage S2r:
        origin not Replay -> RN A2's two statements (S2 puts them there), then `tracker->requestResyncByHand()`; origin
        Replay -> `tracker->requestResync()` as today. ONE condition decides both: the one RN A2 wrote.
      - What changes for a hand Resync while RUNNING, said so nobody finds it in a diff: it is applied in the hop that
        takes the queue, not in a hop already under way when it was posted -- at most one hop (10.7 ms) later than
        main, and identical in a test that posts between hops (T-G6d pins that).
      - RN H-3 is AMENDED: a replayed Resync resyncs a RUNNING beat and leaves the nudge (as ruled); on a held beat it
        does nothing.
      - Cases T-G6, T-G6b, T-G6c, T-G6d, T-G7's two new arms; mutants MR35, MR36, MR37; live LR2 (f), LR7 BAR 4.
      - Harmony constraint (proposed, for the transport lane): S4t's "a Tap keeps the bar" must not move a beat field
        and must not start the timer while it is not Running, and S4t's exit re-runs tests/test_beat_timer.cpp whole
        (T-G4c and T-G6b are the cases that would catch it). And its lock: a hand Resync that starts a held beat is
        published as a Resync AND a start; "121 a" applies on that hop.
NC-6  A RESTART ON A START HOP PUBLISHES THE LAST PUBLISHED COUNT + 1, EXACTLY (own finding; V16). ADDS one sentence
      to RN A2's step 2; REPLACES PL2:192-196 and T-N17b.
      - BeatShift step 2 (RESTART: engaged, a zero request pending, the tracker applied a Resync on this hop): when
        `startApplied` is also set, totalBeatCount' = F.totalBeatCount + 1 EXACTLY -- not the clamp into [F, F + 1].
        The rest of step 2 is A2's: S's "1" block; totalBarCount' = max(F's, S's); resyncBarOrigin' = totalBarCount';
        Da = 0; not held; `adoptCounters` when either counter differs from S's -- the same one call site.
      - A hop with several commands: its FINAL state decides. Not Running at the end and a stop applied -> RR's step 2b.
        Running at the end, a start applied AND a Resync applied -> this rule. Running at the end, a start applied and
        no Resync -> RR's start (PL:245-250).
      - Not engaged (step 1): no byte is written; the tracker's own count rose by exactly 1.
      - `adoptCounters` may now RAISE the tracker's beat count by one (the EARLIER arm of V16, where S = F). RN H-9's
        default lets a restart set both counters; its pre-ruled alternative covers this case unchanged.
      - T-N17b restated with three arms; mutant MR27' = the clamp kept on a start hop, RED on the EARLIER arm.
NC-7  QUANTIZE-OUT: THE CLIP'S TWO SAVED KEYS STAY IN THE FILE, READ BY NOTHING (ST-5). REPLACES PL2 Q-c's "the fields
      `Clip::beatSnap` and `Clip::beatSnapMode`, their save lines; the LOAD reads "beatSnap" and "beatSnapMode" and
      drops them" (PL2:246-247), T-Q-C2's clip clause, QL2's clip clause, the Snap half of R110, risk K10, and "the
      model half of Q-c" in QO-1's row.
      - GOES now: the Clip inspector's combo and its row slot (QO-2); every reader of the two fields outside Clip.h /
        Clip.cpp -- the gate in `Layer::triggerClip`, `processPendingTrigger`, the legacy seek (QO-1).
      - STAYS: `Clip::beatSnap`, `Clip::beatSnapMode`, their copy lines (M:Clip.h:267, :317, :357-358), the two save
        lines and the load with its legacy upgrade (M:Clip.cpp:72-73, :229-233). quantize-out does NOT edit
        src/model/Clip.h or Clip.cpp.
      - The show's own `quantizeMode` GOES whole (his words name it): no member, no enum, no save line and NO load
        line -- a key nobody reads is ignored by the loader, so the token leaves Composition.h entirely (PL2 Q-b's
        "the LOAD reads the key and drops it" is replaced by "no line").
      - When the two fields go: HB-5.
      - Proof that nothing reads them: lint T-Q5 clause (d).
NC-8  THE LINT THAT CAN PASS (GA-2). REPLACES PL2:474-477 (T-Q5) and the lint half of risk K2.
      T-Q5, in tests/test_quantize_out.cpp, over src with `//` comments stripped (string literals are NOT stripped):
      (a) `forcedSnap` occurs only in src/recording/RoutineEngine.h and RoutineEngine.cpp.
      (b) none of `quantizeModeToForcedSnap`, `processPendingTrigger`, `QuantizeMode`, `render_pending_fired`,
          `quantizeSelector`, `beatSnapSelector` occurs anywhere.
      (c) `quantizeMode` occurs only in src/recording/PerfState.h, PerfState.cpp and PerfStateCapture.cpp.
      (d) `beatSnap` (case-sensitive: it also matches `beatSnapMode` and does not match the type `BeatSnapMode`)
          occurs only in src/model/Clip.h and src/model/Clip.cpp.
      (e) in src/model/Layer.h, src/core/DeckCommands.h and src/recording/PerfStateCapture.cpp every statement that
          assigns `pendingTriggerColumn` assigns `-1`; every one that assigns `pendingDeckId` assigns
          `ClipRef::kNoDeck`; every one that assigns `pendingTriggerSnapOverride` assigns its Off value (0 in
          PerfStateCapture.cpp); and no other file of src assigns any of the three -- except the take checkpoint's
          own parser, src/recording/PerfState.cpp, whose two fields nothing restores into a layer (V24).
      (f) in src/MainComponent.cpp the calls `routineEngine_.tick(` and `routineEngine_.fire(` each carry the literal
          `RoutineSnap::Off`.
      The case PRINTS the surviving match count per clause. RED arm: main 7bc6df7, where every clause fails. Clauses
      (a), (b) without the two widget names, (c), (e), (f) are QO-1's exit; (d) and the two widget names are QO-2's.
      Risk K2's argument is then: no parameter, function or caller that could arm the slot exists (the compiler); no
      statement assigns it a live value (e); and T-Q2 / T-Q2c / QL1 never see a pending ref.
NC-9  THE LEGACY SEEK'S PROOF (GA-1). REPLACES PL2:472-473 (T-Q4), "MQ1" in PL2:487 and in QO-1's exit.
      - T-Q4 as a unit case is STRUCK (V10: its subject is not reachable without the app).
      - MQ1 (the seek left in) is RED on lint clause (d): "`beatSnap` in src/MainComponent.cpp".
      - QL1 gains clause (r), with its own RED line on main: a re-fire of the playing clip starts at its beginning
        ("42 default", BD:844).
NC-10 THE COLUMN FIRE IS PROVED TOO (GA-5). ADDS QL1 clause (c) and unit case T-Q2c. RED arm for both: main 7bc6df7.
      No further mutant.
NC-11 ROUTINES ARE AS THEY WERE: THE RED ARMS (GA-3). REPLACES MQ4 (PL2:487-488) and QL4's RED arm.
      - MQ4 (app): BOTH routine call lines pass Bar -> "FAIL  QL4: the off routine waited".
      - MQ4b (app, a build of its own): the TICK alone passes Bar -> "FAIL  QL4c: the beat routine waited for a bar".
      - A fire-only mutation changes nothing a row can see (V12); lint clause (f) is what catches it. Said, not hidden.
      - T-Q6 stays a guard, GREEN before and after, declared as such.
NC-12 L3b IS QL1 RE-RUN ON THE LANE, NOT A ROW OF ITS OWN (GA-6). REPLACES PL2:583-584 and the last clause of PL2:590.
      After S2 Harmony runs QL1 (a) and (c) on the lane's build with the nudge at +250 and again at -250; strings and
      bars are QL1's; the RED arm is QO-0's. RN's L3b and L3d are VOID.
NC-13 LR2 (d) SAYS WHAT IT CAN FAIL ON (GA-7). REPLACES the layer-2 clause of PL2:611-612: in rounds 3 and 5, for 2.5
      s after play no layer has a playing clip and no routine is running or waiting; RED arm MS7. The fixture's
      routines fire a clip at least every 2 beats, so a routine left running refills a layer inside the window; the
      probe's self-test shows that on MS7's app. The autopilot wording is struck; layer 2 needs no autopilot.
NC-14 THE CELLS AND THE FIT, ON THE BAR THE APP HAS (GA-8; own finding V14). REPLACES PL2:353-368 (the cells, the fit,
      the give-up order), PL2:407 (VG-0's exit), T-RB4, T-RW1, and "compact" in T13, T14 and B-7.
      - src/ui/TopBarModel.h (S3m): `inline constexpr int kTopBarHeight = 34;` (S3r makes M:MainComponent.cpp:2688 use
        it) and `inline constexpr int kRowCell = 30;`. Widths: every cell 30 except bpm 96, tap 48, resync 68; 1 px
        between cells; the nudge text 88 after a gap of 4; the gap after it 6; the state word 62, Manual 82, Link 56
        (today's widgets with their gaps: V14); "Bar N" 46; the Audio block 240.
      - A plain cell is `kRowCell` square and the three wide cells are `kRowCell` high, whatever height a test gives
        the bar; all are centred vertically in the inner area.
      - `fitTopBar(innerWidth, labelWidth)` gives up width in THIS order and nothing else: 1 DSP (55), 2 FPS (51), 3
        the state word (62), 4 Master Signal (94 + the label), 5 Master (136). Never given up: a cell, the nudge
        text, "Bar N", Manual, Link, Outputs, the Audio block. PL2's "compact cells" step does not exist.
      - COMPUTED with a label of 86: inner 1712 (a 1728 window) -> 1630 kept, nothing given up, 82 left; inner 1496
        (1512) -> DSP, FPS, the state word; 34 left; inner 1264 (1280) -> all five; 118 left. With a label of 169 at
        inner 1712: DSP is given up.
      - RR NA-6's remedies stand, re-based: (1) the nudge text box takes its widest text as measured + 8, with no 88
        floor; (2) the gap after the text 6 -> 2. Never: a cell narrower than its text.
      - VG-0'S EXIT IS A DECISION TABLE, read by Harmony before S3m:
        the bar is 1720 x 34 at his maximized window and the label is at most 168 wide -> as ruled;
        the label is wider than 168 -> remedies (1), (2); still short -> DSP is given up at his width and B-7 says so
        (RR HR-10);
        the bar is not 1720 x 34 -> STOP before S3m: the manifest goes to the architect, who re-states the two
        constants and the sums. The design (thirteen flush square cells as high as the bar's inner area) does not
        change; the numbers do.
      - T-RW1 sizes the bar as the APP does -- `kTopBarHeight` high and (window - 8) wide, for windows 1728, 1512 and
        1280 -- asserts height - 4 == `kRowCell`, and reads the text widths of the state word, Manual, Link and the
        Master Signal label from the BUILT widgets: each model constant must be at least that text plus its padding.
        So the fit is checked against the widgets, not fed back to itself.
      - LIT: the cell is filled with `AudioDNALookAndFeel::kAccentCyan`, its text dark; unlit is the bar's cell
        colour. Never `kRoutineCue`.
      - HB-6: the bar's height stays 34.
NC-15 THE NUDGE TEXT SITS AFTER RESYNC BY DEFAULT (ST-8). REPLACES PL2:350-352 and the default of PL2's question 145.
      `rowOrder()` = wheel, play, pause, stop, bpm, bpmMinus, bpmPlus, nudgeBack, nudgeForward, half, double, tap,
      resync, nudgeText. Question 145 A is this; B (the text between the two nudge cells) moves one name; no width
      changes. Reading R112.
NC-16 THE OVERLAY'S OLD "Stop" AND "Play / Pause" (ST-6). REPLACES the last sentence of PL2:411 and "HR-3 STANDS (...
      its words are the naming lane's)" in PL2:453.
      S4r sets two titles in `buildBindableTargets` (M:MainComponent.cpp:7584-7588): "Stop" -> "Stop routines" (RR
      HR-3's own string, in today's word: the naming lane re-words every "routine" string at once, this one with
      them) and "Play / Pause" -> "Audio play / pause" (it runs the audio file: V18). Display strings only: a saved
      binding stores the action's number. Case T-B11; VG states T16, T17 with a pre-registered MUST. HB-7.
NC-17 WHAT IS SAID TO BORIS (ST-2, ST-7; own finding V19). REPLACES PL2's B-3 and B-4 and the stop tooltip of PL:409;
      ADDS reading R111.
      - The stop cell's tooltip, exact: "Take every clip off the layers and stop the BPM timer on the 1. The tempo
        stays." (T-RB5 pins it; ASCII.) Play's and pause's (PL:407-408) stand; Resync's stays today's (M:TopBar.cpp:82).
      - B-3 and B-4: section 6. B-4 names the three things that change the nudge number: Resync, opening a show, New.
      - A routine pad pressed while the beat is stopped or paused waits, and starts on the first bar line after play.
        He is told (R111, B-3). Nothing is built: RR's R83 and HR-9 stay PARKED for the actions lane.
NC-18 THE QUESTIONS. PL2's draft 147 (the pad's Quantize menu) is NOT asked: its option B is engine work on routines,
      which this lane does not do; the pads' behaviour is told as R111. 147 is re-assigned to what stop does to a
      picture an effect is holding (NC-4). 144 and 146 stand as drafted; 145's default is swapped (NC-15). 148 and 149
      are not used. None of them has been shown to him.
NC-19 STAGES, MUTANTS, ORDER, THE STOP RULE.
      - quantize-out's order is QO-P -> QO-0 -> QO-1 -> QO-2 -> Harmony's rows -> VQ -> merge. PL2 ran QO-0 before the
        probe QO-0 needs had been written.
      - Mutants added: MS9, MS10, MR37, MQ4b; MQ1's RED arm moved (NC-9); MR27' re-defined (NC-6). Mutant apps:
        section 5.
      - RN A24 holds for every case of section 5 that states a guarantee of NC-1, NC-5 or NC-6: if it cannot be made
        green without a rule that is not written here, the builder STOPS and reports the call or hop sequence.
      - Everything of PL2 not named in NC-1..NC-18 STANDS: RB2 (pause), RB4 (the nudge across stop and play), RB5
        (the steps), RB6's W2, Q-a, Q-d..Q-h, its lists of tests and probes and its table of RN / RR rows, RB7, the
        disposition list at PL2:444-458 (with NA-14 now "REPLACED by NC-5"), the fences at PL2:425-438.

## 4 FINAL STAGES + ORDER (one builder context per stage; Harmony runs every live row, never a builder)
A builder builds, runs unit tests, writes probes and their self-tests; he never runs a live row and never gives a gate
verdict. Every live row, every mutant app's RED line and every verdict is Harmony's. Every live batch: the lane's
test-server build, the real app, `open -g`, the live lock, own-pid quit; no Output window, no full-screen capture, no
synthetic input; never while Boris's own Audio-DNA runs (RIG-RULES; the SCREEN-SAFETY LAW). REPLACES PL2 section 4's two
tables and its ORDER paragraph; RN's G-N0 and S1 rows are untouched.

LANE quantize-out (its own worktree and branch from main; merges to main by RIG-RULES' merge sequence)
| key | who | scope and the files it owns | needs | exit, and what it proves |
|---|---|---|---|---|
| QO-P | builder (short) | THE PROBE; no file under src changes. .harmony/probe-quantize-out.sh / .py / -selftest.py (QL1 a, c, r; QL2; QL3; QL4 a, b, c; it sources probe-quit-ours.sh, takes the live lock, quits only its own pid, deletes the takes it made) and the fixtures: the OLD show (the show's "quantizeMode": 2 and "bpmMultiplier": 2; clip A "beatSnapMode": 2 and clip B "beatSnap": true, "beatSnapMode": 3, both on one layer; a playable video V of at least 8 s with "beatSnap": true, "beatSnapMode": 1; three layers; columns 0 and 1 filled on each), an OLD take (checkpoint "quantizeMode": 2; six fires), three saved routines (own quantize "bar", "off", "beat"). Every route it calls exists on main (V7, V9). | this ruling adopted | The self-test prints "0 case(s) differ". |
| QO-0 | Harmony | The RED arms on main 7bc6df7's test-server build: QL1 a, QL1 c, QL1 r, QL2 and QL3 each print their FAIL line; `ctest -N` total noted. | QO-P | Row QO-0: the rows can fail. If QL1 a does NOT fail on main, STOP: the removal is not needed as planned (PL2 risk K1's refuting test). |
| QO-1 | builder | ENGINE + MODEL: PL2's Q-b (with NC-7: no load line), Q-d, Q-e, Q-f, Q-g, Q-h. src/model/Layer.h (`triggerClip`'s parameter and queue branch; `processPendingTrigger`), src/model/Composition.h, src/model/Autopilot.h/.cpp, src/render/Renderer.h/.cpp (the counter only), src/MainComponent.cpp (the helper, the two handlers' lines, the two routine lines, the preamble apply, the legacy seek), src/recording/Program.cpp, src/recording/PerfStateCapture.cpp, src/api/ApiServer.cpp + src/test/TestServer.cpp (the `render_pending_fired` keys), src/ui/TopBar.h/.cpp (ONLY what names the deleted enum, so the tree compiles: the combo's `onChange` body and the never-assigned `onQuantizeChanged`; the widget itself is QO-2's). The tests PL2 lists under RB6, deleted or rewritten -- except that tests/test_composition.cpp's round trip KEEPS the two clip fields and tests/test_shared_field_types.cpp:19 keeps them (NC-7). tests/test_quantize_out.cpp (new: T-Q1, T-Q2, T-Q2c, T-Q3, T-Q5 clauses a, b, c, e, f, T-Q6); tests/test_composition.cpp (T-Q-C1..C3). NOT edited: src/model/Clip.h, Clip.cpp, anything under src/recording/RoutineEngine. | QO-0 | The new cases RED on the tree before, then GREEN; MQ2, MQ3 RED; every deleted or rewritten case reported by name with its reason. Proves no fire can wait and an old show loads. |
| QO-2 | builder | SURFACES + PROBES + DOCS: PL2's Q-a and the UI half of Q-c. src/ui/TopBar.h/.cpp (the label, the combo, the two layout lines, the seam -> `rightOfTempoBoundsForTest()`), src/ui/ClipInspector.h/.cpp (the Snap combo and its slot; the row closes up), tests/test_master_signal_link.cpp (the four re-pins), T-Q5 clause (d) and the two widget names; the probe edits PL2 lists (probe-tsan scenario d, probe-boxes K5 struck, probe-step3.sh:732, probe-tsan-analyze.py); the build scripts of build-mut-q and build-mut-q2; docs/claude/performance-controls.md (:48, :73), architecture.md (:94 reads "saved, read by nothing"; :123-125), recording.md (:36, :51, :82-83 where they name the global Quantize), integration.md (:31 if it names `render_pending_fired`), CLAUDE.md (the capability "per-clip beat snap granularity" struck; the trigger-table row re-worded), .harmony/APP-INVENTORY.md (counts RE-COUNTED with `ctest -N`). | QO-1 | Unit green; MQ1 RED on lint clause (d). Then Harmony: unit count, QL1-QL5, probe-boxes and probe-routine-display, the two mutant apps, then VQ. |
| VQ | capture builder, five critic seats; Harmony's verdict | As PL2: Q1 the top bar at windows 1728, 1512, 1280 (Manual off and on); Q2 the Clip inspector's Autopilot section for a video clip and an image clip -- one of them a clip whose file carried Snap = Bar; each against the same view of main. | QL rows green | Row VQ. Boris sees nothing before it is green. |

LANE nudge (one worktree and branch, as RN; no two builders in it at once)
| key | who | scope and the files it owns | needs | exit, and what it proves |
|---|---|---|---|---|
| S1 | builder | AS RULED (being built; not re-opened). | -- | as RN |
| S1r | builder | RR's S1r row STANDS with these changes only. src/model/BeatTimer.h: `RunCommand` with four values and its pack / unpack; NO `kGesturesStartTimer`, `kTapStartsTimer`, `kResyncStartsTimer`, `kNudgeAfterStop`. src/analysis/BPMTracker.h/.cpp: `requestResyncByHand`, the queue handler of NC-5, a hop-end Resync dropped while not Running (this replaces RR's "a Resync while held = `applyStop`"). src/analysis/BeatShift.h/.cpp: NC-6's sentence in step 2 and the final-state precedence. Tests: T-G5 (first arm only), T-G6, T-G6b, T-G6c, T-G6d, T-G7 with its two new arms, T-N17, T-N17b (section 5). Step 0 only if S1 used the real `Layer::processPendingTrigger` in T-N5b: swap it for the two-line rule (NOTE below). | S1 green; Harmony's order against the transport lane's S4t (HR-1) | As RR, with MR27 replaced by MR27', and MR35, MR36, MR37 RED. Proves the timer, the two gestures and the publish step on a REAL tracker. |
| S2 | builder | RN's S2 row STANDS, with: step 0 = main (quantize-out merged) taken in; the probe's L3 rows as section 5. The "resync" branch still calls `requestResync()` at S2 -- a held beat cannot be reached before S2r. | S1r, G-N0, quantize-out MERGED | As RN; then Harmony: G-N1, L1, L2, L3a, L3c, QL1 a and c at +250 and -250 (NC-12), L4, L6, L9, L10. |
| S2r | builder | RR's S2r row STANDS, with: src/model/Composition.h (`takeAllClipsOff`); src/MainComponent.h/.cpp (`clearAllLayers`; `setBeatTimer`'s Stop branch calls it first; NO nudge statement in the Stop path; the "resync" branch's hand call of NC-5); tests/test_stop_clears.cpp (new: T-S1, T-S3, T-S3b, T-S5); lint T-G14 as amended; the probe's rows LR1, LR2 (a)(b)(d)(f), LR3, LR4a, LR7 as section 5, with the fixture show; the build scripts of build-mut-gate, build-mut-edge, build-mut-fold, build-mut-stop, build-mut-stop2. | S1r, S2's rows green | Unit green; MR18, MR20-MR22, MR25, MR31, MR33, MS1, MS4-MS7, MS9, MS10 RED; self-test "0 case(s) differ". Then Harmony: G-N1, `.harmony/probe-tsan-unit.sh`, LR1, LR2 (a)(b)(d)(f) with FM-S2, LR3, LR4a, LR7, L10's lines, probe-video.sh and probe-seq-vram.sh still GREEN (FM-R2). Proves: stop empties the layers and the picture, is the "1", and writes no clip; pause holds only BPM Sync clips and writes nothing; a hand Resync starts; a Tap and a replayed Resync do not. |
| S3a | builder (short) | RN's row STANDS; the ui_text "topbar" dump lists no Quantize widget. | S2r's rows green | as RN |
| VG-0 | capture builder | BASELINE on the lane after S3a: V18 the bar at windows 1728, 1512 and 1280, Manual off and on (no Quantize; still the three old buttons and the five); V19 the learn overlay; V20 his picture resolume-tempo-bar.png carried into the set as the reference. The manifest: every top-bar widget's bounds, the BAR's own bounds, the Master Signal label's measured width. | S3a | NC-14's decision table. |
| S3m | builder (short) | RR's row STANDS, with: `kTopBarHeight`, `kRowCell`, NC-14's widths and its five-step `fitTopBar`; `rowOrder()` (NC-15); `rowTexts()` (PL2's eleven); the stop tooltip (NC-17); T-RB3, T-RB3b, T-RB4, T-RB5 (section 5). | VG-0 | as RR; MR32' RED. |
| S3r | builder | RR's row STANDS with PL2's changes (the cluster laid in `resized()` is "from the Audio block's end to Link"; the three OLD buttons, `onPlay`, `onPause`, `onStop` and their lambdas at M:MainComponent.cpp:612-628 REMOVED; no "R[]"; `rightOfTempoBoundsForTest` re-pointed to Link; test_topbar_link_toggle.cpp's routines-stop case deleted and its Link bounds re-pinned) and: M:MainComponent.cpp:2688 uses `kTopBarHeight`; the cells are NC-14's; the lit fill; T-RW1, T-RW5, T-RW7 (section 5). | S3m | Unit green; MR19, MR34, MS8 RED. Then Harmony: G-N1, LR4b, LR5, LR2 (e), and LR1 / LR2 / LR3 / LR4a again via "button"; build-mut-row. |
| S4 | builder | RN's row as RR amended it. STANDS. | S3r | as RN; then Harmony: L7, L8, LR5 (a). |
| S4r | builder (short) | RR's row STANDS (the seven targets -> `tempoRowOp(op, Origin::Human)`; "Beat stop" therefore empties the layers), with NC-16's two titles and T-B11. | S4's rows green; HR-2 | as RR; then Harmony: LR6. |
| VG | capture builder, five critic seats; Harmony's verdict | States T1-T19 and T7b (section 5) against V18-V20. | S4r's row green | Row VG. Boris sees nothing before it is green. |
| S5 | builder | RR's row STANDS, with the pitfall sentence: "the timer is gated at five sites; a hand Resync while held is a START from the 1 and travels in the run queue; a replayed Resync never moves a held beat; a Tap never starts; stop takes every clip off through `Composition::takeAllClipsOff` (the tuple only, no clip field) and is never an Undo step; the row's pause writes no clip; Open and New never touch it; the recorder clock treats a held timer as unmetered". The manual says stop / pause / play / Resync / Tap as B-2..B-5 word them; NO sentence on Quantize, on a waiting fire or on a routine pad's start; recording.md is not touched by this lane. | VG green | Harmony reads the manual against the built app. |
ORDER: QO-P -> QO-0 -> QO-1 -> QO-2 -> Harmony's rows -> VQ -> merge quantize-out. In parallel, in the nudge worktree: S1 ->
S1r (neither touches a file of quantize-out). Then S2 (step 0 takes main in) -> Harmony's rows -> S2r -> Harmony's rows ->
S3a -> VG-0 -> S3m -> S3r -> Harmony's rows -> S4 -> S4r -> Harmony's rows -> VG -> S5 -> merge.
NO STOP FOR S1. Evidence: S1 owns BeatShift, BeatNudge.h, the tracker's two views with `adoptCounters`, and their tests
(RN:476); none of it holds a stop, a pause, the run queue or a start. NC-6's sentence is in step 2 but is conditioned on
`startApplied`, a Flag S1r adds: it is S1r's edit. S1's step 2 stays right for a Resync while Running.
NOTE FOR HARMONY ON S1 (PL2's, kept): RN:524-529 lets S1 call the real `Layer::processPendingTrigger` in T-N5b;
quantize-out deletes that function. If S1's report says "the real function", S1r's step 0 swaps it for the two-line rule
(bars unchanged) BEFORE the lane takes main in at S2. If S1's packet can still be told: "the two-line rule".
BPMTracker.cpp ACROSS LANES: nudge S1, then S1r, then the transport lane's S4t -- HR-1 STANDS.
FENCES. PL2:425-438 stands, with three changes. (1) one-save: quantize-out takes ONE key ("quantizeMode") out of
Composition.h's writer and reader and none out of Clip.cpp (NC-7). (2) transport lane: its own delta for its rows of FQ's
TABLE Q5 is still owed before its S1 (PL2:319-322); the strip's X and its clear tail are that lane's (SF-C1). (3) S2 adds
"beatNudgeMs" beside the line quantize-out removes in Composition.h: S2's step 0 (main taken in) is where that is settled.
WHAT HARMONY RUNS HERSELF, AND WHEN: QO-0 after QO-P and before QO-1. After QO-2: unit count, QL1-QL5 (incl.
probe-tsan-unit.sh), build-mut-q and build-mut-q2, VQ. G-N1 after every nudge builder stage. After S2: L1, L2, L3a, L3c,
QL1 a / c at the two nudges, L4, L6, L9, L10. After S2r: probe-tsan-unit.sh, LR1, LR2 (a)(b)(d)(f), FM-S2, LR3, LR4a, LR7
via "handler", the five mutant apps, the two media probes. After S3a: VG-0 and its table. After S3r: LR4b, LR5, LR2 (e),
LR1-LR3 and LR4a via "button", build-mut-row. After S4: L7, L8. After S4r: LR6. Then VG, then S5's manual read.
NOT IN THIS LANE: PL2 section 9 stands, and with it: the strip's X and its tail; resetting a picture an effect holds at a
stop (question 147); the Undo rule for the layer strip (NC-3); deleting the clip's two dormant fields (HB-5); a taller top
bar (HB-6); the naming lane's re-wording of "routine".
PARKED FOR THE ACTIONS LANE, by name (no routine or take clause is built here): a "stop all routines" control of any
kind (RR NA-5's third bullet, HR-11) and `stopAll()` under the row's stop (HB-1's alternative); the routine sentence of
R75; R83 and HR-9 with an Off lead-in for a pad pressed while the beat is held (NC-17); RR LR2's old clause (c) and the
routine half of its (d); whether stop ends a REPLAYING take (R104); recording a stop's clears or a layer's X in a take
(PL2 SF-B1); a routine's own quantize, its pad menu and the engine's `forcedSnap` parameter (R111); the review screen
and its Quantize (BD:971-978: a design is owed to him first).

## 5 TESTS + GATE ROWS (pre-registered; a bar is met or reported, never loosened; each has its RED arm)
PL2 section 5 STANDS except the rows re-stated here. Harmony copies gate strings from here, then PL2 section 5, then RR,
then RN, in that order of precedence.

UNIT, lane quantize-out (RED arm for each: the tree before QO-1, main 7bc6df7)
 T-Q1, T-Q2, T-Q3, T-Q6, T-Q-C1, T-Q-C3: as PL2:465-486.
 T-Q2c `Composition::triggerColumn` on a deck whose clips' JSON carried "beatSnap": true and "beatSnapMode": 2: after
       the call every layer that does not ignore column fires has that column's clip as its active ref; no layer's
       pending ref is valid. Twenty calls alternating two columns with beat edges fed to `Autopilot::processFrame`
       between them.
 T-Q4  STRUCK (NC-9).
 T-Q5  NC-8, six clauses, the surviving count printed per clause.
 T-Q-C2 `toVar` of any composition has no key "quantizeMode"; a clip loaded with "beatSnap": true and "beatSnapMode":
       3 writes the same two values back. MQ3 RED.
 MUTANTS: MQ1 the legacy seek left in (RED on T-Q5 d). MQ2 `Composition::fire` keeps a snap parameter that one handler
 passes (RED on T-Q5 a and on T-Q2). MQ3 `toVar` still writes "quantizeMode". MQ4, MQ4b: apps (NC-11).
UNIT, tests/test_beat_timer.cpp (S1r), amended and new
 T-G5  PL:543-545's first arm only (a Tap while held changes the tempo and nothing else).
 T-G6  A HAND RESYNC WHILE HELD IS A START (NC-5). Paused at beatInBar 2, phase 0.7; `requestResyncByHand()`: on the
       applying hop the state is Running, totalBeatCount + 1, beatInBar 0, beatPhase 0.0, the level true,
       totalBarCount unchanged, `appliedResyncs()` + 1, `appliedStarts()` + 1, `appliedStops()` unchanged; the next
       100 hops advance the phase at the tempo. The same from Stopped. In Auto with a confident beat fed on that hop
       (RR T-G3b's arm 1): still beatInBar 0. MR35 RED (the state stays held).
 T-G6b A TAP NEVER STARTS: as PL2:495-496. MR36 RED.
 T-G6c A REPLAYED RESYNC NEVER MOVES A HELD BEAT (NC-5). Stopped; then Paused at beatInBar 2, phase 0.7:
       `requestResync()`: over 200 hops the nine accessors, the state and the three applied counters do not change;
       the request sequence is latched. MR37 RED (it starts, or seats the "1").
 T-G6d A HAND RESYNC WHILE RUNNING IS MAIN'S RESYNC. Two trackers fed the same 2000 hops (Manual 120, then Auto with
       RN T-N5's LATE sequence); between the same two hops one gets `requestResync()`, the other
       `requestResyncByHand()`, at phases 0.2 and 0.7: the nine accessors are equal on every hop. RED arm: the queue
       handler runs `startFromOne()` whatever the state.
 T-G7  PL:546-549, plus: between two hops Stop then ResyncByHand -> the next hop ends Running, the count + 1 from the
       count at the stop, beatInBar 0; Running, ResyncByHand then Stop -> the next hop ends Stopped on the "1" block,
       `appliedStarts()` unchanged; pack / unpack round trip over the four commands for every length 0..14.
UNIT, tests/test_beat_shift.cpp (S1r), amended
 T-N17 as PL2:498-501 (the stop hop keeps the nudge; ONE arm with a zero request: a hand Resync and then a Stop in one
       hop -- applied 0.0, the state Stopped).
 T-N17b A HAND RESYNC THAT STARTS A HELD BEAT, THE NUDGE ENGAGED (NC-6; replaces PL2:502-506). Manual 120; plus means
       earlier. Arm LATER: D = -250 applied, Paused at tracker phase 0.2; PRECONDITION asserted: the published count
       is one BEHIND the tracker's. Arm EARLIER: D = +250 applied, Paused at tracker phase 0.7; PRECONDITION: the
       published count is one AHEAD. Arm STOPPED: D = -250, then D = +250, Stopped. In each, a zero request and
       `requestResyncByHand()` on one hop. On that hop: the "1" block; totalBeatCount' == F's + 1 EXACTLY; applied
       0.0; not held; the state bits Running; adoption returned exactly when the tracker's count differs (asserted:
       LATER and EARLIER adopt, STOPPED does not). Over the next 400 hops count' never falls and rises by 0 or 1 per
       hop, and from the second hop the nine published fields are the tracker's own. MR27' RED (the clamp kept: arm
       EARLIER shows no edge).
 T-N19 STANDS and is the live path. T-N18, T-N20, T-N7b STAND.
UNIT, tests/test_stop_clears.cpp (S2r; a Composition with 4 layers and 2 decks; no MainComponent). RED arm for the
 file: a `takeAllClipsOff` that does nothing.
 T-S1  after it every layer's active ref is invalid, its pending ref invalid, crossfadeProgress 1.0, and its previous
       ref is the ref that was active -- with one layer mid-crossfade (0.4) and one whose clip is from the deck that
       is not shown. The return value is the number of layers that had a clip. MS1 RED.
 T-S2  STRUCK as a unit case (NC-3); "no Command" is lint T-G14's.
 T-S3  NO CLIP FIELD CHANGES (NC-1; replaces PL2:516-517). For every clip of every deck, `playing`,
       `hasBeenTriggered`, `playheadPosition` and `beatsPlayed` equal their values before -- with one active clip
       playing, one active clip paused by hand, one inactive clip playing. MS10 RED (the layer's `clearActiveClip`
       used: the playing clip reads false).
 T-S3b FIRED AGAIN (NC-1). A clip with `hasBeenTriggered` true (set as the handler sets it, V3) and `playing` true is
       active; `takeAllClipsOff()`; `Composition::fire` of the same clip: it is the active ref and `playing` is true.
       The same with `playing` false before: still false after the re-fire. MS10 RED.
 T-S4  STRUCK as a unit case (NC-2); its promises are T-G14's, LR2's and LR7's.
 T-S5  a second call returns 0 and changes no tuple.
LINT T-G14 (RR), amended: `setBeatTimer(` is called on one line of src, inside `tempoRowOp`; `clearAllLayers(` is called
 on one line of src, inside `setBeatTimer`; `takeAllClipsOff(` is called on one line of src, inside `clearAllLayers`;
 the body of `clearAllLayers` holds `stopOnLayer(`, then `takeAllClipsOff(`, then `refreshPreviewFromShow(`, in that
 order, and no `capture(`, no `pushCommands(`, no `Cmd>(`, no `clearActiveClip(`; the body of `setBeatTimer` holds no
 `capture(` and no `beatNudgeMs`; `requestRun(` and `requestResyncByHand(` each occur once outside BPMTracker's files;
 `applyClipPlaying(` is not called from any TopBar callback. MR25, MR33, MS4, MS5, MS6, MS7, MS9, MS10 each RED.
MODEL / WIDGET (S3m, S3r, S4r), amended
 T-RB3  `rowOrder()` == NC-15's fourteen names.   T-RB3b as PL2 (eleven texts, pairwise distinct, ASCII). MR32' RED.
 T-RB4 `fitTopBar` with a label of 86: inner 1712 -> nothing given up, 82 left; 1496 -> DSP, FPS, the state word, 34
       left; 1264 -> all five, 118 left. With a label of 169 at 1712: DSP is given up. For every inner width from
       1264 to 2200: the kept widths sum to <= the width; what is given up is a prefix of NC-14's five; a cell, the
       nudge text, "Bar N", Manual, Link and Outputs are never in it.
 T-RB5 the tooltip strings of PL:406-413 with RR NA-5's three and NC-17's stop string, exactly; every character ASCII.
 T-RW1 NC-14's: the bar sized (1720, `kTopBarHeight`): nothing is given up; the thirteen cells are flush (1 px apart),
       each 30 high, ten of them 30 wide; height - 4 == `kRowCell`; no text wider than its cell; the four model
       constants are each at least the built widget's text + its padding; "margin <n> px" printed. Sized (1504, 34)
       and (1272, 34): what is given up equals `fitTopBar`'s for the MEASURED label. MR34 stands.
 T-RW5, T-RW7: as PL2:534-539.
 T-B11 (S4r; NC-16) the bind overlay's and the MIDI-learn overlay's target titles are pairwise distinct and none is
       exactly "Stop", "Play", "Pause" or "Play / Pause". RED arm: the lane at the S4 commit.
MUTANTS, new or re-defined: MS9 (app) `clearAllLayers` without `refreshPreviewFromShow()`. MS10 (app, and the unit
 arm of T-S3) the layer's `clearActiveClip` used in place of the tuple-only clear. MR37 (app, and unit) a Resync that
 reaches a hop's end starts a held timer. MR27' the clamp kept on a start hop. MQ4b (NC-11). MS1 is "`clearAllLayers`
 skips `takeAllClipsOff()`"; MS7 is "the per-layer `stopOnLayer` call removed". PL2's MS4, MS5, MS6, MS8, MR35, MR36
 stand.
MUTANT APPS (each a normal build in its own directory under RN A10's rules; inside one app the mutants fail DISJOINT
 clauses): build-mut-q (MQ4); build-mut-q2 (MQ4b); build-mut-stop (MS1 + MS4 + MS6): LR2a layers, LR2a "ms", LR3g;
 build-mut-stop2 (MS7 + MS9 + MS10 + MR37): LR2a routines, LR2a picture, LR2b, LR7 BAR 4. RR's four stand (MR27
 leaves their lists: it was unit-only). Harmony may merge two apps only if their failing clauses stay disjoint (HR-12).

GATE ROWS, lane quantize-out (Harmony; strings exact; the rig above; Manual 120 on the 120 BPM click file)
QL1  A FIRE LANDS ON THE PRESS AND STARTS AT THE BEGINNING. (replaces PL2's QL1) The OLD-show fixture loaded.
     (a) CLIPS. 30 fires by `POST /api/trigger_clip` at pseudo-random times (fixed seed), alternating A and B;
         `GET /api/composition` polled every 5 ms. "At once" = the first poll in which the layer's playing clip is the
         fired one comes <= 150 ms after the POST returned. BAR: 30 of 30 at once; "pendingClip" is null in every
         poll; at least 20 of 30 land in a poll whose beatInBar != 0 or beatPhase >= 0.15 (they are not on a line).
     (c) COLUMNS. 10 fires by `POST /api/trigger_column` at pseudo-random times, alternating columns 0 and 1. BAR: 10
         of 10 -- every layer shows that column's clip within 150 ms; "pendingClip" null in every poll.
     (r) THE START. V is fired; once it is its layer's clip and its "playheadPosition" has moved, V is fired AGAIN on
         the first features poll whose beatPhase is in 0.45..0.75; the smallest "playheadPosition" read in the polls
         20..150 ms after that POST returned must be <= 0.05 (its in-point is 0.0). 5 rounds. BAR: 5 of 5.
     "PASS  QL1 fires land on the press: 30 of 30 clips, 10 of 10 columns, 0 queued; a re-fire starts at the
     beginning: 5 of 5". RED arms (QO-0, main): "FAIL  QL1a: <k> of 30 within 150 ms" with k <= 10 (a bar at 120 is
     2.0 s); "FAIL  QL1c: <k> of 10 within 150 ms" with k <= 3; "FAIL  QL1r: playhead <p>, want <= 0.05" in at least 4
     of 5 rounds.
QL2  AN OLD SHOW LOADS; ITS QUANTIZE IS GONE FROM THE SAVE; ITS CLIPS' SNAP VALUES ARE KEPT. (replaces PL2's QL2)
     `POST /api/load_composition` of the OLD-show fixture: ok; the deck, clip and layer counts equal the fixture's;
     `POST /api/debug/save_composition`: the file has no "quantizeMode"; for every clip "beatSnap" and "beatSnapMode"
     equal the fixture's values; it still has "bpmMultiplier" (this lane's S3r drops that one). "PASS  QL2 an old
     show loads; no quantizeMode in the save; clip snap values kept". RED arm (main): "FAIL  QL2: quantizeMode in the
     saved file".
QL3  STANDS (PL2:561-565).
QL4  ROUTINES ARE AS THEY WERE. (replaces PL2's QL4) (a) the routine with quantize "bar", fired by `POST
     /api/routine/fire`, starts on a totalBarCount edge and not before it. (b) the one with "off" is running within
     150 ms. (c) the one with "beat", fired on a poll whose beatInBar is 0 or 1 and whose beatPhase is in 0.2..0.6, is
     running within 600 ms (one beat at 120 is 0.5 s; the next bar line is at least 1.0 s away). `.harmony/probe-
     routine-display.sh` prints its PASS lines unchanged. "PASS  QL4 routines keep their own quantize: bar, off,
     beat". RED arms: build-mut-q (MQ4): "FAIL  QL4: the off routine waited"; build-mut-q2 (MQ4b): "FAIL  QL4c: the
     beat routine waited for a bar".
QL5  STANDS (a required regression gate of Pitfall 63, declared as such). VQ STANDS (PL2:573-574).

GATE ROWS, lane nudge, re-stated (everything not re-stated here or in PL2 STANDS)
L3   L3a and L3c: as PL2:579-582 and :585-589 (the readers are an autopilot beat step and an existing routine's bar
     start; no routine code is written; L3c is VOID from the day the actions lane removes the pads). L3b: NC-12 -- not
     a row; QL1 a and c re-run on the lane with the nudge at +250, then at -250. RN's L3d: VOID. RED arm for L3a and
     L3c: build-mut-bypass, on the tracker's-line clause.
LR1  STANDS with PL2's added clause (across the row no clip's "playing" changes in `GET /api/composition`).
LR2  STOP EMPTIES THE LAYERS AND THE PICTURE AND IS THE 1; PLAY IS THE EDGE; A CLIP FIRED AGAIN PLAYS; RESYNC STARTS, A
     TAP DOES NOT; OPEN LEAVES IT STOPPED. (replaces PL2's LR2)
     Manual 120. The fixture show: layer 0 a BPM Sync image sequence S; layer 1 a Timeline video V (transition 2.0 s)
     and a second clip V2; layer 2 two images I and I2; layer 3 a bright procedural source P; two saved routines R1,
     R2 (own quantize Bar, looping, one bar long, each firing I and I2 on layer 2 two beats apart); a second show
     file whose "beatNudgeMs" is 37.
     E0 = `POST /api/render_frame` right after the load, before any fire: the frame of the empty show, whatever it is.
     Before EVERY round S, V, I and P are fired by `POST /api/trigger_clip` (so each has been fired through the
     handler: V3). PRECONDITIONS: a `render_frame` then differs from E0 by a mean absolute difference >= 10.0 (of
     255) per channel, else "INVALID: the fixture shows nothing"; S's and V's "playheadPosition" each differ between
     two reads 300 ms apart.
     Rounds 1-3 before any nudge is set; rounds 4-5 at nudge +40. In rounds 2 and 4 V2 is fired 100 ms before the
     stop (a fade in flight). In rounds 3 and 5 R1 is running and R2 has been fired and is waiting.
     Arm S posts `tempo_row` stop on the first poll whose beatPhase is in 0.65..0.85, arm F in 0.10..0.30; c = that
     poll's totalBeatCount.
     (a) STOP. From the applied poll, 400 polls: beatPhase == 0.0, beatInBar == 0, barCount == 0, downbeatDetected
         true, totalBeatCount == c EXACTLY, the nine fields constant, "beatTimer": "stopped"; `/api/debug/beat_nudge`
         "ms" == its value before the stop (0, or 40 in rounds 4-5); within 150 ms of the POST and in every later poll
         of the window NO layer of `GET /api/composition` has a playing clip and "pendingClip" is null on all; `GET
         /api/routine/status` shows no slot running or waiting; a `render_frame` taken 300 ms after the applied poll
         differs from E0 by a mean absolute difference <= 2.0 (of 255) per channel.
     (b) FIRED AGAIN WHILE STOPPED. `trigger_clip` of V: it is layer 1's clip within 150 ms, its "playing" is true,
         and its "playheadPosition" differs between two reads 300 ms apart. Of S: it is layer 0's clip within 150 ms,
         "playing" true, its position equal in every read for 1.0 s.
     (d) PLAY. `tempo_row` play: in the applied poll totalBeatCount == c + 1, beatInBar == 0, downbeatDetected true,
         totalBarCount unchanged; S's position has changed within 1.0 s and its first changed read is within 0.05 of
         the held one; in rounds 3 and 5, for 2.5 s after play layer 2 has no playing clip and no routine is running
         or waiting; "clockBeat" at the first poll after play is within 0.25 of its held value (RR NA-15).
     (f) RESYNC AND TAP, once after the rounds. Stop; `binding_action` "tapTempo" twice 500 ms apart: bpm within 120
         +/- 2, "beatTimer" still "stopped", the nine fields constant over 200 polls. Then Resync (S2r: `POST
         /api/resync`; from S3r: the row's own RESYNC through `pressRowControlForTest`): in the applied poll
         "beatTimer": "running", totalBeatCount == the held count + 1, beatInBar == 0, "ms" == 0, beatNudgeAppliedMs
         0.0. Then S is fired, the beat paused mid-bar, Resync: "running", the count + 1, beatInBar 0, and S's first
         changed read is within 0.05 of its held one (it runs on; this lane has no lock).
     (e) OPEN, once, from S3r on: as RR's (e), unchanged.
     Five rounds: S, F, S, F, S. INFO arms: RR's FM-R3 arm, unchanged; FM-S2 (NC-4).
     "PASS  LR2 stop empties the layers and the picture and is the 1; play is the edge: 5 of 5 rounds; a clip fired
     again plays; resync starts, tap does not; open leaves it stopped". RED arms: build-mut-stop: MS1 -> "FAIL  LR2a:
     layer <n> still plays"; MS4 -> "FAIL  LR2a: ms 0, want 40". build-mut-stop2: MS7 -> "FAIL  LR2a: routine slot <n>
     not stopped"; MS9 -> "FAIL  LR2a: picture not empty (mean diff <d>)"; MS10 -> "FAIL  LR2b: the re-fired clip is
     not playing". build-mut-edge: MR2 -> "FAIL  LR2a: count c+1, want c" (rounds 1 and 3), MR3 -> "FAIL  LR2d: no
     edge at play". build-mut-fold (MR18) -> "FAIL  LR2d: routine clock jumped". build-mut-row (MR33) -> "FAIL  LR2e:
     beatTimer running after open". The lane at the S2 commit -> "FAIL  LR2f: 404".
LR3  STANDS as PL2:626-642 re-stated it (pause holds only BPM Sync clips; fired while paused; his own pause
     survives). Its RED arms stand.
LR5  STANDS as PL2:643-645, with (e) reading "the fourteen names of NC-15".
LR6  STANDS; its step 1 additionally reads "every layer empty" (PL2).
LR7  STANDS (RR's three bars) with PL2's setup change (two layers are playing when take A is recorded) and a fourth
     bar. BAR 4: take C = record; `POST /api/resync`; stop recording. C replayed with the timer stopped, then with it
     paused mid-bar: "beatTimer" keeps its value in every poll during the replay and for 1.0 s after it, and the nine
     beat fields do not move. "PASS  LR7 the timer is not in a take: points equal, replay leaves it running, a
     stopped timer stays stopped under a replay, a replayed Resync does not start it". RED arms: build-mut-edge
     (MR25): "FAIL  LR7: take A has <n> more point(s) than take B"; build-mut-stop2 (MR37): "FAIL  LR7: beatTimer
     running during the replay".
VG   Five critic seats on T1-T19 and T7b against V18-V20: 0 MUST. States: PL2:652-661's, with T13 = "window 1280,
     Manual on, stopped (both Master faders, the state word, FPS and DSP given up)", T14 = "window 1512 (DSP, FPS and
     the state word given up)", and T7b = T7 with a global Echo on, captured 1.0 s after the stop.
     Pre-registered MUSTs: PL2:662-665's, plus: a cell that is not 30 high, or one of the ten plain cells that is not
     30 wide; the row's order differs from `rowOrder()`; an overlay target titled exactly "Stop" or "Play / Pause";
     the stop cell's tooltip names only the timer.
     Named questions the seats answer in words: PL2:666-669's, with "does the nudge text after RESYNC read as part of
     the row (question 145)" in place of its last one, plus: do 30 px cells read as the picture V20 at arm's length
     (HB-6); in T7b, does what an effect still shows after a stop read as the effect's, not as a stuck clip.
After every live batch: RN's closing line stands.

## 6 WHAT ONLY BORIS CAN CHECK (do -> expect -> what wrong looks like; on his real screens and by his ear)
RR's B-R1, B-R4, B-R7, B-R8, B-R11, B-R13 STAND (B-R8's "Beat stop" pad now also empties the layers). RN's B5, B10
STAND. PL2's B-1, B-2, B-8 STAND as PL2:675-682 and :701-705 word them. REPLACED here: PL2's B-3, B-4, B-5, B-6, B-7,
B-9. NEW: B-10. VOID: RN's B13; RR's B-R5.
B-3  Press stop ([]) -> every layer strip is empty at once (a cut, like a layer's X); stop lights; the circle sits on
     beat 1; the tempo number still shows the tempo; routines that were playing on the layers have stopped. The picture
     is empty too -- unless an effect that holds or trails a picture is on (Freeze keeps its picture; Echo or feedback
     leave a trail): that effect keeps doing what it does (question 147). Fire a clip you were playing before -> it
     plays. Fire one you had paused yourself -> it comes back paused. Fire a BPM Sync clip -> it appears on its first
     frame and waits. Wait for a "1" in the music and press play ON it -> beat 1 is your press and the waiting clip
     starts with it; layers you did not fire stay empty. A routine pad you press WHILE stopped waits, and starts one
     bar after you press play (known: pads are being replaced by actions). WRONG: a clip stays on a strip; a clip you
     fire again sits paused though you never paused it; clips appear after play that no pad of yours asked for; the
     number shows 0.
     (Cmd+Z is added to this check only when the build has the Undo change for the layer strip: HB-8.)
B-4  Set a nudge ("nudge +12 ms"). Stop, then play -> it still reads +12. Pause, then play -> still +12. Three things
     change the number: Resync puts it back to 0; opening a show sets it to that show's own number; New sets it to 0.
     With a nudge set, "play is the 1" is moved by that nudge: that is what the number means. WRONG: stop, pause or
     play changes the number.
B-5  Stopped: tap a new tempo -> the number changes, nothing starts, stop stays lit. Press Resync on a "1" you hear ->
     the beat starts from 1 on that press, play lights, the nudge reads 0. Do the same from pause -> the same, and the
     BPM Sync clips that were standing run on from their frame. WRONG: tapping starts the beat; Resync leaves it
     stopped; Resync starts somewhere else in the bar; a standing clip jumps.
B-6  At arm's length on your screen, beside your picture of Resolume's bar: is the row in your order; are the cells
     the shape you meant, and big enough (they are as tall as today's top bar, a little smaller than Resolume's); can
     you see at a glance which of play / pause / stop is on; can you tell play ">" from nudge forward ">>"; is "nudge
     +12 ms" readable right after RESYNC (question 145); is "Bar 2" left of the circle wanted (question 146)?
B-7  Make the window narrow -> the row never loses a cell; FPS / DSP go first, then the LOCKED word, then Master
     Signal, then Master. Is that the right order to give up?
B-9  (a confirmation; a machine row proves it) Record a take while you pause, play and stop the beat, and press
     Resync; replay it -> the replay does not pause, stop or start your beat. A take you replay after pressing stop
     keeps replaying and puts its clips back (reading R104): tell us if stop should end a replay too.
B-10 Open the key / pad list. The row's stop is "Beat stop" (it empties the layers). The old "Stop" now reads "Stop
     routines"; the old "Play / Pause" reads "Audio play / pause". WRONG: two entries you could mix up on stage.

## 7 BORIS QUESTIONS (144..147; each has a default A; nothing waits; none has been shown to him. 148, 149 are not used)
His words this delta rests on, verbatim (BD:951-959, :971-972, :1007-1012, :1021-1022): "125 whole numbers"; "126 and 127
stop clears all clips from layer strips, pause stops them, tempo setting stays the same. Make thee new buttons as I
asked"; "128 tapping tempo does not start anything but when click resync, that is the 1 and it begins on that button
push"; "129 b"; "135 the beat stops but tempo is not lost, just not playing"; "136 b"; "137 the plus moves the main bpm 1
bpm number regardless of bpm or timeline mode. the plus moves 1 beat in the clip that is in bpm mode"; "This is not for
live usage. We should remove it from the top bar and use it in the recording review screen which we have yet to create.";
"r85 go with the defined stop play pause that we discussed in 135 and place them at the top. use this similar layout
across the top row where it fits: [Image #8]".
144. Stop empties every layer at once. One wrong press in a show empties the screen.
     A (default) One press, at once.
     B Stop only acts when you hold the button (or the pad) for half a second; a short press does nothing.
145. The text "nudge +12 ms". Your picture has the two nudge buttons side by side, with no text.
     A (default) The two nudge buttons sit side by side as in your picture; the text comes right after RESYNC.
     B The text sits between the two nudge buttons: "<<"  nudge +12 ms  ">>".
146. Today the words "Bar 1" .. "Bar 4" sit just left of the beat circle. Your picture has no such text.
     A (default) Keep it.
     B Take it out; the circle alone shows the beat.
147. Stop takes every clip off. Some effects hold or trail a picture by themselves: Freeze holds one; Echo and feedback
     leave a trail.
     A (default) They keep doing what they do: a trail fades out; a frozen picture stays until you switch that effect
       off.
     B Stop also wipes what those effects are holding, so the screen is black at once.
The line that changes for each B: 144 -> one branch in the stop cell and in the "Beat stop" binding case (a press arms a
500 ms timer; the release before it cancels) + T-RW5's stop clause and LR5 (g)'s stop op. 145 -> `rowOrder()` (one name
moves) + T-RB3. 146 -> one name leaves the fixed list (- 46 px) + T-RB4's sums. 147 -> NOT one line: an architect line
(which histories, cleared by the render thread at a frame's start).
READINGS told with them (he corrects only what is wrong; these REPLACE PL2's R103-R110; next free reading R115, next
free question 148):
 R103 Stop also stops the routines that are playing or waiting on the layers, as a layer's X does.
 R104 A recording you replay is not stopped, paused or started by the row: it follows its own sound and can put clips
      back; a Resync inside it does not start a stopped or paused beat.
 R105 A BPM Sync clip you fire while the beat is paused or stopped appears on its first frame and starts moving when
      the beat runs (from stop: on your play press, the "1"). A clip that is not in BPM Sync plays at once.
 R106 Stop is a cut: the clips go at once, like a layer's X. No fade.
 R107 Stop is not an Undo step. Until the Undo rule for the layer strip is built, Cmd+Z after a stop still undoes your
      last fire, as it does today, and that can put a clip back.
 R108 The row's pause and stop never change a clip's own pause: a clip you paused yourself stays paused, a clip that
      was playing plays when you fire it again, and nothing about the row's pause or stop is saved in the show.
 R109 Resync while the beat is stopped or paused: your press is the "1", the beat runs from it, the nudge reads 0.
      Tapping a tempo while it is stopped changes the number and starts nothing. (replaces R84)
 R110 An old show's Quantize setting is dropped the next time it is saved. An old recording made with Quantize on
      replays each press at the moment you pressed, not on the line it landed on then.
 R111 Routine pads keep their own start setting until actions replace them: by default a pad waits for the next bar. A
      pad you press while the beat is stopped waits, and starts one bar after you press play.
 R112 The text "nudge +12 ms" sits right after RESYNC, so your thirteen cells stay side by side as in your picture.
 R113 The cells are as tall as the top bar is today -- a little smaller than Resolume's.
 R114 In the key and pad list the old "Stop" reads "Stop routines" and the old "Play / Pause" reads "Audio play /
      pause", so neither can be taken for the row's "Beat stop" or "Beat play".
 NOT told, because he can neither see nor lose it: the clips' Snap values stay in the show file, unread (NC-7).
Rules of this delta that rest on a reading he did NOT correct (INFERRED consent), and the line that changes if he does:
 R87 / R100 (the clip's Snap goes) -> QO-2's combo and the clip half of `Layer::triggerClip`'s gate come back; the
   values are still in his files (NC-7). R100 was told with his LAST message: he has not yet had a turn to correct it.
 R101 (the thirteen cells, the caption "BPM", upper-case TAP / RESYNC) -> `rowOrder()` / `rowTexts()`.
 R94 (the pads go) -> R111 and NC-18: if the pads stay, PL2's draft question on the pad's Quantize menu is asked under
   a new number.
 R79 (the Bar text left of the circle) -> question 146.
 R86 (every fire at once), R97 (stop / pause / play in one sentence) -> none: his own words carry them.

## 8 HARMONY'S DECISIONS (each has a default)
HB-1  Stop and routines (NC-2). DEFAULT: `stopOnLayer` for every layer -- the X's own rule. ALTERNATIVES: `stopAll()`
      (one call; it also stops routines that hold no layer and resets every pad's marks; R103 then reads "every
      routine"); no call (R103 then says a running routine puts clips back after play; LR2's routine clauses, NC-13
      and MS7 go).
HB-2  quantize-out is a lane of its own, merged before S2's packet and before the transport lane's S1. DEFAULT yes.
      ALTERNATIVE (PL2's W1): stages QO-P..QO-2 inside the nudge worktree before S2; the transport lane then waits for
      the whole nudge lane.
HB-3  probe-boxes K5 `k5_queue_link_off` is struck from the boxes row's count, in writing. DEFAULT yes.
HB-4  S1's builder is told "T-N5b: the two-line rule" if his packet can still be told. DEFAULT yes.
HB-5  The clip's two dormant Snap fields (NC-7). DEFAULT: kept until he has confirmed R100 in words; then ONE short
      stage removes them (Clip.h, Clip.cpp, test_shared_field_types.cpp:19, lint clause d) -- the transport lane's
      panel stage may carry it. ALTERNATIVE: removed in QO-1 as PL2 wrote it, if FM-B5 finds no value in any file of
      his and she accepts that a show kept elsewhere loses its values on its next save.
HB-6  The top bar's height (NC-14). DEFAULT 34: cells 30. ALTERNATIVE 40: cells 36 -- `kTopBarHeight`, `kRowCell`
      and T-RB4's sums re-stated by the architect; every panel below moves 6 px; other lanes' baselines change.
      Decide after the visual gate, with his B-6.
HB-7  The two old overlay titles (NC-16). DEFAULT: S4r sets them, in today's words. ALTERNATIVE: the naming lane sets
      them; then Boris is not shown the key / pad list before that lane (the VG MUST blocks it).
HB-8  B-3's Cmd+Z sentence (NC-3). DEFAULT: left out unless the build he checks has the transport lane's Undo change
      for the layer strip; then it reads "Press Cmd+Z -> nothing comes back".
HB-9  Question 147 (NC-4). DEFAULT: asked with 144-146. ALTERNATIVE: held until FM-B4; under outcome A she may tell it
      as a reading instead.
HB-10 L3c with an existing routine as its reader. DEFAULT: run. ALTERNATIVE: struck -- L4 and T-N5b carry the bar; one
      probe row and one fixture less.
HB-11 A hand Resync travels in the run queue (NC-5), so a Resync pressed while RUNNING is applied up to one hop later
      than on main. DEFAULT: accepted. ALTERNATIVE: an architect line that orders Stop and Resync inside one hop
      another way.
RR's HR-1, HR-2, HR-4..HR-8, HR-10, HR-12, HR-14 STAND. HR-3 is applied by NC-16. HR-9 is PARKED for the actions lane.
HR-11 is VOID (no "R[]"). HR-13 is superseded (asked and answered). PL2's HB-1..HB-4 are HB-1..HB-4 above.

## 9 SIDE FINDINGS
SF-C1 MAIN: a clip taken off by the strip's X and fired again comes back PAUSED (V2, V3) -- the same hazard ST-1 found
      in the plan's Stop. INFERRED from the lines, not run. The X is also an Undo step and captures no take point
      (PL2 SF-B1). For the transport lane, whose TB-7 already says no clear writes the pause.
SF-C2 MAIN: `hasBeenTriggered` is set by the clip handler only (V3). A clip fired only by COLUMN fires auto-plays on
      every new activation; one fired once from its cell never does again. Two fire paths, two behaviours. For the
      transport lane's landing rule.
SF-C3 MAIN: the top bar's layout tests size the bar 1728 x 40 (V14); the app's bar at his window is 1720 x 34. They
      test a bar the app never shows. T-RW1 uses the app's size; the old cases are not re-sized in this lane.
SF-C4 MAIN: the legacy seek runs on whichever clip is active AFTER a fire; when the fire was only queued it seeks the
      OLD clip to the beat phase (V10). Removed by quantize-out.
SF-C5 PLAN: E5's reading of `stopOnLayer` is wrong (V5); NB-8's 40 / 36 / 1720 are not the app's (V14); "MQ1 RED" for
      a unit case that cannot reach the seek; QO-0 scheduled before its probe exists; B-3's and R107's Cmd+Z promise
      (V6); the stop tooltip left saying "stop the BPM timer" only (V19); T-N17b's second arm asserts a count the
      cited rule does not give (V16); "Resync then Stop in one hop ... the queue applies them in order" names an
      order main's code does not have (V17); "as in Resolume" for a one-press stop has no source in the record.
SF-C6 RR: V15's "inner width 1720" is the BAR's width at his window; the inner width is 1712 (V14). RR's "17 px to
      spare" was computed on it. Superseded by NC-14.
SF-C7 SEATS: both seats re-checked their citations, and every line re-read here was where they said, with two small
      offsets (GA-5's ":5078" is the snap's computation, the call is :5080; ST-2's "535-546" is :542-543). Three
      claims inside accepted attacks are wrong and change no verdict: GA-3's first remedy, "mutate the fire call",
      would not go RED (V12); ST-1's "set on every human fire" is true of clip fires, not of column fires (V3); ST-3's
      "it still re-seats the 1 as today" would let a replay turn a paused beat into a stopped one -- ruled stricter.
SF-C8 RECORD: R100 was told with his last message of 21:33:30. "Not corrected" starts to mean something only after
      his next message.

## 10 RISKS (the strongest counterargument first)
R1  STRONGEST: "Do not build stop-clears-the-layers in the beat lane at all. It is a clip-transport act. The transport
    lane is about to re-type a clip's play state (`paused` / `pausedAt`, TB-7) and to take fires out of Undo. Built
    now, on main's `playing` and main's Undo, it needs a special clear without the tail, makes Stop and the strip's X
    differ, and cannot keep 'Cmd+Z brings nothing back'. Let this lane ship pause and the timer; let the transport
    lane ship the clear." It loses because (1) his words make it ONE button of this row -- "stop clears all clips from
    layer strips" and "the beat stops" are one answer about one cell, and a stop that only stops the beat is the
    behaviour he replaced; (2) the clear without the tail is not a special path: it is the layer's own pure tuple
    function through the layer's own update call, and it leaves a clip exactly as a clip replaced by another fire is
    left today -- a state the transport lane must handle anyway; it writes no field that lane re-types (T-S3 pins
    that), so nothing built here is rebuilt there; (3) the Cmd+Z gap exists on main after every fire, with or without
    Stop: this ruling states it instead of promising around it. What would change the ruling: FM-B6 -- LR2 (b) FAILS
    with the tuple-only clear in place. Then the builder stops and the clear goes to the transport lane's architect.
R2  QUANTIZE-OUT FIRST (PL2 K1): engine work on the layer's trigger functions (Pitfall 63) in front of a lane whose S1
    is being built, for a control he could simply stop using. It loses on three verified facts: a clip's own Snap
    queues with the combo gone; an old show with the global value set queues with no control on screen; the legacy
    seek breaks "42 default". Cheapest refuting test: QO-0 -- if QL1 a does not fail on main, the removal is not
    needed as planned.
R3  NOTHING WAS RUN. Section 0 is from reading 7bc6df7; V16 and V17 are arithmetic on two rulings' text. A wrong
    reading of the order inside one hop passes this paper and fails the app. Refuting tests: T-G6, T-G6c, T-G6d, T-G7,
    T-N17b on the real classes, RED first; RN A24.
R4  THE HAND RESYNC NOW TRAVELS IN THE QUEUE (NC-5). It touches the one gesture main already has, by up to a hop.
    T-G6d is the guard that a Resync while running is still main's; HB-11 is Harmony's lever.
R5  THE F + 1 RULE (NC-6) lets the publish step RAISE the tracker's count in one arm. The counters are monotonic and
    H-9's alternative exists, but the arm was found on paper, not on a tracker. If T-N17b cannot be made green, the
    builder stops with the hop sequence.
R6  TWO DORMANT FIELDS (NC-7) stay in the model until HB-5. A later lane could read them by mistake; lint clause (d)
    is the fence. If he never confirms R100, they need an owner: HB-5 names one.
R7  THE HOLD BY dt = 0 IS INFERRED (PL2 K5) and now also carries a BPM Sync clip FIRED while held (Pitfalls 53, 56).
    FM-B3; if it shows black or pending the lane stops at S2r's rows.
R8  WHAT AN EFFECT SHOWS AFTER A STOP IS NOT KNOWN (NC-4). B-3 says only what is proved; FM-B4 measures; question 147
    asks. A stage hand who expects black and gets a trail is the failure this guards.
R9  30 PX CELLS may read small beside his picture (HB-6). The fit has 82 px at his width on two ASSUMED widths; the
    bar's size is verified in code and still to be confirmed in pixels (NC-14's table).
R10 A MIS-PRESSED STOP EMPTIES THE SCREEN, by mouse, key or pad (PL2 K6): question 144.
R11 A ROUTINE THAT HOLDS NO LAYER SURVIVES A STOP (NC-2): its pad still reads "running" while the beat is stopped, and
    it moves its sliders again after play. R103 says which routines stop; `stopAll()` is one call away (HB-1).
R12 A REPLAYED TAKE PUTS CLIPS BACK AFTER A STOP (PL2 K4): a stated limit (R104, B-9).
R13 IN AUTO, "RESYNC IS THE 1" is certain for one hop (RR NA-13) and then belongs to the detector (RR R7): the same
    exposure as play from stop, now reachable from two more states. FM-R3 measures it.
R14 EIGHT MUTANT APPS across the two lanes (q, q2, gate, edge, fold, stop, stop2, row): disk and build time; RN R9's
    rule applies (`df -h` first); HR-12.
R15 FOUR SHARED PLACES: BPMTracker.cpp (S1, S1r, the transport lane's S4t); Renderer.cpp's `syncMedia`; Composition.h's
    save and load lines (quantize-out, S2, S3r, the one-save lane); MainComponent's two fire handlers (quantize-out,
    the transport lane's S2). HR-1's order, "the second lane takes main in as its step 0" and the lints are the control.

FACTS HARMONY MUST MEASURE (each named where it is used; both outcomes ruled)
 FM-B1 QO-0: QL1 a, QL1 c, QL1 r, QL2 and QL3 on main's test-server build. Each FAILS: as ruled. QL1 a PASSES on
       main: STOP; PL2's W3 comes back to the architect.
 FM-B2 VG-0's manifest: the bar's bounds at his maximized window and the Master Signal label's width. NC-14's table.
 FM-B3 (RR's FM-R2, extended) a held BPM Sync clip, and one fired while held, in the real app: LR3 (c1), (c2), (e),
       LR2 (b), state T8, and probe-video.sh and probe-seq-vram.sh GREEN on the lane. On a frame and never pending:
       as ruled. Black, pending or a probe RED: STOP after S2r; RR R6's fallback goes to the architect.
 FM-B4 (= FM-S2) the picture after a stop under a holding or trailing effect: NC-4's two outcomes.
 FM-B5 (read-only; her own search of Boris's show and deck files, no app) how many clips carry "beatSnap": true or a
       "beatSnapMode" other than 0, and how many shows a "quantizeMode" other than 0. None: R110 may add "none of
       your shows used it", and HB-5's alternative is open. Some: B-1 names those clips ("these now fire at once").
 FM-B6 LR2 (b) on the lane at S2r: a clip taken off by Stop and fired again plays. PASS: as ruled. FAIL with the
       tuple-only clear in place: STOP; the call sequence goes to the architect (R1).
 FM-R3 (RR's) Auto, a start pressed on a kick: LR2's INFO arm, which gains ten rounds of a Resync from stop and ten
       of a Resync from pause. No bar; the hop log goes to the architect if beatInBar leaves 0 (RR HR-14).
 RR's FM-R4, FM-R5, FM-R6, FM-R7 stand.

SUMMARY: 16 attacks ruled (13 ACCEPT, 3 PARTIAL, 0 REJECT); 19 amendments (NC-1..NC-19); two lanes -- quantize-out
(QO-P, QO-0, QO-1, QO-2, VQ) and nudge (S1, S1r, S2, S2r, S3a, VG-0, S3m, S3r, S4, S4r, VG, S5); 4 Boris questions
(144..147) with defaults and 12 readings (R103..R114); 11 Harmony decisions with defaults (HB-1..HB-11); 7 facts to
measure. No STOP for S1.

STATUS: DONE
