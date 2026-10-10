# SLICE A -- Triggering clips and the tempo bar: page 2 as Boris saw it, and what he did with it (made by wf/mkslices3.py; never edit by hand)

## HIS BOXES THAT LAND IN THIS TOPIC (3; his words verbatim, with the BF number of the filing and the line of his file)

@@BOX answer 189
BF: BF242
FILE-LINE: 14
HIS-WORDS: I imagine we will be fine tuning this beat detection till it's perfect and we should be able to resync the 1 if the detection is finidng the correct bpm but not the correct 1
@@END

@@BOX 218
BF: BF246
FILE-LINE: 26
PAGE-ITEM: [218] I assume in automatic mode the 1 is always yours (the press that starts the beat, or your Resync): the app never moves it by itself, so after a new track it may sit wrong until you press Resync.
PAGE-WAY-b: The app also places the 1 by itself when it is sure, until your first Resync.
HIS-WORDS: b. app should always try to find the 1 and I will correct if necessary
@@END

@@BOX 250
BF: BF266
FILE-LINE: 86
PAGE-ITEM: [250] I assume tapping changes only the tempo number, from the second tap on; it never shifts the beat or the 1.
PAGE-WAY-b: Each tap also pulls the beat onto your tap, but never moves the 1.
HIS-WORDS: What does shifting the beep mean to you? Does that mean moving the 1 forward or backwards in time?
@@END

## THE PAGE-2 ITEMS OF THIS TOPIC (6), each as he read it

### [217] (a card of its own)
TEXT: I assume a tempo you correct by hand in automatic mode stays close to your number, following only small drifts, until the music clearly changes tempo (a new track that is not beat-matched); then the app follows the music again.
B: It stays exactly at your number until you change it yourself, whatever the music does.
C: none
MADE FROM (blocks of the s-rta-1007 specs; an id like A-4 is an @@ASSUME, any other id is an @@ITEM): A-18, X-4, 189, C14
HE LEFT ITS BOX EMPTY = ACCEPTED AS WRITTEN

### [218] (a card of its own)
TEXT: I assume in automatic mode the 1 is always yours (the press that starts the beat, or your Resync): the app never moves it by itself, so after a new track it may sit wrong until you press Resync.
B: The app also places the 1 by itself when it is sure, until your first Resync.
C: none
MADE FROM (blocks of the s-rta-1007 specs; an id like A-4 is an @@ASSUME, any other id is an @@ITEM): A-18, X-4, 189, R164
HE WROTE IN ITS BOX (BF246): "b. app should always try to find the 1 and I will correct if necessary"

### [219] (a card of its own)
TEXT: I assume that when you start the beat again after a pause or a stop, the nudge is history: the nudge number shown in automatic mode goes back to 0.
B: The nudge number stays through pause, stop and play and keeps shifting the beat; only Resync sets it to 0.
C: none
MADE FROM (blocks of the s-rta-1007 specs; an id like A-4 is an @@ASSUME, any other id is an @@ITEM): A-8, R174, R177
HE LEFT ITS BOX EMPTY = ACCEPTED AS WRITTEN

### [248] (one line in the small list)
TEXT: I assume a click on an empty cell never starts the beat: in a pause the layer's BPM-mode clip stays until the 1 after play.
B: The click sets the beat going, as a click on a clip does.
C: In a pause the layer goes empty at once.
MADE FROM (blocks of the s-rta-1007 specs; an id like A-4 is an @@ASSUME, any other id is an @@ITEM): A-6
HE LEFT ITS BOX EMPTY = ACCEPTED AS WRITTEN

### [249] (one line in the small list)
TEXT: I assume a BPM-mode clip triggered just after the 1 (within a tenth of a beat) still starts at once, in step with the music.
B: It waits a whole bar for the next 1.
C: none
MADE FROM (blocks of the s-rta-1007 specs; an id like A-4 is an @@ASSUME, any other id is an @@ITEM): A-15, D4
HE LEFT ITS BOX EMPTY = ACCEPTED AS WRITTEN

### [250] (one line in the small list)
TEXT: I assume tapping changes only the tempo number, from the second tap on; it never shifts the beat or the 1.
B: Each tap also pulls the beat onto your tap, but never moves the 1.
C: none
MADE FROM (blocks of the s-rta-1007 specs; an id like A-4 is an @@ASSUME, any other id is an @@ITEM): A-2, X-10, 190, R206
HE WROTE IN ITS BOX (BF266): "What does shifting the beep mean to you? Does that mean moving the 1 forward or backwards in time?"

## HARMONY'S ANSWERS ON PAGE 2 THAT BELONG TO THIS TOPIC (1), each as he read it

### answer 189
HE HAD ASKED: "keep it on automatic mode, or will that correction necessitate going back to manual mode?" and "Same question if I correct, where the one is." [L32]
TITLE: Correcting the listening without leaving automatic mode
TEXT: Probably yes, but not proven. The plan: you correct the tempo or the 1, and the app stays in automatic mode; nothing switches to manual by itself. Whether it holds on your music is not measured: I can say only after a test on the DJ tracks you are bringing. If it fails, a correction needs manual mode, and I tell you before anything is built. The tempo: you set the number; the app keeps listening, but only close around your number, so it follows the music's slow changes and keeps the beats on the music. The 1: when you press Resync, that beat is the 1, and the app counts on from it.
LEADS TO ITEM: 217, 218
HE WROTE IN ITS BOX (BF242): "I imagine we will be fine tuning this beat detection till it's perfect and we should be able to resync the 1 if the detection is finidng the correct bpm but not the correct 1"

## THE OLD ASSUMPTIONS OF THIS TOPIC (21 @@ASSUME blocks in spec-A.md of s-rta-1007) AND WHERE EACH WENT
A number = it became that item of page 2 (now answered or accepted: closed). SETTLED / MERGED = closed before page 2. INTERNAL = never shown to him: still Harmony's own, and STILL OPEN -- test each against his new words.

- A-1 -> SETTLED: L13 and L17: a click on the clip starts the held clock, as play does; any clip.
- A-2 -> 250
- A-3 -> SETTLED: L13 with his earlier answer to 111: clips not BPM based play on and are unaffected.
- A-4 -> SETTLED: L11: what is not in BPM mode plays instantly when triggered.
- A-5 -> INTERNAL: Follows "the resolume way" (L11): the X is immediate there; one button, cheap to turn.
- A-6 -> 248
- A-7 -> SETTLED: L12 ("unless resync is clicked") with his earlier 128: Resync is the 1 and starts the beat.
- A-8 -> 219
- A-9 -> INTERNAL: Bookkeeping of a number; what he sees is settled by L14.
- A-10 -> SETTLED: L14: the open range for the listening clock too, narrowed later if it distorts.
- A-11 -> INTERNAL: Technical edge of the range.
- A-12 -> INTERNAL: Belongs to the Link plan, shown to him when Link is planned (item 282).
- A-13 -> SETTLED: L12: "user’s click on play or any clip is the new 1"; L13: the pause holds all the BPM controls, the autopilot too.
- A-14 -> INTERNAL: Technical: a tap count and a gap, tuned at the build.
- A-15 -> 249
- A-16 -> INTERNAL: Presses with no effect; the one other way is the paused start his L30 turned down.
- A-18 -> 217
- A-19 -> SETTLED: L11 applied through L9; L19 shows a column trigger empties a layer.
- A-20 -> INTERNAL: A starting mode; one click either way.
- A-21 -> SETTLED: His earlier 147 a keeps held and trailing pictures after a stop; L72 is about actions and clips.
- A-22 -> INTERNAL: A word on the tempo bar: a display, laid out by Harmony (L8).

## IDS
PAGE-2 ITEMS: 217 218 219 248 249 250
BOXES: BF242(answer 189) BF246(218) BF266(250)
ANSWERS OWED BY THIS TOPIC'S PAPER (an @@ANSWER block each): 250
OLD @@ITEM IDS IN spec-A.md: 172 173 174 175 189 190 191 R128 R168 R139 R140 R142 R163 R169 R164 R174 R175 R176 R214 R215 R216 R177 R178 R205 R206 D1 D2 D3 D4 D5 D6 P1 P2
OLD @@ASSUME IDS STILL OPEN (INTERNAL): A-5 A-9 A-11 A-12 A-14 A-16 A-20 A-22
