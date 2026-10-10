# SPEC A -- Triggering clips and the tempo bar (s-rta-1009): the s-rta-1007 spec with Boris's answers to page 2 laid over it by wf/merge3.py (paper apply-A.md, ruling rule-A.md). This file wins over every older one.

## PAGE 2 ITEMS (the rule now for each item of page 2 in this topic)
@@ITEM 217
TITLE: A hand tempo in automatic mode holds until the music changes
STATUS: ACCEPTED as written; tested against all 33 boxes, BF242 fits it
HIS: box left empty = accepted as written; BF242 "we will be fine tuning this beat detection till it's perfect"
RULE: In automatic mode a tempo he corrects by hand -- by tap, by typing, by "-" or "+", by "/2" or "x2" -- holds, and the app stays in automatic mode: no press of his changes the mode. His number becomes the number the listening works around: the app keeps listening, accepts only tempos close to his number, follows only small drifts of the music, and uses what it hears to keep the beats on the music. The number stays that close to his until the music clearly changes tempo (a new track that is not beat-matched); then the app lets go of his number by itself and follows the music again, and it goes on following the music until he corrects it again. It does NOT stay at exactly his number whatever the music does (way b, not taken). Manual mode is the way to hold a number exactly: there the tempo is what he set and the app corrects nothing. How close "close" is, how small "small" is and how long "clearly" takes are values found by measurement on the audio he brings (BF272), and the beat detection is tuned on that audio until it holds (BF242); nothing of this is promised before it is measured, and if the measurement fails he is told before anything is built. Everything else with @@ITEM 189 as it stands after the amendments of this paper.
CHANGED: nothing: accepted as written. Against the old blocks: item 189 called this rule "RECOMMENDED, NOT YET HIS WORD"; it is now accepted by him (amended below). Assumption A-18, tempo half, and assumption X-4 of the old spec-X are closed by it.
TODAY: With Manual off a heard tempo more than 2.0 BPM away for about 2.1 s replaces the held one, and a hand tempo does not clear the history of heard tempos (old item 189 TODAY: answer-tempo-auto.md F3, F4, F5; not re-read by me). The listening library has no call for "the tempo is near N" (F13): the hold is to be built in the app's own stage after it. Unmeasured: whether heard beats sit on the music's beats while the number is off (U1).
@@END

@@ITEM 218
TITLE: The app always tries to find the 1; Resync corrects it
STATUS: ANSWERED way b, with words of his own that go further than the letter
HIS: BF246 "b. app should always try to find the 1 and I will correct if necessary"; BF242 "we should be able to resync the 1 if the detection is finidng the correct bpm but not the correct 1"
RULE: In automatic mode the app always tries to find the "1" of the music by itself (BF246: "always"): it listens for it the whole time, also while the beat is stopped or paused (with @@ITEM R214 part (d) as it stands), and while the beat runs it places the "1" there whenever it is sure. While it is not sure, the "1" stays where it is and the app counts on from it, four beats to the bar. He corrects it when necessary (BF246; BF242 names the case: a detection that finds the correct BPM but not the correct "1"): a press on Resync is the "1" at once, in automatic mode too, and the mode stays automatic. THE PRESS THAT STARTS THE BEAT -- tempo play, or a clip he triggers, on a stopped beat -- is the "1" at that moment (with @@ITEM R139 as it stands), but it does not hold against the app: he chose way b on an item that named "the press that starts the beat", and way b lets the app place the "1" by itself "until your first Resync". So after a start press the app places the "1" as soon as it is sure, and that can be at once, because it has been listening during the stop. After a pause it is the same: tempo play, or a triggered clip, lets the beat clock run on from where it was held, and the app places the "1" as soon as it is sure. A Resync that starts a stopped or a paused beat is a Resync like any other and holds: it is his way to start with a "1" that stays. AFTER A RESYNC his "1" holds: the app counts on from it, keeps each beat on the music, and does not change which beat is the "1". How long it holds is not in his words -- the letter says "until your first Resync", his own words say "always" -- and is written here in Harmony's middle reading until he answers (assumption A3-1): it holds until the music has clearly changed (a new track that is not beat-matched: the same moment at which the app lets go of a tempo he set, item 217), until he pauses or stops the beat, or until his next Resync; after that the app places the "1" by itself again when it is sure. WHEN THE APP MOVES THE "1" BY ITSELF everything that follows the beat clock follows the new "1": the circle and the count go to it, a BPM-mode clip that waits starts on it, and the tempo number and the nudge amount do not change. A BPM-mode clip that is playing is not cut at that moment: it plays on and falls into step on the next "1", with @@ITEM R175 point (1) as it stands; this holds also when the app moves the "1" shortly after a pause has ended, which is a different thing from the play press itself: that press never moves a clip back to the "1" (BF167: "Pause and play do not move the clip back to the one."). Everything from "WHEN THE APP MOVES" to here is Harmony's, not his words (assumption A3-2); how actions and effects take a "1" that moved is their own topics'. In manual mode the app never places and never moves the "1": it is the press that started the beat, or his Resync, moved only by his nudge. How sure "sure" must be is found by measurement on the audio he brings (BF272) and tuned with the beat detection (BF242: "we will be fine tuning this beat detection till it's perfect"); nothing is promised before that, and if the app's "1" proves wrong more often than right on his music he is told before anything is built.
CHANGED: The item as he read it (the "1" is always his; the app never moves it) is replaced by way b plus his own words. HARMONY'S, and named so in the RULE: how long a Resync holds (the middle reading between the letter's "until your first Resync" and his "always"), with the pause and the stop that end it (assumption A3-1); what follows a "1" the app moved (assumption A3-2); that the app may place the "1" at once after a start press. NOT Harmony's: that a start press does not hold against the app -- it is the letter he chose, on an item that named the press. In automatic mode it narrows BF166 "When stopped, user’s click on play or any clip is the new 1." and BF171 "I would be setting a new 1 for the tempo" to "the 1 at that moment"; that is said to him once more (the text of assumption A3-1; FOR THE PAGE RULING). Against the old blocks: item 189's sentence that the listening "never moves which beat is the 1" and item R164 (b) no longer hold (amended); assumption A-18, the half about the "1", is closed with its way c taken. BF242 "finidng" is read as "finding" (INFERRED, a typing slip).
TODAY: While it listens the app counts the bars afresh after a drop or a breakdown, so its "1" is a guess (old item R164 TODAY: area-tempo.md part 1 point 26, M-7). The beat-in-bar count advances per HEARD beat, so a missed or an extra heard beat moves the "1" (old item 189 TODAY: F8; not re-read by me). To build: a "1" the app places only when it is sure, a Resync that holds against the listening, and the event "the music has clearly changed" shared with item 217. How often the app would find the "1" right is unmeasured (U2).
@@END

@@ITEM 219
TITLE: A start after pause or stop sets the nudge amount to 0
STATUS: ACCEPTED as written; no box of his says otherwise
HIS: box left empty = accepted as written
RULE: Whenever he starts the beat again after a tempo pause or a tempo stop -- by tempo play, by a clip he triggers, or by Resync -- the nudge is history: the nudge amount shown in automatic mode as "nudge X ms" goes back to 0 on that press, and from then on no shift is added to where the app hears the beats (a nudge of 0 shifts nothing; the way he did not take is the one in which it "keeps shifting the beat"). If the beat then sits off the music, he nudges again. Nothing jumps at the press itself: from a pause, tempo play or a triggered clip lets the beat clock run on from the position it was holding at (BF167: "that nudge is history because the user started the clock from the position it was holding at"), and in automatic mode the listening then puts the beats where it hears them; from a stop his press is the new "1"; a Resync is the "1" on its press, from a pause too. The nudge amount therefore goes to 0 in three cases: a Resync, a start after a pause, a start after a stop. Pausing or stopping by itself does not change the number: it goes to 0 at the start that follows. The nudge amount does NOT stay through pause, stop and play (way b, not taken). In manual mode there is no amount to set back: a nudge made there has moved the "1" and stays in the clock's position (with @@ITEM R174 part (d) and @@ITEM R177 as they stand after the amendments of this round). What a show keeps of the nudge amount is assumption A3-4.
CHANGED: nothing in the item: accepted as written. Against the old blocks: item R174 (d) and assumption A-8 said the opposite ("No start changes the nudge amount; only Resync sets it to 0"); they are replaced (amended), and with them the end of item R177. INFERRED: that the shift goes with the number ("no shift is added") -- read from "the nudge is history", from the 0 itself, and from way b, which he did not take ("keeps shifting the beat"). THE PRICE, which his page did not say: a beat that sits off the music by a fixed amount on his rig has to be nudged again after every pause and every stop, and a nudge saved with a show is gone at his first press (assumption A3-4). His earlier "129 b" is reversed: see CONFLICTS of the paper and FOR THE PAGE RULING.
TODAY: No pause or stop of the beat and no nudge exist on main (old item R174 TODAY: area-tempo.md part 1 points 3, 25). Touches the unmerged nudge lane, whose adopted rule kept the amount across stop and play (binding-decisions.md:959): an architect delta is owed there before any packet.
@@END

@@ITEM 248
TITLE: A click on an empty cell never starts the beat
STATUS: ACCEPTED as written; BF255 fits it
HIS: box left empty = accepted as written; BF255 "you can select the cell which triggers it and selects it"
RULE: A click on an empty cell never starts a stopped beat and never ends a pause; the same holds for an empty cell triggered by a key or a pad and for the empty cells inside a column trigger (the clips of that column do start or resume the beat, as any triggered clip). While the beat is paused, a layer that holds a BPM-mode clip keeps that clip, held on its frame, and is cleared on the first "1" after the beat runs again; a layer whose clip is not in BPM mode is cleared at once. Everything else with @@ITEM R168 part (b) as it stands. Selecting an empty cell without triggering it (a click where a clip's name would be) is topic F's and triggers nothing (BF255).
CHANGED: nothing: accepted as written. Assumption A-6 is closed by it; item R168 already says the same.
TODAY: A click on an empty cell clears the layer at once; no BPM-mode wait and no paused or stopped beat exist (old item R168 TODAY, not re-read by me).
@@END

@@ITEM 249
TITLE: A BPM-mode clip triggered a hair after the 1 starts at once
STATUS: ACCEPTED as written; no box of his says otherwise
HIS: box left empty = accepted as written; BF165 "same as a new clip in bpm mode"
RULE: A BPM-mode clip triggered within a tenth of a beat after the "1" starts at once and in step with the music: it does not wait a whole bar for the next "1", and it joins at the place it would have reached had it started on the "1", so that it sits on the beat. A press before the "1" needs no rule of its own: the clip waits that moment and starts on the "1". A press later than a tenth of a beat after the "1" waits for the next "1". The same tenth of a beat holds for a triggered empty cell that would wait for the "1" (BF165: the empty cell triggers "same as a new clip in bpm mode"): within it the layer is cleared at once, so that a column trigger pressed a hair after the "1" brings its clips in and empties its layers together. Everything else with @@ITEM D4, @@ITEM R128 and @@ITEM R168 part (b) as they stand.
CHANGED: nothing in the item: accepted as written. Assumption A-15 is closed by it; item D4 already says the same, and its status is no longer open. Added by this ruling, INFERRED from BF165 by his rule on repeats (BF164: "If I have explained something, use it to answer questions not answered."): the tenth of a beat also for an empty cell that waits for the "1".
TODAY: Not built; the tenth of a beat comes from the unbuilt rule for a clip being in time (old item D4 TODAY: facts-takes-bpm-fire.md section 4 line 62).
@@END

@@ITEM 250
TITLE: Tapping: the tempo number only, or the beats too
STATUS: OPEN he asks back what "shifts the beat" meant (BF266)
HIS: BF266 "What does shifting the beep mean to you?"
RULE: Waits on his answer: the reading put to him again is that tapping sets only how fast the beat runs -- the tempo number, from the second tap on -- and does not slide the beats earlier or later onto his taps, and does not change which beat is the "1"; where the beats fall is set by Resync and the nudge, and in automatic mode by the app's listening (items 217, 218). The other way: from the second tap on each tap also slides the beats onto his finger, without changing which beat is the "1". Settled already and not waiting: a tap starts nothing, neither a stopped beat nor a paused one; a tapped number is held to 22 to 480; a tapped number in automatic mode holds as item 217 says; a tap never makes another beat the "1" (both ways agree on that). Everything else with @@ITEM 190 and @@ITEM D3 as they stand.
CHANGED: No letter and no rule from him: he asks what the words meant. "the beep" is read as "the beat" (INFERRED, a slip of dictation). The item goes to him again in plain words (assumption A-2, re-written) after the answer he is owed (answer 250).
TODAY: From the second tap on every tap sets the tempo AND pulls the beat onto the tap, though not the "1" (old item 190 TODAY: area-tempo.md part 1 point 7, M-2); under the reading above the pull is removed.
@@END

## ITEMS (the items of the first page; a RULE that page 2 changed carries "[page 2, ...]" marks where it changed)
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
RULE: While a BPM-mode clip waits for the "1", the clip that was playing on that layer plays on; on the "1" the new clip takes over and starts from its beginning. On an empty layer nothing shows until the "1". His L31 fits this and says nothing against it (L31: "There is nothing in the layer. The tempo is paused. I click on a BPM clip and it waits for the bpm clock to return to the 1 and then fires."): the clip waits and then fires; he does not say that it stands on its first frame meanwhile. The waiting clip's cell shows that it is waiting; how that mark looks is laid out by Harmony (item P1). The frozen-first-frame variant (option B) is not built.
CHANGED: nothing
TODAY: A fire can wait only through the top bar's Quantize box or the clip's Snap box; while it waits the old clip plays on and nothing on screen shows the waiting clip (facts-takes-bpm-fire.md section 3 line 46, S3). BPM mode by itself makes nothing wait (section 3 line 40). To build: the wait that BPM mode implies, and the waiting mark.
@@END

@@ITEM 174
TITLE: A clip fired while the beat is paused
STATUS: ANSWERED in words; they describe a BPM-mode clip; the question's own case (a clip not in BPM mode) follows from L11, L13 and his earlier 136 b (assumptions A-1, A-3)
HIS: L31, L12, L13, L17, L11
RULE: BEAT PAUSED, he fires a BPM-mode clip: the fire sets the beat clock going again from the position it was holding at, as a press on play does (L13: "that press starts the clock from when the user clicks play or the clip"; L13: "the user started the clock from the position it was holding at"; his Resolume does the same, L17), and the clip waits for the next "1" of that clock and starts there, from its beginning (L31: "I click on a BPM clip and it waits for the bpm clock to return to the 1 and then fires."; L12: "in pause or run mode, unless resync is clicked, when a new clip with BPM mode is triggered, it waits for the one."). Until that "1" the layer shows what item 173 says: the clip that was held there plays on from where it stood, or nothing on an empty layer. BEAT STOPPED: the clip plays the moment he triggers it and that press is the new "1" (L31: "The only difference is if I was stopped, it would play the moment I triggered it and I would be setting a new 1 for the tempo."). A CLIP ALREADY PLAYING WHEN HE PAUSES THE TEMPO stays on screen, held on its frame; when he presses tempo play it plays on from where it was paused and the beat clock continues from where it was paused (L31). A CLIP THAT IS NOT IN BPM MODE, fired while paused (the case the question asked about): it plays at once on his press (item R128 (a); L11 fits it) and, being a fire, it sets the beat clock going like any fire (assumption A-1); a clip not in BPM mode that was already playing plays on through a pause (assumption A-3).
CHANGED: Option A (the default: no fired clip moves while the beat is paused, whatever its mode; it stands on its first frame until play) is replaced by his words. Option B is not chosen by letter; its content for a clip that is not in BPM mode is what assumption A-3 keeps. His earlier "When paused, it will not play but will still display." (binding-decisions.md:1091) is clarified by his L31, which opens "let me clarify this": it holds for the clip that was playing when he paused, not for a clip fired during the pause.
TODAY: No paused beat exists (facts-takes-bpm-fire.md S4). The adopted but unbuilt tempo-row text has a fired BPM clip appear at once on its first frame and wait for play (facts-takes-bpm-fire.md section 4 line 60, 5.5, 5.9): that text is void now.
@@END

@@ITEM 175
TITLE: The tempo of the very first fire
STATUS: DEFAULT A (his L1); nothing in his message says otherwise
HIS: L1, L12
RULE: The tempo number is never empty. The app opens at the tempo he used last, and at 120 the very first time. His first fire, or play, starts the beat at the number shown and is the "1" (L12: "When stopped, user’s click on play or any clip is the new 1."); the music, a tap or a typed number then corrects the number. The last-used tempo is one number kept with the app, not with a show (item D2). The app opens in automatic mode (assumption A-20), so the number follows the music as soon as the app has found its tempo, also while the beat is still stopped (item R214 d); how a number he sets by hand then holds against the listening is item 189.
CHANGED: nothing in the letter. Added: which mode the app opens in (Harmony's, assumption A-20) and the pointer to item 189.
TODAY: At launch the app has no tempo until its listening locks, and the beat does not advance meanwhile (area-tempo.md part 1 point 4); nothing stores a tempo, neither the show nor the app's settings (area-tempo.md part 1 point 21). To build: the remembered number and a beat that can start at it.
@@END

@@ITEM 189
TITLE: His hand against the app's listening
STATUS: OPEN he asked back (L32); Harmony's answer goes to him (answer 189); the rule below is a recommendation until he replies and until it is measured -- AMENDED after page 2 (189 1 by A, 189 2 by A, 189 3 by A, 189 4 by A)
HIS: L32, L14, L13, L12
RULE: ACCEPTED BY HIM in round 2 (item 217 as written; item 218 by way b and his own words; BF242); nothing in it is promised before it is measured on the audio he brings (BF272), and the beat detection is tuned on that audio until it holds. [page 2, A: item 217 accepted as written; BF246 "app should always try to find the 1"; BF242 "we will be fine tuning this beat detection till it's perfect"] In automatic mode a correction made by hand holds and the app stays in automatic mode. No press of his changes the mode: only the switch between automatic mode and manual mode does. TEMPO: a number he sets in automatic mode -- by tap, by typing, by "-" or "+" -- becomes the number the listening works around. The app keeps listening; it accepts only tempos close to his number, follows only small drifts of the music's tempo [page 2, A: item 217 accepted as written], and uses what it hears to keep the beats on the music. "/2" and "x2" pressed in automatic mode halve or double the running tempo, and the listening carries on at half or at double (L14: "The /2 and x2 manual work to both limit as well as the listening clock."). When the music clearly moves to another tempo (a new track that is not beat-matched), the app lets go of his number by itself and follows the music again (assumption A-18). THE "1": a "1" he sets -- by Resync, or by the press that starts a stopped beat (L12: "When stopped, user’s click on play or any clip is the new 1.") -- is the "1" in automatic mode too. Beside that, the app always tries to find the "1" of the music by itself and places it whenever it is sure; he corrects it with Resync when necessary. After a Resync his "1" holds: the app counts on from it and keeps each beat on the music. By Harmony's middle reading, which waits on his answer (assumption A3-1), the app places the "1" by itself again only when the music has clearly changed, or after he has paused or stopped the beat. A press that starts a stopped beat is the "1" at that moment but does not hold against the app as a Resync does. The whole rule is item 218. [page 2, A: BF246 "b. app should always try to find the 1 and I will correct if necessary"] The nudge moves the "1" a hair and shows as "nudge X ms" (L14). No tempo change, "/2" or "x2" moves the "1" (L13: "/2 and x2, tempo change do not move the 1."). MANUAL MODE stays his own switch: there the tempo and the "1" are exactly what he set and the app corrects nothing. How close "close" is, how small "small" is [page 2, A: item 217 accepted as written] and how long "clearly" takes are values found by the measurement, not chosen on paper. IF THE MEASUREMENT FAILS (the beats the listening hears do not sit on the music's beats while its number is off), the answer to him changes to "a correction needs manual mode", and he is told before anything is built.
CHANGED: Default A (any press of his switches Manual on and holds) is not taken: he asked back (L32). Option B (the app takes his number back within about two seconds) is not offered. Option C (the tempo steps switch Manual on, Tap and Resync do not) falls away with A. The reading that a tempo step switches Manual on was never confirmed by him (binding-decisions.md:966, his "R76 please explain wha tit means to switch manual on") and is replaced by this recommendation. One departure from a reading that stood: the page told him that in automatic mode the app finds the "1" itself (reading R164 b, not named by him); the recommendation gives the "1" to him alone, because how well the app finds the "1" is not measured, and keeps the way of the page as way c of assumption A-18. Items 190, R164, R176, R214, R215, 175 and D3, which the paper left waiting on this question, now point here.
TODAY: Read by the answer paper and re-opened line by line by its re-check (answer-tempo-auto.md F1-F20; check-tempo-auto.md), not by me. With Manual off every heard beat the listener is sure of puts the beat edge back to 0 (F2), and a heard tempo more than 2.0 BPM away for about 2.1 s replaces the held one, so a correction of exactly two BPM sits on the edge of that band (F3, F4); a hand tempo does not clear the history of heard tempos (F5); Tap and Resync do not switch Manual on (F6); the typed field and the steps are shown only in Manual (re-check finding 3: TopBar.cpp:101-104); the beat-in-bar count advances per HEARD beat, so a missed or an extra heard beat moves the "1" (F8) -- that, not the relock of F9 / F10, is the real threat to a hand "1" (re-check finding 4); every tempo is folded into 60..200 (F12); the listening library has no call for "the tempo is near N" (F13), so the hold is built in the app's own stage after it. UNMEASURED: whether the heard beats sit on the music's beats while the number is off (U1) and how often a beat is missed or added (U2); the three tracks have not arrived (HANDOFF.md:62, :84). The mechanism stays open until then: a band around his number (the paper's T1), or his correction kept as an offset on what is heard, and the app's own onset detection in place of the library's beat if U1 fails (re-check finding 5). A consequence a builder must know: the beat is stopped at every launch (item R163), so the first "1" of a show is always his, and the listener's own guess of the "1" is not used unless he chooses way c of assumption A-18. Touches the unmerged nudge lane (U3: the offset and the pull must be one mechanism). Size by the answer paper: large as one piece (an estimate).
@@END

@@ITEM 190
TITLE: Tap: the tempo only, or the beat too
STATUS: DEFAULT A (his L1), but his own look in Resolume (L18) can be read against the default's reason: assumption A-2 -- AMENDED after page 2 (190 1 by A)
HIS: L1, L13, L14, L18
RULE: Tap sets the tempo number only, from the second tap on. It never makes another beat the "1". Whether a tap also slides the beats, the "1" with them, a little forward or backwards in time onto his taps waits on his answer (item 250, assumption A-2); until he answers the reading is that it does not: a tap sets the tempo number and nothing else. The "1" is placed with Resync, and in automatic mode also by the app itself when it is sure (item 218). [page 2, A: BF266 "What does shifting the beep mean to you? Does that mean moving the 1 forward or backwards in time?"; BF246 "app should always try to find the 1"] A tap starts nothing: not a stopped beat and not a paused one (binding-decisions.md:957, his "128 tapping tempo does not start anything but when click resync, that is the 1 and it begins on that button push"). A BPM-mode clip that is playing follows the new tempo at once. A tapped number is held to the range 22 to 480 like every tempo set by hand (L14). In automatic mode a tap sets a number that holds, and the mode stays automatic (item 189).
CHANGED: nothing in the letter. His L13 supports it (L13: "/2 and x2, tempo change do not move the 1."). His look (L18: "the beat reacts instantly to the tapping on the second tap, and the clip responds to the beat changing") is evidence that the default's reason ("as Resolume, my guess") may be wrong: it reads either as "the number changes on the second tap" (fits A) or as "the beat jumps to the tap" (option B). The last sentence of the RULE replaces the paper's "waits on question 189".
TODAY: From the second tap on every tap sets the tempo AND pulls the beat onto the tap, though not the "1" (area-tempo.md part 1 point 7, M-2); under A the pull is removed.
@@END

@@ITEM 191
TITLE: A column that holds both kinds of clips
STATUS: DEFAULT A (his L1); the empty cells of the column follow his L11 (by L9)
HIS: L1, L11, L12, L13, L19, L30
RULE: A click on a column trigger while the beat runs: each clip follows its own rule. A clip that is not in BPM mode starts on the click (the standing rule of item R128 (a); L11 fits it, read as in assumption A-4); a BPM-mode clip starts on the next "1" (L12). For up to a bar the column is only partly there. An empty cell of the column clears its layer by the empty-cell rule of item R168: on the "1" when the clip playing there is in BPM mode, at once otherwise (L11, applied by L9; assumption A-19); a layer set to Ignore Column Trigger is left alone. With the beat STOPPED the click is the new "1" and all of the column plays at once (item 172). With the beat PAUSED the click sets the clock going from where it was held and the same per-clip rule applies (L13; assumption A-1).
CHANGED: nothing in the letter. The empty cells of the column now follow L11 (INFERRED by his L9: explained once, used wherever it applies); the page had them clear at once.
TODAY: A column click fires every cell at once, and an empty cell clears its layer at once (facts-takes-bpm-fire.md 5.11; slice text of R205: Layer.h:402-403, Composition.h:447-448, not re-read by me).
@@END

@@ITEM R128
TITLE: A BPM-mode clip starts on the next "1"
STATUS: STANDS -- his L11 and L12 repeat its core
HIS: L11, L12, L14
RULE: (a) A BPM-mode clip fired while the beat runs or is paused waits and starts playing on the next "1", by itself, unless Resync is clicked first: then that press is the "1" and the clip starts on it (L12: "in pause or run mode, unless resync is clicked, when a new clip with BPM mode is triggered, it waits for the one."). The next "1" is the first beat of the next bar, the first of the circle's four spots, never the start of a longer group of bars (L14: "A complete circle is one bar, and each of the four spots on the circle are one beat."; his 154, binding-decisions.md:1081: "will start playing on the next 1 if it is triggered in the middle of a bar"). A clip that is not in BPM mode starts on his press: this is the reading's own sentence, shown to him and not corrected; his 154 ties the wait to a clip "that has BPM mode enabled", and the second sentence of L11 fits it (read as in assumption A-4). (b) There is no Snap box on a clip and no Quantize setting for firing clips live; the review screen has its own Quantize. (c) The wait is part of BPM mode itself, not a separate setting. (d) Firing the clip that is already playing: it plays on and starts again from its beginning on the next "1". (e) A column click starts its BPM-mode clips on the next "1" and the others on the click (item 191). With the beat stopped none of this applies: the press is the new "1" (item R139).
CHANGED: nothing. Added from L12: "unless resync is clicked", and that the wait holds in pause as well as in run. Added from L14 and his 154: which "1" is meant (the paper held it as assumption A-17; his words settle it).
TODAY: The Snap box on a clip and the top bar's Quantize box exist and are the only things that make a fire wait (facts-takes-bpm-fire.md section 3 lines 41-45; area-tempo.md part 1 points 13, 14). The unbuilt quantize-out lane deletes exactly the wait machinery a BPM-mode wait would need (facts-takes-bpm-fire.md section 6): an architect delta is owed there.
@@END

@@ITEM R168
TITLE: While a clip waits: newer fire, empty cell, the layer's X
STATUS: CORRECTED part (b)
HIS: L11, L9, L19
RULE: (a) While a BPM-mode clip waits for the "1", a newer fire on the same layer takes its place: the last press wins. (b) A click on an empty cell: if the clip playing on that layer is in BPM mode, the layer is cleared on the next "1", the same wait as a new BPM-mode clip; if the clip playing there is not in BPM mode, the layer is cleared at once (L11: "let's do the resolume way where the empty cell triggers on the 1 if the clip playing is in bpm mode, same as a new clip in bpm mode. If a layer that is not in BPM mode is triggered, then that plays instantly"). The same holds wherever an empty cell is triggered: by a click, by a key or a pad, and inside a column trigger (by his L9; assumption A-19). A clear that waits follows the last-press rule of (a) like any waiting fire: a newer press on that layer replaces it, and it replaces a clip that was waiting; on a layer where nothing plays it simply ends the wait of that clip, at once (assumption A-19). An empty cell never starts a stopped beat and never ends a pause; during a pause a layer that holds a BPM-mode clip is cleared on the first "1" after the beat runs again (assumption A-6). The layer's X clears the layer at once, in the middle of a bar too, and ends any wait on it (assumption A-5). (c) Pressing the waiting clip again changes nothing: it keeps waiting for the same "1". (d) The layer's fade from the old clip to the new one starts on the "1".
CHANGED: (b) replaced: the page had an empty cell clear the layer at once ("an empty cell has no BPM mode, so it does not wait") and said it would not copy Resolume; his words copy Resolume. L11 opens with the token "dfad": read as a typing slip, not as lettered parts of the reading (INFERRED); if it stood for part (d), the fade, his sentence still speaks only of the empty cell, so (d) stands either way. The layer's X, which (b) treated together with the empty cell, is not named by him. "a layer that is not in BPM mode" is read as "a layer whose playing clip is not in BPM mode" (INFERRED; the other reading is assumption A-4). Added by Harmony, not his words: where else the empty-cell rule holds and how a waiting clear meets a newer press (assumption A-19); what an empty cell does to a stopped or a paused beat (assumption A-6). (a), (c), (d) stand.
TODAY: A click on an empty cell clears the layer at once (slice source: facts-app-cue-today.md section 2, not re-read by me). No BPM-mode wait exists (facts-takes-bpm-fire.md section 3 line 40).
@@END

@@ITEM R139
TITLE: Firing while the beat is stopped, paused or running
STATUS: CORRECTED parts (c) and (d) -- AMENDED after page 2 (R139 1 by A)
HIS: L12, L13, L17, L31
RULE: STOPPED: his click on play, or on any clip, is the new "1" (L12: "When stopped, user’s click on play or any clip is the new 1."). The beat starts on that press at the tempo number shown and the clip plays from its beginning at once; any clip does this, in BPM mode or not, fired by a click, a key, a MIDI pad or a column trigger. A click on an empty cell starts nothing (assumption A-6). Resync starts a stopped beat too and is the "1". PAUSED OR RUNNING: a newly fired BPM-mode clip waits for the "1" and starts there; if Resync is clicked while it waits, that press is the "1" and the clip starts on it (L12: "in pause or run mode, unless resync is clicked, when a new clip with BPM mode is triggered, it waits for the one."; for a Resync during a pause, assumption A-7). PAUSED, in addition: the fire sets the beat clock going again from the position it was holding at, exactly as play does (L13: "that press starts the clock from when the user clicks play or the clip"); the "1" the clip waits for is the next "1" of that resumed clock, not his press (L31: "it waits for the bpm clock to return to the 1 and then fires"). That every fire does this, whatever the clip's mode, is assumption A-1. A clip that was playing when he paused stays displayed on its frame and plays on from there when the clock runs again (L31). Pause and play never move a clip back to the "1" (L13: "Pause and play do not move the clip back to the one."). In automatic mode the "1" that a start press sets is where the count begins, not a Resync: the app places the "1" by itself from there as soon as it is sure where the "1" of the music is (item 218). A start by play or by a triggered clip, from pause or from stop, also sets the nudge amount to 0 (item 219). [page 2, A: BF246 "app should always try to find the 1"; item 219 accepted as written]
CHANGED: (a) and (b), the stopped half, stand and are confirmed by L12 and L31. (c) "PAUSED: a video or picture sequence you fire shows and does not play" and (d) "it stands on its first frame; when you press play the beat runs on from where it was held" are replaced: a fired BPM-mode clip waits for the "1" without showing (item 173), and the fire itself sets the clock going (his L13). The last clause of (d), that a play press on a beat held at its start is the "1", is removed: no paused start exists (L30). Added: "unless resync is clicked" (L12). His earlier "When paused, it will not play but will still display." (binding-decisions.md:1091) now holds for the clip that was playing when he paused (L31), not for a clip fired during the pause.
TODAY: Nothing of this exists: no stopped or paused beat (facts-takes-bpm-fire.md S4). The adopted, unbuilt tempo-row rule "only play or a hand Resync starts a stopped beat" with its tests is void (facts-takes-bpm-fire.md 5.8; area-tempo.md O-03, O-04).
@@END

@@ITEM R140
TITLE: How his show-start sentence is read
STATUS: REPLACED by his answer to question 172
HIS: L30, L12
RULE: "press start" is the play button of the tempo bar (L12 names play as what starts a stopped beat). "load up the clips" is read as filling the clip grid (INFERRED: Harmony's reading, the second sentence of the question's option B; L30 does not say what the phrase means). It is not firing clips so that they stand ready, and no such standing-ready state is built: clips start together by the column trigger and by nothing else (L30: "The only way to do this is with a column trigger."). "cue them ahead of time" is looking at them in the preview monitor first (topic B).
CHANGED: The page's "load up the clips is firing them while the beat does not run, so that they stand ready" is replaced. The page's readings of "press start", of the column and of "cue them ahead of time" stand. What "load up the clips" means instead is Harmony's reading, marked INFERRED in the RULE; nothing is built on it.
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
HIS: L12, L94
RULE: When the app starts it opens to the very last show (L94: "When you open the application, it opens to the very last show."), in automatic mode (assumption A-20), with the beat stopped: until his first fire, play or Resync the circle stands still and everything that follows the beat stands on the "1". That first press is the new "1" (L12). Opening a show never changes the beat or the mode: stopped stays stopped, running keeps running, and the tempo number stays as it is. A show holds no tempo number, no mode and nothing of the tempo bar's play, pause or stop; of the tempo bar it holds only the nudge amount (which exists in automatic mode: assumption A-9). The action buttons open as saved, and no action plays until the beat runs.
CHANGED: nothing. Added: his L94 on what the app opens to; the mode at the start and that a show keeps no mode (Harmony's, assumption A-20).
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
STATUS: STANDS -- part (b) now rests on item 189 -- AMENDED after page 2 (R164 1 by A, R164 2 by A)
HIS: L32, L12
RULE: (a) The audio he brings after this round -- "3 10 min audio clips and a longer set" (BF272), read as the three tracks asked of him before and one longer set (INFERRED: his words do not say what they are for) -- is for measuring [page 2, A: BF272 "after this round of questions, I will give you 3 10 min audio clips and a longer set to look at."] how well the listening follows real music: how well it keeps the beats on the music, how far its tempo number sits from the truth, and how often it would find the "1" right (the list is from the answer to his question on 189, not his words). One part of the BPM-mode work waits for them, and so does the answer to that question (item 189). (b) Who places the "1" in automatic mode is item 189: the app always tries to find the "1" by itself and places it whenever it is sure; he corrects it with Resync, and after a Resync his "1" holds while the music runs on (item 218; what lets the app place it again is assumption A3-1). How reliably the app finds the "1" is one of the things measured on the audio he brings (BF272). [page 2, A: BF246 "b. app should always try to find the 1 and I will correct if necessary"]
CHANGED: The clause "when you press Resync the 1 is yours and stays (question 189 A)" no longer rests on option A: by the recommended answer to 189 it holds in automatic mode too. The sentence that in automatic mode the app finds the "1" itself stood by inferred consent; it is no longer the rule by itself but way c of assumption A-18, which he answers.
TODAY: While it listens the app counts the bars afresh after a drop or a breakdown, so its "1" is a guess (area-tempo.md part 1 point 26; M-7).
@@END

@@ITEM R174
TITLE: The edges: what moves the "1", pause, the nudge
STATUS: CORRECTED parts (b) and (d) -- AMENDED after page 2 (R174 1 by A, R174 2 by A)
HIS: L13, L12
RULE: (a) A clip he paused himself does not wait for the "1": it shows at once on its paused frame. (b) While a BPM-mode clip waits: a Resync press is the "1" and the clip starts on it (L12); "/2", "x2" and any tempo change do NOT move the "1" it waits for, they only change how soon that "1" comes (L13: "/2 and x2, tempo change do not move the 1."); stop takes the waiting clip away with everything else; pause holds the wait, and the clip starts on the first "1" after the clock runs again. Pause pauses the beat clock, the clips and everything the beat clock controls (L13: "Pause pauses, the beat clock, the clips and everything that it controls with BPM."; that "the clips" are the BPM-mode clips is assumption A-3); play, or a fire (L13; every fire, whatever the clip's mode: assumption A-1), sets it all going again from the position it was holding at; neither pause nor play ever moves a clip back to the "1" (L13: "Pause and play do not move the clip back to the one."). (c) Every BPM-mode clip waits, a clip set to Play Once too; a clip that must land on his press is not put in BPM mode. (d) A nudge moves the "1" a hair earlier or later and that move stays in the clock's position. A start from pause continues from the held position, in both modes, so nothing jumps at the press. In manual mode the earlier nudge is simply part of where the clock stands and stays there: nothing is undone and nothing is applied again. In automatic mode the listening then puts the beats where it hears them and no shift is added to them any more (item 219), so what the earlier nudge had moved does not last beyond that start [page 2, A: item 219 accepted as written] (L13: "that nudge is history because the user started the clock from the position it was holding at"). A start from stop makes his press the "1". In automatic mode every start of the beat after a pause or a stop -- by tempo play, by a triggered clip or by Resync -- sets the nudge amount shown as "nudge X ms" back to 0, and from then on no shift is added to where the app hears the beats; a Resync while the beat runs sets it to 0 too (item 219). In manual mode there is no amount to set back. [page 2, A: item 219 accepted as written]
CHANGED: (b): "'/2', 'x2', a tap or a typed tempo move the 1 it waits for" is replaced by "do not move the 1" (L13; the tap is item 190). Added to (b): pause and play never move a clip back to the "1"; what a pause holds, in his own sentence, with assumption A-3 named for which clips. (d): his nudge paragraph is read as agreeing with the page ("leaves the nudge number as it is") and adds the reason; the other reading is in assumption A-8, which he now gets as a line. (a) and (c) stand.
TODAY: Not built: no pause of the beat, no nudge on main (area-tempo.md part 1 points 3, 25).
@@END

@@ITEM R175
TITLE: Four earlier things that later words replaced
STATUS: STANDS -- points (1) and (2) are now confirmed by his words
HIS: L11, L12, L13
RULE: (1) Firing a BPM-mode clip between two "1"s: it waits and starts on the "1" (L12); it is not started at once and cut back later. A clip that is already playing and has fallen out of time plays on and is cut once on the next "1"; a Resync cuts it at once. Pause and play never cut a clip back to the "1" (L13: "Pause and play do not move the clip back to the one."). (2) A stopped beat is started by play, by Resync, and by any clip he fires (L12). (3) The app opens with the beat stopped (item R163). (4) Nothing holds a fire to a line "in Quantize mode": Quantize is out of live use.
CHANGED: nothing replaced. (1) gains his L13 sentence on pause and play. The paper's own sentence, that a tempo change, "/2" and "x2" never put a BPM-mode clip out of time, is NOT taken: L13 says only that they do not move the "1". Whether a tempo change can put a playing clip out of time is a matter of how a clip is held to the beat clock (topic H).
TODAY: not checked
@@END

@@ITEM R176
TITLE: The tempo bar: stop, pause, steps, range, nudge, circle
STATUS: CORRECTED parts (d), (e), (g); (a) and (b) gain his words -- AMENDED after page 2 (R176 1 by A, R176 2 by A)
HIS: L14, L13, L72
RULE: (a) Stop: one press; every clip leaves every layer (BF271: "Stop removes all clips from all layers"), a clip in BPM mode and a clip that is not, alike, a layer set to Ignore Column Trigger too ("all layers"). They leave at once on the press, in the middle of a bar too, with a cut and no fade: that is Harmony's reading, not his words (assumption A3-3). A clip that waits for the "1" goes with the rest (item R174 b). Only the layers go empty: the clips stay in their cells. What the stop does to an audio file is item 273 (topic K), to actions item 226 (topic D); [page 2, A: BF271 "Stop removes all clips from all layers so it would stop."] the beat stops and the tempo number is not lost; stop also stops all actions and everything else (L72: "The tempo stop button stops all actions as well as everything else."); an effect that holds or trails a picture is not switched off by it (item R216). (b) Pause holds the beat clock, the BPM-mode clips and everything the beat clock controls (L13: "Pause pauses, the beat clock, the clips and everything that it controls with BPM."); a clip that is not in BPM mode plays on (assumption A-3). No button pauses every clip at once; each clip keeps its own pause. The tempo bar's play, pause and stop are the only transport buttons at the top. (c) "-" and "+" step the tempo by 1 BPM, in whole numbers. (d) The tempo runs from 22 to 480; by hand it always does (L14: "manually it will always go from 22 to 480"). "/2" and "x2" work all the way to both limits, in manual mode and on the listening clock (L14); what a press at a limit does is assumption A-11. The app's listening is given the same open range from the first build; if that distorts the listening, its range is narrowed later and the hand range is not (L14: "If having such an open range is distorting the listening clock, then we will change that later"; assumption A-10). (e) The nudge: plus moves the beat earlier; one press is 1 ms, a held press repeats; pause by itself never touches it; the start that follows a pause or a stop sets the amount shown in automatic mode to 0 (item 219). [page 2, A: item 219 accepted as written] In automatic mode the amount is shown as the text "nudge X ms", a number can be typed, and it runs from -500 to +500. In manual mode no nudge number is shown: the nudge simply moves the "1" to where he wants it (L14; what is counted, limited and saved in each mode is assumption A-9). (f) A tap starts nothing: not a stopped beat and not a paused one. (g) There are no words "Bar 1" to "Bar 4"; the circle alone shows the beat: one complete turn of the circle is one bar and each of its four spots is one beat (L14). What a press of "-", "+", "/2", "x2", tap, Resync or a typed number does while the app is in automatic mode -- it holds, and the mode stays automatic -- is item 189.
CHANGED: (d) replaced: the page said 30 to 400 with "/2" greyed below 60 and "x2" above 200. (e): added that "nudge X ms" shows only in automatic mode; the typed number and the limits -500 to +500 stand, in the mode where the number shows. (g): added what the circle is. (a): added that stop stops all actions (L72). (b): his L13 sentence added; whether "the clips" there means every clip is assumption A-3. (c) and (f) stand; (f) is his 128 (binding-decisions.md:957), now said for the paused beat too. The closing sentence on automatic mode replaces the paper's silence on question 189.
TODAY: Every tempo request is folded into 60 to 200 (area-tempo.md part 1 point 9); the five buttons "/4 /2 x1 x2 x4" change nothing (point 11); ">" and "||" pause every playing clip and "[]" stops routines only (point 2); the text "Bar N" is shown (point 12); no nudge exists on main (point 3).
@@END

@@ITEM R214
TITLE: What stands still while the beat does not run
STATUS: STANDS -- one clause waits on his question L107 (topic I); part (d) rests on item 189 -- AMENDED after page 2 (R214 1 by A, R214 2 by A)
HIS: L13, L72, L107, L32
RULE: (a) Stop: the circle stands on the "1" and every clip leaves. Everything that follows the beat stands on the "1" until the beat runs again: signals that run on the beat, sliders driven by the beat, actions (stop stops them all, those of clips, of layers and of global alike: L72 and BF250; what that does to their buttons, and whether any of them starts again with the beat, is item 226 of topic D, which wins over this list), [page 2, A: BF250 "tempo stop stops all actions, not just global"] the autopilot's count, MilkDrop's Jukebox count. (b) Pause: the same things stand where they were (L13: "Pause pauses, the beat clock, the clips and everything that it controls with BPM."); a clip that is not in BPM mode plays on (assumption A-3). (c) In both, what the music itself drives (loudness, bass, tones) keeps moving. (d) In automatic mode the app keeps listening while the beat is stopped or paused, and the tempo number keeps following the music -- within the hold of a number he set by hand (item 189) -- so his first fire of the next song starts the beat at that song's tempo; in manual mode the number holds what he set. Effects that read the beat by themselves (a strobe, a pulse) hold still too while the beat is paused or stopped: on pause they stand where they were and go on in step at play; on stop they wait and start on his new "1" (item 243). How their sync is adjusted is topic I's (BF244). [page 2, A: item 243 accepted as written; BF244 "For these effects, we should just be able to adjust the sink"]
CHANGED: nothing replaced. The effects clause of (a) is taken out of "stands" until his question L107 is answered. (d) now names its tie to item 189.
TODAY: No stop and no pause of the beat exist (area-tempo.md part 1 point 3); whether the listening keeps moving the number while the beat is held is not designed (area-tempo.md M-6).
@@END

@@ITEM R215
TITLE: The buttons "/2" and "x2" beside the tempo number
STATUS: REPLACED in one part: the limits (his L14); the rest stands
HIS: L14, L13
RULE: The tempo bar has two such buttons, "/2" and "x2". "/2" halves the tempo number and "x2" doubles it, once per press; neither moves the "1" (L13: "/2 and x2, tempo change do not move the 1."). Both work all the way to the limits 22 and 480, in manual mode and on the listening clock (L14: "The /2 and x2 manual work to both limit as well as the listening clock."). A press that would pass a limit does nothing (assumption A-11). Nothing of them is saved with a show. Pressed in automatic mode they leave the mode automatic, and the listening carries on at half or at double (item 189).
CHANGED: "'/2' is greyed below 60 and 'x2' above 200" is replaced by his limits 22 and 480. "neither moves the 1" is now his word (L13), no longer inferred consent. The last sentence replaces the paper's "waits on question 189". The rest stands.
TODAY: Five buttons "/4 /2 x1 x2 x4" write a saved number that nothing reads (area-tempo.md part 1 point 11; O-11).
@@END

@@ITEM R216
TITLE: Effects that hold or trail a picture after a stop
STATUS: STANDS
HIS: L72
RULE: When stop takes every clip off, an effect that holds or trails a picture goes on doing what it does: an Echo or feedback trail fades out by itself, and a frozen picture stays until he switches that effect off (his answer "147 a", binding-decisions.md:1072). Apart from that, with no clip playing the output is black. The tempo stop's "everything else" (L72: "The tempo stop button stops all actions as well as everything else.") does not switch such an effect off (assumption A-21).
CHANGED: nothing. The paper's reading of L72, which stood inside the RULE marked INFERRED, is now a named assumption (A-21).
TODAY: With no clip playing the global effects do not run and the screen goes dark at once (slice text of R216: CompositorEngine.cpp:1064-1065, Renderer.cpp:709-712, not re-read by me).
@@END

@@ITEM R177
TITLE: The nudge and a screen's Delay
STATUS: STANDS -- his L14 adds where the nudge amount shows -- AMENDED after page 2 (R177 1 by A)
HIS: L14, L13
RULE: Two controls bring picture and sound together. A screen's Delay (0 to 100 ms, one per output screen) makes everything on that screen later: it is for the room. The nudge moves where the "1" sits, earlier or later, for everything that runs on the beat: it is for a beat that sits a little off the music. In automatic mode the nudge is a standing amount, shown as "nudge X ms", kept by the show, and set to 0 by a Resync and by every start of the beat after a pause or a stop (item 219). So a nudge amount saved with a show counts only when the show is opened while the beat is running, and then lasts until his next Resync or his next start after a pause or a stop; opened onto a stopped beat, it is 0 from his first press (assumption A3-4). [page 2, A: item 219 accepted as written] In manual mode no amount is shown and the nudge simply moves the "1" (L14); nothing of a manual-mode nudge is kept, so nothing of it comes back when the show is opened again (assumption A-9). A Delay can only make a screen later. What follows the loudness of the music cannot be moved earlier.
CHANGED: Added from L14: the amount is shown only in automatic mode. Written out: what a show keeps of a nudge made in manual mode (nothing; assumption A-9). Nothing else.
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
STATUS: REPLACED in one part: the moment of the clearing (L11, by L9); what happens stands, confirmed by his look in Resolume (L19)
HIS: L19, L11, L9
RULE: A click on a column trigger also clears every layer whose cell in that column is empty; only a layer set to Ignore Column Trigger is left alone. A click on one empty cell does the same for its layer. When the layer is cleared follows the empty-cell rule (item R168): on the next "1" if the clip playing on that layer is in BPM mode, at once if it is not (L11; that the rule holds inside a column trigger too is assumption A-19).
CHANGED: What happens stands; his Resolume does the same (L19: "D layer goes empty when a column is triggered with an empty"). The moment of the clearing is replaced: the page had "at once"; it now follows L11 (INFERRED by L9: explained once, used wherever it applies).
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
HIS: L18, L14
RULE: The tempo takes effect on the second tap (as he saw in his Resolume, L18). A long run of taps keeps following him: the tempo is the average of his last eight taps, moving on with every further tap; a gap of more than three seconds starts a new run, so that the slowest tempo, 22, can still be tapped (one beat at 22 lasts 2.7 seconds; assumption A-14). A tapped number is held to the range 22 to 480 like every tempo set by hand (L14). What a tap moves besides the number is item 190; that a tapped number holds in automatic mode is item 189.
CHANGED: The paper's gap of two seconds is replaced by three: with two seconds no tempo under 30 could be tapped, against his range (L14: "manually it will always go from 22 to 480"). Nothing else; L18 confirms only that the second tap already sets the tempo.
TODAY: The tempo stops changing after the eighth tap while every tap still pulls the beat (area-tempo.md M-1); a gap over 2.0 s restarts the count (area-tempo.md part 1 point 7).
@@END

@@ITEM D4
TITLE: A press a hair after the "1" counts as on it
STATUS: OPEN no word of his covers it
HIS: none
RULE: A BPM-mode clip fired within a tenth of a beat after the "1" starts at once and in time, as if it had been fired on the "1": it does not wait a whole bar, and it joins at the place it would have reached by then, so that it sits on the beat. A press a hair before the "1" needs no rule of its own: it waits that hair and starts on the "1" (assumption A-15).
CHANGED: The page's "within a tenth of a beat of the 1" (both sides) is narrowed to the side after the "1" (INFERRED: before the "1" the ordinary wait already does it). Added: the clip joins in time (INFERRED; a clip started a hair late from its first frame would sit off the beat).
TODAY: Not built; the tenth of a beat comes from the unbuilt rule for a clip being in time (facts-takes-bpm-fire.md section 4 line 62).
@@END

@@ITEM D5
TITLE: Every bar is four beats
STATUS: SETTLED by his words on the circle and on fours
HIS: L14
RULE: The app counts every bar as four beats, everywhere: the "1" is the first of four (L14: "A complete circle is one bar, and each of the four spots on the circle are one beat."). "Everywhere" rests on his earlier words as well: binding-decisions.md:813 "Music and especially DJ music, is always in multiples of four." and binding-decisions.md:648 "For the circle at the top that counts off 1234 and then starts over, those are beats as well."
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
RULE: laid out by Harmony where it fits in the correct area; its look is settled in the UI redesign (L8). What stays as function, from his words: the switch between automatic mode and manual mode is on the tempo bar; "nudge X ms" appears there only in automatic mode (L14); the circle shows stopped, paused and running; the tempo number can be typed and stepped in both modes (his row, binding-decisions.md:913: "beatWheel play pause stop bpm# bpm- bpm+ nudgeBack nudgeForward /2 *2"). Harmony's own, not his words: in automatic mode a small word says what the listening is doing -- searching, following the music, or holding a number he set (assumption A-22); a switch for Ableton Link joins the bar when Link is built (topic K).
CHANGED: The three drawn variants are not put to him. Added to what stays as function: the word that says whether the app has found the tempo, which the page's item named and the paper had dropped.
TODAY: The top bar's order of controls is in area-tempo.md part 1 point 1 (the state word reads SEARCHING / LOCKING / LOCKED there); the adopted thirteen-cell layout is on paper only (area-tempo.md R-11).
@@END

## ASSUMPTIONS STILL OPEN (new after page 2: ids with a 3; old ones never shown to him keep their ids)
@@ASSUME A3-1
ABOUT: 218, 189, R164, R139
TEXT: I assume the app places the 1 by itself whenever it is sure, also after the press that started the beat. Only your Resync holds: the 1 then stays yours until the music clearly changes (a new track, not beat-matched) or you pause or stop.
WHY: BF246 takes way b, which reads "until your first Resync", and adds "app should always try to find the 1": the letter read alone is way b here, his own word "always" read alone is way c. The TEXT is Harmony's middle reading and is in neither of his wordings; its pause and its stop follow his own logic for the nudge (BF167 "that nudge is history because the user started the clock from the position it was holding at") and item 219. The first sentence is not open -- he chose way b on an item that named the start press -- but it narrows BF166 "When stopped, user’s click on play or any clip is the new 1." and is said to him here once.
ALT: b) After your first Resync the 1 is yours for the rest of the night: the app never places it again. c) The app may move the 1 again whenever it is sure, in the middle of a track too, also after your Resync.
IF-WRONG: STAGE the 1 would jump away from where he put it, or stay wrong all night after a new track; also REBUILD (who counts the bars)
ASK: YES his letter and his own words pull two ways, and he would see it in every set
@@END

@@ASSUME A3-2
ABOUT: 218, R175
TEXT: I assume that when the app moves the 1 by itself, a playing clip in BPM mode falls into step on the next 1, with one cut; your own Resync cuts it at once.
WHY: BF246 lets the app place the "1"; what a playing clip does at that moment is not in his words. His standing rules are applied: a clip in BPM mode is locked to the beat ("it's time will always be locked to the BPM", binding-decisions.md:968), a clip out of time is cut on the next "1" (his "124 a", binding-decisions.md:946), his Resync cuts at once (his "121 a", binding-decisions.md:943). Also Harmony's, and hanging on this block: the circle, the count and a waiting clip go to the new "1"; the tempo number and the nudge amount do not change. He will see it most often just after he has started the beat or ended a pause.
ALT: b) The clip is cut at once to the new 1, as with a Resync.
IF-WRONG: SMALL the moment of one cut, at most one bar
ASK: LINE one cut on every playing clip in BPM mode is seen on stage; his own rules settle its kind, so he will most likely wave it through
@@END

@@ASSUME A3-3
ABOUT: 273, R176, R169
TEXT: I assume the tempo stop takes every clip off every layer at once, in the middle of a bar too, with a cut: no fade.
WHY: BF271 says "Stop removes all clips from all layers" and not whether they cut or fade. There is no "1" to wait for: the stop stops the beat on the same press ("135 the beat stops but tempo is not lost, just not playing", binding-decisions.md:1007). Harmony's own edges in old item R176 (a), as amended, hang on this block: at once, a cut, a waiting clip goes with the rest, the clips stay in their cells.
ALT: b) The clips fade out, each with its layer's own fade.
IF-WRONG: STAGE a hard cut to black where he wanted a fade, or the reverse
ASK: LINE Harmony's pick; a stop is one press that ends everything, and he can strike it
@@END

@@ASSUME A3-4
ABOUT: 219, R177, R163, A-9
TEXT: I assume a show still keeps its nudge amount, but it counts only when you open the show while the beat is running; the first start after a pause or a stop sets it to 0.
WHY: Item 219 (accepted) sets the amount to 0 at every start; his earlier "good (this is just nudge amount)" (binding-decisions.md:916) kept it in the show; the two together leave it almost no life, because the beat is stopped when the app opens. Old assumption A-9 ("saved with the show") stands only in this narrowed sense and is not re-written. He hears of it inside the one line on the nudge (FOR THE PAGE RULING), not as an item of its own.
ALT: b) A show no longer keeps a nudge amount at all.
IF-WRONG: SMALL a number on the tempo bar after opening a show
ASK: NO bookkeeping of one number; what he sees at a start is settled by item 219
@@END

@@ASSUME A-2
ABOUT: 250, 190, D3, R206
TEXT: I assume tapping only sets how fast the beat runs (the tempo number): it does not move the 1, or any other beat, forward or backwards in time. Resync and the nudge do that.
WHY: BF266 asks what "shifts the beat" meant, so 250 is not answered; the item returns in his own words ("moving the 1 forward or backwards in time"). The default is not flipped, because his earlier words lean to it: BF167 "/2 and x2, tempo change do not move the 1."; "If I tapped the tempo again, to set the tempo, the time does not change." (binding-decisions.md:855, said of the nudge number); "128 tapping tempo does not start anything" (binding-decisions.md:957). His look in Resolume (BF169 "the beat reacts instantly to the tapping on the second tap") can be read either way. In automatic mode the listening keeps the beats on the music whichever way is taken (read from item 217, not measured), so the choice shows mostly in manual mode.
ALT: b) Each tap also moves the beats, the 1 with them, a little forward or backwards, so that a beat falls on your tap; which beat is the 1 never changes.
IF-WRONG: STAGE after tapping, the beats sit a little off the music until a Resync, or jump when he did not expect it
ASK: YES he asked back; the item returns in the words his question shows he can answer
@@END

@@ASSUME A-5
ABOUT: R168
TEXT: I assume the layer's X clears the layer at once, also in the middle of a bar, as in Resolume. Only the click on an empty cell waits for the "1".
WHY: L11 changes the empty cell to "the resolume way" and does not name the X; in Resolume the X is immediate (its team, 2018).
ALT: b) The X waits for the "1" too when the clip playing is in BPM mode.
IF-WRONG: SMALL one rule for one button
ASK: NO it follows from "the resolume way"
@@END

@@ASSUME A-9
ABOUT: R176 R177 R163
TEXT: I assume the nudge amount exists only in automatic mode: shown as "nudge X ms", typed, from -500 to +500, saved with the show. In manual mode a nudge just moves the "1": no number, no limit, nothing saved.
WHY: L14 hides the number in manual mode; it does not say whether an amount is still counted, limited or saved there.
ALT: b) The amount is counted in both modes with the same limits and is only hidden in manual mode.
IF-WRONG: SMALL what the number reads after a switch between the modes
ASK: NO internal bookkeeping; what he sees is settled by L14
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

@@ASSUME A-14
ABOUT: D3 190
TEXT: I assume a long run of taps keeps following you: the tempo is the average of your last eight taps and moves with every further tap; a gap of more than three seconds starts a new run.
WHY: No word of his gives the count of taps; L18 shows the second tap already sets the tempo; three seconds because one beat at 22 (L14) lasts 2.7 seconds.
ALT: b) The tempo is fixed after the eighth tap until you pause and tap again.
IF-WRONG: SMALL a tap count
ASK: NO technical; the obvious way
@@END

@@ASSUME A-16
ABOUT: D6 172
TEXT: I assume pause pressed while the beat is stopped does nothing; stop pressed while paused stops and clears as any stop; play while the beat runs, and a second stop, change nothing.
WHY: L30 turns down the paused start, the only reason for a pause on a stopped beat; the other presses are in no word of his.
ALT: b) Pause on a stopped beat arms a held start, and the next play is the "1".
IF-WRONG: SMALL presses with no effect
ASK: NO presses with no effect; the one real other way, b, is what his L30 turned down
@@END

@@ASSUME A-20
ABOUT: 175 R163 D2
TEXT: I assume the app always opens in automatic mode: it listens from the start, and the tempo you used last shows until it has found the music's tempo. A show does not keep the mode.
WHY: The default he took for the first tempo says the music then corrects the number; no word of his says which mode the app opens in.
ALT: b) The app opens in the mode you used last.
IF-WRONG: SMALL one click at the start of a night
ASK: NO one click either way
@@END

@@ASSUME A-22
ABOUT: P2 189
TEXT: I assume the tempo bar shows, in automatic mode, a small word for what the listening is doing: searching, following the music, or holding your number.
WHY: The picture item named a word that says whether the app has found the tempo; he did not read it (L130); L8 leaves layout to Harmony.
ALT: b) No such word: the tempo number alone shows it.
IF-WRONG: SMALL a word on the tempo bar
ASK: NO a display, laid out by Harmony (L8)
@@END

## CLOSED ASSUMPTIONS (one line each)
- A-1 -> SETTLED
- A-3 -> SETTLED
- A-4 -> SETTLED
- A-6 -> page 2, item 248
- A-7 -> SETTLED
- A-8 -> page 2, item 219
- A-10 -> SETTLED
- A-13 -> SETTLED
- A-15 -> page 2, item 249
- A-18 -> page 2, item 217
- A-19 -> SETTLED
- A-21 -> SETTLED

## QUESTIONS BACK
@@ANSWER 250
HIS: BF266 "What does shifting the beep mean to you? Does that mean moving the 1 forward or backwards in time?"
ANSWER: By shifting the beat I meant sliding all the beats, the 1 with them, a little forward or backwards in time, so that a beat falls exactly where you tap. So yes: like a nudge, it moves the 1 forward or backwards in time, but never by more than half a beat, and it never makes another beat the 1. The beat has three things: how FAST it runs (the tempo number), WHERE the beats fall in time, and WHICH beat is the 1. Tapping always sets how fast. The open point is only whether tapping also moves where the beats fall. I assumed it does not: you place the beats with Resync and the nudge, and in automatic mode the app keeps them on the music by itself.
@@END

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

@@NAME automatic mode
MEANS: The app listens to the music: it finds the tempo and the "1" by itself and keeps the beats on the music; what he corrects by hand holds (items 217 and 218).
SOURCE: his words L14 ("automatic mode"), L32; BF246 "app should always try to find the 1 and I will correct if necessary"; the on-screen control now is a tick box "Manual" (off), which is replaced by the pair automatic mode / manual mode
@@END

## REACHES OTHER TOPICS (from the paper)
- K (item 273, the audio file): BF271 "Stop removes all clips from all layers so it would stop. A pause would not pause it unless it is connected to the BPM." For A the first sentence is the stop of item R176 (a), amended above. For K it says the audio file stops with the stop (he reasons from "all clips": he treats what plays as a clip) and plays on through a pause unless it follows the beat. A rules nothing about the audio file.
- K (the audio he brings): BF272 "3 10 min audio clips and a longer set" is the material on which items 217 and 218 are measured; old item R164 (a) still says "the three DJ tracks". K owns that wording and the answer on mp3 / m4a.
- D (item 226, old item 180): BF250 "tempo stop stops all actions, not just global". Items R176 (a) and R214 (a) of this topic already say that the stop stops all actions; whether the buttons go off is D's.
- I (item 243, old item R221): item 243 accepted is applied here to old item R214 (amended). BF244, "we should just be able to adjust the sink" (read as "the sync"), is I's to rule.
- B (items 220, 221): BF247 "only when they trigger and play should they be on time with the beat" confirms old item R128 of this topic and changes nothing here; what a preview does is B's. Whether the tempo stop also empties the preview is B's: BF271 names the layers only.
- E (old item R165, items 258, 259): opening Studio "is like a press on stop": it follows the stop as written here (every clip off every layer at once; the clips stay in their cells). Record show runs on through a stop (259) and record to clip waits for the "1" (258): unchanged by this paper.
- F (item 234): BF256 "Pasting over a clip or deleting a clip, removes it from the layer strip, and it does not play." Whether that removal is at once or waits for the "1" like a click on an empty cell (old item R168 b of this topic) is F's to rule; A's empty-cell rule is the nearest standing rule.
- H: when the app moves the "1" by itself (item 218), how a playing BPM-mode clip is brought into step is old item R175 (1) of this topic applied (assumption A3-2); how a clip is held to the beat clock is H's.
- X: assumptions X-4 and X-10 and item C14 of the old spec-X repeat old items 189 and 190; they follow items 217, 218 and 250 as ruled here.
- J: the tempo bar's buttons in the keyboard and MIDI mapping are untouched by this round.

## CONFLICTS (from the paper)
- The nudge amount across pause, stop and play. NEW: item 219, which he left empty and so accepted as written: "when you start the beat again after a pause or a stop, the nudge is history: the nudge number shown in automatic mode goes back to 0" (the page's words, not his; accepted with the file of 2026-10-09, in which BF246 to BF271 stand). EARLIER: his answer "129 b" (binding-decisions.md:959), which Harmony had filed as "stop and play never touch the nudge; only Resync zeroes it". In between stands his own "that nudge is history because the user started the clock from the position it was holding at" (binding-decisions.md:1118). The newest wins: a start sets the amount to 0. One line for him: "A start after pause or stop now sets the nudge number to 0; on 4 October you had it stay."
- No conflict on the stop: BF271 "Stop removes all clips from all layers" says what he said before, "126 and 127 stop clears all clips from layer strips, pause stops them, tempo setting stays the same." (binding-decisions.md:952) and "135 the beat stops but tempo is not lost, just not playing" (binding-decisions.md:1007). His second sentence, "A pause would not pause it unless it is connected to the BPM.", agrees with his "136 b" (binding-decisions.md:1009).
- Inside one box, not against earlier words: BF246 takes the letter b ("until your first Resync", the page's words) and adds "app should always try to find the 1". Carried by assumption A3-1 (YES).

## NOT DONE / UNSURE (from the paper)
- 218: nothing is known about how often the app would find the "1" right on his music. Cheapest: the measurement on the audio of BF272, before any promise; the rule says so.
- 218: "when it is sure" has no number, and "the music has clearly changed" is one event shared by items 217 and 218; both are values of the measurement, not chosen on paper.
- 219: "no standing shift is applied after the start" is read from the way he did not take. If the app's heard beat sits late by a fixed amount on his rig, he will have to nudge again after every pause; the cure for that is the tuning of the beat detection (BF242), not the nudge. Not asked.
- 219 in manual mode: a start from a pause runs on from the held position, so an earlier nudge stays inside that position; there is no number to set back. Unchanged, old assumption A-9 (internal).
- The stop: "a layer set to Ignore Column Trigger too" now rests on his "all layers" (BF271); before it rested on no word of his. Not asked.
- The stop and the autopilot: whether the autopilot may trigger clips onto the emptied layers while the beat is stopped is not ruled anywhere (old assumption A-13 says only that it never starts a stopped beat). Cheapest: topic K's paper on the automatic features, or one line.
- Old internal assumptions A-5, A-9, A-11, A-12, A-14, A-16, A-20, A-22 were tested against the 33 boxes and the 57 items: none is settled or contradicted; nothing written for them.
- TODAY lines are taken from the old blocks' TODAY lines; no program text was opened for this paper (read, not run, by the earlier papers).

Written (system clock): Fri Oct  9 18:49:19 EDT 2026

## FOR THE PAGE RULING (from the ruling)
- MUST BE PUT TO HIM, by weight: (1) A3-1, YES: who holds the "1". It is the one real question of this topic: he sees it in every set, and it decides who counts the bars. (2) A-2, YES, with ANSWER 250 printed before it: he asked back, so the item returns in his own words ("moving the 1 forward or backwards in time"). Then two lines he can strike: A3-2 (a playing clip in BPM mode when the app moves the "1": one cut on the next "1") and A3-3 (the tempo stop cuts, no fade). A3-4 and the eight old internal assumptions stay with Harmony.
- NEWEST AGAINST EARLIER, the start press: BF246 "b. app should always try to find the 1 and I will correct if necessary" against BF166 "When stopped, user’s click on play or any clip is the new 1." and BF171 "I would be setting a new 1 for the tempo". In automatic mode his start press is the "1" only until the app is sure of the music's "1". This is the first sentence of A3-1's text: if the page ruling shortens A3-1, that half must not be lost. A Resync used as the start does hold.
- NEWEST AGAINST EARLIER, the nudge (the paper's CONFLICTS, first bullet, with its price added): item 219, accepted by an empty box, against his "129 b" (binding-decisions.md:959) and his "good (this is just nudge amount)" (binding-decisions.md:916). One line for him: "A start after a pause or a stop sets the nudge number to 0: if the beat sits off the music you nudge again each time, and a nudge saved with a show is gone at your first press. On 4 October you had the nudge stay through stop and play." If he strikes it, item 219, old items R174 (d), R176 (e) and R177 and assumption A3-4 go back to the old rule.
- NOTHING IS MEASURED: items 217 and 218 rest on a beat detection that finds the tempo and the "1" on his music. BF242 "we will be fine tuning this beat detection till it's perfect" is what he expects, not a result. His page should say once that the test on the audio of BF272 comes first, and that he is told before anything is built if a correction needs manual mode or the app's "1" cannot be trusted.
- AT THE MERGE: AMEND R214 1 stands in BOTH papers, A and I, with the same OLD. A's applies first and I's is reported "NOT applied" (seen in a dry run of merge3.py with this ruling in place: every amendment of topic A applies, 15 in all). The substance is the same; I's wording also points to its Sync control. Keep one of the two. AMEND R214 2 (new) is only a pointer to D's item 226.
- DEPENDS ON D: what the tempo stop does to actions and to their buttons (item 226, D3-1), also with Link on (D3-7). A rules only that the stop stops them all (BF250). D's paper leans on A for "an action's place follows a 1 that the app moved" (D3-6): ITEM 218 now says that everything on the beat clock follows it.
- DEPENDS ON K: the audio file at stop and at pause (item 273); MilkDrop on the output after a stop (K3-3): if he strikes that line, old item R216's "with no clip playing the output is black" gains an exception. K's answer on BF272 says what the audio is for; AMEND R164 2 marks that as INFERRED.
- DEPENDS ON F: item 236 wants a snapshot to open with the tempo PAUSED and clips held on their layers. A's pause does hold clips on layers; but old item R163 says "Opening a show never changes the beat or the mode", and a show holds nothing of play, pause or stop. Whether a snapshot is the exception is F's ruling to make; A writes nothing for it. F3-3 (a playing clip deleted: at once, or on the "1") stays F's: in A the empty cell waits for the "1", the stop and the layer's X do not.
- DEPENDS ON B, E, H, X: B3-5 (the cue after a stop) follows AMEND R176 1. E-7 and item 259 are untouched by this ruling. H owns how a playing clip is brought into step; A3-2 only applies old item R175 (1). X's own paper already follows items 217, 219, 248, 249 and 250 in C2, C14, C15, C16, N9 and X-5; C14 points here for how long a Resync holds: after this ruling that is "until the music clearly changes, a pause or a stop", still open as A3-1.
- NOT RULED ANYWHERE (from the paper's NOT DONE, still true): whether the autopilot may trigger clips onto layers that a stop has emptied while the beat is stopped. Old assumption A-13 says only that it never starts a stopped beat. One line in topic K's automatic features, or at the build.
- THE PAPER'S OWN SECTIONS, which the merge copies unruled: REACHES, bullet X, lacks C15 and X-5 (X amends them itself). CONFLICTS, third bullet, is A3-1; a bullet on the start press is missing there and stands above. NOT DONE, the bullet on "no standing shift", is now in ITEM 219's RULE and CHANGED; the bullet on Ignore Column Trigger stands ("all layers", BF271).
- COUNTS: 14 findings of the checker, none rejected (9 accepted as the checker says, 5 changed differently); 9 findings of my own; 20 blocks under REPLACEMENT BLOCKS (3 ITEM, 10 AMEND, 5 ASSUME, 1 ANSWER, 1 NAME).
Written (system clock): Fri Oct  9 19:34:20 EDT 2026

