# Visual critic (design) -- bf9b
ANSWER: yes. VERDICT: PASS_WITH_NITS (no MUST).
REASON: No strip or tab carries a deck number, dot or mark; strips and tabs match the BEFORE pictures except the intended change (a strip now keeps another deck's clip). Pictures and names are legible and nothing overlaps, apart from the folded row (S7).

## Per-file observations (all looked at)
- B0-deck0-layer0-playing-strip.png vs S1-deck0-shown-strip.png: same frame, same controls (X B S, < || >, S K V faders, F fader, name box, dropdown arrow). Only change: Layer 2 now shows its D2 C2 picture and "D2C2L2" under it. No number, dot or badge anywhere.
- B0-...-tabs.png vs S1-...-tabs.png: "Deck 1".."Deck 20" and "+", identical look. No dot on "Deck 2".
- S3-b-after-remove-tabs.png: "Deck 2" gone, the rest closed up; nothing added.
- S3-b-after-remove-full.png: no button, no sentence about the removal. The toolbar line "D2C2L2.png" is the pre-existing last-fired file name (outside this lane, not disturbing). Preview still shows D1 C1 and D2 C2.
- S7-c-unfolded-deck0-shown-strip.png: three full rows, picture and file name legible, nothing cut off, no mark.
- S7-a-deck0-shown-strip.png: folded Layer 2 row is a thin row with a tiny picture (text "D2 C'" cut off) and no layer name; a clipped X B S fragment shows above Layer 1's own X B S (looks like a double row of buttons). No B-picture of a folded row exists, so I cannot say if this is new.
- S7-a-deck0-shown-tabs.png: third tab "Twenty Char" is cut with no ellipsis (manifest problem 3).

## Findings
- SHOULD: S7-a-deck0-shown-strip.png -- the folded layer row shows a clipped second X B S strip right above Layer 1's real one, and loses its "Layer 2" name. Fix if it is new; if the same before the lane, file as debt.
- NIT: S7-a-deck0-shown-tabs.png -- a long deck name is cut mid-word with no ellipsis.
- NIT: S3-a vs S3-b strip -- the small "layer 2" text inside the thumbnails is redrawn a little differently (manifest problem 1). It also happens before the lane (B0 vs B1-b), so not a regression.
- NIT (outside lane): "D2C2L2.png" text line in the second toolbar row.
