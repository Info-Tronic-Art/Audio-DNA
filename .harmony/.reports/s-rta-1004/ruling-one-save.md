# RULING -- lane "one-save": the blind council's 31 attacks on plan-one-save.md (one Save; the show holds its decks, its key and MIDI settings, its window layout and its routines)

Role: architect, ruling (opus at max effort; Fable is out of usage). Read-only. Nothing built, nothing run, no app launched.
Pin: main 185147b (`git rev-parse --short HEAD`); `git status --short -- src tests docs CMakeLists.txt` printed nothing, so every
src / tests / docs line below is a plain-file read at the pin. Worktrees bf2 (740b6d6) and bf2keys (9eab9bd) are clean and were
not read for code (the lane carries nothing from them: PL:835-842 stands).
Papers (4 of 4 seats, 31 attacks, 40377 characters, a closed array, parsed whole): .harmony/.reports/s-rta-1004/attack-one-save-papers.md
Precedence after adoption: this ruling > plan-one-save.md. An amendment (section 3) OVERRIDES the plan body where they differ.
Short names: PL:n = plan-one-save.md line n. BD:n = .harmony/binding-decisions.md line n. BL:n = .harmony/boris-feedback-backlog.md
line n. FO = facts-one-save.md, FS = facts-saves.md, RO = ruling-outputs.md (all in .harmony/.reports/s-rta-1004/). MC = src/MainComponent.cpp.
JUCE = build/_deps/juce-src/modules. Labels: VERIFIED = read by me in the named file today; INFERRED = reasoned from verified
lines, not run; ASSUMED = neither. Boris is quoted only verbatim and only from BD / BL. Amendments are A-1 .. A-17.

## 0 VERDICT
---------------------------------------------------------------------------------------------------
RULED FIRST

(1) WHAT IS IN THE SHOW FILE AFTER THIS LANE, AND WHAT OLDER FILES READ AS.
    - First key, always: "version": 2 (an integer).
    - Then every key `Composition::toVar` writes today (FO Q3 list), minus ONE: "outputDisplay" (dead; handed to this lane by
      the outputs plan). The other six dead keys stay written: nobody asked for their removal (A-4).
    - Two new top-level blocks, added by the file writer and never by `toVar()`: "keys" = exactly `BindingManager::toVar()`'s
      object ({"bindings":[22 fields each],"version":1}); "layout" = {"deckDividerY": int, "vDividerFrac":[f,f,f]}. Both are
      always written (an empty bindings list is fine).
    - Reading the version (A-3): the key counts ONLY when it is an integer of 2 or more. Anything else -- no key, 0, 1, a
      negative number, a string, a real number -- is "no version", and then the ONE grandfathered presence test that exists
      today names the shape: no top-level "layers" array = 0 (old shape), else 1 (the bf9b shape). From 2 on, no reader looks
      at a key's presence to tell a shape. A version of 3 or more opens as today's unknown-key rule opens it (best effort) and
      is copied aside before this build writes over it.
    - An OLD-SHAPE file (his one show) reads as version 0: it opens exactly as at 185147b -- the same in-memory conversion,
      the same one logged note; it has no "keys" and no "layout", so the live keys and the window stay as they are. A
      version-less bf9b file reads as 1 and loads as today. Opening never writes anything.
    - A show whose "keys" or "layout" block is absent, old or unreadable still OPENS; that block is skipped WHOLE.
    - Every write of a show is VERIFIED before it replaces the file (A-1): today's writer can put a cut-off file over the
      show and report success (V1). And before ANY existing file that is not a readable version-2 show is written over, its
      bytes are copied to `<its folder>/backups/<name>.<tag>.json` (tag v0, v1, v3.., or "other"); the copy is verified; if it
      cannot be made NOTHING is written and the one failure box he kept shows ("Save failed: <path>") (A-2).

(2) HOW THE LIST OPENS A SHOW TO ITS DECKS WITHOUT LOADING SHOWS ON THE MESSAGE THREAD, AND HOW A DECK IS TAKEN.
    - The list shows one row per `*.json` of the compositions folder AT ONCE, from the folder listing alone (name and date:
      no file is opened). Nothing is hidden (A-8): a file that turns out not to be a show stays a plain grey one-line row
      with Show in Finder / Delete..., so a damaged show never vanishes from the one screen where he looks for it.
    - The names come from a NEW names-only reader that runs OFF the message thread: one background thread owned by the
      browser reads and parses each new or changed file (path, modification time, size against a cache) and hands back a
      small summary (is it a show, its version, its canvas, its deck names) with `MessageManager::callAsync` behind a
      `SafePointer`; the row then gains its canvas, its triangle and "N Decks". No mutex is added. The proof is a measured
      one: the existing message-thread heartbeat stays at or under 50 ms while 40 shows of about 1 MB are read (OS-L16).
    - A deck is taken by a double-click on its row, or right-click > "Add to This Composition" (question 91). That ONE show
      file is then parsed on the message thread, as every load is today (MC:3488, MC:3654), into a private donor show (an old
      shape is converted in memory, never on disk); the deck keeps its rows up to its last row that holds a clip; its clip
      ids are re-minted; a row that adds a layer to the current show brings layer settings -- the donor show's shared layer
      for a version 1 / 2 donor, the deck's OWN row for an old-shape donor (A-9); media opens off the message thread; ONE
      undoable "Load Deck" step. Not taken: routines, the pad bank, global effects, keys, layout, the other decks.

(3) THE QUIT WINDOW.
    - WHEN: at every quit of a non-test run -- the Quit menu item, the window's close button, Cmd+Q, the Dock's Quit, a logout
      or shutdown. All of them already reach ONE function, `AudioDNAApplication::systemRequestedQuit()` (V9); it stops
      calling `quit()` and calls `MainComponent::requestQuit()`. A test-mode run never asks (the gates' own quit is an Apple
      event that reaches the same function: V10).
    - WHAT: a secondary window titled "Quit!", as his screenshot (read: a titled window with a close button; "Quit" at the
      left, "Cancel" and "Save & Quit" at the right, "Save & Quit" lit). It is the app's own small panel in a dialog window,
      NOT a JUCE AlertWindow (A-10: an AlertWindow has no title bar, centres its buttons, and its focused button presses
      itself on Return -- V11). Esc, the close button and every other way the window goes = Cancel. Return: question 93,
      default "does nothing". No letter key acts.
    - "Save & Quit", the show HAS a file: the show is written to it (verified; backup rule); saved -> the app quits; failed ->
      the "Save failed: <path>" box and the app stays open.
    - "Save & Quit", the show has NO file yet (or its folder is gone): today's Save As chooser opens. Saved -> the app quits.
      Chooser cancelled -> the app stays open, nothing shows. Failed -> the box, the app stays open. A quit asked again while
      that chooser is open is asked again -- never swallowed (A-10; the plan could leave the app unquittable, V13).
    - WHILE A TAKE OR A RECORDING RUNS: the window stops nothing; Cancel leaves both running. "Quit" and "Save & Quit" end
      them exactly as a quit does today: the app's shutdown finalises the take and the video file (FS B8, B9). "Save & Quit"
      writes the show FIRST, then quits. A take whose last save fails during shutdown shows nothing, as today -- named for
      the messages lane (SF-5), not changed here.
    - While a show is still loading: "Save & Quit" saves the show that is LIVE (the old one, until the cut); the quit drops
      the load, as today.

VERDICT ON THE PLAN: SOUND IN ITS CORE, NEEDS REVISION. Of 31 attacks: 23 ACCEPT, 7 PARTIAL, 1 REJECT; all eight MUSTs upheld (seven
ACCEPT; SC-2 PARTIAL on which rows are kept, not on its principle). What stands: the version rule and the two blocks carried outside `toVar()`; one writer (`writeShow`); the
fail-closed copy before an old file is written over; "an empty or broken set is no set"; one apply function with modes exited
and held keys released first; the layout validated whole and clamped to what a drag can give; the off-thread names reader;
taking a deck through today's staged load and ONE InsertDeckCmd; every quit through one function and a pure flow; every
removal with its menu item, code, tests and docs in one stage. What changes: 17 amendments.
- The finding that would have lost a show: the plan's safety premise E1 is false. JUCE's `replaceWithText` drops the result of
  the temporary write and swaps whatever was written (V1). DA-1 is upheld; the same unverified call writes settings.json,
  where this lane now puts his key and MIDI settings.
- The finding that contradicted his words: question 86 IS answered -- Boris: "86 default yes" (BD:898, recorded 2026-10-04
  13:22:56). The plan still read "ANSWERS (none yet)" (PL:522) and struck the stage that removes the old look buttons. S7 is
  now unconditional (A-11).
- The findings that would have shipped unprovable claims: OS-L4 could not fail (GA-1); the quit rows fed a test route and
  never touched the real quit entry or a real button (GA-4, DA-5, SC-2, ST-2); most visual states needed a menu or a
  hover that no gate may make (GA-2).
- FOUR FINDINGS NO SEAT MADE. (a) The settings.json mend is orphaned: the outputs ruling hands it to "the saves lane" (RO
  A-15, HD-5, SF-1) while this plan says the outputs lane mends it (PL:581-583). It lands here (A-15). (b) A menu opened by
  a route in a background test app is dismissed within about 50 ms (MC:2188-2190), so "open each popup by a route" cannot
  be captured either: menus are proven by their models and drawn by a headless tool (A-13). (c) `/api/debug/save_composition`
  refuses a body without a path and answers before the write (V16): the plan's `{}` contract is new, so "RED: the tree
  before S1" on it would fail on a 400, not on the logic (A-12). (d) His four "deck" files are presets of the app as it was
  before decks existed (audio file, effect chain, slots) -- not decks of clips: no hand-wrap into a show exists (V20).

STATUS: DONE (what only a run can establish is in section 8 "FACTS HARMONY MUST MEASURE", each with the ruling per outcome).

## 1 FACTS RE-DERIVED (every seat citation and every plan fact a ruling rests on, re-read or re-computed)
---------------------------------------------------------------------------------------------------
V1  VERIFIED JUCE/juce_core/files/juce_File.cpp:798-803: `replaceWithText` makes a hidden TemporaryFile, calls
    `tempFile.getFile().appendText(...)` and DISCARDS its bool, then returns `overwriteTargetFileWithTemporary()`. :788-796
    `appendText` returns the stream's result. juce_FileOutputStream.cpp:47-51: the destructor flushes with no result;
    :82-86 `flush()` is void; `flushBuffer` (:69-80, private) is the only place a short write is seen. juce_TemporaryFile.cpp:105-130:
    the swap runs whenever the temporary file EXISTS. juce_SharedCode_posix.h:415-437: `replaceInternal` = `rename()`, which
    needs no free space. So a write cut short by a full disk leaves a short temporary file, the rename succeeds, `true` comes
    back. INFERRED end to end (not run): FS item 4 and FS section 5 say the same and mark it UNKNOWN-NEEDS-A-RUN -> M-1.
    PL:35-37 (E1: "a failed write leaves the old file in place") is WRONG for a short write.
V2  VERIFIED src/model/Composition.h:1153-1157 (`saveToFile` = `replaceWithText`), :1159-1168 (`loadFromFile`: empty text or a
    void parse -> false; sets `filePath`). A cut-off JSON does not parse, so the show would not open (MC:3490-3498).
    src/model/AppSettings.cpp:35-41: `update` = read root, set one key, `replaceWithText`; :19-28 a file that does not parse
    to an object reads as EMPTY, so the next update rewrites it without its other keys (RO RF-19 says the same).
V3  VERIFIED all `replaceWithText` callers in src (grep): Composition.h:1156, AppSettings.cpp:40, BindingManager.cpp:297,
    MC:3814 (deck file), MC:7332 (layout), PresetManager.cpp:156 / :528 / :581, Take.cpp:293, AudioStore.cpp:251,
    FilesBrowser.cpp:634, ProjectMPresetManager.cpp:108. This lane removes or replaces the first eight (S1, S2, S3, S4b, S7);
    the last four are other lanes' (SF-4).
V4  VERIFIED src/model/ShowMigration.h:18-22 (`isLegacyShow` = no top-level "layers" array), :104-135 (layer i = the settings
    of the FIRST deck that has row i; a later deck's differing row is "settings dropped" in the one note), :198-217
    (`legacyRowSettings(deckVar)`: per row of a DECK var, the layer settings of a row that carries "type"). A deck inside an
    old-shape show has exactly that row shape (Composition.h:995-998 feeds the same var to `convertShow`, which reads
    `decks[i].layers[r]` with `Layer::fromVar`, ShowMigration.h:84-93). So `legacyRowSettings(decks[i])` returns an old-shape
    deck's OWN row settings with no new code. tests/test_show_model.cpp:1363-1370 (M3 / M6's helper) is its only test caller;
    MC:3670 its only src caller.
V5  VERIFIED his show, read from the project copy .harmony/.reports/s-rta-1004/boris-show-backup/test with harry.json
    (49367 bytes, sha256 5aabe2e2562189d1f421cf4fe00239e62c94f3c49998c1cd31bfd98c32c6b78b; his real file in
    ~/Library/AudioDNA/compositions has the SAME checksum today -- read-only `shasum`, nothing opened for write): no "version",
    no top-level "layers", no "keys", no "layout"; "outputDisplay" and "globalTransitionSpeed" present; routines 0, pad bank 0,
    global effects 0; decks "Deck 1" (id 0, 12 columns, 3 rows, 3 clips in each) and "Deck 2" (id 100, 12 columns, 3 rows: row
    1 none, row 2 six clips, row 3 none). Row 3's settings DIFFER between the decks (blendMode 46 against 1, keyingMode,
    keyThreshold, transitionSpeed); rows 1 and 2 are equal. So: (a) the conversion at 185147b already gives the shared layer 3
    Deck 1's settings; (b) "Deck 2" taken from his show keeps 2 rows after the trailing-row cut, and those two rows' settings
    are the same in both decks -- DA-3's hazard does not bite on HIS file today, and is real for any old-shape show whose
    later deck uses a row it set differently.
V6  VERIFIED his other files (read-only `ls`): ~/Library/AudioDNA/Decks/ = feafeda.deck.json, tes6.deck.json,
    testetst.deck.json, try.deck.json (2026-03-15, about 50 KB each); ~/Library/Audio-DNA/ = settings.json only (206 bytes).
    FO line 5 and its VERIFICATION header are CONFIRMED.
V7  VERIFIED question 86: boris-clarify-86.md:20-21 "86 = default (A), recorded 2026-10-04 13:22:56"; BD:898-899; BL:612-616.
    The plan file is stamped 13:31 and still says "ANSWERS (none yet)" (PL:522). ALSO VERIFIED: plan-effect-looks.md (a
    skeleton, 13:37) carries a heading "EL6 THE OLD BUTTONS GO" -- two lanes are about to plan one removal (H-2).
V8  VERIFIED questions 47, 48, 49, 50 are ANSWERED in the record (BD:864-871; 50 changed BD:879-881; then BD:887-893, 896-897).
    The dispatch lists them as open; the record wins (RO RF-5 found the same). Only 50 touches this lane; what his words do
    not settle is WHEN the computer's copy is written (A-5, question 94).
V9  VERIFIED src/Main.cpp:34-37 (`systemRequestedQuit` -> `quit()`), :87-90 (`closeButtonPressed` -> `systemRequestedQuit()`);
    MC:6606-6608 (the Quit item -> `systemRequestedQuit()`); JUCE/juce_events/native/juce_MessageManager_mac.mm:62-73
    (`applicationShouldTerminate:` calls `systemRequestedQuit()` and answers NSTerminateCancel when no stop message was
    sent). PL E24 CONFIRMED: one function is the gate for every quit.
V10 VERIFIED .harmony/probe-quit-ours.sh:60, :84: a probe quits its own instance with `osascript -e 'tell application
    "Audio-DNA" to quit'` (an Apple event -> `applicationShouldTerminate:` -> `systemRequestedQuit()`), waits up to 30 s, then
    kills its own pid. So every probe run already drives the real quit entry in test mode: if a test-mode build asked, every
    probe would hang 30 s. The bypass must sit inside `requestQuit()` (as PL:425-426 has it) and gets a row of its own (OS-L23).
V11 VERIFIED JUCE/juce_gui_basics/windows/juce_AlertWindow.cpp:125-139 (`addButton(name, returnValue, key1, key2)`; every
    button `setWantsKeyboardFocus(true)`), :530 (the window itself wants focus only when it has NO children), :549-571
    (`keyPressed`: a button's registered shortcut; Esc -> `exitModalState(0)`; Return ONLY when there is exactly one button);
    juce_Button.cpp:665-674 (`Button::keyPressed`: Return presses the button that holds the focus);
    juce_ComponentPeer.cpp:175-221 (a key goes to the focused component first, then up its parents; when that component is
    blocked by a modal one the key goes to the modal component and ITS parents only -- never to MainComponent);
    juce_LookAndFeel_V2.cpp:396-430 (`createAlertWindow` is what gives the stock boxes their Return / Esc shortcuts, and for
    three buttons it gives the first two their FIRST LETTER as a shortcut and the third Esc). MC:3168-3178: `confirmReplaceShow` uses the stock
    `showOkCancelBox`. So ST-2 is CONFIRMED and it is worse than stated: in a hand-built three-button AlertWindow Return is
    handled by whichever button holds the focus. INFERRED, not run: that is the first one added, "Quit".
V12 VERIFIED the screenshots (Read today): boris-resolume/resolume-quit-dialog.png = a window WITH a title bar "Quit!" and a
    close button; an icon; "Quit!" in bold; "Do you really want to quit?" / "All unsaved progress will be lost."; "Quit" at
    the left under the text, "Cancel" and "Save & Quit" at the right, "Save & Quit" lit. resolume-show-decks-list.png = a
    two-line header ("Example" | "1280 x 720"; a triangle + "4 Decks" | "4 Oct 2026 11:54") and four deck rows indented to
    the name's column. PL:300-302 and PL:414-416 read them correctly, except that the plan's AlertWindow has no title bar.
V13 VERIFIED MC: `grep -c 'fileChooser_ = std::make_unique'` = 17 over ONE member (MainComponent.h:433);
    JUCE/juce_gui_basics/filebrowser/juce_FileChooser.cpp:130-133: the destructor drops the callback without calling it. A
    chooser replaced while the flow waits in Saving never reports; PL:429 then swallows every later quit. ST-4 CONFIRMED.
V14 VERIFIED src/core/DeckCommands.h:818-834: InsertDeckCmd records `priorActiveIndex_`, appends, and sets
    `comp->activeDeckIndex = addedIndex_` (the taken deck becomes the SHOWN deck); :866 undo restores the prior shown deck.
    MC:7689-7698: a ByPosition key resolves against the shown deck. ST-5 CONFIRMED. src/ui/CompDecksBrowser.cpp:133-136:
    today one click on a deck row loads it (the gesture the plan kept).
V15 VERIFIED MC:2188-2224 (`/api/debug/ui_test_menu`): the comment records "a background app's PopupMenu is dismissed within
    ~50 ms"; tests/tool_uitoggle_snapshot.cpp:1-11, :33-38: a ctest-EXCLUDED headless tool already renders production widgets
    and PopupMenus with the app LookAndFeel into PNGs ("no window, no peer, no live app"). src/ui/MenuBarModel.h:8-15:
    `AudioDNAMenuBar` is default-constructible and `getMenuForIndex` returns the PopupMenu, so a ctest can list any menu's
    items (tests/tool_uitoggle_snapshot.cpp includes that header and draws menus; tests/test_output_menu_model.cpp names it). src/Main.cpp:56-58: the menu bar is the NATIVE mac one -- not part of
    the app's window, not capturable by window id. GA-2 CONFIRMED.
V16 VERIFIED src/api/ApiServer.cpp:2178-2197: `/api/debug/save_composition` answers 400 without "path" and answers ok BEFORE
    the write (`callAsync`), the result dropped (MC:2151). :2114-2133: `/api/debug/load_deck` takes {path} only.
V17 VERIFIED src/api/MessageHeartbeat.h:8-19 and ApiServer.cpp:1565-1568, :1840-1879: the message-thread stall instrument
    exists (`POST /api/debug/heartbeat`, `/api/state` field `peak_message_stall_ms`, reset on read; `POST
    /api/debug/stall_message_thread` to check the instrument); the rig's bar is 50 ms (.harmony/probe-async-load.json:3,
    probe-media-open.json:3 "stallMaxMs": 50). GA-5 / ST-6's measured arm needs no new instrument.
V18 VERIFIED tests: test_composition.cpp:1203 and :1220 use `Deck::sourceFile`; test_undo_commands.cpp:234-245 compares it in a
    hand-written `operator==`; test_composition.cpp has NO "outputDisplay" (grep: 0 hits; the :386 case sets `crossfader*`,
    `genrePresetNames`, `smartAutopilotEnabled` at :396-399, :431-436 and asserts them at :455-487, :615-626).
    test_show_model.cpp:1362-1369 is the one test use of `legacyRowSettings`; test_comp_decks_browser.cpp is the one of
    `isV2DeckFile`. GA-6 CONFIRMED; PL:174 and PL:501 mislabel :386.
V19 VERIFIED MC:2798-2858: `deckDividerY_` is used through `jlimit` (the member is not rewritten); the three fractions are used
    raw; the preview's width is `vDividerFrac_[0] * bottomAreaWidth_`. MC:7436-7451: the drag's clamp. Main.cpp:53, :64-70: the
    window opens at the primary display's user area, limits 1280..3840 wide -- a probe cannot choose its size. FO Q5's
    "comment at MainComponent.cpp:2541-2545" is the SHUTDOWN comment (read: :2541-2548), not a statement about a zero-size
    preview: "a zero-width preview detaches the GL context" stays UNKNOWN; the plan closes it by never producing one.
V20 VERIFIED src/ui/CompDecksBrowser.h:43-49 and tests/test_comp_decks_browser.cpp:39-55: his `*.deck.json` files are the old
    `PresetManager::saveDeck` shape ("type":"deck", audioFile / fx / slots, audioSourceMode, viewportResolution ...), with no
    "layers" -- a saved state of the single-chain app, not rows of clips. Nothing in today's model matches it. He was told
    they do not load (boris-clarify-86.md:17, reading R43). ST-7's "hand-wrap into one-deck shows" does not exist for them.
V21 VERIFIED src/binding/BindingManager.cpp:52-102 (the action callback runs inside a loop over `bindings_` and the function
    returns right after it), :250-292 (`fromVar` clears FIRST), :300-308; src/binding/Binding.h:10-21 (no device field);
    MC:4101 (Cmd-modified keys never reach bindings), MC:4120-4145, MC:7553-7559, MC:7674-7685. MC:1819 and :1823 are the two
    overlay exit hooks (the plan cites :1818, :1822). No path applies a key set from inside an action callback today
    (callers of the plan's `applyKeySet`: a staged-load completion, the constructor, a chooser callback): SC-7's premise holds.
V22 VERIFIED RO:177, :428-431, :847, :882-885: the outputs ruling does NOT build the settings.json mend and hands it to "the
    saves lane" as its SF-1. PL:581-583 says the outputs lane "mends `AppSettings::update`". Neither lane builds it as written.
V23 VERIFIED src/ui/DeckTabRow.h:39-63 (the two menus as pure lists), src/ui/DeckView.cpp:549-550 ("+" tooltip "New Deck or
    Load Deck...", click -> menu), :569-576 (the tab tooltip names `sourceFile`); src/ui/MenuBarModel.cpp:85-95, :155-169;
    src/ui/BrowserPanel.h:39 (`enum class Tab { Files, FX, Sources, CompDecks, Record, MilkDrop }`); MC:1384-1394, :1493-1502.
V24 VERIFIED MC:3119-3163 (`swapCompositionModel`: called by New, MC:6640, and by a whole-show load, MC:3353; it already stops
    every routine before the swap, :3128); MC:3546-3617 (the three save functions are void / bool as PL E26 says).

## 2 ATTACK RULINGS (one row per attack; "decided by" = the line that settles it; A-n = the amendment in section 3)
---------------------------------------------------------------------------------------------------
| id | verdict | decided by | consequence |
|---|---|---|---|
| DA-1 | ACCEPT (MUST) | V1, V2 | A-1: one verified writer (write, read back, compare bytes, then swap) for the show, the backup copy and settings.json; unit rows SW-1..4, SB-7; the full-disk run is M-1; PL E1 is struck |
| DA-2 | PARTIAL | PL:146-153 against the Harmony constraint "nothing in this lane may make a file he has unreadable" | A-2: ANY existing target that is not a readable version-2 show is copied first, parse or not (tag "other"); a previous copy at EVERY Save is not built here (H-4) -- A-1 removes the short write that motivated it |
| DA-3 | ACCEPT | V4, V5 | A-9: an old-shape donor gives the deck's OWN row settings (`legacyRowSettings(decks[i])` stays); TD-4 and OS-L12 get a fixture where the rows differ; B-1 says what the backup is for |
| DA-4 | ACCEPT | src/ui/CompDecksBrowser.cpp:281-294 (today every `*.json` is a row); V1 | A-8: every `*.json` is a row; a file that is not a show is a grey one-line row with Show in Finder / Delete...; nothing is hidden (simpler than the seat's "hide a recognised deck file") |
| DA-5 | ACCEPT | V9, V10; PL:715-718 | A-10 / A-12: row OS-L17 calls the REAL `systemRequestedQuit()` in an armed test instance and reads "asking"; RED = the mutant that calls `quit()` |
| DA-6 | ACCEPT | BD:771-772 "if the user is changing things around settings, etc., and they do not save the composition, nothing is saved."; BD:896 does not say which show is "the most recent" | A-5: the computer's copy is written only by a successful Save; an Open switches the live set and writes nothing; question 94 puts the choice to him; one line changes for B |
| DA-7 | ACCEPT (NIT) | PL:121-123 "key present -> trusted as is" | A-3: the key counts only as an integer >= 2; row SF-10, mutant MU-OS-22 |
| GA-1 | ACCEPT (MUST) | PL:194-197, PL:682 | A-12: OS-L4 changes an unsaved key first and holds a DIFFERENT set on the computer; mutant MU-OS-20; a RED arm on a new route is a mutant on the stage head -- "the tree before" counts only where the route and its contract exist at 185147b |
| GA-2 | ACCEPT (MUST) | V15 | A-13: menus, the tab tooltip and the "+" are proven by model rows (MB-1..6) and shown to the critics as a headless tool's pictures and a text dump; V8-V11, V13-V15 leave the capture list |
| GA-3 | PARTIAL | V19; CLAUDE.md Pitfall 37, 53 | A-12: fixed expected fractions with no escape clause, `deckDividerY` asserted, every panel width >= 120, a save -> load round trip (OS-L9b). NOT accepted: "`render_frame` cannot see a dead GL thread" -- not established either way (V19); the call stays as a liveness check and proves nothing about the picture |
| GA-4 | ACCEPT (MUST) | V9, V10, V11; PL:426, PL:715-718 | A-10 / A-12: the real entry (OS-L17), a mutant per row, an event log with sequence numbers for "save done" before "quit issued" (OS-L20). The seat's "allow the window in a test build" is NOT taken: no gate shows the window (ST-8); the real panel object is pressed headless |
| GA-5 | ACCEPT | PL:672-673 against PL:710-714; V17 | A-12: `pending` in the route; the 50 ms heartbeat bar; the visibility path exercised; the counter kept as a second witness at ONE choke point |
| GA-6 | ACCEPT | V18 | A-16: Table 2 corrected (both `sourceFile` sites; :386's true content); gate step LINT-8 greps tests/ for every removed name |
| GA-7 | ACCEPT | PL:545, PL:624-625, PL:732-734 | A-12: MU-OS-19 (an empty set replaces); ONE mutant table (section 5); G-OS-HIS hashes its copy at copy time and compares the whole loaded show between the 185147b app and the new one |
| GA-8 | ACCEPT | PL:739-760 against the routes PL:668-676 | A-12: `playing[]` before and after in OS-L11 / L12; a note fired after launch in OS-L5; MB rows for the menus; section 6 keeps only what needs his eyes, ears and controller |
| ST-1 | ACCEPT (MUST) | V7; BD:898 "86 default yes" | A-11: S7 unconditional, before S4b, inside the visual gate (V16); the "no answer" and "B" branches are struck; the gap until per-effect looks exist is told to him |
| ST-2 | ACCEPT (MUST) | V11, V12 | A-10: the quit window is the app's own panel -- result 0 and every exit other than two buttons = Cancel; `choiceFromResult` pure (QF-10); the real buttons and keys are pressed in ctest (QP-1..4) and in live rows without a window |
| ST-3 | ACCEPT | V11; his words name the window, not a key (BD:822-823) | A-17: question 93 stays his, with the consequence written out; the default becomes "Return does nothing"; the lit button stays "Save & Quit" as his picture |
| ST-4 | ACCEPT | V13 | A-10: a quit asked while the flow waits for a save is ASKED AGAIN (Saving -> Asking); a late save completion carries a retired token and does nothing; row QF-9, live OS-L20b, mutant MU-OS-28. Simpler than the seat's liveness check of the chooser |
| ST-5 | ACCEPT | V14 | A-8 / A-17: a deck is taken by a DOUBLE-click (one click on a deck row does nothing); right-click > "Add to This Composition" stays; question 91 offers one click as B |
| ST-6 | ACCEPT | V17 | merged with GA-5: the measured arm uses the existing heartbeat and the rig's 50 ms bar |
| ST-7 | PARTIAL | V20; boris-clarify-86.md:17 (he was told: R43) | accepted: gate row G-OS4-0 names the four files with their checksums and re-checks them before each merge; B-7 repeats the plain fact. REJECTED: the hand-wrap -- these files are not decks of clips (V20) |
| ST-8 | ACCEPT | RIG-RULES.md:53-55; HANDOFF.md:147-149; V15 | A-13: no gate ever shows the quit window; its picture is a headless snapshot of the real panel (tool precedent); a RIG-RULES line says so (H-11) |
| SC-1 | ACCEPT (MUST) | V7 | A-11 (same as ST-1): PresetManager goes WHOLE in S7, so S4b does no surgery on its dead deck pair and tests/test_preset_manager.cpp is deleted, not edited |
| SC-2 | PARTIAL | PL:715-718, PL:730 | accepted: a row without a RED arm is not a gate row; the real wiring gets an armed row. Not accepted: dropping the cancel row -- it is re-made with a mutant (OS-L18). The bare "quit exits" row goes (every probe's own quit is that row: OS-L23 pins it) |
| SC-3 | ACCEPT | PL:139-145 ("loses only on tidiness"); V18 | A-4: only `outputDisplay` leaves the show (the outputs plan's hand-off, plan-outputs.md:849); the six other dead keys stay written; no edit of test_composition.cpp :386 / :519 |
| SC-4 | PARTIAL | BD:887-888 names one command; V12 | accepted: the list's right-click "Import ..." item, `keyCount`, SS-6, V8, V9 go -- Import is ONE command, Shortcuts > "Import Settings from Show...". REJECTED: no double-click -- opening a show by a right-click menu alone is a step back from today's one click; the double-click stays (SL-3 pins what its first click does) |
| SC-5 | PARTIAL | PL:769-774 | accepted for 92: the order is "as today", Harmony's decision H-3, not asked. REJECTED for 93: his picture fixes which button is lit, not what a key does; it stays his (ST-3) |
| SC-6 | PARTIAL | V5 (his one show is old-shape; today a plain Save rewrites it with no copy, FO M-3) | A-14: TWO merge points. M1 = S1 alone (no visible change, so no visual gate): the protection of his show reaches main first. S7 and the deck removals stay behind the visual gate (they change what he sees) |
| SC-7 | ACCEPT (NIT) | V21 | A-6: no dispatch counter, no `isDispatching`, no re-post, no enum filter; KS-5 and KS-6 are struck; one comment says "message thread only, never from an action callback" |
| SC-8 | REJECT (NIT) | Harmony constraint: "The show file gets a VERSION field, so that the next change of shape is never guessed from a key's presence" | SF-7 and the version-3 half of SB-2 STAY: they are the only rows that say what this build does when it meets the next version (opens it, never converts it by key presence, copies it before writing over it). One fixture is the whole cost |

Seats that conflicted, reconciled: GA-4 (let the real window open in a test build) against ST-8 and DA-5 (no window): no
window, the real panel object pressed headless. SC-2 (drop L17 / L18) against GA-4 (give them mutants): mutants. SC-5 (strike
93) against ST-3 (ask it with the consequence): asked. SC-4 (no double-click) against ST-5 (double-click everywhere): the
double-click is the one gesture that acts. SC-1 (S7 before S4b) and ST-1 (S7 before the visual gate): both. SC-8 (drop the
version-3 rows) against DA-7 (tighten the version reader): the reader is tightened and the rows stay.

## 3 AMENDMENTS (numbered; each OVERRIDES the plan body where they differ)
---------------------------------------------------------------------------------------------------
A-1 THE VERIFIED WRITER (DA-1; overrides PL E1, OS1-c, OS1-e's "copied byte for byte").
 - NEW src/core/SafeFileWrite.h (pure juce_core), two functions:
   `bool safewrite::writeTextVerified(const juce::File& target, const juce::String& text)` and
   `bool safewrite::copyVerified(const juce::File& from, const juce::File& to)`.
 - writeTextVerified: (1) a hidden temporary file beside the target (`juce::TemporaryFile`, as today); deleted first if it
   exists (Pitfall 46: a FileOutputStream opens an existing file at its END); (2) the text's UTF-8 bytes go through a
   `juce::FileOutputStream`: `openedOk()`, `write()` true, `flush()`, `getStatus().wasOk()`; the stream is destroyed BEFORE
   step 3; (3) the temporary file is read back and its bytes must EQUAL the bytes written (length and content); (4) only then
   `overwriteTargetFileWithTemporary()`. False at any step: the temporary file is deleted, the target is untouched, the
   function returns false. Byte equality is the proof; no status flag of JUCE's is trusted alone (V1).
 - copyVerified: copy, then `to`'s bytes must equal `from`'s; else `to` is deleted and the function returns false.
 - A seam for the unit rows only: an overload taking the function that writes the temporary file (`safewrite::WriteFn`), so a
   test can model "half the bytes stored, success reported" -- the JUCE behaviour of V1 -- without a full disk.
 - Users in this lane: `Composition::saveToFile` (the show), `showfile::backupBeforeOverwrite` (A-2), `AppSettings::update`
   (A-15). LINT-7 forbids `replaceWithText` in those three files. The other four `replaceWithText` callers (V3) are not
   touched (SF-4).
 - No re-parse of the written show: the serializer is proven by SF-1, SF-2, SF-9; the I/O by the byte comparison.
 - Message thread only. Nothing here runs on the audio callback, the analysis thread or the render thread.

A-2 THE COPY BEFORE AN OVERWRITE (DA-2 in part; overrides PL OS1-e and SB-6).
 - `showfile::backupBeforeOverwrite(target)` reads the TARGET's bytes before any write over it:
   missing or empty -> NotNeeded. Parses to an object with a "decks" array AND `versionOf` == 2 -> NotNeeded. Parses to an
   object with a "decks" array and any other version -> tag "v<N>" (0, 1, 3 ...). ANYTHING ELSE (does not parse, not an
   object, no "decks") -> tag "other". With a tag: `copyVerified` to `<folder>/backups/<base name>.<tag>.json`.
 - As the plan: an existing copy with the same bytes is kept (AlreadyThere); one with other bytes is never overwritten (the
   copy takes the next free "<name>.<tag> (2).json"); if the folder or the copy cannot be made or verified -> Failed, NOTHING
   is written, and the save fails with today's "Save failed: <path>" box. The rule is about the FILE on disk, so it covers
   Save, Save As onto any existing file (one of his old preset files included), Collect Media's show copy and the test route.
 - NOT built: a previous copy at every Save of a version-2 show (H-4).
 - PL R5 stands (the target is read and parsed at every Save, on the message thread): measured, M-2.

A-3 THE VERSION READER (DA-7; overrides PL OS1-a "key present -> trusted as is").
 - `int showfile::versionOf(const juce::var& root)`: when "version" is an INTEGER var of 2 or more -> that number; in every
   other case (absent, 0, 1, negative, a string, a bool, a real number) -> `ShowMigration::isLegacyShow(root) ? 0 : 1`.
 - `Composition::fromVar` calls `versionOf` once: 0 -> the ShowMigration conversion, as today; 1 or more -> the "layers" /
   "decks" reader, as today. A file of version 2 or more is never converted by key presence, whatever keys it has (SF-5).
 - The later-version rule of PL:156-158 STANDS with its rows (SC-8 rejected): version 3+ opens through the same reader,
   unknown keys ignored; A-2 copies it to `.v3.json` before this build writes version 2 over it.
 - PL:126-127 stands: an optional top-level key read under `hasProperty` does not bump the version.

A-4 DEAD KEYS (SC-3; overrides PL OS1-d, SF-8, PL:174-175, PL:501).
 - Only `outputDisplay` goes: the member (Composition.h:196), its write (:745), its read (:892). A file that has it loads;
   the key is ignored. `crossfaderPhase`, `crossfaderBlendMode`, `crossfaderBehaviour`, `crossfaderCurve`,
   `genrePresetNames`, `smartAutopilotEnabled` stay as they are -- fields, writes, reads and the tests that pin them
   (test_composition.cpp:396-399, :431-436, :455-487, :615-626 are NOT edited).
 - tests/test_output_law.cpp:221-229 stays green by construction. The builder greps `outputDisplay` over src first; a reader
   outside Composition.h stops the drop, not the stage.

A-5 KEYS: WHEN THE COMPUTER'S COPY IS WRITTEN (DA-6; overrides PL:187-188, PL:198-200, OS-L1's side effect, PL R3).
 - The computer's copy (settings.json key `AppSettings::kKeys = "keys"`, the same object as the show's block) is written by
   ONE call, `storeKeysOnComputer()`, made only after `writeShow` has written a show successfully. If that settings write
   fails the show still counts as saved; one stderr line; nothing on screen (not one of his five texts).
 - OPEN of a show with a usable set: the live set becomes the show's, whole. Nothing is written anywhere.
 - LAUNCH: the computer's copy is read and made live before any show is opened. "The most recent show" = the show he SAVED
   last (question 94; B = the show he opened or saved last).
 - NEW: the live keys stay (PL:194-197 stands). IMPORT: replaces the live set, writes nothing (PL:206-211 stands, minus the
   list's right-click copy of the command: A-8).
 - The line for question 94 = B: one `storeKeysOnComputer()` call after a successful apply in `finishStagedLoad`'s
   Composition branch (MC:3353-3363). The lines for "remember every key change at once", should he ever ask: the two overlay
   exit hooks, MC:1819 and MC:1823.

A-6 KEYS: APPLYING A SET (SC-7; overrides PL:227-236, KS-5, KS-6).
 - `MainComponent::applyKeySet(const juce::var& block) -> bool`: (1) `BindingManager::parseSet(block)`, pure; NO SET for: a
   void var, anything that is not an object, no "bindings" array, an inner "version" above 1, an EMPTY array -> return false,
   live keys untouched; (2) `exitAllBindingModes()`; (3) `releaseAllMomentary()`; (4) `bindingManager_.replaceAll(set)`.
 - NOT built: the dispatch depth counter, `isDispatching`, the `callAsync` re-post, the per-binding enum filter. One comment
   on `applyKeySet`: message thread only; never call it from inside a binding action (the action holds a reference into the
   list, V21). A number outside a known enum loads as today (it matches nothing).
 - The pre-swap step moves: `exitAllBindingModes()` and `releaseAllMomentary()` run INSIDE `swapCompositionModel`, beside
   `routineEngine_.stopAll()` (MC:3128) and for the same reason -- they hold coordinates into the model that is leaving. That
   covers every whole-show swap, New included (the plan covered only an Open): a learn screen open across New kept targets
   of the old show.
 - `BindingManager::fromVar` becomes parse + replace (a void or broken var leaves the list alone); `saveToFile` /
   `loadFromFile` go with Export / Import Bindings (PL:212-213, PL:247-256 stand).
 - Real-time: all of it is message-thread code; `releaseAllMomentary` uses the existing one-CAS-per-layer release.

A-7 THE WINDOW LAYOUT (GA-3; PL OS3 stands in behaviour; its proof changes).
 - `GET /api/debug/layout` answers {deckDividerY, deckHeight, vDividerFrac[3], panelW[4] (preview, timing, inspector,
   browser), bottomW}. The rows are in section 5: fixed expected values, no "or the clamp's value" escape, `deckDividerY`
   asserted, every panel at least `kMinPanelWidth`, and a save -> load round trip.
 - What stays UNKNOWN and is not claimed: what a zero-width preview does to the GL context (V19). The lane never produces
   one; OS-L10 asserts that.

A-8 THE LIST OF SHOWS (DA-4, SC-4, ST-5, GA-5, ST-6; overrides PL:310-316, :325-339, SS-6, SL-1..SL-4, V8, V9).
 - ROWS: one row per `*.json` of the compositions folder, in today's order (`files.sort()`, CompDecksBrowser.cpp:285). A row
   is in one of three states; every state is 40 px high, so nothing jumps when a summary lands:
   SHOW    line 1: name (left), "W x H" (right); line 2: triangle + "N Decks" ("1 Deck"), date (right).
   PENDING line 1: name; line 2: date (right). No triangle. (The first few tens of milliseconds, until its summary lands.)
   OTHER   as PENDING, the name in the secondary (grey) colour: the file is not a show. It stays listed.
   Under an OPEN show row: one 24 px row per deck name, indented to the name's column, in file order.
 - GESTURES (question 91, default A, built as A): one click on a SHOW row opens or closes its deck rows. A double-click on a
   SHOW row opens the show through today's `confirmReplaceShow`, same text. One click on a deck row does NOTHING. A
   double-click on a deck row takes the deck (A-9). Clicks on PENDING / OTHER rows do nothing. A double-click's first click
   toggles the row and its second click opens the show: one toggle plus one open, pinned by SL-3.
 - RIGHT-CLICK MENUS (`showMenuAsync`, `.withParentComponent(getTopLevelComponent())`): SHOW = "Open", "Show in Finder",
   separator, "Delete..."; deck = "Add to This Composition"; PENDING / OTHER = "Show in Finder", separator, "Delete...".
   There is NO "Import Key and MIDI Settings" item: Import is ONE command, Shortcuts > "Import Settings from Show...".
 - `ShowSummary` (NEW src/core/ShowSummary.h, pure): {isShow, version, canvasW, canvasH, deckNames}. No `keyCount`. It is
   plain data: the parsed JSON tree is built and destroyed on the reader's thread and never crosses to the message thread.
 - No slider is added anywhere in this lane (PL:322 stands); no drag, no hover highlight, no selection.
 - THE READER: ONE function reads and parses a file for the list, `showlist::readSummary(const juce::File&)`; it runs on a
   `juce::ThreadPool` of one thread owned by the browser; the result comes back with `MessageManager::callAsync` behind a
   `Component::SafePointer` and is stored in a message-thread-only cache keyed by (path, modification time, size). Stated
   plainly: the pool has JUCE's own internal lock, taken by the message thread only to add or remove a job and by its own
   worker; no lock of this lane is reachable from the audio callback, the analysis thread or the render thread, and no mutex
   is added to the app's own code. The browser's destructor removes the queued jobs and waits for the running one.
   `readSummary` increments `messageThreadParses` when it runs on the message thread (the ONE choke point; LINT-10 says no
   other code in src/ui/CompDecksBrowser.* parses a file).
 - REFRESH: today's call sites and Delete, plus `visibilityChanged` (the tab becomes visible). Open / closed is kept per path
   for the session, not saved (PL:315-316 stands).
 - THE TEST LIBRARY FOLDER (PL:407-409 stands, made precise): `CompDecksBrowser::getCompositionsDir()` answers a
   process-wide override that MainComponent sets once, only in a test-server build running --test-mode: `AUDIODNA_LIBRARY_DIR`
   when it is an absolute path, else a fresh scratch folder in the temp directory. The Open and Save As choosers call the
   same function, so a test-mode instance never lists, reads or writes his real folder.
 - What goes with it in S4b: PL:373-392 stands, EXCEPT `ShowMigration::legacyRowSettings` (it stays: A-9) and the
   PresetManager pair (gone already, with the whole class, in S7: A-11).

A-9 TAKING A DECK (DA-3; overrides PL:352-360 "rowSettings = the donor show's shared layer r", PL:381-383, TD-4, OS-L12).
 - `compload::deckFromShow(const juce::var& showRoot, int index, const std::string& name) -> std::optional<TakenDeck>`
   (pure, src/core/CompositionLoad.h). It builds the private donor `Composition` itself (`fromVar`), picks the deck (the
   index when its name matches, else the first deck of that name, else nothing), and fills `rowSettings`:
   `versionOf(showRoot) == 0` (old shape) -> `ShowMigration::legacyRowSettings(decks[picked])`: the deck's OWN rows as the
   file has them; version 1 or more -> one entry per row = the donor's shared layer of that row. Then the trailing-row cut
   (PL:354-356), applied to the rows and to `rowSettings` alike.
 - `InsertDeckCmd` is unchanged: an entry is used ONLY for a row that adds a layer, under a fresh layer id (V14).
 - M3 and M6 of tests/test_show_model.cpp stay GREEN UNCHANGED (their helper calls `legacyRowSettings` directly, V4): they
   are regression rows now, not replaced rows.
 - A donor that does not parse, or has no such deck: one log line, no box, nothing added.

A-10 THE QUIT WINDOW (ST-2, ST-3, ST-4, ST-8, GA-4, DA-5, SC-2; overrides PL:427-438, :446-452, QF-1..QF-8, OS-L17..L20).
 - THE FLOW (NEW src/core/QuitFlow.h, pure). States Idle, Asking, Saving. `enum class Choice { Cancel, Quit, SaveAndQuit }`.
   `Output request(bool ask)`; `Output choose(Choice)`; `Output saveDone(uint32_t token, SaveResult)`;
   `static Choice choiceFromResult(int)`.
   request(false), any state -> DoQuit (test mode: never a window).
   Idle + request(true) -> Asking, ShowWindow. Asking + request(true) -> Asking, ShowWindow (the host brings the window
   that is already there to the front). Saving + request(true) -> Asking, ShowWindow, and the save token is RETIRED.
   Asking + Cancel -> Idle. Asking + Quit -> DoQuit. Asking + SaveAndQuit -> Saving, StartSave(token).
   Saving + saveDone(the current token, Saved) -> DoQuit; (Cancelled) -> Idle; (Failed) -> Idle (the save's own box shows).
   A choice outside Asking, a saveDone outside Saving or with a retired token -> Nothing.
   choiceFromResult: 1 -> Quit, 2 -> SaveAndQuit, EVERYTHING ELSE -> Cancel.
 - THE SAVE: `saveComposition` / `saveCompositionAs` gain `std::function<void(SaveResult)> onDone`, called AT MOST once (a
   chooser that is destroyed never calls: V13). Every existing caller passes none and behaves as today.
 - THE WINDOW: NEW src/ui/QuitPanel.h/.cpp -- a plain `juce::Component`: the heading "Quit!", the two message lines, three
   `juce::TextButton`s placed as his screenshot ("Quit" at the left; "Cancel" and "Save & Quit" at the right; "Save & Quit"
   in the accent colour); `std::function<void(QuitFlow::Choice)> onChoice`. The buttons do NOT take keyboard focus; the panel
   does. `keyPressed`: Esc -> Cancel; Return -> nothing (question 93 = B: Save & Quit -- the one line that changes); every
   other key is swallowed (returns true) so no bound key fires. No letter shortcuts. No icon (PL:417 stands).
   Production host: a `juce::DialogWindow` titled "Quit!" (native title bar, close button, not resizable, centred on the
   main window), shown modal and non-blocking (`enterModalState` with a callback, no modal loop -- the show keeps playing,
   PL:441-443 stands). Its close button and any dismissal without a button = Cancel.
 - EVERY QUIT: `AudioDNAApplication::systemRequestedQuit()` calls `MainComponent::requestQuit()` and nothing else;
   `requestQuit()` = `quitFlow_.request(ask)` with ask = not test mode (or the armed test switch below). `quit()` is called
   in src from ONE place, the DoQuit handler (LINT-5).
 - TEST SEAMS (test-server build only): `POST /api/debug/quit {op: "arm", stub_quit, save_path}` -- ask as production does,
   build the REAL QuitPanel with the REAL `onChoice` wiring but place it in NO window; `stub_quit` records DoQuit instead of
   quitting; `save_path` stands in for the chooser of a show with no file ("<hold>" = never completes). `{op: "system_quit"}`
   calls `juce::JUCEApplication::getInstance()->systemRequestedQuit()` on the message thread. `{op: "press", button}`
   clicks the real button; `{op: "key", key: "escape"|"return"}` calls the real panel's `keyPressed`. `GET /api/debug/quit`
   -> {state, last, panel, events[{seq, what}], fileOkAtQuit}. `{op: "disarm"}` returns the instance to the test-mode rule
   (never asks). No gate ever shows the window (A-13).
 - A take or a recording running, a load in flight: as ruled in section 0 (3); PL:439-440 stands.

A-11 THE OLD LOOK BUTTONS: 86 = A (ST-1, SC-1; overrides PL OS6.4, PL:478, PL:549, PL:568).
 - Boris: "86 default yes" -- the question's default A as asked: "Remove the old Save, Load, FX Save and the ten slots now.
   Looks per effect come as their own build." (boris-clarify-86.md:12). The "no answer" and "B" branches are struck.
 - S7 is unconditional and runs BEFORE S4b: it removes the row-1 buttons Save, Load, FX Save; the ten slot pairs of the
   bottom bar; `savePreset`, `loadPreset`, `fastSave`, `loadSlotPreset`, `populateSlotMenu`, `getFastSaveDir`, `PresetSlot`,
   `presetSlots_`, `fastSaveCounter_`; src/ui/PresetManager.h/.cpp WHOLE (its dead deck pair with it);
   tests/test_preset_manager.cpp WHOLE; their CMake entries. The legacy chain, `MappingEngine`, `EffectsRackPanel` stay. His
   `Presets/` and `FX Saves/` folders are left on disk untouched; the app stops creating `FX Saves/` at launch.
 - VERIFIED today (grep): outside its own two files the class is used in code only by MC:387-388, :2969-3009, :4386-4424
   and the include at MainComponent.h:15 -- all of it code S7 removes; `sourceToString` / `curveToString` have no other
   user. Comments that name the class (six files) are re-worded. `ProjectMPresetManager` is another class and stays.
 - Told to him (section 6, B-7), not asked: from this merge until the per-effect looks are built there is no way to save
   or load an effects look. Boris: "every effect has many looks with specific parameter setups. these are saved with the
   app. always." -- that build is the effect-looks lane's.
 - For the record, had he answered B: S7 would be struck and S4b would remove only the dead deck pair, as PL:528-529.

A-12 GATES (GA-1, GA-3..GA-8, ST-6; overrides PL:600-601 "RED arm = the tree before the stage", PL:664-735).
 - A row's RED arm is a NAMED MUTANT built on the stage head from a normal cmake build (RIG-RULES: never a re-signed copy;
   after a mutant, the restore rebuild must compile at least one object and change the binary's checksum). "The 185147b
   app" is a second RED arm ONLY where the route and its contract exist there (named per row).
 - ONE mutant table defines every mutant (section 5). The plan's MU-OS-1..18 keep their numbers; new ones are 19..41.
 - The save route: `POST /api/debug/save_composition {path}` stays (Save As to the path); `{"plain": true}` is NEW (a plain
   Save). Because the route answers before the write (V16), the result is read from `GET /api/debug/show_file` ->
   {path, loadedVersion, lastSave{seq, result: "saved"|"failed"|"cancelled", path, backup: "not_needed"|"made"|
   "already_there"|"failed"}}.
 - Rows added or changed: section 5.

A-13 THE VISUAL GATE (GA-2, ST-8; overrides PL:394-400, :451-452, :567).
 - LIVE captures (the test app's own main window by Quartz window id; `open -g`; scratch library folder; never an Output
   window, no full-screen capture, no synthetic input): V1 the list empty; V2 one show, closed; V3 the same show open, 2
   decks; V4 three shows, the middle one open; V5 a show with 12 decks open (the list scrolls); V6 a 60-character show name
   and a 60-character deck name; V7 the old-shape fixture open ("Deck 1" / "Deck 2"); V7c a file that is not a show, between
   two shows (the grey row); V12 the list at the browser's narrowest width (reached by loading a fixture show whose layout
   puts the third divider at its limit); V16 the whole window after S7, plus crops of row 1 and the bottom bar; V17 the deck
   tab row with its "+". States are reached with `AUDIODNA_LIBRARY_DIR` fixtures, `POST /api/debug/show_list {op: "reveal"}`
   (the browser shows the Compositions tab, BrowserPanel.h:39) and `{op: "toggle"}`.
 - HEADLESS pictures (NEW tests/tool_one_save_snapshot.cpp, ctest-EXCLUDED, the idiom of tests/tool_uitoggle_snapshot.cpp:
   real widgets, the app LookAndFeel, `createComponentSnapshot`; no window, no peer, no app): Q1 the QuitPanel at 1x and 2x;
   M1 the show row's menu; M2 the deck row's menu; M3 the grey row's menu; M4 the tab's right-click menu; M5 the Deck menu;
   M6 the Shortcuts menu; M7 the View menu. The manifest says M5-M7 are drawn by macOS in the real app (same titles, order).
 - TEXT exhibits for the critics: the expected item lists of rows MB-1..MB-4, the two tooltips (MB-5), the window title.
 - STRUCK: V8, V9 (no Import item in the list), V10, V11 (the Delete confirm pre-dates the lane), V13, V14, V15 as captures
   (they are M1..M7 and the text exhibits now), Q2 (a separate window has no "minimum app size" state), and the route
   `POST /api/debug/quit_window`.
 - The critics get: his words verbatim; his two screenshots; the pre-existing texts that are outside the lane ("Save
   failed: <path>", the Open and Delete confirms, "Saved: <file>"); the stated differences from his pictures (no icon in
   the quit window; Audio-DNA's own colours and fonts; today's date format).

A-14 STAGES AND MERGE POINTS (SC-6, ST-1, SC-1; overrides PL OS8 and PL section 4's order).
 - ONE lane, ONE worktree, strictly serial (every stage edits MainComponent.cpp), TWO merges:
   S1 -> MERGE M1 -> S7 -> S2 -> S3 -> S4a -> S4b -> S5 -> VG -> MERGE M2.
 - M1 = S1 alone: nothing he can see changes (no visual gate is owed), and his one old-shape show is protected on main
   days before the list is restyled (today a plain Save rewrites it with no copy: FO M-3).
 - S7 and the deck removals change what he sees: they wait for the visual gate.

A-15 THE SETTINGS FILE (finding (a); the hand-off RO A-15 / HD-5 / SF-1; overrides PL:581-583).
 - `AppSettings::update` writes through `writeTextVerified` (A-1).
 - THE MEND, as the outputs plan specified it (plan-outputs.md:375-378) and its ruling moved it here: when the file exists,
   is not empty and does not parse to an object, `update` first copies it beside itself as `settings.json.unreadable`
   (replacing an older copy, with `copyVerified`) and only then rewrites. If that copy cannot be made, `update` returns false
   and the file is left as it is. Nothing is shown. The existing case "a corrupt file ... rewritten valid" stays true.
 - Lands in S1 (the writer's stage); rows AS-2..AS-4. `kKeys` is added in S2. The outputs lane adds its own key to the same
   header; the second lane to merge re-bases two one-line edits (H-8).

A-16 TABLE 2 AND DOCS (GA-6; overrides PL:498-512 where they differ).
 - ADD to Table 2: tests/test_composition.cpp:1203, :1220 (`sourceFile` in the duplicateDeck case) and
   tests/test_undo_commands.cpp:234-245 (the hand-written Deck `operator==` and its comment) -- edited in S4b when
   `Deck::sourceFile` goes; src/core/CompositionLoad.h:163 with them.
 - CORRECT: tests/test_composition.cpp :386 and :519 are NOT edited (A-4). tests/test_show_model.cpp M3 :1431 and M6 :1556
   stay green unchanged (A-9). tests/test_preset_manager.cpp is deleted whole in S7 (A-11), not edited in S4b.
 - Gate step LINT-8 (run by Harmony after S4b -- S7 is earlier, so every removal has landed -- and again on the merge
   candidate): `grep -rn` over tests/ for each removed name is EMPTY --
   `sourceFile` on a Deck, `isV2DeckFile`, `getDecksDir`, `onDeckLoad`, `writeDeckFile`, the token `PresetManager` (not `ProjectMPresetManager`), `plusMenu`,
   `kShortcutsExportBindings`, `kViewSaveLayout`. The table is checked, not trusted.
 - Docs move in the stage that changes the behaviour (PL OS6.3 stands), plus: CLAUDE.md's capability line that names
   "instant preset save/recall" (S7); docs/claude/integration.md's settings.json keys and the `.unreadable` copy (S1);
   .harmony/RIG-RULES.md's new line (H-11) is Harmony's.

A-17 QUESTIONS (SC-5, ST-3, ST-5, DA-6; overrides PL section 7).
 - None of the plan's three questions was shown to him (no boris-clarify-91 file exists), so they are worded here. 91 is
   re-cut (a deck needs a double-click); 92 is NOT asked (H-3); 93 keeps its number, its default becomes the safe one and
   its consequence is written out; 94 is new. Section 7.

## 4 FINAL STAGES + ORDER (one builder context per stage; every live row, every mutant arm on a live row, the visual gate and every verdict are Harmony's)
---------------------------------------------------------------------------------------------------
One lane (`lane/one-save`), one worktree, strictly in this order. Each stage ends with the full ctest green and its own
unit rows shown RED-then-GREEN in its report (the red arm = the named mutant, applied and reverted by the builder in its own
tree), and is reviewed pinned before the next starts. A builder never launches the app, never runs a live row, never gives a
gate verdict. A builder that finds a reader or a caller the plan and this ruling did not name STOPS with a report.

| stage | owns (files) | proves in ctest (the builder) | Harmony runs, and when |
|---|---|---|---|
| S1 the show file | NEW src/core/SafeFileWrite.h, src/core/ShowFile.h; src/model/Composition.h (version first; `outputDisplay` out; `saveToFile(file, extras)`; `loadedVersion` / `loadedExtras`); src/model/AppSettings.cpp (verified write, the `.unreadable` copy); MC + MainComponent.h (`writeShow`, its three callers, the `lastSave` record); src/api/ApiServer.cpp/.h (`save_composition {"plain"}`, `GET /api/debug/show_file`); NEW tests/test_safe_write.cpp, test_show_file.cpp, test_show_backup.cpp, test_one_save_lint.cpp; tests/test_app_settings.cpp (+3), test_composition.cpp (the three `saveToFile` call sites pass `{}`), tests/CMakeLists.txt; docs (pitfall NN, integration.md, architecture.md); NEW .harmony/probe-one-save.sh (skeleton, L13, L14, L14b, L21); probe-boxes K7's expectation | SW-1..4, SF-1..SF-10, SB-1..SB-7, AS-2..AS-4, LINT-1, LINT-7; MU-OS-1..5, 21..23, 34 | BEFORE S1: the four deck files' checksums and his show's (G-OS4-0 baseline). AFTER S1: G-OS1 (OS-L13, L14, L14b), M-1 (OS-L21), M-2, probe-boxes K7, G-OS-HIS, G-OS4-0 -> MERGE M1 |
| S7 the old look buttons | MC + MainComponent.h (row 1, the slot bar, the seven functions, their members); src/ui/PresetManager.h/.cpp (deleted); CMakeLists.txt, tests/CMakeLists.txt; tests/test_preset_manager.cpp (deleted); docs (CLAUDE.md capability line, effects.md if it names them, APP-INVENTORY) | LINT-6; full ctest; the builder's grep of the token `PresetManager` (not `ProjectMPresetManager`): 0 in src code; in tests only test_comp_decks_browser.cpp, which leaves in S4b | G-OS7's ctest and `strings`; nothing live until VG (V16) |
| S2 keys | src/binding/BindingManager.h/.cpp; src/model/AppSettings.h (`kKeys`); MC + .h (`applyKeySet`, `releaseAllMomentary`, `storeKeysOnComputer`, the launch read, `swapCompositionModel`'s two calls, Import from a show; Export / Import Bindings out); src/ui/MenuBarModel.cpp/.h; src/api/ApiServer (keys, set_keys, import_keys, binding_mode, midi, new_composition); NEW tests/test_key_set.cpp, test_menu_models.cpp (MB-2); tests/test_app_settings.cpp (AS-1); docs | KS-1..KS-4, KS-7, KS-8, AS-1, MB-2, LINT-2; MU-OS-6, 9, 19, 41 | G-OS2: OS-L1..L8c with MU-OS-6, 7, 8, 9, 10, 17, 19, 20, 32, 41 |
| S3 layout | NEW src/ui/LayoutFit.h; MC + .h (snapshot, apply; Save / Load Layout out); src/ui/MenuBarModel.cpp/.h; ApiServer (layout); NEW tests/test_layout_fit.cpp; test_menu_models.cpp (MB-3); docs | LF-1..LF-5, MB-3, LINT-3; MU-OS-11, 38 | G-OS3: OS-L9, L9b, L10, L10b with MU-OS-11, 31, 37, 38 |
| S4a take a deck | src/core/CompositionLoad.h (`deckFromShow`); src/core/StagedLoad.h (`Queued` gains the deck index); MC + .h (`appendDeckFromShow`, its staging, the route's contract; the old deck-file entry stays reachable from the UI until S4b); ApiServer (load_deck fields); NEW tests/test_take_deck.cpp; the three probes' fixtures (PL E23) | TD-1..TD-7; MU-OS-13, 14, 24 | G-OS4-1: OS-L11, L12; probe-asan-live L1, probe-async-load a6, probe-boxes K1a green on show fixtures |
| S4b the list + the removals | NEW src/core/ShowSummary.h, src/ui/ShowListModel.h; src/ui/CompDecksBrowser.h/.cpp; MC + .h (browser wiring; `loadDeck`, `saveDeck`, `saveDeckAs`, `writeDeckFile`, `stageDeckAppend`, `appendDeckFromFile` out; the test library folder); src/ui/DeckTabRow.h, DeckView.cpp, MenuBarModel.cpp/.h; src/model/Deck.h (`sourceFile` out), src/core/CompositionLoad.h:163; ApiServer (show_list); NEW tests/test_show_summary.cpp, test_show_list_model.cpp; tests/test_deck_tab_row.cpp, test_lookandfeel_square.cpp, test_menu_models.cpp (MB-1, MB-4, MB-5), test_composition.cpp:1203 / :1220, test_undo_commands.cpp:234-245; tests/test_comp_decks_browser.cpp (deleted); docs (CLAUDE.md "Deck tab row", performance-controls.md :52 :67, APP-INVENTORY) | SS-1..SS-5, SL-1..SL-6, MB-1, MB-4, MB-5, LINT-4, LINT-9, LINT-10; MU-OS-12, 30, 33 | LINT-8; G-OS4-2: OS-L15, L16 with MU-OS-12, 18, 30, 33; G-OS4-0 |
| S5 the quit window | NEW src/core/QuitFlow.h, src/ui/QuitPanel.h/.cpp; src/Main.cpp; MC + .h (`requestQuit`, the dialog host, the save completion); ApiServer (`/api/debug/quit`); NEW tests/test_quit_flow.cpp, test_quit_panel.cpp, tool_one_save_snapshot.cpp (ctest-excluded); docs (integration.md, recording.md if it names quitting) | QF-1..QF-10, QP-1..QP-4, LINT-5; MU-OS-15, 16, 26, 27, 28, 29, 35, 36, 40 | G-OS5: OS-L17..L20b, L23 with MU-OS-15, 25, 26, 27, 28, 35, 36, 39 |
| VG the visual gate | a capture builder: the live captures and the tool's pictures of A-13, with a manifest of model facts per state; then five critic seats | -- | the verdict; a fix round goes back to the files of S4b / S5 / S7 |
| M2 | -- | -- | full ctest; LINT-8; every G row re-read on the merge candidate; G-OS-HIS and G-OS4-0 again; RED on the pre-merge copy, GREEN after (RIG-RULES merge sequence) |

Order against the lanes beside this one (PL:570-586 stands, with these changes): the settings.json mend is HERE (A-15), not
in the outputs lane; `AppSettings.h` gets one key from each lane (H-8); the nudge lane's binding action lands in the same
serializer `parseSet` reads (H-9); the effect-looks lane plans NO removal of the old buttons (H-2); the messages lane must not
plan on the "Save Deck" / "Load Deck" boxes (they leave in S4b) and inherits SF-5, SF-7, SF-8.

## 5 TESTS + GATE ROWS (pre-registered; each RED first; a bar is met or reported, never loosened)
---------------------------------------------------------------------------------------------------
Harmony copies gate strings ONLY from this section; a row said to "stand" keeps the words of PL:602-661 under the same id.
RED arms. UNIT rows: the case does not compile or fails on the tree before its stage, AND fails under its named mutant.
LIVE rows: the named mutant(s), built on the stage head; "the 185147b app" only where a row names it.

FIXTURES (PL:590-598 stand: F-A, F-B, F-NOKEYS, F-EMPTY, F-BROKEN, F-OLD, F-V1, F-V3, F-DONOR, F-HOST, F-MOM), plus:
F-B2 version 2, no keys, layout {-1, [0.25, 0.50, 0.70]} | F-TIGHT version 2, layout {-1, [0.05, 0.10, 0.15]} | F-NARROW
version 2, layout {-1, [0.30, 0.55, 0.95]} (the browser at its narrowest, for V12) | F-OLD2 old shape, "Deck 1" and "Deck 2"
of 3 rows each, a clip in Deck 2's row 3, Deck 2's row 3 settings opacity 0.4 and blend 7 against Deck 1's 1.0 and 1 |
F-GARBAGE the 10 bytes `not json!!`. None is a file of his; none is written into his folders.

THE MUTANT TABLE (id | the edit | the rows that must fail)
| MU-OS | edit | rows |
|---|---|---|
| 1 | `toVar` omits "version" | SF-1 |
| 2 | `versionOf` answers 1 for every version-less file | SF-3 |
| 3 | `fromVar` runs the old-shape conversion whatever the version | SF-5 |
| 4 | `backupBeforeOverwrite` always answers NotNeeded | SB-1; OS-L13 |
| 5 | a Failed copy is ignored and the write goes on | SB-4; OS-L14 |
| 6 | the live list is cleared FIRST and the block parsed after (today's `fromVar`) | KS-3; OS-L2 (at F-NOKEYS), OS-L3, OS-L7b |
| 7 | the apply call is removed at the cut of an Open | OS-L1 |
| 8 | the launch read is removed | OS-L5 |
| 9 | the release step is removed from `applyKeySet` | KS-7; OS-L7 |
| 10 | `exitAllBindingModes` is removed from `swapCompositionModel` | OS-L8, OS-L8b |
| 11 | `layoutfit::parse` accepts fractions that are zero, unordered or not finite | LF-3; OS-L10 |
| 12 | deck names are read from the top-level "layers" | SS-1; OS-L15 |
| 13 | no trailing-row cut | TD-2; OS-L11 |
| 14 | `rowSettings` left empty | TD-3; OS-L11 |
| 15 | saveDone(Failed) gives DoQuit | QF-5; OS-L19 |
| 16 | saveDone(Cancelled) gives DoQuit | QF-4 |
| 17 | `storeKeysOnComputer` is not called after a Save | OS-L6 |
| 18 | the list's file parse runs inside `refresh()` on the message thread | OS-L16 |
| 19 | an EMPTY bindings array is a set and replaces | KS-4; OS-L2 (at F-EMPTY) |
| 20 | New applies the computer's copy | OS-L4 |
| 21 | `writeTextVerified` swaps without comparing the bytes read back | SW-2; OS-L21 when decidable |
| 22 | `versionOf` trusts any present value | SF-10 |
| 23 | a target that does not parse needs no copy | SB-6; OS-L14b |
| 24 | an old-shape donor's `rowSettings` come from the converted shared layers | TD-4; OS-L12 |
| 25 | `systemRequestedQuit` calls `quit()` | OS-L17 |
| 26 | the Cancel button reports Quit | QP-2; OS-L18 |
| 27 | DoQuit is issued when the save STARTS | QF-6; OS-L20 |
| 28 | a request while Saving is ignored | QF-9; OS-L20b |
| 29 | `choiceFromResult(0)` answers Quit | QF-10 |
| 30 | a file that is not a show is left out of the list | SL-2; OS-L15 |
| 31 | `writeShow` fills "layout" from the Reset values | OS-L9b |
| 32 | an Open writes the computer's copy | OS-L1 |
| 33 | ONE click on a deck row takes it | SL-3; OS-L15 |
| 34 | `AppSettings::update` rewrites an unreadable file without the copy | AS-2 |
| 35 | the Save & Quit button reports Quit | QP-2; OS-L20 |
| 36 | Return presses Save & Quit | QP-3; OS-L18 |
| 37 | `applyLayout` is not called at the cut | OS-L9 |
| 38 | `clampToDrag` returns its input | LF-2; OS-L10b when decidable |
| 39 | `requestQuit` asks in test mode | OS-L23 |
| 40 | a save completion with a retired token quits | QF-9 |
| 41 | `applyKeySet` does not exit the binding modes | KS-7; OS-L8c |

UNIT ROWS (Catch2 case titles are these strings)
- tests/test_safe_write.cpp [safewrite] (S1):
  SW-1 "safewrite: the bytes written are read back equal before the swap; the target then holds them".
  SW-2 "safewrite: a writer that stores half the bytes and reports success leaves the target's bytes unchanged and returns false".
  SW-3 "safewrite: a writer that fails leaves the target unchanged and no temporary file behind".
  SW-4 "safewrite: copyVerified removes a copy whose bytes differ and returns false".
- tests/test_show_file.cpp [showfile] (S1): SF-2, SF-3, SF-4, SF-5, SF-6, SF-7, SF-9 stand.
  SF-1 "showfile: toVar writes version 2 first and no outputDisplay; saveToFile adds keys and layout".
  SF-8 "showfile: a file that has outputDisplay loads; the six other old keys are still written and read".
  SF-10 "showfile: a version of 0, 1, -3, \"x\", true or 2.5 is no version -- with layers the file reads as 1, without as 0".
- tests/test_show_backup.cpp [showbackup] (S1): SB-1..SB-5 stand.
  SB-6 "showbackup: a missing or empty target needs no copy; a target that does not parse, or is not a show, is copied to backups/<name>.other.json".
  SB-7 "showbackup: a copy that cannot be verified is Failed and nothing is written".
- tests/test_app_settings.cpp (the existing seven cases stay green):
  AS-1 (S2) "appsettings: keys round-trips beside outputs and milkDropPresetDir".
  AS-2 (S1) "appsettings: an unreadable file is copied to settings.json.unreadable before the rewrite".
  AS-3 (S1) "appsettings: when that copy cannot be made the file is left as it is and update returns false".
  AS-4 (S1) "appsettings: a half-written update leaves the old file and returns false".
- tests/test_key_set.cpp [keyset] (S2): KS-1, KS-2, KS-3, KS-4, KS-7, KS-8 stand. KS-5 and KS-6 are STRUCK (A-6).
- tests/test_layout_fit.cpp [layoutfit] (S3): LF-1..LF-5 stand.
- tests/test_show_summary.cpp [showsummary] (S4b): SS-2..SS-5 stand. SS-6 is STRUCK.
  SS-1 "showsummary: deck names, count and canvas from a version-2 show".
- tests/test_show_list_model.cpp [showlist] (S4b): SL-5, SL-6 stand.
  SL-1 "showlist: one row per file in today's order; a pending or not-a-show file is a one-line row with no triangle; deck rows only under an open show".
  SL-2 "showlist: a file that is not a show stays listed, grey, with Show in Finder and Delete...".
  SL-3 "showlist: a click on a show toggles it and a double-click opens it (one toggle plus one open); a click on a deck does nothing and a double-click takes it; nothing acts on a pending or not-a-show row".
  SL-4 "showlist: menus -- show: Open, Show in Finder, Delete...; deck: Add to This Composition; other: Show in Finder, Delete...".
- tests/test_take_deck.cpp [takedeck] (S4a): TD-1, TD-2, TD-3, TD-5, TD-6, TD-7 stand.
  TD-4 "takedeck: from an old-shape donor the added layers carry the deck's OWN row settings, not the first deck's" (F-OLD2).
- tests/test_quit_flow.cpp [quitflow] (S5):
  QF-1 "quitflow: request -> Asking + ShowWindow; a second request while Asking shows the window again".
  QF-2 "quitflow: Cancel -> Idle, no quit". QF-3 "quitflow: Quit -> DoQuit".
  QF-4 "quitflow: Save & Quit then saveDone(Cancelled) -> Idle, no quit".
  QF-5 "quitflow: Save & Quit then saveDone(Failed) -> Idle, no quit".
  QF-6 "quitflow: Save & Quit gives StartSave and no DoQuit; saveDone(Saved) gives DoQuit".
  QF-7 "quitflow: request(false) -> DoQuit at once, never ShowWindow".
  QF-8 "quitflow: the texts are Quit! / Do you really want to quit? / All unsaved progress will be lost. / Quit / Cancel / Save & Quit".
  QF-9 "quitflow: a request while Saving asks again; the retired save's completion then does nothing".
  QF-10 "quitflow: choiceFromResult -- 1 Quit, 2 Save & Quit, 0 and every other number Cancel".
- tests/test_quit_panel.cpp [quitpanel] (S5; the real component, no window, no peer):
  QP-1 "quitpanel: the buttons read Quit, Cancel, Save & Quit; Quit is the leftmost and Save & Quit the rightmost".
  QP-2 "quitpanel: each button reports its own choice".
  QP-3 "quitpanel: Esc reports Cancel; Return reports nothing; any other key is swallowed and reports nothing".
  QP-4 "quitpanel: no button wants keyboard focus; the panel does".
- tests/test_menu_models.cpp [menus] (item texts in order, a separator written "--"; `AudioDNAMenuBar::getMenuForIndex`, DeckTabRow):
  MB-1 (S4b) "menus: Deck is New Deck | -- | Rename Deck... | Duplicate Deck | -- | Clear Clips | Remove Deck".
  MB-2 (S2) "menus: Shortcuts is Edit Keyboard Shortcuts... | Edit MIDI Mappings... | -- | Stop All | -- | Import Settings from Show...".
  MB-3 (S3) "menus: View is Reset Layout".
  MB-4 (S4b) "menus: a tab's menu is Rename Deck... | Duplicate Deck | -- | Remove Deck, Remove enabled only with more than one deck".
  MB-5 (S4b) "menus: the tab tooltip ends Right-click: Rename / Duplicate / Remove and names no file; the plus tooltip is New Deck".
- tests/test_one_save_lint.cpp [onesavelint] (source greps, the pattern of tests/test_output_law.cpp):
  LINT-1 (S1) stands ("`saveToFile(` on a Composition is called only inside MainComponent::writeShow").
  LINT-2 (S2), LINT-3 (S3) stand.
  LINT-4 (S4b) "src names no `Save Deck`, `Load Deck...`, `isV2DeckFile`, `getDecksDir`, `writeDeckFile`, `plusMenu`; model/Deck.h names no `sourceFile`".
  LINT-5 (S5) "Main.cpp's systemRequestedQuit calls requestQuit and names no `quit(`; `quit()` is called in src only in the quit flow's DoQuit handler".
  LINT-6 (S7) "src names no `fastSave`, `presetSlots_`, `FX Save`, and no token `PresetManager`" (a token = not preceded by a
  letter: `ProjectMPresetManager` is another class and stays).
  LINT-7 (S1) "Composition.h, AppSettings.cpp and ShowFile.h name no `replaceWithText`".
  LINT-9 (S4b) "the library-folder override is set only under AUDIODNA_TEST_SERVER".
  LINT-10 (S4b) "CompDecksBrowser.cpp parses a file only inside showlist::readSummary".
  (LINT-8 is Harmony's grep over tests/, A-16 -- a gate step, not a ctest case.)
- Regression rows that must stay GREEN UNCHANGED: tests/test_show_model.cpp M2, M3, M6, M7, T6g, T6h, T6i, AS1, AS6;
  tests/test_undo_commands.cpp :1605, :1646; tests/test_routine_engine.cpp :758; tests/test_composition.cpp :386, :519;
  tests/test_staged_load.cpp, test_load_ticket.cpp, test_media_opener.cpp, test_media_presence.cpp; tests/test_output_law.cpp.

LIVE ROWS (probe .harmony/probe-one-save.sh; a test-mode instance by `open -g`, quit by probe-quit-ours.sh, the live lock;
scratch `AUDIODNA_SETTINGS_FILE` and `AUDIODNA_LIBRARY_DIR`; never an Output window, no full-screen capture, no synthetic
input, no window of the quit flow). A row prints exactly `PASS  OS-L<n>: <facts>` or `FAIL  OS-L<n>: <facts>` (or, where a
row says so, `INFO  OS-L<n>: <facts>`). A stage's gate = its rows all PASS on the stage head AND each named mutant FAILs
its row. After every batch: 0 Audio-DNA / Output / UserNotificationCenter windows (the Quartz count), no Audio-DNA process.
Routes (all TEST-ONLY, answered on the message thread): `GET /api/debug/keys` -> {count, bindings[], bindingMode, learnMode,
held, playing[{layer, deck, column}], computerCount, fired[{seq, action, layer, column}]}; `POST /api/debug/set_keys {keys}`;
`POST /api/debug/import_keys {path}`; `POST /api/debug/binding_mode {mode: "keyboard"|"midi"|"off"}`; `POST /api/debug/midi
{note, channel, on}`; `POST /api/debug/new_composition`; `GET /api/debug/layout` (A-7); `POST /api/debug/load_deck {path,
deck, name}`; `POST /api/debug/save_composition {path}` or `{"plain": true}`; `GET /api/debug/show_file` (A-12); `GET
/api/debug/show_list` -> {pending, shows[{name, state: "show"|"pending"|"other", canvas, decks[], open}],
messageThreadParses}; `POST /api/debug/show_list {op: "refresh"|"reveal"|"hide"|"toggle"|"click"|"dblclick", show, deck}`;
`POST /api/debug/quit`, `GET /api/debug/quit` (A-10). Existing: `/api/load_composition`, `/api/trigger_clip`,
`/api/debug/ui_text`, `/api/debug/undo`, `/api/debug/deck_tabs`, `/api/debug/heartbeat`, `/api/debug/stall_message_thread`,
`/api/state` (`peak_message_stall_ms`), `/api/composition` (read ONCE after ui_text shows the load: RIG-RULES A), `/api/render_frame`.

S1
- OS-L13 copy F-OLD to scratch/s/old.json; load it; `save_composition {path: scratch/s/old.json}` -> show_file.lastSave
  result "saved", backup "made"; scratch/s/backups/old.v0.json exists with the ORIGINAL's sha256; old.json parses, its first
  key is "version" and its value 2; `save_composition {"plain": true}` -> backup "not_needed"; the backups folder holds
  exactly 1 file. RED: MU-OS-4; and the 185147b app on the first step (the route and `{path}` exist there; that arm reads
  the folder only): no backups folder.
- OS-L14 the same with a plain FILE named `backups` already in scratch/s2 -> old.json's sha256 unchanged; lastSave result
  "failed", backup "failed". RED: MU-OS-5 (old.json rewritten).
- OS-L14b scratch/s3/x.json = F-GARBAGE; load F-A; `save_composition {path: scratch/s3/x.json}` -> backups/x.other.json holds
  exactly those 10 bytes; x.json is a version-2 show. RED: MU-OS-23 (no copy).
- OS-L21 = M-1, the full-disk run (section 8).
S2 (scratch settings.json seeded with an "outputs" key and NO "keys")
- OS-L1 load F-A -> keys.count 3 and the three (inputType, key / note / cc, action) tuples equal F-A's; computerCount 0; the
  settings file's sha256 unchanged. RED: MU-OS-7 (count 0); MU-OS-32 (computerCount 3).
- OS-L2 then load F-NOKEYS, then F-EMPTY -> count 3 after each, tuples unchanged, each show's deck name in deck_tabs.
  RED: MU-OS-6 (count 0 after F-NOKEYS); MU-OS-19 (count 0 after F-EMPTY).
- OS-L3 then load F-BROKEN ("keys": "x") -> its deck name in deck_tabs; count 3, tuples unchanged. RED: MU-OS-6. (Its broken
  layout is OS-L10's, in S3.)
- OS-L6 `set_keys` (4 bindings); `save_composition {path}` -> the file has "version" 2 and keys.bindings of length 4 with the
  four tuples; the settings file has "keys".bindings of length 4 and its "outputs" value unchanged. (The layout block's
  content is OS-L9b's, in S3.)
  RED: MU-OS-17 (no "keys" in the settings file).
- OS-L4 (after OS-L6, so the computer holds 4) `set_keys` (5 bindings, not saved) -> count 5, computerCount 4;
  `new_composition` -> count 5, the five tuples unchanged; deck_tabs shows one deck. RED: MU-OS-20 (count 4).
- OS-L5 launch with a settings file whose "keys" is F-B's set -> the FIRST read gives count 1, note 41; `midi {41, 1, on}`
  -> the last `fired` entry is TriggerColumn, column 1. Launch with a settings file that has only "outputs" -> count 0.
  RED: MU-OS-8 (count 0 on the first launch).
- OS-L7 stands (PL:688-690: a held momentary note, then `import_keys`). RED: MU-OS-9.
- OS-L7b `import_keys` F-NOKEYS, then F-GARBAGE -> count and tuples unchanged after each; the app answers. RED: MU-OS-6.
- OS-L8 `binding_mode keyboard`; load F-NOKEYS -> bindingMode false; the same with "midi" -> learnMode false. (A show with
  NO keys: only the pre-swap step can close the screen.) RED: MU-OS-10.
- OS-L8b `binding_mode keyboard`; `new_composition` -> bindingMode false. RED: MU-OS-10.
- OS-L8c `binding_mode midi`; `import_keys` F-B -> learnMode false, count 1. RED: MU-OS-41.
S3
- OS-L9 from Reset values, load F-A -> vDividerFrac == [0.30, 0.55, 0.80], each within 0.005; deckDividerY == 300; every
  panelW >= 120; `/api/render_frame` answers 200 with a PNG that decodes (the GL thread is alive; this says nothing about
  the picture). RED: MU-OS-37 (the fractions stay 0.22 / 0.50 / 0.75). No clamp can bite here: the smallest gap, 0.20, is
  above 120 px at the narrowest window the app allows (1280).
- OS-L9b `save_composition {path: scratch/L.json}` -> the file's layout == {300, [0.30, 0.55, 0.80]} within 0.0001; load
  F-B2 -> the live values are {-1, [0.25, 0.50, 0.70]}; load L.json -> the live values are F-A's four. RED: MU-OS-31.
- OS-L10 load F-BROKEN (layout fractions [0, 0, 0]) -> the four values unchanged from before the load; every panelW >= 120.
  RED: MU-OS-11 (a fraction of 0 or a panelW under 120).
- OS-L10b load F-TIGHT -> every panelW >= 120 and the fractions do not decrease. Decidable only while bottomW < 2400 (the
  fixture's 0.05 gap is then under 120 px); on a wider window the row prints `INFO  OS-L10b: not decidable at bottomW <n>`
  and LF-2 carries the proof. RED when decidable: MU-OS-38.
S4a
- OS-L11 load F-HOST; `/api/trigger_clip` layer 0 column 0; `load_deck {F-DONOR, 1, "Wide"}` -> PL:697-702's assertions, AND
  playing[0] is the same (deck, column) as before the take. Then "Narrow", then two `undo`, as PL:700-701.
  RED: MU-OS-13 (layers 6 after "Narrow"); MU-OS-14 (layer 3 opacity 1.0).
- OS-L12 load F-HOST (2 layers); `load_deck {F-OLD2, 1, "Deck 2"}` -> numDecks 2; layers 3; layer 2's opacity 0.4 and blend 7
  (Deck 2's own); the deck's clip count is F-OLD2's; sha256 of F-OLD2 unchanged; no file appears beside it; playing[]
  unchanged. RED: MU-OS-24 (opacity 1.0, blend 1); and the 185147b app (the route exists there with `{path}`): "not a deck
  file", numDecks unchanged.
S4b
- OS-L15 library folder = F-A, F-OLD, a bare deck file, a preset-shaped JSON, F-GARBAGE -> `show_list` polled until pending
  == 0: FIVE rows in today's order; F-A "show" with decks ["A1"]; F-OLD "show" with ["Deck 1", "Deck 2"]; the other three
  "other". `toggle` F-OLD -> open true. `click` its deck row 1 -> numDecks unchanged. `dblclick` its deck row 1 -> numDecks
  + 1. `dblclick` an "other" row -> deck_tabs and ui_text unchanged. RED: MU-OS-12 (wrong deck names); MU-OS-30 (two rows);
  MU-OS-33 (the click adds a deck).
- OS-L16 library folder = 40 show files of about 1 MB each; heartbeat on (4 ms). INSTRUMENT CHECK FIRST: `stall_message_thread
  {300}` -> the poller's max `peak_message_stall_ms` >= 250 (else the row is INVALID and is re-run, not passed). Then
  `refresh`, polling `/api/state` every 15 ms until pending == 0 -> max `peak_message_stall_ms` <= 50 AND messageThreadParses
  == 0. Then `hide`, give the 40 files a new modification time, `reveal` (the visibility path) -> the same two bars.
  RED: MU-OS-18 (messageThreadParses > 0 and a stall above 50).
S5 (the instance armed with `POST /api/debug/quit {op: "arm"}` unless a row says otherwise; every armed launch that is still
alive ends with `{op: "disarm"}` before the probe's own quit, so that quit is the test-mode one)
- OS-L17 `system_quit` -> state "asking", panel true; the pid alive after 3 s; the count of on-screen windows owned by the
  pid unchanged. RED: MU-OS-25 (the pid exits).
- OS-L18 `press cancel` -> state "idle", panel false, the pid alive after 3 s; `system_quit` -> "asking" again; `key escape`
  -> "idle"; `system_quit`; `key return` -> still "asking". RED: MU-OS-26 (the pid exits at cancel); MU-OS-36 (return
  leaves "asking").
- OS-L19 a show with no file; arm with `save_path` = a path that cannot be written; `system_quit`; `press save_quit` -> the
  pid alive, state "idle", last "save_failed". RED: MU-OS-15 (the pid exits).
- OS-L20 (a fresh launch: the default show has no file) arm {stub_quit: true, save_path: scratch/q.json}; `system_quit`;
  `press save_quit` -> the events hold "save_done"
  with a LOWER seq than "quit_issued"; fileOkAtQuit true; q.json parses with "version" 2. Then, on a fresh launch armed with
  stub_quit false: the same presses -> the pid exits within 10 s and q.json parses with "version" 2. And with a show
  LOADED from scratch/f.json (no save_path; stub_quit true): `press save_quit` -> show_file.lastSave result "saved", path
  f.json, "save_done" before "quit_issued".
  RED: MU-OS-27 ("quit_issued" before "save_done", or fileOkAtQuit false); MU-OS-35 (no q.json at the quit).
- OS-L20b arm {stub_quit: true, save_path: "<hold>"}; `system_quit`; `press save_quit` -> state "saving"; `system_quit` ->
  state "asking". RED: MU-OS-28 (still "saving").
- OS-L23 an UNARMED test-mode instance: the probe's own quit (the Apple event of probe-quit-ours.sh) -> the pid exits
  within 5 s and never owns a second window. RED: MU-OS-39 (alive after 5 s; then killed by its pid).

GATE ROWS (Harmony's)
- G-OS4-0 (recorded before S1; re-checked before M1 and before M2; read-only `ls` and `shasum`): his Decks folder holds
  exactly feafeda.deck.json, tes6.deck.json, testetst.deck.json, try.deck.json with the checksums recorded before S1; his
  compositions folder holds "test with harry.json" (today 5aabe2e2...6b78b, V5 -- if HE has saved it since, the new checksum
  is recorded, not treated as a failure); no file of the lane's appears in either folder.
- G-OS1 (after S1): ctest 100 %; `ctest -R "safewrite|showfile|showbackup|onesavelint"` and test_app_settings all pass;
  OS-L13, OS-L14, OS-L14b PASS with their RED arms; M-1 run and ruled by its outcome; M-2 printed; probe-boxes K7 PASS.
- G-OS-HIS (before M1 and again before M2; on COPIES of his show made by Harmony in scratch, hashed at copy time):
  (a) two copies under the SAME file name in two scratch folders. The 185147b app (main's build before the merge),
  freshly launched, loads copy 1 -> `GET /api/composition` once -> A. The new app, freshly launched,
  loads copy 2 -> B. A and B, parsed, are EQUAL after removing "outputDisplay" from A and "version" from B.
  (b) the new app: `save_composition {"plain": true}` -> backups/test with harry.v0.json has the checksum taken at copy
  time; the saved file's first key is "version", value 2. (c) a fresh launch of the new app loads the saved file -> C; B
  and C are equal after removing every clip "id". His real folders: G-OS4-0.
- G-OS2 (after S2): ctest 100 %; OS-L1, L2, L3, L4, L5, L6, L7, L7b, L8, L8b, L8c PASS; MU-OS-6, 7, 8, 9, 10, 17, 19, 20, 32,
  41 each FAIL its row; `strings` of the app holds no "Export Bindings" / "Import Bindings".
- G-OS3 (after S3): ctest 100 %; OS-L9, L9b, L10 PASS with MU-OS-37, 31, 11; OS-L10b PASS or INFO; `strings` holds no
  "Save Layout" / "Load Layout".
- G-OS4-1 (after S4a): ctest 100 %; OS-L11, L12 PASS with MU-OS-13, 14, 24; probe-asan-live L1, probe-async-load a6,
  probe-boxes K1a PASS on show fixtures.
- G-OS4-2 (after S4b): ctest 100 %; LINT-8 empty; OS-L15, L16 PASS with MU-OS-12, 18, 30, 33; `strings` holds no "Save Deck"
  / "Load Deck...".
- G-OS5 (after S5): ctest 100 %; OS-L17, L18, L19, L20, L20b, L23 PASS; MU-OS-15, 25, 26, 27, 28, 35, 36, 39 each FAIL its row.
- G-OS7 (after S7): ctest 100 %; `strings` holds no "FX Save"; V16 rides the visual gate.
- G-OS-VG: the visual gate over the states of A-13 (V1-V7, V7c, V12, V16, V17 live; Q1, M1-M7 from the tool; the text exhibits).

## 6 WHAT ONLY BORIS CAN CHECK (do -> expect -> what wrong looks like) -- replaces PL section 6
---------------------------------------------------------------------------------------------------
- B-1 His own show. Open "test with harry" from the list (double-click, it asks first), press Cmd+S. Expect: it plays as
  before; in Finder, next to it, a folder "backups" holding "test with harry.v0.json" -- the file exactly as it was.
  Wrong: no backups folder; the show looks different after it is opened again.
- B-2 The list. Expect: his show's name with its canvas size at the right, "2 Decks" and its date under it; one click opens
  it to "Deck 1" and "Deck 2"; a double-click on "Deck 2" adds it as a new tab, what was playing keeps playing, and Cmd+Z
  takes the tab away again; a double-click on the show's name asks before opening it. Wrong: one click opens the show or
  adds a deck; a deck arrives empty; extra empty layers appear. To know: after a deck is added the grid shows the NEW deck,
  so his keys fire that deck's clips until he switches back.
- B-3 Keys travel with the show (his controller). Learn two pads in show A, save A. Start a new show: the pads still work.
  Open an older show that has other keys: the pads do what THAT show says. Open "test with harry" (no keys in it): the pads
  keep doing what they did. Quit and start the app: the pads of the show he SAVED last work before any show is open.
  Wrong: pads dead after launch; pads wiped by opening a show that has none; a clip stuck on after opening a show while a
  pad was held; a learn screen left half open after New or Open.
- B-4 Import. In a new show: Shortcuts > Import Settings from Show..., pick show A. Expect: A's keys and MIDI are live.
  Picking a show that has none changes nothing and shows nothing.
- B-5 The window. Drag the dividers, save, open another show saved with other dividers, open the first again. Expect: the
  panels jump to each show's own arrangement and the preview keeps running. Wrong: a panel of zero width; the preview
  black after the jump; an output screen changes at all.
- B-6 Quit -- the FIRST time the real window opens anywhere (no gate may show it). Cmd+Q, the red close button, the menu's
  Quit and the Dock's Quit, each: a window titled "Quit!" as his screenshot. Cancel, Esc and the window's own close button:
  nothing changes, the show keeps playing. Return: nothing. Quit: the app closes. Save & Quit on a show with a file: it
  saves and closes. Save & Quit on a new show: it asks where to save, then closes; cancelling that chooser leaves the app
  open; Cmd+Q again while the chooser is up asks again. While a take records: Cancel leaves it recording. With the main
  window minimised: the quit window still appears. Wrong: the app closes without asking; Save & Quit closes without having
  saved; a quit that can no longer be asked for.
- B-7 What is gone. The Deck menu has no Load Deck / Save Deck / Save Deck As; "+" adds a deck at once; Shortcuts has no
  Export / Import Bindings; View has only Reset Layout; row 1 has no Save / Load / FX Save and the bottom bar no numbered
  slots. To know: until the per-effect looks are built there is no way to save an effects look; the four old files in his
  Decks folder are saved states of the app as it was before decks existed -- nothing opens them, and nothing touches them.
- B-8 By ear and eye on his real screens: opening a show from the list while music plays is as clean as before this lane.

## 7 BORIS QUESTIONS (new numbers; each has a default A; nothing waits on them) -- replaces PL section 7
---------------------------------------------------------------------------------------------------
91. In the list of shows, what do clicks do?
    A (default) One click on a show opens or closes its list of decks. A double-click on a show opens the show (it asks
      first, as today). A double-click on a deck adds that deck to the show you are in; Cmd+Z takes it away again. One
      click on a deck does nothing.
    B The same, but ONE click on a deck adds it. Faster. A click by mistake adds a deck and the grid switches to it, so
      your keys fire that deck until you press Cmd+Z.
    C One click on a show opens the show, as today (it asks first). Only the small triangle opens its list of decks. A
      double-click on a deck adds it.
93. The quit window while you perform. You hit Return:
    A (default) Nothing happens. "Save & Quit" is lit as in Resolume's window, but saving and quitting -- or quitting --
      always takes a click. Esc = Cancel.
    B Return presses "Save & Quit", the lit button. Cmd+Q by mistake and then Return saves the show and closes the app.
94. You open last month's show (it brings its own keys). You do not save. You quit. Next time the app starts, before any
    show is open, which keys are live?
    A (default) The keys of the show you SAVED last. Opening a show never changes what the computer remembers; a Save does.
    B The keys of the show you had OPEN last.
(92 is not asked: the list keeps today's order -- H-3.)
The line each answer changes: 91 B / C -> the gesture table of src/ui/ShowListModel.h (SL-3's expected values with it);
93 B -> one line in `QuitPanel::keyPressed` (QP-3, OS-L18's last step and MU-OS-36 flip with it); 94 B -> one call in
`finishStagedLoad` (A-5; OS-L1's computerCount and MU-OS-32 flip with it).
READINGS TO TELL HIM (not questions; Harmony numbers them): 86 = A is built: the old Save, Load, FX Save and the ten slots
go, and until the per-effect looks exist no effects look can be saved or loaded. The first Save of an old show keeps the
file as it was under "backups" next to it. In his old show each deck had its own layer settings; a show now has one set of
layers and Deck 1's win where they differ (in his file: the third layer's blend and keying, which Deck 2 does not use).
The computer remembers the keys at each Save of a show; a key change he does not save is gone at quit. The list shows every
file of the folder; one that is not a show is grey. His four old files in Decks stay where they are and cannot be opened.

## 8 HARMONY'S DECISIONS (each with a default), and the FACTS HARMONY MUST MEASURE
---------------------------------------------------------------------------------------------------
H-1  Two merges. Default: M1 = S1 alone as soon as G-OS1, G-OS-HIS and G-OS4-0 pass; M2 after the visual gate (A-14). At
     M1 he is told the one thing he can meet from it: the "backups" folder beside an old show after its first Save.
H-2  Who removes the old look buttons: this lane's S7, or the effect-looks lane (plan-effect-looks.md is a skeleton with a
     heading "EL6 THE OLD BUTTONS GO"). Default: S7 here (the inventory is FO Q6; the default he took says "now"); the effect-looks plan
     cites S7 and plans no removal. If she moves it there: S7, LINT-6, V16, G-OS7 and A-16's test_preset_manager line move
     with it, and S4b removes only the dead deck pair (PL:528-529).
H-3  The list's order. Default: as today (`files.sort()`); not a question for him (SC-5).
H-4  A previous copy at EVERY Save of a version-2 show. Default: not in this lane (DA-2); a finding for a later lane.
H-5  Skipping the read of a target this run wrote itself. Default: not built; decided by M-2.
H-6  The pitfall number. Default: the next free one after the outputs lane's (RO HD-6 takes 68).
H-7  The settings.json mend lands HERE (A-15). Default: yes; the outputs lane's builder is told it builds none (RO A-15).
H-8  `AppSettings.h` gets `kKeys` here and `kOutputSettings` in the outputs lane. Default: the second to merge re-bases.
H-9  `BindingManager`'s serializer is shared with the nudge lane's key action (PL:577-580). Default: the second to merge
     re-bases `parseSet` as its step 0.
H-10 A take that cannot be finalised during a quit shows nothing (SF-5). Default: the messages lane rules it; the one
     line where a stop-and-check would go is the quit flow's DoQuit handler.
H-11 A RIG-RULES line: "no gate shows the quit window; its picture is the headless tool's". Default: added at M2.
H-12 Build while 91, 93, 94 are open. Default: yes, as A; each answer's line is named in section 7.
H-13 Import from a show that has no usable set changes nothing and shows nothing. Default: stands (his rule on texts).
H-14 The conversion note on his show names "Deck 2 row 3: settings dropped" in the log only. Default: stands (his ruling of
     2026-10-03 on notices); B-1 and the readings above say it to him once.

FACTS HARMONY MUST MEASURE (each with the ruling per outcome)
M-1 (OS-L21) A FULL DISK. A 2 MB disk image made and mounted by Harmony with `hdiutil ... -nobrowse -mountpoint <scratch>`
    (no Finder window, no Desktop icon), detached at the end (`hdiutil info` names no such image afterwards). On it: a
    version-2 show of about 200 KB; a filler file leaves LESS free space than the show's own size (a Save writes a second
    copy beside the target before the swap); the app loads the show; `save_composition {"plain": true}` (the 185147b
    arm: `{path}` = the show's own path).
    Outcome O1 -- the 185147b app leaves a cut-off file and reports nothing: V1 is confirmed by a run. The new app must
      answer lastSave result "failed" with the show's sha256 unchanged, and MU-OS-21 must fail the row. OS-L21 is then a gate
      row of G-OS1, with the 185147b app as a second RED arm.
    Outcome O2 -- the 185147b app fails closed (the file unchanged, or the save reported failed): the short write does not
      occur in this shape on this rig. A-1 stays (it costs one read-back); OS-L21 is reported INFO and SW-2 / SW-3 carry the
      proof. E1 stays struck either way: the code path of V1 is what it is.
    Outcome O3 -- the image cannot be made or mounted here: reported as a blocked measurement, never passed by default.
M-2 THE COST OF A SAVE. OS-L13 and G-OS-HIS print the time of `writeShow` (target read + parse, copy, write, read-back) on
    F-OLD and on the copy of his show. Under 16 ms: nothing changes. At or above 16 ms on his 49 KB show: H-5 is built (skip
    the target's read when (path, modification time, size) are those this run wrote) and the row is re-run.
M-3 bottomW ON THE RIG (from `GET /api/debug/layout`): decides whether OS-L10b is a gate row or INFO. No ruling changes.
M-4 THE IDLE STALL. Before OS-L16: 10 s of an idle test-mode instance with the heartbeat on -> the max
    `peak_message_stall_ms`. At or under 50: OS-L16's bar is decidable. Above 50 while idle: the row cannot be read on that
    launch -- re-launch on a quiet machine (RIG-RULES A2 "a quiet row"); the bar is not moved.

## 9 SIDE FINDINGS
---------------------------------------------------------------------------------------------------
SF-1  (PL, stands) The eight Link macros and the user signals have no serializer; "one Save" is not whole until they do.
SF-2  (PL, stands) The Composition inspector's five autopilot controls set values nothing reads; `bpmMultiplier` likewise.
SF-3  (PL, stands) Piano / momentary, relative CC, Selected / ThisItem and velocity-to-opacity can be set from no screen.
SF-4  The unverified `replaceWithText` stays in four writers this lane does not own: take.json (Take.cpp:293), audio.json
      (AudioStore.cpp:251), the file browser's favourites (FilesBrowser.cpp:634), MilkDrop user data
      (ProjectMPresetManager.cpp:108). `safewrite::writeTextVerified` is there for them.
SF-5  A take finalised during a quit that fails to save shows nothing (FS B8), against his "If the user is recording the
      show, that is not saved, and that should be shown." The quit window gives a moment before the app goes where a stop
      and a check could run. Not built here (H-10).
SF-6  `/api/debug/save_composition` answers before the write (ApiServer.cpp:2195-2196): a gate reads the result from
      `GET /api/debug/show_file`, never from the route's own answer.
SF-7  The boxes "<file>: could not read/parse file" and "<file>: <reason>" at Open (MC:3493-3496, :3508-3511) are failure
      texts outside his five. This lane does not touch them; a grey row in the list never raises one.
SF-8  The labels "Saved: <file>" (MC:3553, :3581) are event texts: the messages lane's.
SF-9  His four `*.deck.json` are states of the pre-deck app (V20). After S7 no code in the tree reads that shape; git keeps
      `PresetManager::loadDeck` at 185147b.
SF-10 Collect Media re-points the live clips and can write over the show's own file (FS A3, A4). Through `writeShow` that
      write is now verified, and copied aside first when the file is not a version-2 show; the re-pointing is unchanged.
SF-11 A held momentary KEY released while any modal window is open is not seen until the next key event: key events go
      to the modal window only (V11). True of every box today; unchanged.
SF-12 Fact-sheet correction: FO Q5 cites MC:2541-2545 for "a zero-size preview detaches the GL context"; those lines are
      the shutdown comment. The claim is UNKNOWN (V19).
SF-13 A test-mode instance at 185147b lists his REAL compositions folder (CompDecksBrowser.cpp:322-326 has no test switch).
      A-8's override ends that; until S4b merges, no gate of this lane points a route at a file in that folder.
SF-14 The dispatch lists questions 47-50 as open; the record has all four answered (V8).

## 10 RISKS (the strongest counterargument first; the cheapest refuting test for each)
---------------------------------------------------------------------------------------------------
R1  THE STRONGEST, against this ruling's own biggest change: the quit window is now the app's OWN panel with its own key
    handling and test seams, in the one path where a bug either loses work or leaves an app that cannot be quit -- and
    the piece that shows it (a DialogWindow made modal) is the ONE piece no gate runs, because no gate may show the window.
    The plan's stock AlertWindow is JUCE code that every other box of the app already uses. Why the ruling stands: the
    stock window cannot be proven without showing it either; built by hand it has no Return / Esc contract and its focused
    button presses itself on Return (V11); it cannot look like his screenshot (V12; the plan's own R9 named the custom
    window as the fallback). With the panel, every transition is a unit row, the real buttons and the real key handler are
    pressed in ctest and in live rows, the real quit entry is driven with a mutant, and the one stuck state the plan had is
    gone. What is left unproven is about fifteen lines of host code and macOS's behaviour around it -- said plainly in B-6,
    which is the first time the real window opens. Cheapest refuting test: B-6, all four doors, once.
R2  "Opening a show switches the keys" can take his controller away mid-night (PL R1 stands, with his "yes but you import
    the setup from the most recent show"). The absent / empty / broken rules keep his keys for every show that exists
    today. Test: B-3. The line: the apply call in `finishStagedLoad`.
R3  The computer's copy changes only at a Save (A-5): he opens last night's show, plays, quits without saving, and the
    next launch has the keys of the show he saved last. Read against his "most recent show" this may be the wrong show.
    It is the reading that writes nothing without a Save and it is question 94. Test: OS-L1, OS-L5.
R4  A double-click to take a deck is slower than today's one click. Question 91 B.
R5  Return does nothing in the quit window. If Resolume's Return saves and quits (ASSUMED likely, NOT checked -- a
    Researcher can check), his hands may expect it. The hazard does not depend on that fact. Question 93 B.
R6  M1 puts a new writer under his only show before anything else of the lane. That is the point (today's writer is the
    unsafe one), and it is why G-OS-HIS compares the whole loaded show between the two apps. Test: G-OS-HIS, M-1.
R7  The copy rule makes a `backups` folder beside any file that is not a version-2 show and is written over; a FILE named
    `backups` there makes every save of such a show fail, with only "Save failed: <path>" to tell him. Test: OS-L14.
R8  Every Save now reads the target, may copy it, writes, and reads the result back -- on the message thread. Test: M-2.
R9  The list shows files that are not shows as grey rows; today they look like shows and fail when clicked. A folder full
    of other JSON would be a list full of grey rows. On his disk the folder holds one file (V5).
R10 "An empty set is no set": a show can never mean "no keys at all" (PL R2 stands; no screen can clear bindings today).
R11 A later-version show opens best-effort and may be misread (PL R4 stands); its bytes are kept at the first Save.
R12 The 50 ms bar of OS-L16 may not be readable on a busy machine: M-4 decides whether the launch can be read; the bar
    does not move.
R13 After S7 nothing in the tree reads the old preset files (SF-9) and no effects look can be saved until the
    effect-looks lane lands. His word is "86 default yes"; the gap is told to him (B-7).
R14 A logout or shutdown is held by the quit window until he answers (PL R11 stands).
R15 NOT READ BY ME, so INFERRED where this ruling leans on them: `refreshUiAfterModelSwap`; `BrowserPanel`'s tab switch
    beyond its enum; `EffectChain::render` with every effect off; the three probes' fixture writers; how a DialogWindow
    made modal behaves when the main window is minimised or the app is in the background; which button a hand-built
    AlertWindow focuses first (V11) -- moot, since none is built.
R16 The plan's carrier keeps two `juce::var` members on Composition (PL:130-134), an object other threads read. They are
    written and cleared on the message thread only and no other thread reads them; a builder who finds a copy of the live
    Composition made off the message thread STOPS with a report.

WHAT IS NOT IN THIS LANE: PL section 9 stands, plus: a previous copy at every Save (H-4); the four other unverified
writers (SF-4); any message for a take that fails at quit (SF-5); the Open failure boxes and the "Saved:" labels (SF-7,
SF-8); any reader or converter for the pre-deck preset files (SF-9); per-effect looks; a "changed since the last save"
flag; drag of a deck from the list; a way to clear all keys.

STATUS: DONE
