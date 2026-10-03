# bf9b merge-in lane (s-rta-1003)

## Stage M1 -- merge-in + R-S3 (rulings H-1, R-S3)

STATUS: DONE

Builder started 13:37:43, done 13:52 (2026-10-03, times from `date`). No live app in this stage (none launched, no
lock taken). Scratch (logs, scripts): `<session scratchpad>/bf9b-merge-M1/`.
Pre-merge: lane/bf9b a7491d4 (tree clean), main 5abdf01b3f4dcb5435b95a2a264711628d77fa39, merge-base 11820fa.
Head after M1: the commit that adds this report (see `git log`); src / tests head = 98d71fe (the merge commit).

### Verdict
main is merged into lane/bf9b in ONE merge commit (98d71fe, parents a7491d4 + 5abdf01; the 49 lane commits keep their
SHAs). 6 textual conflicts and 3 semantic conflicts resolved keeping both sides. Build rc 0, ctest serial 1234 / 1234,
probe-tsan-unit 5 / 5 with 0 warnings, 0 conflict markers, CLAUDE.md 24,224 B. Everything below is VERIFIED (run or
read this stage) unless marked INFERRED. Nothing was run live: how the merged app behaves on screen is NOT checked
here (M3).

### Commits
| sha | what |
|---|---|
| 98d71fe | merge(s-rta-1003 bf9b): main into lane/bf9b (6 conflicts + the 3 semantic fixes the build needed) |
| 790fe8f | docs: R-S3 Pitfall NN -> 67 outside the conflict hunks (architecture.md, performance-controls.md, pitfalls.md wording) |
| 8803378 | docs + probe: R-S3 one /api/debug/undo (integration.md, testing-eyes.md, APP-INVENTORY.md, probe-boxes.py) + APP-INVENTORY counts re-derived |
| (next) | this report |

### Conflict hunks (6 files, 8 hunks) and semantic fixes (3)
| # | file | main wanted | the lane wanted | resolution |
|---|---|---|---|---|
| C1 | src/ui/DeckTabRow.h (1 hunk) | `editorRect(tab, rowWidth)` -- the rename box's rect (ui, Pitfall 65) | `undoRemoveHint(deckName, playingLayers)` -- the Undo Remove text naming the layers that keep playing | BOTH functions, the lane's first, each with its own comment; neither body changed |
| C2 | src/ui/DeckView.cpp rebuildGrid (1 hunk) | `cell->setVideoInfoSource(&videoInfoSource_)`; then `setClip(layer->getClipAt(col))` / `setActive(activeClipColumn == col)` (per-deck layers) | `setClip(deck->getClip(layerIdx, col))` / `setActive(runtime().activeRef() == ClipRef{deck->id, col})` | main's setVideoInfoSource line + the lane's two lines (the cell reads the SHOWN deck's row and lights by ClipRef; the codec tooltip keeps its source) |
| C3 | src/ui/DeckView.cpp setupDeckTabs (auto-merged, listed because both sides edited the same statement group) | `tabTooltipFor(deck, showing)`, `onClick = tabClicked(i)` (a click on the showing tab does nothing) | `btn->dot = deckIsPlaying(deck.id)` | git kept all three; read and confirmed |
| C4 | CLAUDE.md Key capabilities (hunk 1) | the 18-category source breakdown dropped (bytes) | "decks are boxes of clips over one shared layer stack" added, "persistent layers across deck switches" and "cross-deck transitions" removed | the lane's line with main's shortening ("18 registry categories,") |
| C5 | CLAUDE.md pitfall index (hunk 2) | lines 64, 65, 66 | 63 reworded (ClipRef packing), "NN." line | 63 = the lane's wording, 64-66 = main's, the lane's line numbered 67 (R-S3) |
| C6 | docs/claude/pitfalls.md (1 hunk) | entries 64, 65, 66 | 63 extended (ClipRef), entry NN | 63 lane, 64-66 main verbatim, the lane's entry = 67 |
| C7 | docs/claude/performance-controls.md (1 hunk) | two new paragraphs (deck rename in place; clip file info + Show in Finder) before Beat Snap; Beat Snap unchanged | Beat Snap reworded (a deck switch never cancels a queued trigger; Remove Deck cancels) | main's two paragraphs, then the lane's Beat Snap paragraph |
| C8 | .harmony/APP-INVENTORY.md (2 hunks) | DeckView row: tab click / double-click rename text; ClipCell and ClipInspector rows (BF3) | DeckView row: bf9b pads / column header / tab dot text; LayerStrip row (badge); LayerInspector row (no persistent) | DeckView = the lane's row with main's tab text spliced in ("click = switch deck = the grid only, bf9b; a small dot ...; a click on the deck already showing does nothing ...; double-click ... BF8"); LayerStrip + LayerInspector = lane; ClipCell + ClipInspector = main |
| S1 | src/api/ApiServer.{h,cpp}, src/MainComponent.cpp (auto-merged; did not compile: two `handleDebugUndo` / `onDebugUndo`, two registrations of POST /api/debug/undo) | ui lane: `POST /api/debug/undo {redo}` -> `handleMenuCommand(kCompUndo / kCompRedo)` | bf9b: `POST /api/debug/undo` -> `undoAffectsLayerOrder(); undo(); refreshAfterUndoRedo()` | R-S3: main's route only. main's `kCompUndo` case is the same three statements (MainComponent.cpp `case C::kCompUndo`), so the lane's registration, handler, declaration, callback and wiring were removed; `/api/debug/remove_deck` and `/api/debug/save_composition` kept |
| S2 | tests/test_clip_cell_media.cpp (main's new file; did not compile on the lane) | `deck->getLayer(0)->clips[3]`, `->getClipAt(3)` (per-deck layers) | a deck has rows (`Deck::getRow`), no layers | 4 lines `getLayer(0)` -> `getRow(0)`; the assertions unchanged |
| S3 | tests/test_deck_tab_rename.cpp case (f) (main's new file; did not compile) | `REQUIRE(r.comp.removeDeck(2))` -- deck gone, the box closes without renaming | `Composition::removeDeck` deleted; `retireOrEraseDeck` returns true only when RETIRED | `CHECK_FALSE(r.comp.retireOrEraseDeck(2))` (erased: no layer plays deck C) + `REQUIRE(r.comp.decks.size() == 2)`; the five following assertions unchanged |

Semantic review of main's code in the auto-merged files (read, not only compiled):
- Every `getActiveDeck()` / `activeDeckIndex` main added (git diff 11820fa main, added lines: MainComponent
  `onDebugClipMedia`, `revealClipAt`; DeckView `tabClicked`, `tabRowMouseDown`, `setupDeckTabs`, `deckTabsStateForTests`)
  is BOX work on the shown deck (the grid's cell, the tab row) -- right under Pitfall 67. None is in src/render or
  Autopilot.cpp. `MainComponent::videoInfoFor` looks the player up by clip id, not by deck.
- main changed NOTHING under src/render (`git diff --stat 11820fa main -- src/render` empty): bf10's work is
  ProjectMSource.{h,cpp} + CMake. So H3 (the playlist advance walks the PLAYING layers, Renderer.cpp) and H2 (a deck
  switch = `handleDeckSwitch`: the index, the fence token, `DeckView::showDeck`) are the lane's code unchanged; their
  text lints pass in ctest (#1107 B4d / H2, test_render_thread_lint).
- The rename box across a deck switch: on the lane a switch with the same grid shape calls `showDeck()` -> `refresh()`
  (no row rebuild); `refresh()` re-sets every tab's colour, label and `tabTooltipFor(deck, isActive)` (auto-merged, read
  at DeckView.cpp `refresh`), so "Double-click: rename" follows the shown tab; the box is not touched. A switch that
  changes the shape goes through `rebuildGrid` -> main's "box back on top" code. INFERRED from reading: not run live.

### Gates (raw lines; logs in the scratch dir)
- Reconfigure + build (build.sh, 13:39:37-13:42:36): `configure rc=0`, `-- libprojectM:
  /Users/boriskarpman/.local/opt/projectm-4.1.1-fbo1/lib/cmake/projectM4 (render_frame_fbo: yes)`; first build
  `build rc=2`, 11 `error:` lines, ALL in the two test files of S2 / S3 (the app and every other target built);
  after S2 / S3 `build rc=0`, 0 `error:` lines (13:43:40); at the final head `build rc=0` (nothing to rebuild).
- ctest SERIAL under /tmp/audiodna-ctest.lock, on the merge tree (13:43:46-13:45:56) and again at the final head
  (13:48:35-13:50:42): `100% tests passed, 0 tests failed out of 1234` / `Total Test time (real) = 126.63 sec`
  (first run 130.46 sec).
- Count: merged 1234 = main (5abdf01) 1191 + lane (a7491d4) 1157 - base (11820fa) 1114. Base 1114 is from the lane
  report ("count(STAGE_P_BASE) 1114") and APP-INVENTORY ("ctest -N 1114 on merged main b844b65"); it was not rebuilt
  here. No test file other than tests/CMakeLists.txt was changed by both sides (comm of the two `git diff
  --name-only` lists), so no case was added twice and none is lost: main +77, lane +43 net (60 added, 21 retired, +3
  Stage P, +1 fix round). `catch_discover_tests(` lines 126 = 124 + 120 - 118.
- `.harmony/probe-tsan-unit.sh build-tsan` (13:46:38-13:46:44): `tsan rc=0`; `probe-tsan-unit: ctest -L tsan finds 5
  [tsan] cases (expected 5)`; `100% tests passed, 0 tests failed out of 5`; "WARNING: ThreadSanitizer" count 0. (cmake
  re-ran the configure; both targets were up to date -- main changed no file they compile.)
- The lane's B4 text lints inside ctest, all Passed: #1104 "render thread: one trigger-tuple load per layer per pass
  (pinned counts)"; #1106 "bf9b: box / stack structure writers sit only in audited sites (pinned counts)" (B4f);
  #1107 "bf9b B4d / H2: a deck switch path touches nothing that plays (smoke, one level)"; #1108 "bf9b B4g: a deck
  switch is never an Undo step -- the tab click is exactly handleDeckSwitch(deckIdx); ..."; #1170 "shared model field
  types are pinned at compile time"; #776 "bf9b S3.3: the TopBar has no deck Fade control". B4a / B4b are greps
  outside ctest, re-run by hand: B4a hits only src/model/ShowMigration.h (the legacy `persistent` /
  `globalTransitionSpeed` readers, as at the lane head) plus two unrelated comment words ("persistent" in
  EmbeddedShaders.h:5037, Renderer.h:347); B4b (`getActiveDeck(` / `activeDeckIndex` on code lines of src/render/* and
  src/model/Autopilot.cpp) 0 hits.
- Conflict markers: `git grep -E '^(<<<<<<<|=======|>>>>>>>)( |$)' -- . ':!.harmony/.reports'` = 0.
- CLAUDE.md: 24,224 B (cap 25,000; main 23,962, lane 24,264). The lane's rule 15 ("Decks are boxes; the layers
  play") and main's lines (Deck tab row "double-click the deck on screen = rename in place", index 64-66) are all
  present. Nothing had to be moved out.

### R-S3
- Pitfall order: 64 mkvidx, 65 ui, 66 bf10, 67 bf9b in CLAUDE.md's index and docs/claude/pitfalls.md; "Pitfall NN" in
  docs / src / tests / CLAUDE.md = 0 (`git grep`). .harmony/HANDOFF.md's "Pitfall NN" lines are about OTHER lanes'
  old comments (ledger 7) -- left alone. 68 is free.
- One undo route: `git grep -c 'debug/undo"' src/api/ApiServer.cpp` = 1. probe-boxes k1a_undo and k9c post
  `{"redo": false}` to it (they already used the same URL; the body is now explicit). Not run live in M1.
- docs/claude/testing-eyes.md lists `/api/debug/undo` once and points at integration.md for the lane's
  remove_deck / save_composition (described there once).

### found_not_fixed
1. `.harmony/probe-ui-files-rename.sh` row R3 expects "tab_click 2 -> builds +1" (a deck switch rebuilds the tab
   row). On the merged lane a same-shape switch is `showDeck()` -> `refresh()`: `tab_row_builds` stays +0. INFERRED
   from the code (not run: no live app in M1). This is a pre-registered bar that Boris's ruling changes ("a deck
   switch changes only the grid; no strip is rebuilt") -- Harmony must rule it (H-5: list with the reason, never
   re-threshold) before M3 re-runs that probe. Rows after R3 that count builds across a switch may be affected the
   same way.
2. APP-INVENTORY's "242 embedded shaders" was NOT re-derived: I found no counting method that gives 243 / 242
   (`R"(` occurs 269 times, 250 `const char*` lines). It stands on: main touched nothing under src/render.
3. APP-INVENTORY's "41 registered REST routes" headline is older than both sides (ApiServer.cpp has 67
   registrations, 25 of them /api/debug/*); not touched (out of this stage's scope).
4. docs/claude/performance-controls.md (main's rename paragraph) says "a REST / MIDI / OSC / genre deck switch keeps
   the box on its deck"; the lane made the genre deck switch inert. Harmless wording, left as main wrote it.
5. R-N1: 33 .harmony scripts match the by-name quit / kill grep on the merged tree (the lane's 31 + main's
   probe-milkdrop.sh and probe-ui-files-rename.sh; probe-quit-ours.sh excluded) -- M2's scope.
6. A git post-commit hook ("graphify hook: launching background rebuild", log ~/.cache/graphify-rebuild.log) fired on
   each of my commits. It is not mine; the worktree stayed clean after it.
7. Out-of-scope items (the round-1 MUST's residual use-after-free read, Undo of Add / Load / Duplicate Deck, the
   wiring lint, K5 Link-on, S2b 4.B omissions, grid NITs, the strip badge removal): not tripped over, not touched.

### next_stage_notes (M2 builder)
- Head: see `git log -1`; build-lane holds the merged Release app (built 13:43 from 98d71fe's src) and all tests;
  build-tsan is current. A reconfigure is NOT needed unless CMake files change.
- The by-name quit list for R-N1 (grep `tell application "Audio-DNA" to quit|pkill.*Audio-DNA|killall.*Audio-DNA|
  adna_kill` over .harmony/*.sh, *.py): gate-s165 probe-async-load probe-beatclock probe-btguard probe-capture
  probe-crossfade probe-deck-path probe-deck-tabs probe-downbeat-level probe-effects-parity probe-finalize-loop
  probe-fitmode probe-idle-paint (.py + .sh) probe-image-load probe-lane3 probe-manual-bpm probe-mastersignal
  probe-media-open probe-milkdrop probe-onset-render probe-outputs probe-resync probe-routine-display probe-routines
  probe-seq-vram probe-step3 probe-tempo-silence probe-tempo-start probe-tsan probe-ui-files-rename probe-video
  probe-vupload. bf10's probe-milkdrop has an attach-mode refusal already -- read it before changing it.
- H-4: gates-r2 NIT 6 (record_ourpid after the 60 s health loop) and NIT 4 (k1b_duplicate registered row) are M2's.
- M3: probe-milkdrop.py is now in the tree (m9b_deck_switch_live goes there); RED arm = main 5abdf01's app
  (/Users/boriskarpman/projects/RealTimeAudio/build/..., read-only). found_not_fixed 1 needs a ruling first.
- ctest count to expect: 1234 (+ whatever M2 adds).

### Notes for .harmony/notebook.md (Harmony appends)
- A merge whose two sides each add a route / handler with the SAME name auto-merges textually and fails only at
  compile (two `handleDebugUndo` declarations): grep `server_.Post("` for duplicate paths after any merge of ApiServer.
  | discovered: src/api/ApiServer.cpp
- `Composition::retireOrEraseDeck` returns true only when the deck was RETIRED (a layer plays it); an erased deck
  returns false -- a test that wants "the deck is gone" checks `decks.size()`, not the return. | discovered:
  tests/test_deck_tab_rename.cpp case (f)
- On the shared stack a same-shape deck switch does not rebuild the tab row (`showDeck` -> `refresh`); anything that
  counted on a rebuild per switch (probe-ui-files-rename R3 `tab_row_builds`) reads +0. | discovered:
  src/ui/DeckView.cpp showDeck

### PACKET QUALITY
- Clarity: CLEAR.
- Missing context: the B4a / B4b grep script (the lane's scratch b4.py is gone; re-derived from the lane report's
  wording); the method behind the "embedded shaders" count.
- Unused context: ruling-bf9b's K gate list (no live rows in M1); ui.md / bf10.md lane reports (the code and
  Pitfalls 65 / 66 were enough for the two source hunks).
- Self-brief files: rulings-bf9b-merge.md, rulings-bf9b-mergein.md, bf9b.md fix-round sections, plan-bf9b HARMONY
  ADOPTION -- all useful, none stale. No DEPARTMENT / KNOWLEDGE_TOOLS block: no knowledge tools -- grep-only (no code
  was judged dead on "no callers"; the removed undo handler was a duplicate definition).
- pulse.json: GREEN; claims "general" by two harmony sessions (the dispatcher), no area conflict.
INBOX-RECHECK: none

### SLIM CHECK
3 source files + 2 test files changed beyond the textual resolutions, each line traced to a compile failure or R-S3;
no drive-by edits; no new abstractions; no test added (none required by the packet).
