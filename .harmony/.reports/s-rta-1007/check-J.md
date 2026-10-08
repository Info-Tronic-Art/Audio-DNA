# check-J -- blind re-check of apply-J.md (topic J)  [checked: 19 items, 17 assumptions, 2 answers, names, conflicts]

## MY OWN READING OF HIS LINES (step 2, written before opening the paper)
- L4: copy and paste for any clip; a selected clip is copied, an empty cell or a cell holding something else, once selected, takes the paste. Changes Q208: real clip copy/paste (A); R145 "none added" falls; Undo steps for clip paste.
- L5: Option-drag copies the clip into the cell it is dragged to. Q208 area.
- L7, L73: ignore-actions toggle on the layer; a layer action bypass like column-trigger bypass. New mappable buttons (206).
- L8: new functions laid out by Harmony; look-only picture items (P32, P33, P34) are DROPPED.
- L9: an explanation given once answers the repeats.
- L11 (+L19): empty cell triggers on the 1 if the playing clip is in BPM mode, instantly otherwise. D32 "empty cell clears its layer" gets this timing.
- L55 (R133): composition = global, move to global. Supports 210 A tab "Global".
- L68 (R162, also his answer to 184), L65 (R159), L80 (R173): three messages he wants (paste: muted + small message with OK; warning in action save screen; Review says clips moved or missing). L122 "209 already discussed above" = these.
- L66 (R129 2): a slider the hand moved moves back to the action position at the Global glide-back speed. Touches R199 e.
- L72 (R224): global stop-actions button; tempo stop stops all actions; name needed for the review screen.
- L114 (R197): the two recordings are "record to clip" and "record show", "All else is gone"; "Review" good, asks for a better name; "Macros" not "Dashboard"; one solid list of names for talk, app, menus, manual later.
- L112 (205): "Timeline" is what connects to the layer's playhead and "should be called as such".
- L116 (R199): "we want mapping files" -> R199 f: export/import files do not go.
- L118 (R227): "all good"; "Spacebar is typically tap tempo" -> a note on the spacebar (default to Tap Tempo, or reserved), an edge.
- L120 (207): choice C (no per-entry settings, a pad only presses, a knob only turns) AND he wants an argument for a hold setting for DJs and bands.
- L128 (215 plan all): OSC (R200 c) is planned.
- L92 (195): record over with the MIDI controller (controller never named).
- NOT NAMED: 206, 208 (but L4/L5), 210 (but L55), 214, R148, R198, R200 (but L128).

## FINDINGS
- MUST J-4 (and 207, D33): marked ASK: LINE but its own IF-WRONG says STAGE, nothing of his settles it (he chose c, and the page said an endless knob cannot work without its setting), and it is about what the app does -> by the three tests it is ASK: YES. Also the hidden consequences are wider than endless knobs: c also removes the hit-strength-to-opacity behaviour (207 RULE says "how hard a pad is hit changes nothing") and the paper's NOT DONE admits the controller is unknown. Fix: ASK: YES; TEXT "I assume your controller's knobs have end stops and you do not use hit strength on pads: under c both stop working (an endless knob, and how hard you hit a pad)."
- MUST 214 RULE: "They light the clip cells of the deck on screen on a Launchpad X or a Launchpad Mini MK3 and nothing else" describes the app as it is now (his L1). Fix RULE: "No new work on pad lights in this build. The new buttons (actions, cue buttons, the tempo bar) get no lights until he names a controller." The Launchpad sentence goes to TODAY only.
- MUST R199 (e): leaves out L66 (R129 2): "the slider moves back to the action position based on the global glide back setting". (e) says only "then the action or the signal takes it back" (quarter second). A knob is a hand move; it must glide back at the Global glide-back speed (instant to four seconds, L64). Fix: add "...then the slider moves back to the action's position at the Global glide-back speed (L66, L64). A signal takes it back as topic D/I rule." and remove "takes it back" bare.
- SHOULD R199 (f): "The mapping lives in the show and on the computer" quotes BD 81 ("with show and app") but his later 83 says "they live in the show" and 85 "the show's keys take over; a new show takes its setup from the most recent show". The rule never says what happens when a show is opened or a new one started, which a builder must know. Fix: restate 85 in (f) or list it under CONFLICTS (81 vs 83) and drop "and on the computer".
- SHOULD 209 RULE: option A's parenthetical "(a film, or one that a full disk ended, counts)" is dropped, and A's "a window you opened yourself may answer you there" survives only in J-5. Put both back into the RULE so the four messages are as complete as A.
- SHOULD P34 / J-6: L68's OK is for one message (a paste). Applying "stays until OK" to every message may mean a box that blocks keys while he performs (HDD space, a recording not saved). Split "stays until clicked" from "blocks the keys" and say it never blocks keys/pads; or make J-6 ASK: YES with that one question.
- SHOULD R197 (b): after "All else is gone" the RULE says nothing on the Record tab's and the list's names ("Recordings" in the original reading; the list of recordings is in topic E). State it or add it to J-1's ALT.
- SHOULD R197 (c)/(d): L112 gives the slider source its name ("Timeline ... should be called as such"); the clip mode is also "Timeline mode" (L21). Two things called Timeline is the confusion he complained of. Say so in one line and leave the name of the clip mode to a LINE.
- SHOULD NAMES "Tempo - and Tempo +": his words in BD:914 are "bpm- bpm+" ("BPM minus and BPM plus"), not Harmony's pick. Use "BPM -" and "BPM +" (same rule the paper applies to "Tempo play").
- SHOULD R227: first half of the added sentence ("out of the box the spacebar is mapped to Tap Tempo") is not flagged as an assumption in the RULE itself; only the Review half has "(assumption J-3)". Add the tag to the first half.
- SHOULD R200 STATUS: "CORRECTED part c" - R200 was NOT NAMED, so by the rule it is REPLACED (his L128 elsewhere); also J-10 ("every mappable control also gets an OSC address") fixes the scope of a build item with ASK: NO; make it LINE or defer in the RULE to topic K.
- SHOULD R198 CHANGED says "nothing" but the RULE gains clip cut/paste/Option-drag (L4, L5) and effect re-ordering (L104).
- SHOULD ANSWER 207-hold: ends in a recommendation but no one-line choice for him; add "Say '207 hold' to add it; otherwise c stands." Drop the "mixer's effect lever" simile and "a lost release could stick" (jargon).
- SHOULD J-17 TEXT describes the app as it is now ("the menu entry that only closes the mapping screens"); say "no menu entry closes the mapping screen; Esc or its close button does".
- SHOULD J-11: "the old clip plays on until that cell is fired again" may not be possible if a layer plays what its cell holds (CLAUDE.md rule 15, by cell reference). Not verified; confirm it is buildable or write the safe reading as "the layer keeps its picture until fired".

## MISSING
- What a mapping does at show open / new show (his 85 words in BD). 
- Nothing else of his lines in this topic is dropped: L4, L5, L7, L73, L34, L35, L38, L64, L72, L106, L114, L116, L118, L120, L122, L128 are all carried.

## WHAT I CHECKED AND FOUND RIGHT
206 (status, list, line cites), 207 (status c, quote), 208 (quotes L4/L5 verbatim, J-11 LINE), 210, R148, D32 (L11 timing and L19), D33, D34, D35, P32 (tempo wording from L13/L17/L31/L72), P33, R227 (a), R199 (a)-(d),(f quote),(g)-(j), all quoted L numbers (L4, L5, L11, L55, L65, L68, L72, L80, L114, L116, L118, L120, L122: verbatim), J-1 (ASK YES right), J-2, J-3, J-5, J-7, J-8, J-9, J-12..J-16 levels, the answer "review-name".

## LINT OUTPUT
blocks: 19 ITEM (slice has 19 ids), 17 ASSUME (1 ask YES, 9 LINE), 2 ANSWER, 35 NAME
OK
