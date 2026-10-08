# SLICE C -- Presets
The items of this topic exactly as they stood on the page shown to Boris on 2026-10-05 (source: s-rta-1005/boris-all-items.json), and for each the lines of his message of 2026-10-07 that name it (line numbers of boris-msg-numbered.txt). "NOT NAMED by him" on a QUESTION = its default is taken by his first line (L1). "NOT NAMED by him" on a READING = it stands as shown unless words of his elsewhere in the message say otherwise. The lists "DECIDED WITHOUT ASKING" and "COMES NEXT AS PICTURES" he did NOT read (L130).

## ITEM IDS OF THIS SLICE (every one needs an @@ITEM block in your paper)
178 192 R136 R127 R154 R155 R171 R156 R157 R180 R207 P6 P7 P8 P9

## QUESTIONS (2)
### Question 178 -- The preset button: a name or Resolume's P   [his lines: L52]
About: 150 and 169: the preset button
Situation: You put an effect on a clip and load your preset "Blue" on it. Look at the small preset button at the right end of the effect's name row.
- A (THE DEFAULT) [your words]: It reads the preset's name, "Blue", with a small arrow ("Default" when none is loaded). The effect remembers which preset it carries, and the name stays after you change a slider.  -- why the default: [your words] Your "150 a is good" (the button keeps reading its name) and your words "the preset name area".
- B: As in Resolume: a small "P" with an arrow and no name anywhere; the effect does not remember its preset. Your small Save button is kept (Resolume has none); it shows whenever the sliders match none of the effect's presets.
- C: A "P" with an arrow, as in your picture, but the effect still remembers its preset: the drop-down marks it, and the small Save button shows when a slider leaves it.
Why it was asked: Your picture and your answer 150 cannot both be taken to the letter: Resolume shows no preset name on an effect, and its manual says an effect does not remember which preset it was set from. If you pick B, everything in R155, R157 and R171 about an effect that "carries" a preset falls away, and Rename and Delete change only the list. Your answer 150 and your picture with "Please emulate this." are in the same message.
His earlier words it rested on: "150 a is good, but as soon as the preset is changed, we have a little button to save this preset, as soon as the button is pushed, the user gets a little window to give the preset a name and Push save" / "In that placement the other presets can be selected by clicking a button with a drop down menu in the preset name area just above the effect parameters." / "Please emulate this."
Source (Harmony-side): BD:1080, BD:1089; facts-resolume-emulate.md A4.1 (VERIFIED), A4.3; facts-presets-delta.md U1, S1

### Question 192 -- A preset and a signal you plugged in yourself   [his lines: L53]
About: 131: a preset and a signal you plugged in yourself
Situation: An effect on a playing clip has the bass plugged into its first slider, so that slider pulses. You load your preset "Blue", which was saved with nothing plugged into that slider.
- A (THE DEFAULT) [your words]: The preset is the whole effect as you saved it: the slider goes to Blue's value and the bass is unplugged; Cmd+Z brings the bass back (your words). Sliders that Blue saved with a signal get that signal.  -- why the default: [your words] Your 102 b (a preset holds each slider's value, Dry / Wet and the signal plugged into each slider) and your answer to 131: "If this is a clip in the show, then ctrl-z brings it back. There is no other way."
- B: Loading sets the values and plugs in the signals the preset holds; a signal you plugged in yourself stays on a slider that the preset saved without one.
Why it was asked: I read your answer to 131 as A; this question is here only so that you can say B if you meant it. With A a click on a preset can stop a slider from pulsing in the middle of a show; with B a preset does not always give you exactly the look you saved.
His earlier words it rested on: "131 is the signal plugged into a slider in a clip in the show, or is it a look which is an effect preset in the effect library that can also be connected to a signal. If this is a clip in the show, then ctrl-z brings it back. There is no other way. If it is a look, there is no way to remove the signal unless the user drops it into the show (clip, layer or global) and then adds a signal and saves that look."
Source (Harmony-side): area-effects-signals.md E5, part 5 (131); BD:926-927, BD:988-992

## READINGS (9)
### R136 (about: 169, presets)   [his lines: L40]
As in Resolume: (a) on an effect you read the effect's name, as today (your picture and its manual). One thing moves: today the small "B" sits at the left of the name; in your picture the fold arrow is at the left and B sits at the right, with the "P" and the "X" beside it. I take it that B moves there. (b) At the right end of that same row there is a small "P" with an arrow, between B (bypass) and X (remove). I take it to be the effect's preset button: the open menu in your picture hangs right above it, and Resolume's manual speaks of "the Preset drop down above the effect parameters". Its drop-down reads: Default, your presets, a line, "Manage...", "Save". (c) In the effects tab (it reads "FX" on screen today) an effect that has presets gets a fold arrow and its presets are listed under it (its manual says so); an effect with none shows no arrow in your picture. What the button reads: question 178.
(why_revised: "Adds where the button sits, which effects get an arrow, and what of it I worked out. Adds that the B button moves to the right end of the row, as in your picture.")
(label: "As in Resolume: his two pictures and the manual VERIFIED; which P holds the menu, and the no-arrow rule, INFERRED from the pictures; where B sits today VERIFIED in the code (src/ui/EffectStackView.cpp:97-98: B at the left edge, X at the right) and on his picture (looked at: fold arrow, name, then B, P, X)")
(source: "BD:1089; facts-resolume-emulate.md A1.1, A1.3, A1.4, A1.5, A1.6, A5.1, A5.2; facts-presets-delta.md T1-T5, T11-T13, 1.5 (the tab reads FX: BrowserPanel.h:48)")
(picture: "resolume-effect-panel-presets-menu.png")

### R127 (about: 150: the small Save button)   [NOT NAMED by him]
The small Save button of your answer 150: (a) it shows beside the preset button the moment the effect's sliders no longer match its preset, and goes away when they match again. (b) It shows on an effect that reads "Default" and never had a preset too: your answer 150 was about exactly that effect, so nearly every effect you touch will carry it. (c) That sets aside your 133 ("no need to show that it was changed") for this one button; the preset's name itself still stays after a change and gets no mark (whether a name is shown at all: question 178). (d) Press the button: a small window asks for a name and already holds the next free one ("Preset 3"; your 104). Press Save: a NEW preset is kept under that name; the effect then has the new preset loaded and reads its name (178 A), and the Save button goes away until a slider moves again. (e) Type the name of a preset that exists: the window asks first, then saves over it (your 103 and 132). Say so if you want the Save button only on effects that carry one of your own presets.
(why_revised: "Adds the window's starting name, what an existing name does, and the Save button on a \"Default\" effect. Adds what the effect reads after Save and when the button goes away.")
(label: "his words (150, 133, 104, 103, 132); how they are joined INFERRED")
(source: "BD:1080, BD:928-932, BD:993-995; boris-clarify-150-plus.md:4-7; facts-resolume-emulate.md A2.2, A2.3 (UNKNOWN); facts-presets-delta.md T6-T10, T28, correction C5")

### R154 (about: presets: dragging one)   [his lines: L41]
As in Resolume (its manual says so): you drag a preset from the effects tab onto a clip, a layer or the global stack, the same way as the effect itself. Worked out, not stated in its manual: the effect arrives under its own name, set to the preset's values, and dragging the effect itself brings it at its defaults. Mine: a click on a preset in the tab only selects it for dragging; a double-click on a preset or on an effect does nothing in the first build (in Resolume a double-click on an effect shows it in its preview monitor, and what it does on a preset is not known; here the preview monitor shows clips and cued layers only, and an effect is judged on the output); a preset you drop is always added as a new effect and never changes an effect that is already there.
(label: "As in Resolume (VERIFIED: the manual sentence on dragging a preset); the arriving values INFERRED; click, double-click and the drop mine")
(source: "facts-resolume-emulate.md A5.4, A5.5, A5.6, A5.8; facts-presets-delta.md 1.12; BD:988-990")
(picture: "resolume-effects-tab-presets.png")

### R155 (about: presets: Save and Manage...)   [his lines: L48]
As in Resolume (worked out: its manual says that "Save" opens a window where you type a name, and shows "Manage..." for its other preset menus, not for effects): (a) in the drop-down, "Save" does the same as the small Save button (R127), and "Manage..." opens a small window that lists this effect's presets, where you rename or delete one. Mine, and only if the effect remembers its preset (178 A or C): (b) Rename = every effect of the open show that carries the preset reads the new name, and so does the effects tab; (c) Delete = those effects read "Default", keep their settings (and so show the Save button), and the preset leaves the tab. (d) "Default" is always first in the drop-down, puts the effect back to its defaults, and is not listed in the tab.
(label: "As in Resolume, INFERRED (manual: Save opens the Manage Presets window; Manage... is described for envelope presets); Rename and Delete reaching the effects: mine (the earlier ruling, not adopted)")
(source: "facts-resolume-emulate.md A2.1, A3.1, A3.2; facts-presets-delta.md 5.1-5.7, T13, 2.20")

### R171 (about: presets: a preset that is not on this computer)   [NOT NAMED by him]
Mine (it takes the place of the note in the old R121), and only if the effect remembers its preset (178 A or C): when the preset an effect carries is not on this computer (the show was opened on another computer, or the preset was renamed or deleted while this show was closed), the effect keeps its settings and reads "Default". The show still keeps the name: on a computer that has that preset the effect reads it again. One thing to know: if you later make a new preset under that same name, such an effect will read it as its own.
(label: "INFERRED (the earlier ruling, not adopted; changed on his 150 a: no dim Presets reading)")
(source: "boris-clarify-150-plus.md:101 (R121 as shown); facts-presets-delta.md 2.18, 2.20, 5.1-5.3; BD:1080")

### R156 (about: presets: what is not copied)   [his lines: L49]
Two things in your picture I do NOT copy unless you say so: the second small "P" at the end of the "Blend Mode" row (in Resolume that appears to be a preset menu for that one row), and the menu's title "Arena Presets" (Resolume's own name on its menu; your app ships no presets of its own, your 101).
(label: "INFERRED (picture only; no word of his names them)")
(source: "facts-resolume-emulate.md A1.4, A1.7; facts-presets-delta.md T18, T19, U6, U9; BD:925")
(picture: "resolume-effect-panel-presets-menu.png")

### R157 (about: presets: what stays)   [his lines: L50]
From your words: presets are kept with the app on this computer, not inside a show ("these are saved with the app. always."); a preset holds each slider's value, Dry / Wet, and the signal plugged into each slider (your 102 b). My plan, not yet told to you: loading a preset is one Undo step; the show keeps the name of the preset an effect carries (only if the effect remembers it, question 178).
(label: "his words (84, 102 b); the last sentence is the earlier ruling, not adopted")
(source: "BD:894-895, BD:926-927; facts-presets-delta.md 2.10, 2.17, 2.18, T21-T23")

### R180 (about: presets: signals, an empty cell, and a reading your picture replaced)   [his lines: L51]
Mine. (a) A preset remembers a plugged-in signal by its name only ("Mod 1", "Bass"). Presets belong to the app and the shapes of your own signals belong to the show (R188), so one preset can pulse differently in a show where "Mod 1" has another shape. (b) A preset dropped on an empty cell makes an effects-only clip with that preset loaded, as dropping the effect itself does today. (c) One earlier reading is replaced: I had read your answer to 138 as: presets are picked only from an effect's own button, and none are listed in the effects tab. Your effects-tab picture, sent later, replaces that (R136, R154).
(label: "INFERRED (the name-only rule is the earlier plan's; the drop on an empty cell from today's code as the area sheet read it); the replaced reading VERIFIED against his picture")
(source: "area-effects-signals.md X2, E4, O4, 1.04, 1.15; BD:1014-1016, BD:1089")

### R207 (about: the name of the effects tab)   [NOT NAMED by him]
Your word for it is the effects tab; in your Resolume picture its list is headed "VIDEO EFFECTS". Today the tab reads "FX". It will read "Effects". Say "R207 FX" if you want it to stay short.
(label: "his words (\"the effects tab\", 169); the heading VERIFIED on his picture (looked at); today's text VERIFIED by the fact sheet (BrowserPanel.h:48); the new name INFERRED")
(source: "BD:1089; facts-presets-delta.md 1.5")
(picture: "resolume-effects-tab-presets.png")

## DECIDED WITHOUT ASKING (0) -- he did NOT read these: each is still Harmony's assumption until his words settle it
## COMES NEXT AS PICTURES (4) -- he did NOT read these; his new rule on layout is L8
### P6: Presets on an effect: the preset button beside B and X at the right end of the name row (today B sits at the left of the name: whether it moves next to the "P"), the small Save button, and the name window.
(variants that were to be drawn: a button that reads the preset's name, Save button to its left | a "P" with an arrow, Save button to its left | the Save button inside the name row, shown only when changed)

### P7: Presets in the effects tab: the fold arrow on an effect and the preset rows under it.
(variants that were to be drawn: as in your Resolume picture, indented names | the same with a small dot in the effect's colour)

### P8: The "Manage..." window. One picture from you would settle its look: Resolume's "Manage..." window, open.
(variants that were to be drawn: a list; double-click a name to rename, a Delete button | a list with a Rename and a Delete button beside each name)

### P9: The open preset drop-down: how it shows which preset is loaded, if the effect remembers its preset (178 A or C).
(variants that were to be drawn: a tick beside the loaded preset's name | the loaded preset's name as the title line of the drop-down, where Resolume's reads "Arena Presets")

