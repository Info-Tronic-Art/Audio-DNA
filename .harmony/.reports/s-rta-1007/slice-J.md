# SLICE J -- The keyboard and MIDI mapping, menus and messages
The items of this topic exactly as they stood on the page shown to Boris on 2026-10-05 (source: s-rta-1005/boris-all-items.json), and for each the lines of his message of 2026-10-07 that name it (line numbers of boris-msg-numbered.txt). "NOT NAMED by him" on a QUESTION = its default is taken by his first line (L1). "NOT NAMED by him" on a READING = it stands as shown unless words of his elsewhere in the message say otherwise. The lists "DECIDED WITHOUT ASKING" and "COMES NEXT AS PICTURES" he did NOT read (L130).

## ITEM IDS OF THIS SLICE (every one needs an @@ITEM block in your paper)
206 207 208 209 210 214 R148 R197 R198 R199 R227 R200 D32 D33 D34 D35 P32 P33 P34

## QUESTIONS (6)
### Question 206 -- What can be put on a key, pad or knob   [NOT NAMED by him]
About: what can be put on a key, a pad or a knob
Situation: You want to ride one effect's slider from a knob on your controller, so you open the keyboard and MIDI mapping. Today what can be mapped is a fixed list: clip cells, columns, a few buttons, and only two sliders (Master Opacity and Master Signal).
- A (THE DEFAULT) [Resolume says so]: Every button and every slider can be mapped. A key or pad for a button (a clip, a column, an action, a cue button, an ignore lamp, the tempo row, Record, Snapshot); a knob for any slider. Only the output screens' settings stay out.  -- why the default: [Resolume says so] Its manual says that a mouse, a MIDI controller and OSC reach every parameter alike (read for me in its manual, not tried); and your ignore lamp on every slider and button assumes that every one of them can be driven.
- B: Buttons, plus the faders you use most: the two masters, each layer's fader and the macro knobs. The ignore lamps stay on the mouse.
- C: As today, plus the new buttons (actions, cue, the tempo row). The ignore lamps stay on the mouse.
Why it was asked: It sizes the build of the keyboard and MIDI mapping, and it decides whether you can ride an effect's slider from a knob on stage.
Source (Harmony-side): area-controls.md U1, M1, M6; area-recording.md H11; area-outputs.md H11; facts-resolume-emulate.md D5; BD:1056-1058, BD:1099

### Question 207 -- A mapped pad: press or hold   [his lines: L120]
About: a mapped pad: press or hold
Situation: You map a pad to a clip. Four settings for a mapped pad or knob exist inside the app and no screen shows them: hold to play (the clip plays only while you hold the pad), how hard you hit sets the clip's opacity, endless knobs, and "this clip" instead of "this cell" (question 197).
- A (THE DEFAULT) [mine]: The keyboard and MIDI mapping shows two per entry: press or hold, and normal or endless knob. A held pad let go before a BPM-mode clip's "1" starts nothing. The other two stay as today, unseen (197 C would show "this cell / this clip").  -- why the default: [mine] Hold to play is what stage pads are for, and an endless knob cannot work without its setting; the other two nobody has asked for.
- B: All four are shown per entry.
- C: None: a key or pad only presses, and a knob only turns.
Why it was asked: You once asked: "What does this mean: Momentary pad released before its quantized beat: now cancels". It means exactly the hold case: a pad set to hold, let go before the clip's start line, starts nothing. The same choice applies to an action's button: with hold, an action runs only while you hold its pad.
His earlier words it rested on: "What does this mean: Momentary pad released before its quantized beat: now cancels"
Source (Harmony-side): area-controls.md U5, U6, O7, K3; area-clip-transport.md U-H23; area-actions.md H10; docs/claude/performance-controls.md:73, 79

### Question 208 -- The Edit menu and copying clips   [NOT NAMED by him]
About: the Edit menu, and copying clips
Situation: You want to copy a clip into another cell. You asked for an Edit menu; today Cmd+X clears the selected clips, and clips cannot be copied or pasted at all.
- A (THE DEFAULT) [mine]: Edit holds Undo, Redo, Cut, Copy, Paste and Delete. They work on what is marked: an action (R145) or the selected clips. Clips get real copy and paste, with their actions (165 A); Cmd+X on clips becomes a cut that can be pasted.  -- why the default: [mine] The default you accepted with 165 speaks of a copied clip, and today there is no way to copy one.
- B: Edit holds Undo and Redo, and Cut, Copy, Paste and Delete for actions only. Clips stay as today: Cmd+X clears, no copy, no paste.
Why it was asked: R145 says that copy and paste of clips are not added; this question is where you can add them. Undo and Redo move from the Composition menu to the Edit menu either way. Under A the Delete key empties the selected cells, as Cmd+X does today. Undo and Redo name what they undo, as today ("Undo Remove Deck").
His earlier words it rested on: "we should have an edit menu"
Source (Harmony-side): area-controls.md U10, U11, C4; area-show-decks.md U-12; BD:712-713, BD:1079; src/MainComponent.cpp:4122-4136

### Question 209 -- Which messages the app may show   [his lines: L122]
About: which messages the app may show
Situation: You paste an action onto a clip that lacks one of its effects, and the app wants to tell you what it left out (184 A). You wrote two things about messages, a day apart; both are quoted below. First: a failed save is the only message. Then: a clip recording or a show recording that was not saved must also be shown, and nothing else. I read the later one as adding to the first. With "not enough HDD space to record" that makes four messages that appear by themselves.
- A (THE DEFAULT) [your words + mine]: Only those four appear by themselves: a failed save; a clip recording or a show recording that was not saved (a film, or one that a full disk ended, counts); "not enough HDD space to record". A window you opened yourself may answer you there.  -- why the default: [your words + mine] Closest to your rule: nothing pops up over the show; a window you opened may still tell you what it did.
- B: Strictly nothing else: a paste that does not fit simply does not happen, and a recording with the wrong show open simply does not open.
- C: Also by themselves, because you would otherwise not know: a recording that was saved but has no sound or was cut short; an imported shader or a camera that failed. This goes against your "remove all others".
Why it was asked: Without your answer each new part has to guess whether it may tell you something. Where a message sits comes with the pictures (today the recording ones are a line in the Record tab, which you cannot see from another tab). A window you opened yourself: the paste that left a part out (184 A), the review screen naming the show a recording needs (R173), Collect Media saying how many files it could not copy, a question before something is replaced.
His earlier words it rested on: "the only fail message will be a failed save. remove all others" / "If the user is recording a clip live, and that is not saved in a message should show. If the user is recording the show, that is not saved, and that should be shown. Nothing else." / "We don't need any text indicating what has happened or what has happened. That is something that happens online and is not necessary in this application. It is extra overhead and bloat. Please remove it cleanly and completely." / "remove the list entirely and cleanly"
Source (Harmony-side): area-recording.md H21, M2, O7; area-actions.md O15; area-sources-auto.md M6; BD:704-711, 720-721, 771-777; the order of his two sentences: BD:718-721 (2026-10-03) then BD:771-774 (2026-10-04, REPLACES "12 b"); the disk line BD:777

### Question 210 -- Composition or show on screen   [NOT NAMED by him]
About: Composition or show
Situation: You look at the menu bar to save your show, and it says "Composition": the Composition menu, New Composition, the tab Compositions, and the tab that holds the global effects. You write "show" for the file and "the global tab" for that tab.
- A (THE DEFAULT) [your words]: Your words: "Show" for the file everywhere on screen (a Show menu, New Show, the tab Shows) and "Global" for the tab.  -- why the default: [your words] Closest to your words in the messages you have written.
- B: Resolume's word, as today: "Composition" everywhere on screen. In my texts to you I keep saying show and global tab.
Why it was asked: It is only a name, but it is on the menu bar and on a tab you look at all night.
His earlier words it rested on: "so the global controls all sliders and buttons in the global tab, and all the layer with it's own actions"
Source (Harmony-side): area-show-decks.md U-13; area-effects-signals.md E14, 1.02 (InspectorPanel.h:87-90; MenuBarModel.cpp:70-95)

### Question 214 -- Pad lights: which controllers   [NOT NAMED by him]
About: pad lights: which controllers you play with
Situation: You plug in the pad controller you play with. Today the app lights the pads of a Launchpad X or a Launchpad Mini MK3 (the layout and the colours are made for those two): the grid shows the clips of the deck on screen, and you pick the device in Preferences each time you start the app. Any other pad controller, an APC included, is untested: its pads may show wrong colours or nothing. I do not know which controllers you play with.
- A (THE DEFAULT) [as today]: Nothing new for now: pad lights stay as today, and the new buttons (actions, cue, the tempo row) get no lights until you name a controller.  -- why the default: [as today] Lights are built per controller, and without a name there is nothing to build them for.
- B: You name it, for example "214 b: APC40", and its lights are planned with the build of the keyboard and MIDI mapping: clips, actions that are on, cued layers.
Why it was asked: It decides whether the build of the keyboard and MIDI mapping includes lights for the new buttons, and for which controller.
Source (Harmony-side): area-controls.md (pad lights; which controllers: UNKNOWN); src/midi/MidiOutputHandler.h:10, 19-20 (read by me: "Velocity values target Launchpad X/Mini MK3"; APC40 only in a comment); the device is picked by hand at every launch: by the two seats (PreferencesDialog.cpp:119-141, AppSettings.h:18-19)

## READINGS (6)
### R148 (about: R120: the name)   [NOT NAMED by him]
Your name for it: the keyboard and MIDI mapping. (a) Today the menu is titled "Shortcuts"; its entries read "Edit Keyboard Shortcuts...", "Edit MIDI Mappings...", "Stop All", "Export Bindings..." and "Import Bindings..."; the two screens they open are headed "Keyboard Binding Mode — Click a target, then press a key" and "MIDI Learn Mode — Click a target, then send MIDI"; the two file windows are titled "Export Bindings" and "Import Bindings". (b) From now on all of it is called "keyboard and MIDI mapping": in everything I write to you from today, and in the app when that part is next built. The words "shortcuts", "bindings" and "MIDI learn" go; the exact new wording of each entry comes with the pictures. (c) The app's own notes also say "mapping" for plugging a signal into a slider, so this one is always written in full.
(label: "his words (the name); what it reads today VERIFIED in the code; a title Mapping Editor also exists in the code and cannot be reached on screen today (by the app inventory, APP-INVENTORY.md:104)")
(source: "BD:1099; src/ui/MenuBarModel.cpp:10, 155-161; src/ui/BindingOverlay.cpp:68; src/ui/MidiLearnOverlay.cpp:95; src/MainComponent.cpp:7342, 7357; src/ui/MappingEditor.cpp:100")

### R197 (about: names on screen)   [his lines: L114]
Names that follow from your words, and mine where no word of yours names the thing; say so if one is wrong. (a) "Action" replaces "routine" everywhere you can read it. New actions are named "Action 1", "Action 2" and so on until you rename them, as presets are (your 104). (b) On the Record tab and in the list the word is "recording" (Record, Recordings) and the word "take" goes from the screen (your words: "A take = action recording."); the screen where you look at one is "Review". (c) The clip's mode keeps Resolume's label "BPM Sync" on screen; in my texts I keep your word, BPM mode. (d) "Timeline" is the name of the other clip mode and of a slider source (question 205); the rows of the review screen are called tracks, as you call them. (e) "Preset" alone means an effect's preset; MilkDrop keeps its own "presets" (151 A); the layer's Feedback list (Custom, Zoom In, Spiral, Drift, Kaleidoscope, Echo, Stretch) keeps its entries; its hint reads "Feedback preset" today and will read "Feedback style", so that "preset" belongs to effects and MilkDrop only. Say "R197 e keep" if the hint should stay. (f) "Cue" means the cue buttons (R179). (g) Plugging a signal into a slider is never called mapping on screen: "mapping" is the keyboard and MIDI mapping (R148). (h) The macro panel's title "Dashboard" stays. (i) The entry "Stop All" in the menu that is titled "Shortcuts" today (R148), which only closes the mapping screens, gets a truthful name with the pictures.
(label: "his words (action for routine; preset; keyboard and MIDI mapping; A take = action recording); the default name of an action, recording for take, Review, and what stays: INFERRED; today's texts VERIFIED by the area sheets")
(source: "area-actions.md H15, W7; area-recording.md H27; area-effects-signals.md E14; area-sources-auto.md H17; area-controls.md U3, U14; area-tempo.md U-HIS-18; BD:923-924, 960-962, 1014-1016, 1095, 1099")

### R198 (about: Undo)   [NOT NAMED by him]
Your rule, with your list: "cmd-z does not affect anything in layer strip: play, play reverse, pause, transparency, bypass, solo, etc." (a) Mine, what "etc." covers once everything is built: firing a clip or a column, a layer's X, the tempo row, every control of the Clip tab that changes how the clip plays (pause, speed, direction, loop style, mode, length, Beat Repeat, Random, the in and out points; none of them is an Undo step today, and that stays), an action's on / off button, an ignore lamp, a cue button, a change in the keyboard and MIDI mapping, an output screen's settings. (b) Cmd+Z does undo what you build: adding, removing or moving a clip, a layer, a column or a deck; adding or removing an effect; deleting, cutting or pasting an action; loading a preset. A layer you delete comes back with Undo, with nothing playing on it (your words). (c) This replaces the second half of R113 on the last page, where I wrote that Cmd+Z after a stop still undoes your last fire: by your own sentence a fire is not undone. Today the app still undoes a fire, B, S and X; that is mended. (d) New and Open clear the Undo history, as today.
(label: "his words (the rule and his list); what etc. covers INFERRED; today's behaviour VERIFIED by the area sheets (MainComponent.cpp:723-768, 5029-5039, 3167)")
(source: "area-clip-transport.md U-H15, O14; area-cue-layers.md O5, H19, W4, W5; area-actions.md H17, W2; area-controls.md O6, M4; area-show-decks.md M-3; BD:688-692, 736-738, 764-768")

### R199 (about: the keyboard and MIDI mapping: how it behaves)   [his lines: L116]
Mine, where no word of yours exists; how the screen looks comes as pictures. (a) A key or pad put on an action's button or on a cue button belongs to that action or button, not to its place in the row: a copy has no key, and deleting the action frees the key. (b) A key that is already in use moves to the new target, and the screen shows what it was taken from; every entry can be removed. A button can have a key and a pad both, and the screen lists every one of them. (c) A held key fires once; only the nudge and tempo keys repeat while held. (d) "Stop actions" (R120) switches every action off at once (their sliders go back, R129); it also gets a button on screen (R224 c). (e) A MIDI knob has no "let go": a slider that an action or a signal moves is yours while you turn the knob and for a quarter of a second after, then the action or the signal takes it back (158 A); the knob and the slider can then differ, and the slider jumps to the knob at your next turn. (f) Importing the keyboard and MIDI mapping of another show replaces yours after asking; a show that holds none changes nothing. Mine: today's "Export Bindings..." and "Import Bindings..." files go, because a show holds its mapping and you import from another show (your 81); say so if you want mapping files as well. (g) Controllers plugged in after the app has started are found without a restart, and two controllers of the same kind are told apart. (h) A change in the keyboard and MIDI mapping is not an Undo step. (i) While a box for typing a name or a number is open, the keys type into it; Return or Esc gives the keyboard back. (j) While you set up an entry, the show and the outputs go on, but no key or pad fires anything until you close the mapping screen, as today: it is for setting up, not for playing. Pad lights: question 214.
(label: "INFERRED (no word of his covers these); today's facts VERIFIED by the area sheet (no way to remove an entry; an import can erase all; inputs are listed once at launch; no device is remembered; the quarter second: Composition.h:147)")
(source: "area-controls.md U4, U7, U9, U12, U13, U14, U16, M2, M3, M4, M5, M7; area-actions.md H10, H22, M4; area-show-decks.md U-16, M-6")
(star: true)

### R227 (about: the keys the app keeps for itself)   [his lines: L118]
Mine, no word of yours covers it. (a) A key held with Cmd belongs to the app (Save, Open, Undo, Cut, Copy, Paste, the output keys) and cannot be mapped, as today. (b) Esc and the Delete key also belong to the app: Esc leaves the keyboard and MIDI mapping screen, Delete removes what is marked (R145). Every other key, the spacebar included, can be mapped; while the review screen is open the spacebar is its own (R185). Tell me if you have a key plan that needs Delete.
(label: "INFERRED; that no key held with Cmd can be mapped today: by the cover-controls seat (MainComponent.cpp:4158)")
(source: "area-controls.md part 1 items 4-5, U10, U15")

### R200 (about: tooltips, the manual, OSC)   [NOT NAMED by him]
Mine. (a) Every button, slider and list gets a tooltip, written when its part is built; the switch for tooltips is remembered (today it is back on at every launch). (b) A manual is owed to you (you wrote "We can just put that in our manual"): it is written after the builds, opens from a Help menu, and its first chapter is the one you named: lining the picture up with the sound (the nudge and the Delay). (c) OSC (messages from a controller program) and the app's own remote-control port (which test tools use) stay as they are; their routine addresses are renamed to action, and new controls get addresses when they are built. Whether you use OSC on stage: question 215 (a).
(label: "INFERRED; the manual is his word; no manual and no Help menu exist today (VERIFIED by the area sheet)")
(source: "area-controls.md U17, U18, U19; BD:800-801")

## DECIDED WITHOUT ASKING (4) -- he did NOT read these: each is still Harmony's assumption until his words settle it
### D32: A mapped pad that points at a cell beyond the last column of the deck on screen does nothing; one that points at an empty cell clears its layer, as a click on an empty cell does.
(decided by: area-show-decks.md M-2 (Layer.h:403, 410-419: INFERRED there, not run); R168.)

### D33: An endless knob starts from where the slider stands, so the first turn does not make the slider jump (today it starts from the middle and jumps).
(decided by: area-controls.md K1 (BindingManager.cpp:173, read there).)

### D34: A pad you hold: its release is kept in a recording like its press (today only the press is kept, so in a replay a held pad would never let go).
(decided by: area-controls.md K3, K8 (read there).)

### D35: Pad lights follow a press up to a quarter of a second late today, and a clip that waits for the "1" is lit like a loaded clip, not as waiting; both are mended when pad lights are built (question 214).
(decided by: area-controls.md C1, M8, K11 (MainComponent.cpp:439, 4372, 4416: every 8th tick of a 30 Hz timer; read there).)

## COMES NEXT AS PICTURES (3) -- he did NOT read these; his new rule on layout is L8
### P32: The keyboard and MIDI mapping: the menu's new title and the wording of its entries and of its two screens. Also the names of the new entries for the tempo row ("Beat play", "Beat pause", "Beat stop", "Tempo -", "Tempo +", "Tempo /2", "Tempo x2").
(variants that were to be drawn: a menu "Keyboard and MIDI Mapping" with "Edit Keyboard Mapping...", "Edit MIDI Mapping..." and "Import Mapping from Show..." | no menu of its own: one entry "Keyboard and MIDI Mapping..." in the Edit menu, which opens one window that holds all of it)

### P33: The keyboard and MIDI mapping screen itself.
(variants that were to be drawn: the real window with every mappable control tinted: click one, then press a key or move a knob | one screen with a Keyboard side and a MIDI side and a list of every entry, each with a remove button | today's two screens, renamed)

### P34: Where a message appears (question 209).
(variants that were to be drawn: a line in the top bar that stays until you click it | a box in the middle of the window | a line in the tab it belongs to)

