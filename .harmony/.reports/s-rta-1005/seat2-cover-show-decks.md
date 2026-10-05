# SEAT 2 (round 2) cover-show-decks: show file, saving, decks, deck tabs, columns, grid, deck names
Written 2026-10-05 16:43:55 by the blind seat. Read-only; nothing run. List read: boris-all-items.json (44 questions 172-215, 87 readings, 28 decided, 31 design, 24 conflicts). Sheet: area-show-decks.md (parts 1-6 + CORRECTIONS + MISSED).
Labels: VERIFIED = I read it (file:line or items-file text). INFERRED = reasoned (from what). UNKNOWN = not established.

## 0 Re-checked from source (five or more load-bearing lines of the sheet I lean on)
- VERIFIED Column menu: Insert Before / Insert After append at the END, Remove Column removes the LAST column (MainComponent.cpp:7028-7058; col = numColumns-1). No "selected column" exists: the column numbers are fire buttons (DeckView), no selection state found.
- VERIFIED Open... (Cmd+O) goes to a chooser and loads with no question; only library click / New ask "Everything playing now will be replaced..." (MainComponent.cpp:3171-3200).
- VERIFIED Quit calls quit() at once, no window (Main.cpp:34-36, 87-90); window title native, never set per show (Main.cpp:50, no setName).
- VERIFIED New deck "Deck N" with N = decks+1, no uniqueness check (DeckCommands.h:703). Tab floor 60 px, "+" clamped, never scrolls (DeckTabRow.h:10-24).
- VERIFIED Cmd+X = Clear on selected cells, no clipboard (MainComponent.cpp:4122-4136). A pad beyond the deck's width: hasCell false -> unchanged; an empty cell inside it clears the layer (ClipRef.h:55-60, Layer.h:410-419) = decided line 28 is right.
- VERIFIED Menu strings: "Composition" menu items, Deck menu (Save Deck, Save Deck As..., Load Deck...), "Shortcuts" menu, View > Save Layout.../Load Layout.../Reset Layout (MenuBarModel.cpp:70-95, 112-120, 155-169). R148's strings: BindingOverlay.cpp:68, MidiLearnOverlay.cpp:95, MainComponent.cpp:7342, 7357.
- VERIFIED no serializer for user signals / macros (grep toVar/fromVar in src/signal, src/routing: none; no key in Composition.h) = R188(b) is right.
- VERIFIED missing media today: a clip cell with a missing file shows a red border and "!" (ClipCell.cpp:203-207); "Relocate Missing Files..." re-points by name search, in memory (MainComponent.cpp:6757+).
- VERIFIED the ten preset slots are a 28 px bar at the BOTTOM of the main area (MainComponent.cpp:2780-2790); "Save", "Load", "FX Save" are in row 1 at the top (MainComponent.cpp:2763-2767). => R211 says "at the top of the window" for the slots: WRONG.
- VERIFIED the second message is handled: R148 gives today's strings and the term "keyboard and MIDI mapping"; intro line 1 quotes "ask me questions about anything that is unclear when you are ready". No other text uses "bindings", "overlay", "key / pad list" except R148 quoting today's strings (grep of all items) and Q203's unrelated "Overlay" blend name.
- NIT (sheet, not list): sheet T31 says the top bar x1/x2/x4 store numbers nothing uses; TopBar.cpp:401-426 stores bpmMultiplier AND calls onBpmMultiplierChanged, so R163's "the show holds its /2 x2 setting today" is plausible. Not my area; area-tempo seat to settle.

## 1 Coverage table: sheet part 4 (HIS), MISSED, part 5, part 3
Covered = his answer (or silence) lets a builder go on.
| Point | Home | Verdict |
|---|---|---|
| U-1 pads follow the deck on screen | Q197 (A today, B pad belongs to its clip, C per pad); decided 28 for the width edge | covered. His "lock a performance deck" (sheet C) not offered; C per pad approximates it. NIT |
| U-2 re-opened show: empty or as it was | Q196 + R142 + R163 | covered |
| U-3 deck from a file outside the folder | R189(f) ("Open..." any folder; list gets "Add show..." which copies) | covered by a reading, but the visible-folder point (sheet option D) is NOT: Save As starts in a hidden ~/Library folder. SHOULD F2 |
| U-4 grid jumps to a taken deck | R189(b) | covered (told in Q91, consent inferred). Does not say that under Q197 A the pads then fire that deck. SHOULD F6 |
| U-5 quit window, New, Open | R189(a),(d); R198(d) | covered for the three buttons. Edges of "Save & Quit" NONE. SHOULD F4 |
| U-6 which show is open | R189(e) (name in title bar; failed save = box) | covered |
| U-7 what a show keeps; signals, macros | R188(a)-(f), R210(a) | covered |
| U-8 layout: show only or computer | R188(a) (layout in the show) | PARTLY: nothing on the View menu's Save Layout / Load Layout / Reset Layout, nor on whether opening a show re-arranges the window (today never). SHOULD F5 |
| U-9 launch and crash | Q198 + R189(h) | covered |
| U-10 Column menu | R190(a)-(c) | covered but "the selected column" does not exist today. MUST F2 below (numbered F1/F3 in the return) |
| U-11 recording and its show | R173, R187(b) | partly: when an action is saved from the review screen, is the show file written at once or does the open show wait for Save / the quit window? SHOULD F7 |
| U-12 Cmd+X and copy | Q208, R145 | covered (Q208 names R145; R145(d) goes stale if he picks A: NIT) |
| U-13 names | Q210, R148, R197 | covered; (d) "Clear Clips" twice (deck menu / layer menu) not mentioned: NIT |
| U-14 Collect Media | R189(c),(g); R188(f) | covered; a failed or partial copy has no message rule (Q209 does not list it). SHOULD F8 |
| U-15 list order, show rename, deck order | design item 24 names "its order, renaming a show" but its variants are only about tabs | NONE as a reading. SHOULD F9 |
| U-16 mapping travels / mismatch | R199(b),(f), R188(c), decided 28, Q207, Q214 | covered |
| U-17 actions and presets when a deck is taken | decided 14, R171 | covered |
| U-18 open/New while recording | R187(b) | covered; two windows could be built (R187(b) ask + R189(d) ask). SHOULD F7b |
| U-19 duplicate deck names | R189(j) | covered (Duplicate Deck "<name> copy" twice is included by "may have the same name") |
| M-1 many deck tabs | design item 24 | covered as a picture |
| M-2 pad beyond the deck's width | decided 28 | covered, VERIFIED right |
| M-3 undo history thrown away | R198(d) | covered (as today) |
| M-4 deck on screen after removal | R190(c) | covered; "next one" omits "else the previous if it was last" (DeckCommands.h:947-953 per sheet). NIT |
| M-5 show / deck file dropped on the window | none | NONE. SHOULD F10 |
| M-6 import can erase | R199(b),(f) | covered |
| M-7 deleting the open show | R189(i) | covered |
| M-8 / U-T1..T10 | architects; decided 22 | n/a |
| C1-C6 (corrections) | C3 told Q91; C4 R189(j); C5 R210(a); C6 Q197 | covered |
| Part 5: R68 quit at every quit | R189(a) | covered |
| Part 5: R1 sec 7 readings (first Save of his old show keeps a backup; Deck 1's layer settings win; his four old files in Decks cannot be opened) | R143 (backups, "opens as today") | PARTLY: the "four old deck files" and the removal of Save Deck / Load Deck / the Decks fold are nowhere. MUST F3 |
| Part 5: drafted 172-176,183,185 | in the list (172-175, 176-177, 183, 185) | covered |
| O-1, O-2 | built; code follows later word | n/a |
| O-3 sync in the show | R177, Q199, R163 | covered |
| O-4 bad characters in a name ("-") | none | NONE (his ruling BD:679 still applies to the show's file name). SHOULD F11 |
| O-5 | R188(c) | covered |
| O-6 "A deck - Save Deck" on his saves list | not stated as gone | see MUST F3 |
| O-7 routines | decided 12, R143 | covered |
| O-8 messages | Q209 | covered for the four; Collect Media failure: F8 |
| O-9 Return = Save & Quit | R189(a) | covered |
| O-10 "Shortcuts" name | R148 | covered |
| O-11 old shows | R143 | covered |
| O-12 resume vs restart | R128(d), R139(b) | covered |
| O-13, O-14, O-15 | docs vs source; R189(d) | n/a |
| T26 old Save / Load / FX Save and ten slots | R211 | WRONG place for the slots. MUST F1 |
| T25 Relocate Missing Files, missing-media look | none | NONE. SHOULD F12 |
| T1-T33 other lines | no list item contradicts them | checked |

## 2 Findings
F1 MUST R211: "The ten numbered preset slots at the top of the window, with their small buttons "Save", "Load" and "FX Save"" -- the ten slots are a bar at the BOTTOM of the main area (MainComponent.cpp:2780-2790); only Save / Load / FX Save are in the top row (:2763-2767). Fix: "The ten numbered preset slots in the bar at the bottom of the window, and the small buttons "Save", "Load" and "FX Save" in the top row, go, as you ruled (86)." (the rest unchanged)
F2 MUST R190(a): "insert at the selected column and remove the selected column" -- no column can be selected today (a click on a column number fires it). Fix: "(a) ... "Insert Before" and "Insert After" put the new column before or after the column of the clip you have selected, "Remove Column" removes that column, and the clips to its right move over. With no clip selected, "New Column" adds one at the end and the other three entries are grey. A key or pad on a place stays on that place (Q197 A), so after an insert it fires another clip."
F3 MUST NEW reading (Save Deck, Load Deck, the Decks fold): add to R189 as (k): "(k) As you ruled (your 80, 91): Save Deck, Save Deck As... and Load Deck... leave the Deck menu and the tab's right-click menu, and the "+" tab only makes a new empty deck. The Decks fold of the Browser's list goes: a deck is taken from the list of shows. Your four old deck files stay in their folder and cannot be opened by the app any more. Rename Deck, Duplicate Deck and Remove Deck stay as today. Say so if wrong."
F4 SHOULD R189 NEW (k2) Save & Quit: "Save & Quit on a show that has no file opens the Save As window; if you cancel it, the app stays open. If the Save fails you see the "Save failed" box and the app stays open. Cancel and Esc leave everything as it is."
F5 SHOULD R188 NEW sentence for the layout: "(a2) The View menu keeps "Save Layout...", "Load Layout..." and "Reset Layout"; a layout you save there is the one the show holds. Opening a show puts the window into the layout it holds; a show without one leaves the window as it is. Say so if the layout should instead belong to the computer (a laptop screen and a venue screen differ)."
F6 SHOULD R189(b): append "...and, as your pads fire the deck on screen (question 197 A), a deck you take during the show changes what they fire until you pick B or C there."
F7 SHOULD R173: append "It goes into the open show in memory; the file changes only at your next Save, and the quit window catches it." (or, if he wants, at once -- ask as a reading, default: with Save). R187(b)/R189(d): "when a recording runs and you open another show, there is ONE window: it says the recording is stopped and kept, then asks Don't save / Cancel / Save."
F8 SHOULD R189(g) + Q209: add to R189(g): "If a file could not be copied, the app shows a box "Collect Media: N files could not be copied", as a failed Save does." and add to Q209 A's list of windows the app may open itself: "the Collect Media box". Today nothing is shown (MainComponent.cpp:6709-6755 per sheet T25) and the open show is re-pointed even for a failed copy.
F9 SHOULD NEW reading R214 (or R190 (d)): "The list of shows stays in alphabetical order, as today; a show is renamed in Finder only; deck tabs keep the order the decks were made in and cannot be dragged. Say so if you want the newest show first, or tabs you can drag." (design item 24 then holds only how twenty tabs fit.)
F10 SHOULD NEW reading R189 (l): "As today, a show or deck file dropped on the window, or double-clicked in Finder, does nothing; a show is opened with Open... or from the list."
F11 SHOULD NEW decided line (F): "A character that cannot be in a file name ("/" and the like) in a show's name becomes "-" (your 13); two shows can then not share the name by accident only because Save As asks before it overwrites." -- consequence he would notice: a show named "Club/Night" is saved as "Club-Night".
F12 SHOULD NEW reading R190 (e) or R189 (m): "A clip whose file is gone shows a red border and a "!" in its cell; it does not play. "Relocate Missing Files..." stays: you pick a folder and it re-points by file name; it changes the open show only, so Save afterwards."
F13 SHOULD R190(b): reword to state it is NEW: "Today a removed column's clips are erased from the model; a clip that is playing when its column is removed plays on (mine, as a clip of a removed deck does)." Source: sheet U-10 left a (plays on) vs b (stops) open; R190(b) chooses a without saying it is new.
F14 NIT R145(d) vs Q208 (Q208 names it; edit R145(d) if he picks A). NIT R190(c) add "else the one before it". NIT Q210: the inspector tab "Composition" (InspectorPanel.h:87-90) is the tab with global effects and autopilot; say "the Composition tab" in the situation so he knows which. NIT R189(d): REST load and Open from a remote program ask nothing. NIT sheet T31 vs TopBar.cpp:401-426.

## 3 Counts
Points checked: 19 HIS (U-1..U-19) + 7 HIS MISSED (M-1..M-7) + 1 TECH missed + 6 corrections + 5 part-5 groups + 15 part-3 + 33 part-1 lines (read against list items) + 3 items of the second message. NONE / partly: U-3 (visible folder), U-8, U-11, U-15, M-5, O-4, O-6/T13-T16, T25 Relocate, Save&Quit edges, O-8/U-14 message = 10 gaps; 3 are MUST/contradiction items (F1-F3).
