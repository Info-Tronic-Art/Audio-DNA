# APPLY I -- Effects, signals and what moves a slider by itself (s-rta-1007)
Written: 2026-10-07 22:48:33 (read-only run; nothing built, run, launched or committed). Labels: his words are quoted verbatim with their L number; everything else in a RULE is Harmony's reading, and every gap is an @@ASSUME.

## SUMMARY
- L106 moves Looping / One Shot off the signal and onto every USER of a signal (a slider or a button in the Clip, Layer or Global tab); every user also gets its own Gain and Falloff; a button can use a signal through a Threshold (with Invert).
- L104: any number of envelopes, one per slider or shared by many; effects can be re-ordered.
- L108: a moving line on each signal.
- L111: the keying list and the keying slider go; a layer is blended by its blend mode and the transparency slider alone. What his words leave open: whether a layer still has a "kind" (I-1) and whether the blend list itself stays (I-2).
- L112: "Timeline" is the one name for connecting a control to the layer's playhead; a drawn shape is an envelope. Open: whether a drawn shape can still run along the clip (I-6; his earlier "build.").
- L110: the blend and transition lists are tidied last, just before the UI redesign.
- L107 is a question back to Harmony (beat-reading effects on pause / stop / Master Signal 0): answered under QUESTIONS BACK; recommendation = yes to both halves; L13 already carries the pause half.
- Carried in from other lines (L9): L66 makes the hand-back after an action a glide (replaces the "jumps" in R195 c); L83 writes lengths under a bar in beats (touches R196); L126 takes MilkDrop's lists out of this build (touches R196).

## ITEMS
@@ITEM 203
TITLE: Blend and transition lists are tidied last
STATUS: ANSWERED c, plus his words on when
HIS: L110, L111
RULE: Option C: the blend list and the transition list are left alone for the coming builds. His words: "203 c this is built after all other parts are done. This is last, before the ui redesign which is the final change". So the tidy-up of these lists is the last function work, done after every other part is built and working, and before the UI redesign. The one part that does not wait is what L111 removes: the keying entries and the keying slider (see 204; timing: I-3). What exactly is kept, mended and removed in the two lists is put to him again when that last part is planned (I-5); the page's proposal for the keying list (3 stay, 2 mended, K connected to Luma Key) is void because the keying goes (L111).
CHANGED: Default A (the proposal of the page of 2026-10-04) is not taken; C is. The keying third of the question is overtaken by L111.
TODAY: area-cue-layers.md items 15, 22, H14, A1: the strip's V drop-down holds 13 keying + 55 blend entries, the F drop-down 55 transition names; as a blend only Normal, Additive, Screen, Multiply, Darken, Lighten exist; 15 real transitions, 40 fall to Dissolve. Nothing changes here until the last stage, except the keying removal.
@@END
@@ITEM 204
TITLE: Keying and its slider go; transparency slider blends the layer
STATUS: ANSWERED in his words (no letter); two parts open (I-1, I-2)
HIS: L111
RULE: His words: "204 I want to remove the keying and slider. We are only going to use the transparency slider to control that layers blend mode." (1) The keying list (all its entries) and the keying slider are removed from the layer strip and from the Layer tab; a layer has no keying setting; a show carries none. (2) A layer has one blend mode and one transparency slider. The transparency slider alone sets how strongly the layer is laid over the layers below with that blend mode: at 0 the layer adds nothing to the picture (it never turns the picture black), at full it is blended at full strength, and every blend mode follows the slider (reading of "control that layers blend mode": I-2). (3) No "kind" drop-down is built (he did not take A). Best reading of his words: every layer, the bottom one too, blends in the same way (I-1). (4) An effects-only cell works on any layer and acts on the layers below it; on the bottom layer it has nothing to act on (his L51: "a cell with effect(s) only can be used similar to a layer effect. They will affect the layers below it. If there is an effect at the bottom layer, it will not be effective"). (5) The effects named Chroma Key and Key Palette in the effects list are effects, not the layer's keying, and stay (I-4).
CHANGED: None of A / B / C is taken as written. Added by him: keying list and keying slider removed; the transparency slider is the only control of the layer's blend. The question's own subject (the hidden kind) is not named by his words: read as B (no kind), held as assumption I-1.
TODAY: area-cue-layers.md items 14, 15, 18, 20, 22, H13, M4, T10: K slider writes keyThreshold (a plain float); keying runs only on Transparent layers; an Opaque layer (layer 1) ignores blend and keying and is black at fader 0; Screen, Multiply, Darken, Lighten ignore the layer's opacity; Layer::Type is read by the compositor, the Layer tab and the Per-Type Autopilot (Autopilot.cpp:148-162). To change: remove keyingMode / keyThreshold UI and render path; make every blend mode obey opacity; decide the fate of Layer::Type (I-1). EffectLibrary.cpp:531, 749 (Chroma Key, Key Palette effects exist).
@@END
@@ITEM 205
TITLE: "Timeline" is the link to the layer's playhead
STATUS: ANSWERED in his words (no letter); one part open (I-6)
HIS: L112, L104, L106
RULE: His words: "205 these terms are confusing. Timeline is connecting anything that can be connected to the layers playhead and should be called as such. The envelope is the envelope and I have already described this above." (1) There is ONE thing called "Timeline": the connection of a control to the layer's playhead. Wherever a slider or a button can be connected to the playhead, the entry in its list of signals reads "Timeline", and nothing else carries that meaning under another name; the entry "Clip Position" goes, because it is the same connection (I-7). (2) What it does: the control follows the playhead of the clip. A clip's control follows that clip's own playhead; a layer's control follows the playhead of the clip playing on that layer; it is not offered in the Global tab (R220 a, b, his earlier words). The value runs from the control's low end at the clip's start to its high end at the clip's end (I-8), goes wherever the playhead goes (backwards in reverse, jumps with Random and Beat Repeat, follows a drag of the playhead) and holds when the clip is paused. (3) Like every user of a signal (L106) a control on Timeline has its own Gain, Falloff, Range and Invert, and a button has its Threshold. (4) A shape he draws is an ENVELOPE, never "a Timeline curve": "The envelope is the envelope". Envelopes are as L104 says: any number, one per slider or shared. Best reading: Timeline itself is a straight link with no drawn shape, and an envelope runs on the beat (I-6: whether an envelope can also be made to run along the clip's playhead, which was the drawn Timeline curve of his "build.").
CHANGED: Neither A nor B as written. Replaced: A's "Timeline is the curve you draw" (a drawn shape is an envelope). In effect B's one entry, but under his rule of naming: the playhead link is called "Timeline". Added: "anything that can be connected" = sliders and buttons (L106).
TODAY: area-effects-signals.md 1.12, 1.16, E7, T8, O11: the picker lists both "Clip Position" and "Timeline"; "Timeline" is a 4-beat ramp on the beat clock, "Clip Position" reads 0 (clip clock nullptr, ConnectionEngine.cpp:327-372); no editor draws a Timeline curve. To change: wire the clip clock; one entry; drop the beat ramp meaning. E14: "Timeline" also names a clip's transport mode and the clip's bar: the names list must separate them.
@@END
@@ITEM R195
TITLE: Signals and effects: what stays, with three parts changed
STATUS: CORRECTED d and g by him; c and e replaced by his words elsewhere
HIS: L104, L106, L66
RULE: (a) stands: a signal is plugged into a slider from the small triangle on its row; taking it off is "Manual". (b) stands: the Master Signal fader scales every signal at once and does not touch actions; the ignore lamp is for actions only. (c) After he lets go of a slider: a slider that a signal drives glides back to the signal ("smooth transition back."); a slider that an action moves ALSO glides back, to the action's position, in the time of the Global glide setting (L66: "the slider moves back to the action position based on the global glide back setting I described earlier"). Which time the glide back to a signal uses: I-18. (d) His words: "we can have infinite envelopes for different sliders. We can have one per slider or have many sliders share the same one." There is no limit on the number of envelopes; a new envelope can be made for one slider, and one envelope can be used by many sliders; each envelope has its own shape and length and is saved with the show. How a new one is made and named, and whether oscillators are unlimited too: I-17. (e) A control that uses a signal has Range and Invert AND its own Gain and Falloff (smoothing), and Looping / One Shot (L106; the full rule is under R221). (f) stands: sliders read 0 to 1, without units. (g) His words: "G we want to re-order effects". Effects in a stack (clip, layer, global) are re-ordered by dragging an effect's name row to a new place, as in Resolume; the picture follows the new order at once; whatever points at an effect (an action, a recording's row, a key or pad of the mapping, a signal on its sliders) stays with that effect when it moves (I-19). Copy and paste of single effects is not offered (he named re-ordering only).
CHANGED: d REPLACED (page: two signals of his own, Mod 1 and Mod 2; more only for envelopes later). g REPLACED in its first half (page: effects cannot be re-ordered); the rest of g (no copy / paste of effects) stands. c REPLACED in its second half by L66 (page: a slider that an action moves jumps, 158 A). e REPLACED in "no smoothing control" by L106. a, b, f stand.
TODAY: area-effects-signals.md 1.07 (no re-order, effects appended at the end), 1.13 (only Mod 1 and Mod 2; the user cannot make another), 1.15 (nothing of a signal's definition is saved with the show), 1.19 (rank chain), X1 (ConnShape has smoothingMs, curve, inMin/inMax saved, no UI), T1 (effects addressed by POSITION in shows, takes, actions, bindings: a re-order needs a stable id). All four must change.
@@END
@@ITEM R210
TITLE: Macros in three banks, the drop switch, signals on pause
STATUS: STANDS -- its part c is now carried by his L13 and L12
HIS: L13, L12, L55, L114
RULE: (a) Macros: three kinds of bank, 8 knobs each: every clip its own 8, every layer its own 8, one global 8; a slider plugged into Macro 3 follows the third knob of its own tab's bank (the clip's, the layer's, the global one). The panel is called "Macros" (L114: "Macro’s panel should be called Macros"); the third bank is "global" (L55). (b) The switch "an oscillator restarts on a drop, or flows through it" gets a control on the row of the slider that uses the signal; this fits L106 (such choices sit on the user of the signal). (c) Signals that run on the beat (oscillators, envelopes) hold still while the beat is paused and stand at their beginning while it is stopped; they start from their beginning on the new "1" after a stop; signals that listen to the music keep moving. His L13: "Pause pauses, the beat clock, the clips and everything that it controls with BPM."; his L12: "When stopped, user’s click on play or any clip is the new 1."
CHANGED: nothing replaced. c was Harmony's own and is now his by L13 (pause) and L12 (the new 1 after a stop). The name "Macros" added (L114).
TODAY: area-effects-signals.md 1.17 (one global bank shown in all three tabs, titled "Dashboard", nothing saved), X3 (resetPhaseOnStructural exists, no control), E11 (beat-locked signals have no pause / stop behaviour because the beat has no pause or stop yet).
@@END
@@ITEM R220
TITLE: Where a slider can follow the Timeline
STATUS: STANDS -- L112 says the same and fixes the name
HIS: L112
RULE: (a) "Timeline" is offered on a clip's controls and on a layer's controls, not in the Global tab. (b) On a layer it follows the playhead of the clip that is playing on that layer. (c) On a clip that has no length of its own (MilkDrop, a generated picture, a still picture, the camera, an effects-only clip) the entry "Timeline" is greyed; every other signal still works there. L112 fits all three: "connecting anything that can be connected to the layers playhead" -- what has no playhead cannot be connected. By L106 the entry is offered on buttons as well as sliders.
CHANGED: nothing. Added from L106 / L112: buttons too.
TODAY: area-effects-signals.md 1.16, N6, T8: the picker offers "Timeline" everywhere, as a beat ramp; no greying.
@@END
@@ITEM R221
TITLE: Looping, One Shot, Gain, Falloff, Threshold move to the user
STATUS: CORRECTED a replaced, c replaced, b is his question back, d stands
HIS: L106, L107, L108, L13
RULE: (a) THE RULE OF L106. His words: "the one shot and looping controls should be on user of the signal (clip, layer or global slider and button), not the signal itself. The slider or button can use the signal. Slider is simple and button needs a threshold setting. Once the signal is above the threshold the button will be on (or off it’s inverted). Each signal user should also have it’s own gain and falloff (smoothing)." 1. A signal itself (an envelope, an oscillator) has NO Looping and NO One Shot control; the Signal tab holds only what the signal IS: its shape and its length. 2. A USER of a signal is any slider or any button in the Clip tab, the Layer tab or the Global tab (effects' sliders and buttons included) that has a signal plugged in. Each user has its own controls on its own row, and they act on that user only; two sliders on the same signal can be set differently. 3. Every user has: Looping / One Shot (one choice of two); Gain (how strongly this user takes the signal); Falloff, which is the smoothing (how slowly this user's value comes down); and, standing from R195 e and R210 b, Range, Invert and the restart-on-a-drop switch. 4. A slider "is simple": it follows the signal through those controls. 5. A button also has a Threshold: while the signal (after Gain and Falloff) is above the Threshold the button is on, below it the button is off; with Invert it is the other way round (off above, on below). 6. Looping: the user follows the signal round and round for as long as it is plugged in. One Shot: the user follows the signal once, from the signal's beginning to its end, and then stops following until it is started again. Each user therefore keeps its OWN place in a shared envelope (I-15). His words do not say when a One Shot starts (I-9), what the control does when it has ended (I-10), nor what the pair means for a signal with no beginning and end, such as the bass (I-11). The numbers of Gain and Falloff: I-12, I-13. Which buttons, and a button that is only pressed: I-14. (b) Effects that read the beat by themselves: his question back, answered under QUESTIONS BACK; the rule recommended there: such an effect holds still while the beat is paused (L13 already says so: "Pause pauses, the beat clock, the clips and everything that it controls with BPM"), stands on the "1" while it is stopped, and keeps pulsing with the Master Signal at 0 while the beat runs; effects that listen to the music itself keep moving. It waits for his yes (I-16). (c) His words: "yes I want a moving line on each signal to know where they are." Every signal that has a shape (each oscillator, each envelope, each sample envelope) shows a line that moves across its shape and marks where the signal is at this moment; it holds still when the beat is paused or stopped. Where it is drawn: laid out by Harmony (L8). Whose place it shows when users are at different places: I-15. (d) stands: loading a preset on Freeze or Echo, or switching one off and on with B, keeps the picture it holds; removing the effect clears it.
CHANGED: a REPLACED (page: the two toggles stay on the envelope in the Signal tab and are made to work there; "a one-shot envelope starts on the next 1, plays once and holds its last value" -- the place is replaced by his words; the start and the hold were Harmony's and are assumptions I-9, I-10). ADDED by him: a button can use a signal, with a Threshold and Invert; Gain and Falloff per user. b: not ruled, asked back. c REPLACED (page: no moving line unless he asks). d stands.
TODAY: area-effects-signals.md 1.14 (Looping / One Shot toggles on the envelope are inert, EnvelopeSignal.h:60-68; the editor has no mouse handling), 1.12 (a signal plugs into sliders only; no button takes a signal), X1 (smoothingMs, playback, loop exist in ConnShape and are saved, no control on screen), 1.18 (Master Signal scales connections; beat-reading effects keep pulsing), O7 / FENV Q2 (no moving line; the Signal tab viewport and Pitfall 57 bound how it is drawn), X5 (what a held picture does on preset load: UNKNOWN in code).
@@END
@@ITEM R196
TITLE: Bars and beats in the lists of lengths
STATUS: STANDS -- except MilkDrop's lists (L126) and how a length under a bar is written (L83)
HIS: L83, L99, L14, L101, L126
RULE: Lists of lengths read in bars: the oscillator, the envelope, the autopilot's count, the count per kind of layer (if kinds stay: I-1). Beats stay in the circle of the tempo row (L14: "A complete circle is one bar, and each of the four spots on the circle are one beat"), in Beat Repeat and Random (L101), and in the length of a BPM-mode clip (L99: "do beats here"). A layer's fade, MilkDrop's blend and an output's Delay stay in seconds or milliseconds. Two changes from his other lines: (1) MilkDrop's two lists are not touched in this build (L126: "I want you to create a dedicated document with how milk drop functions currently and that's it for this upcoming build."). (2) A length shorter than a bar is written in beats, the way he wrote the quantize grid in L83: "4 bars, 1 bar, 2 beats, 1 beat, 1/2 beat, 1/4 beat" (I-20).
CHANGED: REPLACED by L126: MilkDrop's two lists leave this reading. REPLACED by L83 (INFERRED, his newest way of writing lengths): "a length shorter than a bar reads as a part of a bar (1/2 bar)". The rest stands.
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
RULE: A sample envelope is used whole or shortened by dragging its two end handles, which snap to bar lines (his earlier words: "I want to be able to use the whole thing or to shorten it manually but clicking to timing marks." and "drag handles that snap", binding-decisions.md:641, 678). There is no bigger envelope window; every envelope control works in the Signal tab's editor ("I am fine using the envelope editor in the signal tab. I want all of the controls to work in that one rather than an extra bigger envelope window.", binding-decisions.md:641-643). Harmony's own part: the bar lines are counted from the start of the sample at the tempo of the moment (I-22). The moving line of L108 is drawn in this same editor.
CHANGED: nothing
TODAY: area-effects-signals.md E12, W2, O7, T10: no handles, no time axis; the adopted ruling-bf45 still holds a big view (stage S5) that must be re-based.
@@END
@@ITEM D31
TITLE: One blend list in the strip and the Layer tab
STATUS: OPEN fits L111, not said by him
HIS: L111, L110
RULE: A layer has one blend mode ("that layers blend mode", L111); wherever it can be picked (the layer strip, the Layer tab) the list is the same list with the same entries, and both places show the same choice. No keying entries are in it (L111). Which entries the list ends up with is decided at the last stage (L110). (I-23.)
CHANGED: nothing replaced; the keying entries leave the list by L111.
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
STATUS: OPEN hangs on what L112 means for the drawn curve (I-6)
HIS: L112, L8
RULE: By L112 there is no "Timeline curve": "Timeline" is the link to the layer's playhead, and a shape he draws is an envelope, drawn in the Signal tab's editor (his earlier words, binding-decisions.md:641-643). If he says that an envelope can also run along the clip's playhead (the other reading under I-6), it is still an envelope and still drawn in that same editor; nothing is drawn in a strip under the slider. Any remaining choice of place is Harmony's (L8).
CHANGED: Both variants were about "the Timeline curve"; L112 takes that name away from a drawn shape.
TODAY: area-effects-signals.md 1.16, E7, O11: no editor draws a Timeline curve; no plan gives one a host.
@@END

## ASSUMPTIONS
@@ASSUME I-1
ABOUT: 204, R196
TEXT: I assume a layer has no hidden "kind" any more: every layer, the bottom one too, blends the same way. Mask layers go, and the autopilot's separate counts per kind of layer go with them.
WHY: L111 speaks only of keying and the slider; question 204 was about the kind and he took no letter. A standing autopilot reading still counts per kind of layer.
ALT: b) The kind stays and becomes a drop-down you can see (replaces / blends / effects only / mask). c) The kind stays as a hidden setting.
IF-WRONG: REBUILD taking out mask layers and the per-kind autopilot counts, or keeping them, changes the compositor, the Layer tab and the show file.
ASK: YES no word of his settles it; it decides what layers can do and what is removed.
@@END
@@ASSUME I-2
ABOUT: 204, D31
TEXT: I assume each layer keeps its list of blend modes, and the transparency slider sets how strongly the layer is blended in with the chosen mode: at 0 the layer adds nothing, with every mode.
WHY: L111 "use the transparency slider to control that layers blend mode" can also mean that the slider is the only blend control and the list goes.
ALT: b) There is no blend list at all: a layer is simply faded in over the layers below by its transparency slider.
IF-WRONG: STAGE a layer set to Add or Multiply looks different from a plain fade.
ASK: YES two readings of one sentence, seen on stage.
@@END
@@ASSUME I-3
ABOUT: 203, 204
TEXT: I assume the keying list and the keying slider are taken out in the coming build, and only the tidy-up of the blend and transition lists waits until last.
WHY: L110 leaves the lists alone until last; L111 removes the keying, which sits in one of those lists.
ALT: b) The keying is removed at the last stage too, with the lists.
IF-WRONG: SMALL the same removal, earlier or later.
ASK: LINE a choice of order he can strike.
@@END
@@ASSUME I-4
ABOUT: 204
TEXT: I assume the effects called Chroma Key and Key Palette stay in the effects list: they are effects you drop on a clip or layer, not the layer's keying.
WHY: L111 "remove the keying and slider" was said about the layer strip's keying; it does not name effects.
ALT: b) Everything that keys goes, these two effects as well.
IF-WRONG: SMALL two effects removed or kept.
ASK: LINE he can strike it.
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
ABOUT: 205, P31
TEXT: I assume Timeline is a straight link: the slider runs from its low end to its high end as the clip plays from start to end. A shape you draw is always an envelope, and an envelope runs on the beat, not along the clip.
WHY: L112 separates Timeline from the envelope; earlier he said "build." to a drawn Timeline curve. Whether a drawn shape can still follow the clip is not said.
ALT: b) An envelope can also be set to run along the clip's playhead, so a drawn shape is stretched over the clip's length.
IF-WRONG: REBUILD a drawn shape that follows the clip needs its own clock and editor work; without it a slider can only ramp straight along a clip.
ASK: YES his L112 against his earlier "build."; it decides what the tool can do.
@@END
@@ASSUME I-7
ABOUT: 205
TEXT: I assume the entry "Clip Position" goes from the list of signals: it is the same link to the playhead, and that link is called Timeline.
WHY: L112 says the playhead link "should be called as such"; it does not name Clip Position.
ALT: b) Both entries stay.
IF-WRONG: SMALL one list entry.
ASK: LINE he can strike it.
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
TEXT: I assume a slider set to One Shot runs through its signal once, starting on the next 1 after you switch One Shot on; a clip's slider runs once again each time that clip is fired.
WHY: L106 moves One Shot to the user of the signal and does not say what starts the one run.
ALT: b) It starts at once, not on the 1. c) It runs once only when you press a start button on that slider. d) A layer's slider also runs again with every clip fired on that layer.
IF-WRONG: STAGE a move that fires at the wrong moment, or never again, is seen.
ASK: YES no word of his says when a One Shot starts.
@@END
@@ASSUME I-10
ABOUT: R221
TEXT: I assume that when a One Shot has run through, the slider stays on the signal's last value until the next run.
WHY: L106 does not say. For actions he ruled the opposite in L64: played once, the values go back to where they were before.
ALT: b) As with actions: the slider glides back to where you had it by hand, in the Global glide time.
IF-WRONG: STAGE the slider rests in a different place after every One Shot.
ASK: YES his rule for actions (L64) points the other way.
@@END
@@ASSUME I-11
ABOUT: R221
TEXT: I assume Looping / One Shot is shown only where the signal has a beginning and an end (an envelope, an oscillator, Timeline). A slider on the bass or another music signal has only Gain, Falloff, Range and Invert.
WHY: L106 gives the pair to every user of a signal; a music signal has no beginning or end to play once.
ALT: b) On a music signal One Shot means: react to the next hit only, then stop.
IF-WRONG: SMALL one control shown or hidden.
ASK: LINE he can strike it.
@@END
@@ASSUME I-12
ABOUT: R221, R195
TEXT: I assume Gain runs from 0 to 4 times (1 = the signal as it is), is applied before Range, and Range and Invert stay next to it.
WHY: L106 names "gain" with no numbers and does not say what becomes of Range.
ALT: b) Gain replaces Range. c) Another top value than 4.
IF-WRONG: SMALL a number and one control.
ASK: LINE he can strike it.
@@END
@@ASSUME I-13
ABOUT: R221
TEXT: I assume Falloff means: the value jumps up with the signal at once and comes down slowly, from instant to 2 seconds. It is the only smoothing control.
WHY: L106 says "falloff (smoothing)"; whether it slows only the way down, or the way up as well, is not said, nor the longest time.
ALT: b) It smooths the way up and the way down alike. c) Two controls: rise and fall.
IF-WRONG: SMALL seen as a lazy or sharp pulse; one formula to change.
ASK: LINE he can strike it.
@@END
@@ASSUME I-14
ABOUT: R221
TEXT: I assume every on/off button in the Clip, Layer and Global tabs can use a signal. A button that is only pressed, not switched, is pressed once each time the signal rises past the Threshold.
WHY: L106 says "slider and button" and describes on / off; which buttons, and buttons with no on / off state, are not said.
ALT: b) Only on/off buttons can use a signal. c) Buttons with more than two positions can use one too.
IF-WRONG: SMALL the set of buttons can be widened later.
ASK: LINE he can strike it.
@@END
@@ASSUME I-15
ABOUT: R221, R195
TEXT: I assume sliders that share one envelope each keep their own place in it (one may loop while another plays once), and the moving line shows the envelope's own looping place.
WHY: L106 puts Looping / One Shot on each user of a shared signal (L104); L108 asks for one line "on each signal".
ALT: b) A shared envelope has one place for all its sliders, so One Shot on one slider restarts it for all. c) A line per slider.
IF-WRONG: SMALL the line's meaning; the per-slider place follows from his L106.
ASK: LINE he can strike it.
@@END
@@ASSUME I-16
ABOUT: R221
TEXT: I assume, after my answer to your question, that effects which read the beat by themselves hold still while the beat is paused or stopped, and keep pulsing when the Master Signal is at 0.
WHY: L107 asks "Is this a good idea" and does not rule; L13 already covers the pause half.
ALT: b) They keep pulsing through a pause. c) The Master Signal at 0 calms them too.
IF-WRONG: STAGE a strobe that flashes or stands still at the wrong time.
ASK: YES it is his own open question; he decides after the answer.
@@END
@@ASSUME I-17
ABOUT: R195
TEXT: I assume a new envelope is made from a slider's own list of signals ("New Envelope") or in the Signal tab, is named Envelope 1, 2, 3 until you rename it, and that oscillators can be made the same way without limit.
WHY: L104 says "infinite envelopes", one per slider or shared; how one is made and named, and oscillators, are not said.
ALT: b) Only envelopes are unlimited; oscillators stay as the fixed shapes in each slider's list.
IF-WRONG: SMALL a menu entry and names.
ASK: LINE he can strike it.
@@END
@@ASSUME I-18
ABOUT: R195
TEXT: I assume a slider driven by a signal glides back to the signal, after you let go, in the time of the same Global glide slider that actions use.
WHY: "smooth transition back." gives no time; L64 and L66 give the Global glide slider for actions only.
ALT: b) The glide back to a signal has its own short fixed time, apart from the Global glide slider.
IF-WRONG: SMALL one time value.
ASK: LINE he can strike it.
@@END
@@ASSUME I-19
ABOUT: R195
TEXT: I assume you re-order effects by dragging an effect's name row, as in Resolume, and that actions, recordings, keys and pads that point at an effect stay with it when it moves.
WHY: L104 says "we want to re-order effects" and not how, nor what happens to what points at an effect.
ALT: none
IF-WRONG: SMALL the gesture; the second half is how it must work.
ASK: NO technical: an effect gets a lasting identity.
@@END
@@ASSUME I-20
ABOUT: R196
TEXT: I assume lengths shorter than a bar are written in beats, as you wrote the quantize grid: 4 bars, 1 bar, 2 beats, 1 beat, 1/2 beat, 1/4 beat.
WHY: L83 writes them in beats; earlier he wrote "1/2 bar" and "bars in most places unless beats are necessary".
ALT: b) They are written as parts of a bar: 1/2 bar, 1/4 bar.
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
TEXT: I assume the bar lines that a sample envelope's handles snap to are counted from the start of the sample, at the tempo of that moment.
WHY: His earlier "drag handles that snap" settles the snapping; where bar 1 of a sample lies is not said. L130: not read.
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
TEXT: I assume a button switched by a signal uses a small safety margin around the Threshold so that it does not flicker, and every envelope and every setting of a signal's user is saved with the show.
WHY: L106 and L104 do not go into flicker or saving; his One Save rule covers saving in spirit.
ALT: none
IF-WRONG: SMALL internal.
ASK: NO technical.
@@END

## QUESTIONS BACK
@@ANSWER R221-b
HIS: L107, L13
ANSWER: Yes, it is a good idea: keep both halves. PAUSED: these effects freeze with everything else, so you and the audience see one still picture, exactly the frame that was on screen when you pressed pause. If they kept pulsing, a strobe would flash over a frozen picture and the show would look half stopped; it would also break your own rule that pause holds everything the beat controls. One thing to know: a strobe caught in its dark moment stays dark until you press play. STOPPED: every clip has left, so there is almost nothing to see; the effects wait on the 1 and start in time with your next click. MASTER SIGNAL AT 0: sliders stop following signals, but a strobe keeps flashing in time. It is part of the look you built; to stop it, switch that effect off.
@@END

## NAMES
@@NAME Timeline
MEANS: The entry in a control's list of signals that connects the control to the layer's playhead (the playhead of the clip playing there).
SOURCE: his words L112; replaces the on-screen entry "Clip Position" and the old beat-ramp meaning of "Timeline"
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
MEANS: On each user of a signal: its smoothing, how slowly its value comes down.
SOURCE: his words L106 ("falloff (smoothing)")
@@END
@@NAME Threshold
MEANS: On a button that uses a signal: the level above which the button is on (or off when inverted).
SOURCE: his words L106
@@END
@@NAME Transparency slider
MEANS: The layer's one slider that sets how strongly the layer is blended into the picture with its blend mode.
SOURCE: his words L111 (and L34); replaces the on-screen letter "V" of the layer strip
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
MEANS: The line that moves across a signal's shape and shows where the signal is at this moment.
SOURCE: his words L108 ("a moving line on each signal"); Harmony's pick as a working name
@@END

## CONFLICTS
- L110 "203 c this is built after all other parts are done. This is last" (the lists, the keying list among them, are left alone until last) against L111 "204 I want to remove the keying and slider." (the keying goes). Read as: the keying goes in the coming build, the other lists wait (I-3). Asked as a LINE only, not as a question: a wrong guess changes the order of work, nothing on stage.
- binding-decisions.md:258 "build." (said to the drawable Timeline curve) against L112 "Timeline is connecting anything that can be connected to the layers playhead" and "The envelope is the envelope": whether a drawn shape can still run along the clip (I-6, ASK YES).
- L111 read as "no kind of layer" against the standing reading in topic K (slice-K.md R203 c, not named by him: "the count per kind of layer" stays; that text is Harmony's, not his): if kinds go, that count has nothing to count (I-1, ASK YES).
- L64 "they will only play once which means they will go back to the position they were at before they played" (actions) against the page's own line for a one-shot signal, which he did not correct ("plays once and holds its last value"; Harmony's text): I-10, ASK YES.
- binding-decisions.md:646 "Quantize 1 bar, 1/2 bar, 1/4, 1/8, 1/6" against L83 "4 bars, 1 bar, 2 beats, 1 beat, 1/2 beat, 1/4 beat": how a length under a bar is written (I-20, LINE: wording only).
- The earlier default 158 A (a slider that an action moves jumps, kept in R195 c) against L66 "the slider moves back to the action position based on the global glide back setting I described earlier": L66 is newer and wins; no question.

## NOT DONE / UNSURE
- Whether a preset carries the envelope of a slider (its shape), now that envelopes can be made per slider (L104): belongs to topic C (presets); not settled here. Cheapest: one line on his next page under presets.
- What a recording and an action hold of a control's new settings (Gain, Falloff, Threshold, Looping / One Shot): his L76 ("the action holds all parameters that are changed from it’s defaults, including menu changes and backwards forwards changes") points to yes; belongs to topics D and E.
- The second question hidden in 204 (I-1) reaches topic K's autopilot reading and topic F's show file; those papers should carry the same answer once he gives it.
- "every clip has left" in the answer on a stop rests on topic A's standing reading (slice-A.md R176 a), not on a line of this message; if topic A's paper changes it, the STOPPED sentence of the answer changes with it.
- What an effect that "reads the beat by itself" shows at the "1" (a strobe: bright or dark) was not looked up per effect. Cheapest: read the shader of each such effect when that part is planned.
- TODAY lines come from the two fact sheets of 2026-10-05 and one grep (EffectLibrary.cpp:531, 749); nothing was run.
