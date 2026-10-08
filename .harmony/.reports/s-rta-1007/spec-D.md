# SPEC D -- Actions in a show (s-rta-1007): the paper apply-D.md with the ruling rule-D.md laid over it by merge.py. This file wins over both.

## ITEMS
@@ITEM 179
TITLE: Rows of two layers: one global action; a layer can opt out
STATUS: ANSWERED in words that name no letter; option A is INFERRED (assumption D-8)
HIS: L73, L55
RULE: Rows of two or more layers that are ticked together are saved as ONE global action under one button (option A): it holds both faders, and like every global action it holds back the layers' own actions while it is on. His words add: "a layer can have a global action bypass in the same way that I can have column trigger bypass" (L73). A layer whose switch for this is on is left alone by every global action: no global action moves that layer's controls or fires a clip on it, and that layer's own actions and its clips' actions are not held back -- they play on. So layers 3 and 4 of the example keep their own actions by having this switch on. The action save window lists what the pick becomes before it saves (R133 e). Whether this switch is the same one as the "ignore actions" toggle of L7: item G5 and assumption D-2.
CHANGED: The page offered A or B and he named neither letter. His words ADD the layer's opt-out, which removes the drawback the page gave for A ("layers 3 and 4 included, though they have nothing to do with the two faders"). That A is meant and not B is INFERRED: the bypass has a job only if the pick becomes a global action.
TODAY: Nothing called an action exists in the code (area-actions.md section 1, lead line). A layer has only "Ignore Column Trigger" (area-actions.md 1.30). New work: the global kind of action, the per-layer switch, the hold-back of layers' actions and its exception.
@@END

@@ITEM 180
TITLE: The tempo stop and the layer and global actions
STATUS: DEFAULT (his L1); L72 fits it but can also be read as option B -- assumption D-3
HIS: L1, L72
RULE: Option A in full: on the tempo row's stop a layer's and the global actions stop with the beat and their buttons stay ON; their controls stay where they stood (a fader at 0 stays at 0). When the beat runs again they start from their beginning on the "1", and one that fires clips fires them again. A clip's own actions end with their clip, because the stop takes every clip off (R161). His words "The tempo stop button stops all actions as well as everything else." (L72) say that they stop; they do not say whether their buttons go off. A layer's X follows from A: the layer's own actions stay on and keep running with the beat.
CHANGED: No letter of his. L72 confirms that the tempo stop stops every action. It uses the same words as for the Stop actions button ("stops all actions"), which switches actions off; so option B (the tempo stop switches them off) is a possible reading of L72 and is asked.
TODAY: The top bar's Stop takes every routine off and does nothing else (area-actions.md 1.21); there is no stopped or paused state of the beat on main (area-actions.md 1.34). To change: the tempo row's stop, and actions that stand still with their buttons on.
@@END

@@ITEM 181
TITLE: An action against a signal on one slider
STATUS: DEFAULT (his L1); the time of the glide back is now his (L67)
HIS: L1, L67, L71
RULE: Option A: an action wins over a signal while the action is on, as the hand does while it holds a slider. When the action is switched off, or has played once, the slider glides back to the signal over the Global glide time: "whenever an action is switched off and it goes back, it follows the master action fade back time" (L67). A slider that follows a macro knob counts as a slider with a signal. An action can be made from moves that a signal made in a recording; it then replays those moves as recorded, without the music, "treated like a regular action" (L71).
CHANGED: Option A's last words ("as it does today when your hand lets go of it") are replaced: the glide back takes the Global glide time (L67, L64), not a fixed short time.
TODAY: The order is hand > lane (routine / replay) > signal > rest; the signal comes back over a fixed 120 ms (area-actions.md 1.29). To change: the glide-back time becomes the Global glide.
@@END

@@ITEM 182
TITLE: A layer's or a global action fires cells
STATUS: ANSWERED b
HIS: L74
RULE: Option B in full: a layer's action fires CELLS -- the columns it was recorded with (say 1, 3 and 2), on its own layer, in the deck that is shown, whatever clips sit there by then. Put other clips into those cells, or switch the clip grid to another deck, and the same rhythm plays other pictures. A global action does the same on each layer it fires on. The deck is the one on screen at the moment of each fire, so a deck switch changes what a running action plays from its next fire on (assumption D-10). A fire made by an action follows the rules of a fire by hand on that cell: a clip in BPM mode waits for the "1", a clip that is not in BPM mode plays at once, an empty cell empties the layer (assumption D-10). A column that the shown deck does not have is skipped (assumption D-24). A layer's action pasted onto another layer fires the same columns on the new layer. A clip's OWN actions are not touched by this: they belong to the clip and move with it.
CHANGED: The default A (the action fires the clips it was recorded with, wherever they are) is replaced by B. The page's sentence "an action never fires a clip that was not in the recording" falls away. His words for a fire by hand, which the RULE leans on: "If a layer that is not in BPM mode is triggered, then that plays instantly" (L11); "D layer goes empty when a column is triggered with an empty" (L19). That an action's fire behaves like a hand's is INFERRED and shown to him as D-10; a key or pad on a cell also fires the cell of the deck on screen (topic F, question 197 at its default).
TODAY: A routine's fire goes to the deck that was shown at the press and to a position, never to a clip (area-actions.md 1.24, O17). B is close to that; new: the deck is looked up at each fire.
@@END

@@ITEM 183
TITLE: The glide slider counts seconds, 0 to 4
STATUS: DEFAULT (his L1), and his L64 says the same
HIS: L1, L64
RULE: Option A: the slider counts seconds, from 0 (instant) to 4. His words: "from instant to four seconds max" (L64). It is the slider he calls "the Global glide slider" (L64); its jobs are listed under 213 and R172.
CHANGED: nothing in the letter; the page's reason ("like the F (fade) slider of a layer") is no longer needed: the range is his own word. The slider's job is wider than the page said (it was global over layer only).
TODAY: No such slider exists; a layer's F slider counts 0 to 4 seconds (src/ui/LayerStrip.cpp:478-488, cited by the slice, not re-read).
@@END

@@ITEM 184
TITLE: Pasting an action onto a clip without the effect
STATUS: ANSWERED in words (his note on R162): none of A, B, C as asked
HIS: L75, L68
RULE: The paste goes through and never adds or changes anything on the clip except the action. His words: "if an action is is pasted onto a clip that lacks a parameter or an effect, that specific action is muted, and the small messages is displayed where the user needs to say OK they understand." (L68). Best reading, by the sentence as he wrote it (an action is pasted; that specific action is muted): the pasted action is muted as a whole while the clip lacks anything it moves. It sits on the clip with its button, moves nothing, and shows that it is muted; a small message names what the clip lacks and closes on OK; it plays as soon as the clip has what was missing (he adds that effect). The other reading -- only the part without a target is muted and the rest of the action plays -- is in assumption D-6, which asks him. The same rule holds for a layer's action pasted onto a layer that lacks a layer effect it moves (INFERRED: L68 names a clip).
CHANGED: None of the letters is his: he wrote "184 read my note on this above" (L75), and the note is L68. Option A (pasted with the parts that fit, the rest left out) is not taken as the main reading, because his sentence mutes "that specific action" and names no parts; it lives on as the other reading in D-6. "whether the app says so: question 209" is answered: a small message with OK (L68). A's "If nothing fits, nothing is pasted" is not his word and is dropped. B (the paste adds the effect) and C (the paste is refused) are not chosen. Mended by this ruling: the paper had the part-only reading as the main one.
TODAY: A routine addresses an effect by its position and would drive whatever sits there (facts-actions-open.md P8). To change: an action finds its effect by what it is, and an action whose target is missing is muted (whole, or in part: D-6).
@@END

@@ITEM 185
TITLE: Save without the ignored controls makes a new action
STATUS: DEFAULT (his L1)
HIS: L1, L72
RULE: Option A in full: in an action's right-click menu, "Save without ignored controls" makes a NEW action that moves only the controls that are not set to ignore, named after the old one with a number. The old action stays as it was. Tested against "We will only allow users to record actions in the recording review screen, not modify during a show." (L72): the entry records nothing and changes no saved action, and it is in the list of R224 a to which he said yes.
CHANGED: nothing
TODAY: No such entry exists; a routine cannot be copied or trimmed (area-actions.md 1.13). facts-actions-open.md P6.
@@END

@@ITEM 193
TITLE: What an action can hold: everything changed from default
STATUS: ANSWERED b, with his added words
HIS: L76, L55, L85, L89, L66
RULE: Option B in full: an action can hold every slider, every on / off button, every drop-down list, Speed, the clip's play, pause and backwards / forwards buttons, and the in and out points: an action can pause, reverse or re-cut its clip in rhythm. His added words: "the action holds all parameters that are changed from it’s defaults, including menu changes and backwards forwards changes" (L76). Rows exist only for controls that were changed from their default during the recording (topic E), and he picks which rows go into the action (L55: "After use picks rows"). What an action never holds: the tempo row's buttons (R144); a tempo change -- "Actions will not be made with a tempo change." (L89); a deck switch -- "action should not record switching decks" (L85); the cue controls (R131 e; topic B); another action -- "we will not nest an action in an action." (L66); and the controls that steer actions themselves: an action's on / off button and its Loop toggle, an ignore lamp, a layer's switch against actions, Stop actions and the Global glide slider (assumption D-28). A pause that an action presses on its own clip does not hold that action still (assumption D-22).
CHANGED: The default A (no play, pause, backwards, in or out point) is replaced by B. Added by his words: menu changes and backwards / forwards changes are named outright. Completed by this ruling: the list of what an action never holds (the cue controls from the page's R131 e, which he read; an action inside an action from L66; the steering controls as assumption D-28).
TODAY: A recording cannot address clip speed, the layer's F and K sliders, keying, blend, feedback, in / out / cue points or the fit list at all, and does not record a slider moved with the mouse (area-actions.md 1.26, 1.27, H9). All of that is new recorder work. Keying and its slider are being removed by his answer 204 (L111, topic I).
@@END

@@ITEM 194
TITLE: A clip fired by hand on a layer whose action fires clips
STATUS: DEFAULT (his L1)
HIS: L1
RULE: Option A in full: a clip he fires by hand on a layer whose action (or a global action) fires clips plays until the action's next fire; then the action's cell takes the layer again. To keep his clip he switches the action off. Tested against his whole message: nothing says otherwise. A layer whose switch against global actions is on (179) is not fired on by a global action at all.
CHANGED: nothing
TODAY: Not covered by routines in this form (area-actions.md M1).
@@END

@@ITEM 213
TITLE: An action switched off glides back
STATUS: ANSWERED b
HIS: L77, L64, L67
RULE: Option B: when an action is switched off, every control it moved glides back to where it was before the action, over the time the Global glide slider sets -- "from instant to four seconds max" (L64); at 0 it is a jump. The same when a global action goes off, when an action that is not set to loop has played once (L64), and for a slider with a signal, which glides back to the signal (181). His words: "whenever an action is switched off and it goes back, it follows the master action fade back time" (L67). On / off buttons and lists cannot glide: they switch back at once (assumption D-13).
CHANGED: The default A (it jumps back at once) is replaced by B. The page's "transition slider" is the slider he calls "the Global glide slider" (L64): 213 b says the glide back takes the transition slider's time, and L64 says "We will just keep the Global glide slider".
TODAY: A routine that ends leaves its controls where they are; the old "restore" runs at a routine's start, not at its end (area-actions.md 1.14, 1.16; facts-actions-open.md P15). To change: a per-control memory of the value from before any action, and the glide back.
@@END

@@ITEM R133
TITLE: What each kind of action holds; "Global"; a mixed pick is split
STATUS: CORRECTED
HIS: L55, L73
RULE: Clip actions, layer actions and global actions are saved separately. (a) A clip action holds only that clip's controls, its effects included, under one on / off button. (b) A layer action holds the layer tab's controls and the clip fires on that layer (fires of cells: 182 b). (c) A clip fired by a layer action plays its own actions. (d) A global action holds the global tab's controls, any layer's controls, and clip fires on any layer; a clip's own controls never go into it, they are saved as that clip's action. (e) When he picks rows of more than one kind, "the app breaks them down into clean, concise actions that can be saved in the action save window, the final step in creating an action." (L55): the action save window lists the actions the pick becomes, each with its owner and its name, and saving there is the last step. Rows of two or more layers become one global action (179; assumption D-8). Rows of the global tab ticked together with rows of layers become one global action, and in the window he can name each action of the list and leave one out (assumption D-23). The word on screen and in talk is "Global", not "Composition": "composition and global are interchangeable but lets move to global as that is what musicians are more used to." (L55).
CHANGED: Name: the word Composition is replaced by the word Global (his L55); this also gives the tab its name. (e): his "E yes, this is a good idea." confirms it and ADDS that the result is saved in the "action save window", the final step. "One case is yours to pick: question 179" is answered by L73. (a) to (d) stand. Added by this ruling: the pointers to D-8 and D-23, which the RULE did not carry.
TODAY: The tab reads "Composition" (slice J, question 210: InspectorPanel.h:87-90, not re-read). A routine has no owner field (area-actions.md 1.3).
@@END

@@ITEM R134
TITLE: The action buttons; the clip's row sits below the clip
STATUS: CORRECTED
HIS: L56, L8, L64
RULE: Eight action buttons are shown for a clip, eight for a layer, sixteen for global; each row ends in a "+" that opens the rest. The clip's row: "we will make a small row below the clip, still within the layer for the actions like this screenshot." (L56) -- a slim row under the clip's name, inside the clip's cell and inside its layer's row. The layer's row sits in the layer strip, by his earlier word that nothing in this message changes: "They will be in the layer strip" (binding-decisions.md:982); how it fits there is Harmony's layout (L8). The global row: "We will also find a place to have composition level actions, but they will simply be toggle buttons as well." (binding-decisions.md:983-984) -- laid out by Harmony where it fits in the correct area (L8). A row shows the first eight (or sixteen) in the order they were made; he can drag a button along its row; a new action goes to the end of its row; when one is deleted the buttons after it close up; a key or pad put on an action stays with that action. Each action has its Loop toggle right with its button: "Each action where it is placed, will have a loop toggle right there." (L64).
CHANGED: "Where the three rows sit on screen comes next as pictures" is replaced: the clip's row sits BELOW the clip inside its cell (L56); no pictures are made (L8). His earlier "There will be a little area above each clip" (binding-decisions.md:980) is replaced by "below" for the clip's row only; his earlier words for the layer's row (the layer strip) and for the global row (a place to be found) stand. Added: the Loop toggle beside each action (L64). The order rules stand. The picture (img-08, looked at again for this ruling) shows one clip cell of Audio-DNA (badges "SOURCE" and "3 FX"): the clip's picture, the name "Plasma Burst", under the name one slim full-width bar that reads "Retrigger", and free room below it; the action row is to be a slim row like that bar.
TODAY: A cell is 90 x 96 px; the eight routine pads sit in one row above the COLUMNS, not at a clip (area-actions.md 1.10; facts-actions-open.md P7). The layer strip uses about 242 of 250 px (area-actions.md 1.35). To change: cells get a row under the name; the pad row goes; the layer's action buttons need room in or at the strip.
@@END

@@ITEM R172
TITLE: What an action does when it starts; Loop or play once
STATUS: CORRECTED
HIS: L64, L70
RULE: An action moves only the rows that were ticked when it was made. It starts on a "1". From that moment each of its controls goes to the action's value; where that would be a jump the control glides there, because he wants "a Global fade control on all actions starting/stopping that would create a jump, even with a resync" (L70). That fade is read as the same slider as the glide back, the Global glide: at instant it jumps (one slider or several: assumption D-1; the glide begins on the "1", not before it: assumption D-14; buttons and lists switch at once, and a jump inside the recording stays a jump: assumption D-13; how a glide lands on a value that is itself moving: assumption D-25). A ticked row that stays flat between the In and the Out point holds its control at that one value for as long as the action plays. Controls that are not in the action stay where they are. There is no separate restore switch. Every action has a Loop toggle: "Each action where it is placed, will have a loop toggle right there." (L64). Ticked: the action loops until it is switched off (a clip's action: or until its clip stops). Not ticked: "they will only play once which means they will go back to the position they were at before they played." (L64) -- after one pass its controls glide back to where they were before it, over the Global glide time (L64: "how quickly the values go back to their pre-action positions, from instant to four seconds max"). This holds for clip, layer and global actions alike: "Some actions will loop in a clip, layer, global and some actions will not." (L64). A new action arrives switched off with Loop ticked (assumption D-15). What the button of a play-once action does afterwards, and when it plays again: assumption D-4. Loop un-ticked in the middle of a pass: assumption D-31. A clip that an action fired stays on its layer when the action ends: assumption D-26.
CHANGED: Replaced: "a control that was somewhere else jumps there on the 1" -- it glides where it would jump (L70); that this glide is timed by the same slider as the glide back is Harmony's reading and is asked as D-1. Replaced: "A global action is the exception" -- with one slider there is no exception: "We will just keep the Global glide slider" (L64). Added: the Loop toggle per action, and play once then go back (L64); this replaces his "139 a" (an action always loops). The rest stands.
TODAY: A routine has a loop / once field in its pad menu and a "restore first" with Ease or Jump at its start (area-actions.md 1.2, 1.13, 1.14, 1.16). To change: the toggle moves beside the action; restore goes; the glide is one global time.
@@END

@@ITEM R158
TITLE: One timing rule: every action starts on a "1"
STATUS: REPLACED in one part (it loops only if its Loop toggle is ticked, L64); the rest stands
HIS: L64, L12, L13, L31, L36
RULE: Every action starts from its beginning on a "1"; it loops if its Loop toggle is ticked and plays once if not (L64). Its length in beats follows the tempo (R144). (a) A clip in BPM mode: clip and action start together on the "1". (b) A clip not in BPM mode: the picture starts on his press, its action on the next "1". (c) A layer's or a global action that he switches on, or a clip's action that he switches on while the clip already plays, starts from its beginning on the next "1". (d) Beat stopped: no action plays; his fire starts the beat, that press is the "1", and the actions that are on start with it. (e) Beat paused: every action holds still, and carries on from that place when the tempo plays again: "the clock will continue from where it was paused as well" (L31). (f) A clip he paused himself: its actions stop moving its controls, and when it plays again they are back in step with the beat, not shifted. (g) A clip previewed by its name plays its own actions for the preview, starting on the "1": "a previewed clip (clicking its name) shows the clip with all its actions and it is triggered on the 1" (L36); how the preview does that is topic B's.
CHANGED: "and loops" is replaced by the Loop toggle (L64). Tested against L12 ("When stopped, user’s click on play or any clip is the new 1.") and L13 ("Pause pauses, the beat clock, the clips and everything that it controls with BPM."): (d) and (e) say the same; (e) gains his L31 words on what tempo play does after a pause. (g) is added from L36 as a pointer only. Whether a clip that is not in BPM mode plays on during a pause is topic A's, not decided here.
TODAY: A routine starts on its own quantize line (bar by default) and stalls while the beat clock stands (area-actions.md 1.14, 1.15). facts-actions-open.md P5.
@@END

@@ITEM R144
TITLE: An action is counted in beats and follows the tempo
STATUS: STANDS
HIS: L89
RULE: An action's length is counted in beats and it follows the tempo: a faster tempo plays it faster, so its grid lines sit on the live bars. An action never presses the tempo row's buttons, and no action is made with a tempo change in it: "Actions will not be made with a tempo change." (L89).
CHANGED: nothing; L89 says the same for tempo changes.
TODAY: A routine already plays in routine beats on the tracker's beat clock (area-actions.md 1.15, S4); INFERRED there, never run at two tempos.
@@END

@@ITEM R159
TITLE: An action's length; a warning when not a multiple of 4 bars
STATUS: CORRECTED
HIS: L65, L82
RULE: An action can be any whole number of grid steps long. The grid of the In and the Out point is topic E's: its smallest step is one beat by his "1beat is the smallest" (L82), which topic E reads against his Quantize list in its assumption E-14. An action that is not a whole number of bars still starts on a "1" and drifts against the bars as it loops. The action save window shows the length in bars and beats, and shows a warning when the length is not a multiple of 4 bars: "show a warning in save screen if the action is not a multiple of 4 bars" (L65). The warning does not stop the save (INFERRED from the word "warning"; assumption D-16).
CHANGED: Added: the warning, and its measure is 4 bars (16 beats), not one bar. "does not forbid it" stands. Mended by this ruling: "the smallest step is one beat" stood here as a rule of this topic; it is the grid of the In and the Out point, which topic E rules and asks (E-14).
TODAY: Save Routine takes "From bar" / "To bar" in whole bars and shows no warning (area-actions.md 1.6). facts-actions-open.md P13.
@@END

@@ITEM R129
TITLE: Action off, the hand, a preset under an action, recording an action
STATUS: CORRECTED -- point 4 is not confirmed: he asked back (answer R129-4, assumption D-5)
HIS: L66, L64, L67
RULE: When an action is switched off, or has played once, every control it moved glides back to where it was just before any action took hold of it (213). (1) Another action that is still on moves the same control: that one takes it; it goes back only when the last one is off. (2) A hand move never lasts on a slider that an action is moving: the hand wins while it holds, and on letting go "the slider moves back to the action position based on the global glide back setting I described earlier." (L66) -- a glide over the Global glide time, no jump; when the action goes off the slider goes to the value from before the action. To keep his own value on it he lights its ignore lamp while he holds it (R131). (3) A clip's action that ends because its clip stops or is replaced: the same, back to before, so the clip looks the same the next time it is fired. (4) A preset loaded onto an effect while an action plays: the recommendation is that the preset is kept underneath -- the sliders the action is moving stay with the action and glide to the preset's values when it goes off or has played once; the other sliders take the preset at once (assumption D-5; not his word yet: he asked for the explanation, answer R129-4). An action that is not moving a control at that moment (it is off, or it played once and went back) has no say: the preset loads plainly. A slider that follows a macro knob counts as a slider with a signal (181). Added by him: when a recording is made while an action plays, "we will not nest an action in an action." (L66): the recording keeps the moving parameters themselves; a new action made from them holds plain moves; in the review screen those parameters are slightly differently coloured and the action's name is shown in the hierarchy (how it is shown: topic E).
CHANGED: (2): "when you let go it jumps to the action (158 A)" is replaced by a glide over the Global glide time (L66); this overrides the default 158 A he accepted on 2026-10-05. (4): not confirmed; he asks for the explanation (L66). Added: his idea on recording an action that plays (L66). (1) and (3) stand. Added by this ruling: the clause on an action that has already ended, and "while he holds it" in (2), which follows from R131 b.
TODAY: A routine's own writes are never recorded, and a preset load is not recorded (area-actions.md 1.18, 1.26, T5). facts-actions-open.md P1, P3, S1.
@@END

@@ITEM R130
TITLE: Two actions on one control; the light
STATUS: STANDS
HIS: none
RULE: Two actions of one clip that move the same control can both be on: the one switched on last moves it. The earlier one is overridden on that control, not switched off: it keeps moving its other controls, and its button carries a light that means "something else holds a part of this action". Switch the later one off and the earlier one takes the control again and the light goes out. The same light shows on a layer's action while a global action holds it back (R160); a layer that is set to stay out of global actions is not held back and shows no light (L73). The look of the light is Harmony's layout (L8).
CHANGED: nothing. With 181 at its default A, a signal never holds a part of an action, so the light has two causes only: another action, or a global action.
TODAY: The later GESTURE wins and the earlier one does not take the control back (area-actions.md 1.17, O7). New work.
@@END

@@ITEM R131
TITLE: The ignore lamp beside every slider and button
STATUS: CORRECTED in (c)
HIS: L67, L66, L70
RULE: (a) The lamp beside every slider and every button is a toggle: lit = that control ignores ALL its actions and is his; dark = it follows them. (b) A control whose lamp is lit behaves as if it had no actions: no action moves it, switching an action off does not move it back, and it stays where he put it. Lit while an action is moving the control, the action lets go of it: the control glides back to where it was before the action, over the Global glide time, as when the action is switched off -- "whenever an action is switched off and it goes back, it follows the master action fade back time" (L67); if his hand is on the control at that moment, it stays where his hand leaves it; a slider with a signal plugged in glides back to its signal (181). The other way -- the control stays where it is at that moment -- is the ALT of assumption D-11, which shows him both. (c) Set back to follow while an action is on, the control glides to where the action is by now, as when his hand lets go: "the slider moves back to the action position based on the global glide back setting I described earlier." (L66). (d) A control that ignores is still recorded and still has its row in the review screen. (e) Controls that no action can ever move get no lamp: the tempo row, the cue buttons, and the controls that steer actions themselves (assumption D-28). Where the lamp sits is Harmony's layout (L8); it is one lamp per control, never one per group of controls: "every slider, button, everything needs an ignore actions toggle" (binding-decisions.md:1056). The switch on a LAYER (G5) is a different thing and does not change any lamp.
CHANGED: (c): "the control jumps to where the action is by now (as when your hand lets go, 158 A)" is replaced by a glide: the page tied (c) to the hand, and his word for the hand is now a glide (L66); L70 points the same way. (b): the page did not say what the control does at the moment the lamp is lit; this ruling reads it by his L67 (an action that lets go sends the control back, in the fade back time) and shows it to him as D-11. (a), (d) stand; (e) stands, with the steering controls added as assumption D-28. "comes as pictures" is replaced by L8.
TODAY: No ignore flag exists; the write funnel knows rank, not an ignore flag (area-actions.md T12). facts-actions-open.md P4.
@@END

@@ITEM R132
TITLE: What the show remembers about actions
STATUS: STANDS
HIS: none
RULE: The show remembers which controls are set to ignore, and which actions were switched on, so a show opens with its action buttons as he saved them. A control that an action is moving is saved at the value he had set (the value from before the action), never at the action's moving value. Also remembered, as new parts of the same kind: each action's Loop toggle, each layer's switch against global actions and the Global glide time; in a new show the Global glide time starts at 1 second (assumption D-17; topic F's list of what a show holds says the same for what is remembered).
CHANGED: nothing in the reading; three new things are saved with the show because his words created them (L64, L73, L7).
TODAY: A routine has no on / off state; it lives in the show file's keys "routines" and "routineBank" (area-actions.md 1.3, 1.4).
@@END

@@ITEM R160
TITLE: A global action over the layers' actions
STATUS: STANDS -- L73 adds one exception and L64 names the slider
HIS: L73, L64
RULE: (a) While a global action is on, the layers' actions are held back, not switched off: their buttons stay on (with the light of R130), nothing goes back to before, and the controls glide from where the layer's action had them to the global's values over the Global glide time. (b) Global off, or a global action that plays once coming to its end (L64): the layers' actions come back in step with the beat and the controls glide to them over the same time; a control that only the global moved glides back to where it was before. (c) The clips' own actions play on throughout. (d) A clip fire inside a global action cuts at once; only values glide; the clip it fired stays on the layer when the global goes off (assumption D-26). Exception (L73): a layer that is set to stay out of global actions is not held back and not moved (what that switch covers: assumption D-2). The slider counts seconds, 0 to 4 (183).
CHANGED: nothing he corrected. "the transition time" is the Global glide time (L64, 213 b). Added: the exception for a layer with the switch (L73); a play-once global action ending by itself (L64).
TODAY: Not built; facts-actions-open.md P2, U6.
@@END

@@ITEM R161
TITLE: A layer's X, a clip change, an eject, a deck switch
STATUS: REPLACED in one sentence (the deck switch, by 182 b); the eject added (L98); the rest stands
HIS: L74, L98
RULE: A layer's X takes its clip off, and the clip's actions end with it. When one clip replaces another on a layer the old clip's actions end, and the new clip's actions start if they are switched on. The same when a clip that plays once and ejects comes to its end: "if a clip plays once and ejects that is clear the layer." (L98), so its actions end with it as at an X (INFERRED from his word that an eject is a clear). At a tempo stop every clip leaves, so every clip's actions end and their controls go back. The layer's own actions after an X or an eject stay on and keep running with the beat, because they belong to the layer (180 at its default A; assumption D-3). Switching decks changes nothing that is playing; but the NEXT clip fire of a layer's or a global action takes the cell of the deck that is shown by then (182 b; assumption D-10).
CHANGED: "Switching decks changes nothing for actions" is narrowed by his 182 b: what plays goes on, the next fire comes from the shown deck. Added: the eject (L98). The sentence on the layer's own actions depends on 180 (assumption D-3).
TODAY: A layer's X takes every routine off that layer (area-actions.md 1.19). facts-actions-open.md P12.
@@END

@@ITEM R145
TITLE: Delete, copy, cut, paste of an action
STATUS: REPLACED in (d) (clips get copy and paste, L4, L5); the rest stands
HIS: L4, L5, L1
RULE: An action can be deleted, copied, cut and pasted. (a) A plain click on its button only switches it on or off; a right-click marks the action and opens a menu with Rename, Copy, Cut, Paste and Delete. (b) The Mac's own keys work on the marked action: Cmd+C, Cmd+X, Cmd+V and the Delete key. (c) Only one thing is marked at a time: marking an action unselects the clip cells, and selecting a clip cell unmarks the action. (d) With a clip cell selected the same keys work on the CLIP: "We need copy and paste for any clip." (L4); "Option drag on a clip copy and pastes into the cell it is dragged to." (L5); Cmd+X on a clip is a cut that can be pasted (question 208 at its default A, L1). A copied clip brings its actions with their own buttons (165 A). The detail of clip copy and paste is ruled in topics F and J. One clipboard serves both: Cmd+V does what the clipboard holds -- a copied action goes onto the selected clip (R162), a copied clip goes into the selected cell (assumption D-29). (e) Cut = copy, and the action leaves its owner at once. (f) Delete, cut and paste of an action are Undo steps, so Delete does not ask first.
CHANGED: (d): the page's "none is added by this" (no copy or paste of clips) is replaced by L4 and L5; the page's own next words there (question 208 A would add them, and Cmd+X on clips becomes a cut that can be pasted) now hold, because 208 keeps its default A. Added: what Cmd+V does when the clipboard and the selection are of two kinds (D-29). The rest stands.
TODAY: Cmd+X clears the selected clips; clips cannot be copied or pasted (src/MainComponent.cpp:4122-4136, cited by the slice, not re-read). No routine operation is an Undo step (area-actions.md 1.9).
@@END

@@ITEM R162
TITLE: Paste of an action; a clip that lacks a part
STATUS: CORRECTED
HIS: L68
RULE: Paste puts a copy onto the clip, the layer or the global row he pastes on: by a right-click on that row of action buttons and Paste, or by selecting a clip by its name and pressing Cmd+V with an action on the clipboard (assumption D-29). A clip's action goes only onto a clip, a layer's only onto a layer, a global one stays global. The pasted action arrives switched off, under the same name, with the Loop setting of the one it was copied from (assumption D-15). A clip that lacks a parameter or an effect the action moves: "that specific action is muted, and the small messages is displayed where the user needs to say OK they understand." (L68) -- the rule and its two readings are written out under 184 (assumption D-6). Copy, paste and "Save without ignored controls" only repeat or trim an action that exists; the review screen is the only place where an action is made from a recording.
CHANGED: "A clip that lacks an effect the action moves: question 184" is answered by his own words (L68): muted, with a small message and OK. The two ways to paste, which the page gave and he did not change, are kept. The rest stands.
TODAY: Not built (facts-actions-open.md P8).
@@END

@@ITEM R146
TITLE: "Save an option with the ignore action items not in it"
STATUS: STANDS
HIS: none
RULE: An action's right-click menu has an entry that saves the action without the controls that are set to ignore, so that the saved action does not move them. What the entry does exactly: 185 (a new action; the old one stays).
CHANGED: nothing
TODAY: Not built (facts-actions-open.md P6, U4).
@@END

@@ITEM R143
TITLE: Routines replaced outright; old show files are deleted
STATUS: CORRECTED
HIS: L69
RULE: Routines are replaced by actions outright: nothing is carried over and no in-between state is protected. "let's delete all the old show files and start from scratch." (L69): the old show files are removed, the app has no old show to open, and no old recording is brought into the review screen. Every save of the show file is read back and checked before it replaces the file on disk. Which files count as "old show files", and when they go: assumption D-7 (the same question as topic F's F-13).
CHANGED: Replaced: the page's "your show still opens" and its lines on dropped routines and saved Snap numbers -- there is no old show left to open (L69). The page's "One thing stays because it is part of the app" is now written as a plain rule. The backup copy of a file in an older format, as a protection for later format changes, is Harmony's and not his word; topic F rules it.
TODAY: One saved show exists, "test with harry", in ~/Library/AudioDNA/compositions; it holds no routine (area-actions.md 1.38; facts-actions-open.md S8). The read-back before the swap is built and merged (docs/claude/pitfalls.md:146, Pitfall 68, cited by the slice). The deletion is a destructive act on his files: done only at build time under RIG-RULES A3 (BD:1152), after the files have been named to him.
@@END

@@ITEM R147
TITLE: The standing readings; a fade on every action jump
STATUS: CORRECTED
HIS: L70, L69
RULE: His correction: "I do want to have a Global fade control on all actions starting/stopping that would create a jump, even with a resync" (L70). So no control that an action moves snaps when an action starts, stops, or changes its place because of a Resync: it glides. The time is read as the Global glide time; at instant it jumps (one slider or several: assumption D-1; what is never smoothed: assumption D-13). Of the readings that the page listed as standing, these belong to this topic and stand: a clip fired by a layer's or a global action plays its own actions too; a clip's actions belong to the clip and move with it; a recording keeps everything moved with the mouse as well (sliders, the layer strip's buttons, a preset he loads). The others are ruled where they belong and are not changed here: the tempo stop as a cut and no Undo step, a Resync while the beat is stopped or paused, and the height of the tempo row (topic A); replaying a recording in the review screen (topic E). The reading about his one saved show falls away, because the old show files go (L69).
CHANGED: Added: the fade on every action start and stop that would jump, a Resync included (L70). Which of the listed readings his "one small correction" was aimed at is not certain (the "no fade" at a stop, or the Resync); the rule is applied to actions in general, as his sentence says. Mended by this ruling: the paper said the Resync reading "is replaced by his L12 and L13"; it is not -- L12 says "unless resync is clicked", which keeps it, and it is topic A's.
TODAY: A routine's restore eases over the last beat or jumps, per routine (area-actions.md 1.14). To change: one global time for all.
@@END

@@ITEM R181
TITLE: Actions: the edges
STATUS: CORRECTED in (a), (b), (e), (f); (c) gains the eject (L98)
HIS: L71, L64, L70, L74, L98
RULE: (a) A layer's or a global action whose Loop toggle is ticked runs until it is switched off; one that is not ticked ends by itself after one pass (L64). (b) An action keeps its place on the beat: firing its clip again, or the clip looping at its own length, never restarts a looping action (a play-once action: assumption D-4); a Resync moves the "1" and the action's place moves with it, and its controls glide to the new place instead of hopping -- "even with a resync" (L70) -- over the Global glide time (assumption D-1). (c) A clip set to play once and hold: its actions hold with it on the last frame. A clip set to play once and eject: its actions end with it (R161, L98). (d) When another clip replaces a clip that has actions, the old clip keeps the look its actions gave it until its fade-out is over; only then do its controls go back. (e) An action made from a slider that a signal was moving replays that movement as recorded, without the music; in the review screen such moves have their own colour: "we need to have a different color for the actions that were recorded off of signals so the user can see it before creating actions, but the actions can be created with the signal recording and treated like a regular action." (L71). (f) A layer's action pasted onto another layer brings its control moves and fires the same columns on the new layer (182 b). (g) A layer set to Ignore Column Trigger is skipped by a global action's clip fires, as it is by a column click, while its controls still follow that global action (shown to him and not corrected; put to him once more because of the new layer switch: assumption D-9). (h) The actions of a bypassed layer (B) run on unseen and are in step when the layer comes back.
CHANGED: (a): "runs until you switch it off" holds only with Loop ticked (L64). (b): "its sliders can hop at your Resync" is replaced by a glide (L70). (c): the eject added (L98). (e): added the colour in the review screen (L71). (f): settled by 182 b. (d), (g), (h) stand.
TODAY: area-actions.md H4, H5, H6, H12, H13, M2; none of it is built for actions.
@@END

@@ITEM R182
TITLE: Harmony's own correction about two routines on one control
STATUS: STANDS
HIS: none
RULE: His answer 160 a stands as he gave it: of two actions on one control the one switched on last moves it, and when it goes off the earlier one takes the control again. It is new work.
CHANGED: nothing
TODAY: The earlier routine does not take the control back when the later one lets go (area-actions.md 1.17, O7).
@@END

@@ITEM R224
TITLE: Saved actions are not edited; Stop actions button
STATUS: CORRECTED (he says yes and adds)
HIS: L72, L114
RULE: (a) A saved action cannot be opened and changed: to change one he makes a new one from the recording in the review screen and deletes the old one. This stands by his "R224 yes. I like that." (L72); his added sentence says where actions are made and that a show is not the place to change them: "We will only allow users to record actions in the recording review screen, not modify during a show." (L72). What can be done to a saved action: switch it on and off, set its Loop toggle, Rename, Copy, Cut, Paste, Delete, "Save without ignored controls". (b) Two actions that both fire clips on the same layer: a global action wins over a layer's; of two actions of one layer the one switched on last fires, and the earlier one carries the light of R130 and fires again when the later one is off. (c) "We need a global stop actions button that stops all actions." (L72): one button on screen, also in the keyboard and MIDI mapping, for every action of every clip, every layer and global. One press switches them all off and their values glide back; he switches on again the ones he wants. That is how his page had it under the mapping (in the page's words: it switches every action off at once); he named that reading and changed another part of it only (L116). It is put to him once more together with the tempo stop: assumption D-3. "The tempo stop button stops all actions as well as everything else." (L72): see 180. The name of the screen where actions are made: "Review", the name he calls good -- "Show Recording Review should be called Review for short is a good name." (L114) -- and provisional, because he goes on: "Do you have a better name for this?" (L114); topic J answers that, and until he picks the screen is written "Review".
CHANGED: (a): stands by his "yes"; the paper called it "confirmed in his own words", but his sentence speaks of recording actions and of not modifying during a show, not of opening a saved action. (c): confirmed ("R224 yes. I like that."), named "global stop actions button", and its reach stated: all actions. Added: the tempo stop stops all actions too. "where it sits comes as a picture" is replaced by L8. (b) stands. Mended by this ruling: what "stops" does is no longer an assumption of its own but one question with the tempo stop (D-3); the name "Review" is marked provisional.
TODAY: The top bar's Stop button (tooltip "Stop all routines") and the GlobalStop key stop every routine and nothing else (area-actions.md 1.21, O20).
@@END

@@ITEM D10
TITLE: An action stays with its effect, not with a place
STATUS: OPEN (Harmony's own; assumption D-18)
HIS: L68, L104
RULE: An action stays with its effect, not with the effect's place in the list: adding, removing or re-ordering other effects changes nothing for it. If its effect is removed, the action is treated exactly as an action pasted onto a clip that lacks that effect: muted by the rule of 184, in whichever of its two readings he picks (assumption D-6). No message is shown, because nothing was pasted. Put the effect back with Undo and the action plays again.
CHANGED: Not read by him. His L104 ("G we want to re-order effects") makes the first half necessary; his L68 gives the word "muted" for a missing effect at a paste, taken here for a removed effect too. Mended by this ruling: the paper fixed that only the part is muted and the rest plays on; that is one of the two readings of L68 and now follows D-6.
TODAY: A routine finds an effect by its position and drives whatever sits there (area-actions.md 1.24, T1; facts-actions-open.md P8).
@@END

@@ITEM D11
TITLE: Deleting a clip, a layer or a deck takes its actions
STATUS: OPEN (Harmony's own; assumption D-19)
HIS: L74
RULE: Delete a clip, a layer or a deck and its own actions go with it; Undo brings them back together. The second sentence is settled by 182 b: a layer's or a global action fires cells, so a moved or deleted clip changes only what sits in the cell.
CHANGED: Not read by him. "question 182" is answered b (L74).
TODAY: A routine belongs to the show, not to a clip or a layer (area-actions.md 1.3, 1.4).
@@END

@@ITEM D12
TITLE: The recorded tempo is saved inside the action
STATUS: OPEN (Harmony's own; assumption D-20)
HIS: none
RULE: An action's recorded tempo is saved inside it. The action is measured in beats and plays in time at any tempo (R144); the saved tempo changes nothing he sees, and there is no control for it.
CHANGED: Not read by him. That the tempo is saved is his earlier word: "the bpm they were recorded with need to be saved" (BD:960); "no control for it" is Harmony's.
TODAY: A routine has no tempo field; nothing reads a recorded tempo (area-actions.md 1.3, H19, W8).
@@END

@@ITEM D13
TITLE: Nothing of routines is carried over; pads, bank and bands go
STATUS: SETTLED for no carrying-over (L69); the list of what goes is Harmony's consequence (assumption D-30)
HIS: L69, L56
RULE: No carrying-over is built for old routines, old shows or old recordings: "let's delete all the old show files and start from scratch." (L69). His earlier word for the rest: "Just replace with action." (binding-decisions.md:1095). Harmony's list of what that takes away (assumption D-30): the row of eight routine pads above the columns, the bank of eight in the show, the name bands on a layer's picture with their "x", the routine colour and hint on a knob, and the lines a routine writes in the Record tab. In their place stand the action buttons: the clip's row below the clip (L56), the layer's row in the layer strip (binding-decisions.md:982), the global row where it fits (L8). A layer's X takes its clip's actions off (R161).
CHANGED: His words now say the same for the carrying-over (L69) and give the place of the clip's buttons (L56). The list of what goes is Harmony's consequence of "Just replace with action." (binding-decisions.md:1095) and now carries its own assumption (D-30).
TODAY: area-actions.md 1.5, 1.10, 1.11, 1.19, 1.20; all of it is built and pinned by tests (1.39) that fall with it.
@@END

@@ITEM D14
TITLE: An action follows its layer when layers are re-ordered
STATUS: OPEN (Harmony's own; assumptions D-19 and D-6)
HIS: none
RULE: An action follows its layer when the layers are re-ordered; it does not stay on the place (assumption D-19). If a layer is deleted, a global action that moves that layer's controls is treated as an action whose effect was removed (D10): muted by the rule of 184, in whichever of its two readings he picks (assumption D-6).
CHANGED: Not read by him; no word of his covers a re-ordered layer. Mended by this ruling: the paper fixed that only that layer's part is muted and the rest plays on; it now follows D-6, as D10 does.
TODAY: A routine drives a layer by its place (area-actions.md 1.24, M5).
@@END

@@ITEM D15
TITLE: A deck brings its clips' actions; layer and global actions stay
STATUS: OPEN (Harmony's own; assumption D-21)
HIS: L4
RULE: A deck taken from the list of shows, or duplicated, brings its clips' actions with it, each with its own buttons. A layer's actions and the global actions stay with their own show.
CHANGED: Not read by him. It follows the accepted default 165 A (a copied clip has the same actions) and his L4 (clips can be copied).
TODAY: A deck's saved form carries no routine (area-actions.md 1.4, T2).
@@END

@@ITEM P10
TITLE: Where the action buttons sit
STATUS: DROPPED (layout); the clip's row is placed by his L56, the layer's row by his earlier word
HIS: L56, L8
RULE: Laid out by Harmony where it fits in the correct area; its look is settled in the UI redesign (L8). Two places are his: the clip's eight buttons sit in "a small row below the clip, still within the layer" (L56); the layer's eight sit in the layer strip, by his earlier word "They will be in the layer strip" (binding-decisions.md:982). The global sixteen get a place that Harmony finds (L8).
CHANGED: None of the three variants is drawn. L56 places the clip's row below the clip, inside its cell; his earlier word keeps the layer's row in the layer strip.
TODAY: A cell is 90 points wide and the layer strip has about 8 points free (facts-actions-open.md P7, C7).
@@END

@@ITEM P11
TITLE: The look of an action's button and its light
STATUS: DROPPED (look only)
HIS: L8
RULE: Laid out by Harmony where it fits in the correct area; its look is settled in the UI redesign (L8). What the states are (on, off, marked, held by something else) is ruled under R130 and R145.
CHANGED: No variant is drawn.
TODAY: not checked
@@END

@@ITEM P12
TITLE: What the "+" opens
STATUS: DROPPED (look only)
HIS: L8
RULE: Laid out by Harmony where it fits in the correct area; its look is settled in the UI redesign (L8). Whatever it opens shows every further action of that owner with its name, its on / off button and its Loop toggle.
CHANGED: No variant is drawn; both variants do the same thing.
TODAY: not checked
@@END

@@ITEM P13
TITLE: Where the ignore lamp sits
STATUS: DROPPED (look and place); one variant would change function and is ruled out by his earlier word
HIS: L8
RULE: Laid out by Harmony where it fits in the correct area; its look is settled in the UI redesign (L8). The function is fixed by his word: one lamp per control, never one per group of controls -- "every slider, button, everything needs an ignore actions toggle" (BD:1056).
CHANGED: The variant "one lamp per control group on the layer strip" is not taken: it would change what the lamp does.
TODAY: The layer strip uses about 242 of 250 px (area-actions.md 1.35).
@@END

@@ITEM P14
TITLE: A slider or a button while an action is moving it
STATUS: DROPPED (look only)
HIS: L8
RULE: Laid out by Harmony where it fits in the correct area; its look is settled in the UI redesign (L8). A control shows the value the action gives it while the action moves it.
CHANGED: No variant is drawn.
TODAY: A knob held by a routine shows the routine colour and the hint "ROUTINE" (area-actions.md 1.20).
@@END

@@ITEM P15
TITLE: Where the glide slider sits
STATUS: DROPPED (place only)
HIS: L8, L64
RULE: Laid out by Harmony where it fits in the correct area; its look is settled in the UI redesign (L8). The slider is the Global glide slider (L64); that it is the only one, for the whole show, is assumption D-1.
CHANGED: No variant is drawn. The slider no longer serves global over layer alone, as the page had it: his L64, L66, L67 and L70 give it more to do.
TODAY: not checked
@@END

@@ITEM P16
TITLE: An action's button in its other states
STATUS: DROPPED (look only)
HIS: L8, L68
RULE: Laid out by Harmony where it fits in the correct area; its look is settled in the UI redesign (L8). The states exist as function: on while its clip is not playing, waiting for the "1", and muted because a target is missing (L68; the whole action, or a part of it: assumption D-6).
CHANGED: No variant is drawn. "a part it cannot find" is his "muted" (L68).
TODAY: A routine pad shows waiting, playing, a bar counter and a red "!" (area-actions.md 1.11).
@@END

@@ITEM G5
TITLE: A layer's switch against actions, beside Ignore Column Trigger
STATUS: ANSWERED -- whether it is the same switch as L73's bypass, and how far it reaches, is not settled (assumption D-2)
HIS: L7, L73
RULE: "We should have ignore actions toggle on the layer as well as ignore column." (L7). Best reading, taking L7 and his answer to 179 -- "a layer can have a global action bypass in the same way that I can have column trigger bypass" (L73) -- as one thing said twice (L9): each layer has ONE new switch, beside its Ignore Column Trigger switch. While it is on, no global action moves that layer's controls or fires a clip on that layer, and the layer's own actions are not held back by a global action: they keep playing, as its clips' actions do. It does not switch the layer's own actions or its clips' actions off; those have their own buttons, and single controls have their ignore lamps (R131). The other readings -- two switches, or one switch that makes the layer ignore every action, its own and its clips' too -- are asked in assumption D-2. Flipped while a global action is on, nothing jumps, by his rule that no action starting or stopping may "create a jump" (L70): turned on, the layer's controls glide back to the layer's own action, or to where they were before, over the Global glide time; turned off, the global takes the layer with the same glide (assumption D-27). A clip that the global action had fired on the layer stays (assumption D-26). The show remembers the switch (assumption D-17); it can be put on a key or a pad like every button (topic J, question 206 at its default); no action holds it (assumption D-28). It sits with three other things that keep actions away, each with its own reach: the ignore lamp (one control, all actions), Ignore Column Trigger (the layer: column clicks, and also a global action's clip fires -- assumption D-9), Stop actions (everything, once).
CHANGED: New; it answers no numbered item. Mended by this ruling: the edges at a flip and the reach of Ignore Column Trigger stood here as settled; they are Harmony's and now carry their assumptions (D-27, D-26, D-9). Topics J and K read L7 the other way (a layer that ignores every action): the page ruling has to make the topics one (D-2).
TODAY: A layer has "Ignore Column Trigger" in the Layer inspector (area-actions.md 1.30: Layer.h:210, LayerInspector.cpp:155-160). Nothing of the new switch exists.
@@END

## ASSUMPTIONS
@@ASSUME D-1
ABOUT: 183, 213, R172, R129, R131, R147, R160, R181, P15
TEXT: I assume there is ONE slider, called Global glide (instant to 4 seconds). It times every glide of an action: starting, switching off, going back after playing once, your hand letting go, a Resync, and a global action taking over.
WHY: Four names: L64 Global glide slider, L66 global glide back setting, L67 master action fade back time, L70 Global fade control on starting/stopping. Never said to be one.
ALT: b) Two sliders: one for how actions start, one for how values go back. c) The fade for starts, stops and a Resync is a control of its own, apart from the glide back.
IF-WRONG: STAGE with one slider a long glide back also softens the start of every action.
ASK: YES his four names do not settle it, and starts would look different on stage.
@@END

@@ASSUME D-2
ABOUT: G5, 179, R160
TEXT: I assume a layer gets ONE new switch, beside Ignore Column Trigger: while it is on, global actions leave that layer alone. The layer's own actions and its clips' actions keep playing.
WHY: L7 says ignore actions toggle on the layer; L73 says global action bypass. Both compare it to the column switch; one thing or two, and its reach, is not said.
ALT: b) Two switches: the second makes the layer ignore every action, its own and its clips' too. c) One switch, and it makes the layer ignore every action.
IF-WRONG: STAGE the layer's own actions would stop, or not stop, when he flips it.
ASK: YES two lines of his, two possible meanings, and the result differs on stage; topics J and K took the other meaning.
@@END

@@ASSUME D-3
ABOUT: 180, R161, R224
TEXT: I assume the two stops differ. Stop actions switches every action off and their values glide back. The tempo stop leaves the action buttons on: the actions stand still and start again from their beginning when the beat next runs.
WHY: L72 says stops all actions for both buttons. The default he accepted keeps the buttons on at a tempo stop (L1); the page's line for Stop actions said switched off.
ALT: b) The tempo stop also switches every action off and their values go back; you switch on again the ones you want. c) Stop actions is a hold: while it is lit every action stands still, and they carry on when you release it.
IF-WRONG: STAGE after a stop between two songs his actions come back by themselves, or they do not; one press leaves every action button dark, or lit.
ASK: YES his new sentence can be read against the default he accepted; one question covers both stop buttons.
@@END

@@ASSUME D-4
ABOUT: R172, R181, R158
TEXT: I assume an action set to play once on a clip plays once each time that clip is fired, and its button stays on. On a layer or global it plays once and its button goes off by itself.
WHY: L64 says it plays once and goes back; it does not say what its button does afterwards, or when it plays again.
ALT: b) Every play-once action switches its own button off when it is done. c) Its button always stays on, and it plays again only after you switch it off and on.
IF-WRONG: STAGE a one-time move that fires again with its clip, or never again.
ASK: YES the loop toggle is new and this half of it is not in his words.
@@END

@@ASSUME D-5
ABOUT: R129
TEXT: I assume a preset loaded on an effect while an action plays is kept underneath: the action keeps moving its sliders, and when it goes off they glide to the preset's values. The other sliders take the preset at once.
WHY: L66 asks for the explanation and gives no rule; the page's own reading threw the preset's values away on the sliders the action moves.
ALT: b) Those sliders go back to their values from before the action; the preset's values for them are lost.
IF-WRONG: STAGE the look he gets when the action goes off is the preset's, or the old one.
ASK: YES he asked; this is the recommendation of the answer below.
@@END

@@ASSUME D-6
ABOUT: 184, R162, P16, D10, D14
TEXT: I assume the whole pasted action is muted while the clip lacks something it moves: it sits on the clip, silent, and wakes up when you add what is missing. A small message says what is missing; you press OK.
WHY: L68 says that specific action is muted: read as the action that was pasted. It can also mean only its part for the missing parameter or effect.
ALT: b) Only the part the clip lacks is muted; the other parts of the pasted action play.
IF-WRONG: STAGE a pasted action that does not play at all, or plays in part.
ASK: YES his sentence reads two ways; a removed effect and a deleted layer follow the same answer.
@@END

@@ASSUME D-7
ABOUT: R143, D13
TEXT: I assume "delete all the old show files" means every show saved so far, with its backup copies: they go to the Trash when the build starts, after I have named them. Recordings, presets and settings are not touched.
WHY: L69 says all the old show files; whether old recordings, deck files and other saved files count, and when they go, is not said.
ALT: b) Old deck files and old recordings go too. c) Nothing is deleted; the new app simply does not open old shows.
IF-WRONG: REBUILD files of his deleted that he wanted, or old-format reading built for nothing.
ASK: YES it is a deletion of his own files; the same question as topic F's F-13 (one question on his page).
@@END

@@ASSUME D-8
ABOUT: 179, R133
TEXT: I assume rows of two or more layers that you tick together become one global action under one button; a layer that should stay out of it uses its switch against global actions.
WHY: L73 answers with the layer's bypass and names neither letter; the bypass only has a job if the pick becomes a global action.
ALT: b) The pick becomes one layer action per layer, each with its own button.
IF-WRONG: STAGE one button for a move across layers, or two.
ASK: LINE his answer points to it without naming the letter.
@@END

@@ASSUME D-9
ABOUT: R181, G5
TEXT: I assume a layer set to Ignore Column Trigger is also skipped by the clip fires of a global action, while its sliders still follow that global action.
WHY: It was shown and not corrected; but L73 now gives the layer a switch of its own against global actions.
ALT: b) Ignore Column Trigger blocks column clicks only; a global action is kept out only by the layer's new switch.
IF-WRONG: STAGE a layer he protects from column clicks changes its clip, or does not, under a global action.
ASK: LINE the new switch may make this older line unwanted.
@@END

@@ASSUME D-10
ABOUT: 182, 194, R161
TEXT: I assume an action that fires a cell acts like your own click on that cell in the deck on screen at that moment: an empty cell empties the layer, and a clip in BPM mode waits for the 1.
WHY: 182 b: the action fires cells. L11 and L19 cover an empty cell for a click, not for an action; when the deck is read is not said.
ALT: b) An action skips an empty cell and the layer keeps its clip. c) The deck is fixed when the action is switched on; a later deck switch does not change what it fires.
IF-WRONG: STAGE a layer that goes empty in the middle of an action's rhythm; a deck switch that changes the picture at the action's next fire.
ASK: LINE the same as a key or pad on a cell (topic F, question 197 at its default) and his words on empty cells, taken one step further to actions.
@@END

@@ASSUME D-11
ABOUT: R131
TEXT: I assume lighting a control's ignore lamp while an action is moving it lets the control go: it glides back to where it was before the action, and from then on it stays where you put it.
WHY: L67, written on this point, says an action switched off goes back in the fade back time; whether lighting the lamp counts as that is not said.
ALT: b) It stays where it is at that moment, so that you can catch a value the action has reached.
IF-WRONG: STAGE where one slider rests after he takes it away from an action: his own value, or the value of that moment.
ASK: LINE it follows his rule that a control goes back when an action lets it go (159 b, L67); he can strike it.
@@END

@@ASSUME D-13
ABOUT: 213, R172, R147
TEXT: I assume the glide never smooths what you recorded: a jump inside an action, and the step where a loop starts over, stay as recorded. Buttons and lists switch at once; only sliders glide.
WHY: L70 names starting, stopping and a Resync; it does not name the loop's own return or a recorded jump.
ALT: b) The step where a loop starts over is glided too.
IF-WRONG: STAGE a loop whose end and start differ snaps, or eases, each time round.
ASK: LINE a choice of mine he will most likely wave through.
@@END

@@ASSUME D-14
ABOUT: R172, R158
TEXT: I assume a glide at an action's start begins on the 1 and eases into the action's moving value; nothing moves before the 1.
WHY: L70 asks for a fade on starts; when that fade begins is not said.
ALT: b) The glide begins at your press, so the control is already in place on the 1.
IF-WRONG: STAGE the first second of an action is soft, or the control moves before the bar.
ASK: LINE a choice of mine about timing he can strike.
@@END

@@ASSUME D-15
ABOUT: R172, R162, R134
TEXT: I assume a new action arrives switched off and with Loop ticked, and a pasted copy keeps the Loop setting of the action it was copied from.
WHY: L64 gives the toggle, not its first state; his earlier answer was that actions loop. Whether a new action arrives on or off is not said.
ALT: b) A new action arrives with Loop not ticked.
IF-WRONG: SMALL one tick.
ASK: NO a first state that follows his earlier answer and the rule for a pasted action.
@@END

@@ASSUME D-16
ABOUT: R159
TEXT: I assume the warning about an action that is not a multiple of 4 bars does not stop the save: you can save it as it is.
WHY: L65 says show a warning; a warning is not a refusal.
ALT: b) The save is refused until the length is a multiple of 4 bars.
IF-WRONG: SMALL one check in the save window.
ASK: NO the word warning carries it.
@@END

@@ASSUME D-17
ABOUT: R132, G5, 183
TEXT: I assume the show remembers each action's Loop toggle, each layer's switch against global actions and the Global glide time, as it remembers the ignore lamps. In a new show the glide time starts at 1 second.
WHY: L64 and L73 create the settings and do not say where they are kept; L64 gives the slider's range, not where it starts.
ALT: b) The glide time starts at instant in a new show.
IF-WRONG: SMALL one slider move.
ASK: NO internal; his earlier answer already saves the ignore lamps with the show, and he sets the slider himself.
@@END

@@ASSUME D-18
ABOUT: D10
TEXT: I assume an action follows its effect when effects are re-ordered. When its effect is removed, the action is muted in the same way as an action pasted onto a clip that lacks that effect, without a message.
WHY: L104 allows re-ordering effects and L68 mutes at a paste; a removed effect is not named.
ALT: b) Removing an effect removes that part from the action for good.
IF-WRONG: SMALL
ASK: NO the same rule as his paste rule, one case further; it follows his answer on the paste.
@@END

@@ASSUME D-19
ABOUT: D11, D14
TEXT: I assume an action goes with its clip or layer when that is deleted (Undo brings both back) and follows its layer when layers are re-ordered.
WHY: No word of his covers a deleted owner or a re-ordered layer.
ALT: b) An action stays on the place in the stack and drives whatever layer sits there.
IF-WRONG: SMALL
ASK: NO any other behaviour would be a fault he would report.
@@END

@@ASSUME D-20
ABOUT: D12
TEXT: I assume the tempo an action was recorded at is stored inside it and shown nowhere: the action plays in time at any tempo.
WHY: His earlier word asks for the tempo to be saved; nothing says he wants to see or use it.
ALT: b) The action's name window shows the tempo it was recorded at.
IF-WRONG: SMALL
ASK: NO internal.
@@END

@@ASSUME D-21
ABOUT: D15
TEXT: I assume a deck that is duplicated or brought in from another show brings its clips' actions; layer actions and global actions stay with the show they were made in.
WHY: No word of his covers a deck that moves between shows.
ALT: b) Layer and global actions come along with the deck.
IF-WRONG: SMALL
ASK: NO it follows the accepted default that a copied clip keeps its actions.
@@END

@@ASSUME D-22
ABOUT: 193, R158
TEXT: I assume a pause that an action itself presses on its clip does not hold that action still; only a pause you press yourself does.
WHY: 193 b lets an action press pause; the timing rule says a paused clip's actions hold still. Together they would lock.
ALT: none
IF-WRONG: SMALL
ASK: NO internal logic; the other way the action could never un-pause its clip.
@@END

@@ASSUME D-23
ABOUT: R133, 179
TEXT: I assume rows of the global tab ticked together with rows of layers become one global action, and the action save window lets you name each action in its list and leave one out.
WHY: L55 says the app breaks a mixed pick down and saves in the action save window; it does not give the window's parts.
ALT: b) Global-tab rows and each layer's rows always become separate actions.
IF-WRONG: SMALL the window shows the list before anything is saved.
ASK: NO he sees and can change the result in the window before saving.
@@END

@@ASSUME D-24
ABOUT: 182
TEXT: I assume an action skips a column that the deck on screen does not have: nothing happens on that fire and the layer keeps its clip.
WHY: 182 b says the columns of the deck that is shown; a deck with fewer columns is not named.
ALT: b) A missing column counts as an empty cell and empties the layer.
IF-WRONG: SMALL
ASK: NO technical; topic F notes the same for a key or pad on a missing column (under its question 197); the half that shows on stage is in D-10.
@@END

@@ASSUME D-25
ABOUT: 213, R172, R160
TEXT: I assume a glide towards a value that is itself moving blends from the old value into the moving one over the glide time, so it lands exactly on it.
WHY: His words give the time of the glide, not its arithmetic.
ALT: none
IF-WRONG: SMALL
ASK: NO technical.
@@END

@@ASSUME D-26
ABOUT: R172, R160, R129, G5, 213
TEXT: I assume a clip that an action fired stays on its layer when that action goes off, has played once, or the layer is switched out of global actions. Only sliders, buttons and lists go back.
WHY: L64 and 213 b speak of values going back to their pre-action positions; what becomes of a clip the action fired is not said.
ALT: b) The layer goes back to the clip that played before the action.
IF-WRONG: STAGE the picture after an action ends: its last clip, or the one from before.
ASK: LINE a choice of mine he will most likely wave through: a clip is not a value.
@@END

@@ASSUME D-27
ABOUT: G5
TEXT: I assume flipping a layer's switch against global actions while a global action is on never makes a value jump: the layer's values glide to their new place over the glide time.
WHY: L70 names actions starting, stopping and a Resync; flipping the layer's switch is not named.
ALT: none
IF-WRONG: SMALL
ASK: NO it follows his rule that nothing an action moves may jump (L70).
@@END

@@ASSUME D-28
ABOUT: 193, R131, G5
TEXT: I assume an action never holds the controls that steer actions themselves: an action's on / off button and Loop toggle, an ignore lamp, a layer's switch against actions, Stop actions and the glide slider.
WHY: 193 b says everything changed from its default; L66 rules out an action inside an action; the other steering controls are not named.
ALT: b) An action can also hold an ignore lamp or a layer's switch, so that it can flip them in rhythm.
IF-WRONG: SMALL
ASK: NO internal logic; otherwise an action could switch itself off or lock itself out.
@@END

@@ASSUME D-29
ABOUT: R145, R162
TEXT: I assume one clipboard serves clips and actions, and Cmd+V does what it holds: a copied action goes onto the selected clip, a copied clip goes into the selected cell.
WHY: L4 gives clips copy and paste; the page already let Cmd+V paste an action onto a selected clip; which wins is not said.
ALT: b) Cmd+V on a selected cell always pastes a clip; an action is pasted from the right-click menu only.
IF-WRONG: SMALL
ASK: NO how every clipboard works; both ways to paste an action were on his page and he changed neither.
@@END

@@ASSUME D-30
ABOUT: D13
TEXT: I assume everything that showed a routine goes with the routines: the eight pads above the columns, the bank of eight, the name band on a layer with its x, and the routine colour on a knob.
WHY: His earlier word is Just replace with action (binding-decisions.md:1095); the list of what goes is mine.
ALT: b) A layer keeps a band that names the actions playing on it.
IF-WRONG: SMALL how a playing action is shown is settled in the UI redesign (L8).
ASK: NO look only; the action buttons take their place.
@@END

@@ASSUME D-31
ABOUT: R172
TEXT: I assume un-ticking Loop while an action plays lets it finish its pass and then go back; ticking Loop on a play-once action that is still playing lets it loop on.
WHY: L64 gives the toggle and its two states, not a change in the middle of a pass.
ALT: b) Un-ticking Loop ends the action at once.
IF-WRONG: SMALL
ASK: NO the natural result of his two states.
@@END

## QUESTIONS BACK
@@ANSWER R129-4
HIS: L66
ANSWER: I recommend: loading a preset never stops the action, and the preset is never lost. Example: an effect has sliders A, B and C, and an action is moving A. You load a preset. (1) B and C take the preset's values at once. (2) A keeps moving with the action. (3) The preset's value for A is kept underneath: when the action goes off, or has played once, A glides to the preset's value, not to the value from before the action. The other way, which my page described: the preset's value for A is thrown away, and A goes back to its old value when the action goes off. Tell me if you want that instead.
@@END

## NAMES
@@NAME Global
MEANS: The top level of the show and its tab: the controls, effects and actions that act on all layers together.
SOURCE: his words L55 ("lets move to global"); the on-screen name now is "Composition", which is replaced by "Global"
@@END

@@NAME action
MEANS: A saved stretch of recorded moves, counted in beats, that belongs to one clip, one layer or Global and has one on / off button.
SOURCE: binding-decisions.md:960 ("we are calling routines actions."); the on-screen name now is "routine", which is replaced by "action"
@@END

@@NAME clip action, layer action, global action
MEANS: The three kinds of action, named after their owner.
SOURCE: binding-decisions.md:1044-1049; his words L73 ("global action")
@@END

@@NAME Global glide
MEANS: The one slider (instant to 4 seconds) that times every glide of a control into or out of an action.
SOURCE: his words L64 ("the Global glide slider"); Harmony's pick among his four names (L66 "global glide back setting", L67 "master action fade back time", L70 "Global fade control"); it replaces the page's "transition slider"
@@END

@@NAME Loop
MEANS: The toggle beside each action: ticked = the action loops; not ticked = it plays once and its controls go back.
SOURCE: his words L64 ("a loop toggle right there")
@@END

@@NAME Stop actions
MEANS: The one button (and key or pad) that stops every action of every clip, every layer and Global.
SOURCE: his words L72 ("a global stop actions button")
@@END

@@NAME Ignore Global Actions
MEANS: The switch on a layer that keeps every global action away from that layer.
SOURCE: Harmony's pick for his L7 ("ignore actions toggle on the layer") and L73 ("global action bypass"); open until assumption D-2 is answered
@@END

@@NAME Ignore Column Trigger
MEANS: The switch on a layer that keeps column clicks away from that layer.
SOURCE: the on-screen name now, kept; his words L7 ("ignore column") and L73 ("column trigger bypass")
@@END

@@NAME ignore lamp
MEANS: The small toggle beside one slider or button: lit, that control ignores all its actions.
SOURCE: binding-decisions.md:1056 ("an ignore actions toggle") and binding-decisions.md:1085 ("this lamp a toggle")
@@END

@@NAME action save window
MEANS: The last step of making an action in Review: it lists the actions the pick becomes, with owner, name, length and the 4-bar warning, and saves them.
SOURCE: his words L55 ("the action save window"), L65 ("save screen")
@@END

@@NAME Review
MEANS: The screen where a recording is looked at and actions are made (the long name is Show Recording Review).
SOURCE: his words L114 ("Show Recording Review should be called Review for short is a good name."); provisional, because he goes on "Do you have a better name for this?" (L114) and wrote "We need a good name for this screen" (L72); topic J answers that (its answer review-name), topic E owns the screen
@@END

## CONFLICTS (from the paper, unruled)
- Tempo stop and the action buttons: "The tempo stop button stops all actions as well as everything else." (L72), in the same breath as "a global stop actions button that stops all actions", against the default of question 180 that he accepted with "All defaults good except for these." (L1): there the buttons stay ON. Asked: D-3.
- One layer switch or two: "We should have ignore actions toggle on the layer as well as ignore column." (L7) against "a layer can have a global action bypass in the same way that I can have column trigger bypass" (L73). Asked: D-2.
- One glide control or several: "the Global glide slider" (L64), "the global glide back setting" (L66), "the master action fade back time" (L67), "a Global fade control on all actions starting/stopping" (L70). Asked: D-1.
- Hand lets go: "the slider moves back to the action position based on the global glide back setting I described earlier." (L66) against the default 158 A (it jumps at once) accepted on 2026-10-05 (binding-decisions.md:1078). The newer word is taken: a glide. Not asked.
- Loop: "Each action where it is placed, will have a loop toggle right there." (L64) against his "139 a" (binding-decisions.md:1017: an action loops until its clip stops or it is switched off). The newer word is taken. Not asked.
- Place of a clip's action buttons: "a small row below the clip, still within the layer" (L56) against "There will be a little area above each clip" (binding-decisions.md:980). The newer word is taken. Not asked.
- Start of an action: "a Global fade control on all actions starting/stopping that would create a jump" (L70) against the page's line he read under R172 without changing it there (a control "jumps there on the 1"). L70 is taken: a glide. Part of D-1.

## NOT DONE / UNSURE (from the paper, unruled)
- Which standing reading his "one small correction" (L70) was aimed at: the "no fade" at a stop, or the Resync. Applied as a rule for actions in general. Cheapest: his answer to D-1 settles what matters.
- L36 (a previewed clip "shows the clip with all its actions and it is triggered on the 1") means a clip's actions must be able to play for the preview alone. Topic B owns it; nothing here contradicts it, and nothing here says how a clip that is previewed AND playing live shares one set of action values. Cheapest: read apply-B against R129 here.
- L38 ("a global effects and actions toggle button called master cue") is topic B's; it is not the Stop actions button and not the Global glide. Not ruled here.
- L66's idea (a recording that holds an action's moves): how the rows look is topic E's. Not said by him: whether the press on the action's button gets a row of its own. Cheapest: apply-E.
- Whether the picture img-08 is a cell of Audio-DNA or of another app: read as Audio-DNA's own (INFERRED from its badges). It does not change the rule: a slim row under the clip's name.
- Every TODAY line is taken from the two fact sheets or from the slice's source lines; no source file was re-read for this paper.

STAMP: 2026-10-07 22:46:07 EDT

## FOR THE PAGE RULING (from the ruling)
- THE LAYER SWITCH (L7, L73): this topic reads ONE switch that keeps GLOBAL actions off a layer (D-2, YES). Topic J's name "ignore actions" and topic K's autopilot line read L7 as: the layer ignores every action, its clips' too. One reading for all topics; the name on screen ("Ignore Global Actions" is Harmony's pick) follows his answer.
- ONE GLIDE SLIDER (D-1, YES): "Global glide" for L64, L66, L67 and L70's "Global fade control". Topic F saves "the global glide-back time", topic J lists "the glide slider": one name everywhere.
- THE TWO STOPS (D-3, YES; D-12 folded in): Stop actions switches every action off; the tempo stop leaves the buttons on (180 default A). Topic A's stop item and topic J's R199 d quote the same L72 sentence and must say the same. Spelling: "Stop actions" here, "Stop Actions" in J.
- D-7 is the same question as topic F's F-13 (which old files go to the Trash, L69): one question, one scope.
- L68 (D-6, YES): the main reading is now the WHOLE pasted action muted, with the part-only reading as b. A removed effect and a deleted layer follow his answer (D10, D14). The small message with OK is one that topic J's 209 must allow (L122).
- L36 (topic B): a previewed clip plays its own actions from the "1". No topic rules a clip that is live AND previewed: one set of action values, or two. L36's reason (actions need the "1") also sits beside R158 b here: a clip not in BPM mode starts at the press, its action on the next "1".
- L70's "one small correction" may have been aimed at the page's "no fade" at a tempo stop (topic A). Here it is applied to actions only.
- 182 b (D-10, LINE): an action's fire is treated like his own click, and like a pad's fire in topic F's 197: the cell of the deck on screen; an empty cell empties the layer (L11, L19). New here: a RUNNING action keeps firing, so a deck switch changes the picture at its next fire.
- ONE CLIPBOARD for clips and actions (D-29, NO); clip copy, cut and paste belong to topics F and J (208 A, L4, L5). A clip that an action fired stays when the action ends (D-26, LINE). A lamp lit under a running action sends the control back (D-11, LINE; main reading flipped).
- NAMES for the list: Global glide, Loop, Stop actions, action save window, ignore lamp (topic J writes "ignore actions"), Review (provisional: topic J's answer review-name).

