# attack-transport-papers -- the blind council's seat papers on plan-transport.md (s-rta-1003b)
WHOLE. This file holds all five seat papers, verbatim and complete: 45 attacks.
Source: .harmony/.reports/s-rta-1003b/attack-transport-papers-full.json (75366 bytes, sha256 983b343c43f93e9c2109c2cc0859a2c5f274b354f0dc3a9e6497f6968e3b9504).
Content: each seat's paper as one JSON object, re-indented (2 spaces, non-ASCII kept); no word changed.
Rewritten by the architect in the completion round of ruling-transport.md. It REPLACES the earlier version of
this file, which was written from a dispatch cut at 70,000 characters (wf/transport-plan.js:73): that version
ended inside attack SC-6 of the scope seat (no proposed change; no SC-7, no SC-8, no strongest point). Checked in
this round, field by field: every attack that had reached the architect (ST-1..ST-10, UN-1..UN-8, RE-1..RE-9,
GA-1..GA-10, SC-1..SC-5, and SC-6's target, claim, evidence and severity) reads the same in the source file.
Seat order: the order of the first dispatch (the source file lists the scope seat first).
Seats: stage-operator (10 attacks: ST-1..ST-10), undo-model (8 attacks: UN-1..UN-8), real-time (9 attacks: RE-1..RE-9), gates (10 attacks: GA-1..GA-10), scope (MINIMALIST) (8 attacks: SC-1..SC-8)

## seat: stage-operator

```json
{
  "seat": "stage-operator",
  "attacks": [
    {
      "id": "ST-1",
      "target": "T5 / F13 'LIVE' definition vs `Clip::carryLiveTransportFrom` (plan lines 181-182, 427-429, 449); U-C3, TR13",
      "claim": "Undo still changes the transport of a live clip. The plan defines live transport as 'playing, playhead, direction, restartSeq' (line 429), but the carry helper copies only playing, playheadPosition, beatsPlayed, restartSeq and scrubHeld. SetClipCmd value-copies the WHOLE Clip, so reverse (direction), speed, loopMode, in/out points, transportMode and the new clipBpm come back from the old snapshot. Example: Boris adds an effect to the playing clip (a SetClipCmd step), toggles Reverse or trims the out-point (neither is an Undo step), presses Cmd+Z. The clip flips direction or loops differently on stage. U-C3 checks only playing, playhead and restartSeq. TR13 drops a file into ANOTHER cell, so it never exercises a same-id edit of the playing clip.",
      "evidence": "Plan lines 181-182 (carry list) vs 429 ('direction'). Pin 34179a2: src/core/ClipCommands.h:117 `cell = *state; // value copy`; src/MainComponent.cpp:1024 `makeSetClipCmd(... edit, fxDesc)` ('Add Effect' pushes a whole-clip before/after); src/model/Clip.h:122-128 speed/reverse/inPoint/outPoint are plain model fields. Boris verbatim (BF31): \"Let's not allow control Z to change anything that is live in the layer strip. It changes anything else\".",
      "severity": "MUST",
      "proposed_change": "Make a same-id SetClipCmd landing apply only what is NOT transport: carry loopMode, speed, reverse, inPoint, outPoint, transportMode, clipBpm and the runtime fields from the live clip. Add a U-C3 variant that toggles reverse, speed and out-point after the step and asserts they survive Undo and Redo. Change TR13 so the playing cell itself gets an Add Effect, a Reverse toggle, then Undo."
    },
    {
      "id": "ST-2",
      "target": "T3 F7 clamp + U15 (plan lines 282-283, 312-313)",
      "claim": "The clamp does not remove the jump Boris complained about. Dragging past a trimmed out-point lands at outPoint-1e-4, which is about 1 ms of a 10 s file. The next frame's write-back sees ph >= outPoint and seeks to the in-point, so the clip jumps to its start (Loop or PingPong) or stops (OneShot). That is cause C-B again, and 'no drop can be sent elsewhere' is false. U15 cannot fail on this: it asserts only that the write-back in the same step does not send it to the in-point, with no elapsed time.",
      "evidence": "Pin 34179a2 src/render/ClipTransportSync.h:56-70: `if (clip.outPoint < 1.0f && ph >= outPoint)` then OneShot stop, else `player.seekTo(inPoint)`. Plan line 282 'clamped to the range, so no drop can be sent elsewhere'. Boris verbatim: \"play from wherever I drop the play head\".",
      "severity": "MUST",
      "proposed_change": "Either clamp to a position at least one decoded frame plus one write-back period inside the out-point, or (better) refuse the clamp and let the marker region be a stop: a drop past the out-point leaves the playhead where it was and the clip keeps playing. Make U15 a timed case (advance the fake player by 2 frames after the drop). Add a trimmed out-point row to TR7."
    },
    {
      "id": "ST-3",
      "target": "T1 EVERY PATH table, key / MIDI / OSC rows (plan lines 210-211)",
      "claim": "Held keys and continuous controllers are not enumerated, and the plan turns them into restart storms on the column path. Bindings fire with value > 0 on every event, with no edge detection and no held-key state. Today a column re-fire is a no-op, so a repeating key (OS auto-repeat) or a CC fader bound to Trigger Column does nothing harmful. After the lane each repeat restarts every video in the column, and under Quantize each repeat re-queues. The video sits on its first frames while the key is held. The plan neither names this path nor adds a test.",
      "evidence": "Pin 34179a2 src/binding/BindingManager.cpp:52-71 (processKeyDown calls actionCallback_(b, 1.0f) with no held test), :196 (CC absolute passes outputValue on every message), src/MainComponent.cpp:4103 (processKeyDown called from keyPressed with no repeat filter), :7737 `if (value > 0.0f)` then handleColumnTrigger. That JUCE delivers auto-repeat to keyPressed is INFERRED (not read).",
      "severity": "SHOULD",
      "proposed_change": "Add the path to the table. Either drop key-repeat events in keyPressed (a held-key set) and edge-detect CC trigger bindings (fire on a 0-to-positive crossing), or state that repeats restart and let Boris decide. Add a unit case: 5 identical value-1.0 events in a row cause exactly one restartSeq bump."
    },
    {
      "id": "ST-4",
      "target": "T1 F2 (queue a re-fire) x Layer::releaseMomentary (plan lines 168, 211, 757-758 R4)",
      "claim": "Under F2 a piano / momentary press on a clip that is already active is queued, so pending == active for the first time. The plan says 'release = clear (or cancel the queue)', but releaseMomentary tests active first. A key released before the bar therefore CLEARS the playing clip instead of cancelling the queued restart. This is the same release BF12 asked to 'cancel'. R4 mentions the ordering but adds only a ClipCell paint test, and no expectation for what the release should do.",
      "evidence": "Pin 34179a2 src/model/Layer.h:509-526 (`if (r.activeRef() == ref) return clearedNext(r);` before the pending test at :516); :424 (today the queue test excludes the active ref, which F2 removes). Boris verbatim: \"Momentary pad released before its quantized beat: now cancels\".",
      "severity": "SHOULD",
      "proposed_change": "State the intended outcome and pin it in a unit case: when pending == active, a release cancels the queue and leaves the clip playing. Reorder the tests in releaseMomentary, or add a flag that records 'this pad's press only queued a restart'. Add it to the table row for piano / momentary."
    },
    {
      "id": "ST-5",
      "target": "T1 THE RULE: restart is carried out 'on the first frame that draws the clip' (plan lines 155-157, 186-190)",
      "claim": "A fire on a layer that is bypassed, hidden or not soloed does not restart anything. Those layers are skipped before the clip's frame provider runs, so restartSeq is not consumed. The restart then happens later, when Boris un-bypasses or un-solos, as a jump nobody asked for at that moment. 'On every path he can reach' also fails for the fire itself: the clip is 'playing' in the model but frozen. The plan's table says 'restart happens at once or on its beat' for all rows.",
      "evidence": "Pin 34179a2 src/render/CompositorEngine.cpp:1079-1080 (`if (!layer.visible || layer.bypassed || (anySolo && !layer.solo)) continue;`) precedes the only clip-texture calls at :1119 and :1226; the plan consumes restartSeq only from Renderer::syncMedia (line 186).",
      "severity": "SHOULD",
      "proposed_change": "Decide and state it. Option A: also run applyRestart for a layer that is skipped (a cheap non-drawing consume). Option B: document that a fire on a bypassed layer is deferred, and add a Boris check for it. Add a TR row: fire on a bypassed layer, wait 3 s, un-bypass, read t."
    },
    {
      "id": "ST-6",
      "target": "T3 gate: TR7, TR8 (plan lines 621-625), U13/U14, R10",
      "claim": "The live gate for BF25 cannot fail on the actual defect. TR7 and TR8 post to /api/debug/scrub, which calls scrubClip directly. The verified cause C-A is that the layer strip never reached a seek at all, and C-C is the marker-grab geometry. Neither the strip-to-DeckView-to-MainComponent callback chain nor ClipInspector's hit test is exercised live. U13/U14 test the components with a lambda, so a missing or mis-wired `onScrub` assignment in MainComponent passes every automated gate and leaves the strip bar inert. Only Boris's hand would find it, and the RED arm cannot run these rows.",
      "evidence": "Plan lines 299 (route calls scrubClip), 621-625 (TR7/TR8 marked GUARD), 772-773 (R10). The strip's wiring today: LayerStrip.cpp:979-989 writes the model only, so the gap is exactly the new callback chain.",
      "severity": "SHOULD",
      "proposed_change": "Add a lint (L3): `onScrub` is assigned in MainComponent.cpp and forwarded in DeckView, and `scrubPlayhead` has no direct playheadPosition write. Or add a test-mode route that injects a mouseDown/mouseDrag/mouseUp into the live LayerStrip and ClipInspector component at a pixel, so TR7/TR8 drive the real handlers."
    },
    {
      "id": "ST-7",
      "target": "T4 / section 7 check 10 and TR10 (plan lines 355-358, 720, 629-630)",
      "claim": "Check 10 ('open a saved show with BPM-synced clips. They move as they did before') is false for BPM-synced VIDEOS at any show tempo other than 120. Today a BPM-synced video plays at the fixed rate videoBeats/beatDivision whatever the tempo. After the lane it plays at show BPM / clipBpm. The plan itself says 'same speed at a show BPM of 120'. At 128 BPM an old show's videos run 6.7% faster with nothing on screen to say so (BF32 forbids a note). TR10 tests only a sequence, at 120. Boris would be told to expect no change and then see one.",
      "evidence": "Pin 34179a2 src/render/Renderer.cpp:1657-1664 (`player->setSpeed(clip->videoBeats / clip->beatDivision)`, bpm only a gate); plan line 357 and 720.",
      "severity": "SHOULD",
      "proposed_change": "Reword check 10 to 'at a tempo of 120 they move as before; at other tempos BPM-synced videos now follow the tempo'. Add a TR10 half that runs an old BPM-synced video at 90 BPM and states the expected rate change. Optionally put the old-show-video behaviour (fixed rate) to Boris as a question with a default."
    },
    {
      "id": "ST-8",
      "target": "T4 widgets: the x2 / /2 pair (plan lines 349, 392-393, 715-716)",
      "claim": "The button pair is given the same position, order and size as the Speed row's pair, but with the opposite meaning. On the Speed row x2 means twice as fast. In BPM Sync x2 doubles clipBpm and Beats, so the clip plays at HALF speed. A performer who switches a clip between the two modes mid-show, with the same buttons in the same place, gets the opposite result. Boris said only \"a x2 and /2 control to double or half easily\", which does not say what is doubled. The plan asks Q4 about Beats versus BPM but never asks what x2 means.",
      "evidence": "Plan line 392 ('same button pair, order and size as the Speed row's pair today'), line 349 ('twice the beats, half the speed'), line 716 ('Press x2: the clip plays at half speed'). Boris verbatim (answer 7): \"x2 and /2 control to double or half easily\".",
      "severity": "SHOULD",
      "proposed_change": "Add a question with a default: 'x2 doubles the beats (clip plays slower) or doubles the speed?' Or mirror the Speed row's meaning (x2 = faster = halves the beats) and say so in the check list. Put it to Boris before S4b."
    },
    {
      "id": "ST-9",
      "target": "Section 7 check 3 (Quantize re-fire) and TR6 (plan lines 707-708, 618-620)",
      "claim": "Check 3 cannot be performed as written, and a Boris who follows it will report 'wrong'. Quantize only acts while the tempo tracker is LOCKED; otherwise the mode silently becomes immediate, so with no music playing the re-fire restarts at once. Even when locked, in a column with an empty cell that layer is cleared at once (the plan keeps that, line 28 and 246), so 'nothing moves until the bar' is false for such a column. 'Next Beat' quantize waits for a beat, not a bar. The check names none of these preconditions.",
      "evidence": "Pin 34179a2 src/MainComponent.cpp:36-42 (`if (snap.trackerState != BPMTracker::STATE_LOCKED) return ...Off;`); plan line 28 (empty-cell clear stays immediate) and 246; Boris verbatim: \"if we are in Qantize mode, it is triggered on time by the Qantize method\".",
      "severity": "SHOULD",
      "proposed_change": "Rewrite check 3: 'play music until the beat wheel is locked; set Quantize to Next Downbeat; use a column with no empty cells'. Add the expected-result line for Next Beat. State in the plan that an unlocked tempo gives an immediate restart."
    },
    {
      "id": "ST-10",
      "target": "T5 'LIVE' scope, Q3 (plan lines 427-433, 458, 732-733)",
      "claim": "The plan reads \"anything that is live in the layer strip\" as tuple and transport only, and asks Boris just about bypass and solo. Undo of Move Layer (reorders the compositing stack, so it changes the output picture of playing layers) and of the other ToggleLayerFlagCmd flags is listed as '(b) nothing starts or stops' and left unchanged, with no question. The default of Q3 is that Cmd+Z may unhide or hide a playing layer. That is a visible change of what the audience sees, from a keystroke, in the layer strip Boris named.",
      "evidence": "Plan lines 431-433 (the narrow definition), 458 (MoveLayerCmd, ToggleLayerFlagCmd 'unchanged'), 732-733 (Q3 covers bypass and solo only). Pin 34179a2 src/core/DeckCommands.h:621-640 (MoveLayerCmd::undo reorders live layers); src/MainComponent.cpp:749-762 (bypass and solo are Undo steps). Boris verbatim: \"anything that is live in the layer strip\".",
      "severity": "SHOULD",
      "proposed_change": "Widen Q3 to: 'Cmd+Z on a layer move, bypass, solo or hide: undo it (default, as the plan has) or leave playing layers alone?' State in the plan that this reading is a narrowing of his sentence, and label it ASSUMED."
    }
  ],
  "strongest_point": "ST-1. The plan's own definition of live transport includes direction, but the carry helper leaves out reverse, speed, loop mode, in/out points and the new clipBpm. SetClipCmd copies the whole Clip. So Add Effect on a playing clip, then Reverse, then Cmd+Z, still changes the live clip on stage, which breaks BF31 on the plan's central path. The planned gates miss it: U-C3 checks three runtime fields, and TR13 drops a file into a different cell.",
  "citations_rechecked": true
}
```

## seat: undo-model

```json
{
  "seat": "undo-model",
  "attacks": [
    {
      "id": "UN-1",
      "target": "T5 table row SetClipCmd + carryLiveTransportFrom (plan 181-182, 449) vs goal G-E (plan 21); gate TR13 (plan 636-637)",
      "claim": "Undo/Redo of 'Replace Content' on a playing cell swaps the media of what the layer plays. The plan applies a same-id SetClipCmd landing on a live cell and carries only transport (playing, playhead, beatsPlayed, restartSeq, scrubHeld). The media identity is not carried. This breaks the plan's own G-E ('never starts, stops, replaces or rewinds what a layer plays') and Boris's 'not allow control Z to change anything that is live in the layer strip'.",
      "evidence": "The plan's live definition is the tuple plus transport, so the clip's content is not live (plan 427-431). Replace Content mutates the clip in place and keeps the id (src/MainComponent.cpp:7168 `existing->replaceContent`, pushed as a SetClipCmd 'Replace Content' at :7174-7177; Clip.h:254-330 copies mediaFile/mediaType/sequenceFiles/sourceType/name/thumbnail). SetClipCmd::apply does `cell = *state` (src/core/ClipCommands.h:117). src/core/MediaReconnect.h:5-9 says undo/redo of a video->video replaceContent 'keeps the same clip id but changes the media file', and makeClipMediaHook reopens the player (MainComponent.cpp ~5218-5226). So the playing clip's file changes under the layer on Cmd+Z. TR13 (plan 636) drops the file into ANOTHER cell, and U-C3 (plan 489-492) tests only transport fields. Nothing in the plan's gates can fail on this. Boris verbatim (boris-feedback-backlog.md:230): 'Let's not allow control Z to change anything that is live in the layer strip. It changes anything else'.",
      "severity": "MUST",
      "proposed_change": "Treat a live cell's content as live. A same-id SetClipCmd landing on a live cell is applied only for fields that cannot change the picture or the clip's identity, or it is left whole when mediaFile, mediaType, sequenceFiles, sourceType or sourceParams differ. Add a unit case 'Undo and Redo of Replace Content on a playing cell: mediaFile, name and player unchanged'. Change TR13 to replace the PLAYING cell's content, then undo and redo."
    },
    {
      "id": "UN-2",
      "target": "F14 collision rule 'the step is spent either way' + SetClipCmd/SwapClipsCmd skip rows (plan 436-440, 449-450); R1 (plan 742-749); U-C12 (plan 501-502)",
      "claim": "A skipped step breaks the linear-history contract. A later step in the same stack then acts on a model it was not recorded against, and a clip id can land in two cells. Sequence: (1) Drop A into cell1 (SetClipCmd); (2) Move Clip A cell1->cell2 (SwapClipsCmd); (3) fire cell2, so it is live; (4) Undo, which skips the Swap because cell2 is live; (5) Undo, where the Drop's undo lands 'empty' on cell1 (idle, so applied; a no-op); (6) Redo, where the Drop's redo lands A on cell1 (idle, so applied); (7) Redo, where the Swap is skipped again because cell2 is live. Result: the same clip id sits in two cells, one of them live, and it stays that way.",
      "evidence": "Plan 450 ('SwapClipsCmd ... skipped whole when either cell is live') and 449 ('another clip or empty landing on a live cell: that cell is left'). The commands restore by value, with no 'applied' state: SetClipCmd::apply `cell = *state` (ClipCommands.h:117) and SwapClipsCmd::applyCell (:256). UndoManager only moves currentIndex_ (UndoManager.h:51) and has no notion of a skipped step. The dispose hook states the invariant: 'a clip id can appear in AT MOST one live cell' (MainComponent.cpp:5247-5266), and it merely fails safe (leaks) when that is violated. R1 claims 'a skipped part changes nothing' and 'every command restores by value and by id'. Its only test, U-C12 (plan 501-502), asserts rowsEqualLayers and id resolution, not clip-id uniqueness. The 200-step random walk is conditional ('add it to U-C12 if the council asks', plan 747-748), so it is not in the gate.",
      "severity": "MUST",
      "proposed_change": "Make the walk a gate case now: 200+ random Undo/Redo steps with fires between them, asserting unique clip ids across every deck including retired ones, rowsEqualLayers, and no tuple change. Add an id-uniqueness guard to every landing (SetClipCmd or SwapClipsCmd refuses to land an id already present in another cell). Give every skippable command an applied_ state, as RemoveLayerCmd::erased_ has, so Redo or Undo of a spent-but-skipped step is a no-op."
    },
    {
      "id": "UN-3",
      "target": "T5 table row RemoveColumnCmd (plan 451); U-C6 (plan 495)",
      "claim": "RemoveColumnCmd: after a skipped Redo, the next Undo re-inserts the removed cells again and the deck gets a duplicated column. The plan specifies 'Redo: skipped when any tuple names that deck's column' and nothing for the Undo that follows. U-C6 tests only the skipped Redo.",
      "evidence": "Sequence: Remove Column (last column c); Undo re-inserts; fire a clip in c; Redo is skipped, but UndoManager counts it redone (UndoManager.h:51 currentIndex_); Undo again. RemoveColumnCmd::undo (src/core/DeckCommands.h:141-158) inserts `removedCells_[l]` at `column_` in every row unconditionally (:153) and sets numColumns = columnsBefore_. The column already exists, so every row gains a duplicate cell. The restored clips are value copies with the same ids as the live ones (duplicate ids, and the mediaHook runs again). The plan gave an erased_-style guard only to RemoveLayerCmd ('its Undo is then a no-op', U-C8, plan 497-498). The same hole exists for any other command whose Redo is skipped: AddLayerCmd::undo erases addedIndex_ unconditionally (DeckCommands.h:511) once its Redo 'does nothing'.",
      "severity": "MUST",
      "proposed_change": "Give RemoveColumnCmd, AddLayerCmd, InsertDeckCmd, SwapClipsCmd and ClearLayerClipsCmd an applied_/erased_ flag set by what execute/undo really did. Undo and Redo are no-ops when the flag says the step's effect was never applied or was already reverted. Add U-C6b: Remove Column, Undo, fire in c, Redo (skipped), Undo; assert numColumns, row sizes and unique ids."
    },
    {
      "id": "UN-4",
      "target": "T5 table row ClearLayerClipsCmd (plan 448) and the change list (plan 478-479: carryLiveTransportFrom wired only into SetClipCmd and SwapClipsCmd); U-C2 (plan 489-490)",
      "claim": "ClearLayerClipsCmd restores by replacing the whole row's clips with a stale snapshot. The plan wires the live-transport carry only into SetClipCmd and SwapClipsCmd, so Undo/Redo/Undo rewinds or restarts a live clip. Sequence: Clear row; Undo (clips return, tuple empty); fire X (live, restartSeq bumped); Redo ('a live cell stays'); Undo. Undo then overwrites live X with the snapshot copy, whose playing, playhead and restartSeq are stale.",
      "evidence": "ClearLayerClipsCmd::apply does `row->clips = state.clips; // value copy` (src/core/DeckCommands.h:336) with before_ snapshots taken at clear time. The plan itself describes the post-Redo state (plan 448: 'a live cell stays') but gives that command no carry (change list, plan 478-479). T1's consume rule restarts a player when `clip.restartSeq` merely DIFFERS from the last seen value (plan 187-188), so a stale lower counter written by Undo restarts the live clip. The plan notes the 'counter went DOWN' guard only as a fallback in R3 (plan 755). U-C2 (plan 489-490) covers one Undo and one Redo only. Per-cell 'live cell stays' also cannot be done with a whole-vector assignment as written.",
      "severity": "MUST",
      "proposed_change": "Rewrite ClearLayerClipsCmd::apply to go per cell: skip cells that are live, and call carryLiveTransportFrom for same-id cells. Make the consume side ignore a restartSeq that is not greater than the last seen (wrap-safe compare), as a defence for every value-copy path including composition load and take restore. Add U-C2b with the Undo, fire, Redo, Undo sequence asserting playing, playhead and restartSeq unchanged on X."
    },
    {
      "id": "UN-5",
      "target": "F13 'live' definition (plan 427-433), T5 table rows AddDeckCmd/RemoveDeckCmd/InsertDeckCmd (plan 455-457), lint L2 (plan 482-484), TR11-TR13",
      "claim": "The plan's three 'live' predicates disagree, and its lint cannot catch the real tuple writers. F13 says a queued (pending) clip and a fading-out previous clip are live, and 'Undo and Redo never write any of these'. But AddDeckCmd::undo, InsertDeckCmd::undo and RemoveDeckCmd::execute (which Redo runs) all call cancelPendingInto, which rewrites the pending slot of every layer. The plan marks these commands '(b) cannot change what plays / unchanged'. A queued fire into a deck that is un-added is cancelled by Cmd+Z. Lint L2 greps only `setRuntime(`, and cancelPendingInto writes through `updateRuntime`.",
      "evidence": "cancelPendingInto uses `layer.updateRuntime` (src/core/DeckCommands.h:195-206) and is called at :727 (AddDeckCmd::undo), :854 (InsertDeckCmd::undo) and :946 (RemoveDeckCmd::execute). Plan table 456-457 classifies Add/Remove Deck as '(b) ... unchanged'. L2 (plan 482-484): '`setRuntime(` occurs only as `setRuntime(LayerRuntimeSnapshot{})`'. clearTupleForRow also uses updateRuntime (:272-274). Three predicates exist. tupleNamesDeck is active-or-pending (DeckCommands.h:226-230). deckIsPlaying is active, or previous while crossfadeProgress < 1, and never pending (src/model/Composition.h:608-618, used by retireOrEraseDeck :628). The plan's layerIsLive/cellIsLive is active, pending and previous-running (plan 472-473). The InsertDeck row says only 'a tuple names the deck' (plan 455). With tupleNamesDeck a deck whose clip is the previous of a running crossfade counts as not live, so the deck is erased and the outgoing clip is cut, which is exactly the crossfade edge the brief names. With deckIsPlaying a queued clip is silently cancelled. No gate row (TR11-TR13, U-C1..U-C13) queues a trigger into a deck or sets up a deck whose clip is the previous of a running fade; MU-T5f covers only a SetClipCmd variant mid-fade (plan 506).",
      "severity": "MUST",
      "proposed_change": "Define ONE deck-level predicate deckIsLive (active, pending, previous while crossfadeProgress < 1), and use it for the Add/Insert/Remove Deck undo and redo as well as Composition::deckIsPlaying. Either retire the deck while a pending ref names it (the queue then fires from the retired deck) or state that cancelling a queue is the single sanctioned exception, and ask Boris. Extend L2 to forbid `updateRuntime(`, `clearTupleForRow(` and `cancelPendingInto(` in *Commands.h unless allow-listed by name. Add unit cases: undo of Add Deck with a queued trigger into it; Insert Deck whose clip is the previous of a fade."
    },
    {
      "id": "UN-6",
      "target": "F18 Load Deck that added layers (plan 465-466) and T5 table InsertDeckCmd row (plan 455)",
      "claim": "When anything of a Load Deck is live, the plan keeps ALL added layers, idle ones included. That is a behaviour Boris never answered, and it contradicts 'It changes anything else'. Cmd+Z leaves the show with extra empty layers that the reaped deck no longer feeds, and nothing is shown or asked about it.",
      "evidence": "Plan 465-466 rejects 'erase the idle added layers one by one' because 'erasing a layer erases that row in the retired deck too (rows == layers)'. The Boris answers record (boris-feedback-backlog.md:226-230) shows Harmony's question A offered 'Cmd+Z takes those layers and their clips away' as the exception, and Boris answered only 'Let's not allow control Z to change anything that is live in the layer strip. It changes anything else'. Questions Q1-Q6 (plan 727-738) do not include this outcome. The retired deck is reaped once nothing names it (Composition.h:650-662), after which the kept layers have no source and Redo of the Load Deck (re-insert the capture, DeckCommands.h:798-808) re-inserts rows against a layer count that may differ.",
      "severity": "SHOULD",
      "proposed_change": "Add a Boris question with the plain outcome: 'You load a deck that added 2 layers, fire a clip on one of them, press Cmd+Z: both layers stay (default) or only the one that plays?'. Pre-state in the plan that the idle added layers are kept and why. Add a unit case: Redo after the retired deck was reaped while the added layers were kept; assert rowsEqualLayers."
    },
    {
      "id": "UN-7",
      "target": "Q3 / F13 NOT-live list: bypass and solo (plan 432-433, 458, 732-733)",
      "claim": "The plan's default for Q3 is Undo still toggles bypass/solo, but Boris's wording is 'anything that is live in the layer strip', and bypass and solo are layer-strip controls that change what is on the output. Undo of 'Unbypass' hides a playing layer and Undo of 'Solo' mutes all the others. These are instant picture changes of exactly the 'Cmd+Z surprises me on stage' kind. The plan's justification covers effects only ('nearly every edit is made on what is playing').",
      "evidence": "Boris verbatim (boris-feedback-backlog.md:230): 'Let's not allow control Z to change anything that is live in the layer strip.' ToggleLayerFlagCmd flips layer->bypassed/solo (src/core/DeckCommands.h:~456-469, pushed at MainComponent.cpp:753-766) and is classified '(b) nothing starts or stops' (plan 458), which is true only if what is on screen is not 'what plays'. G-E (plan 21) says 'never starts, stops, replaces or rewinds'; bypass is none of those words but is the same effect on the output. The plan resolves it by a default (Q3) with 'nothing waits', so the shipped build carries the less literal reading.",
      "severity": "SHOULD",
      "proposed_change": "Make the Q3 default the literal one: a flag toggle on a layer whose tuple is live is left (spent, like the cell rule), and ask Boris whether he wants the looser reading. Or add the bypass/solo question to the Boris page, step 11, as an explicit check before merge."
    },
    {
      "id": "UN-8",
      "target": "Gate row TR12 undo_load_deck_keeps_playing, RED arm (plan 633-635); ASSUMED note (plan 531-532)",
      "claim": "The TR12 RED-arm observation ('RED arm: 3 layers, the ramp gone') is wrong for the frozen pre-lane app, so the row does not prove T6h on that arm. On main, a REST fire pushes a TriggerClipCmd, so the probe's single `/api/debug/undo` undoes the FIRE, not the Load Deck. The deck stays and 5 layers remain. The row fails for a different reason than the one registered, and the 'teeth' statement is wrong.",
      "evidence": "Plan 633-635: 'fire its row-0 clip ...; 1 s; undo. Bar: 5 layers, the deck absent from decks ... RED arm: 3 layers, the ramp gone.' ruling-bf9b-merge.md:152 (GA-5): 'REST and OSC fires push TriggerClipCmd. k9d as written would FAIL its own VALID clause. No honest REST driver exists'. Plan 531-532 only ASSUMES no probe posts undo right after a fire. TR11 relies on the same fact for its RED arm ('the layer is empty after the undo'), which is correct, so the plan knows a REST fire is an Undo step on the RED arm but registers a contradictory RED result for TR12.",
      "severity": "SHOULD",
      "proposed_change": "Register TR12's RED arm as 'deck still present, 5 layers, ramp stopped (the first undo undid the fire)'. For a RED arm that really exercises T6h, drive the fire with a routine or take replay (which pushes no step), per ruling-bf9b-merge SF-5 (:659-660). Add the Undo, Redo, Undo case to the live row set (see UN-2)."
    }
  ],
  "strongest_point": "UN-1. A user replaces the media of a playing clip (Replace Content), presses Cmd+Z, and the video playing on the layer is swapped for the old file. The plan's own goal G-E promises Cmd+Z never 'replaces ... what a layer plays', and Boris said 'Let's not allow control Z to change anything that is live in the layer strip'. The same-id landing rule carries only transport fields (plan 181-182, 449), the Replace Content step keeps the clip id and changes the file (MainComponent.cpp:7168-7177; MediaReconnect.h:5-9), and TR13 and U-C3 test only a different cell or only transport fields, so no gate row in the plan can fail on it.",
  "citations_rechecked": true
}
```

## seat: real-time

```json
{
  "seat": "real-time",
  "attacks": [
    {
      "id": "RE-1",
      "target": "T1 F2 / THE CHANGE (Layer.h): 'drop `&& ref != r.activeRef()` from the queue test; the release already runs the tail'",
      "claim": "A queued re-fire restarts the clip at QUEUE time, not on the bar. Layer::activate's beforeCas hook runs applyActivationTail whenever `to.activeRef() == ref`. For a re-fire that is only queued, the active ref is unchanged, so that test is true and the tail (playhead = start, playing = true, restartSeq bump) runs on the message thread at the click. The render thread then restarts the player at once. This defeats F2 and Boris's 'it is triggered on time by the Qantize method'. The plan only names the release-side tail (:484-487) and never the queue-side hook.",
      "evidence": "34179a2:src/model/Layer.h:572-576 (activate: `if (to.activeRef() == ref) applyActivationTail(rows, from, to);` inside the beforeCas lambda) and :340-352 (beforeCas runs before EVERY CAS attempt that would install `to`, and a queued re-fire installs a changed pending slot). Today :424 `snapEnabled && ref != r.activeRef()` keeps a re-fire out of the queue, so this hook never saw one. Plan lines 166-168 and 183-185; U3 (line 221) is worded 'queued; the release restarts it' with no assertion that restartSeq is unchanged between queue and release; TR6 (618-620) is the only catch and it is live-only.",
      "severity": "MUST",
      "proposed_change": "Make the hook skip the tail when `to.pendingTriggerColumn >= 0 && to.pendingRef() == ref && to.activeRef() == from.activeRef()` (the transition only queues). Add to U3 `restartSeq`, `playing` and the model playhead are unchanged right after the queued call and change only after processPendingTrigger. Add a mutant that removes the skip. Apply the same check to the post-step at :577-579."
    },
    {
      "id": "RE-2",
      "target": "T3 THE CHANGE: 'pushIntent pushes wanted && !scrubHeld ... writeBack skips the play-state compare-exchange while held'",
      "claim": "scrubHeld is read twice in one syncMedia (once in pushIntent, once in writeBack), so a mouseUp landing between them stops the clip for good. pushIntent sees held and pauses the player. Up clears the hold. writeBack then sees not-held and runs CAS(clip.playing: wanted(true) -> player.isPlaying()(false)), which succeeds and clears the intent. The clip stays paused after the drop, which is the opposite of BF25 ('keep playing ... from wherever I drop'). This is the lost-update class that ClipTransportSync.h was written to remove: the header says the intent is read ONCE.",
      "evidence": "34179a2:src/render/ClipTransportSync.h:9-17 (read the intent once, CAS against it), :33-41 (pushIntent returns only `wanted`), :44-53 (writeBack CAS). Renderer.cpp:1644 (pushIntent) and :1673-1676 (advanceFrame then writeBack) leave a window between the two reads in every drawn frame. Plan lines 288-290; U11 (line 305) steps Down then Up in sequence and never flips the hold inside one sync. The existing four test_clip_transport_sync cases (tests/test_clip_transport_sync.cpp:35,55,86,104) show the in-window-flip pattern the new case should copy.",
      "severity": "MUST",
      "proposed_change": "pushIntent returns a struct {wanted, held} (or writeBack takes the held value pushIntent read). writeBack uses only that local. Add a fake-player case whose setPlaying/advance hook clears scrubHeld between the two calls and asserts clip.playing is still true. Add a mutant that re-reads scrubHeld in writeBack."
    },
    {
      "id": "RE-3",
      "target": "T3 hold lifecycle / R7 / F6 (scrubHeld ends on Up, setClip, destruction)",
      "claim": "A hold can stick on the wrong clip and freeze a live clip permanently, and nothing in the plan clears it. LayerStrip re-resolves `playingClip()` on EVERY mouse event. In a show the playing clip changes during a drag (quantized release, Autopilot advance, MIDI/OSC fire). Down sets the hold on clip A, then Move and Up arrive with clip B. A is never released. A is still scrubHeld when fired again, and pushIntent pushes `wanted && !scrubHeld`, so even a fire (the tail sets playing = true) cannot un-pause it. The plan's cases are only 're-pointed or destroyed' and 'lost mouseUp'. The 10-second render-side watchdog is deferred as 'if the council worries' (line 766-767).",
      "evidence": "34179a2:src/ui/LayerStrip.cpp:979-989 (scrubPlayhead calls playingClip() per event) and :971-976 (mouseDrag). ClipInspector.cpp:1379-1404 (drag state is held per DragTarget, but clip_ can be re-set while the mouse is down). Plan lines 291-298 (scrubClip(Clip&, ...) gets a Clip per call), 183-185 (the tail does not touch scrubHeld), 181-182 (carryLiveTransportFrom copies scrubHeld across value copies), 765-767 (R7).",
      "severity": "MUST",
      "proposed_change": "The scrub session owns the clip id captured at Down. Move and Up act on that clip, never on a re-resolved one. applyActivationTail (and restart()) clears scrubHeld. The render-side watchdog becomes part of the build, not a fallback: a hold with no scrub call for N ms is dropped. carryLiveTransportFrom must not copy scrubHeld. Add U14b: the layer's playing clip changes between Down and Up, and both clips end unheld."
    },
    {
      "id": "RE-4",
      "target": "T3 gate TR7/TR8 + U10/U14 (BF25 proof)",
      "claim": "No live row can fail on the defect Boris reported. Both rows call a NEW test route that invokes scrubClip directly, so they exercise neither LayerStrip::scrubPlayhead (the actual cause C-A: it writes only the model) nor ClipInspector's handlers nor the MainComponent wiring of onScrub. If `onScrub` is left unwired, or the strip keeps its direct model write, TR7 and TR8 still pass. The plan admits only Boris's hand proves BF25 against the old app (R10). U14 tests that the strip calls a callback, but nothing tests that MainComponent assigns it.",
      "evidence": "Plan lines 299 (`POST /api/debug/scrub ... calls scrubClip`), 621-625 (TR7/TR8, both marked GUARD), 772-773 (R10). 34179a2:src/ui/LayerStrip.cpp:988 (`clip->playheadPosition = ...` is the direct write; no seek callback exists) and :959-963 (the mouse entry). 34179a2:src/MainComponent.cpp:1464-1477 (the existing wiring point, which is exactly what goes unchecked).",
      "severity": "MUST",
      "proposed_change": "Make the debug route deliver in-process JUCE MouseEvents (down, drag, up) to the real LayerStrip and ClipInspector components, not call scrubClip. That is not OS input, so the 'no synthetic input' launch rule is unaffected. Alternatively add a headless MainComponent-level case that asserts both callbacks are non-null after construction. Add a mutant 'onScrub unwired' that must turn a live row RED."
    },
    {
      "id": "RE-5",
      "target": "T4 THE MODEL: 'ONE new saved field: float Clip::clipBpm = 120' read by Renderer::syncMedia each frame",
      "claim": "clipBpm is a plain float that the GL thread reads every drawn frame while the message thread (BPM box, x2, /2, right-click) and the httplib thread (the new /api/set_clip_param `clipBpm` and `beats` keys) write it. The plan makes restartSeq and scrubHeld Relaxed but not this field. The house rule is 'never add a plain field that one thread writes and another reads'. A 4-byte float race is UB under TSan, and the plan's TSan case U8 covers restartSeq only. test_shared_field_types would not pin clipBpm either.",
      "evidence": "34179a2:docs/claude/pitfalls.md:137 (Pitfall 63 rule 1: never add a plain field that one thread writes and another (GL, httplib, a worker) reads). 34179a2:src/model/Relaxed.h:5-9. 34179a2:tests/test_shared_field_types.cpp:29-37 (the pin list). Plan lines 334 (plain float), 179-180 and 287 (the two Relaxed fields), 228 (U8), 386 (the REST write).",
      "severity": "MUST",
      "proposed_change": "Declare `RelaxedFloat clipBpm`. Add a static_assert pin in tests/test_shared_field_types.cpp. Extend U8, or add a TSan case, with a message-thread setter against the GL-side bpmSyncRate read. The Beats box and the REST `beats` setter both go through clipBpm. Apply the same check to every other new cross-thread read (the 'last known BPM' stays GL-only; `mediaSeconds` is read on the message thread under videoPlayerMutex_, which is fine because duration_ is written in open() before the player is published, Renderer.cpp:1348-1377)."
    },
    {
      "id": "RE-6",
      "target": "T1 mechanism F3 (restart = a seek on the render thread) + gate TR2/TR3/TR4",
      "claim": "Moving the restart to a render-thread seek makes every quantized release and every column fire seek parked players. For a clip returning from parked, the layer then HOLDS the old frame (the frame the clip showed when it left) until the decode thread re-seeks to a keyframe and catches up. Nothing pre-rolls the player when the fire is queued, so the new start misses the bar by the decode latency. A column of N layers does N such seeks at once. The live gates cannot see any of this: render_frame waits for a complete frame (Pitfall 53), the tolerance is 0.25 s, TR2 uses two layers, and the fixture is a short all-ramp clip. The project already recorded a 3.4 s whole-output freeze from mid-GOP retrigger seeks.",
      "evidence": "34179a2:src/media/VideoPlayer.cpp:354-386 (seekTo is a request; advanceFrame consumes it and bumps the generation, so the decode thread re-seeks). 34179a2:docs/claude/pitfalls.md:121 (Pitfall 56: no frame ready = HOLD the last shown frame; 'a mid-GOP retrigger ... re-seeks ... while the layer holds -- it used to freeze the WHOLE output at 14 / 3.8 fps for 3.4 s'). CLAUDE.md index entry 53 ('render_frame waits for a complete frame'). Plan lines 186-190 (applyRestart in syncMedia), 608-620 (TR1-TR6, tol = 0.25 s, 'capture at el 0.3'), 609 (two layers).",
      "severity": "SHOULD",
      "proposed_change": "Pre-roll: when a fire is queued (pending slot set), have the render thread seek the pending clip's player to its restart position, paused, so the first frame is ready on the bar. Add to TR2/TR6 an 8-layer long-GOP column restart, with an inter-frame time p99 bar and `video_late_frames` / hold counters read before and after (the fixture needs a GOP of at least 2 s with the in-point mid-GOP). Add one capture at el 0.0-0.05 that must NOT show the pre-fire frame, since the hold is the failure."
    },
    {
      "id": "RE-7",
      "target": "T1 restartSeq consume rule: 'a fresh player's seen value is 0' + T5 'a restored clip never restarts' (U-C4)",
      "claim": "Any re-open of a live clip's player restarts it, which Undo is forbidden to do (BF31 'never rewinds'). restartSeen_ lives in the player and starts at 0, and the plan relies on that so a clip fired before its player opened still starts at its start. installVideoPlayer replaces the player whenever media is reconnected (SetClipCmd's media hook on Undo or Redo of a same-id content edit, Replace Content), so the new player sees clip.restartSeq = N > 0, finds N != 0 and restarts the clip. U-C4 asserts only that the counter value is unchanged, not that the player is not seeked. R3's fallback ('ignore a counter that went DOWN') does not apply: the counter did not go down, the player's seen value did.",
      "evidence": "34179a2:src/render/Renderer.cpp:1355-1383 (installVideoPlayer: closeMediaForClip, then a new player). 34179a2:src/core/ClipCommands.h:83 (SetClipCmd takes a ClipMediaHook). Plan lines 187-190 (seen value 0 on a fresh player), 181-182 and 494-495 (U-C3/U-C4), 752-755 (R3, 'NOT read: composition load, take restore, Duplicate Deck').",
      "severity": "SHOULD",
      "proposed_change": "Distinguish 'fired while the player was not open' from 'player re-opened under a live clip'. For example, installVideoPlayer seeds seen = clip.restartSeq when the clip has already been drawn (lastDrawn / playing), and seeds 0 only for a never-drawn clip. U-C4 becomes a render-level case: Undo of a same-id content edit on a playing clip and the new player's first sync makes no seek."
    },
    {
      "id": "RE-8",
      "target": "T4 F9 (rate = show BPM / clip BPM, never a position) + TR9",
      "claim": "The rate is taken straight from the snapshot's raw `bpm` with no smoothing and no use of trackerState, and nothing in the gate bounds drift. The tracker value is a 'stabilized/locked estimate' that moves between analysis hops, so every wobble is a visible speed change. A rate-only clip also slides against the beat over minutes; R5 admits this and defers the check to 'cheapest test', outside the gate. TR9 uses `set_bpm` (a steady manual tempo) and 2-second windows, so neither wobble nor drift can fail it. Boris's own sentence ('where it should be playing') shows he believes a BPM-synced clip HAS a should-be position, and the plan reads that line only as a drag complaint.",
      "evidence": "34179a2:src/analysis/FeatureSnapshot.h:40 (bpm = 'stabilized/locked tempo estimate ... 0 if unknown'), :42 (trackerState 0/1/2), :128-136 (the integrated beat clock exists and is the house way to follow tempo, Pitfall 42). Plan lines 340-348 (rate, 'a Tap, a typed tempo, Link or the tracker moves the speed within one analysis hop'), 369-371 (F9 (ii)), 626-628 (TR9), 759-762 (R5). Boris verbatim, .harmony/boris-feedback-backlog.md:121 ('I do not want it to jump back to where it was or where it should be playing before I grabbed it') and the BF15 line 'always follows the current BPM'.",
      "severity": "SHOULD",
      "proposed_change": "Add a gate row TR9b: an audio-file source with a locked tracker, 5 minutes, drift read from decoded pixels, with a pre-registered bar (for example under a quarter beat). Add a second row with the tracker unlocked: the clip's speed must not change more than X% per second. Slew-limit the rate, or hold the last LOCKED tempo while trackerState != kTrackerLocked. State in the plan that a phase correction on Quantize start (not on drop) is the fallback if the drift bar fails."
    },
    {
      "id": "RE-9",
      "target": "T1 F1 / T6 row 1 and Boris column ('restart')",
      "claim": "The plan credits Boris's word 'restart' for flipping the play state on a fire (`playing == true`, a paused clip now plays). His verbatim answer covers the start position only: 'Every fire of a video starts it from its start (in point)'. The 'no stop buttons' sentence the plan uses is from the routines section (BORIS_DECISIONS.md:365) and the app still has pause. The plan itself calls this a default put to Boris as Q1, but T6 lists the changed test expectation and the removal of Pitfall 7 under 'Boris: restart' as if settled. The test that now asserts playing == true is attributed to words he did not say. The real defect to fix is narrower: an ended OneShot stays dead.",
      "evidence": ".harmony/boris-feedback-backlog.md:183 and :200-201 ('2 restart' and the question 'continue where it left off (today) or restart?'). BORIS_DECISIONS.md:365 (context is the routine stop model). Plan lines 160-165 (F1), 513 (T6 row 1, Boris column 'restart'), 163-164 ('Pitfall 7 are replaced'), 729-730 (Q1).",
      "severity": "SHOULD",
      "proposed_change": "Label F1 INFERRED, not Boris's word, in T6 and the Boris column. Ship the narrow rule by default: the tail sets playing = true only for an ended OneShot or a never-drawn clip, and restarts the playhead for every fire. Flip to 'always play' only on his Q1 answer. A paused clip then stays paused at its start, which is the exact runner-up the plan already knows how to build."
    }
  ],
  "strongest_point": "RE-1. Layer::activate's beforeCas hook (Layer.h:572-576) runs the activation tail whenever to.activeRef()==ref. Once F2 lets a re-fire of the active clip into the queue, the tail (playing=true, playhead=start, restartSeq bump) runs at the click, and the render thread restarts the video at once. The plan's central Quantize guarantee ('triggered on time', T1/F2/TR6) fails by construction, and U3 as worded would not notice. RE-2 and RE-3 are close behind: the scrub hold has a double-read race and a stuck-hold path that both freeze or stop a live clip on stage.",
  "citations_rechecked": true
}
```

## seat: gates

```json
{
  "seat": "gates",
  "attacks": [
    {
      "id": "GA-1",
      "target": "plan section 5.1 G-U1 (and G-U2/G-U3 which lean on it)",
      "claim": "The headline unit gate selects tests by FILE-name tokens with ctest -R. In this repo ctest -R matches Catch2 TEST_CASE names, so the command can select few or zero of the named cases and still print 100% passed. The plan's whole 'U1-U20 and U-C1..U-C13 fail by name on the base, pass at the head' claim rests on this string.",
      "evidence": "Plan line 591: ctest -R \"layer_runtime|clip_transport_sync|show_model|undo_commands|composition|clip_inspector|layer_strip|render_thread_lint|shared_field_types\" with no --no-tests=error. Case names at 34179a2 do not carry those tokens: tests/test_show_model.cpp TEST_CASE \"T6h THE EXCEPTION, pinned: ...\" [show]; tests/test_undo_commands.cpp \"SetClipCmd: drop onto an empty cell\" [undo][setclip]; tests/test_layer_runtime.cpp \"LayerRuntimeCell: pack / unpack round trip ...\" [layer_runtime] (a tag, not a name). tests/CMakeLists.txt uses plain catch_discover_tests(test_show_model) at 3511, 594, 3436, 3627 (no TEST_PREFIX). The repo already ruled on exactly this: .harmony/.reports/s-rta-1002b/ruling-bf6.md:563-565 'ctest -R matches Catch2 TEST_CASE names, not target names ... can select 0 tests and pass without running anything. Run the binaries by path and check --list-tests'.",
      "severity": "MUST",
      "proposed_change": "Replace G-U1 with per-binary runs by path (build/tests/test_show_model etc.) plus a pinned --list-tests count per binary, or use ctest -L with labels / -N to print the selected count and fail below an EXPECTED floor (the probe-asan-unit.sh pattern, EXPECTED_ASAN_CASES). Name the expected case names (U1..U20, U-C1..U-C13) in the gate and require each to appear in the run output as passed on the head and failed on the base."
    },
    {
      "id": "GA-2",
      "target": "plan section 5.2 TR8 scrub_hold vs T3 scrubClip clamp (F7)",
      "claim": "TR8's pre-registered bar cannot hold on the lane build. The fixture in-point is 0.5 unless a row says otherwise; TR8 says nothing, yet it does Down 0.4 and demands 'all four reads within one frame of 0.4'. scrubClip clamps to [inPoint, outPoint-1e-4], so Down 0.4 holds at 0.5 and the bar fails (or the row is silently edited at run time). A pre-registered string that contradicts the plan's own clamp is not a gate.",
      "evidence": "Plan line 605: 'in-point 0.5 unless said'; line 624-625 TR8 'Down 0.4; four playhead reads over 1 s ... all within one frame of 0.4 ... the read 0.5 s after Up is > 0.42'; line 292 scrubClip 'clamps to [inPoint, outPoint - 1e-4]' and F7 line 282. TR7 (lines 621-623) uses 0.6/0.7, which are inside the range; only TR8 is out of range.",
      "severity": "MUST",
      "proposed_change": "Pin TR8 to a position inside the range (e.g. Down 0.62) or state in-point 0.3 for TR8; state the bar in the clamped value. Add the out-of-range press as its own bar (Down 0.4 reads 0.5) so the clamp F7 is also gated."
    },
    {
      "id": "GA-3",
      "target": "plan section 5.2 TR7/TR8 (BF25 live rows) and R10",
      "claim": "The only live BF25 rows drive the NEW test route /api/debug/scrub, which calls scrubClip directly. BF25's causes C-A (LayerStrip writes only the model) and C-C (8 px marker grab on the Clip tab) live in the mouse handlers and in the onScrub wiring, none of which the route touches. These rows pass with the original defect still in the UI and with onScrub left unwired in MainComponent. They also have no RED arm (the route is new) and rely on 'Boris's hand' (R10). A drag row that never goes through the drag.",
      "evidence": "Plan lines 299 (route calls scrubClip on the layer's playing clip), 621-625 (TR7/TR8 GUARD), 772-773 (R10). Source: ui/LayerStrip.cpp:979-988 scrubPlayhead writes only clip->playheadPosition; ui/ClipInspector.cpp:1348-1376 mouseDown grabs the in/out marker when std::abs(mx - inX) < 8 (1361) before scrubbing. U13/U14 are headless widget cases that check the callback fires, not that MainComponent connects it to scrubClip.",
      "severity": "MUST",
      "proposed_change": "Make the debug route inject component-level mouseDown/mouseDrag/mouseUp at pixel positions on the real LayerStrip transport bar and the real ClipInspector timeline (same pattern as the existing onDebugTabDoubleClick), so handler, marker hit-test and MainComponent wiring are all in the loop. Run it on the frozen RED app too: the old UI path exists there, so a true RED arm is available. Add a bar where the press lands within 8 px of an end marker with default in 0 / out 1."
    },
    {
      "id": "GA-4",
      "target": "plan section 5.2 TR13 undo_edit_never_restarts",
      "claim": "TR13 edits ANOTHER cell, so it cannot fail for the risk it is named after. The live clip is never copied over, carryLiveTransportFrom is never exercised, and the row is declared GUARD with no named mutant. It passes with carryLiveTransportFrom deleted. The risky case (Undo/Redo of a SetClipCmd on the playing clip itself, same id) has no live row.",
      "evidence": "Plan lines 636-637: 'A playing ramp; drop a file into ANOTHER cell (/api/debug/drop_files); undo; redo. Bar: t monotonic ... (GUARD.)'. Plan T5 table line 449: the value copy only matters when 'same clip id landing' on a live cell. Mutant MU-T5c (line 505) points only at unit cases U-C3/U-C4.",
      "severity": "MUST",
      "proposed_change": "Drive a per-cell edit on the PLAYING clip (set_clip_param on its effect/param, which pushes a SetClipCmd), then undo and redo it; bar: restartSeq/playhead monotonic across both, and the REST playhead shows no step above tol. Name MU-T5c as this row's tooth (builder must show it RED with carry removed)."
    },
    {
      "id": "GA-5",
      "target": "plan F1 / T1 'every fire plays' and T6 row 1 (test_layer_runtime.cpp:280 playing == false -> true), T9 Pitfall 7",
      "claim": "The expectation flip from 'retrigger keeps its play state' to 'every fire plays' is backed in T6 only by Boris's word 'restart'. His answer 2 was about a clip 'played, replaced, then fire again' (resume vs restart); it says nothing about a paused clip. The other quote the plan leans on ('this is a playing app. there is no stop buttons anywhere') was said about routines and stopping, 2026-09-26. The plan deletes a project rule (CLAUDE.md Transport state, Pitfall 7, hasBeenTriggered) and flips a pinned test on a reading that is Harmony's consequence text, and ships it with Q1 'nothing waits'.",
      "evidence": "Plan lines 151-153 and 513 (T6 row 1 Boris column: '\"restart\"'); backlog 'Boris's answers' item 2 as asked: 'A video you played, replaced, then fire again: continue where it left off (today) or restart?' -> 'restart'. BORIS_DECISIONS.md:365 '(2026-09-26, verbatim): \"this is a playing app. there is no stop buttons anywhere. to end a routine it is replaced or removed ...\"'. Existing test tests/test_layer_runtime.cpp:280-299 pins 'keeps its play state' with playing == false. Plan itself calls it a reversal of a rule 'someone once asked for' (R2, line 750).",
      "severity": "MUST",
      "proposed_change": "Split the lane's rule: restart POSITION on every fire (backed by 'restart'), but keep the play/pause state of a re-fired clip until Boris answers Q1. Leave test_layer_runtime.cpp:280 asserting playing == false and change only the playhead/restartSeq expectation. Delete hasBeenTriggered / Pitfall 7 only after a verbatim answer. If the plan keeps F1, the T6 'Boris' cell must say 'Harmony default, Q1 unanswered' and Harmony must sign it."
    },
    {
      "id": "GA-6",
      "target": "plan T1 restartSeq / applyRestart ('differs from the player's last seen value') with T5 value copies, R3",
      "claim": "restartSeq is compared for inequality against a per-player 'seen' value, but the plan lets a Clip value copy rewind restartSeq on any NOT-live cell (carryLiveTransportFrom is only applied to live cells), while the player persists per clip id. A copy back to an old value that the next fire bumps onto exactly the player's last seen value produces a fire that does not restart, silently. R3 admits take restore, Duplicate Deck and composition load were not read. The fallback 'ignore a counter that went DOWN' does not cover down-then-up-to-equal.",
      "evidence": "Source: render/Renderer.cpp:1635 players are looked up by clip->id (persistent across fires; syncMedia returns 0 only when none). Plan lines 187-190 (restartSeen_ compared for difference), 449 and 493 (U-C4 checks only the same-id live edit), 752-755 (R3). Example: clip fired to seq 5 (player seen 5), an older SetClipCmd state with seq 4 is copied onto the parked cell, next fire gives 5 == seen, so no restart.",
      "severity": "MUST",
      "proposed_change": "Make the counter impossible to rewind or collide: draw restartSeq from one composition-wide monotonic counter (not per-clip fetchAdd), or exclude restartSeq from every Clip value copy (SetClipCmd, SwapClipsCmd, take restore, Duplicate Deck) and not only for live cells. Add U-C4b: Undo of an edit onto an idle cell, then a fire, must restart; plus a randomized fire/undo/redo walk asserting each fire changes (player-seen != seq)."
    },
    {
      "id": "GA-7",
      "target": "plan lints L1/L2 and mutants MU-T5b, T5 definition of 'live' (F13)",
      "claim": "The lints pin spelling while the behaviour is free. L2 bans only the strings setRuntime( (except an empty snapshot), '->playing =', 'playheadPosition =' and the NAME TriggerClipCmd in core/*Commands.h. The same files already write the layer tuple through updateRuntime( (cancelPendingInto, clearTupleForRow), which L2 allows, and F13 defines the QUEUED clip as live, so AddDeck/InsertDeck undo still cancels a queued trigger while the plan marks those commands 'unchanged'. MU-T5b is claimed to be caught by 'U-C2 and lint L2', but a ClearLayerClipsCmd that calls clearTupleForRow passes L2. L1 pins seekTo( counts in MainComponent.cpp only; a message-thread call of the new restart( is not pinned although the plan says GL thread only.",
      "evidence": "Plan lines 482-484 (L2), 229-230 (L1), 505 (MU-T5b), 427-430 (F13 live includes 'the queued clip'), 456-458 (AddDeck/RemoveDeck 'unchanged'). Source: core/DeckCommands.h:195-200 cancelPendingInto uses layer.updateRuntime and is called at 727, 854, 946 inside undo/execute; :272-274 clearTupleForRow uses updateRuntime.",
      "severity": "SHOULD",
      "proposed_change": "Replace the text lint by a behavioural table test over EVERY Command class listed in E19: build a show with an active, a previous (mid-fade) and a pending clip, run execute/undo/redo, assert the full tuple plus playing/playhead/restartSeq of those clips are bit-identical (excluding retire/skip rules stated in T5). Make L2 additionally ban updateRuntime(, clearTupleForRow, triggerClip(, clearActiveClip( in core/*Commands.h, and decide in the plan whether cancelPendingInto on a queued trigger is allowed (then amend F13) or removed. Add a lint that restart( is called only from render/."
    },
    {
      "id": "GA-8",
      "target": "plan F18 / T5 InsertDeckCmd row (Load Deck that added layers)",
      "claim": "When anything is live, Undo of a Load Deck keeps ALL layers the deck added, including idle ones nothing plays on. Boris's rule leaves untouched only what is live and says 'It changes anything else'. The plan leaves non-live layers behind for an implementation reason (rows == layers) and TR12 gates only the live path. There is no row or bar for the idle added layers staying, and Redo of this half-undone step is explained only by prose.",
      "evidence": "Plan lines 455 and 464-466 (F18: 'deck retired, ALL added layers stay'; runner-up rejected because 'erasing a layer erases that row in the retired deck too'). Boris answer a (backlog, 2026-10-03 14:55:32): \"Let's not allow control Z to change anything that is live in the layer strip. It changes anything else\". Answer A as asked offered the exception 'if the loaded deck added layers ... Cmd+Z takes those layers and their clips away' and he answered with the unqualified rule, i.e. the exception is gone for live clips only.",
      "severity": "SHOULD",
      "proposed_change": "Either gate the divergence explicitly (bar: after Undo with one playing added layer, the idle added layers are still present, and put it to Boris as Q7 with his answer-a sentence) or implement erasing idle added layers by id while keeping the live ones, with the retired deck's row snapshot taken per layer id. Add a TR12 variant with a mix of playing and idle added layers."
    },
    {
      "id": "GA-9",
      "target": "plan section 5.4 visual work gate bars B1, B3, B7 and states V6, V9",
      "claim": "Several bars cannot be measured from the widget dump the plan proposes. The row labels in this inspector are PAINTED with g.drawText, not Label widgets, so a dump of widget text cannot assert 'label text is exactly BPM and Beats' (B3) or 'no label or painted string changes' (B7). B1 'bounds inside the inspector' is satisfied by content that sits below the fold, because the Clip tab is the viewed component of a vertical Viewport (two new rows push Beats down at 1280x720). V6 ('length not known') has no deterministic driver (the plan names no fixture), and the gate has no BLOCKED rule, so it can be skipped silently.",
      "evidence": "Source: ui/ClipInspector.cpp:542 g.drawText(\"Speed\", ...) and :546 g.drawText(bpmSyncMode ? \"Beats\" : \"Duration\", ...); the layout comments at 638 and 657 say 'label (painted)'. ui/InspectorPanel.cpp:29-31 clipViewport_.setViewedComponent(&clipInspector_) with vertical scroll. Plan lines 652-657 (B1, B3, B7), 649-651 (V6, V9), 670 (pass rule). Also reverseBtn_ sits in the Speed row (ClipInspector.cpp:642) and the plan hides 'the Speed row' in BPM Sync without saying where Reverse goes, which also bears on B2 (same bounds as the Speed row's pair).",
      "severity": "SHOULD",
      "proposed_change": "Make the two row labels real Label components (or have the dump include a paint-recorder of drawText strings) so B3/B7 are measurable; define B1 against Viewport::getViewArea() at 1280x720 (or require no vertical scroll needed to reach both rows); give V6 a fixture (a clip whose file is missing / still opening) and add 'V-state not capturable = BLOCKED, gate fails'. State where Reverse goes in BPM Sync."
    },
    {
      "id": "GA-10",
      "target": "plan section 5.2 pre-registered strings TR2, TR3, TR6 and the 'every path' table",
      "claim": "Pre-registered RED predictions contradict the plan's own facts, TR6's tooth equals the noise, and the 'one rule, every path' claim has live rows for only some paths. TR2 and TR3 predict RED t of about 7.3 s and about 7 s, but the plan's E6 and TR4 say a first fire on the pre-lane app starts at file position 0, so the RED arm reads about 2.3 s (the row still fails, but the 'fails by ~2 s' string is wrong, which the 'RED that is GREEN is a STOP' rule makes into a misread risk). TR6's second half ('a drop within 100 ms after the downbeat') is read by 50 ms polling plus a 16.7 ms GL frame, a 10.7 ms analysis hop and HTTP jitter, with no pre-registered downbeat reference or definition of 'drop'. Tol 0.25 s is described as 'probe-boxes' resumeTol class' while the file says 0.5. A clause lets any row whose teeth are below 4x noise be downgraded to INFO after the fact. No live row exists for the Autopilot advance, a routine/take replay, a key/MIDI/OSC binding or a column under Quantize, although the table claims each restarts.",
      "evidence": "Plan lines 609-612 (TR2/TR3 RED predictions), 613-614 (TR4 RED 'code < 30'), 618-620 (TR6), 606-607 (tol 0.25, 4x-noise downgrade), 202-215 (every-path table), E6 plan lines 54-58 ('a first fire starts at file position 0'). .harmony/probe-boxes.json:25 \"resumeTol\": 0.5, \"firstMax\": 0.5.",
      "severity": "SHOULD",
      "proposed_change": "Correct TR2/TR3 RED strings to the value derived from E6 (about 2.3 s). Pre-register TR6's downbeat reference (the REST totalBeatCount edge) and the drop threshold, and widen the post-downbeat window to at least 4x the measured poll+frame noise or move the bar to a pure ordering check (no drop before, a drop at or after). Remove the post-hoc INFO downgrade for rows that carry the lane (a downgrade needs Harmony's written waiver). Add one live row per remaining entry in the path table (Autopilot advance, a routine replay, a binding, a column fire under Quantize) or cut those lines from the table's 'restarts' claim."
    }
  ],
  "strongest_point": "GA-1. The gate that is supposed to show the new tests fail on the base and pass on the head is `ctest -R \"layer_runtime|clip_transport_sync|show_model|undo_commands|...\"`. These are file-name tokens, and ctest matches Catch2 test-case names. T6h, the TriggerClipCmd cases and the layer-runtime cases carry no such tokens. There is no --no-tests=error, so the command can report 100% passed over a handful of cases or none. The repo recorded this exact failure in ruling-bf6.md:563-565, and the plan repeats it for G-U1, G-U2 and G-U3, which carry U1-U20 and U-C1..U-C13. If it ships, every later claim that the units are RED on the base is unverified.",
  "citations_rechecked": true
}
```

## seat: scope (MINIMALIST)

```json
{
  "seat": "scope (MINIMALIST)",
  "attacks": [
    {
      "id": "SC-1",
      "target": "Section 4 stage S4a (T4 model + render + REST, no widget) and T6 'complete list'",
      "claim": "S4a deletes beatDivision and videoBeats from Clip, but its file list holds only model/Clip.h/.cpp, render/Renderer.h/.cpp and api/ApiServer.cpp. ClipInspector and four test files still use both fields until S4b. The tree does not build at the end of S4a, so a stage that must end with ctest green cannot. T6 calls itself the 'complete list; no other expectation changes' yet omits the tests that assert the removed fields.",
      "evidence": "Plan lines 334-335 ('beatDivision and videoBeats leave the struct') and 560-561 (S4a file list). At 34179a2, src/ui/ClipInspector.cpp:219-275 (the combo and slider write clip_->beatDivision / videoBeats), :1051 (paint key), :1265-1267 (bar lines) and :1436-1446 (refresh) all read or write them. src/ui/ClipInspector.h:72 holds the PaintKey field. tests/test_clip_inspector_paint_key.cpp:62, tests/test_composition.cpp:224-225, :274-275, :548-549 and tests/test_undo_commands.cpp:116, :344-345 reference them. None of these appear in T6 (plan lines 509-526).",
      "severity": "MUST",
      "proposed_change": "Reorder. Either S4a keeps both fields as deprecated, read-only legacy members (loaded for conversion, no longer written, no longer read by the renderer) until S4b deletes them with the widgets. Or fold S4a and S4b into one stage that ends at the visual gate. Add the six test sites above to T6 with their new expectations. Lost: nothing, only the stage boundary moves. The cost is one more member for one stage."
    },
    {
      "id": "SC-2",
      "target": "T3 (BF25): scrubHeld, the pushIntent / writeBack hold, ScrubPhase, scrubClip, POST /api/debug/scrub, U11, U12, TR7, TR8, MU-T3b, the TSan re-run",
      "claim": "This is wider than what Boris said and it adds a new live-failure mode. Boris asked to 'keep playing at the same speed, but play from wherever I drop'. The plan also pauses the clip while the mouse is held (F6, its own Q2). That needs a new shared runtime field, a changed intent protocol, and a hold that can stick and freeze a live clip. The plan admits this as R7. The core fix is much smaller: the layer-strip bar writes only the model and never seeks (the cause the plan calls C-A), so LayerStrip should call the same seek the Clip tab already uses.",
      "evidence": "src/ui/LayerStrip.cpp:979-989 (scrubPlayhead writes only clip->playheadPosition). src/MainComponent.cpp:1464-1477 (onCuepointJump already seeks the player or sequence). Plan lines 287-299 (scrubHeld, pushIntent change, scrubClip, new debug route), 310-312 (U11, U12) and 765-767 (R7: 'The scrub hold can stick ... and freeze a live clip').",
      "severity": "SHOULD",
      "proposed_change": "Ship C-A, C-B and C-C only. Give LayerStrip an onScrub callback that reuses the existing seek path through one scrubClip function. Clamp to in..out (F7). Grab a marker only on its drawn tab. Drop scrubHeld, the pushIntent / writeBack changes, the debug route, U11, U12, TR7, TR8 and the extra TSan case. Lost: the picture does not hold still under a resting hand. The Clip tab does not hold today either, and Boris did not ask for it. If Boris answers Q2 with 'hold', add it later in one small stage."
    },
    {
      "id": "SC-3",
      "target": "Section 5.2 rows TR1, TR7, TR8, TR13 (GUARD rows) and G-U3 (every MU-* mutant)",
      "claim": "Four of the 13 live rows are GUARDs that pass on the RED arm by design, so they cannot fail on the old app. TR7 and TR8 drive a route that this lane invents (scrubClip). That route bypasses the LayerStrip and ClipInspector mouse code, which is where bug C-A lives. They prove the harness, not the gesture, and U14 plus U10 already cover the callback. Each row also carries the 5-run, two-arm flake rule, and about 25 mutants are applied one by one.",
      "evidence": "Plan lines 586-588 (GUARD rows are INFO for the RED arm), 621-625 (TR7 and TR8 'GUARD for the RED arm: the route is new'), 772-773 (R10: 'weaker than a frozen-app RED'), 596-597 (G-U3 each MU-*), 641 (flake rule). The route is POST /api/debug/scrub, defined at plan line 299.",
      "severity": "SHOULD",
      "proposed_change": "Cut TR1, TR7, TR8 and TR13 (TR13 is already covered by U-C3, U-C4 and TR12). Keep the rows with real RED teeth: TR2, TR3, TR4, TR5, TR6, TR9, TR11 and TR12. Set EXPECTED_ROWS to 9. Limit G-U3 to the mutants for the three lane-defining claims (the tail bump MU-T1a, the before-intent ordering MU-T1b, no-tuple-write MU-T5b) plus whichever Harmony picks. Lost: the live check that a drop plays on. Boris's hand (section 7, steps 5-6) is the only real proof of BF25 anyway, as R10 says."
    },
    {
      "id": "SC-4",
      "target": "T1 F2: a re-fire of the active clip is queued under Quantize (Layer::triggerClip queue test, U3, MU-T1c, TR6, R4)",
      "claim": "F2 gives a new meaning to the layer's pending slot, which can now hold the active ref. The plan admits it has not read the readers that assume pending != active. It also changes the release timing that the bars lane (bf7) is rewriting. BF11 only says 'restart its videos'. The plan needs a locked-tempo driver for TR6, and the plan's own R4 hazards (pads, releaseMomentary, ClipCell queued mark) are untested until after the build.",
      "evidence": "Plan lines 166-168 (F2), 185 ('drop && ref != r.activeRef() from the queue test'), 573-574 (4.2: the bars lane's value list 'applies to every fire'), 618-620 (TR6) and 756-758 (R4). src/model/Layer.h:424 (the current test, 'if (snapEnabled && ref != r.activeRef())').",
      "severity": "SHOULD",
      "proposed_change": "In S2, keep E4: a re-fire of the active clip restarts at once, even under Quantize. This restarts the same clip each time and fixes BF11 and the replaced-then-fired case. A re-fire is then not queued, and R4 and TR6 go. Put the queued re-fire on the ledger for the bars lane, which owns the release edges. Lost: a column re-fire under Quantize restarts the already-playing layers at once, while the other layers still switch on the bar. That is a mild inconsistency, and the plan itself weighs it in F2. If the chair reads Boris's Quantize sentence as covering re-fires, keep F2 and move it to its own stage after the bars lane."
    },
    {
      "id": "SC-5",
      "target": "T1 F3 consume rule: 'a fresh player has restartSeen 0, so a clip fired before its player opened starts at its start'",
      "claim": "Any reopened player under a clip that was already fired restarts it mid-show. Players are recreated on media reconnect, replace-content and undo-redo re-attach. A new player has seen = 0 and the clip's restartSeq is above 0. U-C4 only checks that the clip's counter is unchanged, not what the player saw, so a restart caused by a reopen on an Undo or Redo step is not caught. That is the exact thing BF31 forbids.",
      "evidence": "src/render/Renderer.cpp:1377 (videoPlayers_[clipId] = std::move(player) on every open) and :1427 (erase on close). src/MainComponent.cpp:5210-5222 (the hook comment: 'a clip landing back in a cell via undo must reopen media'). Plan lines 189-190 (fresh player seen 0), 495 (U-C4 asserts only the counter) and 752-755 (R3 admits unread paths).",
      "severity": "SHOULD",
      "proposed_change": "Distinguish 'never drawn since the fire' from 'reopened'. Have the opener (message thread, after open) store the player's restartSeen from the clip's current restartSeq. Also give first-fire an explicit seek to the in-point at open (TR4 already tests this) so the fresh-player case needs no stale counter. Or add a U-C4 variant that reopens the player during an Undo and asserts no restart. Cheapest check: one unit case with a fake player constructed under a clip whose restartSeq is 3."
    },
    {
      "id": "SC-6",
      "target": "T4 THE CHANGE: removals and extras beyond 'a BPM box, a Beats box, /2 and x2'",
      "claim": "The plan removes an inert combo and rewrites bar painting, which Boris did not ask for. It also unilaterally voids two ruled bf7 relabels and widens the REST surface that the sync-dial lane is editing at the same time. Each of these adds layout shifts to the visual gate and merge conflicts with other lanes.",
      "evidence": "Plan lines 379-382 (remove the inert 'Restart / Continue / Relative' combo; change the bar's beat lines), 386-389 (set_clip_param keys transportMode / clipBpm / beats; /api/composition adds seven fields per clip; a new debug dump route), 574-577 (4.2: bf7's relabels 'are void'). Facts sheet Q6 cites ruling-bf7 AM4 and its relabels at ruling-bf7.md:85 and :274-276. The plan's own collision note is at lines 567-572.",
      "severity": "SHOULD",
      "proposed_change": "Leave the inert Restart/Continue/Relative combo for bf7 or ui-polish (it is inert, so it harms nothing). Keep the bar lines but compute them from the new beats. Cut the REST `beats` key and the composition fields that no probe row reads: keep transportMode and clipBpm. Ask the chair to rule on bf7's relabels instead of voiding them here. Lost: a dead control stays one more lane."
    },
    {
      "id": "SC-7",
      "target": "Section 5.4 VISUAL WORK GATE (V1-V10 at two window sizes, five critic seats, B1-B7)",
      "claim": "The visual gate is out of proportion for two number boxes and a button pair. B1-B7 already prove layout, text, enabled state and consistency mechanically from the dump. Ten states at two sizes plus five critic seats repeats B4 and B5, and the 'logic' seat repeats B4 (Beats = L x BPM / 60).",
      "evidence": "Plan lines 646-670: V1-V10, B1-B7, five seats, 'a MUST is fixed and the whole gate re-run'. B4 and the logic seat's check are the same equation (lines 654 and 665).",
      "severity": "NIT",
      "proposed_change": "Keep B1-B7, but run states V1, V2, V4, V6, V8 and V9 only, and use two critic seats: interaction-logic and visual-design. Drop the UX, graphic-design and logic seats, since B3 and B4 cover them. If five seats is Harmony's standing rule for visual work, keep it and state so. Lost: some extra eyes on a small widget. That is acceptable because the dump numbers are the real check."
    },
    {
      "id": "SC-8",
      "target": "Section 4.1 and S0: bf2 collisions, plus the 'Harmony constraint' that the RED arm is a frozen copy of the pre-merge main app",
      "claim": "The plan depends on bf2 having merged before S0, and S0 is 'no code'. S1 (Undo) does not depend on bf2 or on S0's re-read of FeatureSnapshot.bpm or syncMedia. Waiting on S0 for all five stages serialises work that only S4a depends on.",
      "evidence": "Plan lines 544-550 (S0 first thing after bf2 merged; re-read FeatureSnapshot, Renderer syncMedia, ApiServer), 567-572 (collision list: only the BPM and REST edits touch bf2's files), and 551-554 (S1 file list: core/*Commands.h, model/Clip.h, MainComponent.cpp push sites only).",
      "severity": "NIT",
      "proposed_change": "Run S1 and S3 against 34179a2 plus a rebase, and gate only S4a on S0's re-read of the bpm field and the route table. Lost: nothing, because S0's table is still written once before S4a."
    }
  ],
  "strongest_point": "SC-1: stage S4a removes beatDivision and videoBeats from the Clip struct while src/ui/ClipInspector.cpp (:219-275, :1051, :1265, :1436-1446), ClipInspector.h:72 and six test sites in test_clip_inspector_paint_key.cpp, test_composition.cpp and test_undo_commands.cpp still use them until S4b. The tree cannot build at the end of S4a, and T6's 'complete list' omits those tests. This is checkable by grep at 34179a2. The ONE thing that must NOT be cut is T1's render-thread restart mechanism (restartSeq bumped in applyActivationTail and consumed by applyRestart before pushIntent). The render thread overwrites the model playhead every drawn frame (ClipTransportSync.h:44-48, per the plan's E7), so a model-only reset is invisible. The quantized release and the Autopilot advance run on the GL thread and never reach a handler. Every other cut I propose leaves it intact, and TR2, TR3 and TR4 are its teeth.",
  "citations_rechecked": true
}
```
