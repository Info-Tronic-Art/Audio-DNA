# Critic bf9b (logic) -- STATUS: DONE
ANSWER: yes. VERDICT: PASS_WITH_NITS.
Every state's grid, strip and tab picture matches the manifest facts, cell by cell. No strip or tab carries a deck-dependent mark.
(shots dir: .harmony/.reports/s-rta-1003/bf9b-shots/)
## Per file
- S1-deck0-shown-grid/strip/tabs: only bottom-left D1 C1 (layer 0, deck 0) lit; Layer 2 row (plays deck 1's clip) not lit; Layer 2 strip shows D2 C2 / D2C2L2; header none lit; 20 plain tabs, Deck 1 shown-green only. MATCH.
- S2-deck1-shown-grid/tabs: D2 C2 in middle row lit; no bottom-row cell lit (layer 0 plays deck 0); no header lit. MATCH.
- S3-a/b grid, tabs, strip, full: before == after in grid (D1 C1 lit only); Deck 2 tab gone, nothing added; strip still D2 C2 (retired clip keeps playing); no button or sentence. MATCH.
- S5-a-deck0: header 3 lit; column 3 lit in Layer 3 and Layer 1 rows, not Layer 2; strips D1C3 / D2C1 / D1C3. MATCH. S5-b-deck1: no header lit; only middle-row D2 C1 lit. MATCH.
- S7-a/b/c: S7-a lit = bottom-left (Thirty Character C...) only; strips Layer 3 D2 C2 (deck 1 col 1), folded middle thin, Layer 1 D1 C1. S7-b (deck 2): nothing lit. S7-c: strips D2C2 / D2C1 / D1C1, same lit cell. MATCH. Tabs: no mark.
- S10-a..d: header 2 lit + column 2 on all 3 rows / none (8-col, nothing fired) / header 7 + column 7 on 3 rows / none (4-col shown, strips D4 C3). MATCH.
- S12-a/b/c: a: Layer 3 row col 2 and Layer 1 row col 1 lit (layers 2 and 0), middle empty; b (five-rows shown): none lit, strip Layer 3 keeps D1 C2, Layers 4/5 empty; c == a. MATCH.
## Findings
- SHOULD: P1-layer-tab-after-remove-deck0-full.png toolbar row 2 reads "Loaded deck: five-rows" (a stale event sentence from the earlier Load Deck). Boris: no text saying what happened. Not in the pictures of other states (they show a file name). Confirm it is pre-existing/out of lane or remove it.
- NIT: S3/S12: the "layer N" text in strip thumbnails re-renders after a rebuild (manifest problem 1); not a logic mismatch.
- Outside the question, already in the manifest: P1 Layer tab broken after Remove Deck (problem 5) is a visible MUST-grade flaw for the Layer tab, not for lit cells.
