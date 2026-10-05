# SEAT 2 -- cover-clip-transport (blind seat, round 2): does the ONE list cover every point of the area "how a clip plays"?
Written 2026-10-05 16:45:19 EDT by the cover-clip-transport seat. Read-only except this file. Nothing built, run, launched, committed.
Read: area-clip-transport.md (parts 1-6 + CORRECTIONS + MISSED U-H20..U-H27), boris-all-items.json (questions 172-215, readings R128-R213, decided 1-28, design 1-31, conflicts 1-24, area_points), BD lines 658-660, 725-730, 750-756, 776-781, 810-816, 842-847, 995-1006, 1019-1027, 1076-1104, BL:85. Source re-checked by me at HEAD f51f32d: MainComponent.cpp:355 (button "Image Folder"), :357 (label "Beats per Image"), :452-468; MainComponent.h:355; LayerStrip.cpp:972-992 (scrubPlayhead writes only clip->playheadPosition); DeckCommands.h (ClearActiveClipCmd :392, ToggleLayerFlagCmd :435, RemoveLayerCmd :538); TriggerCommands.h:62; nudge-row2 ruling table (lines 25-60); facts-resolume-emulate.md C1.5, C2.1, C2.7 (quotes re-read); facts-resolume-transport.md :47-51, :139.
Labels: VERIFIED = read by me (file:line); INFERRED = reasoned from what is named; UNKNOWN = cheapest check named.

## VERDICT: FAIL (small repairs). 2 MUST, 10 SHOULD, 8 NIT. Of 56 points checked, 47 are covered, 4 partly covered, 5 have NO home.

## A. COVERAGE TABLE (point -> home)
Home legend: Q = question, R = reading, dec = decided line, des = design item. "COVERED" = his answer or silence lets a builder go on.
| Point (sheet) | Home | Verdict |
|---|---|---|
| U-H1 layer while a BPM clip waits | Q173 (A old clip plays on; B new shows frozen); mark on cell = des 1; R168(d); R179(a) | COVERED |
| U-H2 edges of 154: second press, X, empty cell, paused BPM clip, exactly on the "1", no tempo | R168 (a)(b)(c); R174(a); dec 4 (a tenth of a beat); Q175 / R139 for no tempo; Cmd+Z of a fire = R198 | COVERED |
| U-H3 re-fire of the playing BPM clip | R128(d) (mine, he can correct); Resolume fact backs it: its quickstart says clicking the thumbnail again "will start again at the start of the next bar" (facts-resolume-emulate C2.7, quote re-read) | COVERED (but R128 is missing from the intro's "look at first" list: F8) |
| U-H4 column with both kinds of clip | Q191 (+R128(e)) | COVERED |
| U-H5 what "starts the beat" means: which clips, tempo, Auto/Manual, non-hand fires, "load up and press start" | Q172, Q175, R139(b), dec 1, Q189 (hand vs Auto), R213(b) | COVERED, with one hole: Q172 A rests on "pause pressed on a stopped beat" which no ruling defines (F5) |
| U-H6 an action's fire while stopped / paused | R158(d)(e) (no action plays while stopped; all hold while paused), dec 1 | COVERED |
| U-H7 non-BPM clip fired while paused; play press after | Q174; R139(d) | COVERED (wording slip in Q174: F9) |
| U-H8 four pauses / three stops on screen | R176(a)(b), R193(d), R120 (told in chat, names), R197(a); old top-bar buttons go = BD:1021 (not restated; NIT N5) | COVERED |
| U-H9 what a re-opened show shows | Q196 (A empty layers + pause mark; B layers come back); R142; R163 | COVERED |
| U-H10 unit and step of the length row | Q202 | COVERED (wording: F6) |
| U-H11 Random landing | Q201; R192(b) | COVERED |
| U-H12 Beat Repeat buttons / unit | R192(a) (picture list, beats; replaces the planned list) | COVERED |
| U-H13 eight cue points | R179(d) (keep, name "Cuepoints") | COVERED |
| U-H14 folder of pictures / old slideshow row | Q211 | COVERED, but Q211 names the old row wrongly: F1 (MUST) |
| U-H15 Cmd+Z and what is live | R198 (a)(b)(c)(d) | PARTLY: R198(a) lists pause, speed, direction "while it plays" but not loop style, mode, length, in / out points, Beat Repeat, Random; and a deleted layer "comes back not playing" (BD:767) is lost (F7) |
| U-H16 can an action press play / pause | Q193 (A: no) | COVERED |
| U-H17 preview of a clip that also plays live | R151 ("same moment of its playback"); R193(c) fire starts from beginning | COVERED for a playing clip. NONE for a preview while the beat is stopped or paused (F4) |
| U-H18 loop style "Eject" against the clip's own autopilot "Play Next"; two "Random" | R203(b) says the clip's list stays; R213 does not say who wins; the word "Eject" appears nowhere in the list (grep) | NONE (F2, F3) |
| U-H19 speed controls that overlap; master Speed | R193(e)(f) | COVERED (clause owed on the "to 10": F6; CORRECTION 8: in BPM Sync a SEQUENCE is also scaled by the clip's Speed slider, a video is not; R193(e) says "one control, nine steps" for BPM mode, which is silent on a sequence: NIT N6) |
| U-H20 backwards clip fired again | R193(c) | COVERED |
| U-H21 Beat Repeat on fire / drag / cue jump / pause / reverse; saved? | R192(c) (goes off on a new fire or leaving the layer; not saved) | PARTLY: drag on the timeline, a jump point, pause, backwards not named (NIT N3) |
| U-H22 autopilot fire = a fire? | R213(a) (also dec 1, R213(b)) | COVERED |
| U-H23 momentary pad released before the "1" | Q207 A | COVERED (the hold rule is bundled in option A only: NIT N7) |
| U-H24 which clips have a transport | R193(a)(b) | COVERED |
| U-H25 clip waiting + tempo row touched | R174(b) (Resync, /2, x2, tap, typed, stop) | PARTLY: pause pressed while a clip waits not named (NIT N2) |
| U-H26 one-shot in BPM mode | R174(c) | COVERED |
| U-H27 Timeline: out point in, typed Duration | R193(g) | COVERED (BPM-mode contrast owed: F6) |
| Part 5: 154 remainder | Q173 | COVERED |
| Part 5: 73 (old shows' BPM clips) | dropped: BD:1095 R117 words "no shows are saved and the app is not being used for anything" + R143. INFERRED (his words are about routines, shows and recordings; BPM clips of old shows are covered by "old shows") | COVERED, accept |
| Part 5: RQ-0 the three tracks | R164 (NIT N4: say what he hands over) | COVERED |
| Part 5: RQ-3 plus on Speed / Duration | R206(f) | COVERED; NIT N1: half of it is already answered (BD:1002 "time jumps 1 bpm, duration in timeline mode moves up 0.1") and 137 settled the Beats row, so (f) re-asks answered numbers |
| Part 5: 172 / 173 / 174 | Q172 / Q173 / Q174 | COVERED |
| O1 123 b vs 154 | R175(1) | COVERED |
| O2 global Quantize gone | R175(4), R128(b) | COVERED |
| O3 only play / Resync starts a stopped beat | R175(2), R139, dec 1 | COVERED |
| O4 111 / 136 b vs R111 pause | Q174 | COVERED |
| O5 slide / nudge replaced by one cut | R175(1) ("cut once", "Resync cuts at once"); 9 b / 33 b are not named but not carried | COVERED |
| O6 pause belongs to the layer replaced | R176(b), R193(d), R142 | COVERED |
| O7 beats vs bars | Q202 | COVERED |
| O8 timeline lines per beat vs bars | R193(h) | COVERED |
| O9 three old buttons go; Stop = rewind (stale doc) | R176; BD:1021 not restated | COVERED, NIT N5 |
| O10 routines -> actions, TB-8, routine start rule | R143, R158, dec 12 | COVERED |
| O11 restart of an already playing clip | R128(d), R193(c) | COVERED |
| O12 sync dial | architects | n/a |
| O13 BeatLoopr name / list | R192(a) | COVERED |
| O14 "layer strip" widened | R198 | COVERED (but see U-H15) |
| CORR 1-3 (attribution faults), 4-6, 7 | no effect on the list: I checked the list's quotes of 123 b, 122 a, 71 b: R175 and Q-texts use only his own letters and BD words. BD:1000-1002, :1011, :1025 quotes in Q201 / Q202 are verbatim | OK |
| CORR 8 (sequence rate x Speed slider in BPM Sync) | R193(e) silent | NIT N6 |
| Part 1 items 1-32 vs the list's "today" claims | see section B | 2 faults |
| Part 3 O-lines (14), part 4 HIS (27), part 5 (7), corrections (8) | 56 points | 47 covered / 4 partly / 5 NONE (U-H18, Part-1 item 4 "Restart / Continue / Relative" menu, Part-1 item 18 strip playhead bar, U-H17 second half, loop-menu list) |

## B. "TODAY" CLAIMS IN MY AREA CHECKED AGAINST THE SHEET AND SOURCE
- Q211 situation: "Open Folder, with a list from 2 to 128 beats" -> VERIFIED WRONG: the button reads "Image Folder" (MainComponent.h:355 `TextButton openFolderButton_{"Image Folder"}`), the list is labelled "Beats per Image" (MainComponent.cpp:357), 2 to 128 right (:455-457). "only while no layer plays" is INFERRED by the sheet (FAC:33), not run, and is worded as fact. -> F1.
- Q211 "Today three or more pictures become one clip": VERIFIED (MainComponent.cpp:843 "3+ as an ImageSequence").
- Q202 option A "the app's first guess for a new clip is still a multiple of 4 bars": "still" says it is so today. VERIFIED today: every new clip is 4 beats / 4 beats (sheet part 1 item 10; Clip.h:36-41); a length from the file in bars is only ruled, not built. -> F6.
- R198(c) "Today the app still undoes a fire, B, S and X": VERIFIED (TriggerCommands.h:62; DeckCommands.h:435, :392).
- R213(a) "Today the autopilot resumes a clip where it was and brings it in at once": VERIFIED by the sheet's CORRECTIONS (Autopilot.cpp:326, :355, :417; MainComponent.cpp:4881).
- R193(a)(b) which clips have playback: VERIFIED by CORRECTIONS (Clip.h:248 isPlayable).
- Q173 option A quote ("at the start of the next bar, the output will change to play the new clip"): VERIFIED, facts-resolume-emulate.md:115 (quickstart page).
- Q207 quote "Momentary pad released before its quantized beat: now cancels": VERIFIED, boris-feedback-backlog.md:85.
- Q175 "Today the app has no tempo until it has heard enough music": VERIFIED by sheet item 23.
No other contradiction between a list item and the sheet's part 1 found.

## C. WHAT IS MISSING OR WRONG -- findings with exact wording (see the return value for the short form)
F1 MUST Q211 (on-screen words, today claim). Replace "A separate old row at the top (Open Folder, with a list from 2 to 128 beats) shows one picture every so many beats, but only while no layer plays." with: "A separate old row at the top (a button \"Image Folder\" and a list \"Beats per Image\", 2 to 128 beats) shows one picture every so many beats; I believe only while no layer plays (read in the code, not seen running; one look at your app settles it)." Also in options A and B: "The old row at the top goes" stays.

F2 MUST NEW reading (H) -- the loop menu is nowhere on the list. Add as the next free reading number:
"R214 (H) The loop menu of a clip. As in Resolume (its manual, and your picture): Loop; Ping Pong (forwards, then backwards, again and again); Random (the clip jumps by itself: R192, question 201); Play Once and Eject (plays once, then the clip leaves its layer and the layer is empty); Play Once and Hold (plays once and stays on its last frame). Today the menu reads Loop, Ping Pong, One Shot (ClipInspector.cpp:56-58). Eject is the one to look at: on stage a clip that finishes takes its layer to black. In Timeline mode Resolume counts Random's Interval and Distance in seconds, in BPM mode in beats. Mine: what a Play Once clip does with its own 'what comes next' list: question 217."
Source: facts-resolume-transport.md:47-51, :139 (CONFIRMED); area sheet part 2 (RA-8, D-12 "told as a difference, no answer"); BD:1025-1027 (loop menu in his picture). The word "Eject" is absent from the list (grep). Silence is not consent on this page, and the earlier mention (D-12) got no answer.

F3 SHOULD NEW question (H) -- U-H18, who wins at the end of a clip. Proposed:
"Q217 (H) Two things decide what happens when a clip ends: its loop menu (Play Once and Eject, Play Once and Hold) and its own 'what comes next' list on the Clip tab (Play Next, Play Random, Play First ...). A clip on layer 1 is set to Play Once and Eject and its own list says Play Next. It reaches its end. A: its own list wins: the next clip starts; Eject empties the layer only when the list says Do Nothing or Layer Determined. [mine: you chose the list on purpose, so it is the more specific wish] B: Eject wins: the layer goes empty and the list does nothing. C: the list is removed from the Clip tab; the layer's autopilot alone decides. Why it matters: on stage the difference is a clip that follows on or a layer that goes black." Remove the clause 'question 217' from R214 if the seat that numbers the list renumbers.

F4 SHOULD NEW question (B/H) -- U-H17 second half, preview while the beat is held.
"Q218 (B) You cue ahead of time (R140, R149 f): the beat is stopped or paused and you click a BPM-mode clip's name to look at it in the preview monitor. A: it stands on its first frame in the preview, as it would on the output (R139). B: in the preview only, it plays at the tempo number as if the beat ran, so you can judge it; the output is untouched. [mine: A is least surprise; B is what a DJ's cue does] Why it matters: with A you cannot judge a BPM-mode clip's motion before the show starts." Default A [mine].

F5 SHOULD Q172 -- option A depends on a state no ruling defines: "pause pressed on a stopped beat". The adopted table (ruling-nudge-row2.md:33-47) defines pause = all nine fields held and "play from pause: runs on from the held place, nothing is counted", and "play from stop = beat 1"; R139(d) and Q172 A say "held at its start, your play press is the 1". Add to decided: "Pause pressed while the beat is stopped holds it on the 1 and leaves the layers as they are; play from that state is beat 1 (the only difference from Stop is that the clips stay)." Also in Q172's situation add "(the pause button also works from stop)". If he picks B the line is not needed. Status: INFERRED that the architects can do it; UNKNOWN whether the adopted tempo-row text allows it (cheapest: an architect reads NR2 section 1).

F6 SHOULD (wording, three places).
(a) Q202 A: replace "the app's first guess for a new clip is still a multiple of 4 bars" with "the app's first guess for a new clip will be a whole number of bars in multiples of 4 (today every new clip starts at 4 beats)"; and add one sentence: "The '/2' and 'x2' buttons next to the row halve and double the beats." (the buttons' fate under A is not stated; BD:731, RD2).
(b) R193(e): after "(to 10 when the clip is not in BPM mode ...)" add "(the real top is measured before it is built; if it is lower than 10 you are told first)". Source: sheet U-T4, RA TB-10; BD:907 promises 10.
(c) R193(g): add a sentence for BPM mode, his words BD:778-781: "In BPM mode pulling the out point in keeps the same number of beats, so the clip goes through less video and plays slower."

F7 SHOULD R198 (U-H15). Replace (a) with: "(a) Mine, what 'etc.' covers once everything is built: firing a clip or a column, a layer's X, the tempo row, and every control of the Clip tab that changes how the clip plays (pause, speed, direction, loop style, mode, length, Beat Repeat, Random, in and out points), an action's on / off button, an ignore lamp, a cue button, a change in the keyboard and MIDI mapping, an output screen's settings. Today none of the Clip tab's play controls is an Undo step and that stays." And to (b) add "A layer you delete comes back with Undo, with nothing playing on it (your words)". Source BD:767; sheet part 1 item 28; DeckCommands.h:538.

F8 SHOULD Intro line "Fourteen readings hold something that you would see on stage and that is mine": R128 (d), R139 (b)(d), R193 (b)(c)(f), R192(c) are mine and stage-visible and not on the list. Make it "Seventeen readings ... R128, R129, R131, R139, R149, R160, R168, R172, R174, R181, R186, R189, R193, R199, R205, R210, R213" (and R192 if the merger wishes).

F9 SHOULD Q174 situation/option A: "You fire a clip that is NOT in BPM mode" and "nothing you fire moves, whatever its mode" contradict R193(b) (a generated source, camera, still fired during a pause show and move). Fix the situation: "You fire a video or a picture sequence that is not in BPM mode." and in A: "... nothing that has playback moves ...". Source: R193(a)(b), sheet CORRECTIONS item 24 (Clip.h:248).

F10 SHOULD NEW question (H) -- "Restart / Continue / Relative" menu (Part-1 item 4, plan D2-1). His two statements disagree: "42 default" (BD:844: every fire starts the clip from its beginning, no per-clip carry-on menu) and "r87 this is the resolume bpm clip menu, lets model ours based on this" (BD:1025; the adoption block lists the picture's "play-out menu" among the rows). Today the menu is on screen and does nothing (ClipInspector.cpp:69-73; sheet item 4: no handler, no field). Proposed:
"Q216 (H) In your Resolume picture the clip panel has a small menu next to the loop menu (Restart / Continue / Relative) that decides where a clip starts when you fire it again. Today ours shows it and it does nothing. You said '42 default': every fire starts the clip from its beginning, no menu. You also said to model ours on the picture. A: no such menu: every fire starts the clip from its beginning. [your words: 42 default] B: ours shows the menu with Resolume's entries. Why it matters: with B a fire no longer always means 'from the start' (R193 c, R128 d)."
Default A. (Rule f: two of his own statements, the later one does not plainly replace the earlier.)

F11 SHOULD NEW reading (H) -- the layer strip's thin playhead bar (Part-1 item 18). Today dragging it writes only the model position and the renderer puts the real position back (LayerStrip.cpp:979-989, VERIFIED the write; the snap-back is INFERRED from FT:36), so it appears to do nothing. Add:
"R215 (H) The thin bar under a layer's picture in the layer strip shows where the playing clip is. Today you can drag it, and it springs back (it does not move the clip). Mine: it becomes a real jump with the rules you gave for the Clip tab's timeline: it never leaves the in and out points, a click outside does nothing, the clip plays on from where you let go at the same speed." Source: BD:658-660, :725-730, :750-756; sheet items 18-19.

F12 SHOULD NEW design item (H): the clip's Transport panel is not on the design list (only the strip, design 26). Add: "The clip's Transport panel (Clip tab), modelled on your Resolume picture: where Beat Repeat's row, Random's Interval / Distance rows, the Beats row and the eight Cuepoints sit. Variants: exactly as the picture with Beat Repeat as a row of buttons under the timeline; Beat Repeat and Random collapsed behind the loop menu; the Cuepoints in a fold of their own."

## D. NITs (paper only)
N1 R206(f) re-asks numbers he already gave (BD:1002) and the Beats row settled by 137; narrow it to "in BPM Sync click '+' on Speed and send the numbers" or drop it.
N2 R174(b): add "pause holds the wait: it runs on when the beat plays".
N3 R192(c): add "a drag on the timeline, a jump point, pause or playing backwards also switch it off".
N4 R164: add what he hands over ("about ten minutes each; tell me where the files are", BD:1040, BD:1098).
N5 Old top-bar Play / Pause / Stop go (BD:1021, R85); R176 implies it but never says so, and the old Stop (routines only today) is renamed in R120 only for the mapping. One sentence in R176: "The three old buttons in the top bar go; the tempo row's three take their place."
N6 R193(e): add "a picture sequence in BPM mode also follows the Speed slider; a video does not (today)" if it is meant to stay, else "in BPM mode Speed is the nine steps for both" (CORRECTIONS 8: Renderer.cpp:1707-1709).
N7 Q207: the hold rule ("a held pad let go before the 1 starts nothing") is stated only inside option A; add it to the WHY so B and C answers do not drop it.
N8 R193: add (i) the settled playhead rules in one line: "The playhead stays between the in and out points; a click outside does nothing; a drag plays on from where you let go; the picture waits while the mouse is held still" (BD:658-660, 727, 752-756). Design item 1 should also say the cell's pause mark (R142, Q196).
Also: ">|" (the strip's fourth button, one Speed step; hidden unless the strip is wide; LayerStrip.cpp:388-391) is not mentioned; design 26 may cover it.

## E. WHAT I COULD NOT SETTLE
- UNKNOWN: whether the Restart / Continue / Relative menu is visible on his screen (certain in code, ClipInspector.cpp:69-73). Cheapest: one look.
- UNKNOWN: whether Resolume's Random exists in Timeline mode with seconds (manual says "seconds or in beats", FRT:139); R214 words it as such.
- UNKNOWN: NR2 sections 1-6 were read by me only for the table at lines 25-60, not the tracker's Auto path (sheet U-T2): what Stop and "a fire starts the beat" do while the tracker is in AUTO on a DJ track is not on the list for him (it is Q189's neighbour). Cheapest: an architect reads NR2 NC-5 / T-G6; if the answer changes what he sees, one more question.
