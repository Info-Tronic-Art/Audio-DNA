# Reviewer Verdict - lane ui, lens correctness, round 2
STATUS: DONE
VERDICT: PASS_WITH_NITS (APPROVE; 0 MUST, 0 SHOULD, 3 NIT)
PINNED: lane/ui head 04faed2650b051b59988df15232ebdbba8c1a566, base eff2b1c. Read via git diff / git show only; test executables in the lane's build-lane run read-only (not rebuilt).

## Result
Behaviour and threading are correct against the plan + HARMONY ADOPTION + ruling-ui AM1-AM18. No defect that ships, no missed plan item, no toothless test found.

## VERIFIED (read at the pinned head / run)
- VideoInfo is captured in VideoPlayer::open() only (src/media/VideoPlayer.cpp:157-169): avcodec_profile_name + string work on data already in hand, no I/O, no decode; fps never the 30.0 fallback. getInfo() read on the message thread only through MainComponent::videoInfoFor (map lookup under videoPlayerMutex_, Renderer.cpp:1741-1748; retirement is message-thread only, Renderer.cpp:1576-1583, so the pointer is valid for the call). No Renderer / audio / GL file touched; no new mutex; nothing on the audio callback.
- mkvidx fence: only one hunk in open() after the frame-rate block + a getter + a member. `git merge-tree main lane/ui` auto-merges VideoPlayer.h/.cpp; conflicts only in CLAUDE.md and docs/claude/pitfalls.md (the announced Pitfall 64 / 65 numbering).
- Rename editor semantics match AM2-AM6 / AM12: tabClicked ignores the showing tab (DeckView.cpp:~558); TabRowMouse arms on click 2 only when click 1 was on the SAME deck and it was showing (kRenameOnlyTheShowingTab); tabRowDoubleClick never reads a position (JUCE substitutes the nearest parent with comp==original, juce_Component.cpp:2265-2272 + HierarchyChecker::eventWithNearestParent, so the design is right); Return/Tab/Esc handled synchronously in keyPressed with the output keys passed on first; key-state swallowed; focus goes home BEFORE the hide only when focus is nowhere or inside DeckView; focus-loss callback guarded by hasKeyboardFocus(true); cancelDeckRename placed after the inspector-nulling block in refreshUiAfterModelSwap; rebuild re-binds by deck id and re-fronts the box.
- Undo / save-load: one funnel (applyDeckRename) -> existing RenameDeckCmd; empty / unchanged = no step; names saved by the existing path.
- Show in Finder: right button only (Ctrl+click stays a trigger, AM10); missing file -> revealToUser reveals the parent, else status line; test mode records, never calls Finder; one completion path (menuChosen) behind a SafePointer; menu via showMenuAsync + withParentComponent(getTopLevelComponent()) + app LookAndFeel. No sliders added (ResettableSlider n/a).
- Stray check: no .venv / .orig in the tree; `git grep AUDIODNA_DEBUG_SHOW|SNAP -- src tests` = 0; only pre-existing getenv; TEST-ONLY wiring inside `#if AUDIODNA_TEST_SERVER` in both ApiServer.* and MainComponent.cpp; probe_deck_tab_dispatch has no add_test / catch_discover_tests.
- CLAUDE.md = 24,001 bytes (base 24,002; cap 25,000). Docs per AM16 present (performance-controls.md, pitfalls.md 65, testing-eyes.md eight routes, APP-INVENTORY).
- Fix-round items: F-MUST-1 verified under /bin/bash 3.2.57: `${ENVS[@]+"${ENVS[@]}"}` yields no words for an empty array and keeps "A=b c" whole; `${ENVS[*]:-}` on line 92 is safe. F-SHOULD-2 seam (setFocusedComponentForTests, DeckView.cpp:667, empty = shipped path) + test (k) + closedBeforeHide() give the "home before hide" and "focus left elsewhere" claims real teeth. F-SHOULD-3 V4 now covers 8 cells.
- Ran from build-lane (read-only): test_deck_tab_rename 259/17, test_clip_cell_media 42/9, test_clip_inspector_media 53/8, test_video_info 117/17, test_clip_media_text 82/8, test_deck_tab_row 110/5 - all pass. New fixtures used by test_video_info plus the three AM11 fixtures all exist at base.

## NITS
1. (INFERRED, not run) src/ui/DeckView.cpp:667-669 - finishRename calls onRenameClosed (-> MainComponent::grabKeyboardFocus) whenever focus is nowhere. After an app switch with the box open, the posted focus loss keeps the name (right) but "focus nowhere" is also the app-inactive state, so grabKeyboardFocus runs from a background app. Spec (AM4) says exactly this, so it is conformant; Harmony's live run (G1b P3-P9 / R8) is the place to confirm it never raises the window. Cheap fix if it does: skip the call unless juce::Process::isForegroundProcess().
2. (SLIM, JUSTIFIED_KEEP reason="named by ruling AM6; wired only from the AUDIODNA_TEST_SERVER block; precedent setBackendsForTests / setMenuLauncherForTests") src/ui/DeckView.cpp:736-830 (clickTabForTests, doubleClickTabForTests, renameOpForTests, tabRowStateForTests + leftMouseEventForTests) are compiled into the shipping DeckView and unreferenced by any ctest; ~100 lines dead in a release binary. Optional: wrap in `#if AUDIODNA_TEST_SERVER`.
3. (NIT) Headless tests cannot exercise the stale-focus-loss guard `renaming_ && !hasKeyboardFocus(true)` (DeckView.cpp ~36-39) or the real dispatch; both rest on G1b (probe_deck_tab_dispatch, Harmony runs it after asking Boris). Not a lane defect.

## RISK (action-changing)
The real JUCE double-click path and real keyboard focus are only proven by Harmony's G1b probe and the live rows R2-R4 / R8-R10; the lane never ran G1b by design. Treat G1b exit 0 with P1-P9 as the merge condition. Merge needs the hand-keep of CLAUDE.md index lines 64 then 65 and pitfalls.md 64 then 65.

METADATA: reviewer=reviewer, slug=ui-correctness-r2, date=2026-10-02
