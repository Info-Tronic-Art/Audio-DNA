# Reviewer Verdict -- lane ui, lens gates, round 1
STATUS: PARTIAL
VERDICT: FAIL (REQUEST_CHANGES) -- 1 MUST
REVIEWED: branch lane/ui, base eff2b1c, head 24e1f00 (read via git show / git diff only)

## MUST
M1  .harmony/probe-ui-files-rename.sh:93 (set -u at :42, ENVS=() at :90): in the default STANDALONE mode (no UIFR_ATTACH, no --hook)
    "${ENVS[@]}" is an empty array expanded under `set -u`. macOS /bin/bash is 3.2.57 (the only bash on this machine; the shebang is
    #!/bin/bash) and aborts with "ENVS[@]: unbound variable", exit 1, BEFORE `open` launches the app. VERIFIED by running the script's
    own lines 90-93 under /bin/bash 3.2.57 with `open` shimmed: "launch: /x  --test-mode / line 7: ENVS[@]: unbound variable / rc=1".
    G3 says "Run .harmony/probe-ui-files-rename.sh OUT" = this path. The lane only ever ran UIFR_ATTACH=1 (and --hook, whose ENVS is
    non-empty), so it never saw it.
    Fix: `open ... ${ENVS[@]+"${ENVS[@]}"} "$APP" ...` (or ENVS=(--test-flag-less dummy), or drop set -u for that line); then run the
    standalone path once in a locked batch.

## SHOULD
S1  The ordering "hand the keyboard home BEFORE the box hides" (DeckView.cpp:664-670, ruling E-R4, Pitfall 65 point 2) is covered by NO
    test that runs: tests/test_deck_tab_rename.cpp Rig counts `++closes` only (:202), the headless focus is always "nowhere", and G3's
    focus_home_count is only a counter. Moving onRenameClosed() after setVisible(false), or deleting the `focused == nullptr ||
    isParentOf(focused)` test, keeps every unit test and G3 green; only probe_deck_tab_dispatch P5-P7 (never run by the lane) would
    catch it. Proposed mutation (not run; C++ rebuild): swap DeckView.cpp:668-670. Cheap fix: in Rig record
    `renameEditorForTests()->isVisible()` inside the onRenameClosed lambda and CHECK it is true (all (e)/(i) sections), plus one case
    with a focused non-DeckView component asserting onRenameClosed is NOT called.
S2  G3 V1 reads `lines` (describe) for all 8 cells; the Clip inspector's own rows are checked live only for L0C0 and L1C3 (V4).
    HAP Q / missing / PNG / sequence inspector rows are checked by unit test (c) with a fake source and by decoded-not-read captures
    C9-C12. Adding select-then-assert `inspector_lines` for each cell would close it (the route already returns them).

## NIT
N1  Probe default C5 deck count is 27 not the ruling's 12 (12 decks give 100-px tabs at row_width 1720). The lane flagged it and the
    DeckTabRow::layout arithmetic matches (checked: n=27 -> 60 px, n=26 -> 63). NB1's numeric bars still hold. No action.
N2  ClipInspector::updateMediaInfo (ClipInspector.cpp ~1105) keeps videoPending_ true for a file-backed, not-missing video whose player
    never opens (corrupt file): every refresh tick then takes Renderer::videoPlayerMutex_ via getVideoPlayer. Message thread only, a
    tiny map lookup, within the existing msg_video_lock_wait_max_ms accounting; the dropped G6 would have been the check.
N3  CLAUDE.md: removing "### Common Build Issues" is permitted by plan section 6, but the Trigger Table row (:251) names only
    "Building on Windows/Linux, adding Aubio, new dependency"; macOS FetchContent / GL-deprecation no longer route to
    build-other-platforms.md (the topics are in that doc, lines 11-14). Plan allowed moving the topic words into that row.
N4  probe_deck_tab_dispatch (G1b) is statically sound but unrun: geometry checked (22+22+3*96+24 = 356, tab row at the last 24 px,
    P7 click (565,60) = layer 0 col 3 thumbnail, visible above the horizontal scrollbar). P7's hard-coded 250/90 constants are the only
    fragile part; an unexpected P7 FAIL on Harmony's first run should be read against them first.

## VERIFIED (focus list)
- probe_deck_tab_dispatch not in ctest: `ctest -N` in worktree build-lane = 1174 tests, 0 matches for probe_deck; CMake target has no
  add_test / catch_discover_tests (tests/CMakeLists.txt, `if(APPLE)` block); binary exists (built, 8.9 MB). Never run by the lane per
  its report and adoption 3. VERIFIED.
- Temporary hook absent: `git grep AUDIODNA_DEBUG_SHOW|AUDIODNA_DEBUG_SNAP 24e1f00 -- src tests` = 0 lines (G2b); `strings -a` of
  build-lane Audio-DNA binary = 0 matches; the only mentions are in .harmony/probe-ui-files-rename.sh (pass-through env for a
  scratch build) and docs/claude/testing-eyes.md. No getenv hook added in src. No .venv entry, .new, .orig, mutant, printf/DBG
  left in the diff. VERIFIED.
- CLAUDE.md = 24,001 B at head (base 24,002; net -1; cap 25,000). Main HEAD e89bb5f = 23,976 B, so the merged file stays under the cap;
  Pitfall index jumps 63 -> 65 on the lane, main has 64 (mkvidx): keep both, 64 first. VERIFIED.
- Docs per AM16: performance-controls.md (two paragraphs), pitfalls.md 65 (4 points + rebuild/load), testing-eyes.md (8 routes + both
  probes), APP-INVENTORY rows DeckView / ClipCell / ClipInspector + route list: all present and match the code read. VERIFIED.
- probe-ui-files-rename.sh vs G3, row by row (read in full): S0 settle rule, V1 x8 (strings equal to the ruling, missing true,
  file_backed false), V2 x4 (tooltip literal, sequence first line, "" , 3 menus), V3 x3 (path, +1 / +0 / +1), V4 x2, R1-R10 incl.
  R4 two-step, R5 undo.top / index+1 / focus+1, R7 focus+3 and unchanged undo, R8 x3 (index+1 asserted on all three: stricter than
  the ruling), R9a/b, R10 -- all present with the ruling's bars. Teeth: RED on main's app is 1 PASS / 31 FAIL but is 404-driven (inherent:
  the routes are new); the discriminating RED is the unit RED. Row logic itself cannot be shown RED on base.
- Unit tests drive real code: test_deck_tab_rename (real DeckView + nested listener, events in JUCE order), test_clip_cell_media (real
  ClipCell + DeckView fan-out), test_clip_inspector_media (real ClipInspector), test_video_info [open] (real FFmpeg open of 9
  fixtures). RED(stub) quoted in every feature commit; guards that pass on a stub are named in the commits ((a)-(b) guards,
  (c2)(d)(d2), (g)(h) text measures). U3.3 / U2.5 have no unit test by design (MainComponent), covered by G3.
- Real-time rules: no change on the audio callback or analysis thread; no new mutex; VideoPlayer diff = open() +14 lines and one
  accessor / member (mkvidx fence respected: no readKeyIndex / decodeStep / runStep / seek touched); info_ is written in open()
  before installVideoPlayer's mutex publication and read on the message thread; render thread never waits on anything new.
- TEST-ONLY REST handlers are inside the existing `#if AUDIODNA_TEST_SERVER` regions (ApiServer.cpp 299-339, 1786-2371;
  MainComponent.cpp 2178-2303; ApiServer.h block). Reveal under --test-mode is recorded, never sent to Finder.

CONFIDENCE: VERIFIED for M1 (executed), ctest/probe/CMake/strings/CLAUDE.md facts, row-by-row G3 comparison. INFERRED for S1
(no mutation run: needs a C++ rebuild, proposed). G0 / G1 / G1b / G3 / G4 results are Harmony's and were not re-run.
