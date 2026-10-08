# APPLY A -- Firing clips and the tempo row (s-rta-1007)

Architect paper, read-only work: nothing built, run, launched or committed. Boris is quoted only from boris-msg-numbered.txt (L numbers) and binding-decisions.md (BD line). Labels inside fields: VERIFIED = his words say it; INFERRED = reasoned from his words; ASSUMED = see the named @@ASSUME.

## SUMMARY
1. No paused show start is built (L30): clips start together only by the column trigger; with the beat stopped that click is the new "1" and everything in the column plays at once.
2. Stopped against paused is now his own rule (L12, L31): STOPPED = his click on play or on any clip is the new "1" and the clip plays at once; PAUSED or RUNNING = a fired BPM-mode clip waits for the "1".
3. Harmony's seed holds against every line of his: a clip fired while PAUSED starts the beat clock again from where it was held (L13, L17) and the BPM-mode clip then starts on the next "1" (L12, L31). One hedge is left (L12 and L31 alone could also mean "stays paused until play"): assumption A-1, ASK YES.
4. An empty cell now waits for the "1" when the clip playing on that layer is in BPM mode, the Resolume way (L11); the page had it clearing at once.
5. "/2", "x2" and any tempo change never move the "1"; pause and play never move a clip back to the "1" (L13). The page's "a tap or a typed tempo move the 1 it waits for" is gone.
6. The tempo runs from 22 to 480 (L14), not 30 to 400; "/2" and "x2" work to both limits, by hand and on the listening clock.
7. "nudge X ms" is shown only in automatic mode (L14); in manual mode the nudge just moves the "1".
8. The circle: one full turn is one bar, each of its four spots one beat (L14); this settles "four beats to the bar".
9. Question 189 (his hand against the app's listening) is OPEN: he asked back (L32); a separate paper answers it. Everything that leaned on 189 A is marked.
10. Ableton Link is no longer "left as it is": his "215 plan all of these" (L128) puts it on the plan.
11. Two things for him to answer (A-1 fire while paused, A-2 what a tap moves) and eight lines he can strike.

## ITEMS

@@ITEM 172
TITLE: How a show starts: the column trigger, no paused start
STATUS: ANSWERED in words that fit neither letter
HIS: L30, L12, L31
RULE: No paused start is built. Several clips start together in one way only, the column trigger (L30: "The only way to do this is with a column trigger."). With the beat stopped, a click on a column trigger is the new "1": the beat starts on that click and every clip of that column plays from its beginning at once, whether it is in BPM mode or not (L12: "When stopped, user’s click on play or any clip is the new 1."; L31: "if I was stopped, it would play the moment I triggered it and I would be setting a new 1 for the tempo"). Pressing play on a stopped beat with empty layers also starts the beat, and that press is the new "1" (L12). Pause is never a way to stand clips ready on their first frame: nothing is built for that. The other route he names (L30: "trigger all the clips, press, pause, then drag each play head to the beginning manually") stays possible only as a by-product of pause and of dragging a playhead; nothing is added to make it easier. Looking at clips before they go out stays the cue system (topic B).
CHANGED: Option A (the default: pause pressed on a stopped beat, each fired clip standing on its first frame, the play press as the "1") is removed. Option B's content holds, in his words. His earlier sentence "load up the clips and press start" (binding-decisions.md:1092) is no longer read as a second way to start clips together (L30 is the later word).
TODAY: The app has no stopped or paused beat at all (facts-takes-bpm-fire.md S4; area-tempo.md part 1 point 3). A column click fires every cell of the column (facts-takes-bpm-fire.md 5.11). The beat's stop, pause and play, and "a fire starts a stopped beat", are all to be built (area-tempo.md R-07, O-03).
@@END

@@ITEM 173
TITLE: What the layer shows while a BPM-mode clip waits
STATUS: DEFAULT A (his L1); tested against his whole message, nothing says otherwise
HIS: L1, L31
RULE: While a BPM-mode clip waits for the "1", the clip that was playing on that layer plays on; on the "1" the new clip takes over and starts from its beginning. On an empty layer nothing shows until the "1" (his own picture of it, L31: "There is nothing in the layer. The tempo is paused. I click on a BPM clip and it waits for the bpm clock to return to the 1 and then fires."). The waiting clip's cell shows that it is waiting; how that mark looks is laid out by Harmony (item P1). The frozen-first-frame variant (option B) is not built.
CHANGED: nothing
TODAY: A fire can wait only through the top bar's Quantize box or the clip's Snap box; while it waits the old clip plays on and nothing on screen shows the waiting clip (facts-takes-bpm-fire.md section 3 line 46, S3). BPM mode by itself makes nothing wait (section 3 line 40). To build: the wait that BPM mode implies, and the waiting mark.
@@END

@@ITEM 174
TITLE: A clip fired while the beat is paused
STATUS: ANSWERED in words; they describe a BPM-mode clip, the question's own case is assumption A-3
HIS: L31, L12, L13, L17
RULE: BEAT PAUSED, he fires a BPM-mode clip: the clip waits for the "1" and starts there, from its beginning (L31: "I click on a BPM clip and it waits for the bpm clock to return to the 1 and then fires."; L12: "in pause or run mode, unless resync is clicked, when a new clip with BPM mode is triggered, it waits for the one."). The fire itself sets the beat clock going again from the position it was holding at, as a press on play would (L13: "that press starts the clock from when the user clicks play or the clip ... the user started the clock from the position it was holding at"; his Resolume does the same, L17) -- this half is assumption A-1. BEAT STOPPED: the clip plays the moment he triggers it and that press is the new "1" (L31: "The only difference is if I was stopped, it would play the moment I triggered it and I would be setting a new 1 for the tempo."). A CLIP ALREADY PLAYING WHEN HE PAUSES THE TEMPO stays on screen, held on its frame; when he presses tempo play it plays on from where it was paused and the beat clock continues from where it was paused (L31). A clip that is NOT in BPM mode, fired while paused (the case the question asked about): it plays at once on his press (L11: "If a layer that is not in BPM mode is triggered, then that plays instantly") and a clip not in BPM mode that was already playing plays on through a pause (assumption A-3).
CHANGED: Option A (the default: no fired clip moves while the beat is paused, whatever its mode; it stands on its first frame until play) is replaced by his words. Option B is not chosen by letter; its content for a clip that is not in BPM mode is what assumption A-3 keeps.
TODAY: No paused beat exists (facts-takes-bpm-fire.md S4). The adopted but unbuilt tempo-row text has a fired BPM clip appear at once on its first frame and wait for play (facts-takes-bpm-fire.md section 4 line 60, 5.5, 5.9): that text is void now.
@@END

@@ITEM 175
TITLE: The tempo of the very first fire
STATUS: DEFAULT A (his L1); nothing in his message says otherwise
HIS: L1
RULE: The tempo number is never empty. The app opens at the tempo he used last, and at 120 the very first time. His first fire, or play, starts the beat at that number and is the "1" (L12); the music, a tap or a typed number then corrects the number. The last-used tempo is one number kept with the app, not with a show (item D2).
CHANGED: nothing
TODAY: At launch the app has no tempo until its listening locks, and the beat does not advance meanwhile (area-tempo.md part 1 point 4); nothing stores a tempo, neither the show nor the app's settings (area-tempo.md part 1 point 21). To build: the remembered number and a beat that can start at it.
@@END

@@ITEM 189
TITLE: His hand against the app's listening
STATUS: OPEN he asked back; no letter taken
HIS: L32
RULE: waits on Harmony's answer to his question back (L32)
CHANGED: The default A is NOT taken by his first line: he answers the question with a question of his own (L32: "is it possible to correct the app listening to the music if it is off by two BPM's and keep it on automatic mode, or will that correction necessitate going back to manual mode? This is a question for you.  Same question if I correct, where the one is."). Every sentence elsewhere in this paper that leaned on 189 A is marked as waiting on it (items R164, R176, R215, 190, D3; assumptions A-9, A-10).
TODAY: While the app listens, every beat it is sure of pulls the beat back, and a tapped tempo is replaced after about two seconds if the music disagrees; a Resync holds against the listening only in Manual (area-tempo.md part 1 points 5, 7, 8; U-HIS-4; O-20). The answer paper is answer-tempo-auto.md (not mine).
@@END

@@ITEM 190
TITLE: Tap: the tempo only, or the beat too
STATUS: DEFAULT A (his L1), but his own look in Resolume (L18) can be read against the default's reason: assumption A-2
HIS: L1, L13, L18
RULE: Tap sets the tempo number only, from the second tap on. It never moves the "1" and never shifts the beat; the "1" is placed with Resync. A tap never starts a stopped beat (binding-decisions.md:957, his "128 tapping tempo does not start anything but when click resync, that is the 1 and it begins on that button push"). A BPM-mode clip that is playing follows the new tempo at once. Whether a tap keeps the app in automatic mode waits on question 189.
CHANGED: nothing in the letter. His L13 supports it ("/2 and x2, tempo change do not move the 1"). His look L18 ("the beat reacts instantly to the tapping on the second tap, and the clip responds to the beat changing") is evidence that the default's reason ("as Resolume, my guess") may be wrong: it reads either as "the number changes on the second tap" (fits A) or as "the beat jumps to the tap" (option B).
TODAY: From the second tap on every tap sets the tempo AND pulls the beat onto the tap, though not the "1" (area-tempo.md part 1 point 7, M-2); under A the pull is removed.
@@END

@@ITEM 191
TITLE: A column that holds both kinds of clips
STATUS: DEFAULT A (his L1); his L11 says the same for the single clip
HIS: L1, L11, L30
RULE: A click on a column trigger while the beat runs: each clip follows its own rule. A clip that is not in BPM mode starts on the click (L11: "If a layer that is not in BPM mode is triggered, then that plays instantly"); a BPM-mode clip starts on the next "1". For up to a bar the column is only partly there. An empty cell of the column clears its layer by the empty-cell rule of item R168 (on the "1" when the clip playing there is in BPM mode, at once otherwise); a layer set to Ignore Column Trigger is left alone. With the beat STOPPED the click is the new "1" and all of the column plays at once (item 172). With the beat PAUSED the click sets the clock going from where it was held and the same per-clip rule applies (assumption A-1).
CHANGED: nothing in the letter; the empty cells of the column now follow L11 (INFERRED by his L9: explained once, used wherever it applies).
TODAY: A column click fires every cell at once, and an empty cell clears its layer at once (facts-takes-bpm-fire.md 5.11; slice text of R205: Layer.h:402-403, Composition.h:447-448, not re-read by me).
@@END

@@ITEM R128
TITLE: A BPM-mode clip starts on the next "1"
STATUS: STANDS -- his L11 and L12 repeat its core
HIS: L11, L12
RULE: (a) A BPM-mode clip fired while the beat runs or is paused waits and starts playing on the next "1", by itself, unless Resync is clicked first: then that press is the "1" and the clip starts on it (L12). A clip that is not in BPM mode starts on his press (L11). (b) There is no Snap box on a clip and no Quantize setting for firing clips live; the review screen has its own Quantize. (c) The wait is part of BPM mode itself, not a separate setting. (d) Firing the clip that is already playing: it plays on and starts again from its beginning on the next "1". (e) A column click starts its BPM-mode clips on the next "1" and the others on the click (item 191). With the beat stopped none of this applies: the press is the new "1" (item R139).
CHANGED: nothing. Added from L12: "unless resync is clicked", and that the wait holds in pause as well as in run.
TODAY: The Snap box on a clip and the top bar's Quantize box exist and are the only things that make a fire wait (facts-takes-bpm-fire.md section 3 lines 41-45; area-tempo.md part 1 points 13, 14). The unbuilt quantize-out lane deletes exactly the wait machinery a BPM-mode wait would need (facts-takes-bpm-fire.md section 6): an architect delta is owed there.
@@END

@@ITEM R168
TITLE: While a clip waits: newer fire, empty cell, the layer's X
STATUS: CORRECTED part (b)
HIS: L11
RULE: (a) While a BPM-mode clip waits for the "1", a newer fire on the same layer takes its place: the last press wins. (b) A click on an empty cell: if the clip playing on that layer is in BPM mode, the layer is cleared on the next "1", the same wait as a new BPM-mode clip; if the clip playing there is not in BPM mode, the layer is cleared at once (L11: "let's do the resolume way where the empty cell triggers on the 1 if the clip playing is in bpm mode, same as a new clip in bpm mode. If a layer that is not in BPM mode is triggered, then that plays instantly"). A waiting empty-cell clear follows the last-press rule of (a) like any waiting fire (assumption A-6). The layer's X clears the layer at once, in the middle of a bar too (assumption A-5). (c) Pressing the waiting clip again changes nothing: it keeps waiting for the same "1". (d) The layer's fade from the old clip to the new one starts on the "1".
CHANGED: (b) replaced: the page had an empty cell clear the layer at once ("an empty cell has no BPM mode, so it does not wait") and said it would not copy Resolume; his words copy Resolume. The layer's X, which (b) treated together with the empty cell, is not named by him. "a layer that is not in BPM mode" is read as "a layer whose playing clip is not in BPM mode" (INFERRED; the other reading is assumption A-4). (a), (c), (d) stand.
TODAY: A click on an empty cell clears the layer at once (slice source: facts-app-cue-today.md section 2, not re-read by me). No BPM-mode wait exists (facts-takes-bpm-fire.md section 3 line 40).
@@END

@@ITEM R139
TITLE: Firing while the beat is stopped, paused or running
STATUS: CORRECTED parts (c) and (d)
HIS: L12, L13, L17, L31
RULE: STOPPED: his click on play, or on any clip, is the new "1" (L12). The beat starts on that press at the tempo number shown and the clip plays from its beginning at once; any clip does this, in BPM mode or not, fired by a click, a key, a MIDI pad or a column trigger. A click on an empty cell starts nothing. Resync starts a stopped beat too and is the "1". PAUSED OR RUNNING: a newly fired BPM-mode clip waits for the "1" and starts there; if Resync is clicked while it waits, that press is the "1" and the clip starts on it (L12: "in pause or run mode, unless resync is clicked, when a new clip with BPM mode is triggered, it waits for the one."). PAUSED, in addition: the fire sets the beat clock going again from the position it was holding at, exactly as play does; the "1" the clip waits for is the next "1" of that resumed clock, not his press (L13, L31; assumption A-1). A clip that was playing when he paused stays displayed on its frame and plays on from there when the clock runs again (L31). Pause and play never move a clip back to the "1" (L13).
CHANGED: (a) and (b), the stopped half, stand and are confirmed by L12 and L31. (c) "PAUSED: a video or picture sequence you fire shows and does not play" and (d) "it stands on its first frame; when you press play the beat runs on from where it was held" are replaced: a fired BPM-mode clip waits for the "1" without showing (item 173) and the fire itself resumes the clock. The last clause of (d), "when the beat was held at its start, your play press is that 1", is removed: no paused start exists (L30). Added: "unless resync is clicked" (L12). His earlier "When paused, it will not play but will still display." (binding-decisions.md:1091) now holds for the clip that was playing when he paused (L31), not for a clip fired during the pause.
TODAY: Nothing of this exists: no stopped or paused beat (facts-takes-bpm-fire.md S4). The adopted, unbuilt tempo-row rule "only play or a hand Resync starts a stopped beat" with its tests is void (facts-takes-bpm-fire.md 5.8; area-tempo.md O-03, O-04).
@@END

@@ITEM R140
TITLE: How his show-start sentence is read
STATUS: REPLACED by his answer to question 172
HIS: L30
RULE: "press start" is the play button of the tempo bar. "load up the clips" is filling the clip grid; it is not firing clips so that they stand ready, and no such standing-ready state is built. Clips start together by the column trigger and by nothing else (L30: "The only way to do this is with a column trigger."). "cue them ahead of time" is looking at them in the preview monitor first (topic B).
CHANGED: The page's "load up the clips is firing them while the beat does not run, so that they stand ready" is replaced. The page's readings of "press start", of the column and of "cue them ahead of time" stand.
TODAY: not checked
@@END

@@ITEM R142
TITLE: A clip's own pause is saved with the show
STATUS: STANDS
HIS: none
RULE: A clip he paused himself is still paused, on the same frame, after the show is saved, the app quit and the show opened again. This is the clip's own pause; it has nothing to do with the tempo bar's pause, which is never saved (item R163).
CHANGED: nothing
TODAY: A clip's own pause is not saved: after re-opening a show the clip plays when fired (slice text of R142; binding-decisions.md:1094, :944).
@@END

@@ITEM R163
TITLE: The beat when the app or a show opens
STATUS: STANDS
HIS: L12
RULE: When the app starts, the beat is stopped and waits for him: until his first fire, play or Resync the circle stands still and everything that follows the beat stands on the "1". That first press is the new "1" (L12). Opening a show never changes the beat: stopped stays stopped, running keeps running, and the tempo number stays as it is. A show holds no tempo number and nothing of the tempo bar's play, pause or stop; of the tempo bar it holds only the nudge amount (which exists in automatic mode: assumption A-9). The action buttons open as saved, and no action plays until the beat runs.
CHANGED: nothing
TODAY: The app opens with its listening searching and no stopped state (area-tempo.md part 1 point 4); the show stores a quantize mode and a tempo multiplier that nothing reads, and no nudge (area-tempo.md part 1 point 21).
@@END

@@ITEM R169
TITLE: The tempo bar with Ableton Link on
STATUS: STANDS -- Link itself is now on the plan (item R178)
HIS: L128
RULE: With Ableton Link switched on, the tempo bar's pause, Resync and play are greyed out, because the beat is shared with the other programs. A fire never starts or moves the beat: the beat is Link's and always runs. Stop still takes the clips off the layers and leaves the beat running. Link is off unless he switches it on.
CHANGED: nothing
TODAY: Link is compiled out of his build; with Link on the app follows Link's tempo only, never its beat position (area-tempo.md part 1 point 10; M-3).
@@END

@@ITEM R164
TITLE: The three DJ tracks and the "1" the app finds
STATUS: STANDS -- its last clause waits on question 189
HIS: L32
RULE: (a) The three DJ tracks he is bringing are for measuring how well the app finds the "1" in real music; one part of the BPM-mode work waits for them, and so does how far the "1" can be relied on while the app listens by itself. (b) In automatic mode the app finds the "1" itself and can be wrong. What his own Resync or tap does to that "1" while the app stays in automatic mode waits on Harmony's answer to his question back (L32).
CHANGED: The clause "when you press Resync the 1 is yours and stays (question 189 A)" no longer stands by itself: 189 is open (L32).
TODAY: While it listens the app counts the bars afresh after a drop or a breakdown, so its "1" is a guess (area-tempo.md part 1 point 26; M-7).
@@END

@@ITEM R174
TITLE: The edges: what moves the "1", pause, the nudge
STATUS: CORRECTED parts (b) and (d)
HIS: L13, L12
RULE: (a) A clip he paused himself does not wait for the "1": it shows at once on its paused frame. (b) While a BPM-mode clip waits: a Resync press is the "1" and the clip starts on it (L12); "/2", "x2" and any tempo change do NOT move the "1" it waits for, they only change how soon that "1" comes (L13: "/2 and x2, tempo change do not move the 1."); stop takes the waiting clip away with everything else; pause holds the wait, and the clip starts on the first "1" after the clock runs again. Pause pauses the beat clock, the BPM-mode clips and everything the beat clock controls (L13); play, or a fire (assumption A-1), sets it all going again from the position it was holding at; neither pause nor play ever moves a clip back to the "1" (L13: "Pause and play do not move the clip back to the one."). (c) Every BPM-mode clip waits, a clip set to Play Once too; a clip that must land on his press is not put in BPM mode. (d) A nudge moves the "1" a hair earlier or later and that move stays in the clock's position. A start from pause continues from the held position, so the earlier nudge is simply part of where the clock stands: nothing is undone and nothing is applied again (L13: "that nudge is history because the user started the clock from the position it was holding at"). A start from stop makes his press the "1". No start changes the nudge amount; only Resync sets it to 0 (assumption A-8).
CHANGED: (b): "'/2', 'x2', a tap or a typed tempo move the 1 it waits for" is replaced by "do not move the 1" (L13; the tap is item 190). Added to (b): pause and play never move a clip back to the "1"; what a pause holds. (d): his nudge paragraph is read as agreeing with the page ("leaves the nudge number as it is") and adds the reason; the other reading is in assumption A-8. (a) and (c) stand.
TODAY: Not built: no pause of the beat, no nudge on main (area-tempo.md part 1 points 3, 25).
@@END

@@ITEM R175
TITLE: Four earlier things that later words replaced
STATUS: STANDS -- points (1) and (2) are now confirmed by his words
HIS: L11, L12, L13
RULE: (1) Firing a BPM-mode clip between two "1"s: it waits and starts on the "1" (L12); it is not started at once and cut back later. A clip that is already playing and falls out of time plays on and is cut once on the next "1"; a Resync cuts it at once. A tempo change, "/2", "x2", pause and play do not put a BPM-mode clip out of time, because none of them moves the "1" (L13). (2) A stopped beat is started by play, by Resync, and by any clip he fires (L12). (3) The app opens with the beat stopped (item R163). (4) Nothing holds a fire to a line "in Quantize mode": Quantize is out of live use.
CHANGED: nothing. (1) gains one sentence from L13 (INFERRED: what no longer counts as falling out of time; how a BPM-mode clip is held to the beat is topic H).
TODAY: not checked
@@END

@@ITEM R176
TITLE: The tempo bar: stop, pause, steps, range, nudge, circle
STATUS: CORRECTED parts (d), (e), (g); (b) gains his words
HIS: L14, L13, L72
RULE: (a) Stop: one press; every clip leaves every layer, a layer set to Ignore Column Trigger too; the beat stops and the tempo number is not lost; stop also stops all actions and everything else (L72: "The tempo stop button stops all actions as well as everything else."). (b) Pause holds the beat clock, the BPM-mode clips and everything the beat clock controls (L13: "Pause pauses, the beat clock, the clips and everything that it controls with BPM."); a clip that is not in BPM mode plays on (assumption A-3). No button pauses every clip at once; each clip keeps its own pause. The tempo bar's play, pause and stop are the only transport buttons at the top. (c) "-" and "+" step the tempo by 1 BPM, in whole numbers. (d) The tempo runs from 22 to 480; by hand it always does (L14: "manually it will always go from 22 to 480"). "/2" and "x2" work all the way to both limits, in manual mode and on the listening clock (L14). The app's listening is given the same open range; if that distorts the listening, its range is narrowed later and the hand range is not (L14; assumption A-10). (e) The nudge: plus moves the beat earlier; one press is 1 ms, a held press repeats; pause never touches it. The text "nudge X ms" is shown only in automatic mode; in manual mode no nudge number is shown, the nudge simply moves the "1" to where he wants it (L14; assumption A-9). (f) A tap never starts the beat. (g) There are no words "Bar 1" to "Bar 4"; the circle alone shows the beat: one complete turn of the circle is one bar and each of its four spots is one beat (L14).
CHANGED: (d) replaced: the page said 30 to 400 with "/2" greyed below 60 and "x2" above 200. (e): added that "nudge X ms" shows only in automatic mode; the typed number and the limits -500 to +500 therefore belong to automatic mode (assumption A-9). (g): added what the circle is. (a): added that stop stops all actions (L72). (b): his L13 sentence added; whether "the clips" there means every clip is assumption A-3. (c) and (f) stand.
TODAY: Every tempo request is folded into 60 to 200 (area-tempo.md part 1 point 9); the five buttons "/4 /2 x1 x2 x4" change nothing (point 11); ">" and "||" pause every playing clip and "[]" stops routines only (point 2); the text "Bar N" is shown (point 12); no nudge exists on main (point 3).
@@END

@@ITEM R214
TITLE: What stands still while the beat does not run
STATUS: STANDS -- one clause waits on his question L107 (topic I)
HIS: L13, L72, L107
RULE: (a) Stop: the circle stands on the "1" and every clip leaves. Everything that follows the beat stands on the "1" until the beat runs again: signals that run on the beat, sliders driven by the beat, actions (stop stops them all, L72), the autopilot's count, MilkDrop's Jukebox count. (b) Pause: the same things stand where they were (L13: "Pause pauses, the beat clock, the clips and everything that it controls with BPM."); a clip that is not in BPM mode plays on (assumption A-3). (c) In both, what the music itself drives (loudness, bass, tones) keeps moving. (d) In automatic mode the tempo number keeps following the music while the beat is stopped or paused, so his first fire of the next song starts the beat at that song's tempo; in manual mode it holds what he set. The clause "effects that read the beat by themselves (a strobe, a pulse) hold still too" is not settled here: he asks whether it is a good idea (L107) and topic I answers him.
CHANGED: nothing replaced. The effects clause of (a) is taken out of "stands" until his question L107 is answered.
TODAY: No stop and no pause of the beat exist (area-tempo.md part 1 point 3); whether the listening keeps moving the number while the beat is held is not designed (area-tempo.md M-6).
@@END

@@ITEM R215
TITLE: The buttons "/2" and "x2" beside the tempo number
STATUS: REPLACED in one part: the limits (his L14); the rest stands
HIS: L14, L13
RULE: The tempo bar has two such buttons, "/2" and "x2". "/2" halves the tempo number and "x2" doubles it, once per press; neither moves the "1" (L13: "/2 and x2, tempo change do not move the 1."). Both work all the way to the limits 22 and 480, in manual mode and on the listening clock (L14: "The /2 and x2 manual work to both limit as well as the listening clock."). A press that would pass a limit does nothing (assumption A-11). Nothing of them is saved with a show. Whether a press keeps the app in automatic mode waits on question 189.
CHANGED: "'/2' is greyed below 60 and 'x2' above 200" is replaced by his limits 22 and 480. "neither moves the 1" is now his word (L13), no longer inferred consent. The rest stands.
TODAY: Five buttons "/4 /2 x1 x2 x4" write a saved number that nothing reads (area-tempo.md part 1 point 11; O-11).
@@END

@@ITEM R216
TITLE: Effects that hold or trail a picture after a stop
STATUS: STANDS
HIS: L72
RULE: When stop takes every clip off, an effect that holds or trails a picture goes on doing what it does: an Echo or feedback trail fades out by itself, and a frozen picture stays until he switches that effect off. Apart from that, with no clip playing the output is black. His L72 ("The tempo stop button stops all actions as well as everything else.") is read as being about what plays and runs on the beat, not about a held picture (INFERRED; his earlier "147 a", binding-decisions.md:1072, is the word on held pictures).
CHANGED: nothing
TODAY: With no clip playing the global effects do not run and the screen goes dark at once (slice text of R216: CompositorEngine.cpp:1064-1065, Renderer.cpp:709-712, not re-read by me).
@@END

@@ITEM R177
TITLE: The nudge and a screen's Delay
STATUS: STANDS -- his L14 adds where the nudge amount shows
HIS: L14, L13
RULE: Two controls bring picture and sound together. A screen's Delay (0 to 100 ms, one per output screen) makes everything on that screen later: it is for the room. The nudge moves where the "1" sits, earlier or later, for everything that runs on the beat: it is for a beat that sits a little off the music. In automatic mode the nudge is a standing amount, shown as "nudge X ms", kept by the show, and set to 0 by a Resync; so a nudge saved with a show lasts until his first Resync of the night. In manual mode no amount is shown and the nudge simply moves the "1" (L14; assumption A-9). A Delay can only make a screen later. What follows the loudness of the music cannot be moved earlier.
CHANGED: Added from L14: the amount is shown only in automatic mode. Nothing else.
TODAY: Neither the nudge nor the per-screen Delay is built on main (area-tempo.md part 1 points 3, 25; R-18).
@@END

@@ITEM R178
TITLE: Ableton Link: planned, no longer left as it is
STATUS: REPLACED by his answer to question 215
HIS: L128
RULE: Ableton Link is on the plan (L128: "215 plan all of these"; Link is thing (b) of that question). When it is built, item R169 says what the tempo bar does while Link is on. What "plan" covers and when it is built is topic K's (question 215); this topic adds nothing to it (assumption A-12).
CHANGED: "It stays that way and nothing new is built for it" is replaced: he asks for all six things of question 215 to be planned, Link among them.
TODAY: The Link switch is dimmed because his build is compiled without Link (area-tempo.md part 1 point 10).
@@END

@@ITEM R205
TITLE: A column click and the empty cells in it
STATUS: STANDS -- confirmed by his look in Resolume (L19); the moment follows L11
HIS: L19, L11
RULE: A click on a column trigger also clears every layer whose cell in that column is empty; only a layer set to Ignore Column Trigger is left alone. A click on one empty cell does the same for its layer. When the layer is cleared follows the empty-cell rule (item R168): on the next "1" if the clip playing on that layer is in BPM mode, at once if it is not (L11).
CHANGED: Nothing in what happens; his Resolume does the same (L19: "D layer goes empty when a column is triggered with an empty"). The moment of the clearing now follows L11 (INFERRED by L9), where the page had "at once".
TODAY: A column click clears a layer with an empty cell at once (slice text of R205: Layer.h:402-403, Composition.h:447-448, not re-read by me).
@@END

@@ITEM R206
TITLE: The eight looks in his Resolume Arena
STATUS: ANSWERED all eight; three bear on this topic
HIS: L15, L16, L17, L18, L19, L20, L21, L22, L29
RULE: The looks are evidence of what his Resolume does; none is a rule by itself. For this topic: (b) L17 "tempo paused or stopped, when you trigger a clip tempo play his activated": in Resolume a fire starts the tempo from pause and from stop alike. It supports "a fire while paused sets the clock going" (assumption A-1) and "a fire starts a stopped beat" (item R139). His own rule differs from Resolume only in what comes next, and he says so: paused, the BPM-mode clip waits for the "1"; stopped, the press is the new "1" (L12, L31). (c) L18 "the beat reacts instantly to the tapping on the second tap, and the clip responds to the beat changing": the tempo takes effect on the second tap and a playing clip follows it; whether the beat also jumps onto the tap is not said in so many words (item 190, assumption A-2). (d) L19 "layer goes empty when a column is triggered with an empty": confirms item R205. The other five go to their topics: (a) L16 to the cue system (topic B), (e) L20 and (f) L21 and (h) L29 to how a clip plays (topic H), (g) L22 to presets (topic C).
CHANGED: The reading was a request, so nothing in it is replaced; each look is now answered. "Lipp's" in L16 is read as "clip's" and "his activated" in L17 as "is activated" (INFERRED, typing slips).
TODAY: not checked
@@END

@@ITEM D1
TITLE: Only he starts a stopped beat
STATUS: OPEN the hand part is settled by L12; the autopilot and the review screen are still Harmony's
HIS: L12
RULE: A stopped beat is started by his own press only: play, Resync, or a clip he fires by mouse, key, MIDI pad or another controller (L12: "When stopped, user’s click on play or any clip is the new 1."). The autopilot never starts a stopped beat, and a recording played in the review screen never touches the live beat (assumption A-13).
CHANGED: The first sentence is now his word (L12 names "user’s click"). The second sentence is not covered by any word of his.
TODAY: Replayed and autopilot fires go through the same fire path as a hand fire (facts-takes-bpm-fire.md section 3 line 50, 5.10); the origin of a fire would have to be told apart.
@@END

@@ITEM D2
TITLE: The app remembers the last tempo between sessions
STATUS: SETTLED it is the default of question 175, which his first line takes
HIS: L1
RULE: The app keeps one number between sessions, the tempo he used last, and opens at it (120 the very first time). It is kept with the app, not in a show.
CHANGED: nothing
TODAY: Nothing stores a tempo (area-tempo.md part 1 point 21).
@@END

@@ITEM D3
TITLE: A long run of taps keeps following him
STATUS: OPEN no word of his covers the count of taps
HIS: L18
RULE: The tempo takes effect on the second tap (as he saw in his Resolume, L18). A long run of taps keeps following him: the tempo is the average of his last eight taps, moving on with every further tap; a gap of more than two seconds starts a new run (assumption A-14).
CHANGED: nothing replaced; L18 confirms only that the second tap already sets the tempo.
TODAY: The tempo stops changing after the eighth tap while every tap still pulls the beat (area-tempo.md M-1).
@@END

@@ITEM D4
TITLE: A press a hair after the "1" counts as on it
STATUS: OPEN no word of his covers it
HIS: none
RULE: A BPM-mode clip fired within a tenth of a beat after the "1" starts at once, as if fired on the "1"; it does not wait a whole bar (assumption A-15).
CHANGED: nothing
TODAY: Not built; the tenth of a beat comes from the unbuilt rule for a clip being in time (facts-takes-bpm-fire.md section 4 line 62).
@@END

@@ITEM D5
TITLE: Every bar is four beats
STATUS: SETTLED by his words on the circle
HIS: L14
RULE: The app counts every bar as four beats, everywhere: the "1" is the first of four (L14: "A complete circle is one bar, and each of the four spots on the circle are one beat.").
CHANGED: nothing
TODAY: The bar is fixed at four beats (area-tempo.md part 1 point 18).
@@END

@@ITEM D6
TITLE: The odd presses: pause while stopped, stop while paused
STATUS: OPEN its first clause is replaced by L30; the rest is still Harmony's
HIS: L30, L12
RULE: Pause pressed while the beat is stopped does nothing: there is nothing to hold and no paused start (L30). Play pressed while stopped starts the beat and is the new "1" (L12). Stop pressed while paused stops the beat and clears every layer, as any stop. Play pressed while the beat runs, and a second stop, change nothing (assumption A-16).
CHANGED: "Pause pressed on a stopped beat holds it at its start ...; play from there is beat 1" is replaced: that press existed only for the paused start of question 172 A, which he turned down (L30). The other presses stay as Harmony decided them.
TODAY: No ruling and no code defines these presses (area-tempo.md U-TECH-4).
@@END

@@ITEM P1
TITLE: The look of a cell that waits or is previewed
STATUS: DROPPED
HIS: L8
RULE: laid out by Harmony where it fits in the correct area; its look is settled in the UI redesign (L8). What stays as function: a waiting clip's cell shows that it waits (item 173), and a previewed clip's cell shows that it is the previewed one (topic B).
CHANGED: The three drawn variants are not put to him.
TODAY: Nothing on screen shows a waiting clip (facts-takes-bpm-fire.md S3).
@@END

@@ITEM P2
TITLE: The look and layout of the tempo bar
STATUS: DROPPED
HIS: L8, L14
RULE: laid out by Harmony where it fits in the correct area; its look is settled in the UI redesign (L8). What stays as function, from his words: the switch between automatic mode and manual mode is on the tempo bar; "nudge X ms" appears there only in automatic mode (L14); the circle shows stopped, paused and running; the tempo number can be typed.
CHANGED: The three drawn variants are not put to him.
TODAY: The top bar's order of controls is in area-tempo.md part 1 point 1; the adopted thirteen-cell layout is on paper only (area-tempo.md R-11).
@@END

## ASSUMPTIONS

@@ASSUME A-1
ABOUT: 174 R139 R174 191 R206
TEXT: I assume firing any clip while the beat is paused works like pressing play: the beat and every held clip run on from where they stood, and a BPM-mode clip you just fired starts on the next "1".
WHY: L13 and L17 say the fire starts the clock; L12 and L31 alone could also mean the clip waits inside a pause that stays.
ALT: b) The beat stays paused when you fire; the clip waits and starts on the first "1" after you press play.
IF-WRONG: STAGE a fire would un-pause everything on screen, or fail to
ASK: YES it is the one place where his four lines can be read two ways, and it decides what pause is for
@@END

@@ASSUME A-2
ABOUT: 190 R206 D3
TEXT: I assume tapping changes only the tempo number, from the second tap on. It never shifts the beat and never moves the "1"; you place the "1" with Resync.
WHY: The default rested on a guess about Resolume; his look L18 ("the beat reacts instantly to the tapping on the second tap") may say the beat jumps.
ALT: b) Each tap also pulls the beat onto your tap, but not the "1". c) The first tap of a run is also the "1".
IF-WRONG: STAGE after tapping, the beat sits off the music until a Resync, or jumps when he did not expect it
ASK: YES his own look in Resolume can be read against the default he accepted
@@END

@@ASSUME A-3
ABOUT: 174 R176 R214
TEXT: I assume pause holds only what follows the beat. A clip that is not in BPM mode keeps playing through a pause, and one you fire during a pause plays at once.
WHY: L13 says pause pauses "the clips and everything that it controls with BPM"; his earlier "136 b" and BD:918-920 say clips not on BPM are unaffected.
ALT: b) Pause holds every clip on every layer, in BPM mode or not.
IF-WRONG: STAGE a video would freeze, or keep moving, when he presses pause
ASK: LINE two earlier answers of his say it; only one phrase of L13 could be read the other way
@@END

@@ASSUME A-4
ABOUT: R168 R128 191
TEXT: I assume a clip that is not in BPM mode always starts the moment you fire it, also when it replaces a BPM-mode clip in the middle of a bar.
WHY: L11 says "a layer that is not in BPM mode ... plays instantly": the layer's playing clip, or the clip fired, is not said.
ALT: b) On a layer that is playing a BPM-mode clip every change waits for the "1", as the empty cell does.
IF-WRONG: STAGE a clip would cut in mid-bar, or wait when he wanted it now
ASK: LINE the page's text said the same and he did not correct it
@@END

@@ASSUME A-5
ABOUT: R168
TEXT: I assume the layer's X clears the layer at once, also in the middle of a bar, as in Resolume. Only the click on an empty cell waits for the "1".
WHY: L11 changes the empty cell to "the resolume way" and does not name the X; in Resolume the X is immediate (its team, 2018).
ALT: b) The X waits for the "1" too when the clip playing is in BPM mode.
IF-WRONG: SMALL one rule for one button
ASK: NO it follows from "the resolume way"
@@END

@@ASSUME A-6
ABOUT: R168 R139 R205
TEXT: I assume a click on an empty cell never starts or resumes the beat. While it waits for the "1" a newer press on that layer replaces it, and it replaces a clip that was waiting.
WHY: L11 gives the empty cell the same wait as a BPM-mode clip; L12 and L13 name only "play or any clip" as what starts the clock.
ALT: b) An empty cell clicked during a pause sets the clock going like a clip does.
IF-WRONG: SMALL a rare press
ASK: NO follows from his words on the wait and on what starts the clock
@@END

@@ASSUME A-7
ABOUT: R139 R174
TEXT: I assume Resync pressed while the beat is paused is the "1" and sets the beat going on that press; a BPM-mode clip that was waiting starts on it.
WHY: L12 says "in pause or run mode, unless resync is clicked"; his earlier 128 says Resync is the "1" and the beat begins on that push.
ALT: b) Resync during a pause places the "1" and the beat stays paused until play.
IF-WRONG: SMALL one press
ASK: NO his two sentences together say it
@@END

@@ASSUME A-8
ABOUT: R174 R177
TEXT: I assume starting the beat again, from pause or from stop, never changes the nudge amount. Only Resync sets it to 0. "The nudge is history" means its move is already part of where the beat stands.
WHY: L13 "that nudge is history" could also mean the amount is cleared at a start; his earlier 129 b says stop and play never touch it.
ALT: b) A start from pause or stop sets the nudge amount back to 0.
IF-WRONG: SMALL a few milliseconds and a number in the tempo bar
ASK: NO his earlier answer 129 b covers it and the difference is a few milliseconds
@@END

@@ASSUME A-9
ABOUT: R176 R177 R163
TEXT: I assume the nudge amount exists only in automatic mode: shown as "nudge X ms", typed, kept from -500 to +500, saved with the show. In manual mode a nudge just moves the "1", with no number and no limit.
WHY: L14 hides the number in manual mode; it does not say whether an amount is still counted, limited or saved there.
ALT: b) The amount is counted in both modes with the same limits and is only hidden in manual mode.
IF-WRONG: SMALL what the number reads after a switch between the modes
ASK: LINE a real choice about what a show saves
@@END

@@ASSUME A-10
ABOUT: R176 R215 189
TEXT: I assume the app by itself picks a tempo between 60 and 200, because by ear 70 and 140 are the same beat; your "/2" and "x2" then take that number as far as 22 and 480.
WHY: L14 opens the listening clock to 22 to 480 and expects it may distort; which range the listening guesses in by itself is not said.
ALT: b) The listening may land anywhere from 22 to 480 by itself (it will sometimes show half or double). c) Measure both on the DJ tracks first.
IF-WRONG: STAGE the number would jump between half and double tempo by itself
ASK: LINE he foresaw it in L14 and said it can be changed later
@@END

@@ASSUME A-11
ABOUT: R215 R176
TEXT: I assume "/2" does nothing below 44 and "x2" nothing above 240 (the result would pass 22 or 480); "-" and "+" stop at 22 and 480; a typed number outside is set to the nearest limit.
WHY: L14 says they "work to both limit" and not what a press at the limit does.
ALT: b) A press that would pass a limit lands on the limit itself (22 or 480).
IF-WRONG: SMALL edge of the range
ASK: NO technical edge he would not care about
@@END

@@ASSUME A-12
ABOUT: R178 R169
TEXT: I assume Ableton Link, now on the plan, behaves on the tempo bar as written: with Link on, play, pause and Resync are greyed out, a fire never moves the beat, and stop only clears the layers.
WHY: L128 says "215 plan all of these"; it does not say what Link does to the tempo bar.
ALT: b) With Link on, the tempo bar keeps working and only the tempo number is Link's.
IF-WRONG: SMALL Link is planned, not yet built; settled when its plan is shown
ASK: NO belongs to the Link plan of topic K
@@END

@@ASSUME A-13
ABOUT: D1
TEXT: I assume only your own press starts a stopped beat: play, Resync, or a clip fired by mouse, key or pad. The autopilot never starts it, and playing a recording in Review never touches the live beat.
WHY: L12 names "user’s click on play or any clip"; the autopilot and the review screen are not named anywhere (L130: not read).
ALT: b) Any fire starts a stopped beat, whoever made it.
IF-WRONG: STAGE the beat would start by itself
ASK: LINE decided without asking; he asked to be shown these (L130)
@@END

@@ASSUME A-14
ABOUT: D3 190
TEXT: I assume a long run of taps keeps following you: the tempo is the average of your last eight taps and moves with every further tap; a gap of more than two seconds starts a new run.
WHY: No word of his gives the count of taps; L18 only says the second tap already sets the tempo.
ALT: b) The tempo is fixed after the eighth tap until you pause and tap again.
IF-WRONG: SMALL a tap count
ASK: LINE decided without asking; he asked to be shown these (L130)
@@END

@@ASSUME A-15
ABOUT: D4
TEXT: I assume a BPM-mode clip fired a hair after the "1" (within a tenth of a beat) starts at once, as if fired on the "1", and does not wait a whole bar.
WHY: L12 says the clip "waits for the one" and nothing about a press that is a hair late.
ALT: b) Any press after the "1", however close, waits for the next "1".
IF-WRONG: STAGE a clip fired on the beat by hand would come in a bar late
ASK: LINE decided without asking; he asked to be shown these (L130)
@@END

@@ASSUME A-16
ABOUT: D6 172
TEXT: I assume pause pressed while the beat is stopped does nothing; stop pressed while paused stops and clears as any stop; play while the beat runs, and a second stop, change nothing.
WHY: L30 turns down the paused start, which was the only reason for a pause on a stopped beat; the other presses are in no word of his.
ALT: b) Pause on a stopped beat arms a held start, and the next play is the "1".
IF-WRONG: SMALL presses with no effect
ASK: LINE decided without asking; he asked to be shown these (L130)
@@END

@@ASSUME A-17
ABOUT: R128 173 R168
TEXT: I assume "the 1" is always the first beat of the next bar, the first of the circle's four spots, never the start of a longer group of bars.
WHY: L14 says a complete circle is one bar and each spot one beat; L11 and L12 say "the 1" without naming the bar.
ALT: b) A clip waits for the first bar of a four-bar group.
IF-WRONG: STAGE a clip would wait up to four bars instead of one
ASK: NO his circle words (L14) and "next 1 ... in the middle of a bar" (BD:1081) say it
@@END

## QUESTIONS BACK
None answered in this paper. His question back on 189 (L32) is answered by a separate architect (answer-tempo-auto.md); item 189 is filed OPEN.

## NAMES

@@NAME the 1
MEANS: The first beat of a bar, the first of the circle's four spots; what a BPM-mode clip and an empty cell wait for.
SOURCE: his words L11, L12, L13, L31 ("the 1", "the one")
@@END

@@NAME BPM mode
MEANS: The way of playing a clip in which it follows the tempo and, when fired, starts on the "1".
SOURCE: his words L11, L12, L21; the on-screen name now is "BPM Sync", which is replaced by "BPM mode"
@@END

@@NAME timeline mode
MEANS: The way of playing a clip in which it runs at its own speed and starts the moment it is fired.
SOURCE: his words L21 ("timeline mode"); what "Timeline" names elsewhere is his L112 (topic I)
@@END

@@NAME tempo bar
MEANS: The row at the top with the circle, play, pause, stop, the tempo number, "-", "+", the two nudge buttons, "/2", "x2", tap and resync.
SOURCE: his words L13 ("the tempo bar"); Harmony's pages said "tempo row", which is replaced by his word
@@END

@@NAME tempo play, tempo pause, tempo stop
MEANS: The three transport buttons of the tempo bar, which run the beat clock and not a single clip.
SOURCE: his words L31 ("tempo play"), L17 ("tempo play"), L72 ("The tempo stop button"); "tempo pause" is Harmony's pick by the same pattern
@@END

@@NAME beat clock
MEANS: The running count of beats and bars that everything in BPM mode follows; it can run, be paused or be stopped.
SOURCE: his words L13 ("the beat clock"), L31 ("the bpm clock")
@@END

@@NAME automatic mode
MEANS: The app listens to the music and sets the tempo, and the "1", by itself.
SOURCE: his words L14 ("automatic mode"), L32; the on-screen control now is a tick box "Manual" (off), which is replaced by the pair automatic mode / manual mode
@@END

@@NAME manual mode
MEANS: The tempo and the "1" are what he set by hand; the app does not correct them.
SOURCE: his words L14 ("manual mode"), L32
@@END

@@NAME listening clock
MEANS: The beat clock while the app is in automatic mode, driven by what it hears.
SOURCE: his words L14 ("the listening clock")
@@END

@@NAME nudge X ms
MEANS: The text that shows, in automatic mode only, how far the "1" has been nudged.
SOURCE: binding-decisions.md:911 (his "61 b but lets call it "nudge X ms""); L14 for where it shows
@@END

@@NAME resync
MEANS: The button whose press is the "1": it places the first beat of the bar and starts a stopped beat.
SOURCE: his words L12 ("resync"); binding-decisions.md:957
@@END

@@NAME tap tempo
MEANS: Tapping in time to set the tempo number; the spacebar is its usual key.
SOURCE: his words L118 ("Spacebar is typically tap tempo"), L18
@@END

@@NAME the circle
MEANS: The beat display of the tempo bar: one complete turn is one bar and each of its four spots is one beat.
SOURCE: his words L14 ("A complete circle is one bar, and each of the four spots on the circle are one beat.")
@@END

@@NAME column trigger
MEANS: The control at the top of a column that fires every cell of that column at once.
SOURCE: his words L30 ("the column trigger")
@@END

@@NAME empty cell
MEANS: A cell of the clip grid that holds no clip; clicking it clears its layer.
SOURCE: his words L11 ("the empty cell")
@@END

## CONFLICTS
- A clip fired while PAUSED. Earlier: "R111 if I fire a clip while the beat is stopped, it starts playing the beat. When paused, it will not play but will still display." (binding-decisions.md:1091) against L31 "I click on a BPM clip and it waits for the bpm clock to return to the 1 and then fires." and L13 "that press starts the clock from when the user clicks play or the clip". L31 opens with "let me clarify this", so the later word is taken; the earlier sentence is kept only for a clip already playing when he pauses. Assumption A-1 (ASK YES).
- What a tap moves. He takes the default of 190 by L1 (tap = the tempo only, as Harmony guessed Resolume does) against his own look L18 "the beat reacts instantly to the tapping on the second tap, and the clip responds to the beat changing". Assumption A-2 (ASK YES).
- The show start. Earlier: "The way to set up / cue up for a show start is to load up the clips and press start, or click on a column to trigger all of them at the same time" (binding-decisions.md:1092) against L30 "The only way to do this is with a column trigger." Not asked: L30 is the later word, says "only", and gives its reason; no assumption hangs on it beyond A-16.

## NOT DONE / UNSURE
- Question 189 is open by his own question back (L32). Until it is answered these stay undecided: whether a tap, a Resync, "-", "+", "/2", "x2" or a typed number keeps the app in automatic mode; whether the "1" he sets from a stopped beat (L12) is held or moved by the app's listening afterwards; what the nudge amount means across a change of mode (A-9). Cheapest: the answer paper answer-tempo-auto.md and his reply to it.
- If he answers A-1 with b (the beat stays paused when a clip is fired), the original question 174 comes back unanswered for a clip that is not in BPM mode fired inside a held pause, and item 173 needs a line on what a waiting clip shows during a long pause. Cheapest: one follow-up line only in that case.
- L13 "Pause pauses, the beat clock, the clips and everything that it controls with BPM." is read as "the clips that the beat clock controls". Filed as a line he can strike (A-3) and not as a conflict, because two earlier answers of his say the same as the reading.
- "that nudge is history" (L13): I read it as a reason, not a new rule (A-8). He wrote "If you don’t understand ask me." The difference is a few milliseconds, so it is not asked; if Harmony wants it certain, one line.
- The effects that read the beat by themselves (part of R214 a) wait on his question L107, which topic I answers.
- How a BPM-mode clip is held to the beat clock so that a tempo change never puts it out of time (R175 point 1, my sentence from L13) is topic H's to write; the unbuilt transport lane's "cut once on the next 1" and the quantize-out lane both need an architect delta against L11 to L13 before any packet (facts-takes-bpm-fire.md 5.3, 5.4, section 6).
- TODAY lines marked "slice text ..., not re-read by me" rest on the slice's own source notes; I opened no source file (the job is his words).

Written (system clock): Wed Oct  7 22:44:16 EDT 2026
