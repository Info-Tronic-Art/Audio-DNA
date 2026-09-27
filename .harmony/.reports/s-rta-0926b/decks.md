# Lane decks (s-rta-0926b plan6) — Builder report

STATUS: DONE_WITH_CONCERNS

RESULT: plan6 shipped as 7 commits in the plan's order (E → D → A-1 → A-2 → B → C → F) on `lane/decks-0926b`
(base main 41aeb23). The deck tab row has a "+" (New Deck / Load Deck...) and a right-click menu per tab (Save Deck /
Save Deck As... / Rename Deck... / Duplicate Deck / Remove Deck); Remove shows a 10-s `Undo Remove "<name>"` button. The
legacy Deck Save / Deck Load pair is gone. Opening a composition from the library, or New Composition, now asks first.
A library right-click no longer deletes the file silently. Every deck gets a unique id. ctest 613 → 625/625. Live
probe: the pre-change binary fails R2/R3; the lane binary passes 18/0 (with the hook) and 6/0 (final, hook-free). All
§11 shots are taken and match the spec. Concerns (not fixed, need a decision): the new dialogs draw with JUCE's
default look (rounded buttons, like every alert already in the app); the library's Decks section lists Boris's four
old-format deck files, which cannot be loaded anywhere now.

FACTS (disk-cited; paths are relative to the worktree unless absolute):
- Step 0: `git merge-base --is-ancestor lane/probe-hygiene2-0926b main` → exit 0; `git checkout -B lane/decks-0926b main` (41aeb23). Main moved past the plan's base (5285662) only in `.harmony/s-rta-0926b-work.md` (`git diff --stat 5285662 41aeb23`), so every plan anchor held.
- Commits: 075ea2d (E), 01ad154 (D), de98da1 (A-1), 4dc8ae2 (A-2), 413f6d4 (B), 1299971 (C), 5723ed5 (F), plus this report/evidence commit. Each commit stayed inside its fence (`git show --stat <sha>`).
- ctest (serial `ctest --test-dir build-lane -j1`), verbatim summary lines: base `100% tests passed, 0 tests failed out of 613`; after E `... out of 615`; D `615`; A-1 `621`; A-2 `625`; B `625`; C `625`; F/final `100% tests passed, 0 tests failed out of 625`.
- RED lines, run first on the pre-change tree (verbatim):
  - E: `tests/test_undo_commands.cpp:1519: FAILED: REQUIRE( comp.decks[1].id != comp.decks[0].id ) with expansion: 0 != 0`; `tests/test_undo_commands.cpp:1495: FAILED: REQUIRE( comp.decks[1].getNumLayers() == Deck::kDefaultLayers ) with expansion: 0 == 3`; `tests/test_composition.cpp:1100: FAILED: REQUIRE( comp.decks[0].id != comp.decks[1].id ) with expansion: 0 != 0`
  - A-1: `tests/test_undo_commands.cpp:1638: FAILED: REQUIRE( comp.decks[static_cast<size_t>(comp.activeDeckIndex)].name == "C" ) with expansion: "D" == "C"`; `tests/test_undo_commands.cpp:1684: FAILED: REQUIRE( comp.decks[2].getLayer(0)->pendingTriggerColumn == 5 ) with expansion: -1 == 5`; compile-RED `tests/test_undo_commands.cpp:1551:34: error: use of undeclared identifier 'InsertDeckCmd'`, `tests/test_undo_commands.cpp:1620:34: error: unknown type name 'RenameDeckCmd'; did you mean 'RemoveDeckCmd'?`, `tests/test_composition.cpp:1149:33: error: no member named 'duplicateDeck' in namespace 'compload'`
  - A-2: `tests/test_deck_tab_row.cpp:4:10: fatal error: 'ui/DeckTabRow.h' file not found`
  - Live R2/R3 on the pre-change app (`/Users/boriskarpman/projects/RealTimeAudio/build/.../Audio-DNA.app`): `FAIL  R2 /api/composition decks[].id present (got: ABSENT,ABSENT,ABSENT)` / `FAIL  R3 deck ids pairwise distinct (ids: ABSENT,ABSENT,ABSENT)` → `4 PASS / 2 FAIL` (`decks-shots/probe-before.txt`).
- GREEN live: lane binary with the hook `18 PASS / 0 FAIL` (`decks-shots/probe-after.txt`: `decks: names=A,B,C ids=0,100,101`, R1-R6 plus 04-11 rows including `11-remove-hint: deck A removed from before the active deck -> decks B,C, B still active (index 0)`); final hook-free binary `6 PASS / 0 FAIL` (`decks-shots/probe-final.txt`).
- Phase 3 revert proof (after `git apply -R` of the saved hook patch, sha256 ee75761d…66b, then a full rebuild, binary mtime 00:55 > revert): `git diff --stat -- src/MainComponent.cpp` → (empty, 0 lines); `strings build-lane/AudioDNA_artefacts/Release/Audio-DNA.app/Contents/MacOS/Audio-DNA | grep -c AUDIODNA_DEBUG_` → `0` (the hook build read `3`). `grep -c AUDIODNA_DEBUG_ src/MainComponent.cpp` → 0.
- Plan checks: `grep -n "deckSaveButton_\|deckLoadButton_\|::saveDeck()\|::loadDeck()" src/` → empty (after D); `grep -c confirmReplaceShow src/MainComponent.cpp` → 3; `grep -rn "RemoveDeckCmd" src` → only DeckCommands.h + MainComponent.cpp removeDeck.
- Library safety: the temp file `~/Library/AudioDNA/compositions/_probe-deck-tabs.json` (and the `compositions` dir, which did not exist before) was created for shots 09/10 and removed after each launch (`probe-after.txt`: "removed temp library file ..." / "removed ... (created by this run, empty)"); `ls ~/Library/AudioDNA` afterwards = `Decks  FX Saves  Presets`, the four legacy deck files are untouched. Nothing was deleted, renamed or loaded by a dialog: the hook cancels every dialog/menu with result 0.
- Pre-existing bug witnessed and fixed within A-2's fence: `decks-shots/before-03-active-C.png` shows NO green tab after the REST switch to C (DeckView::setupDeckTabs built every tab grey and handleDeckSwitch → rebuildGrid never calls refresh()); `decks-shots/03-active-C.png` shows C green.

METHOD: Read plan6-final.md in full and every cited source region; re-derived each anchor on 41aeb23. Per commit: tests first → built only the test target → captured the RED line → implemented → built the test target to green → full build (`cmake --build build-lane -j3`) → full serial ctest → fence check (`git diff --stat`) → commit with the RED line in the message. Live: one wrapper script under the scratchpad took `/tmp/audiodna-live.lock` (owner `decks <pid> <epoch>`, 20-s poll, AUDIODNA_LOCK_OWNER=decks), symlinked `.venv` only for the run, ran `.harmony/probe-deck-tabs.sh` (pre-change Phase 1, then lane Phase 1 + Phase 2 on the hook build), and released only its own lock (two sessions: 00:50:51-00:53:42 and 01:06:48-01:07:00). All 19 PNGs were opened and checked against §11 before copying them into `decks-shots/`.

CONFIDENCE + VERIFY: HIGH for the model/command layer (RED→GREEN unit tests, deep-equal + hook counters) and for what each §11 shot shows (all opened and checked). MEDIUM for the interactive paths no probe drives: the Save/Load file choosers, the real right-click on a tab (the rig forbids synthetic input), the Undo-hint click. They are covered by code review and by a JUCE source read (`juce_Button.cpp` mouseDown/mouseUp; ModalComponentManager runs the callback before deletion). VERIFY (Harmony gate): rebuild; `ctest --test-dir build-lane` = 625/625; `.harmony/probe-deck-tabs.sh <out>` on the lane build = 6/0 (Phase 1); `git show 5723ed5 -- src/` is empty (no hook shipped); a human check of a real right-click on a tab (menu opens, deck does not switch) and of Load Deck... / Save Deck As... through the choosers.

UNKNOWNS / NOT DONE:
- Not driven live (no synthetic input allowed): the real right-click / Ctrl+click on a tab, clicking the "+" and the menu items, the Undo Remove click, the Save/Load choosers, Duplicate Deck, Rename submit. The hook only OPENS each surface.
- FOUND, NOT FIXED (need a decision): (1) the Rename / Open-Composition / Delete dialogs (and every existing AlertWindow in the app) draw with JUCE's default LookAndFeel: rounded buttons, default dark grey (shots 06/07/10). Only MainComponent calls setLookAndFeel (MainComponent.cpp:186); a top-level AlertWindow does not inherit it. That conflicts with BORIS_DECISIONS "Rejected: Rounded corners (anywhere, ever)" and plan §13. The plan prescribed these exact dialog calls. A fix needs an app-wide LookAndFeel decision (e.g. `LookAndFeel::setDefaultLookAndFeel`, or `setLookAndFeel` on each AlertWindow), then a re-shoot. (2) The library's Decks dir IS the legacy PresetManager "Decks" dir: JUCE userApplicationDataDirectory = `~/Library` on macOS, and APFS is case-insensitive. Boris's 4 v1 `*.deck.json` files (keys audioFile/fx/slots…, no "layers") list as Decks rows `feafeda.deck` etc. (shot 08). Clicking one gives "not a deck file". After D, nothing in the app can load them, and Save Deck As... defaults to the same dir. (3) `MainComponent.h` `currentAudioFile_` comment still says "for deck save" (plan kept the field; it is now write-only). (4) `PresetManager::DeckState/saveDeck/loadDeck` are dead in the app (plan-noted follow-up). (5) Inspectors keep raw Clip*/Layer* across New/Load/Duplicate. This is safe only because Deck is nothrow-move-constructible, so `decks` reallocation moves each layers buffer. Verified today: `juce::File(File&&) noexcept` (juce_File.h:103), and `sourceFile` kept it so. Nothing pins it: a `static_assert(std::is_nothrow_move_constructible_v<Deck>)` would.
- Plan fact corrected: plan §1/§9 say the compositions dir is `~/Library/Application Support/AudioDNA/compositions`; on disk JUCE maps userApplicationDataDirectory to `~/Library` (juce_Files_mac.mm:209), so it is `~/Library/AudioDNA/compositions` (did not exist before this run).

NUANCE (deviations, each within its commit's fence unless stated):
- Menu shots 04/05/09 are in-app `createComponentSnapshot` PNGs of the live top-level window, not Quartz captures. A JUCE PopupMenu is dismissed within ~50 ms while the app is not the foreground process (juce_PopupMenu.cpp:1441-1448 → WindowingHelpers.h:55-58 `Process::isForegroundProcess()`), and `open -g` never makes it foreground. The hook takes the snapshot synchronously right after showMenuAsync; the GL preview is black in it. Dialogs, the tab row and the browser are Quartz window captures as planned.
- Hook (temporary, never committed): AUDIODNA_DEBUG_COMP is loaded at +2.5 s, not synchronously at constructor end, because loadComposition fences with a blocking `executeOnGLThread` (UndoService.cpp:89) before the window/GL context exists. The state fires at +4.5 s; cancel-all at +14 s; an extra AUDIODNA_DEBUG_SNAP env names the snapshot PNG.
- A-2: `setupDeckTabs` also colours the active tab at creation (the pre-existing no-green-tab bug above). DeckTabButton also ignores popup-menu mouseDrag/mouseUp (a right-drag must never reach Button's click path). Added a `writeDeckFile` helper shared by Save / Save As. The DeckTabRow Remove item is disabled with one deck; the Deck-menu Remove stays enabled and guarded, as planned.
- C: the row-menu callback re-resolves the row AND requires the same file (the plan asked only for the range check). Removed CompDecksBrowser.h's now-orphan `#include "model/Composition.h"`. `BrowserPanel::setComposition` stays as a no-op seam: the plan's "other tabs use it" is false on disk, since its only line forwarded to CompDecksBrowser, and BrowserPanel.h / MainComponent are outside C's fence. Right-click keeps `isRightButtonDown()` ("left-click unchanged").
- A-1: also updated RemoveDeckCmd's class comment and cancelPendingTriggers' path list (InsertDeckCmd) for truth.
- F: CLAUDE.md pitfall 35 was already indexed on main; only 36 was added. Shots include `-window` captures of the main window behind each dialog.
- A-2's commit message states the tab-highlight bug before the live witness existed. The later before/after shot 03 confirms it.
- Behaviour changes Boris should know (plan §14 defaults shipped): New Deck = 3 empty layers; Remove = no dialog + 10-s Undo button (retired by any later command); Load Deck adds a tab, is undoable, and no longer stops routines or clears undo history; Save Deck As renames the deck to the file name.

HANDOFF-NEEDS: Harmony's behavioral gate + an independent Reviewer. Decisions for Boris/Harmony: the AlertWindow LookAndFeel (rounded buttons); what to do with the four legacy `*.deck.json` files in `~/Library/AudioDNA/Decks`. Do not merge the scratch hook: it exists only in the scratchpad patch, and HEAD has none of it.

INBOX-RECHECK: none

---

## SUMMARY
plan6 A-F implemented surgically on lane/decks-0926b; 12 new test cases (613 → 625); probe + fixture + docs; 19 evidence PNGs.

## FILES CHANGED (per commit)
- E 075ea2d: `src/core/DeckCommands.h` (AddDeckCmd: initDefault + appendDeck mint; comments), `src/model/Composition.h` (fromVar re-mints repeated deck ids), `src/api/ApiServer.cpp` (decks[].id), `tests/test_undo_commands.cpp` (ids test, :1492 expectation), `tests/test_composition.cpp` (re-mint test).
- D 01ad154: `src/MainComponent.h/.cpp` — legacy Deck Save/Load buttons, wiring, layout block and bodies deleted; one comment trimmed.
- A-1 de98da1: `src/model/Deck.h` (sourceFile), `src/core/DeckCommands.h` (RemoveDeckCmd any-deck + gated undo cancel; InsertDeckCmd; RenameDeckCmd), `src/core/CompositionLoad.h` (duplicateDeck), tests (operator== + 6 cases).
- A-2 4dc8ae2: `src/ui/DeckTabRow.h` (new), `src/ui/DeckView.h/.cpp`, `src/ui/MenuBarModel.h/.cpp`, `src/MainComponent.h/.cpp` (handlers, append via InsertDeckCmd, pushCommands hides hint), `tests/test_deck_tab_row.cpp` (new), `tests/CMakeLists.txt`.
- B 413f6d4: `src/MainComponent.h/.cpp` (confirmReplaceShow + 2 call sites).
- C 1299971: `src/ui/CompDecksBrowser.h/.cpp` (row menu, confirmed Trash delete, Save Deck button + composition_ removed), `src/ui/BrowserPanel.cpp`.
- F 5723ed5: `.harmony/probe-deck-tabs.sh`, `.harmony/probe-deck-tabs.json`, `.harmony/APP-INVENTORY.md`, `CLAUDE.md`, `docs/claude/pitfalls.md`, `.harmony/notebook.md`.
- Report commit: `.harmony/.reports/s-rta-0926b/decks.md`, `.harmony/.reports/s-rta-0926b/decks-shots/*`.

## TESTS
New: AddDeckCmd ids; fromVar re-mint; RemoveDeckCmd before-active; RemoveDeckCmd background-undo trigger; InsertDeckCmd x2; RenameDeckCmd; duplicateDeck; DeckTabRow x4. Full serial ctest after every commit (see FACTS). Live probe rows R1-R6 + Phase 2 rows.

## SHOTS (`.harmony/.reports/s-rta-0926b/decks-shots/`)
before-01-default, before-02-three-decks, before-03-active-C (old Row 1 with Deck Save/Deck Load, no "+", no green tab after the switch) · 01-default (Deck 1 green + "+", Row 1 ends "FX Save | No file loaded") · 02-three-decks (A B C, B green, "+") · 03-active-C (C green) · 04-plus-menu (New Deck / Load Deck...) · 05-deck-menu (header "A", Save Deck, Save Deck As..., ─, Rename Deck..., Duplicate Deck, ─, Remove Deck; app LookAndFeel) · 06-rename-dialog (prefilled "A", Rename / Cancel) · 07-replace-confirm (title, question, two-sentence warning, Open / Cancel) · 08-browser-compositions (one full-width Save Composition; Compositions (0), Decks (4 legacy)) · 09-library-menu (Open, Show in Finder, ─, Delete...) · 10-library-delete-confirm (Trash sentence, Delete / Cancel) · 11-remove-hint (B green, C, "+", `Undo Remove "A"` flush right; label `Removed deck "A"`). Transcripts: probe-before.txt, probe-after.txt, probe-final.txt.

## RISKS
The AlertWindow look (above). The undo hint is bound to the top of the undo stack (by design). Async choosers and menus capture a deck INDEX, as in the plan. Load Deck / Duplicate no longer stop routines. Bindings and genre assignments still target decks by index (pre-existing).

## PACKET QUALITY
- Clarity: CLEAR (the plan is exact, with verified anchors).
- Missing context: JUCE dismisses menus in a background app, so plan §9's "shoot at T+8 s" cannot work for menus under `open -g`. The compositions dir is ~/Library/AudioDNA, not Application Support. The library Decks dir collides with the legacy Decks dir. The dialogs render with the default LookAndFeel.
- Unused context: none.
- Self-brief files: CLAUDE.md (worktree), BORIS_DECISIONS.md:363-372, .harmony/notebook.md:1597-1611, .harmony/gotchas.md:440-470, probe-deck-path.sh, probe-render-state.sh: all useful. No DEPARTMENT or KNOWLEDGE_TOOLS block (grep-only; impact taken conservatively). pulse.json GREEN, no conflicting claim.

## STATUS
DONE_WITH_CONCERNS

## NEXT ACTION
Harmony gate + Reviewer; the two design decisions under HANDOFF-NEEDS.
