# Reviewer Verdict - bf9b merge lens, round 1
STATUS: DONE
VERDICT: PASS_WITH_NITS (0 MUST)
PINNED: lane/bf9b head 4137f6043eb7d177626b73dcd0d60d6658f4da0d, base a7491d4, merge commit 98d71fe (parents a7491d4 lane, 5abdf01 main)
CONFIDENCE: VERIFIED by git reads/greps at the pinned head; build and test results are INFERRED from the lane reports (read-only review, nothing built or run).

## Merge resolution (M1, ruling H-1)
- 6 conflicted files (DeckTabRow.h, DeckView.cpp, CLAUDE.md, pitfalls.md, performance-controls.md, APP-INVENTORY.md): both sides kept. VERIFIED: every identifier main added in DeckView.h/.cpp, DeckTabRow.h, MainComponent.h/.cpp, ClipCell, ClipInspector, VideoPlayer, ApiServer, CMakeLists, tests/CMakeLists exists at the head (comm of main-added identifiers vs head: empty for all 15 files).
- Merge vs lane parent differs from main's own change only in the intended R-S3 dedup (the lane's second onDebugUndo / handleDebugUndo / registration dropped): ApiServer.cpp, ApiServer.h, MainComponent.cpp; plus the two moved tests (test_clip_cell_media, test_deck_tab_rename: Deck::getRow / retireOrEraseDeck). VERIFIED by diffing the +/- line sets.
- ONE POST /api/debug/undo: src/api/ApiServer.cpp:340 (registration), :2367 (handler, honours {"redo"}); src/MainComponent.cpp:2247 (onDebugUndo(bool redo) -> kCompUndo/kCompRedo). VERIFIED single registration.
- ui lane: tabClicked no-op on the shown deck (DeckView.cpp:559), beginRename/finishRename/placeRenameEditor, DeckTabRow::editorRect, tab tooltip with showing flag (refresh updates colour+tooltip, DeckView.cpp:316-323), applyDeckRename, videoInfoFor, revealClipAt/revealClipFile, the test-only routes (clip_media, reveal_clip, inspect_clip, deck_tabs, deck_rename, tab_click, tab_dblclick) all present. cancelDeckRename before a load kept (MainComponent.cpp:3069).
- bf10: ProjectMSource.cpp equals main plus three relaxed counters; playlist advance walks composition_->playingClip(li) per layer inside the deckActive gate, whatever deck is shown (Renderer.cpp:549-640); nothing in the lane calls loadPreset/resize/releaseGL on a deck switch (showDeck -> refresh, DeckView.cpp:338).
- mkvidx: src/media/ diff main->head only drops advanceClock (lane), no keyframe/seek logic touched.
- hyg: lane's new log traces use logLine (MainComponent.cpp:670, 3357, 3378, 3725); the one bare std::cerr "[Replay]" (MainComponent.cpp:1954) was already at a7491d4.
- Lane: no per-deck layers, no Persistent (lint "no Persistent-feature identifier" allow-lists only model/ShowMigration.h; remaining src hits are comments), no DeckClock/AutopilotBank/advanceClock/tickMediaClock anywhere in src/tests, showDeck present, retired decks documented, load notice / Undo Remove / badge / dot gone (lint B4j in tests/test_render_thread_lint.cpp:444, scans src whole lines; no hits for load_notice/undoHint in src, .harmony probes).
- Docs: pitfall order 64 mkvidx, 65 ui, 66 bf10, 67 bf9b in CLAUDE.md:220-223 and pitfalls.md:139-144. CLAUDE.md = 24,224 bytes (<= 25,000). Main's added doc lines are all present at the head except the testing-eyes undo/tab line, which was deliberately rewritten once (testing-eyes.md:24, "the ONE undo route"). APP-INVENTORY routes: 67 registrations / 25 /api/debug (counted at head, matches the inventory).
- tests/CMakeLists.txt: no duplicate target; main's targets all present except test_deck_clock and test_layer_inspector_persistent_toggle, which are the lane's deliberate deletions (absent at a7491d4 too).
- Stray: no symlink, no .venv, no mutant/instrumentation in src; only getenv added is ADNA_INSPECT_LAYER (MainComponent.cpp:2442), inside #if AUDIODNA_TEST_SERVER and inert unless set (already at a7491d4, used by .harmony/probe-asan-live.sh:78). Mutant diffs exist only as evidence records under .harmony/.reports.
- R-S1 / H-6 evidence: m9b RED on main app 0/3 (H2 reads "no reader" there: it proves only that the counters are new, the real H2 teeth are the FIX-4 mutant run), GREEN on lane 3/0 (bf9b-merge-M3/m9b/*.log). probe-ui-files-rename H-6 change touches only the build-count clause (tab_row_builds == bb), with R11 positive control (+1 on duplicate_deck).

## Findings (no MUST)
See structured findings: 4 NITs.
