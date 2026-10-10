# APPLY H -- How a clip plays (s-rta-1009, page 2)

## SUMMARY
- 238 (BF258 "b"): the first number of Beats is settled. A clip switched to BPM mode gets whole groups of 4 bars (16, 32, 48 ... beats, counted at 120 BPM) and its clip out point is moved in to match; the way of his Resolume (4, 8, 16, 32, nothing cut) is not built. R218 (b) is rewritten, C13 closed.
- 239 (BF259) is OPEN: he asks why HAP copies would be made at all. He gets the reason (ANSWER 239) and the item again in a form he can answer (H3-2): no copies and no command for them; asked again only if the test shows a wait.
- The codec answer (BF240): yes, a codec test after the app is built. The page's "first I measure" is no longer a step before the build: the jump timing is now one part of that test (H3-3 puts the other order to him as one line).
- 266 and 267 accepted as written: a trigger of the playing clip always restarts it; a video comes in in Timeline mode.
- Words from other boxes applied to this topic's own old blocks: the keying and its K slider stay on the layer (BF260: R194 f, P28); the screen is called Studio (BF245, BF263: G1); tempo stop takes every clip off its layer (BF271: H-4).
- Asked again: 1 (H3-2). Lines he can strike: 2 (H3-1, H3-3). Nothing measured, nothing built.

## ITEMS
@@ITEM 238
TITLE: The first Beats of a clip in BPM mode: groups of 4 bars
STATUS: ANSWERED way b
HIS: BF258 "b"
RULE: The page's way b, in full: "Its end is cut in so that the clip is whole groups of 4 bars and plays at exactly its normal speed." It is his own rule of 2026-10-04 (binding-decisions.md 812-813: "always work with multiples of 4. If it is uneven, then move the outpoint in to keep those multiples."). Built so: (1) The part of the clip between its clip in point and its clip out point is counted at 120 BPM (his earlier words, binding-decisions.md 684: "default all bpms to 120"; 673: "not current bpm"): at 120 BPM a group of 4 bars is 16 beats and lasts 8 seconds. (2) Beats becomes 16 times the number of whole groups that fit into that part: 16, 32, 48, 64 and so on. (3) The clip out point is moved in to the end of the last whole group, on a whole picture, so that at Speed 1 and a tempo of 120 the clip plays at exactly its normal speed; at any other tempo it plays faster or slower with the tempo, as every clip in BPM mode does. (4) Nothing is taken out of the file: the cut-off end stays in it and plays again when he drags the clip out point out; then the same number of Beats covers more video and the clip looks faster (item R193 g as it stands). (5) A part shorter than one group (under 8 seconds) gets 16 beats and is not cut: it plays slower than normal and Speed makes it fast (his earlier words, binding-decisions.md 846: "a short clip can have 4 bars but it will move super fast"; 865: "47 default"). (6) The number and the cut are worked out once, the first time the clip is switched to BPM mode (H3-1); after that the app never moves the clip out point or changes Beats by itself. (7) Everything else of the Beats row is item R218 (a) as it stands: "-" and "+" change Beats by 1, "/2" and "x2" halve and double it, any whole number is allowed, and none of them moves the clip out point. (8) A clip of many pictures is counted at its Timeline-mode rate (H-10).
CHANGED: The page's own text (4, 8, 16, 32 ... nearest to normal speed, the end never cut; the way of his Resolume, old assumption H-5) is replaced by its way b. Against the old blocks: R218 (b) was open and is now settled (AMEND R218 1); C13 said nothing of the first number is built before his reply (AMEND C13 1); H-10's "nearest, judged as a ratio" and "never under 4" fall away (H-10 rewritten). Decided by Harmony and not by his letter: WHEN the cut is made (H3-1) and the count at 120 BPM (H-10, from his earlier words).
TODAY: Every new clip starts at 4 beats / 4 beats; nothing sets a length from the file or the tempo and nothing moves an out point by itself (Clip.h:36-41 and area-clip-transport.md part 1 item 10, as cited in spec-H R218 TODAY; not re-read). To build: the count, the Beats number, the moved clip out point, at the first switch to BPM mode.
@@END
@@ITEM 239
TITLE: No codec; HAP copies wait on his answer and the test
STATUS: OPEN he asks back why HAP copies at all
HIS: BF259 "Why would we make HAP copies at all?"; BF240 "after we build the app can we test encoding and decoding"
RULE: Waits on his answer: the reading that is put to him again (H3-2) is that no HAP copy is ever made and no command for it is built, unless the codec test shows that his ordinary clips wait. In full: (1) BUILT: no codec of our own. No command that makes copies of his clips, and none is planned. No file of his is converted. Every video file plays as it is; a file he brings that already stores every picture whole (HAP, ProRes, Motion JPEG) plays like any other file (item G1 points 1 and 2 as they stand). Random, Beat Repeat, backwards play, a trigger into a long video and scrubbing in Studio are built to work on his ordinary files as they are. (2) TESTED, after the app is built (his words, BF240): the codec test, on this Mac (M1, 32 GB), with his own clips. Decoding: each test clip as the ordinary file it is and as copies in HAP, ProRes and Motion JPEG, and a DXV file of his if it opens; for each, whether it plays forwards without losing pictures, alone and on several layers at once; how long the first right picture waits after a trigger, a preview, a jump, a Random beat, a Beat Repeat, a turn to backwards play and a scrub in Studio; and how big the file is. Encoding: every codec the app can record in, for Record to Clip and for the low-resolution show recording (topic E names them; BF241: "we should test which codecs work best"); for each, whether the output loses pictures while the recording runs, how big the files are, and how the recorded clip then plays and jumps. The test copies are made by Harmony, of the test clips only, in a folder of their own (H3-5). The result is one short list for him: which codecs work well on this Mac. No number is promised before it. (3) THEN: only if the test shows that a jump, backwards play or a Random beat waits on his ordinary clips is the choice of fast copies put to him again, with the test's result beside it; if nothing waits, it is never raised again. (4) WHERE THE MEASUREMENT STANDS: the page said "First I measure, on two of your clips". That measurement is no longer a step before the build or in its first session: it is the jump part of the codec test, after the build (BF240); the other order is H3-3's way b.
CHANGED: His box holds a question, no letter: none of the three ways is taken. The page's "only if you would notice the wait do I add a command that makes HAP copies" becomes "no command; asked again only if the test shows a wait". The page's "First I measure" moves to after the build (BF240). Against G1: its point 3 (measured in the first build session) and point 4 (a command "Convert for performance" planned if the wait shows, his choice H-14) change (AMEND G1 1, 2, 4); H-14 is replaced by H3-2.
TODAY: Video is decoded by FFmpeg on the CPU, one decode thread per clip; HAP, ProRes, Motion JPEG and DNxHD are treated as every-picture-whole; a jump on an ordinary file re-seeks to a keyframe and catches up, and how long that takes on his clips is UNKNOWN (spec-H G1 TODAY: VideoPlayer.cpp:117, :290; pitfalls 56, 62; not re-read). The app records H.264, ProRes and MJPEG (CLAUDE.md, project identity). This Mac reports "Apple M1 Pro" and 34359738368 bytes of memory (sysctl, read this session). Whether a DXV file of his opens: INFERRED, not tried. The local FFmpeg's HAP encoder may be missing (check-codec.md fact 11, as cited in G1 TODAY): to be checked before the test copies are made.
@@END
@@ITEM 266
TITLE: Triggering the playing clip always restarts it
STATUS: ACCEPTED
HIS: box left empty = accepted as written
RULE: Triggering the clip that is already playing on its layer always restarts it, whatever its start menu says: also when the menu stands on Continue or Relative, and also when the trigger comes from its column. The start menu acts only when a clip comes back to its layer after it has been away. Everything around it is @@ITEM 202 as it stands: a restart begins at the clip in point, or at the clip out point for a clip set to play backwards; a clip in BPM mode restarts on the 1; a clip he paused himself still shows on its paused frame when triggered.
CHANGED: nothing: accepted as written. Item 202 already rules it in the same words; old assumption H-3 is closed by it.
TODAY: A re-trigger through the clip's cell seeks to the in point; a column re-trigger does not restart (area-clip-transport.md part 1 items 12 and 13, as cited in spec-H 202 TODAY; not re-read). To build: the column path restarts too.
@@END
@@ITEM 267
TITLE: A video comes into the show in Timeline mode
STATUS: ACCEPTED
HIS: box left empty = accepted as written
RULE: A video brought into the show starts in Timeline mode: it plays at its own Speed and Duration and starts the moment it is triggered. He switches the clips he wants on the beat to BPM mode himself, clip by clip; there is no setting that says which mode new clips start in. A clip of many pictures comes in the same way (H3-4). At the first switch to BPM mode the clip gets its Beats and its cut end (item 238). The mode belongs to the clip and is saved with the show.
CHANGED: nothing: accepted as written. Old assumption H-15 is closed by it; the rule is added to R193, which did not name the starting mode (AMEND R193 1).
TODAY: A new clip is in Timeline mode (Clip.h:118-119 and area-clip-transport.md part 1 item 1, as cited in spec-H R193 TODAY; not re-read). Nothing to change for the starting mode.
@@END

## AMENDMENTS
@@AMEND R218 1
OLD: (b) The number a new clip starts with is OPEN and is not built until he answers H-5. Three ways are on the table. The best reading, and the text of H-5, is the way of his Resolume, which he reports after calling his bars a mistake (L21: "By default it gives me a decent amount of beats (sometimes 8, sometimes 16) so that it plays at about the same speed as it would play at speed number one in timeline mode"): the power of two from 4 up (4, 8, 16, 32 ...) that plays the whole clip nearest to its normal speed, with no out point moved. Against it stands his rule of 2026-10-04, which the page repeated and he left as it was: whole groups of 4 bars, the out point moved in to match (binding-decisions.md 812-813: "always work with multiples of 4. If it is uneven, then move the outpoint in to keep those multiples."). The third way is that rule in whole bars, that is multiples of 4 beats. His L99 calls the bars of 2026-10-04 his mistake, and that rule was made in bars: whether the mistake reaches the first number is not said. Whichever he picks, the number is worked out once, when the clip comes into the show, counting the clip at 120 BPM and never under 4 beats (H-10; a picture sequence, which has no seconds of its own, is counted at its Timeline-mode rate).
NEW: (b) The number a new clip starts with is settled by his answer to item 238 (BF258: "b", the page's way b: "Its end is cut in so that the clip is whole groups of 4 bars and plays at exactly its normal speed."), which is his rule of 2026-10-04 (binding-decisions.md 812-813: "always work with multiples of 4. If it is uneven, then move the outpoint in to keep those multiples."). The part of the clip between its clip in point and its clip out point is counted at 120 BPM, where a group of 4 bars is 16 beats and lasts 8 seconds. Beats becomes 16 times the number of whole groups that fit (16, 32, 48, 64 ...), and the clip out point is moved in to the end of the last whole group, on a whole picture, so that at Speed 1 and a tempo of 120 the clip plays at exactly its normal speed. The cut-off end stays in the file and plays again when he drags the clip out point out. A part shorter than one group (under 8 seconds) gets 16 beats and is not cut; it plays slower than normal (his earlier words, binding-decisions.md 846: "a short clip can have 4 bars but it will move super fast"; 865: "47 default"). The number and the cut are worked out once, the first time the clip is switched to BPM mode (H3-1); after that the app never moves the clip out point or changes Beats by itself, and "-", "+", "/2" and "x2" never move the clip out point. The way of his Resolume (4, 8, 16, 32 ..., nothing cut; L21) and whole single bars are not built. A clip of many pictures, which has no seconds of its own, is counted at its Timeline-mode rate (H-10).
HIS: BF258 "b"
WHY: Part (b) held the first number open between three ways until he answered H-5; he answered it as item 238 with way b, so the open text is replaced by the rule.
@@END
@@AMEND C13 1
OLD: A look is not a rule; the two are put to him once, in the clip topic (H-5), and nothing of the first number is built before his reply.
NEW: A look is not a rule; the two were put to him as item 238, and he chose his own rule (BF258: "b"): whole groups of 4 bars, the clip out point moved in, exactly normal speed. The rule in full is item R218 (b).
HIS: BF258 "b"
WHY: It said the first number waits on his reply; the reply has come.
@@END
@@AMEND R193 1
OLD: APPEND
NEW: (i) A video or a clip of many pictures brought into the show starts in Timeline mode; he switches the clips he wants on the beat to BPM mode himself, and no setting chooses the starting mode (item 267; for a clip of many pictures: H3-4). At its first switch to BPM mode a clip gets its Beats and its cut end (item R218 b).
HIS: item 267 accepted as written
WHY: No part of this rule said in which mode a new clip starts; it stood only in assumption H-15, which he has now accepted as item 267.
@@END
@@AMEND R194 1
OLD: takes the keying and its K slider off the layer; what it means for the Blend list is ruled in topic I (question 204), and (f) follows that ruling.
NEW: no longer holds: his newer words (BF260) are "All the blend modes and keying stay, and they will be built when there is time." So the keying and its K slider stay on the layer and every blend mode stays in the Blend list; when each of them is built, and the masks he adds, is ruled in topic I (item 240), and (f) follows that ruling.
HIS: BF260 "All the blend modes and keying stay"
WHY: It said the keying and its K slider leave the layer (L111); his words of this round keep them.
@@END
@@AMEND P28 1
OLD: the strip carries no keying control and no K slider (L111; ruled in topic I)
NEW: the strip keeps its keying control and its K slider (BF260: "All the blend modes and keying stay, and they will be built when there is time."; ruled in topic I, item 240)
HIS: BF260 "All the blend modes and keying stay"
WHY: It listed the keying control and the K slider as gone (L111); his words of this round keep them.
@@END
@@AMEND G1 1
OLD: in the first build session, on two of his own clips (one 1080p, one 4K), the wait from a jump to the first right picture is measured
NEW: after the app is built (his words, BF240: "after we build the app can we test encoding and decoding of different codec to have a baseline"), as one part of the codec test of point 7, on his own clips (one 1080p, one 4K), the wait from a jump to the first right picture is measured
HIS: BF240 "after we build the app can we test encoding and decoding"
WHY: It put the measurement into the first build session; he asks for the test after the build, so the measurement moves there (the other order is H3-3's way b).
@@END
@@AMEND G1 2
OLD: whether it is planned at all is his choice (H-14)
NEW: until the codec test is done no such command is planned or built; whether it is ever planned is his choice, put to him again only if the test shows that his ordinary clips wait (H3-2), and why such copies would be made at all (his question, BF259) is answered in the ANSWER block with the key 239
HIS: BF259 "Why would we make HAP copies at all?"
WHY: It left the command to his choice under H-14; he answered that choice with a question, so it stays open and nothing of it is planned.
@@END
@@AMEND G1 3
OLD: scrubbing in Review
NEW: scrubbing in Studio
HIS: BF245 "Let's go with studio. That's perfect."
WHY: The screen was called Review; he named it Studio in this round.
@@END
@@AMEND G1 4
OLD: APPEND
NEW: (7) The codec test (his words, BF240: "after we build the app can we test encoding and decoding of different codec to have a baseline of what codecs work good on this mac m1 32gb?"): yes. It runs after the app is built, on this Mac, with his own clips. Decoding: each test clip as the ordinary file it is and as copies in HAP, ProRes and Motion JPEG, and a DXV file of his if it opens; for each, whether it plays forwards without losing pictures, alone and on several layers at once; how long the first right picture waits after a trigger, a preview, a jump, a Random beat, a Beat Repeat, a turn to backwards play and a scrub in Studio; and the size of the file. Encoding: every codec the app can record in, for Record to Clip and the low-resolution show recording (topic E); for each, whether the output loses pictures while the recording runs, the size of the files, and how the recorded clip then plays and jumps. The test copies are made by Harmony, of the test clips only, in a folder of their own. The result is one short list for him of which codecs work well on this Mac; no number is promised before it, and the codec the app favours (his earlier words, binding-decisions.md 650-651: "We should pick the most optimal Kodex to use") is picked from that list with him.
HIS: BF240 "after we build the app can we test encoding and decoding"
WHY: The rule knew only a jump measurement on two clips; he asks for a wider test of encoding and decoding after the build, which the rule did not hold.
@@END

## ASSUMPTIONS
@@ASSUME H3-1
ABOUT: 238, 267, R218, R193
TEXT: I assume the end is cut the first time you switch a clip to BPM mode, counted from its clip in point. Back in Timeline mode the clip keeps that cut end until you drag the clip out point out.
WHY: BF258 picks the cut but not its moment; item 267 (accepted) brings every video in in Timeline mode, so no clip is "new in BPM mode" when it arrives.
ALT: b) Timeline mode always plays the whole clip again; the cut holds only in BPM mode. c) The end is cut the moment the clip comes into the show, in Timeline mode too.
IF-WRONG: SMALL a clip switched to BPM mode and back would lack, or have, its last seconds; one drag of the clip out point mends it
ASK: LINE a real choice of Harmony's about when his clips lose their end; he would most likely wave it through
@@END
@@ASSUME H3-2
ABOUT: 239, G1
TEXT: I assume we make no HAP copies and build no command for them. Only if the codec test after the build shows that a jump, backwards play or a random beat waits on your ordinary clips do I ask again.
WHY: BF259 is a question, not a choice: "Why would we make HAP copies at all?"; with BF240 the test that could give a reason comes after the build.
ALT: b) Never, whatever the test shows: your clips play as they are.
IF-WRONG: STAGE on ordinary video files a Random jump or backwards play may wait in front of an audience; or a converter is built that he never needed
ASK: YES he asked back; the item returns in a form his question shows he can answer
@@END
@@ASSUME H3-3
ABOUT: 239, G1, answer codec
TEXT: I assume the codec test comes once every function is built and works, and nothing is timed before it. It compares your clips as they are with copies in HAP, ProRes and Motion JPEG: playing, jumping, recording, file size.
WHY: BF240 says "after we build the app" and "different codec": it names neither the codecs nor whether the small jump timing the page promised "first" also waits.
ALT: b) I also time the jumps on two of your clips earlier, while Random and Beat Repeat are being built. c) Other codecs belong in the test: name them.
IF-WRONG: SMALL the test is run again with another codec, or a timing comes later than it could have
ASK: LINE the order and the list are Harmony's picks; his answer (after the build) is the text, the earlier timing is Harmony's advice as way b
@@END
@@ASSUME H3-4
ABOUT: 267, R193
TEXT: I assume a clip of many pictures you bring in starts in Timeline mode too, like a video.
WHY: Item 267 as he read it names only "a video"; the assumption it was made from (H-15) named both.
ALT: b) A clip of many pictures starts in BPM mode, each picture on the beat.
IF-WRONG: SMALL a starting value he changes with one click
ASK: NO the same rule for the only other kind of clip that has playback
@@END
@@ASSUME H3-5
ABOUT: 239, G1
TEXT: I assume I pick the clips for the codec test from your shows, one full HD and one 4K, and make the test copies myself in a folder of their own. None of your files is changed.
WHY: BF240 asks for the test; it does not say which clips or who makes the copies.
ALT: b) You name the clips for the test.
IF-WRONG: SMALL the test is run again on other clips
ASK: NO how a test is prepared; he sees only its result
@@END
@@ASSUME H-10
ABOUT: 238, R218
TEXT: I assume the clip is counted at 120 BPM: a group of 4 bars is 8 seconds of it, so at tempo 120 it plays at exactly its normal speed. A clip under 8 seconds gets 16 beats, uncut.
WHY: BF258's way b does not say at which tempo the groups are counted; his earlier words: "default all bpms to 120", "not current bpm", "a short clip can have 4 bars" (binding-decisions.md 684, 673, 846).
ALT: b) A clip under 8 seconds gets fewer beats (4 or 8), so that it plays near its normal speed.
IF-WRONG: SMALL the starting number of a short clip differs; he sets it himself anyway
ASK: NO internal; his earlier words name 120, rule out the tempo of the moment and give a short clip 4 bars
@@END
@@ASSUME H-4
ABOUT: 202
TEXT: I assume tempo stop, which takes every clip off its layer, also forgets where each clip was: the next trigger starts it from the start, whatever its start menu says. A BPM-mode clip that continues starts on its nearest earlier beat marker.
WHY: BF271 ("Stop removes all clips from all layers") says every clip leaves its layer at a stop; it does not say whether a clip set to Continue then remembers its place.
ALT: b) Tempo stop leaves each clip's place alone, so a clip set to Continue picks up where the stop took it off. c) In BPM mode the start menu is not used: such a clip always starts from the start.
IF-WRONG: SMALL only clips whose start menu he changed are touched; one reset more or less
ASK: NO an edge of a menu that stands on Restart until he changes it; small to change
@@END
@@DROP ASSUME H-14
WHY: It was put to him as item 239; he answered with a question (BF259), and BF240 moves the measurement behind the build. Its text ("First I measure ... Only if the wait shows do I add a Convert for performance command") no longer holds; H3-2 takes its place.
@@END

## QUESTIONS BACK
@@ANSWER 239
HIS: BF259 "Why would we make HAP copies at all?"
ANSWER: Only if your ordinary clips are seen to wait; if nothing waits, never. An ordinary video file stores a few pictures whole and the others only as changes from the picture before. On a jump, on backwards play or on a random beat the app must first rebuild the picture from the last whole one, which can show as a short wait. A HAP file stores every picture whole, so nothing is rebuilt; its price is a much bigger file (my estimate). The test after the build shows it.
@@END
@@ANSWER codec
HIS: BF240 "after we build the app can we test encoding and decoding of different codec to have a baseline of what codecs work good on this mac m1 32gb?"
ANSWER: Yes. Once the app is built I run that test on this Mac with your own clips, and you get one short list of the codecs that work well here. Decoding: each clip as it is and as copies in HAP, ProRes and Motion JPEG: does it play without losing pictures, alone and on several layers, and does a jump, backwards play or a random beat wait. Encoding: every codec the app can record in: does recording disturb the output, how big are the files. No numbers before the test.
@@END

## NAMES
@@NAME codec test
MEANS: The test after the app is built that compares, on his Mac and with his own clips, how video codecs play, jump and record, and gives him one short list of the codecs that work well.
SOURCE: Harmony's pick, from his words (BF240: "test encoding and decoding of different codec to have a baseline")
@@END

## REACHES OTHER TOPICS
- E, item 232 way c (BF254 "c"), must agree with item 238 here: ONE kind of cut. In both, the clip out point is moved in to the end of the last whole group of 4 bars, nothing is taken out of the file ("the rest stays in the file"), and the rest plays again when he drags the clip out point out. One difference E has to rule: a clip made with Record to Clip is counted in the bars the beat clock really ran while it was recorded (5 recorded bars = 4 kept, Beats 16), NOT at 120 BPM as a clip he brings in (238, H-10); counted at 120, a clip recorded at any other tempo would get the wrong Beats. Also E's to rule: a recording shorter than 4 bars, and whether such a clip is born in BPM mode (item 267 speaks only of "a video you bring in"; a clip recorded on the beat that came in in Timeline mode would not loop its 4 bars in time).
- E, BF241 ("we should test which codecs work best"): the encoding half of the codec test is ruled here (item 239 point 2, G1 point 7); E names which codecs and sizes of the low-resolution show recording and of Record to Clip go into it, and refers here for the test.
- A, BF271 ("Stop removes all clips from all layers"): A owns the rule. Applied here only as the ground of H-4: after a tempo stop every clip has been away from its layer, so item 202's start menu acts at the next trigger.
- I, BF260 ("All the blend modes and keying stay ..."): I owns item 240 and the conflict with his L111. Applied here to this topic's own old blocks R194 (f) and P28, which had the keying and its K slider leave the layer strip.
- J, BF245 and BF263 (Studio): applied here to G1 ("scrubbing in Studio"). .harmony/NAMES.md row "Review" is J's to change.
- B, BF247 ("Previewing a clip ... should just be quick"): how fast a previewed clip shows its first picture is one of the waits the codec test times (item 239 point 2); B owns the preview rule.
- F, the list of what a copied clip carries (old G2): a copy carries its Beats and its clip out point as they stand; a copy is not counted and cut a second time (238 point 6).
- A and B, item 266: the restart of an already playing clip is ruled here and in item 202; A's R128 d (a column trigger of a playing column) and D's BF251 ("plays again only when that clip is re-triggered") lean on it: a re-trigger is a restart.

## CONFLICTS
- 238, no conflict to put to him, noted because it looks like one: BF258 "b" (whole groups of 4 bars, the end cut in) against his L99 "R218 do beats here. It was my mistake before" (binding-decisions.md 1177) and his look in his Resolume, L21 (spec-H R218: "By default it gives me a decent amount of beats (sometimes 8, sometimes 16)"). The row still reads and steps in beats (L99 stands); only the FIRST number follows his rule of 2026-10-04 (binding-decisions.md 812-813: "always work with multiples of 4. If it is uneven, then move the outpoint in to keep those multiples."), which BF258 confirms. In one line for him: "Your clips in BPM mode will not start like your Resolume's (8 or 16 beats, nothing cut): they get whole groups of 4 bars and lose the rest of their end, as you chose."
- R194 f and P28 (topic I owns it): BF260 "All the blend modes and keying stay, and they will be built when there is time." against binding-decisions.md 1185: "204 I want to remove the keying and slider. We are only going to use the transparency slider to control that layers blend mode." His newest words win; amended here in this topic's two old blocks.
- 239, not a conflict of his words: the page's "First I measure" was Harmony's text; his BF240 puts the test after the build. His earlier "We should pick the most optimal Kodex to use and not worry about the other ones" (binding-decisions.md 650) is served by the test's result, not contradicted.

## NOT DONE / UNSURE
- 238: the size of the cut was not on the page. A clip of 15 seconds keeps 8; a clip of 23.9 seconds keeps 16: up to just under 8 seconds of every clip longer than 8 seconds goes from its end in BPM mode (arithmetic at 120 BPM, not measured). He chose the cut and it is his own rule of 2026-10-04; it is not asked again. If the page ruling wants it said once, the line is in CONFLICTS.
- 238: "exactly its normal speed" is true at a tempo of 120 only; at 128 the same clip plays faster. That is BPM mode itself and his "not current bpm"; carried in H-10 (not asked).
- 239: nothing is measured. Whether a jump on his ordinary clips waits long enough to be seen is UNKNOWN until the codec test; Random, Beat Repeat and backwards play are therefore built without that knowledge unless he takes H3-3 way b. Cheapest way to know earlier: way b (two clips, one session, while those functions are built).
- 239: whether the FFmpeg on this Mac can write HAP (needed only for the test copies) and whether a DXV file of his opens: not checked. Cheapest: one look at the encoder list and one file tried, in the session of the test.
- The name "Convert for performance" (old spec-H NAMES) is not planned any more; it is not in .harmony/NAMES.md (grep: no hit), so nothing is to be taken out there. The lint's DROP takes a one-word id, so no @@DROP NAME is written for it.
- Not amended, wording only: old RULE lines of this topic still say "fire / fired" where his word is "trigger" (202, R192, R193, R194, G1).
- Old assumptions of this topic tested against his 33 boxes and left untouched: H-2, H-6, H-7, H-9, H-11, H-12, H-13. Changed and kept under their ids: H-4 (BF271), H-10 (BF258).

Written 2026-10-09 18:48:40 EDT by the architect seat of topic H. Read-only; nothing built, run or committed.
