# SPEC H -- How a clip plays (s-rta-1007): the paper apply-H.md with the ruling rule-H.md laid over it by merge.py. This file wins over both.

## ITEMS
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
STATUS: STANDS part a confirmed by his words; part b open, asked (H-5)
HIS: L99, L21
RULE: His words (L99): "do beats here. It was my mistake before"; his look in his Resolume (L21): "There is no duration on a bpm clip, but it has beats. The beats regulate how many beats are on the clip." and "Pressing plus and minus increase or decrease by one beat and there is /2 and x2, which double and halve." Built so: (a) a clip in BPM mode has no Duration row; it has a row "Beats" with "-", "+", "/2" and "x2". "-" and "+" change the length by 1 beat; "/2" and "x2" halve and double it. The number says over how many beats of the tempo the clip plays once from its in point to its out point at Speed 1; more beats = slower. Any whole number from 1 up is allowed (H-6): 16 becomes 17 with one press, and a clip of 17 beats drifts against the bars each time it loops (he was shown this sentence and kept beats); the app leaves that drift alone (H-12). (b) The number a new clip starts with is OPEN and is not built until he answers H-5. Three ways are on the table. The best reading, and the text of H-5, is the way of his Resolume, which he reports after calling his bars a mistake (L21: "By default it gives me a decent amount of beats (sometimes 8, sometimes 16) so that it plays at about the same speed as it would play at speed number one in timeline mode"): the power of two from 4 up (4, 8, 16, 32 ...) that plays the whole clip nearest to its normal speed, with no out point moved. Against it stands his rule of 2026-10-04, which the page repeated and he left as it was: whole groups of 4 bars, the out point moved in to match (binding-decisions.md 812-813: "always work with multiples of 4. If it is uneven, then move the outpoint in to keep those multiples."). The third way is that rule in whole bars, that is multiples of 4 beats. His L99 calls the bars of 2026-10-04 his mistake, and that rule was made in bars: whether the mistake reaches the first number is not said. Whichever he picks, the number is worked out once, when the clip comes into the show, counting the clip at 120 BPM and never under 4 beats (H-10; a picture sequence, which has no seconds of its own, is counted at its Timeline-mode rate). The Speed row in BPM mode: item R193 e.
CHANGED: (a) stands; the invitation "Say R218 bars" is declined by him: his earlier "lets do bars here not beats" (binding-decisions.md 761) is withdrawn for this row. (b) not named by him; open because of L21 and because L99 calls the bars regime his mistake.
TODAY: Every new clip starts at 4 beats / 4 beats; nothing sets a length from the file or the tempo (Clip.h:36-41; area-clip-transport.md part 1 item 10). BPM Sync shows a combo "Beats/ Cycle" and a slider "Content Beats" (item 7); a "Duration" row labelled "Beats" in BPM Sync does nothing (item 6). The adopted rulings RD2 / RA still say Bars in 4, 8, 12, 16 with "/2" greyed: to be re-cut (U-H10). Resolume's own rule for the first number is disputed between its manual (nearest power of 2) and its staff (multiples of 4, or of 8 or 16); all agree it counts at 120 BPM and changes the speed, not the in and out points (facts-resolume-transport.md:24-30).
@@END

@@ITEM R193
TITLE: How a clip plays: the rest
STATUS: REPLACED in parts c and g by his words elsewhere; part h asked (H-1); parts a, b, d, e, f stand
HIS: L102, L21, L101, L13, L11, L12, L17, L31
RULE: (a) Only a video and a picture sequence have playback (BPM mode or Timeline mode, speed, pause, backwards, the loop menu, the start menu). (b) A still picture, a generated source, MilkDrop, the camera and an effects-only clip have none: they show on his press and run on their own clock; the tempo bar's pause does not freeze them, and fired during a pause they show and move. This follows the rule that pause holds what follows the beat and a clip that is not in BPM mode plays on (his "136 b", binding-decisions.md 1009; L13: "Pause pauses, the beat clock, the clips and everything that it controls with BPM."; L11: "If a layer that is not in BPM mode is triggered, then that plays instantly"). Topic A owns the pause and what a fire during a pause does to the beat (L12, L17, L31; its assumptions A-1 and A-3), and (b) follows what is ruled there. (c) A fired video starts from its beginning unless its start menu says otherwise (item 202, L102); a clip set to play backwards keeps its direction and starts from its end. (d) A clip's pause is one thing shown in two places: the Clip tab's buttons and the layer strip's "<", "||", ">" are the same buttons of the clip that plays. (e) Speed is one control shown in two places (the strip's S slider and the Clip tab's Speed). In Timeline mode it runs to 10 (the real top is measured before it is built; if it is lower he is told first). In BPM mode it has "-" and "+" and steps through nine values, as his Resolume (L21, his answer to the look he was asked for, a click on the plus of Speed): "Plus and minus doubles and halves the current. It goes: 0, 1/8, 1/4, 1/2, 1, 2, 4, 8, 16." At Speed 1 the clip plays once over its Beats; the edges (0, and how Speed and Beats work together) are in H-13. (f) The master Speed changes only clips that are not in BPM mode. (g) Timeline mode, as his Resolume (L21): "In timeline mode duration changes don’t affect the speed setting but the video plays faster or slower based on the direction moved. Same with speed. It does not affect the duration setting but make the video move slower and faster." So Speed and Duration are two separate settings: a shorter Duration plays the video faster, a longer one slower, and the Speed number stays; a Speed change plays it faster or slower and the Duration number stays. Duration's "-" and "+" step 0.1 (his earlier words, binding-decisions.md 1002). How the two combine and what pulling the out point in does to the Duration number: H-7. In BPM mode pulling the out point in keeps the same number of beats, so the clip goes through less video and looks slower (his rule, binding-decisions.md 778-782, in beats by L99). (h) What the clip's timeline shows in BPM mode: one line per beat, the reading of L101 and L21 against his earlier "Timeline only shows bars": asked in H-1. In Timeline mode it shows the plain playhead without beat lines (his earlier words, binding-decisions.md 741-742).
CHANGED: (c) gains "unless its start menu says otherwise" (L102; the page said every fire starts from the beginning). (g) the page's "typing a Duration changes the Speed" is replaced by his look L21 (neither number changes the other); "bars" in its BPM half becomes "beats" (L99: INFERRED). (e) the BPM half is now exact from L21 (nine steps); that the list is Speed's is INFERRED from the look he was asked for (R206 f) and from Resolume's manual, which gives the same nine values for its BPM Sync Speed (facts-resolume-transport.md:31). (h) "bars only" is under question (H-1). (a), (b), (d), (f) unchanged.
TODAY: Playback only for video and sequence (Clip.h:248; U-H24). Speed slider 0 to 4 with two halve / double buttons and a Reverse text button (area-clip-transport.md part 1 item 5); in BPM Sync a video ignores the Speed slider and the tempo never enters its rate (item 8), a sequence follows the tempo and its Speed slider (item 9, correction 8). The Duration row does nothing (item 6). Master Speed scales Timeline clips only (item 26). The strip's buttons write the clip (item 17). No beat pause exists (item 23). The timeline draws beat-division lines (item 7). A new clip is in Timeline mode (item 1, Clip.h:118-119).
@@END

@@ITEM R194
TITLE: The layer strip: fade, solo, bypass, blend
STATUS: STANDS part d confirmed by his look in Resolume
HIS: L29, L111, L19
RULE: (a) A new layer's fade (F) is 0, a cut; F counts seconds. (b) A third clip fired while a fade is running: the new fade starts from the picture as it is at that moment. (c) Solo on a layer that holds no clip does not darken the show. (d) B (bypass) and S (solo) are controls of the output. A clip on a layer that is bypassed, or hidden by another layer's solo, plays on out of sight, so when the layer comes back it is further on; his Resolume does the same with a bypassed layer (L29: "H it plays out of sight"). (e) The X's tooltip says what the X does (it takes the clip off the layer) and makes no promise about Undo, because Undo never changes the layer strip (his earlier words, binding-decisions.md 736). (f) A new layer's Blend starts on "Add". His L111 ("I want to remove the keying and slider. We are only going to use the transparency slider to control that layers blend mode.") takes the keying and its K slider off the layer; what it means for the Blend list is ruled in topic I (question 204), and (f) follows that ruling. What a column click does to a layer whose cell is empty: his look L19 ("D layer goes empty when a column is triggered with an empty") agrees with the page; ruled in topic A.
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
STATUS: DROPPED
HIS: L8, L7, L73, L111, L56
RULE: laid out by Harmony where it fits in the correct area; its look is settled in the UI redesign (L8). What his words fix about its contents: a layer has an "ignore actions" toggle as well as "ignore column" (L7: "We should have ignore actions toggle on the layer as well as ignore column."; L73; ruled in topic D); the strip carries no keying control and no K slider (L111; ruled in topic I); the action buttons sit in "a small row below the clip, still within the layer" (L56, topic D). The three play buttons are on the strip as the clip's own buttons (R193 d). A clip he paused himself is told apart from one the tempo bar holds by a mark; how the mark looks is for the redesign.
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
STATUS: ANSWERED no codec of our own; one choice of his asked (H-14)
HIS: L3
RULE: His question (L3): "Is there any reason for us to build our own codec that is optimized for our system like resolume’s DVX 3.0? Is that something that you can do reliably?" The answer for him is the ANSWER block with the key codec: no codec of our own is built. What stands for the build: (1) Ordinary video files play without any conversion, and no file of his is converted without his say. (2) The fast path for jumps (Random, Beat Repeat, backwards play, a fire into a long video, scrubbing in Review) is a format that stores every picture whole and already exists: HAP first, with an all-pictures-whole file of a codec the app plays (ProRes or Motion JPEG) as the comparison. No new format is invented. This is the one optimal codec his earlier words ask for (binding-decisions.md 650-651: "I want codecs that decode easily and play well"). (3) Nothing is planned for it before a measurement: in the first build session, on two of his own clips (one 1080p, one 4K), the wait from a jump to the first right picture is measured for the ordinary file, for a HAP copy and for the comparison copy, together with the size of each copy and the time the conversion takes (the whole list: answer-codec.md, "WHAT HAS TO BE MEASURED WHEN IT IS BUILT"). (4) Only if the wait shows is a command "Convert for performance" planned, which writes the fast copy beside the original; whether it is planned at all is his choice (H-14). (5) Before that command is planned, the FFmpeg the app is built with is pinned and checked for the encoder it needs. (6) Unpacking on the graphics card is no part of this: it is not shown possible here and nothing is promised on it.
CHANGED: nothing of the page: a new point of his message, it answers no numbered item. "DVX 3.0" read as Resolume's DXV 3: INFERRED. The answer paper's own question for him (its "DEFAULT: yes, plan it") is replaced by H-14 (measure first), after its re-check.
TODAY: Video is decoded by FFmpeg on the CPU, one decode thread per clip (VideoPlayer.cpp:117, read by me; no hardware decoder is asked for: the grep of answer-codec.md fact 19 and check-codec.md fact 15). HAP, ProRes, Motion JPEG and DNxHD are treated as every-picture-whole (VideoPlayer.cpp:290, read by me); HAP kinds are named in VideoInfo.h:59-66 and a HAP file showed 270 of 270 pictures in the reverse test (docs/claude/rendering.md:94). A jump on a long-GOP file re-seeks to a keyframe and catches up (pitfalls 56, 62); how long that takes on his clips is UNKNOWN. INFERRED, not tried: a DXV file he owns opens through the same decoder lookup (VideoInfo.h:77 names "dxv"; check-codec.md fact 11 finds the name "Resolume DXV" in the local libavcodec). The Homebrew FFmpeg formula of the day lists no snappy, so the HAP encoder can vanish on an upgrade (check-codec.md fact 11). Whether anyone outside Resolume can write DXV files: not checked (my own finding).
@@END

## ASSUMPTIONS
@@ASSUME H-1
ABOUT: 201, R193
TEXT: I assume a BPM-mode clip's timeline shows one line per beat, as in your Resolume, and Random can land on any of them. Your earlier "Timeline only shows bars" no longer holds for a clip.
WHY: His newer words speak of beat markers (L101, L20, L21); his earlier words say the timeline shows bars only and Random jumps to "random 1's" (binding-decisions.md 825, 1001).
ALT: b) The timeline shows bar lines only, and Random lands only on the first beat of a bar.
IF-WRONG: STAGE a Random clip would come in on beats 2, 3 or 4 when he wants only the 1, or the other way round
ASK: YES his new words and his earlier words pull apart, and the audience sees where a jump lands
@@END

@@ASSUME H-2
ABOUT: 201, R192
TEXT: I assume Random jumps once every "Interval" beats and picks its landing beat within "Distance" beats of the playhead, never outside the in and out points. Both start at the values of your picture: Interval 1, Distance 2.
WHY: L20 describes one setting (a jump on every beat); what Distance does comes from Resolume's manual ("the range from which a random point will be picked"), not from his look.
ALT: b) Distance is a fixed step, not a range. c) The range is counted from the clip's start, not from the playhead.
IF-WRONG: SMALL one rule of the jump changes
ASK: NO it follows Resolume's manual, which his L102 and his "mimic exactly" make the model; he can see it in his Arena
@@END

@@ASSUME H-3
ABOUT: 202, R193
TEXT: I assume the start menu (Restart / Continue / Relative) acts only when a clip comes back to its layer: firing the clip that is already playing restarts it, whatever that menu says, as you ruled for a playing column.
WHY: L102 says only to model the menus "after resolume"; his "Firing a column that is already playing should restart its videos" (binding-decisions.md 616) was said before this menu was planned.
ALT: b) A clip set to Continue plays on undisturbed when you fire it again, alone or with its column.
IF-WRONG: STAGE a clip he set to Continue would restart on a column press, or fail to; only clips whose menu he changed
ASK: LINE his earlier words point this way; what his Resolume does there is not known
@@END

@@ASSUME H-4
ABOUT: 202
TEXT: I assume the tempo bar's Stop also forgets where every clip was: the first fire after Stop starts each clip from the start, whatever its start menu says. A BPM-mode clip that continues starts on its nearest earlier beat marker.
WHY: No word of his names the menu's edges; Resolume's Stop "stops and rewinds all timeline and BPM animations and ejects all playing clips" (facts-resolume-emulate.md C3.4).
ALT: b) Stop leaves each clip's place alone, so a clip set to Continue picks up where Stop took it off. c) In BPM mode the start menu is not used: such a clip always starts from the start, as older Resolume versions did.
IF-WRONG: SMALL only clips whose start menu he changed are touched; one reset more or less
ASK: NO an edge of a menu that stands on "from the start" until he changes it; small to change
@@END

@@ASSUME H-5
ABOUT: R218
TEXT: I assume a new clip in BPM mode gets its first number of beats as in your Resolume: 4, 8, 16, 32 ..., whichever plays the whole clip nearest to its normal speed. No out point is moved.
WHY: L99 calls his bars a mistake; his Resolume look (L21) gives 8 or 16 beats at about normal speed; his rule of 4 October moved the out point in instead.
ALT: b) Your rule of 4 October as you gave it: the largest whole group of 4 bars that fits (16, 32, 48 beats), the out point moved in to match, exact normal speed; a shorter clip gets 16 beats. c) The same in whole bars (4, 8, 12, 16 ... beats), the out point moved in by less than a bar.
IF-WRONG: STAGE every new clip in BPM mode would start at another length or speed than he expects, or lose its end
ASK: YES his rule of 4 October and what he reports from his Resolume differ, and it touches every clip he brings in
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
ABOUT: R218
TEXT: I assume the first number of beats is worked out once, counting the clip at 120 BPM, and is never under 4; "nearest to normal speed" is judged as a ratio, a tie going to the larger number.
WHY: L21 describes the first number he sees, not how it is worked out; his earlier words: "not current bpm" and "default all bpms to 120" (binding-decisions.md 673, 684).
ALT: b) "Nearest" is judged by the difference in beats, not as a ratio. c) A very short clip may start with 1 or 2 beats, as in Resolume.
IF-WRONG: SMALL the starting number differs by one step; he sets it himself anyway
ASK: NO internal; his earlier words name 120 and rule out the tempo of the moment
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

@@ASSUME H-14
ABOUT: G1
TEXT: I assume we build no codec. First I measure, on two of your clips, how long a jump waits. Only if the wait shows do I add a "Convert for performance" command that makes fast copies; ordinary files keep playing.
WHY: L3 asks only whether to build a codec; whether his clips get fast copies, and by which tool, is a new function no words of his rule on.
ALT: b) Plan the "Convert for performance" command now, without measuring first. c) No such command: you make fast copies yourself with tools you own (HAP files open here; DXV files should too: not yet tried). d) Build our own codec after all.
IF-WRONG: STAGE on ordinary video files a Random jump or a scrub may stutter in front of an audience; or a converter is built that he never needed
ASK: YES a new command, a conversion step in his work and much bigger files on his disk are his to choose
@@END

@@ASSUME H-15
ABOUT: R218, R193
TEXT: I assume a video or picture sequence you bring into the show starts in Timeline mode, and you switch the clips you want on the beat to BPM mode yourself.
WHY: No word of his says which mode a new clip has; his "a clip that has BPM mode enabled" (binding-decisions.md 1081) reads as something he switches on.
ALT: b) Every new video starts in BPM mode. c) One setting says which mode new clips start in.
IF-WRONG: SMALL a starting value, but it touches every clip he brings in
ASK: LINE a real choice of mine that decides how much hand work a show on the beat takes
@@END

## QUESTIONS BACK
@@ANSWER codec
HIS: L3
ANSWER: No: do not build a codec of our own, and I could not promise to make one reliable. It would play only here and need its own converter and long testing, with no outside player to check its files. DXV stores every picture whole, so a jump, backwards play or a random beat needs no catching up. HAP, a free format with the same purpose, has existed since 2012, and HAP files already open here. Both are also made for the graphics card to do the unpacking; I have not shown that we can use that here. Not measured yet: how long a jump waits on your ordinary clips, how much a fast copy gains, and how much bigger it is (my estimate: much bigger). So: measure first, then decide.
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

## CONFLICTS (from the paper, unruled)
- Beat markers against bars only. L101: "each of the markers on a clip are bpm lines and the random lands on one of them" and L21: "Bpm mode the clip’s timeline is divided into beat markers." against binding-decisions.md 825-826: "Timeline only shows bars. The only place we see beats is in the circle with 4 positions in top bar that shows the 4 beats repeating." and 1001: "this is essentially jumping to random 1's on the beat". Asked: H-1.
- The first guess of a clip's beats. L21: "By default it gives me a decent amount of beats (sometimes 8, sometimes 16) so that it plays at about the same speed as it would play at speed number one in timeline mode" and L99: "do beats here. It was my mistake before" against binding-decisions.md 812-813: "always work with multiples of 4. If it is uneven, then move the outpoint in to keep those multiples." and 846: "a short clip can have 4 bars but it will move super fast". Asked: H-5.
- What the tempo row's pause holds (owned by topic A; it decides R193 b). L13: "Pause pauses, the beat clock, the clips and everything that it controls with BPM." against binding-decisions.md 1009: "136 b" (the option he chose: only BPM-synced clips hold). Asked: H-8.
- For the record, not asked (he was asked exactly this and answered): L102: "model these 2 little menu’s after resolume" replaces binding-decisions.md 844: "42 default".

## NOT DONE / UNSURE (from the paper, unruled)
- What the three entries of the start menu read in his Arena, and that there are exactly three: taken from the earlier fact sheet (the manual's "from the start / pick-up / relative pick-up"), not seen; his picture shows the menu closed, as an icon. Cheapest: one picture of that menu open.
- What Distance does in his Resolume's Random (H-2): no source. Cheapest: he sets Distance to 1 and to 8 and says what changes.
- Whether his Resolume offers Random and Beat Repeat in Timeline mode (H-9): not established. Cheapest: one look at the loop menu of a Timeline-mode clip.
- "In time" for a BPM-mode clip whose Beats is not a multiple of 4 (after a playhead drag, Beat Repeat without Catch up, a Resync): his "71 b" and "124 a" say it is cut into time on the next "1", written when lengths were whole bars. With free beat counts the cut can only line the clip's beat markers up with the beat. Technical; for the transport delta, not for him.
- The adopted transport rulings (RD2, RA: Bars, the bar lists of Random and Beat Repeat, no start menu, the fit rule) were written before L99, L101 and L102 and have to be re-cut; I did not re-read them, only the area sheet's account of them.
- G1 is not answered here by instruction.

Written 2026-10-07 22:41:44 EDT by the architect seat of topic H. Read-only; nothing built, run or committed.

## FOR THE PAGE RULING (from the ruling)
- H-5 (ask) is the weightiest here: the first number of beats of every new BPM-mode clip. Its text is the way of his Resolume (4, 8, 16, 32, the whole clip; L21 after L99). His rule of 2026-10-04 (groups of 4 bars, out point moved in) is ALT b and has to be shown beside it: that rule moves the out point in on every clip longer than 8 seconds whose length is not a multiple of 8 seconds (16 beats at 120 BPM).
- H-1 (ask): beat markers against his earlier "Timeline only shows bars". Topic X files the same point as SETTLED (C23): one question, this one. If the page ruling takes it as settled by L101, items 201 and R193 h need no change.
- The codec: the ANSWER block codec replaces the answer paper's own two sections (its "DEFAULT: yes, plan it" is not taken). H-14 (ask) carries the one choice: measure first; a "Convert for performance" command only if the wait shows.
- The start menu (202, L102) reaches other topics: every rule that says "starts from its beginning" (A: 172, 173, 174, R128 d, R139; B: R151; K: R213 a) now reads "unless the clip's start menu says otherwise". X-9 (its first setting) is settled by option B's own text and Resolume's default: no line for him. F's list of what a copied clip carries (G2) should also name Beats and the start menu, and drop "beat snap" (his 154).
- The pause: H-8 is dropped (the paper's third CONFLICTS bullet still says "Asked: H-8": read A-3). R193 b follows topic A (A-1, A-3) and changes with it if A's ruling changes what pause holds.
- Names: "Timeline mode" (his own word, L21) against L112, where topic I rules that ONE thing is called Timeline (the link to the layer's playhead); J-7 picks the two modes' names. One decision for the list of names. Restart / Continue / Relative are unseen: one look at his Arena's open menu settles them.
- Lines for him from this topic: H-3 (a fire of the clip that is already playing restarts it whatever its start menu says; it meets topic A's R128 d) and H-15 (a clip comes into the show in Timeline mode: named nowhere else, and it decides how much hand work a show on the beat takes).
- Followers: R194 f and the K slider follow topic I (204); P28's ignore-actions toggle follows topic D (G5, L7, L73); an Eject during a tempo pause is K-16; an action may hold menu and direction changes (L76), which covers the loop menu, the start menu and back / play.
- Internal, not for him: H-2, H-4, H-6, H-7, H-9, H-10, H-11, H-12, H-13. The paper's NOT DONE lines on H-2, H-9 and G1 are overtaken by this ruling.
- Verified by nobody: whether his Arena honours "Continue" on a BPM Sync clip (version 4 did not; H-4 ALT c), and whether the DXV clips he owns open in the app (INFERRED yes). Cheapest: one look in his Arena; one file tried in the first build session.

