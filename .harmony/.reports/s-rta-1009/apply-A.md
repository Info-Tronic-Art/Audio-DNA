# APPLY A -- Triggering clips and the tempo bar (s-rta-1009, page 2)

## SUMMARY
- The "1" in automatic mode is no longer his alone: the app always tries to find it and places it when it is sure; he corrects it with Resync (218 b, BF246, BF242). Open, and put to him: when the app may move the "1" again after his Resync (A3-1).
- His hand tempo in automatic mode holds close to his number until the music clearly changes tempo (217 accepted): item 189 is no longer only a recommendation, but still nothing is promised before it is measured on the audio he brings (BF272).
- A start of the beat after a pause or a stop sets the nudge amount to 0 (219 accepted): this reverses the old rule "only Resync sets it to 0" and his earlier "129 b".
- "Stop removes all clips from all layers" (BF271) is NOT new: it repeats his words of 2026-10-04. It is written out in full here with its edges (at once, every layer, waiting clips too, the cells keep their clips).
- Effects that read the beat by themselves hold still while the beat is paused or stopped (243 accepted): item R214 is closed on that point.
- 250 is open: he asks what "shifts the beat" meant. He gets an answer and the item again in plain words (A-2).
- 248 and 249 accepted as written: no rule changes.

## ITEMS
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
HIS: BF246 "b. app should always try to find the 1 and I will correct if necessary"; BF242 "we should be able to resync the 1"
RULE: In automatic mode the app always tries to find the "1" of the music by itself, from the moment it listens and all night long (BF246), and it places the "1" there whenever it is sure. While it is not sure, the "1" stays where it is and the app counts on from it, four beats to the bar. He corrects it when necessary: a press on Resync is the "1" at once, in automatic mode too, and the mode stays automatic (BF246; BF242: the case he names is a detection that finds the correct BPM but not the correct "1"). A press that starts a stopped beat -- play, or a clip he triggers -- is the "1" at that moment (with @@ITEM R139 as it stands), but it is not a Resync: from there the app places the "1" by itself as soon as it is sure. AFTER A RESYNC his "1" holds: the app counts on from it, keeps each beat on the music, and does not change which beat is the "1" while the music runs on. The app places the "1" by itself again only when the music has clearly changed (a new track that is not beat-matched: the same moment at which it lets go of a tempo he set, item 217) or after the beat has been stopped and started again (assumption A3-1: his words say "always" and the letter he chose says "until your first Resync"; this is the reading between the two). WHEN THE APP MOVES THE "1" BY ITSELF: the circle and the count go to the new "1"; the tempo number does not change; the nudge amount does not change; a BPM-mode clip that waits starts on the new "1"; a BPM-mode clip that is playing is not cut at that moment and falls into step on the next "1", with @@ITEM R175 point (1) as it stands (assumption A3-2). In manual mode the app never places and never moves the "1": it is the press that started the beat, or his Resync, moved only by his nudge. How sure "sure" must be is found by measurement on the audio he brings (BF272) and tuned with the beat detection (BF242); nothing is promised before that, and if the app's "1" proves wrong more often than right on his music he is told before anything is built.
CHANGED: The item as he read it (the "1" is always his; the app never moves it) is replaced by way b plus his own words. "always try to find the 1" goes further than way b's "until your first Resync": the rule gives the app the "1" from the start and again after the music changes, and keeps his Resync safe in between; what that leaves open is assumption A3-1. Against the old blocks: item 189's sentence that the listening "never moves which beat is the 1" and item R164 (b) "the 1 is his ... the listening does not move it" no longer hold (amended below); assumption A-18, the half about the "1", is closed with its way c taken. BF242 "finidng" is read as "finding" (INFERRED, a typing slip).
TODAY: While it listens the app counts the bars afresh after a drop or a breakdown, so its "1" is a guess (old item R164 TODAY: area-tempo.md part 1 point 26, M-7). The beat-in-bar count advances per HEARD beat, so a missed or an extra heard beat moves the "1" (old item 189 TODAY: F8; not re-read by me). To build: a "1" the app places only when it is sure, a Resync that holds against the listening, and the event "the music has clearly changed" shared with item 217. How often the app would find the "1" right is unmeasured (U2).
@@END

@@ITEM 219
TITLE: A start after pause or stop sets the nudge amount to 0
STATUS: ACCEPTED as written; no box of his says otherwise
HIS: box left empty = accepted as written
RULE: Whenever he starts the beat again after a tempo pause or a tempo stop -- by tempo play, by a clip he triggers, or by Resync -- the nudge is history: the nudge amount shown in automatic mode as "nudge X ms" goes back to 0 on that press. From a pause the beat clock runs on from the position it was holding at, so nothing jumps on the start; from a stop his press is the new "1". After the start no standing shift is applied any more: in automatic mode the beats sit where the app hears them, and he nudges again if the beat still sits off the music. The nudge amount therefore goes to 0 in three cases: a Resync, a start after a pause, a start after a stop. Pausing or stopping by itself does not change the number: it goes to 0 at the start that follows. The nudge amount does NOT stay through pause, stop and play (way b, not taken). In manual mode there is no amount to set back (with @@ITEM R177 as it stands after the amendment of this paper). What a show keeps of the nudge amount is assumption A3-4.
CHANGED: nothing: accepted as written. Against the old blocks: item R174 (d) and assumption A-8 said the opposite ("No start changes the nudge amount; only Resync sets it to 0"); they are replaced (amended below), and with them the end of item R177. "no standing shift is applied any more" is read from the item's way b, which he did not take ("keeps shifting the beat"): INFERRED. His earlier "129 b" is reversed: see CONFLICTS.
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
HIS: box left empty = accepted as written
RULE: A BPM-mode clip triggered within a tenth of a beat after the "1" starts at once and in step with the music: it does not wait a whole bar for the next "1", and it joins at the place it would have reached had it started on the "1", so that it sits on the beat. A press before the "1" needs no rule of its own: the clip waits that moment and starts on the "1". A press later than a tenth of a beat after the "1" waits for the next "1". Everything else with @@ITEM D4 and @@ITEM R128 as they stand.
CHANGED: nothing: accepted as written. Assumption A-15 is closed by it; item D4 already says the same, and its status is no longer open.
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

## AMENDMENTS
@@AMEND 189 1
OLD: RECOMMENDED, NOT YET HIS WORD; nothing in it is promised before it is measured on his three DJ tracks.
NEW: ACCEPTED BY HIM in round 2 (item 217 as written; item 218 by way b and his own words; BF242); nothing in it is promised before it is measured on the audio he brings (BF272), and the beat detection is tuned on that audio until it holds.
HIS: item 217 accepted as written; BF246 "app should always try to find the 1"; BF242 "we will be fine tuning this beat detection till it's perfect"
WHY: The rule stood as Harmony's recommendation waiting on his reply; he has now read it as items 217 and 218 and answered.
@@END

@@AMEND 189 2
OLD: The app counts the beats and bars on from it by its own clock; its listening keeps each beat on the music and never moves which beat is the "1"; the "1" stays his until he sets a new one, also after the app has let go of his tempo (assumption A-18).
NEW: Beside that, the app always tries to find the "1" of the music by itself and places it whenever it is sure; he corrects it with Resync when necessary. After a Resync his "1" holds while the music runs on: the app counts on from it and keeps each beat on the music; it places the "1" by itself again only when the music has clearly changed or after a stop (assumption A3-1). A press that starts a stopped beat is the "1" at that moment but does not hold against the app as a Resync does. The whole rule is item 218.
HIS: BF246 "b. app should always try to find the 1 and I will correct if necessary"
WHY: The old rule gave the "1" to him alone and forbade the listening to move it; he chose the other way and added that the app should always try.
@@END

@@AMEND 190 1
OLD: the "1" is placed with Resync (assumption A-2).
NEW: the "1" is placed with Resync, and in automatic mode also by the app itself when it is sure (item 218). Whether a tap also slides the beats onto his taps waits on his answer (item 250, assumption A-2); that a tap never makes another beat the "1" holds either way.
HIS: BF246 "app should always try to find the 1"; BF266 "What does shifting the beep mean to you?"
WHY: The old sentence named Resync as the only thing that places the "1" and treated the tap question as accepted; the app now places the "1" too, and he has asked the tap question back.
@@END

@@AMEND R164 1
OLD: by the recommended rule the "1" is his -- the press that starts a stopped beat (L12) or his Resync -- and the listening does not move it. Whether the app also places the "1" by itself until his first Resync, as the page had it, is his choice (assumption A-18, way c); how reliably it would do so is one of the things the tracks measure.
NEW: the app always tries to find the "1" by itself and places it whenever it is sure; he corrects it with Resync, and after a Resync his "1" holds while the music runs on (item 218; what lets the app place it again is assumption A3-1). How reliably the app finds the "1" is one of the things measured on the audio he brings (BF272).
HIS: BF246 "b. app should always try to find the 1 and I will correct if necessary"
WHY: The old text left the choice to him as way c of an assumption; he has taken it and widened it.
@@END

@@AMEND R139 1
OLD: APPEND
NEW: In automatic mode the "1" that a start press sets is where the count begins, not a Resync: the app places the "1" by itself from there as soon as it is sure where the "1" of the music is (item 218). A start by play or by a triggered clip, from pause or from stop, also sets the nudge amount to 0 (item 219).
HIS: BF246 "app should always try to find the 1"; item 219 accepted as written
WHY: The old rule made the start press the "1" and said nothing of the app moving it or of the nudge amount; both now follow from his answers.
@@END

@@AMEND R174 1
OLD: No start changes the nudge amount; only Resync sets it to 0 (assumption A-8).
NEW: Every start of the beat after a pause or a stop -- by tempo play, by a triggered clip or by Resync -- sets the nudge amount shown in automatic mode back to 0, and no standing shift is applied from then on; a Resync while the beat runs sets it to 0 too (item 219).
HIS: item 219 accepted as written
WHY: The old sentence kept the amount through every start; he accepted the item that says the nudge is history and the number goes back to 0.
@@END

@@AMEND R176 1
OLD: every clip leaves every layer, a layer set to Ignore Column Trigger too;
NEW: every clip leaves every layer (BF271: "Stop removes all clips from all layers"; his words of 2026-10-04, binding-decisions.md:952: "stop clears all clips from layer strips"): at once on the press, in the middle of a bar too, with a cut and no fade (assumption A3-3); a clip in BPM mode and a clip that is not, alike; a layer set to Ignore Column Trigger too ("all layers"); a clip that waits for the "1" and a clearing that waits for the "1" are taken away with the rest. The clips stay in their cells of the deck: only the layers go empty. Each layer keeps its sliders, its switches and its effects as they are. Whatever plays on a layer as a clip leaves with it, whatever kind of clip it is;
HIS: BF271 "Stop removes all clips from all layers so it would stop."
WHY: The old clause said the same in six words; his sentence repeats it, and the edges it leaves (when, which layers, waiting clips, the cells) are now written out.
@@END

@@AMEND R176 2
OLD: pause never touches it.
NEW: pause by itself never touches it; the start that follows a pause or a stop sets the amount shown in automatic mode to 0 (item 219).
HIS: item 219 accepted as written
WHY: The old clause could be read as "the amount comes through a pause unchanged"; after item 219 it does not survive the start that ends the pause.
@@END

@@AMEND R177 1
OLD: and set to 0 by a Resync; so a nudge saved with a show lasts until his first Resync of the night.
NEW: and set to 0 by a Resync and by every start of the beat after a pause or a stop (item 219). So a nudge amount saved with a show counts only when the show is opened while the beat is running, and then lasts until his next Resync, pause or stop; opened onto a stopped beat, it is 0 from his first press (assumption A3-4).
HIS: item 219 accepted as written
WHY: The old sentence had a saved nudge last until the first Resync; the beat is stopped when the app opens, and a start now sets the amount to 0.
@@END

@@AMEND R214 1
OLD: Whether effects that read the beat by themselves (a strobe, a pulse) hold still too is not settled here: he asks whether that is a good idea (L107), and topic I answers him.
NEW: Effects that read the beat by themselves (a strobe, a pulse) hold still too while the beat is paused or stopped: on pause they stand where they were and go on in step at play; on stop they wait and start on his new "1" (item 243). How their sync is adjusted is topic I's (BF244).
HIS: item 243 accepted as written; BF244 "For these effects, we should just be able to adjust the sink"
WHY: The clause waited on his question L107; he has had the answer and accepted item 243.
@@END

## ASSUMPTIONS
@@ASSUME A3-1
ABOUT: 218, 189, R164
TEXT: I assume that after your Resync the 1 stays yours while the music runs on. The app places the 1 by itself again only when the music clearly changes (a new track that is not beat-matched) or after a stop.
WHY: BF246 takes way b, which reads "until your first Resync", and adds "always try to find the 1"; when the app may move a 1 he has corrected is not said.
ALT: b) After your first Resync the 1 is yours for the rest of the night; the app never places it again. c) The app may move the 1 again whenever it is sure, in the middle of a track too.
IF-WRONG: STAGE the 1 would jump away from where he put it, or stay wrong all night after a new track; also REBUILD (who counts the bars)
ASK: YES his letter and his own words pull two ways, and he would see it in every set
@@END

@@ASSUME A3-2
ABOUT: 218, R175
TEXT: I assume that when the app moves the 1 by itself, a BPM-mode clip that is playing is not cut at that moment: it falls into step on the next 1. Only your own Resync cuts it at once.
WHY: BF246 lets the app place the 1; what a playing clip does at that moment is not said. The old rule for a clip out of time (cut once on the next 1) is applied.
ALT: b) The clip is cut at once to the new 1, as with a Resync.
IF-WRONG: SMALL the moment of one cut, at most one bar
ASK: NO it applies a rule that already stood, and how a clip is held to the beat is topic H's
@@END

@@ASSUME A3-3
ABOUT: 273, R176, R169
TEXT: I assume the tempo stop takes every clip off every layer at once, in the middle of a bar too, with a cut: no fade and no waiting for the 1.
WHY: BF271 says "Stop removes all clips from all layers" and not whether they cut or fade; an empty cell, by contrast, waits for the 1.
ALT: b) The clips fade out, each with its layer's own fade. c) BPM-mode clips leave on the next 1.
IF-WRONG: STAGE a hard cut to black where he wanted a fade, or the reverse
ASK: LINE Harmony's pick; a stop is one press that ends everything, and he can strike it
@@END

@@ASSUME A3-4
ABOUT: 219, R177, R163
TEXT: I assume a show still keeps its nudge amount, but it counts only when you open the show while the beat is running; the first start after a pause or a stop sets it to 0.
WHY: Item 219 (accepted) sets the amount to 0 at every start; his earlier "good (this is just nudge amount)" kept it in the show; the two together leave it almost no life.
ALT: b) A show no longer keeps a nudge amount at all.
IF-WRONG: SMALL a number on the tempo bar after opening a show
ASK: NO bookkeeping of one number; what he sees at a start is settled by item 219
@@END

@@ASSUME A-2
ABOUT: 250, 190, D3, R206
TEXT: I assume tapping only sets how fast the beat runs (the tempo number). It does not slide the beats earlier or later onto your taps, and it never changes which beat is the 1; Resync and the nudge do that.
WHY: BF266 asks what "shifts the beat" meant, so 250 is not answered; his look in Resolume ("the beat reacts instantly to the tapping on the second tap") can be read either way.
ALT: b) From the second tap on, each tap also slides the beats onto your finger; which beat is the 1 still does not change.
IF-WRONG: STAGE after tapping, the beats sit a little off the music until a Resync, or jump when he did not expect it
ASK: YES he asked back; the item returns in the words his question shows he can answer
@@END

## QUESTIONS BACK
@@ANSWER 250
HIS: BF266 "What does shifting the beep mean to you? Does that mean moving the 1 forward or backwards in time?"
ANSWER: Only by a hair, and only in way b: it slides all four beats, the 1 with them, a little earlier or later; it never makes another beat the 1. A beat has three things. How FAST it runs: the tempo number. WHERE each beat falls in time. WHICH beat is the 1. Example: you tap a little faster than the beat runs, and each tap lands just after a beat. As I assumed it, only the tempo number changes; the beats run on from where they were. In way b the beats also jump onto your taps, so the 1 comes that same hair later, but it is still the same beat of the bar.
@@END

## NAMES
(no new name in this topic. "Studio" is his pick of this round, BF245 and BF263; topic J files it. The names of this topic in /Users/boriskarpman/projects/RealTimeAudio/.harmony/NAMES.md stand: the 1, tempo bar, tempo play, tempo pause, tempo stop, Resync, Tap Tempo, nudge X ms, automatic mode, manual mode, beat clock, empty cell, column trigger, BPM mode. One row needs a touch by whoever keeps the list: "automatic mode ... what you correct holds" should also say that the app finds the 1 by itself, BF246.)

## REACHES OTHER TOPICS
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

## CONFLICTS
- The nudge amount across pause, stop and play. NEW: item 219, which he left empty and so accepted as written: "when you start the beat again after a pause or a stop, the nudge is history: the nudge number shown in automatic mode goes back to 0" (the page's words, not his; accepted with the file of 2026-10-09, in which BF246 to BF271 stand). EARLIER: his answer "129 b" (binding-decisions.md:959), which Harmony had filed as "stop and play never touch the nudge; only Resync zeroes it". In between stands his own "that nudge is history because the user started the clock from the position it was holding at" (binding-decisions.md:1118). The newest wins: a start sets the amount to 0. One line for him: "A start after pause or stop now sets the nudge number to 0; on 4 October you had it stay."
- No conflict on the stop: BF271 "Stop removes all clips from all layers" says what he said before, "126 and 127 stop clears all clips from layer strips, pause stops them, tempo setting stays the same." (binding-decisions.md:952) and "135 the beat stops but tempo is not lost, just not playing" (binding-decisions.md:1007). His second sentence, "A pause would not pause it unless it is connected to the BPM.", agrees with his "136 b" (binding-decisions.md:1009).
- Inside one box, not against earlier words: BF246 takes the letter b ("until your first Resync", the page's words) and adds "app should always try to find the 1". Carried by assumption A3-1 (YES).

## NOT DONE / UNSURE
- 218: nothing is known about how often the app would find the "1" right on his music. Cheapest: the measurement on the audio of BF272, before any promise; the rule says so.
- 218: "when it is sure" has no number, and "the music has clearly changed" is one event shared by items 217 and 218; both are values of the measurement, not chosen on paper.
- 219: "no standing shift is applied after the start" is read from the way he did not take. If the app's heard beat sits late by a fixed amount on his rig, he will have to nudge again after every pause; the cure for that is the tuning of the beat detection (BF242), not the nudge. Not asked.
- 219 in manual mode: a start from a pause runs on from the held position, so an earlier nudge stays inside that position; there is no number to set back. Unchanged, old assumption A-9 (internal).
- The stop: "a layer set to Ignore Column Trigger too" now rests on his "all layers" (BF271); before it rested on no word of his. Not asked.
- The stop and the autopilot: whether the autopilot may trigger clips onto the emptied layers while the beat is stopped is not ruled anywhere (old assumption A-13 says only that it never starts a stopped beat). Cheapest: topic K's paper on the automatic features, or one line.
- Old internal assumptions A-5, A-9, A-11, A-12, A-14, A-16, A-20, A-22 were tested against the 33 boxes and the 57 items: none is settled or contradicted; nothing written for them.
- TODAY lines are taken from the old blocks' TODAY lines; no program text was opened for this paper (read, not run, by the earlier papers).

Written (system clock): Fri Oct  9 18:49:19 EDT 2026
