# APPLY H -- How a clip plays (s-rta-1007)
## SUMMARY
- 201: Random lands on any beat marker of the clip (L101, his look L20), not only on a "1". This needs beat markers on a BPM-mode clip's timeline, which sits against his earlier "Timeline only shows bars": asked (H-1).
- 202: BOTH small menus are built after Resolume (L102). The start menu (from the start / carry on / carry on as if never stopped) comes back; "42 default" (no such menu) is replaced. Every rule "a fire starts the clip from its beginning" now reads "unless the clip's start menu says otherwise".
- R218: the length row reads Beats and steps 1 beat (L99: "It was my mistake before"). The app's FIRST GUESS of the beats is in conflict: page = whole groups of 4 bars with the out point moved in; his Resolume = 8 or 16 beats at about normal speed (L21): asked (H-5).
- R217: Eject clears the layer; a next clip brought by the autopilot cancels the Eject (L98): the page's reading confirmed.
- R193: part c gets the start-menu exception (L102); part g (Timeline mode) is replaced by his look L21: Speed and Duration are two separate numbers, each makes the video faster or slower, neither changes the other; part h (bars only) is under question H-1.
- R194 d: a bypassed layer's clip plays on out of sight, as his Resolume does (L29): confirmed.
- R192, R219: stand. P28, P29: layout, dropped to the UI redesign (L8); the K slider goes (L111), a layer gets "ignore actions" (L7, L73), Cuepoints keep their name (L36).
- G1 (own codec): OPEN, waits on the researcher.
- Three questions for him (H-1, H-5, H-8), three lines to strike (H-2, H-3, H-9), four internal.
## ITEMS
@@ITEM 201
TITLE: Where a Random jump lands
STATUS: ANSWERED in his words; they fit option B, with one conflict asked (H-1)
HIS: L101, L20, L21
RULE: His words (L101): "each of the markers on a clip are bpm lines and the random lands on one of them"; his look in his own Resolume (L20): "it randomly moves playhead every single beat. When it changes, it jumps to random beat markers on the clip." Built so: a clip in BPM mode whose loop menu is on Random jumps by itself, and every jump lands exactly on one of the clip's beat markers: any whole beat of the clip between its in and out points, not only the first beat of a bar (the page's option B). Between jumps the clip plays on in time from where it landed. The rows Interval and Distance stay as in his Resolume picture, both counted in beats, each with "-" and "+": Interval is how often it jumps (Interval 1 = on every beat, which is what he saw), Distance is how far a jump may go (ASSUMED, H-2). Random follows the beat: while the tempo row is paused or stopped it does not jump. That the markers are BEAT lines (and so that the clip's timeline shows one line per beat) is my reading of L101 with L20 and L21; it sits against his earlier "Timeline only shows bars": CONFLICT, asked in H-1. Whether Random is offered outside BPM mode: H-9.
CHANGED: Option A (the default: every jump lands on a "1", the first beat of a bar of the clip) is replaced by his words, which fit option B (any whole beat). "bpm lines" read as "beat lines / beat markers": INFERRED from L20 and L21, where he says "beat markers".
TODAY: No Random loop style exists (area-clip-transport.md part 1 item 3: the list reads Loop / Ping Pong / One Shot; grep finds no random loop mode). The adopted transport ruling (RA-8) jumps whole beats within a Distance, with lists counted in bars: to be re-cut to beats (U-H11). The clip timeline draws one line per beat division now (part 1 item 7, ClipInspector.cpp:1265-1276).
@@END
@@ITEM 202
TITLE: The two small menus are modelled after Resolume
STATUS: ANSWERED in his words; they fit option B
HIS: L102
RULE: His words (L102): "model these 2 little menu’s after resolume". Both small menus at the right of the clip's transport panel are built as in his Resolume. (1) The loop menu: the five entries of R217. (2) The start menu, which decides where a clip starts when it is fired: three entries, as in Resolume: from the start; carry on from where the clip was when it left its layer; carry on as if it had never stopped. Every clip is set to "from the start" until he changes it (the page's option B; Resolume's own default). The setting belongs to the clip and is saved with the show. Wherever another rule says "a fired clip starts from its beginning", it now reads "unless the clip's start menu says otherwise". WHEN a clip starts is not touched by this menu: a clip in BPM mode still waits for the "1" (ASSUMED, H-3). Firing the clip that is already playing starts it from the start whatever the menu says (ASSUMED, H-3). A clip he paused himself still shows on its paused frame when fired (his earlier words, binding-decisions.md 806 and 904). After the tempo row's Stop every clip starts from the start once (ASSUMED, H-4). The names of the three entries are Harmony's pick until his list of names is made (NAMES).
CHANGED: Option A (the default: no such menu, every fire starts the clip from its beginning; from his "42 default") is replaced by his words of L102, which name both menus. His earlier "42 default" (binding-decisions.md 844) and "restart" (668) now hold only as the menu's starting setting.
TODAY: A second dropdown "Restart / Continue / Relative" is on screen and does nothing: no handler, no model field (area-clip-transport.md part 1 item 4; ClipInspector.cpp:69-73). A clip that leaves its layer freezes and RESUMES when fired again through a column or after leaving (part 1 item 14); a re-fire through its cell seeks to the in point (item 12); a column re-fire does not restart (item 13). So: a model field per clip, saved; the fire path reads it; the adopted plan left the menu out (RD2 D2-1) and has to take it back in.
@@END
@@ITEM R192
TITLE: Random and Beat Repeat: lists and units
STATUS: STANDS
HIS: none
RULE: (a) Beat Repeat's buttons read Off, 4, 2, 1, 1/2, 1/4, 1/8, 1/16, 1/3, 1/6 and "Catch up", and the numbers count beats, as in his Resolume picture. (b) Random shows Interval and Distance, counted in beats; where a jump lands is item 201 (any beat marker). (c) Beat Repeat is a thing of the moment: it repeats from where the clip is, it goes off when the clip is fired again or leaves the layer, and it is not saved with the show. Switching it off without Catch up: the clip carries on and is cut into time on the next "1" (his "124 a", binding-decisions.md 946).
CHANGED: nothing; the open point in (b) is closed by item 201. His L99 ("do beats here") agrees with counting in beats.
TODAY: No Beat Repeat and no Random exist (area-clip-transport.md part 1 header: grep finds neither). The adopted lists of RA-8 are in bars and lack 1/3 and 1/6 (O13, U-H12): re-cut to the picture.
@@END
@@ITEM R217
TITLE: The loop menu; Eject against a next clip
STATUS: STANDS confirmed by his words
HIS: L98, L102
RULE: The loop menu of a clip reads: Loop; Ping Pong (forwards, then backwards, again and again); Random (item 201); Play Once and Eject (it plays once, then the clip leaves its layer and the layer is empty); Play Once and Hold (it plays once and stays on its last frame). His words (L98): "if a clip plays once and ejects that is clear the layer. If the clip plays on autopilot and the next clip plays then the eject would be cancelled by the next clip appearing." So: at the end of a Play Once and Eject clip the layer is cleared, exactly as if its clip had been taken off; when the autopilot (the layer's, or the clip's own choice of what comes next) brings a next clip at that moment, the next clip appears and no empty layer is seen. Eject empties the layer only when nothing follows. The ejected clip stays in its cell. How the autopilot's next clip starts (a BPM-mode clip on the "1") belongs to topic K.
CHANGED: nothing replaced: his words say the same as the page's "the next clip wins, and Eject empties the layer only when nothing follows". "that is clear the layer" read as "that clears the layer": INFERRED (typing).
TODAY: The loop list reads "Loop", "Ping Pong", "One Shot" (ClipInspector.cpp:56-58; area-clip-transport.md part 1 item 3). "One Shot" becomes two entries; Eject has to clear the layer's clip; the autopilot calls Layer::triggerClip from the render thread (Autopilot.cpp:326, 355, 417; U-H22), so the order "next clip before the clear" has to be fixed there.
@@END
@@ITEM R218
TITLE: The length row of a BPM-mode clip reads Beats
STATUS: STANDS part a confirmed by his words; part b in conflict with his look, asked (H-5)
HIS: L99, L21
RULE: His words (L99): "do beats here. It was my mistake before"; his look (L21): "There is no duration on a bpm clip, but it has beats. The beats regulate how many beats are on the clip." and "Pressing plus and minus increase or decrease by one beat and there is /2 and x2, which double and halve." Built so: (a) a clip in BPM mode has no Duration row; it has a row "Beats" with "-", "+", "/2", "x2". "-" and "+" change the length by 1 beat; "/2" and "x2" halve and double it. The number says over how many beats of the tempo the clip plays once from its in point to its out point; more beats = slower. Any whole number is allowed: 16 becomes 17 with one press, and a clip of 17 beats drifts against the bars each time it loops (he was shown this sentence and kept beats). (b) The app's first guess for a new clip, as the page had it and he did not change it: a whole number of bars in groups of 4 (16, 32, 48 ... beats), and if the file is uneven the out point is moved in. His look at his Resolume differs (L21: "By default it gives me a decent amount of beats (sometimes 8, sometimes 16) so that it plays at about the same speed as it would play at speed number one in timeline mode"): CONFLICT, asked in H-5; (b) is not built until he answers. The Speed row in BPM mode: item R193 e.
CHANGED: (a) stands; the invitation "Say R218 bars" is declined by him: his earlier "lets do bars here not beats" (binding-decisions.md 761) is withdrawn for this row. (b) not named by him; under question because of L21.
TODAY: Every new clip starts at 4 beats / 4 beats; nothing sets a length from the file or the tempo (Clip.h:36-41; area-clip-transport.md part 1 item 10). BPM Sync shows a combo "Beats/ Cycle" and a slider "Content Beats" (item 7); a "Duration" row labelled "Beats" in BPM Sync does nothing (item 6). The adopted rulings RD2 / RA still say Bars in 4, 8, 12, 16 with "/2" greyed: to be re-cut (U-H10).
@@END
@@ITEM R193
TITLE: How a clip plays: the rest
STATUS: REPLACED in parts c and g by his words elsewhere; part h asked (H-1); parts a, b, d, e, f stand
HIS: L102, L21, L101, L13
RULE: (a) Only a video and a picture sequence have playback (BPM mode or Timeline mode, speed, pause, backwards, the loop menu, the start menu). (b) A still picture, a generated source, MilkDrop, the camera and an effects-only clip have none: they show on his press and run on their own clock; the tempo row's pause does not freeze them, and fired during a pause they show and move. This rests on the pause holding only BPM-mode clips; his L13 can be read wider: asked in H-8 (topic A owns the pause). (c) A fired video starts from its beginning unless its start menu says otherwise (item 202, L102); a clip set to play backwards keeps its direction and starts from its end. (d) A clip's pause is one thing shown in two places: the Clip tab's buttons and the layer strip's "<", "||", ">" are the same buttons of the clip that plays. (e) Speed is one control shown in two places (the strip's S slider and the Clip tab's Speed). In Timeline mode it runs to 10 (the real top is measured before it is built; if it is lower he is told first). In BPM mode, as his Resolume (L21): "Plus and minus doubles and halves the current. It goes: 0, 1/8, 1/4, 1/2, 1, 2, 4, 8, 16." (f) The master Speed changes only clips that are not in BPM mode. (g) Timeline mode, as his Resolume (L21): "In timeline mode duration changes don’t affect the speed setting but the video plays faster or slower based on the direction moved. Same with speed. It does not affect the duration setting but make the video move slower and faster." So Speed and Duration are two separate settings: a shorter Duration plays the video faster, a longer one slower, and the Speed number stays; a Speed change plays it faster or slower and the Duration number stays. Duration's "-" and "+" step 0.1 (his earlier words, binding-decisions.md 1002). How the two combine and what pulling the out point in does to the Duration number: ASSUMED, H-7. In BPM mode pulling the out point in keeps the same number of beats, so the clip goes through less video and looks slower (his rule, binding-decisions.md 778-780, in beats by L99). (h) What the clip's timeline shows: the page said bars only; his L101 and L21 speak of beat markers: one line per beat on a BPM-mode clip is my reading, asked in H-1.
CHANGED: (c) gains "unless its start menu says otherwise" (L102; the page said every fire starts from the beginning). (g) the page's "typing a Duration changes the Speed" is replaced by his look L21 (neither number changes the other); "bars" in its BPM half becomes "beats" (L99: INFERRED). (e) the BPM half is now exact from L21 (nine steps). (h) "bars only" is under question (H-1). (a), (b), (d), (f) unchanged.
TODAY: Playback only for video and sequence (Clip.h:248; U-H24). Speed slider 0 to 4 with two halve / double buttons and a Reverse text button (area-clip-transport.md part 1 item 5); in BPM Sync a video ignores the Speed slider and the tempo never enters its rate (item 8), a sequence follows the tempo and its Speed slider (item 9, correction 8). The Duration row does nothing (item 6). Master Speed scales Timeline clips only (item 26). The strip's buttons write the clip (item 17). No beat pause exists (item 23). The timeline draws beat-division lines (item 7).
@@END
@@ITEM R194
TITLE: The layer strip: fade, solo, bypass, blend
STATUS: STANDS part d confirmed by his look in Resolume
HIS: L29
RULE: (a) A new layer's fade (F) is 0, a cut; F counts seconds. (b) A third clip fired while a fade is running: the new fade starts from the picture as it is at that moment. (c) Solo on a layer that holds no clip does not darken the show. (d) B (bypass) and S (solo) are controls of the output. A clip on a layer that is bypassed, or hidden by another layer's solo, plays on out of sight, so when the layer comes back it is further on; his Resolume does the same (L29: "H it plays out of sight"). (e) The X's tooltip no longer promises that the clip comes back with Undo. (f) A new layer's Blend starts on "Add". His L111 ("I want to remove the keying and slider. We are only going to use the transparency slider to control that layers blend mode.") takes the keying and its K slider off the layer; what it means for the Blend list is ruled in topic I (question 204), and (f) follows that ruling. What a column click does to a layer whose cell is empty: his look L19 ("D layer goes empty when a column is triggered with an empty") agrees with the page; ruled in topic A.
CHANGED: nothing replaced. (d): the look he was asked for came back as the page had it (plays on), so "R194 d hold" is not taken. The solo half of (d) follows by the same rule: INFERRED.
TODAY: A new layer shows 0.3 seconds on F and cuts anyway; a solo on an empty layer darkens the show; a bypassed layer's clip appears to stand still (slice label: Layer.h:361-374, 402-403; LayerStrip.cpp:345-346; CompositorEngine.cpp:1043-1063, 1079-1080; seen in the code, not run). The K slider is on the strip and goes (L111).
@@END
@@ITEM R219
TITLE: The strip's thin playhead bar becomes a real jump
STATUS: STANDS
HIS: none
RULE: The thin bar under a layer's picture in the layer strip shows where the playing clip is, and dragging it moves the clip: the same rules as the Clip tab's timeline. The playhead never leaves the in and out points; a click outside does nothing; the picture waits while the mouse is held still; the clip plays on from where he lets go, at the same speed. For a clip in BPM mode his earlier answer "71 b" holds after the drop (it plays on and is cut once into time on the next "1"; binding-decisions.md 902). His L30 ("drag each play head to the beginning manually") speaks of playheads as things he drags, which agrees.
CHANGED: nothing.
TODAY: The strip's bar writes only the model; the renderer's write-back puts the player's real position back on the next frame, so the playhead springs back and the picture does not move (LayerStrip.cpp:979-989; area-clip-transport.md part 1 item 18; the spring-back INFERRED there). The Clip tab's timeline already seeks (item 19).
@@END
@@ITEM P28
TITLE: The layer strip as a whole
STATUS: DROPPED
HIS: L8, L7, L73, L111
RULE: laid out by Harmony where it fits in the correct area; its look is settled in the UI redesign (L8). What his words fix about its contents: a layer has an "ignore actions" toggle as well as "ignore column" (L7: "We should have ignore actions toggle on the layer as well as ignore column."; L73); the keying and its K slider leave the strip (L111); the action buttons sit in "a small row below the clip, still within the layer" (L56, topic D). The three play buttons stay on the strip as the clip's own buttons (R193 d). A paused clip is told apart from one the tempo row holds by a mark; how the mark looks is for the redesign.
CHANGED: All three drawn variants differ only in where things sit; none is drawn. The variant "the K slider ... leave the strip" is half settled by L111: the K slider is removed altogether.
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
STATUS: OPEN
HIS: L3
RULE: waits on the researcher's answer (answer-codec.md)
CHANGED: nothing; a new point of his message, it answers no numbered item. "DVX 3.0" read as Resolume's DXV 3: INFERRED.
TODAY: Video decodes off the GL thread through FFmpeg into a frame ring with budgeted uploads (CLAUDE.md pitfalls 56, 60, 62, 64); no own codec exists. Not checked further: the researcher's paper owns it.
@@END
## ASSUMPTIONS
@@ASSUME H-1
ABOUT: 201, R193
TEXT: I assume the timeline of a clip in BPM mode shows one line for every beat, as in your Resolume, and Random can land on any of them. Your earlier "Timeline only shows bars" then no longer holds for a clip.
WHY: L101 says "bpm lines", L20 and L21 say "beat markers"; his earlier words say the timeline shows bars only and Random jumps to "random 1's".
ALT: b) The timeline shows bar lines only, and Random lands only on the first beat of a bar.
IF-WRONG: STAGE a Random clip would come in on beats 2, 3 or 4 when he wants only the 1, or the other way round
ASK: YES his new words and his earlier words pull apart, and the audience sees where a jump lands
@@END
@@ASSUME H-2
ABOUT: 201, R192
TEXT: I assume Random jumps once every "Interval" beats (1 = on every beat) and lands at most "Distance" beats away from where the clip is, forwards or backwards, never outside the in and out points.
WHY: L20 describes one setting only (a jump on every beat); nobody has said what Distance does in his Resolume.
ALT: b) Distance is the size of the steps a jump is made of, not a limit. c) Interval and Distance are fixed, not his to set.
IF-WRONG: SMALL one rule of the jump changes
ASK: LINE a real choice of mine that he can check against his Arena in a minute
@@END
@@ASSUME H-3
ABOUT: 202, R193
TEXT: I assume the start menu only decides where a clip starts when it comes back to its layer. A clip in BPM mode still waits for the 1, and firing the clip that is already playing starts it from the start.
WHY: L102 says only "model ... after resolume"; his earlier "restart" for a column that is already playing names no menu.
ALT: b) With "carry on" set, firing the clip that is already playing changes nothing. c) A clip set to carry on does not wait for the 1.
IF-WRONG: STAGE a re-fired clip would restart when he expects it to play on
ASK: LINE his earlier "restart" points this way; he would most likely wave it through
@@END
@@ASSUME H-4
ABOUT: 202
TEXT: I assume the tempo row's Stop also forgets where every clip was: after Stop each clip starts from its start once, whatever its start menu says. "As if it had never stopped" counts beats for a BPM-mode clip and seconds for the others.
WHY: L102 names the menu, not its edges; Resolume's Stop rewinds everything (facts-resolume-emulate.md C3.4).
ALT: b) Stop leaves each clip's place alone.
IF-WRONG: SMALL one reset more or less
ASK: NO an edge of the menu; Resolume's own Stop does the same
@@END
@@ASSUME H-5
ABOUT: R218
TEXT: I assume a new clip in BPM mode gets its first number of beats in groups of 4 bars (16, 32, 48 beats), the nearest to its normal speed, and an uneven clip has its out point moved in.
WHY: He kept that part of the page unchanged, but his Resolume look (L21) gives 8 or 16 beats and uses the whole clip; L99 calls bars his mistake.
ALT: b) As in Resolume: 8, 16, 32 beats, whichever is nearest to normal speed; the whole clip is used and no out point moves. c) Groups of 4 beats (4, 8, 12, 16), out point moved in.
IF-WRONG: STAGE every new clip in BPM mode would start at another speed or length than he expects
ASK: YES his earlier rule and what he reports from his Resolume differ, and it touches every clip he brings in
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
@@ASSUME H-8
ABOUT: R193
TEXT: I assume the tempo row's pause holds only the clips in BPM mode and what follows the beat. A video in Timeline mode, a still, a generated picture, the camera and MilkDrop keep moving.
WHY: L13 says "Pause pauses, the beat clock, the clips and everything that it controls with BPM"; his earlier "136 b" held only BPM-mode clips; L31 names no mode.
ALT: b) Pause freezes every clip, whatever its mode, as his Resolume does. c) It freezes every video and sequence, but generated pictures and MilkDrop keep moving.
IF-WRONG: STAGE pictures would freeze, or keep moving, when he pauses in front of an audience
ASK: YES one sentence of his can be read both ways and the room sees the difference (topic A owns the pause; asked once)
@@END
@@ASSUME H-9
ABOUT: 201, R192, R217
TEXT: I assume Random and Beat Repeat work only on a clip in BPM mode. In Timeline mode the loop menu offers Loop, Ping Pong, Play Once and Eject, and Play Once and Hold.
WHY: His pictures and his looks (L20, L21) show Random and Beat Repeat only on a BPM-mode clip; no word of his names them in Timeline mode.
ALT: b) Both are offered in Timeline mode too, counted in beats of the tempo.
IF-WRONG: SMALL two entries more in one mode
ASK: LINE a real limit that he may want lifted
@@END
@@ASSUME H-10
ABOUT: R218
TEXT: I assume the first guess of a clip's beats is worked out once, when the clip comes into the show, and never changes by itself afterwards, also when the tempo changes.
WHY: L21 describes the default he sees, not when it is worked out or at which tempo; the adopted plan uses 120 BPM.
ALT: b) It is worked out at the tempo of the moment the clip is brought in.
IF-WRONG: SMALL the starting number differs; he sets it himself anyway
ASK: NO internal; it follows from the answer to the first-guess question
@@END
## QUESTIONS BACK
(none: this task names no question of his to answer; his question of L3 is item G1 and is answered by the researcher's paper)
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
MEANS: The second small menu of a clip, which decides where it starts when fired: Restart (from the start), Continue (from where it was), Relative (as if it had never stopped).
SOURCE: Harmony's pick for the menu's name; the three entries are the on-screen names now, kept until his list of names says otherwise
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
## CONFLICTS
- Beat markers against bars only. L101: "each of the markers on a clip are bpm lines and the random lands on one of them" and L21: "Bpm mode the clip’s timeline is divided into beat markers." against binding-decisions.md 825-826: "Timeline only shows bars. The only place we see beats is in the circle with 4 positions in top bar that shows the 4 beats repeating." and 1001: "this is essentially jumping to random 1's on the beat". Asked: H-1.
- The first guess of a clip's beats. L21: "By default it gives me a decent amount of beats (sometimes 8, sometimes 16) so that it plays at about the same speed as it would play at speed number one in timeline mode" and L99: "do beats here. It was my mistake before" against binding-decisions.md 812-813: "always work with multiples of 4. If it is uneven, then move the outpoint in to keep those multiples." and 846: "a short clip can have 4 bars but it will move super fast". Asked: H-5.
- What the tempo row's pause holds (owned by topic A; it decides R193 b). L13: "Pause pauses, the beat clock, the clips and everything that it controls with BPM." against binding-decisions.md 1009: "136 b" (the option he chose: only BPM-synced clips hold). Asked: H-8.
- For the record, not asked (he was asked exactly this and answered): L102: "model these 2 little menu’s after resolume" replaces binding-decisions.md 844: "42 default".
## NOT DONE / UNSURE
- What the three entries of the start menu read in his Arena, and that there are exactly three: taken from the earlier fact sheet (the manual's "from the start / pick-up / relative pick-up"), not seen; his picture shows the menu closed, as an icon. Cheapest: one picture of that menu open.
- What Distance does in his Resolume's Random (H-2): no source. Cheapest: he sets Distance to 1 and to 8 and says what changes.
- Whether his Resolume offers Random and Beat Repeat in Timeline mode (H-9): not established. Cheapest: one look at the loop menu of a Timeline-mode clip.
- "In time" for a BPM-mode clip whose Beats is not a multiple of 4 (after a playhead drag, Beat Repeat without Catch up, a Resync): his "71 b" and "124 a" say it is cut into time on the next "1", written when lengths were whole bars. With free beat counts the cut can only line the clip's beat markers up with the beat. Technical; for the transport delta, not for him.
- The adopted transport rulings (RD2, RA: Bars, the bar lists of Random and Beat Repeat, no start menu, the fit rule) were written before L99, L101 and L102 and have to be re-cut; I did not re-read them, only the area sheet's account of them.
- G1 is not answered here by instruction.

Written 2026-10-07 22:41:44 EDT by the architect seat of topic H. Read-only; nothing built, run or committed.
