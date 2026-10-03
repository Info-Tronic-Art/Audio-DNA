# Reviewer Verdict -- lane ui, lens correctness, round 1
STATUS: DONE
VERDICT: APPROVE (PASS_WITH_NITS)
REVIEWED: lane/ui head 24e1f00 vs base eff2b1c, read only through git diff / git show (never the working tree).
MUST: none.

## What was checked (VERIFIED = read at the pinned head; INFERRED = reasoned; no build / test was run by me)
- BF3 data path (VERIFIED by reading): VideoInfo is filled once in VideoPlayer::open() (src/media/VideoPlayer.cpp:157-169)
  from the already-open stream; no probe, no decode, no new I/O. The message thread reads it through
  MainComponent::videoInfoFor (src/MainComponent.cpp ~3822-3830): Renderer::getVideoPlayer(clip.id) (map lookup under
  videoPlayerMutex_), file equality, known(). ClipCell::getTooltip builds on demand; ClipInspector::updateMediaInfo polls
  only while a video's player has not answered (videoPending_). Nothing writes a Clip (Pitfall 58); describe() and
  revealTarget() never stat or decode (Pitfall 51). The only stat is on the user's Show in Finder click.
- Fence vs mkvidx (VERIFIED): VideoPlayer.cpp has ONE hunk (open(), after the frame-rate block); VideoPlayer.h has the
  include, getInfo() and info_. No decodeStep / runStep / readKeyIndex / seek / ring edit.
- BF8 rename editor (VERIFIED by reading + the 16 test cases): one DeckView-level TabRowMouse listener; the tab is
  identified by the mouseDown's originalComponent; no position is read; armed only on click 2 when click 1 landed on
  the tab already showing (kRenameOnlyTheShowingTab = true). A double-click on a non-showing tab = one switch, no box.
  tabClicked is a true no-op for the showing tab (no onDeckSwitched, no rebuild). finishRename sets renaming_ = false
  FIRST, hands focus home BEFORE the hide (only when focus is inside DeckView or nowhere), then keeps / discards.
  Return / Tab / Shift+Tab / Esc are handled synchronously in keyPressed; the output keys return false; keyStateChanged
  returns true. A rebuild re-binds by deck id, puts the box back on top (setupDeckTabs) and closes it with no rename
  when its deck is gone (rebuildGrid). refreshUiAfterModelSwap cancels AFTER the inspector-nulling block (AM12).
- Undo / names (VERIFIED): both routes end in MainComponent::applyDeckRename (trim, empty / unchanged -> nothing,
  one RenameDeckCmd "Rename Deck"); Deck names already round-trip (tests/test_composition.cpp:853-867, unchanged).
- Show in Finder (VERIFIED against JUCE 8.0.4 juce_Files_mac.mm:457-465): a missing file reveals its parent folder; the lane
  guards the no-parent case with a status line. --test-mode records instead of calling Finder.
- Menu / patterns (VERIFIED): showMenuAsync with withParentComponent(getTopLevelComponent()), SafePointer guarded
  completion, one completion path (menuChosen). Right button only; Ctrl+left still fires (ModifierKeys: mac Ctrl+left is
  ctrlModifier | leftButton, not rightButton). No new slider, so ResettableSlider does not apply.
- Real-time rules (VERIFIED by grep of the diff): no mutex, lock, thread or allocation on any audio / render path; no
  file under src/audio, src/analysis or src/render touched; no new channel.
- Stray (VERIFIED): no .venv entry, no AUDIODNA_DEBUG_SHOW / _SNAP string in src or tests, no env hook, no mutant,
  probe_deck_tab_dispatch has no add_test / catch_discover_tests. CLAUDE.md = 24,001 B on the lane (main is 23,976 B; net
  -1 for the lane; a merge stays under 25,000).
- Tests RED-first (VERIFIED from commit messages cb36ec4, 9af61b2, b7186be, 6a3a731, da1eaa6, 52db1a3, 2582ebb, 77c01c3):
  each quotes a RED(stub / main) run before GREEN. AM9 (g)/(h) are pure text-measure guards that pass on any tree; the
  ruling itself says so. Tests drive the real DeckView / ClipCell / ClipInspector, not copies.
- Plan conformance AM1-AM16 item by item: all present (AM7 (a)-(j), AM6 route list and JSON, AM10 seam, AM11 fixtures,
  AM12 string + cancel placement, AM16 docs incl. APP-INVENTORY rows and the eight routes).
- Probe vs ruling G3 (VERIFIED by reading): every V / R row copies the bars (strict R8 undo +1 on all three ops).

## Findings (all non-blocking)
1. NIT (INFERRED) focus after a single click on the showing tab -- src/ui/DeckView.cpp:558 (tabClicked) + DeckView.h:175
   (DeckTabButton wants focus by default). On main that click rebuilt the row and focus parked on the grid Viewport; now
   the tab survives and keeps keyboard focus, so a later Return is eaten by Button::keyPressed (juce_Button.cpp:665-674)
   as a no-op click instead of reaching MainComponent's bindings. Same family as X1. Fix (a one-liner, can wait for the X1
   lane): DeckTabButton setWantsKeyboardFocus(false).
2. NIT (EXCESS_DUP) src/media/VideoPlayer.cpp:162-166 repeats the avg / r_frame_rate branching of :150-155. Disposition
   JUSTIFIED_KEEP reason="keeps the lane's hunk disjoint from mkvidx's open() edits and leaves frameRate_'s 30.0 fallback
   untouched while info_.fps must be 0 when the stream has no rate"; or DEBT_FILED to share one helper later.
3. NIT (merge, not a defect) CLAUDE.md index line "65." and pitfalls.md entry 65 sit after 63; main already has 64
   (mkvidx). The merge conflicts in both spots: keep both, 64 first (the lane report says so).
4. NIT (INFERRED, unrun) ClipCell's own mouseDown runs BEFORE the nested listener. If a cell's mouseDown ever destroys
   itself (a rebuild) while the box is open, JUCE bails out and the listener never commits. The posted onFocusLost
   (DeckView.cpp ctor) is the safety net and keeps. onClipTriggered does not rebuild the grid today, so no live case.
   Harmony's G1b P7 (click a cell with the box open) covers the normal path.

## Not verified by me
No build, ctest, probe or live run was done (read-only on git objects). The lane's reported GREEN (ctest 1174 / 0,
probe 46 / 0, 5 x 32 / 0) is taken as the lane's claim; Harmony's G0-G4 / G1b on the merged tree are the real gates.
probe_deck_tab_dispatch (real dispatch, focus-home behaviour, Return after rename) is built but never run by the lane,
as the adoption requires; it is the only check of the actual JUCE focus path.

SUMMARY: 34 files, 4 nits, 0 blocking. Confidence: VERIFIED for the code reading / fences / stray / plan items,
INFERRED for runtime behaviour.
