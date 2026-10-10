# APPLY J -- The keyboard and MIDI mapping, menus and messages (s-rta-1009, page 2)

## SUMMARY
- The screen where a show recording is watched and actions are made is called STUDIO (BF245, BF263); the old word goes everywhere, with its long form.
- A MIDI knob on a clip's slider belongs to the CELL, as a pad does: it moves that slider of whatever clip sits in that cell of the deck on screen (244 b, BF262). It no longer travels with the clip.
- Both kinds of knob must work: one that sends its position and an endless one that sends steps (270 b, BF270). His earlier "207 c" took the endless setting away; it returns, as a toggle per knob only "if we have to".
- The hold setting (246) is OPEN: it waits on the research he ordered (BF264). Until he rules, a key or pad only presses.
- 269 (a mapping file is the whole mapping; an import replaces after asking) and 271 (an output's on / off never goes on a key or pad) are accepted as written.
- One question is left for him from this topic: whether a knob that sends its position may make a slider jump (it matters more now that a knob serves whatever clip sits in a cell). Three lines he can strike.

## ITEMS
@@ITEM 244
TITLE: A MIDI knob on a clip's slider belongs to the cell
STATUS: ANSWERED way b
HIS: BF262 "b"
RULE: A knob or fader of a MIDI controller that is put on a slider of a clip belongs to the cell (its layer and its column), not to the clip: it moves that slider of whatever clip sits in that cell of the deck on screen. So: (1) Switching the grid to another deck makes the knob serve the clip that sits in the same cell of the new deck; a clip of the old deck that is still playing on the layer is no longer moved by it. (2) A clip that is moved, cut or dragged out of the cell leaves the knob behind; a clip that is pasted, dropped or dragged into the cell is served by it from then on; a copy of the clip in another cell has no knob. (3) "That slider" is the same slider by name: the clip's own control of that name (for example its transparency or its speed), or the slider of that name on the effect of that name (J3-1). A clip in the cell that has no such slider is not moved, and nothing else is moved in its place. (4) The knob moves the slider whether the clip in the cell is playing or not, as a drag of the slider does (item 206). (5) With an empty cell there, or with a deck on screen that has no such column, the knob does nothing (J3-4). (6) The same holds for the Gain, Falloff and Threshold of a signal user on a clip's slider, which item 206 counts as sliders. (7) A knob on a layer's slider or on a Global slider stays with that layer or with Global, as the item says; way b does not speak of it. (8) A key or pad on a button of a clip that is not an action's button follows the cell in the same way (J3-3); a key or pad on an action's button stays with the action, with @@ITEM R199 (a) as it stands. A clip's pad means the cell on the deck on screen, with @@ITEM 197 (topic F) as it stands; there is no choice per entry between cell and clip (@@ITEM 207).
CHANGED: the item's first sentence (the knob stays with the clip wherever it goes; a copy has no knob) is replaced by way b. Its second sentence (layer and Global) is kept: way b speaks of the clip's knob only (INFERRED that he leaves it). Named by this paper because a builder would have to guess: what "that slider" is on another clip (J3-1), the deck switch (J3-2), buttons of a clip (J3-3), the empty cell (J3-4).
TODAY: no knob can be put on a clip's slider at all: the mapping screen has a fixed list of boxes and only Master Opacity and Master Signal are sliders (area-controls TODAY 10, 11). The model knows ByPosition / ThisItem / Selected per entry (Binding.h:72-79, read, not run) and no screen sets it; a mapped clip box fires the cell of the deck that is shown (area-controls TODAY 16). To build: the address of a clip-level control is (layer, column, effect name, slider name) resolved against the deck on screen at each turn.
@@END

@@ITEM 245
TITLE: The screen is called Studio
STATUS: ANSWERED way b
HIS: BF263 "b"; BF245 "Let's go with studio. That's perfect."
RULE: The screen where a show recording is watched, mended, recorded over and cut into actions is called "Studio": in talk with him, on screen, in every menu, tooltip and message, in the list of names and later in the manual. The word it had before, and its long form, are read nowhere any more (J3-5). Everything that was named after the old word takes the new one: the picture at its top, its timeline, its tracks, its In and Out points (the list of names decides each spelling; the names it must bring up to date are listed under REACHES OTHER TOPICS). The screen he performs on keeps its name, the live window. What Studio does is topic E's.
CHANGED: the item as he read it (the name stays until he picks another) is replaced by way b, in his own words too (BF245). In BF241 he still says "the Recording Review" for the same screen, in the box above the one in which he chose: read as Studio (INFERRED).
TODAY: the screen does not exist in the app; no name of it is on any screen. The word is in .harmony/NAMES.md (26 lines, listed below), in the twelve spec files and in the plans.
@@END

@@ITEM 246
TITLE: A hold setting for a key or pad
STATUS: OPEN waits on the research he ordered
HIS: BF264 "do some research online and figure out what people are doing"
RULE: Waits on the research he ordered: "I want you to do some research online and figure out what people are doing and if this is worth doing? We need to only match what Dj or bands are doing and also look at Dj/producers." (BF264). The research runs beside this paper, and its answer is written by another seat, not here. What will be put to him with that answer is the item's two ways: (a) no hold setting, a key or pad only presses; (b) every key and pad gets one switch, press or hold, set to press until he changes it. His test for it is his own: the app only has to match what DJs, bands and DJ/producers do. Until he rules, his answer of 2026-10-07 stands and nothing is planned for hold: a key or pad only presses (with @@ITEM 207 and @@ITEM D34 as they stand for that part).
CHANGED: he neither confirms way a nor takes way b: he asks for research first and names the test. The argument on the page (Harmony's) is no longer the ground for the decision.
TODAY: the model has Toggle / Momentary per entry and no screen sets it (area-controls TODAY 15, 18; U5). If hold is taken: a release that is lost leaves the pad on (K3), and a held pad's release is not kept in a recording (K3, K8; @@ITEM D34).
@@END

@@ITEM 269
TITLE: A mapping file holds the whole mapping; import replaces
STATUS: ACCEPTED
HIS: box left empty = accepted as written
RULE: A mapping file holds the whole keyboard and MIDI mapping of the open show, keys, pads, knobs and faders together, with each knob's kind (item 270), and nothing else. "Export Mapping..." writes it. "Import Mapping..." reads one and replaces the open show's whole mapping, after asking; it never adds to the mapping that is there. With @@ITEM R199 (f) and @@ITEM R148 as they stand.
CHANGED: nothing: accepted as written. Added only that the file carries each knob's kind, which item 270 brings back.
TODAY: Export / Import Bindings files in ~/Library/Audio-DNA/bindings are the only way a mapping is kept; the show file's "keys" block is written empty (area-controls TODAY 12-14, 19-22).
@@END

@@ITEM 270
TITLE: Both kinds of MIDI knob must work
STATUS: ANSWERED way b, with words of his own
HIS: BF270 "We need to work with both"; "set a toggle if it's an endless encoder"
RULE: His words: "b due to what we're doing endless will be better, but we can assume that people will use one or the other. We need to work with both and know how they both work so they both work smoothly. If we have to, we could ask the user to set a toggle if it's an endless encoder." (BF270). So two kinds of knob work in the keyboard and MIDI mapping, on every slider that can be mapped. (1) A knob or fader that sends its position: the slider goes to the position the knob sends. (2) An endless knob that sends steps: each step moves the slider a small amount up or down from where the slider stands at that moment, so the first turn never jumps, whatever moved the slider before (his hand, an action, a signal, a preset, another clip in the cell); at either end of the slider it stops, and it comes back at once when turned the other way. How far one step moves: J3-7. (3) Which kind a knob is: the app works it out by itself when the knob is put in the mapping, from what the knob sends while he turns it; and the knob's entry in the mapping carries one toggle, endless or not, that he can set himself where the app is wrong (J3-6; his words allow the toggle only "if we have to"). (4) The kind is kept per knob, saved with the show's mapping and in a mapping file (item 269). (5) When the app takes a slider back from a knob is @@ITEM R199 (e) as amended below; what a position-sending knob does when it and the slider differ: J3-8. How hard a pad is hit still changes nothing (@@ITEM 207).
CHANGED: the item as he read it (knobs must send their position; a step-sending knob will not work) is replaced by way b. His "207 c" of 2026-10-07 had removed the endless-knob setting: his newer words bring it back (CONFLICTS). The old @@ITEM D33 returns in its first form: an endless knob starts from where the slider stands. He did not name a controller: the question "Which MIDI controller do you use?" stays unanswered, and by his words the app does not depend on the answer.
TODAY: read, not run: the model has Absolute / Relative per entry with a step of 0.01 (Binding.h:62-68); no screen sets it (area-controls TODAY 15). The relative path reads one way of sending steps only (above 64 = up, below 64 = down, BindingManager.cpp:168-183) and keeps its own count per knob that starts at 0.5, not at the slider's value, so the first turn jumps (BindingManager.cpp:173; area-controls K1). To change: start from the target's value at every turn; read the other ways controllers send steps (Harmony's knowledge of MIDI, not checked against any controller: there are several, and a knob's messages alone may not tell which one, hence the toggle); a detection at mapping time; the toggle on the mapping screen; how an endless knob's turn enters a recording (K8, topic E).
@@END

@@ITEM 271
TITLE: An output's on / off never goes on a key or pad
STATUS: ACCEPTED
HIS: box left empty = accepted as written
RULE: Switching an output screen on or off cannot be put on a key, a pad, a knob or a fader, so that no stray hit blanks a projector. This covers each output's own on / off and the two commands that switch outputs, All Outputs Off and Restore Last Outputs, and Syphon's on / off (J3-9). The outputs keep their menu and the app's own Cmd keys for them (@@ITEM R227 (a)). The output screens' settings stay out of the mapping as well, with @@ITEM 206 as it stands. The picture can still be dimmed from a fader: the Master is a slider and can be mapped.
CHANGED: nothing: accepted as written. That the two commands and Syphon count as "switching an output on or off" is Harmony's reading of the item (J3-9).
TODAY: no output command is a mapping target; Cmd+Shift+Esc = all outputs off (CLAUDE.md "Outputs"; docs/claude/integration.md, not re-read).
@@END

## AMENDMENTS
@@AMEND 206 1
OLD: What a knob on a clip's slider belongs to: J-8.
NEW: A knob on a clip's slider belongs to the cell: it moves that slider of whatever clip sits in that cell of the deck on screen (page-2 item 244, way b); a knob on a layer's or a Global slider stays there.
HIS: BF262 "b"
WHY: The rule pointed at an open assumption (the knob stays with the clip); he chose the cell.
@@END

@@AMEND 206 2
OLD: Snapshot, for as long as that command stays (his "Do we need snapshot?", L94, is answered by topic F)
NEW: Snapshot, which stays and can be put on a key or pad ("the user can push a shortcut button and save that to look out later", BF243; what a Snapshot saves is topic F's)
HIS: BF243 "the user can push a shortcut button and save that"; BF257 "We keep the safe snapshot somewhere."
WHY: The entry was kept only for as long as the command stays; he keeps it and names a shortcut for it himself.
@@END

@@AMEND 206 3
OLD: Whether the mapping works while Review is open: topic E (E-2).
NEW: Whether the mapping works while Studio is open: topic E (E-2).
HIS: BF245 "Let's go with studio. That's perfect."
WHY: The screen's name was the old word; he chose Studio.
@@END

@@AMEND 207 1
OLD: none of the four settings exists; a key or a pad only presses, and a knob only turns
NEW: of the four settings one returns by his later words: the kind of knob, endless or not (page-2 item 270, BF270). Hit strength and a choice per entry between cell and clip stay out. The hold setting waits on the research he ordered (page-2 item 246, BF264); until he rules, a key or a pad only presses
HIS: BF270 "We need to work with both"; BF264 "do some research online"
WHY: It said that none of the four settings exists; his newer words bring back the endless knob and put the hold setting on hold.
@@END

@@AMEND 207 2
OLD: There is no endless-knob setting: a knob sets the slider to the position the knob sends (what that means for his controller: J-4).
NEW: Both kinds of knob work: a knob that sends its position sets the slider to that position, and an endless knob that sends steps moves the slider from where it stands; the app tells the two apart by itself where it can, and the knob's entry carries one toggle, endless or not (page-2 item 270).
HIS: BF270 "We need to work with both and know how they both work"
WHY: It said there is no endless-knob setting and that a step-sending knob does not work; he says both must work.
@@END

@@AMEND 207 3
OLD: There is no "this cell / this clip" choice per entry: a clip's pad means the cell on the deck on screen (question 197 at its default).
NEW: There is no "this cell / this clip" choice per entry: a clip's pad means the cell on the deck on screen (question 197 at its default), and a knob on a clip's slider means the cell in the same way (page-2 item 244, way b).
HIS: BF262 "b"
WHY: It ruled the pad only; his answer to 244 gives the knob on a clip's slider the same meaning.
@@END

@@AMEND 207 4
OLD: The mapping screen therefore shows no settings per entry.
NEW: The mapping screen therefore shows one setting, on a knob's entry only: endless or not (page-2 item 270). Whether a key's or pad's entry gets a press / hold switch waits on page-2 item 246.
HIS: BF270 "set a toggle if it's an endless encoder"; BF264 "if this is worth doing?"
WHY: It said the mapping screen shows no settings per entry; the toggle for an endless knob is one.
@@END

@@AMEND 207 5
OLD: His question back is answered under QUESTIONS BACK (207-hold); the rule stays c unless he then asks for hold.
NEW: His question back was answered on page 2; he then ordered research (BF264). For the hold setting the rule stays c until he rules on that research (page-2 item 246).
HIS: BF264 "I want you to do some research online"
WHY: It made the hold setting depend on his reply to Harmony's argument; he asked for research instead.
@@END

@@AMEND 209 1
OLD: inside Review
NEW: inside Studio
HIS: BF245 "Let's go with studio. That's perfect."
WHY: The screen's name was the old word; he chose Studio.
@@END

@@AMEND R197 1
OLD: The screen where a show recording is watched is "Review", short for "Show Recording Review", until he answers the name question he asked (the answer review-name).
NEW: The screen where a show recording is watched and actions are made is "Studio": "Let's go with studio. That's perfect." (BF245; page-2 item 245, way b). The words "Review" and "Show Recording Review" are read nowhere for it.
HIS: BF245 "Let's go with studio. That's perfect."; BF263 "b"
WHY: The name was kept until he answered the name question; he answered it.
@@END

@@AMEND R197 2
OLD: The rows of Review are "tracks" (L86, L89).
NEW: The rows of Studio are "tracks" (L86, L89).
HIS: BF245 "Let's go with studio. That's perfect."
WHY: The screen's name was the old word; he chose Studio.
@@END

@@AMEND R199 1
OLD: The knob and the slider can then differ, and the slider jumps to the knob at his next turn.
NEW: A knob that sends its position and the slider can then differ, and the slider jumps to the knob at his next turn (put to him once more: J3-8). An endless knob never differs from the slider: its next turn goes on from where the slider stands (page-2 item 270).
HIS: BF270 "so they both work smoothly"
WHY: It knew one kind of knob only; with endless knobs back, the jump holds for the position-sending kind alone.
@@END

@@AMEND R199 2
OLD: Whether and how the mapping works while Review is open is topic E's
NEW: Whether and how the mapping works while Studio is open is topic E's
HIS: BF245 "Let's go with studio. That's perfect."
WHY: The screen's name was the old word; he chose Studio.
@@END

@@AMEND R227 1
OLD: while Review is open the spacebar is Review's own and stops and starts the playback
NEW: while Studio is open the spacebar is Studio's own and stops and starts the playback
HIS: BF245 "Let's go with studio. That's perfect."
WHY: The screen's name was the old word; he chose Studio.
@@END

@@AMEND D33 1
OLD: There is no endless-knob setting (207 c).
NEW: An endless knob that sends steps works too (page-2 item 270): the app tells the kind by itself where it can, and the knob's entry carries one toggle, endless or not.
HIS: BF270 "We need to work with both"
WHY: It said there is no endless-knob setting; he says both kinds must work.
@@END

@@AMEND D33 2
OLD: a knob that sends only steps (one up, one down) instead of a position does not work in this build (J-4; which kind his controller has is not known). If he asks for such knobs later, this item returns: the first turn starts from where the slider stands.
NEW: a knob that sends only steps (one up, one down) instead of a position works as well, and its first turn, like every later one, starts from where the slider stands: no jump.
HIS: BF270 "know how they both work so they both work smoothly"
WHY: It left step-sending knobs out of the build and promised this item's return if he asked for them; he asked.
@@END

@@AMEND P33 1
OLD: show no settings per entry (207 c)
NEW: show one setting, on a knob's entry only: endless or not (page-2 item 270); a press / hold switch on a key's or pad's entry waits on page-2 item 246
HIS: BF270 "set a toggle if it's an endless encoder"; BF264 "if this is worth doing?"
WHY: It said the mapping screen shows no settings per entry; the endless toggle is one.
@@END

@@AMEND P34 1
OLD: the marks of L80 inside Review
NEW: the marks of L80 inside Studio
HIS: BF245 "Let's go with studio. That's perfect."
WHY: The screen's name was the old word; he chose Studio.
@@END

@@AMEND R224 1
OLD: he makes a new one from the recording in the review screen and deletes the old one
NEW: he makes a new one from the recording in Studio and deletes the old one
HIS: BF245 "Let's go with studio. That's perfect."
WHY: The screen's name was the old word (R224 is named under MADE FROM of item 245); he chose Studio.
@@END

## ASSUMPTIONS
@@ASSUME J3-1
ABOUT: 244, 206
TEXT: I assume the knob finds its slider by name: the same slider of the same effect on whatever clip sits in the cell. If that clip does not have that effect, the knob does nothing.
WHY: BF262 "b" says "that slider of whatever clip sits there"; which slider that is on a clip with other effects is not said.
ALT: b) By place: the knob moves, for example, the second slider of the first effect, whatever that effect is.
IF-WRONG: STAGE a knob would move an unexpected slider of another clip, or none
ASK: LINE a real choice; by name is the safe one and he will most likely wave it through
@@END

@@ASSUME J3-2
ABOUT: 244
TEXT: I assume this also holds after you switch decks: the knob then moves the slider of the clip in that cell of the new deck, not of a clip from the old deck that is still playing.
WHY: BF262 "b": the way he chose says "in the deck on screen"; way c (the clip playing on the layer) was beside it and he did not take it.
ALT: b) While a clip of that cell from another deck is still playing, the knob stays with it.
IF-WRONG: STAGE he would turn a knob and the playing clip would not answer
ASK: NO it is the wording he chose, with the other way on the same card; his pads behave the same
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
TEXT: I assume the app works out by itself which kind a knob is when you map it, and every knob in the mapping also has an Endless toggle that you can set yourself if the app got it wrong.
WHY: BF270 "If we have to, we could ask the user to set a toggle": whether we have to is technical; a knob's messages may not always tell the kind.
ALT: b) No toggle is shown unless the app cannot tell. c) No guessing: you always set the toggle yourself.
IF-WRONG: SMALL it shows while he maps the knob, not in a show
ASK: LINE he allowed the toggle only "if we have to"; this shows it always, as a safety net
@@END

@@ASSUME J3-7
ABOUT: 270, D33
TEXT: I assume an endless knob moves a slider from end to end in about as much turning as an ordinary knob needs, and a faster turn moves it farther where the controller sends that.
WHY: BF270 "so they both work smoothly" gives no number for how far one step moves a slider.
ALT: b) Finer: several full turns from end to end, for slow, exact moves.
IF-WRONG: SMALL one number, tuned with his controller
ASK: NO a number to tune once a controller is on the table
@@END

@@ASSUME J3-8
ABOUT: 270, 244, R199
TEXT: I assume a knob that sends its position makes the slider jump to the knob's position at your first turn, as you accepted before. Only an endless knob carries on from where the slider stands.
WHY: BF270 "so they both work smoothly" may ask for no jump; the jump was on the last page and he left it. With BF262 a knob serves whatever clip sits in a cell, so the two differ often.
ALT: b) No jump: the slider waits until the knob passes the slider's value and follows from there; until then the knob does nothing. c) No jump: the slider moves from where it stands, faster or slower than the knob, until the two meet.
IF-WRONG: STAGE a slider that jumps is seen on the output; a knob that waits feels dead for a moment
ASK: YES "smoothly" can be read both ways, and the audience sees a jump
@@END

@@ASSUME J3-9
ABOUT: 271, 206
TEXT: I assume All Outputs Off, Restore Last Outputs and Syphon's on / off count as switching an output: none of them can be put on a key or pad either.
WHY: Item 271, accepted as written, says "switching an output screen on or off"; it does not name the two commands or Syphon.
ALT: b) All Outputs Off may go on a key or pad, as a panic button.
IF-WRONG: SMALL one entry more or less in the mapping; the Cmd key for all outputs off stays
ASK: NO the item's own reason (no stray hit blanks a projector) covers all three
@@END

@@ASSUME J3-10
ABOUT: 206, D32
TEXT: I assume a pad or key on a cell does all that a click on the cell does: it triggers the cell and selects it, so the Clip tab shows that clip.
WHY: BF255 "you can select the cell which triggers it and selects it" is said of the mouse; whether a pad selects too is not said.
ALT: b) A pad only triggers: what is selected, and what the Clip tab shows, stays as it was.
IF-WRONG: SMALL the Clip tab would follow, or not follow, the pads
ASK: NO follows from "a pad does what a click does", which he accepted; small to change
@@END

@@ASSUME J-1
ABOUT: R197, 206
TEXT: I assume "All else is gone" is about names only. Record Show records the moves and the sound, with the low-resolution show recording beside it; a film in full quality is rendered afterwards in Studio and is not a box of Record Show.
WHY: BF241 "If we want a very hd recording, we can make an HD render from the Recording Review" and BF253 point away from a full-size film box; his earlier "48 b" (binding-decisions.md 867) gave one.
ALT: b) Record Show keeps a Video box that films the show in full size while it is recorded.
IF-WRONG: SMALL a film box is small to add or to remove
ASK: LINE topic E owns the boxes of Record Show: show it once, with topic E's line on the same words
@@END

## QUESTIONS BACK
(none is owed by this topic's paper. His question in box 246, BF264, is a task for Harmony: three researchers look online in this same run and a separate seat writes that answer.)

## NAMES
@@NAME Studio
MEANS: The screen where a show recording is watched, mended, recorded over and cut into actions.
SOURCE: his words (BF245: "Let's go with studio. That's perfect."; BF263: "b"); replaces the name Review (long form Show Recording Review) in /Users/boriskarpman/projects/RealTimeAudio/.harmony/NAMES.md, row at line 203
@@END

@@NAME Endless
MEANS: The toggle on a knob's entry in the keyboard and MIDI mapping that says the knob is an endless one that sends steps, not its position.
SOURCE: Harmony's pick, from his words (BF270: "set a toggle if it's an endless encoder")
@@END

## REACHES OTHER TOPICS
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

## CONFLICTS
- Endless knobs. New (BF270): "We need to work with both and know how they both work so they both work smoothly." Old (binding-decisions.md 1191, BF232): "207 c but make an argument for why we should have a key/pad hold setting." -- way c of 207 read: no setting at all, a knob only turns, so no endless-knob setting. The new words win: the endless knob returns; the rest of c stands (hit strength, cell or clip per entry), and hold waits on the research.
- The screen's name. New (BF245): "Let's go with studio. That's perfect." Old (binding-decisions.md 1187, BF228): "Show Recording Review should be called Review for short is a good name. Do you have a better name for this?" The new word wins; he had asked for a better one himself.
- Not a conflict, said for completeness: the jump of a slider to a position-sending knob (R199 e, on the last page, left by him) against BF270 "so they both work smoothly". Put to him as the one question J3-8.

## NOT DONE / UNSURE
- Item 246 carries no answer and no assumption on purpose: the research he ordered (BF264) is written by another seat. If he then takes hold: @@ITEM 207 (its "no hold-to-play" sentence), @@ITEM D34 (returns as written), @@ITEM P33 and R199 (c) need amendments, and a hold on a BPM mode clip that ends before the 1 needs its rule.
- Which controller he uses is still not known (270's question was not answered). By BF270 the app must not depend on it; it would still settle the tuning of J3-7, the ways of sending steps that must be read first, and pad lights (@@ITEM 214, D35). Cheapest: he names it when the build of the mapping starts.
- How endless knobs send their steps is Harmony's knowledge of MIDI, not checked: several ways exist, and whether a knob's messages alone always tell the kind is not known. Cheapest: a Researcher reads the MIDI notes of three common controllers; it decides whether the Endless toggle is a safety net (J3-6) or the only way.
- J3-2 is ASK NO because he chose the wording himself with the other way on the same card. It is still the consequence of 244 b most likely to surprise on stage (after a deck switch the knob leaves the playing clip): the ruling seat may lift it to a LINE.
- The old word inside the other spec files (E above all) was not amended here, by the rule on other topics' blocks; only R224 was, as a MADE FROM block of item 245. If the merge needs every "Review" replaced block by block, that is one mechanical pass by the seat that owns each file.
- TODAY lines: 270's from BindingManager.cpp:155-198 and Binding.h:62-79 (read, not run); the others from the old blocks and area-controls, not re-read.
- Written 2026-10-09 18:53:11 EDT (from date).
