# SLICE D -- Actions in a show: page 2 as Boris saw it, and what he did with it (made by wf/mkslices3.py; never edit by hand)

## HIS BOXES THAT LAND IN THIS TOPIC (3; his words verbatim, with the BF number of the filing and the line of his file)

@@BOX 226
BF: BF250
FILE-LINE: 38
PAGE-ITEM: [226] I assume the tempo stop also switches every layer and global action off, as Stop actions does; you switch on again the ones you want.
PAGE-WAY-b: Their buttons stay on: the actions stand still and start again from their beginning when the beat next runs.
HIS-WORDS: tempo stop stops all actions, not just global
@@END

@@BOX 227
BF: BF251
FILE-LINE: 41
PAGE-ITEM: [227] I assume a play-once action on a clip plays once every time that clip is triggered. On a layer or on global it plays once, and its button then goes off by itself.
PAGE-WAY-b: On a clip too, its button goes off after one play; you switch it on again yourself.
PAGE-WAY-c: Its button always stays on; it plays again only after you switch it off and on.
HIS-WORDS: neither. It's button stays on and that action plays again only when that clip is re-triggered
@@END

@@BOX 228
BF: BF252
FILE-LINE: 44
PAGE-ITEM: [228] I assume that when you load a preset onto an effect while an action is moving one of its sliders, that slider keeps moving with the action, and when the action goes off it glides to the preset's value.
PAGE-WAY-b: It glides back to the value it had before the action; the preset's value for it is lost.
HIS-WORDS: b
@@END

## THE PAGE-2 ITEMS OF THIS TOPIC (7), each as he read it

### [224] (a card of its own)
TEXT: I assume ONE slider, Global glide (instant to 4 seconds), sets how long every smooth change of an action takes: when it starts, goes off, has played once, takes over from another action, or after a Resync.
B: Two sliders: one for how actions start, one for how values go back.
C: none
MADE FROM (blocks of the s-rta-1007 specs; an id like D-4 is an @@ASSUME, any other id is an @@ITEM): D-1, X-27, 183, 213, R147
HE LEFT ITS BOX EMPTY = ACCEPTED AS WRITTEN

### [225] (a card of its own)
TEXT: I assume a layer gets ONE new switch, like Ignore Column Trigger: while it is on, global actions leave that layer alone. The layer's own actions and its clips' actions keep playing.
B: One switch, and it makes the layer ignore every action, its own and its clips' too.
C: Two switches: one against global actions, one against all actions.
MADE FROM (blocks of the s-rta-1007 specs; an id like D-4 is an @@ASSUME, any other id is an @@ITEM): D-2, X-3, G5, 179
HE LEFT ITS BOX EMPTY = ACCEPTED AS WRITTEN

### [226] (a card of its own)
TEXT: I assume the tempo stop also switches every layer and global action off, as Stop actions does; you switch on again the ones you want.
B: Their buttons stay on: the actions stand still and start again from their beginning when the beat next runs.
C: none
MADE FROM (blocks of the s-rta-1007 specs; an id like D-4 is an @@ASSUME, any other id is an @@ITEM): D-3, 180, R224
HE WROTE IN ITS BOX (BF250): "tempo stop stops all actions, not just global"

### [227] (a card of its own)
TEXT: I assume a play-once action on a clip plays once every time that clip is triggered. On a layer or on global it plays once, and its button then goes off by itself.
B: On a clip too, its button goes off after one play; you switch it on again yourself.
C: Its button always stays on; it plays again only after you switch it off and on.
MADE FROM (blocks of the s-rta-1007 specs; an id like D-4 is an @@ASSUME, any other id is an @@ITEM): D-4
HE WROTE IN ITS BOX (BF251): "neither. It's button stays on and that action plays again only when that clip is re-triggered"

### [228] (a card of its own)
TEXT: I assume that when you load a preset onto an effect while an action is moving one of its sliders, that slider keeps moving with the action, and when the action goes off it glides to the preset's value.
B: It glides back to the value it had before the action; the preset's value for it is lost.
C: none
MADE FROM (blocks of the s-rta-1007 specs; an id like D-4 is an @@ASSUME, any other id is an @@ITEM): D-5, R129
HE WROTE IN ITS BOX (BF252): "b"

### [254] (one line in the small list)
TEXT: I assume a clip that an action triggered stays on its layer when the action ends; only sliders, buttons and menus go back.
B: The layer goes back to the clip that played before the action.
C: none
MADE FROM (blocks of the s-rta-1007 specs; an id like D-4 is an @@ASSUME, any other id is an @@ITEM): D-26
HE LEFT ITS BOX EMPTY = ACCEPTED AS WRITTEN

### [255] (one line in the small list)
TEXT: I assume a control's ignore actions toggle, switched on while an action moves it, sends the control gliding back to where it was before the action.
B: The control stays where the action has it at that moment, so you can catch a value.
C: none
MADE FROM (blocks of the s-rta-1007 specs; an id like D-4 is an @@ASSUME, any other id is an @@ITEM): D-11, R131
HE LEFT ITS BOX EMPTY = ACCEPTED AS WRITTEN

## HARMONY'S ANSWERS ON PAGE 2 THAT BELONG TO THIS TOPIC (1), each as he read it

### answer R129-4
HE HAD ASKED: "please explain in more detail about loading a preset onto an effect with an action playing" [L66]
TITLE: A preset loaded while an action plays
TEXT: My recommendation: loading a preset never stops the action, and the preset is never lost. Example: an effect has sliders A, B and C, and an action is moving A. You load a preset. B and C take the preset's values at once. A keeps moving with the action. The preset's value for A waits underneath: when the action goes off, or has played once, A glides to the preset's value, not to the value from before the action. The other way: the preset's value for A is thrown away.
LEADS TO ITEM: 228
HE LEFT ITS BOX EMPTY (no comment)

## THE OLD ASSUMPTIONS OF THIS TOPIC (30 @@ASSUME blocks in spec-D.md of s-rta-1007) AND WHERE EACH WENT
A number = it became that item of page 2 (now answered or accepted: closed). SETTLED / MERGED = closed before page 2. INTERNAL = never shown to him: still Harmony's own, and STILL OPEN -- test each against his new words.

- D-1 -> 224
- D-2 -> 225
- D-3 -> 226
- D-4 -> 227
- D-5 -> 228
- D-6 -> SETTLED: L68: "that specific action is muted": the whole action; that it wakes when the part is added is internal.
- D-7 -> MERGED 235: The same question as F-13: which old files go to the Trash (L69).
- D-8 -> SETTLED: L73: the bypass he gives a layer only has a job if the pick is one global action.
- D-9 -> SETTLED: L71 names the reading and changes only the colours; its part on Ignore Column Trigger stands (L1).
- D-10 -> SETTLED: L74 (182 b: the deck that is shown, other pictures after a deck switch) with L11 and L19.
- D-11 -> 255
- D-13 -> INTERNAL: L70 names starts, stops and a Resync; what was recorded plays as recorded.
- D-14 -> INTERNAL: Follows L70 with the rule that every action starts on a 1; revisited with his answer to 224.
- D-15 -> INTERNAL: A first state that follows his earlier answer that actions loop.
- D-16 -> SETTLED: L65: "show a warning"; a warning is not a refusal.
- D-17 -> INTERNAL: Follows his One Save rule; he sets the slider himself.
- D-18 -> INTERNAL: His paste rule (L68) one case further; follows his answer on item 255.
- D-19 -> INTERNAL: Any other behaviour would be a fault he would report.
- D-20 -> INTERNAL: Technical: stored inside the action, shown nowhere.
- D-21 -> INTERNAL: Follows the default he took that a copied clip keeps its actions.
- D-22 -> INTERNAL: Internal logic: otherwise an action could never un-pause its clip.
- D-23 -> INTERNAL: A detail of the save window in Review (L55): he sees the list of actions before anything is saved.
- D-24 -> INTERNAL: Technical edge: a column the deck on screen does not have.
- D-25 -> INTERNAL: Technical: the arithmetic of a glide.
- D-26 -> 254
- D-27 -> INTERNAL: Follows L70: nothing an action moves may jump.
- D-28 -> INTERNAL: Internal logic: otherwise an action could switch itself off or lock itself out.
- D-29 -> INTERNAL: How every clipboard works; both ways to paste an action were on his page.
- D-30 -> INTERNAL: A look; the action buttons take their place (L8).
- D-31 -> INTERNAL: The natural result of the two states of L64.

## IDS
PAGE-2 ITEMS: 224 225 226 227 228 254 255
BOXES: BF250(226) BF251(227) BF252(228)
ANSWERS OWED BY THIS TOPIC'S PAPER (an @@ANSWER block each): none
OLD @@ITEM IDS IN spec-D.md: 179 180 181 182 183 184 185 193 194 213 R133 R134 R172 R158 R144 R159 R129 R130 R131 R132 R160 R161 R145 R162 R146 R143 R147 R181 R182 R224 D10 D11 D12 D13 D14 D15 P10 P11 P12 P13 P14 P15 P16 G5
OLD @@ASSUME IDS STILL OPEN (INTERNAL): D-13 D-14 D-15 D-17 D-18 D-19 D-20 D-21 D-22 D-23 D-24 D-25 D-27 D-28 D-29 D-30 D-31
