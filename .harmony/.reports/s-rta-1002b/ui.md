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
STATUS: PENDING (stage U3 of 4, in progress; U1 above is DONE and unchanged)
Base for U3: 9af61b2 (lane/ui after U1).
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
### U3.4 TEST-ONLY REST (AM6) -- PENDING
### U3.5 probe_deck_tab_dispatch (AM8; BUILT, NEVER RUN) -- PENDING
## Builds / tests (U3)
## Rig discipline (U3)
## Notes for .harmony/notebook.md (U3)
## Next stage notes (U3 -> U2)
## PACKET QUALITY (U3)
