# APPLY D -- Actions in a show (s-rta-1009, page 2)
## SUMMARY
- 226 (BF250): the tempo stop switches EVERY action off, as Stop actions does -- clip, layer and global. The old default (buttons stay on, actions start again with the beat) is gone. Open and asked once: whether the buttons of clips' actions really go off too, or stay on for the next trigger of their clip (D3-1).
- 227 (BF251): a play-once action on a clip keeps its button ON and plays again only when its clip is triggered again. Both other ways are rejected. His sentence speaks of a clip; the layer and global half of the item is kept as he read it and asked once (D3-2).
- 228 (BF252, "b"): a preset loaded under a running action is LOST on the sliders the action is moving; they glide back to the value from before the action. Harmony's recommendation (the preset waits underneath) falls.
- 224, 225, 254, 255 accepted as written: ONE Global glide slider; ONE layer switch against global actions only; a clip an action triggered stays; an Ignore Actions toggle switched on under an action sends the control back.
- Applied from other boxes: Studio is the screen's name (BF245); a preview by name is not on the beat (BF247) -- R158 g; a clip deleted or pasted over leaves its layer (BF256) -- R161; the app moves the 1 by itself (BF246) -- R181.
- 7 items, 14 amendments, 8 assumptions (2 asked, 2 one-liners, 4 internal), no drop.
## ITEMS
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
TITLE: A play-once action on a clip keeps its button on
STATUS: ANSWERED in his words: neither of the two other ways. His sentence speaks of a clip; the layer and global half is not in it
HIS: BF251 "neither. It's button stays on" and "plays again only when that clip is re-triggered"
RULE: A play-once action (its Loop toggle not ticked) ON A CLIP: (a) It plays once, from its beginning, each time its clip is triggered, by the timing of @@ITEM R158 as it stands (a BPM mode clip: clip and action start together on the "1"; a Timeline mode clip: the picture at the trigger, the action on the next "1"). (b) After its one pass its controls go back to where they were before it, over the Global glide time (R172), and ITS BUTTON STAYS ON. The button never goes off by itself. (c) While its button is on, the action plays again ONLY when that clip is triggered again. A trigger is any trigger: a click, a key, a pad, a column trigger, a layer's or a global action that triggers its cell (R133 c), and a trigger of the clip while it is already playing, which restarts the clip (item 266, accepted as written; topic H). Nothing else plays it again: not the clip coming round at its own length, not a Resync, not a tempo pause followed by play, not any later "1". (d) The two other ways do not hold: the button does not go off after one play, and switching it off and on is not the only way to play it again. (e) Switched on by hand while its clip is already playing, it plays once from the next "1" (R158 c; assumption D3-3). (f) Its clip triggered again while the pass is still running: the action starts over from its beginning at the start that trigger gives it (assumption D3-4). (g) Its clip leaves during the pass (a layer's X, another clip, an eject, a delete): the action ends with its clip, its controls go back (R161), and its button stays on for the next trigger. At a tempo stop or at Stop actions its button goes off like every other (item 226). (h) A LOOPING action of a clip is not changed: a new trigger of its clip never restarts it (R181 b). A play-once action ON A LAYER OR ON GLOBAL: his sentence does not speak of it; the item's sentence as he read it is kept -- it plays once from the next "1" after it is switched on, its controls go back, and its button then goes off by itself; one press switches it on and plays it once more (assumption D3-2).
CHANGED: Against the item as he read it: the clip half is his now, and sharper -- the button stays on (the item did not say so outright), and "only when that clip is re-triggered" rules out every other cause of a second play. Ways b and c are rejected by "neither". The layer and global half is neither confirmed nor rejected by a sentence about "that clip": it is kept as written and asked (D3-2). INFERRED: "re-triggered" is read as any trigger of the clip, also by an action or a column trigger. Against the old blocks: the pointers to the old question D-4 in R172 and R181 are replaced; D-4 is closed.
TODAY: A routine has a loop / once field in its pad menu and belongs to the show, not to a clip (area-actions.md 1.2, 1.3, 1.13; from the old block R172, not re-read). To build: per action a Loop toggle, and for a clip's play-once action a start at every trigger of its clip.
@@END
@@ITEM 228
TITLE: A preset loaded under an action is lost on the moving sliders
STATUS: ANSWERED b
HIS: BF252 "b"
RULE: A preset is loaded onto an effect while an action is moving one or more of that effect's controls. (a) The load never stops, pauses or switches off the action. (b) Every control of the effect that no action is moving at that moment takes the preset's value at once. (c) Every control that an action is moving at that moment keeps moving with the action. The preset's value for that control is not kept anywhere: it is thrown away at the load. (d) When the action lets go of that control -- it is switched off, it has played once, its clip leaves, a stop, the control's Ignore Actions toggle is lit -- the control glides back, over the Global glide time, to the value it had before the action: the same value as if no preset had been loaded. (e) "Moving at that moment" means: an action that is on and playing holds the control, also where its recorded line is flat (a ticked row that stays flat holds its control, R172). A control has no action on it at that moment when the action is off, when a clip's play-once action has finished its pass and waits with its button on (item 227), when a layer action is held back by a global action that does not move that control, or when the control's Ignore Actions toggle is lit: there the preset loads plainly, and the preset's value is from then on the value "from before" for the next time an action takes the control. (f) On / off buttons and menus of the effect that the action is moving follow the same rule and switch back at once (assumption D-13). (g) This is the same rule as for the hand (R129 point 2): nothing that is set on a control while an action moves it lasts. (h) After such a load the effect differs from the loaded preset on the controls the action held; whether the effect then shows as changed from its preset is topic C's.
CHANGED: His "b" goes against the recommendation Harmony gave in the answer above the item (the preset waits underneath and the slider glides to it): the recommendation falls, and the item's main text with it. Point 4 of the old block R129 is replaced; the old question D-5 is closed. The page's own first reading of 2026-10-05 (the preset's values are thrown away on the sliders the action moves) is thereby what holds.
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
## AMENDMENTS
@@AMEND 180 1
OLD: Option A in full: on the tempo row's stop a layer's and the global actions stop with the beat and their buttons stay ON; their controls stay where they stood (a fader at 0 stays at 0). When the beat runs again they start from their beginning on the "1", and one that fires clips fires them again.
NEW: The tempo stop switches every action off, as Stop actions does (item 226 of page 2 holds the rule in full): the actions of every layer and of global are switched off, their buttons go off, and every control they moved goes back to where it was before the action, over the Global glide time. Nothing starts again by itself when the beat next runs: he switches on again the ones he wants, and an action switched on while the beat is stopped starts from its beginning on the "1" that starts the beat.
HIS: BF250 "tempo stop stops all actions, not just global"
WHY: The default A kept the buttons on and let the actions start again with the beat; he wrote under an item that switches them off and widened it instead of taking the way that keeps them on.
@@END
@@AMEND 180 2
OLD: A clip's own actions end with their clip, because the stop takes every clip off (R161).
NEW: A clip's own actions end with their clip, because the stop takes every clip off (R161; "Stop removes all clips from all layers", BF271), and their buttons go off as well, because he says "all actions" (BF250); that the buttons of clips' actions go off and do not stay on for the next trigger of their clip is assumption D3-1.
HIS: BF250 "tempo stop stops all actions, not just global"; BF271 "Stop removes all clips from all layers"
WHY: The old sentence left the clips' action buttons untouched; "all actions" reaches them.
@@END
@@AMEND 180 3
OLD: say that they stop; they do not say whether their buttons go off.
NEW: say that they stop, and his words of 2026-10-09 say it again for every kind of action: "tempo stop stops all actions, not just global" (BF250), written under an item that switches the actions off; so the buttons go off.
HIS: BF250 "tempo stop stops all actions, not just global"
WHY: The old sentence held the question open; item 226 and his box close it.
@@END
@@AMEND 180 4
OLD: A layer's X follows from A: the layer's own actions stay on and keep running with the beat.
NEW: A layer's X is not the tempo stop: it takes the layer's clip off, and the layer's own actions stay on and keep running with the beat (assumption D3-5).
HIS: BF250 "tempo stop stops all actions, not just global"
WHY: The sentence drew the X from default A, which no longer holds; the X now stands on its own, as an assumption.
@@END
@@AMEND R161 1
OLD: At a tempo stop every clip leaves, so every clip's actions end and their controls go back.
NEW: At a tempo stop every clip leaves ("Stop removes all clips from all layers", BF271; topic A rules it), so every clip's actions end and their controls go back; the stop also switches every action's button off, a clip's, a layer's and a global one alike (item 226; the clips' buttons: assumption D3-1).
HIS: BF250 "tempo stop stops all actions, not just global"; BF271 "Stop removes all clips from all layers"
WHY: The old sentence ended the clips' actions and said nothing of any button; the stop now switches every action off.
@@END
@@AMEND R161 2
OLD: because they belong to the layer (180 at its default A; assumption D-3)
NEW: because they belong to the layer, and an X or an eject is not the tempo stop (assumption D3-5)
HIS: BF250 "tempo stop stops all actions, not just global"
WHY: The reason given was the default A of 180, which his box replaces; the rule for an X is kept as an assumption of its own.
@@END
@@AMEND R161 3
OLD: APPEND
NEW: A clip that is deleted or pasted over while it plays leaves its layer and does not play (his BF256; topic F rules it): its actions end with it as at an X, and a deleted clip's actions go with it (D11).
HIS: BF256 "removes it from the layer strip, and it does not play"
WHY: The old rule named an X, a clip change and an eject as what ends a clip's actions; his words add a delete and a paste-over.
@@END
@@AMEND R224 1
OLD: It is put to him once more together with the tempo stop: assumption D-3. "The tempo stop button stops all actions as well as everything else." (L72): see 180.
NEW: The tempo stop does the same to every action, on top of what it does to the beat and the clips: "The tempo stop button stops all actions as well as everything else." (L72); "tempo stop stops all actions, not just global" (BF250): item 226 of page 2, and 180. That both stops also switch off the buttons of clips' actions, on every clip of the show: assumption D3-1.
HIS: BF250 "tempo stop stops all actions, not just global"
WHY: The old sentence held open what the two stops do to the buttons; he let the item's "switches off, as Stop actions does" stand and widened it to all actions.
@@END
@@AMEND R224 2
OLD: topic J answers that, and until he picks the screen is written "Review".
NEW: he has picked: "Let's go with studio. That's perfect." (BF245), and "b" under item 245 (BF263). The screen is called Studio; wherever the rules of this topic say "the review screen", "the recording review screen" or "Review", Studio is meant.
HIS: BF245 "Let's go with studio. That's perfect."; BF263 "b"
WHY: The name Review was provisional until he picked; he picked Studio.
@@END
@@AMEND R172 1
OLD: What the button of a play-once action does afterwards, and when it plays again: assumption D-4.
NEW: A play-once action on a clip keeps its button on after its pass and plays again only when its clip is triggered again: "It's button stays on and that action plays again only when that clip is re-triggered" (BF251; item 227 of page 2 holds the rule in full). A play-once action on a layer or on global plays once and its button then goes off by itself; one press plays it once more (the item's sentence as he read it; his words do not speak of it: assumption D3-2).
HIS: BF251 "It's button stays on" and "only when that clip is re-triggered"
WHY: The old sentence pointed to an open question; his box answers it for a clip's action.
@@END
@@AMEND R181 1
OLD: (a play-once action: assumption D-4)
NEW: (a play-once action on a clip: the clip coming round at its own length never plays it again; a new trigger of its clip does, and nothing else does -- item 227, BF251)
HIS: BF251 "plays again only when that clip is re-triggered"
WHY: The old clause pointed to an open question; his box answers it.
@@END
@@AMEND R181 2
OLD: APPEND
NEW: (i) In automatic mode the app also looks for the "1" by itself and he corrects it with Resync (his BF246; topic A rules it). When the app moves the "1", an action's place moves with it exactly as at his own Resync, and its controls glide to the new place over the Global glide time (assumption D3-6).
HIS: BF246 "app should always try to find the 1 and I will correct"
WHY: The old rule knew only his own Resync as what moves the "1"; the app may now move it too.
@@END
@@AMEND R129 1
OLD: (4) A preset loaded onto an effect while an action plays: the recommendation is that the preset is kept underneath -- the sliders the action is moving stay with the action and glide to the preset's values when it goes off or has played once; the other sliders take the preset at once (assumption D-5; not his word yet: he asked for the explanation, answer R129-4).
NEW: (4) A preset loaded onto an effect while an action plays: the sliders the action is moving stay with the action, and the preset's values for them are lost; when the action goes off or has played once they glide back to the values they had before the action, over the Global glide time. The other sliders take the preset at once (his "b", BF252; item 228 of page 2 holds the rule in full).
HIS: BF252 "b"
WHY: The old point carried Harmony's recommendation that the preset waits underneath; he chose the other way.
@@END
@@AMEND R158 1
OLD: (g) A clip previewed by its name plays its own actions for the preview, starting on the "1": "a previewed clip (clicking its name) shows the clip with all its actions and it is triggered on the 1" (L36); how the preview does that is topic B's.
NEW: (g) A clip previewed by its name starts in the preview at once, not on the beat: "Previewing a clip should not happen on the beat. It should just be quick" (BF247); this replaces "it is triggered on the 1" (L36). Whether its own actions play in that preview, and from when, is topic B's (item 220). A clip that is loaded on its layer and cued there plays in time, and its actions follow (a) to (c): "If the clip is loaded into the layer, and we are cueing this way, then it should play in time" (BF247).
HIS: BF247 "Previewing a clip should not happen on the beat. It should just be quick"
WHY: The old point started a previewed clip and its actions on the "1" by his L36; his newer words take the preview by name off the beat.
@@END
## ASSUMPTIONS
@@ASSUME D3-1
ABOUT: 226, 180, R161, R224
TEXT: I assume the tempo stop switches every action off, as Stop actions does: the actions of every clip too, playing or not. After a stop you switch on again the ones you want.
WHY: BF250 says "all actions, not just global" and names no letter. With "Stop removes all clips" (BF271) a clip's actions stop anyway; whether their buttons go off is not said.
ALT: b) Only layer and global action buttons go off; a clip's action buttons stay on, so its actions play again when the clip is next triggered. c) No button goes off: every action stands still and starts again when the beat next runs.
IF-WRONG: STAGE after every stop all action buttons of all clips are dark and must be switched on again by hand, or they are not.
ASK: YES his words widen the item but do not say what happens to a clip's buttons; his earlier sentence on clips' actions ("If the actions are toggled on all those actions will play when the clip plays") points to way b.
@@END
@@ASSUME D3-2
ABOUT: 227, R172, R181
TEXT: I assume your answer is about a clip's action. A play-once action on a layer or on global plays once and its button then goes off by itself; one press plays it once more.
WHY: BF251 says "neither" and then speaks of "that clip"; the item's sentence on a layer or global is neither confirmed nor rejected by it.
ALT: b) Its button stays on there too; to play it again you switch it off and on. c) On a layer its button stays on and it plays again each time a clip is triggered on that layer.
IF-WRONG: STAGE one press or two to play a one-time move again; a lit or a dark button after it.
ASK: YES "It's button stays on" can be meant for every play-once action or for a clip's only, and the two differ under his hands.
@@END
@@ASSUME D3-3
ABOUT: 227, R158
TEXT: I assume a play-once action that you switch on while its clip is already playing plays once from the next 1. After that, only triggering the clip again plays it.
WHY: BF251 says it plays again "only when that clip is re-triggered"; the first play after switching it on under a playing clip is not named.
ALT: b) Switching it on plays nothing: it waits for the next trigger of its clip.
IF-WRONG: STAGE a one-time move he switches on in the middle of a clip comes at the next bar, or not until he triggers the clip again.
ASK: LINE it follows the standing rule that an action switched on under a playing clip starts on the next 1; he can strike it.
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
TEXT: I assume a layer's X takes only the clip off. The layer's own actions stay on and keep playing, and one that triggers clips puts a clip on that layer again at its next trigger.
WHY: BF250 speaks of the tempo stop only. The old rule for the X leaned on the default that the tempo stop keeps the buttons on, which is gone.
ALT: b) The X also switches off that layer's own actions.
IF-WRONG: STAGE a layer he cleared with X is filled again by its action, or its action is dark after the X.
ASK: LINE a layer's actions belong to the layer and the X is the clip's; he can strike it in one word.
@@END
@@ASSUME D3-6
ABOUT: R181, 224
TEXT: I assume that when the app moves the 1 by itself in automatic mode, actions move with it as after your own Resync: their sliders glide to the new place over the Global glide time.
WHY: BF246 lets the app place the 1; item 224 names a Resync as a cause of a glide, not a 1 that the app moved.
ALT: none
IF-WRONG: SMALL
ASK: NO actions sit on the beat; a 1 that moves takes them along whoever moved it.
@@END
@@ASSUME D3-7
ABOUT: 226
TEXT: I assume the tempo stop switches every action off also while Ableton Link keeps the beat running.
WHY: BF250 says the tempo stop stops all actions; with Link on the stop takes the clips off and the beat runs on (topic A).
ALT: none
IF-WRONG: SMALL
ASK: NO Link is off unless he switches it on, and his sentence has no exception.
@@END
@@ASSUME D-31
ABOUT: R172, 227
TEXT: I assume un-ticking Loop while an action plays lets it finish its pass and then go back. Ticking Loop on a play-once action lets it loop on; if its one pass is already over, it starts looping on the next 1.
WHY: L64 gives the toggle and its two states, not a change in the middle; BF251 adds a clip's play-once action that waits with its button on.
ALT: b) Un-ticking Loop ends the action at once.
IF-WRONG: SMALL
ASK: NO the natural result of his two states.
@@END
## QUESTIONS BACK
(none owed by this topic: he asked nothing back in boxes 226, 227 and 228)
## NAMES
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
MEANS: The one button (and key or pad) that switches every action of every clip, every layer and global off; the tempo stop does the same to actions.
SOURCE: his words (L72 "a global stop actions button"; BF250 for the tempo stop)
@@END
## REACHES OTHER TOPICS
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
## CONFLICTS
- The tempo stop and the action buttons. New: "tempo stop stops all actions, not just global" (BF250), written under an item that switches the actions off. Earlier: the default A of question 180 (the buttons stay ON, the actions start again with the beat), which he accepted with "All defaults good except for these." (binding-decisions.md:1105; the rule as applied: binding-decisions.md:1249). His own earlier sentence already pointed the new way: "The tempo stop button stops all actions as well as everything else." (binding-decisions.md:1155). The newest words are taken; the clips' buttons are asked (D3-1).
- A preview by name and the beat (topic B carries it; it reaches R158 g here). New: "Previewing a clip should not happen on the beat. It should just be quick" (BF247). Earlier: "a previewed clip (clicking its name) shows the clip with all its actions and it is triggered on the 1" (his L36 of 2026-10-07, binding-decisions.md:1134). The newest words are taken.
- 228: no words of his are overturned. His "b" (BF252) goes against Harmony's recommendation in the answer above the item, not against anything he said.
## NOT DONE / UNSURE
- D3-1 is the one real doubt of this topic: whether a clip's action buttons go off at a stop. The RULE takes the widest reading of "all actions"; his earlier sentence "If the actions are toggled on all those actions will play when the clip plays in time with the clip." (binding-decisions.md:981) points to way b. One question on his next page settles it, for the tempo stop and for Stop actions together.
- Stop actions and the clips' buttons: R224 c already switches off "every action of every clip". If he answers D3-1 with way b, R224 c needs the same change for Stop actions, or the two stops differ; the question as put covers both.
- BF256 "If a clip is playing, and I change the deck, that does not change the clip" fits R161. It was not read as going against his 182 b (a running layer action takes the cell of the deck on screen at its NEXT trigger, so the picture can change one trigger after a deck switch). If he meant more, 182 would have to be put to him again; cheapest: one line on his next page only if topic F sees the same doubt.
- Where the old blocks of this topic say "the review screen" or "Review" (R129, R131 d, R162, R181 e, R143, D13, R224 a, the NAME "action save window"), only R224 was amended, with one sentence that covers them all; a merge script may want to replace the words everywhere.
- The old word "fire / fires" in the old RULE lines is the retired word for "trigger" (NAMES.md); not amended, it changes no rule.
- Every TODAY line is taken from the old blocks and the two fact sheets; no program text was read for this paper, and nothing was run.
STAMP: 2026-10-09 18:49:52 EDT
