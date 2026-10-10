# SLICE I -- Effects, signals and what moves a slider by itself: page 2 as Boris saw it, and what he did with it (made by wf/mkslices3.py; never edit by hand)

## HIS BOXES THAT LAND IN THIS TOPIC (3; his words verbatim, with the BF number of the filing and the line of his file)

@@BOX answer R221-b
BF: BF244
FILE-LINE: 20
HIS-WORDS: For these effects, we should just be able to adjust the sink, and maybe we could use a common macro that we could set later for example shows where a single macro controls the speed of all of these type of effects
@@END

@@BOX 240
BF: BF260
FILE-LINE: 68
PAGE-ITEM: [240] I assume every layer works the same: it is blended in by its blend mode and its transparency slider, the bottom one too. There are no mask layers.
PAGE-WAY-b: Mask layers stay; only the keying list and its slider go.
HIS-WORDS: All the blend modes and keying stay, and they will be built when there is time. Right now they're just sitting there as placeholder. We will also add masks and moving masks with which are alpha channels
@@END

@@BOX 241
BF: BF261
FILE-LINE: 71
PAGE-ITEM: [241] I assume an envelope you draw always runs on the beat, never along the clip's playhead.
PAGE-WAY-b: An envelope can also follow the playhead: its drawn shape is stretched over the clip from start to end.
HIS-WORDS: There should be two different types of envelopes that could work. One is along the beat, which is like a signal, but personalized for that clip, and another one is play head based so we can draw a envelope that happens over the course of the playing of that clip in timeline mode.
@@END

## THE PAGE-2 ITEMS OF THIS TOPIC (5), each as he read it

### [240] (a card of its own)
TEXT: I assume every layer works the same: it is blended in by its blend mode and its transparency slider, the bottom one too. There are no mask layers.
B: Mask layers stay; only the keying list and its slider go.
C: none
MADE FROM (blocks of the s-rta-1007 specs; an id like I-4 is an @@ASSUME, any other id is an @@ITEM): I-1, 204
HE WROTE IN ITS BOX (BF260): "All the blend modes and keying stay, and they will be built when there is time. Right now they're just sitting there as placeholder. We will also add masks and moving masks with which are alpha channels"

### [241] (a card of its own)
TEXT: I assume an envelope you draw always runs on the beat, never along the clip's playhead.
B: An envelope can also follow the playhead: its drawn shape is stretched over the clip from start to end.
C: none
MADE FROM (blocks of the s-rta-1007 specs; an id like I-4 is an @@ASSUME, any other id is an @@ITEM): I-6, 205
HE WROTE IN ITS BOX (BF261): "There should be two different types of envelopes that could work. One is along the beat, which is like a signal, but personalized for that clip, and another one is play head based so we can draw a envelope that happens over the course of the playing of that clip in timeline mode."

### [242] (a card of its own)
TEXT: I assume a One Shot on a slider runs once from the next 1 after its clip is triggered; on a layer's slider after every clip triggered on that layer; on a global slider when you switch it on.
B: It starts the moment its clip starts, off the beat too.
C: It runs only when you press a start button on that slider.
MADE FROM (blocks of the s-rta-1007 specs; an id like I-4 is an @@ASSUME, any other id is an @@ITEM): I-9, R221
HE LEFT ITS BOX EMPTY = ACCEPTED AS WRITTEN

### [243] (a card of its own)
TEXT: I assume effects that read the beat by themselves (a strobe, a pulse) hold still while the beat is paused or stopped.
B: They keep pulsing through a pause.
C: none
MADE FROM (blocks of the s-rta-1007 specs; an id like I-4 is an @@ASSUME, any other id is an @@ITEM): I-16, R221, R214
HE LEFT ITS BOX EMPTY = ACCEPTED AS WRITTEN

### [268] (one line in the small list)
TEXT: I assume Falloff means: the value jumps up with the signal at once and comes down slowly, from instant to 2 seconds.
B: It smooths the way up and the way down alike.
C: none
MADE FROM (blocks of the s-rta-1007 specs; an id like I-4 is an @@ASSUME, any other id is an @@ITEM): I-13
HE LEFT ITS BOX EMPTY = ACCEPTED AS WRITTEN

## HARMONY'S ANSWERS ON PAGE 2 THAT BELONG TO THIS TOPIC (1), each as he read it

### answer R221-b
HE HAD ASKED: "Is this a good idea, help me think through this logically" [L107]
TITLE: Effects that read the beat by themselves
TEXT: Yes, it is a good idea. The logic: such an effect (a strobe, a pulse) follows the beat clock, not the signals. So what holds the clock holds the effect: on pause it stands where it was and goes on in step when you press play; on stop it waits and starts on your new 1. That is your own rule: pause pauses everything the beat controls. The Master Signal only turns signals down, so at 0 the strobe keeps its beat; to stop it, switch the effect off. One thing to know: a strobe paused in its dark moment stays dark until you press play.
LEADS TO ITEM: 243
HE WROTE IN ITS BOX (BF244): "For these effects, we should just be able to adjust the sink, and maybe we could use a common macro that we could set later for example shows where a single macro controls the speed of all of these type of effects"

## THE OLD ASSUMPTIONS OF THIS TOPIC (24 @@ASSUME blocks in spec-I.md of s-rta-1007) AND WHERE EACH WENT
A number = it became that item of page 2 (now answered or accepted: closed). SETTLED / MERGED = closed before page 2. INTERNAL = never shown to him: still Harmony's own, and STILL OPEN -- test each against his new words.

- I-1 -> 240
- I-2 -> SETTLED: L111 ("that layers blend mode") with L110: the list stays, the slider sets its strength.
- I-3 -> INTERNAL: The order of work between L110 and L111; nothing on stage hangs on it.
- I-4 -> INTERNAL: Nothing he asked for is taken away (L111 names the layer's keying only).
- I-5 -> INTERNAL: It only promises a later question (L110).
- I-6 -> 241
- I-7 -> SETTLED: L112: what follows the layer's playhead is called Timeline, so Clip Position takes that name.
- I-8 -> INTERNAL: Internal detail: the two ends, and a fade between clips.
- I-9 -> 242
- I-10 -> SETTLED: L106 names the reading and moves only where the controls sit; "holds its last value" stands (L1).
- I-11 -> INTERNAL: A control is left out only where it has nothing to do.
- I-12 -> INTERNAL: A number that is tuned at the build.
- I-13 -> 268
- I-14 -> SETTLED: L106 describes a button that is on or off above a threshold.
- I-15 -> INTERNAL: L108 gives the line; what it shows on a music signal is a look (L8).
- I-16 -> 243
- I-17 -> INTERNAL: Layout and names (L8).
- I-18 -> INTERNAL: The standing half of the reading; L64 gives the Global glide to actions.
- I-19 -> INTERNAL: Technical: an effect keeps its identity when it is moved (L104).
- I-20 -> INTERNAL: Wording of lists only, after his own list of L83; Harmony adds it to the list of names.
- I-21 -> INTERNAL: A mend: it follows its layer, as an action does; anything else would be a fault.
- I-22 -> INTERNAL: Small: where the bar lines of a sample are counted from.
- I-23 -> INTERNAL: Consistency of one setting.
- I-24 -> INTERNAL: Technical.

## IDS
PAGE-2 ITEMS: 240 241 242 243 268
BOXES: BF244(answer R221-b) BF260(240) BF261(241)
ANSWERS OWED BY THIS TOPIC'S PAPER (an @@ANSWER block each): none
OLD @@ITEM IDS IN spec-I.md: 203 204 205 R195 R210 R220 R221 R196 D29 D30 D31 P30 P31
OLD @@ASSUME IDS STILL OPEN (INTERNAL): I-3 I-4 I-5 I-8 I-11 I-12 I-15 I-17 I-18 I-19 I-20 I-21 I-22 I-23 I-24
