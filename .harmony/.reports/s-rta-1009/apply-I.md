# APPLY I -- Effects, signals and what moves a slider by itself (s-rta-1009, page 2)

## SUMMARY
- 240 (BF260) REVERSES his round-1 answer to 204: the keying list and the keying slider are NOT removed. All blend modes and the keying stay, as placeholders, and are built "when there is time". The removal of the keying and the mending of the blend modes leave the coming build.
- 240 also brings a NEW function: masks and moving masks, "which are alpha channels". The page's sentence "There are no mask layers" is void. What a mask cuts is not said by him: one question (I3-2).
- 241 (BF261): two types of envelope. One on the beat ("like a signal, but personalized for that clip"), one on the playhead (drawn over the course of the clip's playing). This settles the old doubt in favour of his earlier "build." for a drawn shape along the clip.
- The box under the answer on effects that read the beat by themselves (BF244): each such effect gets a Sync control ("adjust the sink" = the sync, INFERRED). What exactly the sync sets is one question (I3-6). The common macro is his "maybe": not built until he asks (I3-7).
- 242, 243, 268 were left empty: accepted as written. No box of another topic goes against them.
- Old assumption I-3 is dropped (contradicted); I-4, I-5, I-8, I-11, I-17 are re-worded; 21 amendments to old rules.

## ITEMS
@@ITEM 240
TITLE: Blend modes and keying stay; masks and moving masks are added
STATUS: ANSWERED in his own words (no letter); they go against his round-1 answer to 204
HIS: BF260 "All the blend modes and keying stay"; BF260 "We will also add masks and moving masks"
RULE: His words: "All the blend modes and keying stay, and they will be built when there is time. Right now they're just sitting there as placeholder. We will also add masks and moving masks with which are alpha channels". (1) WHAT STAYS. Every entry of the layer's blend list stays. The layer's keying stays: its list with every entry, and the keying slider. Nothing of them is removed from the layer strip, from the Layer tab or from the show file; a show keeps a layer's blend mode, its keying choice and its keying slider value. (2) PLACEHOLDERS. An entry that does nothing yet stays where it is, can be picked and is saved; picking it must never break the picture (it shows the layer as plain Normal blending would, with no keying, until it is built). (3) WHEN THEY ARE BUILT. "they will be built when there is time": every blend mode and every keying entry, and the keying slider, are made to work later. He gives no date. Read with his round-1 answer on these lists ("203 c this is built after all other parts are done. This is last, before the ui redesign which is the final change"): they are the last function work, after every other part is built and working and before the UI redesign; so the coming build removes nothing and mends nothing in the blend list, the keying list or the keying slider (I3-1). Before that work starts, what each entry will do is shown to him (I-5); no entry is dropped in it. (4) WHEN BUILT: a layer has one blend mode, one transparency slider, one keying choice and one keying slider. The transparency slider sets how strongly the layer is laid over the layers below with its blend mode, for every blend mode: fully down, the layer adds nothing; fully up, full strength (this half of his round-1 words, "the transparency slider to control that layers blend mode", is not touched by BF260 and stands, with @@ITEM 204 part (2) as amended). The keying decides which parts of the layer's picture are see-through before it is blended; the keying slider sets how much is taken out by the chosen keying entry; a picture's own see-through parts stay see-through (I-4). (5) EVERY LAYER THE SAME. The first sentence of the item as he read it is not touched by his words and is kept as Harmony's reading: every layer, the bottom one too, is blended in by its blend mode and its transparency slider (under the bottom layer there is only black). No control for a layer's "kind" is added, and nothing that belongs to a kind is taken out (effects-only layers, the autopilot's count per kind of layer) (I3-3). (6) WHAT IS NEW: MASKS AND MOVING MASKS. Firm from his words: the app gets masks; a mask can be still (a mask) or moving (a moving mask); a mask is an alpha channel, that is, it decides where a picture shows and where it is see-through. The item's sentence "There are no mask layers" is void. Not said by him: what a mask cuts, where a mask comes from, and when it is built. Best reading, Harmony's and not his word: a mask is a layer that is switched to work as a mask; the clip playing on it (a still picture for a mask; a video, a generated picture or the camera for a moving mask) is not shown itself but is used as the alpha channel of everything below it: where the mask is solid the layers below show, where it is see-through they are hidden (I3-2); the mask's own see-through parts are what counts, and for a picture that has none, its brightness (I3-8). A mask layer plays, fades, is triggered, takes effects, signals and actions like any layer, so a moving mask can be on the beat. Masks are built with the blend modes and the keying, not in the coming build (I3-1); until then the mask kind of layer that exists inside the app is left as it is and gets no control. (7) An effects-only cell and the effects Chroma Key and Key Palette: with @@ITEM 204 parts (4) and (5) as they stand.
CHANGED: Against the item as he read it: its last sentence ("There are no mask layers") is REPLACED by his words (masks and moving masks are added); its way b ("Mask layers stay; only the keying list and its slider go") is not taken either: the keying stays too. Its first sentence is not named by him and stands as a reading (I3-3). Against @@ITEM 204: part (1), the removal of the keying list and the keying slider, is REVERSED (his newest words win: see CONFLICTS); the sentence that this is built in the coming build is void; part (3) no longer waits on an answer. Against old assumption I-1 (the item's source): "Mask layers ... go with it" is contradicted. NEW from him: masks, moving masks, "which are alpha channels". INFERRED: "with which are alpha channels" is read as "which are alpha channels".
TODAY: From @@ITEM 204 and 203 TODAY (area-cue-layers.md items 14, 15, 18, 19, 20, 22, H13, M4): the strip's V drop-down holds 13 keying + 55 blend entries; as a blend only Normal, Additive, Screen, Multiply, Darken, Lighten exist and the last four ignore the layer's opacity; the K slider writes keyThreshold; keying runs only on Transparent layers; an Opaque layer (layer 1) ignores blend and keying. Layer::Type has five kinds, Mask among them ("Content becomes alpha mask for layers below", src/model/Layer.h:199, read not run; drawn at CompositorEngine.cpp:1209 by the fact sheet); no screen sets a layer's kind. What changes for the coming build: the planned removal of keyingMode / keyThreshold / keySoftness and the mending of four blend modes are taken OUT of it; nothing is removed. For the later stage: every blend and keying entry built, the mask kind given a control (if I3-2 holds), the bottom layer blended like the others.
@@END

@@ITEM 241
TITLE: Two types of envelope: on the beat and on the playhead
STATUS: ANSWERED in his own words (neither the item nor its way b as written)
HIS: BF261 "There should be two different types of envelopes that could work"
RULE: His words: "There should be two different types of envelopes that could work. One is along the beat, which is like a signal, but personalized for that clip, and another one is play head based so we can draw a envelope that happens over the course of the playing of that clip in timeline mode." An envelope is a shape he draws; it is of one of two types. (1) THE ENVELOPE ON THE BEAT ("along the beat, which is like a signal, but personalized for that clip"). It runs on the beat exactly as a signal does: it has a shape and a length in bars and beats; a slider or button that uses it has Looping / One Shot, Gain, Falloff, Range, Invert, the restart-on-a-drop switch and, for a button, a Threshold, with @@ITEM R221 (a) as it stands and as amended; a Looping user is locked to the count of bars; a One Shot runs as item 242 says; it holds on a tempo pause and stands on the "1" on a tempo stop, with @@ITEM R210 (c) as it stands. "personalized for that clip": an envelope made on a clip belongs to that clip. It is that clip's own: it is saved with the clip, goes wherever the clip goes (another cell, another deck, a saved deck), a copy of the clip gets its own copy of it, and it is offered to that clip's sliders and buttons only; several sliders and buttons of the clip can share it, and a clip can have any number (his round-1 words: "we can have infinite envelopes for different sliders. We can have one per slider or have many sliders share the same one."). That envelopes for a layer's and a Global slider, and envelopes shared across the show (Mod 2, a new one made in the Signal tab), go on as @@ITEM R195 (d) says is Harmony's reading: his new words speak of the clip and take nothing away (I3-4). (2) THE ENVELOPE ON THE PLAYHEAD ("play head based so we can draw a envelope that happens over the course of the playing of that clip in timeline mode"). Its drawn shape is stretched over the clip from its start to its end (the In and Out points: I-8); the value is read at the place of the clip's playhead. So it happens once over one pass of the clip, repeats with every pass when the clip loops, runs backwards when the clip plays backwards, jumps when the playhead jumps or is dragged, and holds when the clip is paused; it does not follow the beat clock, and a tempo pause holds it only if it holds the clip. It has no length of its own. Its users have Gain, Falloff, Range, Invert and, for a button, a Threshold; Looping / One Shot is not shown: the playhead decides (I-11). It belongs to its clip as in (1). His words name a clip "in timeline mode"; that it also works on a clip in BPM mode, and on a layer's control (stretched over whatever clip plays on that layer), is Harmony's reading (I3-5); it is offered wherever the entry Timeline is offered and greyed where a clip has no length of its own, with @@ITEM R220 as it stands. (3) The entry Timeline stays what @@ITEM 205 says: the straight link of a control to the playhead, with no drawn shape. An envelope on the playhead is the drawn form of the same link. (4) Both types are drawn in the Signal tab's envelope editor (his earlier words, with @@ITEM D30 and P31 as they stand); the type is chosen when the envelope is made and can be changed afterwards, the shape being kept (I-17). The moving line: on an envelope on the beat it shows the place in the bar count; on an envelope on the playhead it shows the playhead's place.
CHANGED: Against the item as he read it: "always runs on the beat, never along the clip's playhead" is REPLACED: both exist. Against way b: taken in substance (a shape stretched over the clip from start to end), and he adds "in timeline mode" and "personalized for that clip". Against @@ITEM 205 part (4) and old assumption I-6: the best reading "an envelope runs on the beat" is replaced; his earlier "build." (a drawn curve along the clip) is upheld by his newest words. NEW from him: an envelope is personal to a clip. Not said by him: whether the playhead type also serves BPM mode clips and layers (I3-5); whether show-wide envelopes go on beside the clip's own (I3-4).
TODAY: From @@ITEM 205, R195, R221 TODAY (area-effects-signals.md 1.12, 1.13, 1.14, 1.15, 1.16, E7, T8, O11): one envelope, Mod 2, show-wide and not saved with the show; the user cannot make another; the envelope editor has no mouse handling; no clip owns a signal; the clip clock is not wired to the signal engine (ConnectionEngine.cpp:327-372: "Clip Position" reads 0, "Timeline" is a 4-beat ramp on the beat clock); no editor draws a shape along a clip. All of it is to be built: clip-owned envelopes in the show file, two types, the clip clock wired.
@@END

@@ITEM 242
TITLE: What starts a One Shot on a slider
STATUS: ACCEPTED as written
HIS: box left empty = accepted as written
RULE: A slider or button set to One Shot follows its signal once, from the signal's beginning to its end, starting on the next "1" after the thing that starts it. (1) On a clip's slider or button: every time that clip is triggered, it runs once from the next "1" after the trigger; when the trigger itself is the new "1" (a BPM mode clip starting on the "1", or a click that starts a stopped beat), it starts with it. Triggering the clip again starts a new run. (2) On a layer's slider or button: after every clip triggered on that layer, once from the next "1". (3) On a Global slider or button: when One Shot is switched on, once from the next "1"; switching it off and on runs it again. It never starts off the beat with the clip's first frame, and it has no start button of its own (the two other ways he did not take). When the run is through, the control keeps the signal's last value until its next run. Everything else about One Shot (which signals show the choice, what a Resync does, pause and stop) is with @@ITEM R221 (a) part 6 as it stands and as amended, and @@ITEM R210 (c) as it stands. Tested against his other boxes: BF251 ("that action plays again only when that clip is re-triggered") says the same for a play-once action on a clip; nothing goes against it.
CHANGED: nothing: accepted as written. It closes old assumption I-9 (the page's best reading is now the rule).
TODAY: From @@ITEM R221 TODAY (area-effects-signals.md 1.14, X1, W4, E11): the Looping / One Shot toggles sit on the envelope and are inert (EnvelopeSignal.h:60-68); ConnShape holds playback and loop, saved, with no control on screen; no run is tied to a clip's trigger. To be built.
@@END

@@ITEM 243
TITLE: Effects that read the beat hold on pause and stop; Sync
STATUS: ACCEPTED as written; his words under the answer above it (BF244) add a Sync control
HIS: box left empty = accepted as written; BF244 "we should just be able to adjust the sink"; BF244 "maybe we could use a common macro that we could set later"
RULE: (1) ACCEPTED AS WRITTEN. An effect that reads the beat by itself (a strobe, a pulse) follows the beat clock. While the beat is PAUSED it holds where it is in its pulse and goes on from there, in step, when the beat plays again. While the beat is STOPPED it stands at its place for the "1" and starts with the new "1". With the Master Signal at 0 it keeps pulsing while the beat runs; an effect that listens to the music itself keeps moving; all with @@ITEM R221 (b) as amended. His box of another topic agrees: BF271 "A pause would not pause it unless it is connected to the BPM". (2) ADDED BY HIS WORDS under the answer (BF244): "For these effects, we should just be able to adjust the sink, and maybe we could use a common macro that we could set later for example shows where a single macro controls the speed of all of these type of effects". FIRM: every effect that reads the beat by itself has a control, on the effect, with which its sync is adjusted ("the sink" is read as the sync). Read from the same sentence, where what one macro would control is "the speed of all of these type of effects": Sync sets how fast the effect pulses against the beat, picked from a list of lengths in bars and beats (one pulse every 4 bars ... 1 bar, 2 beats, 1 beat, 1/2 beat, 1/4 beat); whatever is picked, the effect stays locked to the beat and to the "1", and (1) holds at every setting (I3-6). Sync is a control like any other of the effect: it is saved with the show, held by a preset, can be put on a key, pad or MIDI knob, is recorded, and can be moved by an action; it can take a signal or a macro knob as a slider does, stepping through its list. (3) MAYBE, NOT FIRM: "maybe we could use a common macro that we could set later". One macro that sets the speed of all effects of this type at once is his idea for later, in his words a "maybe" and "later". Nothing is built for it in the coming build, and nothing built shuts it out: because Sync can take a macro knob, each bank (a clip's, a layer's, the Global one) can already drive the Sync of its own effects with one knob; one knob for every such effect of the whole show at once is built only when he asks for it (I3-7). (4) Which effects count: every effect whose picture pulses with the beat without a signal plugged in; the list is made from the effects' program text when this part is planned (I3-9).
CHANGED: The item itself: nothing, accepted as written. ADDED by BF244: a Sync control on each such effect (firm); a common macro for the speed of all of them (maybe). INFERRED: "the sink" = the sync. INFERRED: that the sync he wants to adjust is the effect's speed against the beat (from "controls the speed" in the same sentence); the other readings are in I3-6.
TODAY: From @@ITEM R221 TODAY (area-effects-signals.md 1.18, E11, N7): effects that read the beat directly keep pulsing at Master Signal 0; no pause or stop of the beat exists yet, so nothing holds. Which effects read the beat by themselves and whether any has a rate control already: not checked (a grep finds beatPhase on 68 lines of src/render/EmbeddedShaders.h; counted, not read). To be built: the hold on pause and stop; a Sync row per such effect; a stepped control that can take a signal or macro.
@@END

@@ITEM 268
TITLE: Falloff: up at once, down slowly, instant to 2 seconds
STATUS: ACCEPTED as written
HIS: box left empty = accepted as written
RULE: Falloff is the one smoothing control on a slider or button that uses a signal. The value goes up with the signal at once, with no delay; when the signal drops, the value comes down slowly, in the time Falloff is set to, from instant (the value follows the signal down at once) to 2 seconds. The way up is never slowed, and there is no second control for the rise. Each user of a signal has its own Falloff, saved with the show; on a button the Threshold is tested after Gain and Falloff. With @@ITEM R221 (a) parts 3 and 5 as they stand and as amended.
CHANGED: nothing: accepted as written. It closes old assumption I-13 and adds the range (instant to 2 seconds) to the old rule.
TODAY: From @@ITEM R221 TODAY (area-effects-signals.md X1): ConnShape has smoothingMs, saved, with no control on screen; whether it smooths one way or both: not checked.
@@END

## AMENDMENTS
@@AMEND 203 1
OLD: Two things are read as NOT waiting, because L111 states them as how a layer works and not as a tidy-up of a list: the keying entries and the keying slider go, and every blend mode follows the transparency slider (see 204; the order of work is assumption I-3, a line he can strike).
NEW: Nothing of these lists is done before then: the keying entries and the keying slider stay, no blend mode is removed, and none is mended in the coming build (BF260: "All the blend modes and keying stay, and they will be built when there is time"; that "when there is time" is this last stage is assumption I3-1). Masks and moving masks are added in the same stage (BF260; I3-1, I3-2).
HIS: BF260 "All the blend modes and keying stay"
WHY: It said the keying goes and every blend mode is mended in the coming build; his newest words keep the keying and put the building off.
@@END

@@AMEND 203 2
OLD: What is kept, mended and removed in the two lists is put to him again when that last part is planned (I-5); the page's proposal for the keying list (3 stay, 2 mended, the keying slider connected to Luma Key) is void because the keying goes (L111).
NEW: No blend mode and no keying entry is removed: all stay and all are built (BF260). What each entry will do, and which transitions stay, is put to him when that last part is planned (I-5).
HIS: BF260 "they will be built when there is time"
WHY: It left open which entries are removed and voided the keying proposal because the keying was to go; now nothing goes.
@@END

@@AMEND 204 1
OLD: (1) The keying list (all its entries) and the keying slider are removed from the layer strip and from the Layer tab; a layer has no keying setting and a show carries none.
NEW: (1) REVERSED by his newest words (BF260: "All the blend modes and keying stay, and they will be built when there is time. Right now they're just sitting there as placeholder."): the keying list with all its entries and the keying slider STAY on the layer strip and in the Layer tab, as placeholders until they are built; a layer keeps its keying choice and its keying slider value and a show carries them; every blend mode stays too (item 240 of page 2).
HIS: BF260 "All the blend modes and keying stay"
WHY: It removed the keying list and the keying slider on his round-1 words; his words of 2026-10-09 say they stay.
@@END

@@AMEND 204 2
OLD: (L110; shown in I-3)
NEW: (L110)
HIS: BF260 "All the blend modes and keying stay"
WHY: It pointed at assumption I-3, which is dropped because the keying no longer goes.
@@END

@@AMEND 204 3
OLD: with no setting for it (I-4)
NEW: whatever is later built for the keying (I-4)
HIS: BF260 "All the blend modes and keying stay"
WHY: It said a layer has no setting that touches a picture's own see-through parts; with the keying staying, a layer has keying settings again.
@@END

@@AMEND 204 4
OLD: This is built together with the removal of the keying, not at the last stage (I-3).
NEW: This is built when the blend modes are built ("they will be built when there is time", BF260), which is read as the last stage and not the coming build (I3-1).
HIS: BF260 "they will be built when there is time"
WHY: It tied this work to the removal of the keying in the coming build; there is no removal, and he puts the blend modes off.
@@END

@@AMEND 204 5
OLD: It is asked as I-1; until he answers, nothing is built for a kind and nothing is taken out for it (mask layers, effects-only layers, the autopilot's count per kind of layer).
NEW: It was put to him as item 240 of page 2. His answer (BF260) names no kind, keeps the blend modes and the keying, and adds masks and moving masks, "which are alpha channels": so mask layers do not go. No control for a kind is added and nothing is taken out for a kind (effects-only layers, the autopilot's count per kind of layer) (I3-3); what a mask is and what it cuts is assumption I3-2.
HIS: BF260 "We will also add masks and moving masks"
WHY: It waited on his answer about the layer's kind and held mask layers as something that might go; he has answered, and masks are added.
@@END

@@AMEND D31 1
OLD: No keying entries are in it (L111).
NEW: The keying stays beside it, with its own entries and its slider (BF260).
HIS: BF260 "All the blend modes and keying stay"
WHY: It took the keying entries out of the list because the keying was to go; the keying stays.
@@END

@@AMEND D31 2
OLD: apart from the keying entries (I-3)
NEW: the keying entries too (BF260)
HIS: BF260 "All the blend modes and keying stay"
WHY: It excepted the keying entries, which were to be removed in the coming build; nothing is removed.
@@END

@@AMEND P30 1
OLD: There is no Keying list to place (L111)
NEW: The Keying list stays and is placed by Harmony like the others (BF260)
HIS: BF260 "All the blend modes and keying stay"
WHY: It said there is no keying list to lay out; the keying stays.
@@END

@@AMEND 205 1
OLD: Best reading: Timeline itself is a straight link with no drawn shape, and an envelope runs on the beat. I-6 asks whether an envelope can also be connected to the playhead
NEW: Timeline itself is the straight link with no drawn shape. An envelope is of one of two types (item 241 of page 2, BF261: "There should be two different types of envelopes that could work"): on the beat, like a signal and personal to its clip, or on the playhead, where its drawn shape is stretched over the clip from start to end. So his answer is that an envelope can also be connected to the playhead
HIS: BF261 "another one is play head based so we can draw a envelope"
WHY: It held that an envelope runs on the beat only and asked the rest; he now rules two types.
@@END

@@AMEND P31 1
OLD: Whether an envelope can also be connected to the playhead is a question about what the app does and is asked under 205 (I-6); whatever he answers, the shape is drawn in that same editor.
NEW: An envelope can also be connected to the playhead (item 241 of page 2, BF261); both types of envelope are drawn in that same editor.
HIS: BF261 "There should be two different types of envelopes that could work"
WHY: It left the question open; he has answered it.
@@END

@@AMEND R195 1
OLD: Each envelope has its own shape and length and is saved with the show.
NEW: Each envelope has its own shape and is saved with the show. An envelope is of one of two types (item 241 of page 2, BF261): on the beat, with a length of its own in bars and beats, or on the playhead, with no length (it is stretched over the clip). An envelope made on a clip belongs to that clip, is saved and copied with it and is offered to that clip's sliders and buttons only ("personalized for that clip", BF261; I3-4).
HIS: BF261 "like a signal, but personalized for that clip"
WHY: It knew one type of envelope, always with a length, and no envelope that belongs to a clip.
@@END

@@AMEND R220 1
OLD: APPEND
NEW: An envelope on the playhead (item 241 of page 2, BF261) is offered in the same places and greyed in the same places as the entry Timeline: on a clip's controls it follows that clip's playhead; on a layer's controls it is stretched over whatever clip is playing on that layer (I3-5); not in the Global tab.
HIS: BF261 "another one is play head based so we can draw a envelope"
WHY: It ruled where the straight link to the playhead is offered and did not know a drawn envelope on the playhead.
@@END

@@AMEND R221 1
OLD: comes down slowly (I-13)
NEW: comes down slowly, in a time set from instant to 2 seconds; the way up is never slowed (item 268 of page 2, accepted as written)
HIS: item 268 accepted as written
WHY: It gave no range and pointed at an assumption that is now closed.
@@END

@@AMEND R221 2
OLD: is shown on a user of an envelope or of an oscillator (I-11)
NEW: is shown on a user of an envelope on the beat or of an oscillator, not on a user of an envelope on the playhead, where the playhead decides (item 241 of page 2, BF261; I-11)
HIS: BF261 "There should be two different types of envelopes that could work"
WHY: It knew one type of envelope; an envelope on the playhead has nothing to loop or play once.
@@END

@@AMEND R221 3
OLD: What starts a run is not said by him and is asked (I-9): best reading, the fire of its clip
NEW: What starts a run is settled by item 242 of page 2, which he accepted as written: the trigger of its clip
HIS: item 242 accepted as written
WHY: It held the start of a One Shot as an open question; he accepted the reading.
@@END

@@AMEND R221 4
OLD: his L107 asks whether the page's rule is a good idea; the answer is under QUESTIONS BACK. The rule, unless he says otherwise after reading it (I-16):
NEW: he asked whether the page's rule is a good idea (L107), read the answer and accepted the rule as item 243 of page 2. His words under the answer add a control (BF244: "we should just be able to adjust the sink", read as the sync): every such effect has a Sync control of its own, read as how fast it pulses against the beat, picked from a list of lengths in bars and beats (I3-6); one common macro for the speed of all of them is his "maybe" and is not built until he asks for it (I3-7). The rule, which holds at every Sync setting:
HIS: BF244 "we should just be able to adjust the sink"; item 243 accepted as written
WHY: It waited on his reading of the answer; he accepted the rule and added the sync.
@@END

@@AMEND R221 5
OLD: the line holds still while the beat is paused or stopped
NEW: the line holds still while the beat is paused or stopped; on an envelope on the playhead (item 241 of page 2, BF261) the line is at the playhead's place and moves and stops with the clip
HIS: BF261 "over the course of the playing of that clip"
WHY: It tied every moving line to the beat; an envelope on the playhead follows the clip, not the beat.
@@END

@@AMEND R196 1
OLD: (if kinds stay: I-1)
NEW: (it stays: nothing of a layer's kind is taken out, item 240 of page 2, BF260; I3-3)
HIS: BF260 "All the blend modes and keying stay"
WHY: It hung the count per kind of layer on an open question about kinds; his answer takes nothing out.
@@END

@@AMEND R214 1
OLD: Whether effects that read the beat by themselves (a strobe, a pulse) hold still too is not settled here: he asks whether that is a good idea (L107), and topic I answers him.
NEW: Effects that read the beat by themselves (a strobe, a pulse) hold still too: where they are on a pause, on the "1" on a stop (item 243 of page 2, accepted as written; topic I's R221 b carries the rule and their Sync control, BF244).
HIS: item 243 accepted as written; BF244 "we should just be able to adjust the sink"
WHY: It left this clause open until his question was answered; he read the answer and accepted the rule.
@@END

## ASSUMPTIONS
@@ASSUME I3-1
ABOUT: 240; 203, 204, D31
TEXT: I assume "when there is time" means after the coming build: blend modes, keying and masks are left alone in it, nothing is taken out, and they are built as the last work before the UI redesign.
WHY: BF260 gives no time ("when there is time"; "We will also add"). His round-1 words on these lists said last, before the UI redesign.
ALT: b) Masks and moving masks come in the coming build; only blend modes and keying wait. c) Making every blend mode follow the transparency slider comes in the coming build; the rest waits.
IF-WRONG: SMALL the same work, earlier or later; he would miss masks in the first build if he expected them.
ASK: LINE the order of work; he can strike it in one line.
@@END

@@ASSUME I3-2
ABOUT: 240; 204
TEXT: I assume a mask is a layer you switch to work as a mask: its clip (a still picture, or a video for a moving mask) is not shown itself; it decides where all the layers below it show and where they are hidden.
WHY: BF260 says only "masks and moving masks with which are alpha channels": not what a mask cuts, nor where it comes from.
ALT: b) A mask layer cuts only the one layer directly below it. c) A mask is a picture or video you put on one clip or one layer, and it cuts only that clip or layer.
IF-WRONG: REBUILD what a mask cuts decides how the picture is put together and what the show file holds.
ASK: YES a new function of his with no word on how it works; a wrong guess is rebuilt.
@@END

@@ASSUME I3-3
ABOUT: 240; 204, R196
TEXT: I assume every layer, the bottom one too, is blended in by its blend mode and its transparency slider, and that no control for a layer's "kind" is added. Effects-only cells and the autopilot's count per kind stay as planned.
WHY: He read this sentence of item 240 and his words (BF260) go only against the removal of keying and of mask layers.
ALT: b) The bottom layer always shows plainly, whatever its blend mode says.
IF-WRONG: SMALL one layer's behaviour under a blend mode other than Normal.
ASK: NO he read the sentence and left it; nothing is taken out for it.
@@END

@@ASSUME I3-4
ABOUT: 241; R195
TEXT: I assume an envelope you make on a clip belongs to that clip: saved with it, copied with it, and only that clip's sliders and buttons can use it. Envelopes for a layer's or a Global slider, and shared ones, stay as you described.
WHY: BF261 "personalized for that clip" speaks of the clip only; his round-1 words allow "many sliders share the same one" without saying whose.
ALT: b) Every envelope is in one list for the whole show, and any clip can use any of them. c) There are no shared envelopes at all: each belongs to one clip, one layer or Global.
IF-WRONG: SMALL built as the wider set; a kind of envelope he does not want is hidden.
ASK: LINE a reading of one word of his ("personalized"); he can strike it.
@@END

@@ASSUME I3-5
ABOUT: 241; 205, R220
TEXT: I assume an envelope on the playhead works on any clip that has a playhead, in BPM mode too, and on a layer's slider it is stretched over whatever clip plays on that layer.
WHY: BF261 names "that clip in timeline mode" only; BPM mode clips and a layer's controls are not named.
ALT: b) It is offered only on a clip in Timeline mode; a clip in BPM mode uses the envelope on the beat. c) It is a clip's only: a layer's slider cannot use one.
IF-WRONG: SMALL an entry offered in more places than he wants; it can be greyed.
ASK: LINE he can strike it.
@@END

@@ASSUME I3-6
ABOUT: 243; R221
TEXT: I assume "adjust the sync" means: each effect that reads the beat by itself gets a Sync control that sets how fast it pulses (every bar, every beat, every half beat ...). It always stays locked to the beat.
WHY: BF244 says "adjust the sink" (read: the sync) and does not say what is adjusted; "controls the speed" in the same sentence points to the rate.
ALT: b) Sync is a switch: on, the effect follows the beat; off, it runs free at a speed you set. c) Sync shifts the effect earlier or later against the beat, so its flash lands where you want.
IF-WRONG: STAGE a strobe that cannot be set the way he means it is seen at once; one control on every such effect to change.
ASK: YES a dictated word that can be read three ways; it decides a control he will use live.
@@END

@@ASSUME I3-7
ABOUT: 243; R210
TEXT: I assume the common macro waits, as your "maybe": each of these effects gets its own Sync control, which a macro knob can turn like any slider. One knob for all of them at once is built when you ask for it.
WHY: BF244 says "maybe we could use a common macro that we could set later": a maybe, and later. Macros are in three banks (clip, layer, Global), so one knob for all crosses the banks.
ALT: b) Build it now: one Global knob that sets the speed of every such effect in the show at once.
IF-WRONG: SMALL it can be added later without changing what was built.
ASK: LINE his own "maybe" and "later"; he can strike it.
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
TEXT: I assume "effects that read the beat by themselves" are all effects whose picture pulses with the beat without any signal plugged in; I list them for you when this part is planned.
WHY: BF244 says "these effects" and "these type of effects" and names none; the page named a strobe and a pulse as examples.
ALT: none
IF-WRONG: SMALL an effect gets or lacks one control.
ASK: NO a list made from the effects themselves.
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
TEXT: I assume that when the turn of the blend modes, the keying and the transitions comes, I show you what each entry will do before it is built. No blend mode and no keying entry is removed.
WHY: BF260 says all stay and "will be built" but not what each of the entries should do; L110 leaves the lists until last.
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
TEXT: I assume a new envelope is made from a slider's list of signals or in the Signal tab, where you pick its type (on the beat or on the playhead) and can change it later. Deleting one sets its sliders back to Manual.
WHY: L104 and BF261 say how many envelopes and which two types, not how one is made, named, switched or deleted. Where the entry sits is his L8.
ALT: b) The type is fixed once the envelope is made.
IF-WRONG: SMALL a menu entry and names.
ASK: NO layout and names (L8).
@@END

@@DROP ASSUME I-3
WHY: Contradicted by BF260 ("All the blend modes and keying stay, and they will be built when there is time"): the keying list and the keying slider do not go in the coming build or at all; the order of work is now assumption I3-1. Amended in 203, 204 and D31.
@@END

## QUESTIONS BACK
(none owed by this topic: no box of his in this topic asks back)

## NAMES
@@NAME Mask
MEANS: A picture used as an alpha channel: it decides where other pictures show and where they are hidden (what it cuts: assumption I3-2).
SOURCE: his words (BF260: "masks and moving masks with which are alpha channels")
@@END

@@NAME Moving mask
MEANS: A mask whose picture moves (a video or another moving picture).
SOURCE: his words (BF260)
@@END

@@NAME Keying
MEANS: The layer's list of ways to make parts of its picture see-through, with its keying slider; it stays, as a placeholder until built.
SOURCE: his words (BF260: "All the blend modes and keying stay"); replaces the row "K slider, Keying (modes and slider) -- Taken out" in /Users/boriskarpman/projects/RealTimeAudio/.harmony/NAMES.md, which no longer holds
@@END

@@NAME Envelope on the beat
MEANS: The type of envelope that runs along the beat like a signal and is personal to its clip.
SOURCE: his words for the thing (BF261: "along the beat, which is like a signal, but personalized for that clip"); the name is Harmony's pick
@@END

@@NAME Envelope on the playhead
MEANS: The type of envelope whose drawn shape happens over the course of the playing of its clip.
SOURCE: his words for the thing (BF261: "play head based"); the name is Harmony's pick
@@END

@@NAME Sync
MEANS: The control on an effect that reads the beat by itself with which its sync is adjusted (read as: how fast it pulses against the beat, assumption I3-6).
SOURCE: his words (BF244: "adjust the sink", read as the sync); it meets the signal-list entry "BPM Sync", already noted as an open name in /Users/boriskarpman/projects/RealTimeAudio/.harmony/NAMES.md
@@END

@@NAME Common macro
MEANS: One macro that sets the speed of all effects that read the beat by themselves at once; his "maybe", not built until he asks.
SOURCE: his words (BF244: "a common macro that we could set later")
@@END

## REACHES OTHER TOPICS
- Topic A, @@ITEM R214 (named under MADE FROM of item 243): its open clause on effects that read the beat is closed by @@AMEND R214 1 above. BF271 ("A pause would not pause it unless it is connected to the BPM"), a box of topics A / K, was used here only as support for item 243.
- Topic K, old item R203 (c) (the autopilot's count per kind of layer, which hung on I-1): BF260 takes nothing out, so the count per kind stays; K's paper should say so (I3-3). If I3-2 holds, "mask" becomes a kind of layer he can pick, and K's count per kind then has a visible meaning.
- Topic F (the show file): a show keeps each layer's keying choice and keying slider value (BF260 reverses the removal); it will hold clip-owned envelopes of two types (BF261) and, later, mask layers. A copy of a clip carries its own envelopes.
- Topics B (old assumption B-3), C (C-13) and H (R194 f) lean on "every blend mode follows the transparency slider" from L111: the sentence stands, but it is built at the last stage, not in the coming build (I3-1). Until then they must not count on it.
- Topic C (presets): Sync is a control of the effect and is held by a preset. Whether a preset carries a clip-owned envelope's shape is still not settled (it was open before; BF261 makes it sharper).
- Topics D and E (actions, recordings, Studio): Sync is recorded and can be moved by an action like any control; an envelope's type and owner are part of what a recording must find again.
- Topic B (previews): BF247 says a preview starts at once, not on the beat. What a One Shot (item 242: "from the next 1") or an envelope on the beat does inside a preview is B's to rule; this paper rules the output only.
- Topic J (the mapping): Sync can be put on a key, pad or MIDI knob; on a knob it steps through its list.
- NAMES.md (kept by Harmony): the row that says the keying is taken out must be replaced; new rows Mask, Moving mask, Sync, the two envelope types.

## CONFLICTS
- The keying. NEW (BF260, 2026-10-09): "All the blend modes and keying stay, and they will be built when there is time. Right now they're just sitting there as placeholder." AGAINST his round-1 answer (binding-decisions.md:1185): "204 I want to remove the keying and slider. We are only going to use the transparency slider to control that layers blend mode." and his earlier "Does the keying slider actually do anything? Maybe we get rid of it." (binding-decisions.md:654-655). The newest wins: the keying and its slider stay. To say to him in one line: "On 7 October you wrote to remove the keying and its slider; on 9 October you wrote that the keying stays and is built later. I follow the later one."
- Mask layers. NEW (BF260): "We will also add masks and moving masks with which are alpha channels" AGAINST the page's own text, which is Harmony's and not his ("There are no mask layers", item 240) and the best reading in the old 204 (Harmony's: "a layer has no kind"). His words win; no earlier word of his is overturned.
- When. NEW (BF260): "they will be built when there is time" BESIDE (binding-decisions.md:1184): "203 c this is built after all other parts are done. This is last, before the ui redesign which is the final change". Not against each other; read as the same time (I3-1, a line he can strike).
- A drawn shape along the clip. NEW (BF261): "another one is play head based so we can draw a envelope that happens over the course of the playing of that clip in timeline mode" AGAINST the page's text (Harmony's: "always runs on the beat, never along the clip's playhead"). It agrees with his earlier "build." (binding-decisions.md:258, said to the drawable Timeline curve): the old open conflict there is closed in favour of both.
- One macro for all. NEW (BF244, a "maybe"): "a single macro controls the speed of all of these type of effects" BESIDE his earlier "all. Global, layer, clip." (binding-decisions.md:263, macro banks at all three levels, each slider on its own level's bank). One knob for effects on every level would cross the banks: nothing is built for it until he asks (I3-7).

## NOT DONE / UNSURE
- What a mask is (I3-2) and what "adjust the sync" means (I3-6): his words do not settle either. Cheapest: the two questions, one line each, on his next page.
- Which effects read the beat by themselves, and whether any already has a control for its rate: not looked up (only a count of lines in src/render/EmbeddedShaders.h). Cheapest: read each such effect's program text when the part is planned (I3-9).
- What the keying slider and each keying and blend entry will do when built: not ruled; put to him when that stage is planned (I-5).
- Whether the bottom layer under a blend mode other than Normal should show plainly (I3-3 way b): left as a note for that later stage.
- Whether a preset carries a clip-owned envelope: topic C; one line on a later page.
- How Resolume itself treats a layer in mask mode was not looked up (no web tools here, and his Resolume is never touched); if Harmony wants I3-2 to match it, that is a Researcher's task.
- TODAY lines are taken from the old blocks and the fact sheets of 2026-10-05, plus src/model/Layer.h:199 read by me; nothing was run.
- Written 2026-10-09 18:54:15 EDT by the architect of topic I (read, not run; nothing built).
