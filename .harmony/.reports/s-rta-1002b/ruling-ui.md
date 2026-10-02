# RULING lane ui: the blind council's attacks on plan-ui.md (BF3 codec + Show in Finder, BF8 double-click rename)
Session s-rta-1002b · Architect (Fable) · main fa9604d. Between plan base 5e47d17 and fa9604d the lane's files changed
only in comment text ("Pitfall NN" -> 57), so every plan citation still holds.
Inputs: plan-ui.md; the seat papers (written verbatim to attack-ui-papers.md); boris-feedback-backlog.md;
binding-decisions.md "2026-10-02 (s-rta-1002b)"; BORIS_DECISIONS.md:340-355.
User-voice check: the relayed request (Boris's 14:30 answers) only takes the BF3 / BF8 defaults
(binding-decisions.md:582). Its item 6 ("deck change stops the old deck") was clarified by Boris at 14:44
("decks are boxes of clips", BORIS_DECISIONS.md:350-355). This ruling works under either reading. Under the default
in AM2, a double-click on a tab that is not on screen is exactly one deck switch, the same as a single click, so it
does whatever the binding BF9 rule makes a switch do.
VERDICT: plan AMENDED, ready to build. 21 attacks ruled: 16 accepted, 4 accepted in part, 1 rejected. 18 amendments.

====================================================================================================================
## 0. NEW EVIDENCE (architect scratch runs for this ruling)
====================================================================================================================
Rig: in-process C++ linked against the session build's own objects (build/tests/CMakeFiles/test_deck_thumbnails.dir:
the real DeckView / ClipCell / LayerStrip / LookAndFeel built from main, plus the JUCE 8.0.4 modules). Events go in
through juce::ComponentPeer::handleMouseEvent / handleKeyPress, the same entry points the mac peer calls. Sources are
in $TMPDIR/uirig/ (rig.cpp, rig2.cpp ... rig5.cpp, measure.cpp) with the build script $TMPDIR/rig2.sh
(TMPDIR = /var/folders/xk/4b_nszfx60n0g13f6x0c3vy40000gn/T/). They are ephemeral; the results below are the record.
E-R1  VERIFIED (rig.cpp). When the window is OFF-SCREEN, every injected mouse event is dropped (0 deliveries in 7
      scenarios), but keyboard focus and key routing still work. Cause: the mac peer accepts a mouse event only where
      its window is the topmost one on screen (contains -> isWindowAtPoint -> windowNumberAtPoint,
      juce_NSViewComponentPeer_mac.mm:516-543). CONSEQUENCE: a headless (off-screen) real-dispatch ctest is not
      possible on macOS.
E-R2  VERIFIED (rig2 vs rig3). A small ON-SCREEN, always-on-top, borderless window does get real dispatch after one
      run-loop turn (CFRunLoopRunInMode 0.4 s). contains() was 0 before that turn and 1 after. The rig process is
      never activated.
E-R3  VERIFIED (rig3: real dispatch, real DeckView, onDeckSwitched rebuilds on every call like
      MainComponent.cpp:5550-5551).
      S1/S1b (main's behaviour: every tab click rebuilds the row). On a double-click, the tab's own mouseDoubleClick
        is never called. DeckView's nested listener does get it, but with eventComponent == originalComponent ==
        DeckView, and the position is the DEAD BUTTON'S LOCAL position: (50,12) where DeckView's is (152,344).
        juce_Component.cpp:81-95 keeps me.position when it substitutes the nearest parent.
        e.source.getLastMouseDownPosition() is correct.
      S2/S3 (U3.1: a click on the active tab does nothing). The tab survives and gets mouseDoubleClick. The listener
        gets eventComponent == the tab with correct positions. S7: a surviving button's own mouseDoubleClick override
        is called.
      S4: clicking A then B 100 ms apart gives clicks=1 for each and no double-click (the 8-px rule). S5: two clicks
        500 ms apart give no double-click. S6: with a rebuild BETWEEN the two clicks, the new tab's mouseDown reports
        clicks=2 and the double-click is still delivered (clicks are counted per mouse source, not per component).
      Every tab mouseDown reaches the listener with originalComponent == that tab, which is still alive at that point.
      So plan E5 is now VERIFIED by a run, and plan R1 ("no gate can drive it") is wrong for an on-screen, in-process
      probe.
E-R4  VERIFIED (rig, rig4 F7/F8). Hiding a FOCUSED editor inside DeckView does not leave focus empty. JUCE gives focus
      to DeckView's first focusable descendant (setVisible -> parent->grabKeyboardFocus, juce_Component.cpp:296-307,
      :2667-2673), which is column trigger "1". The NEXT Return then fires column 1 (onColumnTriggered(0)), because
      Button::keyPressed(Return) calls triggerClick (juce_Button.cpp:665-674, :359-362). If focus is handed to the
      home component BEFORE the hide, focus stays there and Return reaches the home component's keyPressed.
E-R5  VERIFIED (rig4). A click on a clip cell moves keyboard focus to the TOP layer strip's "X" button, the first
      focusable descendant of gridContent_ (juce_Component.cpp:2667-2679; LayerStrip's TextButtons want focus by
      default, juce_Button.cpp:89). If the editor has focus, the same click takes focus away from it, so plan E6 is
      WRONG for cells. A click on DeckView's own background leaves focus on the editor, which is the only case where
      E6 holds. A click on the grid's scrollbar gives focus to the Viewport.
E-R6  VERIFIED (rig4 F5/F6). This is PRE-EXISTING on main: click any clip cell, then press Return, and
      onLayerClearClip(top layer) fires. The X button clears that layer's clip and stops every routine on it
      ("a routine stop cannot be undone", LayerStrip.cpp:341-346). See X1.
E-R7  VERIFIED (rig5, main). A click on a tab that is not on screen leaves focus on the grid Viewport. The rebuild
      destroys the focused tab, and JUCE then re-grabs focus for the parent (juce_Component.cpp:1245-1257).
E-R8  VERIFIED (rig4 F9 + source). A TextEditor does not consume Tab. Tab first reaches the home component's
      keyPressed (in the app that is MainComponent::keyPressed -> bindingManager_.processKeyDown,
      MainComponent.cpp:3910-3919, so a binding on Tab fires). It THEN moves focus to the next tab button
      (juce_ComponentPeer.cpp:223-229). Plan F-B3's "Tab keeps" was only INFERRED, and it is half true.
E-R9  VERIFIED (source + rig4 F2b). A TextEditor's Return, Escape and focus-loss callbacks are POSTED messages
      (juce_TextEditor.cpp:1332-1333, :2248, dispatched at :2280-2296). onFocusLost ran only after a run-loop turn.
      So onReturnKey and onEscapeKey run one message-loop turn after the key, and a headless test without a message
      loop never sees them.
E-R10 VERIFIED (measure.cpp, the app LookAndFeel's default sans font at the 10.5-px info size). Full lines:
      "H.264 High, 1920 x 1080, 29.97 frames per second" 225.3 px; "QuickTime Animation, 1920 x 1080, 29.97 frames per
      second" 269.5; "H.264 Constrained Baseline, 1920 x 1080, 29.97 frames per second" 295.4.
      Split lines: the longest codec is "H.264 Constrained Baseline" at 119.5; the longest size + rate is
      "4096 x 2160, 23.976 frames per second" at 175.9.
      Button and tab text is drawn at 14 px with NO ellipsis (AudioDNALookAndFeel::drawButtonText,
      LookAndFeel.cpp:97-105). "Show in Finder" measures 84.7 px and "Deck 12" 47.4 px.
E-R11 INFERRED (arithmetic from cited constants). At the minimum window width (1280, Main.cpp:53;
      MainComponent.cpp:2327) with the default dividers at 0.50 / 0.75 (MainComponent.h:498, .cpp:2714-2729):
      the inspector is about 315 px wide, the ClipInspector about 303 px (InspectorPanel.cpp:97-98, :191-194), and
      the info label about 291 px (kInset 6). So the worst full line (295.4 px) already ellipsizes there, and any
      divider drag (it can go down to kMinPanelWidth 120, MainComponent.h:497) ellipsizes ordinary lines too.
E-R12 VERIFIED (ffprobe 8.0).
      video_h264_vfrgap_64x64.mp4: h264 High avc1, avg rate 85/4 (21.25), r rate 30/1.
      video_h264_vfr_64x64.mp4: avg rate 30/1, so it does NOT test variable frame rate.
      video_rawrgba_63x37.mov: rawvideo RGBA, profile unknown, 63x37, 30/1.
      video_mpeg4_64x64.ts: mpeg4 Simple Profile, tag 0x0010, 30/1.
      video_mpeg4_bf2_64x64.avi: mpeg4 ASP FMP4, 30/1.
      Six scratch encodes (1-frame mkv / mp4 / mov-mjpeg / mov-png, raw .h264, raw .m4v) ALL report a frame rate
      (raw streams get a guessed 25). A stream with no rate at all cannot be made here.
E-R13 VERIFIED (source). POST /api/load_composition answers {"ok":true} only after the staged swap
      (ApiServer.h:70-75; kLoadWaitMs 60000 at :310). The swap runs in finishStagedLoad after EVERY video open has
      landed and been installed under its clip id (MainComponent.cpp:3130-3193). GET /api/state "load" has
      opens_pending, staged and queued (MainComponent.cpp:5331-5343).
E-R14 VERIFIED (source). A user tab switch pushes "Switch Deck" when the deck changed (MainComponent.cpp:1434-1441).
      UndoManager has undoDescription, redoDescription, historySize and undoIndex (src/core/UndoManager.h:27-44).
E-R15 VERIFIED (source). refreshUiAfterModelSwap runs AFTER the model is swapped (its own comment,
      MainComponent.cpp:2949-2951). It nulls the inspectors first ("ORDER MATTERS", :2926-2937) and rebuilds the grid
      after that (:2958-2962).

====================================================================================================================
## 1. RULINGS, ATTACK BY ATTACK   (UX = seat ui-ux, G = seat ui-gates)
====================================================================================================================
UX-A1 [MUST] ACCEPT, sharpened by E-R1 / E-R3. AM2: the DeckView listener becomes the ONLY double-click path, and it
      never reads the event position. AM7: unit tests drive the listener in JUCE's verified order, including a rebuild
      between the two clicks and one inside click 2. AM8: G1b drives real dispatch on screen. The active-tab no-op
      stays.
UX-A2 [MUST] ACCEPT, with the evidence corrected. Focus is not left empty. It lands on column trigger "1", and the
      next Return fires column 1 (E-R4). After a click-away on a cell, it lands on the top strip's X (E-R5).
      AM4: every close hands focus to MainComponent, BEFORE the hide, whenever focus is inside DeckView or nowhere.
      AM6: a focus_home_count witness. AM8 P5-P9: real focus checks. B3 amended.
      Part REJECTED: a REST read of "the focused component", because it could not tell pass from fail. The gate app
      runs in the background (open -g), and a background standalone app grants focus to nothing
      (juce_NSViewComponentPeer_mac.mm:1562-1567).
UX-A3 [MUST] ACCEPT IN PART.
      Accepted: the deliberate-gesture rule. A double-click renames only if its FIRST click landed on the tab that was
      already on screen (AM2; Boris Q1 can flip it). The box's tooltip "Enter keeps the new name, Esc cancels" (AM3).
      The accidental double-click scenario goes into B1 / B3 and the critic brief (AM14, AM17). The 8-px slop the
      fix asks for already exists in JUCE (E-R3 S4).
      Rejected: an idle timeout, and "close on the first clip hotkey". Every letter is both a name character and a
      possible clip key, so no rule can tell them apart. A timer would add state for a case the gesture rule already
      removes, and Esc or one click recovers.
UX-A4 [MUST] ACCEPT. AM14 adds: numeric bars NB1-NB3 (the box sits on its tab and inside the row; text widths; the
      button text fits; the name never runs under the button), the click-target matrix, the blocking rule, C5 at the
      row end, C15 (the focused box from G1b), and the grep gate for the hook.
      "Never truncated at the minimum supported inspector width" cannot be met as written, because Boris can drag the
      panel down to 120 px (MainComponent.h:497). It is replaced by a measured 200-px bar per line, which fits any
      inspector down to about 215 px.
UX-A5 [SHOULD] ACCEPT IN PART.
      Accepted: a video gets two info lines (codec, then size + rate), each at most 200 px at the label font (E-R10,
      AM9).
      Rejected: a height computed from a runtime text measure (a fixed 18 px per line is exact and testable).
      Rejected: a button width computed from the font. "Show in Finder" is 84.7 px at the 14-px button font, so
      104 px leaves 9.6 px on each side, and a unit test guards it (AM9).
UX-A6 [SHOULD] ACCEPT IN PART.
      Accepted: B6 asks whether the tooltip ever covers a cell Boris needs (AM17).
      Rejected: a codec badge on every video cell. It was not requested: Boris took "codec in the inspector and the
      cell tooltip" (binding-decisions.md:582), and cell painting belongs to bf1 / bf9b.
      Rejected: a tooltip line pointing at the inspector button (the button is in plain view whenever the clip is
      inspected).
      Rejected: a tooltip that changes once it is "familiar" (state with no owner).
UX-A7 [SHOULD] ACCEPT, with no Boris question needed. AM10: the menu opens on a real right-button press only.
      Ctrl+left keeps triggering / selecting exactly as on main (ClipCell.cpp:220-233), so nothing regresses. The tab
      row's Ctrl+click menu stays; it never had a performance meaning.
UX-A8 [SHOULD] ACCEPT IN PART.
      Accepted: Tab is now explicit and tested (E-R8; AM3: Tab and Shift+Tab keep the name and are consumed). C5 moves
      to the last 60-px tab, with a numeric bar (AM14).
      Rejected: a name-length cap. The dialog route has none, and the two routes must not disagree; the box scrolls
      and the tab clips.
      Rejected: a narrower box. A 60-px box shows about 6 characters at 14 px ("Deck 12" alone is 47.4 px).
      Rejected: fixing R11 in this lane. It is pre-existing, not destructive, and lives in saveDeckAs, which this lane
      does not touch. It goes to Harmony as X4. It is not a product choice, so there is no Boris question.
UX-A9 [SHOULD] REJECT the extra question. It offers a surface Boris did not ask for, and the BF3 defaults were taken
      with no objection (binding-decisions.md:582). Boris comments freely on the B-checks page; Q1-Q3 are the only
      open product forks in this lane.
UX-A10 [NIT] ACCEPT. AM5: the tab tooltip becomes three short lines, and both tooltips use the same "Double-click:" /
      "Right-click:" verbs.
G-A1 [MUST] ACCEPT. The headless variant is not possible (E-R1: the mac peer's hit test drops off-screen events). The
      on-screen, in-process variant works (E-R2 / E-R3) and becomes G1b (AM8), RED(stub). B1 stays as Boris's
      acceptance check.
      Replaced detail: "map x to a tab with DeckTabRow::layout". A listener that reads the double-click's position
      gets the dead tab's local coordinates (E-R3 S1). AM2 identifies the tab from the mouseDown's originalComponent
      instead.
G-A2 [MUST] ACCEPT. AM6: tab_click runs the tab button's own onClick (the std::function that Button::mouseUp
      reaches), and a new tab_dblclick runs the listener in JUCE's order. AM7 (a2): with a rebuilding onDeckSwitched
      (the MainComponent.cpp:5550 behaviour), the button count stays stable, builds go +0 on the showing tab and +1 on
      another. The fix's "editor opens on a non-active tab" half is superseded by AM2's rule (a): the test asserts the
      editor does NOT open, and answer (b) flips that.
G-A3 [MUST] ACCEPT. AM9 test (b): the same Image clip with mediaFile empty vs set (+18), and the same video clip with
      the fake source empty vs known (18 vs 36). On main this toggle cannot change the height, because
      getPreferredHeight reads only isPlayable, transportMode, mediaType and child heights
      (ClipInspector.cpp:1044-1071). So RED(main) is 0; the builder records the actual run value.
G-A4 [MUST] ACCEPT. AM10: a MenuLauncher seam, menuChosen(int) as the single completion path, tests (b2)-(b4), a
      DeckView fan-out test, and REST reveal_clip routed through the cell. Finder itself stays Boris's B4 check
      (test mode only records the reveal).
G-A5 [MUST] ACCEPT. AM13 adds the settle rule (POST ok + load idle). Given E-R13, the players are installed before the
      POST answers, so the rows are deterministic; the 5-s bounded retry is pre-registered anyway.
G-A6 [SHOULD] ACCEPT. AM11: vfrgap ("21.25"), rawrgba ("Uncompressed", 63 x 37) and ts ("MPEG-4 Part 2") join U1.2's
      open test. The fixture named vfr is not a variable-frame-rate test (E-R12). The "no rate" branch is tested by the
      formatter only (no such file can be made, E-R12). R7's G3 claim is corrected. DXV is "table only, Boris B5".
G-A7 [SHOULD] ACCEPT. AM14 (NB1-NB3, the blocking rule, the hook-free binary for every capture except C6 / C7, the
      C6 / C7 method named) and G2b (grep gate).
G-A8 [SHOULD] ACCEPT. The waiter semantics are now cited (E-R13), and R10 reads only after {"ok":true}. AM7 (i): with
      the box open, cancelDeckRename() discards with no callback. The swap itself happens in MainComponent, which
      cannot be unit-tested, so R10 is its live row. AM12 moves the cancel call to after the inspector-nulling block
      (E-R15).
G-A9 [SHOULD] ACCEPT. AM6 adds an undo witness {top, redo_top, index, size}. Rows assert "Rename Deck" on top and
      index deltas, never a hard-coded depth. R4 asserts nothing about the switch step, because bf9b owns switching.
      B2 says a second Cmd+Z may undo the deck switch.
G-A10 [SHOULD] ACCEPT. Section QUOTE -> GATE below. The sequence tooltip IS corrected to "images per second" (UI Text
      Rules, docs/claude/architecture.md:361-362). No test pins the old text (grep), and this lane rewrites that line
      anyway (AM12).
G-A11 [NIT] ACCEPT. G0 and G1 take their baselines first (AM15). G5 and G6 are DROPPED rather than given bars, because
      neither can fail. G5's caret only blinks with keyboard focus, which the background gate app never has (see
      UX-A2). G6's witness times the MESSAGE thread's wait (Renderer.cpp:1741-1748), not the GL thread's. E23 stays
      INFERRED, with its reasoning.

====================================================================================================================
## ARCHITECT RULING (s-rta-1002b)
====================================================================================================================
### AMENDMENTS (each OVERRIDES plan-ui.md where they differ)
AM1 Facts. Replace E6 with E-R5: a cell click moves focus to the top strip's X, and only DeckView's own background
    keeps the editor focused. Add E-R4, E-R8, E-R9 and E-R10 to section 2. E5's consequence stands and is now VERIFIED
    by a run (E-R3). Plan R1 is void (see AM8).
AM2 BF8 double-click handling: ONE DeckView-level listener, never reading positions, with the deliberate-gesture rule.
    This replaces U3.2's DeckTabButton::onDoubleClick / mouseDoubleClick (DeckTabButton stays exactly as on main) and
    the plan's OutsideClick listener.
    DeckView.h (private):
      struct TabRowMouse : juce::MouseListener {
          explicit TabRowMouse(DeckView& d) : dv(d) {}
          void mouseDown(const juce::MouseEvent& e) override        { dv.tabRowMouseDown(e); }
          void mouseDoubleClick(const juce::MouseEvent& e) override { dv.tabRowDoubleClick(e); }
          DeckView& dv; };
      TabRowMouse tabRowMouse_ { *this };
      struct TabArm { uint32_t deckId = 0; bool wasShowing = false; bool valid = false; };
      TabArm firstClick_, armed_;
      static constexpr bool kRenameOnlyTheShowingTab = true;   // Boris Q1 default (a); answer (b) => false
      int  tabIndexOf(const juce::Component* c) const;         // index in deckTabs_, else -1
      void tabRowMouseDown(const juce::MouseEvent&);
      void tabRowDoubleClick(const juce::MouseEvent&);
    Public: juce::MouseListener& tabRowMouseForTests() { return tabRowMouse_; }
    Constructor: addMouseListener(&tabRowMouse_, true). New: ~DeckView() override { removeMouseListener(&tabRowMouse_); }
    tabRowMouseDown(e):
      1. If renaming_ and e.originalComponent is neither renameEditor_ nor inside it: finishRename(true) (outside click).
      2. idx = tabIndexOf(e.originalComponent). If idx < 0, or !e.mods.isLeftButtonDown(), or e.mods.isPopupMenu(), or
         composition_ == nullptr: firstClick_ = armed_ = {}; return.
      3. id = decks[idx].id.
         If e.getNumberOfClicks() <= 1: firstClick_ = { id, idx == activeDeckIndex, true }; armed_ = {}.
         Otherwise: armed_ = (firstClick_.valid && firstClick_.deckId == id
                              && (firstClick_.wasShowing || !kRenameOnlyTheShowingTab)) ? TabArm{ id, true, true }
                                                                                        : TabArm{}.
    tabRowDoubleClick(e):
      If !armed_.valid or e.mods.isPopupMenu(): return.
      id = armed_.deckId; firstClick_ = armed_ = {}.
      If renaming_ && renamingDeckId_ == id: return.
      i = index of deck id; if i >= 0: beginRename(i).
      NEVER read e.position or e.getEventRelativeTo: they are wrong once the tab has died (E-R3 S1).
    U3.1 stays (tabClicked ignores the showing tab), but tabClicked becomes PRIVATE: REST now goes through the button
    (AM6).
    Resulting behaviour: double-click the tab on screen -> the rename box. Double-click a tab not on screen -> exactly
    one deck switch and no box (default (a)). Under answer (b): switch, then the box.
AM3 The editor handles its keys synchronously, Tab explicitly, and has no popup menu. This replaces U3.2's
    DeckNameEditor.
    struct DeckNameEditor : juce::TextEditor:
      std::function<void(bool keep)> onClose;
      keyPressed(k):
        if classifyOutputKey(k) is CloseAll, RaiseApp or ToggleMain -> return false (MainComponent acts; the box stays
          open);
        if k.isKeyCode(returnKey) || k.isKeyCode(tabKey) -> onClose(true); return true   (any modifiers; Shift+Tab too)
        if k.isKeyCode(escapeKey) -> onClose(false); return true                       (checked after CloseAll)
        otherwise -> juce::TextEditor::keyPressed(k)
      keyStateChanged(down): juce::TextEditor::keyStateChanged(down); return true;
    Constructor:
      renameEditor_.onClose = [this](bool keep) { finishRename(keep); };
      renameEditor_.onFocusLost = [this] { if (renaming_ && ! renameEditor_.hasKeyboardFocus(true)) finishRename(true); };
        (this callback is posted, E-R9; the focus test ignores a stale one that arrives after a new rename opened)
      onReturnKey and onEscapeKey are NOT used. setPopupMenuEnabled(false). Font juce::FontOptions(14.0f), the tab's
      own font (LookAndFeel.cpp:102). Justification centred. Tooltip "Enter keeps the new name, Esc cancels".
AM4 Focus goes home on every close. New public member on DeckView: std::function<void()> onRenameClosed.
    finishRename(keep):
      if (!renaming_) return;  renaming_ = false;
      text = getText().trim();  id = renamingDeckId_;
      f = juce::Component::getCurrentlyFocusedComponent();
      if ((f == nullptr || isParentOf(f)) && onRenameClosed) onRenameClosed();   // BEFORE the hide (E-R4)
      renameEditor_.setVisible(false);
      if keep: i = index of deck id; if i >= 0 && text is not empty && text != decks[i].name -> onDeckRenamed(i, text).
    If focus is elsewhere (Boris clicked into the BPM field or a browser search box), it is left there.
    MainComponent: member int renameFocusHomeCount_ = 0;
      deckView_->onRenameClosed = [this] { ++renameFocusHomeCount_; grabKeyboardFocus(); };
AM5 Tooltips. tabTooltipFor(deck, bool showing):
      line 1 unchanged (the library note or the path);
      line 2 "Double-click: rename", only on the showing tab (on every tab when kRenameOnlyTheShowingTab is false);
      line 3 "Right-click: Save / Rename / Duplicate / Remove", byte-identical to main (DeckView.cpp:513).
    setupDeckTabs passes showing = (i == activeDeckIndex). Every switch rebuilds the row, so this state is always
    current.
AM6 TEST-ONLY REST (replaces U3.4's list). Reads use the handleDebugUiText Box with a 2-s wait on the message thread;
    actions use callAsync.
    GET  /api/debug/deck_tabs -> {ok, active, row_width, tab_row_builds, focus_home_count,
           undo:{top, redo_top, index, size}, tabs:[{index, id, name, label, tooltip, x, y, w, h, showing}],
           editor:{open, deck_id, deck_index, text, x, y, w, h}}
    POST /api/debug/deck_rename {"deck": i, "op": "begin"|"type"|"enter"|"tab"|"escape"|"focus_lost"|"outside_click",
           "text": "..."}
           begin = beginRename(i); type = setText(text);
           enter / tab / escape = renameEditor_.keyPressed(KeyPress(returnKey | tabKey | escapeKey)), the real key
             path;
           focus_lost = the editor's onFocusLost;
           outside_click = tabRowMouseDown(left button, 1 click, originalComponent = the first ClipCell).
    POST /api/debug/tab_click {"deck": i} -> DeckView::clickTabForTests(i): copy deckTabs_[i]->onClick and invoke the
           copy. This is what Button::mouseUp reaches; the copy outlives a rebuild.
    POST /api/debug/tab_dblclick {"deck": i} -> DeckView::doubleClickTabForTests(i), in JUCE's order (E-R3):
           tabRowMouseDown(n=1, tab i) -> that tab's onClick -> tabRowMouseDown(n=2, tab i as rebuilt) -> its onClick
           -> tabRowDoubleClick(event component = tab i if it still exists, else DeckView). MouseEvents are built with
           Desktop::getMainMouseSource() and the left button.
    POST /api/debug/undo {"redo": bool}: unchanged.
AM7 Unit tests in tests/test_deck_tab_rename.cpp (replace U3.2's (a)-(h)). Headless DeckView, decks "A" "B" "C",
    active 1. A helper in the test file drives tabRowMouseForTests() in JUCE's order, exactly as AM6's tab_dblclick
    does; the test owns its own copy of that order.
    (a)  A click on the SHOWING tab (its onClick) makes 0 onDeckSwitched calls (recording handler). RED(main): 1.
    (a2) With a REBUILDING handler (sets activeDeckIndex, then rebuildGrid, like MainComponent.cpp:5550):
         a showing-tab click -> tabRowBuilds +0 and 3 tab buttons; a tab C click -> +1 and 3 tab buttons.
         RED(main) on the first part.
    (b)  Another tab's click -> exactly one call, with its index (GREEN guard).
    (c)  Double-click the showing tab B -> a visible TextEditor child with text "B" all selected; x / w ==
         editorRect(tab B); y / h == the tab's; font height 14. RED(stub).
    (c2) Rebuilding handler, double-click tab C (not showing) -> active == C, NO visible editor. (Under (b): the
         editor opens on C.)
    (c3) A rebuildGrid() between the two clicks on the showing tab -> the editor still opens on that deck (E-R3 S6).
    (c4) The double-click event arrives with event/original component == DeckView (the tab died) -> the editor opens
         on the armed deck (no positions used).
    (d)  A right or Ctrl double-click -> no editor; a popup-menu mouseDown clears the arm.
    (d2) One click on A, then one click on B -> no editor.
    (d3) A double-click inside the open editor -> no commit, no re-open.
    (e)  All SYNCHRONOUS, no message loop:
         keyPressed(Return) with "  Intro  " -> onDeckRenamed(1, "Intro") once, editor hidden, onRenameClosed once;
         Esc -> no rename, closed, onRenameClosed once; "" + Return -> no rename; "B" + Return -> no rename;
         Tab -> keep; Shift+Tab -> keep; tabRowMouseDown(a ClipCell) -> keep;
         tabRowMouseDown(the editor or one of its children) -> stays open;
         onFocusLost while the editor has no focus -> keep. RED(stub).
    (f)  Begin on C, set active 0, rebuildGrid -> still open, bound to C, x == C's tab x, last child.
         Then remove deck C and rebuild -> closed, no rename, onRenameClosed once. RED(stub).
    (g)  keyStateChanged(false) -> true. keyPressed(Cmd+Shift+Esc), keyPressed(Cmd+`) and keyPressed(Cmd+F) each
         return false and leave the box open. Plain Esc -> closed, no rename. RED(stub).
    (h)  Tooltips: the showing tab's contains "Double-click: rename"; the others' do not; every tab's ends with
         "Right-click: Save / Rename / Duplicate / Remove". RED(main).
    (i)  cancelDeckRename() while open -> closed, no rename, onRenameClosed once. RED(stub).
    (j)  The editor's tooltip == "Enter keeps the new name, Esc cancels", and its popup menu is disabled. RED(stub).
    tests/test_deck_tab_row.cpp, editorRect cases: a 100-px tab -> the same rect; a 60-px tab -> 100 wide; the LAST
    60-px tab at the row end -> shifted left with x + w <= rowWidth; one tab in a 100-px row. In every case the box
    covers its tab (x <= tab.x and x + w >= tab.x + tab.w). RED(stub).
AM8 NEW U3.5: a real-dispatch probe. File tests/probe_deck_tab_dispatch.cpp; CMake target probe_deck_tab_dispatch,
    using the test_deck_thumbnails source/link recipe (tests/CMakeLists.txt:2628-2665) plus src/render/PngWrite.h.
    NO Catch2, and NO add_test / catch_discover_tests: it opens an on-screen window, so it must never run inside
    ctest. Plain main(); exit 0 = PASS, 1 = FAIL, 3 = INCONCLUSIVE. The pattern was proven by the architect's rig
    (E-R2 / E-R3).
    Window:
      a Home component that wants keyboard focus and records keyPressed;
      a real DeckView (3 layers x 12 columns, decks A B C, active 0) placed at (0, -236) inside a 700 x 124 Home, so
      only the bottom layer row and the tab row show;
      setAlwaysOnTop(true); addToDesktop(0); top-left = the primary display's userArea + (40, 60), or --offset X,Y;
      setVisible(true); then one CFRunLoopRunInMode(kCFRunLoopDefaultMode, 0.4, false).
      Never call toFront(true) and never activate the app.
    Wiring:
      onDeckSwitched = set activeDeckIndex + rebuildGrid (as MainComponent.cpp:5550);
      onDeckRenamed = set decks[i].name + refresh;
      onRenameClosed = home.grabKeyboardFocus();
      counters on onColumnTriggered, onLayerClearClip and onClipTriggered.
    Input: ComponentPeer::handleMouseEvent (left button down for 40 ms, clicks 100 ms apart, synthetic times) and
      ComponentPeer::handleKeyPress. After each step, a 0.3-s CFRunLoopRunInMode so posted clicks and focus loss land.
    Precheck: peer->contains(the centre of tabs A, B and C, true) must all be true; otherwise exit 3.
    P1 Click tab C (not showing) -> 1 switch, active 2, 1 rebuild.
    P2 Click tab C (now showing) -> 0 switches, 0 rebuilds.
    P3 Double-click tab A (not showing) -> active 0, no visible editor. Then key 'q' -> Home received "q".
    P4 Double-click tab A (now showing) -> visible editor, text "A" all selected, bounds == editorRect(tab A).
       Save OUT/probe-dispatch-P4.png (a component snapshot of Home, via PngWrite::writeReplacing, Pitfall 46); this
       is capture C15.
    P5 Keys I n t r o, then Return -> decks[0].name == "Intro", editor hidden, focused component == Home.
    P6 Return -> 0 column fires, 0 layer clears, 0 switches; Home received "return".
    P7 Double-click A, key 'K', then a real click on the visible clip cell (layer 0, column 3) -> the clip callback
       ran once, decks[0].name == "K", and focused == Home (not a LayerStrip button).
    P8 Double-click A, key 'X', Tab -> name "X" kept; Home did NOT receive "tab"; focused == Home.
    P9 Double-click A, key 'Y', Esc -> name unchanged; focused == Home.
    RED(stub): land AM2-AM4's new functions as empty stubs; the probe must FAIL (at least P4-P9); then implement until GREEN.
AM9 BF3 info shown as two lines, with text-width bars.
    U1.1: add videoinfo::codecLine(const VideoInfo&) (= the codec) and videoinfo::frameLine(const VideoInfo&)
      ("1920 x 1080, 29.97 frames per second"; with fps 0 -> "1920 x 1080"). describe() = codecLine + ", " +
      frameLine, so its strings do not change. Tests cover both new functions.
    U2.1: Described gains std::vector<juce::String> lines:
      Video known -> {codec, frame}; Video not open -> {"Video file not loaded"}; missing -> {"File missing"};
      Image -> {"<KIND> image"}; ImageSequence -> {"Image sequence, N images"}; anything else -> {}.
      `line` (the one-line form) stays, for the tooltip and REST.
    U2.4: two labels, mediaInfoLabel1_ and mediaInfoLabel2_ (10.5 px, kTextSecondary; "File missing" in red
      0xffcc3333), 18 px each. infoRowHeight() = 18 * lines.size(). PaintKey gets int infoRows (not "infoRow").
      getPreferredHeight adds infoRowHeight(). The name is trimmed by kRevealButtonWidth + 8 when the button shows; a
      pure static helper nameTextBounds(int width, bool buttonShown) feeds paint().
    tests/test_clip_inspector_media.cpp: replace (b), add (g)-(i):
      (b) The same Image clip with mediaFile empty vs set -> +18. The same Video clip with the fake source empty vs
          known -> 18 vs 36. RED(main): 0 (record the run value).
      (g) Under AudioDNALookAndFeel at juce::Font(juce::FontOptions(10.5f)), each of these is <= 200 px:
          "H.264 Constrained Baseline", "ProRes 4444 XQ", "QuickTime Animation",
          "4096 x 2160, 23.976 frames per second", "3840 x 2160, 59.94 frames per second"
          (measured 119.5 / 69.9 / 93.6 / 175.9 / about 172).
      (h) GlyphArrangement::getStringWidth(Font(FontOptions(14.0f)), "Show in Finder") + 16 <= kRevealButtonWidth (104)
          (measured 84.7).
      (i) nameTextBounds(w, true).getRight() <= the button's x - 4, for w in {200, 303, 400};
          nameTextBounds(w, false) == main's withTrimmedLeft(4).withTrimmedRight(40).
      (a), (c), (d), (e), (f) stay as planned, using lines instead of line.
AM10 Cell menu: right button only, behind an injectable seam, with one completion path (U2.2 / U2.3 / U2.6).
    ClipCell::mouseDown: if (event.mods.isRightButtonDown()) { if (clip_ != nullptr &&
      ! clipmedia::menuItems(clip_).empty()) showContextMenu(); return; } and then main's code unchanged (Ctrl+left
      works as before).
    using MenuLauncher = std::function<void(const juce::String& header, const juce::StringArray& items,
                                            std::function<void(int)> done)>;
    void setMenuLauncherForTests(MenuLauncher). The default launcher follows the showDeckTabMenu idiom: a PopupMenu,
      header = the clip name, setLookAndFeel, showMenuAsync withTargetComponent(this) and
      withParentComponent(getTopLevelComponent()).
    showContextMenu() calls the launcher with
      done = [safe = juce::Component::SafePointer<ClipCell>(this)](int r) { if (safe != nullptr) safe->menuChosen(r); }.
    public void menuChosen(int result): if result == 1, clip_ is set and its reveal target exists ->
      onRevealInFinder(layerIndex_, column_).
    DeckView: void revealCellForTests(int layer, int column) -> that cell's menuChosen(1).
      POST /api/debug/reveal_clip -> revealCellForTests, so it goes through the cell, the DeckView fan-out and
      MainComponent::revealClipAt.
    tests/test_clip_cell_media.cpp:
      (a) as planned.
      (b) Ctrl+left on a VIDEO cell still calls onTrigger (GREEN guard; replaces the old (b)).
      (b2) Right mouseDown on a video cell -> the fake launcher received header == the clip name and
           items == {"Show in Finder"}. RED(main).
      (b3) done(1) -> onRevealInFinder(layer, column) exactly once; done(0) -> no call.
      (b4) The cell is destroyed before done(1) -> no call and no crash.
      (c), (d), (e) as planned; (e) pins the AM12 string.
      DeckView fan-out: cell->onRevealInFinder -> DeckView::onRevealInFinder(li, c) exactly once.
      No unit test ever opens a real PopupMenu.
AM11 Codec coverage (U1.2): add three fixtures, with expected values taken from ffprobe (E-R12):
    video_h264_vfrgap_64x64.mp4 -> "H.264 High", 64 x 64, fpsText "21.25";
    video_rawrgba_63x37.mov     -> "Uncompressed", 63 x 37, "30";
    video_mpeg4_64x64.ts        -> "MPEG-4 Part 2", "30".
    That makes 9 real fixtures. The no-rate branch (fps 0) is tested in the formatter only (U1.1). R7 corrected: G3
    covers mp4 / mov; avi, ts and variable frame rate are covered by U1.2. DXV: table row only, checked by Boris (B5).
AM12 Small corrections.
    The sequence tooltip line becomes "Image sequence — N images at X.X images per second" (UI Text Rules). Test (e)
    and V2 pin the new string; APP-INVENTORY row :60 is updated.
    In refreshUiAfterModelSwap, deckView_->cancelDeckRename() goes right AFTER the inspector-nulling block
    (MainComponent.cpp:2933-2937) and before clearSelection / rebuildGrid (E-R15), not as the first statement. It
    reads no model data when it discards.
AM13 G3 settle rule and rows: as written in G3 below (replaces section 5's G3).
AM14 G4 visual gate: as written in G4 below (replaces section 5's G4).
AM15 Baselines and dropped rows. G0 and G1 take their baselines first (gate list). G5 and G6 are deleted. Plan R1 is
    void; plan R10 is unchanged (an accepted, transient cost).
AM16 Docs (amends section 6).
    performance-controls.md, "Deck tab row": double-click the deck on screen = rename in place (Enter, Tab or
      click-away keeps; Esc discards; an empty or unchanged name keeps the old one; one undo step, "Rename Deck"). A
      double-click on a tab that is not on screen just shows that deck. Right-click > Rename Deck... still opens the
      dialog. A click on the deck already showing does nothing (it must not rebuild the row). Every close hands the
      keyboard back to the main window.
    pitfalls.md "Pitfall NN", four points:
      (1) only a component that survives its own onClick gets mouseDoubleClick; clicks are counted per mouse source;
          a parent listener gets the dead child's LOCAL position, so identify the tab by the mouseDown's
          originalComponent;
      (2) JUCE parks focus on the first focusable descendant (a cell click -> the top strip's X; a hidden focused
          editor -> column trigger 1), so hand focus home before hiding;
      (3) TextEditor posts Return, Esc and focus loss as messages, so handle Return / Tab / Esc in keyPressed;
      (4) Cmd+Shift+Esc, key-ups and Tab leak out of a TextEditor.
      Guards: test_deck_tab_rename.cpp, probe_deck_tab_dispatch P1-P9, probe-ui-files-rename R2-R10.
    CLAUDE.md UI bullet: "double-click the deck on screen = rename in place".
    testing-eyes.md: the eight TEST-ONLY routes (clip_media, reveal_clip, inspect_clip, deck_tabs, deck_rename,
      tab_click, tab_dblclick, undo) and the probe binary (not part of ctest).
    APP-INVENTORY: rows :58, :60 and :72, plus the route list.
    Bar unchanged: CLAUDE.md net growth <= 0 bytes.
AM17 The Boris page and questions: see the sections below.
AM18 Findings X1-X4 for Harmony (outside the attacks; no change to lane ui): see below.

### FINAL BUILD STAGES (one builder context each)
U1 BF3 data: U1.1 (+ AM9's codecLine / frameLine) and U1.2 (+ AM11's fixtures).
   Files: src/media/VideoInfo.h (new), VideoPlayer.h/.cpp, tests/test_video_info.cpp, 3 new fixtures, an append to
   tests/CMakeLists.txt.
U3 BF8: U3.1 (tabClicked private); U3.2 as amended by AM2-AM5; U3.3 (+ AM4's wiring, AM12's cancel placement);
   U3.4 = AM6; U3.5 = AM8's probe.
   Files: DeckTabRow.h, DeckView.h/.cpp, MainComponent.h/.cpp, ApiServer.h/.cpp, tests/test_deck_tab_rename.cpp,
   tests/test_deck_tab_row.cpp, tests/probe_deck_tab_dispatch.cpp, an append to tests/CMakeLists.txt.
   U1 and U3 can be built in parallel (they share only the CMake append). U3 alone ships BF8 complete.
U2 BF3 UI, after U1 and U3 are merged (it shares DeckView.*, MainComponent.cpp and ApiServer.*):
   U2.1 (+ AM9's lines, AM12's string); U2.2 / U2.3 / U2.6 = AM10; U2.4 (+ AM9); U2.5 unchanged.
U4 The probe script .harmony/probe-ui-files-rename.sh (G3's rows below) and the docs (AM16). The TEMPORARY C6 / C7
   hook is a scratch patch in a separate build dir and is never committed.
Then G0-G4 run on the merged result, and only then is the Boris page made.

### FINAL GATES (Harmony copies these strings only; any FAIL = a fix round, then that WHOLE gate again)
G0 BUILD + WARNINGS. Take the baseline FIRST, on the merge base: Release build with AUDIODNA_BUILD_TEST_SERVER=ON;
   `touch` every touched file listed below, rebuild, and save the log; count the compiler warnings that name each
   file. Do the same on the branch.
   BAR: both builds exit 0, and for every touched file, branch warnings <= base warnings (a new file counts as 0).
   Touched files: src/media/VideoInfo.h, src/media/VideoPlayer.h, src/media/VideoPlayer.cpp, src/ui/ClipMediaText.h,
   src/ui/ClipCell.h, src/ui/ClipCell.cpp, src/ui/DeckTabRow.h, src/ui/DeckView.h, src/ui/DeckView.cpp,
   src/ui/ClipInspector.h, src/ui/ClipInspector.cpp, src/MainComponent.h, src/MainComponent.cpp,
   src/api/ApiServer.h, src/api/ApiServer.cpp.
G1 UNIT. Take the baseline FIRST: run the full `ctest --test-dir <build>` on the merge base and save the names of the
   passing tests.
   On the branch, run these executables directly: test_video_info, test_clip_media_text, test_clip_cell_media,
   test_clip_inspector_media, test_deck_tab_rename, test_deck_tab_row, test_undo_commands,
   test_clip_inspector_paint_key. BAR: every case passes.
   Then run the full ctest on the branch. BAR: every test that passed on the base also passes on the branch.
   BAR: the builder's commit messages quote the RED run (main or stub) for every new TEST_CASE.
G1b REAL DISPATCH. Run `<build>/tests/probe_deck_tab_dispatch <OUT>` once, in Boris's logged-in GUI session, not at
   the same time as G3 or G4.
   BAR: exit 0, with P1-P9 all PASS on stdout.
   Exit 3 (the precheck found a window covering the strip) -> re-run once with `--offset 40,400`. A second exit 3 is
   INCONCLUSIVE: report it, and never count it as a pass.
   BAR: `ctest --test-dir <build> -N` does not list probe_deck_tab_dispatch.
G2 LINTS. BAR: test_hot_thread_io_lint, test_render_thread_lint, test_log_line_lint and test_shared_field_types all
   exit 0 on the branch.
G2b GREP. BAR: on the merged tree, `grep -rn "AUDIODNA_DEBUG_SHOW" src tests` prints nothing.
G3 LIVE. Run `.harmony/probe-ui-files-rename.sh OUT` (lock held; exactly one live Audio-DNA; open -g with
   --test-mode; no Output window). The composition is the one in plan section 5:
     deck A, layer 0: C0 video_h264_64x64.mp4, C1 video_prores_hq_64x64_2997.mov, C2 video_hapq_64x64_60.mov,
                      C3 video_hevc_main10_64x64_23976.mp4;
     deck A, layer 1: C0 test_card.png, C1 a sequence of 3 PNGs, C2 a missing video in a folder that exists,
                      C3 the source perlin_noise;
     decks B and C = copies of A with fresh ids; active deck 0.
   SETTLE (pre-registered): read rows only after POST /api/load_composition has returned {"ok":true} AND
   GET /api/state shows load.opens_pending == 0, load.staged == 0 and load.queued == 0.
   If a V1 or V4 read shows video == null, re-read every 200 ms for up to 5 s. The verdict is the first non-null read;
   still null at 5 s = FAIL. Print every retry.
   V1 GET /api/debug/clip_media (deck A); exact `line` / `inspector_lines`:
      L0C0 "H.264 High, 64 x 64, 30 frames per second" / ["H.264 High", "64 x 64, 30 frames per second"]
      L0C1 "ProRes 422 HQ, 64 x 64, 29.97 frames per second" / ["ProRes 422 HQ", "64 x 64, 29.97 frames per second"]
      L0C2 "HAP Q, 64 x 64, 60 frames per second" / ["HAP Q", "64 x 64, 60 frames per second"]
      L0C3 "HEVC Main 10, 64 x 64, 23.976 frames per second" / ["HEVC Main 10", "64 x 64, 23.976 frames per second"]
      L1C0 "PNG image" / ["PNG image"]
      L1C1 "Image sequence, 3 images" / ["Image sequence, 3 images"]
      L1C2 "File missing" / ["File missing"], with missing true
      L1C3 "" / [], with file_backed false
   V2 Tooltips:
      L0C0 == "video_h264_64x64.mp4\nH.264 High, 64 x 64, 30 frames per second\nRight-click: Show in Finder";
      L1C1's first line == "Image sequence — 3 images at 2.5 images per second"; L1C3 == "".
      Menus: L0C0 == ["Show in Finder"], L1C2 == ["Show in Finder"], L1C3 == [].
   V3 POST /api/debug/reveal_clip (goes through the cell):
      L0C0 -> last_revealed == the fixture's absolute path, reveal_count +1;
      L1C3 -> reveal_count +0;
      L1C2 -> last_revealed == the missing file's absolute path, reveal_count +1.
   V4 POST /api/debug/inspect_clip:
      L0C0 -> inspector_shows true, inspector_lines == V1's L0C0 lines, inspector_button_visible true;
      L1C3 -> inspector_lines [], inspector_button_visible false.
   R1  GET deck_tabs: 3 tabs A / B / C, active 0, editor closed. tabs[0].tooltip contains "Double-click: rename";
       tabs[1] and tabs[2] do not. Every tooltip contains "Right-click: Save / Rename / Duplicate / Remove".
       undo.size < 90.
   R2  tab_click 0 (the showing tab) -> tab_row_builds +0, active 0.
   R3  tab_click 2 -> tab_row_builds +1, active 2.
   R4  tab_dblclick 1 (not showing) -> active 1, tab_row_builds +1, editor closed (no undo assertion).
       Then tab_dblclick 1 again (now showing) -> tab_row_builds +0, editor open, deck_id == B's id, text "B",
       editor.x == tabs[1].x, editor.y == tabs[1].y, editor.h == 24, editor.x + editor.w <= row_width.
   R5  deck_rename type "  Intro  ", then enter -> decks[1].name == "Intro" (GET /api/composition), editor closed,
       tabs[1].label "Intro", undo.top == "Rename Deck", undo.index == (the read just before R5) + 1,
       focus_home_count +1.
   R6  undo -> decks[1].name "B"; redo -> "Intro" with undo.top == "Rename Deck".
   R7  begin 1, type "X", escape -> "Intro"; begin 1, type "", enter -> "Intro"; begin 1, type "Intro", enter ->
       "Intro". Across R7: undo.index and undo.size unchanged, focus_home_count +3.
   R8  begin 1, type "Y", outside_click -> "Y" (undo.top "Rename Deck", index +1), focus_home_count +1;
       begin 1, type "Z", focus_lost -> "Z", focus_home_count +1;
       begin 1, type "T", tab -> "T", focus_home_count +1.
   R9  begin 0, type "Q", then POST /api/switch_deck {"deck": 2} -> editor still open, deck_id == A's id,
       editor.x == tabs[0].x, tab_row_builds +1. Then enter -> decks[0].name == "Q", decks[2].name unchanged.
   R10 begin 0, then POST /api/load_composition (the same file) -> {"ok":true}; then the editor is closed,
       decks[0].name == "A", and focus_home_count +1.
   BAR: V1-V4 and R1-R10 all PASS.
G4 VISUAL. Window captures by Quartz window id only: the app stays in the background and is never brought to the
   front, and no OS-level synthetic input is used. C1-C5 and C8-C14 are taken from the hook-free shipped binary.
     C1  the tab row, default state;
     C2  the rename box open on the showing tab (REST begin);
     C3  "Intro" typed;
     C4  the renamed tab;
     C5  12 decks (60-px tabs), with the box on the LAST tab (REST begin 11);
     C8  the inspector for the H.264 clip; C9 HAP Q; C10 the missing file; C11 the PNG; C12 the image sequence;
     C13 the procedural source (no info rows, no button); C14 a long clip name next to the button.
     C6 (the cell menu) and C7 (the cell tooltip) are component snapshots (Component::createComponentSnapshot) made
       by a TEMPORARY AUDIODNA_DEBUG_SHOW hook in a separate scratch build (the probe-deck-tabs.sh Phase 2 pattern),
       never committed (G2b checks).
     C15 = G1b's probe-dispatch-P4.png: the box WITH keyboard focus (cyan outline, caret). C2-C5 cannot show that look
       because the background app has no focus.
   NB1 (C5, from deck_tabs): editor.w == 100, editor.x >= 0, editor.x + editor.w <= row_width,
       editor.y == tabs[11].y, editor.h == 24.
   NB2 (C2, from deck_tabs): editor.x == tabs[active].x, editor.w == tabs[active].w (100), editor.h == 24.
   NB3: G1's AM9 (g)-(i) and AM7 (c) PASS (text widths <= 200 px, the button text fits, the name never runs under
       the button, the box font is 14 px). These numbers are quoted in the critic brief.
   Critic panel (independent seats): visual-design, UX, graphic-design, logic, and interaction-logic. The
   interaction-logic seat fills the matrix's "observed" column from the outputs of the probe, G1b and G3, plus its own
   reading of the code.
   A finding is BLOCKING if and only if it shows one of these:
     a pre-registered bar violated;
     text clipped or ellipsized in C8-C14 where NB3 says it fits;
     the rename box off its tab or outside the row;
     the name text running under the button;
     a matrix row whose observed behaviour differs from the expected one;
     an abbreviation, or a non-ASCII glyph on a small button, in any new string (UI Text Rules; Pitfall 6).
   Every other finding is should / nit, and Harmony decides. A blocking finding means: fix round -> re-shoot ->
   re-review.
   BAR: NB1-NB3 PASS, the matrix is complete with no unexpected row, and there are zero blocking findings.
   Only then: ONE Boris page (.harmony/.reports/s-rta-1002b/ui-boris.html).
   Dropped: plan G5 and G6 (neither can fail; see G-A11).

### QUOTE -> GATE (each of Boris's words, and the rows that test it)
"Codec display for each video"
    -> U1.2 (9 real files: H.264, HAP Alpha, ProRes 422 HQ, HAP Q, HEVC Main 10, MPEG-4 in avi / ts, Uncompressed,
       variable rate 21.25), U1.1's table (DXV and the rest), V1, V2, V4, C8-C12, B5.
"easy access to that video in finder"
    -> AM10 (b2)-(b4) and the fan-out test, U2.4 (e), V3, C6, B4 (the real Finder call; test mode only records it).
"Need to be able to rename each deck with double click"
    -> AM7 (a)-(j), G1b P1-P9, R1-R10, C1-C5, C15, B1-B3.
BF8's accepted default "Enter saves, Esc cancels" -> AM7 (e) / (g), P5 / P9, R5 / R7.
BF3's accepted default (the codec line in the inspector and the cell tooltip; Show in Finder in the cell menu and the
    inspector) -> V1-V4, AM9 (g)-(i), AM10.

### CLICK-TARGET MATRIX (the box is open on the showing deck's tab; home = MainComponent)
target                                     | commit                   | the click's own action         | focus after
the box itself                             | none (the caret moves)   | -                              | the box
another deck tab                           | keep (on mouseDown, sync)| that deck shows                | home
"+"                                        | keep                     | the + menu opens               | home
clip cell thumbnail / name bar             | keep                     | the clip fires / is selected   | home
layer strip button (X B S transport)       | keep                     | that button acts               | home
layer strip slider                         | keep                     | the slider moves               | home
column trigger / routine pad               | keep                     | the column / routine fires     | home
DeckView background (corner, end of row)   | keep                     | -                              | home
grid scrollbar                             | keep                     | the grid scrolls               | home
right-click on a tab                       | keep                     | that tab's menu opens          | home, then the menu
inspector / top bar / browser control      | keep (async focus loss)  | that control acts              | where JUCE puts it (left there)
a text field (BPM, browser search)         | keep (async)             | typing goes there              | that field
another app / Cmd+Tab                      | keep (async)             | -                              | the main window on return (X3)
Enter / Tab / Shift+Tab                    | keep (sync)              | -                              | home
Esc                                        | discard (sync)           | -                              | home
Cmd+Shift+Esc / Cmd+` / Cmd+F              | none (the box stays)     | outputs close / app raises / main output toggles | the box
Cmd+Z inside the box                       | none                     | undoes text in the box only    | the box
text keys                                  | none                     | typed into the box; no clip fires; key-ups never reach the momentary sweep | the box
REST / MIDI / OSC / genre deck switch      | none                     | the box stays on its deck, text kept | the box
that deck removed by another path          | discard                  | -                              | home
a composition load lands                   | discard                  | -                              | home
double-click inside the box                | none                     | selects a word                 | the box
double-click a tab that is not on screen   | keep (on click 1)        | one deck switch, no new box (a)| home

### WHAT ONLY BORIS CAN CHECK (replaces section 8)
B1 Double-click the deck tab that is on screen: the name box appears right on the tab with the name selected; typing
   replaces it, and Enter keeps it. Double-click a tab that is NOT on screen: it only shows that deck (double-click it
   again to rename it).
B2 Clicking anywhere else, or Tab, keeps the new name; Esc throws it away; clearing the name keeps the old one.
   Cmd+Z (outside the box) undoes the rename; a second Cmd+Z may also undo the deck switch.
B3 While you type a name, no clip fires and nothing that is playing stops (try a name made of your clip keys). After
   renaming, press a clip key: it fires straight away, with no extra click. Press Return: nothing fires, unless you
   bound Return yourself. Flick through decks with quick double-clicks: no name box ever opens.
B4 Right-click a video cell (two-finger click on a trackpad) -> "Show in Finder" -> Finder opens with that file
   selected. The inspector's "Show in Finder" button does the same. For a missing clip, Finder opens the folder it was
   in. Ctrl+click on a clip still fires it, as before.
B5 The codec lines match what you know about your own files (HAP / HAP Q / ProRes / H.264 / DXV), frame rates
   included.
B6 The hover tooltip on cells: is its timing and text readable while you perform, and does it ever cover a cell you
   need to hit?

### QUESTIONS FOR BORIS (each has a default, so the build never waits)
Q1 "Double-clicking a deck tab renames it. If you double-click a tab that is NOT the deck on screen, should it
   (a) just show that deck, so you double-click it again to rename it, and a quick double-click while you flick
   through decks in a show never opens a name box, or (b) show the deck and open the name box at once?"
   DEFAULT (a). If Boris answers (b): set kRenameOnlyTheShowingTab = false, and flip AM7 (c2), AM5's line 2 (every
   tab), R1, R4's first tab_dblclick (the editor opens) and P3 (the editor opens).
Q2 "When you rename: Enter, Tab or clicking anywhere else keeps the new name, Esc cancels, and an empty name keeps the
   old one, like renaming a file in Finder. OK?"
   DEFAULT yes.
Q3 "For pictures (not videos), the info line just says the kind of file, like 'PNG image'. Do you also want the
   picture's size (for example 1920 x 1080)?"
   DEFAULT no, the kind only.

### FINDINGS FOR HARMONY OUTSIDE THE ATTACKS (AM18; no change to lane ui)
X1 [HIGH, PRE-EXISTING, VERIFIED by run (E-R6)] On main: click any clip cell, then press Return, and the TOP layer's
   clip is cleared and every routine on that layer stops (a routine stop cannot be undone).
   Mechanism: the cell click parks keyboard focus on the top LayerStrip's "X" button (juce_Component.cpp:2667-2679;
   buttons want focus by default, juce_Button.cpp:89; LayerStrip.cpp:990-998), and Return clicks whatever button has
   focus (juce_Button.cpp:665-674).
   Related (E-R7): a click on a tab that is not on screen parks focus on the grid Viewport, so arrow and page keys then
   scroll the grid instead of reaching bindings (INFERRED from Viewport::keyPressed; not run).
   AM4 fixes only the rename paths. Proposed fix, as a bug lane after bf9b (LayerStrip.cpp is bf9b's file): the deck
   view never holds keyboard focus. Call setWantsKeyboardFocus(false) on LayerStrip's flat buttons (one line in
   setupFlatButton), DeckView's column triggers, deck tabs, "+", the undo hint, and gridViewport_. A click anywhere in
   the deck view then gives focus to MainComponent, the first ancestor that wants it (MainComponent.cpp:2306).
   Guard: one more row in probe_deck_tab_dispatch (click a cell, then Return -> 0 layer clears; RED on main).
   For Boris this is a found bug, not a question.
X2 [PRE-EXISTING, VERIFIED from source (E-R8 / E-R9)] Every TextEditor in the app (the TopBar BPM field, browser search
   boxes, RecordPanel, bf2's typed sync value) commits Return / Esc / focus loss one message-loop turn late, lets Tab
   through to the bindings, and lets key-ups through to the momentary sweep (plan R3). AM3's DeckNameEditor is the
   pattern for a shared base class later.
X3 [PRE-EXISTING, INFERRED, not run] At launch, or when the app comes back to the front after the last-focused
   component has gone, JUCE gives focus to the top-level window itself (TopLevelWindow wants focus,
   juce_TopLevelWindow.cpp:51; ComponentPeer::handleFocusGain, juce_ComponentPeer.cpp:362-379). So clip keys may need
   one click first. Worth one probe; it needs the app in front, so it has to be done at a time that suits Boris.
X4 [PRE-EXISTING] Plan R11: a "/" in a deck name makes Save Deck As propose a sub-folder
   (MainComponent.cpp:3623-3626). A one-line fix (juce::File::createLegalFileName); not in lane ui (saveDeckAs is
   untouched here).
LEARNING (for Harmony's log; this project-repo boot cannot write it): real JUCE mouse dispatch can be driven in-process
   with ComponentPeer::handleMouseEvent on a small ON-SCREEN, always-on-top window after one CFRunLoopRunInMode turn.
   Off-screen events are dropped by the mac peer's windowNumberAtPoint hit test. Keyboard focus works in a
   non-standalone process without activating it. Posted callbacks need a run-loop turn. Rig: $TMPDIR/uirig/rig3.cpp
   and $TMPDIR/rig2.sh, which link the session build's test_deck_thumbnails objects.

### RESIDUAL RISKS
- G1b shows a 700 x 124 black strip near the top-left of Boris's main display for under 3 s, and needs nothing to be
  covering that spot (the INCONCLUSIVE rule handles it). It proves dispatch and focus at the JUCE level; the real
  app's OS key-window behaviour is not gated, because the gate app runs in the background. B3 covers that.
- Default (a) means the first double-click on a deck that is not on screen only switches; on a first try Boris may
  read that as broken. Mitigation: the showing tab's tooltip says "Double-click: rename", the Boris page says it, and
  Q1 can flip it.
- X1 stays live until Harmony routes it.
- Merge friction with bf9b in DeckView.h/.cpp (constructor, members, setupDeckTabs) and in MainComponent's wiring.
  Edits stay inside the named blocks, and U2 rebases after U3.
- The two info lines add 18 px per video clip to the inspector.
- tabRowMouseDown runs on every mouseDown inside DeckView; it scans at most about 50 tab pointers, which is negligible.
STRONGEST COUNTERARGUMENT: "Rule (a) departs from the accepted BF8 default ('double-click a deck tab = rename in
place'), and Boris said 'each deck'."
  Why it loses: every deck is still renamed with a double-click on screen, which is where Boris is looking when he
  names it. The only change is for a tab that is not on screen, and there a double-click is most often a habit ("open
  it") or an impatient repeat mid-show. The failure it removes costs a show moment: clip keys typing into a name box
  on stage, then a click committing the garbage as the deck name. The failure it adds costs one more double-click
  during prep. And one constant plus five listed test rows flip it if Boris answers (b).
SECOND: "Keep the button override AND add the listener as a fallback, belt and braces."
  Why it loses: when the tab survives, both paths fire for the same double-click (E-R3 S2 / S3 / S7), giving two
  beginRename calls. One path, proven by G1b, is simpler.

STATUS: COMPLETE — 21 attacks ruled (16 accepted, 4 accepted in part, 1 rejected); 18 amendments; gates G0 G1 G1b G2 G2b G3 G4; 3 Boris questions with defaults; ready_to_build: yes
