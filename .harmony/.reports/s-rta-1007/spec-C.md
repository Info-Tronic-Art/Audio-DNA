# SPEC C -- Presets (s-rta-1007): the paper apply-C.md with the ruling rule-C.md laid over it by merge.py. This file wins over both.

## ITEMS
@@ITEM 178
TITLE: The preset's name in the effect's header line; Save replaces it
STATUS: ANSWERED in his own words (no letter fits)
HIS: L52
RULE: His words: "178 I would like to have the presets name in the same header line as the effects name. If the preset gets changed then remove the preset name and replace it with save button". Built as: (1) Every effect's header line reads the effect's name and, in the same line, the preset button: it reads a preset's name with a small arrow; a click opens the drop-down (Default, his presets of this effect in name order, a line, "Manage...", "Save"). (2) WHAT IT READS: the name of the preset of this effect that the effect's settings equal; "Default" when they equal the effect's defaults (his "150 a is good", BD:1080). "Settings" are what a preset holds (R157 (6)); equal means: every slider and Dry / Wet within 0.0005, and the same signals plugged with the same settings. (3) His words: once the settings equal no preset and not the defaults ("If the preset gets changed"), the name is taken away and the small Save button stands in its place, in the same line; the list of presets stays reachable through a small arrow beside it (C-6). (4) When the settings equal a preset again (the slider is put back, a preset is picked, or the settings are saved as a preset) the Save button goes and the name is back. (5) Pressing Save: R127 (d) and (e). (6) THE EFFECT REMEMBERS the preset it was last set from: picked in its drop-down ("Default" counts), dropped from the Effects tab, or saved from it; the show keeps that with the effect (R157 (8)). It is used for one thing: when the settings equal more than one preset, or a preset and the defaults at once, the header reads the remembered one; with none remembered among them, "Default" first, then the first by name. The name shown is always found by comparing, so it never names a preset the effect does not equal: a rename shows at once on every effect that equals the preset; after a delete those effects keep their settings and show the Save button; an effect set by hand to exactly a preset's values reads that preset's name (C-5). (7) WHAT COUNTS AS CHANGED: the value he set on a slider or on Dry / Wet, and what is plugged in. A slider that a signal, a macro knob or an action is moving is compared by the value set underneath, not by where it is at this moment, so a running signal or action never takes the name away; B (bypass) is no part of it. (8) Where the parts sit in the line: R136 (a), laid out by Harmony (L8).
CHANGED: Option A (the default) is replaced in one part: the page said "the name stays after you change a slider"; his words remove the name on a change. A's "The effect remembers which preset it carries" is kept, he did not touch it: it is the remembered pick of (6). Options B and C (a "P" and no name) are not taken. Harmony's, not his word: that the name shown is found by comparing (C-5); the arrow beside Save (C-6); name order, the 0.0005, what counts as changed, and the order between presets that are alike (C-16).
TODAY: No preset code on main (facts-presets-delta 1.1-1.3): an effect's header carries "B", the effect's name and "X" only (1.16); EffectSlot has no preset field (1.2). Everything here is new, including one optional key on the effect's entry in the show for the remembered preset (the unadopted ruling's "fxPreset", 2.18; written and read at the three places of 1.3). The earlier, never adopted ruling's "no mark" pixel gate (2.4-2.5) is overtaken; its reading order (the carried preset first, then any preset the settings equal, 2.3) is close to what is ruled here.
@@END

@@ITEM 192
TITLE: Loading a preset keeps a signal he plugged in himself
STATUS: ANSWERED b
HIS: L53
RULE: His words: "192 b". Loading a preset sets every slider and Dry / Wet to the preset's values at one moment, as a jump, without a glide (his words on a preset drop: "all of a sudden there are 8 jumps" and "the 8 values change at the same time", BD:1089), and plugs in the signals the preset holds, each on its slider with the settings saved for it (C-11). A slider that the preset saved WITHOUT a signal gets the preset's value and keeps whatever signal he plugged into it himself. A slider the preset saved WITH a signal gets that signal; a signal of his own that was on that slider gives way to it. To take a signal off, he unplugs it by hand. The whole load is one Undo step: values and signals come back together. Picking "Default" follows the same rule: values back to the defaults, plugged signals stay (C-8). An effect that keeps a signal the preset does not hold does not equal the preset, so by 178 its header shows the Save button, not the preset's name (C-7).
CHANGED: Option A (the default, "the slider goes to Blue's value and the bass is unplugged; Cmd+Z brings the bass back") is replaced by B. With it goes the earlier reading of his answer 131 as A (BD:988-992, Harmony's text after "->"). Harmony's, not his word: what "Default" does to signals (C-8); what the header reads after such a load (C-7); that a preset's signal takes the place of one of his own on the same slider (C-16).
TODAY: No preset load exists (facts-presets-delta 1.1). The unadopted ruling had "a slider the preset holds without a signal and Default unplug the effect's signals" (2.10): overtaken.
@@END

@@ITEM R136
TITLE: The effect's header row and drop-down; no "Effects" bar
STATUS: CORRECTED one thing added; (a) (b) (c) stand
HIS: L40, L52
RULE: (a) The header of an effect reads the effect's name. The fold arrow sits at the left of the name; B (bypass), the preset button and X (remove) sit at the right end of that row, as in his Resolume picture; the exact places are laid out by Harmony (L8). (b) The preset button reads what 178 settles: the preset's name, replaced by Save on a change. Its drop-down reads: Default, his presets of this effect, a line, "Manage...", "Save". (c) In the Effects tab an effect that has presets gets a fold arrow and its presets are listed under it; an effect with none shows no arrow; "Default" is not listed there. ADDED by his words "R136 if there are no effects in the effects tab for a clip layer or Global, then there is no header bar for any effects. There should not be space wasted with a bar that says “effects”. Each effect has its own header bar and that's it, like resolume": the effects part of the Clip tab, the Layer tab and the Global tab never has a bar that names the effects, under any wording, with or without effects in it; each effect's own header bar is all there is. An empty stack takes no room, and an effect or a preset dragged onto that tab is still taken.
CHANGED: Added: no bar for the effects section (the page did not speak of it). (a) stands as shown: he read "I take it that B moves there" and did not correct it; L8 covers the exact places. The last sentence "What the button reads: question 178" is now answered (L52). INFERRED: "the effects tab for a clip layer or Global" is read as the effects part of the Clip, Layer and Global tabs, not the Effects tab of the browser. Harmony's, not his word: that a drop onto the tab is still taken when there is no bar and no effect (C-16).
TODAY: The three tabs each paint a section bar: "Effects" (src/ui/ClipInspector.cpp:590), "Layer Effects" (src/ui/LayerInspector.cpp:564), "Global Effects" (src/ui/CompositionInspector.cpp:252): all three go. B sits at the left of the name, X at the right (facts-presets-delta 1.16). The Effects tab has no fold arrow on an effect and no rows under one (1.8-1.9).
@@END

@@ITEM R127
TITLE: The small Save button
STATUS: REPLACED part (c) only, by his L52; (a) (b) (d) (e) stand
HIS: L52
RULE: (a) The Save button shows the moment the effect's settings equal no preset of this effect and not its defaults, and goes away when they equal one again. (b) It shows on an effect that never had a preset too, as soon as it leaves its defaults. (c) REPLACED: the preset's name does not stay after a change: "If the preset gets changed then remove the preset name and replace it with save button" (L52); the Save button stands where the name stood. (d) Press it: a small window asks for a name and already holds the next free one ("Preset 3"), selected so that typing replaces it. Press Save in it: a NEW preset of this effect is kept under that name, on the computer at once (C-15); the effect now equals it and remembers it, so the header reads the new name and the Save button is gone until a setting changes again. (e) A name that already exists: the window asks first, then saves over that preset (his 103 and 132). "Save" in the drop-down does the same as the button, also on an effect that already equals a preset: a new name then makes a second preset with the same settings, and the header reads the new one (178 (6)). A name is unique within its effect and is never "Default". Saving a preset is not an Undo step (C-15).
CHANGED: (c) is replaced: the page said "the preset's name itself still stays after a change and gets no mark". In (d) "reads its name (178 A)" now follows from 178 as answered. Harmony's, not his word (C-16): the name in the window is selected; "Save" on an effect that equals a preset makes a second preset; a name is unique within its effect. Nothing else.
TODAY: Nothing of it exists (facts-presets-delta 1.1, T7). Not checked beyond the sheet.
@@END

@@ITEM R154
TITLE: Dragging a preset; a double-click shows the properties
STATUS: CORRECTED the double-click; the rest stands ("good")
HIS: L41
RULE: A preset is dragged from the Effects tab onto a clip, a layer or the Global stack the same way as the effect itself; the effect arrives under its own name, set to the preset's values (so its header reads the preset's name), as one Undo step; the effect dragged by itself arrives at its defaults. A dropped preset is always added as a new effect at the end of the stack and never changes an effect that is already there. A single click on a preset or an effect in the tab only selects it for dragging. REPLACED by his words "R154 good but if you double click an effect in effect tab display the effects properties like resolume does. Look at screenshot where I double clicked on Add Subtract: Red": a double-click on an effect or on a preset in the Effects tab displays that effect's properties in a panel under the list, as in his picture: a header with the effect's name, then every row of the effect with its value. Double-clicked on a preset (his picture: the row Red under Add Subtract), the rows show the preset's values (R 0 %, G -100 %, B -100 %); double-clicked on an effect, its defaults. What his words do not say is built on the best reading until he answers (C-9): the panel is for looking; its rows cannot be moved, nothing in it reaches the show, the preview monitor does not show the effect, and presets are made and changed only on an effect that is in the show. The panel stays until another row is double-clicked.
CHANGED: Replaced: "a double-click on a preset or on an effect does nothing in the first build". Stands: the drag, what arrives, the single click, "always added as a new effect". His picture settles the panel for a preset row too: he double-clicked the preset Red and the rows read Red's values. Not his word: whether the preview monitor also shows the effect and whether the rows can be moved (C-9, asked); "at the end of the stack", "as one Undo step" and how long the panel stays (C-16).
TODAY: A double-click in the tab does nothing at all (area-effects-signals correction W6; facts-presets-delta 1.13). Only "fx:" names can be dragged, taken at six places and appended at the end (1.11-1.12); a preset drag needs a new drag text and "add the effect and load the preset" as one Undo step (2.22). In Resolume a double-click on an effect also shows it in the Preview Output over what is playing (facts-resolume-emulate A5.8, VERIFIED from its manual); what that does on a preset row there is not known beyond his picture.
@@END

@@ITEM R155
TITLE: Save and Manage...; presets travel with the show
STATUS: CORRECTED travel added; (b) (c) re-worded by 178; (a) (d) stand
HIS: L48, L22, L52
RULE: (a) "Save" in the drop-down does the same as the small Save button (R127). "Manage..." opens a small window with the names of this effect's presets, where a name can be changed and a preset deleted (his look in Arena: "G manages a small window with the names of all of the presets and the names can be changed or the presets can be deleted."; his picture: a window "Manage Presets", a list headed "Presets" with Blue, Green, Red, buttons Cancel and Save). Changes made in the window count when its Save is pressed; Cancel drops them (C-14). (b) Rename: the preset has the new name everywhere at once: in the Effects tab, in every drop-down and in the header of every effect that equals it. (c) Delete: the preset leaves the tab and the drop-downs; effects that equalled it keep their settings and, equalling no preset now, show the Save button (178). (d) "Default" is always first in the drop-down, is not listed in the tab or in "Manage...", and puts the effect back to its defaults: every slider and Dry / Wet (what it does to plugged signals: 192 and C-8). ADDED by his words "R155 presets are such low storage files that they should travel with the show file if it goes to another computer": every Save of a show writes a copy of his presets into the show file, so the one file carries them to another computer (which presets, and what opening does there: R157, C-2, C-3).
CHANGED: Added: presets travel with the show file (L48). (b) and (c): by 178 as answered the header names a preset only while the effect equals it; in (c) those effects no longer read "Default": they show the Save button (L52). (a) is now backed by his own look in Arena (L22) and his picture. INFERRED from his two pictures of 2026-10-07: "all of the presets" means all presets of the one effect (the window lists Blue, Green and Red of Add Subtract, while the tab picture shows that Blow has two presets of its own). (d) stands as shown; its "back to its defaults" is kept apart from the signals, which C-8 covers.
TODAY: Nothing exists (facts-presets-delta 1.1). The show file holds no preset (1.3). The unadopted ruling's "Rename" and "Delete" menu entries (2.13-2.14) are replaced by "Manage...".
@@END

@@ITEM R171
TITLE: A preset that is not on this computer
STATUS: REPLACED by his words on travel and on the name
HIS: L48, L50, L52
RULE: The case needs no rule of its own. (1) The show file brings its presets along (L48, L50), so opening a show on another computer finds them (R157 (3)). (2) An effect's look never hangs on a preset being there: the show holds every slider value and every plugged signal of every effect by itself. (3) The header names a preset only while the effect's settings equal a preset that is on this computer; an effect that equals none shows the Save button (L52), and reads "Default" only at the effect's defaults. (4) The preset an effect remembers (R157 (8)) only chooses between presets that are alike; it never puts a name on an effect whose settings differ. So a new preset made later under an old name is read as the effect's own only when the settings equal it.
CHANGED: The whole reading: the page said the effect keeps its settings and reads "Default", and "The show still keeps the name". Replaced by travel (L48, L50) and by 178 (L52). The show still keeps which preset an effect was last set from (R157, stands), but only as the choice between presets that are alike (C-5, C-16).
TODAY: Nothing exists. The "fxPreset" key of the unadopted ruling (facts-presets-delta 2.18) is not built; a key of that kind is needed for the remembered preset.
@@END

@@ITEM R156
TITLE: The second small "P" and the title "Arena Presets"
STATUS: STANDS confirmed by him
HIS: L49
RULE: His words: "R156 we do not need that 2nd small p. It’s superfluous". No preset button on a single row of an effect; one preset button per effect, in its header line. The drop-down has no title line ("Arena Presets" is Resolume's own); the app ships no presets of its own (his 101, BD:925).
CHANGED: nothing
TODAY: Neither exists (facts-presets-delta T18, T19).
@@END

@@ITEM R157
TITLE: Where presets are kept: the app and the show file
STATUS: CORRECTED where presets live; the rest stands
HIS: L50, L48, L106
RULE: His words: "R157 they are kept with the app and the show file. Very little storage overhead". (1) THE COMPUTER'S PRESETS: every preset of every effect is kept with the app on this computer; they are there for every show. Saving, renaming or deleting a preset changes them at once, without a Save of the show (C-15). The Effects tab, the drop-downs and "Manage..." read only these. (2) THE SHOW'S COPY: every Save of a show writes a copy of all the computer's presets into the show file (C-2). (3) OPENING A SHOW, also the show the app opens at launch (L94): the show's presets are compared with the computer's, effect by effect. A preset the computer does not have is added to the computer's presets and stays there for every show (C-2), unless he deleted it on this computer after the show's copy of it was last changed (C-4). The same preset on both sides with the same name and values: nothing happens. The same preset with another name or other values: the version changed last wins, name and values (C-3). A different preset of the same effect that only shares the name: both are kept, and the show's gets a number after its name ("Blue 2") (C-3). Nothing is asked and no window opens. Taking a deck out of another show compares nothing. (4) ANOTHER COMPUTER: copy the one show file there and open it; his presets are then in the Effects tab and in every drop-down, in that show and in every other show on that computer (C-2). (5) The look of a show never hangs on any of this: the show holds every effect's values and plugged signals by itself. (6) WHAT A PRESET HOLDS, for one effect: each slider's value; Dry / Wet; the signal plugged into each slider and into Dry / Wet (his 102 b), by the signal's name (R180 (a), C-12); with it every setting of that slider for its signal: Gain, Falloff, Looping / One Shot (L106) and whatever else topic I leaves on the slider's row, and a Threshold should an effect ever have a button that takes a signal (C-11). Never the effect's B (bypass). (7) Loading a preset is one Undo step: values, signals and the remembered preset come back together. (8) THE SHOW KEEPS, with each effect, which preset it was last set from; it is used only to choose between presets that are exactly alike (178 (6)). (9) So that "the same preset" can be told from "another preset of the same name", every preset carries a hidden identity and the time of its last change, and the computer remembers which presets he deleted and when.
CHANGED: Replaced: "presets are kept with the app on this computer, not inside a show" (his earlier "these are saved with the app. always.", BD:894-895) by the app AND the show file. Stands: what a preset holds; one Undo step; "the show keeps the name of the preset an effect carries" (now (8); its condition, that the effect remembers, holds by 178 (6)). Everything in (2) and (3) beyond "kept with the app and the show file" and "travel with the show file" is Harmony's: C-2, C-3, C-4. "Never bypass" is not his word either: it was told to him as a reading with questions 101 to 106 on 2026-10-04 ("Not its bypass", boris-clarify-101-106.md:32) and not corrected. Harmony's and technical (C-16): the hidden identity, the time of the last change, the remembered deletes, and that a taken deck compares nothing.
TODAY: No preset store exists (facts-presets-delta 1.1); the show file has no preset key (1.3). The unadopted ruling kept presets only under ~/Library/Audio-DNA/Effect Presets/<effect>/<name>.preset.json and identified them by name, with an id put off (2.17, 2.19): the id and a changed-last stamp are now needed. The show file's shape changes; old show files are deleted by his word (L69), so nothing is converted; the one verified writer stays (CLAUDE.md Pitfall 68). An effect has sliders only (src/effects/EffectLibrary.h:16, ParamDef), so no effect button takes a signal yet.
@@END

@@ITEM R180
TITLE: Signals by name; a cell with effects only
STATUS: CORRECTED one thing added; (a) (b) (c) stand ("yes")
HIS: L51, L104, L50, L111
RULE: (a) A preset remembers a plugged-in signal by its name only; a signal's own shape belongs to the show, so one preset can pulse differently in a show where the signal of that name has another shape. When the open show has no signal of that name, the slider gets the preset's value and nothing is plugged in (the best reading until he answers: C-12); the header then shows the Save button, because the effect is not exactly the preset (178). (b) A preset dropped on an empty cell makes a clip that holds only that effect, with the preset loaded. (c) Presets are listed in the Effects tab and dragged from it. ADDED by his words "R180 yes and a cell with effect(s) only can be used similar to a layer effect. They will affect the layers below it. If there is an effect at the bottom layer, it will not be effective": a cell that holds only effect(s) and no picture, when it plays, puts its effects on the picture made by all the layers below its own layer; the layers above are not touched. On the bottom layer there is nothing below it, so it changes nothing and shows nothing. The layer's transparency slider sets how strongly the effects show, from not at all to fully (C-13).
CHANGED: Added: what a cell with effect(s) only does to the layers below, and that it does nothing on the bottom layer. (a), (b), (c) stand by his "yes". In (a) the page's reason "Presets belong to the app" now reads: to the app and the show file (L50); a preset then reaches every show while a signal's shape stays in one show, and with L104 (an envelope per slider) a preset will often meet a show that lacks its signal: asked (C-12). The strength slider is INFERRED from L111 (C-13).
TODAY: A drop of an effect on an empty cell already makes an effects-only clip (area-effects-signals 1.04), and it is already drawn on the layers below with the clip's and the layer's opacity as its strength (src/render/CompositorEngine.cpp:852-866, 1139-1146; read by the ruling too). A connection saves only the signal's name (1.15); the area sheet itself leaned to a preset carrying a copy of the signal's settings (X2). New: the preset drop on a cell.
@@END

@@ITEM R207
TITLE: The tab reads "Effects"
STATUS: STANDS
HIS: none
RULE: The tab of the browser that lists the effects and their presets reads "Effects". His own words for that tab are "effect tab" (L41) and "the effects tab" (BD:1089). The effects part of the Clip, Layer and Global tabs is a different place and carries no name on screen (R136).
CHANGED: nothing
TODAY: The tab's button reads "FX" (facts-presets-delta 1.5: src/ui/BrowserPanel.h:48).
@@END

@@ITEM P6
TITLE: The preset button, the Save button, the name window
STATUS: SETTLED what it does, by 178; where it sits is laid out by Harmony (L8)
HIS: L52, L8
RULE: What it does is settled by his words: the preset's name stands in the effect's header line and is replaced by the Save button on a change (178), which is the third variant that was to be drawn. The starting layout is the one of his Resolume picture as R136 (a) has it (B, the preset button and X at the right end of the row); the exact places and the look are laid out by Harmony where they fit in the correct area and are settled in the UI redesign (L8). The name window: R127 (d).
CHANGED: The variants "a P with an arrow" and "a button that keeps reading the name with Save beside it" are not taken.
TODAY: B at the left of the name, X at the right, no preset button (facts-presets-delta 1.16).
@@END

@@ITEM P7
TITLE: Presets under their effect in the Effects tab
STATUS: DROPPED
HIS: L8
RULE: laid out by Harmony where it fits in the correct area; its look is settled in the UI redesign (L8). What it does is in R136 (c) and R154.
CHANGED: nothing; the two variants differed only in how the rows look
TODAY: No fold arrow on an effect and no rows under one (facts-presets-delta 1.8-1.9).
@@END

@@ITEM P8
TITLE: The "Manage..." window
STATUS: DROPPED his picture came; the look of ours waits for the UI redesign (L8)
HIS: L22, L8
RULE: laid out by Harmony where it fits in the correct area; its look is settled in the UI redesign (L8). What it does is in R155 (a): a small window "Manage Presets" that lists this effect's presets by name, where a name can be changed and a preset deleted, with Cancel and Save (his words: "G manages a small window with the names of all of the presets and the names can be changed or the presets can be deleted."; his picture). How a name is changed and a preset deleted with the mouse or the keys is part of that layout.
CHANGED: nothing in what it does. The picture that was asked for came with his look in Arena (L22); the two variants that were to be drawn differed only in look.
TODAY: Does not exist (facts-presets-delta 1.1).
@@END

@@ITEM P9
TITLE: How the open drop-down shows the preset in use
STATUS: DROPPED
HIS: L8, L52
RULE: laid out by Harmony where it fits in the correct area; its look is settled in the UI redesign (L8). The preset to mark, when one is marked, is the one whose name the header reads (178); when the header shows the Save button none is marked.
CHANGED: nothing; the two variants differed only in look. The condition "if the effect remembers its preset" is met by 178 as ruled: the header names the preset the effect equals.
TODAY: No drop-down exists (facts-presets-delta 1.1).
@@END

## ASSUMPTIONS
@@ASSUME C-2
ABOUT: R155 R157 R171
TEXT: I assume a show file carries all your presets, not only the ones it uses. Opening it on another computer adds the ones missing there to that computer's presets for good, without asking.
WHY: L48 and L50 say presets travel and are kept in both places; not which ones, nor what opening does to the computer's own. Nearest earlier word: keys and MIDI (BD:887-897).
ALT: b) Only the presets that the show's effects are set to travel. c) They are added only when you choose an Import command, as with keys and MIDI. d) They are listed only while that show is open.
IF-WRONG: REBUILD one list of presets or two (the computer's and the show's) is the base of the presets build; and a show handed to someone carries every preset he has.
ASK: YES no words of his settle it; it decides how presets are stored and what leaves his computer with a show.
@@END

@@ASSUME C-3
ABOUT: R157 R155
TEXT: I assume that when a show and the computer hold two versions of one preset, the newer wins, name and values, without asking. Two presets made separately that share a name are both kept; the show's gets a number.
WHY: L50 keeps presets in two places; once one copy changes they differ, and no word says which counts. For keys and MIDI the show's set takes over (BD:896-897).
ALT: b) A small window asks which one to keep. c) The show's version always wins when the show is opened, as with keys and MIDI. d) The computer's always wins.
IF-WRONG: REBUILD a preset he tuned could be replaced without a word, and that cannot be brought back.
ASK: YES no words of his settle it, and a wrong guess loses his work silently.
@@END

@@ASSUME C-4
ABOUT: R157 R155
TEXT: I assume a preset you deleted on this computer stays deleted when you open an older show that still holds it.
WHY: Follows from L50 (two places), but he does not speak of deleting; without it every older show would bring deleted presets back.
ALT: b) Opening a show always brings back the presets it holds, deleted or not.
IF-WRONG: SMALL one rule at opening a show.
ASK: LINE a real choice he would most likely wave through.
@@END

@@ASSUME C-5
ABOUT: 178 R127 R171 R157 P9
TEXT: I assume the header shows a preset's name whenever the effect's settings are exactly that preset's, also when you set them by hand without picking it.
WHY: L52 says when the name goes, not how the app knows which preset to name. The effect still remembers its last pick; that only decides between presets that are alike.
ALT: b) Only the preset you picked or saved on that effect is ever named; an effect set by hand to the same values shows Save.
IF-WRONG: SMALL one test in what the header reads.
ASK: LINE he would see it; most likely waved through.
@@END

@@ASSUME C-6
ABOUT: 178 P6
TEXT: I assume that while the Save button stands in place of the preset's name, a small arrow stays beside it, so that you can still open the list and pick another preset.
WHY: L52 replaces the name with the Save button; the name was also the button that opens the list of presets (his 169, BD:1089).
ALT: none
IF-WRONG: SMALL one small button in the header line.
ASK: NO not a real choice: without it no other preset could be picked on a changed effect; where the arrow sits is layout (L8).
@@END

@@ASSUME C-7
ABOUT: 192 178
TEXT: I assume an effect that keeps a signal of yours after a preset is loaded shows Save, not the preset's name: it is not exactly that preset.
WHY: L53 (192 b) lets a signal of his stay; L52 shows the name only for an unchanged preset; together they leave this case open.
ALT: b) The header reads the preset's name until you move something yourself.
IF-WRONG: SMALL the test for "changed" ignores signals the preset does not hold.
ASK: LINE he would see it on every such load; easy to turn.
@@END

@@ASSUME C-8
ABOUT: 192 R155
TEXT: I assume picking "Default" puts the sliders back to the effect's defaults and leaves plugged-in signals where they are, as a preset does; you unplug a signal by hand.
WHY: L53 answers for a preset saved without a signal. Question 131, re-asked as 192, put loading Default under the same letters; the page's "back to its defaults" reads otherwise.
ALT: b) "Default" also unplugs every signal: the effect is as if freshly dropped.
IF-WRONG: STAGE a slider keeps pulsing, or stops, when he picks "Default" during a show; one switch in the load to turn.
ASK: LINE settled closely enough by his 192 b and the wording of 131 (L9); he can strike it.
@@END

@@ASSUME C-9
ABOUT: R154
TEXT: I assume a double-click on an effect or preset in the Effects tab only shows its rows and values under the list, to look at. The preview monitor does not show it, and the rows cannot be moved there.
WHY: L41 says "display the effects properties like resolume does"; Resolume also shows the effect in its preview. He called the no-preview sentence "good"; L35 names files and clip names only.
ALT: b) As in Resolume: the preview monitor also shows the effect over what is playing, and the rows under the list can be moved to try it. c) As b), and a preset can be saved from that panel too.
IF-WRONG: STAGE he double-clicks an effect to try it before the audience sees it and the preview stays as it was; adding it later means a second picture of the whole mix.
ASK: YES his words can be read both ways; it is what the app does, he would see it, and it changes what the preview monitor has to draw.
@@END

@@ASSUME C-11
ABOUT: R157 R180 192
TEXT: I assume a preset also keeps how each slider of the effect uses its signal: its gain, its falloff, and one shot or looping.
WHY: L106 moves one shot, looping, gain and falloff from the signal to the slider that uses it; his 102 b (BD:926-927) predates that and names only which signal.
ALT: b) A preset keeps only which signal is plugged in; the slider's settings for it start at their defaults.
IF-WRONG: STAGE a loaded preset would pulse differently from the one he saved.
ASK: LINE not his word and seen on stage if wrong; he would most likely wave it through.
@@END

@@ASSUME C-12
ABOUT: R180 R157
TEXT: I assume a preset keeps only the name of a signal you made (an envelope), not its shape. Loaded in a show without a signal of that name, the slider gets the preset's value and nothing is plugged in.
WHY: His "yes" (L51) covers "by its name only"; L104 allows an envelope per slider, so a show will often lack the named signal; no word of his covers that.
ALT: b) The preset carries the signal's shape too and makes that signal in a show that lacks it. c) As assumed, plus a small message that names the missing signal.
IF-WRONG: STAGE a preset he expects to pulse stands still in another show; carrying the shape changes what a preset file holds.
ASK: YES no word of his covers it; seen on stage; b) changes what a preset holds on disk.
@@END

@@ASSUME C-13
ABOUT: R180
TEXT: I assume the layer's transparency slider sets how strongly a cell with only effects changes the layers below it, and that such a cell on the bottom layer shows nothing at all.
WHY: L51 says "similar to a layer effect" and "it will not be effective" at the bottom, not which slider; INFERRED from L111 (the transparency slider sets how a layer shows).
ALT: b) The effects always act at full strength and the slider does nothing for such a cell.
IF-WRONG: SMALL one multiplication in the drawing.
ASK: NO it follows from his L111 and L51; b) is not a real choice, and nothing new is built for it.
@@END

@@ASSUME C-14
ABOUT: R155 P8
TEXT: I assume that in the Manage Presets window your renames and deletes count only when you press its Save; Cancel leaves every preset as it was, so no extra "are you sure" is asked.
WHY: L22 says names can be changed and presets deleted; his picture shows Cancel and Save; neither says when a delete takes hold.
ALT: b) A delete takes hold at once and asks "are you sure" first.
IF-WRONG: SMALL the window's buttons.
ASK: NO taken from his own picture of the window.
@@END

@@ASSUME C-15
ABOUT: R157 R127
TEXT: I assume saving, renaming or deleting a preset changes the computer's presets at once and is not undone by Undo; the copy inside the show file is refreshed the next time you save the show.
WHY: L50 names two places, not the moment each is written; by his ruling that all is saved when a show is saved (BD:873-881), the show file is written on Save.
ALT: b) A new preset reaches the computer's presets only when the show is saved.
IF-WRONG: SMALL the moment of one file write.
ASK: NO technical.
@@END

@@ASSUME C-16
ABOUT: 178 192 R127 R154 R136 R157
TEXT: I assume these small rules: presets are listed by name; a running action or signal never counts as a change; a dropped preset lands at the end of the stack; a name is unique within its effect.
WHY: No word of his covers these; each is marked in its item's CHANGED line. None changes what he asked for (L52, L53, L41, L50).
ALT: none
IF-WRONG: SMALL each is one line of the build.
ASK: NO technical; he would not care, and each is easy to turn.
@@END

## QUESTIONS BACK
## NAMES
@@NAME preset
MEANS: One saved setup of one effect: each slider's value, Dry / Wet and the signals plugged into its sliders.
SOURCE: his words, BD:1014 ("138 look is an effect preset. change the name look to preset to avoid further confusion."); used by him in L48, L50, L52
@@END

@@NAME Effects
MEANS: The tab of the browser that lists every effect and, under each effect, its presets.
SOURCE: Harmony's pick from his words "effect tab" (L41) and "the effects tab" (BD:1089); shown to him as a reading on 2026-10-05 and not corrected
@@END

@@NAME Default
MEANS: The first entry of every preset drop-down, and what the header reads on an effect at its defaults; it puts the effect's sliders back to its defaults and is not a preset he can rename or delete.
SOURCE: his picture of 2026-10-05 (the menu's first entry) and "150 a is good" (BD:1080)
@@END

@@NAME Save
MEANS: The small button that stands in place of the preset's name once the effect is changed, and the last entry of the drop-down; both open the small name window that keeps a preset.
SOURCE: his words L52 ("replace it with save button") and BD:1080 ("a little button to save this preset")
@@END

@@NAME Manage...
MEANS: The entry of the preset drop-down that opens the Manage Presets window.
SOURCE: his picture of 2026-10-05 (the menu) and his words L22
@@END

@@NAME Manage Presets
MEANS: The small window that lists one effect's presets by name, where a name is changed or a preset deleted.
SOURCE: the title in his picture [Image #4] of 2026-10-07; his words L22
@@END

@@NAME Global
MEANS: The third place an effect or preset can be dropped, beside a clip and a layer: the stack that acts on the whole picture.
SOURCE: his words L55 ("lets move to global as that is what musicians are more used to")
@@END

@@NAME effects-only clip
MEANS: A cell that holds one or more effects and no picture; it works like a layer effect on the layers below its own.
SOURCE: Harmony's pick for his words L51 ("a cell with effect(s) only")
@@END

## CONFLICTS (from the paper, unruled)
- Where presets live: "these are saved with the app. always." (BD:894-895) against "R157 they are kept with the app and the show file. Very little storage overhead" (L50) and L48. Taken: L50 and L48, the later words on the same point. Not asked again; what they leave open is C-1 to C-4.
- The name after a change: "150 a is good, but as soon as the preset is changed, we have a little button to save this preset" (BD:1080; option A = the name stays) and "It can be called look 2 but no need to show that it was changed." (BD:994-995) against "If the preset gets changed then remove the preset name and replace it with save button" (L52). Taken: L52, his answer to the question that was asked to settle exactly this.
- A preset and his own signal: "If this is a clip in the show, then ctrl-z brings it back. There is no other way." (BD:988-992, read by Harmony as: loading unplugs) against "192 b" (L53: his own signal stays). Taken: L53. Undo still brings back what a load changed.
- One word for two places: "the effects tab for a clip layer or Global" (L40) against "double click an effect in effect tab" (L41, his picture shows the list of all effects). Read as two places: the effects part of the clip, layer and Global tabs, and the Effects tab of the browser. For the list of names (L114): only the browser tab is called "Effects".

## NOT DONE / UNSURE (from the paper, unruled)
- His L66 ("4- please explain in more detail about loading a preset onto an effect with an action playing") is a question back that belongs to topic D (actions); it is not answered here. What this paper gives topic D: a load is one step that sets values and plugs signals (192); the "changed" test compares set values, not the moving value (178, rule 2).
- That a recording keeps a preset load as a jump on every row of the effect (his 169, BD:1089) is topic E; not restated here.
- Whether Resolume itself marks the preset in use in its drop-down, what its name window starts with, and what a double-click on a preset does to its preview monitor: not known (facts-resolume-emulate A2.2, A2.3, A5.8). Not needed: his words L52, L22 and L41 decide these points; C-9 lets him add the preview.
- "Equal within 0.0005" in 178 rule (2) is taken from the unadopted ruling (facts-presets-delta HD-25); a technical number, to be checked when built.
- A hidden identity and a changed-last stamp per preset (needed for C-3 and C-4) are new and designed nowhere yet; the cheapest way to settle C-2 and C-3 is his answer to those two questions before the store is planned.
- All four CONFLICTS bullets are a later line of his replacing an earlier one on the same point (or one word used for two places); none is left for him to choose between, so no question is asked on them.

## FOR THE PAGE RULING (from the ruling)
- ONE RULE on whether an effect remembers its preset: the header's name is found by comparing; the effect also remembers its last pick and the show keeps it, used only between presets that are alike. apply-F R188 (a) and apply-X C3 stay true as written.
- C-9 (a double-click on an effect: properties only, or also in the preview monitor as Resolume) and topic B's B-9 (effects are not previewed by a double-click) are ONE question: ask it once, as C-9.
- C-12 (a preset names a signal the show lacks; does a preset carry the envelope's shape) is the line apply-I hands to presets. The same gap exists for a deck taken from another show and for a pasted clip (topic F): one rule for all three.
- Presets travel (C-2, C-3, C-4): the show file carries ALL presets, opening merges them into the computer's for good, the newer version wins. apply-F leaves this to topic C. Keys and MIDI (BD:887-897) differ: there the show's set takes over.
- Names for his list: "Effects" is the browser tab only (L41, BD:1089); the effects part of the Clip, Layer and Global tabs has no bar and no name on screen (L40). Pick one of "effects-only clip" (C, H) and "effects-only cell" (F).
- C-11 leans on topic I's list of what a slider keeps for its signal (Gain, Falloff, Looping / One Shot; Range and Invert if they stay): a preset holds whatever topic I rules. C-13 follows topic I's reading of L111.
- Topic D's D-5 (a preset loaded under a running action is kept underneath) fits 178 (7): "changed" is tested on the value underneath, so a running action never puts Save on the header.
- apply-X N11 / X-15 (Freeze and Echo keep their held picture on a preset load) belongs to the presets build.
- The paper's four CONFLICTS stand. Two more pulls, both asked: L41 "like resolume does" against the page's no-preview sentence that he called "good" (C-9); the page's "Default ... back to its defaults" against his 192 b (C-8).
- What matters most here, in order: C-3 (a preset replaced without a word), C-12 (a preset that stands still), C-2 (what leaves his computer with a show), C-9. Lines: C-4, C-5, C-7, C-8, C-11.

