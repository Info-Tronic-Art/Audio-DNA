# EVERY ASSUMPTION THAT IS OPEN after Boris's answers to page 2, topic by topic (the ruled blocks; made by wf/merge3.py). ASK: YES = to be put to him; LINE = one line he can strike; NO = internal.

## TOPIC A -- Triggering clips and the tempo bar
### The topic ruling's notes for the page ruling
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

### CONFLICTS (his newest words against earlier ones)
- The nudge amount across pause, stop and play. NEW: item 219, which he left empty and so accepted as written: "when you start the beat again after a pause or a stop, the nudge is history: the nudge number shown in automatic mode goes back to 0" (the page's words, not his; accepted with the file of 2026-10-09, in which BF246 to BF271 stand). EARLIER: his answer "129 b" (binding-decisions.md:959), which Harmony had filed as "stop and play never touch the nudge; only Resync zeroes it". In between stands his own "that nudge is history because the user started the clock from the position it was holding at" (binding-decisions.md:1118). The newest wins: a start sets the amount to 0. One line for him: "A start after pause or stop now sets the nudge number to 0; on 4 October you had it stay."
- No conflict on the stop: BF271 "Stop removes all clips from all layers" says what he said before, "126 and 127 stop clears all clips from layer strips, pause stops them, tempo setting stays the same." (binding-decisions.md:952) and "135 the beat stops but tempo is not lost, just not playing" (binding-decisions.md:1007). His second sentence, "A pause would not pause it unless it is connected to the BPM.", agrees with his "136 b" (binding-decisions.md:1009).
- Inside one box, not against earlier words: BF246 takes the letter b ("until your first Resync", the page's words) and adds "app should always try to find the 1". Carried by assumption A3-1 (YES).

### REACHES OTHER TOPICS
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

### NOT DONE / UNSURE
- 218: nothing is known about how often the app would find the "1" right on his music. Cheapest: the measurement on the audio of BF272, before any promise; the rule says so.
- 218: "when it is sure" has no number, and "the music has clearly changed" is one event shared by items 217 and 218; both are values of the measurement, not chosen on paper.
- 219: "no standing shift is applied after the start" is read from the way he did not take. If the app's heard beat sits late by a fixed amount on his rig, he will have to nudge again after every pause; the cure for that is the tuning of the beat detection (BF242), not the nudge. Not asked.
- 219 in manual mode: a start from a pause runs on from the held position, so an earlier nudge stays inside that position; there is no number to set back. Unchanged, old assumption A-9 (internal).
- The stop: "a layer set to Ignore Column Trigger too" now rests on his "all layers" (BF271); before it rested on no word of his. Not asked.
- The stop and the autopilot: whether the autopilot may trigger clips onto the emptied layers while the beat is stopped is not ruled anywhere (old assumption A-13 says only that it never starts a stopped beat). Cheapest: topic K's paper on the automatic features, or one line.
- Old internal assumptions A-5, A-9, A-11, A-12, A-14, A-16, A-20, A-22 were tested against the 33 boxes and the 57 items: none is settled or contradicted; nothing written for them.
- TODAY lines are taken from the old blocks' TODAY lines; no program text was opened for this paper (read, not run, by the earlier papers).

Written (system clock): Fri Oct  9 18:49:19 EDT 2026

### The assumptions of this topic after the ruling (YES first, then LINE, then NO)
@@ASSUME A3-1
ABOUT: 218, 189, R164, R139
TEXT: I assume the app places the 1 by itself whenever it is sure, also after the press that started the beat. Only your Resync holds: the 1 then stays yours until the music clearly changes (a new track, not beat-matched) or you pause or stop.
WHY: BF246 takes way b, which reads "until your first Resync", and adds "app should always try to find the 1": the letter read alone is way b here, his own word "always" read alone is way c. The TEXT is Harmony's middle reading and is in neither of his wordings; its pause and its stop follow his own logic for the nudge (BF167 "that nudge is history because the user started the clock from the position it was holding at") and item 219. The first sentence is not open -- he chose way b on an item that named the start press -- but it narrows BF166 "When stopped, user’s click on play or any clip is the new 1." and is said to him here once.
ALT: b) After your first Resync the 1 is yours for the rest of the night: the app never places it again. c) The app may move the 1 again whenever it is sure, in the middle of a track too, also after your Resync.
IF-WRONG: STAGE the 1 would jump away from where he put it, or stay wrong all night after a new track; also REBUILD (who counts the bars)
ASK: YES his letter and his own words pull two ways, and he would see it in every set
@@END

@@ASSUME A-2
ABOUT: 250, 190, D3, R206
TEXT: I assume tapping only sets how fast the beat runs (the tempo number): it does not move the 1, or any other beat, forward or backwards in time. Resync and the nudge do that.
WHY: BF266 asks what "shifts the beat" meant, so 250 is not answered; the item returns in his own words ("moving the 1 forward or backwards in time"). The default is not flipped, because his earlier words lean to it: BF167 "/2 and x2, tempo change do not move the 1."; "If I tapped the tempo again, to set the tempo, the time does not change." (binding-decisions.md:855, said of the nudge number); "128 tapping tempo does not start anything" (binding-decisions.md:957). His look in Resolume (BF169 "the beat reacts instantly to the tapping on the second tap") can be read either way. In automatic mode the listening keeps the beats on the music whichever way is taken (read from item 217, not measured), so the choice shows mostly in manual mode.
ALT: b) Each tap also moves the beats, the 1 with them, a little forward or backwards, so that a beat falls on your tap; which beat is the 1 never changes.
IF-WRONG: STAGE after tapping, the beats sit a little off the music until a Resync, or jump when he did not expect it
ASK: YES he asked back; the item returns in the words his question shows he can answer
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

### Items of page 2 that are still OPEN in this topic: 250

## TOPIC B -- The cue system
### The topic ruling's notes for the page ruling
- MUST be put to him, heaviest first: (1) B3-2 with @@ANSWER 252 -- he asked, and it is the technical call he handed over. The text for him is @@ANSWER 252 of this ruling, NOT the "ANSWER FOR BORIS" paragraph of answer-252.md, which check-252.md found to promise too much; the merge appends that paper's RECOMMENDATION as written, and its points 2 and 6 stand corrected by ITEM 252 here (a player of the preview's own for a clip previewed alone; no guarantee). (2) B3-1 with @@ANSWER 222 -- he asked what and where master cue is; old 177 and everything about master cue waits on it.
- Lines he can strike (3): B3-4 first -- it reads his newest sentence (BF247 "If the clip is loaded into the layer, and we are cueing this way, then it should play in time") as "nothing new is built": show it, do not merge it away. Then B3-3 (a preview's actions run from the click, not lined up with the 1). Then B-11 (two narrow limits: a MilkDrop clip or a simulation previewed while another of its kind plays).
- Internal, not for him (5): B3-5, B3-6, B3-7, B-14, B-18.
- Newest against earlier, to be said to him once: BF247 "Previewing a clip should not happen on the beat." against BF175 "it is triggered on the 1. Resolume plays it right away but we have actions so that will need to be playing in time with the music". The newest wins; B3-3 is the line that tells him what it does to the actions.
- Newest against earlier, to be said to him once: item 221, accepted by his empty box (a preview by name plays on through a tempo pause, a BPM mode clip and its actions too) against BF167 "Pause pauses, the beat clock, the clips and everything that it controls with BPM." The accepted item wins; the pause still holds the clips on the layers.
- Not a clash: BF248 asks about a button he named himself (BF177 "177 add a global effects and actions toggle button called master cue"); @@ANSWER 222 says so.
- Depends on topic D: (1) D's question whether tempo stop switches the button of every clip's action off (BF250 "tempo stop stops all actions, not just global"): if yes, a preview after a stop shows clips without moving actions until he switches one on again (ITEM 221); worth one clause in D's question. (2) D's engine must play a clip's actions for the preview alone, from a click, off the show's beat (B3-3, B3-6); in the show D's rules stand (an action waits for the beat).
- Depends on topics F, I, K, A: F's line on the triggering click ("the preview does not change", a LINE in F's paper) is the twin of the internal B3-7: ask it once, in F. I: keying and masks in the cue (AMEND R149 2) follow I's rule for page item 240; what a One Shot and an envelope do in a preview is ruled here (B3-6), as I's paper asked. K: a MilkDrop clip previewed by its name leans on the one MilkDrop (BF265 "Keep Milk drop as it is"); its preset-load hitch is not measured. A: the trigger rules decide when a cued clip starts (B3-4).
- U18 is amended here AND in apply-X with the same OLD text; the merge can apply only one. This ruling keeps B's (item 221 is made from U18; the rule in it is the cue topic's). If rule-X keeps X's too, B's is applied first and X's shows in the merge report as "NOT applied": that is the wanted outcome, and nothing has to be done by hand.
- Owed before the cue build, Harmony's to schedule with him (answer-252.md, UNKNOWN UNTIL MEASURED 1 to 5; none needs anything built): the frame time of his heaviest show, the cost of master cue, the cost of one previewed clip, the MilkDrop preset-load hitch, which screen paces the show. Added here: the time from a name click to the first picture of a video (his word "quick", BF247).
- The two marks (a paused preview; "not a run of its own") are looks, laid out by Harmony (BF163); not asked.

### CONFLICTS (his newest words against earlier ones)
- A clip previewed by its name: NEW, BF247: "Previewing a clip should not happen on the beat. It should just be quick so the user could go through any amount of previews as quick as they want and only when they trigger and play should they be on time with the beat." EARLIER, binding-decisions.md line 1134 (R179; BF175): "R179 all good except a previewed clip (clicking its name) shows the clip with all its actions and it is triggered on the 1. Resolume plays it right away but we have actions so that will need to be playing in time with the music". The newest words win: a preview starts at the click; its actions come with it and are not lined up with the 1 (B3-3 is the one line that says so to him). His still earlier line agrees with the new one: binding-decisions.md line 1133 (R170; BF174): "They will play right away".
- Not a clash, said once more so that it is clear: BF248 asks "what and where is the master cue?" about a button he named himself, binding-decisions.md line 1136 (177; BF177): "177 add a global effects and actions toggle button called master cue". He is not withdrawing it; he asks what Harmony made of it.

### REACHES OTHER TOPICS
- X (U18, X-24, X-29): U18 is amended here because item 221 is made from it. The old assumptions X-24 and X-29 are settled by BF247 and by item 221 as accepted: the paper of topic X should drop both. X-11 is the same matter as item 252 and stays open with it.
- D (clip actions): BF247 means a clip's own actions can play for the preview alone from the moment of a click, at the tempo's BPM, not lined up with the 1 (assumption B3-3). The actions engine has to allow a start that is not on the show's beat for the preview. BF250 ("tempo stop stops all actions, not just global") is D's: whether tempo stop switches a clip's action buttons off decides what a preview by name shows after a stop; this paper only says the preview shows the actions that are switched on.
- A (tempo bar and triggering): BF247 "only when they trigger and play should they be on time with the beat" confirms A's trigger rules and changes none; previewing never starts the tempo (item 221) stands beside A's rule that a trigger while stopped starts the beat. BF271 "Stop removes all clips from all layers" (A's rule) is applied here only as: after a stop the cue shows empty layers (B3-5).
- K (MilkDrop): BF265 "Keep Milk drop as it is" is applied here to the limit of the first build (B-11): no second MilkDrop for the preview.
- I (blend modes, keying, masks): BF260 says the blend modes and keying stay and masks come later. The cue mixes the cued layers by their blend modes (R141, R149 d) and that stands; what a mask layer does inside the cue is not ruled here and waits for I's rule on masks.
- C (item 253, accepted): only files and deck clips are previewed; nothing here contradicts it.
- F (BF256: "Pasting over a clip or deleting a clip, removes it from the layer strip, and it does not play"): a previewed clip that is deleted leaves the preview black (old B-21, untouched); F owns the rule for the layer.
- J (names): the row "preview mode" of NAMES.md changes its meaning (see NAMES); the row "preview a clip" already says "plays at once" and stands. The word Studio replaces Review in every text for him; the old assumption B-17 keeps "recording review mode" only inside his quote.

### NOT DONE / UNSURE
- Item 252: this paper's answer is a draft from the fact sheets (read, not run). Whether a second picture every frame leaves the output's frame rate alone is NOT measured; the cheapest way to settle it is the spike the old spec already names (blend the saved per-layer pictures into a small second target and read the frame time with and without it), when the build hold lifts. The final text of @@ANSWER 252 and of B3-2 is settled with answer-252.md, which was not there when this was written.
- "Drawn at the size it is shown" is safe for cued layers (they are the output's own layer pictures made smaller: INFERRED from CompositorEngine.cpp:1150-1154, read, not run). For a clip previewed by its name, which is drawn only for the preview, effects that count in pixels (a blur, a pixelate) could look slightly different at a smaller size: ESTIMATE, not checked; way b of B3-2 (draw at full size) is the cure and costs more. The technical paper should rule this.
- A preview that starts at the click needs a clock of its own for the previewed clip and its actions (one player per clip; a clip seen twice is the same playback: facts-app-cue-today.md section 5). How a previewed video starts "at once" depends on how fast its first frame is ready: not measured; the rule only forbids waiting for the beat.
- B3-4: if he strikes the line and means way b (a clip put into the cue of a layer without reaching the output), the cue system grows by a second playback per layer: to be planned only on his word.
- What tempo stop does to a clip's action buttons (BF250, topic D) decides what a preview by name shows after a stop; settled by D's paper, then one sentence in R151.
- A mask layer inside the cue (BF260): waits for topic I.
- No old assumption of this topic is dropped: B-5, B-7, B-8, B-17, B-19, B-20 and B-21 are untouched by his 33 boxes; B-14 and B-18 change and keep their ids.
- Written 2026-10-09 18:47:48 EDT by the architect of topic B; read-only, nothing built, run or measured.

### The assumptions of this topic after the ruling (YES first, then LINE, then NO)
@@ASSUME B3-1
ABOUT: 222 177
TEXT: I assume master cue is one more cue button, for global. On: the preview monitor shows the cued layers through the global effects, moved by the global actions, as the output would. Off: without them. It never changes the output.
WHY: BF248 asks "what and where is the master cue?"; his own line of 2026-10-07 names the button and not what it does.
ALT: b) On, it also lets you switch a global effect or action on for the preview monitor only, to try it first. c) On, the preview monitor shows the whole output, as on a DJ mixer.
IF-WRONG: REBUILD way b needs a second, preview-only copy of the global effects and their actions; and he would press it expecting something else
ASK: YES he asked back; no word of his says what the button does; way b is a far bigger build
@@END

@@ASSUME B3-2
ABOUT: 252 D8 D9 177
TEXT: I assume the output monitor and the preview monitor always run at the same fps, and a cued layer looks exactly as on the output. If the computer cannot keep up, the preview pauses, marked, and the output keeps going.
WHY: BF267 asks "should all run at the same fps, no?" and "What do you think?" and hands the build to Harmony; the design is ruled from answer-252.md with the corrections of check-252.md; nothing is measured.
ALT: b) The preview is always drawn smaller, so it rarely has to pause; price: a few effects look slightly different in it than on the output.
IF-WRONG: STAGE a preview that costs the output frames is seen by the audience; a preview that looks different from the output misleads him
ASK: YES he asked back (BF267); this is the technical call he handed over, ruled and put to him again
@@END

@@ASSUME B3-3
ABOUT: 220 221 R151 R179
TEXT: I assume a clip you preview by its name plays with its actions from the moment you click: they run at the tempo's BPM but are not lined up with the 1. Only a triggered clip is on the beat.
WHY: BF247 takes the wait for the beat out of previewing and does not name the actions; on 2026-10-07 he wanted them "playing in time with the music".
ALT: b) The clip's picture starts at once, and its actions join on the next 1. c) A clip previewed by its name is shown without its actions.
IF-WRONG: SMALL seen in the preview only; it changes when the preview's actions start, not how they are built
ASK: LINE his "only when they trigger and play" all but says it, and he accepted the same for the paused tempo (221)
@@END

@@ASSUME B3-4
ABOUT: 220 R149 R179
TEXT: I assume "loaded into the layer" and "cueing" mean a clip you triggered on a layer, seen through that layer's cue button: it is in time because the trigger put it in time. Nothing new is built for it.
WHY: BF247 "If the clip is loaded into the layer, and we are cueing this way, then it should play in time" names no click or button. His own words of 2026-10-05 describe cueing in this way: BF142 "click on a column to trigger all of them at the same time and cue them ahead of time via the cue system"; BF143 "exactly how a DJ mixer allows the user to cue and listen to how the tracks would be mixed together before they mix them".
ALT: b) With a layer's cue button on, a click on a clip's name in that layer puts the clip into the cue on the next 1, without it reaching the output.
IF-WRONG: REBUILD way b is a layer that plays one clip for the output and another for the cue: a large addition, though nothing built by this reading would have to be undone
ASK: LINE his words of 2026-10-05 (BF142, BF143) and BF248 "The layers have a cue button to display in the preview/cue monitor" describe cueing as triggered clips seen through the layers' cue buttons, and way b is a function he has never described; it is shown to him as a line because it reads his newest sentence
@@END

@@ASSUME B-11
ABOUT: 252 D9
TEXT: I assume two limits in the first build: a MilkDrop clip, or a generated picture that runs by itself, previewed while another of its kind plays, shows the playing one's picture, marked, not a run of its own.
WHY: BF267 asks about "come first", size and fps and does not name MilkDrop; BF265 "Keep Milk drop as it is" keeps the one MilkDrop; the trail limit of the old text is gone, because a cued layer is shown from the output's own picture of it.
ALT: b) It shows no picture, only a note that it cannot be previewed while the other plays. c) Every previewed picture is fully its own, which needs a second MilkDrop and a second copy of each such generated picture.
IF-WRONG: REBUILD way c is a second MilkDrop and second copies of every picture that keeps a state of its own; the difference between the text and way b is small
ASK: LINE a narrow limit he would most likely wave through; the MilkDrop half follows his own words (BF265)
@@END

@@ASSUME B3-5
ABOUT: 220 221 R151 R152
TEXT: I assume the tempo bar's play, pause and stop never touch a clip you preview by its name: it plays on. The cue shows the layers as they are: standing still in a pause, empty after a stop.
WHY: Item 221 covers a preview started in a pause or a stop, not one already running; BF271 "Stop removes all clips from all layers" names layers only.
ALT: b) Tempo stop also empties the preview monitor.
IF-WRONG: SMALL seen in the preview only
ASK: NO follows from item 221 as accepted and from a cued layer being the same layer seen twice
@@END

@@ASSUME B-14
ABOUT: R170 176 R151 220 252
TEXT: I assume a preview lasts until another clip or file is previewed. While the preview monitor is in cue mode it is not drawn; the toggle shows it again, from its beginning. A second click on the same name restarts it at once.
WHY: BF247 says a preview is quick and never on the beat; how long a preview lasts and what a second click does are still not said. Changed by the ruling: a preview that nobody sees is not drawn, because its work would be taken from the output (BF267 "Of course the output is more important"; item 252).
ALT: b) In cue mode it plays on out of sight, and the toggle shows it where it has got to. c) A second click on the same name changes nothing.
IF-WRONG: SMALL seen in the preview only
ASK: NO internal; nothing of it reaches the output
@@END

@@ASSUME B-18
ABOUT: R179 220
TEXT: I assume "all its actions" means the clip's own actions that are switched on. Switching one on or off while the clip is previewed shows in the preview at once and is a real change to the clip.
WHY: His words say "shows the clip with all its actions" (BF175) and not whether actions that are switched off play too. When a switch made during a preview shows is not said by him anywhere: "at once" follows the start at the click (B3-3); BF247 speaks only of when a preview starts.
ALT: b) Every action stored on the clip plays in the preview, switched on or not. c) A switch shows in the preview only from the next 1, as in the show.
IF-WRONG: SMALL seen in the preview only
ASK: NO internal: it follows the rule that an action plays only while its button is on, and the timing follows the line on when a preview's actions start
@@END

@@ASSUME B3-6
ABOUT: 220 221 R151 R179
TEXT: I assume that in a preview your click stands in for the trigger: a play-once action and a One Shot run once from the click. What reads the beat itself (an oscillator, an envelope along the beat, a strobe) follows the show's beat there too.
WHY: BF251 "that action plays again only when that clip is re-triggered", page item 242 (a One Shot runs from the next 1 after its clip is triggered) and page item 243 are written for the show; none of them names a preview, and the papers of topics D and I leave the preview to this topic. BF247 takes every wait for the beat out of a preview.
ALT: b) A play-once action and a One Shot do not play in a preview at all. c) Everything in the preview counts from the click, the effects that read the beat too.
IF-WRONG: SMALL seen in the preview only
ASK: NO internal: it carries the show's rules into the preview by BF247 and by the one beat clock of the show
@@END

@@ASSUME B3-7
ABOUT: R150 176
TEXT: I assume in the deck only a click on a clip's name feeds the preview. A click that triggers, a key, a pad, a column trigger or an action leaves it as it was, and so does a click on an empty cell's name place.
WHY: BF255 "you can select the cell which triggers it and selects it" makes the triggering click select too; his words tie the preview to the name click only (BF174 "name clicked from a clip in the deck"); an empty cell has nothing to preview.
ALT: b) A click on the name place of an empty cell turns preview mode black.
IF-WRONG: SMALL seen in the preview only
ASK: NO internal; the half on the triggering click is put to him by the show-file topic as a line of its own
@@END

### Items of page 2 that are still OPEN in this topic: 222, 252

## TOPIC C -- Presets
### The topic ruling's notes for the page ruling
- AFTER THIS RULING the topic has 9 assumptions: 1 YES (C3-6), 2 LINE (C3-1, C3-2), 6 NO (C3-3, C3-4, C3-5, C3-7, C3-8, C3-9). The paper's own summary ("three lines for him to strike, nothing to ask in full") is overtaken.
- MUST BE PUT TO HIM, in order of weight. (1) C3-6, YES: does a preset carry an envelope drawn for a clip, or only its name. Put it right after topic I's line on who owns an envelope (I3-4): if he strikes that line (every envelope in one list of the show), C3-6 falls away and R180 (a), which he accepted with "R180 yes" (BF183), covers it. (2) C3-1, LINE: no window for an old copy in a show that this same computer saved. For a show from another computer his "b" (BF249) holds in full, in both directions. (3) C3-2, LINE: the show does not wait; its way b is "keep both".
- NEWEST WORDS AGAINST EARLIER ONES: none in this topic. His "b" on 223 (BF249) replaces a rule that was Harmony's; his "b" on 228 (BF252) goes against Harmony's recommendation, not against words of his. Nothing has to be said to him once more.
- THE OLD INTERNAL ASSUMPTIONS (C-4, C-5, C-6, C-7, C-8, C-11, C-13, C-14, C-15, C-16) were tested against the 33 boxes and the 57 items: none is settled, none is contradicted, none is re-written. C-13 leans on the transparency slider, which topic I builds with the blend modes at the last stage (I3-1): a note for the build order, not a doubt for him.
- DEPENDS ON D: AMEND 192 2 and AMEND R155 1 take "moving at that moment" from item 228 and the glide from item 224; they name the item, not its letters, so a change of D's wording needs no change here. What C hands back to D's bullet "is C's": the header shows Save unless the value underneath equals the preset's; a Save writes the value set underneath (AMEND R157 6).
- DEPENDS ON I: AMEND R157 6 puts Sync into a preset only as long as item 243 keeps Sync as a control of the effect; if I's ruling takes that out, drop the Sync clause and C3-7. AMEND R180 1 and C3-6 stand on item 241 (an envelope belongs to its clip).
- DEPENDS ON F AND E: AMEND R157 5 compares a snapshot only "where topic F rules that it opens as a show"; a show recording opened in Studio compares nothing (C3-9), and E should not rule otherwise without this topic. For F's R188 (a), two things more in the show file: a note of which computer saved it last (AMEND R157 3) and room for a second, unpicked version of a preset (AMEND R157 4).
- FOR B: old B-9 was merged into item 253; its half about MilkDrop presets now stands in ITEM 253. Not ruled anywhere and B's to say: what the preview monitor shows when the name of a cell with effects only is clicked (alone it has no picture).
- TWO BULLETS OF THE PAPER'S REACHES ARE OUT OF DATE (a ruling cannot replace a bullet). I / Sync: "if such an effect gets a sync setting" -- it is firm in I's item 243 and now in R157 (6). F / Snapshot: "whether a saved snapshot carries the presets ... is F's to say" -- F's item 236 says it does. Also out of date after H2 and H7: the F bullet that says "item 223 adds nothing to the show file beyond R157 (9) as amended" (two things are added: see DEPENDS ON F AND E).
- X's amendment on the places where the app speaks by itself names the window of 223: it stands. With C3-1 the window comes only for a show from another computer.
- NOT LOOKED UP (no web tools here): whether Resolume's own presets carry a parameter's envelope. He wrote "Please emulate this." about presets (2026-10-05); if the default of C3-6 should match Resolume, that is a Researcher's task before the page is made.

### CONFLICTS (his newest words against earlier ones)
(none: BF249 "b" replaces a rule that was Harmony's, not words of his; his earlier words on keys and MIDI, where the show's set takes over (binding-decisions.md, 2026-10-04, answer 85), are about the mapping and stand beside it)

### REACHES OTHER TOPICS
- D (item 228, old D-5 and R129): his "b" in box 228 (BF252) is applied here in old item 192 only (AMEND 192 1): a slider that an action is moving is left out of a preset load. The rule of the action itself, the glide back and its time are D's. What this paper gives D: after such a load the value underneath is the one from before the action, and the header shows Save (C3-4); "Default" follows the same rule.
- B (old R170, old B-9): item 253 as accepted says a source is not previewed and only files and deck clips are; R170's rule on the double-click in the files window should read the same. MilkDrop presets are not previewed by a double-click either (B-9); K keeps MilkDrop as it is (BF265).
- I (the envelopes, item 241, BF261): two kinds of envelope, both belonging to a clip. This paper assumes a preset keeps neither (C3-6) and keeps a show's signal by its name only (R180 (a) stands). If I rules that a beat envelope is a named signal of the show, R180 (a) covers it and C3-6 shrinks to the playhead envelope.
- I (effects that read the beat by themselves, BF244: "we should just be able to adjust the sink", INFERRED: the sync): if such an effect gets a sync setting of its own, a preset of that effect has to hold it with the slider values (R157 (6) lists sliders, Dry / Wet and signals only). Not ruled here: I owns the setting; one clause to add to R157 (6) once I has named it. His "a common macro that we could set later" is I's too; whether a preset remembers that a slider follows a macro waits on it.
- F (the show file): item 223 adds nothing to the show file beyond R157 (9) as amended (per preset: identity, time of last change, a mark per version). Snapshot as he now describes it (BF243) is F's; whether a saved snapshot carries the presets as a show does is F's to say.
- J (item 244, BF262 "b"): a MIDI knob belongs to the cell, not to the clip or the effect; a preset never held a knob (R157 (6)), so nothing changes here.

### NOT DONE / UNSURE
- The fields the small window of 223 shows (which side was changed when, on which computer) are layout; only what it does is ruled. Cheapest way to settle C3-1 and C3-2: the two lines on his next page.
- C3-6 depends on how topic I's paper rules the two envelopes of BF261; read apply-I before the page is made and drop or narrow C3-6 if I has made the beat envelope a named signal.
- Whether a preset holds an effect's sync setting (BF244) waits on topic I naming that setting; see REACHES.
- Old assumptions C-4, C-5, C-6, C-7, C-8, C-11, C-13, C-14, C-15, C-16 were tested against the 33 boxes and the 57 items: none is settled or contradicted, none is re-written. C-4 (deleted stays deleted) keeps its place beside the new window: a deleted preset is not a second version.
- Nothing about the app was run; the TODAY lines are from the s-rta-1005 fact sheets (read, not run).

### The assumptions of this topic after the ruling (YES first, then LINE, then NO)
@@ASSUME C3-6
ABOUT: R180 R157 241
TEXT: I assume a preset keeps only the name of an envelope you drew for a clip, never its shape: the shape stays with that clip. Loaded on another clip, the preset sets the slider's value and the envelope does not come along.
WHY: BF261 makes an envelope "like a signal, but personalized for that clip". His "R180 yes" (BF183) to "by its name only" was given to a reading that named "Mod 1" and "Bass", before a clip could own an envelope; no word of his says whether a preset carries a clip's envelope.
ALT: b) The preset carries the envelope's shape too: loaded on another clip, it makes a copy of the envelope there and plugs it in.
IF-WRONG: STAGE a preset he saved on a clip with a drawn envelope does not move on another clip; way b changes what a preset file holds and how the header compares.
ASK: YES no words of his settle it, he would see it whenever he loads such a preset on another clip, and it decides what a preset holds.
@@END

@@ASSUME C3-1
ABOUT: 223 R157
TEXT: I assume the window that asks which version of a preset to keep comes only for a show from another computer. For a show saved on this computer, an old copy of a preset you changed here since is not asked about: the computer's stays.
WHY: His "b" (BF249) is way b of the item, "A small window asks which one to keep", put to him for a preset changed on one computer and not on the other: for a show from another computer the window therefore always comes. The item did not name a show saved on this same computer. Such a show holds an old copy of every preset he has changed here since, and a window would come at its next opening, also at launch.
ALT: b) The window asks every time the two differ, also for a show saved on this computer.
IF-WRONG: SMALL nothing is lost either way, the computer's stays; with way b a window comes at the next opening of every older show after a preset was changed here; one test to turn.
ASK: LINE a real choice of Harmony's for a case the item did not name, and the common case on one computer; he would most likely wave it through.
@@END

@@ASSUME C3-2
ABOUT: 223 R157
TEXT: I assume a show opens without waiting for the small window about two versions of a preset. The window offers only what you chose: keep one, the other is gone from this computer.
WHY: His "b" (BF249) is way b of the item, "A small window asks which one to keep": one of two, so that the other one goes is already his answer. Not said: whether the show waits for his pick. Way b of this line would add to his answer, so it is his to say.
ALT: b) The window can also keep both: the show's version gets a number after its name.
IF-WRONG: SMALL one more button in the window, or the order of two steps at opening.
ASK: LINE the version that is not kept cannot be brought back and way b would spare it; he can strike the line.
@@END

@@ASSUME C3-3
ABOUT: 223 R157
TEXT: I assume two presets that were made separately and only share a name are both kept, without asking: the show's gets a number after its name ("Blue 2").
WHY: BF249 answers for two versions of ONE preset. This second half of the old assumption was not on page 2, so he has neither seen nor answered it.
ALT: b) The same small window asks here too: keep one, or keep both.
IF-WRONG: SMALL nothing is lost either way; one more case in the window.
ASK: NO rare, loses nothing and is easy to turn.
@@END

@@ASSUME C3-4
ABOUT: 228 192 178 R155 R157
TEXT: I assume a slider that an action is moving while you load a preset is left out of the load: it keeps what was plugged into it, the header shows Save, and a Save keeps its value from before the action. "Default" works the same.
WHY: BF252 "b" says the preset's value for that slider is lost; not what happens to a signal the preset holds for it, to the header, to a Save pressed then, to Dry / Wet (taken here as one of the effect's sliders) or with "Default".
ALT: b) The header reads the preset's name all the same. c) The preset's signal for that slider is plugged in although its value is lost.
IF-WRONG: SMALL one test in the load and one in what the header reads.
ASK: NO an edge of his answer BF252 and of the header rule; cheap to turn, as with the old C-7.
@@END

@@ASSUME C3-5
ABOUT: 253 R154
TEXT: I assume the properties you see after a double-click on an effect or a preset in the Effects tab are only for looking: the rows cannot be moved there, and a preset is made only on an effect that is in the show.
WHY: Item 253 (accepted, box empty) says "shows its properties but no picture"; whether the rows can be moved was in the old assumption, not in the item he read.
ALT: b) The rows can be moved there and a preset can be saved from them, without a picture.
IF-WRONG: SMALL with no picture there is little to try; rows that move can be added later.
ASK: NO follows from "no picture", which he accepted; where it would matter is the UI redesign.
@@END

@@ASSUME C3-7
ABOUT: R157 243
TEXT: I assume a preset of an effect that reads the beat by itself also keeps that effect's Sync setting, as it keeps every slider.
WHY: BF244 "we should just be able to adjust the sink" (INFERRED: the sync) gives such an effect a setting of its own. That a preset holds it is Harmony's, from what a preset is. What Sync itself does is asked in topic I.
ALT: b) A preset leaves Sync alone: it stays as it is on the effect.
IF-WRONG: SMALL one value more or less in a preset.
ASK: NO it follows from what a preset is, one saved setup of one effect; the open question about Sync is topic I's.
@@END

@@ASSUME C3-8
ABOUT: 223 R157
TEXT: I assume that until you have picked in the small window about two versions of a preset, saving the show keeps the show's own version in the show file, and a window closed without a pick comes again at that show's next opening.
WHY: His "b" (BF249) has him asked before one of two versions goes; a Save that wrote only the computer's presets would lose the show's version before he has picked. That the window comes again follows from it.
ALT: b) The window cannot be closed without a pick.
IF-WRONG: SMALL what a Save writes for one preset; nothing is lost either way.
ASK: NO technical: it only makes his answer hold through a Save.
@@END

@@ASSUME C3-9
ABOUT: 223 R157
TEXT: I assume a snapshot is compared with the computer's presets like a show when it is opened, and that opening a recording in Studio compares nothing: it adds no preset to this computer and never brings the small window.
WHY: BF249 "b" brings a window into the compare at opening; his words on presets name the show file only. A snapshot holds a whole show (BF243); a show recording has its recorded show file. Which of them is compared is Harmony's reading.
ALT: b) A recording opened in Studio is compared like a show and can bring the window.
IF-WRONG: SMALL where one compare runs; nothing is lost either way.
ASK: NO technical and rare: a snapshot taken on this computer counts as a show this computer saved, so no window comes for it; a recording plays from the values it holds.
@@END

## TOPIC D -- Actions in a show
### The topic ruling's notes for the page ruling
- MUST BE PUT TO HIM, heaviest first: D3-1 (what the tempo stop and Stop actions do to the action buttons, those of clips above all; one question for both stops; its way c is page 2's way b of item 226, kept because he gave no letter). Then D3-5 (the Clear button and the layer's own actions), right after D3-1: it follows the stop, and if he answers D3-1 with c its default falls back to its way b.
- ONE LINE EACH: D3-3 (a clip's play-once action that is switched on under a playing clip waits for that clip's next trigger: his "only"), D3-2 (on a layer or on global a play-once action's button goes off by itself).
- D3-5's DEFAULT WAS FLIPPED by this ruling against the paper and the checker (both had "the layer's actions stay on", LINE): R161 and question 180 of the first page told him the X follows the stop. If the page ruling prefers the paper's default, TEXT and way b of D3-5 change places, and the NEW lines of AMEND 180 4 and AMEND R161 2 with them; the question is YES either way.
- TO SAY ONCE MORE IN ONE LINE: the tempo stop switches actions off -- "tempo stop stops all actions, not just global" (BF250), written under an item that switches them off -- against the default he accepted on 2026-10-07 (the buttons stay on and the actions start again with the beat). The preview by name off the beat (BF247 against "it is triggered on the 1", BF175) is topic B's line.
- TOPIC A: its ruling already points R176 (a) and R214 (a) to item 226 for what a stop does to actions (its AMEND R176 1 and AMEND R214 2); nothing more is owed there. How often the app may move the "1" (BF246; A's A3-1 and A3-2) decides how often running actions glide (D3-6); A leaves to this topic how actions take a "1" that moved, and AMEND R181 2 says it. Item 249's tenth of a beat, which A's ruling also gives an empty cell, is now used for actions too (AMEND R158 3, D3-9).
- TOPIC B: R158 (d) and (e) now speak of the output only; a previewed clip's actions are B's (items 220 and 221, B3-3). What a preview by name shows after a stop hangs on D3-1: if every clip's buttons go off, a clip previewed after a stop is shown without its actions. B's ruling asks for one clause on this in D's question; it is left to the page ruling (D3-1's TEXT stands at 41 words).
- TOPIC E: its ruling makes opening Studio a full tempo stop (its AMEND R165 2; its E3-6, NO: nothing comes on again by itself when Studio is closed) and leaves the action buttons to this topic. With D3-1 at its default, every opening of Studio darkens every action button of the live show, those of every clip too, also when he opens Studio only to cut one action. So D3-1 decides E3-6 with it: if D3-1 stays at its default, E3-6 is no longer a plain NO (advice: closing Studio brings the action buttons back as they were). Item 230 changes no block here.
- TOPIC F: a snapshot opened in the middle of an action (item 236) is the tempo-pause state for actions (R158 e: an action holds its place and carries on at tempo play): that is the action "taken up part-way" that F asks for, and R132 stands (a control that an action moves is saved at the value he set). F's item 234 ends a clip's actions "as at an X": read that for the clip's own actions only, because by D3-5's default the X also switches the layer's own actions off and an edit does not (AMEND R161 3).
- TOPICS J, K, C, X: J's R199 (d) pointer is answered by items 226 and 224, and a key or pad on an action's button stays with the action (J's own line). K: a trigger by the Autopilot plays a clip's play-once action (now named in item 227 c). C: its Save in the header after a load under an action fits item 228. X: its own paper closed X-27 and X-3 by items 224 and 225 and amended its C9; nothing is owed (checker F6).
- NAMES: in everything he reads the layer's X is "the Clear button (the X)". The row Ignore Actions of NAMES.md loses "(and on a layer)". "Ignore Global Actions" is still Harmony's pick for the name of the layer's switch; what the switch does is his by item 225.
- A STALE NOTE, not a rule: the TODAY line of old item 193 ("Keying and its slider are being removed") is wrong after BF260 ("All the blend modes and keying stay"); topic I owns it.
- TOPIC D AFTER THIS RULING: 10 assumptions -- 2 YES (D3-1, D3-5), 2 LINE (D3-3, D3-2), 6 NO (D3-4, D3-6, D3-7, D3-8, D3-9, D-31). No item of the topic is OPEN; no answer is owed. The 16 other old assumptions that he never saw (D-13 to D-30) were tested again against his 33 boxes and the 57 items: none is settled or contradicted, so none is written again.
STAMP: 2026-10-09 19:33:47 EDT
STATUS: DONE

### CONFLICTS (his newest words against earlier ones)
- The tempo stop and the action buttons. New: "tempo stop stops all actions, not just global" (BF250), written under an item that switches the actions off. Earlier: the default A of question 180 (the buttons stay ON, the actions start again with the beat), which he accepted with "All defaults good except for these." (binding-decisions.md:1105; the rule as applied: binding-decisions.md:1249). His own earlier sentence already pointed the new way: "The tempo stop button stops all actions as well as everything else." (binding-decisions.md:1155). The newest words are taken; the clips' buttons are asked (D3-1).
- A preview by name and the beat (topic B carries it; it reaches R158 g here). New: "Previewing a clip should not happen on the beat. It should just be quick" (BF247). Earlier: "a previewed clip (clicking its name) shows the clip with all its actions and it is triggered on the 1" (his L36 of 2026-10-07, binding-decisions.md:1134). The newest words are taken.
- 228: no words of his are overturned. His "b" (BF252) goes against Harmony's recommendation in the answer above the item, not against anything he said.

### REACHES OTHER TOPICS
- A (items R176 a, R177 a and the tempo stop wherever it is ruled): "stop also stops all actions" must now read: the tempo stop switches every action off, clip, layer and global (BF250, item 226); nothing starts again by itself with the beat. Open part: the clips' buttons (D3-1). Topic A's Link rule (the stop takes the clips off, the beat runs on): actions are switched off there too (D3-7).
- A: BF246 (the app finds the 1 by itself) is applied here only as: an action's place follows a 1 that the app moved, as at a Resync (R181, D3-6). Topic A rules when the app may move it.
- J (R199 d): its pointer "What a stop leaves of the action buttons ... is topic D's (D-3, D-1)" is answered: both stops switch every action off, values glide back over the one Global glide (items 226, 224). J (R199 a, and R134 here: "a key or pad put on an action stays with that action"): BF262 gives a MIDI knob on a clip's slider to the CELL; whether a key or pad on a clip's action button also belongs to the cell is not said by him and is J's to rule or ask. Not amended here.
- J and K: their reading of L7 (a layer that ignores every action) falls with item 225 accepted: one switch, against global actions only.
- E (R165, "Opening the review screen ... is like a press on stop for the live show" and "Closing it brings back the live window with the show as it was"): if opening Studio counts as a tempo stop, every action button of the live show goes off (item 226) and the show is not "as it was". Topic E must say which holds.
- B (item 220, BF247): applied here to R158 g only as a pointer: a preview by name is off the beat; whether a previewed clip's actions play in the preview, and from when, is B's. For B to know: on the output, an action of a Timeline mode clip still starts on the next 1 (R158 b).
- C (presets): after a preset is loaded under an action (item 228, BF252) the effect differs from that preset on the sliders the action held; whether the effect shows as changed from its preset, and what a save of the preset then writes for those sliders, is C's.
- F (BF256): applied here to R161 only: a clip deleted or pasted over leaves its layer, so its actions end. Whether the layer then plays the pasted clip, and a cut, are F's.
- H (item 266, accepted): a trigger of the playing clip restarts it; used in item 227 c as a re-trigger that plays a clip's play-once action again.
- F (R188 a, what a show holds): "each layer's switch against actions (L7, L73: one switch or two is topic D's)" is settled: one switch, Ignore Global Actions (item 225); one Global glide value (item 224).

### NOT DONE / UNSURE
- D3-1 is the one real doubt of this topic: whether a clip's action buttons go off at a stop. The RULE takes the widest reading of "all actions"; his earlier sentence "If the actions are toggled on all those actions will play when the clip plays in time with the clip." (binding-decisions.md:981) points to way b. One question on his next page settles it, for the tempo stop and for Stop actions together.
- Stop actions and the clips' buttons: R224 c already switches off "every action of every clip". If he answers D3-1 with way b, R224 c needs the same change for Stop actions, or the two stops differ; the question as put covers both.
- BF256 "If a clip is playing, and I change the deck, that does not change the clip" fits R161. It was not read as going against his 182 b (a running layer action takes the cell of the deck on screen at its NEXT trigger, so the picture can change one trigger after a deck switch). If he meant more, 182 would have to be put to him again; cheapest: one line on his next page only if topic F sees the same doubt.
- Where the old blocks of this topic say "the review screen" or "Review" (R129, R131 d, R162, R181 e, R143, D13, R224 a, the NAME "action save window"), only R224 was amended, with one sentence that covers them all; a merge script may want to replace the words everywhere.
- The old word "fire / fires" in the old RULE lines is the retired word for "trigger" (NAMES.md); not amended, it changes no rule.
- Every TODAY line is taken from the old blocks and the two fact sheets; no program text was read for this paper, and nothing was run.
STAMP: 2026-10-09 18:49:52 EDT

### The assumptions of this topic after the ruling (YES first, then LINE, then NO)
@@ASSUME D3-1
ABOUT: 226, 180, R161, R224
TEXT: I assume the tempo stop and Stop actions both switch every action button off: on every layer, on global and on every clip, also on clips that are not playing. After a stop you switch on again the ones you want.
WHY: BF250 says "all actions, not just global" and gives no letter; with "Stop removes all clips" (BF271) a clip's actions stop anyway, and what happens to their buttons is not said. "stops" also fits way c for a layer's and a global action: on the first page he took the default that keeps the buttons on. His words on a clip's buttons point to way b: "If the actions are toggled on all those actions will play when the clip plays" (2026-10-04) and "It's button stays on" (BF251). One question for both stops, because Stop actions is ruled with the same words (R224 c). The answer reaches further than the two buttons: topic E makes opening Studio a full tempo stop, and in topic B a clip previewed after a stop is shown without its actions once their buttons are off.
ALT: b) Only layer and global action buttons go off. A clip's action buttons stay as you set them: its actions stop, and play again the next time that clip is triggered. c) At the tempo stop no button goes off: every action waits and starts again from its beginning when the beat next runs.
IF-WRONG: STAGE after every stop, and after every opening of Studio (topic E), the action buttons of every clip in the show are dark and must be switched on again by hand, and a show saved then is saved that way; or they are not.
ASK: YES his words give the two stops their reach, not what they do to a clip's buttons; two sentences of his pull two ways, and he would meet the difference at every stop between two songs.
@@END

@@ASSUME D3-5
ABOUT: 180, R161, 226
TEXT: I assume a layer's Clear button (the X) does for its layer what the tempo stop does for the show: the clip goes and that layer's own actions are switched off. A clip that ejects, or an empty cell you trigger, leaves them on.
WHY: BF250 speaks of the tempo stop only. The first page told him that the X follows his answer on the stop (the page's words: "with B the X switches them off"), and the stop now switches actions off. His "143 a" (2026-10-04: a layer's action runs on across clip changes) points to way b for the X as well.
ALT: b) Clear takes only the clip off: the layer's own actions stay on and keep playing, and one that triggers clips puts a clip on that layer again at its next trigger.
IF-WRONG: STAGE a layer he cleared is filled again by its own action, or its action buttons are dark after every Clear.
ASK: YES no words of his say what Clear does to a layer's actions; the first page and his 143 a point two ways, and the audience sees the difference.
@@END

@@ASSUME D3-2
ABOUT: 227, R172, R181
TEXT: I assume your answer is about a clip's action. A play-once action on a layer or on global plays once and its button then goes off by itself; one press plays it once more.
WHY: BF251 says "neither" and then speaks of "that clip". "neither" rejects ways b and c and leaves the item's own sentence on a layer or on global standing; way c said the button "always stays on", and he did not take it. Only "It's button stays on", if he meant it for every action, would go against it.
ALT: b) Its button stays on there too; to play it again you switch it off and on. c) On a layer its button stays on and it plays again each time a clip is triggered on that layer.
IF-WRONG: STAGE one press or two to play a one-time move again; a lit or a dark button after it.
ASK: LINE he read this sentence in the item and rejected only the two other ways; he can strike it.
@@END

@@ASSUME D3-3
ABOUT: 227, R158
TEXT: I assume only a trigger of its clip plays a clip's play-once action, as you wrote. Switched on while the clip is already playing, it waits for that clip's next trigger; switching it off and on does not play it.
WHY: BF251 "plays again only when that clip is re-triggered", written in the place of way c ("only after you switch it off and on"). A first switching-on under a playing clip is not named by it, and the old timing rule would start it on the next 1.
ALT: b) Switching it on while its clip plays also plays it once, from the next 1.
IF-WRONG: STAGE a one-time move he switches on in the middle of a clip comes at the next bar, or not until he triggers the clip again.
ASK: LINE his own "only" taken one step further; the other way is the timing rule of every other action, and he can strike this with one letter.
@@END

@@ASSUME D3-4
ABOUT: 227
TEXT: I assume triggering a clip again while its play-once action is still in its one pass starts that action over from its beginning, together with the clip.
WHY: BF251 gives the re-trigger as what plays it again; a re-trigger that comes before the pass is over is not named.
ALT: b) The running pass finishes first, and that trigger does not count.
IF-WRONG: SMALL
ASK: NO the natural result of his sentence and of the accepted rule that a new trigger restarts the playing clip (item 266).
@@END

@@ASSUME D3-6
ABOUT: R181, 224
TEXT: I assume that when the app moves the 1 in automatic mode, actions follow the new 1 as after your own Resync: their sliders glide to the new place over the Global glide time, and a clip's actions stay in step with their clip.
WHY: BF246 lets the app place the 1; item 224 names a Resync as a cause of a glide, not a 1 that the app moved. Topic A's paper lets a playing BPM mode clip fall into step only on the next 1; its own actions must not part from it.
ALT: none
IF-WRONG: SMALL
ASK: NO actions sit on the beat; a 1 that moves takes them along whoever moved it. How often the app may move the 1 is topic A's.
@@END

@@ASSUME D3-7
ABOUT: 226
TEXT: I assume the tempo stop switches every action off also while Ableton Link keeps the beat running.
WHY: BF250 says the tempo stop stops all actions; with Link on the stop takes the clips off and the beat runs on (topic A).
ALT: none
IF-WRONG: SMALL
ASK: NO Link is off unless he switches it on, and his sentence has no exception.
@@END

@@ASSUME D-31
ABOUT: R172, 227
TEXT: I assume un-ticking Loop while an action plays lets it finish its pass and then go back. Ticking Loop on a play-once action lets it loop on; if its one pass is already over, it starts looping on the next 1.
WHY: L64 gives the toggle and its two states, not a change in the middle; BF251 adds a clip's play-once action that waits with its button on.
ALT: b) Un-ticking Loop ends the action at once.
IF-WRONG: SMALL
ASK: NO the natural result of his two states.
@@END

@@ASSUME D3-8
ABOUT: 228, R160, R129
TEXT: I assume a preset loads plainly onto a slider whose only action is a layer action that a global action is holding back at that moment; the preset's value is then the value that slider goes back to later.
WHY: BF252 "b" speaks of a slider that an action is moving; a layer's action that is held back is on but moves nothing, and no rule names that case.
ALT: none
IF-WRONG: SMALL
ASK: NO a rare edge of his "b"; he would not meet it knowingly.
@@END

@@ASSUME D3-9
ABOUT: R158, R172
TEXT: I assume an action whose start comes within a tenth of a beat after the 1 starts at once and in step, as a BPM-mode clip does: with its clip, or when you press its own button a hair late.
WHY: Item 249, accepted as written, gives this to a BPM-mode clip triggered just after the 1; an action is not named, and the timing rule would make it wait for the next 1.
ALT: b) Only the clip starts at once; an action waits for the next 1.
IF-WRONG: SMALL
ASK: NO the same tolerance he accepted for a clip (item 249), by his rule that an explanation given once answers the repeats.
@@END

## TOPIC E -- Studio and the recordings
### The topic ruling's notes for the page ruling
- MUST be put to him, in this order of weight: (1) E-23 (YES): does Studio play the low-resolution show recording together with its own picture; his BF253 gives the film this purpose and no word says how the two are compared. (2) E3-1 (LINE): what "the output monitor" names, and with it the size; print it as the choice under the answer lowres (as 262 stood under that answer on page 2); item 262 stays OPEN on it. (3) E3-3 (LINE): the Video box of Record Show. (4) E3-6 (LINE): opening Studio leaves the action buttons as he set them.
- E3-1 is a LINE and not a YES although his box asks back: by the three tests a wrong size is one number and is never seen on stage (his own "This is just for reference.", BF241). It must still be printed; do not let it fall with the internal ones.
- E3-3 and topic J's J-1 are ONE question with opposite texts: print E3-3 only. His explicit answer is "48 b" (binding-decisions.md:867: three boxes, Video among them), so "the box stays" is the text and "the box goes" is way b; J-1 as written would flip an answer he gave. Topic J's ruling should drop J-1, or the page ruling drops it.
- Not for him (ASK NO): E3-4, E3-5, E3-7, and the old E-30 and E-32, both written again; E3-2 is dropped (his BF241 settles the range of the rate). The other old internal assumptions of this topic are untouched.
- Newest words against earlier ones: nothing of this topic has to be said to him again as a conflict. BF268 (no see-through parts) goes only against a "yes" he gave for the one-layer recording, which he removed himself; BF254 "c" goes against no word of his (it agrees with his "always work with multiples of 4", binding-decisions.md:812); BF253 "It has no sound." goes only against the page's own text. The size (BF241 against the quarter he left in 262 and his earlier "1/4 or 1/8 the size") is the line E3-1.
- Depends on topic A: what the tempo stop and the tempo pause do. AMEND R165 2, item 259, the last sentence of item 232 and the old E-7 and E-30 lean on A's reading that the stop takes every clip off every layer at once and that a pause holds what follows the beat; if A's ruling changes either, re-read them. Item 232 also leans on A for when the 1 can move (BF246).
- Depends on topic D: E3-6 hangs on D's question on what a tempo stop leaves of the action buttons (D3-1): print E3-6 beside it or fold it into it; if no button goes off at a stop, E3-6 falls away. Topic D's ruling advises the text E3-6 has. How "the same clip" is recognised is one rule with item 230.
- Depends on topic K: the audio file under a stop (the silence in item 259; AMEND R165 2); Render (old item 215 f): if he takes way b of E3-3, Render is the only way to a full-quality film and moves up in K's order.
- Depends on topic H: the test of codecs (the codec of the low-resolution show recording and of a recorded clip; the 30-or-15 test can run in the same session); ONE cut rule for items 232 and 238 (multiples of 4 bars, the clip out point moved in, nothing taken out of the file); how far a clip out point can be dragged out again.
- Depends on topic J: the name Studio in every old rule of this topic (one replacement at the merge; "Studio picture" is Harmony's pick); the knob that belongs to a cell (BF262) and both kinds of knob (BF270) inside Record Over (item 257, E3-7, E-32). If J's research gives pads a hold setting, a show recording keeps presses and releases like any button (old item D17); nothing changes here.
- Topic F: its ruling in progress keeps a picture in the Snapshot and lets old item R187 (d) of this topic stand for it (rule-F.md, its F15), so nothing is amended here; it also has Snapshot do nothing while Studio is on screen (its F3-13), which agrees with item 257 and E3-7. F may still want one mechanism for a Snapshot and for the recorded show file (old item R173, old assumption E-34).
- If he answers E3-1 with c (an output screen's own size), items 231 and 262, old item G4 and E3-3 are re-read together: the film is then no longer small, and the Video box is its twin. If he answers E-23 with b, points (2) and (3) of "Harmony's own" in item 231 fall away without harm.
- For the build session, not for him: the test on his Mac with a heavy show still gates "on by default" (item 231, point 5). He wrote "It defaults to running" without the page's condition; if the test fails he is told before anything is decided.
- For Harmony, on the workflow and not on the topic: the lint reads the whole file, but the merge takes a ruling's blocks only between "## REPLACEMENT BLOCKS" and the next heading; a ruling that appends blocks after "## FOR THE PAGE RULING" passes the lint and loses every block at the merge. Read-only check at 2026-10-09 19:32:21 EDT: the eight ruling files then present (A to H) kept all their blocks inside the section; compare each topic's "replaced / new / dropped" in the merge report with its lint line before the spec is laid over.

### CONFLICTS (his newest words against earlier ones)
- The size of the low-resolution show recording. New, BF241: "Maybe something close to the output monitor resolution?" against item 262, whose box he left empty in the same sitting = accepted as written: "a quarter as wide and as high as the show (480 by 270 for full HD)", and against his earlier "Maybe something like 1/4 or 1/8 the size so it is very minimal to use as a double check." (binding-decisions.md:1112, BF161). Not picked: assumption E3-1, asked.
- The sound of the low-resolution show recording. New, BF253: "It has no sound." against the page's own text for item 231 ("with sound") and the old ruling in G4 (sound when the Audio box is ticked). No earlier word of his said it has sound; his newest words win; no question.
- Where a recorded clip ends. New, BF254: "c" (loops only whole groups of 4 bars; the rest stays in the file) against the ruling over all topics of 2026-10-08 that read his "keep recording till it is on an even grid line like the end of the bar. It can be trimmed later if necessary." (binding-decisions.md:1170) to the letter (5 bars stay 5). His letter wins, and it agrees with his earlier "always work with multiples of 4. If it is uneven, then move the outpoint in" (binding-decisions.md:812). No question.
- See-through parts of a recorded clip. New, BF268: "This is exactly the output recorded as one layer. Whatever the output is displaying is what this should look like." against his earlier "yes" to see-through parts, given for a one-layer recording (binding-decisions.md:671 as cited by old item R186), and in line with "The recording should look exactly like the output." (binding-decisions.md:626). The newer words win; no question.
- The purpose of the low-resolution show recording. New, BF253: "so that we can see if there is a mistake in the parameter recording" beside his earlier "so that they are small and if something happens most of the recording is not lost" (binding-decisions.md:1112). Read as two purposes that both hold (the pieces stay); no question.

### REACHES OTHER TOPICS
- K (old item 215 f, assumption K-2): BF241 "If we want a very hd recording, we can make an HD render from the Recording Review" confirms Render as a function of Studio and gives it its job, the film in full quality. It is not a new function: 215 (f) already rules it (the whole recording, at the full size of the show, with its sound, not in real time). "very hd" is read as that full size and no larger. K's paper applies it to 215 (f); this paper amends only R184 and asks about the Video box (E3-3).
- H (the test of codecs after the build, BF240): BF241 "we should test which codecs work best" puts the codec of the low-resolution show recording, and with D22 the file types of a film and of a recorded clip, into that test. BF268 adds one fact for it: a recorded clip needs no see-through channel. BF241 also puts a second question to a test after the build, 30 or 15 pictures a second, which is a load test of this topic (E3-2) and can be run in the same session.
- H (item 238, BF258 "b"): a clip brought in in BPM mode is cut in to whole groups of 4 bars; BF254 "c" gives a recorded clip the same rule (loops whole groups of 4 bars). One rule in both papers.
- J (the name, item 245, BF245, BF263): every old RULE of spec-E says "the review screen" or "Review"; it reads Studio from now on. Not amended line by line here; one global replacement when the specs are merged. "Review picture" becomes Studio picture (Harmony's pick, above).
- A (BF271 "Stop removes all clips from all layers", box 273): with it a tempo stop leaves nothing to record for record to clip; old assumption E-7 (a tempo stop ends a clip recording and keeps it; a pause holds it) is not contradicted and stays as it is. If A's paper rules stop or pause otherwise, E-7 and item 259 need a second look.
- D (item 230): how "the same clip" is recognised after it moved, and where an action is stored, must be one rule in both papers (unchanged from the old spec).
- F (BF243, the snapshot as "a save of the show exactly where it is"): the recorded show file (old item R173, assumption E-34) is the same kind of save taken at the record show press; F's paper may want one mechanism for both. Nothing changes here.
- Applied here from another topic's box: BF245 / BF263 (Studio); BF240 (the test of codecs, into D22); BF258 (item 238 b, as agreement with 232 c).

### NOT DONE / UNSURE
- What "the output monitor resolution" names (BF241): NAMES.md gives "output monitor" as the monitor inside the app (his words of 2026-10-05), and his R138 line uses "monitor" for a connected screen (binding-decisions.md:1161). Not settled by reading; E3-1 asks. The size at which the output monitor is drawn in the app was not read from the program text, so the @@ANSWER says "I take it to be near" and claims no number for it. Cheapest: his answer to E3-1; and one look at the monitor panel's size before the page is printed, so that the sentence can be firmer or cut.
- "the output" in BF268 is read as the picture (the output monitor), not one output screen with its own corrections; his earlier words carry that reading (binding-decisions.md:1170). Not asked.
- "the parameter recording" in BF253 is read as the recorded moves (the Parameters box). Not asked.
- Whether the test "light enough" still gates the default: he wrote "It defaults to running" without the page's condition. Kept as Harmony's safety from G4 (he is told before anything is decided if the test shows a stutter). If the page ruling wants it said to him, it is one line.
- Render's details (whole recording or the stretch between In and Out; its size) are K's assumption K-2; nothing new is assumed here beyond E3-3.
- R183 (d) "with no box ticked the button is greyed": whether the low-resolution box alone counts as a ticked box was not ruled (an edge; E3-4 covers its neighbour).
- Old assumptions tested and left untouched: E-4, E-7, E-12, E-13, E-15, E-16, E-18, E-19, E-24, E-28, E-30 (agrees with 232 c now), E-31, E-32, E-33, E-34, E-36, E-38. Changed: E-23. Dropped: E-27.
- TODAY lines are taken from the old spec's TODAY lines; nothing was re-derived from the program text and nothing was run.
- Written 2026-10-09 18:49:14 EDT.

### The assumptions of this topic after the ruling (YES first, then LINE, then NO)
@@ASSUME E-23
ABOUT: 231, G4, D16
TEXT: I assume Studio can play the low-resolution show recording together with its own picture, both always at the same moment of the show, also while you scrub, so that a mistake in the parameter recording shows at once.
WHY: BF253 says what it is for ("so that we can see if there is a mistake in the parameter recording"), not how the two are compared; no word of his says whether Studio shows the film.
ALT: b) Studio does not show it: the pieces are plain film files that you open in any player and compare by eye.
IF-WRONG: REBUILD a second picture in Studio that follows the playhead is a piece of work of its own; without it the check is done by hand, piece by piece.
ASK: YES his new words give the film its purpose, and whether Studio serves that purpose is a function he would look for there.
@@END

@@ASSUME E3-1
ABOUT: 262, 231, G4, answer lowres
TEXT: I assume you mean the output monitor inside the app, not an output screen such as the projector: so the low-resolution show recording stays small, a quarter as wide and as high as the show (480 by 270 for full HD).
WHY: He left 262 empty (a quarter) and asks in BF241 "Maybe something close to the output monitor resolution?"; the name fits the monitor in the app or a connected screen; his next sentences in the same box ("This is just for reference. If we want a very hd recording, we can make an HD render from the Recording Review") speak for a small film. The monitor in the app has no fixed size, so no number is read from it.
ALT: b) Larger: half as wide and as high as the show (960 by 540 for full HD). c) You mean the output screen's own size: then it is a full-size film, heavier, and no longer small.
IF-WRONG: SMALL one number; the full-size way is the heavy one and is not taken without his word.
ASK: LINE his question in the box must get this line, as the choice under the answer on this recording; but by his own "This is just for reference." a wrong size is never seen on stage and is changed in a moment.
@@END

@@ASSUME E3-3
ABOUT: R183, R184, 215
TEXT: I assume the Video box of Record Show stays, as you answered before: ticked, it writes a full-size film while the show runs. Render in Studio makes such a film afterwards.
WHY: His answer "48 b" (binding-decisions.md:867) gave Record Show three boxes, Video among them; BF241 "If we want a very hd recording, we can make an HD render from the Recording Review" names Render as a way to a full-quality film and does not say that the box goes.
ALT: b) The Video box goes: during a show only the low-resolution show recording is filmed, and a full-quality film is always made afterwards with Render.
IF-WRONG: SMALL one box more or less; way b spares the heaviest of the recordings during a show.
ASK: LINE his earlier answer is the text; his new words may make the box unneeded, and he can strike it in a second. Print it once: topic J's J-1 is the same question with the opposite text.
@@END

@@ASSUME E3-6
ABOUT: R165, 226
TEXT: I assume opening Studio stops everything that plays (the clips leave the layers, the tempo, the actions and an audio file stop) but leaves your action buttons as you set them: when you close Studio the show is as it was, with nothing playing.
WHY: Old item R165, which he read and left, says both that opening it "is like a press on stop" and that closing it brings back "the show as it was"; under page item 226 (BF250 "tempo stop stops all actions, not just global") a tempo stop may switch every action button off, so both cannot hold. The sentence on closing is kept; topic D's ruling advises the same.
ALT: b) Opening Studio is a tempo stop in everything: the action buttons go off as at any stop, and you switch on again the ones you want.
IF-WRONG: SMALL what the opening does to the buttons can be changed without touching Studio.
ASK: LINE two sentences he read pull two ways once a stop switches buttons off, and he would see the difference each time he comes back from Studio; it hangs on his answer to topic D's question on the stop and falls away if no button goes off at a stop.
@@END

@@ASSUME E3-4
ABOUT: 231, R183
TEXT: I assume the low-resolution show recording is written with every record show for which its box is ticked, also when the Parameters box is not.
WHY: BF253 "next to the show recording"; its purpose is to check the parameter recording, yet his first words (BF161) also wanted it against a lost recording.
ALT: b) It is written only when the Parameters box is ticked.
IF-WRONG: SMALL a rare combination of boxes.
ASK: NO an edge of the boxes; neither way loses anything he ticked.
@@END

@@ASSUME E3-5
ABOUT: 257, 195
TEXT: I assume a clip you trigger during a Record Over is added to the clip triggers already recorded; the recorded ones still play.
WHY: Item 257 (accepted) says clip triggers record over too, not whether they join or replace the recorded ones.
ALT: b) From your first trigger on a layer, that layer's recorded triggers are dropped until you stop Record Over.
IF-WRONG: SMALL seen only in Studio, and the original recording is never changed.
ASK: NO it was in the text of the assumption behind item 257; a Record Over can be done again.
@@END

@@ASSUME E3-7
ABOUT: 257, 195, R185
TEXT: I assume Record Over works with the keyboard and MIDI mapping that the show recording's own show file holds, and that the tempo bar's keys and pads and the recording buttons do nothing while Studio is open: the recorded tempo stays.
WHY: Item 257 (accepted) says "everything in the keyboard and MIDI mapping"; no word says whose mapping works in Studio, which works on the recording's own copy of the show (old items R165, R173); old item R138, which he read, says "The tempo row's buttons are not on this screen", and the recorded tempo is the grid of the recording (old items R167, R185 b).
ALT: b) The mapping of the show that is open in the live window is used. c) The tempo bar's keys and pads record over too.
IF-WRONG: SMALL which mapping is read is one choice, seen only in Studio.
ASK: NO technical; it follows the rule that Studio works on the recording's own copy of the show.
@@END

@@ASSUME E-30
ABOUT: R187, R186, 232
TEXT: I assume a record to clip cut short (a quit, a full disk, a tempo stop, its own stop pressed in a tempo pause) ends at once and is kept: out point at the last full group of 4 bars, a shorter one whole.
WHY: L88's "keep recording till it is on an even grid line" cannot hold at a quit, a full disk or a tempo stop, nor while a tempo pause holds the recording (old assumption E-7): the bar does not end. Under 4 bars is not covered. It agrees with item 232, way c (BF254).
ALT: b) With fewer than 4 bars recorded nothing is kept, and the app says so.
IF-WRONG: SMALL a rare edge.
ASK: NO a technical edge; the default never throws a recording away.
@@END

@@ASSUME E-32
ABOUT: 195, R185, 257
TEXT: I assume a knob on the controller counts as let go once it has stood still for a quarter of a second, as everywhere else in the app, whether it sends its position or is endless; then the recorded moves take over again.
WHY: His words: "will snap back to the recorded track as soon as it is let go" (binding-decisions.md:340-341); a knob cannot tell the app that a hand has left it. The mapping rule he read gives a quarter of a second (old item R199 e), and BF270 wants both kinds of knob to work; "about one beat", this assumption's earlier number, is given up so that there is one number.
ALT: b) Once you move a control in Record Over it stays yours until you stop Record Over.
IF-WRONG: SMALL one number, tuned at the build.
ASK: NO one number with the mapping rule he read and left.
@@END

### Items of page 2 that are still OPEN in this topic: 262

## TOPIC F -- The show file, decks and saving
### The topic ruling's notes for the page ruling
- MUST GO TO HIM, one card: F3-6 (YES). What a snapshot keeps beyond a Save (a picture, what was playing) and what opening one later does. It is the large part of the Snapshot build, and ways b and c are other builds. Nothing else in F is YES.
- THEN THREE LINES, by weight: (1) F3-1: a click on a clip triggers and selects it, so Delete, Cut and Paste then work on the clip that is playing, and by BF256 that takes it off its layer; keep that consequence in the line, as written. (2) F3-3: the layer empties at once, also under a BPM-mode clip; the pasted clip waits. (3) F3-4: Remove Column and Remove Deck count as deleting their clips.
- EVERYTHING ELSE IN F IS NO: Snapshot's picture (told inside F3-6), its key (none until he maps it: F3-13), where snapshots are kept (F3-9), sliders as set (F3-11), an opened snapshot as a show without a file (F3-14), the plain drag (F3-15), and F3-2, F3-10, F3-12, F-2, F-20. Dropped, because his words settle them: F3-5 and F3-8.
- ONE RULE FOR THE PAD: whether a key or a pad that triggers a cell also selects it is J3-10 (topic J says yes, ASK NO); this paper no longer rules it. The two papers had read it in opposite ways, and with F3-1 the selected clip is what Delete works on: J3-10 should be a LINE at least, best folded into F3-1's line ("a key or a pad on the cell does the same").
- NEWEST WORDS AGAINST EARLIER ONES: none needs a line of its own. (a) Snapshot is a save apart from the show's Save, against "There's no reason to save them separately" (2026-10-04, binding-decisions.md:874): F3-6 says it. (b) An edit takes a playing clip off its layer (BF256), against two defaults he had let stand (R190 (b); Q1 of 2026-10-02, binding-decisions.md:611): F3-3 and F3-4 say it. (c) A snapshot comes back with its clips, against 196 A: F3-6, with "the layers empty" as its way b.
- DEPENDS ON A: "at once" is the way of the layer's X (old R168, its assumption A-5). If A rules that a layer under a BPM-mode clip only ever goes empty on the 1, F3-3 way b becomes the rule and item 234 (1) follows it. A snapshot's opening needs the state "tempo paused, clips on their layers" (BF171 gives it) and a one-time hold of the clips that are not in BPM mode.
- DEPENDS ON D: a clip that leaves its layer by an edit ends its actions as at an X (D's R161 as amended there); a snapshot that keeps "which actions run and how far" needs an action that can be taken up part-way. D asked whether BF256's deck sentence goes against his 182 b: F reads it as his rule of 2026-10-02 said again; no line.
- DEPENDS ON J: Snapshot is a button of the mapping (J's AMEND 206 2) and comes with no key (R227 (c) stands); J's R198 and G2 (9) as amended must carry the same Undo sentence.
- G, E, K, C: the snapshot's picture is the "snapshots" of R226 (b) and R187 (d), which stand; opening a snapshot is the opening of a show for the outputs (237, 264, 265 as written). K: a snapshot does not hold the audio input. C: a snapshot carries the presets as a show does, so the window of item 223 can come when one is opened; R189 (h) now leaves that window at launch to C. E: Snapshot does nothing in Studio (F3-13's way c would be a new function of Studio).
- I: G2 (2) and R188 (b) now carry the clip's own envelopes of both types (BF261) by pointing at I's item 241; if I's ruling changes who owns an envelope, both follow it.
- NAMES: the list must change Snapshot (a save of the show with its picture, not a still picture), layer strip (his word: BF256), layout (the live window, Studio) and the Output menu's line on Snapshot. On his page the words "safety copy" must not stand for two things: in item 235 they meant the copies in the backups folder, in item 198 the copy written every few minutes.
- STILL OPEN, small: how a snapshot is deleted (laid out with its list). The missing-signal rule of G2 (7) now follows topic C's R180 (a): if C's ruling changes that rule, G2 (7) follows.

Written 2026-10-09 19:30:30 by the ruling architect for topic F (read-only; HEAD 33989a8; nothing built, run or launched).
STATUS: DONE

### CONFLICTS (his newest words against earlier ones)
- Snapshot, a separate save. New (BF243): "Perhaps a good snapshot is a save of the show exactly where it is with all the settings and the output and everything so if there is a cool inspiring moment, that happened, the user can push a shortcut button and save that to look out later". Earlier, on a list that named "a snapshot" among nine kinds of saving (binding-decisions.md:874-876): "All of these things should be saved when a show is saved. There's no reason to save them separately:". The newest words win: a snapshot is a save of its own, beside the show's one Save. One line for him: "Snapshot is now the one thing saved apart from the show's Save."
- Snapshot against the answer he was given. Harmony's answer on page 2 kept Snapshot "only as one small command that saves a still picture of the output"; his BF243 and BF257 ("We keep the safe snapshot somewhere. Earlier in this document, I explained what a snapshot exactly is.") replace it. Not a conflict of his own words; listed so that the still-picture reading is not built.
- An edit and the playing clip. New (BF256): "Pasting over a clip or deleting a clip, removes it from the layer strip, and it does not play." Earlier he had only let stand, unanswered, the reading that an edit of the grid leaves a playing clip playing (old R190 (b), shown to him on page 1; no words of his in binding-decisions.md say it). His Undo rule (binding-decisions.md:688: "Let's not allow control Z to change anything that is live in the layer strip. It changes anything else") is kept and is not a conflict: it is about Undo, and BF256 is about the edit itself (F3-5).

### REACHES OTHER TOPICS
- A (triggering): item 233 leans on A's rules for when a triggered empty cell empties its layer (L11, page item 248) and when a triggered clip starts; item 234 uses A's rule for HOW a layer goes empty, but at once. If A rules that a layer can only go empty on the 1 in BPM mode, F3-3 way c becomes the rule.
- A (tempo): item 236 part (4) opens a snapshot with the tempo PAUSED, because by BF271 ("Stop removes all clips from all layers") a stopped tempo cannot hold clips on layers. A owns what pause holds; a snapshot's restore needs "paused with clips on their layers at a given place in the bar" to be a state A allows.
- B (previews): item 233: the name click on a filled cell previews by B's rule (BF247: quick, not on the beat); the triggering click, which now also selects, must NOT feed the preview monitor (F3-1). B should say so in its own rule for what selecting shows.
- D (actions): item 234: a clip taken off its layer by a paste-over, cut or delete ends its actions as any clip leaving its layer does; BF251's "plays again only when that clip is re-triggered" then holds for the pasted clip's own actions. Item 236 (2b) wants "which actions are running and how far" saved and restored: D owns whether an action can be resumed mid-run.
- D: old items R143 (amended here, named under MADE FROM of 235) and D13 (stands). D-7 is closed by item 235.
- G (outputs): item 236: a snapshot holds which outputs were on, as a show does, and opening one follows G's rules for opening a show (page items 237, 264, and BF269 after All Outputs Off). Syphon in the show (page item 263) is applied in R188 (a).
- J (mapping): BF262 ("b" to item 244) applied in G2 part (2) and old item 197: a MIDI knob on a clip's slider belongs to the cell, so a copied clip carries none. The Snapshot shortcut is one more entry of the mapping (which key: J). Page item 271's reason (no stray hit) does not apply to it.
- J (Edit menu, Undo): G2 part (9) as amended keeps his Undo rule; topic J's Undo reading should carry the same sentence for an Undo that would take away a playing clip (F3-5).
- K (audio): page item 272 applied in R188 (e): the computer remembers the picked audio input. A snapshot holds no sound (F3-11).
- E (Studio, recordings): a snapshot is taken of the LIVE show; whether the shortcut does anything while Studio is on screen is not ruled here (see NOT DONE). A show recording and a snapshot do not depend on each other.
- X / names: NAMES.md's entries Snapshot, layer strip, layout, and "Output menu" (it lists Snapshot as a still-picture entry) need the changes under NAMES.

### NOT DONE / UNSURE
- The other reading of "it does not play" (BF256): "it" could be the PASTED clip ("the pasted clip does not play by itself"). Under that reading the old clip is still removed from the layer strip by the first half of his sentence, so the layer goes empty either way; the two readings give the same rule, and what is left open is asked as F3-3.
- A plain drag (a move, no Option) onto a cell whose clip is playing, and a move of a clip that is itself playing to another layer: not ruled by his words and not by G3 ("this rule adds nothing to it"). Cheapest way: follow F3-3 once he has answered it (a replaced playing clip leaves its layer; a moved clip keeps playing only inside its own layer).
- Snapshot while Studio is on screen, and whether a snapshot press is marked in a running show recording so the moment can be found there: not in his words. Cheapest way: one line on his next page only if E's paper needs it; until then the shortcut acts on the live show only.
- Whether a state "tempo paused, clips on their layers, each at a saved frame, actions part-way" can be restored exactly is a build question (topic A's pause, topic D's actions, topic H's seek to a frame): not checked in the program text. If it cannot, F3-6 way b is what can be built first.
- How a snapshot is deleted and whether its picture can be opened by itself in Finder: left to the layout work; the files are ordinary files in the app's snapshots folder.
- G2 part (7) ties a pasted clip's missing signal to page item 223; his answer there (BF249 "b": a small window asks) is about two versions of one preset, which topic C owns. Whether a clip pasted into another show that has a signal of the same name but another shape asks in the same way is not ruled here.
- Nothing of the app was run or measured for this paper; every TODAY line is from the old spec, a fact sheet or program text read, not run.

Written 2026-10-09 18:50:21 by the architect for topic F (read-only; HEAD 33989a8; nothing built, run or launched).
STATUS: DONE

### The assumptions of this topic after the ruling (YES first, then LINE, then NO)
@@ASSUME F3-6
ABOUT: 236, 196, R188
TEXT: I assume a snapshot keeps a picture of the output and also what was playing. Opening one later takes the place of the open show (after asking to save it) and brings the clips back on their layers, on their frames, with the tempo paused.
WHY: BF243 "a save of the show exactly where it is" and "save that to look out later" do not say whether what plays is kept, nor what opening a snapshot does to the show that is open; "the output" in "with all the settings and the output and everything" can be its picture. Against the default he accepted for a show (196 A: every layer is empty when a show is opened), a snapshot would come back with its clips.
ALT: b) It opens like any show, with the layers empty; you trigger the clips yourself. c) It never replaces the open show: you only look at its picture and take clips or decks from it.
IF-WRONG: REBUILD keeping and restoring what plays is the large part of this build; and he would see at once whether the moment comes back.
ASK: YES no words of his say what "look at later" does, and the three ways are different builds.
@@END

@@ASSUME F3-1
ABOUT: 233, G2
TEXT: I assume a click on a clip, anywhere but its name, triggers it and also selects it: the Clip tab shows it, and Copy, Cut, Paste and Delete then work on that clip.
WHY: BF255 "you can select the cell which triggers it and selects it" stands under an item about empty cells; it does not name a cell that holds a clip. With BF256 ("deleting a clip, removes it from the layer strip, and it does not play") the selected clip is then the one that is playing: a Delete, a Cut or a Paste takes it off its layer.
ALT: b) Only an empty cell is selected by its trigger click; a clip is selected by a click on its name only.
IF-WRONG: STAGE the Clip tab would jump, or not jump, to a clip he triggers; and a stray press of the Delete key would take the clip he has just triggered off the output.
ASK: LINE his sentence is general ("the cell") and he would most likely wave it through; but the Delete key's reach is new and seen on stage, so he gets the line.
@@END

@@ASSUME F3-3
ABOUT: 234, G2, G3
TEXT: I assume the layer goes empty as soon as you paste over, cut or delete the clip it is playing, also when that clip is in BPM mode. The pasted clip waits in its cell until you trigger it.
WHY: BF256 "removes it from the layer strip, and it does not play" names no moment. "it" reads as the clip pasted over; read as the pasted clip, his words settle the second sentence as well. He saw way b of the item ("The layer switches at once to the pasted clip") and wrote his own words in its place.
ALT: b) A clip in BPM mode leaves its layer on the next 1, as when you trigger an empty cell. c) The pasted clip starts playing in its place, as if you had triggered it.
IF-WRONG: STAGE a layer would drop out off the beat, or stay dark where he expected the pasted clip; small to change once built.
ASK: LINE his words settle that the clip leaves the layer strip; the moment and the waiting of the pasted clip are edges he would most likely wave through.
@@END

@@ASSUME F3-4
ABOUT: 234, R190
TEXT: I assume Remove Column and Remove Deck count as deleting their clips: a clip of theirs that is playing leaves its layer at once and no longer plays.
WHY: BF256 speaks of "deleting a clip"; it names neither a column nor a deck that is removed with clips in it. Two earlier defaults, never his words, said the opposite: the page-1 reading R190 (b) ("a clip whose own column is removed plays on until it is replaced") and the default Q1 of 2026-10-02 ("a deleted deck's playing clip keeps playing until replaced", binding-decisions.md:611).
ALT: b) Such a clip plays on until you trigger something else on its layer.
IF-WRONG: STAGE a layer would go dark when a column or a deck is removed in a show.
ASK: LINE it follows his words for a deleted clip, against two defaults he had let stand; one line for both, which he can strike.
@@END

@@ASSUME F3-2
ABOUT: 233
TEXT: I assume an empty cell you click to trigger is selected at the click, also while its layer still waits for the 1 to go empty; and a column trigger or an action that triggers cells selects nothing.
WHY: BF255 says the click "triggers it and selects it", not when the selection lands while the trigger waits for the 1; it speaks of a click on one cell, not of a column trigger or of an action. Whether a key or a pad on a cell selects it is topic J's (its J3-10), not ruled here.
ALT: b) The cell becomes selected only when the layer goes empty.
IF-WRONG: SMALL
ASK: NO technical; a selection that waits would only delay the paste, and a column trigger has no one cell to select.
@@END

@@ASSUME F3-7
ABOUT: 236, R188
TEXT: I assume every snapshot also keeps a still picture of the output, to find the moment in the list; there is no separate command that saves only a picture.
WHY: BF243 "with all the settings and the output and everything": "the output" can be its picture or which outputs were on; BF257 does not speak of a picture-only command. The picture is told to him inside the one question he gets on Snapshot (F3-6).
ALT: b) No picture: a snapshot is listed by show, date and time only. c) A picture-only command stays as well.
IF-WRONG: SMALL one picture file and one menu entry.
ASK: NO the picture is named in F3-6's text, where he can strike it; the rest is one menu entry.
@@END

@@ASSUME F3-9
ABOUT: 236
TEXT: I assume snapshots are kept in a folder of their own, each named by show, date and time, until you delete them, with no limit on their number; taking one never touches your show file.
WHY: BF257 "We keep the safe snapshot somewhere" names no place, no name and no limit.
ALT: b) Snapshots are kept inside the show file and travel with it. c) Only the newest twenty are kept.
IF-WRONG: SMALL where files are stored.
ASK: NO "somewhere" leaves the place to Harmony; his show file is never written by anything but his Save (old item 198).
@@END

@@ASSUME F3-10
ABOUT: 236
TEXT: I assume taking a snapshot never makes the picture stutter and shows nothing when it worked; if it fails, the "Save failed" box shows.
WHY: BF243 says the user pushes a shortcut in an inspiring moment of a running show; nothing says what he sees after the press.
ALT: b) A short sign confirms each snapshot.
IF-WRONG: SMALL
ASK: NO a build requirement, and his rule "the only fail message will be a failed save" (binding-decisions.md:720).
@@END

@@ASSUME F3-11
ABOUT: 236, R188
TEXT: I assume a snapshot keeps each slider as you set it, with its signal still plugged in, not the value the music gave it at that instant; and it leaves out what the computer keeps for every show, the audio input and the cue buttons.
WHY: BF243 "exactly where it is with all the settings and the output and everything" is wide: a slider moved by a signal has a set value and a momentary one, and "the output" could reach each output screen's own settings, which the computer holds (old item R188 (d)). Topic K reads the audio input out of a snapshot (item 272).
ALT: b) Every slider is kept at the exact value it showed at that instant, with its signal unplugged. c) A snapshot also keeps each output screen's own settings.
IF-WRONG: SMALL a snapshot opened without music looks calmer than its picture; the picture kept with it shows the moment itself.
ASK: NO "all the settings" are the sliders as set, and he knows a look moves with the music; what the computer holds is the same split as for a show.
@@END

@@ASSUME F3-12
ABOUT: 235, D23
TEXT: I assume the list of your old show files is shown to you before the build changes anything about show files, and nothing moves to the Trash before your yes.
WHY: Item 235, accepted, says "I list them first; after your yes"; it does not say at which moment of the build.
ALT: none
IF-WRONG: SMALL
ASK: NO the moment is Harmony's to pick; his yes is asked in any case.
@@END

@@ASSUME F-20
ABOUT: R188
TEXT: I assume the show holds one layout for each screen of the app, live and Studio now, more when you add configurations. Save Layout, Load Layout and Reset Layout stay in the View menu, to carry an arrangement between shows.
WHY: L94: the show and the computer hold "the layout". L35 speaks of various configurations. One layout or one per screen is not said. Only the name changes here (BF245, BF263).
ALT: b) The show holds the live screen's layout only; Studio's arrangement belongs to the computer. c) Save Layout and Load Layout go, because the show saves the layout; Reset Layout stays.
IF-WRONG: SMALL where a small block is stored, or two menu entries.
ASK: NO internal; unchanged but for the screen's new name.
@@END

@@ASSUME F3-13
ABOUT: 236
TEXT: I assume Snapshot comes with no key of its own: you put it on a key or a pad in the keyboard and MIDI mapping, like any button. While Studio is on screen it does nothing.
WHY: BF243 "the user can push a shortcut button" names no key; his "R227 all good" let stand that the spacebar is the only key that comes mapped. Nothing of his says what the press does while Studio is on screen, where the keys and pads of the mapping trigger nothing in the live show (topic E's old item R185).
ALT: b) It comes with a key of its own from the first start. c) In Studio it saves the moment of the recording that is shown.
IF-WRONG: SMALL one entry of the mapping.
ASK: NO which key a button sits on is his to set in the mapping (topic J's rule); small to change. Way c would be a new function of Studio, topic E's, and is not built.
@@END

@@ASSUME F3-14
ABOUT: 236, R189
TEXT: I assume a snapshot you open comes up as a show without a file of its own: Save asks for a name and your last show stays the same. It points at the show's media files; Collect Media leaves snapshots out.
WHY: BF243 and BF257 say nothing of what an opened snapshot is to Save, to the very last show (old item R189 (h)) or to the media. A snapshot that is looked at long after shows the red "!" cells of old item R225 (d) where a media file has moved since.
ALT: b) Collect Media takes the snapshots along. c) A snapshot copies its media.
IF-WRONG: SMALL
ASK: NO technical; a show points at its media in the same way (Save never copies media), and a safety copy opens in the same way (old item 198).
@@END

@@ASSUME F3-15
ABOUT: 234, G3
TEXT: I assume a clip you drag to another cell of its own layer keeps playing; dragged to another layer it leaves the layer it was playing on and waits in its new cell, and so does a clip that changes places with it.
WHY: BF256 names pasting over and deleting; a plain drag deletes no clip. Old item R190 (b) already lets a layer follow its clip when columns shift; a layer plays only the clips of its own row (read in CLAUDE.md rule 15, not run).
ALT: b) A dragged clip keeps playing on its old layer until you trigger something else there.
IF-WRONG: SMALL a rare drag during a show.
ASK: NO an edge of his rule for an edited clip; small to change.
@@END

@@ASSUME F-2
ABOUT: G2, G3
TEXT: I assume a copied clip brings everything it has: effects with their sliders and signals, its own envelopes, actions, play settings, saved pause. It does not bring the key, pad or knobs of its cell, nor the keys of its actions.
WHY: L4 and L5 say "copy", not what travels; his accepted defaults give the actions (165 A, 208 A) and keep pads on cells (197 A). New after page 2: BF262 "b" keeps a knob on a clip's slider with the cell, and BF261 makes an envelope "personalized for that clip", so it travels with the copy (topic I's item 241).
ALT: b) The copy brings the media and its play settings only, without effects and actions.
IF-WRONG: SMALL one list of fields in the copy.
ASK: NO the plain meaning of copy, carried by defaults he accepted (L1) and by his answers on knobs and envelopes.
@@END

## TOPIC G -- Output screens
### The topic ruling's notes for the page ruling
- MUST be put to him, the one question of this topic (ASK YES): G3-2. As ruled: All Outputs Off also lasts over a quit (the letter of his "b", BF269); its way b: it ends at the quit and the next start brings the last show's outputs on (his earlier BF216 leans there). It makes the one exception to what old assumptions G-3 and F-12 settled (the last show brings its outputs on at the start of the app): no other topic asks it.
- One line he can strike (ASK LINE), best right after G3-2 because both come from his "b" on item 265: G3-1. All Outputs Off is over once he switches one output on himself, for every output; its way b: each output stays off until he switches that very one on.
- Not shown (ASK NO): G3-3 (no message for a missing screen), G3-4 (another computer), G3-5 (what starts it), G-17 (opening a recording in Studio). The old internal ones (G-4, G-6 to G-13, G-16) are untouched by his 33 boxes.
- Newest against earlier, to be said to him once in one line, best as the lead-in of G3-2 and not a second time: BF269 "b" (the page's way b: "After All Outputs Off nothing comes on until you switch an output on yourself.") against BF216 "I expect the show to remember the outputs connected and not need to connect them again." In plain words: a show still remembers its outputs and brings them on when you open it; only after All Outputs Off do you bring them back yourself, with one press on Restore Last Outputs.
- NOT open, do not ask: that Restore Last Outputs works after All Outputs Off, also after a quit. Reading R191 part i of the first page said it; he named R191 and corrected only part g (H1).
- A word to keep off his page: "hold" is this topic's internal word for what All Outputs Off does afterwards. To him say that All Outputs Off lasts, or is over: "hold" is the pad setting of item 246.
- The paper's SUMMARY, REACHES, CONFLICTS and NOT DONE sections are carried over as written. Read three things in them with this ruling: where they say that a quit ends the hold (SUMMARY; REACHES, the bullet on F-12), that is now way b of G3-2, and the rule is that the hold is kept over a quit; where they call Restore Last Outputs after All Outputs Off an assumption (SUMMARY; the paper's G3-1), it is settled; and CONFLICTS bullet 1, which opens "Not his word against his word", is at the start of the app a real pull between BF269 and BF216.
- Depends on J: J3-9 (whether All Outputs Off and Restore Last Outputs can go on a key or a pad; item 271 names only an output screen's on and off). Whatever J rules, a press of his own counts as "yourself": Restore Last Outputs from a pad would end All Outputs Off like the menu command. Nothing else here changes.
- Depends on F: opening a snapshot counts as opening a show for the outputs (apply-F.md, item 236 part 4), so items 237, 264 and 265 apply as written, All Outputs Off included. F's R188 (a) says "which outputs were on when it was saved"; the rule is which outputs the show remembers (R191 g1, g2: a Save with the projector unplugged does not forget it): F should point here, not restate.
- Depends on E: opening or closing Studio, and opening a recording in it (G-17), switch no output on or off, also while All Outputs Off lasts; what an output that is on shows there is E's (186 b). E's reading of BF268 ("the output" is the picture, never one output screen's corrected picture) agrees with R191 part e.
- Agrees with X: X3-1 and the amended C19 (Syphon follows every rule for outputs); where All Outputs Off ends is ruled here (G3-1, G3-2), not there.
- At the build: the hold is one flag in the computer's settings, never in the show file; CLAUDE.md "Outputs", Pitfall 40's text and docs/claude/integration.md are rewritten (the paper's CONFLICTS bullet 2). What a second Mac reports for the same projector is measured with D28's test (G3-4).

### CONFLICTS (his newest words against earlier ones)
- Not his word against his word, but a narrowing he should hear once in one line. New, BF269 on item 265: "b" (the page's way b: "After All Outputs Off nothing comes on until you switch an output on yourself."). Earlier, binding-decisions.md:1175 (2026-10-07, BF216): "R191 if I save a show with the outputs connected, and I open the show back up with the outputs connected, I expect the show to remember the outputs connected and not need to connect them again." Squared: the show still remembers and brings its outputs on; only after All Outputs Off does he switch one on himself. The edge between the two is the quit (G3-2).
- His way b against the project's standing rule, which is no word of his: CLAUDE.md "Outputs" says "the app never opens an output by itself (only Output > Restore Last Outputs does". A grep for "never opens" in binding-decisions.md finds nothing, so no quoted word of his says it (the old spec found the same). His L96 replaced that rule on 2026-10-07; BF269 brings it back for one case only, the time after All Outputs Off. CLAUDE.md "Outputs", Pitfall 40's text and docs/claude/integration.md are rewritten when it is built.

### REACHES OTHER TOPICS
- F (R188 a, the show file; R189, opening a show): opening a show after All Outputs Off switches no output on (BF269); the show's remembered outputs are loaded and kept all the same. The hold is not a field of the show file.
- F (old assumption F-12, the twin of G-3: outputs at the start of the app): stands, with one new edge that this paper asks once for both topics (G3-2: a quit ends the hold).
- X (old assumption X-7, the twin of G-2; old item C19): closed by item 263, accepted as written; the hold of 265 reaches Syphon too (BF269).
- J (item 271, accepted as written: switching an output screen on or off cannot go on a key or a pad): it fits 265 b. "Switch an output on yourself" therefore means the Outputs list, Cmd+F and Restore Last Outputs, never a pad. Whether All Outputs Off and Restore Last Outputs can go on a pad is J's (old J-18 named both; item 271 as he read it names only the on and off of an output screen).
- E (item 186 b, the outputs while Studio is open; old E-1): closing or opening Studio during a hold must not switch an output on; G-17 says so for opening a recording. E rules what an output that is on shows.
- E (item 231, BF253 "It defaults to running next to the show recording"): applied here without an amendment: the low-resolution show recording runs beside record show, so D26 (no change of picture size while a recording runs) locks no longer than record show does. The worry in the old ruling notes (a lock all night) falls away.
- F (item 236 and the answer on Snapshot, BF243 "with all the settings and the output and everything", BF257): if a Snapshot is a save of the show, F must say whether opening one counts as opening a show for the outputs (then 237 and 265 apply as written) and whether it still holds a still picture; R226 b's word "snapshots" (the Transform shows in them) waits on that and is not amended here.
- B (item 252, BF267, the monitors at one frame rate): not applied here; R209 (the outputs keep going whatever the main window shows) is untouched by it.

### NOT DONE / UNSURE
- Where the hold ends (G3-2) is the only thing here that needs his answer. Cheapest: one question, "After All Outputs Off and a quit, does the next start bring your show's outputs on?"
- A corner the hold makes easier to reach, left with old assumption G-6 (not shown to him, unchanged): a show forgets an output only when he switches it off by its own line; during a hold that output is already off, so to make the show forget it he must tick it on and off again. Cheapest: leave it; if it bothers him after the build, a "forget this screen" entry is a small addition.
- Whether All Outputs Off pressed when no output is on also starts a hold: written as yes (the command always holds); not asked, because the only effect is that a show opened next adds nothing until he ticks an output. Cheapest: fold into the question on G3-2 if he asks.
- TODAY lines are taken from spec-G.md and the fact sheet area-outputs.md; I opened no source file for this paper. Nothing was run.
- Old assumptions G-4, G-6, G-7, G-8, G-9, G-10, G-11, G-12, G-13 and G-16 were tested against the 33 boxes and the four items: untouched, nothing written. G-17 keeps its sense and gets the name Studio. No old assumption of the internal list is dropped.

### The assumptions of this topic after the ruling (YES first, then LINE, then NO)
@@ASSUME G3-2
ABOUT: 265; R191 (g5, g9, i)
TEXT: I assume All Outputs Off also lasts over a quit: if you quit after it, the next start of the app opens your last show with no output on, and one press on Restore Last Outputs brings your outputs back.
WHY: BF269 "b" gives All Outputs Off no end but his own switch-on, and a quit is not that: his answer, read to the letter, is the rule. Against it stands his earlier BF216, "I expect the show to remember the outputs connected and not need to connect them again.", with BF215, "When you open the application, it opens to the very last show." Which of the two he means at the start of the app is not said.
ALT: b) It ends when you quit: the next start of the app brings your last show's outputs on by themselves.
IF-WRONG: STAGE built as written and meant the other way, his outputs stay off at every start that follows a night ended with All Outputs Off, until he presses Restore Last Outputs; built the other way and meant as written, a projector he had cut lights up by itself at the start of the app
ASK: YES his newest word and his earlier one pull apart exactly here (BF269 against BF216) and no word of his names the quit; he sees it at set-up before a show; it is about what the app does
@@END

@@ASSUME G3-1
ABOUT: 265; R191 (g9)
TEXT: I assume All Outputs Off is over once you switch one output on yourself: only that one comes on, and from then on a show you open brings its outputs on again.
WHY: BF269 "b" (the page's way b: "After All Outputs Off nothing comes on until you switch an output on yourself.") can be read two ways: the first switch-on ends it for every output, which is the letter of "an output", or each output stays off until he switches that very one on. Not asked any more: that Restore Last Outputs still works after All Outputs Off is reading R191 part i of the first page, which he named (BF216) and left as it was.
ALT: b) Each output stays off until you switch that very output on yourself, whatever show you open.
IF-WRONG: STAGE he cuts two projectors, switches one on again and opens the next show: the other lights up by itself, or, built the other way, stays dark until he ticks it; a small change to undo
ASK: LINE the text follows the letter of the way he chose; the other way is a stricter guard he may want, and he can strike the line
@@END

@@ASSUME G3-3
ABOUT: 264; R191 (g6)
TEXT: I assume no message is shown when a screen that a show remembers is not plugged in: the show simply opens without it.
WHY: Item 264, which he accepted, says nothing comes on by itself; it does not say whether the app tells him a remembered screen is missing.
ALT: b) A short note names the remembered screen that was not found.
IF-WRONG: SMALL a note can be added later; he sees at once that the projector is dark
ASK: NO how a missing screen is told is a matter of look, left to the UI redesign
@@END

@@ASSUME G-17
ABOUT: R191 (g11)
TEXT: I assume opening a recording in Studio never switches an output on or off by itself, although the show file saved with that recording remembers the outputs of that night.
WHY: Unchanged in what it says; only the screen's name is new (BF245, BF263). His L96 lets an opened show switch outputs on; whether a recording's own show file counts is not said.
ALT: b) Opening a recording in Studio brings on the outputs that were on that night.
IF-WRONG: SMALL he ticks one output to watch a recording on the big screen
ASK: NO the careful choice at no cost; topic E owns Studio and what the outputs show while it is open
@@END

@@ASSUME G3-4
ABOUT: 264; R191 (g8); D28
TEXT: I assume a show opened on another computer switches no screen on by itself, also not the same projector: you tick it once there.
WHY: Item 264, which he accepted, names only a different projector; "on another computer" came from old R191 g8 and old assumption G-5 and was not in the item he read. His BF216 speaks of opening the show back up with the outputs connected and names no computer. Whether a second Mac reports the same projector as the same screen is not known (D28: measured after the build). Syphon is no screen: item 263 as he accepted it names no computer, so Syphon comes on with its show on any computer.
ALT: b) On another computer the same projector comes on by itself too, when that computer reports it as the very screen the show remembers.
IF-WRONG: SMALL one tick at set-up on the second computer
ASK: NO rare, and it hangs on what a second Mac reports, which is measured after the build; the careful choice until then
@@END

@@ASSUME G3-5
ABOUT: 265; R191 (g9)
TEXT: I assume only the command All Outputs Off, or its key, makes the outputs stay off in this way, also when no output was on at the press. Switching every output off one by one, each by its own line, does not.
WHY: BF269 "b" names All Outputs Off and says nothing of its edges: a press while nothing is on, and all outputs switched off singly. An output switched off by its own line is forgotten by the show anyway (R191 g2), so no show brings it back.
ALT: b) Switching the last output off by its own line counts as All Outputs Off too. c) A press while no output is on changes nothing.
IF-WRONG: SMALL after a stray press he ticks an output or presses Restore Last Outputs once
ASK: NO two edges of his own answer with one sensible reading each; nothing for him to choose
@@END

## TOPIC H -- How a clip plays
### The topic ruling's notes for the page ruling
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

### CONFLICTS (his newest words against earlier ones)
- 238, no conflict to put to him, noted because it looks like one: BF258 "b" (whole groups of 4 bars, the end cut in) against his L99 "R218 do beats here. It was my mistake before" (binding-decisions.md 1177) and his look in his Resolume, L21 (spec-H R218: "By default it gives me a decent amount of beats (sometimes 8, sometimes 16)"). The row still reads and steps in beats (L99 stands); only the FIRST number follows his rule of 2026-10-04 (binding-decisions.md 812-813: "always work with multiples of 4. If it is uneven, then move the outpoint in to keep those multiples."), which BF258 confirms. In one line for him: "Your clips in BPM mode will not start like your Resolume's (8 or 16 beats, nothing cut): they get whole groups of 4 bars and lose the rest of their end, as you chose."
- R194 f and P28 (topic I owns it): BF260 "All the blend modes and keying stay, and they will be built when there is time." against binding-decisions.md 1185: "204 I want to remove the keying and slider. We are only going to use the transparency slider to control that layers blend mode." His newest words win; amended here in this topic's two old blocks.
- 239, not a conflict of his words: the page's "First I measure" was Harmony's text; his BF240 puts the test after the build. His earlier "We should pick the most optimal Kodex to use and not worry about the other ones" (binding-decisions.md 650) is served by the test's result, not contradicted.

### REACHES OTHER TOPICS
- E, item 232 way c (BF254 "c"), must agree with item 238 here: ONE kind of cut. In both, the clip out point is moved in to the end of the last whole group of 4 bars, nothing is taken out of the file ("the rest stays in the file"), and the rest plays again when he drags the clip out point out. One difference E has to rule: a clip made with Record to Clip is counted in the bars the beat clock really ran while it was recorded (5 recorded bars = 4 kept, Beats 16), NOT at 120 BPM as a clip he brings in (238, H-10); counted at 120, a clip recorded at any other tempo would get the wrong Beats. Also E's to rule: a recording shorter than 4 bars, and whether such a clip is born in BPM mode (item 267 speaks only of "a video you bring in"; a clip recorded on the beat that came in in Timeline mode would not loop its 4 bars in time).
- E, BF241 ("we should test which codecs work best"): the encoding half of the codec test is ruled here (item 239 point 2, G1 point 7); E names which codecs and sizes of the low-resolution show recording and of Record to Clip go into it, and refers here for the test.
- A, BF271 ("Stop removes all clips from all layers"): A owns the rule. Applied here only as the ground of H-4: after a tempo stop every clip has been away from its layer, so item 202's start menu acts at the next trigger.
- I, BF260 ("All the blend modes and keying stay ..."): I owns item 240 and the conflict with his L111. Applied here to this topic's own old blocks R194 (f) and P28, which had the keying and its K slider leave the layer strip.
- J, BF245 and BF263 (Studio): applied here to G1 ("scrubbing in Studio"). .harmony/NAMES.md row "Review" is J's to change.
- B, BF247 ("Previewing a clip ... should just be quick"): how fast a previewed clip shows its first picture is one of the waits the codec test times (item 239 point 2); B owns the preview rule.
- F, the list of what a copied clip carries (old G2): a copy carries its Beats and its clip out point as they stand; a copy is not counted and cut a second time (238 point 6).
- A and B, item 266: the restart of an already playing clip is ruled here and in item 202; A's R128 d (a column trigger of a playing column) and D's BF251 ("plays again only when that clip is re-triggered") lean on it: a re-trigger is a restart.

### NOT DONE / UNSURE
- 238: the size of the cut was not on the page. A clip of 15 seconds keeps 8; a clip of 23.9 seconds keeps 16: up to just under 8 seconds of every clip longer than 8 seconds goes from its end in BPM mode (arithmetic at 120 BPM, not measured). He chose the cut and it is his own rule of 2026-10-04; it is not asked again. If the page ruling wants it said once, the line is in CONFLICTS.
- 238: "exactly its normal speed" is true at a tempo of 120 only; at 128 the same clip plays faster. That is BPM mode itself and his "not current bpm"; carried in H-10 (not asked).
- 239: nothing is measured. Whether a jump on his ordinary clips waits long enough to be seen is UNKNOWN until the codec test; Random, Beat Repeat and backwards play are therefore built without that knowledge unless he takes H3-3 way b. Cheapest way to know earlier: way b (two clips, one session, while those functions are built).
- 239: whether the FFmpeg on this Mac can write HAP (needed only for the test copies) and whether a DXV file of his opens: not checked. Cheapest: one look at the encoder list and one file tried, in the session of the test.
- The name "Convert for performance" (old spec-H NAMES) is not planned any more; it is not in .harmony/NAMES.md (grep: no hit), so nothing is to be taken out there. The lint's DROP takes a one-word id, so no @@DROP NAME is written for it.
- Not amended, wording only: old RULE lines of this topic still say "fire / fired" where his word is "trigger" (202, R192, R193, R194, G1).
- Old assumptions of this topic tested against his 33 boxes and left untouched: H-2, H-6, H-7, H-9, H-11, H-12, H-13. Changed and kept under their ids: H-4 (BF271), H-10 (BF258).

Written 2026-10-09 18:48:40 EDT by the architect seat of topic H. Read-only; nothing built, run or committed.

### The assumptions of this topic after the ruling (YES first, then LINE, then NO)
@@ASSUME H3-2
ABOUT: 239, G1
TEXT: I assume we make no HAP copies and build no command for them. Only if the codec test after the build shows that a jump, backwards play or a random beat waits on your ordinary clips do I ask again.
WHY: BF259 is a question, not a choice: "Why would we make HAP copies at all?"; with BF240 the test that could give a reason comes after the build.
ALT: b) Never, whatever the test shows: your clips play as they are.
IF-WRONG: STAGE on ordinary video files a Random jump or backwards play may wait in front of an audience; or a converter is built that he never needed
ASK: YES he asked back; the item returns in a form his question shows he can answer
@@END

@@ASSUME H3-1
ABOUT: 238, 267, R218, R193
TEXT: I assume the end is cut the first time you switch a clip to BPM mode: up to 8 seconds, so a loop without a seam can show a jump there. In Timeline mode it stays cut until you drag the clip out point out.
WHY: BF258 picks the cut but not its moment; item 267 (accepted) brings every video in in Timeline mode, so no clip is "new in BPM mode" when it arrives. How much the cut takes (less than one group of 4 bars, which is 8 seconds at 120 BPM) follows from his words (binding-decisions.md 778-782, 812-813) but never stood on his page, and neither did what a cut end does to a clip made to repeat without a seam (it jumps where the end was cut): both are said here once, in the line he reads. The part is counted from the clip in point (item 238 point 1).
ALT: b) Timeline mode always plays the whole clip again; the cut holds only in BPM mode. c) The end is cut the moment the clip comes into the show, in Timeline mode too.
IF-WRONG: SMALL a clip switched to BPM mode and back would lack, or have, its last seconds; one drag of the clip out point mends it
ASK: LINE a real choice of Harmony's about when his clips lose their end; he would most likely wave it through
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

@@ASSUME H-10
ABOUT: 238, R218
TEXT: I assume a clip short of a whole group of 4 bars by less than a tenth of a beat counts as whole and is not cut. A clip of many pictures is counted at its Timeline-mode rate and gets at least one group.
WHY: BF258's way b gives no tolerance and does not name a clip of many pictures. That the count is at 120 BPM and that a short clip gets 16 beats are his words (binding-decisions.md 778-782, 673, 684, 846, 865): they stand in the RULE of item 238 and are no part of this assumption. That a clip of many pictures loses no picture is asked in topic K (K3-5).
ALT: b) No tolerance: a clip one picture short of 16 seconds is cut to 8. c) A clip of many pictures gets the number of groups nearest to its length, not the number that fits.
IF-WRONG: SMALL the starting number of some clips differs; he sets Beats himself anyway
ASK: NO arithmetic of the count; what his words say of it stands in the RULE
@@END

@@ASSUME H-4
ABOUT: 202
TEXT: I assume tempo stop, which takes every clip off its layer, forgets where each clip was: the next trigger starts it from the start, whatever its start menu says. After any other leave, a BPM-mode clip that continues starts on its nearest earlier beat marker.
WHY: His words take every clip off its layer at a stop (BF271: "Stop removes all clips from all layers", which repeats binding-decisions.md 952: "stop clears all clips from layer strips"); they do not say whether a clip set to Continue then remembers its place.
ALT: b) Tempo stop leaves each clip's place alone, so a clip set to Continue picks up where the stop took it off. c) In BPM mode the start menu is not used: such a clip always starts from the start.
IF-WRONG: SMALL only clips whose start menu he changed are touched; one reset more or less
ASK: NO an edge of a menu that stands on Restart until he changes it; small to change
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

### Items of page 2 that are still OPEN in this topic: 239

## TOPIC I -- Effects, signals and what moves a slider by itself
### The topic ruling's notes for the page ruling
- MUST BE PUT TO HIM, in order of weight. (1) I3-6: what his "adjust the sink" sets on a strobe, a pulse and effects like them: how fast against the beat (default), a switch between the beat and a free speed (b), or a shift against the beat (c). It decides a control of the coming build that he uses live. (2) I3-2: what a mask is. Masks come at the last stage and nothing of them is built before his answer; if his page must shrink, this is the one that can wait until masks are planned.
- LINES he can strike: I3-4 (an envelope drawn for a clip is that clip's own; topic C's C3-6 goes right after it), I3-5 (the envelope that follows the playhead is for a clip in Timeline mode, as he wrote; b: in BPM mode too; c: a layer's slider too), I3-7 (the common macro waits; b: built with the rest, set up by him in a show).
- NEWEST WORDS AGAINST EARLIER ONES, one line for him: "On 7 October you wrote 'I want to remove the keying and slider'; on 9 October 'All the blend modes and keying stay'. I follow the later one, and I read the keying slider as staying with the keying." (BF260 against binding-decisions.md:1185 and 654-655.) Nothing else of this topic goes against earlier words of his.
- THE WORD SYNC: before, his "sync" was the timing dial that he replaced by a Delay per output ("replace our sync with this", binding-decisions.md:795). The Sync of an effect is another thing; way c of I3-6 says so to him. NAMES.md must keep the two apart, and the signal-list entry "BPM Sync" as well.
- A FACT FOR HARMONY, not for his page (his rule: no "today"): Strobe and Pulse, the two effects named to him as reading the beat by themselves, run on plain time at a speed of their own before the build (read, not run: verdict H2). The rule he accepted (item 243) makes the build put them on the beat clock. If he answers I3-6 with b, a free speed stays possible.
- TOPIC K: about 50 generated pictures name a beat uniform in their program text (a search, not read one by one): they are what reads the beat by itself now. His rule covers them ("Pause pauses, the beat clock, the clips and everything that it controls with BPM.", BF167); K's paper should say that they hold on pause and stop, and that they get no Sync unless he asks (BF244 names "these effects"). K's count per kind of layer stays (I3-3).
- TOPIC A keeps AMEND R214 1; it is dropped here (H1). What follows a "1" that the app moved (A's item 218, assumption A3-2) takes in the signals and envelopes on the beat, the next "1" of a One Shot (item 242) and the effects of item 243.
- TOPIC C: AMEND R157 6 and C3-7 stand (Sync stays a setting of the effect, now a slider in steps). AMEND R180 1 and C3-6 stand on item 241 part (3): a clip's envelope is the clip's own. TOPIC F: a copy of a clip carries its own envelopes of both types, unchanged. TOPIC B: B3-6 stands; what a mask does in the cue waits on I3-2. TOPIC J: Sync on a MIDI knob moves in steps, with both kinds of knob (BF270).
- TOPIC X (checker F15): X-16 says "which entries stay is put to you then". For the blend modes and the keying no entry is up for removal (BF260: "All the blend modes and keying stay"); only the transitions, and what each entry will do, are put to him. TOPIC D: the TODAY line of old item 193 ("Keying and its slider are being removed") is out of date; a note, no rule.
- TOPICS B (B-3), C (C-13) and H (R194 f) lean on "every blend mode follows the transparency slider": it stands, but it is built at the last stage (I3-1, internal). The coming build must not count on it, nor on any blend or keying entry that is not built.
- NAMES.md: the row "K slider, Keying (modes and slider) -- Taken out" no longer holds. New rows: Mask, Moving mask, Keying, Sync, Common macro, Envelope on the beat, Envelope on the playhead; Falloff re-worded (item 268).
- NOT LOOKED UP (no web tools here): how his Resolume treats a layer in mask mode. If the default of I3-2 should match it, that is a Researcher's task before his page is made.
- THE PAPER'S OWN NOTES are copied into the spec unruled; three of them are overtaken: its REACHES bullet on R214 (that amendment is topic A's now, H1), its NOT DONE bullet that which effects read the beat was "not looked up" (looked up for Strobe and Pulse, H2), and its REACHES bullet for topic J that Sync "steps through its list" (Sync is a slider in steps, H3).

### CONFLICTS (his newest words against earlier ones)
- The keying. NEW (BF260, 2026-10-09): "All the blend modes and keying stay, and they will be built when there is time. Right now they're just sitting there as placeholder." AGAINST his round-1 answer (binding-decisions.md:1185): "204 I want to remove the keying and slider. We are only going to use the transparency slider to control that layers blend mode." and his earlier "Does the keying slider actually do anything? Maybe we get rid of it." (binding-decisions.md:654-655). The newest wins: the keying and its slider stay. To say to him in one line: "On 7 October you wrote to remove the keying and its slider; on 9 October you wrote that the keying stays and is built later. I follow the later one."
- Mask layers. NEW (BF260): "We will also add masks and moving masks with which are alpha channels" AGAINST the page's own text, which is Harmony's and not his ("There are no mask layers", item 240) and the best reading in the old 204 (Harmony's: "a layer has no kind"). His words win; no earlier word of his is overturned.
- When. NEW (BF260): "they will be built when there is time" BESIDE (binding-decisions.md:1184): "203 c this is built after all other parts are done. This is last, before the ui redesign which is the final change". Not against each other; read as the same time (I3-1, a line he can strike).
- A drawn shape along the clip. NEW (BF261): "another one is play head based so we can draw a envelope that happens over the course of the playing of that clip in timeline mode" AGAINST the page's text (Harmony's: "always runs on the beat, never along the clip's playhead"). It agrees with his earlier "build." (binding-decisions.md:258, said to the drawable Timeline curve): the old open conflict there is closed in favour of both.
- One macro for all. NEW (BF244, a "maybe"): "a single macro controls the speed of all of these type of effects" BESIDE his earlier "all. Global, layer, clip." (binding-decisions.md:263, macro banks at all three levels, each slider on its own level's bank). One knob for effects on every level would cross the banks: nothing is built for it until he asks (I3-7).

### REACHES OTHER TOPICS
- Topic A, @@ITEM R214 (named under MADE FROM of item 243): its open clause on effects that read the beat is closed by @@AMEND R214 1 above. BF271 ("A pause would not pause it unless it is connected to the BPM"), a box of topics A / K, was used here only as support for item 243.
- Topic K, old item R203 (c) (the autopilot's count per kind of layer, which hung on I-1): BF260 takes nothing out, so the count per kind stays; K's paper should say so (I3-3). If I3-2 holds, "mask" becomes a kind of layer he can pick, and K's count per kind then has a visible meaning.
- Topic F (the show file): a show keeps each layer's keying choice and keying slider value (BF260 reverses the removal); it will hold clip-owned envelopes of two types (BF261) and, later, mask layers. A copy of a clip carries its own envelopes.
- Topics B (old assumption B-3), C (C-13) and H (R194 f) lean on "every blend mode follows the transparency slider" from L111: the sentence stands, but it is built at the last stage, not in the coming build (I3-1). Until then they must not count on it.
- Topic C (presets): Sync is a control of the effect and is held by a preset. Whether a preset carries a clip-owned envelope's shape is still not settled (it was open before; BF261 makes it sharper).
- Topics D and E (actions, recordings, Studio): Sync is recorded and can be moved by an action like any control; an envelope's type and owner are part of what a recording must find again.
- Topic B (previews): BF247 says a preview starts at once, not on the beat. What a One Shot (item 242: "from the next 1") or an envelope on the beat does inside a preview is B's to rule; this paper rules the output only.
- Topic J (the mapping): Sync can be put on a key, pad or MIDI knob; on a knob it steps through its list.
- NAMES.md (kept by Harmony): the row that says the keying is taken out must be replaced; new rows Mask, Moving mask, Sync, the two envelope types.

### NOT DONE / UNSURE
- What a mask is (I3-2) and what "adjust the sync" means (I3-6): his words do not settle either. Cheapest: the two questions, one line each, on his next page.
- Which effects read the beat by themselves, and whether any already has a control for its rate: not looked up (only a count of lines in src/render/EmbeddedShaders.h). Cheapest: read each such effect's program text when the part is planned (I3-9).
- What the keying slider and each keying and blend entry will do when built: not ruled; put to him when that stage is planned (I-5).
- Whether the bottom layer under a blend mode other than Normal should show plainly (I3-3 way b): left as a note for that later stage.
- Whether a preset carries a clip-owned envelope: topic C; one line on a later page.
- How Resolume itself treats a layer in mask mode was not looked up (no web tools here, and his Resolume is never touched); if Harmony wants I3-2 to match it, that is a Researcher's task.
- TODAY lines are taken from the old blocks and the fact sheets of 2026-10-05, plus src/model/Layer.h:199 read by me; nothing was run.
- Written 2026-10-09 18:54:15 EDT by the architect of topic I (read, not run; nothing built).

### The assumptions of this topic after the ruling (YES first, then LINE, then NO)
@@ASSUME I3-2
ABOUT: 240; 204
TEXT: I assume a mask is a layer switched to work as a mask: its clip is not shown, it decides where the layers below show or hide. With a video, or moved by a signal or an action, it is a moving mask.
WHY: BF260 says only "masks and moving masks with which are alpha channels": not what a mask cuts, where it comes from, or what makes it a moving one. The default covers both senses of "moving" (a video; a still picture that is moved). Not looked up: how his Resolume treats a layer in mask mode (no web tools here; a Researcher's task if the default should match it).
ALT: b) A mask layer cuts only the one layer directly below it. c) A mask is a picture or video you put on one clip or one layer, and it cuts only that clip or layer.
IF-WRONG: REBUILD what a mask cuts decides how the picture is put together and what the show file holds.
ASK: YES a new function of his with no word on how it works. It is the lighter of this topic's two questions: masks come at the last stage and nothing of them is built before his answer.
@@END

@@ASSUME I3-6
ABOUT: 243; R221
TEXT: I assume your "adjust the sink" (I read: the sync) means: a strobe, a pulse and effects like them run locked to the beat, and each gets a Sync slider that sets how fast it pulses (every bar, every beat, every half beat ...).
WHY: BF244 says "adjust the sink" (read: the sync) and does not say what is adjusted; "controls the speed" in the same sentence points to the rate. Against the default: his own earlier word "sync" was the timing dial that shifted the visuals against the music, which he replaced by a Delay per output ("replace our sync with this", binding-decisions.md:795): that is way c. And for his clips "synced" is one of two ways to play ("either BPM synced throughout the whole thing, or just playing with a speed control", binding-decisions.md:630-631): that is way b, which also keeps a free speed, the only thing Strobe and Pulse have before the build (ruling H2).
ALT: b) Sync is a switch on each of them: on, its speed is locked to the beat; off, it runs free at the speed you set. c) Sync shifts the effect earlier or later against the beat, so its flash lands where you want; that is how you used the word sync before.
IF-WRONG: STAGE a strobe that cannot be set the way he means it is seen at once; one control on every such effect to change.
ASK: YES a dictated word that can be read three ways; it decides a control he will use live, in the coming build. The first question of this topic.
@@END

@@ASSUME I3-4
ABOUT: 241; R195
TEXT: I assume an envelope you draw for a clip is that clip's own: saved and copied with it, and only that clip's sliders and buttons can use it. A layer's or a Global slider uses envelopes that the whole show shares.
WHY: BF261 "personalized for that clip" speaks of the clip only. His round-1 words allow "many sliders share the same one" without saying whose, and the rule shown to him then kept signals of his own for the whole show (Mod 1, Mod 2 and new ones for envelopes: the page's text, which he did not correct). Topic C's question on what a preset carries of a clip's envelope (C3-6) hangs on this line and goes right after it.
ALT: b) Every envelope is in one list for the whole show, and any clip can use any of them. c) A layer and Global own their envelopes too; nothing is shared by the whole show.
IF-WRONG: SMALL the build holds both kinds of owner; a kind he does not want is hidden.
ASK: LINE a reading of one word of his ("personalized"); he can strike it.
@@END

@@ASSUME I3-5
ABOUT: 241; 205, R220
TEXT: I assume the envelope that follows the playhead is a clip's own and works on a clip in Timeline mode, as you wrote; a clip in BPM mode uses the envelope along the beat.
WHY: BF261 names "that clip in timeline mode" only; a clip in BPM mode and a layer's slider are not named. "in timeline mode" can also be read as the way the envelope runs (along the clip's timeline), which would not shut out a clip in BPM mode: that reading is way b. The straight link Timeline is offered on a layer's controls and in BPM mode too (old item R220), so the rule as his words give it leaves the drawn form with fewer places than the straight one; Harmony's advice is way b.
ALT: b) It works on a clip in BPM mode too. c) A layer's slider can use one too: it is stretched over whatever clip plays on that layer.
IF-WRONG: SMALL the entry is offered in more places and, for way c, the show gets envelopes on the playhead that belong to no clip; nothing built is rebuilt.
ASK: LINE the rule follows his own words; he can strike it or pick a way.
@@END

@@ASSUME I3-7
ABOUT: 243; R210
TEXT: I assume the common macro waits, as your "maybe": each such effect gets its own Sync, which a macro knob of its own clip, layer or Global can turn. One knob for all of them across the show is built when you ask.
WHY: BF244 says "maybe we could use a common macro that we could set later": a maybe. "set later" reads two ways: built later, or set up by him later in a show. Macros are in three kinds of bank and a slider follows only its own tab's bank (old item R210 a; his "all. Global, layer, clip.", binding-decisions.md:263), so one knob for effects on clips, on layers and on Global together needs something new: a slider that can be plugged into a Global macro knob from anywhere. That is the price of way b.
ALT: b) It is built with the rest: the Sync of any such effect, wherever it sits, can be plugged into one Global macro knob, so you set it up in a show.
IF-WRONG: SMALL it can be added later without changing what was built.
ASK: LINE his own "maybe" and "later"; he can strike it.
@@END

@@ASSUME I3-1
ABOUT: 240; 203, 204, D31
TEXT: I assume "when there is time" means after the coming build: blend modes, keying and masks are left alone in it, nothing is taken out, and they are built as the last work before the UI redesign.
WHY: BF260 gives no date ("they will be built when there is time"; "We will also add"). His round-1 words on these lists said last, before the UI redesign ("203 c this is built after all other parts are done. This is last, before the ui redesign which is the final change", binding-decisions.md:1184). Other orders Harmony could have picked and did not: masks in the coming build; every blend mode made to follow the transparency slider in the coming build.
ALT: none
IF-WRONG: SMALL the same work, earlier or later.
ASK: NO his own words carry the order of work, and the order of work is not what the app does (checker F10).
@@END

@@ASSUME I3-3
ABOUT: 240; 204, R196
TEXT: I assume every layer, the bottom one too, is blended in by its blend mode and its transparency slider. No control for a layer's "kind" is added in the coming build; the switch that makes a layer a mask comes with the masks.
WHY: He read this sentence of item 240, and his words (BF260) go only against the removal of the keying and against "There are no mask layers". Nothing that belongs to a kind is taken out (effects-only cells, the autopilot's count per kind of layer).
ALT: b) The bottom layer always shows plainly, whatever its blend mode says.
IF-WRONG: SMALL one layer's behaviour under a blend mode other than Normal.
ASK: NO he read the sentence and left it; nothing is taken out for it.
@@END

@@ASSUME I3-8
ABOUT: 240
TEXT: I assume a mask is read by its see-through parts; for a picture or video that has none, by its brightness: bright shows, dark hides.
WHY: BF260 says masks "are alpha channels"; most videos carry none, and what is used then is not said.
ALT: b) Only a picture's own see-through parts count; a video without them cannot be a mask. c) A switch on the mask picks which of the two is used.
IF-WRONG: SMALL one formula; put to him with the rest when masks are planned.
ASK: NO technical detail of a part that is planned later and shown to him then.
@@END

@@ASSUME I3-9
ABOUT: 243; R221
TEXT: I assume "these effects" are Strobe, Pulse and every other effect whose picture flashes or pulses by itself at a speed of its own; I list them for you when this part is planned.
WHY: BF244 says "these effects" and "these type of effects" and names none; the page named a strobe and a pulse. Neither reads the beat before the build (ruling H2), so the list cannot be made from which effects read the beat: it is made from which effects pulse by themselves, and each of them is put on the beat clock.
ALT: none
IF-WRONG: SMALL an effect gets or lacks one control.
ASK: NO a list made from the effects themselves and shown to him then.
@@END

@@ASSUME I-4
ABOUT: 204; 240
TEXT: I assume the effects called Chroma Key and Key Palette stay in the effects list beside the layer's keying, and a picture's own see-through parts (a logo) stay see-through on every layer, whatever is later built for the keying.
WHY: BF260 keeps "keying" and names no effect and not a picture's own transparency.
ALT: none
IF-WRONG: SMALL a logo that lost its see-through parts would be a fault, not a choice.
ASK: NO nothing he asked for is taken away; a guard for the build.
@@END

@@ASSUME I-5
ABOUT: 203; 240
TEXT: I assume that when the turn of the blend modes, the keying and the transitions comes, I show you what each entry and the keying slider will do before it is built. No blend mode and no keying entry is removed.
WHY: BF260 says all stay and "will be built", but not what each entry should do, and his own doubt about the slider is still open ("Does the keying slider actually do anything?", binding-decisions.md:654); L110 leaves the lists until last.
ALT: b) Each entry is built to do what the same name does in Resolume, without asking again.
IF-WRONG: SMALL it is asked again before anything is built.
ASK: NO it only promises a later question.
@@END

@@ASSUME I-8
ABOUT: 205, R220; 241
TEXT: I assume "start" and "end", for Timeline and for an envelope on the playhead, are the clip's In and Out points, and during a fade between two clips a layer's slider follows the clip that is coming in.
WHY: L112 and BF261 ("over the course of the playing of that clip") do not say where the two ends lie or what holds during a fade.
ALT: b) Start and end are the whole file. c) During a fade the layer's slider stays with the clip that is leaving.
IF-WRONG: SMALL a constant to change.
ASK: NO internal detail.
@@END

@@ASSUME I-11
ABOUT: R221, 205; 241
TEXT: I assume Looping / One Shot is shown on a slider or button that uses an envelope on the beat or an oscillator. On Timeline and on an envelope on the playhead the playhead decides, and a music signal has no beginning or end: there it is not shown.
WHY: L106 puts the pair on the user of the signal and does not say for which signals; BF261 adds an envelope that follows the playhead.
ALT: b) On a music signal One Shot means: react to the next hit only, then stop. c) An envelope on the playhead can be set to play only on the clip's first pass.
IF-WRONG: SMALL one control shown or hidden.
ASK: NO a control is left out only where it has nothing to do.
@@END

@@ASSUME I-17
ABOUT: R195; 241
TEXT: I assume a new envelope made from a clip's slider or button is that clip's own; one made anywhere else belongs to the whole show. Its type is picked when it is made and can be changed. Deleting one sets its sliders back to Manual.
WHY: L104 and BF261 say how many envelopes and which two types, not how one is made, named, switched or deleted; whose a new one is follows item 241 part (3). Only a clip's own envelope has a type to pick: the envelopes of the whole show run on the beat. Where the entry sits is his L8.
ALT: b) The type is fixed once the envelope is made.
IF-WRONG: SMALL a menu entry and names.
ASK: NO layout and names (L8).
@@END

@@ASSUME I-15
ABOUT: R221; 241
TEXT: I assume the moving line is drawn on every signal that has a shape (oscillators, envelopes, sample envelopes) and shows where its loop is; on an envelope that follows the playhead it shows the playhead's place. A music signal shows its level.
WHY: L108 says "a moving line on each signal to know where they are"; BF261 adds an envelope that follows a clip's playhead, which has no loop place of its own; a music signal has no shape, and a One Shot (L106) runs at a place of its own.
ALT: b) Music signals get nothing extra. c) The line shows the place of the slider you are working on, a One Shot's run too.
IF-WRONG: SMALL what one line means.
ASK: NO what the line shows on each kind of signal is a look (L8); it was internal before and stays so.
@@END

## TOPIC J -- The keyboard and MIDI mapping, menus and messages
### The topic ruling's notes for the page ruling
- MUST GO TO HIM, by weight: (1) J3-hold (YES), with @@ANSWER 246-hold printed above it; both come from answer-hold.md, which the merge lays over this topic AFTER this ruling; item 246 stays OPEN on it. (2) J3-8 (YES): may a knob that sends its position make a slider jump; print it beside the line that says both kinds of knob now work. Then two LINES: (3) J3-2: after a deck switch the knob serves the same cell of the new deck, not the clip that is still playing. (4) J3-10: a key or pad that triggers a cell also selects it; print it INSIDE topic F's line F3-1 ("a key or a pad on the cell does the same") and let that one line also say that Delete acts without asking (the old J-19, internal).
- NOT FOR HIM (ASK NO): J3-1 (the knob finds its slider by name: the way he chose says "that slider"), J3-3, J3-4, J3-5, J3-6 (the Endless toggle: his "If we have to" leaves it to us), J3-7, J3-9, J3-11, and the old J-1, J-16 and J-19, written again. The other old internal assumptions of this topic (J-6, J-12 to J-15, J-17, J-20 to J-23) are untouched by his 33 boxes.
- NEWEST WORDS AGAINST EARLIER ONES, to be said to him once, in one line: on 2026-10-07 he chose "207 c" (nothing to set on a knob); now "We need to work with both and know how they both work so they both work smoothly." (BF270): the endless knob is back, with one toggle per knob. The name Studio is no conflict: he had asked for a better name himself.
- A TENSION, not a clash, carried by J3-2 (the paper's CONFLICTS has no line for it): BF262 "b" (the knob belongs to the cell of the deck on screen) beside BF256 ("If a clip is playing, and I change the deck, that does not change the clip").
- ITEM 246 is not re-typed here: answer-hold.md replaces it whole (its RULE spells out ways a, b and c). If that file were missing at the merge, the paper's block would stand, and its sentence "the app only has to match what DJs, bands and DJ/producers do" must then be read as his words say: match DJs or bands; DJ / producers are looked at too (F6).
- DEPENDS ON E: the Video box of Record Show is E3-3 alone (J-1 is NO now and says "names only"); one number for a knob that is let go, a quarter of a second (old R199 (e), E's E-32); how a cell's knob records in Studio is E's item 257.
- DEPENDS ON F: Snapshot's key is F3-13 alone; AMEND 206 2 here says "no key of its own", as F's item 236 (1) does. AMEND R198 1 is the Undo sentence that F's ruling asked of this topic. F3-1 and J3-10 are one line (above).
- DEPENDS ON G and B: J3-9 keeps All Outputs Off, Restore Last Outputs and Syphon out of the mapping, so no stray pad ends the hold of page item 265. "master cue" keeps its place in the list of old item 206 until B's question to him is answered.
- MESSAGES (old item 209, AMEND 209 2): eight things may speak now; the eighth is page item 230 (an action saved in Studio whose clip is gone). Topic E's readings of that item (a layer that is gone; another show open in the live window) lean on the same case; any further message that another topic's rule lets the app show needs a line in 209 or goes.
- BULLETS OF THE PAPER THAT A RULING CANNOT REPLACE (the merge copies them; read them so): SUMMARY, last bullet: one question (J3-8) and two lines (J3-2, J3-10), not three lines. REACHES, "TOPIC E: J-1 (changed here)": void. REACHES, TOPIC F: "empties the layer (BF256)" is topic F's reading (its F3-3); his words are "removes it from the layer strip, and it does not play". NOT DONE, first bullet (246 without an answer): overtaken by answer-hold.md; fourth bullet (J3-2): lifted to a LINE here.
- FOR THE BUILD, not for him: "know how they both work" (BF270) is his order to learn how endless knobs send their steps: one Researcher task before the mapping is built (item 270 (6)); its result also says how far the app's own guess of a knob's kind can be trusted (J3-6). Which controller he uses is still not known, and by his words the app must not depend on it.
- THE LIST OF NAMES: Studio in every row the paper lists under REACHES; "Review" and "Show Recording Review" go to the words that are no longer used (four NAME blocks here); "Endless" (the toggle) and "Studio picture" are Harmony's picks.

### CONFLICTS (his newest words against earlier ones)
- Endless knobs. New (BF270): "We need to work with both and know how they both work so they both work smoothly." Old (binding-decisions.md 1191, BF232): "207 c but make an argument for why we should have a key/pad hold setting." -- way c of 207 read: no setting at all, a knob only turns, so no endless-knob setting. The new words win: the endless knob returns; the rest of c stands (hit strength, cell or clip per entry), and hold waits on the research.
- The screen's name. New (BF245): "Let's go with studio. That's perfect." Old (binding-decisions.md 1187, BF228): "Show Recording Review should be called Review for short is a good name. Do you have a better name for this?" The new word wins; he had asked for a better one himself.
- Not a conflict, said for completeness: the jump of a slider to a position-sending knob (R199 e, on the last page, left by him) against BF270 "so they both work smoothly". Put to him as the one question J3-8.

### REACHES OTHER TOPICS
- THE LIST OF NAMES (.harmony/NAMES.md), every line that carries the old word for the screen (grep -n -i "review", lines that match only "preview" left out), to be brought up to date to Studio: 21 (layout: "the live window, Review"), 22 (live window: "as against Review"), 63 (clip in point, clip out point: "Review's In and Out points"), 95 (timeline, lower case: "in Review", "Review timeline"), 114 (Quantize: "used only in Review"; its Replaces cell), 192 (action save window: "in Review"), 194 (the heading "7. Recording and the review screen"), 201 (Show Recordings: "opened from it in Review"), 203 (the row Review itself: becomes Studio, with BF245 as its source and "Review, Show Recording Review" in its Replaces cell), 206 (Review picture: the name itself and its text), 207 (track: "One row of Review"), 211 (In and Out points: "the timeline of Review"), 217 (recorded show file: "Review opens the recording"), 219 (Render: "inside Review"), 292 (Load Take..., Play Take: "played in Review"; its Use cell), 294 (replay window: "Use: Review"), 295 (recording review screen / mode, review screen: "Use: Review (long form: Show Recording Review)"), 299 (display screen (in Review): "Use: Review picture"), 310 (top-bar Quantize box: "Quantize lives in Review", "Quantize (Review)"), 340 (open names: "Review picture"), 341 (the note "Review: his L114 asks for a better name": closed by BF245), 357 (live window, new row: quotes his "live and recording review mode": the quote stays), 361 (timeline: "timeline of Review"), 362 (clip in point: "Review has In and Out points"), 366 (Review picture, new row), 369 (the note "Review: replaces now also ..."). A new retired-words row is needed: "Review, Show Recording Review -- your words of 2026-10-07. Use: Studio."
- TOPIC E (owns Studio's functions): every RULE of spec-E, and of the other spec files, that says "Review", "the review screen" or "Review picture" reads Studio from now on (J3-5 proposes "Studio picture" and "Studio timeline"; E or the list of names decides). In BF241 his "the Recording Review" is this screen.
- TOPIC E: J-1 (changed here) reads BF241 and BF253 as: a film in full quality comes from a render in Studio, not from a Video box of Record Show. E owns the boxes of Record Show and rules it; J-1 is to be shown with E's line, once.
- TOPIC E: how a turn of an endless knob enters a show recording and a Record Over (the slider's values, not the knob's steps) is E's; E-32 ("let go" for a knob, about one beat) against R199 (e) (a quarter of a second) is still two numbers.
- TOPIC D (R224, named under MADE FROM of item 245): one @@AMEND above changes only the screen's name in its part (a). Its quote of his, "the recording review screen" (L72), stays as he wrote it.
- TOPIC F (item 197; the Snapshot): 244 b gives a knob on a clip's slider the same meaning as a pad on a cell in 197; 197 itself is not changed. Applied here from F's boxes: BF243 ("the user can push a shortcut button and save that to look out later") and BF257 keep Snapshot, so it stays a button in the mapping's list (@@AMEND 206 2); whether it comes with a key out of the box, and what it saves, is F's. @@ITEM R227 (c) says the spacebar is the only key that comes mapped: if F gives Snapshot a key out of the box, R227 needs one more amendment.
- TOPIC F (BF255, BF256), applied here without a change of rule: @@ITEM 208 leaves the selecting of an empty cell and a paste over a playing clip to F (F-1, F-3), which his two boxes now answer; @@ITEM R198 stands: a delete or a paste over a playing clip empties the layer (BF256), and an Undo brings the clip back into its cell and never starts it again. Whether a pad selects the cell it fires: J3-10.
- TOPIC G (BF269, 265 b; item 271): switching outputs stays out of the mapping, All Outputs Off and Restore Last Outputs included (J3-9). G owns what All Outputs Off does afterwards.
- TOPIC C (BF249, 223 b): the small window that asks which version of a preset to keep is a question the app asks before it replaces something, not a message: @@ITEM 209 already lets such questions stand; nothing changes there.
- TOPIC B (BF248): master cue and the cue / preview toggle stay in the list of @@ITEM 206 as buttons that can be mapped; what master cue is waits on B's answer to his question.
- TOPICS A and D (BF250, BF271): "Stop actions" and the tempo stop keep their mapping entries (@@ITEM R199 (d), @@ITEM P32); what each stops is theirs.

### NOT DONE / UNSURE
- Item 246 carries no answer and no assumption on purpose: the research he ordered (BF264) is written by another seat. If he then takes hold: @@ITEM 207 (its "no hold-to-play" sentence), @@ITEM D34 (returns as written), @@ITEM P33 and R199 (c) need amendments, and a hold on a BPM mode clip that ends before the 1 needs its rule.
- Which controller he uses is still not known (270's question was not answered). By BF270 the app must not depend on it; it would still settle the tuning of J3-7, the ways of sending steps that must be read first, and pad lights (@@ITEM 214, D35). Cheapest: he names it when the build of the mapping starts.
- How endless knobs send their steps is Harmony's knowledge of MIDI, not checked: several ways exist, and whether a knob's messages alone always tell the kind is not known. Cheapest: a Researcher reads the MIDI notes of three common controllers; it decides whether the Endless toggle is a safety net (J3-6) or the only way.
- J3-2 is ASK NO because he chose the wording himself with the other way on the same card. It is still the consequence of 244 b most likely to surprise on stage (after a deck switch the knob leaves the playing clip): the ruling seat may lift it to a LINE.
- The old word inside the other spec files (E above all) was not amended here, by the rule on other topics' blocks; only R224 was, as a MADE FROM block of item 245. If the merge needs every "Review" replaced block by block, that is one mechanical pass by the seat that owns each file.
- TODAY lines: 270's from BindingManager.cpp:155-198 and Binding.h:62-79 (read, not run); the others from the old blocks and area-controls, not re-read.
- Written 2026-10-09 18:53:11 EDT (from date).

### The assumptions of this topic after the ruling (YES first, then LINE, then NO)
@@ASSUME J3-8
ABOUT: 270, 244, R199
TEXT: I assume that when a slider stands somewhere else than its knob (an action, a preset or another clip in the cell moved it), a knob that sends its position makes it jump to the knob at your next turn. An endless knob never jumps.
WHY: BF270 "so they both work smoothly" may ask for no jump. The jump is in the reading of the mapping that he read on the last page and left as it was (old R199 (e): inferred consent, not an answer of his). With BF262 a knob serves whatever clip sits in a cell, so knob and slider differ often.
ALT: b) No jump: the knob does nothing until it passes the slider's value, and the slider follows it from there. c) No jump: the slider moves from where it stands, faster or slower than the knob, until the two meet.
IF-WRONG: STAGE a slider that jumps is seen on the output; a knob that waits feels dead for a moment
ASK: YES "smoothly" can be read both ways, and the audience sees a jump
@@END

@@ASSUME J3-hold
ABOUT: 246, 207, D34
TEXT: I assume your answer stands until you say otherwise: no hold setting. A pad or a key only presses, and letting go does nothing.
WHY: He chose "none" and has not taken it back; he asked whether it is worth doing. The research says yes, medium sure: way b is Harmony's recommendation.
ALT: b) Every pad and key gets one switch, press or hold, set to press until you change it. On hold, a clip, an effect's button or an action is on only while you hold, and goes back when you let go. c) The same switch, but only for an effect's button and an action; a clip's pad always only presses.
IF-WRONG: SMALL an estimate: one switch per mapped key or pad plus the release kept in a recording (D34), added later
ASK: YES a, b or c
@@END

@@ASSUME J3-2
ABOUT: 244
TEXT: I assume a knob on a clip's slider follows the deck on screen: after a deck switch it moves that slider of the clip in the same cell of the new deck, and no longer the clip from the old deck that is still playing.
WHY: BF262 "b": the way he chose says "in the deck on screen", with way c (the clip playing on the layer) beside it. By BF256 ("If a clip is playing, and I change the deck, that does not change the clip") the old clip plays on, and its knob then serves a clip that may not be playing.
ALT: b) While a clip from that cell of another deck is still playing, the knob stays with it.
IF-WRONG: STAGE he would turn a knob and the playing clip would not answer, while a clip that is not playing is changed unseen
ASK: LINE it is the wording he chose and his pads behave the same, so he will most likely wave it through; but it is the consequence of his "b" most likely to surprise him on stage
@@END

@@ASSUME J3-10
ABOUT: 206, D32
TEXT: I assume a key or pad on a cell does all that a click on the cell does: it triggers the cell and also selects it, so the Clip tab shows that clip and Copy, Cut, Paste and Delete then work on it.
WHY: BF255 "you can select the cell which triggers it and selects it" is said of a click; old item 206, which he let stand, says a key or pad does exactly what a click does. With BF256 ("deleting a clip, removes it from the layer strip, and it does not play") the selected clip is then the one just triggered.
ALT: b) A key or pad only triggers: what is selected, and what the Clip tab shows, stays as it was.
IF-WRONG: STAGE the Clip tab would follow every pad he hits, and a stray press of the Delete key would take the clip he has just triggered off the output; small to change once built
ASK: LINE best printed inside topic F's line on the same click (its F3-1), as "a key or a pad on the cell does the same"
@@END

@@ASSUME J3-1
ABOUT: 244, 206
TEXT: I assume the knob finds its slider by name: the same slider of the same effect on whatever clip sits in the cell. If that clip does not have that effect, the knob does nothing.
WHY: BF262 "b" is the page's sentence "it moves that slider of whatever clip sits there": the same slider, which on another clip can only be found by its name. The edges of finding it (the same effect twice, a source, a macro knob) are J3-11.
ALT: b) By place: the knob moves, for example, the second slider of the first effect, whatever that effect is.
IF-WRONG: STAGE a knob would move an unexpected slider of another clip; found by name it moves the right one or none
ASK: NO the plain meaning of "that slider" in the way he chose; nobody asked for "by place", and a knob that does nothing on a clip without that effect is part of that way
@@END

@@ASSUME J3-3
ABOUT: 244, 206, R199
TEXT: I assume a key or pad on a button of a clip, such as an effect's on / off, also belongs to the cell. A key or pad on an action's button stays with that action, as before.
WHY: BF262 speaks of a knob on a slider; a clip's buttons are not named. He accepted earlier that a key on an action's button follows the action.
ALT: b) A key or pad on a clip's button stays with that clip wherever it goes.
IF-WRONG: SMALL one kind of entry
ASK: NO the same rule as for the knob, and no word of his sets buttons apart
@@END

@@ASSUME J3-4
ABOUT: 244, D32
TEXT: I assume a knob whose cell is empty, or is not there in the deck on screen, does nothing. A knob moves its slider also while the clip in the cell is not playing.
WHY: BF262 "whatever clip sits there" does not say what happens with no clip there, or with a clip that is not playing.
ALT: b) The knob works only while the clip in its cell is playing.
IF-WRONG: SMALL an edge
ASK: NO a knob does what a drag of the slider does, which he accepted; the empty cell is an edge
@@END

@@ASSUME J3-5
ABOUT: 245, R197
TEXT: I assume Studio is the whole name: no long form. The picture at its top, its timeline and its tracks are named after it, for example "Studio picture".
WHY: BF245 "Let's go with studio." names the screen; his earlier long form "Show Recording Review" and the names built on the old word are not mentioned.
ALT: b) A long form stays for the manual, such as "Recording Studio".
IF-WRONG: SMALL a name in the list of names
ASK: NO names; the list of names shows each one and he can change any
@@END

@@ASSUME J3-6
ABOUT: 270, 207, P33
TEXT: I assume every knob in the mapping has an Endless toggle. The app sets it by itself when you map the knob, from what the knob sends, and you change it where the app got it wrong.
WHY: BF270 "If we have to, we could ask the user to set a toggle if it's an endless encoder." Whether we have to is a technical call: a knob's messages alone may not always tell the kind (Harmony's knowledge of MIDI, not checked against any controller; item 270 (6) has it looked up).
ALT: b) No toggle is shown unless the app cannot tell. c) No guessing: you always set the toggle yourself.
IF-WRONG: SMALL it shows while he maps the knob, not in a show
ASK: NO technical; his "If we have to" leaves the call to Harmony, and the toggle costs him nothing where the app guesses right
@@END

@@ASSUME J3-7
ABOUT: 270, D33
TEXT: I assume an endless knob moves a slider from end to end in about as much turning as an ordinary knob needs, and a faster turn moves it farther where the controller sends that.
WHY: BF270 "so they both work smoothly" gives no number for how far one step moves a slider.
ALT: b) Finer: several full turns from end to end, for slow, exact moves.
IF-WRONG: SMALL one number, tuned with his controller
ASK: NO a number to tune once a controller is on the table
@@END

@@ASSUME J3-9
ABOUT: 271, 206
TEXT: I assume All Outputs Off, Restore Last Outputs and Syphon's on / off count as switching an output: none of them can be put on a key or pad either.
WHY: Item 271, accepted as written, says "switching an output screen on or off"; it names neither the two commands nor Syphon, which is an output but not a screen ("syphon is an output and treated with same output settings as a screen", binding-decisions.md 834).
ALT: b) All Outputs Off may go on a key or pad, as a panic button. c) Syphon's on / off may go on a key or pad: it is not a screen.
IF-WRONG: SMALL one entry more or less in the mapping; the Cmd key for all outputs off stays
ASK: NO the item's own reason (no stray hit blanks a projector) covers all three, and a stray hit on Restore Last Outputs would end the hold of All Outputs Off (BF269 "b")
@@END

@@ASSUME J-1
ABOUT: R197, 206
TEXT: I assume "All else is gone" is about names only: the two recordings are called Record to Clip and Record Show, no other word is used for them, and the sentence takes away nothing that records.
WHY: L114 "All else is gone" follows a sentence about names, and his newer words keep the other things that record (BF253 "It defaults to running next to the show recording"; BF241 "we can make an HD render from the Recording Review"). The one thing the sentence could still take away, the Video box of Record Show (his "48 b", binding-decisions.md 867), is asked once, by topic E (its E3-3).
ALT: b) The sentence also takes away the full-size film box of Record Show.
IF-WRONG: SMALL a film box is small to add or to remove
ASK: NO the Video box is topic E's line (E3-3), printed once, and its text is that the box stays; nothing else of this reading is in doubt after BF253 and BF241
@@END

@@ASSUME J3-11
ABOUT: 244, 206
TEXT: I assume that when a clip carries the same effect twice, the knob moves the one in the same place among them; a source's slider is found by the source's and the slider's name, and a clip's macro knob by its number.
WHY: BF262 "b" says "that slider of whatever clip sits there"; it does not say which of two effects of one name is meant, nor how a source's slider or a macro knob is found on another clip.
ALT: b) With the same effect twice the knob moves both.
IF-WRONG: SMALL an edge of finding a slider by its name
ASK: NO technical; an edge he would not notice
@@END

@@ASSUME J-16
ABOUT: R199, 244
TEXT: I assume a mapping entry that points at a layer or an action the open show does not have is kept, does nothing and is shown as missing. An entry on a cell is never missing: it waits for a clip that has that control.
WHY: L116 and his earlier import words do not say what happens when two shows differ. BF262 "b" makes a knob on a clip's slider belong to the cell, so a cell that is empty, or whose clip has no such slider, is the normal case and not a fault.
ALT: b) Entries that point at something the show does not have are dropped at import.
IF-WRONG: SMALL list housekeeping
ASK: NO internal; nothing fires either way
@@END

@@ASSUME J-19
ABOUT: 208, R198
TEXT: I assume the Edit menu follows the Mac's habits: an entry is grey when nothing is marked; in a box for typing, Cut, Copy and Paste work on the text; Delete never asks first, also when the clip is playing.
WHY: 208 A names the six entries; whether Delete asks first is in no line of his. BF256 ("deleting a clip, removes it from the layer strip, and it does not play") describes the delete as an act with an immediate effect and names no question; but the old "(Undo brings it back)" no longer holds whole: an Undo brings the clip back into its cell, not onto its layer (@@ITEM R198 as amended).
ALT: b) Delete asks first when the clip is playing.
IF-WRONG: STAGE a stray Delete takes a playing clip off the output, and Undo does not put it back on its layer; a question box is small to add
ASK: NO his words describe the delete as immediate; topic F's line on the triggering click (its F3-1) shows him what Delete then works on, and the page ruling is asked to let it say "without asking"
@@END

### Items of page 2 that are still OPEN in this topic: 246

## TOPIC K -- Sources and the automatic features
### The topic ruling's notes for the page ruling
- ONE QUESTION from this topic: K3-1 (YES), what "connected to the BPM" means for an audio file. Print it right after ANSWER general: his "audio clips" (BF272) and his "all clips from all layers" (BF271) point to its way b, an audio file as a clip in a cell. If he takes b it is a new kind of clip: topics H, A, F and E must then rule how it plays, stops, is saved and is recorded, and item 273 parts 2 to 5 are written again.
- LINES, by weight: K3-2 (after tempo stop the audio file is back at its beginning and silent until he starts it; print it under K3-1, same subject). K3-5 (no picture of a many-picture clip is cut in BPM mode; topic H's item 238 point 10 hangs on it: asked once, here). K3-8 (a snapshot does not bring back the MilkDrop preset that was showing; it can ride as one clause on topic F's snapshot question F3-6).
- NOT FOR HIM (NO): K3-3 (the tempo stop takes a clicked MilkDrop picture away; carried by BF196 and BF271; if topic A's line A3-3 is printed, add "MilkDrop you clicked goes too"), K3-4, K3-6, K3-7, K3-9, K-2, K-23. Old internal ones untouched: K-7, K-9, K-12, K-13, K-14, K-16, K-20, K-21, K-22.
- ANSWER general is owed and must be printed: he asked. It says yes to MP3 and M4A and says plainly that neither was tried in the app.
- NEWEST WORDS AGAINST EARLIER ONES, one line to say back, no decision needed: he now designs the new MilkDrop himself, while the build runs (BF265), where he had said "we will do that as a dedicated session where I will design the UI and how we will use it but not right now" (BF235). Until his design arrives nothing of MilkDrop is planned, changed or built, its labels in bars included (R212; K3-4).
- OVERTURNED READING, nothing to put to him again: R228 part a (a click in the MilkDrop tab no longer reaches the output) was a reading he left standing; BF265 answers it as item 247.
- DEPENDS ON A: the stop (R176 a as amended there) is the ground of item 273, K3-3 and K3-7. Topic X's AMEND N18 1 (a second stop still stops the audio file) leans on item 273.
- DEPENDS ON E: item 259 and AMEND R165 2 lean on item 273 and K3-2 (silence after a stop; opening Studio stops the audio file). E3-3 (the Video box) decides how early Render must come in this topic's order of work (K-20). E's NAME Render and this topic's are word for word the same.
- DEPENDS ON F: item 236 leaves the picked audio input out of a snapshot, as item 272 has it. K3-8 borders F3-6: if he answers F3-6 with way b or c (a snapshot does not bring the clips back), K3-8 falls away.
- DEPENDS ON H AND I: H's item 238 point 10 follows K3-5. AMEND R203 1 says only that the autopilot's count per kind of layer stays; it was written before topic I's ruling was finished (its paper and its checker both read BF260 so). If that ruling takes a layer's kinds out after all, R203 part c follows it. Topic I's ruling (its H2) leaves the generated pictures that read the beat to this topic: AMEND R201 1 and K3-9 (NO) hold their beat part still in a pause or a stop, as item 243 does for effects; if his answer to I3-6 gives effects a Sync, whether these pictures get one too is a new question, not asked now.
- NOT MEASURED, and not said to him as fact: whether an MP3 or an M4A opens in the app. Read, not run: the file window lists wav, aiff, aif, mp3, flac and ogg (MainComponent.cpp:310, :547); JUCE's own MP3 reader is off by default and not switched on in CMakeLists.txt, so on a Mac an MP3 or M4A would go through Apple's reader (juce_AudioFormatManager.cpp, registerBasicFormats). Cheapest check when the build hold ends: open one of each. ffmpeg and afconvert are on this Mac (seen by ls, not run).
- MERGE: this topic amends R228 (1 to 4), 215 (1 to 4), R202 (1, 2), R204, 211 and R203, and R213 and R201 (both appended). No other topic's paper or ruling amends these items (grep over all papers and the rulings written so far; topic I's ruling was not finished). Every OLD text was found exactly once in its old RULE line.

### CONFLICTS (his newest words against earlier ones)
- MilkDrop's click: new, BF265 "Keep Milk drop as it is." against the reading R228 a that he left standing on 2026-10-07 (MilkDrop reaches the output only through a cell that plays; binding-decisions.md line 1199, "THE READINGS HE DID NOT NAME ... stand as shown"). His newest words win; it was asked as item 247 and is answered, so there is nothing to put to him again.
- Who designs the new MilkDrop and when: new, BF265 "When I have time while you are building, I will design a whole system for Milk drop" against binding-decisions.md line 1194: "we will do that as a dedicated session where I will design the UI and how we will use it but not right now". A shift of when, not of what: he designs it either way. Newest words win (R202 part 3 amended).
- The audio file and the stop: no conflict with words of his. L72 (binding-decisions.md line 1155: "The tempo stop button stops all actions as well as everything else.") agrees with BF271; what falls is Harmony's narrower reading of L72 (old K-11).

### REACHES OTHER TOPICS
- A (R176 a, R214, R216): BF271 "Stop removes all clips from all layers" is A's rule for the stop; applied here only to the audio file (item 273). A's paper says the same of K (apply-A.md, its bullet on item 273).
- A (R176 a, R216): if K3-3 holds, the tempo stop also takes away MilkDrop that was put on the output by a click in the MilkDrop tab; if he strikes it, A's "with no clip playing the output is black" gains MilkDrop as an exception after a stop.
- A (the pause, old A-3): BF271 "A pause would not pause it unless it is connected to the BPM" restates his rule of 2026-10-07 that a pause holds what the BPM controls; it supports the reading that a clip not in BPM mode plays on through a pause. Not ruled here.
- E (page item 259, record show runs on through a tempo stop; R165, opening Studio is like a press on stop): the audio file stops at a tempo stop, so a show recording that runs on records silence from there if the audio file was its sound; and opening Studio stops the audio file if it stops the show like a press on stop. E rules both.
- E (the old reading on Render): 215 part f now reads "an HD film file ... in Studio" (BF241, BF245); E's own Render reading should say the same. K-2 no longer offers "Render is gone".
- H (page item 238, BF258 "b"): whether the cut to whole groups of 4 bars applies to a clip of many pictures (old item 211); K reads it as no (K3-5).
- B (page item 252, old B-11): BF265 "Keep Milk drop as it is" also means no second MilkDrop for the preview; B's paper already applies it.
- F (BF243, the snapshot that saves "all the settings"): item 272 keeps the audio input out of the show; whether a snapshot holds it is F's, and K's reading is that it does not.
- J (NAMES.md): the row Review is replaced by Studio (BF245, BF263); the rows Audio Input, audio file, Loop File and Audio play / pause stand. Applied here from J's box: the name Studio.
- D (BF251, a play-once action plays again "only when that clip is re-triggered"): a trigger by the autopilot is a trigger like his own (R213 a), so it plays the clip's play-once actions again. No change to R213; D rules the action.

### NOT DONE / UNSURE
- Which audio file types open: READ, NOT RUN. The file window's list is "*.wav;*.aiff;*.aif;*.mp3;*.flac;*.ogg" (MainComponent.cpp:310 and :547); the reader is JUCE's registerBasicFormats (AudioEngine.cpp:8, :60), which on a Mac also registers Apple's Core Audio reader (juce_AudioFormatManager.cpp:76-78 in build/_deps). INFERRED and not claimed to him: that reader would open an M4A, and also the MP3 if JUCE's own MP3 reader is off; no JUCE_USE_MP3AUDIOFORMAT line was found in CMakeLists.txt. docs/claude/architecture.md:169 lists WAV/AIFF/FLAC/MP3/OGG. Cheapest check when the build hold ends: open one MP3 and one M4A through the Audio File entry. Until then an M4A is converted with /opt/homebrew/bin/ffmpeg (present on this Mac, seen by ls; not run).
- "connected to the BPM" (BF271): not settled; K3-1 asks. If he means way b (an audio file as a clip in a cell), that is a new kind of clip and topics H and A must rule its playback.
- Whether the new tempo stop clears MilkDrop's loose picture (K3-3) depends on how topic A's stop is built (a clear of every layer refreshes the loose picture away, docs/claude/milkdrop.md line 46: INFERRED there). Cheapest: decide it in the stop's plan and say it on his page as one line.
- The old assumptions K-7, K-9, K-12, K-13, K-14, K-16, K-20, K-21, K-22 were tested against his 33 boxes and left untouched: no box of his speaks of the time per picture, the autopilot's count on a still, the Text source, OSC, Link, Play Once and Eject in a pause, the order of the six planned things, Loop File or the return of a pulled-out input. BF270 (endless knobs) is MIDI, not OSC; BF246 (the app tries to find the 1) is automatic mode without Link.
- A clip deleted or pasted over while the autopilot holds it (BF256: it "does not play"): the layer goes empty and the autopilot has nothing to move on from until he triggers a clip. Read from R203 and D36 as they stand, not ruled; topic F owns the delete.
- TODAY lines are taken from the old spec and the fact sheets except the audio file path and the MilkDrop pick, which were read in the docs and source named above. Nothing was run.

### The assumptions of this topic after the ruling (YES first, then LINE, then NO)
@@ASSUME K3-1
ABOUT: 273; R204, 215
TEXT: I assume an audio file the app plays is the music the app listens to, with its own play control: not a clip on a layer, and nothing connects it to the BPM. So tempo stop stops it, and tempo pause never pauses it.
WHY: BF271 reasons from "all clips from all layers" and says "unless it is connected to the BPM"; what would connect an audio file to the BPM is in no line. In his mouth the phrase means what follows the beat clock ("everything that is connected to BPM shifts forward or back. I mean everything.", binding-decisions.md:859-860); for a clip that would be BPM mode (INFERRED). He also writes "audio clips" for the files he will bring (BF272). So he may see an audio file as a clip, while the build has it as the sound source only.
ALT: b) An audio file can also be a clip in a cell, with Timeline mode and BPM mode like a video: tempo stop takes it off with the other clips, and in BPM mode tempo pause pauses it. c) It is not a clip, but it gets one switch that connects it to the BPM: with it on, tempo pause pauses the music and tempo play carries it on.
IF-WRONG: REBUILD audio clips in cells would be a new kind of clip (how it plays, how it is stopped, saved and recorded), not a small change; and on stage the music would pause, or play on, against what he expects
ASK: YES no words of his settle it; it is about what the app does; way b is a large piece of work he may be taking for granted
@@END

@@ASSUME K3-2
ABOUT: 273; R204, 215
TEXT: I assume that after tempo stop the audio file is back at its beginning and stays silent until you start it with its own play control; tempo play or a triggered clip does not start it.
WHY: BF271 says the stop stops it; where it stands afterwards and what starts it again is in no line.
ALT: b) The audio file starts again together with the beat: on tempo play or on the first clip you trigger. c) It keeps its place and goes on from there when you start it.
IF-WRONG: STAGE silence, or music, where he expects the other after a stop; a small change to undo
ASK: LINE a real choice of Harmony's that follows his "it would stop"; he can strike it
@@END

@@ASSUME K3-5
ABOUT: 211; page item 238
TEXT: I assume a clip of many pictures never loses a picture in BPM mode: all its pictures are spread over whole groups of 4 bars, or each stays a set number of beats, and none is cut off the end.
WHY: With "b" (BF258) he chose the page's way b for item 238, which cuts a new BPM-mode clip's end so that it is whole groups of 4 bars and "plays at exactly its normal speed" (the page's words); a clip of many pictures has no normal speed, and his words do not name it. Default A of question 211, which he took (BF154: "All defaults good except for these."), keeps such a clip spreading all its pictures over its length. Topic H's ruling builds on this reading (its item 238, point 10) and leaves the asking to this line.
ALT: b) A clip of many pictures is cut like a video: the pictures past the last whole group of 4 bars are not shown.
IF-WRONG: STAGE pictures missing from the end of a slideshow, or a slideshow that is not cut where he wanted it cut; small to change
ASK: LINE it is where his answer to item 238 meets the many-picture clip; topic H leaves it to this line, and he would most likely wave it through
@@END

@@ASSUME K3-8
ABOUT: 247; page item 236
TEXT: I assume a snapshot does not bring back the MilkDrop preset that was showing: MilkDrop stays as it is, and nothing remembers which preset was on. The picture kept with the snapshot shows how the moment looked.
WHY: BF243 has a snapshot save "the show exactly where it is with all the settings and the output and everything"; BF265 keeps MilkDrop as it is, and as it is no show keeps which preset is showing and a MilkDrop clip loads no preset of its own when it is triggered (docs/claude/milkdrop.md 1.8 and 1.10 point 1; read, not run). His two answers of this round meet here and neither names the other.
ALT: b) A snapshot remembers the preset that was showing and loads it when the snapshot is opened: likely a small change to MilkDrop in the coming build.
IF-WRONG: SMALL he opens the snapshot of a MilkDrop moment and sees another preset; the exception is small to add later
ASK: LINE a limit of the snapshot he should know before he relies on it; his own answer on MilkDrop is the text and the exception is way b; it borders topic F's snapshot question (F3-6) and can ride on it
@@END

@@ASSUME K3-3
ABOUT: 247; R228, R202
TEXT: I assume tempo stop also takes away a MilkDrop picture you put on the output by clicking a preset: the output is black until you click a preset again or trigger a clip.
WHY: BF265 keeps MilkDrop as it is and the tempo stop is new, so no line of his names this meeting. His own words carry the reading: BF271 "Stop removes all clips from all layers so it would stop." (he reasons that the stop takes everything, the audio file too) and "The tempo stop button stops all actions as well as everything else." (BF196, binding-decisions.md:1155).
ALT: b) Tempo stop removes only clips: MilkDrop that you clicked in its tab stays on the output.
IF-WRONG: STAGE with way b he presses stop and MilkDrop is still on the output; with the text it is gone when he wanted it kept, and one click brings it back
ASK: NO his words on the stop carry it ("everything else"), and the other way is the one that would hurt on stage; if topic A's line on the stop is printed, six words there cover it
@@END

@@ASSUME K3-4
ABOUT: 247; R202, R212
TEXT: I assume your MilkDrop design comes to me whenever you have it, and until then nothing of MilkDrop is planned, changed or built, its labels included.
WHY: BF265: "When I have time while you are building, I will design a whole system for Milk drop"; when and how it is handed over is not said.
ALT: b) Harmony drafts a proposal for the new MilkDrop system for him to correct.
IF-WRONG: SMALL a draft he did not ask for, or a wait
ASK: NO the order of work; his words already say who designs it
@@END

@@ASSUME K-2
ABOUT: 215
TEXT: I assume Render is one button in Studio that makes a film file of the recording you have open: the whole recording, at the size of the full composition, with the sound that was recorded with it.
WHY: BF241 names "an HD render from the Recording Review", so Render stands on his word and the old way "Render is gone" falls; "HD" is read as the full composition size. Sound in the film is his September yes ("do this but triage the correct build order.", binding-decisions.md:235), and item 229, which he accepted, plays the show recording with its sound; BF253 "It has no sound." speaks of the low-resolution show recording only. Still open: the whole recording or a stretch of it.
ALT: b) Render makes a film only of the stretch between the In and Out points.
IF-WRONG: SMALL a planned thing, not built yet
ASK: NO what Render makes returns to him with Render's plan (item 215: none of the six is built before its own plan is clear); nothing of it is in the coming build
@@END

@@ASSUME K-23
ABOUT: R228
TEXT: I assume only the two picture buttons and a picture dropped outside the grid stop feeding the output; the remote-control and test calls that load a loose picture keep working, and so does a click in the MilkDrop tab.
WHY: BF265 keeps the MilkDrop click; the calls the test tools use for the same slot are in no line.
ALT: b) The slot goes altogether except for MilkDrop, and the test tools are rebuilt on clips.
IF-WRONG: SMALL the test tools would need rework; nothing he sees
ASK: NO technical; the remote-control port is not his concern
@@END

@@ASSUME K3-6
ABOUT: 247; R228
TEXT: I assume a MilkDrop picture you put on the output by a click stays until a clip really starts: while a triggered clip in BPM mode waits for the 1, MilkDrop keeps showing, and the clip takes its place on the 1.
WHY: BF265 keeps MilkDrop as it is; the wait for the "1" is new, so no line of his names this moment. The reading follows the default he took for a waiting clip (item 173 of topic A; BF154: "All defaults good except for these."): what was showing goes on until the "1".
ALT: b) The MilkDrop picture goes at the trigger: the output is black until the 1.
IF-WRONG: STAGE up to a bar of black before the first clip comes in
ASK: NO it follows the default he took for a waiting clip; the other way is a gap of black that nobody would choose
@@END

@@ASSUME K3-7
ABOUT: R213, R203
TEXT: I assume the autopilot never brings a clip onto an empty layer: after tempo stop, or after you cleared a layer or deleted its playing clip, a layer on autopilot stays empty until you trigger a clip there.
WHY: BF271 "Stop removes all clips from all layers so it would stop." empties every layer; what a layer on autopilot does then is in no line, and topic A's paper left it to this topic. The reading he left standing in round 1 has the autopilot bring "the next clip", which needs a clip on the layer; BF256 has a deleted or pasted-over clip leave its layer.
ALT: b) When the beat runs again, a layer on autopilot brings a clip by itself.
IF-WRONG: STAGE clips would come on by themselves after a stop, or a layer he expected to fill stays empty until he triggers it
ASK: NO it follows his own sentence on the stop and the autopilot he left standing; a layer that fills by itself after a stop would go against "Stop removes all clips"
@@END

@@ASSUME K3-9
ABOUT: R201; page item 243
TEXT: I assume a generated picture that reads the beat by itself does as effects that read the beat do: while the beat is paused or stopped, the part of it that pulses on the beat holds still, and the rest keeps moving.
WHY: Item 243, which he accepted, names effects (a strobe, a pulse); about 52 generated pictures name a beat uniform in the program text (topic I's ruling; read, not run), and no line of his names them. His rule carries the reading: "Pause pauses, the beat clock, the clips and everything that it controls with BPM." (BF167); way b of item 243 (they keep pulsing through a pause) is the one he did not take. Whether such a picture gets a Sync like those effects (BF244) is not in his words; it waits on topic I's question about the Sync (its I3-6).
ALT: b) Generated pictures keep pulsing on the beat the app hears, also while the tempo is paused or stopped.
IF-WRONG: STAGE a picture that goes on pulsing during a pause, or holds still when he expected it to pulse; small to change
ASK: NO his rule on the pause and the item he accepted for effects carry it; it is the same answer as for effects, so there is nothing new for him to read
@@END

## TOPIC X -- The loose lists
### The topic ruling's notes for the page ruling
- Topic X puts NOTHING of its own to him: after this ruling its five written assumptions are all NO (X-5, X-13, X-16, X-17 re-written here; X3-3 as the paper has it), with the ten old internal ones untouched (X-12, X-14, X-15, X-18 to X-21, X-23, X-25, X-26); X-22, X3-1 and X3-2 are dropped. YES 0, LINE 0.
- What X's rules lean on is asked by the owners, once each; in order of weight for this topic: (1) who holds the 1 after a start press and after a Resync (tempo topic; C14, and every X rule that says a start press "is the new 1": C1, C2, C16, C22, N18, U17); (2) what the tempo stop does to action buttons (actions topic; N18, X-17); (3) how long All Outputs Off lasts (output-screens topic; C19); (4) what "adjust the sink" asks for (effects topic; C26, N23); (5) the tap (tempo topic; N9); (6) the preview's fps and size (cue topic; N4).
- Newest words against earlier ones, each to be said to him once, by its owner and not from X: a preview at once (BF247) against "triggered on the 1" (BF175): cue topic. Keying stays (BF260) against "remove the keying and slider" (BF226): effects topic. The nudge goes to 0 at every start (item 219, accepted with an empty box) against his "129 b" and "good (this is just nudge amount)": tempo topic, one line. All Outputs Off holds (BF269) against "I expect the show to remember the outputs connected" (BF216): output-screens topic. The app places the 1 (BF246) against "click on play or any clip is the new 1" (BF166): tempo topic. A clicked MilkDrop preset shows with no clip playing (BF265) against the black output he had left standing: sources topic.
- The paper's CONFLICTS section is carried over as it is and lacks the fourth and fifth of these (checker F7); they are not X's to add.
- Written while the effects ruling had its verdicts but not yet its blocks: C26, N23, N13, U13 and X-16 agree with those verdicts (the sync is asked as "adjust the sink"; the order of work is internal there; the two gaps of BF261 are its two lines for him). If its blocks say more, these five X blocks are read with them. U17 follows the sources ruling's item 247 as it stood when this ruling was written.
- MERGE, dry run made for this ruling: all 30 amendments of X apply. U18 is now amended by the cue topic alone and the shared sentence of C13 by the clip topic alone; X keeps C13 1 (another sentence). The one amendment that dry run could not apply was topic I's AMEND R214 1, which the effects ruling says it drops (its H1).
- MERGE: after all amendments "Review" still stands, outside quotes of his, in seven RULE lines of spec-X (C10, C11, C12, N5, U1, U5, U11: counted by a script; the paper's list named C20, N6 and U21, which do not carry it, and missed U1 and U11) and in the internal X-12 and X-14; the new @@NAME Review says how they are read. One replace pass at the merge would be cleaner; no amendment was written for the name alone.
- C18 (messages) now counts as the keyboard topic's item 209 does after its ruling: eight messages, and the questions before a replacement apart. If item 209 changes again, C18 follows it.
- N18 and X-17 now give a stop pressed on a stopped tempo its work (BF250, BF271). The tempo topic's internal A-16 still says "a second stop" changes nothing: it should follow, or say why not.
- C23 (Random) is closed in X by BF219, as the ruling that made page 2 settled it; U7, N15 and U12 still say "asked once" and point to C23, which now says settled. The clip topic's item 201 carries the rule and still names H-1 as asked: the same stale pointer there.
- N12 stands: asked on page 2 which MIDI controller he uses (item 270), he named none (BF270); pad lights for the new buttons keep waiting on that. One line for the keyboard topic if the next page has room.
- Still owed by Harmony before the plan is called clear, none of it for him: the sweep of the Preferences window, the store of recorded sound and the remote-control port (U20, X-25).
- Nothing in X is measured: N4, N5, N6, N20, N21, N22, N24 stay Harmony's own looks and measurements.
- Ruled 2026-10-09 19:56:23 EDT by the ruling seat of topic X. Read-only; nothing built, run or committed.

### CONFLICTS (his newest words against earlier ones)
- Preview on the beat or at once. New, BF247: "I think I want to change this. Previewing a clip should not happen on the beat. It should just be quick" -- against his earlier "a previewed clip (clicking its name) shows the clip with all its actions and it is triggered on the 1. Resolume plays it right away but we have actions so that will need to be playing in time with the music" (binding-decisions.md:1134). The new words win (N2, U4, U18).
- Keying. New, BF260: "All the blend modes and keying stay, and they will be built when there is time." -- against his earlier "204 I want to remove the keying and slider. We are only going to use the transparency slider to control that layers blend mode." (binding-decisions.md:1185). The new words win (N13, X-16).
- The nudge amount and the show. Item 219, which he accepted with its box empty (the nudge number goes back to 0 at a start after a pause or a stop), against his earlier "good (this is just nudge amount)" on a show keeping its nudge amount (binding-decisions.md:916). No new words of his: the accepted item wins, and the kept amount has almost no life left (C15, X-5; the tempo topic carries it).
- The output is black with no clip playing (a reading he left standing, U17) against "Keep Milk drop as it is." (BF265, on item 247, whose text says a clicked MilkDrop preset still shows on the output). His words win for the coming build (U17 amended).

### REACHES OTHER TOPICS
- A (tempo bar): BF246 and BF242 with items 217, 219, 248 and 249 were applied here in C2, C14, C15, C16 and X-5. How long his Resync holds against the app's own search for the 1 ("always", BF246, against way b's "until your first Resync") is A's to rule; C14 only points there. N9 now waits on his reply to BF266 (item 250), which A puts to him again.
- A and D (tempo stop): BF271 "Stop removes all clips from all layers" and BF250 "tempo stop stops all actions, not just global" fit the X blocks that mention stop (N8, N18: "stop still clears the layers", "stops and clears as any stop"); neither block says what stop does to actions, and none was amended. A and D rule it.
- B (previews and cue): BF247 was applied here in N2, U4 and U18; item 221 in U18; item 253 in U4; BF267 in N4. Left to B: when a previewed clip's actions start while the tempo runs; what "loaded into the layer, and we are cueing this way" covers; the reply he is owed on "come first" and on the frame rate (item 252); what and where the master cue is (BF248, item 222) -- U10 does not speak of the master cue and was not amended.
- C (presets): BF249 (a small window asks which preset version to keep) was applied here only as one more place where the app speaks by itself (C18). BF252 (item 228 b: after an action the slider glides back to its value from before the action; the preset's value for it is lost) fits C5, C7 and C8 as they stand; no amendment.
- D (actions): items 224 and 225 close what C5, C7, C8 and C9 held open (one Global glide slider; one layer switch against global actions); only C9 was amended, the others already say it. BF251 (a play-once action on a clip keeps its button on and plays again only when the clip is re-triggered) does not touch a RULE here: C8 says what the controls do, not what the button does.
- E (recordings, Studio): items 229, 257 and 262 and BF268, BF241 were applied here in C20, U21, N24 and U16. BF241 ("Maybe something close to the output monitor resolution? 30 fps is the fastest or even 15 could be ok.") stands against the accepted item 262 (a quarter as wide and as high): E rules it; N24 names no size.
- F (show file, Snapshot): item 235 was applied in U19. BF243 and BF257 make Snapshot a save of the whole show at one moment; U8's RULE only mentions the snapshot in passing and was not amended. NAMES.md line 220 (Snapshot = a still picture) no longer holds: F's.
- G (outputs): item 263, item 237 and BF269 were applied to Syphon in C19 (X3-1). What ends the hold after All Outputs Off (Restore Last Outputs, a quit) is G's.
- H (how a clip plays, codecs): BF258 was applied in C13, items 266 and 267 in C25. Left to H: a clip shorter than 4 bars; what "continue" still means once a re-trigger of the playing clip always restarts; the codec test (BF240) that N24 points to; his question "Why would we make HAP copies at all?" (BF259).
- I (effects): BF260 was applied in N13 and X-16, BF261 in U13, item 243 and BF244 in C26 and N23. What a mask is, when keying is built, what the sync control and the common macro of the beat-reading effects are: I's.
- J (keyboard and MIDI mapping): the name Studio (BF245, BF263) is J's; here one @@NAME block. N12 (no pad lights until he names a controller) stands: BF270 names no controller, it asks for both kinds of knob. U8 stands with item 269.
- K (MilkDrop, audio): BF265 was applied in U17 and closes X-22. BF271's second sentence ("A pause would not pause it unless it is connected to the BPM") fits C26 as it stands (pause holds what the beat clock controls and nothing else).

### NOT DONE / UNSURE
- The name: "Review" still stands in the RULE lines of C10, C11, C12, C20, N5, N6, U5, U21 and in the assumptions X-12 and X-13. They are to be read as Studio; I wrote no amendment for the name alone. Cheapest: one replace pass by the merge script.
- C23, U7, N15, U12 (Random) still read OPEN in spec-X.md although the slice lists their assumption X-28 as SETTLED before page 2. Nothing on page 2 touches Random, so nothing is written here. Cheapest: the ruling checks that the clip topic's block carries the settled rule.
- C5, C7 and C8 still say "is asked (X-27)" and C6 says "see X-3"; items 224 and 225 close both, and the RULE texts already say what holds, so by the rule of this paper they are not amended.
- X-5 and the nudge: I followed the tempo topic's reading (a show's nudge counts only when the show opens onto a running beat). Whether a show should keep a nudge at all is its way b; not asked.
- C25: what "continue where it left off" still does once a re-trigger of the playing clip always restarts (item 266) is not said by the item; left to the clip topic.
- N4: the wording of the reading put to him again follows the cue topic's paper (apply-B.md, item 252), read by me, not ruled yet.
- Nothing about the app was measured or run for this paper; the one file of the app's documents I read is docs/claude/milkdrop.md (for X-22).
- written 2026-10-09 18:54:45 EDT

### The assumptions of this topic after the ruling (YES first, then LINE, then NO)
@@ASSUME X-5
ABOUT: 219, C15
TEXT: I assume the nudge amount is counted only in automatic mode; in manual mode a nudge just moves the 1. A show still keeps the amount, but it counts only when the show is opened while the beat runs.
WHY: Item 219 (accepted) sets the amount to 0 at every start after a pause or a stop; his earlier "good (this is just nudge amount)" (binding-decisions.md:916) kept it in the show; together they leave the kept amount almost no life. L14 shows the amount only in automatic mode and does not say whether one is counted in manual mode.
ALT: b) A show no longer keeps a nudge amount at all. c) The amount is counted in both modes and only hidden in manual mode.
IF-WRONG: SMALL a number on the tempo bar after a show is opened.
ASK: NO bookkeeping of one number; what he sees at a start is settled by item 219; the twin of the tempo topic's line on what a show keeps of the nudge.
@@END

@@ASSUME X-16
ABOUT: 240, N13
TEXT: I assume nothing of the blend, keying and transition lists changes until that part is built, and that every blend mode and every keying entry stays. Before that I settle the small difference between my two count tables.
WHY: BF260 keeps them ("All the blend modes and keying stay, and they will be built when there is time."), against his earlier L111; L110 puts the lists last; the two count tables differ by one or two. BF260 does not name the transition list, so nothing is said here of its entries.
ALT: none
IF-WRONG: SMALL one or two list entries.
ASK: NO technical: an architect reads both tables before that part is planned; the order of work is held by the effects topic, internal there too.
@@END

@@ASSUME X3-3
ABOUT: 241, U13
TEXT: I assume the show keeps both kinds of envelope, the one along the beat and the one along the clip's playhead, with the slider or sliders each one drives.
WHY: BF261 adds the playhead kind; that a show keeps envelopes rests on a reading he left standing, said before there were two kinds.
ALT: none
IF-WRONG: SMALL an envelope missing after a show is opened again.
ASK: NO it follows from the standing rule that the show keeps his envelopes.
@@END

@@ASSUME X-13
ABOUT: N5, 230
TEXT: I assume opening a recording in Studio never replaces the show you have open and changes none of its settings: Studio plays the recording with the show file saved with it. Only an action you save in Studio goes into your show.
WHY: L80 says the recording saves and opens its own show file; what becomes of the open show meanwhile is not said. Item 230 (accepted) puts a saved action into the show the recording was made in, so "never changes" needed that one exception.
ALT: b) Opening a recording first asks to save your open show, as opening another show does.
IF-WRONG: SMALL one question window more or fewer.
ASK: NO it repeats a reading he left standing, with the exception he accepted in item 230; how it is done is technical, and what opening Studio stops is the recordings topic's line.
@@END

@@ASSUME X-17
ABOUT: N18
TEXT: I assume pause does nothing while the tempo is stopped, stop works from pause as from running, and play does nothing while the tempo already runs. A stop pressed again while stopped still stops the actions you switched on since, and an audio file.
WHY: L30 drops the pause-first show start that needed this press; no word of his defines the press itself. BF250 ("tempo stop stops all actions, not just global") and BF271 ("Stop removes all clips from all layers so it would stop.") give a stop work to do also when the tempo is already stopped; they name no exception.
ALT: b) Pause on a stopped tempo arms it: clips you trigger then wait, and play is the 1. c) A stop pressed while the tempo is already stopped does nothing at all.
IF-WRONG: SMALL one button press with or without an effect.
ASK: NO presses at the edge; the stop follows his own words for every stop, and the other way of pause is the paused start that his L30 did not take; the tempo topic holds the twin.
@@END

