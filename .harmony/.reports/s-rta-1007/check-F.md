# CHECK F -- re-check of apply-F.md (blind, s-rta-1007)
Written: 2026-10-07 22:59:30

## MY OWN READING OF HIS LINES (step 2, written before opening the paper)
- L94 "the show will hold the layout and so will the comp": R188 window-layout part. The page asked "say so if the layout should belong to the computer instead"; he says BOTH. (R188's title is "what a show keeps and what the computer keeps", so "comp" = computer is the best reading; BD:904 "if comp is saved" shows he also writes comp for composition.)
- L94 "When you open the application, it opens to the very last show.": replaces R189 (h) "opens with a new, empty show"; touches R188 (c) "the app opens with it", Q196 (layers empty at launch), Q198 (what a crash opens), and by L96 the outputs at launch.
- L94 "Do we need snapshot?": a question TO Harmony about R188 (f) (the One Save list's line "A snapshot - a still picture of the screen", BF backlog 558). Needs an answer, not a ruling.
- L4: copy+paste for any clip; target = empty cell or a cell with something in it. L5: Option-drag copies into the cell dragged to. Both leave open: what a copy carries, selecting an EMPTY cell (L11 and L19 make a click on an empty cell fire/clear), paste over a playing clip, clipboard vs drag, several clips, other decks.
- L69 (R143): "delete all the old show files and start from scratch": kills the converter need; the files named are "old show files" only.
- L48/L50 (R155/R157): presets travel with the show file AND are kept with the app -> R188 (d); but L52 (178) says the header shows the preset name only while the settings equal a preset -> topic C stores no name on an effect.
- L7 + L73: ignore-actions toggle on a layer (one thing said twice, L9) -> saved in show. L64: each action's loop toggle -> saved. L67/L70: glide-back time and global fade (two controls).
- L96 (R191): the show remembers the connected outputs -> R188 (a); with L94 the launch.
- L80 (R173): a recording saves its own show file; L116: mapping files exist (R199 f).
- L74 "182 b": a layer's action fires the CELLS of the deck that is shown -> supports 197 A (pads on cells).
- L35 (R170): "various configurations other than live and recording review mode" -> the window layouts the show holds are several (topic B hands this to F/L94).
- L104 (R195 d) and L106 (R221): many envelopes shared by sliders; signal users carry threshold, gain, falloff, one-shot/loop -> these live in the show.
- L8: pictures laid out by Harmony (P26 DROPPED). L9: repeats answered by earlier explanation. L1: no "today" in what he reads. L130: D/P items unread.

## FINDINGS
(see the return value for the compact list; full text here)

MUST
1. R188 (a) + CHANGED: "the preset an effect has loaded, by name" is kept in the show. Topic C (apply-C R157 (8), R171, C-5; item 178 from his L52) rules the opposite: "The show keeps no preset name on an effect"; the header finds the name by comparing. Direct cross-topic contradiction. Fix: delete the phrase from (a); write "no preset name is stored on an effect (topic C, L52); the presets themselves travel with the file (L48, L50)"; fix CHANGED accordingly.
2. R188 layout: L35 "We will have various configurations other than live and recording review mode." was handed to F by topic B (apply-B:333 "belongs with ... the show's layout (L94)") and F carries nothing: is "the layout" one, or one per mode (live, review, later ones)? A builder must guess. Fix: add @@ASSUME "the show holds the window layout of every mode (live, review), and opening a show puts the window into the layout of the mode you are in", ASK LINE; list L35 in HIS.
3. 'last show' defined as "open when the app was last quit" (R189 (h), F-11, NAME last show) but rule 198 and F-17 make the app open the last show AFTER A CRASH, when no quit happened. Fix F-11 TEXT: "the show that was open when the app last closed or crashed (remembered when a show is opened or saved, not only at quit)"; same in NAME and R189 (h).
4. G3 RULE "Option-drag does not touch the clipboard" and "copy or move is decided by the Option key at the moment of the drop" are decisions with no @@ASSUME. His words "copy and pastes into the cell" can be read as using the clipboard. Fix: add @@ASSUME (clipboard untouched; Option read at the drop), ASK NO, ALT: the dragged clip also becomes the clipboard content.

SHOULD
5. F-12 (ASK YES, outputs at launch) and topic G's G-3 (same question, ASK LINE) disagree; he would see it twice or get two ratings. Pick one place (G owns R191) and make F-12 a pointer.
6. F-13 (ASK YES, which files go) duplicates topic D's D-7 (ASK YES) with a DIFFERENT scope (F: shows + backups + old deck files; D: show files only, "recordings, presets and settings not touched"). Merge into one question with one scope.
7. D23 RULE writes Harmony's own procedure (only when the build starts, named one by one with size and date, moved to the Trash) as the rule; his word is "delete". Only the Trash/naming is in F-13; "only when the build starts" is in no @@ASSUME. Put the procedure into F-13's TEXT.
8. R188 (b): the page said "your envelopes" and "shapes and lengths of Mod 1 and Mod 2"; the paper's "His own signals" drops the envelopes, and his L104 (many envelopes, shared by sliders) and L106 (per-user threshold, gain, falloff, one-shot/loop) are not carried as saved in the show. Add them to (b), cite L104, L106.
9. R188 (a) HIS lacks L73 ("a layer can have a global action bypass"), which topic D reads as the same switch as L7 (L9). Cite it.
10. F-18 names only the glide-back time. His L70 adds a separate "Global fade control" for action start/stop jumps (R147); where it is kept is not said. Add it; and ASK LINE rather than NO for show-vs-computer (ALT b is a real choice, nobody else rules it: apply-D R132 does not say).
11. G2 RULE (2): "the keys put on its actions (a copied action has no key)" is not in F-2's TEXT. It comes from R199 (a), which was shown to him (stands). Cite R199 (a) in the RULE or add to F-2's TEXT.
12. F-1 WHY: "L11 and L19 say a click on an empty cell triggers it and the layer goes empty": L19 is about a COLUMN trigger. TEXT "a plain click on an empty cell clears that layer" is also incomplete: by topic A (R168 (b), L11) it clears on the next "1" when the playing clip is in BPM mode. Fix both.
13. R190 (a) talks of "the selected clip"/"no clip selected", but G2 makes an EMPTY cell selectable (right-click). Say "the selected cell" and what Insert/Remove do when the marked cell is empty.
14. F-16 TEXT "Snapshot stays as it is" describes the app now (L1). Write "Snapshot stays a command that saves one still picture of the output to a file".
15. D24 RULE adds "the same holds for the folder Collect Media makes", which F-15's TEXT does not say; add it to F-15. F-15 is ASK LINE; his "switch to - is fine for all bad chars" (BD:679) was for deck files; LINE is acceptable, but then say that in the line.
16. R188 (c) "a new show starts with the mapping of the last show": the page (and BD Q94) said "the show you saved last"; with F-11 "last show" now means "open at quit". State which is meant.
17. F-17 TEXT fixes "about every three minutes", a number he never gave; the 198 RULE says "every few minutes". Keep the number out of the line he reads or say it is a guess.

## MISSING
- Layout per mode (L35): finding 2.
- Envelopes and signal-user settings saved in the show (L104, L106): finding 8.
- Global fade (L70) in the list of what the show/computer holds: finding 10.
- What the View menu's Save Layout / Load Layout files do now that the show and the computer both hold the layout (kept by the paper without a word; his L94 question "Do we need snapshot?" may be next to it). Add as a one-line @@ASSUME, LINE.

## WHAT I CHECKED AND FOUND RIGHT
Quotes verbatim with right L numbers: L4, L5, L69, L94 (both parts), L96, L11, L19, L48, L50, L55, L74, L6, L114, BD:679, BD:904 area. Statuses: 196, 197, 198 (DEFAULT, nothing in his message changes them; 197 is supported by his 182 b = L74), R189 (REPLACED (h) only), R190 (STANDS), R225 (STANDS), P26 (DROPPED, correct under L8), G2, G3 (ANSWERED), D23/D24 (OPEN with assumption). "comp" read as computer, with the other reading in ALT: right (R188's title). ASK YES on F-1, F-3 passes the three tests; F-2, F-5, F-8, F-9, F-10, F-11, F-15, F-17, F-19 LINE are fair; F-4, F-6, F-7, F-14 NO fair. @@ANSWER R188-snapshot answers his question and is plain. No "today" in RULE lines except the small points above.

## LINT OUTPUT
blocks: 12 ITEM (slice has 12 ids), 19 ASSUME (4 ask YES, 9 LINE), 1 ANSWER, 10 NAME
OK
