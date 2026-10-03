# Critic bf9b F5 (UX) -- STATUS: DONE -- VERDICT: PASS (answer yes)
Question: after Remove Deck with Layer 3 selected + Layer tab open, is the tab whole, strip highlighted, no removal button/sentence?
- F5-P1a (empty deck removed): inspector shows title "Layer 3", 8 dashboard knobs with values/Link names/Manual buttons, then Autopilot, Layer, Video, Transition sections (F5-P1a-...-inspector.png). Layer 3 name box has cyan highlight (-strip.png). Deck tabs: deck1, deck2, + (-tabs.png).
- F5-P1b (played deck removed): inspector identical and complete; highlight kept on Layer 3; tabs "Deck 1", "deck2" (-tabs.png). Strips still show D1 C2 / D1 C1 thumbnails (-strip.png) -- consistent with layers playing on.
- F5-P1c (undo): inspector + highlight unchanged; tabs back to Deck 1 / deck1 / deck2 (-tabs.png, -full.png).
- BEFORE (P1-layer-tab-after-remove-deck0-inspector.png): title "Layer 3" over cut-off knobs (values clipped), "No layer selected" printed across knobs, empty panel below. All of that is gone in F5.
- No button or sentence about the removal in any full frame (F5-P1a/P1b/P1c-...-full.png); only the standing "ROUTINES - Save one in the Record tab" hint.
MUST: none.
SHOULD: none.
NIT: in F5-P1b/P1c-full.png the Preview "D1 C2" glyphs look slightly overprinted on the "2" (pre-existing test-pattern look, unrelated to this fix). Tab names "Deck 1" vs "deck1" differ in case (fixture naming).
