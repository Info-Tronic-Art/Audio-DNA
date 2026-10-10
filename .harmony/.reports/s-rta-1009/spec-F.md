# SPEC F -- The show file, decks and saving (s-rta-1009): the s-rta-1007 spec with Boris's answers to page 2 laid over it by wf/merge3.py (paper apply-F.md, ruling rule-F.md). This file wins over every older one.

## PAGE 2 ITEMS (the rule now for each item of page 2 in this topic)
@@ITEM 233
TITLE: Which click selects a cell and which click triggers it
STATUS: ANSWERED both ways are right; the cell that holds a clip is read from the same words
HIS: BF255 "Both are correct."; "you can select the cell which triggers it and selects it"
RULE: Every cell has two places to click. (1) THE NAME, at the bottom of the cell (on an empty cell: the place where a clip's name would be). A click there selects the cell and triggers nothing. On an empty cell: the layer keeps playing what it plays, the cell is the selected cell, and Paste puts the copied clip into it. On a cell that holds a clip: the clip is the selected clip, the Clip tab shows it, and it shows in the preview monitor by topic B's rule for a click on a name; the output does not change. (2) ANYWHERE ELSE ON THE CELL. A click there triggers the cell and also selects it. On an empty cell: the layer goes empty (when it does so, on the 1 under a BPM-mode clip, and what holds in a pause, are topic A's rules, page item 248 with them), and the cell is the selected cell from the moment of the click, so Paste then goes into it (F3-2). On a cell that holds a clip: the clip is triggered (when it starts is topic A's rule) and it becomes the selected clip, so the Clip tab shows it (Harmony's reading of his sentence for a cell that holds a clip: F3-1); the preview monitor is not changed by this click, because by topic B's rule the preview follows a click on a name, not a selection. (3) The selection made by either click takes the place of the selection before it, in the way the name click does with @@ITEM G2 as it stands. (4) A right-click on any cell selects it without triggering it and opens the small menu, with @@ITEM G2 part (4) as it stands. (5) A column trigger, and a trigger made by an action, select nothing: the selected cell and the Clip tab stay as they were (F3-2). Whether a key or a pad that sits on a cell also selects the cell it triggers is not ruled here: it is topic J's rule for what a pad does (its assumption J3-10). (6) What is selected is what Copy, Cut, Paste and the Delete key work on (@@ITEM G2 as amended). After a click of kind (2) on a clip, the selected clip is the clip just triggered, the one that is playing or about to play: a Cut, a Delete or a Paste then takes it off its layer, by page item 234.
CHANGED: The item as he read it made the click "anywhere else" on an empty cell trigger only; its way b made a plain click select and trigger. His words keep the name click of the item AND the select-with-trigger of way b. Against G2 part (4): a triggering click now also selects. Harmony's reading, not his words: that the same holds for a cell with a clip (his sentence is general but stands under an item about empty cells) and that this click leaves the preview alone: F3-1; that the selection lands at the click and that a column trigger or an action selects nothing: F3-2. Part (6) adds no rule: it says what part (2) and page item 234 give together. By the ruling: keys and pads are no longer ruled here (topic J's J3-10).
TODAY: Selecting is a click on the name bar of a filled cell (area-show-decks T20); whether an empty cell has a name bar to click, and whether a trigger click selects anything now: not checked. To build: the name place on an empty cell, selection on the trigger click, Paste into a selected empty cell.
@@END

@@ITEM 234
TITLE: Pasting over, cutting or deleting a clip that is playing
STATUS: ANSWERED against the item: the clip leaves its layer; the deck sentence is already the rule
HIS: BF256 "removes it from the layer strip, and it does not play"; "I change the deck, that does not change the clip"
RULE: (1) A clip that is pasted over, cut or deleted is gone from its cell. If its layer is playing it, it leaves the layer strip at that moment and no longer plays: the layer goes empty at once, in the middle of a bar too and also when the clip is in BPM mode, in the way the layer's X empties a layer (topic A's @@ITEM R168; this is not the trigger of an empty cell, which waits for the 1 under a BPM-mode clip) (F3-3). Its actions end with it as at an X (topic D's @@ITEM R161 as amended there). A clip that has been triggered and still waits for the 1 when it is pasted over, cut or deleted never starts ("it does not play"); what the layer is playing then plays on. Another clip that waits for the 1 on the same layer is not touched and starts on its 1. (2) After a paste the pasted clip sits in the cell and does not play: it plays when it is triggered (F3-3). (3) The same holds for every other act that takes a clip out of its cell or puts another in its place: an Option-drag onto its cell (it is a copy and a paste by his own words for it, with @@ITEM G3 as amended), and Remove Column and Remove Deck, which delete their clips (@@ITEM R190 as amended; read so: F3-4). (4) A clip that is neither playing nor waiting is pasted over, cut or deleted with @@ITEM G2 as it stands: nothing on any layer changes. (5) Undo keeps his earlier rule ("cmd-z does not affect anything in layer strip", binding-decisions.md:764): the Undo of a cut, a delete or a paste-over brings the clip back into its cell and does not put it back on its layer; an Undo or Redo that would take away a clip that is playing leaves that clip playing until something else is triggered on its layer or the layer is emptied. (6) A drag without Option moves a clip and deletes none: @@ITEM G3 as amended (F3-15). (7) HIS SECOND SENTENCE: changing the deck on screen never changes a clip that is playing, nor how it plays. This is already the rule: @@ITEM 197 ("Switching the grid to another deck ... never changes what is already playing") and his words of 2026-10-02 on decks as a box of clips; nothing is added to it.
CHANGED: The item said the picture does not change and the clip plays on; its way b said the layer switches at once to the pasted clip and goes empty on a cut or a delete. His words take neither whole: the clip is removed from the layer strip and does not play. "it" is read as the clip that was pasted over or deleted; read as the pasted clip, his words say outright that it does not play by itself: either way the layer goes empty and the pasted clip waits. He names pasting over and deleting; cut is read as a delete with a copy (INFERRED). Harmony's own, not his words: at once and not on the 1, and that the pasted clip waits for a trigger (F3-3); Remove Column and Remove Deck (F3-4); the plain drag (F3-15). The Undo part is his earlier rule, kept. Replaces G2 part (5), G3's "nothing that is playing changes" and R190 (b) and (d).
TODAY: Not checked what a playing layer does when its cell is replaced or cleared; a layer finds its clip by (deck, column) (area-show-decks U-10), so it most likely shows the new clip at once: to change, the layer must go empty and the pasted clip must wait. No copy or paste of clips exists yet (U-12). A deck switch changes only the grid: already so (T21, CLAUDE.md rule 15).
@@END

@@ITEM 235
TITLE: Which old show files are deleted
STATUS: ACCEPTED
HIS: box left empty = accepted as written
RULE: "The old show files" are every show saved up to the build and the app's safety copies of those shows, which are the copies in the app's backups folder (the copies a Save put there before it wrote over a show of an older shape: @@ITEM D23). Before any of them is touched they are listed to him, each by name, size and date. Only after his yes do they go to the Trash; they are moved, never erased, so he can take one back. Saved decks, recordings, presets and settings stay where they are (the saved decks are no longer opened by the app, with @@ITEM R225 (a) as amended). No work is done to open a show of the old shapes, with @@ITEM D23 as amended, topic D's @@ITEM R143 as amended and topic D's @@ITEM D13 as it stands. The listing is made before the build changes the show file's shape, and nothing is moved before his yes (F3-12).
CHANGED: The item is accepted as written. One word of it is read, not his: "safety copies" is taken as the copies in the app's backups folder (INFERRED: the old question F-13 said "backup copies"; the safety copy of old item 198, the copy written every few minutes, is not built, so no such file is among the old ones). Against the old blocks: the open question (F-13, D-7) is closed with its main reading; "at build start" is not in the item he accepted, so the moment is Harmony's (F3-12).
TODAY: His files: shows and decks in ~/Library/AudioDNA, settings / bindings / layouts in ~/Library/Audio-DNA, takes / snapshots / recordings in ~/Documents/Audio-DNA (area-show-decks T29); one real old-shape show exists (O-11, T33). Nothing of his was opened, listed or touched for this paper or this ruling. The move is a destructive act under RIG-RULES A3: named and checksummed first.
@@END

@@ITEM 236
TITLE: Snapshot: a save of the show exactly where it is
STATUS: ANSWERED he re-defines Snapshot against the item and against Harmony's answer
HIS: BF243 "a save of the show exactly where it is"; "push a shortcut button and save that to look out later"; BF257 "We keep the safe snapshot somewhere."
RULE: (1) WHAT IT IS. Snapshot is one command: one press saves the open show exactly as it stands at that instant, as a new snapshot. It asks nothing and opens no window, and it works whether the show has a file or not and whether it has unsaved changes or not. It is a menu entry and a button of the keyboard and MIDI mapping, which he puts on a key or a pad like any button (topic J's rule: its @@ITEM 206 as amended there); it comes with no key of its own, with topic J's @@ITEM R227 (c) as it stands (F3-13). It acts on the show of the live window; while Studio is on screen it does nothing (F3-13). (2) WHAT A SNAPSHOT HOLDS. (a) Everything a Save of the show holds (@@ITEM R188 (a) to (c) as amended), the presets too, taken from the show as it is at the press, unsaved changes included. Every slider is kept as it is set, with whatever moves it (a signal, an envelope, an action) still plugged in, not at the value it showed at that instant (F3-11). (b) In addition, what a show does not keep (Harmony's reading of "exactly where it is": F3-6): on every layer, which clip is playing and the frame it is on; the tempo number, where the beat stands in the bar, and whether the beat is running, paused or stopped; which actions are running and how far each has run (whether an action can be taken up part-way is topic D's). (c) A still picture of the output at that instant, at the size of the show's canvas: the composition with the global Transform, never with one screen's Delay, Brightness, Contrast or colour (topic G's @@ITEM R226 (b) and topic E's @@ITEM R187 (d) as they stand: their word "snapshots" means this picture) (F3-7). (d) Not in a snapshot: sound; what the computer holds for every show (@@ITEM R188 (d) as amended: each output screen's own settings and the picked audio input among them); the cue buttons and the cue sliders (F3-11). (3) WHERE IT IS KEPT. Each snapshot is a file of its own in a snapshots folder of the app, apart from the shows, named by itself with the show's name and the date and time of the press, and kept until he deletes it; there is no limit on their number (F3-9). Taking a snapshot never writes to his show file, never makes the show count as saved and never changes which show is the very last show. Media are not copied: a snapshot points at the same files the show does, and Collect Media does not take snapshots along (F3-14). (4) LOOKING AT ONE LATER (all of this part is Harmony's reading of "to look out later": F3-6). The app lists the snapshots, each with its picture, the show's name, the date and the time, newest first; they are not listed among the shows. Picking one opens it in place of the show that is open, after the same question every opening of another show asks (@@ITEM R189 (d)). Unlike a show (@@ITEM 196 as amended), a snapshot comes up as it was: the clips that were playing are on their layers, each on the frame it was on, the actions that were running are on, and the tempo is paused at the place in the bar where it stood. Nothing moves until the tempo's play, which runs everything on from there: clips in BPM mode are held by the pause as in any pause (topic A's rule), and the clips that are not in BPM mode are held on their frames in the same way for this one start. What the outputs do when it opens is what they do when any show is opened (topic G's page items 237, 264 and 265 as written). It opens as a show that has no file of its own: Save asks for a name, as for a new show, so the snapshot file is never changed, the show it was taken from is never written over by it, and the very last show stays what it was until that Save (F3-14). The app never opens a snapshot by itself. (5) IT NEVER DISTURBS THE SHOW. Taking a snapshot never makes the picture, an output or a control stutter, never pauses anything and never takes the keyboard away; the file is written, read back and checked away from the picture, as a Save is. A snapshot that worked shows nothing; one that failed shows the "Save failed" box (F3-10). (6) There is no separate command that saves only a still picture: the picture comes with every snapshot (F3-7).
CHANGED: The item kept Snapshot as "one small command that saves a still picture of the output to a file"; Harmony's answer said the same. His words replace both: "a save of the show exactly where it is with all the settings and the output and everything", by "a shortcut button", to look at later ("look out later" = look at later; "the safe snapshot" = the saved snapshot: both INFERRED). His "Perhaps" in BF243 is settled by BF257 ("Earlier in this document, I explained what a snapshot exactly is."): Snapshot is built as he explained it. His words give the first sentence of part (1) and the opening of (2a); "the output" is read both as which outputs were on (a Save holds that) and as a picture of the output. Harmony's own, not his words: what "exactly where it is" adds to a Save (2b) and everything that opening one does (4): F3-6, asked; the picture and no picture-only command (2c, 6): F3-7; sliders as set and what is left out (2a, 2d): F3-11; the place, the name, the keeping and no limit (3): F3-9; no key of its own and nothing in Studio (1): F3-13; an opened snapshot as a show without a file, the media and Collect Media (3, 4): F3-14; part (5): F3-10. Replaces the last sentence of R188 and its "snapshot pictures" in (f); closes F-16.
TODAY: "Snapshot" in the Output menu (src/ui/MenuBarModel.cpp:143, read, not run) saves a still picture; snapshots folder under ~/Documents/Audio-DNA (area-show-decks T29). No save of the show's playing state exists: a show keeps nothing of what plays (T9) and no clip pause (T8); a Save's effect on the message thread is not checked (old item 198 TODAY). To build: the playing-state block and its restore (with the one-time hold of clips that are not in BPM mode), the picture grab, the off-thread checked write, the list, the entry in the mapping.
@@END

## ITEMS (the items of the first page; a RULE that page 2 changed carries "[page 2, ...]" marks where it changed)
@@ITEM 196
TITLE: What a re-opened show shows
STATUS: DEFAULT (A) -- AMENDED after page 2 (196 1 by F)
HIS: L1, L94
RULE: When a show is opened, every layer is empty: a show does not keep what was playing. A clip that was paused when the show was saved keeps its pause and its frame: its cell shows a pause mark, and the clip appears on that frame, paused, when it is fired. The same holds at launch, when the app opens the very last show by itself (L94: "When you open the application, it opens to the very last show."): the show is there, its layers are empty, nothing plays until he fires a clip or a column. One exception: a snapshot that is opened comes up as it was taken, with its clips on their layers and the tempo paused (page item 236). [page 2, F: BF243 "a save of the show exactly where it is"]
CHANGED: nothing in the default; added from L94: the rule also covers the show the app opens by itself at launch.
TODAY: Layers are empty after every open (area-show-decks T9). No clip pause is saved and no cell shows a pause mark (T8; R-4 on paper only). To build: the saved pause per clip, the pause mark on the cell.
@@END

@@ITEM 197
TITLE: Pads and keys when you switch decks
STATUS: DEFAULT (A) -- AMENDED after page 2 (197 1 by F)
HIS: L1, L74
RULE: A key or pad that was put on a cell fires that same cell (layer and column) of the deck that is on screen. Switching the grid to another deck changes what the pad fires next and never changes what is already playing. A pad stays on its cell when a clip is moved, copied, cut or pasted: it fires whatever clip sits in that cell now. His "182 b" (L74) says the same for a layer's action: it fires the cells of the deck that is shown. A MIDI knob mapped to a clip's slider belongs to the cell in the same way: it moves that slider of whatever clip sits in that cell of the deck on screen (page item 244, way b; topic J's rule). [page 2, F: BF262 "b"]
CHANGED: nothing.
TODAY: This is what a ByPosition binding does now (area-show-decks T21, U-1, C6). A pad on a column beyond the shown deck's width does nothing while a pad on an empty cell clears its layer (M-2): not touched by his words.
@@END

@@ITEM 198
TITLE: A safety copy after a crash
STATUS: DEFAULT (A)
HIS: L1, L94
RULE: The app keeps a safety copy of the open show by itself every few minutes (three minutes to begin with: F-17), in a place of its own, apart from the show file. It never writes over his show file: only his own Save does that. A show that has never been saved to a file gets a safety copy too. Writing the copy never makes the picture, an output or a control stutter. After a crash or a power cut the next launch opens the very last show from its file, as he saved it (L94; a new, empty show when there is none: R189 h), and then offers the safety copy once, in one window with two choices: open the safety copy, or leave it. No offer is made when the copy is the same as the show as saved (for a show that was never saved: the same as a new, empty show). Open: the copy takes the place of the open show, as a show with changes that are not saved yet; his show file is still untouched until he presses Save, and a copy of a show that never had a file asks for a name at Save, as a new show does. Leave: the copy is discarded and is not offered again. After a normal quit no copy is offered and the copy is discarded. A start of the app made by a test or a tool writes no safety copy and offers none (F-23).
CHANGED: nothing in the default. Added from L94: after a crash the app opens the last show first and then makes the offer. Harmony's own, not his words: the number of minutes, what "open" and "leave" do, no offer for a copy that equals the saved show (F-17), that writing the copy must not disturb a running show (a build requirement), and the guard for test starts (F-23).
TODAY: No autosave, no crash recovery, no changed-since-save mark (area-show-decks T3, U-9, O-15). To build: a timed writer to a separate file through the verified writer, kept off the message and render threads (whether a Save blocks the message thread now: not checked), a clean-quit marker, the one-time offer at launch.
@@END

@@ITEM R188
TITLE: What a show keeps and what the computer keeps
STATUS: CORRECTED (a), (c), (d), (f), and the opening of the app -- AMENDED after page 2 (R188 1 by F, R188 2 by F, R188 3 by F, R188 4 by F, R188 5 by F, R188 6 by F, R188 7 by F, R188 8 by F, R188 9 by F, R188 10 by F)
HIS: L94, L48, L50, L52, L96, L7, L73, L64, L70, L35, L104, L106, L80, L116, L55, L128, L13, L14
RULE: (a) A saved show holds: its decks, clips, layers and effects with every slider; each clip's own pause; its actions, which of them are switched on, and each action's loop toggle (L64); the ignore lamps, and each layer's one switch against global actions (L7, L73; page item 225, topic D's rule) [page 2, F: item 225 accepted as written]; the presets themselves, so that they travel with the show file to another computer (L48, L50; which presets, and what opening a show does with them, is topic C's); the nudge amount (what it is after his words on the nudge, L13 and L14, is topic A's: its assumption A-9); the Master Signal fader, the master opacity, the global Transform, and the Global glide time, which is one slider for every smooth change of an action (page item 224, topic D's rule; that it is kept in the show: F-18) [page 2, F: item 224 accepted as written]; its keyboard and MIDI mapping; the window layout; the canvas size; which outputs were on when it was saved, Syphon counted as one of them (L96; page item 263; topic G's rule) [page 2, F: item 263 accepted as written]. Whether the show also keeps, on an effect, the name of the preset it was set from is ruled once, in topic C (question 178, L52): this paper follows it. (a, layout) "the show will hold the layout and so will the comp" (L94): opening a show puts the window into the layout that show holds; the computer also keeps the layout used last, and that one is used for a new show and for a show that holds none (F-10). The show holds one layout for each screen of the app: the live window and Studio now [page 2, F: BF245 "Let's go with studio. That's perfect."], and the further configurations he announces (L35: "We will have various configurations other than live and recording review mode") get theirs when they are built (F-20). A layout made on a bigger screen is fitted to the screen it is opened on. The View menu's "Save Layout...", "Load Layout..." and "Reset Layout" stay: Save Layout writes the arrangement on screen to a layout file; Load Layout puts the window into a file's arrangement, which from then on is the open show's (kept at its next Save) and the computer's last one; Reset Layout puts back the standard arrangement (F-20). (b) Saved in the show as well: his own signals, whichever topic I's rule lets him make, each with its shape and its length, and every envelope of both types, on the beat and on the playhead, any number of them, one per slider or shared by many (L104; page item 241), an envelope made on a clip being saved with that clip (topic I's rule); [page 2, F: BF261 "There should be two different types of envelopes that could work."] for every slider or button that uses a signal, which signal it uses and that user's own settings: one shot or looping, gain and falloff, and for a button its threshold and whether it is inverted (L106; topic I's rule); the macro knobs of clip, layer and global, with their names and what they drive. (c) Every show brings its own keyboard and MIDI mapping; opening a show switches keys and pads to that show's; a show that holds none leaves them as they are. A new show starts with the mapping of the show he saved last: the computer keeps a copy for that. The mapping of another show can be imported; mapping files exist as well (L116; topic J's rule). (d) The computer holds, for every show: the presets (they are kept with the app AND in the show file, L50); each output screen's own settings; MilkDrop's favourites; the controller whose pad lights were chosen; the tooltip switch; the tempo used last (question 175, default A; topic A); the layout used last (F-10); the copy of the mapping of (c); which show file is the very last show (R189 h); the audio input picked in the list Audio Input (page item 272; topic K's rule). [page 2, F: item 272 accepted as written] (e) Held neither by a show nor by the computer (a snapshot alone keeps the first two entries of this list, what is playing and the beat's state: page item 236): what is playing (question 196); [page 2, F: BF243 "a save of the show exactly where it is"] the beat's play, pause or stop; the cue buttons, and with them the cue sliders, master cue and the cue / preview toggle (topic B's assumption B-7); the gain of the audio input. The audio input that was picked is not in this list: the computer holds it, for every show (part (d); page item 272; topic K's rule). [page 2, F: item 272 accepted as written] (f) Show recordings, films, snapshots and recorded clips are files of their own [page 2, F: BF257 "We keep the safe snapshot somewhere."] in the app's folders, not inside the show and not carried by the show file; Collect Media copies recorded clips along with the other media. Each show recording keeps with itself its own show file, with the clips exactly as they were (L80; topic E's rule); that file stays with its recording and is not listed among the shows (F-18). (g) At launch the app opens the very last show (L94): R189 (h). Snapshot is a save of the show exactly where it is, with all the settings and the output, made by one shortcut press and kept apart from the show file, to look at later: page item 236 rules it in full. [page 2, F: BF243 "a save of the show exactly where it is"; BF257 "I explained what a snapshot exactly is"]
CHANGED: (a) layout: the page said the show holds it and asked "say so if the layout should belong to the computer instead"; his words: "the show will hold the layout and so will the comp" -- "comp" read as the computer (INFERRED; he writes "comp resource" for the computer in L6; the other reading is in F-10). Added from L35: one layout per screen (F-20); Harmony's own: what the three View entries do with the show and the computer (F-20), and that a layout is fitted to a smaller screen (technical). (a) preset: the page had "the name of the preset an effect has loaded (question 178)", a clause that hung on question 178; his answer to 178 (L52) fits no letter and does not say whether an effect remembers its preset, so the clause is no longer stated here and follows topic C's ruling (its assumption C-5). Added by L48 "they should travel with the show file" and L50 "they are kept with the app and the show file": the presets themselves are in the show file too. (c) and the page's R189 (h): "the app opens with it", in a new empty show, is replaced by L94 "it opens to the very last show"; "the show you saved last" stands. (d) "each output screen's settings" on the computer stands; L96 adds that the SHOW remembers which outputs were on. (e) stands as shown; L128 plans an audio-input picker, whose memory is topic K's. (f) snapshot: he asks back "Do we need snapshot?" -- answered, not yet ruled by him. Added from his words elsewhere (L9): the layer's switch against actions (L7, L73), the action loop toggles and the Global glide (L64, L70), the envelopes (L104), each signal user's own settings (L106), a recording's own show file (L80), mapping files (L116); "the composition's Transform" now reads "the global Transform" (L55). The rest of the reading stands.
TODAY: The show file has "keys" and "layout" blocks written empty (area-show-decks T7); not saved: clip pause, mapping, layout, user signals, macros (T8, U-7, C5; masterSignal IS saved). A layout is the deck-divider height and three divider fractions, applied only by Load / Reset Layout, never at launch or with a show (T24). The app always launches into a new "Untitled" show (T3). Presets are kept with the app only (R-20). Output screens live in settings.json (T8). Nothing of the audio setup is saved (apply-K, citing its sheet's TODAY 24; not re-read). To build: all of the above, plus a "last show" entry and a "last layout" entry in settings.json.
@@END

@@ITEM R189
TITLE: Opening, saving and leaving a show
STATUS: REPLACED part (h) only; (a) to (g) and (i) to (k) stand -- AMENDED after page 2 (R189 1 by F)
HIS: L94
RULE: (a) The quit window comes at every quit ("Quit!", with Quit, Cancel and Save & Quit; Return saves and quits, Esc cancels). (b) A deck taken from the list of shows becomes the deck on screen, and the pads then fire that deck (question 197 A). (c) Collect Media is a command of its own; Save never copies media. (d) New, and every way of opening another show, ask the same question as quit: do not save, cancel, save first (while a recording runs that one window also says the recording will be stopped and kept: topic E's rule). (e) The show's name stands in the window's title bar; a Save that worked shows nothing; a Save that failed shows the "Save failed" box. (f) "Open..." opens a show file from any folder; the list shows the shows in the app's own folder and has an entry "Add show..." that copies a show file from anywhere into it, so that its decks can be taken. (g) Collect Media makes a copy (a folder with the media and a copy of the show that points at them; the copy of the show carries its presets, L48) and leaves the open show as it was. (h) REPLACED by L94: "When you open the application, it opens to the very last show." At launch the app opens the very last show by itself, from its file, asking nothing about the opening itself (whether the small window of page item 223, for two versions of one preset, can come at that moment is topic C's rule): [page 2, F: BF249 "b"] with its layout and its mapping, the layers empty (question 196), the beat stopped (topic A). "The very last show" is the show file that was opened or saved last on this computer; the computer notes it at every open and at every Save, so it is known after a crash or a power cut as well (F-11). If there is none yet, or the file is gone or cannot be read, the app opens a new, empty show and says nothing (F-11). What the outputs do at that moment is topic G's rule (its assumption G-3, the same as F-12). After a crash the safety copy is offered next (question 198). A start of the app made by a test or a tool opens a new, empty show and leaves the note of the last show alone (F-23). (i) The show that is open cannot be deleted from the list. (j) Two decks may have the same name. (k) Save & Quit on a show that has no file yet opens the Save As window; if it is cancelled the app stays open; if the save fails the "Save failed" box shows and the app stays open.
CHANGED: (h) the page said "The app opens with a new, empty show, as today"; his L94 replaces it. Harmony's own in (h): how the last show is known and what happens when its file is missing (F-11); the guard for test starts (F-23). Nothing else.
TODAY: Quit quits at once with no window (area-show-decks T11); New and a list click ask OK / Cancel with no Save choice, Cmd+O asks nothing (T10); the title is always "Audio-DNA" (T4); the list reads one folder (T12); Collect Media re-points the live show (T25); launch builds "Untitled" (T3, MC:516). All of (a)-(k) is still to build except the "Save failed" box (T5). The open at launch must be staged as any other open is (Pitfall 58). The test-mode app still lists his real folders (U-T5): the guard of F-23 comes with this build.
@@END

@@ITEM R190
TITLE: Columns and deck tabs
STATUS: STANDS (the "selected clip" of (a) widened to the selected cell by L4: F-22) -- AMENDED after page 2 (R190 1 by F, R190 2 by F)
HIS: L4
RULE: (a) The Column menu does what its entries say: "Insert Before" and "Insert After" put a new column before or after the column of the selected cell (a selected clip, or an empty cell selected for pasting: G2); with cells selected in several columns, before the leftmost or after the rightmost of them. "Remove Column" removes the selected cell's column and the clips to the right move over; it works only when the selection lies in one column. With nothing selected only "New Column" works, at the end (F-22). Inserting and removing a column are Undo steps (the reading on Undo, topic J). (b) A clip that is playing keeps playing when columns shift (its layer follows the clip); a clip whose own column is removed is deleted with it: if it is playing it leaves its layer at that moment and no longer plays (G2, part 5). The same holds for the clips of a deck that is removed. [page 2, F: BF256 "removes it from the layer strip, and it does not play"] (c) Keys and pads stay on their column numbers: after an insert in the middle a pad fires the clip that is now in that column (question 197 A). (d) When the deck on screen is removed, the next one is shown (the one before it, if it was the last). Tested against his whole message: nothing in it says otherwise; copy, cut and paste of clips (L4, L5) follow the first principle (pads stay on cells); of the second only this holds: an edit that shifts a clip leaves it playing, and an edit that takes a playing clip out of its cell takes it off its layer (G2, part 5) [page 2, F: BF256 "removes it from the layer strip, and it does not play"].
CHANGED: nothing he named. Widened by L4, which lets an empty cell be selected: the page's "the column of the clip you have selected" and "with no clip selected" now read "the selected cell" and "with nothing selected" (Harmony's, F-22).
TODAY: Insert Before / Insert After / New Column all append at the end; Remove Column removes the last one (area-show-decks T18, MC:7028-7058). A layer finds its clip by (deck, column), so a shifted column would change what the layer shows (U-10). Removing the shown deck shows the next, else the previous (M-4): already so.
@@END

@@ITEM R225
TITLE: Menus and files of shows and decks
STATUS: STANDS (old deck files: asked inside F-13) -- AMENDED after page 2 (R225 1 by F)
HIS: L69, L94
RULE: (a) "Save Deck", "Save Deck As..." and "Load Deck..." leave the Deck menu and the tab's right-click menu; the "+" tab only makes a new empty deck; the Decks fold of the list goes; a deck is taken from the list of shows. "Rename Deck", "Duplicate Deck" and "Remove Deck" stay. Deck files saved earlier are no longer opened by the app; they stay on the disk: the removal of the old show files leaves the saved decks alone (page item 235). [page 2, F: item 235 accepted as written] (b) The list of shows is in alphabetical order; a show is renamed in Finder only; deck tabs keep the order in which the decks were made and cannot be dragged. (c) A show file dropped on the window or double-clicked in Finder does nothing; a show is opened with "Open..." or from the list, and by the app itself at launch (L94). (d) A clip whose file is gone shows a red border and a "!" in its cell and does not play; "Relocate Missing Files..." finds the files again by name and changes the open show only, to be saved afterwards. (e) Shows are kept in the app's folder under Library; Save As starts there.
CHANGED: nothing by a word that names it. (a): the page's sentence "Deck files you saved earlier stay on the disk and can no longer be opened" stands as shown; his L69 speaks of "old show files" only, and whether the dead deck files go with them is an ALT of the one question on that deletion (F-13). (c): the launch added (L94).
TODAY: Deck menu and tab menu still hold Save Deck / Save Deck As / Load Deck (area-show-decks T13, T15, T16); the list has a Decks fold (T12); list sorted alphabetically (T12); a dropped .json does nothing (M-5); folders per T29.
@@END

@@ITEM D23
TITLE: New show-file version; the safe write stays
STATUS: OPEN (technical: F-14; his old files: F-13) -- AMENDED after page 2 (D23 1 by F)
HIS: L69
RULE: The show file gets a new version number when actions, the saved pause, the mapping, the layouts, signals, macros, presets and the outputs' on / off arrive in it. Every Save is read back and checked before it replaces the file on disk; a file of an older version is copied to the backups folder before it is written over. No work is done to carry shows of the old shapes into the new one: "let's delete all the old show files and start from scratch." (L69). Removing his old files is a one-time act outside the app's rules: the files are every show saved up to the build and the app's safety copies of them; they are listed to him first and go to the Trash only after his yes; saved decks, recordings, presets and settings stay (page item 235). [page 2, F: item 235 accepted as written]
CHANGED: The page said the protection "stays" for his old files; L69 removes those files, so what stays is the safe write itself, as a feature of the app for every later version change (Harmony's: F-14). The steps of the removal (listed first, the Trash, at build start) are no longer written here as a rule: they are Harmony's and stand in F-13.
TODAY: Version 2, the verified writer and the backups copy are built and merged (area-show-decks T5, T32, Pitfall 68, src/core/ShowFile.h). The converter for old-shape shows is merged (O-11). U-T1: the "routines" / "routineBank" keys go. His files: shows and decks in ~/Library/AudioDNA, settings / bindings / layouts in ~/Library/Audio-DNA, takes / snapshots / recordings in ~/Documents/Audio-DNA (T29); one real old-shape show exists (O-11, T33); none of them was opened, listed or touched for this paper or this ruling.
@@END

@@ITEM D24
TITLE: Bad characters in a show's file name
STATUS: OPEN (his words were about deck files: F-15)
HIS: none
RULE: A show's name is the name of its file (R225 b: a show is renamed in Finder). A character that a file name cannot hold (such as "/" or ":") typed into a show's name becomes "-": a show typed as Club/Night is saved, listed and shown in the title bar as Club-Night. The same holds for the folder Collect Media makes from the show's name.
CHANGED: nothing; his message of 2026-10-07 does not touch it. His only words: "switch to - is fine for all bad chars" (binding-decisions.md:679), said for deck files, which now leave. Made exact by Harmony: the show reads the changed name everywhere, because its name is its file's name.
TODAY: Nothing sanitises a name before it becomes a file name (area-show-decks T15, O-4, R-26). Save As gives the show the file's base name as its name (T6).
@@END

@@ITEM P26
TITLE: The row of deck tabs with twenty decks
STATUS: DROPPED
HIS: L8
RULE: Laid out by Harmony where it fits in the correct area; its look is settled in the UI redesign (L8). The one function that holds whatever the look: with twenty decks every deck can be reached and its name read.
CHANGED: The three variants (tabs that scroll | a drop-down list beside the tabs | two rows) differ only in look and place; none is put to him.
TODAY: Tabs shrink to 60 px and the row never scrolls; past the row's width the "+" is clamped (area-show-decks T13, M-1).
@@END

@@ITEM G2
TITLE: Copy and paste for any clip
STATUS: ANSWERED -- how an empty cell is selected is asked (page item 233), and what an edit does to a playing cell (page item 234) -- AMENDED after page 2 (G2 1 by F, G2 2 by F, G2 3 by F, G2 4 by F, G2 5 by F, G2 6 by F)
HIS: L4, L11, L1
RULE: His words (L4): "We need copy and paste for any clip. If it is selected, it can be copied and then if they empty sell or sell with something else in it is selected then it can be pasted." Built as: (1) COPY: with one or more clips selected, Copy (Cmd+C, Edit > Copy, the cell's right-click menu) puts them on the app's clipboard. Any clip can be copied: a video, a picture, a sequence, a source, a camera, a cell that holds effects only. (2) WHAT A COPY CARRIES (F-2): the whole clip: its media or source with the source's settings, its name, its play settings (everything the Clip tab sets: BPM mode or timeline mode, beats, speed, direction, loop mode, in and out points, fit), its saved pause, its effects with every slider and every signal plugged into them, the envelopes that are the clip's own, of both types (the copy gets its own copy of each: page item 241, topic I's rule) [page 2, F: BF261 "personalized for that clip"], its clip macros, and its actions with their own on / off buttons (questions 165 A and 208 A, his accepted defaults). It does NOT carry: its place in the deck; the key or pad that sits on its cell (pads stay on cells, question 197 A); the MIDI knobs mapped to its sliders, which belong to the cell and move that slider of whatever clip sits there (page item 244, way b; topic J's rule); [page 2, F: BF262 "b"] the keys put on its actions (a copy of an action has no key: reading R199 (a), which he named and left unchanged); whether it is playing. (3) PASTE: with a cell selected, empty or holding another clip, Paste (Cmd+V, Edit > Paste, the right-click menu) puts a new, independent copy there; later changes to the copy never change the original. A clip that was in the cell is replaced (F-8); Undo brings it back. Paste can be repeated: the clipboard keeps its content. (4) THE CELL TO PASTE INTO is selected without firing it: a filled cell by a click on its name (which also shows it in the preview monitor: topic B); an empty cell by a click on the strip at its bottom where a clip's name would be, which selects it without triggering it, while a click anywhere else on an empty cell triggers it (the layer goes empty, L11) and also selects it, so that Paste then goes into it. In the same way a click on a filled cell anywhere but its name triggers the clip and also selects it, without changing the preview monitor (page item 233). [page 2, F: BF255 "you can select the cell which triggers it and selects it"] A right-click on any cell, also an empty one, selects it too, without firing or previewing it, and opens a small menu (on a filled cell Copy, Cut, Paste and Delete; on an empty cell Paste). (5) PLAYING: a clip that is playing and is pasted over, cut or deleted leaves its layer at that moment and no longer plays: the layer goes empty at once. The pasted clip sits in the cell and plays when it is triggered. An edit to a cell whose clip is not playing changes nothing on any layer (page item 234). [page 2, F: BF256 "removes it from the layer strip, and it does not play"] (6) CUT (Cmd+X, Edit > Cut) = copy, and the cell is emptied; the Delete key empties the selected cells without touching the clipboard (question 208 A, default by L1). (7) HOW FAR A COPY REACHES (F-6): the clipboard survives a deck switch, and a clip is pasted into any cell of any layer of any deck of the open show; it also survives the opening of another show, and pasted there a slider whose signal that show does not have keeps its value with nothing plugged in (the same rule as a preset that names a signal the show lacks: topic C's @@ITEM R180 (a), where nothing is asked; the envelopes that are the clip's own come with it, part 2) [page 2, F: BF249 "b"]. The clipboard is empty at every start of the app. (8) SEVERAL CLIPS (F-5): copied together they are pasted in the same arrangement, the top-left one landing in the selected cell; a clip that would land outside the grid is left out. (9) UNDO: each paste, cut and delete is one Undo step, and Undo and Redo never change what is playing: the Undo of a cut, a delete or a paste-over brings the clip back into its cell without putting it back on its layer, and an Undo or Redo that would take away a clip that is playing leaves it playing until something else is triggered on its layer (his rule: [page 2, F: BF256 "removes it from the layer strip, and it does not play"] "Let's not allow control Z to change anything that is live in the layer strip.", binding-decisions.md:688). (10) One thing is on the clipboard at a time, clips or an action; Paste does what fits: clips go into the selected cell, an action goes onto the marked owner (topic D's rule for actions).
CHANGED: New (a general point; no page text). "sell" read as "cell" twice, and "if they empty" as "if the empty": INFERRED. It makes question 208's default A (topic J: Edit menu with Cut, Copy, Paste, Delete; clips get real copy and paste) his own word for clips, and replaces R145 (d) of topic D ("there is no copy or paste of clips ... and none is added by this"). Parts (2), (4), (5), (7) and (8) go beyond his words: F-1 and F-3 are asked; F-2, F-5, F-6 and F-8 are Harmony's and not asked. BY THE RULING OVER ALL TOPICS (2026-10-08): part (4) gains the click on an empty cell's name strip as the main way to select it, because L4 says the empty cell "is selected" and then pasted into; the right-click stays as a second way; part (7) ties the missing-signal rule to page item 223.
TODAY: No copy or paste of clips exists; Cmd+X is "Clear" on the selected clip cells (area-show-decks U-12, MC:4130-4135); no Edit menu, Undo / Redo sit in "Composition" (T1). Selecting = a click on the name bar (T20). SetClipCmd / SwapClipsCmd exist as Undo commands (MC:5301). A Duplicate re-mints clip ids (Pitfall 36): a paste must do the same. What a playing layer does when its cell is replaced: not checked (a layer finds its clip by deck and column, U-10, so it would most likely show the new clip at once: to change for F-3).
@@END

@@ITEM G3
TITLE: Option-drag copies a clip into the cell dragged to
STATUS: ANSWERED -- AMENDED after page 2 (G3 1 by F, G3 2 by F)
HIS: L5
RULE: His words (L5): "Option drag on a clip copy and pastes into the cell it is dragged to." Built as: dragging a clip by its name with the Option key held puts a copy into the cell it is dropped on and leaves the original where it was. The copy carries exactly what a pasted copy carries (G2, part 2) and follows the same rules: the target may be empty or filled; a clip in the target is replaced, not swapped (F-8); a clip in the target that is playing leaves its layer at that moment and no longer plays, and the copy does not play until it is triggered (G2, part 5) [page 2, F: BF256 "removes it from the layer strip, and it does not play"]; one Undo step removes the copy and brings back what was in the target (G2, part 9). The copy can land on any layer of the deck on screen (F-9). An Option-drag leaves the clipboard as it is, and it is a copy when the Option key is held at the moment of the drop (F-21). A drag without Option moves the clip and deletes none, so his words on a clip that is pasted over or deleted do not reach it. A playing clip that is moved to another cell of its own layer keeps playing: its layer follows it, as when columns shift (R190 (b)). A playing clip that is moved to another layer leaves the layer it was playing on at that moment and waits in its new cell until it is triggered; the same holds for a clip that changes places with the dragged one (F3-15). [page 2, F: BF256 "removes it from the layer strip, and it does not play"]
CHANGED: New (a general point; no page text). Harmony's own, not his words: replace rather than swap (F-8), the deck on screen only (F-9), the clipboard left alone and the Option key read at the drop (F-21).
TODAY: A drag from the name bar moves the clip; onto a filled cell the two swap ("Move Clip" / "Swap Clips", one Undo unit: MC:1205-1235; ClipCell.cpp:297-305). No Option handling in ClipCell (grep isAltDown: none). The drop works inside the shown deck only (composition_.activeDeckIndex, MC:1232).
@@END

## ASSUMPTIONS STILL OPEN (new after page 2: ids with a 3; old ones never shown to him keep their ids)
@@ASSUME F3-1
ABOUT: 233, G2
TEXT: I assume a click on a clip, anywhere but its name, triggers it and also selects it: the Clip tab shows it, and Copy, Cut, Paste and Delete then work on that clip.
WHY: BF255 "you can select the cell which triggers it and selects it" stands under an item about empty cells; it does not name a cell that holds a clip. With BF256 ("deleting a clip, removes it from the layer strip, and it does not play") the selected clip is then the one that is playing: a Delete, a Cut or a Paste takes it off its layer.
ALT: b) Only an empty cell is selected by its trigger click; a clip is selected by a click on its name only.
IF-WRONG: STAGE the Clip tab would jump, or not jump, to a clip he triggers; and a stray press of the Delete key would take the clip he has just triggered off the output.
ASK: LINE his sentence is general ("the cell") and he would most likely wave it through; but the Delete key's reach is new and seen on stage, so he gets the line.
@@END

@@ASSUME F3-2
ABOUT: 233
TEXT: I assume an empty cell you click to trigger is selected at the click, also while its layer still waits for the 1 to go empty; and a column trigger or an action that triggers cells selects nothing.
WHY: BF255 says the click "triggers it and selects it", not when the selection lands while the trigger waits for the 1; it speaks of a click on one cell, not of a column trigger or of an action. Whether a key or a pad on a cell selects it is topic J's (its J3-10), not ruled here.
ALT: b) The cell becomes selected only when the layer goes empty.
IF-WRONG: SMALL
ASK: NO technical; a selection that waits would only delay the paste, and a column trigger has no one cell to select.
@@END

@@ASSUME F3-3
ABOUT: 234, G2, G3
TEXT: I assume the layer goes empty as soon as you paste over, cut or delete the clip it is playing, also when that clip is in BPM mode. The pasted clip waits in its cell until you trigger it.
WHY: BF256 "removes it from the layer strip, and it does not play" names no moment. "it" reads as the clip pasted over; read as the pasted clip, his words settle the second sentence as well. He saw way b of the item ("The layer switches at once to the pasted clip") and wrote his own words in its place.
ALT: b) A clip in BPM mode leaves its layer on the next 1, as when you trigger an empty cell. c) The pasted clip starts playing in its place, as if you had triggered it.
IF-WRONG: STAGE a layer would drop out off the beat, or stay dark where he expected the pasted clip; small to change once built.
ASK: LINE his words settle that the clip leaves the layer strip; the moment and the waiting of the pasted clip are edges he would most likely wave through.
@@END

@@ASSUME F3-4
ABOUT: 234, R190
TEXT: I assume Remove Column and Remove Deck count as deleting their clips: a clip of theirs that is playing leaves its layer at once and no longer plays.
WHY: BF256 speaks of "deleting a clip"; it names neither a column nor a deck that is removed with clips in it. Two earlier defaults, never his words, said the opposite: the page-1 reading R190 (b) ("a clip whose own column is removed plays on until it is replaced") and the default Q1 of 2026-10-02 ("a deleted deck's playing clip keeps playing until replaced", binding-decisions.md:611).
ALT: b) Such a clip plays on until you trigger something else on its layer.
IF-WRONG: STAGE a layer would go dark when a column or a deck is removed in a show.
ASK: LINE it follows his words for a deleted clip, against two defaults he had let stand; one line for both, which he can strike.
@@END

@@ASSUME F3-6
ABOUT: 236, 196, R188
TEXT: I assume a snapshot keeps a picture of the output and also what was playing. Opening one later takes the place of the open show (after asking to save it) and brings the clips back on their layers, on their frames, with the tempo paused.
WHY: BF243 "a save of the show exactly where it is" and "save that to look out later" do not say whether what plays is kept, nor what opening a snapshot does to the show that is open; "the output" in "with all the settings and the output and everything" can be its picture. Against the default he accepted for a show (196 A: every layer is empty when a show is opened), a snapshot would come back with its clips.
ALT: b) It opens like any show, with the layers empty; you trigger the clips yourself. c) It never replaces the open show: you only look at its picture and take clips or decks from it.
IF-WRONG: REBUILD keeping and restoring what plays is the large part of this build; and he would see at once whether the moment comes back.
ASK: YES no words of his say what "look at later" does, and the three ways are different builds.
@@END

@@ASSUME F3-7
ABOUT: 236, R188
TEXT: I assume every snapshot also keeps a still picture of the output, to find the moment in the list; there is no separate command that saves only a picture.
WHY: BF243 "with all the settings and the output and everything": "the output" can be its picture or which outputs were on; BF257 does not speak of a picture-only command. The picture is told to him inside the one question he gets on Snapshot (F3-6).
ALT: b) No picture: a snapshot is listed by show, date and time only. c) A picture-only command stays as well.
IF-WRONG: SMALL one picture file and one menu entry.
ASK: NO the picture is named in F3-6's text, where he can strike it; the rest is one menu entry.
@@END

@@ASSUME F3-9
ABOUT: 236
TEXT: I assume snapshots are kept in a folder of their own, each named by show, date and time, until you delete them, with no limit on their number; taking one never touches your show file.
WHY: BF257 "We keep the safe snapshot somewhere" names no place, no name and no limit.
ALT: b) Snapshots are kept inside the show file and travel with it. c) Only the newest twenty are kept.
IF-WRONG: SMALL where files are stored.
ASK: NO "somewhere" leaves the place to Harmony; his show file is never written by anything but his Save (old item 198).
@@END

@@ASSUME F3-10
ABOUT: 236
TEXT: I assume taking a snapshot never makes the picture stutter and shows nothing when it worked; if it fails, the "Save failed" box shows.
WHY: BF243 says the user pushes a shortcut in an inspiring moment of a running show; nothing says what he sees after the press.
ALT: b) A short sign confirms each snapshot.
IF-WRONG: SMALL
ASK: NO a build requirement, and his rule "the only fail message will be a failed save" (binding-decisions.md:720).
@@END

@@ASSUME F3-11
ABOUT: 236, R188
TEXT: I assume a snapshot keeps each slider as you set it, with its signal still plugged in, not the value the music gave it at that instant; and it leaves out what the computer keeps for every show, the audio input and the cue buttons.
WHY: BF243 "exactly where it is with all the settings and the output and everything" is wide: a slider moved by a signal has a set value and a momentary one, and "the output" could reach each output screen's own settings, which the computer holds (old item R188 (d)). Topic K reads the audio input out of a snapshot (item 272).
ALT: b) Every slider is kept at the exact value it showed at that instant, with its signal unplugged. c) A snapshot also keeps each output screen's own settings.
IF-WRONG: SMALL a snapshot opened without music looks calmer than its picture; the picture kept with it shows the moment itself.
ASK: NO "all the settings" are the sliders as set, and he knows a look moves with the music; what the computer holds is the same split as for a show.
@@END

@@ASSUME F3-12
ABOUT: 235, D23
TEXT: I assume the list of your old show files is shown to you before the build changes anything about show files, and nothing moves to the Trash before your yes.
WHY: Item 235, accepted, says "I list them first; after your yes"; it does not say at which moment of the build.
ALT: none
IF-WRONG: SMALL
ASK: NO the moment is Harmony's to pick; his yes is asked in any case.
@@END

@@ASSUME F3-13
ABOUT: 236
TEXT: I assume Snapshot comes with no key of its own: you put it on a key or a pad in the keyboard and MIDI mapping, like any button. While Studio is on screen it does nothing.
WHY: BF243 "the user can push a shortcut button" names no key; his "R227 all good" let stand that the spacebar is the only key that comes mapped. Nothing of his says what the press does while Studio is on screen, where the keys and pads of the mapping trigger nothing in the live show (topic E's old item R185).
ALT: b) It comes with a key of its own from the first start. c) In Studio it saves the moment of the recording that is shown.
IF-WRONG: SMALL one entry of the mapping.
ASK: NO which key a button sits on is his to set in the mapping (topic J's rule); small to change. Way c would be a new function of Studio, topic E's, and is not built.
@@END

@@ASSUME F3-14
ABOUT: 236, R189
TEXT: I assume a snapshot you open comes up as a show without a file of its own: Save asks for a name and your last show stays the same. It points at the show's media files; Collect Media leaves snapshots out.
WHY: BF243 and BF257 say nothing of what an opened snapshot is to Save, to the very last show (old item R189 (h)) or to the media. A snapshot that is looked at long after shows the red "!" cells of old item R225 (d) where a media file has moved since.
ALT: b) Collect Media takes the snapshots along. c) A snapshot copies its media.
IF-WRONG: SMALL
ASK: NO technical; a show points at its media in the same way (Save never copies media), and a safety copy opens in the same way (old item 198).
@@END

@@ASSUME F3-15
ABOUT: 234, G3
TEXT: I assume a clip you drag to another cell of its own layer keeps playing; dragged to another layer it leaves the layer it was playing on and waits in its new cell, and so does a clip that changes places with it.
WHY: BF256 names pasting over and deleting; a plain drag deletes no clip. Old item R190 (b) already lets a layer follow its clip when columns shift; a layer plays only the clips of its own row (read in CLAUDE.md rule 15, not run).
ALT: b) A dragged clip keeps playing on its old layer until you trigger something else there.
IF-WRONG: SMALL a rare drag during a show.
ASK: NO an edge of his rule for an edited clip; small to change.
@@END

@@ASSUME F-2
ABOUT: G2, G3
TEXT: I assume a copied clip brings everything it has: effects with their sliders and signals, its own envelopes, actions, play settings, saved pause. It does not bring the key, pad or knobs of its cell, nor the keys of its actions.
WHY: L4 and L5 say "copy", not what travels; his accepted defaults give the actions (165 A, 208 A) and keep pads on cells (197 A). New after page 2: BF262 "b" keeps a knob on a clip's slider with the cell, and BF261 makes an envelope "personalized for that clip", so it travels with the copy (topic I's item 241).
ALT: b) The copy brings the media and its play settings only, without effects and actions.
IF-WRONG: SMALL one list of fields in the copy.
ASK: NO the plain meaning of copy, carried by defaults he accepted (L1) and by his answers on knobs and envelopes.
@@END

@@ASSUME F-5
ABOUT: G2
TEXT: I assume several selected clips are copied together and pasted in the same arrangement, the top-left one landing in the cell you selected. A clip that would land outside the grid is left out.
WHY: L4 speaks of one clip ("it is selected"); the default of question 208 he accepted speaks of "the selected clips"; how several land is not said.
ALT: b) Copy and paste work on one clip at a time only. c) The grid grows by the columns needed, so that no clip is left out.
IF-WRONG: SMALL the block paste can be changed or left out later.
ASK: NO the usual way of a grid; nothing of it is seen until he copies several clips at once.
@@END

@@ASSUME F-6
ABOUT: G2
TEXT: I assume a copied clip can be pasted into any deck of the open show, and into another show you open afterwards, until you copy something else or quit. A slider whose signal that show lacks keeps its value, unplugged.
WHY: L4 does not say how far a copy reaches, how long it lasts, or what a pasted clip does with a signal the other show does not have.
ALT: b) The copy is lost when another show is opened.
IF-WRONG: SMALL
ASK: NO technical; decks are boxes of clips by his own rule, so any deck is implied.
@@END

@@ASSUME F-9
ABOUT: G3
TEXT: I assume Option-drag copies inside the deck on screen; to copy a clip into another deck you use copy and paste.
WHY: L5 names the cell dragged to and no deck; a drag cannot reach a deck that is not shown.
ALT: b) Holding the drag over a deck tab shows that deck, so the clip can be dropped there.
IF-WRONG: SMALL the tab hover can be added later.
ASK: NO a matter of the mouse gesture, for the UI redesign (L8); copy and paste already reach every deck.
@@END

@@ASSUME F-10
ABOUT: R188
TEXT: I assume "the comp" means the computer: the layout is saved in the show, and the computer also remembers the layout used last, for a new show and for a show that holds none.
WHY: L94 "the show will hold the layout and so will the comp"; he writes "comp" for computer in L6 and for composition in earlier messages.
ALT: b) "The comp" means the composition, which is the show itself: the layout is in the show only, and a new show starts in the standard layout.
IF-WRONG: SMALL where one small block is stored.
ASK: LINE two readings of one word; the difference is small.
@@END

@@ASSUME F-11
ABOUT: R188, R189
TEXT: I assume "the very last show" is the show file you opened or saved last, also after a crash. If that file is gone, or there is none yet, the app opens a new, empty show and says nothing.
WHY: L94 says "it opens to the very last show", not how it is known after a crash or an unsaved new show, nor what happens when the file is missing.
ALT: b) After New and a quit without saving, the app opens a new, empty show and not the earlier saved one. c) If the file is missing a box says so.
IF-WRONG: SMALL the readings differ only after a new show that was not saved, or when a file is gone.
ASK: NO edges of a clear instruction; the silent fallback follows his rule "the only fail message will be a failed save" (binding-decisions.md:720).
@@END

@@ASSUME F-14
ABOUT: D23
TEXT: I assume the show file gets a new version number in this build, every save stays checked before it replaces your file, and no work is spent on opening shows made before this build.
WHY: He did not read this point (L130); L69 removes the old files but does not speak of the file's version or the safe write.
ALT: b) Shows of the old shapes must still open after the build.
IF-WRONG: SMALL
ASK: NO technical; his L69 and his earlier "no shows are saved" cover the intent.
@@END

@@ASSUME F-15
ABOUT: D24
TEXT: I assume a character a file name cannot hold, such as "/", becomes "-" in a show's name and in the folder Collect Media makes: Club/Night is saved as Club-Night. It is your rule for deck files, carried over.
WHY: He did not read this point (L130); his "switch to - is fine for all bad chars" (binding-decisions.md:679) was said for deck files, which now leave.
ALT: b) The Save As window refuses such a name.
IF-WRONG: SMALL
ASK: NO his own rule for deck files, carried to the show's file.
@@END

@@ASSUME F-17
ABOUT: 198
TEXT: I assume that after a crash the app opens your last show as you saved it, then offers the safety copy once, if it differs. A copy is made about every three minutes (my number, inside "every few minutes").
WHY: Default 198 says "every few minutes" and "offers it once"; L94 adds the last show at launch. The order, the number and what taking or leaving does are not said.
ALT: b) After a crash the app opens the safety copy directly and says so.
IF-WRONG: SMALL
ASK: NO the offer is the default he accepted (L1); the order and the number are technical.
@@END

@@ASSUME F-18
ABOUT: R188
TEXT: I assume everything your message adds is saved in the show too: each action's loop switch, each layer's switch against actions, the Global glide time, every envelope, each signal user's settings. A recording's show file stays with its recording.
WHY: L7, L73, L64, L70, L104, L106, L80 add things to what a show holds; none says where each is kept. His One Save rule points to the show.
ALT: b) The Global glide time, and the fade control of L70 if it is a control of its own, belong to the computer, for every show.
IF-WRONG: SMALL where a few values are stored.
ASK: NO follows his One Save rule and "every show remembers it's sync"; the cue controls are topic B's (B-7), the glide's owner is topic D.
@@END

@@ASSUME F-19
ABOUT: R188, R189, R225
TEXT: I assume the thing you save and open is called a "show" everywhere on screen (the menu "Show", "New Show", "Open...", "Save"), and "global" names the level above the layers.
WHY: L55 moves the level's name from composition to global; he writes "show" and "show file" throughout (L48, L50, L69, L94) but did not rule the menu's name.
ALT: b) The menu and the file keep the name "Composition", as in Resolume.
IF-WRONG: SMALL words on screen.
ASK: NO a name on screen: it goes into the list of names he asked for (L114), where he reads it and can change it.
@@END

@@ASSUME F-20
ABOUT: R188
TEXT: I assume the show holds one layout for each screen of the app, live and Studio now, more when you add configurations. Save Layout, Load Layout and Reset Layout stay in the View menu, to carry an arrangement between shows.
WHY: L94: the show and the computer hold "the layout". L35 speaks of various configurations. One layout or one per screen is not said. Only the name changes here (BF245, BF263).
ALT: b) The show holds the live screen's layout only; Studio's arrangement belongs to the computer. c) Save Layout and Load Layout go, because the show saves the layout; Reset Layout stays.
IF-WRONG: SMALL where a small block is stored, or two menu entries.
ASK: NO internal; unchanged but for the screen's new name.
@@END

@@ASSUME F-21
ABOUT: G3
TEXT: I assume an Option-drag leaves whatever you copied earlier ready to paste, and that it is a copy when Option is held at the moment you let go of the mouse.
WHY: L5 says "copy and pastes"; read to the letter, the dragged clip could also become what Paste pastes next. When the Option key is read is not said.
ALT: b) The dragged clip also becomes what Paste pastes next.
IF-WRONG: SMALL
ASK: NO technical; the usual way of an Option-drag on a Mac.
@@END

@@ASSUME F-22
ABOUT: R190
TEXT: I assume Insert Before, Insert After and Remove Column work from the column of the selected cell, also when that cell is empty. With cells selected in several columns, Remove Column does nothing.
WHY: The page spoke of "the clip you have selected"; L4 makes an empty cell selectable, and several cells can be selected at once.
ALT: b) They work only from a selected clip, never from an empty cell.
IF-WRONG: SMALL
ASK: NO technical; it follows a reading he let stand.
@@END

@@ASSUME F-23
ABOUT: R189, 198
TEXT: I assume a start of the app made by a test or a tool opens a new, empty show: it never opens your last show, never changes which show is your last one, and never writes a safety copy.
WHY: L94 makes the app open a show file by itself; nothing says what Harmony's own test starts do, and they must not read or change his files.
ALT: none
IF-WRONG: REBUILD a test run could change which show opens at his next start, or leave copies among his files.
ASK: NO internal: how Harmony's own test runs behave (the twin of topic G's G-9).
@@END

## CLOSED ASSUMPTIONS (one line each)
- F-1 -> page 2, item 233
- F-3 -> page 2, item 234
- F-8 -> SETTLED
- F-12 -> SETTLED
- F-13 -> page 2, item 235
- F-16 -> page 2, item 236

## QUESTIONS BACK
## NAMES
@@NAME show
MEANS: The one file that is saved and opened; it holds the decks, layers, clips, effects, actions, mapping, layout and presets.
SOURCE: his words L48, L50, L69, L94 ("show file", "the show"); as the menu's name it is Harmony's pick (F-19), replacing the on-screen name now, "Composition"
@@END

@@NAME last show
MEANS: The show file that was opened or saved last on this computer; the app opens it by itself at launch.
SOURCE: his words L94 ("it opens to the very last show"); "opened or saved last" is Harmony's reading (F-11)
@@END

@@NAME global
MEANS: The level above the layers (effects, actions and macros that work on the whole picture).
SOURCE: his words L55 ("lets move to global"); replaces "composition" as the level's name
@@END

@@NAME deck
MEANS: A box of clips inside a show; the grid shows one deck at a time and the layers play whatever was fired from any of them.
SOURCE: binding-decisions.md:598-602 ("treat the decks as just a box of clips")
@@END

@@NAME Copy / Cut / Paste
MEANS: The three Edit-menu commands (Cmd+C, Cmd+X, Cmd+V) that work on the selected clips or the marked action.
SOURCE: his words L4 ("copy and paste for any clip"); Cut and the keys from question 208's default; the Edit menu from binding-decisions.md:712
@@END

@@NAME Option-drag
MEANS: Dragging a clip by its name with the Option key held, which puts a copy in the cell it is dropped on.
SOURCE: his words L5
@@END

@@NAME safety copy
MEANS: The copy of the open show that the app writes by itself every few minutes and offers once after a crash.
SOURCE: Harmony's pick (the page's word in question 198, default taken by L1)
@@END

@@NAME Collect Media
MEANS: The command that makes a folder with a copy of the show and copies of all its media, leaving the open show as it was.
SOURCE: the on-screen name now; kept by his "82 default" (binding-decisions.md:890)
@@END

@@NAME keyboard and MIDI mapping
MEANS: Which key, pad or knob does what; every show holds its own and the computer keeps the last one.
SOURCE: binding-decisions.md:1099 ("we call it keyboard and midi 'mapping'"); L116 ("we want mapping files")
@@END

@@NAME Snapshot
MEANS: A save of the whole show exactly as it stands at one moment, with a picture of the output, made by one shortcut press and kept apart from the show file to look at later.
SOURCE: his words (BF243, BF257); replaces the name Snapshot ("A command that saves one still picture of the output as an image file; never part of a show") in /Users/boriskarpman/projects/RealTimeAudio/.harmony/NAMES.md
@@END

@@NAME layer strip
MEANS: The block of each layer that shows what the layer is playing, with the layer's buttons and sliders; a clip that is "removed from the layer strip" is no longer on its layer.
SOURCE: his words (BF256; binding-decisions.md:688, 764); /Users/boriskarpman/projects/RealTimeAudio/.harmony/NAMES.md lists it as Harmony's pick: it is his word
@@END

@@NAME layout
MEANS: The arrangement of the panels of one screen of the app (the live window, Studio); the show holds one for each screen and the computer keeps the ones used last.
SOURCE: his words L94; "Studio" by BF245; "live window" is his name for the screen he performs on (NAMES.md, replacing "live screen"); replaces the entry that says "Review" in /Users/boriskarpman/projects/RealTimeAudio/.harmony/NAMES.md
@@END

## REACHES OTHER TOPICS (from the paper)
- A (triggering): item 233 leans on A's rules for when a triggered empty cell empties its layer (L11, page item 248) and when a triggered clip starts; item 234 uses A's rule for HOW a layer goes empty, but at once. If A rules that a layer can only go empty on the 1 in BPM mode, F3-3 way c becomes the rule.
- A (tempo): item 236 part (4) opens a snapshot with the tempo PAUSED, because by BF271 ("Stop removes all clips from all layers") a stopped tempo cannot hold clips on layers. A owns what pause holds; a snapshot's restore needs "paused with clips on their layers at a given place in the bar" to be a state A allows.
- B (previews): item 233: the name click on a filled cell previews by B's rule (BF247: quick, not on the beat); the triggering click, which now also selects, must NOT feed the preview monitor (F3-1). B should say so in its own rule for what selecting shows.
- D (actions): item 234: a clip taken off its layer by a paste-over, cut or delete ends its actions as any clip leaving its layer does; BF251's "plays again only when that clip is re-triggered" then holds for the pasted clip's own actions. Item 236 (2b) wants "which actions are running and how far" saved and restored: D owns whether an action can be resumed mid-run.
- D: old items R143 (amended here, named under MADE FROM of 235) and D13 (stands). D-7 is closed by item 235.
- G (outputs): item 236: a snapshot holds which outputs were on, as a show does, and opening one follows G's rules for opening a show (page items 237, 264, and BF269 after All Outputs Off). Syphon in the show (page item 263) is applied in R188 (a).
- J (mapping): BF262 ("b" to item 244) applied in G2 part (2) and old item 197: a MIDI knob on a clip's slider belongs to the cell, so a copied clip carries none. The Snapshot shortcut is one more entry of the mapping (which key: J). Page item 271's reason (no stray hit) does not apply to it.
- J (Edit menu, Undo): G2 part (9) as amended keeps his Undo rule; topic J's Undo reading should carry the same sentence for an Undo that would take away a playing clip (F3-5).
- K (audio): page item 272 applied in R188 (e): the computer remembers the picked audio input. A snapshot holds no sound (F3-11).
- E (Studio, recordings): a snapshot is taken of the LIVE show; whether the shortcut does anything while Studio is on screen is not ruled here (see NOT DONE). A show recording and a snapshot do not depend on each other.
- X / names: NAMES.md's entries Snapshot, layer strip, layout, and "Output menu" (it lists Snapshot as a still-picture entry) need the changes under NAMES.

## CONFLICTS (from the paper)
- Snapshot, a separate save. New (BF243): "Perhaps a good snapshot is a save of the show exactly where it is with all the settings and the output and everything so if there is a cool inspiring moment, that happened, the user can push a shortcut button and save that to look out later". Earlier, on a list that named "a snapshot" among nine kinds of saving (binding-decisions.md:874-876): "All of these things should be saved when a show is saved. There's no reason to save them separately:". The newest words win: a snapshot is a save of its own, beside the show's one Save. One line for him: "Snapshot is now the one thing saved apart from the show's Save."
- Snapshot against the answer he was given. Harmony's answer on page 2 kept Snapshot "only as one small command that saves a still picture of the output"; his BF243 and BF257 ("We keep the safe snapshot somewhere. Earlier in this document, I explained what a snapshot exactly is.") replace it. Not a conflict of his own words; listed so that the still-picture reading is not built.
- An edit and the playing clip. New (BF256): "Pasting over a clip or deleting a clip, removes it from the layer strip, and it does not play." Earlier he had only let stand, unanswered, the reading that an edit of the grid leaves a playing clip playing (old R190 (b), shown to him on page 1; no words of his in binding-decisions.md say it). His Undo rule (binding-decisions.md:688: "Let's not allow control Z to change anything that is live in the layer strip. It changes anything else") is kept and is not a conflict: it is about Undo, and BF256 is about the edit itself (F3-5).

## NOT DONE / UNSURE (from the paper)
- The other reading of "it does not play" (BF256): "it" could be the PASTED clip ("the pasted clip does not play by itself"). Under that reading the old clip is still removed from the layer strip by the first half of his sentence, so the layer goes empty either way; the two readings give the same rule, and what is left open is asked as F3-3.
- A plain drag (a move, no Option) onto a cell whose clip is playing, and a move of a clip that is itself playing to another layer: not ruled by his words and not by G3 ("this rule adds nothing to it"). Cheapest way: follow F3-3 once he has answered it (a replaced playing clip leaves its layer; a moved clip keeps playing only inside its own layer).
- Snapshot while Studio is on screen, and whether a snapshot press is marked in a running show recording so the moment can be found there: not in his words. Cheapest way: one line on his next page only if E's paper needs it; until then the shortcut acts on the live show only.
- Whether a state "tempo paused, clips on their layers, each at a saved frame, actions part-way" can be restored exactly is a build question (topic A's pause, topic D's actions, topic H's seek to a frame): not checked in the program text. If it cannot, F3-6 way b is what can be built first.
- How a snapshot is deleted and whether its picture can be opened by itself in Finder: left to the layout work; the files are ordinary files in the app's snapshots folder.
- G2 part (7) ties a pasted clip's missing signal to page item 223; his answer there (BF249 "b": a small window asks) is about two versions of one preset, which topic C owns. Whether a clip pasted into another show that has a signal of the same name but another shape asks in the same way is not ruled here.
- Nothing of the app was run or measured for this paper; every TODAY line is from the old spec, a fact sheet or program text read, not run.

Written 2026-10-09 18:50:21 by the architect for topic F (read-only; HEAD 33989a8; nothing built, run or launched).
STATUS: DONE

## FOR THE PAGE RULING (from the ruling)
- MUST GO TO HIM, one card: F3-6 (YES). What a snapshot keeps beyond a Save (a picture, what was playing) and what opening one later does. It is the large part of the Snapshot build, and ways b and c are other builds. Nothing else in F is YES.
- THEN THREE LINES, by weight: (1) F3-1: a click on a clip triggers and selects it, so Delete, Cut and Paste then work on the clip that is playing, and by BF256 that takes it off its layer; keep that consequence in the line, as written. (2) F3-3: the layer empties at once, also under a BPM-mode clip; the pasted clip waits. (3) F3-4: Remove Column and Remove Deck count as deleting their clips.
- EVERYTHING ELSE IN F IS NO: Snapshot's picture (told inside F3-6), its key (none until he maps it: F3-13), where snapshots are kept (F3-9), sliders as set (F3-11), an opened snapshot as a show without a file (F3-14), the plain drag (F3-15), and F3-2, F3-10, F3-12, F-2, F-20. Dropped, because his words settle them: F3-5 and F3-8.
- ONE RULE FOR THE PAD: whether a key or a pad that triggers a cell also selects it is J3-10 (topic J says yes, ASK NO); this paper no longer rules it. The two papers had read it in opposite ways, and with F3-1 the selected clip is what Delete works on: J3-10 should be a LINE at least, best folded into F3-1's line ("a key or a pad on the cell does the same").
- NEWEST WORDS AGAINST EARLIER ONES: none needs a line of its own. (a) Snapshot is a save apart from the show's Save, against "There's no reason to save them separately" (2026-10-04, binding-decisions.md:874): F3-6 says it. (b) An edit takes a playing clip off its layer (BF256), against two defaults he had let stand (R190 (b); Q1 of 2026-10-02, binding-decisions.md:611): F3-3 and F3-4 say it. (c) A snapshot comes back with its clips, against 196 A: F3-6, with "the layers empty" as its way b.
- DEPENDS ON A: "at once" is the way of the layer's X (old R168, its assumption A-5). If A rules that a layer under a BPM-mode clip only ever goes empty on the 1, F3-3 way b becomes the rule and item 234 (1) follows it. A snapshot's opening needs the state "tempo paused, clips on their layers" (BF171 gives it) and a one-time hold of the clips that are not in BPM mode.
- DEPENDS ON D: a clip that leaves its layer by an edit ends its actions as at an X (D's R161 as amended there); a snapshot that keeps "which actions run and how far" needs an action that can be taken up part-way. D asked whether BF256's deck sentence goes against his 182 b: F reads it as his rule of 2026-10-02 said again; no line.
- DEPENDS ON J: Snapshot is a button of the mapping (J's AMEND 206 2) and comes with no key (R227 (c) stands); J's R198 and G2 (9) as amended must carry the same Undo sentence.
- G, E, K, C: the snapshot's picture is the "snapshots" of R226 (b) and R187 (d), which stand; opening a snapshot is the opening of a show for the outputs (237, 264, 265 as written). K: a snapshot does not hold the audio input. C: a snapshot carries the presets as a show does, so the window of item 223 can come when one is opened; R189 (h) now leaves that window at launch to C. E: Snapshot does nothing in Studio (F3-13's way c would be a new function of Studio).
- I: G2 (2) and R188 (b) now carry the clip's own envelopes of both types (BF261) by pointing at I's item 241; if I's ruling changes who owns an envelope, both follow it.
- NAMES: the list must change Snapshot (a save of the show with its picture, not a still picture), layer strip (his word: BF256), layout (the live window, Studio) and the Output menu's line on Snapshot. On his page the words "safety copy" must not stand for two things: in item 235 they meant the copies in the backups folder, in item 198 the copy written every few minutes.
- STILL OPEN, small: how a snapshot is deleted (laid out with its list). The missing-signal rule of G2 (7) now follows topic C's R180 (a): if C's ruling changes that rule, G2 (7) follows.

Written 2026-10-09 19:30:30 by the ruling architect for topic F (read-only; HEAD 33989a8; nothing built, run or launched).
STATUS: DONE

