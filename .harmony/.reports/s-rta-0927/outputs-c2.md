## BUILDER REPORT -- lane outputs-c2 (plan5 slice C2: the composition on any number of displays -- one item list, two doors, the keys)

STATUS: DONE_WITH_CONCERNS
RESULT: Plan5 slice C2 is built on `lane/outputs-c2-0927` (base main `f4507e8`, C1 merged).
- `output::OutputManager` owns one output window per display, as many as are connected, the main screen included.
- The Output menu shows one tickable "Display N (WxH[, main])" item per display, then "All Outputs Off".
- The TopBar "Outputs: Off / Outputs: N" button opens the SAME list. Both lists come from `output::buildOutputMenu`.
- Both old combos are deleted: the hidden one in MainComponent and the one in the TopBar.
- Keys: Cmd+Shift+Esc closes every output (checked before the bare-Esc case), Cmd+` raises the app window, and Cmd+F toggles the main display. Plain Esc is still swallowed but no longer touches outputs.
- A deck file no longer carries an output display (R7). `currentImageFile_` is gone.
- `/api/state.outputs.displays` lists the displays.
- The level probe's labels are updated. Its RED, the refusal of the old constants, was recorded without opening a window.
- No window was ever opened.

Commits: C2a `548e2ce`, C2b `8d708f6`, C2c `7ea9a30`, a close-path fix `50f1311`, C2d `12f192e` (docs), and the commit carrying this report.
FACTS: `/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0927-w5/src/output/OutputMenuModel.h` (DisplayInfo, buildOutputMenu, addOutputMenuItems, outputsButtonText, classifyOutputKey); `/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0927-w5/src/output/OutputManager.cpp` (live windows, graveyard, stateVar, shutdown); `/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0927-w5/src/MainComponent.cpp` (keyPressed switch, Output handlers, TopBar button, providers, shutdown); `/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0927-w5/src/ui/MenuBarModel.cpp` (case 6); `/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0927-w5/src/ui/TopBar.cpp`; `/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0927-w5/src/ui/PresetManager.cpp` (R7); `/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0927-w5/tests/test_output_menu_model.cpp`; `/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0927-w5/.harmony/probe-outputs.py` (o_state_displays); evidence `/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0927-w5/.harmony/.reports/s-rta-0927/outputs-c2-evidence/`; shots `/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0927-w5/.harmony/.reports/s-rta-0927/outputs-c2-shots/`
METHOD: I re-anchored C2 at main first (section 1). Every new test and probe row ran RED first, on one of three bases: main's tree (a scratch export, `git archive f4507e8`), the code before the change, or the pre-change app (the main checkout's `build/`, used read-only). Guards were checked on mutated COPIES only; the deliverables' sha256 was the same before and after. ctest ran serially after each commit. probe-outputs ran on the pre-change app and on the lane app, and again on the final binary. All 15 existing probes were re-run, with no threshold edits, each under its own live-lock hold, while a Quartz window-list sampler ran the whole time. The level-probe RED used the probe's own discovery function: it was imported, never run, and fed the new menu titles.
CONFIDENCE+VERIFY: HIGH for the item list, the Output menu, the keys classification, the REST view, R7 and screen safety. Each has a unit test or live REST check, with RED/GREEN pairs. MEDIUM for the window side: ticks and TopBar text after a real toggle, N windows at once, and Cmd+`/Cmd+Shift+Esc reaching the app while an output covers it. The law forbids any gate from opening a window, so these are Boris checks 1-3 and 5-9 (section 11). Verify: `ctest --test-dir build-lane` (729/729); `OUTP_APP=<lane app> bash .harmony/probe-outputs.sh` under the live lock (`PY 13 PASS / 0 FAIL`, `PROBE-OUTPUTS GREEN`; pre-change app `PY 10 PASS / 3 FAIL`); `build-lane/tests/tool_uitoggle_snapshot <dir>` prints this machine's Output menu.
UNKNOWNS/NOT-DONE:
1. No output window was opened (the law). The multi-window behaviour is INFERRED from source (Boris checks). So are Cmd+` against macOS's own "next window" chord, and Cmd+Shift+Esc reaching the app.
2. C3 is not built: hot-plug reconcile, AppSettings, Restore Last Outputs. Until then, a window whose display is unplugged stays open, and macOS may move it onto another screen. All Outputs Off and Cmd+Shift+Esc still close it, because both count windows, not ticked displays.
3. `/api/state.outputs.displays` is rebuilt at startup and on every open/close. It is not rebuilt on a plug event; that is C3's poll.
4. tests/visual Tier-1 was not run. C2 touches no render, effect or source code, and the plan's C2 gate does not list it.
NUANCE: The plan's deck-restore lines (`MainComponent.cpp:3737`, `:3828-3829`) no longer exist; plan6 `01ad154` retired that Deck Save/Load. The only carrier left is `PresetManager::saveDeck/loadDeck` (DeckState, no app caller). R7 is implemented there, and a source-law row guards it. CLAUDE.md is 22,452 B at main, not at the 25,000 cap, because the phase protocol moved out in `20cede2`. Ruling (d) was applied anyway: net -16 B (section 8).
HANDOFF-NEEDS: none

INBOX-RECHECK: none (no addendum message received during the lane)

### 1. DRIFT CHECK (plan written at 5285662; this lane branches from main f4507e8 with C1 merged)

Verdict: NO STRUCTURAL BREAK. Every C2 name exists or is new. The only real drift is D2: the deck-restore lines are gone from MainComponent.

| # | plan5-final.md assumed | main f4507e8 has | effect on C2 |
|---|---|---|---|
| D1 | Output menu `MenuBarModel.cpp:125-154`, "Disabled" `:127` | `case 6` `:134-163`, "Disabled" `:136`, Fullscreen items `:139-152` | replaced by `populateOutputItems(menu)`; enum untouched |
| D2 | deck-file restore in MainComponent `:3737` (saveDeck writes) / `:3828-3829` (loadDeck opens) | GONE: plan6 `01ad154` retired that Deck Save/Load. The v2 deck files (`writeDeckFile` / `appendDeckFromFile`, `Deck::toVar`) carry no outputDisplay. Only `PresetManager::saveDeck/loadDeck` (DeckState.outputDisplay) still write/read it, and nothing in the app calls them (only test_preset_manager) | R7 implemented in `PresetManager.cpp` (write 1, ignore on read) + ctest; a source-law row keeps `outputDisplay` out of MainComponent/OutputManager |
| D3 | `currentImageFile_` STAYS (8.8) | write-only since C1 (C1 D11) | Harmony ruling (a): member + 20 writes + one comment mention deleted |
| D4 | Esc branch `MainComponent.cpp:3232`, Cmd+F `:3284-3296` | Esc `:3421-3430`, Cmd+F `:3472-3486` | both replaced by ONE `switch (output::classifyOutputKey(key))` at the Esc position; Cmd+F moved into it (key codes disjoint from the branches in between) |
| D5 | "confirm the backtick key code with one debug print on the rig" (7.3) | -- | Harmony ruling (b): unit test only (`createFromDescription("command + `")` == 0x60, plus the peer's own derivation `toUpperCase('`') == 0x60`); no key pressed |
| D6 | TopBar `.h:138-139`, `.cpp:289-293`, layout `:625-626`, accessor `.h:67` | `.h:139-140`, `.cpp:290-294`, `:626-627`, `.h:68` | same content; button in the combo's 100 px, the label's 42 px returned to rightSection |
| D7 | hidden combo `MainComponent.h:347-348`, `.cpp:331`, `:500-509`, `:2521-2522`, `refreshDisplayList :3574-3592`, `.h:155` | `.h:358-359`, `.cpp:335`, `:435-444`, `:2495-2496`, `:3775-3793`, `.h:171`; TopBar wiring `:561-589` | all deleted |
| D8 | handlers `:5928-5933` / `:6579-6580`; menu wiring `:1844` | `:6016-6021` / `:6663-6665`; `:1790` | toggleDisplay / closeAll; populateOutputItems + onLiveCountChanged wired next to isSyphonOutputEnabled |
| D9 | `buildOutputMenu(displays, live, base, allOff)`, "All Outputs Off enabled iff any live" (5) | -- | added a `liveWindows` argument: with no reconcile before C3, a window whose display vanished ticks no item, yet the panic item must stay enabled |
| D10 | `OutputMenuItem {label,id,ticked,enabled}` | -- | + `shortcut` (the display-only "Cmd+Shift+Esc" lives in the model, so both doors carry it) |
| D11 | OutputMenuModel.h "pure: juce_core"; DisplayInfo in OutputTargets.h (C3) | -- | DisplayInfo defined in OutputMenuModel.h (C2 needs it; C3 should include it, not redefine it). The header includes juce_gui_basics for PopupMenu/KeyPress (addOutputMenuItems, classifyOutputKey), with still no window, GL or Desktop access |
| D12 | `OutputWindow::target()` (8.3) | not in C1 | not added: `OutputManager::Live` holds the target (plan 8.4's struct), the only user |
| D13 | label consumers HANDOFF `:642`, `:663`; VALIDATION `:82-83`; gotchas `:228`, `:252` | HANDOFF `:376-378`, `:642`, `:683`, `:704`, `:717`; VALIDATION `:33`, `:82`, `:83`; gotchas `:228`, `:252`; the level probe `:44`, `:142-144`, `:698-728`, `:748-760`, `:816-846` | all updated in C2b (the label-change commit) |
| D14 | CLAUDE.md "at its 25,000-byte cap" (ruling d) | 22,452 B (phase protocol moved to docs/claude/phase-protocol.md, `20cede2`) | ruling applied anyway: net -16 B |
| D15 | shutdown law "servers stop -> outputs close -> main renderer detach" (14) | C1 D12: servers stop -> main detach -> output detach | kept exactly: `outputs_.shutdown()` where the output detach was |
| D16 | "o_state_outputs (C2) displays non-empty with one main" (10.2) | -- | a separate row `o_state_displays`, so o_state_outputs' C1 RED/GREEN lines stay comparable. Adds a CoreGraphics oracle and the level-probe picker check |
| D17 | test_output_law "from C2, src/output/OutputManager.cpp" (10.1) | -- | two rows: OutputManager never raises/focuses/floats/shows a window itself; no `outputDisplay` in the output code |

### 2. COMMITS (on `lane/outputs-c2-0927`, base `f4507e8`)
| commit | what | gate at the commit |
|---|---|---|
| `548e2ce` C2a | `src/output/OutputMenuModel.h`, `src/output/OutputManager.h/.cpp` (not wired yet), `tests/test_output_menu_model.cpp`, CMake | RED by absence (against main's src: `fatal error: 'output/OutputMenuModel.h' file not found`); GREEN `All tests passed (125 assertions in 5 test cases)`; ctest 720 -> `100% tests passed, 0 tests failed out of 725` |
| `8d708f6` C2b | Output menu (MenuBarModel case 6 + `populateOutputItems`), TopBar "Outputs" button, both combos + refreshDisplayList/openOutputOnDisplay/closeOutput/outputWindow_ deleted, keys + Cmd+F, OutputManager wiring + shutdown, R7 in PresetManager, `currentImageFile_` deleted; tests: menu-bar case, 2 law rows, preset R7 case, tool_uitoggle_snapshot renders; label consumers (level probe, HANDOFF, VALIDATION, gotchas) | RED first (section 3); GREEN; ctest `out of 729` |
| `7ea9a30` C2c | `setOutputsStateProvider` on 7070 + 8080, `/api/state.outputs.displays`; probe-outputs `o_state_displays` + `Connection: close`; shots | probe-outputs RED on the pre-change app / GREEN on the lane; ctest 729 |
| `50f1311` fix | `OutputManager::closeLive` no longer resets `onCloseRequested` (it can run INSIDE that std::function; resetting it destroys the executing lambda). Found by reading the diff | ctest 729; probe-outputs GREEN on the final binary |
| `12f192e` C2d | CLAUDE.md, docs/claude/{integration,pitfalls,architecture}.md, APP-INVENTORY, notebook | docs only |

### 3. RED FIRST (raw lines, verbatim; files in outputs-c2-evidence/)
- test_output_menu_model, RED by absence (`RED-C2a-test_output_menu_model-absence.txt`, the lane test compiled with the target's flags against main's src): `tests/test_output_menu_model.cpp:8:10: fatal error: 'output/OutputMenuModel.h' file not found`.
- The C2b tests were run before the code change (`RED-C2b-tests.txt`).
  - test_preset_manager R7: `CHECK( static_cast<int>(obj->getProperty("outputDisplay")) == 1 ) with expansion: 3 == 1` and `CHECK( loaded.outputDisplay == 1 ) with expansion: 2 == 1`, `test cases: 1 | 1 failed`.
  - The menu-bar case: `error: no member named 'populateOutputItems' in 'AudioDNAMenuBar'`.
  - test_output_law on main's tree (a binary compiled with `AUDIODNA_SRC_DIR=<scratch export>/src`): `output law: OutputManager never raises, focuses, floats or shows a window ... FAILED: REQUIRE( in.good() )`, same for the outputDisplay row, `test cases: 13 | 11 passed | 2 failed`.
  - The outputDisplay row's MainComponent half is a GUARD (green on main: plan6 already removed the reads). Its teeth are below.
- Guard teeth, on COPIES (`TEETH-C2b-test_output_law-guards.txt`):
  - The control copy: `All tests passed (68 assertions in 13 test cases)`.
  - Each of these mutants gave `test cases: 13 | 12 passed | 1 failed`: OutputManager `window->toFront (true);`; OutputManager shows the window itself (`setBounds` + `setVisible (true)` instead of `openOnDisplay`); OutputManager `setAlwaysOnTop(true)`; MainComponent `composition_.outputDisplay` read; MainComponent `getProperty("outputDisplay", 1)` (a string literal, which proves literals are scanned).
  - The key named only in a comment stayed green.
  - The deliverables' sha256 was identical before and after (`8747dd84...` OutputManager.cpp, `25e61362...` MainComponent.cpp).
- Level-probe constants. The probe was never run: its module was imported, and `main`, `_spawn_app`, `_click_output_item`, `_open_output_window`, `_osascript`, `_press_escape`, `_measure`, `_close_output_window`, `_teardown` and `_terminate_app` were replaced by a raise before anything was called.
  - Headless run (`RED-GREEN-C2b-level-probe-constants-headless.txt`, titles = MenuBarModel case 6 built by the C2b code from this machine's displays):
    - OLD main f4507e8: `open item -> REFUSES (ProbeBlocked -> the probe exits BLOCKED before opening anything): No 'Fullscreen: <WxH> (main)' item in the Output menu. Items seen: ['Display 1 (1728x1117, main)', 'All Outputs Off', 'Snapshot', 'Start Recording', 'Stop Recording', 'Syphon Output']`, `close item -> ABSENT (teardown rung 1 would fail): 'Disabled'`.
    - NEW lane: `open item -> PICKS 'Display 1 (1728x1117, main)'`, `close item -> PRESENT: 'All Outputs Off'`.
  - The same pair against the LIVE lane app's `/api/state.outputs.displays[].label` (`RED-GREEN-C2c-level-probe-constants-live.txt`): OLD REFUSES, NEW PICKS.
- probe-outputs on the PRE-CHANGE app (`RED-C2c-probe-outputs-preC2-app.txt`, the main checkout's `build/…/Audio-DNA.app`, lane probe):
  - `FAIL  o_state_displays: http://[::1]:8080 outputs.displays = [] (CoreGraphics: 1 display(s), main (1728, 1117)) -- displays missing or empty (None)`; the same line for `http://127.0.0.1:7070`; `FAIL  o_state_displays: no main display label to hand to the level probe's picker`.
  - `PY 10 PASS / 3 FAIL`, `PROBE-OUTPUTS RED`.
  - The 10 C1 rows are GREEN on it, because that app already carries C1.

### 4. GREEN (raw lines, verbatim)
- Unit tests (`GREEN-C2b-tests.txt`): test_preset_manager R7 `All tests passed (9 assertions in 1 test case)`; test_output_menu_model `All tests passed (160 assertions in 6 test cases)`; test_output_law `All tests passed (68 assertions in 13 test cases)`.
- probe-outputs on the lane (`GREEN-C2c-probe-outputs-lane.txt`; again on the final binary, `GREEN-final-probe-outputs-lane.txt`):
  - `PASS  o_state_displays: http://127.0.0.1:7070 outputs.displays = [{'index': 0, 'x': 0, 'y': 0, 'w': 1728, 'h': 1117, 'scale': 2.0, 'main': True, 'live': False, 'label': 'Display 1 (1728x1117, main)'}] (CoreGraphics: 1 display(s), main (1728, 1117))` (and the same for 8080)
  - `PASS  o_state_displays: the level probe's _pick_main_fullscreen_item (FULLSCREEN_PREFIX 'Display ', MAIN_SUFFIX ', main)') picks 'Display 1 (1728x1117, main)' from the app's labels; close item DISABLED_ITEM 'All Outputs Off'`
  - `PASS  o_no_window_opened: 72 Quartz samples over the run: Output-named Audio-DNA windows [], max on-screen Audio-DNA layer-0 windows 1`
  - `PY 13 PASS / 0 FAIL`, `PASS  after quit: 0 Audio-DNA windows in the FULL Quartz window list (no output window survives)`, `PROBE-OUTPUTS GREEN`
- ctest serial (`ctest-*-tail.txt`): base `100% tests passed, 0 tests failed out of 720`; C2a 725; C2b 729; C2c 729; fix / HEAD `100% tests passed, 0 tests failed out of 729`.

### 5. EXISTING BATTERY (lane build at 7ea9a30; each probe under its own lock hold; no threshold edits)
`battery-summary.txt`: render-state `PY 31 PASS / 0 FAIL`; crossfade `PY 35 PASS / 0 FAIL`; effects-parity `PY 46 PASS / 0 FAIL`; canvas `PY 15 PASS / 0 FAIL`; deck-clock `PY 10 PASS / 0 FAIL`; fitmode `PY 10 PASS / 0 FAIL`; deck-tabs `6 PASS / 0 FAIL`; step3 `94 PASS / 0 FAIL`; mastersignal `22 PASS / 0 FAIL`; routines `98 PASS / 0 FAIL` (ROUTINES_RECORD_PAUSE=1.8); resync `16 PASS / 0 FAIL`; onset-render `13 PASS / 0 FAIL`; downbeat-level `14 PASS / 0 FAIL`; manual-bpm `22 PASS / 0 FAIL`; routine-display `16 PASS / 0 FAIL` (Phase 1, no `--hook`).
- All 15 are GREEN.
- NOT run: deck-path, tempo-silence, lane3 and finalize-loop. Their launch forms (direct exec / plain `open`) break the rig rules; C1 skipped the same four.
- The battery ran on the binary before `50f1311`. That fix changes only one line, in the close path of an OPEN output window, which no probe can reach. probe-outputs and ctest were re-run on the final binary.

### 6. SCREEN SAFETY (output_window_opened = false, proven)
- By construction, only three things can open a window: `OutputManager::openDisplay`, reached only through `toggleDisplay`; that in turn is called only from `handleMenuCommand` (menu-bar / TopBar-popup picks) and from `keyPressed` (Cmd+F). No REST, OSC, MIDI or binding path calls them (`grep -rn "outputs_\." src`). No gate made a menu pick or a key press: no synthetic input, no AppleScript UI, no AX, and no deck/composition file with an output display.
- Quartz evidence:
  - probe-outputs RED, GREEN and final runs: 72 samples each, no Output-named window, max one layer-0 window.
  - The battery sampler over 14:05:09..14:58:16: `12121 samples, Output-named Audio-DNA windows [], max on-screen Audio-DNA layer-0 windows 1` (`battery-window-sampler.txt`).
  - After every probe quit, the FULL window list held 0 Audio-DNA windows.
- The only captures are window-only (the main window, by Quartz id, taken inside probe-outputs.sh). There was no full-screen capture, no lldb/dtrace/Instruments, and no temporary hook: `strings build-lane/.../Audio-DNA | grep -c AUDIODNA_DEBUG_` = 0.
- The level probe (`tests/visual/test_output_window_level.py`) was never run. Only its pure picker was imported, with every launching/driving function replaced by a raise. No pytest ran on tests/visual.

### 7. UI EVIDENCE (shots, absolute paths)
- `/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0927-w5/.harmony/.reports/s-rta-0927/outputs-c2-shots/01-before-main-topbar-right-headless.png` (+ `01-before-main-topbar-headless-full.png`): tool_uitoggle_snapshot built at main. The combo is blank headless; MainComponent fills in its "Off" text in the app, see shot 10.
- `.../02-after-topbar-right-outputs-0-headless.png`, `03-...-outputs-1-...`, `04-...-outputs-2-...` (+ `05-after-topbar-full-outputs-0-headless.png`): "Outputs: Off" / "Outputs: 1" / "Outputs: 2", set through `TopBar::setLiveOutputCount` (the call `OutputManager::onLiveCountChanged` makes). No window, no app hook.
- `.../06-outputs-button-menu-fake3-headless.png`: the TopBar door's popup for a FAKE 3-display set: "Display 1 (1728x1117, main)", "Display 2 (1920x1080)" ticked, "Display 3 (3840x2160)", separator, "All Outputs Off" with "Cmd+Shift+Esc". There is no headless PopupMenu window, so the items are drawn row by row with the app LookAndFeel's own `drawPopupMenuBackground` / `drawPopupMenuItem`, the painters JUCE's PopupMenu window uses.
- `.../07-output-menubar-fake3-headless.png`: the whole Output menu, `AudioDNAMenuBar::getMenuForIndex(6)` built from the same list, drawn the same way. In the app the menu-bar door is the native NSMenu: same titles, ids and ticks, drawn by macOS, with no shortcut text (JUCE sets NSMenu key equivalents only through an ApplicationCommandManager, exactly like Undo's "Cmd+Z"). The item lists are printed in `tool_uitoggle_snapshot-C2b-output.txt`.
- `.../08-before-main-window-preC2-app-quartz.png` / `.../09-after-main-window-lane-app-quartz.png`: window-only Quartz shots of the main window (pre-change app / lane app). Crops of the TopBar's right section: `.../10-before-main-window-topbar-right-crop.png` ("Output: [Off v]") and `.../11-after-main-window-topbar-right-crop.png` ("Outputs: Off"). I looked at all of them. The FPS readout is intact, and Master Signal / Master shifted right by the freed 42 px.

### 8. DOCS (C2d) and the CLAUDE.md ruling
- CLAUDE.md:
  - Key capabilities now reads "output to any number of connected displays incl. the main screen (macOS)" (C1 deferred that wording to C2).
  - A new UI Patterns "**Outputs**" line points to docs/claude/integration.md.
  - Paid for per ruling (d): the ResettableSlider text-box/0.5-default sentence moved VERBATIM to `docs/claude/pitfalls.md` Pitfall 5, and the paragraph now points there. Transport state's loop-mode sentence became "Loop modes: Pitfall 2."; Pitfall 2 already states it in full (read back, player state after `advanceFrame()`, PingPong, OneShot).
  - 22,452 B -> 22,436 B. No fact deleted.
- `docs/claude/integration.md`: the Output windows paragraph (two doors, one list; keys; machine state, not file state; the C3 gap; the displays JSON).
- `docs/claude/architecture.md`: tree lines for the two new files.
- `.harmony/APP-INVENTORY.md`: the menu bar, OutputWindow and TopBar rows, and the keyboard shortcuts.
- `.harmony/notebook.md`: one entry.
- Label consumers landed with the label change in C2b. The report check (`grep -rn '"Disabled"' .harmony tests docs src --include=*.py --include=*.sh --include=*.md --include=*.cpp`, excluding `.harmony/*.log`, `.harmony/black-overlay-rootcause.md` and `docs/archive/`) gives **0 hits** at C2b and at HEAD.
  - The SCREEN-SAFETY LAW EOS snippet in `.harmony/HANDOFF.md` now reads `click menu item "All Outputs Off" of menu 1 of menu bar item "Output" of menu bar 1`.
  - `tests/visual/test_mapping_tick.py` needed no change: Cmd+F still opens the main-display output when it is off.

### 9. HARMONY RULINGS
- (a) `MainComponent::currentImageFile_`: the member, its 20 writes and one comment mention are deleted (C2b).
- (b) Backtick: the key code is confirmed with no key press, by a unit test: `KeyPress::createFromDescription("command + " + "`").getKeyCode() == 0x60`, and the mac peer's own derivation `CharacterFunctions::toUpperCase('`') == 0x60` (its kVK_ANSI_Grave fallback is '`' too).
- (c) R7: the outputDisplay key is kept, the value written is 1, and a 2+ file loads as 1. Covered by a ctest plus a source-law guard (drift D2).
- (d) CLAUDE.md is paid for (section 8).
- (e) My MenuBarModel hunks are case 6 + one member only; MainComponent hunks are output code, deletions and the key switch. The Deck menu (case 2) and the deck tabs are untouched.

### 10. KNOWN GAP (C2 without C3)
Until C3's reconcile, a window whose display is unplugged stays open. AppKit may move it onto a remaining display, where it covers that screen and shows the composition. All Outputs Off and Cmd+Shift+Esc close it: `closeAll` walks the live windows, and the menu enables "All Outputs Off" from the WINDOW count, not from ticked displays (D9). The `/api/state` display list also refreshes only on open/close until C3.

### 11. BORIS CHECKLIST (plan section 12 items 1-3, 5-9, updated for C2)
1. Plug in the projector. Output menu -> tick `Display 2 (…)` (or the TopBar `Outputs: Off` button, which opens the same list). The projector should show exactly what the lower-left panel shows: same black bars, sharp, the right size. The TopBar should read `Outputs: 1` and the item should be ticked. Untick it (or `All Outputs Off`): it should disappear, nothing should stay on any screen, and the TopBar should go back to `Outputs: Off`.
2. Tick `Display 1 (…, main)`, the laptop screen. The picture covers the app. Press Cmd+` (Cmd + the key under Esc): the app should come back on top while the picture keeps running behind it. With both displays ticked, the TopBar should read `Outputs: 2` and both screens should show the composition. Press Cmd+Shift+Esc: every output should close. Plain Esc should now do NOTHING to the outputs. This is the muscle-memory change (plan Q2).
3. With the picture covering the laptop screen, click on it, then use any shortcut you normally use (a clip key, the Output menu). They should all still work, because the picture never takes the keyboard. Cmd+F now toggles only the laptop-screen output. With the projector output on, Cmd+F no longer closes it; it opens a second output on the laptop screen. Also try the TopBar button while the main-screen output is off.
5. With the projector on, open the big signal bar (the preview hides). The projector should keep the last picture, frozen; close the bar and it should move again. Same for minimising the app.
6. Quit with two outputs open (Cmd+Q). The quit should be clean and every screen clean.
7. The top edge: is there a translucent menu-bar strip across the top of the projector picture ("Displays have separate Spaces" ON)? If yes: System Settings > Control Center > "Automatically hide and show the menu bar: Always".
8. 120 Hz laptop + 60 Hz projector with fast motion: it should be smooth, with no stutter and no horizontal seam, with one and with two outputs.
9. The TopBar fps readout with 1, 2 and 3 outputs on: that is the per-output cost in his numbers.
(C2 note for check 4, which belongs to C3: until C3, pulling a projector cable while its output is on may move that picture onto another screen. Cmd+Shift+Esc clears it.)

### FILES CHANGED
- NEW `src/output/OutputMenuModel.h`, `src/output/OutputManager.h/.cpp`; NEW `tests/test_output_menu_model.cpp`.
- `src/MainComponent.h/.cpp`: combos, display functions, outputWindow_ and currentImageFile_ deleted; outputs_ member + wiring; the key switch; shutdown; providers.
- `src/ui/MenuBarModel.h/.cpp`: case 6 + `populateOutputItems`.
- `src/ui/TopBar.h/.cpp`: the Outputs button + `setLiveOutputCount`.
- `src/ui/PresetManager.h/.cpp`: R7.
- `src/api/ApiServer.h/.cpp`, `src/test/TestServer.h/.cpp`: `setOutputsStateProvider`, `outputs.displays`.
- `CMakeLists.txt`: 3 sources.
- `tests/CMakeLists.txt`: the new ctest; MenuBarModel.cpp added to tool_uitoggle_snapshot.
- `tests/test_output_law.cpp` (+2 rows), `tests/test_preset_manager.cpp` (+R7 case), `tests/tool_uitoggle_snapshot.cpp` (Outputs states + menu renders), `tests/visual/test_output_window_level.py` (labels, rung 2, AXFocusedWindow INFO line; never run).
- `.harmony/probe-outputs.py`: o_state_displays, Connection: close.
- `.harmony/HANDOFF.md`, `.harmony/VALIDATION.md`, `.harmony/gotchas.md`.
- docs listed in section 8.

### TESTS
- New/extended ctests: test_output_menu_model (6 cases), test_output_law (+2 rows, 13 total), test_preset_manager (+1 case). Serial total 720 -> 729, all passing.
- Live: probe-outputs 13/0 GREEN (pre-change app 10/3 RED); 15 existing probes GREEN.

### SLIM CHECK
- Cut:
  - `OutputWindow::target()`: the Live struct holds the target.
  - A REST route for the menu: `displays[].label` carries the titles.
  - A temporary env-var hook for the live "Outputs: 1/2" state: the headless renders call the same `TopBar::setLiveOutputCount`, so no hook, no revert and no rebuild were needed.
- Kept on purpose:
  - `classifyOutputKey` as a free function: the only way to unit-test the key semantics without pressing a key.
  - `addOutputMenuItems`: one PopupMenu builder for both doors and the tests.
  - The `liveWindows` argument (D9).

### ISSUES
1. D9/D10/D11 are small signature deviations from plan section 5, for the Reviewer to rule on.
2. `tests/tool_uitoggle_snapshot.cpp` was extended. It is outside the plan's literal fence list, but the plan names it as the TopBar-render tool (section 6).
3. `src/ui/PresetManager.h/.cpp` is outside the plan's literal fence. It is the drift-adjusted home of the plan's deck lines (D2) and of Harmony ruling (c).
4. The app's LookAndFeel draws a ticked popup item with its text shifted right (shot 06). That is existing LAF behaviour, not changed here.

### SKILL_PROPOSALS
- none new; C1's `offscreen-window-law-gate` proposal covers this lane's source-law rows. This lane adds the "import-and-neuter a window-opening probe to RED its constants" step, recorded in the notebook.

### RISKS
- R-a (medium): the window-side behaviour (ticks after a real toggle, N windows, the keys while covered, Cmd+` vs macOS's own chord) is INFERRED. See Boris checks 1-3 and 5-9.
- R-b (medium, behaviour change Boris must hear): with a projector output on, Cmd+F no longer closes it; it now toggles only the main-screen output (plan 7.2). Plain Esc no longer closes outputs (Q2).
- R-c (medium until C3): the hot-plug gap (section 10).
- R-d (merge): MainComponent.cpp and MenuBarModel are edited by other lanes. My hunks are disjoint from case 2 and the deck tabs. The 20 one-line `currentImageFile_` deletions are spread across MainComponent.cpp and could touch lines another lane edits.

### METRICS
- Build-lane full builds exit 0 at every commit; ctest serial 720 -> 725 -> 729 -> 729 -> 729; CLAUDE.md 22,436 B; strings check 0.
- Live lock: 18 holds as `outputs-c2`, each released; >= 45 s between my holds; waits for probe-hardening / routines-timing / harmony.

### KNOWLEDGE CONTEXT
- Tools: grep + direct reads (the vendored JUCE tree for KeyPress/mac menu/peer key codes). Impact authority: grep, which is NOT authoritative. So every removed symbol was enumerated in src/ and tests/ before deletion (`outputWindow_`, `displaySelector_`, `outputLabel_`, `closeOutput`, `openOutputOnDisplay`, `refreshDisplayList`, `getDisplaySelector`, `currentImageFile_`: 0 hits after).
- Risk level: ELEVATED (cross-cutting: MainComponent + TopBar + menu + servers), mitigated by the full existing battery.

### PACKET QUALITY
- Clarity: CLEAR.
- Missing context: the deck-restore lines no longer exist (D2); CLAUDE.md is not at the cap (D14).
- Unused context: plan sections 5 (OutputTargets), 9 (C3).
- Self-assembly: LEGACY (no DEPARTMENT field).
- Self-brief files: plan5-final.md (the spec), outputs-c1.md (drift + what C1 built; very useful), notebook (TEST_SERVER flag, Connection: close, pitfall renumbering, the headless ScopedJuceInitialiser_GUI rule), CLAUDE.md.

### STATUS
DONE_WITH_CONCERNS. Every C2 gate item that can run without a window is done and green:
- RED first everywhere, with each RED/GREEN pair recorded verbatim.
- ctest 729/729.
- probe-outputs GREEN, 15 existing probes GREEN.
- TopBar and menu renders looked at.
- The label check is 0.
- No window was ever opened.

Concerns:
1. The window-side behaviour and the Cmd+F / Esc semantic changes need Boris checks 1-3 and 5-9.
2. The C3 hot-plug gap (section 10).
3. D9-D11 are signature deviations, and two files sit outside the literal fence (ISSUES 2-3).

### NEXT ACTION
For Harmony: run the behavioural gate on build-lane (probe-outputs plus the battery), get an independent review, then merge. Boris then runs checklist items 1-3 and 5-9 at the rig. C3 builds on `OutputManager` (`live_`, `closeAll`, `stateVar`) and should `#include "output/OutputMenuModel.h"` for `DisplayInfo`.
