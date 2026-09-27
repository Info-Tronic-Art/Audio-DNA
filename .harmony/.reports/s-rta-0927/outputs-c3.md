## BUILDER REPORT -- lane outputs-c3 (plan5 slice C3: hot-plug + persistence + restore)

STATUS: DONE_WITH_CONCERNS
RESULT: Plan5 slice C3 is built on `lane/outputs-c3-0927` (base main `dc7adf9`, C1+C2 merged), in 6 commits plus this report.
- Hot-plug: `OutputManager::reconcile()` is one idempotent message-thread function over the pure `output::diffOutputs`.
  - An unplugged display's output closes, and its target is kept as interrupted. When the display comes back, the output reopens by itself (Q6).
  - A display that moves, changes MODE (resolution/scale) or swaps the main flag keeps its output: the window follows it.
- Two triggers:
  - `OutputWindow::parentSizeChanged()` schedules the reconcile on the next message-loop turn. It is coalesced and never runs inside the window's own callback.
  - `pollDisplays()` runs on EVERY 30 Hz UI timer tick.
- Harmony ruling (a): the menu ticks, the TopBar count, `/api/state.outputs(.displays)` and the live windows are rebuilt from one state on every reconcile.
- Persistence: the wanted set (live + interrupted) goes to settings.json `"outputs"` `{version 1, targets}`, only when it changes. It is written through a new read-modify-write `src/model/AppSettings`. The MilkDrop folder reader/writer now go through it too, so neither key can erase the other.
- Restore: the saved set is only LOADED at launch. "Restore Last Outputs" is the only way to open it. It is `kOutputRestoreLast` = 1697, appended after the last Output id. It sits in both doors right below All Outputs Off, and is enabled only when it would open something. Nothing opens at launch or on a file load (Q1); a new source-law row guards this.
- Ruling (b): Cmd+F and Cmd+` need Shift up.
- Ruling (c): `DisplayInfo` has one definition, in `src/output/OutputTargets.h`.
- No output window was opened by anything in this lane.
FACTS: `/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0927-w9/src/output/OutputTargets.cpp` (matchDisplay, diffOutputs, wantedToVar/FromVar); `/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0927-w9/src/output/OutputManager.cpp` (reconcile, pollDisplays, scheduleReconcile, persistWanted, restoreLast, statsVar); `/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0927-w9/src/model/AppSettings.cpp`; `/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0927-w9/src/MainComponent.cpp` (appSettingsFile, MilkDrop on AppSettings, attachSettings, timer poll, the kOutputRestoreLast case, the test hooks); `/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0927-w9/src/output/OutputMenuModel.h` (the restore item, the Shift rule); `/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0927-w9/src/test/TestServer.cpp` (8080 outputs.manager, output_restore_last, set_output_poll); `/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0927-w9/tests/test_output_plan.cpp`, `tests/test_app_settings.cpp`, `tests/test_output_law.cpp` (the Q1 row), `tests/test_output_menu_model.cpp`; `/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0927-w9/.harmony/probe-outputs.py` (o_restore_empty, o_poll_idle); evidence `/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0927-w9/.harmony/.reports/s-rta-0927/outputs-c3-evidence/`; shots `/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0927-w9/.harmony/.reports/s-rta-0927/outputs-c3-shots/`
METHOD: I re-anchored C3 at main first (section 1).
- RED first, on one of three bases, for every new test and probe row:
  - main's tree (`git archive dc7adf9`), used for compiles and law runs;
  - the unchanged `classifyOutputKey`;
  - the pre-change app (the main checkout's `build/`, used read-only).
- Teeth ran on mutated COPIES in the scratchpad; the deliverables' sha256 matched before and after.
- One commit per plan item. The intermediate file versions were staged as blobs, and each commit's index was exported and syntax-checked (a negative control shows the check catches errors). Its menu-model and law tests were built and run against it.
- Full ctest ran serially.
- Live runs: every one took the live lock, launched with `open -g` only, and pointed the app at a scratch settings file.
  - probe-outputs: RED, GREEN and seeded runs in one lock hold.
  - The 15 existing probes: in 3 lock-hold batches, under a Quartz window sampler.
CONFIDENCE+VERIFY:
- HIGH for:
  - the matching / diff rules (unit tests with teeth);
  - settings read-modify-write (unit tests + a live byte-identical seeded file);
  - Q1 (a source law with 4 teeth + live `saved 1, restorable 0, opened nothing`);
  - the Shift rule;
  - the poll's zero cost when idle (live A/B);
  - screen safety.
- MEDIUM for the live hot-plug behaviour itself: a real unplug, replug or mode change, the window following a re-moded display, and the hook firing before the poll. Every automated path to it would need a live output window (forbidden), so it rests on the unit-level rehearsal and is Boris check 4.
- Verify:
  - `ctest --test-dir build-lane` gives 741/741.
  - `OUTP_APP=<lane app> bash .harmony/probe-outputs.sh` under the lock gives `PY 17 PASS / 0 FAIL`; the pre-change app gives `PY 13 PASS / 3 FAIL`.
  - `build-lane/tests/tool_uitoggle_snapshot <dir>` renders the menus.
UNKNOWNS/NOT-DONE:
1. The real cable pull, replug and mode change (Boris check 4), and whether `parentSizeChanged` reaches a window whose display vanished before AppKit relocates it (R6/R14; the 30 Hz poll is the backstop).
2. tests/visual Tier-1 was not run: C3 touches no render, effect or source code. C2 skipped it for the same reason.
3. The deck-path, tempo-silence, lane3 and finalize-loop probes were not run: their launch forms break the rig rules, the same four C1 and C2 skipped.
4. A live `outputs` settings WRITE was never provoked: no window-free path can change the wanted set. It is covered by test_app_settings with the app's own keys and serializer.
NUANCE:
- Two plan assumptions were wrong for this rig. The settings file is `~/Library/Audio-DNA/settings.json` (JUCE maps userApplicationDataDirectory to ~/Library), not Application Support. Neither path exists on this machine, before or after.
- The plan never said what the launch-loaded set is. I made it SAVED, not interrupted (drift D7); otherwise Q6 would reopen it at the first poll, i.e. at launch.
- Beyond the plan's matching rules, a live output follows a MODE change, and loose matches consider only new or changed displays (D6). Without that, a projector's output could jump onto an identical untouched one.
- One rig-rule breach: a `cd /tmp;` inside a command (ISSUES 1).
HANDOFF-NEEDS: none

INBOX-RECHECK: none (no addendum message received during the lane)

### 1. DRIFT CHECK (plan written at 5285662; this lane branches from main dc7adf9, C1 and C2 merged)

Verdict: NO STRUCTURAL BREAK. Every C3 anchor exists. Two plan assumptions were wrong about this rig (D1, D7), and I added two matching rules on top of the plan's (D6).

| # | plan5-final.md / dispatch assumed | main dc7adf9 has | effect on C3 |
|---|---|---|---|
| D1 | settings.json at `~/Library/Application Support/Audio-DNA/settings.json` (plan 9; the dispatch's SETTINGS SAFETY) | the code resolves `userApplicationDataDirectory`, which JUCE maps to `~/Library` on macOS (`juce_Files_mac.mm:209`). The file is therefore `~/Library/Audio-DNA/settings.json`. On this rig NEITHER path exists: no MilkDrop folder was ever saved | the path is unchanged (moving it would orphan a real user's file). I checked both paths before and after every live step (section 6), and the docs name the real path |
| D2 | MilkDrop reader/writer `MainComponent.cpp:2260-2282` | `:2203-2225`. The writer builds a fresh one-key object and calls `replaceWithText`, which erases every other key (VERIFIED at dc7adf9 `:2216-2225`) | both functions now go through `AppSettings`, as one-line bodies (C3a) |
| D3 | timer `startTimerHz(30)` `:426`, tick `:3515` | `:423`; `timerCallback` `:3663` | `outputs_.pollDisplays()` is the first statement of every tick (now `:3689`) |
| D4 | `DisplayInfo` in `src/output/OutputTargets.h` (plan 5) | C2 defined it in `OutputMenuModel.h` (C2 drift D11) | moved to `OutputTargets.h` (Harmony ruling (c)). `OutputMenuModel.h` includes it, so there is one definition |
| D5 | `OutputTarget`, `WantedOutputs`, `LiveEntry`, `toVar/fromVar`, and `OutputDiff { toClose, toRebound, toOpen }` as bare indices | -- | simplified: the sets are `std::vector<DisplayInfo>`, with `wantedToVar` / `wantedFromVar`. `toRebound` and `toOpen` carry `{from, display}` pairs, because the manager needs the matched display, not just the entry |
| D6 | `matchDisplay`: exact -> same (w,h,scale) moved -> main; `diffOutputs(live, interrupted, current)` | -- | Two additions, both tested and teeth-proven (section 3). (i) A LIVE output also follows its display through a MODE change (same top-left, other w/h/scale; `MatchRule::Track`). This is Harmony ruling (a); the plan's rungs would have closed the output. (ii) `diffOutputs` takes the PREVIOUS display list, and the loose rungs consider only NEW or CHANGED displays. Without that, a mode change on projector P1 "moves" its output onto an identical untouched P2, and an interrupted target reopens on P2 as soon as any other display changes |
| D7 | "wanted set = live + interrupted", "Q6 reopens interrupted", "Q1: never opens at launch" | -- (the plan does not say what the launch-loaded set is) | the saved set is loaded at launch as SAVED targets, NOT as interrupted ones, because interrupted ones would reopen through Q6 at the first poll, i.e. at launch. Only Restore Last Outputs opens saved targets: those whose display is connected and free. Saved targets whose display is absent stay saved and are not armed. The first output change of a session rewrites the file with live + interrupted, so "last" means the set as it stood at the last change |
| D8 | trigger (a) "`callAsync` coalesced by a pending flag" | the manager already privately inherits `juce::AsyncUpdater` (C2's deferred destroy) | `scheduleReconcile()` sets `reconcilePending_` and calls `triggerAsyncUpdate()`, which is JUCE's coalesced async call. Unlike a raw `callAsync` capturing `this`, it is cancelled in the destructor. `handleAsyncUpdate` drains the graveyard, then reconciles if a reconcile is pending |
| D9 | `o_poll_idle`: "5 s of live == 0 and frame_time_ms unchanged within 0.3 ms" | -- | an A/B inside one run: 5 s with the poll paused (test-only `set_output_poll`), then 5 s running. The row also checks the tick rate, the reconcile and settings-write counters, and the displays list. It runs only with no compiler running. A later fix made the row load its own composition and require non-zero frame times (section 4) |
| D10 | C3 fence: no `src/test/TestServer.*`, no `.harmony/probe-outputs.*` | -- | the dispatch's probe rows need a window-free trigger and counters. So there are test-mode-only 8080 additions (C3d): `outputs.manager`, `output_restore_last`, `set_output_poll`. 7070 is untouched |
| D11 | All Outputs Off / Cmd+Shift+Esc = close every live window | -- | they now also cancel pending hot-plug reopens (interrupted targets). A panic must not be undone by a cable coming back |
| D12 | `/api/state.outputs.displays` rebuilt on open/close (C2) | -- | also rebuilt on every reconcile (Harmony ruling (a)) |

### 2. COMMITS (on `lane/outputs-c3-0927`, base `dc7adf9`)
| commit | what | gate at the commit |
|---|---|---|
| `54faa36` C3a | `src/output/OutputTargets.{h,cpp}`; `src/model/AppSettings.{h,cpp}`; the MilkDrop reader/writer on AppSettings; `appSettingsFile()` (test-only `AUDIODNA_SETTINGS_FILE`); DisplayInfo moved; `tests/test_output_plan.cpp`, `tests/test_app_settings.cpp`; CMake | RED by absence (main's src); teeth (section 3); the index exported and syntax-checked (15 TUs OK) |
| `b79eecc` C3b | `classifyOutputKey`: Shift up for ToggleMain / RaiseApp (Harmony ruling (b)) + the Shift rows | RED first on the unchanged classifier; the test built from the C3b index: `All tests passed (157 assertions in 6 test cases)` |
| `d10aece` C3c | OutputManager reconcile / poll / hook / persistence / restore; `OutputWindow::parentSizeChanged`; `kOutputRestoreLast` appended; the restore item in the model; MainComponent (attach, tick, menu case); test_output_menu_model (+Restore case), test_output_law (+Q1 row), tool_uitoggle_snapshot | index syntax-checked (15 TUs OK); menu-model test at C3c `All tests passed (188 assertions in 7 test cases)`; law at C3c `All tests passed (87 assertions in 14 test cases)` |
| `3552204` C3d | TestServer test-mode hooks + MainComponent wiring; probe-outputs rows `o_restore_empty`, `o_poll_idle`; scratch settings on every run | probe-outputs RED on the pre-change app / GREEN on the lane |
| `29c7e9e` C3d fix | `o_poll_idle` loads its own composition and requires frame_time_ms > 0 (a subset run had passed on 0.000 vs 0.000) | re-run RED / GREEN / seeded (section 4) |
| `a98e319` C3e | docs: CLAUDE.md, docs/claude/{integration,testing-eyes,architecture}.md, APP-INVENTORY, notebook | docs only |
| (this commit) | this report + `outputs-c3-evidence/` + `outputs-c3-shots/` | -- |

Why the per-commit checks look like this: all the work was already in the tree when I committed. Each intermediate version of the files that span commits (MainComponent.cpp, OutputMenuModel.h, test_output_menu_model.cpp) was generated by reverting the later hunks, and then staged as a blob. The working tree was never checked out or stashed. `git checkout-index --prefix` exported each commit's index to the scratchpad for a syntax check of every TU C3 touches, and the harness caught an injected error (negative control). Full builds and the full ctest ran on the final tree (section 5).

### 3. RED FIRST, TEETH (raw lines, verbatim; files in `outputs-c3-evidence/`)
- test_output_plan / test_app_settings, RED by absence. Both were compiled with the target's flags against main's src (`RED-C3a-absence-test_output_plan-test_app_settings.txt`): `tests/test_output_plan.cpp:7:10: fatal error: 'output/OutputTargets.h' file not found` and `tests/test_app_settings.cpp:9:10: fatal error: 'model/AppSettings.h' file not found`.
- test_output_menu_model's Restore case, RED by absence (`RED-C3c-absence-test_output_menu_model-restore.txt`): `test_output_menu_model.cpp:189:31: error: no member named 'kOutputRestoreLast' in 'AudioDNAMenuBar::CommandID'`.
- classifyOutputKey Shift rows, RED on the unchanged classifier (`RED-C3b-classifyOutputKey-shift-rows.txt`): `CHECK( output::classifyOutputKey(KeyPress::createFromDescription("command + shift + F")) == OutputKey::None ) with expansion: 3 == 0`, the same for `"command + shift + f"`, and `CHECK( output::classifyOutputKey(k) == OutputKey::None ) with expansion: 2 == 0` (Cmd+Shift+`). `test cases: 1 | 0 passed | 1 failed`, `assertions: 21 | 18 passed | 3 failed`.
- test_output_law Q1 row, RED on main's src (`RED-C3c-test_output_law-Q1-row-on-main.txt`): `test_output_law.cpp:265: FAILED: REQUIRE( calls == 1 )`, `test cases: 14 | 13 passed | 1 failed`.
- Teeth, all on COPIES in the scratchpad. The deliverables' sha256 was identical before and after (OutputTargets.cpp `b7e21442...`, AppSettings.cpp `8ab7af85...`, OutputManager.cpp `1b94759e...`, MainComponent.cpp `feb416dd...`).
  - `TEETH-C3a-test_output_plan.txt`:
    - Swap the exact/moved rungs (the plan's teeth): `test_output_plan.cpp:48: FAILED: CHECK( output::matchDisplay(kProjector2, { kLaptop, kProjector, kProjector2 }, {}) == 2 )`, `test cases: 4 | 3 passed | 1 failed`.
    - Loose rungs also consider unchanged displays: `:116 FAILED: CHECK( hasPair(diff.toRebound, 0, 1) )` (the output jumps to the untouched projector) and `:133 FAILED: CHECK( diff.empty() )` (an interrupted target reopens on it).
    - No MODE-change rung: 5 failures, e.g. `:103 FAILED: CHECK( diff.toClose.empty() )`.
    - Control: `All tests passed (63 assertions in 4 test cases)`.
  - `TEETH-C3a-test_app_settings-clobbering-writer.txt`: `update()` rewritten as the pre-C3 writer (a fresh one-key object) gives `test cases: 6 | 3 passed | 3 failed`, `assertions: 59 | 52 passed | 7 failed` (both-kept, MilkDrop+outputs coexist, unknown keys survive).
  - `TEETH-C3c-test_output_law-Q1-row.txt`: control `All tests passed (87 assertions in 14 test cases)`. Each mutant gives `test cases: 14 | 13 passed | 1 failed`: attachSettings calls restoreLast; the constructor reconciles; MainComponent restores next to attachSettings (2 calls); the menu case does something else first.

### 4. GREEN (raw lines, verbatim)
- ctests: test_output_plan `All tests passed (63 assertions in 4 test cases)`; test_app_settings `All tests passed (64 assertions in 6 test cases)`; test_output_menu_model `All tests passed (188 assertions in 7 test cases)`; test_output_law `All tests passed (87 assertions in 14 test cases)`.
- probe-outputs on the PRE-CHANGE app (the main checkout's `build/.../Audio-DNA.app`, built 15:22 from dc7adf9; `RED-C3d-probe-outputs-preC3-app-final.txt`, hardened row):
  - `FAIL  o_restore_empty: the app reads and writes its settings in the run's scratch file .../settings.json (NOT honoured: no test-mode settings line in err.log)`
  - `FAIL  o_restore_empty: 8080 /api/state.outputs.manager missing (...)`
  - `FAIL  o_poll_idle: 8080 /api/state.outputs.manager missing (...)`
  - `PY 13 PASS / 3 FAIL`, `PROBE-OUTPUTS RED`. The 13 C1/C2 rows are green on it (it carries C2).
- probe-outputs on the LANE app (`GREEN-C3d-probe-outputs-lane-final.txt`):
  - `PASS  o_restore_empty: the app reads and writes its settings in the run's scratch file .../outputs.d2ovfF/settings.json (err.log: AUDIODNA_SETTINGS_FILE honoured)`
  - `PASS  o_restore_empty: before: manager {'poll_enabled': True, 'poll_ticks': 618, 'reconciles': 0, 'settings_writes': 0, 'restore_calls': 0, 'interrupted': 0, 'saved': 0, 'restorable': 0}, outputs.live 0 (scratch settings absent: want saved == 0, restorable == 0, live == 0)`
  - `PASS  o_restore_empty: Restore Last Outputs ran (HTTP 200 {'ok': True, 'queued': True}; restore_calls 0 -> 1) and opened NOTHING: outputs.live 8080 0 / 7070 0, saved 0 -> 0, settings_writes 0 -> 0, scratch settings absent -> absent, Output-named windows so far [], max layer-0 windows 1`
  - `PASS  o_poll_idle: poll paused 5.00 s: 0 ticks, frame_time_ms 1.232 (18 samples) | poll running 5.28 s: 154 ticks = 29.2/s (>= 20.0), frame_time_ms 1.324 (19 samples) | delta +0.093 ms (|d| <= 0.3); reconciles +0, settings writes +0, outputs.live values seen [0], distinct displays lists 1 (load avg 4.33 3.84 3.66)`
  - `PASS  o_no_window_opened: 128 Quartz samples over the run: Output-named Audio-DNA windows [], max on-screen Audio-DNA layer-0 windows 1`
  - `PY 17 PASS / 0 FAIL`, `PROBE-OUTPUTS GREEN`
- SEEDED scratch settings (`GREEN-C3d-probe-outputs-lane-seeded-settings-final.txt`). The seed is `{"milkDropPresetDir": "/nonexistent/...", "outputs": {"version": 1, "targets": [{x:-100000, y:-100000, w:7, h:7, scale:1, main:false}]}, "futureKey": {"n": 7}}`: a saved target that can match no display.
  - `PASS  o_restore_empty: before: manager {..., 'saved': 1, 'restorable': 0}` shows that the app LOADED the saved set through its own path and opened nothing.
  - `PASS  o_restore_empty: Restore Last Outputs ran (...; restore_calls 0 -> 1) and opened NOTHING: outputs.live 8080 0 / 7070 0, saved 1 -> 1, settings_writes 0 -> 0, scratch settings f03a2a8e7b6c -> f03a2a8e7b6c` shows the file byte-identical: nothing was written at launch or by a no-op restore, and milkDropPresetDir and futureKey are untouched.
  - `PASS  o_poll_idle: ... 150 ticks = 29.1/s ... frame_time_ms 1.210 -> 1.080 ... delta -0.130 ms`
  - `PY 8 PASS / 0 FAIL`, `PROBE-OUTPUTS GREEN`
- Why `29c7e9e` exists: the first seeded subset run (`GREEN-C3d-probe-outputs-lane-seeded-settings-rerun.txt`) loaded no composition. It printed `frame_time_ms 0.000 (19 samples) | ... frame_time_ms 0.000 ... delta +0.000 ms` and PASSED vacuously. The row now loads static A and requires both means > 0. The earlier seeded run (`...-seeded-settings.txt`) failed only its sampler minimum: `o_no_window_opened: 3 Quartz samples` against the probe's own `>= 4`, because a 1-s subset is too short. The re-runs include o_poll_idle.

### 5. ctest, EXISTING BATTERY (lane build; no threshold edits; each batch under its own live-lock hold)
- ctest serial:
  - base 729. It was derived: the first build-lane listed 731, including the 2 new TEST_CASEs it had already compiled in, and C2's final count was 729 (`ctest-base-count.txt`).
  - C3 full build `100% tests passed, 0 tests failed out of 741` (`ctest-C3-full-build-tail.txt`).
  - Final HEAD `100% tests passed, 0 tests failed out of 741` (`ctest-final-HEAD-tail.txt`; build-lane was a no-op rebuild at HEAD).
  - The +12: test_output_plan 4, test_app_settings 6, test_output_menu_model +1, test_output_law +1.
- Existing probes on build-lane (`battery-existing-probes-lane.txt`), all GREEN:
  - render-state `PY 31 PASS / 0 FAIL`; crossfade `PY 35 PASS / 0 FAIL`; effects-parity `PY 46 PASS / 0 FAIL`; canvas `PY 15 PASS / 0 FAIL`; deck-clock `PY 10 PASS / 0 FAIL`;
  - fitmode `PY 10 PASS / 0 FAIL`; deck-tabs `6 PASS / 0 FAIL`; step3 `94 PASS / 0 FAIL`; mastersignal `22 PASS / 0 FAIL`; routines `98 PASS / 0 FAIL` (ROUTINES_RECORD_PAUSE=1.8);
  - resync `16 PASS / 0 FAIL`; onset-render `13 PASS / 0 FAIL`; downbeat-level `14 PASS / 0 FAIL`; manual-bpm `22 PASS / 0 FAIL`; routine-display `16 PASS / 0 FAIL`.
- Lock holds: the first probe-outputs runs 17:18:13-17:19:29 and 17:23:49-17:24:08; the final probe-outputs 17:56:28-17:57:59; batch 1 18:18:26-18:30:19; batch 2 18:52:05-18:59:53; batch 3 19:03:38-19:07:20.
- The first battery attempt used one lock hold per probe. It waited 31 min for render-state's first hold under heavy contention (renderperf, beatclock, tier1-diag, harmony). I stopped it while it was still WAITING: it held no lock and had launched nothing (`batch2-aborted.log` in the scratchpad). I re-ran it as 3 batches of 5 probes.
- Perf: probe-outputs' o_poll_idle ran with no compiler running (load avg 4.3-5.0 printed). The canvas probe's REPORT rows are its own.

### 6. SETTINGS SAFETY (binding rule) -- scratch file + before/after check
- Method: BOTH. (1) A TEST-ONLY settings-path override. `appSettingsFile()` honours `AUDIODNA_SETTINGS_FILE` (an absolute path) only in a test-server build (`#if AUDIODNA_TEST_SERVER`) running `--test-mode`, and logs `[Settings] test mode: AUDIODNA_SETTINGS_FILE = <path>`. `probe-outputs.sh` passes `$OUT/settings.json` on every run. Every C3-app probe-outputs run used a scratch file (absent, or seeded), and `o_restore_empty` asserts the log line. (2) The real paths were checked before and after all live work.
- The existing-battery probes launch without the variable. There the lane app reads the real path and writes nothing: no output was ever opened or closed and no MilkDrop preference was changed.
- BEFORE (`settings-safety-BEFORE.txt`, 16:11:36): `ABSENT  /Users/boriskarpman/Library/Audio-DNA/settings.json`, `ABSENT  /Users/boriskarpman/Library/Application Support/Audio-DNA/settings.json`, and `~/Library/Audio-DNA` does not exist.
- AFTER (`settings-safety-AFTER.txt`): (19:07:35) `ABSENT  /Users/boriskarpman/Library/Audio-DNA/settings.json`, `ABSENT  /Users/boriskarpman/Library/Application Support/Audio-DNA/settings.json`, and neither folder exists
- So there is no sha256 to compare: the file did not exist before and does not exist after. A byte-identical comparison against a real file was not possible, and none was needed. The seeded scratch file's sha256 was unchanged across the lane app's run (`f03a2a8e7b6c -> f03a2a8e7b6c`).
- The plan gate "the MilkDrop key survives an outputs write and vice versa" is `test_app_settings` on temp files, with the app's own key constants and `output::wantedToVar`: `AppSettings: the MilkDrop folder and the output set coexist, whichever is written last`. Its teeth are the pre-C3 writer (3 cases fail). The live seeded run adds that a real app LOADS the set through its own path and rewrites nothing. No live path can write `outputs` without opening a window, so an outputs write was never provoked live.

### 7. SCREEN SAFETY (output_window_opened = false, proven)
- By construction, only four paths can open an output window:
  1. `OutputManager::toggleDisplay` / `openDisplay`: the Output menu, the TopBar popup, Cmd+F. No gate made a menu pick or a key press.
  2. `restoreLast()`: only through the menu case. The test route refuses (409) unless `restorable == 0`, and the message thread re-checks. Every run had `restorable == 0`: the scratch file was absent, or held a target that matches no display.
  3. `reconcile()`'s Q6 reopen: only for an interrupted target. Those exist only after a LIVE output was closed by a display change, and no output was ever live.
  4. Nothing else. The `test_output_law` rows hold that OutputManager never shows, raises or floats a window itself, and that nothing opens at launch.
- There was no synthetic input, no AppleScript UI, no AX, and no deck or composition file with an output display.
- Quartz evidence:
  - probe-outputs runs: 71 (RED), 128 (GREEN), 58 (seeded) samples. Each run had `Output-named Audio-DNA windows []` and at most 1 layer-0 window, and after every quit `0 Audio-DNA windows in the FULL Quartz window list`.
  - The battery sampler: `window sampler 17:56:28..19:07:20: 16321 samples, Output-named Audio-DNA windows [], max on-screen Audio-DNA layer-0 windows 1, names seen ['Audio-DNA']` (`battery-window-sampler.txt`)
- Captures: the main window only, by Quartz window id, taken inside probe-outputs.sh. There was no full-screen capture, no lldb/debugserver/gdb/dtrace/Instruments, and no temporary env-var hook: `AUDIODNA_SETTINGS_FILE` is a permanent test-mode hook, and `strings build-lane/.../Audio-DNA | grep -c AUDIODNA_DEBUG_` = 0.
- `tests/visual/test_output_window_level.py` was never run or imported (probe-outputs imports only its pure picker, as C2 did). No pytest ran on tests/visual.

### 8. UI EVIDENCE (shots, absolute paths)
- `/Users/boriskarpman/projects/RealTimeAudio/.claude/worktrees/rta0927-w9/.harmony/.reports/s-rta-0927/outputs-c3-shots/06-output-menubar-fake3-restore-DISABLED-headless.png`: the whole Output menu (`AudioDNAMenuBar::getMenuForIndex(6)`) for a FAKE 3-display set. Display 2 is ticked, All Outputs Off carries its "Cmd+Shift+Esc", and **Restore Last Outputs is greyed** (nothing restorable: first launch, or an empty/absent saved set). The separator and Snapshot / Recording / Syphon follow, unchanged.
- `.../07-output-menubar-fake3-restore-ENABLED-headless.png`: the same menu, with Restore Last Outputs enabled (a saved output whose display is connected and free).
- `.../08-topbar-outputs-button-menu-fake3-restore-DISABLED-headless.png` and `.../09-topbar-outputs-button-menu-fake3-restore-ENABLED-headless.png`: the TopBar "Outputs" button's popup (the same item list), disabled and enabled.
- They are drawn headless with the app LookAndFeel's own popup painters (C2's method). In the app the menu-bar door is the native NSMenu: same titles, ids and enabled states, drawn by macOS. The item lists, including ids and "disabled" flags, are printed in `.../tool_uitoggle_snapshot-C3-output.txt`. For this machine's real display the list reads `Display 1 (1728x1117, main)`, `All Outputs Off (disabled)`, `Restore Last Outputs (disabled)`.
- TopBar unchanged:
  - `.../01-topbar-outputs-0-headless-pixel-identical-to-C2.png` (tool render) has the SAME sha256 prefix (`ec08aa641a2dec47`) as C2's `outputs-c2-shots/05-after-topbar-full-outputs-0-headless.png`. That is 0 differing pixels.
  - Live: `.../02-before-main-window-preC3-app-quartz.png` / `.../03-after-main-window-lane-app-quartz.png` (window-only, by Quartz id), cropped to the TopBar right section in `.../04-before-...-crop-preC3-app.png` / `.../05-after-...-crop-lane-app.png`. They show "Outputs: Off" identically; the only differing pixels (160, at x 3294-3313) are the FPS digits (119 vs 105).
- I looked at every shot listed here.

### 9. BORIS CHECKLIST (plan section 12 items 4 and 10, plus what C3 changes in 1-3)
1. (C3 change) Untick a display, or use All Outputs Off: the app FORGETS that output for "Restore Last Outputs". Quit with the outputs still ON if you want them back next time.
2. (C3 change) Cmd+Shift+Esc now also cancels a pending automatic comeback (see 4). Cmd+` works only WITHOUT Shift: Cmd+Shift+` does nothing to outputs.
3. (C3 change) Cmd+F works only WITHOUT Shift: Cmd+Shift+F no longer toggles the laptop-screen output.
4. Pull the projector cable while it plays. Within a blink the picture should be gone from every screen, and nothing black should stay. macOS may move the window onto the laptop for an instant before the app closes it (plan R6); it should not stay there. The TopBar should drop to the right count, and the projector's Output-menu item should disappear. Plug it back: the picture should return on the projector by itself (Q6), and the count and tick should come back. Also try changing the projector's resolution (System Settings > Displays) while it shows the picture: the picture should stay on the projector, resized, not close. With TWO identical projectors, only one with an output, change that one's resolution: the picture must stay on it, never jump to the other.
10. Quit with one or two outputs on. Launch again: no output should open by itself. Output > "Restore Last Outputs" (also in the TopBar "Outputs" button's list, right below All Outputs Off) should bring the set back with one click. When nothing is saved, or none of the saved screens is connected, the item is greyed. After your first change of outputs in a session, "last" means that new set.
(Unchanged from C1/C2: items 5-9 stand as delivered; C3 does not touch the frame path.)

### FILES CHANGED (dc7adf9..HEAD, 30 files, +1520 / -93)
- NEW `src/output/OutputTargets.{h,cpp}` -- DisplayInfo (moved), MatchRule, matchDisplay, OutputDiff, diffOutputs, sameTargets, wantedToVar/FromVar.
- NEW `src/model/AppSettings.{h,cpp}` -- settings.json read-modify-write; the kMilkDropPresetDir / kOutputs keys; defaultFile().
- `src/output/OutputManager.{h,cpp}`:
  - reconcile / scheduleReconcile / pollDisplays / reconcileIfDisplaysChanged; attachSettings / persistWanted / restorePlan / restorableCount / restoreLast;
  - interrupted_ / saved_ / lastSeen_ / lastWanted_; statsVar / setPollEnabled (test);
  - closeAll also clears interrupted; the menu gains the restore item; rebuildState keeps the counters.
- `src/output/OutputMenuModel.h` -- includes OutputTargets.h; buildOutputMenu(..., restoreId, canRestore); classifyOutputKey Shift-up.
- `src/ui/OutputWindow.{h,cpp}` -- parentSizeChanged() -> onDisplaysChanged.
- `src/ui/MenuBarModel.{h,cpp}` -- kOutputRestoreLast appended (1697); comments.
- `src/MainComponent.cpp` -- appSettingsFile() (test-only override); the MilkDrop load/save on AppSettings; attachSettings; the poll on every tick; the kOutputRestoreLast case; the test hooks.
- `src/test/TestServer.{h,cpp}` -- OutputsTestHooks; outputs.manager; output_restore_last; set_output_poll.
- `CMakeLists.txt` (+4 sources); `tests/CMakeLists.txt` (+2 ctests).
- NEW `tests/test_output_plan.cpp`, `tests/test_app_settings.cpp`; `tests/test_output_menu_model.cpp` (+Shift rows, +Restore case); `tests/test_output_law.cpp` (+Q1 row); `tests/tool_uitoggle_snapshot.cpp` (restore on/off renders).
- `.harmony/probe-outputs.{sh,py,json}` -- a scratch settings file on every run; o_restore_empty, o_poll_idle.
- Docs: `CLAUDE.md`, `docs/claude/{integration,testing-eyes,architecture}.md`, `.harmony/APP-INVENTORY.md`, `.harmony/notebook.md`.

### TESTS
- New ctests: test_output_plan (4 cases), test_app_settings (6 cases).
- Extended: test_output_menu_model (+1 case, +1 section; 7 cases), test_output_law (+1 row; 14 cases).
- Serial ctest 729 -> 741, all passing.
- Live:
  - probe-outputs lane `PY 17 PASS / 0 FAIL` (pre-change app `PY 13 PASS / 3 FAIL`); seeded `PY 8 PASS / 0 FAIL`.
  - The 15 existing probes: GREEN.

### SLIM CHECK
- Cut:
  - the plan's `OutputTarget` / `WantedOutputs` / `LiveEntry` wrappers (a vector of DisplayInfo carries everything C3 needs; later fields can add a struct);
  - arming unconnected saved targets on Restore (they stay saved; no automatic open that the user did not see);
  - a separate callAsync path (the existing AsyncUpdater is coalesced and cancel-safe).
- Kept on purpose:
  - the `previous` list in diffOutputs (the untouched-projector jump, teeth-proven);
  - the MatchRule::Track mode-change rung (ruling (a));
  - the test-only 8080 hooks (the only window-free way to RED/GREEN the two probe rows);
  - the Q1 law row (the only automated guard of "nothing opens at launch" that opens no window).

### ISSUES
1. **Rig-rule breach (disclosed):** one Bash command began with `cd /tmp;` ("NEVER put cd in a command"). It ran the four new/changed ctest binaries from /tmp (16:09, section 4's GREEN lines). It changed no file and touched neither the main checkout nor build/. Every later multi-step sequence went into a scratch script.
2. The fence was widened by the dispatch's own rows: `src/test/TestServer.{h,cpp}` and `.harmony/probe-outputs.*` are outside the plan's literal C3 fence (drift D10). `tests/test_output_menu_model.cpp`, `tests/test_output_law.cpp` and `tests/tool_uitoggle_snapshot.cpp` are also outside it (the Shift rows, the Q1 row, the UI evidence the dispatch asked for).
3. Signature deviations from plan section 5 (D5, D6, D8) are for the Reviewer.
4. Semantics decided without a plan line (D7, D11), for Harmony / Boris:
   - The launch-loaded set is SAVED and never auto-opens.
   - Restore opens only the saved targets whose display is connected and free; the others stay saved.
   - "last" = the set at the last output change: the first change of a session rewrites the file.
   - All Outputs Off / Cmd+Shift+Esc also cancels pending hot-plug reopens.
5. The first seeded probe-outputs subset run was RED only on the probe's own sampler minimum (3 samples < 4, a 1-s subset). A second subset exposed the vacuous frame-time pass that `29c7e9e` fixes. Both runs are kept as evidence.

### SKILL_PROPOSALS
- none new. C1's `offscreen-window-law-gate` covers the law row. The per-commit index export (`hash-object` + `update-index --cacheinfo` + `checkout-index --prefix`) is recorded in the notebook.

### RISKS
- R-a (medium, Boris check 4): real hot-plug is INFERRED from JUCE source, the display list refresh and parentSizeChanged (VERIFIED in the vendored tree: `juce_Displays.cpp` refresh -> `handleScreenSizeChange` -> `parentSizeChanged`). The matching rules are unit-proven, but the macOS event sequence on a real cable pull is not. AppKit may park the window on the laptop for up to one loop turn, or 33 ms via the poll (plan R6).
- R-b (medium): known imperfect matches.
  - Clamshell (the laptop display vanishes while the external becomes main): the laptop output's target follows the new main by the same-top-left rung. The projector's own output then closes as interrupted, so one window remains on the projector. When the lid opens, it stays there and the laptop gets none back.
  - A cable swap within one reconcile can move an output onto the swapped-in display of the same size.
  - Both are unit-visible, but not every case is pinned.
- R-c (low): `saved_` lives only in memory once the session's first output change rewrites the file. Restoring after a manual change still offers the launch set in that session, but a quit before the restore forgets it.
- R-d (merge): MainComponent.cpp, MenuBarModel.h and TestServer.cpp are edited by other lanes. My hunks are the settings helper, one line in the tick, one menu case, the test-hook block, and one appended enum value.

### METRICS
- Build-lane: full builds exit 0; the final no-op rebuild at HEAD; ctest 741/741; CLAUDE.md 22,679 B (cap 25,000); `strings ... | grep -c AUDIODNA_DEBUG_` = 0.
- Live lock: 6 holds as `outputs-c3` (17:18:13-17:19:29, 17:23:49-17:24:08, 17:56:28-17:57:59, 18:18:26-18:30:19, 18:52:05-18:59:53, 19:03:38-19:07:20), each released; at least 45 s between my holds (the withlock guard).

### KNOWLEDGE CONTEXT
- Tools used: grep + direct reads (the vendored JUCE tree: juce_Displays.cpp refresh, juce_ComponentPeer.cpp handleScreenSizeChange, juce_ResizableWindow parentSizeChanged, juce_MainMenu_mac.mm menuNeedsUpdate, juce_Files_mac.mm userApplicationDataDirectory, juce_Desktop.h getDisplays).
- Impact authority: grep, which is NOT authoritative, so I took a conservative posture: every consumer of DisplayInfo / buildOutputMenu / classifyOutputKey / the settings keys was enumerated before the change.
- Risk level: ELEVATED (cross-cutting: MainComponent + OutputManager + TestServer + menu). Mitigated by the full existing battery.

### PACKET QUALITY
- Clarity: CLEAR (plan5 sections 9/11 plus the dispatch's rulings).
- Missing context: the settings path (the plan and dispatch say Application Support; the code says ~/Library). What the launch-loaded set is (D7). How the probe rows can trigger the menu action without input (they need test-mode routes outside the fence).
- Unused context: plan sections 8.1-8.3 (C1 frame path).
- Self-assembly: LEGACY (no DEPARTMENT field).
- Self-brief files: plan5-final.md (the spec), outputs-c1.md / outputs-c2.md (drift and what exists; very useful), `.harmony/notebook.md` (TEST_SERVER flag, Connection: close, pitfall renumbering, the C2 menu-render method), CLAUDE.md.

### STATUS
DONE_WITH_CONCERNS. Every C3 gate item that can run without a window is done and green:
- RED first everywhere, with teeth on copies;
- ctest 741/741;
- probe-outputs GREEN on the lane / RED on the pre-change app;
- the seeded settings run byte-identical;
- the 15 existing probes GREEN;
- the menu renders looked at, the TopBar unchanged (pixel-identical headless, and identical live except the FPS digits);
- settings safety held (absent before and after, scratch files for every C3 run);
- no window opened.

Concerns:
1. The live hot-plug is Boris check 4.
2. The D7/D11 semantics need a ruling.
3. The fence widening (ISSUES 2).
4. The `cd /tmp;` rig breach (ISSUES 1).

### NEXT ACTION
For Harmony:
1. Rule on D6/D7/D11 and on ISSUES 1-2.
2. Run the behavioural gate on build-lane: probe-outputs under the lock, plus the battery.
3. Get an independent review, then merge.

Boris then runs checklist items 4 and 10 (and the C3 notes in 1-3) at the rig, ideally with the projector.

---

## Fix round (lane-name outputs-c3-fix; continues `lane/outputs-c3-0927` from `44f24a9`; 2026-09-27 19:22-19:55)

STATUS: DONE_WITH_CONCERNS
RESULT: 3 review findings verified. The MUST and the cheap SHOULD are fixed, one commit each. The third SHOULD is confirmed and left for a Harmony ruling, as the finding itself proposes.
- `fadfdda` (MUST): a settings write no longer drops saved output targets that Restore has not opened yet.
- `21b9a2b` (SHOULD 1): a test-mode app launched without `AUDIODNA_SETTINGS_FILE` now uses a scratch settings file, never the real one.
- SHOULD 3 (menu index race): the race is real. It is not fixed here.
- ctest 744/744 serial. probe-outputs `PY 17 PASS / 0 FAIL` on the fixed lane app. 4 existing probes GREEN. No output window was opened.
- One settings-safety incident in this round, disclosed below (FR-ISSUES 1). A teeth mutant briefly created and deleted `~/Library/Audio-DNA/settings.json`. The end state is the same as before: absent.
INBOX-RECHECK: none (no addendum message received during the fix round)

### FR-1. Findings: verify first, then the verdict
| # | finding | verified? | how | verdict |
|---|---|---|---|---|
| MUST | `persistWanted()` writes live + interrupted only, so saved targets are lost from disk | **CONFIRMED** | Read at 44f24a9: `OutputManager.cpp:279-296` builds `wanted` from `live_` + `interrupted_`. `AppSettings::update()` (`AppSettings.cpp`) sets the key on the root object, so `"outputs"` is replaced whole. `saved_` is not in the write. The 3 reproductions follow from that code. One refinement: All Outputs Off with nothing live and nothing interrupted returns early (`closeAll`, `:126`) and writes nothing. It drops saved targets only when at least one output was live or interrupted. | FIXED `fadfdda` |
| SHOULD 1 | a test-mode launch without `AUDIODNA_SETTINGS_FILE` falls back to the real settings.json | **CONFIRMED, with one correction** | Read at 44f24a9: `MainComponent.cpp:73-89` returns `AppSettings::defaultFile()` when the variable is unset or not absolute. The finding said "no test exercises this fallback". That is REFUTED: **11 of the 12 `--test-mode` probe scripts launch without the variable**. `grep -l -- --test-mode .harmony/probe-*.sh` lists 12, and `grep -l AUDIODNA_SETTINGS_FILE` lists only `probe-outputs.sh`. So every existing-battery run read the real path. None of them wrote it: nothing opened an output, and the MilkDrop folder is set only from the Preferences UI. | FIXED `21b9a2b`. I used the finding's second option (a definitely-scratch path). Its first option, refusing to start, would break those 11 probes. |
| SHOULD 3 | a menu command id is resolved to a display by a raw index at click time, so a hot-plug while the menu is open can act on another display | **CONFIRMED, with one correction** | `MainComponent.cpp:5944-5946` calls `toggleDisplay(commandId - kOutputFullscreenBase)`. `toggleDisplay` → `isLive` / `openDisplay` / `closeDisplay` index `currentDisplays()` at click time. JUCE's message queue source is added with `kCFRunLoopCommonModes` (`juce_MessageQueue_mac.h:56`), so the 30 Hz timer, and with it `pollDisplays()`, does run during NSMenu tracking. Correction: the renumbering comes from JUCE refreshing `Displays` on the screen-change notification. It does not come from the poll. The click resolves the index against the refreshed list with or without the poll, so C3's poll does not make the mis-resolution more likely. It only makes C3 act on the change sooner. | NOT FIXED. Harmony ruling needed. The fingerprint-at-menu-build fix touches `populateMenu`, both menu doors and JUCE's menu rebuilds (`menuItemsChanged` can rebuild the NSMenu while it is open). That is more than a cheap SHOULD. |

### FR-2. What changed
- `src/output/OutputTargets.{h,cpp}`: a new pure function, `wantedSet(live, interrupted, saved)`. It returns every live target, then every interrupted target, then every saved target that Restore has not opened yet, each target once.
- `src/output/OutputManager.{h,cpp}`:
  - `persistWanted()` writes `wantedSet(liveTargets, interrupted_, saved_)`.
  - A new `forgetSaved(target)` is called by `openDisplay`, `closeDisplay` and `closeWindow`. A manual open or close is the user deciding about that display, so Boris check 1 ("untick = forget") still holds for a display that was also in the saved set.
  - `attachSettings()` sets `lastWanted_ = saved_`. The file already holds that set, so nothing is rewritten until the set changes. The launch-writes-nothing property is preserved, and it is checked live below (`settings_writes 0 -> 0`).
  - `restoreLast()` is unchanged. It still erases restored saved targets by index.
  - `closeAll()` is unchanged. All Outputs Off still clears live and interrupted targets and now keeps the never-restored saved targets.
- `src/model/AppSettings.{h,cpp}`: a new `static juce::File testModeFile(const juce::String& overridePath)`. It returns the absolute override. Otherwise it returns one scratch file per process: `tempDirectory.getNonexistentChildFile("test-mode-settings", ".json")`, which is `~/Library/Caches/Audio-DNA/test-mode-settings.json`, cached in a function-local static.
- `src/MainComponent.cpp` `appSettingsFile(true)` (only under `#if AUDIODNA_TEST_SERVER`): routes through `testModeFile`. The `[Settings] test mode: AUDIODNA_SETTINGS_FILE = <path>` line is byte-identical to before, and probe-outputs asserts on it. There is a new line for the fallback: `[Settings] test mode: AUDIODNA_SETTINGS_FILE unset or not absolute -- scratch settings <path> (never the real settings.json)`. Production builds (`AUDIODNA_BUILD_TEST_SERVER=OFF`) are unchanged.
- Tests:
  - `tests/test_output_plan.cpp`: +1 case, "the wanted set keeps every saved target not yet restored". It covers the partial Restore, an unrelated output change, All Outputs Off, each target once, and the empty set.
  - `tests/test_output_law.cpp`: +1 row. `persistWanted` writes `wantedSet(...saved_)`; open, close and closeWindow each call `forgetSaved(`; `attachSettings` has `lastWanted_=saved_;`.
  - `tests/test_app_settings.cpp`: +1 case, "test mode never falls back to the user's real settings file". The safety checks are REQUIREs placed before the one write, so a regression cannot reach the real file.
- Docs:
  - `docs/claude/integration.md`: the saved-set semantics and the test-mode scratch file.
  - `docs/claude/testing-eyes.md`: the scratch fallback.
  - `.harmony/notebook.md`: a new entry covering the whole-key write, the 11 probes without the variable, and the teeth hazard.

### FR-3. RED first, teeth, GREEN (raw lines, verbatim; `outputs-c3-evidence/FIX-*`)
- MUST, RED on the pre-change tree (`git archive 44f24a9 src`, `FIX-RED-fix-persistWanted-pre-tree.txt`):
  - law row: `test_output_law.cpp:297: FAILED: CHECK( persist.find("wantedSet(") != std::string::npos )`, `:298 ... "saved_)"`, `:305 ... forgetSaved(` ×3, `:309 ... "lastWanted_=saved_;"`, `test cases:  1 | 1 failed`, `assertions: 11 | 5 passed | 6 failed`.
  - test_output_plan, RED by absence: `test_output_plan.cpp:178:43: error: no member named 'wantedSet' in namespace 'output'`.
- MUST teeth on COPIES (`FIX-TEETH-fix-persistWanted.txt`). The deliverables' sha256 was identical before and after: OutputTargets.cpp `bf01589c...`, OutputManager.cpp `aaa62c62...`.
  - Control: `All tests passed (68 assertions in 5 test cases)`.
  - **The PRE-CHANGE semantics transplanted** (wantedSet = live + interrupted, the pre-fix loop): `:178 FAILED: CHECK( output::sameTargets(output::wantedSet({ kLaptop }, {}, { kProjector }), { kLaptop, kProjector }) )` (the partial Restore). `:183` (an unrelated change), `:187` (All Outputs Off) and `:192` also fail. `test cases:  5 |  4 passed | 1 failed`, `assertions: 68 | 64 passed | 4 failed`.
  - No de-duplication: `:192 FAILED`, `assertions: 68 | 67 passed | 1 failed`.
  - Law control: `All tests passed (11 assertions in 1 test case)`. Each law mutant fails with `assertions: 11 | 10 passed | 1 failed`: persistWanted with `{}` instead of `saved_` (`:298`), openDisplay without forgetSaved (`:305`), and attachSettings without `lastWanted_ = saved_` (`:309`).
- SHOULD 1, RED on the pre-change tree (`FIX-RED-fix-testModeFile-pre-tree.txt`): `test_app_settings.cpp:110:28: error: no member named 'testModeFile' in 'AppSettings'`.
- SHOULD 1 teeth on SANDBOXED copies (`FIX-TEETH-fix-testModeFile.txt`). In every copy, including the control, `defaultFile()` points into the scratchpad. The real paths were ABSENT before and after, and the deliverable sha256 `39fdf29a...` was identical before and after.
  - Control: `All tests passed (16 assertions in 1 test case)`.
  - **The PRE-CHANGE fallback transplanted** (not absolute → `defaultFile()`): `test_app_settings.cpp:118: FAILED:` (`REQUIRE( f != real )`, `AUDIODNA_SETTINGS_FILE = ''`), `test cases: 1 | 1 failed`. The run stops there, before any write. The sandboxed fake-real file was never created.
  - A new scratch file on every call: `:127 FAILED: CHECK( AppSettings::testModeFile({}) == f )`, `:128 FAILED: ... read("probe")) == 7 )`, `assertions: 16 | 14 passed | 2 failed`.
  - This mutant was toothless in the first version of the test (`All tests passed`), because nothing had been written, so both calls got the same name. The case now writes the scratch file and reads it back.
- SHOULD 1 live witness (`FIX-live-witness-and-probe-outputs.txt`, one lock hold 19:45:00-19:45:58). Both runs were `open -g ... --args --test-mode` with `AUDIODNA_SETTINGS_FILE` unset.
  - RED on the pre-fix lane app: a byte copy of build-lane at 44f24a9, sha256 `54bf7fc9...` = build-lane at 44f24a9. err.log `[Settings] lines:` is empty, so the app silently used `defaultFile()`.
  - GREEN on the fixed app: `2 [Settings] test mode: AUDIODNA_SETTINGS_FILE unset or not absolute -- scratch settings /Users/boriskarpman/Library/Caches/Audio-DNA/test-mode-settings.json (never the real settings.json)`.
  - Both runs: the real paths were ABSENT before and after, no scratch file was written, `Output-named Audio-DNA windows []`, max layer-0 windows 1, `PASS app terminated`.
  - The 8080 `/api/state` read in the witness raised a Python traceback (it used 127.0.0.1:8080; the 8080 server answers on `[::1]`, per probe-outputs). This had no effect on the witness verdict. The manager counters come from probe-outputs instead.
- GREEN probe-outputs on the fixed lane app, same hold. `settings: .../settings.json (absent)`.
  - `PASS  o_restore_empty: ... Restore Last Outputs ran (HTTP 200 {'ok': True, 'queued': True}; restore_calls 0 -> 1) and opened NOTHING: outputs.live 8080 0 / 7070 0, saved 0 -> 0, settings_writes 0 -> 0, scratch settings absent -> absent`.
  - `PASS  o_poll_idle: poll paused 5.05 s: 0 ticks, frame_time_ms 1.219 (18 samples) | poll running 5.25 s: 153 ticks = 29.1/s (>= 20.0), frame_time_ms 1.127 (19 samples) | delta -0.092 ms (|d| <= 0.3); reconciles +0, settings writes +0 ... (load avg 3.07 3.26 3.39)`.
  - `PASS  o_no_window_opened: 129 Quartz samples`; `PY 17 PASS / 0 FAIL`; `PROBE-OUTPUTS GREEN`.
- The MUST fix has no live witness, and cannot have one screen-safely. `persistWanted` changes the file only after a window opens or closes. Without a window, wanted = saved = `lastWanted_` at every reconcile, and the old code likewise wrote nothing (`{}` vs `{}`). probe-outputs above confirms the no-window path still writes nothing (`settings_writes 0 -> 0`). The fixed behaviour rests on the pure unit case plus the law row that pins the wiring. It joins Boris check 10.

### FR-4. Battery (existing probes re-run, never re-thresholded)
- ctest serial at the final tree: `100% tests passed, 0 tests failed out of 744` (`FIX-ctest-final-tail.txt`). 741 → 744 is the 3 new cases.
- 4 existing probes that launch `--test-mode` WITHOUT the variable, so they now take the scratch fallback. One lock hold, 19:49:08-19:53:35 (`FIX-battery-existing-probes.txt`):
  - deck-tabs `6 PASS / 0 FAIL`
  - canvas `PY 15 PASS / 0 FAIL`
  - fitmode `PY 10 PASS / 0 FAIL`
  - deck-clock `PY 10 PASS / 0 FAIL`
- Canvas and deck-clock err.logs carry the scratch line (4 occurrences in all). Sampler: `1018 samples, Output-named Audio-DNA windows [], max on-screen Audio-DNA layer-0 windows 1`.
- Not re-run: the other 11 probes (C3 round: all GREEN) and tests/visual Tier-1. The fix touches only the settings path (absent either way on this rig) and the output-persistence path, which no probe reaches without a window.
- Build: `build-lane` no-op rebuild at HEAD (0 compile/link steps). `strings ... | grep -c AUDIODNA_DEBUG_` = 0 (no temporary hook was used). The fallback string is in the binary (`grep -c "scratch settings"` = 1).

### FR-5. Settings and screen safety
- Real paths BEFORE (19:22:53): `ABSENT ~/Library/Audio-DNA/settings.json`, `ABSENT ~/Library/Application Support/Audio-DNA/settings.json`. AFTER (19:54:03): the same, plus `ABSENT ~/Library/Audio-DNA` (the folder) and no `test-mode-settings` file in `~/Library/Caches/Audio-DNA`.
- Screen: no output window was opened by any path. Every live launch used `open -g`, with Quartz samplers running: `[]` Output-named windows in 21 + 21 + 129 + 1018 samples. There was no synthetic input, no lldb, no full-screen capture, and no pytest on tests/visual.

### FR-ISSUES
1. **Settings-safety incident (disclosed; state restored).** The first run of the SHOULD-1 teeth transplanted the pre-change fallback (`return defaultFile();`) into a scratch copy. The first version of the new test guarded with CHECK, not REQUIRE, so Catch continued to the case's `update("probe", 7)` and then `deleteFile()`, both on the REAL path. The file was created and deleted within that run. The run also created the folder `~/Library/Audio-DNA/` (birth `Sep 27 19:27:20 2026`, `stat -f %SB`), which was left empty. I removed it with `rmdir`, which removes only an empty folder. I also removed the empty `~/Library/Caches/mutant` that the mutant binary created. The net state is identical to BEFORE: all real paths are absent, and no other process was involved. The fix has two layers:
   - The test REQUIREs the path is not the real file and is inside the temp folder before its one write.
   - The teeth script sandboxes `defaultFile()` in every copy and prints the real paths before and after.
   The output of the unsafe run was overwritten by the safe re-run. This account comes from the live `ls` / `stat` taken at the time. The lesson is in `.harmony/notebook.md`.
2. Semantics change for Harmony/Boris. This supersedes the C3 report's D7 sentence "the first output change of a session rewrites the file with live + interrupted", R-c, and Boris check 10's "after your first change of outputs in a session, 'last' means that new set". The saved set now survives until each saved target is either restored or decided for by a manual open or close of its display. All Outputs Off keeps never-restored saved targets. Boris check 1's "All Outputs Off forgets" now applies to live and interrupted outputs only.
3. The first run of the MUST teeth printed a Python `AssertionError: ('namespace output', 2)` for the two controls. The control's no-op patch matched twice, and the copy was left unmodified, so the controls were still valid. The script was fixed and re-run; the committed evidence is the clean run.
4. The first version of the scratch-file test had a toothless "same file for the whole run" check (FR-3). It was strengthened before commit.

### FR-FILES (44f24a9..HEAD)
- `fadfdda`: `src/output/OutputTargets.{h,cpp}`, `src/output/OutputManager.{h,cpp}`, `tests/test_output_plan.cpp`, `tests/test_output_law.cpp`, `docs/claude/integration.md`
- `21b9a2b`: `src/model/AppSettings.{h,cpp}`, `src/MainComponent.cpp`, `tests/test_app_settings.cpp`, `docs/claude/{integration,testing-eyes}.md`, `.harmony/notebook.md`
- this commit: this section and `outputs-c3-evidence/FIX-*`

### FR-BORIS CHECKLIST (replaces item 10 and adjusts item 1)
1. Untick a display: the app forgets that output for "Restore Last Outputs". All Outputs Off forgets every output that was on, or waiting for its cable. It keeps saved outputs from a previous session that you have not restored yet.
10. Quit with two outputs on (laptop + projector). Unplug the projector and launch again: nothing opens. Output > Restore Last Outputs opens the laptop output only. Quit and plug the projector back in, then launch: Restore Last Outputs must still offer the projector output and open it. Before this fix, the projector was forgotten the moment you clicked Restore the first time.

### FR-LANE STATE
- Branch `lane/outputs-c3-0927`. Base for this round `44f24a9`. HEAD = the report commit after `21b9a2b`. Not merged or pushed.
- Lock: every hold was released (19:45:00-19:45:58 and 19:49:08-19:53:35). No app I launched is running. `.venv` link removed. `build-lane` kept (no-op at HEAD).
- Found, not fixed: SHOULD 3 (the index-at-click race; ruling needed).
