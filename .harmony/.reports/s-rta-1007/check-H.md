# check-H: blind re-check of apply-H.md (topic H, How a clip plays)
Written by the checker; read-only on everything else. Verdict: SOUND_WITH_CORRECTIONS.

## MY OWN READING OF HIS LINES (step 2, written before the paper was opened)
- L98 (R217): Eject clears the layer; if autopilot brings the next clip, the eject is cancelled by it. Only the AUTOPILOT case is his; "clip's own next choice" is not his word.
- L99 (R218): "do beats here. It was my mistake before": the row unit goes back to BEATS; his bars sentence is withdrawn for this row. That touches R218 (b) too (its "bars in multiples of 4").
- L101 (201): markers on a clip are bpm lines; Random lands on one of them. L20 (E): moves every beat to random beat markers. L21 F: timeline "divided into beat markers". Reads as option B; conflict with "Timeline only shows bars" (BD 825) is real and must be asked or struck.
- L102 (202): both little menus modelled after Resolume: the start menu is built (option B), "42 default" replaced. The three entries mean what RESOLUME's manual says (facts-resolume-transport.md:54).
- L21 F: no duration on a BPM clip, only beats; default 8 or 16 beats so it plays at about speed 1 of timeline mode; +/- one beat; /2 x2. Timeline mode: Duration and Speed are separate, neither changes the other. The 0..16 list belongs to a +/- doubling row (Speed) = my inference, to be flagged.
- L29 (H): bypassed/solo-hidden clip plays out of sight: settles R194 d.
- L111: keying and K slider removed (topic I decides Blend). L19 (D): empty cell in column triggers empties layer (R205). L7/L73: ignore-actions toggle on layer (P28).
- L11-L13, L17, L31: BPM clip waits for the 1, stopped = new 1, pause/play never move the 1; fire during pause restarts the clock (topic A). L13 "Pause pauses the beat clock, the clips and everything it controls with BPM".
- L8: P28/P29 look-only variants are dropped. L3: G1 codec, researcher answers.

## FINDINGS
- MUST R218 (b) / H-5: RULE (b) keeps "a whole number of BARS in groups of 4 (16, 32, 48 ... beats)" and says "he did not change it", but L99 ("do beats here. It was my mistake before") withdraws bars for this row, and the paper's own H-5 WHY says so. Fix RULE (b): "A new BPM clip's first count is a multiple of 4 BEATS (BD 812 'always work with multiples of 4'), the one nearest normal speed (L21: 'sometimes 8, sometimes 16'); an uneven file has its out point moved in" + @@ASSUME naming it; H-5 default = that, not 16/32/48.
- MUST R193 (e) / NAME Speed: the nine-step list "0, 1/8 ... 16" (L21) is written as his rule FOR THE SPEED ROW; L21 never says "Speed" for it (the next sentence is the Beats row). Inference without @@ASSUME. Add @@ASSUME (ASK LINE): list belongs to Speed in BPM mode; "+"/"-" step along it; what 0 does (stand still?); whether /2 x2 also exist on Speed; how Speed multiplies Beats.
- MUST 202 RULE entry 3: "carry on as if it had never stopped" is the page's wording; the Resolume manual (facts-resolume-transport.md:54) says relative pick-up "will start the clip at the same relative position the previously played clip was at". L102 says "after resolume". Fix: entry 3 = "start at the same relative position the clip that played before it had reached"; reword H-4 accordingly.
- MUST 201 RULE: "while the tempo row is paused or stopped it does not jump" and "Interval 1 ... is what he saw" are decisions/claims not in his words (L20 says only "every single beat") and no @@ASSUME names them. Add H-2 text: "Random stands still while the beat is paused or stopped" and drop "which is what he saw".
- SHOULD H-8 (ASK YES): 136 b (BD 1009: "only BPM-synced clips hold with it; a clip that is not BPM-synced plays on") and L13 ("the clips and everything that it controls with BPM") already settle BPM-only; L9 says use it. Same point is A-3 (asked once there). Make H-8 LINE or delete; CONFLICTS bullet 3 is too strong.
- SHOULD H-4 (ASK NO): Stop forgets every clip's place, so "Continue" never continues after Stop: seen on stage, about what the app does. Make it LINE.
- SHOULD 202 RULE: "(page's option B; Resolume's own default)" is fine (manual: from the start is the default, verified) but the NAMES entries Restart/Continue/Relative are page words; keep the manual's three names until his list.
- SHOULD R217 RULE: "(the layer's, or the clip's own choice of what comes next)": L98 names only autopilot; the clip-own-next part is R203 b, not his word. Say so (INFERRED) or cut.
- SHOULD HIS fields: R194 lists only L29 but RULE uses L111, L19; R219 says "none" but cites L30; R193 omits L12/L31 although (b) rests on them.
- SHOULD "today" wording in RULE lines (L1): R194 (e) "no longer promises", P28 "leave the strip". Say what the app does, not what stops.
- SHOULD G1: answer-codec.md was written 22:48, after the paper (22:41). File G1 as ANSWERED: no codec of our own; one new question "Convert for performance (HAP)", default yes; HAP makes jumps/backwards/Random instant (touches H).
- SHOULD H-1 stays ASK YES by the conflict rule, but state in the question that L21 and L20 (later words) already say "beat markers", so the default is B and he strikes it if not.

## MISSING
- L17 (B, paused or stopped: triggering a clip activates tempo play) and L31 (a playing BPM clip shows its paused frame on tempo pause and resumes from there) are not cited in H; R193 (b) "fired during a pause they show and move" should cite them and point to topic A (A-3).
- Edge: what Beats does to the dropped-on playhead / Beat Repeat when Beats is not a multiple of 4 is only in NOT DONE; fine for a builder note.
- L21 "decent amount of beats ... so that it plays at about the same speed as speed 1": the rule for choosing 8 versus 16 (from file length and tempo) is not stated as an algorithm; H-5/H-10 only partly cover.

## WHAT I CHECKED AND FOUND RIGHT
Quotes of L20, L21, L29, L98, L99, L101, L102, L111, L19, L7, L30, L13, L36 all verbatim with right L numbers; BD 761, 812-813, 825-826, 846, 1001, 1009 quotes verbatim. 201 (status, option B fit), 202 (option B, 42-default replaced), R192, R217 core, R218 (a), R193 (a)(c)(d)(f)(g)(h), R194 (a)-(d)(f), R219, P28, P29 (Cuepoints keep name: R179 d verified), H-2, H-3, H-6, H-7, H-9, H-10, 11 ITEM blocks complete. Stop rewinds and ejects: facts-resolume-emulate.md C3.4 verified.

## LINT OUTPUT
blocks: 11 ITEM (slice has 11 ids), 10 ASSUME (3 ask YES, 3 LINE), 0 ANSWER, 15 NAME
OK
