# PLAN lane ui — BF3 (video codec line + Show in Finder) and BF8 (double-click a deck tab to rename it)
Session s-rta-1002b · Architect (Fable) · base main 5e47d17 · build slot: after bf9b and bf6 (pre-registered order)
NOTE ON A NEWER RULING (read on disk mid-plan): BF9 was CLARIFIED at 14:44:36 — "decks are boxes of clips; switching
decks never changes what plays" (BORIS_DECISIONS.md:350-355; binding-decisions.md:598-606; lane renamed bf9b). This
plan is written against bf9b (it only matters for BF8's "double-click on a tab that is not showing", fork F-B1).

====================================================================================================================
## 1. GOAL
====================================================================================================================
BF3: every video clip shows what it is (codec, size, frame rate) in the Clip inspector and the clip cell's hover
tooltip, and the file is one click from Finder (clip cell right-click menu + a button in the Clip inspector).
  Boris (verbatim): "Codec display for each video and easy access to that video in finder"
  Defaults taken, no objection (binding-decisions.md:582): codec + resolution + fps in the Clip inspector and the cell
  tooltip; "Show in Finder" in the cell's right-click menu and a button in the Clip inspector.
BF8: double-click a deck tab = rename it in place (Enter saves, Esc cancels), with one undo step.
  Boris (verbatim): "Need to be able to rename each deck with double click"

====================================================================================================================
## 2. ESTABLISHED FACTS (VERIFIED = read / run, cited; INFERRED = reasoned, says why)
====================================================================================================================
### BF8 — the deck tab row
E1 VERIFIED. A tab is `DeckView::DeckTabButton` (a juce::TextButton): right-click / Ctrl+click (isPopupMenu) opens
   its menu in mouseDown and never reaches the base class (src/ui/DeckView.h:131-145). A left click switches decks via
   onClick -> onDeckSwitched(index) (src/ui/DeckView.cpp:489-494). No double-click handling exists anywhere in
   src/ui except FilesBrowser (grep `mouseDoubleClick`: only src/ui/FilesBrowser.cpp:101).
E2 VERIFIED. Today's rename: tab right-click > "Rename Deck..." (src/ui/DeckTabRow.h:40-51) -> MainComponent::renameDeck
   (src/MainComponent.cpp:3674-3702): a modal AlertWindow; trimmed text; empty -> nothing; unchanged -> nothing; else
   pushes RenameDeckCmd ("Rename Deck") and calls deckView_->refresh() to relabel. The Deck menu has Rename for the active
   deck too (MainComponent.cpp:6635).
E3 VERIFIED. RenameDeckCmd is index-keyed, command-owns-the-mutation, a stale index is a no-op, no fence (the GL thread
   never reads deck.name) (src/core/DeckCommands.h:1079-1110); unit test exists (tests/test_undo_commands.cpp:1636-1654).
E4 VERIFIED. EVERY tab click rebuilds the tab row, even a click on the deck already showing: onDeckSwitched
   (MainComponent.cpp:1411-1442) -> handleDeckSwitch -> `deckView_->rebuildGrid()` unconditionally
   (MainComponent.cpp:5550-5551) -> setupDeckTabs -> `deckTabs_.clear()` (DeckView.cpp:124, :471-473). So the clicked
   button is destroyed inside its own onClick.
E5 VERIFIED (JUCE 8.0.4 source). (a) JUCE delivers mouseDoubleClick AFTER mouseUp (where a TextButton fires onClick)
   and only to a component that survived it: `mouseUp(me); if (checker.shouldBailOut()) return; ... if
   (me.getNumberOfClicks() >= 2) { if (checker.nearestNonNullParent() == this) mouseDoubleClick(...)`
   (build/_deps/juce-src/modules/juce_gui_basics/components/juce_Component.cpp:2253-2270, HierarchyChecker :57-100).
   (b) The click count is kept per mouse SOURCE, not per component: same peer, <400 ms, <8 px, same buttons
   (detail/juce_MouseInputSourceImpl.h:405-421, :553-562; juce_MouseEvent.cpp:142 timeout 400 ms). CONSEQUENCE: with
   today's code a double-click on a tab can never reach the tab (the second click's onClick rebuilds the row first); a
   tab that is NOT rebuilt by its click does receive it, and so does a freshly rebuilt tab at the same place.
E6 VERIFIED. Keyboard focus on a click: JUCE moves focus to the first ancestor that wants it, BUT stops without moving
   it when an ancestor already contains the focused component (juce_Component.cpp:2649-2680). DeckView does not want
   focus; juce::Button does (juce_Button.cpp:89); ClipCell / LayerStrip areas do not. CONSEQUENCE: an editor inside
   DeckView keeps focus when the user clicks a clip cell, a strip or empty grid space -> "click-away" cannot rely on
   focus loss alone. Clicks on any button, or anywhere outside DeckView (MainComponent wants focus,
   MainComponent.cpp:2306), do move focus.
E7 VERIFIED. A focused juce::TextEditor consumes text keys, Return (onReturnKey) and Escape (onEscapeKey)
   (juce_TextEditor.cpp:2153-2196) and Cmd+Z / Cmd+Shift+Z as ITS OWN undo (juce_TextEditorKeyMapper.h:121-125). Its
   Escape test is modifier-blind (`key.isKeyCode(escapeKey)`, :2175) -> it would swallow Cmd+Shift+Esc, the output
   PANIC key (MainComponent.cpp:3839-3858, output::classifyOutputKey src/output/OutputMenuModel.h:95-97). Its
   keyStateChanged returns false on a key RELEASE (:2198-2201) -> the release bubbles to MainComponent::keyStateChanged,
   which fires the momentary-release action of EVERY momentary keyboard binding whose key is not down
   (MainComponent.cpp:3930-3955); a momentary release clears the layer when the pad's clip is active
   (MainComponent.cpp:7599-7607). True today for every TextEditor in the app (TopBar BPM field, browser search boxes,
   RecordPanel editors).
E8 VERIFIED. The app's in-place-rename precedent commits on focus loss: LayerInspector's name label
   `setEditable(false, true, false)` (lossOfFocusDiscardsChanges = false), src/ui/LayerInspector.cpp:17, :725-733.
E9 VERIFIED. Deck names are saved (Deck.h:168 toVar, :184 fromVar), already pinned by a save / load round trip
   (tests/test_composition.cpp:853-867); `sourceFile` is not saved (Deck.h:18). Save Deck keeps
   its file link only while the file's base name equals the deck name — "a Rename breaks the link"
   (MainComponent.cpp:3598-3613): after ANY rename, Save Deck falls through to Save Deck As... (unchanged by this plan).
E10 VERIFIED. A composition swap rebuilds the grid via refreshUiAfterModelSwap (MainComponent.cpp:2924-2960; called
   from swapCompositionModel :2988-3031); deck ids are unique per composition only (Pitfall 36) — a newly loaded
   composition may reuse the id of a deck being renamed.
E11 VERIFIED. TextEditor colours already come from the app LookAndFeel (background kSurface, outline kPanelBorder,
   focused outline kAccentCyan, highlight cyan 30 %: src/ui/LookAndFeel.cpp:49-53).
E12 VERIFIED. Pure, tested tab-row geometry lives in src/ui/DeckTabRow.h (layout :20-33) with tests/test_deck_tab_row.cpp;
   headless DeckView tests already exist under ScopedJuceInitialiser_GUI (tests/test_deck_thumbnails.cpp:1-60).

### BF3 — codec info and Finder
E13 VERIFIED. ClipCell has NO right-click menu today: `if (event.mods.isRightButtonDown()) return;`
   (src/ui/ClipCell.cpp:222-223); APP-INVENTORY.md:60 "Right-click: no-op". A Ctrl+left-click is not a right button,
   so today it triggers / selects like a left click (ClipCell.cpp:225-233). Only image-sequence cells have a tooltip
   ("Image sequence — N images at X.X images/sec", ClipCell.cpp:374-385). ClipCell is a SettableTooltipClient
   (ClipCell.h:13-16); TooltipClient::getTooltip is virtual (juce_TooltipClient.h:57, :89); the app TooltipWindow polls
   the hovered component's getTooltip every 123 ms (juce_TooltipWindow.cpp:54, :206-221).
E14 VERIFIED. "Show in Finder" already exists twice, both `juce::File::revealToUser()` on the message thread
   (src/ui/CompDecksBrowser.cpp:224, :248; src/ui/RecordPanel.cpp:63-68). macOS JUCE: if the file exists it is
   selected in Finder; else if its parent folder exists, the folder is revealed; else nothing
   (juce_core/native/juce_Files_mac.mm:457-465).
E15 VERIFIED. Every video already passes through VideoPlayer::open(), which has the stream's codec parameters in hand:
   codec id / tag (codecpar), width / height (codecCtx_), avg / r frame rate (src/media/VideoPlayer.cpp:113-155).
   frameRate_ falls back to a made-up 30.0 when the stream has no rate (:149-154) — the info must not show that.
   open() runs before start() on the message thread (drop, replace, reopen) or on MediaOpener's pool thread on an
   unpublished player (VideoPlayer.h:40-44; src/core/MediaOpener.cpp:27-54); width_/height_/frameRate_ are "written in
   open(), read-only after" (VideoPlayer.h:168-176). No codec name is kept today.
E16 VERIFIED. The message thread reaches a clip's player by id: Renderer::getVideoPlayer(clipId) under
   videoPlayerMutex_ (map lookup only; src/render/Renderer.cpp:1741-1748) and getVideoPlayerFile (:1750-1757; the
   player's getFile() is an unlocked read, safe because every caller is message-thread confined, VideoPlayer.h:59-64).
   A player is retired only on the message thread (closeMediaForClip) and destroyed on the GL thread after drain, so a
   pointer got on the message thread is valid for that call (Renderer.cpp:1512-1540; Pitfall 56).
E17 VERIFIED. Player properties are copied onto Clip only at 3 sites — staged-load landing (MainComponent.cpp:3167-3185),
   drop (:5271-5280), Replace Content (:7008-7016) — and NOT at the 2 reopen sites (:4742 preview reopen, :5039 the
   undo media-attach hook, which receives `const Clip&`). Pitfall 58: "never write a landed value into a live clip".
E18 VERIFIED (ran ffprobe 8.0 on the repo fixtures + scratch encodes, scratchpad/bf3/): codec_name / profile /
   FourCC are: h264 "High" avc1 (tests/fixtures/video_h264_64x64.mp4, 30/1); hap Hap5 = HAP Alpha
   (tests/fixtures/video_hapa_64x64.mov); hap Hap1 = HAP, HapY = HAP Q; prores "HQ" apch (30000/1001), prores "4444"
   ap4h; hevc "Main 10" hev1 (24000/1001); mpeg4 "Simple Profile" / "Advanced Simple Profile"; rawvideo RGBA. The local
   ffmpeg 8.0 (/opt/homebrew/bin) has the prores_ks, hap, libx265 encoders, so tiny fixtures can be made.
E19 VERIFIED. ClipInspector layout: name bar 28 px with the name drawn `withTrimmedRight(40)` (an empty reserved strip)
   (src/ui/ClipInspector.cpp:469-476); then the Dashboard (global MacroBank) and sections; paint(), resized() and
   getPreferredHeight() each track y by hand (:479, :573-583, :1034-1044); paint repaints only when PaintKey changes
   ("add, never remove", ClipInspector.h:60-75, .cpp:981-1017). InspectorPanel re-sizes the inspector to its preferred
   height on every 10 Hz refresh (src/ui/InspectorPanel.cpp:188-196), so a conditional row is absorbed.
E20 VERIFIED. UI Text Rules: whole words only, no abbreviations, in labels, buttons, menus AND tooltips
   (docs/claude/architecture.md:360-362) -> "frames per second", not "fps". Pitfall 6: ASCII in small buttons.
E21 VERIFIED. TEST-ONLY REST routes are compiled only with AUDIODNA_TEST_SERVER (src/api/ApiServer.cpp:299-329); a
   UI read is done ON the message thread with a 2 s wait (handleDebugUiText, :2013-2042); MainComponent wires them in
   its constructor (MainComponent.cpp:2141-2170). Test mode suppresses UI side effects by `testMode_`
   (MainComponent.h:653; precedent MainComponent.cpp:3666). No REST undo, inspect or rename route exists today.
E22 VERIFIED. A JUCE PopupMenu is dismissed within ~50 ms when the app is not frontmost, so menu states are shot by a
   TEMPORARY, never-committed hook (AUDIODNA_DEBUG_SHOW) via a component snapshot (.harmony/probe-deck-tabs.sh:16-26).
E23 INFERRED (reasoned from E16 + Pitfall 56's "videoPlayerMutex_ guards the map lookup only"): one extra lookup per
   inspector selection and one per tooltip poll (8/s, only while the pointer rests on a video cell) is negligible
   next to the GL thread's per-frame lookups. Witness kept: /api/state msg_video_lock_wait_max_ms (Renderer.cpp:1743).

====================================================================================================================
## 3. DESIGN FORKS (choice first; the runner-up and why it loses)
====================================================================================================================
F-B1 (BF8) What a double-click on a tab does, given a single click must stay an instant deck switch.
   CHOSEN: single click unchanged (switch at once on release); a click on the ACTIVE tab becomes a true no-op (no
   onDeckSwitched, no rebuild); a left double-click on any tab opens the rename box on that tab. On a tab that is not
   showing, the double-click's first click shows that deck (as any click does) and the second opens the box over the
   same tab — no extra switch, no switch-back. Under bf9b a switch changes only which clips the grid shows, so the
   "switch then rename" costs nothing that plays (BORIS_DECISIONS.md:350-355). Boris Q1 confirms.
   - Runner-up: delay every single-click switch by the double-click window (400 ms, E5b) so a double-click never
     switches. LOSES: every deck switch in a live set gets a 0.4 s lag to serve a rare prep-time action.
   - Rejected: switch, then switch BACK on the second click. Two rebuilds (a visible flash) and, before bf9b lands, a
     destructive round trip. Rejected: double-click renames only the active tab. Boris said "each deck"; a double-click
     that does nothing on most tabs reads as broken.
   - Why the active-tab no-op is load-bearing (E4 + E5a): without it the second click's onClick rebuilds the row and
     destroys the button before JUCE delivers mouseDoubleClick. It is placed in DeckView (the tab's onClick), not in
     MainComponent::onDeckSwitched, so bf9b's rework of the switch path cannot remove it.
F-B2 (BF8) Where the editor lives.
   CHOSEN: ONE juce::TextEditor subclass owned by DeckView (created once, hidden; Pitfall 34), laid over the tab,
   bound to the deck's ID (not index), re-positioned after every rebuild. LOSES: an editor inside DeckTabButton dies on
   every rebuild (E4) — a REST / MIDI / genre deck switch mid-typing would eat the edit. LOSES: a juce::Label in place
   of the button changes the tab's click semantics.
F-B3 (BF8) What ends an edit.
   CHOSEN (Finder / LayerInspector convention, E8): Enter, click-away, Tab (INFERRED: an unconsumed Tab moves
   focus in JUCE), app switch = KEEP; Esc = discard; empty or
   unchanged text = keep the old name (no undo step). Click-away = focus loss (E6: clicks outside DeckView or on any
   button) PLUS a DeckView-wide outside-click listener (E6: clicks on cells / strips / empty grid inside DeckView do
   not move focus). LOSES: focus loss alone (misses the most common click-away, on the grid). LOSES: click-away =
   discard (loses typing; contradicts the app's own LayerInspector behaviour).
F-B4 (BF8) Right-click > "Rename Deck..." stays the dialog.
   CHOSEN: unchanged; both routes share one funnel (MainComponent::applyDeckRename). Runner-up: make the menu item open
   the in-place box too (Finder does). LOSES for now: an unrequested change to a working, screenshot-tested flow
   (probe-deck-tabs 06-rename-dialog); trivial to switch later.
F-U1 (BF3) Where the codec info comes from.
   CHOSEN: captured inside VideoPlayer::open() (zero extra I/O — the format context is already open), kept as an
   immutable VideoInfo on the player, read on the message thread through the player that is actually open for that
   clip id and file (E15, E16). No probe, no decode on the message or GL thread (Pitfalls 51 / 53 / 56 / 58).
   - Runner-up: copy the info onto Clip at the 3 adopt sites, like thumbnail / hasAlpha (E17). LOSES: the 2 reopen
     paths never copy (a clip whose file came back would never get a codec line) and fixing them means writing into a
     LIVE clip (Pitfall 58); it also adds a field to Clip.h, which bf9b / bf6 / bf1 all touch.
   - Rejected: a separate off-thread probe cache keyed by path (the ClipThumbnails shape). Duplicates every open's
     avformat_find_stream_info I/O and can disagree with what actually plays; its one extra (codec of a file that failed
     to open) is rare with Homebrew FFmpeg.
F-U2 (BF3) Where the inspector shows it.
   CHOSEN: "Show in Finder" button at the right of the name bar (the strip the name bar already reserves, E19) + ONE
   info row under the name, both only for file-backed clips (video, picture, image sequence). Runner-up: the first row
   of the "Video" section. LOSES: that section is about how the clip is drawn (opacity / size / blend) and is shown for
   every clip type; the file's identity belongs with the name. Rejected: a two-line name bar for every clip (an empty
   line on sources).
F-U3 (BF3) What the right-click on a cell does.
   CHOSEN: right-click or Ctrl+click on a cell whose clip has a file -> a menu headed by the clip name with "Show in
   Finder". Every other right / Ctrl click is exactly as today (right: nothing; Ctrl+left on a cell without a file:
   trigger / select). Runner-up: open the menu on every popup click, even with nothing in it. LOSES: a one-row disabled
   menu on sources, and Ctrl+click on empty cells would stop triggering.
F-U4 (BF3) Pictures, sequences, sources, missing files.
   CHOSEN: picture = "PNG image" (kind from the extension, no decode); sequence = "Image sequence, N images", Show in
   Finder selects its first image; procedural source / camera / effects-only = no row, no button, no tooltip change;
   missing file = "File missing" (the clip's existing presence flag, Clip.h mediaMissing — no stat), Show in Finder
   then reveals the folder it was in (JUCE, E14), and if even that folder is gone the status line says so.
F-U5 (BF3) Test mode.
   CHOSEN: under --test-mode the reveal is RECORDED (path + count, read by REST), never sent to Finder (no window
   pops on Boris's screen during gates; precedent E21). The one real Finder call is checked by Boris (section 8).

====================================================================================================================
## 4. ITEMS (files + functions, behaviour, RED-first test, GREEN bar, risk) + BUILD STAGES
====================================================================================================================
RED convention: a test that compiles on main 5e47d17 and fails there is marked RED(main). A test of brand-new API is
RED(stub): the builder first lands the declaration with an empty body, runs the test, sees it fail on its
assertions, then implements.

---------------------------------------- STAGE U1 — BF3 data (no visible change; disjoint from U3) -----------------
U1.1 NEW src/media/VideoInfo.h (pure, std only; no JUCE, no FFmpeg includes)
   struct VideoInfo { std::string codec; int width = 0, height = 0; double fps = 0.0;  // 0 = the stream has no rate
                      bool known() const { return !codec.empty(); } };
   namespace videoinfo {
     std::string codecLabel(std::string_view ffName, uint32_t fourcc, std::string_view profile);
     std::string fpsText(double fps);      // round to 3 decimals, strip trailing zeros: 29.97 / 23.976 / 59.94 / 30 / 25
     std::string describe(const VideoInfo&); // "H.264 High, 1920 x 1080, 29.97 frames per second"; fps 0 -> no rate part
   }
   codecLabel table (ffName = avcodec_get_name; fourcc = codecpar->codec_tag as FFmpeg stores it, MKTAG order;
   profile = avcodec_profile_name or ""):
     h264 -> "H.264" + " " + profile ("H.264 High")          hevc -> "HEVC" + " " + profile ("HEVC Main 10")
     prores -> "ProRes " + {Proxy:"422 Proxy", LT:"422 LT", Standard:"422", HQ:"422 HQ", 4444:"4444", XQ:"4444 XQ"};
               no profile -> by FourCC apco/apcs/apcn/apch/ap4h/ap4x (same names); neither -> "ProRes"
     hap -> by FourCC Hap1 "HAP", Hap5 "HAP Alpha", HapY "HAP Q", HapM "HAP Q Alpha", HapA "HAP Alpha Only"; else "HAP"
     dxv "DXV", mjpeg "Motion JPEG", mpeg4 "MPEG-4 Part 2", mpeg2video "MPEG-2", qtrle "QuickTime Animation",
     png "PNG", rawvideo "Uncompressed", vp8 "VP8", vp9 "VP9", av1 "AV1", cfhd "CineForm",
     dnxhd -> "DNxHD", or the profile when it starts with DNXHR ("DNXHR HQ" -> "DNxHR HQ")
     anything else -> the FFmpeg short name upper-cased ("weird" -> "WEIRD"); empty ffName -> "" (unknown).
   TEST tests/test_video_info.cpp [videoinfo]: one CHECK per table row incl. the fallbacks; fpsText(30000/1001.0)
   == "29.97", (24000/1001.0) == "23.976", (60000/1001.0) == "59.94", 25.0 == "25", 0.0 == ""; describe of
   {"H.264 High",1920,1080,30000/1001.0} == "H.264 High, 1920 x 1080, 29.97 frames per second"; fps 0 -> "H.264 High,
   1920 x 1080". RED(stub). GREEN: all pass. Risk: low (pure).
U1.2 src/media/VideoPlayer.h / .cpp — keep the info.
   VideoPlayer.h: in the "Video properties (valid after open)" accessor block (:66-71) add
     `const VideoInfo& getInfo() const { return info_; }   // BF3: written in open(), read-only after (message thread)`
   and the member `VideoInfo info_;` in the "written in open(), read-only after" block (:168-176).
   VideoPlayer.cpp open(): immediately AFTER the frame-rate block (:149-155), before `pol_` (:161):
     info_.width = width_; info_.height = height_;
     info_.fps = avg_frame_rate valid ? av_q2d(avg) : r_frame_rate valid ? av_q2d(r) : 0.0;   // never the 30.0 fallback
     const char* prof = avcodec_profile_name(codecpar->codec_id, codecpar->profile);
     info_.codec = videoinfo::codecLabel(avcodec_get_name(codecpar->codec_id), codecpar->codec_tag, prof ? prof : "");
   Nothing else in VideoPlayer changes (no decode / upload / ring code; the decode and GL threads never read info_).
   Merge note: lane mkvidx edits open() at :275-304 and VideoPlayer.h :279-284 — disjoint hunks.
   NEW fixtures (tests/fixtures/, commands recorded in the test file header, sizes from the scratch run):
     video_prores_hq_64x64_2997.mov  ffmpeg -f lavfi -i testsrc2=s=64x64:r=30000/1001:d=0.2 -c:v prores_ks -profile:v 3
                                     (20,345 B; ffprobe prores HQ apch 30000/1001)
     video_hapq_64x64_60.mov         ffmpeg -f lavfi -i testsrc2=s=64x64:r=60:d=0.2 -c:v hap -format hap_q
                                     (25,312 B; hap HapY 60/1)
     video_hevc_main10_64x64_23976.mp4 ffmpeg -f lavfi -i testsrc2=s=64x64:r=24000/1001:d=0.2 -c:v libx265
                                     -pix_fmt yuv420p10le -x265-params log-level=none (4,529 B; hevc Main 10 24000/1001)
   TEST tests/test_video_info.cpp [videoinfo][open] (in-process VideoPlayer::open of VALID fixtures only — the
   cut-header file stays in its child-process test): video_h264_64x64.mp4 -> codec "H.264 High", 64 x 64, fps 30.0
   exactly; video_hapa_64x64.mov -> "HAP Alpha"; prores -> "ProRes 422 HQ", fpsText "29.97"; hapq -> "HAP Q", "60";
   hevc -> "HEVC Main 10", "23.976"; video_mpeg4_bf2_64x64.avi -> "MPEG-4 Part 2". Each player closed, never started.
   CMake target test_video_info: the link recipe of test_video_player_open (tests/CMakeLists.txt:2855+).
   RED(stub: getInfo returns a default VideoInfo). GREEN: 6/6. Risk: a container whose codecpar->profile is unknown
   -> label without the profile (still correct); covered by the table's fallback rows.

---------------------------------------- STAGE U3 — BF8 (independent of U1; can ship alone) ------------------------
U3.1 Active-tab click = no-op. src/ui/DeckView.h/.cpp
   New public `void tabClicked(int deckIndex);` (also the REST hook's target). The tab's onClick (DeckView.cpp:490-493)
   becomes `[this, capturedIdx] { tabClicked(capturedIdx); }`; tabClicked: `if (composition_ == nullptr || deckIndex ==
   composition_->activeDeckIndex) return; if (onDeckSwitched) onDeckSwitched(deckIndex);`. Add `int tabRowBuilds_`
   (++ in setupDeckTabs) + `int tabRowBuilds() const` (witness).
   TEST tests/test_deck_tab_rename.cpp [decktabs][rename] (headless DeckView, decks "A" "B" "C", active 1; the tab is
   found among DeckView's children as the TextButton whose text is the deck name, so the test compiles on main):
     (a) "a click on the ACTIVE tab never calls onDeckSwitched": btn("B")->onClick() -> 0 calls. RED(main): 1 call.
         (onDeckSwitched in the test only records — never rebuilds inside the button's own std::function.)
     (b) "a click on another tab switches at once": btn("C")->onClick() -> exactly one call with 2 (regression guard,
         GREEN on main).
   Risk: anything that relied on clicking the showing tab to rebuild the grid. None found: a same-deck handleDeckSwitch
   only re-stores the same pointer and rebuilds (MainComponent.cpp:5488-5552); it also hid the Remove-Deck undo hint
   (DeckView.cpp:117-118) — a click on the showing tab no longer hides it (harmless; it times out in 10 s).
U3.2 The in-place editor. src/ui/DeckTabRow.h, src/ui/DeckView.h/.cpp
   DeckTabRow.h: pure `Rect editorRect(Rect tab, int rowWidth)` — x = tab.x, w = max(tab.w, kTabWidth) clamped so the
   box stays inside [0, rowWidth) (it may cover the next tab while editing). TEST added to tests/test_deck_tab_row.cpp
   (100-px tab -> same rect; 60-px tab -> 100 wide; last 60-px tab near the row end -> shifted left inside). RED(stub).
   DeckView.h (private): `struct DeckNameEditor : juce::TextEditor` beside DeckTabButton:
     - keyPressed(k): if output::classifyOutputKey(k) is CloseAll, RaiseApp or ToggleMain -> return false (the PANIC /
       raise / main-display keys reach MainComponent, E7); else juce::TextEditor::keyPressed(k).
     - keyStateChanged(down): call the base, then return true (no key-up while typing reaches the momentary-release
       sweep, E7).
   DeckTabButton gains `std::function<void()> onDoubleClick;` and
     `void mouseDoubleClick(const juce::MouseEvent& e) override { if (!e.mods.isPopupMenu() && onDoubleClick) onDoubleClick(); }`
   setupDeckTabs: `btn->onDoubleClick = [this, capturedIdx] { beginRename(capturedIdx); };` and, at its end, if an edit
   is open, `renameEditor_.toFront(false)` (new tab buttons are added above it).
   DeckView members: DeckNameEditor renameEditor_ (addChildComponent in the constructor, never rebuilt); bool
   renaming_ = false; uint32_t renamingDeckId_ = 0; an OutsideClick MouseListener registered once with
   `addMouseListener(&outsideClick_, true)`; public `std::function<void(int deckIndex, const juce::String& name)>
   onDeckRenamed;`, `void beginRename(int deckIndex)`, `void cancelDeckRename()`, `void noteMouseDownForRename(const
   juce::Component* target)` (the listener's body; public for tests / REST), test accessor
   `juce::TextEditor* renameEditorForTests()`.
   Behaviour:
     beginRename(i): guard i; if already renaming, finish the current edit (keep) first; renamingDeckId_ =
       decks[i].id; text = name, select all, centred, the font a 24-px tab is drawn with (LookAndFeel_V4::getTextButtonFont); bounds
       from editorRect; visible; toFront; grabKeyboardFocus().
     keep (finishRename(true)) on: onReturnKey; onFocusLost; noteMouseDownForRename(target) with target != editor and
       not inside it (TextEditor has internal children: test `renameEditor_.isParentOf(target)`).
     discard (finishRename(false)) on: onEscapeKey; cancelDeckRename(); a rebuild that no longer has a deck with
       renamingDeckId_.
     finishRename(keep): if (!renaming_) return; renaming_ = false (FIRST: hiding a focused editor fires onFocusLost
       again); hide; if keep: index = the deck with renamingDeckId_; trimmed = text.trim(); if index found, trimmed not
       empty and trimmed != the deck's current name -> onDeckRenamed(index, trimmed).
     rebuildGrid / resized while renaming: re-bind by id, bounds = editorRect(that deck's tab), keep the typed text.
     tabTooltipFor (DeckView.cpp:509-514): last line becomes "Double-click: rename. Right-click: Save / Rename /
       Duplicate / Remove".
   TEST tests/test_deck_tab_rename.cpp (cont.):
     (c) "a left double-click on a tab opens the box over that tab with its name selected": btn("B")->mouseDoubleClick(
         e{left, 2 clicks}) -> a visible juce::TextEditor child of DeckView, text "B", all selected, bounds ==
         editorRect(tab B). RED(main): no editor child.
     (d) "a right / Ctrl double-click never opens it". GREEN on main (guard).
     (e) Return with "  Intro  " -> onDeckRenamed(1, "Intro") once, box hidden; Esc -> no call; "" -> no call; "B" ->
         no call; focus-lost -> call; noteMouseDownForRename(a ClipCell) -> call; noteMouseDownForRename(an editor
         child) -> no call. RED(stub).
     (f) "a rebuild while open keeps the box on the same deck and above the tabs": begin on C, set active 0, rebuildGrid
         -> still open, bound to C's id, bounds == C's tab, it is the last child (on top); remove deck C + rebuild ->
         closed, no callback. RED(stub).
     (g) keys: the editor's keyStateChanged(false) returns true; keyPressed(Cmd+Shift+Esc) returns false; plain Esc
         closes without a callback. RED(stub).
     (h) the tab tooltip contains "Double-click: rename". RED(main).
   GREEN: (a)-(h) pass; tests/test_deck_tab_row.cpp all pass. Risk: section 7 R1-R4.
U3.3 One rename funnel. src/MainComponent.h/.cpp
   New `void applyDeckRename(int deckIndex, const juce::String& text);` = renameDeck's callback body
   (MainComponent.cpp:3690-3700) made shared: guard index; trimmed; empty -> return; equal to the deck's CURRENT name
   -> return; else pushCommands(RenameDeckCmd(makeCompositionResolver(), i, current, trimmed, "Rename Deck"), "Rename
   Deck"); deckView_->refresh(). renameDeck's modal callback keeps its own early-outs for result != 1 and calls
   applyDeckRename. Wiring next to deckView_->onDeckAction (MainComponent.cpp:1445-1456):
   `deckView_->onDeckRenamed = [this](int i, const juce::String& t) { applyDeckRename(i, t); };`.
   refreshUiAfterModelSwap (MainComponent.cpp:2924): first statement `if (deckView_) deckView_->cancelDeckRename();`
   (a loaded composition may reuse the deck id, E10). Save / load: no change — a renamed deck saves its name
   through the existing path (E9), and save / load round trips stay covered by tests/test_composition.cpp:853-867.
   Undo / redo: the existing "Rename Deck" step; Edit > Undo / Redo
   (handleMenuCommand kCompUndo / kCompRedo, MainComponent.cpp:6470-6481) runs refreshAfterUndoRedo, which rebuilds the
   grid (:5176-5180) and so relabels the tab. Test: live rows R5-R8 (MainComponent is not
   unit-testable headless); the command itself is already covered (test_undo_commands.cpp:1636-1654).
U3.4 TEST-ONLY REST (src/api/ApiServer.h/.cpp inside the AUDIODNA_TEST_SERVER block :299-329; MainComponent wiring
   in the TEST-ONLY block :2141-2170). Every action is marshalled with callAsync; every read uses the
   handleDebugUiText Box + 2 s wait (no new HTTP-thread reader of model strings — tsan-r5 stays clean).
     GET  /api/debug/deck_tabs -> {ok, active, tab_row_builds, tabs:[{index,id,name,label,tooltip,x,w}],
          editor:{open, deck_id, deck_index, text, x, y, w, h}}
     POST /api/debug/deck_rename {"deck": i, "op": "begin"|"type"|"enter"|"escape"|"focus_lost"|"outside_click",
          "text": "..."} -> the SAME DeckView functions the mouse / keys reach (begin = beginRename; type = setText;
          enter / escape / focus_lost = the editor callbacks; outside_click = noteMouseDownForRename(first clip cell))
     POST /api/debug/tab_click {"deck": i} -> DeckView::tabClicked(i)
     POST /api/debug/undo {"redo": false|true} -> handleMenuCommand(kCompUndo / kCompRedo) (the Edit menu path;
          onUndoHint already uses kCompUndo, MainComponent.cpp:1459-1462)

---------------------------------------- STAGE U2 — BF3 visible (needs U1; after U3 merges: same files) ------------
U2.1 NEW src/ui/ClipMediaText.h (pure; juce_core + Clip + VideoInfo)
   using VideoInfoSource = std::function<std::optional<VideoInfo>(const Clip&)>;
   struct Described { juce::String line, pathTip; juce::File revealTarget; bool fileBacked = false, missing = false; };
   namespace clipmedia {
     juce::File revealTarget(const Clip&);   // Video / Image: mediaFile; ImageSequence: sequenceFiles[0]; else File()
     Described describe(const Clip&, const std::optional<VideoInfo>& video);
     juce::String cellTooltip(const Clip&, const std::optional<VideoInfo>& video);
     std::vector<juce::String> menuItems(const Clip*);   // {"Show in Finder"} when revealTarget exists, else {}
   }
   describe(): Video -> missing ? "File missing" : video ? videoinfo::describe(*video) : "Video file not loaded";
   Image -> missing ? "File missing" : "<KIND> image" (extension upper-cased without the dot; JPG -> JPEG, TIF -> TIFF);
   ImageSequence -> "Image sequence, N images"; Source / Camera / None -> fileBacked false, line "". pathTip = the
   reveal target's full path. missing = Clip::mediaMissing (Image / Video only; no stat here).
   cellTooltip(): Video / Image -> "<file name with extension>\n<line>\nRight-click: Show in Finder"; ImageSequence ->
   today's exact first line ("Image sequence — N images at X.X images/sec", ClipCell.cpp:379-380) +
   "\nRight-click: Show in Finder"; others -> "".
   TEST tests/test_clip_media_text.cpp [clipmedia]: every branch above incl. missing, unknown video, JPG/JPEG, empty
   sequence (no target), Source / Camera / None (no target, no tooltip). RED(stub). GREEN: all.
U2.2 src/ui/ClipCell.h/.cpp
   - `void setVideoInfoSource(const VideoInfoSource* s)` (pointer to DeckView's, like setThumbnails).
   - `juce::String getTooltip() override` -> clip_ ? clipmedia::cellTooltip(*clip_, lookup) : {} (lookup = the source,
     Video clips only). Remove the setTooltip branch from setClip (ClipCell.cpp:374-385) — the text moves into
     cellTooltip unchanged.
   - mouseDown (ClipCell.cpp:220-234): first line
     `if (event.mods.isPopupMenu() && clip_ != nullptr && !clipmedia::menuItems(clip_).empty()) { showContextMenu(); return; }`
     then today's `if (event.mods.isRightButtonDown()) return;` unchanged.
   - showContextMenu(): the showDeckTabMenu idiom (DeckView.cpp:524-540): section header = clip name; items from
     menuItems; setLookAndFeel(&getLookAndFeel()); showMenuAsync(withTargetComponent(this).withParentComponent(
     getTopLevelComponent())); result 1 -> `onRevealInFinder(layerIndex_, column_)` (re-check clip_ still has a
     target). New callback `std::function<void(int layerIndex, int column)> onRevealInFinder;`.
   - Header comment (ClipCell.h:8-12) updated: right-click = the file menu on file clips.
   TEST tests/test_clip_cell_media.cpp [clipcell] (headless, fake VideoInfoSource):
     (a) a video cell's getTooltip() == cellTooltip(...) with the fake info. RED(main): "" (no tooltip on video cells).
     (b) Ctrl+left mouseDown on a VIDEO cell's thumbnail does NOT call onTrigger. RED(main): onTrigger called.
     (c) Ctrl+left mouseDown on a SOURCE cell still calls onTrigger (unchanged). GREEN on main (guard).
     (d) right mouseDown on a source / empty cell calls nothing (unchanged). GREEN on main (guard).
     (e) a sequence cell's tooltip first line is byte-identical to today's. RED(main) only for the added last line.
   GREEN: all.
U2.3 src/ui/DeckView.h/.cpp — fan-out only.
   Member `VideoInfoSource videoInfoSource_;` declared BEFORE layerStrips_ / clipCells_ (DeckView.h:120-124: outlives
   the cells, like thumbnails_); `void setVideoInfoSource(VideoInfoSource s)`; `std::function<void(int layerIndex, int
   column)> onRevealInFinder;`. In rebuildGrid's cell loop (DeckView.cpp:185-231) add
   `cell->setVideoInfoSource(&videoInfoSource_);` and `cell->onRevealInFinder = [this](int li, int c) { if
   (onRevealInFinder) onRevealInFinder(li, c); };`.
U2.4 src/ui/ClipInspector.h/.cpp
   - New children: `juce::TextButton revealBtn_{"Show in Finder"}` (componentID "revealClipFile"), `juce::Label
     mediaInfoLabel_` (10.5 px, kTextSecondary; "File missing" drawn in the cell's missing red 0xffcc3333,
     ClipCell.cpp:210; tooltip = pathTip; no editing). `void setVideoInfoSource(VideoInfoSource)`,
     `std::function<void(Clip*)> onRevealInFinder;`, private `void updateMediaInfo();`, `int infoRowHeight() const`
     (kInfoRowHeight = 18 when describe(...).fileBacked, else 0), `static constexpr int kRevealButtonWidth = 104;`.
   - Layout: resized (:578-583): revealBtn_ at the name bar's right (104 x 20, centred in the 28-px bar, kInset from the
     edge), visible iff fileBacked; after `y += kNameBarHeight;` place mediaInfoLabel_ (area.getX(), y, area.getWidth(),
     infoRowHeight()) and `y += infoRowHeight();`. paint (:469-479): name trimmed right by kRevealButtonWidth + 8 when the
     button shows (else the existing 40), section y starts at kNameBarHeight + infoRowHeight() + .... getPreferredHeight
     (:1044): + infoRowHeight() (the no-clip branch unchanged). PaintKey (ClipInspector.h:63-75): add `int infoRow = 0;`
     set in paintKeyNow (paint() offsets depend on it; "add, never remove").
   - updateMediaInfo(): called from setClip (:788-809) always, and from refresh (:960-991) when the key (clip ptr, media
     type, reveal path, mediaMissing) changed OR the last Video lookup returned nothing (so a player that opens later
     is picked up; bounded to that case). Sets label text / tooltip / colour compare-before-set, button visibility.
   - revealBtn_.onClick -> `if (clip_ && onRevealInFinder) onRevealInFinder(clip_);`
   TEST tests/test_clip_inspector_media.cpp [clipinspector] (headless, fake source):
     (a) setClip(video clip) -> a VISIBLE TextButton child with text "Show in Finder" exists. RED(main).
     (b) getPreferredHeight(video clip) - getPreferredHeight(source clip with no params) == 18. RED(main): 0.
     (c) the info label text == describe().line; for a source clip the button and label are hidden. RED(stub).
     (d) the fake source returns nothing, then info: after one refresh() the label shows the codec line. RED(stub).
     (e) revealBtn_->onClick() fires onRevealInFinder(clip) once. RED(main).
     (f) tests/test_clip_inspector_paint_key.cpp stays green; a new case: toggling fileBacked changes PaintKey.
U2.5 src/MainComponent.h/.cpp
   - `std::optional<VideoInfo> videoInfoFor(const Clip& c)`: Video only; `auto* p = renderer.getVideoPlayer(c.id)`;
     p && p->getFile() == c.mediaFile && p->getInfo().known() -> p->getInfo(); else nullopt (message thread, E16).
   - `void revealClipFile(const Clip& c)`: target = clipmedia::revealTarget(c); none -> return; testMode_ ->
     lastRevealPath_ = target path, ++revealCount_, return; else if target exists or its parent folder exists ->
     target.revealToUser(); else setFileLabel("Show in Finder: not found - " + path).
   - `void revealClipAt(int layer, int column)`: the active deck's clip there -> revealClipFile.
   - Wiring (constructor, beside :1445-1462 and :1500-1520): deckView_->setVideoInfoSource([this](const Clip& c) {
     return videoInfoFor(c); }); deckView_->onRevealInFinder = [this](int l, int c) { revealClipAt(l, c); };
     inspectorPanel_->getClipInspector().setVideoInfoSource(same); ...onRevealInFinder = [this](Clip* c) { if (c)
     revealClipFile(*c); };
U2.6 TEST-ONLY REST (same blocks as U3.4):
     GET  /api/debug/clip_media?layer=L&column=C -> {ok, media_type, line, tooltip, path_tip, reveal_target, file_backed,
          missing, video:{codec,width,height,fps}|null, menu:[...], inspector_shows, inspector_line,
          inspector_button_visible, last_revealed, reveal_count} (message-thread read)
     POST /api/debug/reveal_clip {"layer": L, "column": C} -> revealClipAt (the cell menu's path)
     POST /api/debug/inspect_clip {"layer": L, "column": C} -> DeckView::selectCell + onClipSelected (a name-bar click)

---------------------------------------- STAGE U4 — docs, probe, captures (after U1-U3 merge) ----------------------
U4.1 .harmony/probe-ui-files-rename.sh (NEW; modeled on .harmony/probe-deck-tabs.sh: the live-lock gate, open -g ...
   --args --test-mode, Connection: close, Quartz window-id captures only, graceful quit, refuses without the lock) +
   its composition written at run time with absolute paths (section 5 table). TEMPORARY hook (NEVER committed; the
   probe-deck-tabs Phase 2 pattern): AUDIODNA_DEBUG_SHOW=cell-menu (opens a video cell's menu, component snapshot),
   =cell-tooltip (TooltipWindow::displayTip over a video cell, component snapshot).
U4.2 Docs (section 6).

BUILD STAGES (one builder context each; dependency order):
   U1 (BF3 data)  ||  U3 (BF8)   — disjoint files except an append to tests/CMakeLists.txt; either can run first.
   U2 (BF3 UI)    — after U1 and U3 are merged (U2 and U3 share DeckView.*, MainComponent.cpp, ApiServer.*).
   U4 (docs, probe, captures) — last; the critic panel and the Boris page follow U4.
   Ships alone: U3 (BF8 complete). U1 alone is invisible (data only). U2 needs U1.

FILES SHARED WITH OTHER LANES (exact blocks, for merge sequencing):
   src/ui/DeckView.h  DeckTabButton (:131-145), public API block (:51-100), members (:118-170)          bf9b (rows/strips)
   src/ui/DeckView.cpp setupDeckTabs (:471-507), tabTooltipFor (:509-514), resized tab block (:91-103),
                       rebuildGrid cell loop (:185-231), constructor (:4-36)                            bf9b, bf1 (cell UI?)
   src/ui/ClipCell.h/.cpp mouseDown (:220-234), setClip (:369-388), new getTooltip / menu              bf1 (paint, if any)
   src/ui/ClipInspector.h/.cpp constructor, paint (:469-479), resized (:573-583), setClip (:788-809),
                       refresh (:960-991), paintKeyNow (:993-1017), getPreferredHeight (:1034-1044)       bf7, bf6
   src/MainComponent.cpp constructor wiring (:1445-1462, :1500-1520, TEST-ONLY :2141-2170), renameDeck
                       (:3674-3702), refreshUiAfterModelSwap (:2924), new functions appended after renameDeck  bf9b, bf1, bf2
                       NOT touched: onDeckSwitched (:1411-1442), handleDeckSwitch (:5488-5552) — bf9b's core.
   src/media/VideoPlayer.h/.cpp accessor (:66-71), members (:168-176), open() after :155              mkvidx (:275-304)
   src/api/ApiServer.h/.cpp TEST-ONLY block (:299-329) + appended handlers                           bf1, bf2, bf45
   tests/CMakeLists.txt (append), docs/claude/performance-controls.md (:51 paragraph), CLAUDE.md       bf9b (perf-controls, CLAUDE.md)

====================================================================================================================
## 5. GATES Harmony runs after the merge (pre-registered bars; any FAIL = fix round, then the WHOLE gate again)
====================================================================================================================
G0 Build the session's Release + AUDIODNA_BUILD_TEST_SERVER build; zero new warnings in the touched files.
G1 Unit: run the test executables test_video_info, test_clip_media_text, test_clip_cell_media,
   test_clip_inspector_media, test_deck_tab_rename, test_deck_tab_row, test_undo_commands, test_clip_inspector_paint_key
   directly = every case passes; then the FULL `ctest --test-dir build` suite: no case that passes on main 5e47d17 fails
   (compare the two lists; catch_discover_tests names cases by TEST_CASE text, so -R on tags does not select them).
G2 Lints unchanged-green: test_hot_thread_io_lint, test_render_thread_lint, test_log_line_lint, test_shared_field_types.
G3 Live: `.harmony/probe-ui-files-rename.sh OUT` (lock held; one live Audio-DNA; --test-mode; no Output window).
   Composition (written by the probe, absolute paths): deck "A" = 2 layers x 4 columns:
     L0: C0 tests/fixtures/video_h264_64x64.mp4 | C1 video_prores_hq_64x64_2997.mov | C2 video_hapq_64x64_60.mov |
         C3 video_hevc_main10_64x64_23976.mp4
     L1: C0 tests/fixtures/test_card.png | C1 an image sequence of 3 PNGs (made in the probe's temp dir) |
         C2 a video path that does not exist (its folder does) | C3 the source "perlin_noise"
   decks "B" and "C" = copies of "A" (fresh ids), active deck 0.
   BF3 rows (GET /api/debug/clip_media), bar = exact string equality, 8/8:
     V1 L0C0 line "H.264 High, 64 x 64, 30 frames per second"   L0C1 "ProRes 422 HQ, 64 x 64, 29.97 frames per second"
        L0C2 "HAP Q, 64 x 64, 60 frames per second"              L0C3 "HEVC Main 10, 64 x 64, 23.976 frames per second"
        L1C0 "PNG image"   L1C1 "Image sequence, 3 images"   L1C2 "File missing"   L1C3 "" (file_backed false)
     V2 tooltip L0C0 == "video_h264_64x64.mp4\nH.264 High, 64 x 64, 30 frames per second\nRight-click: Show in Finder";
        L1C1 first line == "Image sequence — 3 images at 2.5 images/sec"; L1C3 == "".  menu: L0C0 ["Show in Finder"],
        L1C3 [].
     V3 POST reveal_clip L0C0 -> last_revealed == that fixture's absolute path, reveal_count +1; L1C3 -> count +0;
        L1C2 -> last_revealed == the missing file's path, count +1.
     V4 POST inspect_clip L0C0 -> inspector_shows true, inspector_line == V1's L0C0 line, inspector_button_visible
        true; inspect L1C3 -> inspector_line "", button hidden.
   BF8 rows (GET /api/debug/deck_tabs, /api/composition decks[].name):
     R1 3 tabs A/B/C, active 0, editor closed, every tooltip contains "Double-click: rename".
     R2 tab_click 0 (the active one) -> tab_row_builds +0, active 0.
     R3 tab_click 2 -> builds +1, active 2 (a single click still switches at once).
     R4 double-click emulation on tab 1 (not active): tab_click 1, tab_click 1, deck_rename{1, begin} -> builds +1 in
        total, active 1, editor open, deck_id == B's id, text "B", editor x == tab 1 x.
     R5 type "  Intro  ", enter -> decks[1].name == "Intro", editor closed, tab 1 label "Intro".
     R6 undo -> "B"; redo -> "Intro".
     R7 begin 1, type "X", escape -> "Intro"; begin 1, type "", enter -> "Intro"; begin 1, type "Intro", enter ->
        undo stack top unchanged (no new step: next undo restores "B").
     R8 begin 1, type "Y", outside_click -> "Y"; begin 1, type "Z", focus_lost -> "Z".
     R9 begin 0, type "Q", then POST /api/switch_deck {"deck": 2} (a non-UI switch rebuilds the row) -> editor still
        open, deck_id == A's id, x == tab 0 x; enter -> decks[0].name == "Q", decks[2] unchanged.
     R10 begin 0, then POST /api/load_composition (the same file) -> editor closed, no name changed by the edit.
   Decision: 8/8 V-strings and R1-R10 all PASS = BF3/BF8 behaviour GREEN.
G4 VISUAL WORK GATE (before Boris sees anything). Decoded window captures by Quartz window id only, app never
   brought to the front, no synthetic input:
     C1 tab row default; C2 rename box open over the active tab, name selected (REST begin); C3 typed "Intro";
     C4 renamed tab; C5 rename box on a narrow tab (12 decks: 60-px tabs -> 100-px box); C6 video cell menu and C7 video
     cell tooltip (TEMPORARY hook build, component snapshots); C8 Clip inspector, H.264 clip; C9 HAP Q clip;
     C10 missing file; C11 PNG picture; C12 image sequence; C13 procedural source (no row, no button); C14 a long clip
     name next to the button.
   Critic panel (independent seats, findings classified blocking / should / nit): visual-design, UX, graphic-design,
   logic, and a dedicated INTERACTION-LOGIC critic for the rename box (open / keep / discard / rebuild / focus / keys)
   and the cell menu. Blocking findings -> fix round -> re-shoot -> re-review. Then ONE artifact page for Boris
   (.harmony/.reports/s-rta-1002b/ui-boris.html): the captures + "what to try" in plain words (section 8).
   Note for the critics: in gate captures the app is in the background, so the rename box may show no caret (focus is
   not granted to a background window); that is a rig artefact, not a defect.
G5 INFO only: GET /api/debug/ui_passes for 10 s with the rename box open vs closed (the caret blinks: Pitfall 57's
   union) — report passes/s and the union rect; no bar (transient state, same as every text field today).
G6 INFO only: /api/state msg_video_lock_wait_max_ms over 60 s with GET clip_media on L0C0 at 8 Hz (the tooltip poll
   rate) vs idle; if Harmony wants a verdict, INTERLEAVED A/B, >= 5 runs per arm, ps checked for CPU burners first; a
   difference inside the run-to-run drift is INFO.

====================================================================================================================
## 6. DOCS
====================================================================================================================
- docs/claude/performance-controls.md "Deck tab row" (:51): add "double-click a tab = rename in place (Enter, Tab or
  click-away keeps, Esc discards, empty or unchanged keeps the old name; one undo step "Rename Deck"; right-click >
  Rename Deck... still opens the dialog, same rules); a click on the deck already showing does nothing (it must not
  rebuild the row: Pitfall NN); on a tab that is not showing, the first click of the double-click switches decks";
  plus a short paragraph "Clip file info and Show in Finder": the cell tooltip / cell menu / inspector row and button,
  where the info comes from (VideoPlayer::open; never a decode or probe on the message or GL thread; Pitfalls
  51 / 55 / 56 / 58), pictures / sequences / sources / missing files, and that --test-mode records the reveal instead
  of opening Finder.
- docs/claude/testing-eyes.md: the seven TEST-ONLY routes (clip_media, reveal_clip, inspect_clip, deck_tabs,
  deck_rename, tab_click, undo) with their JSON.
- docs/claude/pitfalls.md: "Pitfall NN" (Harmony assigns; next free 64): "An in-place editor over a rebuilt row —
  (1) a click that changes nothing must not rebuild the row: JUCE delivers mouseDoubleClick after mouseUp / onClick and
  only to a component that survived it, and counts clicks per mouse source, not per component (juce_Component.cpp
  :2253-2270; juce_MouseInputSourceImpl.h:405-421, :553-562); (2) JUCE does not move keyboard focus when the click lands
  on a non-focusable component inside an ancestor of the editor (juce_Component.cpp:2649-2680): click-away needs an
  explicit outside-click commit; (3) a TextEditor swallows Cmd+Shift+Esc (modifier-blind Escape) and lets key-ups
  through to MainComponent's momentary-release sweep — the deck-name editor passes the output keys on and swallows key
  state. Guards: tests/test_deck_tab_rename.cpp; live probe-ui-files-rename R2-R4, R8, R9."
- CLAUDE.md (24,006 bytes measured now; cap 25,000): Pitfall-index line "NN. In-place editor over a rebuilt row -- before
  adding double-click / in-place editing to a row that rebuilds." (~115 B) and the UI Patterns bullet (:128) becomes
  "**Deck tab row**: right-click a tab = its menu, never a deck switch; double-click = rename in place
  (`docs/claude/performance-controls.md`)." (~+30 B). Paid by removing the "### Common Build Issues" block (:62-64,
  ~125 B), which the Trigger Table's build row already routes to docs/claude/build-other-platforms.md (move its four
  topic words into that row if Harmony prefers). Bar: net growth <= 0 bytes for this lane.
- .harmony/APP-INVENTORY.md: row :58 (deck tabs: click = switch, click on the showing tab = nothing, double-click =
  rename in place, right-click = menu), row :60 (ClipCell: right-click / Ctrl+click on a file clip = "Show in Finder"
  menu; hover tooltip for video / picture / sequence cells), row :72 (ClipInspector: name bar "Show in Finder" + file
  info row for file clips), the TEST-ONLY route list.
- BORIS_DECISIONS.md (Harmony writes, Boris's words verbatim): under "Clip Cells": "Codec display for each video and
  easy access to that video in finder" (2026-10-02) -> what was built; a deck-tab entry: "Need to be able to rename each
  deck with double click" (2026-10-02) -> the rename rules + Boris's answers to Q1-Q3.

====================================================================================================================
## 7. RISKS (with the cheapest discriminating test)
====================================================================================================================
R1 The real double-click path (JUCE's event dispatch) cannot be driven by any gate (no synthetic input). The design
   rests on E5 (read in JUCE source) + the active-tab no-op. Cheapest test: unit (a) pins the no-op; Boris's first
   double-click on the live rig (section 8, B1) is the only end-to-end check. If it fails there, the fallback needs no
   redesign: a DeckView-level MouseListener receives the double-click even when the button died (VERIFIED: JUCE sends it
   to the nearest surviving parent's listeners, juce_Component.cpp:2272-2276 + MouseListenerList::sendMouseEvent :138-166)
   and maps the position with DeckTabRow::layout.
R2 Click-away inside the grid (E6). If the outside-click listener misses a case, an edit stays open while Boris plays.
   Test: unit (e) noteMouseDownForRename + live R8; critic "interaction-logic" seat walks every click target.
R3 Keys while typing: a name containing a momentary key letter must not stop a playing clip (E7); the PANIC key must
   still close outputs. Test: unit (g). Wider (NOT fixed here, flagged for Harmony): every other TextEditor in the app —
   TopBar BPM, browser search boxes, RecordPanel, and bf2's typed sync value — has both holes today; a shared editor
   base class is the follow-up. Side effect accepted: a momentary key HELD when the box opens and released while typing
   is not released until the next key-up that reaches MainComponent (the sweep then releases it).
R4 A rebuild while the box is open (MIDI / OSC / genre auto-switch, a staged load landing): z-order (new tabs are added
   above), position, and id re-use across a composition swap. Tests: unit (f), live R9 / R10.
R5 Ctrl+click on a video / picture / sequence cell changes from "trigger" to "menu" (macOS convention; matches the deck
   tabs and routine pads). Anyone who triggers with Ctrl+click would notice. Test: unit (b)/(c); Boris B4.
R6 Show in Finder under --test-mode never calls Finder, so no gate exercises revealToUser. It is the same one-line JUCE
   call already shipped twice (E14). Test: Boris B4.
R7 A label without a profile for odd containers (profile unknown) or VFR files showing an averaged rate (29.83 is
   honest). Test: G3 V1 across mp4 / mov / avi / ts fixtures (U1.2's test covers .avi MPEG-4).
R8 The info row + button crowd a narrow inspector (long names truncate ~60 px earlier; "frames per second" may
   ellipsize). Test: C14 + critic panel; the full text stays in the row's tooltip.
R9 Merge friction: bf9b reworks DeckView rows / strips and performance-controls.md before this lane builds; bf7 edits
   ClipInspector's selectors. Mitigation: edits confined to the named blocks; U2 rebases after U3.
R10 Caret blink while renaming joins the window repaint union (Pitfall 57). Transient; G5 reports it (INFO).
R11 A deck name containing "/" makes Save Deck As... propose a sub-folder path (dir.getChildFile(name + ".json"),
   MainComponent.cpp:3623-3626). Pre-existing (the dialog allows it too); easier to hit now. Not fixed here; flagged.
STRONGEST COUNTERARGUMENT to the whole plan: "BF3's info should simply live on Clip — every other media property
   (thumbnail, hasAlpha, clipWidth/Height) does, and it would serialize for free." It loses because (1) reopen paths
   never copy properties (E17) so the codec line would be missing exactly for clips whose files came back, (2) the fix
   for that writes a live Clip from the message thread, the race class Pitfall 58 forbids, and (3) serializing codec
   strings would let a saved file describe a video that has since been re-encoded; the open player is the only source
   that is always true.

====================================================================================================================
## 8. WHAT ONLY BORIS CAN CHECK (live rig, his real show files)
====================================================================================================================
B1 Double-click a deck tab (one that is showing, then one that is not): the name box appears right on the tab with the
   old name selected; no flicker beyond the normal deck change; typing replaces the name; Enter keeps it.
B2 Click somewhere else (a clip, the grid, the inspector) keeps the new name; Esc throws it away; clearing the name
   keeps the old one; Cmd+Z (outside the box) undoes the rename.
B3 While typing a deck name, no clip fires and nothing that is playing stops — try a name using your clip keys.
B4 Right-click (or Ctrl+click) a video cell -> "Show in Finder" -> Finder opens with that file selected; the same from
   the inspector button; a missing clip opens the folder it was in.
B5 The codec line matches what he knows about his real files (HAP / HAP Q / ProRes / H.264 / DXV), including frame rates.
B6 The hover tooltip on cells: timing and readability while performing.

====================================================================================================================
## 9. QUESTIONS FOR BORIS (product / taste only; each has a default so the build never waits)
====================================================================================================================
Q1 "If you double-click a deck tab that isn't the one showing, the first click shows that deck (like any click) and
   then the name box opens. With your new rule, switching decks doesn't change what's playing — it only changes which
   clips you see. OK?"  DEFAULT: yes.
Q2 "When you rename: Enter or clicking anywhere else keeps the new name, Esc cancels, and an empty name keeps the old
   one — like renaming a file in Finder. OK?"  DEFAULT: yes.
Q3 "For pictures (not videos) the info line just says the kind of file, like 'PNG image'. Do you also want the
   picture's size (for example 1920 x 1080)?"  DEFAULT: no, kind only.

STATUS: COMPLETE — plan ready for review / build (stages U1 || U3, then U2, then U4)

## HARMONY ADOPTION (s-rta-1002b, 2026-10-02 15:48:27) — overrides the ruling, which overrides the plan body
1. ADOPTED: .harmony/.reports/s-rta-1002b/ruling-ui.md IN FULL (AM1-AM18, FINAL BUILD STAGES U1 / U3 / U2 / U4 — built
   SEQUENTIALLY in this one lane in that order —, FINAL GATES G0-G4). Base = main 9a832a0 (src identical to fa9604d).
2. BORIS QUESTIONS — defaults taken until he answers: Q1 (a) a double-click on a tab that is not on screen only shows that
   deck (kRenameOnlyTheShowingTab = true); Q2 yes (Enter / Tab / click-away keep, Esc cancels, empty keeps the old name);
   Q3 no (pictures show the kind only).
3. G1b (probe_deck_tab_dispatch, an ON-SCREEN always-on-top window on Boris's display): Harmony constraint: the builder
   BUILDS it and proves P1-P9's logic compiles, but NEVER RUNS it (no window on Boris's screen from any lane). Harmony runs
   it once, herself, after asking Boris. Its RED(stub) evidence is replaced by the unit RED of test_deck_tab_rename /
   test_deck_tab_row. Also never run any other on-screen rig (the architect's rig2 / rig3).
4. SHARED FILE with the RUNNING lane mkvidx: src/media/VideoPlayer.h / .cpp. U1 adds open-time info extraction + a getter
   only; mkvidx owns readKeyIndex / decodeStep / runStep / seek paths — touch none of them. Whichever merges second
   rebases (Harmony sequences). Later lanes (bf9b) rebase over DeckView / MainComponent / ApiServer.
5. NOT IN THIS LANE (filed by Harmony): X1 (cell click + Return clears the top layer and stops its routines — HIGH, a
   bug lane after bf9b, LayerStrip is bf9b's file), X2 (TextEditor commit timing), X3 (launch focus), X4 ("/" in a deck
   name -> Save Deck As sub-folder).
6. Harmony constraint: BORIS USES THIS MACHINE AND THIS APP — an Audio-DNA your lane did not start is his (s-rta-1002b
   incident): never quit / kill / touch it; the lock helper waits for it; if start_app refuses, stop the batch and release.
7. Visual: the critic panel (visual + UX + graphic + logic + interaction-logic for the rename flow) runs on Harmony's
   decoded window captures after the merge, before Boris sees anything. MERGE by Harmony.
