# APPLY F -- The show file, decks and saving (s-rta-1007)
## SUMMARY
- The app opens to the very last show (L94); the page's "opens with a new, empty show" is gone. Layers are still empty at launch (question 196 default).
- The show holds the window layout "and so will the comp" (L94): read as show AND computer; one line for him to strike.
- Clips get copy and paste (L4) and Option-drag copy (L5); with the accepted defaults 208 A and 165 A this is Cmd+C / Cmd+X / Cmd+V and the copy brings its actions. Two function gaps are asked.
- "let's delete all the old show files and start from scratch" (L69): done only when the build starts, files named to him first, moved to the Trash, never erased. Which files count is asked.
- Presets now travel with the show file as well as the app (L48, L50); a show remembers its connected outputs (L96). With L94 this means outputs come on at launch: asked, because the standing rule said the app never opens an output by itself.
- His question "Do we need snapshot?" (L94) is answered: not for saving anything.
- Questions 196, 197, 198 keep their defaults; R190 stands; R189 and R225 stand except one part each.
## ITEMS
@@ITEM 196
TITLE: What a re-opened show shows
STATUS: DEFAULT (A)
HIS: L1, L94
RULE: When a show is opened, every layer is empty: a show does not keep what was playing. A clip that was paused when the show was saved keeps its pause and its frame: its cell shows a pause mark, and the clip appears on that frame, paused, when it is fired. The same holds at launch, when the app opens the very last show by itself (L94: "When you open the application, it opens to the very last show."): the show is there, its layers are empty, nothing plays until he fires a clip or a column.
CHANGED: nothing in the default; added from L94: the rule also covers the show the app opens by itself at launch.
TODAY: Layers are empty after every open (area-show-decks T9). No clip pause is saved and no cell shows a pause mark (T8; R-4 on paper only). To build: the saved pause per clip, the pause mark on the cell.
@@END
@@ITEM 197
TITLE: Pads and keys when you switch decks
STATUS: DEFAULT (A)
HIS: L1, L74
RULE: A key or pad that was put on a cell fires that same cell (layer and column) of the deck that is on screen. Switching the grid to another deck changes what the pad fires next and never changes what is already playing. A pad stays on its cell when a clip is moved, copied, cut or pasted: it fires whatever clip sits in that cell now. His "182 b" (L74) says the same for a layer's action: it fires the cells of the deck that is shown.
CHANGED: nothing.
TODAY: This is what a ByPosition binding does now (area-show-decks T21, U-1, C6). A pad on a column beyond the shown deck's width does nothing while a pad on an empty cell clears its layer (M-2): not touched by his words.
@@END
@@ITEM 198
TITLE: A safety copy after a crash
STATUS: DEFAULT (A)
HIS: L1, L94
RULE: The app keeps a safety copy of the open show by itself every few minutes, in a place of its own, apart from the show file. It never writes over his show file: only his own Save does that. After a crash or a power cut, the next launch opens the very last show from its file as he saved it (L94) and offers the safety copy once: take it, or leave it. After a normal quit no copy is offered and the copy is discarded. A show that has never been saved to a file also gets a safety copy.
CHANGED: nothing in the default; added from L94: after a crash the app opens the last show first and then makes the offer (see F-17).
TODAY: No autosave, no crash recovery, no changed-since-save mark (area-show-decks T3, U-9, O-15). To build: a timed writer to a separate file through the verified writer, a clean-quit marker, the one-time offer at launch.
@@END
@@ITEM R188
TITLE: What a show keeps and what the computer keeps
STATUS: CORRECTED (a), (c), (d), (f), and the opening of the app
HIS: L94, L48, L50, L52, L96, L7, L64, L80, L116
RULE: (a) A saved show holds: its decks, clips, layers and effects with every slider; each clip's own pause; its actions, which of them are switched on, each action's loop toggle where it is placed (L64), the ignore lamps including the ignore-actions toggle of each layer (L7); the preset an effect has loaded, by name, and the presets themselves so that they travel with the show file to another computer (L48, L50; which presets and what happens on a name clash is topic C's); the nudge amount; the Master Signal fader, the master opacity and the global Transform; its keyboard and MIDI mapping; the window layout; the canvas size; which output screens were connected and on when it was saved (L96; topic G's rule). (a, layout) "the show will hold the layout and so will the comp" (L94): opening a show puts the window into the layout that show holds; the computer also keeps the layout last used, and that one is used for a new show and for a show that holds none (F-10). The View menu's "Save Layout...", "Load Layout..." and "Reset Layout" stay. (b) His own signals and the macro knobs of clip, layer and global, with their names and what they drive, are saved in the show. (c) Every show brings its own keyboard and MIDI mapping; opening a show switches keys and pads to that show's; a show that holds none leaves them as they are; a new show starts with the mapping of the last show; the mapping of another show can be imported; mapping files exist as well (L116; topic J's rule). (d) The computer holds, for every show: the presets (they are kept with the app AND in the show file, L50); each output screen's own settings; MilkDrop's favourites; the controller whose pad lights were chosen; the tooltip switch; the tempo used last (question 175 default, topic A); which show was open last. (e) Held nowhere: what is playing; the beat's play, pause or stop; the cue buttons; the audio input and its gain. (f) Recordings, films, snapshot pictures and recorded clips are files of their own, not inside the show; Collect Media copies recorded clips along with the other media; each show recording also saves, with itself, a show file with the clips exactly as they were (L80; topic E's rule). (g) At launch the app opens the very last show (L94), not a new empty one. Snapshot: see the answer R188-snapshot; until he says otherwise it stays a command that saves a still picture file and is never part of a show (F-16).
CHANGED: (a) layout: the page said the show holds it and asked "say so if the layout should belong to the computer instead"; his words: "the show will hold the layout and so will the comp" -- "comp" read as the computer (INFERRED; he writes "comp resource" for the computer in L6; the other reading is in F-10). (a) preset: the page kept only the preset's NAME in the show; L48 "they should travel with the show file" and L50 "kept with the app and the show file" put the presets in the show file too. (c) and the page's R189 (h): "the app opens with it" in a new empty show is replaced by L94 "it opens to the very last show". (d) "each output screen's settings" on the computer: L96 adds that the SHOW remembers the connected outputs. (f) snapshot: he asks back "Do we need snapshot?" -- answered below, not yet ruled. Added, from his words elsewhere (L9): the layer's ignore-actions toggle (L7), the action loop toggles (L64), a recording's own show file (L80), mapping files (L116). The rest of the reading stands.
TODAY: The show file has "keys" and "layout" blocks written empty (area-show-decks T7); not saved: clip pause, mapping, layout, user signals, macros (T8, U-7, C5; masterSignal IS saved). A layout is applied only by Load / Reset Layout, never at launch or with a show (T24). The app always launches into a new "Untitled" show (T3). Presets are kept with the app only (R-20). Output screens live in settings.json (T8). To build: all of the above, plus a "last show" entry in settings.json.
@@END
@@ITEM R189
TITLE: Opening, saving and leaving a show
STATUS: REPLACED part (h) only; (a) to (g) and (i) to (k) stand
HIS: L94
RULE: (a) The quit window comes at every quit ("Quit!", with Quit, Cancel and Save & Quit; Return saves and quits, Esc cancels). (b) A deck taken from the list of shows becomes the deck on screen, and the pads then fire that deck (question 197 A). (c) Collect Media is a command of its own; Save never copies media. (d) New, and every way of opening another show, ask the same question as quit: do not save, cancel, save first. (e) The show's name stands in the window's title bar; a Save that worked shows nothing; a Save that failed shows the "Save failed" box. (f) "Open..." opens a show file from any folder; the list shows the shows in the app's own folder and has an entry "Add show..." that copies a show file from anywhere into it, so that its decks can be taken. (g) Collect Media makes a copy (a folder with the media and a copy of the show that points at them; the copy of the show carries its presets, L48) and leaves the open show as it was. (h) REPLACED by L94: "When you open the application, it opens to the very last show." The app opens the show that was open when it was last quit, from its file, with its layout and its mapping, layers empty (question 196). If there is no last show, or its file is gone or cannot be read, the app opens a new, empty show and says nothing (F-11). (i) The show that is open cannot be deleted from the list. (j) Two decks may have the same name. (k) Save & Quit on a show that has no file yet opens the Save As window; if it is cancelled the app stays open; if the save fails the "Save failed" box shows and the app stays open.
CHANGED: (h) the page said "The app opens with a new, empty show, as today"; his L94 replaces it. Nothing else.
TODAY: Quit quits at once with no window (area-show-decks T11); New and a list click ask OK / Cancel with no Save choice, Cmd+O asks nothing (T10); the title is always "Audio-DNA" (T4); the list reads one folder (T12); Collect Media re-points the live show (T25); launch builds "Untitled" (T3, MC:516). All of (a)-(k) is still to build except the "Save failed" box (T5).
@@END
@@ITEM R190
TITLE: Columns and deck tabs
STATUS: STANDS
HIS: none
RULE: (a) The Column menu does what its entries say: "Insert Before" and "Insert After" put a new column before or after the column of the selected clip, "Remove Column" removes that column and the clips to the right move over; with no clip selected only "New Column" works, at the end. (b) A clip that is playing keeps playing when columns shift (its layer follows the clip); a clip whose own column is removed plays on until it is replaced. (c) Keys and pads stay on their column numbers: after an insert in the middle a pad fires the clip that is now in that column (question 197 A). (d) When the deck on screen is removed, the next one is shown (the one before it, if it was the last). Tested against his whole message: nothing in it says otherwise; copy, cut and paste of clips (L4, L5) follow the same two principles (pads stay on cells; what plays is not changed by editing the grid, see F-3).
CHANGED: nothing.
TODAY: Insert Before / Insert After / New Column all append at the end; Remove Column removes the last one (area-show-decks T18, MC:7028-7058). A layer finds its clip by (deck, column), so a shifted column would change what the layer shows (U-10). Removing the shown deck shows the next, else the previous (M-4): already so.
@@END
@@ITEM R225
TITLE: Menus and files of shows and decks
STATUS: STANDS (the old deck files of (a): see F-13)
HIS: L69
RULE: (a) "Save Deck", "Save Deck As..." and "Load Deck..." leave the Deck menu and the tab's right-click menu; the "+" tab only makes a new empty deck; the Decks fold of the list goes; a deck is taken from the list of shows. "Rename Deck", "Duplicate Deck" and "Remove Deck" stay. (b) The list of shows is in alphabetical order; a show is renamed in Finder only; deck tabs keep the order in which the decks were made and cannot be dragged. (c) A show file dropped on the window or double-clicked in Finder does nothing; a show is opened with "Open..." or from the list (and by the app itself at launch, L94). (d) A clip whose file is gone shows a red border and a "!" in its cell and does not play; "Relocate Missing Files..." finds the files again by name and changes the open show only, to be saved afterwards. (e) Shows are kept in the app's folder under Library; Save As starts there.
CHANGED: nothing by a word that names it. Touched by L69: the page's sentence in (a) "Deck files you saved earlier stay on the disk and can no longer be opened" is overtaken IF his "old show files" include the old deck files (assumed so, F-13: they go to the Trash with the old shows).
TODAY: Deck menu and tab menu still hold Save Deck / Save Deck As / Load Deck (area-show-decks T13, T15, T16); the list has a Decks fold (T12); list sorted alphabetically (T12); a dropped .json does nothing (M-5); folders per T29.
@@END
@@ITEM D23
TITLE: New show-file version; the safe write stays
STATUS: OPEN (technical; half settled by L69)
HIS: L69
RULE: The show file gets a new version number when actions, the saved pause, the mapping, the layout, signals, macros and presets arrive in it. Every Save is read back and checked before it replaces the file on disk; a file of an older version is copied to the backups folder before it is written over. Because the old show files are removed before the build starts (L69: "let's delete all the old show files and start from scratch"), no new work is done to carry shows of the old shapes into the new one. THE DELETION ITSELF (L69) is a destructive act on his own files: it is done only when the build starts, never before; the files are named to him first, one by one, with size and date; after his yes they are moved to the Trash, never erased; nothing else in those folders is touched (which files: F-13).
CHANGED: The page said the protection "stays" for his old files; L69 removes those files, so what stays is the safe write itself, as a feature of the app for every later version change.
TODAY: Version 2, the verified writer and the backups copy are built and merged (area-show-decks T5, T32, Pitfall 68, src/core/ShowFile.h). The converter for old-shape shows is merged (O-11). U-T1: the "routines" / "routineBank" keys go. His files: shows and decks in ~/Library/AudioDNA, settings / bindings / layouts in ~/Library/Audio-DNA, takes / snapshots / recordings in ~/Documents/Audio-DNA (T29); one real old-shape show exists (O-11, T33); none of them was opened, listed or touched for this paper.
@@END
@@ITEM D24
TITLE: Bad characters in a show's file name
STATUS: OPEN (his words were about deck files)
HIS: none
RULE: A character that a file name cannot hold (such as "/" or ":") in a show's name becomes "-" in its file name: a show named Club/Night is saved as Club-Night. The same holds for the folder Collect Media makes from the show's name.
CHANGED: nothing; his message of 2026-10-07 does not touch it. His only words: "switch to - is fine for all bad chars" (binding-decisions.md:679), said for deck files, which now leave.
TODAY: Nothing sanitises a name before it becomes a file name (area-show-decks T15, O-4, R-26).
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
STATUS: ANSWERED
HIS: L4, L1
RULE: His words (L4): "We need copy and paste for any clip. If it is selected, it can be copied and then if they empty sell or sell with something else in it is selected then it can be pasted." Built as: (1) COPY: with one or more clips selected, Copy (Cmd+C, Edit > Copy, the cell's right-click menu) puts them on the app's clipboard. Any clip can be copied: a video, a picture, a sequence, a source, a camera, an effects-only cell. (2) WHAT A COPY CARRIES (F-2): the whole clip: its media or source and the source's settings, its name, its play settings (BPM mode or timeline mode, speed, direction, loop mode, in and out points, fit, beat snap), its saved pause, its effects with every slider and every signal plugged into them, its clip macros, and its actions with their own on / off buttons (question 165 A, his accepted default). It does NOT carry: its place in the deck; the key or pad that sits on its cell (pads stay on cells, question 197 A); the keys put on its actions (a copied action has no key); whether it is playing. (3) PASTE: with a cell selected, empty or holding another clip, Paste (Cmd+V, Edit > Paste, the right-click menu) puts a new, independent copy there. Later changes to the copy never change the original. A clip that was in the cell is replaced; one Undo brings it back. Paste can be repeated: the clipboard keeps its content. (4) THE CELL TO PASTE INTO is marked without firing it: a filled cell by a click on its name, any cell (also an empty one) by a right-click, whose menu holds Copy, Cut, Paste and Delete (F-1). (5) PLAYING: pasting, cutting or deleting never changes what a layer is playing at that moment: a clip that is playing and is pasted over, cut or deleted plays on until something else is fired on its layer; the new clip in the cell plays when it is fired (F-3). (6) CUT (Cmd+X, Edit > Cut) = copy, and the cell is emptied; the Delete key empties the selected cells without touching the clipboard (question 208 A, default by L1; F-4). (7) ACROSS DECKS: the clipboard survives a deck switch; a clip is pasted into any cell of any layer of any deck of the open show (F-6). (8) SEVERAL CLIPS: copied together they are pasted in the same arrangement, the top-left one landing in the selected cell; a clip that would land outside the grid is left out (F-5). (9) UNDO: each paste, cut and delete is one Undo step; Undo and Redo never change what is playing (F-7). (10) One thing is on the clipboard at a time, a clip or an action; Paste does what fits the content.
CHANGED: New (a general point; no page text). "sell" read as "cell" twice, and "if they empty" as "if the empty": INFERRED. It makes question 208's default A (topic J: Edit menu with Cut, Copy, Paste, Delete; clips get real copy and paste) his own word for clips, and replaces R145 (d) of topic D ("there is no copy or paste of clips ... and none is added by this").
TODAY: No copy or paste of clips exists; Cmd+X is "Clear" on the selected clip cells (area-show-decks U-12, MC:4130-4135); no Edit menu, Undo / Redo sit in "Composition" (T1). Selecting = a click on the name bar (T20). SetClipCmd / SwapClipsCmd exist as Undo commands (MC:5301). A Duplicate re-mints clip ids (Pitfall 36): a paste must do the same. What a playing layer does when its cell is replaced: not checked.
@@END
@@ITEM G3
TITLE: Option-drag copies a clip into the cell dragged to
STATUS: ANSWERED
HIS: L5
RULE: His words (L5): "Option drag on a clip copy and pastes into the cell it is dragged to." Built as: dragging a clip by its name with the Option key held puts a copy into the cell it is dropped on and leaves the original where it was. The copy carries exactly what a pasted copy carries (G2, part 2) and follows the same rules: the target may be empty or filled; a clip in the target is replaced, not swapped (F-8); nothing that is playing changes (F-3); one Undo step removes the copy and brings back what was in the target (F-7). The copy can land on any layer of the deck on screen (F-9). Option-drag does not touch the clipboard. Whether it is a copy or a move is decided by the Option key at the moment of the drop. A drag without Option moves the clip; this rule adds nothing to it.
CHANGED: New (a general point; no page text).
TODAY: A drag from the name bar moves the clip; onto a filled cell the two swap ("Move Clip" / "Swap Clips", one Undo unit: MC:1205-1235; ClipCell.cpp:297-305). No Option handling in ClipCell (grep isAltDown: none). The drop works inside the shown deck only (composition_.activeDeckIndex, MC:1232).
@@END
## ASSUMPTIONS
@@ASSUME F-1
ABOUT: G2
TEXT: I assume an empty cell is picked for pasting with a right-click (its menu has Copy, Cut, Paste, Delete), because a plain click on an empty cell clears that layer.
WHY: L4 says an empty cell "is selected" and then pasted into; L11 and L19 say a click on an empty cell triggers it and the layer goes empty.
ALT: b) A plain click on an empty cell selects it AND clears its layer, as his Resolume does. c) Only filled cells are selected by click; an empty one is reached by hovering the mouse over it and pressing Cmd+V.
IF-WRONG: STAGE a layer could go empty in a show when he only meant to paste.
ASK: YES his two rules meet in one click; no word of his says which wins.
@@END
@@ASSUME F-2
ABOUT: G2, G3
TEXT: I assume a copied clip brings everything it has: its effects with their sliders and signals, its actions, its play settings and its saved pause. It does not bring the key or pad of its cell.
WHY: L4 and L5 say "copy" and nothing about what travels; actions come from his accepted default 165 A; pads on cells from default 197 A.
ALT: b) The copy brings the media and its play settings only, without effects and actions.
IF-WRONG: SMALL one list of fields in the copy.
ASK: LINE he most likely means a full copy, as Resolume does.
@@END
@@ASSUME F-3
ABOUT: G2, G3, R190
TEXT: I assume that pasting over, cutting or deleting a clip that is playing does not change the picture: it plays on until you fire something else on that layer.
WHY: L4 allows pasting into a cell "with something else in it"; no word says what the layer does if that something is playing.
ALT: b) The layer switches at once to the pasted clip. c) The layer goes empty at once.
IF-WRONG: STAGE the output would change, or not change, at an edit made during a show.
ASK: YES seen on stage, and no word of his covers an edit to a playing cell.
@@END
@@ASSUME F-4
ABOUT: G2
TEXT: I assume the keys are Cmd+C, Cmd+X and Cmd+V, that Cmd+X is a cut you can paste, and that the Delete key empties the selected cells.
WHY: L4 names copy and paste only; the keys and the cut come from the default of question 208, which he did not name (L1).
ALT: b) Cmd+X keeps only clearing the cell, with nothing to paste afterwards.
IF-WRONG: SMALL a key binding.
ASK: NO it is his accepted default, restated.
@@END
@@ASSUME F-5
ABOUT: G2
TEXT: I assume several selected clips are copied together and pasted in the same arrangement, the top-left one landing in the cell you selected.
WHY: L4 speaks of one clip ("it is selected"); cells can be selected several at a time.
ALT: b) Copy and paste work on one clip at a time only.
IF-WRONG: SMALL the block paste can be added or left out later.
ASK: LINE a choice of mine he would most likely wave through.
@@END
@@ASSUME F-6
ABOUT: G2
TEXT: I assume a copied clip can be pasted into any deck of the open show, and stays ready to paste until you copy something else or quit.
WHY: L4 does not say how far a copy reaches or how long it lasts.
ALT: b) The copy is lost when another show is opened.
IF-WRONG: SMALL
ASK: NO technical; decks are boxes of clips by his own rule, so any deck is implied.
@@END
@@ASSUME F-7
ABOUT: G2, G3
TEXT: I assume each paste, cut, delete and Option-drag is one step for Undo, and Undo never changes what is playing.
WHY: L4 and L5 say nothing of Undo; his earlier rule is that Cmd+Z changes nothing that is live in the layer strip.
ALT: none
IF-WRONG: SMALL
ASK: NO follows his standing Undo rule.
@@END
@@ASSUME F-8
ABOUT: G3
TEXT: I assume an Option-drag onto a cell that holds a clip replaces that clip with the copy, and a drag without Option still moves the clip.
WHY: L5 says the copy goes "into the cell it is dragged to" and not what happens to a clip already there.
ALT: b) An Option-drag onto a filled cell is refused. c) The clip that was there moves to the next free cell of its layer.
IF-WRONG: SMALL one Undo brings the replaced clip back.
ASK: LINE the same as paste over a filled cell, which his L4 allows.
@@END
@@ASSUME F-9
ABOUT: G3
TEXT: I assume Option-drag copies inside the deck on screen; to copy a clip into another deck you use copy and paste.
WHY: L5 names the cell dragged to and no deck; a drag cannot reach a deck that is not shown.
ALT: b) Holding the drag over a deck tab shows that deck, so the clip can be dropped there.
IF-WRONG: SMALL the tab hover can be added later.
ASK: LINE a limit of mine he may want lifted.
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
TEXT: I assume "the very last show" is the show that was open when you last quit. If its file is gone, or there is none yet, the app opens a new, empty show and says nothing.
WHY: L94 says "it opens to the very last show" and not whether last opened or last saved, nor what happens when the file is missing.
ALT: b) The show you SAVED last, even if another one was open at quit. c) If the file is missing the app says so in a box.
IF-WRONG: SMALL the two differ only when a show was opened and not saved.
ASK: LINE a small choice of mine he can strike.
@@END
@@ASSUME F-12
ABOUT: R188, R189, 196
TEXT: I assume that when the app starts and opens your last show, the output screens that show was saved with come on by themselves, if they are connected.
WHY: L94 (the app opens the last show) and L96 (a show opened with its outputs connected remembers them) together; the standing rule was that the app never opens an output by itself.
ALT: b) At launch the outputs stay off until you press "Restore Last Outputs"; they come on by themselves only when you open a show yourself.
IF-WRONG: STAGE a projector lights up, or stays dark, at launch in front of a room.
ASK: YES it reverses a standing rule, and it is seen by an audience.
@@END
@@ASSUME F-13
ABOUT: R225, D23
TEXT: I assume "old show files" means the show files in the shows folder, their backups and the old deck files; not recordings, snapshots, media, presets or settings. I name each file before it goes to the Trash.
WHY: L69 "let's delete all the old show files and start from scratch" does not say which files; deck files and backups are not shows by name.
ALT: b) Shows and their backups only; the old deck files stay on the disk. c) Also the old recordings (takes) and the saved layouts and exported key settings.
IF-WRONG: REBUILD files of his would be moved that he wanted kept (recoverable from the Trash), or left that he wanted gone.
ASK: YES a destructive act on his own files.
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
TEXT: I assume a character that a file name cannot hold, such as "/", becomes "-" in a show's file name: a show named Club/Night is saved as Club-Night.
WHY: He did not read this point (L130); his "switch to - is fine for all bad chars" was said for deck files, which now leave.
ALT: b) The Save As window refuses such a name.
IF-WRONG: SMALL
ASK: LINE his own rule carried from deck files to show files.
@@END
@@ASSUME F-16
ABOUT: R188
TEXT: I assume Snapshot stays as it is, a command that saves one still picture of the output to a file, and has no part in saving or opening a show.
WHY: L94 "Do we need snapshot?" is a question to Harmony, not a ruling; the answer is given to him and his word decides.
ALT: b) Snapshot is removed from the app.
IF-WRONG: SMALL one menu entry.
ASK: NO carried by the answer to his question; not asked twice.
@@END
@@ASSUME F-17
ABOUT: 198
TEXT: I assume that after a crash the app opens your last show as you saved it and then offers the safety copy once. The safety copy is made about every three minutes.
WHY: The default of 198 says "every few minutes" and "offers it once"; L94 adds that the app opens the last show at launch; the order and the interval are not said.
ALT: b) After a crash the app opens the safety copy directly and says so.
IF-WRONG: SMALL
ASK: LINE a small choice of mine he can strike.
@@END
@@ASSUME F-18
ABOUT: R188
TEXT: I assume the new switches are saved in the show like everything else: each action's loop switch, each layer's ignore-actions switch and the global glide-back time. The cue buttons and the master cue switch are not saved.
WHY: L7, L64 and L38 add new controls after the list of what a show holds was written; none says where it is kept.
ALT: b) The glide-back time belongs to the computer, for every show.
IF-WRONG: SMALL
ASK: NO follows his One Save rule; the owners are topics D and B.
@@END
@@ASSUME F-19
ABOUT: R188, R189, R225
TEXT: I assume the thing you save and open is called a "show" everywhere on screen (the menu "Show", "New Show", "Open...", "Save"), and "global" names the level above the layers.
WHY: L55 moves the level's name from composition to global; he writes "show" and "show file" throughout (L48, L50, L69, L94) but did not rule the menu's name.
ALT: b) The menu and the file keep the name "Composition", as in Resolume.
IF-WRONG: SMALL words on screen; settled in the list of names he asked for (L114).
ASK: LINE a name of mine that he can strike.
@@END
## QUESTIONS BACK
@@ANSWER R188-snapshot
HIS: L94
ANSWER: No, not for saving anything. I recommend keeping Snapshot only as what it is: one command that saves a still picture of the output as an image file, for a flyer, a post or a note of a look. It was on the page only because the old list of "things you can save" had a line for it, and I wanted to say that a show does not keep its snapshots. Nothing else leans on it: the show holds the layout and everything in it, the app opens your last show, and a crash is covered by the safety copy. So it is never part of a show, it is not a way to store or recall a look, and it gets no further work. If you never use it, say "drop snapshot" and it leaves the app.
@@END
## NAMES
@@NAME show
MEANS: The one file that is saved and opened; it holds the decks, layers, clips, effects, actions, mapping, layout and presets.
SOURCE: his words L48, L50, L69, L94 ("show file", "the show"); as the menu's name it is Harmony's pick (F-19), replacing the on-screen name now, "Composition"
@@END
@@NAME last show
MEANS: The show that was open when the app was last quit; the app opens it by itself at launch.
SOURCE: his words L94 ("it opens to the very last show"); "open at quit" is Harmony's reading (F-11)
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
@@NAME Snapshot
MEANS: A command that saves one still picture of the output as an image file; never part of a show.
SOURCE: the on-screen name now (Output menu "Snapshot"); whether it stays is his to say (L94, answer R188-snapshot)
@@END
@@NAME Collect Media
MEANS: The command that makes a folder with a copy of the show and copies of all its media, leaving the open show as it was.
SOURCE: the on-screen name now; kept by his "82 default" (binding-decisions.md:890)
@@END
@@NAME keyboard and MIDI mapping
MEANS: Which key, pad or knob does what; every show holds its own and the computer keeps the last one.
SOURCE: binding-decisions.md:1099 ("we call it keyboard and midi 'mapping'"); L116 ("we want mapping files")
@@END
## CONFLICTS
- Launch and outputs. His L94 "When you open the application, it opens to the very last show." with his L96 "if I save a show with the outputs connected, and I open the show back up with the outputs connected, I expect the show to remember the outputs connected and not need to connect them again." AGAINST the standing rule told to him as reading R191 (g), which he did not correct in that part: "The app never opens an output by itself" (slice-G.md:29; CLAUDE.md "Outputs"). Read together his two lines make a projector come on at launch. Asked: F-12.
- Selecting an empty cell. His L4 "if they empty sell or sell with something else in it is selected then it can be pasted" AGAINST his L11 "the empty cell triggers on the 1 if the clip playing is in bpm mode" and L19 "layer goes empty when a column is triggered with an empty": a plain click cannot both select an empty cell and leave its layer alone. Asked: F-1.
- "the comp". His L94 "the show will hold the layout and so will the comp" can mean the computer (as in L6 "comp resource") or the composition (binding-decisions.md:904 "if comp is saved"). Not a contradiction, two readings: F-10.
- Presets and the show. His earlier "these are saved with the app. always." (binding-decisions.md:894) against L48 "they should travel with the show file" is resolved by his own L50 "they are kept with the app and the show file": both. Listed so that the older line is not built alone.
## NOT DONE / UNSURE
- Which presets a show file carries (all of the app's, or only those its effects use) and what happens on another computer when a name already exists there: topic C's (R155, R157). R188 (a) here only says that they travel.
- What "remember the outputs connected" stores (which screens were on; or also each screen's Delay and colour): topic G's (R191). Only the launch consequence is raised here (F-12).
- The exact list of old files for L69: NOT looked up, on purpose (no file of his was opened, listed or touched). Cheapest way: at build start, one listing of ~/Library/AudioDNA/compositions, its backups folder and ~/Library/AudioDNA/decks, shown to him by name, size and date; then moved to the Trash through Finder under RIG-RULES A3 (named, checksummed), never erased.
- What a layer that is playing does now when its cell is replaced or cleared (the TODAY side of F-3): not checked; one read of SetClipCmd and Layer's (deck, column) lookup settles how much there is to build.
- Whether his Resolume selects an empty cell on a plain click (ALT b of F-1): not known; his answer to F-1 settles it without a look.
- The interval of the safety copy (F-17: three minutes) is a guess inside "every few minutes".
- That nothing but the user's Snapshot command depends on snapshot pictures (the answer R188-snapshot) is INFERRED from the fact sheet and the Output menu (MenuBarModel.cpp:143), not from a full search of the source.

Written 2026-10-07 22:42:40 by the architect for topic F (read-only; HEAD a86cf0a).
STATUS: DONE
