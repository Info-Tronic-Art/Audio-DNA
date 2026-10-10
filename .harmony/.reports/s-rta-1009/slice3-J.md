# SLICE J -- The keyboard and MIDI mapping, menus and messages: page 2 as Boris saw it, and what he did with it (made by wf/mkslices3.py; never edit by hand)

## HIS BOXES THAT LAND IN THIS TOPIC (5; his words verbatim, with the BF number of the filing and the line of his file)

@@BOX answer review-name
BF: BF245
FILE-LINE: 23
HIS-WORDS: Let's go with studio. That's perfect.
@@END

@@BOX 244
BF: BF262
FILE-LINE: 74
PAGE-ITEM: [244] I assume a MIDI knob you map to a clip's slider stays with that clip, wherever the clip goes; a copy of the clip has no knob. A MIDI knob on a layer's or a global slider stays there.
PAGE-WAY-b: The knob belongs to the cell: it moves that slider of whatever clip sits there in the deck on screen.
PAGE-WAY-c: The knob moves that slider of whatever clip is playing on that layer.
HIS-WORDS: b
@@END

@@BOX 245
BF: BF263
FILE-LINE: 77
PAGE-ITEM: [245] I assume the screen where you watch a recording and make actions keeps the name Review, as you wrote, until you pick another.
PAGE-WAY-b: It is called Studio.
PAGE-WAY-c: It is called Replay.
HIS-WORDS: b
@@END

@@BOX 246
BF: BF264
FILE-LINE: 80
PAGE-ITEM: [246] I assume your answer stands: no hold setting. A pad or a key only presses.
PAGE-WAY-b: Every pad and key gets one switch, press or hold, set to press until you change it, so nothing changes for a pad you leave alone.
HIS-WORDS: I want you to do some research online and figure out what people are doing and if this is worth doing? We need to only match what Dj or bands are doing and also look at Dj/producers.
@@END

@@BOX 270
BF: BF270
FILE-LINE: 98
PAGE-ITEM: [270] I assume MIDI knobs must send their position: an endless knob that only sends steps will not work. Which MIDI controller do you use?
PAGE-WAY-b: Knobs that send steps must work too.
HIS-WORDS: b due to what we're doing endless will be better, but we can assume that people will use one or the other. We need to work with both and know how they both work so they both work smoothly. If we have to, we could ask the user to set a toggle if it's an endless encoder.
@@END

## THE PAGE-2 ITEMS OF THIS TOPIC (6), each as he read it

### [244] (a card of its own)
TEXT: I assume a MIDI knob you map to a clip's slider stays with that clip, wherever the clip goes; a copy of the clip has no knob. A MIDI knob on a layer's or a global slider stays there.
B: The knob belongs to the cell: it moves that slider of whatever clip sits there in the deck on screen.
C: The knob moves that slider of whatever clip is playing on that layer.
MADE FROM (blocks of the s-rta-1007 specs; an id like J-4 is an @@ASSUME, any other id is an @@ITEM): J-8, 206
HE WROTE IN ITS BOX (BF262): "b"

### [245] (a card of its own)
TEXT: I assume the screen where you watch a recording and make actions keeps the name Review, as you wrote, until you pick another.
B: It is called Studio.
C: It is called Replay.
MADE FROM (blocks of the s-rta-1007 specs; an id like J-4 is an @@ASSUME, any other id is an @@ITEM): R197, R224 (the answer review-name)
HE WROTE IN ITS BOX (BF263): "b"

### [246] (a card of its own)
TEXT: I assume your answer stands: no hold setting. A pad or a key only presses.
B: Every pad and key gets one switch, press or hold, set to press until you change it, so nothing changes for a pad you leave alone.
C: none
MADE FROM (blocks of the s-rta-1007 specs; an id like J-4 is an @@ASSUME, any other id is an @@ITEM): 207 (the answer 207-hold), D34
HE WROTE IN ITS BOX (BF264): "I want you to do some research online and figure out what people are doing and if this is worth doing? We need to only match what Dj or bands are doing and also look at Dj/producers."

### [269] (one line in the small list)
TEXT: I assume a mapping file holds the whole keyboard and MIDI mapping of the open show; importing one replaces that show's mapping, after asking.
B: An import adds to the mapping and overwrites only the keys that clash.
C: none
MADE FROM (blocks of the s-rta-1007 specs; an id like J-4 is an @@ASSUME, any other id is an @@ITEM): J-2
HE LEFT ITS BOX EMPTY = ACCEPTED AS WRITTEN

### [270] (one line in the small list)
TEXT: I assume MIDI knobs must send their position: an endless knob that only sends steps will not work. Which MIDI controller do you use?
B: Knobs that send steps must work too.
C: none
MADE FROM (blocks of the s-rta-1007 specs; an id like J-4 is an @@ASSUME, any other id is an @@ITEM): J-4, 207, D33
HE WROTE IN ITS BOX (BF270): "b due to what we're doing endless will be better, but we can assume that people will use one or the other. We need to work with both and know how they both work so they both work smoothly. If we have to, we could ask the user to set a toggle if it's an endless encoder."

### [271] (one line in the small list)
TEXT: I assume switching an output screen on or off cannot be put on a key or a pad, so that no stray hit blanks a projector.
B: It can go on a key or a pad like any button.
C: none
MADE FROM (blocks of the s-rta-1007 specs; an id like J-4 is an @@ASSUME, any other id is an @@ITEM): J-18
HE LEFT ITS BOX EMPTY = ACCEPTED AS WRITTEN

## HARMONY'S ANSWERS ON PAGE 2 THAT BELONG TO THIS TOPIC (2), each as he read it

### answer review-name
HE HAD ASKED: "Show Recording Review should be called Review for short is a good name. Do you have a better name for this?" [L114]
TITLE: A name for the review screen
TEXT: Not clearly better. Review is a good name, and I keep writing it until you say otherwise. One risk: when you speak or dictate, Review and Preview differ by one sound, and the app has both. If that bothers you, my pick is Studio: musicians pair Live and Studio, and the studio is where you listen back to a recording and make things from it. The other candidate is Replay, your own earlier word ("the replay window"): clear, but close to play on the tempo bar.
LEADS TO ITEM: 245
HE WROTE IN ITS BOX (BF245): "Let's go with studio. That's perfect."

### answer 207-hold
HE HAD ASKED: "make an argument for why we should have a key/pad hold setting" [L120]
TITLE: Why a pad could have a hold setting
TEXT: The argument: hold turns a pad into an instrument. With a band you play by hand: the flash, the logo or the effect is there exactly as long as you hold the pad, through a drum fill or a held chord, and gone when you let go. A DJ holds a pad through the build-up and lets go on the drop. With press only, each of these needs a second press that must also be on time; miss it and the flash stays on. It would be one switch per pad, set to press until you change it. One limit: on a BPM mode clip a short hold can end before the 1, and then nothing shows. I would add it; but your answer stands unless you say otherwise.
LEADS TO ITEM: 246
HE LEFT ITS BOX EMPTY (no comment)

## THE OLD ASSUMPTIONS OF THIS TOPIC (20 @@ASSUME blocks in spec-J.md of s-rta-1007) AND WHERE EACH WENT
A number = it became that item of page 2 (now answered or accepted: closed). SETTLED / MERGED = closed before page 2. INTERNAL = never shown to him: still Harmony's own, and STILL OPEN -- test each against his new words.

- J-1 -> INTERNAL: L114 is read as names ("what the 2 recordings are called"); nothing is taken away; a film box is small to remove.
- J-2 -> 269
- J-3 -> SETTLED: L118: tap tempo, mappable ("R227 all good"); in Review his own words: the spacebar stops the playing.
- J-4 -> 270
- J-5 -> SETTLED: L122 with L65, L68, L80: strictly the messages he named and a failed save; the "may tell you" clause is struck.
- J-6 -> INTERNAL: The safe way for the stage; how a message leaves is a look (L8).
- J-7 -> SETTLED: L112: the playhead link "should be called as such"; L21 "timeline mode" and L11 "BPM mode" are his.
- J-8 -> 244
- J-12 -> INTERNAL: Follows 206 A and 207 c; small to widen later.
- J-13 -> INTERNAL: An edge he would not notice.
- J-14 -> INTERNAL: Deferred with 214 by the default he took.
- J-15 -> INTERNAL: His Undo rule applied to the new controls.
- J-16 -> INTERNAL: List housekeeping; nothing fires either way.
- J-17 -> INTERNAL: Layout and wording (L8).
- J-18 -> 271
- J-19 -> INTERNAL: The Mac's own habits.
- J-20 -> INTERNAL: A held /2 would run the tempo to its limit; nobody wants that.
- J-21 -> INTERNAL: An edge of a rule he gave.
- J-22 -> INTERNAL: The plain meaning of his words "not saved".
- J-23 -> INTERNAL: Not offered on the page; small to add.

## IDS
PAGE-2 ITEMS: 244 245 246 269 270 271
BOXES: BF245(answer review-name) BF262(244) BF263(245) BF264(246) BF270(270)
ANSWERS OWED BY THIS TOPIC'S PAPER (an @@ANSWER block each): none
OLD @@ITEM IDS IN spec-J.md: 206 207 208 209 210 214 R148 R197 R198 R199 R227 R200 D32 D33 D34 D35 P32 P33 P34
OLD @@ASSUME IDS STILL OPEN (INTERNAL): J-1 J-6 J-12 J-13 J-14 J-15 J-16 J-17 J-19 J-20 J-21 J-22 J-23
