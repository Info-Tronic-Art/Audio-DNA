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

---------------------------------------------------------------------------------------------------------------------
# LANE ui -- stage U2 (BF3 UI) -- builder report
STATUS: DONE (stage U2 of 4: U2.1-U2.6 committed; U4 not started -- next stage)
Base for U2: f2690de (lane/ui after U3). Commits: da1eaa6 U2.1, 52db1a3 U2.2, 2582ebb U2.3, 77c01c3 U2.4, 34263a7 U2.5,
0d5d41c U2.6, + this report commit.
## Items
### U2.1 ClipMediaText.h (+ AM9 lines, AM12 string) -- DONE (da1eaa6)
- NEW src/ui/ClipMediaText.h (pure, juce_core + Clip + VideoInfo; no I/O): VideoInfoSource; struct Described {line,
  lines, pathTip, revealTarget, fileBacked, missing}; clipmedia::revealTarget (Video / Image = mediaFile, sequence = first
  image, else File()), imageKind (extension upper-cased, JPG -> JPEG, TIF -> TIFF), describe (fileBacked = a reveal
  target exists; Video: missing -> {"File missing"}, known info -> {codecLine, frameLine}, else {"Video file not
  loaded"}; Image: "File missing" or "<KIND> image"; sequence "Image sequence, N images"; line = lines joined ", "),
  cellTooltip (video / picture "<file name>\n<line>\nRight-click: Show in Finder"; sequence "Image sequence — N images at
  X.X images per second" + the menu line when it has an image; else ""), menuItems ({"Show in Finder"} iff a reveal
  target exists).
- Inferred details (not in the plan text): (1) menuItems returns juce::StringArray (the AM10 launcher takes a
  StringArray) instead of std::vector<juce::String>; (2) an EMPTY sequence's tooltip keeps its first line without the
  "Right-click: Show in Finder" line (it has no menu); (3) a Video / Image clip with an empty mediaFile is not file-backed.
- NEW tests/test_clip_media_text.cpp [clipmedia] 8 cases; tests/CMakeLists.txt target test_clip_media_text (Clip.cpp +
  ConnSerialization.cpp, juce_core + juce_graphics); reconfigured.
- RED(stub) 16:54:11 (every body returns {}), raw:
    test cases:  8 |  1 passed |  7 failed
    assertions: 82 | 37 passed | 45 failed
- GREEN 16:54:32, raw: All tests passed (82 assertions in 8 test cases). Build log: 0 warnings.
### U2.2 ClipCell tooltip + menu (AM10) -- DONE (see git log: "U2.2 clip cell")
- src/ui/ClipCell.h/.cpp: #include "ui/ClipMediaText.h"; setVideoInfoSource(const VideoInfoSource*) (DeckView's, the
  setThumbnails shape); getTooltip() override = clipmedia::cellTooltip(*clip_, lookupVideo()) (lookup only for Video
  clips); the setTooltip branch removed from setClip. mouseDown: `if (isRightButtonDown()) { if (clip_ && !menuItems
  .isEmpty()) showContextMenu(); return; }` then main's code (Ctrl+left unchanged). MenuLauncher + setMenuLauncherForTests;
  showContextMenu: done = SafePointer -> menuChosen; default = PopupMenu (section header = clip name, setLookAndFeel,
  withTargetComponent(this).withParentComponent(getTopLevelComponent())). menuChosen(1) -> onRevealInFinder(layer,
  column) iff the clip still has a reveal target. Header comment updated (right-click = the file menu).
- NEW tests/test_clip_cell_media.cpp: (a) tooltip == cellTooltip with a fake source, re-asked on hover, "" on a source;
  (b) Ctrl+left on a video cell triggers (guard); (b2) right-click -> launcher header "clip", items {"Show in Finder"};
  (b3) done(0) nothing, done(1) -> (2, 5) once, then a source clip -> no call; (b4) cell destroyed before done(1) -> no
  call; (c) Ctrl+left on a source triggers (guard); (d) right-click on source / empty / empty-sequence -> nothing (guard);
  (e) sequence tooltip line 1 "Image sequence — 3 images at 2.5 images per second", line 2 the menu hint.
  tests/CMakeLists.txt target test_clip_cell_media (test_deck_tab_rename's recipe); reconfigured.
- RED(stub) 16:56:40 (getTooltip = main's stored tooltip; launcher / menuChosen / showContextMenu empty; mouseDown =
  main's), raw:
    test cases:  8 | 3 passed | 5 failed
    assertions: 16 | 9 passed | 7 failed
  (the 3 passing = guards (b) (c) (d).)
- GREEN 16:57:00, raw: All tests passed (28 assertions in 8 test cases); build log 0 warnings. Consumers re-run:
  test_deck_thumbnails (62 / 4), test_deck_tab_rename (218 / 16) all passed.
### U2.3 DeckView fan-out -- DONE (see git log: "U2.3 DeckView fan-out")
- src/ui/DeckView.h/.cpp: `VideoInfoSource videoInfoSource_` declared right after thumbnails_ (before layerStrips_ /
  clipCells_), public setVideoInfoSource, onRevealInFinder; rebuildGrid's cell loop: setVideoInfoSource(&videoInfoSource_)
  (before setClip) and onRevealInFinder forwarding. TEST-ONLY cellForTests(layer, column) / revealCellForTests (=
  that cell's menuChosen(1)) after updateSelectionVisuals' neighbours. U3's blocks untouched.
- tests/test_clip_cell_media.cpp: "DeckView fan-out (U2.3)" case.
- RED(stub) 16:58:05, raw:
    test cases:  9 |  8 passed | 1 failed
    assertions: 31 | 30 passed | 1 failed
- GREEN 16:58:18, raw: All tests passed (42 assertions in 9 test cases). Consumers rebuilt + run: test_deck_thumbnails
  (62 / 4), test_deck_tab_rename (218 / 16) passed; probe_deck_tab_dispatch rebuilt EXIT 0 (NOT run, adoption 3).
### U2.4 Clip inspector button + info rows (AM9) -- DONE (see git log: "U2.4 Clip inspector")
- src/ui/ClipInspector.h/.cpp: revealBtn_ "Show in Finder" (componentID "revealClipFile", kSurface / kTextPrimary like
  the inspector's small buttons, setWantsKeyboardFocus(false), addChildComponent = hidden until a clip with a file);
  mediaInfoLabel1_ / mediaInfoLabel2_ (FontOptions(10.5f), kTextSecondary, "File missing" in 0xffcc3333, tooltip = the
  path, setMinimumHorizontalScale(1.0f) -> ellipsize, never squash). updateMediaInfo(force): MediaKey {clip, mediaType,
  reveal path, mediaMissing}; refresh() re-describes only on a key change or while videoPending_ (a Video clip with a
  file whose source has not answered); compare-before-set; resized() only when the row count or the button changes.
  setClip -> updateMediaInfo(true). paint: nameTextBounds(getWidth(), revealShown_), sections at kNameBarHeight +
  infoRowHeight() + ...; resized: button at revealButtonBounds(width) = (w - kInset - 104, 4, 104, 20), rows after the
  name bar; getPreferredHeight + infoRowHeight() (no-clip branch unchanged); PaintKey + infoRows + revealShown.
- DEVIATION (inferred, flagged): AM9 says "the name is trimmed by kRevealButtonWidth + 8 when the button shows", and its
  test (i) requires the name's right <= the button's x - 4. With the button kInset (6 px) from the edge (plan U2.4) a
  +8 trim leaves a 2-px gap and FAILS (i). I kept the plan's button position (aligned with the sections' kInset) and
  trimmed by kRevealButtonWidth + kInset + 4 (114) -- the smallest trim that passes (i). Measured: w 200 -> button x 90,
  name right 86; w 303 -> 193 / 189; w 400 -> 290 / 286.
- NEW tests/test_clip_inspector_media.cpp [clipinspector][clipmedia]: (a) visible "Show in Finder" (componentID);
  (b) picture without / with file +18; video no file / not answering / answering = +18 / +36; (c) lines == describe's
  for video / picture / missing; source and no clip: no row, no button; (d) a source answering later -> one refresh
  shows the codec; (e) onClick -> onRevealInFinder(&clip) once; (g) widths at 10.5 px under AudioDNALookAndFeel: "H.264
  Constrained Baseline" 119.461, "ProRes 4444 XQ" 69.8998, "QuickTime Animation" 93.5682, "4096 x 2160, 23.976 frames per
  second" 175.931, "3840 x 2160, 59.94 frames per second" 170.294 (all <= 200); (h) "Show in Finder" 84.7312 + 16 <= 104;
  (i) as above, plus nameTextBounds(w, false) == main's rect. tests/test_clip_inspector_paint_key.cpp: new case (f)
  (a sequence gains / loses its file -> infoRows 0 -> 1 -> 0, revealShown, one repaint each, idle between).
  tests/CMakeLists.txt target test_clip_inspector_media (paint_key's recipe + LookAndFeel.cpp); reconfigured.
- RED(stub) 17:00:42, raw:
    test_clip_inspector_media:     test cases:  8 |  2 passed |  6 failed
                                   assertions: 25 | 11 passed | 14 failed
    test_clip_inspector_paint_key: test cases:  4 |  3 passed |  1 failed
                                   assertions: 44 | 38 passed |  6 failed
  ((g) and (h) pass on any tree -- pure text measures of the constants; (b)'s RED value = 0 px: picture 812 vs 812,
  video 812 / 812 / 812.)
- GREEN 17:01:47 / 17:01:50, raw: All tests passed (53 assertions in 8 test cases) / All tests passed (44 assertions in 4
  test cases). Build logs: 0 warnings naming ClipInspector.
### U2.5 MainComponent videoInfoFor / revealClipFile / revealClipAt + wiring -- DONE (see git log: "U2.5 MainComponent")
- src/MainComponent.h (after renameFocusHomeCount_): videoInfoFor, revealClipFile, revealClipAt, lastRevealPath_,
  revealCount_. src/MainComponent.cpp: the three functions right after applyDeckRename (U3's funnel untouched).
  videoInfoFor uses p->getInfo().known() (U1 / U3 hand-over) and p->getFile() == clip.mediaFile. revealClipFile:
  testMode_ -> record (path, count) and return -- also for a missing file (V3 L1C2); else revealToUser() iff the file
  exists or its parent folder is a directory, else setFileLabel("Show in Finder: not found - " + path).
  Wiring: deckView_->setVideoInfoSource + onRevealInFinder right after U3's onRenameClosed block; the ClipInspector's
  setVideoInfoSource + onRevealInFinder right after onLayerNameChanged.
- No unit test (MainComponent is not headless-testable; plan U2.5): live rows V1-V4 of U4's probe. App build 17:03:13
  BUILD_EXIT=0; warnings naming MainComponent.cpp 8 -> 8, MainComponent.h 2 -> 2 (same pre-existing lines as U3).
### U2.6 TEST-ONLY REST (AM10) -- DONE (0d5d41c)
- src/api/ApiServer.h/.cpp inside `#if AUDIODNA_TEST_SERVER`, right after U3's block (callbacks after onDebugUndo,
  handlers after handleDebugUndo, routes after /api/debug/undo): GET /api/debug/clip_media?layer=L&column=C (400 without
  both query params; read ON the message thread, the deck_tabs Box + 2-s wait), POST /api/debug/reveal_clip {layer,
  column} and POST /api/debug/inspect_clip {layer, column} (400 without both; callAsync, answered at once; 503 unwired).
- src/MainComponent.cpp TEST-ONLY block (after U3's onDebugUndo): onDebugClipMedia -> {layer, column, cell, media_type
  (none / image / video / camera / source / image_sequence), line, lines, tooltip (the CELL's own getTooltip -- proves
  the DeckView fan-out live), path_tip, reveal_target, file_backed, missing, video {codec, width, height, fps} | null,
  menu (clipmedia::menuItems), inspector_shows (Clip tab active AND the inspector's clip is this one), inspector_lines /
  inspector_line (the labels actually visible, joined ", "), inspector_button_visible (the button's isVisible),
  last_revealed, reveal_count}; onDebugRevealClip -> DeckView::revealCellForTests (through the cell, AM10);
  onDebugInspectClip -> that cell's onSelect(layer, column, false) (= a name-bar click: selectCell + onClipSelected).
- App build 17:04:41 BUILD_EXIT=0; `strings` of the binary: "api/debug/clip_media" x1. Warnings naming ApiServer.cpp
  2 -> 2 (lines 1045, 1868: pre-existing), MainComponent.cpp 8 -> 8 (all pre-existing lines).
- BUILDER SMOKE (sanity only, NOT the G3 gate; one locked batch, lock waited 17:06:06 -> 17:11:46 behind bf2-S1a,
  released 17:11:58; lane app --test-mode, open -g; cells made with /api/debug/drop_files, the missing file by deleting a
  dropped copy so MediaPresence flags it): V1 L0C0-L0C3 (H.264 High / ProRes 422 HQ / HAP Q / HEVC Main 10, all lines
  exact, video non-null on the first read, 0 retries), V2 L0C0 tooltip exact, L1C0 "PNG image", L1C1 "Image sequence, 3
  images" + tooltip "Image sequence — 3 images at 2.5 images per second", L1C3 (empty cell) "" / [] / file_backed false,
  L1C2 "File missing" (4 polls after the delete), V3 reveal L0C0 -> fixture path count 0 -> 1, empty cell +0, missing ->
  its path count 2, V4 inspect L0C0 / L0C2 / L1C2 -> inspector_shows, the exact lines, button visible; GET clip_media
  without params -> HTTP 400 -> "SMOKE fails=0". NOT covered by the smoke (U4's probe): a procedural-source cell (V1 / V4
  L1C3 as written: no source-in-cell REST route used here) and R10. Raw log scratchpad/ui-U2/smoke-170606/smoke.txt +
  scratchpad/ui-U2/smoke-run.log. Output-named windows 0 before / after; UserNotificationCenter windows 16 s after the
  quit: 0; app-err.log crash|assert lines: 0; "app running after quit: no".

## Builds / tests (U2)
- Reconfigured after each tests/CMakeLists.txt append (U2.1 test_clip_media_text, U2.2 test_clip_cell_media, U2.4
  test_clip_inspector_media). Disk 291 GiB free (no new build dir). -j3 throughout, one build at a time.
- Full incremental build 17:12:22 -> 17:12:33 BUILD_EXIT=0. Touch-rebuild of ClipCell.cpp / DeckView.cpp /
  ClipInspector.cpp in the app target (17:13:31, 3 objects): 0 warnings naming any of them or their headers (ClipCell.h,
  DeckView.h, ClipInspector.h, ClipMediaText.h, VideoInfo.h). App-target warnings naming MainComponent.cpp 8, .h 2,
  ApiServer.cpp 2 = U3's counts, all on pre-existing lines. Every new test target's build log: 0 warnings.
- FULL ctest -j3 17:12:33 -> 17:13:06, raw: 100% tests passed, 0 tests failed out of 1174 (U3: 1148; +26 = 8
  test_clip_media_text + 9 test_clip_cell_media + 8 test_clip_inspector_media + 1 paint-key case). `ctest -N | grep -c
  probe_deck` = 0.
- Direct runs 17:13:11 (raw, each "All tests passed"): test_video_info (117 / 17), test_clip_media_text (82 / 8),
  test_clip_cell_media (42 / 9), test_clip_inspector_media (53 / 8), test_deck_tab_rename (218 / 16), test_deck_tab_row
  (110 / 5), test_undo_commands (562 / 81), test_clip_inspector_paint_key (44 / 4), test_deck_thumbnails (62 / 4),
  test_clip_thumbnails (112 / 6); lints test_hot_thread_io_lint (324 / 2; U3 323 -- one more scanned file, the new
  header), test_render_thread_lint (19 / 2), test_log_line_lint (58 / 3), test_shared_field_types (1 / 1).
- G2b: `grep -rn AUDIODNA_DEBUG_SHOW src tests` -> 0 lines.

## Rig discipline (U2)
- ONE live batch (the smoke): helper LANE=ui-U2, acquire_lock waited for bf2-S1a's lock (never touched it), start_app
  --test-mode via open -g, REST only, quit_app, release_lock. No foreign Audio-DNA was running when ours started; none
  of ours runs now; the lock dir is gone.
- No Output window, no screen capture, no synthetic OS input, no debugger / sampler, no temporary hook, no copied or
  re-signed bundle, no probe_deck_tab_dispatch run (rebuilt only). The .venv symlink was never created (the main .venv
  python by absolute path). No perf number taken (none needed in U2).
- Worked only in the ui worktree; main checkout and lane mkvidx untouched (U2 did not touch VideoPlayer.*).
- Commit hook: every commit printed "[graphify hook] launching background rebuild" (a repo hook, not started by me).

## Notes for .harmony/notebook.md (U2)
- 2026-10-02 A juce::Label's default minimumHorizontalScale (0) lets drawFittedText SQUASH text to ~70 % width before it
  ellipsizes; for a read-only info row set setMinimumHorizontalScale(1.0f) so it ellipsizes instead | discovered:
  src/ui/ClipInspector.cpp U2.4 labels.
- 2026-10-02 SettableTooltipClient::getTooltip is virtual: overriding it makes a tooltip built on hover (TooltipWindow
  polls the hovered component), so live data (a player that opened later) shows without any refresh | discovered:
  src/ui/ClipCell.cpp getTooltip.
- 2026-10-02 /api/debug/drop_files + deleting the dropped copy is a cheap way to get a "File missing" clip live:
  MediaPresence's 1 Hz sweep flags it within ~1 s | discovered: scratchpad/ui-U2/smoke.py.

## Next stage notes (U2 -> U4)
- G3 V-rows: clip_media answers `line`, `lines`, `inspector_lines`, `inspector_line`, `inspector_button_visible`,
  `menu`, `video` (null until the player answers), `last_revealed`, `reveal_count`, plus `cell` / `media_type`. The
  tooltip is the CELL's own getTooltip (proves the DeckView fan-out). inspect_clip = the cell's onSelect (selects and
  inspects; an Image clip also loads into the preview, as a real click does).
- V1 / V4 L1C3 (procedural source): the probe's composition file must place the source in the cell (no REST here puts
  a source into a cell); the smoke used an EMPTY cell for the "no row" case.
- R10 (load_composition) is still unexercised live (U3 note stands).
- C5: the U3 note stands (>= 25 decks, or a narrower window, for 60-px tabs).
- C8-C14 numbers for the critic brief: the inspector button = (w - 110, 4, 104, 20); the name stops at w - 114 (AM9 (i)
  deviation, see U2.4); info labels 18 px each at x = kInset, width w - 12, 10.5 px, ellipsized not squashed.
- Docs for U4 (AM16): testing-eyes.md must list clip_media / reveal_clip / inspect_clip with the JSON above; APP-INVENTORY
  rows :60 (cell right-click = Show in Finder menu on file clips; hover tooltip on video / picture / sequence cells;
  sequence tooltip now "images per second") and :72 (ClipInspector name-bar button + info rows).
- Merging with mkvidx: U2 touched no VideoPlayer.* file.

## PACKET QUALITY (U2)
- Clarity: HAD_TO_INFER (small): plan U2.1-U2.6 + AM9 / AM10 / AM12 gave the shapes; three inferences recorded:
  (1) AM9's "+8" name trim contradicts its own test (i) with the plan's kInset button -> trim 114 (U2.4 DEVIATION);
  (2) menuItems returns juce::StringArray (the AM10 launcher's type); (3) an empty sequence's tooltip has no menu line.
- Missing context: no REST route places a procedural source into a cell (the smoke could not cover L1C3's source row).
- Unused context: U3 / U4 bodies, G1b, G4 critic rules (later stages / Harmony).
- Self-brief: plan-ui.md (incl. HARMONY ADOPTION) and ruling-ui.md read in full; the U1 / U3 reports; CLAUDE.md loaded by
  the harness. Knowledge tools: grep only (no KNOWLEDGE_TOOLS block); impact taken conservatively (every consumer target
  of ClipCell / DeckView / ClipInspector rebuilt and run: test_deck_thumbnails, test_deck_tab_rename,
  test_clip_inspector_paint_key, probe_deck_tab_dispatch (built only), the app, the full ctest).
INBOX-RECHECK: none

---------------------------------------------------------------------------------------------------------------------
# LANE ui -- stage U4 (probe, docs, report) -- builder report
STATUS: DONE (stage U4 of 4: U4.1 probe 9af069e, U4.2 hook (scratch, never committed), U4.3 docs e5d81c2, this report)
Base for U4: 46d3ff2 (lane/ui after U2). Lane: eff2b1c..HEAD = U1, U3, U2, U4. Merge, G0-G4 on the merged tree, G1b, critic panel: Harmony.
## Items
### U4.1 .harmony/probe-ui-files-rename.sh -- DONE (9af069e)
- NEW .harmony/probe-ui-files-rename.sh (bash + embedded python, the probe-deck-tabs.sh shape): live-lock gate (exit 64
  without /tmp/audiodna-live.lock/owner or with a mismatching AUDIODNA_LOCK_OWNER); `OUT [--shots] [--hook STATE]`;
  UIFR_APP (default <root>/build/...), UIFR_ATTACH=1 (the caller started ONE Audio-DNA --test-mode and quits it -- how
  this lane ran it, through the helper's start_app / quit_app), standalone: refuses if any Audio-DNA runs or 7070 / 8080
  listen, launches `open -g ... --args --test-mode`, quits ONLY the pid it launched. Every request Connection: close;
  a 404 / error is a FAIL row, never a crash.
- Composition written at run time into OUT with absolute paths, exactly G3's: deck A L0 = h264 / prores HQ / HAP Q /
  HEVC Main 10 fixtures, L1 = test_card.png / a sequence of 3 PNG copies in OUT/media / OUT/media/missing_clip.mp4
  (folder exists, file never) / source perlin_noise (hand-over U4 note); B, C = copies, fresh deck ids 1 2 3 and clip
  ids (100s / 200s / 300s); active 0. L0C3's CLIP NAME is long ("A very long clip name that runs right up to the Show
  in Finder button") so C14 needs no extra deck (V rows read file names / codec lines only, not clip names).
- Rows = G3's strings: S0 (load ok + settle), V1 x8, V2 x4, V3 x3, V4 x2, R1, R2, R3, R4a/R4b, R5, R6, R7, R8 x3, R9a/R9b,
  R10 = 32 rows. Settle exactly as pre-registered; every null-video retry is printed (none happened: retries 0).
  Inferred detail: R8 checks "one Rename Deck step" (index +1) for all three ops, the ruling writes it for outside_click
  only (a stronger bar, same semantics). Names for R5-R10 come from GET /api/composition AFTER a GET deck_tabs barrier
  (see the R6 note below).
- --shots (G4, hook-free binary): C1-C4, NB2, C8-C14, then C5 + NB1. Window-only captures (`screencapture -x -o -l
  <CGWindowID>` of the largest on-screen Audio-DNA window, never one named Output), each DECODED (CGImageSource: size +
  distinct sampled colours). C5: N = the smallest deck count >= 12 whose DeckTabRow::layout tab width is 60 at the live
  row_width (1720 px -> N = 27; the hand-over's ">= 25" was approximate: 25 decks give 65-px tabs), box on tab N-1.
  NB1 = editor w 100, x >= 0, x + w <= row_width, y == tabs[N-1].y, h 24, the last tab 60 px, the box covers it.
- --hook STATE (cell-menu = C6, cell-tooltip = C7): loads the composition and waits for the hook's snapshot file, decodes
  it. V / R rows skipped in this mode.
- PROBE FIX caught during my first GREEN run (17:26): R6 FAILED -- "after redo ['A', 'B', 'C']" while deck_tabs a moment
  later showed the redo landed. Cause: the debug POSTs are callAsync, GET /api/composition is answered on the HTTP thread
  (not ordered after them); deck_tabs is read ON the message thread (FIFO after the queued actions). Fix: names() first
  reads deck_tabs as a barrier. Not a product defect; the RED / GREEN below are on the fixed probe.
- RED on main's pre-change app (/Users/boriskarpman/projects/RealTimeAudio/build/AudioDNA_artefacts/Release, read-only,
  --test-mode), final probe, 17:30:25, raw summary line:
    1 PASS / 31 FAIL   (artifacts in .../scratchpad/ui-U4/red-final-173016)
  (every V / R row FAILs on `{'_http': 404}` for /api/debug/*; only S0 load passes; first RED 17:25:31 on the pre-fix
  probe: "1 PASS / 31 FAIL", PROBE_EXIT=1.)
- GREEN on the lane app (build-lane, 46d3ff2 src), --shots, 17:29:14-17:29:31, raw:
    46 PASS / 0 FAIL   (artifacts in .../scratchpad/ui-U4/green-lane-172814)
  G3 rows only, 5 / 5 separate launches 17:30:33-17:31:16, raw each: "32 PASS / 0 FAIL".
  Observed values (green-lane-172814): V1 video non-null on the first read (retries 0) for all four codecs, ProRes fps
  29.97002997, HEVC 23.976023976; V3 last_revealed = the fixture path, count 0 -> 1 -> 1 -> 2; deck_tabs row_width 1720,
  R4b editor {x 102, y 236, w 100, h 24} == tab 1; R5 undo {Switch Deck, 2, 2} -> {Rename Deck, 3, 3}, focus 0 -> 1;
  R7 focus 1 -> 4, undo unchanged; R9a editor stays on deck id 1 at x 0 with builds 6 -> 7; R10 focus 8 -> 9, names A B C;
  NB2 editor {0, 236, 100, 24} == tab 0; NB1 27 decks, last tab {x 1612, y 140, w 60, h 24}, editor {1612, 140, 100, 24}.
- I LOOKED at C2 (box open on tab A, name selected), C5 (box "Deck 27" over the last 60-px tab; at 27 decks the "+" is
  clamped under the last tab and the box covers it -- pre-existing layout clamp, DeckTabRow.h), C8 (inspector: "Show in
  Finder" at the name bar's right, rows "H.264 High" / "64 x 64, 30 frames per second") and C14 (the long name ellipsized
  "... the Show i..." before the button, 2 info rows). Captures are full-window 3456 x 2158 PNGs (Retina); the file
  browser in them lists folder names of the home directory.
### U4.2 C6 / C7 TEMPORARY hook (scratch patch, separate build dir, never committed) -- DONE
- The worktree was never edited: a SOURCE COPY of 46d3ff2 (`git archive HEAD | tar -x` into scratchpad/ui-U4/hook-src),
  the hook written into the copy's src/MainComponent.cpp by scratchpad/ui-U4/mkhook.py, patch kept as
  scratchpad/ui-U4/hook.patch (79 lines: 2 includes + one block at the constructor end), configured into its own build
  dir scratchpad/ui-U4/hook-build (same flags as build-lane) and built with `--target AudioDNA -j3` (17:22:24 ->
  17:24:10 BUILD_EXIT=0; a normal cmake build, signed by the build's own 'Audio-DNA Dev' step -- no copied bundle).
- Hook: AUDIODNA_DEBUG_SHOW=cell-menu | cell-tooltip, AUDIODNA_DEBUG_SNAP=<png>; polls 250 ms until cell (0, 0)'s own
  getTooltip contains "frames per second", then a right-button mouseDown ON the cell (the AM10 menu path; PopupMenu
  parented to the top-level window) or the app's TooltipWindow::displayTip over the cell, then a synchronous
  createComponentSnapshot of the top-level window -> PngWrite::writeReplacing; dismisses after 1.5 s.
- Run (one locked batch 17:32:05-17:32:29, the hook app launched open -g --env ... --args --test-mode, pid recorded in
  the helper's file, quit by quit_app): raw "3 PASS / 0 FAIL" for cell-menu and for cell-tooltip; app-err:
  "DEBUG_SHOW cell-menu: snapshot .../C6-cell-menu.png 1728x1051 ok=1 cell=254,290 90x96" (same for C7).
  LOOKED: C6 = a menu under the H.264 cell, header "video_h264_64x64", item "Show in Finder", app LookAndFeel.
  C7 = the tooltip over the cell: "video_h264_64x64.mp4" / "H.264 High, 64 x 64, 30 frames per" / "second" /
  "Right-click: Show in Finder" -- JUCE's TooltipWindow WRAPS the 41-character line (see found_not_fixed F1).
- STRINGS CHECK (raw): `strings <hook app binary> | grep -c AUDIODNA_DEBUG_SHOW` = 1 (teeth);
  `strings build-lane/.../Audio-DNA | grep -c AUDIODNA_DEBUG_SHOW` = 0; G2b `grep -rn AUDIODNA_DEBUG_SHOW src tests` on
  the worktree = 0 lines (re-run at the end of U4, below).
### U4.3 Docs (AM16) -- DONE (e5d81c2)
- docs/claude/performance-controls.md: two paragraphs after "Deck tab row": "Deck rename in place (BF8)" (AM16 rules
  word for word: double-click the deck on screen; Enter / Tab / click-away keep; Esc discards; empty / unchanged keeps;
  one "Rename Deck" step; a double-click on a tab not on screen just shows it; right-click > Rename Deck... still the
  dialog; a click on the showing deck does nothing -- it must not rebuild the row; every close hands the keyboard home)
  and "Clip file info and Show in Finder (BF3)" (cell tooltip / menu, inspector rows + button, pictures / sequences /
  sources / missing, where the info comes from: VideoPlayer::open, never a probe or decode on the message / GL thread,
  Pitfalls 51 / 55 / 56 / 58; --test-mode records the reveal).
- docs/claude/pitfalls.md: NEW "65." (AM16's four points + the rebuild / load cases + guards). NUMBER: main already has
  64 (lane mkvidx, 649baf7); this branch (base eff2b1c) does not, so the merge will conflict at the end of pitfalls.md
  and at the CLAUDE.md index (64 + 65 both appended after 63): keep both, 64 first. Harmony may renumber.
- docs/claude/testing-eyes.md: one paragraph listing the eight TEST-ONLY routes (deck_tabs, deck_rename, tab_click,
  tab_dblclick, undo, clip_media, reveal_clip, inspect_clip) with their JSON, the message-thread ordering note (GET
  /api/composition is not ordered after the callAsync actions), the probe, and probe_deck_tab_dispatch (on-screen, not in
  ctest, Harmony only).
- .harmony/APP-INVENTORY.md: DeckView row (:58, deck tabs: click / showing-tab no-op / double-click rename / right-click
  menu), ClipCell row (:60, right-click "Show in Finder" menu on file clips, Ctrl+click unchanged, video / picture
  tooltip, sequence tooltip "images per second"), ClipInspector row (:72 of the Inspector table, the button + info rows),
  and the route list paragraph after s-rta-0930 bt2 (8 routes, the probe, probe_deck_tab_dispatch, ctest 1114 -> 1174).
- CLAUDE.md: UI bullet "**Deck tab row**: right-click a tab = its menu, never a deck switch; double-click the deck on
  screen = rename in place (...)" and index line "65. Rename box over a rebuilt row -- before a double-click or in-place
  editor." Paid by removing the "### Common Build Issues" pointer block (its four topics are rows of
  docs/claude/build-other-platforms.md's table, which the Trigger Table's build row routes to). Bytes 24,002 -> 24,001
  (net -1; bar "net growth <= 0" met). Main's CLAUDE.md is 23,976 B after mkvidx; the merged file = 23,975 B by
  arithmetic (INFERRED, not measured -- the merge is Harmony's).
## Gate list status (G0-G4; G1b is Harmony's) -- builder sanity runs on lane/ui e5d81c2, NOT the merged-tree gate
- G0 BUILD + WARNINGS: touch of the 15 listed files + `cmake --build build-lane -j3` 17:37:10 -> 17:38:01 BUILD_EXIT=0
  (45 TUs). Baseline = the U1 full build of eff2b1c (scratchpad/ui-U1/build-base.log, every TU compiled). Counted as
  UNIQUE warning lines per file (path:line:col:text, sort -u; raw counts repeat a header's warning per TU), base ->
  branch: VideoInfo.h 0 -> 0, VideoPlayer.h 0 -> 0, VideoPlayer.cpp 0 -> 0, ClipMediaText.h 0 -> 0, ClipCell.h 0 -> 0,
  ClipCell.cpp 0 -> 0, DeckTabRow.h 0 -> 0, DeckView.h 0 -> 0, DeckView.cpp 0 -> 0, ClipInspector.h 0 -> 0,
  ClipInspector.cpp 0 -> 0, MainComponent.h 1 -> 1, MainComponent.cpp 8 -> 7, ApiServer.h 0 -> 0, ApiServer.cpp 2 -> 2.
  Bar (branch <= base per file) holds on this count. Harmony's G0 (base = the merge base, same method) is the gate.
- G1 UNIT: direct runs 17:38:01, raw: test_video_info All tests passed (117 assertions in 17 test cases);
  test_clip_media_text (82 / 8); test_clip_cell_media (42 / 9); test_clip_inspector_media (53 / 8); test_deck_tab_rename
  (218 / 16); test_deck_tab_row (110 / 5); test_undo_commands (562 / 81); test_clip_inspector_paint_key (44 / 4) -- each
  "All tests passed". Full ctest -j3 17:38:02 -> 17:38:38, raw: "100% tests passed, 0 tests failed out of 1174".
  ctest -N DELTA: base eff2b1c 1114 -> branch 1174 (+60) = test_video_info 17, test_deck_tab_rename 16,
  test_clip_media_text 8, test_clip_cell_media 9, test_clip_inspector_media 8, +1 test_deck_tab_row (4 -> 5 TEST_CASEs,
  `git show eff2b1c:`), +1 test_clip_inspector_paint_key (3 -> 4); measured per executable from `ctest --show-only=
  json-v1`. `ctest -N | grep -c probe_deck` = 0. The base-vs-branch pass-list comparison needs a base ctest run: Harmony's.
  RED quotes for every new TEST_CASE are in the U1 / U3 / U2 sections above and their commit messages.
- G1b: NOT RUN (Harmony's, adoption 3). probe_deck_tab_dispatch is built (rebuilt in G0's touch build), never run.
- G2 LINTS 17:38: test_hot_thread_io_lint exit 0 (324 / 2), test_render_thread_lint exit 0 (19 / 2), test_log_line_lint
  exit 0 (58 / 3), test_shared_field_types exit 0 (1 / 1).
- G2b GREP: `grep -rn AUDIODNA_DEBUG_SHOW src tests` = 0 lines (17:38:38); strings: lane app 0, hook app 1.
- G3 LIVE: probe written; RED on main's app "1 PASS / 31 FAIL"; GREEN on the lane app "46 PASS / 0 FAIL" (--shots) and
  5 / 5 "32 PASS / 0 FAIL" (G3 rows only). Harmony re-runs it on the merged build (standalone, or UIFR_ATTACH=1 with the
  helper).
- G4 VISUAL: captures C1-C5, C8-C14 (green-lane-172814/*.png, 3456 x 2158, decoded) + NB1 / NB2 PASS; C6 / C7 hook
  snapshots (hook-173205/C6-cell-menu.png, C7-cell-tooltip.png, 1728 x 1051, decoded). NB3 = G1's AM9 (g)-(i) + AM7 (c)
  cases: PASS in the G1 direct runs above. C15 = G1b's P4 (Harmony). The critic panel and the matrix's "observed" column
  are Harmony's. Notes for the critic brief: (1) inspector "Show in Finder" at (w - 110, 4, 104, 20), the name stops at
  w - 114 (U2.4 deviation); (2) C5 at 27 decks: the "+" is already clamped under the 27th tab by DeckTabRow::layout and
  the open box covers both (pre-existing clamp); (3) C7: JUCE's TooltipWindow wraps the 41-character codec line ("30
  frames per" / "second") -- F1 below; (4) captures show the app in the background: no caret / focus outline in C2 / C3.
  Capture dirs (scratch): .../scratchpad/ui-U4/green-lane-172814 and .../scratchpad/ui-U4/hook-173205.

## Builds / tests (U4)
- No src change in U4 (probe + docs only); build-lane relinked by G0's touch rebuild (BUILD_EXIT=0). Hook build:
  separate source copy + separate build dir (U4.2), 17:22:24 -> 17:24:10 BUILD_EXIT=0. Disk 296 GiB free before it.
- Live batches (all through the helper, LANE=ui-U4, lock released after each): 17:25:19-17:25:31 RED (pre-fix probe),
  17:26:16-17:26:34 GREEN (pre-fix, R6 FAIL -> probe fix), 17:29:14-17:29:31 GREEN --shots, 17:30:16-17:31:20 RED final +
  GREEN x5, 17:32:05-17:32:29 hook C6 / C7. One wait on bf2-S1a's lock (17:28:14-17:29:14), never touched.
- Wrapper note: run-batch.sh printed "PROBE_EXIT=0" for red-final -- my echo read `$?` after a `$(date)` substitution
  (wrapper bug, not the probe); the probe's own exit on RED is 1 (run-probe.sh 17:25: "PROBE_EXIT=1"), and red-final's
  summary line is "1 PASS / 31 FAIL".

## Rig discipline (U4)
- Every app launch: open -g, --test-mode, started by the helper's start_app (or, for the hook build, my start_env that
  repeats start_app's refusals and records the pid in the helper's file), quit by quit_app ("app running after quit: no"
  every time). No foreign Audio-DNA ran at any start. outwins Output-named 0 before / during / after every batch;
  UserNotificationCenter windows 16 s after each batch's last quit: 0; app-err crash|assert lines 0.
- Window-only captures by Quartz window id (screencapture -x -o -l); no full-screen capture, no synthetic OS input (the
  hook's right-button mouseDown and displayTip are in-process calls inside the TEMPORARY build only), no debugger /
  sampler, no copied or re-signed bundle (the hook app is a normal cmake build, signed by its own build step), no Output
  window by any path, no TCC prompt seen, tests/visual never run.
- The worktree was never edited for the hook; .venv symlink never created (main .venv python by absolute path); worked
  only in the ui worktree + scratchpad; main checkout and lane mkvidx untouched; -j3 respected.
- Commit hook: every commit printed "[graphify hook] launching background rebuild" (a repo hook, not started by me). The
  harness stash-guard blocked one combined command (it read `git add -f .harmony/...` as a whole-tree add); re-run as
  separate commands staging the one file by path.
- Left on disk for Harmony (scratch, not committed): hook source copy scratchpad/ui-U4/hook-src, hook build
  scratchpad/ui-U4/hook-build (Audio-DNA.app with the hook), hook.patch, mkhook.py, the capture dirs.

## Notes for .harmony/notebook.md (U4)
- 2026-10-02 The debug POST routes are callAsync and GET /api/debug/deck_tabs is read ON the message thread, so it is
  FIFO after them; GET /api/composition is answered on the HTTP thread and races them (a redo read back as not yet
  applied). Read deck_tabs (or any message-thread GET) first as a barrier | discovered: .harmony/probe-ui-files-rename.sh
  names(), first GREEN R6.
- 2026-10-02 A TEMPORARY hook build without touching the worktree: `git archive HEAD | tar -x` into scratch, patch the
  copy, configure a separate build dir with the same FETCHCONTENT_SOURCE_DIR_* flags; building only `--target AudioDNA`
  took < 2 min (compiler cache) | discovered: scratchpad/ui-U4/hook-build.sh.
- 2026-10-02 DeckTabRow tabs reach the 60-px floor only at N >= (row - 24) / 63 decks (27 at a 1720-px row), not 12;
  at that count the "+" is clamped under the last tab | discovered: probe C5 / NB1.
- 2026-10-02 A PopupMenu / TooltipWindow parented to the top-level window is a CHILD component, so
  createComponentSnapshot of the top-level window captures it synchronously even in a background app (where JUCE
  dismisses the menu within ~50 ms) | discovered: hook C6 / C7.

## found_not_fixed (U4)
- F1 (should / nit, for the critic panel): the cell tooltip's codec line wraps in JUCE's TooltipWindow ("H.264 High, 64
  x 64, 30 frames per" / "second"; C7) -- the tooltip's max width cuts the 41-character line; a 1920 x 1080, 29.97 line is
  longer. Not a bar in G3 / NB3 (those pin the strings and the inspector widths); out of U4's scope (probe + docs).
- F2 (pre-existing): at the 60-px tab floor the "+" button sits under the last tab (DeckTabRow::layout clamps it to
  row - 24); with the rename box open on the last tab both are covered.

## PACKET QUALITY (U4)
- Clarity: HAD_TO_INFER (small). Inferred: (1) "the TEMPORARY C6 / C7 hook ... in a separate build dir" -- built from a
  scratch SOURCE COPY so the worktree never carried the hook; (2) G4 captures are Harmony's gate, but I added --shots /
  --hook to the probe and ran them once so the C-files exist and decode (critic panel untouched); (3) C5's deck count is
  derived from row_width (27), the ruling's "12 decks" does not give 60-px tabs at 1720 px (U3 hand-over); (4) Pitfall
  number 65 (64 is mkvidx's on main); (5) R8 checks the index +1 for all three ops.
- Missing context: the pre-change app has no TEST_SERVER debug routes, so the RED is 404-driven for every V / R row
  (expected: the routes are new).
- Unused context: plan U1-U3 bodies, Boris questions.
- Self-brief: the lane report (U1 / U3 / U2), plan-ui.md incl. HARMONY ADOPTION, ruling-ui.md (all read in full); the
  lock helper; probe-deck-tabs.sh (pattern); notebook plan6 entry (hook pattern); CLAUDE.md loaded by the harness.
  Knowledge tools: none (grep only).
INBOX-RECHECK: none

# LANE ui -- Fix round (lane-name ui-fix) -- builder report
STATUS: DONE (3 / 3 findings verified and fixed: 92d3897 F-MUST-1, 30bd378 F-SHOULD-2, 4f6d5f8 F-SHOULD-3, + this report)
Base for the fix round: 24e1f00 (lane/ui after U4). Lane base: eff2b1c. Findings: F-MUST-1 (probe standalone ENVS
unbound under bash 3.2), F-SHOULD-2 (no test for "focus home BEFORE the hide"), F-SHOULD-3 (G3 V4 inspector rows for 2
of 8 cells only).
## Items
### F-SHOULD-2 focus home BEFORE the hide / focus elsewhere left there -- FIXED (30bd378)
- VERIFIED the finding: headless, keyboard focus is always nowhere (JUCE grabKeyboardFocusInternal returns unless
  isShowing(), juce_Component.cpp:2654 in the 8.0.4 source), and the Rig only did ++closes. Mutants from a SOURCE COPY
  (scratchpad/ui-fix/mut-src, own build dir mut-build, worktree never edited) run against the 24e1f00 test file:
  M1 (onRenameClosed moved after setVisible(false)) "All tests passed (218 assertions in 16 test cases)";
  M2 (the `focused == nullptr || isParentOf(focused)` test deleted) "All tests passed (218 assertions in 16 test cases)".
- Fix: DeckView::setFocusedComponentForTests(std::function<juce::Component*()>) -- the focus finishRename reads (empty =
  juce::Component::getCurrentlyFocusedComponent(), the shipped path; 1 changed line in DeckView.cpp). The Rig records
  the box's isVisible() at each onRenameClosed; CHECK(r.closedBeforeHide()) in every (e) section, (f) and (i). New
  TEST_CASE (k): focus on a component outside DeckView -> the focus-loss close keeps the name and calls NO
  onRenameClosed; focus on the box, on a tab, or nowhere -> called once each.
  The second half of the finding's fix ("a focused non-DeckView component") cannot be done headless without a window;
  the seam is the cheapest way (precedent: ClipCell::setMenuLauncherForTests, ruling AM10). An off-screen window was
  rejected (a window from ctest; screen-safety).
- RED (new test file vs the mutants, raw): M1 "test cases:  17 |  13 passed | 4 failed" ((e), (f), (i), (k));
  M2 "test cases:  17 |  16 passed | 1 failed" ((k)); M0 unmutated copy "All tests passed (259 assertions in 17 test cases)".
- GREEN (build-lane, raw): "All tests passed (259 assertions in 17 test cases)".
- numstat: src/ui/DeckView.cpp 1/1, src/ui/DeckView.h 4/0, tests/test_deck_tab_rename.cpp 55/1.
### F-MUST-1 probe standalone mode aborts under bash 3.2 -- FIXED (92d3897)
- VERIFIED: /bin/bash is "GNU bash, version 3.2.57(1)-release"; `set -u; ENVS=(); echo "${ENVS[@]}"` -> "ENVS[@]:
  unbound variable" rc 127. On the real UNFIXED probe (worktree 24e1f00, sha256 b552ca31..., git diff 0 lines), default
  standalone mode (no UIFR_ATTACH, no --hook), lock held 18:13:13, raw:
    ".harmony/probe-ui-files-rename.sh: line 93: ENVS[@]: unbound variable"  PROBE_EXIT=1
  (it aborted before `open`: Audio-DNA before / after '' ; outwins 0 Output-named).
- Fix (1 line + 1 comment line): `${ENVS[@]+"${ENVS[@]}"}`. Shim check under /bin/bash with `open` as a printing
  function: no hook -> [-g][--stdout][/x/app-out.log][--stderr][/x/app-err.log][/a b/App.app][--args][--test-mode];
  --hook cell-menu -> the two --env pairs as separate words, an element with a space kept whole. rc 0 both.
- GREEN: the FIXED probe in standalone mode on build-lane's app (it launched `open -g ... --test-mode` itself, pid 21914,
  and quit only that pid), 18:18:45-18:18:52, raw: "39 PASS / 0 FAIL" ... "app running after quit: no".
- numstat 92d3897: .harmony/probe-ui-files-rename.sh 2/1.
### F-SHOULD-3 G3 V4 inspector rows for 2 of 8 cells -- FIXED (4f6d5f8)
- VERIFIED (probe 24e1f00 lines 285-290): V4 inspected L0C0 and L1C3 only; --shots inspected 7 cells but only printed.
- Fix: the two V4 rows unchanged (bars not touched); added "V4 inspect L1C3 (source) shows the clip" (inspector_shows
  true) and one row per other cell (L0C1 L0C2 L0C3 L1C0 L1C1 L1C2): POST inspect_clip, then GET clip_media (a
  message-thread read, FIFO after the callAsync select; video cells through the pre-registered null-video retry) ->
  inspector_shows true AND inspector_lines == V1's lines AND inspector_button_visible true. G3 rows 32 -> 39.
- RED (main's PRE-CHANGE app via the helper, UIFR_ATTACH=1, 18:18:33-18:18:41), raw: "1 PASS / 38 FAIL"; every V4 row
  FAIL `{"inspector_shows": null, "inspector_lines": null, "inspector_button_visible": null}` (no debug routes before
  this lane -- the same 404-driven RED convention as U4).
- GREEN (build-lane, standalone, same run as F-MUST-1), raw "39 PASS / 0 FAIL"; the new rows' observed values:
  L0C1 ["ProRes 422 HQ", "64 x 64, 29.97 frames per second"], L0C2 ["HAP Q", "64 x 64, 60 frames per second"],
  L0C3 ["HEVC Main 10", "64 x 64, 23.976 frames per second"], L1C0 ["PNG image"], L1C1 ["Image sequence, 3 images"],
  L1C2 ["File missing"], each inspector_shows true, button true, retries 0; L1C3 shows true, lines [], button false.
- numstat 4f6d5f8: .harmony/probe-ui-files-rename.sh 10/0.
## Builds / tests (fix round)
- build-lane (existing; disk 292 GiB free): `cmake --build build-lane -j3` 18:14:28 -> 18:14:55 BUILD_EXIT=0 (DeckView.cpp
  recompiled into AudioDNA + tests, app relinked); 0 warning lines naming DeckView.* or test_deck_tab_rename.cpp. No
  CMakeLists change (no reconfigure needed).
- Mutant build: source copy scratchpad/ui-fix/mut-src (rsync of the working tree minus build-lane / .git), own build dir
  scratchpad/ui-fix/mut-build (same flags), target test_deck_tab_rename only, 18:15:39 -> 18:17:23. The worktree's
  DeckView.cpp sha256 0b654d88... before and after; it was never edited for a mutant.
- G1 direct runs at HEAD 4f6d5f8 (18:19:53), raw: test_video_info "All tests passed (117 assertions in 17 test cases)";
  test_clip_media_text (82 / 8); test_clip_cell_media (42 / 9); test_clip_inspector_media (53 / 8); test_deck_tab_rename
  "All tests passed (259 assertions in 17 test cases)"; test_deck_tab_row (110 / 5); test_undo_commands (562 / 81);
  test_clip_inspector_paint_key (44 / 4).
- G2 lints, raw: test_hot_thread_io_lint exit 0 (324 / 2), test_render_thread_lint exit 0 (19 / 2), test_log_line_lint
  exit 0 (58 / 3), test_shared_field_types exit 0 (1 / 1).
- G2b: `grep -rn AUDIODNA_DEBUG_SHOW src tests` = 0 lines; `strings` lane app binary = 0.
- Full ctest SERIAL (no -j) 18:20:01 -> 18:21:56, raw: "100% tests passed, 0 tests failed out of 1175" (Total Test time
  (real) = 115.12 sec). ctest -N 1174 -> 1175 (+1 = TEST_CASE (k)); `ctest -N | grep -c probe_deck` = 0.
## Rig discipline (fix round)
- Two locked batches (LANE=ui-fix, the helper): 18:13:13-18:13:16 (the RED standalone run; it aborted before any
  launch) and 18:18:29-18:18:52 (RED on main's app via start_app / quit_app, then the fixed probe standalone, which
  refuses if any Audio-DNA runs or 7070 / 8080 listen and quits only its own pid). The 45-s cooldown held (5 min
  between). No foreign Audio-DNA at any start; "Audio-DNA after: ''" after each run; outwins Output-named 0 before /
  during / after; UserNotificationCenter windows 16 s after the last quit: 0; app-err crash|assert lines 0.
- open -g --test-mode only; no screen capture this round; no synthetic input; no debugger / sampler; no copied or
  re-signed bundle; no Output window by any path; tests/visual never run; no TCC prompt seen; no .venv symlink (main
  .venv python by absolute path via UIFR_PY); -j3; only the ui worktree + scratchpad touched (main checkout and lane
  mkvidx untouched). Commits staged by path (item 1's hunk via `git apply --cached` of its saved patch, so items 1 and 3
  are separate commits of the same file).
## Notes for .harmony/notebook.md (fix round)
- 2026-10-02 macOS /bin/bash is 3.2: under `set -u` an EMPTY array expansion "${A[@]}" aborts ("unbound variable");
  use ${A[@]+"${A[@]}"}. A probe mode the lane never runs (here: standalone) is where it hides -- run every documented
  entry mode once | discovered: .harmony/probe-ui-files-rename.sh:94.
- 2026-10-02 Headless JUCE (no peer) can never hold keyboard focus (grabKeyboardFocusInternal returns unless
  isShowing()), so any code branching on getCurrentlyFocusedComponent() needs a focus seam to be unit-tested; also
  record STATE AT CALLBACK TIME (e.g. the box's visibility inside onRenameClosed) to pin call order | discovered:
  src/ui/DeckView.cpp finishRename, tests/test_deck_tab_rename.cpp (k).
## found_not_fixed (fix round)
- none new. Carried from U4 (unchanged): F1 tooltip codec line wraps in JUCE's TooltipWindow; F2 "+" under the last
  60-px tab (pre-existing clamp).
## PACKET QUALITY (fix round)
- Clarity: CLEAR for F-MUST-1 and F-SHOULD-3; HAD_TO_INFER for F-SHOULD-2's second half ("a focused non-DeckView
  component" cannot exist headless) -> added the smallest seam (setFocusedComponentForTests, empty = shipped behaviour).
- Missing context: none.
- Unused context: plan U1-U3 bodies, the Boris questions, the click-target matrix (read, not needed for these fixes).
- Self-brief: plan-ui.md incl. HARMONY ADOPTION and ruling-ui.md (read in full), the lane report U1-U4, the lock helper,
  run-batch.sh / run-probe.sh / hook-build.sh (U4 patterns). Knowledge tools: none (grep only).
INBOX-RECHECK: none
