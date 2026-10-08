# APPLY C -- Presets (s-rta-1007)
Written: 2026-10-07 22:43:10 (read-only run at HEAD a86cf0a; nothing built, run or launched). Boris is quoted from boris-msg-numbered.txt (L numbers) and binding-decisions.md (BD:line). Pictures looked at: img-04 (Manage Presets), img-06 (effects tab, "Red" double-clicked), and the two of 2026-10-05.

## SUMMARY
- Presets now live in TWO places: with the app on the computer AND inside the show file (L48, L50). This replaces "these are saved with the app. always." (BD:894-895). What his words leave open (which presets go into the show, what opening does on another computer, one preset with two sets of values) is in C-1 to C-4; two of them are questions for him.
- The header line of an effect reads the effect's name AND the preset's name; once the effect no longer equals that preset the name is taken away and the small Save button stands in its place (L52). This replaces option 178 A ("the name stays after you change a slider") and part (c) of R127.
- Loading a preset keeps a signal he plugged in himself on a slider the preset saved without one (L53: 192 b). This replaces the reading of his answer 131 as A.
- A double-click on an effect or a preset in the Effects tab shows its properties under the list, as in his picture (L41); the page had said a double-click does nothing.
- "Manage..." is a small window with the names of the effect's presets; a name can be changed, a preset deleted (L22, his picture).
- No bar that says "Effects" in the clip, layer or Global tab; each effect has its own header bar and that is all (L40).
- A cell that holds only effect(s) works like a layer effect on the layers below it; on the bottom layer it does nothing (L51).
- The second small "P" is not built (L49).
- Because presets travel with the show and the header names a preset only while the effect equals it, R171 (a preset that is not on this computer) falls away.

## ITEMS
@@ITEM 178
TITLE: The preset's name in the effect's header line; Save replaces it
STATUS: ANSWERED in his own words (no letter fits)
HIS: L52
RULE: His words: "178 I would like to have the presets name in the same header line as the effects name. If the preset gets changed then remove the preset name and replace it with save button". Built as: (1) every effect's header line reads the effect's name and, in the same line, a preset button that reads a preset's name with a small arrow; a click opens the drop-down (Default, his presets of this effect in name order, a line, "Manage...", "Save"). (2) The button reads "Default" while the effect is at its defaults (his "150 a is good", BD:1080), and the name of one of his presets while the effect's settings equal that preset. "Settings" = what a preset holds: each slider's set value, Dry / Wet, and the signal plugged into each slider with its settings; a slider that a signal or an action is moving is compared by its set value, not by where it is at this moment; equal = within 0.0005. (3) The moment the settings equal no preset and not the defaults, the name is taken away and the small Save button stands in its place, in the same line; a small arrow stays beside it so that another preset can still be picked (C-6). (4) When the settings equal a preset again (the slider is put back, or a preset is picked or saved) the Save button goes and the name is back. (5) Pressing Save: R127 (d) and (e). (6) The effect itself stores nothing about presets: the name is found by comparing with the presets on this computer (C-5), so a rename shows at once on every effect that equals the preset and a delete leaves those effects with their settings and the Save button. (7) Where exactly in the line the name, the arrow, B and X sit is laid out by Harmony (L8).
CHANGED: Option A (the default) is replaced: the page said "the name stays after you change a slider" and "The effect remembers which preset it carries"; his words remove the name on a change. Options B and C (a "P" and no name) are not taken. That the name is found by comparing, and that an arrow stays beside Save, are Harmony's (C-5, C-6).
TODAY: No preset code on main (facts-presets-delta 1.1-1.3): an effect's header carries "B", the effect's name and "X" only (1.16); EffectSlot has no preset field (1.2). Everything here is new. The earlier, never adopted ruling's four-case button and its "no mark" pixel gate (2.3-2.5) are overtaken.
@@END
@@ITEM 192
TITLE: Loading a preset keeps a signal he plugged in himself
STATUS: ANSWERED b
HIS: L53
RULE: His words: "192 b". Loading a preset sets every slider and Dry / Wet to the preset's values and plugs in the signals the preset holds, each on its slider with the settings saved for it. A slider that the preset saved WITHOUT a signal gets the preset's value and keeps whatever signal is plugged into it. A slider the preset saved WITH a signal gets that signal in place of the one that was there. To take a signal off, he unplugs it by hand. The whole load is one Undo step (values and signals come back together). Picking "Default" follows the same rule: values back to the defaults, plugged signals stay (C-8). An effect that keeps a signal the preset does not hold no longer equals the preset, so by 178 its header shows the Save button, not the preset's name (C-7).
CHANGED: Option A (the default, "the slider goes to Blue's value and the bass is unplugged; Cmd+Z brings the bass back") is replaced by B. With it goes the earlier reading of his answer 131 as A (BD:988-992, Harmony's text after "->"). What "Default" does to signals and what the header reads after such a load are Harmony's (C-7, C-8).
TODAY: No preset load exists (facts-presets-delta 1.1). The unadopted ruling had "a slider the preset holds without a signal and Default unplug the effect's signals" (2.10): overtaken.
@@END
@@ITEM R136
TITLE: The effect's header row and drop-down; no "Effects" bar
STATUS: CORRECTED one thing added; (a) (b) (c) stand
HIS: L40, L52
RULE: (a) The header of an effect reads the effect's name; B (bypass) and X (remove) sit in that row; where each sits is laid out by Harmony (L8). (b) The same row holds the preset button; what it reads is settled by 178 (the preset's name, replaced by Save on a change). Its drop-down reads: Default, his presets of this effect, a line, "Manage...", "Save". (c) In the Effects tab an effect that has presets gets a fold arrow and its presets are listed under it; an effect with none shows no arrow; "Default" is not listed there. ADDED by his words "R136 if there are no effects in the effects tab for a clip layer or Global, then there is no header bar for any effects. There should not be space wasted with a bar that says “effects”. Each effect has its own header bar and that's it, like resolume": the effects part of the clip tab, the layer tab and the Global tab has no bar that says "Effects" (or "Layer Effects", "Global Effects"), with or without effects in it (C-10); each effect's own header bar is all there is; an empty stack takes no room, and an effect or preset dragged anywhere onto that tab is still taken.
CHANGED: Added: no section bar for effects (the page did not speak of it). The last sentence "What the button reads: question 178" is now answered (L52). INFERRED: "the effects tab for a clip layer or Global" is read as the effects part of the clip, layer and Global tabs, not the Effects tab of the browser.
TODAY: The three tabs each paint a section bar: "Effects" (src/ui/ClipInspector.cpp:590), "Layer Effects" (src/ui/LayerInspector.cpp:564), "Global Effects" (src/ui/CompositionInspector.cpp:252): all three go. B sits at the left of the name, X at the right (facts-presets-delta 1.16). The Effects tab has no fold arrow on an effect and no rows under one (1.8-1.9).
@@END
@@ITEM R127
TITLE: The small Save button
STATUS: REPLACED part (c) only, by his L52; (a) (b) (d) (e) stand
HIS: L52
RULE: (a) The Save button shows the moment the effect's settings equal no preset of this effect and not its defaults, and goes away when they equal one again. (b) It shows on an effect that never had a preset too, as soon as it leaves its defaults. (c) REPLACED: the preset's name does not stay after a change: "If the preset gets changed then remove the preset name and replace it with save button" (L52); the Save button stands where the name stood. (d) Press it: a small window asks for a name and already holds the next free one ("Preset 3"), selected so that typing replaces it. Press Save in it: a NEW preset of this effect is kept under that name, on the computer at once; the effect now equals it, so the header reads the new name and the Save button is gone. (e) A name that already exists: the window asks first, then saves over that preset (his 103 and 132). "Save" in the drop-down does the same as the button. Saving a preset is not an Undo step (C-15).
CHANGED: (c) is replaced: the page said "the preset's name itself still stays after a change and gets no mark". In (d) "reads its name (178 A)" now follows from 178 as answered. Nothing else.
TODAY: Nothing of it exists (facts-presets-delta 1.1, T7). Not checked beyond the sheet.
@@END
@@ITEM R154
TITLE: Dragging a preset; a double-click shows the properties
STATUS: CORRECTED the double-click; the rest stands ("good")
HIS: L41
RULE: A preset is dragged from the Effects tab onto a clip, a layer or the Global stack the same way as the effect itself; the effect arrives under its own name, set to the preset's values (so its header reads the preset's name); the effect dragged by itself arrives at its defaults. A dropped preset is always added as a new effect at the end of the stack and never changes an effect that is already there. A single click on a preset or an effect in the tab only selects it for dragging. REPLACED by his words "R154 good but if you double click an effect in effect tab display the effects properties like resolume does. Look at screenshot where I double clicked on Add Subtract: Red": a double-click on an effect or on a preset in the Effects tab shows that effect's properties in a panel under the list, as in his picture: a header with the effect's name, then every row of the effect with its value; for a preset the rows show the preset's values (his picture: Red = R 0 %, G -100 %, B -100 %), for an effect its defaults. The panel is for looking: nothing in it reaches the show, it shows no picture in the preview, and presets are made and changed only on an effect that is in the show (C-9). The panel stays until another row is double-clicked.
CHANGED: Replaced: "a double-click on a preset or on an effect does nothing in the first build". Stands: the drag, what arrives, the single click, "always added as a new effect". What the panel lets him do beyond showing is Harmony's (C-9).
TODAY: A double-click in the tab does nothing at all (area-effects-signals correction W6; facts-presets-delta 1.13). Only "fx:" names can be dragged, taken at six places and appended at the end (1.11-1.12); a preset drag needs a new drag text and "add the effect and load the preset" as one Undo step (2.22).
@@END
@@ITEM R155
TITLE: Save and Manage...; presets travel with the show
STATUS: CORRECTED travel added; (b) (c) re-worded by 178; (a) (d) stand
HIS: L48, L22, L52
RULE: (a) "Save" in the drop-down does the same as the small Save button. "Manage..." opens a small window with the names of this effect's presets, where a name can be changed and a preset deleted (his look in Arena: "G manages a small window with the names of all of the presets and the names can be changed or the presets can be deleted."; his picture: a window "Manage Presets", a list headed "Presets" with Blue, Green, Red, buttons Cancel and Save). Changes made in the window count when its Save is pressed; Cancel drops them (C-14). (b) Rename: the preset has the new name everywhere at once: in the Effects tab, in every drop-down and in the header of every effect that equals it. (c) Delete: the preset leaves the tab and the drop-downs; effects that equalled it keep their settings and, equalling no preset now, show the Save button (178). (d) "Default" is always first in the drop-down, puts the effect's sliders back to its defaults, and is not listed in the tab or in "Manage...". ADDED by his words "R155 presets are such low storage files that they should travel with the show file if it goes to another computer": saving a show writes a copy of his presets into the show file (which ones: C-1), so the file alone carries them to another computer; what opening does there is under R157.
CHANGED: Added: presets travel with the show file. (b) and (c) no longer hang on "if the effect remembers its preset": by 178 the header names a preset only while the effect equals it; in (c) "those effects read Default" becomes "show the Save button". (a) is now backed by his own look in Arena (L22) and his picture.
TODAY: Nothing exists (facts-presets-delta 1.1). The show file holds no preset (1.3). The unadopted ruling's "Rename" and "Delete" menu entries (2.13-2.14) are replaced by "Manage...".
@@END
@@ITEM R171
TITLE: A preset that is not on this computer
STATUS: REPLACED by his words on travel and on the name
HIS: L48, L50, L52
RULE: The case no longer needs a rule of its own. (1) The show file brings its presets along (L48, L50), so opening a show on another computer finds them (R157). (2) An effect's look never hangs on a preset being there: the show holds every slider value and every plugged signal of every effect by itself. (3) The header names a preset only while the effect's settings equal a preset on this computer; an effect that equals none shows the Save button (L52). The show keeps no preset name on an effect, so "a new preset under the same name is read as its own" cannot happen.
CHANGED: The whole reading: the page said the effect "keeps its settings and reads Default" and "The show still keeps the name". Replaced by travel (L48, L50) and by 178 (L52). That the show stores no name on an effect is Harmony's (C-5).
TODAY: Nothing exists; the "fxPreset" key of the unadopted ruling (facts-presets-delta 2.18) is not built and is not needed.
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
RULE: His words: "R157 they are kept with the app and the show file. Very little storage overhead". (1) THE COMPUTER'S PRESETS: every preset of every effect is kept with the app on this computer, one small file per preset, grouped by effect; they are there for every show. Saving, renaming or deleting a preset changes them at once, without a Save of the show (C-15). The Effects tab, the drop-downs and "Manage..." read only these. (2) THE SHOW'S COPY: every Save of the show writes a copy of all the computer's presets into the show file (C-1). (3) OPENING A SHOW: its presets are compared with the computer's. A preset the computer does not have is added to the computer's presets and stays there (C-2). The same preset with the same values: nothing happens. The same preset with different values: the one that was changed last wins (C-3). A different preset of the same effect that only shares the name: both are kept, the show's gets a number after its name ("Blue 2") (C-3). A preset he deleted or renamed on this computer does not come back from an older show (C-4). Nothing is asked and no window opens. (4) ANOTHER COMPUTER: copy the one show file there and open it; all his presets are in the Effects tab and in every drop-down, in that show and in every other show on that computer. (5) The look of the show never hangs on any of this: the show holds every effect's values and signals by itself. (6) A preset holds, for one effect: each slider's value, Dry / Wet, and the signal plugged into each slider (his 102 b) together with that slider's own settings for the signal (C-11); never bypass. (7) Loading a preset is one Undo step. (8) The show keeps no preset name on an effect (C-5).
CHANGED: Replaced: "presets are kept with the app on this computer, not inside a show" (his earlier "these are saved with the app. always.", BD:894-895) by the app AND the show file. Removed: "the show keeps the name of the preset an effect carries" (falls with 178). Stands: what a preset holds; one Undo step. Everything in (2) and (3) beyond "kept with the app and the show file" and "travel with the show file" is Harmony's: C-1 to C-4.
TODAY: No preset store exists (facts-presets-delta 1.1); the show file has no preset key (1.3). The unadopted ruling kept presets only under ~/Library/Audio-DNA/Effect Presets/<effect>/<name>.preset.json and identified them by name, with an id put off (2.17, 2.19): the id and a changed-last stamp are now needed. The show file's shape changes; old show files are deleted by his word (L69), so nothing is converted; the one verified writer stays (CLAUDE.md Pitfall 68).
@@END
@@ITEM R180
TITLE: Signals by name; a cell with effects only
STATUS: CORRECTED one thing added; (a) (b) (c) stand ("yes")
HIS: L51, L104, L50
RULE: (a) A preset remembers a plugged-in signal by its name only; the signal's own shape belongs to the show, so one preset can pulse differently in a show where the signal of that name has another shape. When the open show has no signal of that name, the slider gets the preset's value and nothing is plugged in (C-12). (The page's reason "Presets belong to the app" now reads: to the app and the show file, L50; the rule is the same.) (b) A preset dropped on an empty cell makes a clip that holds only that effect, with the preset loaded. (c) Presets are listed in the Effects tab and dragged from it. ADDED by his words "R180 yes and a cell with effect(s) only can be used similar to a layer effect. They will affect the layers below it. If there is an effect at the bottom layer, it will not be effective": a cell that holds only effect(s) and no picture, when it plays, puts its effects on the picture made by all the layers below its own layer; the layers above are not touched. On the bottom layer there is nothing below it, so it changes nothing and shows nothing. The layer's own transparency slider sets how strongly the effects show, from not at all to fully (C-13).
CHANGED: Added: what a cell with effect(s) only does to the layers below, and that it does nothing on the bottom layer. (a), (b), (c) stand by his "yes". The missing-signal case and the strength slider are Harmony's (C-12, C-13).
TODAY: A drop of an effect on an empty cell already makes an effects-only clip (area-effects-signals 1.04), and it is already drawn on the layers below with the clip's and the layer's opacity as its strength (src/render/CompositorEngine.cpp:1139-1143, 852-866). A connection saves only the signal's name (1.15). New: the preset drop on a cell.
@@END
@@ITEM R207
TITLE: The tab reads "Effects"
STATUS: STANDS
HIS: none
RULE: The tab of the browser that lists the effects and their presets reads "Effects". His own words for it in this message are "effect tab" (L41) and "the effects tab" (L40).
CHANGED: nothing
TODAY: The tab's button reads "FX" (facts-presets-delta 1.5: src/ui/BrowserPanel.h:48).
@@END
@@ITEM P6
TITLE: The preset button, the Save button, the name window
STATUS: SETTLED what it does, by 178; where it sits is Harmony's (L8)
HIS: L52, L8
RULE: What it does is settled by his words: the preset's name stands in the effect's header line and is replaced by the Save button on a change (178), which is the third variant that was to be drawn. Where the name, the arrow, B and X sit in the line is laid out by Harmony where it fits in the correct area; its look is settled in the UI redesign (L8). The name window: R127 (d).
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
STATUS: SETTLED by his look in Arena and his picture
HIS: L22, L8
RULE: His words: "G manages a small window with the names of all of the presets and the names can be changed or the presets can be deleted." A small window "Manage Presets": a list of this effect's presets by name; a name can be changed in the list; a preset can be deleted from it; two buttons, Cancel and Save (his picture). What it does: R155 (a). How a name is changed and a preset deleted with the mouse or the keys (double-click, a key, a button) is laid out by Harmony; its look is settled in the UI redesign (L8).
CHANGED: The picture that was asked for came; the two variants that were to be drawn differed only in look.
TODAY: Does not exist (facts-presets-delta 1.1).
@@END
@@ITEM P9
TITLE: How the open drop-down shows the preset in use
STATUS: DROPPED
HIS: L8, L52
RULE: laid out by Harmony where it fits in the correct area; its look is settled in the UI redesign (L8). The preset to mark, when one is marked, is the one whose name the header reads (178); when the header shows the Save button none is marked.
CHANGED: nothing; the two variants differed only in look. The condition "if the effect remembers its preset" falls with 178.
TODAY: No drop-down exists (facts-presets-delta 1.1).
@@END

## ASSUMPTIONS
@@ASSUME C-1
ABOUT: R155 R157
TEXT: I assume a show file carries a copy of every preset you have at the moment you save it, not only the presets its own effects use.
WHY: L50 says presets are kept "with the app and the show file", L48 that they "travel with the show file"; neither says which presets go in.
ALT: b) Only the presets of the effects that are in the show. c) Only the presets that effects in the show are set to.
IF-WRONG: SMALL the list written into the file changes; nothing else.
ASK: LINE a real choice, most likely waved through ("Very little storage overhead").
@@END
@@ASSUME C-2
ABOUT: R157 R155 R171
TEXT: I assume that when you open a show on a computer that lacks some of its presets, they are added to that computer's presets for good, and can be used in every other show there too.
WHY: L48 and L50 say presets travel with the show and are kept in both places; they do not say what opening a show does to the computer's presets.
ALT: b) They are listed only while that show is open and are gone from the list when it is closed. c) They are added only when you say so with a command.
IF-WRONG: REBUILD one list of presets or two (the computer's and the show's) is the base of the whole presets build.
ASK: YES no words of his settle it; it decides how the store is built.
@@END
@@ASSUME C-3
ABOUT: R157 R155
TEXT: I assume that when the show and the computer hold the same preset with different values, the one changed last wins, without asking. Two different presets that only share a name are both kept; the show's gets a number.
WHY: L50 keeps presets in two places; once one copy is changed they differ, and no word of his says which copy counts.
ALT: b) The computer's preset always wins and the show's is dropped. c) The show's always wins. d) A window asks each time.
IF-WRONG: REBUILD a preset he tuned could be replaced without a word, and that cannot be brought back.
ASK: YES no words of his settle it, and a wrong guess loses his work silently.
@@END
@@ASSUME C-4
ABOUT: R157 R155
TEXT: I assume a preset you deleted or renamed on this computer does not come back when you open an older show that still holds it.
WHY: Follows from L50 (two places) but he does not speak of deleting; without it every older show would bring deleted presets back.
ALT: b) Opening an older show brings its presets back, deleted or not.
IF-WRONG: SMALL one rule at opening a show.
ASK: LINE a real choice he would most likely wave through.
@@END
@@ASSUME C-5
ABOUT: 178 R127 R171 R157 P9
TEXT: I assume an effect does not remember which preset it was set from: its header names a preset whenever its settings equal one. An effect you set by hand to exactly the values of Blue reads "Blue".
WHY: L52 says the name goes on a change; it does not say whether the effect remembers the preset. Comparing gives all he asked and needs nothing stored in the show.
ALT: b) The effect remembers the preset it was last set from and names only that one.
IF-WRONG: SMALL one stored name per effect is added later without changing what he sees in nearly every case.
ASK: NO internal; the two ways look the same except for an effect set by hand to a preset's exact values.
@@END
@@ASSUME C-6
ABOUT: 178 P6
TEXT: I assume that while the Save button stands in place of the preset's name, a small arrow stays beside it, so that you can still open the list and pick another preset.
WHY: L52 replaces the name with the Save button; the name was also the button that opens the list of presets (his 169, BD:1089).
ALT: b) While Save shows, the list is not reachable until you save or put the slider back.
IF-WRONG: SMALL one small button in the header line.
ASK: LINE a choice of Harmony's about what stays reachable; he can strike it.
@@END
@@ASSUME C-7
ABOUT: 192 178
TEXT: I assume that after you load a preset onto an effect that keeps a signal you plugged in yourself, the header shows the Save button and not the preset's name, because the effect is not exactly that preset.
WHY: L53 (192 b) lets a signal of his stay; L52 shows the name only for an unchanged preset; together they leave this case open.
ALT: b) The header reads the preset's name until you move something yourself.
IF-WRONG: SMALL the test for "changed" ignores signals the preset does not hold.
ASK: LINE he would see it on every such load; easy to turn.
@@END
@@ASSUME C-8
ABOUT: 192 R155
TEXT: I assume picking "Default" puts the sliders back to the effect's defaults and leaves plugged-in signals where they are, the same way a preset does; you unplug a signal by hand.
WHY: L53 (192 b) speaks of a preset saved without a signal; "Default" is the same case but he does not name it.
ALT: b) "Default" also unplugs every signal: the effect is as if freshly dropped.
IF-WRONG: SMALL one switch in the load.
ASK: LINE a real choice; b) is as easy to argue.
@@END
@@ASSUME C-9
ABOUT: R154
TEXT: I assume the double-click in the Effects tab only shows the effect's rows and values under the list, to look at. It shows no picture in the preview, and presets are still made and changed only on an effect in the show.
WHY: L41 says "display the effects properties like resolume does"; it does not say whether the rows can be moved or saved there, or whether a picture is previewed. His 131 (BD:988-992) has presets changed in the show.
ALT: b) The sliders can be moved and saved as a preset right there. c) The effect is also shown on the preview monitor over what is playing, as Resolume does.
IF-WRONG: SMALL the panel gains live sliders or a preview later.
ASK: LINE he may want Resolume's preview of an effect; one line lets him say so.
@@END
@@ASSUME C-10
ABOUT: R136
TEXT: I assume there is never a bar that says "Effects" in the clip, layer or Global tab, also when effects are in it: the effects' own header bars are all you see.
WHY: L40 opens with "if there are no effects" but goes on "Each effect has its own header bar and that's it".
ALT: b) The bar is hidden only while the stack is empty and comes back with the first effect.
IF-WRONG: SMALL one bar drawn or not.
ASK: NO how it looks; the redesign settles it (L8).
@@END
@@ASSUME C-11
ABOUT: R157 R180 192
TEXT: I assume a preset also keeps, for each slider with a signal, that slider's own settings for the signal: its gain, its smoothing, one shot or looping, and the range it moves in.
WHY: L106 moves one shot, looping, gain and falloff from the signal to the slider that uses it; his 102 b (BD:926-927) predates that.
ALT: b) A preset keeps only which signal is plugged in; the slider's settings for it start at their defaults.
IF-WRONG: SMALL more fields in the preset file.
ASK: NO follows from his own words that a preset is the effect as he set it up.
@@END
@@ASSUME C-12
ABOUT: R180
TEXT: I assume that when a preset names a signal the open show does not have, the slider gets the preset's value and no signal is plugged in.
WHY: His "yes" to presets keeping a signal by name only (L51), with L104 (any number of envelopes of his own), leaves the missing name open.
ALT: b) The preset carries a copy of the signal's shape and makes that signal in the show. c) A small message says which signal is missing.
IF-WRONG: STAGE a preset he expects to pulse would stand still in a show that lacks the signal.
ASK: LINE he agreed to "by name only"; this is the edge of it, and b) is a bigger build.
@@END
@@ASSUME C-13
ABOUT: R180
TEXT: I assume the layer's transparency slider sets how strongly a cell with only effects changes the layers below it, and that such a cell on the bottom layer shows nothing at all.
WHY: L51 says it works "similar to a layer effect" on the layers below and is "not effective" at the bottom; it does not name the slider.
ALT: b) The effects always act at full strength and the slider does nothing for such a cell.
IF-WRONG: SMALL one multiplication in the drawing.
ASK: NO it follows his "similar to a layer effect" and is how Resolume's effect clips behave.
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
WHY: L50 names the two places, not the moment each is written; his "one Save" (BD:873-881) writes the show only on Save.
ALT: b) A new preset reaches the computer's presets only when the show is saved.
IF-WRONG: SMALL the moment of one file write.
ASK: NO technical.
@@END

## NAMES
@@NAME preset
MEANS: One saved setup of one effect: each slider's value, Dry / Wet and the signals plugged into its sliders.
SOURCE: his words, BD:1014 ("138 look is an effect preset. change the name look to preset to avoid further confusion."); used by him in L48, L50, L52
@@END
@@NAME Effects
MEANS: The tab of the browser that lists every effect and, under each effect, its presets.
SOURCE: Harmony's pick from his words "effect tab" (L41) and "the effects tab" (BD:1089); the on-screen name now is "FX", which is replaced by "Effects"
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
MEANS: The third place an effect or preset can be dropped, beside a clip and a layer: the stack that acts on the whole picture (was "Composition").
SOURCE: his words L55 ("lets move to global as that is what musicians are more used to"); the on-screen name now is "Composition", which is replaced by "Global"
@@END
@@NAME effects-only clip
MEANS: A cell that holds one or more effects and no picture; it works like a layer effect on the layers below its own.
SOURCE: Harmony's pick for his words L51 ("a cell with effect(s) only")
@@END

## CONFLICTS
- Where presets live: "these are saved with the app. always." (BD:894-895) against "R157 they are kept with the app and the show file. Very little storage overhead" (L50) and L48. Taken: L50 and L48, the later words on the same point. Not asked again; what they leave open is C-1 to C-4.
- The name after a change: "150 a is good, but as soon as the preset is changed, we have a little button to save this preset" (BD:1080; option A = the name stays) and "It can be called look 2 but no need to show that it was changed." (BD:994-995) against "If the preset gets changed then remove the preset name and replace it with save button" (L52). Taken: L52, his answer to the question that was asked to settle exactly this.
- A preset and his own signal: "If this is a clip in the show, then ctrl-z brings it back. There is no other way." (BD:988-992, read by Harmony as: loading unplugs) against "192 b" (L53: his own signal stays). Taken: L53. Undo still brings back what a load changed.
- One word for two places: "the effects tab for a clip layer or Global" (L40) against "double click an effect in effect tab" (L41, his picture shows the list of all effects). Read as two places: the effects part of the clip, layer and Global tabs, and the Effects tab of the browser. For the list of names (L114): only the browser tab is called "Effects".

## NOT DONE / UNSURE
- His L66 ("4- please explain in more detail about loading a preset onto an effect with an action playing") is a question back that belongs to topic D (actions); it is not answered here. What this paper gives topic D: a load is one step that sets values and plugs signals (192); the "changed" test compares set values, not the moving value (178, rule 2).
- That a recording keeps a preset load as a jump on every row of the effect (his 169, BD:1089) is topic E; not restated here.
- Whether Resolume itself marks the preset in use in its drop-down, what its name window starts with, and what a double-click on a preset does to its preview monitor: not known (facts-resolume-emulate A2.2, A2.3, A5.8). Not needed: his words L52, L22 and L41 decide these points; C-9 lets him add the preview.
- "Equal within 0.0005" in 178 rule (2) is taken from the unadopted ruling (facts-presets-delta HD-25); a technical number, to be checked when built.
- A hidden identity and a changed-last stamp per preset (needed for C-3 and C-4) are new and designed nowhere yet; the cheapest way to settle C-2 and C-3 is his answer to those two questions before the store is planned.
- All four CONFLICTS bullets are a later line of his replacing an earlier one on the same point (or one word used for two places); none is left for him to choose between, so no question is asked on them.
