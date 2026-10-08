# SPEC I -- Effects, signals and what moves a slider by itself (s-rta-1007): the paper apply-I.md with the ruling rule-I.md laid over it by merge.py. This file wins over both.

## ITEMS
@@ITEM 203
TITLE: Blend and transition lists are tidied last
STATUS: ANSWERED c, plus his words on when
HIS: L110, L111
RULE: Option C: the blend list and the transition list are left as they are for the coming builds. His words: "203 c this is built after all other parts are done. This is last, before the ui redesign which is the final change". So the tidy-up of these lists is the last function work: after every other part is built and working, and before the UI redesign. Two things are read as NOT waiting, because L111 states them as how a layer works and not as a tidy-up of a list: the keying entries and the keying slider go, and every blend mode follows the transparency slider (see 204; the order of work is assumption I-3, a line he can strike). The fade slider and its list of transitions stay. What is kept, mended and removed in the two lists is put to him again when that last part is planned (I-5); the page's proposal for the keying list (3 stay, 2 mended, the keying slider connected to Luma Key) is void because the keying goes (L111).
CHANGED: Default A (the proposal of the page of 2026-10-04) is not taken; C is. The keying third of the question is overtaken by L111.
TODAY: area-cue-layers.md items 15, 22, H14, A1: the strip's V drop-down holds 13 keying + 55 blend entries, the F drop-down 55 transition names; as a blend only Normal, Additive, Screen, Multiply, Darken, Lighten exist, and the last four ignore the layer's opacity; 15 real transitions, 40 fall to Dissolve. Nothing changes here until the last stage, except the keying removal and the four modes obeying opacity (I-3).
@@END

@@ITEM 204
TITLE: Keying and its slider go; transparency slider blends the layer
STATUS: ANSWERED in his words (no letter); the layer's kind is not named by them (I-1)
HIS: L111, L51, L110
RULE: His words: "204 I want to remove the keying and slider. We are only going to use the transparency slider to control that layers blend mode." (1) The keying list (all its entries) and the keying slider are removed from the layer strip and from the Layer tab; a layer has no keying setting and a show carries none. "the keying and slider" is read as the keying list and the keying slider, from his earlier words "Does the keying slider actually do anything? Maybe we get rid of it." (binding-decisions.md:654-655); the fade slider with its list of transitions stays (L110; shown in I-3). A picture's own see-through parts (a logo) stay see-through on every layer, with no setting for it (I-4). (2) A layer has one blend mode and one transparency slider. The transparency slider sets how strongly the layer is laid over the layers below with that blend mode: fully down, the layer adds nothing to what is below it (under the bottom layer there is only black); fully up, it is laid over at full strength; every blend mode follows the slider (reading of "control that layers blend mode": I-2). This is built together with the removal of the keying, not at the last stage (I-3). (3) The layer's kind, which the question asked about, is not named by his words, and they take none of the three letters. Best reading, Harmony's and not his word: a layer has no kind and every layer, the bottom one too, is blended in the same way, because he writes "We are only going to use the transparency slider" and "that layers blend mode" (the layer the question was about). It is asked as I-1; until he answers, nothing is built for a kind and nothing is taken out for it (mask layers, effects-only layers, the autopilot's count per kind of layer). (4) An effects-only cell works on any layer and acts on the layers below it; on the bottom layer it has nothing to act on (his L51: "a cell with effect(s) only can be used similar to a layer effect. They will affect the layers below it. If there is an effect at the bottom layer, it will not be effective"). (5) The effects named Chroma Key and Key Palette in the effects list are effects, not the layer's keying, and stay (I-4).
CHANGED: None of A / B / C is taken as written. Added by him: the keying list and the keying slider are removed; the transparency slider is the one control of the layer's blend. The question's own subject (the hidden kind) is not named by his words: it is asked (I-1), with "no kind" as the best reading and the page's default A as the other way.
TODAY: area-cue-layers.md items 14, 15, 18, 20, 22, H13, M4, T10: K slider writes keyThreshold (a plain float); keying runs only on Transparent layers; an Opaque layer (layer 1) ignores blend and keying and is black at fader 0; Screen, Multiply, Darken, Lighten ignore the layer's opacity; Layer::Type is read by the compositor, the Layer tab and the Per-Type Autopilot (Autopilot.cpp:148-162). Read by me: src/model/Layer.h:193-201 (five kinds: Opaque = the default, Transparent, FXOnly, ThreeD, Mask) and :255-261 (thirteen keying modes; Alpha is the default: what it does for a picture's own transparency must remain as fixed behaviour when keyingMode goes). To change: remove keyingMode / keyThreshold / keySoftness UI and render path; make every blend mode obey opacity; the fate of Layer::Type waits for I-1. EffectLibrary.cpp:531, 749 (Chroma Key, Key Palette effects exist).
@@END

@@ITEM 205
TITLE: "Timeline" is the link to the layer's playhead
STATUS: ANSWERED in his words (no letter); two parts open (I-6, I-7)
HIS: L112, L104, L106
RULE: His words: "205 these terms are confusing. Timeline is connecting anything that can be connected to the layers playhead and should be called as such. The envelope is the envelope and I have already described this above." (1) The connection of a control to the layer's playhead is one thing with one entry: wherever a slider or a button can be connected to the playhead, its list of signals has ONE entry for it, and the second entry for the same connection ("Clip Position") goes. The name on that entry is read as "Timeline", his word for it here and earlier ("when I set any clip parameter to timeline it should be locked to the clips playhead", binding-decisions.md:591). "should be called as such" can also mean that the link is to be named for what it is, the playhead: both ways are in I-7. The same word is also his word for how a clip plays ("timeline mode", L21): which of the two carries the name in the list of names is for the ruling over all topics. (2) What it does: the control follows the playhead of the clip. A clip's control follows that clip's own playhead; a layer's control follows the playhead of the clip playing on that layer; it is not offered in the Global tab (R220 a, b; his earlier words). The value runs from the control's low end at the clip's start to its high end at the clip's end (the In and Out points, by his earlier "It should be as if that is the extent of the timeline", binding-decisions.md:729; I-8), goes wherever the playhead goes (backwards in reverse, jumps with Random and Beat Repeat, follows a drag of the playhead) and holds when the clip is paused. (3) A control on Timeline is a user of a signal (L106): it has its own Gain, Falloff, Range and Invert, and a button has its Threshold. Looping / One Shot is not shown there: the playhead decides (I-11). (4) A shape he draws is an ENVELOPE, never "a Timeline curve": "The envelope is the envelope". Envelopes are as L104 says: any number, one per slider or shared. Best reading: Timeline itself is a straight link with no drawn shape, and an envelope runs on the beat. I-6 asks whether an envelope can also be connected to the playhead, which was the drawn Timeline curve of his earlier "build." (binding-decisions.md:258).
CHANGED: Neither A nor B as written. Replaced: A's "Timeline is the curve you draw" (a drawn shape is an envelope). In effect B's one entry. Added: "anything that can be connected" = sliders and buttons (L106). The entry's NAME is an assumption (I-7), not his ruling.
TODAY: area-effects-signals.md 1.12, 1.16, E7, T8, O11: the picker lists both "Clip Position" and "Timeline"; "Timeline" is a 4-beat ramp on the beat clock, "Clip Position" reads 0 (clip clock nullptr, ConnectionEngine.cpp:327-372); no editor draws a Timeline curve. To change: wire the clip clock; one entry; drop the beat ramp meaning. E14: "Timeline" also names a clip's transport mode (ClipInspector.cpp:9), the clip's bar and the review screen's rows: the list of names must separate them.
@@END

@@ITEM R195
TITLE: Signals and effects: what stays, with four parts changed
STATUS: CORRECTED d widened and g replaced by him; c and e changed by his words elsewhere
HIS: L104, L106, L66
RULE: (a) stands: a signal is plugged into a slider from the small triangle on its row; taking it off is "Manual". By L106 a button is plugged in and taken off in the same way (see R221; where its control sits: L8). (b) stands: the Master Signal fader scales every signal at once and does not touch actions; the ignore lamp is for actions only. (c) After he lets go of a slider: a slider that a signal drives glides back to the signal ("smooth transition back."), in a short time of its own, as it stood on the page (I-18); a slider that an action moves ALSO glides back, to the action's position, in the time of the Global glide setting (L66: "the slider moves back to the action position based on the global glide back setting I described earlier"). (d) His words: "we can have infinite envelopes for different sliders. We can have one per slider or have many sliders share the same one." There is no limit on the number of envelopes: a new envelope can be made for one slider, and one envelope can be used by many sliders; a change to a shared envelope shows on every slider that uses it. Each envelope has its own shape and length and is saved with the show. The rest of (d) stands as it was shown to him: the two signals of his own, Mod 1 and Mod 2, stay, and new signals can be made "only for envelopes" (the oscillators offered in a slider's list stay as they are). How a new envelope is made and named: I-17. (e) A control that uses a signal keeps Range and Invert and gets its own Gain and Falloff as well (L106: "Each signal user should also have it’s own gain and falloff (smoothing)"); the full rule is under R221. (f) stands: sliders read 0 to 1, without units. (g) His words: "G we want to re-order effects". Effects in one stack (a clip's, a layer's, the global one) are re-ordered by dragging an effect's header bar to a new place, as the page offered ("re-order effects by dragging") and as in Resolume; the picture follows the new order at once; whatever points at an effect (an action, a recording's row, a key or pad of the mapping, a signal on its sliders) stays with that effect when it moves (I-19). The rest of (g) stands: a new effect is added at the end, and single effects are not copied or pasted.
CHANGED: d ADDED TO, not replaced (page: two signals of his own, Mod 1 and Mod 2; "more can be made only for envelopes, when that part is built"): new is "infinite", "one per slider" and "many sliders share the same one"; Mod 1, Mod 2 and "only for envelopes" stand. g REPLACED in its first half (page: effects cannot be re-ordered); the rest of g stands. c REPLACED in its second half by L66 (page: a slider that an action moves jumps, 158 A); its first half stands. e REPLACED in "no smoothing control" by L106. a, b, f stand.
TODAY: area-effects-signals.md 1.07 (no re-order, effects appended at the end), 1.13 (Mod 1 = a sine oscillator, Mod 2 = an envelope; the user cannot make another), 1.15 (nothing of a signal's definition is saved with the show), 1.19 (rank chain), 2.17 (the glide back to a signal is built; its time not re-read), X1 (ConnShape has smoothingMs, curve, inMin/inMax saved, no UI), T1 (effects addressed by POSITION in shows, takes, actions, bindings: a re-order needs a stable id). All of these must change except the glide back to a signal.
@@END

@@ITEM R210
TITLE: Macros in three banks, the drop switch, signals on pause
STATUS: STANDS -- the pause half of c is his own rule now (L13)
HIS: L13, L12, L31, L55, L114
RULE: (a) Macros: three kinds of bank, 8 knobs each: every clip its own 8, every layer its own 8, one global 8; a slider plugged into Macro 3 follows the third knob of its own tab's bank (the clip's, the layer's, the global one). The panel is called "Macros" (L114: "Macro’s panel should be called Macros"); the third bank is "global" (L55). (b) The switch "an oscillator restarts on a drop, or flows through it" gets a control on the row of the slider that uses the signal; this fits L106 (such choices sit on the user of the signal). (c) Signals that run on the beat (oscillators, envelopes). PAUSED: they hold where they are and go on from there when the beat plays again (L13: "Pause pauses, the beat clock, the clips and everything that it controls with BPM."; L31: "the clock will continue from where it was paused as well"). STOPPED: they stand on the "1" until the beat runs again and start from their beginning on the new "1" (the standing reading of the tempo topic, R214 a, which he did not name; it fits his L12: "When stopped, user’s click on play or any clip is the new 1."). Signals that listen to the music keep moving in both.
CHANGED: nothing replaced. The pause half of c was Harmony's and is now carried by L13. The stop half stays the page's reading (inferred consent; it stands with R214 a) and fits L12. Added from L13 and L31: after a pause they go on from where they were. The name "Macros" added (L114).
TODAY: area-effects-signals.md 1.17 (one global bank shown in all three tabs, titled "Dashboard", nothing saved), X3 (resetPhaseOnStructural exists, no control), E11 (beat-locked signals have no pause / stop behaviour because the beat has no pause or stop yet).
@@END

@@ITEM R220
TITLE: Where a slider can follow the Timeline
STATUS: STANDS -- L112 fits it; the name of the entry hangs on I-7
HIS: L112, L106
RULE: (a) The link to the playhead (the entry "Timeline": I-7) is offered on a clip's controls and on a layer's controls, not in the Global tab. (b) On a layer it follows the playhead of the clip that is playing on that layer. (c) On a clip that has no length of its own (MilkDrop, a generated picture, a still picture, the camera, an effects-only clip) the entry is greyed; every other signal still works there. L112 fits all three: "connecting anything that can be connected to the layers playhead" -- what has no playhead cannot be connected. By L106 the entry is offered on buttons as well as sliders.
CHANGED: nothing. Added from L106 / L112: buttons too.
TODAY: area-effects-signals.md 1.16, N6, T8: the picker offers "Timeline" everywhere, as a beat ramp; no greying.
@@END

@@ITEM R221
TITLE: Looping, One Shot, Gain, Falloff, Threshold move to the user
STATUS: CORRECTED a moved to the user by him, c replaced, b is his question back, d stands
HIS: L106, L107, L108, L13
RULE: (a) THE RULE OF L106. His words: "the one shot and looping controls should be on user of the signal (clip, layer or global slider and button), not the signal itself. The slider or button can use the signal. Slider is simple and button needs a threshold setting. Once the signal is above the threshold the button will be on (or off it’s inverted). Each signal user should also have it’s own gain and falloff (smoothing)." 1. A signal itself has NO Looping and NO One Shot control; the Signal tab holds only what the signal IS: its shape and its length. 2. A USER of a signal is a slider or a button in the Clip tab, the Layer tab or the Global tab (the sliders and buttons of effects included) that has a signal plugged in. A button is plugged in and taken off as a slider is (the same list of signals; "Manual" takes it off); which buttons: I-14. Each user has its own controls on its own row, and they act on that user only: two sliders on the same signal can be set differently. All of a user's settings are saved with the show. 3. Every user has: Gain (how strongly this user takes the signal; its numbers: I-12); Falloff, his word for the smoothing, read as: the value goes up with the signal at once and comes down slowly (I-13); and, standing from R195 e and R210 b (his "should also have"), Range, Invert and the restart-on-a-drop switch. 4. A slider "is simple": it follows the signal through those controls. 5. A button also has a Threshold: while the signal (after Gain and Falloff) is above the Threshold the button is on, below it the button is off; with Invert it is the other way round. 6. Looping / One Shot is read as one choice of two (his "controls"; a user either goes round or plays once) and is shown on a user of an envelope or of an oscillator (I-11); a newly plugged-in user starts as Looping (I-24). Looping: the user follows the signal round and round for as long as it is plugged in, locked to the count of bars, so every Looping user of one signal is at the same place. One Shot: the user follows the signal once, from the signal's beginning to its end, and then keeps the signal's last value until its next run. That it starts on the next "1" and keeps its last value stands from the page's text, which he read and corrected only in where the controls sit. What starts a run is not said by him and is asked (I-9): best reading, the fire of its clip (a clip's user with that clip, a layer's user with every clip fired on that layer; when the click itself is the new "1", the run starts with it), and for a Global user the switching-on of One Shot; switching One Shot on also starts one run from the next "1". That the last value is kept, and not glided back as an action is (L64), is shown to him as a line (I-10). While a One Shot runs, that user is at a place of its own in the signal. (b) Effects that read the beat by themselves (a strobe, a pulse): his L107 asks whether the page's rule is a good idea; the answer is under QUESTIONS BACK. The rule, unless he says otherwise after reading it (I-16): such an effect holds where it is while the beat is paused and goes on from there (L13: "Pause pauses, the beat clock, the clips and everything that it controls with BPM."); it stands on the "1" while the beat is stopped and starts with the new "1" (L12); it keeps pulsing with the Master Signal at 0 while the beat runs (his earlier "keep pulsing, this control is only for signals", binding-decisions.md:566); an effect that listens to the music itself keeps moving. (c) His words: "yes I want a moving line on each signal to know where they are." Every signal that has a shape (each oscillator, each envelope, each sample envelope) shows a line that moves across its shape and marks where the signal is at this moment (its Looping place); the line holds still while the beat is paused or stopped. A signal that listens to the music has no shape to move along: it shows its level at this moment (I-15). Where the line is drawn: laid out by Harmony (L8). (d) stands: loading a preset on Freeze or Echo, or switching one off and on with B, keeps the picture it holds; removing the effect clears it.
CHANGED: a REPLACED in its place only (page: the two toggles stay on the envelope in the Signal tab and are made to work there); the page's start on the next "1" and its "plays once and holds its last value" stand as text he read and did not correct. ADDED by him: a button can use a signal, with a Threshold and Invert; Gain and Falloff per user. b: he asks back (L107); answered, and the page's rule kept as a line he can strike (I-16). c REPLACED (page: no moving line unless he asks). d stands.
TODAY: area-effects-signals.md 1.14 (Looping / One Shot toggles on the envelope are inert, EnvelopeSignal.h:60-68; the editor has no mouse handling), 1.12 (a signal plugs into sliders only; no button takes a signal), 1.13 (the SignalBar shows one meter per visible signal), X1 (smoothingMs, playback, loop exist in ConnShape and are saved, no control on screen), 1.18 (Master Signal scales connections; beat-reading effects keep pulsing), W4 (a manual Resync re-aligns shapes and re-pins a one-shot: binding-decisions.md:534-543), E11 (an earlier question "Play once starts on the next bar or right away? Default: on the next bar" was never clearly answered), O7 / FENV Q2 (no moving line; the Signal tab viewport and Pitfall 57 bound how it is drawn), X5 (what a held picture does on preset load: UNKNOWN in code).
@@END

@@ITEM R196
TITLE: Bars and beats in the lists of lengths
STATUS: REPLACED in two parts (MilkDrop's lists by L126; lengths under a bar by L83, INFERRED); the rest stands
HIS: L83, L126, L99, L14
RULE: Lists of lengths read in bars from one bar up: the oscillator, the envelope, the autopilot's count, the count per kind of layer (if kinds stay: I-1). Beats stay in the circle of the tempo bar (L14: "A complete circle is one bar, and each of the four spots on the circle are one beat"), in Beat Repeat and Random (the standing reading of how Resolume counts them; his L20 "it randomly moves playhead every single beat" and L101 fit it), and in the length of a BPM-mode clip (L99: "do beats here"). A layer's fade and an output's Delay stay in seconds or milliseconds: they are times, not musical lengths. Two changes from his other lines: (1) MilkDrop's lists and MilkDrop's blend time are not touched in this build (L126: "I want you to create a dedicated document with how milk drop functions currently and that's it for this upcoming build."). (2) A length shorter than a bar is written in beats, the way he wrote the Quantize grid in L83: "4 bars, 1 bar, 2 beats, 1 beat, 1/2 beat, 1/4 beat". L83 is about that one list; carrying it to the other lists is Harmony's reading, shown to him as a line (I-20).
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
STATUS: OPEN fits L111, not said by him
HIS: L111, L110
RULE: A layer has one blend mode ("that layers blend mode", L111); wherever it can be picked (the layer strip, the Layer tab) the list is the same list with the same entries, and both places show the same choice. No keying entries are in it (L111). The two lists are made one at the last stage, together with the tidy-up of their entries (L110); until then they are left as they are, apart from the keying entries (I-3). (I-23.)
CHANGED: nothing replaced; the keying entries leave the list by L111. Added: when it is done (L110).
TODAY: area-cue-layers.md M5: the Layer tab's "Video Blend Mode" lists 25 names, the strip 55; a strip value above 24 shows blank in the Layer tab; "Alpha" appears twice in the strip's V menu.
@@END

@@ITEM P30
TITLE: The strip's drop-down lists after 203
STATUS: DROPPED
HIS: L8, L110, L111
RULE: Laid out by Harmony where it fits in the correct area; its look is settled in the UI redesign (L8). There is no Keying list to place (L111); the Blend list and the Transition list are tidied at the last stage (L110).
CHANGED: The variant with a Keying list or a "Keying" group is void (L111).
TODAY: area-cue-layers.md item 15: one V drop-down (keying + blend) and a caret-only F drop-down.
@@END

@@ITEM P31
TITLE: Where a drawn shape for the playhead is drawn
STATUS: DROPPED
HIS: L112, L8
RULE: Laid out by Harmony where it fits in the correct area; its look is settled in the UI redesign (L8). By L112 there is no "Timeline curve": a shape he draws is an envelope, and an envelope is drawn in the Signal tab's editor (his earlier words: "I want all of the controls to work in that one rather than an extra bigger envelope window.", binding-decisions.md:641-643). Whether an envelope can also be connected to the playhead is a question about what the app does and is asked under 205 (I-6); whatever he answers, the shape is drawn in that same editor.
CHANGED: Both variants were about where "the Timeline curve" is drawn; L112 takes that name away from a drawn shape, and the two variants differ only in where it sits.
TODAY: area-effects-signals.md 1.16, E7, O11: no editor draws a Timeline curve; no plan gives one a host.
@@END

## ASSUMPTIONS
@@ASSUME I-1
ABOUT: 204, R196
TEXT: I assume a layer has no "kind": every layer, the bottom one too, is blended in by its blend mode and its transparency slider. Mask layers and the autopilot's count per kind of layer go with it.
WHY: L111 takes no letter of 204 and does not name the kind; "only" and "that layers blend mode" point to no kind. A standing autopilot reading still counts per kind.
ALT: b) A layer keeps its kind and you get it as a drop-down in the Layer tab (replaces / blends / effects only / mask), as the page proposed. c) Every layer blends the same way, but mask layers and the autopilot's count per kind stay.
IF-WRONG: REBUILD taking out mask layers and the count per kind, or keeping them, changes how the picture is put together, the Layer tab and the show file.
ASK: YES no word of his names the kind; it decides what a layer can be and what is taken out.
@@END

@@ASSUME I-2
ABOUT: 204, D31
TEXT: I assume each layer keeps its list of blend modes, and the transparency slider sets how strongly the layer is blended in with the chosen mode: fully down, the layer adds nothing, whatever the mode.
WHY: L111 "that layers blend mode" takes the mode as given and L110 leaves its list for later; read loosely, "We are only going to use the transparency slider" could mean the list goes.
ALT: b) There is no blend list at all: a layer is simply faded in over the layers below by its transparency slider.
IF-WRONG: SMALL a list he did not want stays until he says so; nothing is lost on stage.
ASK: LINE his own words (L111 with L110) carry the reading; he can strike it.
@@END

@@ASSUME I-3
ABOUT: 203, 204, D31
TEXT: I assume the keying list and the keying slider go in the coming build, and every blend mode follows the transparency slider from then on. The fade slider stays. Only tidying the blend and transition lists waits until last.
WHY: L110 leaves the lists until last; L111 removes the keying, which sits in one of those lists, and gives no time. "the keying and slider" is read as the keying slider.
ALT: b) The keying goes at the last stage too, with the lists.
IF-WRONG: SMALL the same work, earlier or later.
ASK: LINE the order of work between two lines of his; nothing on stage hangs on it.
@@END

@@ASSUME I-4
ABOUT: 204
TEXT: I assume the effects called Chroma Key and Key Palette stay in the effects list, and a picture's own see-through parts (a logo) stay see-through on every layer: only the layer's keying list and its slider go.
WHY: L111 "remove the keying and slider" is said of the layer's keying; it names no effect and not a picture's own transparency.
ALT: b) Everything that keys goes, these two effects as well.
IF-WRONG: SMALL two effects removed or kept; a logo that lost its see-through parts would be a fault, not a choice.
ASK: NO nothing he asked for is taken away; a guard for the build.
@@END

@@ASSUME I-5
ABOUT: 203
TEXT: I assume that when the last stage comes I show you the blend and transition lists again and you say what stays; until then every entry stays as it is.
WHY: L110 picks "c" (wait) and does not say what is done to the lists when their turn comes.
ALT: b) At the last stage the earlier proposal is built without asking again (blend: 2 stay, 4 mended, 49 go; transitions: 15 stay, 40 go).
IF-WRONG: SMALL it is asked again before anything is built.
ASK: NO it only promises a later question.
@@END

@@ASSUME I-6
ABOUT: 205
TEXT: I assume a slider on Timeline moves straight from its low end to its high end as the clip plays through. A shape you draw is an envelope, and an envelope always runs on the beat, never along the clip.
WHY: L112 parts Timeline from the envelope; earlier he said "build." to a drawn Timeline curve. "anything that can be connected" could take in an envelope. Not said.
ALT: b) An envelope can also be connected to the playhead: the drawn shape is then stretched over the clip from start to end.
IF-WRONG: REBUILD a drawn shape that follows the clip is a tool of its own; without it a slider can only go straight along a clip.
ASK: YES his L112 against his earlier "build."; it decides what the tool can do.
@@END

@@ASSUME I-7
ABOUT: 205, R220
TEXT: I assume the link to the playhead reads "Timeline" in a control's list of signals, and the entry "Clip Position" goes. Say so if it should read "Playhead": "Timeline mode" is also your word for how a clip plays.
WHY: L112 "should be called as such" reads two ways: keep the name Timeline, or name the link for what it is. It does not name Clip Position. L21 uses "timeline mode" for a clip.
ALT: b) The entry reads "Playhead" (or "Layer Playhead"), and "Timeline" is left to the clip's mode. c) Both entries stay.
IF-WRONG: SMALL one list entry and its name.
ASK: LINE a name: he can strike it.
@@END

@@ASSUME I-8
ABOUT: 205, R220
TEXT: I assume "start" and "end" for Timeline are the clip's In and Out points, and during a fade between two clips a layer's slider follows the clip that is coming in.
WHY: L112 names the layer's playhead only; the two ends and the moment of a fade are not said.
ALT: b) Start and end are the whole file. c) During a fade the layer's slider stays with the clip that is leaving.
IF-WRONG: SMALL a constant to change.
ASK: NO internal detail.
@@END

@@ASSUME I-9
ABOUT: R221
TEXT: I assume a One Shot runs once from the next 1 after its clip is fired: a clip's slider with that clip, a layer's slider with every clip fired on that layer, a Global slider when you switch it on.
WHY: L106 moves One Shot to the slider or button and does not say what starts its one run. The page's start on the next 1 stands; it named no start for a slider.
ALT: b) It starts the moment its clip starts, off the beat too. c) It runs only when you press a start button on that slider. d) A layer's slider runs once only, not again with every clip.
IF-WRONG: STAGE a move that comes at the wrong moment, or never again, is seen.
ASK: YES no word of his says what starts a One Shot on a slider.
@@END

@@ASSUME I-10
ABOUT: R221
TEXT: I assume that when a One Shot has run through, the slider stays on the signal's last value until its next run, as the last page said. It does not glide back the way an action does.
WHY: The page's "plays once and holds its last value" was read by him and not corrected (L106 moves only the controls). For actions L64 rules the opposite.
ALT: b) As with actions: the slider glides back to where you had it by hand, in the Global glide time.
IF-WRONG: STAGE the slider rests in a different place after every One Shot; one rule to change.
ASK: LINE the page's own text stands; shown because his rule for actions (L64) points the other way.
@@END

@@ASSUME I-11
ABOUT: R221, 205
TEXT: I assume Looping / One Shot is shown on a slider or button that uses an envelope or an oscillator. On Timeline the playhead decides, and a music signal has no beginning or end: there the choice is not shown.
WHY: L106 puts the pair on the user of the signal; it does not say for which signals. Timeline and the music signals have nothing to play once.
ALT: b) On a music signal One Shot means: react to the next hit only, then stop. c) Only users of an envelope get the pair.
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

@@ASSUME I-13
ABOUT: R221
TEXT: I assume Falloff means: the value jumps up with the signal at once and comes down slowly, from instant to 2 seconds. It is the only smoothing control.
WHY: L106 says "falloff (smoothing)": the word says the way down; whether the way up is slowed too, and the longest time, are not said.
ALT: b) It smooths the way up and the way down alike. c) Two controls: rise and fall.
IF-WRONG: SMALL seen as a lazy or a sharp pulse; one formula to change.
ASK: LINE he can strike it.
@@END

@@ASSUME I-14
ABOUT: R221
TEXT: I assume every on/off button in the Clip, Layer and Global tabs can use a signal, an effect's on/off too. A button that is only pressed (remove, load, save) cannot.
WHY: L106 says "slider and button" and describes a button that is on or off; which buttons, and buttons with no on / off state, are not said.
ALT: b) A button that is only pressed can use one too: it is pressed once each time the signal rises past the Threshold. c) Controls with more than two positions can use one too.
IF-WRONG: SMALL the set of buttons can be widened later.
ASK: LINE he can strike it.
@@END

@@ASSUME I-15
ABOUT: R221
TEXT: I assume the moving line is drawn on every signal that has a shape (oscillators, envelopes, sample envelopes) and shows where its loop is now. A signal that listens to the music has no shape: it shows its level.
WHY: L108 says "a moving line on each signal to know where they are"; a music signal has no place in a shape, and a One Shot (L106) runs at a place of its own.
ALT: b) Music signals get nothing extra. c) The line shows the place of the slider you are working on, a One Shot's run too.
IF-WRONG: SMALL what one line means.
ASK: LINE he can strike it.
@@END

@@ASSUME I-16
ABOUT: R221
TEXT: I assume, as my answer to your question recommends, that effects which read the beat by themselves hold still while the beat is paused or stopped, and keep pulsing when the Master Signal is at 0.
WHY: L107 asks "Is this a good idea" and does not rule. Both halves are his own earlier rules: L13 for the pause, "keep pulsing, this control is only for signals" for the fader.
ALT: b) They keep pulsing through a pause. c) The Master Signal at 0 calms them too.
IF-WRONG: STAGE a strobe that flashes or stands still at the wrong time; one switch per effect to change.
ASK: LINE his own words carry both halves; he reads the answer and can strike this.
@@END

@@ASSUME I-17
ABOUT: R195
TEXT: I assume a new envelope is made from a slider's list of signals ("New Envelope") or in the Signal tab, named Envelope 1, 2, 3 until you rename it. Deleting one sets the sliders that used it back to Manual.
WHY: L104 says "infinite envelopes", one per slider or shared; how one is made, named and deleted is not said. Where the entry sits is his L8.
ALT: b) New envelopes are made in the Signal tab only.
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

## QUESTIONS BACK
@@ANSWER R221-b
HIS: L107, L13
ANSWER: Yes, it is a good idea. The logic: such an effect listens to the beat clock, not to the signals. So what holds the clock holds the effect: on pause it stands where it was and goes on in step when you press play; on stop it waits and starts on your new 1. That is your own rule: pause pauses everything the beat controls. A strobe that kept flashing through a pause would be out of step. What only turns the signals down does not touch it: with the Master Signal at 0 the sliders stand still and the strobe keeps its beat; to stop it, switch the effect off. One thing to know: a strobe paused in its dark moment stays dark until you press play.
@@END

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

@@NAME Falloff
MEANS: On each user of a signal: its smoothing (his word); read as how slowly the value comes down after the signal drops (assumption I-13).
SOURCE: his words L106 ("falloff (smoothing)")
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

## CONFLICTS (from the paper, unruled)
- L110 "203 c this is built after all other parts are done. This is last" (the lists, the keying list among them, are left alone until last) against L111 "204 I want to remove the keying and slider." (the keying goes). Read as: the keying goes in the coming build, the other lists wait (I-3). Asked as a LINE only, not as a question: a wrong guess changes the order of work, nothing on stage.
- binding-decisions.md:258 "build." (said to the drawable Timeline curve) against L112 "Timeline is connecting anything that can be connected to the layers playhead" and "The envelope is the envelope": whether a drawn shape can still run along the clip (I-6, ASK YES).
- L111 read as "no kind of layer" against the standing reading in topic K (slice-K.md R203 c, not named by him: "the count per kind of layer" stays; that text is Harmony's, not his): if kinds go, that count has nothing to count (I-1, ASK YES).
- L64 "they will only play once which means they will go back to the position they were at before they played" (actions) against the page's own line for a one-shot signal, which he did not correct ("plays once and holds its last value"; Harmony's text): I-10, ASK YES.
- binding-decisions.md:646 "Quantize 1 bar, 1/2 bar, 1/4, 1/8, 1/6" against L83 "4 bars, 1 bar, 2 beats, 1 beat, 1/2 beat, 1/4 beat": how a length under a bar is written (I-20, LINE: wording only).
- The earlier default 158 A (a slider that an action moves jumps, kept in R195 c) against L66 "the slider moves back to the action position based on the global glide back setting I described earlier": L66 is newer and wins; no question.

## NOT DONE / UNSURE (from the paper, unruled)
- Whether a preset carries the envelope of a slider (its shape), now that envelopes can be made per slider (L104): belongs to topic C (presets); not settled here. Cheapest: one line on his next page under presets.
- What a recording and an action hold of a control's new settings (Gain, Falloff, Threshold, Looping / One Shot): his L76 ("the action holds all parameters that are changed from it’s defaults, including menu changes and backwards forwards changes") points to yes; belongs to topics D and E.
- The second question hidden in 204 (I-1) reaches topic K's autopilot reading and topic F's show file; those papers should carry the same answer once he gives it.
- "every clip has left" in the answer on a stop rests on topic A's standing reading (slice-A.md R176 a), not on a line of this message; if topic A's paper changes it, the STOPPED sentence of the answer changes with it.
- What an effect that "reads the beat by itself" shows at the "1" (a strobe: bright or dark) was not looked up per effect. Cheapest: read the shader of each such effect when that part is planned.
- TODAY lines come from the two fact sheets of 2026-10-05 and one grep (EffectLibrary.cpp:531, 749); nothing was run.

## FOR THE PAGE RULING (from the ruling)
- THREE QUESTIONS for him from this topic (ASK: YES): I-1 (a layer has no kind; mask layers and the autopilot's count per kind go), I-6 (can a drawn envelope follow the clip: L112 against his earlier "build."), I-9 (what starts a One Shot on a slider).
- I-1 reaches topic K (R203 c keeps "the count per kind of layer") and topic F (the show file): both follow his answer; nothing is taken out before it.
- L111 as ruled here: the keying list and the keying slider go, the fade slider stays, every blend mode follows the transparency slider. Topics B (B-3), C (C-13) and H (R194 f) lean on it. Names: "Transparency slider" (L111, L34) against his earlier "opacity" (binding-decisions.md:247): one word in the list.
- "Timeline": the paper's sentence that only one thing is called Timeline is withdrawn. I-7 (a line) asks the name of the playhead link; it collides with "Timeline mode" (his L21; J-7, H-15). One decision for the list of names.
- WHAT A SIGNAL USER KEEPS, for presets (C-11), copied clips (F), actions and recordings (L71, L76): Range, Invert, Gain, Falloff, the restart-on-a-drop switch, Looping / One Shot (on an envelope or oscillator), Threshold (on a button). A signal can now switch a button (L106).
- The effects clause of the tempo reading R214 is ruled HERE: it holds with the beat (L13) and keeps pulsing at Master Signal 0 (binding-decisions.md:566); I-16 is a line and the ANSWER R221-b argues it. Topic A's "not settled here" follows this.
- The Global glide slider (D-1) times actions only; the glide back to a SIGNAL keeps its own short time (I-18, NO). D-1's "your hand letting go" must not be read as covering it.
- Re-ordering effects (L104 g) gives every effect a lasting identity (I-19): actions, recordings and the mapping (topics D, E, J) point at the effect, not at its place.
- Lists of lengths: bars from one bar up, beats below (I-20, a line), to match the Quantize grid (L83, topic E) and the autopilot's counts (topic K, R213 c).
- The paper's CONFLICTS and NOT DONE are copied unruled: its bullet on I-10 ("ASK YES") is a line now, as is I-2, and its NOT DONE note on "every clip has left" is overtaken (the answer no longer says it). Lines for him: I-2, I-3, I-7, I-10, I-13, I-14, I-15, I-16, I-20, I-21, I-22; all else NO.

