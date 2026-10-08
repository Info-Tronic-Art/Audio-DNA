# SLICE I -- Effects, signals and what moves a slider by itself
The items of this topic exactly as they stood on the page shown to Boris on 2026-10-05 (source: s-rta-1005/boris-all-items.json), and for each the lines of his message of 2026-10-07 that name it (line numbers of boris-msg-numbered.txt). "NOT NAMED by him" on a QUESTION = its default is taken by his first line (L1). "NOT NAMED by him" on a READING = it stands as shown unless words of his elsewhere in the message say otherwise. The lists "DECIDED WITHOUT ASKING" and "COMES NEXT AS PICTURES" he did NOT read (L130).

## ITEM IDS OF THIS SLICE (every one needs an @@ITEM block in your paper)
203 204 205 R195 R210 R220 R221 R196 D29 D30 D31 P30 P31

## QUESTIONS (3)
### Question 203 -- The blend, keying and transition lists   [his lines: L110]
About: the layer strip's blend, keying and transition lists (the page of 2026-10-04)
Situation: You open a layer's V drop-down to pick a blend: it lists 13 keying entries and 55 blend entries, and the F drop-down lists 55 transitions. I tested every entry and put the result on a page for you on 2026-10-04; it has no answer yet. Of the 123 entries, 20 do what their name says, 55 do nothing, 43 draw the same picture as another entry, and 5 are wrong. I take your "right slider in layer strip" to be F, the fade-time slider with its list of transitions: say so if you meant another.
- A (THE DEFAULT) [mine]: As that page proposed. Keying: 3 stay, 2 are mended, 8 go. Blend: 2 stay, 4 are mended, 49 go. Transitions: 15 stay, 40 go. Blend and Keying become two lists; the K slider is connected to the Luma Key; "Cut" becomes a true cut.  -- why the default: [mine] It is the proposal of the page of 2026-10-04, which answered your request: "Can you figure out how to get everything working in the keying menu?" It takes out entries that you asked to have working: say B to have every one made to work. "Maybe we get rid of it" was about the keying slider only.
- B: Every entry is made to work, however long it takes; nothing is taken out.
- C: Leave the lists as they are for now; this waits until after the first builds.
Why it was asked: With A the lists become short and every entry does something. The counts are that page's own (a later check differs by one or two). You can keep single entries: "203 a, but keep Overlay and Difference". Say "show me the lists" and I put that page in front of you again. You also asked whether the transparency slider should do the keying. I say no: V stays the fade of the whole layer, and the key gets its own amount in K. Under A, Screen, Multiply, Darken and Lighten keep ignoring the layer's opacity unless you say otherwise.
His earlier words it rested on: "Can you figure out how to get everything working in the keying menu? Does the keying slider actually do anything? Maybe we get rid of it." / "Also review the compositing menu, right slider in layer strip." / "Should we use the transparency slider to do the work for keying elements?"
Source (Harmony-side): area-cue-layers.md H14, A1, M5, T11 (the page's own counts and the ruling's differ by one or two: T11); BD:654-657; the counts: .harmony/.reports/s-rta-1003b/boris-keying.html (VERIFIED: read, the table Keep / Fix / Remove)

### Question 204 -- A layer's hidden kind   [his lines: L111]
About: a layer's hidden kind
Situation: You set layer 1's Blend to Add and nothing happens; and with its fader at 0, layer 1 itself shows black (seen in how the app is built, not tried). The reason is a setting that no screen shows or changes, the layer's kind: layer 1 REPLACES what is below it and ignores Blend and Keying; every layer you add BLENDS over what is below; two more kinds, effects only and mask, exist and your show does not use them (a fifth, 3D, draws nothing and would not be offered).
- A (THE DEFAULT) [mine]: The kind becomes a visible drop-down in the Layer tab (replaces / blends / effects only / mask), so that you can change it and can see why a layer behaves as it does.  -- why the default: [mine] Nothing that exists is taken away, and the setting that silently decides whether Blend and Keying work becomes visible.
- B: Layers have no kind: every layer blends the same way, layer 1 too. Effects-only layers, mask layers and the autopilot that is set per kind of layer go with it.
- C: As today: hidden.
Why it was asked: Today the only way to change a layer's kind is to edit the show file by hand. It decides what layer 1's Blend list does.
Source (Harmony-side): area-cue-layers.md H13, M4 (Composition.h:220; Autopilot.cpp:148-162; LayerInspector.cpp:534, 594, 828)

### Question 205 -- Timeline and Clip Position: keep both or one   [his lines: L112]
About: the Timeline curve and Clip Position
Situation: You ruled on the Timeline curve on 2026-09-05 with one word, "build.", and later that you are fine with the editor in the Signal tab. Then you asked: "Explain this to me. Where do I draw the timeline curve?" The answer: in the Signal tab, where you draw an envelope; it is not built yet. Today "Timeline" in a slider's source list is a plain ramp over 4 beats, and the list also holds "Clip Position", a straight line from the start of the clip to its end, which reads 0 and does nothing today because it is not connected to the clip yet (seen in how the app is built, not tried). The one thing still open is what I asked you then: keep both, or remove Clip Position?
- A (THE DEFAULT) [mine]: Keep both: Timeline is the curve you draw, and Clip Position stays as the straight line and is connected to the clip.  -- why the default: [mine] It was the default of the question as first asked (keep both), and nothing that is on screen is taken away.
- B: Remove Clip Position: Timeline alone, a straight line along the clip until you draw a curve.
Why it was asked: Until you draw a curve, Timeline and Clip Position do the same thing, so one of them may be one entry too many in the list. Where Timeline is offered: R220. Whether the curve is drawn in the Signal tab's small editor or in a strip under the slider comes as a picture.
His earlier words it rested on: "build." / "Explain this to me. Where do I draw the timeline curve?" / "I am fine using the envelope editor in the signal tab. I want all of the controls to work in that one rather than an extra bigger envelope window."
Source (Harmony-side): area-effects-signals.md E6, E7, O11, 1.16, part 5 (page Q25 / BF19); BD:258, BD:641-644; BD:258 ("build."); src/connect/ConnectionEngine.cpp:327-345 (no clip clock is passed: lines read by me for macros, global and layers; the clip site by the seats); boris-feedback-backlog.md:109 (the question as first asked: keep both, or remove Clip Position)

## READINGS (5)
### R195 (about: signals and effects: what stays)   [his lines: L104]
Stays as it is unless you say otherwise. (a) A signal is plugged into a slider with the small triangle on its row; taking it off is "Manual". (b) The Master Signal fader scales every signal at once and does not touch actions; the ignore lamp is for actions only. (c) Two hand-back rules live side by side, both yours: a slider that a signal drives glides back to the signal when you let go ("smooth transition back."), and a slider that an action moves jumps (158 A). (d) You have two signals of your own, Mod 1 and Mod 2; more can be made only for envelopes, when that part is built. (e) A plugged-in slider has Range and Invert, and no smoothing control. (f) Sliders read 0 to 1, without units. (g) Effects cannot be re-ordered, copied or pasted: an effect is added at the end and can only be removed (in Resolume you drag an effect's name row to move it). Say "R195 g drag" if you want to re-order effects by dragging. What changes for signals and macros: R210.
(label: "his words (smooth transition back; the Master Signal); today's facts VERIFIED by the area sheet (1.07, 1.12-1.19); the changes are reading R210")
(source: "area-effects-signals.md E10, E11, E15, E16, E18, X1, X3, X4, X6, O9, W4, W5; BD:254-256, 263, 425-433, 550-570")

### R210 (about: signals and macros: what changes)   [NOT NAMED by him]
Three changes, the first two from your own words. (a) Macros: three kinds of bank, as you ruled ("all. Global, layer, clip."), 8 knobs each: every clip has its own 8, every layer its own 8, and there is one global 8. A slider in the Clip tab plugged into Macro 3 follows that clip's third knob, in the Layer tab that layer's, in the global tab the global one. Today there is one bank, shown in all three tabs. Say so if all clips should share one bank. (b) The switch you ruled on 2026-09-06 (an oscillator restarts on a drop, or flows through it) exists in the app but has no control on screen: it gets one on the slider's row. (c) Mine: signals that run on the beat (oscillators, envelopes) hold still while the beat is paused or stopped, and start from their beginning on the "1" after a stop; signals that listen to the music keep moving.
(label: "his words (the three macro banks; the drop switch); today's facts VERIFIED by the area sheet (1.07, 1.12-1.19); (c) INFERRED")
(source: "area-effects-signals.md E10, E11, E18, X1, X3, O9, W4; BD:263, 425-433, 550-570")
(star: true)

### R220 (about: where a slider can follow the Timeline)   [NOT NAMED by him]
Your words: "when I set any clip parameter to timeline it should be locked to the clips playhead, same with layer, not necessary for composition controls" and "For Milk drop there is no timeline, but there are effects." So: (a) "Timeline" is offered on a clip's sliders and on a layer's sliders, and not on the global tab's. (b) On a layer it follows the clip that is playing on that layer. (c) Mine: on a clip that has no length of its own (MilkDrop, a generated picture, a still picture, the camera, an effects-only clip) the entry "Timeline" is greyed; every other signal still works there. I read your "no timeline" for MilkDrop as covering those too: say so if one of them should have a Timeline.
(label: "his words (both sentences); (c) INFERRED from his MilkDrop sentence")
(source: "BD:591-594, BD:645; area-effects-signals.md part 6 (N6)")

### R221 (about: signals and effects: the edges)   [his lines: L106, L107, L108]
Mine, edges that no word of yours covers. (a) In the Signal tab an envelope has two toggles, "Looping" and "One Shot"; today they change nothing (seen in how the app is built). They are made to work: a one-shot envelope starts on the next "1", plays once and holds its last value. (b) An effect that reads the beat by itself (a strobe, a pulse) follows the beat as the signals do: it holds still while the beat is paused or stopped (R214); the Master Signal at 0 still leaves such effects pulsing while the beat runs, as you ruled. (c) The Signal tab's small editor has no moving line that shows where a sample is now; say so if you want one. (d) Not tried: loading a preset on Freeze or Echo, or switching one off and on with B, keeps the picture it holds; removing the effect clears it.
(label: "INFERRED (no word of his covers them); the two toggles that do nothing: VERIFIED in the code by the cover-effects seat (EnvelopeSignal.h:60-68); what an effect that reads the beat does on a pause today, and what the code does with a held picture when a preset is loaded: UNKNOWN")
(source: "area-effects-signals.md E11, E12, X5, W2; BD:641-644, 678")

### R196 (about: bars and beats in what you read)   [NOT NAMED by him]
Your rule ("bars in most places unless beats are necessary"), applied to every list of lengths: the oscillator, the envelope, the autopilot's count, the count per kind of layer and MilkDrop's two lists (R212) read in bars, and a length shorter than a bar reads as a part of a bar, the way you write it yourself ("1/2 bar"). Beats stay in the circle of the tempo row, in Beat Repeat and Random (as Resolume counts them, R192) and in the length of a BPM-mode clip (R218). A layer's fade (F), MilkDrop's blend and an output's Delay stay in seconds or milliseconds: they are times, not musical lengths.
(label: "his words (the rule and his own fractions of a bar); which lists it reaches INFERRED from most places")
(source: "area-tempo.md U-HIS-11; area-effects-signals.md E13, W3, O8; area-sources-auto.md H7, M2; BD:646-648")

## DECIDED WITHOUT ASKING (3) -- he did NOT read these: each is still Harmony's assumption until his words settle it
### D29: The Layer Router's "Source Layer" names the layer it takes its picture from and follows that layer when the layers are re-ordered.
(decided by: area-cue-layers.md M6, T7 (Renderer.cpp:1285-1296: today a slider that picks one of the first ten places).)

### D30: A sample envelope's handles snap to bar lines counted from the start of the sample at the tempo of the moment. There is no bigger envelope window (your words).
(decided by: His words (BD:641-644, 678); area-effects-signals.md E12, W2, O7: the adopted plan's big view is not built.)

### D31: The Layer tab's Video Blend Mode list and the layer strip's V list become one list with the same entries (today the Layer tab shows 25 names and a blank for the others).
(decided by: area-cue-layers.md M5 (read there, not by me): INFERRED.)

## COMES NEXT AS PICTURES (2) -- he did NOT read these; his new rule on layout is L8
### P30: The drop-down lists of the layer strip after question 203: Blend, Keying, Transition.
(variants that were to be drawn: two drop-downs on V (Blend, Keying) and one on F | one drop-down on V with two headed groups, one on F)

### P31: Where the Timeline curve is drawn (you ruled: "build.").
(variants that were to be drawn: in the Signal tab's small editor, where an envelope is drawn | in a strip that opens under the slider's own row)

