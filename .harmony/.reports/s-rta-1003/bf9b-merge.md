# bf9b merge-in lane (s-rta-1003)

## Stage M1 -- merge-in + R-S3 (rulings H-1, R-S3)

STATUS: DONE

Builder started 13:37:43, done 13:52 (2026-10-03, times from `date`). No live app in this stage (none launched, no
lock taken). Scratch (logs, scripts): `<session scratchpad>/bf9b-merge-M1/`.
Pre-merge: lane/bf9b a7491d4 (tree clean), main 5abdf01b3f4dcb5435b95a2a264711628d77fa39, merge-base 11820fa.
Head after M1: the commit that adds this report (see `git log`); src / tests head = 98d71fe (the merge commit).

### Verdict
main is merged into lane/bf9b in ONE merge commit (98d71fe, parents a7491d4 + 5abdf01; the 49 lane commits keep their
SHAs). 6 textual conflicts and 3 semantic conflicts resolved keeping both sides. Build rc 0, ctest serial 1234 / 1234,
probe-tsan-unit 5 / 5 with 0 warnings, 0 conflict markers, CLAUDE.md 24,224 B. Everything below is VERIFIED (run or
read this stage) unless marked INFERRED. Nothing was run live: how the merged app behaves on screen is NOT checked
here (M3).

### Commits
| sha | what |
|---|---|
| 98d71fe | merge(s-rta-1003 bf9b): main into lane/bf9b (6 conflicts + the 3 semantic fixes the build needed) |
| 790fe8f | docs: R-S3 Pitfall NN -> 67 outside the conflict hunks (architecture.md, performance-controls.md, pitfalls.md wording) |
| 8803378 | docs + probe: R-S3 one /api/debug/undo (integration.md, testing-eyes.md, APP-INVENTORY.md, probe-boxes.py) + APP-INVENTORY counts re-derived |
| (next) | this report |

### Conflict hunks (6 files, 8 hunks) and semantic fixes (3)
| # | file | main wanted | the lane wanted | resolution |
|---|---|---|---|---|
| C1 | src/ui/DeckTabRow.h (1 hunk) | `editorRect(tab, rowWidth)` -- the rename box's rect (ui, Pitfall 65) | `undoRemoveHint(deckName, playingLayers)` -- the Undo Remove text naming the layers that keep playing | BOTH functions, the lane's first, each with its own comment; neither body changed |
| C2 | src/ui/DeckView.cpp rebuildGrid (1 hunk) | `cell->setVideoInfoSource(&videoInfoSource_)`; then `setClip(layer->getClipAt(col))` / `setActive(activeClipColumn == col)` (per-deck layers) | `setClip(deck->getClip(layerIdx, col))` / `setActive(runtime().activeRef() == ClipRef{deck->id, col})` | main's setVideoInfoSource line + the lane's two lines (the cell reads the SHOWN deck's row and lights by ClipRef; the codec tooltip keeps its source) |
| C3 | src/ui/DeckView.cpp setupDeckTabs (auto-merged, listed because both sides edited the same statement group) | `tabTooltipFor(deck, showing)`, `onClick = tabClicked(i)` (a click on the showing tab does nothing) | `btn->dot = deckIsPlaying(deck.id)` | git kept all three; read and confirmed |
| C4 | CLAUDE.md Key capabilities (hunk 1) | the 18-category source breakdown dropped (bytes) | "decks are boxes of clips over one shared layer stack" added, "persistent layers across deck switches" and "cross-deck transitions" removed | the lane's line with main's shortening ("18 registry categories,") |
| C5 | CLAUDE.md pitfall index (hunk 2) | lines 64, 65, 66 | 63 reworded (ClipRef packing), "NN." line | 63 = the lane's wording, 64-66 = main's, the lane's line numbered 67 (R-S3) |
| C6 | docs/claude/pitfalls.md (1 hunk) | entries 64, 65, 66 | 63 extended (ClipRef), entry NN | 63 lane, 64-66 main verbatim, the lane's entry = 67 |
| C7 | docs/claude/performance-controls.md (1 hunk) | two new paragraphs (deck rename in place; clip file info + Show in Finder) before Beat Snap; Beat Snap unchanged | Beat Snap reworded (a deck switch never cancels a queued trigger; Remove Deck cancels) | main's two paragraphs, then the lane's Beat Snap paragraph |
| C8 | .harmony/APP-INVENTORY.md (2 hunks) | DeckView row: tab click / double-click rename text; ClipCell and ClipInspector rows (BF3) | DeckView row: bf9b pads / column header / tab dot text; LayerStrip row (badge); LayerInspector row (no persistent) | DeckView = the lane's row with main's tab text spliced in ("click = switch deck = the grid only, bf9b; a small dot ...; a click on the deck already showing does nothing ...; double-click ... BF8"); LayerStrip + LayerInspector = lane; ClipCell + ClipInspector = main |
| S1 | src/api/ApiServer.{h,cpp}, src/MainComponent.cpp (auto-merged; did not compile: two `handleDebugUndo` / `onDebugUndo`, two registrations of POST /api/debug/undo) | ui lane: `POST /api/debug/undo {redo}` -> `handleMenuCommand(kCompUndo / kCompRedo)` | bf9b: `POST /api/debug/undo` -> `undoAffectsLayerOrder(); undo(); refreshAfterUndoRedo()` | R-S3: main's route only. main's `kCompUndo` case is the same three statements (MainComponent.cpp `case C::kCompUndo`), so the lane's registration, handler, declaration, callback and wiring were removed; `/api/debug/remove_deck` and `/api/debug/save_composition` kept |
| S2 | tests/test_clip_cell_media.cpp (main's new file; did not compile on the lane) | `deck->getLayer(0)->clips[3]`, `->getClipAt(3)` (per-deck layers) | a deck has rows (`Deck::getRow`), no layers | 4 lines `getLayer(0)` -> `getRow(0)`; the assertions unchanged |
| S3 | tests/test_deck_tab_rename.cpp case (f) (main's new file; did not compile) | `REQUIRE(r.comp.removeDeck(2))` -- deck gone, the box closes without renaming | `Composition::removeDeck` deleted; `retireOrEraseDeck` returns true only when RETIRED | `CHECK_FALSE(r.comp.retireOrEraseDeck(2))` (erased: no layer plays deck C) + `REQUIRE(r.comp.decks.size() == 2)`; the five following assertions unchanged |

Semantic review of main's code in the auto-merged files (read, not only compiled):
- Every `getActiveDeck()` / `activeDeckIndex` main added (git diff 11820fa main, added lines: MainComponent
  `onDebugClipMedia`, `revealClipAt`; DeckView `tabClicked`, `tabRowMouseDown`, `setupDeckTabs`, `deckTabsStateForTests`)
  is BOX work on the shown deck (the grid's cell, the tab row) -- right under Pitfall 67. None is in src/render or
  Autopilot.cpp. `MainComponent::videoInfoFor` looks the player up by clip id, not by deck.
- main changed NOTHING under src/render (`git diff --stat 11820fa main -- src/render` empty): bf10's work is
  ProjectMSource.{h,cpp} + CMake. So H3 (the playlist advance walks the PLAYING layers, Renderer.cpp) and H2 (a deck
  switch = `handleDeckSwitch`: the index, the fence token, `DeckView::showDeck`) are the lane's code unchanged; their
  text lints pass in ctest (#1107 B4d / H2, test_render_thread_lint).
- The rename box across a deck switch: on the lane a switch with the same grid shape calls `showDeck()` -> `refresh()`
  (no row rebuild); `refresh()` re-sets every tab's colour, label and `tabTooltipFor(deck, isActive)` (auto-merged, read
  at DeckView.cpp `refresh`), so "Double-click: rename" follows the shown tab; the box is not touched. A switch that
  changes the shape goes through `rebuildGrid` -> main's "box back on top" code. INFERRED from reading: not run live.

### Gates (raw lines; logs in the scratch dir)
- Reconfigure + build (build.sh, 13:39:37-13:42:36): `configure rc=0`, `-- libprojectM:
  /Users/boriskarpman/.local/opt/projectm-4.1.1-fbo1/lib/cmake/projectM4 (render_frame_fbo: yes)`; first build
  `build rc=2`, 11 `error:` lines, ALL in the two test files of S2 / S3 (the app and every other target built);
  after S2 / S3 `build rc=0`, 0 `error:` lines (13:43:40); at the final head `build rc=0` (nothing to rebuild).
- ctest SERIAL under /tmp/audiodna-ctest.lock, on the merge tree (13:43:46-13:45:56) and again at the final head
  (13:48:35-13:50:42): `100% tests passed, 0 tests failed out of 1234` / `Total Test time (real) = 126.63 sec`
  (first run 130.46 sec).
- Count: merged 1234 = main (5abdf01) 1191 + lane (a7491d4) 1157 - base (11820fa) 1114. Base 1114 is from the lane
  report ("count(STAGE_P_BASE) 1114") and APP-INVENTORY ("ctest -N 1114 on merged main b844b65"); it was not rebuilt
  here. No test file other than tests/CMakeLists.txt was changed by both sides (comm of the two `git diff
  --name-only` lists), so no case was added twice and none is lost: main +77, lane +43 net (60 added, 21 retired, +3
  Stage P, +1 fix round). `catch_discover_tests(` lines 126 = 124 + 120 - 118.
- `.harmony/probe-tsan-unit.sh build-tsan` (13:46:38-13:46:44): `tsan rc=0`; `probe-tsan-unit: ctest -L tsan finds 5
  [tsan] cases (expected 5)`; `100% tests passed, 0 tests failed out of 5`; "WARNING: ThreadSanitizer" count 0. (cmake
  re-ran the configure; both targets were up to date -- main changed no file they compile.)
- The lane's B4 text lints inside ctest, all Passed: #1104 "render thread: one trigger-tuple load per layer per pass
  (pinned counts)"; #1106 "bf9b: box / stack structure writers sit only in audited sites (pinned counts)" (B4f);
  #1107 "bf9b B4d / H2: a deck switch path touches nothing that plays (smoke, one level)"; #1108 "bf9b B4g: a deck
  switch is never an Undo step -- the tab click is exactly handleDeckSwitch(deckIdx); ..."; #1170 "shared model field
  types are pinned at compile time"; #776 "bf9b S3.3: the TopBar has no deck Fade control". B4a / B4b are greps
  outside ctest, re-run by hand: B4a hits only src/model/ShowMigration.h (the legacy `persistent` /
  `globalTransitionSpeed` readers, as at the lane head) plus two unrelated comment words ("persistent" in
  EmbeddedShaders.h:5037, Renderer.h:347); B4b (`getActiveDeck(` / `activeDeckIndex` on code lines of src/render/* and
  src/model/Autopilot.cpp) 0 hits.
- Conflict markers: `git grep -E '^(<<<<<<<|=======|>>>>>>>)( |$)' -- . ':!.harmony/.reports'` = 0.
- CLAUDE.md: 24,224 B (cap 25,000; main 23,962, lane 24,264). The lane's rule 15 ("Decks are boxes; the layers
  play") and main's lines (Deck tab row "double-click the deck on screen = rename in place", index 64-66) are all
  present. Nothing had to be moved out.

### R-S3
- Pitfall order: 64 mkvidx, 65 ui, 66 bf10, 67 bf9b in CLAUDE.md's index and docs/claude/pitfalls.md; "Pitfall NN" in
  docs / src / tests / CLAUDE.md = 0 (`git grep`). .harmony/HANDOFF.md's "Pitfall NN" lines are about OTHER lanes'
  old comments (ledger 7) -- left alone. 68 is free.
- One undo route: `git grep -c 'debug/undo"' src/api/ApiServer.cpp` = 1. probe-boxes k1a_undo and k9c post
  `{"redo": false}` to it (they already used the same URL; the body is now explicit). Not run live in M1.
- docs/claude/testing-eyes.md lists `/api/debug/undo` once and points at integration.md for the lane's
  remove_deck / save_composition (described there once).

### found_not_fixed
1. `.harmony/probe-ui-files-rename.sh` row R3 expects "tab_click 2 -> builds +1" (a deck switch rebuilds the tab
   row). On the merged lane a same-shape switch is `showDeck()` -> `refresh()`: `tab_row_builds` stays +0. INFERRED
   from the code (not run: no live app in M1). This is a pre-registered bar that Boris's ruling changes ("a deck
   switch changes only the grid; no strip is rebuilt") -- Harmony must rule it (H-5: list with the reason, never
   re-threshold) before M3 re-runs that probe. Rows after R3 that count builds across a switch may be affected the
   same way.
2. APP-INVENTORY's "242 embedded shaders" was NOT re-derived: I found no counting method that gives 243 / 242
   (`R"(` occurs 269 times, 250 `const char*` lines). It stands on: main touched nothing under src/render.
3. APP-INVENTORY's "41 registered REST routes" headline is older than both sides (ApiServer.cpp has 67
   registrations, 25 of them /api/debug/*); not touched (out of this stage's scope).
4. docs/claude/performance-controls.md (main's rename paragraph) says "a REST / MIDI / OSC / genre deck switch keeps
   the box on its deck"; the lane made the genre deck switch inert. Harmless wording, left as main wrote it.
5. R-N1: 33 .harmony scripts match the by-name quit / kill grep on the merged tree (the lane's 31 + main's
   probe-milkdrop.sh and probe-ui-files-rename.sh; probe-quit-ours.sh excluded) -- M2's scope.
6. A git post-commit hook ("graphify hook: launching background rebuild", log ~/.cache/graphify-rebuild.log) fired on
   each of my commits. It is not mine; the worktree stayed clean after it.
7. Out-of-scope items (the round-1 MUST's residual use-after-free read, Undo of Add / Load / Duplicate Deck, the
   wiring lint, K5 Link-on, S2b 4.B omissions, grid NITs, the strip badge removal): not tripped over, not touched.

### next_stage_notes (M2 builder)
- Head: see `git log -1`; build-lane holds the merged Release app (built 13:43 from 98d71fe's src) and all tests;
  build-tsan is current. A reconfigure is NOT needed unless CMake files change.
- The by-name quit list for R-N1 (grep `tell application "Audio-DNA" to quit|pkill.*Audio-DNA|killall.*Audio-DNA|
  adna_kill` over .harmony/*.sh, *.py): gate-s165 probe-async-load probe-beatclock probe-btguard probe-capture
  probe-crossfade probe-deck-path probe-deck-tabs probe-downbeat-level probe-effects-parity probe-finalize-loop
  probe-fitmode probe-idle-paint (.py + .sh) probe-image-load probe-lane3 probe-manual-bpm probe-mastersignal
  probe-media-open probe-milkdrop probe-onset-render probe-outputs probe-resync probe-routine-display probe-routines
  probe-seq-vram probe-step3 probe-tempo-silence probe-tempo-start probe-tsan probe-ui-files-rename probe-video
  probe-vupload. bf10's probe-milkdrop has an attach-mode refusal already -- read it before changing it.
- H-4: gates-r2 NIT 6 (record_ourpid after the 60 s health loop) and NIT 4 (k1b_duplicate registered row) are M2's.
- M3: probe-milkdrop.py is now in the tree (m9b_deck_switch_live goes there); RED arm = main 5abdf01's app
  (/Users/boriskarpman/projects/RealTimeAudio/build/..., read-only). found_not_fixed 1 needs a ruling first.
- ctest count to expect: 1234 (+ whatever M2 adds).

### Notes for .harmony/notebook.md (Harmony appends)
- A merge whose two sides each add a route / handler with the SAME name auto-merges textually and fails only at
  compile (two `handleDebugUndo` declarations): grep `server_.Post("` for duplicate paths after any merge of ApiServer.
  | discovered: src/api/ApiServer.cpp
- `Composition::retireOrEraseDeck` returns true only when the deck was RETIRED (a layer plays it); an erased deck
  returns false -- a test that wants "the deck is gone" checks `decks.size()`, not the return. | discovered:
  tests/test_deck_tab_rename.cpp case (f)
- On the shared stack a same-shape deck switch does not rebuild the tab row (`showDeck` -> `refresh`); anything that
  counted on a rebuild per switch (probe-ui-files-rename R3 `tab_row_builds`) reads +0. | discovered:
  src/ui/DeckView.cpp showDeck

### PACKET QUALITY
- Clarity: CLEAR.
- Missing context: the B4a / B4b grep script (the lane's scratch b4.py is gone; re-derived from the lane report's
  wording); the method behind the "embedded shaders" count.
- Unused context: ruling-bf9b's K gate list (no live rows in M1); ui.md / bf10.md lane reports (the code and
  Pitfalls 65 / 66 were enough for the two source hunks).
- Self-brief files: rulings-bf9b-merge.md, rulings-bf9b-mergein.md, bf9b.md fix-round sections, plan-bf9b HARMONY
  ADOPTION -- all useful, none stale. No DEPARTMENT / KNOWLEDGE_TOOLS block: no knowledge tools -- grep-only (no code
  was judged dead on "no callers"; the removed undo handler was a duplicate definition).
- pulse.json: GREEN; claims "general" by two harmony sessions (the dispatcher), no area conflict.
INBOX-RECHECK: none

### SLIM CHECK
3 source files + 2 test files changed beyond the textual resolutions, each line traced to a compile failure or R-S3;
no drive-by edits; no new abstractions; no test added (none required by the packet).

---

## Stage M2 -- by-name-quit sweep (R-N1; H-3, H-4)

STATUS: DONE_WITH_CONCERNS (2026-10-03 13:52 -> 14:16, from 555983a). Concerns = the stop items below.

### Verdict
No script in `.harmony/*.sh|*.py`, `tests/`, `cmake/` quits or kills Audio-DNA by name any more, outside
`.harmony/probe-quit-ours.sh`'s two only-ours lines. VERIFIED (run this stage): the by-name grep, `bash -n` /
`py_compile` on every edited file, the helper selftest (88 ok / 0 FAIL, 7 mutants each FAIL), a shimmed run of 33
probes that each refuse a fake foreign pid before any `open` / `osascript`, three live probes + `k1b_duplicate` on
both arms, 0 windows after. NOT covered: the 15 archived scripts under `.harmony/.reports/**` (stop item 1);
probe-tsan's and probe-outputs' start refusal dynamically (static only); all but 4 of the converted scripts were not run
live end to end (only their start refusal ran).

### Commits
| sha | what |
|---|---|
| bcee670 | helper: `refuse_foreign_start`, `record_ourpid` waits for the process (callable right after the launch), `quit_ours` says when no pid was recorded, `ask_ours_to_quit` / `kill_ours` |
| 5f608e2 | H-4 NITs: probe-canvas / probe-boxes / probe-render-state record the pid BEFORE the health loop (NIT 6); `k1b_duplicate` a registered row (NIT 4) |
| 3c57c8c | `.harmony/probe-quit-ours-selftest.sh` (needed `git add -f`: `.harmony/*` is gitignored -- bcee670's message names it) |
| 425d499 | the sweep: 33 scripts |
| d5ddd03 | tests/visual/test_output_window_level.py's printed "pkill -9 -f Audio-DNA" remedy reworded; selftest `--sweep` arguments for finalize-loop / tsan |
| (next) | this report + evidence (`.harmony/.reports/s-rta-1003/bf9b-merge-M2/`) |

### Step 1 -- inventory (re-counted on the merged tree)
Pattern used (the packet's, plus the `quit app "Audio-DNA"` form it did not list -- 9 more lines in 8 scripts):
`tell application "Audio-DNA" to quit|quit app "Audio-DNA"|pkill.*Audio-DNA|killall.*Audio-DNA|adna_kill|kill .*pgrep.*Audio-DNA|pkill.*AudioDNA|killall.*AudioDNA`
over `.harmony tests cmake` (`scripts/` does not exist), `*.sh` + `*.py`.
- 33 scripts in `.harmony/` (32 .sh + probe-idle-paint.py) = the lane's 31 + main's probe-milkdrop.sh and
  probe-ui-files-rename.sh. Every one defined or called `adna_kill` (= `kill` of EVERY Audio-DNA pid) and / or sent a
  by-name Apple-event quit. Also found by a wider `kill` read: probe-step3.sh's opt-in crash row
  (`kill -9 "$(adna_pids | head -1)"`), probe-lane3.sh (`adna_kill` with no graceful quit at all),
  probe-idle-paint.py (`os.kill` of every Audio-DNA pid).
- 1 in tests: tests/visual/test_output_window_level.py:1015 PRINTED "Manual remedy: pkill -9 -f Audio-DNA" (its own
  teardown kills by pid). `tests/visual/vj_controller.py` kills its own Popen only.
- 15 archived scripts under `.harmony/.reports/**` (21 lines): NOT converted -- stop item 1.
Before / after per file: `git show 425d499 --stat`; the full pre-sweep hit list is the selftest's RED run below.

### Step 2 -- conversion (same shape everywhere)
1. `adna_kill` deleted; `. "$ROOT/.harmony/probe-quit-ours.sh"` right after `ROOT=`.
2. start: `adna_running && { echo "REFUSE: ... Quit it, then re-run."; exit 64; }` -> `refuse_foreign_start || exit 64`
   ("REFUSE: Audio-DNA already running (pid N) -- this run did not start it (it may be Boris's): never quit, kill or
   touch it"). No script kills "the stale one".
3. `record_ourpid; echo "ours: pid ${OURPID:-none}"` on the line after every launch, before the health wait
   (NIT 6 -- also moved in probe-canvas / probe-boxes / probe-render-state). record_ourpid now waits up to 10 s for
   the process to exist.
4. every quit -> `quit_ours` (`|| RC=1` where the script has RC). Where the ROW times the quit or reports how it
   ended, the two halves are used so the row keeps its structure and timing: `ask_ours_to_quit` (the by-name event
   only while OURPID is the only Audio-DNA) + `kill_ours` (OURPID only): probe-async-load, probe-btguard (event in
   the background, 30 s clock), probe-tsan (`graceful=yes|no`), gate-s165 / probe-deck-path / probe-tempo-silence
   (their by-unix-id System Events quit stays; only the by-name fallback is guarded), probe-lane3 (it only ever
   SIGTERMed: `kill_ours`), probe-step3's opt-in crash row (`kill_ours -9`).
5. probe-idle-paint.py: `launch()` records the one pid it started (file `$OUT/ours.pid`), `quit_app()` runs the
   helper's `quit_ours` on that pid through bash; probe-idle-paint.sh's safety net quits only that recorded pid.
6. ATTACH: probe-ui-files-rename.sh `UIFR_ATTACH=1` attached to ANY single running Audio-DNA. Now bf10's rule
   (copied from probe-milkdrop.sh): the running pid == `UIFR_ATTACH_PID`, else the lock helper's start_app record
   (`LOCK_LIB` + `LANE`), it owns the 8080 listener and health answers; else REFUSE, exit 2, no request. Its private
   `quit_ours` is gone (the helper's is used). probe-milkdrop.sh: attach block untouched; its standalone branch uses
   the helper; probe-milkdrop-selftest still 51 ok / 0 FAIL.
7. probe-outputs.sh: converted (quit path + refusal), NEVER run -- not even under the shims.
No threshold, row, timing or fixture changed. H-3 held: quit_ours' "only OURPID running -> quit by name" branch is
byte-identical.

### Step 3 -- k1b_duplicate (gates-r2 NIT 4)
`("k1b_duplicate", k1b_duplicate)` is in probe-boxes.py's row list. In a run that includes k1b_switch_video it still
runs inside that row's flow on the shared fixture (t ~ 7 s) and the registered entry prints one "ran inside ..."
line (no second PASS: full-run totals are unchanged). Selected alone it sets K1b's show up itself (same `lead` 4.5 s).
Live, `probe-boxes.sh <out> k1b_duplicate`:
```
lane  (build-lane app):   PASS  k1b_duplicate: the video keeps playing on screen across Duplicate Deck (HTTP 200; numDecks 3 (was 2), activeDeck 2; t_dup 4.47 s, t at +2.01 s = 6.45 s, expected 6.48 +- 0.5)
                          PY 1 PASS / 0 FAIL / 0 BLOCKED (arm BF9B)      PROBE-BOXES GREEN
main 5abdf01 (H-5 arm):   FAIL  k1b_duplicate: the video keeps playing on screen across Duplicate Deck (HTTP 200; numDecks 3 (was 2), activeDeck 2; t_dup 4.47 s, t at +2.01 s = 1.84 s, expected 6.48 +- 0.5)
                          PY 0 PASS / 1 FAIL / 0 BLOCKED (arm STAGE_P)   PROBE-BOXES RED
```
Same RED pattern as the lane report's STAGE_P table (1.93 vs 6.57: the copy restarts the clip). Frame looked at:
`k1b_dup_after.png` (lane) = one flat olive colour over the whole 1920x1080 canvas (the ramp video's colour = t).

### Step 4 -- proof
(a) grep (evidence `bf9b-merge-M2/grep-final.txt`): 25 hits in the tree; outside `.harmony/.reports/` exactly 3 --
```
.harmony/probe-quit-ours.sh:60:      osascript -e 'tell application "Audio-DNA" to quit' >/dev/null 2>&1     (quit_ours, only-ours branch, unchanged)
.harmony/probe-quit-ours.sh:84:  osascript -e 'tell application "Audio-DNA" to quit' >/dev/null 2>&1         (ask_ours_to_quit, only-ours)
.harmony/probe-quit-ours-selftest.sh:137:HITS="$(grep -nE '...                                             (the selftest's own pattern string)
```
0 comment-line hits are left either (headers reworded). The other 21 are the archived scripts (stop item 1).
(b) `bash -n` on the 37 edited / new .sh: 0 errors; `python -m py_compile` probe-idle-paint.py, probe-boxes.py,
tests/visual/test_output_window_level.py: ok.
(c) selftest `.harmony/probe-quit-ours-selftest.sh --sweep` (evidence `selftest-final.log`): `SELFTEST 88 ok / 0 FAIL`.
No Audio-DNA is involved: the helper is driven through its seam (the caller's `adna_pids` lists re-parented `sleep`
dummies, `QUIT_OURS_UCOMM=sleep`, `osascript` is a shell function of the selftest).
```
ok a2 foreign running at start -> returns 1, the line names pid 90141: REFUSE: Audio-DNA already running (pid 90141) -- this run did not start it ...
ok b2 the process appears 1 s after the launch -> still recorded            ok b3 no pid -> OURPID empty + WARN      ok b4 two pids -> none is ours
ok c  ours only -> quits: rc 0, quit events 1, ours alive no
ok d  ours ignores the quit event -> killed after the 30 s wait: rc 0, events 1, 33 s, ours alive no
ok e1 foreign appeared before the quit -> refuse: rc 1, quit events 0     ok e2 FOREIGN Audio-DNA pid 90989 running -- untouched
ok e3 the foreign pid 90989 is alive after (left alone)                    ok e4 ours (90974) is gone (SIGTERM to OURPID only)
ok f1 no pid recorded, nothing running -> says so, rc 0, events 0: quit_ours: no pid recorded by this run -- nothing is quit
ok f2 no pid recorded, a foreign pid running -> quits nothing and says so: rc 1, events 0, foreign alive yes
ok g  OURPID's ucomm is no longer ours (recycled pid) -> no event, no signal
ok h1-h4 ask_ours_to_quit / kill_ours
part 3 static: 34 launching scripts each source the helper + refuse_foreign_start + record_ourpid; by-name code lines outside the helper: 0
part 2 --sweep (lock held, no Audio-DNA running; `ps` shim reports ONE foreign Audio-DNA pid 99999, `open` / `osascript` shims only log, `curl` shim fails):
  33 x  ok r <script>: rc 64, open calls 0, osascript calls 0 -- REFUSE: Audio-DNA already running (pid 99999) -- this run did not start it ...
  skip probe-outputs.sh (never run)   skip probe-tsan.sh (no TSan app bundle exists on this machine to pass its spec check)
```
RED first: the same selftest on the unconverted tree = `SELFTEST 19 ok / 35 FAIL` (90 by-name code lines listed).
Teeth: 7 mutated COPIES of the helper (scratch; helper sha256 f3b76ba6... before and after) each FAIL:
M1 kills the foreign pid (e3, f2) | M2 by-name quit with a foreign pid running (e1, e2, f2) | M3 no start refusal
(a2) | M4 record without the wait (b2) | M5 ask with foreign (h1) | M6 silent no-pid (f1, f2) | M7 recycled pid is
ours (g).
(d) live smoke under the lock (14:05:10 -> 14:10:57, lane app `build-lane/.../Audio-DNA.app`; logs `bf9b-merge-M2/live/`):
```
probe-deck-tabs    ours: pid 13352   6 PASS / 0 FAIL  (R6: app quit, 0 Audio-DNA windows in the FULL window list)   adna after: []
probe-image-load   ours: pid 14595   PY 37 PASS / 0 FAIL   PASS  app terminated   PROBE-IMAGE-LOAD GREEN              adna after: []
probe-crossfade    ours: pid 15207   PY 35 PASS / 0 FAIL   PASS  app terminated   PROBE-CROSSFADE GREEN               adna after: []
probe-boxes k1b_duplicate   ours: pid 16720 (lane) GREEN / ours: pid 16915 (main arm) RED -- step 3                    adna after: []
```
No FOREIGN / WARN / REFUSE line in any log; each run found no Audio-DNA before and left none after.
(e) 16 s after the last quit: `audio-dna windows 0, Output-named 0`; `UserNotificationCenter windows (OptionAll): 0`;
same again at 14:12:38 after the last selftest. Lock released 14:12:38. No Output window was opened.
ctest: tests/visual/test_output_window_level.py changed (a printed string; not a ctest), so per the packet: incremental
rebuild rc 0 (nothing to compile), full serial ctest under the cross-lane mutex 14:13:20 -> 14:15:28: `100% tests passed, 0 tests failed out of 1234`,
Total Test time (real) = 127.69 sec. TSan: not re-run (no src change; M1's result stands).

### Deviations (flagged)
- H-3 says quit_ours is not changed. Its by-name branch is untouched, but I ADDED one line at its top (the "no pid
  recorded ... nothing is quit" message the packet's proof (c) asks for) and a second by-name line exists in the NEW
  `ask_ours_to_quit` (same only-ours condition). Harmony may want to rule on both.
- "Extend the helper's selftest": none existed; I wrote one.
- The packet's pattern missed `quit app "Audio-DNA"` (8 scripts); the sweep covers it.

### stop items for Harmony
1. 15 archived evidence scripts under `.harmony/.reports/` still quit / kill by name (s-rta-0926b: run_diag.sh,
   stop-witness.sh; s-rta-0927: witness.sh, capture_race.sh, 2 x lock.sh, probe-routines-timed.sh, run-t2.sh, live.sh
   (`pkill -x Audio-DNA`); lock.sh of s-rta-0928 / 0928b / 0929 / 0929b / 0930 / 1002b). They are committed session
   records, so I did not edit them. Rule: neutralise (an `exit 64` first line), convert, or leave as records.
2. probe-ui-files-rename.sh `UIFR_ATTACH=1` now REFUSES unless `UIFR_ATTACH_PID` (or `LOCK_LIB` + `LANE`) names the
   running test-mode app: any gate script that attached without them must pass one (M3 / Harmony's G3 runs).
3. (from M1) probe-ui-files-rename R3 `tab_row_builds +1` still needs the ruling before M3 re-runs it.

### found_not_fixed
1. probe-lane3.sh launches with plain `open "$APPBUNDLE"` and probe-step3.sh's opt-in crash row with `open --stdout`
   (no `-g`): both bring the app to the front. Pre-existing, outside R-N1; not changed.
2. gate-s165.sh still finds its pid with `pgrep -f 'MacOS/Audio-DNA'` (argv match); `ours_running` re-checks the
   ucomm before any event, so a wrong pid is never quit. Not changed.
3. probe-tsan.sh's start refusal could not be exercised under the shims: no TSan APP bundle exists (build-tsan holds
   tests only). Static check only.
4. Of the converted scripts, 4 ran live to the end (deck-tabs, image-load, crossfade, boxes); the others ran only
   to their refusal line. A typo past that line in a never-run script would
   show at its next real run (`bash -n` is clean on all).
5. The graphify post-commit hook fired on every commit (not mine; worktree stayed clean).

### next_stage_notes (M3 builder)
- Every probe now prints `ours: pid N` right after the launch and refuses with `REFUSE: Audio-DNA already running
  (pid N) ...` -- a refusal means STOP and release the lock.
- `probe-boxes.sh <out> k1b_duplicate` works alone; RED on main 5abdf01 is recorded above (H-5 arm).
- m9b_deck_switch_live goes in probe-milkdrop.py; probe-milkdrop.sh's standalone branch now uses the helper, the
  attach / LOCK_LIB paths are as bf10 left them.
- probe-ui-files-rename with UIFR_ATTACH=1: export UIFR_ATTACH_PID (stop item 2).
- No src / CMake change in M2: build-lane's app is still the 13:43 build of 98d71fe's src.

### Notes for .harmony/notebook.md (Harmony appends)
- A new file under `.harmony/` needs `git add -f` (`.gitignore:64 .harmony/*`); a plain `git add a b` with one
  ignored path stages the tracked one and silently leaves the new one out. | discovered: .harmony/probe-quit-ours-selftest.sh
- macOS: a symlink to /bin/sleep keeps ucomm "sleep" and a COPY of /bin/sleep did not stay alive (gone within 0.3 s; cause not looked into), so a dummy process
  cannot be given the ucomm "Audio-DNA" cheaply; test a ucomm-keyed helper through a seam (caller-defined pid list +
  a ucomm variable) instead. | discovered: .harmony/probe-quit-ours-selftest.sh
- A shell FUNCTION named like a command (`osascript(){...}`) shadows PATH for everything sourced into that shell:
  the safe way to selftest a helper that would otherwise send a real Apple event. | discovered: same
- `kill -0` succeeds on a zombie child: selftest dummies must be re-parented (`( sleep 300 & )`) or reaped.
  | discovered: same

### PACKET QUALITY
- Clarity: CLEAR (two HAD_TO_INFER points: "extend the helper's selftest" when none existed; whether the archived
  `.harmony/.reports/**` scripts are in the sweep -- left as stop item 1).
- Missing context: that `.harmony/*` is gitignored for new files; that no TSan app bundle exists.
- Unused context: plan-bf9b.md / ruling-bf9b.md (not needed for a probe-rig sweep); the bf9b.md fix-round sections
  (M1's notes + the review file carried the needed lines).
- Self-brief files: rulings-bf9b-merge.md, rulings-bf9b-mergein.md, review-bf9b-gates-r2.md (NIT 4 / NIT 6),
  probe-milkdrop.sh + its selftest (the bf10 pattern) -- all useful, none stale. No DEPARTMENT / KNOWLEDGE_TOOLS
  block: no knowledge tools -- grep-only (nothing was judged dead on "no callers": `adna_kill` was deleted with every
  call site converted, grep = 0).
- pulse.json: GREEN; claims "general" by two harmony sessions (the dispatcher), no area conflict.
INBOX-RECHECK: none

### SLIM CHECK
36 probe-rig files changed (the helper, 32 .sh + 1 .py of the sweep, probe-canvas / -boxes / -render-state,
probe-boxes.py -- see `git diff --stat 555983a..HEAD`) + 1 new selftest + 1 test file; every line traces to R-N1 / H-4. New helper surface: 3 functions
(refuse_foreign_start, ask_ours_to_quit, kill_ours). No src, no CMake, no threshold. Smells named: the 33 scripts
still each carry their own copy of the lock gate + adna_pids preamble (duplicated code, pre-existing, not touched).
