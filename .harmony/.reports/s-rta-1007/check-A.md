# CHECK A -- re-check of apply-A.md (Firing clips and the tempo row)
Read-only check. Boris quoted only from boris-msg-numbered.txt (L numbers) and binding-decisions.md (BD lines). Nothing built or run.

## MY OWN READING OF HIS LINES (step 2, written before opening the paper)
- L1: all defaults taken for the NOT NAMED questions (173, 175, 190, 191); readings not named stand; no "today" in what he reads; ask focused questions on assumptions.
- L9: an explanation given once answers the repeats (L11's empty-cell wait, L12's "any clip", L13's "tempo change does not move the 1" apply wherever they fit).
- L8: new functions laid out where they fit; P items about looks are dropped.
- L11 (R168): token "dfad" is not a clear letter list; then: an empty cell triggers on the 1 when the clip playing is in BPM mode, same as a new BPM clip; "a layer that is not in BPM mode ... plays instantly" -- reads two ways (the empty-cell clear, or a clip fired onto such a layer). Changes R168 (b).
- L12 (R139): in pause or run mode a new BPM clip waits for the one, unless Resync is clicked; when stopped, a click on play or on any clip is the new 1. Changes R139 (c)(d), adds "unless resync" to R128.
- L13 (R174): /2, x2, tempo change do not move the 1; pause and play do not move a clip back to the one; pause pauses beat clock, clips and all BPM-controlled things; a press after pause starts the clock from the held position, "nudge is history" (reads two ways: embedded in the position, or wiped). Changes R174 (b)(d). He says "if you don't understand ask me".
- L14 (R176): range 22 to 480 by hand AND for the listening clock (narrow later only if it distorts); /2 x2 work to both limits in manual and on the listening clock; nudge X ms displayed only in automatic mode; circle = one bar, four spots = four beats. Changes R176 (d)(e)(g) (and R215's greying).
- L30 (172): column trigger is the only way to start several clips together; pause-then-drag is a waste.
- L31 (174): paused, empty layer, BPM clip waits for the clock to return to the 1 then fires; stopped = plays the moment triggered and sets a new 1; a playing clip when tempo is paused shows paused and continues from there on play (mode not stated).
- L32 (189): he asks Harmony back; default A not taken; also "where the one is".
- L17/L18/L19 (R206 b c d): Resolume fires start tempo from paused or stopped; "the beat reacts instantly to the tapping on the second tap" (tempo or beat position: ambiguous); column with empty cell empties the layer.
- L72: tempo stop button stops all actions and everything else (lands in R176 a, R214 a).
- L107: he asks Harmony whether strobe/pulse effects holding still is a good idea (R214 effects clause stays open).
- L128: "215 plan all of these" puts Link on the plan (R178).
- L130: D/P items unread: each needs his words or an @@ASSUME.

## FINDINGS
- MUST | R176 (d) against A-10 | The RULE (d) says the listening clock gets the same open range 22 to 480 and is narrowed only later if it distorts (L14: "If having such an open range is distorting the listening clock, then we will change that later"). A-10 TEXT says the opposite: the app by itself picks 60 to 200 and /2 x2 take it to 22 and 480. So A-10 builds the "later" option now, against L14, and files it as a LINE he can strike. Fix: make A-10 TEXT = "I assume the listening clock may land anywhere from 22 to 480 from the first build; it may show half or double tempo at times; narrowing comes only if you see it distort (L14)". ASK: NO (his words settle it). Move "60 to 200 by ear" to ALT.
- MUST | R175 RULE (1) last sentence | "A tempo change, "/2", "x2", pause and play do not put a BPM-mode clip out of time, because none of them moves the 1 (L13)" is an assumption written as fact and no @@ASSUME names it. L13 says only that they do not move the 1; the page's 123 b / 71 b say a clip falls out of time "after a tempo change" and is cut once on the next 1, and L13 does not undo that. STATUS says CHANGED: nothing yet a sentence is added. Fix: delete the sentence, or add an @@ASSUME (ASK LINE) "a clip already playing when the tempo changes is cut once on the next 1, as before" and set STATUS CORRECTED.
- SHOULD | A-1 (ASK: YES) | Words of his largely settle it: L13 "if ... user triggers clicking on the clip or presses play ... that press starts the clock from when the user clicks play or the clip", plus L17 and L31 ("waits for the bpm clock to return to the 1"). The doubt left is narrow: does a clip fired in a pause also show its first frame meanwhile (BD:1091 "will not play but will still display")? Fix: make A-1 a LINE for the resume, and put the display question in TEXT/ALT: "b) the clip stands on its first frame until play". Today ALT b does not say what the layer shows.
- SHOULD | R176 (e) RULE | Drops "a number can be typed" and "from -500 to +500", which stand (he did not change them), and moves them into A-9 only. A builder reading the RULE loses them. Fix: add to (e): "In automatic mode a nudge number can be typed and runs from -500 to +500 (assumption A-9)."
- SHOULD | R168 CHANGED | L11 opens with the token "dfad", which is not a clear letter list for (a)-(d). The paper does not mention it. Fix: add to CHANGED "L11 opens with 'dfad'; read as a slip, not as lettered parts (INFERRED); the sentence after it changes (b)".
- SHOULD | R205 STATUS | Says STANDS but the moment of clearing changes (from "at once" to "on the next 1 when the clip playing is in BPM mode") on the strength of L11 by L9, an inference. Fix: STATUS "CORRECTED (the moment)"; add to A-6 or a new LINE assumption: "a column click clears layers with an empty cell on the 1 when the clip there is in BPM mode". Same for the empty-cell sentence in 191.
- SHOULD | A-8 (ASK: NO) | L13 "that nudge is history" can mean the nudge amount is wiped at a start; this pulls against his 129 b (stop and play never touch it) and is a conflict between two statements of his, filed only under NOT DONE. IF-WRONG says "a few milliseconds" but the nudge runs to +/-500 ms and he wrote "If you don't understand ask me". Fix: add a CONFLICTS bullet and make A-8 ASK LINE with IF-WRONG "up to half a second".
- SHOULD | R140 RULE | "load up the clips is filling the clip grid; not firing clips so that they stand ready" is option B's second sentence, not his word (L30 is silent). Fix: label it INFERRED or add to A-16.
- SHOULD | D4 | The page said "within a tenth of a beat of the 1" (both sides); the paper says only "after the 1" without flagging the narrowing. Fix: restore "of" or note the change in A-15.
- SHOULD | R214 RULE end | Quotes a clause ("effects that read the beat by themselves (a strobe, a pulse) hold still too") that is not verbatim from the page (it said "sliders and effects driven by the beat"). Fix: quote the page or paraphrase without quote marks.
- SHOULD | R216 RULE | "L72 is read as being about what plays and runs on the beat, not about a held picture (INFERRED)" is an assumption inside a RULE; L72 says "everything else". Fix: add an @@ASSUME (LINE): "stop does not switch off echo/freeze effects".
- SHOULD | NAME blocks (BPM mode, automatic mode) | SOURCE says "the on-screen name now is 'BPM Sync'" and "the on-screen control now is a tick box 'Manual'"; these lists go to him (L114) and L1 forbids "today". Fix: drop the "now" clauses (keep them in TODAY lines).
- SHOULD | P2 RULE | Drops the function "the word that says whether the app has found the tempo" (a state display, not a look). Fix: add it to "What stays as function" as an INFERRED line.
- SHOULD | A-6 (ASK: NO) | During a pause a click on an empty cell does not resume the clock while a clip click does (A-1): a stage-visible asymmetry (the click seems to do nothing until play). Fix: ASK LINE, or state the alternative in the RULE.
- SHOULD | R128/191 | Cite L11 as his word for "a clip that is not in BPM mode starts on the click", but L11's sentence names a layer and is read two ways (A-4). Fix: also cite BD:1081 and say "see A-4".
- SHOULD | D5 | SETTLED says "everywhere"; L14 settles the circle only. Fix: "in the circle and the "1""; other places stay Harmony's.

## MISSING
- No @@ASSUME on what a layer shows for a clip fired during a pause if the clock stays paused (A-1 b) except in NOT DONE.
- The "dfad" token of L11.
- R214 (d) / item 175: whether the tempo number keeps following the music while stopped depends on whether a hand correction stays in automatic mode (189); not marked as waiting on 189.
- R176 (a)-(f) RULE does not say it waits on 189 (only the CHANGED of 189 lists it).
- R177 in manual mode: what happens to a nudge across show re-open (nothing saved) is implied, not written.

## WHAT I CHECKED AND FOUND RIGHT (ids only)
Every L-number quote in the paper matches boris-msg-numbered.txt (checked by script). Statuses/readings of: 172, 173, 174, 175, 189, 190, 191, R128, R139, R142, R163, R164, R169, R174 (b), R176 (g), R177, R178, R206, D1, D2, D3, D6, P1, A-2 (YES is right: L18 reads both ways), A-3, A-5, A-7, A-11 to A-17, NAMES, CONFLICTS bullets 1-3. No RULE line describes the app as it is now.

## LINT OUTPUT
blocks: 33 ITEM (slice has 33 ids), 17 ASSUME (2 ask YES, 8 LINE), 0 ANSWER, 15 NAME
OK
