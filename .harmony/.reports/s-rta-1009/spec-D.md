# SPEC D -- Actions in a show (s-rta-1009): the s-rta-1007 spec with Boris's answers to page 2 laid over it by wf/merge3.py (paper apply-D.md, ruling rule-D.md). This file wins over every older one.

## PAGE 2 ITEMS (the rule now for each item of page 2 in this topic)
@@ITEM 224
TITLE: One slider, Global glide, times every smooth change of an action
STATUS: ACCEPTED
HIS: box left empty = accepted as written
RULE: There is ONE slider for the whole show, Global glide, from instant (0) to 4 seconds, counted in seconds (with @@ITEM 183 as it stands). It sets how long every smooth change of a control that an action moves takes: (1) when an action starts and a control would jump to the action's value (with @@ITEM R172 as it stands); (2) when an action is switched off and its controls go back to where they were before it (with @@ITEM 213 as it stands); (3) when a play-once action has played once and its controls go back (R172; item 227); (4) when one action takes a control over from another or hands it back -- a later action over an earlier one, a global action over a layer's actions and back (with @@ITEM R130 and @@ITEM R160 as they stand); (5) after a Resync, when an action's place on the beat moves and its controls would hop (with @@ITEM R181 b and @@ITEM R147 as they stand). The same slider, and no other, also times the changes that the other rules already tie to it: his hand letting go of a slider that an action moves (R129 point 2, his own word L66), a slider with a signal going back to its signal (181), a control whose Ignore Actions toggle is switched on or off under a running action (R131; item 255), a layer's Ignore Global Actions switch flipped under a running global action (G5), a slider going back after a preset was loaded under an action (item 228), and every action switched off by Stop actions or by the tempo stop (item 226). There is no second slider, and no glide time of its own for one action, one clip or one layer. At instant every one of these changes is a jump. On / off buttons and menus cannot glide: they switch at once; what was recorded inside an action plays as recorded (assumption D-13). The show remembers the slider's value (R132).
CHANGED: nothing: accepted as written. It closes the question whether his four names (L64 "the Global glide slider", L66 "the global glide back setting", L67 "the master action fade back time", L70 "a Global fade control") are one slider: they are one. The old rules that carried that question as a pointer (183, 213, R172, R147, R160, R181, P15) already say one slider and are not amended.
TODAY: No such slider exists. A routine's restore eases over the last beat or jumps, per routine (area-actions.md 1.14); a signal comes back to a slider over a fixed 120 ms (area-actions.md 1.29). Taken from the old blocks 183, 181 and R147; not re-read.
@@END

@@ITEM 225
TITLE: A layer's one switch against global actions
STATUS: ACCEPTED
HIS: box left empty = accepted as written
RULE: Every layer gets ONE new switch, Ignore Global Actions, of the same kind as its Ignore Column Trigger switch. While it is on: no global action moves any control of that layer; no global action triggers a clip on that layer; and the layer's own actions are not held back by a global action that is on. The layer's own actions and its clips' actions keep playing exactly as if no global action were on. The switch switches no action off, and it does not touch the layer's own action buttons, its clips' action buttons or the Ignore Actions toggle of any single control. There is no second switch, and no switch that makes a layer ignore every action. What happens when it is flipped while a global action is on (the layer's values glide, nothing jumps), that the show remembers it, that it can be put on a key or a pad and that no action holds it: with @@ITEM G5 as it stands. The global action's side (rows of two or more layers become one global action; held-back layer actions): with @@ITEM 179 and @@ITEM R160 as they stand. The switch is no protection against the two stops: Stop actions and the tempo stop switch off the actions of such a layer like all others (item 226).
CHANGED: nothing: accepted as written. It closes "one switch or two, and how far it reaches" (his L7 "ignore actions toggle on the layer" and L73 "a global action bypass" are one switch, against global actions only). The other reading (a layer that ignores every action, its own and its clips' too), which topics J and K had taken, falls. G5, 179 and R160 already say this and are not amended.
TODAY: A layer has only "Ignore Column Trigger" in the Layer inspector (area-actions.md 1.30: Layer.h:210, LayerInspector.cpp:155-160, cited by the old block G5, not re-read). Nothing of the new switch exists.
@@END

@@ITEM 226
TITLE: The tempo stop switches every action off, clips' actions too
STATUS: ANSWERED in his words; no letter. The item's way (the buttons go off) stands because his words do not go against it; its reach is widened by him
HIS: BF250 "tempo stop stops all actions, not just global"; BF271 "Stop removes all clips from all layers"
RULE: The tempo stop stops ALL actions: the actions of every clip, of every layer and of global. One press on the tempo stop does to actions what one press on Stop actions does (with @@ITEM R224 c as it stands): every action that is on is switched off -- its button goes off -- and it stays off. Nothing starts again by itself when the beat next runs. (a) Layer actions and global actions: switched off; every control they moved goes back to where it was before the action, over the Global glide time (213; item 224); on / off buttons and menus switch back at once. A layer action that a global action was holding back is switched off as well; nothing is left held. (b) Clip actions: the stop takes every clip off every layer (his BF271; topic A rules it), so every clip's actions end with their clip and their controls go back (R161). In addition the BUTTONS of clip actions go off, on every clip of the show, playing or not, exactly as at Stop actions: that is what "all actions" adds to the item, and it is the part that is asked (assumption D3-1). (c) A clip that an action had triggered leaves with every other clip; nothing is brought back. (d) After the stop he switches on the actions he wants. An action switched on while the beat is stopped waits and moves nothing; a layer's or a global action starts from its beginning on the "1" that starts the beat, a clip's action when its clip is triggered (with @@ITEM R158 d as it stands). (e) Nothing protects an action from the stop: not Ignore Global Actions on a layer, not Ignore Column Trigger. A control whose Ignore Actions toggle is lit was not being moved by any action, so the stop does not move it. (f) The stop changes no Loop toggle, no Ignore Actions toggle, no layer switch and not the Global glide value. (g) The stop switches the actions off also when the beat itself runs on because Ableton Link holds it (assumption D3-7). The tempo PAUSE is not changed by any of this: every action holds still with its button on and carries on from that place when the tempo plays again (R158 e). What the stop does to the beat, the clips and the layers is topic A's; what it does to an audio file is topic K's.
CHANGED: Against the item as he read it: it spoke of "every layer and global action"; his words say "all actions, not just global", so the actions of clips are added. He gave no letter: the item's own way (switched off, as Stop actions does; he switches on again the ones he wants) is kept because he wrote under it without going against it, and way b (the buttons stay on, the actions stand still and start again) is not taken. That the buttons of CLIPS' actions go off too is the widest reading of "all" and is asked (D3-1). Against the old blocks: 180 at its default A (buttons stay ON, controls stay where they stood, a new start with the beat) is replaced; R161's and R224's pointers to the old question are replaced; the old question D-3 is closed.
TODAY: The top bar's Stop takes every routine off and does nothing else; there is no stopped or paused state of the beat on main (area-actions.md 1.21, 1.34; from the old block 180, not re-read). To build: action buttons with an on / off state for clips, layers and global, and the tempo stop and Stop actions both clearing every one of them.
@@END

@@ITEM 227
TITLE: A play-once action on a clip keeps its button on and plays only at a trigger of its clip
STATUS: ANSWERED in his words: neither of the two other ways. His sentence speaks of a clip; the layer and global half of the item stands as he read it
HIS: BF251 "neither. It's button stays on and that action plays again only when that clip is re-triggered"
RULE: A play-once action (its Loop toggle not ticked) ON A CLIP: (a) It plays once, from its beginning, each time its clip is triggered, by the timing of @@ITEM R158 as amended in this round (a BPM mode clip: clip and action start together on the "1"; a Timeline mode clip: the picture at the trigger, the action on the next "1"). (b) After its one pass its controls go back to where they were before it, over the Global glide time (R172), and ITS BUTTON STAYS ON. The button never goes off by itself. (c) The action plays again ONLY when that clip is triggered again. A trigger is any trigger of the clip: a click, a key, a pad, a column trigger, the Autopilot (topic K), a layer's or a global action that triggers its cell (R133 c), and a trigger of the clip while it is already playing, which restarts the clip (item 266, accepted as written; topic H). Nothing else plays it: not the clip coming round at its own length, not a Resync, not a "1" that the app moved, not a tempo pause followed by play, not any later "1", and not its own button: switching the button off and on never plays it. (d) The two other ways of the item do not hold: the button does not go off after one play, and switching it off and on does not play it again; his sentence puts the trigger of the clip in the place of "switch it off and on". (e) Switched on by hand while its clip is already playing, it moves nothing and waits with its button on; it plays at the next trigger of its clip. This is his "only" taken one step further, to a first switching-on that his sentence does not name (assumption D3-3). (f) Its clip triggered again while the pass is still running: the action starts over from its beginning at the start that trigger gives it (assumption D3-4). (g) Its clip leaves during the pass (a layer's Clear button, another clip, an eject, a delete, a paste-over): the action ends with its clip, its controls go back (R161), and its button stays on for the next trigger. Its button switched off during the pass: the action ends at once and its controls go back over the Global glide time, as for any action (213). What a tempo stop or Stop actions does to its button: item 226 and assumption D3-1. (h) A LOOPING action of a clip is not changed: switched on under a playing clip it starts on the next "1" (R158 c), and a new trigger of its clip never restarts it (R181 b). A play-once action ON A LAYER OR ON GLOBAL: his sentence speaks of "that clip" and not of these; "neither" rejects ways b and c and leaves the item's own sentence standing -- it plays once from the next "1" after it is switched on, its controls go back, and its button then goes off by itself; one press switches it on and plays it once more (assumption D3-2). A previewed clip's play-once action in the preview is topic B's (items 220 and 221).
CHANGED: Against the item as he read it: the clip half is his now, and sharper -- the button stays on (the item did not say so outright), and "only when that clip is re-triggered" rules out every other cause of a play, the action's own button included: way c said "only after you switch it off and on", and he wrote the trigger of the clip in its place. Ways b and c are rejected by "neither". The layer and global half stands as he read it and goes to him as one line (D3-2). INFERRED: "re-triggered" is read as any trigger of the clip, also by an action, a column trigger or the Autopilot. Mended by the ruling: the paper let the button, switched off and on, play the action again from the next "1" ("not the only way"); that went against his "only". Against the old blocks: the pointers to the old question D-4 in R172 and R181 are replaced; R158 (c) gets the exception; D-4 is closed.
TODAY: A routine has a loop / once field in its pad menu and belongs to the show, not to a clip (area-actions.md 1.2, 1.3, 1.13; from the old block R172, not re-read). To build: per action a Loop toggle, and for a clip's play-once action a start at every trigger of its clip and at no other moment.
@@END

@@ITEM 228
TITLE: A preset loaded under an action is lost on the moving sliders
STATUS: ANSWERED b
HIS: BF252 "b"
RULE: A preset is loaded onto an effect while an action is moving one or more of that effect's controls. (a) The load never stops, pauses or switches off the action. (b) Every control of the effect that no action is moving at that moment takes the preset's value at once. (c) Every control that an action is moving at that moment keeps moving with the action. The preset's value for that control is not kept anywhere: it is thrown away at the load. (d) When the action lets go of that control -- it is switched off, it has played once, its clip leaves, a stop, the control's Ignore Actions toggle is lit -- the control glides back, over the Global glide time, to the value it had before the action: the same value as if no preset had been loaded. (e) "Moving at that moment" means: an action that is on and playing holds the control, also where its recorded line is flat (a ticked row that stays flat holds its control, R172). A control has no action on it at that moment when the action is off, when a clip's play-once action has finished its pass and waits with its button on (item 227), when the control's Ignore Actions toggle is lit (R131 b), or when the only action that moves it is a layer's action that a global action is holding back while that global action does not move this control (assumption D3-8): there the preset loads plainly, and the preset's value is from then on the value "from before" for the next time an action takes the control. (f) On / off buttons and menus of the effect that the action is moving follow the same rule and switch back at once (assumption D-13). (g) His "b" decides the preset only. The hand on a control that an action moves is ruled in R129 point 2 and R131 b, and a MIDI knob in topic J (R199 e); none of them is changed by this item. (h) After such a load the effect differs from the loaded preset on the controls the action held; whether the effect then shows as changed from its preset is topic C's.
CHANGED: His "b" goes against the recommendation Harmony gave in the answer above the item (the preset waits underneath and the slider glides to it): the recommendation falls, and the item's main text with it. Point 4 of the old block R129 is replaced; the old question D-5 is closed. The page's own first reading of 2026-10-05 (the preset's values are thrown away on the sliders the action moves) is thereby what holds. Mended by the ruling: (g) no longer states a general rule ("nothing that is set on a control while an action moves it lasts") as if it followed from his "b"; the one case in (e) that no rule named, a layer's action that is held back, carries its own assumption (D3-8).
TODAY: A routine's own writes are never recorded and a preset load knows nothing of routines (area-actions.md 1.18, 1.26; facts-actions-open.md P1, P3; from the old block R129, not re-read). To build: a per-control memory of the value from before any action, which a preset load does not write while an action holds the control.
@@END

@@ITEM 254
TITLE: A clip that an action triggered stays on its layer
STATUS: ACCEPTED
HIS: box left empty = accepted as written
RULE: A clip that an action triggered stays on its layer when that action ends: when it is switched off, when it has played once, when a global action is switched off or comes to its end, when Stop actions is pressed, and when the layer is taken out of a global action by its Ignore Global Actions switch. Only what the action moved goes back: sliders glide back over the Global glide time, on / off buttons and menus switch back at once. The layer is never put back to the clip that played before the action and is never emptied by the action's end; the clip plays on by its own loop menu like any clip that was triggered. With @@ITEM R160 d, @@ITEM R172 and @@ITEM G5 as they stand. Not the action's doing: the tempo stop takes every clip off every layer (topic A), this one too (item 226 c).
CHANGED: nothing: accepted as written. The old rules already say it (R160 d, R172, G5) and are not amended.
TODAY: A routine's trigger goes to a position of the deck that was shown at the press (area-actions.md 1.24; from the old block 182, not re-read); a routine that ends leaves everything where it is (area-actions.md 1.14, 1.16).
@@END

@@ITEM 255
TITLE: Ignore Actions switched on under an action sends the control back
STATUS: ACCEPTED
HIS: box left empty = accepted as written
RULE: Every slider and every button has its own Ignore Actions toggle (with @@ITEM R131 as it stands). Switched on while an action is moving that control: the action lets go of that one control at once, and the control glides, over the Global glide time, back to where it was before the action (an on / off button or a menu switches back at once; a slider with a signal glides back to its signal, 181). If his hand is on the control at that moment it stays where his hand leaves it (R131 b). From then on no action moves that control and it stays where he puts it. The action plays on with its other controls. The control does not stay at the value the action had reached: a value is not caught this way.
CHANGED: nothing: accepted as written; R131 b already says it and is not amended. The toggle is written "Ignore Actions" here, the name in NAMES.md ("ignore lamp" is the retired word of the old blocks).
TODAY: No ignore flag exists; the write funnel knows rank, not an ignore flag (area-actions.md T12; facts-actions-open.md P4; from the old block R131, not re-read).
@@END

## ITEMS (the items of the first page; a RULE that page 2 changed carries "[page 2, ...]" marks where it changed)
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
STATUS: DEFAULT (his L1); L72 fits it but can also be read as option B -- assumption D-3 -- AMENDED after page 2 (180 1 by D, 180 2 by D, 180 3 by D, 180 4 by D)
HIS: L1, L72
RULE: The tempo stop switches every action off, as Stop actions does (item 226 of page 2 holds the rule in full): the actions of every layer and of global are switched off, their buttons go off, and every control they moved goes back to where it was before the action, over the Global glide time. Nothing starts again by itself when the beat next runs: he switches on again the ones he wants, and an action switched on while the beat is stopped starts from its beginning on the "1" that starts the beat. [page 2, D: BF250 "tempo stop stops all actions, not just global"] A clip's own actions end with their clip, because the stop takes every clip off (R161; "Stop removes all clips from all layers", BF271), and their buttons go off as well, because he says "all actions" (BF250); that the buttons of clips' actions go off and do not stay on for the next trigger of their clip is assumption D3-1. [page 2, D: BF250 "tempo stop stops all actions, not just global"; BF271 "Stop removes all clips from all layers"] His words "The tempo stop button stops all actions as well as everything else." (L72) say that they stop, and his words of 2026-10-09 say it again for every kind of action: "tempo stop stops all actions, not just global" (BF250). Neither sentence says in so many words that the buttons go off: that they do, as at Stop actions, is the way of page-2 item 226 itself, under which he wrote without going against it and without taking its way b (the buttons stay on). The whole of it is put to him once more in assumption D3-1. [page 2, D: BF250 "tempo stop stops all actions, not just global"] A layer's X (the Clear button) follows the tempo stop, as the first page said it would (the page's words: "with B it also switches that layer's own actions off"): it takes the layer's clip off and switches that layer's own actions off; their controls go back over the Global glide time, and he switches on again the ones he wants. Global actions, the other layers and the buttons of the clip's own actions are not touched by it. An eject and every other way a clip leaves its layer leave the layer's own actions on (R161; assumption D3-5). [page 2, D: BF250 "tempo stop stops all actions, not just global"]
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
STATUS: CORRECTED -- AMENDED after page 2 (R172 1 by D)
HIS: L64, L70
RULE: An action moves only the rows that were ticked when it was made. It starts on a "1". From that moment each of its controls goes to the action's value; where that would be a jump the control glides there, because he wants "a Global fade control on all actions starting/stopping that would create a jump, even with a resync" (L70). That fade is read as the same slider as the glide back, the Global glide: at instant it jumps (one slider or several: assumption D-1; the glide begins on the "1", not before it: assumption D-14; buttons and lists switch at once, and a jump inside the recording stays a jump: assumption D-13; how a glide lands on a value that is itself moving: assumption D-25). A ticked row that stays flat between the In and the Out point holds its control at that one value for as long as the action plays. Controls that are not in the action stay where they are. There is no separate restore switch. Every action has a Loop toggle: "Each action where it is placed, will have a loop toggle right there." (L64). Ticked: the action loops until it is switched off (a clip's action: or until its clip stops). Not ticked: "they will only play once which means they will go back to the position they were at before they played." (L64) -- after one pass its controls glide back to where they were before it, over the Global glide time (L64: "how quickly the values go back to their pre-action positions, from instant to four seconds max"). This holds for clip, layer and global actions alike: "Some actions will loop in a clip, layer, global and some actions will not." (L64). A new action arrives switched off with Loop ticked (assumption D-15). A play-once action on a clip keeps its button on after its pass and plays again only when its clip is triggered again: "It's button stays on and that action plays again only when that clip is re-triggered" (BF251; item 227 of page 2 holds the rule in full). A play-once action on a layer or on global plays once and its button then goes off by itself; one press plays it once more (the item's sentence as he read it; his words do not speak of it: assumption D3-2). [page 2, D: BF251 "It's button stays on" and "only when that clip is re-triggered"] Loop un-ticked in the middle of a pass: assumption D-31. A clip that an action fired stays on its layer when the action ends: assumption D-26.
CHANGED: Replaced: "a control that was somewhere else jumps there on the 1" -- it glides where it would jump (L70); that this glide is timed by the same slider as the glide back is Harmony's reading and is asked as D-1. Replaced: "A global action is the exception" -- with one slider there is no exception: "We will just keep the Global glide slider" (L64). Added: the Loop toggle per action, and play once then go back (L64); this replaces his "139 a" (an action always loops). The rest stands.
TODAY: A routine has a loop / once field in its pad menu and a "restore first" with Ease or Jump at its start (area-actions.md 1.2, 1.13, 1.14, 1.16). To change: the toggle moves beside the action; restore goes; the glide is one global time.
@@END

@@ITEM R158
TITLE: One timing rule: every action starts on a "1"
STATUS: REPLACED in one part (it loops only if its Loop toggle is ticked, L64); the rest stands -- AMENDED after page 2 (R158 1 by D, R158 2 by D, R158 3 by D)
HIS: L64, L12, L13, L31, L36
RULE: Every action starts from its beginning on a "1"; it loops if its Loop toggle is ticked and plays once if not (L64). Its length in beats follows the tempo (R144). (a) A clip in BPM mode: clip and action start together on the "1". (b) A clip not in BPM mode: the picture starts on his press, its action on the next "1". (c) A layer's or a global action that he switches on, or a clip's looping action that he switches on while the clip already plays, starts from its beginning on the next "1". A clip's play-once action is the exception: only a trigger of its clip plays it, so switched on while the clip already plays it waits for that clip's next trigger (item 227 of page 2; assumption D3-3). [page 2, D: BF251 "that action plays again only when that clip is re-triggered"] (d) Beat stopped: no action plays; his fire starts the beat, that press is the "1", and the actions that are on start with it. (e) Beat paused: every action holds still, and carries on from that place when the tempo plays again: "the clock will continue from where it was paused as well" (L31). (f) A clip he paused himself: its actions stop moving its controls, and when it plays again they are back in step with the beat, not shifted. (g) A clip previewed by its name starts in the preview at once, not on the beat: "Previewing a clip should not happen on the beat. It should just be quick" (BF247); this replaces "it is triggered on the 1" (L36). Its own actions in that preview are topic B's: whether and from when they play while the tempo runs (item 220 of page 2 and its assumption); while the tempo is paused or stopped they play in the preview from the click, at the tempo's BPM (item 221 of page 2, accepted as written). So (d) and (e) speak of the output, not of the preview. A clip that is loaded on its layer and cued there plays in time, and its actions follow (a) to (c): "If the clip is loaded into the layer, and we are cueing this way, then it should play in time" (BF247). [page 2, D: BF247 "Previewing a clip should not happen on the beat. It should just be quick"; item 221 accepted as written] (h) The grace that item 249 of page 2 gives a BPM mode clip (topic A) holds for actions too: where an action's start falls within a tenth of a beat after a "1" -- its BPM mode clip was triggered a hair late and starts at once, a Timeline mode clip was triggered a hair after the "1", or the action's own button was pressed a hair late -- the action starts at once and joins at the place it would have reached had it started on the "1"; it does not wait a whole bar (assumption D3-9). [page 2, D: item 249 accepted as written]
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
STATUS: CORRECTED -- point 4 is not confirmed: he asked back (answer R129-4, assumption D-5) -- AMENDED after page 2 (R129 1 by D)
HIS: L66, L64, L67
RULE: When an action is switched off, or has played once, every control it moved glides back to where it was just before any action took hold of it (213). (1) Another action that is still on moves the same control: that one takes it; it goes back only when the last one is off. (2) A hand move never lasts on a slider that an action is moving: the hand wins while it holds, and on letting go "the slider moves back to the action position based on the global glide back setting I described earlier." (L66) -- a glide over the Global glide time, no jump; when the action goes off the slider goes to the value from before the action. To keep his own value on it he lights its ignore lamp while he holds it (R131). (3) A clip's action that ends because its clip stops or is replaced: the same, back to before, so the clip looks the same the next time it is fired. (4) A preset loaded onto an effect while an action plays: the sliders the action is moving stay with the action, and the preset's values for them are lost; when the action goes off or has played once they glide back to the values they had before the action, over the Global glide time. The other sliders take the preset at once (his "b", BF252; item 228 of page 2 holds the rule in full). [page 2, D: BF252 "b"] An action that is not moving a control at that moment (it is off, or it played once and went back) has no say: the preset loads plainly. A slider that follows a macro knob counts as a slider with a signal (181). Added by him: when a recording is made while an action plays, "we will not nest an action in an action." (L66): the recording keeps the moving parameters themselves; a new action made from them holds plain moves; in the review screen those parameters are slightly differently coloured and the action's name is shown in the hierarchy (how it is shown: topic E).
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
STATUS: REPLACED in one sentence (the deck switch, by 182 b); the eject added (L98); the rest stands -- AMENDED after page 2 (R161 1 by D, R161 2 by D, R161 3 by D)
HIS: L74, L98
RULE: A layer's X takes its clip off, and the clip's actions end with it. When one clip replaces another on a layer the old clip's actions end, and the new clip's actions start if they are switched on. The same when a clip that plays once and ejects comes to its end: "if a clip plays once and ejects that is clear the layer." (L98), so its actions end with it as at an X (INFERRED from his word that an eject is a clear). At a tempo stop every clip leaves ("Stop removes all clips from all layers", BF271; topic A rules it), so every clip's actions end and their controls go back; the stop also switches every action's button off, a clip's, a layer's and a global one alike (item 226; the clips' buttons: assumption D3-1). [page 2, D: BF250 "tempo stop stops all actions, not just global"; BF271 "Stop removes all clips from all layers"] After an X (the Clear button) the layer's own actions are switched off, as all actions are at the tempo stop: their buttons go off, their controls go back over the Global glide time, and he switches on again the ones he wants. That is what this reading said, when he read it, for a stop that switches actions off (the page's words: "with B the X switches them off"). The buttons of the clip's own actions are not touched by an X. After an eject, which is the clip's own end and no press of his, and after every other clip change (an empty cell that is triggered, a delete, a paste-over) the layer's own actions stay on and keep running with the beat, because they belong to the layer (his "143 a" of 2026-10-04). All of this is assumption D3-5. [page 2, D: BF250 "tempo stop stops all actions, not just global"] Switching decks changes nothing that is playing; but the NEXT clip fire of a layer's or a global action takes the cell of the deck that is shown by then (182 b; assumption D-10). A clip that is deleted or pasted over while it plays leaves its layer and does not play (his BF256; topic F rules it, and a cut with it): its own actions end with it, as when a clip leaves its layer in any other way, and the layer's own actions are not touched. A clip that is deleted or pasted over takes its own actions with it (D11); the clip that is pasted in brings its own (R145 d); no action stays behind on the cell. [page 2, D: BF256 "Pasting over a clip or deleting a clip, removes it from the layer strip, and it does not play."]
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
STATUS: CORRECTED -- AMENDED after page 2 (R143 1 by F)
HIS: L69
RULE: Routines are replaced by actions outright: nothing is carried over and no in-between state is protected. "let's delete all the old show files and start from scratch." (L69): the old show files are removed, the app has no old show to open, and no old recording is brought into the review screen. Every save of the show file is read back and checked before it replaces the file on disk. The old show files are every show saved up to the build and the app's safety copies of them: they are listed to him first and go to the Trash only after his yes; saved decks, recordings, presets and settings stay (page item 235, ruled in topic F). [page 2, F: item 235 accepted as written]
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
STATUS: CORRECTED in (a), (b), (e), (f); (c) gains the eject (L98) -- AMENDED after page 2 (R181 1 by D, R181 2 by D)
HIS: L71, L64, L70, L74, L98
RULE: (a) A layer's or a global action whose Loop toggle is ticked runs until it is switched off; one that is not ticked ends by itself after one pass (L64). (b) An action keeps its place on the beat: firing its clip again, or the clip looping at its own length, never restarts a looping action (a play-once action on a clip: the clip coming round at its own length never plays it again; a new trigger of its clip does, and nothing else does -- item 227, BF251) [page 2, D: BF251 "plays again only when that clip is re-triggered"]; a Resync moves the "1" and the action's place moves with it, and its controls glide to the new place instead of hopping -- "even with a resync" (L70) -- over the Global glide time (assumption D-1). (c) A clip set to play once and hold: its actions hold with it on the last frame. A clip set to play once and eject: its actions end with it (R161, L98). (d) When another clip replaces a clip that has actions, the old clip keeps the look its actions gave it until its fade-out is over; only then do its controls go back. (e) An action made from a slider that a signal was moving replays that movement as recorded, without the music; in the review screen such moves have their own colour: "we need to have a different color for the actions that were recorded off of signals so the user can see it before creating actions, but the actions can be created with the signal recording and treated like a regular action." (L71). (f) A layer's action pasted onto another layer brings its control moves and fires the same columns on the new layer (182 b). (g) A layer set to Ignore Column Trigger is skipped by a global action's clip fires, as it is by a column click, while its controls still follow that global action (shown to him and not corrected; put to him once more because of the new layer switch: assumption D-9). (h) The actions of a bypassed layer (B) run on unseen and are in step when the layer comes back. (i) In automatic mode the app also looks for the "1" by itself and he corrects it with Resync (his BF246; topic A rules when the app may move it). When the app moves the "1", an action's place moves with it and its controls glide to the new place over the Global glide time, as at his own Resync. A clip's own actions move when their clip falls into step with the new "1" (topic A says when a playing BPM mode clip does), so that a clip and its actions never part (assumption D3-6). [page 2, D: BF246 "app should always try to find the 1 and I will correct if necessary"]
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
STATUS: CORRECTED (he says yes and adds) -- AMENDED after page 2 (R224 1 by D, R224 1 by J, R224 2 by D)
HIS: L72, L114
RULE: (a) A saved action cannot be opened and changed: to change one he makes a new one from the recording in Studio and deletes the old one [page 2, J: BF245 "Let's go with studio. That's perfect."]. This stands by his "R224 yes. I like that." (L72); his added sentence says where actions are made and that a show is not the place to change them: "We will only allow users to record actions in the recording review screen, not modify during a show." (L72). What can be done to a saved action: switch it on and off, set its Loop toggle, Rename, Copy, Cut, Paste, Delete, "Save without ignored controls". (b) Two actions that both fire clips on the same layer: a global action wins over a layer's; of two actions of one layer the one switched on last fires, and the earlier one carries the light of R130 and fires again when the later one is off. (c) "We need a global stop actions button that stops all actions." (L72): one button on screen, also in the keyboard and MIDI mapping, for every action of every clip, every layer and global. One press switches them all off and their values glide back; he switches on again the ones he wants. That is how his page had it under the mapping (in the page's words: it switches every action off at once); he named that reading and changed another part of it only (L116). The tempo stop does the same to every action, on top of what it does to the beat and the clips: "The tempo stop button stops all actions as well as everything else." (L72); "tempo stop stops all actions, not just global" (BF250): item 226 of page 2, and 180. That both stops also switch off the buttons of clips' actions, on every clip of the show: assumption D3-1. [page 2, D: BF250 "tempo stop stops all actions, not just global"] The name of the screen where actions are made: "Review", the name he calls good -- "Show Recording Review should be called Review for short is a good name." (L114) -- and provisional, because he goes on: "Do you have a better name for this?" (L114); he has picked: "Let's go with studio. That's perfect." (BF245), and "b" under item 245 (BF263). The screen is called Studio; wherever the rules of this topic say "the review screen", "the recording review screen" or "Review", Studio is meant. [page 2, D: BF245 "Let's go with studio. That's perfect."; BF263 "b"]
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

## ASSUMPTIONS STILL OPEN (new after page 2: ids with a 3; old ones never shown to him keep their ids)
@@ASSUME D3-1
ABOUT: 226, 180, R161, R224
TEXT: I assume the tempo stop and Stop actions both switch every action button off: on every layer, on global and on every clip, also on clips that are not playing. After a stop you switch on again the ones you want.
WHY: BF250 says "all actions, not just global" and gives no letter; with "Stop removes all clips" (BF271) a clip's actions stop anyway, and what happens to their buttons is not said. "stops" also fits way c for a layer's and a global action: on the first page he took the default that keeps the buttons on. His words on a clip's buttons point to way b: "If the actions are toggled on all those actions will play when the clip plays" (2026-10-04) and "It's button stays on" (BF251). One question for both stops, because Stop actions is ruled with the same words (R224 c). The answer reaches further than the two buttons: topic E makes opening Studio a full tempo stop, and in topic B a clip previewed after a stop is shown without its actions once their buttons are off.
ALT: b) Only layer and global action buttons go off. A clip's action buttons stay as you set them: its actions stop, and play again the next time that clip is triggered. c) At the tempo stop no button goes off: every action waits and starts again from its beginning when the beat next runs.
IF-WRONG: STAGE after every stop, and after every opening of Studio (topic E), the action buttons of every clip in the show are dark and must be switched on again by hand, and a show saved then is saved that way; or they are not.
ASK: YES his words give the two stops their reach, not what they do to a clip's buttons; two sentences of his pull two ways, and he would meet the difference at every stop between two songs.
@@END

@@ASSUME D3-2
ABOUT: 227, R172, R181
TEXT: I assume your answer is about a clip's action. A play-once action on a layer or on global plays once and its button then goes off by itself; one press plays it once more.
WHY: BF251 says "neither" and then speaks of "that clip". "neither" rejects ways b and c and leaves the item's own sentence on a layer or on global standing; way c said the button "always stays on", and he did not take it. Only "It's button stays on", if he meant it for every action, would go against it.
ALT: b) Its button stays on there too; to play it again you switch it off and on. c) On a layer its button stays on and it plays again each time a clip is triggered on that layer.
IF-WRONG: STAGE one press or two to play a one-time move again; a lit or a dark button after it.
ASK: LINE he read this sentence in the item and rejected only the two other ways; he can strike it.
@@END

@@ASSUME D3-3
ABOUT: 227, R158
TEXT: I assume only a trigger of its clip plays a clip's play-once action, as you wrote. Switched on while the clip is already playing, it waits for that clip's next trigger; switching it off and on does not play it.
WHY: BF251 "plays again only when that clip is re-triggered", written in the place of way c ("only after you switch it off and on"). A first switching-on under a playing clip is not named by it, and the old timing rule would start it on the next 1.
ALT: b) Switching it on while its clip plays also plays it once, from the next 1.
IF-WRONG: STAGE a one-time move he switches on in the middle of a clip comes at the next bar, or not until he triggers the clip again.
ASK: LINE his own "only" taken one step further; the other way is the timing rule of every other action, and he can strike this with one letter.
@@END

@@ASSUME D3-4
ABOUT: 227
TEXT: I assume triggering a clip again while its play-once action is still in its one pass starts that action over from its beginning, together with the clip.
WHY: BF251 gives the re-trigger as what plays it again; a re-trigger that comes before the pass is over is not named.
ALT: b) The running pass finishes first, and that trigger does not count.
IF-WRONG: SMALL
ASK: NO the natural result of his sentence and of the accepted rule that a new trigger restarts the playing clip (item 266).
@@END

@@ASSUME D3-5
ABOUT: 180, R161, 226
TEXT: I assume a layer's Clear button (the X) does for its layer what the tempo stop does for the show: the clip goes and that layer's own actions are switched off. A clip that ejects, or an empty cell you trigger, leaves them on.
WHY: BF250 speaks of the tempo stop only. The first page told him that the X follows his answer on the stop (the page's words: "with B the X switches them off"), and the stop now switches actions off. His "143 a" (2026-10-04: a layer's action runs on across clip changes) points to way b for the X as well.
ALT: b) Clear takes only the clip off: the layer's own actions stay on and keep playing, and one that triggers clips puts a clip on that layer again at its next trigger.
IF-WRONG: STAGE a layer he cleared is filled again by its own action, or its action buttons are dark after every Clear.
ASK: YES no words of his say what Clear does to a layer's actions; the first page and his 143 a point two ways, and the audience sees the difference.
@@END

@@ASSUME D3-6
ABOUT: R181, 224
TEXT: I assume that when the app moves the 1 in automatic mode, actions follow the new 1 as after your own Resync: their sliders glide to the new place over the Global glide time, and a clip's actions stay in step with their clip.
WHY: BF246 lets the app place the 1; item 224 names a Resync as a cause of a glide, not a 1 that the app moved. Topic A's paper lets a playing BPM mode clip fall into step only on the next 1; its own actions must not part from it.
ALT: none
IF-WRONG: SMALL
ASK: NO actions sit on the beat; a 1 that moves takes them along whoever moved it. How often the app may move the 1 is topic A's.
@@END

@@ASSUME D3-7
ABOUT: 226
TEXT: I assume the tempo stop switches every action off also while Ableton Link keeps the beat running.
WHY: BF250 says the tempo stop stops all actions; with Link on the stop takes the clips off and the beat runs on (topic A).
ALT: none
IF-WRONG: SMALL
ASK: NO Link is off unless he switches it on, and his sentence has no exception.
@@END

@@ASSUME D3-8
ABOUT: 228, R160, R129
TEXT: I assume a preset loads plainly onto a slider whose only action is a layer action that a global action is holding back at that moment; the preset's value is then the value that slider goes back to later.
WHY: BF252 "b" speaks of a slider that an action is moving; a layer's action that is held back is on but moves nothing, and no rule names that case.
ALT: none
IF-WRONG: SMALL
ASK: NO a rare edge of his "b"; he would not meet it knowingly.
@@END

@@ASSUME D3-9
ABOUT: R158, R172
TEXT: I assume an action whose start comes within a tenth of a beat after the 1 starts at once and in step, as a BPM-mode clip does: with its clip, or when you press its own button a hair late.
WHY: Item 249, accepted as written, gives this to a BPM-mode clip triggered just after the 1; an action is not named, and the timing rule would make it wait for the next 1.
ALT: b) Only the clip starts at once; an action waits for the next 1.
IF-WRONG: SMALL
ASK: NO the same tolerance he accepted for a clip (item 249), by his rule that an explanation given once answers the repeats.
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
ABOUT: R172, 227
TEXT: I assume un-ticking Loop while an action plays lets it finish its pass and then go back. Ticking Loop on a play-once action lets it loop on; if its one pass is already over, it starts looping on the next 1.
WHY: L64 gives the toggle and its two states, not a change in the middle; BF251 adds a clip's play-once action that waits with its button on.
ALT: b) Un-ticking Loop ends the action at once.
IF-WRONG: SMALL
ASK: NO the natural result of his two states.
@@END

## CLOSED ASSUMPTIONS (one line each)
- D-1 -> page 2, item 224
- D-2 -> page 2, item 225
- D-3 -> page 2, item 226
- D-4 -> page 2, item 227
- D-5 -> page 2, item 228
- D-6 -> SETTLED
- D-7 -> MERGED 235
- D-8 -> SETTLED
- D-9 -> SETTLED
- D-10 -> SETTLED
- D-11 -> page 2, item 255
- D-16 -> SETTLED
- D-26 -> page 2, item 254

## QUESTIONS BACK
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

@@NAME Loop
MEANS: The toggle beside each action: ticked = the action loops; not ticked = it plays once and its controls go back.
SOURCE: his words L64 ("a loop toggle right there")
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

@@NAME Studio
MEANS: The screen where a show recording is watched and actions are made; the only place where an action is made from a recording.
SOURCE: his words (BF245: "Let's go with studio. That's perfect."; BF263: "b"); replaces the name Review in /Users/boriskarpman/projects/RealTimeAudio/.harmony/NAMES.md (topic J owns the name)
@@END

@@NAME Global glide
MEANS: The ONE slider (instant to 4 seconds) that times every smooth change of a control that an action moves.
SOURCE: his words (L64 "the Global glide slider"); that it is the only one is now his by item 224 accepted as written -- the note "one slider is assumed and asked" in /Users/boriskarpman/projects/RealTimeAudio/.harmony/NAMES.md can go
@@END

@@NAME Ignore Global Actions
MEANS: The one switch on a layer that keeps every global action away from that layer, while the layer's own actions and its clips' actions keep playing.
SOURCE: Harmony's pick for the name; what it does is his by item 225 accepted as written -- "open until you say" in /Users/boriskarpman/projects/RealTimeAudio/.harmony/NAMES.md can go
@@END

@@NAME play-once action
MEANS: An action whose Loop toggle is not ticked: it plays one pass and its controls go back; on a clip its button stays on and it plays again when the clip is triggered again.
SOURCE: Harmony's pick for the name (the page used it); what it does on a clip is his words (BF251)
@@END

@@NAME Stop actions
MEANS: The one button (and key or pad) that stops every action of every clip, every layer and global: it switches them off; the tempo stop does the same to actions. Whether that also darkens the action buttons of clips, on clips that are not playing too, is still asked.
SOURCE: his words (L72 "a global stop actions button"; BF250 for the tempo stop: "tempo stop stops all actions, not just global")
@@END

@@NAME Ignore Actions
MEANS: The toggle beside one slider or one button: lit, that one control ignores all its actions. The switch on a layer is a different thing with its own name, Ignore Global Actions.
SOURCE: his words (2026-10-04 "an ignore actions toggle"); it retires the word "ignore lamp" of the old blocks. In /Users/boriskarpman/projects/RealTimeAudio/.harmony/NAMES.md its row still says "(and on a layer)": that can go, because the layer's switch keeps only global actions away (page-2 item 225, accepted as written)
@@END

## REACHES OTHER TOPICS (from the paper)
- A (items R176 a, R177 a and the tempo stop wherever it is ruled): "stop also stops all actions" must now read: the tempo stop switches every action off, clip, layer and global (BF250, item 226); nothing starts again by itself with the beat. Open part: the clips' buttons (D3-1). Topic A's Link rule (the stop takes the clips off, the beat runs on): actions are switched off there too (D3-7).
- A: BF246 (the app finds the 1 by itself) is applied here only as: an action's place follows a 1 that the app moved, as at a Resync (R181, D3-6). Topic A rules when the app may move it.
- J (R199 d): its pointer "What a stop leaves of the action buttons ... is topic D's (D-3, D-1)" is answered: both stops switch every action off, values glide back over the one Global glide (items 226, 224). J (R199 a, and R134 here: "a key or pad put on an action stays with that action"): BF262 gives a MIDI knob on a clip's slider to the CELL; whether a key or pad on a clip's action button also belongs to the cell is not said by him and is J's to rule or ask. Not amended here.
- J and K: their reading of L7 (a layer that ignores every action) falls with item 225 accepted: one switch, against global actions only.
- E (R165, "Opening the review screen ... is like a press on stop for the live show" and "Closing it brings back the live window with the show as it was"): if opening Studio counts as a tempo stop, every action button of the live show goes off (item 226) and the show is not "as it was". Topic E must say which holds.
- B (item 220, BF247): applied here to R158 g only as a pointer: a preview by name is off the beat; whether a previewed clip's actions play in the preview, and from when, is B's. For B to know: on the output, an action of a Timeline mode clip still starts on the next 1 (R158 b).
- C (presets): after a preset is loaded under an action (item 228, BF252) the effect differs from that preset on the sliders the action held; whether the effect shows as changed from its preset, and what a save of the preset then writes for those sliders, is C's.
- F (BF256): applied here to R161 only: a clip deleted or pasted over leaves its layer, so its actions end. Whether the layer then plays the pasted clip, and a cut, are F's.
- H (item 266, accepted): a trigger of the playing clip restarts it; used in item 227 c as a re-trigger that plays a clip's play-once action again.
- F (R188 a, what a show holds): "each layer's switch against actions (L7, L73: one switch or two is topic D's)" is settled: one switch, Ignore Global Actions (item 225); one Global glide value (item 224).

## CONFLICTS (from the paper)
- The tempo stop and the action buttons. New: "tempo stop stops all actions, not just global" (BF250), written under an item that switches the actions off. Earlier: the default A of question 180 (the buttons stay ON, the actions start again with the beat), which he accepted with "All defaults good except for these." (binding-decisions.md:1105; the rule as applied: binding-decisions.md:1249). His own earlier sentence already pointed the new way: "The tempo stop button stops all actions as well as everything else." (binding-decisions.md:1155). The newest words are taken; the clips' buttons are asked (D3-1).
- A preview by name and the beat (topic B carries it; it reaches R158 g here). New: "Previewing a clip should not happen on the beat. It should just be quick" (BF247). Earlier: "a previewed clip (clicking its name) shows the clip with all its actions and it is triggered on the 1" (his L36 of 2026-10-07, binding-decisions.md:1134). The newest words are taken.
- 228: no words of his are overturned. His "b" (BF252) goes against Harmony's recommendation in the answer above the item, not against anything he said.

## NOT DONE / UNSURE (from the paper)
- D3-1 is the one real doubt of this topic: whether a clip's action buttons go off at a stop. The RULE takes the widest reading of "all actions"; his earlier sentence "If the actions are toggled on all those actions will play when the clip plays in time with the clip." (binding-decisions.md:981) points to way b. One question on his next page settles it, for the tempo stop and for Stop actions together.
- Stop actions and the clips' buttons: R224 c already switches off "every action of every clip". If he answers D3-1 with way b, R224 c needs the same change for Stop actions, or the two stops differ; the question as put covers both.
- BF256 "If a clip is playing, and I change the deck, that does not change the clip" fits R161. It was not read as going against his 182 b (a running layer action takes the cell of the deck on screen at its NEXT trigger, so the picture can change one trigger after a deck switch). If he meant more, 182 would have to be put to him again; cheapest: one line on his next page only if topic F sees the same doubt.
- Where the old blocks of this topic say "the review screen" or "Review" (R129, R131 d, R162, R181 e, R143, D13, R224 a, the NAME "action save window"), only R224 was amended, with one sentence that covers them all; a merge script may want to replace the words everywhere.
- The old word "fire / fires" in the old RULE lines is the retired word for "trigger" (NAMES.md); not amended, it changes no rule.
- Every TODAY line is taken from the old blocks and the two fact sheets; no program text was read for this paper, and nothing was run.
STAMP: 2026-10-09 18:49:52 EDT

## FOR THE PAGE RULING (from the ruling)
- MUST BE PUT TO HIM, heaviest first: D3-1 (what the tempo stop and Stop actions do to the action buttons, those of clips above all; one question for both stops; its way c is page 2's way b of item 226, kept because he gave no letter). Then D3-5 (the Clear button and the layer's own actions), right after D3-1: it follows the stop, and if he answers D3-1 with c its default falls back to its way b.
- ONE LINE EACH: D3-3 (a clip's play-once action that is switched on under a playing clip waits for that clip's next trigger: his "only"), D3-2 (on a layer or on global a play-once action's button goes off by itself).
- D3-5's DEFAULT WAS FLIPPED by this ruling against the paper and the checker (both had "the layer's actions stay on", LINE): R161 and question 180 of the first page told him the X follows the stop. If the page ruling prefers the paper's default, TEXT and way b of D3-5 change places, and the NEW lines of AMEND 180 4 and AMEND R161 2 with them; the question is YES either way.
- TO SAY ONCE MORE IN ONE LINE: the tempo stop switches actions off -- "tempo stop stops all actions, not just global" (BF250), written under an item that switches them off -- against the default he accepted on 2026-10-07 (the buttons stay on and the actions start again with the beat). The preview by name off the beat (BF247 against "it is triggered on the 1", BF175) is topic B's line.
- TOPIC A: its ruling already points R176 (a) and R214 (a) to item 226 for what a stop does to actions (its AMEND R176 1 and AMEND R214 2); nothing more is owed there. How often the app may move the "1" (BF246; A's A3-1 and A3-2) decides how often running actions glide (D3-6); A leaves to this topic how actions take a "1" that moved, and AMEND R181 2 says it. Item 249's tenth of a beat, which A's ruling also gives an empty cell, is now used for actions too (AMEND R158 3, D3-9).
- TOPIC B: R158 (d) and (e) now speak of the output only; a previewed clip's actions are B's (items 220 and 221, B3-3). What a preview by name shows after a stop hangs on D3-1: if every clip's buttons go off, a clip previewed after a stop is shown without its actions. B's ruling asks for one clause on this in D's question; it is left to the page ruling (D3-1's TEXT stands at 41 words).
- TOPIC E: its ruling makes opening Studio a full tempo stop (its AMEND R165 2; its E3-6, NO: nothing comes on again by itself when Studio is closed) and leaves the action buttons to this topic. With D3-1 at its default, every opening of Studio darkens every action button of the live show, those of every clip too, also when he opens Studio only to cut one action. So D3-1 decides E3-6 with it: if D3-1 stays at its default, E3-6 is no longer a plain NO (advice: closing Studio brings the action buttons back as they were). Item 230 changes no block here.
- TOPIC F: a snapshot opened in the middle of an action (item 236) is the tempo-pause state for actions (R158 e: an action holds its place and carries on at tempo play): that is the action "taken up part-way" that F asks for, and R132 stands (a control that an action moves is saved at the value he set). F's item 234 ends a clip's actions "as at an X": read that for the clip's own actions only, because by D3-5's default the X also switches the layer's own actions off and an edit does not (AMEND R161 3).
- TOPICS J, K, C, X: J's R199 (d) pointer is answered by items 226 and 224, and a key or pad on an action's button stays with the action (J's own line). K: a trigger by the Autopilot plays a clip's play-once action (now named in item 227 c). C: its Save in the header after a load under an action fits item 228. X: its own paper closed X-27 and X-3 by items 224 and 225 and amended its C9; nothing is owed (checker F6).
- NAMES: in everything he reads the layer's X is "the Clear button (the X)". The row Ignore Actions of NAMES.md loses "(and on a layer)". "Ignore Global Actions" is still Harmony's pick for the name of the layer's switch; what the switch does is his by item 225.
- A STALE NOTE, not a rule: the TODAY line of old item 193 ("Keying and its slider are being removed") is wrong after BF260 ("All the blend modes and keying stay"); topic I owns it.
- TOPIC D AFTER THIS RULING: 10 assumptions -- 2 YES (D3-1, D3-5), 2 LINE (D3-3, D3-2), 6 NO (D3-4, D3-6, D3-7, D3-8, D3-9, D-31). No item of the topic is OPEN; no answer is owed. The 16 other old assumptions that he never saw (D-13 to D-30) were tested again against his 33 boxes and the 57 items: none is settled or contradicted, so none is written again.
STAMP: 2026-10-09 19:33:47 EDT
STATUS: DONE

