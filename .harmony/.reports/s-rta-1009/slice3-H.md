# SLICE H -- How a clip plays: page 2 as Boris saw it, and what he did with it (made by wf/mkslices3.py; never edit by hand)

## HIS BOXES THAT LAND IN THIS TOPIC (3; his words verbatim, with the BF number of the filing and the line of his file)

@@BOX answer codec
BF: BF240
FILE-LINE: 8
HIS-WORDS: after we build the app can we test encoding and decoding of different codec to have a baseline of what codecs work good on this mac m1 32gb?
@@END

@@BOX 238
BF: BF258
FILE-LINE: 62
PAGE-ITEM: [238] I assume a new clip in BPM mode gets a number of beats (4, 8, 16, 32 ...) that plays the whole clip at about its normal speed, and its end is never cut.
PAGE-WAY-b: Its end is cut in so that the clip is whole groups of 4 bars and plays at exactly its normal speed.
HIS-WORDS: b
@@END

@@BOX 239
BF: BF259
FILE-LINE: 65
PAGE-ITEM: [239] I assume we build no codec. First I measure, on two of your clips, how long a clip waits when it jumps; only if you would notice the wait do I add a command that makes HAP copies. Ordinary files keep playing.
PAGE-WAY-b: Plan the command that makes HAP copies now, without timing first.
PAGE-WAY-c: No such command: you make HAP copies yourself with tools you own.
HIS-WORDS: Why would we make HAP copies at all?
@@END

## THE PAGE-2 ITEMS OF THIS TOPIC (4), each as he read it

### [238] (a card of its own)
TEXT: I assume a new clip in BPM mode gets a number of beats (4, 8, 16, 32 ...) that plays the whole clip at about its normal speed, and its end is never cut.
B: Its end is cut in so that the clip is whole groups of 4 bars and plays at exactly its normal speed.
C: none
MADE FROM (blocks of the s-rta-1007 specs; an id like H-4 is an @@ASSUME, any other id is an @@ITEM): H-5, R218, C13
HE WROTE IN ITS BOX (BF258): "b"

### [239] (a card of its own)
TEXT: I assume we build no codec. First I measure, on two of your clips, how long a clip waits when it jumps; only if you would notice the wait do I add a command that makes HAP copies. Ordinary files keep playing.
B: Plan the command that makes HAP copies now, without timing first.
C: No such command: you make HAP copies yourself with tools you own.
MADE FROM (blocks of the s-rta-1007 specs; an id like H-4 is an @@ASSUME, any other id is an @@ITEM): H-14, G1
HE WROTE IN ITS BOX (BF259): "Why would we make HAP copies at all?"

### [266] (one line in the small list)
TEXT: I assume triggering the clip that is already playing always restarts it, even when it is set to continue where it left off.
B: Set to continue, it plays on undisturbed when you trigger it again.
C: none
MADE FROM (blocks of the s-rta-1007 specs; an id like H-4 is an @@ASSUME, any other id is an @@ITEM): H-3
HE LEFT ITS BOX EMPTY = ACCEPTED AS WRITTEN

### [267] (one line in the small list)
TEXT: I assume a video you bring in starts in Timeline mode; you switch the clips you want on the beat to BPM mode yourself.
B: Every new video starts in BPM mode.
C: One setting says which mode new clips start in.
MADE FROM (blocks of the s-rta-1007 specs; an id like H-4 is an @@ASSUME, any other id is an @@ITEM): H-15
HE LEFT ITS BOX EMPTY = ACCEPTED AS WRITTEN

## HARMONY'S ANSWERS ON PAGE 2 THAT BELONG TO THIS TOPIC (1), each as he read it

### answer codec
HE HAD ASKED: "Is there any reason for us to build our own codec that is optimized for our system" [L3]
TITLE: A codec of our own: no
TEXT: No. I would not build one, and I could not promise to make one reliable: it would play only in this app and need its own tools and long testing, and no other program could play or check its files. What you want from DXV exists in a free format with the same purpose, HAP; as far as I can see without running it, the app already plays HAP files. Such formats store each picture whole, so a jump, backwards play or a random beat should need no catching up. Not measured yet: how long a jump waits on your ordinary clips, and how much bigger a copy made in HAP is (my estimate: much bigger). So I measure first, on two of your clips, and then we decide.
LEADS TO ITEM: 239
HE WROTE IN ITS BOX (BF240): "after we build the app can we test encoding and decoding of different codec to have a baseline of what codecs work good on this mac m1 32gb?"

## THE OLD ASSUMPTIONS OF THIS TOPIC (14 @@ASSUME blocks in spec-H.md of s-rta-1007) AND WHERE EACH WENT
A number = it became that item of page 2 (now answered or accepted: closed). SETTLED / MERGED = closed before page 2. INTERNAL = never shown to him: still Harmony's own, and STILL OPEN -- test each against his new words.

- H-1 -> SETTLED: L101, L20, L21: one marker per beat, and Random lands on a beat marker.
- H-2 -> INTERNAL: Follows Resolume's manual, his model (L102); he can see it in his Arena.
- H-3 -> 266
- H-4 -> INTERNAL: An edge of a menu that stands on "from the start" until he changes it.
- H-5 -> 238
- H-6 -> INTERNAL: An edge he will rarely meet.
- H-7 -> INTERNAL: Arithmetic of the Duration row; the out point part stood on his page.
- H-9 -> INTERNAL: Follows Resolume's manual, his model (L102).
- H-10 -> INTERNAL: Internal; follows his answer to item 238.
- H-11 -> INTERNAL: L102 makes Resolume the model; one look in his Arena checks the third entry.
- H-12 -> INTERNAL: Technical: the size of one corrective jump.
- H-13 -> INTERNAL: Arithmetic of a row he asked to be as Resolume's (L21).
- H-14 -> 239
- H-15 -> 267

## IDS
PAGE-2 ITEMS: 238 239 266 267
BOXES: BF240(answer codec) BF258(238) BF259(239)
ANSWERS OWED BY THIS TOPIC'S PAPER (an @@ANSWER block each): 239 codec
OLD @@ITEM IDS IN spec-H.md: 201 202 R192 R217 R218 R193 R194 R219 P28 P29 G1
OLD @@ASSUME IDS STILL OPEN (INTERNAL): H-2 H-4 H-6 H-7 H-9 H-10 H-11 H-12 H-13
