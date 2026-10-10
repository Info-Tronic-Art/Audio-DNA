# SLICE B -- The cue system: page 2 as Boris saw it, and what he did with it (made by wf/mkslices3.py; never edit by hand)

## HIS BOXES THAT LAND IN THIS TOPIC (3; his words verbatim, with the BF number of the filing and the line of his file)

@@BOX 220
BF: BF247
FILE-LINE: 29
PAGE-ITEM: [220] I assume a clip you preview by its name waits for the next 1 before it starts, in BPM mode or not, so that its actions play in time with the music.
PAGE-WAY-b: Only a BPM-mode clip waits; any other clip starts at once, and its actions join on the next 1.
HIS-WORDS: I think I want to change this. Previewing a clip should not happen on the beat. It should just be quick so the user could go through any amount of previews as quick as they want and only when they trigger and play should they be on time with the beat. If the clip is loaded into the layer, and we are cueing this way, then it should play in time
@@END

@@BOX 222
BF: BF248
FILE-LINE: 32
PAGE-ITEM: [222] I assume master cue only shows in the preview what global does on the output at that moment: the global effects, moved by any global action that runs. It is not a way to try a global effect or action unseen.
PAGE-WAY-b: Master cue also lets you switch a global effect or action on for the preview only, to try it first.
PAGE-WAY-c: Master cue shows the whole output in the preview, as on a DJ mixer.
HIS-WORDS: what and where is the master cue? The layers have a cue button to display in the preview/cue monitor but where is the cue button and where would the master cue be displayed?
@@END

@@BOX 252
BF: BF267
FILE-LINE: 89
PAGE-ITEM: [252] I assume the output always comes first: the preview may be smaller, less smooth and, in the first build, show trails (Echo, Freeze, Feedback) and MilkDrop a little differently.
HIS-WORDS: What do you mean by come first? Playing something smaller or less smooth may create problems. What do you think? You understand how the system needs to be built better than me. Of course the output is more important, but the output preview screen which we have and the preview-cure screen below it should all run at the same fps, no?
@@END

## THE PAGE-2 ITEMS OF THIS TOPIC (5), each as he read it

### [220] (a card of its own)
TEXT: I assume a clip you preview by its name waits for the next 1 before it starts, in BPM mode or not, so that its actions play in time with the music.
B: Only a BPM-mode clip waits; any other clip starts at once, and its actions join on the next 1.
C: none
MADE FROM (blocks of the s-rta-1007 specs; an id like B-4 is an @@ASSUME, any other id is an @@ITEM): B-1, X-29, R151, R179
HE WROTE IN ITS BOX (BF247): "I think I want to change this. Previewing a clip should not happen on the beat. It should just be quick so the user could go through any amount of previews as quick as they want and only when they trigger and play should they be on time with the beat. If the clip is loaded into the layer, and we are cueing this way, then it should play in time"

### [221] (a card of its own)
TEXT: I assume that while the tempo is paused or stopped, a clip you preview by its name still plays in the preview, with its actions, at the tempo's BPM, from your click. Previewing never starts the tempo.
B: It waits on its first frame until the tempo runs again and reaches the 1.
C: none
MADE FROM (blocks of the s-rta-1007 specs; an id like B-4 is an @@ASSUME, any other id is an @@ITEM): B-2, X-24, U18
HE LEFT ITS BOX EMPTY = ACCEPTED AS WRITTEN

### [222] (a card of its own)
TEXT: I assume master cue only shows in the preview what global does on the output at that moment: the global effects, moved by any global action that runs. It is not a way to try a global effect or action unseen.
B: Master cue also lets you switch a global effect or action on for the preview only, to try it first.
C: Master cue shows the whole output in the preview, as on a DJ mixer.
MADE FROM (blocks of the s-rta-1007 specs; an id like B-4 is an @@ASSUME, any other id is an @@ITEM): B-4, 177
HE WROTE IN ITS BOX (BF248): "what and where is the master cue? The layers have a cue button to display in the preview/cue monitor but where is the cue button and where would the master cue be displayed?"

### [251] (one line in the small list)
TEXT: I assume the transparency slider beside a cue button is a try-out for the preview only: it starts at full and never moves the layer's own slider.
B: It starts at the layer's own transparency.
C: none
MADE FROM (blocks of the s-rta-1007 specs; an id like B-4 is an @@ASSUME, any other id is an @@ITEM): B-3, R141
HE LEFT ITS BOX EMPTY = ACCEPTED AS WRITTEN

### [252] (one line in the small list)
TEXT: I assume the output always comes first: the preview may be smaller, less smooth and, in the first build, show trails (Echo, Freeze, Feedback) and MilkDrop a little differently.
B: none
C: none
MADE FROM (blocks of the s-rta-1007 specs; an id like B-4 is an @@ASSUME, any other id is an @@ITEM): B-10, B-11, X-11, D8, D9
HE WROTE IN ITS BOX (BF267): "What do you mean by come first? Playing something smaller or less smooth may create problems. What do you think? You understand how the system needs to be built better than me. Of course the output is more important, but the output preview screen which we have and the preview-cure screen below it should all run at the same fps, no?"

## HARMONY'S ANSWERS ON PAGE 2 THAT BELONG TO THIS TOPIC (0), each as he read it

(none)

## THE OLD ASSUMPTIONS OF THIS TOPIC (18 @@ASSUME blocks in spec-B.md of s-rta-1007) AND WHERE EACH WENT
A number = it became that item of page 2 (now answered or accepted: closed). SETTLED / MERGED = closed before page 2. INTERNAL = never shown to him: still Harmony's own, and STILL OPEN -- test each against his new words.

- B-1 -> 220
- B-2 -> 221
- B-3 -> 251
- B-4 -> 222
- B-5 -> INTERNAL: As in Resolume, a clip shown alone has its own effects only; small, cheap to turn.
- B-7 -> INTERNAL: Extends a rule he read and let stand (cue controls are not saved) to the new cue controls.
- B-8 -> INTERNAL: On the page and left standing; a restart would disturb the output.
- B-9 -> MERGED 253: The same point as C-9: what a double-click previews.
- B-10 -> 252
- B-11 -> MERGED 252: One line on what the preview may show differently; second copies of a simulation are not built.
- B-12 -> SETTLED: L35 and L88 keep the preview off screens and recordings; Syphon follows as an output.
- B-13 -> SETTLED: L51 carried to the cued mix by L9; the edge of a cell previewed alone is cheap.
- B-14 -> INTERNAL: Nothing of it reaches the output; small.
- B-17 -> INTERNAL: L35 announces more configurations and describes none; nothing is built from it now.
- B-18 -> INTERNAL: Follows the rule that an action plays only while its button is on.
- B-19 -> INTERNAL: The page said black and he let it stand; a look (L8).
- B-20 -> INTERNAL: A detail of drawing the preview.
- B-21 -> INTERNAL: How a previewed file is fitted is a look (L8).

## IDS
PAGE-2 ITEMS: 220 221 222 251 252
BOXES: BF247(220) BF248(222) BF267(252)
ANSWERS OWED BY THIS TOPIC'S PAPER (an @@ANSWER block each): 222 252
OLD @@ITEM IDS IN spec-B.md: 176 177 R141 R149 R150 R151 R152 R153 R170 R179 D7 D8 D9 P3 P4 P5
OLD @@ASSUME IDS STILL OPEN (INTERNAL): B-5 B-7 B-8 B-14 B-17 B-18 B-19 B-20 B-21
