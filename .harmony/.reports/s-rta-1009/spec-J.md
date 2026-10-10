# SPEC J -- The keyboard and MIDI mapping, menus and messages (s-rta-1009): the s-rta-1007 spec with Boris's answers to page 2 laid over it by wf/merge3.py (paper apply-J.md, ruling rule-J.md). This file wins over every older one.

## PAGE 2 ITEMS (the rule now for each item of page 2 in this topic)
@@ITEM 244
TITLE: A MIDI knob on a clip's slider belongs to the cell
STATUS: ANSWERED way b
HIS: BF262 "b"
RULE: A knob or fader of a MIDI controller that is put on a slider of a clip belongs to the cell (its layer and its column), not to the clip: it moves that slider of whatever clip sits in that cell of the deck on screen. So: (1) Switching the grid to another deck makes the knob serve the clip that sits in the same cell of the new deck, playing or not; a clip of the old deck that is still playing on the layer is no longer moved by it (J3-2). (2) A clip that is moved, cut or dragged out of the cell leaves the knob behind; a clip that is pasted, dropped or dragged into the cell is served by it from then on; a copy of the clip in another cell has no knob. (3) "That slider" is the same slider, found by its name (J3-1): the clip's own control of that name (for example its Speed), or the slider of that name on the effect of that name. Where the clip carries that effect more than once, the knob means the one in the same place among the effects of that name (a knob put on the second Blur moves the second Blur); a slider of a source is found by the source's name and the slider's name; a macro knob of the clip by its number (J3-11). A clip in the cell that has no such slider is not moved, and nothing else is moved in its place. (4) The knob moves the slider whether the clip in the cell is playing or not, as a drag of the slider does (@@ITEM 206). (5) With an empty cell there, or with a deck on screen that has no such column, the knob does nothing (J3-4). (6) The same holds for the Gain, Falloff and Threshold of a signal user on a clip's slider, which @@ITEM 206 counts as sliders. (7) A knob on a layer's slider or on a Global slider stays with that layer or with Global, as the item says; way b does not speak of it. (8) A key or pad on a button of a clip that is not an action's button follows the cell in the same way (J3-3); a key or pad on an action's button stays with the action, with @@ITEM R199 (a) as it stands. A clip's pad means the cell on the deck on screen, with @@ITEM 197 (topic F) as it stands; there is no choice per entry between cell and clip (@@ITEM 207). (9) Both kinds of knob follow this rule alike (page item 270). In Studio a knob finds its cell in the deck the show recording has on screen: topic E's rule (its page item 257).
CHANGED: the item's first sentence (the knob stays with the clip wherever it goes; a copy has no knob) is replaced by way b. Its second sentence (layer and Global) is kept: way b speaks of the clip's knob only (INFERRED that he leaves it). Named by the paper and by the ruling because a builder would have to guess, none of it his words: what "that slider" is on another clip (J3-1); the same effect twice, a source's slider and a macro knob (J3-11); the deck switch (J3-2: the letter of the way he chose, put to him as one line); buttons of a clip (J3-3); the empty cell and the clip that is not playing (J3-4).
TODAY: no knob can be put on a clip's slider at all: the mapping screen has a fixed list of boxes and only Master Opacity and Master Signal are sliders (area-controls TODAY 10, 11). The model knows ByPosition / ThisItem / Selected per entry (src/binding/Binding.h:70-79, read, not run) and no screen sets it; a mapped clip box fires the cell of the deck that is shown (area-controls TODAY 16). To build: the address of a clip-level control is (layer, column, effect name and its place among the effects of that name, slider name), resolved against the deck on screen at each turn.
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
TITLE: A pad or a key: press only, or press or hold
STATUS: OPEN waits on his answer to J3-hold (a, b or c)
HIS: BF264 -- "I want you to do some research online and figure out what people are doing and if this is worth doing? We need to only match what Dj or bands are doing and also look at Dj/producers."
RULE: Waits on his answer: WAY A (his standing answer, no hold setting) -- a press on a clip's key or pad triggers the clip exactly as a click on its picture does (a Timeline mode clip at once, a BPM mode clip at the next 1) and letting go does nothing; a press on an effect's on / off button or on any other on / off button switches it over and it stays; a press on an action button switches the action on or off as a click does; a key on the computer keyboard does the same as a pad; no mapping entry carries a setting; a recording keeps each press and has no release to keep. WAY B (Harmony's recommendation) -- every mapped key or pad carries one switch, press or hold, starting on press; an entry on press behaves exactly as in way A; the switch belongs to the key or pad, not to the clip, so a mouse click always only presses. On hold: (1) a Timeline mode clip starts at once on the press and its layer is cleared at once on the release, as a click on an empty cell clears it; (2) a BPM mode clip waits for the next 1 on the press; if the pad is still held at the 1 it starts there and its layer is cleared AT ONCE on the release (Harmony's pick, not asked: at once, not at the next 1, because the length of the hold is the gesture -- live 12); if the pad is let go before the 1 nothing starts; (3) in both modes the release clears only if that clip is still the one playing or waiting on its layer -- if another clip was triggered there meanwhile the release does nothing; (4) an effect's on / off button, or any other on / off button, switches over on the press and switches back to what it was on the release; (5) an action goes on at the press and off at the release, its controls going back with the Global glide as for any action switched off; a play-once action that has already ended is not touched by the release; (6) a button that is a single act (tap tempo, Resync, Stop actions, BPM -, BPM +, BPM /2, BPM x2) and every knob or fader has no such switch; (7) a key on the computer keyboard works as a pad: key down is the press, key up is the release, the computer's own key repeat while a key is down counts as nothing, and a key still down when the app loses the keyboard counts as let go at that moment; on MIDI, note off or note on with velocity 0 is the release; (8) how hard a pad is hit changes nothing; (9) a recording keeps the release like the press, so a replay and an action made from it let go where he let go (item D34 returns as written). WAY C -- way B without points 1 to 3: the switch exists only on an entry whose target is an effect's on / off button, another on / off button or an action; a clip's key or pad always only presses.
CHANGED: Against old item 207 (ANSWERED c, with a question back): the rule is unchanged while he has not answered; what is new is that the question back is now answered with research (@@ANSWER 246-hold) and Harmony's recommendation moves from an argument ("I would add it") to a researched yes at medium confidence; way b is now spelled out to build from (points 1 to 9), and a smaller way c is added. Against old item D34 (REPLACED by 207 c): it stays replaced under way A; under way B or C it returns as written (point 9). The page-2 sentence "on a BPM mode clip a short hold can end before the 1, and then nothing shows" becomes point 2 of way B.
TODAY: Harmony's notes, read, not run: a hold-to-play setting ("Momentary") exists in the mapping model next to "Toggle", and no screen sets it; the only route is an edited exported file (area-controls TODAY 15: Binding.h:51-82; handlers MainComponent.cpp:7767-7830, BindingManager.cpp:161-198, 229-279; U5). When the file sets it: a key's release is found by polling every momentary key at any key-up; the release clears the clip, or cancels it if it was still queued (TODAY 18: MainComponent.cpp:4177-4205; O7, U6). A lost key-up (the app loses the keyboard) can leave the pad on (K3). The release is not kept in a take (TODAY 18; K3, K8; FQ:95). A key fires on every OS key press with no repeat filter, so a held clip key is INFERRED to fire again at the system repeat rate (TODAY 19: MainComponent.cpp:4158-4165). A Momentary binding's release stops a routine fired from its pad (O4, PC:40). Under way A all of this comes out of the model or stays unreachable; under way B or C the lost release, the repeat and the unrecorded release are mended with it.
@@END

@@ITEM 269
TITLE: A mapping file holds the whole mapping; import replaces
STATUS: ACCEPTED
HIS: box left empty = accepted as written
RULE: A mapping file holds the whole keyboard and MIDI mapping of the open show, keys, pads, knobs and faders together, with each knob's kind (page item 270), and nothing else. "Export Mapping..." writes it. "Import Mapping..." reads one and replaces the open show's whole mapping, after asking; it never adds to the mapping that is there. With @@ITEM R199 (f) and @@ITEM R148 as they stand.
CHANGED: nothing in what he accepted. Not in the sentence he read, carried from the old assumption J-2 and Harmony's own: "keys, pads, knobs and faders together" and "and nothing else" (no clip, effect or setting of the show travels in a mapping file). "It never adds" is only the other side of "replaces": the page's way b (an import that adds and overwrites only the keys that clash) stood beside it and he did not take it. Added by this round: the file carries each knob's kind (page item 270); if he takes a hold setting (page item 246), it carries each key's and pad's switch as well.
TODAY: Export / Import Bindings files in ~/Library/Audio-DNA/bindings are the only way a mapping is kept; the show file's "keys" block is written empty (area-controls TODAY 12-14, 19-22).
@@END

@@ITEM 270
TITLE: Both kinds of MIDI knob must work
STATUS: ANSWERED way b, with words of his own
HIS: BF270 "We need to work with both and know how they both work so they both work smoothly."; "If we have to, we could ask the user to set a toggle if it's an endless encoder."
RULE: His words: "b due to what we're doing endless will be better, but we can assume that people will use one or the other. We need to work with both and know how they both work so they both work smoothly. If we have to, we could ask the user to set a toggle if it's an endless encoder." (BF270). So two kinds of knob work in the keyboard and MIDI mapping, on every slider that can be mapped. (1) A knob or fader that sends its position: the slider goes to the position the knob sends. (2) An endless knob that sends steps: each step moves the slider a small amount up or down from where the slider stands at that moment, so the first turn never jumps, whatever moved the slider before (his hand, an action, a signal, a preset, another clip in the cell); at either end of the slider it stops, and it comes back at once when turned the other way. How far one step moves: J3-7. (3) Which kind a knob is: the knob's entry in the mapping carries one toggle, endless or not. When the knob is put in the mapping the app sets the toggle by itself, as well as it can tell from what the knob sends while he turns it; he sets it himself where the app is wrong or cannot tell. His words allow the toggle "If we have to"; that we have to is Harmony's judgement and not his word, and it is not checked: a knob's messages alone may not always tell the kind (J3-6). (4) The kind is kept per knob, saved with the show's mapping and in a mapping file (page item 269). (5) When the app takes a slider back from a knob is @@ITEM R199 (e) as amended; what a position-sending knob does when it and the slider differ: J3-8. (6) "know how they both work" is his order: before this part is built, the ways controllers send the steps of an endless knob are looked up, and the app reads every common one; which ways those are is not known here. How hard a pad is hit still changes nothing (@@ITEM 207).
CHANGED: the item as he read it (knobs must send their position; a step-sending knob will not work) is replaced by way b. His "207 c" of 2026-10-07 had removed the endless-knob setting: his newer words bring it back (CONFLICTS). The old @@ITEM D33 returns in its first form: an endless knob starts from where the slider stands. He did not name a controller: the question "Which MIDI controller do you use?" stays unanswered, and by his words ("people will use one or the other") the app does not depend on the answer. By the ruling: (3) no longer states as a fact that the app can tell the kind; (6) is added from his own words, which the paper had left as an optional step.
TODAY: read, not run: the model has Absolute / Relative per entry with a step of 0.01 (src/binding/Binding.h:60-68); no screen sets it (area-controls TODAY 15). The relative path reads one way of sending steps only (above 64 = up, below 64 = down, src/binding/BindingManager.cpp, processMidiCC) and keeps its own count per knob that starts at 0.5, not at the slider's value, so the first turn jumps (area-controls K1). To change: start from the target's value at every turn; read the other ways controllers send steps (Harmony's knowledge of MIDI, not checked against any controller: there are several, and a knob's messages alone may not tell which one, hence the toggle); a best guess at mapping time; the toggle on the mapping screen; how an endless knob's turn enters a recording (K8, topic E).
@@END

@@ITEM 271
TITLE: An output's on / off never goes on a key or pad
STATUS: ACCEPTED
HIS: box left empty = accepted as written
RULE: Switching an output screen on or off cannot be put on a key or a pad, so that no stray hit blanks a projector: an output screen's own on / off has no entry in the keyboard and MIDI mapping. Harmony's reading of the same sentence and of its reason, not his words (J3-9): the two commands that switch outputs, All Outputs Off and Restore Last Outputs, have no entry either; nor has the on / off of Syphon, which is an output like a screen and can be what feeds a projector through another program. The outputs keep their menu and the app's own Cmd keys for them (@@ITEM R227 (a)). The output screens' settings stay out of the mapping as well, with @@ITEM 206 as it stands. The picture can still be dimmed from a knob or a fader: the Master is a slider and can be mapped.
CHANGED: nothing in what he accepted. Harmony's reading, set apart in the RULE and carried by J3-9: the two commands (a stray hit on Restore Last Outputs would end the hold of All Outputs Off, topic G's page item 265, and switch on again the outputs that were on before) and Syphon (his earlier words: "syphon is an output and treated with same output settings as a screen", binding-decisions.md 834; the item's own reason holds for it, because a Syphon feed can be what feeds a projector through another program).
TODAY: no output command is a mapping target; Cmd+Shift+Esc = all outputs off (CLAUDE.md "Outputs"; docs/claude/integration.md, not re-read).
@@END

## ITEMS (the items of the first page; a RULE that page 2 changed carries "[page 2, ...]" marks where it changed)
@@ITEM 206
TITLE: What can be put on a key, pad or knob
STATUS: DEFAULT option A; no word of his says otherwise; his message adds buttons and sliders to it -- AMENDED after page 2 (206 1 by J, 206 2 by J, 206 3 by J, 206 4 by J)
HIS: L1 (default); touched by L7, L11, L12, L34, L35, L38, L64, L72, L73, L94, L106, L114, L120
RULE: Option A, the default (L1): every button and every slider of the app can be put in the keyboard and MIDI mapping: a key or a pad on a button, a knob or a fader on a slider (lists: J-12). Only the output screens stay out: their settings (option A), and switching an output on or off (J-18). Buttons, with the ones his message adds: a clip cell (the key or pad triggers the cell, as a click on its picture does, and like that click it also selects the cell, so that the Clip tab shows its clip and Copy, Cut, Paste and Delete then work on it: topic F's page item 233; that a key or pad selects as the click does is Harmony's reading, J3-10; a preview by pad: J-23) [page 2, J: BF255 "you can select the cell which triggers it and selects it"]; a column trigger; a deck tab; each layer's X, bypass, solo and cue button, its Ignore Column Trigger switch and its new switch against actions (L7, L73; what that switch reaches and what it is called is topic D's, D-2); every action's on / off button and its Loop toggle (L64); Stop actions (L72); master cue (L38); the cue / preview toggle (L35); the ignore lamp of any control; the tempo bar (tap tempo, resync, tempo play, tempo pause, tempo stop, BPM -, BPM +, BPM /2, BPM x2, nudge back, nudge forward); Record to Clip and Record Show (L114); Audio play / pause; Snapshot, which stays and is a button of the mapping like any other: he puts it on a key or a pad himself, and it comes with no key of its own, with @@ITEM R227 (c) as it stands ("the user can push a shortcut button and save that to look out later", BF243, names no key; what a Snapshot saves, and the one assumption on its key, are topic F's) [page 2, J: BF243 "the user can push a shortcut button and save that to look out later"; BF257 "We keep the safe snapshot somewhere."]. Sliders: the two masters, each layer's transparency slider, the slider beside each cue button (L34), the Global glide (L64), the Macros knobs, every slider of every effect on a clip, a layer or Global, and each signal user's Gain, Falloff and Threshold (L106). A key or pad does exactly what a click on that button does, at the moment a click would act: when a fired clip starts is topic A's rule (L11, L12) and is the same for a click, a key and a pad. A knob does what a drag of that slider does (207 c); when the app takes the slider back from the knob: R199 e. A knob on a clip's slider belongs to the cell: it moves that slider of whatever clip sits in that cell of the deck on screen (page-2 item 244, way b); a knob on a layer's or a Global slider stays there. [page 2, J: BF262 "b"] Whether the mapping works while Studio is open: topic E (E-2). [page 2, J: BF245 "Let's go with studio. That's perfect."]
CHANGED: nothing in option A. Its list of buttons is brought up to his message (L7, L35, L38, L64, L72, L73; "Record" becomes his two names, L114; "tempo row" becomes his "tempo bar", L13). Named by this ruling because a builder would have to guess: switching an output on or off stays out (J-18); a pad on a clip cell fires and does not preview (J-23); the layer's switch against actions is topic D's (D-2), not ruled here.
TODAY: a fixed list of boxes; only Master Opacity and Master Signal are sliders; six handled targets have no box (area-controls TODAY 10, 11; U1, M1, M6). The whole target model (Binding.h enum, append-only, K6) has to become "any control", with an address per control. For a clip-level control keep its scope (this clip / this cell / the clip playing on the layer) a field of the address, so that the other ways of J-8 need no new file shape; the model already knows ByPosition / ThisItem / Selected (TODAY 15, 16). A rule he accepted earlier keeps the output settings out (area-controls M1: his "51 default is good").
@@END

@@ITEM 207
TITLE: A mapped pad: press or hold
STATUS: ANSWERED c, with a question back to Harmony -- AMENDED after page 2 (207 1 by J, 207 2 by J, 207 3 by J, 207 4 by J, 207 5 by J)
HIS: L120
RULE: His words: "207 c but make an argument for why we should have a key/pad hold setting. How does this help dj’s or bands using this software?" (L120). Option c in full: of the four settings one returns by his later words: the kind of knob, endless or not (page-2 item 270, BF270). Hit strength and a choice per entry between cell and clip stay out. The hold setting waits on the research he ordered (page-2 item 246, BF264); until he rules, a key or a pad only presses [page 2, J: BF270 "We need to work with both"; BF264 "do some research online"]. So: no hold-to-play: a press on a clip's pad fires it as a click does and letting go does nothing; a press on an action's button or on any other on / off button switches it over, as a click does. How hard a pad is hit changes nothing. Both kinds of knob work: a knob that sends its position sets the slider to that position, and an endless knob that sends steps moves the slider from where it stands; the app tells the two apart by itself where it can, and the knob's entry carries one toggle, endless or not (page-2 item 270). [page 2, J: BF270 "We need to work with both and know how they both work"] There is no "this cell / this clip" choice per entry: a clip's pad means the cell on the deck on screen (question 197 at its default), and a knob on a clip's slider means the cell in the same way (page-2 item 244, way b). [page 2, J: BF262 "b"] The mapping screen therefore shows one setting, on a knob's entry only: endless or not (page-2 item 270). Whether a key's or pad's entry gets a press / hold switch waits on page-2 item 246. [page 2, J: BF270 "set a toggle if it's an endless encoder"; BF264 "if this is worth doing?"] His question back was answered on page 2; he then ordered research (BF264). For the hold setting the rule stays c until he rules on that research (page-2 item 246). [page 2, J: BF264 "I want you to do some research online"]
CHANGED: the default A (two settings shown per entry: press or hold, normal or endless knob) is replaced by c; the sentence "A held pad let go before a BPM-mode clip's 1 starts nothing" has nothing left to act on
TODAY: the four settings exist in the model and no screen sets them (area-controls TODAY 15, 18; U5, U6, O7, K1, K3). They come out of the mapping model, or stay unreachable; hold comes back only if he says so after the answer. If it does: a release that is lost (a key let go while another window has the keyboard) leaves the pad on (K3), and a held pad's release is not kept in a recording (K3, K8; item D34): both are mended with it.
@@END

@@ITEM 208
TITLE: The Edit menu and copying clips
STATUS: DEFAULT option A; his L4 and L5 make it his own word for clips
HIS: L1 (default), L4, L5
RULE: Option A, the default (L1). The Edit menu holds Undo, Redo, Cut, Copy, Paste and Delete, and they work on what is marked: an action, or the selected clips; one thing is marked at a time. Undo and Redo sit in the Edit menu only and name what they undo ("The only place that we will see undo remove, will be in the top edit menu.", binding-decisions.md 698-699). Clips get real copy and paste, now in his own words: "We need copy and paste for any clip. If it is selected, it can be copied and then if they empty sell or sell with something else in it is selected then it can be pasted." (L4) and "Option drag on a clip copy and pastes into the cell it is dragged to." (L5). So: Cmd+C copies the selected clips; Cmd+V pastes into the selected cell, empty or filled, and a clip that was there is replaced; Cmd+X is a cut (a copy, and the cell is emptied); the Delete key empties the selected cells; an Option-drag puts a copy into the cell it is dropped on and leaves the original where it was. Everything else about a copied clip is topic F's and is not ruled a second time here (its items on L4 and L5): what a copy carries, how an empty cell is selected without clearing its layer (F-1), what a layer does when the cell that is pasted over, cut or deleted is playing (F-3), several clips at once, other decks. Cut, paste, delete and an Option-drag are Undo steps (R198). The menu's own edges: J-19.
CHANGED: nothing in option A; L4 adds that a paste may land on a cell that holds something (it is replaced), L5 adds the Option-drag ("sell" read as "cell": INFERRED). By this ruling: the edges of a copied clip are left to topic F, which owns L4 and L5, so that one rule exists and not two.
TODAY: no Edit menu; Undo / Redo head the Composition menu; Cmd+X clears, Cmd+C / Cmd+V do nothing, the Delete key does nothing; the menu ids for clip cut / copy / paste exist with no handler (area-controls TODAY 1-4; U10, U11, C4).
@@END

@@ITEM 209
TITLE: Which messages the app may show
STATUS: ANSWERED in words ("already dicsussed above"); they fit no letter, nearest A -- AMENDED after page 2 (209 1 by J, 209 2 by J)
HIS: L122, L68, L65, L80, L75
RULE: His words: "209 already dicsussed above" (L122). What he discussed above are three things the app tells him, so the rule is his earlier four plus these three. THE FOUR THAT APPEAR BY THEMSELVES (his earlier words): a failed save ("the only fail message will be a failed save.", binding-decisions.md 720); a clip recording that was not saved and a show recording that was not saved ("If the user is recording a clip live, and that is not saved in a message should show. If the user is recording the show, that is not saved, and that should be shown. Nothing else.", 772-774); and "not enough HDD space to record" ("display not enough HDD space to record.", 777). "Not saved" covers every part of a recording (the moves, the sound, the film) and a recording that a full disk ended (J-22). THE THREE OF THIS MESSAGE: (1) an action pasted onto a clip that lacks one of its parameters or effects: "that specific action is muted, and the small messages is displayed where the user needs to say OK they understand." (L68; it is also his answer to 184: "184 read my note on this above", L75); what exactly is muted is topic D's (D-6). (2) "show a warning in save screen if the action is not a multiple of 4 bars" (L65): inside the action save window. (3) a show recording whose show has changed "still opens it up correctly and indicates this to the user that these clips have been moved or missing." (L80): inside Studio [page 2, J: BF245 "Let's go with studio. That's perfect."]. (4), added by page 2: an action saved in Studio that cannot be saved because its clip is no longer in its show: the app tells him, and nothing is saved (page item 230, accepted as written; what is said is topic E's, whose rule reads the same for a layer that is gone and for another show open in the live window). All four answer something he did himself, in the window where he did it. There is no wider rule: beyond these four and the four above, a window or a command he opened himself says nothing, and nothing appears by itself. The clause of option A that let any window he opened answer him was struck when the old assumption J-5 was settled, on his earlier words ("Nothing else."; "ok. the only fail message will be a failed save. remove all others"). [page 2, J: page item 230 accepted as written (his box left empty): "If the clip is gone, the app tells you and nothing is saved." is the page's sentence, not his own words] Questions the app asks before it replaces or closes something (the quit window, an import that replaces) are not messages and stay. What a message does while it is up: item P34.
CHANGED: option A's list gains the three cases of L68, L65 and L80 as his own words; option B (strictly nothing) is ruled out by L68; the paste case is no longer "a window you opened may answer" but a small message with an OK. Kept from option A and marked as Harmony's, because L122 names no letter: its bracket on what counts as not saved (J-22) and its last sentence (J-5).
TODAY: recording messages are a line in the Record tab; the save-failed alerts exist (area-controls has no point on messages; area-recording H21, M2, O7 by the slice: not re-read).
@@END

@@ITEM 210
TITLE: Composition or show on screen
STATUS: DEFAULT option A; "Global" confirmed by his L55
HIS: L1 (default), L55
RULE: Option A, the default (L1): on screen the thing he saves and opens is a "Show" everywhere (a Show menu, New Show, the tab "Shows"). The level above the layers, and the tab that holds its effects and actions, is "Global"; his words: "composition and global are interchangeable but lets move to global as that is what musicians are more used to." (L55). So neither the file nor that level is called "Composition" anywhere on screen, in a tooltip or in the manual. This does not rule the word for the picture that all layers make together, which he himself calls "the full screen composition" (L88): where a control needs a word for it, that word is entered in the list of names (L114).
CHANGED: nothing in option A. By this ruling: the paper's "the word Composition is read nowhere" is limited to the two things option A and L55 name, the file and the level.
TODAY: the menu bar says "Composition"; the tab is "Compositions"; the global effects tab is the Composition inspector (area-controls TODAY 1, 2; slice source MenuBarModel.cpp:70-95, InspectorPanel.h:87-90: not re-read).
@@END

@@ITEM 214
TITLE: Pad lights: which controllers
STATUS: DEFAULT option A; no controller is named anywhere in his message
HIS: L1 (default)
RULE: Option A, the default (L1): nothing new is built for pad lights in this build. The buttons his message adds (actions, cue buttons, master cue, the tempo bar) get no lights until he names a controller, and he names none (L92 says only "the midi controller"). The rebuild of the keyboard and MIDI mapping leaves the pad lights that exist working as they are (which those are: Harmony's own notes). Lights for a controller he names are planned when he names it (item D35).
CHANGED: nothing
TODAY: the pad lights show the clip cells of the deck on screen in the note layout and colours of a Launchpad X or a Launchpad Mini MK3 and nothing else; the light device is picked by hand at every launch and not remembered; lights are sent about every 267 ms (area-controls TODAY 23, 24; C1; U12, K4).
@@END

@@ITEM R148
TITLE: The name: keyboard and MIDI mapping
STATUS: STANDS -- L116 only adds that files of it exist
HIS: none (L116 touches the file entries)
RULE: Everything about putting functions on keys, pads and knobs is called "keyboard and MIDI mapping": in talk, in the menu, on its screen, in tooltips and in the manual. The words "shortcuts", "bindings" and "MIDI learn" are read nowhere. Plugging a signal into a slider is never called mapping. The entries are named by Harmony and laid out where they fit (L8): "Keyboard and MIDI Mapping..." (opens the screen), "Export Mapping...", "Import Mapping...", "Import Mapping from Show...". The file is a "mapping file" (L116).
CHANGED: (b) "the exact new wording of each entry comes with the pictures" is replaced by L8: Harmony names and places them; the export / import file windows stay, under the new name (L116)
TODAY: menu "Shortcuts" with "Edit Keyboard Shortcuts...", "Edit MIDI Mappings...", "Stop All", "Export Bindings...", "Import Bindings..."; screens "Keyboard Binding Mode", "MIDI Learn Mode" (area-controls TODAY 6-9; U3, O3).
@@END

@@ITEM R197
TITLE: Names on screen
STATUS: CORRECTED parts b and h; c, d, f and i touched; one list of names is ordered -- AMENDED after page 2 (R197 1 by J, R197 2 by J)
HIS: L114; also L6, L21, L35, L38, L55, L72, L112
RULE: His words (L114): "we will record to clip or record show. That’s what the 2 recordings are called. All else is gone. Show Recording Review should be called Review for short is a good name. Do you have a better name for this? Macro’s panel should be called Macros. We need a solid list with what we call everything. You create and keep one and I will ask questions and you can give me the truth, which will be this doc. You will use this for comms with me and to name all features in the app, menus and manual(later)." Built so: (a) "Action" replaces "routine" everywhere; new ones are named "Action 1", "Action 2" and so on until renamed. (b) The two recordings are "Record to Clip" and "Record Show", and their two buttons read exactly that. No other name is used for either of the two: "take" goes. This is read as a rule about names; what else it could take away is J-1. Harmony's picks for the two places the page named: the tab that holds the two buttons keeps the word "Record" (the Record tab), and the list reads "Show Recordings", from his own "show recording" (L6). The screen where a show recording is watched and actions are made is "Studio": "Let's go with studio. That's perfect." (BF245; page-2 item 245, way b). The words "Review" and "Show Recording Review" are read nowhere for it. [page 2, J: BF245 "Let's go with studio. That's perfect."; BF263 "b"] (c) One name per thing, the same in talk, on screen and in the manual: the clip's two modes read "BPM mode" and "Timeline mode", his own words (L11, L12, L21), not "BPM Sync" (J-7). (d) "Timeline" alone is the link of a control to the layer's playhead: "Timeline is connecting anything that can be connected to the layers playhead and should be called as such." (L112; topic I). The clip's mode is always written with its second word, "Timeline mode" (L21). The rows of Studio are "tracks" (L86, L89). [page 2, J: BF245 "Let's go with studio. That's perfect."] (e) "Preset" alone means an effect's preset; MilkDrop keeps its presets; the hint on the layer's Feedback list reads "Feedback style". (f) "Cue" means the cue buttons; added by his message: "master cue" (L38) and the toggle between cue mode and preview mode (L35); topic B owns both. (g) "Mapping" means the keyboard and MIDI mapping only. (h) The macro panel is titled "Macros". (i) No menu entry is called "Stop All"; the button and the mapping entry that stop every action are "Stop actions" (L72). (j) THE LIST OF NAMES: Harmony creates and keeps ONE list of what everything is called, built from these papers; it is the truth for talk with him, for every name in the app and its menus, and later for the manual. Where two papers spell one thing two ways, the list has one spelling.
CHANGED: (b) "recording (Record, Recordings)" is replaced by his two names; "All else is gone" is read as about names (J-1); the tab's word and the list's title are Harmony's picks; "Review" is confirmed with its long form, and his question back is answered. (h) "Dashboard stays" is replaced by "Macros". (c) "keeps Resolume's label BPM Sync on screen" is replaced by one name for talk and screen: INFERRED from "to name all features in the app" (J-7); he read (c) and did not change it himself. (d) the slider source gets its name from L112, and the clip mode's name is kept apart from it. (i) "gets a truthful name with the pictures" is replaced by L8 and L72. Added: the one list of names (j). (a), (e), (f), (g) stand.
TODAY: "routine" in about 20 UI files; "take" on the Record tab; panel title "Dashboard"; clip mode label "BPM Sync"; menu entry "Stop All" only closes the mapping screens (area-controls TODAY 7, 30; U14, U20; O4, O5).
@@END

@@ITEM R198
TITLE: Undo: what Cmd+Z touches and what it never touches
STATUS: STANDS -- no line of his names it; L4, L5 and L104 add Undo steps -- AMENDED after page 2 (R198 1 by J)
HIS: none names it; L4, L5 and L104 add to it
RULE: His rule, with his list: "cmd-z does not affect anything in layer strip: play, play reverse, pause, transparency, bypass, solo, etc." (binding-decisions.md 764). (a) Cmd+Z never changes anything that is played live: firing a clip or a column, a layer's X, the tempo bar, every control of the Clip tab that changes how the clip plays, an action's on / off button, an ignore lamp, a cue button, a change in the keyboard and MIDI mapping, an output screen's settings. The controls his message adds follow the same split (J-15). (b) Cmd+Z does undo what is built: adding, removing or moving a clip, a layer, a column or a deck; cutting, pasting, deleting or Option-dragging a clip (L4, L5); adding, removing or re-ordering an effect ("G we want to re-order effects", L104); deleting, cutting or pasting an action; loading a preset. A deleted layer comes back with nothing playing on it. Undo and Redo never change what a layer is playing. A clip that was playing when it was cut, deleted or pasted over has left its layer (topic F's page item 234): the Undo brings it back into its cell and does not put it back on its layer. An Undo or Redo that would take away a clip that is playing leaves it playing until something else is triggered on its layer or the layer is emptied. [page 2, J: BF256 "Pasting over a clip or deleting a clip, removes it from the layer strip, and it does not play."; his Undo rule of 2026-10-04: "If a clip is triggered and] (c) New and Open clear the Undo history.
CHANGED: nothing replaced. Added by his other lines: a clip that is cut, pasted, deleted or Option-dragged (L4, L5) and a re-ordered effect (L104) are Undo steps; that the new live controls are not is Harmony's (J-15).
TODAY: a fire, B, S and X are Undo steps ("Trigger Clip", merged per layer); the undo-live lane is on paper, not built (area-controls TODAY 17; O6, M4).
@@END

@@ITEM R199
TITLE: The keyboard and MIDI mapping: how it behaves
STATUS: CORRECTED part f (mapping files are wanted); e gains his L66 (a glide, no jump); d is now his own word (L72); the rest stands -- AMENDED after page 2 (R199 1 by J, R199 2 by J)
HIS: L116; also L66, L64, L72, L94, L120
RULE: (a) A key or pad put on an action's button or on a cue button belongs to that action or button, not to its place in the row: a copy has no key, and deleting the action frees the key. (b) A key that is already in use moves to the new target and the screen shows what it was taken from; every entry can be removed; a button can have a key and a pad both, and the screen lists every one. (c) A held key fires once; only the nudge keys and the tempo keys repeat while held (which keys those are: J-20). (d) "Stop actions" stops every action at once; it is a button on screen and an entry of the mapping: "We need a global stop actions button that stops all actions. The tempo stop button stops all actions as well as everything else." (L72). What a stop leaves of the action buttons and how the values go back is topic D's (D-3, D-1). (e) A knob has no "let go": a slider that an action or a signal moves is his while he turns the knob and for a quarter of a second after. Then, where an action moves it, "the slider moves back to the action position based on the global glide back setting I described earlier." (L66; he says it of the hand, and by L9 it holds for the knob): a glide to the action's position over the Global glide time, from instant to four seconds (L64), never a jump. Where a signal moves it, the signal takes it back as topic I rules for a hand that lets go. A knob that sends its position and the slider can then differ, and the slider jumps to the knob at his next turn (put to him once more: J3-8). An endless knob never differs from the slider: its next turn goes on from where the slider stands (page-2 item 270). [page 2, J: BF270 "so they both work smoothly"] (f) His words: "we want mapping files" (L116). A mapping travels in three ways, and none replaces another: (1) it is saved in the show and kept on the computer ("with show and app. these stay.", binding-decisions.md 887); (2) another show's mapping can be imported from that show's file ("they can import the settings from another show file.", same line), which replaces the open show's mapping after asking; (3) "Export Mapping..." writes the open show's whole keyboard and MIDI mapping to a mapping file, and "Import Mapping..." reads one and replaces the open show's mapping after asking (J-2). Which mapping is live: the open show's. Opening a show makes that show's mapping live, and a new show starts with the mapping of the most recent show (asked whether the keys change when a show is opened, he answered "yes but you import the setup from the most recent show", binding-decisions.md 896; J-21). The app opens the very last show when it starts (L94), and with it that show's mapping. A show or a file that holds no mapping changes nothing; entries that point at something the open show lacks: J-16. (g) Controllers plugged in after the app has started are found without a restart, and two controllers of the same kind are told apart. (h) A change in the mapping is not an Undo step. (i) While a box for typing a name or a number is open, the keys type into it; Return or Esc gives the keyboard back. (j) While the mapping screen is open the show and the outputs go on, and no key or pad fires anything until it is closed. Whether and how the mapping works while Studio is open is topic E's [page 2, J: BF245 "Let's go with studio. That's perfect."] (E-2, with his earlier words on play mode and record over mode).
CHANGED: (f) the page's sentence that the Export Bindings and Import Bindings files go is replaced: files stay, as mapping files, beside the import from another show (L116); added to (f) from his earlier words and L94, because a builder must know it: which mapping is live when a show is opened, for a new show and at the start. (e) "then the action ... takes it back (158 A)" is replaced by a glide over the Global glide time (L66, carried from the hand to the knob by L9); "normal knob" is the only kind (207 c, L120). (d) is now his own word (L72). (c) "the tempo keys" is narrowed by Harmony (J-20). (j) gains the pointer to topic E. (a), (b), (g), (h), (i) stand.
TODAY: the mapping is never saved except through Export / Import Bindings files in ~/Library/Audio-DNA/bindings; the show file's "keys" block is written empty; no entry can be removed; a taken key moves silently; inputs are listed once at launch; no device is stored per entry (area-controls TODAY 12-14, 19-22; U4, U7, U9, U13, U16; M2-M5, M7; K1, K2). The adopted one-save plan (its stage S2, by area-controls part 2: keys block, a copy on the computer, "Import Settings from Show...") drops Export / Import Bindings: that part is reversed by L116. The quarter of a second is the 250 ms grip hold (area-controls U16); a jump to the action's position is what happens after it now, to become the glide.
@@END

@@ITEM R227
TITLE: The keys the app keeps for itself; the spacebar
STATUS: CORRECTED by one addition (the spacebar is tap tempo); the rest "all good" -- AMENDED after page 2 (R227 1 by J)
HIS: L118
RULE: His words: "R227 all good. Spacebar is typically tap tempo" (L118). (a) A key held with Cmd belongs to the app (Save, Open, Undo, Cut, Copy, Paste, the output keys) and cannot be mapped. (b) Esc and the Delete key belong to the app: Esc leaves the keyboard and MIDI mapping screen, Delete removes what is marked. Every other key can be mapped, the spacebar included. (c) Added by his line, in Harmony's reading (J-3 for both halves): in the live window the spacebar comes mapped to tap tempo, the only key that comes mapped, and it can be put on something else like any key; while Studio is open the spacebar is Studio's own and stops and starts the playback [page 2, J: BF245 "Let's go with studio. That's perfect."] (the stop is his earlier word: "or presses the spacebar, it will stop playing but nothing else happens", binding-decisions.md 1090). A tap counts at the press, once, however long the key is held (R199 c).
CHANGED: added to (b): the spacebar is tap tempo (L118); that it comes mapped, can be put elsewhere, and stays Review's own key in Review is Harmony's reading of "typically" (J-3). Nothing removed ("all good").
TODAY: no Cmd key can be mapped; Space, Delete and Backspace are handled nowhere, so Space does nothing until mapped; nothing is mapped out of the box (area-controls TODAY 4, 5; U15). Two traps for the builder: a button that has the keyboard focus answers to Space and Return (bug X1 on the board: a clip-cell click + Return clears the top layer; area-controls TODAY 32, K10), so with the spacebar as tap tempo no button may keep the focus, or a tap also presses it; and the Mac's main Delete key arrives as Backspace, the forward-delete key as Delete (from memory, not read in the source): both must remove what is marked.
@@END

@@ITEM R200
TITLE: Tooltips, the manual, OSC
STATUS: REPLACED in part c by his L128 (OSC is planned); a and b stand
HIS: L128; L114 for the manual
RULE: (a) Every button, slider and list gets a tooltip, written when its part is built, in the words of the list of names; the tooltip switch is remembered between launches. (b) A manual is written after the builds ("manual(later)", L114), opens from a Help menu, uses the list of names, and its first chapter is lining the picture up with the sound. (c) His answer to 215 is "plan all of these" (L128), and OSC is its letter a: OSC is planned, not left as it is. What its plan holds (which controls get an address, whether the app only receives, the port) is topic K's, with topic K's own assumption; this item adds none. The addresses that say routine say action. The app's own remote-control port, which test tools use, keeps working, and new controls get their routes when they are built.
CHANGED: (c) "OSC ... stay as they are" is replaced by L128 for OSC; (a), (b) nothing
TODAY: tooltips in eight files only, the switch is back on at every launch; no manual, no Help menu; OSC on UDP 8000 with 15 address patterns, one of them /audiodna/routine/{slot}; 42 production routes on port 7070 (area-controls TODAY 25-28; U17-U19; K7).
@@END

@@ITEM D32
TITLE: A mapped pad on an empty cell or beyond the last column
STATUS: SETTLED for the empty cell (L11, L19); the cell beyond the last column is still Harmony's
HIS: L11, L19, L120
RULE: A pad does what a click does (207 c). A pad that points at an empty cell empties its layer as a click on the empty cell does; his words: "the empty cell triggers on the 1 if the clip playing is in bpm mode, same as a new clip in bpm mode. If a layer that is not in BPM mode is triggered, then that plays instantly" (L11). A pad that points at a cell beyond the last column of the deck on screen does nothing (assumption J-13).
CHANGED: "clears its layer, as a click on an empty cell does" now carries his timing: on the 1 when the playing clip is in BPM mode, at once otherwise
TODAY: INFERRED in area-show-decks M-2 (Layer.h:403, 410-419), not run; not re-read.
@@END

@@ITEM D33
TITLE: An endless knob starts from where the slider stands
STATUS: REPLACED by 207 c (L120): there is no endless-knob setting -- AMENDED after page 2 (D33 1 by J, D33 2 by J)
HIS: L120
RULE: A knob only turns: the slider goes to the position the knob sends. An endless knob that sends steps works too (page-2 item 270): the app tells the kind by itself where it can, and the knob's entry carries one toggle, endless or not. [page 2, J: BF270 "We need to work with both"] A knob that sends its position works whether it has end stops or not; a knob that sends only steps (one up, one down) instead of a position works as well, and its first turn, like every later one, starts from where the slider stands: no jump. [page 2, J: BF270 "know how they both work so they both work smoothly"]
CHANGED: the whole item: it mended a setting that 207 c removes. By this ruling: "a knob without end stops is not supported" is narrowed to knobs that send steps.
TODAY: a relative CC keeps its own count starting at 0.5 and jumps the slider on the first turn; settable only by hand-editing a file (area-controls TODAY 15; K1). That an endless knob can send its position instead of steps is Harmony's knowledge of MIDI controllers, not checked for any controller of his.
@@END

@@ITEM D34
TITLE: A held pad's release is kept in a recording
STATUS: REPLACED by 207 c (L120): no pad is held
HIS: L120
RULE: A pad only presses, so a recording keeps each press and there is no release to keep. If he takes the hold setting after the answer to his question, this item returns as written: a held pad's release is recorded like its press, so that a replay lets go where he let go.
CHANGED: the whole item: it has nothing to act on under 207 c
TODAY: a hold-to-play release is not recorded in a take (area-controls TODAY 18; K3, K8).
@@END

@@ITEM D35
TITLE: Pad lights lag and the waiting clip's light
STATUS: OPEN -- deferred with 214 A: mended only when pad lights are built
HIS: none
RULE: Nothing is built for pad lights in this build (214 A). When they are built for a controller he names: a light follows a press at once, and a clip that waits for the 1 is lit as waiting, not as loaded (assumption J-14).
CHANGED: nothing
TODAY: lights are sent every 8th tick of a 30 Hz timer, about every 267 ms; a waiting clip is lit like a loaded one (area-controls C1, M8 / K11).
@@END

@@ITEM P32
TITLE: The mapping menu's title and entry wording
STATUS: DROPPED as a picture (L8); the names go into the list of names
HIS: L8, L114; for the tempo entries L12, L13, L17, L31, L72, L118
RULE: laid out by Harmony where it fits in the correct area; its look is settled in the UI redesign (L8). The names are in the NAMES section. The tempo bar's entries take his own words, not "Beat ...": "Tempo play", "Tempo pause", "Tempo stop" (his "tempo play", L17, L31; "pause is pushed on the tempo bar", L13; "tempo stop button", L72); "BPM -" and "BPM +" (his list of the row: "bpm- bpm+", binding-decisions.md 913); "Nudge back" and "Nudge forward" (his "nudgeBack nudgeForward", same line); "BPM /2" and "BPM x2" (his "/2 and x2", L13; the prefix is Harmony's pick); "Tap tempo" (L118) and "Resync" (L12).
CHANGED: the drawn variants go; "Beat play", "Beat pause", "Beat stop" become "Tempo play", "Tempo pause", "Tempo stop" (INFERRED from his own wording); "Tempo -" and "Tempo +" become "BPM -" and "BPM +", his own words for the two buttons; "Tempo /2" and "Tempo x2" follow with the same prefix
TODAY: the entries do not exist; the adopted tempo-row plan names them "Beat play" and so on (area-controls part 2, BD:1069 line; K6).
@@END

@@ITEM P33
TITLE: The keyboard and MIDI mapping screen itself
STATUS: DROPPED as a picture (L8) -- AMENDED after page 2 (P33 1 by J)
HIS: L8
RULE: laid out by Harmony where it fits in the correct area; its look is settled in the UI redesign (L8). What it must be able to do is fixed elsewhere: reach every button and slider (206 A), show every entry and remove any (R199 b), show one setting, on a knob's entry only: endless or not (page-2 item 270); a press / hold switch on a key's or pad's entry waits on page-2 item 246 [page 2, J: BF270 "set a toggle if it's an endless encoder"; BF264 "if this is worth doing?"].
CHANGED: the three drawn variants go
TODAY: two full-window covers on a made-up grid of fixed boxes, no list, no remove (area-controls TODAY 8-12; U2, K5).
@@END

@@ITEM P34
TITLE: Where a message appears, and what it does while it is up
STATUS: OPEN for what a message does (J-6); its look and place are dropped as a picture (L8) -- AMENDED after page 2 (P34 1 by J)
HIS: L8, L68
RULE: Look and place: laid out by Harmony where it fits in the correct area; its look is settled in the UI redesign (L8). What a message does is not a matter of look, and no line of his gives it beyond the paste message's OK ("where the user needs to say OK they understand", L68), so it is Harmony's (J-6): a message is seen whichever tab is open; it stays until he clicks it away; it never stops the show (keys, pads, the outputs and the picture go on while it is up); it is never drawn on an output screen. The warning of L65 sits inside the action save window, and the marks of L80 inside Studio [page 2, J: BF245 "Let's go with studio. That's perfect."].
CHANGED: the three drawn variants go as pictures. Because they differ in what the app does (a box that must be answered, a line that stays, a line he may not see from another tab), the function is named here and carried by J-6 instead of being dropped with the look.
TODAY: the recording messages are a line in the Record tab, not seen from another tab (slice text; area-recording, not re-read).
@@END

## ASSUMPTIONS STILL OPEN (new after page 2: ids with a 3; old ones never shown to him keep their ids)
@@ASSUME J3-1
ABOUT: 244, 206
TEXT: I assume the knob finds its slider by name: the same slider of the same effect on whatever clip sits in the cell. If that clip does not have that effect, the knob does nothing.
WHY: BF262 "b" is the page's sentence "it moves that slider of whatever clip sits there": the same slider, which on another clip can only be found by its name. The edges of finding it (the same effect twice, a source, a macro knob) are J3-11.
ALT: b) By place: the knob moves, for example, the second slider of the first effect, whatever that effect is.
IF-WRONG: STAGE a knob would move an unexpected slider of another clip; found by name it moves the right one or none
ASK: NO the plain meaning of "that slider" in the way he chose; nobody asked for "by place", and a knob that does nothing on a clip without that effect is part of that way
@@END

@@ASSUME J3-2
ABOUT: 244
TEXT: I assume a knob on a clip's slider follows the deck on screen: after a deck switch it moves that slider of the clip in the same cell of the new deck, and no longer the clip from the old deck that is still playing.
WHY: BF262 "b": the way he chose says "in the deck on screen", with way c (the clip playing on the layer) beside it. By BF256 ("If a clip is playing, and I change the deck, that does not change the clip") the old clip plays on, and its knob then serves a clip that may not be playing.
ALT: b) While a clip from that cell of another deck is still playing, the knob stays with it.
IF-WRONG: STAGE he would turn a knob and the playing clip would not answer, while a clip that is not playing is changed unseen
ASK: LINE it is the wording he chose and his pads behave the same, so he will most likely wave it through; but it is the consequence of his "b" most likely to surprise him on stage
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
TEXT: I assume every knob in the mapping has an Endless toggle. The app sets it by itself when you map the knob, from what the knob sends, and you change it where the app got it wrong.
WHY: BF270 "If we have to, we could ask the user to set a toggle if it's an endless encoder." Whether we have to is a technical call: a knob's messages alone may not always tell the kind (Harmony's knowledge of MIDI, not checked against any controller; item 270 (6) has it looked up).
ALT: b) No toggle is shown unless the app cannot tell. c) No guessing: you always set the toggle yourself.
IF-WRONG: SMALL it shows while he maps the knob, not in a show
ASK: NO technical; his "If we have to" leaves the call to Harmony, and the toggle costs him nothing where the app guesses right
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
TEXT: I assume that when a slider stands somewhere else than its knob (an action, a preset or another clip in the cell moved it), a knob that sends its position makes it jump to the knob at your next turn. An endless knob never jumps.
WHY: BF270 "so they both work smoothly" may ask for no jump. The jump is in the reading of the mapping that he read on the last page and left as it was (old R199 (e): inferred consent, not an answer of his). With BF262 a knob serves whatever clip sits in a cell, so knob and slider differ often.
ALT: b) No jump: the knob does nothing until it passes the slider's value, and the slider follows it from there. c) No jump: the slider moves from where it stands, faster or slower than the knob, until the two meet.
IF-WRONG: STAGE a slider that jumps is seen on the output; a knob that waits feels dead for a moment
ASK: YES "smoothly" can be read both ways, and the audience sees a jump
@@END

@@ASSUME J3-9
ABOUT: 271, 206
TEXT: I assume All Outputs Off, Restore Last Outputs and Syphon's on / off count as switching an output: none of them can be put on a key or pad either.
WHY: Item 271, accepted as written, says "switching an output screen on or off"; it names neither the two commands nor Syphon, which is an output but not a screen ("syphon is an output and treated with same output settings as a screen", binding-decisions.md 834).
ALT: b) All Outputs Off may go on a key or pad, as a panic button. c) Syphon's on / off may go on a key or pad: it is not a screen.
IF-WRONG: SMALL one entry more or less in the mapping; the Cmd key for all outputs off stays
ASK: NO the item's own reason (no stray hit blanks a projector) covers all three, and a stray hit on Restore Last Outputs would end the hold of All Outputs Off (BF269 "b")
@@END

@@ASSUME J3-10
ABOUT: 206, D32
TEXT: I assume a key or pad on a cell does all that a click on the cell does: it triggers the cell and also selects it, so the Clip tab shows that clip and Copy, Cut, Paste and Delete then work on it.
WHY: BF255 "you can select the cell which triggers it and selects it" is said of a click; old item 206, which he let stand, says a key or pad does exactly what a click does. With BF256 ("deleting a clip, removes it from the layer strip, and it does not play") the selected clip is then the one just triggered.
ALT: b) A key or pad only triggers: what is selected, and what the Clip tab shows, stays as it was.
IF-WRONG: STAGE the Clip tab would follow every pad he hits, and a stray press of the Delete key would take the clip he has just triggered off the output; small to change once built
ASK: LINE best printed inside topic F's line on the same click (its F3-1), as "a key or a pad on the cell does the same"
@@END

@@ASSUME J3-11
ABOUT: 244, 206
TEXT: I assume that when a clip carries the same effect twice, the knob moves the one in the same place among them; a source's slider is found by the source's and the slider's name, and a clip's macro knob by its number.
WHY: BF262 "b" says "that slider of whatever clip sits there"; it does not say which of two effects of one name is meant, nor how a source's slider or a macro knob is found on another clip.
ALT: b) With the same effect twice the knob moves both.
IF-WRONG: SMALL an edge of finding a slider by its name
ASK: NO technical; an edge he would not notice
@@END

@@ASSUME J3-hold
ABOUT: 246, 207, D34
TEXT: I assume your answer stands until you say otherwise: no hold setting. A pad or a key only presses, and letting go does nothing.
WHY: He chose "none" and has not taken it back; he asked whether it is worth doing. The research says yes, medium sure: way b is Harmony's recommendation.
ALT: b) Every pad and key gets one switch, press or hold, set to press until you change it. On hold, a clip, an effect's button or an action is on only while you hold, and goes back when you let go. c) The same switch, but only for an effect's button and an action; a clip's pad always only presses.
IF-WRONG: SMALL an estimate: one switch per mapped key or pad plus the release kept in a recording (D34), added later
ASK: YES a, b or c
@@END

@@ASSUME J-1
ABOUT: R197, 206
TEXT: I assume "All else is gone" is about names only: the two recordings are called Record to Clip and Record Show, no other word is used for them, and the sentence takes away nothing that records.
WHY: L114 "All else is gone" follows a sentence about names, and his newer words keep the other things that record (BF253 "It defaults to running next to the show recording"; BF241 "we can make an HD render from the Recording Review"). The one thing the sentence could still take away, the Video box of Record Show (his "48 b", binding-decisions.md 867), is asked once, by topic E (its E3-3).
ALT: b) The sentence also takes away the full-size film box of Record Show.
IF-WRONG: SMALL a film box is small to add or to remove
ASK: NO the Video box is topic E's line (E3-3), printed once, and its text is that the box stays; nothing else of this reading is in doubt after BF253 and BF241
@@END

@@ASSUME J-6
ABOUT: P34, 209
TEXT: I assume a message never stops the show: keys, pads and outputs keep working while it is up. It shows whichever tab is open, stays until you click it away, and never appears on an output screen.
WHY: L68 asks for an OK on the paste message only; how the others leave, and that none may hold up a show, is in no line of his.
ALT: b) Only the paste message needs a click; the others are a line that goes away by itself. c) A message is a box that must be answered before anything else works.
IF-WRONG: SMALL with this reading no message can hold up a show; how one leaves is small to change
ASK: LINE taken from one message of his and applied to all, with the safe choice for the stage
@@END

@@ASSUME J-12
ABOUT: 206, 207
TEXT: I assume keys and pads go on buttons only, and knobs and faders on sliders only. A list you pick from, such as a blend mode, cannot be put on a key in this build.
WHY: 206 A says every button and every slider; 207 c says a pad only presses and a knob only turns; lists are not named.
ALT: b) A pad on a list steps to its next entry. c) A pad can be put on a slider and switches it between zero and full.
IF-WRONG: SMALL can be added per kind of control
ASK: NO follows from the two answers; small to widen later
@@END

@@ASSUME J-13
ABOUT: D32
TEXT: I assume a pad that points at a cell beyond the last column of the deck on screen does nothing.
WHY: Not in his words; a pad means the cell on the deck on screen (197 A) and a smaller deck may not have that cell.
ALT: b) It empties the layer, like an empty cell.
IF-WRONG: SMALL one edge
ASK: NO an edge he would not notice
@@END

@@ASSUME J-14
ABOUT: D35, 214
TEXT: I assume pad lights wait: when they are built for a controller you name, a light follows a press at once and a clip that waits for the 1 is lit as waiting.
WHY: 214 stands at its default (nothing new for pad lights) and he names no controller (L92 says only "the midi controller").
ALT: none
IF-WRONG: SMALL deferred work
ASK: NO deferred with 214 by his own default
@@END

@@ASSUME J-15
ABOUT: R198
TEXT: I assume Cmd+Z never undoes the new live controls: master cue, the cue / preview toggle, a layer's switches, an action's Loop toggle, Stop actions, the Global glide. Pasting, cutting or Option-dragging a clip and re-ordering effects are Undo steps.
WHY: His Undo rule (binding-decisions.md 764) lists live controls and ends with "etc."; L4, L5, L7, L35, L38, L64, L72, L104 add controls and steps it could not name.
ALT: none
IF-WRONG: SMALL one control in or out of Undo
ASK: NO his rule applied to new controls
@@END

@@ASSUME J-16
ABOUT: R199, 244
TEXT: I assume a mapping entry that points at a layer or an action the open show does not have is kept, does nothing and is shown as missing. An entry on a cell is never missing: it waits for a clip that has that control.
WHY: L116 and his earlier import words do not say what happens when two shows differ. BF262 "b" makes a knob on a clip's slider belong to the cell, so a cell that is empty, or whose clip has no such slider, is the normal case and not a fault.
ALT: b) Entries that point at something the show does not have are dropped at import.
IF-WRONG: SMALL list housekeeping
ASK: NO internal; nothing fires either way
@@END

@@ASSUME J-17
ABOUT: R197, P32
TEXT: I assume no menu entry is needed to close the keyboard and MIDI mapping screen: Esc or the screen's own close button does that.
WHY: L8 leaves layout to Harmony; the page promised the entry that closes the mapping screens a truthful name with the pictures.
ALT: b) An entry "Close Mapping" stays in the menu.
IF-WRONG: SMALL a menu entry
ASK: NO layout and wording
@@END

@@ASSUME J-19
ABOUT: 208, R198
TEXT: I assume the Edit menu follows the Mac's habits: an entry is grey when nothing is marked; in a box for typing, Cut, Copy and Paste work on the text; Delete never asks first, also when the clip is playing.
WHY: 208 A names the six entries; whether Delete asks first is in no line of his. BF256 ("deleting a clip, removes it from the layer strip, and it does not play") describes the delete as an act with an immediate effect and names no question; but the old "(Undo brings it back)" no longer holds whole: an Undo brings the clip back into its cell, not onto its layer (@@ITEM R198 as amended).
ALT: b) Delete asks first when the clip is playing.
IF-WRONG: STAGE a stray Delete takes a playing clip off the output, and Undo does not put it back on its layer; a question box is small to add
ASK: NO his words describe the delete as immediate; topic F's line on the triggering click (its F3-1) shows him what Delete then works on, and the page ruling is asked to let it say "without asking"
@@END

@@ASSUME J-20
ABOUT: R199
TEXT: I assume the keys that repeat while held are the two nudge keys and BPM - and BPM + only. Tap tempo, /2, x2, play, pause, stop and every other key fire once per press.
WHY: The page said "only the nudge and tempo keys repeat while held" and he left it; which keys count as tempo keys is not said.
ALT: b) /2 and x2 repeat as well.
IF-WRONG: SMALL one key's habit
ASK: NO a held /2 would run the tempo to its limit; nobody wants that
@@END

@@ASSUME J-21
ABOUT: R199
TEXT: I assume a new show starts with the mapping that was live just before, which the computer keeps. Opening a show makes that show's mapping live; the app starts with the mapping of the show it opens.
WHY: "you import the setup from the most recent show" (binding-decisions.md 896) and "94 default" (939) predate L94; "most recent" may mean open last or saved last.
ALT: b) A new show takes the mapping of the show saved last, even if another show was open since.
IF-WRONG: SMALL the two differ only when a show was opened and not saved
ASK: NO an edge of a rule he gave
@@END

@@ASSUME J-22
ABOUT: 209
TEXT: I assume "not saved" covers every part of a recording, the moves, the sound and the film, and also a recording that a full disk cut off.
WHY: His words say "not saved" (binding-decisions.md 772-774); the bracket of option A that added the film and the full disk was Harmony's, and L122 names no letter.
ALT: b) Only a recording that is lost as a whole shows a message.
IF-WRONG: SMALL a message more or less
ASK: NO the plain meaning of his words
@@END

@@ASSUME J-23
ABOUT: 206
TEXT: I assume a pad on a clip cell always fires the clip. Previewing a clip, which a click on its name does, cannot be put on a pad.
WHY: 206 A offers a pad for a clip; L16 and L35 make a click on the name a preview; no line says whether a pad can preview.
ALT: b) A second kind of entry lets a pad preview the clip of a cell.
IF-WRONG: SMALL one more kind of entry
ASK: NO not offered on the page and small to add
@@END

## CLOSED ASSUMPTIONS (one line each)
- J-2 -> page 2, item 269
- J-3 -> SETTLED
- J-4 -> page 2, item 270
- J-5 -> SETTLED
- J-7 -> SETTLED
- J-8 -> page 2, item 244
- J-18 -> page 2, item 271

## QUESTIONS BACK
@@ANSWER 246-hold
HIS: BF264 -- "I want you to do some research online and figure out what people are doing and if this is worth doing? We need to only match what Dj or bands are doing and also look at Dj/producers."
ANSWER: Yes, I think it is worth doing, in its smallest form. DJs hold buttons as a matter of course: AlphaTheta (Pioneer) calls it Gate Cue, where a hot cue plays only while the pad is held, and Denon calls it Momentary. For bands, the big lighting desks (grandMA, ChamSys, Avolites) have a Flash or Bump button that is on only while held, used for hits, strobes and blinders; your Resolume calls it Piano and offers it per clip and per shortcut. DJ / producers have it in their music tools (Gate in Ableton and Traktor), but the few who described their own visuals let Ableton trigger them; I found none who holds a pad for visuals, which does not prove they would not. Nobody has numbers on how many use it. I recommend one press-or-hold switch per pad and key, set to press. [Resolume](https://resolume.com/support/en/keyboard-shortcuts) [AlphaTheta](https://downloads.support.alphatheta.com/manuals/all-in-one-dj-systems/XDJ-AZ/html/en/000COV_en/Using_the_Performance_Pads/Using_the_Performance_Pads.htm) [ChamSys](https://docs.chamsys.co.uk/magicq/1.9.9.x/manual/playback.html)
@@END

## NAMES
@@NAME keyboard and MIDI mapping
MEANS: Everything about putting a button on a key or pad and a slider on a knob or fader; always written in full.
SOURCE: his words, BD:1099 ("r120 we call it keyboard and midi 'mapping' lets use this term")
@@END

@@NAME mapping file
MEANS: A file that holds a show's whole keyboard and MIDI mapping, written by Export Mapping and read by Import Mapping.
SOURCE: his words L116 ("we want mapping files"); what it holds is assumption J-2
@@END

@@NAME Keyboard and MIDI Mapping...
MEANS: The menu entry that opens the keyboard and MIDI mapping screen.
SOURCE: Harmony's pick from his term (BD:1099); the on-screen names now, "Edit Keyboard Shortcuts..." and "Edit MIDI Mappings...", are replaced by it
@@END

@@NAME Export Mapping...
MEANS: The menu entry that writes the open show's keyboard and MIDI mapping to a mapping file.
SOURCE: Harmony's pick (L116); the on-screen name now, "Export Bindings...", is replaced by it
@@END

@@NAME Import Mapping...
MEANS: The menu entry that reads a mapping file and replaces the open show's mapping after asking.
SOURCE: Harmony's pick (L116); the on-screen name now, "Import Bindings...", is replaced by it
@@END

@@NAME Import Mapping from Show...
MEANS: The menu entry that takes the keyboard and MIDI mapping out of another show file.
SOURCE: Harmony's pick, from his earlier words BD:887-888 ("they can import the settings from another show file")
@@END

@@NAME Show
MEANS: The file that holds everything he saves; the word for it in every menu and tab.
SOURCE: his word throughout (L69, L80, L94); 210 A by default (L1); the on-screen name now, "Composition", is replaced by it
@@END

@@NAME Shows
MEANS: The tab that lists the show files and their decks.
SOURCE: 210 A by default (L1); the on-screen name now, "Compositions", is replaced by it
@@END

@@NAME Global
MEANS: The level above the layers, and the tab that holds its effects and actions.
SOURCE: his words L55 ("lets move to global")
@@END

@@NAME Edit menu
MEANS: The menu that holds Undo, Redo, Cut, Copy, Paste and Delete.
SOURCE: his words BD:712 ("we should have an edit menu."); its contents 208 A by default
@@END

@@NAME Action
MEANS: A saved piece of recorded control movement that can be switched on for a clip, a layer or Global.
SOURCE: his words BD:1095 ("Just replace with action."); the on-screen name now, "routine", is replaced by it
@@END

@@NAME Record to Clip
MEANS: One of his two recordings: the full composition is recorded into a new clip; topic E owns what it does.
SOURCE: his words L114 ("we will record to clip or record show") and L88
@@END

@@NAME Macros
MEANS: The panel of macro knobs.
SOURCE: his words L114 ("Macro’s panel should be called Macros"); the on-screen name now, "Dashboard", is replaced by it
@@END

@@NAME BPM mode
MEANS: The clip mode in which the clip's length is counted in beats and it starts on the 1.
SOURCE: his words L11, L12, L21; the on-screen name now, "BPM Sync", is replaced by it (assumption J-7)
@@END

@@NAME Timeline mode
MEANS: The clip mode in which the clip plays by its own duration and speed; always written with its second word, because "Timeline" alone is the link of a control to the layer's playhead (L112, topic I).
SOURCE: his words L21 ("timeline mode"); whether the screen reads it so is assumption J-7
@@END

@@NAME Tap Tempo
MEANS: The button, and the spacebar out of the box, that sets the tempo by tapping.
SOURCE: his words L118 ("Spacebar is typically tap tempo")
@@END

@@NAME Tempo play
MEANS: The tempo bar's play button and its mapping entry.
SOURCE: his words L17, L31 ("tempo play"); replaces the plan's "Beat play"
@@END

@@NAME Tempo pause
MEANS: The tempo bar's pause button and its mapping entry.
SOURCE: his words L13 ("the pause is pushed on the tempo bar"); replaces the plan's "Beat pause"
@@END

@@NAME Tempo stop
MEANS: The tempo bar's stop button and its mapping entry.
SOURCE: his words L72 ("The tempo stop button"); replaces the plan's "Beat stop"
@@END

@@NAME Nudge back and Nudge forward
MEANS: The two mapping entries, and the two buttons of the tempo bar, that move the 1 a hair earlier or later.
SOURCE: his words, binding-decisions.md 913 ("nudgeBack nudgeForward" in his list of the row) and L13 ("it moves the one a hair forward or back")
@@END

@@NAME Resync
MEANS: The button, and its mapping entry, that sets the 1 at the press.
SOURCE: his words L12 ("unless resync is clicked")
@@END

@@NAME cue button
MEANS: The button per layer that puts that layer into the preview monitor.
SOURCE: his words L34 ("next to each cue button for each layer"); topic B owns it
@@END

@@NAME ignore actions
MEANS: His words for keeping actions off a control. On a single slider or button it is the toggle that topic D calls the ignore lamp; the switch on a whole layer takes its name and its reach from topic D (D-2).
SOURCE: his words L7 ("ignore actions toggle on the layer"), L73, and binding-decisions.md 1056 ("every slider, button, everything needs an ignore actions toggle")
@@END

@@NAME Preset
MEANS: A saved setting of one effect; MilkDrop keeps its own presets.
SOURCE: his words (L48, L52); the page's reading stands
@@END

@@NAME Feedback style
MEANS: The hint on the layer's Feedback list (Custom, Zoom In, Spiral and so on).
SOURCE: Harmony's pick, not corrected by him; the on-screen name now, "Feedback preset", is replaced by it
@@END

@@NAME Audio play / pause
MEANS: The mapping entry that plays and pauses the audio file, named so that it is not taken for Tempo play.
SOURCE: Harmony's pick (chat answer of 2026-10-05); the on-screen name now, "Play / Pause", is replaced by it
@@END

@@NAME Help menu and Manual
MEANS: The menu that opens the manual, which is written after the builds from the list of names.
SOURCE: his words L114 ("manual(later)"), BD:801; the menu is Harmony's pick
@@END

@@NAME the list of names
MEANS: The one document that says what everything is called, kept by Harmony, used in talk with him, in the app, its menus and the manual.
SOURCE: his words L114 ("We need a solid list with what we call everything."); its title is Harmony's pick
@@END

@@NAME BPM - and BPM +
MEANS: The two mapping entries, and the two buttons of the tempo bar, that lower and raise the tempo by 1 BPM.
SOURCE: his words, binding-decisions.md 913 ("bpm- bpm+" in his list of the row); they replace the paper's "Tempo -" and "Tempo +"
@@END

@@NAME BPM /2 and BPM x2
MEANS: The two mapping entries that halve and double the tempo; on the tempo bar the two buttons read "/2" and "x2".
SOURCE: his words L13, L14 ("/2 and x2"); the prefix "BPM" is Harmony's pick, so that the entries are not taken for a clip's own /2 and x2 (L21); they replace the paper's "Tempo /2" and "Tempo x2"
@@END

@@NAME Stop actions
MEANS: The button, and its mapping entry, that stops every action at once; what it leaves of the action buttons is topic D's.
SOURCE: his words L72 ("a global stop actions button that stops all actions"); spelled as topic D spells it
@@END

@@NAME Record tab
MEANS: The tab that holds the two buttons Record to Clip and Record Show and the list Show Recordings.
SOURCE: Harmony's pick: L114 names the two recordings, not the tab; "Record" here is the tab's word, not a name for a recording
@@END

@@NAME Studio
MEANS: The screen where a show recording is watched, mended, recorded over and cut into actions.
SOURCE: his words (BF245: "Let's go with studio. That's perfect."; BF263: "b"); replaces the name Review (long form Show Recording Review) in /Users/boriskarpman/projects/RealTimeAudio/.harmony/NAMES.md, row at line 203
@@END

@@NAME Endless
MEANS: The toggle on a knob's entry in the keyboard and MIDI mapping that says the knob is an endless one that sends steps, not its position.
SOURCE: Harmony's pick, from his words (BF270: "set a toggle if it's an endless encoder")
@@END

@@NAME Review
MEANS: A word that is no longer used: his earlier name for the screen that is now called Studio (long form: Show Recording Review). Use: Studio.
SOURCE: his words BF245 ("Let's go with studio. That's perfect.") and BF263 ("b"); they replace his earlier words of L114, where he asked for a better name himself
@@END

@@NAME Record Show
MEANS: The recording of a whole performance, which is looked at afterwards in Studio.
SOURCE: his words L114; "Studio" by BF245
@@END

@@NAME track
MEANS: One row of Studio: one recorded button or slider.
SOURCE: his words L86, L89; "Studio" by BF245
@@END

@@NAME Show Recordings
MEANS: The list of show recordings; a recording is opened from it in Studio.
SOURCE: Harmony's pick from his words "show recording" (L6); it replaces the page's word "Recordings"; "Studio" by BF245
@@END

## REACHES OTHER TOPICS (from the paper)
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

## CONFLICTS (from the paper)
- Endless knobs. New (BF270): "We need to work with both and know how they both work so they both work smoothly." Old (binding-decisions.md 1191, BF232): "207 c but make an argument for why we should have a key/pad hold setting." -- way c of 207 read: no setting at all, a knob only turns, so no endless-knob setting. The new words win: the endless knob returns; the rest of c stands (hit strength, cell or clip per entry), and hold waits on the research.
- The screen's name. New (BF245): "Let's go with studio. That's perfect." Old (binding-decisions.md 1187, BF228): "Show Recording Review should be called Review for short is a good name. Do you have a better name for this?" The new word wins; he had asked for a better one himself.
- Not a conflict, said for completeness: the jump of a slider to a position-sending knob (R199 e, on the last page, left by him) against BF270 "so they both work smoothly". Put to him as the one question J3-8.

## NOT DONE / UNSURE (from the paper)
- Item 246 carries no answer and no assumption on purpose: the research he ordered (BF264) is written by another seat. If he then takes hold: @@ITEM 207 (its "no hold-to-play" sentence), @@ITEM D34 (returns as written), @@ITEM P33 and R199 (c) need amendments, and a hold on a BPM mode clip that ends before the 1 needs its rule.
- Which controller he uses is still not known (270's question was not answered). By BF270 the app must not depend on it; it would still settle the tuning of J3-7, the ways of sending steps that must be read first, and pad lights (@@ITEM 214, D35). Cheapest: he names it when the build of the mapping starts.
- How endless knobs send their steps is Harmony's knowledge of MIDI, not checked: several ways exist, and whether a knob's messages alone always tell the kind is not known. Cheapest: a Researcher reads the MIDI notes of three common controllers; it decides whether the Endless toggle is a safety net (J3-6) or the only way.
- J3-2 is ASK NO because he chose the wording himself with the other way on the same card. It is still the consequence of 244 b most likely to surprise on stage (after a deck switch the knob leaves the playing clip): the ruling seat may lift it to a LINE.
- The old word inside the other spec files (E above all) was not amended here, by the rule on other topics' blocks; only R224 was, as a MADE FROM block of item 245. If the merge needs every "Review" replaced block by block, that is one mechanical pass by the seat that owns each file.
- TODAY lines: 270's from BindingManager.cpp:155-198 and Binding.h:62-79 (read, not run); the others from the old blocks and area-controls, not re-read.
- Written 2026-10-09 18:53:11 EDT (from date).

## FOR THE PAGE RULING (from the ruling)
- MUST GO TO HIM, by weight: (1) J3-hold (YES), with @@ANSWER 246-hold printed above it; both come from answer-hold.md, which the merge lays over this topic AFTER this ruling; item 246 stays OPEN on it. (2) J3-8 (YES): may a knob that sends its position make a slider jump; print it beside the line that says both kinds of knob now work. Then two LINES: (3) J3-2: after a deck switch the knob serves the same cell of the new deck, not the clip that is still playing. (4) J3-10: a key or pad that triggers a cell also selects it; print it INSIDE topic F's line F3-1 ("a key or a pad on the cell does the same") and let that one line also say that Delete acts without asking (the old J-19, internal).
- NOT FOR HIM (ASK NO): J3-1 (the knob finds its slider by name: the way he chose says "that slider"), J3-3, J3-4, J3-5, J3-6 (the Endless toggle: his "If we have to" leaves it to us), J3-7, J3-9, J3-11, and the old J-1, J-16 and J-19, written again. The other old internal assumptions of this topic (J-6, J-12 to J-15, J-17, J-20 to J-23) are untouched by his 33 boxes.
- NEWEST WORDS AGAINST EARLIER ONES, to be said to him once, in one line: on 2026-10-07 he chose "207 c" (nothing to set on a knob); now "We need to work with both and know how they both work so they both work smoothly." (BF270): the endless knob is back, with one toggle per knob. The name Studio is no conflict: he had asked for a better name himself.
- A TENSION, not a clash, carried by J3-2 (the paper's CONFLICTS has no line for it): BF262 "b" (the knob belongs to the cell of the deck on screen) beside BF256 ("If a clip is playing, and I change the deck, that does not change the clip").
- ITEM 246 is not re-typed here: answer-hold.md replaces it whole (its RULE spells out ways a, b and c). If that file were missing at the merge, the paper's block would stand, and its sentence "the app only has to match what DJs, bands and DJ/producers do" must then be read as his words say: match DJs or bands; DJ / producers are looked at too (F6).
- DEPENDS ON E: the Video box of Record Show is E3-3 alone (J-1 is NO now and says "names only"); one number for a knob that is let go, a quarter of a second (old R199 (e), E's E-32); how a cell's knob records in Studio is E's item 257.
- DEPENDS ON F: Snapshot's key is F3-13 alone; AMEND 206 2 here says "no key of its own", as F's item 236 (1) does. AMEND R198 1 is the Undo sentence that F's ruling asked of this topic. F3-1 and J3-10 are one line (above).
- DEPENDS ON G and B: J3-9 keeps All Outputs Off, Restore Last Outputs and Syphon out of the mapping, so no stray pad ends the hold of page item 265. "master cue" keeps its place in the list of old item 206 until B's question to him is answered.
- MESSAGES (old item 209, AMEND 209 2): eight things may speak now; the eighth is page item 230 (an action saved in Studio whose clip is gone). Topic E's readings of that item (a layer that is gone; another show open in the live window) lean on the same case; any further message that another topic's rule lets the app show needs a line in 209 or goes.
- BULLETS OF THE PAPER THAT A RULING CANNOT REPLACE (the merge copies them; read them so): SUMMARY, last bullet: one question (J3-8) and two lines (J3-2, J3-10), not three lines. REACHES, "TOPIC E: J-1 (changed here)": void. REACHES, TOPIC F: "empties the layer (BF256)" is topic F's reading (its F3-3); his words are "removes it from the layer strip, and it does not play". NOT DONE, first bullet (246 without an answer): overtaken by answer-hold.md; fourth bullet (J3-2): lifted to a LINE here.
- FOR THE BUILD, not for him: "know how they both work" (BF270) is his order to learn how endless knobs send their steps: one Researcher task before the mapping is built (item 270 (6)); its result also says how far the app's own guess of a knob's kind can be trusted (J3-6). Which controller he uses is still not known, and by his words the app must not depend on it.
- THE LIST OF NAMES: Studio in every row the paper lists under REACHES; "Review" and "Show Recording Review" go to the words that are no longer used (four NAME blocks here); "Endless" (the toggle) and "Studio picture" are Harmony's picks.

