# SPEC I -- Effects, signals and what moves a slider by itself (s-rta-1009): the s-rta-1007 spec with Boris's answers to page 2 laid over it by wf/merge3.py (paper apply-I.md, ruling rule-I.md). This file wins over every older one.

## PAGE 2 ITEMS (the rule now for each item of page 2 in this topic)
@@ITEM 240
TITLE: Blend modes and keying stay; masks and moving masks are added
STATUS: ANSWERED in his own words (no letter); they go against his round-1 answer to 204
HIS: BF260 "All the blend modes and keying stay"; BF260 "We will also add masks and moving masks"
RULE: His words: "All the blend modes and keying stay, and they will be built when there is time. Right now they're just sitting there as placeholder. We will also add masks and moving masks with which are alpha channels". (1) WHAT STAYS. Every entry of the layer's blend list stays. The layer's keying stays: its list with every entry and, read in with it, the keying slider (INFERRED: he writes "keying" and does not name the slider; his round-1 words named the two together, "the keying and slider"). Nothing of them is removed from the layer strip, from the Layer tab or from the show file: a show keeps each layer's blend mode, its keying choice and its keying slider value. (2) THE COMING BUILD ("Right now they're just sitting there as placeholder"). It leaves all of it alone: no entry is removed, none is mended and none is built; every blend entry, every keying entry and the keying slider do exactly what they did before that build, whether they work or not. Nothing that the coming build adds may count on a blend mode or a keying entry that is not built. (3) WHEN THEY ARE BUILT. "they will be built when there is time": all of them, later; he gives no date. Read with his round-1 answer on these lists ("203 c this is built after all other parts are done. This is last, before the ui redesign which is the final change"): they are the last function work, after every other part is built and working and before the UI redesign (Harmony's reading of "when there is time": I3-1). Before that work starts he is shown what each blend entry, each keying entry and the keying slider will do (I-5); no entry is dropped in it. (4) WHAT IS ALREADY RULED FOR THEN. A layer has one blend mode, one transparency slider, one keying choice and one keying slider. The transparency slider sets how strongly the layer is laid over the layers below with its blend mode, for every blend mode: fully down, the layer adds nothing; fully up, full strength (this half of his round-1 words, "the transparency slider to control that layers blend mode", is not touched by BF260 and stands, with @@ITEM 204 part (2) as amended). A picture's own see-through parts stay see-through whatever is built for the keying (I-4). What each keying entry and the keying slider do is NOT ruled here: it is shown to him first (I-5), and his own earlier doubt stands until then ("Does the keying slider actually do anything?", binding-decisions.md:654). (5) EVERY LAYER THE SAME. The first sentence of the item as he read it is not named by his words and is kept as Harmony's reading (I3-3): every layer, the bottom one too, is blended in by its blend mode and its transparency slider (under the bottom layer there is only black); it comes with the blend modes, at that later stage. In the coming build nothing about a layer's kind changes: no control for it is added, and nothing that belongs to a kind is taken out (effects-only layers, the autopilot's count per kind of layer). The switch that makes a layer a mask comes with the masks (6). (6) WHAT IS NEW: MASKS AND MOVING MASKS. Firm from his words: the app gets masks and moving masks, and a mask is an alpha channel, that is, it decides where a picture shows and where it is see-through. The item's sentence "There are no mask layers" is void. Not said by him: what a mask cuts, where a mask comes from, what makes it a moving one, and when it is built. Best reading, Harmony's and not his word (I3-2): a mask is a layer that is switched to work as a mask; the clip playing on it is not shown itself but is used as the alpha channel of the layers below it: where the mask is solid the layers below show, where it is see-through they are hidden. It is a moving mask when its picture moves: its clip is a video, a generated picture or the camera, or it is a still picture that its own sliders, a signal or an action move. What counts is the mask picture's own see-through parts and, for a picture that has none, its brightness (I3-8). A mask layer is triggered, fades and takes effects, signals and actions like any layer, so a moving mask can be on the beat. Masks are built with the blend modes and the keying, not in the coming build (I3-1), and nothing of them is built before he has answered I3-2. (7) An effects-only cell and the effects Chroma Key and Key Palette: with @@ITEM 204 parts (4) and (5) as they stand.
CHANGED: Against the item as he read it: its last sentence ("There are no mask layers") is REPLACED by his words (masks and moving masks are added); its way b ("Mask layers stay; only the keying list and its slider go") is not taken either: the keying stays too. Its first sentence is not named by him and stands as a reading (I3-3). Against @@ITEM 204: part (1), the removal of the keying list and the keying slider, is REVERSED (his newest words win); the sentence that this is built in the coming build is void; part (3) no longer waits on an answer. Against old assumption I-1 (the item's source): "Mask layers ... go with it" is contradicted. NEW from him: masks, moving masks, "which are alpha channels". INFERRED: "with which are alpha channels" is read as "which are alpha channels". INFERRED: "keying" takes in the keying slider. Changed by the ruling: the coming build leaves every entry exactly as it is (the paper's own fallback to Normal for an entry that is not built is cut); what the keying entries and the keying slider do is no longer ruled; the rule no longer says both that no control for a layer's kind is added and that a layer is switched to a mask; both senses of "moving" are covered; nothing of masks is built before his answer.
TODAY: From @@ITEM 204 and 203 TODAY (area-cue-layers.md items 14, 15, 18, 19, 20, 22, H13, M4): the strip's V drop-down holds 13 keying + 55 blend entries; as a blend only Normal, Additive, Screen, Multiply, Darken, Lighten exist and the last four ignore the layer's opacity; the K slider writes keyThreshold; keying runs only on Transparent layers; an Opaque layer (layer 1) ignores blend and keying. Layer::Type has five kinds, Mask among them ("Content becomes alpha mask for layers below", src/model/Layer.h:199, read not run by the paper's architect; drawn at CompositorEngine.cpp:1209 by the fact sheet); no screen sets a layer's kind. What changes for the coming build: the planned removal of keyingMode / keyThreshold / keySoftness and the mending of four blend modes are taken OUT of it; nothing is removed and nothing is mended. For the later stage: every blend and keying entry built, the mask kind given a control (if I3-2 holds), the bottom layer blended like the others.
@@END

@@ITEM 241
TITLE: Two types of envelope: on the beat and on the playhead
STATUS: ANSWERED in his own words (neither the item nor its way b as written)
HIS: BF261 "There should be two different types of envelopes that could work"
RULE: His words: "There should be two different types of envelopes that could work. One is along the beat, which is like a signal, but personalized for that clip, and another one is play head based so we can draw a envelope that happens over the course of the playing of that clip in timeline mode." An envelope is a shape he draws; it is of one of two types. (1) THE ENVELOPE ON THE BEAT ("along the beat, which is like a signal"). It runs on the beat exactly as a signal does: it has a shape and a length in bars and beats; a slider or button that uses it has Looping / One Shot, Gain, Falloff, Range, Invert, the restart-on-a-drop switch and, for a button, a Threshold, with @@ITEM R221 (a) as it stands and as amended. A Looping user is locked to the count of bars like every signal, so it does not start over when its clip is triggered; to start a shape with the clip there is One Shot, which runs as item 242 says. It holds on a tempo pause and stands on the "1" on a tempo stop, with @@ITEM R210 (c) as amended. A sample envelope is of this type (its handles snap to bar lines: @@ITEM D30). (2) THE ENVELOPE ON THE PLAYHEAD ("play head based so we can draw a envelope that happens over the course of the playing of that clip in timeline mode"). Its drawn shape is stretched over the clip from its start to its end (the In and Out points: I-8), and the value is read at the place of the clip's playhead. So it happens once over one pass of the clip, repeats with every pass when the clip loops, runs backwards when the clip plays backwards, jumps when the playhead jumps or is dragged, and holds when the clip is paused; it does not follow the beat clock, and a tempo pause holds it only if it holds the clip. It has no length of its own. Its users have Gain, Falloff, Range, Invert and, for a button, a Threshold; Looping / One Shot is not shown: the playhead decides (I-11). WHERE, by his words ("that clip in timeline mode"): it is offered on the sliders and buttons of a clip in Timeline mode. It is greyed on a clip in BPM mode and on a clip that has no length of its own (@@ITEM R220 (c)), and it is not offered on a layer's controls nor in the Global tab; a control that uses one rests where the hand set it for as long as its clip is in BPM mode (Harmony's). That it could also serve a clip in BPM mode or a layer's slider is not his word and is not built unless he says so (I3-5). (3) WHOSE AN ENVELOPE IS ("personalized for that clip"). An envelope made for a clip's slider or button is that clip's own: it is saved with the clip, goes wherever the clip goes (another cell, another deck, a saved deck), a copy of the clip gets its own copy of it, and it is offered to that clip's sliders and buttons only, those of its effects included; several of them can share it, and a clip can have any number (his round-1 words: "we can have infinite envelopes for different sliders. We can have one per slider or have many sliders share the same one."). An envelope on the playhead is always a clip's own. His new words speak of the clip only and take nothing away from a layer's or a Global slider or button: those use the envelopes that belong to the whole show, as @@ITEM R195 (d) says (Mod 2, and every new signal of his own that is made for an envelope); these are of the type on the beat, and any slider or button of the show can use them, a clip's too (Harmony's reading: I3-4). (4) The entry Timeline stays what @@ITEM 205 and @@ITEM R220 say: the straight link of a control to the playhead, with no drawn shape, offered on a clip's and on a layer's controls. An envelope on the playhead is the drawn form of the same link, for a clip. (5) Both types are drawn in the Signal tab's envelope editor (his earlier words, with @@ITEM D30 and P31 as they stand). A clip's own envelope gets its type when it is made, and the type can be changed afterwards, the shape being kept (I-17). The moving line: on an envelope on the beat it shows the place in the bar count; on an envelope on the playhead it shows the playhead's place (I-15).
CHANGED: Against the item as he read it: "always runs on the beat, never along the clip's playhead" is REPLACED: both exist. Against way b: taken in substance (a shape stretched over the clip from start to end), and he adds "in timeline mode" and "personalized for that clip". Against @@ITEM 205 part (4) and old assumption I-6: the best reading "an envelope runs on the beat" is replaced; his earlier "build." (a drawn curve along the clip) is upheld by his newest words. NEW from him: an envelope is personal to a clip. Not said by him: whether the playhead type also serves a clip in BPM mode or a layer's slider (I3-5: the rule follows his words, "that clip in timeline mode"); whether envelopes of the whole show go on beside the clip's own (I3-4). Changed by the ruling: the playhead type is a clip's own and for a clip in Timeline mode (the paper offered it on a layer's slider and in BPM mode as well, and in the same part gave it to its clip alone); whose an envelope is has a part of its own; a Looping user does not start over with its clip; the sample envelope is placed with the type on the beat.
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
STATUS: ACCEPTED as written; his words under the answer above it (BF244) add that their sync can be adjusted
HIS: box left empty = accepted as written; BF244 "we should just be able to adjust the sink"; BF244 "maybe we could use a common macro that we could set later"
RULE: (1) ACCEPTED AS WRITTEN. An effect that reads the beat by itself (a strobe, a pulse) follows the beat clock. While the beat is PAUSED it holds where it is in its pulse and goes on from there, in step, when the beat plays again. While the beat is STOPPED it stands at its place for the "1" and starts with the new "1". With the Master Signal at 0 it keeps pulsing while the beat runs; an effect that listens to the music itself keeps moving; all with @@ITEM R221 (b) as amended. (BF271, "A pause would not pause it unless it is connected to the BPM", is said of the audio file: the same principle, not a ruling of his on effects.) (2) ADDED BY HIS WORDS under the answer (BF244): "For these effects, we should just be able to adjust the sink, and maybe we could use a common macro that we could set later for example shows where a single macro controls the speed of all of these type of effects". FIRM: for every such effect its sync can be adjusted, on the effect itself ("the sink" is read as the sync: INFERRED). NOT SAID: what is adjusted. Best reading, Harmony's and not his word, taken from the same sentence, in which one macro would control "the speed of all of these type of effects" (I3-6): each such effect has a Sync slider that sets how fast it pulses against the beat. The slider moves in steps, each step one length in bars and beats (the list is Harmony's, written like the other lists of lengths: 4 bars, 1 bar, 2 beats, 1 beat, 1/2 beat, 1/4 beat; which steps an effect gets is fixed at the build). Whatever is set, the effect stays locked to the beat and to the "1", and (1) holds at every setting. An effect that has a speed or rate slider of its own gets no second one: that slider is its Sync. Sync starts at a step picked per effect at the build, so that at 120 BPM the effect looks about as fast as it did before (Harmony's; tuned at the build). Sync is a slider like any other of the effect: it is saved with the show and held by a preset, can be put on a key, a pad or a MIDI knob, is recorded, can be moved by an action, and can take a signal or a macro knob. Nothing of Sync is built before he has answered I3-6. (3) MAYBE, NOT FIRM: "maybe we could use a common macro that we could set later". One macro for the speed of all effects of this type is his idea, in his words a "maybe" and "later". "set later" reads two ways: built later, or set up by him later in a show; both are in I3-7. What holds either way: because Sync can take a macro knob, each bank (a clip's, a layer's, the Global one) can drive the Sync of its own effects with one knob. What does not follow from it: one knob for such effects on clips, on layers and on Global together, because a slider follows only its own tab's bank (@@ITEM R210 (a)); that needs something new and is built only when he says so (I3-7). Nothing built for Sync shuts it out. (4) WHICH EFFECTS. Strobe and Pulse, the two that were named to him, and every other effect whose picture flashes or pulses by itself at a speed of its own; the list is made when this part is planned and shown to him (I3-9). Each of them follows the beat clock after the build, whatever it followed before it: that is the rule he accepted in (1). His words name effects only: a generated picture that pulses with the beat by itself is not ruled here (topic K).
CHANGED: The item itself: nothing, accepted as written. ADDED by BF244: the sync of each such effect can be adjusted (firm); a common macro for the speed of all of them (maybe). INFERRED: "the sink" = the sync. Harmony's reading and not his word: that the sync he wants to adjust is the effect's speed against the beat (from "controls the speed" in the same sentence); the other readings are in I3-6. Changed by the ruling: Strobe and Pulse count by name and are put on the beat clock (they do not read the beat before the build: see TODAY); an effect's own speed slider is its Sync; Sync is a slider in steps, not a list; it has a starting step; the list of lengths is marked as Harmony's; "set later" is read both ways; BF271 is no longer cited as his agreement about effects.
TODAY: Read, not run, at HEAD 33989a8: the Strobe effect has the sliders rate and intensity and flashes 1 to 16 times a second on plain time (src/render/EmbeddedShaders.h:2216-2234; src/effects/EffectLibrary.cpp:408-411); the Pulse effect has amount and speed and swells on plain time (EmbeddedShaders.h:2236-2253; EffectLibrary.cpp:413-416). Neither reads the beat: the time they run on is seconds since the renderer started (src/render/Renderer.cpp:646-648). By a search of uniform names (u_beatPhase, u_bpm and the like), not shader by shader: among the shaders that are not sources only compositeBlend names a beat uniform; about 50 source shaders do. Whether program code outside the shaders drives an effect by the beat: not checked. From @@ITEM R221 TODAY (area-effects-signals.md 1.18, E11, N7): no pause or stop of the beat exists yet, so nothing holds; the fact sheet's "effects that read the beat directly keep pulsing" at Master Signal 0 names no effect. To be built: Strobe, Pulse and the effects like them put on the beat clock; the hold on pause and stop; their speed slider turned into a Sync slider in steps that can take a signal or a macro knob.
@@END

@@ITEM 268
TITLE: Falloff: up at once, down slowly, instant to 2 seconds
STATUS: ACCEPTED as written
HIS: box left empty = accepted as written
RULE: Falloff is the one smoothing control on a slider or button that uses a signal. The value goes up with the signal at once, with no delay; when the signal drops, the value comes down slowly, in the time Falloff is set to, from instant (the value follows the signal down at once) to 2 seconds. The way up is never slowed, and there is no second control for the rise. Each user of a signal has its own Falloff, saved with the show; on a button the Threshold is tested after Gain and Falloff. With @@ITEM R221 (a) parts 3 and 5 as they stand and as amended.
CHANGED: nothing: accepted as written. It closes old assumption I-13 and adds the range (instant to 2 seconds) to the old rule.
TODAY: From @@ITEM R221 TODAY (area-effects-signals.md X1): ConnShape has smoothingMs, saved, with no control on screen; whether it smooths one way or both: not checked.
@@END

## ITEMS (the items of the first page; a RULE that page 2 changed carries "[page 2, ...]" marks where it changed)
@@ITEM 203
TITLE: Blend and transition lists are tidied last
STATUS: ANSWERED c, plus his words on when -- AMENDED after page 2 (203 1 by I, 203 2 by I)
HIS: L110, L111
RULE: Option C: the blend list and the transition list are left as they are for the coming builds. His words: "203 c this is built after all other parts are done. This is last, before the ui redesign which is the final change". So the tidy-up of these lists is the last function work: after every other part is built and working, and before the UI redesign. Nothing of these lists is done before then: the keying entries and the keying slider stay, no blend mode is removed, and none is mended in the coming build (BF260: "All the blend modes and keying stay, and they will be built when there is time"; that "when there is time" is this last stage is assumption I3-1). Masks and moving masks are added in the same stage (BF260; I3-1, I3-2). [page 2, I: BF260 "All the blend modes and keying stay"] The fade slider and its list of transitions stay. No blend mode and no keying entry is removed: all stay and all are built (BF260). What each entry will do, and which transitions stay, is put to him when that last part is planned (I-5). [page 2, I: BF260 "they will be built when there is time"]
CHANGED: Default A (the proposal of the page of 2026-10-04) is not taken; C is. The keying third of the question is overtaken by L111.
TODAY: area-cue-layers.md items 15, 22, H14, A1: the strip's V drop-down holds 13 keying + 55 blend entries, the F drop-down 55 transition names; as a blend only Normal, Additive, Screen, Multiply, Darken, Lighten exist, and the last four ignore the layer's opacity; 15 real transitions, 40 fall to Dissolve. Nothing changes here until the last stage, except the keying removal and the four modes obeying opacity (I-3).
@@END

@@ITEM 204
TITLE: Keying and its slider go; transparency slider blends the layer
STATUS: ANSWERED in his words (no letter); the layer's kind is not named by them (I-1) -- AMENDED after page 2 (204 1 by I, 204 2 by I, 204 3 by I, 204 4 by I, 204 5 by I, 204 6 by I, 204 7 by I)
HIS: L111, L51, L110
RULE: His words: "204 I want to remove the keying and slider. We are only going to use the transparency slider to control that layers blend mode." (1) REVERSED by his newest words (BF260: "All the blend modes and keying stay, and they will be built when there is time. Right now they're just sitting there as placeholder."): the keying list with all its entries and the keying slider STAY on the layer strip and in the Layer tab, as placeholders until they are built; a layer keeps its keying choice and its keying slider value and a show carries them; every blend mode stays too (item 240 of page 2). [page 2, I: BF260 "All the blend modes and keying stay"] His round-1 words named "the keying and slider" together: the keying list and the keying slider, as in his earlier "Does the keying slider actually do anything? Maybe we get rid of it." (binding-decisions.md:654-655). "keying" in BF260 is read in the same way, the slider with the list (INFERRED: BF260 does not name the slider), so both stay [page 2, I: BF260 "All the blend modes and keying stay"]; the fade slider with its list of transitions stays (L110) [page 2, I: BF260 "All the blend modes and keying stay"]. A picture's own see-through parts (a logo) stay see-through on every layer, whatever is later built for the keying (I-4) [page 2, I: BF260 "All the blend modes and keying stay"]. (2) A layer has one blend mode and one transparency slider. The transparency slider sets how strongly the layer is laid over the layers below with that blend mode: fully down, the layer adds nothing to what is below it (under the bottom layer there is only black); fully up, it is laid over at full strength; every blend mode follows the slider (reading of "control that layers blend mode": I-2). This is built when the blend modes are built ("they will be built when there is time", BF260), which is read as the last stage and not the coming build (I3-1). [page 2, I: BF260 "they will be built when there is time"] (3) The layer's kind, which the question asked about, is not named by his words, and they take none of the three letters. Best reading, Harmony's and not his word: every layer, the bottom one too, is blended in the same way (he read this as the first sentence of item 240 of page 2 and did not speak against it: I3-3); no kind is picked for a layer in the coming build, and a layer that works as a mask comes with the masks (BF260; I3-2) [page 2, I: BF260 "We will also add masks and moving masks"], because he writes "We are only going to use the transparency slider" and "that layers blend mode" (the layer the question was about). It was put to him as item 240 of page 2. His answer (BF260) names no kind: it keeps the blend modes and the keying and adds masks and moving masks, "which are alpha channels". So in the coming build no control for a layer's kind is added and nothing is taken out for a kind (effects-only layers, the autopilot's count per kind of layer) (I3-3); what a mask is and what it cuts is assumption I3-2, and the switch that makes a layer a mask comes with the masks. [page 2, I: BF260 "We will also add masks and moving masks"] (4) An effects-only cell works on any layer and acts on the layers below it; on the bottom layer it has nothing to act on (his L51: "a cell with effect(s) only can be used similar to a layer effect. They will affect the layers below it. If there is an effect at the bottom layer, it will not be effective"). (5) The effects named Chroma Key and Key Palette in the effects list are effects, not the layer's keying, and stay (I-4).
CHANGED: None of A / B / C is taken as written. Added by him: the keying list and the keying slider are removed; the transparency slider is the one control of the layer's blend. The question's own subject (the hidden kind) is not named by his words: it is asked (I-1), with "no kind" as the best reading and the page's default A as the other way.
TODAY: area-cue-layers.md items 14, 15, 18, 20, 22, H13, M4, T10: K slider writes keyThreshold (a plain float); keying runs only on Transparent layers; an Opaque layer (layer 1) ignores blend and keying and is black at fader 0; Screen, Multiply, Darken, Lighten ignore the layer's opacity; Layer::Type is read by the compositor, the Layer tab and the Per-Type Autopilot (Autopilot.cpp:148-162). Read by me: src/model/Layer.h:193-201 (five kinds: Opaque = the default, Transparent, FXOnly, ThreeD, Mask) and :255-261 (thirteen keying modes; Alpha is the default: what it does for a picture's own transparency must remain as fixed behaviour when keyingMode goes). To change: remove keyingMode / keyThreshold / keySoftness UI and render path; make every blend mode obey opacity; the fate of Layer::Type waits for I-1. EffectLibrary.cpp:531, 749 (Chroma Key, Key Palette effects exist).
@@END

@@ITEM 205
TITLE: "Timeline" is the link to the layer's playhead
STATUS: ANSWERED in his words (no letter); two parts open (I-6, I-7) -- AMENDED after page 2 (205 1 by I)
HIS: L112, L104, L106
RULE: His words: "205 these terms are confusing. Timeline is connecting anything that can be connected to the layers playhead and should be called as such. The envelope is the envelope and I have already described this above." (1) The connection of a control to the layer's playhead is one thing with one entry: wherever a slider or a button can be connected to the playhead, its list of signals has ONE entry for it, and the second entry for the same connection ("Clip Position") goes. The name on that entry is read as "Timeline", his word for it here and earlier ("when I set any clip parameter to timeline it should be locked to the clips playhead", binding-decisions.md:591). "should be called as such" can also mean that the link is to be named for what it is, the playhead: both ways are in I-7. The same word is also his word for how a clip plays ("timeline mode", L21): which of the two carries the name in the list of names is for the ruling over all topics. (2) What it does: the control follows the playhead of the clip. A clip's control follows that clip's own playhead; a layer's control follows the playhead of the clip playing on that layer; it is not offered in the Global tab (R220 a, b; his earlier words). The value runs from the control's low end at the clip's start to its high end at the clip's end (the In and Out points, by his earlier "It should be as if that is the extent of the timeline", binding-decisions.md:729; I-8), goes wherever the playhead goes (backwards in reverse, jumps with Random and Beat Repeat, follows a drag of the playhead) and holds when the clip is paused. (3) A control on Timeline is a user of a signal (L106): it has its own Gain, Falloff, Range and Invert, and a button has its Threshold. Looping / One Shot is not shown there: the playhead decides (I-11). (4) A shape he draws is an ENVELOPE, never "a Timeline curve": "The envelope is the envelope". Envelopes are as L104 says: any number, one per slider or shared. Timeline itself is the straight link with no drawn shape. An envelope is of one of two types (item 241 of page 2, BF261: "There should be two different types of envelopes that could work"): on the beat, like a signal, or on the playhead, where its drawn shape is stretched over its clip from start to end; whose an envelope is and where each type is offered: item 241 parts (2) and (3). So his answer is that an envelope can also be connected to the playhead [page 2, I: BF261 "another one is play head based so we can draw a envelope"], which was the drawn Timeline curve of his earlier "build." (binding-decisions.md:258).
CHANGED: Neither A nor B as written. Replaced: A's "Timeline is the curve you draw" (a drawn shape is an envelope). In effect B's one entry. Added: "anything that can be connected" = sliders and buttons (L106). The entry's NAME is an assumption (I-7), not his ruling.
TODAY: area-effects-signals.md 1.12, 1.16, E7, T8, O11: the picker lists both "Clip Position" and "Timeline"; "Timeline" is a 4-beat ramp on the beat clock, "Clip Position" reads 0 (clip clock nullptr, ConnectionEngine.cpp:327-372); no editor draws a Timeline curve. To change: wire the clip clock; one entry; drop the beat ramp meaning. E14: "Timeline" also names a clip's transport mode (ClipInspector.cpp:9), the clip's bar and the review screen's rows: the list of names must separate them.
@@END

@@ITEM R195
TITLE: Signals and effects: what stays, with four parts changed
STATUS: CORRECTED d widened and g replaced by him; c and e changed by his words elsewhere -- AMENDED after page 2 (R195 1 by I)
HIS: L104, L106, L66
RULE: (a) stands: a signal is plugged into a slider from the small triangle on its row; taking it off is "Manual". By L106 a button is plugged in and taken off in the same way (see R221; where its control sits: L8). (b) stands: the Master Signal fader scales every signal at once and does not touch actions; the ignore lamp is for actions only. (c) After he lets go of a slider: a slider that a signal drives glides back to the signal ("smooth transition back."), in a short time of its own, as it stood on the page (I-18); a slider that an action moves ALSO glides back, to the action's position, in the time of the Global glide setting (L66: "the slider moves back to the action position based on the global glide back setting I described earlier"). (d) His words: "we can have infinite envelopes for different sliders. We can have one per slider or have many sliders share the same one." There is no limit on the number of envelopes: a new envelope can be made for one slider, and one envelope can be used by many sliders; a change to a shared envelope shows on every slider that uses it. Each envelope has its own shape and is saved with the show. An envelope is of one of two types (item 241 of page 2, BF261): on the beat, with a length of its own in bars and beats, or on the playhead, with no length (it is stretched over its clip). An envelope made for a clip's slider or button is that clip's own: it is saved and copied with the clip and offered to that clip's sliders and buttons only ("personalized for that clip", BF261); an envelope on the playhead is always a clip's own. A layer's or a Global slider or button uses the envelopes that belong to the whole show, which are of the type on the beat (Harmony's reading: I3-4). [page 2, I: BF261 "like a signal, but personalized for that clip"] The rest of (d) stands as it was shown to him: the two signals of his own, Mod 1 and Mod 2, stay, and new signals can be made "only for envelopes" (the oscillators offered in a slider's list stay as they are). How a new envelope is made and named: I-17. (e) A control that uses a signal keeps Range and Invert and gets its own Gain and Falloff as well (L106: "Each signal user should also have it’s own gain and falloff (smoothing)"); the full rule is under R221. (f) stands: sliders read 0 to 1, without units. (g) His words: "G we want to re-order effects". Effects in one stack (a clip's, a layer's, the global one) are re-ordered by dragging an effect's header bar to a new place, as the page offered ("re-order effects by dragging") and as in Resolume; the picture follows the new order at once; whatever points at an effect (an action, a recording's row, a key or pad of the mapping, a signal on its sliders) stays with that effect when it moves (I-19). The rest of (g) stands: a new effect is added at the end, and single effects are not copied or pasted.
CHANGED: d ADDED TO, not replaced (page: two signals of his own, Mod 1 and Mod 2; "more can be made only for envelopes, when that part is built"): new is "infinite", "one per slider" and "many sliders share the same one"; Mod 1, Mod 2 and "only for envelopes" stand. g REPLACED in its first half (page: effects cannot be re-ordered); the rest of g stands. c REPLACED in its second half by L66 (page: a slider that an action moves jumps, 158 A); its first half stands. e REPLACED in "no smoothing control" by L106. a, b, f stand.
TODAY: area-effects-signals.md 1.07 (no re-order, effects appended at the end), 1.13 (Mod 1 = a sine oscillator, Mod 2 = an envelope; the user cannot make another), 1.15 (nothing of a signal's definition is saved with the show), 1.19 (rank chain), 2.17 (the glide back to a signal is built; its time not re-read), X1 (ConnShape has smoothingMs, curve, inMin/inMax saved, no UI), T1 (effects addressed by POSITION in shows, takes, actions, bindings: a re-order needs a stable id). All of these must change except the glide back to a signal.
@@END

@@ITEM R210
TITLE: Macros in three banks, the drop switch, signals on pause
STATUS: STANDS -- the pause half of c is his own rule now (L13) -- AMENDED after page 2 (R210 1 by I)
HIS: L13, L12, L31, L55, L114
RULE: (a) Macros: three kinds of bank, 8 knobs each: every clip its own 8, every layer its own 8, one global 8; a slider plugged into Macro 3 follows the third knob of its own tab's bank (the clip's, the layer's, the global one). The panel is called "Macros" (L114: "Macro’s panel should be called Macros"); the third bank is "global" (L55). (b) The switch "an oscillator restarts on a drop, or flows through it" gets a control on the row of the slider that uses the signal; this fits L106 (such choices sit on the user of the signal). (c) Signals that run on the beat (oscillators, envelopes on the beat; an envelope on the playhead follows its clip's playhead and not the beat clock: item 241 of page 2) [page 2, I: BF261 "another one is play head based so we can draw a envelope"]. PAUSED: they hold where they are and go on from there when the beat plays again (L13: "Pause pauses, the beat clock, the clips and everything that it controls with BPM."; L31: "the clock will continue from where it was paused as well"). STOPPED: they stand on the "1" until the beat runs again and start from their beginning on the new "1" (the standing reading of the tempo topic, R214 a, which he did not name; it fits his L12: "When stopped, user’s click on play or any clip is the new 1."). Signals that listen to the music keep moving in both.
CHANGED: nothing replaced. The pause half of c was Harmony's and is now carried by L13. The stop half stays the page's reading (inferred consent; it stands with R214 a) and fits L12. Added from L13 and L31: after a pause they go on from where they were. The name "Macros" added (L114).
TODAY: area-effects-signals.md 1.17 (one global bank shown in all three tabs, titled "Dashboard", nothing saved), X3 (resetPhaseOnStructural exists, no control), E11 (beat-locked signals have no pause / stop behaviour because the beat has no pause or stop yet).
@@END

@@ITEM R220
TITLE: Where a slider can follow the Timeline
STATUS: STANDS -- L112 fits it; the name of the entry hangs on I-7 -- AMENDED after page 2 (R220 1 by I)
HIS: L112, L106
RULE: (a) The link to the playhead (the entry "Timeline": I-7) is offered on a clip's controls and on a layer's controls, not in the Global tab. (b) On a layer it follows the playhead of the clip that is playing on that layer. (c) On a clip that has no length of its own (MilkDrop, a generated picture, a still picture, the camera, an effects-only clip) the entry is greyed; every other signal still works there. L112 fits all three: "connecting anything that can be connected to the layers playhead" -- what has no playhead cannot be connected. By L106 the entry is offered on buttons as well as sliders. An envelope on the playhead (item 241 of page 2, BF261) is narrower than the entry Timeline: it is a clip's own and is offered on the sliders and buttons of a clip in Timeline mode, where it follows that clip's playhead; it is greyed on a clip in BPM mode and on a clip that has no length of its own, and it is not offered on a layer's controls or in the Global tab. That it could also serve a clip in BPM mode or a layer's slider is not his word (I3-5). [page 2, I: BF261 "over the course of the playing of that clip in timeline mode"]
CHANGED: nothing. Added from L106 / L112: buttons too.
TODAY: area-effects-signals.md 1.16, N6, T8: the picker offers "Timeline" everywhere, as a beat ramp; no greying.
@@END

@@ITEM R221
TITLE: Looping, One Shot, Gain, Falloff, Threshold move to the user
STATUS: CORRECTED a moved to the user by him, c replaced, b is his question back, d stands -- AMENDED after page 2 (R221 1 by I, R221 2 by I, R221 3 by I, R221 4 by I, R221 5 by I)
HIS: L106, L107, L108, L13
RULE: (a) THE RULE OF L106. His words: "the one shot and looping controls should be on user of the signal (clip, layer or global slider and button), not the signal itself. The slider or button can use the signal. Slider is simple and button needs a threshold setting. Once the signal is above the threshold the button will be on (or off it’s inverted). Each signal user should also have it’s own gain and falloff (smoothing)." 1. A signal itself has NO Looping and NO One Shot control; the Signal tab holds only what the signal IS: its shape and its length. 2. A USER of a signal is a slider or a button in the Clip tab, the Layer tab or the Global tab (the sliders and buttons of effects included) that has a signal plugged in. A button is plugged in and taken off as a slider is (the same list of signals; "Manual" takes it off); which buttons: I-14. Each user has its own controls on its own row, and they act on that user only: two sliders on the same signal can be set differently. All of a user's settings are saved with the show. 3. Every user has: Gain (how strongly this user takes the signal; its numbers: I-12); Falloff, his word for the smoothing, read as: the value goes up with the signal at once and comes down slowly, in a time set from instant to 2 seconds; the way up is never slowed (item 268 of page 2, accepted as written) [page 2, I: item 268 accepted as written]; and, standing from R195 e and R210 b (his "should also have"), Range, Invert and the restart-on-a-drop switch. 4. A slider "is simple": it follows the signal through those controls. 5. A button also has a Threshold: while the signal (after Gain and Falloff) is above the Threshold the button is on, below it the button is off; with Invert it is the other way round. 6. Looping / One Shot is read as one choice of two (his "controls"; a user either goes round or plays once) and is shown on a user of an envelope on the beat or of an oscillator, not on a user of an envelope on the playhead, where the playhead decides (item 241 of page 2, BF261; I-11) [page 2, I: BF261 "There should be two different types of envelopes that could work"]; a newly plugged-in user starts as Looping (I-24). Looping: the user follows the signal round and round for as long as it is plugged in, locked to the count of bars, so every Looping user of one signal is at the same place. One Shot: the user follows the signal once, from the signal's beginning to its end, and then keeps the signal's last value until its next run. That it starts on the next "1" and keeps its last value stands from the page's text, which he read and corrected only in where the controls sit. What starts a run is settled by item 242 of page 2, which he accepted as written: the trigger of its clip [page 2, I: item 242 accepted as written] (a clip's user with that clip, a layer's user with every clip fired on that layer; when the click itself is the new "1", the run starts with it), and for a Global user the switching-on of One Shot; switching One Shot on also starts one run from the next "1". That the last value is kept, and not glided back as an action is (L64), is shown to him as a line (I-10). While a One Shot runs, that user is at a place of its own in the signal. (b) Effects that read the beat by themselves (a strobe, a pulse): he asked whether the page's rule is a good idea (L107), read the answer and accepted the rule as item 243 of page 2. His words under the answer add that their sync can be adjusted (BF244: "we should just be able to adjust the sink", read as the sync). That is read as a Sync slider on each such effect which sets, in steps of lengths in bars and beats, how fast it pulses against the beat (I3-6, which is put to him; nothing of Sync is built before his answer); one common macro for the speed of all of them is his "maybe" (I3-7). Strobe, Pulse and the effects like them are put on the beat clock for this rule (item 243 part (4)). The rule, which holds at every Sync setting: [page 2, I: BF244 "we should just be able to adjust the sink"; item 243 accepted as written] such an effect holds where it is while the beat is paused and goes on from there (L13: "Pause pauses, the beat clock, the clips and everything that it controls with BPM."); it stands on the "1" while the beat is stopped and starts with the new "1" (L12); it keeps pulsing with the Master Signal at 0 while the beat runs (his earlier "keep pulsing, this control is only for signals", binding-decisions.md:566); an effect that listens to the music itself keeps moving. (c) His words: "yes I want a moving line on each signal to know where they are." Every signal that has a shape (each oscillator, each envelope, each sample envelope) shows a line that moves across its shape and marks where the signal is at this moment (its Looping place); the line holds still while the beat is paused or stopped; on an envelope on the playhead (item 241 of page 2, BF261) the line is at the playhead's place and moves and stops with the clip [page 2, I: BF261 "over the course of the playing of that clip"]. A signal that listens to the music has no shape to move along: it shows its level at this moment (I-15). Where the line is drawn: laid out by Harmony (L8). (d) stands: loading a preset on Freeze or Echo, or switching one off and on with B, keeps the picture it holds; removing the effect clears it.
CHANGED: a REPLACED in its place only (page: the two toggles stay on the envelope in the Signal tab and are made to work there); the page's start on the next "1" and its "plays once and holds its last value" stand as text he read and did not correct. ADDED by him: a button can use a signal, with a Threshold and Invert; Gain and Falloff per user. b: he asks back (L107); answered, and the page's rule kept as a line he can strike (I-16). c REPLACED (page: no moving line unless he asks). d stands.
TODAY: area-effects-signals.md 1.14 (Looping / One Shot toggles on the envelope are inert, EnvelopeSignal.h:60-68; the editor has no mouse handling), 1.12 (a signal plugs into sliders only; no button takes a signal), 1.13 (the SignalBar shows one meter per visible signal), X1 (smoothingMs, playback, loop exist in ConnShape and are saved, no control on screen), 1.18 (Master Signal scales connections; beat-reading effects keep pulsing), W4 (a manual Resync re-aligns shapes and re-pins a one-shot: binding-decisions.md:534-543), E11 (an earlier question "Play once starts on the next bar or right away? Default: on the next bar" was never clearly answered), O7 / FENV Q2 (no moving line; the Signal tab viewport and Pitfall 57 bound how it is drawn), X5 (what a held picture does on preset load: UNKNOWN in code).
@@END

@@ITEM R196
TITLE: Bars and beats in the lists of lengths
STATUS: REPLACED in two parts (MilkDrop's lists by L126; lengths under a bar by L83, INFERRED); the rest stands -- AMENDED after page 2 (R196 1 by I)
HIS: L83, L126, L99, L14
RULE: Lists of lengths read in bars from one bar up: the oscillator, the envelope, the autopilot's count, the count per kind of layer (it stays: nothing of a layer's kind is taken out, item 240 of page 2, BF260; I3-3) [page 2, I: BF260 "All the blend modes and keying stay"]. Beats stay in the circle of the tempo bar (L14: "A complete circle is one bar, and each of the four spots on the circle are one beat"), in Beat Repeat and Random (the standing reading of how Resolume counts them; his L20 "it randomly moves playhead every single beat" and L101 fit it), and in the length of a BPM-mode clip (L99: "do beats here"). A layer's fade and an output's Delay stay in seconds or milliseconds: they are times, not musical lengths. Two changes from his other lines: (1) MilkDrop's lists and MilkDrop's blend time are not touched in this build (L126: "I want you to create a dedicated document with how milk drop functions currently and that's it for this upcoming build."). (2) A length shorter than a bar is written in beats, the way he wrote the Quantize grid in L83: "4 bars, 1 bar, 2 beats, 1 beat, 1/2 beat, 1/4 beat". L83 is about that one list; carrying it to the other lists is Harmony's reading, shown to him as a line (I-20).
CHANGED: REPLACED by L126: MilkDrop's two lists leave this reading. REPLACED by L83 (INFERRED: his newest way of writing a list of lengths, said about the Quantize grid): the page's rule that a length shorter than a bar "reads as a part of a bar". The rest stands. The beats of Beat Repeat and Random rest on the standing reading R192 (topic H), not on L101.
TODAY: area-effects-signals.md 1.12, 1.14, 1.21, 1.22, E13, W3: oscillator "1/4 Beat" .. "8 Beats", envelope "1 Beat" .. "16 Beats", autopilot and per-type counts in beats.
@@END

@@ITEM D29
TITLE: Layer Router names its source layer and follows it
STATUS: OPEN no word of his touches it
HIS: none
RULE: The Layer Router's "Source Layer" names the layer it takes its picture from (by the layer's name) and stays on that layer when the layers are re-ordered or another layer is removed; any layer can be picked, however many there are. If the named layer is removed, the router shows nothing until another is picked. (Harmony's own: I-21.)
CHANGED: nothing
TODAY: area-cue-layers.md item 27, M6, T7: a 0..1 slider mapped to layer INDEX 0..9 (Renderer.cpp:1285-1296); a re-order re-points it; layers 11 and up cannot be reached.
@@END

@@ITEM D30
TITLE: Sample envelope handles snap to bar lines; no bigger window
STATUS: OPEN only where the bar lines are counted from; the rest is his earlier words
HIS: none
RULE: A sample envelope is used whole or shortened by dragging its two end handles, which snap to the bar lines. His earlier words: "I want to be able to use the whole thing or to shorten it manually but clicking to timing marks." (binding-decisions.md:641); then, asked "Trimming a long sample: drag the two end handles; they snap to the bar lines." (Harmony's question as asked, boris-feedback-backlog.md:215), he answered "drag handles that snap" (binding-decisions.md:678). There is no bigger envelope window; every envelope control works in the Signal tab's editor ("I am fine using the envelope editor in the signal tab. I want all of the controls to work in that one rather than an extra bigger envelope window.", binding-decisions.md:641-643). Harmony's own part: the bar lines are counted from the start of the sample at the tempo of the moment (I-22). The moving line of L108 is drawn in this same editor.
CHANGED: nothing
TODAY: area-effects-signals.md E12, W2, O7, T10: no handles, no time axis; the adopted ruling-bf45 still holds a big view (stage S5) that must be re-based.
@@END

@@ITEM D31
TITLE: One blend list in the strip and the Layer tab
STATUS: OPEN fits L111, not said by him -- AMENDED after page 2 (D31 1 by I, D31 2 by I)
HIS: L111, L110
RULE: A layer has one blend mode ("that layers blend mode", L111); wherever it can be picked (the layer strip, the Layer tab) the list is the same list with the same entries, and both places show the same choice. The keying stays beside it, with its own entries and its slider (BF260). [page 2, I: BF260 "All the blend modes and keying stay"] The two lists are made one at the last stage, together with the tidy-up of their entries (L110); until then they are left as they are, the keying entries too (BF260) [page 2, I: BF260 "All the blend modes and keying stay"]. (I-23.)
CHANGED: nothing replaced; the keying entries leave the list by L111. Added: when it is done (L110).
TODAY: area-cue-layers.md M5: the Layer tab's "Video Blend Mode" lists 25 names, the strip 55; a strip value above 24 shows blank in the Layer tab; "Alpha" appears twice in the strip's V menu.
@@END

@@ITEM P30
TITLE: The strip's drop-down lists after 203
STATUS: DROPPED -- AMENDED after page 2 (P30 1 by I)
HIS: L8, L110, L111
RULE: Laid out by Harmony where it fits in the correct area; its look is settled in the UI redesign (L8). The Keying list stays and is placed by Harmony like the others (BF260) [page 2, I: BF260 "All the blend modes and keying stay"]; the Blend list and the Transition list are tidied at the last stage (L110).
CHANGED: The variant with a Keying list or a "Keying" group is void (L111).
TODAY: area-cue-layers.md item 15: one V drop-down (keying + blend) and a caret-only F drop-down.
@@END

@@ITEM P31
TITLE: Where a drawn shape for the playhead is drawn
STATUS: DROPPED -- AMENDED after page 2 (P31 1 by I)
HIS: L112, L8
RULE: Laid out by Harmony where it fits in the correct area; its look is settled in the UI redesign (L8). By L112 there is no "Timeline curve": a shape he draws is an envelope, and an envelope is drawn in the Signal tab's editor (his earlier words: "I want all of the controls to work in that one rather than an extra bigger envelope window.", binding-decisions.md:641-643). An envelope can also be connected to the playhead (item 241 of page 2, BF261); both types of envelope are drawn in that same editor. [page 2, I: BF261 "There should be two different types of envelopes that could work"]
CHANGED: Both variants were about where "the Timeline curve" is drawn; L112 takes that name away from a drawn shape, and the two variants differ only in where it sits.
TODAY: area-effects-signals.md 1.16, E7, O11: no editor draws a Timeline curve; no plan gives one a host.
@@END

## ASSUMPTIONS STILL OPEN (new after page 2: ids with a 3; old ones never shown to him keep their ids)
@@ASSUME I3-1
ABOUT: 240; 203, 204, D31
TEXT: I assume "when there is time" means after the coming build: blend modes, keying and masks are left alone in it, nothing is taken out, and they are built as the last work before the UI redesign.
WHY: BF260 gives no date ("they will be built when there is time"; "We will also add"). His round-1 words on these lists said last, before the UI redesign ("203 c this is built after all other parts are done. This is last, before the ui redesign which is the final change", binding-decisions.md:1184). Other orders Harmony could have picked and did not: masks in the coming build; every blend mode made to follow the transparency slider in the coming build.
ALT: none
IF-WRONG: SMALL the same work, earlier or later.
ASK: NO his own words carry the order of work, and the order of work is not what the app does (checker F10).
@@END

@@ASSUME I3-2
ABOUT: 240; 204
TEXT: I assume a mask is a layer switched to work as a mask: its clip is not shown, it decides where the layers below show or hide. With a video, or moved by a signal or an action, it is a moving mask.
WHY: BF260 says only "masks and moving masks with which are alpha channels": not what a mask cuts, where it comes from, or what makes it a moving one. The default covers both senses of "moving" (a video; a still picture that is moved). Not looked up: how his Resolume treats a layer in mask mode (no web tools here; a Researcher's task if the default should match it).
ALT: b) A mask layer cuts only the one layer directly below it. c) A mask is a picture or video you put on one clip or one layer, and it cuts only that clip or layer.
IF-WRONG: REBUILD what a mask cuts decides how the picture is put together and what the show file holds.
ASK: YES a new function of his with no word on how it works. It is the lighter of this topic's two questions: masks come at the last stage and nothing of them is built before his answer.
@@END

@@ASSUME I3-3
ABOUT: 240; 204, R196
TEXT: I assume every layer, the bottom one too, is blended in by its blend mode and its transparency slider. No control for a layer's "kind" is added in the coming build; the switch that makes a layer a mask comes with the masks.
WHY: He read this sentence of item 240, and his words (BF260) go only against the removal of the keying and against "There are no mask layers". Nothing that belongs to a kind is taken out (effects-only cells, the autopilot's count per kind of layer).
ALT: b) The bottom layer always shows plainly, whatever its blend mode says.
IF-WRONG: SMALL one layer's behaviour under a blend mode other than Normal.
ASK: NO he read the sentence and left it; nothing is taken out for it.
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

@@ASSUME I3-6
ABOUT: 243; R221
TEXT: I assume your "adjust the sink" (I read: the sync) means: a strobe, a pulse and effects like them run locked to the beat, and each gets a Sync slider that sets how fast it pulses (every bar, every beat, every half beat ...).
WHY: BF244 says "adjust the sink" (read: the sync) and does not say what is adjusted; "controls the speed" in the same sentence points to the rate. Against the default: his own earlier word "sync" was the timing dial that shifted the visuals against the music, which he replaced by a Delay per output ("replace our sync with this", binding-decisions.md:795): that is way c. And for his clips "synced" is one of two ways to play ("either BPM synced throughout the whole thing, or just playing with a speed control", binding-decisions.md:630-631): that is way b, which also keeps a free speed, the only thing Strobe and Pulse have before the build (ruling H2).
ALT: b) Sync is a switch on each of them: on, its speed is locked to the beat; off, it runs free at the speed you set. c) Sync shifts the effect earlier or later against the beat, so its flash lands where you want; that is how you used the word sync before.
IF-WRONG: STAGE a strobe that cannot be set the way he means it is seen at once; one control on every such effect to change.
ASK: YES a dictated word that can be read three ways; it decides a control he will use live, in the coming build. The first question of this topic.
@@END

@@ASSUME I3-7
ABOUT: 243; R210
TEXT: I assume the common macro waits, as your "maybe": each such effect gets its own Sync, which a macro knob of its own clip, layer or Global can turn. One knob for all of them across the show is built when you ask.
WHY: BF244 says "maybe we could use a common macro that we could set later": a maybe. "set later" reads two ways: built later, or set up by him later in a show. Macros are in three kinds of bank and a slider follows only its own tab's bank (old item R210 a; his "all. Global, layer, clip.", binding-decisions.md:263), so one knob for effects on clips, on layers and on Global together needs something new: a slider that can be plugged into a Global macro knob from anywhere. That is the price of way b.
ALT: b) It is built with the rest: the Sync of any such effect, wherever it sits, can be plugged into one Global macro knob, so you set it up in a show.
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

@@ASSUME I-12
ABOUT: R221, R195
TEXT: I assume Gain runs from 0 to 4 times (1 = the signal as it is) and is applied before Falloff, Invert and Range. Range and Invert stay next to it.
WHY: L106 names "gain" with no numbers; "should also have" keeps what a user of a signal has (Range, Invert).
ALT: b) Another top value than 4.
IF-WRONG: SMALL a number.
ASK: NO a number that is tuned at the build.
@@END

@@ASSUME I-15
ABOUT: R221; 241
TEXT: I assume the moving line is drawn on every signal that has a shape (oscillators, envelopes, sample envelopes) and shows where its loop is; on an envelope that follows the playhead it shows the playhead's place. A music signal shows its level.
WHY: L108 says "a moving line on each signal to know where they are"; BF261 adds an envelope that follows a clip's playhead, which has no loop place of its own; a music signal has no shape, and a One Shot (L106) runs at a place of its own.
ALT: b) Music signals get nothing extra. c) The line shows the place of the slider you are working on, a One Shot's run too.
IF-WRONG: SMALL what one line means.
ASK: NO what the line shows on each kind of signal is a look (L8); it was internal before and stays so.
@@END

@@ASSUME I-17
ABOUT: R195; 241
TEXT: I assume a new envelope made from a clip's slider or button is that clip's own; one made anywhere else belongs to the whole show. Its type is picked when it is made and can be changed. Deleting one sets its sliders back to Manual.
WHY: L104 and BF261 say how many envelopes and which two types, not how one is made, named, switched or deleted; whose a new one is follows item 241 part (3). Only a clip's own envelope has a type to pick: the envelopes of the whole show run on the beat. Where the entry sits is his L8.
ALT: b) The type is fixed once the envelope is made.
IF-WRONG: SMALL a menu entry and names.
ASK: NO layout and names (L8).
@@END

@@ASSUME I-18
ABOUT: R195
TEXT: I assume a slider that a signal drives glides back to the signal in a short time of its own when you let go. The Global glide slider times what actions do, not this.
WHY: "smooth transition back." gives no time and stood on the page; L64 gives the Global glide slider for values going "back to their pre-action positions" only.
ALT: b) The Global glide slider times the glide back to a signal too.
IF-WRONG: SMALL one time value.
ASK: NO the standing half of the reading; L64 speaks of actions.
@@END

@@ASSUME I-19
ABOUT: R195
TEXT: I assume you re-order effects by dragging an effect's header bar, as in Resolume, and that actions, recordings, keys and pads that point at an effect stay with it when it moves.
WHY: L104 says "we want to re-order effects"; dragging is what the page offered him. What happens to what points at an effect is not said.
ALT: none
IF-WRONG: SMALL the gesture; the second half is how it must work.
ASK: NO technical: an effect gets a lasting identity.
@@END

@@ASSUME I-20
ABOUT: R196
TEXT: I assume every list of lengths is written the way you wrote the Quantize grid: bars from one bar up, beats below it (4 bars, 1 bar, 2 beats, 1 beat, 1/2 beat, 1/4 beat).
WHY: L83 writes that one list so; earlier he wrote "1/2 bar" and "bars in most places unless beats are necessary", and the page's "part of a bar" was not corrected.
ALT: b) Lengths under a bar are written as parts of a bar (1/2 bar, 1/4 bar) in the lists of the oscillator, the envelope and the autopilot; only the Quantize grid reads in beats.
IF-WRONG: SMALL words in a list.
ASK: LINE two ways of his own; wording only.
@@END

@@ASSUME I-21
ABOUT: D29
TEXT: I assume the Layer Router names the layer it takes its picture from and stays on that layer when you re-order layers; if that layer is removed it shows nothing until you pick another.
WHY: No line of his names the Layer Router (L130: he did not read this list).
ALT: b) It follows the place in the stack (the third layer, whichever that is).
IF-WRONG: STAGE after a re-order the router would show another layer's picture; cheap to change.
ASK: LINE he would most likely wave it through.
@@END

@@ASSUME I-22
ABOUT: D30
TEXT: I assume the bar lines that a sample envelope's two end handles snap to are counted from the start of the sample, at the tempo of that moment.
WHY: Bar lines are settled (asked "they snap to the bar lines", he answered "drag handles that snap"); where bar 1 of a sample lies is not said. L130: not read.
ALT: b) They are counted from the sample's own first beat, which the app would have to find.
IF-WRONG: SMALL the handles land a little off the music on a sample that does not start on a beat.
ASK: LINE he can strike it.
@@END

@@ASSUME I-23
ABOUT: D31
TEXT: I assume the blend mode of a layer is picked from one and the same list wherever it is shown, and both places always show the same choice.
WHY: L111 speaks of "that layers blend mode" (one); two lists for it are not named. L130: not read.
ALT: none
IF-WRONG: SMALL a list.
ASK: NO consistency of one setting.
@@END

@@ASSUME I-24
ABOUT: R221, R195
TEXT: I assume a newly plugged-in slider or button starts as Looping; a button switched by a signal has a small margin against flicker and rests as you set it by hand while the Master Signal is fully down.
WHY: L106 and L104 do not go into the starting choice, flicker or the Master Signal at 0. Saving is covered by his One Save rule and written into the rule itself.
ALT: none
IF-WRONG: SMALL internal.
ASK: NO technical.
@@END

## CLOSED ASSUMPTIONS (one line each)
- I-1 -> page 2, item 240
- I-2 -> SETTLED
- I-3 -> CLOSED by page 2 (I): Contradicted by BF260 ("All the blend modes and keying stay, and they will be built when there is time"): the keying list and the keying slider do not go in the coming build or at all; the order of work is now assumption I3-1. Amended in 203, 204 and D31.
- I-6 -> page 2, item 241
- I-7 -> SETTLED
- I-9 -> page 2, item 242
- I-10 -> SETTLED
- I-13 -> page 2, item 268
- I-14 -> SETTLED
- I-16 -> page 2, item 243

## QUESTIONS BACK
## NAMES
@@NAME Timeline
MEANS: The entry in a control's list of signals that connects the control to the layer's playhead (the playhead of the clip playing there).
SOURCE: his words L112 for the thing; the name itself is assumption I-7 (it could read "Playhead"); the on-screen name now, "Clip Position", is replaced by it. The same word is his word for how a clip plays ("timeline mode", L21): one decision in the list of names.
@@END

@@NAME Envelope
MEANS: A shape he draws that moves a slider or switches a button; any number of them, one per slider or shared by many.
SOURCE: his words L104, L112
@@END

@@NAME Looping / One Shot
MEANS: The choice, on each slider or button that uses a signal, between following the signal round and round and following it once.
SOURCE: his words L106 (the on-screen names now, moved from the envelope to the user)
@@END

@@NAME Gain
MEANS: On each user of a signal: how strongly that slider or button takes the signal.
SOURCE: his words L106
@@END

@@NAME Threshold
MEANS: On a button that uses a signal: the level above which the button is on (or off when inverted).
SOURCE: his words L106
@@END

@@NAME Transparency slider
MEANS: The one slider of a layer that controls how the layer is blended in (his L111: "the transparency slider to control that layers blend mode").
SOURCE: his words L111 and L34; the on-screen name now, the letter "V" of the layer strip, is replaced by it
@@END

@@NAME Blend mode
MEANS: The way a layer is mixed with the layers below it; one per layer.
SOURCE: his words L111
@@END

@@NAME Global
MEANS: The third level beside clip and layer (the tab, its effects, its macro bank); the same thing the page called composition.
SOURCE: his words L55; replaces the on-screen name "Composition"
@@END

@@NAME Macros
MEANS: The panel of 8 knobs per bank (clip, layer, global) that sliders can be plugged into.
SOURCE: his words L114; replaces the on-screen title "Dashboard"
@@END

@@NAME Master Signal
MEANS: The fader that scales every signal at once; it does not touch actions or effects that read the beat by themselves.
SOURCE: binding-decisions.md:545-566 (his "keep pulsing, this control is only for signals"); the name is the on-screen name now and stands
@@END

@@NAME Moving line
MEANS: The line on a signal that shows where the signal is at this moment.
SOURCE: his words L108 ("a moving line on each signal to know where they are"); Harmony's pick as a working name
@@END

@@NAME Signal user
MEANS: A slider or a button that has a signal plugged in; it carries its own Looping / One Shot, Gain and Falloff and, for a button, its Threshold.
SOURCE: his words L106 ("user of the signal", "Each signal user")
@@END

@@NAME Mask
MEANS: A picture used as an alpha channel: it decides where other pictures show and where they are hidden (what it cuts: assumption I3-2).
SOURCE: his words (BF260: "masks and moving masks with which are alpha channels")
@@END

@@NAME Moving mask
MEANS: A mask whose picture moves: its clip is a video or another moving picture, or it is a still picture that a signal or an action moves (assumption I3-2).
SOURCE: his words (BF260: "masks and moving masks"); what makes it move is Harmony's reading
@@END

@@NAME Keying
MEANS: The layer's list of ways to make parts of its picture see-through, with its keying slider; it stays, as a placeholder until built.
SOURCE: his words (BF260: "All the blend modes and keying stay"); replaces the row "K slider, Keying (modes and slider) -- Taken out" in /Users/boriskarpman/projects/RealTimeAudio/.harmony/NAMES.md, which no longer holds
@@END

@@NAME Envelope on the beat
MEANS: The type of envelope that runs along the beat like a signal. One made for a clip is that clip's own; the envelopes of the whole show are of this type too.
SOURCE: his words for the thing (BF261: "along the beat, which is like a signal, but personalized for that clip"); the name is Harmony's pick
@@END

@@NAME Envelope on the playhead
MEANS: The type of envelope whose drawn shape happens over the course of the playing of its clip; always a clip's own, for a clip in Timeline mode (assumption I3-5).
SOURCE: his words for the thing (BF261: "play head based"); the name is Harmony's pick
@@END

@@NAME Sync
MEANS: The slider on an effect that pulses by itself (a strobe, a pulse) with which its sync is adjusted; read as how fast it pulses against the beat, in steps (assumption I3-6, which is put to him).
SOURCE: his words (BF244: "adjust the sink", read as the sync). Two older uses of the word are kept apart from it in /Users/boriskarpman/projects/RealTimeAudio/.harmony/NAMES.md: his "sync" for the timing dial, which he replaced by Delay ("replace our sync with this", binding-decisions.md:795; the row Delay lists "sync dial" as its old name), and the signal-list entry "BPM Sync", noted there as an open name.
@@END

@@NAME Common macro
MEANS: One macro that sets the speed of all effects that pulse by themselves at once; his "maybe". Whether it waits or is built with the rest is assumption I3-7.
SOURCE: his words (BF244: "a common macro that we could set later")
@@END

@@NAME Falloff
MEANS: On each user of a signal: its smoothing. The value goes up with the signal at once and comes down slowly, in a time set from instant to 2 seconds.
SOURCE: his words L106 ("falloff (smoothing)"); what it does is item 268 of page 2, which he accepted as written
@@END

## REACHES OTHER TOPICS (from the paper)
- Topic A, @@ITEM R214 (named under MADE FROM of item 243): its open clause on effects that read the beat is closed by @@AMEND R214 1 above. BF271 ("A pause would not pause it unless it is connected to the BPM"), a box of topics A / K, was used here only as support for item 243.
- Topic K, old item R203 (c) (the autopilot's count per kind of layer, which hung on I-1): BF260 takes nothing out, so the count per kind stays; K's paper should say so (I3-3). If I3-2 holds, "mask" becomes a kind of layer he can pick, and K's count per kind then has a visible meaning.
- Topic F (the show file): a show keeps each layer's keying choice and keying slider value (BF260 reverses the removal); it will hold clip-owned envelopes of two types (BF261) and, later, mask layers. A copy of a clip carries its own envelopes.
- Topics B (old assumption B-3), C (C-13) and H (R194 f) lean on "every blend mode follows the transparency slider" from L111: the sentence stands, but it is built at the last stage, not in the coming build (I3-1). Until then they must not count on it.
- Topic C (presets): Sync is a control of the effect and is held by a preset. Whether a preset carries a clip-owned envelope's shape is still not settled (it was open before; BF261 makes it sharper).
- Topics D and E (actions, recordings, Studio): Sync is recorded and can be moved by an action like any control; an envelope's type and owner are part of what a recording must find again.
- Topic B (previews): BF247 says a preview starts at once, not on the beat. What a One Shot (item 242: "from the next 1") or an envelope on the beat does inside a preview is B's to rule; this paper rules the output only.
- Topic J (the mapping): Sync can be put on a key, pad or MIDI knob; on a knob it steps through its list.
- NAMES.md (kept by Harmony): the row that says the keying is taken out must be replaced; new rows Mask, Moving mask, Sync, the two envelope types.

## CONFLICTS (from the paper)
- The keying. NEW (BF260, 2026-10-09): "All the blend modes and keying stay, and they will be built when there is time. Right now they're just sitting there as placeholder." AGAINST his round-1 answer (binding-decisions.md:1185): "204 I want to remove the keying and slider. We are only going to use the transparency slider to control that layers blend mode." and his earlier "Does the keying slider actually do anything? Maybe we get rid of it." (binding-decisions.md:654-655). The newest wins: the keying and its slider stay. To say to him in one line: "On 7 October you wrote to remove the keying and its slider; on 9 October you wrote that the keying stays and is built later. I follow the later one."
- Mask layers. NEW (BF260): "We will also add masks and moving masks with which are alpha channels" AGAINST the page's own text, which is Harmony's and not his ("There are no mask layers", item 240) and the best reading in the old 204 (Harmony's: "a layer has no kind"). His words win; no earlier word of his is overturned.
- When. NEW (BF260): "they will be built when there is time" BESIDE (binding-decisions.md:1184): "203 c this is built after all other parts are done. This is last, before the ui redesign which is the final change". Not against each other; read as the same time (I3-1, a line he can strike).
- A drawn shape along the clip. NEW (BF261): "another one is play head based so we can draw a envelope that happens over the course of the playing of that clip in timeline mode" AGAINST the page's text (Harmony's: "always runs on the beat, never along the clip's playhead"). It agrees with his earlier "build." (binding-decisions.md:258, said to the drawable Timeline curve): the old open conflict there is closed in favour of both.
- One macro for all. NEW (BF244, a "maybe"): "a single macro controls the speed of all of these type of effects" BESIDE his earlier "all. Global, layer, clip." (binding-decisions.md:263, macro banks at all three levels, each slider on its own level's bank). One knob for effects on every level would cross the banks: nothing is built for it until he asks (I3-7).

## NOT DONE / UNSURE (from the paper)
- What a mask is (I3-2) and what "adjust the sync" means (I3-6): his words do not settle either. Cheapest: the two questions, one line each, on his next page.
- Which effects read the beat by themselves, and whether any already has a control for its rate: not looked up (only a count of lines in src/render/EmbeddedShaders.h). Cheapest: read each such effect's program text when the part is planned (I3-9).
- What the keying slider and each keying and blend entry will do when built: not ruled; put to him when that stage is planned (I-5).
- Whether the bottom layer under a blend mode other than Normal should show plainly (I3-3 way b): left as a note for that later stage.
- Whether a preset carries a clip-owned envelope: topic C; one line on a later page.
- How Resolume itself treats a layer in mask mode was not looked up (no web tools here, and his Resolume is never touched); if Harmony wants I3-2 to match it, that is a Researcher's task.
- TODAY lines are taken from the old blocks and the fact sheets of 2026-10-05, plus src/model/Layer.h:199 read by me; nothing was run.
- Written 2026-10-09 18:54:15 EDT by the architect of topic I (read, not run; nothing built).

## FOR THE PAGE RULING (from the ruling)
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

