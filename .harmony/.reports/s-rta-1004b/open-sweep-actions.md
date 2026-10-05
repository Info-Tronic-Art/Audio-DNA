# Open sweep "actions" and the recording review screen (s-rta-1004b, 2026-10-04)

Read-only sweep. Nothing built, run or launched. Paths: BD = /Users/boriskarpman/projects/RealTimeAudio/.harmony/binding-decisions.md, BL = .../.harmony/boris-feedback-backlog.md, FA = .reports/s-rta-1004b/facts-actions-today.md, FQ = .../facts-quantize.md.
Labels: VERIFIED = I read it (file:line). INFERRED = my reading, not his word. Boris is quoted only from BD / BL; the text after "->" in BD is Harmony's and is never quoted as his.
I read his reference picture (VERIFIED): five stacked grey rows on a fine grid, one red curve per row with small round keyframe points; curved rises and straight drops; no names, no audio, no in/out marks in it (his words say where those go: BD:1062-1066, BL:871).

What is already SETTLED (so not listed below; VERIFIED in BD): actions replace routines (BD:922-924); an action loops in time while its clip plays (139 a, BD:1017); a held slider wins, then the action catches up (140, BD:1018); actions are made only in the review screen (141 a); a layer's actions can trigger clips, a composition action can fire clips (142); a layer's action runs on across clip changes (143 a); clip actions = sliders and buttons of the clip tab incl. effects; layer actions = clips on its layer + layer tab + layer effects; global = clips + global tab + all layers, layer actions turned off while it is on, ONE transition slider both ways (BD:1044-1055); "ignore actions" on every slider and button (BD:1056-1059); review screen layout from the picture (BD:1062-1066); flow "pick a stretch ... sliders or trigger buttons ... In and Out ... a button to save ... name box ... hierarchy clip, layer, Global" (BD:1028-1036); Quantize only in the review screen, for recorded button pushes (BD:968-978).

Ranking = how soon a wrong guess hurts him on stage or leaves the review screen empty. 16 kept. Four more found are at the end ("found, not kept").

======================================================================
## 1. What a take records: what he does with his hands must end up in the review screen
Undecided: which of his hand moves a recording keeps, because today a take misses most of them.
Fact (VERIFIED, FA Q2c + its re-read rows 6, 7): a take today records clip fires, deck switches, play/pause, tempo, audio transport and key/MIDI/OSC/REST writes; it records NOT a slider moved with the mouse, NOT the layer strip's B and S buttons, NOT the strip's speed slider, NOT a preset load, NOT a transform or dry/wet move.
 A (default) Everything he touches is recorded, mouse included; a preset load shows as the sliders it moved.
 B Everything he touches is recorded; a preset load shows as ONE named press ("Preset 2") and the action plays by loading it again.
 C As today: only what keys, MIDI and OSC do; the mouse is not recorded.
His words lean to A (INFERRED from VERIFIED words): "Actions will be recorded and displayed" and "they are just automations for any parameters within the clip or its effects" (BD:984-985, BL:728); "all of the parameter changes are displayed" (BD:1033-1034).
Wrong guess: with C he plays a show with the mouse, opens the review screen and finds it nearly empty, and believes the recorder is broken; with B a preset can silently re-load over sliders the action also moves.

## 2. Global action comes on: what exactly goes quiet
Undecided: his "the layer action is turned off" does not say whether that is every layer's action, only those fighting the global one, or clip actions too.
 A (default) Every layer's actions go off, all layers; clip actions keep running (nothing in his words touches them).
 B Layer AND clip actions go off everywhere while the global runs; only the global is heard.
 C Only the layer actions that move the same controls as the global one go off; the rest keep playing under it.
Lean to A: "when the global is triggered the layer action is turned off" (BD:1050-1051, BL:850) against "as the global and layer actions do some similar things" (BD:1051-1052), which could favour C. Harmony's reading R105 (BL:855-860) is A = Harmony's, not his word.
Wrong guess: with A when he meant C, layer actions that had nothing to do with the global go silent mid-show and the picture goes flat; with C when he meant A, two actions fight over one slider on stage.

## 3. Global action goes off: what comes back, and how
Undecided: after the global stops, do the layers' actions return on their own, where in time, and in what state.
 A (default) Only the layer actions that were on before come back on, in time with the beat as if they had never stopped; the sliders glide back over the transition time.
 B They come back but restart from their beginning on the next "1".
 C They stay off; he switches them on again by hand.
Lean to A: "this same slider sets the time when going back to layer actions from global" (BD:1052-1054) says they go back by themselves (so not C); 143 a "runs on with the beat across clip changes" (BD:1020).
Wrong guess: B puts a layer action off the beat against the music; C leaves him in the global look after it ended, with layer actions dead until he notices.

## 4. The "ignore actions" switch: where it sits and how he sees it is on (and whether the show remembers)
Undecided: its place on every slider and button, how it shows, and whether it is saved with the show.
Fact (VERIFIED): every slider is a ResettableSlider whose RIGHT-CLICK resets it (CLAUDE.md "UI Patterns"), and the layer strip is full: 242 of 250 px used (FA Q6, row 20 re-read). So a right-click menu cannot be the way on sliders.
 A (default) A tiny lamp-button beside every slider and button, lit in one colour when on; click toggles.
 B A modifier-click on the control (no extra button) plus a coloured ring or tint on the control while on.
 C A switch per tab (clip / layer / global header) that mutes all actions of that tab, plus A on single controls.
Saved with the show? A (default) No: a show always opens with every control listening; he sets ignores live. B Yes: it opens as left.
Lean (INFERRED): "turn off action control of something during a show if they want to keep the action but need to cancel something live" (BD:1056-1059, BL:851) is a LIVE cancel, so not saved; his words never say "save".
Wrong guess: saved + invisible = next week a slider ignores its action and he cannot see why; not saved = he must re-ignore after every opening of the show.

## 5. Review screen: which rows appear and how a row is picked
Undecided: rows for everything recorded, only what moved inside his stretch, or every control of the show.
 A (default) One row per control the take recorded a change on, grouped Global / Layer / Clip; he ticks a row, or a group header to take its whole group.
 B Only rows that moved between In and Out are shown; all start ticked; he unticks.
 C Every slider and button of the show gets a row, empty ones too.
Lean to A: "The sliders and trigger buttons will each have their own row, and I could select them" (BD:1030-1031, BL:794); "pick a stretch of the take and the sliders or trigger buttons to include" (same). Earlier rule that only changes are recorded: BD:404-406 (2026-09-05).
Wrong guess: C buries him under hundreds of empty rows; B hides the one row he wanted to add to the action by hand.

## 6. One saved action that mixes clip, layer and global rows
Undecided: his stretch will typically hold clip fires (a layer thing) AND slider moves (a clip thing); does one saved action hold both.
Fact (VERIFIED, FA Q1c): the old slicer cut ONE routine out of any rows; no owner was stored (FA Q1a, "no owner clip/layer").
 A (default) The app splits his pick into one action per owner (e.g. "Layer 2: fires" + "Clip Tunnel: sliders") and shows the list before saving; each lands on its own owner's buttons.
 B One action may mix; it sits at the highest level it touches and plays all its parts together.
 C The save refuses a mixed pick: "pick rows of one owner".
Lean to a single owner (INFERRED): the name box "will show me exactly the details of the action that are needed to know whether it is global layer or clip" (BD:1031-1032, BL:794); buttons are per clip, per layer, per composition (BD:980-986).
Wrong guess: B hides what is inside an action behind one button; C fails on the most common stretch of a recorded show; A makes two buttons he must switch on together unless the app links them.

## 7. What he sees when his hand lets go: "catches up", and where a slider rests when an action ends
Undecided: the look of "after I let go it catches up" (BD:1018), and the end case.
 A (default) The slider glides to where the action is by now, in about one beat; while he holds, a small marker shows where the action is.
 B It jumps there at once.
 C It glides over the transition slider's time (the only time knob he has given).
Also undecided in this item: when an action is switched off, or its clip stops, the slider stays where the action left it (A, default; his hold-at-the-end rule for replays, BD:503-505: "keep playing at with the current parameters"), goes back to what it was before (B), or to the control's default (C).
Wrong guess: B makes a visible jump on stage each time he lets go of a slider; C ties a slider glide to a time knob meant for global-to-layer changes.

## 8. Where in the bar an action starts when its clip is fired between two "1"s
Undecided: "play when the clip plays in time with the clip" (BD:982-984) does not say how an action that is several beats long lines up when the clip is fired off the beat.
 A (default) On the next "1", from the action's beginning (same as a BPM-synced clip: BD:945, "123 b").
 B At once, from the action's beginning.
 C At once, but at the point of the action that matches the beat now (bar 3 of 4 starts mid-action).
Lean to A (INFERRED from BD:945 + BD:982-984). Note: a clip in Timeline mode has no "1"; whether A still waits for one for such a clip is the same question.
Wrong guess: B leaves the action half a bar off the music for its whole loop; A makes an action wait up to a bar after he pressed.

## 9. The transition slider: where it is, its range, and what it does to a clip trigger
Undecided: his words give "a small transition slider" (BD:1051-1054) and nothing else; a clip trigger cannot be blended.
 Place: A (default) in the global tab beside the global actions' buttons; B in the top bar; C on each global action.
 Range: A (default) seconds, 0 to 4 (like the layer's fade slider, 0 to 4, VERIFIED FA Q5); B beats, 0 to 4, so it follows the tempo.
 A clip trigger inside the switch: A (default) it happens at once (a cut), sliders glide; B the layer's own fade time applies as always; C the trigger waits until the sliders have finished moving.
No word of his settles these (INFERRED: "how fast to transition" suggests a time, not a unit).
Wrong guess: C delays the fired clip, so a global "drop" fires late; A a cut on a drop he wanted faded.

## 10. Two actions of one clip move the same slider
Undecided: both on, both moving "Opacity" (or any slider).
 A (default) The one switched on later wins; when it goes off the earlier one is heard again, and the loser's button dims.
 B They add together.
 C Switching on the second switches the first off.
No word of his. Today's engine rule for stacked routines is "the gesture that BEGAN later wins" (VERIFIED, FA Q1f) = A.
Wrong guess: B pushes a slider past where either action meant; C turns off an action he thought was still running.

## 11. A clip's actions when the clip is copied, moved, or its effect changes
Undecided: whose property is an action, and what silently follows what.
Fact (VERIFIED, FA Q4c): today a routine finds its target by POSITION; after a clip is moved or effects re-ordered, it silently drives whatever sits at the old place. His rule BF41 (Harmony's wording of his rule, BL:424): the app shows no message except unsaved recordings, so nothing can announce a change.
 Clip copied to another cell / deck: A (default) the copy keeps the clip's actions (own on/off); B the copy has none.
 Clip moved: A (default) its actions go with it (they belong to the clip); B they stay in the old cell.
 Effect removed or re-ordered: A (default) the action follows the effect it was recorded on; rows of a removed effect do nothing and show greyed in the clip's list; B by position, as today.
Lean (INFERRED): "a little area above each clip where there will be toggle buttons for each action that was recorded for that clip" (BD:980-981).
Wrong guess: by position, a moved clip's action drives another clip's sliders on stage with no sign.

## 12. How many on/off buttons fit above a clip; how an action is named, listed, re-ordered, deleted
Undecided: a clip cell is 90 px wide (VERIFIED FA Q6) and today 22 px rows sit above each column; his "little area above each clip" has no count and no name rules.
 A (default) A row of small numbered buttons above the clip, about four fit, more behind a "+"; the name in the tooltip; right-click = rename, move left / right, delete (a small window asks first, as "Save over" does: BD:993).
 B One "Actions" button above the clip opens a list with a switch per action; rename, drag to reorder and delete live in the list.
 C Named buttons; the cell grows taller as actions are added.
Naming: A (default) a name box opens holding "Action 1" that he can change (the rule he gave for presets: "open a name box but with default name look x that can easily be changed", BD:931).
Lean to A: "toggle buttons for each action" (BD:981-983).
Wrong guess: C pushes the deck grid out of the screen in a show with many actions; B hides what is switched on.

## 13. Where composition-level actions live, and can a key or MIDI pad fire any action button
Undecided: "We will also find a place to have composition level actions" (BD:983, BL:728) and "when the global is triggered" (BD:1050) name no place and no way to trigger.
 Place: A (default) in the 250 px corner above the layer strips where "ROUTINES" is painted today (VERIFIED FA Q3/Q6; the pads go, Harmony's reading R94); B in the global tab of the inspector; C in the top bar (already crowded by the new tempo row).
 Hands: A (default) every on/off button can be bound to a key or MIDI pad in learn mode, like clips; B mouse only.
Wrong guess: B in a live show where all else is on keys and pads; C overfills the top row he has just redesigned.

## 14. The grid sizes and the Quantize control in the review screen
Undecided: the choices he gets and what Quantize touches. His words: "a grid that can be adjusted, smaller and larger, plus quantize control" (BD:1062-1066, BL:871); Quantize "will clip to the exact bar or beat" for "button pushes" (BD:971-973, BL:727).
 Choices: A (default) one musical list for both: 1 bar, 1/2, 1/4, 1/8, 1/16 (his earlier Quantize list BF20, named in BD:975-977 as replaced as a live control); B a zoom slider that makes the grid finer or coarser plus a separate Quantize list; C quarter beats only (his 2026-09-05 "snap too quarter beats", BD:320-322).
 Touches: A (default) only recorded button presses of the ticked rows, undoable; B presses and slider keyframes; C presses with a strength slider.
 Also undecided: In and Out free, or snapped to the grid, so a looping action is a whole number of beats.
Wrong guess: B moves slider points he wanted left alone; free In/Out makes a loop drift against the bar.

## 15. Can he move or draw keyframes; what the number on a row means
Undecided: his latest flow lists no editing, his picture shows keyframes with curves, his older words ask for drawing.
Fact (VERIFIED, FA Q4a): no keyframe editor exists anywhere today; the nearest code paints an envelope with no mouse editing.
 A View, pick, Quantize only (what BD:1028-1036 lists).
 B (default) Also drag a keyframe up, down, left and right with snap to the grid, and delete one.
 C B plus draw a new shape by hand (grid or free hand): "draw rather than drag" and "select and move it left and right, that will snap to grid" (BD:397-402, 2026-09-05).
Units on a row: A (default) real units where the app knows them (degrees for rotation; the app stores degrees, he said "radians": BD:974); B always 0 to 1; C the curve is 0 to 1 and a typed number shows the unit. FA Q5: effect sliders carry no unit at all.
Lean to B or C (INFERRED): his old words win unless a newer line says no; a newer line says nothing.
Wrong guess: A leaves him unable to fix one wrong keyframe except by re-recording; C costs a drawing tool he may not want.

## 16. The take's audio in the review screen, and the take after an action is cut
Undecided: how the sound is shown and heard, and what "the audio is gone" means.
His words (VERIFIED): "After it is recorded it does not follow audio, the audio is gone when it has been turned into an action. The only time that an action will be connected to its audio is when the user is reviewing" (BD:962-965, BL:725). Against: "never delete", the app never deletes stored audio (BD:521); one audio, many take edits (BD:452-458); audio locked with the knob changes in replay (BD:320-322).
 A (default) The audio shows as a wave across the top and plays locked with the playhead (and loops In to Out on request); after a cut, the take and its audio stay on disk and he can cut more actions; the action carries no sound.
 B The take is marked used or removed after the cut; the audio file stays (never deleted).
 C The audio is removed with the take.
Wrong guess: C breaks his "never delete" rule and a 4-hour set's audio; B makes him re-record to cut a second action.

======================================================================
## Found, not kept in the 16 (next in rank)
 F1 What an action may press beyond sliders and clip fires: tempo, deck switch, stop / play, loading a preset (the presets lane asks this, ruling-looks-answers2.md SF-10; the old slicer dropped tempo, audio and deck lanes: FA Q1c).
 F2 His old routines in the saved show and the 8 pads: converted into actions or dropped; his only word is for old presets, "delete them. this is a new build" (BD:933), not for routines.
 F3 Whether the row's stop ends a take that is replaying, and whether a stop or a layer's X is recorded in a take (parked by ruling-nudge-row2.md "PARKED FOR THE ACTIONS LANE").
 F4 Whether a clip fired by a layer or global action starts that clip's own actions (his R83: "When the clip starts playing, if its actions are turned on, they will play", BD:984-985, suggests yes).

## Gaps in the sources (VERIFIED)
 The reference picture shows no row names, audio or in/out marks, so none of 5, 14, 15, 16 can be read off it. No word of his names a count, a range, a colour or a key for any item above; every default A is Harmony-neutral unless a quote is given.

STATUS: DONE. Items listed: 16 (+4 found, not kept).
