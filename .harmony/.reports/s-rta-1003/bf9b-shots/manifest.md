# bf9b visual gate (B7) -- capture manifest

Captured 2026-10-03 17:22-17:54 EDT by the B7 capture driver (scratch: `scratchpad/b7/`: `common.sh`, `lib.py`, `drive.py`, `run.sh`, `run2.sh`, `fixtures.py`).
Method: REST only (port 7070, `Connection: close`), composition / deck files, the test-only lever `ADNA_INSPECT_LAYER`. No synthetic input, no Output window, window captures by the Quartz window id of the pid this driver started (`screencapture -x -o -l <id>`), every PNG decoded and looked at.
Window: 1728 x 1079 pt at 2x = 3456 x 2158 px in every capture. `*-full.png` = the whole window at half size. Crops are full resolution, window pixels: `strip` (0,354)-(505,1020), `grid` (505,354)-(3456,1020), `tabs` (0,1020)-(3456,1072), `topbar` (0,56)-(3456,134), `inspector` (1733,1080)-(2592,2140).
Indices in this file are 0-based as REST uses them (layer 1 = the row named "Layer 2" = the middle row; the grid draws Layer 3 at the top). "d" = mean |a - b| over RGB of the `strip` crop, 0..255. The noise floor measured four times (two captures 0.5 s apart, same state) is 0.0000 each time.

Labels: VERIFIED = read from REST or seen in the decoded PNG. INFERRED = my reading of the source, not run.

## S1 -- 20-deck check show, deck 0 shown, layer 1 plays deck 1's clip -- VALID yes
- Did: `load_composition bf9b-check.json`; `trigger_clip {layer 0, column 0}` (deck 0 shown); `switch_deck 1`; `trigger_clip {layer 1, column 1}`; `switch_deck 0`.
- Model facts: shown 0; numDecks 20; retiredDeckCount 0; layer 0 activeClip {deckId 100 (deck 0), column 0, retired false}; layer 1 {deckId 101 (deck 1), column 1, retired false}; layer 2 nothing; selected layer: none (inspected_layer "", inspector_tab "Clip" -- no lever in this launch).
- Seen: no lit cell in the Layer 2 row; Layer 1 row's "D1 C1" cell lit; the Layer 2 strip shows the D2 C2 picture; no number / dot / mark on a strip or tab.
- Files: `S1-deck0-shown-{full,strip,tabs,grid,topbar}.png`, `S1b-deck0-shown-again-*` (floor pair).
- Numeric: floor d(S1, S1b) = 0.0000.

## S2 -- the same, deck 1 shown -- VALID yes
- Did: `switch_deck 1`.
- Model facts: shown 1; layers as S1.
- Seen: the "D2 C2" cell in the Layer 2 row lit; no header lit.
- Files: `S2-deck1-shown-*`.
- Numeric: d(strip S1, S2) = 0.0000 (0 pixels differ) against floor 0.0000 -- PASS.

## S3 -- remove the deck whose clip layer 1 plays -- VALID yes
- Did: `switch_deck 0`; capture; `POST /api/debug/remove_deck {deck: 1}`; capture.
- Model facts before: shown 0, numDecks 20, retiredDeckCount 0, layer 1 {deckId 101, column 1, retired false}. After: shown 0, numDecks 19, retiredDeckCount 1, layer 1 {deck -1, deckId 101, column 1, retired TRUE}, layer 0 unchanged. deck_tabs undo top "Remove Deck".
- `/api/debug/ui_text` after, verbatim: `{"ok": true, "file_label": "D2C2L2.png", "audio_notice": "", "inspected_layer": "", "inspected_clip": "", "inspector_tab": "Clip"}`. file_label before: "D2C2L2.png" -- unchanged.
- Seen: the "Deck 2" tab is gone (Deck 1, Deck 3, ...); the Preview still shows D2 C2; no button, no sentence.
- Files: `S3-a-before-remove-*`, `S3-b-after-remove-*`.
- Numeric: d(strip before, after) = 0.7240 against floor 0.0000 -- NOT within the floor. 4875 pixels differ, all inside the two thumbnail squares (mean 2.39 there, 0.0000 everywhere else in the strip column). See problem 1.

## S4 -- top bar -- VALID yes
- Files: `S4-topbar.png` (= `S1-deck0-shown-topbar.png`), BEFORE `B4-topbar.png`.
- Seen: no "Fade:" label, slider or value. Where it was there is now an empty stretch of about 185 pt between "Quantize: Off" and "Master Signal:" (the right-hand group did not move). See problem 4.

## S5 -- column fire with an ignoring layer -- VALID yes
- Did: `load_composition s5.json` (the check show with layer 1 `ignoreColumnTrigger: true`); `switch_deck 1`; `trigger_clip {layer 1, column 0}`; `switch_deck 0`; `trigger_column {column: 2}` (the header labelled "3"); capture; `switch_deck 1`; capture.
- Model facts: numDecks 20; layer 0 {deckId 100, column 2}; layer 1 {deckId 101, column 0, ignoreColumnTrigger true}; layer 2 {deckId 100, column 2}; none retired; shown 0 then 1.
- Seen, deck 0: header "3" lit; the cells of column 3 lit in the Layer 3 and Layer 1 rows, NOT in the Layer 2 row; the Layer 2 strip keeps D2 C1. Deck 1: no header lit; the Layer 2 row's "D2 C1" cell lit.
- Files: `S5-a-deck0-shown-*`, `S5-b-deck1-shown-*`.
- Note: "column 3" was taken as the header labelled 3 (REST column 2).

## S6 -- the Layer tab through the lever -- VALID yes
- Did: launch with `open -g --env ADNA_INSPECT_LAYER=2 ... --args --test-mode`; nothing else (the default show: 1 deck, 3 layers, 12 columns).
- Model facts: inspected_layer "Layer 3", inspector_tab "Layer"; numDecks 1; nothing playing.
- Seen: no "Persistent" anywhere; "Ignore Column Trigger" alone on its row under "Master"; the Layer 3 name box has the cyan highlight.
- Files: `S6-layer-tab-default-show-{full,strip,inspector,topbar}.png`.

## S7 -- 20 decks, 30-character clip names, a 20-character deck name, a folded layer -- VALID yes (fold driven by the file)
- Did: `load_composition s7.json` (check show; clip names "Thirty Character Clip Name 012" / "...01B" / "...01C" on deck 0 row 0 col 0, deck 1 row 1 col 0, deck 1 row 2 col 1; deck 2 named "Twenty Char Deck Nam"; layer 1 `folded: true`); `switch_deck 1`; `trigger_clip {1,0}`, `{2,1}`; `switch_deck 0`; `trigger_clip {0,0}`; capture; `switch_deck 2`; capture. Then the same file with `folded: false` and the same steps (`S7-c`).
- Model facts: numDecks 20; layer 0 {deckId 100, column 0}; layer 1 {deckId 101, column 0} (folded, plays another deck's clip); layer 2 {deckId 101, column 1}; none retired; shown 0 / 2 / 0.
- Seen: no number, no dot on any strip or tab. The folded row is a thin row with a tiny D2 C1 picture. The grid cell shows "Thirty Character C..." (ellipsis). See problems 2 and 3 for the strip name and the tab label.
- Files: `S7-a-deck0-shown-*`, `S7-b-deck2-shown-*`, `S7-c-unfolded-deck0-shown-*`.

## S10 -- 4-column deck <-> 8-column deck -- VALID yes, on the default 3-layer show (not the check show)
- Why not the check show: `load_composition` unbinds the Layer tab (VERIFIED: inspected_layer "" after it), and the lever only acts at launch. So the decks were loaded as DECK files into the default show.
- Did (launch with the lever = 2): load_deck deck1.json, deck2.json, earlier S12 steps (first lever launch: remove_deck 0, five-rows load + undo), `load_deck eight-cols.json` (3 rows x 8 columns), `switch_deck 0`, `trigger_column 1`; capture a + a2 (floor); `switch_deck 2`; capture b; `trigger_column 6`; capture c; `switch_deck 0`; capture d.
- Model facts: numDecks 3; inspected_layer "Layer 3", tab "Layer" in all five. a/b: every layer {deckId 100, column 1}. c/d: every layer {deckId 103 (the 8-column deck), column 6}. None retired.
- Seen: the Layer 3 highlight is kept in all five. a: header "2" lit on the 4-column deck. b: 8 headers, none lit, no cell lit. c: header "7" lit, column 7 cells lit. d: 4 headers, none lit, no cell lit; strips show D4 C3.
- Files: `S10-a-4col-shown-*`, `S10-a2-4col-shown-again-*`, `S10-b-8col-shown-*`, `S10-c-8col-col7-fired-*`, `S10-d-4col-shown-plays-8col-*`.
- Numeric: floor d(a, a2) = 0.0000. d(a, b) = 1.1973; d(c, d) = 1.0289 -- NOT within the floor. All differing pixels are inside the three thumbnail squares (mean 3.95 / 3.40 there, 0.0000 outside). See problem 1.

## S12 -- Load Deck of five-rows.json with a layer selected, then Undo -- VALID yes, on the default 3-layer show (not bf9b-check.json)
- Why not bf9b-check.json: as S10. The show here is the default 3 layers + the check show's decks 1 and 2 loaded as deck files (tabs "Deck 1" (empty default), "deck1", "deck2").
- "layer 2 selected" was taken as REST index 2 = "Layer 3".
- Did (second lever launch, lever = 2): `load_deck deck1.json`, `deck2.json`; `switch_deck 0`; `switch_deck 1`; `trigger_clip {0,0}`, `{2,1}`; capture a; `load_deck five-rows.json`; capture b; `POST /api/debug/undo {redo:false}`; capture c + c2 (floor).
- Model facts: a: shown 1, numDecks 3, 3 layers, layer 0 {deckId 100, col 0}, layer 2 {deckId 100, col 1}, inspected_layer "Layer 3", tab "Layer". b: shown 3, numDecks 4, 5 layers (Layer 4, Layer 5 added, playing nothing), layers 0 and 2 unchanged, inspected_layer "Layer 3". c: shown 1, numDecks 3, 3 layers, inspected_layer "Layer 3", tab "Layer". inspected_layer stayed "Layer 3" after every step of this launch.
- Seen: b: rows Layer 5, Layer 4, Layer 3 visible (Layer 2 and Layer 1 are below the fold of the grid area), the Layer 3 strip keeps its highlight and its D1 C2 picture, the Layer tab shows Layer 3. c: the "five-rows" tab and the two rows are gone, the Layer tab still shows Layer 3.
- Files: `S12-a-before-load-*`, `S12-b-five-rows-loaded-*`, `S12-c-after-undo-*`, `S12-c2-after-undo-again-*`.
- Numeric: floor d(c, c2) = 0.0000. d(strip a, c) = 0.7784 -- NOT within the floor (thumbnail squares only, problem 1). d(inspector a, c) = 0.0000; d(inspector a, b) = 0.0000.

## P1 -- extra evidence: the Layer tab after Remove Deck -- problem 5
- Did (same launch as S12, after capture c2): `POST /api/debug/remove_deck {deck: 0}` -- the EMPTY default deck, not shown, nothing plays from it; capture; `undo`; capture.
- Model facts: after the removal inspected_layer "" (tab still "Layer"); after the undo "Layer 3" again.
- Files: `P1-layer-tab-after-remove-deck0-{full,strip,tabs,inspector}.png`, `P1-layer-tab-after-undo-of-remove-*`.

## BEFORE (the pre-merge app)
- B0 -- VALID yes. Did: `load_composition pre-old.json` (the same 20 decks and pictures in the old per-deck-layer format); `trigger_clip {0,0}`. Facts: shown 0, numDecks 20, deck 0 activeClipColumn [0,-1,-1]. Files `B0-deck0-layer0-playing-*`, `B0b-again-*`. Floor d = 0.0000.
- B4 -- VALID yes. `B4-topbar.png`: "Fade:" + slider + "0.30" between Quantize and Master Signal.
- B1 -- VALID yes. Did: `switch_deck 1`; `trigger_clip {1,1}`; capture a; `switch_deck 0`; capture b. Facts: deck 0 activeClipColumn [0,-1,-1], deck 1 [-1,1,-1] (per-deck layers). Seen: with deck 1 shown the Layer 1 strip is empty and the Preview shows only D2 C2; back on deck 0 the Layer 2 strip is empty and the Preview shows only D1 C1 -- the clips disappear on a deck switch. Files `B1-a-deck1-shown-*`, `B1-b-deck0-shown-*`. d(strip B0, B1-a) = 9.3971; d(B1-a, B1-b) = 9.3962; d(B0, B1-b) = 0.3768 (same state as B0 after a round trip: thumbnail squares only -- problem 1 exists before the lane).
- The pre-merge app has no `inspected_layer` / `inspector_tab` fields and no top-level `layers`; those facts are not available there.

## NOT REACHED
- S10 and S12 on `bf9b-check.json` itself with a lever-selected layer: `load_composition` unbinds the Layer tab and nothing but the launch-time lever selects a layer without input. Both were captured on the default 3-layer show with the same deck files instead.
- A 30-character name on a STRIP: the strip does not draw the clip's name (problem 2), so no strip shows the long name.

## PROBLEMS seen on screen
1. Strip pictures are redrawn slightly differently after a grid rebuild. After Remove Deck (S3), a 4 <-> 8 column switch (S10) and Load Deck + Undo (S12) the thumbnail squares differ from before (d 0.72-1.20 against a floor of 0.0000; "layer 2" text in the thumbnail visibly re-rendered); everything else in the strip column is pixel-identical. A plain switch between two 4-column decks (S1 -> S2) is exactly 0. The pre-merge app does the same after a round trip (0.3768). Files: `S3-a-before-remove-strip.png` vs `S3-b-after-remove-strip.png`.
2. The strip name is the media FILE's name, not the clip's name: the strip says "D2C2L2" where the grid cell says "D2 C2", and "D1C1L1" where the clip is named "Thirty Character Clip Name 012". Same on the pre-merge app. Files: `S1-deck0-shown-strip.png`, `S7-c-unfolded-deck0-shown-strip.png`, `B0-deck0-layer0-playing-strip.png`.
3. The 20-character deck name is cut to "Twenty Char" on its tab with no ellipsis. File: `S7-a-deck0-shown-tabs.png`.
4. The top bar has an empty stretch (about 185 pt) where "Fade:" was; the right-hand group kept its place. File: `S4-topbar.png` vs `B4-topbar.png`.
5. Remove Deck -- of any deck, here an empty one nobody plays from -- breaks the open Layer tab: the title still says "Layer 3", the dashboard knobs are drawn cut off, and "No layer selected" is printed over them; the strip keeps its highlight. Undo brings the tab back. VERIFIED on screen and by inspected_layer "". INFERRED cause (source read, not run): `MainComponent::removeDeck` nulls both inspectors "because the erased deck's Layer objects die", which is no longer true with shared layers. File: `P1-layer-tab-after-remove-deck0-inspector.png`.
6. Decks loaded from a file get the FILE's name on the tab ("deck1", "five-rows", "eight-cols"), not the "name" inside the file ("Deck 1", "Five Rows", "Eight Columns"). File: `S12-b-five-rows-loaded-tabs.png`.

## WHAT A CRITIC SHOULD LOOK AT in each file
- `S1-deck0-shown-grid.png`: the middle row (Layer 2) has no lit cell; only "D1 C1" bottom-left is lit.
- `S1-deck0-shown-strip.png`: the middle strip shows the D2 C2 picture while deck 1 is not shown; no number, dot or mark; the name under it reads "D2C2L2".
- `S1-deck0-shown-tabs.png`: 20 plain tabs, no mark on "Deck 2".
- `S2-deck1-shown-grid.png`: "D2 C2" in the middle row is lit.
- `S3-a-before-remove-tabs.png` vs `S3-b-after-remove-tabs.png`: "Deck 2" gone, nothing else added.
- `S3-b-after-remove-full.png`: the Preview still shows D2 C2; no button or sentence anywhere; the second toolbar row still reads "D2C2L2.png".
- `S3-a-before-remove-strip.png` vs `S3-b-after-remove-strip.png`: the small "layer 2" / "layer 1" text inside the thumbnails changes shape.
- `S4-topbar.png` vs `B4-topbar.png`: Fade gone; the empty stretch between Quantize and Master Signal.
- `S5-a-deck0-shown-grid.png`: header 3 lit; column 3 lit in the top and bottom rows, not the middle one.
- `S5-b-deck1-shown-grid.png`: header 3 not lit; "D2 C1" in the middle row lit.
- `S6-layer-tab-default-show-inspector.png`: the "Layer" section: Master, then "Ignore Column Trigger" alone; no "Persistent".
- `S7-a-deck0-shown-strip.png`: the folded middle row (tiny picture, no name); no number or dot.
- `S7-a-deck0-shown-tabs.png`: "Twenty Char" cut off on the third tab.
- `S7-a-deck0-shown-grid.png`: "Thirty Character C..." under the lit cell.
- `S7-c-unfolded-deck0-shown-strip.png`: file names, not the long clip names, under the pictures.
- `S10-a-4col-shown-grid.png` / `S10-b-8col-shown-grid.png` / `S10-c-8col-col7-fired-grid.png` / `S10-d-4col-shown-plays-8col-grid.png`: header 2 lit / none lit / header 7 lit / none lit; the 8 small routine slots above never change.
- `S10-a-4col-shown-strip.png` vs `S10-b-8col-shown-strip.png`: Layer 3 highlight kept; thumbnail text re-rendered.
- `S12-a-before-load-*`, `S12-b-five-rows-loaded-*`, `S12-c-after-undo-*`: strip: Layer 5 / 4 / 3 rows in b, Layer 3 highlighted in all; inspector: "Layer 3" in all three; tabs: "five-rows" present only in b.
- `P1-layer-tab-after-remove-deck0-inspector.png`: the broken Layer tab.
- `B1-a-deck1-shown-full.png`, `B1-b-deck0-shown-full.png`: one strip empty and half the Preview gone after each switch.

## Apps, pids, counts
- AFTER: `/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/bf9b/build-lane/AudioDNA_artefacts/Release/Audio-DNA.app` (lane head 4137f60).
- BEFORE: `/private/tmp/claude-501/-Users-boriskarpman-projects-RealTimeAudio/a868a537-8200-4641-a63f-f3029b3cf7ee/scratchpad/apps/pre-bf9b.app`.
- Started and quit by this driver (each quit by the lock helper's `quit_app`, "app running after quit: no"):
  - 54553 lane app, lever 2 -- exploration (geometry, routes; no deliverable file)
  - 55635 lane app, lever 2 -- S6, S10
  - 58053 lane app, lever 2 -- S12, P1
  - 58572 lane app -- S1, S2, S3, S4, S5, S7
  - 58690 pre-merge app -- B0, B1, B4
- Lock: taken four times as `harmony-b7`, released each time; waited 11 min and 24 min for `harmony-bf9b`.
- Our window each time: one on-screen window, 1728 x 1079, named "Audio-DNA". Output-named windows: 0 at every check.
- UserNotificationCenter windows 16 s after each last quit: 0, 0, 0, 0. No system dialog, no permission prompt.
- An Audio-DNA seen running 16 s after two of the runs (pids 55777, 58800) was started by another lock owner after this driver released the lock; it was not touched.

## F5 -- the Layer tab after Remove Deck, at the fix (lane stage FIX-5) -- VALID yes
Captured 2026-10-03 18:11 EDT by `scratchpad/bf9b-fix-FIX-5/f5drive.py` + `f5run.sh` (the B7 driver's `lib.py`, `common.sh`, fixtures and crop boxes, reused). Same method as above: REST only, the lever `ADNA_INSPECT_LAYER=2`, window capture by the Quartz id of the pid the run started, no synthetic input, no Output window. App: the lane app at head 4811c8e (src = 9ad9e90), binary sha256 7239a7f2616a516e, pid 70754. Window 3456 x 2158 px in every capture. Each state has `-full.png` (half size), `-inspector.png`, `-strip.png`, `-tabs.png` (full-resolution crops, the boxes of this file's header).
- The show: the default 3-layer show (lever: REST layer index 2 = "Layer 3" selected, Layer tab open); `load_deck deck1.json`, `deck2.json` (tabs "Deck 1" (empty default), "deck1", "deck2"); `switch_deck 1`; `trigger_clip {2,1}`, `{0,0}` -- Layer 3 and Layer 1 play from "deck1" (deckId 100).
- `F5-P0-before-*`: that state. ui_text: inspected_layer "Layer 3", inspector_tab "Layer".
- P1a `F5-P1a-after-remove-empty-deck-*`: `POST /api/debug/remove_deck {deck: 0}` -- the EMPTY default deck, not shown, nothing plays from it. ui_text after: inspected_layer "Layer 3", inspector_tab "Layer"; numDecks 2. Then undo / redo / undo: "Layer 3" each time. Seen: the Layer tab is whole (title "Layer 3", dashboard knobs with values, link names and Manual buttons, every section below); the Layer 3 name box keeps its cyan highlight; the "Deck 1" tab is gone.
- P1b `F5-P1b-after-remove-played-deck-*`: `switch_deck 2`; `POST /api/debug/remove_deck {deck: 1}` -- the deck Layer 3 and Layer 1 play from, not shown. After: inspected_layer "Layer 3"; numDecks 2, retiredDeckCount 1, layers 0 and 2 {deckId 100, retired true}. Seen: the Layer tab whole on Layer 3; the highlight kept; the "deck1" tab gone; the strips and the Preview still show D1 C2 / D1 C1.
- P1c `F5-P1c-after-undo-*`: `POST /api/debug/undo {redo:false}`. After: inspected_layer "Layer 3"; numDecks 3, retiredDeckCount 0. Seen: the "deck1" tab is back; the Layer tab and the highlight unchanged.
- P1d `F5-P1d-after-remove-shown-played-deck-*` (extra): `switch_deck 1`; `remove_deck {deck: 1}` -- the SHOWN deck, the one the layers play from (what the Deck menu's Remove Deck does). After: inspected_layer "Layer 3". Seen: "deck2" is the deck on screen, the Layer tab whole on Layer 3, the highlight kept. P1e `F5-P1e-after-undo-of-shown-*`: undo; inspected_layer "Layer 3".
- P2 `F5-P2-layer-tab-no-layer-*`: `POST /api/load_composition bf9b-check.json` with the Layer tab open. After: inspected_layer "", inspector_tab "Layer", numDecks 20. Seen: the Layer tab is an empty dark panel with the words "No layer selected" -- no title, no knobs; no strip is highlighted. (The toolbar's second row reads "Loaded: bf9b-check".)
- The same ten ui_text rows on the app BEFORE the fix (4137f60, sha256 87a10c736d13d228, pid 65770, no captures): P1a "" , P1b "", P1d "" and -- after removing the SHOWN deck -- the undo left "" too (the selected row had been dropped); 4 FAIL of 10. At the fix: 0 FAIL of 10.
- After the run: `audio-dna windows 0, Output-named 0`, UserNotificationCenter windows 0 (16 s after the quit), lock released.
