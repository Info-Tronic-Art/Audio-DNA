# APPLY B -- The cue system (s-rta-1009, page 2)

## SUMMARY
- 220 (BF247) turns the old rule round: a clip previewed by its name starts AT THE CLICK, never on the 1, so that he can run through any number of previews as fast as he likes. His words of 2026-10-07 ("it is triggered on the 1") are replaced by his newest words; the old items R151, R179 (c), 176, R170 and U18 are amended.
- His second sentence in that box keeps the beat where it belongs: a clip that was triggered on a layer and is looked at through that layer's cue button plays in time, because the trigger put it in time. Read as: nothing new is built for it (assumption B3-4, one line for him).
- 221 (box empty) is accepted and is no longer an exception: with 220 the same rule holds whether the tempo runs, is paused or is stopped.
- 222 (BF248): he asks what and where the master cue is. The name and the button are his (2026-10-07); what the button does was Harmony's guess. Answered, and put to him again (B3-1).
- 252 (BF267): he asks what "come first" means and whether all three pictures should run at the same fps. Draft answer: yes, the same fps; "less smooth" is taken back; put to him again (B3-2). The final text is settled with the technical paper answer-252.md.
- 251 (box empty): the cue transparency slider is a try-out for the preview only; accepted as written.
- Nothing here is measured; nothing is built.

## ITEMS
@@ITEM 220
TITLE: A clip previewed by its name starts at the click
STATUS: ANSWERED in his own words: he changes the item (neither the text nor its way b)
HIS: BF247 "Previewing a clip should not happen on the beat." and "we are cueing this way, then it should play in time"
RULE: A click on a clip's name in the deck shows that clip in the preview monitor at once and starts it at once, from its beginning, at the click. It never waits for the 1 or for any beat, whether the clip is in BPM mode or in Timeline mode and whether it has actions or not. A click on another name replaces the previewed clip at once, in the same way, as often and as fast as he clicks (BF247: "the user could go through any amount of previews as quick as they want"); no preview ever waits for the one before it to finish or to reach a beat. A second click on the name of the clip that is already previewed starts it again at once (assumption B-14). The previewed clip loops. A BPM mode clip plays in the preview at the speed that the tempo's BPM gives it, counted from the click and not lined up with the 1; a Timeline mode clip plays at its own speed. The clip's own actions that are switched on start with the clip, at the click, at the tempo's BPM, and are likewise not lined up with the 1 (assumption B3-3). Previewing never triggers the clip, never changes the output, and never starts, resumes or moves the tempo or the 1. Being in time with the beat belongs to triggering only (BF247: "only when they trigger and play should they be on time with the beat"): a clip that is triggered on a layer starts by the trigger rules as they stand in topic A, and a layer whose cue button is on shows in the preview what that layer holds, at the same moment as the layer itself, so a clip that was triggered on a layer and is looked at through the cue button plays in time with the music (BF247: "If the clip is loaded into the layer, and we are cueing this way, then it should play in time"; read as the triggered clip of a cued layer: assumption B3-4). Cueing a clip ahead of time therefore stays what @@ITEM R149 (f) says: the layer's own transparency slider down, trigger the clip, cue button on. A clip that is already playing in the output is not started again by a click on its name: with @@ITEM R151 as it stands in that part (assumption B-8). A file double-clicked in the Files tab plays at once, with @@ITEM R170 as it stands. What a previewed clip is shown with (its own effects, its own Opacity, no layer effects, no master): with @@ITEM R151 as it stands.
CHANGED: The item as he read it ("waits for the next 1 before it starts, in BPM mode or not") and its way b ("Only a BPM-mode clip waits") are both rejected: nothing waits. Against the old blocks: R151's "WHILE THE TEMPO RUNS it is triggered on the 1" and the assumed first-frame wait (B-1), R179 (c) "it is triggered on the 1", and U18's first half are replaced (amendments below). His words of 2026-10-07 (L36) go the other way: see CONFLICTS. INFERRED: "loaded into the layer ... cueing this way" is read as a clip triggered on a layer and seen through that layer's cue button (B3-4). Not said by him and filled by Harmony: when the previewed clip's actions start (B3-3), what a second click on the same name does (B-14).
TODAY: No preview monitor exists; a name click selects the clip and, for a picture or a generated clip, loads it into the main renderer's fallback slot, which reaches the outputs when nothing plays (old 176 and R150 TODAY lines; src/MainComponent.cpp:693-716, read, not run). No clip can be drawn outside a layer; one player per clip (facts-app-cue-today.md section 5). Clip actions are not built. A preview that starts at the click needs a clock of its own for the previewed clip and its actions: not checked.
@@END

@@ITEM 221
TITLE: A preview by name while the tempo is paused or stopped
STATUS: ACCEPTED as written; tested against BF247, which says the same for the running tempo
HIS: box left empty = accepted as written
RULE: While the tempo is paused or stopped, a clip previewed by its name plays in the preview monitor exactly as @@ITEM 220 says for the running tempo: it starts at the click, from its beginning, loops, with its own actions that are switched on, a BPM mode clip and the actions at the tempo's BPM (the BPM the tempo bar shows). One rule therefore holds for every state of the tempo. Previewing never starts the tempo, never resumes it and never sets or moves the 1. A preview that is already playing when the tempo is paused, stopped or started again plays on untouched, and nothing in the preview is triggered again when the tempo runs again (assumption B3-5). The way b of the item (the clip waits on its first frame until the tempo runs and reaches the 1) is not built. What tempo stop does to the switched-on state of a clip's actions is topic D's rule (BF250); the preview shows the actions that are switched on, whatever that rule leaves switched on.
CHANGED: nothing in the item: accepted as written. Against the old blocks: R151's last clause of this part ("when the tempo runs again the previewed clip is triggered again on the next 1") falls, because 220 (BF247) takes every start on the 1 out of the preview; the rule is no longer an exception to the running-tempo rule but the same rule. U18's "Open besides" is closed.
TODAY: The app has no stopped or paused state of the beat (facts-app-cue-today.md line 79) and no preview monitor. Old R151 TODAY: a preview that plays while the show's beat stands needs a beat of its own; not checked.
@@END

@@ITEM 222
TITLE: What master cue does and where it is shown
STATUS: OPEN he asks back what and where the master cue is
HIS: BF248 "what and where is the master cue?" and "where would the master cue be displayed?"
RULE: Waits on his answer: the reading put to him again (assumption B3-1) is that master cue is one toggle button that belongs with the layers' cue buttons, the controls under the preview monitor. It has no picture and no monitor of its own: it changes what the preview monitor shows in cue mode. Off: the preview shows the cued layers mixed, each with its clip's effects and its layer's effects, without the global effects. On: the same mix is also run through the global effects as they are on the output, moved by whatever global actions are running, so the preview shows what the mix would look like on the output. It only decides what the preview shows: it never changes the output and never starts, stops or tries out a global effect or a global action. Everything else with @@ITEM 177 as it stands, all of it waiting on the same answer. Fixed by his own words and not waiting: the layers have a cue button each, which shows the layer in the preview monitor (BF248: "The layers have a cue button to display in the preview/cue monitor"), and the cue controls are under the preview monitor (2026-10-05: "we will have the controls under the preview monitor").
CHANGED: Nothing is decided: he chose no way and asks what the thing is. The item's text and its ways b and c stand as the three readings. His "the preview/cue monitor" is his name for the preview monitor (noted under NAMES).
TODAY: No cue button, no master cue and no preview monitor exist (old 177 and R141 TODAY lines; area-cue-layers.md 1c item 16). That is why he cannot find the cue button on screen.
@@END

@@ITEM 251
TITLE: The cue transparency slider is a try-out for the preview
STATUS: ACCEPTED as written; no box of his says otherwise
HIS: box left empty = accepted as written
RULE: The transparency slider beside a layer's cue button sets how strongly that layer shows in the preview monitor and nothing else. It starts at full. It never moves the layer's own transparency slider, and the layer's own slider never moves it; a layer whose own slider is all the way down and whose cue slider is up is fully seen in the preview and not at all in the output. The way b of the item (it starts at the layer's own transparency) is not built. The rest with @@ITEM R141 and @@ITEM R149 (a), (d) and (f) as they stand; that it is not saved with the show, never recorded and can be put on a key, a pad or a knob: with @@ITEM R152 as it stands.
CHANGED: nothing: accepted as written. The old assumption B-3 is now his accepted rule; the old RULE lines of R141 and R149 already say the same and are not amended.
TODAY: No cue control exists on the layer strip or the monitor (old R141 TODAY; area-cue-layers.md 1c item 16).
@@END

@@ITEM 252
TITLE: The preview against the output: frames per second, size, limits
STATUS: OPEN he asks back what "come first" means and whether all run at the same fps
HIS: BF267 "What do you mean by come first?" and "should all run at the same fps, no?"
RULE: Waits on his answer: the reading put to him again (assumption B3-2) is that the output screens, the output monitor and the preview monitor are all drawn in the same frame, every frame, and so always run at the same frames per second; the preview is never drawn at a lower rate on purpose. The preview picture is drawn at the size at which it is shown, which is smaller than the output; the cued layers are taken from the layer pictures that the output already draws, made smaller, so a cued layer looks in the preview as it looks in the output. The output is the more important picture (BF267: "Of course the output is more important"): the cost of the preview is measured on a heavy show before the cue system is built, and if the preview would cost the output frames he is told the number and asked before anything is built. Two limits of the first build, put to him as one line of their own (assumption B-11): a trail (Echo, Freeze, Feedback) may look a little different in the preview than in the output, and a previewed simulation or MilkDrop clip of the same kind as one that is playing shows the playing one's motion, not a fresh run; MilkDrop itself stays as it is (BF265). The final wording of this rule is settled with the technical paper answer-252.md of this run.
CHANGED: "The output always comes first" and "less smooth" are withdrawn as written: his question shows the first was not understood and he doubts the second. His own half-sentence "Of course the output is more important" is kept as the order of importance. INFERRED: "the preview-cure screen" is read as the preview-cue screen, his name for the preview monitor. The two limits (trails, MilkDrop) were in the item and are not named in his box: they stay assumed, as a line of their own.
TODAY: One picture is drawn per frame, offscreen, and shown in the monitor panel and on the outputs (facts-app-cue-today.md line 17; Pitfall 37). No second picture exists; the compositor cannot draw a subset of layers or a clip that is on no layer (facts-app-cue-today.md section 5). A per-layer picture after the layer's effects already exists for the Layer Router (CompositorEngine.cpp:1150-1154, read, not run). The stated budget is 16.67 ms per frame at 60 fps; the frame cost on his machine is not measured (facts-app-cue-today.md line 71).
@@END

## AMENDMENTS
@@AMEND R151 1
OLD: WHILE THE TEMPO RUNS it is triggered on the "1" (L36: "it is triggered on the 1. Resolume plays it right away but we have actions so that will need to be playing in time with the music"). As assumed (B-1, because L35 says "They will play right away"): the preview takes the clip over at the click, the clip stands on its first frame while it waits, and on the next "1" it starts from its beginning with its actions; a Resync while it waits is that "1" (INFERRED from L12, as topic A rules it for a fired clip). It loops; a BPM-mode clip follows the tempo there too.
NEW: IT STARTS AT THE CLICK, never on the "1" and never on any beat (BF247: "Previewing a clip should not happen on the beat. It should just be quick so the user could go through any amount of previews as quick as they want and only when they trigger and play should they be on time with the beat"): the preview takes the clip over at the click and the clip starts from its beginning at once, in BPM mode or in Timeline mode, with actions or without. A click on another name replaces it at once, as often and as fast as he clicks. It loops. A BPM mode clip plays at the speed the tempo's BPM gives it, counted from the click and not lined up with the "1". Its own actions that are switched on start with it at the click, at the tempo's BPM, not lined up with the "1" (assumption B3-3). A Resync, a tempo change of the "1" or a new "1" does nothing to a preview.
HIS: BF247 "Previewing a clip should not happen on the beat."
WHY: The old rule started a name-previewed clip on the next 1 after a wait on its first frame (his L36, with B-1 assumed); his newest words take the beat out of previewing.
@@END

@@AMEND R151 2
OLD: WHILE THE TEMPO IS PAUSED OR STOPPED no "1" is coming. As assumed (B-2): the clip still plays in the preview, with its actions, at the tempo number, starting at the click; a preview that is already playing when the tempo is paused or stopped plays on in the same way; previewing never starts or resumes the tempo; and when the tempo runs again the previewed clip is triggered again on the next "1", so that it is in time with the music (INFERRED from L36).
NEW: WHILE THE TEMPO IS PAUSED OR STOPPED the same rule holds (item 221, accepted as written): the clip plays in the preview, with its actions, at the tempo's BPM, starting at the click. Previewing never starts or resumes the tempo. A preview that is already playing when the tempo is paused, stopped or started again plays on untouched, and nothing in the preview is triggered again when the tempo runs again (assumption B3-5).
HIS: item 221 accepted as written; BF247 "only when they trigger and play should they be on time"
WHY: The old text made the paused or stopped tempo an exception and re-triggered the preview on the next 1 when the tempo ran again; with BF247 no preview is ever started on a 1.
@@END

@@AMEND R151 3
OLD: A second click on the name of the clip that is already previewed starts it again on the next "1" (assumption B-14).
NEW: A second click on the name of the clip that is already previewed starts it again at once (assumption B-14).
HIS: BF247 "It should just be quick"
WHY: The old text made the restart wait for the next 1; nothing in a preview waits for the beat any more.
@@END

@@AMEND R179 1
OLD: and it is triggered on the "1" so that the actions play in time with the music (L36: "a previewed clip (clicking its name) shows the clip with all its actions and it is triggered on the 1").
NEW: and it starts at the click, never on the "1" (BF247: "Previewing a clip should not happen on the beat"); the first half of his L36 stands ("a previewed clip (clicking its name) shows the clip with all its actions"), and the actions start with the clip at the click, at the tempo's BPM, not lined up with the "1" (assumption B3-3).
HIS: BF247 "Previewing a clip should not happen on the beat."
WHY: Part (c) started the previewed clip on the 1 so that its actions were in time with the music; his newest words say a preview is quick and only a triggered clip is in time.
@@END

@@AMEND R179 2
OLD: shows in the preview by the same timing as in the show and is a real change to the clip. When no "1" is coming, and against L35's "They will play right away": R151 and assumptions B-1 and B-2.
NEW: shows in the preview at once and is a real change to the clip (assumption B-18). The state of the tempo changes nothing in a preview by name: R151.
HIS: BF247 "It should just be quick"; item 221 accepted as written
WHY: The old text tied an action switched on during a preview to the show's beat timing and pointed at two assumptions that his answers have closed.
@@END

@@AMEND 176 1
OLD: When the previewed clip starts to move: R151, R179 and assumptions B-1 and B-2.
NEW: The previewed clip starts to move at the click, whatever the tempo does (BF247; item 221): R151.
HIS: BF247 "Previewing a clip should not happen on the beat."
WHY: The old sentence left the start of the previewed clip to two assumptions about waiting for the 1; both are closed by his words.
@@END

@@AMEND R170 1
OLD: When a deck clip previewed by its name starts to move: R151 and assumptions B-1 and B-2.
NEW: A deck clip previewed by its name starts at the click too, whatever the tempo does (BF247; item 221): R151. A file and a deck clip therefore start alike; they differ only in that the deck clip has its effects and its actions.
HIS: BF247 "Previewing a clip should not happen on the beat."
WHY: The old sentence left the start of a name-previewed deck clip to two assumptions about waiting for the 1; both are closed by his words.
@@END

@@AMEND R149 1
OLD: APPEND
NEW: A clip triggered on a layer in the way of (f) starts by the trigger rules as they stand in topic A, so through the cue button it is seen in time with the music (BF247: "If the clip is loaded into the layer, and we are cueing this way, then it should play in time"; read as the triggered clip of a cued layer: assumption B3-4). Nothing in cue mode has a start of its own.
HIS: BF247 "we are cueing this way, then it should play in time"
WHY: The old rule described cueing ahead of time without saying that the cued clip is in time; his words now say so, and they are what separates cueing from previewing by name.
@@END

@@AMEND 177 1
OLD: APPEND
NEW: The whole of this rule waits on his answer to item 222 (BF248: "what and where is the master cue?"): his are the name and the button (L38); what the button does when on and off is Harmony's reading, put to him again as assumption B3-1. Nothing of master cue is built before he answers.
HIS: BF248 "what and where is the master cue?"
WHY: The old rule read as settled apart from one assumption; his question shows that the button itself has not been understood by him as written, so the rule cannot be built from yet.
@@END

@@AMEND D8 1
OLD: The preview picture is smaller than the output and may run less smoothly; the output's smoothness always comes first.
NEW: Waits on his answer to item 252 (BF267). The reading put to him (assumption B3-2): the output screens, the output monitor and the preview monitor are drawn in the same frame and always run at the same frames per second; the preview is never drawn at a lower rate on purpose; the preview picture is drawn at the size at which it is shown. The output is the more important picture (BF267: "Of course the output is more important").
HIS: BF267 "should all run at the same fps, no?"
WHY: The old rule allowed a less smooth preview and used the words "comes first"; he asks what that means, doubts that smaller or less smooth is safe, and expects one frame rate for all three pictures.
@@END

@@AMEND U18 1
OLD: A clip previewed by its name, while the tempo runs, starts in the preview on the 1 with all its actions, in time with the music (L36); his L35 says previews "will play right away", and the two lines are put to him together (X-29). Open besides: what a previewed clip does while the tempo is stopped or paused, when no 1 is coming. Best reading (X-24): it plays in the preview at once, at the tempo number, with its actions, and previewing never starts the tempo.
NEW: A clip previewed by its name starts in the preview at the click, never on the 1, with all its actions that are switched on (BF247: "Previewing a clip should not happen on the beat"); only a triggered clip is in time with the beat. The same holds while the tempo is stopped or paused (item 221, accepted as written): it plays in the preview at once, at the tempo's BPM, with its actions, and previewing never starts the tempo. The full rule: R151 in the cue topic.
HIS: BF247 "Previewing a clip should not happen on the beat."; item 221 accepted as written
WHY: The old rule started the preview on the 1 and left the stopped or paused tempo open; his box 220 and the accepted item 221 settle both.
@@END

## ASSUMPTIONS
@@ASSUME B3-1
ABOUT: 222 177
TEXT: I assume master cue is one more cue button, for global. On: the preview monitor shows the cued layers through the global effects, moved by the global actions, as the output would. Off: without them. It never changes the output.
WHY: BF248 asks "what and where is the master cue?"; his own line of 2026-10-07 names the button and not what it does.
ALT: b) On, it also lets you switch a global effect or action on for the preview monitor only, to try it first. c) On, the preview monitor shows the whole output, as on a DJ mixer.
IF-WRONG: REBUILD way b needs a second, preview-only copy of the global effects and their actions; and he would press it expecting something else
ASK: YES he asked back; no word of his says what the button does; way b is a far bigger build
@@END

@@ASSUME B3-2
ABOUT: 252 D8
TEXT: I assume the output screens, the output monitor and the preview monitor always run at the same frames per second. The preview picture is drawn only as large as you see it. If that slows the output, I ask you first.
WHY: BF267 asks "should all run at the same fps, no?" and "What do you think?"; he leaves the build to Harmony but asks; nothing is measured.
ALT: b) The preview monitor's picture is also drawn at the output's full size, then shown small. c) If the computer cannot keep up, the preview monitor alone loses frames; the output never does.
IF-WRONG: STAGE a preview that costs the output frames is seen by the audience; a jerky preview is seen by him
ASK: YES he asked back, and it decides how the second picture is built
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
WHY: BF247 "If the clip is loaded into the layer, and we are cueing this way, then it should play in time" names no click or button.
ALT: b) With a layer's cue button on, a click on a clip's name in that layer puts the clip into the cue on the next 1, without it reaching the output.
IF-WRONG: REBUILD way b is a layer that plays one clip for the output and another for the cue
ASK: LINE his BF248 words ("The layers have a cue button to display") fit the reading; way b would be a new function he has not described
@@END

@@ASSUME B3-5
ABOUT: 220 221 R151 R152
TEXT: I assume the tempo bar's play, pause and stop never touch a clip you preview by its name: it plays on. The cue shows the layers as they are: standing still in a pause, empty after a stop.
WHY: Item 221 covers a preview started in a pause or a stop, not one already running; BF271 "Stop removes all clips from all layers" names layers only.
ALT: b) Tempo stop also empties the preview monitor.
IF-WRONG: SMALL seen in the preview only
ASK: NO follows from item 221 as accepted and from a cued layer being the same layer seen twice
@@END

@@ASSUME B-11
ABOUT: 252 D9 177
TEXT: I assume two limits in the first build. Trails (Echo, Freeze, Feedback) may look a little different in the preview. A previewed simulation or MilkDrop clip shows the motion of one of the same kind that is playing, not a fresh run.
WHY: BF267 asks about "come first", size and fps and does not name the trails or MilkDrop; BF265 "Keep Milk drop as it is".
ALT: b) Every previewed picture must be fully its own, which needs a second copy of each simulation and of MilkDrop.
IF-WRONG: REBUILD second copies of every picture that keeps a state of its own are a large addition
ASK: LINE it was half of an item whose box asks about the other half; he would most likely wave it through
@@END

@@ASSUME B-14
ABOUT: R170 176 R151 220
TEXT: I assume a preview lasts until another clip or file is previewed. In cue mode it plays on out of sight, and the toggle shows it again. A second click on the same name restarts it at once.
WHY: BF247 says a preview is quick and never on the beat; how long a preview lasts and what a second click does are still not said.
ALT: b) Going back to cue mode ends the preview; preview mode is then black until the next click. c) A second click on the same name changes nothing.
IF-WRONG: SMALL
ASK: NO internal; nothing of it reaches the output
@@END

@@ASSUME B-18
ABOUT: R179 220
TEXT: I assume "all its actions" means the clip's own actions that are switched on. Switching one on or off while the clip is previewed shows in the preview at once and is a real change to the clip.
WHY: BF247 takes the beat out of previewing; his earlier words say "with all its actions" and not whether switched-off actions play or when a switch shows.
ALT: b) Every action stored on the clip plays in the preview, switched on or not. c) A switch shows in the preview only from the next 1, as in the show.
IF-WRONG: SMALL
ASK: NO follows the rule that an action plays only while its button is on, and BF247 for the timing
@@END

## QUESTIONS BACK
@@ANSWER 222
HIS: BF248 "what and where is the master cue? The layers have a cue button to display in the preview/cue monitor but where is the cue button and where would the master cue be displayed?"
ANSWER: Master cue is one on/off button that sits with the layers' cue buttons under the preview monitor. It has no picture of its own: it changes what the preview monitor shows. The name and the button are yours, from 7 October: "177 add a global effects and actions toggle button called master cue". What it does is my guess: off, the preview monitor shows the cued layers mixed, without the global effects; on, the same mix goes through the global effects, as the global actions move them, as it would look on the output. No cue button is built yet, which is why you cannot find one in the app.
@@END

@@ANSWER 252
HIS: BF267 "What do you mean by come first? Playing something smaller or less smooth may create problems. What do you think? You understand how the system needs to be built better than me. Of course the output is more important, but the output preview screen which we have and the preview-cure screen below it should all run at the same fps, no?"
ANSWER: Yes, they should all run at the same fps, and I take "less smooth" back. "Come first" meant only this: if the computer ever cannot keep up, the picture the audience sees must not be the one that suffers. My plan: the output screens, the output monitor and the preview monitor are drawn together, frame by frame, so they cannot run apart. "Smaller" means only that the preview monitor's picture is drawn as large as you see it; the cued layers are the output's own layer pictures, made smaller. Nothing is measured yet: before building I test it on a heavy show and tell you what it costs.
@@END

## NAMES
@@NAME preview mode
MEANS: The state of the preview monitor in which it shows one clip or one file, alone, playing from the moment it was clicked and never waiting for the beat.
SOURCE: his words BF247 ("Previewing a clip should not happen on the beat"); replaces the meaning "alone, playing in time with the music" of the row preview mode in /Users/boriskarpman/projects/RealTimeAudio/.harmony/NAMES.md
@@END

@@NAME preview monitor
MEANS: The monitor below the output monitor that shows the cue or one previewed clip or file; he also calls it "the preview/cue monitor" and "the preview-cue screen", which are the same thing.
SOURCE: his words BF248 ("the preview/cue monitor") and BF267 ("the preview-cure screen", read as preview-cue: INFERRED); the name preview monitor in /Users/boriskarpman/projects/RealTimeAudio/.harmony/NAMES.md stays, his two variants are added to its row
@@END

@@NAME cueing
MEANS: Looking, in the preview monitor, at a layer whose cue button is on: the clip triggered on that layer is seen as the layer plays it, in time with the music.
SOURCE: his words BF247 ("we are cueing this way"); the meaning is Harmony's reading (assumption B3-4)
@@END

@@NAME master cue
MEANS: The toggle button, with the layers' cue buttons, that puts the global effects and what the global actions do to them on the preview monitor, or leaves them off; the meaning waits on his answer to item 222.
SOURCE: his words of 2026-10-07 ("called master cue") for the name; the meaning is Harmony's reading, asked again after BF248
@@END

## REACHES OTHER TOPICS
- X (U18, X-24, X-29): U18 is amended here because item 221 is made from it. The old assumptions X-24 and X-29 are settled by BF247 and by item 221 as accepted: the paper of topic X should drop both. X-11 is the same matter as item 252 and stays open with it.
- D (clip actions): BF247 means a clip's own actions can play for the preview alone from the moment of a click, at the tempo's BPM, not lined up with the 1 (assumption B3-3). The actions engine has to allow a start that is not on the show's beat for the preview. BF250 ("tempo stop stops all actions, not just global") is D's: whether tempo stop switches a clip's action buttons off decides what a preview by name shows after a stop; this paper only says the preview shows the actions that are switched on.
- A (tempo bar and triggering): BF247 "only when they trigger and play should they be on time with the beat" confirms A's trigger rules and changes none; previewing never starts the tempo (item 221) stands beside A's rule that a trigger while stopped starts the beat. BF271 "Stop removes all clips from all layers" (A's rule) is applied here only as: after a stop the cue shows empty layers (B3-5).
- K (MilkDrop): BF265 "Keep Milk drop as it is" is applied here to the limit of the first build (B-11): no second MilkDrop for the preview.
- I (blend modes, keying, masks): BF260 says the blend modes and keying stay and masks come later. The cue mixes the cued layers by their blend modes (R141, R149 d) and that stands; what a mask layer does inside the cue is not ruled here and waits for I's rule on masks.
- C (item 253, accepted): only files and deck clips are previewed; nothing here contradicts it.
- F (BF256: "Pasting over a clip or deleting a clip, removes it from the layer strip, and it does not play"): a previewed clip that is deleted leaves the preview black (old B-21, untouched); F owns the rule for the layer.
- J (names): the row "preview mode" of NAMES.md changes its meaning (see NAMES); the row "preview a clip" already says "plays at once" and stands. The word Studio replaces Review in every text for him; the old assumption B-17 keeps "recording review mode" only inside his quote.

## CONFLICTS
- A clip previewed by its name: NEW, BF247: "Previewing a clip should not happen on the beat. It should just be quick so the user could go through any amount of previews as quick as they want and only when they trigger and play should they be on time with the beat." EARLIER, binding-decisions.md line 1134 (R179; BF175): "R179 all good except a previewed clip (clicking its name) shows the clip with all its actions and it is triggered on the 1. Resolume plays it right away but we have actions so that will need to be playing in time with the music". The newest words win: a preview starts at the click; its actions come with it and are not lined up with the 1 (B3-3 is the one line that says so to him). His still earlier line agrees with the new one: binding-decisions.md line 1133 (R170; BF174): "They will play right away".
- Not a clash, said once more so that it is clear: BF248 asks "what and where is the master cue?" about a button he named himself, binding-decisions.md line 1136 (177; BF177): "177 add a global effects and actions toggle button called master cue". He is not withdrawing it; he asks what Harmony made of it.

## NOT DONE / UNSURE
- Item 252: this paper's answer is a draft from the fact sheets (read, not run). Whether a second picture every frame leaves the output's frame rate alone is NOT measured; the cheapest way to settle it is the spike the old spec already names (blend the saved per-layer pictures into a small second target and read the frame time with and without it), when the build hold lifts. The final text of @@ANSWER 252 and of B3-2 is settled with answer-252.md, which was not there when this was written.
- "Drawn at the size it is shown" is safe for cued layers (they are the output's own layer pictures made smaller: INFERRED from CompositorEngine.cpp:1150-1154, read, not run). For a clip previewed by its name, which is drawn only for the preview, effects that count in pixels (a blur, a pixelate) could look slightly different at a smaller size: ESTIMATE, not checked; way b of B3-2 (draw at full size) is the cure and costs more. The technical paper should rule this.
- A preview that starts at the click needs a clock of its own for the previewed clip and its actions (one player per clip; a clip seen twice is the same playback: facts-app-cue-today.md section 5). How a previewed video starts "at once" depends on how fast its first frame is ready: not measured; the rule only forbids waiting for the beat.
- B3-4: if he strikes the line and means way b (a clip put into the cue of a layer without reaching the output), the cue system grows by a second playback per layer: to be planned only on his word.
- What tempo stop does to a clip's action buttons (BF250, topic D) decides what a preview by name shows after a stop; settled by D's paper, then one sentence in R151.
- A mask layer inside the cue (BF260): waits for topic I.
- No old assumption of this topic is dropped: B-5, B-7, B-8, B-17, B-19, B-20 and B-21 are untouched by his 33 boxes; B-14 and B-18 change and keep their ids.
- Written 2026-10-09 18:47:48 EDT by the architect of topic B; read-only, nothing built, run or measured.
