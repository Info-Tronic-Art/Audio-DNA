# SPEC H -- How a clip plays (s-rta-1009): the s-rta-1007 spec with Boris's answers to page 2 laid over it by wf/merge3.py (paper apply-H.md, ruling rule-H.md). This file wins over every older one.

## PAGE 2 ITEMS (the rule now for each item of page 2 in this topic)
@@ITEM 238
TITLE: The first Beats of a clip in BPM mode: groups of 4 bars
STATUS: ANSWERED way b
HIS: BF258 "b"
RULE: The page's way b, in full: "Its end is cut in so that the clip is whole groups of 4 bars and plays at exactly its normal speed." It is his own rule of 2026-10-04 (binding-decisions.md 812-813: "always work with multiples of 4. If it is uneven, then move the outpoint in to keep those multiples."). Built so: (1) The part of the clip between its clip in point and its clip out point is counted at 120 BPM, never at the tempo that is running (his earlier words, binding-decisions.md 778-782: "To create a 120 BPM, we figure out some kind of math and look at how resolution does it. It creates a nice in and out point and even amounts of bars in the timeline for 120 bpm."; 673: "not current bpm"; 684: "default all bpms to 120"): at 120 BPM a group of 4 bars is 16 beats and lasts 8 seconds. (2) Beats becomes 16 times the number of whole groups that fit into that part: 16, 32, 48, 64 and so on. (3) The clip out point is moved in to the end of the last whole group, on a whole picture, so that at Speed 1 and a tempo of 120 the clip plays at exactly its normal speed; at any other tempo it plays faster or slower with the tempo, as every clip in BPM mode does. (4) Nothing is taken out of the file: the cut-off end stays in it, the clip out point can be dragged out again as far as the end of the file, and what he drags back in plays again; then the same number of Beats covers more video and the clip looks faster (item R193 g as it stands; his words, binding-decisions.md 780-782: "if the user pulls the outpoint out, and the same amount of bars, it covers more video appearing to play faster"). (5) A part shorter than one group (under 8 seconds) gets 16 beats and is not cut: it starts slower than normal, and Speed makes it fast. That is his "47 default" (binding-decisions.md 865) to the question whose default read "Yes. 4 bars is the least; bar counts are 4, 8, 12, 16 ... A short clip starts slow and I use Speed."; his first words on it were "a short clip can have 4 bars but it will move super fast" (846), and that question set "fast" right. (6) A part that is short of a whole group by less than a tenth of a beat (0.05 seconds) counts as that whole group and is not cut; its pace then differs from normal by less than one percent (arithmetic). This tolerance is Harmony's, not his word (H-10). (7) The number and the cut are worked out once, the first time the clip is switched to BPM mode (that moment is Harmony's reading: H3-1); after that the app never moves the clip out point or changes Beats by itself. A clip that is playing when it is switched: H3-6. (8) A clip that comes with its Beats is not counted: a clip made with record to clip gets its mode, its Beats and its clip out point from its recording (topic E, item 232), and a copy of a clip carries those of its original (topic F). A clip of a deck saved before this build that is already in BPM mode: H3-7. (9) Everything else of the Beats row is item R218 (a) as it stands: "-" and "+" change Beats by 1, "/2" and "x2" halve and double it, any whole number is allowed, and none of them moves the clip out point. (10) A clip of many pictures has no normal speed, and nothing of it is cut: its Beats becomes 16 times the number of whole groups that fit into its length at its Timeline-mode rate, at least one group, and all its pictures are spread over them (item 211 of topic K, as it stands). That no picture is cut is not his word: it is topic K's reading and is put to him there (K3-5); the arithmetic is H-10.
CHANGED: The page's own text (4, 8, 16, 32 ... nearest to normal speed, the end never cut; the way of his Resolume, old assumption H-5) is replaced by its way b. Against the old blocks: R218 (b) was open and is now settled (AMEND R218 1); C13 said nothing of the first number is built before his reply (AMEND C13 1); H-10's "nearest, judged as a ratio" and "never under 4" fall away (H-10 rewritten). His words, not Harmony's: the cut, the groups of 4 bars, the count at 120 BPM, the short clip. Harmony's, each flagged: WHEN the cut is made (H3-1), a clip switched while it plays (H3-6), the tolerance and the count of a clip of many pictures (H-10), a clip of an earlier saved deck (H3-7); that a clip of many pictures loses no picture is topic K's reading (K3-5). By the ruling: point 5's quotes set right ("fast" was his first word, "slow" the default he then took); points 6 and 8 added; point 10 rewritten (the paper counted a clip of many pictures like a video, which would have cut pictures off its end).
TODAY: Every new clip starts at 4 beats / 4 beats; nothing sets a length from the file or the tempo and nothing moves an out point by itself (Clip.h:36-41 and area-clip-transport.md part 1 item 10, as cited in spec-H R218 TODAY; not re-read). To build: the count, the Beats number, the moved clip out point, at the first switch to BPM mode.
@@END

@@ITEM 239
TITLE: No codec; HAP copies wait on his answer and the test
STATUS: OPEN he asks back why HAP copies at all
HIS: BF259 "Why would we make HAP copies at all?"; BF240 "after we build the app can we test encoding and decoding"
RULE: Waits on his answer: the reading that is put to him again (H3-2) is that no HAP copy is ever made and no command for it is built, unless the codec test shows that his ordinary clips wait. In full: (1) BUILT: no codec of our own. No command that makes copies of his clips, and none is planned. No file of his is converted. Every video file plays as it is; a file he brings that already stores every picture whole (HAP, ProRes, Motion JPEG; a DXV file if it opens) plays like any other file (item G1 points 1 and 2, as amended). Random, Beat Repeat, backwards play, a trigger into a long video and scrubbing in Studio are built to work on his ordinary files as they are. (2) TESTED, after the app is built (his words, BF240): the codec test, on this Mac (M1, 32 GB), with his own clips. It starts with the files he has: which codecs they are in, and whether DXV files among them open and play. Decoding: each test clip as the file it is and as copies in HAP, ProRes and Motion JPEG; for each, whether it plays forwards without losing pictures, alone and on several layers at once; how long the first right picture waits after a trigger, a preview, a jump, a Random beat, a Beat Repeat, a turn to backwards play and a scrub in Studio; and how big the file is. Encoding: every codec the app can record in, for record to clip, for the low-resolution show recording and for a film made with Render in Studio (topic E names them and refers here; BF241: "we should test which codecs work best"; a clip made with record to clip needs no see-through channel, BF268: "This is exactly the output recorded as one layer."); for each, whether the output loses pictures while the recording runs, how big the files are, and how the recorded clip then plays and jumps. The test copies are made by Harmony, of the test clips only, in a folder of their own (H3-5); which codecs are compared is Harmony's list (H3-3). The result is one short list for him: which codecs work well on this Mac. No number is promised before it. (3) THEN: only if the test shows that a jump, backwards play or a Random beat waits on his ordinary clips is the choice of fast copies put to him again, with the test's result beside it; if nothing waits, it is never raised again. (4) WHERE THE MEASUREMENT STANDS: the page said "First I measure, on two of your clips". No choice of his waits on a measurement before the build any more: that measurement is the jump part of the codec test, after the build (BF240). What a builder times while building and checking Random, Beat Repeat and backwards play is the build's own checking and not this test; a wait he would see is told to him when it is found, with its number.
CHANGED: His box holds a question, no letter: none of the three ways is taken. The page's "only if you would notice the wait do I add a command that makes HAP copies" becomes "no command; asked again only if the test shows a wait". The page's "First I measure" moves to after the build (BF240). "No codec of our own" stands: neither of his two boxes speaks against it (INFERRED consent). Against G1: its first sentence (where the answer stands), its point 2 (HAP first, "the one optimal codec"), point 3 (measured in the first build session) and point 4 (a command "Convert for performance" planned if the wait shows, his choice H-14) change (AMEND G1 1, 2, 4, 5, 6); H-14 is replaced by H3-2. By the ruling: the film made with Render in Studio joins the encoding half (topic E sends it to this test); the test starts with the codecs of the files he has, DXV among them; point 4 no longer reads his "after we build the app" as a ban on a builder's own timing.
TODAY: Video is decoded by FFmpeg on the CPU, one decode thread per clip; HAP, ProRes, Motion JPEG and DNxHD are treated as every-picture-whole; a jump on an ordinary file re-seeks to a keyframe and catches up, and how long that takes on his clips is UNKNOWN (spec-H G1 TODAY: VideoPlayer.cpp:117, :290; pitfalls 56, 62; not re-read). The app records H.264, ProRes and MJPEG (CLAUDE.md, project identity). This Mac reports "Apple M1 Pro" and 34359738368 bytes of memory (sysctl, read this session). Whether a DXV file of his opens: INFERRED, not tried. The local FFmpeg's HAP encoder may be missing (check-codec.md fact 11, as cited in G1 TODAY): to be checked before the test copies are made.
@@END

@@ITEM 266
TITLE: Triggering the playing clip always restarts it
STATUS: ACCEPTED
HIS: box left empty = accepted as written
RULE: Triggering the clip that is already playing on its layer always restarts it, whatever its start menu says: also when the menu stands on Continue or Relative, and also when the trigger comes from its column (his earlier words, binding-decisions.md 616: "Firing a column that is already playing should restart its videos"). The start menu acts only when a clip comes back to its layer after it has been away (@@ITEM 202 as it stands). Everything around it stands as ruled: where a restart begins (the clip in point; for a clip set to play backwards its end, the clip out point: @@ITEM R193 (c)); when it begins for a clip in BPM mode (it plays on and starts again on the next 1: topic A, @@ITEM R128 (d), with item 249); and that a clip he paused himself still shows on its paused frame when triggered and stays paused (@@ITEM 202).
CHANGED: nothing of his: accepted as written. Item 202 already rules it in the same words; old assumption H-3 is closed by it. By the ruling: the pointers now lead to the items that hold each rule (the paper sent all three to item 202, which holds only the last).
TODAY: A re-trigger through the clip's cell seeks to the in point; a column re-trigger does not restart (area-clip-transport.md part 1 items 12 and 13, as cited in spec-H 202 TODAY; not re-read). To build: the column path restarts too.
@@END

@@ITEM 267
TITLE: A video comes into the show in Timeline mode
STATUS: ACCEPTED
HIS: box left empty = accepted as written
RULE: A video he brings into the show starts in Timeline mode: it plays at its own Speed and Duration and starts the moment it is triggered. He switches the clips he wants on the beat to BPM mode himself; there is no setting that says which mode new clips start in. The mode is the clip's own setting. A clip of many pictures comes in the same way (Harmony's reading: H3-4; item 211 of topic K says the same). At its first switch to BPM mode the clip gets its Beats and its cut end (item 238; that the cut is made at that moment is Harmony's reading: H3-1). A clip made with record to clip is not a video he brings in: topic E rules its mode and its Beats (item 232).
CHANGED: nothing of his: accepted as written. Old assumption H-15 is closed by it; the rule is added to R193, which did not name the starting mode (AMEND R193 1). By the ruling: the moment of the cut is marked as Harmony's reading (H3-1); "clip by clip" and "saved with the show" are taken out of the RULE (neither is in the text he accepted; what the show file holds is topic F's); the clip made with record to clip is set apart (topic E's paper makes it a clip in BPM mode from the start, item 232).
TODAY: A new clip is in Timeline mode (Clip.h:118-119 and area-clip-transport.md part 1 item 1, as cited in spec-H R193 TODAY; not re-read). Nothing to change for the starting mode.
@@END

## ITEMS (the items of the first page; a RULE that page 2 changed carries "[page 2, ...]" marks where it changed)
@@ITEM 201
TITLE: Where a Random jump lands
STATUS: ANSWERED in his words; they fit option B; his earlier "bars only" is asked (H-1)
HIS: L101, L20, L21, L13
RULE: His words (L101): "each of the markers on a clip are bpm lines and the random lands on one of them"; his look in his own Resolume (L20): "it randomly moves playhead every single beat. When it changes, it jumps to random beat markers on the clip." Built so: a clip in BPM mode whose loop menu is on Random jumps by itself, on a beat, and every jump lands exactly on one of the clip's beat markers: any whole beat of the clip between its in and out points, not only the first beat of a bar (the page's option B). Between jumps the clip plays on from where it landed, its beat markers on the beat; a jump is never corrected afterwards. The rows Interval and Distance are as in his Resolume picture, both counted in beats, each with "-" and "+": Interval says how often the clip jumps, Distance how wide the range is from which the landing beat is picked (Resolume's manual: "how often the playhead will jump to a new random point" and "the range from which a random point will be picked"; the edges and the starting values: H-2). While the tempo bar is paused a clip on Random holds with every other BPM-mode clip and does not jump (L13: "Pause pauses, the beat clock, the clips and everything that it controls with BPM."). That the markers are beat lines, one per beat on the clip's timeline, is the reading of L101 with L20 and L21 ("Bpm mode the clip’s timeline is divided into beat markers."); it sits against his earlier "Timeline only shows bars": asked in H-1. Random on a clip in Timeline mode: H-9.
CHANGED: Option A (the default: every jump lands on a "1", the first beat of a bar of the clip) is replaced by his words, which fit option B (any whole beat). "bpm lines" read as beat lines / beat markers: INFERRED from L20 and L21, where he says "beat markers". Taken out of the paper's rule: that Interval 1 "is what he saw" (L20 names no Interval; his picture of 2026-10-04 shows Interval 1 and Distance 2).
TODAY: No Random loop style exists (area-clip-transport.md part 1 item 3: the list reads Loop / Ping Pong / One Shot; grep finds no random loop mode). The adopted transport ruling (RA-8) jumps whole beats within a Distance, with lists counted in bars: to be re-cut to beats (U-H11). The clip timeline draws one line per beat division now (part 1 item 7, ClipInspector.cpp:1265-1276). A jump on an ordinary long-GOP video file re-seeks to a keyframe and catches up (pitfall 56); how long that wait is on his clips is not measured (item G1).
@@END

@@ITEM 202
TITLE: The two small menus are modelled after Resolume
STATUS: ANSWERED in his words; they fit option B
HIS: L102, L12, L11
RULE: His words (L102): "model these 2 little menu’s after resolume". Both small menus at the right of the clip's transport panel are built as in his Resolume. (1) The loop menu: the five entries of item R217. (2) The start menu, which decides where a clip starts when it is fired after it has been away from its layer (Resolume's manual: "These buttons decide what happens when a clip is triggered when you've been away from it for a bit."). It has three entries, with the manual's meanings: "Restart" = from the start: the clip starts at its in point; "Continue" (the manual's "pick-up") = the clip starts where it was when it last left its layer; "Relative" (the manual's "relative pick-up") = the clip starts at the same relative position that the clip which played before it on that layer had reached, so halfway through stays halfway through (H-11). Every clip is set to "Restart" until he changes it (the page's option B; Resolume's own default). A clip with nothing to continue from (never played, or no clip before it on the layer) starts from the start. The setting belongs to the clip and is saved with the show. Wherever another rule says "a fired clip starts from its beginning", it now reads "unless the clip's start menu says otherwise". WHEN a clip starts is not touched by this menu: a clip in BPM mode waits for the "1" (L12: "when a new clip with BPM mode is triggered, it waits for the one"), a clip in Timeline mode starts on the press (L11; when it replaces a BPM-mode clip in mid-bar: topic A, A-4). Firing the clip that is already playing restarts it whatever the menu says (his earlier words, binding-decisions.md 616: "Firing a column that is already playing should restart its videos"; shown to him as H-3). A clip he paused himself still shows on its paused frame when fired (his earlier words, binding-decisions.md 748, 806 and 904). The first fire after the tempo bar's Stop, and where a BPM-mode clip that continues lands: H-4.
CHANGED: Option A (the default: no such menu, every fire starts the clip from its beginning; from his "42 default") is replaced by his words of L102, which name both menus. His earlier "42 default" (binding-decisions.md 844) and "restart" (668) now hold only as the menu's starting setting. The page's wording for the third entry ("carrying on as if it had never stopped") is replaced by the meaning in Resolume's manual, because L102 says "after resolume" (facts-resolume-transport.md:54, confirmed at :140).
TODAY: A second dropdown "Restart / Continue / Relative" is on screen and does nothing: no handler, no model field (area-clip-transport.md part 1 item 4; ClipInspector.cpp:69-73). A clip that leaves its layer freezes and RESUMES when fired again through a column or after leaving (part 1 item 14); a re-fire through its cell seeks to the in point (item 12); a column re-fire does not restart (item 13). So: a model field per clip, saved; the fire path reads it; the adopted plan left the menu out (RD2 D2-1) and has to take it back in. Whether Resolume honours "Continue" on a BPM Sync clip is NOT DOCUMENTED for version 7 (facts-resolume-transport.md:56; version 4 ignored it).
@@END

@@ITEM R192
TITLE: Random and Beat Repeat: lists and units
STATUS: STANDS
HIS: none
RULE: (a) Beat Repeat's buttons read Off, 4, 2, 1, 1/2, 1/4, 1/8, 1/16, 1/3, 1/6 and "Catch up", and the numbers count beats, as in his Resolume picture. It is offered on a clip in BPM mode only, as in Resolume (H-9). (b) Random shows Interval and Distance, counted in beats on a clip in BPM mode; where a jump lands is item 201 (any beat marker); what the two rows do is H-2. (c) Beat Repeat is a thing of the moment: it repeats from where the clip is, it goes off when the clip is fired again or leaves the layer, and it is not saved with the show. With "Catch up" lit, switching it off puts the clip where it would have been without the repeat (Resolume's manual); without it the clip carries on and is cut into time on the next "1" (his "124 a", binding-decisions.md 946; what "into time" means: H-12).
CHANGED: nothing replaced by his words; the open point in (b) is closed by item 201. His L99 ("do beats here") agrees with counting in beats. Added by the ruling: where Beat Repeat and Random are offered (H-9) and what "Catch up" does (Resolume's manual, facts-resolume-transport.md:66).
TODAY: No Beat Repeat and no Random exist (area-clip-transport.md part 1 header: grep finds neither). The adopted lists of RA-8 are in bars and lack 1/3 and 1/6 (O13, U-H12): re-cut to the picture. That Resolume shows its BeatLoopr only in BPM Sync is from the fact sheet (facts-resolume-transport.md:66), not seen by me.
@@END

@@ITEM R217
TITLE: The loop menu; Eject against a next clip
STATUS: STANDS confirmed by his words
HIS: L98, L102
RULE: The loop menu of a clip reads the same five entries in BPM mode and in Timeline mode, as in his Resolume (L102): Loop; Ping Pong (forwards, then backwards, again and again); Random (the clip jumps by itself: item 201; in Timeline mode: H-9); Play Once and Eject (it plays once, then the clip leaves its layer and the layer is empty); Play Once and Hold (it plays once and stays on its last frame). His words (L98): "if a clip plays once and ejects that is clear the layer. If the clip plays on autopilot and the next clip plays then the eject would be cancelled by the next clip appearing." So: at the end of a Play Once and Eject clip the layer is cleared, exactly as if its clip had been taken off; when the autopilot brings a next clip at that moment, the next clip appears and no empty layer is seen. Eject empties the layer only when nothing follows. "Autopilot" covers both ways a next clip comes by itself, the layer's autopilot and the clip's own list of what comes next: his words name the autopilot; that both are meant is the text of this reading, which he read and left standing (inferred consent), and topic K rules how each works. The ejected clip stays in its cell. How the autopilot's next clip starts, and a clip that ends while the tempo bar is paused, belong to topic K (K-16).
CHANGED: nothing replaced: his words say the same as the page's "the next clip wins, and Eject empties the layer only when nothing follows". "that is clear the layer" read as "that clears the layer": INFERRED (typing). Made explicit by the ruling: the same five entries in both modes (L102; H-9).
TODAY: The loop list reads "Loop", "Ping Pong", "One Shot" (ClipInspector.cpp:56-58; area-clip-transport.md part 1 item 3). "One Shot" becomes two entries; Eject has to clear the layer's clip; the autopilot calls Layer::triggerClip from the render thread (Autopilot.cpp:326, 355, 417; U-H22), so the order "next clip before the clear" has to be fixed there.
@@END

@@ITEM R218
TITLE: The length row of a BPM-mode clip reads Beats
STATUS: STANDS part a confirmed by his words; part b open, asked (H-5) -- AMENDED after page 2 (R218 1 by H)
HIS: L99, L21
RULE: His words (L99): "do beats here. It was my mistake before"; his look in his Resolume (L21): "There is no duration on a bpm clip, but it has beats. The beats regulate how many beats are on the clip." and "Pressing plus and minus increase or decrease by one beat and there is /2 and x2, which double and halve." Built so: (a) a clip in BPM mode has no Duration row; it has a row "Beats" with "-", "+", "/2" and "x2". "-" and "+" change the length by 1 beat; "/2" and "x2" halve and double it. The number says over how many beats of the tempo the clip plays once from its in point to its out point at Speed 1; more beats = slower. Any whole number from 1 up is allowed (H-6): 16 becomes 17 with one press, and a clip of 17 beats drifts against the bars each time it loops (he was shown this sentence and kept beats); the app leaves that drift alone (H-12). (b) The number a new clip starts with is settled by his answer to item 238 (BF258: "b", the page's way b: "Its end is cut in so that the clip is whole groups of 4 bars and plays at exactly its normal speed."), which is his rule of 2026-10-04 (binding-decisions.md 812-813: "always work with multiples of 4. If it is uneven, then move the outpoint in to keep those multiples."). The part of the clip between its clip in point and its clip out point is counted at 120 BPM, never at the tempo that is running (his earlier words, binding-decisions.md 778-782: "It creates a nice in and out point and even amounts of bars in the timeline for 120 bpm."; 673: "not current bpm"; 684: "default all bpms to 120"), where a group of 4 bars is 16 beats and lasts 8 seconds. Beats becomes 16 times the number of whole groups that fit (16, 32, 48, 64 ...), and the clip out point is moved in to the end of the last whole group, on a whole picture, so that at Speed 1 and a tempo of 120 the clip plays at exactly its normal speed. The cut-off end stays in the file; the clip out point can be dragged out again as far as the end of the file, and what he drags back in plays again. A part shorter than one group (under 8 seconds) gets 16 beats and is not cut; it starts slower than normal and Speed makes it fast (his "47 default", binding-decisions.md 865, to the question whose default read "Yes. 4 bars is the least; bar counts are 4, 8, 12, 16 ... A short clip starts slow and I use Speed."; his first words on it, 846: "a short clip can have 4 bars but it will move super fast"). A part that is short of a whole group by less than a tenth of a beat counts as that whole group and is not cut (Harmony's tolerance: H-10). The number and the cut are worked out once, the first time the clip is switched to BPM mode (that moment is Harmony's reading: H3-1; a clip switched while it plays: H3-6); after that the app never moves the clip out point or changes Beats by itself, and "-", "+", "/2" and "x2" never move the clip out point. A clip that comes with its Beats is not counted: one made with record to clip (topic E, item 232), a copy of a clip (topic F); a clip of a deck saved before this build that is already in BPM mode: H3-7. The way of his Resolume (4, 8, 16, 32 ..., nothing cut; L21) and whole single bars are not built. A clip of many pictures has no normal speed and nothing of it is cut: its Beats becomes 16 times the number of whole groups that fit into its length at its Timeline-mode rate, at least one group, and all its pictures are spread over them (item 211 of topic K); that no picture is cut is topic K's reading, put to him there (K3-5), and the arithmetic is H-10. [page 2, H: BF258 "b"] The Speed row in BPM mode: item R193 e.
CHANGED: (a) stands; the invitation "Say R218 bars" is declined by him: his earlier "lets do bars here not beats" (binding-decisions.md 761) is withdrawn for this row. (b) not named by him; open because of L21 and because L99 calls the bars regime his mistake.
TODAY: Every new clip starts at 4 beats / 4 beats; nothing sets a length from the file or the tempo (Clip.h:36-41; area-clip-transport.md part 1 item 10). BPM Sync shows a combo "Beats/ Cycle" and a slider "Content Beats" (item 7); a "Duration" row labelled "Beats" in BPM Sync does nothing (item 6). The adopted rulings RD2 / RA still say Bars in 4, 8, 12, 16 with "/2" greyed: to be re-cut (U-H10). Resolume's own rule for the first number is disputed between its manual (nearest power of 2) and its staff (multiples of 4, or of 8 or 16); all agree it counts at 120 BPM and changes the speed, not the in and out points (facts-resolume-transport.md:24-30).
@@END

@@ITEM R193
TITLE: How a clip plays: the rest
STATUS: REPLACED in parts c and g by his words elsewhere; part h asked (H-1); parts a, b, d, e, f stand -- AMENDED after page 2 (R193 1 by H)
HIS: L102, L21, L101, L13, L11, L12, L17, L31
RULE: (a) Only a video and a picture sequence have playback (BPM mode or Timeline mode, speed, pause, backwards, the loop menu, the start menu). (b) A still picture, a generated source, MilkDrop, the camera and an effects-only clip have none: they show on his press and run on their own clock; the tempo bar's pause does not freeze them, and fired during a pause they show and move. This follows the rule that pause holds what follows the beat and a clip that is not in BPM mode plays on (his "136 b", binding-decisions.md 1009; L13: "Pause pauses, the beat clock, the clips and everything that it controls with BPM."; L11: "If a layer that is not in BPM mode is triggered, then that plays instantly"). Topic A owns the pause and what a fire during a pause does to the beat (L12, L17, L31; its assumptions A-1 and A-3), and (b) follows what is ruled there. (c) A fired video starts from its beginning unless its start menu says otherwise (item 202, L102); a clip set to play backwards keeps its direction and starts from its end. (d) A clip's pause is one thing shown in two places: the Clip tab's buttons and the layer strip's "<", "||", ">" are the same buttons of the clip that plays. (e) Speed is one control shown in two places (the strip's S slider and the Clip tab's Speed). In Timeline mode it runs to 10 (the real top is measured before it is built; if it is lower he is told first). In BPM mode it has "-" and "+" and steps through nine values, as his Resolume (L21, his answer to the look he was asked for, a click on the plus of Speed): "Plus and minus doubles and halves the current. It goes: 0, 1/8, 1/4, 1/2, 1, 2, 4, 8, 16." At Speed 1 the clip plays once over its Beats; the edges (0, and how Speed and Beats work together) are in H-13. (f) The master Speed changes only clips that are not in BPM mode. (g) Timeline mode, as his Resolume (L21): "In timeline mode duration changes don’t affect the speed setting but the video plays faster or slower based on the direction moved. Same with speed. It does not affect the duration setting but make the video move slower and faster." So Speed and Duration are two separate settings: a shorter Duration plays the video faster, a longer one slower, and the Speed number stays; a Speed change plays it faster or slower and the Duration number stays. Duration's "-" and "+" step 0.1 (his earlier words, binding-decisions.md 1002). How the two combine and what pulling the out point in does to the Duration number: H-7. In BPM mode pulling the out point in keeps the same number of beats, so the clip goes through less video and looks slower (his rule, binding-decisions.md 778-782, in beats by L99). (h) What the clip's timeline shows in BPM mode: one line per beat, the reading of L101 and L21 against his earlier "Timeline only shows bars": asked in H-1. In Timeline mode it shows the plain playhead without beat lines (his earlier words, binding-decisions.md 741-742). (i) A video he brings into the show starts in Timeline mode; he switches the clips he wants on the beat to BPM mode himself, and no setting chooses the starting mode (item 267). A clip of many pictures comes in the same way (Harmony's reading: H3-4). At its first switch to BPM mode a clip gets its Beats and its cut end (item R218 b; that the cut is made at that moment is Harmony's reading: H3-1). A clip made with record to clip is not brought in: topic E rules its mode and its Beats (item 232). [page 2, H: item 267 accepted as written]
CHANGED: (c) gains "unless its start menu says otherwise" (L102; the page said every fire starts from the beginning). (g) the page's "typing a Duration changes the Speed" is replaced by his look L21 (neither number changes the other); "bars" in its BPM half becomes "beats" (L99: INFERRED). (e) the BPM half is now exact from L21 (nine steps); that the list is Speed's is INFERRED from the look he was asked for (R206 f) and from Resolume's manual, which gives the same nine values for its BPM Sync Speed (facts-resolume-transport.md:31). (h) "bars only" is under question (H-1). (a), (b), (d), (f) unchanged.
TODAY: Playback only for video and sequence (Clip.h:248; U-H24). Speed slider 0 to 4 with two halve / double buttons and a Reverse text button (area-clip-transport.md part 1 item 5); in BPM Sync a video ignores the Speed slider and the tempo never enters its rate (item 8), a sequence follows the tempo and its Speed slider (item 9, correction 8). The Duration row does nothing (item 6). Master Speed scales Timeline clips only (item 26). The strip's buttons write the clip (item 17). No beat pause exists (item 23). The timeline draws beat-division lines (item 7). A new clip is in Timeline mode (item 1, Clip.h:118-119).
@@END

@@ITEM R194
TITLE: The layer strip: fade, solo, bypass, blend
STATUS: STANDS part d confirmed by his look in Resolume -- AMENDED after page 2 (R194 1 by H)
HIS: L29, L111, L19
RULE: (a) A new layer's fade (F) is 0, a cut; F counts seconds. (b) A third clip fired while a fade is running: the new fade starts from the picture as it is at that moment. (c) Solo on a layer that holds no clip does not darken the show. (d) B (bypass) and S (solo) are controls of the output. A clip on a layer that is bypassed, or hidden by another layer's solo, plays on out of sight, so when the layer comes back it is further on; his Resolume does the same with a bypassed layer (L29: "H it plays out of sight"). (e) The X's tooltip says what the X does (it takes the clip off the layer) and makes no promise about Undo, because Undo never changes the layer strip (his earlier words, binding-decisions.md 736). (f) A new layer's Blend starts on "Add". His L111 ("I want to remove the keying and slider. We are only going to use the transparency slider to control that layers blend mode.") no longer holds: his newer words (BF260) are "All the blend modes and keying stay, and they will be built when there is time." So the keying and its K slider stay on the layer and every blend mode stays in the Blend list; when each of them is built, and the masks he adds, is ruled in topic I (item 240), and (f) follows that ruling. [page 2, H: BF260 "All the blend modes and keying stay"] What a column click does to a layer whose cell is empty: his look L19 ("D layer goes empty when a column is triggered with an empty") agrees with the page; ruled in topic A.
CHANGED: nothing replaced. (d): the look he was asked for came back as the page had it (plays on), so "R194 d hold" is not taken. The solo half of (d) follows by the same rule: INFERRED.
TODAY: A new layer shows 0.3 seconds on F and cuts anyway; a solo on an empty layer darkens the show; a bypassed layer's clip appears to stand still (slice label: Layer.h:361-374, 402-403; LayerStrip.cpp:345-346; CompositorEngine.cpp:1043-1063, 1079-1080; seen in the code, not run). The K slider is on the strip and goes (L111). The X's tooltip promises that the clip comes back with Undo (the page's R194 e).
@@END

@@ITEM R219
TITLE: The strip's thin playhead bar becomes a real jump
STATUS: STANDS
HIS: none decides it; L30 agrees
RULE: The thin bar under a layer's picture in the layer strip shows where the playing clip is, and dragging it moves the clip: the same rules as the Clip tab's timeline. The playhead never leaves the in and out points; a click outside does nothing; the picture waits while the mouse is held still; the clip plays on from where he lets go, at the same speed. For a clip in BPM mode his earlier answer "71 b" holds after the drop: it plays on and is cut once into time on the next "1" (binding-decisions.md 902; what "into time" means when Beats is not a multiple of 4: H-12). His L30 ("drag each play head to the beginning manually") speaks of playheads as things he drags, which agrees.
CHANGED: nothing.
TODAY: The strip's bar writes only the model; the renderer's write-back puts the player's real position back on the next frame, so the playhead springs back and the picture does not move (LayerStrip.cpp:979-989; area-clip-transport.md part 1 item 18; the spring-back INFERRED there). The Clip tab's timeline already seeks (item 19).
@@END

@@ITEM P28
TITLE: The layer strip as a whole
STATUS: DROPPED -- AMENDED after page 2 (P28 1 by H)
HIS: L8, L7, L73, L111, L56
RULE: laid out by Harmony where it fits in the correct area; its look is settled in the UI redesign (L8). What his words fix about its contents: a layer has an "ignore actions" toggle as well as "ignore column" (L7: "We should have ignore actions toggle on the layer as well as ignore column."; L73; ruled in topic D); the strip keeps its keying control and its K slider (BF260: "All the blend modes and keying stay, and they will be built when there is time."; ruled in topic I, item 240) [page 2, H: BF260 "All the blend modes and keying stay"]; the action buttons sit in "a small row below the clip, still within the layer" (L56, topic D). The three play buttons are on the strip as the clip's own buttons (R193 d). A clip he paused himself is told apart from one the tempo bar holds by a mark; how the mark looks is for the redesign.
CHANGED: All three drawn variants differ only in where things sit; none is drawn. The variant with the K slider is half settled by L111: there is no K slider at all.
TODAY: not checked
@@END

@@ITEM P29
TITLE: The clip's Transport panel
STATUS: DROPPED
HIS: L8, L102, L36
RULE: laid out by Harmony where it fits in the correct area; its look is settled in the UI redesign (L8). Until then the panel follows his Resolume pictures as closely as it can: the mode menu, the timeline with its markers, back / pause / play, the loop menu and the start menu at the right (L102), the rows Speed and Beats; with Random on, the rows Interval and Distance between them; Beat Repeat as a row of buttons with "Catch up". The eight Cuepoints stay and keep their name (his L36 "R179 all good except" leaves that part of the cue reading standing).
CHANGED: All three drawn variants differ only in where the rows sit; none is drawn.
TODAY: The Transport section has the mode combo, three buttons, a loop dropdown, the dead second dropdown, Speed, a dead Duration row, "Beats/ Cycle", "Content Beats", a per-clip Snap combo and eight Cuepoints (area-clip-transport.md part 1 items 1-7, 22, 24). The Snap combo goes (his 154).
@@END

@@ITEM G1
TITLE: A codec of our own, like DXV 3.0
STATUS: ANSWERED no codec of our own; one choice of his asked (H-14) -- AMENDED after page 2 (G1 1 by H, G1 2 by H, G1 3 by H, G1 4 by H, G1 5 by H, G1 6 by H)
HIS: L3
RULE: His question (L3): "Is there any reason for us to build our own codec that is optimized for our system like resolume’s DVX 3.0? Is that something that you can do reliably?" The answer he got on page 2 was no, and it stands: no codec of our own is built. Neither of his two boxes on it speaks against that (INFERRED consent): BF240 asks for a test of codecs after the build and BF259 asks why HAP copies would be made; they are answered in the ANSWER blocks with the keys codec and 239. [page 2, H: BF240 "after we build the app can we test encoding and decoding"; BF259 "Why would we make HAP copies at all?"] What stands for the build: (1) Ordinary video files play without any conversion, and no file of his is converted without his say. (2) The fast path for jumps (Random, Beat Repeat, backwards play, a fire into a long video, scrubbing in Studio [page 2, H: BF245 "Let's go with studio. That's perfect."]) is not decided before the codec test of point 7 (BF240): these functions are built to work on his ordinary files as they are, and only if the test shows that a faster format is needed is one picked from the test's list, a format that stores every picture whole and already exists (HAP, ProRes and Motion JPEG are compared in it). No new format is invented, and no copy of a clip of his is made unless he says so (BF259; item 239). The choice his earlier words ask for (binding-decisions.md 650-651: "We should pick the most optimal Kodex to use and not worry about the other ones"; "I want codecs that decode easily and play well") is made from that test, with him. [page 2, H: BF240 "after we build the app can we test encoding and decoding of different codec"; BF259 "Why would we make HAP copies at all?"] (3) Nothing is planned for it before a measurement: after the app is built (his words, BF240: "after we build the app can we test encoding and decoding of different codec to have a baseline"), as one part of the codec test of point 7, on his own clips (one 1080p, one 4K), the wait from a jump to the first right picture is measured [page 2, H: BF240 "after we build the app can we test encoding and decoding"] for the ordinary file, for a HAP copy and for the comparison copy, together with the size of each copy and the time the conversion takes (the whole list: answer-codec.md, "WHAT HAS TO BE MEASURED WHEN IT IS BUILT"). (4) Only if the wait shows is a command "Convert for performance" planned, which writes the fast copy beside the original; until the codec test is done no such command is planned or built; whether it is ever planned is his choice, put to him again only if the test shows that his ordinary clips wait (H3-2), and why such copies would be made at all (his question, BF259) is answered in the ANSWER block with the key 239 [page 2, H: BF259 "Why would we make HAP copies at all?"]. (5) Before that command is planned, the FFmpeg the app is built with is pinned and checked for the encoder it needs. (6) Unpacking on the graphics card is no part of this: it is not shown possible here and nothing is promised on it. (7) The codec test (his words, BF240: "after we build the app can we test encoding and decoding of different codec to have a baseline of what codecs work good on this mac m1 32gb?"): yes. It runs after the app is built, on this Mac, with his own clips, and it starts with the files he has: which codecs they are in, and whether DXV files among them open and play. Decoding: each test clip as the file it is and as copies in HAP, ProRes and Motion JPEG; for each, whether it plays forwards without losing pictures, alone and on several layers at once; how long the first right picture waits after a trigger, a preview, a jump, a Random beat, a Beat Repeat, a turn to backwards play and a scrub in Studio; and the size of the file. Encoding: every codec the app can record in, for record to clip, the low-resolution show recording and a film made with Render in Studio (topic E; a clip made with record to clip needs no see-through channel, BF268); for each, whether the output loses pictures while the recording runs, the size of the files, and how the recorded clip then plays and jumps. The test copies are made by Harmony, of the test clips only, in a folder of their own. The result is one short list for him of which codecs work well on this Mac; no number is promised before it, and the codec the app favours (his earlier words, binding-decisions.md 650-651: "We should pick the most optimal Kodex to use") is picked from that list with him. [page 2, H: BF240 "after we build the app can we test encoding and decoding"]
CHANGED: nothing of the page: a new point of his message, it answers no numbered item. "DVX 3.0" read as Resolume's DXV 3: INFERRED. The answer paper's own question for him (its "DEFAULT: yes, plan it") is replaced by H-14 (measure first), after its re-check.
TODAY: Video is decoded by FFmpeg on the CPU, one decode thread per clip (VideoPlayer.cpp:117, read by me; no hardware decoder is asked for: the grep of answer-codec.md fact 19 and check-codec.md fact 15). HAP, ProRes, Motion JPEG and DNxHD are treated as every-picture-whole (VideoPlayer.cpp:290, read by me); HAP kinds are named in VideoInfo.h:59-66 and a HAP file showed 270 of 270 pictures in the reverse test (docs/claude/rendering.md:94). A jump on a long-GOP file re-seeks to a keyframe and catches up (pitfalls 56, 62); how long that takes on his clips is UNKNOWN. INFERRED, not tried: a DXV file he owns opens through the same decoder lookup (VideoInfo.h:77 names "dxv"; check-codec.md fact 11 finds the name "Resolume DXV" in the local libavcodec). The Homebrew FFmpeg formula of the day lists no snappy, so the HAP encoder can vanish on an upgrade (check-codec.md fact 11). Whether anyone outside Resolume can write DXV files: not checked (my own finding).
@@END

## ASSUMPTIONS STILL OPEN (new after page 2: ids with a 3; old ones never shown to him keep their ids)
@@ASSUME H3-1
ABOUT: 238, 267, R218, R193
TEXT: I assume the end is cut the first time you switch a clip to BPM mode: up to 8 seconds, so a loop without a seam can show a jump there. In Timeline mode it stays cut until you drag the clip out point out.
WHY: BF258 picks the cut but not its moment; item 267 (accepted) brings every video in in Timeline mode, so no clip is "new in BPM mode" when it arrives. How much the cut takes (less than one group of 4 bars, which is 8 seconds at 120 BPM) follows from his words (binding-decisions.md 778-782, 812-813) but never stood on his page, and neither did what a cut end does to a clip made to repeat without a seam (it jumps where the end was cut): both are said here once, in the line he reads. The part is counted from the clip in point (item 238 point 1).
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
TEXT: I assume the codec test compares your clips as they are with copies in HAP, ProRes and Motion JPEG: playing, jumping, recording, file size. It starts with the files you have: which codecs they are in, and whether DXV files open.
WHY: BF240 says "different codec" and names none. That the test comes after the build is his own word (BF240: "after we build the app") and no part of this assumption; his words put no ban on what a builder times while building.
ALT: b) Other codecs belong in the test: name them.
IF-WRONG: SMALL the test is run again with another codec
ASK: NO the list is Harmony's preparation of a test of which he reads only the result; the order is his own word
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

@@ASSUME H3-6
ABOUT: 238, R218, R219
TEXT: I assume a clip you switch to BPM mode while it plays carries on from where it is and is cut into time on the next 1. If it stood in the part the cut takes off, it carries on from its clip in point.
WHY: Item 267 brings every video in in Timeline mode, so the first switch to BPM mode, and with it the cut of item 238, will often be made on a clip that is playing; no words of his name that moment. It follows his answer "71 b" (binding-decisions.md 902), which chose that a clip out of time plays on and cuts once into time on the next 1, and item R219 (the playhead never leaves the in and out points).
ALT: b) It starts again from its clip in point on the next 1, as if triggered. c) The cut waits until the clip has left the part that is taken off.
IF-WRONG: SMALL one jump at the moment of a switch that he makes while preparing a show
ASK: NO an edge of a switch made in preparation; it follows his rule for a clip that is out of time
@@END

@@ASSUME H3-7
ABOUT: 238, R218
TEXT: I assume a clip of a deck saved before this build that is already in BPM mode is counted and cut the first time that deck is opened, like a clip switched to BPM mode for the first time.
WHY: Item 238 counts a clip at its first switch to BPM mode; saved decks stay (item 235, which he accepted as written), and a clip in one of them can be in BPM mode already, with a number of beats that meant something else. No words of his name it.
ALT: b) Such a clip keeps the number it was saved with and is not cut.
IF-WRONG: SMALL the starting number of clips in earlier decks; he sets Beats himself
ASK: NO how an earlier file is read; topic F owns the show file and the decks
@@END

@@ASSUME H-2
ABOUT: 201, R192
TEXT: I assume Random jumps once every "Interval" beats and picks its landing beat within "Distance" beats of the playhead, never outside the in and out points. Both start at the values of your picture: Interval 1, Distance 2.
WHY: L20 describes one setting (a jump on every beat); what Distance does comes from Resolume's manual ("the range from which a random point will be picked"), not from his look.
ALT: b) Distance is a fixed step, not a range. c) The range is counted from the clip's start, not from the playhead.
IF-WRONG: SMALL one rule of the jump changes
ASK: NO it follows Resolume's manual, which his L102 and his "mimic exactly" make the model; he can see it in his Arena
@@END

@@ASSUME H-4
ABOUT: 202
TEXT: I assume tempo stop, which takes every clip off its layer, forgets where each clip was: the next trigger starts it from the start, whatever its start menu says. After any other leave, a BPM-mode clip that continues starts on its nearest earlier beat marker.
WHY: His words take every clip off its layer at a stop (BF271: "Stop removes all clips from all layers", which repeats binding-decisions.md 952: "stop clears all clips from layer strips"); they do not say whether a clip set to Continue then remembers its place.
ALT: b) Tempo stop leaves each clip's place alone, so a clip set to Continue picks up where the stop took it off. c) In BPM mode the start menu is not used: such a clip always starts from the start.
IF-WRONG: SMALL only clips whose start menu he changed are touched; one reset more or less
ASK: NO an edge of a menu that stands on Restart until he changes it; small to change
@@END

@@ASSUME H-6
ABOUT: R218
TEXT: I assume Beats is always a whole number of at least 1, and "/2" does nothing on an odd number.
WHY: L21 says plus and minus move by one beat and "/2" halves; it does not say what half of an odd number is.
ALT: b) Half beats are allowed, so 17 halves to 8.5.
IF-WRONG: SMALL one button rule
ASK: NO an edge he will rarely meet
@@END

@@ASSUME H-7
ABOUT: R193
TEXT: I assume in Timeline mode the video's pace is its Speed times what its Duration asks for, and pulling the out point in makes the clip shorter at the same pace, with the Duration number following.
WHY: L21 says neither number changes the other; it does not say what a moved out point does to the Duration number.
ALT: b) A moved out point leaves the Duration number alone, so the clip plays slower.
IF-WRONG: SMALL one rule of the Duration row
ASK: NO the out point part stood on his page unchanged; the rest is arithmetic
@@END

@@ASSUME H-9
ABOUT: 201, R192, R217
TEXT: I assume the loop menu has the same five entries in Timeline mode, as in Resolume: there Random's Interval and Distance count seconds and a jump can land anywhere. Beat Repeat is offered only in BPM mode.
WHY: L102 models both menus after Resolume, whose manual counts Random's rows "in seconds or in beats, depending on which mode you're in"; his words name Random only in BPM mode.
ALT: b) Random is offered only on a clip in BPM mode. c) Beat Repeat is offered in Timeline mode too, counted in beats of the tempo.
IF-WRONG: SMALL one entry more or less in one mode
ASK: NO it follows Resolume's manual, which his L102 makes the model
@@END

@@ASSUME H-10
ABOUT: 238, R218
TEXT: I assume a clip short of a whole group of 4 bars by less than a tenth of a beat counts as whole and is not cut. A clip of many pictures is counted at its Timeline-mode rate and gets at least one group.
WHY: BF258's way b gives no tolerance and does not name a clip of many pictures. That the count is at 120 BPM and that a short clip gets 16 beats are his words (binding-decisions.md 778-782, 673, 684, 846, 865): they stand in the RULE of item 238 and are no part of this assumption. That a clip of many pictures loses no picture is asked in topic K (K3-5).
ALT: b) No tolerance: a clip one picture short of 16 seconds is cut to 8. c) A clip of many pictures gets the number of groups nearest to its length, not the number that fits.
IF-WRONG: SMALL the starting number of some clips differs; he sets Beats himself anyway
ASK: NO arithmetic of the count; what his words say of it stands in the RULE
@@END

@@ASSUME H-11
ABOUT: 202
TEXT: I assume the start menu's three entries work as Resolume's manual says: from the start; from where this clip was when it last left its layer; or equally far in as the clip that played before it (halfway stays halfway).
WHY: L102 says "after resolume"; the page described the third entry as carrying on "as if it had never stopped", which is not what Resolume's manual says.
ALT: b) The third entry carries on as if the clip had never stopped, as the page of 2026-10-05 described it. c) "Equally far in" is counted in seconds or beats, not as a share of the clip.
IF-WRONG: SMALL one entry of a menu that stands on "from the start" until he changes it
ASK: NO his L102 makes Resolume the model and its manual is clear; the cheapest check of the third entry is one look in his Arena
@@END

@@ASSUME H-12
ABOUT: R218, R219, R192
TEXT: I assume a BPM-mode clip cut "into time" jumps, on the next 1, to its nearest bar line, counted in fours from its in point. The drift of a clip whose Beats is no multiple of 4 is left alone.
WHY: His "71 b", "121 a" and "124 a" cut a clip into time on the next 1; they were answered when lengths were whole bars; L99 allows any beat count.
ALT: b) It only lines its beat markers up with the beat: a jump of less than a beat. c) It jumps to where it would be had it never been touched.
IF-WRONG: SMALL the size of one corrective jump
ASK: NO technical; it belongs to the re-cut of the transport plan
@@END

@@ASSUME H-13
ABOUT: R193
TEXT: I assume in BPM mode Speed multiplies the pace set by Beats: at Speed 2 a 16-beat clip plays once in 8 beats, at 1/2 in 32; at 0 the picture stands still. "+" from 0 goes to 1/8.
WHY: L21 gives the nine values and says plus and minus double and halve; it does not say what 0 does or how Speed and Beats work together.
ALT: b) Speed 0 is left out of the list in our app.
IF-WRONG: SMALL one step of one row
ASK: NO arithmetic of a row he asked to be as Resolume's
@@END

## CLOSED ASSUMPTIONS (one line each)
- H-1 -> SETTLED
- H-3 -> page 2, item 266
- H-5 -> page 2, item 238
- H-14 -> CLOSED by page 2 (H): It was put to him as item 239; he answered with a question (BF259), and BF240 moves the measurement behind the build. Its text ("First I measure ... Only if the wait shows do I add a Convert for performance command") no longer holds; H3-2 takes its place.
- H-15 -> page 2, item 267

## QUESTIONS BACK
@@ANSWER 239
HIS: BF259 "Why would we make HAP copies at all?"
ANSWER: Only if your clips are seen to wait; if nothing waits, never. An ordinary video file stores a few pictures whole and the others only as changes from the picture before. On a jump, on backwards play or on a random beat the app must first rebuild the picture from the last whole one, which can show as a short wait. A HAP file stores every picture whole, as Resolume's DXV does, so nothing is rebuilt; its price is a much bigger file (my estimate). A clip you already have as HAP or DXV is stored that way and would need no copy; whether DXV files open here is not tried yet, and the test after the build starts with that. That test shows whether your clips wait at all.
@@END

@@ANSWER codec
HIS: BF240 "after we build the app can we test encoding and decoding of different codec to have a baseline of what codecs work good on this mac m1 32gb?"
ANSWER: Yes. Once the app is built I run that test on this Mac with your own clips, and you get one short list of the codecs that work well here. Decoding: each clip as it is and as copies in HAP, ProRes and Motion JPEG: does it play without losing pictures, alone and on several layers, and does a jump, backwards play or a random beat wait. Encoding: every codec the app can record in: does recording disturb the output, how big are the files. No numbers before the test.
@@END

## NAMES
@@NAME Beats
MEANS: The row of a BPM-mode clip that says over how many beats the clip plays once, with "-", "+", "/2" and "x2".
SOURCE: his words L99, L21
@@END

@@NAME Speed
MEANS: The row that makes a clip play faster or slower: free up to 10 in Timeline mode, the nine halves and doubles from 0 to 16 in BPM mode.
SOURCE: his words L21
@@END

@@NAME Duration
MEANS: The row of a Timeline-mode clip that says how long one pass of the clip takes; a BPM-mode clip has none.
SOURCE: his words L21
@@END

@@NAME BPM mode
MEANS: The way of playing in which a clip is locked to the beat and its length is counted in beats.
SOURCE: his words L11, L12, L21; the on-screen name now is "BPM Sync" (as in his Resolume picture): which word goes on screen is for his list of names
@@END

@@NAME Timeline mode
MEANS: The way of playing in which a clip runs at its own Speed and Duration and does not follow the beat.
SOURCE: his words L21; note his L112 uses "Timeline" for a slider that follows the layer's playhead (topic I): the two uses have to be told apart in his list of names
@@END

@@NAME beat marker
MEANS: One of the lines on the timeline of a BPM-mode clip, one per beat; Random lands on them.
SOURCE: his words L20, L21 ("beat markers"), L101 ("markers", "bpm lines")
@@END

@@NAME Random
MEANS: The loop menu entry that makes a clip jump by itself to another beat marker.
SOURCE: his words L20, L101
@@END

@@NAME Interval
MEANS: Random's row that says how often the clip jumps, in beats.
SOURCE: his earlier words and picture (binding-decisions.md 1000-1002)
@@END

@@NAME Distance
MEANS: Random's row that says how far a jump may go, in beats.
SOURCE: his earlier words and picture (binding-decisions.md 1000-1002)
@@END

@@NAME Beat Repeat
MEANS: The row of buttons that repeats a short piece of the playing clip, with "Catch up" to go back into time.
SOURCE: his earlier words (binding-decisions.md 947)
@@END

@@NAME loop menu
MEANS: The first small menu of a clip: Loop, Ping Pong, Random, Play Once and Eject, Play Once and Hold.
SOURCE: Harmony's pick for the menu's name; the entries are Resolume's, "ejects" is his word (L98)
@@END

@@NAME start menu
MEANS: The second small menu of a clip, which decides where the clip starts when it is fired after being away from its layer: Restart (from the start), Continue (from where it was when it last left its layer), Relative (equally far in as the clip that played before it).
SOURCE: Harmony's pick for the menu's name (Resolume's manual calls it "Playmode Away"); the meanings are the manual's (his L102); the words Restart / Continue / Relative are the on-screen names now and the likely words of his Arena's menu (not yet seen open); the manual says from the start / pick-up / relative pick-up
@@END

@@NAME Play Once and Eject
MEANS: The loop menu entry that plays a clip once and then clears its layer, unless a next clip appears.
SOURCE: Resolume's name; his words L98
@@END

@@NAME Play Once and Hold
MEANS: The loop menu entry that plays a clip once and leaves it standing on its last frame.
SOURCE: Resolume's name; replaces the on-screen name now, "One Shot", together with Play Once and Eject
@@END

@@NAME Cuepoints
MEANS: The eight jump points of a clip in the Clip tab; everywhere else "cue" means the cue buttons under the preview monitor.
SOURCE: the on-screen name now, kept (his L36 leaves that part standing)
@@END

@@NAME Convert for performance
MEANS: A command that writes a fast copy of a video (every picture stored whole) beside the original; planned only if the jump measurement shows a wait and he says yes.
SOURCE: Harmony's pick, from the codec answer paper; not a name of his; it enters his list of names only if he takes assumption H-14
@@END

@@NAME codec test
MEANS: The test after the app is built that compares, on his Mac and with his own clips, how video codecs play, jump and record, and gives him one short list of the codecs that work well.
SOURCE: Harmony's pick, from his words (BF240: "test encoding and decoding of different codec to have a baseline")
@@END

## REACHES OTHER TOPICS (from the paper)
- E, item 232 way c (BF254 "c"), must agree with item 238 here: ONE kind of cut. In both, the clip out point is moved in to the end of the last whole group of 4 bars, nothing is taken out of the file ("the rest stays in the file"), and the rest plays again when he drags the clip out point out. One difference E has to rule: a clip made with Record to Clip is counted in the bars the beat clock really ran while it was recorded (5 recorded bars = 4 kept, Beats 16), NOT at 120 BPM as a clip he brings in (238, H-10); counted at 120, a clip recorded at any other tempo would get the wrong Beats. Also E's to rule: a recording shorter than 4 bars, and whether such a clip is born in BPM mode (item 267 speaks only of "a video you bring in"; a clip recorded on the beat that came in in Timeline mode would not loop its 4 bars in time).
- E, BF241 ("we should test which codecs work best"): the encoding half of the codec test is ruled here (item 239 point 2, G1 point 7); E names which codecs and sizes of the low-resolution show recording and of Record to Clip go into it, and refers here for the test.
- A, BF271 ("Stop removes all clips from all layers"): A owns the rule. Applied here only as the ground of H-4: after a tempo stop every clip has been away from its layer, so item 202's start menu acts at the next trigger.
- I, BF260 ("All the blend modes and keying stay ..."): I owns item 240 and the conflict with his L111. Applied here to this topic's own old blocks R194 (f) and P28, which had the keying and its K slider leave the layer strip.
- J, BF245 and BF263 (Studio): applied here to G1 ("scrubbing in Studio"). .harmony/NAMES.md row "Review" is J's to change.
- B, BF247 ("Previewing a clip ... should just be quick"): how fast a previewed clip shows its first picture is one of the waits the codec test times (item 239 point 2); B owns the preview rule.
- F, the list of what a copied clip carries (old G2): a copy carries its Beats and its clip out point as they stand; a copy is not counted and cut a second time (238 point 6).
- A and B, item 266: the restart of an already playing clip is ruled here and in item 202; A's R128 d (a column trigger of a playing column) and D's BF251 ("plays again only when that clip is re-triggered") lean on it: a re-trigger is a restart.

## CONFLICTS (from the paper)
- 238, no conflict to put to him, noted because it looks like one: BF258 "b" (whole groups of 4 bars, the end cut in) against his L99 "R218 do beats here. It was my mistake before" (binding-decisions.md 1177) and his look in his Resolume, L21 (spec-H R218: "By default it gives me a decent amount of beats (sometimes 8, sometimes 16)"). The row still reads and steps in beats (L99 stands); only the FIRST number follows his rule of 2026-10-04 (binding-decisions.md 812-813: "always work with multiples of 4. If it is uneven, then move the outpoint in to keep those multiples."), which BF258 confirms. In one line for him: "Your clips in BPM mode will not start like your Resolume's (8 or 16 beats, nothing cut): they get whole groups of 4 bars and lose the rest of their end, as you chose."
- R194 f and P28 (topic I owns it): BF260 "All the blend modes and keying stay, and they will be built when there is time." against binding-decisions.md 1185: "204 I want to remove the keying and slider. We are only going to use the transparency slider to control that layers blend mode." His newest words win; amended here in this topic's two old blocks.
- 239, not a conflict of his words: the page's "First I measure" was Harmony's text; his BF240 puts the test after the build. His earlier "We should pick the most optimal Kodex to use and not worry about the other ones" (binding-decisions.md 650) is served by the test's result, not contradicted.

## NOT DONE / UNSURE (from the paper)
- 238: the size of the cut was not on the page. A clip of 15 seconds keeps 8; a clip of 23.9 seconds keeps 16: up to just under 8 seconds of every clip longer than 8 seconds goes from its end in BPM mode (arithmetic at 120 BPM, not measured). He chose the cut and it is his own rule of 2026-10-04; it is not asked again. If the page ruling wants it said once, the line is in CONFLICTS.
- 238: "exactly its normal speed" is true at a tempo of 120 only; at 128 the same clip plays faster. That is BPM mode itself and his "not current bpm"; carried in H-10 (not asked).
- 239: nothing is measured. Whether a jump on his ordinary clips waits long enough to be seen is UNKNOWN until the codec test; Random, Beat Repeat and backwards play are therefore built without that knowledge unless he takes H3-3 way b. Cheapest way to know earlier: way b (two clips, one session, while those functions are built).
- 239: whether the FFmpeg on this Mac can write HAP (needed only for the test copies) and whether a DXV file of his opens: not checked. Cheapest: one look at the encoder list and one file tried, in the session of the test.
- The name "Convert for performance" (old spec-H NAMES) is not planned any more; it is not in .harmony/NAMES.md (grep: no hit), so nothing is to be taken out there. The lint's DROP takes a one-word id, so no @@DROP NAME is written for it.
- Not amended, wording only: old RULE lines of this topic still say "fire / fired" where his word is "trigger" (202, R192, R193, R194, G1).
- Old assumptions of this topic tested against his 33 boxes and left untouched: H-2, H-6, H-7, H-9, H-11, H-12, H-13. Changed and kept under their ids: H-4 (BF271), H-10 (BF258).

Written 2026-10-09 18:48:40 EDT by the architect seat of topic H. Read-only; nothing built, run or committed.

## FOR THE PAGE RULING (from the ruling)
- MUST be put to him, in this order of weight: (1) H3-2, the only YES of this topic (item 239 is OPEN because he asked back), with ANSWER 239 straight above it; (2) ANSWER codec ("Yes", owed to BF240); (3) H3-1 as one line he can strike. Nothing else of this topic is for him.
- H3-1 carries on purpose what his page never showed: way b of item 238 takes up to 8 seconds off the end of every clip longer than 8 seconds (arithmetic at 120 BPM, not measured), and a clip made to loop without a seam then jumps where its end was cut. Keep both clauses when the line is set on his page. If there is room for one more: "exactly its normal speed" holds at a tempo of 120 (his "for 120 bpm") and the clip runs faster or slower with the tempo, as in his Resolume; a clip under 8 seconds starts slow (his "47 default"). His choice stands (BF258 "b"; binding-decisions.md 812-813) and is NOT asked again.
- Newest words against earlier ones, to be said once: none of his own in this topic. BF258 "b" confirms his rule of 2026-10-04 against his look in his Resolume (L21: 8 or 16 beats, nothing cut); the paper's first CONFLICTS bullet holds the one line. BF260 against L111 (the keying) is topic I's to say; this topic only follows it in R194 (f) and P28.
- Depends on topic K: K3-5 (a clip of many pictures loses no picture in BPM mode). This topic now says the same in item 238 (10) and R218 (b) and names K3-5 as the place where he is asked; if K's ruling or his answer turns it, those two sentences follow K. Ask it once, in K.
- Depends on topic E: item 232 as ruled there (a clip made with record to clip is a clip in BPM mode from the start, its Beats counted by the beat clock and never at 120 BPM) is set apart in 267, 238 (8), R193 (i) and R218 (b). E's items 256 and 262 and its D22 amendment mean this topic's "codec test" (NAME block) when they say "the test of codecs"; its encoding half now names record to clip, the low-resolution show recording and the film made with Render in Studio.
- Depends on topic A: when a re-triggered clip in BPM mode restarts (R128 d as A amends it; item 249), and what tempo stop does to clips (BF271, the ground of the internal H-4). If A rules that the app placing the 1 by itself (BF246) puts playing BPM-mode clips back into time, the internal H-12 says what "into time" means.
- Depends on topic I: item 240 (R194 f and P28 follow it, the keying slider as I rules it). Topic F: a copy of a clip carries its mode, its Beats and its clip out point; how a deck saved before this build is read (internal H3-7).
- MERGE: old item C13 is amended by this topic (C13 1) and by topic X (C13 1, C13 2). X's C13 2 has the same OLD sentence as this topic's C13 1, and a dry run of wf/merge3.py reports it "NOT applied"; this topic's block is the one to keep (C13 stands under MADE FROM of item 238), so X's C13 2 should be dropped in X's ruling. All 11 amendments of this topic applied in that dry run.
- UNKNOWN, and cheap to settle before the page is made: which codecs his own clips are in (nobody has looked; this ruling only listed ~/Library/Audio-DNA, which holds settings.json and names no media, and opened nothing else of his). If Harmony reads the codec of the media files his decks name (read-only), ANSWER 239 can say outright whether any copy could ever be needed. Whether DXV files open in the app is INFERRED, not tried: one file in the first build session.
- Internal, never for him: H3-3, H3-4, H3-5, H3-6, H3-7, H-4, H-10, and the untouched H-2, H-6, H-7, H-9, H-11, H-12, H-13. The old name "Convert for performance" is not planned any more; it is not in .harmony/NAMES.md and must not enter it.

Ruled 2026-10-09 19:32:53 EDT by the ruling seat of topic H. Read-only; nothing built, run or committed.

