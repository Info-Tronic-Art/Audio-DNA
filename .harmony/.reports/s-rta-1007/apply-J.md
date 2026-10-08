# APPLY J -- The keyboard and MIDI mapping, menus and messages (s-rta-1007)
Written 2026-10-07 22:50:37 EDT by the architect (read-only; nothing built, run, launched or committed). His words: boris-msg-numbered.txt (L numbers); earlier words: binding-decisions.md (BD:line). TODAY notes cite s-rta-1005/area-controls.md (points TODAY n, U, K, M, C).

## SUMMARY
- 207 c (L120): a key or pad only presses and a knob only turns. No hold, no velocity, no endless-knob setting, no "this cell / this clip" choice. D33 and D34 fall with it. His question back (why a hold setting) is answered under QUESTIONS BACK; the rule stays c until he says otherwise.
- R199 (L116): mapping files are wanted AS WELL as the mapping living in the show and on the computer. What a file holds is assumed (J-2).
- R227 (L118): all good, and the spacebar is Tap Tempo. Read as: its mapping out of the box in the live window; in Review it stops and plays (his earlier words, BD:1090). Assumed (J-3).
- R197 (L114): the two recordings are "Record to Clip" and "Record Show", "All else is gone" (two readings: J-1, the one YES of this paper); the screen is "Review" (short for Show Recording Review) until he answers the name question; "Macros" replaces "Dashboard"; ONE list of names for talk, app, menus and manual.
- 209 (L122 "already dicsussed above"): the messages he described above are allowed besides the four: the paste message with OK (L68), the warning in the action save screen (L65), the Review marking clips moved or missing (L80).
- 208 default A is confirmed and widened by L4 and L5 (copy / paste for any clip, paste replaces, Option-drag copies).
- 210 default A is confirmed for "Global" by L55.
- 206, 214 defaults stand; R200 (c) changes because of L128 (OSC is planned; topic K).
- P32, P33, P34 are layout: dropped under L8. The tempo entries take his own words (Tempo play / pause / stop), not "Beat ...".

## ITEMS
@@ITEM 206
TITLE: What can be put on a key, pad or knob
STATUS: DEFAULT option A; no word of his says otherwise
HIS: L1 (default); touched by L7, L34, L35, L38, L64, L72, L73, L106, L114, L120
RULE: Every button and every slider of the app can be put in the keyboard and MIDI mapping: a key or a pad on a button, a knob or a fader on a slider. Only the output screens' settings stay out. Buttons, with the new ones his message adds: a clip cell; a column trigger; a deck tab; each layer's X, bypass, solo, its cue button, its ignore-column and its ignore-actions toggle (L7, L73); every action's on / off button and its loop toggle (L64); Stop Actions (L72); Master Cue (L38); the cue / preview toggle (L35); the ignore-actions lamp of any control; the tempo bar (Tap Tempo, Resync, Tempo play, Tempo pause, Tempo stop, Tempo -, Tempo +, Tempo /2, Tempo x2, Nudge back, Nudge forward); Record to Clip and Record Show (L114); Audio play / pause. Sliders: the two masters, each layer's transparency, the transparency slider beside each cue button (L34), the global glide slider (L64), the Macros knobs, every slider of every effect on a clip, a layer or Global, and each signal user's gain, falloff and threshold (L106). A pad or key does exactly what a click on that button does, with the same timing (a BPM mode clip waits for the 1, L11, L12); a knob does what a drag of that slider does (207 c). The Snapshot entry of the page's default follows what topic F rules on his "Do we need snapshot?" (L94).
CHANGED: nothing in option A; its list of buttons is brought up to his message (the new buttons of L7, L35, L38, L64, L72, L73; "Record" becomes his two names, L114)
TODAY: a fixed list of boxes; only Master Opacity and Master Signal are sliders; six handled targets have no box (area-controls TODAY 10, 11; U1, M1, M6). The whole target model (Binding.h enum, append-only, K6) has to become "any control", with an address per control.
@@END
@@ITEM 207
TITLE: A mapped pad: press or hold
STATUS: ANSWERED c, with a question back to Harmony
HIS: L120
RULE: His words: "207 c but make an argument for why we should have a key/pad hold setting. How does this help dj’s or bands using this software?" Option c in full: none of the four settings exists. A key or a pad only presses, and a knob only turns. So: no hold-to-play (a press on a clip's pad fires it as a click does, and letting go does nothing; a press on an action's or any other on / off button switches it over, as a click does); how hard a pad is hit changes nothing; there is no endless-knob setting (a knob sets the slider to the knob's position); there is no "this cell / this clip" choice per entry (a clip's pad means the cell on the deck on screen, question 197 A). The mapping screen therefore shows no settings per entry. His question back is answered under QUESTIONS BACK (207-hold); the rule stays c until he answers it.
CHANGED: the default A (two settings shown per entry: press or hold, normal or endless knob) is replaced by c; the sentence "A held pad let go before a BPM-mode clip's 1 starts nothing" has nothing left to act on
TODAY: the four settings exist in the model and no screen sets them (area-controls TODAY 15, 18; U5, U6, O7, K1, K3). They come out of the mapping model, or stay unreachable; hold comes back only if he says so after the answer.
@@END
@@ITEM 208
TITLE: The Edit menu and copying clips
STATUS: DEFAULT option A; confirmed and widened by his L4 and L5
HIS: L1 (default), L4, L5
RULE: The Edit menu holds Undo, Redo, Cut, Copy, Paste and Delete. They work on what is marked: an action, or the selected clips. Clips get real copy and paste. His words: "We need copy and paste for any clip. If it is selected, it can be copied and then if they empty sell or sell with something else in it is selected then it can be pasted." (L4) and "Option drag on a clip copy and pastes into the cell it is dragged to." (L5). So: a selected clip is copied with Cmd+C or Edit > Copy; a selected cell, empty or holding another clip, takes the paste, and what was in it is replaced; a copy carries the clip's effects, settings and actions (165 A); Cmd+X is a cut (a copy, and the cell is emptied); the Delete key empties the selected cells; dragging a clip with Option held puts a copy into the cell it is dropped on and leaves the original where it was. Undo and Redo sit in the Edit menu only and name what they undo. Cut, paste, delete and an Option-drag are Undo steps.
CHANGED: nothing in option A; L4 adds that a paste may land on a cell that holds something (it is replaced), L5 adds the Option-drag ("sell" read as "cell": INFERRED)
TODAY: no Edit menu; Undo / Redo head the Composition menu; Cmd+X clears, Cmd+C / Cmd+V do nothing, the Delete key does nothing; the menu ids for clip cut / copy / paste exist with no handler (area-controls TODAY 1-4; U10, U11, C4).
@@END
@@ITEM 209
TITLE: Which messages the app may show
STATUS: ANSWERED in words ("already dicsussed above"); fits no single letter, nearest A
HIS: L122, L68, L65, L80, L75
RULE: His words: "209 already dicsussed above". Above he describes three messages, so these are allowed besides the four he ruled earlier. The four that appear by themselves: a failed save; a clip recording that was not saved; a show recording that was not saved; "not enough HDD space to record". Added by this message: (1) pasting an action onto a clip that lacks one of its parameters or effects: "that specific action is muted, and the small messages is displayed where the user needs to say OK they understand." (L68; this is also his answer to 184, L75); (2) "show a warning in save screen if the action is not a multiple of 4 bars" (L65): a line inside the action save screen, not a pop-up; (3) a recording whose show has changed "still opens it up correctly and indicates this to the user that these clips have been moved or missing." (L80): shown inside Review. Nothing else appears by itself (assumption J-5). Questions the app asks before it replaces or closes something (the quit window, an import that replaces) are not messages and stay.
CHANGED: option A's list gains the three cases of L68, L65 and L80 as his own words; option B (strictly nothing) is ruled out by L68; the paste case is no longer "a window you opened may answer" but a small message with an OK
TODAY: recording messages are a line in the Record tab; the save-failed alerts exist (area-controls has no point on messages; area-recording H21, M2, O7 by the slice: not re-read).
@@END
@@ITEM 210
TITLE: Composition or show on screen
STATUS: DEFAULT option A; "Global" confirmed by his L55
HIS: L1 (default), L55
RULE: On screen the file is a "Show" everywhere: a Show menu, New Show, Open Show, Save Show, the tab "Shows". The tab and the level that holds the global effects and actions is "Global"; his words: "composition and global are interchangeable but lets move to global as that is what musicians are more used to." (L55). The word "Composition" is read nowhere on screen, in a tooltip or in the manual.
CHANGED: nothing
TODAY: the menu bar says "Composition"; the tab is "Compositions"; the global effects tab is the Composition inspector (area-controls TODAY 1, 2; slice source MenuBarModel.cpp:70-95, InspectorPanel.h:87-90: not re-read).
@@END
@@ITEM 214
TITLE: Pad lights: which controllers
STATUS: DEFAULT option A; no controller is named anywhere in his message
HIS: L1 (default)
RULE: Pad lights are not part of this build. They light the clip cells of the deck on screen on a Launchpad X or a Launchpad Mini MK3 and nothing else; actions, cue buttons and the tempo bar get no lights until he names a controller. He says "the midi controller" once (L92) without naming it.
CHANGED: nothing
TODAY: Launchpad X / Mini MK3 note layout and colours only; the light device is picked by hand at every launch and not remembered (area-controls TODAY 23, 24; U12, K4).
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
STATUS: CORRECTED parts b and h; c and d touched; one list of names is ordered
HIS: L114; also L55, L112, L21, L35, L38
RULE: His words: "we will record to clip or record show. That’s what the 2 recordings are called. All else is gone. Show Recording Review should be called Review for short is a good name. Do you have a better name for this? Macro’s panel should be called Macros. We need a solid list with what we call everything. You create and keep one and I will ask questions and you can give me the truth, which will be this doc. You will use this for comms with me and to name all features in the app, menus and manual(later)." So: (a) "Action" replaces "routine" everywhere; new ones are "Action 1", "Action 2" until renamed. (b) The two recordings are "Record to Clip" and "Record Show"; no other name for a recording is read anywhere ("take" goes); what else "All else is gone" removes is assumption J-1. The screen where a show recording is looked at is "Review", short for "Show Recording Review", until he answers the name question (QUESTIONS BACK). (c) One name per thing, the same in talk and on screen: the clip's two modes are "BPM mode" and "Timeline mode", his words (L11, L21) (assumption J-7). (d) The rows of Review are "tracks"; what the slider source is called follows his answer to 205 (L112; topic I). (e) "Preset" alone means an effect's preset; MilkDrop keeps its presets; the layer's Feedback hint reads "Feedback style". (f) "Cue" means the cue buttons; added by his message: "Master Cue" (L38) and the toggle between cue mode and preview mode (L35; topic B). (g) "Mapping" means the keyboard and MIDI mapping only. (h) The macro panel is titled "Macros". (i) No menu entry is called "Stop All"; the button and the mapping entry that switch every action off are "Stop Actions" (L72). Harmony keeps ONE list of names, built from these papers; it is the truth for talk with him, for every name in the app and its menus, and later for the manual.
CHANGED: (b) "recording (Record, Recordings)" gains his two names and "All else is gone"; "Review" is confirmed and its long form added, with his question back; (h) "Dashboard stays" is replaced by "Macros"; (c) "keeps Resolume's label BPM Sync on screen" is replaced by one name for talk and screen (INFERRED from "to name all features in the app"); (d) the slider-source half moves to 205 (L112); (i) "gets a truthful name with the pictures" is replaced by L8 and L72; added: the one list of names
TODAY: "routine" in about 20 UI files; "take" on the Record tab; panel title "Dashboard"; clip mode label "BPM Sync"; menu entry "Stop All" only closes the mapping screens (area-controls TODAY 7, 30; U14, U20; O4, O5).
@@END
@@ITEM R198
TITLE: Undo: what Cmd+Z touches and what it never touches
STATUS: STANDS -- nothing in his message says otherwise
HIS: none
RULE: Cmd+Z never changes anything that is played live: firing a clip or a column, a layer's X, play, reverse, pause, transparency, bypass, solo, the tempo bar, every control of the Clip tab that changes how the clip plays, an action's on / off button, an ignore lamp, a cue button, a change in the keyboard and MIDI mapping, an output screen's settings. Cmd+Z does undo what is built: adding, removing or moving a clip, a layer, a column or a deck; cutting, pasting or Option-dragging a clip (L4, L5); adding, removing or re-ordering an effect; deleting, cutting or pasting an action; loading a preset. A deleted layer comes back with nothing playing on it. New and Open clear the Undo history. The controls his message adds follow the same split (assumption J-15).
CHANGED: nothing
TODAY: a fire, B, S and X are Undo steps ("Trigger Clip", merged per layer); the undo-live lane is on paper, not built (area-controls TODAY 17; O6, M4).
@@END
@@ITEM R199
TITLE: The keyboard and MIDI mapping: how it behaves
STATUS: CORRECTED part f (mapping files are wanted); d confirmed by L72; the rest stands
HIS: L116; also L72, L120, L92
RULE: (a) A key or pad put on an action's button or a cue button belongs to that action or button, not to its place in the row: a copy has no key, and deleting the action frees the key. (b) A key that is already in use moves to the new target and the screen shows what it was taken from; every entry can be removed; a button can have a key and a pad both, and the screen lists every one. (c) A held key fires once; only the nudge and tempo keys repeat while held. (d) "Stop Actions" stops every action at once; it is an entry of the mapping and a button on screen (L72: "We need a global stop actions button that stops all actions. The tempo stop button stops all actions as well as everything else."); how the sliders go back is topic D's (the global glide, L64, L67). (e) A knob has no "let go": a slider that an action or a signal moves is his while he turns the knob and for a quarter of a second after, then the action or the signal takes it back; the slider jumps to the knob at his next turn. (f) His words: "we want mapping files" (L116). The mapping lives in the show and on the computer, another show's mapping can be imported from that show (his earlier words, BD:887-897), AND it can be written to a mapping file and read from one: "Export Mapping..." writes the open show's whole keyboard and MIDI mapping to a file; "Import Mapping..." reads a file and replaces the open show's mapping after asking; a file or a show that holds no mapping changes nothing (what a file holds: assumption J-2). (g) Controllers plugged in after the app has started are found without a restart, and two controllers of the same kind are told apart. (h) A change in the mapping is not an Undo step. (i) While a box for typing a name or a number is open, the keys type into it; Return or Esc gives the keyboard back. (j) While the mapping screen is open the show and the outputs go on, and no key or pad fires anything until it is closed.
CHANGED: (f) "today's Export Bindings... and Import Bindings... files go" is replaced: files stay, as mapping files, beside the import from another show; (d) is now his own word (L72); (e) "normal knob" is the only kind (207 c, L120); nothing else
TODAY: the mapping is never saved except through Export / Import Bindings files in ~/Library/Audio-DNA/bindings; the show file's "keys" block is written empty; no entry can be removed; a taken key moves silently; inputs are listed once at launch; no device is stored per entry (area-controls TODAY 12-14, 19-22; U4, U7, U9, U13, U16; M2-M5, M7; K1, K2).
@@END
@@ITEM R227
TITLE: The keys the app keeps for itself; the spacebar
STATUS: CORRECTED by one addition (the spacebar is Tap Tempo); the rest "all good"
HIS: L118
RULE: His words: "R227 all good. Spacebar is typically tap tempo". (a) A key held with Cmd belongs to the app (Save, Open, Undo, Cut, Copy, Paste, the output keys) and cannot be mapped. (b) Esc and the Delete key belong to the app: Esc leaves the keyboard and MIDI mapping screen, Delete removes what is marked. Every other key can be mapped, the spacebar included. Added: out of the box the spacebar is mapped to Tap Tempo in the live window; it can be put on something else like any key. While Review is open the spacebar is Review's own: it stops the playback (his earlier words, BD:1090) and starts it again (assumption J-3).
CHANGED: added to (b): the spacebar comes mapped to Tap Tempo; nothing removed ("all good")
TODAY: no Cmd key can be mapped; Space, Delete and Backspace are handled nowhere, so Space does nothing until mapped; nothing is mapped out of the box (area-controls TODAY 4, 5; U15).
@@END
@@ITEM R200
TITLE: Tooltips, the manual, OSC
STATUS: CORRECTED part c through his L128 (OSC is planned); a and b stand
HIS: L128, L114, L124
RULE: (a) Every button, slider and list gets a tooltip, written when its part is built, in the words of the list of names; the tooltip switch is remembered between launches. (b) A manual is written after the builds ("manual(later)", L114), opens from a Help menu, uses the list of names, and its first chapter is lining the picture up with the sound. (c) His answer to 215 is "plan all of these" (L128), and OSC is its letter a: OSC is planned, not left as it is; what is planned for it is topic K's (assumption J-10). The addresses that say routine say action. The app's own remote-control port for test tools is not his concern and keeps working.
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
STATUS: REPLACED by 207 c (L120): there is no endless-knob setting
HIS: L120
RULE: A knob only turns: it sets the slider to the knob's own position. There is no endless-knob setting, so a knob without end stops is not supported in this build (assumption J-4). If he asks for endless knobs later, this rule returns: the first turn starts from where the slider stands.
CHANGED: the whole item: it mended a setting that 207 c removes
TODAY: a relative CC keeps its own count starting at 0.5 and jumps the slider on the first turn; settable only by hand-editing a file (area-controls TODAY 15; K1).
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
HIS: L8, L114; for the tempo entries L13, L17, L31, L72
RULE: laid out by Harmony where it fits in the correct area; its look is settled in the UI redesign (L8). The names are in the NAMES section. The tempo bar's entries take his own words, not "Beat ...": "Tempo play", "Tempo pause", "Tempo stop" (his "tempo play", L17, L31; "pause is pushed on the tempo bar", L13; "tempo stop button", L72), "Tempo /2", "Tempo x2" (L13), "Tempo -", "Tempo +".
CHANGED: the drawn variants go; "Beat play", "Beat pause", "Beat stop" become "Tempo play", "Tempo pause", "Tempo stop" (INFERRED from his own wording)
TODAY: the entries do not exist; the adopted tempo-row plan names them "Beat play" and so on (area-controls part 2, BD:1069 line; K6).
@@END
@@ITEM P33
TITLE: The keyboard and MIDI mapping screen itself
STATUS: DROPPED as a picture (L8)
HIS: L8
RULE: laid out by Harmony where it fits in the correct area; its look is settled in the UI redesign (L8). What it must be able to do is fixed elsewhere: reach every button and slider (206 A), show every entry and remove any (R199 b), show no settings per entry (207 c).
CHANGED: the three drawn variants go
TODAY: two full-window covers on a made-up grid of fixed boxes, no list, no remove (area-controls TODAY 8-12; U2, K5).
@@END
@@ITEM P34
TITLE: Where a message appears
STATUS: DROPPED as a picture (L8); one function point taken from L68
HIS: L8, L68
RULE: laid out by Harmony where it fits in the correct area; its look is settled in the UI redesign (L8). Function, from his paste message "where the user needs to say OK they understand" (L68): a message that appears by itself stays until he clicks OK, sits on the app's main window and never on an output (assumption J-6). The warning of L65 and the marks of L80 sit inside their own screens.
CHANGED: the three drawn variants go; "stays until OK" is taken from L68 for all messages that appear by themselves (INFERRED: L68 is about one message)
TODAY: the recording messages are a line in the Record tab, not seen from another tab (slice text; area-recording, not re-read).
@@END

## ASSUMPTIONS
@@ASSUME J-1
ABOUT: R197, 206
TEXT: I assume "All else is gone" means the app has only two recordings, Record to Clip and Record Show, and no other word for a recording. A film of the show is part of Record Show, not a third recording.
WHY: L114 "All else is gone" can be about the names only or about the kinds of recording; L6 and L88 speak of the show recording's picture without naming a third kind.
ALT: b) Only the other NAMES go; every kind of recording the app has stays, renamed. c) Everything else goes outright, a film file of the show included.
IF-WRONG: REBUILD a recording kind removed or kept by mistake
ASK: YES his words read two ways and it decides what the recording build contains
@@END
@@ASSUME J-2
ABOUT: R199, R148
TEXT: I assume a mapping file holds the whole keyboard and MIDI mapping of the show that is open, keys, pads and knobs together, and nothing else. Importing one replaces the open show's mapping after asking.
WHY: L116 "we want mapping files" says files exist, not what one holds or whether an import replaces or adds.
ALT: b) Separate files: one for the keyboard, one per MIDI controller. c) An import adds to the mapping you have and only overwrites keys that clash.
IF-WRONG: SMALL the file layout can be split later
ASK: LINE a real choice he will most likely wave through
@@END
@@ASSUME J-3
ABOUT: R227
TEXT: I assume the spacebar is Tap Tempo in the live window out of the box, and you can map it to something else. In Review the spacebar stops and starts the playback, as you wrote before.
WHY: L118 "Spacebar is typically tap tempo" follows "all good" on a text that gives the spacebar to the review screen; his earlier words (BD:1090) make it stop the playback there.
ALT: b) The spacebar is always Tap Tempo and cannot be mapped to anything else. c) The spacebar is Tap Tempo in Review as well, and Review is stopped with its button or the mouse.
IF-WRONG: SMALL one key's default
ASK: LINE two statements of his, most likely meant together
@@END
@@ASSUME J-4
ABOUT: 207, D33
TEXT: I assume your knobs and faders have end stops. A knob that turns forever (an endless knob) will not work until you ask for it, because your answer removes its setting.
WHY: L120 picks c ("a knob only turns"); the page's default said an endless knob cannot work without its setting; he names no controller.
ALT: b) Endless knobs must work: the app keeps that one setting per knob. c) The app finds out by itself whether a knob is endless.
IF-WRONG: STAGE a knob of his controller would move its slider wrongly
ASK: LINE he chose c; this is its one hidden consequence
@@END
@@ASSUME J-5
ABOUT: 209
TEXT: I assume no message appears by itself except the ones you named. A window you opened yourself may still tell you what it did, for example Collect Media saying how many files it could not copy.
WHY: L122 "already dicsussed above" points at L65, L68 and L80 and does not say whether any other case may speak.
ALT: b) Strictly only the named ones: a window you opened says nothing either. c) Also by themselves: a recording that was saved without sound or cut short, a camera or an imported shader that failed.
IF-WRONG: SMALL a message more or less
ASK: LINE closest to his earlier "Nothing else."
@@END
@@ASSUME J-6
ABOUT: P34, 209
TEXT: I assume every message that appears by itself stays on the app's main window until you click OK, and never shows on an output screen.
WHY: L68 asks for an OK on the paste message only; nothing says how the other messages go away.
ALT: b) Only the paste message needs an OK; the others are a line that stays until clicked or fades.
IF-WRONG: SMALL how a message leaves
ASK: LINE taken from one message of his and applied to all
@@END
@@ASSUME J-7
ABOUT: R197
TEXT: I assume the clip's two modes read "BPM mode" and "Timeline mode" on screen, the words you use, and not Resolume's label "BPM Sync".
WHY: L114 wants one list used for talk and for every name in the app; the page kept "BPM Sync" on screen and "BPM mode" in talk; he writes "BPM mode" (L11, L12, L21).
ALT: b) The screen keeps Resolume's "BPM Sync" and the list of names notes both.
IF-WRONG: SMALL a label
ASK: LINE a name he reads all night
@@END
@@ASSUME J-8
ABOUT: 206, R199
TEXT: I assume a knob put on a clip's slider stays with that clip: it moves with the clip, and a copy has no knob. A knob on a layer's or a Global slider stays there.
WHY: 206 A makes every slider mappable; no word says what a knob on a clip's slider follows. Pads on clip cells follow the cell on screen (197 A), action keys follow the action.
ALT: b) The knob belongs to the cell: it drives the same slider of whatever clip sits there on the deck on screen. c) The knob drives that slider of the clip playing on the layer.
IF-WRONG: STAGE a knob would move another clip's slider, or none
ASK: LINE a real choice; it shapes how the mapping names a control
@@END
@@ASSUME J-9
ABOUT: 206, R199, R227
TEXT: I assume that in Review your mapped knobs and pads work only during a record over, where they move the controls being recorded. Otherwise they do nothing there and never touch the live show.
WHY: L92 "we can record over using the midi controller" needs the mapping to work in Review; the page said the mapping fires nothing while Review is open.
ALT: b) Mapped keys and pads work in Review all the time, on the recording's controls.
IF-WRONG: SMALL when the mapping listens in Review
ASK: LINE follows from L92; topic E owns the record over itself
@@END
@@ASSUME J-10
ABOUT: R200
TEXT: I assume planning OSC means that every button and slider that can be put on a key or a knob can also be driven by an OSC message, and nothing more for now.
WHY: L128 "215 plan all of these" includes OSC (letter a) without saying what is wanted from it.
ALT: b) Only the existing OSC messages are kept and renamed. c) OSC also gets its own side in the mapping screen with a port setting.
IF-WRONG: SMALL the size of the OSC plan
ASK: NO topic K owns question 215 and asks it there
@@END
@@ASSUME J-11
ABOUT: 208
TEXT: I assume pasting over a clip that is playing does not change the picture: the old clip plays on until that cell is fired again. Several clips copied together are pasted in the same arrangement, starting at the selected cell.
WHY: L4 says a paste may land on a cell "with something else in it"; it does not say what happens if that clip is playing, or how several clips land.
ALT: b) The pasted clip takes over at once, or on the next 1 in BPM mode. c) The layer goes empty when its playing clip is pasted over.
IF-WRONG: STAGE a paste during a show would change or blank a layer
ASK: LINE a real choice with a safe default
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
TEXT: I assume the new live controls are never undone by Cmd+Z: Master Cue, the cue and preview toggle, a layer's ignore toggles, an action's loop toggle, Stop Actions, the glide slider. Pasting or Option-dragging a clip is an Undo step.
WHY: His Undo rule (BD:764) lists live controls and ends with "etc."; L4, L5, L7, L35, L38, L64, L72 add controls it could not name.
ALT: none
IF-WRONG: SMALL one control in or out of Undo
ASK: NO his rule applied to new controls
@@END
@@ASSUME J-16
ABOUT: R199
TEXT: I assume an imported mapping keeps entries that point at something the open show lacks, such as a layer or an effect that is not there; they do nothing and the mapping screen shows them as missing.
WHY: L116 and his earlier import words do not say what happens when the two shows differ.
ALT: b) Such entries are dropped at import.
IF-WRONG: SMALL list housekeeping
ASK: NO internal; nothing fires either way
@@END
@@ASSUME J-17
ABOUT: R197, P32
TEXT: I assume the menu entry that only closes the mapping screens goes; the screen is left with Esc or its own close button.
WHY: L8 leaves layout to Harmony; the page promised that entry a truthful name with the pictures.
ALT: b) It stays as "Close Mapping".
IF-WRONG: SMALL a menu entry
ASK: NO layout and wording
@@END

## QUESTIONS BACK
@@ANSWER 207-hold
HIS: L120
ANSWER: My recommendation: keep one setting per pad, "press / hold", off unless you switch it on. For it: hold lets visuals be played like an instrument. A band's VJ holds a pad for a white flash or the logo exactly as long as the hit or chord lasts; it is gone on release, with no second press to forget. A DJ holds a pad through a build-up to keep an effect or an action on and lets go on the drop, like the hold side of a mixer's effect lever. Against: a short hold on a BPM mode clip ends before the 1 and shows nothing; it is one more setting; a lost release could stick. Without it, two presses do the same.
@@END
@@ANSWER review-name
HIS: L114, L72
ANSWER: My pick: "Studio". Musicians already pair Live and Studio: live is the stage, the studio is where you listen back to what you recorded and make things from it, which is what this screen does. It also survives dictation: "Review" and the cue system's "Preview" sound alike and would get mixed up in speech and in voice typing. Alternative 1: "Replay", your own earlier word ("the replay window"); clear, but it sounds like "play" on the tempo bar and names only the watching, not the making of actions. Alternative 2: "Review", your current word; short and true, good if the nearness to "Preview" does not bother you. Until you answer I write "Review".
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
@@NAME Stop Actions
MEANS: The button, and its mapping entry, that stops every action at once.
SOURCE: his words L72 ("a global stop actions button that stops all actions")
@@END
@@NAME Record to Clip
MEANS: The recording that lands as a new clip in a layer.
SOURCE: his words L114 ("we will record to clip or record show")
@@END
@@NAME Record Show
MEANS: The recording of a whole performance, which is looked at afterwards in Review.
SOURCE: his words L114
@@END
@@NAME Review
MEANS: The screen where a show recording is watched, mended and turned into actions; short for Show Recording Review.
SOURCE: his words L114; his question for a better name is open (answer review-name proposes Studio)
@@END
@@NAME track
MEANS: One row of the Review screen: one recorded button or slider.
SOURCE: his words L86, L89
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
MEANS: The clip mode in which the clip plays by its own duration and speed.
SOURCE: his words L21 ("in timeline mode")
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
@@NAME Tempo /2 and Tempo x2
MEANS: The two mapping entries that halve and double the tempo.
SOURCE: his words L13, L14 ("/2 and x2")
@@END
@@NAME Tempo - and Tempo +
MEANS: The two mapping entries that lower and raise the tempo by a step.
SOURCE: Harmony's pick (the adopted tempo-row plan, by area-controls part 2)
@@END
@@NAME Nudge back and Nudge forward
MEANS: The two mapping entries that move the 1 a hair earlier or later.
SOURCE: his words L13 ("it moves the one a hair forward or back"); the entry wording is Harmony's pick
@@END
@@NAME Resync
MEANS: The button, and its mapping entry, that sets the 1 at the press.
SOURCE: his words L12 ("unless resync is clicked")
@@END
@@NAME Master Cue
MEANS: The toggle that puts the global effects and actions into the cue.
SOURCE: his words L38 ("called master cue"); topic B owns what it does
@@END
@@NAME cue button
MEANS: The button per layer that puts that layer into the preview monitor.
SOURCE: his words L34 ("next to each cue button for each layer"); topic B owns it
@@END
@@NAME ignore actions
MEANS: The toggle on a control, and on a whole layer, that keeps actions from moving it.
SOURCE: his words L7 ("ignore actions toggle on the layer"), BD:1056-1058
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

## CONFLICTS
- The spacebar. L118 "R227 all good. Spacebar is typically tap tempo" against his earlier BD:1090 "or presses the spacebar, it will stop playing but nothing else happens" (the review screen). Not a real conflict in my reading: "all good" accepts the text that gives the spacebar to Review while it is open, and Tap Tempo is the live window's. Shown to him as one line (J-3), not as a question.
- Messages. BD:771-774 "Nothing else." and BD:720 "the only fail message will be a failed save. remove all others" against L68 "the small messages is displayed where the user needs to say OK they understand", L65 "show a warning in save screen" and L80 "indicates this to the user". The later words win and add three cases (item 209); whether anything further may speak is J-5.
- Names. L114 "Show Recording Review should be called Review for short is a good name." against his earlier BD:1090 "the replay window": the later word wins; he asks for a better one himself (answer review-name).
- L114 "All else is gone" against L6 "record a very low resolution show recording" and the page's Video box for a film of the show: one more thing that records, or a part of Record Show (J-1, ASK YES).

## NOT DONE / UNSURE
- Which controller he plays with is still unknown (214 default; L92 says only "the midi controller"). It decides pad lights, whether J-4 (endless knobs) bites, and how strong the hold argument is for him. Cheapest: one question, "which controller?", if Harmony wants to spend it.
- "Snapshot" in 206 A's list: L94 "Do we need snapshot?" is about what a show keeps (topic F); I did not settle which snapshot he means. Cheapest: topic F's paper.
- L94 "so will the comp": I read "comp" as the computer (as in L6 "How much comp resource"), not as "composition"; if it meant composition it would touch 210. Topic F owns that line.
- Stop Actions: whether "stops all actions" (L72) leaves the action buttons on or switches them off, and how it sits with question 180's default, is topic D's; item R199 (d) only fixes that the button and the entry exist.
- The edges of L4 and L5 (what a copy carries besides actions; pasting across decks; Option-drag onto a playing cell) are written here only as far as 208 needs them (J-11); the topic that holds the general points should own them.
- The recollection that Resolume itself offers a hold ("piano") choice for a mapped button is NOT checked and is left out of the answer; cheapest: he looks at one mapped pad in his Arena, or a Researcher reads the manual.
- TODAY lines on messages (209, P34) and D32 lean on the slice's sources (area-recording, area-show-decks), not re-read.
