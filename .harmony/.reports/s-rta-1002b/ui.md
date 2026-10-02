# LANE ui -- stage U1 (BF3 data) -- builder report
STATUS: DONE (stage U1 of 4; U3 / U2 / U4 not started -- later stages)
Worktree: /Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/ui  branch lane/ui  base eff2b1c

## Items
### U1.1 VideoInfo.h (+ AM9 codecLine / frameLine) -- DONE
- NEW src/media/VideoInfo.h (std only): struct VideoInfo {codec, width, height, fps; known()}; namespace videoinfo:
  fourcc(a,b,c,d) (MKTAG order, = FFmpeg codec_tag), codecLabel(ffName, fourcc, profile) (plan table: H.264 / HEVC +
  profile; ProRes by profile, else FourCC apco/apcs/apcn/apch/ap4h/ap4x, else "ProRes"; HAP by FourCC
  Hap1/Hap5/HapY/HapM/HapA, else "HAP"; DXV, Motion JPEG, MPEG-4 Part 2, MPEG-2, QuickTime Animation, PNG, Uncompressed,
  VP8, VP9, AV1, CineForm; DNxHD or "DNxHR <grade>"; else the FFmpeg name upper-cased; empty name -> ""),
  fpsText (integer milli-rounding, no locale; <= 0 / NaN / huge -> ""), codecLine, frameLine, describe
  (= codecLine + ", " + frameLine).
- NEW tests/test_video_info.cpp [videoinfo]: 8 TEST_CASEs / 61 assertions (every table row + fallbacks, fpsText incl.
  30000/1001, 24000/1001, 60000/1001, 25, 30, 85/4, 0, -1; codecLine / frameLine / describe with and without a rate).
- tests/CMakeLists.txt: target test_video_info appended (test_video_player_open's link recipe; TEST_FIXTURES_DIR).
- RED(stub) 2026-10-02 16:23:09 (stub bodies return {}), raw:
    test cases:  8 |  8 failed
    assertions: 61 | 6 passed | 55 failed
  (the 6 passes = the "" expectations: codecLabel("") x2, fpsText(0 / -1), known() true/false on the struct.)
- GREEN 16:23:35, raw: All tests passed (61 assertions in 8 test cases)
- ctest -N lists the 8 cases (#1114-#1121).
### U1.2 VideoPlayer open-time info + getter (+ AM11 fixtures) -- DONE
- src/media/VideoPlayer.h: #include "media/VideoInfo.h"; in the "Video properties (valid after open)" accessor block
  `const VideoInfo& getInfo() const { return info_; }`; member `VideoInfo info_;` in the "written in open(), read-only
  after" block (after timeBase_).
- src/media/VideoPlayer.cpp open(): ONE hunk, immediately after the frame-rate block (`frameDur_ = 1.0 / frameRate_;`),
  before the pol_ block: info_.width/height = width_/height_; info_.fps = avg_frame_rate if valid, else r_frame_rate if
  valid, else 0.0 (never the 30.0 fallback); info_.codec = videoinfo::codecLabel(avcodec_get_name(codec_id),
  codecpar->codec_tag, avcodec_profile_name(...) or ""). Nothing else in VideoPlayer changed (adoption 4 fence:
  readKeyIndex / decodeStep / runStep / seek paths untouched; `git diff` = +14 lines in open(), +5 in the header).
- NEW fixtures tests/fixtures/ (ffmpeg 8.0, commands in the test file header; sizes + ffprobe identical to the plan's):
    video_prores_hq_64x64_2997.mov     20,345 B  prores HQ apch 30000/1001
    video_hapq_64x64_60.mov            25,312 B  hap (profile unknown) HapY 60/1
    video_hevc_main10_64x64_23976.mp4   4,529 B  hevc Main 10 hev1 24000/1001
- tests/test_video_info.cpp [videoinfo][open]: 9 TEST_CASEs, in-process open() of valid fixtures, closed, never
  started: h264_64x64.mp4 "H.264 High" 64x64 fps == 30.0 exactly (+ describe == "H.264 High, 64 x 64, 30 frames per
  second"); hapa .mov "HAP Alpha" "30"; prores "ProRes 422 HQ" "29.97"; hapq "HAP Q" "60"; hevc "HEVC Main 10"
  "23.976"; mpeg4_bf2 .avi "MPEG-4 Part 2" "30"; AM11: h264_vfrgap .mp4 "H.264 High" "21.25"; rawrgba .mov
  "Uncompressed" 63 x 37 "30"; mpeg4 .ts "MPEG-4 Part 2" "30". Expected values from ffprobe 8.0 (run this session).
- RED(stub) 2026-10-02 16:24:43 (getInfo() + info_ landed, open() not writing it), raw:
    test cases:  17 |  8 passed |  9 failed
    assertions: 117 | 79 passed | 38 failed
  ("[open]" alone: test cases:  9 |  0 passed |  9 failed / assertions: 56 | 18 passed | 38 failed -- the 18 passes are
  the REQUIREs that each fixture exists and open() succeeds, so all 9 opens work, incl. HEVC 10-bit and HAP Q.)
- GREEN 16:24:56, raw: All tests passed (117 assertions in 17 test cases); then 5 / 5 repeats the same line.

## Builds / tests
- Configure 15:49 (Release, TEST_SERVER=ON, SYPHON=ON, FETCHCONTENT_FULLY_DISCONNECTED, SOURCE_DIR_* = main build/_deps;
  cmake warned FETCHCONTENT_SOURCE_DIR_MELATONIN_INSPECTOR unused -- harmless, Release does not fetch it). Disk: 301 GiB
  free.
- Base build (tree at eff2b1c, -j3) 15:50 -> 16:22:22, BUILD_EXIT=0 (log scratchpad/ui-U1/build-base.log).
- Reconfigure after the tests/CMakeLists.txt append (rig rule), then target builds.
- Branch build after U1.2 (-j3) -> 16:25:42 BUILD_EXIT=0; 21 objects rebuilt (the app's VideoPlayer.cpp + the 5 test
  targets that compile it). Compiler warnings naming the touched files, base -> branch: VideoInfo.h 0 -> 0,
  VideoPlayer.h 0 -> 0, VideoPlayer.cpp 0 -> 0.
- Direct runs (16:26): test_video_info 117/17, test_video_player_open 10/2, test_video_decode_trace 510/5,
  test_gop_cache_store 662/26, test_video_player_gl 284/5, test_media_opener 58/7, lints test_hot_thread_io_lint 323/2,
  test_render_thread_lint 19/2, test_log_line_lint 58/3, test_shared_field_types 1/1 -- each "All tests passed".
- FULL ctest -j3 on the branch (16:26:35 -> 16:27:12), raw: 100% tests passed, 0 tests failed out of 1131
  (no main-side baseline list was taken by this lane: G1's base-vs-branch comparison is Harmony's gate; nothing failed).

## Rig discipline
- No app launched, no lock taken (U1 is data only: no live row in this stage). No Output window, no screen capture, no
  synthetic input, no debugger. .venv symlink never created. No temporary hook (AUDIODNA_DEBUG_SHOW not used).
- Worked only in the ui worktree; main checkout and lane mkvidx untouched. -j3 respected (one build at a time).
- Commit hook note: the U1.1 commit printed "[graphify hook] launching background rebuild" -- a repo hook, not started by
  me; git status stayed clean.

## Notes for .harmony/notebook.md
- 2026-10-02 VideoInfo codec labels | FFmpeg's codec_tag is MKTAG order (first char in the low byte: "apch" = 0x68637061);
  avcodec_profile_name returns NULL for HAP / rawvideo (their kind lives in the FourCC), "HQ" / "4444" etc. for ProRes,
  "DNXHR HQ" for DNxHR | discovered: src/media/VideoInfo.h, ffprobe of tests/fixtures/*.
- 2026-10-02 A tests/CMakeLists.txt append made DURING a running build is not picked up ("No rule to make target"):
  reconfigure explicitly | discovered: build-lane.

## Next stage notes (U3 / U2)
- U2.5's videoInfoFor reads `renderer.getVideoPlayer(id)->getInfo()`; getInfo() is message-thread-safe because info_ is
  written only in open() (before publication) -- same contract as getWidth().
- describe() of a VideoInfo with an empty codec gives ", W x H": U2 must only call it when known() (plan U2.5 does).
- Merge with lane mkvidx: VideoPlayer.h gets one include line (after VideoUploadBudget.h), 3 lines in the accessor block,
  1 member after timeBase_; VideoPlayer.cpp one hunk after the frame-rate block in open(). Whichever merges second rebases.

## PACKET QUALITY
- Clarity: CLEAR (plan U1.1 / U1.2 + ruling AM9 / AM11 + adoption 4 fully specified the API, table, fixtures and tests).
- Missing context: none. Small inference: the plan's "fps 0.0 == ''" was extended in the formatter to any non-positive /
  NaN rate (tested with -1).
- Unused context: everything for U3 / U2 / U4 (later stages).
- Self-brief: plan-ui.md (incl. HARMONY ADOPTION), ruling-ui.md read in full; CLAUDE.md (project) loaded by the harness.
INBOX-RECHECK: none

---------------------------------------------------------------------------------------------------------------------
# LANE ui -- stage U3 (BF8 double-click rename) -- builder report
STATUS: DONE (stage U3 of 4: U3.1-U3.5 committed; U2 / U4 not started -- later stages)
Base for U3: 9af61b2 (lane/ui after U1). Head: the report commit after 2274abf. Commits: b7186be U3.1, 6a3a731 U3.2,
92e7ff8 U3.3, 9f8bb0f U3.4, 2274abf U3.5 (+ this report).
## Items
### U3.1 active-tab click = no-op -- DONE
- src/ui/DeckView.h/.cpp: private `tabClicked(int)` (the tab's onClick is `[this, capturedIdx] { tabClicked(capturedIdx); }`):
  `composition_ == nullptr || deckIndex == activeDeckIndex` -> return, else onDeckSwitched(deckIndex). Witness
  `int tabRowBuilds_` (++ at the top of setupDeckTabs) + public `tabRowBuilds()`.
- NEW tests/test_deck_tab_rename.cpp (decks A B C, B showing; the tab = the direct-child TextButton with the deck's
  text; onClick invoked through a COPY): (a) showing-tab click -> 0 onDeckSwitched calls; (a2) REBUILDING handler
  (active = i; rebuildGrid -- MainComponent.cpp:5550's shape): showing tab -> builds +0, 3 tabs; tab C -> +1, 3 tabs;
  (b) tab C -> one call with 2, tab A -> one call with 0. tests/CMakeLists.txt: target test_deck_tab_rename appended
  (test_deck_thumbnails' recipe); reconfigured.
- RED(stub) 16:33:20 -- the stub tree = tabRowBuilds witness landed, tabClicked forwarding EVERY click (main's exact
  behaviour), raw:
    test cases:  3 |  1 passed | 2 failed
    assertions: 21 | 18 passed | 3 failed
  (fails: (a) calls.empty(); (a2) builds == b0 after the showing-tab click; (a2) builds == b0 + 1 after tab C, because
  the showing-tab click had already rebuilt once.)
- GREEN 16:33:33, raw: All tests passed (21 assertions in 3 test cases)
### U3.2 editor + listener (AM2-AM5) + editorRect -- DONE
- src/ui/DeckTabRow.h: pure `editorRect(Rect tab, int rowWidth)` = { max(0, min(tab.x, rowWidth - w)), w = max(tab.w,
  kTabWidth) }.
- src/ui/DeckView.h/.cpp (AM2-AM5 as written): DeckTabButton UNCHANGED (no double-click override). Private
  `DeckNameEditor : juce::TextEditor` (onClose(keep); keyPressed: classifyOutputKey CloseAll / RaiseApp / ToggleMain ->
  false; Return or Tab (any modifiers) -> onClose(true), true; Esc -> onClose(false), true; else base. keyStateChanged:
  base, then true). `renameEditor_` created once in the constructor (addChildComponent; onFocusLost = `if (renaming_ &&
  !hasKeyboardFocus(true)) finishRename(true)`; onReturnKey / onEscapeKey unused; setPopupMenuEnabled(false); font
  FontOptions(14); centred; tooltip "Enter keeps the new name, Esc cancels"). Nested `TabRowMouse` listener
  (addMouseListener(&tabRowMouse_, true); removed in the new ~DeckView): tabRowMouseDown = (1) a press outside the open
  box -> finishRename(true); (2) arm by originalComponent -> tab index -> deck ID, firstClick_ / armed_ exactly as AM2,
  kRenameOnlyTheShowingTab = true; tabRowDoubleClick reads no position, opens beginRename(armed deck) unless that deck is
  already being renamed. finishRename (AM4): renaming_ = false first; onRenameClosed BEFORE the hide when focus is in
  DeckView or nowhere; keep -> onDeckRenamed(index of the ID, trimmed) iff non-empty and != the current name.
  Rebuild: rebuildGrid closes (discard) a box whose deck ID is gone; setupDeckTabs ends with toFront(false) when open;
  resized() stores tabRow_ and re-places the box over its deck's tab (editorRect). Tab tooltip (AM5): line 2
  "Double-click: rename" only on the showing tab (setupDeckTabs and refresh() pass showing); line 3 byte-identical.
  Public: onDeckRenamed, onRenameClosed, beginRename, cancelDeckRename, isRenaming, renamingDeckIndex,
  tabRowMouseForTests, renameEditorForTests.
- tests/test_deck_tab_rename.cpp (AM7 (c) (c2) (c3) (c4) (d) (d2) (d3) (e: 6 SECTIONs) (f) (g) (h) (i) (j)): the test
  owns its own copy of JUCE's order (press = listener mouseDown, then the tab's onClick via a copy; double-click =
  press 1, press 2 on the tab as rebuilt, then mouseDoubleClick to that tab if it survived (SafePointer) else to
  DeckView); MouseEvents from Desktop::getMainMouseSource(); a rebuilding switch handler by default.
  tests/test_deck_tab_row.cpp: editorRect case (100-px tab same; 60-px tab -> 100 wide; LAST 60-px tab of 12 in a
  768-px row -> x 668; one 74-px tab in a 100-px row -> 0 / 100; the box covers its tab and stays in the row).
- RED(stub) 16:37:04 -- stubs: editorRect {0,0}; every new DeckView function an empty body; the editor added hidden and
  unconfigured; DeckNameEditor forwarding to the base; tooltip = main's. raw:
    test_deck_tab_rename:  test cases:  16 |   6 passed | 10 failed
                           assertions: 170 | 151 passed | 19 failed
    test_deck_tab_row:     test cases:   5 |   4 passed | 1 failed
                           assertions: 110 | 102 passed | 8 failed
  (the 6 passing = (a) (a2) (b) and the "no box" guards (c2) (d) (d2), which a do-nothing stub satisfies.)
- GREEN 16:37:59, raw: All tests passed (218 assertions in 16 test cases) / All tests passed (110 assertions in 5 test
  cases); 5 / 5 repeats of test_deck_tab_rename print the same line.
### U3.3 one rename funnel (AM4 wiring, AM12 cancel placement) -- DONE
- src/MainComponent.h/.cpp: NEW `applyDeckRename(int, const juce::String&)` (guard index; trim; empty or == the deck's
  CURRENT name -> return; else RenameDeckCmd(current -> trimmed, "Rename Deck") via pushCommands + deckView_->refresh()).
  renameDeck's modal callback keeps only `result != 1 -> return` and calls applyDeckRename (it used to compare with the
  name captured when the dialog opened; now with the current name -- plan U3.3). Wiring beside onUndoHint:
  `onDeckRenamed -> applyDeckRename`; `onRenameClosed -> ++renameFocusHomeCount_; grabKeyboardFocus()` (AM4).
  refreshUiAfterModelSwap: `deckView_->cancelDeckRename()` right AFTER the inspector-nulling block, before
  clearSelection / rebuildGrid (AM12).
- No unit test (MainComponent is not headless-testable): the live rows R5-R10 of probe-ui-files-rename (U4) cover it;
  RenameDeckCmd itself is covered by test_undo_commands. App build 16:39:44 BUILD_EXIT=0.
### U3.4 TEST-ONLY REST (AM6) -- DONE
- src/api/ApiServer.h/.cpp, inside `#if AUDIODNA_TEST_SERVER` (callbacks in a public block after the bt2 one, handlers
  after handleDebugCancelLoad, routes after audio_stop): GET /api/debug/deck_tabs (onDebugDeckTabs read ON the message
  thread, the handleDebugUiText Box + 2-s wait; "ok" set on the message thread), POST /api/debug/deck_rename {deck, op,
  text} (400 unless op is one of begin|type|enter|tab|escape|focus_lost|outside_click; begin needs deck), POST
  /api/debug/tab_click {deck}, POST /api/debug/tab_dblclick {deck}, POST /api/debug/undo {redo} (body optional). POSTs:
  callAsync, answered at once.
- src/ui/DeckView.h/.cpp: public clickTabForTests (a COPY of deckTabs_[i]->onClick, invoked), doubleClickTabForTests
  (JUCE's order: mouseDown n=1 -> onClick -> mouseDown n=2 on the tab as rebuilt -> onClick -> double-click to that tab if
  it survived (SafePointer) else DeckView; left-button MouseEvents of Desktop's main mouse source), renameOpForTests
  (begin = beginRename; type = setText; enter / tab / escape = renameEditor_.keyPressed(KeyPress) -- the real key path;
  focus_lost = the editor's onFocusLost; outside_click = tabRowMouseDown(left, 1 click, the first ClipCell, else
  DeckView)), tabRowStateForTests (active, row_width, tab_row_builds, tabs[{index,id,name,label,tooltip,x,y,w,h,showing}],
  editor{open, deck_id (-1 closed), deck_index, text, x, y, w, h}).
- src/MainComponent.cpp TEST-ONLY block (after onDebugUiRepaintAll): onDebugDeckTabs adds focus_home_count and
  undo{top, redo_top, index, size}; deck_rename / tab_click / tab_dblclick -> the DeckView functions; undo ->
  handleMenuCommand(kCompUndo / kCompRedo).
- App build 16:41:38 BUILD_EXIT=0 (strings: "api/debug/deck_rename" present once).
- BUILDER SMOKE (sanity only, NOT the G3 gate; one locked batch 16:42:31 -> 16:42:40, lane app --test-mode, open -g,
  2 decks made with /api/debug/duplicate_deck): R1 R2 R3 R4a R4b R5 R6 R7 R8 (outside_click / focus_lost / tab) R9a R9b
  all PASS -> "SMOKE fails=0"; deck_rename {"op":"nope"} -> {"ok": false, "error": ...}. Observed e.g. R4b editor
  {x 102, y 332, w 100, h 24} == tab 1 {x 102, y 332, w 100, h 24}, row 1720; R5 undo.index 4 -> 5, focus 0 -> 1;
  R7 focus +3, undo unchanged; R9a box stays on deck id 0 after /api/switch_deck {2} (builds +1). R10 (load) NOT run
  (needs a composition file: U4's probe). Raw log scratchpad/ui-U3/smoke-164231/smoke.txt. Outwins before / after:
  "audio-dna windows 2, Output-named 0"; after quit 0 / 0; UserNotificationCenter windows 16 s after the quit: 0;
  app-err.log: 0 lines matching crash|assert.
### U3.5 probe_deck_tab_dispatch (AM8; BUILT, NEVER RUN) -- DONE (built only)
- NEW tests/probe_deck_tab_dispatch.cpp: plain main() (no Catch2), `probe_deck_tab_dispatch <OUT> [--offset X,Y]`, exit
  0 PASS / 1 FAIL / 3 INCONCLUSIVE. Home (wants focus, records keys) 700 x 124 holding a real DeckView (3 layers x 12
  columns, decks A B C, A showing) at (0, -236) so only layer 0's row and the tab row show; AudioDNALookAndFeel as the
  default; setAlwaysOnTop(true), addToDesktop(0) at the primary display's userArea + (40, 60) or --offset; setVisible;
  one CFRunLoopRunInMode 0.4 s; never toFront(true), never activated. Wiring: onDeckSwitched = active + rebuildGrid;
  onDeckRenamed = write the name + refresh; onRenameClosed = home.grabKeyboardFocus(); counters on onColumnTriggered,
  onLayerClearClip, onClipTriggered + onClipSelected. Input: ComponentPeer::handleMouseEvent (move, down, +40 ms, up;
  double-click clicks 100 ms apart, gestures 1 s apart, synthetic times) and handleKeyUpOrDown / handleKeyPress, 0.3 s
  CFRunLoop turn after each step. Precheck peer->contains(centre of tabs A B C, true) else exit 3. Rows P1-P9 exactly as
  AM8 (P4 saves OUT/probe-dispatch-P4.png = capture C15, PngWrite::writeReplacing of Home's component snapshot; P7 clicks
  layer 0 / column 3 at Home (565, 60)).
- tests/CMakeLists.txt: `if(APPLE)` target probe_deck_tab_dispatch = test_deck_thumbnails' sources, linked to
  juce_gui_basics + juce_opengl + CoreFoundation; NO add_test / catch_discover_tests / Catch2; reconfigured.
- Built 16:46:01 EXIT 0; touch-rebuild of probe + both test files + DeckView.cpp: 0 compiler warnings. `ctest -N |
  grep -c probe_deck` = 0 (Total Tests: 1148). NEVER RUN by this lane (adoption 3): its RED evidence is the unit RED of
  test_deck_tab_rename / test_deck_tab_row above; whether P1-P9 PASS is unknown until Harmony runs it.
## Builds / tests (U3)
- Reconfigured after each tests/CMakeLists.txt append (U3.1 test_deck_tab_rename, U3.5 probe_deck_tab_dispatch).
  Disk 294 GiB free (no new build dir). -j3 throughout, one build at a time.
- Full incremental build 16:47:14 BUILD_EXIT=0. Compiler warnings naming the touched files (U1 base log vs this
  stage's full logs): DeckView.h 0 -> 0, DeckView.cpp 0 -> 0, DeckTabRow.h 0 -> 0, MainComponent.h 2 -> 2,
  MainComponent.cpp 8 -> 8 (all pre-existing lines), ApiServer.h 0 -> 0, ApiServer.cpp 2 -> 2; probe + both test files
  0 (touch-rebuild). G0's own touch-every-file baseline is Harmony's.
- Direct runs 16:47 (raw): test_deck_tab_rename All tests passed (218 assertions in 16 test cases); test_deck_tab_row
  (110 / 5); test_undo_commands (562 / 81); test_deck_thumbnails (62 / 4); test_video_info (117 / 17);
  test_clip_thumbnails (112 / 6); lints test_hot_thread_io_lint (323 / 2), test_render_thread_lint (19 / 2),
  test_log_line_lint (58 / 3), test_shared_field_types (1 / 1) -- each "All tests passed".
- FULL ctest -j3 on the branch 16:47:15 -> 16:47:48, raw: 100% tests passed, 0 tests failed out of 1148 (U1: 1131;
  +17 = the editorRect case + the 16 test_deck_tab_rename cases). `ctest -N` lists no probe_deck_tab_dispatch.
- G2b: `grep -rn AUDIODNA_DEBUG_SHOW src tests` -> 0 lines.

## Rig discipline (U3)
- ONE live batch (the U3.4 smoke): lock taken with the helper (LANE=ui-U3) 16:42:31, lane app started by start_app in
  --test-mode (open -g), REST only, quit_app ("app running after quit: no"), lock released 16:42:40. Output-named windows
  0 before / during / after; UserNotificationCenter windows 16 s after the quit: 0. No foreign Audio-DNA was running.
- No Output window, no screen capture, no synthetic OS input, no debugger / sampler, no temporary hook, no copied or
  re-signed bundle. probe_deck_tab_dispatch BUILT, NEVER RUN. The .venv symlink was never created (the smoke used the
  main .venv python by absolute path).
- Worked only in the ui worktree; lane mkvidx's worktree / files untouched (VideoPlayer.* not edited in U3).
- Commit hook: every commit printed "[graphify hook] launching background rebuild" (a repo hook, not started by me).

## Notes for .harmony/notebook.md (U3)
- 2026-10-02 A juce::MouseEvent for a headless listener test is built with Desktop::getInstance().getMainMouseSource()
  (exists without a peer: MouseInputSourceList adds source 0 in its constructor) and the 15-argument constructor; a
  nested DeckView listener can then be driven in JUCE's verified order | discovered: tests/test_deck_tab_rename.cpp.
- 2026-10-02 Headless (no peer) grabKeyboardFocus never sets the focused component, so DeckView::finishRename sees
  "focus nowhere" and calls onRenameClosed -- the same branch the background gate app takes (a background peer is never
  focused) | discovered: src/ui/DeckView.cpp finishRename, live smoke focus_home_count.
- 2026-10-02 build-target greps that match "warning: .*<file>" miss every warning: clang prints the PATH before
  "warning:"; count warnings with `awk -F': warning:'` on the path | discovered: scratchpad/ui-U3/warn-check.sh.

## Next stage notes (U3 -> U2 / U4)
- U2 shares the files U3 touched: DeckView.h (public block: U3 added onDeckRenamed .. tabRowStateForTests after
  getNaturalHeight / tabRowBuilds; private: tabClicked, DeckNameEditor, renameEditor_, TabRowMouse ... after deckTabs_),
  DeckView.cpp (constructor end, rebuildGrid top, resized tab block, setupDeckTabs end, tabTooltipFor + the rename
  functions after it), MainComponent.cpp (wiring after onUndoHint; TEST-ONLY block after onDebugUiRepaintAll; renameDeck
  + applyDeckRename), ApiServer.h/.cpp (a new public #if block after the bt2 one; handlers after handleDebugCancelLoad).
  U2.3's cell-loop fan-out is untouched by U3. U2.5's videoInfoFor: use p->getInfo().known() (U1 hand-over).
- U4 probe (G3 R1-R10): the routes answer as AM6 says; editor.deck_id is -1 while closed. The builder smoke showed R1-R9
  PASS on the lane app; R10 (load_composition of a saved file) is still unexercised live. NB1 / C5 caution: in the
  gate app the tab row is 1720 px wide (smoke row_width), so 12 decks still get 100-px tabs (12 x 102 + 24 = 1248 <=
  1720); a 60-px last tab needs >= 25 decks at that width (or a narrower window) -- C5's "12 decks (60-px tabs)" will
  not produce 60-px tabs as written. NB1's numeric bars (w 100, inside the row) hold either way.
- Docs (AM16: performance-controls.md, pitfalls.md "Pitfall NN" four points, CLAUDE.md UI bullet, testing-eyes.md eight
  routes + the probe, APP-INVENTORY rows) are U4; nothing of them was written in U3.

## PACKET QUALITY (U3)
- Clarity: CLEAR -- AM2-AM8 gave the code shapes verbatim; the adoption fixed the probe as build-only.
- Missing context: none blocking. Inferred: (1) the RED for (a) / (a2) / (h) is RED(stub) on a tree whose tab-click and
  tooltip code is main's byte-identical behaviour (the file uses new API, so it cannot compile on main); (2) "editor
  deck_id" while closed reported as -1; (3) deck_rename "type" sets the text even with the box closed (harmless).
- Unused context: plan U2 / U4 bodies, G3 V-rows, G4 (later stages / Harmony).
- Self-brief: plan-ui.md (incl. HARMONY ADOPTION) and ruling-ui.md read in full; the U1 report; CLAUDE.md loaded by the
  harness. Knowledge tools: grep only (no KNOWLEDGE_TOOLS block); impact taken conservatively (every DeckView.cpp
  consumer target rebuilt and run: test_deck_thumbnails, test_deck_tab_rename, the app).
INBOX-RECHECK: none
